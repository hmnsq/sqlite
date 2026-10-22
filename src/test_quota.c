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
#അന്തര്ഭാവ "test_quota.h"
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#എങ്കിൽ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)
#define sqlite3_mutex_held(X)     ((ശൂന്യം)(X),1)
#define sqlite3_mutex_notheld(X)  ((ശൂന്യം)(X),1)
#endif /* SQLITE_THREADSAFE==0 */


#ifdef _WIN32
# include <windows.h>
# include <io.h>
#അല്ലെങ്കിൽ
# include <unistd.h>
#endif


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
തരനിർവചനം ഘടന quotaGroup quotaGroup;
തരനിർവചനം ഘടന quotaConn quotaConn;
തരനിർവചനം ഘടന quotaFile quotaFile;

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
ഘടന quotaGroup {
  സ്ഥിരം അക്ഷരം *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 iSize;           /* Current size of all files */
  ശൂന്യം (*xCallback)(             /* Callback invoked when going over quota */
     സ്ഥിരം അക്ഷരം *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     ശൂന്യം *pArg                     /* Client data */
  );
  ശൂന്യം *pArg;                    /* Third argument to the xCallback() */
  ശൂന്യം (*xDestroy)(ശൂന്യം*);       /* Optional destructor for pArg */
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
ഘടന quotaFile {
  അക്ഷരം *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 iSize;            /* Current size of this file */
  പൂർണ്ണസംഖ്യ nRef;                       /* Number of times this file is open */
  പൂർണ്ണസംഖ്യ deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *pNext, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ഘടന quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
ഘടന quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ifdef _WIN32
  അക്ഷരം *zMbcsName;        /* Full MBCS pathname of the file */
#endif
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
സ്ഥിതം ഘടന {
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
  പൂർണ്ണസംഖ്യ isInitialized;

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
സ്ഥിതം ശൂന്യം quotaEnter(ശൂന്യം){ sqlite3_mutex_enter(gQuota.pMutex); }
സ്ഥിതം ശൂന്യം quotaLeave(ശൂന്യം){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quota_Group_Open_File_എണ്ണം(quotaGroup *pGroup){
  പൂർണ്ണസംഖ്യ N = 0;
  quotaFile *pFile = pGroup->pFiles;
  ആകുമ്പോൾ( pFile ){
    എങ്കിൽ( pFile->nRef ) N++;
    pFile = pFile->pNext;
  }
  തിരികെ N;
}

/* Remove a file from a quota group.
*/
സ്ഥിതം ശൂന്യം quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->iSize -= pFile->iSize;
  *pFile->ppPrev = pFile->pNext;
  എങ്കിൽ( pFile->pNext ) pFile->pNext->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
സ്ഥിതം ശൂന്യം quotaRemoveAllFiles(quotaGroup *pGroup){
  ആകുമ്പോൾ( pGroup->pFiles ){
    പ്രതിജ്ഞാ( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
സ്ഥിതം ശൂന്യം quotaGroupDeref(quotaGroup *pGroup){
  എങ്കിൽ( pGroup->iLimit==0 && quota_Group_Open_File_എണ്ണം(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->ppPrev = pGroup->pNext;
    എങ്കിൽ( pGroup->pNext ) pGroup->pNext->ppPrev = pGroup->ppPrev;
    എങ്കിൽ( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaStrglob(സ്ഥിരം അക്ഷരം *zGlob, സ്ഥിരം അക്ഷരം *z){
  പൂർണ്ണസംഖ്യ c, c2, cx;
  പൂർണ്ണസംഖ്യ invert;
  പൂർണ്ണസംഖ്യ seen;

  ആകുമ്പോൾ( (c = (*(zGlob++)))!=0 ){
    എങ്കിൽ( c=='*' ){
      ആകുമ്പോൾ( (c=(*(zGlob++))) == '*' || c=='?' ){
        എങ്കിൽ( c=='?' && (*(z++))==0 ) തിരികെ 0;
      }
      എങ്കിൽ( c==0 ){
        തിരികെ 1;
      }അല്ലെങ്കിൽ എങ്കിൽ( c=='[' ){
        ആകുമ്പോൾ( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        തിരികെ (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      ആകുമ്പോൾ( (c2 = (*(z++)))!=0 ){
        ആകുമ്പോൾ( c2!=c && c2!=cx ){
          c2 = *(z++);
          എങ്കിൽ( c2==0 ) തിരികെ 0;
        }
        എങ്കിൽ( quotaStrglob(zGlob,z) ) തിരികെ 1;
      }
      തിരികെ 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( c=='?' ){
      എങ്കിൽ( (*(z++))==0 ) തിരികെ 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( c=='[' ){
      പൂർണ്ണസംഖ്യ prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      എങ്കിൽ( c==0 ) തിരികെ 0;
      c2 = *(zGlob++);
      എങ്കിൽ( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      എങ്കിൽ( c2==']' ){
        എങ്കിൽ( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      ആകുമ്പോൾ( c2 && c2!=']' ){
        എങ്കിൽ( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          എങ്കിൽ( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }അല്ലെങ്കിൽ{
          എങ്കിൽ( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      എങ്കിൽ( c2==0 || (seen ^ invert)==0 ) തിരികെ 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( c=='/' ){
      എങ്കിൽ( z[0]!='/' && z[0]!='\\' ) തിരികെ 0;
      z++;
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( c!=(*(z++)) ) തിരികെ 0;
    }
  }
  തിരികെ *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
സ്ഥിതം quotaGroup *quotaGroupFind(സ്ഥിരം അക്ഷരം *zFilename){
  quotaGroup *p;
  വേണ്ടി(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->pNext){}
  തിരികെ p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
സ്ഥിതം sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  തിരികെ (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
സ്ഥിതം quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  സ്ഥിരം അക്ഷരം *zName,      /* Full pathname of the file */
  പൂർണ്ണസംഖ്യ createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  ആകുമ്പോൾ( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->pNext;
  }
  എങ്കിൽ( pFile==0 && createFlag ){
    പൂർണ്ണസംഖ്യ nName = (പൂർണ്ണസംഖ്യ)(strlen(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( വലുപ്പം(*pFile) + nName + 1 );
    എങ്കിൽ( pFile ){
      memset(pFile, 0, വലുപ്പം(*pFile));
      pFile->zFilename = (അക്ഷരം*)&pFile[1];
      memcpy(pFile->zFilename, zName, nName+1);
      pFile->pNext = pGroup->pFiles;
      എങ്കിൽ( pGroup->pFiles ) pGroup->pFiles->ppPrev = &pFile->pNext;
      pFile->ppPrev = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  തിരികെ pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
സ്ഥിതം അക്ഷരം *quota_utf8_to_mbcs(സ്ഥിരം അക്ഷരം *zUtf8){
#ifdef _WIN32
  size_t n;          /* Bytes in zUtf8 */
  പൂർണ്ണസംഖ്യ nWide;         /* number of UTF-16 characters */
  പൂർണ്ണസംഖ്യ nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  അക്ഷരം *zMbcs;       /* The MBCS text */
  പൂർണ്ണസംഖ്യ codepage;      /* Code page used by fopen() */

  n = strlen(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, ശൂന്യ, 0);
  എങ്കിൽ( nWide==0 ) തിരികെ 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*വലുപ്പം(zTmpWide[0]) );
  എങ്കിൽ( zTmpWide==0 ) തിരികെ 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (അക്ഷരം*)sqlite3_malloc( nMbcs+1 ) : 0;
  എങ്കിൽ( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  തിരികെ zMbcs;
#അല്ലെങ്കിൽ
  തിരികെ (അക്ഷരം*)zUtf8;  /* No-op on unix */
#endif
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
സ്ഥിതം ശൂന്യം quota_mbcs_free(അക്ഷരം *zOld){
#ifdef _WIN32
  sqlite3_free(zOld);
#അല്ലെങ്കിൽ
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
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  സ്ഥിരം അക്ഷരം *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  പൂർണ്ണസംഖ്യ flags,                  /* Flags to control the opening */
  പൂർണ്ണസംഖ്യ *pOutFlags              /* Flags showing results of opening */
){
  പൂർണ്ണസംഖ്യ rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  എങ്കിൽ( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    തിരികെ pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  എങ്കിൽ( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }അല്ലെങ്കിൽ{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    എങ്കിൽ( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      എങ്കിൽ( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        തിരികെ SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      എങ്കിൽ( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }അല്ലെങ്കിൽ{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  തിരികെ rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  സ്ഥിരം അക്ഷരം *zName,          /* Name of file to be deleted */
  പൂർണ്ണസംഖ്യ syncDir                 /* Do a directory sync after deleting */
){
  പൂർണ്ണസംഖ്യ rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  എങ്കിൽ( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    എങ്കിൽ( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      എങ്കിൽ( pFile ){
        എങ്കിൽ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }അല്ലെങ്കിൽ{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  തിരികെ rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  പൂർണ്ണസംഖ്യ rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  എങ്കിൽ( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    എങ്കിൽ( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  തിരികെ rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaRead(
  sqlite3_file *pConn,
  ശൂന്യം *pBuf,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaWrite(
  sqlite3_file *pConn,
  സ്ഥിരം ശൂന്യം *pBuf,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  എങ്കിൽ( pFile->iSize<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    എങ്കിൽ( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      എങ്കിൽ( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      എങ്കിൽ( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        തിരികെ SQLITE_FULL;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }
  തിരികെ pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  പൂർണ്ണസംഖ്യ rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  എങ്കിൽ( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = size;
    pGroup->iSize += size;
    quotaLeave();
  }
  തിരികെ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaSync(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  പൂർണ്ണസംഖ്യ rc;

  rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz);
  എങ്കിൽ( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = sz;
    pGroup->iSize += sz;
    quotaLeave();
    *pSize = sz;
  }
  തിരികെ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaLock(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaUnlock(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaCheckReservedLock(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaFileControl(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  പൂർണ്ണസംഖ്യ rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#എങ്കിൽ defined(SQLITE_FCNTL_VFSNAME)
  എങ്കിൽ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(അക്ഷരം**)pArg = sqlite3_mprintf("quota/%z", *(അക്ഷരം**)pArg);
  }
#endif
  തിരികെ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaSectorSize(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xSectorSize(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  പൂർണ്ണസംഖ്യ iRegion,                    /* Region to retrieve */
  പൂർണ്ണസംഖ്യ szRegion,                   /* Size of regions */
  പൂർണ്ണസംഖ്യ bExtend,                    /* True to extend file if necessary */
  ശൂന്യം അസ്ഥിര **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  പൂർണ്ണസംഖ്യ ofst,                  /* First lock to acquire or release */
  പൂർണ്ണസംഖ്യ n,                     /* Number of locks to acquire or release */
  പൂർണ്ണസംഖ്യ flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
സ്ഥിതം ശൂന്യം quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ quotaShmUnmap(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  തിരികെ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
പൂർണ്ണസംഖ്യ sqlite3_quota_initialize(സ്ഥിരം അക്ഷരം *zOrigVfsName, പൂർണ്ണസംഖ്യ makeDefault){
  sqlite3_vfs *pOrigVfs;
  എങ്കിൽ( gQuota.isInitialized ) തിരികെ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  എങ്കിൽ( pOrigVfs==0 ) തിരികെ SQLITE_ERROR;
  പ്രതിജ്ഞാ( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  എങ്കിൽ( !gQuota.pMutex ){
    തിരികെ SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += വലുപ്പം(quotaConn);
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
  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3_quota_shutdown(ശൂന്യം){
  quotaGroup *pGroup;
  എങ്കിൽ( gQuota.isInitialized==0 ) തിരികെ SQLITE_MISUSE;
  വേണ്ടി(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    എങ്കിൽ( quota_Group_Open_File_എണ്ണം(pGroup)>0 ) തിരികെ SQLITE_MISUSE;
  }
  ആകുമ്പോൾ( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->pNext;
    pGroup->iLimit = 0;
    പ്രതിജ്ഞാ( quota_Group_Open_File_എണ്ണം(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, വലുപ്പം(gQuota));
  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3_quota_set(
  സ്ഥിരം അക്ഷരം *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  ശൂന്യം (*xCallback)(              /* Callback invoked when going over quota */
     സ്ഥിരം അക്ഷരം *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     ശൂന്യം *pArg                     /* Client data */
  ),
  ശൂന്യം *pArg,                     /* client data passed thru to callback */
  ശൂന്യം (*xDestroy)(ശൂന്യം*)         /* Optional destructor for pArg */
){
  quotaGroup *pGroup;
  quotaEnter();
  pGroup = gQuota.pGroup;
  ആകുമ്പോൾ( pGroup && strcmp(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->pNext;
  }
  എങ്കിൽ( pGroup==0 ){
    പൂർണ്ണസംഖ്യ nPattern = (പൂർണ്ണസംഖ്യ)(strlen(zPattern) & 0x3fffffff);
    എങ്കിൽ( iLimit<=0 ){
      quotaLeave();
      തിരികെ SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( വലുപ്പം(*pGroup) + nPattern + 1 );
    എങ്കിൽ( pGroup==0 ){
      quotaLeave();
      തിരികെ SQLITE_NOMEM;
    }
    memset(pGroup, 0, വലുപ്പം(*pGroup));
    pGroup->zPattern = (അക്ഷരം*)&pGroup[1];
    memcpy((അക്ഷരം *)pGroup->zPattern, zPattern, nPattern+1);
    എങ്കിൽ( gQuota.pGroup ) gQuota.pGroup->ppPrev = &pGroup->pNext;
    pGroup->pNext = gQuota.pGroup;
    pGroup->ppPrev = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  എങ്കിൽ( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  തിരികെ SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
പൂർണ്ണസംഖ്യ sqlite3_quota_file(സ്ഥിരം അക്ഷരം *zFilename){
  അക്ഷരം *zFull = 0;
  sqlite3_file *fd;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ outFlags = 0;
  sqlite3_int64 iSize;
  പൂർണ്ണസംഖ്യ nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  എങ്കിൽ( fd==0 ){
    rc = SQLITE_NOMEM;
  }അല്ലെങ്കിൽ{
    zFull = &((അക്ഷരം *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    zFull[strlen(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    എങ്കിൽ( rc==SQLITE_OK ){
      fd->pMethods->xFileSize(fd, &iSize);
      fd->pMethods->xClose(fd);
    }അല്ലെങ്കിൽ എങ്കിൽ( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      എങ്കിൽ( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        എങ്കിൽ( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  തിരികെ rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(സ്ഥിരം അക്ഷരം *zFilename, സ്ഥിരം അക്ഷരം *zMode){
  quota_FILE *p = 0;
  അക്ഷരം *zFull = 0;
  അക്ഷരം *zFullTranslated = 0;
  പൂർണ്ണസംഖ്യ rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (അക്ഷരം*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  എങ്കിൽ( zFull==0 ) തിരികെ 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  എങ്കിൽ( rc ) പോകുക quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(വലുപ്പം(*p));
  എങ്കിൽ( p==0 ) പോകുക quota_fopen_error;
  memset(p, 0, വലുപ്പം(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  എങ്കിൽ( zFullTranslated==0 ) പോകുക quota_fopen_error;
  p->f = fopen(zFullTranslated, zMode);
  എങ്കിൽ( p->f==0 ) പോകുക quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  എങ്കിൽ( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    എങ്കിൽ( pFile==0 ){
      quotaLeave();
      പോകുക quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(zFull);
#ifdef _WIN32
  p->zMbcsName = zFullTranslated;
#endif
  തിരികെ p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  എങ്കിൽ( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  തിരികെ 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  ശൂന്യം *pBuf,            /* Store the content here */
  size_t size,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  തിരികെ fread(pBuf, size, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  സ്ഥിരം ശൂന്യം *pBuf,      /* Take content to write from here */
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
  എങ്കിൽ( pFile && pFile->iSize<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    എങ്കിൽ( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      എങ്കിൽ( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      എങ്കിൽ( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->iSize + pFile->iSize;
        nmemb = (size_t)((iEnd - iOfst)/size);
        iEnd = iOfst + size*nmemb;
        szNew = pGroup->iSize - pFile->iSize + iEnd;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }അല്ലെങ്കിൽ{
    pFile = 0;
  }
  rc = fwrite(pBuf, size, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  എങ്കിൽ( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + size*nWritten;
    എങ്കിൽ( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->iSize += iNewEnd - pFile->iSize;
    pFile->iSize = iNewEnd;
    quotaLeave();
  }
  തിരികെ rc;
}

/*
** Close an open quota_FILE stream.
*/
പൂർണ്ണസംഖ്യ sqlite3_quota_fclose(quota_FILE *p){
  പൂർണ്ണസംഖ്യ rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  എങ്കിൽ( pFile ){
    quotaEnter();
    pFile->nRef--;
    എങ്കിൽ( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      എങ്കിൽ( pFile->deleteOnClose ){
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
  തിരികെ rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
പൂർണ്ണസംഖ്യ sqlite3_quota_fflush(quota_FILE *p, പൂർണ്ണസംഖ്യ doFsync){
  പൂർണ്ണസംഖ്യ rc;
  rc = fflush(p->f);
  എങ്കിൽ( rc==0 && doFsync ){
#ifdef _WIN32
    rc = _commit(_fileno(p->f));
#അല്ലെങ്കിൽ
    rc = fsync(fileno(p->f));
#endif
  }
  തിരികെ rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
പൂർണ്ണസംഖ്യ sqlite3_quota_fseek(quota_FILE *p, long offset, പൂർണ്ണസംഖ്യ whence){
  തിരികെ fseek(p->f, offset, whence);
}

/*
** rewind a quota_FILE stream.
*/
ശൂന്യം sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
long sqlite3_quota_ftell(quota_FILE *p){
  തിരികെ ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
പൂർണ്ണസംഖ്യ sqlite3_quota_ferror(quota_FILE *p){
  തിരികെ ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
പൂർണ്ണസംഖ്യ sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( (pFile = p->pFile)!=0 && pFile->iSize<szNew ){
    quotaGroup *pGroup;
    എങ്കിൽ( pFile->iSize<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      തിരികെ -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    quotaLeave();
  }
#ifdef _WIN32
#  എങ്കിൽ defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (long)szNew);
#  അല്ലെങ്കിൽ
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#അല്ലെങ്കിൽ
  rc = ftruncate(fileno(p->f), szNew);
#endif
  എങ്കിൽ( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    pFile->iSize = szNew;
    quotaLeave();
  }
  തിരികെ rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
പൂർണ്ണസംഖ്യ sqlite3_quota_file_mtime(quota_FILE *p, time_t *pTime){
  പൂർണ്ണസംഖ്യ rc;
#ifdef _WIN32
  ഘടന _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#അല്ലെങ്കിൽ
  ഘടന stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  എങ്കിൽ( rc==0 ) *pTime = buf.st_mtime;
  തിരികെ rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  പൂർണ്ണസംഖ്യ rc;
#ifdef _WIN32
  ഘടന _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#അല്ലെങ്കിൽ
  ഘടന stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  തിരികെ rc==0 ? buf.st_size : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_size(quota_FILE *p){
  തിരികെ p->pFile ? p->pFile->iSize : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
long sqlite3_quota_file_available(quota_FILE *p){
  FILE* f = p->f;
  long pos1, pos2;
  പൂർണ്ണസംഖ്യ rc;
  pos1 = ftell(f);
  എങ്കിൽ ( pos1 < 0 ) തിരികെ -1;
  rc = fseek(f, 0, SEEK_END);
  എങ്കിൽ ( rc != 0 ) തിരികെ -1;
  pos2 = ftell(f);
  എങ്കിൽ ( pos2 < 0 ) തിരികെ -1;
  rc = fseek(f, pos1, SEEK_SET);
  എങ്കിൽ ( rc != 0 ) തിരികെ -1;
  തിരികെ pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
പൂർണ്ണസംഖ്യ sqlite3_quota_remove(സ്ഥിരം അക്ഷരം *zFilename){
  അക്ഷരം *zFull;            /* Full pathname for zFilename */
  size_t nFull;           /* Number of bytes in zFilename */
  പൂർണ്ണസംഖ്യ rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *pNextFile;   /* next file in the group */
  പൂർണ്ണസംഖ്യ diff;               /* Difference between filenames */
  അക്ഷരം c;                 /* First character past end of pattern */

  zFull = (അക്ഷരം*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  എങ്കിൽ( zFull==0 ) തിരികെ SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  എങ്കിൽ( rc ){
    sqlite3_free(zFull);
    തിരികെ rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = strlen(zFull);
  എങ്കിൽ( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  എങ്കിൽ( pGroup ){
    വേണ്ടി(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=pNextFile){
      pNextFile = pFile->pNext;
      diff = strncmp(zFull, pFile->zFilename, nFull);
      എങ്കിൽ( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        എങ്കിൽ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }അല്ലെങ്കിൽ{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(zFull);
  തിരികെ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#അന്തര്ഭാവ "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
തരനിർവചനം ഘടന TclQuotaCallback TclQuotaCallback;
ഘടന TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);


/*
** This is the callback from a quota-over-limit.
*/
സ്ഥിതം ശൂന്യം tclQuotaCallback(
  സ്ഥിരം അക്ഷരം *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 iSize,            /* Total size of all files in the group */
  ശൂന്യം *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  unsigned പൂർണ്ണസംഖ്യ rnd;               /* Random part of pVarname */
  പൂർണ്ണസംഖ്യ rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  എങ്കിൽ( p==0 ) തിരികെ;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_എണ്ണം(pVarname);
  sqlite3_randomness(വലുപ്പം(rnd), (ശൂന്യം *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((പൂർണ്ണസംഖ്യ)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_എണ്ണം(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(iSize));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  എങ്കിൽ( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_എണ്ണം(pEval);
  Tcl_Decr_Ref_എണ്ണം(pVarname);
  എങ്കിൽ( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
സ്ഥിതം ശൂന്യം tclCallbackDestructor(ശൂന്യം *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  എങ്കിൽ( p ){
    Tcl_Decr_Ref_എണ്ണം(p->pScript);
    sqlite3_free((അക്ഷരം *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_initialize(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *zName;              /* Name of new quota VFS */
  പൂർണ്ണസംഖ്യ makeDefault;                /* True to make the new VFS the default */
  പൂർണ്ണസംഖ്യ rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    തിരികെ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);

  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_shutdown(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;                         /* Value returned by quota_shutdown() */

  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);

  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_set(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  പൂർണ്ണസംഖ്യ rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_Size nScript;               /* Length of callback script */
  ശൂന്യം (*xDestroy)(ശൂന്യം*);        /* Optional destructor for pArg */
  ശൂന്യം (*xCallback)(സ്ഥിരം അക്ഷരം *, sqlite3_int64 *, sqlite3_int64, ശൂന്യം *);

  /* Process arguments */
  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    തിരികെ TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  എങ്കിൽ( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) തിരികെ TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  എങ്കിൽ( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(വലുപ്പം(TclQuotaCallback));
    എങ്കിൽ( !p ){
      Tcl_SetResult(interp, (അക്ഷരം *)"SQLITE_NOMEM", TCL_STATIC);
      തിരികെ TCL_OK;
    }
    memset(p, 0, വലുപ്പം(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_എണ്ണം(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }അല്ലെങ്കിൽ{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (ശൂന്യം*)p, xDestroy);

  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_file(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *zFilename;          /* File pattern to configure */
  പൂർണ്ണസംഖ്യ rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "FILENAME");
    തിരികെ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);
  തിരികെ TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_dump(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pResult;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  pResult = Tcl_NewObj();
  quotaEnter();
  വേണ്ടി(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iSize));
    വേണ്ടി(pFile=pGroup->pFiles; pFile; pFile=pFile->pNext){
      പൂർണ്ണസംഖ്യ i;
      അക്ഷരം zTemp[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(വലുപ്പം(zTemp), zTemp, "%s", pFile->zFilename);
      വേണ്ടി(i=0; zTemp[i]; i++){ എങ്കിൽ( zTemp[i]=='\\' ) zTemp[i] = '/'; }
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
  തിരികെ TCL_OK;
}

/* Defined in test1.c */
ബാഹ്യ ശൂന്യം *sqlite3TestTextToPtr(സ്ഥിരം അക്ഷരം*);
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3TestPtrToText(ശൂന്യം*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_fopen(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *zFilename;          /* File pattern to configure */
  സ്ഥിരം അക്ഷരം *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "FILENAME MODE");
    തിരികെ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_SetResult(interp, (അക്ഷരം*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_fread(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  അക്ഷരം *zBuf;
  പൂർണ്ണസംഖ്യ sz;
  പൂർണ്ണസംഖ്യ nElem;
  size_t got;

  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE SIZE NELEM");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) തിരികെ TCL_ERROR;
  zBuf = (അക്ഷരം*)sqlite3_malloc( sz*nElem + 1 );
  എങ്കിൽ( zBuf==0 ){
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
    തിരികെ TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_SetResult(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_fwrite(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  അക്ഷരം *zBuf;
  പൂർണ്ണസംഖ്യ sz;
  പൂർണ്ണസംഖ്യ nElem;
  size_t got;

  എങ്കിൽ( objc!=5 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) തിരികെ TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(got));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_fclose(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_fflush(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ doSync = 0;

  എങ്കിൽ( objc!=2 && objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE ?HARDSYNC?");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  എങ്കിൽ( objc==3 ){
    എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) തിരികെ TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_fseek(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  പൂർണ്ണസംഖ്യ ofst;
  സ്ഥിരം അക്ഷരം *zWhence;
  പൂർണ്ണസംഖ്യ whence;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE OFFSET WHENCE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) തിരികെ TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  എങ്കിൽ( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }അല്ലെങ്കിൽ{
    Tcl_AppendResult(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_rewind(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_ftell(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_ftruncate(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE SIZE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  എങ്കിൽ( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) തിരികെ TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_file_size(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_size(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_file_truesize(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_file_mtime(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  time_t t;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(t));
  തിരികെ TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_remove(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *zFilename;          /* File pattern to configure */
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "FILENAME");
    തിരികെ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_glob(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *zPattern;          /* The glob pattern */
  സ്ഥിരം അക്ഷരം *zText;             /* Text to compare against the pattern */
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "PATTERN TEXT");
    തിരികെ TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_file_available(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_quota_ferror(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  പൂർണ്ണസംഖ്യ x;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE");
    തിരികെ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(x));
  തിരികെ TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
പൂർണ്ണസംഖ്യ Sqlitequota_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
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
  പൂർണ്ണസംഖ്യ i;

  വേണ്ടി(i=0; i<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  തിരികെ TCL_OK;
}
#endif
