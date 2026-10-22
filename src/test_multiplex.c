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
  ကိန်းပြည့် nReal;                       /* Number of chunks */
  var_na *zName;                     /* Base filename of this group */
  ကိန်းပြည့် nName;                       /* Length of base filename */
  ကိန်းပြည့် flags;                       /* Flags used for original opening */
  unsigned ကိန်းပြည့် szChunk;            /* Chunk size used for this group */
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
  ကိန်းပြည့် isInitialized;
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
static ကိန်းပြည့် multiplexStrlen30(const var_na *z){
  const var_na *z2 = z;
  အကယ်၍( z==0 ) ပြန်ပေး 0;
  နေစဉ်( *z2 ){ z2++; }
  ပြန်ပေး 0x3fffffff & (ကိန်းပြည့်)(z2 - z);
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
  ကိန်းပြည့် nBase,                      /* Size of zBase in bytes (without \0) */
  ကိန်းပြည့် flags,                      /* Flags used to open file */
  ကိန်းပြည့် iChunk,                     /* Chunk to generate filename for */
  var_na *zOut                      /* Buffer to write generated name to */
){
  ကိန်းပြည့် n = nBase;
  memcpy(zOut, zBase, n+1);
  အကယ်၍( iChunk!=0 && iChunk<=MX_CHUNK_NUMBER ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    ကိန်းပြည့် i;
    အတွက်(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    အကယ်၍( i>=n-4 ) n = i+1;
    အကယ်၍( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }မဟုတ်ပါက အကယ်၍( flags & SQLITE_OPEN_WAL ){
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
static ကိန်းပြည့် multiplexSubFilename(multiplexGroup *pGroup, ကိန်းပြည့် iChunk){
  အကယ်၍( iChunk>=pGroup->nReal ){
    sa_nracanaa multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*parimaa_na(*p));
    အကယ်၍( p==0 ){
      ပြန်ပေး SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, parimaa_na(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  အကယ်၍( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    var_na *z;
    ကိန်းပြည့် n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    အကယ်၍( z==0 ){
      ပြန်ပေး SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (var_na*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    အကယ်၍( pGroup->aReal[iChunk].z==0 ) ပြန်ပေး SQLITE_NOMEM;
  }
  ပြန်ပေး SQLITE_OK;
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
  ကိန်းပြည့် iChunk,                /* Which chunk to open.  0==original file */
  ကိန်းပြည့် *rc,                   /* Result code in and out */
  ကိန်းပြည့် *pOutFlags,            /* Output flags */
  ကိန်းပြည့် createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  အကယ်၍( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    ပြန်ပေး 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  အကယ်၍( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    ကိန်းပြည့် flags, bExists;
    flags = pGroup->flags;
    အကယ်၍( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }မဟုတ်ပါက အကယ်၍( iChunk==0 ){
      /* Fall through */
    }မဟုတ်ပါက အကယ်၍( pGroup->aReal[iChunk].z==0 ){
      ပြန်ပေး 0;
    }မဟုတ်ပါက{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     အကယ်၍( *rc || !bExists ){
        အကယ်၍( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        ပြန်ပေး 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    အကယ်၍( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      ပြန်ပေး 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    အကယ်၍( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      ပြန်ပေး 0;
    }
  }
  ပြန်ပေး pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
static sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ကိန်းပြည့် iChunk,                /* Which chunk to open.  0==original file */
  ကိန်းပြည့် *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  အကယ်၍( *rc ) ပြန်ပေး 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, NULL, 0);
  အကယ်၍( pSub==0 ) ပြန်ပေး 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  ပြန်ပေး sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
static void multiplexControlFunc(
  sqlite3_context *context,
  ကိန်းပြည့် argc,
  sqlite3_value **argv
){
  ကိန်းပြည့် rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ကိန်းပြည့် op = 0;
  ကိန်းပြည့် iVal;

  အကယ်၍( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }မဟုတ်ပါက{
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
  အကယ်၍( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
static ကိန်းပြည့် multiplexFuncInit(
  sqlite3 *db,
  var_na **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  ကိန်းပြည့် rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  ပြန်ပေး rc;
}

/*
** Close a single sub-file in the connection group.
*/
static void multiplexSubClose(
  multiplexGroup *pGroup,
  ကိန်းပြည့် iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  အကယ်၍( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    အကယ်၍( pOrigVfs && pGroup->aReal[iChunk].z ){
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
  ကိန်းပြည့် i;
  အတွက်(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
static ကိန်းပြည့် multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const var_na *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  ကိန်းပြည့် flags,                 /* Flags to control the opening */
  ကိန်းပြည့် *pOutFlags             /* Flags showing results of opening */
){
  ကိန်းပြည့် rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  ကိန်းပြည့် nName = 0;
  ကိန်းပြည့် sz = 0;
  var_na *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  assert( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  အကယ်၍( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = parimaa_na(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    အကယ်၍( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  အကယ်၍( rc==SQLITE_OK ){
    const var_na *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned var_na)-1;
    pGroup->bTruncate = (unsigned var_na)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (ကိန်းပြည့်)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    အကယ်၍( zName ){
      var_na *p = (var_na *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    အကယ်၍( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      extern ကိန်းပြည့် sqlite3PendingByte;
#မဟုတ်ပါက
      ကိန်းပြည့် sqlite3PendingByte = 0x40000000;
#endif
      နေစဉ်( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    အကယ်၍( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      အကယ်၍( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    အကယ်၍( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      အကယ်၍( rc==SQLITE_OK && zName ){
        ကိန်းပြည့် bExists;
        အကယ်၍( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }မဟုတ်ပါက
        အကယ်၍( sz64==0 ){
          အကယ်၍( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            ကိန်းပြည့် iChunk = 1;
            do {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              အကယ်၍( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                အကယ်၍( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }နေစဉ်( rc==SQLITE_OK && bExists );
          }
        }မဟုတ်ပါက{
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
          အကယ်၍( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (ကိန်းပြည့်)sz64;
          }မဟုတ်ပါက အကယ်၍( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    အကယ်၍( rc==SQLITE_OK ){
      အကယ်၍( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }မဟုတ်ပါက{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }မဟုတ်ပါက{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  ပြန်ပေး rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
static ကိန်းပြည့် multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  const var_na *zName,         /* Name of file to delete */
  ကိန်းပြည့် syncDir
){
  ကိန်းပြည့် rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  အကယ်၍( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    ကိန်းပြည့် nName = (ကိန်းပြည့်)strlen(zName);
    var_na *z;
    z = sqlite3_malloc64(nName + 5);
    အကယ်၍( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }မဟုတ်ပါက{
      ကိန်းပြည့် iChunk = 0;
      ကိန်းပြည့် bExists;
      do{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }နေစဉ်( rc==SQLITE_OK && bExists );
      နေစဉ်( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      အကယ်၍( rc==SQLITE_OK ){
        iChunk = 0;
        do{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }နေစဉ်( rc==SQLITE_OK && bExists );
        နေစဉ်( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  ပြန်ပေး rc;
}

static ကိန်းပြည့် multiplexAccess(sqlite3_vfs *a, const var_na *b, ကိန်းပြည့် c, ကိန်းပြည့် *d){
  ပြန်ပေး gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
static ကိန်းပြည့် multiplexFullPathname(sqlite3_vfs *a, const var_na *b, ကိန်းပြည့် c, var_na *d){
  ပြန်ပေး gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
static void *multiplexDlOpen(sqlite3_vfs *a, const var_na *b){
  ပြန်ပေး gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
static void multiplexDlError(sqlite3_vfs *a, ကိန်းပြည့် b, var_na *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
static void (*multiplexDlSym(sqlite3_vfs *a, void *b, const var_na *c))(void){
  ပြန်ပေး gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
static void multiplexDlClose(sqlite3_vfs *a, void *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
static ကိန်းပြည့် multiplexRandomness(sqlite3_vfs *a, ကိန်းပြည့် b, var_na *c){
  ပြန်ပေး gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
static ကိန်းပြည့် multiplexSleep(sqlite3_vfs *a, ကိန်းပြည့် b){
  ပြန်ပေး gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
static ကိန်းပြည့် multiplexCurrentTime(sqlite3_vfs *a, dviguu_na *b){
  ပြန်ပေး gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
static ကိန်းပြည့် multiplexGetLastError(sqlite3_vfs *a, ကိန်းပြည့် b, var_na *c){
  အကယ်၍( gMultiplex.pOrigVfs->xGetLastError ){
    ပြန်ပေး gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }မဟုတ်ပါက{
    ပြန်ပေး 0;
  }
}
static ကိန်းပြည့် multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  ပြန်ပေး gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
static ကိန်းပြည့် multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ကိန်းပြည့် rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  ပြန်ပေး rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
static ကိန်းပြည့် multiplexRead(
  sqlite3_file *pConn,
  void *pBuf,
  ကိန်းပြည့် iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ကိန်းပြည့် rc = SQLITE_OK;
  အကယ်၍( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    အကယ်၍( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }မဟုတ်ပါက{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }မဟုတ်ပါက{
    နေစဉ်( iAmt > 0 ){
      ကိန်းပြည့် i = (ကိန်းပြည့်)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      အကယ်၍( pSubOpen ){
        ကိန်းပြည့် extra = ((ကိန်းပြည့်)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        အကယ်၍( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        အကယ်၍( rc!=SQLITE_OK ) bha_nga;
        pBuf = (var_na *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }မဟုတ်ပါက{
        rc = SQLITE_IOERR_READ;
        bha_nga;
      }
    }
  }

  ပြန်ပေး rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
static ကိန်းပြည့် multiplexWrite(
  sqlite3_file *pConn,
  const void *pBuf,
  ကိန်းပြည့် iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ကိန်းပြည့် rc = SQLITE_OK;
  အကယ်၍( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    အကယ်၍( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }မဟုတ်ပါက{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }မဟုတ်ပါက{
    နေစဉ်( rc==SQLITE_OK && iAmt>0 ){
      ကိန်းပြည့် i = (ကိန်းပြည့်)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, NULL, 1);
      အကယ်၍( pSubOpen ){
        ကိန်းပြည့် extra = ((ကိန်းပြည့်)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        အကယ်၍( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (var_na *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  ပြန်ပေး rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
static ကိန်းပြည့် multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ကိန်းပြည့် rc = SQLITE_OK;
  အကယ်၍( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    အကယ်၍( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }မဟုတ်ပါက{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }မဟုတ်ပါက{
    ကိန်းပြည့် i;
    ကိန်းပြည့် iBaseGroup = (ကိန်းပြည့်)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    အတွက်(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      အကယ်၍( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }မဟုတ်ပါက{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        အကယ်၍( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    အကယ်၍( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      အကယ်၍( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    အကယ်၍( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  ပြန်ပေး rc;
}

/* Pass xSync requests through to the original VFS without change
*/
static ကိန်းပြည့် multiplexSync(sqlite3_file *pConn, ကိန်းပြည့် flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ကိန်းပြည့် rc = SQLITE_OK;
  ကိန်းပြည့် i;
  အတွက်(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    အကယ်၍( pSubOpen ){
      ကိန်းပြည့် rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      အကယ်၍( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  ပြန်ပေး rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
static ကိန်းပြည့် multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ကိန်းပြည့် rc = SQLITE_OK;
  ကိန်းပြည့် i;
  အကယ်၍( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, NULL, 0);
    အကယ်၍( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }မဟုတ်ပါက{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }မဟုတ်ပါက{
    *pSize = 0;
    အတွက်(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      အကယ်၍( sz==0 ) bha_nga;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  ပြန်ပေး rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် multiplexLock(sqlite3_file *pConn, ကိန်းပြည့် lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen ){
    ပြန်ပေး pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  ပြန်ပေး SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် multiplexUnlock(sqlite3_file *pConn, ကိန်းပြည့် lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen ){
    ပြန်ပေး pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  ပြန်ပေး SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် multiplexCheckReservedLock(sqlite3_file *pConn, ကိန်းပြည့် *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen ){
    ပြန်ပေး pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  ပြန်ပေး SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
static ကိန်းပြည့် multiplexFileControl(sqlite3_file *pConn, ကိန်းပြည့် op, void *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ကိန်းပြည့် rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  အကယ်၍( !gMultiplex.isInitialized ) ပြန်ပေး SQLITE_MISUSE;
  switch( op ){
    pak_sa MULTIPLEX_CTRL_ENABLE:
      အကယ်၍( pArg ) {
        ကိန်းပြည့် bEnabled = *(ကိန်းပြည့် *)pArg;
        pGroup->bEnabled = (unsigned var_na)bEnabled;
        rc = SQLITE_OK;
      }
      bha_nga;
    pak_sa MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      အကယ်၍( pArg ) {
        unsigned ကိန်းပြည့် szChunk = *(unsigned*)pArg;
        အကယ်၍( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }မဟုတ်ပါက{
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
      အကယ်၍( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
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
        အကယ်၍( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          အကယ်၍( aFcntl[2] && aFcntl[2][0] ){
            အကယ်၍( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }မဟုတ်ပါက
            အကယ်၍( sqlite3_stricmp(aFcntl[2], "off")==0
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
        အကယ်၍( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
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
        အကယ်၍( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
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
        အကယ်၍( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          ကိန်းပြည့် n = 0;
          ကိန်းပြည့် ii;
          အတွက်(ii=0; ii<pGroup->nReal; ii++){
            အကယ်၍( pGroup->aReal[ii].p!=0 ) n++;
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
      အကယ်၍( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        အကယ်၍( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(var_na**)pArg = sqlite3_mprintf("multiplex/%z", *(var_na**)pArg);
        }
      }
      bha_nga;
  }
  ပြန်ပေး rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    ပြန်ပေး pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  ပြန်ပေး DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen ){
    ပြန်ပေး pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  ပြန်ပေး 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ကိန်းပြည့် iRegion,                    /* Region to retrieve */
  ကိန်းပြည့် szRegion,                   /* Size of regions */
  ကိန်းပြည့် bExtend,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen ){
    ပြန်ပေး pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  ပြန်ပေး SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ကိန်းပြည့် ofst,                  /* First lock to acquire or release */
  ကိန်းပြည့် n,                     /* Number of locks to acquire or release */
  ကိန်းပြည့် flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen ){
    ပြန်ပေး pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  ပြန်ပေး SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
static void multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
static ကိန်းပြည့် multiplexShmUnmap(sqlite3_file *pConn, ကိန်းပြည့် deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  ကိန်းပြည့် rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, NULL, 0);
  အကယ်၍( pSubOpen ){
    ပြန်ပေး pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  ပြန်ပေး SQLITE_OK;
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
ကိန်းပြည့် sqlite3_multiplex_initialize(const var_na *zOrigVfsName, ကိန်းပြည့် makeDefault){
  sqlite3_vfs *pOrigVfs;
  အကယ်၍( gMultiplex.isInitialized ) ပြန်ပေး SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  အကယ်၍( pOrigVfs==0 ) ပြန်ပေး SQLITE_ERROR;
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

  ပြန်ပေး SQLITE_OK;
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
ကိန်းပြည့် sqlite3_multiplex_shutdown(ကိန်းပြည့် eForce){
  ကိန်းပြည့် rc = SQLITE_OK;
  အကယ်၍( gMultiplex.isInitialized==0 ) ပြန်ပေး SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, parimaa_na(gMultiplex));
  ပြန်ပေး rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#include "tclsqlite.h"
extern const var_na *sqlite3ErrName(ကိန်းပြည့်);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_multiplex_initialize(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zName;              /* Name of new multiplex VFS */
  ကိန်းပြည့် makeDefault;                /* True to make the new VFS the default */
  ကိန်းပြည့် rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  အကယ်၍( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NAME MAKEDEFAULT");
    ပြန်ပေး TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  အကယ်၍( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ပြန်ပေး TCL_ERROR;
  အကယ်၍( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);

  ပြန်ပေး TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_multiplex_shutdown(
  void * clientData,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  ကိန်းပြည့် rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  အကယ်၍( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  အကယ်၍( (objc!=1 && objc!=2) ){
    Tcl_WrongNumArgs(interp, 1, objv, "?-force?");
    ပြန်ပေး TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);

  ပြန်ပေး TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
static ကိန်းပြည့် SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  ကိန်းပြည့် objc,
  Tcl_Obj *CONST objv[]
){
  ကိန်းပြည့် rc;                         /* Return code from file_control() */
  ကိန်းပြည့် idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  ကိန်းပြည့် iValue = 0;
  void *pArg = 0;

  sa_nracanaa SubCommand {
    const var_na *zName;
    ကိန်းပြည့် op;
    ကိန်းပြည့် argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  အကယ်၍( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    ပြန်ပေး TCL_ERROR;
  }

  အကယ်၍( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", NULL);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", NULL);
    ပြန်ပေး TCL_ERROR;
  }မဟုတ်ပါက{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, parimaa_na(aSub[0]), "sub-command", 0, &idx
  );
  အကယ်၍( rc!=TCL_OK ) ပြန်ပေး rc;

  switch( aSub[idx].argtype ){
    pak_sa 1:
      အကယ်၍( Tcl_GetIntFromObj(interp, objv[4], &iValue) ){
        ပြန်ပေး TCL_ERROR;
      }
      pArg = (void *)&iValue;
      bha_nga;
    default:
      Tcl_WrongNumArgs(interp, 4, objv, "SUB-COMMAND");
      ပြန်ပေး TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);
  ပြန်ပေး (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ကိန်းပြည့် Sqlitemultiplex_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  ကိန်းပြည့် i;

  အတွက်(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ပြန်ပေး TCL_OK;
}
#endif
