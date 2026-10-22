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
  整数 nReal;                       /* Number of chunks */
  char *zName;                     /* Base filename of this group */
  整数 nName;                       /* Length of base filename */
  整数 flags;                       /* Flags used for original opening */
  unsigned 整数 szChunk;            /* Chunk size used for this group */
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
  整数 isInitialized;
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
static 整数 multiplexStrlen30(const char *z){
  const char *z2 = z;
  もし( z==0 ) 戻る 0;
  ながら( *z2 ){ z2++; }
  戻る 0x3fffffff & (整数)(z2 - z);
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
  整数 nBase,                      /* Size of zBase in bytes (without \0) */
  整数 flags,                      /* Flags used to open file */
  整数 iChunk,                     /* Chunk to generate filename for */
  char *zOut                      /* Buffer to write generated name to */
){
  整数 n = nBase;
  memcpy(zOut, zBase, n+1);
  もし( iChunk!=0 && iChunk<=MX_CHUNK_NUMBER ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    整数 i;
    ために(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    もし( i>=n-4 ) n = i+1;
    もし( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }否则 もし( flags & SQLITE_OPEN_WAL ){
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
static 整数 multiplexSubFilename(multiplexGroup *pGroup, 整数 iChunk){
  もし( iChunk>=pGroup->nReal ){
    struct multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*sizeof(*p));
    もし( p==0 ){
      戻る SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, sizeof(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  もし( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    char *z;
    整数 n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    もし( z==0 ){
      戻る SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (char*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    もし( pGroup->aReal[iChunk].z==0 ) 戻る SQLITE_NOMEM;
  }
  戻る SQLITE_OK;
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
  整数 iChunk,                /* Which chunk to open.  0==original file */
  整数 *rc,                   /* Result code in and out */
  整数 *pOutFlags,            /* Output flags */
  整数 createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  もし( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    戻る 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  もし( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    整数 flags, bExists;
    flags = pGroup->flags;
    もし( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }否则 もし( iChunk==0 ){
      /* Fall through */
    }否则 もし( pGroup->aReal[iChunk].z==0 ){
      戻る 0;
    }否则{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     もし( *rc || !bExists ){
        もし( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        戻る 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    もし( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      戻る 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    もし( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      戻る 0;
    }
  }
  戻る pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
static sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  整数 iChunk,                /* Which chunk to open.  0==original file */
  整数 *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  もし( *rc ) 戻る 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, NULL, 0);
  もし( pSub==0 ) 戻る 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  戻る sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
static void multiplexControlFunc(
  sqlite3_context *context,
  整数 argc,
  sqlite3_value **argv
){
  整数 rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  整数 op = 0;
  整数 iVal;

  もし( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }否则{
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
  もし( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
static 整数 multiplexFuncInit(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  整数 rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  戻る rc;
}

/*
** Close a single sub-file in the connection group.
*/
static void multiplexSubClose(
  multiplexGroup *pGroup,
  整数 iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  もし( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    もし( pOrigVfs && pGroup->aReal[iChunk].z ){
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
  整数 i;
  ために(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
static 整数 multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const char *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  整数 flags,                 /* Flags to control the opening */
  整数 *pOutFlags             /* Flags showing results of opening */
){
  整数 rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  整数 nName = 0;
  整数 sz = 0;
  char *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  assert( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  もし( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = sizeof(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    もし( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  もし( rc==SQLITE_OK ){
    const char *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned char)-1;
    pGroup->bTruncate = (unsigned char)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (整数)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    もし( zName ){
      char *p = (char *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    もし( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      extern 整数 sqlite3PendingByte;
#否则
      整数 sqlite3PendingByte = 0x40000000;
#endif
      ながら( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    もし( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      もし( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    もし( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      もし( rc==SQLITE_OK && zName ){
        整数 bExists;
        もし( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }否则
        もし( sz64==0 ){
          もし( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            整数 iChunk = 1;
            do {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              もし( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                もし( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }ながら( rc==SQLITE_OK && bExists );
          }
        }否则{
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
          もし( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (整数)sz64;
          }否则 もし( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    もし( rc==SQLITE_OK ){
      もし( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }否则{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }否则{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  戻る rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
static 整数 multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const char *zName,         /* Name of file to delete */
  整数 syncDir
){
  整数 rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  もし( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    整数 nName = (整数)strlen(zName);
    char *z;
    z = sqlite3_malloc64(nName + 5);
    もし( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }否则{
      整数 iChunk = 0;
      整数 bExists;
      do{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }ながら( rc==SQLITE_OK && bExists );
      ながら( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      もし( rc==SQLITE_OK ){
        iChunk = 0;
        do{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }ながら( rc==SQLITE_OK && bExists );
        ながら( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  戻る rc;
}

static 整数 multiplexAccess(sqlite3_vfs *a, const char *b, 整数 c, 整数 *d){
  戻る gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
static 整数 multiplexFullPathname(sqlite3_vfs *a, const char *b, 整数 c, char *d){
  戻る gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
static void *multiplexDlOpen(sqlite3_vfs *a, const char *b){
  戻る gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
static void multiplexDlError(sqlite3_vfs *a, 整数 b, char *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
static void (*multiplexDlSym(sqlite3_vfs *a, void *b, const char *c))(void){
  戻る gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
static void multiplexDlClose(sqlite3_vfs *a, void *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
static 整数 multiplexRandomness(sqlite3_vfs *a, 整数 b, char *c){
  戻る gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
static 整数 multiplexSleep(sqlite3_vfs *a, 整数 b){
  戻る gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
static 整数 multiplexCurrentTime(sqlite3_vfs *a, double *b){
  戻る gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
static 整数 multiplexGetLastError(sqlite3_vfs *a, 整数 b, char *c){
  もし( gMultiplex.pOrigVfs->xGetLastError ){
    戻る gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }否则{
    戻る 0;
  }
}
static 整数 multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  戻る gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
static 整数 multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  整数 rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  戻る rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
static 整数 multiplexRead(
  sqlite3_file *pConn,
  void *pBuf,
  整数 iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  整数 rc = SQLITE_OK;
  もし( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    もし( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }否则{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }否则{
    ながら( iAmt > 0 ){
      整数 i = (整数)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      もし( pSubOpen ){
        整数 extra = ((整数)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        もし( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        もし( rc!=SQLITE_OK ) break;
        pBuf = (char *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }否则{
        rc = SQLITE_IOERR_READ;
        break;
      }
    }
  }

  戻る rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
static 整数 multiplexWrite(
  sqlite3_file *pConn,
  const void *pBuf,
  整数 iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  整数 rc = SQLITE_OK;
  もし( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    もし( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }否则{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }否则{
    ながら( rc==SQLITE_OK && iAmt>0 ){
      整数 i = (整数)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      もし( pSubOpen ){
        整数 extra = ((整数)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        もし( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (char *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  戻る rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
static 整数 multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  整数 rc = SQLITE_OK;
  もし( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    もし( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }否则{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }否则{
    整数 i;
    整数 iBaseGroup = (整数)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    ために(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      もし( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }否则{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        もし( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    もし( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      もし( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    もし( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  戻る rc;
}

/* Pass xSync requests through to the original VFS without change
*/
static 整数 multiplexSync(sqlite3_file *pConn, 整数 flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  整数 rc = SQLITE_OK;
  整数 i;
  ために(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    もし( pSubOpen ){
      整数 rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      もし( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  戻る rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
static 整数 multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  整数 rc = SQLITE_OK;
  整数 i;
  もし( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    もし( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }否则{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }否则{
    *pSize = 0;
    ために(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      もし( sz==0 ) break;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  戻る rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
static 整数 multiplexLock(sqlite3_file *pConn, 整数 lock){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen ){
    戻る pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  戻る SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
static 整数 multiplexUnlock(sqlite3_file *pConn, 整数 lock){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen ){
    戻る pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  戻る SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
static 整数 multiplexCheckReservedLock(sqlite3_file *pConn, 整数 *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen ){
    戻る pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  戻る SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
static 整数 multiplexFileControl(sqlite3_file *pConn, 整数 op, void *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  整数 rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  もし( !gMultiplex.isInitialized ) 戻る SQLITE_MISUSE;
  switch( op ){
    case MULTIPLEX_CTRL_ENABLE:
      もし( pArg ) {
        整数 bEnabled = *(整数 *)pArg;
        pGroup->bEnabled = (unsigned char)bEnabled;
        rc = SQLITE_OK;
      }
      break;
    case MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      もし( pArg ) {
        unsigned 整数 szChunk = *(unsigned*)pArg;
        もし( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }否则{
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
      もし( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
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
        もし( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          もし( aFcntl[2] && aFcntl[2][0] ){
            もし( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }否则
            もし( sqlite3_stricmp(aFcntl[2], "off")==0
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
        もし( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
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
        もし( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
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
        もし( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          整数 n = 0;
          整数 ii;
          ために(ii=0; ii<pGroup->nReal; ii++){
            もし( pGroup->aReal[ii].p!=0 ) n++;
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
      もし( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        もし( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(char**)pArg = sqlite3_mprintf("multiplex/%z", *(char**)pArg);
        }
      }
      break;
  }
  戻る rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
static 整数 multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    戻る pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  戻る DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
static 整数 multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen ){
    戻る pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  戻る 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
static 整数 multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  整数 iRegion,                    /* Region to retrieve */
  整数 szRegion,                   /* Size of regions */
  整数 bExtend,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen ){
    戻る pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  戻る SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
static 整数 multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  整数 ofst,                  /* First lock to acquire or release */
  整数 n,                     /* Number of locks to acquire or release */
  整数 flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen ){
    戻る pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  戻る SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
static void multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
static 整数 multiplexShmUnmap(sqlite3_file *pConn, 整数 deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  整数 rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  もし( pSubOpen ){
    戻る pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  戻る SQLITE_OK;
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
整数 sqlite3_multiplex_initialize(const char *zOrigVfsName, 整数 makeDefault){
  sqlite3_vfs *pOrigVfs;
  もし( gMultiplex.isInitialized ) 戻る SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  もし( pOrigVfs==0 ) 戻る SQLITE_ERROR;
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

  戻る SQLITE_OK;
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
整数 sqlite3_multiplex_shutdown(整数 eForce){
  整数 rc = SQLITE_OK;
  もし( gMultiplex.isInitialized==0 ) 戻る SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, sizeof(gMultiplex));
  戻る rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#include "tclsqlite.h"
extern const char *sqlite3ErrName(整数);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
static 整数 SQLITE_TCLAPI test_multiplex_initialize(
  void * clientData,
  Tcl_Interp *interp,
  整数 objc,
  Tcl_Obj *CONST objv[]
){
  const char *zName;              /* Name of new multiplex VFS */
  整数 makeDefault;                /* True to make the new VFS the default */
  整数 rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  もし( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME MAKEDEFAULT");
    戻る TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  もし( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) 戻る TCL_ERROR;
  もし( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);

  戻る TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
static 整数 SQLITE_TCLAPI test_multiplex_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  整数 objc,
  Tcl_Obj *CONST objv[]
){
  整数 rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  もし( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  もし( (objc!=1 && objc!=2) ){
    Tcl_WrongNumArgs(interp, 1, objv, "?-force?");
    戻る TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);

  戻る TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
static 整数 SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  整数 objc,
  Tcl_Obj *CONST objv[]
){
  整数 rc;                         /* Return code from file_control() */
  整数 idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  整数 iValue = 0;
  void *pArg = 0;

  struct SubCommand {
    const char *zName;
    整数 op;
    整数 argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  もし( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    戻る TCL_ERROR;
  }

  もし( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", NULL);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", NULL);
    戻る TCL_ERROR;
  }否则{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, sizeof(aSub[0]), "sub-command", 0, &idx
  );
  もし( rc!=TCL_OK ) 戻る rc;

  switch( aSub[idx].argtype ){
    case 1:
      もし( Tcl_GetIntFromObj(interp, objv[4], &iValue) ){
        戻る TCL_ERROR;
      }
      pArg = (void *)&iValue;
      break;
    default:
      Tcl_WrongNumArgs(interp, 4, objv, "SUB-COMMAND");
      戻る TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);
  戻る (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
整数 Sqlitemultiplex_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  整数 i;

  ために(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  戻る TCL_OK;
}
#endif
