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
#અન્તર્ભાવ "test_quota.h"
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#જો defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)
#define sqlite3_mutex_held(X)     ((શૂન્ય)(X),1)
#define sqlite3_mutex_notheld(X)  ((શૂન્ય)(X),1)
#endif /* SQLITE_THREADSAFE==0 */


#ifdef _WIN32
# include <windows.h>
# include <io.h>
#નહીંતર
# include <unistd.h>
#endif


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
પ્રકારવ્યાખ્યા સંરચના quotaGroup quotaGroup;
પ્રકારવ્યાખ્યા સંરચના quotaConn quotaConn;
પ્રકારવ્યાખ્યા સંરચના quotaFile quotaFile;

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
સંરચના quotaGroup {
  અચલ અક્ષર *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 iSize;           /* Current size of all files */
  શૂન્ય (*xCallback)(             /* Callback invoked when going over quota */
     અચલ અક્ષર *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     શૂન્ય *pArg                     /* Client data */
  );
  શૂન્ય *pArg;                    /* Third argument to the xCallback() */
  શૂન્ય (*xDestroy)(શૂન્ય*);       /* Optional destructor for pArg */
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
સંરચના quotaFile {
  અક્ષર *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 iSize;            /* Current size of this file */
  પૂર્ણાંક nRef;                       /* Number of times this file is open */
  પૂર્ણાંક deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *pNext, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
સંરચના quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
સંરચના quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ifdef _WIN32
  અક્ષર *zMbcsName;        /* Full MBCS pathname of the file */
#endif
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
સ્થિર સંરચના {
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
  પૂર્ણાંક isInitialized;

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
સ્થિર શૂન્ય quotaEnter(શૂન્ય){ sqlite3_mutex_enter(gQuota.pMutex); }
સ્થિર શૂન્ય quotaLeave(શૂન્ય){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
સ્થિર પૂર્ણાંક quota_Group_Open_File_ગણતરી(quotaGroup *pGroup){
  પૂર્ણાંક N = 0;
  quotaFile *pFile = pGroup->pFiles;
  જ્યારે( pFile ){
    જો( pFile->nRef ) N++;
    pFile = pFile->pNext;
  }
  પાછા N;
}

/* Remove a file from a quota group.
*/
સ્થિર શૂન્ય quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->iSize -= pFile->iSize;
  *pFile->ppPrev = pFile->pNext;
  જો( pFile->pNext ) pFile->pNext->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
સ્થિર શૂન્ય quotaRemoveAllFiles(quotaGroup *pGroup){
  જ્યારે( pGroup->pFiles ){
    પ્રતિજ્ઞા( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
સ્થિર શૂન્ય quotaGroupDeref(quotaGroup *pGroup){
  જો( pGroup->iLimit==0 && quota_Group_Open_File_ગણતરી(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->ppPrev = pGroup->pNext;
    જો( pGroup->pNext ) pGroup->pNext->ppPrev = pGroup->ppPrev;
    જો( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
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
સ્થિર પૂર્ણાંક quotaStrglob(અચલ અક્ષર *zGlob, અચલ અક્ષર *z){
  પૂર્ણાંક c, c2, cx;
  પૂર્ણાંક invert;
  પૂર્ણાંક seen;

  જ્યારે( (c = (*(zGlob++)))!=0 ){
    જો( c=='*' ){
      જ્યારે( (c=(*(zGlob++))) == '*' || c=='?' ){
        જો( c=='?' && (*(z++))==0 ) પાછા 0;
      }
      જો( c==0 ){
        પાછા 1;
      }નહીંતર જો( c=='[' ){
        જ્યારે( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        પાછા (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      જ્યારે( (c2 = (*(z++)))!=0 ){
        જ્યારે( c2!=c && c2!=cx ){
          c2 = *(z++);
          જો( c2==0 ) પાછા 0;
        }
        જો( quotaStrglob(zGlob,z) ) પાછા 1;
      }
      પાછા 0;
    }નહીંતર જો( c=='?' ){
      જો( (*(z++))==0 ) પાછા 0;
    }નહીંતર જો( c=='[' ){
      પૂર્ણાંક prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      જો( c==0 ) પાછા 0;
      c2 = *(zGlob++);
      જો( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      જો( c2==']' ){
        જો( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      જ્યારે( c2 && c2!=']' ){
        જો( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          જો( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }નહીંતર{
          જો( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      જો( c2==0 || (seen ^ invert)==0 ) પાછા 0;
    }નહીંતર જો( c=='/' ){
      જો( z[0]!='/' && z[0]!='\\' ) પાછા 0;
      z++;
    }નહીંતર{
      જો( c!=(*(z++)) ) પાછા 0;
    }
  }
  પાછા *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
સ્થિર quotaGroup *quotaGroupFind(અચલ અક્ષર *zFilename){
  quotaGroup *p;
  માટે(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->pNext){}
  પાછા p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
સ્થિર sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  પાછા (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
સ્થિર quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  અચલ અક્ષર *zName,      /* Full pathname of the file */
  પૂર્ણાંક createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  જ્યારે( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->pNext;
  }
  જો( pFile==0 && createFlag ){
    પૂર્ણાંક nName = (પૂર્ણાંક)(strlen(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( કદ(*pFile) + nName + 1 );
    જો( pFile ){
      memset(pFile, 0, કદ(*pFile));
      pFile->zFilename = (અક્ષર*)&pFile[1];
      memcpy(pFile->zFilename, zName, nName+1);
      pFile->pNext = pGroup->pFiles;
      જો( pGroup->pFiles ) pGroup->pFiles->ppPrev = &pFile->pNext;
      pFile->ppPrev = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  પાછા pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
સ્થિર અક્ષર *quota_utf8_to_mbcs(અચલ અક્ષર *zUtf8){
#ifdef _WIN32
  size_t n;          /* Bytes in zUtf8 */
  પૂર્ણાંક nWide;         /* number of UTF-16 characters */
  પૂર્ણાંક nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  અક્ષર *zMbcs;       /* The MBCS text */
  પૂર્ણાંક codepage;      /* Code page used by fopen() */

  n = strlen(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, ખાલી, 0);
  જો( nWide==0 ) પાછા 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*કદ(zTmpWide[0]) );
  જો( zTmpWide==0 ) પાછા 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (અક્ષર*)sqlite3_malloc( nMbcs+1 ) : 0;
  જો( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  પાછા zMbcs;
#નહીંતર
  પાછા (અક્ષર*)zUtf8;  /* No-op on unix */
#endif
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
સ્થિર શૂન્ય quota_mbcs_free(અક્ષર *zOld){
#ifdef _WIN32
  sqlite3_free(zOld);
#નહીંતર
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
સ્થિર પૂર્ણાંક quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  અચલ અક્ષર *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  પૂર્ણાંક flags,                  /* Flags to control the opening */
  પૂર્ણાંક *pOutFlags              /* Flags showing results of opening */
){
  પૂર્ણાંક rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  જો( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    પાછા pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  જો( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }નહીંતર{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    જો( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      જો( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        પાછા SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      જો( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }નહીંતર{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  પાછા rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
સ્થિર પૂર્ણાંક quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  અચલ અક્ષર *zName,          /* Name of file to be deleted */
  પૂર્ણાંક syncDir                 /* Do a directory sync after deleting */
){
  પૂર્ણાંક rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  જો( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    જો( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      જો( pFile ){
        જો( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }નહીંતર{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  પાછા rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
સ્થિર પૂર્ણાંક quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પૂર્ણાંક rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  જો( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    જો( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  પાછા rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
સ્થિર પૂર્ણાંક quotaRead(
  sqlite3_file *pConn,
  શૂન્ય *pBuf,
  પૂર્ણાંક iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
સ્થિર પૂર્ણાંક quotaWrite(
  sqlite3_file *pConn,
  અચલ શૂન્ય *pBuf,
  પૂર્ણાંક iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  જો( pFile->iSize<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    જો( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      જો( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      જો( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        પાછા SQLITE_FULL;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }
  પાછા pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
સ્થિર પૂર્ણાંક quotaTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પૂર્ણાંક rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  જો( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = size;
    pGroup->iSize += size;
    quotaLeave();
  }
  પાછા rc;
}

/* Pass xSync requests through to the original VFS without change
*/
સ્થિર પૂર્ણાંક quotaSync(sqlite3_file *pConn, પૂર્ણાંક flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
સ્થિર પૂર્ણાંક quotaFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  પૂર્ણાંક rc;

  rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz);
  જો( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = sz;
    pGroup->iSize += sz;
    quotaLeave();
    *pSize = sz;
  }
  પાછા rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaLock(sqlite3_file *pConn, પૂર્ણાંક lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaUnlock(sqlite3_file *pConn, પૂર્ણાંક lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaCheckReservedLock(sqlite3_file *pConn, પૂર્ણાંક *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaFileControl(sqlite3_file *pConn, પૂર્ણાંક op, શૂન્ય *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પૂર્ણાંક rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#જો defined(SQLITE_FCNTL_VFSNAME)
  જો( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(અક્ષર**)pArg = sqlite3_mprintf("quota/%z", *(અક્ષર**)pArg);
  }
#endif
  પાછા rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaSectorSize(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xSectorSize(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  પૂર્ણાંક iRegion,                    /* Region to retrieve */
  પૂર્ણાંક szRegion,                   /* Size of regions */
  પૂર્ણાંક bExtend,                    /* True to extend file if necessary */
  શૂન્ય અસ્થિર **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  પૂર્ણાંક ofst,                  /* First lock to acquire or release */
  પૂર્ણાંક n,                     /* Number of locks to acquire or release */
  પૂર્ણાંક flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
સ્થિર શૂન્ય quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
સ્થિર પૂર્ણાંક quotaShmUnmap(sqlite3_file *pConn, પૂર્ણાંક deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  પાછા pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
પૂર્ણાંક sqlite3_quota_initialize(અચલ અક્ષર *zOrigVfsName, પૂર્ણાંક makeDefault){
  sqlite3_vfs *pOrigVfs;
  જો( gQuota.isInitialized ) પાછા SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  જો( pOrigVfs==0 ) પાછા SQLITE_ERROR;
  પ્રતિજ્ઞા( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  જો( !gQuota.pMutex ){
    પાછા SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += કદ(quotaConn);
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
  પાછા SQLITE_OK;
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
પૂર્ણાંક sqlite3_quota_shutdown(શૂન્ય){
  quotaGroup *pGroup;
  જો( gQuota.isInitialized==0 ) પાછા SQLITE_MISUSE;
  માટે(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    જો( quota_Group_Open_File_ગણતરી(pGroup)>0 ) પાછા SQLITE_MISUSE;
  }
  જ્યારે( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->pNext;
    pGroup->iLimit = 0;
    પ્રતિજ્ઞા( quota_Group_Open_File_ગણતરી(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, કદ(gQuota));
  પાછા SQLITE_OK;
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
પૂર્ણાંક sqlite3_quota_set(
  અચલ અક્ષર *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  શૂન્ય (*xCallback)(              /* Callback invoked when going over quota */
     અચલ અક્ષર *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     શૂન્ય *pArg                     /* Client data */
  ),
  શૂન્ય *pArg,                     /* client data passed thru to callback */
  શૂન્ય (*xDestroy)(શૂન્ય*)         /* Optional destructor for pArg */
){
  quotaGroup *pGroup;
  quotaEnter();
  pGroup = gQuota.pGroup;
  જ્યારે( pGroup && strcmp(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->pNext;
  }
  જો( pGroup==0 ){
    પૂર્ણાંક nPattern = (પૂર્ણાંક)(strlen(zPattern) & 0x3fffffff);
    જો( iLimit<=0 ){
      quotaLeave();
      પાછા SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( કદ(*pGroup) + nPattern + 1 );
    જો( pGroup==0 ){
      quotaLeave();
      પાછા SQLITE_NOMEM;
    }
    memset(pGroup, 0, કદ(*pGroup));
    pGroup->zPattern = (અક્ષર*)&pGroup[1];
    memcpy((અક્ષર *)pGroup->zPattern, zPattern, nPattern+1);
    જો( gQuota.pGroup ) gQuota.pGroup->ppPrev = &pGroup->pNext;
    pGroup->pNext = gQuota.pGroup;
    pGroup->ppPrev = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  જો( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  પાછા SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
પૂર્ણાંક sqlite3_quota_file(અચલ અક્ષર *zFilename){
  અક્ષર *zFull = 0;
  sqlite3_file *fd;
  પૂર્ણાંક rc;
  પૂર્ણાંક outFlags = 0;
  sqlite3_int64 iSize;
  પૂર્ણાંક nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  જો( fd==0 ){
    rc = SQLITE_NOMEM;
  }નહીંતર{
    zFull = &((અક્ષર *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  જો( rc==SQLITE_OK ){
    zFull[strlen(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    જો( rc==SQLITE_OK ){
      fd->pMethods->xFileSize(fd, &iSize);
      fd->pMethods->xClose(fd);
    }નહીંતર જો( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      જો( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        જો( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  પાછા rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(અચલ અક્ષર *zFilename, અચલ અક્ષર *zMode){
  quota_FILE *p = 0;
  અક્ષર *zFull = 0;
  અક્ષર *zFullTranslated = 0;
  પૂર્ણાંક rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (અક્ષર*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  જો( zFull==0 ) પાછા 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  જો( rc ) જાઓ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(કદ(*p));
  જો( p==0 ) જાઓ quota_fopen_error;
  memset(p, 0, કદ(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  જો( zFullTranslated==0 ) જાઓ quota_fopen_error;
  p->f = fopen(zFullTranslated, zMode);
  જો( p->f==0 ) જાઓ quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  જો( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    જો( pFile==0 ){
      quotaLeave();
      જાઓ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(zFull);
#ifdef _WIN32
  p->zMbcsName = zFullTranslated;
#endif
  પાછા p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  જો( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  પાછા 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  શૂન્ય *pBuf,            /* Store the content here */
  size_t size,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  પાછા fread(pBuf, size, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  અચલ શૂન્ય *pBuf,      /* Take content to write from here */
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
  જો( pFile && pFile->iSize<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    જો( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      જો( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      જો( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->iSize + pFile->iSize;
        nmemb = (size_t)((iEnd - iOfst)/size);
        iEnd = iOfst + size*nmemb;
        szNew = pGroup->iSize - pFile->iSize + iEnd;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }નહીંતર{
    pFile = 0;
  }
  rc = fwrite(pBuf, size, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  જો( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + size*nWritten;
    જો( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->iSize += iNewEnd - pFile->iSize;
    pFile->iSize = iNewEnd;
    quotaLeave();
  }
  પાછા rc;
}

/*
** Close an open quota_FILE stream.
*/
પૂર્ણાંક sqlite3_quota_fclose(quota_FILE *p){
  પૂર્ણાંક rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  જો( pFile ){
    quotaEnter();
    pFile->nRef--;
    જો( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      જો( pFile->deleteOnClose ){
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
  પાછા rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
પૂર્ણાંક sqlite3_quota_fflush(quota_FILE *p, પૂર્ણાંક doFsync){
  પૂર્ણાંક rc;
  rc = fflush(p->f);
  જો( rc==0 && doFsync ){
#ifdef _WIN32
    rc = _commit(_fileno(p->f));
#નહીંતર
    rc = fsync(fileno(p->f));
#endif
  }
  પાછા rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
પૂર્ણાંક sqlite3_quota_fseek(quota_FILE *p, long offset, પૂર્ણાંક whence){
  પાછા fseek(p->f, offset, whence);
}

/*
** rewind a quota_FILE stream.
*/
શૂન્ય sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
long sqlite3_quota_ftell(quota_FILE *p){
  પાછા ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
પૂર્ણાંક sqlite3_quota_ferror(quota_FILE *p){
  પાછા ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
પૂર્ણાંક sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  પૂર્ણાંક rc;
  જો( (pFile = p->pFile)!=0 && pFile->iSize<szNew ){
    quotaGroup *pGroup;
    જો( pFile->iSize<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      પાછા -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    quotaLeave();
  }
#ifdef _WIN32
#  જો defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (long)szNew);
#  નહીંતર
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#નહીંતર
  rc = ftruncate(fileno(p->f), szNew);
#endif
  જો( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    pFile->iSize = szNew;
    quotaLeave();
  }
  પાછા rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
પૂર્ણાંક sqlite3_quota_file_mtime(quota_FILE *p, time_t *pTime){
  પૂર્ણાંક rc;
#ifdef _WIN32
  સંરચના _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#નહીંતર
  સંરચના stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  જો( rc==0 ) *pTime = buf.st_mtime;
  પાછા rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  પૂર્ણાંક rc;
#ifdef _WIN32
  સંરચના _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#નહીંતર
  સંરચના stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  પાછા rc==0 ? buf.st_size : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_size(quota_FILE *p){
  પાછા p->pFile ? p->pFile->iSize : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
long sqlite3_quota_file_available(quota_FILE *p){
  FILE* f = p->f;
  long pos1, pos2;
  પૂર્ણાંક rc;
  pos1 = ftell(f);
  જો ( pos1 < 0 ) પાછા -1;
  rc = fseek(f, 0, SEEK_END);
  જો ( rc != 0 ) પાછા -1;
  pos2 = ftell(f);
  જો ( pos2 < 0 ) પાછા -1;
  rc = fseek(f, pos1, SEEK_SET);
  જો ( rc != 0 ) પાછા -1;
  પાછા pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
પૂર્ણાંક sqlite3_quota_remove(અચલ અક્ષર *zFilename){
  અક્ષર *zFull;            /* Full pathname for zFilename */
  size_t nFull;           /* Number of bytes in zFilename */
  પૂર્ણાંક rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *pNextFile;   /* next file in the group */
  પૂર્ણાંક diff;               /* Difference between filenames */
  અક્ષર c;                 /* First character past end of pattern */

  zFull = (અક્ષર*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  જો( zFull==0 ) પાછા SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  જો( rc ){
    sqlite3_free(zFull);
    પાછા rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = strlen(zFull);
  જો( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  જો( pGroup ){
    માટે(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=pNextFile){
      pNextFile = pFile->pNext;
      diff = strncmp(zFull, pFile->zFilename, nFull);
      જો( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        જો( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }નહીંતર{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(zFull);
  પાછા rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#અન્તર્ભાવ "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
પ્રકારવ્યાખ્યા સંરચના TclQuotaCallback TclQuotaCallback;
સંરચના TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);


/*
** This is the callback from a quota-over-limit.
*/
સ્થિર શૂન્ય tclQuotaCallback(
  અચલ અક્ષર *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 iSize,            /* Total size of all files in the group */
  શૂન્ય *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  unsigned પૂર્ણાંક rnd;               /* Random part of pVarname */
  પૂર્ણાંક rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  જો( p==0 ) પાછા;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_ગણતરી(pVarname);
  sqlite3_randomness(કદ(rnd), (શૂન્ય *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((પૂર્ણાંક)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ગણતરી(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(iSize));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  જો( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_ગણતરી(pEval);
  Tcl_Decr_Ref_ગણતરી(pVarname);
  જો( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
સ્થિર શૂન્ય tclCallbackDestructor(શૂન્ય *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  જો( p ){
    Tcl_Decr_Ref_ગણતરી(p->pScript);
    sqlite3_free((અક્ષર *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_initialize(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *zName;              /* Name of new quota VFS */
  પૂર્ણાંક makeDefault;                /* True to make the new VFS the default */
  પૂર્ણાંક rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    પાછા TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  જો( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) પાછા TCL_ERROR;
  જો( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_STATIC);

  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_shutdown(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc;                         /* Value returned by quota_shutdown() */

  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_STATIC);

  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_set(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  પૂર્ણાંક rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_Size nScript;               /* Length of callback script */
  શૂન્ય (*xDestroy)(શૂન્ય*);        /* Optional destructor for pArg */
  શૂન્ય (*xCallback)(અચલ અક્ષર *, sqlite3_int64 *, sqlite3_int64, શૂન્ય *);

  /* Process arguments */
  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    પાછા TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  જો( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) પાછા TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  જો( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(કદ(TclQuotaCallback));
    જો( !p ){
      Tcl_SetResult(interp, (અક્ષર *)"SQLITE_NOMEM", TCL_STATIC);
      પાછા TCL_OK;
    }
    memset(p, 0, કદ(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_ગણતરી(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }નહીંતર{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (શૂન્ય*)p, xDestroy);

  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_STATIC);
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_file(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *zFilename;          /* File pattern to configure */
  પૂર્ણાંક rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "FILENAME");
    પાછા TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_STATIC);
  પાછા TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_dump(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pResult;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  pResult = Tcl_NewObj();
  quotaEnter();
  માટે(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iSize));
    માટે(pFile=pGroup->pFiles; pFile; pFile=pFile->pNext){
      પૂર્ણાંક i;
      અક્ષર zTemp[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(કદ(zTemp), zTemp, "%s", pFile->zFilename);
      માટે(i=0; zTemp[i]; i++){ જો( zTemp[i]=='\\' ) zTemp[i] = '/'; }
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
  પાછા TCL_OK;
}

/* Defined in test1.c */
બાહ્ય શૂન્ય *sqlite3TestTextToPtr(અચલ અક્ષર*);
બાહ્ય અચલ અક્ષર *sqlite3TestPtrToText(શૂન્ય*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_fopen(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *zFilename;          /* File pattern to configure */
  અચલ અક્ષર *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "FILENAME MODE");
    પાછા TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_SetResult(interp, (અક્ષર*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_fread(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  અક્ષર *zBuf;
  પૂર્ણાંક sz;
  પૂર્ણાંક nElem;
  size_t got;

  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE SIZE NELEM");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  જો( Tcl_GetIntFromObj(interp, objv[2], &sz) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) પાછા TCL_ERROR;
  zBuf = (અક્ષર*)sqlite3_malloc( sz*nElem + 1 );
  જો( zBuf==0 ){
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
    પાછા TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_SetResult(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_fwrite(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  અક્ષર *zBuf;
  પૂર્ણાંક sz;
  પૂર્ણાંક nElem;
  size_t got;

  જો( objc!=5 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  જો( Tcl_GetIntFromObj(interp, objv[2], &sz) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) પાછા TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(got));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_fclose(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  પૂર્ણાંક rc;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_fflush(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  પૂર્ણાંક rc;
  પૂર્ણાંક doSync = 0;

  જો( objc!=2 && objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE ?HARDSYNC?");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  જો( objc==3 ){
    જો( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) પાછા TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_fseek(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  પૂર્ણાંક ofst;
  અચલ અક્ષર *zWhence;
  પૂર્ણાંક whence;
  પૂર્ણાંક rc;

  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE OFFSET WHENCE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  જો( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) પાછા TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  જો( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }નહીંતર જો( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }નહીંતર જો( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }નહીંતર{
    Tcl_AppendResult(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_rewind(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_ftell(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_ftruncate(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  પૂર્ણાંક rc;
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE SIZE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  જો( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) પાછા TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_file_size(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_size(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_file_truesize(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_file_mtime(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  time_t t;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(t));
  પાછા TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_remove(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *zFilename;          /* File pattern to configure */
  પૂર્ણાંક rc;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "FILENAME");
    પાછા TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_glob(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *zPattern;          /* The glob pattern */
  અચલ અક્ષર *zText;             /* Text to compare against the pattern */
  પૂર્ણાંક rc;
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "PATTERN TEXT");
    પાછા TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_file_available(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  પાછા TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_quota_ferror(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  પૂર્ણાંક x;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "HANDLE");
    પાછા TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(x));
  પાછા TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
પૂર્ણાંક Sqlitequota_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
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
  પૂર્ણાંક i;

  માટે(i=0; i<કદ(aCmd)/કદ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  પાછા TCL_OK;
}
#endif
