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
#অন্তর্ভাৱ "sqlite3.h"
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ "test_multiplex.h"

#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#অন্তর্ভাৱ "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#define UNUSED_PARAMETER(x) (শূন্য)(x)
#define MAX_PAGE_SIZE       0x10000
#define DEFAULT_SECTOR_SIZE 0x1000

/* Maximum chunk number */
#define MX_CHUNK_সংখ্যা 299

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
প্রকারনির্দেশ সংরচনা multiplexGroup multiplexGroup;
প্রকারনির্দেশ সংরচনা multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
সংরচনা multiplexGroup {
  সংরচনা multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    ৱর্ণ *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  পূর্ণাঙ্ক nReal;                       /* Number of chunks */
  ৱর্ণ *zName;                     /* Base filename of this group */
  পূর্ণাঙ্ক nName;                       /* Length of base filename */
  পূর্ণাঙ্ক flags;                       /* Flags used for original opening */
  unsigned পূর্ণাঙ্ক szChunk;            /* Chunk size used for this group */
  unsigned ৱর্ণ bEnabled;          /* TRUE to use Multiplex VFS for this file */
  unsigned ৱর্ণ bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
সংরচনা multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
স্থির সংরচনা {
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
  পূর্ণাঙ্ক isInitialized;
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
স্থির পূর্ণাঙ্ক multiplexStrlen30(ধ্রুৱ ৱর্ণ *z){
  ধ্রুৱ ৱর্ণ *z2 = z;
  যদি( z==0 ) ঘূৰাই 0;
  যেতিয়ালৈ( *z2 ){ z2++; }
  ঘূৰাই 0x3fffffff & (পূর্ণাঙ্ক)(z2 - z);
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
স্থির শূন্য multiplexFilename(
  ধ্রুৱ ৱর্ণ *zBase,              /* Filename for chunk 0 */
  পূর্ণাঙ্ক nBase,                      /* Size of zBase in bytes (without \0) */
  পূর্ণাঙ্ক flags,                      /* Flags used to open file */
  পূর্ণাঙ্ক iChunk,                     /* Chunk to generate filename for */
  ৱর্ণ *zOut                      /* Buffer to write generated name to */
){
  পূর্ণাঙ্ক n = nBase;
  memcpy(zOut, zBase, n+1);
  যদি( iChunk!=0 && iChunk<=MX_CHUNK_সংখ্যা ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    পূর্ণাঙ্ক i;
    বাবে(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    যদি( i>=n-4 ) n = i+1;
    যদি( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }অন্যথা যদি( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_OFFSET;
    }
#endif
    sqlite3_snprintf(4,&zOut[n],"%03d",iChunk);
    n += 3;
  }

  প্রতিজ্ঞা( zOut[n]=='\0' );
  zOut[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
স্থির পূর্ণাঙ্ক multiplexSubFilename(multiplexGroup *pGroup, পূর্ণাঙ্ক iChunk){
  যদি( iChunk>=pGroup->nReal ){
    সংরচনা multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*পরিমাণ(*p));
    যদি( p==0 ){
      ঘূৰাই SQLITE_NOMEM;
    }
    memset(&p[pGroup->nReal], 0, পরিমাণ(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  যদি( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    ৱর্ণ *z;
    পূর্ণাঙ্ক n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    যদি( z==0 ){
      ঘূৰাই SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (ৱর্ণ*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    যদি( pGroup->aReal[iChunk].z==0 ) ঘূৰাই SQLITE_NOMEM;
  }
  ঘূৰাই SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
স্থির sqlite3_file *multiplexSubOpen(
  multiplexGroup *pGroup,    /* The multiplexor group */
  পূর্ণাঙ্ক iChunk,                /* Which chunk to open.  0==original file */
  পূর্ণাঙ্ক *rc,                   /* Result code in and out */
  পূর্ণাঙ্ক *pOutFlags,            /* Output flags */
  পূর্ণাঙ্ক createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  যদি( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    ঘূৰাই 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  যদি( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    পূর্ণাঙ্ক flags, bExists;
    flags = pGroup->flags;
    যদি( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }অন্যথা যদি( iChunk==0 ){
      /* Fall through */
    }অন্যথা যদি( pGroup->aReal[iChunk].z==0 ){
      ঘূৰাই 0;
    }অন্যথা{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     যদি( *rc || !bExists ){
        যদি( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        ঘূৰাই 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    যদি( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      ঘূৰাই 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    যদি( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      ঘূৰাই 0;
    }
  }
  ঘূৰাই pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
স্থির sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  পূর্ণাঙ্ক iChunk,                /* Which chunk to open.  0==original file */
  পূর্ণাঙ্ক *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  যদি( *rc ) ঘূৰাই 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, রিক্ত, 0);
  যদি( pSub==0 ) ঘূৰাই 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  ঘূৰাই sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
স্থির শূন্য multiplexControlFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  পূর্ণাঙ্ক op = 0;
  পূর্ণাঙ্ক iVal;

  যদি( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    /* extract params */
    op = sqlite3_মান_int(argv[0]);
    iVal = sqlite3_মান_int(argv[1]);
    /* map function op to file_control op */
    ৱিকল্প( op ){
      পক্ষ 1:
        op = MULTIPLEX_CTRL_ENABLE;
        ভঙ্গ;
      পক্ষ 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_SIZE;
        ভঙ্গ;
      পক্ষ 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        ভঙ্গ;
      স্ৱভাৱ:
        rc = SQLITE_NOTFOUND;
        ভঙ্গ;
    }
  }
  যদি( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
স্থির পূর্ণাঙ্ক multiplexFuncInit(
  sqlite3 *db,
  ৱর্ণ **pz_Err_বাৰ্তা,
  ধ্রুৱ sqlite3_api_routines *pApi
){
  পূর্ণাঙ্ক rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  ঘূৰাই rc;
}

/*
** Close a single sub-file in the connection group.
*/
স্থির শূন্য multiplexSubClose(
  multiplexGroup *pGroup,
  পূর্ণাঙ্ক iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  যদি( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    যদি( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  memset(&pGroup->aReal[iChunk], 0, পরিমাণ(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
স্থির শূন্য multiplexFreeComponents(multiplexGroup *pGroup){
  পূর্ণাঙ্ক i;
  বাবে(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
স্থির পূর্ণাঙ্ক multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ধ্রুৱ ৱর্ণ *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  পূর্ণাঙ্ক flags,                 /* Flags to control the opening */
  পূর্ণাঙ্ক *pOutFlags             /* Flags showing results of opening */
){
  পূর্ণাঙ্ক rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  পূর্ণাঙ্ক nName = 0;
  পূর্ণাঙ্ক sz = 0;
  ৱর্ণ *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  প্রতিজ্ঞা( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  যদি( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = পরিমাণ(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    যদি( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  যদি( rc==SQLITE_OK ){
    ধ্রুৱ ৱর্ণ *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (unsigned ৱর্ণ)-1;
    pGroup->bTruncate = (unsigned ৱর্ণ)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (পূর্ণাঙ্ক)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    যদি( zName ){
      ৱর্ণ *p = (ৱর্ণ *)&pGroup[1];
      pGroup->zName = p;
      memcpy(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    যদি( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      বাহ্য পূর্ণাঙ্ক sqlite3PendingByte;
#অন্যথা
      পূর্ণাঙ্ক sqlite3PendingByte = 0x40000000;
#endif
      যেতিয়ালৈ( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    যদি( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      যদি( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    যদি( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      যদি( rc==SQLITE_OK && zName ){
        পূর্ণাঙ্ক bExists;
        যদি( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }অন্যথা
        যদি( sz64==0 ){
          যদি( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            পূর্ণাঙ্ক iChunk = 1;
            কৰা {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              যদি( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                যদি( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }যেতিয়ালৈ( rc==SQLITE_OK && bExists );
          }
        }অন্যথা{
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
          যদি( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (পূর্ণাঙ্ক)sz64;
          }অন্যথা যদি( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    যদি( rc==SQLITE_OK ){
      যদি( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }অন্যথা{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }অন্যথা{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  ঘূৰাই rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
স্থির পূর্ণাঙ্ক multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ধ্রুৱ ৱর্ণ *zName,         /* Name of file to delete */
  পূর্ণাঙ্ক syncDir
){
  পূর্ণাঙ্ক rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  যদি( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    পূর্ণাঙ্ক nName = (পূর্ণাঙ্ক)strlen(zName);
    ৱর্ণ *z;
    z = sqlite3_malloc64(nName + 5);
    যদি( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }অন্যথা{
      পূর্ণাঙ্ক iChunk = 0;
      পূর্ণাঙ্ক bExists;
      কৰা{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }যেতিয়ালৈ( rc==SQLITE_OK && bExists );
      যেতিয়ালৈ( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      যদি( rc==SQLITE_OK ){
        iChunk = 0;
        কৰা{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }যেতিয়ালৈ( rc==SQLITE_OK && bExists );
        যেতিয়ালৈ( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  ঘূৰাই rc;
}

স্থির পূর্ণাঙ্ক multiplexAccess(sqlite3_vfs *a, ধ্রুৱ ৱর্ণ *b, পূর্ণাঙ্ক c, পূর্ণাঙ্ক *d){
  ঘূৰাই gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
স্থির পূর্ণাঙ্ক multiplexFullPathname(sqlite3_vfs *a, ধ্রুৱ ৱর্ণ *b, পূর্ণাঙ্ক c, ৱর্ণ *d){
  ঘূৰাই gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
স্থির শূন্য *multiplexDlOpen(sqlite3_vfs *a, ধ্রুৱ ৱর্ণ *b){
  ঘূৰাই gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
স্থির শূন্য multiplexDlError(sqlite3_vfs *a, পূর্ণাঙ্ক b, ৱর্ণ *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
স্থির শূন্য (*multiplexDlSym(sqlite3_vfs *a, শূন্য *b, ধ্রুৱ ৱর্ণ *c))(শূন্য){
  ঘূৰাই gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
স্থির শূন্য multiplexDlClose(sqlite3_vfs *a, শূন্য *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
স্থির পূর্ণাঙ্ক multiplexRandomness(sqlite3_vfs *a, পূর্ণাঙ্ক b, ৱর্ণ *c){
  ঘূৰাই gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
স্থির পূর্ণাঙ্ক multiplexSleep(sqlite3_vfs *a, পূর্ণাঙ্ক b){
  ঘূৰাই gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
স্থির পূর্ণাঙ্ক multiplexCurrentTime(sqlite3_vfs *a, দ্ৱিগুণ *b){
  ঘূৰাই gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
স্থির পূর্ণাঙ্ক multiplexGetLastError(sqlite3_vfs *a, পূর্ণাঙ্ক b, ৱর্ণ *c){
  যদি( gMultiplex.pOrigVfs->xGetLastError ){
    ঘূৰাই gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }অন্যথা{
    ঘূৰাই 0;
  }
}
স্থির পূর্ণাঙ্ক multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  ঘূৰাই gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
স্থির পূর্ণাঙ্ক multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  ঘূৰাই rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
স্থির পূর্ণাঙ্ক multiplexRead(
  sqlite3_file *pConn,
  শূন্য *pBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  যদি( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, রিক্ত, 0);
    যদি( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }অন্যথা{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }অন্যথা{
    যেতিয়ালৈ( iAmt > 0 ){
      পূর্ণাঙ্ক i = (পূর্ণাঙ্ক)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, রিক্ত, 1);
      যদি( pSubOpen ){
        পূর্ণাঙ্ক extra = ((পূর্ণাঙ্ক)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        যদি( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        যদি( rc!=SQLITE_OK ) ভঙ্গ;
        pBuf = (ৱর্ণ *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }অন্যথা{
        rc = SQLITE_IOERR_READ;
        ভঙ্গ;
      }
    }
  }

  ঘূৰাই rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
স্থির পূর্ণাঙ্ক multiplexWrite(
  sqlite3_file *pConn,
  ধ্রুৱ শূন্য *pBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  যদি( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, রিক্ত, 0);
    যদি( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }অন্যথা{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }অন্যথা{
    যেতিয়ালৈ( rc==SQLITE_OK && iAmt>0 ){
      পূর্ণাঙ্ক i = (পূর্ণাঙ্ক)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, রিক্ত, 1);
      যদি( pSubOpen ){
        পূর্ণাঙ্ক extra = ((পূর্ণাঙ্ক)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        যদি( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (ৱর্ণ *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  ঘূৰাই rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
স্থির পূর্ণাঙ্ক multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  যদি( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, রিক্ত, 0);
    যদি( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }অন্যথা{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }অন্যথা{
    পূর্ণাঙ্ক i;
    পূর্ণাঙ্ক iBaseGroup = (পূর্ণাঙ্ক)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    বাবে(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      যদি( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }অন্যথা{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        যদি( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    যদি( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      যদি( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    যদি( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  ঘূৰাই rc;
}

/* Pass xSync requests through to the original VFS without change
*/
স্থির পূর্ণাঙ্ক multiplexSync(sqlite3_file *pConn, পূর্ণাঙ্ক flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক i;
  বাবে(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    যদি( pSubOpen ){
      পূর্ণাঙ্ক rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      যদি( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  ঘূৰাই rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
স্থির পূর্ণাঙ্ক multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক i;
  যদি( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, রিক্ত, 0);
    যদি( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }অন্যথা{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }অন্যথা{
    *pSize = 0;
    বাবে(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      যদি( sz==0 ) ভঙ্গ;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  ঘূৰাই rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক multiplexLock(sqlite3_file *pConn, পূর্ণাঙ্ক lock){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen ){
    ঘূৰাই pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  ঘূৰাই SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক multiplexUnlock(sqlite3_file *pConn, পূর্ণাঙ্ক lock){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen ){
    ঘূৰাই pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  ঘূৰাই SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক multiplexCheckReservedLock(sqlite3_file *pConn, পূর্ণাঙ্ক *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen ){
    ঘূৰাই pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  ঘূৰাই SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
স্থির পূর্ণাঙ্ক multiplexFileControl(sqlite3_file *pConn, পূর্ণাঙ্ক op, শূন্য *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  পূর্ণাঙ্ক rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  যদি( !gMultiplex.isInitialized ) ঘূৰাই SQLITE_MISUSE;
  ৱিকল্প( op ){
    পক্ষ MULTIPLEX_CTRL_ENABLE:
      যদি( pArg ) {
        পূর্ণাঙ্ক bEnabled = *(পূর্ণাঙ্ক *)pArg;
        pGroup->bEnabled = (unsigned ৱর্ণ)bEnabled;
        rc = SQLITE_OK;
      }
      ভঙ্গ;
    পক্ষ MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      যদি( pArg ) {
        unsigned পূর্ণাঙ্ক szChunk = *(unsigned*)pArg;
        যদি( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }অন্যথা{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_SIZE-1));
          szChunk &= ~(MAX_PAGE_SIZE-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      ভঙ্গ;
    পক্ষ MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      ভঙ্গ;
    পক্ষ SQLITE_FCNTL_SIZE_HINT:
    পক্ষ SQLITE_FCNTL_CHUNK_SIZE:
      /* no-op these */
      rc = SQLITE_OK;
      ভঙ্গ;
    পক্ষ SQLITE_FCNTL_PRAGMA: {
      ৱর্ণ **aFcntl = (ৱর্ণ**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      যদি( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (শূন্য)multiplexFileSize(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        যদি( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          যদি( aFcntl[2] && aFcntl[2][0] ){
            যদি( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }অন্যথা
            যদি( sqlite3_stricmp(aFcntl[2], "off")==0
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
          ভঙ্গ;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        যদি( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          ভঙ্গ;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        যদি( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          ভঙ্গ;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        যদি( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          পূর্ণাঙ্ক n = 0;
          পূর্ণাঙ্ক ii;
          বাবে(ii=0; ii<pGroup->nReal; ii++){
            যদি( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          ভঙ্গ;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    স্ৱভাৱ:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, রিক্ত, 0);
      যদি( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        যদি( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(ৱর্ণ**)pArg = sqlite3_mprintf("multiplex/%z", *(ৱর্ণ**)pArg);
        }
      }
      ভঙ্গ;
  }
  ঘূৰাই rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    ঘূৰাই pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  ঘূৰাই DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen ){
    ঘূৰাই pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  ঘূৰাই 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  পূর্ণাঙ্ক iRegion,                    /* Region to retrieve */
  পূর্ণাঙ্ক szRegion,                   /* Size of regions */
  পূর্ণাঙ্ক bExtend,                    /* True to extend file if necessary */
  শূন্য চঞ্চল **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen ){
    ঘূৰাই pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  ঘূৰাই SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  পূর্ণাঙ্ক ofst,                  /* First lock to acquire or release */
  পূর্ণাঙ্ক n,                     /* Number of locks to acquire or release */
  পূর্ণাঙ্ক flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen ){
    ঘূৰাই pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  ঘূৰাই SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
স্থির শূন্য multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
স্থির পূর্ণাঙ্ক multiplexShmUnmap(sqlite3_file *pConn, পূর্ণাঙ্ক deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  পূর্ণাঙ্ক rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, রিক্ত, 0);
  যদি( pSubOpen ){
    ঘূৰাই pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  ঘূৰাই SQLITE_OK;
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
পূর্ণাঙ্ক sqlite3_multiplex_initialize(ধ্রুৱ ৱর্ণ *zOrigVfsName, পূর্ণাঙ্ক makeDefault){
  sqlite3_vfs *pOrigVfs;
  যদি( gMultiplex.isInitialized ) ঘূৰাই SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  যদি( pOrigVfs==0 ) ঘূৰাই SQLITE_ERROR;
  প্রতিজ্ঞা( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += পরিমাণ(multiplexConn);
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

  sqlite3_auto_extension((শূন্য(*)(শূন্য))multiplexFuncInit);

  ঘূৰাই SQLITE_OK;
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
পূর্ণাঙ্ক sqlite3_multiplex_shutdown(পূর্ণাঙ্ক eForce){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  যদি( gMultiplex.isInitialized==0 ) ঘূৰাই SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, পরিমাণ(gMultiplex));
  ঘূৰাই rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#অন্তর্ভাৱ "tclsqlite.h"
বাহ্য ধ্রুৱ ৱর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_multiplex_initialize(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুৱ ৱর্ণ *zName;              /* Name of new multiplex VFS */
  পূর্ণাঙ্ক makeDefault;                /* True to make the new VFS the default */
  পূর্ণাঙ্ক rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    ঘূৰাই TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  যদি( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) ঘূৰাই TCL_ERROR;
  যদি( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_STATIC);

  ঘূৰাই TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_multiplex_shutdown(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  যদি( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  যদি( (objc!=1 && objc!=2) ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "?-force?");
    ঘূৰাই TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_STATIC);

  ঘূৰাই TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;                         /* Return code from file_control() */
  পূর্ণাঙ্ক idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  পূর্ণাঙ্ক i_মান = 0;
  শূন্য *pArg = 0;

  সংরচনা SubCommand {
    ধ্রুৱ ৱর্ণ *zName;
    পূর্ণাঙ্ক op;
    পূর্ণাঙ্ক argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  যদি( objc!=5 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    ঘূৰাই TCL_ERROR;
  }

  যদি( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", রিক্ত);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", রিক্ত);
    ঘূৰাই TCL_ERROR;
  }অন্যথা{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, পরিমাণ(aSub[0]), "sub-command", 0, &idx
  );
  যদি( rc!=TCL_OK ) ঘূৰাই rc;

  ৱিকল্প( aSub[idx].argtype ){
    পক্ষ 1:
      যদি( Tcl_GetIntFromObj(interp, objv[4], &i_মান) ){
        ঘূৰাই TCL_ERROR;
      }
      pArg = (শূন্য *)&i_মান;
      ভঙ্গ;
    স্ৱভাৱ:
      Tcl_Wrong_অঙ্ক_Args(interp, 4, objv, "SUB-COMMAND");
      ঘূৰাই TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_STATIC);
  ঘূৰাই (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
পূর্ণাঙ্ক Sqlitemultiplex_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
     ৱর্ণ *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  পূর্ণাঙ্ক i;

  বাবে(i=0; i<পরিমাণ(aCmd)/পরিমাণ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  ঘূৰাই TCL_OK;
}
#endif
