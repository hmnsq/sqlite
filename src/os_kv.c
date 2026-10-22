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
#تعریف_کرو SQLITE_KV_TRACE(X)  لکھو X
#وإلا
#تعریف_کرو SQLITE_KV_TRACE(X)
#جے_انت

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#إذا 0
#تعریف_کرو SQLITE_KV_LOG(X)  لکھو X
#وإلا
#تعریف_کرو SQLITE_KV_LOG(X)
#جے_انت

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
  sqlite3_file ادھار;              /* IO methods */
  ثابت حرف *zClass;             /* Storage class */
  صحيح isJournal;                  /* True if this is a journal file */
  بنانشانی صحيح nJrnl;             /* Space allocated for aJrnl[] */
  حرف *aJrnl;                    /* Journal content */
  صحيح sz_ورقہ;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  حرف *a_انکڑے;                    /* Buffer to hold page data */
};
#تعریف_کرو SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
ساكن صحيح kvvfsClose(sqlite3_file*);
ساكن صحيح kvvfsReadDb(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح kvvfsReadJrnl(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح kvvfsWriteDb(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64);
ساكن صحيح kvvfsWriteJrnl(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64);
ساكن صحيح kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 ناپ);
ساكن صحيح kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 ناپ);
ساكن صحيح kvvfsSyncDb(sqlite3_file*, صحيح flags);
ساكن صحيح kvvfsSyncJrnl(sqlite3_file*, صحيح flags);
ساكن صحيح kvvfs_File_ناپ_Db(sqlite3_file*, sqlite3_int64 *p_ناپ);
ساكن صحيح kvvfs_File_ناپ_Jrnl(sqlite3_file*, sqlite3_int64 *p_ناپ);
ساكن صحيح kvvfs_جندرا(sqlite3_file*, صحيح);
ساكن صحيح kvvfsUnlock(sqlite3_file*, صحيح);
ساكن صحيح kvvfs_Check_Reserved_جندرا(sqlite3_file*, صحيح *p_Res_باہرلا);
ساكن صحيح kvvfsFileControlDb(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح kvvfsFileControlJrnl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح kvvfs_Sector_ناپ(sqlite3_file*);
ساكن صحيح kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
ساكن صحيح kvvfsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح kvvfsDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح kvvfsAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح kvvfs_بھریا_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *z_باہرلا);
ساكن فراغ *kvvfsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن صحيح kvvfsRandomness(sqlite3_vfs*, صحيح nByte, حرف *z_باہرلا);
ساكن صحيح kvvfsSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح kvvfs_Current_ویلا(sqlite3_vfs*, مزدوج*);
ساكن صحيح kvvfs_Current_ویلا_Int64(sqlite3_vfs*, sqlite3_int64*);

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
  kvvfs_بھریا_Pathname,              /* xFullPathname */
  kvvfsDlOpen,                    /* xDlOpen */
  0,                              /* xDlError */
  0,                              /* xDlSym */
  0,                              /* xDlClose */
  kvvfsRandomness,                /* xRandomness */
  kvvfsSleep,                     /* xSleep */
  kvvfs_Current_ویلا,               /* xCurrentTime */
  0,                              /* xGetLastError */
  kvvfs_Current_ویلا_Int64           /* xCurrentTimeInt64 */
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
  kvvfs_File_ناپ_Db,                /* xFileSize */
  kvvfs_جندرا,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_جندرا,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfs_Sector_ناپ,                /* xSectorSize */
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
  kvvfs_File_ناپ_Jrnl,              /* xFileSize */
  kvvfs_جندرا,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfs_Check_Reserved_جندرا,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfs_Sector_ناپ,                /* xSectorSize */
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
#جے_بنا_تعریف SQLITE_WASM
/* In WASM builds these are implemented in JS. */
ساكن صحيح kvrecordWrite(ثابت حرف*, ثابت حرف *z_کلید, ثابت حرف *z_انکڑے);
ساكن صحيح kvrecordDelete(ثابت حرف*, ثابت حرف *z_کلید);
ساكن صحيح kvrecordRead(ثابت حرف*, ثابت حرف *z_کلید, حرف *zBuf, صحيح nBuf);
#جے_انت
#جے_بنا_تعریف KVRECORD_کلید_SZ
#تعریف_کرو KVRECORD_کلید_SZ 32
#جے_انت

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
ساكن فراغ kvrecord_Make_کلید(
  ثابت حرف *zClass,
  ثابت حرف *z_کلید_وچ,
  حرف *z_کلید_باہرلا
){
  أكد( z_کلید_وچ );
  أكد( z_کلید_باہرلا );
  أكد( zClass );
  sqlite3_snprintf(KVRECORD_کلید_SZ, z_کلید_باہرلا, "kvvfs-%s-%s",
                   zClass, z_کلید_وچ);
}

#جے_بنا_تعریف SQLITE_WASM
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
  ثابت حرف *z_کلید,
  ثابت حرف *z_انکڑے
){
  FILE *fd;
  حرف zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
  fd = fopen(zXKey, "wb");
  إذا( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (صحيح)لڑی_لمبائی(z_انکڑے), z_انکڑے,
                 لڑی_لمبائی(z_انکڑے)>50 ? "..." : ""));
    fputs(z_انکڑے, fd);
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
ساكن صحيح kvrecordDelete(ثابت حرف *zClass, ثابت حرف *z_کلید){
  حرف zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
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
  ثابت حرف *z_کلید,
  حرف *zBuf,
  صحيح nBuf
){
  FILE *fd;
  بنية stat buf;
  حرف zXKey[KVRECORD_کلید_SZ];
  kvrecord_Make_کلید(zClass, z_کلید, zXKey);
  إذا( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_منوال)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    أرجع -1;
  }
  إذا( nBuf<=0 ){
    أرجع (صحيح)buf.st_ناپ;
  }وإلا إذا( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (صحيح)buf.st_ناپ));
    أرجع (صحيح)buf.st_ناپ;
  }
  إذا( nBuf > buf.st_ناپ + 1 ){
    nBuf = buf.st_ناپ + 1;
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
#جے_انت /* #ifndef SQLITE_WASM */


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
  صحيح (*xRcrdRead)(ثابت حرف*, ثابت حرف *z_کلید, حرف *zBuf, صحيح nBuf);
  صحيح (*xRcrdWrite)(ثابت حرف*, ثابت حرف *z_کلید, ثابت حرف *z_انکڑے);
  صحيح (*xRcrdDelete)(ثابت حرف*, ثابت حرف *z_کلید);
  ثابت صحيح n_کلید_ناپ;
  ثابت صحيح n_بفر_ناپ;
#جے_بنا_تعریف SQLITE_WASM
#  define MAYBE_CONST ثابت
#وإلا
#  define MAYBE_CONST
#جے_انت
  MAYBE_CONST sqlite3_vfs * pVfs;
  MAYBE_CONST sqlite3_io_methods *pIoDb;
  MAYBE_CONST sqlite3_io_methods *pIoJrnl;
#تعریف_رد MAYBE_CONST
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
#جے_بنا_تعریف SQLITE_WASM
ثابت
#جے_انت
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#جے_بنا_تعریف SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#وإلا
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#جے_انت
  .n_کلید_ناپ        = KVRECORD_کلید_SZ,
  .n_بفر_ناپ     = SQLITE_KVOS_SZ,
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
#جے_بنا_تعریف SQLITE_WASM
ساكن
#جے_انت
صحيح kvvfsEncode(ثابت حرف *a_انکڑے, صحيح n_انکڑے, حرف *a_باہرلا){
  صحيح i, j;
  ثابت بنانشانی حرف *a = (ثابت بنانشانی حرف*)a_انکڑے;
  لكل(i=j=0; i<n_انکڑے; i++){
    بنانشانی حرف c = a[i];
    إذا( c!=0 ){
      a_باہرلا[j++] = "0123456789ABCDEF"[c>>4];
      a_باہرلا[j++] = "0123456789ABCDEF"[c&0xf];
    }وإلا{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      صحيح k;
      لكل(k=1; i+k<n_انکڑے && a[i+k]==0; k++){}
      i += k-1;
      طالما( k>0 ){
        a_باہرلا[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  a_باہرلا[j] = 0;
  أرجع j;
}

ساكن ثابت نشانیوالا حرف kvvfs_Hex_قدر[256] = {
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
#جے_بنا_تعریف SQLITE_WASM
ساكن
#جے_انت
صحيح kvvfsDecode(ثابت حرف *a, حرف *a_باہرلا, صحيح n_باہرلا){
  صحيح i, j;
  صحيح c;
  ثابت بنانشانی حرف *a_وچ = (ثابت بنانشانی حرف*)a;
  i = 0;
  j = 0;
  طالما( 1 ){
    c = kvvfs_Hex_قدر[a_وچ[i]];
    إذا( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = a_وچ[i];
      إذا( c==0 ) اكسر;
      طالما( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        إذا( n>n_باہرلا ) أرجع -1 /* oversized/malformed input */;
        mult *= 26;
        c = a_وچ[++i];
      }
      إذا( j+n>n_باہرلا ) أرجع -1 /* oversized/malformed input */;
      memset(&a_باہرلا[j], 0, n);
      j += n;
      إذا( c==0 || mult==1 ) اكسر; /* progress stalled if mult==1 */
    }وإلا إذا( j<n_باہرلا ){
      a_باہرلا[j] = c<<4;
      c = kvvfs_Hex_قدر[a_وچ[++i]];
      إذا( c<0 ) أرجع -1 /* hex bytes are always in pairs */;
      a_باہرلا[j++] += c;
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
  بنانشانی صحيح n = 0, mult;
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
ساكن sqlite3_int64 kvvfs_Read_File_ناپ(KVVfsFile *pFile){
  حرف z_انکڑے[50];
  z_انکڑے[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", z_انکڑے,
                                حجم(z_انکڑے)-1);
  أرجع strtoll(z_انکڑے, 0, 0);
}
ساكن صحيح kvvfs_Write_File_ناپ(KVVfsFile *pFile, sqlite3_int64 sz){
  حرف z_انکڑے[50];
  sqlite3_snprintf(حجم(z_انکڑے), z_انکڑے, "%lld", sz);
  أرجع sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", z_انکڑے);
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
  sqlite3_free(pFile->a_انکڑے);
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
  حافظہ_نقل(zBuf, pFile->aJrnl+iOfst, iAmt);
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
  بنانشانی صحيح pgno;
  صحيح got, n;
  حرف z_کلید[30];
  حرف *a_انکڑے = pFile->a_انکڑے;
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
    pFile->sz_ورقہ = iAmt;
    pgno = 1 + iOfst/iAmt;
  }وإلا{
    pgno = 1;
  }
  sqlite3_snprintf(حجم(z_کلید), z_کلید, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_کلید,
                                      a_انکڑے, SQLITE_KVOS_SZ-1);
  إذا( got<0 ){
    n = 0;
  }وإلا{
    a_انکڑے[got] = 0;
    إذا( iOfst+iAmt<512 ){
      صحيح k = iOfst+iAmt;
      a_انکڑے[k*2] = 0;
      n = kvvfsDecode(a_انکڑے, &a_انکڑے[2000], SQLITE_KVOS_SZ-2000);
      إذا( n>=iOfst+iAmt ){
        حافظہ_نقل(zBuf, &a_انکڑے[2000+iOfst], iAmt);
        n = iAmt;
      }وإلا{
        n = 0;
      }
    }وإلا{
      n = kvvfsDecode(a_انکڑے, zBuf, iAmt);
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
  sqlite3_int64 i_سرا = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  إذا( i_سرا>=0x10000000 ) أرجع SQLITE_بھریا;
  إذا( pFile->aJrnl==0 || pFile->nJrnl<i_سرا ){
    حرف *aNew = sqlite3_realloc(pFile->aJrnl, i_سرا);
    إذا( aNew==0 ){
      أرجع SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    إذا( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = i_سرا;
  }
  حافظہ_نقل(pFile->aJrnl+iOfst, zBuf, iAmt);
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
  بنانشانی صحيح pgno;
  حرف z_کلید[30];
  حرف *a_انکڑے = pFile->a_انکڑے;
  صحيح rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  أكد( iAmt>=512 && iAmt<=65536 );
  أكد( (iAmt & (iAmt-1))==0 );
  أكد( pFile->sz_ورقہ<0 || pFile->sz_ورقہ==iAmt );
  pFile->sz_ورقہ = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(حجم(z_کلید), z_کلید, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, a_انکڑے);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_کلید, a_انکڑے);
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
ساكن صحيح kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 ناپ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, ناپ));
  أكد( ناپ==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  أرجع SQLITE_OK;
}
ساكن صحيح kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 ناپ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  إذا( pFile->szDb>ناپ
   && pFile->sz_ورقہ>0
   && (ناپ % pFile->sz_ورقہ)==0
  ){
    حرف z_کلید[50];
    بنانشانی صحيح pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, ناپ));
    pgno = 1 + ناپ/pFile->sz_ورقہ;
    pgnoMax = 2 + pFile->szDb/pFile->sz_ورقہ;
    طالما( pgno<=pgnoMax ){
      sqlite3_snprintf(حجم(z_کلید), z_کلید, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_کلید);
      pgno++;
    }
    pFile->szDb = ناپ;
    أرجع kvvfs_Write_File_ناپ(pFile, ناپ) ? SQLITE_IOERR : SQLITE_OK;
  }
  أرجع SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
ساكن صحيح kvvfsSyncJrnl(sqlite3_file *pProtoFile, صحيح flags){
  صحيح i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  حرف *z_باہرلا;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  إذا( pFile->nJrnl<=0 ){
    أرجع kvvfsTruncateJrnl(pProtoFile, 0);
  }
  z_باہرلا = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  إذا( z_باہرلا==0 ){
    أرجع SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  افعل{
    z_باہرلا[i++] = 'a' + (n%26);
    n /= 26;
  }طالما( n>0 );
  z_باہرلا[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &z_باہرلا[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", z_باہرلا);
  sqlite3_free(z_باہرلا);
  أرجع i ? SQLITE_IOERR : SQLITE_OK;
}
ساكن صحيح kvvfsSyncDb(sqlite3_file *pProtoFile, صحيح flags){
  أرجع SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
ساكن صحيح kvvfs_File_ناپ_Jrnl(sqlite3_file *pProtoFile, sqlite_int64 *p_ناپ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *p_ناپ = pFile->nJrnl;
  أرجع SQLITE_OK;
}
ساكن صحيح kvvfs_File_ناپ_Db(sqlite3_file *pProtoFile, sqlite_int64 *p_ناپ){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  إذا( pFile->szDb>=0 ){
    *p_ناپ = pFile->szDb;
  }وإلا{
    *p_ناپ = kvvfs_Read_File_ناپ(pFile);
  }
  أرجع SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
ساكن صحيح kvvfs_جندرا(sqlite3_file *pProtoFile, صحيح e_جندرا){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, e_جندرا));

  إذا( e_جندرا!=SQLITE_جندرا_NONE ){
    pFile->szDb = kvvfs_Read_File_ناپ(pFile);
  }
  أرجع SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
ساكن صحيح kvvfsUnlock(sqlite3_file *pProtoFile, صحيح e_جندرا){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  أكد( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, e_جندرا));
  إذا( e_جندرا==SQLITE_جندرا_NONE ){
    pFile->szDb = -1;
  }
  أرجع SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
ساكن صحيح kvvfs_Check_Reserved_جندرا(sqlite3_file *pProtoFile, صحيح *p_Res_باہرلا){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *p_Res_باہرلا = 0;
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
    إذا( pFile->szDb>0 && 0!=kvvfs_Write_File_ناپ(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    أرجع rc;
  }
  أرجع SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
ساكن صحيح kvvfs_Sector_ناپ(sqlite3_file *pFile){
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
  صحيح *p_باہرلا_Flags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  إذا( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  أكد(!pFile->zClass);
  أكد(!pFile->a_انکڑے);
  أكد(!pFile->aJrnl);
  أكد(!pFile->nJrnl);
  أكد(!pFile->ادھار.pMethods);
  pFile->sz_ورقہ = -1;
  pFile->szDb = -1;
  إذا( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->ادھار.pMethods = &kvvfs_jrnl_io_methods;
    إذا( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }وإلا إذا( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }وإلا{
    pFile->isJournal = 0;
    pFile->ادھار.pMethods = &kvvfs_db_io_methods;
  }
  إذا( !pFile->zClass ){
#جے_تعریف_شدہ SQLITE_WASM
    إذا( لڑی_لمبائی(zName) >= (KVRECORD_کلید_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      أرجع SQLITE_CANTOPEN;
    }
#وإلا
    إذا( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      أرجع SQLITE_CANTOPEN;
    }
#جے_انت
    pFile->zClass = zName;
  }
  pFile->a_انکڑے = sqlite3_malloc64(SQLITE_KVOS_SZ);
  إذا( pFile->a_انکڑے==0 ){
    أرجع SQLITE_NOMEM;
  }
  أرجع SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح kvvfsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_رستہ, صحيح dirSync){
  صحيح rc /* The JS impl can fail with OOM in argument conversion */;
  إذا( strcmp(z_رستہ, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }وإلا
  إذا( strcmp(z_رستہ, "session-journal")==0 ){
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
  ثابت حرف *z_رستہ,
  صحيح flags,
  صحيح *p_Res_باہرلا
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", z_رستہ));
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
  ثابت حرف *z_کلید = (0==sqlite3_strglob("*-journal", z_رستہ))
    ? "jrnl" : "sz";
  *p_Res_باہرلا =
    sqlite3KvvfsMethods.xRcrdRead(z_رستہ, z_کلید, 0, 0)>0;
#وإلا
  إذا( strcmp(z_رستہ, "local-journal")==0 ){
    *p_Res_باہرلا =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }وإلا
  إذا( strcmp(z_رستہ, "session-journal")==0 ){
    *p_Res_باہرلا =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }وإلا
  إذا( strcmp(z_رستہ, "local")==0 ){
    *p_Res_باہرلا =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }وإلا
  إذا( strcmp(z_رستہ, "session")==0 ){
    *p_Res_باہرلا =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }وإلا
  {
    *p_Res_باہرلا = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#جے_انت
  SQLITE_KV_LOG(("xAccess returns %d\n",*p_Res_باہرلا));
  أرجع SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح kvvfs_بھریا_Pathname(
  sqlite3_vfs *pVfs,
  ثابت حرف *z_رستہ,
  صحيح n_باہرلا,
  حرف *z_باہرلا
){
  size_t n_رستہ;
#جے_تعریف_شدہ SQLITE_OS_KV_ALWAYS_LOCAL
  z_رستہ = "local";
#جے_انت
  n_رستہ = لڑی_لمبائی(z_رستہ);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", z_رستہ));
  إذا( n_باہرلا<n_رستہ+1 ) n_رستہ = n_باہرلا - 1;
  حافظہ_نقل(z_باہرلا, z_رستہ, n_رستہ);
  z_باہرلا[n_رستہ] = 0;
  أرجع SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *kvvfsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_رستہ){
  أرجع 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
ساكن صحيح kvvfsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_باہرلا){
  memset(z_Buf_باہرلا, 0, nByte);
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
ساكن صحيح kvvfs_Current_ویلا(sqlite3_vfs *pVfs, مزدوج *p_ویلا_باہرلا){
  sqlite3_int64 i = 0;
  صحيح rc;
  rc = kvvfs_Current_ویلا_Int64(0, &i);
  *p_ویلا_باہرلا = i/86400000.0;
  أرجع rc;
}
#ضمن <sys/time.h>
ساكن صحيح kvvfs_Current_ویلا_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p_ویلا_باہرلا){
  ساكن ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  بنية timeval sNow;
  (فراغ)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *p_ویلا_باہرلا = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  أرجع SQLITE_OK;
}
#جے_انت /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#إذا SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
صحيح sqlite3_os_init(فراغ){
  أرجع sqlite3_vfs_رجسٹر(&sqlite3OsKvvfsObject, 1);
}
صحيح sqlite3_os_سرا(فراغ){
  أرجع SQLITE_OK;
}
#جے_انت /* SQLITE_OS_KV */

#إذا SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
صحيح sqlite3KvvfsInit(فراغ){
  أرجع sqlite3_vfs_رجسٹر(&sqlite3OsKvvfsObject, 0);
}
#جے_انت
