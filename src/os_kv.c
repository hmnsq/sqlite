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
#تعریف_کرو SQLITE_KV_TRACE(X)  لکھو X
#ورنہ
#تعریف_کرو SQLITE_KV_TRACE(X)
#اگر_ختم

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#اگر 0
#تعریف_کرو SQLITE_KV_LOG(X)  لکھو X
#ورنہ
#تعریف_کرو SQLITE_KV_LOG(X)
#اگر_ختم

/*
** Forward declaration of objects used by this VFS implementation
*/
قسمتعریف ساخت KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
ساخت KVVfsFile {
  sqlite3_file بنیاد;              /* IO methods */
  ثابت حرف *zClass;             /* Storage class */
  صحیح isJournal;                  /* True if this is a journal file */
  غیرعلامتی صحیح nJrnl;             /* Space allocated for aJrnl[] */
  حرف *aJrnl;                    /* Journal content */
  صحیح sz_صفحہ;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  حرف *a_ڈیٹا;                    /* Buffer to hold page data */
};
#تعریف_کرو SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ساکن صحیح kvvfsClose(sqlite3_file*);
ساکن صحیح kvvfsReadDb(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح kvvfsReadJrnl(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح kvvfsWriteDb(sqlite3_file*,ثابت خالی*,صحیح iAmt, sqlite3_int64);
ساکن صحیح kvvfsWriteJrnl(sqlite3_file*,ثابت خالی*,صحیح iAmt, sqlite3_int64);
ساکن صحیح kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح kvvfsSyncDb(sqlite3_file*, صحیح flags);
ساکن صحیح kvvfsSyncJrnl(sqlite3_file*, صحیح flags);
ساکن صحیح kvvfs_File_سائز_Db(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح kvvfs_File_سائز_Jrnl(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح kvvfs_تالا(sqlite3_file*, صحیح);
ساکن صحیح kvvfsUnlock(sqlite3_file*, صحیح);
ساکن صحیح kvvfs_Check_Reserved_تالا(sqlite3_file*, صحیح *p_Res_برآمد);
ساکن صحیح kvvfsFileControlDb(sqlite3_file*, صحیح op, خالی *pArg);
ساکن صحیح kvvfsFileControlJrnl(sqlite3_file*, صحیح op, خالی *pArg);
ساکن صحیح kvvfs_Sector_سائز(sqlite3_file*);
ساکن صحیح kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ساکن صحیح kvvfsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحیح , صحیح *);
ساکن صحیح kvvfsDelete(sqlite3_vfs*, ثابت حرف *zName, صحیح syncDir);
ساکن صحیح kvvfsAccess(sqlite3_vfs*, ثابت حرف *zName, صحیح flags, صحیح *);
ساکن صحیح kvvfs_بھرا_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحیح, حرف *z_برآمد);
ساکن خالی *kvvfsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساکن صحیح kvvfsRandomness(sqlite3_vfs*, صحیح nByte, حرف *z_برآمد);
ساکن صحیح kvvfsSleep(sqlite3_vfs*, صحیح microseconds);
ساکن صحیح kvvfs_Current_وقت(sqlite3_vfs*, دوہرا*);
ساکن صحیح kvvfs_Current_وقت_Int64(sqlite3_vfs*, sqlite3_int64*);

ساکن sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  حجم(KVVfsFile),              /* szOsFile */
  1024,                           /* mxPathname */
  0,                              /* pNext */
  "kvvfs",                        /* zName */
  0,                              /* pAppData */
  kvvfsOpen,                      /* xOpen */
  kvvfsDelete,                    /* xDelete */
  kvvfsAccess,                    /* xAccess */
  kvvfs_بھرا_Pathname,              /* xFullPathname */
  kvvfsDlOpen,                    /* xDlOpen */
  0,                              /* xDlError */
  0,                              /* xDlSym */
  0,                              /* xDlClose */
  kvvfsRandomness,                /* xRandomness */
  kvvfsSleep,                     /* xSleep */
  kvvfs_Current_وقت,               /* xCurrentTime */
  0,                              /* xGetLastError */
  kvvfs_Current_وقت_Int64           /* xCurrentTimeInt64 */
};

/* Methods for sqlite3_file objects referencing a database file
*/
ساکن sqlite3_io_methods kvvfs_db_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadDb,                    /* xRead */
  kvvfsWriteDb,                   /* xWrite */
  kvvfsTruncateDb,                /* xTruncate */
  kvvfsSyncDb,                    /* xSync */
  kvvfs_File_سائز_Db,                /* xFileSize */
  kvvfs_تالا,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_تالا,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfs_Sector_سائز,                /* xSectorSize */
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
ساکن sqlite3_io_methods kvvfs_jrnl_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadJrnl,                  /* xRead */
  kvvfsWriteJrnl,                 /* xWrite */
  kvvfsTruncateJrnl,              /* xTruncate */
  kvvfsSyncJrnl,                  /* xSync */
  kvvfs_File_سائز_Jrnl,              /* xFileSize */
  kvvfs_تالا,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_تالا,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfs_Sector_سائز,                /* xSectorSize */
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
#اگر_غیر_تعریف_شدہ SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ساکن صحیح kvrecordWrite(ثابت حرف*, ثابت حرف *z_کلید, ثابت حرف *z_ڈیٹا);
ساکن صحیح kvrecordDelete(ثابت حرف*, ثابت حرف *z_کلید);
ساکن صحیح kvrecordRead(ثابت حرف*, ثابت حرف *z_کلید, حرف *zBuf, صحیح nBuf);
#اگر_ختم
#اگر_غیر_تعریف_شدہ KVRECORD_کلید_SZ
#تعریف_کرو KVRECORD_کلید_SZ 32
#اگر_ختم

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ساکن خالی kvrecord_Make_کلید(
  ثابت حرف *zClass,
  ثابت حرف *z_کلید_اندر,
  حرف *z_کلید_برآمد
){
  أكد( z_کلید_اندر );
  أكد( z_کلید_برآمد );
  أكد( zClass );
  sqlite3_snprintf(KVRECORD_کلید_SZ, z_کلید_برآمد, "kvvfs-%s-%s",
                   zClass, z_کلید_اندر);
}

#اگر_غیر_تعریف_شدہ SQLITE_WASM
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
ساکن صحیح kvrecordWrite(
  ثابت حرف *zClass,
  ثابت حرف *z_کلید,
  ثابت حرف *z_ڈیٹا
){
  FILE *fd;
  حرف zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
  fd = fopen(zXKey, "wb");
  اگر( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (صحیح)لڑی_لمبائی(z_ڈیٹا), z_ڈیٹا,
                 لڑی_لمبائی(z_ڈیٹا)>50 ? "..." : ""));
    fputs(z_ڈیٹا, fd);
    fclose(fd);
    واپس 0;
  }ورنہ{
    واپس 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
ساکن صحیح kvrecordDelete(ثابت حرف *zClass, ثابت حرف *z_کلید){
  حرف zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  واپس 0;
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
ساکن صحیح kvrecordRead(
  ثابت حرف *zClass,
  ثابت حرف *z_کلید,
  حرف *zBuf,
  صحیح nBuf
){
  FILE *fd;
  ساخت stat buf;
  حرف zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
  اگر( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_منوال)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    واپس -1;
  }
  اگر( nBuf<=0 ){
    واپس (صحیح)buf.st_سائز;
  }ورنہ اگر( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (صحیح)buf.st_سائز));
    واپس (صحیح)buf.st_سائز;
  }
  اگر( nBuf > buf.st_سائز + 1 ){
    nBuf = buf.st_سائز + 1;
  }
  fd = fopen(zXKey, "rb");
  اگر( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    واپس -1;
  }ورنہ{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    واپس (صحیح)n;
  }
}
#اگر_ختم /* #ifndef SQLITE_WASM */


/*
** An internal level of indirection which enables us to replace the
** kvvfs i/o methods with JavaScript implementations in WASM builds.
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
قسمتعریف ساخت sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
ساخت sqlite3_kvvfs_methods {
  صحیح (*xRcrdRead)(ثابت حرف*, ثابت حرف *z_کلید, حرف *zBuf, صحیح nBuf);
  صحیح (*xRcrdWrite)(ثابت حرف*, ثابت حرف *z_کلید, ثابت حرف *z_ڈیٹا);
  صحیح (*xRcrdDelete)(ثابت حرف*, ثابت حرف *z_کلید);
  ثابت صحیح n_کلید_سائز;
  ثابت صحیح n_بفر_سائز;
#اگر_غیر_تعریف_شدہ SQLITE_WASM
#  define MAYBE_CONST ثابت
#ورنہ
#  define MAYBE_CONST
#اگر_ختم
  MAYBE_CONST sqlite3_vfs * pVfs;
  MAYBE_CONST sqlite3_io_methods *pIoDb;
  MAYBE_CONST sqlite3_io_methods *pIoJrnl;
#تعریف_منسوخ MAYBE_CONST
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
#اگر_غیر_تعریف_شدہ SQLITE_WASM
ثابت
#اگر_ختم
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#اگر_غیر_تعریف_شدہ SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#ورنہ
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#اگر_ختم
  .n_کلید_سائز        = KVRECORD_کلید_SZ,
  .n_بفر_سائز     = SQLITE_KVOS_SZ,
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
#اگر_غیر_تعریف_شدہ SQLITE_WASM
ساکن
#اگر_ختم
صحیح kvvfsEncode(ثابت حرف *a_ڈیٹا, صحیح n_ڈیٹا, حرف *a_برآمد){
  صحیح i, j;
  ثابت غیرعلامتی حرف *a = (ثابت غیرعلامتی حرف*)a_ڈیٹا;
  برائے(i=j=0; i<n_ڈیٹا; i++){
    غیرعلامتی حرف c = a[i];
    اگر( c!=0 ){
      a_برآمد[j++] = "0123456789ABCDEF"[c>>4];
      a_برآمد[j++] = "0123456789ABCDEF"[c&0xf];
    }ورنہ{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      صحیح k;
      برائے(k=1; i+k<n_ڈیٹا && a[i+k]==0; k++){}
      i += k-1;
      جب( k>0 ){
        a_برآمد[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  a_برآمد[j] = 0;
  واپس j;
}

ساکن ثابت علامتی حرف kvvfs_Hex_قدر[256] = {
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
#اگر_غیر_تعریف_شدہ SQLITE_WASM
ساکن
#اگر_ختم
صحیح kvvfsDecode(ثابت حرف *a, حرف *a_برآمد, صحیح n_برآمد){
  صحیح i, j;
  صحیح c;
  ثابت غیرعلامتی حرف *a_اندر = (ثابت غیرعلامتی حرف*)a;
  i = 0;
  j = 0;
  جب( 1 ){
    c = kvvfs_Hex_قدر[a_اندر[i]];
    اگر( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = a_اندر[i];
      اگر( c==0 ) توڑو;
      جب( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        اگر( n>n_برآمد ) واپس -1 /* oversized/malformed input */;
        mult *= 26;
        c = a_اندر[++i];
      }
      اگر( j+n>n_برآمد ) واپس -1 /* oversized/malformed input */;
      memset(&a_برآمد[j], 0, n);
      j += n;
      اگر( c==0 || mult==1 ) توڑو; /* progress stalled if mult==1 */
    }ورنہ اگر( j<n_برآمد ){
      a_برآمد[j] = c<<4;
      c = kvvfs_Hex_قدر[a_اندر[++i]];
      اگر( c<0 ) واپس -1 /* hex bytes are always in pairs */;
      a_برآمد[j++] += c;
      i++;
    }ورنہ{
      واپس -1;
    }
  }
  واپس j;
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
ساکن خالی kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ثابت حرف *zTxt,      /* Text encoding.  Zero-terminated */
  صحیح nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  غیرعلامتی صحیح n = 0, mult;
  صحیح c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  جب( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  اگر( ' '!=zTxt[i++] ){
    /* Malformed input */
    واپس;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  اگر( pFile->aJrnl==0 ){
    واپس;
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
ساکن sqlite3_int64 kvvfs_Read_File_سائز(KVVfsFile *pFile){
  حرف z_ڈیٹا[50];
  z_ڈیٹا[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", z_ڈیٹا,
                                حجم(z_ڈیٹا)-1);
  واپس strtoll(z_ڈیٹا, 0, 0);
}
ساکن صحیح kvvfs_Write_File_سائز(KVVfsFile *pFile, sqlite3_int64 sz){
  حرف z_ڈیٹا[50];
  sqlite3_snprintf(حجم(z_ڈیٹا), z_ڈیٹا, "%lld", sz);
  واپس sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", z_ڈیٹا);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
ساکن صحیح kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->a_ڈیٹا);
  memset(pFile, 0, حجم(*pFile));
  واپس SQLITE_OK;
}

/*
** Read from the -journal file.
*/
ساکن صحیح kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  خالی *zBuf,
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
    حرف *aTxt;
    اگر( szTxt<=4 ){
      واپس SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    اگر( aTxt==0 ) واپس SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    اگر( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    اگر( rc ) واپس rc;
    اگر( pFile->aJrnl==0 ) واپس SQLITE_IOERR;
  }
  اگر( iOfst+iAmt>pFile->nJrnl ){
    واپس SQLITE_IOERR_SHORT_READ;
  }
  حافظہ_نقل(zBuf, pFile->aJrnl+iOfst, iAmt);
  واپس SQLITE_OK;
}

/*
** Read from the database file.
*/
ساکن صحیح kvvfsReadDb(
  sqlite3_file *pProtoFile,
  خالی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  غیرعلامتی صحیح pgno;
  صحیح got, n;
  حرف z_کلید[30];
  حرف *a_ڈیٹا = pFile->a_ڈیٹا;
  أكد( iOfst>=0 );
  أكد( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  اگر( iOfst+iAmt>=512 ){
    اگر( (iOfst % iAmt)!=0 ){
      واپس SQLITE_IOERR_READ;
    }
    اگر( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      واپس SQLITE_IOERR_READ;
    }
    pFile->sz_صفحہ = iAmt;
    pgno = 1 + iOfst/iAmt;
  }ورنہ{
    pgno = 1;
  }
  sqlite3_snprintf(حجم(z_کلید), z_کلید, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_کلید,
                                      a_ڈیٹا, SQLITE_KVOS_SZ-1);
  اگر( got<0 ){
    n = 0;
  }ورنہ{
    a_ڈیٹا[got] = 0;
    اگر( iOfst+iAmt<512 ){
      صحیح k = iOfst+iAmt;
      a_ڈیٹا[k*2] = 0;
      n = kvvfsDecode(a_ڈیٹا, &a_ڈیٹا[2000], SQLITE_KVOS_SZ-2000);
      اگر( n>=iOfst+iAmt ){
        حافظہ_نقل(zBuf, &a_ڈیٹا[2000+iOfst], iAmt);
        n = iAmt;
      }ورنہ{
        n = 0;
      }
    }ورنہ{
      n = kvvfsDecode(a_ڈیٹا, zBuf, iAmt);
    }
  }
  اگر( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    واپس SQLITE_IOERR_SHORT_READ;
  }
  واپس SQLITE_OK;
}


/*
** Write into the -journal file.
*/
ساکن صحیح kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ثابت خالی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 i_انتہا = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  اگر( i_انتہا>=0x10000000 ) واپس SQLITE_بھرا;
  اگر( pFile->aJrnl==0 || pFile->nJrnl<i_انتہا ){
    حرف *aNew = sqlite3_realloc(pFile->aJrnl, i_انتہا);
    اگر( aNew==0 ){
      واپس SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    اگر( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = i_انتہا;
  }
  حافظہ_نقل(pFile->aJrnl+iOfst, zBuf, iAmt);
  واپس SQLITE_OK;
}

/*
** Write into the database file.
*/
ساکن صحیح kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ثابت خالی *zBuf,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  غیرعلامتی صحیح pgno;
  حرف z_کلید[30];
  حرف *a_ڈیٹا = pFile->a_ڈیٹا;
  صحیح rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  أكد( iAmt>=512 && iAmt<=65536 );
  أكد( (iAmt & (iAmt-1))==0 );
  أكد( pFile->sz_صفحہ<0 || pFile->sz_صفحہ==iAmt );
  pFile->sz_صفحہ = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(حجم(z_کلید), z_کلید, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, a_ڈیٹا);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_کلید, a_ڈیٹا);
  اگر( 0==rc ){
    اگر( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  واپس rc;
}

/*
** Truncate an kvvfs-file.
*/
ساکن صحیح kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 سائز){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, سائز));
  أكد( سائز==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  واپس SQLITE_OK;
}
ساکن صحیح kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 سائز){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  اگر( pFile->szDb>سائز
   && pFile->sz_صفحہ>0
   && (سائز % pFile->sz_صفحہ)==0
  ){
    حرف z_کلید[50];
    غیرعلامتی صحیح pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, سائز));
    pgno = 1 + سائز/pFile->sz_صفحہ;
    pgnoMax = 2 + pFile->szDb/pFile->sz_صفحہ;
    جب( pgno<=pgnoMax ){
      sqlite3_snprintf(حجم(z_کلید), z_کلید, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_کلید);
      pgno++;
    }
    pFile->szDb = سائز;
    واپس kvvfs_Write_File_سائز(pFile, سائز) ? SQLITE_IOERR : SQLITE_OK;
  }
  واپس SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ساکن صحیح kvvfsSyncJrnl(sqlite3_file *pProtoFile, صحیح flags){
  صحیح i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  حرف *z_برآمد;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  اگر( pFile->nJrnl<=0 ){
    واپس kvvfsTruncateJrnl(pProtoFile, 0);
  }
  z_برآمد = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  اگر( z_برآمد==0 ){
    واپس SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  کرو{
    z_برآمد[i++] = 'a' + (n%26);
    n /= 26;
  }جب( n>0 );
  z_برآمد[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &z_برآمد[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", z_برآمد);
  sqlite3_free(z_برآمد);
  واپس i ? SQLITE_IOERR : SQLITE_OK;
}
ساکن صحیح kvvfsSyncDb(sqlite3_file *pProtoFile, صحیح flags){
  واپس SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ساکن صحیح kvvfs_File_سائز_Jrnl(sqlite3_file *pProtoFile, sqlite_int64 *p_سائز){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *p_سائز = pFile->nJrnl;
  واپس SQLITE_OK;
}
ساکن صحیح kvvfs_File_سائز_Db(sqlite3_file *pProtoFile, sqlite_int64 *p_سائز){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  اگر( pFile->szDb>=0 ){
    *p_سائز = pFile->szDb;
  }ورنہ{
    *p_سائز = kvvfs_Read_File_سائز(pFile);
  }
  واپس SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ساکن صحیح kvvfs_تالا(sqlite3_file *pProtoFile, صحیح e_تالا){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, e_تالا));

  اگر( e_تالا!=SQLITE_تالا_NONE ){
    pFile->szDb = kvvfs_Read_File_سائز(pFile);
  }
  واپس SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ساکن صحیح kvvfsUnlock(sqlite3_file *pProtoFile, صحیح e_تالا){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, e_تالا));
  اگر( e_تالا==SQLITE_تالا_NONE ){
    pFile->szDb = -1;
  }
  واپس SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ساکن صحیح kvvfs_Check_Reserved_تالا(sqlite3_file *pProtoFile, صحیح *p_Res_برآمد){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *p_Res_برآمد = 0;
  واپس SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
ساکن صحیح kvvfsFileControlJrnl(sqlite3_file *pProtoFile, صحیح op, خالی *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  واپس SQLITE_NOTFOUND;
}
ساکن صحیح kvvfsFileControlDb(sqlite3_file *pProtoFile, صحیح op, خالی *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  اگر( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    صحیح rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    اگر( pFile->szDb>0 && 0!=kvvfs_Write_File_سائز(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    واپس rc;
  }
  واپس SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ساکن صحیح kvvfs_Sector_سائز(sqlite3_file *pFile){
  واپس 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
ساکن صحیح kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  واپس 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
ساکن صحیح kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ثابت حرف *zName,
  sqlite3_file *pProtoFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  اگر( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  أكد(!pFile->zClass);
  أكد(!pFile->a_ڈیٹا);
  أكد(!pFile->aJrnl);
  أكد(!pFile->nJrnl);
  أكد(!pFile->بنیاد.pMethods);
  pFile->sz_صفحہ = -1;
  pFile->szDb = -1;
  اگر( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->بنیاد.pMethods = &kvvfs_jrnl_io_methods;
    اگر( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }ورنہ اگر( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }ورنہ{
    pFile->isJournal = 0;
    pFile->بنیاد.pMethods = &kvvfs_db_io_methods;
  }
  اگر( !pFile->zClass ){
#اگر_تعریف_شدہ SQLITE_WASM
    اگر( لڑی_لمبائی(zName) >= (KVRECORD_کلید_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      واپس SQLITE_CANTOPEN;
    }
#ورنہ
    اگر( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      واپس SQLITE_CANTOPEN;
    }
#اگر_ختم
    pFile->zClass = zName;
  }
  pFile->a_ڈیٹا = sqlite3_malloc64(SQLITE_KVOS_SZ);
  اگر( pFile->a_ڈیٹا==0 ){
    واپس SQLITE_NOMEM;
  }
  واپس SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساکن صحیح kvvfsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  صحیح rc /* The JS impl can fail with OOM in argument conversion */;
  اگر( strcmp(z_راستہ, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }ورنہ
  اگر( strcmp(z_راستہ, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  ورنہ{
    rc = 0;
  }
  واپس rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساکن صحیح kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ثابت حرف *z_راستہ,
  صحیح flags,
  صحیح *p_Res_برآمد
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", z_راستہ));
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
  ثابت حرف *z_کلید = (0==sqlite3_strglob("*-journal", z_راستہ))
    ? "jrnl" : "sz";
  *p_Res_برآمد =
    sqlite3KvvfsMethods.xRcrdRead(z_راستہ, z_کلید, 0, 0)>0;
#ورنہ
  اگر( strcmp(z_راستہ, "local-journal")==0 ){
    *p_Res_برآمد =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }ورنہ
  اگر( strcmp(z_راستہ, "session-journal")==0 ){
    *p_Res_برآمد =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }ورنہ
  اگر( strcmp(z_راستہ, "local")==0 ){
    *p_Res_برآمد =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }ورنہ
  اگر( strcmp(z_راستہ, "session")==0 ){
    *p_Res_برآمد =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }ورنہ
  {
    *p_Res_برآمد = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#اگر_ختم
  SQLITE_KV_LOG(("xAccess returns %d\n",*p_Res_برآمد));
  واپس SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساکن صحیح kvvfs_بھرا_Pathname(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_راستہ,
  صحیح n_برآمد,
  حرف *z_برآمد
){
  size_t n_راستہ;
#اگر_تعریف_شدہ SQLITE_OS_KV_ALWAYS_LOCAL
  z_راستہ = "local";
#اگر_ختم
  n_راستہ = لڑی_لمبائی(z_راستہ);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", z_راستہ));
  اگر( n_برآمد<n_راستہ+1 ) n_راستہ = n_برآمد - 1;
  حافظہ_نقل(z_برآمد, z_راستہ, n_راستہ);
  z_برآمد[n_راستہ] = 0;
  واپس SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساکن خالی *kvvfsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  واپس 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ساکن صحیح kvvfsRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  memset(z_Buf_برآمد, 0, nByte);
  واپس nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
ساکن صحیح kvvfsSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  واپس SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساکن صحیح kvvfs_Current_وقت(sqlite3_vfs *pVfs, دوہرا *p_وقت_برآمد){
  sqlite3_int64 i = 0;
  صحیح rc;
  rc = kvvfs_Current_وقت_Int64(0, &i);
  *p_وقت_برآمد = i/86400000.0;
  واپس rc;
}
#شامل <sys/time.h>
ساکن صحیح kvvfs_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_وقت_برآمد){
  ساکن ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ساخت timeval sNow;
  (خالی)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *p_وقت_برآمد = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#اگر SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
صحیح sqlite3_os_init(خالی){
  واپس sqlite3_vfs_رجسٹر(&sqlite3OsKvvfsObject, 1);
}
صحیح sqlite3_os_انتہا(خالی){
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_OS_KV */

#اگر SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
صحیح sqlite3KvvfsInit(خالی){
  واپس sqlite3_vfs_رجسٹر(&sqlite3OsKvvfsObject, 0);
}
#اگر_ختم
