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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "test_multiplex.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_CORE
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਣਵਰਤਿਆ_PARAMETER(x) (ਖਾਲੀ)(x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ਪੰਨਾ_ਮਾਪ       0x10000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_SECTOR_ਮਾਪ 0x1000

/* Maximum chunk number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MX_CHUNK_ਸੰਖਿਆ 299

/* First chunk for rollback journal files */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MULTIPLEX_JOURNAL_8_3_ਵਿੱਥ 400
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MULTIPLEX_WAL_8_3_ਵਿੱਥ 700


/************************ Shim Definitions ******************************/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MULTIPLEX_VFS_NAME
# define SQLITE_MULTIPLEX_VFS_NAME "multiplex"
#ਜੇ_ਅੰਤ

/* This is the limit on the chunk size.  It may be changed by calling
** the xFileControl() interface.  It will be rounded up to a
** multiple of MAX_PAGE_SIZE.  We default it here to 2GiB less 64KiB.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MULTIPLEX_CHUNK_ਮਾਪ
# define SQLITE_MULTIPLEX_CHUNK_ਮਾਪ 2147418112
#ਜੇ_ਅੰਤ

/* This used to be the default limit on number of chunks, but
** it is no longer enforced. There is currently no limit to the
** number of chunks.
**
** May be changed by calling the xFileControl() interface.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MULTIPLEX_MAX_CHUNKS
# define SQLITE_MULTIPLEX_MAX_CHUNKS 12
#ਜੇ_ਅੰਤ

/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ multiplex_ਸਮੂਹ multiplex_ਸਮੂਹ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
ਬਣਤਰ multiplex_ਸਮੂਹ {
  ਬਣਤਰ multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    ਅੱਖਰ *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  ਪੂਰਨ nReal;                       /* Number of chunks */
  ਅੱਖਰ *zName;                     /* Base filename of this group */
  ਪੂਰਨ nName;                       /* Length of base filename */
  ਪੂਰਨ flags;                       /* Flags used for original opening */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ szChunk;            /* Chunk size used for this group */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ bEnabled;          /* TRUE to use Multiplex VFS for this file */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ਬਣਤਰ multiplexConn {
  sqlite3_file ਅਧਾਰ;              /* Base class - must be first */
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
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

  /* True when this shim has been initialized.
  */
  ਪੂਰਨ isInitialized;
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
ਸਥਿਰ ਪੂਰਨ multiplexStrlen30(ਅਚਲ ਅੱਖਰ *z){
  ਅਚਲ ਅੱਖਰ *z2 = z;
  ਜੇ( z==0 ) ਵਾਪਸ 0;
  ਜਦੋਂ( *z2 ){ z2++; }
  ਵਾਪਸ 0x3fffffff & (ਪੂਰਨ)(z2 - z);
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
ਸਥਿਰ ਖਾਲੀ multiplexFilename(
  ਅਚਲ ਅੱਖਰ *z_ਅਧਾਰ,              /* Filename for chunk 0 */
  ਪੂਰਨ n_ਅਧਾਰ,                      /* Size of zBase in bytes (without \0) */
  ਪੂਰਨ flags,                      /* Flags used to open file */
  ਪੂਰਨ iChunk,                     /* Chunk to generate filename for */
  ਅੱਖਰ *z_ਬਾਹਰਲਾ                      /* Buffer to write generated name to */
){
  ਪੂਰਨ n = n_ਅਧਾਰ;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_ਬਾਹਰਲਾ, z_ਅਧਾਰ, n+1);
  ਜੇ( iChunk!=0 && iChunk<=MX_CHUNK_ਸੰਖਿਆ ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_8_3_ਨਾਂ
    ਪੂਰਨ i;
    ਲਈ(i=n-1; i>0 && i>=n-4 && z_ਬਾਹਰਲਾ[i]!='.'; i--){}
    ਜੇ( i>=n-4 ) n = i+1;
    ਜੇ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_ਵਿੱਥ;
    }ਨਹੀਂਤਾਂ ਜੇ( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_ਵਿੱਥ;
    }
#ਜੇ_ਅੰਤ
    sqlite3_snprintf(4,&z_ਬਾਹਰਲਾ[n],"%03d",iChunk);
    n += 3;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਬਾਹਰਲਾ[n]=='\0' );
  z_ਬਾਹਰਲਾ[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
ਸਥਿਰ ਪੂਰਨ multiplexSubFilename(multiplex_ਸਮੂਹ *p_ਸਮੂਹ, ਪੂਰਨ iChunk){
  ਜੇ( iChunk>=p_ਸਮੂਹ->nReal ){
    ਬਣਤਰ multiplexReal *p;
    p = sqlite3_realloc64(p_ਸਮੂਹ->aReal, (iChunk+1)*ਆਕਾਰ(*p));
    ਜੇ( p==0 ){
      ਵਾਪਸ SQLITE_NOMEM;
    }
    memset(&p[p_ਸਮੂਹ->nReal], 0, ਆਕਾਰ(p[0])*(iChunk+1-p_ਸਮੂਹ->nReal));
    p_ਸਮੂਹ->aReal = p;
    p_ਸਮੂਹ->nReal = iChunk+1;
  }
  ਜੇ( p_ਸਮੂਹ->zName && p_ਸਮੂਹ->aReal[iChunk].z==0 ){
    ਅੱਖਰ *z;
    ਪੂਰਨ n = p_ਸਮੂਹ->nName;
    z = sqlite3_malloc64( n+5 );
    ਜੇ( z==0 ){
      ਵਾਪਸ SQLITE_NOMEM;
    }
    multiplexFilename(p_ਸਮੂਹ->zName, p_ਸਮੂਹ->nName, p_ਸਮੂਹ->flags, iChunk, z);
    p_ਸਮੂਹ->aReal[iChunk].z = (ਅੱਖਰ*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    ਜੇ( p_ਸਮੂਹ->aReal[iChunk].z==0 ) ਵਾਪਸ SQLITE_NOMEM;
  }
  ਵਾਪਸ SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
ਸਥਿਰ sqlite3_file *multiplexSubOpen(
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ,    /* The multiplexor group */
  ਪੂਰਨ iChunk,                /* Which chunk to open.  0==original file */
  ਪੂਰਨ *rc,                   /* Result code in and out */
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags,            /* Output flags */
  ਪੂਰਨ createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_8_3_ਨਾਂ
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  ਜੇ( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_ਵਿੱਥ ){
    sqlite3_log(SQLITE_ਭਰਿਆ, "multiplexed chunk overflow: %s", p_ਸਮੂਹ->zName);
    *rc = SQLITE_ਭਰਿਆ;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ

  *rc = multiplexSubFilename(p_ਸਮੂਹ, iChunk);
  ਜੇ( (*rc)==SQLITE_OK && (pSubOpen = p_ਸਮੂਹ->aReal[iChunk].p)==0 ){
    ਪੂਰਨ flags, bExists;
    flags = p_ਸਮੂਹ->flags;
    ਜੇ( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }ਨਹੀਂਤਾਂ ਜੇ( iChunk==0 ){
      /* Fall through */
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਸਮੂਹ->aReal[iChunk].z==0 ){
      ਵਾਪਸ 0;
    }ਨਹੀਂਤਾਂ{
      *rc = pOrigVfs->xAccess(pOrigVfs, p_ਸਮੂਹ->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     ਜੇ( *rc || !bExists ){
        ਜੇ( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      p_ਸਮੂਹ->aReal[iChunk].z);
        }
        ਵਾਪਸ 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    ਜੇ( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      ਵਾਪਸ 0;
    }
    p_ਸਮੂਹ->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, p_ਸਮੂਹ->aReal[iChunk].z, pSubOpen,
                          flags, p_ਬਾਹਰਲਾ_Flags);
    ਜੇ( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  p_ਸਮੂਹ->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      p_ਸਮੂਹ->aReal[iChunk].p = 0;
      ਵਾਪਸ 0;
    }
  }
  ਵਾਪਸ pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
ਸਥਿਰ sqlite3_int64 multiplex_Sub_ਮਾਪ(
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ,    /* The multiplexor group */
  ਪੂਰਨ iChunk,                /* Which chunk to open.  0==original file */
  ਪੂਰਨ *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  ਜੇ( *rc ) ਵਾਪਸ 0;
  pSub = multiplexSubOpen(p_ਸਮੂਹ, iChunk, rc, ਨਿਰਾ, 0);
  ਜੇ( pSub==0 ) ਵਾਪਸ 0;
  *rc = pSub->pMethods->x_File_ਮਾਪ(pSub, &sz);
  ਵਾਪਸ sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
ਸਥਿਰ ਖਾਲੀ multiplexControlFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ਪੂਰਨ op = 0;
  ਪੂਰਨ iVal;

  ਜੇ( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    /* extract params */
    op = sqlite3_ਮੁੱਲ_int(argv[0]);
    iVal = sqlite3_ਮੁੱਲ_int(argv[1]);
    /* map function op to file_control op */
    ਚੋਣ( op ){
      ਹਾਲਤ 1:
        op = MULTIPLEX_CTRL_ENABLE;
        ਰੋਕੋ;
      ਹਾਲਤ 2:
        op = MULTIPLEX_CTRL_ਸੈੱਟ_CHUNK_ਮਾਪ;
        ਰੋਕੋ;
      ਹਾਲਤ 3:
        op = MULTIPLEX_CTRL_ਸੈੱਟ_MAX_CHUNKS;
        ਰੋਕੋ;
      ਮੂਲ:
        rc = SQLITE_NOTFOUND;
        ਰੋਕੋ;
    }
  }
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
ਸਥਿਰ ਪੂਰਨ multiplexFuncInit(
  sqlite3 *db,
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ,
  ਅਚਲ sqlite3_api_routines *pApi
){
  ਪੂਰਨ rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  ਵਾਪਸ rc;
}

/*
** Close a single sub-file in the connection group.
*/
ਸਥਿਰ ਖਾਲੀ multiplexSubClose(
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ,
  ਪੂਰਨ iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = p_ਸਮੂਹ->aReal[iChunk].p;
  ਜੇ( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    ਜੇ( pOrigVfs && p_ਸਮੂਹ->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, p_ਸਮੂਹ->aReal[iChunk].z, 0);
    }
    sqlite3_free(p_ਸਮੂਹ->aReal[iChunk].p);
  }
  sqlite3_free_filename(p_ਸਮੂਹ->aReal[iChunk].z);
  memset(&p_ਸਮੂਹ->aReal[iChunk], 0, ਆਕਾਰ(p_ਸਮੂਹ->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
ਸਥਿਰ ਖਾਲੀ multiplexFreeComponents(multiplex_ਸਮੂਹ *p_ਸਮੂਹ){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<p_ਸਮੂਹ->nReal; i++){ multiplexSubClose(p_ਸਮੂਹ, i, 0); }
  sqlite3_free(p_ਸਮੂਹ->aReal);
  p_ਸਮੂਹ->aReal = 0;
  p_ਸਮੂਹ->nReal = 0;
}


/************************* VFS Method Wrappers *****************************/

/*
** This is the xOpen method used for the "multiplex" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate multiplex group if it is a
** file that needs to be tracked.
*/
ਸਥਿਰ ਪੂਰਨ multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ਅਚਲ ਅੱਖਰ *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  ਪੂਰਨ flags,                 /* Flags to control the opening */
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags             /* Flags showing results of opening */
){
  ਪੂਰਨ rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  ਪੂਰਨ nName = 0;
  ਪੂਰਨ sz = 0;
  ਅੱਖਰ *z_ਮੰਜ਼ਿਲ_Free = 0;

  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  ਪ੍ਰਤਿਜ੍ਞਾ( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  ਜੇ( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = ਆਕਾਰ(multiplex_ਸਮੂਹ)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    p_ਸਮੂਹ = sqlite3_malloc64( sz );
    ਜੇ( p_ਸਮੂਹ==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  ਜੇ( rc==SQLITE_OK ){
    ਅਚਲ ਅੱਖਰ *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(p_ਸਮੂਹ, 0, sz);
    pMultiplexOpen->p_ਸਮੂਹ = p_ਸਮੂਹ;
    p_ਸਮੂਹ->bEnabled = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)-1;
    p_ਸਮੂਹ->bTruncate = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    p_ਸਮੂਹ->szChunk = (ਪੂਰਨ)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_ਮਾਪ);
    p_ਸਮੂਹ->szChunk = (p_ਸਮੂਹ->szChunk+0xffff)&~0xffff;
    ਜੇ( zName ){
      ਅੱਖਰ *p = (ਅੱਖਰ *)&p_ਸਮੂਹ[1];
      p_ਸਮੂਹ->zName = p;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਸਮੂਹ->zName, zName, nName+1);
      p_ਸਮੂਹ->nName = nName;
    }
    ਜੇ( p_ਸਮੂਹ->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
      ਬਾਹਰੀ ਪੂਰਨ sqlite3PendingByte;
#ਨਹੀਂਤਾਂ
      ਪੂਰਨ sqlite3PendingByte = 0x40000000;
#ਜੇ_ਅੰਤ
      ਜਦੋਂ( (sqlite3PendingByte % p_ਸਮੂਹ->szChunk)>=(p_ਸਮੂਹ->szChunk-65536) ){
        p_ਸਮੂਹ->szChunk += 65536;
      }
    }
    p_ਸਮੂਹ->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(p_ਸਮੂਹ, 1);
    ਜੇ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, 0, &rc, p_ਬਾਹਰਲਾ_Flags, 0);
      ਜੇ( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->x_File_ਮਾਪ(pSubOpen, &sz64);
      ਜੇ( rc==SQLITE_OK && zName ){
        ਪੂਰਨ bExists;
        ਜੇ( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          p_ਸਮੂਹ->bEnabled = 0;
        }ਨਹੀਂਤਾਂ
        ਜੇ( sz64==0 ){
          ਜੇ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            ਪੂਰਨ iChunk = 1;
            ਕਰੋ {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  p_ਸਮੂਹ->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              ਜੇ( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, p_ਸਮੂਹ->aReal[iChunk].z, 0);
                ਜੇ( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(p_ਸਮੂਹ, ++iChunk);
                }
              }
            }ਜਦੋਂ( rc==SQLITE_OK && bExists );
          }
        }ਨਹੀਂਤਾਂ{
          /* If the first overflow file exists and if the size of the main file
          ** is different from the chunk size, that means the chunk size is set
          ** set incorrectly.  So fix it.
          **
          ** Or, if the first overflow file does not exist and the main file is
          ** larger than the chunk size, that means the chunk size is too small.
          ** But we have no way of determining the intended chunk size, so
          ** just disable the multiplexor all together.
          */
          rc = pOrigVfs->xAccess(pOrigVfs, p_ਸਮੂਹ->aReal[1].z,
              SQLITE_ACCESS_EXISTS, &bExists);
          bExists = multiplex_Sub_ਮਾਪ(p_ਸਮੂਹ, 1, &rc)>0;
          ਜੇ( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=p_ਸਮੂਹ->szChunk ){
            p_ਸਮੂਹ->szChunk = (ਪੂਰਨ)sz64;
          }ਨਹੀਂਤਾਂ ਜੇ( rc==SQLITE_OK && !bExists && sz64>p_ਸਮੂਹ->szChunk ){
            p_ਸਮੂਹ->bEnabled = 0;
          }
        }
      }
    }

    ਜੇ( rc==SQLITE_OK ){
      ਜੇ( pSubOpen->pMethods->i_ਸੰਸਕਰਣ==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }ਨਹੀਂਤਾਂ{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }ਨਹੀਂਤਾਂ{
      multiplexFreeComponents(p_ਸਮੂਹ);
      sqlite3_free(p_ਸਮੂਹ);
    }
  }
  sqlite3_free(z_ਮੰਜ਼ਿਲ_Free);
  ਵਾਪਸ rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
ਸਥਿਰ ਪੂਰਨ multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ਅਚਲ ਅੱਖਰ *zName,         /* Name of file to delete */
  ਪੂਰਨ syncDir
){
  ਪੂਰਨ rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  ਜੇ( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    ਪੂਰਨ nName = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zName);
    ਅੱਖਰ *z;
    z = sqlite3_malloc64(nName + 5);
    ਜੇ( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ iChunk = 0;
      ਪੂਰਨ bExists;
      ਕਰੋ{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }ਜਦੋਂ( rc==SQLITE_OK && bExists );
      ਜਦੋਂ( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      ਜੇ( rc==SQLITE_OK ){
        iChunk = 0;
        ਕਰੋ{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }ਜਦੋਂ( rc==SQLITE_OK && bExists );
        ਜਦੋਂ( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ multiplexAccess(sqlite3_vfs *a, ਅਚਲ ਅੱਖਰ *b, ਪੂਰਨ c, ਪੂਰਨ *d){
  ਵਾਪਸ gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
ਸਥਿਰ ਪੂਰਨ multiplex_ਭਰਿਆ_Pathname(sqlite3_vfs *a, ਅਚਲ ਅੱਖਰ *b, ਪੂਰਨ c, ਅੱਖਰ *d){
  ਵਾਪਸ gMultiplex.pOrigVfs->x_ਭਰਿਆ_Pathname(gMultiplex.pOrigVfs, b, c, d);
}
ਸਥਿਰ ਖਾਲੀ *multiplexDlOpen(sqlite3_vfs *a, ਅਚਲ ਅੱਖਰ *b){
  ਵਾਪਸ gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
ਸਥਿਰ ਖਾਲੀ multiplexDlError(sqlite3_vfs *a, ਪੂਰਨ b, ਅੱਖਰ *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
ਸਥਿਰ ਖਾਲੀ (*multiplexDlSym(sqlite3_vfs *a, ਖਾਲੀ *b, ਅਚਲ ਅੱਖਰ *c))(ਖਾਲੀ){
  ਵਾਪਸ gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
ਸਥਿਰ ਖਾਲੀ multiplexDlClose(sqlite3_vfs *a, ਖਾਲੀ *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
ਸਥਿਰ ਪੂਰਨ multiplexRandomness(sqlite3_vfs *a, ਪੂਰਨ b, ਅੱਖਰ *c){
  ਵਾਪਸ gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
ਸਥਿਰ ਪੂਰਨ multiplexSleep(sqlite3_vfs *a, ਪੂਰਨ b){
  ਵਾਪਸ gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
ਸਥਿਰ ਪੂਰਨ multiplex_Current_ਸਮਾਂ(sqlite3_vfs *a, ਦੁੱਗਣਾ *b){
  ਵਾਪਸ gMultiplex.pOrigVfs->x_Current_ਸਮਾਂ(gMultiplex.pOrigVfs, b);
}
ਸਥਿਰ ਪੂਰਨ multiplex_Get_ਆਖਰੀ_Error(sqlite3_vfs *a, ਪੂਰਨ b, ਅੱਖਰ *c){
  ਜੇ( gMultiplex.pOrigVfs->x_Get_ਆਖਰੀ_Error ){
    ਵਾਪਸ gMultiplex.pOrigVfs->x_Get_ਆਖਰੀ_Error(gMultiplex.pOrigVfs, b, c);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}
ਸਥਿਰ ਪੂਰਨ multiplex_Current_ਸਮਾਂ_Int64(sqlite3_vfs *a, sqlite3_int64 *b){
  ਵਾਪਸ gMultiplex.pOrigVfs->x_Current_ਸਮਾਂ_Int64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
ਸਥਿਰ ਪੂਰਨ multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ = p->p_ਸਮੂਹ;
  ਪੂਰਨ rc = SQLITE_OK;
  multiplexFreeComponents(p_ਸਮੂਹ);
  sqlite3_free(p_ਸਮੂਹ);
  ਵਾਪਸ rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
ਸਥਿਰ ਪੂਰਨ multiplexRead(
  sqlite3_file *pConn,
  ਖਾਲੀ *pBuf,
  ਪੂਰਨ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ = p->p_ਸਮੂਹ;
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( !p_ਸਮੂਹ->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
    ਜੇ( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }ਨਹੀਂਤਾਂ{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ਨਹੀਂਤਾਂ{
    ਜਦੋਂ( iAmt > 0 ){
      ਪੂਰਨ i = (ਪੂਰਨ)(iOfst / p_ਸਮੂਹ->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, i, &rc, ਨਿਰਾ, 1);
      ਜੇ( pSubOpen ){
        ਪੂਰਨ extra = ((ਪੂਰਨ)(iOfst % p_ਸਮੂਹ->szChunk) + iAmt) - p_ਸਮੂਹ->szChunk;
        ਜੇ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % p_ਸਮੂਹ->szChunk);
        ਜੇ( rc!=SQLITE_OK ) ਰੋਕੋ;
        pBuf = (ਅੱਖਰ *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }ਨਹੀਂਤਾਂ{
        rc = SQLITE_IOERR_READ;
        ਰੋਕੋ;
      }
    }
  }

  ਵਾਪਸ rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
ਸਥਿਰ ਪੂਰਨ multiplexWrite(
  sqlite3_file *pConn,
  ਅਚਲ ਖਾਲੀ *pBuf,
  ਪੂਰਨ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ = p->p_ਸਮੂਹ;
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( !p_ਸਮੂਹ->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
    ਜੇ( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }ਨਹੀਂਤਾਂ{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ਨਹੀਂਤਾਂ{
    ਜਦੋਂ( rc==SQLITE_OK && iAmt>0 ){
      ਪੂਰਨ i = (ਪੂਰਨ)(iOfst / p_ਸਮੂਹ->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, i, &rc, ਨਿਰਾ, 1);
      ਜੇ( pSubOpen ){
        ਪੂਰਨ extra = ((ਪੂਰਨ)(iOfst % p_ਸਮੂਹ->szChunk) + iAmt) -
                    p_ਸਮੂਹ->szChunk;
        ਜੇ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % p_ਸਮੂਹ->szChunk);
        pBuf = (ਅੱਖਰ *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  ਵਾਪਸ rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
ਸਥਿਰ ਪੂਰਨ multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 ਮਾਪ){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ = p->p_ਸਮੂਹ;
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( !p_ਸਮੂਹ->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
    ਜੇ( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }ਨਹੀਂਤਾਂ{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, ਮਾਪ);
    }
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i;
    ਪੂਰਨ i_ਅਧਾਰ_ਸਮੂਹ = (ਪੂਰਨ)(ਮਾਪ / p_ਸਮੂਹ->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    ਲਈ(i = p_ਸਮੂਹ->nReal-1; i>i_ਅਧਾਰ_ਸਮੂਹ && rc==SQLITE_OK; i--){
      ਜੇ( p_ਸਮੂਹ->bTruncate ){
        multiplexSubClose(p_ਸਮੂਹ, i, pOrigVfs);
      }ਨਹੀਂਤਾਂ{
        pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, i, &rc, 0, 0);
        ਜੇ( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    ਜੇ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, i_ਅਧਾਰ_ਸਮੂਹ, &rc, 0, 0);
      ਜੇ( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, ਮਾਪ % p_ਸਮੂਹ->szChunk);
      }
    }
    ਜੇ( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  ਵਾਪਸ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ਸਥਿਰ ਪੂਰਨ multiplexSync(sqlite3_file *pConn, ਪੂਰਨ flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ = p->p_ਸਮੂਹ;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ i;
  ਲਈ(i=0; i<p_ਸਮੂਹ->nReal; i++){
    sqlite3_file *pSubOpen = p_ਸਮੂਹ->aReal[i].p;
    ਜੇ( pSubOpen ){
      ਪੂਰਨ rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      ਜੇ( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  ਵਾਪਸ rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
ਸਥਿਰ ਪੂਰਨ multiplex_File_ਮਾਪ(sqlite3_file *pConn, sqlite3_int64 *p_ਮਾਪ){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ = p->p_ਸਮੂਹ;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ i;
  ਜੇ( !p_ਸਮੂਹ->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
    ਜੇ( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }ਨਹੀਂਤਾਂ{
      rc = pSubOpen->pMethods->x_File_ਮਾਪ(pSubOpen, p_ਮਾਪ);
    }
  }ਨਹੀਂਤਾਂ{
    *p_ਮਾਪ = 0;
    ਲਈ(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplex_Sub_ਮਾਪ(p_ਸਮੂਹ, i, &rc);
      ਜੇ( sz==0 ) ਰੋਕੋ;
      *p_ਮਾਪ = i*(sqlite3_int64)p_ਸਮੂਹ->szChunk + sz;
    }
  }
  ਵਾਪਸ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ multiplex_ਤਾਲਾ(sqlite3_file *pConn, ਪੂਰਨ ਤਾਲਾ){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen ){
    ਵਾਪਸ pSubOpen->pMethods->x_ਤਾਲਾ(pSubOpen, ਤਾਲਾ);
  }
  ਵਾਪਸ SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ multiplexUnlock(sqlite3_file *pConn, ਪੂਰਨ ਤਾਲਾ){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen ){
    ਵਾਪਸ pSubOpen->pMethods->xUnlock(pSubOpen, ਤਾਲਾ);
  }
  ਵਾਪਸ SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ multiplex_Check_Reserved_ਤਾਲਾ(sqlite3_file *pConn, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen ){
    ਵਾਪਸ pSubOpen->pMethods->x_Check_Reserved_ਤਾਲਾ(pSubOpen, p_Res_ਬਾਹਰਲਾ);
  }
  ਵਾਪਸ SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
ਸਥਿਰ ਪੂਰਨ multiplexFileControl(sqlite3_file *pConn, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ਸਮੂਹ *p_ਸਮੂਹ = p->p_ਸਮੂਹ;
  ਪੂਰਨ rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  ਜੇ( !gMultiplex.isInitialized ) ਵਾਪਸ SQLITE_MISUSE;
  ਚੋਣ( op ){
    ਹਾਲਤ MULTIPLEX_CTRL_ENABLE:
      ਜੇ( pArg ) {
        ਪੂਰਨ bEnabled = *(ਪੂਰਨ *)pArg;
        p_ਸਮੂਹ->bEnabled = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)bEnabled;
        rc = SQLITE_OK;
      }
      ਰੋਕੋ;
    ਹਾਲਤ MULTIPLEX_CTRL_ਸੈੱਟ_CHUNK_ਮਾਪ:
      ਜੇ( pArg ) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ szChunk = *(ਬਿਨਾਨਿਸ਼ਾਨੀ*)pArg;
        ਜੇ( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }ਨਹੀਂਤਾਂ{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_ਪੰਨਾ_ਮਾਪ-1));
          szChunk &= ~(MAX_ਪੰਨਾ_ਮਾਪ-1);
          p_ਸਮੂਹ->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      ਰੋਕੋ;
    ਹਾਲਤ MULTIPLEX_CTRL_ਸੈੱਟ_MAX_CHUNKS:
      rc = SQLITE_OK;
      ਰੋਕੋ;
    ਹਾਲਤ SQLITE_FCNTL_ਮਾਪ_HINT:
    ਹਾਲਤ SQLITE_FCNTL_CHUNK_ਮਾਪ:
      /* no-op these */
      rc = SQLITE_OK;
      ਰੋਕੋ;
    ਹਾਲਤ SQLITE_FCNTL_PRAGMA: {
      ਅੱਖਰ **aFcntl = (ਅੱਖਰ**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      ਜੇ( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (ਖਾਲੀ)multiplex_File_ਮਾਪ(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        ਜੇ( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          ਜੇ( aFcntl[2] && aFcntl[2][0] ){
            ਜੇ( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              p_ਸਮੂਹ->bTruncate = 1;
            }ਨਹੀਂਤਾਂ
            ਜੇ( sqlite3_stricmp(aFcntl[2], "off")==0
             || sqlite3_stricmp(aFcntl[2], "0")==0 ){
              p_ਸਮੂਹ->bTruncate = 0;
            }
          }
          /* EVIDENCE-OF: R-27806-26076 The handler for an SQLITE_FCNTL_PRAGMA
          ** file control can optionally make the first element of the char**
          ** argument point to a string obtained from sqlite3_mprintf() or the
          ** equivalent and that string will become the result of the pragma
          ** or the error message if the pragma fails.
          */
          aFcntl[0] = sqlite3_mprintf(p_ਸਮੂਹ->bTruncate ? "on" : "off");
          rc = SQLITE_OK;
          ਰੋਕੋ;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        ਜੇ( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", p_ਸਮੂਹ->bEnabled!=0);
          rc = SQLITE_OK;
          ਰੋਕੋ;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        ਜੇ( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && p_ਸਮੂਹ->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", p_ਸਮੂਹ->szChunk);
          rc = SQLITE_OK;
          ਰੋਕੋ;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        ਜੇ( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          ਪੂਰਨ n = 0;
          ਪੂਰਨ ii;
          ਲਈ(ii=0; ii<p_ਸਮੂਹ->nReal; ii++){
            ਜੇ( p_ਸਮੂਹ->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          ਰੋਕੋ;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    ਮੂਲ:
      pSubOpen = multiplexSubOpen(p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
      ਜੇ( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        ਜੇ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(ਅੱਖਰ**)pArg = sqlite3_mprintf("multiplex/%z", *(ਅੱਖਰ**)pArg);
        }
      }
      ਰੋਕੋ;
  }
  ਵਾਪਸ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ multiplex_Sector_ਮਾਪ(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen && pSubOpen->pMethods->x_Sector_ਮਾਪ ){
    ਵਾਪਸ pSubOpen->pMethods->x_Sector_ਮਾਪ(pSubOpen);
  }
  ਵਾਪਸ DEFAULT_SECTOR_ਮਾਪ;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen ){
    ਵਾਪਸ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  ਵਾਪਸ 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ਪੂਰਨ iRegion,                    /* Region to retrieve */
  ਪੂਰਨ szRegion,                   /* Size of regions */
  ਪੂਰਨ bExtend,                    /* True to extend file if necessary */
  ਖਾਲੀ ਅਸਥਿਰ **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen ){
    ਵਾਪਸ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  ਵਾਪਸ SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ multiplex_Shm_ਤਾਲਾ(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ਪੂਰਨ ofst,                  /* First lock to acquire or release */
  ਪੂਰਨ n,                     /* Number of locks to acquire or release */
  ਪੂਰਨ flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen ){
    ਵਾਪਸ pSubOpen->pMethods->x_Shm_ਤਾਲਾ(pSubOpen, ofst, n, flags);
  }
  ਵਾਪਸ SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਖਾਲੀ multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ਸਥਿਰ ਪੂਰਨ multiplexShmUnmap(sqlite3_file *pConn, ਪੂਰਨ deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  ਪੂਰਨ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ਸਮੂਹ, 0, &rc, ਨਿਰਾ, 0);
  ਜੇ( pSubOpen ){
    ਵਾਪਸ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  ਵਾਪਸ SQLITE_OK;
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
ਪੂਰਨ sqlite3_multiplex_initialize(ਅਚਲ ਅੱਖਰ *zOrigVfsName, ਪੂਰਨ makeDefault){
  sqlite3_vfs *pOrigVfs;
  ਜੇ( gMultiplex.isInitialized ) ਵਾਪਸ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_ਲੱਭੋ(zOrigVfsName);
  ਜੇ( pOrigVfs==0 ) ਵਾਪਸ SQLITE_ERROR;
  ਪ੍ਰਤਿਜ੍ਞਾ( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += ਆਕਾਰ(multiplexConn);
  gMultiplex.sThisVfs.zName = SQLITE_MULTIPLEX_VFS_NAME;
  gMultiplex.sThisVfs.xOpen = multiplexOpen;
  gMultiplex.sThisVfs.xDelete = multiplexDelete;
  gMultiplex.sThisVfs.xAccess = multiplexAccess;
  gMultiplex.sThisVfs.x_ਭਰਿਆ_Pathname = multiplex_ਭਰਿਆ_Pathname;
  gMultiplex.sThisVfs.xDlOpen = multiplexDlOpen;
  gMultiplex.sThisVfs.xDlError = multiplexDlError;
  gMultiplex.sThisVfs.xDlSym = multiplexDlSym;
  gMultiplex.sThisVfs.xDlClose = multiplexDlClose;
  gMultiplex.sThisVfs.xRandomness = multiplexRandomness;
  gMultiplex.sThisVfs.xSleep = multiplexSleep;
  gMultiplex.sThisVfs.x_Current_ਸਮਾਂ = multiplex_Current_ਸਮਾਂ;
  gMultiplex.sThisVfs.x_Get_ਆਖਰੀ_Error = multiplex_Get_ਆਖਰੀ_Error;
  gMultiplex.sThisVfs.x_Current_ਸਮਾਂ_Int64 = multiplex_Current_ਸਮਾਂ_Int64;

  gMultiplex.sIoMethodsV1.i_ਸੰਸਕਰਣ = 1;
  gMultiplex.sIoMethodsV1.xClose = multiplexClose;
  gMultiplex.sIoMethodsV1.xRead = multiplexRead;
  gMultiplex.sIoMethodsV1.xWrite = multiplexWrite;
  gMultiplex.sIoMethodsV1.xTruncate = multiplexTruncate;
  gMultiplex.sIoMethodsV1.xSync = multiplexSync;
  gMultiplex.sIoMethodsV1.x_File_ਮਾਪ = multiplex_File_ਮਾਪ;
  gMultiplex.sIoMethodsV1.x_ਤਾਲਾ = multiplex_ਤਾਲਾ;
  gMultiplex.sIoMethodsV1.xUnlock = multiplexUnlock;
  gMultiplex.sIoMethodsV1.x_Check_Reserved_ਤਾਲਾ = multiplex_Check_Reserved_ਤਾਲਾ;
  gMultiplex.sIoMethodsV1.xFileControl = multiplexFileControl;
  gMultiplex.sIoMethodsV1.x_Sector_ਮਾਪ = multiplex_Sector_ਮਾਪ;
  gMultiplex.sIoMethodsV1.xDeviceCharacteristics =
                                            multiplexDeviceCharacteristics;
  gMultiplex.sIoMethodsV2 = gMultiplex.sIoMethodsV1;
  gMultiplex.sIoMethodsV2.i_ਸੰਸਕਰਣ = 2;
  gMultiplex.sIoMethodsV2.xShmMap = multiplexShmMap;
  gMultiplex.sIoMethodsV2.x_Shm_ਤਾਲਾ = multiplex_Shm_ਤਾਲਾ;
  gMultiplex.sIoMethodsV2.xShmBarrier = multiplexShmBarrier;
  gMultiplex.sIoMethodsV2.xShmUnmap = multiplexShmUnmap;
  sqlite3_vfs_ਰਜਿਸਟਰ(&gMultiplex.sThisVfs, makeDefault);

  sqlite3_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))multiplexFuncInit);

  ਵਾਪਸ SQLITE_OK;
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
ਪੂਰਨ sqlite3_multiplex_shutdown(ਪੂਰਨ eForce){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( gMultiplex.isInitialized==0 ) ਵਾਪਸ SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, ਆਕਾਰ(gMultiplex));
  ਵਾਪਸ rc;
}

/***************************** Test Code ***********************************/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_multiplex_initialize(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *zName;              /* Name of new multiplex VFS */
  ਪੂਰਨ makeDefault;                /* True to make the new VFS the default */
  ਪੂਰਨ rc;                         /* Value returned by multiplex_initialize() */

  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);

  /* Process arguments */
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "NAME MAKEDEFAULT");
    ਵਾਪਸ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ਜੇ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);

  ਵਾਪਸ TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_multiplex_shutdown(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;                         /* Value returned by multiplex_shutdown() */

  ਅਣਵਰਤਿਆ_PARAMETER(client_ਅੰਕੜੇ);

  ਜੇ( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  ਜੇ( (objc!=1 && objc!=2) ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "?-force?");
    ਵਾਪਸ TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);

  ਵਾਪਸ TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_multiplex_control(
  Client_ਅੰਕੜੇ cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;                         /* Return code from file_control() */
  ਪੂਰਨ idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  ਪੂਰਨ i_ਮੁੱਲ = 0;
  ਖਾਲੀ *pArg = 0;

  ਬਣਤਰ SubCommand {
    ਅਚਲ ਅੱਖਰ *zName;
    ਪੂਰਨ op;
    ਪੂਰਨ argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_ਸੈੱਟ_CHUNK_ਮਾਪ,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_ਸੈੱਟ_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  ਜੇ( objc!=5 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "expected database handle, got \"", ਨਿਰਾ);
    Tcl_Append_ਨਤੀਜਾ(interp, Tcl_GetString(objv[1]), "\"", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }ਨਹੀਂਤਾਂ{
    db = *(sqlite3 **)cmdInfo.obj_Client_ਅੰਕੜੇ;
  }

  rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(
      interp, objv[3], aSub, ਆਕਾਰ(aSub[0]), "sub-command", 0, &idx
  );
  ਜੇ( rc!=TCL_OK ) ਵਾਪਸ rc;

  ਚੋਣ( aSub[idx].argtype ){
    ਹਾਲਤ 1:
      ਜੇ( Tcl_GetIntFromObj(interp, objv[4], &i_ਮੁੱਲ) ){
        ਵਾਪਸ TCL_ERROR;
      }
      pArg = (ਖਾਲੀ *)&i_ਮੁੱਲ;
      ਰੋਕੋ;
    ਮੂਲ:
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 4, objv, "SUB-COMMAND");
      ਵਾਪਸ TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);
  ਵਾਪਸ (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ਪੂਰਨ Sqlitemultiplex_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  ਪੂਰਨ i;

  ਲਈ(i=0; i<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ਵਾਪਸ TCL_OK;
}
#ਜੇ_ਅੰਤ
