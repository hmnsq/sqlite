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
#परिभाषय sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#परिभाषय sqlite3_mutex_free(X)
#परिभाषय sqlite3_mutex_enter(X)
#परिभाषय sqlite3_mutex_try(X)      SQLITE_OK
#परिभाषय sqlite3_mutex_leave(X)
#परिभाषय sqlite3_mutex_held(X)     ((शून्य)(X),1)
#परिभाषय sqlite3_mutex_notheld(X)  ((शून्य)(X),1)
#यदि_अन्तः /* SQLITE_THREADSAFE==0 */


#यदि_परिभाषितम् _WIN32
# include <windows.h>
# include <io.h>
#अन्यथा
# include <unistd.h>
#यदि_अन्तः


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
प्रकारनिर्देश संरचना quota_गणः quota_गणः;
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
संरचना quota_गणः {
  ध्रुव वर्ण *z_आदर्शः;          /* Filename pattern to be quotaed */
  sqlite3_int64 i_सीमा;          /* Upper bound on total file size */
  sqlite3_int64 i_आकार;           /* Current size of all files */
  शून्य (*xCallback)(             /* Callback invoked when going over quota */
     ध्रुव वर्ण *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_सीमा,        /* IN/OUT: The current limit */
     sqlite3_int64 i_आकार,           /* Total size of all files in the group */
     शून्य *pArg                     /* Client data */
  );
  शून्य *pArg;                    /* Third argument to the xCallback() */
  शून्य (*xDestroy)(शून्य*);       /* Optional destructor for pArg */
  quota_गणः *p_अग्र, **pp_पूर्व;   /* Doubly linked list of all quota objects */
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
  quota_गणः *p_गणः;             /* Quota group to which this file belongs */
  sqlite3_int64 i_आकार;            /* Current size of this file */
  पूर्णाङ्क nRef;                       /* Number of times this file is open */
  पूर्णाङ्क deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *p_अग्र, **pp_पूर्व;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
संरचना quotaConn {
  sqlite3_file आधारः;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
संरचना quota_FILE {
  FILE *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#यदि_परिभाषितम् _WIN32
  वर्ण *zMbcsName;        /* Full MBCS pathname of the file */
#यदि_अन्तः
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
  quota_गणः *p_गणः;

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
स्थिर पूर्णाङ्क quota_गणः_Open_File_गणनम्(quota_गणः *p_गणः){
  पूर्णाङ्क N = 0;
  quotaFile *pFile = p_गणः->pFiles;
  यावत्( pFile ){
    यदि( pFile->nRef ) N++;
    pFile = pFile->p_अग्र;
  }
  प्रत्यागम N;
}

/* Remove a file from a quota group.
*/
स्थिर शून्य quotaRemoveFile(quotaFile *pFile){
  quota_गणः *p_गणः = pFile->p_गणः;
  p_गणः->i_आकार -= pFile->i_आकार;
  *pFile->pp_पूर्व = pFile->p_अग्र;
  यदि( pFile->p_अग्र ) pFile->p_अग्र->pp_पूर्व = pFile->pp_पूर्व;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
स्थिर शून्य quotaRemoveAllFiles(quota_गणः *p_गणः){
  यावत्( p_गणः->pFiles ){
    प्रतिज्ञा( p_गणः->pFiles->nRef==0 );
    quotaRemoveFile(p_गणः->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
स्थिर शून्य quota_गणः_Deref(quota_गणः *p_गणः){
  यदि( p_गणः->i_सीमा==0 && quota_गणः_Open_File_गणनम्(p_गणः)==0 ){
    quotaRemoveAllFiles(p_गणः);
    *p_गणः->pp_पूर्व = p_गणः->p_अग्र;
    यदि( p_गणः->p_अग्र ) p_गणः->p_अग्र->pp_पूर्व = p_गणः->pp_पूर्व;
    यदि( p_गणः->xDestroy ) p_गणः->xDestroy(p_गणः->pArg);
    sqlite3_free(p_गणः);
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
स्थिर पूर्णाङ्क quotaStrglob(ध्रुव वर्ण *zGlob, ध्रुव वर्ण *z){
  पूर्णाङ्क c, c2, cx;
  पूर्णाङ्क invert;
  पूर्णाङ्क आलोकितम्;

  यावत्( (c = (*(zGlob++)))!=0 ){
    यदि( c=='*' ){
      यावत्( (c=(*(zGlob++))) == '*' || c=='?' ){
        यदि( c=='?' && (*(z++))==0 ) प्रत्यागम 0;
      }
      यदि( c==0 ){
        प्रत्यागम 1;
      }अन्यथा यदि( c=='[' ){
        यावत्( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        प्रत्यागम (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      यावत्( (c2 = (*(z++)))!=0 ){
        यावत्( c2!=c && c2!=cx ){
          c2 = *(z++);
          यदि( c2==0 ) प्रत्यागम 0;
        }
        यदि( quotaStrglob(zGlob,z) ) प्रत्यागम 1;
      }
      प्रत्यागम 0;
    }अन्यथा यदि( c=='?' ){
      यदि( (*(z++))==0 ) प्रत्यागम 0;
    }अन्यथा यदि( c=='[' ){
      पूर्णाङ्क प्राक्_c = 0;
      आलोकितम् = 0;
      invert = 0;
      c = *(z++);
      यदि( c==0 ) प्रत्यागम 0;
      c2 = *(zGlob++);
      यदि( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      यदि( c2==']' ){
        यदि( c==']' ) आलोकितम् = 1;
        c2 = *(zGlob++);
      }
      यावत्( c2 && c2!=']' ){
        यदि( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && प्राक्_c>0 ){
          c2 = *(zGlob++);
          यदि( c>=प्राक्_c && c<=c2 ) आलोकितम् = 1;
          प्राक्_c = 0;
        }अन्यथा{
          यदि( c==c2 ){
            आलोकितम् = 1;
          }
          प्राक्_c = c2;
        }
        c2 = *(zGlob++);
      }
      यदि( c2==0 || (आलोकितम् ^ invert)==0 ) प्रत्यागम 0;
    }अन्यथा यदि( c=='/' ){
      यदि( z[0]!='/' && z[0]!='\\' ) प्रत्यागम 0;
      z++;
    }अन्यथा{
      यदि( c!=(*(z++)) ) प्रत्यागम 0;
    }
  }
  प्रत्यागम *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
स्थिर quota_गणः *quota_गणः_Find(ध्रुव वर्ण *zFilename){
  quota_गणः *p;
  प्रति(p=gQuota.p_गणः; p && quotaStrglob(p->z_आदर्शः, zFilename)==0;
      p=p->p_अग्र){}
  प्रत्यागम p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
स्थिर sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  प्रत्यागम (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
स्थिर quotaFile *quotaFindFile(
  quota_गणः *p_गणः,     /* Group in which to look for the file */
  ध्रुव वर्ण *zName,      /* Full pathname of the file */
  पूर्णाङ्क createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = p_गणः->pFiles;
  यावत्( pFile && वाक्यं_तुलय(pFile->zFilename, zName)!=0 ){
    pFile = pFile->p_अग्र;
  }
  यदि( pFile==0 && createFlag ){
    पूर्णाङ्क nName = (पूर्णाङ्क)(वाक्यदैर्घ्यम्(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( परिमाण(*pFile) + nName + 1 );
    यदि( pFile ){
      स्मृतिं_पूरय(pFile, 0, परिमाण(*pFile));
      pFile->zFilename = (वर्ण*)&pFile[1];
      स्मृतिं_प्रतिलिख(pFile->zFilename, zName, nName+1);
      pFile->p_अग्र = p_गणः->pFiles;
      यदि( p_गणः->pFiles ) p_गणः->pFiles->pp_पूर्व = &pFile->p_अग्र;
      pFile->pp_पूर्व = &p_गणः->pFiles;
      p_गणः->pFiles = pFile;
      pFile->p_गणः = p_गणः;
    }
  }
  प्रत्यागम pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
स्थिर वर्ण *quota_utf8_गन्तव्यम्_mbcs(ध्रुव वर्ण *zUtf8){
#यदि_परिभाषितम् _WIN32
  size_t n;          /* Bytes in zUtf8 */
  पूर्णाङ्क nWide;         /* number of UTF-16 characters */
  पूर्णाङ्क nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  वर्ण *zMbcs;       /* The MBCS text */
  पूर्णाङ्क codepage;      /* Code page used by fopen() */

  n = वाक्यदैर्घ्यम्(zUtf8);
  nWide = Multi_Byte_गन्तव्यम्_Wide_Char(CP_UTF8, 0, zUtf8, -1, रिक्त, 0);
  यदि( nWide==0 ) प्रत्यागम 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*परिमाण(zTmpWide[0]) );
  यदि( zTmpWide==0 ) प्रत्यागम 0;
  Multi_Byte_गन्तव्यम्_Wide_Char(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = Wide_Char_गन्तव्यम्_Multi_Byte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (वर्ण*)sqlite3_malloc( nMbcs+1 ) : 0;
  यदि( zMbcs ){
    Wide_Char_गन्तव्यम्_Multi_Byte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  प्रत्यागम zMbcs;
#अन्यथा
  प्रत्यागम (वर्ण*)zUtf8;  /* No-op on unix */
#यदि_अन्तः
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
स्थिर शून्य quota_mbcs_free(वर्ण *zOld){
#यदि_परिभाषितम् _WIN32
  sqlite3_free(zOld);
#अन्यथा
  /* No-op on unix */
#यदि_अन्तः
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
  ध्रुव वर्ण *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  पूर्णाङ्क flags,                  /* Flags to control the opening */
  पूर्णाङ्क *p_बहिर्गमनम्_Flags              /* Flags showing results of opening */
){
  पूर्णाङ्क rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quota_गणः *p_गणः;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  यदि( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    प्रत्यागम pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_बहिर्गमनम्_Flags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  p_गणः = quota_गणः_Find(zName);
  यदि( p_गणः==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, p_बहिर्गमनम्_Flags);
  }अन्यथा{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, p_बहिर्गमनम्_Flags);
    यदि( rc==SQLITE_OK ){
      pFile = quotaFindFile(p_गणः, zName, 1);
      यदि( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        प्रत्यागम SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      यदि( pSubOpen->pMethods->i_संस्करणम्==1 ){
        pQuotaOpen->आधारः.pMethods = &gQuota.sIoMethodsV1;
      }अन्यथा{
        pQuotaOpen->आधारः.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  प्रत्यागम rc;
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
  ध्रुव वर्ण *zName,          /* Name of file to be deleted */
  पूर्णाङ्क syncDir                 /* Do a directory sync after deleting */
){
  पूर्णाङ्क rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quota_गणः *p_गणः;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  यदि( rc==SQLITE_OK ){
    quotaEnter();
    p_गणः = quota_गणः_Find(zName);
    यदि( p_गणः ){
      pFile = quotaFindFile(p_गणः, zName, 0);
      यदि( pFile ){
        यदि( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }अन्यथा{
          quotaRemoveFile(pFile);
          quota_गणः_Deref(p_गणः);
        }
      }
    }
    quotaLeave();
  }
  प्रत्यागम rc;
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
    quota_गणः *p_गणः = pFile->p_गणः;
    यदि( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quota_गणः_Deref(p_गणः);
  }
  quotaLeave();
  प्रत्यागम rc;
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
  प्रत्यागम pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
स्थिर पूर्णाङ्क quotaWrite(
  sqlite3_file *pConn,
  ध्रुव शून्य *pBuf,
  पूर्णाङ्क iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 i_अन्तः = iOfst+iAmt;
  quota_गणः *p_गणः;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  यदि( pFile->i_आकार<i_अन्तः ){
    p_गणः = pFile->p_गणः;
    quotaEnter();
    szNew = p_गणः->i_आकार - pFile->i_आकार + i_अन्तः;
    यदि( szNew>p_गणः->i_सीमा && p_गणः->i_सीमा>0 ){
      यदि( p_गणः->xCallback ){
        p_गणः->xCallback(pFile->zFilename, &p_गणः->i_सीमा, szNew,
                          p_गणः->pArg);
      }
      यदि( szNew>p_गणः->i_सीमा && p_गणः->i_सीमा>0 ){
        quotaLeave();
        प्रत्यागम SQLITE_पूर्णम्;
      }
    }
    p_गणः->i_आकार = szNew;
    pFile->i_आकार = i_अन्तः;
    quotaLeave();
  }
  प्रत्यागम pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
स्थिर पूर्णाङ्क quotaTruncate(sqlite3_file *pConn, sqlite3_int64 आकार){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  पूर्णाङ्क rc = pSubOpen->pMethods->xTruncate(pSubOpen, आकार);
  quotaFile *pFile = p->pFile;
  quota_गणः *p_गणः;
  यदि( rc==SQLITE_OK ){
    quotaEnter();
    p_गणः = pFile->p_गणः;
    p_गणः->i_आकार -= pFile->i_आकार;
    pFile->i_आकार = आकार;
    p_गणः->i_आकार += आकार;
    quotaLeave();
  }
  प्रत्यागम rc;
}

/* Pass xSync requests through to the original VFS without change
*/
स्थिर पूर्णाङ्क quotaSync(sqlite3_file *pConn, पूर्णाङ्क flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  प्रत्यागम pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
स्थिर पूर्णाङ्क quota_File_आकार(sqlite3_file *pConn, sqlite3_int64 *p_आकार){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quota_गणः *p_गणः;
  sqlite3_int64 sz;
  पूर्णाङ्क rc;

  rc = pSubOpen->pMethods->x_File_आकार(pSubOpen, &sz);
  यदि( rc==SQLITE_OK ){
    quotaEnter();
    p_गणः = pFile->p_गणः;
    p_गणः->i_आकार -= pFile->i_आकार;
    pFile->i_आकार = sz;
    p_गणः->i_आकार += sz;
    quotaLeave();
    *p_आकार = sz;
  }
  प्रत्यागम rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quota_तालकम्(sqlite3_file *pConn, पूर्णाङ्क तालकम्){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  प्रत्यागम pSubOpen->pMethods->x_तालकम्(pSubOpen, तालकम्);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaUnlock(sqlite3_file *pConn, पूर्णाङ्क तालकम्){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  प्रत्यागम pSubOpen->pMethods->xUnlock(pSubOpen, तालकम्);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quota_Check_Reserved_तालकम्(sqlite3_file *pConn, पूर्णाङ्क *p_Res_बहिर्गमनम्){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  प्रत्यागम pSubOpen->pMethods->x_Check_Reserved_तालकम्(pSubOpen, p_Res_बहिर्गमनम्);
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
#यदि_अन्तः
  प्रत्यागम rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quota_Sector_आकार(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  प्रत्यागम pSubOpen->pMethods->x_Sector_आकार(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  प्रत्यागम pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
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
  प्रत्यागम pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क quota_Shm_तालकम्(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  पूर्णाङ्क ofst,                  /* First lock to acquire or release */
  पूर्णाङ्क n,                     /* Number of locks to acquire or release */
  पूर्णाङ्क flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  प्रत्यागम pSubOpen->pMethods->x_Shm_तालकम्(pSubOpen, ofst, n, flags);
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
  प्रत्यागम pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
पूर्णाङ्क sqlite3_quota_initialize(ध्रुव वर्ण *zOrigVfsName, पूर्णाङ्क makeDefault){
  sqlite3_vfs *pOrigVfs;
  यदि( gQuota.isInitialized ) प्रत्यागम SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  यदि( pOrigVfs==0 ) प्रत्यागम SQLITE_ERROR;
  प्रतिज्ञा( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  यदि( !gQuota.pMutex ){
    प्रत्यागम SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += परिमाण(quotaConn);
  gQuota.sThisVfs.zName = "quota";
  gQuota.sIoMethodsV1.i_संस्करणम् = 1;
  gQuota.sIoMethodsV1.xClose = quotaClose;
  gQuota.sIoMethodsV1.xRead = quotaRead;
  gQuota.sIoMethodsV1.xWrite = quotaWrite;
  gQuota.sIoMethodsV1.xTruncate = quotaTruncate;
  gQuota.sIoMethodsV1.xSync = quotaSync;
  gQuota.sIoMethodsV1.x_File_आकार = quota_File_आकार;
  gQuota.sIoMethodsV1.x_तालकम् = quota_तालकम्;
  gQuota.sIoMethodsV1.xUnlock = quotaUnlock;
  gQuota.sIoMethodsV1.x_Check_Reserved_तालकम् = quota_Check_Reserved_तालकम्;
  gQuota.sIoMethodsV1.xFileControl = quotaFileControl;
  gQuota.sIoMethodsV1.x_Sector_आकार = quota_Sector_आकार;
  gQuota.sIoMethodsV1.xDeviceCharacteristics = quotaDeviceCharacteristics;
  gQuota.sIoMethodsV2 = gQuota.sIoMethodsV1;
  gQuota.sIoMethodsV2.i_संस्करणम् = 2;
  gQuota.sIoMethodsV2.xShmMap = quotaShmMap;
  gQuota.sIoMethodsV2.x_Shm_तालकम् = quota_Shm_तालकम्;
  gQuota.sIoMethodsV2.xShmBarrier = quotaShmBarrier;
  gQuota.sIoMethodsV2.xShmUnmap = quotaShmUnmap;
  sqlite3_vfs_पञ्जिकाकोशः(&gQuota.sThisVfs, makeDefault);
  प्रत्यागम SQLITE_OK;
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
  quota_गणः *p_गणः;
  यदि( gQuota.isInitialized==0 ) प्रत्यागम SQLITE_MISUSE;
  प्रति(p_गणः=gQuota.p_गणः; p_गणः; p_गणः=p_गणः->p_अग्र){
    यदि( quota_गणः_Open_File_गणनम्(p_गणः)>0 ) प्रत्यागम SQLITE_MISUSE;
  }
  यावत्( gQuota.p_गणः ){
    p_गणः = gQuota.p_गणः;
    gQuota.p_गणः = p_गणः->p_अग्र;
    p_गणः->i_सीमा = 0;
    प्रतिज्ञा( quota_गणः_Open_File_गणनम्(p_गणः)==0 );
    quota_गणः_Deref(p_गणः);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  स्मृतिं_पूरय(&gQuota, 0, परिमाण(gQuota));
  प्रत्यागम SQLITE_OK;
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
पूर्णाङ्क sqlite3_quota_समुच्चयः(
  ध्रुव वर्ण *z_आदर्शः,           /* The filename pattern */
  sqlite3_int64 i_सीमा,           /* New quota to set for this quota group */
  शून्य (*xCallback)(              /* Callback invoked when going over quota */
     ध्रुव वर्ण *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *pi_सीमा,        /* IN/OUT: The current limit */
     sqlite3_int64 i_आकार,           /* Total size of all files in the group */
     शून्य *pArg                     /* Client data */
  ),
  शून्य *pArg,                     /* client data passed thru to callback */
  शून्य (*xDestroy)(शून्य*)         /* Optional destructor for pArg */
){
  quota_गणः *p_गणः;
  quotaEnter();
  p_गणः = gQuota.p_गणः;
  यावत्( p_गणः && वाक्यं_तुलय(p_गणः->z_आदर्शः, z_आदर्शः)!=0 ){
    p_गणः = p_गणः->p_अग्र;
  }
  यदि( p_गणः==0 ){
    पूर्णाङ्क n_आदर्शः = (पूर्णाङ्क)(वाक्यदैर्घ्यम्(z_आदर्शः) & 0x3fffffff);
    यदि( i_सीमा<=0 ){
      quotaLeave();
      प्रत्यागम SQLITE_OK;
    }
    p_गणः = (quota_गणः *)sqlite3_malloc( परिमाण(*p_गणः) + n_आदर्शः + 1 );
    यदि( p_गणः==0 ){
      quotaLeave();
      प्रत्यागम SQLITE_NOMEM;
    }
    स्मृतिं_पूरय(p_गणः, 0, परिमाण(*p_गणः));
    p_गणः->z_आदर्शः = (वर्ण*)&p_गणः[1];
    स्मृतिं_प्रतिलिख((वर्ण *)p_गणः->z_आदर्शः, z_आदर्शः, n_आदर्शः+1);
    यदि( gQuota.p_गणः ) gQuota.p_गणः->pp_पूर्व = &p_गणः->p_अग्र;
    p_गणः->p_अग्र = gQuota.p_गणः;
    p_गणः->pp_पूर्व = &gQuota.p_गणः;
    gQuota.p_गणः = p_गणः;
  }
  p_गणः->i_सीमा = i_सीमा;
  p_गणः->xCallback = xCallback;
  यदि( p_गणः->xDestroy && p_गणः->pArg!=pArg ){
    p_गणः->xDestroy(p_गणः->pArg);
  }
  p_गणः->pArg = pArg;
  p_गणः->xDestroy = xDestroy;
  quota_गणः_Deref(p_गणः);
  quotaLeave();
  प्रत्यागम SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
पूर्णाङ्क sqlite3_quota_file(ध्रुव वर्ण *zFilename){
  वर्ण *z_पूर्णम् = 0;
  sqlite3_file *fd;
  पूर्णाङ्क rc;
  पूर्णाङ्क बहिर्गमनम्_Flags = 0;
  sqlite3_int64 i_आकार;
  पूर्णाङ्क nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  यदि( fd==0 ){
    rc = SQLITE_NOMEM;
  }अन्यथा{
    z_पूर्णम् = &((वर्ण *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->x_पूर्णम्_Pathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, z_पूर्णम्);
  }

  यदि( rc==SQLITE_OK ){
    z_पूर्णम्[वाक्यदैर्घ्यम्(z_पूर्णम्)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, z_पूर्णम्, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &बहिर्गमनम्_Flags);
    यदि( rc==SQLITE_OK ){
      fd->pMethods->x_File_आकार(fd, &i_आकार);
      fd->pMethods->xClose(fd);
    }अन्यथा यदि( rc==SQLITE_CANTOPEN ){
      quota_गणः *p_गणः;
      quotaFile *pFile;
      quotaEnter();
      p_गणः = quota_गणः_Find(z_पूर्णम्);
      यदि( p_गणः ){
        pFile = quotaFindFile(p_गणः, z_पूर्णम्, 0);
        यदि( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  प्रत्यागम rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(ध्रुव वर्ण *zFilename, ध्रुव वर्ण *z_बहुलकम्){
  quota_FILE *p = 0;
  वर्ण *z_पूर्णम् = 0;
  वर्ण *z_पूर्णम्_Translated = 0;
  पूर्णाङ्क rc;
  quota_गणः *p_गणः;
  quotaFile *pFile;

  z_पूर्णम् = (वर्ण*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  यदि( z_पूर्णम्==0 ) प्रत्यागम 0;
  rc = gQuota.pOrigVfs->x_पूर्णम्_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_पूर्णम्);
  यदि( rc ) गच्छ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(परिमाण(*p));
  यदि( p==0 ) गच्छ quota_fopen_error;
  स्मृतिं_पूरय(p, 0, परिमाण(*p));
  z_पूर्णम्_Translated = quota_utf8_गन्तव्यम्_mbcs(z_पूर्णम्);
  यदि( z_पूर्णम्_Translated==0 ) गच्छ quota_fopen_error;
  p->f = fopen(z_पूर्णम्_Translated, z_बहुलकम्);
  यदि( p->f==0 ) गच्छ quota_fopen_error;
  quotaEnter();
  p_गणः = quota_गणः_Find(z_पूर्णम्);
  यदि( p_गणः ){
    pFile = quotaFindFile(p_गणः, z_पूर्णम्, 1);
    यदि( pFile==0 ){
      quotaLeave();
      गच्छ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(z_पूर्णम्);
#यदि_परिभाषितम् _WIN32
  p->zMbcsName = z_पूर्णम्_Translated;
#यदि_अन्तः
  प्रत्यागम p;

quota_fopen_error:
  quota_mbcs_free(z_पूर्णम्_Translated);
  sqlite3_free(z_पूर्णम्);
  यदि( p && p->f ) fclose(p->f);
  sqlite3_free(p);
  प्रत्यागम 0;
}

/*
** Read content from a quota_FILE
*/
size_t sqlite3_quota_fread(
  शून्य *pBuf,            /* Store the content here */
  size_t आकार,           /* Size of each element */
  size_t nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  प्रत्यागम fread(pBuf, आकार, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
size_t sqlite3_quota_fwrite(
  ध्रुव शून्य *pBuf,      /* Take content to write from here */
  size_t आकार,           /* Size of each element */
  size_t nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 i_अन्तः;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  size_t rc;

  iOfst = ftell(p->f);
  i_अन्तः = iOfst + आकार*nmemb;
  pFile = p->pFile;
  यदि( pFile && pFile->i_आकार<i_अन्तः ){
    quota_गणः *p_गणः = pFile->p_गणः;
    quotaEnter();
    szNew = p_गणः->i_आकार - pFile->i_आकार + i_अन्तः;
    यदि( szNew>p_गणः->i_सीमा && p_गणः->i_सीमा>0 ){
      यदि( p_गणः->xCallback ){
        p_गणः->xCallback(pFile->zFilename, &p_गणः->i_सीमा, szNew,
                          p_गणः->pArg);
      }
      यदि( szNew>p_गणः->i_सीमा && p_गणः->i_सीमा>0 ){
        i_अन्तः = p_गणः->i_सीमा - p_गणः->i_आकार + pFile->i_आकार;
        nmemb = (size_t)((i_अन्तः - iOfst)/आकार);
        i_अन्तः = iOfst + आकार*nmemb;
        szNew = p_गणः->i_आकार - pFile->i_आकार + i_अन्तः;
      }
    }
    p_गणः->i_आकार = szNew;
    pFile->i_आकार = i_अन्तः;
    quotaLeave();
  }अन्यथा{
    pFile = 0;
  }
  rc = fwrite(pBuf, आकार, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  यदि( rc<nmemb && pFile ){
    size_t nWritten = rc;
    sqlite3_int64 i_New_अन्तः = iOfst + आकार*nWritten;
    यदि( i_New_अन्तः<i_अन्तः ) i_New_अन्तः = i_अन्तः;
    quotaEnter();
    pFile->p_गणः->i_आकार += i_New_अन्तः - pFile->i_आकार;
    pFile->i_आकार = i_New_अन्तः;
    quotaLeave();
  }
  प्रत्यागम rc;
}

/*
** Close an open quota_FILE stream.
*/
पूर्णाङ्क sqlite3_quota_fclose(quota_FILE *p){
  पूर्णाङ्क rc;
  quotaFile *pFile;
  rc = fclose(p->f);
  pFile = p->pFile;
  यदि( pFile ){
    quotaEnter();
    pFile->nRef--;
    यदि( pFile->nRef==0 ){
      quota_गणः *p_गणः = pFile->p_गणः;
      यदि( pFile->deleteOnClose ){
        gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
        quotaRemoveFile(pFile);
      }
      quota_गणः_Deref(p_गणः);
    }
    quotaLeave();
  }
#यदि_परिभाषितम् _WIN32
  quota_mbcs_free(p->zMbcsName);
#यदि_अन्तः
  sqlite3_free(p);
  प्रत्यागम rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
पूर्णाङ्क sqlite3_quota_fflush(quota_FILE *p, पूर्णाङ्क doFsync){
  पूर्णाङ्क rc;
  rc = fflush(p->f);
  यदि( rc==0 && doFsync ){
#यदि_परिभाषितम् _WIN32
    rc = _commit(_fileno(p->f));
#अन्यथा
    rc = fsync(fileno(p->f));
#यदि_अन्तः
  }
  प्रत्यागम rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
पूर्णाङ्क sqlite3_quota_fseek(quota_FILE *p, दीर्घ विस्थापनम्, पूर्णाङ्क whence){
  प्रत्यागम fseek(p->f, विस्थापनम्, whence);
}

/*
** rewind a quota_FILE stream.
*/
शून्य sqlite3_quota_rewind(quota_FILE *p){
  rewind(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
दीर्घ sqlite3_quota_ftell(quota_FILE *p){
  प्रत्यागम ftell(p->f);
}

/*
** Test the error indicator for the given file.
*/
पूर्णाङ्क sqlite3_quota_ferror(quota_FILE *p){
  प्रत्यागम ferror(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
पूर्णाङ्क sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  पूर्णाङ्क rc;
  यदि( (pFile = p->pFile)!=0 && pFile->i_आकार<szNew ){
    quota_गणः *p_गणः;
    यदि( pFile->i_आकार<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      प्रत्यागम -1;
    }
    p_गणः = pFile->p_गणः;
    quotaEnter();
    p_गणः->i_आकार += szNew - pFile->i_आकार;
    quotaLeave();
  }
#यदि_परिभाषितम् _WIN32
#  यदि defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (दीर्घ)szNew);
#  अन्यथा
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#अन्यथा
  rc = ftruncate(fileno(p->f), szNew);
#यदि_अन्तः
  यदि( pFile && rc==0 ){
    quota_गणः *p_गणः = pFile->p_गणः;
    quotaEnter();
    p_गणः->i_आकार += szNew - pFile->i_आकार;
    pFile->i_आकार = szNew;
    quotaLeave();
  }
  प्रत्यागम rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
पूर्णाङ्क sqlite3_quota_file_mtime(quota_FILE *p, time_t *pTime){
  पूर्णाङ्क rc;
#यदि_परिभाषितम् _WIN32
  संरचना _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#अन्यथा
  संरचना stat buf;
  rc = fstat(fileno(p->f), &buf);
#यदि_अन्तः
  यदि( rc==0 ) *pTime = buf.st_mtime;
  प्रत्यागम rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  पूर्णाङ्क rc;
#यदि_परिभाषितम् _WIN32
  संरचना _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#अन्यथा
  संरचना stat buf;
  rc = fstat(fileno(p->f), &buf);
#यदि_अन्तः
  प्रत्यागम rc==0 ? buf.st_आकार : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_आकार(quota_FILE *p){
  प्रत्यागम p->pFile ? p->pFile->i_आकार : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
दीर्घ sqlite3_quota_file_उपलब्धम्(quota_FILE *p){
  FILE* f = p->f;
  दीर्घ pos1, pos2;
  पूर्णाङ्क rc;
  pos1 = ftell(f);
  यदि ( pos1 < 0 ) प्रत्यागम -1;
  rc = fseek(f, 0, SEEK_END);
  यदि ( rc != 0 ) प्रत्यागम -1;
  pos2 = ftell(f);
  यदि ( pos2 < 0 ) प्रत्यागम -1;
  rc = fseek(f, pos1, SEEK_SET);
  यदि ( rc != 0 ) प्रत्यागम -1;
  प्रत्यागम pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
पूर्णाङ्क sqlite3_quota_remove(ध्रुव वर्ण *zFilename){
  वर्ण *z_पूर्णम्;            /* Full pathname for zFilename */
  size_t n_पूर्णम्;           /* Number of bytes in zFilename */
  पूर्णाङ्क rc;                 /* Result code */
  quota_गणः *p_गणः;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *p_अग्र_File;   /* next file in the group */
  पूर्णाङ्क diff;               /* Difference between filenames */
  वर्ण c;                 /* First character past end of pattern */

  z_पूर्णम् = (वर्ण*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  यदि( z_पूर्णम्==0 ) प्रत्यागम SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->x_पूर्णम्_Pathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, z_पूर्णम्);
  यदि( rc ){
    sqlite3_free(z_पूर्णम्);
    प्रत्यागम rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  n_पूर्णम् = वाक्यदैर्घ्यम्(z_पूर्णम्);
  यदि( n_पूर्णम्>0 && (z_पूर्णम्[n_पूर्णम्-1]=='/' || z_पूर्णम्[n_पूर्णम्-1]=='\\') ){
    n_पूर्णम्--;
    z_पूर्णम्[n_पूर्णम्] = 0;
  }

  quotaEnter();
  p_गणः = quota_गणः_Find(z_पूर्णम्);
  यदि( p_गणः ){
    प्रति(pFile=p_गणः->pFiles; pFile && rc==SQLITE_OK; pFile=p_अग्र_File){
      p_अग्र_File = pFile->p_अग्र;
      diff = वाक्ये_तुलय(z_पूर्णम्, pFile->zFilename, n_पूर्णम्);
      यदि( diff==0 && ((c = pFile->zFilename[n_पूर्णम्])==0 || c=='/' || c=='\\') ){
        यदि( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }अन्यथा{
          rc = gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
          quotaRemoveFile(pFile);
          quota_गणः_Deref(p_गणः);
        }
      }
    }
  }
  quotaLeave();
  sqlite3_free(z_पूर्णम्);
  प्रत्यागम rc;
}

/***************************** Test Code ***********************************/
#यदि_परिभाषितम् SQLITE_TEST
#अन्तर्भाव "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
प्रकारनिर्देश संरचना TclQuotaCallback TclQuotaCallback;
संरचना TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);


/*
** This is the callback from a quota-over-limit.
*/
स्थिर शून्य tclQuotaCallback(
  ध्रुव वर्ण *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *pi_सीमा,         /* IN/OUT: The current limit */
  sqlite3_int64 i_आकार,            /* Total size of all files in the group */
  शून्य *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  अचिह्नित पूर्णाङ्क rnd;               /* Random part of pVarname */
  पूर्णाङ्क rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  यदि( p==0 ) प्रत्यागम;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_गणनम्(pVarname);
  sqlite3_randomness(परिमाण(rnd), (शून्य *)&rnd);
  Tcl_Append_Obj_गन्तव्यम्_Obj(pVarname, Tcl_NewIntObj((पूर्णाङ्क)(rnd&0x7FFFFFFF)));
  Tcl_Obj_समुच्चयः_Var2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*pi_सीमा), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_गणनम्(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(i_आकार));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  यदि( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *p_सीमा = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, p_सीमा, &x);
    *pi_सीमा = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_गणनम्(pEval);
  Tcl_Decr_Ref_गणनम्(pVarname);
  यदि( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
स्थिर शून्य tclCallbackDestructor(शून्य *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  यदि( p ){
    Tcl_Decr_Ref_गणनम्(p->pScript);
    sqlite3_free((वर्ण *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_initialize(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *zName;              /* Name of new quota VFS */
  पूर्णाङ्क makeDefault;                /* True to make the new VFS the default */
  पूर्णाङ्क rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "NAME MAKEDEFAULT");
    प्रत्यागम TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  यदि( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) प्रत्यागम TCL_ERROR;
  यदि( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);

  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_shutdown(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;                         /* Value returned by quota_shutdown() */

  यदि( objc!=1 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);

  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_समुच्चयः(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *z_आदर्शः;           /* File pattern to configure */
  Tcl_WideInt i_सीमा;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  पूर्णाङ्क rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_आकार nScript;               /* Length of callback script */
  शून्य (*xDestroy)(शून्य*);        /* Optional destructor for pArg */
  शून्य (*xCallback)(ध्रुव वर्ण *, sqlite3_int64 *, sqlite3_int64, शून्य *);

  /* Process arguments */
  यदि( objc!=4 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    प्रत्यागम TCL_ERROR;
  }
  z_आदर्शः = Tcl_GetString(objv[1]);
  यदि( Tcl_GetWideIntFromObj(interp, objv[2], &i_सीमा) ) प्रत्यागम TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  यदि( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(परिमाण(TclQuotaCallback));
    यदि( !p ){
      Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)"SQLITE_NOMEM", TCL_STATIC);
      प्रत्यागम TCL_OK;
    }
    स्मृतिं_पूरय(p, 0, परिमाण(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_गणनम्(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }अन्यथा{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_समुच्चयः(z_आदर्शः, i_सीमा, xCallback, (शून्य*)p, xDestroy);

  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *zFilename;          /* File pattern to configure */
  पूर्णाङ्क rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "FILENAME");
    प्रत्यागम TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
  प्रत्यागम TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_dump(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *p_परिणाम;
  Tcl_Obj *p_गणः_पदांशः;
  Tcl_Obj *p_File_पदांशः;
  quota_गणः *p_गणः;
  quotaFile *pFile;

  p_परिणाम = Tcl_NewObj();
  quotaEnter();
  प्रति(p_गणः=gQuota.p_गणः; p_गणः; p_गणः=p_गणः->p_अग्र){
    p_गणः_पदांशः = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, p_गणः_पदांशः,
          Tcl_NewStringObj(p_गणः->z_आदर्शः, -1));
    Tcl_ListObjAppendElement(interp, p_गणः_पदांशः,
          Tcl_NewWideIntObj(p_गणः->i_सीमा));
    Tcl_ListObjAppendElement(interp, p_गणः_पदांशः,
          Tcl_NewWideIntObj(p_गणः->i_आकार));
    प्रति(pFile=p_गणः->pFiles; pFile; pFile=pFile->p_अग्र){
      पूर्णाङ्क i;
      वर्ण z_अस्थायी[1000];
      p_File_पदांशः = Tcl_NewObj();
      sqlite3_snprintf(परिमाण(z_अस्थायी), z_अस्थायी, "%s", pFile->zFilename);
      प्रति(i=0; z_अस्थायी[i]; i++){ यदि( z_अस्थायी[i]=='\\' ) z_अस्थायी[i] = '/'; }
      Tcl_ListObjAppendElement(interp, p_File_पदांशः,
            Tcl_NewStringObj(z_अस्थायी, -1));
      Tcl_ListObjAppendElement(interp, p_File_पदांशः,
            Tcl_NewWideIntObj(pFile->i_आकार));
      Tcl_ListObjAppendElement(interp, p_File_पदांशः,
            Tcl_NewWideIntObj(pFile->nRef));
      Tcl_ListObjAppendElement(interp, p_File_पदांशः,
            Tcl_NewWideIntObj(pFile->deleteOnClose));
      Tcl_ListObjAppendElement(interp, p_गणः_पदांशः, p_File_पदांशः);
    }
    Tcl_ListObjAppendElement(interp, p_परिणाम, p_गणः_पदांशः);
  }
  quotaLeave();
  Tcl_समुच्चयः_Obj_परिणाम(interp, p_परिणाम);
  प्रत्यागम TCL_OK;
}

/* Defined in test1.c */
बाह्य शून्य *sqlite3_Test_Text_गन्तव्यम्_Ptr(ध्रुव वर्ण*);
बाह्य ध्रुव वर्ण *sqlite3_Test_Ptr_गन्तव्यम्_Text(शून्य*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fopen(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *zFilename;          /* File pattern to configure */
  ध्रुव वर्ण *z_बहुलकम्;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "FILENAME MODE");
    प्रत्यागम TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  z_बहुलकम् = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, z_बहुलकम्);
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण*)sqlite3_Test_Ptr_गन्तव्यम्_Text(p), TCL_VOLATILE);
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fread(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  वर्ण *zBuf;
  पूर्णाङ्क sz;
  पूर्णाङ्क nElem;
  size_t got;

  यदि( objc!=4 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE SIZE NELEM");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  यदि( Tcl_GetIntFromObj(interp, objv[2], &sz) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) प्रत्यागम TCL_ERROR;
  zBuf = (वर्ण*)sqlite3_malloc( sz*nElem + 1 );
  यदि( zBuf==0 ){
    Tcl_समुच्चयः_परिणाम(interp, "out of memory", TCL_STATIC);
    प्रत्यागम TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_समुच्चयः_परिणाम(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fwrite(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  वर्ण *zBuf;
  पूर्णाङ्क sz;
  पूर्णाङ्क nElem;
  size_t got;

  यदि( objc!=5 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  यदि( Tcl_GetIntFromObj(interp, objv[2], &sz) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) प्रत्यागम TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewWideIntObj(got));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fclose(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fflush(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णाङ्क rc;
  पूर्णाङ्क doSync = 0;

  यदि( objc!=2 && objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE ?HARDSYNC?");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  यदि( objc==3 ){
    यदि( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) प्रत्यागम TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_fseek(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णाङ्क ofst;
  ध्रुव वर्ण *zWhence;
  पूर्णाङ्क whence;
  पूर्णाङ्क rc;

  यदि( objc!=4 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE OFFSET WHENCE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  यदि( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) प्रत्यागम TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  यदि( वाक्यं_तुलय(zWhence, "SEEK_SET")==0 ){
    whence = SEEK_SET;
  }अन्यथा यदि( वाक्यं_तुलय(zWhence, "SEEK_CUR")==0 ){
    whence = SEEK_CUR;
  }अन्यथा यदि( वाक्यं_तुलय(zWhence, "SEEK_END")==0 ){
    whence = SEEK_END;
  }अन्यथा{
    Tcl_Append_परिणाम(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_rewind(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_ftell(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewWideIntObj(x));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_ftruncate(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  पूर्णाङ्क rc;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE SIZE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  यदि( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) प्रत्यागम TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file_आकार(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_आकार(p);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewWideIntObj(x));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file_truesize(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewWideIntObj(x));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file_mtime(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  time_t t;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewWideIntObj(t));
  प्रत्यागम TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_remove(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *zFilename;          /* File pattern to configure */
  पूर्णाङ्क rc;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "FILENAME");
    प्रत्यागम TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_glob(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *z_आदर्शः;          /* The glob pattern */
  ध्रुव वर्ण *zText;             /* Text to compare against the pattern */
  पूर्णाङ्क rc;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "PATTERN TEXT");
    प्रत्यागम TCL_ERROR;
  }
  z_आदर्शः = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(z_आदर्शः, zText);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_file_उपलब्धम्(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_उपलब्धम्(p);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewWideIntObj(x));
  प्रत्यागम TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_quota_ferror(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णाङ्क x;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "HANDLE");
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_Test_Text_गन्तव्यम्_Ptr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(x));
  प्रत्यागम TCL_OK;
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
    { "sqlite3_quota_set",           test_quota_समुच्चयः },
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
    { "sqlite3_quota_file_size",     test_quota_file_आकार },
    { "sqlite3_quota_file_truesize", test_quota_file_truesize },
    { "sqlite3_quota_file_mtime",    test_quota_file_mtime },
    { "sqlite3_quota_remove",        test_quota_remove },
    { "sqlite3_quota_glob",          test_quota_glob },
    { "sqlite3_quota_file_available",test_quota_file_उपलब्धम् },
    { "sqlite3_quota_ferror",        test_quota_ferror },
  };
  पूर्णाङ्क i;

  प्रति(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  प्रत्यागम TCL_OK;
}
#यदि_अन्तः
