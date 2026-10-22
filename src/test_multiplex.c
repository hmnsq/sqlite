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
#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ "test_multiplex.h"

#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#அந்தர்பாவ "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#define UNUSED_PARAMETER(x) (வெறுமை)(x)
#define MAX_PAGE_SIZE       0x10000
#define DEFAULT_SECTOR_SIZE 0x1000

/* Maximum chunk number */
#define MX_CHUNK_எண் 299

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
வகைவரை கட்டமைப்பு multiplexGroup multiplexGroup;
வகைவரை கட்டமைப்பு multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
கட்டமைப்பு multiplexGroup {
  கட்டமைப்பு multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    எழுத்து *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  முழு nReal;                       /* Number of chunks */
  எழுத்து *zName;                     /* Base filename of this group */
  முழு nName;                       /* Length of base filename */
  முழு flags;                       /* Flags used for original opening */
  unsigned முழு szChunk;            /* Chunk size used for this group */
  unsigned எழுத்து bEnabled;          /* TRUE to use Multiplex VFS for this file */
  unsigned எழுத்து bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
கட்டமைப்பு multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
நிலையான கட்டமைப்பு {
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
  முழு isInitialized;
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
நிலையான முழு multiplexStrlen30(மாறிலி எழுத்து *z){
  மாறிலி எழுத்து *z2 = z;
  எனில்( z==0 ) திரும்பு 0;
  போது( *z2 ){ z2++; }
  திரும்பு 0x3fffffff & (முழு)(z2 - z);
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
நிலையான வெறுமை multiplexFilename(
  மாறிலி எழுத்து *zBase,              /* Filename for chunk 0 */
  முழு nBase,                      /* Size of zBase in bytes (without \0) */
  முழு flags,                      /* Flags used to open file */
  முழு iChunk,                     /* Chunk to generate filename for */
  எழுத்து *zOut                      /* Buffer to write generated name to */
){
  முழு n = nBase;
  memcpy(zOut, zBase, n+1);
  எனில்( iChunk!=0 && iChunk<=MX_CHUNK_எண் ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    முழு i;
    க்கு(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    எனில்( i>=n-4 ) n = i+1;
    எனில்( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }இல்லையெனில் எனில்( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_OFFSET;
    }
#endif
    sqlite3_snprintf(4,&zOut[n],"%03d",iChunk);
    n += 3;
  }

  ப்ரதிஜ்ஞா( zOut[n]=='\0' );
  zOut[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
நிலையான முழு multiplexSubFilename(multiplexGroup *pGroup, முழு iChunk){
  எனில்( iChunk>=pGroup->nReal ){
    கட்டமைப்பு multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*அளவு(*p));
    எனில்( p==0 ){
      திரும்பு SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, அளவு(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  எனில்( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    எழுத்து *z;
    முழு n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    எனில்( z==0 ){
      திரும்பு SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (எழுத்து*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    எனில்( pGroup->aReal[iChunk].z==0 ) திரும்பு SQLITE_NOMEM;
  }
  திரும்பு SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
நிலையான sqlite3_file *multiplexSubOpen(
  multiplexGroup *pGroup,    /* The multiplexor group */
  முழு iChunk,                /* Which chunk to open.  0==original file */
  முழு *rc,                   /* Result code in and out */
  முழு *pOutFlags,            /* Output flags */
  முழு createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  எனில்( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    திரும்பு 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  எனில்( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    முழு flags, bExists;
    flags = pGroup->flags;
    எனில்( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }இல்லையெனில் எனில்( iChunk==0 ){
      /* Fall through */
    }இல்லையெனில் எனில்( pGroup->aReal[iChunk].z==0 ){
      திரும்பு 0;
    }இல்லையெனில்{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     எனில்( *rc || !bExists ){
        எனில்( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        திரும்பு 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    எனில்( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      திரும்பு 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    எனில்( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      திரும்பு 0;
    }
  }
  திரும்பு pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
நிலையான sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  முழு iChunk,                /* Which chunk to open.  0==original file */
  முழு *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  எனில்( *rc ) திரும்பு 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, வெற்று, 0);
  எனில்( pSub==0 ) திரும்பு 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  திரும்பு sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
நிலையான வெறுமை multiplexControlFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  முழு rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  முழு op = 0;
  முழு iVal;

  எனில்( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    /* extract params */
    op = sqlite3_மதிப்பு_int(argv[0]);
    iVal = sqlite3_மதிப்பு_int(argv[1]);
    /* map function op to file_control op */
    தேர்வு( op ){
      நிலை 1:
        op = MULTIPLEX_CTRL_ENABLE;
        நிறுத்து;
      நிலை 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_SIZE;
        நிறுத்து;
      நிலை 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        நிறுத்து;
      இயல்பு:
        rc = SQLITE_NOTFOUND;
        நிறுத்து;
    }
  }
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
நிலையான முழு multiplexFuncInit(
  sqlite3 *db,
  எழுத்து **pz_Err_செய்தி,
  மாறிலி sqlite3_api_routines *pApi
){
  முழு rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  திரும்பு rc;
}

/*
** Close a single sub-file in the connection group.
*/
நிலையான வெறுமை multiplexSubClose(
  multiplexGroup *pGroup,
  முழு iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  எனில்( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    எனில்( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  memset(&pGroup->aReal[iChunk], 0, அளவு(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
நிலையான வெறுமை multiplexFreeComponents(multiplexGroup *pGroup){
  முழு i;
  க்கு(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
நிலையான முழு multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  மாறிலி எழுத்து *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  முழு flags,                 /* Flags to control the opening */
  முழு *pOutFlags             /* Flags showing results of opening */
){
  முழு rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  முழு nName = 0;
  முழு sz = 0;
  எழுத்து *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  ப்ரதிஜ்ஞா( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  எனில்( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = அளவு(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    எனில்( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  எனில்( rc==SQLITE_OK ){
    மாறிலி எழுத்து *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned எழுத்து)-1;
    pGroup->bTruncate = (unsigned எழுத்து)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (முழு)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    எனில்( zName ){
      எழுத்து *p = (எழுத்து *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    எனில்( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      வெளி முழு sqlite3PendingByte;
#இல்லையெனில்
      முழு sqlite3PendingByte = 0x40000000;
#endif
      போது( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    எனில்( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      எனில்( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    எனில்( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      எனில்( rc==SQLITE_OK && zName ){
        முழு bExists;
        எனில்( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }இல்லையெனில்
        எனில்( sz64==0 ){
          எனில்( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            முழு iChunk = 1;
            செய் {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              எனில்( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                எனில்( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }போது( rc==SQLITE_OK && bExists );
          }
        }இல்லையெனில்{
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
          எனில்( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (முழு)sz64;
          }இல்லையெனில் எனில்( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    எனில்( rc==SQLITE_OK ){
      எனில்( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }இல்லையெனில்{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }இல்லையெனில்{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  திரும்பு rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
நிலையான முழு multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  மாறிலி எழுத்து *zName,         /* Name of file to delete */
  முழு syncDir
){
  முழு rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  எனில்( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    முழு nName = (முழு)strlen(zName);
    எழுத்து *z;
    z = sqlite3_malloc64(nName + 5);
    எனில்( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }இல்லையெனில்{
      முழு iChunk = 0;
      முழு bExists;
      செய்{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }போது( rc==SQLITE_OK && bExists );
      போது( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      எனில்( rc==SQLITE_OK ){
        iChunk = 0;
        செய்{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }போது( rc==SQLITE_OK && bExists );
        போது( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  திரும்பு rc;
}

நிலையான முழு multiplexAccess(sqlite3_vfs *a, மாறிலி எழுத்து *b, முழு c, முழு *d){
  திரும்பு gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
நிலையான முழு multiplexFullPathname(sqlite3_vfs *a, மாறிலி எழுத்து *b, முழு c, எழுத்து *d){
  திரும்பு gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
நிலையான வெறுமை *multiplexDlOpen(sqlite3_vfs *a, மாறிலி எழுத்து *b){
  திரும்பு gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
நிலையான வெறுமை multiplexDlError(sqlite3_vfs *a, முழு b, எழுத்து *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
நிலையான வெறுமை (*multiplexDlSym(sqlite3_vfs *a, வெறுமை *b, மாறிலி எழுத்து *c))(வெறுமை){
  திரும்பு gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
நிலையான வெறுமை multiplexDlClose(sqlite3_vfs *a, வெறுமை *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
நிலையான முழு multiplexRandomness(sqlite3_vfs *a, முழு b, எழுத்து *c){
  திரும்பு gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
நிலையான முழு multiplexSleep(sqlite3_vfs *a, முழு b){
  திரும்பு gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
நிலையான முழு multiplexCurrentTime(sqlite3_vfs *a, இரட்டை *b){
  திரும்பு gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
நிலையான முழு multiplexGetLastError(sqlite3_vfs *a, முழு b, எழுத்து *c){
  எனில்( gMultiplex.pOrigVfs->xGetLastError ){
    திரும்பு gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }இல்லையெனில்{
    திரும்பு 0;
  }
}
நிலையான முழு multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  திரும்பு gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
நிலையான முழு multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  முழு rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  திரும்பு rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
நிலையான முழு multiplexRead(
  sqlite3_file *pConn,
  வெறுமை *pBuf,
  முழு iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  முழு rc = SQLITE_OK;
  எனில்( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, வெற்று, 0);
    எனில்( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }இல்லையெனில்{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }இல்லையெனில்{
    போது( iAmt > 0 ){
      முழு i = (முழு)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, வெற்று, 1);
      எனில்( pSubOpen ){
        முழு extra = ((முழு)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        எனில்( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        எனில்( rc!=SQLITE_OK ) நிறுத்து;
        pBuf = (எழுத்து *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }இல்லையெனில்{
        rc = SQLITE_IOERR_READ;
        நிறுத்து;
      }
    }
  }

  திரும்பு rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
நிலையான முழு multiplexWrite(
  sqlite3_file *pConn,
  மாறிலி வெறுமை *pBuf,
  முழு iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  முழு rc = SQLITE_OK;
  எனில்( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, வெற்று, 0);
    எனில்( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }இல்லையெனில்{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }இல்லையெனில்{
    போது( rc==SQLITE_OK && iAmt>0 ){
      முழு i = (முழு)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, வெற்று, 1);
      எனில்( pSubOpen ){
        முழு extra = ((முழு)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        எனில்( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (எழுத்து *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  திரும்பு rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
நிலையான முழு multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  முழு rc = SQLITE_OK;
  எனில்( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, வெற்று, 0);
    எனில்( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }இல்லையெனில்{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }இல்லையெனில்{
    முழு i;
    முழு iBaseGroup = (முழு)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    க்கு(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      எனில்( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }இல்லையெனில்{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        எனில்( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    எனில்( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      எனில்( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    எனில்( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  திரும்பு rc;
}

/* Pass xSync requests through to the original VFS without change
*/
நிலையான முழு multiplexSync(sqlite3_file *pConn, முழு flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  முழு rc = SQLITE_OK;
  முழு i;
  க்கு(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    எனில்( pSubOpen ){
      முழு rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      எனில்( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  திரும்பு rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
நிலையான முழு multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  முழு rc = SQLITE_OK;
  முழு i;
  எனில்( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, வெற்று, 0);
    எனில்( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }இல்லையெனில்{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }இல்லையெனில்{
    *pSize = 0;
    க்கு(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      எனில்( sz==0 ) நிறுத்து;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  திரும்பு rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
நிலையான முழு multiplexLock(sqlite3_file *pConn, முழு lock){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen ){
    திரும்பு pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  திரும்பு SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
நிலையான முழு multiplexUnlock(sqlite3_file *pConn, முழு lock){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen ){
    திரும்பு pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  திரும்பு SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
நிலையான முழு multiplexCheckReservedLock(sqlite3_file *pConn, முழு *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen ){
    திரும்பு pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  திரும்பு SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
நிலையான முழு multiplexFileControl(sqlite3_file *pConn, முழு op, வெறுமை *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  முழு rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  எனில்( !gMultiplex.isInitialized ) திரும்பு SQLITE_MISUSE;
  தேர்வு( op ){
    நிலை MULTIPLEX_CTRL_ENABLE:
      எனில்( pArg ) {
        முழு bEnabled = *(முழு *)pArg;
        pGroup->bEnabled = (unsigned எழுத்து)bEnabled;
        rc = SQLITE_OK;
      }
      நிறுத்து;
    நிலை MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      எனில்( pArg ) {
        unsigned முழு szChunk = *(unsigned*)pArg;
        எனில்( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }இல்லையெனில்{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_SIZE-1));
          szChunk &= ~(MAX_PAGE_SIZE-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      நிறுத்து;
    நிலை MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      நிறுத்து;
    நிலை SQLITE_FCNTL_SIZE_HINT:
    நிலை SQLITE_FCNTL_CHUNK_SIZE:
      /* no-op these */
      rc = SQLITE_OK;
      நிறுத்து;
    நிலை SQLITE_FCNTL_PRAGMA: {
      எழுத்து **aFcntl = (எழுத்து**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      எனில்( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (வெறுமை)multiplexFileSize(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        எனில்( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          எனில்( aFcntl[2] && aFcntl[2][0] ){
            எனில்( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }இல்லையெனில்
            எனில்( sqlite3_stricmp(aFcntl[2], "off")==0
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
          நிறுத்து;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        எனில்( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          நிறுத்து;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        எனில்( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          நிறுத்து;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        எனில்( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          முழு n = 0;
          முழு ii;
          க்கு(ii=0; ii<pGroup->nReal; ii++){
            எனில்( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          நிறுத்து;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    இயல்பு:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, வெற்று, 0);
      எனில்( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        எனில்( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(எழுத்து**)pArg = sqlite3_mprintf("multiplex/%z", *(எழுத்து**)pArg);
        }
      }
      நிறுத்து;
  }
  திரும்பு rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
நிலையான முழு multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    திரும்பு pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  திரும்பு DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
நிலையான முழு multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen ){
    திரும்பு pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  திரும்பு 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
நிலையான முழு multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  முழு iRegion,                    /* Region to retrieve */
  முழு szRegion,                   /* Size of regions */
  முழு bExtend,                    /* True to extend file if necessary */
  வெறுமை நிலையற்ற **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen ){
    திரும்பு pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  திரும்பு SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
நிலையான முழு multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  முழு ofst,                  /* First lock to acquire or release */
  முழு n,                     /* Number of locks to acquire or release */
  முழு flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen ){
    திரும்பு pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  திரும்பு SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
நிலையான வெறுமை multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
நிலையான முழு multiplexShmUnmap(sqlite3_file *pConn, முழு deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  முழு rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, வெற்று, 0);
  எனில்( pSubOpen ){
    திரும்பு pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  திரும்பு SQLITE_OK;
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
முழு sqlite3_multiplex_initialize(மாறிலி எழுத்து *zOrigVfsName, முழு makeDefault){
  sqlite3_vfs *pOrigVfs;
  எனில்( gMultiplex.isInitialized ) திரும்பு SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  எனில்( pOrigVfs==0 ) திரும்பு SQLITE_ERROR;
  ப்ரதிஜ்ஞா( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += அளவு(multiplexConn);
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

  sqlite3_auto_extension((வெறுமை(*)(வெறுமை))multiplexFuncInit);

  திரும்பு SQLITE_OK;
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
முழு sqlite3_multiplex_shutdown(முழு eForce){
  முழு rc = SQLITE_OK;
  எனில்( gMultiplex.isInitialized==0 ) திரும்பு SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, அளவு(gMultiplex));
  திரும்பு rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#அந்தர்பாவ "tclsqlite.h"
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
நிலையான முழு SQLITE_TCLAPI test_multiplex_initialize(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  மாறிலி எழுத்து *zName;              /* Name of new multiplex VFS */
  முழு makeDefault;                /* True to make the new VFS the default */
  முழு rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    திரும்பு TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  எனில்( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) திரும்பு TCL_ERROR;
  எனில்( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_STATIC);

  திரும்பு TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
நிலையான முழு SQLITE_TCLAPI test_multiplex_shutdown(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  எனில்( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  எனில்( (objc!=1 && objc!=2) ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "?-force?");
    திரும்பு TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_STATIC);

  திரும்பு TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
நிலையான முழு SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;                         /* Return code from file_control() */
  முழு idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  முழு i_மதிப்பு = 0;
  வெறுமை *pArg = 0;

  கட்டமைப்பு SubCommand {
    மாறிலி எழுத்து *zName;
    முழு op;
    முழு argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  எனில்( objc!=5 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    திரும்பு TCL_ERROR;
  }

  எனில்( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", வெற்று);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", வெற்று);
    திரும்பு TCL_ERROR;
  }இல்லையெனில்{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, அளவு(aSub[0]), "sub-command", 0, &idx
  );
  எனில்( rc!=TCL_OK ) திரும்பு rc;

  தேர்வு( aSub[idx].argtype ){
    நிலை 1:
      எனில்( Tcl_GetIntFromObj(interp, objv[4], &i_மதிப்பு) ){
        திரும்பு TCL_ERROR;
      }
      pArg = (வெறுமை *)&i_மதிப்பு;
      நிறுத்து;
    இயல்பு:
      Tcl_Wrong_இலக்கம்_Args(interp, 4, objv, "SUB-COMMAND");
      திரும்பு TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_STATIC);
  திரும்பு (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
முழு Sqlitemultiplex_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  முழு i;

  க்கு(i=0; i<அளவு(aCmd)/அளவு(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  திரும்பு TCL_OK;
}
#endif
