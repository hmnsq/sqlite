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
#ضمن "test_quota.h"
#ضمن <string.h>
#ضمن <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#إذا defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#تعریف_کړه sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#تعریف_کړه sqlite3_mutex_free(X)
#تعریف_کړه sqlite3_mutex_enter(X)
#تعریف_کړه sqlite3_mutex_try(X)      SQLITE_OK
#تعریف_کړه sqlite3_mutex_leave(X)
#تعریف_کړه sqlite3_mutex_held(X)     ((فراغ)(X),1)
#تعریف_کړه sqlite3_mutex_notheld(X)  ((فراغ)(X),1)
#د_که_پای /* SQLITE_THREADSAFE==0 */


#که_تعریف_شوی _WIN32
# include <windows.h>
# include <io.h>
#وإلا
# include <unistd.h>
#د_که_پای


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
تعريفنوع بنية quota_ډله quota_ډله;
تعريفنوع بنية quotaConn quotaConn;
تعريفنوع بنية quotaFile quotaFile;

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
بنية quota_ډله {
  ثابت حرف *z_نمونه;          /* Filename pattern to be quotaed */
  sqlite3_int64 i_حد;          /* Upper bound on total file size */
  sqlite3_int64 i_اندازه;           /* Current size of all files */
  فراغ (*xCallback)(             /* Callback invoked when going over quota */
     ثابت حرف *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_حد,        /* IN/OUT: The current limit */
     sqlite3_int64 i_اندازه,           /* Total size of all files in the group */
     فراغ *pArg                     /* Client data */
  );
  فراغ *pArg;                    /* Third argument to the xCallback() */
  فراغ (*xDestroy)(فراغ*);       /* Optional destructor for pArg */
  quota_ډله *p_بل, **ppPrev;   /* Doubly linked list of all quota objects */
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
بنية quotaFile {
  حرف *zFilename;                /* Name of this file */
  quota_ډله *p_ډله;             /* Quota group to which this file belongs */
  sqlite3_int64 i_اندازه;            /* Current size of this file */
  صحيح nRef;                       /* Number of times this file is open */
  صحيح deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *p_بل, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
بنية quotaConn {
  sqlite3_file بنسټ;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
بنية quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#که_تعریف_شوی _WIN32
  حرف *zMbcsName;        /* Full MBCS pathname of the file */
#د_که_پای
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
ساكن بنية {
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
  صحيح isInitialized;

  /* For run-time access any of the other global data structures in this
  ** shim, the following mutex must be held.
  */
  sqlite3_mutex *pMutex;

  /* List of quotaGroup objects.
  */
  quota_ډله *p_ډله;

} gQuota;

/************************* Utility Routines *********************************/
/*
** Acquire and release the mutex used to serialize access to the
** list of quotaGroups.
*/
ساكن فراغ quotaEnter(فراغ){ sqlite3_mutex_enter(gQuota.pMutex); }
ساكن فراغ quotaLeave(فراغ){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
ساكن صحيح quota_ډله_Open_File_شمېر(quota_ډله *p_ډله){
  صحيح N = 0;
  quotaFile *pFile = p_ډله->pFiles;
  طالما( pFile ){
    إذا( pFile->nRef ) N++;
    pFile = pFile->p_بل;
  }
  أرجع N;
}

/* Remove a file from a quota group.
*/
ساكن فراغ quotaRemoveFile(quotaFile *pFile){
  quota_ډله *p_ډله = pFile->p_ډله;
  p_ډله->i_اندازه -= pFile->i_اندازه;
  *pFile->ppPrev = pFile->p_بل;
  إذا( pFile->p_بل ) pFile->p_بل->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
ساكن فراغ quotaRemoveAllFiles(quota_ډله *p_ډله){
  طالما( p_ډله->pFiles ){
    أكد( p_ډله->pFiles->nRef==0 );
    quotaRemoveFile(p_ډله->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
ساكن فراغ quota_ډله_Deref(quota_ډله *p_ډله){
  إذا( p_ډله->i_حد==0 && quota_ډله_Open_File_شمېر(p_ډله)==0 ){
    quotaRemoveAllFiles(p_ډله);
    *p_ډله->ppPrev = p_ډله->p_بل;
    إذا( p_ډله->p_بل ) p_ډله->p_بل->ppPrev = p_ډله->ppPrev;
    إذا( p_ډله->xDestroy ) p_ډله->xDestroy(p_ډله->pArg);
    sqlite3_free(p_ډله);
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
ساكن صحيح quotaStrglob(ثابت حرف *zGlob, ثابت حرف *z){
  صحيح c, c2, cx;
  صحيح invert;
  صحيح کتل_شوی;

  طالما( (c = (*(zGlob++)))!=0 ){
    إذا( c=='*' ){
      طالما( (c=(*(zGlob++))) == '*' || c=='?' ){
        إذا( c=='?' && (*(z++))==0 ) أرجع 0;
      }
      إذا( c==0 ){
        أرجع 1;
      }وإلا إذا( c=='[' ){
        طالما( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        أرجع (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      طالما( (c2 = (*(z++)))!=0 ){
        طالما( c2!=c && c2!=cx ){
          c2 = *(z++);
          إذا( c2==0 ) أرجع 0;
        }
        إذا( quotaStrglob(zGlob,z) ) أرجع 1;
      }
      أرجع 0;
    }وإلا إذا( c=='?' ){
      إذا( (*(z++))==0 ) أرجع 0;
    }وإلا إذا( c=='[' ){
      صحيح مخکینی_c = 0;
      کتل_شوی = 0;
      invert = 0;
      c = *(z++);
      إذا( c==0 ) أرجع 0;
      c2 = *(zGlob++);
      إذا( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      إذا( c2==']' ){
        إذا( c==']' ) کتل_شوی = 1;
        c2 = *(zGlob++);
      }
      طالما( c2 && c2!=']' ){
        إذا( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && مخکینی_c>0 ){
          c2 = *(zGlob++);
          إذا( c>=مخکینی_c && c<=c2 ) کتل_شوی = 1;
          مخکینی_c = 0;
        }وإلا{
          إذا( c==c2 ){
            کتل_شوی = 1;
          }
          مخکینی_c = c2;
        }
        c2 = *(zGlob++);
      }
      إذا( c2==0 || (کتل_شوی ^ invert)==0 ) أرجع 0;
    }وإلا إذا( c=='/' ){
      إذا( z[0]!='/' && z[0]!='\\' ) أرجع 0;
      z++;
    }وإلا{
      إذا( c!=(*(z++)) ) أرجع 0;
    }
  }
  أرجع *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
ساكن quota_ډله *quota_ډله_پیدا_کړه(ثابت حرف *zFilename){
  quota_ډله *p;
  لكل(p=gQuota.p_ډله; p && quotaStrglob(p->z_نمونه, zFilename)==0;
      p=p->p_بل){}
  أرجع p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
ساكن sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  أرجع (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
ساكن quotaFile *quota_پیدا_کړه_File(
  quota_ډله *p_ډله,     /* Group in which to look for the file */
  ثابت حرف *zName,      /* Full pathname of the file */
  صحيح createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = p_ډله->pFiles;
  طالما( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->p_بل;
  }
  إذا( pFile==0 && createFlag ){
    صحيح nName = (صحيح)(مزي_اوږدوالی(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( حجم(*pFile) + nName + 1 );
    إذا( pFile ){
      memset(pFile, 0, حجم(*pFile));
      pFile->zFilename = (حرف*)&pFile[1];
      حافظه_نقل(pFile->zFilename, zName, nName+1);
      pFile->p_بل = p_ډله->pFiles;
      إذا( p_ډله->pFiles ) p_ډله->pFiles->ppPrev = &pFile->p_بل;
      pFile->ppPrev = &p_ډله->pFiles;
      p_ډله->pFiles = pFile;
      pFile->p_ډله = p_ډله;
    }
  }
  أرجع pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
ساكن حرف *quota_utf8_منزل_mbcs(ثابت حرف *zUtf8){
#که_تعریف_شوی _WIN32
  size_t n;          /* Bytes in zUtf8 */
  صحيح nWide;         /* number of UTF-16 characters */
  صحيح nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  حرف *zMbcs;       /* The MBCS text */
  صحيح codepage;      /* Code page used by fopen() */

  n = مزي_اوږدوالی(zUtf8);
  nWide = Multi_Byte_منزل_Wide_Char(CP_UTF8, 0, zUtf8, -1, عدم, 0);
  إذا( nWide==0 ) أرجع 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*حجم(zTmpWide[0]) );
  إذا( zTmpWide==0 ) أرجع 0;
  Multi_Byte_منزل_Wide_Char(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = Wide_Char_منزل_Multi_Byte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (حرف*)sqlite3_malloc( nMbcs+1 ) : 0;
  إذا( zMbcs ){
    Wide_Char_منزل_Multi_Byte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  أرجع zMbcs;
#وإلا
  أرجع (حرف*)zUtf8;  /* No-op on unix */
#د_که_پای
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
ساكن فراغ quota_mbcs_free(حرف *zOld){
#که_تعریف_شوی _WIN32
  sqlite3_free(zOld);
#وإلا
  /* No-op on unix */
#د_که_پای
}

/************************* VFS Method Wrappers *****************************/
/*
** This is the xOpen method used for the "quota" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate quota group if it is a
** file that needs to be tracked.
*/
ساكن صحيح quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ثابت حرف *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  صحيح flags,                  /* Flags to control the opening */
  صحيح *p_بهر_Flags              /* Flags showing results of opening */
){
  صحيح rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quota_ډله *p_ډله;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  إذا( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    أرجع pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_بهر_Flags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  p_ډله = quota_ډله_پیدا_کړه(zName);
  إذا( p_ډله==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_بهر_Flags);
  }وإلا{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, p_بهر_Flags);
    إذا( rc==SQLITE_OK ){
      pFile = quota_پیدا_کړه_File(p_ډله, zName, 1);
      إذا( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        أرجع SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      إذا( pSubOpen->pMethods->i_نسخه==1 ){
        pQuotaOpen->بنسټ.pMethods = &gQuota.sIoMethodsV1;
      }وإلا{
        pQuotaOpen->بنسټ.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  أرجع rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
ساكن صحيح quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ثابت حرف *zName,          /* Name of file to be deleted */
  صحيح syncDir                 /* Do a directory sync after deleting */
){
  صحيح rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quota_ډله *p_ډله;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  إذا( rc==SQLITE_OK ){
    quotaEnter();
    p_ډله = quota_ډله_پیدا_کړه(zName);
    إذا( p_ډله ){
      pFile = quota_پیدا_کړه_File(p_ډله, zName, 0);
      إذا( pFile ){
        إذا( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }وإلا{
          quotaRemoveFile(pFile);
          quota_ډله_Deref(p_ډله);
        }
      }
    }
    quotaLeave();
  }
  أرجع rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
ساكن صحيح quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  صحيح rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  إذا( pFile->nRef==0 ){
    quota_ډله *p_ډله = pFile->p_ډله;
    إذا( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quota_ډله_Deref(p_ډله);
  }
  quotaLeave();
  أرجع rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
ساكن صحيح quotaRead(
  sqlite3_file *pConn,
  فراغ *pBuf,
  صحيح iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
ساكن صحيح quotaWrite(
  sqlite3_file *pConn,
  ثابت فراغ *pBuf,
  صحيح iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 i_پای_ټکی = iOfst+iAmt;
  quota_ډله *p_ډله;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  إذا( pFile->i_اندازه<i_پای_ټکی ){
    p_ډله = pFile->p_ډله;
    quotaEnter();
    szNew = p_ډله->i_اندازه - pFile->i_اندازه + i_پای_ټکی;
    إذا( szNew>p_ډله->i_حد && p_ډله->i_حد>0 ){
      إذا( p_ډله->xCallback ){
        p_ډله->xCallback(pFile->zFilename, &p_ډله->i_حد, szNew,
                          p_ډله->pArg);
      }
      إذا( szNew>p_ډله->i_حد && p_ډله->i_حد>0 ){
        quotaLeave();
        أرجع SQLITE_ډک;
      }
    }
    p_ډله->i_اندازه = szNew;
    pFile->i_اندازه = i_پای_ټکی;
    quotaLeave();
  }
  أرجع pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
ساكن صحيح quotaTruncate(sqlite3_file *pConn, sqlite3_int64 اندازه){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  صحيح rc = pSubOpen->pMethods->xTruncate(pSubOpen, اندازه);
  quotaFile *pFile = p->pFile;
  quota_ډله *p_ډله;
  إذا( rc==SQLITE_OK ){
    quotaEnter();
    p_ډله = pFile->p_ډله;
    p_ډله->i_اندازه -= pFile->i_اندازه;
    pFile->i_اندازه = اندازه;
    p_ډله->i_اندازه += اندازه;
    quotaLeave();
  }
  أرجع rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ساكن صحيح quotaSync(sqlite3_file *pConn, صحيح flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
ساكن صحيح quota_File_اندازه(sqlite3_file *pConn, sqlite3_int64 *p_اندازه){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quota_ډله *p_ډله;
  sqlite3_int64 sz;
  صحيح rc;

  rc = pSubOpen->pMethods->x_File_اندازه(pSubOpen, &sz);
  إذا( rc==SQLITE_OK ){
    quotaEnter();
    p_ډله = pFile->p_ډله;
    p_ډله->i_اندازه -= pFile->i_اندازه;
    pFile->i_اندازه = sz;
    p_ډله->i_اندازه += sz;
    quotaLeave();
    *p_اندازه = sz;
  }
  أرجع rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ساكن صحيح quota_قلف(sqlite3_file *pConn, صحيح قلف){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->x_قلف(pSubOpen, قلف);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ساكن صحيح quotaUnlock(sqlite3_file *pConn, صحيح قلف){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->xUnlock(pSubOpen, قلف);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ساكن صحيح quota_Check_Reserved_قلف(sqlite3_file *pConn, صحيح *p_Res_بهر){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->x_Check_Reserved_قلف(pSubOpen, p_Res_بهر);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
ساكن صحيح quotaFileControl(sqlite3_file *pConn, صحيح op, فراغ *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  صحيح rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#إذا defined(SQLITE_FCNTL_VFSNAME)
  إذا( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(حرف**)pArg = sqlite3_mprintf("quota/%z", *(حرف**)pArg);
  }
#د_که_پای
  أرجع rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ساكن صحيح quota_Sector_اندازه(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->x_Sector_اندازه(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ساكن صحيح quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ساكن صحيح quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  صحيح iRegion,                    /* Region to retrieve */
  صحيح szRegion,                   /* Size of regions */
  صحيح bExtend,                    /* True to extend file if necessary */
  فراغ متطاير **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ساكن صحيح quota_Shm_قلف(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  صحيح ofst,                  /* First lock to acquire or release */
  صحيح n,                     /* Number of locks to acquire or release */
  صحيح flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->x_Shm_قلف(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ساكن فراغ quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ساكن صحيح quotaShmUnmap(sqlite3_file *pConn, صحيح deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  أرجع pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
صحيح sqlite3_quota_initialize(ثابت حرف *zOrigVfsName, صحيح makeDefault){
  sqlite3_vfs *pOrigVfs;
  إذا( gQuota.isInitialized ) أرجع SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_پیدا_کړه(zOrigVfsName);
  إذا( pOrigVfs==0 ) أرجع SQLITE_ERROR;
  أكد( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  إذا( !gQuota.pMutex ){
    أرجع SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += حجم(quotaConn);
  gQuota.sThisVfs.zName = "quota";
  gQuota.sIoMethodsV1.i_نسخه = 1;
  gQuota.sIoMethodsV1.xClose = quotaClose;
  gQuota.sIoMethodsV1.xRead = quotaRead;
  gQuota.sIoMethodsV1.xWrite = quotaWrite;
  gQuota.sIoMethodsV1.xTruncate = quotaTruncate;
  gQuota.sIoMethodsV1.xSync = quotaSync;
  gQuota.sIoMethodsV1.x_File_اندازه = quota_File_اندازه;
  gQuota.sIoMethodsV1.x_قلف = quota_قلف;
  gQuota.sIoMethodsV1.xUnlock = quotaUnlock;
  gQuota.sIoMethodsV1.x_Check_Reserved_قلف = quota_Check_Reserved_قلف;
  gQuota.sIoMethodsV1.xFileControl = quotaFileControl;
  gQuota.sIoMethodsV1.x_Sector_اندازه = quota_Sector_اندازه;
  gQuota.sIoMethodsV1.xDeviceCharacteristics = quotaDeviceCharacteristics;
  gQuota.sIoMethodsV2 = gQuota.sIoMethodsV1;
  gQuota.sIoMethodsV2.i_نسخه = 2;
  gQuota.sIoMethodsV2.xShmMap = quotaShmMap;
  gQuota.sIoMethodsV2.x_Shm_قلف = quota_Shm_قلف;
  gQuota.sIoMethodsV2.xShmBarrier = quotaShmBarrier;
  gQuota.sIoMethodsV2.xShmUnmap = quotaShmUnmap;
  sqlite3_vfs_ثبتوونکی(&gQuota.sThisVfs, makeDefault);
  أرجع SQLITE_OK;
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
صحيح sqlite3_quota_shutdown(فراغ){
  quota_ډله *p_ډله;
  إذا( gQuota.isInitialized==0 ) أرجع SQLITE_MISUSE;
  لكل(p_ډله=gQuota.p_ډله; p_ډله; p_ډله=p_ډله->p_بل){
    إذا( quota_ډله_Open_File_شمېر(p_ډله)>0 ) أرجع SQLITE_MISUSE;
  }
  طالما( gQuota.p_ډله ){
    p_ډله = gQuota.p_ډله;
    gQuota.p_ډله = p_ډله->p_بل;
    p_ډله->i_حد = 0;
    أكد( quota_ډله_Open_File_شمېر(p_ډله)==0 );
    quota_ډله_Deref(p_ډله);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, حجم(gQuota));
  أرجع SQLITE_OK;
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
صحيح sqlite3_quota_سېټ(
  ثابت حرف *z_نمونه,           /* The filename pattern */
  sqlite3_int64 i_حد,           /* New quota to set for this quota group */
  فراغ (*xCallback)(              /* Callback invoked when going over quota */
     ثابت حرف *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_حد,        /* IN/OUT: The current limit */
     sqlite3_int64 i_اندازه,           /* Total size of all files in the group */
     فراغ *pArg                     /* Client data */
  ),
  فراغ *pArg,                     /* client data passed thru to callback */
  فراغ (*xDestroy)(فراغ*)         /* Optional destructor for pArg */
){
  quota_ډله *p_ډله;
  quotaEnter();
  p_ډله = gQuota.p_ډله;
  طالما( p_ډله && strcmp(p_ډله->z_نمونه, z_نمونه)!=0 ){
    p_ډله = p_ډله->p_بل;
  }
  إذا( p_ډله==0 ){
    صحيح n_نمونه = (صحيح)(مزي_اوږدوالی(z_نمونه) & 0x3fffffff);
    إذا( i_حد<=0 ){
      quotaLeave();
      أرجع SQLITE_OK;
    }
    p_ډله = (quota_ډله *)sqlite3_malloc( حجم(*p_ډله) + n_نمونه + 1 );
    إذا( p_ډله==0 ){
      quotaLeave();
      أرجع SQLITE_NOMEM;
    }
    memset(p_ډله, 0, حجم(*p_ډله));
    p_ډله->z_نمونه = (حرف*)&p_ډله[1];
    حافظه_نقل((حرف *)p_ډله->z_نمونه, z_نمونه, n_نمونه+1);
    إذا( gQuota.p_ډله ) gQuota.p_ډله->ppPrev = &p_ډله->p_بل;
    p_ډله->p_بل = gQuota.p_ډله;
    p_ډله->ppPrev = &gQuota.p_ډله;
    gQuota.p_ډله = p_ډله;
  }
  p_ډله->i_حد = i_حد;
  p_ډله->xCallback = xCallback;
  إذا( p_ډله->xDestroy && p_ډله->pArg!=pArg ){
    p_ډله->xDestroy(p_ډله->pArg);
  }
  p_ډله->pArg = pArg;
  p_ډله->xDestroy = xDestroy;
  quota_ډله_Deref(p_ډله);
  quotaLeave();
  أرجع SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
صحيح sqlite3_quota_file(ثابت حرف *zFilename){
  حرف *z_ډک = 0;
  sqlite3_file *fd;
  صحيح rc;
  صحيح بهر_Flags = 0;
  sqlite3_int64 i_اندازه;
  صحيح nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  إذا( fd==0 ){
    rc = SQLITE_NOMEM;
  }وإلا{
    z_ډک = &((حرف *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->x_ډک_Pathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, z_ډک);
  }

  إذا( rc==SQLITE_OK ){
    z_ډک[مزي_اوږدوالی(z_ډک)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, z_ډک, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &بهر_Flags);
    إذا( rc==SQLITE_OK ){
      fd->pMethods->x_File_اندازه(fd, &i_اندازه);
      fd->pMethods->xClose(fd);
    }وإلا إذا( rc==SQLITE_CANTOPEN ){
      quota_ډله *p_ډله;
      quotaFile *pFile;
      quotaEnter();
      p_ډله = quota_ډله_پیدا_کړه(z_ډک);
      إذا( p_ډله ){
        pFile = quota_پیدا_کړه_File(p_ډله, z_ډک, 0);
        إذا( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  أرجع rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(ثابت حرف *zFilename, ثابت حرف *z_ډېرپېښ){
  quota_FILE *p = 0;
  حرف *z_ډک = 0;
  حرف *z_ډک_Translated = 0;
  صحيح rc;
  quota_ډله *p_ډله;
  quotaFile *pFile;

  z_ډک = (حرف*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  إذا( z_ډک==0 ) أرجع 0;
  rc = gQuota.pOrigVfs->x_ډک_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_ډک);
  إذا( rc ) اذهب quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(حجم(*p));
  إذا( p==0 ) اذهب quota_fopen_error;
  memset(p, 0, حجم(*p));
  z_ډک_Translated = quota_utf8_منزل_mbcs(z_ډک);
  إذا( z_ډک_Translated==0 ) اذهب quota_fopen_error;
  p->f = fopen(z_ډک_Translated, z_ډېرپېښ);
  إذا( p->f==0 ) اذهب quota_fopen_error;
  quotaEnter();
  p_ډله = quota_ډله_پیدا_کړه(z_ډک);
  إذا( p_ډله ){
    pFile = quota_پیدا_کړه_File(p_ډله, z_ډک, 1);
    إذا( pFile==0 ){
      quotaLeave();
      اذهب quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(z_ډک);
#که_تعریف_شوی _WIN32
  p->zMbcsName = z_ډک_Translated;
#د_که_پای
  أرجع p;

quota_fopen_error:
  quota_mbcs_free(z_ډک_Translated);
  sqlite3_free(z_ډک);
  إذا( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  أرجع 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  فراغ *pBuf,            /* Store the content here */
  size_t اندازه,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  أرجع fread(pBuf, اندازه, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  ثابت فراغ *pBuf,      /* Take content to write from here */
  size_t اندازه,           /* Size of each element */
  size_t nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 i_پای_ټکی;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  size_t rc;

  iOfst = ftell(p->f);
  i_پای_ټکی = iOfst + اندازه*nmemb;
  pFile = p->pFile;
  إذا( pFile && pFile->i_اندازه<i_پای_ټکی ){
    quota_ډله *p_ډله = pFile->p_ډله;
    quotaEnter();
    szNew = p_ډله->i_اندازه - pFile->i_اندازه + i_پای_ټکی;
    إذا( szNew>p_ډله->i_حد && p_ډله->i_حد>0 ){
      إذا( p_ډله->xCallback ){
        p_ډله->xCallback(pFile->zFilename, &p_ډله->i_حد, szNew,
                          p_ډله->pArg);
      }
      إذا( szNew>p_ډله->i_حد && p_ډله->i_حد>0 ){
        i_پای_ټکی = p_ډله->i_حد - p_ډله->i_اندازه + pFile->i_اندازه;
        nmemb = (size_t)((i_پای_ټکی - iOfst)/اندازه);
        i_پای_ټکی = iOfst + اندازه*nmemb;
        szNew = p_ډله->i_اندازه - pFile->i_اندازه + i_پای_ټکی;
      }
    }
    p_ډله->i_اندازه = szNew;
    pFile->i_اندازه = i_پای_ټکی;
    quotaLeave();
  }وإلا{
    pFile = 0;
  }
  rc = fwrite(pBuf, اندازه, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  إذا( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 i_New_پای_ټکی = iOfst + اندازه*nWritten;
    إذا( i_New_پای_ټکی<i_پای_ټکی ) i_New_پای_ټکی = i_پای_ټکی;
    quotaEnter();
    pFile->p_ډله->i_اندازه += i_New_پای_ټکی - pFile->i_اندازه;
    pFile->i_اندازه = i_New_پای_ټکی;
    quotaLeave();
  }
  أرجع rc;
}

/*
** Close an open quota_FILE stream.
*/
صحيح sqlite3_quota_fclose(quota_FILE *p){
  صحيح rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  إذا( pFile ){
    quotaEnter();
    pFile->nRef--;
    إذا( pFile->nRef==0 ){
      quota_ډله *p_ډله = pFile->p_ډله;
      إذا( pFile->deleteOnClose ){
        gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
        quotaRemoveFile(pFile);
      }
      quota_ډله_Deref(p_ډله);
    }
    quotaLeave();
  }
#که_تعریف_شوی _WIN32
  quota_mbcs_free(p->zMbcsName);
#د_که_پای
  sqlite3_free(p);
  أرجع rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
صحيح sqlite3_quota_fflush(quota_FILE *p, صحيح doFsync){
  صحيح rc;
  rc = fflush(p->f);
  إذا( rc==0 && doFsync ){
#که_تعریف_شوی _WIN32
    rc = _commit(_fileno(p->f));
#وإلا
    rc = fsync(fileno(p->f));
#د_که_پای
  }
  أرجع rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
صحيح sqlite3_quota_fseek(quota_FILE *p, اوږد دننه_واټن, صحيح whence){
  أرجع fseek(p->f, دننه_واټن, whence);
}

/*
** rewind a quota_FILE stream.
*/
فراغ sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
اوږد sqlite3_quota_ftell(quota_FILE *p){
  أرجع ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
صحيح sqlite3_quota_ferror(quota_FILE *p){
  أرجع ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
صحيح sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  صحيح rc;
  إذا( (pFile = p->pFile)!=0 && pFile->i_اندازه<szNew ){
    quota_ډله *p_ډله;
    إذا( pFile->i_اندازه<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      أرجع -1;
    }
    p_ډله = pFile->p_ډله;
    quotaEnter();
    p_ډله->i_اندازه += szNew - pFile->i_اندازه;
    quotaLeave();
  }
#که_تعریف_شوی _WIN32
#  إذا defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (اوږد)szNew);
#  وإلا
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#وإلا
  rc = ftruncate(fileno(p->f), szNew);
#د_که_پای
  إذا( pFile && rc==0 ){
    quota_ډله *p_ډله = pFile->p_ډله;
    quotaEnter();
    p_ډله->i_اندازه += szNew - pFile->i_اندازه;
    pFile->i_اندازه = szNew;
    quotaLeave();
  }
  أرجع rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
صحيح sqlite3_quota_file_mtime(quota_FILE *p, وخت_t *p_وخت){
  صحيح rc;
#که_تعریف_شوی _WIN32
  بنية _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#وإلا
  بنية stat buf;
  rc = fstat(fileno(p->f), &buf);
#د_که_پای
  إذا( rc==0 ) *p_وخت = buf.st_mtime;
  أرجع rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  صحيح rc;
#که_تعریف_شوی _WIN32
  بنية _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#وإلا
  بنية stat buf;
  rc = fstat(fileno(p->f), &buf);
#د_که_پای
  أرجع rc==0 ? buf.st_اندازه : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_اندازه(quota_FILE *p){
  أرجع p->pFile ? p->pFile->i_اندازه : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
اوږد sqlite3_quota_file_شته(quota_FILE *p){
  FILE* f = p->f;
  اوږد pos1, pos2;
  صحيح rc;
  pos1 = ftell(f);
  إذا ( pos1 < 0 ) أرجع -1;
  rc = fseek(f, 0, SEEK_END);
  إذا ( rc != 0 ) أرجع -1;
  pos2 = ftell(f);
  إذا ( pos2 < 0 ) أرجع -1;
  rc = fseek(f, pos1, SEEK_SET);
  إذا ( rc != 0 ) أرجع -1;
  أرجع pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
صحيح sqlite3_quota_remove(ثابت حرف *zFilename){
  حرف *z_ډک;            /* Full pathname for zFilename */
  size_t n_ډک;           /* Number of bytes in zFilename */
  صحيح rc;                 /* Result code */
  quota_ډله *p_ډله;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *p_بل_File;   /* next file in the group */
  صحيح diff;               /* Difference between filenames */
  حرف c;                 /* First character past end of pattern */

  z_ډک = (حرف*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  إذا( z_ډک==0 ) أرجع SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->x_ډک_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_ډک);
  إذا( rc ){
    sqlite3_free(z_ډک);
    أرجع rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  n_ډک = مزي_اوږدوالی(z_ډک);
  إذا( n_ډک>0 && (z_ډک[n_ډک-1]=='/' || z_ډک[n_ډک-1]=='\\') ){
    n_ډک--;
    z_ډک[n_ډک] = 0;
  }

  quotaEnter();
  p_ډله = quota_ډله_پیدا_کړه(z_ډک);
  إذا( p_ډله ){
    لكل(pFile=p_ډله->pFiles; pFile && rc==SQLITE_OK; pFile=p_بل_File){
      p_بل_File = pFile->p_بل;
      diff = مزي_پرتله(z_ډک, pFile->zFilename, n_ډک);
      إذا( diff==0 && ((c = pFile->zFilename[n_ډک])==0 || c=='/' || c=='\\') ){
        إذا( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }وإلا{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quota_ډله_Deref(p_ډله);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(z_ډک);
  أرجع rc;
}

/***************************** Test Code ***********************************/
#که_تعریف_شوی SQLITE_TEST
#ضمن "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
تعريفنوع بنية TclQuotaCallback TclQuotaCallback;
بنية TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

خارجي ثابت حرف *sqlite3ErrName(صحيح);


/*
** This is the callback from a quota-over-limit.
*/
ساكن فراغ tclQuotaCallback(
  ثابت حرف *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *pi_حد,         /* IN/OUT: The current limit */
  sqlite3_int64 i_اندازه,            /* Total size of all files in the group */
  فراغ *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  بینښه صحيح rnd;               /* Random part of pVarname */
  صحيح rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  إذا( p==0 ) أرجع;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_شمېر(pVarname);
  sqlite3_randomness(حجم(rnd), (فراغ *)&rnd);
  Tcl_Append_Obj_منزل_Obj(pVarname, Tcl_NewIntObj((صحيح)(rnd&0x7FFFFFFF)));
  Tcl_Obj_سېټ_Var2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*pi_حد), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_شمېر(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(i_اندازه));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  إذا( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *p_حد = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, p_حد, &x);
    *pi_حد = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_شمېر(pEval);
  Tcl_Decr_Ref_شمېر(pVarname);
  إذا( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
ساكن فراغ tclCallbackDestructor(فراغ *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  إذا( p ){
    Tcl_Decr_Ref_شمېر(p->pScript);
    sqlite3_free((حرف *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
ساكن صحيح SQLITE_TCLAPI test_quota_initialize(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zName;              /* Name of new quota VFS */
  صحيح makeDefault;                /* True to make the new VFS the default */
  صحيح rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "NAME MAKEDEFAULT");
    أرجع TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  إذا( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) أرجع TCL_ERROR;
  إذا( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);

  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
ساكن صحيح SQLITE_TCLAPI test_quota_shutdown(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;                         /* Value returned by quota_shutdown() */

  إذا( objc!=1 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);

  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
ساكن صحيح SQLITE_TCLAPI test_quota_سېټ(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *z_نمونه;           /* File pattern to configure */
  Tcl_WideInt i_حد;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  صحيح rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_اندازه nScript;               /* Length of callback script */
  فراغ (*xDestroy)(فراغ*);        /* Optional destructor for pArg */
  فراغ (*xCallback)(ثابت حرف *, sqlite3_int64 *, sqlite3_int64, فراغ *);

  /* Process arguments */
  إذا( objc!=4 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    أرجع TCL_ERROR;
  }
  z_نمونه = Tcl_GetString(objv[1]);
  إذا( Tcl_GetWideIntFromObj(interp, objv[2], &i_حد) ) أرجع TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  إذا( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(حجم(TclQuotaCallback));
    إذا( !p ){
      Tcl_سېټ_پایله(interp, (حرف *)"SQLITE_NOMEM", TCL_STATIC);
      أرجع TCL_OK;
    }
    memset(p, 0, حجم(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_شمېر(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }وإلا{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_سېټ(z_نمونه, i_حد, xCallback, (فراغ*)p, xDestroy);

  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
ساكن صحيح SQLITE_TCLAPI test_quota_file(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zFilename;          /* File pattern to configure */
  صحيح rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "FILENAME");
    أرجع TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
  أرجع TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
ساكن صحيح SQLITE_TCLAPI test_quota_dump(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *p_پایله;
  Tcl_Obj *p_ډله_غړی;
  Tcl_Obj *p_File_غړی;
  quota_ډله *p_ډله;
  quotaFile *pFile;

  p_پایله = Tcl_NewObj();
  quotaEnter();
  لكل(p_ډله=gQuota.p_ډله; p_ډله; p_ډله=p_ډله->p_بل){
    p_ډله_غړی = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, p_ډله_غړی,
          Tcl_NewStringObj(p_ډله->z_نمونه, -1));
    Tcl_ListObjAppendElement(interp, p_ډله_غړی,
          Tcl_NewWideIntObj(p_ډله->i_حد));
    Tcl_ListObjAppendElement(interp, p_ډله_غړی,
          Tcl_NewWideIntObj(p_ډله->i_اندازه));
    لكل(pFile=p_ډله->pFiles; pFile; pFile=pFile->p_بل){
      صحيح i;
      حرف z_لنډمهاله[1000];
      p_File_غړی = Tcl_NewObj();
      sqlite3_snprintf(حجم(z_لنډمهاله), z_لنډمهاله, "%s", pFile->zFilename);
      لكل(i=0; z_لنډمهاله[i]; i++){ إذا( z_لنډمهاله[i]=='\\' ) z_لنډمهاله[i] = '/'; }
      Tcl_ListObjAppendElement(interp, p_File_غړی,
            Tcl_NewStringObj(z_لنډمهاله, -1));
      Tcl_ListObjAppendElement(interp, p_File_غړی,
            Tcl_NewWideIntObj(pFile->i_اندازه));
      Tcl_ListObjAppendElement(interp, p_File_غړی,
            Tcl_NewWideIntObj(pFile->nRef));
      Tcl_ListObjAppendElement(interp, p_File_غړی,
            Tcl_NewWideIntObj(pFile->deleteOnClose));
      Tcl_ListObjAppendElement(interp, p_ډله_غړی, p_File_غړی);
    }
    Tcl_ListObjAppendElement(interp, p_پایله, p_ډله_غړی);
  }
  quotaLeave();
  Tcl_سېټ_Obj_پایله(interp, p_پایله);
  أرجع TCL_OK;
}

/* Defined in test1.c */
خارجي فراغ *sqlite3_Test_متن_منزل_Ptr(ثابت حرف*);
خارجي ثابت حرف *sqlite3_Test_Ptr_منزل_متن(فراغ*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_fopen(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zFilename;          /* File pattern to configure */
  ثابت حرف *z_ډېرپېښ;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "FILENAME MODE");
    أرجع TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  z_ډېرپېښ = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, z_ډېرپېښ);
  Tcl_سېټ_پایله(interp, (حرف*)sqlite3_Test_Ptr_منزل_متن(p), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
ساكن صحيح SQLITE_TCLAPI test_quota_fread(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  حرف *zBuf;
  صحيح sz;
  صحيح nElem;
  size_t got;

  إذا( objc!=4 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE SIZE NELEM");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  إذا( Tcl_GetIntFromObj(interp, objv[2], &sz) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) أرجع TCL_ERROR;
  zBuf = (حرف*)sqlite3_malloc( sz*nElem + 1 );
  إذا( zBuf==0 ){
    Tcl_سېټ_پایله(interp, "out of memory", TCL_STATIC);
    أرجع TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_سېټ_پایله(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
ساكن صحيح SQLITE_TCLAPI test_quota_fwrite(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  حرف *zBuf;
  صحيح sz;
  صحيح nElem;
  size_t got;

  إذا( objc!=5 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  إذا( Tcl_GetIntFromObj(interp, objv[2], &sz) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) أرجع TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewWideIntObj(got));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_fclose(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
ساكن صحيح SQLITE_TCLAPI test_quota_fflush(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  صحيح rc;
  صحيح doSync = 0;

  إذا( objc!=2 && objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE ?HARDSYNC?");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  إذا( objc==3 ){
    إذا( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) أرجع TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_fseek(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  صحيح ofst;
  ثابت حرف *zWhence;
  صحيح whence;
  صحيح rc;

  إذا( objc!=4 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE OFFSET WHENCE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  إذا( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) أرجع TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  إذا( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }وإلا إذا( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }وإلا إذا( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }وإلا{
    Tcl_Append_پایله(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (حرف*)0);
    أرجع TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_rewind(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_ftell(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewWideIntObj(x));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_ftruncate(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  صحيح rc;
  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE SIZE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  إذا( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) أرجع TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_file_اندازه(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_اندازه(p);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewWideIntObj(x));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_file_truesize(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewWideIntObj(x));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
ساكن صحيح SQLITE_TCLAPI test_quota_file_mtime(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  وخت_t t;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewWideIntObj(t));
  أرجع TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
ساكن صحيح SQLITE_TCLAPI test_quota_remove(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zFilename;          /* File pattern to configure */
  صحيح rc;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "FILENAME");
    أرجع TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
ساكن صحيح SQLITE_TCLAPI test_quota_glob(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *z_نمونه;          /* The glob pattern */
  ثابت حرف *z_متن;             /* Text to compare against the pattern */
  صحيح rc;
  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "PATTERN TEXT");
    أرجع TCL_ERROR;
  }
  z_نمونه = Tcl_GetString(objv[1]);
  z_متن = Tcl_GetString(objv[2]);
  rc = quotaStrglob(z_نمونه, z_متن);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
ساكن صحيح SQLITE_TCLAPI test_quota_file_شته(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_شته(p);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewWideIntObj(x));
  أرجع TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
ساكن صحيح SQLITE_TCLAPI test_quota_ferror(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  صحيح x;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "HANDLE");
    أرجع TCL_ERROR;
  }
  p = sqlite3_Test_متن_منزل_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(x));
  أرجع TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
صحيح Sqlitequota_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_quota_initialize",    test_quota_initialize },
    { "sqlite3_quota_shutdown",      test_quota_shutdown },
    { "sqlite3_quota_set",           test_quota_سېټ },
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
    { "sqlite3_quota_file_size",     test_quota_file_اندازه },
    { "sqlite3_quota_file_truesize", test_quota_file_truesize },
    { "sqlite3_quota_file_mtime",    test_quota_file_mtime },
    { "sqlite3_quota_remove",        test_quota_remove },
    { "sqlite3_quota_glob",          test_quota_glob },
    { "sqlite3_quota_file_available",test_quota_file_شته },
    { "sqlite3_quota_ferror",        test_quota_ferror },
  };
  صحيح i;

  لكل(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  أرجع TCL_OK;
}
#د_که_پای
