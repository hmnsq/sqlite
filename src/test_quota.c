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
#ಆದರೆ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_free(X)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_enter(X)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_try(X)      SQLITE_OK
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_leave(X)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_held(X)     ((ಶೂನ್ಯ)(X),1)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_notheld(X)  ((ಶೂನ್ಯ)(X),1)
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_THREADSAFE==0 */


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
# include <windows.h>
# include <io.h>
#ಇಲ್ಲದಿದ್ದರೆ
# include <unistd.h>
#ಆದರೆ_ಅಂತ್ಯ


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ quota_ಗುಂಪು quota_ಗುಂಪು;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ quotaConn quotaConn;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ quotaFile quotaFile;

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
ರಚನೆ quota_ಗುಂಪು {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ನಮೂನೆ;          /* Filename pattern to be quotaed */
  sqlite3_int64 i_ಎಲ್ಲೆ;          /* Upper bound on total file size */
  sqlite3_int64 i_ಗಾತ್ರ;           /* Current size of all files */
  ಶೂನ್ಯ (*xCallback)(             /* Callback invoked when going over quota */
     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_ಎಲ್ಲೆ,        /* IN/OUT: The current limit */
     sqlite3_int64 i_ಗಾತ್ರ,           /* Total size of all files in the group */
     ಶೂನ್ಯ *pArg                     /* Client data */
  );
  ಶೂನ್ಯ *pArg;                    /* Third argument to the xCallback() */
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*);       /* Optional destructor for pArg */
  quota_ಗುಂಪು *p_ಮುಂದಿನ, **ppPrev;   /* Doubly linked list of all quota objects */
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
ರಚನೆ quotaFile {
  ಅಕ್ಷರ *zFilename;                /* Name of this file */
  quota_ಗುಂಪು *p_ಗುಂಪು;             /* Quota group to which this file belongs */
  sqlite3_int64 i_ಗಾತ್ರ;            /* Current size of this file */
  ಪೂರ್ಣಾಂಕ nRef;                       /* Number of times this file is open */
  ಪೂರ್ಣಾಂಕ deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *p_ಮುಂದಿನ, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ರಚನೆ quotaConn {
  sqlite3_file ತಳ;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
ರಚನೆ quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಅಕ್ಷರ *zMbcsName;        /* Full MBCS pathname of the file */
#ಆದರೆ_ಅಂತ್ಯ
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
ಸ್ಥಿರ ರಚನೆ {
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
  ಪೂರ್ಣಾಂಕ isInitialized;

  /* For run-time access any of the other global data structures in this
  ** shim, the following mutex must be held.
  */
  sqlite3_mutex *pMutex;

  /* List of quotaGroup objects.
  */
  quota_ಗುಂಪು *p_ಗುಂಪು;

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quota_ಗುಂಪು_Open_File_ಎಣಿಕೆ(quota_ಗುಂಪು *p_ಗುಂಪು){
  ಪೂರ್ಣಾಂಕ N = 0;
  quotaFile *pFile = p_ಗುಂಪು->pFiles;
  ಆಗ( pFile ){
    ಆದರೆ( pFile->nRef ) N++;
    pFile = pFile->p_ಮುಂದಿನ;
  }
  ಹಿಂತಿರುಗಿ N;
}

/* Remove a file from a quota group.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quotaRemoveFile(quotaFile *pFile){
  quota_ಗುಂಪು *p_ಗುಂಪು = pFile->p_ಗುಂಪು;
  p_ಗುಂಪು->i_ಗಾತ್ರ -= pFile->i_ಗಾತ್ರ;
  *pFile->ppPrev = pFile->p_ಮುಂದಿನ;
  ಆದರೆ( pFile->p_ಮುಂದಿನ ) pFile->p_ಮುಂದಿನ->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quotaRemoveAllFiles(quota_ಗುಂಪು *p_ಗುಂಪು){
  ಆಗ( p_ಗುಂಪು->pFiles ){
    ಪ್ರತಿಜ್ಞಾ( p_ಗುಂಪು->pFiles->nRef==0 );
    quotaRemoveFile(p_ಗುಂಪು->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quota_ಗುಂಪು_Deref(quota_ಗುಂಪು *p_ಗುಂಪು){
  ಆದರೆ( p_ಗುಂಪು->i_ಎಲ್ಲೆ==0 && quota_ಗುಂಪು_Open_File_ಎಣಿಕೆ(p_ಗುಂಪು)==0 ){
    quotaRemoveAllFiles(p_ಗುಂಪು);
    *p_ಗುಂಪು->ppPrev = p_ಗುಂಪು->p_ಮುಂದಿನ;
    ಆದರೆ( p_ಗುಂಪು->p_ಮುಂದಿನ ) p_ಗುಂಪು->p_ಮುಂದಿನ->ppPrev = p_ಗುಂಪು->ppPrev;
    ಆದರೆ( p_ಗುಂಪು->xDestroy ) p_ಗುಂಪು->xDestroy(p_ಗುಂಪು->pArg);
    sqlite3_free(p_ಗುಂಪು);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaStrglob(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zGlob, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಪೂರ್ಣಾಂಕ c, c2, cx;
  ಪೂರ್ಣಾಂಕ invert;
  ಪೂರ್ಣಾಂಕ ನೋಡಿದ;

  ಆಗ( (c = (*(zGlob++)))!=0 ){
    ಆದರೆ( c=='*' ){
      ಆಗ( (c=(*(zGlob++))) == '*' || c=='?' ){
        ಆದರೆ( c=='?' && (*(z++))==0 ) ಹಿಂತಿರುಗಿ 0;
      }
      ಆದರೆ( c==0 ){
        ಹಿಂತಿರುಗಿ 1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='[' ){
        ಆಗ( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        ಹಿಂತಿರುಗಿ (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      ಆಗ( (c2 = (*(z++)))!=0 ){
        ಆಗ( c2!=c && c2!=cx ){
          c2 = *(z++);
          ಆದರೆ( c2==0 ) ಹಿಂತಿರುಗಿ 0;
        }
        ಆದರೆ( quotaStrglob(zGlob,z) ) ಹಿಂತಿರುಗಿ 1;
      }
      ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='?' ){
      ಆದರೆ( (*(z++))==0 ) ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='[' ){
      ಪೂರ್ಣಾಂಕ ಪೂರ್ವ_c = 0;
      ನೋಡಿದ = 0;
      invert = 0;
      c = *(z++);
      ಆದರೆ( c==0 ) ಹಿಂತಿರುಗಿ 0;
      c2 = *(zGlob++);
      ಆದರೆ( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      ಆದರೆ( c2==']' ){
        ಆದರೆ( c==']' ) ನೋಡಿದ = 1;
        c2 = *(zGlob++);
      }
      ಆಗ( c2 && c2!=']' ){
        ಆದರೆ( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && ಪೂರ್ವ_c>0 ){
          c2 = *(zGlob++);
          ಆದರೆ( c>=ಪೂರ್ವ_c && c<=c2 ) ನೋಡಿದ = 1;
          ಪೂರ್ವ_c = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಆದರೆ( c==c2 ){
            ನೋಡಿದ = 1;
          }
          ಪೂರ್ವ_c = c2;
        }
        c2 = *(zGlob++);
      }
      ಆದರೆ( c2==0 || (ನೋಡಿದ ^ invert)==0 ) ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='/' ){
      ಆದರೆ( z[0]!='/' && z[0]!='\\' ) ಹಿಂತಿರುಗಿ 0;
      z++;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( c!=(*(z++)) ) ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಹಿಂತಿರುಗಿ *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
ಸ್ಥಿರ quota_ಗುಂಪು *quota_ಗುಂಪು_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  quota_ಗುಂಪು *p;
  ಗಾಗಿ(p=gQuota.p_ಗುಂಪು; p && quotaStrglob(p->z_ನಮೂನೆ, zFilename)==0;
      p=p->p_ಮುಂದಿನ){}
  ಹಿಂತಿರುಗಿ p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
ಸ್ಥಿರ sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  ಹಿಂತಿರುಗಿ (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
ಸ್ಥಿರ quotaFile *quota_ಕಂಡುಹಿಡಿ_File(
  quota_ಗುಂಪು *p_ಗುಂಪು,     /* Group in which to look for the file */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,      /* Full pathname of the file */
  ಪೂರ್ಣಾಂಕ createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = p_ಗುಂಪು->pFiles;
  ಆಗ( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->p_ಮುಂದಿನ;
  }
  ಆದರೆ( pFile==0 && createFlag ){
    ಪೂರ್ಣಾಂಕ nName = (ಪೂರ್ಣಾಂಕ)(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( ಗಾತ್ರ(*pFile) + nName + 1 );
    ಆದರೆ( pFile ){
      memset(pFile, 0, ಗಾತ್ರ(*pFile));
      pFile->zFilename = (ಅಕ್ಷರ*)&pFile[1];
      ಸ್ಮರಣೆ_ನಕಲು(pFile->zFilename, zName, nName+1);
      pFile->p_ಮುಂದಿನ = p_ಗುಂಪು->pFiles;
      ಆದರೆ( p_ಗುಂಪು->pFiles ) p_ಗುಂಪು->pFiles->ppPrev = &pFile->p_ಮುಂದಿನ;
      pFile->ppPrev = &p_ಗುಂಪು->pFiles;
      p_ಗುಂಪು->pFiles = pFile;
      pFile->p_ಗುಂಪು = p_ಗುಂಪು;
    }
  }
  ಹಿಂತಿರುಗಿ pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
ಸ್ಥಿರ ಅಕ್ಷರ *quota_utf8_ಗಮ್ಯ_mbcs(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUtf8){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  size_t n;          /* Bytes in zUtf8 */
  ಪೂರ್ಣಾಂಕ nWide;         /* number of UTF-16 characters */
  ಪೂರ್ಣಾಂಕ nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  ಅಕ್ಷರ *zMbcs;       /* The MBCS text */
  ಪೂರ್ಣಾಂಕ codepage;      /* Code page used by fopen() */

  n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zUtf8);
  nWide = Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_UTF8, 0, zUtf8, -1, ಖಾಲಿ, 0);
  ಆದರೆ( nWide==0 ) ಹಿಂತಿರುಗಿ 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*ಗಾತ್ರ(zTmpWide[0]) );
  ಆದರೆ( zTmpWide==0 ) ಹಿಂತಿರುಗಿ 0;
  Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = Wide_Char_ಗಮ್ಯ_Multi_Byte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (ಅಕ್ಷರ*)sqlite3_malloc( nMbcs+1 ) : 0;
  ಆದರೆ( zMbcs ){
    Wide_Char_ಗಮ್ಯ_Multi_Byte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  ಹಿಂತಿರುಗಿ zMbcs;
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)zUtf8;  /* No-op on unix */
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
ಸ್ಥಿರ ಶೂನ್ಯ quota_mbcs_free(ಅಕ್ಷರ *zOld){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  sqlite3_free(zOld);
#ಇಲ್ಲದಿದ್ದರೆ
  /* No-op on unix */
#ಆದರೆ_ಅಂತ್ಯ
}

/************************* VFS Method Wrappers *****************************/
/*
** This is the xOpen method used for the "quota" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate quota group if it is a
** file that needs to be tracked.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  ಪೂರ್ಣಾಂಕ flags,                  /* Flags to control the opening */
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags              /* Flags showing results of opening */
){
  ಪೂರ್ಣಾಂಕ rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quota_ಗುಂಪು *p_ಗುಂಪು;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  ಆದರೆ( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    ಹಿಂತಿರುಗಿ pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_ಹೊರಹೋಗು_Flags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  p_ಗುಂಪು = quota_ಗುಂಪು_ಕಂಡುಹಿಡಿ(zName);
  ಆದರೆ( p_ಗುಂಪು==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_ಹೊರಹೋಗು_Flags);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, p_ಹೊರಹೋಗು_Flags);
    ಆದರೆ( rc==SQLITE_OK ){
      pFile = quota_ಕಂಡುಹಿಡಿ_File(p_ಗುಂಪು, zName, 1);
      ಆದರೆ( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      ಆದರೆ( pSubOpen->pMethods->i_ಆವೃತ್ತಿ==1 ){
        pQuotaOpen->ತಳ.pMethods = &gQuota.sIoMethodsV1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pQuotaOpen->ತಳ.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,          /* Name of file to be deleted */
  ಪೂರ್ಣಾಂಕ syncDir                 /* Do a directory sync after deleting */
){
  ಪೂರ್ಣಾಂಕ rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quota_ಗುಂಪು *p_ಗುಂಪು;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  ಆದರೆ( rc==SQLITE_OK ){
    quotaEnter();
    p_ಗುಂಪು = quota_ಗುಂಪು_ಕಂಡುಹಿಡಿ(zName);
    ಆದರೆ( p_ಗುಂಪು ){
      pFile = quota_ಕಂಡುಹಿಡಿ_File(p_ಗುಂಪು, zName, 0);
      ಆದರೆ( pFile ){
        ಆದರೆ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          quotaRemoveFile(pFile);
          quota_ಗುಂಪು_Deref(p_ಗುಂಪು);
        }
      }
    }
    quotaLeave();
  }
  ಹಿಂತಿರುಗಿ rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪೂರ್ಣಾಂಕ rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  ಆದರೆ( pFile->nRef==0 ){
    quota_ಗುಂಪು *p_ಗುಂಪು = pFile->p_ಗುಂಪು;
    ಆದರೆ( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quota_ಗುಂಪು_Deref(p_ಗುಂಪು);
  }
  quotaLeave();
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaRead(
  sqlite3_file *pConn,
  ಶೂನ್ಯ *pBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaWrite(
  sqlite3_file *pConn,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 i_ಕೊನೆ = iOfst+iAmt;
  quota_ಗುಂಪು *p_ಗುಂಪು;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  ಆದರೆ( pFile->i_ಗಾತ್ರ<i_ಕೊನೆ ){
    p_ಗುಂಪು = pFile->p_ಗುಂಪು;
    quotaEnter();
    szNew = p_ಗುಂಪು->i_ಗಾತ್ರ - pFile->i_ಗಾತ್ರ + i_ಕೊನೆ;
    ಆದರೆ( szNew>p_ಗುಂಪು->i_ಎಲ್ಲೆ && p_ಗುಂಪು->i_ಎಲ್ಲೆ>0 ){
      ಆದರೆ( p_ಗುಂಪು->xCallback ){
        p_ಗುಂಪು->xCallback(pFile->zFilename, &p_ಗುಂಪು->i_ಎಲ್ಲೆ, szNew,
                          p_ಗುಂಪು->pArg);
      }
      ಆದರೆ( szNew>p_ಗುಂಪು->i_ಎಲ್ಲೆ && p_ಗುಂಪು->i_ಎಲ್ಲೆ>0 ){
        quotaLeave();
        ಹಿಂತಿರುಗಿ SQLITE_ತುಂಬಿದ;
      }
    }
    p_ಗುಂಪು->i_ಗಾತ್ರ = szNew;
    pFile->i_ಗಾತ್ರ = i_ಕೊನೆ;
    quotaLeave();
  }
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaTruncate(sqlite3_file *pConn, sqlite3_int64 ಗಾತ್ರ_2){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪೂರ್ಣಾಂಕ rc = pSubOpen->pMethods->xTruncate(pSubOpen, ಗಾತ್ರ_2);
  quotaFile *pFile = p->pFile;
  quota_ಗುಂಪು *p_ಗುಂಪು;
  ಆದರೆ( rc==SQLITE_OK ){
    quotaEnter();
    p_ಗುಂಪು = pFile->p_ಗುಂಪು;
    p_ಗುಂಪು->i_ಗಾತ್ರ -= pFile->i_ಗಾತ್ರ;
    pFile->i_ಗಾತ್ರ = ಗಾತ್ರ_2;
    p_ಗುಂಪು->i_ಗಾತ್ರ += ಗಾತ್ರ_2;
    quotaLeave();
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaSync(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quota_File_ಗಾತ್ರ(sqlite3_file *pConn, sqlite3_int64 *p_ಗಾತ್ರ){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quota_ಗುಂಪು *p_ಗುಂಪು;
  sqlite3_int64 sz;
  ಪೂರ್ಣಾಂಕ rc;

  rc = pSubOpen->pMethods->x_File_ಗಾತ್ರ(pSubOpen, &sz);
  ಆದರೆ( rc==SQLITE_OK ){
    quotaEnter();
    p_ಗುಂಪು = pFile->p_ಗುಂಪು;
    p_ಗುಂಪು->i_ಗಾತ್ರ -= pFile->i_ಗಾತ್ರ;
    pFile->i_ಗಾತ್ರ = sz;
    p_ಗುಂಪು->i_ಗಾತ್ರ += sz;
    quotaLeave();
    *p_ಗಾತ್ರ = sz;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quota_ಬೀಗ(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ ಬೀಗ){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->x_ಬೀಗ(pSubOpen, ಬೀಗ);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaUnlock(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ ಬೀಗ){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xUnlock(pSubOpen, ಬೀಗ);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quota_Check_Reserved_ಬೀಗ(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->x_Check_Reserved_ಬೀಗ(pSubOpen, p_Res_ಹೊರಹೋಗು);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaFileControl(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಪೂರ್ಣಾಂಕ rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#ಆದರೆ defined(SQLITE_FCNTL_VFSNAME)
  ಆದರೆ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(ಅಕ್ಷರ**)pArg = sqlite3_mprintf("quota/%z", *(ಅಕ್ಷರ**)pArg);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quota_Sector_ಗಾತ್ರ(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->x_Sector_ಗಾತ್ರ(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ಪೂರ್ಣಾಂಕ iRegion,                    /* Region to retrieve */
  ಪೂರ್ಣಾಂಕ szRegion,                   /* Size of regions */
  ಪೂರ್ಣಾಂಕ bExtend,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಅಸ್ಥಿರ **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quota_Shm_ಬೀಗ(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ಪೂರ್ಣಾಂಕ ofst,                  /* First lock to acquire or release */
  ಪೂರ್ಣಾಂಕ n,                     /* Number of locks to acquire or release */
  ಪೂರ್ಣಾಂಕ flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->x_Shm_ಬೀಗ(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಶೂನ್ಯ quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotaShmUnmap(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
ಪೂರ್ಣಾಂಕ sqlite3_quota_initialize(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOrigVfsName, ಪೂರ್ಣಾಂಕ makeDefault){
  sqlite3_vfs *pOrigVfs;
  ಆದರೆ( gQuota.isInitialized ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(zOrigVfsName);
  ಆದರೆ( pOrigVfs==0 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  ಪ್ರತಿಜ್ಞಾ( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ಆದರೆ( !gQuota.pMutex ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += ಗಾತ್ರ(quotaConn);
  gQuota.sThisVfs.zName = "quota";
  gQuota.sIoMethodsV1.i_ಆವೃತ್ತಿ = 1;
  gQuota.sIoMethodsV1.xClose = quotaClose;
  gQuota.sIoMethodsV1.xRead = quotaRead;
  gQuota.sIoMethodsV1.xWrite = quotaWrite;
  gQuota.sIoMethodsV1.xTruncate = quotaTruncate;
  gQuota.sIoMethodsV1.xSync = quotaSync;
  gQuota.sIoMethodsV1.x_File_ಗಾತ್ರ = quota_File_ಗಾತ್ರ;
  gQuota.sIoMethodsV1.x_ಬೀಗ = quota_ಬೀಗ;
  gQuota.sIoMethodsV1.xUnlock = quotaUnlock;
  gQuota.sIoMethodsV1.x_Check_Reserved_ಬೀಗ = quota_Check_Reserved_ಬೀಗ;
  gQuota.sIoMethodsV1.xFileControl = quotaFileControl;
  gQuota.sIoMethodsV1.x_Sector_ಗಾತ್ರ = quota_Sector_ಗಾತ್ರ;
  gQuota.sIoMethodsV1.xDeviceCharacteristics = quotaDeviceCharacteristics;
  gQuota.sIoMethodsV2 = gQuota.sIoMethodsV1;
  gQuota.sIoMethodsV2.i_ಆವೃತ್ತಿ = 2;
  gQuota.sIoMethodsV2.xShmMap = quotaShmMap;
  gQuota.sIoMethodsV2.x_Shm_ಬೀಗ = quota_Shm_ಬೀಗ;
  gQuota.sIoMethodsV2.xShmBarrier = quotaShmBarrier;
  gQuota.sIoMethodsV2.xShmUnmap = quotaShmUnmap;
  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&gQuota.sThisVfs, makeDefault);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3_quota_shutdown(ಶೂನ್ಯ){
  quota_ಗುಂಪು *p_ಗುಂಪು;
  ಆದರೆ( gQuota.isInitialized==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE;
  ಗಾಗಿ(p_ಗುಂಪು=gQuota.p_ಗುಂಪು; p_ಗುಂಪು; p_ಗುಂಪು=p_ಗುಂಪು->p_ಮುಂದಿನ){
    ಆದರೆ( quota_ಗುಂಪು_Open_File_ಎಣಿಕೆ(p_ಗುಂಪು)>0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE;
  }
  ಆಗ( gQuota.p_ಗುಂಪು ){
    p_ಗುಂಪು = gQuota.p_ಗುಂಪು;
    gQuota.p_ಗುಂಪು = p_ಗುಂಪು->p_ಮುಂದಿನ;
    p_ಗುಂಪು->i_ಎಲ್ಲೆ = 0;
    ಪ್ರತಿಜ್ಞಾ( quota_ಗುಂಪು_Open_File_ಎಣಿಕೆ(p_ಗುಂಪು)==0 );
    quota_ಗುಂಪು_Deref(p_ಗುಂಪು);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, ಗಾತ್ರ(gQuota));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3_quota_ಗಣ(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ನಮೂನೆ,           /* The filename pattern */
  sqlite3_int64 i_ಎಲ್ಲೆ,           /* New quota to set for this quota group */
  ಶೂನ್ಯ (*xCallback)(              /* Callback invoked when going over quota */
     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_ಎಲ್ಲೆ,        /* IN/OUT: The current limit */
     sqlite3_int64 i_ಗಾತ್ರ,           /* Total size of all files in the group */
     ಶೂನ್ಯ *pArg                     /* Client data */
  ),
  ಶೂನ್ಯ *pArg,                     /* client data passed thru to callback */
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*)         /* Optional destructor for pArg */
){
  quota_ಗುಂಪು *p_ಗುಂಪು;
  quotaEnter();
  p_ಗುಂಪು = gQuota.p_ಗುಂಪು;
  ಆಗ( p_ಗುಂಪು && strcmp(p_ಗುಂಪು->z_ನಮೂನೆ, z_ನಮೂನೆ)!=0 ){
    p_ಗುಂಪು = p_ಗುಂಪು->p_ಮುಂದಿನ;
  }
  ಆದರೆ( p_ಗುಂಪು==0 ){
    ಪೂರ್ಣಾಂಕ n_ನಮೂನೆ = (ಪೂರ್ಣಾಂಕ)(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ನಮೂನೆ) & 0x3fffffff);
    ಆದರೆ( i_ಎಲ್ಲೆ<=0 ){
      quotaLeave();
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    p_ಗುಂಪು = (quota_ಗುಂಪು *)sqlite3_malloc( ಗಾತ್ರ(*p_ಗುಂಪು) + n_ನಮೂನೆ + 1 );
    ಆದರೆ( p_ಗುಂಪು==0 ){
      quotaLeave();
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    memset(p_ಗುಂಪು, 0, ಗಾತ್ರ(*p_ಗುಂಪು));
    p_ಗುಂಪು->z_ನಮೂನೆ = (ಅಕ್ಷರ*)&p_ಗುಂಪು[1];
    ಸ್ಮರಣೆ_ನಕಲು((ಅಕ್ಷರ *)p_ಗುಂಪು->z_ನಮೂನೆ, z_ನಮೂನೆ, n_ನಮೂನೆ+1);
    ಆದರೆ( gQuota.p_ಗುಂಪು ) gQuota.p_ಗುಂಪು->ppPrev = &p_ಗುಂಪು->p_ಮುಂದಿನ;
    p_ಗುಂಪು->p_ಮುಂದಿನ = gQuota.p_ಗುಂಪು;
    p_ಗುಂಪು->ppPrev = &gQuota.p_ಗುಂಪು;
    gQuota.p_ಗುಂಪು = p_ಗುಂಪು;
  }
  p_ಗುಂಪು->i_ಎಲ್ಲೆ = i_ಎಲ್ಲೆ;
  p_ಗುಂಪು->xCallback = xCallback;
  ಆದರೆ( p_ಗುಂಪು->xDestroy && p_ಗುಂಪು->pArg!=pArg ){
    p_ಗುಂಪು->xDestroy(p_ಗುಂಪು->pArg);
  }
  p_ಗುಂಪು->pArg = pArg;
  p_ಗುಂಪು->xDestroy = xDestroy;
  quota_ಗುಂಪು_Deref(p_ಗುಂಪು);
  quotaLeave();
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
ಪೂರ್ಣಾಂಕ sqlite3_quota_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  ಅಕ್ಷರ *z_ತುಂಬಿದ = 0;
  sqlite3_file *fd;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ ಹೊರಹೋಗು_Flags = 0;
  sqlite3_int64 i_ಗಾತ್ರ;
  ಪೂರ್ಣಾಂಕ nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  ಆದರೆ( fd==0 ){
    rc = SQLITE_NOMEM;
  }ಇಲ್ಲದಿದ್ದರೆ{
    z_ತುಂಬಿದ = &((ಅಕ್ಷರ *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->x_ತುಂಬಿದ_Pathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, z_ತುಂಬಿದ);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    z_ತುಂಬಿದ[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ತುಂಬಿದ)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, z_ತುಂಬಿದ, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &ಹೊರಹೋಗು_Flags);
    ಆದರೆ( rc==SQLITE_OK ){
      fd->pMethods->x_File_ಗಾತ್ರ(fd, &i_ಗಾತ್ರ);
      fd->pMethods->xClose(fd);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==SQLITE_CANTOPEN ){
      quota_ಗುಂಪು *p_ಗುಂಪು;
      quotaFile *pFile;
      quotaEnter();
      p_ಗುಂಪು = quota_ಗುಂಪು_ಕಂಡುಹಿಡಿ(z_ತುಂಬಿದ);
      ಆದರೆ( p_ಗುಂಪು ){
        pFile = quota_ಕಂಡುಹಿಡಿ_File(p_ಗುಂಪು, z_ತುಂಬಿದ, 0);
        ಆದರೆ( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಹುಲಕ){
  quota_FILE *p = 0;
  ಅಕ್ಷರ *z_ತುಂಬಿದ = 0;
  ಅಕ್ಷರ *z_ತುಂಬಿದ_Translated = 0;
  ಪೂರ್ಣಾಂಕ rc;
  quota_ಗುಂಪು *p_ಗುಂಪು;
  quotaFile *pFile;

  z_ತುಂಬಿದ = (ಅಕ್ಷರ*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ಆದರೆ( z_ತುಂಬಿದ==0 ) ಹಿಂತಿರುಗಿ 0;
  rc = gQuota.pOrigVfs->x_ತುಂಬಿದ_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_ತುಂಬಿದ);
  ಆದರೆ( rc ) ಹೋಗು quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(ಗಾತ್ರ(*p));
  ಆದರೆ( p==0 ) ಹೋಗು quota_fopen_error;
  memset(p, 0, ಗಾತ್ರ(*p));
  z_ತುಂಬಿದ_Translated = quota_utf8_ಗಮ್ಯ_mbcs(z_ತುಂಬಿದ);
  ಆದರೆ( z_ತುಂಬಿದ_Translated==0 ) ಹೋಗು quota_fopen_error;
  p->f = fopen(z_ತುಂಬಿದ_Translated, z_ಬಹುಲಕ);
  ಆದರೆ( p->f==0 ) ಹೋಗು quota_fopen_error;
  quotaEnter();
  p_ಗುಂಪು = quota_ಗುಂಪು_ಕಂಡುಹಿಡಿ(z_ತುಂಬಿದ);
  ಆದರೆ( p_ಗುಂಪು ){
    pFile = quota_ಕಂಡುಹಿಡಿ_File(p_ಗುಂಪು, z_ತುಂಬಿದ, 1);
    ಆದರೆ( pFile==0 ){
      quotaLeave();
      ಹೋಗು quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(z_ತುಂಬಿದ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  p->zMbcsName = z_ತುಂಬಿದ_Translated;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ p;

quota_fopen_error:
  quota_mbcs_free(z_ತುಂಬಿದ_Translated);
  sqlite3_free(z_ತುಂಬಿದ);
  ಆದರೆ( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  ಶೂನ್ಯ *pBuf,            /* Store the content here */
  size_t ಗಾತ್ರ_2,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  ಹಿಂತಿರುಗಿ fread(pBuf, ಗಾತ್ರ_2, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pBuf,      /* Take content to write from here */
  size_t ಗಾತ್ರ_2,           /* Size of each element */
  size_t nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 i_ಕೊನೆ;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  size_t rc;

  iOfst = ftell(p->f);
  i_ಕೊನೆ = iOfst + ಗಾತ್ರ_2*nmemb;
  pFile = p->pFile;
  ಆದರೆ( pFile && pFile->i_ಗಾತ್ರ<i_ಕೊನೆ ){
    quota_ಗುಂಪು *p_ಗುಂಪು = pFile->p_ಗುಂಪು;
    quotaEnter();
    szNew = p_ಗುಂಪು->i_ಗಾತ್ರ - pFile->i_ಗಾತ್ರ + i_ಕೊನೆ;
    ಆದರೆ( szNew>p_ಗುಂಪು->i_ಎಲ್ಲೆ && p_ಗುಂಪು->i_ಎಲ್ಲೆ>0 ){
      ಆದರೆ( p_ಗುಂಪು->xCallback ){
        p_ಗುಂಪು->xCallback(pFile->zFilename, &p_ಗುಂಪು->i_ಎಲ್ಲೆ, szNew,
                          p_ಗುಂಪು->pArg);
      }
      ಆದರೆ( szNew>p_ಗುಂಪು->i_ಎಲ್ಲೆ && p_ಗುಂಪು->i_ಎಲ್ಲೆ>0 ){
        i_ಕೊನೆ = p_ಗುಂಪು->i_ಎಲ್ಲೆ - p_ಗುಂಪು->i_ಗಾತ್ರ + pFile->i_ಗಾತ್ರ;
        nmemb = (size_t)((i_ಕೊನೆ - iOfst)/ಗಾತ್ರ_2);
        i_ಕೊನೆ = iOfst + ಗಾತ್ರ_2*nmemb;
        szNew = p_ಗುಂಪು->i_ಗಾತ್ರ - pFile->i_ಗಾತ್ರ + i_ಕೊನೆ;
      }
    }
    p_ಗುಂಪು->i_ಗಾತ್ರ = szNew;
    pFile->i_ಗಾತ್ರ = i_ಕೊನೆ;
    quotaLeave();
  }ಇಲ್ಲದಿದ್ದರೆ{
    pFile = 0;
  }
  rc = fwrite(pBuf, ಗಾತ್ರ_2, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  ಆದರೆ( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 i_New_ಕೊನೆ = iOfst + ಗಾತ್ರ_2*nWritten;
    ಆದರೆ( i_New_ಕೊನೆ<i_ಕೊನೆ ) i_New_ಕೊನೆ = i_ಕೊನೆ;
    quotaEnter();
    pFile->p_ಗುಂಪು->i_ಗಾತ್ರ += i_New_ಕೊನೆ - pFile->i_ಗಾತ್ರ;
    pFile->i_ಗಾತ್ರ = i_New_ಕೊನೆ;
    quotaLeave();
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Close an open quota_FILE stream.
*/
ಪೂರ್ಣಾಂಕ sqlite3_quota_fclose(quota_FILE *p){
  ಪೂರ್ಣಾಂಕ rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  ಆದರೆ( pFile ){
    quotaEnter();
    pFile->nRef--;
    ಆದರೆ( pFile->nRef==0 ){
      quota_ಗುಂಪು *p_ಗುಂಪು = pFile->p_ಗುಂಪು;
      ಆದರೆ( pFile->deleteOnClose ){
        gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
        quotaRemoveFile(pFile);
      }
      quota_ಗುಂಪು_Deref(p_ಗುಂಪು);
    }
    quotaLeave();
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  quota_mbcs_free(p->zMbcsName);
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
ಪೂರ್ಣಾಂಕ sqlite3_quota_fflush(quota_FILE *p, ಪೂರ್ಣಾಂಕ doFsync){
  ಪೂರ್ಣಾಂಕ rc;
  rc = fflush(p->f);
  ಆದರೆ( rc==0 && doFsync ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
    rc = _commit(_fileno(p->f));
#ಇಲ್ಲದಿದ್ದರೆ
    rc = fsync(fileno(p->f));
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಹಿಂತಿರುಗಿ rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
ಪೂರ್ಣಾಂಕ sqlite3_quota_fseek(quota_FILE *p, ದೀರ್ಘ ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ whence){
  ಹಿಂತಿರುಗಿ fseek(p->f, ಸ್ಥಾನಾಂತರ, whence);
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
ದೀರ್ಘ sqlite3_quota_ftell(quota_FILE *p){
  ಹಿಂತಿರುಗಿ ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
ಪೂರ್ಣಾಂಕ sqlite3_quota_ferror(quota_FILE *p){
  ಹಿಂತಿರುಗಿ ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
ಪೂರ್ಣಾಂಕ sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( (pFile = p->pFile)!=0 && pFile->i_ಗಾತ್ರ<szNew ){
    quota_ಗುಂಪು *p_ಗುಂಪು;
    ಆದರೆ( pFile->i_ಗಾತ್ರ<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      ಹಿಂತಿರುಗಿ -1;
    }
    p_ಗುಂಪು = pFile->p_ಗುಂಪು;
    quotaEnter();
    p_ಗುಂಪು->i_ಗಾತ್ರ += szNew - pFile->i_ಗಾತ್ರ;
    quotaLeave();
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#  ಆದರೆ defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (ದೀರ್ಘ)szNew);
#  ಇಲ್ಲದಿದ್ದರೆ
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#ಇಲ್ಲದಿದ್ದರೆ
  rc = ftruncate(fileno(p->f), szNew);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pFile && rc==0 ){
    quota_ಗುಂಪು *p_ಗುಂಪು = pFile->p_ಗುಂಪು;
    quotaEnter();
    p_ಗುಂಪು->i_ಗಾತ್ರ += szNew - pFile->i_ಗಾತ್ರ;
    pFile->i_ಗಾತ್ರ = szNew;
    quotaLeave();
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
ಪೂರ್ಣಾಂಕ sqlite3_quota_file_mtime(quota_FILE *p, ಸಮಯ_t *p_ಸಮಯ){
  ಪೂರ್ಣಾಂಕ rc;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ರಚನೆ _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ಇಲ್ಲದಿದ್ದರೆ
  ರಚನೆ stat buf;
  rc = fstat(fileno(p->f), &buf);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( rc==0 ) *p_ಸಮಯ = buf.st_mtime;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  ಪೂರ್ಣಾಂಕ rc;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ರಚನೆ _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#ಇಲ್ಲದಿದ್ದರೆ
  ರಚನೆ stat buf;
  rc = fstat(fileno(p->f), &buf);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc==0 ? buf.st_ಗಾತ್ರ : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_ಗಾತ್ರ(quota_FILE *p){
  ಹಿಂತಿರುಗಿ p->pFile ? p->pFile->i_ಗಾತ್ರ : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
ದೀರ್ಘ sqlite3_quota_file_ಲಭ್ಯ(quota_FILE *p){
  FILE* f = p->f;
  ದೀರ್ಘ pos1, pos2;
  ಪೂರ್ಣಾಂಕ rc;
  pos1 = ftell(f);
  ಆದರೆ ( pos1 < 0 ) ಹಿಂತಿರುಗಿ -1;
  rc = fseek(f, 0, SEEK_END);
  ಆದರೆ ( rc != 0 ) ಹಿಂತಿರುಗಿ -1;
  pos2 = ftell(f);
  ಆದರೆ ( pos2 < 0 ) ಹಿಂತಿರುಗಿ -1;
  rc = fseek(f, pos1, SEEK_SET);
  ಆದರೆ ( rc != 0 ) ಹಿಂತಿರುಗಿ -1;
  ಹಿಂತಿರುಗಿ pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
ಪೂರ್ಣಾಂಕ sqlite3_quota_remove(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  ಅಕ್ಷರ *z_ತುಂಬಿದ;            /* Full pathname for zFilename */
  size_t n_ತುಂಬಿದ;           /* Number of bytes in zFilename */
  ಪೂರ್ಣಾಂಕ rc;                 /* Result code */
  quota_ಗುಂಪು *p_ಗುಂಪು;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *p_ಮುಂದಿನ_File;   /* next file in the group */
  ಪೂರ್ಣಾಂಕ diff;               /* Difference between filenames */
  ಅಕ್ಷರ c;                 /* First character past end of pattern */

  z_ತುಂಬಿದ = (ಅಕ್ಷರ*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ಆದರೆ( z_ತುಂಬಿದ==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->x_ತುಂಬಿದ_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_ತುಂಬಿದ);
  ಆದರೆ( rc ){
    sqlite3_free(z_ತುಂಬಿದ);
    ಹಿಂತಿರುಗಿ rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  n_ತುಂಬಿದ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ತುಂಬಿದ);
  ಆದರೆ( n_ತುಂಬಿದ>0 && (z_ತುಂಬಿದ[n_ತುಂಬಿದ-1]=='/' || z_ತುಂಬಿದ[n_ತುಂಬಿದ-1]=='\\') ){
    n_ತುಂಬಿದ--;
    z_ತುಂಬಿದ[n_ತುಂಬಿದ] = 0;
  }

  quotaEnter();
  p_ಗುಂಪು = quota_ಗುಂಪು_ಕಂಡುಹಿಡಿ(z_ತುಂಬಿದ);
  ಆದರೆ( p_ಗುಂಪು ){
    ಗಾಗಿ(pFile=p_ಗುಂಪು->pFiles; pFile && rc==SQLITE_OK; pFile=p_ಮುಂದಿನ_File){
      p_ಮುಂದಿನ_File = pFile->p_ಮುಂದಿನ;
      diff = ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z_ತುಂಬಿದ, pFile->zFilename, n_ತುಂಬಿದ);
      ಆದರೆ( diff==0 && ((c = pFile->zFilename[n_ತುಂಬಿದ])==0 || c=='/' || c=='\\') ){
        ಆದರೆ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quota_ಗುಂಪು_Deref(p_ಗುಂಪು);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(z_ತುಂಬಿದ);
  ಹಿಂತಿರುಗಿ rc;
}

/***************************** Test Code ***********************************/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TclQuotaCallback TclQuotaCallback;
ರಚನೆ TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);


/*
** This is the callback from a quota-over-limit.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tclQuotaCallback(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *pi_ಎಲ್ಲೆ,         /* IN/OUT: The current limit */
  sqlite3_int64 i_ಗಾತ್ರ,            /* Total size of all files in the group */
  ಶೂನ್ಯ *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ rnd;               /* Random part of pVarname */
  ಪೂರ್ಣಾಂಕ rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pVarname);
  sqlite3_randomness(ಗಾತ್ರ(rnd), (ಶೂನ್ಯ *)&rnd);
  Tcl_Append_Obj_ಗಮ್ಯ_Obj(pVarname, Tcl_NewIntObj((ಪೂರ್ಣಾಂಕ)(rnd&0x7FFFFFFF)));
  Tcl_Obj_ಗಣ_Var2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*pi_ಎಲ್ಲೆ), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(i_ಗಾತ್ರ));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  ಆದರೆ( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *p_ಎಲ್ಲೆ = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, p_ಎಲ್ಲೆ, &x);
    *pi_ಎಲ್ಲೆ = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pVarname);
  ಆದರೆ( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tclCallbackDestructor(ಶೂನ್ಯ *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  ಆದರೆ( p ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(p->pScript);
    sqlite3_free((ಅಕ್ಷರ *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_initialize(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;              /* Name of new quota VFS */
  ಪೂರ್ಣಾಂಕ makeDefault;                /* True to make the new VFS the default */
  ಪೂರ್ಣಾಂಕ rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "NAME MAKEDEFAULT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_shutdown(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;                         /* Value returned by quota_shutdown() */

  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_ಗಣ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ನಮೂನೆ;           /* File pattern to configure */
  Tcl_WideInt i_ಎಲ್ಲೆ;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  ಪೂರ್ಣಾಂಕ rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_ಗಾತ್ರ nScript;               /* Length of callback script */
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*);        /* Optional destructor for pArg */
  ಶೂನ್ಯ (*xCallback)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_int64 *, sqlite3_int64, ಶೂನ್ಯ *);

  /* Process arguments */
  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  z_ನಮೂನೆ = Tcl_GetString(objv[1]);
  ಆದರೆ( Tcl_GetWideIntFromObj(interp, objv[2], &i_ಎಲ್ಲೆ) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  ಆದರೆ( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(ಗಾತ್ರ(TclQuotaCallback));
    ಆದರೆ( !p ){
      Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)"SQLITE_NOMEM", TCL_STATIC);
      ಹಿಂತಿರುಗಿ TCL_OK;
    }
    memset(p, 0, ಗಾತ್ರ(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_ಗಣ(z_ನಮೂನೆ, i_ಎಲ್ಲೆ, xCallback, (ಶೂನ್ಯ*)p, xDestroy);

  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_file(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename;          /* File pattern to configure */
  ಪೂರ್ಣಾಂಕ rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "FILENAME");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_dump(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *p_ಫಲಿತಾಂಶ;
  Tcl_Obj *p_ಗುಂಪು_ಪದ;
  Tcl_Obj *p_File_ಪದ;
  quota_ಗುಂಪು *p_ಗುಂಪು;
  quotaFile *pFile;

  p_ಫಲಿತಾಂಶ = Tcl_NewObj();
  quotaEnter();
  ಗಾಗಿ(p_ಗುಂಪು=gQuota.p_ಗುಂಪು; p_ಗುಂಪು; p_ಗುಂಪು=p_ಗುಂಪು->p_ಮುಂದಿನ){
    p_ಗುಂಪು_ಪದ = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, p_ಗುಂಪು_ಪದ,
          Tcl_NewStringObj(p_ಗುಂಪು->z_ನಮೂನೆ, -1));
    Tcl_ListObjAppendElement(interp, p_ಗುಂಪು_ಪದ,
          Tcl_NewWideIntObj(p_ಗುಂಪು->i_ಎಲ್ಲೆ));
    Tcl_ListObjAppendElement(interp, p_ಗುಂಪು_ಪದ,
          Tcl_NewWideIntObj(p_ಗುಂಪು->i_ಗಾತ್ರ));
    ಗಾಗಿ(pFile=p_ಗುಂಪು->pFiles; pFile; pFile=pFile->p_ಮುಂದಿನ){
      ಪೂರ್ಣಾಂಕ i;
      ಅಕ್ಷರ z_ತಾತ್ಕಾಲಿಕ[1000];
      p_File_ಪದ = Tcl_NewObj();
      sqlite3_snprintf(ಗಾತ್ರ(z_ತಾತ್ಕಾಲಿಕ), z_ತಾತ್ಕಾಲಿಕ, "%s", pFile->zFilename);
      ಗಾಗಿ(i=0; z_ತಾತ್ಕಾಲಿಕ[i]; i++){ ಆದರೆ( z_ತಾತ್ಕಾಲಿಕ[i]=='\\' ) z_ತಾತ್ಕಾಲಿಕ[i] = '/'; }
      Tcl_ListObjAppendElement(interp, p_File_ಪದ,
            Tcl_NewStringObj(z_ತಾತ್ಕಾಲಿಕ, -1));
      Tcl_ListObjAppendElement(interp, p_File_ಪದ,
            Tcl_NewWideIntObj(pFile->i_ಗಾತ್ರ));
      Tcl_ListObjAppendElement(interp, p_File_ಪದ,
            Tcl_NewWideIntObj(pFile->nRef));
      Tcl_ListObjAppendElement(interp, p_File_ಪದ,
            Tcl_NewWideIntObj(pFile->deleteOnClose));
      Tcl_ListObjAppendElement(interp, p_ಗುಂಪು_ಪದ, p_File_ಪದ);
    }
    Tcl_ListObjAppendElement(interp, p_ಫಲಿತಾಂಶ, p_ಗುಂಪು_ಪದ);
  }
  quotaLeave();
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, p_ಫಲಿತಾಂಶ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/* Defined in test1.c */
ಬಾಹ್ಯ ಶೂನ್ಯ *sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(ಶೂನ್ಯ*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_fopen(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename;          /* File pattern to configure */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಹುಲಕ;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "FILENAME MODE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  z_ಬಹುಲಕ = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, z_ಬಹುಲಕ);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ*)sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(p), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_fread(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಅಕ್ಷರ *zBuf;
  ಪೂರ್ಣಾಂಕ sz;
  ಪೂರ್ಣಾಂಕ nElem;
  size_t got;

  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE SIZE NELEM");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zBuf = (ಅಕ್ಷರ*)sqlite3_malloc( sz*nElem + 1 );
  ಆದರೆ( zBuf==0 ){
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, "out of memory", TCL_STATIC);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_fwrite(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಅಕ್ಷರ *zBuf;
  ಪೂರ್ಣಾಂಕ sz;
  ಪೂರ್ಣಾಂಕ nElem;
  size_t got;

  ಆದರೆ( objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(got));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_fclose(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_fflush(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ doSync = 0;

  ಆದರೆ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE ?HARDSYNC?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  ಆದರೆ( objc==3 ){
    ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_fseek(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಪೂರ್ಣಾಂಕ ofst;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zWhence;
  ಪೂರ್ಣಾಂಕ whence;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE OFFSET WHENCE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  ಆದರೆ( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }ಇಲ್ಲದಿದ್ದರೆ{
    Tcl_Append_ಫಲಿತಾಂಶ(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_rewind(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_ftell(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(x));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_ftruncate(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE SIZE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  ಆದರೆ( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_file_ಗಾತ್ರ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_ಗಾತ್ರ(p);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(x));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_file_truesize(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(x));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_file_mtime(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಸಮಯ_t t;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(t));
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_remove(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename;          /* File pattern to configure */
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "FILENAME");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_glob(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ನಮೂನೆ;          /* The glob pattern */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ;             /* Text to compare against the pattern */
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "PATTERN TEXT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  z_ನಮೂನೆ = Tcl_GetString(objv[1]);
  z_ಪಠ್ಯ = Tcl_GetString(objv[2]);
  rc = quotaStrglob(z_ನಮೂನೆ, z_ಪಠ್ಯ);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_file_ಲಭ್ಯ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_ಲಭ್ಯ(p);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(x));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_quota_ferror(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ಪೂರ್ಣಾಂಕ x;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(x));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ಪೂರ್ಣಾಂಕ Sqlitequota_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_quota_initialize",    test_quota_initialize },
    { "sqlite3_quota_shutdown",      test_quota_shutdown },
    { "sqlite3_quota_set",           test_quota_ಗಣ },
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
    { "sqlite3_quota_file_size",     test_quota_file_ಗಾತ್ರ },
    { "sqlite3_quota_file_truesize", test_quota_file_truesize },
    { "sqlite3_quota_file_mtime",    test_quota_file_mtime },
    { "sqlite3_quota_remove",        test_quota_remove },
    { "sqlite3_quota_glob",          test_quota_glob },
    { "sqlite3_quota_file_available",test_quota_file_ಲಭ್ಯ },
    { "sqlite3_quota_ferror",        test_quota_ferror },
  };
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
