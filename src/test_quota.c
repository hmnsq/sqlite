/*
** 2010 September 31
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains a VFS "shim" - a layer that sits in between the
** pager and the real VFS.
**
** This particular shim enforces a quota system on files.  One or more
** database files are in a "quota group" that is defined by a GLOB
** pattern.  A quota is set for the combined size of all files in the
** the group.  A quota of zero means "no limit".  If the total size
** of all files in the quota group is greater than the limit, then
** write requests that attempt to enlarge a file fail with SQLITE_FULL.
**
** However, before returning SQLITE_FULL, the write requests invoke
** a callback function that is configurable for each quota group.
** This callback has the opportunity to enlarge the quota.  If the
** callback does enlarge the quota such that the total size of all
** files within the group is less than the new quota, then the write
** continues as if nothing had happened.
*/
#include "test_quota.h"
#include <string.h>
#include <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#если defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)
#define sqlite3_mutex_held(X)     ((void)(X),1)
#define sqlite3_mutex_notheld(X)  ((void)(X),1)
#endif /* SQLITE_THREADSAFE==0 */


#ifdef _WIN32
# include <windows.h>
# include <io.h>
#иначе
# include <unistd.h>
#endif


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
typedef struct quotaGroup quotaGroup;
typedef struct quotaConn quotaConn;
typedef struct quotaFile quotaFile;

/*
** A "quota group" is a collection of files whose collective size we want
** to limit.  Each quota group is defined by a GLOB pattern.
**
** There is an instance of the following object for each defined quota
** group.  This object records the GLOB pattern that defines which files
** belong to the quota group.  The object also remembers the size limit
** for the group (the quota) and the callback to be invoked when the
** sum of the sizes of the files within the group goes over the limit.
**
** A quota group must be established (using sqlite3_quota_set(...))
** prior to opening any of the database connections that access files
** within the quota group.
*/
struct quotaGroup {
  const char *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 iSize;           /* Current size of all files */
  void (*xCallback)(             /* Callback invoked when going over quota */
     const char *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     void *pArg                     /* Client data */
  );
  void *pArg;                    /* Third argument to the xCallback() */
  void (*xDestroy)(void*);       /* Optional destructor for pArg */
  quotaGroup *pNext, **ppPrev;   /* Doubly linked list of all quota objects */
  quotaFile *pFiles;             /* Files within this group */
};

/*
** An instance of this structure represents a single file that is part
** of a quota group.  A single file can be opened multiple times.  In
** order keep multiple openings of the same file from causing the size
** of the file to count against the quota multiple times, each file
** has a unique instance of this object and multiple open connections
** to the same file each point to a single instance of this object.
*/
struct quotaFile {
  char *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 iSize;            /* Current size of this file */
  целое nRef;                       /* Number of times this file is open */
  целое deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *pNext, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
struct quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
struct quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ifdef _WIN32
  char *zMbcsName;        /* Full MBCS pathname of the file */
#endif
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
static struct {
  /* The pOrigVfs is the real, original underlying VFS implementation.
  ** Most operations pass-through to the real VFS.  This value is read-only
  ** during operation.  It is only modified at start-time and thus does not
  ** require a mutex.
  */
  sqlite3_vfs *pOrigVfs;

  /* The sThisVfs is the VFS structure used by this shim.  It is initialized
  ** at start-time and thus does not require a mutex
  */
  sqlite3_vfs sThisVfs;

  /* The sIoMethods defines the methods used by sqlite3_file objects
  ** associated with this shim.  It is initialized at start-time and does
  ** not require a mutex.
  **
  ** When the underlying VFS is called to open a file, it might return
  ** either a version 1 or a version 2 sqlite3_file object.  This shim
  ** has to create a wrapper sqlite3_file of the same version.  Hence
  ** there are two I/O method structures, one for version 1 and the other
  ** for version 2.
  */
  sqlite3_io_methods sIoMethodsV1;
  sqlite3_io_methods sIoMethodsV2;

  /* True when this shim as been initialized.
  */
  целое isInitialized;

  /* For run-time access any of the other global data structures in this
  ** shim, the following mutex must be held.
  */
  sqlite3_mutex *pMutex;

  /* List of quotaGroup objects.
  */
  quotaGroup *pGroup;

} gQuota;

/************************* Utility Routines *********************************/
/*
** Acquire and release the mutex used to serialize access to the
** list of quotaGroups.
*/
static void quotaEnter(void){ sqlite3_mutex_enter(gQuota.pMutex); }
static void quotaLeave(void){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
static целое quotaGroupOpenFileCount(quotaGroup *pGroup){
  целое N = 0;
  quotaFile *pFile = pGroup->pFiles;
  пока( pFile ){
    если( pFile->nRef ) N++;
    pFile = pFile->pNext;
  }
  вернуть N;
}

/* Remove a file from a quota group.
*/
static void quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->iSize -= pFile->iSize;
  *pFile->ppPrev = pFile->pNext;
  если( pFile->pNext ) pFile->pNext->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
static void quotaRemoveAllFiles(quotaGroup *pGroup){
  пока( pGroup->pFiles ){
    assert( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
static void quotaGroupDeref(quotaGroup *pGroup){
  если( pGroup->iLimit==0 && quotaGroupOpenFileCount(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->ppPrev = pGroup->pNext;
    если( pGroup->pNext ) pGroup->pNext->ppPrev = pGroup->ppPrev;
    если( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
    sqlite3_free(pGroup);
  }
}

/*
** Return TRUE if string z matches glob pattern zGlob.
**
** Globbing rules:
**
**      '*'       Matches any sequence of zero or more characters.
**
**      '?'       Matches exactly one character.
**
**     [...]      Matches one character from the enclosed list of
**                characters.
**
**     [^...]     Matches one character not in the enclosed list.
**
**     /          Matches "/" or "\\"
**
*/
static целое quotaStrglob(const char *zGlob, const char *z){
  целое c, c2, cx;
  целое invert;
  целое seen;

  пока( (c = (*(zGlob++)))!=0 ){
    если( c=='*' ){
      пока( (c=(*(zGlob++))) == '*' || c=='?' ){
        если( c=='?' && (*(z++))==0 ) вернуть 0;
      }
      если( c==0 ){
        вернуть 1;
      }иначе если( c=='[' ){
        пока( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        вернуть (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      пока( (c2 = (*(z++)))!=0 ){
        пока( c2!=c && c2!=cx ){
          c2 = *(z++);
          если( c2==0 ) вернуть 0;
        }
        если( quotaStrglob(zGlob,z) ) вернуть 1;
      }
      вернуть 0;
    }иначе если( c=='?' ){
      если( (*(z++))==0 ) вернуть 0;
    }иначе если( c=='[' ){
      целое prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      если( c==0 ) вернуть 0;
      c2 = *(zGlob++);
      если( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      если( c2==']' ){
        если( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      пока( c2 && c2!=']' ){
        если( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          если( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }иначе{
          если( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      если( c2==0 || (seen ^ invert)==0 ) вернуть 0;
    }иначе если( c=='/' ){
      если( z[0]!='/' && z[0]!='\\' ) вернуть 0;
      z++;
    }иначе{
      если( c!=(*(z++)) ) вернуть 0;
    }
  }
  вернуть *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
static quotaGroup *quotaGroupFind(const char *zFilename){
  quotaGroup *p;
  для(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->pNext){}
  вернуть p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
static sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  вернуть (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
static quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  const char *zName,      /* Full pathname of the file */
  целое createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  пока( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->pNext;
  }
  если( pFile==0 && createFlag ){
    целое nName = (целое)(strlen(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( sizeof(*pFile) + nName + 1 );
    если( pFile ){
      memset(pFile, 0, sizeof(*pFile));
      pFile->zFilename = (char*)&pFile[1];
      memcpy(pFile->zFilename, zName, nName+1);
      pFile->pNext = pGroup->pFiles;
      если( pGroup->pFiles ) pGroup->pFiles->ppPrev = &pFile->pNext;
      pFile->ppPrev = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  вернуть pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
static char *quota_utf8_to_mbcs(const char *zUtf8){
#ifdef _WIN32
  size_t n;          /* Bytes in zUtf8 */
  целое nWide;         /* number of UTF-16 characters */
  целое nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  char *zMbcs;       /* The MBCS text */
  целое codepage;      /* Code page used by fopen() */

  n = strlen(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, NULL, 0);
  если( nWide==0 ) вернуть 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*sizeof(zTmpWide[0]) );
  если( zTmpWide==0 ) вернуть 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (char*)sqlite3_malloc( nMbcs+1 ) : 0;
  если( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  вернуть zMbcs;
#иначе
  вернуть (char*)zUtf8;  /* No-op on unix */
#endif
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
static void quota_mbcs_free(char *zOld){
#ifdef _WIN32
  sqlite3_free(zOld);
#иначе
  /* No-op on unix */
#endif
}

/************************* VFS Method Wrappers *****************************/
/*
** This is the xOpen method used for the "quota" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate quota group if it is a
** file that needs to be tracked.
*/
static целое quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  const char *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  целое flags,                  /* Flags to control the opening */
  целое *pOutFlags              /* Flags showing results of opening */
){
  целое rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  если( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    вернуть pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  если( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }иначе{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    если( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      если( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        вернуть SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      если( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }иначе{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  вернуть rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
static целое quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  const char *zName,          /* Name of file to be deleted */
  целое syncDir                 /* Do a directory sync after deleting */
){
  целое rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  если( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    если( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      если( pFile ){
        если( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }иначе{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  вернуть rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
static целое quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  целое rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  если( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    если( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  вернуть rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
static целое quotaRead(
  sqlite3_file *pConn,
  void *pBuf,
  целое iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
static целое quotaWrite(
  sqlite3_file *pConn,
  const void *pBuf,
  целое iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  если( pFile->iSize<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    если( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      если( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      если( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        вернуть SQLITE_FULL;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }
  вернуть pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
static целое quotaTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  целое rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  если( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = size;
    pGroup->iSize += size;
    quotaLeave();
  }
  вернуть rc;
}

/* Pass xSync requests through to the original VFS without change
*/
static целое quotaSync(sqlite3_file *pConn, целое flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
static целое quotaFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  целое rc;

  rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz);
  если( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = sz;
    pGroup->iSize += sz;
    quotaLeave();
    *pSize = sz;
  }
  вернуть rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
static целое quotaLock(sqlite3_file *pConn, целое lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
static целое quotaUnlock(sqlite3_file *pConn, целое lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
static целое quotaCheckReservedLock(sqlite3_file *pConn, целое *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
static целое quotaFileControl(sqlite3_file *pConn, целое op, void *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  целое rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#если defined(SQLITE_FCNTL_VFSNAME)
  если( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(char**)pArg = sqlite3_mprintf("quota/%z", *(char**)pArg);
  }
#endif
  вернуть rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
static целое quotaSectorSize(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xSectorSize(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
static целое quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
static целое quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  целое iRegion,                    /* Region to retrieve */
  целое szRegion,                   /* Size of regions */
  целое bExtend,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
static целое quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  целое ofst,                  /* First lock to acquire or release */
  целое n,                     /* Number of locks to acquire or release */
  целое flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
static void quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
static целое quotaShmUnmap(sqlite3_file *pConn, целое deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  вернуть pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
}

/************************** Public Interfaces *****************************/
/*
** Initialize the quota VFS shim.  Use the VFS named zOrigVfsName
** as the VFS that does the actual work.  Use the default if
** zOrigVfsName==NULL.
**
** The quota VFS shim is named "quota".  It will become the default
** VFS if makeDefault is non-zero.
**
** THIS ROUTINE IS NOT THREADSAFE.  Call this routine exactly once
** during start-up.
*/
целое sqlite3_quota_initialize(const char *zOrigVfsName, целое makeDefault){
  sqlite3_vfs *pOrigVfs;
  если( gQuota.isInitialized ) вернуть SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  если( pOrigVfs==0 ) вернуть SQLITE_ERROR;
  assert( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  если( !gQuota.pMutex ){
    вернуть SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += sizeof(quotaConn);
  gQuota.sThisVfs.zName = "quota";
  gQuota.sIoMethodsV1.iVersion = 1;
  gQuota.sIoMethodsV1.xClose = quotaClose;
  gQuota.sIoMethodsV1.xRead = quotaRead;
  gQuota.sIoMethodsV1.xWrite = quotaWrite;
  gQuota.sIoMethodsV1.xTruncate = quotaTruncate;
  gQuota.sIoMethodsV1.xSync = quotaSync;
  gQuota.sIoMethodsV1.xFileSize = quotaFileSize;
  gQuota.sIoMethodsV1.xLock = quotaLock;
  gQuota.sIoMethodsV1.xUnlock = quotaUnlock;
  gQuota.sIoMethodsV1.xCheckReservedLock = quotaCheckReservedLock;
  gQuota.sIoMethodsV1.xFileControl = quotaFileControl;
  gQuota.sIoMethodsV1.xSectorSize = quotaSectorSize;
  gQuota.sIoMethodsV1.xDeviceCharacteristics = quotaDeviceCharacteristics;
  gQuota.sIoMethodsV2 = gQuota.sIoMethodsV1;
  gQuota.sIoMethodsV2.iVersion = 2;
  gQuota.sIoMethodsV2.xShmMap = quotaShmMap;
  gQuota.sIoMethodsV2.xShmLock = quotaShmLock;
  gQuota.sIoMethodsV2.xShmBarrier = quotaShmBarrier;
  gQuota.sIoMethodsV2.xShmUnmap = quotaShmUnmap;
  sqlite3_vfs_register(&gQuota.sThisVfs, makeDefault);
  вернуть SQLITE_OK;
}

/*
** Shutdown the quota system.
**
** All SQLite database connections must be closed before calling this
** routine.
**
** THIS ROUTINE IS NOT THREADSAFE.  Call this routine exactly once while
** shutting down in order to free all remaining quota groups.
*/
целое sqlite3_quota_shutdown(void){
  quotaGroup *pGroup;
  если( gQuota.isInitialized==0 ) вернуть SQLITE_MISUSE;
  для(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    если( quotaGroupOpenFileCount(pGroup)>0 ) вернуть SQLITE_MISUSE;
  }
  пока( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->pNext;
    pGroup->iLimit = 0;
    assert( quotaGroupOpenFileCount(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, sizeof(gQuota));
  вернуть SQLITE_OK;
}

/*
** Create or destroy a quota group.
**
** The quota group is defined by the zPattern.  When calling this routine
** with a zPattern for a quota group that already exists, this routine
** merely updates the iLimit, xCallback, and pArg values for that quota
** group.  If zPattern is new, then a new quota group is created.
**
** If the iLimit for a quota group is set to zero, then the quota group
** is disabled and will be deleted when the last database connection using
** the quota group is closed.
**
** Calling this routine on a zPattern that does not exist and with a
** zero iLimit is a no-op.
**
** A quota group must exist with a non-zero iLimit prior to opening
** database connections if those connections are to participate in the
** quota group.  Creating a quota group does not affect database connections
** that are already open.
*/
целое sqlite3_quota_set(
  const char *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  void (*xCallback)(              /* Callback invoked when going over quota */
     const char *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     void *pArg                     /* Client data */
  ),
  void *pArg,                     /* client data passed thru to callback */
  void (*xDestroy)(void*)         /* Optional destructor for pArg */
){
  quotaGroup *pGroup;
  quotaEnter();
  pGroup = gQuota.pGroup;
  пока( pGroup && strcmp(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->pNext;
  }
  если( pGroup==0 ){
    целое nPattern = (целое)(strlen(zPattern) & 0x3fffffff);
    если( iLimit<=0 ){
      quotaLeave();
      вернуть SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( sizeof(*pGroup) + nPattern + 1 );
    если( pGroup==0 ){
      quotaLeave();
      вернуть SQLITE_NOMEM;
    }
    memset(pGroup, 0, sizeof(*pGroup));
    pGroup->zPattern = (char*)&pGroup[1];
    memcpy((char *)pGroup->zPattern, zPattern, nPattern+1);
    если( gQuota.pGroup ) gQuota.pGroup->ppPrev = &pGroup->pNext;
    pGroup->pNext = gQuota.pGroup;
    pGroup->ppPrev = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  если( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  вернуть SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
целое sqlite3_quota_file(const char *zFilename){
  char *zFull = 0;
  sqlite3_file *fd;
  целое rc;
  целое outFlags = 0;
  sqlite3_int64 iSize;
  целое nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  если( fd==0 ){
    rc = SQLITE_NOMEM;
  }иначе{
    zFull = &((char *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  если( rc==SQLITE_OK ){
    zFull[strlen(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    если( rc==SQLITE_OK ){
      fd->pMethods->xFileSize(fd, &iSize);
      fd->pMethods->xClose(fd);
    }иначе если( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      если( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        если( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  вернуть rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(const char *zFilename, const char *zMode){
  quota_FILE *p = 0;
  char *zFull = 0;
  char *zFullTranslated = 0;
  целое rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (char*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  если( zFull==0 ) вернуть 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  если( rc ) goto quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(sizeof(*p));
  если( p==0 ) goto quota_fopen_error;
  memset(p, 0, sizeof(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  если( zFullTranslated==0 ) goto quota_fopen_error;
  p->f = fopen(zFullTranslated, zMode);
  если( p->f==0 ) goto quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  если( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    если( pFile==0 ){
      quotaLeave();
      goto quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(zFull);
#ifdef _WIN32
  p->zMbcsName = zFullTranslated;
#endif
  вернуть p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  если( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  вернуть 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  void *pBuf,            /* Store the content here */
  size_t size,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  вернуть fread(pBuf, size, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  const void *pBuf,      /* Take content to write from here */
  size_t size,           /* Size of each element */
  size_t nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 iEnd;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  size_t rc;

  iOfst = ftell(p->f);
  iEnd = iOfst + size*nmemb;
  pFile = p->pFile;
  если( pFile && pFile->iSize<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    если( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      если( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      если( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->iSize + pFile->iSize;
        nmemb = (size_t)((iEnd - iOfst)/size);
        iEnd = iOfst + size*nmemb;
        szNew = pGroup->iSize - pFile->iSize + iEnd;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }иначе{
    pFile = 0;
  }
  rc = fwrite(pBuf, size, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  если( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + size*nWritten;
    если( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->iSize += iNewEnd - pFile->iSize;
    pFile->iSize = iNewEnd;
    quotaLeave();
  }
  вернуть rc;
}

/*
** Close an open quota_FILE stream.
*/
целое sqlite3_quota_fclose(quota_FILE *p){
  целое rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  если( pFile ){
    quotaEnter();
    pFile->nRef--;
    если( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      если( pFile->deleteOnClose ){
        gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
        quotaRemoveFile(pFile);
      }
      quotaGroupDeref(pGroup);
    }
    quotaLeave();
  }
#ifdef _WIN32
  quota_mbcs_free(p->zMbcsName);
#endif
  sqlite3_free(p);
  вернуть rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
целое sqlite3_quota_fflush(quota_FILE *p, целое doFsync){
  целое rc;
  rc = fflush(p->f);
  если( rc==0 && doFsync ){
#ifdef _WIN32
    rc = _commit(_fileno(p->f));
#иначе
    rc = fsync(fileno(p->f));
#endif
  }
  вернуть rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
целое sqlite3_quota_fseek(quota_FILE *p, long offset, целое whence){
  вернуть fseek(p->f, offset, whence);
}

/*
** rewind a quota_FILE stream.
*/
void sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
long sqlite3_quota_ftell(quota_FILE *p){
  вернуть ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
целое sqlite3_quota_ferror(quota_FILE *p){
  вернуть ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
целое sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  целое rc;
  если( (pFile = p->pFile)!=0 && pFile->iSize<szNew ){
    quotaGroup *pGroup;
    если( pFile->iSize<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      вернуть -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    quotaLeave();
  }
#ifdef _WIN32
#  если defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (long)szNew);
#  иначе
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#иначе
  rc = ftruncate(fileno(p->f), szNew);
#endif
  если( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    pFile->iSize = szNew;
    quotaLeave();
  }
  вернуть rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
целое sqlite3_quota_file_mtime(quota_FILE *p, time_t *pTime){
  целое rc;
#ifdef _WIN32
  struct _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#иначе
  struct stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  если( rc==0 ) *pTime = buf.st_mtime;
  вернуть rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  целое rc;
#ifdef _WIN32
  struct _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#иначе
  struct stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  вернуть rc==0 ? buf.st_size : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_size(quota_FILE *p){
  вернуть p->pFile ? p->pFile->iSize : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
long sqlite3_quota_file_available(quota_FILE *p){
  FILE* f = p->f;
  long pos1, pos2;
  целое rc;
  pos1 = ftell(f);
  если ( pos1 < 0 ) вернуть -1;
  rc = fseek(f, 0, SEEK_END);
  если ( rc != 0 ) вернуть -1;
  pos2 = ftell(f);
  если ( pos2 < 0 ) вернуть -1;
  rc = fseek(f, pos1, SEEK_SET);
  если ( rc != 0 ) вернуть -1;
  вернуть pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
целое sqlite3_quota_remove(const char *zFilename){
  char *zFull;            /* Full pathname for zFilename */
  size_t nFull;           /* Number of bytes in zFilename */
  целое rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *pNextFile;   /* next file in the group */
  целое diff;               /* Difference between filenames */
  char c;                 /* First character past end of pattern */

  zFull = (char*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  если( zFull==0 ) вернуть SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  если( rc ){
    sqlite3_free(zFull);
    вернуть rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = strlen(zFull);
  если( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  если( pGroup ){
    для(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=pNextFile){
      pNextFile = pFile->pNext;
      diff = strncmp(zFull, pFile->zFilename, nFull);
      если( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        если( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }иначе{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(zFull);
  вернуть rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#include "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
typedef struct TclQuotaCallback TclQuotaCallback;
struct TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

extern const char *sqlite3ErrName(целое);


/*
** This is the callback from a quota-over-limit.
*/
static void tclQuotaCallback(
  const char *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 iSize,            /* Total size of all files in the group */
  void *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  unsigned целое rnd;               /* Random part of pVarname */
  целое rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  если( p==0 ) вернуть;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_IncrRefCount(pVarname);
  sqlite3_randomness(sizeof(rnd), (void *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((целое)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_IncrRefCount(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(iSize));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  если( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_DecrRefCount(pEval);
  Tcl_DecrRefCount(pVarname);
  если( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
static void tclCallbackDestructor(void *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  если( p ){
    Tcl_DecrRefCount(p->pScript);
    sqlite3_free((char *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
static целое SQLITE_TCLAPI test_quota_initialize(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  const char *zName;              /* Name of new quota VFS */
  целое makeDefault;                /* True to make the new VFS the default */
  целое rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  если( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME MAKEDEFAULT");
    вернуть TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  если( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) вернуть TCL_ERROR;
  если( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);

  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
static целое SQLITE_TCLAPI test_quota_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  целое rc;                         /* Value returned by quota_shutdown() */

  если( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    вернуть TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);

  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
static целое SQLITE_TCLAPI test_quota_set(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  const char *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  целое rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_Size nScript;               /* Length of callback script */
  void (*xDestroy)(void*);        /* Optional destructor for pArg */
  void (*xCallback)(const char *, sqlite3_int64 *, sqlite3_int64, void *);

  /* Process arguments */
  если( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    вернуть TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  если( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) вернуть TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  если( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(sizeof(TclQuotaCallback));
    если( !p ){
      Tcl_SetResult(interp, (char *)"SQLITE_NOMEM", TCL_STATIC);
      вернуть TCL_OK;
    }
    memset(p, 0, sizeof(TclQuotaCallback));
    p->interp = interp;
    Tcl_IncrRefCount(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }иначе{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (void*)p, xDestroy);

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
static целое SQLITE_TCLAPI test_quota_file(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  const char *zFilename;          /* File pattern to configure */
  целое rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    вернуть TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);
  вернуть TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
static целое SQLITE_TCLAPI test_quota_dump(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pResult;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  pResult = Tcl_NewObj();
  quotaEnter();
  для(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iSize));
    для(pFile=pGroup->pFiles; pFile; pFile=pFile->pNext){
      целое i;
      char zTemp[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(sizeof(zTemp), zTemp, "%s", pFile->zFilename);
      для(i=0; zTemp[i]; i++){ если( zTemp[i]=='\\' ) zTemp[i] = '/'; }
      Tcl_ListObjAppendElement(interp, pFileTerm,
            Tcl_NewStringObj(zTemp, -1));
      Tcl_ListObjAppendElement(interp, pFileTerm,
            Tcl_NewWideIntObj(pFile->iSize));
      Tcl_ListObjAppendElement(interp, pFileTerm,
            Tcl_NewWideIntObj(pFile->nRef));
      Tcl_ListObjAppendElement(interp, pFileTerm,
            Tcl_NewWideIntObj(pFile->deleteOnClose));
      Tcl_ListObjAppendElement(interp, pGroupTerm, pFileTerm);
    }
    Tcl_ListObjAppendElement(interp, pResult, pGroupTerm);
  }
  quotaLeave();
  Tcl_SetObjResult(interp, pResult);
  вернуть TCL_OK;
}

/* Defined in test1.c */
extern void *sqlite3TestTextToPtr(const char*);
extern const char *sqlite3TestPtrToText(void*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
static целое SQLITE_TCLAPI test_quota_fopen(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  const char *zFilename;          /* File pattern to configure */
  const char *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  если( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME MODE");
    вернуть TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_SetResult(interp, (char*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
static целое SQLITE_TCLAPI test_quota_fread(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  char *zBuf;
  целое sz;
  целое nElem;
  size_t got;

  если( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE SIZE NELEM");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  если( Tcl_GetIntFromObj(interp, objv[2], &sz) ) вернуть TCL_ERROR;
  если( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) вернуть TCL_ERROR;
  zBuf = (char*)sqlite3_malloc( sz*nElem + 1 );
  если( zBuf==0 ){
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
    вернуть TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_SetResult(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
static целое SQLITE_TCLAPI test_quota_fwrite(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  char *zBuf;
  целое sz;
  целое nElem;
  size_t got;

  если( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  если( Tcl_GetIntFromObj(interp, objv[2], &sz) ) вернуть TCL_ERROR;
  если( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) вернуть TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(got));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
static целое SQLITE_TCLAPI test_quota_fclose(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  целое rc;

  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
static целое SQLITE_TCLAPI test_quota_fflush(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  целое rc;
  целое doSync = 0;

  если( objc!=2 && objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE ?HARDSYNC?");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  если( objc==3 ){
    если( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) вернуть TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
static целое SQLITE_TCLAPI test_quota_fseek(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  целое ofst;
  const char *zWhence;
  целое whence;
  целое rc;

  если( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE OFFSET WHENCE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  если( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) вернуть TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  если( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }иначе если( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }иначе если( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }иначе{
    Tcl_AppendResult(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (char*)0);
    вернуть TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
static целое SQLITE_TCLAPI test_quota_rewind(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
static целое SQLITE_TCLAPI test_quota_ftell(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
static целое SQLITE_TCLAPI test_quota_ftruncate(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  целое rc;
  если( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE SIZE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  если( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) вернуть TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
static целое SQLITE_TCLAPI test_quota_file_size(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_size(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
static целое SQLITE_TCLAPI test_quota_file_truesize(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
static целое SQLITE_TCLAPI test_quota_file_mtime(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  time_t t;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(t));
  вернуть TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
static целое SQLITE_TCLAPI test_quota_remove(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  const char *zFilename;          /* File pattern to configure */
  целое rc;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    вернуть TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
static целое SQLITE_TCLAPI test_quota_glob(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  const char *zPattern;          /* The glob pattern */
  const char *zText;             /* Text to compare against the pattern */
  целое rc;
  если( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PATTERN TEXT");
    вернуть TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
static целое SQLITE_TCLAPI test_quota_file_available(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  вернуть TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
static целое SQLITE_TCLAPI test_quota_ferror(
  void * clientData,
  Tcl_Interp *interp,
  целое objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  целое x;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    вернуть TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(x));
  вернуть TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
целое Sqlitequota_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_quota_initialize",    test_quota_initialize },
    { "sqlite3_quota_shutdown",      test_quota_shutdown },
    { "sqlite3_quota_set",           test_quota_set },
    { "sqlite3_quota_file",          test_quota_file },
    { "sqlite3_quota_dump",          test_quota_dump },
    { "sqlite3_quota_fopen",         test_quota_fopen },
    { "sqlite3_quota_fread",         test_quota_fread },
    { "sqlite3_quota_fwrite",        test_quota_fwrite },
    { "sqlite3_quota_fclose",        test_quota_fclose },
    { "sqlite3_quota_fflush",        test_quota_fflush },
    { "sqlite3_quota_fseek",         test_quota_fseek },
    { "sqlite3_quota_rewind",        test_quota_rewind },
    { "sqlite3_quota_ftell",         test_quota_ftell },
    { "sqlite3_quota_ftruncate",     test_quota_ftruncate },
    { "sqlite3_quota_file_size",     test_quota_file_size },
    { "sqlite3_quota_file_truesize", test_quota_file_truesize },
    { "sqlite3_quota_file_mtime",    test_quota_file_mtime },
    { "sqlite3_quota_remove",        test_quota_remove },
    { "sqlite3_quota_glob",          test_quota_glob },
    { "sqlite3_quota_file_available",test_quota_file_available },
    { "sqlite3_quota_ferror",        test_quota_ferror },
  };
  целое i;

  для(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  вернуть TCL_OK;
}
#endif
