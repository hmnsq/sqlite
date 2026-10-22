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
#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ "test_multiplex.h"

#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#അന്തര്ഭാവ "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#define UNUSED_PARAMETER(x) (ശൂന്യം)(x)
#define MAX_PAGE_SIZE       0x10000
#define DEFAULT_SECTOR_SIZE 0x1000

/* Maximum chunk number */
#define MX_CHUNK_സംഖ്യ 299

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
തരനിർവചനം ഘടന multiplexGroup multiplexGroup;
തരനിർവചനം ഘടന multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
ഘടന multiplexGroup {
  ഘടന multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    അക്ഷരം *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  പൂർണ്ണസംഖ്യ nReal;                       /* Number of chunks */
  അക്ഷരം *zName;                     /* Base filename of this group */
  പൂർണ്ണസംഖ്യ nName;                       /* Length of base filename */
  പൂർണ്ണസംഖ്യ flags;                       /* Flags used for original opening */
  unsigned പൂർണ്ണസംഖ്യ szChunk;            /* Chunk size used for this group */
  unsigned അക്ഷരം bEnabled;          /* TRUE to use Multiplex VFS for this file */
  unsigned അക്ഷരം bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ഘടന multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
സ്ഥിതം ഘടന {
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
  പൂർണ്ണസംഖ്യ isInitialized;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexStrlen30(സ്ഥിരം അക്ഷരം *z){
  സ്ഥിരം അക്ഷരം *z2 = z;
  എങ്കിൽ( z==0 ) തിരികെ 0;
  ആകുമ്പോൾ( *z2 ){ z2++; }
  തിരികെ 0x3fffffff & (പൂർണ്ണസംഖ്യ)(z2 - z);
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
സ്ഥിതം ശൂന്യം multiplexFilename(
  സ്ഥിരം അക്ഷരം *zBase,              /* Filename for chunk 0 */
  പൂർണ്ണസംഖ്യ nBase,                      /* Size of zBase in bytes (without \0) */
  പൂർണ്ണസംഖ്യ flags,                      /* Flags used to open file */
  പൂർണ്ണസംഖ്യ iChunk,                     /* Chunk to generate filename for */
  അക്ഷരം *zOut                      /* Buffer to write generated name to */
){
  പൂർണ്ണസംഖ്യ n = nBase;
  memcpy(zOut, zBase, n+1);
  എങ്കിൽ( iChunk!=0 && iChunk<=MX_CHUNK_സംഖ്യ ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    എങ്കിൽ( i>=n-4 ) n = i+1;
    എങ്കിൽ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }അല്ലെങ്കിൽ എങ്കിൽ( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_OFFSET;
    }
#endif
    sqlite3_snprintf(4,&zOut[n],"%03d",iChunk);
    n += 3;
  }

  പ്രതിജ്ഞാ( zOut[n]=='\0' );
  zOut[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexSubFilename(multiplexGroup *pGroup, പൂർണ്ണസംഖ്യ iChunk){
  എങ്കിൽ( iChunk>=pGroup->nReal ){
    ഘടന multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*വലുപ്പം(*p));
    എങ്കിൽ( p==0 ){
      തിരികെ SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, വലുപ്പം(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  എങ്കിൽ( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    അക്ഷരം *z;
    പൂർണ്ണസംഖ്യ n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    എങ്കിൽ( z==0 ){
      തിരികെ SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (അക്ഷരം*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    എങ്കിൽ( pGroup->aReal[iChunk].z==0 ) തിരികെ SQLITE_NOMEM;
  }
  തിരികെ SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
സ്ഥിതം sqlite3_file *multiplexSubOpen(
  multiplexGroup *pGroup,    /* The multiplexor group */
  പൂർണ്ണസംഖ്യ iChunk,                /* Which chunk to open.  0==original file */
  പൂർണ്ണസംഖ്യ *rc,                   /* Result code in and out */
  പൂർണ്ണസംഖ്യ *pOutFlags,            /* Output flags */
  പൂർണ്ണസംഖ്യ createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  എങ്കിൽ( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    തിരികെ 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  എങ്കിൽ( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    പൂർണ്ണസംഖ്യ flags, bExists;
    flags = pGroup->flags;
    എങ്കിൽ( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }അല്ലെങ്കിൽ എങ്കിൽ( iChunk==0 ){
      /* Fall through */
    }അല്ലെങ്കിൽ എങ്കിൽ( pGroup->aReal[iChunk].z==0 ){
      തിരികെ 0;
    }അല്ലെങ്കിൽ{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     എങ്കിൽ( *rc || !bExists ){
        എങ്കിൽ( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        തിരികെ 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    എങ്കിൽ( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      തിരികെ 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    എങ്കിൽ( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      തിരികെ 0;
    }
  }
  തിരികെ pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
സ്ഥിതം sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  പൂർണ്ണസംഖ്യ iChunk,                /* Which chunk to open.  0==original file */
  പൂർണ്ണസംഖ്യ *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  എങ്കിൽ( *rc ) തിരികെ 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, ശൂന്യ, 0);
  എങ്കിൽ( pSub==0 ) തിരികെ 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  തിരികെ sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
സ്ഥിതം ശൂന്യം multiplexControlFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  പൂർണ്ണസംഖ്യ op = 0;
  പൂർണ്ണസംഖ്യ iVal;

  എങ്കിൽ( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ{
    /* extract params */
    op = sqlite3_മൂല്യം_int(argv[0]);
    iVal = sqlite3_മൂല്യം_int(argv[1]);
    /* map function op to file_control op */
    തിരഞ്ഞെടുപ്പ്( op ){
      അവസ്ഥ 1:
        op = MULTIPLEX_CTRL_ENABLE;
        നിർത്തുക;
      അവസ്ഥ 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_SIZE;
        നിർത്തുക;
      അവസ്ഥ 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
        rc = SQLITE_NOTFOUND;
        നിർത്തുക;
    }
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexFuncInit(
  sqlite3 *db,
  അക്ഷരം **pz_Err_സന്ദേശം,
  സ്ഥിരം sqlite3_api_routines *pApi
){
  പൂർണ്ണസംഖ്യ rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  തിരികെ rc;
}

/*
** Close a single sub-file in the connection group.
*/
സ്ഥിതം ശൂന്യം multiplexSubClose(
  multiplexGroup *pGroup,
  പൂർണ്ണസംഖ്യ iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  എങ്കിൽ( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    എങ്കിൽ( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  memset(&pGroup->aReal[iChunk], 0, വലുപ്പം(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
സ്ഥിതം ശൂന്യം multiplexFreeComponents(multiplexGroup *pGroup){
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  സ്ഥിരം അക്ഷരം *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  പൂർണ്ണസംഖ്യ flags,                 /* Flags to control the opening */
  പൂർണ്ണസംഖ്യ *pOutFlags             /* Flags showing results of opening */
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  പൂർണ്ണസംഖ്യ nName = 0;
  പൂർണ്ണസംഖ്യ sz = 0;
  അക്ഷരം *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  പ്രതിജ്ഞാ( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  എങ്കിൽ( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = വലുപ്പം(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    എങ്കിൽ( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    സ്ഥിരം അക്ഷരം *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned അക്ഷരം)-1;
    pGroup->bTruncate = (unsigned അക്ഷരം)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (പൂർണ്ണസംഖ്യ)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    എങ്കിൽ( zName ){
      അക്ഷരം *p = (അക്ഷരം *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    എങ്കിൽ( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3PendingByte;
#അല്ലെങ്കിൽ
      പൂർണ്ണസംഖ്യ sqlite3PendingByte = 0x40000000;
#endif
      ആകുമ്പോൾ( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    എങ്കിൽ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      എങ്കിൽ( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    എങ്കിൽ( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      എങ്കിൽ( rc==SQLITE_OK && zName ){
        പൂർണ്ണസംഖ്യ bExists;
        എങ്കിൽ( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }അല്ലെങ്കിൽ
        എങ്കിൽ( sz64==0 ){
          എങ്കിൽ( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            പൂർണ്ണസംഖ്യ iChunk = 1;
            ചെയ്യുക {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              എങ്കിൽ( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                എങ്കിൽ( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }ആകുമ്പോൾ( rc==SQLITE_OK && bExists );
          }
        }അല്ലെങ്കിൽ{
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
          എങ്കിൽ( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (പൂർണ്ണസംഖ്യ)sz64;
          }അല്ലെങ്കിൽ എങ്കിൽ( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    എങ്കിൽ( rc==SQLITE_OK ){
      എങ്കിൽ( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }അല്ലെങ്കിൽ{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }അല്ലെങ്കിൽ{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  തിരികെ rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  സ്ഥിരം അക്ഷരം *zName,         /* Name of file to delete */
  പൂർണ്ണസംഖ്യ syncDir
){
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  എങ്കിൽ( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    പൂർണ്ണസംഖ്യ nName = (പൂർണ്ണസംഖ്യ)strlen(zName);
    അക്ഷരം *z;
    z = sqlite3_malloc64(nName + 5);
    എങ്കിൽ( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ iChunk = 0;
      പൂർണ്ണസംഖ്യ bExists;
      ചെയ്യുക{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }ആകുമ്പോൾ( rc==SQLITE_OK && bExists );
      ആകുമ്പോൾ( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      എങ്കിൽ( rc==SQLITE_OK ){
        iChunk = 0;
        ചെയ്യുക{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }ആകുമ്പോൾ( rc==SQLITE_OK && bExists );
        ആകുമ്പോൾ( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexAccess(sqlite3_vfs *a, സ്ഥിരം അക്ഷരം *b, പൂർണ്ണസംഖ്യ c, പൂർണ്ണസംഖ്യ *d){
  തിരികെ gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexFullPathname(sqlite3_vfs *a, സ്ഥിരം അക്ഷരം *b, പൂർണ്ണസംഖ്യ c, അക്ഷരം *d){
  തിരികെ gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
സ്ഥിതം ശൂന്യം *multiplexDlOpen(sqlite3_vfs *a, സ്ഥിരം അക്ഷരം *b){
  തിരികെ gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
സ്ഥിതം ശൂന്യം multiplexDlError(sqlite3_vfs *a, പൂർണ്ണസംഖ്യ b, അക്ഷരം *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
സ്ഥിതം ശൂന്യം (*multiplexDlSym(sqlite3_vfs *a, ശൂന്യം *b, സ്ഥിരം അക്ഷരം *c))(ശൂന്യം){
  തിരികെ gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
സ്ഥിതം ശൂന്യം multiplexDlClose(sqlite3_vfs *a, ശൂന്യം *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexRandomness(sqlite3_vfs *a, പൂർണ്ണസംഖ്യ b, അക്ഷരം *c){
  തിരികെ gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexSleep(sqlite3_vfs *a, പൂർണ്ണസംഖ്യ b){
  തിരികെ gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexCurrentTime(sqlite3_vfs *a, ഇരട്ട *b){
  തിരികെ gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexGetLastError(sqlite3_vfs *a, പൂർണ്ണസംഖ്യ b, അക്ഷരം *c){
  എങ്കിൽ( gMultiplex.pOrigVfs->xGetLastError ){
    തിരികെ gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
  }
}
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  തിരികെ gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  തിരികെ rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexRead(
  sqlite3_file *pConn,
  ശൂന്യം *pBuf,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  എങ്കിൽ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ശൂന്യ, 0);
    എങ്കിൽ( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }അല്ലെങ്കിൽ{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }അല്ലെങ്കിൽ{
    ആകുമ്പോൾ( iAmt > 0 ){
      പൂർണ്ണസംഖ്യ i = (പൂർണ്ണസംഖ്യ)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, ശൂന്യ, 1);
      എങ്കിൽ( pSubOpen ){
        പൂർണ്ണസംഖ്യ extra = ((പൂർണ്ണസംഖ്യ)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        എങ്കിൽ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        എങ്കിൽ( rc!=SQLITE_OK ) നിർത്തുക;
        pBuf = (അക്ഷരം *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }അല്ലെങ്കിൽ{
        rc = SQLITE_IOERR_READ;
        നിർത്തുക;
      }
    }
  }

  തിരികെ rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexWrite(
  sqlite3_file *pConn,
  സ്ഥിരം ശൂന്യം *pBuf,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  എങ്കിൽ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ശൂന്യ, 0);
    എങ്കിൽ( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }അല്ലെങ്കിൽ{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }അല്ലെങ്കിൽ{
    ആകുമ്പോൾ( rc==SQLITE_OK && iAmt>0 ){
      പൂർണ്ണസംഖ്യ i = (പൂർണ്ണസംഖ്യ)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, ശൂന്യ, 1);
      എങ്കിൽ( pSubOpen ){
        പൂർണ്ണസംഖ്യ extra = ((പൂർണ്ണസംഖ്യ)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        എങ്കിൽ( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (അക്ഷരം *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  തിരികെ rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  എങ്കിൽ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ശൂന്യ, 0);
    എങ്കിൽ( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }അല്ലെങ്കിൽ{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ iBaseGroup = (പൂർണ്ണസംഖ്യ)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    വേണ്ടി(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      എങ്കിൽ( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }അല്ലെങ്കിൽ{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        എങ്കിൽ( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    എങ്കിൽ( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      എങ്കിൽ( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    എങ്കിൽ( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  തിരികെ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexSync(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    എങ്കിൽ( pSubOpen ){
      പൂർണ്ണസംഖ്യ rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      എങ്കിൽ( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  തിരികെ rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ശൂന്യ, 0);
    എങ്കിൽ( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }അല്ലെങ്കിൽ{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }അല്ലെങ്കിൽ{
    *pSize = 0;
    വേണ്ടി(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      എങ്കിൽ( sz==0 ) നിർത്തുക;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  തിരികെ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexLock(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ lock){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen ){
    തിരികെ pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  തിരികെ SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexUnlock(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ lock){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen ){
    തിരികെ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  തിരികെ SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexCheckReservedLock(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen ){
    തിരികെ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  തിരികെ SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexFileControl(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  പൂർണ്ണസംഖ്യ rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  എങ്കിൽ( !gMultiplex.isInitialized ) തിരികെ SQLITE_MISUSE;
  തിരഞ്ഞെടുപ്പ്( op ){
    അവസ്ഥ MULTIPLEX_CTRL_ENABLE:
      എങ്കിൽ( pArg ) {
        പൂർണ്ണസംഖ്യ bEnabled = *(പൂർണ്ണസംഖ്യ *)pArg;
        pGroup->bEnabled = (unsigned അക്ഷരം)bEnabled;
        rc = SQLITE_OK;
      }
      നിർത്തുക;
    അവസ്ഥ MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      എങ്കിൽ( pArg ) {
        unsigned പൂർണ്ണസംഖ്യ szChunk = *(unsigned*)pArg;
        എങ്കിൽ( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }അല്ലെങ്കിൽ{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_SIZE-1));
          szChunk &= ~(MAX_PAGE_SIZE-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      നിർത്തുക;
    അവസ്ഥ MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      നിർത്തുക;
    അവസ്ഥ SQLITE_FCNTL_SIZE_HINT:
    അവസ്ഥ SQLITE_FCNTL_CHUNK_SIZE:
      /* no-op these */
      rc = SQLITE_OK;
      നിർത്തുക;
    അവസ്ഥ SQLITE_FCNTL_PRAGMA: {
      അക്ഷരം **aFcntl = (അക്ഷരം**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      എങ്കിൽ( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (ശൂന്യം)multiplexFileSize(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        എങ്കിൽ( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          എങ്കിൽ( aFcntl[2] && aFcntl[2][0] ){
            എങ്കിൽ( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }അല്ലെങ്കിൽ
            എങ്കിൽ( sqlite3_stricmp(aFcntl[2], "off")==0
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
          നിർത്തുക;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        എങ്കിൽ( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          നിർത്തുക;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        എങ്കിൽ( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          നിർത്തുക;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        എങ്കിൽ( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          പൂർണ്ണസംഖ്യ n = 0;
          പൂർണ്ണസംഖ്യ ii;
          വേണ്ടി(ii=0; ii<pGroup->nReal; ii++){
            എങ്കിൽ( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          നിർത്തുക;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    സ്ഥിരസ്ഥിതി:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ശൂന്യ, 0);
      എങ്കിൽ( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        എങ്കിൽ( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(അക്ഷരം**)pArg = sqlite3_mprintf("multiplex/%z", *(അക്ഷരം**)pArg);
        }
      }
      നിർത്തുക;
  }
  തിരികെ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    തിരികെ pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  തിരികെ DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen ){
    തിരികെ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  തിരികെ 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  പൂർണ്ണസംഖ്യ iRegion,                    /* Region to retrieve */
  പൂർണ്ണസംഖ്യ szRegion,                   /* Size of regions */
  പൂർണ്ണസംഖ്യ bExtend,                    /* True to extend file if necessary */
  ശൂന്യം അസ്ഥിര **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen ){
    തിരികെ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  തിരികെ SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  പൂർണ്ണസംഖ്യ ofst,                  /* First lock to acquire or release */
  പൂർണ്ണസംഖ്യ n,                     /* Number of locks to acquire or release */
  പൂർണ്ണസംഖ്യ flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen ){
    തിരികെ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  തിരികെ SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
സ്ഥിതം ശൂന്യം multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ multiplexShmUnmap(sqlite3_file *pConn, പൂർണ്ണസംഖ്യ deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ശൂന്യ, 0);
  എങ്കിൽ( pSubOpen ){
    തിരികെ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3_multiplex_initialize(സ്ഥിരം അക്ഷരം *zOrigVfsName, പൂർണ്ണസംഖ്യ makeDefault){
  sqlite3_vfs *pOrigVfs;
  എങ്കിൽ( gMultiplex.isInitialized ) തിരികെ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  എങ്കിൽ( pOrigVfs==0 ) തിരികെ SQLITE_ERROR;
  പ്രതിജ്ഞാ( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += വലുപ്പം(multiplexConn);
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

  sqlite3_auto_extension((ശൂന്യം(*)(ശൂന്യം))multiplexFuncInit);

  തിരികെ SQLITE_OK;
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
പൂർണ്ണസംഖ്യ sqlite3_multiplex_shutdown(പൂർണ്ണസംഖ്യ eForce){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  എങ്കിൽ( gMultiplex.isInitialized==0 ) തിരികെ SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, വലുപ്പം(gMultiplex));
  തിരികെ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#അന്തര്ഭാവ "tclsqlite.h"
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_multiplex_initialize(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *zName;              /* Name of new multiplex VFS */
  പൂർണ്ണസംഖ്യ makeDefault;                /* True to make the new VFS the default */
  പൂർണ്ണസംഖ്യ rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    തിരികെ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);

  തിരികെ TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_multiplex_shutdown(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  എങ്കിൽ( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  എങ്കിൽ( (objc!=1 && objc!=2) ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "?-force?");
    തിരികെ TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);

  തിരികെ TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;                         /* Return code from file_control() */
  പൂർണ്ണസംഖ്യ idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  പൂർണ്ണസംഖ്യ i_മൂല്യം = 0;
  ശൂന്യം *pArg = 0;

  ഘടന SubCommand {
    സ്ഥിരം അക്ഷരം *zName;
    പൂർണ്ണസംഖ്യ op;
    പൂർണ്ണസംഖ്യ argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  എങ്കിൽ( objc!=5 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", ശൂന്യ);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }അല്ലെങ്കിൽ{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, വലുപ്പം(aSub[0]), "sub-command", 0, &idx
  );
  എങ്കിൽ( rc!=TCL_OK ) തിരികെ rc;

  തിരഞ്ഞെടുപ്പ്( aSub[idx].argtype ){
    അവസ്ഥ 1:
      എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[4], &i_മൂല്യം) ){
        തിരികെ TCL_ERROR;
      }
      pArg = (ശൂന്യം *)&i_മൂല്യം;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      Tcl_Wrong_അക്കം_Args(interp, 4, objv, "SUB-COMMAND");
      തിരികെ TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);
  തിരികെ (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
പൂർണ്ണസംഖ്യ Sqlitemultiplex_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  പൂർണ്ണസംഖ്യ i;

  വേണ്ടി(i=0; i<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  തിരികെ TCL_OK;
}
#endif
