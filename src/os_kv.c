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
#ಆದರೆ SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#ಆದರೆ 0
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_KV_TRACE(X)  ಬರೆ X
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_KV_TRACE(X)
#ಆದರೆ_ಅಂತ್ಯ

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#ಆದರೆ 0
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_KV_LOG(X)  ಬರೆ X
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_KV_LOG(X)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Forward declaration of objects used by this VFS implementation
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ರಚನೆ KVVfsFile {
  sqlite3_file ತಳ;              /* IO methods */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zClass;             /* Storage class */
  ಪೂರ್ಣಾಂಕ isJournal;                  /* True if this is a journal file */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nJrnl;             /* Space allocated for aJrnl[] */
  ಅಕ್ಷರ *aJrnl;                    /* Journal content */
  ಪೂರ್ಣಾಂಕ sz_ಪುಟ;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  ಅಕ್ಷರ *a_ದತ್ತಾಂಶ;                    /* Buffer to hold page data */
};
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsReadDb(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsReadJrnl(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsWriteDb(sqlite3_file*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsWriteJrnl(sqlite3_file*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsSyncDb(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsSyncJrnl(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_File_ಗಾತ್ರ_Db(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_File_ಗಾತ್ರ_Jrnl(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsUnlock(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Check_Reserved_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsFileControlDb(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsFileControlJrnl(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Sector_ಗಾತ್ರ(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_file*, ಪೂರ್ಣಾಂಕ , ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsDelete(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsAccess(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_ತುಂಬಿದ_Pathname(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಶೂನ್ಯ *kvvfsDlOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsSleep(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Current_ಸಮಯ(sqlite3_vfs*, ದ್ವಿಗುಣ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Current_ಸಮಯ_Int64(sqlite3_vfs*, sqlite3_int64*);

ಸ್ಥಿರ sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  ಗಾತ್ರ(KVVfsFile),              /* szOsFile */
  1024,                           /* mxPathname */
  0,                              /* pNext */
  "kvvfs",                        /* zName */
  0,                              /* pAppData */
  kvvfsOpen,                      /* xOpen */
  kvvfsDelete,                    /* xDelete */
  kvvfsAccess,                    /* xAccess */
  kvvfs_ತುಂಬಿದ_Pathname,              /* xFullPathname */
  kvvfsDlOpen,                    /* xDlOpen */
  0,                              /* xDlError */
  0,                              /* xDlSym */
  0,                              /* xDlClose */
  kvvfsRandomness,                /* xRandomness */
  kvvfsSleep,                     /* xSleep */
  kvvfs_Current_ಸಮಯ,               /* xCurrentTime */
  0,                              /* xGetLastError */
  kvvfs_Current_ಸಮಯ_Int64           /* xCurrentTimeInt64 */
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
  kvvfs_File_ಗಾತ್ರ_Db,                /* xFileSize */
  kvvfs_ಬೀಗ,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_ಬೀಗ,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfs_Sector_ಗಾತ್ರ,                /* xSectorSize */
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
  kvvfs_File_ಗಾತ್ರ_Jrnl,              /* xFileSize */
  kvvfs_ಬೀಗ,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_ಬೀಗ,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfs_Sector_ಗಾತ್ರ,                /* xSectorSize */
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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvrecordWrite(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ದತ್ತಾಂಶ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvrecordDelete(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvrecordRead(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ, ಅಕ್ಷರ *zBuf, ಪೂರ್ಣಾಂಕ nBuf);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ KVRECORD_ಕೀಲಿ_SZ
#ವ್ಯಾಖ್ಯಾನಿಸು KVRECORD_ಕೀಲಿ_SZ 32
#ಆದರೆ_ಅಂತ್ಯ

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ಸ್ಥಿರ ಶೂನ್ಯ kvrecord_Make_ಕೀಲಿ(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zClass,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ_ಒಳ,
  ಅಕ್ಷರ *z_ಕೀಲಿ_ಹೊರಹೋಗು
){
  ಪ್ರತಿಜ್ಞಾ( z_ಕೀಲಿ_ಒಳ );
  ಪ್ರತಿಜ್ಞಾ( z_ಕೀಲಿ_ಹೊರಹೋಗು );
  ಪ್ರತಿಜ್ಞಾ( zClass );
  sqlite3_snprintf(KVRECORD_ಕೀಲಿ_SZ, z_ಕೀಲಿ_ಹೊರಹೋಗು, "kvvfs-%s-%s",
                   zClass, z_ಕೀಲಿ_ಒಳ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WASM
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvrecordWrite(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zClass,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ದತ್ತಾಂಶ
){
  FILE *fd;
  ಅಕ್ಷರ zXKey[KVRECORD_ಕೀಲಿ_SZ];
  kvrecord_Make_ಕೀಲಿ(zClass, z_ಕೀಲಿ, zXKey);
  fd = fopen(zXKey, "wb");
  ಆದರೆ( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ದತ್ತಾಂಶ), z_ದತ್ತಾಂಶ,
                 ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ದತ್ತಾಂಶ)>50 ? "..." : ""));
    fputs(z_ದತ್ತಾಂಶ, fd);
    fclose(fd);
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvrecordDelete(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zClass, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ){
  ಅಕ್ಷರ zXKey[KVRECORD_ಕೀಲಿ_SZ];
  kvrecord_Make_ಕೀಲಿ(zClass, z_ಕೀಲಿ, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvrecordRead(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zClass,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ,
  ಅಕ್ಷರ *zBuf,
  ಪೂರ್ಣಾಂಕ nBuf
){
  FILE *fd;
  ರಚನೆ stat buf;
  ಅಕ್ಷರ zXKey[KVRECORD_ಕೀಲಿ_SZ];
  kvrecord_Make_ಕೀಲಿ(zClass, z_ಕೀಲಿ, zXKey);
  ಆದರೆ( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_ಬಹುಲಕ)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ಹಿಂತಿರುಗಿ -1;
  }
  ಆದರೆ( nBuf<=0 ){
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)buf.st_ಗಾತ್ರ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (ಪೂರ್ಣಾಂಕ)buf.st_ಗಾತ್ರ));
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)buf.st_ಗಾತ್ರ;
  }
  ಆದರೆ( nBuf > buf.st_ಗಾತ್ರ + 1 ){
    nBuf = buf.st_ಗಾತ್ರ + 1;
  }
  fd = fopen(zXKey, "rb");
  ಆದರೆ( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ಹಿಂತಿರುಗಿ -1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)n;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_WASM */


/*
** An internal level of indirection which enables us to replace the
** kvvfs i/o methods with JavaScript implementations in WASM builds.
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
ರಚನೆ sqlite3_kvvfs_methods {
  ಪೂರ್ಣಾಂಕ (*xRcrdRead)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ, ಅಕ್ಷರ *zBuf, ಪೂರ್ಣಾಂಕ nBuf);
  ಪೂರ್ಣಾಂಕ (*xRcrdWrite)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ದತ್ತಾಂಶ);
  ಪೂರ್ಣಾಂಕ (*xRcrdDelete)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ);
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ_ಗಾತ್ರ;
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_ಬಫರ್_ಗಾತ್ರ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WASM
#  define MAYBE_CONST ಸ್ಥಿರಾಂಕ
#ಇಲ್ಲದಿದ್ದರೆ
#  define MAYBE_CONST
#ಆದರೆ_ಅಂತ್ಯ
  MAYBE_CONST sqlite3_vfs * pVfs;
  MAYBE_CONST sqlite3_io_methods *pIoDb;
  MAYBE_CONST sqlite3_io_methods *pIoJrnl;
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು MAYBE_CONST
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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WASM
ಸ್ಥಿರಾಂಕ
#ಆದರೆ_ಅಂತ್ಯ
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#ಇಲ್ಲದಿದ್ದರೆ
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#ಆದರೆ_ಅಂತ್ಯ
  .n_ಕೀಲಿ_ಗಾತ್ರ        = KVRECORD_ಕೀಲಿ_SZ,
  .n_ಬಫರ್_ಗಾತ್ರ     = SQLITE_KVOS_SZ,
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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WASM
ಸ್ಥಿರ
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ kvvfsEncode(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *a_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ, ಅಕ್ಷರ *a_ಹೊರಹೋಗು){
  ಪೂರ್ಣಾಂಕ i, j;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)a_ದತ್ತಾಂಶ;
  ಗಾಗಿ(i=j=0; i<n_ದತ್ತಾಂಶ; i++){
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c = a[i];
    ಆದರೆ( c!=0 ){
      a_ಹೊರಹೋಗು[j++] = "0123456789ABCDEF"[c>>4];
      a_ಹೊರಹೋಗು[j++] = "0123456789ABCDEF"[c&0xf];
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      ಪೂರ್ಣಾಂಕ k;
      ಗಾಗಿ(k=1; i+k<n_ದತ್ತಾಂಶ && a[i+k]==0; k++){}
      i += k-1;
      ಆಗ( k>0 ){
        a_ಹೊರಹೋಗು[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  a_ಹೊರಹೋಗು[j] = 0;
  ಹಿಂತಿರುಗಿ j;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ kvvfs_Hex_ಮೌಲ್ಯ[256] = {
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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WASM
ಸ್ಥಿರ
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ kvvfsDecode(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *a, ಅಕ್ಷರ *a_ಹೊರಹೋಗು, ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು){
  ಪೂರ್ಣಾಂಕ i, j;
  ಪೂರ್ಣಾಂಕ c;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ಒಳ = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)a;
  i = 0;
  j = 0;
  ಆಗ( 1 ){
    c = kvvfs_Hex_ಮೌಲ್ಯ[a_ಒಳ[i]];
    ಆದರೆ( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = a_ಒಳ[i];
      ಆದರೆ( c==0 ) ನಿಲ್ಲಿಸು;
      ಆಗ( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        ಆದರೆ( n>n_ಹೊರಹೋಗು ) ಹಿಂತಿರುಗಿ -1 /* oversized/malformed input */;
        mult *= 26;
        c = a_ಒಳ[++i];
      }
      ಆದರೆ( j+n>n_ಹೊರಹೋಗು ) ಹಿಂತಿರುಗಿ -1 /* oversized/malformed input */;
      memset(&a_ಹೊರಹೋಗು[j], 0, n);
      j += n;
      ಆದರೆ( c==0 || mult==1 ) ನಿಲ್ಲಿಸು; /* progress stalled if mult==1 */
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( j<n_ಹೊರಹೋಗು ){
      a_ಹೊರಹೋಗು[j] = c<<4;
      c = kvvfs_Hex_ಮೌಲ್ಯ[a_ಒಳ[++i]];
      ಆದರೆ( c<0 ) ಹಿಂತಿರುಗಿ -1 /* hex bytes are always in pairs */;
      a_ಹೊರಹೋಗು[j++] += c;
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ -1;
    }
  }
  ಹಿಂತಿರುಗಿ j;
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
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTxt,      /* Text encoding.  Zero-terminated */
  ಪೂರ್ಣಾಂಕ nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n = 0, mult;
  ಪೂರ್ಣಾಂಕ c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ಆಗ( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  ಆದರೆ( ' '!=zTxt[i++] ){
    /* Malformed input */
    ಹಿಂತಿರುಗಿ;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  ಆದರೆ( pFile->aJrnl==0 ){
    ಹಿಂತಿರುಗಿ;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  ಆದರೆ( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
ಸ್ಥಿರ sqlite3_int64 kvvfs_Read_File_ಗಾತ್ರ(KVVfsFile *pFile){
  ಅಕ್ಷರ z_ದತ್ತಾಂಶ[50];
  z_ದತ್ತಾಂಶ[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", z_ದತ್ತಾಂಶ,
                                ಗಾತ್ರ(z_ದತ್ತಾಂಶ)-1);
  ಹಿಂತಿರುಗಿ strtoll(z_ದತ್ತಾಂಶ, 0, 0);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Write_File_ಗಾತ್ರ(KVVfsFile *pFile, sqlite3_int64 sz){
  ಅಕ್ಷರ z_ದತ್ತಾಂಶ[50];
  sqlite3_snprintf(ಗಾತ್ರ(z_ದತ್ತಾಂಶ), z_ದತ್ತಾಂಶ, "%lld", sz);
  ಹಿಂತಿರುಗಿ sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", z_ದತ್ತಾಂಶ);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->a_ದತ್ತಾಂಶ);
  memset(pFile, 0, ಗಾತ್ರ(*pFile));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read from the -journal file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ಪ್ರತಿಜ್ಞಾ( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ಆದರೆ( pFile->aJrnl==0 ){
    ಪೂರ್ಣಾಂಕ rc;
    ಪೂರ್ಣಾಂಕ szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    ಅಕ್ಷರ *aTxt;
    ಆದರೆ( szTxt<=4 ){
      ಹಿಂತಿರುಗಿ SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    ಆದರೆ( aTxt==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    ಆದರೆ( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    ಆದರೆ( pFile->aJrnl==0 ) ಹಿಂತಿರುಗಿ SQLITE_IOERR;
  }
  ಆದರೆ( iOfst+iAmt>pFile->nJrnl ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHORT_READ;
  }
  ಸ್ಮರಣೆ_ನಕಲು(zBuf, pFile->aJrnl+iOfst, iAmt);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read from the database file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsReadDb(
  sqlite3_file *pProtoFile,
  ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pgno;
  ಪೂರ್ಣಾಂಕ got, n;
  ಅಕ್ಷರ z_ಕೀಲಿ[30];
  ಅಕ್ಷರ *a_ದತ್ತಾಂಶ = pFile->a_ದತ್ತಾಂಶ;
  ಪ್ರತಿಜ್ಞಾ( iOfst>=0 );
  ಪ್ರತಿಜ್ಞಾ( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ಆದರೆ( iOfst+iAmt>=512 ){
    ಆದರೆ( (iOfst % iAmt)!=0 ){
      ಹಿಂತಿರುಗಿ SQLITE_IOERR_READ;
    }
    ಆದರೆ( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      ಹಿಂತಿರುಗಿ SQLITE_IOERR_READ;
    }
    pFile->sz_ಪುಟ = iAmt;
    pgno = 1 + iOfst/iAmt;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pgno = 1;
  }
  sqlite3_snprintf(ಗಾತ್ರ(z_ಕೀಲಿ), z_ಕೀಲಿ, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_ಕೀಲಿ,
                                      a_ದತ್ತಾಂಶ, SQLITE_KVOS_SZ-1);
  ಆದರೆ( got<0 ){
    n = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    a_ದತ್ತಾಂಶ[got] = 0;
    ಆದರೆ( iOfst+iAmt<512 ){
      ಪೂರ್ಣಾಂಕ k = iOfst+iAmt;
      a_ದತ್ತಾಂಶ[k*2] = 0;
      n = kvvfsDecode(a_ದತ್ತಾಂಶ, &a_ದತ್ತಾಂಶ[2000], SQLITE_KVOS_SZ-2000);
      ಆದರೆ( n>=iOfst+iAmt ){
        ಸ್ಮರಣೆ_ನಕಲು(zBuf, &a_ದತ್ತಾಂಶ[2000+iOfst], iAmt);
        n = iAmt;
      }ಇಲ್ಲದಿದ್ದರೆ{
        n = 0;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      n = kvvfsDecode(a_ದತ್ತಾಂಶ, zBuf, iAmt);
    }
  }
  ಆದರೆ( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHORT_READ;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** Write into the -journal file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 i_ಕೊನೆ = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ಆದರೆ( i_ಕೊನೆ>=0x10000000 ) ಹಿಂತಿರುಗಿ SQLITE_ತುಂಬಿದ;
  ಆದರೆ( pFile->aJrnl==0 || pFile->nJrnl<i_ಕೊನೆ ){
    ಅಕ್ಷರ *aNew = sqlite3_realloc(pFile->aJrnl, i_ಕೊನೆ);
    ಆದರೆ( aNew==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    ಆದರೆ( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = i_ಕೊನೆ;
  }
  ಸ್ಮರಣೆ_ನಕಲು(pFile->aJrnl+iOfst, zBuf, iAmt);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Write into the database file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pgno;
  ಅಕ್ಷರ z_ಕೀಲಿ[30];
  ಅಕ್ಷರ *a_ದತ್ತಾಂಶ = pFile->a_ದತ್ತಾಂಶ;
  ಪೂರ್ಣಾಂಕ rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ಪ್ರತಿಜ್ಞಾ( iAmt>=512 && iAmt<=65536 );
  ಪ್ರತಿಜ್ಞಾ( (iAmt & (iAmt-1))==0 );
  ಪ್ರತಿಜ್ಞಾ( pFile->sz_ಪುಟ<0 || pFile->sz_ಪುಟ==iAmt );
  pFile->sz_ಪುಟ = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(ಗಾತ್ರ(z_ಕೀಲಿ), z_ಕೀಲಿ, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, a_ದತ್ತಾಂಶ);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_ಕೀಲಿ, a_ದತ್ತಾಂಶ);
  ಆದರೆ( 0==rc ){
    ಆದರೆ( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Truncate an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 ಗಾತ್ರ_2){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, ಗಾತ್ರ_2));
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ_2==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 ಗಾತ್ರ_2){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ಆದರೆ( pFile->szDb>ಗಾತ್ರ_2
   && pFile->sz_ಪುಟ>0
   && (ಗಾತ್ರ_2 % pFile->sz_ಪುಟ)==0
  ){
    ಅಕ್ಷರ z_ಕೀಲಿ[50];
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, ಗಾತ್ರ_2));
    pgno = 1 + ಗಾತ್ರ_2/pFile->sz_ಪುಟ;
    pgnoMax = 2 + pFile->szDb/pFile->sz_ಪುಟ;
    ಆಗ( pgno<=pgnoMax ){
      sqlite3_snprintf(ಗಾತ್ರ(z_ಕೀಲಿ), z_ಕೀಲಿ, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_ಕೀಲಿ);
      pgno++;
    }
    pFile->szDb = ಗಾತ್ರ_2;
    ಹಿಂತಿರುಗಿ kvvfs_Write_File_ಗಾತ್ರ(pFile, ಗಾತ್ರ_2) ? SQLITE_IOERR : SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsSyncJrnl(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಂಕ flags){
  ಪೂರ್ಣಾಂಕ i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  ಆದರೆ( pFile->nJrnl<=0 ){
    ಹಿಂತಿರುಗಿ kvvfsTruncateJrnl(pProtoFile, 0);
  }
  z_ಹೊರಹೋಗು = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  ಆದರೆ( z_ಹೊರಹೋಗು==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  ಮಾಡು{
    z_ಹೊರಹೋಗು[i++] = 'a' + (n%26);
    n /= 26;
  }ಆಗ( n>0 );
  z_ಹೊರಹೋಗು[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &z_ಹೊರಹೋಗು[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", z_ಹೊರಹೋಗು);
  sqlite3_free(z_ಹೊರಹೋಗು);
  ಹಿಂತಿರುಗಿ i ? SQLITE_IOERR : SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsSyncDb(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಂಕ flags){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_File_ಗಾತ್ರ_Jrnl(sqlite3_file *pProtoFile, sqlite_int64 *p_ಗಾತ್ರ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *p_ಗಾತ್ರ = pFile->nJrnl;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_File_ಗಾತ್ರ_Db(sqlite3_file *pProtoFile, sqlite_int64 *p_ಗಾತ್ರ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  ಆದರೆ( pFile->szDb>=0 ){
    *p_ಗಾತ್ರ = pFile->szDb;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *p_ಗಾತ್ರ = kvvfs_Read_File_ಗಾತ್ರ(pFile);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_ಬೀಗ(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ಪ್ರತಿಜ್ಞಾ( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, e_ಬೀಗ));

  ಆದರೆ( e_ಬೀಗ!=SQLITE_ಬೀಗ_NONE ){
    pFile->szDb = kvvfs_Read_File_ಗಾತ್ರ(pFile);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsUnlock(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ಪ್ರತಿಜ್ಞಾ( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, e_ಬೀಗ));
  ಆದರೆ( e_ಬೀಗ==SQLITE_ಬೀಗ_NONE ){
    pFile->szDb = -1;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Check_Reserved_ಬೀಗ(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *p_Res_ಹೊರಹೋಗು = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsFileControlJrnl(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  ಹಿಂತಿರುಗಿ SQLITE_NOTFOUND;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsFileControlDb(sqlite3_file *pProtoFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  ಆದರೆ( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    ಆದರೆ( pFile->szDb>0 && 0!=kvvfs_Write_File_ಗಾತ್ರ(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    ಹಿಂತಿರುಗಿ rc;
  }
  ಹಿಂತಿರುಗಿ SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  ಹಿಂತಿರುಗಿ 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pProtoFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ಆದರೆ( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  ಪ್ರತಿಜ್ಞಾ(!pFile->zClass);
  ಪ್ರತಿಜ್ಞಾ(!pFile->a_ದತ್ತಾಂಶ);
  ಪ್ರತಿಜ್ಞಾ(!pFile->aJrnl);
  ಪ್ರತಿಜ್ಞಾ(!pFile->nJrnl);
  ಪ್ರತಿಜ್ಞಾ(!pFile->ತಳ.pMethods);
  pFile->sz_ಪುಟ = -1;
  pFile->szDb = -1;
  ಆದರೆ( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->ತಳ.pMethods = &kvvfs_jrnl_io_methods;
    ಆದರೆ( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    pFile->isJournal = 0;
    pFile->ತಳ.pMethods = &kvvfs_db_io_methods;
  }
  ಆದರೆ( !pFile->zClass ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_WASM
    ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName) >= (KVRECORD_ಕೀಲಿ_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      ಹಿಂತಿರುಗಿ SQLITE_CANTOPEN;
    }
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      ಹಿಂತಿರುಗಿ SQLITE_CANTOPEN;
    }
#ಆದರೆ_ಅಂತ್ಯ
    pFile->zClass = zName;
  }
  pFile->a_ದತ್ತಾಂಶ = sqlite3_malloc64(SQLITE_KVOS_SZ);
  ಆದರೆ( pFile->a_ದತ್ತಾಂಶ==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  ಪೂರ್ಣಾಂಕ rc /* The JS impl can fail with OOM in argument conversion */;
  ಆದರೆ( strcmp(z_ಮಾರ್ಗ, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( strcmp(z_ಮಾರ್ಗ, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  ಇಲ್ಲದಿದ್ದರೆ{
    rc = 0;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", z_ಮಾರ್ಗ));
#ಆದರೆ 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೀಲಿ = (0==sqlite3_strglob("*-journal", z_ಮಾರ್ಗ))
    ? "jrnl" : "sz";
  *p_Res_ಹೊರಹೋಗು =
    sqlite3KvvfsMethods.xRcrdRead(z_ಮಾರ್ಗ, z_ಕೀಲಿ, 0, 0)>0;
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( strcmp(z_ಮಾರ್ಗ, "local-journal")==0 ){
    *p_Res_ಹೊರಹೋಗು =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( strcmp(z_ಮಾರ್ಗ, "session-journal")==0 ){
    *p_Res_ಹೊರಹೋಗು =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( strcmp(z_ಮಾರ್ಗ, "local")==0 ){
    *p_Res_ಹೊರಹೋಗು =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( strcmp(z_ಮಾರ್ಗ, "session")==0 ){
    *p_Res_ಹೊರಹೋಗು =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }ಇಲ್ಲದಿದ್ದರೆ
  {
    *p_Res_ಹೊರಹೋಗು = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#ಆದರೆ_ಅಂತ್ಯ
  SQLITE_KV_LOG(("xAccess returns %d\n",*p_Res_ಹೊರಹೋಗು));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು,
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು
){
  size_t n_ಮಾರ್ಗ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OS_KV_ALWAYS_LOCAL
  z_ಮಾರ್ಗ = "local";
#ಆದರೆ_ಅಂತ್ಯ
  n_ಮಾರ್ಗ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಮಾರ್ಗ);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", z_ಮಾರ್ಗ));
  ಆದರೆ( n_ಹೊರಹೋಗು<n_ಮಾರ್ಗ+1 ) n_ಮಾರ್ಗ = n_ಹೊರಹೋಗು - 1;
  ಸ್ಮರಣೆ_ನಕಲು(z_ಹೊರಹೋಗು, z_ಮಾರ್ಗ, n_ಮಾರ್ಗ);
  z_ಹೊರಹೋಗು[n_ಮಾರ್ಗ] = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *kvvfsDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  memset(z_Buf_ಹೊರಹೋಗು, 0, nByte);
  ಹಿಂತಿರುಗಿ nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfsSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *p_ಸಮಯ_ಹೊರಹೋಗು){
  sqlite3_int64 i = 0;
  ಪೂರ್ಣಾಂಕ rc;
  rc = kvvfs_Current_ಸಮಯ_Int64(0, &i);
  *p_ಸಮಯ_ಹೊರಹೋಗು = i/86400000.0;
  ಹಿಂತಿರುಗಿ rc;
}
#ಅನ್ತರ್ಭಾವ <sys/time.h>
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kvvfs_Current_ಸಮಯ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_ಸಮಯ_ಹೊರಹೋಗು){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ರಚನೆ timeval sNow;
  (ಶೂನ್ಯ)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *p_ಸಮಯ_ಹೊರಹೋಗು = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#ಆದರೆ SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
ಪೂರ್ಣಾಂಕ sqlite3_os_init(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&sqlite3OsKvvfsObject, 1);
}
ಪೂರ್ಣಾಂಕ sqlite3_os_ಕೊನೆ(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_KV */

#ಆದರೆ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
ಪೂರ್ಣಾಂಕ sqlite3KvvfsInit(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&sqlite3OsKvvfsObject, 0);
}
#ಆದರೆ_ಅಂತ್ಯ
