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
typedef struct multiplexGroup multiplexGroup;
typedef struct multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
struct multiplexGroup {
  struct multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    char *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  ціле nReal;                       /* Number of chunks */
  char *zName;                     /* Base filename of this group */
  ціле nName;                       /* Length of base filename */
  ціле flags;                       /* Flags used for original opening */
  unsigned ціле szChunk;            /* Chunk size used for this group */
  unsigned char bEnabled;          /* TRUE to use Multiplex VFS for this file */
  unsigned char bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
struct multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
static struct {
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
  ціле isInitialized;
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
static ціле multiplexStrlen30(const char *z){
  const char *z2 = z;
  якщо( z==0 ) повернути 0;
  поки( *z2 ){ z2++; }
  повернути 0x3fffffff & (ціле)(z2 - z);
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
  const char *zBase,              /* Filename for chunk 0 */
  ціле nBase,                      /* Size of zBase in bytes (without \0) */
  ціле flags,                      /* Flags used to open file */
  ціле iChunk,                     /* Chunk to generate filename for */
  char *zOut                      /* Buffer to write generated name to */
){
  ціле n = nBase;
  memcpy(zOut, zBase, n+1);
  якщо( iChunk!=0 && iChunk<=MX_CHUNK_NUMBER ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    ціле i;
    для(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    якщо( i>=n-4 ) n = i+1;
    якщо( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }інакше якщо( flags & SQLITE_OPEN_WAL ){
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
static ціле multiplexSubFilename(multiplexGroup *pGroup, ціле iChunk){
  якщо( iChunk>=pGroup->nReal ){
    struct multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*sizeof(*p));
    якщо( p==0 ){
      повернути SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, sizeof(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  якщо( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    char *z;
    ціле n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    якщо( z==0 ){
      повернути SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (char*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    якщо( pGroup->aReal[iChunk].z==0 ) повернути SQLITE_NOMEM;
  }
  повернути SQLITE_OK;
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
  ціле iChunk,                /* Which chunk to open.  0==original file */
  ціле *rc,                   /* Result code in and out */
  ціле *pOutFlags,            /* Output flags */
  ціле createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  якщо( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    повернути 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  якщо( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    ціле flags, bExists;
    flags = pGroup->flags;
    якщо( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }інакше якщо( iChunk==0 ){
      /* Fall through */
    }інакше якщо( pGroup->aReal[iChunk].z==0 ){
      повернути 0;
    }інакше{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     якщо( *rc || !bExists ){
        якщо( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        повернути 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    якщо( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      повернути 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    якщо( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      повернути 0;
    }
  }
  повернути pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
static sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ціле iChunk,                /* Which chunk to open.  0==original file */
  ціле *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  якщо( *rc ) повернути 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, NULL, 0);
  якщо( pSub==0 ) повернути 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  повернути sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
static void multiplexControlFunc(
  sqlite3_context *context,
  ціле argc,
  sqlite3_value **argv
){
  ціле rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ціле op = 0;
  ціле iVal;

  якщо( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }інакше{
    /* extract params */
    op = sqlite3_value_int(argv[0]);
    iVal = sqlite3_value_int(argv[1]);
    /* map function op to file_control op */
    switch( op ){
      case 1:
        op = MULTIPLEX_CTRL_ENABLE;
        break;
      case 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_SIZE;
        break;
      case 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        break;
      default:
        rc = SQLITE_NOTFOUND;
        break;
    }
  }
  якщо( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
static ціле multiplexFuncInit(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  ціле rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  повернути rc;
}

/*
** Close a single sub-file in the connection group.
*/
static void multiplexSubClose(
  multiplexGroup *pGroup,
  ціле iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  якщо( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    якщо( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  memset(&pGroup->aReal[iChunk], 0, sizeof(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
static void multiplexFreeComponents(multiplexGroup *pGroup){
  ціле i;
  для(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
static ціле multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const char *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  ціле flags,                 /* Flags to control the opening */
  ціле *pOutFlags             /* Flags showing results of opening */
){
  ціле rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  ціле nName = 0;
  ціле sz = 0;
  char *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  assert( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  якщо( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = sizeof(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    якщо( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  якщо( rc==SQLITE_OK ){
    const char *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned char)-1;
    pGroup->bTruncate = (unsigned char)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (ціле)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    якщо( zName ){
      char *p = (char *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    якщо( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      extern ціле sqlite3PendingByte;
#інакше
      ціле sqlite3PendingByte = 0x40000000;
#endif
      поки( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    якщо( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      якщо( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    якщо( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      якщо( rc==SQLITE_OK && zName ){
        ціле bExists;
        якщо( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }інакше
        якщо( sz64==0 ){
          якщо( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            ціле iChunk = 1;
            do {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              якщо( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                якщо( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }поки( rc==SQLITE_OK && bExists );
          }
        }інакше{
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
          якщо( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (ціле)sz64;
          }інакше якщо( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    якщо( rc==SQLITE_OK ){
      якщо( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }інакше{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }інакше{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  повернути rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
static ціле multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const char *zName,         /* Name of file to delete */
  ціле syncDir
){
  ціле rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  якщо( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    ціле nName = (ціле)strlen(zName);
    char *z;
    z = sqlite3_malloc64(nName + 5);
    якщо( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }інакше{
      ціле iChunk = 0;
      ціле bExists;
      do{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }поки( rc==SQLITE_OK && bExists );
      поки( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      якщо( rc==SQLITE_OK ){
        iChunk = 0;
        do{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }поки( rc==SQLITE_OK && bExists );
        поки( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  повернути rc;
}

static ціле multiplexAccess(sqlite3_vfs *a, const char *b, ціле c, ціле *d){
  повернути gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
static ціле multiplexFullPathname(sqlite3_vfs *a, const char *b, ціле c, char *d){
  повернути gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
static void *multiplexDlOpen(sqlite3_vfs *a, const char *b){
  повернути gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
static void multiplexDlError(sqlite3_vfs *a, ціле b, char *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
static void (*multiplexDlSym(sqlite3_vfs *a, void *b, const char *c))(void){
  повернути gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
static void multiplexDlClose(sqlite3_vfs *a, void *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
static ціле multiplexRandomness(sqlite3_vfs *a, ціле b, char *c){
  повернути gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
static ціле multiplexSleep(sqlite3_vfs *a, ціле b){
  повернути gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
static ціле multiplexCurrentTime(sqlite3_vfs *a, double *b){
  повернути gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
static ціле multiplexGetLastError(sqlite3_vfs *a, ціле b, char *c){
  якщо( gMultiplex.pOrigVfs->xGetLastError ){
    повернути gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }інакше{
    повернути 0;
  }
}
static ціле multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  повернути gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
static ціле multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ціле rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  повернути rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
static ціле multiplexRead(
  sqlite3_file *pConn,
  void *pBuf,
  ціле iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ціле rc = SQLITE_OK;
  якщо( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    якщо( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }інакше{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }інакше{
    поки( iAmt > 0 ){
      ціле i = (ціле)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      якщо( pSubOpen ){
        ціле extra = ((ціле)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        якщо( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        якщо( rc!=SQLITE_OK ) break;
        pBuf = (char *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }інакше{
        rc = SQLITE_IOERR_READ;
        break;
      }
    }
  }

  повернути rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
static ціле multiplexWrite(
  sqlite3_file *pConn,
  const void *pBuf,
  ціле iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ціле rc = SQLITE_OK;
  якщо( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    якщо( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }інакше{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }інакше{
    поки( rc==SQLITE_OK && iAmt>0 ){
      ціле i = (ціле)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      якщо( pSubOpen ){
        ціле extra = ((ціле)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        якщо( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (char *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  повернути rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
static ціле multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ціле rc = SQLITE_OK;
  якщо( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    якщо( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }інакше{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }інакше{
    ціле i;
    ціле iBaseGroup = (ціле)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    для(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      якщо( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }інакше{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        якщо( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    якщо( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      якщо( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    якщо( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  повернути rc;
}

/* Pass xSync requests through to the original VFS without change
*/
static ціле multiplexSync(sqlite3_file *pConn, ціле flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ціле rc = SQLITE_OK;
  ціле i;
  для(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    якщо( pSubOpen ){
      ціле rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      якщо( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  повернути rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
static ціле multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ціле rc = SQLITE_OK;
  ціле i;
  якщо( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    якщо( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }інакше{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }інакше{
    *pSize = 0;
    для(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      якщо( sz==0 ) break;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  повернути rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
static ціле multiplexLock(sqlite3_file *pConn, ціле lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen ){
    повернути pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  повернути SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
static ціле multiplexUnlock(sqlite3_file *pConn, ціле lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen ){
    повернути pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  повернути SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
static ціле multiplexCheckReservedLock(sqlite3_file *pConn, ціле *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen ){
    повернути pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  повернути SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
static ціле multiplexFileControl(sqlite3_file *pConn, ціле op, void *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ціле rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  якщо( !gMultiplex.isInitialized ) повернути SQLITE_MISUSE;
  switch( op ){
    case MULTIPLEX_CTRL_ENABLE:
      якщо( pArg ) {
        ціле bEnabled = *(ціле *)pArg;
        pGroup->bEnabled = (unsigned char)bEnabled;
        rc = SQLITE_OK;
      }
      break;
    case MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      якщо( pArg ) {
        unsigned ціле szChunk = *(unsigned*)pArg;
        якщо( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }інакше{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_SIZE-1));
          szChunk &= ~(MAX_PAGE_SIZE-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      break;
    case MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      break;
    case SQLITE_FCNTL_SIZE_HINT:
    case SQLITE_FCNTL_CHUNK_SIZE:
      /* no-op these */
      rc = SQLITE_OK;
      break;
    case SQLITE_FCNTL_PRAGMA: {
      char **aFcntl = (char**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      якщо( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
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
        якщо( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          якщо( aFcntl[2] && aFcntl[2][0] ){
            якщо( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }інакше
            якщо( sqlite3_stricmp(aFcntl[2], "off")==0
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
          break;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        якщо( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          break;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        якщо( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          break;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        якщо( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          ціле n = 0;
          ціле ii;
          для(ii=0; ii<pGroup->nReal; ii++){
            якщо( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          break;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    default:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
      якщо( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        якщо( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(char**)pArg = sqlite3_mprintf("multiplex/%z", *(char**)pArg);
        }
      }
      break;
  }
  повернути rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
static ціле multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    повернути pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  повернути DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
static ціле multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen ){
    повернути pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  повернути 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
static ціле multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ціле iRegion,                    /* Region to retrieve */
  ціле szRegion,                   /* Size of regions */
  ціле bExtend,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen ){
    повернути pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  повернути SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
static ціле multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ціле ofst,                  /* First lock to acquire or release */
  ціле n,                     /* Number of locks to acquire or release */
  ціле flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen ){
    повернути pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  повернути SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
static void multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
static ціле multiplexShmUnmap(sqlite3_file *pConn, ціле deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  ціле rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  якщо( pSubOpen ){
    повернути pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  повернути SQLITE_OK;
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
ціле sqlite3_multiplex_initialize(const char *zOrigVfsName, ціле makeDefault){
  sqlite3_vfs *pOrigVfs;
  якщо( gMultiplex.isInitialized ) повернути SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  якщо( pOrigVfs==0 ) повернути SQLITE_ERROR;
  assert( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += sizeof(multiplexConn);
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

  повернути SQLITE_OK;
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
ціле sqlite3_multiplex_shutdown(ціле eForce){
  ціле rc = SQLITE_OK;
  якщо( gMultiplex.isInitialized==0 ) повернути SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, sizeof(gMultiplex));
  повернути rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#include "tclsqlite.h"
extern const char *sqlite3ErrName(ціле);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
static ціле SQLITE_TCLAPI test_multiplex_initialize(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  const char *zName;              /* Name of new multiplex VFS */
  ціле makeDefault;                /* True to make the new VFS the default */
  ціле rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME MAKEDEFAULT");
    повернути TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  якщо( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) повернути TCL_ERROR;
  якщо( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);

  повернути TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
static ціле SQLITE_TCLAPI test_multiplex_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  якщо( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  якщо( (objc!=1 && objc!=2) ){
    Tcl_WrongNumArgs(interp, 1, objv, "?-force?");
    повернути TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);

  повернути TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
static ціле SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc;                         /* Return code from file_control() */
  ціле idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  ціле iValue = 0;
  void *pArg = 0;

  struct SubCommand {
    const char *zName;
    ціле op;
    ціле argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  якщо( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    повернути TCL_ERROR;
  }

  якщо( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", NULL);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", NULL);
    повернути TCL_ERROR;
  }інакше{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, sizeof(aSub[0]), "sub-command", 0, &idx
  );
  якщо( rc!=TCL_OK ) повернути rc;

  switch( aSub[idx].argtype ){
    case 1:
      якщо( Tcl_GetIntFromObj(interp, objv[4], &iValue) ){
        повернути TCL_ERROR;
      }
      pArg = (void *)&iValue;
      break;
    default:
      Tcl_WrongNumArgs(interp, 4, objv, "SUB-COMMAND");
      повернути TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);
  повернути (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ціле Sqlitemultiplex_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  ціле i;

  для(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  повернути TCL_OK;
}
#endif
