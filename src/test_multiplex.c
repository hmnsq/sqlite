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
  ចំនួនគត់ nReal;                       /* Number of chunks */
  var_na *zName;                     /* Base filename of this group */
  ចំនួនគត់ nName;                       /* Length of base filename */
  ចំនួនគត់ flags;                       /* Flags used for original opening */
  unsigned ចំនួនគត់ szChunk;            /* Chunk size used for this group */
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
  ចំនួនគត់ isInitialized;
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
static ចំនួនគត់ multiplexStrlen30(const var_na *z){
  const var_na *z2 = z;
  ប្រសិនបើ( z==0 ) ត្រឡប់ 0;
  ខណៈពេល( *z2 ){ z2++; }
  ត្រឡប់ 0x3fffffff & (ចំនួនគត់)(z2 - z);
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
  ចំនួនគត់ nBase,                      /* Size of zBase in bytes (without \0) */
  ចំនួនគត់ flags,                      /* Flags used to open file */
  ចំនួនគត់ iChunk,                     /* Chunk to generate filename for */
  var_na *zOut                      /* Buffer to write generated name to */
){
  ចំនួនគត់ n = nBase;
  memcpy(zOut, zBase, n+1);
  ប្រសិនបើ( iChunk!=0 && iChunk<=MX_CHUNK_NUMBER ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    ចំនួនគត់ i;
    សម្រាប់(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    ប្រសិនបើ( i>=n-4 ) n = i+1;
    ប្រសិនបើ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }បើមិនដូច្នេះ ប្រសិនបើ( flags & SQLITE_OPEN_WAL ){
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
static ចំនួនគត់ multiplexSubFilename(multiplexGroup *pGroup, ចំនួនគត់ iChunk){
  ប្រសិនបើ( iChunk>=pGroup->nReal ){
    sa_nracanaa multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*parimaa_na(*p));
    ប្រសិនបើ( p==0 ){
      ត្រឡប់ SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, parimaa_na(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  ប្រសិនបើ( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    var_na *z;
    ចំនួនគត់ n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    ប្រសិនបើ( z==0 ){
      ត្រឡប់ SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (var_na*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    ប្រសិនបើ( pGroup->aReal[iChunk].z==0 ) ត្រឡប់ SQLITE_NOMEM;
  }
  ត្រឡប់ SQLITE_OK;
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
  ចំនួនគត់ iChunk,                /* Which chunk to open.  0==original file */
  ចំនួនគត់ *rc,                   /* Result code in and out */
  ចំនួនគត់ *pOutFlags,            /* Output flags */
  ចំនួនគត់ createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  ប្រសិនបើ( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    ត្រឡប់ 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  ប្រសិនបើ( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    ចំនួនគត់ flags, bExists;
    flags = pGroup->flags;
    ប្រសិនបើ( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }បើមិនដូច្នេះ ប្រសិនបើ( iChunk==0 ){
      /* Fall through */
    }បើមិនដូច្នេះ ប្រសិនបើ( pGroup->aReal[iChunk].z==0 ){
      ត្រឡប់ 0;
    }បើមិនដូច្នេះ{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     ប្រសិនបើ( *rc || !bExists ){
        ប្រសិនបើ( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        ត្រឡប់ 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    ប្រសិនបើ( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      ត្រឡប់ 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    ប្រសិនបើ( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      ត្រឡប់ 0;
    }
  }
  ត្រឡប់ pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
static sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ចំនួនគត់ iChunk,                /* Which chunk to open.  0==original file */
  ចំនួនគត់ *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  ប្រសិនបើ( *rc ) ត្រឡប់ 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, NULL, 0);
  ប្រសិនបើ( pSub==0 ) ត្រឡប់ 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  ត្រឡប់ sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
static void multiplexControlFunc(
  sqlite3_context *context,
  ចំនួនគត់ argc,
  sqlite3_value **argv
){
  ចំនួនគត់ rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ចំនួនគត់ op = 0;
  ចំនួនគត់ iVal;

  ប្រសិនបើ( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }បើមិនដូច្នេះ{
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
  ប្រសិនបើ( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
static ចំនួនគត់ multiplexFuncInit(
  sqlite3 *db,
  var_na **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  ចំនួនគត់ rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  ត្រឡប់ rc;
}

/*
** Close a single sub-file in the connection group.
*/
static void multiplexSubClose(
  multiplexGroup *pGroup,
  ចំនួនគត់ iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  ប្រសិនបើ( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    ប្រសិនបើ( pOrigVfs && pGroup->aReal[iChunk].z ){
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
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
static ចំនួនគត់ multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const var_na *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  ចំនួនគត់ flags,                 /* Flags to control the opening */
  ចំនួនគត់ *pOutFlags             /* Flags showing results of opening */
){
  ចំនួនគត់ rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  ចំនួនគត់ nName = 0;
  ចំនួនគត់ sz = 0;
  var_na *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  assert( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  ប្រសិនបើ( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = parimaa_na(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    ប្រសិនបើ( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  ប្រសិនបើ( rc==SQLITE_OK ){
    const var_na *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned var_na)-1;
    pGroup->bTruncate = (unsigned var_na)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (ចំនួនគត់)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    ប្រសិនបើ( zName ){
      var_na *p = (var_na *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    ប្រសិនបើ( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      extern ចំនួនគត់ sqlite3PendingByte;
#បើមិនដូច្នេះ
      ចំនួនគត់ sqlite3PendingByte = 0x40000000;
#endif
      ខណៈពេល( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    ប្រសិនបើ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      ប្រសិនបើ( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    ប្រសិនបើ( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      ប្រសិនបើ( rc==SQLITE_OK && zName ){
        ចំនួនគត់ bExists;
        ប្រសិនបើ( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }បើមិនដូច្នេះ
        ប្រសិនបើ( sz64==0 ){
          ប្រសិនបើ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            ចំនួនគត់ iChunk = 1;
            do {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              ប្រសិនបើ( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                ប្រសិនបើ( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }ខណៈពេល( rc==SQLITE_OK && bExists );
          }
        }បើមិនដូច្នេះ{
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
          ប្រសិនបើ( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (ចំនួនគត់)sz64;
          }បើមិនដូច្នេះ ប្រសិនបើ( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    ប្រសិនបើ( rc==SQLITE_OK ){
      ប្រសិនបើ( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }បើមិនដូច្នេះ{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }បើមិនដូច្នេះ{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  ត្រឡប់ rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
static ចំនួនគត់ multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const var_na *zName,         /* Name of file to delete */
  ចំនួនគត់ syncDir
){
  ចំនួនគត់ rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  ប្រសិនបើ( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    ចំនួនគត់ nName = (ចំនួនគត់)strlen(zName);
    var_na *z;
    z = sqlite3_malloc64(nName + 5);
    ប្រសិនបើ( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }បើមិនដូច្នេះ{
      ចំនួនគត់ iChunk = 0;
      ចំនួនគត់ bExists;
      do{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }ខណៈពេល( rc==SQLITE_OK && bExists );
      ខណៈពេល( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      ប្រសិនបើ( rc==SQLITE_OK ){
        iChunk = 0;
        do{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }ខណៈពេល( rc==SQLITE_OK && bExists );
        ខណៈពេល( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  ត្រឡប់ rc;
}

static ចំនួនគត់ multiplexAccess(sqlite3_vfs *a, const var_na *b, ចំនួនគត់ c, ចំនួនគត់ *d){
  ត្រឡប់ gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
static ចំនួនគត់ multiplexFullPathname(sqlite3_vfs *a, const var_na *b, ចំនួនគត់ c, var_na *d){
  ត្រឡប់ gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
static void *multiplexDlOpen(sqlite3_vfs *a, const var_na *b){
  ត្រឡប់ gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
static void multiplexDlError(sqlite3_vfs *a, ចំនួនគត់ b, var_na *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
static void (*multiplexDlSym(sqlite3_vfs *a, void *b, const var_na *c))(void){
  ត្រឡប់ gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
static void multiplexDlClose(sqlite3_vfs *a, void *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
static ចំនួនគត់ multiplexRandomness(sqlite3_vfs *a, ចំនួនគត់ b, var_na *c){
  ត្រឡប់ gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
static ចំនួនគត់ multiplexSleep(sqlite3_vfs *a, ចំនួនគត់ b){
  ត្រឡប់ gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
static ចំនួនគត់ multiplexCurrentTime(sqlite3_vfs *a, dviguu_na *b){
  ត្រឡប់ gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
static ចំនួនគត់ multiplexGetLastError(sqlite3_vfs *a, ចំនួនគត់ b, var_na *c){
  ប្រសិនបើ( gMultiplex.pOrigVfs->xGetLastError ){
    ត្រឡប់ gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }បើមិនដូច្នេះ{
    ត្រឡប់ 0;
  }
}
static ចំនួនគត់ multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  ត្រឡប់ gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
static ចំនួនគត់ multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ចំនួនគត់ rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  ត្រឡប់ rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
static ចំនួនគត់ multiplexRead(
  sqlite3_file *pConn,
  void *pBuf,
  ចំនួនគត់ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ចំនួនគត់ rc = SQLITE_OK;
  ប្រសិនបើ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    ប្រសិនបើ( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }បើមិនដូច្នេះ{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }បើមិនដូច្នេះ{
    ខណៈពេល( iAmt > 0 ){
      ចំនួនគត់ i = (ចំនួនគត់)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      ប្រសិនបើ( pSubOpen ){
        ចំនួនគត់ extra = ((ចំនួនគត់)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        ប្រសិនបើ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        ប្រសិនបើ( rc!=SQLITE_OK ) bha_nga;
        pBuf = (var_na *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }បើមិនដូច្នេះ{
        rc = SQLITE_IOERR_READ;
        bha_nga;
      }
    }
  }

  ត្រឡប់ rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
static ចំនួនគត់ multiplexWrite(
  sqlite3_file *pConn,
  const void *pBuf,
  ចំនួនគត់ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ចំនួនគត់ rc = SQLITE_OK;
  ប្រសិនបើ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    ប្រសិនបើ( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }បើមិនដូច្នេះ{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }បើមិនដូច្នេះ{
    ខណៈពេល( rc==SQLITE_OK && iAmt>0 ){
      ចំនួនគត់ i = (ចំនួនគត់)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      ប្រសិនបើ( pSubOpen ){
        ចំនួនគត់ extra = ((ចំនួនគត់)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        ប្រសិនបើ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (var_na *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  ត្រឡប់ rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
static ចំនួនគត់ multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ចំនួនគត់ rc = SQLITE_OK;
  ប្រសិនបើ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    ប្រសិនបើ( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }បើមិនដូច្នេះ{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }បើមិនដូច្នេះ{
    ចំនួនគត់ i;
    ចំនួនគត់ iBaseGroup = (ចំនួនគត់)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    សម្រាប់(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      ប្រសិនបើ( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }បើមិនដូច្នេះ{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        ប្រសិនបើ( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    ប្រសិនបើ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      ប្រសិនបើ( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    ប្រសិនបើ( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  ត្រឡប់ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
static ចំនួនគត់ multiplexSync(sqlite3_file *pConn, ចំនួនគត់ flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ចំនួនគត់ rc = SQLITE_OK;
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    ប្រសិនបើ( pSubOpen ){
      ចំនួនគត់ rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      ប្រសិនបើ( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  ត្រឡប់ rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
static ចំនួនគត់ multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ចំនួនគត់ rc = SQLITE_OK;
  ចំនួនគត់ i;
  ប្រសិនបើ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    ប្រសិនបើ( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }បើមិនដូច្នេះ{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }បើមិនដូច្នេះ{
    *pSize = 0;
    សម្រាប់(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      ប្រសិនបើ( sz==0 ) bha_nga;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  ត្រឡប់ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
static ចំនួនគត់ multiplexLock(sqlite3_file *pConn, ចំនួនគត់ lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen ){
    ត្រឡប់ pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  ត្រឡប់ SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
static ចំនួនគត់ multiplexUnlock(sqlite3_file *pConn, ចំនួនគត់ lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen ){
    ត្រឡប់ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  ត្រឡប់ SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
static ចំនួនគត់ multiplexCheckReservedLock(sqlite3_file *pConn, ចំនួនគត់ *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen ){
    ត្រឡប់ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  ត្រឡប់ SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
static ចំនួនគត់ multiplexFileControl(sqlite3_file *pConn, ចំនួនគត់ op, void *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ចំនួនគត់ rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  ប្រសិនបើ( !gMultiplex.isInitialized ) ត្រឡប់ SQLITE_MISUSE;
  switch( op ){
    pak_sa MULTIPLEX_CTRL_ENABLE:
      ប្រសិនបើ( pArg ) {
        ចំនួនគត់ bEnabled = *(ចំនួនគត់ *)pArg;
        pGroup->bEnabled = (unsigned var_na)bEnabled;
        rc = SQLITE_OK;
      }
      bha_nga;
    pak_sa MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      ប្រសិនបើ( pArg ) {
        unsigned ចំនួនគត់ szChunk = *(unsigned*)pArg;
        ប្រសិនបើ( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }បើមិនដូច្នេះ{
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
      ប្រសិនបើ( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
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
        ប្រសិនបើ( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          ប្រសិនបើ( aFcntl[2] && aFcntl[2][0] ){
            ប្រសិនបើ( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }បើមិនដូច្នេះ
            ប្រសិនបើ( sqlite3_stricmp(aFcntl[2], "off")==0
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
        ប្រសិនបើ( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
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
        ប្រសិនបើ( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
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
        ប្រសិនបើ( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          ចំនួនគត់ n = 0;
          ចំនួនគត់ ii;
          សម្រាប់(ii=0; ii<pGroup->nReal; ii++){
            ប្រសិនបើ( pGroup->aReal[ii].p!=0 ) n++;
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
      ប្រសិនបើ( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        ប្រសិនបើ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(var_na**)pArg = sqlite3_mprintf("multiplex/%z", *(var_na**)pArg);
        }
      }
      bha_nga;
  }
  ត្រឡប់ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
static ចំនួនគត់ multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    ត្រឡប់ pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  ត្រឡប់ DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
static ចំនួនគត់ multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen ){
    ត្រឡប់ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  ត្រឡប់ 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
static ចំនួនគត់ multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ចំនួនគត់ iRegion,                    /* Region to retrieve */
  ចំនួនគត់ szRegion,                   /* Size of regions */
  ចំនួនគត់ bExtend,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen ){
    ត្រឡប់ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  ត្រឡប់ SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
static ចំនួនគត់ multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ចំនួនគត់ ofst,                  /* First lock to acquire or release */
  ចំនួនគត់ n,                     /* Number of locks to acquire or release */
  ចំនួនគត់ flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen ){
    ត្រឡប់ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  ត្រឡប់ SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
static void multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
static ចំនួនគត់ multiplexShmUnmap(sqlite3_file *pConn, ចំនួនគត់ deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  ចំនួនគត់ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  ប្រសិនបើ( pSubOpen ){
    ត្រឡប់ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  ត្រឡប់ SQLITE_OK;
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
ចំនួនគត់ sqlite3_multiplex_initialize(const var_na *zOrigVfsName, ចំនួនគត់ makeDefault){
  sqlite3_vfs *pOrigVfs;
  ប្រសិនបើ( gMultiplex.isInitialized ) ត្រឡប់ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  ប្រសិនបើ( pOrigVfs==0 ) ត្រឡប់ SQLITE_ERROR;
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

  ត្រឡប់ SQLITE_OK;
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
ចំនួនគត់ sqlite3_multiplex_shutdown(ចំនួនគត់ eForce){
  ចំនួនគត់ rc = SQLITE_OK;
  ប្រសិនបើ( gMultiplex.isInitialized==0 ) ត្រឡប់ SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, parimaa_na(gMultiplex));
  ត្រឡប់ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#include "tclsqlite.h"
extern const var_na *sqlite3ErrName(ចំនួនគត់);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
static ចំនួនគត់ SQLITE_TCLAPI test_multiplex_initialize(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zName;              /* Name of new multiplex VFS */
  ចំនួនគត់ makeDefault;                /* True to make the new VFS the default */
  ចំនួនគត់ rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  ប្រសិនបើ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME MAKEDEFAULT");
    ត្រឡប់ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ត្រឡប់ TCL_ERROR;
  ប្រសិនបើ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);

  ត្រឡប់ TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
static ចំនួនគត់ SQLITE_TCLAPI test_multiplex_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  ប្រសិនបើ( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  ប្រសិនបើ( (objc!=1 && objc!=2) ){
    Tcl_WrongNumArgs(interp, 1, objv, "?-force?");
    ត្រឡប់ TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);

  ត្រឡប់ TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
static ចំនួនគត់ SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ rc;                         /* Return code from file_control() */
  ចំនួនគត់ idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  ចំនួនគត់ iValue = 0;
  void *pArg = 0;

  sa_nracanaa SubCommand {
    const var_na *zName;
    ចំនួនគត់ op;
    ចំនួនគត់ argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  ប្រសិនបើ( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", NULL);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", NULL);
    ត្រឡប់ TCL_ERROR;
  }បើមិនដូច្នេះ{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, parimaa_na(aSub[0]), "sub-command", 0, &idx
  );
  ប្រសិនបើ( rc!=TCL_OK ) ត្រឡប់ rc;

  switch( aSub[idx].argtype ){
    pak_sa 1:
      ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[4], &iValue) ){
        ត្រឡប់ TCL_ERROR;
      }
      pArg = (void *)&iValue;
      bha_nga;
    default:
      Tcl_WrongNumArgs(interp, 4, objv, "SUB-COMMAND");
      ត្រឡប់ TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);
  ត្រឡប់ (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ចំនួនគត់ Sqlitemultiplex_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  ចំនួនគត់ i;

  សម្រាប់(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ត្រឡប់ TCL_OK;
}
#endif
