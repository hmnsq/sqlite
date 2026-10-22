/*
** 2022-09-06
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains an experimental VFS layer that operates on a
** Key/Value storage engine where both keys and values must be pure
** text.
**
** DEBUG AND TEST
**
** For testing on Unix, compile using:
**
**    make clean sqlite3d CFLAGS='-DSQLITE_OS_KV_OPTIONAL'
**
** Then start up a shell using something like:
**
**    ./sqlite3d 'file:dbname?vfs=kvvfs'
**
** Each K/V entry is stored in a separate file in the working
** directory that has a name like "kvvfs-dbname-*".  Due to limitations
** on the key size, the name of the database must be very short - just
** a few characters.  If the database name is too long, the VFS will
** malfunction and you will get SQLITE_CORRUPT errors.
*/
#ضمن <sqliteInt.h>
#אם SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#אם 0
#define SQLITE_KV_TRACE(X)  printf X
#אחרת
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#אם 0
#define SQLITE_KV_LOG(X)  printf X
#אחרת
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
تعريفنوع بنية KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
بنية KVVfsFile {
  sqlite3_file base;              /* IO methods */
  ثابت حرف *zClass;             /* Storage class */
  صحيح isJournal;                  /* True if this is a journal file */
  unsigned صحيح nJrnl;             /* Space allocated for aJrnl[] */
  حرف *aJrnl;                    /* Journal content */
  صحيح szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  حرف *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ساكن صحيح kvvfsClose(sqlite3_file*);
ساكن صحيح kvvfsReadDb(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح kvvfsReadJrnl(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح kvvfsWriteDb(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64);
ساكن صحيح kvvfsWriteJrnl(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64);
ساكن صحيح kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
ساكن صحيح kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
ساكن صحيح kvvfsSyncDb(sqlite3_file*, صحيح flags);
ساكن صحيح kvvfsSyncJrnl(sqlite3_file*, صحيح flags);
ساكن صحيح kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
ساكن صحيح kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
ساكن صحيح kvvfsLock(sqlite3_file*, صحيح);
ساكن صحيح kvvfsUnlock(sqlite3_file*, صحيح);
ساكن صحيح kvvfsCheckReservedLock(sqlite3_file*, صحيح *pResOut);
ساكن صحيح kvvfsFileControlDb(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح kvvfsFileControlJrnl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح kvvfsSectorSize(sqlite3_file*);
ساكن صحيح kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ساكن صحيح kvvfsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح kvvfsDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح kvvfsAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح kvvfsFullPathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *zOut);
ساكن فراغ *kvvfsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن صحيح kvvfsRandomness(sqlite3_vfs*, صحيح nByte, حرف *zOut);
ساكن صحيح kvvfsSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح kvvfsCurrentTime(sqlite3_vfs*, مزدوج*);
ساكن صحيح kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ساكن sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  حجم(KVVfsFile),              /* szOsFile */
  1024,                           /* mxPathname */
  0,                              /* pNext */
  "kvvfs",                        /* zName */
  0,                              /* pAppData */
  kvvfsOpen,                      /* xOpen */
  kvvfsDelete,                    /* xDelete */
  kvvfsAccess,                    /* xAccess */
  kvvfsFullPathname,              /* xFullPathname */
  kvvfsDlOpen,                    /* xDlOpen */
  0,                              /* xDlError */
  0,                              /* xDlSym */
  0,                              /* xDlClose */
  kvvfsRandomness,                /* xRandomness */
  kvvfsSleep,                     /* xSleep */
  kvvfsCurrentTime,               /* xCurrentTime */
  0,                              /* xGetLastError */
  kvvfsCurrentTimeInt64           /* xCurrentTimeInt64 */
};

/* Methods for sqlite3_file objects referencing a database file
*/
ساكن sqlite3_io_methods kvvfs_db_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadDb,                    /* xRead */
  kvvfsWriteDb,                   /* xWrite */
  kvvfsTruncateDb,                /* xTruncate */
  kvvfsSyncDb,                    /* xSync */
  kvvfsFileSizeDb,                /* xFileSize */
  kvvfsLock,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfsCheckReservedLock,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfsSectorSize,                /* xSectorSize */
  kvvfsDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                              /* xShmMap */
  0,                              /* xShmLock */
  0,                              /* xShmBarrier */
  0,                              /* xShmUnmap */
  0,                              /* xFetch */
  0                               /* xUnfetch */
};

/* Methods for sqlite3_file objects referencing a rollback journal
*/
ساكن sqlite3_io_methods kvvfs_jrnl_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadJrnl,                  /* xRead */
  kvvfsWriteJrnl,                 /* xWrite */
  kvvfsTruncateJrnl,              /* xTruncate */
  kvvfsSyncJrnl,                  /* xSync */
  kvvfsFileSizeJrnl,              /* xFileSize */
  kvvfsLock,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfsCheckReservedLock,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfsSectorSize,                /* xSectorSize */
  kvvfsDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                              /* xShmMap */
  0,                              /* xShmLock */
  0,                              /* xShmBarrier */
  0,                              /* xShmUnmap */
  0,                              /* xFetch */
  0                               /* xUnfetch */
};

/****** Storage subsystem **************************************************/
#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ساكن صحيح kvrecordWrite(ثابت حرف*, ثابت حرف *zKey, ثابت حرف *zData);
ساكن صحيح kvrecordDelete(ثابت حرف*, ثابت حرف *zKey);
ساكن صحيح kvrecordRead(ثابت حرف*, ثابت حرف *zKey, حرف *zBuf, صحيح nBuf);
#endif
#ifndef KVRECORD_KEY_SZ
#define KVRECORD_KEY_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ساكن فراغ kvrecordMakeKey(
  ثابت حرف *zClass,
  ثابت حرف *zKeyIn,
  حرف *zKeyOut
){
  أكد( zKeyIn );
  أكد( zKeyOut );
  أكد( zClass );
  sqlite3_snprintf(KVRECORD_KEY_SZ, zKeyOut, "kvvfs-%s-%s",
                   zClass, zKeyIn);
}

#ifndef SQLITE_WASM
/* In WASM builds do not define APIs which use fopen(), fwrite(),
** and the like because those APIs are a portability issue for
** WASM.
*/
/* Write content into a key.  zClass is the particular namespace of the
** underlying key/value store to use - either "local" or "session".
**
** Both zKey and zData are zero-terminated pure text strings.
**
** Return the number of errors.
*/
ساكن صحيح kvrecordWrite(
  ثابت حرف *zClass,
  ثابت حرف *zKey,
  ثابت حرف *zData
){
  FILE *fd;
  حرف zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  fd = fopen(zXKey, "wb");
  אם( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (صحيح)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    החזר 0;
  }אחרת{
    החזר 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
ساكن صحيح kvrecordDelete(ثابت حرف *zClass, ثابت حرف *zKey){
  حرف zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  החזר 0;
}

/* Read the value associated with a zKey from the key/value namespace given
** by zClass and put the text data associated with that key in the first
** nBuf bytes of zBuf[].  The value might be truncated if zBuf is not large
** enough to hold it all.  The value put into zBuf must always be zero
** terminated, even if it gets truncated because nBuf is not large enough.
**
** Return the total number of bytes in the data, without truncation, and
** not counting the final zero terminator.   Return -1 if the key does
** not exist or its key cannot be read.
**
** If nBuf<=0 then this routine simply returns the size of the data
** without actually reading it.  Similarly, if nBuf==1 then it
** zero-terminates zBuf at zBuf[0] and returns the size of the data
** without reading it.
*/
ساكن صحيح kvrecordRead(
  ثابت حرف *zClass,
  ثابت حرف *zKey,
  حرف *zBuf,
  صحيح nBuf
){
  FILE *fd;
  بنية stat buf;
  حرف zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  אם( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    החזר -1;
  }
  אם( nBuf<=0 ){
    החזר (صحيح)buf.st_size;
  }אחרת אם( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (صحيح)buf.st_size));
    החזר (صحيح)buf.st_size;
  }
  אם( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  אם( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    החזר -1;
  }אחרת{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    החזר (صحيح)n;
  }
}
#endif /* #ifndef SQLITE_WASM */


/*
** An internal level of indirection which enables us to replace the
** kvvfs i/o methods with JavaScript implementations in WASM builds.
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
تعريفنوع بنية sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
بنية sqlite3_kvvfs_methods {
  صحيح (*xRcrdRead)(ثابت حرف*, ثابت حرف *zKey, حرف *zBuf, صحيح nBuf);
  صحيح (*xRcrdWrite)(ثابت حرف*, ثابت حرف *zKey, ثابت حرف *zData);
  صحيح (*xRcrdDelete)(ثابت حرف*, ثابت حرف *zKey);
  ثابت صحيح nKeySize;
  ثابت صحيح nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST ثابت
#אחרת
#  define MAYBE_CONST
#endif
  MAYBE_CONST sqlite3_vfs * pVfs;
  MAYBE_CONST sqlite3_io_methods *pIoDb;
  MAYBE_CONST sqlite3_io_methods *pIoJrnl;
#undef MAYBE_CONST
};


/*
** This object holds the kvvfs I/O methods which may be swapped out
** for JavaScript-side implementations in WASM builds. In such builds
** it cannot be const, but in native builds it should be so that
** the compiler can hopefully optimize this level of indirection out.
** That said, kvvfs is intended primarily for use in WASM builds.
**
** This is not explicitly flagged as static because the amalgamation
** build will tag it with SQLITE_PRIVATE.
*/
#ifndef SQLITE_WASM
ثابت
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#אחרת
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .nKeySize        = KVRECORD_KEY_SZ,
  .nBufferSize     = SQLITE_KVOS_SZ,
  .pVfs            = &sqlite3OsKvvfsObject,
  .pIoDb           = &kvvfs_db_io_methods,
  .pIoJrnl         = &kvvfs_jrnl_io_methods
};

/****** Utility subroutines ************************************************/

/*
** Encode binary into the text encoded used to persist on disk.
** The output text is stored in aOut[], which must be at least
** nData+1 bytes in length.
**
** Return the actual length of the encoded text, not counting the
** zero terminator at the end.
**
** Encoding format
** ---------------
**
**   *  Non-zero bytes are encoded as upper-case hexadecimal
**
**   *  A sequence of one or more zero-bytes that are not at the
**      beginning of the buffer are encoded as a little-endian
**      base-26 number using a..z.  "a" means 0.  "b" means 1,
**      "z" means 25.  "ab" means 26.  "ac" means 52.  And so forth.
**
**   *  Because there is no overlap between the encoding characters
**      of hexadecimal and base-26 numbers, it is always clear where
**      one stops and the next begins.
*/
#ifndef SQLITE_WASM
ساكن
#endif
صحيح kvvfsEncode(ثابت حرف *aData, صحيح nData, حرف *aOut){
  صحيح i, j;
  ثابت unsigned حرف *a = (ثابت unsigned حرف*)aData;
  עבור(i=j=0; i<nData; i++){
    unsigned حرف c = a[i];
    אם( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }אחרת{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      صحيح k;
      עבור(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      כלעוד( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  החזר j;
}

ساكن ثابت signed حرف kvvfsHexValue[256] = {
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
   0,  1,  2,  3,  4,  5,  6,  7,    8,  9, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,

  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1
};

/*
** Decode the text encoding back to binary.  The binary content is
** written into pOut, which must be at least nOut bytes in length.
**
** The return value is the number of bytes actually written into aOut[], or
** -1 for malformed inputs.
*/
#ifndef SQLITE_WASM
ساكن
#endif
صحيح kvvfsDecode(ثابت حرف *a, حرف *aOut, صحيح nOut){
  صحيح i, j;
  صحيح c;
  ثابت unsigned حرف *aIn = (ثابت unsigned حرف*)a;
  i = 0;
  j = 0;
  כלעוד( 1 ){
    c = kvvfsHexValue[aIn[i]];
    אם( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      אם( c==0 ) اكسر;
      כלעוד( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        אם( n>nOut ) החזר -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      אם( j+n>nOut ) החזר -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      אם( c==0 || mult==1 ) اكسر; /* progress stalled if mult==1 */
    }אחרת אם( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfsHexValue[aIn[++i]];
      אם( c<0 ) החזר -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }אחרת{
      החזר -1;
    }
  }
  החזר j;
}

/*
** Decode a complete journal file.  Allocate space in pFile->aJrnl
** and store the decoding there.  Or leave pFile->aJrnl set to NULL
** if an error is encountered.
**
** The first few characters of the text encoding will be a little-endian
** base-26 number (digits a..z) that is the total number of bytes
** in the decoded journal file image.  This base-26 number is followed
** by a single space, then the encoding of the journal.  The space
** separator is required to act as a terminator for the base-26 number.
*/
ساكن فراغ kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ثابت حرف *zTxt,      /* Text encoding.  Zero-terminated */
  صحيح nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned صحيح n = 0, mult;
  صحيح c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  כלעוד( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  אם( ' '!=zTxt[i++] ){
    /* Malformed input */
    החזר;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  אם( pFile->aJrnl==0 ){
    החזר;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  אם( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
ساكن sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  حرف zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                حجم(zData)-1);
  החזר strtoll(zData, 0, 0);
}
ساكن صحيح kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  حرف zData[50];
  sqlite3_snprintf(حجم(zData), zData, "%lld", sz);
  החזר sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
ساكن صحيح kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, حجم(*pFile));
  החזר SQLITE_OK;
}

/*
** Read from the -journal file.
*/
ساكن صحيح kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  أكد( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  אם( pFile->aJrnl==0 ){
    صحيح rc;
    صحيح szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    حرف *aTxt;
    אם( szTxt<=4 ){
      החזר SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    אם( aTxt==0 ) החזר SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    אם( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    אם( rc ) החזר rc;
    אם( pFile->aJrnl==0 ) החזר SQLITE_IOERR;
  }
  אם( iOfst+iAmt>pFile->nJrnl ){
    החזר SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  החזר SQLITE_OK;
}

/*
** Read from the database file.
*/
ساكن صحيح kvvfsReadDb(
  sqlite3_file *pProtoFile,
  فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned صحيح pgno;
  صحيح got, n;
  حرف zKey[30];
  حرف *aData = pFile->aData;
  أكد( iOfst>=0 );
  أكد( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  אם( iOfst+iAmt>=512 ){
    אם( (iOfst % iAmt)!=0 ){
      החזר SQLITE_IOERR_READ;
    }
    אם( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      החזר SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }אחרת{
    pgno = 1;
  }
  sqlite3_snprintf(حجم(zKey), zKey, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, zKey,
                                      aData, SQLITE_KVOS_SZ-1);
  אם( got<0 ){
    n = 0;
  }אחרת{
    aData[got] = 0;
    אם( iOfst+iAmt<512 ){
      صحيح k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      אם( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }אחרת{
        n = 0;
      }
    }אחרת{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  אם( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    החזר SQLITE_IOERR_SHORT_READ;
  }
  החזר SQLITE_OK;
}


/*
** Write into the -journal file.
*/
ساكن صحيح kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ثابت فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  אם( iEnd>=0x10000000 ) החזר SQLITE_FULL;
  אם( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    حرف *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    אם( aNew==0 ){
      החזר SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    אם( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  החזר SQLITE_OK;
}

/*
** Write into the database file.
*/
ساكن صحيح kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ثابت فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned صحيح pgno;
  حرف zKey[30];
  حرف *aData = pFile->aData;
  صحيح rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  أكد( iAmt>=512 && iAmt<=65536 );
  أكد( (iAmt & (iAmt-1))==0 );
  أكد( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(حجم(zKey), zKey, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, zKey, aData);
  אם( 0==rc ){
    אם( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  החזר rc;
}

/*
** Truncate an kvvfs-file.
*/
ساكن صحيح kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  أكد( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  החזר SQLITE_OK;
}
ساكن صحيح kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  אם( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    حرف zKey[50];
    unsigned صحيح pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    כלעוד( pgno<=pgnoMax ){
      sqlite3_snprintf(حجم(zKey), zKey, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, zKey);
      pgno++;
    }
    pFile->szDb = size;
    החזר kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  החזר SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ساكن صحيح kvvfsSyncJrnl(sqlite3_file *pProtoFile, صحيح flags){
  صحيح i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  حرف *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  אם( pFile->nJrnl<=0 ){
    החזר kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  אם( zOut==0 ){
    החזר SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  עשה{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }כלעוד( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  החזר i ? SQLITE_IOERR : SQLITE_OK;
}
ساكن صحيح kvvfsSyncDb(sqlite3_file *pProtoFile, صحيح flags){
  החזר SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ساكن صحيح kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  החזר SQLITE_OK;
}
ساكن صحيح kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  אם( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }אחרת{
    *pSize = kvvfsReadFileSize(pFile);
  }
  החזר SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ساكن صحيح kvvfsLock(sqlite3_file *pProtoFile, صحيح eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  אם( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  החזר SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ساكن صحيح kvvfsUnlock(sqlite3_file *pProtoFile, صحيح eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  אם( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  החזר SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ساكن صحيح kvvfsCheckReservedLock(sqlite3_file *pProtoFile, صحيح *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  החזר SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
ساكن صحيح kvvfsFileControlJrnl(sqlite3_file *pProtoFile, صحيح op, فراغ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  החזר SQLITE_NOTFOUND;
}
ساكن صحيح kvvfsFileControlDb(sqlite3_file *pProtoFile, صحيح op, فراغ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  אם( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    صحيح rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    אם( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    החזר rc;
  }
  החזר SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ساكن صحيح kvvfsSectorSize(sqlite3_file *pFile){
  החזר 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
ساكن صحيح kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  החזר 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
ساكن صحيح kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ثابت حرف *zName,
  sqlite3_file *pProtoFile,
  صحيح flags,
  صحيح *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  אם( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  أكد(!pFile->zClass);
  أكد(!pFile->aData);
  أكد(!pFile->aJrnl);
  أكد(!pFile->nJrnl);
  أكد(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  אם( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    אם( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }אחרת אם( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }אחרת{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  אם( !pFile->zClass ){
#ifdef SQLITE_WASM
    אם( strlen(zName) >= (KVRECORD_KEY_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      החזר SQLITE_CANTOPEN;
    }
#אחרת
    אם( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      החזר SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  אם( pFile->aData==0 ){
    החזר SQLITE_NOMEM;
  }
  החזר SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح kvvfsDelete(sqlite3_vfs *pVfs, ثابت حرف *zPath, صحيح dirSync){
  صحيح rc /* The JS impl can fail with OOM in argument conversion */;
  אם( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }אחרת
  אם( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  אחרת{
    rc = 0;
  }
  החזר rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ثابت حرف *zPath,
  صحيح flags,
  صحيح *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#אם 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ثابت حرف *zKey = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, zKey, 0, 0)>0;
#אחרת
  אם( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }אחרת
  אם( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }אחרת
  אם( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }אחרת
  אם( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }אחרת
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  החזר SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  ثابت حرف *zPath,
  صحيح nOut,
  حرف *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  אם( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  החזר SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *kvvfsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zPath){
  החזר 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ساكن صحيح kvvfsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *zBufOut){
  memset(zBufOut, 0, nByte);
  החזר nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
ساكن صحيح kvvfsSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  החזר SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح kvvfsCurrentTime(sqlite3_vfs *pVfs, مزدوج *pTimeOut){
  sqlite3_int64 i = 0;
  صحيح rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  החזר rc;
}
#ضمن <sys/time.h>
ساكن صحيح kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ساكن ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  بنية timeval sNow;
  (فراغ)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  החזר SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#אם SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
صحيح sqlite3_os_init(فراغ){
  החזר sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
صحيح sqlite3_os_end(فراغ){
  החזר SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#אם SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
صحيح sqlite3KvvfsInit(فراغ){
  החזר sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
