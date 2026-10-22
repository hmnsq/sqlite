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
#include "sqlite3.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "test_multiplex.h"

#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#include "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#define UNUSED_PARAMETER(x) (void)(x)
#define MAX_PAGE_SIZE       0x10000
#define DEFAULT_SECTOR_SIZE 0x1000

/* Maximum chunk number */
#define MX_CHUNK_NUMBER 299

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
typedef sa_nracanaa multiplexGroup multiplexGroup;
typedef sa_nracanaa multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
sa_nracanaa multiplexGroup {
  sa_nracanaa multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    var_na *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  ຈຳນວນເຕັມ nReal;                       /* Number of chunks */
  var_na *zName;                     /* Base filename of this group */
  ຈຳນວນເຕັມ nName;                       /* Length of base filename */
  ຈຳນວນເຕັມ flags;                       /* Flags used for original opening */
  unsigned ຈຳນວນເຕັມ szChunk;            /* Chunk size used for this group */
  unsigned var_na bEnabled;          /* TRUE to use Multiplex VFS for this file */
  unsigned var_na bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
sa_nracanaa multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
static sa_nracanaa {
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
  ຈຳນວນເຕັມ isInitialized;
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
static ຈຳນວນເຕັມ multiplexStrlen30(const var_na *z){
  const var_na *z2 = z;
  ຖ້າ( z==0 ) ສົ່ງຄືນ 0;
  ໃນຂະນະທີ່( *z2 ){ z2++; }
  ສົ່ງຄືນ 0x3fffffff & (ຈຳນວນເຕັມ)(z2 - z);
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
static void multiplexFilename(
  const var_na *zBase,              /* Filename for chunk 0 */
  ຈຳນວນເຕັມ nBase,                      /* Size of zBase in bytes (without \0) */
  ຈຳນວນເຕັມ flags,                      /* Flags used to open file */
  ຈຳນວນເຕັມ iChunk,                     /* Chunk to generate filename for */
  var_na *zOut                      /* Buffer to write generated name to */
){
  ຈຳນວນເຕັມ n = nBase;
  memcpy(zOut, zBase, n+1);
  ຖ້າ( iChunk!=0 && iChunk<=MX_CHUNK_NUMBER ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    ຈຳນວນເຕັມ i;
    ສຳລັບ(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    ຖ້າ( i>=n-4 ) n = i+1;
    ຖ້າ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }ບໍ່ດັ່ງນັ້ນ ຖ້າ( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_OFFSET;
    }
#endif
    sqlite3_snprintf(4,&zOut[n],"%03d",iChunk);
    n += 3;
  }

  assert( zOut[n]=='\0' );
  zOut[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
static ຈຳນວນເຕັມ multiplexSubFilename(multiplexGroup *pGroup, ຈຳນວນເຕັມ iChunk){
  ຖ້າ( iChunk>=pGroup->nReal ){
    sa_nracanaa multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*parimaa_na(*p));
    ຖ້າ( p==0 ){
      ສົ່ງຄືນ SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, parimaa_na(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  ຖ້າ( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    var_na *z;
    ຈຳນວນເຕັມ n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    ຖ້າ( z==0 ){
      ສົ່ງຄືນ SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (var_na*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    ຖ້າ( pGroup->aReal[iChunk].z==0 ) ສົ່ງຄືນ SQLITE_NOMEM;
  }
  ສົ່ງຄືນ SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
static sqlite3_file *multiplexSubOpen(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ຈຳນວນເຕັມ iChunk,                /* Which chunk to open.  0==original file */
  ຈຳນວນເຕັມ *rc,                   /* Result code in and out */
  ຈຳນວນເຕັມ *pOutFlags,            /* Output flags */
  ຈຳນວນເຕັມ createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  ຖ້າ( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    ສົ່ງຄືນ 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  ຖ້າ( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    ຈຳນວນເຕັມ flags, bExists;
    flags = pGroup->flags;
    ຖ້າ( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }ບໍ່ດັ່ງນັ້ນ ຖ້າ( iChunk==0 ){
      /* Fall through */
    }ບໍ່ດັ່ງນັ້ນ ຖ້າ( pGroup->aReal[iChunk].z==0 ){
      ສົ່ງຄືນ 0;
    }ບໍ່ດັ່ງນັ້ນ{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     ຖ້າ( *rc || !bExists ){
        ຖ້າ( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        ສົ່ງຄືນ 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    ຖ້າ( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      ສົ່ງຄືນ 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    ຖ້າ( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      ສົ່ງຄືນ 0;
    }
  }
  ສົ່ງຄືນ pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
static sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ຈຳນວນເຕັມ iChunk,                /* Which chunk to open.  0==original file */
  ຈຳນວນເຕັມ *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  ຖ້າ( *rc ) ສົ່ງຄືນ 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, NULL, 0);
  ຖ້າ( pSub==0 ) ສົ່ງຄືນ 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  ສົ່ງຄືນ sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
static void multiplexControlFunc(
  sqlite3_context *context,
  ຈຳນວນເຕັມ argc,
  sqlite3_value **argv
){
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ຈຳນວນເຕັມ op = 0;
  ຈຳນວນເຕັມ iVal;

  ຖ້າ( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }ບໍ່ດັ່ງນັ້ນ{
    /* extract params */
    op = sqlite3_value_int(argv[0]);
    iVal = sqlite3_value_int(argv[1]);
    /* map function op to file_control op */
    switch( op ){
      pak_sa 1:
        op = MULTIPLEX_CTRL_ENABLE;
        bha_nga;
      pak_sa 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_SIZE;
        bha_nga;
      pak_sa 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        bha_nga;
      default:
        rc = SQLITE_NOTFOUND;
        bha_nga;
    }
  }
  ຖ້າ( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
static ຈຳນວນເຕັມ multiplexFuncInit(
  sqlite3 *db,
  var_na **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  ຈຳນວນເຕັມ rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  ສົ່ງຄືນ rc;
}

/*
** Close a single sub-file in the connection group.
*/
static void multiplexSubClose(
  multiplexGroup *pGroup,
  ຈຳນວນເຕັມ iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  ຖ້າ( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    ຖ້າ( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  memset(&pGroup->aReal[iChunk], 0, parimaa_na(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
static void multiplexFreeComponents(multiplexGroup *pGroup){
  ຈຳນວນເຕັມ i;
  ສຳລັບ(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
static ຈຳນວນເຕັມ multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const var_na *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  ຈຳນວນເຕັມ flags,                 /* Flags to control the opening */
  ຈຳນວນເຕັມ *pOutFlags             /* Flags showing results of opening */
){
  ຈຳນວນເຕັມ rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  ຈຳນວນເຕັມ nName = 0;
  ຈຳນວນເຕັມ sz = 0;
  var_na *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  assert( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  ຖ້າ( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = parimaa_na(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    ຖ້າ( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  ຖ້າ( rc==SQLITE_OK ){
    const var_na *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned var_na)-1;
    pGroup->bTruncate = (unsigned var_na)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (ຈຳນວນເຕັມ)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    ຖ້າ( zName ){
      var_na *p = (var_na *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    ຖ້າ( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      extern ຈຳນວນເຕັມ sqlite3PendingByte;
#ບໍ່ດັ່ງນັ້ນ
      ຈຳນວນເຕັມ sqlite3PendingByte = 0x40000000;
#endif
      ໃນຂະນະທີ່( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    ຖ້າ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      ຖ້າ( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    ຖ້າ( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      ຖ້າ( rc==SQLITE_OK && zName ){
        ຈຳນວນເຕັມ bExists;
        ຖ້າ( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }ບໍ່ດັ່ງນັ້ນ
        ຖ້າ( sz64==0 ){
          ຖ້າ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            ຈຳນວນເຕັມ iChunk = 1;
            do {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              ຖ້າ( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                ຖ້າ( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }ໃນຂະນະທີ່( rc==SQLITE_OK && bExists );
          }
        }ບໍ່ດັ່ງນັ້ນ{
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
          ຖ້າ( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (ຈຳນວນເຕັມ)sz64;
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    ຖ້າ( rc==SQLITE_OK ){
      ຖ້າ( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }ບໍ່ດັ່ງນັ້ນ{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }ບໍ່ດັ່ງນັ້ນ{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  ສົ່ງຄືນ rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
static ຈຳນວນເຕັມ multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const var_na *zName,         /* Name of file to delete */
  ຈຳນວນເຕັມ syncDir
){
  ຈຳນວນເຕັມ rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  ຖ້າ( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    ຈຳນວນເຕັມ nName = (ຈຳນວນເຕັມ)strlen(zName);
    var_na *z;
    z = sqlite3_malloc64(nName + 5);
    ຖ້າ( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }ບໍ່ດັ່ງນັ້ນ{
      ຈຳນວນເຕັມ iChunk = 0;
      ຈຳນວນເຕັມ bExists;
      do{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }ໃນຂະນະທີ່( rc==SQLITE_OK && bExists );
      ໃນຂະນະທີ່( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      ຖ້າ( rc==SQLITE_OK ){
        iChunk = 0;
        do{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }ໃນຂະນະທີ່( rc==SQLITE_OK && bExists );
        ໃນຂະນະທີ່( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  ສົ່ງຄືນ rc;
}

static ຈຳນວນເຕັມ multiplexAccess(sqlite3_vfs *a, const var_na *b, ຈຳນວນເຕັມ c, ຈຳນວນເຕັມ *d){
  ສົ່ງຄືນ gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
static ຈຳນວນເຕັມ multiplexFullPathname(sqlite3_vfs *a, const var_na *b, ຈຳນວນເຕັມ c, var_na *d){
  ສົ່ງຄືນ gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
static void *multiplexDlOpen(sqlite3_vfs *a, const var_na *b){
  ສົ່ງຄືນ gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
static void multiplexDlError(sqlite3_vfs *a, ຈຳນວນເຕັມ b, var_na *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
static void (*multiplexDlSym(sqlite3_vfs *a, void *b, const var_na *c))(void){
  ສົ່ງຄືນ gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
static void multiplexDlClose(sqlite3_vfs *a, void *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
static ຈຳນວນເຕັມ multiplexRandomness(sqlite3_vfs *a, ຈຳນວນເຕັມ b, var_na *c){
  ສົ່ງຄືນ gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
static ຈຳນວນເຕັມ multiplexSleep(sqlite3_vfs *a, ຈຳນວນເຕັມ b){
  ສົ່ງຄືນ gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
static ຈຳນວນເຕັມ multiplexCurrentTime(sqlite3_vfs *a, dviguu_na *b){
  ສົ່ງຄືນ gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
static ຈຳນວນເຕັມ multiplexGetLastError(sqlite3_vfs *a, ຈຳນວນເຕັມ b, var_na *c){
  ຖ້າ( gMultiplex.pOrigVfs->xGetLastError ){
    ສົ່ງຄືນ gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }ບໍ່ດັ່ງນັ້ນ{
    ສົ່ງຄືນ 0;
  }
}
static ຈຳນວນເຕັມ multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  ສົ່ງຄືນ gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
static ຈຳນວນເຕັມ multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  ສົ່ງຄືນ rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
static ຈຳນວນເຕັມ multiplexRead(
  sqlite3_file *pConn,
  void *pBuf,
  ຈຳນວນເຕັມ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  ຖ້າ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    ຖ້າ( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }ບໍ່ດັ່ງນັ້ນ{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ບໍ່ດັ່ງນັ້ນ{
    ໃນຂະນະທີ່( iAmt > 0 ){
      ຈຳນວນເຕັມ i = (ຈຳນວນເຕັມ)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      ຖ້າ( pSubOpen ){
        ຈຳນວນເຕັມ extra = ((ຈຳນວນເຕັມ)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        ຖ້າ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        ຖ້າ( rc!=SQLITE_OK ) bha_nga;
        pBuf = (var_na *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }ບໍ່ດັ່ງນັ້ນ{
        rc = SQLITE_IOERR_READ;
        bha_nga;
      }
    }
  }

  ສົ່ງຄືນ rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
static ຈຳນວນເຕັມ multiplexWrite(
  sqlite3_file *pConn,
  const void *pBuf,
  ຈຳນວນເຕັມ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  ຖ້າ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    ຖ້າ( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }ບໍ່ດັ່ງນັ້ນ{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ບໍ່ດັ່ງນັ້ນ{
    ໃນຂະນະທີ່( rc==SQLITE_OK && iAmt>0 ){
      ຈຳນວນເຕັມ i = (ຈຳນວນເຕັມ)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      ຖ້າ( pSubOpen ){
        ຈຳນວນເຕັມ extra = ((ຈຳນວນເຕັມ)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        ຖ້າ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (var_na *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  ສົ່ງຄືນ rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
static ຈຳນວນເຕັມ multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  ຖ້າ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    ຖ້າ( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }ບໍ່ດັ່ງນັ້ນ{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }ບໍ່ດັ່ງນັ້ນ{
    ຈຳນວນເຕັມ i;
    ຈຳນວນເຕັມ iBaseGroup = (ຈຳນວນເຕັມ)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    ສຳລັບ(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      ຖ້າ( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }ບໍ່ດັ່ງນັ້ນ{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        ຖ້າ( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    ຖ້າ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      ຖ້າ( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    ຖ້າ( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  ສົ່ງຄືນ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
static ຈຳນວນເຕັມ multiplexSync(sqlite3_file *pConn, ຈຳນວນເຕັມ flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  ຈຳນວນເຕັມ i;
  ສຳລັບ(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    ຖ້າ( pSubOpen ){
      ຈຳນວນເຕັມ rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      ຖ້າ( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  ສົ່ງຄືນ rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
static ຈຳນວນເຕັມ multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  ຈຳນວນເຕັມ i;
  ຖ້າ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    ຖ້າ( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }ບໍ່ດັ່ງນັ້ນ{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }ບໍ່ດັ່ງນັ້ນ{
    *pSize = 0;
    ສຳລັບ(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      ຖ້າ( sz==0 ) bha_nga;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  ສົ່ງຄືນ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
static ຈຳນວນເຕັມ multiplexLock(sqlite3_file *pConn, ຈຳນວນເຕັມ lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen ){
    ສົ່ງຄືນ pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  ສົ່ງຄືນ SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
static ຈຳນວນເຕັມ multiplexUnlock(sqlite3_file *pConn, ຈຳນວນເຕັມ lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen ){
    ສົ່ງຄືນ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  ສົ່ງຄືນ SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
static ຈຳນວນເຕັມ multiplexCheckReservedLock(sqlite3_file *pConn, ຈຳນວນເຕັມ *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen ){
    ສົ່ງຄືນ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  ສົ່ງຄືນ SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
static ຈຳນວນເຕັມ multiplexFileControl(sqlite3_file *pConn, ຈຳນວນເຕັມ op, void *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ຈຳນວນເຕັມ rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  ຖ້າ( !gMultiplex.isInitialized ) ສົ່ງຄືນ SQLITE_MISUSE;
  switch( op ){
    pak_sa MULTIPLEX_CTRL_ENABLE:
      ຖ້າ( pArg ) {
        ຈຳນວນເຕັມ bEnabled = *(ຈຳນວນເຕັມ *)pArg;
        pGroup->bEnabled = (unsigned var_na)bEnabled;
        rc = SQLITE_OK;
      }
      bha_nga;
    pak_sa MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      ຖ້າ( pArg ) {
        unsigned ຈຳນວນເຕັມ szChunk = *(unsigned*)pArg;
        ຖ້າ( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }ບໍ່ດັ່ງນັ້ນ{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_SIZE-1));
          szChunk &= ~(MAX_PAGE_SIZE-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      bha_nga;
    pak_sa MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      bha_nga;
    pak_sa SQLITE_FCNTL_SIZE_HINT:
    pak_sa SQLITE_FCNTL_CHUNK_SIZE:
      /* no-op these */
      rc = SQLITE_OK;
      bha_nga;
    pak_sa SQLITE_FCNTL_PRAGMA: {
      var_na **aFcntl = (var_na**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      ຖ້າ( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (void)multiplexFileSize(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        ຖ້າ( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          ຖ້າ( aFcntl[2] && aFcntl[2][0] ){
            ຖ້າ( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }ບໍ່ດັ່ງນັ້ນ
            ຖ້າ( sqlite3_stricmp(aFcntl[2], "off")==0
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
          bha_nga;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        ຖ້າ( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          bha_nga;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        ຖ້າ( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          bha_nga;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        ຖ້າ( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          ຈຳນວນເຕັມ n = 0;
          ຈຳນວນເຕັມ ii;
          ສຳລັບ(ii=0; ii<pGroup->nReal; ii++){
            ຖ້າ( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          bha_nga;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    default:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
      ຖ້າ( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        ຖ້າ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(var_na**)pArg = sqlite3_mprintf("multiplex/%z", *(var_na**)pArg);
        }
      }
      bha_nga;
  }
  ສົ່ງຄືນ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
static ຈຳນວນເຕັມ multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    ສົ່ງຄືນ pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  ສົ່ງຄືນ DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
static ຈຳນວນເຕັມ multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen ){
    ສົ່ງຄືນ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  ສົ່ງຄືນ 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
static ຈຳນວນເຕັມ multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ຈຳນວນເຕັມ iRegion,                    /* Region to retrieve */
  ຈຳນວນເຕັມ szRegion,                   /* Size of regions */
  ຈຳນວນເຕັມ bExtend,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen ){
    ສົ່ງຄືນ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  ສົ່ງຄືນ SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
static ຈຳນວນເຕັມ multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ຈຳນວນເຕັມ ofst,                  /* First lock to acquire or release */
  ຈຳນວນເຕັມ n,                     /* Number of locks to acquire or release */
  ຈຳນວນເຕັມ flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen ){
    ສົ່ງຄືນ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  ສົ່ງຄືນ SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
static void multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
static ຈຳນວນເຕັມ multiplexShmUnmap(sqlite3_file *pConn, ຈຳນວນເຕັມ deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  ຈຳນວນເຕັມ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ຖ້າ( pSubOpen ){
    ສົ່ງຄືນ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  ສົ່ງຄືນ SQLITE_OK;
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
ຈຳນວນເຕັມ sqlite3_multiplex_initialize(const var_na *zOrigVfsName, ຈຳນວນເຕັມ makeDefault){
  sqlite3_vfs *pOrigVfs;
  ຖ້າ( gMultiplex.isInitialized ) ສົ່ງຄືນ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  ຖ້າ( pOrigVfs==0 ) ສົ່ງຄືນ SQLITE_ERROR;
  assert( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += parimaa_na(multiplexConn);
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

  sqlite3_auto_extension((void(*)(void))multiplexFuncInit);

  ສົ່ງຄືນ SQLITE_OK;
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
ຈຳນວນເຕັມ sqlite3_multiplex_shutdown(ຈຳນວນເຕັມ eForce){
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  ຖ້າ( gMultiplex.isInitialized==0 ) ສົ່ງຄືນ SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, parimaa_na(gMultiplex));
  ສົ່ງຄືນ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#include "tclsqlite.h"
extern const var_na *sqlite3ErrName(ຈຳນວນເຕັມ);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
static ຈຳນວນເຕັມ SQLITE_TCLAPI test_multiplex_initialize(
  void * clientData,
  Tcl_Interp *interp,
  ຈຳນວນເຕັມ objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zName;              /* Name of new multiplex VFS */
  ຈຳນວນເຕັມ makeDefault;                /* True to make the new VFS the default */
  ຈຳນວນເຕັມ rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  ຖ້າ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME MAKEDEFAULT");
    ສົ່ງຄືນ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ຖ້າ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ສົ່ງຄືນ TCL_ERROR;
  ຖ້າ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);

  ສົ່ງຄືນ TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
static ຈຳນວນເຕັມ SQLITE_TCLAPI test_multiplex_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  ຈຳນວນເຕັມ objc,
  Tcl_Obj *CONST objv[]
){
  ຈຳນວນເຕັມ rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  ຖ້າ( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  ຖ້າ( (objc!=1 && objc!=2) ){
    Tcl_WrongNumArgs(interp, 1, objv, "?-force?");
    ສົ່ງຄືນ TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);

  ສົ່ງຄືນ TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
static ຈຳນວນເຕັມ SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  ຈຳນວນເຕັມ objc,
  Tcl_Obj *CONST objv[]
){
  ຈຳນວນເຕັມ rc;                         /* Return code from file_control() */
  ຈຳນວນເຕັມ idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  ຈຳນວນເຕັມ iValue = 0;
  void *pArg = 0;

  sa_nracanaa SubCommand {
    const var_na *zName;
    ຈຳນວນເຕັມ op;
    ຈຳນວນເຕັມ argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  ຖ້າ( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    ສົ່ງຄືນ TCL_ERROR;
  }

  ຖ້າ( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", NULL);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", NULL);
    ສົ່ງຄືນ TCL_ERROR;
  }ບໍ່ດັ່ງນັ້ນ{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, parimaa_na(aSub[0]), "sub-command", 0, &idx
  );
  ຖ້າ( rc!=TCL_OK ) ສົ່ງຄືນ rc;

  switch( aSub[idx].argtype ){
    pak_sa 1:
      ຖ້າ( Tcl_GetIntFromObj(interp, objv[4], &iValue) ){
        ສົ່ງຄືນ TCL_ERROR;
      }
      pArg = (void *)&iValue;
      bha_nga;
    default:
      Tcl_WrongNumArgs(interp, 4, objv, "SUB-COMMAND");
      ສົ່ງຄືນ TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);
  ສົ່ງຄືນ (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ຈຳນວນເຕັມ Sqlitemultiplex_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  ຈຳນວນເຕັມ i;

  ສຳລັບ(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ສົ່ງຄືນ TCL_OK;
}
#endif
