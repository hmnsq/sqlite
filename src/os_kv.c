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
#ضمن <sqliteInt.h>
#إذا SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#إذا 0
#وصف_ڪريو SQLITE_KV_TRACE(X)  لکو X
#وإلا
#وصف_ڪريو SQLITE_KV_TRACE(X)
#جيڪڏهن_پورو

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#إذا 0
#وصف_ڪريو SQLITE_KV_LOG(X)  لکو X
#وإلا
#وصف_ڪريو SQLITE_KV_LOG(X)
#جيڪڏهن_پورو

/*
** Forward declaration of objects used by this VFS implementation
*/
تعريفنوع بنية KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
بنية KVVfsFile {
  sqlite3_file بنياد;              /* IO methods */
  ثابت حرف *zClass;             /* Storage class */
  صحيح isJournal;                  /* True if this is a journal file */
  بنانشان صحيح nJrnl;             /* Space allocated for aJrnl[] */
  حرف *aJrnl;                    /* Journal content */
  صحيح sz_صفحو;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  حرف *a_معلومات;                    /* Buffer to hold page data */
};
#وصف_ڪريو SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ساكن صحيح kvvfsClose(sqlite3_file*);
ساكن صحيح kvvfsReadDb(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح kvvfsReadJrnl(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح kvvfsWriteDb(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64);
ساكن صحيح kvvfsWriteJrnl(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64);
ساكن صحيح kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 سائيز);
ساكن صحيح kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 سائيز);
ساكن صحيح kvvfsSyncDb(sqlite3_file*, صحيح flags);
ساكن صحيح kvvfsSyncJrnl(sqlite3_file*, صحيح flags);
ساكن صحيح kvvfs_File_سائيز_Db(sqlite3_file*, sqlite3_int64 *p_سائيز);
ساكن صحيح kvvfs_File_سائيز_Jrnl(sqlite3_file*, sqlite3_int64 *p_سائيز);
ساكن صحيح kvvfs_تالو(sqlite3_file*, صحيح);
ساكن صحيح kvvfsUnlock(sqlite3_file*, صحيح);
ساكن صحيح kvvfs_Check_Reserved_تالو(sqlite3_file*, صحيح *p_Res_ٻاهريون);
ساكن صحيح kvvfsFileControlDb(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح kvvfsFileControlJrnl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح kvvfs_Sector_سائيز(sqlite3_file*);
ساكن صحيح kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ساكن صحيح kvvfsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح kvvfsDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح kvvfsAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح kvvfs_ڀريل_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *z_ٻاهريون);
ساكن فراغ *kvvfsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن صحيح kvvfsRandomness(sqlite3_vfs*, صحيح nByte, حرف *z_ٻاهريون);
ساكن صحيح kvvfsSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح kvvfs_Current_وقت(sqlite3_vfs*, مزدوج*);
ساكن صحيح kvvfs_Current_وقت_Int64(sqlite3_vfs*, sqlite3_int64*);

ساكن sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  حجم(KVVfsFile),              /* szOsFile */
  1024,                           /* mxPathname */
  0,                              /* pNext */
  "kvvfs",                        /* zName */
  0,                              /* pAppData */
  kvvfsOpen,                      /* xOpen */
  kvvfsDelete,                    /* xDelete */
  kvvfsAccess,                    /* xAccess */
  kvvfs_ڀريل_Pathname,              /* xFullPathname */
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
ساكن sqlite3_io_methods kvvfs_db_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadDb,                    /* xRead */
  kvvfsWriteDb,                   /* xWrite */
  kvvfsTruncateDb,                /* xTruncate */
  kvvfsSyncDb,                    /* xSync */
  kvvfs_File_سائيز_Db,                /* xFileSize */
  kvvfs_تالو,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_تالو,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfs_Sector_سائيز,                /* xSectorSize */
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
ساكن sqlite3_io_methods kvvfs_jrnl_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadJrnl,                  /* xRead */
  kvvfsWriteJrnl,                 /* xWrite */
  kvvfsTruncateJrnl,              /* xTruncate */
  kvvfsSyncJrnl,                  /* xSync */
  kvvfs_File_سائيز_Jrnl,              /* xFileSize */
  kvvfs_تالو,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_تالو,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfs_Sector_سائيز,                /* xSectorSize */
  kvvfsDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                              /* xShmMap */
  0,                              /* xShmLock */
  0,                              /* xShmBarrier */
  0,                              /* xShmUnmap */
  0,                              /* xFetch */
  0                               /* xUnfetch */
};

/****** Storage subsystem **************************************************/
#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ساكن صحيح kvrecordWrite(ثابت حرف*, ثابت حرف *z_ڪنجي, ثابت حرف *z_معلومات);
ساكن صحيح kvrecordDelete(ثابت حرف*, ثابت حرف *z_ڪنجي);
ساكن صحيح kvrecordRead(ثابت حرف*, ثابت حرف *z_ڪنجي, حرف *zBuf, صحيح nBuf);
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل KVRECORD_ڪنجي_SZ
#وصف_ڪريو KVRECORD_ڪنجي_SZ 32
#جيڪڏهن_پورو

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ساكن فراغ kvrecord_Make_ڪنجي(
  ثابت حرف *zClass,
  ثابت حرف *z_ڪنجي_اندريون,
  حرف *z_ڪنجي_ٻاهريون
){
  أكد( z_ڪنجي_اندريون );
  أكد( z_ڪنجي_ٻاهريون );
  أكد( zClass );
  sqlite3_snprintf(KVRECORD_ڪنجي_SZ, z_ڪنجي_ٻاهريون, "kvvfs-%s-%s",
                   zClass, z_ڪنجي_اندريون);
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_WASM
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
ساكن صحيح kvrecordWrite(
  ثابت حرف *zClass,
  ثابت حرف *z_ڪنجي,
  ثابت حرف *z_معلومات
){
  FILE *fd;
  حرف zXKey[KVRECORD_ڪنجي_SZ];
  kvrecord_Make_ڪنجي(zClass, z_ڪنجي, zXKey);
  fd = fopen(zXKey, "wb");
  إذا( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (صحيح)لڙي_ڊيگهه(z_معلومات), z_معلومات,
                 لڙي_ڊيگهه(z_معلومات)>50 ? "..." : ""));
    fputs(z_معلومات, fd);
    fclose(fd);
    أرجع 0;
  }وإلا{
    أرجع 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
ساكن صحيح kvrecordDelete(ثابت حرف *zClass, ثابت حرف *z_ڪنجي){
  حرف zXKey[KVRECORD_ڪنجي_SZ];
  kvrecord_Make_ڪنجي(zClass, z_ڪنجي, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  أرجع 0;
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
ساكن صحيح kvrecordRead(
  ثابت حرف *zClass,
  ثابت حرف *z_ڪنجي,
  حرف *zBuf,
  صحيح nBuf
){
  FILE *fd;
  بنية stat buf;
  حرف zXKey[KVRECORD_ڪنجي_SZ];
  kvrecord_Make_ڪنجي(zClass, z_ڪنجي, zXKey);
  إذا( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_گهڻائي)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    أرجع -1;
  }
  إذا( nBuf<=0 ){
    أرجع (صحيح)buf.st_سائيز;
  }وإلا إذا( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (صحيح)buf.st_سائيز));
    أرجع (صحيح)buf.st_سائيز;
  }
  إذا( nBuf > buf.st_سائيز + 1 ){
    nBuf = buf.st_سائيز + 1;
  }
  fd = fopen(zXKey, "rb");
  إذا( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    أرجع -1;
  }وإلا{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    أرجع (صحيح)n;
  }
}
#جيڪڏهن_پورو /* #ifndef SQLITE_WASM */


/*
** An internal level of indirection which enables us to replace the
** kvvfs i/o methods with JavaScript implementations in WASM builds.
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
تعريفنوع بنية sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
بنية sqlite3_kvvfs_methods {
  صحيح (*xRcrdRead)(ثابت حرف*, ثابت حرف *z_ڪنجي, حرف *zBuf, صحيح nBuf);
  صحيح (*xRcrdWrite)(ثابت حرف*, ثابت حرف *z_ڪنجي, ثابت حرف *z_معلومات);
  صحيح (*xRcrdDelete)(ثابت حرف*, ثابت حرف *z_ڪنجي);
  ثابت صحيح n_ڪنجي_سائيز;
  ثابت صحيح n_بفر_سائيز;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WASM
#  define MAYBE_CONST ثابت
#وإلا
#  define MAYBE_CONST
#جيڪڏهن_پورو
  MAYBE_CONST sqlite3_vfs * pVfs;
  MAYBE_CONST sqlite3_io_methods *pIoDb;
  MAYBE_CONST sqlite3_io_methods *pIoJrnl;
#وصف_رد MAYBE_CONST
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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WASM
ثابت
#جيڪڏهن_پورو
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#وإلا
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#جيڪڏهن_پورو
  .n_ڪنجي_سائيز        = KVRECORD_ڪنجي_SZ,
  .n_بفر_سائيز     = SQLITE_KVOS_SZ,
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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WASM
ساكن
#جيڪڏهن_پورو
صحيح kvvfsEncode(ثابت حرف *a_معلومات, صحيح n_معلومات, حرف *a_ٻاهريون){
  صحيح i, j;
  ثابت بنانشان حرف *a = (ثابت بنانشان حرف*)a_معلومات;
  لكل(i=j=0; i<n_معلومات; i++){
    بنانشان حرف c = a[i];
    إذا( c!=0 ){
      a_ٻاهريون[j++] = "0123456789ABCDEF"[c>>4];
      a_ٻاهريون[j++] = "0123456789ABCDEF"[c&0xf];
    }وإلا{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      صحيح k;
      لكل(k=1; i+k<n_معلومات && a[i+k]==0; k++){}
      i += k-1;
      طالما( k>0 ){
        a_ٻاهريون[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  a_ٻاهريون[j] = 0;
  أرجع j;
}

ساكن ثابت نشانوارو حرف kvvfs_Hex_قدر[256] = {
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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WASM
ساكن
#جيڪڏهن_پورو
صحيح kvvfsDecode(ثابت حرف *a, حرف *a_ٻاهريون, صحيح n_ٻاهريون){
  صحيح i, j;
  صحيح c;
  ثابت بنانشان حرف *a_اندريون = (ثابت بنانشان حرف*)a;
  i = 0;
  j = 0;
  طالما( 1 ){
    c = kvvfs_Hex_قدر[a_اندريون[i]];
    إذا( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = a_اندريون[i];
      إذا( c==0 ) اكسر;
      طالما( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        إذا( n>n_ٻاهريون ) أرجع -1 /* oversized/malformed input */;
        mult *= 26;
        c = a_اندريون[++i];
      }
      إذا( j+n>n_ٻاهريون ) أرجع -1 /* oversized/malformed input */;
      memset(&a_ٻاهريون[j], 0, n);
      j += n;
      إذا( c==0 || mult==1 ) اكسر; /* progress stalled if mult==1 */
    }وإلا إذا( j<n_ٻاهريون ){
      a_ٻاهريون[j] = c<<4;
      c = kvvfs_Hex_قدر[a_اندريون[++i]];
      إذا( c<0 ) أرجع -1 /* hex bytes are always in pairs */;
      a_ٻاهريون[j++] += c;
      i++;
    }وإلا{
      أرجع -1;
    }
  }
  أرجع j;
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
ساكن فراغ kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ثابت حرف *zTxt,      /* Text encoding.  Zero-terminated */
  صحيح nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  بنانشان صحيح n = 0, mult;
  صحيح c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  طالما( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  إذا( ' '!=zTxt[i++] ){
    /* Malformed input */
    أرجع;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  إذا( pFile->aJrnl==0 ){
    أرجع;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  إذا( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
ساكن sqlite3_int64 kvvfs_Read_File_سائيز(KVVfsFile *pFile){
  حرف z_معلومات[50];
  z_معلومات[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", z_معلومات,
                                حجم(z_معلومات)-1);
  أرجع strtoll(z_معلومات, 0, 0);
}
ساكن صحيح kvvfs_Write_File_سائيز(KVVfsFile *pFile, sqlite3_int64 sz){
  حرف z_معلومات[50];
  sqlite3_snprintf(حجم(z_معلومات), z_معلومات, "%lld", sz);
  أرجع sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", z_معلومات);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
ساكن صحيح kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->a_معلومات);
  memset(pFile, 0, حجم(*pFile));
  أرجع SQLITE_OK;
}

/*
** Read from the -journal file.
*/
ساكن صحيح kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  أكد( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  إذا( pFile->aJrnl==0 ){
    صحيح rc;
    صحيح szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    حرف *aTxt;
    إذا( szTxt<=4 ){
      أرجع SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    إذا( aTxt==0 ) أرجع SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    إذا( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    إذا( rc ) أرجع rc;
    إذا( pFile->aJrnl==0 ) أرجع SQLITE_IOERR;
  }
  إذا( iOfst+iAmt>pFile->nJrnl ){
    أرجع SQLITE_IOERR_SHORT_READ;
  }
  يادگيري_نقل(zBuf, pFile->aJrnl+iOfst, iAmt);
  أرجع SQLITE_OK;
}

/*
** Read from the database file.
*/
ساكن صحيح kvvfsReadDb(
  sqlite3_file *pProtoFile,
  فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  بنانشان صحيح pgno;
  صحيح got, n;
  حرف z_ڪنجي[30];
  حرف *a_معلومات = pFile->a_معلومات;
  أكد( iOfst>=0 );
  أكد( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  إذا( iOfst+iAmt>=512 ){
    إذا( (iOfst % iAmt)!=0 ){
      أرجع SQLITE_IOERR_READ;
    }
    إذا( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      أرجع SQLITE_IOERR_READ;
    }
    pFile->sz_صفحو = iAmt;
    pgno = 1 + iOfst/iAmt;
  }وإلا{
    pgno = 1;
  }
  sqlite3_snprintf(حجم(z_ڪنجي), z_ڪنجي, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_ڪنجي,
                                      a_معلومات, SQLITE_KVOS_SZ-1);
  إذا( got<0 ){
    n = 0;
  }وإلا{
    a_معلومات[got] = 0;
    إذا( iOfst+iAmt<512 ){
      صحيح k = iOfst+iAmt;
      a_معلومات[k*2] = 0;
      n = kvvfsDecode(a_معلومات, &a_معلومات[2000], SQLITE_KVOS_SZ-2000);
      إذا( n>=iOfst+iAmt ){
        يادگيري_نقل(zBuf, &a_معلومات[2000+iOfst], iAmt);
        n = iAmt;
      }وإلا{
        n = 0;
      }
    }وإلا{
      n = kvvfsDecode(a_معلومات, zBuf, iAmt);
    }
  }
  إذا( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    أرجع SQLITE_IOERR_SHORT_READ;
  }
  أرجع SQLITE_OK;
}


/*
** Write into the -journal file.
*/
ساكن صحيح kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ثابت فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 i_پڇاڙي = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  إذا( i_پڇاڙي>=0x10000000 ) أرجع SQLITE_ڀريل;
  إذا( pFile->aJrnl==0 || pFile->nJrnl<i_پڇاڙي ){
    حرف *aNew = sqlite3_realloc(pFile->aJrnl, i_پڇاڙي);
    إذا( aNew==0 ){
      أرجع SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    إذا( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = i_پڇاڙي;
  }
  يادگيري_نقل(pFile->aJrnl+iOfst, zBuf, iAmt);
  أرجع SQLITE_OK;
}

/*
** Write into the database file.
*/
ساكن صحيح kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ثابت فراغ *zBuf,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  بنانشان صحيح pgno;
  حرف z_ڪنجي[30];
  حرف *a_معلومات = pFile->a_معلومات;
  صحيح rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  أكد( iAmt>=512 && iAmt<=65536 );
  أكد( (iAmt & (iAmt-1))==0 );
  أكد( pFile->sz_صفحو<0 || pFile->sz_صفحو==iAmt );
  pFile->sz_صفحو = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(حجم(z_ڪنجي), z_ڪنجي, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, a_معلومات);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_ڪنجي, a_معلومات);
  إذا( 0==rc ){
    إذا( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  أرجع rc;
}

/*
** Truncate an kvvfs-file.
*/
ساكن صحيح kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 سائيز){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, سائيز));
  أكد( سائيز==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  أرجع SQLITE_OK;
}
ساكن صحيح kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 سائيز){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  إذا( pFile->szDb>سائيز
   && pFile->sz_صفحو>0
   && (سائيز % pFile->sz_صفحو)==0
  ){
    حرف z_ڪنجي[50];
    بنانشان صحيح pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, سائيز));
    pgno = 1 + سائيز/pFile->sz_صفحو;
    pgnoMax = 2 + pFile->szDb/pFile->sz_صفحو;
    طالما( pgno<=pgnoMax ){
      sqlite3_snprintf(حجم(z_ڪنجي), z_ڪنجي, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_ڪنجي);
      pgno++;
    }
    pFile->szDb = سائيز;
    أرجع kvvfs_Write_File_سائيز(pFile, سائيز) ? SQLITE_IOERR : SQLITE_OK;
  }
  أرجع SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ساكن صحيح kvvfsSyncJrnl(sqlite3_file *pProtoFile, صحيح flags){
  صحيح i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  حرف *z_ٻاهريون;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  إذا( pFile->nJrnl<=0 ){
    أرجع kvvfsTruncateJrnl(pProtoFile, 0);
  }
  z_ٻاهريون = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  إذا( z_ٻاهريون==0 ){
    أرجع SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  افعل{
    z_ٻاهريون[i++] = 'a' + (n%26);
    n /= 26;
  }طالما( n>0 );
  z_ٻاهريون[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &z_ٻاهريون[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", z_ٻاهريون);
  sqlite3_free(z_ٻاهريون);
  أرجع i ? SQLITE_IOERR : SQLITE_OK;
}
ساكن صحيح kvvfsSyncDb(sqlite3_file *pProtoFile, صحيح flags){
  أرجع SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ساكن صحيح kvvfs_File_سائيز_Jrnl(sqlite3_file *pProtoFile, sqlite_int64 *p_سائيز){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *p_سائيز = pFile->nJrnl;
  أرجع SQLITE_OK;
}
ساكن صحيح kvvfs_File_سائيز_Db(sqlite3_file *pProtoFile, sqlite_int64 *p_سائيز){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  إذا( pFile->szDb>=0 ){
    *p_سائيز = pFile->szDb;
  }وإلا{
    *p_سائيز = kvvfs_Read_File_سائيز(pFile);
  }
  أرجع SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ساكن صحيح kvvfs_تالو(sqlite3_file *pProtoFile, صحيح e_تالو){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, e_تالو));

  إذا( e_تالو!=SQLITE_تالو_NONE ){
    pFile->szDb = kvvfs_Read_File_سائيز(pFile);
  }
  أرجع SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ساكن صحيح kvvfsUnlock(sqlite3_file *pProtoFile, صحيح e_تالو){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, e_تالو));
  إذا( e_تالو==SQLITE_تالو_NONE ){
    pFile->szDb = -1;
  }
  أرجع SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ساكن صحيح kvvfs_Check_Reserved_تالو(sqlite3_file *pProtoFile, صحيح *p_Res_ٻاهريون){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *p_Res_ٻاهريون = 0;
  أرجع SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
ساكن صحيح kvvfsFileControlJrnl(sqlite3_file *pProtoFile, صحيح op, فراغ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  أرجع SQLITE_NOTFOUND;
}
ساكن صحيح kvvfsFileControlDb(sqlite3_file *pProtoFile, صحيح op, فراغ *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  إذا( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    صحيح rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    إذا( pFile->szDb>0 && 0!=kvvfs_Write_File_سائيز(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    أرجع rc;
  }
  أرجع SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ساكن صحيح kvvfs_Sector_سائيز(sqlite3_file *pFile){
  أرجع 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
ساكن صحيح kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  أرجع 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
ساكن صحيح kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ثابت حرف *zName,
  sqlite3_file *pProtoFile,
  صحيح flags,
  صحيح *p_ٻاهريون_Flags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  إذا( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  أكد(!pFile->zClass);
  أكد(!pFile->a_معلومات);
  أكد(!pFile->aJrnl);
  أكد(!pFile->nJrnl);
  أكد(!pFile->بنياد.pMethods);
  pFile->sz_صفحو = -1;
  pFile->szDb = -1;
  إذا( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->بنياد.pMethods = &kvvfs_jrnl_io_methods;
    إذا( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }وإلا إذا( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }وإلا{
    pFile->isJournal = 0;
    pFile->بنياد.pMethods = &kvvfs_db_io_methods;
  }
  إذا( !pFile->zClass ){
#جيڪڏهن_وصف_ٿيل SQLITE_WASM
    إذا( لڙي_ڊيگهه(zName) >= (KVRECORD_ڪنجي_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      أرجع SQLITE_CANTOPEN;
    }
#وإلا
    إذا( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      أرجع SQLITE_CANTOPEN;
    }
#جيڪڏهن_پورو
    pFile->zClass = zName;
  }
  pFile->a_معلومات = sqlite3_malloc64(SQLITE_KVOS_SZ);
  إذا( pFile->a_معلومات==0 ){
    أرجع SQLITE_NOMEM;
  }
  أرجع SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح kvvfsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_رستو, صحيح dirSync){
  صحيح rc /* The JS impl can fail with OOM in argument conversion */;
  إذا( strcmp(z_رستو, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }وإلا
  إذا( strcmp(z_رستو, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  وإلا{
    rc = 0;
  }
  أرجع rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ثابت حرف *z_رستو,
  صحيح flags,
  صحيح *p_Res_ٻاهريون
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", z_رستو));
#إذا 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ثابت حرف *z_ڪنجي = (0==sqlite3_strglob("*-journal", z_رستو))
    ? "jrnl" : "sz";
  *p_Res_ٻاهريون =
    sqlite3KvvfsMethods.xRcrdRead(z_رستو, z_ڪنجي, 0, 0)>0;
#وإلا
  إذا( strcmp(z_رستو, "local-journal")==0 ){
    *p_Res_ٻاهريون =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }وإلا
  إذا( strcmp(z_رستو, "session-journal")==0 ){
    *p_Res_ٻاهريون =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }وإلا
  إذا( strcmp(z_رستو, "local")==0 ){
    *p_Res_ٻاهريون =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }وإلا
  إذا( strcmp(z_رستو, "session")==0 ){
    *p_Res_ٻاهريون =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }وإلا
  {
    *p_Res_ٻاهريون = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#جيڪڏهن_پورو
  SQLITE_KV_LOG(("xAccess returns %d\n",*p_Res_ٻاهريون));
  أرجع SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح kvvfs_ڀريل_Pathname(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_رستو,
  صحيح n_ٻاهريون,
  حرف *z_ٻاهريون
){
  size_t n_رستو;
#جيڪڏهن_وصف_ٿيل SQLITE_OS_KV_ALWAYS_LOCAL
  z_رستو = "local";
#جيڪڏهن_پورو
  n_رستو = لڙي_ڊيگهه(z_رستو);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", z_رستو));
  إذا( n_ٻاهريون<n_رستو+1 ) n_رستو = n_ٻاهريون - 1;
  يادگيري_نقل(z_ٻاهريون, z_رستو, n_رستو);
  z_ٻاهريون[n_رستو] = 0;
  أرجع SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *kvvfsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_رستو){
  أرجع 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ساكن صحيح kvvfsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_ٻاهريون){
  memset(z_Buf_ٻاهريون, 0, nByte);
  أرجع nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
ساكن صحيح kvvfsSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  أرجع SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح kvvfs_Current_وقت(sqlite3_vfs *pVfs, مزدوج *p_وقت_ٻاهريون){
  sqlite3_int64 i = 0;
  صحيح rc;
  rc = kvvfs_Current_وقت_Int64(0, &i);
  *p_وقت_ٻاهريون = i/86400000.0;
  أرجع rc;
}
#ضمن <sys/time.h>
ساكن صحيح kvvfs_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_وقت_ٻاهريون){
  ساكن ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  بنية timeval sNow;
  (فراغ)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *p_وقت_ٻاهريون = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#إذا SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
صحيح sqlite3_os_init(فراغ){
  أرجع sqlite3_vfs_رجسٽر(&sqlite3OsKvvfsObject, 1);
}
صحيح sqlite3_os_پڇاڙي(فراغ){
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو /* SQLITE_OS_KV */

#إذا SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
صحيح sqlite3KvvfsInit(فراغ){
  أرجع sqlite3_vfs_رجسٽر(&sqlite3OsKvvfsObject, 0);
}
#جيڪڏهن_پورو
