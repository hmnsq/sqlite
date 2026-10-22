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
#समावेश "test_quota.h"
#समावेश <string.h>
#समावेश <assert.h>

/*
** For an build without mutexes, no-op the mutex calls.
*/
#अगर defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#घोषणा sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#घोषणा sqlite3_mutex_free(X)
#घोषणा sqlite3_mutex_enter(X)
#घोषणा sqlite3_mutex_try(X)      SQLITE_OK
#घोषणा sqlite3_mutex_leave(X)
#घोषणा sqlite3_mutex_held(X)     ((व्योम)(X),1)
#घोषणा sqlite3_mutex_notheld(X)  ((व्योम)(X),1)
#पूर्ण_अगर /* SQLITE_THREADSAFE==0 */


#अगर_घोषित _WIN32
# include <windows.h>
# include <io.h>
#अन्यथा
# include <unistd.h>
#पूर्ण_अगर


/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
प्रकार काष्ठा quotaGroup quotaGroup;
प्रकार काष्ठा quotaConn quotaConn;
प्रकार काष्ठा quotaFile quotaFile;

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
काष्ठा quotaGroup {
  स्थिर अक्षर *zPattern;          /* Filename pattern to be quotaed */
  sqlite3_int64 iLimit;          /* Upper bound on total file size */
  sqlite3_int64 i_आकार;           /* Current size of all files */
  व्योम (*xCallback)(             /* Callback invoked when going over quota */
     स्थिर अक्षर *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 i_आकार,           /* Total size of all files in the group */
     व्योम *pArg                     /* Client data */
  );
  व्योम *pArg;                    /* Third argument to the xCallback() */
  व्योम (*xDestroy)(व्योम*);       /* Optional destructor for pArg */
  quotaGroup *p_अग्र, **pp_पूर्व;   /* Doubly linked list of all quota objects */
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
काष्ठा quotaFile {
  अक्षर *zFilename;                /* Name of this file */
  quotaGroup *pGroup;             /* Quota group to which this file belongs */
  sqlite3_int64 i_आकार;            /* Current size of this file */
  पूर्णांक nRef;                       /* Number of times this file is open */
  पूर्णांक deleteOnClose;              /* True to delete this file when it closes */
  quotaFile *p_अग्र, **pp_पूर्व;     /* Linked list of files in the same group */
};

/*
** An instance of the following object represents each open connection
** to a file that participates in quota tracking.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
काष्ठा quotaConn {
  sqlite3_file base;              /* Base class - must be first */
  quotaFile *pFile;               /* The underlying file */
  /* The underlying VFS sqlite3_file is appended to this object */
};

/*
** An instance of the following object records the state of an
** open file.  This object is opaque to all users - the internal
** structure is only visible to the functions below.
*/
काष्ठा quota_FILE {
  खाता *f;                /* Open stdio file pointer */
  sqlite3_int64 iOfst;    /* Current offset into the file */
  quotaFile *pFile;       /* The file record in the quota system */
#अगर_घोषित _WIN32
  अक्षर *zMbcsName;        /* Full MBCS pathname of the file */
#पूर्ण_अगर
};


/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gQuota structure.
*/
अटल काष्ठा {
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
  पूर्णांक isInitialized;

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
अटल व्योम quotaEnter(व्योम){ sqlite3_mutex_enter(gQuota.pMutex); }
अटल व्योम quotaLeave(व्योम){ sqlite3_mutex_leave(gQuota.pMutex); }

/* Count the number of open files in a quotaGroup
*/
अटल पूर्णांक quota_Group_Open_File_गिनती(quotaGroup *pGroup){
  पूर्णांक N = 0;
  quotaFile *pFile = pGroup->pFiles;
  जबतक( pFile ){
    अगर( pFile->nRef ) N++;
    pFile = pFile->p_अग्र;
  }
  वापस N;
}

/* Remove a file from a quota group.
*/
अटल व्योम quotaRemoveFile(quotaFile *pFile){
  quotaGroup *pGroup = pFile->pGroup;
  pGroup->i_आकार -= pFile->i_आकार;
  *pFile->pp_पूर्व = pFile->p_अग्र;
  अगर( pFile->p_अग्र ) pFile->p_अग्र->pp_पूर्व = pFile->pp_पूर्व;
  sqlite3_free(pFile);
}

/* Remove all files from a quota group.  It is always the case that
** all files will be closed when this routine is called.
*/
अटल व्योम quotaRemoveAllFiles(quotaGroup *pGroup){
  जबतक( pGroup->pFiles ){
    निश्चित( pGroup->pFiles->nRef==0 );
    quotaRemoveFile(pGroup->pFiles);
  }
}


/* If the reference count and threshold for a quotaGroup are both
** zero, then destroy the quotaGroup.
*/
अटल व्योम quotaGroupDeref(quotaGroup *pGroup){
  अगर( pGroup->iLimit==0 && quota_Group_Open_File_गिनती(pGroup)==0 ){
    quotaRemoveAllFiles(pGroup);
    *pGroup->pp_पूर्व = pGroup->p_अग्र;
    अगर( pGroup->p_अग्र ) pGroup->p_अग्र->pp_पूर्व = pGroup->pp_पूर्व;
    अगर( pGroup->xDestroy ) pGroup->xDestroy(pGroup->pArg);
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
अटल पूर्णांक quotaStrglob(स्थिर अक्षर *zGlob, स्थिर अक्षर *z){
  पूर्णांक c, c2, cx;
  पूर्णांक invert;
  पूर्णांक seen;

  जबतक( (c = (*(zGlob++)))!=0 ){
    अगर( c=='*' ){
      जबतक( (c=(*(zGlob++))) == '*' || c=='?' ){
        अगर( c=='?' && (*(z++))==0 ) वापस 0;
      }
      अगर( c==0 ){
        वापस 1;
      }अन्यथा अगर( c=='[' ){
        जबतक( *z && quotaStrglob(zGlob-1,z)==0 ){
          z++;
        }
        वापस (*z)!=0;
      }
      cx = (c=='/') ? '\\' : c;
      जबतक( (c2 = (*(z++)))!=0 ){
        जबतक( c2!=c && c2!=cx ){
          c2 = *(z++);
          अगर( c2==0 ) वापस 0;
        }
        अगर( quotaStrglob(zGlob,z) ) वापस 1;
      }
      वापस 0;
    }अन्यथा अगर( c=='?' ){
      अगर( (*(z++))==0 ) वापस 0;
    }अन्यथा अगर( c=='[' ){
      पूर्णांक prior_c = 0;
      seen = 0;
      invert = 0;
      c = *(z++);
      अगर( c==0 ) वापस 0;
      c2 = *(zGlob++);
      अगर( c2=='^' ){
        invert = 1;
        c2 = *(zGlob++);
      }
      अगर( c2==']' ){
        अगर( c==']' ) seen = 1;
        c2 = *(zGlob++);
      }
      जबतक( c2 && c2!=']' ){
        अगर( c2=='-' && zGlob[0]!=']' && zGlob[0]!=0 && prior_c>0 ){
          c2 = *(zGlob++);
          अगर( c>=prior_c && c<=c2 ) seen = 1;
          prior_c = 0;
        }अन्यथा{
          अगर( c==c2 ){
            seen = 1;
          }
          prior_c = c2;
        }
        c2 = *(zGlob++);
      }
      अगर( c2==0 || (seen ^ invert)==0 ) वापस 0;
    }अन्यथा अगर( c=='/' ){
      अगर( z[0]!='/' && z[0]!='\\' ) वापस 0;
      z++;
    }अन्यथा{
      अगर( c!=(*(z++)) ) वापस 0;
    }
  }
  वापस *z==0;
}


/* Find a quotaGroup given the filename.
**
** Return a pointer to the quotaGroup object. Return NULL if not found.
*/
अटल quotaGroup *quotaGroupFind(स्थिर अक्षर *zFilename){
  quotaGroup *p;
  क्रम(p=gQuota.pGroup; p && quotaStrglob(p->zPattern, zFilename)==0;
      p=p->p_अग्र){}
  वापस p;
}

/* Translate an sqlite3_file* that is really a quotaConn* into
** the sqlite3_file* for the underlying original VFS.
*/
अटल sqlite3_file *quotaSubOpen(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  वापस (sqlite3_file*)&p[1];
}

/* Find a file in a quota group and return a pointer to that file.
** Return NULL if the file is not in the group.
*/
अटल quotaFile *quotaFindFile(
  quotaGroup *pGroup,     /* Group in which to look for the file */
  स्थिर अक्षर *zName,      /* Full pathname of the file */
  पूर्णांक createFlag          /* Try to create the file if not found */
){
  quotaFile *pFile = pGroup->pFiles;
  जबतक( pFile && म_भेद(pFile->zFilename, zName)!=0 ){
    pFile = pFile->p_अग्र;
  }
  अगर( pFile==0 && createFlag ){
    पूर्णांक nName = (पूर्णांक)(म_माप(zName) & 0x3fffffff);
    pFile = (quotaFile *)sqlite3_malloc( माप(*pFile) + nName + 1 );
    अगर( pFile ){
      स_रखो(pFile, 0, माप(*pFile));
      pFile->zFilename = (अक्षर*)&pFile[1];
      स_नकल(pFile->zFilename, zName, nName+1);
      pFile->p_अग्र = pGroup->pFiles;
      अगर( pGroup->pFiles ) pGroup->pFiles->pp_पूर्व = &pFile->p_अग्र;
      pFile->pp_पूर्व = &pGroup->pFiles;
      pGroup->pFiles = pFile;
      pFile->pGroup = pGroup;
    }
  }
  वापस pFile;
}
/*
** Translate UTF8 to MBCS for use in fopen() calls.  Return a pointer to the
** translated text..  Call quota_mbcs_free() to deallocate any memory
** used to store the returned pointer when done.
*/
अटल अक्षर *quota_utf8_to_mbcs(स्थिर अक्षर *zUtf8){
#अगर_घोषित _WIN32
  माप_प्रकार n;          /* Bytes in zUtf8 */
  पूर्णांक nWide;         /* number of UTF-16 characters */
  पूर्णांक nMbcs;         /* Bytes of MBCS */
  LPWSTR zTmpWide;   /* The UTF16 text */
  अक्षर *zMbcs;       /* The MBCS text */
  पूर्णांक codepage;      /* Code page used by fopen() */

  n = म_माप(zUtf8);
  nWide = MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, शून्य, 0);
  अगर( nWide==0 ) वापस 0;
  zTmpWide = (LPWSTR)sqlite3_malloc( (nWide+1)*माप(zTmpWide[0]) );
  अगर( zTmpWide==0 ) वापस 0;
  MultiByteToWideChar(CP_UTF8, 0, zUtf8, -1, zTmpWide, nWide);
  codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
  nMbcs = WideCharToMultiByte(codepage, 0, zTmpWide, nWide, 0, 0, 0, 0);
  zMbcs = nMbcs ? (अक्षर*)sqlite3_malloc( nMbcs+1 ) : 0;
  अगर( zMbcs ){
    WideCharToMultiByte(codepage, 0, zTmpWide, nWide, zMbcs, nMbcs, 0, 0);
  }
  sqlite3_free(zTmpWide);
  वापस zMbcs;
#अन्यथा
  वापस (अक्षर*)zUtf8;  /* No-op on unix */
#पूर्ण_अगर
}

/*
** Deallocate any memory allocated by quota_utf8_to_mbcs().
*/
अटल व्योम quota_mbcs_free(अक्षर *zOld){
#अगर_घोषित _WIN32
  sqlite3_free(zOld);
#अन्यथा
  /* No-op on unix */
#पूर्ण_अगर
}

/************************* VFS Method Wrappers *****************************/
/*
** This is the xOpen method used for the "quota" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate quota group if it is a
** file that needs to be tracked.
*/
अटल पूर्णांक quotaOpen(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  स्थिर अक्षर *zName,          /* Name of file to be opened */
  sqlite3_file *pConn,        /* Fill in this file descriptor */
  पूर्णांक flags,                  /* Flags to control the opening */
  पूर्णांक *pOutFlags              /* Flags showing results of opening */
){
  पूर्णांक rc;                                    /* Result code */
  quotaConn *pQuotaOpen;                     /* The new quota file descriptor */
  quotaFile *pFile;                          /* Corresponding quotaFile obj */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_file *pSubOpen;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* If the file is not a main database file or a WAL, then use the
  ** normal xOpen method.
  */
  अगर( (flags & (SQLITE_OPEN_MAIN_DB|SQLITE_OPEN_WAL))==0 ){
    वापस pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }

  /* If the name of the file does not match any quota group, then
  ** use the normal xOpen method.
  */
  quotaEnter();
  pGroup = quotaGroupFind(zName);
  अगर( pGroup==0 ){
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
  }अन्यथा{
    /* If we get to this point, it means the file needs to be quota tracked.
    */
    pQuotaOpen = (quotaConn*)pConn;
    pSubOpen = quotaSubOpen(pConn);
    rc = pOrigVfs->xOpen(pOrigVfs, zName, pSubOpen, flags, pOutFlags);
    अगर( rc==SQLITE_OK ){
      pFile = quotaFindFile(pGroup, zName, 1);
      अगर( pFile==0 ){
        quotaLeave();
        pSubOpen->pMethods->xClose(pSubOpen);
        वापस SQLITE_NOMEM;
      }
      pFile->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE)!=0;
      pFile->nRef++;
      pQuotaOpen->pFile = pFile;
      अगर( pSubOpen->pMethods->iVersion==1 ){
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV1;
      }अन्यथा{
        pQuotaOpen->base.pMethods = &gQuota.sIoMethodsV2;
      }
    }
  }
  quotaLeave();
  वापस rc;
}

/*
** This is the xDelete method used for the "quota" VFS.
**
** If the file being deleted is part of the quota group, then reduce
** the size of the quota group accordingly.  And remove the file from
** the set of files in the quota group.
*/
अटल पूर्णांक quotaDelete(
  sqlite3_vfs *pVfs,          /* The quota VFS */
  स्थिर अक्षर *zName,          /* Name of file to be deleted */
  पूर्णांक syncDir                 /* Do a directory sync after deleting */
){
  पूर्णांक rc;                                    /* Result code */
  quotaFile *pFile;                          /* Files in the quota */
  quotaGroup *pGroup;                        /* The group file belongs to */
  sqlite3_vfs *pOrigVfs = gQuota.pOrigVfs;   /* Real VFS */

  /* Do the actual file delete */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);

  /* If the file just deleted is a member of a quota group, then remove
  ** it from that quota group.
  */
  अगर( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = quotaGroupFind(zName);
    अगर( pGroup ){
      pFile = quotaFindFile(pGroup, zName, 0);
      अगर( pFile ){
        अगर( pFile->nRef ){
          pFile->deleteOnClose = 1;
        }अन्यथा{
          quotaRemoveFile(pFile);
          quotaGroupDeref(pGroup);
        }
      }
    }
    quotaLeave();
  }
  वापस rc;
}


/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.  But we
** also have to unlink the quotaConn from the quotaFile and quotaGroup.
** The quotaFile and/or quotaGroup are freed if they are no longer in use.
*/
अटल पूर्णांक quotaClose(sqlite3_file *pConn){
  quotaConn *p = (quotaConn*)pConn;
  quotaFile *pFile = p->pFile;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  पूर्णांक rc;
  rc = pSubOpen->pMethods->xClose(pSubOpen);
  quotaEnter();
  pFile->nRef--;
  अगर( pFile->nRef==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    अगर( pFile->deleteOnClose ){
      gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
      quotaRemoveFile(pFile);
    }
    quotaGroupDeref(pGroup);
  }
  quotaLeave();
  वापस rc;
}

/* Pass xRead requests directory thru to the original VFS without
** further processing.
*/
अटल पूर्णांक quotaRead(
  sqlite3_file *pConn,
  व्योम *pBuf,
  पूर्णांक iAmt,
  sqlite3_int64 iOfst
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
}

/* Check xWrite requests to see if they expand the file.  If they do,
** the perform a quota check before passing them through to the
** original VFS.
*/
अटल पूर्णांक quotaWrite(
  sqlite3_file *pConn,
  स्थिर व्योम *pBuf,
  पूर्णांक iAmt,
  sqlite3_int64 iOfst
){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  sqlite3_int64 iEnd = iOfst+iAmt;
  quotaGroup *pGroup;
  quotaFile *pFile = p->pFile;
  sqlite3_int64 szNew;

  अगर( pFile->i_आकार<iEnd ){
    pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->i_आकार - pFile->i_आकार + iEnd;
    अगर( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      अगर( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      अगर( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        quotaLeave();
        वापस SQLITE_FULL;
      }
    }
    pGroup->i_आकार = szNew;
    pFile->i_आकार = iEnd;
    quotaLeave();
  }
  वापस pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
}

/* Pass xTruncate requests thru to the original VFS.  If the
** success, update the file size.
*/
अटल पूर्णांक quotaTruncate(sqlite3_file *pConn, sqlite3_int64 आकार){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  पूर्णांक rc = pSubOpen->pMethods->xTruncate(pSubOpen, आकार);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  अगर( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->i_आकार -= pFile->i_आकार;
    pFile->i_आकार = आकार;
    pGroup->i_आकार += आकार;
    quotaLeave();
  }
  वापस rc;
}

/* Pass xSync requests through to the original VFS without change
*/
अटल पूर्णांक quotaSync(sqlite3_file *pConn, पूर्णांक flags){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xSync(pSubOpen, flags);
}

/* Pass xFileSize requests through to the original VFS but then
** update the quotaGroup with the new size before returning.
*/
अटल पूर्णांक quota_File_आकार(sqlite3_file *pConn, sqlite3_int64 *p_आकार){
  quotaConn *p = (quotaConn*)pConn;
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  quotaFile *pFile = p->pFile;
  quotaGroup *pGroup;
  sqlite3_int64 sz;
  पूर्णांक rc;

  rc = pSubOpen->pMethods->x_File_आकार(pSubOpen, &sz);
  अगर( rc==SQLITE_OK ){
    quotaEnter();
    pGroup = pFile->pGroup;
    pGroup->i_आकार -= pFile->i_आकार;
    pFile->i_आकार = sz;
    pGroup->i_आकार += sz;
    quotaLeave();
    *p_आकार = sz;
  }
  वापस rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
अटल पूर्णांक quotaLock(sqlite3_file *pConn, पूर्णांक lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xLock(pSubOpen, lock);
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
अटल पूर्णांक quotaUnlock(sqlite3_file *pConn, पूर्णांक lock){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xUnlock(pSubOpen, lock);
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
अटल पूर्णांक quotaCheckReservedLock(sqlite3_file *pConn, पूर्णांक *pResOut){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
}

/* Pass xFileControl requests through to the original VFS unchanged.
*/
अटल पूर्णांक quotaFileControl(sqlite3_file *pConn, पूर्णांक op, व्योम *pArg){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  पूर्णांक rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
#अगर defined(SQLITE_FCNTL_VFSNAME)
  अगर( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
    *(अक्षर**)pArg = sqlite3_mprintf("quota/%z", *(अक्षर**)pArg);
  }
#पूर्ण_अगर
  वापस rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
अटल पूर्णांक quota_Sector_आकार(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->x_Sector_आकार(pSubOpen);
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
अटल पूर्णांक quotaDeviceCharacteristics(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
अटल पूर्णांक quotaShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  पूर्णांक iRegion,                    /* Region to retrieve */
  पूर्णांक szRegion,                   /* Size of regions */
  पूर्णांक bExtend,                    /* True to extend file if necessary */
  व्योम अस्थिर **pp              /* OUT: Mapped memory */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend, pp);
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
अटल पूर्णांक quotaShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  पूर्णांक ofst,                  /* First lock to acquire or release */
  पूर्णांक n,                     /* Number of locks to acquire or release */
  पूर्णांक flags                  /* What to do with the lock */
){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
अटल व्योम quotaShmBarrier(sqlite3_file *pConn){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  pSubOpen->pMethods->xShmBarrier(pSubOpen);
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
अटल पूर्णांक quotaShmUnmap(sqlite3_file *pConn, पूर्णांक deleteFlag){
  sqlite3_file *pSubOpen = quotaSubOpen(pConn);
  वापस pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
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
पूर्णांक sqlite3_quota_initialize(स्थिर अक्षर *zOrigVfsName, पूर्णांक makeDefault){
  sqlite3_vfs *pOrigVfs;
  अगर( gQuota.isInitialized ) वापस SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  अगर( pOrigVfs==0 ) वापस SQLITE_ERROR;
  निश्चित( pOrigVfs!=&gQuota.sThisVfs );
  gQuota.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  अगर( !gQuota.pMutex ){
    वापस SQLITE_NOMEM;
  }
  gQuota.isInitialized = 1;
  gQuota.pOrigVfs = pOrigVfs;
  gQuota.sThisVfs = *pOrigVfs;
  gQuota.sThisVfs.xOpen = quotaOpen;
  gQuota.sThisVfs.xDelete = quotaDelete;
  gQuota.sThisVfs.szOsFile += माप(quotaConn);
  gQuota.sThisVfs.zName = "quota";
  gQuota.sIoMethodsV1.iVersion = 1;
  gQuota.sIoMethodsV1.xClose = quotaClose;
  gQuota.sIoMethodsV1.xRead = quotaRead;
  gQuota.sIoMethodsV1.xWrite = quotaWrite;
  gQuota.sIoMethodsV1.xTruncate = quotaTruncate;
  gQuota.sIoMethodsV1.xSync = quotaSync;
  gQuota.sIoMethodsV1.x_File_आकार = quota_File_आकार;
  gQuota.sIoMethodsV1.xLock = quotaLock;
  gQuota.sIoMethodsV1.xUnlock = quotaUnlock;
  gQuota.sIoMethodsV1.xCheckReservedLock = quotaCheckReservedLock;
  gQuota.sIoMethodsV1.xFileControl = quotaFileControl;
  gQuota.sIoMethodsV1.x_Sector_आकार = quota_Sector_आकार;
  gQuota.sIoMethodsV1.xDeviceCharacteristics = quotaDeviceCharacteristics;
  gQuota.sIoMethodsV2 = gQuota.sIoMethodsV1;
  gQuota.sIoMethodsV2.iVersion = 2;
  gQuota.sIoMethodsV2.xShmMap = quotaShmMap;
  gQuota.sIoMethodsV2.xShmLock = quotaShmLock;
  gQuota.sIoMethodsV2.xShmBarrier = quotaShmBarrier;
  gQuota.sIoMethodsV2.xShmUnmap = quotaShmUnmap;
  sqlite3_vfs_register(&gQuota.sThisVfs, makeDefault);
  वापस SQLITE_OK;
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
पूर्णांक sqlite3_quota_shutdown(व्योम){
  quotaGroup *pGroup;
  अगर( gQuota.isInitialized==0 ) वापस SQLITE_MISUSE;
  क्रम(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->p_अग्र){
    अगर( quota_Group_Open_File_गिनती(pGroup)>0 ) वापस SQLITE_MISUSE;
  }
  जबतक( gQuota.pGroup ){
    pGroup = gQuota.pGroup;
    gQuota.pGroup = pGroup->p_अग्र;
    pGroup->iLimit = 0;
    निश्चित( quota_Group_Open_File_गिनती(pGroup)==0 );
    quotaGroupDeref(pGroup);
  }
  gQuota.isInitialized = 0;
  sqlite3_mutex_free(gQuota.pMutex);
  sqlite3_vfs_unregister(&gQuota.sThisVfs);
  स_रखो(&gQuota, 0, माप(gQuota));
  वापस SQLITE_OK;
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
पूर्णांक sqlite3_quota_set(
  स्थिर अक्षर *zPattern,           /* The filename pattern */
  sqlite3_int64 iLimit,           /* New quota to set for this quota group */
  व्योम (*xCallback)(              /* Callback invoked when going over quota */
     स्थिर अक्षर *zFilename,         /* Name of file whose size increases */
     sqlite3_int64 *piLimit,        /* IN/OUT: The current limit */
     sqlite3_int64 i_आकार,           /* Total size of all files in the group */
     व्योम *pArg                     /* Client data */
  ),
  व्योम *pArg,                     /* client data passed thru to callback */
  व्योम (*xDestroy)(व्योम*)         /* Optional destructor for pArg */
){
  quotaGroup *pGroup;
  quotaEnter();
  pGroup = gQuota.pGroup;
  जबतक( pGroup && म_भेद(pGroup->zPattern, zPattern)!=0 ){
    pGroup = pGroup->p_अग्र;
  }
  अगर( pGroup==0 ){
    पूर्णांक nPattern = (पूर्णांक)(म_माप(zPattern) & 0x3fffffff);
    अगर( iLimit<=0 ){
      quotaLeave();
      वापस SQLITE_OK;
    }
    pGroup = (quotaGroup *)sqlite3_malloc( माप(*pGroup) + nPattern + 1 );
    अगर( pGroup==0 ){
      quotaLeave();
      वापस SQLITE_NOMEM;
    }
    स_रखो(pGroup, 0, माप(*pGroup));
    pGroup->zPattern = (अक्षर*)&pGroup[1];
    स_नकल((अक्षर *)pGroup->zPattern, zPattern, nPattern+1);
    अगर( gQuota.pGroup ) gQuota.pGroup->pp_पूर्व = &pGroup->p_अग्र;
    pGroup->p_अग्र = gQuota.pGroup;
    pGroup->pp_पूर्व = &gQuota.pGroup;
    gQuota.pGroup = pGroup;
  }
  pGroup->iLimit = iLimit;
  pGroup->xCallback = xCallback;
  अगर( pGroup->xDestroy && pGroup->pArg!=pArg ){
    pGroup->xDestroy(pGroup->pArg);
  }
  pGroup->pArg = pArg;
  pGroup->xDestroy = xDestroy;
  quotaGroupDeref(pGroup);
  quotaLeave();
  वापस SQLITE_OK;
}

/*
** Bring the named file under quota management.  Or if it is already under
** management, update its size.
*/
पूर्णांक sqlite3_quota_file(स्थिर अक्षर *zFilename){
  अक्षर *zFull = 0;
  sqlite3_file *fd;
  पूर्णांक rc;
  पूर्णांक outFlags = 0;
  sqlite3_int64 i_आकार;
  पूर्णांक nAlloc = gQuota.sThisVfs.szOsFile + gQuota.sThisVfs.mxPathname+2;

  /* Allocate space for a file-handle and the full path for file zFilename */
  fd = (sqlite3_file *)sqlite3_malloc(nAlloc);
  अगर( fd==0 ){
    rc = SQLITE_NOMEM;
  }अन्यथा{
    zFull = &((अक्षर *)fd)[gQuota.sThisVfs.szOsFile];
    rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
        gQuota.sThisVfs.mxPathname+1, zFull);
  }

  अगर( rc==SQLITE_OK ){
    zFull[म_माप(zFull)+1] = '\0';
    rc = quotaOpen(&gQuota.sThisVfs, zFull, fd,
                   SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_DB, &outFlags);
    अगर( rc==SQLITE_OK ){
      fd->pMethods->x_File_आकार(fd, &i_आकार);
      fd->pMethods->xClose(fd);
    }अन्यथा अगर( rc==SQLITE_CANTOPEN ){
      quotaGroup *pGroup;
      quotaFile *pFile;
      quotaEnter();
      pGroup = quotaGroupFind(zFull);
      अगर( pGroup ){
        pFile = quotaFindFile(pGroup, zFull, 0);
        अगर( pFile ) quotaRemoveFile(pFile);
      }
      quotaLeave();
    }
  }

  sqlite3_free(fd);
  वापस rc;
}

/*
** Open a potentially quotaed file for I/O.
*/
quota_FILE *sqlite3_quota_fopen(स्थिर अक्षर *zFilename, स्थिर अक्षर *zMode){
  quota_FILE *p = 0;
  अक्षर *zFull = 0;
  अक्षर *zFullTranslated = 0;
  पूर्णांक rc;
  quotaGroup *pGroup;
  quotaFile *pFile;

  zFull = (अक्षर*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  अगर( zFull==0 ) वापस 0;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  अगर( rc ) जाओ quota_fopen_error;
  p = (quota_FILE*)sqlite3_malloc(माप(*p));
  अगर( p==0 ) जाओ quota_fopen_error;
  स_रखो(p, 0, माप(*p));
  zFullTranslated = quota_utf8_to_mbcs(zFull);
  अगर( zFullTranslated==0 ) जाओ quota_fopen_error;
  p->f = ख_खोलो(zFullTranslated, zMode);
  अगर( p->f==0 ) जाओ quota_fopen_error;
  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  अगर( pGroup ){
    pFile = quotaFindFile(pGroup, zFull, 1);
    अगर( pFile==0 ){
      quotaLeave();
      जाओ quota_fopen_error;
    }
    pFile->nRef++;
    p->pFile = pFile;
  }
  quotaLeave();
  sqlite3_free(zFull);
#अगर_घोषित _WIN32
  p->zMbcsName = zFullTranslated;
#पूर्ण_अगर
  वापस p;

quota_fopen_error:
  quota_mbcs_free(zFullTranslated);
  sqlite3_free(zFull);
  अगर( p && p->f ) ख_बंद(p->f);
  sqlite3_free(p);
  वापस 0;
}

/*
** Read content from a quota_FILE
*/
माप_प्रकार sqlite3_quota_fread(
  व्योम *pBuf,            /* Store the content here */
  माप_प्रकार आकार,           /* Size of each element */
  माप_प्रकार nmemb,          /* Number of elements to read */
  quota_FILE *p          /* Read from this quota_FILE object */
){
  वापस ख_पढ़ो(pBuf, आकार, nmemb, p->f);
}

/*
** Write content into a quota_FILE.  Invoke the quota callback and block
** the write if we exceed quota.
*/
माप_प्रकार sqlite3_quota_fwrite(
  स्थिर व्योम *pBuf,      /* Take content to write from here */
  माप_प्रकार आकार,           /* Size of each element */
  माप_प्रकार nmemb,          /* Number of elements */
  quota_FILE *p          /* Write to this quota_FILE object */
){
  sqlite3_int64 iOfst;
  sqlite3_int64 iEnd;
  sqlite3_int64 szNew;
  quotaFile *pFile;
  माप_प्रकार rc;

  iOfst = ख_बताओ(p->f);
  iEnd = iOfst + आकार*nmemb;
  pFile = p->pFile;
  अगर( pFile && pFile->i_आकार<iEnd ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    szNew = pGroup->i_आकार - pFile->i_आकार + iEnd;
    अगर( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
      अगर( pGroup->xCallback ){
        pGroup->xCallback(pFile->zFilename, &pGroup->iLimit, szNew,
                          pGroup->pArg);
      }
      अगर( szNew>pGroup->iLimit && pGroup->iLimit>0 ){
        iEnd = pGroup->iLimit - pGroup->i_आकार + pFile->i_आकार;
        nmemb = (माप_प्रकार)((iEnd - iOfst)/आकार);
        iEnd = iOfst + आकार*nmemb;
        szNew = pGroup->i_आकार - pFile->i_आकार + iEnd;
      }
    }
    pGroup->i_आकार = szNew;
    pFile->i_आकार = iEnd;
    quotaLeave();
  }अन्यथा{
    pFile = 0;
  }
  rc = ख_डालो(pBuf, आकार, nmemb, p->f);

  /* If the write was incomplete, adjust the file size and group size
  ** downward */
  अगर( rc<nmemb && pFile ){
    माप_प्रकार nWritten = rc;
    sqlite3_int64 iNewEnd = iOfst + आकार*nWritten;
    अगर( iNewEnd<iEnd ) iNewEnd = iEnd;
    quotaEnter();
    pFile->pGroup->i_आकार += iNewEnd - pFile->i_आकार;
    pFile->i_आकार = iNewEnd;
    quotaLeave();
  }
  वापस rc;
}

/*
** Close an open quota_FILE stream.
*/
पूर्णांक sqlite3_quota_fclose(quota_FILE *p){
  पूर्णांक rc;
  quotaFile *pFile;
  rc = ख_बंद(p->f);
  pFile = p->pFile;
  अगर( pFile ){
    quotaEnter();
    pFile->nRef--;
    अगर( pFile->nRef==0 ){
      quotaGroup *pGroup = pFile->pGroup;
      अगर( pFile->deleteOnClose ){
        gQuota.pOrigVfs->xDelete(gQuota.pOrigVfs, pFile->zFilename, 0);
        quotaRemoveFile(pFile);
      }
      quotaGroupDeref(pGroup);
    }
    quotaLeave();
  }
#अगर_घोषित _WIN32
  quota_mbcs_free(p->zMbcsName);
#पूर्ण_अगर
  sqlite3_free(p);
  वापस rc;
}

/*
** Flush memory buffers for a quota_FILE to disk.
*/
पूर्णांक sqlite3_quota_fflush(quota_FILE *p, पूर्णांक doFsync){
  पूर्णांक rc;
  rc = ख_साफ(p->f);
  अगर( rc==0 && doFsync ){
#अगर_घोषित _WIN32
    rc = _commit(_fileno(p->f));
#अन्यथा
    rc = fsync(fileno(p->f));
#पूर्ण_अगर
  }
  वापस rc!=0;
}

/*
** Seek on a quota_FILE stream.
*/
पूर्णांक sqlite3_quota_fseek(quota_FILE *p, दीर्घ offset, पूर्णांक whence){
  वापस ख_जाओ(p->f, offset, whence);
}

/*
** rewind a quota_FILE stream.
*/
व्योम sqlite3_quota_rewind(quota_FILE *p){
  शुरुआत(p->f);
}

/*
** Tell the current location of a quota_FILE stream.
*/
दीर्घ sqlite3_quota_ftell(quota_FILE *p){
  वापस ख_बताओ(p->f);
}

/*
** Test the error indicator for the given file.
*/
पूर्णांक sqlite3_quota_ferror(quota_FILE *p){
  वापस ख_त्रुटि(p->f);
}

/*
** Truncate a file to szNew bytes.
*/
पूर्णांक sqlite3_quota_ftruncate(quota_FILE *p, sqlite3_int64 szNew){
  quotaFile *pFile = p->pFile;
  पूर्णांक rc;
  अगर( (pFile = p->pFile)!=0 && pFile->i_आकार<szNew ){
    quotaGroup *pGroup;
    अगर( pFile->i_आकार<szNew ){
      /* This routine cannot be used to extend a file that is under
      ** quota management.  Only true truncation is allowed. */
      वापस -1;
    }
    pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->i_आकार += szNew - pFile->i_आकार;
    quotaLeave();
  }
#अगर_घोषित _WIN32
#  अगर defined(__MSVCRT__) && defined(SQLITE_TEST)
     /* _chsize_s() is missing from MingW (as of 2012-11-06).  Use
     ** _chsize() as a work-around for testing purposes. */
     rc = _chsize(_fileno(p->f), (दीर्घ)szNew);
#  अन्यथा
     rc = _chsize_s(_fileno(p->f), szNew);
#  endif
#अन्यथा
  rc = ftruncate(fileno(p->f), szNew);
#पूर्ण_अगर
  अगर( pFile && rc==0 ){
    quotaGroup *pGroup = pFile->pGroup;
    quotaEnter();
    pGroup->i_आकार += szNew - pFile->i_आकार;
    pFile->i_आकार = szNew;
    quotaLeave();
  }
  वापस rc;
}

/*
** Determine the time that the given file was last modified, in
** seconds size 1970.  Write the result into *pTime.  Return 0 on
** success and non-zero on any kind of error.
*/
पूर्णांक sqlite3_quota_file_mtime(quota_FILE *p, समय_प्रकार *pTime){
  पूर्णांक rc;
#अगर_घोषित _WIN32
  काष्ठा _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#अन्यथा
  काष्ठा stat buf;
  rc = ख_स्थिति(fileno(p->f), &buf);
#पूर्ण_अगर
  अगर( rc==0 ) *pTime = buf.st_mtime;
  वापस rc;
}

/*
** Return the true size of the file, as reported by the operating
** system.
*/
sqlite3_int64 sqlite3_quota_file_truesize(quota_FILE *p){
  पूर्णांक rc;
#अगर_घोषित _WIN32
  काष्ठा _stati64 buf;
  rc = _stati64(p->zMbcsName, &buf);
#अन्यथा
  काष्ठा stat buf;
  rc = ख_स्थिति(fileno(p->f), &buf);
#पूर्ण_अगर
  वापस rc==0 ? buf.st_आकार : -1;
}

/*
** Return the size of the file, as it is known to the quota subsystem.
*/
sqlite3_int64 sqlite3_quota_file_आकार(quota_FILE *p){
  वापस p->pFile ? p->pFile->i_आकार : -1;
}

/*
** Determine the amount of data in bytes available for reading
** in the given file.
*/
दीर्घ sqlite3_quota_file_available(quota_FILE *p){
  खाता* f = p->f;
  दीर्घ pos1, pos2;
  पूर्णांक rc;
  pos1 = ख_बताओ(f);
  अगर ( pos1 < 0 ) वापस -1;
  rc = ख_जाओ(f, 0, अंत_से);
  अगर ( rc != 0 ) वापस -1;
  pos2 = ख_बताओ(f);
  अगर ( pos2 < 0 ) वापस -1;
  rc = ख_जाओ(f, pos1, शुरू_से);
  अगर ( rc != 0 ) वापस -1;
  वापस pos2 - pos1;
}

/*
** Remove a managed file.  Update quotas accordingly.
*/
पूर्णांक sqlite3_quota_remove(स्थिर अक्षर *zFilename){
  अक्षर *zFull;            /* Full pathname for zFilename */
  माप_प्रकार nFull;           /* Number of bytes in zFilename */
  पूर्णांक rc;                 /* Result code */
  quotaGroup *pGroup;     /* Group containing zFilename */
  quotaFile *pFile;       /* A file in the group */
  quotaFile *p_अग्र_File;   /* next file in the group */
  पूर्णांक diff;               /* Difference between filenames */
  अक्षर c;                 /* First character past end of pattern */

  zFull = (अक्षर*)sqlite3_malloc(gQuota.sThisVfs.mxPathname + 1);
  अगर( zFull==0 ) वापस SQLITE_NOMEM;
  rc = gQuota.pOrigVfs->xFullPathname(gQuota.pOrigVfs, zFilename,
                                      gQuota.sThisVfs.mxPathname+1, zFull);
  अगर( rc ){
    sqlite3_free(zFull);
    वापस rc;
  }

  /* Figure out the length of the full pathname.  If the name ends with
  ** / (or \ on windows) then remove the trailing /.
  */
  nFull = म_माप(zFull);
  अगर( nFull>0 && (zFull[nFull-1]=='/' || zFull[nFull-1]=='\\') ){
    nFull--;
    zFull[nFull] = 0;
  }

  quotaEnter();
  pGroup = quotaGroupFind(zFull);
  अगर( pGroup ){
    क्रम(pFile=pGroup->pFiles; pFile && rc==SQLITE_OK; pFile=p_अग्र_File){
      p_अग्र_File = pFile->p_अग्र;
      diff = म_भेदन(zFull, pFile->zFilename, nFull);
      अगर( diff==0 && ((c = pFile->zFilename[nFull])==0 || c=='/' || c=='\\') ){
        अगर( pFile->nRef ){
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
  वापस rc;
}

/***************************** Test Code ***********************************/
#अगर_घोषित SQLITE_TEST
#समावेश "tclsqlite.h"

/*
** Argument passed to a TCL quota-over-limit callback.
*/
प्रकार काष्ठा TclQuotaCallback TclQuotaCallback;
काष्ठा TclQuotaCallback {
  Tcl_Interp *interp;    /* Interpreter in which to run the script */
  Tcl_Obj *pScript;      /* Script to be run */
};

बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);


/*
** This is the callback from a quota-over-limit.
*/
अटल व्योम tclQuotaCallback(
  स्थिर अक्षर *zFilename,          /* Name of file whose size increases */
  sqlite3_int64 *piLimit,         /* IN/OUT: The current limit */
  sqlite3_int64 i_आकार,            /* Total size of all files in the group */
  व्योम *pArg                      /* Client data */
){
  TclQuotaCallback *p;            /* Callback script object */
  Tcl_Obj *pEval;                 /* Script to evaluate */
  Tcl_Obj *pVarname;              /* Name of variable to pass as 2nd arg */
  अचिन्हित पूर्णांक rnd;               /* Random part of pVarname */
  पूर्णांक rc;                         /* Tcl error code */

  p = (TclQuotaCallback *)pArg;
  अगर( p==0 ) वापस;

  pVarname = Tcl_NewStringObj("::piLimit_", -1);
  Tcl_Incr_Ref_गिनती(pVarname);
  sqlite3_randomness(माप(rnd), (व्योम *)&rnd);
  Tcl_AppendObjToObj(pVarname, Tcl_NewIntObj((पूर्णांक)(rnd&0x7FFFFFFF)));
  Tcl_ObjSetVar2(p->interp, pVarname, 0, Tcl_NewWideIntObj(*piLimit), 0);

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_गिनती(pEval);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewStringObj(zFilename, -1));
  Tcl_ListObjAppendElement(0, pEval, pVarname);
  Tcl_ListObjAppendElement(0, pEval, Tcl_NewWideIntObj(i_आकार));
  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);

  अगर( rc==TCL_OK ){
    Tcl_WideInt x;
    Tcl_Obj *pLimit = Tcl_ObjGetVar2(p->interp, pVarname, 0, 0);
    rc = Tcl_GetWideIntFromObj(p->interp, pLimit, &x);
    *piLimit = x;
    Tcl_UnsetVar(p->interp, Tcl_GetString(pVarname), 0);
  }

  Tcl_Decr_Ref_गिनती(pEval);
  Tcl_Decr_Ref_गिनती(pVarname);
  अगर( rc!=TCL_OK ) Tcl_BackgroundError(p->interp);
}

/*
** Destructor for a TCL quota-over-limit callback.
*/
अटल व्योम tclCallbackDestructor(व्योम *pObj){
  TclQuotaCallback *p = (TclQuotaCallback*)pObj;
  अगर( p ){
    Tcl_Decr_Ref_गिनती(p->pScript);
    sqlite3_free((अक्षर *)p);
  }
}

/*
** tclcmd: sqlite3_quota_initialize NAME MAKEDEFAULT
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_initialize(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zName;              /* Name of new quota VFS */
  पूर्णांक makeDefault;                /* True to make the new VFS the default */
  पूर्णांक rc;                         /* Value returned by quota_initialize() */

  /* Process arguments */
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    वापस TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  अगर( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) वापस TCL_ERROR;
  अगर( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_quota_initialize() */
  rc = sqlite3_quota_initialize(zName, makeDefault);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);

  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_shutdown
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_shutdown(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;                         /* Value returned by quota_shutdown() */

  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }

  /* Call sqlite3_quota_shutdown() */
  rc = sqlite3_quota_shutdown();
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);

  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_set PATTERN LIMIT SCRIPT
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_set(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zPattern;           /* File pattern to configure */
  Tcl_WideInt iLimit;             /* Initial quota in bytes */
  Tcl_Obj *pScript;               /* Tcl script to invoke to increase quota */
  पूर्णांक rc;                         /* Value returned by quota_set() */
  TclQuotaCallback *p;            /* Callback object */
  Tcl_आकार nScript;               /* Length of callback script */
  व्योम (*xDestroy)(व्योम*);        /* Optional destructor for pArg */
  व्योम (*xCallback)(स्थिर अक्षर *, sqlite3_int64 *, sqlite3_int64, व्योम *);

  /* Process arguments */
  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "PATTERN LIMIT SCRIPT");
    वापस TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  अगर( Tcl_GetWideIntFromObj(interp, objv[2], &iLimit) ) वापस TCL_ERROR;
  pScript = objv[3];
  Tcl_GetStringFromObj(pScript, &nScript);

  अगर( nScript>0 ){
    /* Allocate a TclQuotaCallback object */
    p = (TclQuotaCallback *)sqlite3_malloc(माप(TclQuotaCallback));
    अगर( !p ){
      Tcl_Set_परिणाम(interp, (अक्षर *)"SQLITE_NOMEM", TCL_STATIC);
      वापस TCL_OK;
    }
    स_रखो(p, 0, माप(TclQuotaCallback));
    p->interp = interp;
    Tcl_Incr_Ref_गिनती(pScript);
    p->pScript = pScript;
    xDestroy = tclCallbackDestructor;
    xCallback = tclQuotaCallback;
  }अन्यथा{
    p = 0;
    xDestroy = 0;
    xCallback = 0;
  }

  /* Invoke sqlite3_quota_set() */
  rc = sqlite3_quota_set(zPattern, iLimit, xCallback, (व्योम*)p, xDestroy);

  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file FILENAME
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_file(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zFilename;          /* File pattern to configure */
  पूर्णांक rc;                         /* Value returned by quota_file() */

  /* Process arguments */
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME");
    वापस TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);

  /* Invoke sqlite3_quota_file() */
  rc = sqlite3_quota_file(zFilename);

  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
  वापस TCL_OK;
}

/*
** tclcmd:  sqlite3_quota_dump
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_dump(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Obj *p_परिणाम;
  Tcl_Obj *pGroupTerm;
  Tcl_Obj *pFileTerm;
  quotaGroup *pGroup;
  quotaFile *pFile;

  p_परिणाम = Tcl_NewObj();
  quotaEnter();
  क्रम(pGroup=gQuota.pGroup; pGroup; pGroup=pGroup->p_अग्र){
    pGroupTerm = Tcl_NewObj();
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewStringObj(pGroup->zPattern, -1));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->iLimit));
    Tcl_ListObjAppendElement(interp, pGroupTerm,
          Tcl_NewWideIntObj(pGroup->i_आकार));
    क्रम(pFile=pGroup->pFiles; pFile; pFile=pFile->p_अग्र){
      पूर्णांक i;
      अक्षर z_अस्थायी[1000];
      pFileTerm = Tcl_NewObj();
      sqlite3_snprintf(माप(z_अस्थायी), z_अस्थायी, "%s", pFile->zFilename);
      क्रम(i=0; z_अस्थायी[i]; i++){ अगर( z_अस्थायी[i]=='\\' ) z_अस्थायी[i] = '/'; }
      Tcl_ListObjAppendElement(interp, pFileTerm,
            Tcl_NewStringObj(z_अस्थायी, -1));
      Tcl_ListObjAppendElement(interp, pFileTerm,
            Tcl_NewWideIntObj(pFile->i_आकार));
      Tcl_ListObjAppendElement(interp, pFileTerm,
            Tcl_NewWideIntObj(pFile->nRef));
      Tcl_ListObjAppendElement(interp, pFileTerm,
            Tcl_NewWideIntObj(pFile->deleteOnClose));
      Tcl_ListObjAppendElement(interp, pGroupTerm, pFileTerm);
    }
    Tcl_ListObjAppendElement(interp, p_परिणाम, pGroupTerm);
  }
  quotaLeave();
  Tcl_Set_Obj_परिणाम(interp, p_परिणाम);
  वापस TCL_OK;
}

/* Defined in test1.c */
बाह्य व्योम *sqlite3TestTextToPtr(स्थिर अक्षर*);
बाह्य स्थिर अक्षर *sqlite3TestPtrToText(व्योम*);

/*
** tclcmd: sqlite3_quota_fopen FILENAME MODE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_fopen(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zFilename;          /* File pattern to configure */
  स्थिर अक्षर *zMode;              /* Mode string */
  quota_FILE *p;                  /* Open string object */

  /* Process arguments */
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME MODE");
    वापस TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  zMode = Tcl_GetString(objv[2]);
  p = sqlite3_quota_fopen(zFilename, zMode);
  Tcl_Set_परिणाम(interp, (अक्षर*)sqlite3TestPtrToText(p), TCL_VOLATILE);
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fread HANDLE SIZE NELEM
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_fread(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  अक्षर *zBuf;
  पूर्णांक sz;
  पूर्णांक nElem;
  माप_प्रकार got;

  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE SIZE NELEM");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  अगर( Tcl_GetIntFromObj(interp, objv[2], &sz) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) वापस TCL_ERROR;
  zBuf = (अक्षर*)sqlite3_malloc( sz*nElem + 1 );
  अगर( zBuf==0 ){
    Tcl_Set_परिणाम(interp, "out of memory", TCL_STATIC);
    वापस TCL_ERROR;
  }
  got = sqlite3_quota_fread(zBuf, sz, nElem, p);
  zBuf[got*sz] = 0;
  Tcl_Set_परिणाम(interp, zBuf, TCL_VOLATILE);
  sqlite3_free(zBuf);
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fwrite HANDLE SIZE NELEM CONTENT
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_fwrite(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  अक्षर *zBuf;
  पूर्णांक sz;
  पूर्णांक nElem;
  माप_प्रकार got;

  अगर( objc!=5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE SIZE NELEM CONTENT");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  अगर( Tcl_GetIntFromObj(interp, objv[2], &sz) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[3], &nElem) ) वापस TCL_ERROR;
  zBuf = Tcl_GetString(objv[4]);
  got = sqlite3_quota_fwrite(zBuf, sz, nElem, p);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewWideIntObj(got));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fclose HANDLE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_fclose(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णांक rc;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  rc = sqlite3_quota_fclose(p);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fflush HANDLE ?HARDSYNC?
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_fflush(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णांक rc;
  पूर्णांक doSync = 0;

  अगर( objc!=2 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE ?HARDSYNC?");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  अगर( objc==3 ){
    अगर( Tcl_GetBooleanFromObj(interp, objv[2], &doSync) ) वापस TCL_ERROR;
  }
  rc = sqlite3_quota_fflush(p, doSync);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_fseek HANDLE OFFSET WHENCE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_fseek(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णांक ofst;
  स्थिर अक्षर *zWhence;
  पूर्णांक whence;
  पूर्णांक rc;

  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE OFFSET WHENCE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  अगर( Tcl_GetIntFromObj(interp, objv[2], &ofst) ) वापस TCL_ERROR;
  zWhence = Tcl_GetString(objv[3]);
  अगर( म_भेद(zWhence, "SEEK_SET")==0 ){
    whence = शुरू_से;
  }अन्यथा अगर( म_भेद(zWhence, "SEEK_CUR")==0 ){
    whence = प्रस्तुत_से;
  }अन्यथा अगर( म_भेद(zWhence, "SEEK_END")==0 ){
    whence = अंत_से;
  }अन्यथा{
    Tcl_Append_परिणाम(interp,
           "WHENCE should be SEEK_SET, SEEK_CUR, or SEEK_END", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  rc = sqlite3_quota_fseek(p, ofst, whence);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_rewind HANDLE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_rewind(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  sqlite3_quota_rewind(p);
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftell HANDLE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_ftell(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ftell(p);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewWideIntObj(x));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ftruncate HANDLE SIZE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_ftruncate(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  Tcl_WideInt w;
  पूर्णांक rc;
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE SIZE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  अगर( Tcl_GetWideIntFromObj(interp, objv[2], &w) ) वापस TCL_ERROR;
  x = (sqlite3_int64)w;
  rc = sqlite3_quota_ftruncate(p, x);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_size HANDLE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_file_आकार(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_आकार(p);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewWideIntObj(x));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_truesize HANDLE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_file_truesize(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_truesize(p);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewWideIntObj(x));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_mtime HANDLE
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_file_mtime(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  समय_प्रकार t;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  t = 0;
  sqlite3_quota_file_mtime(p, &t);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewWideIntObj(t));
  वापस TCL_OK;
}


/*
** tclcmd: sqlite3_quota_remove FILENAME
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_remove(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zFilename;          /* File pattern to configure */
  पूर्णांक rc;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME");
    वापस TCL_ERROR;
  }
  zFilename = Tcl_GetString(objv[1]);
  rc = sqlite3_quota_remove(zFilename);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_glob PATTERN TEXT
**
** Test the glob pattern matching.  Return 1 if TEXT matches PATTERN
** and return 0 if it does not.
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_glob(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zPattern;          /* The glob pattern */
  स्थिर अक्षर *zText;             /* Text to compare against the pattern */
  पूर्णांक rc;
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "PATTERN TEXT");
    वापस TCL_ERROR;
  }
  zPattern = Tcl_GetString(objv[1]);
  zText = Tcl_GetString(objv[2]);
  rc = quotaStrglob(zPattern, zText);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_file_available HANDLE
**
** Return the number of bytes from the current file point to the end of
** the file.
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_file_available(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  sqlite3_int64 x;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_file_available(p);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewWideIntObj(x));
  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_quota_ferror HANDLE
**
** Return true if the file handle is in the error state.
*/
अटल पूर्णांक SQLITE_TCLAPI test_quota_ferror(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  quota_FILE *p;
  पूर्णांक x;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE");
    वापस TCL_ERROR;
  }
  p = sqlite3TestTextToPtr(Tcl_GetString(objv[1]));
  x = sqlite3_quota_ferror(p);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(x));
  वापस TCL_OK;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
पूर्णांक Sqlitequota_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
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
    { "sqlite3_quota_file_size",     test_quota_file_आकार },
    { "sqlite3_quota_file_truesize", test_quota_file_truesize },
    { "sqlite3_quota_file_mtime",    test_quota_file_mtime },
    { "sqlite3_quota_remove",        test_quota_remove },
    { "sqlite3_quota_glob",          test_quota_glob },
    { "sqlite3_quota_file_available",test_quota_file_available },
    { "sqlite3_quota_ferror",        test_quota_ferror },
  };
  पूर्णांक i;

  क्रम(i=0; i<माप(aCmd)/माप(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  वापस TCL_OK;
}
#पूर्ण_अगर
