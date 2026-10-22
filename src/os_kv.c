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
#ଅନ୍ତର୍ଭାଵ <sqliteInt.h>
#ଯଦି SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#ଯଦି 0
#define SQLITE_KV_TRACE(X)  printf X
#ଅନ୍ୟଥା
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#ଯଦି 0
#define SQLITE_KV_LOG(X)  printf X
#ଅନ୍ୟଥା
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ସଂରଚନା KVVfsFile {
  sqlite3_file base;              /* IO methods */
  ଅଚଳ ଅକ୍ଷର *zClass;             /* Storage class */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isJournal;                  /* True if this is a journal file */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nJrnl;             /* Space allocated for aJrnl[] */
  ଅକ୍ଷର *aJrnl;                    /* Journal content */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  ଅକ୍ଷର *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsClose(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsReadDb(sqlite3_file*, ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsReadJrnl(sqlite3_file*, ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsWriteDb(sqlite3_file*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsWriteJrnl(sqlite3_file*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsSyncDb(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsSyncJrnl(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsUnlock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsCheckReservedLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFileControlDb(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFileControlJrnl(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsSectorSize(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *, sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା , ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsDelete(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା syncDir);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsAccess(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFullPathname(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅକ୍ଷର *zOut);
ସ୍ଥିର ଶୂନ୍ୟ *kvvfsDlOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zFilename);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsRandomness(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsSleep(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା microseconds);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsCurrentTime(sqlite3_vfs*, ଦ୍ୱିଗୁଣ*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ସ୍ଥିର sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  ଆକାର(KVVfsFile),              /* szOsFile */
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
ସ୍ଥିର sqlite3_io_methods kvvfs_db_io_methods = {
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
ସ୍ଥିର sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvrecordWrite(ଅଚଳ ଅକ୍ଷର*, ଅଚଳ ଅକ୍ଷର *z_ଚାବି, ଅଚଳ ଅକ୍ଷର *zData);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvrecordDelete(ଅଚଳ ଅକ୍ଷର*, ଅଚଳ ଅକ୍ଷର *z_ଚାବି);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvrecordRead(ଅଚଳ ଅକ୍ଷର*, ଅଚଳ ଅକ୍ଷର *z_ଚାବି, ଅକ୍ଷର *zBuf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf);
#endif
#ifndef KVRECORD_ଚାବି_SZ
#define KVRECORD_ଚାବି_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ସ୍ଥିର ଶୂନ୍ୟ kvrecord_Make_ଚାବି(
  ଅଚଳ ଅକ୍ଷର *zClass,
  ଅଚଳ ଅକ୍ଷର *z_ଚାବି_In,
  ଅକ୍ଷର *z_ଚାବି_Out
){
  ପ୍ରତିଜ୍ଞା( z_ଚାବି_In );
  ପ୍ରତିଜ୍ଞା( z_ଚାବି_Out );
  ପ୍ରତିଜ୍ଞା( zClass );
  sqlite3_snprintf(KVRECORD_ଚାବି_SZ, z_ଚାବି_Out, "kvvfs-%s-%s",
                   zClass, z_ଚାବି_In);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvrecordWrite(
  ଅଚଳ ଅକ୍ଷର *zClass,
  ଅଚଳ ଅକ୍ଷର *z_ଚାବି,
  ଅଚଳ ଅକ୍ଷର *zData
){
  FILE *fd;
  ଅକ୍ଷର zXKey[KVRECORD_ଚାବି_SZ];
  kvrecord_Make_ଚାବି(zClass, z_ଚାବି, zXKey);
  fd = fopen(zXKey, "wb");
  ଯଦି( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvrecordDelete(ଅଚଳ ଅକ୍ଷର *zClass, ଅଚଳ ଅକ୍ଷର *z_ଚାବି){
  ଅକ୍ଷର zXKey[KVRECORD_ଚାବି_SZ];
  kvrecord_Make_ଚାବି(zClass, z_ଚାବି, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvrecordRead(
  ଅଚଳ ଅକ୍ଷର *zClass,
  ଅଚଳ ଅକ୍ଷର *z_ଚାବି,
  ଅକ୍ଷର *zBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf
){
  FILE *fd;
  ସଂରଚନା stat buf;
  ଅକ୍ଷର zXKey[KVRECORD_ଚାବି_SZ];
  kvrecord_Make_ଚାବି(zClass, z_ଚାବି, zXKey);
  ଯଦି( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ଫେରନ୍ତୁ -1;
  }
  ଯଦି( nBuf<=0 ){
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)buf.st_size;
  }ଅନ୍ୟଥା ଯଦି( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)buf.st_size));
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)buf.st_size;
  }
  ଯଦି( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  ଯଦି( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ଫେରନ୍ତୁ -1;
  }ଅନ୍ୟଥା{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
ସଂରଚନା sqlite3_kvvfs_methods {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xRcrdRead)(ଅଚଳ ଅକ୍ଷର*, ଅଚଳ ଅକ୍ଷର *z_ଚାବି, ଅକ୍ଷର *zBuf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xRcrdWrite)(ଅଚଳ ଅକ୍ଷର*, ଅଚଳ ଅକ୍ଷର *z_ଚାବି, ଅଚଳ ଅକ୍ଷର *zData);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xRcrdDelete)(ଅଚଳ ଅକ୍ଷର*, ଅଚଳ ଅକ୍ଷର *z_ଚାବି);
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି_Size;
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST ଅଚଳ
#ଅନ୍ୟଥା
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
ଅଚଳ
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#ଅନ୍ୟଥା
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .n_ଚାବି_Size        = KVRECORD_ଚାବି_SZ,
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
ସ୍ଥିର
#endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsEncode(ଅଚଳ ଅକ୍ଷର *aData, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData, ଅକ୍ଷର *aOut){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ଅଚଳ unsigned ଅକ୍ଷର *a = (ଅଚଳ unsigned ଅକ୍ଷର*)aData;
  ପାଇଁ(i=j=0; i<nData; i++){
    unsigned ଅକ୍ଷର c = a[i];
    ଯଦି( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }ଅନ୍ୟଥା{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା k;
      ପାଇଁ(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      ଯେତେବେଳେ( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  ଫେରନ୍ତୁ j;
}

ସ୍ଥିର ଅଚଳ signed ଅକ୍ଷର kvvfs_Hex_ମୂଲ୍ୟ[256] = {
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
ସ୍ଥିର
#endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsDecode(ଅଚଳ ଅକ୍ଷର *a, ଅକ୍ଷର *aOut, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
  ଅଚଳ unsigned ଅକ୍ଷର *aIn = (ଅଚଳ unsigned ଅକ୍ଷର*)a;
  i = 0;
  j = 0;
  ଯେତେବେଳେ( 1 ){
    c = kvvfs_Hex_ମୂଲ୍ୟ[aIn[i]];
    ଯଦି( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      ଯଦି( c==0 ) ଭାଙ୍ଗ;
      ଯେତେବେଳେ( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        ଯଦି( n>nOut ) ଫେରନ୍ତୁ -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      ଯଦି( j+n>nOut ) ଫେରନ୍ତୁ -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      ଯଦି( c==0 || mult==1 ) ଭାଙ୍ଗ; /* progress stalled if mult==1 */
    }ଅନ୍ୟଥା ଯଦି( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_ମୂଲ୍ୟ[aIn[++i]];
      ଯଦି( c<0 ) ଫେରନ୍ତୁ -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }ଅନ୍ୟଥା{
      ଫେରନ୍ତୁ -1;
    }
  }
  ଫେରନ୍ତୁ j;
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
ସ୍ଥିର ଶୂନ୍ୟ kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ଅଚଳ ଅକ୍ଷର *zTxt,      /* Text encoding.  Zero-terminated */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0, mult;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ଯେତେବେଳେ( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  ଯଦି( ' '!=zTxt[i++] ){
    /* Malformed input */
    ଫେରନ୍ତୁ;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  ଯଦି( pFile->aJrnl==0 ){
    ଫେରନ୍ତୁ;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  ଯଦି( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
ସ୍ଥିର sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  ଅକ୍ଷର zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                ଆକାର(zData)-1);
  ଫେରନ୍ତୁ strtoll(zData, 0, 0);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  ଅକ୍ଷର zData[50];
  sqlite3_snprintf(ଆକାର(zData), zData, "%lld", sz);
  ଫେରନ୍ତୁ sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, ଆକାର(*pFile));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Read from the -journal file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  ଶୂନ୍ୟ *zBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ପ୍ରତିଜ୍ଞା( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ଯଦି( pFile->aJrnl==0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    ଅକ୍ଷର *aTxt;
    ଯଦି( szTxt<=4 ){
      ଫେରନ୍ତୁ SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    ଯଦି( aTxt==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    ଯଦି( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    ଯଦି( rc ) ଫେରନ୍ତୁ rc;
    ଯଦି( pFile->aJrnl==0 ) ଫେରନ୍ତୁ SQLITE_IOERR;
  }
  ଯଦି( iOfst+iAmt>pFile->nJrnl ){
    ଫେରନ୍ତୁ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Read from the database file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsReadDb(
  sqlite3_file *pProtoFile,
  ଶୂନ୍ୟ *zBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgno;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା got, n;
  ଅକ୍ଷର z_ଚାବି[30];
  ଅକ୍ଷର *aData = pFile->aData;
  ପ୍ରତିଜ୍ଞା( iOfst>=0 );
  ପ୍ରତିଜ୍ଞା( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ଯଦି( iOfst+iAmt>=512 ){
    ଯଦି( (iOfst % iAmt)!=0 ){
      ଫେରନ୍ତୁ SQLITE_IOERR_READ;
    }
    ଯଦି( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      ଫେରନ୍ତୁ SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }ଅନ୍ୟଥା{
    pgno = 1;
  }
  sqlite3_snprintf(ଆକାର(z_ଚାବି), z_ଚାବି, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_ଚାବି,
                                      aData, SQLITE_KVOS_SZ-1);
  ଯଦି( got<0 ){
    n = 0;
  }ଅନ୍ୟଥା{
    aData[got] = 0;
    ଯଦି( iOfst+iAmt<512 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      ଯଦି( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }ଅନ୍ୟଥା{
        n = 0;
      }
    }ଅନ୍ୟଥା{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  ଯଦି( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    ଫେରନ୍ତୁ SQLITE_IOERR_SHORT_READ;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}


/*
** Write into the -journal file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ଅଚଳ ଶୂନ୍ୟ *zBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ଯଦି( iEnd>=0x10000000 ) ଫେରନ୍ତୁ SQLITE_FULL;
  ଯଦି( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    ଅକ୍ଷର *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    ଯଦି( aNew==0 ){
      ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    ଯଦି( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Write into the database file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ଅଚଳ ଶୂନ୍ୟ *zBuf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgno;
  ଅକ୍ଷର z_ଚାବି[30];
  ଅକ୍ଷର *aData = pFile->aData;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ପ୍ରତିଜ୍ଞା( iAmt>=512 && iAmt<=65536 );
  ପ୍ରତିଜ୍ଞା( (iAmt & (iAmt-1))==0 );
  ପ୍ରତିଜ୍ଞା( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(ଆକାର(z_ଚାବି), z_ଚାବି, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_ଚାବି, aData);
  ଯଦି( 0==rc ){
    ଯଦି( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Truncate an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  ପ୍ରତିଜ୍ଞା( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ଯଦି( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    ଅକ୍ଷର z_ଚାବି[50];
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    ଯେତେବେଳେ( pgno<=pgnoMax ){
      sqlite3_snprintf(ଆକାର(z_ଚାବି), z_ଚାବି, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_ଚାବି);
      pgno++;
    }
    pFile->szDb = size;
    ଫେରନ୍ତୁ kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  ଫେରନ୍ତୁ SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsSyncJrnl(sqlite3_file *pProtoFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ଅକ୍ଷର *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  ଯଦି( pFile->nJrnl<=0 ){
    ଫେରନ୍ତୁ kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  ଯଦି( zOut==0 ){
    ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  କର{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }ଯେତେବେଳେ( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  ଫେରନ୍ତୁ i ? SQLITE_IOERR : SQLITE_OK;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsSyncDb(sqlite3_file *pProtoFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  ଫେରନ୍ତୁ SQLITE_OK;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  ଯଦି( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }ଅନ୍ୟଥା{
    *pSize = kvvfsReadFileSize(pFile);
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsLock(sqlite3_file *pProtoFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ପ୍ରତିଜ୍ଞା( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  ଯଦି( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsUnlock(sqlite3_file *pProtoFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ପ୍ରତିଜ୍ଞା( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  ଯଦି( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsCheckReservedLock(sqlite3_file *pProtoFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFileControlJrnl(sqlite3_file *pProtoFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  ଫେରନ୍ତୁ SQLITE_NOTFOUND;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFileControlDb(sqlite3_file *pProtoFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  ଯଦି( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    ଯଦି( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    ଫେରନ୍ତୁ rc;
  }
  ଫେରନ୍ତୁ SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsSectorSize(sqlite3_file *pFile){
  ଫେରନ୍ତୁ 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  ଫେରନ୍ତୁ 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ଅଚଳ ଅକ୍ଷର *zName,
  sqlite3_file *pProtoFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ଯଦି( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  ପ୍ରତିଜ୍ଞା(!pFile->zClass);
  ପ୍ରତିଜ୍ଞା(!pFile->aData);
  ପ୍ରତିଜ୍ଞା(!pFile->aJrnl);
  ପ୍ରତିଜ୍ଞା(!pFile->nJrnl);
  ପ୍ରତିଜ୍ଞା(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  ଯଦି( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    ଯଦି( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }ଅନ୍ୟଥା ଯଦି( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }ଅନ୍ୟଥା{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  ଯଦି( !pFile->zClass ){
#ifdef SQLITE_WASM
    ଯଦି( strlen(zName) >= (KVRECORD_ଚାବି_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      ଫେରନ୍ତୁ SQLITE_CANTOPEN;
    }
#ଅନ୍ୟଥା
    ଯଦି( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      ଫେରନ୍ତୁ SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  ଯଦି( pFile->aData==0 ){
    ଫେରନ୍ତୁ SQLITE_NOMEM;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsDelete(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirSync){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc /* The JS impl can fail with OOM in argument conversion */;
  ଯଦି( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }ଅନ୍ୟଥା
  ଯଦି( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  ଅନ୍ୟଥା{
    rc = 0;
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ଅଚଳ ଅକ୍ଷର *zPath,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#ଯଦି 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ଅଚଳ ଅକ୍ଷର *z_ଚାବି = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_ଚାବି, 0, 0)>0;
#ଅନ୍ୟଥା
  ଯଦି( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }ଅନ୍ୟଥା
  ଯଦି( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }ଅନ୍ୟଥା
  ଯଦି( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }ଅନ୍ୟଥା
  ଯଦି( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }ଅନ୍ୟଥା
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zPath,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut,
  ଅକ୍ଷର *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  ଯଦି( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ସ୍ଥିର ଶୂନ୍ୟ *kvvfsDlOpen(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath){
  ଫେରନ୍ତୁ 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsRandomness(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zBufOut){
  memset(zBufOut, 0, nByte);
  ଫେରନ୍ତୁ nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsSleep(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMicro){
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsCurrentTime(sqlite3_vfs *pVfs, ଦ୍ୱିଗୁଣ *pTimeOut){
  sqlite3_int64 i = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  ଫେରନ୍ତୁ rc;
}
#ଅନ୍ତର୍ଭାଵ <sys/time.h>
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ସ୍ଥିର ଅଚଳ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ସଂରଚନା timeval sNow;
  (ଶୂନ୍ୟ)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  ଫେରନ୍ତୁ SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#ଯଦି SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_os_init(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_os_end(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#ଯଦି SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3KvvfsInit(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
