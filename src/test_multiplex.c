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
#समावेश "sqlite3.h"
#समावेश <string.h>
#समावेश <assert.h>
#समावेश <stdlib.h>
#समावेश "test_multiplex.h"

#अगर_अघोषित SQLITE_CORE
  #घोषणा SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#पूर्ण_अगर
#समावेश "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#घोषणा UNUSED_PARAMETER(x) (व्योम)(x)
#घोषणा MAX_PAGE_आकार       0x10000
#घोषणा DEFAULT_SECTOR_आकार 0x1000

/* Maximum chunk number */
#घोषणा MX_CHUNK_संख्या 299

/* First chunk for rollback journal files */
#घोषणा SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET 400
#घोषणा SQLITE_MULTIPLEX_WAL_8_3_OFFSET 700


/************************ Shim Definitions ******************************/

#अगर_अघोषित SQLITE_MULTIPLEX_VFS_NAME
# define SQLITE_MULTIPLEX_VFS_NAME "multiplex"
#पूर्ण_अगर

/* This is the limit on the chunk size.  It may be changed by calling
** the xFileControl() interface.  It will be rounded up to a
** multiple of MAX_PAGE_SIZE.  We default it here to 2GiB less 64KiB.
*/
#अगर_अघोषित SQLITE_MULTIPLEX_CHUNK_आकार
# define SQLITE_MULTIPLEX_CHUNK_आकार 2147418112
#पूर्ण_अगर

/* This used to be the default limit on number of chunks, but
** it is no longer enforced. There is currently no limit to the
** number of chunks.
**
** May be changed by calling the xFileControl() interface.
*/
#अगर_अघोषित SQLITE_MULTIPLEX_MAX_CHUNKS
# define SQLITE_MULTIPLEX_MAX_CHUNKS 12
#पूर्ण_अगर

/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
प्रकार काष्ठा multiplexGroup multiplexGroup;
प्रकार काष्ठा multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
काष्ठा multiplexGroup {
  काष्ठा multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    अक्षर *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  पूर्णांक nReal;                       /* Number of chunks */
  अक्षर *zName;                     /* Base filename of this group */
  पूर्णांक nName;                       /* Length of base filename */
  पूर्णांक flags;                       /* Flags used for original opening */
  अचिन्हित पूर्णांक szChunk;            /* Chunk size used for this group */
  अचिन्हित अक्षर bEnabled;          /* TRUE to use Multiplex VFS for this file */
  अचिन्हित अक्षर bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
काष्ठा multiplexConn {
  sqlite3_file base;              /* Base class - must be first */
  multiplexGroup *pGroup;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
अटल काष्ठा {
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
  पूर्णांक isInitialized;
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
अटल पूर्णांक multiplexStrlen30(स्थिर अक्षर *z){
  स्थिर अक्षर *z2 = z;
  अगर( z==0 ) वापस 0;
  जबतक( *z2 ){ z2++; }
  वापस 0x3fffffff & (पूर्णांक)(z2 - z);
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
अटल व्योम multiplexFilename(
  स्थिर अक्षर *zBase,              /* Filename for chunk 0 */
  पूर्णांक nBase,                      /* Size of zBase in bytes (without \0) */
  पूर्णांक flags,                      /* Flags used to open file */
  पूर्णांक iChunk,                     /* Chunk to generate filename for */
  अक्षर *zOut                      /* Buffer to write generated name to */
){
  पूर्णांक n = nBase;
  स_नकल(zOut, zBase, n+1);
  अगर( iChunk!=0 && iChunk<=MX_CHUNK_संख्या ){
#अगर_घोषित SQLITE_ENABLE_8_3_NAMES
    पूर्णांक i;
    क्रम(i=n-1; i>0 && i>=n-4 && zOut[i]!='.'; i--){}
    अगर( i>=n-4 ) n = i+1;
    अगर( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET;
    }अन्यथा अगर( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_OFFSET;
    }
#पूर्ण_अगर
    sqlite3_snprintf(4,&zOut[n],"%03d",iChunk);
    n += 3;
  }

  निश्चित( zOut[n]=='\0' );
  zOut[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
अटल पूर्णांक multiplexSubFilename(multiplexGroup *pGroup, पूर्णांक iChunk){
  अगर( iChunk>=pGroup->nReal ){
    काष्ठा multiplexReal *p;
    p = sqlite3_realloc64(pGroup->aReal, (iChunk+1)*माप(*p));
    अगर( p==0 ){
      वापस SQLITE_NOMEM;
    }
    स_रखो(&p[pGroup->nReal], 0, माप(p[0])*(iChunk+1-pGroup->nReal));
    pGroup->aReal = p;
    pGroup->nReal = iChunk+1;
  }
  अगर( pGroup->zName && pGroup->aReal[iChunk].z==0 ){
    अक्षर *z;
    पूर्णांक n = pGroup->nName;
    z = sqlite3_malloc64( n+5 );
    अगर( z==0 ){
      वापस SQLITE_NOMEM;
    }
    multiplexFilename(pGroup->zName, pGroup->nName, pGroup->flags, iChunk, z);
    pGroup->aReal[iChunk].z = (अक्षर*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    अगर( pGroup->aReal[iChunk].z==0 ) वापस SQLITE_NOMEM;
  }
  वापस SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
अटल sqlite3_file *multiplexSubOpen(
  multiplexGroup *pGroup,    /* The multiplexor group */
  पूर्णांक iChunk,                /* Which chunk to open.  0==original file */
  पूर्णांक *rc,                   /* Result code in and out */
  पूर्णांक *pOutFlags,            /* Output flags */
  पूर्णांक createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#अगर_घोषित SQLITE_ENABLE_8_3_NAMES
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  अगर( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET ){
    sqlite3_log(SQLITE_FULL, "multiplexed chunk overflow: %s", pGroup->zName);
    *rc = SQLITE_FULL;
    वापस 0;
  }
#पूर्ण_अगर

  *rc = multiplexSubFilename(pGroup, iChunk);
  अगर( (*rc)==SQLITE_OK && (pSubOpen = pGroup->aReal[iChunk].p)==0 ){
    पूर्णांक flags, bExists;
    flags = pGroup->flags;
    अगर( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }अन्यथा अगर( iChunk==0 ){
      /* Fall through */
    }अन्यथा अगर( pGroup->aReal[iChunk].z==0 ){
      वापस 0;
    }अन्यथा{
      *rc = pOrigVfs->xAccess(pOrigVfs, pGroup->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     अगर( *rc || !bExists ){
        अगर( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      pGroup->aReal[iChunk].z);
        }
        वापस 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    अगर( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      वापस 0;
    }
    pGroup->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, pGroup->aReal[iChunk].z, pSubOpen,
                          flags, pOutFlags);
    अगर( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  pGroup->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      pGroup->aReal[iChunk].p = 0;
      वापस 0;
    }
  }
  वापस pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
अटल sqlite3_int64 multiplex_Sub_आकार(
  multiplexGroup *pGroup,    /* The multiplexor group */
  पूर्णांक iChunk,                /* Which chunk to open.  0==original file */
  पूर्णांक *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  अगर( *rc ) वापस 0;
  pSub = multiplexSubOpen(pGroup, iChunk, rc, शून्य, 0);
  अगर( pSub==0 ) वापस 0;
  *rc = pSub->pMethods->x_File_आकार(pSub, &sz);
  वापस sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
अटल व्योम multiplexControlFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  पूर्णांक op = 0;
  पूर्णांक iVal;

  अगर( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    /* extract params */
    op = sqlite3_मूल्य_int(argv[0]);
    iVal = sqlite3_मूल्य_int(argv[1]);
    /* map function op to file_control op */
    चयन( op ){
      हाल 1:
        op = MULTIPLEX_CTRL_ENABLE;
        अवरोध;
      हाल 2:
        op = MULTIPLEX_CTRL_SET_CHUNK_आकार;
        अवरोध;
      हाल 3:
        op = MULTIPLEX_CTRL_SET_MAX_CHUNKS;
        अवरोध;
      शेष:
        rc = SQLITE_NOTFOUND;
        अवरोध;
    }
  }
  अगर( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_परिणाम_error_code(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
अटल पूर्णांक multiplexFuncInit(
  sqlite3 *db,
  अक्षर **pz_Err_संदेश,
  स्थिर sqlite3_api_routines *pApi
){
  पूर्णांक rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  वापस rc;
}

/*
** Close a single sub-file in the connection group.
*/
अटल व्योम multiplexSubClose(
  multiplexGroup *pGroup,
  पूर्णांक iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = pGroup->aReal[iChunk].p;
  अगर( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    अगर( pOrigVfs && pGroup->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
    }
    sqlite3_free(pGroup->aReal[iChunk].p);
  }
  sqlite3_free_filename(pGroup->aReal[iChunk].z);
  स_रखो(&pGroup->aReal[iChunk], 0, माप(pGroup->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
अटल व्योम multiplexFreeComponents(multiplexGroup *pGroup){
  पूर्णांक i;
  क्रम(i=0; i<pGroup->nReal; i++){ multiplexSubClose(pGroup, i, 0); }
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
अटल पूर्णांक multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  स्थिर अक्षर *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  पूर्णांक flags,                 /* Flags to control the opening */
  पूर्णांक *pOutFlags             /* Flags showing results of opening */
){
  पूर्णांक rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplexGroup *pGroup = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  पूर्णांक nName = 0;
  पूर्णांक sz = 0;
  अक्षर *zToFree = 0;

  UNUSED_PARAMETER(pVfs);
  स_रखो(pConn, 0, pVfs->szOsFile);
  निश्चित( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  अगर( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = माप(multiplexGroup)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    pGroup = sqlite3_malloc64( sz );
    अगर( pGroup==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  अगर( rc==SQLITE_OK ){
    स्थिर अक्षर *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    स_रखो(pGroup, 0, sz);
    pMultiplexOpen->pGroup = pGroup;
    pGroup->bEnabled = (अचिन्हित अक्षर)-1;
    pGroup->bTruncate = (अचिन्हित अक्षर)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    pGroup->szChunk = (पूर्णांक)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_आकार);
    pGroup->szChunk = (pGroup->szChunk+0xffff)&~0xffff;
    अगर( zName ){
      अक्षर *p = (अक्षर *)&pGroup[1];
      pGroup->zName = p;
      स_नकल(pGroup->zName, zName, nName+1);
      pGroup->nName = nName;
    }
    अगर( pGroup->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#अगर_अघोषित SQLITE_OMIT_WSD
      बाह्य पूर्णांक sqlite3PendingByte;
#अन्यथा
      पूर्णांक sqlite3PendingByte = 0x40000000;
#पूर्ण_अगर
      जबतक( (sqlite3PendingByte % pGroup->szChunk)>=(pGroup->szChunk-65536) ){
        pGroup->szChunk += 65536;
      }
    }
    pGroup->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(pGroup, 1);
    अगर( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, pOutFlags, 0);
      अगर( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    अगर( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->x_File_आकार(pSubOpen, &sz64);
      अगर( rc==SQLITE_OK && zName ){
        पूर्णांक bExists;
        अगर( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          pGroup->bEnabled = 0;
        }अन्यथा
        अगर( sz64==0 ){
          अगर( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            पूर्णांक iChunk = 1;
            करो {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  pGroup->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              अगर( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, pGroup->aReal[iChunk].z, 0);
                अगर( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(pGroup, ++iChunk);
                }
              }
            }जबतक( rc==SQLITE_OK && bExists );
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
          bExists = multiplex_Sub_आकार(pGroup, 1, &rc)>0;
          अगर( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=pGroup->szChunk ){
            pGroup->szChunk = (पूर्णांक)sz64;
          }अन्यथा अगर( rc==SQLITE_OK && !bExists && sz64>pGroup->szChunk ){
            pGroup->bEnabled = 0;
          }
        }
      }
    }

    अगर( rc==SQLITE_OK ){
      अगर( pSubOpen->pMethods->iVersion==1 ){
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
  वापस rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
अटल पूर्णांक multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  स्थिर अक्षर *zName,         /* Name of file to delete */
  पूर्णांक syncDir
){
  पूर्णांक rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  अगर( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    पूर्णांक nName = (पूर्णांक)म_माप(zName);
    अक्षर *z;
    z = sqlite3_malloc64(nName + 5);
    अगर( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }अन्यथा{
      पूर्णांक iChunk = 0;
      पूर्णांक bExists;
      करो{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }जबतक( rc==SQLITE_OK && bExists );
      जबतक( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      अगर( rc==SQLITE_OK ){
        iChunk = 0;
        करो{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }जबतक( rc==SQLITE_OK && bExists );
        जबतक( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  वापस rc;
}

अटल पूर्णांक multiplexAccess(sqlite3_vfs *a, स्थिर अक्षर *b, पूर्णांक c, पूर्णांक *d){
  वापस gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
अटल पूर्णांक multiplexFullPathname(sqlite3_vfs *a, स्थिर अक्षर *b, पूर्णांक c, अक्षर *d){
  वापस gMultiplex.pOrigVfs->xFullPathname(gMultiplex.pOrigVfs, b, c, d);
}
अटल व्योम *multiplexDlOpen(sqlite3_vfs *a, स्थिर अक्षर *b){
  वापस gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
अटल व्योम multiplexDlError(sqlite3_vfs *a, पूर्णांक b, अक्षर *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
अटल व्योम (*multiplexDlSym(sqlite3_vfs *a, व्योम *b, स्थिर अक्षर *c))(व्योम){
  वापस gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
अटल व्योम multiplexDlClose(sqlite3_vfs *a, व्योम *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
अटल पूर्णांक multiplexRandomness(sqlite3_vfs *a, पूर्णांक b, अक्षर *c){
  वापस gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
अटल पूर्णांक multiplexSleep(sqlite3_vfs *a, पूर्णांक b){
  वापस gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
अटल पूर्णांक multiplexCurrentTime(sqlite3_vfs *a, द्विगुन *b){
  वापस gMultiplex.pOrigVfs->xCurrentTime(gMultiplex.pOrigVfs, b);
}
अटल पूर्णांक multiplexGetLastError(sqlite3_vfs *a, पूर्णांक b, अक्षर *c){
  अगर( gMultiplex.pOrigVfs->xGetLastError ){
    वापस gMultiplex.pOrigVfs->xGetLastError(gMultiplex.pOrigVfs, b, c);
  }अन्यथा{
    वापस 0;
  }
}
अटल पूर्णांक multiplexCurrentTimeInt64(sqlite3_vfs *a, sqlite3_int64 *b){
  वापस gMultiplex.pOrigVfs->xCurrentTimeInt64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
अटल पूर्णांक multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णांक rc = SQLITE_OK;
  multiplexFreeComponents(pGroup);
  sqlite3_free(pGroup);
  वापस rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
अटल पूर्णांक multiplexRead(
  sqlite3_file *pConn,
  व्योम *pBuf,
  पूर्णांक iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णांक rc = SQLITE_OK;
  अगर( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, शून्य, 0);
    अगर( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }अन्यथा{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }अन्यथा{
    जबतक( iAmt > 0 ){
      पूर्णांक i = (पूर्णांक)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(pGroup, i, &rc, शून्य, 1);
      अगर( pSubOpen ){
        पूर्णांक extra = ((पूर्णांक)(iOfst % pGroup->szChunk) + iAmt) - pGroup->szChunk;
        अगर( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % pGroup->szChunk);
        अगर( rc!=SQLITE_OK ) अवरोध;
        pBuf = (अक्षर *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }अन्यथा{
        rc = SQLITE_IOERR_READ;
        अवरोध;
      }
    }
  }

  वापस rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
अटल पूर्णांक multiplexWrite(
  sqlite3_file *pConn,
  स्थिर व्योम *pBuf,
  पूर्णांक iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णांक rc = SQLITE_OK;
  अगर( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, शून्य, 0);
    अगर( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }अन्यथा{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }अन्यथा{
    जबतक( rc==SQLITE_OK && iAmt>0 ){
      पूर्णांक i = (पूर्णांक)(iOfst / pGroup->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, i, &rc, शून्य, 1);
      अगर( pSubOpen ){
        पूर्णांक extra = ((पूर्णांक)(iOfst % pGroup->szChunk) + iAmt) -
                    pGroup->szChunk;
        अगर( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % pGroup->szChunk);
        pBuf = (अक्षर *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  वापस rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
अटल पूर्णांक multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 आकार){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णांक rc = SQLITE_OK;
  अगर( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, शून्य, 0);
    अगर( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }अन्यथा{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, आकार);
    }
  }अन्यथा{
    पूर्णांक i;
    पूर्णांक iBaseGroup = (पूर्णांक)(आकार / pGroup->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    क्रम(i = pGroup->nReal-1; i>iBaseGroup && rc==SQLITE_OK; i--){
      अगर( pGroup->bTruncate ){
        multiplexSubClose(pGroup, i, pOrigVfs);
      }अन्यथा{
        pSubOpen = multiplexSubOpen(pGroup, i, &rc, 0, 0);
        अगर( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    अगर( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(pGroup, iBaseGroup, &rc, 0, 0);
      अगर( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, आकार % pGroup->szChunk);
      }
    }
    अगर( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  वापस rc;
}

/* Pass xSync requests through to the original VFS without change
*/
अटल पूर्णांक multiplexSync(sqlite3_file *pConn, पूर्णांक flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक i;
  क्रम(i=0; i<pGroup->nReal; i++){
    sqlite3_file *pSubOpen = pGroup->aReal[i].p;
    अगर( pSubOpen ){
      पूर्णांक rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      अगर( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  वापस rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
अटल पूर्णांक multiplex_File_आकार(sqlite3_file *pConn, sqlite3_int64 *p_आकार){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णांक rc = SQLITE_OK;
  पूर्णांक i;
  अगर( !pGroup->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(pGroup, 0, &rc, शून्य, 0);
    अगर( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }अन्यथा{
      rc = pSubOpen->pMethods->x_File_आकार(pSubOpen, p_आकार);
    }
  }अन्यथा{
    *p_आकार = 0;
    क्रम(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplex_Sub_आकार(pGroup, i, &rc);
      अगर( sz==0 ) अवरोध;
      *p_आकार = i*(sqlite3_int64)pGroup->szChunk + sz;
    }
  }
  वापस rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
अटल पूर्णांक multiplexLock(sqlite3_file *pConn, पूर्णांक lock){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen ){
    वापस pSubOpen->pMethods->xLock(pSubOpen, lock);
  }
  वापस SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
अटल पूर्णांक multiplexUnlock(sqlite3_file *pConn, पूर्णांक lock){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen ){
    वापस pSubOpen->pMethods->xUnlock(pSubOpen, lock);
  }
  वापस SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
अटल पूर्णांक multiplexCheckReservedLock(sqlite3_file *pConn, पूर्णांक *pResOut){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen ){
    वापस pSubOpen->pMethods->xCheckReservedLock(pSubOpen, pResOut);
  }
  वापस SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
अटल पूर्णांक multiplexFileControl(sqlite3_file *pConn, पूर्णांक op, व्योम *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplexGroup *pGroup = p->pGroup;
  पूर्णांक rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  अगर( !gMultiplex.isInitialized ) वापस SQLITE_MISUSE;
  चयन( op ){
    हाल MULTIPLEX_CTRL_ENABLE:
      अगर( pArg ) {
        पूर्णांक bEnabled = *(पूर्णांक *)pArg;
        pGroup->bEnabled = (अचिन्हित अक्षर)bEnabled;
        rc = SQLITE_OK;
      }
      अवरोध;
    हाल MULTIPLEX_CTRL_SET_CHUNK_आकार:
      अगर( pArg ) {
        अचिन्हित पूर्णांक szChunk = *(अचिन्हित*)pArg;
        अगर( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }अन्यथा{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_PAGE_आकार-1));
          szChunk &= ~(MAX_PAGE_आकार-1);
          pGroup->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      अवरोध;
    हाल MULTIPLEX_CTRL_SET_MAX_CHUNKS:
      rc = SQLITE_OK;
      अवरोध;
    हाल SQLITE_FCNTL_आकार_HINT:
    हाल SQLITE_FCNTL_CHUNK_आकार:
      /* no-op these */
      rc = SQLITE_OK;
      अवरोध;
    हाल SQLITE_FCNTL_PRAGMA: {
      अक्षर **aFcntl = (अक्षर**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      अगर( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (व्योम)multiplex_File_आकार(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        अगर( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          अगर( aFcntl[2] && aFcntl[2][0] ){
            अगर( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              pGroup->bTruncate = 1;
            }अन्यथा
            अगर( sqlite3_stricmp(aFcntl[2], "off")==0
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
          अवरोध;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        अगर( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", pGroup->bEnabled!=0);
          rc = SQLITE_OK;
          अवरोध;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        अगर( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && pGroup->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", pGroup->szChunk);
          rc = SQLITE_OK;
          अवरोध;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        अगर( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          पूर्णांक n = 0;
          पूर्णांक ii;
          क्रम(ii=0; ii<pGroup->nReal; ii++){
            अगर( pGroup->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          अवरोध;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    शेष:
      pSubOpen = multiplexSubOpen(pGroup, 0, &rc, शून्य, 0);
      अगर( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        अगर( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(अक्षर**)pArg = sqlite3_mprintf("multiplex/%z", *(अक्षर**)pArg);
        }
      }
      अवरोध;
  }
  वापस rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
अटल पूर्णांक multiplex_Sector_आकार(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen && pSubOpen->pMethods->x_Sector_आकार ){
    वापस pSubOpen->pMethods->x_Sector_आकार(pSubOpen);
  }
  वापस DEFAULT_SECTOR_आकार;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
अटल पूर्णांक multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen ){
    वापस pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  वापस 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
अटल पूर्णांक multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  पूर्णांक iRegion,                    /* Region to retrieve */
  पूर्णांक szRegion,                   /* Size of regions */
  पूर्णांक bExtend,                    /* True to extend file if necessary */
  व्योम अस्थिर **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen ){
    वापस pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  वापस SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
अटल पूर्णांक multiplexShmLock(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  पूर्णांक ofst,                  /* First lock to acquire or release */
  पूर्णांक n,                     /* Number of locks to acquire or release */
  पूर्णांक flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen ){
    वापस pSubOpen->pMethods->xShmLock(pSubOpen, ofst, n, flags);
  }
  वापस SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
अटल व्योम multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
अटल पूर्णांक multiplexShmUnmap(sqlite3_file *pConn, पूर्णांक deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  पूर्णांक rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->pGroup, 0, &rc, शून्य, 0);
  अगर( pSubOpen ){
    वापस pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  वापस SQLITE_OK;
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
पूर्णांक sqlite3_multiplex_initialize(स्थिर अक्षर *zOrigVfsName, पूर्णांक makeDefault){
  sqlite3_vfs *pOrigVfs;
  अगर( gMultiplex.isInitialized ) वापस SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  अगर( pOrigVfs==0 ) वापस SQLITE_ERROR;
  निश्चित( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += माप(multiplexConn);
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
  gMultiplex.sIoMethodsV1.x_File_आकार = multiplex_File_आकार;
  gMultiplex.sIoMethodsV1.xLock = multiplexLock;
  gMultiplex.sIoMethodsV1.xUnlock = multiplexUnlock;
  gMultiplex.sIoMethodsV1.xCheckReservedLock = multiplexCheckReservedLock;
  gMultiplex.sIoMethodsV1.xFileControl = multiplexFileControl;
  gMultiplex.sIoMethodsV1.x_Sector_आकार = multiplex_Sector_आकार;
  gMultiplex.sIoMethodsV1.xDeviceCharacteristics =
                                            multiplexDeviceCharacteristics;
  gMultiplex.sIoMethodsV2 = gMultiplex.sIoMethodsV1;
  gMultiplex.sIoMethodsV2.iVersion = 2;
  gMultiplex.sIoMethodsV2.xShmMap = multiplexShmMap;
  gMultiplex.sIoMethodsV2.xShmLock = multiplexShmLock;
  gMultiplex.sIoMethodsV2.xShmBarrier = multiplexShmBarrier;
  gMultiplex.sIoMethodsV2.xShmUnmap = multiplexShmUnmap;
  sqlite3_vfs_register(&gMultiplex.sThisVfs, makeDefault);

  sqlite3_auto_extension((व्योम(*)(व्योम))multiplexFuncInit);

  वापस SQLITE_OK;
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
पूर्णांक sqlite3_multiplex_shutdown(पूर्णांक eForce){
  पूर्णांक rc = SQLITE_OK;
  अगर( gMultiplex.isInitialized==0 ) वापस SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  स_रखो(&gMultiplex, 0, माप(gMultiplex));
  वापस rc;
}

/***************************** Test Code ***********************************/
#अगर_घोषित SQLITE_TEST
#समावेश "tclsqlite.h"
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
अटल पूर्णांक SQLITE_TCLAPI test_multiplex_initialize(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zName;              /* Name of new multiplex VFS */
  पूर्णांक makeDefault;                /* True to make the new VFS the default */
  पूर्णांक rc;                         /* Value returned by multiplex_initialize() */

  UNUSED_PARAMETER(client_तथ्य);

  /* Process arguments */
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NAME MAKEDEFAULT");
    वापस TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  अगर( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) वापस TCL_ERROR;
  अगर( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);

  वापस TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
अटल पूर्णांक SQLITE_TCLAPI test_multiplex_shutdown(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;                         /* Value returned by multiplex_shutdown() */

  UNUSED_PARAMETER(client_तथ्य);

  अगर( objc==2 && म_भेद(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  अगर( (objc!=1 && objc!=2) ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?-force?");
    वापस TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);

  वापस TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
अटल पूर्णांक SQLITE_TCLAPI test_multiplex_control(
  Client_तथ्य cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;                         /* Return code from file_control() */
  पूर्णांक idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  पूर्णांक i_मूल्य = 0;
  व्योम *pArg = 0;

  काष्ठा SubCommand {
    स्थिर अक्षर *zName;
    पूर्णांक op;
    पूर्णांक argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_SET_CHUNK_आकार,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_SET_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  अगर( objc!=5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    वापस TCL_ERROR;
  }

  अगर( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_Append_परिणाम(interp, "expected database handle, got \"", शून्य);
    Tcl_Append_परिणाम(interp, Tcl_GetString(objv[1]), "\"", शून्य);
    वापस TCL_ERROR;
  }अन्यथा{
    db = *(sqlite3 **)cmdInfo.obj_Client_तथ्य;
  }

  rc = Tcl_Get_सूचकांक_From_Obj_Struct(
      interp, objv[3], aSub, माप(aSub[0]), "sub-command", 0, &idx
  );
  अगर( rc!=TCL_OK ) वापस rc;

  चयन( aSub[idx].argtype ){
    हाल 1:
      अगर( Tcl_GetIntFromObj(interp, objv[4], &i_मूल्य) ){
        वापस TCL_ERROR;
      }
      pArg = (व्योम *)&i_मूल्य;
      अवरोध;
    शेष:
      Tcl_Wrong_अंक_Args(interp, 4, objv, "SUB-COMMAND");
      वापस TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
  वापस (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
पूर्णांक Sqlitemultiplex_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  पूर्णांक i;

  क्रम(i=0; i<माप(aCmd)/माप(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  वापस TCL_OK;
}
#पूर्ण_अगर
