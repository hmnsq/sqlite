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
#ಅನ್ತರ್ಭಾವ "test_quota.h"
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#ಯದಿ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)
#define sqlite3_mutex_held(X)     ((ಶೂನ್ಯ)(X),1)
#define sqlite3_mutex_notheld(X)  ((ಶೂನ್ಯ)(X),1)
#endif /* SQLITE_THREADSAFE==0 */


#ifdef _WIN32
# include <windows.h>
# include <io.h>
#ಅನ್ಯಥಾ
# include <unistd.h>
#endif


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ quotaGroup quotaGroup;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ quotaConn quotaConn;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ quotaFile quotaFile;

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
ಸಂರಚನಾ quotaGroup {
  ಧ್ರುವ ವರ್ಣ *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 iSize;           /* Current size of all files */
  ಶೂನ್ಯ (*xCallback)(             /* Callback invoked when going over quota */
     ಧ್ರುವ ವರ್ಣ *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     ಶೂನ್ಯ *pArg                     /* Client data */
  );
  ಶೂನ್ಯ *pArg;                    /* Third argument to the xCallback() */
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*);       /* Optional destructor for pArg */
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
ಸಂರಚನಾ quotaFile {
  ವರ್ಣ *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 iSize;            /* Current size of this file */
  ಪೂರ್ಣಾಙ್ಕ nRef;                       /* Number of times this file is open */
  ಪೂರ್ಣಾಙ್ಕ deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *pNext, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ಸಂರಚನಾ quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
ಸಂರಚನಾ quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ifdef _WIN32
  ವರ್ಣ *zMbcsName;        /* Full MBCS pathname of the file */
#endif
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
ಸ್ಥಿರ ಸಂರಚನಾ {
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
  ಪೂರ್ಣಾಙ್ಕ isInitialized;

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
ಸ್ಥಿರ ಶೂನ್ಯ quotaEnter(ಶೂನ್ಯ){ sqlite3_mutex_enter(gQuota.pMutex); }
ಸ್ಥಿರ ಶೂನ್ಯ quotaLeave(ಶೂನ್ಯ){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quota_Group_Open_File_ಎಣಿಕೆ(quotaGroup *pGroup){
  ಪೂರ್ಣಾಙ್ಕ N = 0;
  quotaFile *pFile = pGroup->pFiles;
  ಯಾವತ್( pFile ){
    ಯದಿ( pFile->nRef ) N++;
    pFile = pFile->pNext;
  }
  ಪ್ರತ್ಯಾಗಮ N;
}

/* Remove a file from a quota group.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->iSize -= pFile->iSize;
  *pFile->ppPrev = pFile->pNext;
  ಯದಿ( pFile->pNext ) pFile->pNext->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quotaRemoveAllFiles(quotaGroup *pGroup){
  ಯಾವತ್( pGroup->pFiles ){
    ಪ್ರತಿಜ್ಞಾ( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quotaGroupDeref(quotaGroup *pGroup){
  ಯದಿ( pGroup->iLimit==0 && quota_Group_Open_File_ಎಣಿಕೆ(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->ppPrev = pGroup->pNext;
    ಯದಿ( pGroup->pNext ) pGroup->pNext->ppPrev = pGroup->ppPrev;
    ಯದಿ( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaStrglob(ಧ್ರುವ ವರ್ಣ *zGlob, ಧ್ರುವ ವರ್ಣ *z){
  ಪೂರ್ಣಾಙ್ಕ c, c2, cx;
  ಪೂರ್ಣಾಙ್ಕ invert;
  ಪೂರ್ಣಾಙ್ಕ seen;

  ಯಾವತ್( (c = (*(zGlob++)))!=0 ){
    ಯದಿ( c=='*' ){
      ಯಾವತ್( (c=(*(zGlob++))) == '*' || c=='?' ){
        ಯದಿ( c=='?' && (*(z++))==0 ) ಪ್ರತ್ಯಾಗಮ 0;
      }
      ಯದಿ( c==0 ){
        ಪ್ರತ್ಯಾಗಮ 1;
      }ಅನ್ಯಥಾ ಯದಿ( c=='[' ){
        ಯಾವತ್( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        ಪ್ರತ್ಯಾಗಮ (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      ಯಾವತ್( (c2 = (*(z++)))!=0 ){
        ಯಾವತ್( c2!=c && c2!=cx ){
          c2 = *(z++);
          ಯದಿ( c2==0 ) ಪ್ರತ್ಯಾಗಮ 0;
        }
        ಯದಿ( quotaStrglob(zGlob,z) ) ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಪ್ರತ್ಯಾಗಮ 0;
    }ಅನ್ಯಥಾ ಯದಿ( c=='?' ){
      ಯದಿ( (*(z++))==0 ) ಪ್ರತ್ಯಾಗಮ 0;
    }ಅನ್ಯಥಾ ಯದಿ( c=='[' ){
      ಪೂರ್ಣಾಙ್ಕ prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      ಯದಿ( c==0 ) ಪ್ರತ್ಯಾಗಮ 0;
      c2 = *(zGlob++);
      ಯದಿ( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      ಯದಿ( c2==']' ){
        ಯದಿ( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      ಯಾವತ್( c2 && c2!=']' ){
        ಯದಿ( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          ಯದಿ( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }ಅನ್ಯಥಾ{
          ಯದಿ( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      ಯದಿ( c2==0 || (seen ^ invert)==0 ) ಪ್ರತ್ಯಾಗಮ 0;
    }ಅನ್ಯಥಾ ಯದಿ( c=='/' ){
      ಯದಿ( z[0]!='/' && z[0]!='\\' ) ಪ್ರತ್ಯಾಗಮ 0;
      z++;
    }ಅನ್ಯಥಾ{
      ಯದಿ( c!=(*(z++)) ) ಪ್ರತ್ಯಾಗಮ 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
ಸ್ಥಿರ quotaGroup *quotaGroupFind(ಧ್ರುವ ವರ್ಣ *zFilename){
  quotaGroup *p;
  ಪ್ರತಿ(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->pNext){}
  ಪ್ರತ್ಯಾಗಮ p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
ಸ್ಥಿರ sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  ಪ್ರತ್ಯಾಗಮ (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
ಸ್ಥಿರ quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  ಧ್ರುವ ವರ್ಣ *zName,      /* Full pathname of the file */
  ಪೂರ್ಣಾಙ್ಕ createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  ಯಾವತ್( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->pNext;
  }
  ಯದಿ( pFile==0 && createFlag ){
    ಪೂರ್ಣಾಙ್ಕ nName = (ಪೂರ್ಣಾಙ್ಕ)(strlen(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( ಪರಿಮಾಣ(*pFile) + nName + 1 );
    ಯದಿ( pFile ){
      memset(pFile, 0, ಪರಿಮಾಣ(*pFile));
      pFile->zFilename = (ವರ್ಣ*)&pFile[1];
      memcpy(pFile->zFilename, zName, nName+1);
      pFile->pNext = pGroup->pFiles;
      ಯದಿ( pGroup->pFiles ) pGroup->pFiles->ppPrev = &pFile->pNext;
      pFile->ppPrev = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  ಪ್ರತ್ಯಾಗಮ pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
ಸ್ಥಿರ ವರ್ಣ *quota_utf8_to_mbcs(ಧ್ರುವ ವರ್ಣ *zUtf8){
#ifdef _WIN32
  size_t n;          /* Bytes in zUtf8 */
  ಪೂರ್ಣಾಙ್ಕ nWide;         /* number of UTF-16 characters */
  ಪೂರ್ಣಾಙ್ಕ nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  ವರ್ಣ *zMbcs;       /* The MBCS text */
  ಪೂರ್ಣಾಙ್ಕ codepage;      /* Code page used by fopen() */

  n = strlen(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, ರಿಕ್ತ, 0);
  ಯದಿ( nWide==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*ಪರಿಮಾಣ(zTmpWide[0]) );
  ಯದಿ( zTmpWide==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (ವರ್ಣ*)sqlite3_malloc( nMbcs+1 ) : 0;
  ಯದಿ( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  ಪ್ರತ್ಯಾಗಮ zMbcs;
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ*)zUtf8;  /* No-op on unix */
#endif
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
ಸ್ಥಿರ ಶೂನ್ಯ quota_mbcs_free(ವರ್ಣ *zOld){
#ifdef _WIN32
  sqlite3_free(zOld);
#ಅನ್ಯಥಾ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ಧ್ರುವ ವರ್ಣ *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  ಪೂರ್ಣಾಙ್ಕ flags,                  /* Flags to control the opening */
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags              /* Flags showing results of opening */
){
  ಪೂರ್ಣಾಙ್ಕ rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  ಯದಿ( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    ಪ್ರತ್ಯಾಗಮ pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  ಯದಿ( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }ಅನ್ಯಥಾ{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    ಯದಿ( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      ಯದಿ( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      ಯದಿ( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }ಅನ್ಯಥಾ{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ಧ್ರುವ ವರ್ಣ *zName,          /* Name of file to be deleted */
  ಪೂರ್ಣಾಙ್ಕ syncDir                 /* Do a directory sync after deleting */
){
  ಪೂರ್ಣಾಙ್ಕ rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  ಯದಿ( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    ಯದಿ( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      ಯದಿ( pFile ){
        ಯದಿ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ಅನ್ಯಥಾ{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  ಯದಿ( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    ಯದಿ( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaRead(
  sqlite3_file *pConn,
  ಶೂನ್ಯ *pBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaWrite(
  sqlite3_file *pConn,
  ಧ್ರುವ ಶೂನ್ಯ *pBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  ಯದಿ( pFile->iSize<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    ಯದಿ( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      ಯದಿ( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      ಯದಿ( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        ಪ್ರತ್ಯಾಗಮ SQLITE_FULL;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪೂರ್ಣಾಙ್ಕ rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  ಯದಿ( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = size;
    pGroup->iSize += size;
    quotaLeave();
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaSync(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  ಪೂರ್ಣಾಙ್ಕ rc;

  rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz);
  ಯದಿ( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = sz;
    pGroup->iSize += sz;
    quotaLeave();
    *pSize = sz;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaLock(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaUnlock(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaCheckReservedLock(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaFileControl(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪೂರ್ಣಾಙ್ಕ rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#ಯದಿ defined(SQLITE_FCNTL_VFSNAME)
  ಯದಿ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(ವರ್ಣ**)pArg = sqlite3_mprintf("quota/%z", *(ವರ್ಣ**)pArg);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaSectorSize(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xSectorSize(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ಪೂರ್ಣಾಙ್ಕ iRegion,                    /* Region to retrieve */
  ಪೂರ್ಣಾಙ್ಕ szRegion,                   /* Size of regions */
  ಪೂರ್ಣಾಙ್ಕ bExtend,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಚಞ್ಚಲ **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ಪೂರ್ಣಾಙ್ಕ ofst,                  /* First lock to acquire or release */
  ಪೂರ್ಣಾಙ್ಕ n,                     /* Number of locks to acquire or release */
  ಪೂರ್ಣಾಙ್ಕ flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotaShmUnmap(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_initialize(ಧ್ರುವ ವರ್ಣ *zOrigVfsName, ಪೂರ್ಣಾಙ್ಕ makeDefault){
  sqlite3_vfs *pOrigVfs;
  ಯದಿ( gQuota.isInitialized ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  ಯದಿ( pOrigVfs==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  ಪ್ರತಿಜ್ಞಾ( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ಯದಿ( !gQuota.pMutex ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += ಪರಿಮಾಣ(quotaConn);
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
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_shutdown(ಶೂನ್ಯ){
  quotaGroup *pGroup;
  ಯದಿ( gQuota.isInitialized==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE;
  ಪ್ರತಿ(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    ಯದಿ( quota_Group_Open_File_ಎಣಿಕೆ(pGroup)>0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE;
  }
  ಯಾವತ್( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->pNext;
    pGroup->iLimit = 0;
    ಪ್ರತಿಜ್ಞಾ( quota_Group_Open_File_ಎಣಿಕೆ(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, ಪರಿಮಾಣ(gQuota));
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_set(
  ಧ್ರುವ ವರ್ಣ *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  ಶೂನ್ಯ (*xCallback)(              /* Callback invoked when going over quota */
     ಧ್ರುವ ವರ್ಣ *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     ಶೂನ್ಯ *pArg                     /* Client data */
  ),
  ಶೂನ್ಯ *pArg,                     /* client data passed thru to callback */
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*)         /* Optional destructor for pArg */
){
  quotaGroup *pGroup;
  quotaEnter();
  pGroup = gQuota.pGroup;
  ಯಾವತ್( pGroup && strcmp(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->pNext;
  }
  ಯದಿ( pGroup==0 ){
    ಪೂರ್ಣಾಙ್ಕ nPattern = (ಪೂರ್ಣಾಙ್ಕ)(strlen(zPattern) & 0x3fffffff);
    ಯದಿ( iLimit<=0 ){
      quotaLeave();
      ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( ಪರಿಮಾಣ(*pGroup) + nPattern + 1 );
    ಯದಿ( pGroup==0 ){
      quotaLeave();
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    memset(pGroup, 0, ಪರಿಮಾಣ(*pGroup));
    pGroup->zPattern = (ವರ್ಣ*)&pGroup[1];
    memcpy((ವರ್ಣ *)pGroup->zPattern, zPattern, nPattern+1);
    ಯದಿ( gQuota.pGroup ) gQuota.pGroup->ppPrev = &pGroup->pNext;
    pGroup->pNext = gQuota.pGroup;
    pGroup->ppPrev = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  ಯದಿ( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_file(ಧ್ರುವ ವರ್ಣ *zFilename){
  ವರ್ಣ *zFull = 0;
  sqlite3_file *fd;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ outFlags = 0;
  sqlite3_int64 iSize;
  ಪೂರ್ಣಾಙ್ಕ nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  ಯದಿ( fd==0 ){
    rc = SQLITE_NOMEM;
  }ಅನ್ಯಥಾ{
    zFull = &((ವರ್ಣ *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  ಯದಿ( rc==SQLITE_OK ){
    zFull[strlen(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    ಯದಿ( rc==SQLITE_OK ){
      fd->pMethods->xFileSize(fd, &iSize);
      fd->pMethods->xClose(fd);
    }ಅನ್ಯಥಾ ಯದಿ( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      ಯದಿ( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        ಯದಿ( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(ಧ್ರುವ ವರ್ಣ *zFilename, ಧ್ರುವ ವರ್ಣ *zMode){
  quota_FILE *p = 0;
  ವರ್ಣ *zFull = 0;
  ವರ್ಣ *zFullTranslated = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (ವರ್ಣ*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ಯದಿ( zFull==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  ಯದಿ( rc ) ಗಚ್ಛ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(ಪರಿಮಾಣ(*p));
  ಯದಿ( p==0 ) ಗಚ್ಛ quota_fopen_error;
  memset(p, 0, ಪರಿಮಾಣ(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  ಯದಿ( zFullTranslated==0 ) ಗಚ್ಛ quota_fopen_error;
  p->f = fopen(zFullTranslated, zMode);
  ಯದಿ( p->f==0 ) ಗಚ್ಛ quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  ಯದಿ( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    ಯದಿ( pFile==0 ){
      quotaLeave();
      ಗಚ್ಛ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(zFull);
#ifdef _WIN32
  p->zMbcsName = zFullTranslated;
#endif
  ಪ್ರತ್ಯಾಗಮ p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  ಯದಿ( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  ಶೂನ್ಯ *pBuf,            /* Store the content here */
  size_t size,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  ಪ್ರತ್ಯಾಗಮ fread(pBuf, size, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  ಧ್ರುವ ಶೂನ್ಯ *pBuf,      /* Take content to write from here */
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
  ಯದಿ( pFile && pFile->iSize<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    ಯದಿ( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      ಯದಿ( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      ಯದಿ( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->iSize + pFile->iSize;
        nmemb = (size_t)((iEnd - iOfst)/size);
        iEnd = iOfst + size*nmemb;
        szNew = pGroup->iSize - pFile->iSize + iEnd;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }ಅನ್ಯಥಾ{
    pFile = 0;
  }
  rc = fwrite(pBuf, size, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  ಯದಿ( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + size*nWritten;
    ಯದಿ( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->iSize += iNewEnd - pFile->iSize;
    pFile->iSize = iNewEnd;
    quotaLeave();
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Close an open quota_FILE stream.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_fclose(quota_FILE *p){
  ಪೂರ್ಣಾಙ್ಕ rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  ಯದಿ( pFile ){
    quotaEnter();
    pFile->nRef--;
    ಯದಿ( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      ಯದಿ( pFile->deleteOnClose ){
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
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_fflush(quota_FILE *p, ಪೂರ್ಣಾಙ್ಕ doFsync){
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = fflush(p->f);
  ಯದಿ( rc==0 && doFsync ){
#ifdef _WIN32
    rc = _commit(_fileno(p->f));
#ಅನ್ಯಥಾ
    rc = fsync(fileno(p->f));
#endif
  }
  ಪ್ರತ್ಯಾಗಮ rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_fseek(quota_FILE *p, long offset, ಪೂರ್ಣಾಙ್ಕ whence){
  ಪ್ರತ್ಯಾಗಮ fseek(p->f, offset, whence);
}

/*
** rewind a quota_FILE stream.
*/
ಶೂನ್ಯ sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
long sqlite3_quota_ftell(quota_FILE *p){
  ಪ್ರತ್ಯಾಗಮ ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_ferror(quota_FILE *p){
  ಪ್ರತ್ಯಾಗಮ ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( (pFile = p->pFile)!=0 && pFile->iSize<szNew ){
    quotaGroup *pGroup;
    ಯದಿ( pFile->iSize<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      ಪ್ರತ್ಯಾಗಮ -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    quotaLeave();
  }
#ifdef _WIN32
#  ಯದಿ defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (long)szNew);
#  ಅನ್ಯಥಾ
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#ಅನ್ಯಥಾ
  rc = ftruncate(fileno(p->f), szNew);
#endif
  ಯದಿ( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    pFile->iSize = szNew;
    quotaLeave();
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_file_mtime(quota_FILE *p, time_t *pTime){
  ಪೂರ್ಣಾಙ್ಕ rc;
#ifdef _WIN32
  ಸಂರಚನಾ _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ಅನ್ಯಥಾ
  ಸಂರಚನಾ stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  ಯದಿ( rc==0 ) *pTime = buf.st_mtime;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  ಪೂರ್ಣಾಙ್ಕ rc;
#ifdef _WIN32
  ಸಂರಚನಾ _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ಅನ್ಯಥಾ
  ಸಂರಚನಾ stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  ಪ್ರತ್ಯಾಗಮ rc==0 ? buf.st_size : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_size(quota_FILE *p){
  ಪ್ರತ್ಯಾಗಮ p->pFile ? p->pFile->iSize : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
long sqlite3_quota_file_available(quota_FILE *p){
  FILE* f = p->f;
  long pos1, pos2;
  ಪೂರ್ಣಾಙ್ಕ rc;
  pos1 = ftell(f);
  ಯದಿ ( pos1 < 0 ) ಪ್ರತ್ಯಾಗಮ -1;
  rc = fseek(f, 0, SEEK_END);
  ಯದಿ ( rc != 0 ) ಪ್ರತ್ಯಾಗಮ -1;
  pos2 = ftell(f);
  ಯದಿ ( pos2 < 0 ) ಪ್ರತ್ಯಾಗಮ -1;
  rc = fseek(f, pos1, SEEK_SET);
  ಯದಿ ( rc != 0 ) ಪ್ರತ್ಯಾಗಮ -1;
  ಪ್ರತ್ಯಾಗಮ pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_quota_remove(ಧ್ರುವ ವರ್ಣ *zFilename){
  ವರ್ಣ *zFull;            /* Full pathname for zFilename */
  size_t nFull;           /* Number of bytes in zFilename */
  ಪೂರ್ಣಾಙ್ಕ rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *pNextFile;   /* next file in the group */
  ಪೂರ್ಣಾಙ್ಕ diff;               /* Difference between filenames */
  ವರ್ಣ c;                 /* First character past end of pattern */

  zFull = (ವರ್ಣ*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ಯದಿ( zFull==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  ಯದಿ( rc ){
    sqlite3_free(zFull);
    ಪ್ರತ್ಯಾಗಮ rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = strlen(zFull);
  ಯದಿ( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  ಯದಿ( pGroup ){
    ಪ್ರತಿ(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=pNextFile){
      pNextFile = pFile->pNext;
      diff = strncmp(zFull, pFile->zFilename, nFull);
      ಯದಿ( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        ಯದಿ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ಅನ್ಯಥಾ{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(zFull);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TclQuotaCallback TclQuotaCallback;
ಸಂರಚನಾ TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);


/*
** This is the callback from a quota-over-limit.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tclQuotaCallback(
  ಧ್ರುವ ವರ್ಣ *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 iSize,            /* Total size of all files in the group */
  ಶೂನ್ಯ *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  unsigned ಪೂರ್ಣಾಙ್ಕ rnd;               /* Random part of pVarname */
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pVarname);
  sqlite3_randomness(ಪರಿಮಾಣ(rnd), (ಶೂನ್ಯ *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((ಪೂರ್ಣಾಙ್ಕ)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(iSize));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  ಯದಿ( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pVarname);
  ಯದಿ( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tclCallbackDestructor(ಶೂನ್ಯ *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  ಯದಿ( p ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(p->pScript);
    sqlite3_free((ವರ್ಣ *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_initialize(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *zName;              /* Name of new quota VFS */
  ಪೂರ್ಣಾಙ್ಕ makeDefault;                /* True to make the new VFS the default */
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_shutdown(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Value returned by quota_shutdown() */

  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_set(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_Size nScript;               /* Length of callback script */
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*);        /* Optional destructor for pArg */
  ಶೂನ್ಯ (*xCallback)(ಧ್ರುವ ವರ್ಣ *, sqlite3_int64 *, sqlite3_int64, ಶೂನ್ಯ *);

  /* Process arguments */
  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  ಯದಿ( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  ಯದಿ( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(ಪರಿಮಾಣ(TclQuotaCallback));
    ಯದಿ( !p ){
      Tcl_SetResult(interp, (ವರ್ಣ *)"SQLITE_NOMEM", TCL_STATIC);
      ಪ್ರತ್ಯಾಗಮ TCL_OK;
    }
    memset(p, 0, ಪರಿಮಾಣ(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }ಅನ್ಯಥಾ{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (ಶೂನ್ಯ*)p, xDestroy);

  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_file(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *zFilename;          /* File pattern to configure */
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "FILENAME");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_dump(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pResult;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  pResult = Tcl_NewObj();
  quotaEnter();
  ಪ್ರತಿ(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iSize));
    ಪ್ರತಿ(pFile=pGroup->pFiles; pFile; pFile=pFile->pNext){
      ಪೂರ್ಣಾಙ್ಕ i;
      ವರ್ಣ zTemp[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(ಪರಿಮಾಣ(zTemp), zTemp, "%s", pFile->zFilename);
      ಪ್ರತಿ(i=0; zTemp[i]; i++){ ಯದಿ( zTemp[i]=='\\' ) zTemp[i] = '/'; }
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
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/* Defined in test1.c */
ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3TestTextToPtr(ಧ್ರುವ ವರ್ಣ*);
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3TestPtrToText(ಶೂನ್ಯ*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_fopen(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *zFilename;          /* File pattern to configure */
  ಧ್ರುವ ವರ್ಣ *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "FILENAME MODE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_SetResult(interp, (ವರ್ಣ*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_fread(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ವರ್ಣ *zBuf;
  ಪೂರ್ಣಾಙ್ಕ sz;
  ಪೂರ್ಣಾಙ್ಕ nElem;
  size_t got;

  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE SIZE NELEM");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zBuf = (ವರ್ಣ*)sqlite3_malloc( sz*nElem + 1 );
  ಯದಿ( zBuf==0 ){
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_SetResult(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_fwrite(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ವರ್ಣ *zBuf;
  ಪೂರ್ಣಾಙ್ಕ sz;
  ಪೂರ್ಣಾಙ್ಕ nElem;
  size_t got;

  ಯದಿ( objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(got));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_fclose(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_fflush(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ doSync = 0;

  ಯದಿ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE ?HARDSYNC?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ಯದಿ( objc==3 ){
    ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_fseek(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಪೂರ್ಣಾಙ್ಕ ofst;
  ಧ್ರುವ ವರ್ಣ *zWhence;
  ಪೂರ್ಣಾಙ್ಕ whence;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE OFFSET WHENCE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  ಯದಿ( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }ಅನ್ಯಥಾ ಯದಿ( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }ಅನ್ಯಥಾ ಯದಿ( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }ಅನ್ಯಥಾ{
    Tcl_AppendResult(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_rewind(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_ftell(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_ftruncate(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE SIZE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ಯದಿ( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_file_size(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_size(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_file_truesize(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_file_mtime(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  time_t t;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(t));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_remove(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *zFilename;          /* File pattern to configure */
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "FILENAME");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_glob(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *zPattern;          /* The glob pattern */
  ಧ್ರುವ ವರ್ಣ *zText;             /* Text to compare against the pattern */
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "PATTERN TEXT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_file_available(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_quota_ferror(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಪೂರ್ಣಾಙ್ಕ x;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(x));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitequota_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
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
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aCmd)/ಪರಿಮಾಣ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
#endif
