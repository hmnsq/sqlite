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
#شامل "test_quota.h"
#شامل <string.h>
#شامل <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#اگر defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#تعریف_کرو sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#تعریف_کرو sqlite3_mutex_free(X)
#تعریف_کرو sqlite3_mutex_enter(X)
#تعریف_کرو sqlite3_mutex_try(X)      SQLITE_OK
#تعریف_کرو sqlite3_mutex_leave(X)
#تعریف_کرو sqlite3_mutex_held(X)     ((خالی)(X),1)
#تعریف_کرو sqlite3_mutex_notheld(X)  ((خالی)(X),1)
#اگر_ختم /* SQLITE_THREADSAFE==0 */


#اگر_تعریف_شدہ _WIN32
# include <windows.h>
# include <io.h>
#ورنہ
# include <unistd.h>
#اگر_ختم


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
قسمتعریف ساخت quota_گروہ quota_گروہ;
قسمتعریف ساخت quotaConn quotaConn;
قسمتعریف ساخت quotaFile quotaFile;

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
ساخت quota_گروہ {
  ثابت حرف *z_نمونہ;          /* Filename pattern to be quotaed */
  sqlite3_int64 i_حد_بندی;          /* Upper bound on total file size */
  sqlite3_int64 i_سائز;           /* Current size of all files */
  خالی (*xCallback)(             /* Callback invoked when going over quota */
     ثابت حرف *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_حد_بندی,        /* IN/OUT: The current limit */
     sqlite3_int64 i_سائز,           /* Total size of all files in the group */
     خالی *pArg                     /* Client data */
  );
  خالی *pArg;                    /* Third argument to the xCallback() */
  خالی (*xDestroy)(خالی*);       /* Optional destructor for pArg */
  quota_گروہ *p_اگلا, **pp_پچھلا;   /* Doubly linked list of all quota objects */
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
ساخت quotaFile {
  حرف *zFilename;                /* Name of this file */
  quota_گروہ *p_گروہ;             /* Quota group to which this file belongs */
  sqlite3_int64 i_سائز;            /* Current size of this file */
  صحیح nRef;                       /* Number of times this file is open */
  صحیح deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *p_اگلا, **pp_پچھلا;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ساخت quotaConn {
  sqlite3_file بنیاد;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
ساخت quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#اگر_تعریف_شدہ _WIN32
  حرف *zMbcsName;        /* Full MBCS pathname of the file */
#اگر_ختم
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
ساکن ساخت {
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
  صحیح isInitialized;

  /* For run-time access any of the other global data structures in this
  ** shim, the following mutex must be held.
  */
  sqlite3_mutex *pMutex;

  /* List of quotaGroup objects.
  */
  quota_گروہ *p_گروہ;

} gQuota;

/************************* Utility Routines *********************************/
/*
** Acquire and release the mutex used to serialize access to the
** list of quotaGroups.
*/
ساکن خالی quotaEnter(خالی){ sqlite3_mutex_enter(gQuota.pMutex); }
ساکن خالی quotaLeave(خالی){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
ساکن صحیح quota_گروہ_Open_File_گنتی(quota_گروہ *p_گروہ){
  صحیح N = 0;
  quotaFile *pFile = p_گروہ->pFiles;
  جب( pFile ){
    اگر( pFile->nRef ) N++;
    pFile = pFile->p_اگلا;
  }
  واپس N;
}

/* Remove a file from a quota group.
*/
ساکن خالی quotaRemoveFile(quotaFile *pFile){
  quota_گروہ *p_گروہ = pFile->p_گروہ;
  p_گروہ->i_سائز -= pFile->i_سائز;
  *pFile->pp_پچھلا = pFile->p_اگلا;
  اگر( pFile->p_اگلا ) pFile->p_اگلا->pp_پچھلا = pFile->pp_پچھلا;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
ساکن خالی quotaRemoveAllFiles(quota_گروہ *p_گروہ){
  جب( p_گروہ->pFiles ){
    أكد( p_گروہ->pFiles->nRef==0 );
    quotaRemoveFile(p_گروہ->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
ساکن خالی quota_گروہ_Deref(quota_گروہ *p_گروہ){
  اگر( p_گروہ->i_حد_بندی==0 && quota_گروہ_Open_File_گنتی(p_گروہ)==0 ){
    quotaRemoveAllFiles(p_گروہ);
    *p_گروہ->pp_پچھلا = p_گروہ->p_اگلا;
    اگر( p_گروہ->p_اگلا ) p_گروہ->p_اگلا->pp_پچھلا = p_گروہ->pp_پچھلا;
    اگر( p_گروہ->xDestroy ) p_گروہ->xDestroy(p_گروہ->pArg);
    sqlite3_free(p_گروہ);
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
ساکن صحیح quotaStrglob(ثابت حرف *zGlob, ثابت حرف *z){
  صحیح c, c2, cx;
  صحیح invert;
  صحیح نظر_آیا;

  جب( (c = (*(zGlob++)))!=0 ){
    اگر( c=='*' ){
      جب( (c=(*(zGlob++))) == '*' || c=='?' ){
        اگر( c=='?' && (*(z++))==0 ) واپس 0;
      }
      اگر( c==0 ){
        واپس 1;
      }ورنہ اگر( c=='[' ){
        جب( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        واپس (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      جب( (c2 = (*(z++)))!=0 ){
        جب( c2!=c && c2!=cx ){
          c2 = *(z++);
          اگر( c2==0 ) واپس 0;
        }
        اگر( quotaStrglob(zGlob,z) ) واپس 1;
      }
      واپس 0;
    }ورنہ اگر( c=='?' ){
      اگر( (*(z++))==0 ) واپس 0;
    }ورنہ اگر( c=='[' ){
      صحیح پیشگی_c = 0;
      نظر_آیا = 0;
      invert = 0;
      c = *(z++);
      اگر( c==0 ) واپس 0;
      c2 = *(zGlob++);
      اگر( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      اگر( c2==']' ){
        اگر( c==']' ) نظر_آیا = 1;
        c2 = *(zGlob++);
      }
      جب( c2 && c2!=']' ){
        اگر( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && پیشگی_c>0 ){
          c2 = *(zGlob++);
          اگر( c>=پیشگی_c && c<=c2 ) نظر_آیا = 1;
          پیشگی_c = 0;
        }ورنہ{
          اگر( c==c2 ){
            نظر_آیا = 1;
          }
          پیشگی_c = c2;
        }
        c2 = *(zGlob++);
      }
      اگر( c2==0 || (نظر_آیا ^ invert)==0 ) واپس 0;
    }ورنہ اگر( c=='/' ){
      اگر( z[0]!='/' && z[0]!='\\' ) واپس 0;
      z++;
    }ورنہ{
      اگر( c!=(*(z++)) ) واپس 0;
    }
  }
  واپس *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
ساکن quota_گروہ *quota_گروہ_ڈھونڈو(ثابت حرف *zFilename){
  quota_گروہ *p;
  برائے(p=gQuota.p_گروہ; p && quotaStrglob(p->z_نمونہ, zFilename)==0;
      p=p->p_اگلا){}
  واپس p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
ساکن sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  واپس (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
ساکن quotaFile *quota_ڈھونڈو_File(
  quota_گروہ *p_گروہ,     /* Group in which to look for the file */
  ثابت حرف *zName,      /* Full pathname of the file */
  صحیح createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = p_گروہ->pFiles;
  جب( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->p_اگلا;
  }
  اگر( pFile==0 && createFlag ){
    صحیح nName = (صحیح)(لڑی_لمبائی(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( حجم(*pFile) + nName + 1 );
    اگر( pFile ){
      memset(pFile, 0, حجم(*pFile));
      pFile->zFilename = (حرف*)&pFile[1];
      حافظہ_نقل(pFile->zFilename, zName, nName+1);
      pFile->p_اگلا = p_گروہ->pFiles;
      اگر( p_گروہ->pFiles ) p_گروہ->pFiles->pp_پچھلا = &pFile->p_اگلا;
      pFile->pp_پچھلا = &p_گروہ->pFiles;
      p_گروہ->pFiles = pFile;
      pFile->p_گروہ = p_گروہ;
    }
  }
  واپس pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
ساکن حرف *quota_utf8_مقصد_mbcs(ثابت حرف *zUtf8){
#اگر_تعریف_شدہ _WIN32
  size_t n;          /* Bytes in zUtf8 */
  صحیح nWide;         /* number of UTF-16 characters */
  صحیح nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  حرف *zMbcs;       /* The MBCS text */
  صحیح codepage;      /* Code page used by fopen() */

  n = لڑی_لمبائی(zUtf8);
  nWide = Multi_Byte_مقصد_Wide_Char(CP_UTF8, 0, zUtf8, -1, تہی, 0);
  اگر( nWide==0 ) واپس 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*حجم(zTmpWide[0]) );
  اگر( zTmpWide==0 ) واپس 0;
  Multi_Byte_مقصد_Wide_Char(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = Wide_Char_مقصد_Multi_Byte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (حرف*)sqlite3_malloc( nMbcs+1 ) : 0;
  اگر( zMbcs ){
    Wide_Char_مقصد_Multi_Byte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  واپس zMbcs;
#ورنہ
  واپس (حرف*)zUtf8;  /* No-op on unix */
#اگر_ختم
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
ساکن خالی quota_mbcs_free(حرف *zOld){
#اگر_تعریف_شدہ _WIN32
  sqlite3_free(zOld);
#ورنہ
  /* No-op on unix */
#اگر_ختم
}

/************************* VFS Method Wrappers *****************************/
/*
** This is the xOpen method used for the "quota" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate quota group if it is a
** file that needs to be tracked.
*/
ساکن صحیح quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ثابت حرف *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  صحیح flags,                  /* Flags to control the opening */
  صحیح *p_برآمد_Flags              /* Flags showing results of opening */
){
  صحیح rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quota_گروہ *p_گروہ;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  اگر( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    واپس pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_برآمد_Flags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  p_گروہ = quota_گروہ_ڈھونڈو(zName);
  اگر( p_گروہ==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_برآمد_Flags);
  }ورنہ{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, p_برآمد_Flags);
    اگر( rc==SQLITE_OK ){
      pFile = quota_ڈھونڈو_File(p_گروہ, zName, 1);
      اگر( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        واپس SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      اگر( pSubOpen->pMethods->i_نسخہ==1 ){
        pQuotaOpen->بنیاد.pMethods = &gQuota.sIoMethodsV1;
      }ورنہ{
        pQuotaOpen->بنیاد.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  واپس rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
ساکن صحیح quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ثابت حرف *zName,          /* Name of file to be deleted */
  صحیح syncDir                 /* Do a directory sync after deleting */
){
  صحیح rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quota_گروہ *p_گروہ;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  اگر( rc==SQLITE_OK ){
    quotaEnter();
    p_گروہ = quota_گروہ_ڈھونڈو(zName);
    اگر( p_گروہ ){
      pFile = quota_ڈھونڈو_File(p_گروہ, zName, 0);
      اگر( pFile ){
        اگر( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ورنہ{
          quotaRemoveFile(pFile);
          quota_گروہ_Deref(p_گروہ);
        }
      }
    }
    quotaLeave();
  }
  واپس rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
ساکن صحیح quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  صحیح rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  اگر( pFile->nRef==0 ){
    quota_گروہ *p_گروہ = pFile->p_گروہ;
    اگر( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quota_گروہ_Deref(p_گروہ);
  }
  quotaLeave();
  واپس rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
ساکن صحیح quotaRead(
  sqlite3_file *pConn,
  خالی *pBuf,
  صحیح iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
ساکن صحیح quotaWrite(
  sqlite3_file *pConn,
  ثابت خالی *pBuf,
  صحیح iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 i_انتہا = iOfst+iAmt;
  quota_گروہ *p_گروہ;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  اگر( pFile->i_سائز<i_انتہا ){
    p_گروہ = pFile->p_گروہ;
    quotaEnter();
    szNew = p_گروہ->i_سائز - pFile->i_سائز + i_انتہا;
    اگر( szNew>p_گروہ->i_حد_بندی && p_گروہ->i_حد_بندی>0 ){
      اگر( p_گروہ->xCallback ){
        p_گروہ->xCallback(pFile->zFilename, &p_گروہ->i_حد_بندی, szNew,
                          p_گروہ->pArg);
      }
      اگر( szNew>p_گروہ->i_حد_بندی && p_گروہ->i_حد_بندی>0 ){
        quotaLeave();
        واپس SQLITE_بھرا;
      }
    }
    p_گروہ->i_سائز = szNew;
    pFile->i_سائز = i_انتہا;
    quotaLeave();
  }
  واپس pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
ساکن صحیح quotaTruncate(sqlite3_file *pConn, sqlite3_int64 سائز){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  صحیح rc = pSubOpen->pMethods->xTruncate(pSubOpen, سائز);
  quotaFile *pFile = p->pFile;
  quota_گروہ *p_گروہ;
  اگر( rc==SQLITE_OK ){
    quotaEnter();
    p_گروہ = pFile->p_گروہ;
    p_گروہ->i_سائز -= pFile->i_سائز;
    pFile->i_سائز = سائز;
    p_گروہ->i_سائز += سائز;
    quotaLeave();
  }
  واپس rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ساکن صحیح quotaSync(sqlite3_file *pConn, صحیح flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
ساکن صحیح quota_File_سائز(sqlite3_file *pConn, sqlite3_int64 *p_سائز){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quota_گروہ *p_گروہ;
  sqlite3_int64 sz;
  صحیح rc;

  rc = pSubOpen->pMethods->x_File_سائز(pSubOpen, &sz);
  اگر( rc==SQLITE_OK ){
    quotaEnter();
    p_گروہ = pFile->p_گروہ;
    p_گروہ->i_سائز -= pFile->i_سائز;
    pFile->i_سائز = sz;
    p_گروہ->i_سائز += sz;
    quotaLeave();
    *p_سائز = sz;
  }
  واپس rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ساکن صحیح quota_تالا(sqlite3_file *pConn, صحیح تالا){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->x_تالا(pSubOpen, تالا);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ساکن صحیح quotaUnlock(sqlite3_file *pConn, صحیح تالا){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->xUnlock(pSubOpen, تالا);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ساکن صحیح quota_Check_Reserved_تالا(sqlite3_file *pConn, صحیح *p_Res_برآمد){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->x_Check_Reserved_تالا(pSubOpen, p_Res_برآمد);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
ساکن صحیح quotaFileControl(sqlite3_file *pConn, صحیح op, خالی *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  صحیح rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#اگر defined(SQLITE_FCNTL_VFSNAME)
  اگر( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(حرف**)pArg = sqlite3_mprintf("quota/%z", *(حرف**)pArg);
  }
#اگر_ختم
  واپس rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ساکن صحیح quota_Sector_سائز(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->x_Sector_سائز(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ساکن صحیح quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ساکن صحیح quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  صحیح iRegion,                    /* Region to retrieve */
  صحیح szRegion,                   /* Size of regions */
  صحیح bExtend,                    /* True to extend file if necessary */
  خالی متلون **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ساکن صحیح quota_Shm_تالا(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  صحیح ofst,                  /* First lock to acquire or release */
  صحیح n,                     /* Number of locks to acquire or release */
  صحیح flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->x_Shm_تالا(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ساکن خالی quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ساکن صحیح quotaShmUnmap(sqlite3_file *pConn, صحیح deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  واپس pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
صحیح sqlite3_quota_initialize(ثابت حرف *zOrigVfsName, صحیح makeDefault){
  sqlite3_vfs *pOrigVfs;
  اگر( gQuota.isInitialized ) واپس SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_ڈھونڈو(zOrigVfsName);
  اگر( pOrigVfs==0 ) واپس SQLITE_ERROR;
  أكد( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  اگر( !gQuota.pMutex ){
    واپس SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += حجم(quotaConn);
  gQuota.sThisVfs.zName = "quota";
  gQuota.sIoMethodsV1.i_نسخہ = 1;
  gQuota.sIoMethodsV1.xClose = quotaClose;
  gQuota.sIoMethodsV1.xRead = quotaRead;
  gQuota.sIoMethodsV1.xWrite = quotaWrite;
  gQuota.sIoMethodsV1.xTruncate = quotaTruncate;
  gQuota.sIoMethodsV1.xSync = quotaSync;
  gQuota.sIoMethodsV1.x_File_سائز = quota_File_سائز;
  gQuota.sIoMethodsV1.x_تالا = quota_تالا;
  gQuota.sIoMethodsV1.xUnlock = quotaUnlock;
  gQuota.sIoMethodsV1.x_Check_Reserved_تالا = quota_Check_Reserved_تالا;
  gQuota.sIoMethodsV1.xFileControl = quotaFileControl;
  gQuota.sIoMethodsV1.x_Sector_سائز = quota_Sector_سائز;
  gQuota.sIoMethodsV1.xDeviceCharacteristics = quotaDeviceCharacteristics;
  gQuota.sIoMethodsV2 = gQuota.sIoMethodsV1;
  gQuota.sIoMethodsV2.i_نسخہ = 2;
  gQuota.sIoMethodsV2.xShmMap = quotaShmMap;
  gQuota.sIoMethodsV2.x_Shm_تالا = quota_Shm_تالا;
  gQuota.sIoMethodsV2.xShmBarrier = quotaShmBarrier;
  gQuota.sIoMethodsV2.xShmUnmap = quotaShmUnmap;
  sqlite3_vfs_رجسٹر(&gQuota.sThisVfs, makeDefault);
  واپس SQLITE_OK;
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
صحیح sqlite3_quota_shutdown(خالی){
  quota_گروہ *p_گروہ;
  اگر( gQuota.isInitialized==0 ) واپس SQLITE_MISUSE;
  برائے(p_گروہ=gQuota.p_گروہ; p_گروہ; p_گروہ=p_گروہ->p_اگلا){
    اگر( quota_گروہ_Open_File_گنتی(p_گروہ)>0 ) واپس SQLITE_MISUSE;
  }
  جب( gQuota.p_گروہ ){
    p_گروہ = gQuota.p_گروہ;
    gQuota.p_گروہ = p_گروہ->p_اگلا;
    p_گروہ->i_حد_بندی = 0;
    أكد( quota_گروہ_Open_File_گنتی(p_گروہ)==0 );
    quota_گروہ_Deref(p_گروہ);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, حجم(gQuota));
  واپس SQLITE_OK;
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
صحیح sqlite3_quota_سیٹ(
  ثابت حرف *z_نمونہ,           /* The filename pattern */
  sqlite3_int64 i_حد_بندی,           /* New quota to set for this quota group */
  خالی (*xCallback)(              /* Callback invoked when going over quota */
     ثابت حرف *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_حد_بندی,        /* IN/OUT: The current limit */
     sqlite3_int64 i_سائز,           /* Total size of all files in the group */
     خالی *pArg                     /* Client data */
  ),
  خالی *pArg,                     /* client data passed thru to callback */
  خالی (*xDestroy)(خالی*)         /* Optional destructor for pArg */
){
  quota_گروہ *p_گروہ;
  quotaEnter();
  p_گروہ = gQuota.p_گروہ;
  جب( p_گروہ && strcmp(p_گروہ->z_نمونہ, z_نمونہ)!=0 ){
    p_گروہ = p_گروہ->p_اگلا;
  }
  اگر( p_گروہ==0 ){
    صحیح n_نمونہ = (صحیح)(لڑی_لمبائی(z_نمونہ) & 0x3fffffff);
    اگر( i_حد_بندی<=0 ){
      quotaLeave();
      واپس SQLITE_OK;
    }
    p_گروہ = (quota_گروہ *)sqlite3_malloc( حجم(*p_گروہ) + n_نمونہ + 1 );
    اگر( p_گروہ==0 ){
      quotaLeave();
      واپس SQLITE_NOMEM;
    }
    memset(p_گروہ, 0, حجم(*p_گروہ));
    p_گروہ->z_نمونہ = (حرف*)&p_گروہ[1];
    حافظہ_نقل((حرف *)p_گروہ->z_نمونہ, z_نمونہ, n_نمونہ+1);
    اگر( gQuota.p_گروہ ) gQuota.p_گروہ->pp_پچھلا = &p_گروہ->p_اگلا;
    p_گروہ->p_اگلا = gQuota.p_گروہ;
    p_گروہ->pp_پچھلا = &gQuota.p_گروہ;
    gQuota.p_گروہ = p_گروہ;
  }
  p_گروہ->i_حد_بندی = i_حد_بندی;
  p_گروہ->xCallback = xCallback;
  اگر( p_گروہ->xDestroy && p_گروہ->pArg!=pArg ){
    p_گروہ->xDestroy(p_گروہ->pArg);
  }
  p_گروہ->pArg = pArg;
  p_گروہ->xDestroy = xDestroy;
  quota_گروہ_Deref(p_گروہ);
  quotaLeave();
  واپس SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
صحیح sqlite3_quota_file(ثابت حرف *zFilename){
  حرف *z_بھرا = 0;
  sqlite3_file *fd;
  صحیح rc;
  صحیح برآمد_Flags = 0;
  sqlite3_int64 i_سائز;
  صحیح nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  اگر( fd==0 ){
    rc = SQLITE_NOMEM;
  }ورنہ{
    z_بھرا = &((حرف *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->x_بھرا_Pathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, z_بھرا);
  }

  اگر( rc==SQLITE_OK ){
    z_بھرا[لڑی_لمبائی(z_بھرا)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, z_بھرا, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &برآمد_Flags);
    اگر( rc==SQLITE_OK ){
      fd->pMethods->x_File_سائز(fd, &i_سائز);
      fd->pMethods->xClose(fd);
    }ورنہ اگر( rc==SQLITE_CANTOPEN ){
      quota_گروہ *p_گروہ;
      quotaFile *pFile;
      quotaEnter();
      p_گروہ = quota_گروہ_ڈھونڈو(z_بھرا);
      اگر( p_گروہ ){
        pFile = quota_ڈھونڈو_File(p_گروہ, z_بھرا, 0);
        اگر( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  واپس rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(ثابت حرف *zFilename, ثابت حرف *z_منوال){
  quota_FILE *p = 0;
  حرف *z_بھرا = 0;
  حرف *z_بھرا_Translated = 0;
  صحیح rc;
  quota_گروہ *p_گروہ;
  quotaFile *pFile;

  z_بھرا = (حرف*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  اگر( z_بھرا==0 ) واپس 0;
  rc = gQuota.pOrigVfs->x_بھرا_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_بھرا);
  اگر( rc ) جاؤ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(حجم(*p));
  اگر( p==0 ) جاؤ quota_fopen_error;
  memset(p, 0, حجم(*p));
  z_بھرا_Translated = quota_utf8_مقصد_mbcs(z_بھرا);
  اگر( z_بھرا_Translated==0 ) جاؤ quota_fopen_error;
  p->f = fopen(z_بھرا_Translated, z_منوال);
  اگر( p->f==0 ) جاؤ quota_fopen_error;
  quotaEnter();
  p_گروہ = quota_گروہ_ڈھونڈو(z_بھرا);
  اگر( p_گروہ ){
    pFile = quota_ڈھونڈو_File(p_گروہ, z_بھرا, 1);
    اگر( pFile==0 ){
      quotaLeave();
      جاؤ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(z_بھرا);
#اگر_تعریف_شدہ _WIN32
  p->zMbcsName = z_بھرا_Translated;
#اگر_ختم
  واپس p;

quota_fopen_error:
  quota_mbcs_free(z_بھرا_Translated);
  sqlite3_free(z_بھرا);
  اگر( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  واپس 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  خالی *pBuf,            /* Store the content here */
  size_t سائز,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  واپس fread(pBuf, سائز, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  ثابت خالی *pBuf,      /* Take content to write from here */
  size_t سائز,           /* Size of each element */
  size_t nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 i_انتہا;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  size_t rc;

  iOfst = ftell(p->f);
  i_انتہا = iOfst + سائز*nmemb;
  pFile = p->pFile;
  اگر( pFile && pFile->i_سائز<i_انتہا ){
    quota_گروہ *p_گروہ = pFile->p_گروہ;
    quotaEnter();
    szNew = p_گروہ->i_سائز - pFile->i_سائز + i_انتہا;
    اگر( szNew>p_گروہ->i_حد_بندی && p_گروہ->i_حد_بندی>0 ){
      اگر( p_گروہ->xCallback ){
        p_گروہ->xCallback(pFile->zFilename, &p_گروہ->i_حد_بندی, szNew,
                          p_گروہ->pArg);
      }
      اگر( szNew>p_گروہ->i_حد_بندی && p_گروہ->i_حد_بندی>0 ){
        i_انتہا = p_گروہ->i_حد_بندی - p_گروہ->i_سائز + pFile->i_سائز;
        nmemb = (size_t)((i_انتہا - iOfst)/سائز);
        i_انتہا = iOfst + سائز*nmemb;
        szNew = p_گروہ->i_سائز - pFile->i_سائز + i_انتہا;
      }
    }
    p_گروہ->i_سائز = szNew;
    pFile->i_سائز = i_انتہا;
    quotaLeave();
  }ورنہ{
    pFile = 0;
  }
  rc = fwrite(pBuf, سائز, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  اگر( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 i_New_انتہا = iOfst + سائز*nWritten;
    اگر( i_New_انتہا<i_انتہا ) i_New_انتہا = i_انتہا;
    quotaEnter();
    pFile->p_گروہ->i_سائز += i_New_انتہا - pFile->i_سائز;
    pFile->i_سائز = i_New_انتہا;
    quotaLeave();
  }
  واپس rc;
}

/*
** Close an open quota_FILE stream.
*/
صحیح sqlite3_quota_fclose(quota_FILE *p){
  صحیح rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  اگر( pFile ){
    quotaEnter();
    pFile->nRef--;
    اگر( pFile->nRef==0 ){
      quota_گروہ *p_گروہ = pFile->p_گروہ;
      اگر( pFile->deleteOnClose ){
        gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
        quotaRemoveFile(pFile);
      }
      quota_گروہ_Deref(p_گروہ);
    }
    quotaLeave();
  }
#اگر_تعریف_شدہ _WIN32
  quota_mbcs_free(p->zMbcsName);
#اگر_ختم
  sqlite3_free(p);
  واپس rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
صحیح sqlite3_quota_fflush(quota_FILE *p, صحیح doFsync){
  صحیح rc;
  rc = fflush(p->f);
  اگر( rc==0 && doFsync ){
#اگر_تعریف_شدہ _WIN32
    rc = _commit(_fileno(p->f));
#ورنہ
    rc = fsync(fileno(p->f));
#اگر_ختم
  }
  واپس rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
صحیح sqlite3_quota_fseek(quota_FILE *p, طویل انحراف, صحیح whence){
  واپس fseek(p->f, انحراف, whence);
}

/*
** rewind a quota_FILE stream.
*/
خالی sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
طویل sqlite3_quota_ftell(quota_FILE *p){
  واپس ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
صحیح sqlite3_quota_ferror(quota_FILE *p){
  واپس ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
صحیح sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  صحیح rc;
  اگر( (pFile = p->pFile)!=0 && pFile->i_سائز<szNew ){
    quota_گروہ *p_گروہ;
    اگر( pFile->i_سائز<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      واپس -1;
    }
    p_گروہ = pFile->p_گروہ;
    quotaEnter();
    p_گروہ->i_سائز += szNew - pFile->i_سائز;
    quotaLeave();
  }
#اگر_تعریف_شدہ _WIN32
#  اگر defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (طویل)szNew);
#  ورنہ
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#ورنہ
  rc = ftruncate(fileno(p->f), szNew);
#اگر_ختم
  اگر( pFile && rc==0 ){
    quota_گروہ *p_گروہ = pFile->p_گروہ;
    quotaEnter();
    p_گروہ->i_سائز += szNew - pFile->i_سائز;
    pFile->i_سائز = szNew;
    quotaLeave();
  }
  واپس rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
صحیح sqlite3_quota_file_mtime(quota_FILE *p, وقت_t *p_وقت){
  صحیح rc;
#اگر_تعریف_شدہ _WIN32
  ساخت _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ورنہ
  ساخت stat buf;
  rc = fstat(fileno(p->f), &buf);
#اگر_ختم
  اگر( rc==0 ) *p_وقت = buf.st_mtime;
  واپس rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  صحیح rc;
#اگر_تعریف_شدہ _WIN32
  ساخت _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ورنہ
  ساخت stat buf;
  rc = fstat(fileno(p->f), &buf);
#اگر_ختم
  واپس rc==0 ? buf.st_سائز : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_سائز(quota_FILE *p){
  واپس p->pFile ? p->pFile->i_سائز : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
طویل sqlite3_quota_file_دستیاب(quota_FILE *p){
  FILE* f = p->f;
  طویل pos1, pos2;
  صحیح rc;
  pos1 = ftell(f);
  اگر ( pos1 < 0 ) واپس -1;
  rc = fseek(f, 0, SEEK_END);
  اگر ( rc != 0 ) واپس -1;
  pos2 = ftell(f);
  اگر ( pos2 < 0 ) واپس -1;
  rc = fseek(f, pos1, SEEK_SET);
  اگر ( rc != 0 ) واپس -1;
  واپس pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
صحیح sqlite3_quota_remove(ثابت حرف *zFilename){
  حرف *z_بھرا;            /* Full pathname for zFilename */
  size_t n_بھرا;           /* Number of bytes in zFilename */
  صحیح rc;                 /* Result code */
  quota_گروہ *p_گروہ;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *p_اگلا_File;   /* next file in the group */
  صحیح diff;               /* Difference between filenames */
  حرف c;                 /* First character past end of pattern */

  z_بھرا = (حرف*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  اگر( z_بھرا==0 ) واپس SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->x_بھرا_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_بھرا);
  اگر( rc ){
    sqlite3_free(z_بھرا);
    واپس rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  n_بھرا = لڑی_لمبائی(z_بھرا);
  اگر( n_بھرا>0 && (z_بھرا[n_بھرا-1]=='/' || z_بھرا[n_بھرا-1]=='\\') ){
    n_بھرا--;
    z_بھرا[n_بھرا] = 0;
  }

  quotaEnter();
  p_گروہ = quota_گروہ_ڈھونڈو(z_بھرا);
  اگر( p_گروہ ){
    برائے(pFile=p_گروہ->pFiles; pFile && rc==SQLITE_OK; pFile=p_اگلا_File){
      p_اگلا_File = pFile->p_اگلا;
      diff = لڑی_موازنہ(z_بھرا, pFile->zFilename, n_بھرا);
      اگر( diff==0 && ((c = pFile->zFilename[n_بھرا])==0 || c=='/' || c=='\\') ){
        اگر( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ورنہ{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quota_گروہ_Deref(p_گروہ);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(z_بھرا);
  واپس rc;
}

/***************************** Test Code ***********************************/
#اگر_تعریف_شدہ SQLITE_TEST
#شامل "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
قسمتعریف ساخت TclQuotaCallback TclQuotaCallback;
ساخت TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

خارجی ثابت حرف *sqlite3ErrName(صحیح);


/*
** This is the callback from a quota-over-limit.
*/
ساکن خالی tclQuotaCallback(
  ثابت حرف *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *pi_حد_بندی,         /* IN/OUT: The current limit */
  sqlite3_int64 i_سائز,            /* Total size of all files in the group */
  خالی *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  غیرعلامتی صحیح rnd;               /* Random part of pVarname */
  صحیح rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  اگر( p==0 ) واپس;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_گنتی(pVarname);
  sqlite3_randomness(حجم(rnd), (خالی *)&rnd);
  Tcl_Append_Obj_مقصد_Obj(pVarname, Tcl_NewIntObj((صحیح)(rnd&0x7FFFFFFF)));
  Tcl_Obj_سیٹ_Var2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*pi_حد_بندی), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_گنتی(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(i_سائز));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  اگر( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *p_حد_بندی = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, p_حد_بندی, &x);
    *pi_حد_بندی = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_گنتی(pEval);
  Tcl_Decr_Ref_گنتی(pVarname);
  اگر( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
ساکن خالی tclCallbackDestructor(خالی *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  اگر( p ){
    Tcl_Decr_Ref_گنتی(p->pScript);
    sqlite3_free((حرف *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
ساکن صحیح SQLITE_TCLAPI test_quota_initialize(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zName;              /* Name of new quota VFS */
  صحیح makeDefault;                /* True to make the new VFS the default */
  صحیح rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "NAME MAKEDEFAULT");
    واپس TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  اگر( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) واپس TCL_ERROR;
  اگر( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);

  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
ساکن صحیح SQLITE_TCLAPI test_quota_shutdown(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;                         /* Value returned by quota_shutdown() */

  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);

  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
ساکن صحیح SQLITE_TCLAPI test_quota_سیٹ(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *z_نمونہ;           /* File pattern to configure */
  Tcl_WideInt i_حد_بندی;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  صحیح rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_سائز nScript;               /* Length of callback script */
  خالی (*xDestroy)(خالی*);        /* Optional destructor for pArg */
  خالی (*xCallback)(ثابت حرف *, sqlite3_int64 *, sqlite3_int64, خالی *);

  /* Process arguments */
  اگر( objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    واپس TCL_ERROR;
  }
  z_نمونہ = Tcl_GetString(objv[1]);
  اگر( Tcl_GetWideIntFromObj(interp, objv[2], &i_حد_بندی) ) واپس TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  اگر( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(حجم(TclQuotaCallback));
    اگر( !p ){
      Tcl_سیٹ_نتیجہ(interp, (حرف *)"SQLITE_NOMEM", TCL_STATIC);
      واپس TCL_OK;
    }
    memset(p, 0, حجم(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_گنتی(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }ورنہ{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_سیٹ(z_نمونہ, i_حد_بندی, xCallback, (خالی*)p, xDestroy);

  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
ساکن صحیح SQLITE_TCLAPI test_quota_file(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zFilename;          /* File pattern to configure */
  صحیح rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "FILENAME");
    واپس TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
  واپس TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
ساکن صحیح SQLITE_TCLAPI test_quota_dump(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *p_نتیجہ;
  Tcl_Obj *p_گروہ_رکن;
  Tcl_Obj *p_File_رکن;
  quota_گروہ *p_گروہ;
  quotaFile *pFile;

  p_نتیجہ = Tcl_NewObj();
  quotaEnter();
  برائے(p_گروہ=gQuota.p_گروہ; p_گروہ; p_گروہ=p_گروہ->p_اگلا){
    p_گروہ_رکن = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, p_گروہ_رکن,
          Tcl_NewStringObj(p_گروہ->z_نمونہ, -1));
    Tcl_ListObjAppendElement(interp, p_گروہ_رکن,
          Tcl_NewWideIntObj(p_گروہ->i_حد_بندی));
    Tcl_ListObjAppendElement(interp, p_گروہ_رکن,
          Tcl_NewWideIntObj(p_گروہ->i_سائز));
    برائے(pFile=p_گروہ->pFiles; pFile; pFile=pFile->p_اگلا){
      صحیح i;
      حرف z_عارضی[1000];
      p_File_رکن = Tcl_NewObj();
      sqlite3_snprintf(حجم(z_عارضی), z_عارضی, "%s", pFile->zFilename);
      برائے(i=0; z_عارضی[i]; i++){ اگر( z_عارضی[i]=='\\' ) z_عارضی[i] = '/'; }
      Tcl_ListObjAppendElement(interp, p_File_رکن,
            Tcl_NewStringObj(z_عارضی, -1));
      Tcl_ListObjAppendElement(interp, p_File_رکن,
            Tcl_NewWideIntObj(pFile->i_سائز));
      Tcl_ListObjAppendElement(interp, p_File_رکن,
            Tcl_NewWideIntObj(pFile->nRef));
      Tcl_ListObjAppendElement(interp, p_File_رکن,
            Tcl_NewWideIntObj(pFile->deleteOnClose));
      Tcl_ListObjAppendElement(interp, p_گروہ_رکن, p_File_رکن);
    }
    Tcl_ListObjAppendElement(interp, p_نتیجہ, p_گروہ_رکن);
  }
  quotaLeave();
  Tcl_سیٹ_Obj_نتیجہ(interp, p_نتیجہ);
  واپس TCL_OK;
}

/* Defined in test1.c */
خارجی خالی *sqlite3_Test_متن_مقصد_Ptr(ثابت حرف*);
خارجی ثابت حرف *sqlite3_Test_Ptr_مقصد_متن(خالی*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_fopen(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zFilename;          /* File pattern to configure */
  ثابت حرف *z_منوال;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "FILENAME MODE");
    واپس TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  z_منوال = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, z_منوال);
  Tcl_سیٹ_نتیجہ(interp, (حرف*)sqlite3_Test_Ptr_مقصد_متن(p), TCL_VOLATILE);
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
ساکن صحیح SQLITE_TCLAPI test_quota_fread(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  حرف *zBuf;
  صحیح sz;
  صحیح nElem;
  size_t got;

  اگر( objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE SIZE NELEM");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  اگر( Tcl_GetIntFromObj(interp, objv[2], &sz) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) واپس TCL_ERROR;
  zBuf = (حرف*)sqlite3_malloc( sz*nElem + 1 );
  اگر( zBuf==0 ){
    Tcl_سیٹ_نتیجہ(interp, "out of memory", TCL_STATIC);
    واپس TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_سیٹ_نتیجہ(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
ساکن صحیح SQLITE_TCLAPI test_quota_fwrite(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  حرف *zBuf;
  صحیح sz;
  صحیح nElem;
  size_t got;

  اگر( objc!=5 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  اگر( Tcl_GetIntFromObj(interp, objv[2], &sz) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) واپس TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(got));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_fclose(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
ساکن صحیح SQLITE_TCLAPI test_quota_fflush(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  صحیح rc;
  صحیح doSync = 0;

  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE ?HARDSYNC?");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  اگر( objc==3 ){
    اگر( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) واپس TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_fseek(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  صحیح ofst;
  ثابت حرف *zWhence;
  صحیح whence;
  صحیح rc;

  اگر( objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE OFFSET WHENCE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  اگر( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) واپس TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  اگر( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }ورنہ اگر( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }ورنہ اگر( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }ورنہ{
    Tcl_Append_نتیجہ(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (حرف*)0);
    واپس TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_rewind(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_ftell(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(x));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_ftruncate(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  صحیح rc;
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE SIZE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  اگر( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) واپس TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_file_سائز(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_سائز(p);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(x));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_file_truesize(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(x));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
ساکن صحیح SQLITE_TCLAPI test_quota_file_mtime(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  وقت_t t;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(t));
  واپس TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
ساکن صحیح SQLITE_TCLAPI test_quota_remove(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zFilename;          /* File pattern to configure */
  صحیح rc;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "FILENAME");
    واپس TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
ساکن صحیح SQLITE_TCLAPI test_quota_glob(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *z_نمونہ;          /* The glob pattern */
  ثابت حرف *z_متن;             /* Text to compare against the pattern */
  صحیح rc;
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "PATTERN TEXT");
    واپس TCL_ERROR;
  }
  z_نمونہ = Tcl_GetString(objv[1]);
  z_متن = Tcl_GetString(objv[2]);
  rc = quotaStrglob(z_نمونہ, z_متن);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
ساکن صحیح SQLITE_TCLAPI test_quota_file_دستیاب(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_دستیاب(p);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(x));
  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
ساکن صحیح SQLITE_TCLAPI test_quota_ferror(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  صحیح x;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE");
    واپس TCL_ERROR;
  }
  p = sqlite3_Test_متن_مقصد_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(x));
  واپس TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
صحیح Sqlitequota_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_quota_initialize",    test_quota_initialize },
    { "sqlite3_quota_shutdown",      test_quota_shutdown },
    { "sqlite3_quota_set",           test_quota_سیٹ },
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
    { "sqlite3_quota_file_size",     test_quota_file_سائز },
    { "sqlite3_quota_file_truesize", test_quota_file_truesize },
    { "sqlite3_quota_file_mtime",    test_quota_file_mtime },
    { "sqlite3_quota_remove",        test_quota_remove },
    { "sqlite3_quota_glob",          test_quota_glob },
    { "sqlite3_quota_file_available",test_quota_file_دستیاب },
    { "sqlite3_quota_ferror",        test_quota_ferror },
  };
  صحیح i;

  برائے(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  واپس TCL_OK;
}
#اگر_ختم
