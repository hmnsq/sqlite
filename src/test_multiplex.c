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
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ "test_multiplex.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_CORE
  #ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಳಸದ_PARAMETER(x) (ಶೂನ್ಯ)(x)
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ಪುಟ_ಗಾತ್ರ       0x10000
#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_SECTOR_ಗಾತ್ರ 0x1000

/* Maximum chunk number */
#ವ್ಯಾಖ್ಯಾನಿಸು MX_CHUNK_ಸಂಖ್ಯೆ 299

/* First chunk for rollback journal files */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MULTIPLEX_JOURNAL_8_3_ಸ್ಥಾನಾಂತರ 400
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MULTIPLEX_WAL_8_3_ಸ್ಥಾನಾಂತರ 700


/************************ Shim Definitions ******************************/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MULTIPLEX_VFS_NAME
# define SQLITE_MULTIPLEX_VFS_NAME "multiplex"
#ಆದರೆ_ಅಂತ್ಯ

/* This is the limit on the chunk size.  It may be changed by calling
** the xFileControl() interface.  It will be rounded up to a
** multiple of MAX_PAGE_SIZE.  We default it here to 2GiB less 64KiB.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MULTIPLEX_CHUNK_ಗಾತ್ರ
# define SQLITE_MULTIPLEX_CHUNK_ಗಾತ್ರ 2147418112
#ಆದರೆ_ಅಂತ್ಯ

/* This used to be the default limit on number of chunks, but
** it is no longer enforced. There is currently no limit to the
** number of chunks.
**
** May be changed by calling the xFileControl() interface.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MULTIPLEX_MAX_CHUNKS
# define SQLITE_MULTIPLEX_MAX_CHUNKS 12
#ಆದರೆ_ಅಂತ್ಯ

/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ multiplex_ಗುಂಪು multiplex_ಗುಂಪು;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
ರಚನೆ multiplex_ಗುಂಪು {
  ರಚನೆ multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    ಅಕ್ಷರ *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  ಪೂರ್ಣಾಂಕ nReal;                       /* Number of chunks */
  ಅಕ್ಷರ *zName;                     /* Base filename of this group */
  ಪೂರ್ಣಾಂಕ nName;                       /* Length of base filename */
  ಪೂರ್ಣಾಂಕ flags;                       /* Flags used for original opening */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ szChunk;            /* Chunk size used for this group */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ bEnabled;          /* TRUE to use Multiplex VFS for this file */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ರಚನೆ multiplexConn {
  sqlite3_file ತಳ;              /* Base class - must be first */
  multiplex_ಗುಂಪು *p_ಗುಂಪು;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
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

  /* True when this shim has been initialized.
  */
  ಪೂರ್ಣಾಂಕ isInitialized;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexStrlen30(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z2 = z;
  ಆದರೆ( z==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆಗ( *z2 ){ z2++; }
  ಹಿಂತಿರುಗಿ 0x3fffffff & (ಪೂರ್ಣಾಂಕ)(z2 - z);
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
ಸ್ಥಿರ ಶೂನ್ಯ multiplexFilename(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ತಳ,              /* Filename for chunk 0 */
  ಪೂರ್ಣಾಂಕ n_ತಳ,                      /* Size of zBase in bytes (without \0) */
  ಪೂರ್ಣಾಂಕ flags,                      /* Flags used to open file */
  ಪೂರ್ಣಾಂಕ iChunk,                     /* Chunk to generate filename for */
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು                      /* Buffer to write generated name to */
){
  ಪೂರ್ಣಾಂಕ n = n_ತಳ;
  ಸ್ಮರಣೆ_ನಕಲು(z_ಹೊರಹೋಗು, z_ತಳ, n+1);
  ಆದರೆ( iChunk!=0 && iChunk<=MX_CHUNK_ಸಂಖ್ಯೆ ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_8_3_ಹೆಸರುಗಳು
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=n-1; i>0 && i>=n-4 && z_ಹೊರಹೋಗು[i]!='.'; i--){}
    ಆದರೆ( i>=n-4 ) n = i+1;
    ಆದರೆ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_ಸ್ಥಾನಾಂತರ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_ಸ್ಥಾನಾಂತರ;
    }
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_snprintf(4,&z_ಹೊರಹೋಗು[n],"%03d",iChunk);
    n += 3;
  }

  ಪ್ರತಿಜ್ಞಾ( z_ಹೊರಹೋಗು[n]=='\0' );
  z_ಹೊರಹೋಗು[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexSubFilename(multiplex_ಗುಂಪು *p_ಗುಂಪು, ಪೂರ್ಣಾಂಕ iChunk){
  ಆದರೆ( iChunk>=p_ಗುಂಪು->nReal ){
    ರಚನೆ multiplexReal *p;
    p = sqlite3_realloc64(p_ಗುಂಪು->aReal, (iChunk+1)*ಗಾತ್ರ(*p));
    ಆದರೆ( p==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    memset(&p[p_ಗುಂಪು->nReal], 0, ಗಾತ್ರ(p[0])*(iChunk+1-p_ಗುಂಪು->nReal));
    p_ಗುಂಪು->aReal = p;
    p_ಗುಂಪು->nReal = iChunk+1;
  }
  ಆದರೆ( p_ಗುಂಪು->zName && p_ಗುಂಪು->aReal[iChunk].z==0 ){
    ಅಕ್ಷರ *z;
    ಪೂರ್ಣಾಂಕ n = p_ಗುಂಪು->nName;
    z = sqlite3_malloc64( n+5 );
    ಆದರೆ( z==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    multiplexFilename(p_ಗುಂಪು->zName, p_ಗುಂಪು->nName, p_ಗುಂಪು->flags, iChunk, z);
    p_ಗುಂಪು->aReal[iChunk].z = (ಅಕ್ಷರ*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    ಆದರೆ( p_ಗುಂಪು->aReal[iChunk].z==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
ಸ್ಥಿರ sqlite3_file *multiplexSubOpen(
  multiplex_ಗುಂಪು *p_ಗುಂಪು,    /* The multiplexor group */
  ಪೂರ್ಣಾಂಕ iChunk,                /* Which chunk to open.  0==original file */
  ಪೂರ್ಣಾಂಕ *rc,                   /* Result code in and out */
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags,            /* Output flags */
  ಪೂರ್ಣಾಂಕ createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_8_3_ಹೆಸರುಗಳು
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  ಆದರೆ( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_ಸ್ಥಾನಾಂತರ ){
    sqlite3_log(SQLITE_ತುಂಬಿದ, "multiplexed chunk overflow: %s", p_ಗುಂಪು->zName);
    *rc = SQLITE_ತುಂಬಿದ;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  *rc = multiplexSubFilename(p_ಗುಂಪು, iChunk);
  ಆದರೆ( (*rc)==SQLITE_OK && (pSubOpen = p_ಗುಂಪು->aReal[iChunk].p)==0 ){
    ಪೂರ್ಣಾಂಕ flags, bExists;
    flags = p_ಗುಂಪು->flags;
    ಆದರೆ( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iChunk==0 ){
      /* Fall through */
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಗುಂಪು->aReal[iChunk].z==0 ){
      ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      *rc = pOrigVfs->xAccess(pOrigVfs, p_ಗುಂಪು->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     ಆದರೆ( *rc || !bExists ){
        ಆದರೆ( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      p_ಗುಂಪು->aReal[iChunk].z);
        }
        ಹಿಂತಿರುಗಿ 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    ಆದರೆ( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      ಹಿಂತಿರುಗಿ 0;
    }
    p_ಗುಂಪು->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, p_ಗುಂಪು->aReal[iChunk].z, pSubOpen,
                          flags, p_ಹೊರಹೋಗು_Flags);
    ಆದರೆ( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  p_ಗುಂಪು->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      p_ಗುಂಪು->aReal[iChunk].p = 0;
      ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಹಿಂತಿರುಗಿ pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
ಸ್ಥಿರ sqlite3_int64 multiplex_Sub_ಗಾತ್ರ(
  multiplex_ಗುಂಪು *p_ಗುಂಪು,    /* The multiplexor group */
  ಪೂರ್ಣಾಂಕ iChunk,                /* Which chunk to open.  0==original file */
  ಪೂರ್ಣಾಂಕ *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  ಆದರೆ( *rc ) ಹಿಂತಿರುಗಿ 0;
  pSub = multiplexSubOpen(p_ಗುಂಪು, iChunk, rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSub==0 ) ಹಿಂತಿರುಗಿ 0;
  *rc = pSub->pMethods->x_File_ಗಾತ್ರ(pSub, &sz);
  ಹಿಂತಿರುಗಿ sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ multiplexControlFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪೂರ್ಣಾಂಕ op = 0;
  ಪೂರ್ಣಾಂಕ iVal;

  ಆದರೆ( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* extract params */
    op = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
    iVal = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
    /* map function op to file_control op */
    ಆಯ್ಕೆ( op ){
      ಸ್ಥಿತಿ 1:
        op = MULTIPLEX_CTRL_ENABLE;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 2:
        op = MULTIPLEX_CTRL_ಗಣ_CHUNK_ಗಾತ್ರ;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ 3:
        op = MULTIPLEX_CTRL_ಗಣ_MAX_CHUNKS;
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
        rc = SQLITE_NOTFOUND;
        ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexFuncInit(
  sqlite3 *db,
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ,
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pApi
){
  ಪೂರ್ಣಾಂಕ rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Close a single sub-file in the connection group.
*/
ಸ್ಥಿರ ಶೂನ್ಯ multiplexSubClose(
  multiplex_ಗುಂಪು *p_ಗುಂಪು,
  ಪೂರ್ಣಾಂಕ iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = p_ಗುಂಪು->aReal[iChunk].p;
  ಆದರೆ( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    ಆದರೆ( pOrigVfs && p_ಗುಂಪು->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, p_ಗುಂಪು->aReal[iChunk].z, 0);
    }
    sqlite3_free(p_ಗುಂಪು->aReal[iChunk].p);
  }
  sqlite3_free_filename(p_ಗುಂಪು->aReal[iChunk].z);
  memset(&p_ಗುಂಪು->aReal[iChunk], 0, ಗಾತ್ರ(p_ಗುಂಪು->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
ಸ್ಥಿರ ಶೂನ್ಯ multiplexFreeComponents(multiplex_ಗುಂಪು *p_ಗುಂಪು){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<p_ಗುಂಪು->nReal; i++){ multiplexSubClose(p_ಗುಂಪು, i, 0); }
  sqlite3_free(p_ಗುಂಪು->aReal);
  p_ಗುಂಪು->aReal = 0;
  p_ಗುಂಪು->nReal = 0;
}


/************************* VFS Method Wrappers *****************************/

/*
** This is the xOpen method used for the "multiplex" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate multiplex group if it is a
** file that needs to be tracked.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  ಪೂರ್ಣಾಂಕ flags,                 /* Flags to control the opening */
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags             /* Flags showing results of opening */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplex_ಗುಂಪು *p_ಗುಂಪು = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  ಪೂರ್ಣಾಂಕ nName = 0;
  ಪೂರ್ಣಾಂಕ sz = 0;
  ಅಕ್ಷರ *z_ಗಮ್ಯ_Free = 0;

  ಬಳಸದ_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  ಪ್ರತಿಜ್ಞಾ( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  ಆದರೆ( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = ಗಾತ್ರ(multiplex_ಗುಂಪು)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    p_ಗುಂಪು = sqlite3_malloc64( sz );
    ಆದರೆ( p_ಗುಂಪು==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  ಆದರೆ( rc==SQLITE_OK ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(p_ಗುಂಪು, 0, sz);
    pMultiplexOpen->p_ಗುಂಪು = p_ಗುಂಪು;
    p_ಗುಂಪು->bEnabled = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)-1;
    p_ಗುಂಪು->bTruncate = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    p_ಗುಂಪು->szChunk = (ಪೂರ್ಣಾಂಕ)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_ಗಾತ್ರ);
    p_ಗುಂಪು->szChunk = (p_ಗುಂಪು->szChunk+0xffff)&~0xffff;
    ಆದರೆ( zName ){
      ಅಕ್ಷರ *p = (ಅಕ್ಷರ *)&p_ಗುಂಪು[1];
      p_ಗುಂಪು->zName = p;
      ಸ್ಮರಣೆ_ನಕಲು(p_ಗುಂಪು->zName, zName, nName+1);
      p_ಗುಂಪು->nName = nName;
    }
    ಆದರೆ( p_ಗುಂಪು->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WSD
      ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3PendingByte;
#ಇಲ್ಲದಿದ್ದರೆ
      ಪೂರ್ಣಾಂಕ sqlite3PendingByte = 0x40000000;
#ಆದರೆ_ಅಂತ್ಯ
      ಆಗ( (sqlite3PendingByte % p_ಗುಂಪು->szChunk)>=(p_ಗುಂಪು->szChunk-65536) ){
        p_ಗುಂಪು->szChunk += 65536;
      }
    }
    p_ಗುಂಪು->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(p_ಗುಂಪು, 1);
    ಆದರೆ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(p_ಗುಂಪು, 0, &rc, p_ಹೊರಹೋಗು_Flags, 0);
      ಆದರೆ( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->x_File_ಗಾತ್ರ(pSubOpen, &sz64);
      ಆದರೆ( rc==SQLITE_OK && zName ){
        ಪೂರ್ಣಾಂಕ bExists;
        ಆದರೆ( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          p_ಗುಂಪು->bEnabled = 0;
        }ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( sz64==0 ){
          ಆದರೆ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            ಪೂರ್ಣಾಂಕ iChunk = 1;
            ಮಾಡು {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  p_ಗುಂಪು->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              ಆದರೆ( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, p_ಗುಂಪು->aReal[iChunk].z, 0);
                ಆದರೆ( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(p_ಗುಂಪು, ++iChunk);
                }
              }
            }ಆಗ( rc==SQLITE_OK && bExists );
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* If the first overflow file exists and if the size of the main file
          ** is different from the chunk size, that means the chunk size is set
          ** set incorrectly.  So fix it.
          **
          ** Or, if the first overflow file does not exist and the main file is
          ** larger than the chunk size, that means the chunk size is too small.
          ** But we have no way of determining the intended chunk size, so
          ** just disable the multiplexor all together.
          */
          rc = pOrigVfs->xAccess(pOrigVfs, p_ಗುಂಪು->aReal[1].z,
              SQLITE_ACCESS_EXISTS, &bExists);
          bExists = multiplex_Sub_ಗಾತ್ರ(p_ಗುಂಪು, 1, &rc)>0;
          ಆದರೆ( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=p_ಗುಂಪು->szChunk ){
            p_ಗುಂಪು->szChunk = (ಪೂರ್ಣಾಂಕ)sz64;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==SQLITE_OK && !bExists && sz64>p_ಗುಂಪು->szChunk ){
            p_ಗುಂಪು->bEnabled = 0;
          }
        }
      }
    }

    ಆದರೆ( rc==SQLITE_OK ){
      ಆದರೆ( pSubOpen->pMethods->i_ಆವೃತ್ತಿ==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      multiplexFreeComponents(p_ಗುಂಪು);
      sqlite3_free(p_ಗುಂಪು);
    }
  }
  sqlite3_free(z_ಗಮ್ಯ_Free);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,         /* Name of file to delete */
  ಪೂರ್ಣಾಂಕ syncDir
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  ಆದರೆ( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    ಪೂರ್ಣಾಂಕ nName = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName);
    ಅಕ್ಷರ *z;
    z = sqlite3_malloc64(nName + 5);
    ಆದರೆ( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ iChunk = 0;
      ಪೂರ್ಣಾಂಕ bExists;
      ಮಾಡು{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }ಆಗ( rc==SQLITE_OK && bExists );
      ಆಗ( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      ಆದರೆ( rc==SQLITE_OK ){
        iChunk = 0;
        ಮಾಡು{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }ಆಗ( rc==SQLITE_OK && bExists );
        ಆಗ( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexAccess(sqlite3_vfs *a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b, ಪೂರ್ಣಾಂಕ c, ಪೂರ್ಣಾಂಕ *d){
  ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_ತುಂಬಿದ_Pathname(sqlite3_vfs *a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b, ಪೂರ್ಣಾಂಕ c, ಅಕ್ಷರ *d){
  ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->x_ತುಂಬಿದ_Pathname(gMultiplex.pOrigVfs, b, c, d);
}
ಸ್ಥಿರ ಶೂನ್ಯ *multiplexDlOpen(sqlite3_vfs *a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b){
  ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
ಸ್ಥಿರ ಶೂನ್ಯ multiplexDlError(sqlite3_vfs *a, ಪೂರ್ಣಾಂಕ b, ಅಕ್ಷರ *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
ಸ್ಥಿರ ಶೂನ್ಯ (*multiplexDlSym(sqlite3_vfs *a, ಶೂನ್ಯ *b, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *c))(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
ಸ್ಥಿರ ಶೂನ್ಯ multiplexDlClose(sqlite3_vfs *a, ಶೂನ್ಯ *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexRandomness(sqlite3_vfs *a, ಪೂರ್ಣಾಂಕ b, ಅಕ್ಷರ *c){
  ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexSleep(sqlite3_vfs *a, ಪೂರ್ಣಾಂಕ b){
  ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_Current_ಸಮಯ(sqlite3_vfs *a, ದ್ವಿಗುಣ *b){
  ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->x_Current_ಸಮಯ(gMultiplex.pOrigVfs, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_Get_ಕೊನೆಯ_Error(sqlite3_vfs *a, ಪೂರ್ಣಾಂಕ b, ಅಕ್ಷರ *c){
  ಆದರೆ( gMultiplex.pOrigVfs->x_Get_ಕೊನೆಯ_Error ){
    ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->x_Get_ಕೊನೆಯ_Error(gMultiplex.pOrigVfs, b, c);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_Current_ಸಮಯ_Int64(sqlite3_vfs *a, sqlite3_int64 *b){
  ಹಿಂತಿರುಗಿ gMultiplex.pOrigVfs->x_Current_ಸಮಯ_Int64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ಗುಂಪು *p_ಗುಂಪು = p->p_ಗುಂಪು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  multiplexFreeComponents(p_ಗುಂಪು);
  sqlite3_free(p_ಗುಂಪು);
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexRead(
  sqlite3_file *pConn,
  ಶೂನ್ಯ *pBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ಗುಂಪು *p_ಗುಂಪು = p->p_ಗುಂಪು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( !p_ಗುಂಪು->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
    ಆದರೆ( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆಗ( iAmt > 0 ){
      ಪೂರ್ಣಾಂಕ i = (ಪೂರ್ಣಾಂಕ)(iOfst / p_ಗುಂಪು->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(p_ಗುಂಪು, i, &rc, ಖಾಲಿ, 1);
      ಆದರೆ( pSubOpen ){
        ಪೂರ್ಣಾಂಕ extra = ((ಪೂರ್ಣಾಂಕ)(iOfst % p_ಗುಂಪು->szChunk) + iAmt) - p_ಗುಂಪು->szChunk;
        ಆದರೆ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % p_ಗುಂಪು->szChunk);
        ಆದರೆ( rc!=SQLITE_OK ) ನಿಲ್ಲಿಸು;
        pBuf = (ಅಕ್ಷರ *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = SQLITE_IOERR_READ;
        ನಿಲ್ಲಿಸು;
      }
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexWrite(
  sqlite3_file *pConn,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ಗುಂಪು *p_ಗುಂಪು = p->p_ಗುಂಪು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( !p_ಗುಂಪು->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
    ಆದರೆ( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆಗ( rc==SQLITE_OK && iAmt>0 ){
      ಪೂರ್ಣಾಂಕ i = (ಪೂರ್ಣಾಂಕ)(iOfst / p_ಗುಂಪು->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(p_ಗುಂಪು, i, &rc, ಖಾಲಿ, 1);
      ಆದರೆ( pSubOpen ){
        ಪೂರ್ಣಾಂಕ extra = ((ಪೂರ್ಣಾಂಕ)(iOfst % p_ಗುಂಪು->szChunk) + iAmt) -
                    p_ಗುಂಪು->szChunk;
        ಆದರೆ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % p_ಗುಂಪು->szChunk);
        pBuf = (ಅಕ್ಷರ *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 ಗಾತ್ರ_2){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ಗುಂಪು *p_ಗುಂಪು = p->p_ಗುಂಪು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( !p_ಗುಂಪು->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
    ಆದರೆ( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, ಗಾತ್ರ_2);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ i_ತಳ_ಗುಂಪು = (ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ_2 / p_ಗುಂಪು->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    ಗಾಗಿ(i = p_ಗುಂಪು->nReal-1; i>i_ತಳ_ಗುಂಪು && rc==SQLITE_OK; i--){
      ಆದರೆ( p_ಗುಂಪು->bTruncate ){
        multiplexSubClose(p_ಗುಂಪು, i, pOrigVfs);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pSubOpen = multiplexSubOpen(p_ಗುಂಪು, i, &rc, 0, 0);
        ಆದರೆ( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    ಆದರೆ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(p_ಗುಂಪು, i_ತಳ_ಗುಂಪು, &rc, 0, 0);
      ಆದರೆ( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, ಗಾತ್ರ_2 % p_ಗುಂಪು->szChunk);
      }
    }
    ಆದರೆ( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexSync(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ಗುಂಪು *p_ಗುಂಪು = p->p_ಗುಂಪು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<p_ಗುಂಪು->nReal; i++){
    sqlite3_file *pSubOpen = p_ಗುಂಪು->aReal[i].p;
    ಆದರೆ( pSubOpen ){
      ಪೂರ್ಣಾಂಕ rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      ಆದರೆ( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_File_ಗಾತ್ರ(sqlite3_file *pConn, sqlite3_int64 *p_ಗಾತ್ರ){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ಗುಂಪು *p_ಗುಂಪು = p->p_ಗುಂಪು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( !p_ಗುಂಪು->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
    ಆದರೆ( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = pSubOpen->pMethods->x_File_ಗಾತ್ರ(pSubOpen, p_ಗಾತ್ರ);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    *p_ಗಾತ್ರ = 0;
    ಗಾಗಿ(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplex_Sub_ಗಾತ್ರ(p_ಗುಂಪು, i, &rc);
      ಆದರೆ( sz==0 ) ನಿಲ್ಲಿಸು;
      *p_ಗಾತ್ರ = i*(sqlite3_int64)p_ಗುಂಪು->szChunk + sz;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_ಬೀಗ(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ ಬೀಗ){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen ){
    ಹಿಂತಿರುಗಿ pSubOpen->pMethods->x_ಬೀಗ(pSubOpen, ಬೀಗ);
  }
  ಹಿಂತಿರುಗಿ SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexUnlock(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ ಬೀಗ){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen ){
    ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xUnlock(pSubOpen, ಬೀಗ);
  }
  ಹಿಂತಿರುಗಿ SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_Check_Reserved_ಬೀಗ(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen ){
    ಹಿಂತಿರುಗಿ pSubOpen->pMethods->x_Check_Reserved_ಬೀಗ(pSubOpen, p_Res_ಹೊರಹೋಗು);
  }
  ಹಿಂತಿರುಗಿ SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexFileControl(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_ಗುಂಪು *p_ಗುಂಪು = p->p_ಗುಂಪು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  ಆದರೆ( !gMultiplex.isInitialized ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE;
  ಆಯ್ಕೆ( op ){
    ಸ್ಥಿತಿ MULTIPLEX_CTRL_ENABLE:
      ಆದರೆ( pArg ) {
        ಪೂರ್ಣಾಂಕ bEnabled = *(ಪೂರ್ಣಾಂಕ *)pArg;
        p_ಗುಂಪು->bEnabled = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)bEnabled;
        rc = SQLITE_OK;
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ MULTIPLEX_CTRL_ಗಣ_CHUNK_ಗಾತ್ರ:
      ಆದರೆ( pArg ) {
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ szChunk = *(ಚಿಹ್ನೆರಹಿತ*)pArg;
        ಆದರೆ( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_ಪುಟ_ಗಾತ್ರ-1));
          szChunk &= ~(MAX_ಪುಟ_ಗಾತ್ರ-1);
          p_ಗುಂಪು->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ MULTIPLEX_CTRL_ಗಣ_MAX_CHUNKS:
      rc = SQLITE_OK;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_FCNTL_ಗಾತ್ರ_HINT:
    ಸ್ಥಿತಿ SQLITE_FCNTL_CHUNK_ಗಾತ್ರ:
      /* no-op these */
      rc = SQLITE_OK;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_FCNTL_PRAGMA: {
      ಅಕ್ಷರ **aFcntl = (ಅಕ್ಷರ**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      ಆದರೆ( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (ಶೂನ್ಯ)multiplex_File_ಗಾತ್ರ(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        ಆದರೆ( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          ಆದರೆ( aFcntl[2] && aFcntl[2][0] ){
            ಆದರೆ( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              p_ಗುಂಪು->bTruncate = 1;
            }ಇಲ್ಲದಿದ್ದರೆ
            ಆದರೆ( sqlite3_stricmp(aFcntl[2], "off")==0
             || sqlite3_stricmp(aFcntl[2], "0")==0 ){
              p_ಗುಂಪು->bTruncate = 0;
            }
          }
          /* EVIDENCE-OF: R-27806-26076 The handler for an SQLITE_FCNTL_PRAGMA
          ** file control can optionally make the first element of the char**
          ** argument point to a string obtained from sqlite3_mprintf() or the
          ** equivalent and that string will become the result of the pragma
          ** or the error message if the pragma fails.
          */
          aFcntl[0] = sqlite3_mprintf(p_ಗುಂಪು->bTruncate ? "on" : "off");
          rc = SQLITE_OK;
          ನಿಲ್ಲಿಸು;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        ಆದರೆ( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", p_ಗುಂಪು->bEnabled!=0);
          rc = SQLITE_OK;
          ನಿಲ್ಲಿಸು;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        ಆದರೆ( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && p_ಗುಂಪು->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", p_ಗುಂಪು->szChunk);
          rc = SQLITE_OK;
          ನಿಲ್ಲಿಸು;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        ಆದರೆ( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          ಪೂರ್ಣಾಂಕ n = 0;
          ಪೂರ್ಣಾಂಕ ii;
          ಗಾಗಿ(ii=0; ii<p_ಗುಂಪು->nReal; ii++){
            ಆದರೆ( p_ಗುಂಪು->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          ನಿಲ್ಲಿಸು;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    ಪೂರ್ವನಿಯೋಜಿತ:
      pSubOpen = multiplexSubOpen(p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
      ಆದರೆ( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        ಆದರೆ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(ಅಕ್ಷರ**)pArg = sqlite3_mprintf("multiplex/%z", *(ಅಕ್ಷರ**)pArg);
        }
      }
      ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_Sector_ಗಾತ್ರ(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen && pSubOpen->pMethods->x_Sector_ಗಾತ್ರ ){
    ಹಿಂತಿರುಗಿ pSubOpen->pMethods->x_Sector_ಗಾತ್ರ(pSubOpen);
  }
  ಹಿಂತಿರುಗಿ DEFAULT_SECTOR_ಗಾತ್ರ;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen ){
    ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  ಹಿಂತಿರುಗಿ 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ಪೂರ್ಣಾಂಕ iRegion,                    /* Region to retrieve */
  ಪೂರ್ಣಾಂಕ szRegion,                   /* Size of regions */
  ಪೂರ್ಣಾಂಕ bExtend,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಅಸ್ಥಿರ **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen ){
    ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  ಹಿಂತಿರುಗಿ SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplex_Shm_ಬೀಗ(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ಪೂರ್ಣಾಂಕ ofst,                  /* First lock to acquire or release */
  ಪೂರ್ಣಾಂಕ n,                     /* Number of locks to acquire or release */
  ಪೂರ್ಣಾಂಕ flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen ){
    ಹಿಂತಿರುಗಿ pSubOpen->pMethods->x_Shm_ಬೀಗ(pSubOpen, ofst, n, flags);
  }
  ಹಿಂತಿರುಗಿ SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಶೂನ್ಯ multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiplexShmUnmap(sqlite3_file *pConn, ಪೂರ್ಣಾಂಕ deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_ಗುಂಪು, 0, &rc, ಖಾಲಿ, 0);
  ಆದರೆ( pSubOpen ){
    ಹಿಂತಿರುಗಿ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3_multiplex_initialize(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOrigVfsName, ಪೂರ್ಣಾಂಕ makeDefault){
  sqlite3_vfs *pOrigVfs;
  ಆದರೆ( gMultiplex.isInitialized ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(zOrigVfsName);
  ಆದರೆ( pOrigVfs==0 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  ಪ್ರತಿಜ್ಞಾ( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += ಗಾತ್ರ(multiplexConn);
  gMultiplex.sThisVfs.zName = SQLITE_MULTIPLEX_VFS_NAME;
  gMultiplex.sThisVfs.xOpen = multiplexOpen;
  gMultiplex.sThisVfs.xDelete = multiplexDelete;
  gMultiplex.sThisVfs.xAccess = multiplexAccess;
  gMultiplex.sThisVfs.x_ತುಂಬಿದ_Pathname = multiplex_ತುಂಬಿದ_Pathname;
  gMultiplex.sThisVfs.xDlOpen = multiplexDlOpen;
  gMultiplex.sThisVfs.xDlError = multiplexDlError;
  gMultiplex.sThisVfs.xDlSym = multiplexDlSym;
  gMultiplex.sThisVfs.xDlClose = multiplexDlClose;
  gMultiplex.sThisVfs.xRandomness = multiplexRandomness;
  gMultiplex.sThisVfs.xSleep = multiplexSleep;
  gMultiplex.sThisVfs.x_Current_ಸಮಯ = multiplex_Current_ಸಮಯ;
  gMultiplex.sThisVfs.x_Get_ಕೊನೆಯ_Error = multiplex_Get_ಕೊನೆಯ_Error;
  gMultiplex.sThisVfs.x_Current_ಸಮಯ_Int64 = multiplex_Current_ಸಮಯ_Int64;

  gMultiplex.sIoMethodsV1.i_ಆವೃತ್ತಿ = 1;
  gMultiplex.sIoMethodsV1.xClose = multiplexClose;
  gMultiplex.sIoMethodsV1.xRead = multiplexRead;
  gMultiplex.sIoMethodsV1.xWrite = multiplexWrite;
  gMultiplex.sIoMethodsV1.xTruncate = multiplexTruncate;
  gMultiplex.sIoMethodsV1.xSync = multiplexSync;
  gMultiplex.sIoMethodsV1.x_File_ಗಾತ್ರ = multiplex_File_ಗಾತ್ರ;
  gMultiplex.sIoMethodsV1.x_ಬೀಗ = multiplex_ಬೀಗ;
  gMultiplex.sIoMethodsV1.xUnlock = multiplexUnlock;
  gMultiplex.sIoMethodsV1.x_Check_Reserved_ಬೀಗ = multiplex_Check_Reserved_ಬೀಗ;
  gMultiplex.sIoMethodsV1.xFileControl = multiplexFileControl;
  gMultiplex.sIoMethodsV1.x_Sector_ಗಾತ್ರ = multiplex_Sector_ಗಾತ್ರ;
  gMultiplex.sIoMethodsV1.xDeviceCharacteristics =
                                            multiplexDeviceCharacteristics;
  gMultiplex.sIoMethodsV2 = gMultiplex.sIoMethodsV1;
  gMultiplex.sIoMethodsV2.i_ಆವೃತ್ತಿ = 2;
  gMultiplex.sIoMethodsV2.xShmMap = multiplexShmMap;
  gMultiplex.sIoMethodsV2.x_Shm_ಬೀಗ = multiplex_Shm_ಬೀಗ;
  gMultiplex.sIoMethodsV2.xShmBarrier = multiplexShmBarrier;
  gMultiplex.sIoMethodsV2.xShmUnmap = multiplexShmUnmap;
  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&gMultiplex.sThisVfs, makeDefault);

  sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))multiplexFuncInit);

  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3_multiplex_shutdown(ಪೂರ್ಣಾಂಕ eForce){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( gMultiplex.isInitialized==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, ಗಾತ್ರ(gMultiplex));
  ಹಿಂತಿರುಗಿ rc;
}

/***************************** Test Code ***********************************/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_multiplex_initialize(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;              /* Name of new multiplex VFS */
  ಪೂರ್ಣಾಂಕ makeDefault;                /* True to make the new VFS the default */
  ಪೂರ್ಣಾಂಕ rc;                         /* Value returned by multiplex_initialize() */

  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);

  /* Process arguments */
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "NAME MAKEDEFAULT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_multiplex_shutdown(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;                         /* Value returned by multiplex_shutdown() */

  ಬಳಸದ_PARAMETER(client_ದತ್ತಾಂಶ);

  ಆದರೆ( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  ಆದರೆ( (objc!=1 && objc!=2) ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "?-force?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_multiplex_control(
  Client_ದತ್ತಾಂಶ cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code from file_control() */
  ಪೂರ್ಣಾಂಕ idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  ಪೂರ್ಣಾಂಕ i_ಮೌಲ್ಯ = 0;
  ಶೂನ್ಯ *pArg = 0;

  ರಚನೆ SubCommand {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    ಪೂರ್ಣಾಂಕ op;
    ಪೂರ್ಣಾಂಕ argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_ಗಣ_CHUNK_ಗಾತ್ರ,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_ಗಣ_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  ಆದರೆ( objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "expected database handle, got \"", ಖಾಲಿ);
    Tcl_Append_ಫಲಿತಾಂಶ(interp, Tcl_GetString(objv[1]), "\"", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    db = *(sqlite3 **)cmdInfo.obj_Client_ದತ್ತಾಂಶ;
  }

  rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(
      interp, objv[3], aSub, ಗಾತ್ರ(aSub[0]), "sub-command", 0, &idx
  );
  ಆದರೆ( rc!=TCL_OK ) ಹಿಂತಿರುಗಿ rc;

  ಆಯ್ಕೆ( aSub[idx].argtype ){
    ಸ್ಥಿತಿ 1:
      ಆದರೆ( Tcl_GetIntFromObj(interp, objv[4], &i_ಮೌಲ್ಯ) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      pArg = (ಶೂನ್ಯ *)&i_ಮೌಲ್ಯ;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 4, objv, "SUB-COMMAND");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);
  ಹಿಂತಿರುಗಿ (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ಪೂರ್ಣಾಂಕ Sqlitemultiplex_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
