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
#якщо SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#якщо 0
#define SQLITE_KV_TRACE(X)  printf X
#інакше
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#якщо 0
#define SQLITE_KV_LOG(X)  printf X
#інакше
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
  ціле isJournal;                  /* True if this is a journal file */
  unsigned ціле nJrnl;             /* Space allocated for aJrnl[] */
  char *aJrnl;                    /* Journal content */
  ціле szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  char *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
static ціле kvvfsClose(sqlite3_file*);
static ціле kvvfsReadDb(sqlite3_file*, void*, ціле iAmt, sqlite3_int64 iOfst);
static ціле kvvfsReadJrnl(sqlite3_file*, void*, ціле iAmt, sqlite3_int64 iOfst);
static ціле kvvfsWriteDb(sqlite3_file*,const void*,ціле iAmt, sqlite3_int64);
static ціле kvvfsWriteJrnl(sqlite3_file*,const void*,ціле iAmt, sqlite3_int64);
static ціле kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
static ціле kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
static ціле kvvfsSyncDb(sqlite3_file*, ціле flags);
static ціле kvvfsSyncJrnl(sqlite3_file*, ціле flags);
static ціле kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
static ціле kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
static ціле kvvfsLock(sqlite3_file*, ціле);
static ціле kvvfsUnlock(sqlite3_file*, ціле);
static ціле kvvfsCheckReservedLock(sqlite3_file*, ціле *pResOut);
static ціле kvvfsFileControlDb(sqlite3_file*, ціле op, void *pArg);
static ціле kvvfsFileControlJrnl(sqlite3_file*, ціле op, void *pArg);
static ціле kvvfsSectorSize(sqlite3_file*);
static ціле kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
static ціле kvvfsOpen(sqlite3_vfs*, const char *, sqlite3_file*, ціле , ціле *);
static ціле kvvfsDelete(sqlite3_vfs*, const char *zName, ціле syncDir);
static ціле kvvfsAccess(sqlite3_vfs*, const char *zName, ціле flags, ціле *);
static ціле kvvfsFullPathname(sqlite3_vfs*, const char *zName, ціле, char *zOut);
static void *kvvfsDlOpen(sqlite3_vfs*, const char *zFilename);
static ціле kvvfsRandomness(sqlite3_vfs*, ціле nByte, char *zOut);
static ціле kvvfsSleep(sqlite3_vfs*, ціле microseconds);
static ціле kvvfsCurrentTime(sqlite3_vfs*, double*);
static ціле kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

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
static ціле kvrecordWrite(const char*, const char *zKey, const char *zData);
static ціле kvrecordDelete(const char*, const char *zKey);
static ціле kvrecordRead(const char*, const char *zKey, char *zBuf, ціле nBuf);
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
static ціле kvrecordWrite(
  const char *zClass,
  const char *zKey,
  const char *zData
){
  FILE *fd;
  char zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  fd = fopen(zXKey, "wb");
  якщо( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (ціле)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    повернути 0;
  }інакше{
    повернути 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
static ціле kvrecordDelete(const char *zClass, const char *zKey){
  char zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  повернути 0;
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
static ціле kvrecordRead(
  const char *zClass,
  const char *zKey,
  char *zBuf,
  ціле nBuf
){
  FILE *fd;
  struct stat buf;
  char zXKey[KVRECORD_KEY_SZ];
  kvrecordMakeKey(zClass, zKey, zXKey);
  якщо( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    повернути -1;
  }
  якщо( nBuf<=0 ){
    повернути (ціле)buf.st_size;
  }інакше якщо( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (ціле)buf.st_size));
    повернути (ціле)buf.st_size;
  }
  якщо( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  якщо( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    повернути -1;
  }інакше{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    повернути (ціле)n;
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
  ціле (*xRcrdRead)(const char*, const char *zKey, char *zBuf, ціле nBuf);
  ціле (*xRcrdWrite)(const char*, const char *zKey, const char *zData);
  ціле (*xRcrdDelete)(const char*, const char *zKey);
  const ціле nKeySize;
  const ціле nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST const
#інакше
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
#інакше
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
ціле kvvfsEncode(const char *aData, ціле nData, char *aOut){
  ціле i, j;
  const unsigned char *a = (const unsigned char*)aData;
  для(i=j=0; i<nData; i++){
    unsigned char c = a[i];
    якщо( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }інакше{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      ціле k;
      для(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      поки( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  повернути j;
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
ціле kvvfsDecode(const char *a, char *aOut, ціле nOut){
  ціле i, j;
  ціле c;
  const unsigned char *aIn = (const unsigned char*)a;
  i = 0;
  j = 0;
  поки( 1 ){
    c = kvvfsHexValue[aIn[i]];
    якщо( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      якщо( c==0 ) break;
      поки( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        якщо( n>nOut ) повернути -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      якщо( j+n>nOut ) повернути -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      якщо( c==0 || mult==1 ) break; /* progress stalled if mult==1 */
    }інакше якщо( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfsHexValue[aIn[++i]];
      якщо( c<0 ) повернути -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }інакше{
      повернути -1;
    }
  }
  повернути j;
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
  ціле nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned ціле n = 0, mult;
  ціле c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  поки( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  якщо( ' '!=zTxt[i++] ){
    /* Malformed input */
    повернути;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  якщо( pFile->aJrnl==0 ){
    повернути;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  якщо( n<pFile->nJrnl ){
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
  повернути strtoll(zData, 0, 0);
}
static ціле kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  char zData[50];
  sqlite3_snprintf(sizeof(zData), zData, "%lld", sz);
  повернути sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
static ціле kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, sizeof(*pFile));
  повернути SQLITE_OK;
}

/*
** Read from the -journal file.
*/
static ціле kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  void *zBuf,
  ціле iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  assert( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  якщо( pFile->aJrnl==0 ){
    ціле rc;
    ціле szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    char *aTxt;
    якщо( szTxt<=4 ){
      повернути SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    якщо( aTxt==0 ) повернути SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    якщо( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    якщо( rc ) повернути rc;
    якщо( pFile->aJrnl==0 ) повернути SQLITE_IOERR;
  }
  якщо( iOfst+iAmt>pFile->nJrnl ){
    повернути SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  повернути SQLITE_OK;
}

/*
** Read from the database file.
*/
static ціле kvvfsReadDb(
  sqlite3_file *pProtoFile,
  void *zBuf,
  ціле iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned ціле pgno;
  ціле got, n;
  char zKey[30];
  char *aData = pFile->aData;
  assert( iOfst>=0 );
  assert( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  якщо( iOfst+iAmt>=512 ){
    якщо( (iOfst % iAmt)!=0 ){
      повернути SQLITE_IOERR_READ;
    }
    якщо( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      повернути SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }інакше{
    pgno = 1;
  }
  sqlite3_snprintf(sizeof(zKey), zKey, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, zKey,
                                      aData, SQLITE_KVOS_SZ-1);
  якщо( got<0 ){
    n = 0;
  }інакше{
    aData[got] = 0;
    якщо( iOfst+iAmt<512 ){
      ціле k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      якщо( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }інакше{
        n = 0;
      }
    }інакше{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  якщо( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    повернути SQLITE_IOERR_SHORT_READ;
  }
  повернути SQLITE_OK;
}


/*
** Write into the -journal file.
*/
static ціле kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  const void *zBuf,
  ціле iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  якщо( iEnd>=0x10000000 ) повернути SQLITE_FULL;
  якщо( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    char *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    якщо( aNew==0 ){
      повернути SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    якщо( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  повернути SQLITE_OK;
}

/*
** Write into the database file.
*/
static ціле kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  const void *zBuf,
  ціле iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned ціле pgno;
  char zKey[30];
  char *aData = pFile->aData;
  ціле rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  assert( iAmt>=512 && iAmt<=65536 );
  assert( (iAmt & (iAmt-1))==0 );
  assert( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(sizeof(zKey), zKey, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, zKey, aData);
  якщо( 0==rc ){
    якщо( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  повернути rc;
}

/*
** Truncate an kvvfs-file.
*/
static ціле kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  assert( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  повернути SQLITE_OK;
}
static ціле kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  якщо( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    char zKey[50];
    unsigned ціле pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    поки( pgno<=pgnoMax ){
      sqlite3_snprintf(sizeof(zKey), zKey, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, zKey);
      pgno++;
    }
    pFile->szDb = size;
    повернути kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  повернути SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
static ціле kvvfsSyncJrnl(sqlite3_file *pProtoFile, ціле flags){
  ціле i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  char *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  якщо( pFile->nJrnl<=0 ){
    повернути kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  якщо( zOut==0 ){
    повернути SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  do{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }поки( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  повернути i ? SQLITE_IOERR : SQLITE_OK;
}
static ціле kvvfsSyncDb(sqlite3_file *pProtoFile, ціле flags){
  повернути SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
static ціле kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  повернути SQLITE_OK;
}
static ціле kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  якщо( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }інакше{
    *pSize = kvvfsReadFileSize(pFile);
  }
  повернути SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
static ціле kvvfsLock(sqlite3_file *pProtoFile, ціле eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  assert( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  якщо( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  повернути SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
static ціле kvvfsUnlock(sqlite3_file *pProtoFile, ціле eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  assert( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  якщо( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  повернути SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
static ціле kvvfsCheckReservedLock(sqlite3_file *pProtoFile, ціле *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  повернути SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
static ціле kvvfsFileControlJrnl(sqlite3_file *pProtoFile, ціле op, void *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  повернути SQLITE_NOTFOUND;
}
static ціле kvvfsFileControlDb(sqlite3_file *pProtoFile, ціле op, void *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  якщо( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    ціле rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    якщо( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    повернути rc;
  }
  повернути SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
static ціле kvvfsSectorSize(sqlite3_file *pFile){
  повернути 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
static ціле kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  повернути 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
static ціле kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  const char *zName,
  sqlite3_file *pProtoFile,
  ціле flags,
  ціле *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  якщо( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  assert(!pFile->zClass);
  assert(!pFile->aData);
  assert(!pFile->aJrnl);
  assert(!pFile->nJrnl);
  assert(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  якщо( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    якщо( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }інакше якщо( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }інакше{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  якщо( !pFile->zClass ){
#ifdef SQLITE_WASM
    якщо( strlen(zName) >= (KVRECORD_KEY_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      повернути SQLITE_CANTOPEN;
    }
#інакше
    якщо( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      повернути SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  якщо( pFile->aData==0 ){
    повернути SQLITE_NOMEM;
  }
  повернути SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static ціле kvvfsDelete(sqlite3_vfs *pVfs, const char *zPath, ціле dirSync){
  ціле rc /* The JS impl can fail with OOM in argument conversion */;
  якщо( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }інакше
  якщо( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  інакше{
    rc = 0;
  }
  повернути rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static ціле kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  const char *zPath,
  ціле flags,
  ціле *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#якщо 0 && defined(SQLITE_WASM)
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
#інакше
  якщо( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }інакше
  якщо( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }інакше
  якщо( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }інакше
  якщо( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }інакше
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  повернути SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static ціле kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  const char *zPath,
  ціле nOut,
  char *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  якщо( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  повернути SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *kvvfsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  повернути 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
static ціле kvvfsRandomness(sqlite3_vfs *pVfs, ціле nByte, char *zBufOut){
  memset(zBufOut, 0, nByte);
  повернути nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
static ціле kvvfsSleep(sqlite3_vfs *pVfs, ціле nMicro){
  повернути SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static ціле kvvfsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  sqlite3_int64 i = 0;
  ціле rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  повернути rc;
}
#include <sys/time.h>
static ціле kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  static const sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  struct timeval sNow;
  (void)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  повернути SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#якщо SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
ціле sqlite3_os_init(void){
  повернути sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
ціле sqlite3_os_end(void){
  повернути SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#якщо SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
ціле sqlite3KvvfsInit(void){
  повернути sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
