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
#شامل "sqlite3.h"
#شامل <string.h>
#شامل <assert.h>
#شامل <stdlib.h>
#شامل "test_multiplex.h"

#اگر_غیر_تعریف_شدہ SQLITE_CORE
  #تعریف_کرو SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#اگر_ختم
#شامل "sqlite3ext.h"

/*
** These should be defined to be the same as the values in
** sqliteInt.h.  They are defined separately here so that
** the multiplex VFS shim can be built as a loadable
** module.
*/
#تعریف_کرو غیر_مستعمل_PARAMETER(x) (خالی)(x)
#تعریف_کرو MAX_صفحہ_سائز       0x10000
#تعریف_کرو DEFAULT_SECTOR_سائز 0x1000

/* Maximum chunk number */
#تعریف_کرو MX_CHUNK_نمبر 299

/* First chunk for rollback journal files */
#تعریف_کرو SQLITE_MULTIPLEX_JOURNAL_8_3_انحراف 400
#تعریف_کرو SQLITE_MULTIPLEX_WAL_8_3_انحراف 700


/************************ Shim Definitions ******************************/

#اگر_غیر_تعریف_شدہ SQLITE_MULTIPLEX_VFS_NAME
# define SQLITE_MULTIPLEX_VFS_NAME "multiplex"
#اگر_ختم

/* This is the limit on the chunk size.  It may be changed by calling
** the xFileControl() interface.  It will be rounded up to a
** multiple of MAX_PAGE_SIZE.  We default it here to 2GiB less 64KiB.
*/
#اگر_غیر_تعریف_شدہ SQLITE_MULTIPLEX_CHUNK_سائز
# define SQLITE_MULTIPLEX_CHUNK_سائز 2147418112
#اگر_ختم

/* This used to be the default limit on number of chunks, but
** it is no longer enforced. There is currently no limit to the
** number of chunks.
**
** May be changed by calling the xFileControl() interface.
*/
#اگر_غیر_تعریف_شدہ SQLITE_MULTIPLEX_MAX_CHUNKS
# define SQLITE_MULTIPLEX_MAX_CHUNKS 12
#اگر_ختم

/************************ Object Definitions ******************************/

/* Forward declaration of all object types */
قسمتعریف ساخت multiplex_گروہ multiplex_گروہ;
قسمتعریف ساخت multiplexConn multiplexConn;

/*
** A "multiplex group" is a collection of files that collectively
** makeup a single SQLite DB file.  This allows the size of the DB
** to exceed the limits imposed by the file system.
**
** There is an instance of the following object for each defined multiplex
** group.
*/
ساخت multiplex_گروہ {
  ساخت multiplexReal {           /* For each chunk */
    sqlite3_file *p;                  /* Handle for the chunk */
    حرف *z;                          /* Name of this chunk */
  } *aReal;                        /* list of all chunks */
  صحیح nReal;                       /* Number of chunks */
  حرف *zName;                     /* Base filename of this group */
  صحیح nName;                       /* Length of base filename */
  صحیح flags;                       /* Flags used for original opening */
  غیرعلامتی صحیح szChunk;            /* Chunk size used for this group */
  غیرعلامتی حرف bEnabled;          /* TRUE to use Multiplex VFS for this file */
  غیرعلامتی حرف bTruncate;         /* TRUE to enable truncation of databases */
};

/*
** An instance of the following object represents each open connection
** to a file that is multiplex'ed.  This object is a
** subclass of sqlite3_file.  The sqlite3_file object for the underlying
** VFS is appended to this structure.
*/
ساخت multiplexConn {
  sqlite3_file بنیاد;              /* Base class - must be first */
  multiplex_گروہ *p_گروہ;         /* The underlying group of files */
};

/************************* Global Variables **********************************/
/*
** All global variables used by this file are containing within the following
** gMultiplex structure.
*/
ساکن ساخت {
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
  صحیح isInitialized;
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
ساکن صحیح multiplexStrlen30(ثابت حرف *z){
  ثابت حرف *z2 = z;
  اگر( z==0 ) واپس 0;
  جب( *z2 ){ z2++; }
  واپس 0x3fffffff & (صحیح)(z2 - z);
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
ساکن خالی multiplexFilename(
  ثابت حرف *z_بنیاد,              /* Filename for chunk 0 */
  صحیح n_بنیاد,                      /* Size of zBase in bytes (without \0) */
  صحیح flags,                      /* Flags used to open file */
  صحیح iChunk,                     /* Chunk to generate filename for */
  حرف *z_برآمد                      /* Buffer to write generated name to */
){
  صحیح n = n_بنیاد;
  حافظہ_نقل(z_برآمد, z_بنیاد, n+1);
  اگر( iChunk!=0 && iChunk<=MX_CHUNK_نمبر ){
#اگر_تعریف_شدہ SQLITE_ENABLE_8_3_نام
    صحیح i;
    برائے(i=n-1; i>0 && i>=n-4 && z_برآمد[i]!='.'; i--){}
    اگر( i>=n-4 ) n = i+1;
    اگر( flags & SQLITE_OPEN_MAIN_JOURNAL ){
      /* The extensions on overflow files for main databases are 001, 002,
      ** 003 and so forth.  To avoid name collisions, add 400 to the
      ** extensions of journal files so that they are 401, 402, 403, ....
      */
      iChunk += SQLITE_MULTIPLEX_JOURNAL_8_3_انحراف;
    }ورنہ اگر( flags & SQLITE_OPEN_WAL ){
      /* To avoid name collisions, add 700 to the
      ** extensions of WAL files so that they are 701, 702, 703, ....
      */
      iChunk += SQLITE_MULTIPLEX_WAL_8_3_انحراف;
    }
#اگر_ختم
    sqlite3_snprintf(4,&z_برآمد[n],"%03d",iChunk);
    n += 3;
  }

  أكد( z_برآمد[n]=='\0' );
  z_برآمد[n+1] = '\0';
}

/* Compute the filename for the iChunk-th chunk
*/
ساکن صحیح multiplexSubFilename(multiplex_گروہ *p_گروہ, صحیح iChunk){
  اگر( iChunk>=p_گروہ->nReal ){
    ساخت multiplexReal *p;
    p = sqlite3_realloc64(p_گروہ->aReal, (iChunk+1)*حجم(*p));
    اگر( p==0 ){
      واپس SQLITE_NOMEM;
    }
    memset(&p[p_گروہ->nReal], 0, حجم(p[0])*(iChunk+1-p_گروہ->nReal));
    p_گروہ->aReal = p;
    p_گروہ->nReal = iChunk+1;
  }
  اگر( p_گروہ->zName && p_گروہ->aReal[iChunk].z==0 ){
    حرف *z;
    صحیح n = p_گروہ->nName;
    z = sqlite3_malloc64( n+5 );
    اگر( z==0 ){
      واپس SQLITE_NOMEM;
    }
    multiplexFilename(p_گروہ->zName, p_گروہ->nName, p_گروہ->flags, iChunk, z);
    p_گروہ->aReal[iChunk].z = (حرف*)sqlite3_create_filename(z,"","",0,0);
    sqlite3_free(z);
    اگر( p_گروہ->aReal[iChunk].z==0 ) واپس SQLITE_NOMEM;
  }
  واپس SQLITE_OK;
}

/* Translate an sqlite3_file* that is really a multiplexGroup* into
** the sqlite3_file* for the underlying original VFS.
**
** For chunk 0, the pGroup->flags determines whether or not a new file
** is created if it does not already exist.  For chunks 1 and higher, the
** file is created only if createFlag is 1.
*/
ساکن sqlite3_file *multiplexSubOpen(
  multiplex_گروہ *p_گروہ,    /* The multiplexor group */
  صحیح iChunk,                /* Which chunk to open.  0==original file */
  صحیح *rc,                   /* Result code in and out */
  صحیح *p_برآمد_Flags,            /* Output flags */
  صحیح createFlag             /* True to create if iChunk>0 */
){
  sqlite3_file *pSubOpen = 0;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;        /* Real VFS */

#اگر_تعریف_شدہ SQLITE_ENABLE_8_3_نام
  /* If JOURNAL_8_3_OFFSET is set to (say) 400, then any overflow files are
  ** part of a database journal are named db.401, db.402, and so on. A
  ** database may therefore not grow to larger than 400 chunks. Attempting
  ** to open chunk 401 indicates the database is full. */
  اگر( iChunk>=SQLITE_MULTIPLEX_JOURNAL_8_3_انحراف ){
    sqlite3_log(SQLITE_بھرا, "multiplexed chunk overflow: %s", p_گروہ->zName);
    *rc = SQLITE_بھرا;
    واپس 0;
  }
#اگر_ختم

  *rc = multiplexSubFilename(p_گروہ, iChunk);
  اگر( (*rc)==SQLITE_OK && (pSubOpen = p_گروہ->aReal[iChunk].p)==0 ){
    صحیح flags, bExists;
    flags = p_گروہ->flags;
    اگر( createFlag ){
      flags |= SQLITE_OPEN_CREATE;
    }ورنہ اگر( iChunk==0 ){
      /* Fall through */
    }ورنہ اگر( p_گروہ->aReal[iChunk].z==0 ){
      واپس 0;
    }ورنہ{
      *rc = pOrigVfs->xAccess(pOrigVfs, p_گروہ->aReal[iChunk].z,
                              SQLITE_ACCESS_EXISTS, &bExists);
     اگر( *rc || !bExists ){
        اگر( *rc ){
          sqlite3_log(*rc, "multiplexor.xAccess failure on %s",
                      p_گروہ->aReal[iChunk].z);
        }
        واپس 0;
      }
      flags &= ~SQLITE_OPEN_CREATE;
    }
    pSubOpen = sqlite3_malloc64( pOrigVfs->szOsFile );
    اگر( pSubOpen==0 ){
      *rc = SQLITE_IOERR_NOMEM;
      واپس 0;
    }
    p_گروہ->aReal[iChunk].p = pSubOpen;
    *rc = pOrigVfs->xOpen(pOrigVfs, p_گروہ->aReal[iChunk].z, pSubOpen,
                          flags, p_برآمد_Flags);
    اگر( (*rc)!=SQLITE_OK ){
      sqlite3_log(*rc, "multiplexor.xOpen failure on %s",
                  p_گروہ->aReal[iChunk].z);
      sqlite3_free(pSubOpen);
      p_گروہ->aReal[iChunk].p = 0;
      واپس 0;
    }
  }
  واپس pSubOpen;
}

/*
** Return the size, in bytes, of chunk number iChunk.  If that chunk
** does not exist, then return 0.  This function does not distinguish between
** non-existent files and zero-length files.
*/
ساکن sqlite3_int64 multiplex_Sub_سائز(
  multiplex_گروہ *p_گروہ,    /* The multiplexor group */
  صحیح iChunk,                /* Which chunk to open.  0==original file */
  صحیح *rc                    /* Result code in and out */
){
  sqlite3_file *pSub;
  sqlite3_int64 sz = 0;

  اگر( *rc ) واپس 0;
  pSub = multiplexSubOpen(p_گروہ, iChunk, rc, تہی, 0);
  اگر( pSub==0 ) واپس 0;
  *rc = pSub->pMethods->x_File_سائز(pSub, &sz);
  واپس sz;
}  

/*
** This is the implementation of the multiplex_control() SQL function.
*/
ساکن خالی multiplexControlFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح rc = SQLITE_OK;
  sqlite3 *db = sqlite3_context_db_handle(context);
  صحیح op = 0;
  صحیح iVal;

  اگر( !db || argc!=2 ){
    rc = SQLITE_ERROR;
  }ورنہ{
    /* extract params */
    op = sqlite3_قدر_int(argv[0]);
    iVal = sqlite3_قدر_int(argv[1]);
    /* map function op to file_control op */
    انتخاب( op ){
      حالت 1:
        op = MULTIPLEX_CTRL_ENABLE;
        توڑو;
      حالت 2:
        op = MULTIPLEX_CTRL_سیٹ_CHUNK_سائز;
        توڑو;
      حالت 3:
        op = MULTIPLEX_CTRL_سیٹ_MAX_CHUNKS;
        توڑو;
      طےشدہ:
        rc = SQLITE_NOTFOUND;
        توڑو;
    }
  }
  اگر( rc==SQLITE_OK ){
    rc = sqlite3_file_control(db, 0, op, &iVal);
  }
  sqlite3_نتیجہ_error_رمز(context, rc);
}

/*
** This is the entry point to register the auto-extension for the
** multiplex_control() function.
*/
ساکن صحیح multiplexFuncInit(
  sqlite3 *db,
  حرف **pz_Err_پیغام,
  ثابت sqlite3_api_routines *pApi
){
  صحیح rc;
  rc = sqlite3_create_function(db, "multiplex_control", 2, SQLITE_ANY,
      0, multiplexControlFunc, 0, 0);
  واپس rc;
}

/*
** Close a single sub-file in the connection group.
*/
ساکن خالی multiplexSubClose(
  multiplex_گروہ *p_گروہ,
  صحیح iChunk,
  sqlite3_vfs *pOrigVfs
){
  sqlite3_file *pSubOpen = p_گروہ->aReal[iChunk].p;
  اگر( pSubOpen ){
    pSubOpen->pMethods->xClose(pSubOpen);
    اگر( pOrigVfs && p_گروہ->aReal[iChunk].z ){
      pOrigVfs->xDelete(pOrigVfs, p_گروہ->aReal[iChunk].z, 0);
    }
    sqlite3_free(p_گروہ->aReal[iChunk].p);
  }
  sqlite3_free_filename(p_گروہ->aReal[iChunk].z);
  memset(&p_گروہ->aReal[iChunk], 0, حجم(p_گروہ->aReal[iChunk]));
}

/*
** Deallocate memory held by a multiplexGroup
*/
ساکن خالی multiplexFreeComponents(multiplex_گروہ *p_گروہ){
  صحیح i;
  برائے(i=0; i<p_گروہ->nReal; i++){ multiplexSubClose(p_گروہ, i, 0); }
  sqlite3_free(p_گروہ->aReal);
  p_گروہ->aReal = 0;
  p_گروہ->nReal = 0;
}


/************************* VFS Method Wrappers *****************************/

/*
** This is the xOpen method used for the "multiplex" VFS.
**
** Most of the work is done by the underlying original VFS.  This method
** simply links the new file into the appropriate multiplex group if it is a
** file that needs to be tracked.
*/
ساکن صحیح multiplexOpen(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ثابت حرف *zName,         /* Name of file to be opened */
  sqlite3_file *pConn,       /* Fill in this file descriptor */
  صحیح flags,                 /* Flags to control the opening */
  صحیح *p_برآمد_Flags             /* Flags showing results of opening */
){
  صحیح rc = SQLITE_OK;                  /* Result code */
  multiplexConn *pMultiplexOpen;       /* The new multiplex file descriptor */
  multiplex_گروہ *p_گروہ = 0;          /* Corresponding multiplexGroup object */
  sqlite3_file *pSubOpen = 0;                    /* Real file descriptor */
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  صحیح nName = 0;
  صحیح sz = 0;
  حرف *z_مقصد_Free = 0;

  غیر_مستعمل_PARAMETER(pVfs);
  memset(pConn, 0, pVfs->szOsFile);
  أكد( zName || (flags & SQLITE_OPEN_DELETEONCLOSE) );

  /* We need to create a group structure and manage
  ** access to this group of files.
  */
  pMultiplexOpen = (multiplexConn*)pConn;

  اگر( rc==SQLITE_OK ){
    /* allocate space for group */
    nName = zName ? multiplexStrlen30(zName) : 0;
    sz = حجم(multiplex_گروہ)                             /* multiplexGroup */
       + nName + 1;                                         /* zName */
    p_گروہ = sqlite3_malloc64( sz );
    اگر( p_گروہ==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  اگر( rc==SQLITE_OK ){
    ثابت حرف *zUri = (flags & SQLITE_OPEN_URI) ? zName : 0;
    /* assign pointers to extra space allocated */
    memset(p_گروہ, 0, sz);
    pMultiplexOpen->p_گروہ = p_گروہ;
    p_گروہ->bEnabled = (غیرعلامتی حرف)-1;
    p_گروہ->bTruncate = (غیرعلامتی حرف)sqlite3_uri_boolean(zUri, "truncate",
                                   (flags & SQLITE_OPEN_MAIN_DB)==0);
    p_گروہ->szChunk = (صحیح)sqlite3_uri_int64(zUri, "chunksize",
                                        SQLITE_MULTIPLEX_CHUNK_سائز);
    p_گروہ->szChunk = (p_گروہ->szChunk+0xffff)&~0xffff;
    اگر( zName ){
      حرف *p = (حرف *)&p_گروہ[1];
      p_گروہ->zName = p;
      حافظہ_نقل(p_گروہ->zName, zName, nName+1);
      p_گروہ->nName = nName;
    }
    اگر( p_گروہ->bEnabled ){
      /* Make sure that the chunksize is such that the pending byte does not
      ** falls at the end of a chunk.  A region of up to 64K following
      ** the pending byte is never written, so if the pending byte occurs
      ** near the end of a chunk, that chunk will be too small. */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WSD
      خارجی صحیح sqlite3PendingByte;
#ورنہ
      صحیح sqlite3PendingByte = 0x40000000;
#اگر_ختم
      جب( (sqlite3PendingByte % p_گروہ->szChunk)>=(p_گروہ->szChunk-65536) ){
        p_گروہ->szChunk += 65536;
      }
    }
    p_گروہ->flags = (flags & ~SQLITE_OPEN_URI);
    rc = multiplexSubFilename(p_گروہ, 1);
    اگر( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(p_گروہ, 0, &rc, p_برآمد_Flags, 0);
      اگر( pSubOpen==0 && rc==SQLITE_OK ) rc = SQLITE_CANTOPEN;
    }
    اگر( rc==SQLITE_OK ){
      sqlite3_int64 sz64;

      rc = pSubOpen->pMethods->x_File_سائز(pSubOpen, &sz64);
      اگر( rc==SQLITE_OK && zName ){
        صحیح bExists;
        اگر( flags & SQLITE_OPEN_SUPER_JOURNAL ){
          p_گروہ->bEnabled = 0;
        }ورنہ
        اگر( sz64==0 ){
          اگر( flags & SQLITE_OPEN_MAIN_JOURNAL ){
            /* If opening a main journal file and the first chunk is zero
            ** bytes in size, delete any subsequent chunks from the
            ** file-system. */
            صحیح iChunk = 1;
            کرو {
              rc = pOrigVfs->xAccess(pOrigVfs,
                  p_گروہ->aReal[iChunk].z, SQLITE_ACCESS_EXISTS, &bExists
              );
              اگر( rc==SQLITE_OK && bExists ){
                rc = pOrigVfs->xDelete(pOrigVfs, p_گروہ->aReal[iChunk].z, 0);
                اگر( rc==SQLITE_OK ){
                  rc = multiplexSubFilename(p_گروہ, ++iChunk);
                }
              }
            }جب( rc==SQLITE_OK && bExists );
          }
        }ورنہ{
          /* If the first overflow file exists and if the size of the main file
          ** is different from the chunk size, that means the chunk size is set
          ** set incorrectly.  So fix it.
          **
          ** Or, if the first overflow file does not exist and the main file is
          ** larger than the chunk size, that means the chunk size is too small.
          ** But we have no way of determining the intended chunk size, so
          ** just disable the multiplexor all together.
          */
          rc = pOrigVfs->xAccess(pOrigVfs, p_گروہ->aReal[1].z,
              SQLITE_ACCESS_EXISTS, &bExists);
          bExists = multiplex_Sub_سائز(p_گروہ, 1, &rc)>0;
          اگر( rc==SQLITE_OK && bExists && sz64==(sz64&0xffff0000) && sz64>0
              && sz64!=p_گروہ->szChunk ){
            p_گروہ->szChunk = (صحیح)sz64;
          }ورنہ اگر( rc==SQLITE_OK && !bExists && sz64>p_گروہ->szChunk ){
            p_گروہ->bEnabled = 0;
          }
        }
      }
    }

    اگر( rc==SQLITE_OK ){
      اگر( pSubOpen->pMethods->i_نسخہ==1 ){
        pConn->pMethods = &gMultiplex.sIoMethodsV1;
      }ورنہ{
        pConn->pMethods = &gMultiplex.sIoMethodsV2;
      }
    }ورنہ{
      multiplexFreeComponents(p_گروہ);
      sqlite3_free(p_گروہ);
    }
  }
  sqlite3_free(z_مقصد_Free);
  واپس rc;
}

/*
** This is the xDelete method used for the "multiplex" VFS.
** It attempts to delete the filename specified.
*/
ساکن صحیح multiplexDelete(
  sqlite3_vfs *pVfs,         /* The multiplex VFS */
  ثابت حرف *zName,         /* Name of file to delete */
  صحیح syncDir
){
  صحیح rc;
  sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
  rc = pOrigVfs->xDelete(pOrigVfs, zName, syncDir);
  اگر( rc==SQLITE_OK ){
    /* If the main chunk was deleted successfully, also delete any subsequent
    ** chunks - starting with the last (highest numbered).
    */
    صحیح nName = (صحیح)لڑی_لمبائی(zName);
    حرف *z;
    z = sqlite3_malloc64(nName + 5);
    اگر( z==0 ){
      rc = SQLITE_IOERR_NOMEM;
    }ورنہ{
      صحیح iChunk = 0;
      صحیح bExists;
      کرو{
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, ++iChunk, z);
        rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
      }جب( rc==SQLITE_OK && bExists );
      جب( rc==SQLITE_OK && iChunk>1 ){
        multiplexFilename(zName, nName, SQLITE_OPEN_MAIN_JOURNAL, --iChunk, z);
        rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
      }
      اگر( rc==SQLITE_OK ){
        iChunk = 0;
        کرو{
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, ++iChunk, z);
          rc = pOrigVfs->xAccess(pOrigVfs, z, SQLITE_ACCESS_EXISTS, &bExists);
        }جب( rc==SQLITE_OK && bExists );
        جب( rc==SQLITE_OK && iChunk>1 ){
          multiplexFilename(zName, nName, SQLITE_OPEN_WAL, --iChunk, z);
          rc = pOrigVfs->xDelete(pOrigVfs, z, syncDir);
        }
      }
    }
    sqlite3_free(z);
  }
  واپس rc;
}

ساکن صحیح multiplexAccess(sqlite3_vfs *a, ثابت حرف *b, صحیح c, صحیح *d){
  واپس gMultiplex.pOrigVfs->xAccess(gMultiplex.pOrigVfs, b, c, d);
}
ساکن صحیح multiplex_بھرا_Pathname(sqlite3_vfs *a, ثابت حرف *b, صحیح c, حرف *d){
  واپس gMultiplex.pOrigVfs->x_بھرا_Pathname(gMultiplex.pOrigVfs, b, c, d);
}
ساکن خالی *multiplexDlOpen(sqlite3_vfs *a, ثابت حرف *b){
  واپس gMultiplex.pOrigVfs->xDlOpen(gMultiplex.pOrigVfs, b);
}
ساکن خالی multiplexDlError(sqlite3_vfs *a, صحیح b, حرف *c){
  gMultiplex.pOrigVfs->xDlError(gMultiplex.pOrigVfs, b, c);
}
ساکن خالی (*multiplexDlSym(sqlite3_vfs *a, خالی *b, ثابت حرف *c))(خالی){
  واپس gMultiplex.pOrigVfs->xDlSym(gMultiplex.pOrigVfs, b, c);
}
ساکن خالی multiplexDlClose(sqlite3_vfs *a, خالی *b){
  gMultiplex.pOrigVfs->xDlClose(gMultiplex.pOrigVfs, b);
}
ساکن صحیح multiplexRandomness(sqlite3_vfs *a, صحیح b, حرف *c){
  واپس gMultiplex.pOrigVfs->xRandomness(gMultiplex.pOrigVfs, b, c);
}
ساکن صحیح multiplexSleep(sqlite3_vfs *a, صحیح b){
  واپس gMultiplex.pOrigVfs->xSleep(gMultiplex.pOrigVfs, b);
}
ساکن صحیح multiplex_Current_وقت(sqlite3_vfs *a, دوہرا *b){
  واپس gMultiplex.pOrigVfs->x_Current_وقت(gMultiplex.pOrigVfs, b);
}
ساکن صحیح multiplex_Get_آخری_Error(sqlite3_vfs *a, صحیح b, حرف *c){
  اگر( gMultiplex.pOrigVfs->x_Get_آخری_Error ){
    واپس gMultiplex.pOrigVfs->x_Get_آخری_Error(gMultiplex.pOrigVfs, b, c);
  }ورنہ{
    واپس 0;
  }
}
ساکن صحیح multiplex_Current_وقت_Int64(sqlite3_vfs *a, sqlite3_int64 *b){
  واپس gMultiplex.pOrigVfs->x_Current_وقت_Int64(gMultiplex.pOrigVfs, b);
}

/************************ I/O Method Wrappers *******************************/

/* xClose requests get passed through to the original VFS.
** We loop over all open chunk handles and close them.
** The group structure for this file is unlinked from
** our list of groups and freed.
*/
ساکن صحیح multiplexClose(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_گروہ *p_گروہ = p->p_گروہ;
  صحیح rc = SQLITE_OK;
  multiplexFreeComponents(p_گروہ);
  sqlite3_free(p_گروہ);
  واپس rc;
}

/* Pass xRead requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up reads across chunk boundaries.
*/
ساکن صحیح multiplexRead(
  sqlite3_file *pConn,
  خالی *pBuf,
  صحیح iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_گروہ *p_گروہ = p->p_گروہ;
  صحیح rc = SQLITE_OK;
  اگر( !p_گروہ->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_گروہ, 0, &rc, تہی, 0);
    اگر( pSubOpen==0 ){
      rc = SQLITE_IOERR_READ;
    }ورنہ{
      rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ورنہ{
    جب( iAmt > 0 ){
      صحیح i = (صحیح)(iOfst / p_گروہ->szChunk);
      sqlite3_file *pSubOpen;
      pSubOpen = multiplexSubOpen(p_گروہ, i, &rc, تہی, 1);
      اگر( pSubOpen ){
        صحیح extra = ((صحیح)(iOfst % p_گروہ->szChunk) + iAmt) - p_گروہ->szChunk;
        اگر( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xRead(pSubOpen, pBuf, iAmt,
                                       iOfst % p_گروہ->szChunk);
        اگر( rc!=SQLITE_OK ) توڑو;
        pBuf = (حرف *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }ورنہ{
        rc = SQLITE_IOERR_READ;
        توڑو;
      }
    }
  }

  واپس rc;
}

/* Pass xWrite requests thru to the original VFS after
** determining the correct chunk to operate on.
** Break up writes across chunk boundaries.
*/
ساکن صحیح multiplexWrite(
  sqlite3_file *pConn,
  ثابت خالی *pBuf,
  صحیح iAmt,
  sqlite3_int64 iOfst
){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_گروہ *p_گروہ = p->p_گروہ;
  صحیح rc = SQLITE_OK;
  اگر( !p_گروہ->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_گروہ, 0, &rc, تہی, 0);
    اگر( pSubOpen==0 ){
      rc = SQLITE_IOERR_WRITE;
    }ورنہ{
      rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt, iOfst);
    }
  }ورنہ{
    جب( rc==SQLITE_OK && iAmt>0 ){
      صحیح i = (صحیح)(iOfst / p_گروہ->szChunk);
      sqlite3_file *pSubOpen = multiplexSubOpen(p_گروہ, i, &rc, تہی, 1);
      اگر( pSubOpen ){
        صحیح extra = ((صحیح)(iOfst % p_گروہ->szChunk) + iAmt) -
                    p_گروہ->szChunk;
        اگر( extra<0 ) extra = 0;
        iAmt -= extra;
        rc = pSubOpen->pMethods->xWrite(pSubOpen, pBuf, iAmt,
                                        iOfst % p_گروہ->szChunk);
        pBuf = (حرف *)pBuf + iAmt;
        iOfst += iAmt;
        iAmt = extra;
      }
    }
  }
  واپس rc;
}

/* Pass xTruncate requests thru to the original VFS after
** determining the correct chunk to operate on.  Delete any
** chunks above the truncate mark.
*/
ساکن صحیح multiplexTruncate(sqlite3_file *pConn, sqlite3_int64 سائز){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_گروہ *p_گروہ = p->p_گروہ;
  صحیح rc = SQLITE_OK;
  اگر( !p_گروہ->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_گروہ, 0, &rc, تہی, 0);
    اگر( pSubOpen==0 ){
      rc = SQLITE_IOERR_TRUNCATE;
    }ورنہ{
      rc = pSubOpen->pMethods->xTruncate(pSubOpen, سائز);
    }
  }ورنہ{
    صحیح i;
    صحیح i_بنیاد_گروہ = (صحیح)(سائز / p_گروہ->szChunk);
    sqlite3_file *pSubOpen;
    sqlite3_vfs *pOrigVfs = gMultiplex.pOrigVfs;   /* Real VFS */
    /* delete the chunks above the truncate limit */
    برائے(i = p_گروہ->nReal-1; i>i_بنیاد_گروہ && rc==SQLITE_OK; i--){
      اگر( p_گروہ->bTruncate ){
        multiplexSubClose(p_گروہ, i, pOrigVfs);
      }ورنہ{
        pSubOpen = multiplexSubOpen(p_گروہ, i, &rc, 0, 0);
        اگر( pSubOpen ){
          rc = pSubOpen->pMethods->xTruncate(pSubOpen, 0);
        }
      }
    }
    اگر( rc==SQLITE_OK ){
      pSubOpen = multiplexSubOpen(p_گروہ, i_بنیاد_گروہ, &rc, 0, 0);
      اگر( pSubOpen ){
        rc = pSubOpen->pMethods->xTruncate(pSubOpen, سائز % p_گروہ->szChunk);
      }
    }
    اگر( rc ) rc = SQLITE_IOERR_TRUNCATE;
  }
  واپس rc;
}

/* Pass xSync requests through to the original VFS without change
*/
ساکن صحیح multiplexSync(sqlite3_file *pConn, صحیح flags){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_گروہ *p_گروہ = p->p_گروہ;
  صحیح rc = SQLITE_OK;
  صحیح i;
  برائے(i=0; i<p_گروہ->nReal; i++){
    sqlite3_file *pSubOpen = p_گروہ->aReal[i].p;
    اگر( pSubOpen ){
      صحیح rc2 = pSubOpen->pMethods->xSync(pSubOpen, flags);
      اگر( rc2!=SQLITE_OK ) rc = rc2;
    }
  }
  واپس rc;
}

/* Pass xFileSize requests through to the original VFS.
** Aggregate the size of all the chunks before returning.
*/
ساکن صحیح multiplex_File_سائز(sqlite3_file *pConn, sqlite3_int64 *p_سائز){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_گروہ *p_گروہ = p->p_گروہ;
  صحیح rc = SQLITE_OK;
  صحیح i;
  اگر( !p_گروہ->bEnabled ){
    sqlite3_file *pSubOpen = multiplexSubOpen(p_گروہ, 0, &rc, تہی, 0);
    اگر( pSubOpen==0 ){
      rc = SQLITE_IOERR_FSTAT;
    }ورنہ{
      rc = pSubOpen->pMethods->x_File_سائز(pSubOpen, p_سائز);
    }
  }ورنہ{
    *p_سائز = 0;
    برائے(i=0; rc==SQLITE_OK; i++){
      sqlite3_int64 sz = multiplex_Sub_سائز(p_گروہ, i, &rc);
      اگر( sz==0 ) توڑو;
      *p_سائز = i*(sqlite3_int64)p_گروہ->szChunk + sz;
    }
  }
  واپس rc;
}

/* Pass xLock requests through to the original VFS unchanged.
*/
ساکن صحیح multiplex_تالا(sqlite3_file *pConn, صحیح تالا){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen ){
    واپس pSubOpen->pMethods->x_تالا(pSubOpen, تالا);
  }
  واپس SQLITE_BUSY;
}

/* Pass xUnlock requests through to the original VFS unchanged.
*/
ساکن صحیح multiplexUnlock(sqlite3_file *pConn, صحیح تالا){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen ){
    واپس pSubOpen->pMethods->xUnlock(pSubOpen, تالا);
  }
  واپس SQLITE_IOERR_UNLOCK;
}

/* Pass xCheckReservedLock requests through to the original VFS unchanged.
*/
ساکن صحیح multiplex_Check_Reserved_تالا(sqlite3_file *pConn, صحیح *p_Res_برآمد){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen ){
    واپس pSubOpen->pMethods->x_Check_Reserved_تالا(pSubOpen, p_Res_برآمد);
  }
  واپس SQLITE_IOERR_CHECKRESERVEDLOCK;
}

/* Pass xFileControl requests through to the original VFS unchanged,
** except for any MULTIPLEX_CTRL_* requests here.
*/
ساکن صحیح multiplexFileControl(sqlite3_file *pConn, صحیح op, خالی *pArg){
  multiplexConn *p = (multiplexConn*)pConn;
  multiplex_گروہ *p_گروہ = p->p_گروہ;
  صحیح rc = SQLITE_ERROR;
  sqlite3_file *pSubOpen;

  اگر( !gMultiplex.isInitialized ) واپس SQLITE_MISUSE;
  انتخاب( op ){
    حالت MULTIPLEX_CTRL_ENABLE:
      اگر( pArg ) {
        صحیح bEnabled = *(صحیح *)pArg;
        p_گروہ->bEnabled = (غیرعلامتی حرف)bEnabled;
        rc = SQLITE_OK;
      }
      توڑو;
    حالت MULTIPLEX_CTRL_سیٹ_CHUNK_سائز:
      اگر( pArg ) {
        غیرعلامتی صحیح szChunk = *(غیرعلامتی*)pArg;
        اگر( szChunk<1 ){
          rc = SQLITE_MISUSE;
        }ورنہ{
          /* Round up to nearest multiple of MAX_PAGE_SIZE. */
          szChunk = (szChunk + (MAX_صفحہ_سائز-1));
          szChunk &= ~(MAX_صفحہ_سائز-1);
          p_گروہ->szChunk = szChunk;
          rc = SQLITE_OK;
        }
      }
      توڑو;
    حالت MULTIPLEX_CTRL_سیٹ_MAX_CHUNKS:
      rc = SQLITE_OK;
      توڑو;
    حالت SQLITE_FCNTL_سائز_HINT:
    حالت SQLITE_FCNTL_CHUNK_سائز:
      /* no-op these */
      rc = SQLITE_OK;
      توڑو;
    حالت SQLITE_FCNTL_PRAGMA: {
      حرف **aFcntl = (حرف**)pArg;
      /*
      ** EVIDENCE-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
      ** file control is an array of pointers to strings (char**) in which the
      ** second element of the array is the name of the pragma and the third
      ** element is the argument to the pragma or NULL if the pragma has no
      ** argument.
      */
      اگر( aFcntl[1] && sqlite3_strnicmp(aFcntl[1],"multiplex_",10)==0 ){
        sqlite3_int64 sz = 0;
        (خالی)multiplex_File_سائز(pConn, &sz);
        /*
        ** PRAGMA multiplex_truncate=BOOLEAN;
        ** PRAGMA multiplex_truncate;
        **
        ** Turn the multiplexor truncate feature on or off.  Return either
        ** "on" or "off" to indicate the new setting.  If the BOOLEAN argument
        ** is omitted, just return the current value for the truncate setting.
        */
        اگر( sqlite3_stricmp(aFcntl[1],"multiplex_truncate")==0 ){
          اگر( aFcntl[2] && aFcntl[2][0] ){
            اگر( sqlite3_stricmp(aFcntl[2], "on")==0
             || sqlite3_stricmp(aFcntl[2], "1")==0 ){
              p_گروہ->bTruncate = 1;
            }ورنہ
            اگر( sqlite3_stricmp(aFcntl[2], "off")==0
             || sqlite3_stricmp(aFcntl[2], "0")==0 ){
              p_گروہ->bTruncate = 0;
            }
          }
          /* EVIDENCE-OF: R-27806-26076 The handler for an SQLITE_FCNTL_PRAGMA
          ** file control can optionally make the first element of the char**
          ** argument point to a string obtained from sqlite3_mprintf() or the
          ** equivalent and that string will become the result of the pragma
          ** or the error message if the pragma fails.
          */
          aFcntl[0] = sqlite3_mprintf(p_گروہ->bTruncate ? "on" : "off");
          rc = SQLITE_OK;
          توڑو;
        }
        /*
        ** PRAGMA multiplex_enabled;
        **
        ** Return 0 or 1 depending on whether the multiplexor is enabled or
        ** disabled, respectively.
        */
        اگر( sqlite3_stricmp(aFcntl[1],"multiplex_enabled")==0 ){
          aFcntl[0] = sqlite3_mprintf("%d", p_گروہ->bEnabled!=0);
          rc = SQLITE_OK;
          توڑو;
        }
        /*
        ** PRAGMA multiplex_chunksize;
        **
        ** Return the chunksize for the multiplexor, or no-op if the
        ** multiplexor is not active.
        */
        اگر( sqlite3_stricmp(aFcntl[1],"multiplex_chunksize")==0
         && p_گروہ->bEnabled
        ){
          aFcntl[0] = sqlite3_mprintf("%u", p_گروہ->szChunk);
          rc = SQLITE_OK;
          توڑو;
        }
        /*
        ** PRAGMA multiplex_filecount;
        **
        ** Return the number of disk files currently in use by the
        ** multiplexor.  This should be the total database size size
        ** divided by the chunksize and rounded up.
        */
        اگر( sqlite3_stricmp(aFcntl[1],"multiplex_filecount")==0 ){
          صحیح n = 0;
          صحیح ii;
          برائے(ii=0; ii<p_گروہ->nReal; ii++){
            اگر( p_گروہ->aReal[ii].p!=0 ) n++;
          }
          aFcntl[0] = sqlite3_mprintf("%d", n);
          rc = SQLITE_OK;
          توڑو;
        }
      }
      /* If the multiplexor does not handle the pragma, pass it through
      ** into the default case. */
    }
    طےشدہ:
      pSubOpen = multiplexSubOpen(p_گروہ, 0, &rc, تہی, 0);
      اگر( pSubOpen ){
        rc = pSubOpen->pMethods->xFileControl(pSubOpen, op, pArg);
        اگر( op==SQLITE_FCNTL_VFSNAME && rc==SQLITE_OK ){
         *(حرف**)pArg = sqlite3_mprintf("multiplex/%z", *(حرف**)pArg);
        }
      }
      توڑو;
  }
  واپس rc;
}

/* Pass xSectorSize requests through to the original VFS unchanged.
*/
ساکن صحیح multiplex_Sector_سائز(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen && pSubOpen->pMethods->x_Sector_سائز ){
    واپس pSubOpen->pMethods->x_Sector_سائز(pSubOpen);
  }
  واپس DEFAULT_SECTOR_سائز;
}

/* Pass xDeviceCharacteristics requests through to the original VFS unchanged.
*/
ساکن صحیح multiplexDeviceCharacteristics(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen ){
    واپس pSubOpen->pMethods->xDeviceCharacteristics(pSubOpen);
  }
  واپس 0;
}

/* Pass xShmMap requests through to the original VFS unchanged.
*/
ساکن صحیح multiplexShmMap(
  sqlite3_file *pConn,            /* Handle open on database file */
  صحیح iRegion,                    /* Region to retrieve */
  صحیح szRegion,                   /* Size of regions */
  صحیح bExtend,                    /* True to extend file if necessary */
  خالی متلون **pp              /* OUT: Mapped memory */
){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen ){
    واپس pSubOpen->pMethods->xShmMap(pSubOpen, iRegion, szRegion, bExtend,pp);
  }
  واپس SQLITE_IOERR;
}

/* Pass xShmLock requests through to the original VFS unchanged.
*/
ساکن صحیح multiplex_Shm_تالا(
  sqlite3_file *pConn,       /* Database file holding the shared memory */
  صحیح ofst,                  /* First lock to acquire or release */
  صحیح n,                     /* Number of locks to acquire or release */
  صحیح flags                  /* What to do with the lock */
){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen ){
    واپس pSubOpen->pMethods->x_Shm_تالا(pSubOpen, ofst, n, flags);
  }
  واپس SQLITE_BUSY;
}

/* Pass xShmBarrier requests through to the original VFS unchanged.
*/
ساکن خالی multiplexShmBarrier(sqlite3_file *pConn){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen ){
    pSubOpen->pMethods->xShmBarrier(pSubOpen);
  }
}

/* Pass xShmUnmap requests through to the original VFS unchanged.
*/
ساکن صحیح multiplexShmUnmap(sqlite3_file *pConn, صحیح deleteFlag){
  multiplexConn *p = (multiplexConn*)pConn;
  صحیح rc;
  sqlite3_file *pSubOpen = multiplexSubOpen(p->p_گروہ, 0, &rc, تہی, 0);
  اگر( pSubOpen ){
    واپس pSubOpen->pMethods->xShmUnmap(pSubOpen, deleteFlag);
  }
  واپس SQLITE_OK;
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
صحیح sqlite3_multiplex_initialize(ثابت حرف *zOrigVfsName, صحیح makeDefault){
  sqlite3_vfs *pOrigVfs;
  اگر( gMultiplex.isInitialized ) واپس SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_ڈھونڈو(zOrigVfsName);
  اگر( pOrigVfs==0 ) واپس SQLITE_ERROR;
  أكد( pOrigVfs!=&gMultiplex.sThisVfs );
  gMultiplex.isInitialized = 1;
  gMultiplex.pOrigVfs = pOrigVfs;
  gMultiplex.sThisVfs = *pOrigVfs;
  gMultiplex.sThisVfs.szOsFile += حجم(multiplexConn);
  gMultiplex.sThisVfs.zName = SQLITE_MULTIPLEX_VFS_NAME;
  gMultiplex.sThisVfs.xOpen = multiplexOpen;
  gMultiplex.sThisVfs.xDelete = multiplexDelete;
  gMultiplex.sThisVfs.xAccess = multiplexAccess;
  gMultiplex.sThisVfs.x_بھرا_Pathname = multiplex_بھرا_Pathname;
  gMultiplex.sThisVfs.xDlOpen = multiplexDlOpen;
  gMultiplex.sThisVfs.xDlError = multiplexDlError;
  gMultiplex.sThisVfs.xDlSym = multiplexDlSym;
  gMultiplex.sThisVfs.xDlClose = multiplexDlClose;
  gMultiplex.sThisVfs.xRandomness = multiplexRandomness;
  gMultiplex.sThisVfs.xSleep = multiplexSleep;
  gMultiplex.sThisVfs.x_Current_وقت = multiplex_Current_وقت;
  gMultiplex.sThisVfs.x_Get_آخری_Error = multiplex_Get_آخری_Error;
  gMultiplex.sThisVfs.x_Current_وقت_Int64 = multiplex_Current_وقت_Int64;

  gMultiplex.sIoMethodsV1.i_نسخہ = 1;
  gMultiplex.sIoMethodsV1.xClose = multiplexClose;
  gMultiplex.sIoMethodsV1.xRead = multiplexRead;
  gMultiplex.sIoMethodsV1.xWrite = multiplexWrite;
  gMultiplex.sIoMethodsV1.xTruncate = multiplexTruncate;
  gMultiplex.sIoMethodsV1.xSync = multiplexSync;
  gMultiplex.sIoMethodsV1.x_File_سائز = multiplex_File_سائز;
  gMultiplex.sIoMethodsV1.x_تالا = multiplex_تالا;
  gMultiplex.sIoMethodsV1.xUnlock = multiplexUnlock;
  gMultiplex.sIoMethodsV1.x_Check_Reserved_تالا = multiplex_Check_Reserved_تالا;
  gMultiplex.sIoMethodsV1.xFileControl = multiplexFileControl;
  gMultiplex.sIoMethodsV1.x_Sector_سائز = multiplex_Sector_سائز;
  gMultiplex.sIoMethodsV1.xDeviceCharacteristics =
                                            multiplexDeviceCharacteristics;
  gMultiplex.sIoMethodsV2 = gMultiplex.sIoMethodsV1;
  gMultiplex.sIoMethodsV2.i_نسخہ = 2;
  gMultiplex.sIoMethodsV2.xShmMap = multiplexShmMap;
  gMultiplex.sIoMethodsV2.x_Shm_تالا = multiplex_Shm_تالا;
  gMultiplex.sIoMethodsV2.xShmBarrier = multiplexShmBarrier;
  gMultiplex.sIoMethodsV2.xShmUnmap = multiplexShmUnmap;
  sqlite3_vfs_رجسٹر(&gMultiplex.sThisVfs, makeDefault);

  sqlite3_auto_extension((خالی(*)(خالی))multiplexFuncInit);

  واپس SQLITE_OK;
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
صحیح sqlite3_multiplex_shutdown(صحیح eForce){
  صحیح rc = SQLITE_OK;
  اگر( gMultiplex.isInitialized==0 ) واپس SQLITE_MISUSE;
  gMultiplex.isInitialized = 0;
  sqlite3_vfs_unregister(&gMultiplex.sThisVfs);
  memset(&gMultiplex, 0, حجم(gMultiplex));
  واپس rc;
}

/***************************** Test Code ***********************************/
#اگر_تعریف_شدہ SQLITE_TEST
#شامل "tclsqlite.h"
خارجی ثابت حرف *sqlite3ErrName(صحیح);


/*
** tclcmd: sqlite3_multiplex_initialize NAME MAKEDEFAULT
*/
ساکن صحیح SQLITE_TCLAPI test_multiplex_initialize(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zName;              /* Name of new multiplex VFS */
  صحیح makeDefault;                /* True to make the new VFS the default */
  صحیح rc;                         /* Value returned by multiplex_initialize() */

  غیر_مستعمل_PARAMETER(client_ڈیٹا);

  /* Process arguments */
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "NAME MAKEDEFAULT");
    واپس TCL_ERROR;
  }
  zName = Tcl_GetString(objv[1]);
  اگر( Tcl_GetBooleanFromObj(interp, objv[2], &makeDefault) ) واپس TCL_ERROR;
  اگر( zName[0]=='\0' ) zName = 0;

  /* Call sqlite3_multiplex_initialize() */
  rc = sqlite3_multiplex_initialize(zName, makeDefault);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);

  واپس TCL_OK;
}

/*
** tclcmd: sqlite3_multiplex_shutdown
*/
ساکن صحیح SQLITE_TCLAPI test_multiplex_shutdown(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;                         /* Value returned by multiplex_shutdown() */

  غیر_مستعمل_PARAMETER(client_ڈیٹا);

  اگر( objc==2 && strcmp(Tcl_GetString(objv[1]),"-force")!=0 ){
    objc = 3;
  }
  اگر( (objc!=1 && objc!=2) ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "?-force?");
    واپس TCL_ERROR;
  }

  /* Call sqlite3_multiplex_shutdown() */
  rc = sqlite3_multiplex_shutdown(objc==2);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);

  واپس TCL_OK;
}

/*
** Tclcmd: test_multiplex_control HANDLE DBNAME SUB-COMMAND ?INT-VALUE?
*/
ساکن صحیح SQLITE_TCLAPI test_multiplex_control(
  Client_ڈیٹا cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;                         /* Return code from file_control() */
  صحیح idx;                        /* Index in aSub[] */
  Tcl_CmdInfo cmdInfo;            /* Command info structure for HANDLE */
  sqlite3 *db;                    /* Underlying db handle for HANDLE */
  صحیح i_قدر = 0;
  خالی *pArg = 0;

  ساخت SubCommand {
    ثابت حرف *zName;
    صحیح op;
    صحیح argtype;
  } aSub[] = {
    { "enable",       MULTIPLEX_CTRL_ENABLE,           1 },
    { "chunk_size",   MULTIPLEX_CTRL_سیٹ_CHUNK_سائز,   1 },
    { "max_chunks",   MULTIPLEX_CTRL_سیٹ_MAX_CHUNKS,   1 },
    { 0, 0, 0 }
  };

  اگر( objc!=5 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "HANDLE DBNAME SUB-COMMAND INT-VALUE");
    واپس TCL_ERROR;
  }

  اگر( 0==Tcl_GetCommandInfo(interp, Tcl_GetString(objv[1]), &cmdInfo) ){
    Tcl_Append_نتیجہ(interp, "expected database handle, got \"", تہی);
    Tcl_Append_نتیجہ(interp, Tcl_GetString(objv[1]), "\"", تہی);
    واپس TCL_ERROR;
  }ورنہ{
    db = *(sqlite3 **)cmdInfo.obj_Client_ڈیٹا;
  }

  rc = Tcl_Get_اشاریہ_From_Obj_Struct(
      interp, objv[3], aSub, حجم(aSub[0]), "sub-command", 0, &idx
  );
  اگر( rc!=TCL_OK ) واپس rc;

  انتخاب( aSub[idx].argtype ){
    حالت 1:
      اگر( Tcl_GetIntFromObj(interp, objv[4], &i_قدر) ){
        واپس TCL_ERROR;
      }
      pArg = (خالی *)&i_قدر;
      توڑو;
    طےشدہ:
      Tcl_Wrong_عدد_دلائل(interp, 4, objv, "SUB-COMMAND");
      واپس TCL_ERROR;
  }

  rc = sqlite3_file_control(db, Tcl_GetString(objv[2]), aSub[idx].op, pArg);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
  واپس (rc==SQLITE_OK) ? TCL_OK : TCL_ERROR;
}

/*
** This routine registers the custom TCL commands defined in this
** module.  This should be the only procedure visible from outside
** of this module.
*/
صحیح Sqlitemultiplex_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aCmd[] = {
    { "sqlite3_multiplex_initialize", test_multiplex_initialize },
    { "sqlite3_multiplex_shutdown", test_multiplex_shutdown },
    { "sqlite3_multiplex_control", test_multiplex_control },
  };
  صحیح i;

  برائے(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xProc, 0, 0);
  }

  واپس TCL_OK;
}
#اگر_ختم
