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
#अन्तर्भाव "test_quota.h"
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#यदि defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)
#define sqlite3_mutex_held(X)     ((शून्य)(X),1)
#define sqlite3_mutex_notheld(X)  ((शून्य)(X),1)
#endif /* SQLITE_THREADSAFE==0 */


#ifdef _WIN32
# include <windows.h>
# include <io.h>
#अन्यथा
# include <unistd.h>
#endif


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
प्रकारनिर्देश संरचना quotaGroup quotaGroup;
प्रकारनिर्देश संरचना quotaConn quotaConn;
प्रकारनिर्देश संरचना quotaFile quotaFile;

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
संरचना quotaGroup {
  अचर वर्ण *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 iSize;           /* Current size of all files */
  शून्य (*xCallback)(             /* Callback invoked when going over quota */
     अचर वर्ण *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     शून्य *pArg                     /* Client data */
  );
  शून्य *pArg;                    /* Third argument to the xCallback() */
  शून्य (*xDestroy)(शून्य*);       /* Optional destructor for pArg */
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
संरचना quotaFile {
  वर्ण *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 iSize;            /* Current size of this file */
  पूर्णाङ्क nRef;                       /* Number of times this file is open */
  पूर्णाङ्क deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *pNext, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
संरचना quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
संरचना quota_FILE {
  खाता *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ifdef _WIN32
  वर्ण *zMbcsName;        /* Full MBCS pathname of the file */
#endif
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
स्थिर संरचना {
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
  पूर्णाङ्क isInitialized;

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
स्थिर शून्य quotaEnter(शून्य){ sqlite3_mutex_enter(gQuota.pMutex); }
स्थिर शून्य quotaLeave(शून्य){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
स्थिर पूर्णाङ्क quota_Group_Open_File_गन्ती(quotaGroup *pGroup){
  पूर्णाङ्क N = 0;
  quotaFile *pFile = pGroup->pFiles;
  जबसम्म( pFile ){
    यदि( pFile->nRef ) N++;
    pFile = pFile->pNext;
  }
  फर्काऊ N;
}

/* Remove a file from a quota group.
*/
स्थिर शून्य quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->iSize -= pFile->iSize;
  *pFile->ppPrev = pFile->pNext;
  यदि( pFile->pNext ) pFile->pNext->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
स्थिर शून्य quotaRemoveAllFiles(quotaGroup *pGroup){
  जबसम्म( pGroup->pFiles ){
    प्रतिज्ञा( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
स्थिर शून्य quotaGroupDeref(quotaGroup *pGroup){
  यदि( pGroup->iLimit==0 && quota_Group_Open_File_गन्ती(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->ppPrev = pGroup->pNext;
    यदि( pGroup->pNext ) pGroup->pNext->ppPrev = pGroup->ppPrev;
    यदि( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
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
स्थिर पूर्णाङ्क quotaStrglob(अचर वर्ण *zGlob, अचर वर्ण *z){
  पूर्णाङ्क c, c2, cx;
  पूर्णाङ्क invert;
  पूर्णाङ्क seen;

  जबसम्म( (c = (*(zGlob++)))!=0 ){
    यदि( c=='*' ){
      जबसम्म( (c=(*(zGlob++))) == '*' || c=='?' ){
        यदि( c=='?' && (*(z++))==0 ) फर्काऊ 0;
      }
      यदि( c==0 ){
        फर्काऊ 1;
      }अन्यथा यदि( c=='[' ){
        जबसम्म( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        फर्काऊ (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      जबसम्म( (c2 = (*(z++)))!=0 ){
        जबसम्म( c2!=c && c2!=cx ){
          c2 = *(z++);
          यदि( c2==0 ) फर्काऊ 0;
        }
        यदि( quotaStrglob(zGlob,z) ) फर्काऊ 1;
      }
      फर्काऊ 0;
    }अन्यथा यदि( c=='?' ){
      यदि( (*(z++))==0 ) फर्काऊ 0;
    }अन्यथा यदि( c=='[' ){
      पूर्णाङ्क prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      यदि( c==0 ) फर्काऊ 0;
      c2 = *(zGlob++);
      यदि( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      यदि( c2==']' ){
        यदि( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      जबसम्म( c2 && c2!=']' ){
        यदि( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          यदि( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }अन्यथा{
          यदि( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      यदि( c2==0 || (seen ^ invert)==0 ) फर्काऊ 0;
    }अन्यथा यदि( c=='/' ){
      यदि( z[0]!='/' && z[0]!='\\' ) फर्काऊ 0;
      z++;
    }अन्यथा{
      यदि( c!=(*(z++)) ) फर्काऊ 0;
    }
  }
  फर्काऊ *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
स्थिर quotaGroup *quotaGroupFind(अचर वर्ण *zFilename){
  quotaGroup *p;
  लागि(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->pNext){}
  फर्काऊ p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
स्थिर sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  फर्काऊ (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
स्थिर quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  अचर वर्ण *zName,      /* Full pathname of the file */
  पूर्णाङ्क createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  जबसम्म( pFile && म_भेद(pFile->zFilename, zName)!=0 ){
    pFile = pFile->pNext;
  }
  यदि( pFile==0 && createFlag ){
    पूर्णाङ्क nName = (पूर्णाङ्क)(म_माप(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( परिमाण(*pFile) + nName + 1 );
    यदि( pFile ){
      स_रखो(pFile, 0, परिमाण(*pFile));
      pFile->zFilename = (वर्ण*)&pFile[1];
      स_नकल(pFile->zFilename, zName, nName+1);
      pFile->pNext = pGroup->pFiles;
      यदि( pGroup->pFiles ) pGroup->pFiles->ppPrev = &pFile->pNext;
      pFile->ppPrev = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  फर्काऊ pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
स्थिर वर्ण *quota_utf8_to_mbcs(अचर वर्ण *zUtf8){
#ifdef _WIN32
  माप_प्रकार n;          /* Bytes in zUtf8 */
  पूर्णाङ्क nWide;         /* number of UTF-16 characters */
  पूर्णाङ्क nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  वर्ण *zMbcs;       /* The MBCS text */
  पूर्णाङ्क codepage;      /* Code page used by fopen() */

  n = म_माप(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, रिक्त, 0);
  यदि( nWide==0 ) फर्काऊ 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*परिमाण(zTmpWide[0]) );
  यदि( zTmpWide==0 ) फर्काऊ 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (वर्ण*)sqlite3_malloc( nMbcs+1 ) : 0;
  यदि( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  फर्काऊ zMbcs;
#अन्यथा
  फर्काऊ (वर्ण*)zUtf8;  /* No-op on unix */
#endif
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
स्थिर शून्य quota_mbcs_free(वर्ण *zOld){
#ifdef _WIN32
  sqlite3_free(zOld);
#अन्यथा
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
स्थिर पूर्णाङ्क quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  अचर वर्ण *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  पूर्णाङ्क flags,                  /* Flags to control the opening */
  पूर्णाङ्क *pOutFlags              /* Flags showing results of opening */
){
  पूर्णाङ्क rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  यदि( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    फर्काऊ pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  यदि( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }अन्यथा{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    यदि( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      यदि( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        फर्काऊ SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      यदि( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }अन्यथा{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  फर्काऊ rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
स्थिर पूर्णाङ्क quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  अचर वर्ण *zName,          /* Name of file to be deleted */
  पूर्णाङ्क syncDir                 /* Do a directory sync after deleting */
){
  पूर्णाङ्क rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  यदि( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    यदि( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      यदि( pFile ){
        यदि( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }अन्यथा{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  फर्काऊ rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
स्थिर पूर्णाङ्क quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  पूर्णाङ्क rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  यदि( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    यदि( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  फर्काऊ rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
स्थिर पूर्णाङ्क quotaRead(
  sqlite3_file *pConn,
  शून्य *pBuf,
  पूर्णाङ्क iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
स्थिर पूर्णाङ्क quotaWrite(
  sqlite3_file *pConn,
  अचर शून्य *pBuf,
  पूर्णाङ्क iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  यदि( pFile->iSize<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    यदि( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      यदि( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      यदि( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        फर्काऊ SQLITE_FULL;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }
  फर्काऊ pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
स्थिर पूर्णाङ्क quotaTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  पूर्णाङ्क rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  यदि( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = size;
    pGroup->iSize += size;
    quotaLeave();
  }
  फर्काऊ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
स्थिर पूर्णाङ्क quotaSync(sqlite3_file *pConn, पूर्णाङ्क flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
स्थिर पूर्णाङ्क quotaFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  पूर्णाङ्क rc;

  rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz);
  यदि( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->iSize -= pFile->iSize;
    pFile->iSize = sz;
    pGroup->iSize += sz;
    quotaLeave();
    *pSize = sz;
  }
  फर्काऊ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaLock(sqlite3_file *pConn, पूर्णाङ्क lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaUnlock(sqlite3_file *pConn, पूर्णाङ्क lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaCheckReservedLock(sqlite3_file *pConn, पूर्णाङ्क *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaFileControl(sqlite3_file *pConn, पूर्णाङ्क op, शून्य *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  पूर्णाङ्क rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#यदि defined(SQLITE_FCNTL_VFSNAME)
  यदि( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(वर्ण**)pArg = sqlite3_mprintf("quota/%z", *(वर्ण**)pArg);
  }
#endif
  फर्काऊ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaSectorSize(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xSectorSize(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  पूर्णाङ्क iRegion,                    /* Region to retrieve */
  पूर्णाङ्क szRegion,                   /* Size of regions */
  पूर्णाङ्क bExtend,                    /* True to extend file if necessary */
  शून्य चञ्चल **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  पूर्णाङ्क ofst,                  /* First lock to acquire or release */
  पूर्णाङ्क n,                     /* Number of locks to acquire or release */
  पूर्णाङ्क flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
स्थिर शून्य quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaShmUnmap(sqlite3_file *pConn, पूर्णाङ्क deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  फर्काऊ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
पूर्णाङ्क sqlite3_quota_initialize(अचर वर्ण *zOrigVfsName, पूर्णाङ्क makeDefault){
  sqlite3_vfs *pOrigVfs;
  यदि( gQuota.isInitialized ) फर्काऊ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  यदि( pOrigVfs==0 ) फर्काऊ SQLITE_ERROR;
  प्रतिज्ञा( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  यदि( !gQuota.pMutex ){
    फर्काऊ SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += परिमाण(quotaConn);
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
  फर्काऊ SQLITE_OK;
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
पूर्णाङ्क sqlite3_quota_shutdown(शून्य){
  quotaGroup *pGroup;
  यदि( gQuota.isInitialized==0 ) फर्काऊ SQLITE_MISUSE;
  लागि(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    यदि( quota_Group_Open_File_गन्ती(pGroup)>0 ) फर्काऊ SQLITE_MISUSE;
  }
  जबसम्म( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->pNext;
    pGroup->iLimit = 0;
    प्रतिज्ञा( quota_Group_Open_File_गन्ती(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  स_रखो(&gQuota, 0, परिमाण(gQuota));
  फर्काऊ SQLITE_OK;
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
पूर्णाङ्क sqlite3_quota_set(
  अचर वर्ण *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  शून्य (*xCallback)(              /* Callback invoked when going over quota */
     अचर वर्ण *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 iSize,           /* Total size of all files in the group */
     शून्य *pArg                     /* Client data */
  ),
  शून्य *pArg,                     /* client data passed thru to callback */
  शून्य (*xDestroy)(शून्य*)         /* Optional destructor for pArg */
){
  quotaGroup *pGroup;
  quotaEnter();
  pGroup = gQuota.pGroup;
  जबसम्म( pGroup && म_भेद(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->pNext;
  }
  यदि( pGroup==0 ){
    पूर्णाङ्क nPattern = (पूर्णाङ्क)(म_माप(zPattern) & 0x3fffffff);
    यदि( iLimit<=0 ){
      quotaLeave();
      फर्काऊ SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( परिमाण(*pGroup) + nPattern + 1 );
    यदि( pGroup==0 ){
      quotaLeave();
      फर्काऊ SQLITE_NOMEM;
    }
    स_रखो(pGroup, 0, परिमाण(*pGroup));
    pGroup->zPattern = (वर्ण*)&pGroup[1];
    स_नकल((वर्ण *)pGroup->zPattern, zPattern, nPattern+1);
    यदि( gQuota.pGroup ) gQuota.pGroup->ppPrev = &pGroup->pNext;
    pGroup->pNext = gQuota.pGroup;
    pGroup->ppPrev = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  यदि( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  फर्काऊ SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
पूर्णाङ्क sqlite3_quota_file(अचर वर्ण *zFilename){
  वर्ण *zFull = 0;
  sqlite3_file *fd;
  पूर्णाङ्क rc;
  पूर्णाङ्क outFlags = 0;
  sqlite3_int64 iSize;
  पूर्णाङ्क nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  यदि( fd==0 ){
    rc = SQLITE_NOMEM;
  }अन्यथा{
    zFull = &((वर्ण *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  यदि( rc==SQLITE_OK ){
    zFull[म_माप(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    यदि( rc==SQLITE_OK ){
      fd->pMethods->xFileSize(fd, &iSize);
      fd->pMethods->xClose(fd);
    }अन्यथा यदि( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      यदि( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        यदि( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  फर्काऊ rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(अचर वर्ण *zFilename, अचर वर्ण *zMode){
  quota_FILE *p = 0;
  वर्ण *zFull = 0;
  वर्ण *zFullTranslated = 0;
  पूर्णाङ्क rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (वर्ण*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  यदि( zFull==0 ) फर्काऊ 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  यदि( rc ) गच्छ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(परिमाण(*p));
  यदि( p==0 ) गच्छ quota_fopen_error;
  स_रखो(p, 0, परिमाण(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  यदि( zFullTranslated==0 ) गच्छ quota_fopen_error;
  p->f = ख_खोलो(zFullTranslated, zMode);
  यदि( p->f==0 ) गच्छ quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  यदि( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    यदि( pFile==0 ){
      quotaLeave();
      गच्छ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(zFull);
#ifdef _WIN32
  p->zMbcsName = zFullTranslated;
#endif
  फर्काऊ p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  यदि( p && p->f ) ख_बंद(p->f);
  sqlite3_free(p);
  फर्काऊ 0;
}

/*
** Read content from a quota_FILE
*/
माप_प्रकार sqlite3_quota_fread(
  शून्य *pBuf,            /* Store the content here */
  माप_प्रकार size,           /* Size of each element */
  माप_प्रकार nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  फर्काऊ ख_पढ़ो(pBuf, size, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
माप_प्रकार sqlite3_quota_fwrite(
  अचर शून्य *pBuf,      /* Take content to write from here */
  माप_प्रकार size,           /* Size of each element */
  माप_प्रकार nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 iEnd;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  माप_प्रकार rc;

  iOfst = ख_बताओ(p->f);
  iEnd = iOfst + size*nmemb;
  pFile = p->pFile;
  यदि( pFile && pFile->iSize<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->iSize - pFile->iSize + iEnd;
    यदि( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      यदि( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      यदि( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->iSize + pFile->iSize;
        nmemb = (माप_प्रकार)((iEnd - iOfst)/size);
        iEnd = iOfst + size*nmemb;
        szNew = pGroup->iSize - pFile->iSize + iEnd;
      }
    }
    pGroup->iSize = szNew;
    pFile->iSize = iEnd;
    quotaLeave();
  }अन्यथा{
    pFile = 0;
  }
  rc = ख_डालो(pBuf, size, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  यदि( rc<nmemb && pFile ){
    माप_प्रकार nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + size*nWritten;
    यदि( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->iSize += iNewEnd - pFile->iSize;
    pFile->iSize = iNewEnd;
    quotaLeave();
  }
  फर्काऊ rc;
}

/*
** Close an open quota_FILE stream.
*/
पूर्णाङ्क sqlite3_quota_fclose(quota_FILE *p){
  पूर्णाङ्क rc;
  quotaFile *pFile;
  rc = ख_बंद(p->f);
  pFile = p->pFile;
  यदि( pFile ){
    quotaEnter();
    pFile->nRef--;
    यदि( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      यदि( pFile->deleteOnClose ){
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
  फर्काऊ rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
पूर्णाङ्क sqlite3_quota_fflush(quota_FILE *p, पूर्णाङ्क doFsync){
  पूर्णाङ्क rc;
  rc = ख_साफ(p->f);
  यदि( rc==0 && doFsync ){
#ifdef _WIN32
    rc = _commit(_fileno(p->f));
#अन्यथा
    rc = fsync(fileno(p->f));
#endif
  }
  फर्काऊ rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
पूर्णाङ्क sqlite3_quota_fseek(quota_FILE *p, दीर्घ offset, पूर्णाङ्क whence){
  फर्काऊ ख_जाओ(p->f, offset, whence);
}

/*
** rewind a quota_FILE stream.
*/
शून्य sqlite3_quota_rewind(quota_FILE *p){
  शुरुआत(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
दीर्घ sqlite3_quota_ftell(quota_FILE *p){
  फर्काऊ ख_बताओ(p->f);
}

/*
** Test the error indicator for the given file.
*/
पूर्णाङ्क sqlite3_quota_ferror(quota_FILE *p){
  फर्काऊ ख_त्रुटि(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
पूर्णाङ्क sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  पूर्णाङ्क rc;
  यदि( (pFile = p->pFile)!=0 && pFile->iSize<szNew ){
    quotaGroup *pGroup;
    यदि( pFile->iSize<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      फर्काऊ -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    quotaLeave();
  }
#ifdef _WIN32
#  यदि defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (दीर्घ)szNew);
#  अन्यथा
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#अन्यथा
  rc = ftruncate(fileno(p->f), szNew);
#endif
  यदि( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->iSize += szNew - pFile->iSize;
    pFile->iSize = szNew;
    quotaLeave();
  }
  फर्काऊ rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
पूर्णाङ्क sqlite3_quota_file_mtime(quota_FILE *p, समय_प्रकार *pTime){
  पूर्णाङ्क rc;
#ifdef _WIN32
  संरचना _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#अन्यथा
  संरचना stat buf;
  rc = ख_स्थिति(fileno(p->f), &buf);
#endif
  यदि( rc==0 ) *pTime = buf.st_mtime;
  फर्काऊ rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  पूर्णाङ्क rc;
#ifdef _WIN32
  संरचना _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#अन्यथा
  संरचना stat buf;
  rc = ख_स्थिति(fileno(p->f), &buf);
#endif
  फर्काऊ rc==0 ? buf.st_size : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_size(quota_FILE *p){
  फर्काऊ p->pFile ? p->pFile->iSize : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
दीर्घ sqlite3_quota_file_available(quota_FILE *p){
  खाता* f = p->f;
  दीर्घ pos1, pos2;
  पूर्णाङ्क rc;
  pos1 = ख_बताओ(f);
  यदि ( pos1 < 0 ) फर्काऊ -1;
  rc = ख_जाओ(f, 0, अंत_से);
  यदि ( rc != 0 ) फर्काऊ -1;
  pos2 = ख_बताओ(f);
  यदि ( pos2 < 0 ) फर्काऊ -1;
  rc = ख_जाओ(f, pos1, शुरू_से);
  यदि ( rc != 0 ) फर्काऊ -1;
  फर्काऊ pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
पूर्णाङ्क sqlite3_quota_remove(अचर वर्ण *zFilename){
  वर्ण *zFull;            /* Full pathname for zFilename */
  माप_प्रकार nFull;           /* Number of bytes in zFilename */
  पूर्णाङ्क rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *pNextFile;   /* next file in the group */
  पूर्णाङ्क diff;               /* Difference between filenames */
  वर्ण c;                 /* First character past end of pattern */

  zFull = (वर्ण*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  यदि( zFull==0 ) फर्काऊ SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  यदि( rc ){
    sqlite3_free(zFull);
    फर्काऊ rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = म_माप(zFull);
  यदि( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  यदि( pGroup ){
    लागि(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=pNextFile){
      pNextFile = pFile->pNext;
      diff = म_भेदन(zFull, pFile->zFilename, nFull);
      यदि( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        यदि( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }अन्यथा{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(zFull);
  फर्काऊ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#अन्तर्भाव "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
प्रकारनिर्देश संरचना TclQuotaCallback TclQuotaCallback;
संरचना TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

बाह्य अचर वर्ण *sqlite3ErrName(पूर्णाङ्क);


/*
** This is the callback from a quota-over-limit.
*/
स्थिर शून्य tclQuotaCallback(
  अचर वर्ण *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 iSize,            /* Total size of all files in the group */
  शून्य *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  अचिन्हित पूर्णाङ्क rnd;               /* Random part of pVarname */
  पूर्णाङ्क rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  यदि( p==0 ) फर्काऊ;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_गन्ती(pVarname);
  sqlite3_randomness(परिमाण(rnd), (शून्य *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((पूर्णाङ्क)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_गन्ती(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(iSize));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  यदि( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_गन्ती(pEval);
  Tcl_Decr_Ref_गन्ती(pVarname);
  यदि( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
स्थिर शून्य tclCallbackDestructor(शून्य *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  यदि( p ){
    Tcl_Decr_Ref_गन्ती(p->pScript);
    sqlite3_free((वर्ण *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_initialize(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  अचर वर्ण *zName;              /* Name of new quota VFS */
  पूर्णाङ्क makeDefault;                /* True to make the new VFS the default */
  पूर्णाङ्क rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    फर्काऊ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  यदि( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) फर्काऊ TCL_ERROR;
  यदि( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);

  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_shutdown(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;                         /* Value returned by quota_shutdown() */

  यदि( objc!=1 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "");
    फर्काऊ TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);

  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_set(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  अचर वर्ण *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  पूर्णाङ्क rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_Size nScript;               /* Length of callback script */
  शून्य (*xDestroy)(शून्य*);        /* Optional destructor for pArg */
  शून्य (*xCallback)(अचर वर्ण *, sqlite3_int64 *, sqlite3_int64, शून्य *);

  /* Process arguments */
  यदि( objc!=4 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    फर्काऊ TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  यदि( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) फर्काऊ TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  यदि( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(परिमाण(TclQuotaCallback));
    यदि( !p ){
      Tcl_SetResult(interp, (वर्ण *)"SQLITE_NOMEM", TCL_STATIC);
      फर्काऊ TCL_OK;
    }
    स_रखो(p, 0, परिमाण(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_गन्ती(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }अन्यथा{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (शून्य*)p, xDestroy);

  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  अचर वर्ण *zFilename;          /* File pattern to configure */
  पूर्णाङ्क rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "FILENAME");
    फर्काऊ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
  फर्काऊ TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_dump(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *pResult;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  pResult = Tcl_NewObj();
  quotaEnter();
  लागि(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->pNext){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iSize));
    लागि(pFile=pGroup->pFiles; pFile; pFile=pFile->pNext){
      पूर्णाङ्क i;
      वर्ण zTemp[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(परिमाण(zTemp), zTemp, "%s", pFile->zFilename);
      लागि(i=0; zTemp[i]; i++){ यदि( zTemp[i]=='\\' ) zTemp[i] = '/'; }
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
  फर्काऊ TCL_OK;
}

/* Defined in test1.c */
बाह्य शून्य *sqlite3TestTextToPtr(अचर वर्ण*);
बाह्य अचर वर्ण *sqlite3TestPtrToText(शून्य*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fopen(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  अचर वर्ण *zFilename;          /* File pattern to configure */
  अचर वर्ण *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "FILENAME MODE");
    फर्काऊ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_SetResult(interp, (वर्ण*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fread(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  वर्ण *zBuf;
  पूर्णाङ्क sz;
  पूर्णाङ्क nElem;
  माप_प्रकार got;

  यदि( objc!=4 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE SIZE NELEM");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  यदि( Tcl_GetIntFromObj(interp, objv[2], &sz) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) फर्काऊ TCL_ERROR;
  zBuf = (वर्ण*)sqlite3_malloc( sz*nElem + 1 );
  यदि( zBuf==0 ){
    Tcl_SetResult(interp, "out of memory", TCL_STATIC);
    फर्काऊ TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_SetResult(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fwrite(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  वर्ण *zBuf;
  पूर्णाङ्क sz;
  पूर्णाङ्क nElem;
  माप_प्रकार got;

  यदि( objc!=5 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  यदि( Tcl_GetIntFromObj(interp, objv[2], &sz) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) फर्काऊ TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(got));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fclose(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fflush(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णाङ्क rc;
  पूर्णाङ्क doSync = 0;

  यदि( objc!=2 && objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE ?HARDSYNC?");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  यदि( objc==3 ){
    यदि( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) फर्काऊ TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fseek(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णाङ्क ofst;
  अचर वर्ण *zWhence;
  पूर्णाङ्क whence;
  पूर्णाङ्क rc;

  यदि( objc!=4 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE OFFSET WHENCE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  यदि( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) फर्काऊ TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  यदि( म_भेद(zWhence, "SEEK_SET")==0 ){
    whence = शुरू_से;
  }अन्यथा यदि( म_भेद(zWhence, "SEEK_CUR")==0 ){
    whence = प्रस्तुत_से;
  }अन्यथा यदि( म_भेद(zWhence, "SEEK_END")==0 ){
    whence = अंत_से;
  }अन्यथा{
    Tcl_AppendResult(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_rewind(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_ftell(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_ftruncate(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  पूर्णाङ्क rc;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE SIZE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  यदि( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) फर्काऊ TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file_size(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_size(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file_truesize(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file_mtime(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  समय_प्रकार t;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(t));
  फर्काऊ TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_remove(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  अचर वर्ण *zFilename;          /* File pattern to configure */
  पूर्णाङ्क rc;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "FILENAME");
    फर्काऊ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_glob(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  अचर वर्ण *zPattern;          /* The glob pattern */
  अचर वर्ण *zText;             /* Text to compare against the pattern */
  पूर्णाङ्क rc;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "PATTERN TEXT");
    फर्काऊ TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file_available(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(x));
  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_ferror(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णाङ्क x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE");
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(x));
  फर्काऊ TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
पूर्णाङ्क Sqlitequota_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
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
  पूर्णाङ्क i;

  लागि(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  फर्काऊ TCL_OK;
}
#endif
