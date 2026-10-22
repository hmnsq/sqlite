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
#অন্তর্ভাব "test_quota.h"
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#যদি defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)
#define sqlite3_mutex_held(X)     ((শূন্য)(X),1)
#define sqlite3_mutex_notheld(X)  ((শূন্য)(X),1)
#endif /* SQLITE_THREADSAFE==0 */


#ifdef _WIN32
# include <windows.h>
# include <io.h>
#অন্যথা
# include <unistd.h>
#endif


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
প্রকারনির্দেশ সংরচনা quotaGroup quotaGroup;
প্রকারনির্দেশ সংরচনা quotaConn quotaConn;
প্রকারনির্দেশ সংরচনা quotaFile quotaFile;

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
সংরচনা quotaGroup {
  ধ্রুব বর্ণ *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 iSize;           /* Current size of all files */
  শূন্য (*xCallback)(             /* Callback invoked when going over quota */
     ধ্রুব বর্ণ *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     শূন্য *pArg                     /* Client data */
  );
  শূন্য *pArg;                    /* Third argument to the xCallback() */
  শূন্য (*xDestroy)(শূন্য*);       /* Optional destructor for pArg */
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
সংরচনা quotaFile {
  বর্ণ *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 iSize;            /* Current size of this file */
  পূর্ণাঙ্ক nRef;                       /* Number of times this file is open */
  পূর্ণাঙ্ক deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *pNext, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
সংরচনা quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
সংরচনা quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ifdef _WIN32
  বর্ণ *zMbcsName;        /* Full MBCS pathname of the file */
#endif
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
স্থির সংরচনা {
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
  পূর্ণাঙ্ক isInitialized;

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
স্থির শূন্য quotaEnter(শূন্য){ sqlite3_mutex_enter(gQuota.pMutex); }
স্থির শূন্য quotaLeave(শূন্য){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
স্থির পূর্ণাঙ্ক quota_Group_Open_File_গুনতি(quotaGroup *pGroup){
  পূর্ণাঙ্ক N = 0;
  quotaFile *pFile = pGroup->pFiles;
  যাবত্( pFile ){
    যদি( pFile->nRef ) N++;
    pFile = pFile->pNext;
  }
  প্রত্যাগম N;
}

/* Remove a file from a quota group.
*/
স্থির শূন্য quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->iSize -= pFile->iSize;
  *pFile->ppPrev = pFile->pNext;
  যদি( pFile->pNext ) pFile->pNext->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
স্থির শূন্য quotaRemoveAllFiles(quotaGroup *pGroup){
  যাবত্( pGroup->pFiles ){
    প্রতিজ্ঞা( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
স্থির শূন্য quotaGroupDeref(quotaGroup *pGroup){
  যদি( pGroup->iLimit==0 && quota_Group_Open_File_গুনতি(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->ppPrev = pGroup->pNext;
    যদি( pGroup->pNext ) pGroup->pNext->ppPrev = pGroup->ppPrev;
    যদি( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
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
স্থির পূর্ণাঙ্ক quotaStrglob(ধ্রুব বর্ণ *zGlob, ধ্রুব বর্ণ *z){
  পূর্ণাঙ্ক c, c2, cx;
  পূর্ণাঙ্ক invert;
  পূর্ণাঙ্ক seen;

  যাবত্( (c = (*(zGlob++)))!=0 ){
    যদি( c=='*' ){
      যাবত্( (c=(*(zGlob++))) == '*' || c=='?' ){
        যদি( c=='?' && (*(z++))==0 ) প্রত্যাগম 0;
      }
      যদি( c==0 ){
        প্রত্যাগম 1;
      }অন্যথা যদি( c=='[' ){
        যাবত্( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        প্রত্যাগম (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      যাবত্( (c2 = (*(z++)))!=0 ){
        যাবত্( c2!=c && c2!=cx ){
          c2 = *(z++);
          যদি( c2==0 ) প্রত্যাগম 0;
        }
        যদি( quotaStrglob(zGlob,z) ) প্রত্যাগম 1;
      }
      প্রত্যাগম 0;
    }অন্যথা যদি( c=='?' ){
      যদি( (*(z++))==0 ) প্রত্যাগম 0;
    }অন্যথা যদি( c=='[' ){
      পূর্ণাঙ্ক prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      যদি( c==0 ) প্রত্যাগম 0;
      c2 = *(zGlob++);
      যদি( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      যদি( c2==']' ){
        যদি( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      যাবত্( c2 && c2!=']' ){
        যদি( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          যদি( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }অন্যথা{
          যদি( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      যদি( c2==0 || (seen ^ invert)==0 ) প্রত্যাগম 0;
    }অন্যথা যদি( c=='/' ){
      যদি( z[0]!='/' && z[0]!='\\' ) প্রত্যাগম 0;
      z++;
    }অন্যথা{
      যদি( c!=(*(z++)) ) প্রত্যাগম 0;
    }
  }
  প্রত্যাগম *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
স্থির quotaGroup *quotaGroupFind(ধ্রুব বর্ণ *zFilename){
  quotaGroup *p;
  প্রতি(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->pNext){}
  প্রত্যাগম p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
স্থির sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  প্রত্যাগম (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
স্থির quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  ধ্রুব বর্ণ *zName,      /* Full pathname of the file */
  পূর্ণাঙ্ক createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  যাবত্( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->pNext;
  }
  যদি( pFile==0 && createFlag ){
    পূর্ণাঙ্ক nName = (পূর্ণাঙ্ক)(strlen(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( পরিমাণ(*pFile) + nName + 1 );
    যদি( pFile ){
      memset(pFile, 0, পরিমাণ(*pFile));
      pFile->zFilename = (বর্ণ*)&pFile[1];
      memcpy(pFile->zFilename, zName, nName+1);
      pFile->pNext = pGroup->pFiles;
      যদি( pGroup->pFiles ) pGroup->pFiles->ppPrev = &pFile->pNext;
      pFile->ppPrev = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  প্রত্যাগম pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
স্থির বর্ণ *quota_utf8_to_mbcs(ধ্রুব বর্ণ *zUtf8){
#ifdef _WIN32
  size_t n;          /* Bytes in zUtf8 */
  পূর্ণাঙ্ক nWide;         /* number of UTF-16 characters */
  পূর্ণাঙ্ক nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  বর্ণ *zMbcs;       /* The MBCS text */
  পূর্ণাঙ্ক codepage;      /* Code page used by fopen() */

  n = strlen(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, রিক্ত, 0);
  যদি( nWide==0 ) প্রত্যাগম 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*পরিমাণ(zTmpWide[0]) );
  যদি( zTmpWide==0 ) প্রত্যাগম 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (বর্ণ*)sqlite3_malloc( nMbcs+1 ) : 0;
  যদি( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  প্রত্যাগম zMbcs;
#অন্যথা
  প্রত্যাগম (বর্ণ*)zUtf8;  /* No-op on unix */
#endif
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
স্থির শূন্য quota_mbcs_free(বর্ণ *zOld){
#ifdef _WIN32
  sqlite3_free(zOld);
#অন্যথা
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
স্থির পূর্ণাঙ্ক quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ধ্রুব বর্ণ *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  পূর্ণাঙ্ক flags,                  /* Flags to control the opening */
  পূর্ণাঙ্ক *pOutFlags              /* Flags showing results of opening */
){
  পূর্ণাঙ্ক rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  যদি( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    প্রত্যাগম pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  যদি( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }অন্যথা{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    যদি( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      যদি( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        প্রত্যাগম SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      যদি( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }অন্যথা{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  প্রত্যাগম rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
স্থির পূর্ণাঙ্ক quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ধ্রুব বর্ণ *zName,          /* Name of file to be deleted */
  পূর্ণাঙ্ক syncDir                 /* Do a directory sync after deleting */
){
  পূর্ণাঙ্ক rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  যদি( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    যদি( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      যদি( pFile ){
        যদি( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }অন্যথা{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  প্রত্যাগম rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
স্থির পূর্ণাঙ্ক quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  পূর্ণাঙ্ক rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  যদি( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    যদি( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  প্রত্যাগম rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
স্থির পূর্ণাঙ্ক quotaRead(
  sqlite3_file *pConn,
  শূন্য *pBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
স্থির পূর্ণাঙ্ক quotaWrite(
  sqlite3_file *pConn,
  ধ্রুব শূন্য *pBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  যদি( pFile->iSize<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    যদি( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      যদি( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      যদি( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        প্রত্যাগম SQLITE_FULL;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }
  প্রত্যাগম pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
স্থির পূর্ণাঙ্ক quotaTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  পূর্ণাঙ্ক rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  যদি( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = size;
    pGroup->iSize += size;
    quotaLeave();
  }
  প্রত্যাগম rc;
}

/* Pass xSync requests through to the original VFS without change
*/
স্থির পূর্ণাঙ্ক quotaSync(sqlite3_file *pConn, পূর্ণাঙ্ক flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
স্থির পূর্ণাঙ্ক quotaFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  পূর্ণাঙ্ক rc;

  rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz);
  যদি( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = sz;
    pGroup->iSize += sz;
    quotaLeave();
    *pSize = sz;
  }
  প্রত্যাগম rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaLock(sqlite3_file *pConn, পূর্ণাঙ্ক lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaUnlock(sqlite3_file *pConn, পূর্ণাঙ্ক lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaCheckReservedLock(sqlite3_file *pConn, পূর্ণাঙ্ক *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaFileControl(sqlite3_file *pConn, পূর্ণাঙ্ক op, শূন্য *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  পূর্ণাঙ্ক rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#যদি defined(SQLITE_FCNTL_VFSNAME)
  যদি( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(বর্ণ**)pArg = sqlite3_mprintf("quota/%z", *(বর্ণ**)pArg);
  }
#endif
  প্রত্যাগম rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaSectorSize(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xSectorSize(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  পূর্ণাঙ্ক iRegion,                    /* Region to retrieve */
  পূর্ণাঙ্ক szRegion,                   /* Size of regions */
  পূর্ণাঙ্ক bExtend,                    /* True to extend file if necessary */
  শূন্য চঞ্চল **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  পূর্ণাঙ্ক ofst,                  /* First lock to acquire or release */
  পূর্ণাঙ্ক n,                     /* Number of locks to acquire or release */
  পূর্ণাঙ্ক flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
স্থির শূন্য quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক quotaShmUnmap(sqlite3_file *pConn, পূর্ণাঙ্ক deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  প্রত্যাগম pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
পূর্ণাঙ্ক sqlite3_quota_initialize(ধ্রুব বর্ণ *zOrigVfsName, পূর্ণাঙ্ক makeDefault){
  sqlite3_vfs *pOrigVfs;
  যদি( gQuota.isInitialized ) প্রত্যাগম SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  যদি( pOrigVfs==0 ) প্রত্যাগম SQLITE_ERROR;
  প্রতিজ্ঞা( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  যদি( !gQuota.pMutex ){
    প্রত্যাগম SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += পরিমাণ(quotaConn);
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
  প্রত্যাগম SQLITE_OK;
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
পূর্ণাঙ্ক sqlite3_quota_shutdown(শূন্য){
  quotaGroup *pGroup;
  যদি( gQuota.isInitialized==0 ) প্রত্যাগম SQLITE_MISUSE;
  প্রতি(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    যদি( quota_Group_Open_File_গুনতি(pGroup)>0 ) প্রত্যাগম SQLITE_MISUSE;
  }
  যাবত্( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->pNext;
    pGroup->iLimit = 0;
    প্রতিজ্ঞা( quota_Group_Open_File_গুনতি(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, পরিমাণ(gQuota));
  প্রত্যাগম SQLITE_OK;
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
পূর্ণাঙ্ক sqlite3_quota_set(
  ধ্রুব বর্ণ *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  শূন্য (*xCallback)(              /* Callback invoked when going over quota */
     ধ্রুব বর্ণ *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     শূন্য *pArg                     /* Client data */
  ),
  শূন্য *pArg,                     /* client data passed thru to callback */
  শূন্য (*xDestroy)(শূন্য*)         /* Optional destructor for pArg */
){
  quotaGroup *pGroup;
  quotaEnter();
  pGroup = gQuota.pGroup;
  যাবত্( pGroup && strcmp(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->pNext;
  }
  যদি( pGroup==0 ){
    পূর্ণাঙ্ক nPattern = (পূর্ণাঙ্ক)(strlen(zPattern) & 0x3fffffff);
    যদি( iLimit<=0 ){
      quotaLeave();
      প্রত্যাগম SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( পরিমাণ(*pGroup) + nPattern + 1 );
    যদি( pGroup==0 ){
      quotaLeave();
      প্রত্যাগম SQLITE_NOMEM;
    }
    memset(pGroup, 0, পরিমাণ(*pGroup));
    pGroup->zPattern = (বর্ণ*)&pGroup[1];
    memcpy((বর্ণ *)pGroup->zPattern, zPattern, nPattern+1);
    যদি( gQuota.pGroup ) gQuota.pGroup->ppPrev = &pGroup->pNext;
    pGroup->pNext = gQuota.pGroup;
    pGroup->ppPrev = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  যদি( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  প্রত্যাগম SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
পূর্ণাঙ্ক sqlite3_quota_file(ধ্রুব বর্ণ *zFilename){
  বর্ণ *zFull = 0;
  sqlite3_file *fd;
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক outFlags = 0;
  sqlite3_int64 iSize;
  পূর্ণাঙ্ক nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  যদি( fd==0 ){
    rc = SQLITE_NOMEM;
  }অন্যথা{
    zFull = &((বর্ণ *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  যদি( rc==SQLITE_OK ){
    zFull[strlen(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    যদি( rc==SQLITE_OK ){
      fd->pMethods->xFileSize(fd, &iSize);
      fd->pMethods->xClose(fd);
    }অন্যথা যদি( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      যদি( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        যদি( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  প্রত্যাগম rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(ধ্রুব বর্ণ *zFilename, ধ্রুব বর্ণ *zMode){
  quota_FILE *p = 0;
  বর্ণ *zFull = 0;
  বর্ণ *zFullTranslated = 0;
  পূর্ণাঙ্ক rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (বর্ণ*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  যদি( zFull==0 ) প্রত্যাগম 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  যদি( rc ) গচ্ছ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(পরিমাণ(*p));
  যদি( p==0 ) গচ্ছ quota_fopen_error;
  memset(p, 0, পরিমাণ(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  যদি( zFullTranslated==0 ) গচ্ছ quota_fopen_error;
  p->f = fopen(zFullTranslated, zMode);
  যদি( p->f==0 ) গচ্ছ quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  যদি( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    যদি( pFile==0 ){
      quotaLeave();
      গচ্ছ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(zFull);
#ifdef _WIN32
  p->zMbcsName = zFullTranslated;
#endif
  প্রত্যাগম p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  যদি( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  প্রত্যাগম 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  শূন্য *pBuf,            /* Store the content here */
  size_t size,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  প্রত্যাগম fread(pBuf, size, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  ধ্রুব শূন্য *pBuf,      /* Take content to write from here */
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
  যদি( pFile && pFile->iSize<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    যদি( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      যদি( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      যদি( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->iSize + pFile->iSize;
        nmemb = (size_t)((iEnd - iOfst)/size);
        iEnd = iOfst + size*nmemb;
        szNew = pGroup->iSize - pFile->iSize + iEnd;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }অন্যথা{
    pFile = 0;
  }
  rc = fwrite(pBuf, size, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  যদি( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + size*nWritten;
    যদি( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->iSize += iNewEnd - pFile->iSize;
    pFile->iSize = iNewEnd;
    quotaLeave();
  }
  প্রত্যাগম rc;
}

/*
** Close an open quota_FILE stream.
*/
পূর্ণাঙ্ক sqlite3_quota_fclose(quota_FILE *p){
  পূর্ণাঙ্ক rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  যদি( pFile ){
    quotaEnter();
    pFile->nRef--;
    যদি( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      যদি( pFile->deleteOnClose ){
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
  প্রত্যাগম rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
পূর্ণাঙ্ক sqlite3_quota_fflush(quota_FILE *p, পূর্ণাঙ্ক doFsync){
  পূর্ণাঙ্ক rc;
  rc = fflush(p->f);
  যদি( rc==0 && doFsync ){
#ifdef _WIN32
    rc = _commit(_fileno(p->f));
#অন্যথা
    rc = fsync(fileno(p->f));
#endif
  }
  প্রত্যাগম rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
পূর্ণাঙ্ক sqlite3_quota_fseek(quota_FILE *p, long offset, পূর্ণাঙ্ক whence){
  প্রত্যাগম fseek(p->f, offset, whence);
}

/*
** rewind a quota_FILE stream.
*/
শূন্য sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
long sqlite3_quota_ftell(quota_FILE *p){
  প্রত্যাগম ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
পূর্ণাঙ্ক sqlite3_quota_ferror(quota_FILE *p){
  প্রত্যাগম ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
পূর্ণাঙ্ক sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  পূর্ণাঙ্ক rc;
  যদি( (pFile = p->pFile)!=0 && pFile->iSize<szNew ){
    quotaGroup *pGroup;
    যদি( pFile->iSize<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      প্রত্যাগম -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    quotaLeave();
  }
#ifdef _WIN32
#  যদি defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (long)szNew);
#  অন্যথা
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#অন্যথা
  rc = ftruncate(fileno(p->f), szNew);
#endif
  যদি( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    pFile->iSize = szNew;
    quotaLeave();
  }
  প্রত্যাগম rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
পূর্ণাঙ্ক sqlite3_quota_file_mtime(quota_FILE *p, time_t *pTime){
  পূর্ণাঙ্ক rc;
#ifdef _WIN32
  সংরচনা _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#অন্যথা
  সংরচনা stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  যদি( rc==0 ) *pTime = buf.st_mtime;
  প্রত্যাগম rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  পূর্ণাঙ্ক rc;
#ifdef _WIN32
  সংরচনা _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#অন্যথা
  সংরচনা stat buf;
  rc = fstat(fileno(p->f), &buf);
#endif
  প্রত্যাগম rc==0 ? buf.st_size : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_size(quota_FILE *p){
  প্রত্যাগম p->pFile ? p->pFile->iSize : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
long sqlite3_quota_file_available(quota_FILE *p){
  FILE* f = p->f;
  long pos1, pos2;
  পূর্ণাঙ্ক rc;
  pos1 = ftell(f);
  যদি ( pos1 < 0 ) প্রত্যাগম -1;
  rc = fseek(f, 0, SEEK_END);
  যদি ( rc != 0 ) প্রত্যাগম -1;
  pos2 = ftell(f);
  যদি ( pos2 < 0 ) প্রত্যাগম -1;
  rc = fseek(f, pos1, SEEK_SET);
  যদি ( rc != 0 ) প্রত্যাগম -1;
  প্রত্যাগম pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
পূর্ণাঙ্ক sqlite3_quota_remove(ধ্রুব বর্ণ *zFilename){
  বর্ণ *zFull;            /* Full pathname for zFilename */
  size_t nFull;           /* Number of bytes in zFilename */
  পূর্ণাঙ্ক rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *pNextFile;   /* next file in the group */
  পূর্ণাঙ্ক diff;               /* Difference between filenames */
  বর্ণ c;                 /* First character past end of pattern */

  zFull = (বর্ণ*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  যদি( zFull==0 ) প্রত্যাগম SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  যদি( rc ){
    sqlite3_free(zFull);
    প্রত্যাগম rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = strlen(zFull);
  যদি( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  যদি( pGroup ){
    প্রতি(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=pNextFile){
      pNextFile = pFile->pNext;
      diff = strncmp(zFull, pFile->zFilename, nFull);
      যদি( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        যদি( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }অন্যথা{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(zFull);
  প্রত্যাগম rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#অন্তর্ভাব "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
প্রকারনির্দেশ সংরচনা TclQuotaCallback TclQuotaCallback;
সংরচনা TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

বাহ্য ধ্রুব বর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);


/*
** This is the callback from a quota-over-limit.
*/
স্থির শূন্য tclQuotaCallback(
  ধ্রুব বর্ণ *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 iSize,            /* Total size of all files in the group */
  শূন্য *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  unsigned পূর্ণাঙ্ক rnd;               /* Random part of pVarname */
  পূর্ণাঙ্ক rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  যদি( p==0 ) প্রত্যাগম;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_গুনতি(pVarname);
  sqlite3_randomness(পরিমাণ(rnd), (শূন্য *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((পূর্ণাঙ্ক)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_গুনতি(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(iSize));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  যদি( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_গুনতি(pEval);
  Tcl_Decr_Ref_গুনতি(pVarname);
  যদি( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
স্থির শূন্য tclCallbackDestructor(শূন্য *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  যদি( p ){
    Tcl_Decr_Ref_গুনতি(p->pScript);
    sqlite3_free((বর্ণ *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_initialize(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুব বর্ণ *zName;              /* Name of new quota VFS */
  পূর্ণাঙ্ক makeDefault;                /* True to make the new VFS the default */
  পূর্ণাঙ্ক rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    প্রত্যাগম TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  যদি( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) প্রত্যাগম TCL_ERROR;
  যদি( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_STATIC);

  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_shutdown(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;                         /* Value returned by quota_shutdown() */

  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    প্রত্যাগম TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_STATIC);

  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_set(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুব বর্ণ *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  পূর্ণাঙ্ক rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_Size nScript;               /* Length of callback script */
  শূন্য (*xDestroy)(শূন্য*);        /* Optional destructor for pArg */
  শূন্য (*xCallback)(ধ্রুব বর্ণ *, sqlite3_int64 *, sqlite3_int64, শূন্য *);

  /* Process arguments */
  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    প্রত্যাগম TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  যদি( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) প্রত্যাগম TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  যদি( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(পরিমাণ(TclQuotaCallback));
    যদি( !p ){
      Tcl_SetResult(interp, (বর্ণ *)"SQLITE_NOMEM", TCL_STATIC);
      প্রত্যাগম TCL_OK;
    }
    memset(p, 0, পরিমাণ(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_গুনতি(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }অন্যথা{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (শূন্য*)p, xDestroy);

  Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_STATIC);
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_file(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুব বর্ণ *zFilename;          /* File pattern to configure */
  পূর্ণাঙ্ক rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME");
    প্রত্যাগম TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_SetResult(interp, (বর্ণ *)sqlite3ErrName(rc), TCL_STATIC);
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_dump(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pResult;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  pResult = Tcl_NewObj();
  quotaEnter();
  প্রতি(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iSize));
    প্রতি(pFile=pGroup->pFiles; pFile; pFile=pFile->pNext){
      পূর্ণাঙ্ক i;
      বর্ণ zTemp[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(পরিমাণ(zTemp), zTemp, "%s", pFile->zFilename);
      প্রতি(i=0; zTemp[i]; i++){ যদি( zTemp[i]=='\\' ) zTemp[i] = '/'; }
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
  প্রত্যাগম TCL_OK;
}

/* Defined in test1.c */
বাহ্য শূন্য *sqlite3TestTextToPtr(ধ্রুব বর্ণ*);
বাহ্য ধ্রুব বর্ণ *sqlite3TestPtrToText(শূন্য*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_fopen(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুব বর্ণ *zFilename;          /* File pattern to configure */
  ধ্রুব বর্ণ *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME MODE");
    প্রত্যাগম TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_SetResult(interp, (বর্ণ*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_fread(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  বর্ণ *zBuf;
  পূর্ণাঙ্ক sz;
  পূর্ণাঙ্ক nElem;
  size_t got;

  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE SIZE NELEM");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  যদি( Tcl_GetIntFromObj(interp, objv[2], &sz) ) প্রত্যাগম TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) প্রত্যাগম TCL_ERROR;
  zBuf = (বর্ণ*)sqlite3_malloc( sz*nElem + 1 );
  যদি( zBuf==0 ){
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
    প্রত্যাগম TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_SetResult(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_fwrite(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  বর্ণ *zBuf;
  পূর্ণাঙ্ক sz;
  পূর্ণাঙ্ক nElem;
  size_t got;

  যদি( objc!=5 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  যদি( Tcl_GetIntFromObj(interp, objv[2], &sz) ) প্রত্যাগম TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) প্রত্যাগম TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(got));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_fclose(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  পূর্ণাঙ্ক rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_fflush(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক doSync = 0;

  যদি( objc!=2 && objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE ?HARDSYNC?");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  যদি( objc==3 ){
    যদি( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) প্রত্যাগম TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_fseek(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  পূর্ণাঙ্ক ofst;
  ধ্রুব বর্ণ *zWhence;
  পূর্ণাঙ্ক whence;
  পূর্ণাঙ্ক rc;

  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE OFFSET WHENCE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  যদি( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) প্রত্যাগম TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  যদি( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }অন্যথা যদি( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }অন্যথা যদি( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }অন্যথা{
    Tcl_AppendResult(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (বর্ণ*)0);
    প্রত্যাগম TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_rewind(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_ftell(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_ftruncate(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  পূর্ণাঙ্ক rc;
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE SIZE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  যদি( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) প্রত্যাগম TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_file_size(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_size(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_file_truesize(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_file_mtime(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  time_t t;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(t));
  প্রত্যাগম TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_remove(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুব বর্ণ *zFilename;          /* File pattern to configure */
  পূর্ণাঙ্ক rc;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME");
    প্রত্যাগম TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_glob(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুব বর্ণ *zPattern;          /* The glob pattern */
  ধ্রুব বর্ণ *zText;             /* Text to compare against the pattern */
  পূর্ণাঙ্ক rc;
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "PATTERN TEXT");
    প্রত্যাগম TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_file_available(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  প্রত্যাগম TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_quota_ferror(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  পূর্ণাঙ্ক x;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE");
    প্রত্যাগম TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(x));
  প্রত্যাগম TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
পূর্ণাঙ্ক Sqlitequota_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
     বর্ণ *zName;
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
  পূর্ণাঙ্ক i;

  প্রতি(i=0; i<পরিমাণ(aCmd)/পরিমাণ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  প্রত্যাগম TCL_OK;
}
#endif
