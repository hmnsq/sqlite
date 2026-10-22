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
#అన్తర్భావ "test_quota.h"
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#ఒకవేళ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#నిర్వచించు sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#నిర్వచించు sqlite3_mutex_free(X)
#నిర్వచించు sqlite3_mutex_enter(X)
#నిర్వచించు sqlite3_mutex_try(X)      SQLITE_OK
#నిర్వచించు sqlite3_mutex_leave(X)
#నిర్వచించు sqlite3_mutex_held(X)     ((శూన్య)(X),1)
#నిర్వచించు sqlite3_mutex_notheld(X)  ((శూన్య)(X),1)
#అయితే_ముగింపు /* SQLITE_THREADSAFE==0 */


#నిర్వచితమైతే _WIN32
# include <windows.h>
# include <io.h>
#లేకపోతే
# include <unistd.h>
#అయితే_ముగింపు


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
రకనిర్వచనం నిర్మాణం quota_సమూహం quota_సమూహం;
రకనిర్వచనం నిర్మాణం quotaConn quotaConn;
రకనిర్వచనం నిర్మాణం quotaFile quotaFile;

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
నిర్మాణం quota_సమూహం {
  స్థిరరాశి అక్షరం *z_నమూనా;          /* Filename pattern to be quotaed */
  sqlite3_int64 i_హద్దు;          /* Upper bound on total file size */
  sqlite3_int64 i_కొలత;           /* Current size of all files */
  శూన్య (*xCallback)(             /* Callback invoked when going over quota */
     స్థిరరాశి అక్షరం *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_హద్దు,        /* IN/OUT: The current limit */
     sqlite3_int64 i_కొలత,           /* Total size of all files in the group */
     శూన్య *pArg                     /* Client data */
  );
  శూన్య *pArg;                    /* Third argument to the xCallback() */
  శూన్య (*xDestroy)(శూన్య*);       /* Optional destructor for pArg */
  quota_సమూహం *p_తదుపరి, **ppPrev;   /* Doubly linked list of all quota objects */
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
నిర్మాణం quotaFile {
  అక్షరం *zFilename;                /* Name of this file */
  quota_సమూహం *p_సమూహం;             /* Quota group to which this file belongs */
  sqlite3_int64 i_కొలత;            /* Current size of this file */
  పూర్ణాంకం nRef;                       /* Number of times this file is open */
  పూర్ణాంకం deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *p_తదుపరి, **ppPrev;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
నిర్మాణం quotaConn {
  sqlite3_file పునాది;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
నిర్మాణం quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#నిర్వచితమైతే _WIN32
  అక్షరం *zMbcsName;        /* Full MBCS pathname of the file */
#అయితే_ముగింపు
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
స్థిర నిర్మాణం {
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
  పూర్ణాంకం isInitialized;

  /* For run-time access any of the other global data structures in this
  ** shim, the following mutex must be held.
  */
  sqlite3_mutex *pMutex;

  /* List of quotaGroup objects.
  */
  quota_సమూహం *p_సమూహం;

} gQuota;

/************************* Utility Routines *********************************/
/*
** Acquire and release the mutex used to serialize access to the
** list of quotaGroups.
*/
స్థిర శూన్య quotaEnter(శూన్య){ sqlite3_mutex_enter(gQuota.pMutex); }
స్థిర శూన్య quotaLeave(శూన్య){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
స్థిర పూర్ణాంకం quota_సమూహం_Open_File_లెక్క(quota_సమూహం *p_సమూహం){
  పూర్ణాంకం N = 0;
  quotaFile *pFile = p_సమూహం->pFiles;
  అయితే( pFile ){
    ఒకవేళ( pFile->nRef ) N++;
    pFile = pFile->p_తదుపరి;
  }
  తిరిగి N;
}

/* Remove a file from a quota group.
*/
స్థిర శూన్య quotaRemoveFile(quotaFile *pFile){
  quota_సమూహం *p_సమూహం = pFile->p_సమూహం;
  p_సమూహం->i_కొలత -= pFile->i_కొలత;
  *pFile->ppPrev = pFile->p_తదుపరి;
  ఒకవేళ( pFile->p_తదుపరి ) pFile->p_తదుపరి->ppPrev = pFile->ppPrev;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
స్థిర శూన్య quotaRemoveAllFiles(quota_సమూహం *p_సమూహం){
  అయితే( p_సమూహం->pFiles ){
    ప్రతిజ్ఞా( p_సమూహం->pFiles->nRef==0 );
    quotaRemoveFile(p_సమూహం->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
స్థిర శూన్య quota_సమూహం_Deref(quota_సమూహం *p_సమూహం){
  ఒకవేళ( p_సమూహం->i_హద్దు==0 && quota_సమూహం_Open_File_లెక్క(p_సమూహం)==0 ){
    quotaRemoveAllFiles(p_సమూహం);
    *p_సమూహం->ppPrev = p_సమూహం->p_తదుపరి;
    ఒకవేళ( p_సమూహం->p_తదుపరి ) p_సమూహం->p_తదుపరి->ppPrev = p_సమూహం->ppPrev;
    ఒకవేళ( p_సమూహం->xDestroy ) p_సమూహం->xDestroy(p_సమూహం->pArg);
    sqlite3_free(p_సమూహం);
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
స్థిర పూర్ణాంకం quotaStrglob(స్థిరరాశి అక్షరం *zGlob, స్థిరరాశి అక్షరం *z){
  పూర్ణాంకం c, c2, cx;
  పూర్ణాంకం invert;
  పూర్ణాంకం చూసిన;

  అయితే( (c = (*(zGlob++)))!=0 ){
    ఒకవేళ( c=='*' ){
      అయితే( (c=(*(zGlob++))) == '*' || c=='?' ){
        ఒకవేళ( c=='?' && (*(z++))==0 ) తిరిగి 0;
      }
      ఒకవేళ( c==0 ){
        తిరిగి 1;
      }లేకపోతే ఒకవేళ( c=='[' ){
        అయితే( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        తిరిగి (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      అయితే( (c2 = (*(z++)))!=0 ){
        అయితే( c2!=c && c2!=cx ){
          c2 = *(z++);
          ఒకవేళ( c2==0 ) తిరిగి 0;
        }
        ఒకవేళ( quotaStrglob(zGlob,z) ) తిరిగి 1;
      }
      తిరిగి 0;
    }లేకపోతే ఒకవేళ( c=='?' ){
      ఒకవేళ( (*(z++))==0 ) తిరిగి 0;
    }లేకపోతే ఒకవేళ( c=='[' ){
      పూర్ణాంకం పూర్వ_c = 0;
      చూసిన = 0;
      invert = 0;
      c = *(z++);
      ఒకవేళ( c==0 ) తిరిగి 0;
      c2 = *(zGlob++);
      ఒకవేళ( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      ఒకవేళ( c2==']' ){
        ఒకవేళ( c==']' ) చూసిన = 1;
        c2 = *(zGlob++);
      }
      అయితే( c2 && c2!=']' ){
        ఒకవేళ( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && పూర్వ_c>0 ){
          c2 = *(zGlob++);
          ఒకవేళ( c>=పూర్వ_c && c<=c2 ) చూసిన = 1;
          పూర్వ_c = 0;
        }లేకపోతే{
          ఒకవేళ( c==c2 ){
            చూసిన = 1;
          }
          పూర్వ_c = c2;
        }
        c2 = *(zGlob++);
      }
      ఒకవేళ( c2==0 || (చూసిన ^ invert)==0 ) తిరిగి 0;
    }లేకపోతే ఒకవేళ( c=='/' ){
      ఒకవేళ( z[0]!='/' && z[0]!='\\' ) తిరిగి 0;
      z++;
    }లేకపోతే{
      ఒకవేళ( c!=(*(z++)) ) తిరిగి 0;
    }
  }
  తిరిగి *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
స్థిర quota_సమూహం *quota_సమూహం_కనుగొను(స్థిరరాశి అక్షరం *zFilename){
  quota_సమూహం *p;
  కొరకు(p=gQuota.p_సమూహం; p && quotaStrglob(p->z_నమూనా, zFilename)==0;
      p=p->p_తదుపరి){}
  తిరిగి p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
స్థిర sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  తిరిగి (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
స్థిర quotaFile *quota_కనుగొను_File(
  quota_సమూహం *p_సమూహం,     /* Group in which to look for the file */
  స్థిరరాశి అక్షరం *zName,      /* Full pathname of the file */
  పూర్ణాంకం createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = p_సమూహం->pFiles;
  అయితే( pFile && strcmp(pFile->zFilename, zName)!=0 ){
    pFile = pFile->p_తదుపరి;
  }
  ఒకవేళ( pFile==0 && createFlag ){
    పూర్ణాంకం nName = (పూర్ణాంకం)(అక్షరశ్రేణి_పొడవు(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( పరిమాణం(*pFile) + nName + 1 );
    ఒకవేళ( pFile ){
      memset(pFile, 0, పరిమాణం(*pFile));
      pFile->zFilename = (అక్షరం*)&pFile[1];
      జ్ఞాపకం_నకలు(pFile->zFilename, zName, nName+1);
      pFile->p_తదుపరి = p_సమూహం->pFiles;
      ఒకవేళ( p_సమూహం->pFiles ) p_సమూహం->pFiles->ppPrev = &pFile->p_తదుపరి;
      pFile->ppPrev = &p_సమూహం->pFiles;
      p_సమూహం->pFiles = pFile;
      pFile->p_సమూహం = p_సమూహం;
    }
  }
  తిరిగి pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
స్థిర అక్షరం *quota_utf8_గమ్యం_mbcs(స్థిరరాశి అక్షరం *zUtf8){
#నిర్వచితమైతే _WIN32
  size_t n;          /* Bytes in zUtf8 */
  పూర్ణాంకం nWide;         /* number of UTF-16 characters */
  పూర్ణాంకం nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  అక్షరం *zMbcs;       /* The MBCS text */
  పూర్ణాంకం codepage;      /* Code page used by fopen() */

  n = అక్షరశ్రేణి_పొడవు(zUtf8);
  nWide = Multi_Byte_గమ్యం_Wide_Char(CP_UTF8, 0, zUtf8, -1, ఖాళీ, 0);
  ఒకవేళ( nWide==0 ) తిరిగి 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*పరిమాణం(zTmpWide[0]) );
  ఒకవేళ( zTmpWide==0 ) తిరిగి 0;
  Multi_Byte_గమ్యం_Wide_Char(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = Wide_Char_గమ్యం_Multi_Byte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (అక్షరం*)sqlite3_malloc( nMbcs+1 ) : 0;
  ఒకవేళ( zMbcs ){
    Wide_Char_గమ్యం_Multi_Byte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  తిరిగి zMbcs;
#లేకపోతే
  తిరిగి (అక్షరం*)zUtf8;  /* No-op on unix */
#అయితే_ముగింపు
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
స్థిర శూన్య quota_mbcs_free(అక్షరం *zOld){
#నిర్వచితమైతే _WIN32
  sqlite3_free(zOld);
#లేకపోతే
  /* No-op on unix */
#అయితే_ముగింపు
}

/************************* VFS Method Wrappers *****************************/
/*
** This is the xOpen method used for the "quota" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate quota group if it is a
** file that needs to be tracked.
*/
స్థిర పూర్ణాంకం quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  స్థిరరాశి అక్షరం *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  పూర్ణాంకం flags,                  /* Flags to control the opening */
  పూర్ణాంకం *p_బహిర్గతం_Flags              /* Flags showing results of opening */
){
  పూర్ణాంకం rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quota_సమూహం *p_సమూహం;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  ఒకవేళ( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    తిరిగి pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_బహిర్గతం_Flags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  p_సమూహం = quota_సమూహం_కనుగొను(zName);
  ఒకవేళ( p_సమూహం==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_బహిర్గతం_Flags);
  }లేకపోతే{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, p_బహిర్గతం_Flags);
    ఒకవేళ( rc==SQLITE_OK ){
      pFile = quota_కనుగొను_File(p_సమూహం, zName, 1);
      ఒకవేళ( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        తిరిగి SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      ఒకవేళ( pSubOpen->pMethods->i_సంస్కరణ==1 ){
        pQuotaOpen->పునాది.pMethods = &gQuota.sIoMethodsV1;
      }లేకపోతే{
        pQuotaOpen->పునాది.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  తిరిగి rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
స్థిర పూర్ణాంకం quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  స్థిరరాశి అక్షరం *zName,          /* Name of file to be deleted */
  పూర్ణాంకం syncDir                 /* Do a directory sync after deleting */
){
  పూర్ణాంకం rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quota_సమూహం *p_సమూహం;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  ఒకవేళ( rc==SQLITE_OK ){
    quotaEnter();
    p_సమూహం = quota_సమూహం_కనుగొను(zName);
    ఒకవేళ( p_సమూహం ){
      pFile = quota_కనుగొను_File(p_సమూహం, zName, 0);
      ఒకవేళ( pFile ){
        ఒకవేళ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }లేకపోతే{
          quotaRemoveFile(pFile);
          quota_సమూహం_Deref(p_సమూహం);
        }
      }
    }
    quotaLeave();
  }
  తిరిగి rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
స్థిర పూర్ణాంకం quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  పూర్ణాంకం rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  ఒకవేళ( pFile->nRef==0 ){
    quota_సమూహం *p_సమూహం = pFile->p_సమూహం;
    ఒకవేళ( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quota_సమూహం_Deref(p_సమూహం);
  }
  quotaLeave();
  తిరిగి rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
స్థిర పూర్ణాంకం quotaRead(
  sqlite3_file *pConn,
  శూన్య *pBuf,
  పూర్ణాంకం iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
స్థిర పూర్ణాంకం quotaWrite(
  sqlite3_file *pConn,
  స్థిరరాశి శూన్య *pBuf,
  పూర్ణాంకం iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 i_చివర = iOfst+iAmt;
  quota_సమూహం *p_సమూహం;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  ఒకవేళ( pFile->i_కొలత<i_చివర ){
    p_సమూహం = pFile->p_సమూహం;
    quotaEnter();
    szNew = p_సమూహం->i_కొలత - pFile->i_కొలత + i_చివర;
    ఒకవేళ( szNew>p_సమూహం->i_హద్దు && p_సమూహం->i_హద్దు>0 ){
      ఒకవేళ( p_సమూహం->xCallback ){
        p_సమూహం->xCallback(pFile->zFilename, &p_సమూహం->i_హద్దు, szNew,
                          p_సమూహం->pArg);
      }
      ఒకవేళ( szNew>p_సమూహం->i_హద్దు && p_సమూహం->i_హద్దు>0 ){
        quotaLeave();
        తిరిగి SQLITE_నిండిన;
      }
    }
    p_సమూహం->i_కొలత = szNew;
    pFile->i_కొలత = i_చివర;
    quotaLeave();
  }
  తిరిగి pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
స్థిర పూర్ణాంకం quotaTruncate(sqlite3_file *pConn, sqlite3_int64 కొలత){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  పూర్ణాంకం rc = pSubOpen->pMethods->xTruncate(pSubOpen, కొలత);
  quotaFile *pFile = p->pFile;
  quota_సమూహం *p_సమూహం;
  ఒకవేళ( rc==SQLITE_OK ){
    quotaEnter();
    p_సమూహం = pFile->p_సమూహం;
    p_సమూహం->i_కొలత -= pFile->i_కొలత;
    pFile->i_కొలత = కొలత;
    p_సమూహం->i_కొలత += కొలత;
    quotaLeave();
  }
  తిరిగి rc;
}

/* Pass xSync requests through to the original VFS without change
*/
స్థిర పూర్ణాంకం quotaSync(sqlite3_file *pConn, పూర్ణాంకం flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
స్థిర పూర్ణాంకం quota_File_కొలత(sqlite3_file *pConn, sqlite3_int64 *p_కొలత){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quota_సమూహం *p_సమూహం;
  sqlite3_int64 sz;
  పూర్ణాంకం rc;

  rc = pSubOpen->pMethods->x_File_కొలత(pSubOpen, &sz);
  ఒకవేళ( rc==SQLITE_OK ){
    quotaEnter();
    p_సమూహం = pFile->p_సమూహం;
    p_సమూహం->i_కొలత -= pFile->i_కొలత;
    pFile->i_కొలత = sz;
    p_సమూహం->i_కొలత += sz;
    quotaLeave();
    *p_కొలత = sz;
  }
  తిరిగి rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quota_తాళం(sqlite3_file *pConn, పూర్ణాంకం తాళం){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->x_తాళం(pSubOpen, తాళం);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quotaUnlock(sqlite3_file *pConn, పూర్ణాంకం తాళం){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->xUnlock(pSubOpen, తాళం);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quota_Check_Reserved_తాళం(sqlite3_file *pConn, పూర్ణాంకం *p_Res_బహిర్గతం){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->x_Check_Reserved_తాళం(pSubOpen, p_Res_బహిర్గతం);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quotaFileControl(sqlite3_file *pConn, పూర్ణాంకం op, శూన్య *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  పూర్ణాంకం rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#ఒకవేళ defined(SQLITE_FCNTL_VFSNAME)
  ఒకవేళ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(అక్షరం**)pArg = sqlite3_mprintf("quota/%z", *(అక్షరం**)pArg);
  }
#అయితే_ముగింపు
  తిరిగి rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quota_Sector_కొలత(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->x_Sector_కొలత(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  పూర్ణాంకం iRegion,                    /* Region to retrieve */
  పూర్ణాంకం szRegion,                   /* Size of regions */
  పూర్ణాంకం bExtend,                    /* True to extend file if necessary */
  శూన్య అస్థిర **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quota_Shm_తాళం(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  పూర్ణాంకం ofst,                  /* First lock to acquire or release */
  పూర్ణాంకం n,                     /* Number of locks to acquire or release */
  పూర్ణాంకం flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->x_Shm_తాళం(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
స్థిర శూన్య quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం quotaShmUnmap(sqlite3_file *pConn, పూర్ణాంకం deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  తిరిగి pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
పూర్ణాంకం sqlite3_quota_initialize(స్థిరరాశి అక్షరం *zOrigVfsName, పూర్ణాంకం makeDefault){
  sqlite3_vfs *pOrigVfs;
  ఒకవేళ( gQuota.isInitialized ) తిరిగి SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_కనుగొను(zOrigVfsName);
  ఒకవేళ( pOrigVfs==0 ) తిరిగి SQLITE_ERROR;
  ప్రతిజ్ఞా( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  ఒకవేళ( !gQuota.pMutex ){
    తిరిగి SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += పరిమాణం(quotaConn);
  gQuota.sThisVfs.zName = "quota";
  gQuota.sIoMethodsV1.i_సంస్కరణ = 1;
  gQuota.sIoMethodsV1.xClose = quotaClose;
  gQuota.sIoMethodsV1.xRead = quotaRead;
  gQuota.sIoMethodsV1.xWrite = quotaWrite;
  gQuota.sIoMethodsV1.xTruncate = quotaTruncate;
  gQuota.sIoMethodsV1.xSync = quotaSync;
  gQuota.sIoMethodsV1.x_File_కొలత = quota_File_కొలత;
  gQuota.sIoMethodsV1.x_తాళం = quota_తాళం;
  gQuota.sIoMethodsV1.xUnlock = quotaUnlock;
  gQuota.sIoMethodsV1.x_Check_Reserved_తాళం = quota_Check_Reserved_తాళం;
  gQuota.sIoMethodsV1.xFileControl = quotaFileControl;
  gQuota.sIoMethodsV1.x_Sector_కొలత = quota_Sector_కొలత;
  gQuota.sIoMethodsV1.xDeviceCharacteristics = quotaDeviceCharacteristics;
  gQuota.sIoMethodsV2 = gQuota.sIoMethodsV1;
  gQuota.sIoMethodsV2.i_సంస్కరణ = 2;
  gQuota.sIoMethodsV2.xShmMap = quotaShmMap;
  gQuota.sIoMethodsV2.x_Shm_తాళం = quota_Shm_తాళం;
  gQuota.sIoMethodsV2.xShmBarrier = quotaShmBarrier;
  gQuota.sIoMethodsV2.xShmUnmap = quotaShmUnmap;
  sqlite3_vfs_రిజిస్టరు(&gQuota.sThisVfs, makeDefault);
  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3_quota_shutdown(శూన్య){
  quota_సమూహం *p_సమూహం;
  ఒకవేళ( gQuota.isInitialized==0 ) తిరిగి SQLITE_MISUSE;
  కొరకు(p_సమూహం=gQuota.p_సమూహం; p_సమూహం; p_సమూహం=p_సమూహం->p_తదుపరి){
    ఒకవేళ( quota_సమూహం_Open_File_లెక్క(p_సమూహం)>0 ) తిరిగి SQLITE_MISUSE;
  }
  అయితే( gQuota.p_సమూహం ){
    p_సమూహం = gQuota.p_సమూహం;
    gQuota.p_సమూహం = p_సమూహం->p_తదుపరి;
    p_సమూహం->i_హద్దు = 0;
    ప్రతిజ్ఞా( quota_సమూహం_Open_File_లెక్క(p_సమూహం)==0 );
    quota_సమూహం_Deref(p_సమూహం);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  memset(&gQuota, 0, పరిమాణం(gQuota));
  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3_quota_సమితి(
  స్థిరరాశి అక్షరం *z_నమూనా,           /* The filename pattern */
  sqlite3_int64 i_హద్దు,           /* New quota to set for this quota group */
  శూన్య (*xCallback)(              /* Callback invoked when going over quota */
     స్థిరరాశి అక్షరం *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_హద్దు,        /* IN/OUT: The current limit */
     sqlite3_int64 i_కొలత,           /* Total size of all files in the group */
     శూన్య *pArg                     /* Client data */
  ),
  శూన్య *pArg,                     /* client data passed thru to callback */
  శూన్య (*xDestroy)(శూన్య*)         /* Optional destructor for pArg */
){
  quota_సమూహం *p_సమూహం;
  quotaEnter();
  p_సమూహం = gQuota.p_సమూహం;
  అయితే( p_సమూహం && strcmp(p_సమూహం->z_నమూనా, z_నమూనా)!=0 ){
    p_సమూహం = p_సమూహం->p_తదుపరి;
  }
  ఒకవేళ( p_సమూహం==0 ){
    పూర్ణాంకం n_నమూనా = (పూర్ణాంకం)(అక్షరశ్రేణి_పొడవు(z_నమూనా) & 0x3fffffff);
    ఒకవేళ( i_హద్దు<=0 ){
      quotaLeave();
      తిరిగి SQLITE_OK;
    }
    p_సమూహం = (quota_సమూహం *)sqlite3_malloc( పరిమాణం(*p_సమూహం) + n_నమూనా + 1 );
    ఒకవేళ( p_సమూహం==0 ){
      quotaLeave();
      తిరిగి SQLITE_NOMEM;
    }
    memset(p_సమూహం, 0, పరిమాణం(*p_సమూహం));
    p_సమూహం->z_నమూనా = (అక్షరం*)&p_సమూహం[1];
    జ్ఞాపకం_నకలు((అక్షరం *)p_సమూహం->z_నమూనా, z_నమూనా, n_నమూనా+1);
    ఒకవేళ( gQuota.p_సమూహం ) gQuota.p_సమూహం->ppPrev = &p_సమూహం->p_తదుపరి;
    p_సమూహం->p_తదుపరి = gQuota.p_సమూహం;
    p_సమూహం->ppPrev = &gQuota.p_సమూహం;
    gQuota.p_సమూహం = p_సమూహం;
  }
  p_సమూహం->i_హద్దు = i_హద్దు;
  p_సమూహం->xCallback = xCallback;
  ఒకవేళ( p_సమూహం->xDestroy && p_సమూహం->pArg!=pArg ){
    p_సమూహం->xDestroy(p_సమూహం->pArg);
  }
  p_సమూహం->pArg = pArg;
  p_సమూహం->xDestroy = xDestroy;
  quota_సమూహం_Deref(p_సమూహం);
  quotaLeave();
  తిరిగి SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
పూర్ణాంకం sqlite3_quota_file(స్థిరరాశి అక్షరం *zFilename){
  అక్షరం *z_నిండిన = 0;
  sqlite3_file *fd;
  పూర్ణాంకం rc;
  పూర్ణాంకం బహిర్గతం_Flags = 0;
  sqlite3_int64 i_కొలత;
  పూర్ణాంకం nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  ఒకవేళ( fd==0 ){
    rc = SQLITE_NOMEM;
  }లేకపోతే{
    z_నిండిన = &((అక్షరం *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->x_నిండిన_Pathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, z_నిండిన);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    z_నిండిన[అక్షరశ్రేణి_పొడవు(z_నిండిన)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, z_నిండిన, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &బహిర్గతం_Flags);
    ఒకవేళ( rc==SQLITE_OK ){
      fd->pMethods->x_File_కొలత(fd, &i_కొలత);
      fd->pMethods->xClose(fd);
    }లేకపోతే ఒకవేళ( rc==SQLITE_CANTOPEN ){
      quota_సమూహం *p_సమూహం;
      quotaFile *pFile;
      quotaEnter();
      p_సమూహం = quota_సమూహం_కనుగొను(z_నిండిన);
      ఒకవేళ( p_సమూహం ){
        pFile = quota_కనుగొను_File(p_సమూహం, z_నిండిన, 0);
        ఒకవేళ( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  తిరిగి rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(స్థిరరాశి అక్షరం *zFilename, స్థిరరాశి అక్షరం *z_బహుళకం){
  quota_FILE *p = 0;
  అక్షరం *z_నిండిన = 0;
  అక్షరం *z_నిండిన_Translated = 0;
  పూర్ణాంకం rc;
  quota_సమూహం *p_సమూహం;
  quotaFile *pFile;

  z_నిండిన = (అక్షరం*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ఒకవేళ( z_నిండిన==0 ) తిరిగి 0;
  rc = gQuota.pOrigVfs->x_నిండిన_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_నిండిన);
  ఒకవేళ( rc ) వెళ్ళు quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(పరిమాణం(*p));
  ఒకవేళ( p==0 ) వెళ్ళు quota_fopen_error;
  memset(p, 0, పరిమాణం(*p));
  z_నిండిన_Translated = quota_utf8_గమ్యం_mbcs(z_నిండిన);
  ఒకవేళ( z_నిండిన_Translated==0 ) వెళ్ళు quota_fopen_error;
  p->f = fopen(z_నిండిన_Translated, z_బహుళకం);
  ఒకవేళ( p->f==0 ) వెళ్ళు quota_fopen_error;
  quotaEnter();
  p_సమూహం = quota_సమూహం_కనుగొను(z_నిండిన);
  ఒకవేళ( p_సమూహం ){
    pFile = quota_కనుగొను_File(p_సమూహం, z_నిండిన, 1);
    ఒకవేళ( pFile==0 ){
      quotaLeave();
      వెళ్ళు quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(z_నిండిన);
#నిర్వచితమైతే _WIN32
  p->zMbcsName = z_నిండిన_Translated;
#అయితే_ముగింపు
  తిరిగి p;

quota_fopen_error:
  quota_mbcs_free(z_నిండిన_Translated);
  sqlite3_free(z_నిండిన);
  ఒకవేళ( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  తిరిగి 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  శూన్య *pBuf,            /* Store the content here */
  size_t కొలత,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  తిరిగి fread(pBuf, కొలత, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  స్థిరరాశి శూన్య *pBuf,      /* Take content to write from here */
  size_t కొలత,           /* Size of each element */
  size_t nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 i_చివర;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  size_t rc;

  iOfst = ftell(p->f);
  i_చివర = iOfst + కొలత*nmemb;
  pFile = p->pFile;
  ఒకవేళ( pFile && pFile->i_కొలత<i_చివర ){
    quota_సమూహం *p_సమూహం = pFile->p_సమూహం;
    quotaEnter();
    szNew = p_సమూహం->i_కొలత - pFile->i_కొలత + i_చివర;
    ఒకవేళ( szNew>p_సమూహం->i_హద్దు && p_సమూహం->i_హద్దు>0 ){
      ఒకవేళ( p_సమూహం->xCallback ){
        p_సమూహం->xCallback(pFile->zFilename, &p_సమూహం->i_హద్దు, szNew,
                          p_సమూహం->pArg);
      }
      ఒకవేళ( szNew>p_సమూహం->i_హద్దు && p_సమూహం->i_హద్దు>0 ){
        i_చివర = p_సమూహం->i_హద్దు - p_సమూహం->i_కొలత + pFile->i_కొలత;
        nmemb = (size_t)((i_చివర - iOfst)/కొలత);
        i_చివర = iOfst + కొలత*nmemb;
        szNew = p_సమూహం->i_కొలత - pFile->i_కొలత + i_చివర;
      }
    }
    p_సమూహం->i_కొలత = szNew;
    pFile->i_కొలత = i_చివర;
    quotaLeave();
  }లేకపోతే{
    pFile = 0;
  }
  rc = fwrite(pBuf, కొలత, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  ఒకవేళ( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 i_New_చివర = iOfst + కొలత*nWritten;
    ఒకవేళ( i_New_చివర<i_చివర ) i_New_చివర = i_చివర;
    quotaEnter();
    pFile->p_సమూహం->i_కొలత += i_New_చివర - pFile->i_కొలత;
    pFile->i_కొలత = i_New_చివర;
    quotaLeave();
  }
  తిరిగి rc;
}

/*
** Close an open quota_FILE stream.
*/
పూర్ణాంకం sqlite3_quota_fclose(quota_FILE *p){
  పూర్ణాంకం rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  ఒకవేళ( pFile ){
    quotaEnter();
    pFile->nRef--;
    ఒకవేళ( pFile->nRef==0 ){
      quota_సమూహం *p_సమూహం = pFile->p_సమూహం;
      ఒకవేళ( pFile->deleteOnClose ){
        gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
        quotaRemoveFile(pFile);
      }
      quota_సమూహం_Deref(p_సమూహం);
    }
    quotaLeave();
  }
#నిర్వచితమైతే _WIN32
  quota_mbcs_free(p->zMbcsName);
#అయితే_ముగింపు
  sqlite3_free(p);
  తిరిగి rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
పూర్ణాంకం sqlite3_quota_fflush(quota_FILE *p, పూర్ణాంకం doFsync){
  పూర్ణాంకం rc;
  rc = fflush(p->f);
  ఒకవేళ( rc==0 && doFsync ){
#నిర్వచితమైతే _WIN32
    rc = _commit(_fileno(p->f));
#లేకపోతే
    rc = fsync(fileno(p->f));
#అయితే_ముగింపు
  }
  తిరిగి rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
పూర్ణాంకం sqlite3_quota_fseek(quota_FILE *p, దీర్ఘ స్థానభ్రంశం, పూర్ణాంకం whence){
  తిరిగి fseek(p->f, స్థానభ్రంశం, whence);
}

/*
** rewind a quota_FILE stream.
*/
శూన్య sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
దీర్ఘ sqlite3_quota_ftell(quota_FILE *p){
  తిరిగి ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
పూర్ణాంకం sqlite3_quota_ferror(quota_FILE *p){
  తిరిగి ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
పూర్ణాంకం sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  పూర్ణాంకం rc;
  ఒకవేళ( (pFile = p->pFile)!=0 && pFile->i_కొలత<szNew ){
    quota_సమూహం *p_సమూహం;
    ఒకవేళ( pFile->i_కొలత<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      తిరిగి -1;
    }
    p_సమూహం = pFile->p_సమూహం;
    quotaEnter();
    p_సమూహం->i_కొలత += szNew - pFile->i_కొలత;
    quotaLeave();
  }
#నిర్వచితమైతే _WIN32
#  ఒకవేళ defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (దీర్ఘ)szNew);
#  లేకపోతే
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#లేకపోతే
  rc = ftruncate(fileno(p->f), szNew);
#అయితే_ముగింపు
  ఒకవేళ( pFile && rc==0 ){
    quota_సమూహం *p_సమూహం = pFile->p_సమూహం;
    quotaEnter();
    p_సమూహం->i_కొలత += szNew - pFile->i_కొలత;
    pFile->i_కొలత = szNew;
    quotaLeave();
  }
  తిరిగి rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
పూర్ణాంకం sqlite3_quota_file_mtime(quota_FILE *p, సమయం_t *p_సమయం){
  పూర్ణాంకం rc;
#నిర్వచితమైతే _WIN32
  నిర్మాణం _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#లేకపోతే
  నిర్మాణం stat buf;
  rc = fstat(fileno(p->f), &buf);
#అయితే_ముగింపు
  ఒకవేళ( rc==0 ) *p_సమయం = buf.st_mtime;
  తిరిగి rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  పూర్ణాంకం rc;
#నిర్వచితమైతే _WIN32
  నిర్మాణం _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#లేకపోతే
  నిర్మాణం stat buf;
  rc = fstat(fileno(p->f), &buf);
#అయితే_ముగింపు
  తిరిగి rc==0 ? buf.st_కొలత : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_కొలత(quota_FILE *p){
  తిరిగి p->pFile ? p->pFile->i_కొలత : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
దీర్ఘ sqlite3_quota_file_అందుబాటు(quota_FILE *p){
  FILE* f = p->f;
  దీర్ఘ pos1, pos2;
  పూర్ణాంకం rc;
  pos1 = ftell(f);
  ఒకవేళ ( pos1 < 0 ) తిరిగి -1;
  rc = fseek(f, 0, SEEK_END);
  ఒకవేళ ( rc != 0 ) తిరిగి -1;
  pos2 = ftell(f);
  ఒకవేళ ( pos2 < 0 ) తిరిగి -1;
  rc = fseek(f, pos1, SEEK_SET);
  ఒకవేళ ( rc != 0 ) తిరిగి -1;
  తిరిగి pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
పూర్ణాంకం sqlite3_quota_remove(స్థిరరాశి అక్షరం *zFilename){
  అక్షరం *z_నిండిన;            /* Full pathname for zFilename */
  size_t n_నిండిన;           /* Number of bytes in zFilename */
  పూర్ణాంకం rc;                 /* Result code */
  quota_సమూహం *p_సమూహం;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *p_తదుపరి_File;   /* next file in the group */
  పూర్ణాంకం diff;               /* Difference between filenames */
  అక్షరం c;                 /* First character past end of pattern */

  z_నిండిన = (అక్షరం*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  ఒకవేళ( z_నిండిన==0 ) తిరిగి SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->x_నిండిన_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_నిండిన);
  ఒకవేళ( rc ){
    sqlite3_free(z_నిండిన);
    తిరిగి rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  n_నిండిన = అక్షరశ్రేణి_పొడవు(z_నిండిన);
  ఒకవేళ( n_నిండిన>0 && (z_నిండిన[n_నిండిన-1]=='/' || z_నిండిన[n_నిండిన-1]=='\\') ){
    n_నిండిన--;
    z_నిండిన[n_నిండిన] = 0;
  }

  quotaEnter();
  p_సమూహం = quota_సమూహం_కనుగొను(z_నిండిన);
  ఒకవేళ( p_సమూహం ){
    కొరకు(pFile=p_సమూహం->pFiles; pFile && rc==SQLITE_OK; pFile=p_తదుపరి_File){
      p_తదుపరి_File = pFile->p_తదుపరి;
      diff = అక్షరశ్రేణి_పోలిక(z_నిండిన, pFile->zFilename, n_నిండిన);
      ఒకవేళ( diff==0 && ((c = pFile->zFilename[n_నిండిన])==0 || c=='/' || c=='\\') ){
        ఒకవేళ( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }లేకపోతే{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quota_సమూహం_Deref(p_సమూహం);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(z_నిండిన);
  తిరిగి rc;
}

/***************************** Test Code ***********************************/
#నిర్వచితమైతే SQLITE_TEST
#అన్తర్భావ "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
రకనిర్వచనం నిర్మాణం TclQuotaCallback TclQuotaCallback;
నిర్మాణం TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);


/*
** This is the callback from a quota-over-limit.
*/
స్థిర శూన్య tclQuotaCallback(
  స్థిరరాశి అక్షరం *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *pi_హద్దు,         /* IN/OUT: The current limit */
  sqlite3_int64 i_కొలత,            /* Total size of all files in the group */
  శూన్య *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  గుర్తులేని పూర్ణాంకం rnd;               /* Random part of pVarname */
  పూర్ణాంకం rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  ఒకవేళ( p==0 ) తిరిగి;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_లెక్క(pVarname);
  sqlite3_randomness(పరిమాణం(rnd), (శూన్య *)&rnd);
  Tcl_Append_Obj_గమ్యం_Obj(pVarname, Tcl_NewIntObj((పూర్ణాంకం)(rnd&0x7FFFFFFF)));
  Tcl_Obj_సమితి_Var2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*pi_హద్దు), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_లెక్క(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(i_కొలత));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  ఒకవేళ( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *p_హద్దు = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, p_హద్దు, &x);
    *pi_హద్దు = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_లెక్క(pEval);
  Tcl_Decr_Ref_లెక్క(pVarname);
  ఒకవేళ( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
స్థిర శూన్య tclCallbackDestructor(శూన్య *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  ఒకవేళ( p ){
    Tcl_Decr_Ref_లెక్క(p->pScript);
    sqlite3_free((అక్షరం *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_initialize(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *zName;              /* Name of new quota VFS */
  పూర్ణాంకం makeDefault;                /* True to make the new VFS the default */
  పూర్ణాంకం rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "NAME MAKEDEFAULT");
    తిరిగి TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) తిరిగి TCL_ERROR;
  ఒకవేళ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);

  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_shutdown(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;                         /* Value returned by quota_shutdown() */

  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);

  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_సమితి(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *z_నమూనా;           /* File pattern to configure */
  Tcl_WideInt i_హద్దు;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  పూర్ణాంకం rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_కొలత nScript;               /* Length of callback script */
  శూన్య (*xDestroy)(శూన్య*);        /* Optional destructor for pArg */
  శూన్య (*xCallback)(స్థిరరాశి అక్షరం *, sqlite3_int64 *, sqlite3_int64, శూన్య *);

  /* Process arguments */
  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    తిరిగి TCL_ERROR;
  }
  z_నమూనా = Tcl_GetString(objv[1]);
  ఒకవేళ( Tcl_GetWideIntFromObj(interp, objv[2], &i_హద్దు) ) తిరిగి TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  ఒకవేళ( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(పరిమాణం(TclQuotaCallback));
    ఒకవేళ( !p ){
      Tcl_సమితి_ఫలితం(interp, (అక్షరం *)"SQLITE_NOMEM", TCL_STATIC);
      తిరిగి TCL_OK;
    }
    memset(p, 0, పరిమాణం(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_లెక్క(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }లేకపోతే{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_సమితి(z_నమూనా, i_హద్దు, xCallback, (శూన్య*)p, xDestroy);

  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_file(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *zFilename;          /* File pattern to configure */
  పూర్ణాంకం rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "FILENAME");
    తిరిగి TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);
  తిరిగి TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_dump(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *p_ఫలితం;
  Tcl_Obj *p_సమూహం_పదం;
  Tcl_Obj *p_File_పదం;
  quota_సమూహం *p_సమూహం;
  quotaFile *pFile;

  p_ఫలితం = Tcl_NewObj();
  quotaEnter();
  కొరకు(p_సమూహం=gQuota.p_సమూహం; p_సమూహం; p_సమూహం=p_సమూహం->p_తదుపరి){
    p_సమూహం_పదం = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, p_సమూహం_పదం,
          Tcl_NewStringObj(p_సమూహం->z_నమూనా, -1));
    Tcl_ListObjAppendElement(interp, p_సమూహం_పదం,
          Tcl_NewWideIntObj(p_సమూహం->i_హద్దు));
    Tcl_ListObjAppendElement(interp, p_సమూహం_పదం,
          Tcl_NewWideIntObj(p_సమూహం->i_కొలత));
    కొరకు(pFile=p_సమూహం->pFiles; pFile; pFile=pFile->p_తదుపరి){
      పూర్ణాంకం i;
      అక్షరం z_తాత్కాలిక[1000];
      p_File_పదం = Tcl_NewObj();
      sqlite3_snprintf(పరిమాణం(z_తాత్కాలిక), z_తాత్కాలిక, "%s", pFile->zFilename);
      కొరకు(i=0; z_తాత్కాలిక[i]; i++){ ఒకవేళ( z_తాత్కాలిక[i]=='\\' ) z_తాత్కాలిక[i] = '/'; }
      Tcl_ListObjAppendElement(interp, p_File_పదం,
            Tcl_NewStringObj(z_తాత్కాలిక, -1));
      Tcl_ListObjAppendElement(interp, p_File_పదం,
            Tcl_NewWideIntObj(pFile->i_కొలత));
      Tcl_ListObjAppendElement(interp, p_File_పదం,
            Tcl_NewWideIntObj(pFile->nRef));
      Tcl_ListObjAppendElement(interp, p_File_పదం,
            Tcl_NewWideIntObj(pFile->deleteOnClose));
      Tcl_ListObjAppendElement(interp, p_సమూహం_పదం, p_File_పదం);
    }
    Tcl_ListObjAppendElement(interp, p_ఫలితం, p_సమూహం_పదం);
  }
  quotaLeave();
  Tcl_సమితి_Obj_ఫలితం(interp, p_ఫలితం);
  తిరిగి TCL_OK;
}

/* Defined in test1.c */
బాహ్య శూన్య *sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(స్థిరరాశి అక్షరం*);
బాహ్య స్థిరరాశి అక్షరం *sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(శూన్య*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_fopen(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *zFilename;          /* File pattern to configure */
  స్థిరరాశి అక్షరం *z_బహుళకం;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "FILENAME MODE");
    తిరిగి TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  z_బహుళకం = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, z_బహుళకం);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం*)sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(p), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_fread(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  అక్షరం *zBuf;
  పూర్ణాంకం sz;
  పూర్ణాంకం nElem;
  size_t got;

  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE SIZE NELEM");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) తిరిగి TCL_ERROR;
  zBuf = (అక్షరం*)sqlite3_malloc( sz*nElem + 1 );
  ఒకవేళ( zBuf==0 ){
    Tcl_సమితి_ఫలితం(interp, "out of memory", TCL_STATIC);
    తిరిగి TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_సమితి_ఫలితం(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_fwrite(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  అక్షరం *zBuf;
  పూర్ణాంకం sz;
  పూర్ణాంకం nElem;
  size_t got;

  ఒకవేళ( objc!=5 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &sz) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) తిరిగి TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(got));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_fclose(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_fflush(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  పూర్ణాంకం rc;
  పూర్ణాంకం doSync = 0;

  ఒకవేళ( objc!=2 && objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE ?HARDSYNC?");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  ఒకవేళ( objc==3 ){
    ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) తిరిగి TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_fseek(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  పూర్ణాంకం ofst;
  స్థిరరాశి అక్షరం *zWhence;
  పూర్ణాంకం whence;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE OFFSET WHENCE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) తిరిగి TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  ఒకవేళ( strcmp(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }లేకపోతే ఒకవేళ( strcmp(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }లేకపోతే ఒకవేళ( strcmp(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }లేకపోతే{
    Tcl_Append_ఫలితం(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_rewind(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_ftell(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(x));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_ftruncate(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  పూర్ణాంకం rc;
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE SIZE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  ఒకవేళ( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) తిరిగి TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_file_కొలత(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_కొలత(p);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(x));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_file_truesize(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(x));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_file_mtime(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  సమయం_t t;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(t));
  తిరిగి TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_remove(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *zFilename;          /* File pattern to configure */
  పూర్ణాంకం rc;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "FILENAME");
    తిరిగి TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_glob(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *z_నమూనా;          /* The glob pattern */
  స్థిరరాశి అక్షరం *z_పాఠ్యం;             /* Text to compare against the pattern */
  పూర్ణాంకం rc;
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "PATTERN TEXT");
    తిరిగి TCL_ERROR;
  }
  z_నమూనా = Tcl_GetString(objv[1]);
  z_పాఠ్యం = Tcl_GetString(objv[2]);
  rc = quotaStrglob(z_నమూనా, z_పాఠ్యం);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_file_అందుబాటు(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_అందుబాటు(p);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(x));
  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_quota_ferror(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  పూర్ణాంకం x;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE");
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(x));
  తిరిగి TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
పూర్ణాంకం Sqlitequota_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_quota_initialize",    test_quota_initialize },
    { "sqlite3_quota_shutdown",      test_quota_shutdown },
    { "sqlite3_quota_set",           test_quota_సమితి },
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
    { "sqlite3_quota_file_size",     test_quota_file_కొలత },
    { "sqlite3_quota_file_truesize", test_quota_file_truesize },
    { "sqlite3_quota_file_mtime",    test_quota_file_mtime },
    { "sqlite3_quota_remove",        test_quota_remove },
    { "sqlite3_quota_glob",          test_quota_glob },
    { "sqlite3_quota_file_available",test_quota_file_అందుబాటు },
    { "sqlite3_quota_ferror",        test_quota_ferror },
  };
  పూర్ణాంకం i;

  కొరకు(i=0; i<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  తిరిగి TCL_OK;
}
#అయితే_ముగింపు
