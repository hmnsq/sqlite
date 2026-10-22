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
#ଅନ୍ତର୍ଭାଵ "test_quota.h"
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#ଯଦି defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)
#define sqlite3_mutex_held(X)     ((ଶୂନ୍ୟ)(X),1)
#define sqlite3_mutex_notheld(X)  ((ଶୂନ୍ୟ)(X),1)
#endif /* SQLITE_THREADSAFE==0 */


#ifdef _WIN32
# include <windows.h>
# include <io.h>
#ଅନ୍ୟଥା
# include <unistd.h>
#endif


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା quotaGroup quotaGroup;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା quotaConn quotaConn;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା quotaFile quotaFile;

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
ସଂରଚନା quotaGroup {
  ଅଚଳ ଅକ୍ଷର *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 iSize;           /* Current size of all files */
  ଶୂନ୍ୟ (*xCallback)(             /* Callback invoked when going over quota */
     ଅଚଳ ଅକ୍ଷର *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     ଶୂନ୍ୟ *pArg                     /* Client data */
  );
  ଶୂନ୍ୟ *pArg;                    /* Third argument to the xCallback() */
  ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ*);       /* Optional destructor for pArg */
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
ସଂରଚନା quotaFile {
  ଅକ୍ଷର *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 iSize;            /* Current size of this file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRef;                       /* Number of times this file is open */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *pNext, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ସଂରଚନା quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
ସଂରଚନା quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ifdef _WIN32
  ଅକ୍ଷର *zMbcsName;        /* Full MBCS pathname of the file */
#endif
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
ସ୍ଥିର ସଂରଚନା {
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInitialized;

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
ସ୍ଥିର ଶୂନ୍ୟ quotaEnter(ଶୂନ୍ୟ){ sqlite3_mutex_enter(gQuota.pMutex); }
ସ୍ଥିର ଶୂନ୍ୟ quotaLeave(ଶୂନ୍ୟ){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quota_Group_Open_File_ଗଣନା(quotaGroup *pGroup){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା N = 0;
  quotaFile *pFile = pGroup->pFiles;
  ଯେତେବେଳେ( pFile ){
    ଯଦି( pFile->nRef ) N++;
    pFile = pFile->pNext;
  }
  ଫେରନ୍ତୁ N;
}

/* Remove a file from a quota group.
*/
ସ୍ଥିର ଶୂନ୍ୟ quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->iSize -= pFile->iSize;
  *pFile->ppPrev = pFile->pNext;
  ଯଦି( pFile->pNext ) pFile->pNext->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
ସ୍ଥିର ଶୂନ୍ୟ quotaRemoveAllFiles(quotaGroup *pGroup){
  ଯେତେବେଳେ( pGroup->pFiles ){
    ପ୍ରତିଜ୍ଞା( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
ସ୍ଥିର ଶୂନ୍ୟ quotaGroupDeref(quotaGroup *pGroup){
  ଯଦି( pGroup->iLimit==0 && quota_Group_Open_File_ଗଣନା(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->ppPrev = pGroup->pNext;
    ଯଦି( pGroup->pNext ) pGroup->pNext->ppPrev = pGroup->ppPrev;
    ଯଦି( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaStrglob(ଅଚଳ ଅକ୍ଷର *zGlob, ଅଚଳ ଅକ୍ଷର *z){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, c2, cx;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା invert;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା seen;

  ଯେତେବେଳେ( (c = (*(zGlob++)))!=0 ){
    ଯଦି( c=='*' ){
      ଯେତେବେଳେ( (c=(*(zGlob++))) == '*' || c=='?' ){
        ଯଦି( c=='?' && (*(z++))==0 ) ଫେରନ୍ତୁ 0;
      }
      ଯଦି( c==0 ){
        ଫେରନ୍ତୁ 1;
      }ଅନ୍ୟଥା ଯଦି( c=='[' ){
        ଯେତେବେଳେ( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        ଫେରନ୍ତୁ (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      ଯେତେବେଳେ( (c2 = (*(z++)))!=0 ){
        ଯେତେବେଳେ( c2!=c && c2!=cx ){
          c2 = *(z++);
          ଯଦି( c2==0 ) ଫେରନ୍ତୁ 0;
        }
        ଯଦି( quotaStrglob(zGlob,z) ) ଫେରନ୍ତୁ 1;
      }
      ଫେରନ୍ତୁ 0;
    }ଅନ୍ୟଥା ଯଦି( c=='?' ){
      ଯଦି( (*(z++))==0 ) ଫେରନ୍ତୁ 0;
    }ଅନ୍ୟଥା ଯଦି( c=='[' ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      ଯଦି( c==0 ) ଫେରନ୍ତୁ 0;
      c2 = *(zGlob++);
      ଯଦି( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      ଯଦି( c2==']' ){
        ଯଦି( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      ଯେତେବେଳେ( c2 && c2!=']' ){
        ଯଦି( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          ଯଦି( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }ଅନ୍ୟଥା{
          ଯଦି( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      ଯଦି( c2==0 || (seen ^ invert)==0 ) ଫେରନ୍ତୁ 0;
    }ଅନ୍ୟଥା ଯଦି( c=='/' ){
      ଯଦି( z[0]!='/' && z[0]!='\\' ) ଫେରନ୍ତୁ 0;
      z++;
    }ଅନ୍ୟଥା{
      ଯଦି( c!=(*(z++)) ) ଫେରନ୍ତୁ 0;
    }
  }
  ଫେରନ୍ତୁ *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
ସ୍ଥିର quotaGroup *quotaGroupFind(ଅଚଳ ଅକ୍ଷର *zFilename){
  quotaGroup *p;
  ପାଇଁ(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->pNext){}
  ଫେରନ୍ତୁ p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
ସ୍ଥିର sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  ଫେରନ୍ତୁ (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
ସ୍ଥିର quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  ଅଚଳ ଅକ୍ଷର *zName,      /* Full pathname of the file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  ଯେତେବେଳେ( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->pNext;
  }
  ଯଦି( pFile==0 && createFlag ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(strlen(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( ଆକାର(*pFile) + nName + 1 );
    ଯଦି( pFile ){
      memset(pFile, 0, ଆକାର(*pFile));
      pFile->zFilename = (ଅକ୍ଷର*)&pFile[1];
      memcpy(pFile->zFilename, zName, nName+1);
      pFile->pNext = pGroup->pFiles;
      ଯଦି( pGroup->pFiles ) pGroup->pFiles->ppPrev = &pFile->pNext;
      pFile->ppPrev = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  ଫେରନ୍ତୁ pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
ସ୍ଥିର ଅକ୍ଷର *quota_utf8_to_mbcs(ଅଚଳ ଅକ୍ଷର *zUtf8){
#ifdef _WIN32
  size_t n;          /* Bytes in zUtf8 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nWide;         /* number of UTF-16 characters */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  ଅକ୍ଷର *zMbcs;       /* The MBCS text */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା codepage;      /* Code page used by fopen() */

  n = strlen(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, ଖାଲି, 0);
  ଯଦି( nWide==0 ) ଫେରନ୍ତୁ 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*ଆକାର(zTmpWide[0]) );
  ଯଦି( zTmpWide==0 ) ଫେରନ୍ତୁ 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (ଅକ୍ଷର*)sqlite3_malloc( nMbcs+1 ) : 0;
  ଯଦି( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  ଫେରନ୍ତୁ zMbcs;
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ (ଅକ୍ଷର*)zUtf8;  /* No-op on unix */
#endif
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
ସ୍ଥିର ଶୂନ୍ୟ quota_mbcs_free(ଅକ୍ଷର *zOld){
#ifdef _WIN32
  sqlite3_free(zOld);
#ଅନ୍ୟଥା
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ଅଚଳ ଅକ୍ଷର *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,                  /* Flags to control the opening */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags              /* Flags showing results of opening */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  ଯଦି( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    ଫେରନ୍ତୁ pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  ଯଦି( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }ଅନ୍ୟଥା{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    ଯଦି( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      ଯଦି( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        ଫେରନ୍ତୁ SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      ଯଦି( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }ଅନ୍ୟଥା{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  ଫେରନ୍ତୁ rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ଅଚଳ ଅକ୍ଷର *zName,          /* Name of file to be deleted */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା syncDir                 /* Do a directory sync after deleting */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  ଯଦି( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    ଯଦି( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      ଯଦି( pFile ){
        ଯଦି( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ଅନ୍ୟଥା{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  ଫେରନ୍ତୁ rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  ଯଦି( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    ଯଦି( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  ଫେରନ୍ତୁ rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaRead(
  sqlite3_file *pConn,
  ଶୂନ୍ୟ *pBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaWrite(
  sqlite3_file *pConn,
  ଅଚଳ ଶୂନ୍ୟ *pBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  ଯଦି( pFile->iSize<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    ଯଦି( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      ଯଦି( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      ଯଦି( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        ଫେରନ୍ତୁ SQLITE_FULL;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }
  ଫେରନ୍ତୁ pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  ଯଦି( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = size;
    pGroup->iSize += size;
    quotaLeave();
  }
  ଫେରନ୍ତୁ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaSync(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz);
  ଯଦି( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = sz;
    pGroup->iSize += sz;
    quotaLeave();
    *pSize = sz;
  }
  ଫେରନ୍ତୁ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaLock(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaUnlock(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaCheckReservedLock(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaFileControl(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#ଯଦି defined(SQLITE_FCNTL_VFSNAME)
  ଯଦି( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(ଅକ୍ଷର**)pArg = sqlite3_mprintf("quota/%z", *(ଅକ୍ଷର**)pArg);
  }
#endif
  ଫେରନ୍ତୁ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaSectorSize(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xSectorSize(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRegion,                    /* Region to retrieve */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szRegion,                   /* Size of regions */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bExtend,                    /* True to extend file if necessary */
  ଶୂନ୍ୟ ଅସ୍ଥିର **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst,                  /* First lock to acquire or release */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,                     /* Number of locks to acquire or release */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ସ୍ଥିର ଶୂନ୍ୟ quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotaShmUnmap(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ଫେରନ୍ତୁ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_initialize(ଅଚଳ ଅକ୍ଷର *zOrigVfsName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା makeDefault){
  sqlite3_vfs *pOrigVfs;
  ଯଦି( gQuota.isInitialized ) ଫେରନ୍ତୁ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  ଯଦି( pOrigVfs==0 ) ଫେରନ୍ତୁ SQLITE_ERROR;
  ପ୍ରତିଜ୍ଞା( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ଯଦି( !gQuota.pMutex ){
    ଫେରନ୍ତୁ SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += ଆକାର(quotaConn);
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
  ଫେରନ୍ତୁ SQLITE_OK;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_shutdown(ଶୂନ୍ୟ){
  quotaGroup *pGroup;
  ଯଦି( gQuota.isInitialized==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE;
  ପାଇଁ(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    ଯଦି( quota_Group_Open_File_ଗଣନା(pGroup)>0 ) ଫେରନ୍ତୁ SQLITE_MISUSE;
  }
  ଯେତେବେଳେ( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->pNext;
    pGroup->iLimit = 0;
    ପ୍ରତିଜ୍ଞା( quota_Group_Open_File_ଗଣନା(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, ଆକାର(gQuota));
  ଫେରନ୍ତୁ SQLITE_OK;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_set(
  ଅଚଳ ଅକ୍ଷର *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  ଶୂନ୍ୟ (*xCallback)(              /* Callback invoked when going over quota */
     ଅଚଳ ଅକ୍ଷର *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     ଶୂନ୍ୟ *pArg                     /* Client data */
  ),
  ଶୂନ୍ୟ *pArg,                     /* client data passed thru to callback */
  ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ*)         /* Optional destructor for pArg */
){
  quotaGroup *pGroup;
  quotaEnter();
  pGroup = gQuota.pGroup;
  ଯେତେବେଳେ( pGroup && strcmp(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->pNext;
  }
  ଯଦି( pGroup==0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPattern = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(strlen(zPattern) & 0x3fffffff);
    ଯଦି( iLimit<=0 ){
      quotaLeave();
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( ଆକାର(*pGroup) + nPattern + 1 );
    ଯଦି( pGroup==0 ){
      quotaLeave();
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
    memset(pGroup, 0, ଆକାର(*pGroup));
    pGroup->zPattern = (ଅକ୍ଷର*)&pGroup[1];
    memcpy((ଅକ୍ଷର *)pGroup->zPattern, zPattern, nPattern+1);
    ଯଦି( gQuota.pGroup ) gQuota.pGroup->ppPrev = &pGroup->pNext;
    pGroup->pNext = gQuota.pGroup;
    pGroup->ppPrev = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  ଯଦି( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_file(ଅଚଳ ଅକ୍ଷର *zFilename){
  ଅକ୍ଷର *zFull = 0;
  sqlite3_file *fd;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା outFlags = 0;
  sqlite3_int64 iSize;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  ଯଦି( fd==0 ){
    rc = SQLITE_NOMEM;
  }ଅନ୍ୟଥା{
    zFull = &((ଅକ୍ଷର *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  ଯଦି( rc==SQLITE_OK ){
    zFull[strlen(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    ଯଦି( rc==SQLITE_OK ){
      fd->pMethods->xFileSize(fd, &iSize);
      fd->pMethods->xClose(fd);
    }ଅନ୍ୟଥା ଯଦି( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      ଯଦି( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        ଯଦି( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  ଫେରନ୍ତୁ rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(ଅଚଳ ଅକ୍ଷର *zFilename, ଅଚଳ ଅକ୍ଷର *zMode){
  quota_FILE *p = 0;
  ଅକ୍ଷର *zFull = 0;
  ଅକ୍ଷର *zFullTranslated = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (ଅକ୍ଷର*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ଯଦି( zFull==0 ) ଫେରନ୍ତୁ 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  ଯଦି( rc ) ଯାଅ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(ଆକାର(*p));
  ଯଦି( p==0 ) ଯାଅ quota_fopen_error;
  memset(p, 0, ଆକାର(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  ଯଦି( zFullTranslated==0 ) ଯାଅ quota_fopen_error;
  p->f = fopen(zFullTranslated, zMode);
  ଯଦି( p->f==0 ) ଯାଅ quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  ଯଦି( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    ଯଦି( pFile==0 ){
      quotaLeave();
      ଯାଅ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(zFull);
#ifdef _WIN32
  p->zMbcsName = zFullTranslated;
#endif
  ଫେରନ୍ତୁ p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  ଯଦି( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  ଫେରନ୍ତୁ 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  ଶୂନ୍ୟ *pBuf,            /* Store the content here */
  size_t size,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  ଫେରନ୍ତୁ fread(pBuf, size, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  ଅଚଳ ଶୂନ୍ୟ *pBuf,      /* Take content to write from here */
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
  ଯଦି( pFile && pFile->iSize<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    ଯଦି( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      ଯଦି( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      ଯଦି( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->iSize + pFile->iSize;
        nmemb = (size_t)((iEnd - iOfst)/size);
        iEnd = iOfst + size*nmemb;
        szNew = pGroup->iSize - pFile->iSize + iEnd;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }ଅନ୍ୟଥା{
    pFile = 0;
  }
  rc = fwrite(pBuf, size, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  ଯଦି( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + size*nWritten;
    ଯଦି( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->iSize += iNewEnd - pFile->iSize;
    pFile->iSize = iNewEnd;
    quotaLeave();
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Close an open quota_FILE stream.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_fclose(quota_FILE *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  ଯଦି( pFile ){
    quotaEnter();
    pFile->nRef--;
    ଯଦି( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      ଯଦି( pFile->deleteOnClose ){
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
  ଫେରନ୍ତୁ rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_fflush(quota_FILE *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା doFsync){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  rc = fflush(p->f);
  ଯଦି( rc==0 && doFsync ){
#ifdef _WIN32
    rc = _commit(_fileno(p->f));
#ଅନ୍ୟଥା
    rc = fsync(fileno(p->f));
#endif
  }
  ଫେରନ୍ତୁ rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_fseek(quota_FILE *p, long offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା whence){
  ଫେରନ୍ତୁ fseek(p->f, offset, whence);
}

/*
** rewind a quota_FILE stream.
*/
ଶୂନ୍ୟ sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
long sqlite3_quota_ftell(quota_FILE *p){
  ଫେରନ୍ତୁ ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_ferror(quota_FILE *p){
  ଫେରନ୍ତୁ ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( (pFile = p->pFile)!=0 && pFile->iSize<szNew ){
    quotaGroup *pGroup;
    ଯଦି( pFile->iSize<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      ଫେରନ୍ତୁ -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    quotaLeave();
  }
#ifdef _WIN32
#  ଯଦି defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (long)szNew);
#  ଅନ୍ୟଥା
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#ଅନ୍ୟଥା
  rc = ftruncate(fileno(p->f), szNew);
#endif
  ଯଦି( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    pFile->iSize = szNew;
    quotaLeave();
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_file_mtime(quota_FILE *p, time_t *pTime){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
#ifdef _WIN32
  ସଂରଚନା _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ଅନ୍ୟଥା
  ସଂରଚନା stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  ଯଦି( rc==0 ) *pTime = buf.st_mtime;
  ଫେରନ୍ତୁ rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
#ifdef _WIN32
  ସଂରଚନା _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ଅନ୍ୟଥା
  ସଂରଚନା stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  ଫେରନ୍ତୁ rc==0 ? buf.st_size : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_size(quota_FILE *p){
  ଫେରନ୍ତୁ p->pFile ? p->pFile->iSize : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
long sqlite3_quota_file_available(quota_FILE *p){
  FILE* f = p->f;
  long pos1, pos2;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  pos1 = ftell(f);
  ଯଦି ( pos1 < 0 ) ଫେରନ୍ତୁ -1;
  rc = fseek(f, 0, SEEK_END);
  ଯଦି ( rc != 0 ) ଫେରନ୍ତୁ -1;
  pos2 = ftell(f);
  ଯଦି ( pos2 < 0 ) ଫେରନ୍ତୁ -1;
  rc = fseek(f, pos1, SEEK_SET);
  ଯଦି ( rc != 0 ) ଫେରନ୍ତୁ -1;
  ଫେରନ୍ତୁ pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_quota_remove(ଅଚଳ ଅକ୍ଷର *zFilename){
  ଅକ୍ଷର *zFull;            /* Full pathname for zFilename */
  size_t nFull;           /* Number of bytes in zFilename */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *pNextFile;   /* next file in the group */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା diff;               /* Difference between filenames */
  ଅକ୍ଷର c;                 /* First character past end of pattern */

  zFull = (ଅକ୍ଷର*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ଯଦି( zFull==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  ଯଦି( rc ){
    sqlite3_free(zFull);
    ଫେରନ୍ତୁ rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = strlen(zFull);
  ଯଦି( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  ଯଦି( pGroup ){
    ପାଇଁ(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=pNextFile){
      pNextFile = pFile->pNext;
      diff = strncmp(zFull, pFile->zFilename, nFull);
      ଯଦି( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        ଯଦି( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ଅନ୍ୟଥା{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(zFull);
  ଫେରନ୍ତୁ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TclQuotaCallback TclQuotaCallback;
ସଂରଚନା TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);


/*
** This is the callback from a quota-over-limit.
*/
ସ୍ଥିର ଶୂନ୍ୟ tclQuotaCallback(
  ଅଚଳ ଅକ୍ଷର *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 iSize,            /* Total size of all files in the group */
  ଶୂନ୍ୟ *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା rnd;               /* Random part of pVarname */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_ଗଣନା(pVarname);
  sqlite3_randomness(ଆକାର(rnd), (ଶୂନ୍ୟ *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ଗଣନା(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(iSize));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  ଯଦି( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_ଗଣନା(pEval);
  Tcl_Decr_Ref_ଗଣନା(pVarname);
  ଯଦି( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
ସ୍ଥିର ଶୂନ୍ୟ tclCallbackDestructor(ଶୂନ୍ୟ *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  ଯଦି( p ){
    Tcl_Decr_Ref_ଗଣନା(p->pScript);
    sqlite3_free((ଅକ୍ଷର *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_initialize(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *zName;              /* Name of new quota VFS */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା makeDefault;                /* True to make the new VFS the default */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ଯଦି( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_shutdown(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Value returned by quota_shutdown() */

  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_set(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_Size nScript;               /* Length of callback script */
  ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ*);        /* Optional destructor for pArg */
  ଶୂନ୍ୟ (*xCallback)(ଅଚଳ ଅକ୍ଷର *, sqlite3_int64 *, sqlite3_int64, ଶୂନ୍ୟ *);

  /* Process arguments */
  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  ଯଦି( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) ଫେରନ୍ତୁ TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  ଯଦି( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(ଆକାର(TclQuotaCallback));
    ଯଦି( !p ){
      Tcl_SetResult(interp, (ଅକ୍ଷର *)"SQLITE_NOMEM", TCL_STATIC);
      ଫେରନ୍ତୁ TCL_OK;
    }
    memset(p, 0, ଆକାର(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_ଗଣନା(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }ଅନ୍ୟଥା{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (ଶୂନ୍ୟ*)p, xDestroy);

  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_file(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *zFilename;          /* File pattern to configure */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "FILENAME");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_dump(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pResult;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  pResult = Tcl_NewObj();
  quotaEnter();
  ପାଇଁ(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iSize));
    ପାଇଁ(pFile=pGroup->pFiles; pFile; pFile=pFile->pNext){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ଅକ୍ଷର zTemp[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(ଆକାର(zTemp), zTemp, "%s", pFile->zFilename);
      ପାଇଁ(i=0; zTemp[i]; i++){ ଯଦି( zTemp[i]=='\\' ) zTemp[i] = '/'; }
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
  ଫେରନ୍ତୁ TCL_OK;
}

/* Defined in test1.c */
ବାହ୍ୟ ଶୂନ୍ୟ *sqlite3TestTextToPtr(ଅଚଳ ଅକ୍ଷର*);
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3TestPtrToText(ଶୂନ୍ୟ*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_fopen(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *zFilename;          /* File pattern to configure */
  ଅଚଳ ଅକ୍ଷର *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "FILENAME MODE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_SetResult(interp, (ଅକ୍ଷର*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_fread(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ଅକ୍ଷର *zBuf;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nElem;
  size_t got;

  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE SIZE NELEM");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ଫେରନ୍ତୁ TCL_ERROR;
  zBuf = (ଅକ୍ଷର*)sqlite3_malloc( sz*nElem + 1 );
  ଯଦି( zBuf==0 ){
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_SetResult(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_fwrite(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ଅକ୍ଷର *zBuf;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nElem;
  size_t got;

  ଯଦି( objc!=5 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ଫେରନ୍ତୁ TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(got));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_fclose(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_fflush(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା doSync = 0;

  ଯଦି( objc!=2 && objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE ?HARDSYNC?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ଯଦି( objc==3 ){
    ଯଦି( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) ଫେରନ୍ତୁ TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_fseek(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst;
  ଅଚଳ ଅକ୍ଷର *zWhence;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା whence;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE OFFSET WHENCE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) ଫେରନ୍ତୁ TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  ଯଦି( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }ଅନ୍ୟଥା ଯଦି( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }ଅନ୍ୟଥା ଯଦି( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }ଅନ୍ୟଥା{
    Tcl_AppendResult(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_rewind(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_ftell(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_ftruncate(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE SIZE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  ଯଦି( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) ଫେରନ୍ତୁ TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_file_size(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_size(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_file_truesize(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_file_mtime(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  time_t t;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(t));
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_remove(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *zFilename;          /* File pattern to configure */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "FILENAME");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_glob(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *zPattern;          /* The glob pattern */
  ଅଚଳ ଅକ୍ଷର *zText;             /* Text to compare against the pattern */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "PATTERN TEXT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_file_available(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_quota_ferror(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(x));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitequota_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; i<ଆକାର(aCmd)/ଆକାର(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ଫେରନ୍ତୁ TCL_OK;
}
#endif
