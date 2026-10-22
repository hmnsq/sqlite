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
#ᬬᬾᬦ᭄ SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#ᬬᬾᬦ᭄ 0
#define SQLITE_KV_TRACE(X)  printf X
#ᬮᬶᬬ
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#ᬬᬾᬦ᭄ 0
#define SQLITE_KV_LOG(X)  printf X
#ᬮᬶᬬ
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
  puur_naa_nka isJournal;                  /* True if this is a journal file */
  unsigned puur_naa_nka nJrnl;             /* Space allocated for aJrnl[] */
  var_na *aJrnl;                    /* Journal content */
  puur_naa_nka szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  var_na *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
static puur_naa_nka kvvfsClose(sqlite3_file*);
static puur_naa_nka kvvfsReadDb(sqlite3_file*, void*, puur_naa_nka iAmt, sqlite3_int64 iOfst);
static puur_naa_nka kvvfsReadJrnl(sqlite3_file*, void*, puur_naa_nka iAmt, sqlite3_int64 iOfst);
static puur_naa_nka kvvfsWriteDb(sqlite3_file*,const void*,puur_naa_nka iAmt, sqlite3_int64);
static puur_naa_nka kvvfsWriteJrnl(sqlite3_file*,const void*,puur_naa_nka iAmt, sqlite3_int64);
static puur_naa_nka kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
static puur_naa_nka kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
static puur_naa_nka kvvfsSyncDb(sqlite3_file*, puur_naa_nka flags);
static puur_naa_nka kvvfsSyncJrnl(sqlite3_file*, puur_naa_nka flags);
static puur_naa_nka kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
static puur_naa_nka kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
static puur_naa_nka kvvfsLock(sqlite3_file*, puur_naa_nka);
static puur_naa_nka kvvfsUnlock(sqlite3_file*, puur_naa_nka);
static puur_naa_nka kvvfsCheckReservedLock(sqlite3_file*, puur_naa_nka *pResOut);
static puur_naa_nka kvvfsFileControlDb(sqlite3_file*, puur_naa_nka op, void *pArg);
static puur_naa_nka kvvfsFileControlJrnl(sqlite3_file*, puur_naa_nka op, void *pArg);
static puur_naa_nka kvvfsSectorSize(sqlite3_file*);
static puur_naa_nka kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
static puur_naa_nka kvvfsOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, puur_naa_nka , puur_naa_nka *);
static puur_naa_nka kvvfsDelete(sqlite3_vfs*, const var_na *zName, puur_naa_nka syncDir);
static puur_naa_nka kvvfsAccess(sqlite3_vfs*, const var_na *zName, puur_naa_nka flags, puur_naa_nka *);
static puur_naa_nka kvvfsFullPathname(sqlite3_vfs*, const var_na *zName, puur_naa_nka, var_na *zOut);
static void *kvvfsDlOpen(sqlite3_vfs*, const var_na *zFilename);
static puur_naa_nka kvvfsRandomness(sqlite3_vfs*, puur_naa_nka nByte, var_na *zOut);
static puur_naa_nka kvvfsSleep(sqlite3_vfs*, puur_naa_nka microseconds);
static puur_naa_nka kvvfsCurrentTime(sqlite3_vfs*, dviguu_na*);
static puur_naa_nka kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

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
static puur_naa_nka kvrecordWrite(const var_na*, const var_na *zKey, const var_na *zData);
static puur_naa_nka kvrecordDelete(const var_na*, const var_na *zKey);
static puur_naa_nka kvrecordRead(const var_na*, const var_na *zKey, var_na *zBuf, puur_naa_nka nBuf);
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
static puur_naa_nka kvrecordWrite(
  const var_na *zClass,
  const var_na *zKey,
  const var_na *zData
){
  FILE *fd;
  var_na zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  fd = fopen(zXKey, "wb");
  ᬬᬾᬦ᭄( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (puur_naa_nka)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    ᬯᬮᬶ 0;
  }ᬮᬶᬬ{
    ᬯᬮᬶ 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
static puur_naa_nka kvrecordDelete(const var_na *zClass, const var_na *zKey){
  var_na zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  ᬯᬮᬶ 0;
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
static puur_naa_nka kvrecordRead(
  const var_na *zClass,
  const var_na *zKey,
  var_na *zBuf,
  puur_naa_nka nBuf
){
  FILE *fd;
  sa_nracanaa stat buf;
  var_na zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  ᬬᬾᬦ᭄( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ᬯᬮᬶ -1;
  }
  ᬬᬾᬦ᭄( nBuf<=0 ){
    ᬯᬮᬶ (puur_naa_nka)buf.st_size;
  }ᬮᬶᬬ ᬬᬾᬦ᭄( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (puur_naa_nka)buf.st_size));
    ᬯᬮᬶ (puur_naa_nka)buf.st_size;
  }
  ᬬᬾᬦ᭄( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  ᬬᬾᬦ᭄( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    ᬯᬮᬶ -1;
  }ᬮᬶᬬ{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    ᬯᬮᬶ (puur_naa_nka)n;
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
  puur_naa_nka (*xRcrdRead)(const var_na*, const var_na *zKey, var_na *zBuf, puur_naa_nka nBuf);
  puur_naa_nka (*xRcrdWrite)(const var_na*, const var_na *zKey, const var_na *zData);
  puur_naa_nka (*xRcrdDelete)(const var_na*, const var_na *zKey);
  const puur_naa_nka nKeySize;
  const puur_naa_nka nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST const
#ᬮᬶᬬ
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
#ᬮᬶᬬ
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
puur_naa_nka kvvfsEncode(const var_na *aData, puur_naa_nka nData, var_na *aOut){
  puur_naa_nka i, j;
  const unsigned var_na *a = (const unsigned var_na*)aData;
  for(i=j=0; i<nData; i++){
    unsigned var_na c = a[i];
    ᬬᬾᬦ᭄( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }ᬮᬶᬬ{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      puur_naa_nka k;
      for(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      ᬲᭂᬤᭂᬮ( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  ᬯᬮᬶ j;
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
puur_naa_nka kvvfsDecode(const var_na *a, var_na *aOut, puur_naa_nka nOut){
  puur_naa_nka i, j;
  puur_naa_nka c;
  const unsigned var_na *aIn = (const unsigned var_na*)a;
  i = 0;
  j = 0;
  ᬲᭂᬤᭂᬮ( 1 ){
    c = kvvfsHexValue[aIn[i]];
    ᬬᬾᬦ᭄( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      ᬬᬾᬦ᭄( c==0 ) bha_nga;
      ᬲᭂᬤᭂᬮ( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        ᬬᬾᬦ᭄( n>nOut ) ᬯᬮᬶ -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      ᬬᬾᬦ᭄( j+n>nOut ) ᬯᬮᬶ -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      ᬬᬾᬦ᭄( c==0 || mult==1 ) bha_nga; /* progress stalled if mult==1 */
    }ᬮᬶᬬ ᬬᬾᬦ᭄( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfsHexValue[aIn[++i]];
      ᬬᬾᬦ᭄( c<0 ) ᬯᬮᬶ -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }ᬮᬶᬬ{
      ᬯᬮᬶ -1;
    }
  }
  ᬯᬮᬶ j;
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
  puur_naa_nka nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned puur_naa_nka n = 0, mult;
  puur_naa_nka c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ᬲᭂᬤᭂᬮ( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  ᬬᬾᬦ᭄( ' '!=zTxt[i++] ){
    /* Malformed input */
    ᬯᬮᬶ;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  ᬬᬾᬦ᭄( pFile->aJrnl==0 ){
    ᬯᬮᬶ;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  ᬬᬾᬦ᭄( n<pFile->nJrnl ){
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
  ᬯᬮᬶ strtoll(zData, 0, 0);
}
static puur_naa_nka kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  var_na zData[50];
  sqlite3_snprintf(parimaa_na(zData), zData, "%lld", sz);
  ᬯᬮᬶ sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
static puur_naa_nka kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, parimaa_na(*pFile));
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Read from the -journal file.
*/
static puur_naa_nka kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  void *zBuf,
  puur_naa_nka iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  assert( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ᬬᬾᬦ᭄( pFile->aJrnl==0 ){
    puur_naa_nka rc;
    puur_naa_nka szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    var_na *aTxt;
    ᬬᬾᬦ᭄( szTxt<=4 ){
      ᬯᬮᬶ SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    ᬬᬾᬦ᭄( aTxt==0 ) ᬯᬮᬶ SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    ᬬᬾᬦ᭄( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    ᬬᬾᬦ᭄( rc ) ᬯᬮᬶ rc;
    ᬬᬾᬦ᭄( pFile->aJrnl==0 ) ᬯᬮᬶ SQLITE_IOERR;
  }
  ᬬᬾᬦ᭄( iOfst+iAmt>pFile->nJrnl ){
    ᬯᬮᬶ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Read from the database file.
*/
static puur_naa_nka kvvfsReadDb(
  sqlite3_file *pProtoFile,
  void *zBuf,
  puur_naa_nka iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned puur_naa_nka pgno;
  puur_naa_nka got, n;
  var_na zKey[30];
  var_na *aData = pFile->aData;
  assert( iOfst>=0 );
  assert( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ᬬᬾᬦ᭄( iOfst+iAmt>=512 ){
    ᬬᬾᬦ᭄( (iOfst % iAmt)!=0 ){
      ᬯᬮᬶ SQLITE_IOERR_READ;
    }
    ᬬᬾᬦ᭄( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      ᬯᬮᬶ SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }ᬮᬶᬬ{
    pgno = 1;
  }
  sqlite3_snprintf(parimaa_na(zKey), zKey, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, zKey,
                                      aData, SQLITE_KVOS_SZ-1);
  ᬬᬾᬦ᭄( got<0 ){
    n = 0;
  }ᬮᬶᬬ{
    aData[got] = 0;
    ᬬᬾᬦ᭄( iOfst+iAmt<512 ){
      puur_naa_nka k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      ᬬᬾᬦ᭄( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }ᬮᬶᬬ{
        n = 0;
      }
    }ᬮᬶᬬ{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  ᬬᬾᬦ᭄( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    ᬯᬮᬶ SQLITE_IOERR_SHORT_READ;
  }
  ᬯᬮᬶ SQLITE_OK;
}


/*
** Write into the -journal file.
*/
static puur_naa_nka kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  const void *zBuf,
  puur_naa_nka iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  ᬬᬾᬦ᭄( iEnd>=0x10000000 ) ᬯᬮᬶ SQLITE_FULL;
  ᬬᬾᬦ᭄( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    var_na *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    ᬬᬾᬦ᭄( aNew==0 ){
      ᬯᬮᬶ SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    ᬬᬾᬦ᭄( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Write into the database file.
*/
static puur_naa_nka kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  const void *zBuf,
  puur_naa_nka iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned puur_naa_nka pgno;
  var_na zKey[30];
  var_na *aData = pFile->aData;
  puur_naa_nka rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  assert( iAmt>=512 && iAmt<=65536 );
  assert( (iAmt & (iAmt-1))==0 );
  assert( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(parimaa_na(zKey), zKey, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, zKey, aData);
  ᬬᬾᬦ᭄( 0==rc ){
    ᬬᬾᬦ᭄( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  ᬯᬮᬶ rc;
}

/*
** Truncate an kvvfs-file.
*/
static puur_naa_nka kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  assert( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  ᬯᬮᬶ SQLITE_OK;
}
static puur_naa_nka kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  ᬬᬾᬦ᭄( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    var_na zKey[50];
    unsigned puur_naa_nka pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    ᬲᭂᬤᭂᬮ( pgno<=pgnoMax ){
      sqlite3_snprintf(parimaa_na(zKey), zKey, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, zKey);
      pgno++;
    }
    pFile->szDb = size;
    ᬯᬮᬶ kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  ᬯᬮᬶ SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
static puur_naa_nka kvvfsSyncJrnl(sqlite3_file *pProtoFile, puur_naa_nka flags){
  puur_naa_nka i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  var_na *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  ᬬᬾᬦ᭄( pFile->nJrnl<=0 ){
    ᬯᬮᬶ kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  ᬬᬾᬦ᭄( zOut==0 ){
    ᬯᬮᬶ SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  do{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }ᬲᭂᬤᭂᬮ( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  ᬯᬮᬶ i ? SQLITE_IOERR : SQLITE_OK;
}
static puur_naa_nka kvvfsSyncDb(sqlite3_file *pProtoFile, puur_naa_nka flags){
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
static puur_naa_nka kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  ᬯᬮᬶ SQLITE_OK;
}
static puur_naa_nka kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  ᬬᬾᬦ᭄( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }ᬮᬶᬬ{
    *pSize = kvvfsReadFileSize(pFile);
  }
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
static puur_naa_nka kvvfsLock(sqlite3_file *pProtoFile, puur_naa_nka eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  assert( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  ᬬᬾᬦ᭄( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
static puur_naa_nka kvvfsUnlock(sqlite3_file *pProtoFile, puur_naa_nka eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  assert( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  ᬬᬾᬦ᭄( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
static puur_naa_nka kvvfsCheckReservedLock(sqlite3_file *pProtoFile, puur_naa_nka *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  ᬯᬮᬶ SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
static puur_naa_nka kvvfsFileControlJrnl(sqlite3_file *pProtoFile, puur_naa_nka op, void *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  ᬯᬮᬶ SQLITE_NOTFOUND;
}
static puur_naa_nka kvvfsFileControlDb(sqlite3_file *pProtoFile, puur_naa_nka op, void *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  ᬬᬾᬦ᭄( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    puur_naa_nka rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    ᬬᬾᬦ᭄( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    ᬯᬮᬶ rc;
  }
  ᬯᬮᬶ SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
static puur_naa_nka kvvfsSectorSize(sqlite3_file *pFile){
  ᬯᬮᬶ 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
static puur_naa_nka kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  ᬯᬮᬶ 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
static puur_naa_nka kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  const var_na *zName,
  sqlite3_file *pProtoFile,
  puur_naa_nka flags,
  puur_naa_nka *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  ᬬᬾᬦ᭄( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  assert(!pFile->zClass);
  assert(!pFile->aData);
  assert(!pFile->aJrnl);
  assert(!pFile->nJrnl);
  assert(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  ᬬᬾᬦ᭄( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    ᬬᬾᬦ᭄( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }ᬮᬶᬬ ᬬᬾᬦ᭄( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }ᬮᬶᬬ{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  ᬬᬾᬦ᭄( !pFile->zClass ){
#ifdef SQLITE_WASM
    ᬬᬾᬦ᭄( strlen(zName) >= (KVRECORD_KEY_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      ᬯᬮᬶ SQLITE_CANTOPEN;
    }
#ᬮᬶᬬ
    ᬬᬾᬦ᭄( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      ᬯᬮᬶ SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  ᬬᬾᬦ᭄( pFile->aData==0 ){
    ᬯᬮᬶ SQLITE_NOMEM;
  }
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static puur_naa_nka kvvfsDelete(sqlite3_vfs *pVfs, const var_na *zPath, puur_naa_nka dirSync){
  puur_naa_nka rc /* The JS impl can fail with OOM in argument conversion */;
  ᬬᬾᬦ᭄( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }ᬮᬶᬬ
  ᬬᬾᬦ᭄( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  ᬮᬶᬬ{
    rc = 0;
  }
  ᬯᬮᬶ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static puur_naa_nka kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  const var_na *zPath,
  puur_naa_nka flags,
  puur_naa_nka *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#ᬬᬾᬦ᭄ 0 && defined(SQLITE_WASM)
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
#ᬮᬶᬬ
  ᬬᬾᬦ᭄( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }ᬮᬶᬬ
  ᬬᬾᬦ᭄( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }ᬮᬶᬬ
  ᬬᬾᬦ᭄( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }ᬮᬶᬬ
  ᬬᬾᬦ᭄( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }ᬮᬶᬬ
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static puur_naa_nka kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  const var_na *zPath,
  puur_naa_nka nOut,
  var_na *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  ᬬᬾᬦ᭄( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *kvvfsDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  ᬯᬮᬶ 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
static puur_naa_nka kvvfsRandomness(sqlite3_vfs *pVfs, puur_naa_nka nByte, var_na *zBufOut){
  memset(zBufOut, 0, nByte);
  ᬯᬮᬶ nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
static puur_naa_nka kvvfsSleep(sqlite3_vfs *pVfs, puur_naa_nka nMicro){
  ᬯᬮᬶ SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static puur_naa_nka kvvfsCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  sqlite3_int64 i = 0;
  puur_naa_nka rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  ᬯᬮᬶ rc;
}
#include <sys/time.h>
static puur_naa_nka kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  static const sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  sa_nracanaa timeval sNow;
  (void)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  ᬯᬮᬶ SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#ᬬᬾᬦ᭄ SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
puur_naa_nka sqlite3_os_init(void){
  ᬯᬮᬶ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
puur_naa_nka sqlite3_os_end(void){
  ᬯᬮᬶ SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#ᬬᬾᬦ᭄ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
puur_naa_nka sqlite3KvvfsInit(void){
  ᬯᬮᬶ sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
