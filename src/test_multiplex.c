/*
** 2010 October 28
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
** pager and the real VFS - that breaks up a very large database file
** into two or more smaller files on disk.  This is useful, for example,
** in order to support large, multi-gigabyte databases on older filesystems
** that limit the maximum file size to 2 GiB.
**
** USAGE:
**
** Compile this source file and link it with your application.  Then
** at start-time, invoke the following procedure:
**
**   int sqlite3_multiplex_initialize(
**      const char *zOrigVfsName,    // The underlying real VFS
**      int makeDefault              // True to make multiplex the default VFS
**   );
**
** The procedure call above will create and register a new VFS shim named
** "multiplex".  The multiplex VFS will use the VFS named by zOrigVfsName to
** do the actual disk I/O.  (The zOrigVfsName parameter may be NULL, in
** which case the default VFS at the moment sqlite3_multiplex_initialize()
** is called will be used as the underlying real VFS.)
**
** If the makeDefault parameter is TRUE then multiplex becomes the new
** default VFS.  Otherwise, you can use the multiplex VFS by specifying
** "multiplex" as the 4th parameter to sqlite3_open_v2() or by employing
** URI filenames and adding "vfs=multiplex" as a parameter to the filename
** URI.
**
** The multiplex VFS allows databases up to 32 GiB in size.  But it splits
** the files up into smaller pieces, so that they will work even on
** filesystems that do not support large files.  The default chunk size
** is 2147418112 bytes (which is 64KiB less than 2GiB) but this can be
** changed at compile-time by defining the SQLITE_MULTIPLEX_CHUNK_SIZE
** macro.  Use the "chunksize=NNNN" query parameter with a URI filename
** in order to select an alternative chunk size for individual connections
** at run-time.
*/
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ "test_multiplex.h"

#నిర్వచితం_కాకపోతే SQLITE_CORE
  #నిర్వచించు SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#అయితే_ముగింపు
#అన్తర్భావ "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#నిర్వచించు వాడని_PARAMETER(x) (శూన్య)(x)
#నిర్వచించు MAX_పుట_కొలత       0x10000
#నిర్వచించు DEFAULT_SECTOR_కొలత 0x1000

/* Maximum chunk number */
#నిర్వచించు MX_CHUNK_సంఖ్య 299

/* First chunk for rollback journal files */
#నిర్వచించు SQLITE_MULTIPLEX_JOURNAL_8_3_స్థానభ్రంశం 400
#నిర్వచించు SQLITE_MULTIPLEX_WAL_8_3_స్థానభ్రంశం 700


/************************ Shim Definitions ******************************/

#నిర్వచితం_కాకపోతే SQLITE_MULTIPLEX_VFS_NAME
# define SQLITE_MULTIPLEX_VFS_NAME "multiplex"
#అయితే_ముగింపు

/* This is the limit on the chunk size.  It may be changed by calling
** the xFileControl() interface.  It will be rounded up to a
** multiple of MAX_PAGE_SIZE.  We default it here to 2GiB less 64KiB.
*/
#నిర్వచితం_కాకపోతే SQLITE_MULTIPLEX_CHUNK_కొలత
# define SQLITE_MULTIPLEX_CHUNK_కొలత 2147418112
#అయితే_ముగింపు

/* This used to be the default limit on number of chunks, but
** it is no longer enforced. There is currently no limit to the
** number of chunks.
**
** May be changed by calling the xFileControl() interface.
*/
#నిర్వచితం_కాకపోతే SQLITE_MULTIPLEX_MAX_CHUNKS
# define SQLITE_MULTIPLEX_MAX_CHUNKS 12
#అయితే_ముగింపు

/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
రకనిర్వచనం నిర్మాణం multiplex_సమూహం multiplex_సమూహం;
రకనిర్వచనం నిర్మాణం multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
నిర్మాణం multiplex_సమూహం {
  నిర్మాణం multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    అక్షరం *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  పూర్ణాంకం nReal;                       /* Number of chunks */
  అక్షరం *zName;                     /* Base filename of this group */
  పూర్ణాంకం nName;                       /* Length of base filename */
  పూర్ణాంకం flags;                       /* Flags used for original opening */
  గుర్తులేని పూర్ణాంకం szChunk;            /* Chunk size used for this group */
  గుర్తులేని అక్షరం bEnabled;          /* TRUE to use Multiplex VFS for this file */
  గుర్తులేని అక్షరం bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
నిర్మాణం multiplexConn {
  sqlite3_file పునాది;              /* Base class - must be first */
  multiplex_సమూహం *p_సమూహం;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
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

  /* True when this shim has been initialized.
  */
  పూర్ణాంకం isInitialized;
} gMultiplex;

/************************* Utility Routines *********************************/
/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
**
** The value returned will never be negative.  Nor will it ever be greater
** than the actual length of the string.  For very long strings (greater
** than 1GiB) the value returned might be less than the true string length.
*/
స్థిర పూర్ణాంకం multiplexStrlen30(స్థిరరాశి అక్షరం *z){
  స్థిరరాశి అక్షరం *z2 = z;
  ఒకవేళ( z==0 ) తిరిగి 0;
  అయితే( *z2 ){ z2++; }
  తిరిగి 0x3fffffff & (పూర్ణాంకం)(z2 - z);
}

/*
** Generate the file-name for chunk iChunk of the group with base name
** zBase. The file-name is written to buffer zOut before returning. Buffer
** zOut must be allocated by the caller so that it is at least (nBase+5)
** bytes in size, where nBase is the length of zBase, not including the
** nul-terminator.
**
** If iChunk is 0 (or 400 - the number for the first journal file chunk),
** the output is a copy of the input string. Otherwise, if
** SQLITE_ENABLE_8_3_NAMES is not defined or the input buffer does not contain
** a "." character, then the output is a copy of the input string with the
** three-digit zero-padded decimal representation if iChunk appended to it.
** For example:
**
**   zBase="test.db", iChunk=4  ->  zOut="test.db004"
**
** Or, if SQLITE_ENABLE_8_3_NAMES is defined and the input buffer contains
** a "." character, then everything after the "." is replaced by the
** three-digit representation of iChunk.
**
**   zBase="test.db", iChunk=4  ->  zOut="test.004"
**
** The output buffer string is terminated by 2 0x00 bytes. This makes it safe
** to pass to sqlite3_uri_parameter() and similar.
*/
స్థిర శూన్య multiplexFilename(
  స్థిరరాశి అక్షరం *z_పునాది,              /* Filename for chunk 0 */
  పూర్ణాంకం n_పునాది,                      /* Size of zBase in bytes (without \0) */
  పూర్ణాంకం flags,                      /* Flags used to open file */
  పూర్ణాంకం iChunk,                     /* Chunk to generate filename for */
  అక్షరం *z_బహిర్గతం                      /* Buffer to write generated name to */
){
  పూర్ణాంకం n = n_పునాది;
  జ్ఞాపకం_నకలు(z_బహిర్గతం, z_పునాది, n+1);
  ఒకవేళ( iChunk!=0 && iChunk<=MX_CHUNK_సంఖ్య ){
#నిర్వచితమైతే SQLITE_ENABLE_8_3_పేర్లు
    పూర్ణాంకం i;
    కొరకు(i=n-1; i>0 && i>=n-4 && z_బహిర్గతం[i]!='.'; i--){}
    ఒకవేళ( i>=n-4 ) n = i+1;
    ఒకవేళ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_స్థానభ్రంశం;
    }లేకపోతే ఒకవేళ( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_స్థానభ్రంశం;
    }
#అయితే_ముగింపు
    sqlite3_snprintf(4,&z_బహిర్గతం[n],"%03d",iChunk);
    n += 3;
  }

  ప్రతిజ్ఞా( z_బహిర్గతం[n]=='\0' );
  z_బహిర్గతం[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
స్థిర పూర్ణాంకం multiplexSubFilename(multiplex_సమూహం *p_సమూహం, పూర్ణాంకం iChunk){
  ఒకవేళ( iChunk>=p_సమూహం->nReal ){
    నిర్మాణం multiplexReal *p;
    p = sqlite3_realloc64(p_సమూహం->aReal, (iChunk+1)*పరిమాణం(*p));
    ఒకవేళ( p==0 ){
      తిరిగి SQLITE_NOMEM;
    }
    memset(&p[p_సమూహం->nReal], 0, పరిమాణం(p[0])*(iChunk+1-p_సమూహం->nReal));
    p_సమూహం->aReal = p;
    p_సమూహం->nReal = iChunk+1;
  }
  ఒకవేళ( p_సమూహం->zName && p_సమూహం->aReal[iChunk].z==0 ){
    అక్షరం *z;
    పూర్ణాంకం n = p_సమూహం->nName;
    z = sqlite3_malloc64( n+5 );
    ఒకవేళ( z==0 ){
      తిరిగి SQLITE_NOMEM;
    }
    multiplexFilename(p_సమూహం->zName, p_సమూహం->nName, p_సమూహం->flags, iChunk, z);
    p_సమూహం->aReal[iChunk].z = (అక్షరం*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    ఒకవేళ( p_సమూహం->aReal[iChunk].z==0 ) తిరిగి SQLITE_NOMEM;
  }
  తిరిగి SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
స్థిర sqlite3_file *multiplexSubOpen(
  multiplex_సమూహం *p_సమూహం,    /* The multiplexor group */
  పూర్ణాంకం iChunk,                /* Which chunk to open.  0==original file */
  పూర్ణాంకం *rc,                   /* Result code in and out */
  పూర్ణాంకం *p_బహిర్గతం_Flags,            /* Output flags */
  పూర్ణాంకం createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#నిర్వచితమైతే SQLITE_ENABLE_8_3_పేర్లు
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  ఒకవేళ( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_స్థానభ్రంశం ){
    sqlite3_log(SQLITE_నిండిన, "multiplexed chunk overflow: %s", p_సమూహం->zName);
    *rc = SQLITE_నిండిన;
    తిరిగి 0;
  }
#అయితే_ముగింపు

  *rc = multiplexSubFilename(p_సమూహం, iChunk);
  ఒకవేళ( (*rc)==SQLITE_OK && (pSubOpen = p_సమూహం->aReal[iChunk].p)==0 ){
    పూర్ణాంకం flags, bExists;
    flags = p_సమూహం->flags;
    ఒకవేళ( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }లేకపోతే ఒకవేళ( iChunk==0 ){
      /* Fall through */
    }లేకపోతే ఒకవేళ( p_సమూహం->aReal[iChunk].z==0 ){
      తిరిగి 0;
    }లేకపోతే{
      *rc = pOrigVfs->xAccess(pOrigVfs, p_సమూహం->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     ఒకవేళ( *rc || !bExists ){
        ఒకవేళ( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      p_సమూహం->aReal[iChunk].z);
        }
        తిరిగి 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    ఒకవేళ( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      తిరిగి 0;
    }
    p_సమూహం->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, p_సమూహం->aReal[iChunk].z, pSubOpen,
                          flags, p_బహిర్గతం_Flags);
    ఒకవేళ( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  p_సమూహం->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      p_సమూహం->aReal[iChunk].p = 0;
      తిరిగి 0;
    }
  }
  తిరిగి pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
స్థిర sqlite3_int64 multiplex_Sub_కొలత(
  multiplex_సమూహం *p_సమూహం,    /* The multiplexor group */
  పూర్ణాంకం iChunk,                /* Which chunk to open.  0==original file */
  పూర్ణాంకం *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  ఒకవేళ( *rc ) తిరిగి 0;
  pSub = multiplexSubOpen(p_సమూహం, iChunk, rc, ఖాళీ, 0);
  ఒకవేళ( pSub==0 ) తిరిగి 0;
  *rc = pSub->pMethods->x_File_కొలత(pSub, &sz);
  తిరిగి sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
స్థిర శూన్య multiplexControlFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  పూర్ణాంకం op = 0;
  పూర్ణాంకం iVal;

  ఒకవేళ( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }లేకపోతే{
    /* extract params */
    op = sqlite3_విలువ_int(argv[0]);
    iVal = sqlite3_విలువ_int(argv[1]);
    /* map function op to file_control op */
    ఎంపిక( op ){
      స్థితి 1:
        op = MULTIPLEX_CTRL_ENABLE;
        ఆపు;
      స్థితి 2:
        op = MULTIPLEX_CTRL_సమితి_CHUNK_కొలత;
        ఆపు;
      స్థితి 3:
        op = MULTIPLEX_CTRL_సమితి_MAX_CHUNKS;
        ఆపు;
      అప్రమేయ:
        rc = SQLITE_NOTFOUND;
        ఆపు;
    }
  }
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_ఫలితం_error_సంకేతం(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
స్థిర పూర్ణాంకం multiplexFuncInit(
  sqlite3 *db,
  అక్షరం **pz_Err_సందేశం,
  స్థిరరాశి sqlite3_api_routines *pApi
){
  పూర్ణాంకం rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  తిరిగి rc;
}

/*
** Close a single sub-file in the connection group.
*/
స్థిర శూన్య multiplexSubClose(
  multiplex_సమూహం *p_సమూహం,
  పూర్ణాంకం iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = p_సమూహం->aReal[iChunk].p;
  ఒకవేళ( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    ఒకవేళ( pOrigVfs && p_సమూహం->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, p_సమూహం->aReal[iChunk].z, 0);
    }
    sqlite3_free(p_సమూహం->aReal[iChunk].p);
  }
  sqlite3_free_filename(p_సమూహం->aReal[iChunk].z);
  memset(&p_సమూహం->aReal[iChunk], 0, పరిమాణం(p_సమూహం->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
స్థిర శూన్య multiplexFreeComponents(multiplex_సమూహం *p_సమూహం){
  పూర్ణాంకం i;
  కొరకు(i=0; i<p_సమూహం->nReal; i++){ multiplexSubClose(p_సమూహం, i, 0); }
  sqlite3_free(p_సమూహం->aReal);
  p_సమూహం->aReal = 0;
  p_సమూహం->nReal = 0;
}


/************************* VFS Method Wrappers *****************************/

/*
** This is the xOpen method used for the "multiplex" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate multiplex group if it is a
** file that needs to be tracked.
*/
స్థిర పూర్ణాంకం multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  స్థిరరాశి అక్షరం *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  పూర్ణాంకం flags,                 /* Flags to control the opening */
  పూర్ణాంకం *p_బహిర్గతం_Flags             /* Flags showing results of opening */
){
  పూర్ణాంకం rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplex_సమూహం *p_సమూహం = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  పూర్ణాంకం nName = 0;
  పూర్ణాంకం sz = 0;
  అక్షరం *z_గమ్యం_Free = 0;

  వాడని_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  ప్రతిజ్ఞా( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  ఒకవేళ( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = పరిమాణం(multiplex_సమూహం)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    p_సమూహం = sqlite3_malloc64( sz );
    ఒకవేళ( p_సమూహం==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  ఒకవేళ( rc==SQLITE_OK ){
    స్థిరరాశి అక్షరం *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(p_సమూహం, 0, sz);
    pMultiplexOpen->p_సమూహం = p_సమూహం;
    p_సమూహం->bEnabled = (గుర్తులేని అక్షరం)-1;
    p_సమూహం->bTruncate = (గుర్తులేని అక్షరం)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    p_సమూహం->szChunk = (పూర్ణాంకం)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_కొలత);
    p_సమూహం->szChunk = (p_సమూహం->szChunk+0xffff)&~0xffff;
    ఒకవేళ( zName ){
      అక్షరం *p = (అక్షరం *)&p_సమూహం[1];
      p_సమూహం->zName = p;
      జ్ఞాపకం_నకలు(p_సమూహం->zName, zName, nName+1);
      p_సమూహం->nName = nName;
    }
    ఒకవేళ( p_సమూహం->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WSD
      బాహ్య పూర్ణాంకం sqlite3PendingByte;
#లేకపోతే
      పూర్ణాంకం sqlite3PendingByte = 0x40000000;
#అయితే_ముగింపు
      అయితే( (sqlite3PendingByte % p_సమూహం->szChunk)>=(p_సమూహం->szChunk-65536) ){
        p_సమూహం->szChunk += 65536;
      }
    }
    p_సమూహం->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(p_సమూహం, 1);
    ఒకవేళ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(p_సమూహం, 0, &rc, p_బహిర్గతం_Flags, 0);
      ఒకవేళ( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->x_File_కొలత(pSubOpen, &sz64);
      ఒకవేళ( rc==SQLITE_OK && zName ){
        పూర్ణాంకం bExists;
        ఒకవేళ( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          p_సమూహం->bEnabled = 0;
        }లేకపోతే
        ఒకవేళ( sz64==0 ){
          ఒకవేళ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            పూర్ణాంకం iChunk = 1;
            చేయి {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  p_సమూహం->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              ఒకవేళ( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, p_సమూహం->aReal[iChunk].z, 0);
                ఒకవేళ( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(p_సమూహం, ++iChunk);
                }
              }
            }అయితే( rc==SQLITE_OK && bExists );
          }
        }లేకపోతే{
          /* If the first overflow file exists and if the size of the main file
          ** is different from the chunk size, that means the chunk size is set
          ** set incorrectly.  So fix it.
          **
          ** Or, if the first overflow file does not exist and the main file is
          ** larger than the chunk size, that means the chunk size is too small.
          ** But we have no way of determining the intended chunk size, so
          ** just disable the multiplexor all together.
          */
          rc = pOrigVfs->xAccess(pOrigVfs, p_సమూహం->aReal[1].z,
              SQLITE_ACCESS_EXISTS, &bExists);
          bExists = multiplex_Sub_కొలత(p_సమూహం, 1, &rc)>0;
          ఒకవేళ( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=p_సమూహం->szChunk ){
            p_సమూహం->szChunk = (పూర్ణాంకం)sz64;
          }లేకపోతే ఒకవేళ( rc==SQLITE_OK && !bExists && sz64>p_సమూహం->szChunk ){
            p_సమూహం->bEnabled = 0;
          }
        }
      }
    }

    ఒకవేళ( rc==SQLITE_OK ){
      ఒకవేళ( pSubOpen->pMethods->i_సంస్కరణ==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }లేకపోతే{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }లేకపోతే{
      multiplexFreeComponents(p_సమూహం);
      sqlite3_free(p_సమూహం);
    }
  }
  sqlite3_free(z_గమ్యం_Free);
  తిరిగి rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
స్థిర పూర్ణాంకం multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  స్థిరరాశి అక్షరం *zName,         /* Name of file to delete */
  పూర్ణాంకం syncDir
){
  పూర్ణాంకం rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  ఒకవేళ( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    పూర్ణాంకం nName = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zName);
    అక్షరం *z;
    z = sqlite3_malloc64(nName + 5);
    ఒకవేళ( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }లేకపోతే{
      పూర్ణాంకం iChunk = 0;
      పూర్ణాంకం bExists;
      చేయి{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }అయితే( rc==SQLITE_OK && bExists );
      అయితే( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      ఒకవేళ( rc==SQLITE_OK ){
        iChunk = 0;
        చేయి{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }అయితే( rc==SQLITE_OK && bExists );
        అయితే( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  తిరిగి rc;
}

స్థిర పూర్ణాంకం multiplexAccess(sqlite3_vfs *a, స్థిరరాశి అక్షరం *b, పూర్ణాంకం c, పూర్ణాంకం *d){
  తిరిగి gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
స్థిర పూర్ణాంకం multiplex_నిండిన_Pathname(sqlite3_vfs *a, స్థిరరాశి అక్షరం *b, పూర్ణాంకం c, అక్షరం *d){
  తిరిగి gMultiplex.pOrigVfs->x_నిండిన_Pathname(gMultiplex.pOrigVfs, b, c, d);
}
స్థిర శూన్య *multiplexDlOpen(sqlite3_vfs *a, స్థిరరాశి అక్షరం *b){
  తిరిగి gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
స్థిర శూన్య multiplexDlError(sqlite3_vfs *a, పూర్ణాంకం b, అక్షరం *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
స్థిర శూన్య (*multiplexDlSym(sqlite3_vfs *a, శూన్య *b, స్థిరరాశి అక్షరం *c))(శూన్య){
  తిరిగి gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
స్థిర శూన్య multiplexDlClose(sqlite3_vfs *a, శూన్య *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
స్థిర పూర్ణాంకం multiplexRandomness(sqlite3_vfs *a, పూర్ణాంకం b, అక్షరం *c){
  తిరిగి gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
స్థిర పూర్ణాంకం multiplexSleep(sqlite3_vfs *a, పూర్ణాంకం b){
  తిరిగి gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
స్థిర పూర్ణాంకం multiplex_Current_సమయం(sqlite3_vfs *a, ద్విగుణ *b){
  తిరిగి gMultiplex.pOrigVfs->x_Current_సమయం(gMultiplex.pOrigVfs, b);
}
స్థిర పూర్ణాంకం multiplex_Get_చివరి_Error(sqlite3_vfs *a, పూర్ణాంకం b, అక్షరం *c){
  ఒకవేళ( gMultiplex.pOrigVfs->x_Get_చివరి_Error ){
    తిరిగి gMultiplex.pOrigVfs->x_Get_చివరి_Error(gMultiplex.pOrigVfs, b, c);
  }లేకపోతే{
    తిరిగి 0;
  }
}
స్థిర పూర్ణాంకం multiplex_Current_సమయం_Int64(sqlite3_vfs *a, sqlite3_int64 *b){
  తిరిగి gMultiplex.pOrigVfs->x_Current_సమయం_Int64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
స్థిర పూర్ణాంకం multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_సమూహం *p_సమూహం = p->p_సమూహం;
  పూర్ణాంకం rc = SQLITE_OK;
  multiplexFreeComponents(p_సమూహం);
  sqlite3_free(p_సమూహం);
  తిరిగి rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
స్థిర పూర్ణాంకం multiplexRead(
  sqlite3_file *pConn,
  శూన్య *pBuf,
  పూర్ణాంకం iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_సమూహం *p_సమూహం = p->p_సమూహం;
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( !p_సమూహం->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_సమూహం, 0, &rc, ఖాళీ, 0);
    ఒకవేళ( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }లేకపోతే{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }లేకపోతే{
    అయితే( iAmt > 0 ){
      పూర్ణాంకం i = (పూర్ణాంకం)(iOfst / p_సమూహం->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(p_సమూహం, i, &rc, ఖాళీ, 1);
      ఒకవేళ( pSubOpen ){
        పూర్ణాంకం extra = ((పూర్ణాంకం)(iOfst % p_సమూహం->szChunk) + iAmt) - p_సమూహం->szChunk;
        ఒకవేళ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % p_సమూహం->szChunk);
        ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
        pBuf = (అక్షరం *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }లేకపోతే{
        rc = SQLITE_IOERR_READ;
        ఆపు;
      }
    }
  }

  తిరిగి rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
స్థిర పూర్ణాంకం multiplexWrite(
  sqlite3_file *pConn,
  స్థిరరాశి శూన్య *pBuf,
  పూర్ణాంకం iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_సమూహం *p_సమూహం = p->p_సమూహం;
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( !p_సమూహం->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_సమూహం, 0, &rc, ఖాళీ, 0);
    ఒకవేళ( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }లేకపోతే{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }లేకపోతే{
    అయితే( rc==SQLITE_OK && iAmt>0 ){
      పూర్ణాంకం i = (పూర్ణాంకం)(iOfst / p_సమూహం->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(p_సమూహం, i, &rc, ఖాళీ, 1);
      ఒకవేళ( pSubOpen ){
        పూర్ణాంకం extra = ((పూర్ణాంకం)(iOfst % p_సమూహం->szChunk) + iAmt) -
                    p_సమూహం->szChunk;
        ఒకవేళ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % p_సమూహం->szChunk);
        pBuf = (అక్షరం *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  తిరిగి rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
స్థిర పూర్ణాంకం multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 కొలత){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_సమూహం *p_సమూహం = p->p_సమూహం;
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( !p_సమూహం->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_సమూహం, 0, &rc, ఖాళీ, 0);
    ఒకవేళ( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }లేకపోతే{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, కొలత);
    }
  }లేకపోతే{
    పూర్ణాంకం i;
    పూర్ణాంకం i_పునాది_సమూహం = (పూర్ణాంకం)(కొలత / p_సమూహం->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    కొరకు(i = p_సమూహం->nReal-1; i>i_పునాది_సమూహం && rc==SQLITE_OK; i--){
      ఒకవేళ( p_సమూహం->bTruncate ){
        multiplexSubClose(p_సమూహం, i, pOrigVfs);
      }లేకపోతే{
        pSubOpen = multiplexSubOpen(p_సమూహం, i, &rc, 0, 0);
        ఒకవేళ( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    ఒకవేళ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(p_సమూహం, i_పునాది_సమూహం, &rc, 0, 0);
      ఒకవేళ( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, కొలత % p_సమూహం->szChunk);
      }
    }
    ఒకవేళ( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  తిరిగి rc;
}

/* Pass xSync requests through to the original VFS without change
*/
స్థిర పూర్ణాంకం multiplexSync(sqlite3_file *pConn, పూర్ణాంకం flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_సమూహం *p_సమూహం = p->p_సమూహం;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం i;
  కొరకు(i=0; i<p_సమూహం->nReal; i++){
    sqlite3_file *pSubOpen = p_సమూహం->aReal[i].p;
    ఒకవేళ( pSubOpen ){
      పూర్ణాంకం rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      ఒకవేళ( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  తిరిగి rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
స్థిర పూర్ణాంకం multiplex_File_కొలత(sqlite3_file *pConn, sqlite3_int64 *p_కొలత){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_సమూహం *p_సమూహం = p->p_సమూహం;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం i;
  ఒకవేళ( !p_సమూహం->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_సమూహం, 0, &rc, ఖాళీ, 0);
    ఒకవేళ( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }లేకపోతే{
      rc = pSubOpen->pMethods->x_File_కొలత(pSubOpen, p_కొలత);
    }
  }లేకపోతే{
    *p_కొలత = 0;
    కొరకు(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplex_Sub_కొలత(p_సమూహం, i, &rc);
      ఒకవేళ( sz==0 ) ఆపు;
      *p_కొలత = i*(sqlite3_int64)p_సమూహం->szChunk + sz;
    }
  }
  తిరిగి rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం multiplex_తాళం(sqlite3_file *pConn, పూర్ణాంకం తాళం){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen ){
    తిరిగి pSubOpen->pMethods->x_తాళం(pSubOpen, తాళం);
  }
  తిరిగి SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం multiplexUnlock(sqlite3_file *pConn, పూర్ణాంకం తాళం){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen ){
    తిరిగి pSubOpen->pMethods->xUnlock(pSubOpen, తాళం);
  }
  తిరిగి SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం multiplex_Check_Reserved_తాళం(sqlite3_file *pConn, పూర్ణాంకం *p_Res_బహిర్గతం){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen ){
    తిరిగి pSubOpen->pMethods->x_Check_Reserved_తాళం(pSubOpen, p_Res_బహిర్గతం);
  }
  తిరిగి SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
స్థిర పూర్ణాంకం multiplexFileControl(sqlite3_file *pConn, పూర్ణాంకం op, శూన్య *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_సమూహం *p_సమూహం = p->p_సమూహం;
  పూర్ణాంకం rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  ఒకవేళ( !gMultiplex.isInitialized ) తిరిగి SQLITE_MISUSE;
  ఎంపిక( op ){
    స్థితి MULTIPLEX_CTRL_ENABLE:
      ఒకవేళ( pArg ) {
        పూర్ణాంకం bEnabled = *(పూర్ణాంకం *)pArg;
        p_సమూహం->bEnabled = (గుర్తులేని అక్షరం)bEnabled;
        rc = SQLITE_OK;
      }
      ఆపు;
    స్థితి MULTIPLEX_CTRL_సమితి_CHUNK_కొలత:
      ఒకవేళ( pArg ) {
        గుర్తులేని పూర్ణాంకం szChunk = *(గుర్తులేని*)pArg;
        ఒకవేళ( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }లేకపోతే{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_పుట_కొలత-1));
          szChunk &= ~(MAX_పుట_కొలత-1);
          p_సమూహం->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      ఆపు;
    స్థితి MULTIPLEX_CTRL_సమితి_MAX_CHUNKS:
      rc = SQLITE_OK;
      ఆపు;
    స్థితి SQLITE_FCNTL_కొలత_HINT:
    స్థితి SQLITE_FCNTL_CHUNK_కొలత:
      /* no-op these */
      rc = SQLITE_OK;
      ఆపు;
    స్థితి SQLITE_FCNTL_PRAGMA: {
      అక్షరం **aFcntl = (అక్షరం**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      ఒకవేళ( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (శూన్య)multiplex_File_కొలత(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        ఒకవేళ( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          ఒకవేళ( aFcntl[2] && aFcntl[2][0] ){
            ఒకవేళ( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              p_సమూహం->bTruncate = 1;
            }లేకపోతే
            ఒకవేళ( sqlite3_stricmp(aFcntl[2], "off")==0
             || sqlite3_stricmp(aFcntl[2], "0")==0 ){
              p_సమూహం->bTruncate = 0;
            }
          }
          /* EVIDENCE-OF: R-27806-26076 The handler for an SQLITE_FCNTL_PRAGMA
          ** file control can optionally make the first element of the char**
          ** argument point to a string obtained from sqlite3_mprintf() or the
          ** equivalent and that string will become the result of the pragma
          ** or the error message if the pragma fails.
          */
          aFcntl[0] = sqlite3_mprintf(p_సమూహం->bTruncate ? "on" : "off");
          rc = SQLITE_OK;
          ఆపు;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        ఒకవేళ( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", p_సమూహం->bEnabled!=0);
          rc = SQLITE_OK;
          ఆపు;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        ఒకవేళ( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && p_సమూహం->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", p_సమూహం->szChunk);
          rc = SQLITE_OK;
          ఆపు;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        ఒకవేళ( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          పూర్ణాంకం n = 0;
          పూర్ణాంకం ii;
          కొరకు(ii=0; ii<p_సమూహం->nReal; ii++){
            ఒకవేళ( p_సమూహం->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          ఆపు;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    అప్రమేయ:
      pSubOpen = multiplexSubOpen(p_సమూహం, 0, &rc, ఖాళీ, 0);
      ఒకవేళ( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        ఒకవేళ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(అక్షరం**)pArg = sqlite3_mprintf("multiplex/%z", *(అక్షరం**)pArg);
        }
      }
      ఆపు;
  }
  తిరిగి rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం multiplex_Sector_కొలత(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen && pSubOpen->pMethods->x_Sector_కొలత ){
    తిరిగి pSubOpen->pMethods->x_Sector_కొలత(pSubOpen);
  }
  తిరిగి DEFAULT_SECTOR_కొలత;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen ){
    తిరిగి pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  తిరిగి 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  పూర్ణాంకం iRegion,                    /* Region to retrieve */
  పూర్ణాంకం szRegion,                   /* Size of regions */
  పూర్ణాంకం bExtend,                    /* True to extend file if necessary */
  శూన్య అస్థిర **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen ){
    తిరిగి pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  తిరిగి SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం multiplex_Shm_తాళం(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  పూర్ణాంకం ofst,                  /* First lock to acquire or release */
  పూర్ణాంకం n,                     /* Number of locks to acquire or release */
  పూర్ణాంకం flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen ){
    తిరిగి pSubOpen->pMethods->x_Shm_తాళం(pSubOpen, ofst, n, flags);
  }
  తిరిగి SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
స్థిర శూన్య multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
స్థిర పూర్ణాంకం multiplexShmUnmap(sqlite3_file *pConn, పూర్ణాంకం deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  పూర్ణాంకం rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_సమూహం, 0, &rc, ఖాళీ, 0);
  ఒకవేళ( pSubOpen ){
    తిరిగి pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  తిరిగి SQLITE_OK;
}

/************************** Public Interfaces *****************************/
/*
** CAPI: Initialize the multiplex VFS shim - sqlite3_multiplex_initialize()
**
** Use the VFS named zOrigVfsName as the VFS that does the actual work.
** Use the default if zOrigVfsName==NULL.
**
** The multiplex VFS shim is named "multiplex".  It will become the default
** VFS if makeDefault is non-zero.
**
** THIS ROUTINE IS NOT THREADSAFE.  Call this routine exactly once
** during start-up.
*/
పూర్ణాంకం sqlite3_multiplex_initialize(స్థిరరాశి అక్షరం *zOrigVfsName, పూర్ణాంకం makeDefault){
  sqlite3_vfs *pOrigVfs;
  ఒకవేళ( gMultiplex.isInitialized ) తిరిగి SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_కనుగొను(zOrigVfsName);
  ఒకవేళ( pOrigVfs==0 ) తిరిగి SQLITE_ERROR;
  ప్రతిజ్ఞా( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += పరిమాణం(multiplexConn);
  gMultiplex.sThisVfs.zName = SQLITE_MULTIPLEX_VFS_NAME;
  gMultiplex.sThisVfs.xOpen = multiplexOpen;
  gMultiplex.sThisVfs.xDelete = multiplexDelete;
  gMultiplex.sThisVfs.xAccess = multiplexAccess;
  gMultiplex.sThisVfs.x_నిండిన_Pathname = multiplex_నిండిన_Pathname;
  gMultiplex.sThisVfs.xDlOpen = multiplexDlOpen;
  gMultiplex.sThisVfs.xDlError = multiplexDlError;
  gMultiplex.sThisVfs.xDlSym = multiplexDlSym;
  gMultiplex.sThisVfs.xDlClose = multiplexDlClose;
  gMultiplex.sThisVfs.xRandomness = multiplexRandomness;
  gMultiplex.sThisVfs.xSleep = multiplexSleep;
  gMultiplex.sThisVfs.x_Current_సమయం = multiplex_Current_సమయం;
  gMultiplex.sThisVfs.x_Get_చివరి_Error = multiplex_Get_చివరి_Error;
  gMultiplex.sThisVfs.x_Current_సమయం_Int64 = multiplex_Current_సమయం_Int64;

  gMultiplex.sIoMethodsV1.i_సంస్కరణ = 1;
  gMultiplex.sIoMethodsV1.xClose = multiplexClose;
  gMultiplex.sIoMethodsV1.xRead = multiplexRead;
  gMultiplex.sIoMethodsV1.xWrite = multiplexWrite;
  gMultiplex.sIoMethodsV1.xTruncate = multiplexTruncate;
  gMultiplex.sIoMethodsV1.xSync = multiplexSync;
  gMultiplex.sIoMethodsV1.x_File_కొలత = multiplex_File_కొలత;
  gMultiplex.sIoMethodsV1.x_తాళం = multiplex_తాళం;
  gMultiplex.sIoMethodsV1.xUnlock = multiplexUnlock;
  gMultiplex.sIoMethodsV1.x_Check_Reserved_తాళం = multiplex_Check_Reserved_తాళం;
  gMultiplex.sIoMethodsV1.xFileControl = multiplexFileControl;
  gMultiplex.sIoMethodsV1.x_Sector_కొలత = multiplex_Sector_కొలత;
  gMultiplex.sIoMethodsV1.xDeviceCharacteristics =
                                            multiplexDeviceCharacteristics;
  gMultiplex.sIoMethodsV2 = gMultiplex.sIoMethodsV1;
  gMultiplex.sIoMethodsV2.i_సంస్కరణ = 2;
  gMultiplex.sIoMethodsV2.xShmMap = multiplexShmMap;
  gMultiplex.sIoMethodsV2.x_Shm_తాళం = multiplex_Shm_తాళం;
  gMultiplex.sIoMethodsV2.xShmBarrier = multiplexShmBarrier;
  gMultiplex.sIoMethodsV2.xShmUnmap = multiplexShmUnmap;
  sqlite3_vfs_రిజిస్టరు(&gMultiplex.sThisVfs, makeDefault);

  sqlite3_auto_extension((శూన్య(*)(శూన్య))multiplexFuncInit);

  తిరిగి SQLITE_OK;
}

/*
** CAPI: Shutdown the multiplex system - sqlite3_multiplex_shutdown()
**
** All SQLite database connections must be closed before calling this
** routine.
**
** THIS ROUTINE IS NOT THREADSAFE.  Call this routine exactly once while
** shutting down in order to free all remaining multiplex groups.
*/
పూర్ణాంకం sqlite3_multiplex_shutdown(పూర్ణాంకం eForce){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( gMultiplex.isInitialized==0 ) తిరిగి SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, పరిమాణం(gMultiplex));
  తిరిగి rc;
}

/***************************** Test Code ***********************************/
#నిర్వచితమైతే SQLITE_TEST
#అన్తర్భావ "tclsqlite.h"
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_multiplex_initialize(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *zName;              /* Name of new multiplex VFS */
  పూర్ణాంకం makeDefault;                /* True to make the new VFS the default */
  పూర్ణాంకం rc;                         /* Value returned by multiplex_initialize() */

  వాడని_PARAMETER(client_దత్తాంశం);

  /* Process arguments */
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "NAME MAKEDEFAULT");
    తిరిగి TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) తిరిగి TCL_ERROR;
  ఒకవేళ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);

  తిరిగి TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_multiplex_shutdown(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;                         /* Value returned by multiplex_shutdown() */

  వాడని_PARAMETER(client_దత్తాంశం);

  ఒకవేళ( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  ఒకవేళ( (objc!=1 && objc!=2) ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "?-force?");
    తిరిగి TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);

  తిరిగి TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_multiplex_control(
  Client_దత్తాంశం cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;                         /* Return code from file_control() */
  పూర్ణాంకం idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  పూర్ణాంకం i_విలువ = 0;
  శూన్య *pArg = 0;

  నిర్మాణం SubCommand {
    స్థిరరాశి అక్షరం *zName;
    పూర్ణాంకం op;
    పూర్ణాంకం argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_సమితి_CHUNK_కొలత,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_సమితి_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  ఒకవేళ( objc!=5 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_Append_ఫలితం(interp, "expected database handle, got \"", ఖాళీ);
    Tcl_Append_ఫలితం(interp, Tcl_GetString(objv[1]), "\"", ఖాళీ);
    తిరిగి TCL_ERROR;
  }లేకపోతే{
    db = *(sqlite3 **)cmdInfo.obj_Client_దత్తాంశం;
  }

  rc = Tcl_Get_సూచీ_From_Obj_Struct(
      interp, objv[3], aSub, పరిమాణం(aSub[0]), "sub-command", 0, &idx
  );
  ఒకవేళ( rc!=TCL_OK ) తిరిగి rc;

  ఎంపిక( aSub[idx].argtype ){
    స్థితి 1:
      ఒకవేళ( Tcl_GetIntFromObj(interp, objv[4], &i_విలువ) ){
        తిరిగి TCL_ERROR;
      }
      pArg = (శూన్య *)&i_విలువ;
      ఆపు;
    అప్రమేయ:
      Tcl_Wrong_అంకె_వాదనలు(interp, 4, objv, "SUB-COMMAND");
      తిరిగి TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);
  తిరిగి (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
పూర్ణాంకం Sqlitemultiplex_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  పూర్ణాంకం i;

  కొరకు(i=0; i<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  తిరిగి TCL_OK;
}
#అయితే_ముగింపు
