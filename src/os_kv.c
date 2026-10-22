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
#شامل <sqliteInt.h>
#اگر SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#اگر 0
#define SQLITE_KV_TRACE(X)  printf X
#وگرنه
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#اگر 0
#define SQLITE_KV_LOG(X)  printf X
#وگرنه
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
تعریفنوع ساختار KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ساختار KVVfsFile {
  sqlite3_file base;              /* IO methods */
  ثابت نویسه *zClass;             /* Storage class */
  صحیح isJournal;                  /* True if this is a journal file */
  unsigned صحیح nJrnl;             /* Space allocated for aJrnl[] */
  نویسه *aJrnl;                    /* Journal content */
  صحیح szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  نویسه *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ایستا صحیح kvvfsClose(sqlite3_file*);
ایستا صحیح kvvfsReadDb(sqlite3_file*, تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح kvvfsReadJrnl(sqlite3_file*, تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح kvvfsWriteDb(sqlite3_file*,ثابت تهی*,صحیح iAmt, sqlite3_int64);
ایستا صحیح kvvfsWriteJrnl(sqlite3_file*,ثابت تهی*,صحیح iAmt, sqlite3_int64);
ایستا صحیح kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
ایستا صحیح kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
ایستا صحیح kvvfsSyncDb(sqlite3_file*, صحیح flags);
ایستا صحیح kvvfsSyncJrnl(sqlite3_file*, صحیح flags);
ایستا صحیح kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
ایستا صحیح kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
ایستا صحیح kvvfsLock(sqlite3_file*, صحیح);
ایستا صحیح kvvfsUnlock(sqlite3_file*, صحیح);
ایستا صحیح kvvfsCheckReservedLock(sqlite3_file*, صحیح *pResOut);
ایستا صحیح kvvfsFileControlDb(sqlite3_file*, صحیح op, تهی *pArg);
ایستا صحیح kvvfsFileControlJrnl(sqlite3_file*, صحیح op, تهی *pArg);
ایستا صحیح kvvfsSectorSize(sqlite3_file*);
ایستا صحیح kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ایستا صحیح kvvfsOpen(sqlite3_vfs*, ثابت نویسه *, sqlite3_file*, صحیح , صحیح *);
ایستا صحیح kvvfsDelete(sqlite3_vfs*, ثابت نویسه *zName, صحیح syncDir);
ایستا صحیح kvvfsAccess(sqlite3_vfs*, ثابت نویسه *zName, صحیح flags, صحیح *);
ایستا صحیح kvvfsFullPathname(sqlite3_vfs*, ثابت نویسه *zName, صحیح, نویسه *zOut);
ایستا تهی *kvvfsDlOpen(sqlite3_vfs*, ثابت نویسه *zFilename);
ایستا صحیح kvvfsRandomness(sqlite3_vfs*, صحیح nByte, نویسه *zOut);
ایستا صحیح kvvfsSleep(sqlite3_vfs*, صحیح microseconds);
ایستا صحیح kvvfsCurrentTime(sqlite3_vfs*, دوگانه*);
ایستا صحیح kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ایستا sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  اندازه(KVVfsFile),              /* szOsFile */
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
ایستا sqlite3_io_methods kvvfs_db_io_methods = {
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
ایستا sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ایستا صحیح kvrecordWrite(ثابت نویسه*, ثابت نویسه *z_کلید, ثابت نویسه *zData);
ایستا صحیح kvrecordDelete(ثابت نویسه*, ثابت نویسه *z_کلید);
ایستا صحیح kvrecordRead(ثابت نویسه*, ثابت نویسه *z_کلید, نویسه *zBuf, صحیح nBuf);
#endif
#ifndef KVRECORD_کلید_SZ
#define KVRECORD_کلید_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ایستا تهی kvrecord_Make_کلید(
  ثابت نویسه *zClass,
  ثابت نویسه *z_کلید_In,
  نویسه *z_کلید_Out
){
  أكد( z_کلید_In );
  أكد( z_کلید_Out );
  أكد( zClass );
  sqlite3_snprintf(KVRECORD_کلید_SZ, z_کلید_Out, "kvvfs-%s-%s",
                   zClass, z_کلید_In);
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
ایستا صحیح kvrecordWrite(
  ثابت نویسه *zClass,
  ثابت نویسه *z_کلید,
  ثابت نویسه *zData
){
  FILE *fd;
  نویسه zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
  fd = fopen(zXKey, "wb");
  اگر( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (صحیح)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    بازگشت 0;
  }وگرنه{
    بازگشت 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
ایستا صحیح kvrecordDelete(ثابت نویسه *zClass, ثابت نویسه *z_کلید){
  نویسه zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  بازگشت 0;
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
ایستا صحیح kvrecordRead(
  ثابت نویسه *zClass,
  ثابت نویسه *z_کلید,
  نویسه *zBuf,
  صحیح nBuf
){
  FILE *fd;
  ساختار stat buf;
  نویسه zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
  اگر( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    بازگشت -1;
  }
  اگر( nBuf<=0 ){
    بازگشت (صحیح)buf.st_size;
  }وگرنه اگر( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (صحیح)buf.st_size));
    بازگشت (صحیح)buf.st_size;
  }
  اگر( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  اگر( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    بازگشت -1;
  }وگرنه{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    بازگشت (صحیح)n;
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
تعریفنوع ساختار sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
ساختار sqlite3_kvvfs_methods {
  صحیح (*xRcrdRead)(ثابت نویسه*, ثابت نویسه *z_کلید, نویسه *zBuf, صحیح nBuf);
  صحیح (*xRcrdWrite)(ثابت نویسه*, ثابت نویسه *z_کلید, ثابت نویسه *zData);
  صحیح (*xRcrdDelete)(ثابت نویسه*, ثابت نویسه *z_کلید);
  ثابت صحیح n_کلید_Size;
  ثابت صحیح nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST ثابت
#وگرنه
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
#وگرنه
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .n_کلید_Size        = KVRECORD_کلید_SZ,
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
ایستا
#endif
صحیح kvvfsEncode(ثابت نویسه *aData, صحیح nData, نویسه *aOut){
  صحیح i, j;
  ثابت unsigned نویسه *a = (ثابت unsigned نویسه*)aData;
  برای(i=j=0; i<nData; i++){
    unsigned نویسه c = a[i];
    اگر( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }وگرنه{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      صحیح k;
      برای(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      وقتی( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  بازگشت j;
}

ایستا ثابت signed نویسه kvvfs_Hex_مقدار[256] = {
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
ایستا
#endif
صحیح kvvfsDecode(ثابت نویسه *a, نویسه *aOut, صحیح nOut){
  صحیح i, j;
  صحیح c;
  ثابت unsigned نویسه *aIn = (ثابت unsigned نویسه*)a;
  i = 0;
  j = 0;
  وقتی( 1 ){
    c = kvvfs_Hex_مقدار[aIn[i]];
    اگر( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      اگر( c==0 ) بشکن;
      وقتی( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        اگر( n>nOut ) بازگشت -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      اگر( j+n>nOut ) بازگشت -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      اگر( c==0 || mult==1 ) بشکن; /* progress stalled if mult==1 */
    }وگرنه اگر( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_مقدار[aIn[++i]];
      اگر( c<0 ) بازگشت -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }وگرنه{
      بازگشت -1;
    }
  }
  بازگشت j;
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
ایستا تهی kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ثابت نویسه *zTxt,      /* Text encoding.  Zero-terminated */
  صحیح nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned صحیح n = 0, mult;
  صحیح c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  وقتی( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  اگر( ' '!=zTxt[i++] ){
    /* Malformed input */
    بازگشت;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  اگر( pFile->aJrnl==0 ){
    بازگشت;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  اگر( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
ایستا sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  نویسه zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                اندازه(zData)-1);
  بازگشت strtoll(zData, 0, 0);
}
ایستا صحیح kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  نویسه zData[50];
  sqlite3_snprintf(اندازه(zData), zData, "%lld", sz);
  بازگشت sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
ایستا صحیح kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, اندازه(*pFile));
  بازگشت SQLITE_OK;
}

/*
** Read from the -journal file.
*/
ایستا صحیح kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  تهی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  أكد( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  اگر( pFile->aJrnl==0 ){
    صحیح rc;
    صحیح szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    نویسه *aTxt;
    اگر( szTxt<=4 ){
      بازگشت SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    اگر( aTxt==0 ) بازگشت SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    اگر( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    اگر( rc ) بازگشت rc;
    اگر( pFile->aJrnl==0 ) بازگشت SQLITE_IOERR;
  }
  اگر( iOfst+iAmt>pFile->nJrnl ){
    بازگشت SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  بازگشت SQLITE_OK;
}

/*
** Read from the database file.
*/
ایستا صحیح kvvfsReadDb(
  sqlite3_file *pProtoFile,
  تهی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned صحیح pgno;
  صحیح got, n;
  نویسه z_کلید[30];
  نویسه *aData = pFile->aData;
  أكد( iOfst>=0 );
  أكد( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  اگر( iOfst+iAmt>=512 ){
    اگر( (iOfst % iAmt)!=0 ){
      بازگشت SQLITE_IOERR_READ;
    }
    اگر( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      بازگشت SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }وگرنه{
    pgno = 1;
  }
  sqlite3_snprintf(اندازه(z_کلید), z_کلید, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_کلید,
                                      aData, SQLITE_KVOS_SZ-1);
  اگر( got<0 ){
    n = 0;
  }وگرنه{
    aData[got] = 0;
    اگر( iOfst+iAmt<512 ){
      صحیح k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      اگر( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }وگرنه{
        n = 0;
      }
    }وگرنه{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  اگر( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    بازگشت SQLITE_IOERR_SHORT_READ;
  }
  بازگشت SQLITE_OK;
}


/*
** Write into the -journal file.
*/
ایستا صحیح kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ثابت تهی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  اگر( iEnd>=0x10000000 ) بازگشت SQLITE_FULL;
  اگر( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    نویسه *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    اگر( aNew==0 ){
      بازگشت SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    اگر( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  بازگشت SQLITE_OK;
}

/*
** Write into the database file.
*/
ایستا صحیح kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ثابت تهی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned صحیح pgno;
  نویسه z_کلید[30];
  نویسه *aData = pFile->aData;
  صحیح rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  أكد( iAmt>=512 && iAmt<=65536 );
  أكد( (iAmt & (iAmt-1))==0 );
  أكد( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(اندازه(z_کلید), z_کلید, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_کلید, aData);
  اگر( 0==rc ){
    اگر( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  بازگشت rc;
}

/*
** Truncate an kvvfs-file.
*/
ایستا صحیح kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  أكد( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  بازگشت SQLITE_OK;
}
ایستا صحیح kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  اگر( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    نویسه z_کلید[50];
    unsigned صحیح pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    وقتی( pgno<=pgnoMax ){
      sqlite3_snprintf(اندازه(z_کلید), z_کلید, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_کلید);
      pgno++;
    }
    pFile->szDb = size;
    بازگشت kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  بازگشت SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ایستا صحیح kvvfsSyncJrnl(sqlite3_file *pProtoFile, صحیح flags){
  صحیح i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  نویسه *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  اگر( pFile->nJrnl<=0 ){
    بازگشت kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  اگر( zOut==0 ){
    بازگشت SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  انجامبده{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }وقتی( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  بازگشت i ? SQLITE_IOERR : SQLITE_OK;
}
ایستا صحیح kvvfsSyncDb(sqlite3_file *pProtoFile, صحیح flags){
  بازگشت SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ایستا صحیح kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  بازگشت SQLITE_OK;
}
ایستا صحیح kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  اگر( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }وگرنه{
    *pSize = kvvfsReadFileSize(pFile);
  }
  بازگشت SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ایستا صحیح kvvfsLock(sqlite3_file *pProtoFile, صحیح eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  اگر( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  بازگشت SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ایستا صحیح kvvfsUnlock(sqlite3_file *pProtoFile, صحیح eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  اگر( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  بازگشت SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ایستا صحیح kvvfsCheckReservedLock(sqlite3_file *pProtoFile, صحیح *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  بازگشت SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
ایستا صحیح kvvfsFileControlJrnl(sqlite3_file *pProtoFile, صحیح op, تهی *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  بازگشت SQLITE_NOTFOUND;
}
ایستا صحیح kvvfsFileControlDb(sqlite3_file *pProtoFile, صحیح op, تهی *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  اگر( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    صحیح rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    اگر( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    بازگشت rc;
  }
  بازگشت SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ایستا صحیح kvvfsSectorSize(sqlite3_file *pFile){
  بازگشت 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
ایستا صحیح kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  بازگشت 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
ایستا صحیح kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ثابت نویسه *zName,
  sqlite3_file *pProtoFile,
  صحیح flags,
  صحیح *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  اگر( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  أكد(!pFile->zClass);
  أكد(!pFile->aData);
  أكد(!pFile->aJrnl);
  أكد(!pFile->nJrnl);
  أكد(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  اگر( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    اگر( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }وگرنه اگر( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }وگرنه{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  اگر( !pFile->zClass ){
#ifdef SQLITE_WASM
    اگر( strlen(zName) >= (KVRECORD_کلید_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      بازگشت SQLITE_CANTOPEN;
    }
#وگرنه
    اگر( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      بازگشت SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  اگر( pFile->aData==0 ){
    بازگشت SQLITE_NOMEM;
  }
  بازگشت SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ایستا صحیح kvvfsDelete(sqlite3_vfs *pVfs, ثابت نویسه *zPath, صحیح dirSync){
  صحیح rc /* The JS impl can fail with OOM in argument conversion */;
  اگر( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }وگرنه
  اگر( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  وگرنه{
    rc = 0;
  }
  بازگشت rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ایستا صحیح kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ثابت نویسه *zPath,
  صحیح flags,
  صحیح *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#اگر 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ثابت نویسه *z_کلید = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_کلید, 0, 0)>0;
#وگرنه
  اگر( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }وگرنه
  اگر( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }وگرنه
  اگر( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }وگرنه
  اگر( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }وگرنه
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  بازگشت SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ایستا صحیح kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zPath,
  صحیح nOut,
  نویسه *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  اگر( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  بازگشت SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ایستا تهی *kvvfsDlOpen(sqlite3_vfs *pVfs, ثابت نویسه *zPath){
  بازگشت 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ایستا صحیح kvvfsRandomness(sqlite3_vfs *pVfs, صحیح nByte, نویسه *zBufOut){
  memset(zBufOut, 0, nByte);
  بازگشت nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
ایستا صحیح kvvfsSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  بازگشت SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ایستا صحیح kvvfsCurrentTime(sqlite3_vfs *pVfs, دوگانه *pTimeOut){
  sqlite3_int64 i = 0;
  صحیح rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  بازگشت rc;
}
#شامل <sys/time.h>
ایستا صحیح kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ایستا ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ساختار timeval sNow;
  (تهی)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  بازگشت SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#اگر SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
صحیح sqlite3_os_init(تهی){
  بازگشت sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
صحیح sqlite3_os_end(تهی){
  بازگشت SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#اگر SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
صحیح sqlite3KvvfsInit(تهی){
  بازگشت sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
