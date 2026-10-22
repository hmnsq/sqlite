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
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ "test_multiplex.h"

#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#ଅନ୍ତର୍ଭାଵ "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#define UNUSED_PARAMETER(x) (ଶୂନ୍ୟ)(x)
#define MAX_PAGE_SIZE       0x10000
#define DEFAULT_SECTOR_SIZE 0x1000

/* Maximum chunk number */
#define MX_CHUNK_ସଂଖ୍ୟା 299

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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା multiplexGroup multiplexGroup;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
ସଂରଚନା multiplexGroup {
  ସଂରଚନା multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    ଅକ୍ଷର *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nReal;                       /* Number of chunks */
  ଅକ୍ଷର *zName;                     /* Base filename of this group */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName;                       /* Length of base filename */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;                       /* Flags used for original opening */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା szChunk;            /* Chunk size used for this group */
  unsigned ଅକ୍ଷର bEnabled;          /* TRUE to use Multiplex VFS for this file */
  unsigned ଅକ୍ଷର bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ସଂରଚନା multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
ସ୍ଥିର ସଂରଚନା {
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInitialized;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexStrlen30(ଅଚଳ ଅକ୍ଷର *z){
  ଅଚଳ ଅକ୍ଷର *z2 = z;
  ଯଦି( z==0 ) ଫେରନ୍ତୁ 0;
  ଯେତେବେଳେ( *z2 ){ z2++; }
  ଫେରନ୍ତୁ 0x3fffffff & (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(z2 - z);
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
ସ୍ଥିର ଶୂନ୍ୟ multiplexFilename(
  ଅଚଳ ଅକ୍ଷର *zBase,              /* Filename for chunk 0 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBase,                      /* Size of zBase in bytes (without \0) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,                      /* Flags used to open file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChunk,                     /* Chunk to generate filename for */
  ଅକ୍ଷର *zOut                      /* Buffer to write generated name to */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = nBase;
  memcpy(zOut, zBase, n+1);
  ଯଦି( iChunk!=0 && iChunk<=MX_CHUNK_ସଂଖ୍ୟା ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    ଯଦି( i>=n-4 ) n = i+1;
    ଯଦି( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }ଅନ୍ୟଥା ଯଦି( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_OFFSET;
    }
#endif
    sqlite3_snprintf(4,&zOut[n],"%03d",iChunk);
    n += 3;
  }

  ପ୍ରତିଜ୍ଞା( zOut[n]=='\0' );
  zOut[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexSubFilename(multiplexGroup *pGroup, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChunk){
  ଯଦି( iChunk>=pGroup->nReal ){
    ସଂରଚନା multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*ଆକାର(*p));
    ଯଦି( p==0 ){
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, ଆକାର(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  ଯଦି( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    ଅକ୍ଷର *z;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    ଯଦି( z==0 ){
      ଫେରନ୍ତୁ SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (ଅକ୍ଷର*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    ଯଦି( pGroup->aReal[iChunk].z==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
ସ୍ଥିର sqlite3_file *multiplexSubOpen(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChunk,                /* Which chunk to open.  0==original file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *rc,                   /* Result code in and out */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags,            /* Output flags */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  ଯଦି( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    ଫେରନ୍ତୁ 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  ଯଦି( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, bExists;
    flags = pGroup->flags;
    ଯଦି( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }ଅନ୍ୟଥା ଯଦି( iChunk==0 ){
      /* Fall through */
    }ଅନ୍ୟଥା ଯଦି( pGroup->aReal[iChunk].z==0 ){
      ଫେରନ୍ତୁ 0;
    }ଅନ୍ୟଥା{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     ଯଦି( *rc || !bExists ){
        ଯଦି( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        ଫେରନ୍ତୁ 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    ଯଦି( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      ଫେରନ୍ତୁ 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    ଯଦି( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      ଫେରନ୍ତୁ 0;
    }
  }
  ଫେରନ୍ତୁ pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
ସ୍ଥିର sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChunk,                /* Which chunk to open.  0==original file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  ଯଦି( *rc ) ଫେରନ୍ତୁ 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, ଖାଲି, 0);
  ଯଦି( pSub==0 ) ଫେରନ୍ତୁ 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  ଫେରନ୍ତୁ sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
ସ୍ଥିର ଶୂନ୍ୟ multiplexControlFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iVal;

  ଯଦି( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }ଅନ୍ୟଥା{
    /* extract params */
    op = sqlite3_ମୂଲ୍ୟ_int(argv[0]);
    iVal = sqlite3_ମୂଲ୍ୟ_int(argv[1]);
    /* map function op to file_control op */
    ବାଛ( op ){
      ସ୍ଥିତି 1:
        op = MULTIPLEX_CTRL_ENABLE;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_SIZE;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        rc = SQLITE_NOTFOUND;
        ଭାଙ୍ଗ;
    }
  }
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexFuncInit(
  sqlite3 *db,
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ,
  ଅଚଳ sqlite3_api_routines *pApi
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  ଫେରନ୍ତୁ rc;
}

/*
** Close a single sub-file in the connection group.
*/
ସ୍ଥିର ଶୂନ୍ୟ multiplexSubClose(
  multiplexGroup *pGroup,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  ଯଦି( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    ଯଦି( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  memset(&pGroup->aReal[iChunk], 0, ଆକାର(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
ସ୍ଥିର ଶୂନ୍ୟ multiplexFreeComponents(multiplexGroup *pGroup){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ଅଚଳ ଅକ୍ଷର *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,                 /* Flags to control the opening */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags             /* Flags showing results of opening */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = 0;
  ଅକ୍ଷର *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  ପ୍ରତିଜ୍ଞା( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  ଯଦି( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = ଆକାର(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    ଯଦି( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  ଯଦି( rc==SQLITE_OK ){
    ଅଚଳ ଅକ୍ଷର *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned ଅକ୍ଷର)-1;
    pGroup->bTruncate = (unsigned ଅକ୍ଷର)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    ଯଦି( zName ){
      ଅକ୍ଷର *p = (ଅକ୍ଷର *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    ଯଦି( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PendingByte;
#ଅନ୍ୟଥା
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PendingByte = 0x40000000;
#endif
      ଯେତେବେଳେ( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    ଯଦି( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      ଯଦି( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    ଯଦି( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      ଯଦି( rc==SQLITE_OK && zName ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା bExists;
        ଯଦି( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }ଅନ୍ୟଥା
        ଯଦି( sz64==0 ){
          ଯଦି( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChunk = 1;
            କର {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              ଯଦି( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                ଯଦି( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }ଯେତେବେଳେ( rc==SQLITE_OK && bExists );
          }
        }ଅନ୍ୟଥା{
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
          ଯଦି( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sz64;
          }ଅନ୍ୟଥା ଯଦି( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    ଯଦି( rc==SQLITE_OK ){
      ଯଦି( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }ଅନ୍ୟଥା{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }ଅନ୍ୟଥା{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  ଫେରନ୍ତୁ rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ଅଚଳ ଅକ୍ଷର *zName,         /* Name of file to delete */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା syncDir
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  ଯଦି( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zName);
    ଅକ୍ଷର *z;
    z = sqlite3_malloc64(nName + 5);
    ଯଦି( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChunk = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bExists;
      କର{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }ଯେତେବେଳେ( rc==SQLITE_OK && bExists );
      ଯେତେବେଳେ( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      ଯଦି( rc==SQLITE_OK ){
        iChunk = 0;
        କର{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }ଯେତେବେଳେ( rc==SQLITE_OK && bExists );
        ଯେତେବେଳେ( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexAccess(sqlite3_vfs *a, ଅଚଳ ଅକ୍ଷର *b, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *d){
  ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexFullPathname(sqlite3_vfs *a, ଅଚଳ ଅକ୍ଷର *b, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, ଅକ୍ଷର *d){
  ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
ସ୍ଥିର ଶୂନ୍ୟ *multiplexDlOpen(sqlite3_vfs *a, ଅଚଳ ଅକ୍ଷର *b){
  ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
ସ୍ଥିର ଶୂନ୍ୟ multiplexDlError(sqlite3_vfs *a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b, ଅକ୍ଷର *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
ସ୍ଥିର ଶୂନ୍ୟ (*multiplexDlSym(sqlite3_vfs *a, ଶୂନ୍ୟ *b, ଅଚଳ ଅକ୍ଷର *c))(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
ସ୍ଥିର ଶୂନ୍ୟ multiplexDlClose(sqlite3_vfs *a, ଶୂନ୍ୟ *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexRandomness(sqlite3_vfs *a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b, ଅକ୍ଷର *c){
  ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexSleep(sqlite3_vfs *a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b){
  ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexCurrentTime(sqlite3_vfs *a, ଦ୍ୱିଗୁଣ *b){
  ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexGetLastError(sqlite3_vfs *a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b, ଅକ୍ଷର *c){
  ଯଦି( gMultiplex.pOrigVfs->xGetLastError ){
    ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
  }
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  ଫେରନ୍ତୁ gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  ଫେରନ୍ତୁ rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexRead(
  sqlite3_file *pConn,
  ଶୂନ୍ୟ *pBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଯଦି( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ଖାଲି, 0);
    ଯଦି( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }ଅନ୍ୟଥା{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ଅନ୍ୟଥା{
    ଯେତେବେଳେ( iAmt > 0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, ଖାଲି, 1);
      ଯଦି( pSubOpen ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        ଯଦି( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        ଯଦି( rc!=SQLITE_OK ) ଭାଙ୍ଗ;
        pBuf = (ଅକ୍ଷର *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }ଅନ୍ୟଥା{
        rc = SQLITE_IOERR_READ;
        ଭାଙ୍ଗ;
      }
    }
  }

  ଫେରନ୍ତୁ rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexWrite(
  sqlite3_file *pConn,
  ଅଚଳ ଶୂନ୍ୟ *pBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଯଦି( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ଖାଲି, 0);
    ଯଦି( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }ଅନ୍ୟଥା{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ଅନ୍ୟଥା{
    ଯେତେବେଳେ( rc==SQLITE_OK && iAmt>0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, ଖାଲି, 1);
      ଯଦି( pSubOpen ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        ଯଦି( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (ଅକ୍ଷର *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  ଫେରନ୍ତୁ rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଯଦି( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ଖାଲି, 0);
    ଯଦି( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }ଅନ୍ୟଥା{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBaseGroup = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    ପାଇଁ(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      ଯଦି( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }ଅନ୍ୟଥା{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        ଯଦି( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    ଯଦି( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      ଯଦି( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    ଯଦି( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  ଫେରନ୍ତୁ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexSync(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    ଯଦି( pSubOpen ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      ଯଦି( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  ଫେରନ୍ତୁ rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ଖାଲି, 0);
    ଯଦି( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }ଅନ୍ୟଥା{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }ଅନ୍ୟଥା{
    *pSize = 0;
    ପାଇଁ(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      ଯଦି( sz==0 ) ଭାଙ୍ଗ;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  ଫେରନ୍ତୁ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexLock(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen ){
    ଫେରନ୍ତୁ pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  ଫେରନ୍ତୁ SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexUnlock(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lock){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen ){
    ଫେରନ୍ତୁ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  ଫେରନ୍ତୁ SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexCheckReservedLock(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen ){
    ଫେରନ୍ତୁ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  ଫେରନ୍ତୁ SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexFileControl(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  ଯଦି( !gMultiplex.isInitialized ) ଫେରନ୍ତୁ SQLITE_MISUSE;
  ବାଛ( op ){
    ସ୍ଥିତି MULTIPLEX_CTRL_ENABLE:
      ଯଦି( pArg ) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା bEnabled = *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)pArg;
        pGroup->bEnabled = (unsigned ଅକ୍ଷର)bEnabled;
        rc = SQLITE_OK;
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      ଯଦି( pArg ) {
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା szChunk = *(unsigned*)pArg;
        ଯଦି( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }ଅନ୍ୟଥା{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_SIZE-1));
          szChunk &= ~(MAX_PAGE_SIZE-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_FCNTL_SIZE_HINT:
    ସ୍ଥିତି SQLITE_FCNTL_CHUNK_SIZE:
      /* no-op these */
      rc = SQLITE_OK;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_FCNTL_PRAGMA: {
      ଅକ୍ଷର **aFcntl = (ଅକ୍ଷର**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      ଯଦି( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (ଶୂନ୍ୟ)multiplexFileSize(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        ଯଦି( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          ଯଦି( aFcntl[2] && aFcntl[2][0] ){
            ଯଦି( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }ଅନ୍ୟଥା
            ଯଦି( sqlite3_stricmp(aFcntl[2], "off")==0
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
          ଭାଙ୍ଗ;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        ଯଦି( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          ଭାଙ୍ଗ;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        ଯଦି( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          ଭାଙ୍ଗ;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        ଯଦି( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
          ପାଇଁ(ii=0; ii<pGroup->nReal; ii++){
            ଯଦି( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          ଭାଙ୍ଗ;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, ଖାଲି, 0);
      ଯଦି( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        ଯଦି( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(ଅକ୍ଷର**)pArg = sqlite3_mprintf("multiplex/%z", *(ଅକ୍ଷର**)pArg);
        }
      }
      ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    ଫେରନ୍ତୁ pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  ଫେରନ୍ତୁ DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen ){
    ଫେରନ୍ତୁ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  ଫେରନ୍ତୁ 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRegion,                    /* Region to retrieve */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szRegion,                   /* Size of regions */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bExtend,                    /* True to extend file if necessary */
  ଶୂନ୍ୟ ଅସ୍ଥିର **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen ){
    ଫେରନ୍ତୁ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  ଫେରନ୍ତୁ SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst,                  /* First lock to acquire or release */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,                     /* Number of locks to acquire or release */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen ){
    ଫେରନ୍ତୁ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  ଫେରନ୍ତୁ SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ସ୍ଥିର ଶୂନ୍ୟ multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiplexShmUnmap(sqlite3_file *pConn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, ଖାଲି, 0);
  ଯଦି( pSubOpen ){
    ଫେରନ୍ତୁ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  ଫେରନ୍ତୁ SQLITE_OK;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_multiplex_initialize(ଅଚଳ ଅକ୍ଷର *zOrigVfsName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା makeDefault){
  sqlite3_vfs *pOrigVfs;
  ଯଦି( gMultiplex.isInitialized ) ଫେରନ୍ତୁ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  ଯଦି( pOrigVfs==0 ) ଫେରନ୍ତୁ SQLITE_ERROR;
  ପ୍ରତିଜ୍ଞା( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += ଆକାର(multiplexConn);
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

  sqlite3_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))multiplexFuncInit);

  ଫେରନ୍ତୁ SQLITE_OK;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_multiplex_shutdown(ପୂର୍ଣ୍ଣସଂଖ୍ୟା eForce){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଯଦି( gMultiplex.isInitialized==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, ଆକାର(gMultiplex));
  ଫେରନ୍ତୁ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_multiplex_initialize(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *zName;              /* Name of new multiplex VFS */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା makeDefault;                /* True to make the new VFS the default */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  ଯଦି( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_multiplex_shutdown(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  ଯଦି( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  ଯଦି( (objc!=1 && objc!=2) ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "?-force?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Return code from file_control() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ମୂଲ୍ୟ = 0;
  ଶୂନ୍ୟ *pArg = 0;

  ସଂରଚନା SubCommand {
    ଅଚଳ ଅକ୍ଷର *zName;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  ଯଦି( objc!=5 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", ଖାଲି);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }ଅନ୍ୟଥା{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, ଆକାର(aSub[0]), "sub-command", 0, &idx
  );
  ଯଦି( rc!=TCL_OK ) ଫେରନ୍ତୁ rc;

  ବାଛ( aSub[idx].argtype ){
    ସ୍ଥିତି 1:
      ଯଦି( Tcl_GetIntFromObj(interp, objv[4], &i_ମୂଲ୍ୟ) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      pArg = (ଶୂନ୍ୟ *)&i_ମୂଲ୍ୟ;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 4, objv, "SUB-COMMAND");
      ଫେରନ୍ତୁ TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);
  ଫେରନ୍ତୁ (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitemultiplex_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; i<ଆକାର(aCmd)/ଆକାର(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ଫେରନ୍ତୁ TCL_OK;
}
#endif
