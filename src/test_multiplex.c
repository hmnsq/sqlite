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
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "test_multiplex.h"

#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#अन्तर्भाव "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#define UNUSED_PARAMETER(x) (शून्य)(x)
#define MAX_PAGE_SIZE       0x10000
#define DEFAULT_SECTOR_SIZE 0x1000

/* Maximum chunk number */
#define MX_CHUNK_सङ्ख्या 299

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
प्रकारनिर्देश संरचना multiplexGroup multiplexGroup;
प्रकारनिर्देश संरचना multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
संरचना multiplexGroup {
  संरचना multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    वर्ण *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  पूर्णाङ्क nReal;                       /* Number of chunks */
  वर्ण *zName;                     /* Base filename of this group */
  पूर्णाङ्क nName;                       /* Length of base filename */
  पूर्णाङ्क flags;                       /* Flags used for original opening */
  अचिन्हित पूर्णाङ्क szChunk;            /* Chunk size used for this group */
  अचिन्हित वर्ण bEnabled;          /* TRUE to use Multiplex VFS for this file */
  अचिन्हित वर्ण bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
संरचना multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
स्थिर संरचना {
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
  पूर्णाङ्क isInitialized;
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
स्थिर पूर्णाङ्क multiplexStrlen30(अचर वर्ण *z){
  अचर वर्ण *z2 = z;
  यदि( z==0 ) फर्काऊ 0;
  जबसम्म( *z2 ){ z2++; }
  फर्काऊ 0x3fffffff & (पूर्णाङ्क)(z2 - z);
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
स्थिर शून्य multiplexFilename(
  अचर वर्ण *zBase,              /* Filename for chunk 0 */
  पूर्णाङ्क nBase,                      /* Size of zBase in bytes (without \0) */
  पूर्णाङ्क flags,                      /* Flags used to open file */
  पूर्णाङ्क iChunk,                     /* Chunk to generate filename for */
  वर्ण *zOut                      /* Buffer to write generated name to */
){
  पूर्णाङ्क n = nBase;
  स_नकल(zOut, zBase, n+1);
  यदि( iChunk!=0 && iChunk<=MX_CHUNK_सङ्ख्या ){
#ifdef SQLITE_ENABLE_8_3_NAMES
    पूर्णाङ्क i;
    लागि(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    यदि( i>=n-4 ) n = i+1;
    यदि( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }अन्यथा यदि( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_OFFSET;
    }
#endif
    sqlite3_snprintf(4,&zOut[n],"%03d",iChunk);
    n += 3;
  }

  प्रतिज्ञा( zOut[n]=='\0' );
  zOut[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
स्थिर पूर्णाङ्क multiplexSubFilename(multiplexGroup *pGroup, पूर्णाङ्क iChunk){
  यदि( iChunk>=pGroup->nReal ){
    संरचना multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*परिमाण(*p));
    यदि( p==0 ){
      फर्काऊ SQLITE_NOMEM;
    }
    स_रखो(&p[pGroup->nReal], 0, परिमाण(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  यदि( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    वर्ण *z;
    पूर्णाङ्क n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    यदि( z==0 ){
      फर्काऊ SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (वर्ण*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    यदि( pGroup->aReal[iChunk].z==0 ) फर्काऊ SQLITE_NOMEM;
  }
  फर्काऊ SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
स्थिर sqlite3_file *multiplexSubOpen(
  multiplexGroup *pGroup,    /* The multiplexor group */
  पूर्णाङ्क iChunk,                /* Which chunk to open.  0==original file */
  पूर्णाङ्क *rc,                   /* Result code in and out */
  पूर्णाङ्क *pOutFlags,            /* Output flags */
  पूर्णाङ्क createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#ifdef SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  यदि( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    फर्काऊ 0;
  }
#endif

  *rc = multiplexSubFilename(pGroup, iChunk);
  यदि( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    पूर्णाङ्क flags, bExists;
    flags = pGroup->flags;
    यदि( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }अन्यथा यदि( iChunk==0 ){
      /* Fall through */
    }अन्यथा यदि( pGroup->aReal[iChunk].z==0 ){
      फर्काऊ 0;
    }अन्यथा{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     यदि( *rc || !bExists ){
        यदि( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        फर्काऊ 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    यदि( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      फर्काऊ 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    यदि( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      फर्काऊ 0;
    }
  }
  फर्काऊ pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
स्थिर sqlite3_int64 multiplexSubSize(
  multiplexGroup *pGroup,    /* The multiplexor group */
  पूर्णाङ्क iChunk,                /* Which chunk to open.  0==original file */
  पूर्णाङ्क *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  यदि( *rc ) फर्काऊ 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, रिक्त, 0);
  यदि( pSub==0 ) फर्काऊ 0;
  *rc = pSub->pMethods->xFileSize(pSub, &sz);
  फर्काऊ sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
स्थिर शून्य multiplexControlFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मान **argv
){
  पूर्णाङ्क rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  पूर्णाङ्क op = 0;
  पूर्णाङ्क iVal;

  यदि( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    /* extract params */
    op = sqlite3_मान_int(argv[0]);
    iVal = sqlite3_मान_int(argv[1]);
    /* map function op to file_control op */
    विकल्प( op ){
      पक्ष 1:
        op = MULTIPLEX_CTRL_ENABLE;
        रोक;
      पक्ष 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_SIZE;
        रोक;
      पक्ष 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        रोक;
      स्वभाव:
        rc = SQLITE_NOTFOUND;
        रोक;
    }
  }
  यदि( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_result_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
स्थिर पूर्णाङ्क multiplexFuncInit(
  sqlite3 *db,
  वर्ण **pz_Err_सन्देश,
  अचर sqlite3_api_routines *pApi
){
  पूर्णाङ्क rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  फर्काऊ rc;
}

/*
** Close a single sub-file in the connection group.
*/
स्थिर शून्य multiplexSubClose(
  multiplexGroup *pGroup,
  पूर्णाङ्क iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  यदि( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    यदि( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  स_रखो(&pGroup->aReal[iChunk], 0, परिमाण(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
स्थिर शून्य multiplexFreeComponents(multiplexGroup *pGroup){
  पूर्णाङ्क i;
  लागि(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
स्थिर पूर्णाङ्क multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  अचर वर्ण *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  पूर्णाङ्क flags,                 /* Flags to control the opening */
  पूर्णाङ्क *pOutFlags             /* Flags showing results of opening */
){
  पूर्णाङ्क rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  पूर्णाङ्क nName = 0;
  पूर्णाङ्क sz = 0;
  वर्ण *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  स_रखो(pConn, 0, pVfs->szOsFile);
  प्रतिज्ञा( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  यदि( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = परिमाण(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    यदि( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  यदि( rc==SQLITE_OK ){
    अचर वर्ण *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    स_रखो(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (अचिन्हित वर्ण)-1;
    pGroup->bTruncate = (अचिन्हित वर्ण)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (पूर्णाङ्क)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_SIZE);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    यदि( zName ){
      वर्ण *p = (वर्ण *)&pGroup[1];
      pGroup->zName = p;
      स_नकल(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    यदि( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#ifndef SQLITE_OMIT_WSD
      बाह्य पूर्णाङ्क sqlite3PendingByte;
#अन्यथा
      पूर्णाङ्क sqlite3PendingByte = 0x40000000;
#endif
      जबसम्म( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    यदि( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      यदि( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    यदि( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->xFileSize(pSubOpen, &sz64);
      यदि( rc==SQLITE_OK && zName ){
        पूर्णाङ्क bExists;
        यदि( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }अन्यथा
        यदि( sz64==0 ){
          यदि( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            पूर्णाङ्क iChunk = 1;
            गर {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              यदि( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                यदि( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }जबसम्म( rc==SQLITE_OK && bExists );
          }
        }अन्यथा{
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
          यदि( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (पूर्णाङ्क)sz64;
          }अन्यथा यदि( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    यदि( rc==SQLITE_OK ){
      यदि( pSubOpen->pMethods->iVersion==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }अन्यथा{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }अन्यथा{
      multiplexFreeComponents(pGroup);
      sqlite3_free(pGroup);
    }
  }
  sqlite3_free(zToFree);
  फर्काऊ rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
स्थिर पूर्णाङ्क multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  अचर वर्ण *zName,         /* Name of file to delete */
  पूर्णाङ्क syncDir
){
  पूर्णाङ्क rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  यदि( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    पूर्णाङ्क nName = (पूर्णाङ्क)म_माप(zName);
    वर्ण *z;
    z = sqlite3_malloc64(nName + 5);
    यदि( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }अन्यथा{
      पूर्णाङ्क iChunk = 0;
      पूर्णाङ्क bExists;
      गर{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }जबसम्म( rc==SQLITE_OK && bExists );
      जबसम्म( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      यदि( rc==SQLITE_OK ){
        iChunk = 0;
        गर{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }जबसम्म( rc==SQLITE_OK && bExists );
        जबसम्म( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  फर्काऊ rc;
}

स्थिर पूर्णाङ्क multiplexAccess(sqlite3_vfs *a, अचर वर्ण *b, पूर्णाङ्क c, पूर्णाङ्क *d){
  फर्काऊ gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
स्थिर पूर्णाङ्क multiplexFullPathname(sqlite3_vfs *a, अचर वर्ण *b, पूर्णाङ्क c, वर्ण *d){
  फर्काऊ gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
स्थिर शून्य *multiplexDlOpen(sqlite3_vfs *a, अचर वर्ण *b){
  फर्काऊ gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
स्थिर शून्य multiplexDlError(sqlite3_vfs *a, पूर्णाङ्क b, वर्ण *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
स्थिर शून्य (*multiplexDlSym(sqlite3_vfs *a, शून्य *b, अचर वर्ण *c))(शून्य){
  फर्काऊ gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
स्थिर शून्य multiplexDlClose(sqlite3_vfs *a, शून्य *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
स्थिर पूर्णाङ्क multiplexRandomness(sqlite3_vfs *a, पूर्णाङ्क b, वर्ण *c){
  फर्काऊ gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
स्थिर पूर्णाङ्क multiplexSleep(sqlite3_vfs *a, पूर्णाङ्क b){
  फर्काऊ gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
स्थिर पूर्णाङ्क multiplexCurrentTime(sqlite3_vfs *a, द्विगुण *b){
  फर्काऊ gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
स्थिर पूर्णाङ्क multiplexGetLastError(sqlite3_vfs *a, पूर्णाङ्क b, वर्ण *c){
  यदि( gMultiplex.pOrigVfs->xGetLastError ){
    फर्काऊ gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }अन्यथा{
    फर्काऊ 0;
  }
}
स्थिर पूर्णाङ्क multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  फर्काऊ gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
स्थिर पूर्णाङ्क multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णाङ्क rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  फर्काऊ rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
स्थिर पूर्णाङ्क multiplexRead(
  sqlite3_file *pConn,
  शून्य *pBuf,
  पूर्णाङ्क iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णाङ्क rc = SQLITE_OK;
  यदि( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, रिक्त, 0);
    यदि( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }अन्यथा{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }अन्यथा{
    जबसम्म( iAmt > 0 ){
      पूर्णाङ्क i = (पूर्णाङ्क)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, रिक्त, 1);
      यदि( pSubOpen ){
        पूर्णाङ्क extra = ((पूर्णाङ्क)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        यदि( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        यदि( rc!=SQLITE_OK ) रोक;
        pBuf = (वर्ण *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }अन्यथा{
        rc = SQLITE_IOERR_READ;
        रोक;
      }
    }
  }

  फर्काऊ rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
स्थिर पूर्णाङ्क multiplexWrite(
  sqlite3_file *pConn,
  अचर शून्य *pBuf,
  पूर्णाङ्क iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णाङ्क rc = SQLITE_OK;
  यदि( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, रिक्त, 0);
    यदि( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }अन्यथा{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }अन्यथा{
    जबसम्म( rc==SQLITE_OK && iAmt>0 ){
      पूर्णाङ्क i = (पूर्णाङ्क)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, रिक्त, 1);
      यदि( pSubOpen ){
        पूर्णाङ्क extra = ((पूर्णाङ्क)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        यदि( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (वर्ण *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  फर्काऊ rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
स्थिर पूर्णाङ्क multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 size){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णाङ्क rc = SQLITE_OK;
  यदि( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, रिक्त, 0);
    यदि( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }अन्यथा{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, size);
    }
  }अन्यथा{
    पूर्णाङ्क i;
    पूर्णाङ्क iBaseGroup = (पूर्णाङ्क)(size / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    लागि(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      यदि( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }अन्यथा{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        यदि( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    यदि( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      यदि( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, size % pGroup->szChunk);
      }
    }
    यदि( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  फर्काऊ rc;
}

/* Pass xSync requests through to the original VFS without change
*/
स्थिर पूर्णाङ्क multiplexSync(sqlite3_file *pConn, पूर्णाङ्क flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क i;
  लागि(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    यदि( pSubOpen ){
      पूर्णाङ्क rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      यदि( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  फर्काऊ rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
स्थिर पूर्णाङ्क multiplexFileSize(sqlite3_file *pConn, sqlite3_int64 *pSize){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णाङ्क rc = SQLITE_OK;
  पूर्णाङ्क i;
  यदि( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, रिक्त, 0);
    यदि( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }अन्यथा{
      rc = pSubOpen->pMethods->xFileSize(pSubOpen, pSize);
    }
  }अन्यथा{
    *pSize = 0;
    लागि(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplexSubSize(pGroup, i, &rc);
      यदि( sz==0 ) रोक;
      *pSize = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  फर्काऊ rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क multiplexLock(sqlite3_file *pConn, पूर्णाङ्क lock){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen ){
    फर्काऊ pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  फर्काऊ SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क multiplexUnlock(sqlite3_file *pConn, पूर्णाङ्क lock){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen ){
    फर्काऊ pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  फर्काऊ SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क multiplexCheckReservedLock(sqlite3_file *pConn, पूर्णाङ्क *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen ){
    फर्काऊ pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  फर्काऊ SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
स्थिर पूर्णाङ्क multiplexFileControl(sqlite3_file *pConn, पूर्णाङ्क op, शून्य *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णाङ्क rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  यदि( !gMultiplex.isInitialized ) फर्काऊ SQLITE_MISUSE;
  विकल्प( op ){
    पक्ष MULTIPLEX_CTRL_ENABLE:
      यदि( pArg ) {
        पूर्णाङ्क bEnabled = *(पूर्णाङ्क *)pArg;
        pGroup->bEnabled = (अचिन्हित वर्ण)bEnabled;
        rc = SQLITE_OK;
      }
      रोक;
    पक्ष MULTIPLEX_CTRL_SET_CHUNK_SIZE:
      यदि( pArg ) {
        अचिन्हित पूर्णाङ्क szChunk = *(अचिन्हित*)pArg;
        यदि( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }अन्यथा{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_SIZE-1));
          szChunk &= ~(MAX_PAGE_SIZE-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      रोक;
    पक्ष MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      रोक;
    पक्ष SQLITE_FCNTL_SIZE_HINT:
    पक्ष SQLITE_FCNTL_CHUNK_SIZE:
      /* no-op these */
      rc = SQLITE_OK;
      रोक;
    पक्ष SQLITE_FCNTL_PRAGMA: {
      वर्ण **aFcntl = (वर्ण**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      यदि( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (शून्य)multiplexFileSize(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        यदि( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          यदि( aFcntl[2] && aFcntl[2][0] ){
            यदि( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }अन्यथा
            यदि( sqlite3_stricmp(aFcntl[2], "off")==0
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
          रोक;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        यदि( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          रोक;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        यदि( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          रोक;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        यदि( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          पूर्णाङ्क n = 0;
          पूर्णाङ्क ii;
          लागि(ii=0; ii<pGroup->nReal; ii++){
            यदि( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          रोक;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    स्वभाव:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, रिक्त, 0);
      यदि( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        यदि( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(वर्ण**)pArg = sqlite3_mprintf("multiplex/%z", *(वर्ण**)pArg);
        }
      }
      रोक;
  }
  फर्काऊ rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क multiplexSectorSize(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen && pSubOpen->pMethods->xSectorSize ){
    फर्काऊ pSubOpen->pMethods->xSectorSize(pSubOpen);
  }
  फर्काऊ DEFAULT_SECTOR_SIZE;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen ){
    फर्काऊ pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  फर्काऊ 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  पूर्णाङ्क iRegion,                    /* Region to retrieve */
  पूर्णाङ्क szRegion,                   /* Size of regions */
  पूर्णाङ्क bExtend,                    /* True to extend file if necessary */
  शून्य चञ्चल **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen ){
    फर्काऊ pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  फर्काऊ SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  पूर्णाङ्क ofst,                  /* First lock to acquire or release */
  पूर्णाङ्क n,                     /* Number of locks to acquire or release */
  पूर्णाङ्क flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen ){
    फर्काऊ pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  फर्काऊ SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
स्थिर शून्य multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
स्थिर पूर्णाङ्क multiplexShmUnmap(sqlite3_file *pConn, पूर्णाङ्क deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णाङ्क rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, रिक्त, 0);
  यदि( pSubOpen ){
    फर्काऊ pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  फर्काऊ SQLITE_OK;
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
पूर्णाङ्क sqlite3_multiplex_initialize(अचर वर्ण *zOrigVfsName, पूर्णाङ्क makeDefault){
  sqlite3_vfs *pOrigVfs;
  यदि( gMultiplex.isInitialized ) फर्काऊ SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  यदि( pOrigVfs==0 ) फर्काऊ SQLITE_ERROR;
  प्रतिज्ञा( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += परिमाण(multiplexConn);
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

  sqlite3_auto_extension((शून्य(*)(शून्य))multiplexFuncInit);

  फर्काऊ SQLITE_OK;
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
पूर्णाङ्क sqlite3_multiplex_shutdown(पूर्णाङ्क eForce){
  पूर्णाङ्क rc = SQLITE_OK;
  यदि( gMultiplex.isInitialized==0 ) फर्काऊ SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  स_रखो(&gMultiplex, 0, परिमाण(gMultiplex));
  फर्काऊ rc;
}

/***************************** Test Code ***********************************/
#ifdef SQLITE_TEST
#अन्तर्भाव "tclsqlite.h"
बाह्य अचर वर्ण *sqlite3ErrName(पूर्णाङ्क);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_multiplex_initialize(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  अचर वर्ण *zName;              /* Name of new multiplex VFS */
  पूर्णाङ्क makeDefault;                /* True to make the new VFS the default */
  पूर्णाङ्क rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(clientData);

  /* Process arguments */
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    फर्काऊ TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  यदि( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) फर्काऊ TCL_ERROR;
  यदि( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);

  फर्काऊ TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_multiplex_shutdown(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(clientData);

  यदि( objc==2 && म_भेद(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  यदि( (objc!=1 && objc!=2) ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "?-force?");
    फर्काऊ TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);

  फर्काऊ TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_multiplex_control(
  ClientData cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;                         /* Return code from file_control() */
  पूर्णाङ्क idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  पूर्णाङ्क i_मान = 0;
  शून्य *pArg = 0;

  संरचना SubCommand {
    अचर वर्ण *zName;
    पूर्णाङ्क op;
    पूर्णाङ्क argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_SIZE,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  यदि( objc!=5 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    फर्काऊ TCL_ERROR;
  }

  यदि( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_AppendResult(interp, "expected database handle, got \"", रिक्त);
    Tcl_AppendResult(interp, Tcl_GetString(objv[1]), "\"", रिक्त);
    फर्काऊ TCL_ERROR;
  }अन्यथा{
    db = *(sqlite3 **)cmdInfo.objClientData;
  }

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[3], aSub, परिमाण(aSub[0]), "sub-command", 0, &idx
  );
  यदि( rc!=TCL_OK ) फर्काऊ rc;

  विकल्प( aSub[idx].argtype ){
    पक्ष 1:
      यदि( Tcl_GetIntFromObj(interp, objv[4], &i_मान) ){
        फर्काऊ TCL_ERROR;
      }
      pArg = (शून्य *)&i_मान;
      रोक;
    स्वभाव:
      Tcl_Wrong_अङ्क_Args(interp, 4, objv, "SUB-COMMAND");
      फर्काऊ TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
  फर्काऊ (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
पूर्णाङ्क Sqlitemultiplex_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  पूर्णाङ्क i;

  लागि(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  फर्काऊ TCL_OK;
}
#endif
