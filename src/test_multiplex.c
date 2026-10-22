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

#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#ಅನ್ತರ್ಭಾವ "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#define UNUSED_PARAMETER(x) (ಶೂನ್ಯ)(x)
#define MAX_PAGE_SIZE       0x10000
#define DEFAULT_SECTOR_SIZE 0x1000

/* Maximum chunk number */
#define MX_CHUNK_ಸಂಖ್ಯೆ 299

/* First chunk for rollback journal files */
#define SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET 400
#define SQLITE_MULTIPLEX_WAL_8_3_OFFSET 700


/************************ Shim Definitions ******************************/

#ifndef SQLITE_MULTIPLEX_VFS_NAME
# define SQLITE_MULTIPLEX_VFS_NAME "multiplex"
#endif

/* This is the limit on the chunk size.  It may be changed by calling
** the xFileControl() interface.  It will be rounded up to a
** multiple of MAX_PAGE_SIZE.  We default it here to 2GiB less 64KiB.
*/
#ifndef SQLITE_MULTIPLEX_CHUNK_SIZE
# define SQLITE_MULTIPLEX_CHUNK_SIZE 2147418112
#endif

/* This used to be the default limit on number of chunks, but
** it is no longer enforced. There is currently no limit to the
** number of chunks.
**
** May be changed by calling the xFileControl() interface.
*/
#ifndef SQLITE_MULTIPLEX_MAX_CHUNKS
# define SQLITE_MULTIPLEX_MAX_CHUNKS 12
#endif

/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ multiplexGroup multiplexGroup;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
ಸಂರಚನಾ multiplexGroup {
  ಸಂರಚನಾ multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    ವರ್ಣ *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  ಪೂರ್ಣಾಙ್ಕ nReal;                       /* Number of chunks */
  ವರ್ಣ *zName;                     /* Base filename of this group */
  ಪೂರ್ಣಾಙ್ಕ nName;                       /* Length of base filename */
  ಪೂರ್ಣಾಙ್ಕ flags;                       /* Flags used for original opening */
  unsigned ಪೂರ್ಣಾಙ್ಕ szChunk;            /* Chunk size used for this group */
  unsigned ವರ್ಣ bEnabled;          /* TRUE to use Multiplex VFS for this file */
  unsigned ವರ್ಣ bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ಸಂರಚನಾ multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
ಸ್ಥಿರ ಸಂರಚನಾ {
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
  ಪೂರ್ಣಾಙ್ಕ isInitialized;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexStrlen30(ಧ್ರುವ ವರ್ಣ *z){
  ಧ್ರುವ ವರ್ಣ *z2 = z;
  ಯದಿ( z==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯಾವತ್( *z2 ){ z2++; }
  ಪ್ರತ್ಯಾಗಮ 0x3fffffff & (ಪೂರ್ಣಾಙ್ಕ)(z2 - z);
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
  ಧ್ರುವ ವರ್ಣ *zBase,              /* Filename for chunk 0 */
  ಪೂರ್ಣಾಙ್ಕ nBase,                      /* Size of zBase in bytes (without \0) */
  ಪೂರ್ಣಾಙ್ಕ flags,                      /* Flags used to open file */
  ಪೂರ್ಣಾಙ್ಕ iChunk,                     /* Chunk to generate filename for */
  ವರ್ಣ *zOut                      /* Buffer to write generated name to */
){
  ಪೂರ್ಣಾಙ್ಕ n = nBase;
  memcpy(zOut, zBase, n+1);
  ಯದಿ( iChunk!=0 && iChunk<=MX_CHUNK_ಸಂಖ್ಯೆ ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    ಯದಿ( i>=n-4 ) n = i+1;
    ಯದಿ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }ಅನ್ಯಥಾ ಯದಿ( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_OFFSET;
    }
#endif
    sqlite3_snprintf(4,&zOut[n],"%03d",iChunk);
    n += 3;
  }

  ಪ್ರತಿಜ್ಞಾ( zOut[n]=='\0' );
  zOut[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexSubFilename(multiplexGroup *pGroup, ಪೂರ್ಣಾಙ್ಕ iChunk){
  ಯದಿ( iChunk>=pGroup->nReal ){
    ಸಂರಚನಾ multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*ಪರಿಮಾಣ(*p));
    ಯದಿ( p==0 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, ಪರಿಮಾಣ(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  ಯದಿ( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    ವರ್ಣ *z;
    ಪೂರ್ಣಾಙ್ಕ n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    ಯದಿ( z==0 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (ವರ್ಣ*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    ಯದಿ( pGroup->aReal[iChunk].z==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
ಸ್ಥಿರ sqlite3_file *multiplexSubOpen(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ಪೂರ್ಣಾಙ್ಕ iChunk,                /* Which chunk to open.  0==original file */
  ಪೂರ್ಣಾಙ್ಕ *rc,                   /* Result code in and out */
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags,            /* Output flags */
  ಪೂರ್ಣಾಙ್ಕ createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  ಯದಿ( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  ಯದಿ( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    ಪೂರ್ಣಾಙ್ಕ flags, bExists;
    flags = pGroup->flags;
    ಯದಿ( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }ಅನ್ಯಥಾ ಯದಿ( iChunk==0 ){
      /* Fall through */
    }ಅನ್ಯಥಾ ಯದಿ( pGroup->aReal[iChunk].z==0 ){
      ಪ್ರತ್ಯಾಗಮ 0;
    }ಅನ್ಯಥಾ{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     ಯದಿ( *rc || !bExists ){
        ಯದಿ( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        ಪ್ರತ್ಯಾಗಮ 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    ಯದಿ( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    ಯದಿ( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
ಸ್ಥಿರ sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ಪೂರ್ಣಾಙ್ಕ iChunk,                /* Which chunk to open.  0==original file */
  ಪೂರ್ಣಾಙ್ಕ *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  ಯದಿ( *rc ) ಪ್ರತ್ಯಾಗಮ 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSub==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  ಪ್ರತ್ಯಾಗಮ sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ multiplexControlFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ಪೂರ್ಣಾಙ್ಕ op = 0;
  ಪೂರ್ಣಾಙ್ಕ iVal;

  ಯದಿ( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ{
    /* extract params */
    op = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
    iVal = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
    /* map function op to file_control op */
    ವಿಕಲ್ಪ( op ){
      ಪಕ್ಷ 1:
        op = MULTIPLEX_CTRL_ENABLE;
        ಭಙ್ಗ;
      ಪಕ್ಷ 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_SIZE;
        ಭಙ್ಗ;
      ಪಕ್ಷ 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        ಭಙ್ಗ;
      ಸ್ವಭಾವ:
        rc = SQLITE_NOTFOUND;
        ಭಙ್ಗ;
    }
  }
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexFuncInit(
  sqlite3 *db,
  ವರ್ಣ **pz_Err_ಸಂದೇಶ,
  ಧ್ರುವ sqlite3_api_routines *pApi
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Close a single sub-file in the connection group.
*/
ಸ್ಥಿರ ಶೂನ್ಯ multiplexSubClose(
  multiplexGroup *pGroup,
  ಪೂರ್ಣಾಙ್ಕ iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  ಯದಿ( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    ಯದಿ( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  memset(&pGroup->aReal[iChunk], 0, ಪರಿಮಾಣ(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
ಸ್ಥಿರ ಶೂನ್ಯ multiplexFreeComponents(multiplexGroup *pGroup){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
  sqlite3_free(pGroup->aReal);
  pGroup->aReal = 0;
  pGroup->nReal = 0;
}


/************************* VFS Method Wrappers *****************************/

/*
** This is the xOpen method used for the "multiplex" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate multiplex group if it is a
** file that needs to be tracked.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ಧ್ರುವ ವರ್ಣ *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  ಪೂರ್ಣಾಙ್ಕ flags,                 /* Flags to control the opening */
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags             /* Flags showing results of opening */
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  ಪೂರ್ಣಾಙ್ಕ nName = 0;
  ಪೂರ್ಣಾಙ್ಕ sz = 0;
  ವರ್ಣ *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  ಪ್ರತಿಜ್ಞಾ( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  ಯದಿ( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = ಪರಿಮಾಣ(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    ಯದಿ( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  ಯದಿ( rc==SQLITE_OK ){
    ಧ್ರುವ ವರ್ಣ *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned ವರ್ಣ)-1;
    pGroup->bTruncate = (unsigned ವರ್ಣ)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (ಪೂರ್ಣಾಙ್ಕ)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    ಯದಿ( zName ){
      ವರ್ಣ *p = (ವರ್ಣ *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    ಯದಿ( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3PendingByte;
#ಅನ್ಯಥಾ
      ಪೂರ್ಣಾಙ್ಕ sqlite3PendingByte = 0x40000000;
#endif
      ಯಾವತ್( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    ಯದಿ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      ಯದಿ( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    ಯದಿ( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      ಯದಿ( rc==SQLITE_OK && zName ){
        ಪೂರ್ಣಾಙ್ಕ bExists;
        ಯದಿ( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }ಅನ್ಯಥಾ
        ಯದಿ( sz64==0 ){
          ಯದಿ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            ಪೂರ್ಣಾಙ್ಕ iChunk = 1;
            ಕುರು {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              ಯದಿ( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                ಯದಿ( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }ಯಾವತ್( rc==SQLITE_OK && bExists );
          }
        }ಅನ್ಯಥಾ{
          /* If the first overflow file exists and if the size of the main file
          ** is different from the chunk size, that means the chunk size is set
          ** set incorrectly.  So fix it.
          **
          ** Or, if the first overflow file does not exist and the main file is
          ** larger than the chunk size, that means the chunk size is too small.
          ** But we have no way of determining the intended chunk size, so
          ** just disable the multiplexor all together.
          */
          rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[1].z,
              SQLITE_ACCESS_EXISTS, &bExists);
          bExists = multiplexSubSize(pGroup, 1, &rc)>0;
          ಯದಿ( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (ಪೂರ್ಣಾಙ್ಕ)sz64;
          }ಅನ್ಯಥಾ ಯದಿ( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    ಯದಿ( rc==SQLITE_OK ){
      ಯದಿ( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }ಅನ್ಯಥಾ{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }ಅನ್ಯಥಾ{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ಧ್ರುವ ವರ್ಣ *zName,         /* Name of file to delete */
  ಪೂರ್ಣಾಙ್ಕ syncDir
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  ಯದಿ( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    ಪೂರ್ಣಾಙ್ಕ nName = (ಪೂರ್ಣಾಙ್ಕ)strlen(zName);
    ವರ್ಣ *z;
    z = sqlite3_malloc64(nName + 5);
    ಯದಿ( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ iChunk = 0;
      ಪೂರ್ಣಾಙ್ಕ bExists;
      ಕುರು{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }ಯಾವತ್( rc==SQLITE_OK && bExists );
      ಯಾವತ್( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      ಯದಿ( rc==SQLITE_OK ){
        iChunk = 0;
        ಕುರು{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }ಯಾವತ್( rc==SQLITE_OK && bExists );
        ಯಾವತ್( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexAccess(sqlite3_vfs *a, ಧ್ರುವ ವರ್ಣ *b, ಪೂರ್ಣಾಙ್ಕ c, ಪೂರ್ಣಾಙ್ಕ *d){
  ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexFullPathname(sqlite3_vfs *a, ಧ್ರುವ ವರ್ಣ *b, ಪೂರ್ಣಾಙ್ಕ c, ವರ್ಣ *d){
  ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
ಸ್ಥಿರ ಶೂನ್ಯ *multiplexDlOpen(sqlite3_vfs *a, ಧ್ರುವ ವರ್ಣ *b){
  ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
ಸ್ಥಿರ ಶೂನ್ಯ multiplexDlError(sqlite3_vfs *a, ಪೂರ್ಣಾಙ್ಕ b, ವರ್ಣ *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
ಸ್ಥಿರ ಶೂನ್ಯ (*multiplexDlSym(sqlite3_vfs *a, ಶೂನ್ಯ *b, ಧ್ರುವ ವರ್ಣ *c))(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
ಸ್ಥಿರ ಶೂನ್ಯ multiplexDlClose(sqlite3_vfs *a, ಶೂನ್ಯ *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexRandomness(sqlite3_vfs *a, ಪೂರ್ಣಾಙ್ಕ b, ವರ್ಣ *c){
  ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexSleep(sqlite3_vfs *a, ಪೂರ್ಣಾಙ್ಕ b){
  ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexCurrentTime(sqlite3_vfs *a, ದ್ವಿಗುಣ *b){
  ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexGetLastError(sqlite3_vfs *a, ಪೂರ್ಣಾಙ್ಕ b, ವರ್ಣ *c){
  ಯದಿ( gMultiplex.pOrigVfs->xGetLastError ){
    ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  ಪ್ರತ್ಯಾಗಮ gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexRead(
  sqlite3_file *pConn,
  ಶೂನ್ಯ *pBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ರಿಕ್ತ, 0);
    ಯದಿ( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }ಅನ್ಯಥಾ{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ಅನ್ಯಥಾ{
    ಯಾವತ್( iAmt > 0 ){
      ಪೂರ್ಣಾಙ್ಕ i = (ಪೂರ್ಣಾಙ್ಕ)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, ರಿಕ್ತ, 1);
      ಯದಿ( pSubOpen ){
        ಪೂರ್ಣಾಙ್ಕ extra = ((ಪೂರ್ಣಾಙ್ಕ)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        ಯದಿ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        ಯದಿ( rc!=SQLITE_OK ) ಭಙ್ಗ;
        pBuf = (ವರ್ಣ *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }ಅನ್ಯಥಾ{
        rc = SQLITE_IOERR_READ;
        ಭಙ್ಗ;
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexWrite(
  sqlite3_file *pConn,
  ಧ್ರುವ ಶೂನ್ಯ *pBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ರಿಕ್ತ, 0);
    ಯದಿ( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }ಅನ್ಯಥಾ{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ಅನ್ಯಥಾ{
    ಯಾವತ್( rc==SQLITE_OK && iAmt>0 ){
      ಪೂರ್ಣಾಙ್ಕ i = (ಪೂರ್ಣಾಙ್ಕ)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, ರಿಕ್ತ, 1);
      ಯದಿ( pSubOpen ){
        ಪೂರ್ಣಾಙ್ಕ extra = ((ಪೂರ್ಣಾಙ್ಕ)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        ಯದಿ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (ವರ್ಣ *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ರಿಕ್ತ, 0);
    ಯದಿ( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }ಅನ್ಯಥಾ{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪೂರ್ಣಾಙ್ಕ iBaseGroup = (ಪೂರ್ಣಾಙ್ಕ)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    ಪ್ರತಿ(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      ಯದಿ( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }ಅನ್ಯಥಾ{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        ಯದಿ( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    ಯದಿ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      ಯದಿ( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    ಯದಿ( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexSync(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    ಯದಿ( pSubOpen ){
      ಪೂರ್ಣಾಙ್ಕ rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      ಯದಿ( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ರಿಕ್ತ, 0);
    ಯದಿ( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }ಅನ್ಯಥಾ{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }ಅನ್ಯಥಾ{
    *pSize = 0;
    ಪ್ರತಿ(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      ಯದಿ( sz==0 ) ಭಙ್ಗ;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexLock(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen ){
    ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexUnlock(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen ){
    ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexCheckReservedLock(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen ){
    ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexFileControl(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  ಯದಿ( !gMultiplex.isInitialized ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE;
  ವಿಕಲ್ಪ( op ){
    ಪಕ್ಷ MULTIPLEX_CTRL_ENABLE:
      ಯದಿ( pArg ) {
        ಪೂರ್ಣಾಙ್ಕ bEnabled = *(ಪೂರ್ಣಾಙ್ಕ *)pArg;
        pGroup->bEnabled = (unsigned ವರ್ಣ)bEnabled;
        rc = SQLITE_OK;
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      ಯದಿ( pArg ) {
        unsigned ಪೂರ್ಣಾಙ್ಕ szChunk = *(unsigned*)pArg;
        ಯದಿ( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }ಅನ್ಯಥಾ{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_SIZE-1));
          szChunk &= ~(MAX_PAGE_SIZE-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      ಭಙ್ಗ;
    ಪಕ್ಷ SQLITE_FCNTL_SIZE_HINT:
    ಪಕ್ಷ SQLITE_FCNTL_CHUNK_SIZE:
      /* no-op these */
      rc = SQLITE_OK;
      ಭಙ್ಗ;
    ಪಕ್ಷ SQLITE_FCNTL_PRAGMA: {
      ವರ್ಣ **aFcntl = (ವರ್ಣ**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      ಯದಿ( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (ಶೂನ್ಯ)multiplexFileSize(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        ಯದಿ( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          ಯದಿ( aFcntl[2] && aFcntl[2][0] ){
            ಯದಿ( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }ಅನ್ಯಥಾ
            ಯದಿ( sqlite3_stricmp(aFcntl[2], "off")==0
             || sqlite3_stricmp(aFcntl[2], "0")==0 ){
              pGroup->bTruncate = 0;
            }
          }
          /* EVIDENCE-OF: R-27806-26076 The handler for an SQLITE_FCNTL_PRAGMA
          ** file control can optionally make the first element of the char**
          ** argument point to a string obtained from sqlite3_mprintf() or the
          ** equivalent and that string will become the result of the pragma
          ** or the error message if the pragma fails.
          */
          aFcntl[0] = sqlite3_mprintf(pGroup->bTruncate ? "on" : "off");
          rc = SQLITE_OK;
          ಭಙ್ಗ;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        ಯದಿ( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          ಭಙ್ಗ;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        ಯದಿ( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          ಭಙ್ಗ;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        ಯದಿ( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          ಪೂರ್ಣಾಙ್ಕ n = 0;
          ಪೂರ್ಣಾಙ್ಕ ii;
          ಪ್ರತಿ(ii=0; ii<pGroup->nReal; ii++){
            ಯದಿ( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          ಭಙ್ಗ;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    ಸ್ವಭಾವ:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ರಿಕ್ತ, 0);
      ಯದಿ( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        ಯದಿ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(ವರ್ಣ**)pArg = sqlite3_mprintf("multiplex/%z", *(ವರ್ಣ**)pArg);
        }
      }
      ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  ಪ್ರತ್ಯಾಗಮ DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen ){
    ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ಪೂರ್ಣಾಙ್ಕ iRegion,                    /* Region to retrieve */
  ಪೂರ್ಣಾಙ್ಕ szRegion,                   /* Size of regions */
  ಪೂರ್ಣಾಙ್ಕ bExtend,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಚಞ್ಚಲ **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen ){
    ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ಪೂರ್ಣಾಙ್ಕ ofst,                  /* First lock to acquire or release */
  ಪೂರ್ಣಾಙ್ಕ n,                     /* Number of locks to acquire or release */
  ಪೂರ್ಣಾಙ್ಕ flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen ){
    ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಶೂನ್ಯ multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiplexShmUnmap(sqlite3_file *pConn, ಪೂರ್ಣಾಙ್ಕ deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ರಿಕ್ತ, 0);
  ಯದಿ( pSubOpen ){
    ಪ್ರತ್ಯಾಗಮ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_multiplex_initialize(ಧ್ರುವ ವರ್ಣ *zOrigVfsName, ಪೂರ್ಣಾಙ್ಕ makeDefault){
  sqlite3_vfs *pOrigVfs;
  ಯದಿ( gMultiplex.isInitialized ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  ಯದಿ( pOrigVfs==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  ಪ್ರತಿಜ್ಞಾ( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += ಪರಿಮಾಣ(multiplexConn);
  gMultiplex.sThisVfs.zName = SQLITE_MULTIPLEX_VFS_NAME;
  gMultiplex.sThisVfs.xOpen = multiplexOpen;
  gMultiplex.sThisVfs.xDelete = multiplexDelete;
  gMultiplex.sThisVfs.xAccess = multiplexAccess;
  gMultiplex.sThisVfs.xFullPathname = multiplexFullPathname;
  gMultiplex.sThisVfs.xDlOpen = multiplexDlOpen;
  gMultiplex.sThisVfs.xDlError = multiplexDlError;
  gMultiplex.sThisVfs.xDlSym = multiplexDlSym;
  gMultiplex.sThisVfs.xDlClose = multiplexDlClose;
  gMultiplex.sThisVfs.xRandomness = multiplexRandomness;
  gMultiplex.sThisVfs.xSleep = multiplexSleep;
  gMultiplex.sThisVfs.xCurrentTime = multiplexCurrentTime;
  gMultiplex.sThisVfs.xGetLastError = multiplexGetLastError;
  gMultiplex.sThisVfs.xCurrentTimeInt64 = multiplexCurrentTimeInt64;

  gMultiplex.sIoMethodsV1.iVersion = 1;
  gMultiplex.sIoMethodsV1.xClose = multiplexClose;
  gMultiplex.sIoMethodsV1.xRead = multiplexRead;
  gMultiplex.sIoMethodsV1.xWrite = multiplexWrite;
  gMultiplex.sIoMethodsV1.xTruncate = multiplexTruncate;
  gMultiplex.sIoMethodsV1.xSync = multiplexSync;
  gMultiplex.sIoMethodsV1.xFileSize = multiplexFileSize;
  gMultiplex.sIoMethodsV1.xLock = multiplexLock;
  gMultiplex.sIoMethodsV1.xUnlock = multiplexUnlock;
  gMultiplex.sIoMethodsV1.xCheckReservedLock = multiplexCheckReservedLock;
  gMultiplex.sIoMethodsV1.xFileControl = multiplexFileControl;
  gMultiplex.sIoMethodsV1.xSectorSize = multiplexSectorSize;
  gMultiplex.sIoMethodsV1.xDeviceCharacteristics =
                                            multiplexDeviceCharacteristics;
  gMultiplex.sIoMethodsV2 = gMultiplex.sIoMethodsV1;
  gMultiplex.sIoMethodsV2.iVersion = 2;
  gMultiplex.sIoMethodsV2.xShmMap = multiplexShmMap;
  gMultiplex.sIoMethodsV2.xShmLock = multiplexShmLock;
  gMultiplex.sIoMethodsV2.xShmBarrier = multiplexShmBarrier;
  gMultiplex.sIoMethodsV2.xShmUnmap = multiplexShmUnmap;
  sqlite3_vfs_register(&gMultiplex.sThisVfs, makeDefault);

  sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))multiplexFuncInit);

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_multiplex_shutdown(ಪೂರ್ಣಾಙ್ಕ eForce){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( gMultiplex.isInitialized==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, ಪರಿಮಾಣ(gMultiplex));
  ಪ್ರತ್ಯಾಗಮ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_multiplex_initialize(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *zName;              /* Name of new multiplex VFS */
  ಪೂರ್ಣಾಙ್ಕ makeDefault;                /* True to make the new VFS the default */
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_multiplex_shutdown(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  ಯದಿ( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  ಯದಿ( (objc!=1 && objc!=2) ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "?-force?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Return code from file_control() */
  ಪೂರ್ಣಾಙ್ಕ idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  ಪೂರ್ಣಾಙ್ಕ i_ಮೌಲ್ಯ = 0;
  ಶೂನ್ಯ *pArg = 0;

  ಸಂರಚನಾ SubCommand {
    ಧ್ರುವ ವರ್ಣ *zName;
    ಪೂರ್ಣಾಙ್ಕ op;
    ಪೂರ್ಣಾಙ್ಕ argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  ಯದಿ( objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", ರಿಕ್ತ);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }ಅನ್ಯಥಾ{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, ಪರಿಮಾಣ(aSub[0]), "sub-command", 0, &idx
  );
  ಯದಿ( rc!=TCL_OK ) ಪ್ರತ್ಯಾಗಮ rc;

  ವಿಕಲ್ಪ( aSub[idx].argtype ){
    ಪಕ್ಷ 1:
      ಯದಿ( Tcl_GetIntFromObj(interp, objv[4], &i_ಮೌಲ್ಯ) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      pArg = (ಶೂನ್ಯ *)&i_ಮೌಲ್ಯ;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      Tcl_Wrong_ಅಂಕೆ_Args(interp, 4, objv, "SUB-COMMAND");
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);
  ಪ್ರತ್ಯಾಗಮ (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitemultiplex_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aCmd)/ಪರಿಮಾಣ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
#endif
