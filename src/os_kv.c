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
#include <sqliteInt.h>
#ຖ້າ SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#ຖ້າ 0
#define SQLITE_KV_TRACE(X)  printf X
#ບໍ່ດັ່ງນັ້ນ
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#ຖ້າ 0
#define SQLITE_KV_LOG(X)  printf X
#ບໍ່ດັ່ງນັ້ນ
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
typedef sa_nracanaa KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
sa_nracanaa KVVfsFile {
  sqlite3_file base;              /* IO methods */
  const var_na *zClass;             /* Storage class */
  ຈຳນວນເຕັມ isJournal;                  /* True if this is a journal file */
  unsigned ຈຳນວນເຕັມ nJrnl;             /* Space allocated for aJrnl[] */
  var_na *aJrnl;                    /* Journal content */
  ຈຳນວນເຕັມ szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  var_na *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
static ຈຳນວນເຕັມ kvvfsClose(sqlite3_file*);
static ຈຳນວນເຕັມ kvvfsReadDb(sqlite3_file*, void*, ຈຳນວນເຕັມ iAmt, sqlite3_int64 iOfst);
static ຈຳນວນເຕັມ kvvfsReadJrnl(sqlite3_file*, void*, ຈຳນວນເຕັມ iAmt, sqlite3_int64 iOfst);
static ຈຳນວນເຕັມ kvvfsWriteDb(sqlite3_file*,const void*,ຈຳນວນເຕັມ iAmt, sqlite3_int64);
static ຈຳນວນເຕັມ kvvfsWriteJrnl(sqlite3_file*,const void*,ຈຳນວນເຕັມ iAmt, sqlite3_int64);
static ຈຳນວນເຕັມ kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
static ຈຳນວນເຕັມ kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
static ຈຳນວນເຕັມ kvvfsSyncDb(sqlite3_file*, ຈຳນວນເຕັມ flags);
static ຈຳນວນເຕັມ kvvfsSyncJrnl(sqlite3_file*, ຈຳນວນເຕັມ flags);
static ຈຳນວນເຕັມ kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
static ຈຳນວນເຕັມ kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
static ຈຳນວນເຕັມ kvvfsLock(sqlite3_file*, ຈຳນວນເຕັມ);
static ຈຳນວນເຕັມ kvvfsUnlock(sqlite3_file*, ຈຳນວນເຕັມ);
static ຈຳນວນເຕັມ kvvfsCheckReservedLock(sqlite3_file*, ຈຳນວນເຕັມ *pResOut);
static ຈຳນວນເຕັມ kvvfsFileControlDb(sqlite3_file*, ຈຳນວນເຕັມ op, void *pArg);
static ຈຳນວນເຕັມ kvvfsFileControlJrnl(sqlite3_file*, ຈຳນວນເຕັມ op, void *pArg);
static ຈຳນວນເຕັມ kvvfsSectorSize(sqlite3_file*);
static ຈຳນວນເຕັມ kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
static ຈຳນວນເຕັມ kvvfsOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, ຈຳນວນເຕັມ , ຈຳນວນເຕັມ *);
static ຈຳນວນເຕັມ kvvfsDelete(sqlite3_vfs*, const var_na *zName, ຈຳນວນເຕັມ syncDir);
static ຈຳນວນເຕັມ kvvfsAccess(sqlite3_vfs*, const var_na *zName, ຈຳນວນເຕັມ flags, ຈຳນວນເຕັມ *);
static ຈຳນວນເຕັມ kvvfsFullPathname(sqlite3_vfs*, const var_na *zName, ຈຳນວນເຕັມ, var_na *zOut);
static void *kvvfsDlOpen(sqlite3_vfs*, const var_na *zFilename);
static ຈຳນວນເຕັມ kvvfsRandomness(sqlite3_vfs*, ຈຳນວນເຕັມ nByte, var_na *zOut);
static ຈຳນວນເຕັມ kvvfsSleep(sqlite3_vfs*, ຈຳນວນເຕັມ microseconds);
static ຈຳນວນເຕັມ kvvfsCurrentTime(sqlite3_vfs*, dviguu_na*);
static ຈຳນວນເຕັມ kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

static sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  parimaa_na(KVVfsFile),              /* szOsFile */
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
static sqlite3_io_methods kvvfs_db_io_methods = {
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
static sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
static ຈຳນວນເຕັມ kvrecordWrite(const var_na*, const var_na *zKey, const var_na *zData);
static ຈຳນວນເຕັມ kvrecordDelete(const var_na*, const var_na *zKey);
static ຈຳນວນເຕັມ kvrecordRead(const var_na*, const var_na *zKey, var_na *zBuf, ຈຳນວນເຕັມ nBuf);
#endif
#ifndef KVRECORD_KEY_SZ
#define KVRECORD_KEY_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
static void kvrecordMakeKey(
  const var_na *zClass,
  const var_na *zKeyIn,
  var_na *zKeyOut
){
  assert( zKeyIn );
  assert( zKeyOut );
  assert( zClass );
  sqlite3_snprintf(KVRECORD_KEY_SZ, zKeyOut, "kvvfs-%s-%s",
                   zClass, zKeyIn);
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
static ຈຳນວນເຕັມ kvrecordWrite(
  const var_na *zClass,
  const var_na *zKey,
  const var_na *zData
){
  FILE *fd;
  var_na zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  fd = fopen(zXKey, "wb");
  ຖ້າ( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (ຈຳນວນເຕັມ)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    ສົ່ງຄືນ 0;
  }ບໍ່ດັ່ງນັ້ນ{
    ສົ່ງຄືນ 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
static ຈຳນວນເຕັມ kvrecordDelete(const var_na *zClass, const var_na *zKey){
  var_na zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  ສົ່ງຄືນ 0;
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
static ຈຳນວນເຕັມ kvrecordRead(
  const var_na *zClass,
  const var_na *zKey,
  var_na *zBuf,
  ຈຳນວນເຕັມ nBuf
){
  FILE *fd;
  sa_nracanaa stat buf;
  var_na zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  ຖ້າ( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ສົ່ງຄືນ -1;
  }
  ຖ້າ( nBuf<=0 ){
    ສົ່ງຄືນ (ຈຳນວນເຕັມ)buf.st_size;
  }ບໍ່ດັ່ງນັ້ນ ຖ້າ( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (ຈຳນວນເຕັມ)buf.st_size));
    ສົ່ງຄືນ (ຈຳນວນເຕັມ)buf.st_size;
  }
  ຖ້າ( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  ຖ້າ( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ສົ່ງຄືນ -1;
  }ບໍ່ດັ່ງນັ້ນ{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    ສົ່ງຄືນ (ຈຳນວນເຕັມ)n;
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
typedef sa_nracanaa sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
sa_nracanaa sqlite3_kvvfs_methods {
  ຈຳນວນເຕັມ (*xRcrdRead)(const var_na*, const var_na *zKey, var_na *zBuf, ຈຳນວນເຕັມ nBuf);
  ຈຳນວນເຕັມ (*xRcrdWrite)(const var_na*, const var_na *zKey, const var_na *zData);
  ຈຳນວນເຕັມ (*xRcrdDelete)(const var_na*, const var_na *zKey);
  const ຈຳນວນເຕັມ nKeySize;
  const ຈຳນວນເຕັມ nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST const
#ບໍ່ດັ່ງນັ້ນ
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
const
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#ບໍ່ດັ່ງນັ້ນ
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .nKeySize        = KVRECORD_KEY_SZ,
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
static
#endif
ຈຳນວນເຕັມ kvvfsEncode(const var_na *aData, ຈຳນວນເຕັມ nData, var_na *aOut){
  ຈຳນວນເຕັມ i, j;
  const unsigned var_na *a = (const unsigned var_na*)aData;
  ສຳລັບ(i=j=0; i<nData; i++){
    unsigned var_na c = a[i];
    ຖ້າ( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }ບໍ່ດັ່ງນັ້ນ{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      ຈຳນວນເຕັມ k;
      ສຳລັບ(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      ໃນຂະນະທີ່( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  ສົ່ງຄືນ j;
}

static const signed var_na kvvfsHexValue[256] = {
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
static
#endif
ຈຳນວນເຕັມ kvvfsDecode(const var_na *a, var_na *aOut, ຈຳນວນເຕັມ nOut){
  ຈຳນວນເຕັມ i, j;
  ຈຳນວນເຕັມ c;
  const unsigned var_na *aIn = (const unsigned var_na*)a;
  i = 0;
  j = 0;
  ໃນຂະນະທີ່( 1 ){
    c = kvvfsHexValue[aIn[i]];
    ຖ້າ( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      ຖ້າ( c==0 ) bha_nga;
      ໃນຂະນະທີ່( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        ຖ້າ( n>nOut ) ສົ່ງຄືນ -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      ຖ້າ( j+n>nOut ) ສົ່ງຄືນ -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      ຖ້າ( c==0 || mult==1 ) bha_nga; /* progress stalled if mult==1 */
    }ບໍ່ດັ່ງນັ້ນ ຖ້າ( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfsHexValue[aIn[++i]];
      ຖ້າ( c<0 ) ສົ່ງຄືນ -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }ບໍ່ດັ່ງນັ້ນ{
      ສົ່ງຄືນ -1;
    }
  }
  ສົ່ງຄືນ j;
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
static void kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  const var_na *zTxt,      /* Text encoding.  Zero-terminated */
  ຈຳນວນເຕັມ nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned ຈຳນວນເຕັມ n = 0, mult;
  ຈຳນວນເຕັມ c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ໃນຂະນະທີ່( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  ຖ້າ( ' '!=zTxt[i++] ){
    /* Malformed input */
    ສົ່ງຄືນ;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  ຖ້າ( pFile->aJrnl==0 ){
    ສົ່ງຄືນ;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  ຖ້າ( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
static sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  var_na zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                parimaa_na(zData)-1);
  ສົ່ງຄືນ strtoll(zData, 0, 0);
}
static ຈຳນວນເຕັມ kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  var_na zData[50];
  sqlite3_snprintf(parimaa_na(zData), zData, "%lld", sz);
  ສົ່ງຄືນ sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, parimaa_na(*pFile));
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Read from the -journal file.
*/
static ຈຳນວນເຕັມ kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  void *zBuf,
  ຈຳນວນເຕັມ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  assert( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ຖ້າ( pFile->aJrnl==0 ){
    ຈຳນວນເຕັມ rc;
    ຈຳນວນເຕັມ szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    var_na *aTxt;
    ຖ້າ( szTxt<=4 ){
      ສົ່ງຄືນ SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    ຖ້າ( aTxt==0 ) ສົ່ງຄືນ SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    ຖ້າ( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    ຖ້າ( rc ) ສົ່ງຄືນ rc;
    ຖ້າ( pFile->aJrnl==0 ) ສົ່ງຄືນ SQLITE_IOERR;
  }
  ຖ້າ( iOfst+iAmt>pFile->nJrnl ){
    ສົ່ງຄືນ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Read from the database file.
*/
static ຈຳນວນເຕັມ kvvfsReadDb(
  sqlite3_file *pProtoFile,
  void *zBuf,
  ຈຳນວນເຕັມ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned ຈຳນວນເຕັມ pgno;
  ຈຳນວນເຕັມ got, n;
  var_na zKey[30];
  var_na *aData = pFile->aData;
  assert( iOfst>=0 );
  assert( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ຖ້າ( iOfst+iAmt>=512 ){
    ຖ້າ( (iOfst % iAmt)!=0 ){
      ສົ່ງຄືນ SQLITE_IOERR_READ;
    }
    ຖ້າ( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      ສົ່ງຄືນ SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }ບໍ່ດັ່ງນັ້ນ{
    pgno = 1;
  }
  sqlite3_snprintf(parimaa_na(zKey), zKey, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, zKey,
                                      aData, SQLITE_KVOS_SZ-1);
  ຖ້າ( got<0 ){
    n = 0;
  }ບໍ່ດັ່ງນັ້ນ{
    aData[got] = 0;
    ຖ້າ( iOfst+iAmt<512 ){
      ຈຳນວນເຕັມ k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      ຖ້າ( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }ບໍ່ດັ່ງນັ້ນ{
        n = 0;
      }
    }ບໍ່ດັ່ງນັ້ນ{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  ຖ້າ( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    ສົ່ງຄືນ SQLITE_IOERR_SHORT_READ;
  }
  ສົ່ງຄືນ SQLITE_OK;
}


/*
** Write into the -journal file.
*/
static ຈຳນວນເຕັມ kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  const void *zBuf,
  ຈຳນວນເຕັມ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ຖ້າ( iEnd>=0x10000000 ) ສົ່ງຄືນ SQLITE_FULL;
  ຖ້າ( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    var_na *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    ຖ້າ( aNew==0 ){
      ສົ່ງຄືນ SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    ຖ້າ( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Write into the database file.
*/
static ຈຳນວນເຕັມ kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  const void *zBuf,
  ຈຳນວນເຕັມ iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned ຈຳນວນເຕັມ pgno;
  var_na zKey[30];
  var_na *aData = pFile->aData;
  ຈຳນວນເຕັມ rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  assert( iAmt>=512 && iAmt<=65536 );
  assert( (iAmt & (iAmt-1))==0 );
  assert( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(parimaa_na(zKey), zKey, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, zKey, aData);
  ຖ້າ( 0==rc ){
    ຖ້າ( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  ສົ່ງຄືນ rc;
}

/*
** Truncate an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  assert( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ສົ່ງຄືນ SQLITE_OK;
}
static ຈຳນວນເຕັມ kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ຖ້າ( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    var_na zKey[50];
    unsigned ຈຳນວນເຕັມ pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    ໃນຂະນະທີ່( pgno<=pgnoMax ){
      sqlite3_snprintf(parimaa_na(zKey), zKey, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, zKey);
      pgno++;
    }
    pFile->szDb = size;
    ສົ່ງຄືນ kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  ສົ່ງຄືນ SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsSyncJrnl(sqlite3_file *pProtoFile, ຈຳນວນເຕັມ flags){
  ຈຳນວນເຕັມ i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  var_na *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  ຖ້າ( pFile->nJrnl<=0 ){
    ສົ່ງຄືນ kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  ຖ້າ( zOut==0 ){
    ສົ່ງຄືນ SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  do{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }ໃນຂະນະທີ່( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  ສົ່ງຄືນ i ? SQLITE_IOERR : SQLITE_OK;
}
static ຈຳນວນເຕັມ kvvfsSyncDb(sqlite3_file *pProtoFile, ຈຳນວນເຕັມ flags){
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  ສົ່ງຄືນ SQLITE_OK;
}
static ຈຳນວນເຕັມ kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  ຖ້າ( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }ບໍ່ດັ່ງນັ້ນ{
    *pSize = kvvfsReadFileSize(pFile);
  }
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsLock(sqlite3_file *pProtoFile, ຈຳນວນເຕັມ eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  assert( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  ຖ້າ( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsUnlock(sqlite3_file *pProtoFile, ຈຳນວນເຕັມ eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  assert( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  ຖ້າ( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsCheckReservedLock(sqlite3_file *pProtoFile, ຈຳນວນເຕັມ *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsFileControlJrnl(sqlite3_file *pProtoFile, ຈຳນວນເຕັມ op, void *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  ສົ່ງຄືນ SQLITE_NOTFOUND;
}
static ຈຳນວນເຕັມ kvvfsFileControlDb(sqlite3_file *pProtoFile, ຈຳນວນເຕັມ op, void *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  ຖ້າ( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    ຈຳນວນເຕັມ rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    ຖ້າ( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    ສົ່ງຄືນ rc;
  }
  ສົ່ງຄືນ SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsSectorSize(sqlite3_file *pFile){
  ສົ່ງຄືນ 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
static ຈຳນວນເຕັມ kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  ສົ່ງຄືນ 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
static ຈຳນວນເຕັມ kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  const var_na *zName,
  sqlite3_file *pProtoFile,
  ຈຳນວນເຕັມ flags,
  ຈຳນວນເຕັມ *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ຖ້າ( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  assert(!pFile->zClass);
  assert(!pFile->aData);
  assert(!pFile->aJrnl);
  assert(!pFile->nJrnl);
  assert(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  ຖ້າ( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    ຖ້າ( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }ບໍ່ດັ່ງນັ້ນ ຖ້າ( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }ບໍ່ດັ່ງນັ້ນ{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  ຖ້າ( !pFile->zClass ){
#ifdef SQLITE_WASM
    ຖ້າ( strlen(zName) >= (KVRECORD_KEY_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      ສົ່ງຄືນ SQLITE_CANTOPEN;
    }
#ບໍ່ດັ່ງນັ້ນ
    ຖ້າ( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      ສົ່ງຄືນ SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  ຖ້າ( pFile->aData==0 ){
    ສົ່ງຄືນ SQLITE_NOMEM;
  }
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static ຈຳນວນເຕັມ kvvfsDelete(sqlite3_vfs *pVfs, const var_na *zPath, ຈຳນວນເຕັມ dirSync){
  ຈຳນວນເຕັມ rc /* The JS impl can fail with OOM in argument conversion */;
  ຖ້າ( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }ບໍ່ດັ່ງນັ້ນ
  ຖ້າ( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  ບໍ່ດັ່ງນັ້ນ{
    rc = 0;
  }
  ສົ່ງຄືນ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static ຈຳນວນເຕັມ kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  const var_na *zPath,
  ຈຳນວນເຕັມ flags,
  ຈຳນວນເຕັມ *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#ຖ້າ 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  const var_na *zKey = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, zKey, 0, 0)>0;
#ບໍ່ດັ່ງນັ້ນ
  ຖ້າ( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }ບໍ່ດັ່ງນັ້ນ
  ຖ້າ( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }ບໍ່ດັ່ງນັ້ນ
  ຖ້າ( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }ບໍ່ດັ່ງນັ້ນ
  ຖ້າ( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }ບໍ່ດັ່ງນັ້ນ
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static ຈຳນວນເຕັມ kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  ຈຳນວນເຕັມ nOut,
  var_na *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  ຖ້າ( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *kvvfsDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  ສົ່ງຄືນ 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
static ຈຳນວນເຕັມ kvvfsRandomness(sqlite3_vfs *pVfs, ຈຳນວນເຕັມ nByte, var_na *zBufOut){
  memset(zBufOut, 0, nByte);
  ສົ່ງຄືນ nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
static ຈຳນວນເຕັມ kvvfsSleep(sqlite3_vfs *pVfs, ຈຳນວນເຕັມ nMicro){
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static ຈຳນວນເຕັມ kvvfsCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  sqlite3_int64 i = 0;
  ຈຳນວນເຕັມ rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  ສົ່ງຄືນ rc;
}
#include <sys/time.h>
static ຈຳນວນເຕັມ kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  static const sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  sa_nracanaa timeval sNow;
  (void)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  ສົ່ງຄືນ SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#ຖ້າ SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
ຈຳນວນເຕັມ sqlite3_os_init(void){
  ສົ່ງຄືນ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
ຈຳນວນເຕັມ sqlite3_os_end(void){
  ສົ່ງຄືນ SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#ຖ້າ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
ຈຳນວນເຕັມ sqlite3KvvfsInit(void){
  ສົ່ງຄືນ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
