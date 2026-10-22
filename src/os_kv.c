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
#অন্তর্ভাব <sqliteInt.h>
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
প্রকারলেখ কাঠামো KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
কাঠামো KVVfsFile {
  sqlite3_file base;              /* IO methods */
  ধ্রুবক অক্ষর *zClass;             /* Storage class */
  পূর্ণসংখ্যা isJournal;                  /* True if this is a journal file */
  unsigned পূর্ণসংখ্যা nJrnl;             /* Space allocated for aJrnl[] */
  অক্ষর *aJrnl;                    /* Journal content */
  পূর্ণসংখ্যা szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  অক্ষর *a_তথ্য;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
স্থির পূর্ণসংখ্যা kvvfsClose(sqlite3_file*);
স্থির পূর্ণসংখ্যা kvvfsReadDb(sqlite3_file*, শূন্য*, পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা kvvfsReadJrnl(sqlite3_file*, শূন্য*, পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা kvvfsWriteDb(sqlite3_file*,ধ্রুবক শূন্য*,পূর্ণসংখ্যা iAmt, sqlite3_int64);
স্থির পূর্ণসংখ্যা kvvfsWriteJrnl(sqlite3_file*,ধ্রুবক শূন্য*,পূর্ণসংখ্যা iAmt, sqlite3_int64);
স্থির পূর্ণসংখ্যা kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 আকার_2);
স্থির পূর্ণসংখ্যা kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 আকার_2);
স্থির পূর্ণসংখ্যা kvvfsSyncDb(sqlite3_file*, পূর্ণসংখ্যা flags);
স্থির পূর্ণসংখ্যা kvvfsSyncJrnl(sqlite3_file*, পূর্ণসংখ্যা flags);
স্থির পূর্ণসংখ্যা kvvfs_File_আকার_Db(sqlite3_file*, sqlite3_int64 *p_আকার);
স্থির পূর্ণসংখ্যা kvvfs_File_আকার_Jrnl(sqlite3_file*, sqlite3_int64 *p_আকার);
স্থির পূর্ণসংখ্যা kvvfsLock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা kvvfsUnlock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা kvvfsCheckReservedLock(sqlite3_file*, পূর্ণসংখ্যা *pResOut);
স্থির পূর্ণসংখ্যা kvvfsFileControlDb(sqlite3_file*, পূর্ণসংখ্যা op, শূন্য *pArg);
স্থির পূর্ণসংখ্যা kvvfsFileControlJrnl(sqlite3_file*, পূর্ণসংখ্যা op, শূন্য *pArg);
স্থির পূর্ণসংখ্যা kvvfs_Sector_আকার(sqlite3_file*);
স্থির পূর্ণসংখ্যা kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
স্থির পূর্ণসংখ্যা kvvfsOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *, sqlite3_file*, পূর্ণসংখ্যা , পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা kvvfsDelete(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা syncDir);
স্থির পূর্ণসংখ্যা kvvfsAccess(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা flags, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা kvvfsFullPathname(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা, অক্ষর *zOut);
স্থির শূন্য *kvvfsDlOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *zFilename);
স্থির পূর্ণসংখ্যা kvvfsRandomness(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *zOut);
স্থির পূর্ণসংখ্যা kvvfsSleep(sqlite3_vfs*, পূর্ণসংখ্যা microseconds);
স্থির পূর্ণসংখ্যা kvvfsCurrentTime(sqlite3_vfs*, দ্বিগুণ*);
স্থির পূর্ণসংখ্যা kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

স্থির sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  আকার(KVVfsFile),              /* szOsFile */
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
  kvvfs_File_আকার_Db,                /* xFileSize */
  kvvfsLock,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfsCheckReservedLock,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfs_Sector_আকার,                /* xSectorSize */
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
  kvvfs_File_আকার_Jrnl,              /* xFileSize */
  kvvfsLock,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfsCheckReservedLock,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfs_Sector_আকার,                /* xSectorSize */
  kvvfsDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                              /* xShmMap */
  0,                              /* xShmLock */
  0,                              /* xShmBarrier */
  0,                              /* xShmUnmap */
  0,                              /* xFetch */
  0                               /* xUnfetch */
};

/****** Storage subsystem **************************************************/
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
স্থির পূর্ণসংখ্যা kvrecordWrite(ধ্রুবক অক্ষর*, ধ্রুবক অক্ষর *z_চাবি, ধ্রুবক অক্ষর *z_তথ্য);
স্থির পূর্ণসংখ্যা kvrecordDelete(ধ্রুবক অক্ষর*, ধ্রুবক অক্ষর *z_চাবি);
স্থির পূর্ণসংখ্যা kvrecordRead(ধ্রুবক অক্ষর*, ধ্রুবক অক্ষর *z_চাবি, অক্ষর *zBuf, পূর্ণসংখ্যা nBuf);
#endif
#ifndef KVRECORD_চাবি_SZ
#define KVRECORD_চাবি_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
স্থির শূন্য kvrecord_Make_চাবি(
  ধ্রুবক অক্ষর *zClass,
  ধ্রুবক অক্ষর *z_চাবি_In,
  অক্ষর *z_চাবি_Out
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
স্থির পূর্ণসংখ্যা kvrecordWrite(
  ধ্রুবক অক্ষর *zClass,
  ধ্রুবক অক্ষর *z_চাবি,
  ধ্রুবক অক্ষর *z_তথ্য
){
  FILE *fd;
  অক্ষর zXKey[KVRECORD_চাবি_SZ];
  kvrecord_Make_চাবি(zClass, z_চাবি, zXKey);
  fd = fopen(zXKey, "wb");
  যদি( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (পূর্ণসংখ্যা)strlen(z_তথ্য), z_তথ্য,
                 strlen(z_তথ্য)>50 ? "..." : ""));
    fputs(z_তথ্য, fd);
    fclose(fd);
    ফেরত 0;
  }অন্যথা{
    ফেরত 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
স্থির পূর্ণসংখ্যা kvrecordDelete(ধ্রুবক অক্ষর *zClass, ধ্রুবক অক্ষর *z_চাবি){
  অক্ষর zXKey[KVRECORD_চাবি_SZ];
  kvrecord_Make_চাবি(zClass, z_চাবি, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  ফেরত 0;
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
স্থির পূর্ণসংখ্যা kvrecordRead(
  ধ্রুবক অক্ষর *zClass,
  ধ্রুবক অক্ষর *z_চাবি,
  অক্ষর *zBuf,
  পূর্ণসংখ্যা nBuf
){
  FILE *fd;
  কাঠামো stat buf;
  অক্ষর zXKey[KVRECORD_চাবি_SZ];
  kvrecord_Make_চাবি(zClass, z_চাবি, zXKey);
  যদি( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ফেরত -1;
  }
  যদি( nBuf<=0 ){
    ফেরত (পূর্ণসংখ্যা)buf.st_আকার;
  }অন্যথা যদি( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (পূর্ণসংখ্যা)buf.st_আকার));
    ফেরত (পূর্ণসংখ্যা)buf.st_আকার;
  }
  যদি( nBuf > buf.st_আকার + 1 ){
    nBuf = buf.st_আকার + 1;
  }
  fd = fopen(zXKey, "rb");
  যদি( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ফেরত -1;
  }অন্যথা{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    ফেরত (পূর্ণসংখ্যা)n;
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
প্রকারলেখ কাঠামো sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
কাঠামো sqlite3_kvvfs_methods {
  পূর্ণসংখ্যা (*xRcrdRead)(ধ্রুবক অক্ষর*, ধ্রুবক অক্ষর *z_চাবি, অক্ষর *zBuf, পূর্ণসংখ্যা nBuf);
  পূর্ণসংখ্যা (*xRcrdWrite)(ধ্রুবক অক্ষর*, ধ্রুবক অক্ষর *z_চাবি, ধ্রুবক অক্ষর *z_তথ্য);
  পূর্ণসংখ্যা (*xRcrdDelete)(ধ্রুবক অক্ষর*, ধ্রুবক অক্ষর *z_চাবি);
  ধ্রুবক পূর্ণসংখ্যা n_চাবি_আকার;
  ধ্রুবক পূর্ণসংখ্যা n_বাফার_আকার;
#ifndef SQLITE_WASM
#  define MAYBE_CONST ধ্রুবক
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
ধ্রুবক
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
  .n_চাবি_আকার        = KVRECORD_চাবি_SZ,
  .n_বাফার_আকার     = SQLITE_KVOS_SZ,
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
পূর্ণসংখ্যা kvvfsEncode(ধ্রুবক অক্ষর *a_তথ্য, পূর্ণসংখ্যা n_তথ্য, অক্ষর *aOut){
  পূর্ণসংখ্যা i, j;
  ধ্রুবক unsigned অক্ষর *a = (ধ্রুবক unsigned অক্ষর*)a_তথ্য;
  জন্য(i=j=0; i<n_তথ্য; i++){
    unsigned অক্ষর c = a[i];
    যদি( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }অন্যথা{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      পূর্ণসংখ্যা k;
      জন্য(k=1; i+k<n_তথ্য && a[i+k]==0; k++){}
      i += k-1;
      যখন( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  ফেরত j;
}

স্থির ধ্রুবক signed অক্ষর kvvfs_Hex_মান[256] = {
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
পূর্ণসংখ্যা kvvfsDecode(ধ্রুবক অক্ষর *a, অক্ষর *aOut, পূর্ণসংখ্যা nOut){
  পূর্ণসংখ্যা i, j;
  পূর্ণসংখ্যা c;
  ধ্রুবক unsigned অক্ষর *aIn = (ধ্রুবক unsigned অক্ষর*)a;
  i = 0;
  j = 0;
  যখন( 1 ){
    c = kvvfs_Hex_মান[aIn[i]];
    যদি( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      যদি( c==0 ) বিরতি;
      যখন( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        যদি( n>nOut ) ফেরত -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      যদি( j+n>nOut ) ফেরত -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      যদি( c==0 || mult==1 ) বিরতি; /* progress stalled if mult==1 */
    }অন্যথা যদি( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_মান[aIn[++i]];
      যদি( c<0 ) ফেরত -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }অন্যথা{
      ফেরত -1;
    }
  }
  ফেরত j;
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
  ধ্রুবক অক্ষর *zTxt,      /* Text encoding.  Zero-terminated */
  পূর্ণসংখ্যা nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned পূর্ণসংখ্যা n = 0, mult;
  পূর্ণসংখ্যা c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  যখন( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  যদি( ' '!=zTxt[i++] ){
    /* Malformed input */
    ফেরত;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  যদি( pFile->aJrnl==0 ){
    ফেরত;
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
স্থির sqlite3_int64 kvvfs_Read_File_আকার(KVVfsFile *pFile){
  অক্ষর z_তথ্য[50];
  z_তথ্য[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", z_তথ্য,
                                আকার(z_তথ্য)-1);
  ফেরত strtoll(z_তথ্য, 0, 0);
}
স্থির পূর্ণসংখ্যা kvvfs_Write_File_আকার(KVVfsFile *pFile, sqlite3_int64 sz){
  অক্ষর z_তথ্য[50];
  sqlite3_snprintf(আকার(z_তথ্য), z_তথ্য, "%lld", sz);
  ফেরত sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", z_তথ্য);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->a_তথ্য);
  memset(pFile, 0, আকার(*pFile));
  ফেরত SQLITE_OK;
}

/*
** Read from the -journal file.
*/
স্থির পূর্ণসংখ্যা kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  শূন্য *zBuf,
  পূর্ণসংখ্যা iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  প্রতিজ্ঞা( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  যদি( pFile->aJrnl==0 ){
    পূর্ণসংখ্যা rc;
    পূর্ণসংখ্যা szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    অক্ষর *aTxt;
    যদি( szTxt<=4 ){
      ফেরত SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    যদি( aTxt==0 ) ফেরত SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    যদি( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    যদি( rc ) ফেরত rc;
    যদি( pFile->aJrnl==0 ) ফেরত SQLITE_IOERR;
  }
  যদি( iOfst+iAmt>pFile->nJrnl ){
    ফেরত SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  ফেরত SQLITE_OK;
}

/*
** Read from the database file.
*/
স্থির পূর্ণসংখ্যা kvvfsReadDb(
  sqlite3_file *pProtoFile,
  শূন্য *zBuf,
  পূর্ণসংখ্যা iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned পূর্ণসংখ্যা pgno;
  পূর্ণসংখ্যা got, n;
  অক্ষর z_চাবি[30];
  অক্ষর *a_তথ্য = pFile->a_তথ্য;
  প্রতিজ্ঞা( iOfst>=0 );
  প্রতিজ্ঞা( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  যদি( iOfst+iAmt>=512 ){
    যদি( (iOfst % iAmt)!=0 ){
      ফেরত SQLITE_IOERR_READ;
    }
    যদি( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      ফেরত SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }অন্যথা{
    pgno = 1;
  }
  sqlite3_snprintf(আকার(z_চাবি), z_চাবি, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_চাবি,
                                      a_তথ্য, SQLITE_KVOS_SZ-1);
  যদি( got<0 ){
    n = 0;
  }অন্যথা{
    a_তথ্য[got] = 0;
    যদি( iOfst+iAmt<512 ){
      পূর্ণসংখ্যা k = iOfst+iAmt;
      a_তথ্য[k*2] = 0;
      n = kvvfsDecode(a_তথ্য, &a_তথ্য[2000], SQLITE_KVOS_SZ-2000);
      যদি( n>=iOfst+iAmt ){
        memcpy(zBuf, &a_তথ্য[2000+iOfst], iAmt);
        n = iAmt;
      }অন্যথা{
        n = 0;
      }
    }অন্যথা{
      n = kvvfsDecode(a_তথ্য, zBuf, iAmt);
    }
  }
  যদি( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    ফেরত SQLITE_IOERR_SHORT_READ;
  }
  ফেরত SQLITE_OK;
}


/*
** Write into the -journal file.
*/
স্থির পূর্ণসংখ্যা kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ধ্রুবক শূন্য *zBuf,
  পূর্ণসংখ্যা iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  যদি( iEnd>=0x10000000 ) ফেরত SQLITE_FULL;
  যদি( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    অক্ষর *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    যদি( aNew==0 ){
      ফেরত SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    যদি( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  ফেরত SQLITE_OK;
}

/*
** Write into the database file.
*/
স্থির পূর্ণসংখ্যা kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ধ্রুবক শূন্য *zBuf,
  পূর্ণসংখ্যা iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned পূর্ণসংখ্যা pgno;
  অক্ষর z_চাবি[30];
  অক্ষর *a_তথ্য = pFile->a_তথ্য;
  পূর্ণসংখ্যা rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  প্রতিজ্ঞা( iAmt>=512 && iAmt<=65536 );
  প্রতিজ্ঞা( (iAmt & (iAmt-1))==0 );
  প্রতিজ্ঞা( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(আকার(z_চাবি), z_চাবি, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, a_তথ্য);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_চাবি, a_তথ্য);
  যদি( 0==rc ){
    যদি( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  ফেরত rc;
}

/*
** Truncate an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 আকার_2){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, আকার_2));
  প্রতিজ্ঞা( আকার_2==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ফেরত SQLITE_OK;
}
স্থির পূর্ণসংখ্যা kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 আকার_2){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  যদি( pFile->szDb>আকার_2
   && pFile->szPage>0
   && (আকার_2 % pFile->szPage)==0
  ){
    অক্ষর z_চাবি[50];
    unsigned পূর্ণসংখ্যা pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, আকার_2));
    pgno = 1 + আকার_2/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    যখন( pgno<=pgnoMax ){
      sqlite3_snprintf(আকার(z_চাবি), z_চাবি, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_চাবি);
      pgno++;
    }
    pFile->szDb = আকার_2;
    ফেরত kvvfs_Write_File_আকার(pFile, আকার_2) ? SQLITE_IOERR : SQLITE_OK;
  }
  ফেরত SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfsSyncJrnl(sqlite3_file *pProtoFile, পূর্ণসংখ্যা flags){
  পূর্ণসংখ্যা i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  অক্ষর *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  যদি( pFile->nJrnl<=0 ){
    ফেরত kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  যদি( zOut==0 ){
    ফেরত SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  করো{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }যখন( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  ফেরত i ? SQLITE_IOERR : SQLITE_OK;
}
স্থির পূর্ণসংখ্যা kvvfsSyncDb(sqlite3_file *pProtoFile, পূর্ণসংখ্যা flags){
  ফেরত SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfs_File_আকার_Jrnl(sqlite3_file *pProtoFile, sqlite_int64 *p_আকার){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *p_আকার = pFile->nJrnl;
  ফেরত SQLITE_OK;
}
স্থির পূর্ণসংখ্যা kvvfs_File_আকার_Db(sqlite3_file *pProtoFile, sqlite_int64 *p_আকার){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  যদি( pFile->szDb>=0 ){
    *p_আকার = pFile->szDb;
  }অন্যথা{
    *p_আকার = kvvfs_Read_File_আকার(pFile);
  }
  ফেরত SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfsLock(sqlite3_file *pProtoFile, পূর্ণসংখ্যা eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  প্রতিজ্ঞা( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  যদি( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfs_Read_File_আকার(pFile);
  }
  ফেরত SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfsUnlock(sqlite3_file *pProtoFile, পূর্ণসংখ্যা eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  প্রতিজ্ঞা( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  যদি( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  ফেরত SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfsCheckReservedLock(sqlite3_file *pProtoFile, পূর্ণসংখ্যা *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  ফেরত SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfsFileControlJrnl(sqlite3_file *pProtoFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  ফেরত SQLITE_NOTFOUND;
}
স্থির পূর্ণসংখ্যা kvvfsFileControlDb(sqlite3_file *pProtoFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  যদি( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    পূর্ণসংখ্যা rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    যদি( pFile->szDb>0 && 0!=kvvfs_Write_File_আকার(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    ফেরত rc;
  }
  ফেরত SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfs_Sector_আকার(sqlite3_file *pFile){
  ফেরত 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
স্থির পূর্ণসংখ্যা kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  ফেরত 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
স্থির পূর্ণসংখ্যা kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ধ্রুবক অক্ষর *zName,
  sqlite3_file *pProtoFile,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  যদি( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  প্রতিজ্ঞা(!pFile->zClass);
  প্রতিজ্ঞা(!pFile->a_তথ্য);
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
      ফেরত SQLITE_CANTOPEN;
    }
#অন্যথা
    যদি( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      ফেরত SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->a_তথ্য = sqlite3_malloc64(SQLITE_KVOS_SZ);
  যদি( pFile->a_তথ্য==0 ){
    ফেরত SQLITE_NOMEM;
  }
  ফেরত SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণসংখ্যা kvvfsDelete(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা dirSync){
  পূর্ণসংখ্যা rc /* The JS impl can fail with OOM in argument conversion */;
  যদি( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }অন্যথা
  যদি( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  অন্যথা{
    rc = 0;
  }
  ফেরত rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণসংখ্যা kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ধ্রুবক অক্ষর *zPath,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pResOut
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
  ধ্রুবক অক্ষর *z_চাবি = (0==sqlite3_strglob("*-journal", zPath))
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
  ফেরত SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণসংখ্যা kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  ধ্রুবক অক্ষর *zPath,
  পূর্ণসংখ্যা nOut,
  অক্ষর *zOut
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
  ফেরত SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *kvvfsDlOpen(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath){
  ফেরত 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
স্থির পূর্ণসংখ্যা kvvfsRandomness(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *zBufOut){
  memset(zBufOut, 0, nByte);
  ফেরত nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
স্থির পূর্ণসংখ্যা kvvfsSleep(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nMicro){
  ফেরত SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণসংখ্যা kvvfsCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  sqlite3_int64 i = 0;
  পূর্ণসংখ্যা rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  ফেরত rc;
}
#অন্তর্ভাব <sys/time.h>
স্থির পূর্ণসংখ্যা kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  স্থির ধ্রুবক sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  কাঠামো timeval sNow;
  (শূন্য)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  ফেরত SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#যদি SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
পূর্ণসংখ্যা sqlite3_os_init(শূন্য){
  ফেরত sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
পূর্ণসংখ্যা sqlite3_os_end(শূন্য){
  ফেরত SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#যদি SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
পূর্ণসংখ্যা sqlite3KvvfsInit(শূন্য){
  ফেরত sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
