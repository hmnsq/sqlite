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
#அந்தர்பாவ <sqliteInt.h>
#எனில் SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#எனில் 0
#define SQLITE_KV_TRACE(X)  printf X
#இல்லையெனில்
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#எனில் 0
#define SQLITE_KV_LOG(X)  printf X
#இல்லையெனில்
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
வகைவரை கட்டமைப்பு KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
கட்டமைப்பு KVVfsFile {
  sqlite3_file base;              /* IO methods */
  மாறிலி எழுத்து *zClass;             /* Storage class */
  முழு isJournal;                  /* True if this is a journal file */
  unsigned முழு nJrnl;             /* Space allocated for aJrnl[] */
  எழுத்து *aJrnl;                    /* Journal content */
  முழு szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  எழுத்து *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
நிலையான முழு kvvfsClose(sqlite3_file*);
நிலையான முழு kvvfsReadDb(sqlite3_file*, வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு kvvfsReadJrnl(sqlite3_file*, வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு kvvfsWriteDb(sqlite3_file*,மாறிலி வெறுமை*,முழு iAmt, sqlite3_int64);
நிலையான முழு kvvfsWriteJrnl(sqlite3_file*,மாறிலி வெறுமை*,முழு iAmt, sqlite3_int64);
நிலையான முழு kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
நிலையான முழு kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
நிலையான முழு kvvfsSyncDb(sqlite3_file*, முழு flags);
நிலையான முழு kvvfsSyncJrnl(sqlite3_file*, முழு flags);
நிலையான முழு kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
நிலையான முழு kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
நிலையான முழு kvvfsLock(sqlite3_file*, முழு);
நிலையான முழு kvvfsUnlock(sqlite3_file*, முழு);
நிலையான முழு kvvfsCheckReservedLock(sqlite3_file*, முழு *pResOut);
நிலையான முழு kvvfsFileControlDb(sqlite3_file*, முழு op, வெறுமை *pArg);
நிலையான முழு kvvfsFileControlJrnl(sqlite3_file*, முழு op, வெறுமை *pArg);
நிலையான முழு kvvfsSectorSize(sqlite3_file*);
நிலையான முழு kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
நிலையான முழு kvvfsOpen(sqlite3_vfs*, மாறிலி எழுத்து *, sqlite3_file*, முழு , முழு *);
நிலையான முழு kvvfsDelete(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு syncDir);
நிலையான முழு kvvfsAccess(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு flags, முழு *);
நிலையான முழு kvvfsFullPathname(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு, எழுத்து *zOut);
நிலையான வெறுமை *kvvfsDlOpen(sqlite3_vfs*, மாறிலி எழுத்து *zFilename);
நிலையான முழு kvvfsRandomness(sqlite3_vfs*, முழு nByte, எழுத்து *zOut);
நிலையான முழு kvvfsSleep(sqlite3_vfs*, முழு microseconds);
நிலையான முழு kvvfsCurrentTime(sqlite3_vfs*, இரட்டை*);
நிலையான முழு kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

நிலையான sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  அளவு(KVVfsFile),              /* szOsFile */
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
நிலையான sqlite3_io_methods kvvfs_db_io_methods = {
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
நிலையான sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
நிலையான முழு kvrecordWrite(மாறிலி எழுத்து*, மாறிலி எழுத்து *z_சாவி, மாறிலி எழுத்து *zData);
நிலையான முழு kvrecordDelete(மாறிலி எழுத்து*, மாறிலி எழுத்து *z_சாவி);
நிலையான முழு kvrecordRead(மாறிலி எழுத்து*, மாறிலி எழுத்து *z_சாவி, எழுத்து *zBuf, முழு nBuf);
#endif
#ifndef KVRECORD_சாவி_SZ
#define KVRECORD_சாவி_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
நிலையான வெறுமை kvrecord_Make_சாவி(
  மாறிலி எழுத்து *zClass,
  மாறிலி எழுத்து *z_சாவி_In,
  எழுத்து *z_சாவி_Out
){
  ப்ரதிஜ்ஞா( z_சாவி_In );
  ப்ரதிஜ்ஞா( z_சாவி_Out );
  ப்ரதிஜ்ஞா( zClass );
  sqlite3_snprintf(KVRECORD_சாவி_SZ, z_சாவி_Out, "kvvfs-%s-%s",
                   zClass, z_சாவி_In);
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
நிலையான முழு kvrecordWrite(
  மாறிலி எழுத்து *zClass,
  மாறிலி எழுத்து *z_சாவி,
  மாறிலி எழுத்து *zData
){
  FILE *fd;
  எழுத்து zXKey[KVRECORD_சாவி_SZ];
  kvrecord_Make_சாவி(zClass, z_சாவி, zXKey);
  fd = fopen(zXKey, "wb");
  எனில்( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (முழு)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    திரும்பு 0;
  }இல்லையெனில்{
    திரும்பு 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
நிலையான முழு kvrecordDelete(மாறிலி எழுத்து *zClass, மாறிலி எழுத்து *z_சாவி){
  எழுத்து zXKey[KVRECORD_சாவி_SZ];
  kvrecord_Make_சாவி(zClass, z_சாவி, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  திரும்பு 0;
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
நிலையான முழு kvrecordRead(
  மாறிலி எழுத்து *zClass,
  மாறிலி எழுத்து *z_சாவி,
  எழுத்து *zBuf,
  முழு nBuf
){
  FILE *fd;
  கட்டமைப்பு stat buf;
  எழுத்து zXKey[KVRECORD_சாவி_SZ];
  kvrecord_Make_சாவி(zClass, z_சாவி, zXKey);
  எனில்( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    திரும்பு -1;
  }
  எனில்( nBuf<=0 ){
    திரும்பு (முழு)buf.st_size;
  }இல்லையெனில் எனில்( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (முழு)buf.st_size));
    திரும்பு (முழு)buf.st_size;
  }
  எனில்( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  எனில்( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    திரும்பு -1;
  }இல்லையெனில்{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    திரும்பு (முழு)n;
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
வகைவரை கட்டமைப்பு sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
கட்டமைப்பு sqlite3_kvvfs_methods {
  முழு (*xRcrdRead)(மாறிலி எழுத்து*, மாறிலி எழுத்து *z_சாவி, எழுத்து *zBuf, முழு nBuf);
  முழு (*xRcrdWrite)(மாறிலி எழுத்து*, மாறிலி எழுத்து *z_சாவி, மாறிலி எழுத்து *zData);
  முழு (*xRcrdDelete)(மாறிலி எழுத்து*, மாறிலி எழுத்து *z_சாவி);
  மாறிலி முழு n_சாவி_Size;
  மாறிலி முழு nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST மாறிலி
#இல்லையெனில்
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
மாறிலி
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#இல்லையெனில்
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .n_சாவி_Size        = KVRECORD_சாவி_SZ,
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
நிலையான
#endif
முழு kvvfsEncode(மாறிலி எழுத்து *aData, முழு nData, எழுத்து *aOut){
  முழு i, j;
  மாறிலி unsigned எழுத்து *a = (மாறிலி unsigned எழுத்து*)aData;
  க்கு(i=j=0; i<nData; i++){
    unsigned எழுத்து c = a[i];
    எனில்( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }இல்லையெனில்{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      முழு k;
      க்கு(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      போது( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  திரும்பு j;
}

நிலையான மாறிலி signed எழுத்து kvvfs_Hex_மதிப்பு[256] = {
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
நிலையான
#endif
முழு kvvfsDecode(மாறிலி எழுத்து *a, எழுத்து *aOut, முழு nOut){
  முழு i, j;
  முழு c;
  மாறிலி unsigned எழுத்து *aIn = (மாறிலி unsigned எழுத்து*)a;
  i = 0;
  j = 0;
  போது( 1 ){
    c = kvvfs_Hex_மதிப்பு[aIn[i]];
    எனில்( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      எனில்( c==0 ) நிறுத்து;
      போது( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        எனில்( n>nOut ) திரும்பு -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      எனில்( j+n>nOut ) திரும்பு -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      எனில்( c==0 || mult==1 ) நிறுத்து; /* progress stalled if mult==1 */
    }இல்லையெனில் எனில்( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_மதிப்பு[aIn[++i]];
      எனில்( c<0 ) திரும்பு -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }இல்லையெனில்{
      திரும்பு -1;
    }
  }
  திரும்பு j;
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
நிலையான வெறுமை kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  மாறிலி எழுத்து *zTxt,      /* Text encoding.  Zero-terminated */
  முழு nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned முழு n = 0, mult;
  முழு c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  போது( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  எனில்( ' '!=zTxt[i++] ){
    /* Malformed input */
    திரும்பு;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  எனில்( pFile->aJrnl==0 ){
    திரும்பு;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  எனில்( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
நிலையான sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  எழுத்து zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                அளவு(zData)-1);
  திரும்பு strtoll(zData, 0, 0);
}
நிலையான முழு kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  எழுத்து zData[50];
  sqlite3_snprintf(அளவு(zData), zData, "%lld", sz);
  திரும்பு sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
நிலையான முழு kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, அளவு(*pFile));
  திரும்பு SQLITE_OK;
}

/*
** Read from the -journal file.
*/
நிலையான முழு kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  வெறுமை *zBuf,
  முழு iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ப்ரதிஜ்ஞா( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  எனில்( pFile->aJrnl==0 ){
    முழு rc;
    முழு szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    எழுத்து *aTxt;
    எனில்( szTxt<=4 ){
      திரும்பு SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    எனில்( aTxt==0 ) திரும்பு SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    எனில்( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    எனில்( rc ) திரும்பு rc;
    எனில்( pFile->aJrnl==0 ) திரும்பு SQLITE_IOERR;
  }
  எனில்( iOfst+iAmt>pFile->nJrnl ){
    திரும்பு SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  திரும்பு SQLITE_OK;
}

/*
** Read from the database file.
*/
நிலையான முழு kvvfsReadDb(
  sqlite3_file *pProtoFile,
  வெறுமை *zBuf,
  முழு iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned முழு pgno;
  முழு got, n;
  எழுத்து z_சாவி[30];
  எழுத்து *aData = pFile->aData;
  ப்ரதிஜ்ஞா( iOfst>=0 );
  ப்ரதிஜ்ஞா( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  எனில்( iOfst+iAmt>=512 ){
    எனில்( (iOfst % iAmt)!=0 ){
      திரும்பு SQLITE_IOERR_READ;
    }
    எனில்( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      திரும்பு SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }இல்லையெனில்{
    pgno = 1;
  }
  sqlite3_snprintf(அளவு(z_சாவி), z_சாவி, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_சாவி,
                                      aData, SQLITE_KVOS_SZ-1);
  எனில்( got<0 ){
    n = 0;
  }இல்லையெனில்{
    aData[got] = 0;
    எனில்( iOfst+iAmt<512 ){
      முழு k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      எனில்( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }இல்லையெனில்{
        n = 0;
      }
    }இல்லையெனில்{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  எனில்( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    திரும்பு SQLITE_IOERR_SHORT_READ;
  }
  திரும்பு SQLITE_OK;
}


/*
** Write into the -journal file.
*/
நிலையான முழு kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  மாறிலி வெறுமை *zBuf,
  முழு iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  எனில்( iEnd>=0x10000000 ) திரும்பு SQLITE_FULL;
  எனில்( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    எழுத்து *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    எனில்( aNew==0 ){
      திரும்பு SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    எனில்( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  திரும்பு SQLITE_OK;
}

/*
** Write into the database file.
*/
நிலையான முழு kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  மாறிலி வெறுமை *zBuf,
  முழு iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned முழு pgno;
  எழுத்து z_சாவி[30];
  எழுத்து *aData = pFile->aData;
  முழு rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ப்ரதிஜ்ஞா( iAmt>=512 && iAmt<=65536 );
  ப்ரதிஜ்ஞா( (iAmt & (iAmt-1))==0 );
  ப்ரதிஜ்ஞா( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(அளவு(z_சாவி), z_சாவி, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_சாவி, aData);
  எனில்( 0==rc ){
    எனில்( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  திரும்பு rc;
}

/*
** Truncate an kvvfs-file.
*/
நிலையான முழு kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  ப்ரதிஜ்ஞா( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  திரும்பு SQLITE_OK;
}
நிலையான முழு kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  எனில்( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    எழுத்து z_சாவி[50];
    unsigned முழு pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    போது( pgno<=pgnoMax ){
      sqlite3_snprintf(அளவு(z_சாவி), z_சாவி, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_சாவி);
      pgno++;
    }
    pFile->szDb = size;
    திரும்பு kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  திரும்பு SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
நிலையான முழு kvvfsSyncJrnl(sqlite3_file *pProtoFile, முழு flags){
  முழு i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  எழுத்து *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  எனில்( pFile->nJrnl<=0 ){
    திரும்பு kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  எனில்( zOut==0 ){
    திரும்பு SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  செய்{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }போது( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  திரும்பு i ? SQLITE_IOERR : SQLITE_OK;
}
நிலையான முழு kvvfsSyncDb(sqlite3_file *pProtoFile, முழு flags){
  திரும்பு SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
நிலையான முழு kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  திரும்பு SQLITE_OK;
}
நிலையான முழு kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  எனில்( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }இல்லையெனில்{
    *pSize = kvvfsReadFileSize(pFile);
  }
  திரும்பு SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
நிலையான முழு kvvfsLock(sqlite3_file *pProtoFile, முழு eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ப்ரதிஜ்ஞா( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  எனில்( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  திரும்பு SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
நிலையான முழு kvvfsUnlock(sqlite3_file *pProtoFile, முழு eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ப்ரதிஜ்ஞா( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  எனில்( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  திரும்பு SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
நிலையான முழு kvvfsCheckReservedLock(sqlite3_file *pProtoFile, முழு *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  திரும்பு SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
நிலையான முழு kvvfsFileControlJrnl(sqlite3_file *pProtoFile, முழு op, வெறுமை *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  திரும்பு SQLITE_NOTFOUND;
}
நிலையான முழு kvvfsFileControlDb(sqlite3_file *pProtoFile, முழு op, வெறுமை *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  எனில்( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    முழு rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    எனில்( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    திரும்பு rc;
  }
  திரும்பு SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
நிலையான முழு kvvfsSectorSize(sqlite3_file *pFile){
  திரும்பு 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
நிலையான முழு kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  திரும்பு 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
நிலையான முழு kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  மாறிலி எழுத்து *zName,
  sqlite3_file *pProtoFile,
  முழு flags,
  முழு *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  எனில்( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  ப்ரதிஜ்ஞா(!pFile->zClass);
  ப்ரதிஜ்ஞா(!pFile->aData);
  ப்ரதிஜ்ஞா(!pFile->aJrnl);
  ப்ரதிஜ்ஞா(!pFile->nJrnl);
  ப்ரதிஜ்ஞா(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  எனில்( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    எனில்( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }இல்லையெனில் எனில்( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }இல்லையெனில்{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  எனில்( !pFile->zClass ){
#ifdef SQLITE_WASM
    எனில்( strlen(zName) >= (KVRECORD_சாவி_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      திரும்பு SQLITE_CANTOPEN;
    }
#இல்லையெனில்
    எனில்( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      திரும்பு SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  எனில்( pFile->aData==0 ){
    திரும்பு SQLITE_NOMEM;
  }
  திரும்பு SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
நிலையான முழு kvvfsDelete(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath, முழு dirSync){
  முழு rc /* The JS impl can fail with OOM in argument conversion */;
  எனில்( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }இல்லையெனில்
  எனில்( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  இல்லையெனில்{
    rc = 0;
  }
  திரும்பு rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
நிலையான முழு kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  மாறிலி எழுத்து *zPath,
  முழு flags,
  முழு *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#எனில் 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  மாறிலி எழுத்து *z_சாவி = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_சாவி, 0, 0)>0;
#இல்லையெனில்
  எனில்( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }இல்லையெனில்
  எனில்( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }இல்லையெனில்
  எனில்( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }இல்லையெனில்
  எனில்( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }இல்லையெனில்
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  திரும்பு SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
நிலையான முழு kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zPath,
  முழு nOut,
  எழுத்து *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  எனில்( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  திரும்பு SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
நிலையான வெறுமை *kvvfsDlOpen(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath){
  திரும்பு 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
நிலையான முழு kvvfsRandomness(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *zBufOut){
  memset(zBufOut, 0, nByte);
  திரும்பு nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
நிலையான முழு kvvfsSleep(sqlite3_vfs *pVfs, முழு nMicro){
  திரும்பு SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
நிலையான முழு kvvfsCurrentTime(sqlite3_vfs *pVfs, இரட்டை *pTimeOut){
  sqlite3_int64 i = 0;
  முழு rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  திரும்பு rc;
}
#அந்தர்பாவ <sys/time.h>
நிலையான முழு kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  நிலையான மாறிலி sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  கட்டமைப்பு timeval sNow;
  (வெறுமை)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  திரும்பு SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#எனில் SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
முழு sqlite3_os_init(வெறுமை){
  திரும்பு sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
முழு sqlite3_os_end(வெறுமை){
  திரும்பு SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#எனில் SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
முழு sqlite3KvvfsInit(வெறுமை){
  திரும்பு sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
