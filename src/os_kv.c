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
#അന്തര്ഭാവ <sqliteInt.h>
#എങ്കിൽ SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#എങ്കിൽ 0
#define SQLITE_KV_TRACE(X)  printf X
#അല്ലെങ്കിൽ
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#എങ്കിൽ 0
#define SQLITE_KV_LOG(X)  printf X
#അല്ലെങ്കിൽ
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
തരനിർവചനം ഘടന KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ഘടന KVVfsFile {
  sqlite3_file base;              /* IO methods */
  സ്ഥിരം അക്ഷരം *zClass;             /* Storage class */
  പൂർണ്ണസംഖ്യ isJournal;                  /* True if this is a journal file */
  unsigned പൂർണ്ണസംഖ്യ nJrnl;             /* Space allocated for aJrnl[] */
  അക്ഷരം *aJrnl;                    /* Journal content */
  പൂർണ്ണസംഖ്യ szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  അക്ഷരം *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsClose(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsReadDb(sqlite3_file*, ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsReadJrnl(sqlite3_file*, ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsWriteDb(sqlite3_file*,സ്ഥിരം ശൂന്യം*,പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsWriteJrnl(sqlite3_file*,സ്ഥിരം ശൂന്യം*,പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsSyncDb(sqlite3_file*, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsSyncJrnl(sqlite3_file*, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsLock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsUnlock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsCheckReservedLock(sqlite3_file*, പൂർണ്ണസംഖ്യ *pResOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFileControlDb(sqlite3_file*, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFileControlJrnl(sqlite3_file*, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsSectorSize(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *, sqlite3_file*, പൂർണ്ണസംഖ്യ , പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsDelete(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ syncDir);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsAccess(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFullPathname(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ, അക്ഷരം *zOut);
സ്ഥിതം ശൂന്യം *kvvfsDlOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zFilename);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsRandomness(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsSleep(sqlite3_vfs*, പൂർണ്ണസംഖ്യ microseconds);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsCurrentTime(sqlite3_vfs*, ഇരട്ട*);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

സ്ഥിതം sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  വലുപ്പം(KVVfsFile),              /* szOsFile */
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
സ്ഥിതം sqlite3_io_methods kvvfs_db_io_methods = {
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
സ്ഥിതം sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
സ്ഥിതം പൂർണ്ണസംഖ്യ kvrecordWrite(സ്ഥിരം അക്ഷരം*, സ്ഥിരം അക്ഷരം *z_താക്കോൽ, സ്ഥിരം അക്ഷരം *zData);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvrecordDelete(സ്ഥിരം അക്ഷരം*, സ്ഥിരം അക്ഷരം *z_താക്കോൽ);
സ്ഥിതം പൂർണ്ണസംഖ്യ kvrecordRead(സ്ഥിരം അക്ഷരം*, സ്ഥിരം അക്ഷരം *z_താക്കോൽ, അക്ഷരം *zBuf, പൂർണ്ണസംഖ്യ nBuf);
#endif
#ifndef KVRECORD_താക്കോൽ_SZ
#define KVRECORD_താക്കോൽ_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
സ്ഥിതം ശൂന്യം kvrecord_Make_താക്കോൽ(
  സ്ഥിരം അക്ഷരം *zClass,
  സ്ഥിരം അക്ഷരം *z_താക്കോൽ_In,
  അക്ഷരം *z_താക്കോൽ_Out
){
  പ്രതിജ്ഞാ( z_താക്കോൽ_In );
  പ്രതിജ്ഞാ( z_താക്കോൽ_Out );
  പ്രതിജ്ഞാ( zClass );
  sqlite3_snprintf(KVRECORD_താക്കോൽ_SZ, z_താക്കോൽ_Out, "kvvfs-%s-%s",
                   zClass, z_താക്കോൽ_In);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ kvrecordWrite(
  സ്ഥിരം അക്ഷരം *zClass,
  സ്ഥിരം അക്ഷരം *z_താക്കോൽ,
  സ്ഥിരം അക്ഷരം *zData
){
  FILE *fd;
  അക്ഷരം zXKey[KVRECORD_താക്കോൽ_SZ];
  kvrecord_Make_താക്കോൽ(zClass, z_താക്കോൽ, zXKey);
  fd = fopen(zXKey, "wb");
  എങ്കിൽ( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (പൂർണ്ണസംഖ്യ)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    തിരികെ 0;
  }അല്ലെങ്കിൽ{
    തിരികെ 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvrecordDelete(സ്ഥിരം അക്ഷരം *zClass, സ്ഥിരം അക്ഷരം *z_താക്കോൽ){
  അക്ഷരം zXKey[KVRECORD_താക്കോൽ_SZ];
  kvrecord_Make_താക്കോൽ(zClass, z_താക്കോൽ, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ kvrecordRead(
  സ്ഥിരം അക്ഷരം *zClass,
  സ്ഥിരം അക്ഷരം *z_താക്കോൽ,
  അക്ഷരം *zBuf,
  പൂർണ്ണസംഖ്യ nBuf
){
  FILE *fd;
  ഘടന stat buf;
  അക്ഷരം zXKey[KVRECORD_താക്കോൽ_SZ];
  kvrecord_Make_താക്കോൽ(zClass, z_താക്കോൽ, zXKey);
  എങ്കിൽ( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    തിരികെ -1;
  }
  എങ്കിൽ( nBuf<=0 ){
    തിരികെ (പൂർണ്ണസംഖ്യ)buf.st_size;
  }അല്ലെങ്കിൽ എങ്കിൽ( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (പൂർണ്ണസംഖ്യ)buf.st_size));
    തിരികെ (പൂർണ്ണസംഖ്യ)buf.st_size;
  }
  എങ്കിൽ( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  എങ്കിൽ( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    തിരികെ -1;
  }അല്ലെങ്കിൽ{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    തിരികെ (പൂർണ്ണസംഖ്യ)n;
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
തരനിർവചനം ഘടന sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
ഘടന sqlite3_kvvfs_methods {
  പൂർണ്ണസംഖ്യ (*xRcrdRead)(സ്ഥിരം അക്ഷരം*, സ്ഥിരം അക്ഷരം *z_താക്കോൽ, അക്ഷരം *zBuf, പൂർണ്ണസംഖ്യ nBuf);
  പൂർണ്ണസംഖ്യ (*xRcrdWrite)(സ്ഥിരം അക്ഷരം*, സ്ഥിരം അക്ഷരം *z_താക്കോൽ, സ്ഥിരം അക്ഷരം *zData);
  പൂർണ്ണസംഖ്യ (*xRcrdDelete)(സ്ഥിരം അക്ഷരം*, സ്ഥിരം അക്ഷരം *z_താക്കോൽ);
  സ്ഥിരം പൂർണ്ണസംഖ്യ n_താക്കോൽ_Size;
  സ്ഥിരം പൂർണ്ണസംഖ്യ nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST സ്ഥിരം
#അല്ലെങ്കിൽ
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
സ്ഥിരം
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#അല്ലെങ്കിൽ
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .n_താക്കോൽ_Size        = KVRECORD_താക്കോൽ_SZ,
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
സ്ഥിതം
#endif
പൂർണ്ണസംഖ്യ kvvfsEncode(സ്ഥിരം അക്ഷരം *aData, പൂർണ്ണസംഖ്യ nData, അക്ഷരം *aOut){
  പൂർണ്ണസംഖ്യ i, j;
  സ്ഥിരം unsigned അക്ഷരം *a = (സ്ഥിരം unsigned അക്ഷരം*)aData;
  വേണ്ടി(i=j=0; i<nData; i++){
    unsigned അക്ഷരം c = a[i];
    എങ്കിൽ( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }അല്ലെങ്കിൽ{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      പൂർണ്ണസംഖ്യ k;
      വേണ്ടി(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      ആകുമ്പോൾ( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  തിരികെ j;
}

സ്ഥിതം സ്ഥിരം signed അക്ഷരം kvvfs_Hex_മൂല്യം[256] = {
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
സ്ഥിതം
#endif
പൂർണ്ണസംഖ്യ kvvfsDecode(സ്ഥിരം അക്ഷരം *a, അക്ഷരം *aOut, പൂർണ്ണസംഖ്യ nOut){
  പൂർണ്ണസംഖ്യ i, j;
  പൂർണ്ണസംഖ്യ c;
  സ്ഥിരം unsigned അക്ഷരം *aIn = (സ്ഥിരം unsigned അക്ഷരം*)a;
  i = 0;
  j = 0;
  ആകുമ്പോൾ( 1 ){
    c = kvvfs_Hex_മൂല്യം[aIn[i]];
    എങ്കിൽ( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      എങ്കിൽ( c==0 ) നിർത്തുക;
      ആകുമ്പോൾ( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        എങ്കിൽ( n>nOut ) തിരികെ -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      എങ്കിൽ( j+n>nOut ) തിരികെ -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      എങ്കിൽ( c==0 || mult==1 ) നിർത്തുക; /* progress stalled if mult==1 */
    }അല്ലെങ്കിൽ എങ്കിൽ( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_മൂല്യം[aIn[++i]];
      എങ്കിൽ( c<0 ) തിരികെ -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }അല്ലെങ്കിൽ{
      തിരികെ -1;
    }
  }
  തിരികെ j;
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
സ്ഥിതം ശൂന്യം kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  സ്ഥിരം അക്ഷരം *zTxt,      /* Text encoding.  Zero-terminated */
  പൂർണ്ണസംഖ്യ nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned പൂർണ്ണസംഖ്യ n = 0, mult;
  പൂർണ്ണസംഖ്യ c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ആകുമ്പോൾ( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  എങ്കിൽ( ' '!=zTxt[i++] ){
    /* Malformed input */
    തിരികെ;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  എങ്കിൽ( pFile->aJrnl==0 ){
    തിരികെ;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  എങ്കിൽ( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
സ്ഥിതം sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  അക്ഷരം zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                വലുപ്പം(zData)-1);
  തിരികെ strtoll(zData, 0, 0);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  അക്ഷരം zData[50];
  sqlite3_snprintf(വലുപ്പം(zData), zData, "%lld", sz);
  തിരികെ sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, വലുപ്പം(*pFile));
  തിരികെ SQLITE_OK;
}

/*
** Read from the -journal file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  ശൂന്യം *zBuf,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  പ്രതിജ്ഞാ( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  എങ്കിൽ( pFile->aJrnl==0 ){
    പൂർണ്ണസംഖ്യ rc;
    പൂർണ്ണസംഖ്യ szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    അക്ഷരം *aTxt;
    എങ്കിൽ( szTxt<=4 ){
      തിരികെ SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    എങ്കിൽ( aTxt==0 ) തിരികെ SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    എങ്കിൽ( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    എങ്കിൽ( rc ) തിരികെ rc;
    എങ്കിൽ( pFile->aJrnl==0 ) തിരികെ SQLITE_IOERR;
  }
  എങ്കിൽ( iOfst+iAmt>pFile->nJrnl ){
    തിരികെ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  തിരികെ SQLITE_OK;
}

/*
** Read from the database file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsReadDb(
  sqlite3_file *pProtoFile,
  ശൂന്യം *zBuf,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned പൂർണ്ണസംഖ്യ pgno;
  പൂർണ്ണസംഖ്യ got, n;
  അക്ഷരം z_താക്കോൽ[30];
  അക്ഷരം *aData = pFile->aData;
  പ്രതിജ്ഞാ( iOfst>=0 );
  പ്രതിജ്ഞാ( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  എങ്കിൽ( iOfst+iAmt>=512 ){
    എങ്കിൽ( (iOfst % iAmt)!=0 ){
      തിരികെ SQLITE_IOERR_READ;
    }
    എങ്കിൽ( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      തിരികെ SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }അല്ലെങ്കിൽ{
    pgno = 1;
  }
  sqlite3_snprintf(വലുപ്പം(z_താക്കോൽ), z_താക്കോൽ, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_താക്കോൽ,
                                      aData, SQLITE_KVOS_SZ-1);
  എങ്കിൽ( got<0 ){
    n = 0;
  }അല്ലെങ്കിൽ{
    aData[got] = 0;
    എങ്കിൽ( iOfst+iAmt<512 ){
      പൂർണ്ണസംഖ്യ k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      എങ്കിൽ( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }അല്ലെങ്കിൽ{
        n = 0;
      }
    }അല്ലെങ്കിൽ{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  എങ്കിൽ( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    തിരികെ SQLITE_IOERR_SHORT_READ;
  }
  തിരികെ SQLITE_OK;
}


/*
** Write into the -journal file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  സ്ഥിരം ശൂന്യം *zBuf,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  എങ്കിൽ( iEnd>=0x10000000 ) തിരികെ SQLITE_FULL;
  എങ്കിൽ( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    അക്ഷരം *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    എങ്കിൽ( aNew==0 ){
      തിരികെ SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    എങ്കിൽ( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  തിരികെ SQLITE_OK;
}

/*
** Write into the database file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  സ്ഥിരം ശൂന്യം *zBuf,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned പൂർണ്ണസംഖ്യ pgno;
  അക്ഷരം z_താക്കോൽ[30];
  അക്ഷരം *aData = pFile->aData;
  പൂർണ്ണസംഖ്യ rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  പ്രതിജ്ഞാ( iAmt>=512 && iAmt<=65536 );
  പ്രതിജ്ഞാ( (iAmt & (iAmt-1))==0 );
  പ്രതിജ്ഞാ( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(വലുപ്പം(z_താക്കോൽ), z_താക്കോൽ, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_താക്കോൽ, aData);
  എങ്കിൽ( 0==rc ){
    എങ്കിൽ( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  തിരികെ rc;
}

/*
** Truncate an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  പ്രതിജ്ഞാ( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  തിരികെ SQLITE_OK;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  എങ്കിൽ( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    അക്ഷരം z_താക്കോൽ[50];
    unsigned പൂർണ്ണസംഖ്യ pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    ആകുമ്പോൾ( pgno<=pgnoMax ){
      sqlite3_snprintf(വലുപ്പം(z_താക്കോൽ), z_താക്കോൽ, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_താക്കോൽ);
      pgno++;
    }
    pFile->szDb = size;
    തിരികെ kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  തിരികെ SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsSyncJrnl(sqlite3_file *pProtoFile, പൂർണ്ണസംഖ്യ flags){
  പൂർണ്ണസംഖ്യ i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  അക്ഷരം *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  എങ്കിൽ( pFile->nJrnl<=0 ){
    തിരികെ kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  എങ്കിൽ( zOut==0 ){
    തിരികെ SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  ചെയ്യുക{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }ആകുമ്പോൾ( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  തിരികെ i ? SQLITE_IOERR : SQLITE_OK;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsSyncDb(sqlite3_file *pProtoFile, പൂർണ്ണസംഖ്യ flags){
  തിരികെ SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  തിരികെ SQLITE_OK;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  എങ്കിൽ( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }അല്ലെങ്കിൽ{
    *pSize = kvvfsReadFileSize(pFile);
  }
  തിരികെ SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsLock(sqlite3_file *pProtoFile, പൂർണ്ണസംഖ്യ eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  പ്രതിജ്ഞാ( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  എങ്കിൽ( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  തിരികെ SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsUnlock(sqlite3_file *pProtoFile, പൂർണ്ണസംഖ്യ eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  പ്രതിജ്ഞാ( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  എങ്കിൽ( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  തിരികെ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsCheckReservedLock(sqlite3_file *pProtoFile, പൂർണ്ണസംഖ്യ *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  തിരികെ SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFileControlJrnl(sqlite3_file *pProtoFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  തിരികെ SQLITE_NOTFOUND;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFileControlDb(sqlite3_file *pProtoFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  എങ്കിൽ( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    എങ്കിൽ( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    തിരികെ rc;
  }
  തിരികെ SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsSectorSize(sqlite3_file *pFile){
  തിരികെ 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  തിരികെ 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  സ്ഥിരം അക്ഷരം *zName,
  sqlite3_file *pProtoFile,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  എങ്കിൽ( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  പ്രതിജ്ഞാ(!pFile->zClass);
  പ്രതിജ്ഞാ(!pFile->aData);
  പ്രതിജ്ഞാ(!pFile->aJrnl);
  പ്രതിജ്ഞാ(!pFile->nJrnl);
  പ്രതിജ്ഞാ(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  എങ്കിൽ( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    എങ്കിൽ( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }അല്ലെങ്കിൽ എങ്കിൽ( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }അല്ലെങ്കിൽ{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  എങ്കിൽ( !pFile->zClass ){
#ifdef SQLITE_WASM
    എങ്കിൽ( strlen(zName) >= (KVRECORD_താക്കോൽ_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      തിരികെ SQLITE_CANTOPEN;
    }
#അല്ലെങ്കിൽ
    എങ്കിൽ( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      തിരികെ SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  എങ്കിൽ( pFile->aData==0 ){
    തിരികെ SQLITE_NOMEM;
  }
  തിരികെ SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsDelete(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ dirSync){
  പൂർണ്ണസംഖ്യ rc /* The JS impl can fail with OOM in argument conversion */;
  എങ്കിൽ( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }അല്ലെങ്കിൽ
  എങ്കിൽ( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  അല്ലെങ്കിൽ{
    rc = 0;
  }
  തിരികെ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  സ്ഥിരം അക്ഷരം *zPath,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#എങ്കിൽ 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  സ്ഥിരം അക്ഷരം *z_താക്കോൽ = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_താക്കോൽ, 0, 0)>0;
#അല്ലെങ്കിൽ
  എങ്കിൽ( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }അല്ലെങ്കിൽ
  എങ്കിൽ( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }അല്ലെങ്കിൽ
  എങ്കിൽ( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }അല്ലെങ്കിൽ
  എങ്കിൽ( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }അല്ലെങ്കിൽ
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  തിരികെ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zPath,
  പൂർണ്ണസംഖ്യ nOut,
  അക്ഷരം *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  എങ്കിൽ( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  തിരികെ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
സ്ഥിതം ശൂന്യം *kvvfsDlOpen(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath){
  തിരികെ 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsRandomness(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zBufOut){
  memset(zBufOut, 0, nByte);
  തിരികെ nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsSleep(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nMicro){
  തിരികെ SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsCurrentTime(sqlite3_vfs *pVfs, ഇരട്ട *pTimeOut){
  sqlite3_int64 i = 0;
  പൂർണ്ണസംഖ്യ rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  തിരികെ rc;
}
#അന്തര്ഭാവ <sys/time.h>
സ്ഥിതം പൂർണ്ണസംഖ്യ kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  സ്ഥിതം സ്ഥിരം sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ഘടന timeval sNow;
  (ശൂന്യം)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  തിരികെ SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#എങ്കിൽ SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
പൂർണ്ണസംഖ്യ sqlite3_os_init(ശൂന്യം){
  തിരികെ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
പൂർണ്ണസംഖ്യ sqlite3_os_end(ശൂന്യം){
  തിരികെ SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#എങ്കിൽ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
പൂർണ്ണസംഖ്യ sqlite3KvvfsInit(ശൂന്യം){
  തിരികെ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
