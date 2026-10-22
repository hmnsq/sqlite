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
#ಅನ್ತರ್ಭಾವ <sqliteInt.h>
#ಯದಿ SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#ಯದಿ 0
#define SQLITE_KV_TRACE(X)  printf X
#ಅನ್ಯಥಾ
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#ಯದಿ 0
#define SQLITE_KV_LOG(X)  printf X
#ಅನ್ಯಥಾ
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ಸಂರಚನಾ KVVfsFile {
  sqlite3_file base;              /* IO methods */
  ಧ್ರುವ ವರ್ಣ *zClass;             /* Storage class */
  ಪೂರ್ಣಾಙ್ಕ isJournal;                  /* True if this is a journal file */
  unsigned ಪೂರ್ಣಾಙ್ಕ nJrnl;             /* Space allocated for aJrnl[] */
  ವರ್ಣ *aJrnl;                    /* Journal content */
  ಪೂರ್ಣಾಙ್ಕ szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  ವರ್ಣ *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsReadDb(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsReadJrnl(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsWriteDb(sqlite3_file*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsWriteJrnl(sqlite3_file*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsSyncDb(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsSyncJrnl(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsUnlock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsCheckReservedLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ *pResOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFileControlDb(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFileControlJrnl(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsSectorSize(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *, sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ , ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsDelete(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsAccess(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFullPathname(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಶೂನ್ಯ *kvvfsDlOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zFilename);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsSleep(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsCurrentTime(sqlite3_vfs*, ದ್ವಿಗುಣ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ಸ್ಥಿರ sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  ಪರಿಮಾಣ(KVVfsFile),              /* szOsFile */
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
ಸ್ಥಿರ sqlite3_io_methods kvvfs_db_io_methods = {
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
ಸ್ಥಿರ sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvrecordWrite(ಧ್ರುವ ವರ್ಣ*, ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ, ಧ್ರುವ ವರ್ಣ *zData);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvrecordDelete(ಧ್ರುವ ವರ್ಣ*, ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvrecordRead(ಧ್ರುವ ವರ್ಣ*, ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ, ವರ್ಣ *zBuf, ಪೂರ್ಣಾಙ್ಕ nBuf);
#endif
#ifndef KVRECORD_ಕೀಲಿ_SZ
#define KVRECORD_ಕೀಲಿ_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ಸ್ಥಿರ ಶೂನ್ಯ kvrecord_Make_ಕೀಲಿ(
  ಧ್ರುವ ವರ್ಣ *zClass,
  ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ_In,
  ವರ್ಣ *z_ಕೀಲಿ_Out
){
  ಪ್ರತಿಜ್ಞಾ( z_ಕೀಲಿ_In );
  ಪ್ರತಿಜ್ಞಾ( z_ಕೀಲಿ_Out );
  ಪ್ರತಿಜ್ಞಾ( zClass );
  sqlite3_snprintf(KVRECORD_ಕೀಲಿ_SZ, z_ಕೀಲಿ_Out, "kvvfs-%s-%s",
                   zClass, z_ಕೀಲಿ_In);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvrecordWrite(
  ಧ್ರುವ ವರ್ಣ *zClass,
  ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ,
  ಧ್ರುವ ವರ್ಣ *zData
){
  FILE *fd;
  ವರ್ಣ zXKey[KVRECORD_ಕೀಲಿ_SZ];
  kvrecord_Make_ಕೀಲಿ(zClass, z_ಕೀಲಿ, zXKey);
  fd = fopen(zXKey, "wb");
  ಯದಿ( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (ಪೂರ್ಣಾಙ್ಕ)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvrecordDelete(ಧ್ರುವ ವರ್ಣ *zClass, ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ){
  ವರ್ಣ zXKey[KVRECORD_ಕೀಲಿ_SZ];
  kvrecord_Make_ಕೀಲಿ(zClass, z_ಕೀಲಿ, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvrecordRead(
  ಧ್ರುವ ವರ್ಣ *zClass,
  ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ,
  ವರ್ಣ *zBuf,
  ಪೂರ್ಣಾಙ್ಕ nBuf
){
  FILE *fd;
  ಸಂರಚನಾ stat buf;
  ವರ್ಣ zXKey[KVRECORD_ಕೀಲಿ_SZ];
  kvrecord_Make_ಕೀಲಿ(zClass, z_ಕೀಲಿ, zXKey);
  ಯದಿ( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಯದಿ( nBuf<=0 ){
    ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)buf.st_size;
  }ಅನ್ಯಥಾ ಯದಿ( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (ಪೂರ್ಣಾಙ್ಕ)buf.st_size));
    ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)buf.st_size;
  }
  ಯದಿ( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  ಯದಿ( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ಪ್ರತ್ಯಾಗಮ -1;
  }ಅನ್ಯಥಾ{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)n;
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
ಸಂರಚನಾ sqlite3_kvvfs_methods {
  ಪೂರ್ಣಾಙ್ಕ (*xRcrdRead)(ಧ್ರುವ ವರ್ಣ*, ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ, ವರ್ಣ *zBuf, ಪೂರ್ಣಾಙ್ಕ nBuf);
  ಪೂರ್ಣಾಙ್ಕ (*xRcrdWrite)(ಧ್ರುವ ವರ್ಣ*, ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ, ಧ್ರುವ ವರ್ಣ *zData);
  ಪೂರ್ಣಾಙ್ಕ (*xRcrdDelete)(ಧ್ರುವ ವರ್ಣ*, ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ);
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ_Size;
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST ಧ್ರುವ
#ಅನ್ಯಥಾ
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
ಧ್ರುವ
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#ಅನ್ಯಥಾ
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .n_ಕೀಲಿ_Size        = KVRECORD_ಕೀಲಿ_SZ,
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
ಸ್ಥಿರ
#endif
ಪೂರ್ಣಾಙ್ಕ kvvfsEncode(ಧ್ರುವ ವರ್ಣ *aData, ಪೂರ್ಣಾಙ್ಕ nData, ವರ್ಣ *aOut){
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಧ್ರುವ unsigned ವರ್ಣ *a = (ಧ್ರುವ unsigned ವರ್ಣ*)aData;
  ಪ್ರತಿ(i=j=0; i<nData; i++){
    unsigned ವರ್ಣ c = a[i];
    ಯದಿ( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }ಅನ್ಯಥಾ{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      ಪೂರ್ಣಾಙ್ಕ k;
      ಪ್ರತಿ(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      ಯಾವತ್( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  ಪ್ರತ್ಯಾಗಮ j;
}

ಸ್ಥಿರ ಧ್ರುವ signed ವರ್ಣ kvvfs_Hex_ಮೌಲ್ಯ[256] = {
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
ಸ್ಥಿರ
#endif
ಪೂರ್ಣಾಙ್ಕ kvvfsDecode(ಧ್ರುವ ವರ್ಣ *a, ವರ್ಣ *aOut, ಪೂರ್ಣಾಙ್ಕ nOut){
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಪೂರ್ಣಾಙ್ಕ c;
  ಧ್ರುವ unsigned ವರ್ಣ *aIn = (ಧ್ರುವ unsigned ವರ್ಣ*)a;
  i = 0;
  j = 0;
  ಯಾವತ್( 1 ){
    c = kvvfs_Hex_ಮೌಲ್ಯ[aIn[i]];
    ಯದಿ( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      ಯದಿ( c==0 ) ಭಙ್ಗ;
      ಯಾವತ್( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        ಯದಿ( n>nOut ) ಪ್ರತ್ಯಾಗಮ -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      ಯದಿ( j+n>nOut ) ಪ್ರತ್ಯಾಗಮ -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      ಯದಿ( c==0 || mult==1 ) ಭಙ್ಗ; /* progress stalled if mult==1 */
    }ಅನ್ಯಥಾ ಯದಿ( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_ಮೌಲ್ಯ[aIn[++i]];
      ಯದಿ( c<0 ) ಪ್ರತ್ಯಾಗಮ -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }ಅನ್ಯಥಾ{
      ಪ್ರತ್ಯಾಗಮ -1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ j;
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
ಸ್ಥಿರ ಶೂನ್ಯ kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ಧ್ರುವ ವರ್ಣ *zTxt,      /* Text encoding.  Zero-terminated */
  ಪೂರ್ಣಾಙ್ಕ nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned ಪೂರ್ಣಾಙ್ಕ n = 0, mult;
  ಪೂರ್ಣಾಙ್ಕ c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ಯಾವತ್( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  ಯದಿ( ' '!=zTxt[i++] ){
    /* Malformed input */
    ಪ್ರತ್ಯಾಗಮ;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  ಯದಿ( pFile->aJrnl==0 ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  ಯದಿ( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
ಸ್ಥಿರ sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  ವರ್ಣ zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                ಪರಿಮಾಣ(zData)-1);
  ಪ್ರತ್ಯಾಗಮ strtoll(zData, 0, 0);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  ವರ್ಣ zData[50];
  sqlite3_snprintf(ಪರಿಮಾಣ(zData), zData, "%lld", sz);
  ಪ್ರತ್ಯಾಗಮ sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, ಪರಿಮಾಣ(*pFile));
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Read from the -journal file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ಪ್ರತಿಜ್ಞಾ( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ಯದಿ( pFile->aJrnl==0 ){
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಪೂರ್ಣಾಙ್ಕ szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    ವರ್ಣ *aTxt;
    ಯದಿ( szTxt<=4 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    ಯದಿ( aTxt==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    ಯದಿ( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
    ಯದಿ( pFile->aJrnl==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR;
  }
  ಯದಿ( iOfst+iAmt>pFile->nJrnl ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Read from the database file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsReadDb(
  sqlite3_file *pProtoFile,
  ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned ಪೂರ್ಣಾಙ್ಕ pgno;
  ಪೂರ್ಣಾಙ್ಕ got, n;
  ವರ್ಣ z_ಕೀಲಿ[30];
  ವರ್ಣ *aData = pFile->aData;
  ಪ್ರತಿಜ್ಞಾ( iOfst>=0 );
  ಪ್ರತಿಜ್ಞಾ( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ಯದಿ( iOfst+iAmt>=512 ){
    ಯದಿ( (iOfst % iAmt)!=0 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_READ;
    }
    ಯದಿ( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }ಅನ್ಯಥಾ{
    pgno = 1;
  }
  sqlite3_snprintf(ಪರಿಮಾಣ(z_ಕೀಲಿ), z_ಕೀಲಿ, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_ಕೀಲಿ,
                                      aData, SQLITE_KVOS_SZ-1);
  ಯದಿ( got<0 ){
    n = 0;
  }ಅನ್ಯಥಾ{
    aData[got] = 0;
    ಯದಿ( iOfst+iAmt<512 ){
      ಪೂರ್ಣಾಙ್ಕ k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      ಯದಿ( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }ಅನ್ಯಥಾ{
        n = 0;
      }
    }ಅನ್ಯಥಾ{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  ಯದಿ( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_SHORT_READ;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}


/*
** Write into the -journal file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ಧ್ರುವ ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ಯದಿ( iEnd>=0x10000000 ) ಪ್ರತ್ಯಾಗಮ SQLITE_FULL;
  ಯದಿ( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    ವರ್ಣ *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    ಯದಿ( aNew==0 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    ಯದಿ( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Write into the database file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ಧ್ರುವ ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned ಪೂರ್ಣಾಙ್ಕ pgno;
  ವರ್ಣ z_ಕೀಲಿ[30];
  ವರ್ಣ *aData = pFile->aData;
  ಪೂರ್ಣಾಙ್ಕ rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ಪ್ರತಿಜ್ಞಾ( iAmt>=512 && iAmt<=65536 );
  ಪ್ರತಿಜ್ಞಾ( (iAmt & (iAmt-1))==0 );
  ಪ್ರತಿಜ್ಞಾ( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(ಪರಿಮಾಣ(z_ಕೀಲಿ), z_ಕೀಲಿ, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_ಕೀಲಿ, aData);
  ಯದಿ( 0==rc ){
    ಯದಿ( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Truncate an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  ಪ್ರತಿಜ್ಞಾ( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ಯದಿ( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    ವರ್ಣ z_ಕೀಲಿ[50];
    unsigned ಪೂರ್ಣಾಙ್ಕ pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    ಯಾವತ್( pgno<=pgnoMax ){
      sqlite3_snprintf(ಪರಿಮಾಣ(z_ಕೀಲಿ), z_ಕೀಲಿ, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_ಕೀಲಿ);
      pgno++;
    }
    pFile->szDb = size;
    ಪ್ರತ್ಯಾಗಮ kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsSyncJrnl(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪೂರ್ಣಾಙ್ಕ i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ವರ್ಣ *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  ಯದಿ( pFile->nJrnl<=0 ){
    ಪ್ರತ್ಯಾಗಮ kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  ಯದಿ( zOut==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  ಕುರು{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }ಯಾವತ್( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  ಪ್ರತ್ಯಾಗಮ i ? SQLITE_IOERR : SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsSyncDb(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  ಯದಿ( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }ಅನ್ಯಥಾ{
    *pSize = kvvfsReadFileSize(pFile);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsLock(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ಪ್ರತಿಜ್ಞಾ( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  ಯದಿ( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsUnlock(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ಪ್ರತಿಜ್ಞಾ( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  ಯದಿ( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsCheckReservedLock(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFileControlJrnl(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  ಪ್ರತ್ಯಾಗಮ SQLITE_NOTFOUND;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFileControlDb(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  ಯದಿ( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    ಯದಿ( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    ಪ್ರತ್ಯಾಗಮ rc;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsSectorSize(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  ಪ್ರತ್ಯಾಗಮ 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_file *pProtoFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ಯದಿ( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  ಪ್ರತಿಜ್ಞಾ(!pFile->zClass);
  ಪ್ರತಿಜ್ಞಾ(!pFile->aData);
  ಪ್ರತಿಜ್ಞಾ(!pFile->aJrnl);
  ಪ್ರತಿಜ್ಞಾ(!pFile->nJrnl);
  ಪ್ರತಿಜ್ಞಾ(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  ಯದಿ( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    ಯದಿ( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }ಅನ್ಯಥಾ ಯದಿ( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }ಅನ್ಯಥಾ{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  ಯದಿ( !pFile->zClass ){
#ifdef SQLITE_WASM
    ಯದಿ( strlen(zName) >= (KVRECORD_ಕೀಲಿ_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_CANTOPEN;
    }
#ಅನ್ಯಥಾ
    ಯದಿ( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      ಪ್ರತ್ಯಾಗಮ SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  ಯದಿ( pFile->aData==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsDelete(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ dirSync){
  ಪೂರ್ಣಾಙ್ಕ rc /* The JS impl can fail with OOM in argument conversion */;
  ಯದಿ( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }ಅನ್ಯಥಾ
  ಯದಿ( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  ಅನ್ಯಥಾ{
    rc = 0;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ಧ್ರುವ ವರ್ಣ *zPath,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#ಯದಿ 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ಧ್ರುವ ವರ್ಣ *z_ಕೀಲಿ = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_ಕೀಲಿ, 0, 0)>0;
#ಅನ್ಯಥಾ
  ಯದಿ( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }ಅನ್ಯಥಾ
  ಯದಿ( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }ಅನ್ಯಥಾ
  ಯದಿ( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }ಅನ್ಯಥಾ
  ಯದಿ( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }ಅನ್ಯಥಾ
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zPath,
  ಪೂರ್ಣಾಙ್ಕ nOut,
  ವರ್ಣ *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  ಯದಿ( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *kvvfsDlOpen(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath){
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  memset(zBufOut, 0, nByte);
  ಪ್ರತ್ಯಾಗಮ nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nMicro){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsCurrentTime(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *pTimeOut){
  sqlite3_int64 i = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  ಪ್ರತ್ಯಾಗಮ rc;
}
#ಅನ್ತರ್ಭಾವ <sys/time.h>
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  ಸ್ಥಿರ ಧ್ರುವ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ಸಂರಚನಾ timeval sNow;
  (ಶೂನ್ಯ)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#ಯದಿ SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_os_init(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_os_end(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#ಯದಿ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
ಪೂರ್ಣಾಙ್ಕ sqlite3KvvfsInit(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
