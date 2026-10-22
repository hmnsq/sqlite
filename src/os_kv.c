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
#অন্তর্ভাৱ <sqliteInt.h>
#যদি SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#যদি 0
#define SQLITE_KV_TRACE(X)  printf X
#অন্যথা
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#যদি 0
#define SQLITE_KV_LOG(X)  printf X
#অন্যথা
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
প্রকারনির্দেশ সংরচনা KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
সংরচনা KVVfsFile {
  sqlite3_file base;              /* IO methods */
  ধ্রুৱ ৱর্ণ *zClass;             /* Storage class */
  পূর্ণাঙ্ক isJournal;                  /* True if this is a journal file */
  unsigned পূর্ণাঙ্ক nJrnl;             /* Space allocated for aJrnl[] */
  ৱর্ণ *aJrnl;                    /* Journal content */
  পূর্ণাঙ্ক szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  ৱর্ণ *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
স্থির পূর্ণাঙ্ক kvvfsClose(sqlite3_file*);
স্থির পূর্ণাঙ্ক kvvfsReadDb(sqlite3_file*, শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক kvvfsReadJrnl(sqlite3_file*, শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক kvvfsWriteDb(sqlite3_file*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক iAmt, sqlite3_int64);
স্থির পূর্ণাঙ্ক kvvfsWriteJrnl(sqlite3_file*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক iAmt, sqlite3_int64);
স্থির পূর্ণাঙ্ক kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
স্থির পূর্ণাঙ্ক kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
স্থির পূর্ণাঙ্ক kvvfsSyncDb(sqlite3_file*, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক kvvfsSyncJrnl(sqlite3_file*, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
স্থির পূর্ণাঙ্ক kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
স্থির পূর্ণাঙ্ক kvvfsLock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক kvvfsUnlock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক kvvfsCheckReservedLock(sqlite3_file*, পূর্ণাঙ্ক *pResOut);
স্থির পূর্ণাঙ্ক kvvfsFileControlDb(sqlite3_file*, পূর্ণাঙ্ক op, শূন্য *pArg);
স্থির পূর্ণাঙ্ক kvvfsFileControlJrnl(sqlite3_file*, পূর্ণাঙ্ক op, শূন্য *pArg);
স্থির পূর্ণাঙ্ক kvvfsSectorSize(sqlite3_file*);
স্থির পূর্ণাঙ্ক kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
স্থির পূর্ণাঙ্ক kvvfsOpen(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *, sqlite3_file*, পূর্ণাঙ্ক , পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক kvvfsDelete(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক syncDir);
স্থির পূর্ণাঙ্ক kvvfsAccess(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক kvvfsFullPathname(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক, ৱর্ণ *zOut);
স্থির শূন্য *kvvfsDlOpen(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zFilename);
স্থির পূর্ণাঙ্ক kvvfsRandomness(sqlite3_vfs*, পূর্ণাঙ্ক nByte, ৱর্ণ *zOut);
স্থির পূর্ণাঙ্ক kvvfsSleep(sqlite3_vfs*, পূর্ণাঙ্ক microseconds);
স্থির পূর্ণাঙ্ক kvvfsCurrentTime(sqlite3_vfs*, দ্ৱিগুণ*);
স্থির পূর্ণাঙ্ক kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

স্থির sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  পরিমাণ(KVVfsFile),              /* szOsFile */
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
স্থির sqlite3_io_methods kvvfs_db_io_methods = {
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
স্থির sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
স্থির পূর্ণাঙ্ক kvrecordWrite(ধ্রুৱ ৱর্ণ*, ধ্রুৱ ৱর্ণ *z_চাবি, ধ্রুৱ ৱর্ণ *zData);
স্থির পূর্ণাঙ্ক kvrecordDelete(ধ্রুৱ ৱর্ণ*, ধ্রুৱ ৱর্ণ *z_চাবি);
স্থির পূর্ণাঙ্ক kvrecordRead(ধ্রুৱ ৱর্ণ*, ধ্রুৱ ৱর্ণ *z_চাবি, ৱর্ণ *zBuf, পূর্ণাঙ্ক nBuf);
#endif
#ifndef KVRECORD_চাবি_SZ
#define KVRECORD_চাবি_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
স্থির শূন্য kvrecord_Make_চাবি(
  ধ্রুৱ ৱর্ণ *zClass,
  ধ্রুৱ ৱর্ণ *z_চাবি_In,
  ৱর্ণ *z_চাবি_Out
){
  প্রতিজ্ঞা( z_চাবি_In );
  প্রতিজ্ঞা( z_চাবি_Out );
  প্রতিজ্ঞা( zClass );
  sqlite3_snprintf(KVRECORD_চাবি_SZ, z_চাবি_Out, "kvvfs-%s-%s",
                   zClass, z_চাবি_In);
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
স্থির পূর্ণাঙ্ক kvrecordWrite(
  ধ্রুৱ ৱর্ণ *zClass,
  ধ্রুৱ ৱর্ণ *z_চাবি,
  ধ্রুৱ ৱর্ণ *zData
){
  FILE *fd;
  ৱর্ণ zXKey[KVRECORD_চাবি_SZ];
  kvrecord_Make_চাবি(zClass, z_চাবি, zXKey);
  fd = fopen(zXKey, "wb");
  যদি( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (পূর্ণাঙ্ক)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    ঘূৰাই 0;
  }অন্যথা{
    ঘূৰাই 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
স্থির পূর্ণাঙ্ক kvrecordDelete(ধ্রুৱ ৱর্ণ *zClass, ধ্রুৱ ৱর্ণ *z_চাবি){
  ৱর্ণ zXKey[KVRECORD_চাবি_SZ];
  kvrecord_Make_চাবি(zClass, z_চাবি, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  ঘূৰাই 0;
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
স্থির পূর্ণাঙ্ক kvrecordRead(
  ধ্রুৱ ৱর্ণ *zClass,
  ধ্রুৱ ৱর্ণ *z_চাবি,
  ৱর্ণ *zBuf,
  পূর্ণাঙ্ক nBuf
){
  FILE *fd;
  সংরচনা stat buf;
  ৱর্ণ zXKey[KVRECORD_চাবি_SZ];
  kvrecord_Make_চাবি(zClass, z_চাবি, zXKey);
  যদি( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ঘূৰাই -1;
  }
  যদি( nBuf<=0 ){
    ঘূৰাই (পূর্ণাঙ্ক)buf.st_size;
  }অন্যথা যদি( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (পূর্ণাঙ্ক)buf.st_size));
    ঘূৰাই (পূর্ণাঙ্ক)buf.st_size;
  }
  যদি( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  যদি( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ঘূৰাই -1;
  }অন্যথা{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    ঘূৰাই (পূর্ণাঙ্ক)n;
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
প্রকারনির্দেশ সংরচনা sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
সংরচনা sqlite3_kvvfs_methods {
  পূর্ণাঙ্ক (*xRcrdRead)(ধ্রুৱ ৱর্ণ*, ধ্রুৱ ৱর্ণ *z_চাবি, ৱর্ণ *zBuf, পূর্ণাঙ্ক nBuf);
  পূর্ণাঙ্ক (*xRcrdWrite)(ধ্রুৱ ৱর্ণ*, ধ্রুৱ ৱর্ণ *z_চাবি, ধ্রুৱ ৱর্ণ *zData);
  পূর্ণাঙ্ক (*xRcrdDelete)(ধ্রুৱ ৱর্ণ*, ধ্রুৱ ৱর্ণ *z_চাবি);
  ধ্রুৱ পূর্ণাঙ্ক n_চাবি_Size;
  ধ্রুৱ পূর্ণাঙ্ক nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST ধ্রুৱ
#অন্যথা
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
ধ্রুৱ
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#অন্যথা
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .n_চাবি_Size        = KVRECORD_চাবি_SZ,
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
স্থির
#endif
পূর্ণাঙ্ক kvvfsEncode(ধ্রুৱ ৱর্ণ *aData, পূর্ণাঙ্ক nData, ৱর্ণ *aOut){
  পূর্ণাঙ্ক i, j;
  ধ্রুৱ unsigned ৱর্ণ *a = (ধ্রুৱ unsigned ৱর্ণ*)aData;
  বাবে(i=j=0; i<nData; i++){
    unsigned ৱর্ণ c = a[i];
    যদি( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }অন্যথা{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      পূর্ণাঙ্ক k;
      বাবে(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      যেতিয়ালৈ( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  ঘূৰাই j;
}

স্থির ধ্রুৱ signed ৱর্ণ kvvfs_Hex_মান[256] = {
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
স্থির
#endif
পূর্ণাঙ্ক kvvfsDecode(ধ্রুৱ ৱর্ণ *a, ৱর্ণ *aOut, পূর্ণাঙ্ক nOut){
  পূর্ণাঙ্ক i, j;
  পূর্ণাঙ্ক c;
  ধ্রুৱ unsigned ৱর্ণ *aIn = (ধ্রুৱ unsigned ৱর্ণ*)a;
  i = 0;
  j = 0;
  যেতিয়ালৈ( 1 ){
    c = kvvfs_Hex_মান[aIn[i]];
    যদি( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      যদি( c==0 ) ভঙ্গ;
      যেতিয়ালৈ( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        যদি( n>nOut ) ঘূৰাই -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      যদি( j+n>nOut ) ঘূৰাই -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      যদি( c==0 || mult==1 ) ভঙ্গ; /* progress stalled if mult==1 */
    }অন্যথা যদি( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_মান[aIn[++i]];
      যদি( c<0 ) ঘূৰাই -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }অন্যথা{
      ঘূৰাই -1;
    }
  }
  ঘূৰাই j;
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
স্থির শূন্য kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ধ্রুৱ ৱর্ণ *zTxt,      /* Text encoding.  Zero-terminated */
  পূর্ণাঙ্ক nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned পূর্ণাঙ্ক n = 0, mult;
  পূর্ণাঙ্ক c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  যেতিয়ালৈ( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  যদি( ' '!=zTxt[i++] ){
    /* Malformed input */
    ঘূৰাই;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  যদি( pFile->aJrnl==0 ){
    ঘূৰাই;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  যদি( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
স্থির sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  ৱর্ণ zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                পরিমাণ(zData)-1);
  ঘূৰাই strtoll(zData, 0, 0);
}
স্থির পূর্ণাঙ্ক kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  ৱর্ণ zData[50];
  sqlite3_snprintf(পরিমাণ(zData), zData, "%lld", sz);
  ঘূৰাই sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, পরিমাণ(*pFile));
  ঘূৰাই SQLITE_OK;
}

/*
** Read from the -journal file.
*/
স্থির পূর্ণাঙ্ক kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  শূন্য *zBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  প্রতিজ্ঞা( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  যদি( pFile->aJrnl==0 ){
    পূর্ণাঙ্ক rc;
    পূর্ণাঙ্ক szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    ৱর্ণ *aTxt;
    যদি( szTxt<=4 ){
      ঘূৰাই SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    যদি( aTxt==0 ) ঘূৰাই SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    যদি( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    যদি( rc ) ঘূৰাই rc;
    যদি( pFile->aJrnl==0 ) ঘূৰাই SQLITE_IOERR;
  }
  যদি( iOfst+iAmt>pFile->nJrnl ){
    ঘূৰাই SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  ঘূৰাই SQLITE_OK;
}

/*
** Read from the database file.
*/
স্থির পূর্ণাঙ্ক kvvfsReadDb(
  sqlite3_file *pProtoFile,
  শূন্য *zBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned পূর্ণাঙ্ক pgno;
  পূর্ণাঙ্ক got, n;
  ৱর্ণ z_চাবি[30];
  ৱর্ণ *aData = pFile->aData;
  প্রতিজ্ঞা( iOfst>=0 );
  প্রতিজ্ঞা( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  যদি( iOfst+iAmt>=512 ){
    যদি( (iOfst % iAmt)!=0 ){
      ঘূৰাই SQLITE_IOERR_READ;
    }
    যদি( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      ঘূৰাই SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }অন্যথা{
    pgno = 1;
  }
  sqlite3_snprintf(পরিমাণ(z_চাবি), z_চাবি, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_চাবি,
                                      aData, SQLITE_KVOS_SZ-1);
  যদি( got<0 ){
    n = 0;
  }অন্যথা{
    aData[got] = 0;
    যদি( iOfst+iAmt<512 ){
      পূর্ণাঙ্ক k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      যদি( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }অন্যথা{
        n = 0;
      }
    }অন্যথা{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  যদি( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    ঘূৰাই SQLITE_IOERR_SHORT_READ;
  }
  ঘূৰাই SQLITE_OK;
}


/*
** Write into the -journal file.
*/
স্থির পূর্ণাঙ্ক kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ধ্রুৱ শূন্য *zBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  যদি( iEnd>=0x10000000 ) ঘূৰাই SQLITE_FULL;
  যদি( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    ৱর্ণ *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    যদি( aNew==0 ){
      ঘূৰাই SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    যদি( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  ঘূৰাই SQLITE_OK;
}

/*
** Write into the database file.
*/
স্থির পূর্ণাঙ্ক kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ধ্রুৱ শূন্য *zBuf,
  পূর্ণাঙ্ক iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned পূর্ণাঙ্ক pgno;
  ৱর্ণ z_চাবি[30];
  ৱর্ণ *aData = pFile->aData;
  পূর্ণাঙ্ক rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  প্রতিজ্ঞা( iAmt>=512 && iAmt<=65536 );
  প্রতিজ্ঞা( (iAmt & (iAmt-1))==0 );
  প্রতিজ্ঞা( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(পরিমাণ(z_চাবি), z_চাবি, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_চাবি, aData);
  যদি( 0==rc ){
    যদি( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  ঘূৰাই rc;
}

/*
** Truncate an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  প্রতিজ্ঞা( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ঘূৰাই SQLITE_OK;
}
স্থির পূর্ণাঙ্ক kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  যদি( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    ৱর্ণ z_চাবি[50];
    unsigned পূর্ণাঙ্ক pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    যেতিয়ালৈ( pgno<=pgnoMax ){
      sqlite3_snprintf(পরিমাণ(z_চাবি), z_চাবি, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_চাবি);
      pgno++;
    }
    pFile->szDb = size;
    ঘূৰাই kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  ঘূৰাই SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsSyncJrnl(sqlite3_file *pProtoFile, পূর্ণাঙ্ক flags){
  পূর্ণাঙ্ক i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ৱর্ণ *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  যদি( pFile->nJrnl<=0 ){
    ঘূৰাই kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  যদি( zOut==0 ){
    ঘূৰাই SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  কৰা{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }যেতিয়ালৈ( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  ঘূৰাই i ? SQLITE_IOERR : SQLITE_OK;
}
স্থির পূর্ণাঙ্ক kvvfsSyncDb(sqlite3_file *pProtoFile, পূর্ণাঙ্ক flags){
  ঘূৰাই SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  ঘূৰাই SQLITE_OK;
}
স্থির পূর্ণাঙ্ক kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  যদি( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }অন্যথা{
    *pSize = kvvfsReadFileSize(pFile);
  }
  ঘূৰাই SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsLock(sqlite3_file *pProtoFile, পূর্ণাঙ্ক eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  প্রতিজ্ঞা( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  যদি( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  ঘূৰাই SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsUnlock(sqlite3_file *pProtoFile, পূর্ণাঙ্ক eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  প্রতিজ্ঞা( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  যদি( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  ঘূৰাই SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsCheckReservedLock(sqlite3_file *pProtoFile, পূর্ণাঙ্ক *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  ঘূৰাই SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsFileControlJrnl(sqlite3_file *pProtoFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  ঘূৰাই SQLITE_NOTFOUND;
}
স্থির পূর্ণাঙ্ক kvvfsFileControlDb(sqlite3_file *pProtoFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  যদি( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    পূর্ণাঙ্ক rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    যদি( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    ঘূৰাই rc;
  }
  ঘূৰাই SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsSectorSize(sqlite3_file *pFile){
  ঘূৰাই 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
স্থির পূর্ণাঙ্ক kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  ঘূৰাই 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
স্থির পূর্ণাঙ্ক kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ধ্রুৱ ৱর্ণ *zName,
  sqlite3_file *pProtoFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  যদি( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  প্রতিজ্ঞা(!pFile->zClass);
  প্রতিজ্ঞা(!pFile->aData);
  প্রতিজ্ঞা(!pFile->aJrnl);
  প্রতিজ্ঞা(!pFile->nJrnl);
  প্রতিজ্ঞা(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  যদি( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    যদি( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }অন্যথা যদি( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }অন্যথা{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  যদি( !pFile->zClass ){
#ifdef SQLITE_WASM
    যদি( strlen(zName) >= (KVRECORD_চাবি_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      ঘূৰাই SQLITE_CANTOPEN;
    }
#অন্যথা
    যদি( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      ঘূৰাই SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  যদি( pFile->aData==0 ){
    ঘূৰাই SQLITE_NOMEM;
  }
  ঘূৰাই SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণাঙ্ক kvvfsDelete(sqlite3_vfs *pVfs, ধ্রুৱ ৱর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  পূর্ণাঙ্ক rc /* The JS impl can fail with OOM in argument conversion */;
  যদি( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }অন্যথা
  যদি( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  অন্যথা{
    rc = 0;
  }
  ঘূৰাই rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণাঙ্ক kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ধ্রুৱ ৱর্ণ *zPath,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#যদি 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ধ্রুৱ ৱর্ণ *z_চাবি = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_চাবি, 0, 0)>0;
#অন্যথা
  যদি( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }অন্যথা
  যদি( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }অন্যথা
  যদি( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }অন্যথা
  যদি( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }অন্যথা
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  ঘূৰাই SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণাঙ্ক kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  ধ্রুৱ ৱর্ণ *zPath,
  পূর্ণাঙ্ক nOut,
  ৱর্ণ *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  যদি( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  ঘূৰাই SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *kvvfsDlOpen(sqlite3_vfs *pVfs, ধ্রুৱ ৱর্ণ *zPath){
  ঘূৰাই 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
স্থির পূর্ণাঙ্ক kvvfsRandomness(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, ৱর্ণ *zBufOut){
  memset(zBufOut, 0, nByte);
  ঘূৰাই nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
স্থির পূর্ণাঙ্ক kvvfsSleep(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nMicro){
  ঘূৰাই SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণাঙ্ক kvvfsCurrentTime(sqlite3_vfs *pVfs, দ্ৱিগুণ *pTimeOut){
  sqlite3_int64 i = 0;
  পূর্ণাঙ্ক rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  ঘূৰাই rc;
}
#অন্তর্ভাৱ <sys/time.h>
স্থির পূর্ণাঙ্ক kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  স্থির ধ্রুৱ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  সংরচনা timeval sNow;
  (শূন্য)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  ঘূৰাই SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#যদি SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
পূর্ণাঙ্ক sqlite3_os_init(শূন্য){
  ঘূৰাই sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
পূর্ণাঙ্ক sqlite3_os_end(শূন্য){
  ঘূৰাই SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#যদি SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
পূর্ণাঙ্ক sqlite3KvvfsInit(শূন্য){
  ঘূৰাই sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
