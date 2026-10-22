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
#အကယ်၍ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
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
#မဟုတ်ပါက
# include <unistd.h>
#endif


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
typedef sa_nracanaa quotaGroup quotaGroup;
typedef sa_nracanaa quotaConn quotaConn;
typedef sa_nracanaa quotaFile quotaFile;

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
sa_nracanaa quotaGroup {
  const var_na *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 iSize;           /* Current size of all files */
  void (*xCallback)(             /* Callback invoked when going over quota */
     const var_na *zFilename,         /* Name of file whose size increases */
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
sa_nracanaa quotaFile {
  var_na *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 iSize;            /* Current size of this file */
  ကိန်းပြည့် nRef;                       /* Number of times this file is open */
  ကိန်းပြည့် deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *pNext, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
sa_nracanaa quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
sa_nracanaa quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ifdef _WIN32
  var_na *zMbcsName;        /* Full MBCS pathname of the file */
#endif
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
static sa_nracanaa {
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
  ကိန်းပြည့် isInitialized;

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
static ကိန်းပြည့် quotaGroupOpenFileCount(quotaGroup *pGroup){
  ကိန်းပြည့် N = 0;
  quotaFile *pFile = pGroup->pFiles;
  နေစဉ်( pFile ){
    အကယ်၍( pFile->nRef ) N++;
    pFile = pFile->pNext;
  }
  ပြန်ပေး N;
}

/* Remove a file from a quota group.
*/
static void quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->iSize -= pFile->iSize;
  *pFile->ppPrev = pFile->pNext;
  အကယ်၍( pFile->pNext ) pFile->pNext->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
static void quotaRemoveAllFiles(quotaGroup *pGroup){
  နေစဉ်( pGroup->pFiles ){
    assert( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
static void quotaGroupDeref(quotaGroup *pGroup){
  အကယ်၍( pGroup->iLimit==0 && quotaGroupOpenFileCount(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->ppPrev = pGroup->pNext;
    အကယ်၍( pGroup->pNext ) pGroup->pNext->ppPrev = pGroup->ppPrev;
    အကယ်၍( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
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
static ကိန်းပြည့် quotaStrglob(const var_na *zGlob, const var_na *z){
  ကိန်းပြည့် c, c2, cx;
  ကိန်းပြည့် invert;
  ကိန်းပြည့် seen;

  နေစဉ်( (c = (*(zGlob++)))!=0 ){
    အကယ်၍( c=='*' ){
      နေစဉ်( (c=(*(zGlob++))) == '*' || c=='?' ){
        အကယ်၍( c=='?' && (*(z++))==0 ) ပြန်ပေး 0;
      }
      အကယ်၍( c==0 ){
        ပြန်ပေး 1;
      }မဟုတ်ပါက အကယ်၍( c=='[' ){
        နေစဉ်( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        ပြန်ပေး (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      နေစဉ်( (c2 = (*(z++)))!=0 ){
        နေစဉ်( c2!=c && c2!=cx ){
          c2 = *(z++);
          အကယ်၍( c2==0 ) ပြန်ပေး 0;
        }
        အကယ်၍( quotaStrglob(zGlob,z) ) ပြန်ပေး 1;
      }
      ပြန်ပေး 0;
    }မဟုတ်ပါက အကယ်၍( c=='?' ){
      အကယ်၍( (*(z++))==0 ) ပြန်ပေး 0;
    }မဟုတ်ပါက အကယ်၍( c=='[' ){
      ကိန်းပြည့် prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      အကယ်၍( c==0 ) ပြန်ပေး 0;
      c2 = *(zGlob++);
      အကယ်၍( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      အကယ်၍( c2==']' ){
        အကယ်၍( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      နေစဉ်( c2 && c2!=']' ){
        အကယ်၍( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          အကယ်၍( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }မဟုတ်ပါက{
          အကယ်၍( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      အကယ်၍( c2==0 || (seen ^ invert)==0 ) ပြန်ပေး 0;
    }မဟုတ်ပါက အကယ်၍( c=='/' ){
      အကယ်၍( z[0]!='/' && z[0]!='\\' ) ပြန်ပေး 0;
      z++;
    }မဟုတ်ပါက{
      အကယ်၍( c!=(*(z++)) ) ပြန်ပေး 0;
    }
  }
  ပြန်ပေး *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
static quotaGroup *quotaGroupFind(const var_na *zFilename){
  quotaGroup *p;
  အတွက်(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->pNext){}
  ပြန်ပေး p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
static sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  ပြန်ပေး (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
static quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  const var_na *zName,      /* Full pathname of the file */
  ကိန်းပြည့် createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  နေစဉ်( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->pNext;
  }
  အကယ်၍( pFile==0 && createFlag ){
    ကိန်းပြည့် nName = (ကိန်းပြည့်)(strlen(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( parimaa_na(*pFile) + nName + 1 );
    အကယ်၍( pFile ){
      memset(pFile, 0, parimaa_na(*pFile));
      pFile->zFilename = (var_na*)&pFile[1];
      memcpy(pFile->zFilename, zName, nName+1);
      pFile->pNext = pGroup->pFiles;
      အကယ်၍( pGroup->pFiles ) pGroup->pFiles->ppPrev = &pFile->pNext;
      pFile->ppPrev = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  ပြန်ပေး pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
static var_na *quota_utf8_to_mbcs(const var_na *zUtf8){
#ifdef _WIN32
  size_t n;          /* Bytes in zUtf8 */
  ကိန်းပြည့် nWide;         /* number of UTF-16 characters */
  ကိန်းပြည့် nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  var_na *zMbcs;       /* The MBCS text */
  ကိန်းပြည့် codepage;      /* Code page used by fopen() */

  n = strlen(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, NULL, 0);
  အကယ်၍( nWide==0 ) ပြန်ပေး 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*parimaa_na(zTmpWide[0]) );
  အကယ်၍( zTmpWide==0 ) ပြန်ပေး 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (var_na*)sqlite3_malloc( nMbcs+1 ) : 0;
  အကယ်၍( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  ပြန်ပေး zMbcs;
#မဟုတ်ပါက
  ပြန်ပေး (var_na*)zUtf8;  /* No-op on unix */
#endif
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
static void quota_mbcs_free(var_na *zOld){
#ifdef _WIN32
  sqlite3_free(zOld);
#မဟုတ်ပါက
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
static ကိန်းပြည့် quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  const var_na *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  ကိန်းပြည့် flags,                  /* Flags to control the opening */
  ကိန်းပြည့် *pOutFlags              /* Flags showing results of opening */
){
  ကိန်းပြည့် rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  အကယ်၍( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    ပြန်ပေး pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  အကယ်၍( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }မဟုတ်ပါက{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    အကယ်၍( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      အကယ်၍( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        ပြန်ပေး SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      အကယ်၍( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }မဟုတ်ပါက{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  ပြန်ပေး rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
static ကိန်းပြည့် quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  const var_na *zName,          /* Name of file to be deleted */
  ကိန်းပြည့် syncDir                 /* Do a directory sync after deleting */
){
  ကိန်းပြည့် rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  အကယ်၍( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    အကယ်၍( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      အကယ်၍( pFile ){
        အကယ်၍( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }မဟုတ်ပါက{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  ပြန်ပေး rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
static ကိန်းပြည့် quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ကိန်းပြည့် rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  အကယ်၍( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    အကယ်၍( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  ပြန်ပေး rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
static ကိန်းပြည့် quotaRead(
  sqlite3_file *pConn,
  void *pBuf,
  ကိန်းပြည့် iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
static ကိန်းပြည့် quotaWrite(
  sqlite3_file *pConn,
  const void *pBuf,
  ကိန်းပြည့် iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  အကယ်၍( pFile->iSize<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    အကယ်၍( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      အကယ်၍( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      အကယ်၍( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        ပြန်ပေး SQLITE_FULL;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }
  ပြန်ပေး pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
static ကိန်းပြည့် quotaTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ကိန်းပြည့် rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  အကယ်၍( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = size;
    pGroup->iSize += size;
    quotaLeave();
  }
  ပြန်ပေး rc;
}

/* Pass xSync requests through to the original VFS without change
*/
static ကိန်းပြည့် quotaSync(sqlite3_file *pConn, ကိန်းပြည့် flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
static ကိန်းပြည့် quotaFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  ကိန်းပြည့် rc;

  rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz);
  အကယ်၍( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = sz;
    pGroup->iSize += sz;
    quotaLeave();
    *pSize = sz;
  }
  ပြန်ပေး rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaLock(sqlite3_file *pConn, ကိန်းပြည့် lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaUnlock(sqlite3_file *pConn, ကိန်းပြည့် lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaCheckReservedLock(sqlite3_file *pConn, ကိန်းပြည့် *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaFileControl(sqlite3_file *pConn, ကိန်းပြည့် op, void *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ကိန်းပြည့် rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#အကယ်၍ defined(SQLITE_FCNTL_VFSNAME)
  အကယ်၍( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(var_na**)pArg = sqlite3_mprintf("quota/%z", *(var_na**)pArg);
  }
#endif
  ပြန်ပေး rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaSectorSize(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xSectorSize(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ကိန်းပြည့် iRegion,                    /* Region to retrieve */
  ကိန်းပြည့် szRegion,                   /* Size of regions */
  ကိန်းပြည့် bExtend,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ကိန်းပြည့် ofst,                  /* First lock to acquire or release */
  ကိန်းပြည့် n,                     /* Number of locks to acquire or release */
  ကိန်းပြည့် flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
static void quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် quotaShmUnmap(sqlite3_file *pConn, ကိန်းပြည့် deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ပြန်ပေး pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
ကိန်းပြည့် sqlite3_quota_initialize(const var_na *zOrigVfsName, ကိန်းပြည့် makeDefault){
  sqlite3_vfs *pOrigVfs;
  အကယ်၍( gQuota.isInitialized ) ပြန်ပေး SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  အကယ်၍( pOrigVfs==0 ) ပြန်ပေး SQLITE_ERROR;
  assert( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  အကယ်၍( !gQuota.pMutex ){
    ပြန်ပေး SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += parimaa_na(quotaConn);
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
  ပြန်ပေး SQLITE_OK;
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
ကိန်းပြည့် sqlite3_quota_shutdown(void){
  quotaGroup *pGroup;
  အကယ်၍( gQuota.isInitialized==0 ) ပြန်ပေး SQLITE_MISUSE;
  အတွက်(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    အကယ်၍( quotaGroupOpenFileCount(pGroup)>0 ) ပြန်ပေး SQLITE_MISUSE;
  }
  နေစဉ်( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->pNext;
    pGroup->iLimit = 0;
    assert( quotaGroupOpenFileCount(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, parimaa_na(gQuota));
  ပြန်ပေး SQLITE_OK;
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
ကိန်းပြည့် sqlite3_quota_set(
  const var_na *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  void (*xCallback)(              /* Callback invoked when going over quota */
     const var_na *zFilename,         /* Name of file whose size increases */
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
  နေစဉ်( pGroup && strcmp(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->pNext;
  }
  အကယ်၍( pGroup==0 ){
    ကိန်းပြည့် nPattern = (ကိန်းပြည့်)(strlen(zPattern) & 0x3fffffff);
    အကယ်၍( iLimit<=0 ){
      quotaLeave();
      ပြန်ပေး SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( parimaa_na(*pGroup) + nPattern + 1 );
    အကယ်၍( pGroup==0 ){
      quotaLeave();
      ပြန်ပေး SQLITE_NOMEM;
    }
    memset(pGroup, 0, parimaa_na(*pGroup));
    pGroup->zPattern = (var_na*)&pGroup[1];
    memcpy((var_na *)pGroup->zPattern, zPattern, nPattern+1);
    အကယ်၍( gQuota.pGroup ) gQuota.pGroup->ppPrev = &pGroup->pNext;
    pGroup->pNext = gQuota.pGroup;
    pGroup->ppPrev = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  အကယ်၍( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  ပြန်ပေး SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
ကိန်းပြည့် sqlite3_quota_file(const var_na *zFilename){
  var_na *zFull = 0;
  sqlite3_file *fd;
  ကိန်းပြည့် rc;
  ကိန်းပြည့် outFlags = 0;
  sqlite3_int64 iSize;
  ကိန်းပြည့် nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  အကယ်၍( fd==0 ){
    rc = SQLITE_NOMEM;
  }မဟုတ်ပါက{
    zFull = &((var_na *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  အကယ်၍( rc==SQLITE_OK ){
    zFull[strlen(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    အကယ်၍( rc==SQLITE_OK ){
      fd->pMethods->xFileSize(fd, &iSize);
      fd->pMethods->xClose(fd);
    }မဟုတ်ပါက အကယ်၍( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      အကယ်၍( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        အကယ်၍( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  ပြန်ပေး rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(const var_na *zFilename, const var_na *zMode){
  quota_FILE *p = 0;
  var_na *zFull = 0;
  var_na *zFullTranslated = 0;
  ကိန်းပြည့် rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (var_na*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  အကယ်၍( zFull==0 ) ပြန်ပေး 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  အကယ်၍( rc ) goto quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(parimaa_na(*p));
  အကယ်၍( p==0 ) goto quota_fopen_error;
  memset(p, 0, parimaa_na(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  အကယ်၍( zFullTranslated==0 ) goto quota_fopen_error;
  p->f = fopen(zFullTranslated, zMode);
  အကယ်၍( p->f==0 ) goto quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  အကယ်၍( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    အကယ်၍( pFile==0 ){
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
  ပြန်ပေး p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  အကယ်၍( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  ပြန်ပေး 0;
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
  ပြန်ပေး fread(pBuf, size, nmemb, p->f);
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
  အကယ်၍( pFile && pFile->iSize<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    အကယ်၍( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      အကယ်၍( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      အကယ်၍( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->iSize + pFile->iSize;
        nmemb = (size_t)((iEnd - iOfst)/size);
        iEnd = iOfst + size*nmemb;
        szNew = pGroup->iSize - pFile->iSize + iEnd;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }မဟုတ်ပါက{
    pFile = 0;
  }
  rc = fwrite(pBuf, size, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  အကယ်၍( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + size*nWritten;
    အကယ်၍( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->iSize += iNewEnd - pFile->iSize;
    pFile->iSize = iNewEnd;
    quotaLeave();
  }
  ပြန်ပေး rc;
}

/*
** Close an open quota_FILE stream.
*/
ကိန်းပြည့် sqlite3_quota_fclose(quota_FILE *p){
  ကိန်းပြည့် rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  အကယ်၍( pFile ){
    quotaEnter();
    pFile->nRef--;
    အကယ်၍( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      အကယ်၍( pFile->deleteOnClose ){
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
  ပြန်ပေး rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
ကိန်းပြည့် sqlite3_quota_fflush(quota_FILE *p, ကိန်းပြည့် doFsync){
  ကိန်းပြည့် rc;
  rc = fflush(p->f);
  အကယ်၍( rc==0 && doFsync ){
#ifdef _WIN32
    rc = _commit(_fileno(p->f));
#မဟုတ်ပါက
    rc = fsync(fileno(p->f));
#endif
  }
  ပြန်ပေး rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
ကိန်းပြည့် sqlite3_quota_fseek(quota_FILE *p, long offset, ကိန်းပြည့် whence){
  ပြန်ပေး fseek(p->f, offset, whence);
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
  ပြန်ပေး ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
ကိန်းပြည့် sqlite3_quota_ferror(quota_FILE *p){
  ပြန်ပေး ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
ကိန်းပြည့် sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  ကိန်းပြည့် rc;
  အကယ်၍( (pFile = p->pFile)!=0 && pFile->iSize<szNew ){
    quotaGroup *pGroup;
    အကယ်၍( pFile->iSize<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      ပြန်ပေး -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    quotaLeave();
  }
#ifdef _WIN32
#  အကယ်၍ defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (long)szNew);
#  မဟုတ်ပါက
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#မဟုတ်ပါက
  rc = ftruncate(fileno(p->f), szNew);
#endif
  အကယ်၍( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    pFile->iSize = szNew;
    quotaLeave();
  }
  ပြန်ပေး rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
ကိန်းပြည့် sqlite3_quota_file_mtime(quota_FILE *p, time_t *pTime){
  ကိန်းပြည့် rc;
#ifdef _WIN32
  sa_nracanaa _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#မဟုတ်ပါက
  sa_nracanaa stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  အကယ်၍( rc==0 ) *pTime = buf.st_mtime;
  ပြန်ပေး rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  ကိန်းပြည့် rc;
#ifdef _WIN32
  sa_nracanaa _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#မဟုတ်ပါက
  sa_nracanaa stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  ပြန်ပေး rc==0 ? buf.st_size : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_size(quota_FILE *p){
  ပြန်ပေး p->pFile ? p->pFile->iSize : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
long sqlite3_quota_file_available(quota_FILE *p){
  FILE* f = p->f;
  long pos1, pos2;
  ကိန်းပြည့် rc;
  pos1 = ftell(f);
  အကယ်၍ ( pos1 < 0 ) ပြန်ပေး -1;
  rc = fseek(f, 0, SEEK_END);
  အကယ်၍ ( rc != 0 ) ပြန်ပေး -1;
  pos2 = ftell(f);
  အကယ်၍ ( pos2 < 0 ) ပြန်ပေး -1;
  rc = fseek(f, pos1, SEEK_SET);
  အကယ်၍ ( rc != 0 ) ပြန်ပေး -1;
  ပြန်ပေး pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
ကိန်းပြည့် sqlite3_quota_remove(const var_na *zFilename){
  var_na *zFull;            /* Full pathname for zFilename */
  size_t nFull;           /* Number of bytes in zFilename */
  ကိန်းပြည့် rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *pNextFile;   /* next file in the group */
  ကိန်းပြည့် diff;               /* Difference between filenames */
  var_na c;                 /* First character past end of pattern */

  zFull = (var_na*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  အကယ်၍( zFull==0 ) ပြန်ပေး SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  အကယ်၍( rc ){
    sqlite3_free(zFull);
    ပြန်ပေး rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = strlen(zFull);
  အကယ်၍( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  အကယ်၍( pGroup ){
    အတွက်(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=pNextFile){
      pNextFile = pFile->pNext;
      diff = strncmp(zFull, pFile->zFilename, nFull);
      အကယ်၍( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        အကယ်၍( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }မဟုတ်ပါက{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(zFull);
  ပြန်ပေး rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#include "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
typedef sa_nracanaa TclQuotaCallback TclQuotaCallback;
sa_nracanaa TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

extern const var_na *sqlite3ErrName(ကိန်းပြည့်);


/*
** This is the callback from a quota-over-limit.
*/
static void tclQuotaCallback(
  const var_na *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 iSize,            /* Total size of all files in the group */
  void *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  unsigned ကိန်းပြည့် rnd;               /* Random part of pVarname */
  ကိန်းပြည့် rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  အကယ်၍( p==0 ) ပြန်ပေး;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_IncrRefCount(pVarname);
  sqlite3_randomness(parimaa_na(rnd), (void *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((ကိန်းပြည့်)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_IncrRefCount(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(iSize));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  အကယ်၍( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_DecrRefCount(pEval);
  Tcl_DecrRefCount(pVarname);
  အကယ်၍( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
static void tclCallbackDestructor(void *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  အကယ်၍( p ){
    Tcl_DecrRefCount(p->pScript);
    sqlite3_free((var_na *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_initialize(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zName;              /* Name of new quota VFS */
  ကိန်းပြည့် makeDefault;                /* True to make the new VFS the default */
  ကိန်းပြည့် rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  အကယ်၍( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME MAKEDEFAULT");
    ပြန်ပေး TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  အကယ်၍( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ပြန်ပေး TCL_ERROR;
  အကယ်၍( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);

  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  ကိန်းပြည့် rc;                         /* Value returned by quota_shutdown() */

  အကယ်၍( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ပြန်ပေး TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);

  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_set(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  ကိန်းပြည့် rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_Size nScript;               /* Length of callback script */
  void (*xDestroy)(void*);        /* Optional destructor for pArg */
  void (*xCallback)(const var_na *, sqlite3_int64 *, sqlite3_int64, void *);

  /* Process arguments */
  အကယ်၍( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    ပြန်ပေး TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  အကယ်၍( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) ပြန်ပေး TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  အကယ်၍( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(parimaa_na(TclQuotaCallback));
    အကယ်၍( !p ){
      Tcl_SetResult(interp, (var_na *)"SQLITE_NOMEM", TCL_STATIC);
      ပြန်ပေး TCL_OK;
    }
    memset(p, 0, parimaa_na(TclQuotaCallback));
    p->interp = interp;
    Tcl_IncrRefCount(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }မဟုတ်ပါက{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (void*)p, xDestroy);

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_file(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zFilename;          /* File pattern to configure */
  ကိန်းပြည့် rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    ပြန်ပေး TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_dump(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pResult;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  pResult = Tcl_NewObj();
  quotaEnter();
  အတွက်(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iSize));
    အတွက်(pFile=pGroup->pFiles; pFile; pFile=pFile->pNext){
      ကိန်းပြည့် i;
      var_na zTemp[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(parimaa_na(zTemp), zTemp, "%s", pFile->zFilename);
      အတွက်(i=0; zTemp[i]; i++){ အကယ်၍( zTemp[i]=='\\' ) zTemp[i] = '/'; }
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
  ပြန်ပေး TCL_OK;
}

/* Defined in test1.c */
extern void *sqlite3TestTextToPtr(const var_na*);
extern const var_na *sqlite3TestPtrToText(void*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_fopen(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zFilename;          /* File pattern to configure */
  const var_na *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  အကယ်၍( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME MODE");
    ပြန်ပေး TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_SetResult(interp, (var_na*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_fread(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  var_na *zBuf;
  ကိန်းပြည့် sz;
  ကိန်းပြည့် nElem;
  size_t got;

  အကယ်၍( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE SIZE NELEM");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  အကယ်၍( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ပြန်ပေး TCL_ERROR;
  အကယ်၍( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ပြန်ပေး TCL_ERROR;
  zBuf = (var_na*)sqlite3_malloc( sz*nElem + 1 );
  အကယ်၍( zBuf==0 ){
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
    ပြန်ပေး TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_SetResult(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_fwrite(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  var_na *zBuf;
  ကိန်းပြည့် sz;
  ကိန်းပြည့် nElem;
  size_t got;

  အကယ်၍( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  အကယ်၍( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ပြန်ပေး TCL_ERROR;
  အကယ်၍( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ပြန်ပေး TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(got));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_fclose(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ကိန်းပြည့် rc;

  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_fflush(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ကိန်းပြည့် rc;
  ကိန်းပြည့် doSync = 0;

  အကယ်၍( objc!=2 && objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE ?HARDSYNC?");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  အကယ်၍( objc==3 ){
    အကယ်၍( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) ပြန်ပေး TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_fseek(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ကိန်းပြည့် ofst;
  const var_na *zWhence;
  ကိန်းပြည့် whence;
  ကိန်းပြည့် rc;

  အကယ်၍( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE OFFSET WHENCE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  အကယ်၍( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) ပြန်ပေး TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  အကယ်၍( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }မဟုတ်ပါက အကယ်၍( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }မဟုတ်ပါက အကယ်၍( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }မဟုတ်ပါက{
    Tcl_AppendResult(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (var_na*)0);
    ပြန်ပေး TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_rewind(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_ftell(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_ftruncate(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  ကိန်းပြည့် rc;
  အကယ်၍( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE SIZE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  အကယ်၍( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) ပြန်ပေး TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_file_size(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_size(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_file_truesize(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_file_mtime(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  time_t t;
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(t));
  ပြန်ပေး TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_remove(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zFilename;          /* File pattern to configure */
  ကိန်းပြည့် rc;
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    ပြန်ပေး TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_glob(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zPattern;          /* The glob pattern */
  const var_na *zText;             /* Text to compare against the pattern */
  ကိန်းပြည့် rc;
  အကယ်၍( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PATTERN TEXT");
    ပြန်ပေး TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_file_available(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_quota_ferror(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ကိန်းပြည့် x;
  အကယ်၍( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE");
    ပြန်ပေး TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(x));
  ပြန်ပေး TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ကိန်းပြည့် Sqlitequota_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
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
  ကိန်းပြည့် i;

  အတွက်(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ပြန်ပေး TCL_OK;
}
#endif
