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
#અન્તર્ભાવ <sqliteInt.h>
#જો SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#જો 0
#define SQLITE_KV_TRACE(X)  printf X
#નહીંતર
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#જો 0
#define SQLITE_KV_LOG(X)  printf X
#નહીંતર
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
પ્રકારવ્યાખ્યા સંરચના KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
સંરચના KVVfsFile {
  sqlite3_file base;              /* IO methods */
  અચલ અક્ષર *zClass;             /* Storage class */
  પૂર્ણાંક isJournal;                  /* True if this is a journal file */
  unsigned પૂર્ણાંક nJrnl;             /* Space allocated for aJrnl[] */
  અક્ષર *aJrnl;                    /* Journal content */
  પૂર્ણાંક szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  અક્ષર *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
સ્થિર પૂર્ણાંક kvvfsClose(sqlite3_file*);
સ્થિર પૂર્ણાંક kvvfsReadDb(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક kvvfsReadJrnl(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક kvvfsWriteDb(sqlite3_file*,અચલ શૂન્ય*,પૂર્ણાંક iAmt, sqlite3_int64);
સ્થિર પૂર્ણાંક kvvfsWriteJrnl(sqlite3_file*,અચલ શૂન્ય*,પૂર્ણાંક iAmt, sqlite3_int64);
સ્થિર પૂર્ણાંક kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક kvvfsSyncDb(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક kvvfsSyncJrnl(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક kvvfsLock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક kvvfsUnlock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક kvvfsCheckReservedLock(sqlite3_file*, પૂર્ણાંક *pResOut);
સ્થિર પૂર્ણાંક kvvfsFileControlDb(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
સ્થિર પૂર્ણાંક kvvfsFileControlJrnl(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
સ્થિર પૂર્ણાંક kvvfsSectorSize(sqlite3_file*);
સ્થિર પૂર્ણાંક kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
સ્થિર પૂર્ણાંક kvvfsOpen(sqlite3_vfs*, અચલ અક્ષર *, sqlite3_file*, પૂર્ણાંક , પૂર્ણાંક *);
સ્થિર પૂર્ણાંક kvvfsDelete(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક syncDir);
સ્થિર પૂર્ણાંક kvvfsAccess(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક flags, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક kvvfsFullPathname(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક, અક્ષર *zOut);
સ્થિર શૂન્ય *kvvfsDlOpen(sqlite3_vfs*, અચલ અક્ષર *zFilename);
સ્થિર પૂર્ણાંક kvvfsRandomness(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *zOut);
સ્થિર પૂર્ણાંક kvvfsSleep(sqlite3_vfs*, પૂર્ણાંક microseconds);
સ્થિર પૂર્ણાંક kvvfsCurrentTime(sqlite3_vfs*, દ્વિગુણ*);
સ્થિર પૂર્ણાંક kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

સ્થિર sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  કદ(KVVfsFile),              /* szOsFile */
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
સ્થિર sqlite3_io_methods kvvfs_db_io_methods = {
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
સ્થિર sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
સ્થિર પૂર્ણાંક kvrecordWrite(અચલ અક્ષર*, અચલ અક્ષર *z_ચાવી, અચલ અક્ષર *zData);
સ્થિર પૂર્ણાંક kvrecordDelete(અચલ અક્ષર*, અચલ અક્ષર *z_ચાવી);
સ્થિર પૂર્ણાંક kvrecordRead(અચલ અક્ષર*, અચલ અક્ષર *z_ચાવી, અક્ષર *zBuf, પૂર્ણાંક nBuf);
#endif
#ifndef KVRECORD_ચાવી_SZ
#define KVRECORD_ચાવી_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
સ્થિર શૂન્ય kvrecord_Make_ચાવી(
  અચલ અક્ષર *zClass,
  અચલ અક્ષર *z_ચાવી_In,
  અક્ષર *z_ચાવી_Out
){
  પ્રતિજ્ઞા( z_ચાવી_In );
  પ્રતિજ્ઞા( z_ચાવી_Out );
  પ્રતિજ્ઞા( zClass );
  sqlite3_snprintf(KVRECORD_ચાવી_SZ, z_ચાવી_Out, "kvvfs-%s-%s",
                   zClass, z_ચાવી_In);
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
સ્થિર પૂર્ણાંક kvrecordWrite(
  અચલ અક્ષર *zClass,
  અચલ અક્ષર *z_ચાવી,
  અચલ અક્ષર *zData
){
  FILE *fd;
  અક્ષર zXKey[KVRECORD_ચાવી_SZ];
  kvrecord_Make_ચાવી(zClass, z_ચાવી, zXKey);
  fd = fopen(zXKey, "wb");
  જો( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (પૂર્ણાંક)strlen(zData), zData,
                 strlen(zData)>50 ? "..." : ""));
    fputs(zData, fd);
    fclose(fd);
    પાછા 0;
  }નહીંતર{
    પાછા 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
સ્થિર પૂર્ણાંક kvrecordDelete(અચલ અક્ષર *zClass, અચલ અક્ષર *z_ચાવી){
  અક્ષર zXKey[KVRECORD_ચાવી_SZ];
  kvrecord_Make_ચાવી(zClass, z_ચાવી, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  પાછા 0;
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
સ્થિર પૂર્ણાંક kvrecordRead(
  અચલ અક્ષર *zClass,
  અચલ અક્ષર *z_ચાવી,
  અક્ષર *zBuf,
  પૂર્ણાંક nBuf
){
  FILE *fd;
  સંરચના stat buf;
  અક્ષર zXKey[KVRECORD_ચાવી_SZ];
  kvrecord_Make_ચાવી(zClass, z_ચાવી, zXKey);
  જો( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    પાછા -1;
  }
  જો( nBuf<=0 ){
    પાછા (પૂર્ણાંક)buf.st_size;
  }નહીંતર જો( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (પૂર્ણાંક)buf.st_size));
    પાછા (પૂર્ણાંક)buf.st_size;
  }
  જો( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = fopen(zXKey, "rb");
  જો( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    પાછા -1;
  }નહીંતર{
    sqlite3_int64 n = fread(zBuf, 1, nBuf-1, fd);
    fclose(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    પાછા (પૂર્ણાંક)n;
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
પ્રકારવ્યાખ્યા સંરચના sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
સંરચના sqlite3_kvvfs_methods {
  પૂર્ણાંક (*xRcrdRead)(અચલ અક્ષર*, અચલ અક્ષર *z_ચાવી, અક્ષર *zBuf, પૂર્ણાંક nBuf);
  પૂર્ણાંક (*xRcrdWrite)(અચલ અક્ષર*, અચલ અક્ષર *z_ચાવી, અચલ અક્ષર *zData);
  પૂર્ણાંક (*xRcrdDelete)(અચલ અક્ષર*, અચલ અક્ષર *z_ચાવી);
  અચલ પૂર્ણાંક n_ચાવી_Size;
  અચલ પૂર્ણાંક nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST અચલ
#નહીંતર
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
અચલ
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#નહીંતર
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .n_ચાવી_Size        = KVRECORD_ચાવી_SZ,
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
સ્થિર
#endif
પૂર્ણાંક kvvfsEncode(અચલ અક્ષર *aData, પૂર્ણાંક nData, અક્ષર *aOut){
  પૂર્ણાંક i, j;
  અચલ unsigned અક્ષર *a = (અચલ unsigned અક્ષર*)aData;
  માટે(i=j=0; i<nData; i++){
    unsigned અક્ષર c = a[i];
    જો( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }નહીંતર{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      પૂર્ણાંક k;
      માટે(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      જ્યારે( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  પાછા j;
}

સ્થિર અચલ signed અક્ષર kvvfs_Hex_કિંમત[256] = {
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
સ્થિર
#endif
પૂર્ણાંક kvvfsDecode(અચલ અક્ષર *a, અક્ષર *aOut, પૂર્ણાંક nOut){
  પૂર્ણાંક i, j;
  પૂર્ણાંક c;
  અચલ unsigned અક્ષર *aIn = (અચલ unsigned અક્ષર*)a;
  i = 0;
  j = 0;
  જ્યારે( 1 ){
    c = kvvfs_Hex_કિંમત[aIn[i]];
    જો( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      જો( c==0 ) તોડો;
      જ્યારે( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        જો( n>nOut ) પાછા -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      જો( j+n>nOut ) પાછા -1 /* oversized/malformed input */;
      memset(&aOut[j], 0, n);
      j += n;
      જો( c==0 || mult==1 ) તોડો; /* progress stalled if mult==1 */
    }નહીંતર જો( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_કિંમત[aIn[++i]];
      જો( c<0 ) પાછા -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }નહીંતર{
      પાછા -1;
    }
  }
  પાછા j;
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
સ્થિર શૂન્ય kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  અચલ અક્ષર *zTxt,      /* Text encoding.  Zero-terminated */
  પૂર્ણાંક nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  unsigned પૂર્ણાંક n = 0, mult;
  પૂર્ણાંક c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  જ્યારે( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  જો( ' '!=zTxt[i++] ){
    /* Malformed input */
    પાછા;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  જો( pFile->aJrnl==0 ){
    પાછા;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  જો( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
સ્થિર sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  અક્ષર zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                કદ(zData)-1);
  પાછા strtoll(zData, 0, 0);
}
સ્થિર પૂર્ણાંક kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  અક્ષર zData[50];
  sqlite3_snprintf(કદ(zData), zData, "%lld", sz);
  પાછા sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  memset(pFile, 0, કદ(*pFile));
  પાછા SQLITE_OK;
}

/*
** Read from the -journal file.
*/
સ્થિર પૂર્ણાંક kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  શૂન્ય *zBuf,
  પૂર્ણાંક iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  પ્રતિજ્ઞા( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  જો( pFile->aJrnl==0 ){
    પૂર્ણાંક rc;
    પૂર્ણાંક szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    અક્ષર *aTxt;
    જો( szTxt<=4 ){
      પાછા SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    જો( aTxt==0 ) પાછા SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    જો( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    જો( rc ) પાછા rc;
    જો( pFile->aJrnl==0 ) પાછા SQLITE_IOERR;
  }
  જો( iOfst+iAmt>pFile->nJrnl ){
    પાછા SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, pFile->aJrnl+iOfst, iAmt);
  પાછા SQLITE_OK;
}

/*
** Read from the database file.
*/
સ્થિર પૂર્ણાંક kvvfsReadDb(
  sqlite3_file *pProtoFile,
  શૂન્ય *zBuf,
  પૂર્ણાંક iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned પૂર્ણાંક pgno;
  પૂર્ણાંક got, n;
  અક્ષર z_ચાવી[30];
  અક્ષર *aData = pFile->aData;
  પ્રતિજ્ઞા( iOfst>=0 );
  પ્રતિજ્ઞા( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  જો( iOfst+iAmt>=512 ){
    જો( (iOfst % iAmt)!=0 ){
      પાછા SQLITE_IOERR_READ;
    }
    જો( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      પાછા SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }નહીંતર{
    pgno = 1;
  }
  sqlite3_snprintf(કદ(z_ચાવી), z_ચાવી, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_ચાવી,
                                      aData, SQLITE_KVOS_SZ-1);
  જો( got<0 ){
    n = 0;
  }નહીંતર{
    aData[got] = 0;
    જો( iOfst+iAmt<512 ){
      પૂર્ણાંક k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      જો( n>=iOfst+iAmt ){
        memcpy(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }નહીંતર{
        n = 0;
      }
    }નહીંતર{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  જો( n<iAmt ){
    memset(zBuf+n, 0, iAmt-n);
    પાછા SQLITE_IOERR_SHORT_READ;
  }
  પાછા SQLITE_OK;
}


/*
** Write into the -journal file.
*/
સ્થિર પૂર્ણાંક kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  અચલ શૂન્ય *zBuf,
  પૂર્ણાંક iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  જો( iEnd>=0x10000000 ) પાછા SQLITE_FULL;
  જો( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    અક્ષર *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    જો( aNew==0 ){
      પાછા SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    જો( pFile->nJrnl<iOfst ){
      memset(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  memcpy(pFile->aJrnl+iOfst, zBuf, iAmt);
  પાછા SQLITE_OK;
}

/*
** Write into the database file.
*/
સ્થિર પૂર્ણાંક kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  અચલ શૂન્ય *zBuf,
  પૂર્ણાંક iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  unsigned પૂર્ણાંક pgno;
  અક્ષર z_ચાવી[30];
  અક્ષર *aData = pFile->aData;
  પૂર્ણાંક rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  પ્રતિજ્ઞા( iAmt>=512 && iAmt<=65536 );
  પ્રતિજ્ઞા( (iAmt & (iAmt-1))==0 );
  પ્રતિજ્ઞા( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(કદ(z_ચાવી), z_ચાવી, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_ચાવી, aData);
  જો( 0==rc ){
    જો( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  પાછા rc;
}

/*
** Truncate an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  પ્રતિજ્ઞા( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  પાછા SQLITE_OK;
}
સ્થિર પૂર્ણાંક kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  જો( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    અક્ષર z_ચાવી[50];
    unsigned પૂર્ણાંક pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    જ્યારે( pgno<=pgnoMax ){
      sqlite3_snprintf(કદ(z_ચાવી), z_ચાવી, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_ચાવી);
      pgno++;
    }
    pFile->szDb = size;
    પાછા kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  પાછા SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsSyncJrnl(sqlite3_file *pProtoFile, પૂર્ણાંક flags){
  પૂર્ણાંક i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  અક્ષર *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  જો( pFile->nJrnl<=0 ){
    પાછા kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  જો( zOut==0 ){
    પાછા SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  કરો{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }જ્યારે( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  પાછા i ? SQLITE_IOERR : SQLITE_OK;
}
સ્થિર પૂર્ણાંક kvvfsSyncDb(sqlite3_file *pProtoFile, પૂર્ણાંક flags){
  પાછા SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  પાછા SQLITE_OK;
}
સ્થિર પૂર્ણાંક kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  જો( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }નહીંતર{
    *pSize = kvvfsReadFileSize(pFile);
  }
  પાછા SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsLock(sqlite3_file *pProtoFile, પૂર્ણાંક eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  પ્રતિજ્ઞા( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  જો( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  પાછા SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsUnlock(sqlite3_file *pProtoFile, પૂર્ણાંક eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  પ્રતિજ્ઞા( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  જો( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  પાછા SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsCheckReservedLock(sqlite3_file *pProtoFile, પૂર્ણાંક *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  પાછા SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsFileControlJrnl(sqlite3_file *pProtoFile, પૂર્ણાંક op, શૂન્ય *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  પાછા SQLITE_NOTFOUND;
}
સ્થિર પૂર્ણાંક kvvfsFileControlDb(sqlite3_file *pProtoFile, પૂર્ણાંક op, શૂન્ય *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  જો( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    પૂર્ણાંક rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    જો( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    પાછા rc;
  }
  પાછા SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsSectorSize(sqlite3_file *pFile){
  પાછા 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
સ્થિર પૂર્ણાંક kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  પાછા 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
સ્થિર પૂર્ણાંક kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pProtoFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  જો( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  પ્રતિજ્ઞા(!pFile->zClass);
  પ્રતિજ્ઞા(!pFile->aData);
  પ્રતિજ્ઞા(!pFile->aJrnl);
  પ્રતિજ્ઞા(!pFile->nJrnl);
  પ્રતિજ્ઞા(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  જો( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    જો( 0==strcmp("session-journal",zName) ){
      pFile->zClass = "session";
    }નહીંતર જો( 0==strcmp("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }નહીંતર{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  જો( !pFile->zClass ){
#ifdef SQLITE_WASM
    જો( strlen(zName) >= (KVRECORD_ચાવી_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      પાછા SQLITE_CANTOPEN;
    }
#નહીંતર
    જો( 0!=strcmp(zName, "local") && 0!=strcmp(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      પાછા SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  જો( pFile->aData==0 ){
    પાછા SQLITE_NOMEM;
  }
  પાછા SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
સ્થિર પૂર્ણાંક kvvfsDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  પૂર્ણાંક rc /* The JS impl can fail with OOM in argument conversion */;
  જો( strcmp(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }નહીંતર
  જો( strcmp(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  નહીંતર{
    rc = 0;
  }
  પાછા rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
સ્થિર પૂર્ણાંક kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  અચલ અક્ષર *zPath,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#જો 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  અચલ અક્ષર *z_ચાવી = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_ચાવી, 0, 0)>0;
#નહીંતર
  જો( strcmp(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }નહીંતર
  જો( strcmp(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }નહીંતર
  જો( strcmp(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }નહીંતર
  જો( strcmp(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }નહીંતર
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  પાછા SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
સ્થિર પૂર્ણાંક kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zPath,
  પૂર્ણાંક nOut,
  અક્ષર *zOut
){
  size_t nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = strlen(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  જો( nOut<nPath+1 ) nPath = nOut - 1;
  memcpy(zOut, zPath, nPath);
  zOut[nPath] = 0;
  પાછા SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
સ્થિર શૂન્ય *kvvfsDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  પાછા 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
સ્થિર પૂર્ણાંક kvvfsRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  memset(zBufOut, 0, nByte);
  પાછા nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
સ્થિર પૂર્ણાંક kvvfsSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  પાછા SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
સ્થિર પૂર્ણાંક kvvfsCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *pTimeOut){
  sqlite3_int64 i = 0;
  પૂર્ણાંક rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  પાછા rc;
}
#અન્તર્ભાવ <sys/time.h>
સ્થિર પૂર્ણાંક kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  સ્થિર અચલ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  સંરચના timeval sNow;
  (શૂન્ય)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  પાછા SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#જો SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
પૂર્ણાંક sqlite3_os_init(શૂન્ય){
  પાછા sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
પૂર્ણાંક sqlite3_os_end(શૂન્ય){
  પાછા SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#જો SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
પૂર્ણાંક sqlite3KvvfsInit(શૂન્ય){
  પાછા sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
