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
#如果 SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#如果 0
#define SQLITE_KV_TRACE(X)  printf X
#否则
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#如果 0
#define SQLITE_KV_LOG(X)  printf X
#否则
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
typedef struct KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
struct KVVfsFile {
  sqlite3_file base;              /* IO methods */
  const char *zClass;             /* Storage class */
  整数 isJournal;                  /* True if this is a journal file */
  unsigned 整数 nJrnl;             /* Space allocated for aJrnl[] */
  char *aJrnl;                    /* Journal content */
  整数 szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  char *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
static 整数 kvvfsClose(sqlite3_file*);
static 整数 kvvfsReadDb(sqlite3_file*, void*, 整数 iAmt, sqlite3_int64 iOfst);
static 整数 kvvfsReadJrnl(sqlite3_file*, void*, 整数 iAmt, sqlite3_int64 iOfst);
static 整数 kvvfsWriteDb(sqlite3_file*,const void*,整数 iAmt, sqlite3_int64);
static 整数 kvvfsWriteJrnl(sqlite3_file*,const void*,整数 iAmt, sqlite3_int64);
static 整数 kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
static 整数 kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
static 整数 kvvfsSyncDb(sqlite3_file*, 整数 flags);
static 整数 kvvfsSyncJrnl(sqlite3_file*, 整数 flags);
static 整数 kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
static 整数 kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
static 整数 kvvfsLock(sqlite3_file*, 整数);
static 整数 kvvfsUnlock(sqlite3_file*, 整数);
static 整数 kvvfsCheckReservedLock(sqlite3_file*, 整数 *pResOut);
static 整数 kvvfsFileControlDb(sqlite3_file*, 整数 op, void *pArg);
static 整数 kvvfsFileControlJrnl(sqlite3_file*, 整数 op, void *pArg);
static 整数 kvvfsSectorSize(sqlite3_file*);
static 整数 kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
static 整数 kvvfsOpen(sqlite3_vfs*, const char *, sqlite3_file*, 整数 , 整数 *);
static 整数 kvvfsDelete(sqlite3_vfs*, const char *zName, 整数 syncDir);
static 整数 kvvfsAccess(sqlite3_vfs*, const char *zName, 整数 flags, 整数 *);
static 整数 kvvfsFullPathname(sqlite3_vfs*, const char *zName, 整数, char *zOut);
static void *kvvfsDlOpen(sqlite3_vfs*, const char *zFilename);
static 整数 kvvfsRandomness(sqlite3_vfs*, 整数 nByte, char *zOut);
static 整数 kvvfsSleep(sqlite3_vfs*, 整数 microseconds);
static 整数 kvvfsCurrentTime(sqlite3_vfs*, double*);
static 整数 kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

static sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  sizeof(KVVfsFile),              /* szOsFile */
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
static 整数 kvrecordWrite(const char*, const char *zKey, const char *zData);
static 整数 kvrecordDelete(const char*, const char *zKey);
static 整数 kvrecordRead(const char*, const char *zKey, char *zBuf, 整数 nBuf);
#endif
#ifndef KVRECORD_KEY_SZ
#define KVRECORD_KEY_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
static void kvrecordMakeKey(
  const char *zClass,
  const char *zKeyIn,
  char *zKeyOut
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
static 整数 kvrecordWrite(
  const char *zClass,
  const char *zKey,
  const char *zData
){
  FILE *fd;
  char zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  fd = fopen(zXKey, "wb");
  如果( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (整数)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    返回 0;
  }否则{
    返回 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
static 整数 kvrecordDelete(const char *zClass, const char *zKey){
  char zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  返回 0;
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
static 整数 kvrecordRead(
  const char *zClass,
  const char *zKey,
  char *zBuf,
  整数 nBuf
){
  FILE *fd;
  struct stat buf;
  char zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  如果( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    返回 -1;
  }
  如果( nBuf<=0 ){
    返回 (整数)buf.st_size;
  }否则 如果( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (整数)buf.st_size));
    返回 (整数)buf.st_size;
  }
  如果( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  如果( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    返回 -1;
  }否则{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    返回 (整数)n;
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
typedef struct sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
struct sqlite3_kvvfs_methods {
  整数 (*xRcrdRead)(const char*, const char *zKey, char *zBuf, 整数 nBuf);
  整数 (*xRcrdWrite)(const char*, const char *zKey, const char *zData);
  整数 (*xRcrdDelete)(const char*, const char *zKey);
  const 整数 nKeySize;
  const 整数 nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST const
#否则
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
#否则
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
整数 kvvfsEncode(const char *aData, 整数 nData, char *aOut){
  整数 i, j;
  const unsigned char *a = (const unsigned char*)aData;
  为(i=j=0; i<nData; i++){
    unsigned char c = a[i];
    如果( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }否则{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      整数 k;
      为(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      当( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  返回 j;
}

static const signed char kvvfsHexValue[256] = {
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
整数 kvvfsDecode(const char *a, char *aOut, 整数 nOut){
  整数 i, j;
  整数 c;
  const unsigned char *aIn = (const unsigned char*)a;
  i = 0;
  j = 0;
  当( 1 ){
    c = kvvfsHexValue[aIn[i]];
    如果( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      如果( c==0 ) break;
      当( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        如果( n>nOut ) 返回 -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      如果( j+n>nOut ) 返回 -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      如果( c==0 || mult==1 ) break; /* progress stalled if mult==1 */
    }否则 如果( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfsHexValue[aIn[++i]];
      如果( c<0 ) 返回 -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }否则{
      返回 -1;
    }
  }
  返回 j;
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
  const char *zTxt,      /* Text encoding.  Zero-terminated */
  整数 nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned 整数 n = 0, mult;
  整数 c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  当( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  如果( ' '!=zTxt[i++] ){
    /* Malformed input */
    返回;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  如果( pFile->aJrnl==0 ){
    返回;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  如果( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
static sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  char zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                sizeof(zData)-1);
  返回 strtoll(zData, 0, 0);
}
static 整数 kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  char zData[50];
  sqlite3_snprintf(sizeof(zData), zData, "%lld", sz);
  返回 sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
static 整数 kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, sizeof(*pFile));
  返回 SQLITE_OK;
}

/*
** Read from the -journal file.
*/
static 整数 kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  void *zBuf,
  整数 iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  assert( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  如果( pFile->aJrnl==0 ){
    整数 rc;
    整数 szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    char *aTxt;
    如果( szTxt<=4 ){
      返回 SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    如果( aTxt==0 ) 返回 SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    如果( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    如果( rc ) 返回 rc;
    如果( pFile->aJrnl==0 ) 返回 SQLITE_IOERR;
  }
  如果( iOfst+iAmt>pFile->nJrnl ){
    返回 SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  返回 SQLITE_OK;
}

/*
** Read from the database file.
*/
static 整数 kvvfsReadDb(
  sqlite3_file *pProtoFile,
  void *zBuf,
  整数 iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned 整数 pgno;
  整数 got, n;
  char zKey[30];
  char *aData = pFile->aData;
  assert( iOfst>=0 );
  assert( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  如果( iOfst+iAmt>=512 ){
    如果( (iOfst % iAmt)!=0 ){
      返回 SQLITE_IOERR_READ;
    }
    如果( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      返回 SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }否则{
    pgno = 1;
  }
  sqlite3_snprintf(sizeof(zKey), zKey, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, zKey,
                                      aData, SQLITE_KVOS_SZ-1);
  如果( got<0 ){
    n = 0;
  }否则{
    aData[got] = 0;
    如果( iOfst+iAmt<512 ){
      整数 k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      如果( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }否则{
        n = 0;
      }
    }否则{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  如果( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    返回 SQLITE_IOERR_SHORT_READ;
  }
  返回 SQLITE_OK;
}


/*
** Write into the -journal file.
*/
static 整数 kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  const void *zBuf,
  整数 iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  如果( iEnd>=0x10000000 ) 返回 SQLITE_FULL;
  如果( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    char *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    如果( aNew==0 ){
      返回 SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    如果( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  返回 SQLITE_OK;
}

/*
** Write into the database file.
*/
static 整数 kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  const void *zBuf,
  整数 iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned 整数 pgno;
  char zKey[30];
  char *aData = pFile->aData;
  整数 rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  assert( iAmt>=512 && iAmt<=65536 );
  assert( (iAmt & (iAmt-1))==0 );
  assert( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(sizeof(zKey), zKey, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, zKey, aData);
  如果( 0==rc ){
    如果( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  返回 rc;
}

/*
** Truncate an kvvfs-file.
*/
static 整数 kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  assert( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  返回 SQLITE_OK;
}
static 整数 kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  如果( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    char zKey[50];
    unsigned 整数 pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    当( pgno<=pgnoMax ){
      sqlite3_snprintf(sizeof(zKey), zKey, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, zKey);
      pgno++;
    }
    pFile->szDb = size;
    返回 kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  返回 SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
static 整数 kvvfsSyncJrnl(sqlite3_file *pProtoFile, 整数 flags){
  整数 i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  char *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  如果( pFile->nJrnl<=0 ){
    返回 kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  如果( zOut==0 ){
    返回 SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  do{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }当( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  返回 i ? SQLITE_IOERR : SQLITE_OK;
}
static 整数 kvvfsSyncDb(sqlite3_file *pProtoFile, 整数 flags){
  返回 SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
static 整数 kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  返回 SQLITE_OK;
}
static 整数 kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  如果( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }否则{
    *pSize = kvvfsReadFileSize(pFile);
  }
  返回 SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
static 整数 kvvfsLock(sqlite3_file *pProtoFile, 整数 eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  assert( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  如果( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  返回 SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
static 整数 kvvfsUnlock(sqlite3_file *pProtoFile, 整数 eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  assert( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  如果( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  返回 SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
static 整数 kvvfsCheckReservedLock(sqlite3_file *pProtoFile, 整数 *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  返回 SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
static 整数 kvvfsFileControlJrnl(sqlite3_file *pProtoFile, 整数 op, void *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  返回 SQLITE_NOTFOUND;
}
static 整数 kvvfsFileControlDb(sqlite3_file *pProtoFile, 整数 op, void *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  如果( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    整数 rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    如果( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    返回 rc;
  }
  返回 SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
static 整数 kvvfsSectorSize(sqlite3_file *pFile){
  返回 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
static 整数 kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  返回 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
static 整数 kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  const char *zName,
  sqlite3_file *pProtoFile,
  整数 flags,
  整数 *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  如果( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  assert(!pFile->zClass);
  assert(!pFile->aData);
  assert(!pFile->aJrnl);
  assert(!pFile->nJrnl);
  assert(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  如果( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    如果( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }否则 如果( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }否则{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  如果( !pFile->zClass ){
#ifdef SQLITE_WASM
    如果( strlen(zName) >= (KVRECORD_KEY_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      返回 SQLITE_CANTOPEN;
    }
#否则
    如果( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      返回 SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  如果( pFile->aData==0 ){
    返回 SQLITE_NOMEM;
  }
  返回 SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static 整数 kvvfsDelete(sqlite3_vfs *pVfs, const char *zPath, 整数 dirSync){
  整数 rc /* The JS impl can fail with OOM in argument conversion */;
  如果( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }否则
  如果( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  否则{
    rc = 0;
  }
  返回 rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static 整数 kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  const char *zPath,
  整数 flags,
  整数 *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#如果 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  const char *zKey = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, zKey, 0, 0)>0;
#否则
  如果( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }否则
  如果( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }否则
  如果( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }否则
  如果( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }否则
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  返回 SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static 整数 kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  const char *zPath,
  整数 nOut,
  char *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  如果( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  返回 SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *kvvfsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  返回 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
static 整数 kvvfsRandomness(sqlite3_vfs *pVfs, 整数 nByte, char *zBufOut){
  memset(zBufOut, 0, nByte);
  返回 nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
static 整数 kvvfsSleep(sqlite3_vfs *pVfs, 整数 nMicro){
  返回 SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static 整数 kvvfsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_int64 i = 0;
  整数 rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  返回 rc;
}
#include <sys/time.h>
static 整数 kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  static const sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  struct timeval sNow;
  (void)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  返回 SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#如果 SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
整数 sqlite3_os_init(void){
  返回 sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
整数 sqlite3_os_end(void){
  返回 SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#如果 SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
整数 sqlite3KvvfsInit(void){
  返回 sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
