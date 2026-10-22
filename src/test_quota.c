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
#ਅਨ੍ਤਰ੍ਭਾਵ "test_quota.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#ਜੇ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_free(X)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_enter(X)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_try(X)      SQLITE_OK
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_leave(X)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_held(X)     ((ਖਾਲੀ)(X),1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_notheld(X)  ((ਖਾਲੀ)(X),1)
#ਜੇ_ਅੰਤ /* SQLITE_THREADSAFE==0 */


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
# include <windows.h>
# include <io.h>
#ਨਹੀਂਤਾਂ
# include <unistd.h>
#ਜੇ_ਅੰਤ


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ quota_ਸਮੂਹ quota_ਸਮੂਹ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ quotaConn quotaConn;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ quotaFile quotaFile;

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
ਬਣਤਰ quota_ਸਮੂਹ {
  ਅਚਲ ਅੱਖਰ *z_ਨਮੂਨਾ;          /* Filename pattern to be quotaed */
  sqlite3_int64 i_ਹੱਦਬੰਦੀ;          /* Upper bound on total file size */
  sqlite3_int64 i_ਮਾਪ;           /* Current size of all files */
  ਖਾਲੀ (*xCallback)(             /* Callback invoked when going over quota */
     ਅਚਲ ਅੱਖਰ *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_ਹੱਦਬੰਦੀ,        /* IN/OUT: The current limit */
     sqlite3_int64 i_ਮਾਪ,           /* Total size of all files in the group */
     ਖਾਲੀ *pArg                     /* Client data */
  );
  ਖਾਲੀ *pArg;                    /* Third argument to the xCallback() */
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ*);       /* Optional destructor for pArg */
  quota_ਸਮੂਹ *p_ਅਗਲਾ, **ppPrev;   /* Doubly linked list of all quota objects */
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
ਬਣਤਰ quotaFile {
  ਅੱਖਰ *zFilename;                /* Name of this file */
  quota_ਸਮੂਹ *p_ਸਮੂਹ;             /* Quota group to which this file belongs */
  sqlite3_int64 i_ਮਾਪ;            /* Current size of this file */
  ਪੂਰਨ nRef;                       /* Number of times this file is open */
  ਪੂਰਨ deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *p_ਅਗਲਾ, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ਬਣਤਰ quotaConn {
  sqlite3_file ਅਧਾਰ;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
ਬਣਤਰ quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਅੱਖਰ *zMbcsName;        /* Full MBCS pathname of the file */
#ਜੇ_ਅੰਤ
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
ਸਥਿਰ ਬਣਤਰ {
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
  ਪੂਰਨ isInitialized;

  /* For run-time access any of the other global data structures in this
  ** shim, the following mutex must be held.
  */
  sqlite3_mutex *pMutex;

  /* List of quotaGroup objects.
  */
  quota_ਸਮੂਹ *p_ਸਮੂਹ;

} gQuota;

/************************* Utility Routines *********************************/
/*
** Acquire and release the mutex used to serialize access to the
** list of quotaGroups.
*/
ਸਥਿਰ ਖਾਲੀ quotaEnter(ਖਾਲੀ){ sqlite3_mutex_enter(gQuota.pMutex); }
ਸਥਿਰ ਖਾਲੀ quotaLeave(ਖਾਲੀ){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
ਸਥਿਰ ਪੂਰਨ quota_ਸਮੂਹ_Open_File_ਗਿਣਤੀ(quota_ਸਮੂਹ *p_ਸਮੂਹ){
  ਪੂਰਨ N = 0;
  quotaFile *pFile = p_ਸਮੂਹ->pFiles;
  ਜਦੋਂ( pFile ){
    ਜੇ( pFile->nRef ) N++;
    pFile = pFile->p_ਅਗਲਾ;
  }
  ਵਾਪਸ N;
}

/* Remove a file from a quota group.
*/
ਸਥਿਰ ਖਾਲੀ quotaRemoveFile(quotaFile *pFile){
  quota_ਸਮੂਹ *p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
  p_ਸਮੂਹ->i_ਮਾਪ -= pFile->i_ਮਾਪ;
  *pFile->ppPrev = pFile->p_ਅਗਲਾ;
  ਜੇ( pFile->p_ਅਗਲਾ ) pFile->p_ਅਗਲਾ->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
ਸਥਿਰ ਖਾਲੀ quotaRemoveAllFiles(quota_ਸਮੂਹ *p_ਸਮੂਹ){
  ਜਦੋਂ( p_ਸਮੂਹ->pFiles ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਮੂਹ->pFiles->nRef==0 );
    quotaRemoveFile(p_ਸਮੂਹ->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
ਸਥਿਰ ਖਾਲੀ quota_ਸਮੂਹ_Deref(quota_ਸਮੂਹ *p_ਸਮੂਹ){
  ਜੇ( p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ==0 && quota_ਸਮੂਹ_Open_File_ਗਿਣਤੀ(p_ਸਮੂਹ)==0 ){
    quotaRemoveAllFiles(p_ਸਮੂਹ);
    *p_ਸਮੂਹ->ppPrev = p_ਸਮੂਹ->p_ਅਗਲਾ;
    ਜੇ( p_ਸਮੂਹ->p_ਅਗਲਾ ) p_ਸਮੂਹ->p_ਅਗਲਾ->ppPrev = p_ਸਮੂਹ->ppPrev;
    ਜੇ( p_ਸਮੂਹ->xDestroy ) p_ਸਮੂਹ->xDestroy(p_ਸਮੂਹ->pArg);
    sqlite3_free(p_ਸਮੂਹ);
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
ਸਥਿਰ ਪੂਰਨ quotaStrglob(ਅਚਲ ਅੱਖਰ *zGlob, ਅਚਲ ਅੱਖਰ *z){
  ਪੂਰਨ c, c2, cx;
  ਪੂਰਨ invert;
  ਪੂਰਨ ਵੇਖਿਆ;

  ਜਦੋਂ( (c = (*(zGlob++)))!=0 ){
    ਜੇ( c=='*' ){
      ਜਦੋਂ( (c=(*(zGlob++))) == '*' || c=='?' ){
        ਜੇ( c=='?' && (*(z++))==0 ) ਵਾਪਸ 0;
      }
      ਜੇ( c==0 ){
        ਵਾਪਸ 1;
      }ਨਹੀਂਤਾਂ ਜੇ( c=='[' ){
        ਜਦੋਂ( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        ਵਾਪਸ (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      ਜਦੋਂ( (c2 = (*(z++)))!=0 ){
        ਜਦੋਂ( c2!=c && c2!=cx ){
          c2 = *(z++);
          ਜੇ( c2==0 ) ਵਾਪਸ 0;
        }
        ਜੇ( quotaStrglob(zGlob,z) ) ਵਾਪਸ 1;
      }
      ਵਾਪਸ 0;
    }ਨਹੀਂਤਾਂ ਜੇ( c=='?' ){
      ਜੇ( (*(z++))==0 ) ਵਾਪਸ 0;
    }ਨਹੀਂਤਾਂ ਜੇ( c=='[' ){
      ਪੂਰਨ ਪੂਰਵ_c = 0;
      ਵੇਖਿਆ = 0;
      invert = 0;
      c = *(z++);
      ਜੇ( c==0 ) ਵਾਪਸ 0;
      c2 = *(zGlob++);
      ਜੇ( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      ਜੇ( c2==']' ){
        ਜੇ( c==']' ) ਵੇਖਿਆ = 1;
        c2 = *(zGlob++);
      }
      ਜਦੋਂ( c2 && c2!=']' ){
        ਜੇ( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && ਪੂਰਵ_c>0 ){
          c2 = *(zGlob++);
          ਜੇ( c>=ਪੂਰਵ_c && c<=c2 ) ਵੇਖਿਆ = 1;
          ਪੂਰਵ_c = 0;
        }ਨਹੀਂਤਾਂ{
          ਜੇ( c==c2 ){
            ਵੇਖਿਆ = 1;
          }
          ਪੂਰਵ_c = c2;
        }
        c2 = *(zGlob++);
      }
      ਜੇ( c2==0 || (ਵੇਖਿਆ ^ invert)==0 ) ਵਾਪਸ 0;
    }ਨਹੀਂਤਾਂ ਜੇ( c=='/' ){
      ਜੇ( z[0]!='/' && z[0]!='\\' ) ਵਾਪਸ 0;
      z++;
    }ਨਹੀਂਤਾਂ{
      ਜੇ( c!=(*(z++)) ) ਵਾਪਸ 0;
    }
  }
  ਵਾਪਸ *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
ਸਥਿਰ quota_ਸਮੂਹ *quota_ਸਮੂਹ_ਲੱਭੋ(ਅਚਲ ਅੱਖਰ *zFilename){
  quota_ਸਮੂਹ *p;
  ਲਈ(p=gQuota.p_ਸਮੂਹ; p && quotaStrglob(p->z_ਨਮੂਨਾ, zFilename)==0;
      p=p->p_ਅਗਲਾ){}
  ਵਾਪਸ p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
ਸਥਿਰ sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  ਵਾਪਸ (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
ਸਥਿਰ quotaFile *quota_ਲੱਭੋ_File(
  quota_ਸਮੂਹ *p_ਸਮੂਹ,     /* Group in which to look for the file */
  ਅਚਲ ਅੱਖਰ *zName,      /* Full pathname of the file */
  ਪੂਰਨ createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = p_ਸਮੂਹ->pFiles;
  ਜਦੋਂ( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->p_ਅਗਲਾ;
  }
  ਜੇ( pFile==0 && createFlag ){
    ਪੂਰਨ nName = (ਪੂਰਨ)(ਲੜੀ_ਲੰਬਾਈ(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( ਆਕਾਰ(*pFile) + nName + 1 );
    ਜੇ( pFile ){
      memset(pFile, 0, ਆਕਾਰ(*pFile));
      pFile->zFilename = (ਅੱਖਰ*)&pFile[1];
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pFile->zFilename, zName, nName+1);
      pFile->p_ਅਗਲਾ = p_ਸਮੂਹ->pFiles;
      ਜੇ( p_ਸਮੂਹ->pFiles ) p_ਸਮੂਹ->pFiles->ppPrev = &pFile->p_ਅਗਲਾ;
      pFile->ppPrev = &p_ਸਮੂਹ->pFiles;
      p_ਸਮੂਹ->pFiles = pFile;
      pFile->p_ਸਮੂਹ = p_ਸਮੂਹ;
    }
  }
  ਵਾਪਸ pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
ਸਥਿਰ ਅੱਖਰ *quota_utf8_ਮੰਜ਼ਿਲ_mbcs(ਅਚਲ ਅੱਖਰ *zUtf8){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  size_t n;          /* Bytes in zUtf8 */
  ਪੂਰਨ nWide;         /* number of UTF-16 characters */
  ਪੂਰਨ nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  ਅੱਖਰ *zMbcs;       /* The MBCS text */
  ਪੂਰਨ codepage;      /* Code page used by fopen() */

  n = ਲੜੀ_ਲੰਬਾਈ(zUtf8);
  nWide = Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_UTF8, 0, zUtf8, -1, ਨਿਰਾ, 0);
  ਜੇ( nWide==0 ) ਵਾਪਸ 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*ਆਕਾਰ(zTmpWide[0]) );
  ਜੇ( zTmpWide==0 ) ਵਾਪਸ 0;
  Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = Wide_Char_ਮੰਜ਼ਿਲ_Multi_Byte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (ਅੱਖਰ*)sqlite3_malloc( nMbcs+1 ) : 0;
  ਜੇ( zMbcs ){
    Wide_Char_ਮੰਜ਼ਿਲ_Multi_Byte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  ਵਾਪਸ zMbcs;
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ (ਅੱਖਰ*)zUtf8;  /* No-op on unix */
#ਜੇ_ਅੰਤ
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
ਸਥਿਰ ਖਾਲੀ quota_mbcs_free(ਅੱਖਰ *zOld){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  sqlite3_free(zOld);
#ਨਹੀਂਤਾਂ
  /* No-op on unix */
#ਜੇ_ਅੰਤ
}

/************************* VFS Method Wrappers *****************************/
/*
** This is the xOpen method used for the "quota" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate quota group if it is a
** file that needs to be tracked.
*/
ਸਥਿਰ ਪੂਰਨ quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ਅਚਲ ਅੱਖਰ *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  ਪੂਰਨ flags,                  /* Flags to control the opening */
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags              /* Flags showing results of opening */
){
  ਪੂਰਨ rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quota_ਸਮੂਹ *p_ਸਮੂਹ;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  ਜੇ( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    ਵਾਪਸ pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_ਬਾਹਰਲਾ_Flags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  p_ਸਮੂਹ = quota_ਸਮੂਹ_ਲੱਭੋ(zName);
  ਜੇ( p_ਸਮੂਹ==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_ਬਾਹਰਲਾ_Flags);
  }ਨਹੀਂਤਾਂ{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, p_ਬਾਹਰਲਾ_Flags);
    ਜੇ( rc==SQLITE_OK ){
      pFile = quota_ਲੱਭੋ_File(p_ਸਮੂਹ, zName, 1);
      ਜੇ( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        ਵਾਪਸ SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      ਜੇ( pSubOpen->pMethods->i_ਸੰਸਕਰਣ==1 ){
        pQuotaOpen->ਅਧਾਰ.pMethods = &gQuota.sIoMethodsV1;
      }ਨਹੀਂਤਾਂ{
        pQuotaOpen->ਅਧਾਰ.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  ਵਾਪਸ rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
ਸਥਿਰ ਪੂਰਨ quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ਅਚਲ ਅੱਖਰ *zName,          /* Name of file to be deleted */
  ਪੂਰਨ syncDir                 /* Do a directory sync after deleting */
){
  ਪੂਰਨ rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quota_ਸਮੂਹ *p_ਸਮੂਹ;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  ਜੇ( rc==SQLITE_OK ){
    quotaEnter();
    p_ਸਮੂਹ = quota_ਸਮੂਹ_ਲੱਭੋ(zName);
    ਜੇ( p_ਸਮੂਹ ){
      pFile = quota_ਲੱਭੋ_File(p_ਸਮੂਹ, zName, 0);
      ਜੇ( pFile ){
        ਜੇ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ਨਹੀਂਤਾਂ{
          quotaRemoveFile(pFile);
          quota_ਸਮੂਹ_Deref(p_ਸਮੂਹ);
        }
      }
    }
    quotaLeave();
  }
  ਵਾਪਸ rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
ਸਥਿਰ ਪੂਰਨ quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਪੂਰਨ rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  ਜੇ( pFile->nRef==0 ){
    quota_ਸਮੂਹ *p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
    ਜੇ( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quota_ਸਮੂਹ_Deref(p_ਸਮੂਹ);
  }
  quotaLeave();
  ਵਾਪਸ rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
ਸਥਿਰ ਪੂਰਨ quotaRead(
  sqlite3_file *pConn,
  ਖਾਲੀ *pBuf,
  ਪੂਰਨ iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
ਸਥਿਰ ਪੂਰਨ quotaWrite(
  sqlite3_file *pConn,
  ਅਚਲ ਖਾਲੀ *pBuf,
  ਪੂਰਨ iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 i_ਸਿਰਾ = iOfst+iAmt;
  quota_ਸਮੂਹ *p_ਸਮੂਹ;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  ਜੇ( pFile->i_ਮਾਪ<i_ਸਿਰਾ ){
    p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
    quotaEnter();
    szNew = p_ਸਮੂਹ->i_ਮਾਪ - pFile->i_ਮਾਪ + i_ਸਿਰਾ;
    ਜੇ( szNew>p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ && p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ>0 ){
      ਜੇ( p_ਸਮੂਹ->xCallback ){
        p_ਸਮੂਹ->xCallback(pFile->zFilename, &p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ, szNew,
                          p_ਸਮੂਹ->pArg);
      }
      ਜੇ( szNew>p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ && p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ>0 ){
        quotaLeave();
        ਵਾਪਸ SQLITE_ਭਰਿਆ;
      }
    }
    p_ਸਮੂਹ->i_ਮਾਪ = szNew;
    pFile->i_ਮਾਪ = i_ਸਿਰਾ;
    quotaLeave();
  }
  ਵਾਪਸ pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
ਸਥਿਰ ਪੂਰਨ quotaTruncate(sqlite3_file *pConn, sqlite3_int64 ਮਾਪ){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਪੂਰਨ rc = pSubOpen->pMethods->xTruncate(pSubOpen, ਮਾਪ);
  quotaFile *pFile = p->pFile;
  quota_ਸਮੂਹ *p_ਸਮੂਹ;
  ਜੇ( rc==SQLITE_OK ){
    quotaEnter();
    p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
    p_ਸਮੂਹ->i_ਮਾਪ -= pFile->i_ਮਾਪ;
    pFile->i_ਮਾਪ = ਮਾਪ;
    p_ਸਮੂਹ->i_ਮਾਪ += ਮਾਪ;
    quotaLeave();
  }
  ਵਾਪਸ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ਸਥਿਰ ਪੂਰਨ quotaSync(sqlite3_file *pConn, ਪੂਰਨ flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
ਸਥਿਰ ਪੂਰਨ quota_File_ਮਾਪ(sqlite3_file *pConn, sqlite3_int64 *p_ਮਾਪ){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quota_ਸਮੂਹ *p_ਸਮੂਹ;
  sqlite3_int64 sz;
  ਪੂਰਨ rc;

  rc = pSubOpen->pMethods->x_File_ਮਾਪ(pSubOpen, &sz);
  ਜੇ( rc==SQLITE_OK ){
    quotaEnter();
    p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
    p_ਸਮੂਹ->i_ਮਾਪ -= pFile->i_ਮਾਪ;
    pFile->i_ਮਾਪ = sz;
    p_ਸਮੂਹ->i_ਮਾਪ += sz;
    quotaLeave();
    *p_ਮਾਪ = sz;
  }
  ਵਾਪਸ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quota_ਤਾਲਾ(sqlite3_file *pConn, ਪੂਰਨ ਤਾਲਾ){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->x_ਤਾਲਾ(pSubOpen, ਤਾਲਾ);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quotaUnlock(sqlite3_file *pConn, ਪੂਰਨ ਤਾਲਾ){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->xUnlock(pSubOpen, ਤਾਲਾ);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quota_Check_Reserved_ਤਾਲਾ(sqlite3_file *pConn, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->x_Check_Reserved_ਤਾਲਾ(pSubOpen, p_Res_ਬਾਹਰਲਾ);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quotaFileControl(sqlite3_file *pConn, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਪੂਰਨ rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#ਜੇ defined(SQLITE_FCNTL_VFSNAME)
  ਜੇ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(ਅੱਖਰ**)pArg = sqlite3_mprintf("quota/%z", *(ਅੱਖਰ**)pArg);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quota_Sector_ਮਾਪ(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->x_Sector_ਮਾਪ(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ਪੂਰਨ iRegion,                    /* Region to retrieve */
  ਪੂਰਨ szRegion,                   /* Size of regions */
  ਪੂਰਨ bExtend,                    /* True to extend file if necessary */
  ਖਾਲੀ ਅਸਥਿਰ **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quota_Shm_ਤਾਲਾ(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ਪੂਰਨ ofst,                  /* First lock to acquire or release */
  ਪੂਰਨ n,                     /* Number of locks to acquire or release */
  ਪੂਰਨ flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->x_Shm_ਤਾਲਾ(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਖਾਲੀ quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ quotaShmUnmap(sqlite3_file *pConn, ਪੂਰਨ deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ਵਾਪਸ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
ਪੂਰਨ sqlite3_quota_initialize(ਅਚਲ ਅੱਖਰ *zOrigVfsName, ਪੂਰਨ makeDefault){
  sqlite3_vfs *pOrigVfs;
  ਜੇ( gQuota.isInitialized ) ਵਾਪਸ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_ਲੱਭੋ(zOrigVfsName);
  ਜੇ( pOrigVfs==0 ) ਵਾਪਸ SQLITE_ERROR;
  ਪ੍ਰਤਿਜ੍ਞਾ( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ਜੇ( !gQuota.pMutex ){
    ਵਾਪਸ SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += ਆਕਾਰ(quotaConn);
  gQuota.sThisVfs.zName = "quota";
  gQuota.sIoMethodsV1.i_ਸੰਸਕਰਣ = 1;
  gQuota.sIoMethodsV1.xClose = quotaClose;
  gQuota.sIoMethodsV1.xRead = quotaRead;
  gQuota.sIoMethodsV1.xWrite = quotaWrite;
  gQuota.sIoMethodsV1.xTruncate = quotaTruncate;
  gQuota.sIoMethodsV1.xSync = quotaSync;
  gQuota.sIoMethodsV1.x_File_ਮਾਪ = quota_File_ਮਾਪ;
  gQuota.sIoMethodsV1.x_ਤਾਲਾ = quota_ਤਾਲਾ;
  gQuota.sIoMethodsV1.xUnlock = quotaUnlock;
  gQuota.sIoMethodsV1.x_Check_Reserved_ਤਾਲਾ = quota_Check_Reserved_ਤਾਲਾ;
  gQuota.sIoMethodsV1.xFileControl = quotaFileControl;
  gQuota.sIoMethodsV1.x_Sector_ਮਾਪ = quota_Sector_ਮਾਪ;
  gQuota.sIoMethodsV1.xDeviceCharacteristics = quotaDeviceCharacteristics;
  gQuota.sIoMethodsV2 = gQuota.sIoMethodsV1;
  gQuota.sIoMethodsV2.i_ਸੰਸਕਰਣ = 2;
  gQuota.sIoMethodsV2.xShmMap = quotaShmMap;
  gQuota.sIoMethodsV2.x_Shm_ਤਾਲਾ = quota_Shm_ਤਾਲਾ;
  gQuota.sIoMethodsV2.xShmBarrier = quotaShmBarrier;
  gQuota.sIoMethodsV2.xShmUnmap = quotaShmUnmap;
  sqlite3_vfs_ਰਜਿਸਟਰ(&gQuota.sThisVfs, makeDefault);
  ਵਾਪਸ SQLITE_OK;
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
ਪੂਰਨ sqlite3_quota_shutdown(ਖਾਲੀ){
  quota_ਸਮੂਹ *p_ਸਮੂਹ;
  ਜੇ( gQuota.isInitialized==0 ) ਵਾਪਸ SQLITE_MISUSE;
  ਲਈ(p_ਸਮੂਹ=gQuota.p_ਸਮੂਹ; p_ਸਮੂਹ; p_ਸਮੂਹ=p_ਸਮੂਹ->p_ਅਗਲਾ){
    ਜੇ( quota_ਸਮੂਹ_Open_File_ਗਿਣਤੀ(p_ਸਮੂਹ)>0 ) ਵਾਪਸ SQLITE_MISUSE;
  }
  ਜਦੋਂ( gQuota.p_ਸਮੂਹ ){
    p_ਸਮੂਹ = gQuota.p_ਸਮੂਹ;
    gQuota.p_ਸਮੂਹ = p_ਸਮੂਹ->p_ਅਗਲਾ;
    p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ = 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( quota_ਸਮੂਹ_Open_File_ਗਿਣਤੀ(p_ਸਮੂਹ)==0 );
    quota_ਸਮੂਹ_Deref(p_ਸਮੂਹ);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, ਆਕਾਰ(gQuota));
  ਵਾਪਸ SQLITE_OK;
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
ਪੂਰਨ sqlite3_quota_ਸੈੱਟ(
  ਅਚਲ ਅੱਖਰ *z_ਨਮੂਨਾ,           /* The filename pattern */
  sqlite3_int64 i_ਹੱਦਬੰਦੀ,           /* New quota to set for this quota group */
  ਖਾਲੀ (*xCallback)(              /* Callback invoked when going over quota */
     ਅਚਲ ਅੱਖਰ *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_ਹੱਦਬੰਦੀ,        /* IN/OUT: The current limit */
     sqlite3_int64 i_ਮਾਪ,           /* Total size of all files in the group */
     ਖਾਲੀ *pArg                     /* Client data */
  ),
  ਖਾਲੀ *pArg,                     /* client data passed thru to callback */
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ*)         /* Optional destructor for pArg */
){
  quota_ਸਮੂਹ *p_ਸਮੂਹ;
  quotaEnter();
  p_ਸਮੂਹ = gQuota.p_ਸਮੂਹ;
  ਜਦੋਂ( p_ਸਮੂਹ && strcmp(p_ਸਮੂਹ->z_ਨਮੂਨਾ, z_ਨਮੂਨਾ)!=0 ){
    p_ਸਮੂਹ = p_ਸਮੂਹ->p_ਅਗਲਾ;
  }
  ਜੇ( p_ਸਮੂਹ==0 ){
    ਪੂਰਨ n_ਨਮੂਨਾ = (ਪੂਰਨ)(ਲੜੀ_ਲੰਬਾਈ(z_ਨਮੂਨਾ) & 0x3fffffff);
    ਜੇ( i_ਹੱਦਬੰਦੀ<=0 ){
      quotaLeave();
      ਵਾਪਸ SQLITE_OK;
    }
    p_ਸਮੂਹ = (quota_ਸਮੂਹ *)sqlite3_malloc( ਆਕਾਰ(*p_ਸਮੂਹ) + n_ਨਮੂਨਾ + 1 );
    ਜੇ( p_ਸਮੂਹ==0 ){
      quotaLeave();
      ਵਾਪਸ SQLITE_NOMEM;
    }
    memset(p_ਸਮੂਹ, 0, ਆਕਾਰ(*p_ਸਮੂਹ));
    p_ਸਮੂਹ->z_ਨਮੂਨਾ = (ਅੱਖਰ*)&p_ਸਮੂਹ[1];
    ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਅੱਖਰ *)p_ਸਮੂਹ->z_ਨਮੂਨਾ, z_ਨਮੂਨਾ, n_ਨਮੂਨਾ+1);
    ਜੇ( gQuota.p_ਸਮੂਹ ) gQuota.p_ਸਮੂਹ->ppPrev = &p_ਸਮੂਹ->p_ਅਗਲਾ;
    p_ਸਮੂਹ->p_ਅਗਲਾ = gQuota.p_ਸਮੂਹ;
    p_ਸਮੂਹ->ppPrev = &gQuota.p_ਸਮੂਹ;
    gQuota.p_ਸਮੂਹ = p_ਸਮੂਹ;
  }
  p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ = i_ਹੱਦਬੰਦੀ;
  p_ਸਮੂਹ->xCallback = xCallback;
  ਜੇ( p_ਸਮੂਹ->xDestroy && p_ਸਮੂਹ->pArg!=pArg ){
    p_ਸਮੂਹ->xDestroy(p_ਸਮੂਹ->pArg);
  }
  p_ਸਮੂਹ->pArg = pArg;
  p_ਸਮੂਹ->xDestroy = xDestroy;
  quota_ਸਮੂਹ_Deref(p_ਸਮੂਹ);
  quotaLeave();
  ਵਾਪਸ SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
ਪੂਰਨ sqlite3_quota_file(ਅਚਲ ਅੱਖਰ *zFilename){
  ਅੱਖਰ *z_ਭਰਿਆ = 0;
  sqlite3_file *fd;
  ਪੂਰਨ rc;
  ਪੂਰਨ ਬਾਹਰਲਾ_Flags = 0;
  sqlite3_int64 i_ਮਾਪ;
  ਪੂਰਨ nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  ਜੇ( fd==0 ){
    rc = SQLITE_NOMEM;
  }ਨਹੀਂਤਾਂ{
    z_ਭਰਿਆ = &((ਅੱਖਰ *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->x_ਭਰਿਆ_Pathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, z_ਭਰਿਆ);
  }

  ਜੇ( rc==SQLITE_OK ){
    z_ਭਰਿਆ[ਲੜੀ_ਲੰਬਾਈ(z_ਭਰਿਆ)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, z_ਭਰਿਆ, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &ਬਾਹਰਲਾ_Flags);
    ਜੇ( rc==SQLITE_OK ){
      fd->pMethods->x_File_ਮਾਪ(fd, &i_ਮਾਪ);
      fd->pMethods->xClose(fd);
    }ਨਹੀਂਤਾਂ ਜੇ( rc==SQLITE_CANTOPEN ){
      quota_ਸਮੂਹ *p_ਸਮੂਹ;
      quotaFile *pFile;
      quotaEnter();
      p_ਸਮੂਹ = quota_ਸਮੂਹ_ਲੱਭੋ(z_ਭਰਿਆ);
      ਜੇ( p_ਸਮੂਹ ){
        pFile = quota_ਲੱਭੋ_File(p_ਸਮੂਹ, z_ਭਰਿਆ, 0);
        ਜੇ( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  ਵਾਪਸ rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(ਅਚਲ ਅੱਖਰ *zFilename, ਅਚਲ ਅੱਖਰ *z_ਬਹੁਲਕ){
  quota_FILE *p = 0;
  ਅੱਖਰ *z_ਭਰਿਆ = 0;
  ਅੱਖਰ *z_ਭਰਿਆ_Translated = 0;
  ਪੂਰਨ rc;
  quota_ਸਮੂਹ *p_ਸਮੂਹ;
  quotaFile *pFile;

  z_ਭਰਿਆ = (ਅੱਖਰ*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ਜੇ( z_ਭਰਿਆ==0 ) ਵਾਪਸ 0;
  rc = gQuota.pOrigVfs->x_ਭਰਿਆ_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_ਭਰਿਆ);
  ਜੇ( rc ) ਜਾਓ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(ਆਕਾਰ(*p));
  ਜੇ( p==0 ) ਜਾਓ quota_fopen_error;
  memset(p, 0, ਆਕਾਰ(*p));
  z_ਭਰਿਆ_Translated = quota_utf8_ਮੰਜ਼ਿਲ_mbcs(z_ਭਰਿਆ);
  ਜੇ( z_ਭਰਿਆ_Translated==0 ) ਜਾਓ quota_fopen_error;
  p->f = fopen(z_ਭਰਿਆ_Translated, z_ਬਹੁਲਕ);
  ਜੇ( p->f==0 ) ਜਾਓ quota_fopen_error;
  quotaEnter();
  p_ਸਮੂਹ = quota_ਸਮੂਹ_ਲੱਭੋ(z_ਭਰਿਆ);
  ਜੇ( p_ਸਮੂਹ ){
    pFile = quota_ਲੱਭੋ_File(p_ਸਮੂਹ, z_ਭਰਿਆ, 1);
    ਜੇ( pFile==0 ){
      quotaLeave();
      ਜਾਓ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(z_ਭਰਿਆ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  p->zMbcsName = z_ਭਰਿਆ_Translated;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ p;

quota_fopen_error:
  quota_mbcs_free(z_ਭਰਿਆ_Translated);
  sqlite3_free(z_ਭਰਿਆ);
  ਜੇ( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  ਵਾਪਸ 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  ਖਾਲੀ *pBuf,            /* Store the content here */
  size_t ਮਾਪ,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  ਵਾਪਸ fread(pBuf, ਮਾਪ, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  ਅਚਲ ਖਾਲੀ *pBuf,      /* Take content to write from here */
  size_t ਮਾਪ,           /* Size of each element */
  size_t nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 i_ਸਿਰਾ;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  size_t rc;

  iOfst = ftell(p->f);
  i_ਸਿਰਾ = iOfst + ਮਾਪ*nmemb;
  pFile = p->pFile;
  ਜੇ( pFile && pFile->i_ਮਾਪ<i_ਸਿਰਾ ){
    quota_ਸਮੂਹ *p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
    quotaEnter();
    szNew = p_ਸਮੂਹ->i_ਮਾਪ - pFile->i_ਮਾਪ + i_ਸਿਰਾ;
    ਜੇ( szNew>p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ && p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ>0 ){
      ਜੇ( p_ਸਮੂਹ->xCallback ){
        p_ਸਮੂਹ->xCallback(pFile->zFilename, &p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ, szNew,
                          p_ਸਮੂਹ->pArg);
      }
      ਜੇ( szNew>p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ && p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ>0 ){
        i_ਸਿਰਾ = p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ - p_ਸਮੂਹ->i_ਮਾਪ + pFile->i_ਮਾਪ;
        nmemb = (size_t)((i_ਸਿਰਾ - iOfst)/ਮਾਪ);
        i_ਸਿਰਾ = iOfst + ਮਾਪ*nmemb;
        szNew = p_ਸਮੂਹ->i_ਮਾਪ - pFile->i_ਮਾਪ + i_ਸਿਰਾ;
      }
    }
    p_ਸਮੂਹ->i_ਮਾਪ = szNew;
    pFile->i_ਮਾਪ = i_ਸਿਰਾ;
    quotaLeave();
  }ਨਹੀਂਤਾਂ{
    pFile = 0;
  }
  rc = fwrite(pBuf, ਮਾਪ, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  ਜੇ( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 i_New_ਸਿਰਾ = iOfst + ਮਾਪ*nWritten;
    ਜੇ( i_New_ਸਿਰਾ<i_ਸਿਰਾ ) i_New_ਸਿਰਾ = i_ਸਿਰਾ;
    quotaEnter();
    pFile->p_ਸਮੂਹ->i_ਮਾਪ += i_New_ਸਿਰਾ - pFile->i_ਮਾਪ;
    pFile->i_ਮਾਪ = i_New_ਸਿਰਾ;
    quotaLeave();
  }
  ਵਾਪਸ rc;
}

/*
** Close an open quota_FILE stream.
*/
ਪੂਰਨ sqlite3_quota_fclose(quota_FILE *p){
  ਪੂਰਨ rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  ਜੇ( pFile ){
    quotaEnter();
    pFile->nRef--;
    ਜੇ( pFile->nRef==0 ){
      quota_ਸਮੂਹ *p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
      ਜੇ( pFile->deleteOnClose ){
        gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
        quotaRemoveFile(pFile);
      }
      quota_ਸਮੂਹ_Deref(p_ਸਮੂਹ);
    }
    quotaLeave();
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  quota_mbcs_free(p->zMbcsName);
#ਜੇ_ਅੰਤ
  sqlite3_free(p);
  ਵਾਪਸ rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
ਪੂਰਨ sqlite3_quota_fflush(quota_FILE *p, ਪੂਰਨ doFsync){
  ਪੂਰਨ rc;
  rc = fflush(p->f);
  ਜੇ( rc==0 && doFsync ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
    rc = _commit(_fileno(p->f));
#ਨਹੀਂਤਾਂ
    rc = fsync(fileno(p->f));
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
ਪੂਰਨ sqlite3_quota_fseek(quota_FILE *p, ਲੰਮਾ ਵਿੱਥ, ਪੂਰਨ whence){
  ਵਾਪਸ fseek(p->f, ਵਿੱਥ, whence);
}

/*
** rewind a quota_FILE stream.
*/
ਖਾਲੀ sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
ਲੰਮਾ sqlite3_quota_ftell(quota_FILE *p){
  ਵਾਪਸ ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
ਪੂਰਨ sqlite3_quota_ferror(quota_FILE *p){
  ਵਾਪਸ ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
ਪੂਰਨ sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  ਪੂਰਨ rc;
  ਜੇ( (pFile = p->pFile)!=0 && pFile->i_ਮਾਪ<szNew ){
    quota_ਸਮੂਹ *p_ਸਮੂਹ;
    ਜੇ( pFile->i_ਮਾਪ<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      ਵਾਪਸ -1;
    }
    p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
    quotaEnter();
    p_ਸਮੂਹ->i_ਮਾਪ += szNew - pFile->i_ਮਾਪ;
    quotaLeave();
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#  ਜੇ defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (ਲੰਮਾ)szNew);
#  ਨਹੀਂਤਾਂ
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#ਨਹੀਂਤਾਂ
  rc = ftruncate(fileno(p->f), szNew);
#ਜੇ_ਅੰਤ
  ਜੇ( pFile && rc==0 ){
    quota_ਸਮੂਹ *p_ਸਮੂਹ = pFile->p_ਸਮੂਹ;
    quotaEnter();
    p_ਸਮੂਹ->i_ਮਾਪ += szNew - pFile->i_ਮਾਪ;
    pFile->i_ਮਾਪ = szNew;
    quotaLeave();
  }
  ਵਾਪਸ rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
ਪੂਰਨ sqlite3_quota_file_mtime(quota_FILE *p, ਸਮਾਂ_t *p_ਸਮਾਂ){
  ਪੂਰਨ rc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਬਣਤਰ _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ਨਹੀਂਤਾਂ
  ਬਣਤਰ stat buf;
  rc = fstat(fileno(p->f), &buf);
#ਜੇ_ਅੰਤ
  ਜੇ( rc==0 ) *p_ਸਮਾਂ = buf.st_mtime;
  ਵਾਪਸ rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  ਪੂਰਨ rc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਬਣਤਰ _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ਨਹੀਂਤਾਂ
  ਬਣਤਰ stat buf;
  rc = fstat(fileno(p->f), &buf);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc==0 ? buf.st_ਮਾਪ : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_ਮਾਪ(quota_FILE *p){
  ਵਾਪਸ p->pFile ? p->pFile->i_ਮਾਪ : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
ਲੰਮਾ sqlite3_quota_file_ਉਪਲਬਧ(quota_FILE *p){
  FILE* f = p->f;
  ਲੰਮਾ pos1, pos2;
  ਪੂਰਨ rc;
  pos1 = ftell(f);
  ਜੇ ( pos1 < 0 ) ਵਾਪਸ -1;
  rc = fseek(f, 0, SEEK_END);
  ਜੇ ( rc != 0 ) ਵਾਪਸ -1;
  pos2 = ftell(f);
  ਜੇ ( pos2 < 0 ) ਵਾਪਸ -1;
  rc = fseek(f, pos1, SEEK_SET);
  ਜੇ ( rc != 0 ) ਵਾਪਸ -1;
  ਵਾਪਸ pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
ਪੂਰਨ sqlite3_quota_remove(ਅਚਲ ਅੱਖਰ *zFilename){
  ਅੱਖਰ *z_ਭਰਿਆ;            /* Full pathname for zFilename */
  size_t n_ਭਰਿਆ;           /* Number of bytes in zFilename */
  ਪੂਰਨ rc;                 /* Result code */
  quota_ਸਮੂਹ *p_ਸਮੂਹ;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *p_ਅਗਲਾ_File;   /* next file in the group */
  ਪੂਰਨ diff;               /* Difference between filenames */
  ਅੱਖਰ c;                 /* First character past end of pattern */

  z_ਭਰਿਆ = (ਅੱਖਰ*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ਜੇ( z_ਭਰਿਆ==0 ) ਵਾਪਸ SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->x_ਭਰਿਆ_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_ਭਰਿਆ);
  ਜੇ( rc ){
    sqlite3_free(z_ਭਰਿਆ);
    ਵਾਪਸ rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  n_ਭਰਿਆ = ਲੜੀ_ਲੰਬਾਈ(z_ਭਰਿਆ);
  ਜੇ( n_ਭਰਿਆ>0 && (z_ਭਰਿਆ[n_ਭਰਿਆ-1]=='/' || z_ਭਰਿਆ[n_ਭਰਿਆ-1]=='\\') ){
    n_ਭਰਿਆ--;
    z_ਭਰਿਆ[n_ਭਰਿਆ] = 0;
  }

  quotaEnter();
  p_ਸਮੂਹ = quota_ਸਮੂਹ_ਲੱਭੋ(z_ਭਰਿਆ);
  ਜੇ( p_ਸਮੂਹ ){
    ਲਈ(pFile=p_ਸਮੂਹ->pFiles; pFile && rc==SQLITE_OK; pFile=p_ਅਗਲਾ_File){
      p_ਅਗਲਾ_File = pFile->p_ਅਗਲਾ;
      diff = ਲੜੀ_ਤੁਲਨਾ(z_ਭਰਿਆ, pFile->zFilename, n_ਭਰਿਆ);
      ਜੇ( diff==0 && ((c = pFile->zFilename[n_ਭਰਿਆ])==0 || c=='/' || c=='\\') ){
        ਜੇ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ਨਹੀਂਤਾਂ{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quota_ਸਮੂਹ_Deref(p_ਸਮੂਹ);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(z_ਭਰਿਆ);
  ਵਾਪਸ rc;
}

/***************************** Test Code ***********************************/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TclQuotaCallback TclQuotaCallback;
ਬਣਤਰ TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);


/*
** This is the callback from a quota-over-limit.
*/
ਸਥਿਰ ਖਾਲੀ tclQuotaCallback(
  ਅਚਲ ਅੱਖਰ *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *pi_ਹੱਦਬੰਦੀ,         /* IN/OUT: The current limit */
  sqlite3_int64 i_ਮਾਪ,            /* Total size of all files in the group */
  ਖਾਲੀ *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ rnd;               /* Random part of pVarname */
  ਪੂਰਨ rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  ਜੇ( p==0 ) ਵਾਪਸ;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pVarname);
  sqlite3_randomness(ਆਕਾਰ(rnd), (ਖਾਲੀ *)&rnd);
  Tcl_Append_Obj_ਮੰਜ਼ਿਲ_Obj(pVarname, Tcl_NewIntObj((ਪੂਰਨ)(rnd&0x7FFFFFFF)));
  Tcl_Obj_ਸੈੱਟ_Var2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*pi_ਹੱਦਬੰਦੀ), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(i_ਮਾਪ));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  ਜੇ( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *p_ਹੱਦਬੰਦੀ = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, p_ਹੱਦਬੰਦੀ, &x);
    *pi_ਹੱਦਬੰਦੀ = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_ਗਿਣਤੀ(pEval);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pVarname);
  ਜੇ( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
ਸਥਿਰ ਖਾਲੀ tclCallbackDestructor(ਖਾਲੀ *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  ਜੇ( p ){
    Tcl_Decr_Ref_ਗਿਣਤੀ(p->pScript);
    sqlite3_free((ਅੱਖਰ *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_initialize(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *zName;              /* Name of new quota VFS */
  ਪੂਰਨ makeDefault;                /* True to make the new VFS the default */
  ਪੂਰਨ rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "NAME MAKEDEFAULT");
    ਵਾਪਸ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ਜੇ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);

  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_shutdown(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;                         /* Value returned by quota_shutdown() */

  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);

  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_ਸੈੱਟ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *z_ਨਮੂਨਾ;           /* File pattern to configure */
  Tcl_WideInt i_ਹੱਦਬੰਦੀ;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  ਪੂਰਨ rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_ਮਾਪ nScript;               /* Length of callback script */
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ*);        /* Optional destructor for pArg */
  ਖਾਲੀ (*xCallback)(ਅਚਲ ਅੱਖਰ *, sqlite3_int64 *, sqlite3_int64, ਖਾਲੀ *);

  /* Process arguments */
  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    ਵਾਪਸ TCL_ERROR;
  }
  z_ਨਮੂਨਾ = Tcl_GetString(objv[1]);
  ਜੇ( Tcl_GetWideIntFromObj(interp, objv[2], &i_ਹੱਦਬੰਦੀ) ) ਵਾਪਸ TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  ਜੇ( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(ਆਕਾਰ(TclQuotaCallback));
    ਜੇ( !p ){
      Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)"SQLITE_NOMEM", TCL_STATIC);
      ਵਾਪਸ TCL_OK;
    }
    memset(p, 0, ਆਕਾਰ(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }ਨਹੀਂਤਾਂ{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_ਸੈੱਟ(z_ਨਮੂਨਾ, i_ਹੱਦਬੰਦੀ, xCallback, (ਖਾਲੀ*)p, xDestroy);

  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_file(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *zFilename;          /* File pattern to configure */
  ਪੂਰਨ rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "FILENAME");
    ਵਾਪਸ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_dump(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *p_ਨਤੀਜਾ;
  Tcl_Obj *p_ਸਮੂਹ_ਪਦ;
  Tcl_Obj *p_File_ਪਦ;
  quota_ਸਮੂਹ *p_ਸਮੂਹ;
  quotaFile *pFile;

  p_ਨਤੀਜਾ = Tcl_NewObj();
  quotaEnter();
  ਲਈ(p_ਸਮੂਹ=gQuota.p_ਸਮੂਹ; p_ਸਮੂਹ; p_ਸਮੂਹ=p_ਸਮੂਹ->p_ਅਗਲਾ){
    p_ਸਮੂਹ_ਪਦ = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, p_ਸਮੂਹ_ਪਦ,
          Tcl_NewStringObj(p_ਸਮੂਹ->z_ਨਮੂਨਾ, -1));
    Tcl_ListObjAppendElement(interp, p_ਸਮੂਹ_ਪਦ,
          Tcl_NewWideIntObj(p_ਸਮੂਹ->i_ਹੱਦਬੰਦੀ));
    Tcl_ListObjAppendElement(interp, p_ਸਮੂਹ_ਪਦ,
          Tcl_NewWideIntObj(p_ਸਮੂਹ->i_ਮਾਪ));
    ਲਈ(pFile=p_ਸਮੂਹ->pFiles; pFile; pFile=pFile->p_ਅਗਲਾ){
      ਪੂਰਨ i;
      ਅੱਖਰ z_ਆਰਜ਼ੀ[1000];
      p_File_ਪਦ = Tcl_NewObj();
      sqlite3_snprintf(ਆਕਾਰ(z_ਆਰਜ਼ੀ), z_ਆਰਜ਼ੀ, "%s", pFile->zFilename);
      ਲਈ(i=0; z_ਆਰਜ਼ੀ[i]; i++){ ਜੇ( z_ਆਰਜ਼ੀ[i]=='\\' ) z_ਆਰਜ਼ੀ[i] = '/'; }
      Tcl_ListObjAppendElement(interp, p_File_ਪਦ,
            Tcl_NewStringObj(z_ਆਰਜ਼ੀ, -1));
      Tcl_ListObjAppendElement(interp, p_File_ਪਦ,
            Tcl_NewWideIntObj(pFile->i_ਮਾਪ));
      Tcl_ListObjAppendElement(interp, p_File_ਪਦ,
            Tcl_NewWideIntObj(pFile->nRef));
      Tcl_ListObjAppendElement(interp, p_File_ਪਦ,
            Tcl_NewWideIntObj(pFile->deleteOnClose));
      Tcl_ListObjAppendElement(interp, p_ਸਮੂਹ_ਪਦ, p_File_ਪਦ);
    }
    Tcl_ListObjAppendElement(interp, p_ਨਤੀਜਾ, p_ਸਮੂਹ_ਪਦ);
  }
  quotaLeave();
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, p_ਨਤੀਜਾ);
  ਵਾਪਸ TCL_OK;
}

/* Defined in test1.c */
ਬਾਹਰੀ ਖਾਲੀ *sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(ਅਚਲ ਅੱਖਰ*);
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(ਖਾਲੀ*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_fopen(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *zFilename;          /* File pattern to configure */
  ਅਚਲ ਅੱਖਰ *z_ਬਹੁਲਕ;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "FILENAME MODE");
    ਵਾਪਸ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  z_ਬਹੁਲਕ = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, z_ਬਹੁਲਕ);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ*)sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(p), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_fread(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ਅੱਖਰ *zBuf;
  ਪੂਰਨ sz;
  ਪੂਰਨ nElem;
  size_t got;

  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE SIZE NELEM");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ਵਾਪਸ TCL_ERROR;
  zBuf = (ਅੱਖਰ*)sqlite3_malloc( sz*nElem + 1 );
  ਜੇ( zBuf==0 ){
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, "out of memory", TCL_STATIC);
    ਵਾਪਸ TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_fwrite(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ਅੱਖਰ *zBuf;
  ਪੂਰਨ sz;
  ਪੂਰਨ nElem;
  size_t got;

  ਜੇ( objc!=5 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ਵਾਪਸ TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(got));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_fclose(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_fflush(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ਪੂਰਨ rc;
  ਪੂਰਨ doSync = 0;

  ਜੇ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE ?HARDSYNC?");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  ਜੇ( objc==3 ){
    ਜੇ( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) ਵਾਪਸ TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_fseek(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ਪੂਰਨ ofst;
  ਅਚਲ ਅੱਖਰ *zWhence;
  ਪੂਰਨ whence;
  ਪੂਰਨ rc;

  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE OFFSET WHENCE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) ਵਾਪਸ TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  ਜੇ( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }ਨਹੀਂਤਾਂ{
    Tcl_Append_ਨਤੀਜਾ(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_rewind(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_ftell(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(x));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_ftruncate(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  ਪੂਰਨ rc;
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE SIZE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  ਜੇ( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) ਵਾਪਸ TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_file_ਮਾਪ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_ਮਾਪ(p);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(x));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_file_truesize(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(x));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_file_mtime(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ਸਮਾਂ_t t;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(t));
  ਵਾਪਸ TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_remove(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *zFilename;          /* File pattern to configure */
  ਪੂਰਨ rc;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "FILENAME");
    ਵਾਪਸ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_glob(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *z_ਨਮੂਨਾ;          /* The glob pattern */
  ਅਚਲ ਅੱਖਰ *z_ਪਾਠ;             /* Text to compare against the pattern */
  ਪੂਰਨ rc;
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "PATTERN TEXT");
    ਵਾਪਸ TCL_ERROR;
  }
  z_ਨਮੂਨਾ = Tcl_GetString(objv[1]);
  z_ਪਾਠ = Tcl_GetString(objv[2]);
  rc = quotaStrglob(z_ਨਮੂਨਾ, z_ਪਾਠ);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_file_ਉਪਲਬਧ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_ਉਪਲਬਧ(p);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(x));
  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_quota_ferror(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ਪੂਰਨ x;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE");
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(x));
  ਵਾਪਸ TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ਪੂਰਨ Sqlitequota_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_quota_initialize",    test_quota_initialize },
    { "sqlite3_quota_shutdown",      test_quota_shutdown },
    { "sqlite3_quota_set",           test_quota_ਸੈੱਟ },
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
    { "sqlite3_quota_file_size",     test_quota_file_ਮਾਪ },
    { "sqlite3_quota_file_truesize", test_quota_file_truesize },
    { "sqlite3_quota_file_mtime",    test_quota_file_mtime },
    { "sqlite3_quota_remove",        test_quota_remove },
    { "sqlite3_quota_glob",          test_quota_glob },
    { "sqlite3_quota_file_available",test_quota_file_ਉਪਲਬਧ },
    { "sqlite3_quota_ferror",        test_quota_ferror },
  };
  ਪੂਰਨ i;

  ਲਈ(i=0; i<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ਵਾਪਸ TCL_OK;
}
#ਜੇ_ਅੰਤ
