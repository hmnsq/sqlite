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
#అన్తర్భావ <sqliteInt.h>
#ఒకవేళ SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#ఒకవేళ 0
#నిర్వచించు SQLITE_KV_TRACE(X)  రాయి X
#లేకపోతే
#నిర్వచించు SQLITE_KV_TRACE(X)
#అయితే_ముగింపు

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#ఒకవేళ 0
#నిర్వచించు SQLITE_KV_LOG(X)  రాయి X
#లేకపోతే
#నిర్వచించు SQLITE_KV_LOG(X)
#అయితే_ముగింపు

/*
** Forward declaration of objects used by this VFS implementation
*/
రకనిర్వచనం నిర్మాణం KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
నిర్మాణం KVVfsFile {
  sqlite3_file పునాది;              /* IO methods */
  స్థిరరాశి అక్షరం *zClass;             /* Storage class */
  పూర్ణాంకం isJournal;                  /* True if this is a journal file */
  గుర్తులేని పూర్ణాంకం nJrnl;             /* Space allocated for aJrnl[] */
  అక్షరం *aJrnl;                    /* Journal content */
  పూర్ణాంకం sz_పుట;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  అక్షరం *a_దత్తాంశం;                    /* Buffer to hold page data */
};
#నిర్వచించు SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
స్థిర పూర్ణాంకం kvvfsClose(sqlite3_file*);
స్థిర పూర్ణాంకం kvvfsReadDb(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం kvvfsReadJrnl(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం kvvfsWriteDb(sqlite3_file*,స్థిరరాశి శూన్య*,పూర్ణాంకం iAmt, sqlite3_int64);
స్థిర పూర్ణాంకం kvvfsWriteJrnl(sqlite3_file*,స్థిరరాశి శూన్య*,పూర్ణాంకం iAmt, sqlite3_int64);
స్థిర పూర్ణాంకం kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం kvvfsSyncDb(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం kvvfsSyncJrnl(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం kvvfs_File_కొలత_Db(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం kvvfs_File_కొలత_Jrnl(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం kvvfs_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం kvvfsUnlock(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం kvvfs_Check_Reserved_తాళం(sqlite3_file*, పూర్ణాంకం *p_Res_బహిర్గతం);
స్థిర పూర్ణాంకం kvvfsFileControlDb(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
స్థిర పూర్ణాంకం kvvfsFileControlJrnl(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
స్థిర పూర్ణాంకం kvvfs_Sector_కొలత(sqlite3_file*);
స్థిర పూర్ణాంకం kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
స్థిర పూర్ణాంకం kvvfsOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *, sqlite3_file*, పూర్ణాంకం , పూర్ణాంకం *);
స్థిర పూర్ణాంకం kvvfsDelete(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం syncDir);
స్థిర పూర్ణాంకం kvvfsAccess(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం flags, పూర్ణాంకం *);
స్థిర పూర్ణాంకం kvvfs_నిండిన_Pathname(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం, అక్షరం *z_బహిర్గతం);
స్థిర శూన్య *kvvfsDlOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *zFilename);
స్థిర పూర్ణాంకం kvvfsRandomness(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_బహిర్గతం);
స్థిర పూర్ణాంకం kvvfsSleep(sqlite3_vfs*, పూర్ణాంకం microseconds);
స్థిర పూర్ణాంకం kvvfs_Current_సమయం(sqlite3_vfs*, ద్విగుణ*);
స్థిర పూర్ణాంకం kvvfs_Current_సమయం_Int64(sqlite3_vfs*, sqlite3_int64*);

స్థిర sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  పరిమాణం(KVVfsFile),              /* szOsFile */
  1024,                           /* mxPathname */
  0,                              /* pNext */
  "kvvfs",                        /* zName */
  0,                              /* pAppData */
  kvvfsOpen,                      /* xOpen */
  kvvfsDelete,                    /* xDelete */
  kvvfsAccess,                    /* xAccess */
  kvvfs_నిండిన_Pathname,              /* xFullPathname */
  kvvfsDlOpen,                    /* xDlOpen */
  0,                              /* xDlError */
  0,                              /* xDlSym */
  0,                              /* xDlClose */
  kvvfsRandomness,                /* xRandomness */
  kvvfsSleep,                     /* xSleep */
  kvvfs_Current_సమయం,               /* xCurrentTime */
  0,                              /* xGetLastError */
  kvvfs_Current_సమయం_Int64           /* xCurrentTimeInt64 */
};

/* Methods for sqlite3_file objects referencing a database file
*/
స్థిర sqlite3_io_methods kvvfs_db_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadDb,                    /* xRead */
  kvvfsWriteDb,                   /* xWrite */
  kvvfsTruncateDb,                /* xTruncate */
  kvvfsSyncDb,                    /* xSync */
  kvvfs_File_కొలత_Db,                /* xFileSize */
  kvvfs_తాళం,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_తాళం,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfs_Sector_కొలత,                /* xSectorSize */
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
స్థిర sqlite3_io_methods kvvfs_jrnl_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadJrnl,                  /* xRead */
  kvvfsWriteJrnl,                 /* xWrite */
  kvvfsTruncateJrnl,              /* xTruncate */
  kvvfsSyncJrnl,                  /* xSync */
  kvvfs_File_కొలత_Jrnl,              /* xFileSize */
  kvvfs_తాళం,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_తాళం,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfs_Sector_కొలత,                /* xSectorSize */
  kvvfsDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                              /* xShmMap */
  0,                              /* xShmLock */
  0,                              /* xShmBarrier */
  0,                              /* xShmUnmap */
  0,                              /* xFetch */
  0                               /* xUnfetch */
};

/****** Storage subsystem **************************************************/
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#నిర్వచితం_కాకపోతే SQLITE_WASM
/* In WASM builds these are implemented in JS. */
స్థిర పూర్ణాంకం kvrecordWrite(స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం *z_కీలకం, స్థిరరాశి అక్షరం *z_దత్తాంశం);
స్థిర పూర్ణాంకం kvrecordDelete(స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం *z_కీలకం);
స్థిర పూర్ణాంకం kvrecordRead(స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం *z_కీలకం, అక్షరం *zBuf, పూర్ణాంకం nBuf);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే KVRECORD_కీలకం_SZ
#నిర్వచించు KVRECORD_కీలకం_SZ 32
#అయితే_ముగింపు

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
స్థిర శూన్య kvrecord_Make_కీలకం(
  స్థిరరాశి అక్షరం *zClass,
  స్థిరరాశి అక్షరం *z_కీలకం_లోపల,
  అక్షరం *z_కీలకం_బహిర్గతం
){
  ప్రతిజ్ఞా( z_కీలకం_లోపల );
  ప్రతిజ్ఞా( z_కీలకం_బహిర్గతం );
  ప్రతిజ్ఞా( zClass );
  sqlite3_snprintf(KVRECORD_కీలకం_SZ, z_కీలకం_బహిర్గతం, "kvvfs-%s-%s",
                   zClass, z_కీలకం_లోపల);
}

#నిర్వచితం_కాకపోతే SQLITE_WASM
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
స్థిర పూర్ణాంకం kvrecordWrite(
  స్థిరరాశి అక్షరం *zClass,
  స్థిరరాశి అక్షరం *z_కీలకం,
  స్థిరరాశి అక్షరం *z_దత్తాంశం
){
  FILE *fd;
  అక్షరం zXKey[KVRECORD_కీలకం_SZ];
  kvrecord_Make_కీలకం(zClass, z_కీలకం, zXKey);
  fd = fopen(zXKey, "wb");
  ఒకవేళ( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z_దత్తాంశం), z_దత్తాంశం,
                 అక్షరశ్రేణి_పొడవు(z_దత్తాంశం)>50 ? "..." : ""));
    fputs(z_దత్తాంశం, fd);
    fclose(fd);
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
స్థిర పూర్ణాంకం kvrecordDelete(స్థిరరాశి అక్షరం *zClass, స్థిరరాశి అక్షరం *z_కీలకం){
  అక్షరం zXKey[KVRECORD_కీలకం_SZ];
  kvrecord_Make_కీలకం(zClass, z_కీలకం, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  తిరిగి 0;
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
స్థిర పూర్ణాంకం kvrecordRead(
  స్థిరరాశి అక్షరం *zClass,
  స్థిరరాశి అక్షరం *z_కీలకం,
  అక్షరం *zBuf,
  పూర్ణాంకం nBuf
){
  FILE *fd;
  నిర్మాణం stat buf;
  అక్షరం zXKey[KVRECORD_కీలకం_SZ];
  kvrecord_Make_కీలకం(zClass, z_కీలకం, zXKey);
  ఒకవేళ( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_బహుళకం)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    తిరిగి -1;
  }
  ఒకవేళ( nBuf<=0 ){
    తిరిగి (పూర్ణాంకం)buf.st_కొలత;
  }లేకపోతే ఒకవేళ( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (పూర్ణాంకం)buf.st_కొలత));
    తిరిగి (పూర్ణాంకం)buf.st_కొలత;
  }
  ఒకవేళ( nBuf > buf.st_కొలత + 1 ){
    nBuf = buf.st_కొలత + 1;
  }
  fd = fopen(zXKey, "rb");
  ఒకవేళ( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    తిరిగి -1;
  }లేకపోతే{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    తిరిగి (పూర్ణాంకం)n;
  }
}
#అయితే_ముగింపు /* #ifndef SQLITE_WASM */


/*
** An internal level of indirection which enables us to replace the
** kvvfs i/o methods with JavaScript implementations in WASM builds.
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
రకనిర్వచనం నిర్మాణం sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
నిర్మాణం sqlite3_kvvfs_methods {
  పూర్ణాంకం (*xRcrdRead)(స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం *z_కీలకం, అక్షరం *zBuf, పూర్ణాంకం nBuf);
  పూర్ణాంకం (*xRcrdWrite)(స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం *z_కీలకం, స్థిరరాశి అక్షరం *z_దత్తాంశం);
  పూర్ణాంకం (*xRcrdDelete)(స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం *z_కీలకం);
  స్థిరరాశి పూర్ణాంకం n_కీలకం_కొలత;
  స్థిరరాశి పూర్ణాంకం n_బఫరు_కొలత;
#నిర్వచితం_కాకపోతే SQLITE_WASM
#  define MAYBE_CONST స్థిరరాశి
#లేకపోతే
#  define MAYBE_CONST
#అయితే_ముగింపు
  MAYBE_CONST sqlite3_vfs * pVfs;
  MAYBE_CONST sqlite3_io_methods *pIoDb;
  MAYBE_CONST sqlite3_io_methods *pIoJrnl;
#నిర్వచనం_రద్దు MAYBE_CONST
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
#నిర్వచితం_కాకపోతే SQLITE_WASM
స్థిరరాశి
#అయితే_ముగింపు
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#నిర్వచితం_కాకపోతే SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#లేకపోతే
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#అయితే_ముగింపు
  .n_కీలకం_కొలత        = KVRECORD_కీలకం_SZ,
  .n_బఫరు_కొలత     = SQLITE_KVOS_SZ,
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
#నిర్వచితం_కాకపోతే SQLITE_WASM
స్థిర
#అయితే_ముగింపు
పూర్ణాంకం kvvfsEncode(స్థిరరాశి అక్షరం *a_దత్తాంశం, పూర్ణాంకం n_దత్తాంశం, అక్షరం *a_బహిర్గతం){
  పూర్ణాంకం i, j;
  స్థిరరాశి గుర్తులేని అక్షరం *a = (స్థిరరాశి గుర్తులేని అక్షరం*)a_దత్తాంశం;
  కొరకు(i=j=0; i<n_దత్తాంశం; i++){
    గుర్తులేని అక్షరం c = a[i];
    ఒకవేళ( c!=0 ){
      a_బహిర్గతం[j++] = "0123456789ABCDEF"[c>>4];
      a_బహిర్గతం[j++] = "0123456789ABCDEF"[c&0xf];
    }లేకపోతే{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      పూర్ణాంకం k;
      కొరకు(k=1; i+k<n_దత్తాంశం && a[i+k]==0; k++){}
      i += k-1;
      అయితే( k>0 ){
        a_బహిర్గతం[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  a_బహిర్గతం[j] = 0;
  తిరిగి j;
}

స్థిర స్థిరరాశి గుర్తుగల అక్షరం kvvfs_Hex_విలువ[256] = {
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
#నిర్వచితం_కాకపోతే SQLITE_WASM
స్థిర
#అయితే_ముగింపు
పూర్ణాంకం kvvfsDecode(స్థిరరాశి అక్షరం *a, అక్షరం *a_బహిర్గతం, పూర్ణాంకం n_బహిర్గతం){
  పూర్ణాంకం i, j;
  పూర్ణాంకం c;
  స్థిరరాశి గుర్తులేని అక్షరం *a_లోపల = (స్థిరరాశి గుర్తులేని అక్షరం*)a;
  i = 0;
  j = 0;
  అయితే( 1 ){
    c = kvvfs_Hex_విలువ[a_లోపల[i]];
    ఒకవేళ( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = a_లోపల[i];
      ఒకవేళ( c==0 ) ఆపు;
      అయితే( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        ఒకవేళ( n>n_బహిర్గతం ) తిరిగి -1 /* oversized/malformed input */;
        mult *= 26;
        c = a_లోపల[++i];
      }
      ఒకవేళ( j+n>n_బహిర్గతం ) తిరిగి -1 /* oversized/malformed input */;
      memset(&a_బహిర్గతం[j], 0, n);
      j += n;
      ఒకవేళ( c==0 || mult==1 ) ఆపు; /* progress stalled if mult==1 */
    }లేకపోతే ఒకవేళ( j<n_బహిర్గతం ){
      a_బహిర్గతం[j] = c<<4;
      c = kvvfs_Hex_విలువ[a_లోపల[++i]];
      ఒకవేళ( c<0 ) తిరిగి -1 /* hex bytes are always in pairs */;
      a_బహిర్గతం[j++] += c;
      i++;
    }లేకపోతే{
      తిరిగి -1;
    }
  }
  తిరిగి j;
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
స్థిర శూన్య kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  స్థిరరాశి అక్షరం *zTxt,      /* Text encoding.  Zero-terminated */
  పూర్ణాంకం nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  గుర్తులేని పూర్ణాంకం n = 0, mult;
  పూర్ణాంకం c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  అయితే( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  ఒకవేళ( ' '!=zTxt[i++] ){
    /* Malformed input */
    తిరిగి;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  ఒకవేళ( pFile->aJrnl==0 ){
    తిరిగి;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  ఒకవేళ( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
స్థిర sqlite3_int64 kvvfs_Read_File_కొలత(KVVfsFile *pFile){
  అక్షరం z_దత్తాంశం[50];
  z_దత్తాంశం[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", z_దత్తాంశం,
                                పరిమాణం(z_దత్తాంశం)-1);
  తిరిగి strtoll(z_దత్తాంశం, 0, 0);
}
స్థిర పూర్ణాంకం kvvfs_Write_File_కొలత(KVVfsFile *pFile, sqlite3_int64 sz){
  అక్షరం z_దత్తాంశం[50];
  sqlite3_snprintf(పరిమాణం(z_దత్తాంశం), z_దత్తాంశం, "%lld", sz);
  తిరిగి sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", z_దత్తాంశం);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->a_దత్తాంశం);
  memset(pFile, 0, పరిమాణం(*pFile));
  తిరిగి SQLITE_OK;
}

/*
** Read from the -journal file.
*/
స్థిర పూర్ణాంకం kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  శూన్య *zBuf,
  పూర్ణాంకం iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ప్రతిజ్ఞా( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ఒకవేళ( pFile->aJrnl==0 ){
    పూర్ణాంకం rc;
    పూర్ణాంకం szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    అక్షరం *aTxt;
    ఒకవేళ( szTxt<=4 ){
      తిరిగి SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    ఒకవేళ( aTxt==0 ) తిరిగి SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    ఒకవేళ( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    ఒకవేళ( rc ) తిరిగి rc;
    ఒకవేళ( pFile->aJrnl==0 ) తిరిగి SQLITE_IOERR;
  }
  ఒకవేళ( iOfst+iAmt>pFile->nJrnl ){
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }
  జ్ఞాపకం_నకలు(zBuf, pFile->aJrnl+iOfst, iAmt);
  తిరిగి SQLITE_OK;
}

/*
** Read from the database file.
*/
స్థిర పూర్ణాంకం kvvfsReadDb(
  sqlite3_file *pProtoFile,
  శూన్య *zBuf,
  పూర్ణాంకం iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  గుర్తులేని పూర్ణాంకం pgno;
  పూర్ణాంకం got, n;
  అక్షరం z_కీలకం[30];
  అక్షరం *a_దత్తాంశం = pFile->a_దత్తాంశం;
  ప్రతిజ్ఞా( iOfst>=0 );
  ప్రతిజ్ఞా( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ఒకవేళ( iOfst+iAmt>=512 ){
    ఒకవేళ( (iOfst % iAmt)!=0 ){
      తిరిగి SQLITE_IOERR_READ;
    }
    ఒకవేళ( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      తిరిగి SQLITE_IOERR_READ;
    }
    pFile->sz_పుట = iAmt;
    pgno = 1 + iOfst/iAmt;
  }లేకపోతే{
    pgno = 1;
  }
  sqlite3_snprintf(పరిమాణం(z_కీలకం), z_కీలకం, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_కీలకం,
                                      a_దత్తాంశం, SQLITE_KVOS_SZ-1);
  ఒకవేళ( got<0 ){
    n = 0;
  }లేకపోతే{
    a_దత్తాంశం[got] = 0;
    ఒకవేళ( iOfst+iAmt<512 ){
      పూర్ణాంకం k = iOfst+iAmt;
      a_దత్తాంశం[k*2] = 0;
      n = kvvfsDecode(a_దత్తాంశం, &a_దత్తాంశం[2000], SQLITE_KVOS_SZ-2000);
      ఒకవేళ( n>=iOfst+iAmt ){
        జ్ఞాపకం_నకలు(zBuf, &a_దత్తాంశం[2000+iOfst], iAmt);
        n = iAmt;
      }లేకపోతే{
        n = 0;
      }
    }లేకపోతే{
      n = kvvfsDecode(a_దత్తాంశం, zBuf, iAmt);
    }
  }
  ఒకవేళ( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }
  తిరిగి SQLITE_OK;
}


/*
** Write into the -journal file.
*/
స్థిర పూర్ణాంకం kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  స్థిరరాశి శూన్య *zBuf,
  పూర్ణాంకం iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 i_చివర = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ఒకవేళ( i_చివర>=0x10000000 ) తిరిగి SQLITE_నిండిన;
  ఒకవేళ( pFile->aJrnl==0 || pFile->nJrnl<i_చివర ){
    అక్షరం *aNew = sqlite3_realloc(pFile->aJrnl, i_చివర);
    ఒకవేళ( aNew==0 ){
      తిరిగి SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    ఒకవేళ( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = i_చివర;
  }
  జ్ఞాపకం_నకలు(pFile->aJrnl+iOfst, zBuf, iAmt);
  తిరిగి SQLITE_OK;
}

/*
** Write into the database file.
*/
స్థిర పూర్ణాంకం kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  స్థిరరాశి శూన్య *zBuf,
  పూర్ణాంకం iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  గుర్తులేని పూర్ణాంకం pgno;
  అక్షరం z_కీలకం[30];
  అక్షరం *a_దత్తాంశం = pFile->a_దత్తాంశం;
  పూర్ణాంకం rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ప్రతిజ్ఞా( iAmt>=512 && iAmt<=65536 );
  ప్రతిజ్ఞా( (iAmt & (iAmt-1))==0 );
  ప్రతిజ్ఞా( pFile->sz_పుట<0 || pFile->sz_పుట==iAmt );
  pFile->sz_పుట = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(పరిమాణం(z_కీలకం), z_కీలకం, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, a_దత్తాంశం);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_కీలకం, a_దత్తాంశం);
  ఒకవేళ( 0==rc ){
    ఒకవేళ( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  తిరిగి rc;
}

/*
** Truncate an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 కొలత){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, కొలత));
  ప్రతిజ్ఞా( కొలత==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  తిరిగి SQLITE_OK;
}
స్థిర పూర్ణాంకం kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 కొలత){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ఒకవేళ( pFile->szDb>కొలత
   && pFile->sz_పుట>0
   && (కొలత % pFile->sz_పుట)==0
  ){
    అక్షరం z_కీలకం[50];
    గుర్తులేని పూర్ణాంకం pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, కొలత));
    pgno = 1 + కొలత/pFile->sz_పుట;
    pgnoMax = 2 + pFile->szDb/pFile->sz_పుట;
    అయితే( pgno<=pgnoMax ){
      sqlite3_snprintf(పరిమాణం(z_కీలకం), z_కీలకం, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_కీలకం);
      pgno++;
    }
    pFile->szDb = కొలత;
    తిరిగి kvvfs_Write_File_కొలత(pFile, కొలత) ? SQLITE_IOERR : SQLITE_OK;
  }
  తిరిగి SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfsSyncJrnl(sqlite3_file *pProtoFile, పూర్ణాంకం flags){
  పూర్ణాంకం i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  అక్షరం *z_బహిర్గతం;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  ఒకవేళ( pFile->nJrnl<=0 ){
    తిరిగి kvvfsTruncateJrnl(pProtoFile, 0);
  }
  z_బహిర్గతం = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  ఒకవేళ( z_బహిర్గతం==0 ){
    తిరిగి SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  చేయి{
    z_బహిర్గతం[i++] = 'a' + (n%26);
    n /= 26;
  }అయితే( n>0 );
  z_బహిర్గతం[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &z_బహిర్గతం[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", z_బహిర్గతం);
  sqlite3_free(z_బహిర్గతం);
  తిరిగి i ? SQLITE_IOERR : SQLITE_OK;
}
స్థిర పూర్ణాంకం kvvfsSyncDb(sqlite3_file *pProtoFile, పూర్ణాంకం flags){
  తిరిగి SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfs_File_కొలత_Jrnl(sqlite3_file *pProtoFile, sqlite_int64 *p_కొలత){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *p_కొలత = pFile->nJrnl;
  తిరిగి SQLITE_OK;
}
స్థిర పూర్ణాంకం kvvfs_File_కొలత_Db(sqlite3_file *pProtoFile, sqlite_int64 *p_కొలత){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  ఒకవేళ( pFile->szDb>=0 ){
    *p_కొలత = pFile->szDb;
  }లేకపోతే{
    *p_కొలత = kvvfs_Read_File_కొలత(pFile);
  }
  తిరిగి SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfs_తాళం(sqlite3_file *pProtoFile, పూర్ణాంకం e_తాళం){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ప్రతిజ్ఞా( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, e_తాళం));

  ఒకవేళ( e_తాళం!=SQLITE_తాళం_NONE ){
    pFile->szDb = kvvfs_Read_File_కొలత(pFile);
  }
  తిరిగి SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfsUnlock(sqlite3_file *pProtoFile, పూర్ణాంకం e_తాళం){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ప్రతిజ్ఞా( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, e_తాళం));
  ఒకవేళ( e_తాళం==SQLITE_తాళం_NONE ){
    pFile->szDb = -1;
  }
  తిరిగి SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfs_Check_Reserved_తాళం(sqlite3_file *pProtoFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *p_Res_బహిర్గతం = 0;
  తిరిగి SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfsFileControlJrnl(sqlite3_file *pProtoFile, పూర్ణాంకం op, శూన్య *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  తిరిగి SQLITE_NOTFOUND;
}
స్థిర పూర్ణాంకం kvvfsFileControlDb(sqlite3_file *pProtoFile, పూర్ణాంకం op, శూన్య *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  ఒకవేళ( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    పూర్ణాంకం rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    ఒకవేళ( pFile->szDb>0 && 0!=kvvfs_Write_File_కొలత(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    తిరిగి rc;
  }
  తిరిగి SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfs_Sector_కొలత(sqlite3_file *pFile){
  తిరిగి 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
స్థిర పూర్ణాంకం kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  తిరిగి 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
స్థిర పూర్ణాంకం kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pProtoFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ఒకవేళ( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  ప్రతిజ్ఞా(!pFile->zClass);
  ప్రతిజ్ఞా(!pFile->a_దత్తాంశం);
  ప్రతిజ్ఞా(!pFile->aJrnl);
  ప్రతిజ్ఞా(!pFile->nJrnl);
  ప్రతిజ్ఞా(!pFile->పునాది.pMethods);
  pFile->sz_పుట = -1;
  pFile->szDb = -1;
  ఒకవేళ( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->పునాది.pMethods = &kvvfs_jrnl_io_methods;
    ఒకవేళ( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }లేకపోతే ఒకవేళ( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }లేకపోతే{
    pFile->isJournal = 0;
    pFile->పునాది.pMethods = &kvvfs_db_io_methods;
  }
  ఒకవేళ( !pFile->zClass ){
#నిర్వచితమైతే SQLITE_WASM
    ఒకవేళ( అక్షరశ్రేణి_పొడవు(zName) >= (KVRECORD_కీలకం_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      తిరిగి SQLITE_CANTOPEN;
    }
#లేకపోతే
    ఒకవేళ( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      తిరిగి SQLITE_CANTOPEN;
    }
#అయితే_ముగింపు
    pFile->zClass = zName;
  }
  pFile->a_దత్తాంశం = sqlite3_malloc64(SQLITE_KVOS_SZ);
  ఒకవేళ( pFile->a_దత్తాంశం==0 ){
    తిరిగి SQLITE_NOMEM;
  }
  తిరిగి SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
స్థిర పూర్ణాంకం kvvfsDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  పూర్ణాంకం rc /* The JS impl can fail with OOM in argument conversion */;
  ఒకవేళ( strcmp(z_మార్గం, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }లేకపోతే
  ఒకవేళ( strcmp(z_మార్గం, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  లేకపోతే{
    rc = 0;
  }
  తిరిగి rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
స్థిర పూర్ణాంకం kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  స్థిరరాశి అక్షరం *z_మార్గం,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", z_మార్గం));
#ఒకవేళ 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  స్థిరరాశి అక్షరం *z_కీలకం = (0==sqlite3_strglob("*-journal", z_మార్గం))
    ? "jrnl" : "sz";
  *p_Res_బహిర్గతం =
    sqlite3KvvfsMethods.xRcrdRead(z_మార్గం, z_కీలకం, 0, 0)>0;
#లేకపోతే
  ఒకవేళ( strcmp(z_మార్గం, "local-journal")==0 ){
    *p_Res_బహిర్గతం =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }లేకపోతే
  ఒకవేళ( strcmp(z_మార్గం, "session-journal")==0 ){
    *p_Res_బహిర్గతం =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }లేకపోతే
  ఒకవేళ( strcmp(z_మార్గం, "local")==0 ){
    *p_Res_బహిర్గతం =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }లేకపోతే
  ఒకవేళ( strcmp(z_మార్గం, "session")==0 ){
    *p_Res_బహిర్గతం =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }లేకపోతే
  {
    *p_Res_బహిర్గతం = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#అయితే_ముగింపు
  SQLITE_KV_LOG(("xAccess returns %d\n",*p_Res_బహిర్గతం));
  తిరిగి SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
స్థిర పూర్ణాంకం kvvfs_నిండిన_Pathname(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *z_మార్గం,
  పూర్ణాంకం n_బహిర్గతం,
  అక్షరం *z_బహిర్గతం
){
  size_t n_మార్గం;
#నిర్వచితమైతే SQLITE_OS_KV_ALWAYS_LOCAL
  z_మార్గం = "local";
#అయితే_ముగింపు
  n_మార్గం = అక్షరశ్రేణి_పొడవు(z_మార్గం);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", z_మార్గం));
  ఒకవేళ( n_బహిర్గతం<n_మార్గం+1 ) n_మార్గం = n_బహిర్గతం - 1;
  జ్ఞాపకం_నకలు(z_బహిర్గతం, z_మార్గం, n_మార్గం);
  z_బహిర్గతం[n_మార్గం] = 0;
  తిరిగి SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
స్థిర శూన్య *kvvfsDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  తిరిగి 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
స్థిర పూర్ణాంకం kvvfsRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  memset(z_Buf_బహిర్గతం, 0, nByte);
  తిరిగి nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
స్థిర పూర్ణాంకం kvvfsSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  తిరిగి SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
స్థిర పూర్ణాంకం kvvfs_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *p_సమయం_బహిర్గతం){
  sqlite3_int64 i = 0;
  పూర్ణాంకం rc;
  rc = kvvfs_Current_సమయం_Int64(0, &i);
  *p_సమయం_బహిర్గతం = i/86400000.0;
  తిరిగి rc;
}
#అన్తర్భావ <sys/time.h>
స్థిర పూర్ణాంకం kvvfs_Current_సమయం_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_సమయం_బహిర్గతం){
  స్థిర స్థిరరాశి sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  నిర్మాణం timeval sNow;
  (శూన్య)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *p_సమయం_బహిర్గతం = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#ఒకవేళ SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
పూర్ణాంకం sqlite3_os_init(శూన్య){
  తిరిగి sqlite3_vfs_రిజిస్టరు(&sqlite3OsKvvfsObject, 1);
}
పూర్ణాంకం sqlite3_os_చివర(శూన్య){
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_OS_KV */

#ఒకవేళ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
పూర్ణాంకం sqlite3KvvfsInit(శూన్య){
  తిరిగి sqlite3_vfs_రిజిస్టరు(&sqlite3OsKvvfsObject, 0);
}
#అయితే_ముగింపు
