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
#ਅਨ੍ਤਰ੍ਭਾਵ <sqliteInt.h>
#ਜੇ SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#ਜੇ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_KV_TRACE(X)  ਲਿਖੋ X
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_KV_TRACE(X)
#ਜੇ_ਅੰਤ

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#ਜੇ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_KV_LOG(X)  ਲਿਖੋ X
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_KV_LOG(X)
#ਜੇ_ਅੰਤ

/*
** Forward declaration of objects used by this VFS implementation
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ਬਣਤਰ KVVfsFile {
  sqlite3_file ਅਧਾਰ;              /* IO methods */
  ਅਚਲ ਅੱਖਰ *zClass;             /* Storage class */
  ਪੂਰਨ isJournal;                  /* True if this is a journal file */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nJrnl;             /* Space allocated for aJrnl[] */
  ਅੱਖਰ *aJrnl;                    /* Journal content */
  ਪੂਰਨ sz_ਪੰਨਾ;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  ਅੱਖਰ *a_ਅੰਕੜੇ;                    /* Buffer to hold page data */
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ਸਥਿਰ ਪੂਰਨ kvvfsClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ kvvfsReadDb(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ kvvfsReadJrnl(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ kvvfsWriteDb(sqlite3_file*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ iAmt, sqlite3_int64);
ਸਥਿਰ ਪੂਰਨ kvvfsWriteJrnl(sqlite3_file*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ iAmt, sqlite3_int64);
ਸਥਿਰ ਪੂਰਨ kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ kvvfsSyncDb(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ kvvfsSyncJrnl(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ kvvfs_File_ਮਾਪ_Db(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ kvvfs_File_ਮਾਪ_Jrnl(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ kvvfs_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ kvvfsUnlock(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ kvvfs_Check_Reserved_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ kvvfsFileControlDb(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
ਸਥਿਰ ਪੂਰਨ kvvfsFileControlJrnl(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
ਸਥਿਰ ਪੂਰਨ kvvfs_Sector_ਮਾਪ(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ਸਥਿਰ ਪੂਰਨ kvvfsOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *, sqlite3_file*, ਪੂਰਨ , ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ kvvfsDelete(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ syncDir);
ਸਥਿਰ ਪੂਰਨ kvvfsAccess(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ flags, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ kvvfs_ਭਰਿਆ_Pathname(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਖਾਲੀ *kvvfsDlOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zFilename);
ਸਥਿਰ ਪੂਰਨ kvvfsRandomness(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ kvvfsSleep(sqlite3_vfs*, ਪੂਰਨ microseconds);
ਸਥਿਰ ਪੂਰਨ kvvfs_Current_ਸਮਾਂ(sqlite3_vfs*, ਦੁੱਗਣਾ*);
ਸਥਿਰ ਪੂਰਨ kvvfs_Current_ਸਮਾਂ_Int64(sqlite3_vfs*, sqlite3_int64*);

ਸਥਿਰ sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  ਆਕਾਰ(KVVfsFile),              /* szOsFile */
  1024,                           /* mxPathname */
  0,                              /* pNext */
  "kvvfs",                        /* zName */
  0,                              /* pAppData */
  kvvfsOpen,                      /* xOpen */
  kvvfsDelete,                    /* xDelete */
  kvvfsAccess,                    /* xAccess */
  kvvfs_ਭਰਿਆ_Pathname,              /* xFullPathname */
  kvvfsDlOpen,                    /* xDlOpen */
  0,                              /* xDlError */
  0,                              /* xDlSym */
  0,                              /* xDlClose */
  kvvfsRandomness,                /* xRandomness */
  kvvfsSleep,                     /* xSleep */
  kvvfs_Current_ਸਮਾਂ,               /* xCurrentTime */
  0,                              /* xGetLastError */
  kvvfs_Current_ਸਮਾਂ_Int64           /* xCurrentTimeInt64 */
};

/* Methods for sqlite3_file objects referencing a database file
*/
ਸਥਿਰ sqlite3_io_methods kvvfs_db_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadDb,                    /* xRead */
  kvvfsWriteDb,                   /* xWrite */
  kvvfsTruncateDb,                /* xTruncate */
  kvvfsSyncDb,                    /* xSync */
  kvvfs_File_ਮਾਪ_Db,                /* xFileSize */
  kvvfs_ਤਾਲਾ,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_ਤਾਲਾ,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfs_Sector_ਮਾਪ,                /* xSectorSize */
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
ਸਥਿਰ sqlite3_io_methods kvvfs_jrnl_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadJrnl,                  /* xRead */
  kvvfsWriteJrnl,                 /* xWrite */
  kvvfsTruncateJrnl,              /* xTruncate */
  kvvfsSyncJrnl,                  /* xSync */
  kvvfs_File_ਮਾਪ_Jrnl,              /* xFileSize */
  kvvfs_ਤਾਲਾ,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_ਤਾਲਾ,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfs_Sector_ਮਾਪ,                /* xSectorSize */
  kvvfsDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                              /* xShmMap */
  0,                              /* xShmLock */
  0,                              /* xShmBarrier */
  0,                              /* xShmUnmap */
  0,                              /* xFetch */
  0                               /* xUnfetch */
};

/****** Storage subsystem **************************************************/
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ਸਥਿਰ ਪੂਰਨ kvrecordWrite(ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ, ਅਚਲ ਅੱਖਰ *z_ਅੰਕੜੇ);
ਸਥਿਰ ਪੂਰਨ kvrecordDelete(ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ);
ਸਥਿਰ ਪੂਰਨ kvrecordRead(ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ, ਅੱਖਰ *zBuf, ਪੂਰਨ nBuf);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ KVRECORD_ਕੁੰਜੀ_SZ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KVRECORD_ਕੁੰਜੀ_SZ 32
#ਜੇ_ਅੰਤ

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ਸਥਿਰ ਖਾਲੀ kvrecord_Make_ਕੁੰਜੀ(
  ਅਚਲ ਅੱਖਰ *zClass,
  ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ_ਵਿੱਚ,
  ਅੱਖਰ *z_ਕੁੰਜੀ_ਬਾਹਰਲਾ
){
  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਕੁੰਜੀ_ਵਿੱਚ );
  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਕੁੰਜੀ_ਬਾਹਰਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( zClass );
  sqlite3_snprintf(KVRECORD_ਕੁੰਜੀ_SZ, z_ਕੁੰਜੀ_ਬਾਹਰਲਾ, "kvvfs-%s-%s",
                   zClass, z_ਕੁੰਜੀ_ਵਿੱਚ);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WASM
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
ਸਥਿਰ ਪੂਰਨ kvrecordWrite(
  ਅਚਲ ਅੱਖਰ *zClass,
  ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ,
  ਅਚਲ ਅੱਖਰ *z_ਅੰਕੜੇ
){
  FILE *fd;
  ਅੱਖਰ zXKey[KVRECORD_ਕੁੰਜੀ_SZ];
  kvrecord_Make_ਕੁੰਜੀ(zClass, z_ਕੁੰਜੀ, zXKey);
  fd = fopen(zXKey, "wb");
  ਜੇ( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z_ਅੰਕੜੇ), z_ਅੰਕੜੇ,
                 ਲੜੀ_ਲੰਬਾਈ(z_ਅੰਕੜੇ)>50 ? "..." : ""));
    fputs(z_ਅੰਕੜੇ, fd);
    fclose(fd);
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
ਸਥਿਰ ਪੂਰਨ kvrecordDelete(ਅਚਲ ਅੱਖਰ *zClass, ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ){
  ਅੱਖਰ zXKey[KVRECORD_ਕੁੰਜੀ_SZ];
  kvrecord_Make_ਕੁੰਜੀ(zClass, z_ਕੁੰਜੀ, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ kvrecordRead(
  ਅਚਲ ਅੱਖਰ *zClass,
  ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ,
  ਅੱਖਰ *zBuf,
  ਪੂਰਨ nBuf
){
  FILE *fd;
  ਬਣਤਰ stat buf;
  ਅੱਖਰ zXKey[KVRECORD_ਕੁੰਜੀ_SZ];
  kvrecord_Make_ਕੁੰਜੀ(zClass, z_ਕੁੰਜੀ, zXKey);
  ਜੇ( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_ਬਹੁਲਕ)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ਵਾਪਸ -1;
  }
  ਜੇ( nBuf<=0 ){
    ਵਾਪਸ (ਪੂਰਨ)buf.st_ਮਾਪ;
  }ਨਹੀਂਤਾਂ ਜੇ( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (ਪੂਰਨ)buf.st_ਮਾਪ));
    ਵਾਪਸ (ਪੂਰਨ)buf.st_ਮਾਪ;
  }
  ਜੇ( nBuf > buf.st_ਮਾਪ + 1 ){
    nBuf = buf.st_ਮਾਪ + 1;
  }
  fd = fopen(zXKey, "rb");
  ਜੇ( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ਵਾਪਸ -1;
  }ਨਹੀਂਤਾਂ{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    ਵਾਪਸ (ਪੂਰਨ)n;
  }
}
#ਜੇ_ਅੰਤ /* #ifndef SQLITE_WASM */


/*
** An internal level of indirection which enables us to replace the
** kvvfs i/o methods with JavaScript implementations in WASM builds.
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
ਬਣਤਰ sqlite3_kvvfs_methods {
  ਪੂਰਨ (*xRcrdRead)(ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ, ਅੱਖਰ *zBuf, ਪੂਰਨ nBuf);
  ਪੂਰਨ (*xRcrdWrite)(ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ, ਅਚਲ ਅੱਖਰ *z_ਅੰਕੜੇ);
  ਪੂਰਨ (*xRcrdDelete)(ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ);
  ਅਚਲ ਪੂਰਨ n_ਕੁੰਜੀ_ਮਾਪ;
  ਅਚਲ ਪੂਰਨ n_ਬਫਰ_ਮਾਪ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WASM
#  define MAYBE_CONST ਅਚਲ
#ਨਹੀਂਤਾਂ
#  define MAYBE_CONST
#ਜੇ_ਅੰਤ
  MAYBE_CONST sqlite3_vfs * pVfs;
  MAYBE_CONST sqlite3_io_methods *pIoDb;
  MAYBE_CONST sqlite3_io_methods *pIoJrnl;
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ MAYBE_CONST
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WASM
ਅਚਲ
#ਜੇ_ਅੰਤ
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#ਨਹੀਂਤਾਂ
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#ਜੇ_ਅੰਤ
  .n_ਕੁੰਜੀ_ਮਾਪ        = KVRECORD_ਕੁੰਜੀ_SZ,
  .n_ਬਫਰ_ਮਾਪ     = SQLITE_KVOS_SZ,
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WASM
ਸਥਿਰ
#ਜੇ_ਅੰਤ
ਪੂਰਨ kvvfsEncode(ਅਚਲ ਅੱਖਰ *a_ਅੰਕੜੇ, ਪੂਰਨ n_ਅੰਕੜੇ, ਅੱਖਰ *a_ਬਾਹਰਲਾ){
  ਪੂਰਨ i, j;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)a_ਅੰਕੜੇ;
  ਲਈ(i=j=0; i<n_ਅੰਕੜੇ; i++){
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c = a[i];
    ਜੇ( c!=0 ){
      a_ਬਾਹਰਲਾ[j++] = "0123456789ABCDEF"[c>>4];
      a_ਬਾਹਰਲਾ[j++] = "0123456789ABCDEF"[c&0xf];
    }ਨਹੀਂਤਾਂ{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      ਪੂਰਨ k;
      ਲਈ(k=1; i+k<n_ਅੰਕੜੇ && a[i+k]==0; k++){}
      i += k-1;
      ਜਦੋਂ( k>0 ){
        a_ਬਾਹਰਲਾ[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  a_ਬਾਹਰਲਾ[j] = 0;
  ਵਾਪਸ j;
}

ਸਥਿਰ ਅਚਲ ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ kvvfs_Hex_ਮੁੱਲ[256] = {
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WASM
ਸਥਿਰ
#ਜੇ_ਅੰਤ
ਪੂਰਨ kvvfsDecode(ਅਚਲ ਅੱਖਰ *a, ਅੱਖਰ *a_ਬਾਹਰਲਾ, ਪੂਰਨ n_ਬਾਹਰਲਾ){
  ਪੂਰਨ i, j;
  ਪੂਰਨ c;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਵਿੱਚ = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)a;
  i = 0;
  j = 0;
  ਜਦੋਂ( 1 ){
    c = kvvfs_Hex_ਮੁੱਲ[a_ਵਿੱਚ[i]];
    ਜੇ( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = a_ਵਿੱਚ[i];
      ਜੇ( c==0 ) ਰੋਕੋ;
      ਜਦੋਂ( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        ਜੇ( n>n_ਬਾਹਰਲਾ ) ਵਾਪਸ -1 /* oversized/malformed input */;
        mult *= 26;
        c = a_ਵਿੱਚ[++i];
      }
      ਜੇ( j+n>n_ਬਾਹਰਲਾ ) ਵਾਪਸ -1 /* oversized/malformed input */;
      memset(&a_ਬਾਹਰਲਾ[j], 0, n);
      j += n;
      ਜੇ( c==0 || mult==1 ) ਰੋਕੋ; /* progress stalled if mult==1 */
    }ਨਹੀਂਤਾਂ ਜੇ( j<n_ਬਾਹਰਲਾ ){
      a_ਬਾਹਰਲਾ[j] = c<<4;
      c = kvvfs_Hex_ਮੁੱਲ[a_ਵਿੱਚ[++i]];
      ਜੇ( c<0 ) ਵਾਪਸ -1 /* hex bytes are always in pairs */;
      a_ਬਾਹਰਲਾ[j++] += c;
      i++;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ -1;
    }
  }
  ਵਾਪਸ j;
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
ਸਥਿਰ ਖਾਲੀ kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ਅਚਲ ਅੱਖਰ *zTxt,      /* Text encoding.  Zero-terminated */
  ਪੂਰਨ nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n = 0, mult;
  ਪੂਰਨ c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ਜਦੋਂ( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  ਜੇ( ' '!=zTxt[i++] ){
    /* Malformed input */
    ਵਾਪਸ;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  ਜੇ( pFile->aJrnl==0 ){
    ਵਾਪਸ;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  ਜੇ( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
ਸਥਿਰ sqlite3_int64 kvvfs_Read_File_ਮਾਪ(KVVfsFile *pFile){
  ਅੱਖਰ z_ਅੰਕੜੇ[50];
  z_ਅੰਕੜੇ[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", z_ਅੰਕੜੇ,
                                ਆਕਾਰ(z_ਅੰਕੜੇ)-1);
  ਵਾਪਸ strtoll(z_ਅੰਕੜੇ, 0, 0);
}
ਸਥਿਰ ਪੂਰਨ kvvfs_Write_File_ਮਾਪ(KVVfsFile *pFile, sqlite3_int64 sz){
  ਅੱਖਰ z_ਅੰਕੜੇ[50];
  sqlite3_snprintf(ਆਕਾਰ(z_ਅੰਕੜੇ), z_ਅੰਕੜੇ, "%lld", sz);
  ਵਾਪਸ sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", z_ਅੰਕੜੇ);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->a_ਅੰਕੜੇ);
  memset(pFile, 0, ਆਕਾਰ(*pFile));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Read from the -journal file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  ਖਾਲੀ *zBuf,
  ਪੂਰਨ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ਜੇ( pFile->aJrnl==0 ){
    ਪੂਰਨ rc;
    ਪੂਰਨ szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    ਅੱਖਰ *aTxt;
    ਜੇ( szTxt<=4 ){
      ਵਾਪਸ SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    ਜੇ( aTxt==0 ) ਵਾਪਸ SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    ਜੇ( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    ਜੇ( rc ) ਵਾਪਸ rc;
    ਜੇ( pFile->aJrnl==0 ) ਵਾਪਸ SQLITE_IOERR;
  }
  ਜੇ( iOfst+iAmt>pFile->nJrnl ){
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, pFile->aJrnl+iOfst, iAmt);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Read from the database file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsReadDb(
  sqlite3_file *pProtoFile,
  ਖਾਲੀ *zBuf,
  ਪੂਰਨ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pgno;
  ਪੂਰਨ got, n;
  ਅੱਖਰ z_ਕੁੰਜੀ[30];
  ਅੱਖਰ *a_ਅੰਕੜੇ = pFile->a_ਅੰਕੜੇ;
  ਪ੍ਰਤਿਜ੍ਞਾ( iOfst>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ਜੇ( iOfst+iAmt>=512 ){
    ਜੇ( (iOfst % iAmt)!=0 ){
      ਵਾਪਸ SQLITE_IOERR_READ;
    }
    ਜੇ( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      ਵਾਪਸ SQLITE_IOERR_READ;
    }
    pFile->sz_ਪੰਨਾ = iAmt;
    pgno = 1 + iOfst/iAmt;
  }ਨਹੀਂਤਾਂ{
    pgno = 1;
  }
  sqlite3_snprintf(ਆਕਾਰ(z_ਕੁੰਜੀ), z_ਕੁੰਜੀ, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_ਕੁੰਜੀ,
                                      a_ਅੰਕੜੇ, SQLITE_KVOS_SZ-1);
  ਜੇ( got<0 ){
    n = 0;
  }ਨਹੀਂਤਾਂ{
    a_ਅੰਕੜੇ[got] = 0;
    ਜੇ( iOfst+iAmt<512 ){
      ਪੂਰਨ k = iOfst+iAmt;
      a_ਅੰਕੜੇ[k*2] = 0;
      n = kvvfsDecode(a_ਅੰਕੜੇ, &a_ਅੰਕੜੇ[2000], SQLITE_KVOS_SZ-2000);
      ਜੇ( n>=iOfst+iAmt ){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, &a_ਅੰਕੜੇ[2000+iOfst], iAmt);
        n = iAmt;
      }ਨਹੀਂਤਾਂ{
        n = 0;
      }
    }ਨਹੀਂਤਾਂ{
      n = kvvfsDecode(a_ਅੰਕੜੇ, zBuf, iAmt);
    }
  }
  ਜੇ( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }
  ਵਾਪਸ SQLITE_OK;
}


/*
** Write into the -journal file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ਅਚਲ ਖਾਲੀ *zBuf,
  ਪੂਰਨ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 i_ਸਿਰਾ = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ਜੇ( i_ਸਿਰਾ>=0x10000000 ) ਵਾਪਸ SQLITE_ਭਰਿਆ;
  ਜੇ( pFile->aJrnl==0 || pFile->nJrnl<i_ਸਿਰਾ ){
    ਅੱਖਰ *aNew = sqlite3_realloc(pFile->aJrnl, i_ਸਿਰਾ);
    ਜੇ( aNew==0 ){
      ਵਾਪਸ SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    ਜੇ( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = i_ਸਿਰਾ;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pFile->aJrnl+iOfst, zBuf, iAmt);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Write into the database file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ਅਚਲ ਖਾਲੀ *zBuf,
  ਪੂਰਨ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pgno;
  ਅੱਖਰ z_ਕੁੰਜੀ[30];
  ਅੱਖਰ *a_ਅੰਕੜੇ = pFile->a_ਅੰਕੜੇ;
  ਪੂਰਨ rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ਪ੍ਰਤਿਜ੍ਞਾ( iAmt>=512 && iAmt<=65536 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (iAmt & (iAmt-1))==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->sz_ਪੰਨਾ<0 || pFile->sz_ਪੰਨਾ==iAmt );
  pFile->sz_ਪੰਨਾ = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(ਆਕਾਰ(z_ਕੁੰਜੀ), z_ਕੁੰਜੀ, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, a_ਅੰਕੜੇ);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_ਕੁੰਜੀ, a_ਅੰਕੜੇ);
  ਜੇ( 0==rc ){
    ਜੇ( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  ਵਾਪਸ rc;
}

/*
** Truncate an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 ਮਾਪ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, ਮਾਪ));
  ਪ੍ਰਤਿਜ੍ਞਾ( ਮਾਪ==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 ਮਾਪ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ਜੇ( pFile->szDb>ਮਾਪ
   && pFile->sz_ਪੰਨਾ>0
   && (ਮਾਪ % pFile->sz_ਪੰਨਾ)==0
  ){
    ਅੱਖਰ z_ਕੁੰਜੀ[50];
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, ਮਾਪ));
    pgno = 1 + ਮਾਪ/pFile->sz_ਪੰਨਾ;
    pgnoMax = 2 + pFile->szDb/pFile->sz_ਪੰਨਾ;
    ਜਦੋਂ( pgno<=pgnoMax ){
      sqlite3_snprintf(ਆਕਾਰ(z_ਕੁੰਜੀ), z_ਕੁੰਜੀ, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_ਕੁੰਜੀ);
      pgno++;
    }
    pFile->szDb = ਮਾਪ;
    ਵਾਪਸ kvvfs_Write_File_ਮਾਪ(pFile, ਮਾਪ) ? SQLITE_IOERR : SQLITE_OK;
  }
  ਵਾਪਸ SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsSyncJrnl(sqlite3_file *pProtoFile, ਪੂਰਨ flags){
  ਪੂਰਨ i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  ਜੇ( pFile->nJrnl<=0 ){
    ਵਾਪਸ kvvfsTruncateJrnl(pProtoFile, 0);
  }
  z_ਬਾਹਰਲਾ = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  ਜੇ( z_ਬਾਹਰਲਾ==0 ){
    ਵਾਪਸ SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  ਕਰੋ{
    z_ਬਾਹਰਲਾ[i++] = 'a' + (n%26);
    n /= 26;
  }ਜਦੋਂ( n>0 );
  z_ਬਾਹਰਲਾ[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &z_ਬਾਹਰਲਾ[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", z_ਬਾਹਰਲਾ);
  sqlite3_free(z_ਬਾਹਰਲਾ);
  ਵਾਪਸ i ? SQLITE_IOERR : SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ kvvfsSyncDb(sqlite3_file *pProtoFile, ਪੂਰਨ flags){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfs_File_ਮਾਪ_Jrnl(sqlite3_file *pProtoFile, sqlite_int64 *p_ਮਾਪ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *p_ਮਾਪ = pFile->nJrnl;
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ kvvfs_File_ਮਾਪ_Db(sqlite3_file *pProtoFile, sqlite_int64 *p_ਮਾਪ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  ਜੇ( pFile->szDb>=0 ){
    *p_ਮਾਪ = pFile->szDb;
  }ਨਹੀਂਤਾਂ{
    *p_ਮਾਪ = kvvfs_Read_File_ਮਾਪ(pFile);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfs_ਤਾਲਾ(sqlite3_file *pProtoFile, ਪੂਰਨ e_ਤਾਲਾ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ਪ੍ਰਤਿਜ੍ਞਾ( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, e_ਤਾਲਾ));

  ਜੇ( e_ਤਾਲਾ!=SQLITE_ਤਾਲਾ_NONE ){
    pFile->szDb = kvvfs_Read_File_ਮਾਪ(pFile);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsUnlock(sqlite3_file *pProtoFile, ਪੂਰਨ e_ਤਾਲਾ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ਪ੍ਰਤਿਜ੍ਞਾ( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, e_ਤਾਲਾ));
  ਜੇ( e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_NONE ){
    pFile->szDb = -1;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfs_Check_Reserved_ਤਾਲਾ(sqlite3_file *pProtoFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *p_Res_ਬਾਹਰਲਾ = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsFileControlJrnl(sqlite3_file *pProtoFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  ਵਾਪਸ SQLITE_NOTFOUND;
}
ਸਥਿਰ ਪੂਰਨ kvvfsFileControlDb(sqlite3_file *pProtoFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  ਜੇ( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    ਪੂਰਨ rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    ਜੇ( pFile->szDb>0 && 0!=kvvfs_Write_File_ਮਾਪ(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    ਵਾਪਸ rc;
  }
  ਵਾਪਸ SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfs_Sector_ਮਾਪ(sqlite3_file *pFile){
  ਵਾਪਸ 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  ਵਾਪਸ 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pProtoFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ਜੇ( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  ਪ੍ਰਤਿਜ੍ਞਾ(!pFile->zClass);
  ਪ੍ਰਤਿਜ੍ਞਾ(!pFile->a_ਅੰਕੜੇ);
  ਪ੍ਰਤਿਜ੍ਞਾ(!pFile->aJrnl);
  ਪ੍ਰਤਿਜ੍ਞਾ(!pFile->nJrnl);
  ਪ੍ਰਤਿਜ੍ਞਾ(!pFile->ਅਧਾਰ.pMethods);
  pFile->sz_ਪੰਨਾ = -1;
  pFile->szDb = -1;
  ਜੇ( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->ਅਧਾਰ.pMethods = &kvvfs_jrnl_io_methods;
    ਜੇ( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }ਨਹੀਂਤਾਂ ਜੇ( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }ਨਹੀਂਤਾਂ{
    pFile->isJournal = 0;
    pFile->ਅਧਾਰ.pMethods = &kvvfs_db_io_methods;
  }
  ਜੇ( !pFile->zClass ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_WASM
    ਜੇ( ਲੜੀ_ਲੰਬਾਈ(zName) >= (KVRECORD_ਕੁੰਜੀ_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      ਵਾਪਸ SQLITE_CANTOPEN;
    }
#ਨਹੀਂਤਾਂ
    ਜੇ( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      ਵਾਪਸ SQLITE_CANTOPEN;
    }
#ਜੇ_ਅੰਤ
    pFile->zClass = zName;
  }
  pFile->a_ਅੰਕੜੇ = sqlite3_malloc64(SQLITE_KVOS_SZ);
  ਜੇ( pFile->a_ਅੰਕੜੇ==0 ){
    ਵਾਪਸ SQLITE_NOMEM;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  ਪੂਰਨ rc /* The JS impl can fail with OOM in argument conversion */;
  ਜੇ( strcmp(z_ਰਸਤਾ, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }ਨਹੀਂਤਾਂ
  ਜੇ( strcmp(z_ਰਸਤਾ, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  ਨਹੀਂਤਾਂ{
    rc = 0;
  }
  ਵਾਪਸ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", z_ਰਸਤਾ));
#ਜੇ 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ਅਚਲ ਅੱਖਰ *z_ਕੁੰਜੀ = (0==sqlite3_strglob("*-journal", z_ਰਸਤਾ))
    ? "jrnl" : "sz";
  *p_Res_ਬਾਹਰਲਾ =
    sqlite3KvvfsMethods.xRcrdRead(z_ਰਸਤਾ, z_ਕੁੰਜੀ, 0, 0)>0;
#ਨਹੀਂਤਾਂ
  ਜੇ( strcmp(z_ਰਸਤਾ, "local-journal")==0 ){
    *p_Res_ਬਾਹਰਲਾ =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }ਨਹੀਂਤਾਂ
  ਜੇ( strcmp(z_ਰਸਤਾ, "session-journal")==0 ){
    *p_Res_ਬਾਹਰਲਾ =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }ਨਹੀਂਤਾਂ
  ਜੇ( strcmp(z_ਰਸਤਾ, "local")==0 ){
    *p_Res_ਬਾਹਰਲਾ =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }ਨਹੀਂਤਾਂ
  ਜੇ( strcmp(z_ਰਸਤਾ, "session")==0 ){
    *p_Res_ਬਾਹਰਲਾ =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }ਨਹੀਂਤਾਂ
  {
    *p_Res_ਬਾਹਰਲਾ = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#ਜੇ_ਅੰਤ
  SQLITE_KV_LOG(("xAccess returns %d\n",*p_Res_ਬਾਹਰਲਾ));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ਸਥਿਰ ਪੂਰਨ kvvfs_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,
  ਪੂਰਨ n_ਬਾਹਰਲਾ,
  ਅੱਖਰ *z_ਬਾਹਰਲਾ
){
  size_t n_ਰਸਤਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OS_KV_ALWAYS_LOCAL
  z_ਰਸਤਾ = "local";
#ਜੇ_ਅੰਤ
  n_ਰਸਤਾ = ਲੜੀ_ਲੰਬਾਈ(z_ਰਸਤਾ);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", z_ਰਸਤਾ));
  ਜੇ( n_ਬਾਹਰਲਾ<n_ਰਸਤਾ+1 ) n_ਰਸਤਾ = n_ਬਾਹਰਲਾ - 1;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_ਬਾਹਰਲਾ, z_ਰਸਤਾ, n_ਰਸਤਾ);
  z_ਬਾਹਰਲਾ[n_ਰਸਤਾ] = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ਸਥਿਰ ਖਾਲੀ *kvvfsDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  ਵਾਪਸ 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  memset(z_Buf_ਬਾਹਰਲਾ, 0, nByte);
  ਵਾਪਸ nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
ਸਥਿਰ ਪੂਰਨ kvvfsSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ਸਥਿਰ ਪੂਰਨ kvvfs_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  sqlite3_int64 i = 0;
  ਪੂਰਨ rc;
  rc = kvvfs_Current_ਸਮਾਂ_Int64(0, &i);
  *p_ਸਮਾਂ_ਬਾਹਰਲਾ = i/86400000.0;
  ਵਾਪਸ rc;
}
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
ਸਥਿਰ ਪੂਰਨ kvvfs_Current_ਸਮਾਂ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  ਸਥਿਰ ਅਚਲ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ਬਣਤਰ timeval sNow;
  (ਖਾਲੀ)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *p_ਸਮਾਂ_ਬਾਹਰਲਾ = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#ਜੇ SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
ਪੂਰਨ sqlite3_os_init(ਖਾਲੀ){
  ਵਾਪਸ sqlite3_vfs_ਰਜਿਸਟਰ(&sqlite3OsKvvfsObject, 1);
}
ਪੂਰਨ sqlite3_os_ਸਿਰਾ(ਖਾਲੀ){
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_OS_KV */

#ਜੇ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
ਪੂਰਨ sqlite3KvvfsInit(ਖਾਲੀ){
  ਵਾਪਸ sqlite3_vfs_ਰਜਿਸਟਰ(&sqlite3OsKvvfsObject, 0);
}
#ਜੇ_ਅੰਤ
