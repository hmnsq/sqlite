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
#अन्तर्भाव <sqliteInt.h>
#यदि SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#यदि 0
#define SQLITE_KV_TRACE(X)  म_लिखो X
#अन्यथा
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#यदि 0
#define SQLITE_KV_LOG(X)  म_लिखो X
#अन्यथा
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
प्रकारनिर्देश संरचना KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
संरचना KVVfsFile {
  sqlite3_file base;              /* IO methods */
  ध्रुव वर्ण *zClass;             /* Storage class */
  पूर्णाङ्क isJournal;                  /* True if this is a journal file */
  अचिन्हित पूर्णाङ्क nJrnl;             /* Space allocated for aJrnl[] */
  वर्ण *aJrnl;                    /* Journal content */
  पूर्णाङ्क szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  वर्ण *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
स्थिर पूर्णाङ्क kvvfsClose(sqlite3_file*);
स्थिर पूर्णाङ्क kvvfsReadDb(sqlite3_file*, शून्य*, पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क kvvfsReadJrnl(sqlite3_file*, शून्य*, पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क kvvfsWriteDb(sqlite3_file*,ध्रुव शून्य*,पूर्णाङ्क iAmt, sqlite3_int64);
स्थिर पूर्णाङ्क kvvfsWriteJrnl(sqlite3_file*,ध्रुव शून्य*,पूर्णाङ्क iAmt, sqlite3_int64);
स्थिर पूर्णाङ्क kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णाङ्क kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णाङ्क kvvfsSyncDb(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क kvvfsSyncJrnl(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णाङ्क kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णाङ्क kvvfsLock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क kvvfsUnlock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क kvvfsCheckReservedLock(sqlite3_file*, पूर्णाङ्क *pResOut);
स्थिर पूर्णाङ्क kvvfsFileControlDb(sqlite3_file*, पूर्णाङ्क op, शून्य *pArg);
स्थिर पूर्णाङ्क kvvfsFileControlJrnl(sqlite3_file*, पूर्णाङ्क op, शून्य *pArg);
स्थिर पूर्णाङ्क kvvfsSectorSize(sqlite3_file*);
स्थिर पूर्णाङ्क kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
स्थिर पूर्णाङ्क kvvfsOpen(sqlite3_vfs*, ध्रुव वर्ण *, sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क *);
स्थिर पूर्णाङ्क kvvfsDelete(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क syncDir);
स्थिर पूर्णाङ्क kvvfsAccess(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क flags, पूर्णाङ्क *);
स्थिर पूर्णाङ्क kvvfsFullPathname(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क, वर्ण *zOut);
स्थिर शून्य *kvvfsDlOpen(sqlite3_vfs*, ध्रुव वर्ण *zFilename);
स्थिर पूर्णाङ्क kvvfsRandomness(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *zOut);
स्थिर पूर्णाङ्क kvvfsSleep(sqlite3_vfs*, पूर्णाङ्क microseconds);
स्थिर पूर्णाङ्क kvvfsCurrentTime(sqlite3_vfs*, द्विगुण*);
स्थिर पूर्णाङ्क kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

स्थिर sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  परिमाण(KVVfsFile),              /* szOsFile */
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
स्थिर sqlite3_io_methods kvvfs_db_io_methods = {
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
स्थिर sqlite3_io_methods kvvfs_jrnl_io_methods = {
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
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#ifndef SQLITE_WASM
/* In WASM builds these are implemented in JS. */
स्थिर पूर्णाङ्क kvrecordWrite(ध्रुव वर्ण*, ध्रुव वर्ण *z_कुंजी, ध्रुव वर्ण *zData);
स्थिर पूर्णाङ्क kvrecordDelete(ध्रुव वर्ण*, ध्रुव वर्ण *z_कुंजी);
स्थिर पूर्णाङ्क kvrecordRead(ध्रुव वर्ण*, ध्रुव वर्ण *z_कुंजी, वर्ण *zBuf, पूर्णाङ्क nBuf);
#endif
#ifndef KVRECORD_कुंजी_SZ
#define KVRECORD_कुंजी_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
स्थिर शून्य kvrecord_Make_कुंजी(
  ध्रुव वर्ण *zClass,
  ध्रुव वर्ण *z_कुंजी_In,
  वर्ण *z_कुंजी_Out
){
  प्रतिज्ञा( z_कुंजी_In );
  प्रतिज्ञा( z_कुंजी_Out );
  प्रतिज्ञा( zClass );
  sqlite3_snprintf(KVRECORD_कुंजी_SZ, z_कुंजी_Out, "kvvfs-%s-%s",
                   zClass, z_कुंजी_In);
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
स्थिर पूर्णाङ्क kvrecordWrite(
  ध्रुव वर्ण *zClass,
  ध्रुव वर्ण *z_कुंजी,
  ध्रुव वर्ण *zData
){
  खाता *fd;
  वर्ण zXKey[KVRECORD_कुंजी_SZ];
  kvrecord_Make_कुंजी(zClass, z_कुंजी, zXKey);
  fd = ख_खोलो(zXKey, "wb");
  यदि( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (पूर्णाङ्क)म_माप(zData), zData,
                 म_माप(zData)>50 ? "..." : ""));
    ख_माला_दो(zData, fd);
    ख_बंद(fd);
    प्रत्यागम 0;
  }अन्यथा{
    प्रत्यागम 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
स्थिर पूर्णाङ्क kvrecordDelete(ध्रुव वर्ण *zClass, ध्रुव वर्ण *z_कुंजी){
  वर्ण zXKey[KVRECORD_कुंजी_SZ];
  kvrecord_Make_कुंजी(zClass, z_कुंजी, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  प्रत्यागम 0;
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
स्थिर पूर्णाङ्क kvrecordRead(
  ध्रुव वर्ण *zClass,
  ध्रुव वर्ण *z_कुंजी,
  वर्ण *zBuf,
  पूर्णाङ्क nBuf
){
  खाता *fd;
  संरचना stat buf;
  वर्ण zXKey[KVRECORD_कुंजी_SZ];
  kvrecord_Make_कुंजी(zClass, z_कुंजी, zXKey);
  यदि( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    प्रत्यागम -1;
  }
  यदि( nBuf<=0 ){
    प्रत्यागम (पूर्णाङ्क)buf.st_size;
  }अन्यथा यदि( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (पूर्णाङ्क)buf.st_size));
    प्रत्यागम (पूर्णाङ्क)buf.st_size;
  }
  यदि( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = ख_खोलो(zXKey, "rb");
  यदि( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    प्रत्यागम -1;
  }अन्यथा{
    sqlite3_int64 n = ख_पढ़ो(zBuf, 1, nBuf-1, fd);
    ख_बंद(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    प्रत्यागम (पूर्णाङ्क)n;
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
प्रकारनिर्देश संरचना sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
संरचना sqlite3_kvvfs_methods {
  पूर्णाङ्क (*xRcrdRead)(ध्रुव वर्ण*, ध्रुव वर्ण *z_कुंजी, वर्ण *zBuf, पूर्णाङ्क nBuf);
  पूर्णाङ्क (*xRcrdWrite)(ध्रुव वर्ण*, ध्रुव वर्ण *z_कुंजी, ध्रुव वर्ण *zData);
  पूर्णाङ्क (*xRcrdDelete)(ध्रुव वर्ण*, ध्रुव वर्ण *z_कुंजी);
  ध्रुव पूर्णाङ्क n_कुंजी_Size;
  ध्रुव पूर्णाङ्क nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST ध्रुव
#अन्यथा
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
ध्रुव
#endif
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#ifndef SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#अन्यथा
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#endif
  .n_कुंजी_Size        = KVRECORD_कुंजी_SZ,
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
स्थिर
#endif
पूर्णाङ्क kvvfsEncode(ध्रुव वर्ण *aData, पूर्णाङ्क nData, वर्ण *aOut){
  पूर्णाङ्क i, j;
  ध्रुव अचिन्हित वर्ण *a = (ध्रुव अचिन्हित वर्ण*)aData;
  प्रति(i=j=0; i<nData; i++){
    अचिन्हित वर्ण c = a[i];
    यदि( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }अन्यथा{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      पूर्णाङ्क k;
      प्रति(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      यावत्( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  प्रत्यागम j;
}

स्थिर ध्रुव चिन्हित वर्ण kvvfs_Hex_मूल्य[256] = {
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
स्थिर
#endif
पूर्णाङ्क kvvfsDecode(ध्रुव वर्ण *a, वर्ण *aOut, पूर्णाङ्क nOut){
  पूर्णाङ्क i, j;
  पूर्णाङ्क c;
  ध्रुव अचिन्हित वर्ण *aIn = (ध्रुव अचिन्हित वर्ण*)a;
  i = 0;
  j = 0;
  यावत्( 1 ){
    c = kvvfs_Hex_मूल्य[aIn[i]];
    यदि( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      यदि( c==0 ) भङ्ग;
      यावत्( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        यदि( n>nOut ) प्रत्यागम -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      यदि( j+n>nOut ) प्रत्यागम -1 /* oversized/malformed input */;
      स_रखो(&aOut[j], 0, n);
      j += n;
      यदि( c==0 || mult==1 ) भङ्ग; /* progress stalled if mult==1 */
    }अन्यथा यदि( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_मूल्य[aIn[++i]];
      यदि( c<0 ) प्रत्यागम -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }अन्यथा{
      प्रत्यागम -1;
    }
  }
  प्रत्यागम j;
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
स्थिर शून्य kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  ध्रुव वर्ण *zTxt,      /* Text encoding.  Zero-terminated */
  पूर्णाङ्क nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  अचिन्हित पूर्णाङ्क n = 0, mult;
  पूर्णाङ्क c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  यावत्( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  यदि( ' '!=zTxt[i++] ){
    /* Malformed input */
    प्रत्यागम;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  यदि( pFile->aJrnl==0 ){
    प्रत्यागम;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  यदि( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
स्थिर sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  वर्ण zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                परिमाण(zData)-1);
  प्रत्यागम strtoll(zData, 0, 0);
}
स्थिर पूर्णाङ्क kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  वर्ण zData[50];
  sqlite3_snprintf(परिमाण(zData), zData, "%lld", sz);
  प्रत्यागम sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  स_रखो(pFile, 0, परिमाण(*pFile));
  प्रत्यागम SQLITE_OK;
}

/*
** Read from the -journal file.
*/
स्थिर पूर्णाङ्क kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  शून्य *zBuf,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  प्रतिज्ञा( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  यदि( pFile->aJrnl==0 ){
    पूर्णाङ्क rc;
    पूर्णाङ्क szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    वर्ण *aTxt;
    यदि( szTxt<=4 ){
      प्रत्यागम SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    यदि( aTxt==0 ) प्रत्यागम SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    यदि( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    यदि( rc ) प्रत्यागम rc;
    यदि( pFile->aJrnl==0 ) प्रत्यागम SQLITE_IOERR;
  }
  यदि( iOfst+iAmt>pFile->nJrnl ){
    प्रत्यागम SQLITE_IOERR_SHORT_READ;
  }
  स_नकल(zBuf, pFile->aJrnl+iOfst, iAmt);
  प्रत्यागम SQLITE_OK;
}

/*
** Read from the database file.
*/
स्थिर पूर्णाङ्क kvvfsReadDb(
  sqlite3_file *pProtoFile,
  शून्य *zBuf,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  अचिन्हित पूर्णाङ्क pgno;
  पूर्णाङ्क got, n;
  वर्ण z_कुंजी[30];
  वर्ण *aData = pFile->aData;
  प्रतिज्ञा( iOfst>=0 );
  प्रतिज्ञा( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  यदि( iOfst+iAmt>=512 ){
    यदि( (iOfst % iAmt)!=0 ){
      प्रत्यागम SQLITE_IOERR_READ;
    }
    यदि( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      प्रत्यागम SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }अन्यथा{
    pgno = 1;
  }
  sqlite3_snprintf(परिमाण(z_कुंजी), z_कुंजी, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_कुंजी,
                                      aData, SQLITE_KVOS_SZ-1);
  यदि( got<0 ){
    n = 0;
  }अन्यथा{
    aData[got] = 0;
    यदि( iOfst+iAmt<512 ){
      पूर्णाङ्क k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      यदि( n>=iOfst+iAmt ){
        स_नकल(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }अन्यथा{
        n = 0;
      }
    }अन्यथा{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  यदि( n<iAmt ){
    स_रखो(zBuf+n, 0, iAmt-n);
    प्रत्यागम SQLITE_IOERR_SHORT_READ;
  }
  प्रत्यागम SQLITE_OK;
}


/*
** Write into the -journal file.
*/
स्थिर पूर्णाङ्क kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  ध्रुव शून्य *zBuf,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  यदि( iEnd>=0x10000000 ) प्रत्यागम SQLITE_FULL;
  यदि( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    वर्ण *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    यदि( aNew==0 ){
      प्रत्यागम SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    यदि( pFile->nJrnl<iOfst ){
      स_रखो(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  स_नकल(pFile->aJrnl+iOfst, zBuf, iAmt);
  प्रत्यागम SQLITE_OK;
}

/*
** Write into the database file.
*/
स्थिर पूर्णाङ्क kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  ध्रुव शून्य *zBuf,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  अचिन्हित पूर्णाङ्क pgno;
  वर्ण z_कुंजी[30];
  वर्ण *aData = pFile->aData;
  पूर्णाङ्क rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  प्रतिज्ञा( iAmt>=512 && iAmt<=65536 );
  प्रतिज्ञा( (iAmt & (iAmt-1))==0 );
  प्रतिज्ञा( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(परिमाण(z_कुंजी), z_कुंजी, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_कुंजी, aData);
  यदि( 0==rc ){
    यदि( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  प्रत्यागम rc;
}

/*
** Truncate an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  प्रतिज्ञा( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  प्रत्यागम SQLITE_OK;
}
स्थिर पूर्णाङ्क kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  यदि( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    वर्ण z_कुंजी[50];
    अचिन्हित पूर्णाङ्क pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    यावत्( pgno<=pgnoMax ){
      sqlite3_snprintf(परिमाण(z_कुंजी), z_कुंजी, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_कुंजी);
      pgno++;
    }
    pFile->szDb = size;
    प्रत्यागम kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  प्रत्यागम SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsSyncJrnl(sqlite3_file *pProtoFile, पूर्णाङ्क flags){
  पूर्णाङ्क i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  वर्ण *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  यदि( pFile->nJrnl<=0 ){
    प्रत्यागम kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  यदि( zOut==0 ){
    प्रत्यागम SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  कुरु{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }यावत्( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  प्रत्यागम i ? SQLITE_IOERR : SQLITE_OK;
}
स्थिर पूर्णाङ्क kvvfsSyncDb(sqlite3_file *pProtoFile, पूर्णाङ्क flags){
  प्रत्यागम SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  प्रत्यागम SQLITE_OK;
}
स्थिर पूर्णाङ्क kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  यदि( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }अन्यथा{
    *pSize = kvvfsReadFileSize(pFile);
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsLock(sqlite3_file *pProtoFile, पूर्णाङ्क eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  प्रतिज्ञा( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  यदि( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsUnlock(sqlite3_file *pProtoFile, पूर्णाङ्क eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  प्रतिज्ञा( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  यदि( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsCheckReservedLock(sqlite3_file *pProtoFile, पूर्णाङ्क *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  प्रत्यागम SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsFileControlJrnl(sqlite3_file *pProtoFile, पूर्णाङ्क op, शून्य *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  प्रत्यागम SQLITE_NOTFOUND;
}
स्थिर पूर्णाङ्क kvvfsFileControlDb(sqlite3_file *pProtoFile, पूर्णाङ्क op, शून्य *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  यदि( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    पूर्णाङ्क rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    यदि( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    प्रत्यागम rc;
  }
  प्रत्यागम SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsSectorSize(sqlite3_file *pFile){
  प्रत्यागम 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
स्थिर पूर्णाङ्क kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  प्रत्यागम 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
स्थिर पूर्णाङ्क kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  ध्रुव वर्ण *zName,
  sqlite3_file *pProtoFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  यदि( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  प्रतिज्ञा(!pFile->zClass);
  प्रतिज्ञा(!pFile->aData);
  प्रतिज्ञा(!pFile->aJrnl);
  प्रतिज्ञा(!pFile->nJrnl);
  प्रतिज्ञा(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  यदि( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    यदि( 0==म_भेद("session-journal",zName) ){
      pFile->zClass = "session";
    }अन्यथा यदि( 0==म_भेद("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }अन्यथा{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  यदि( !pFile->zClass ){
#ifdef SQLITE_WASM
    यदि( म_माप(zName) >= (KVRECORD_कुंजी_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      प्रत्यागम SQLITE_CANTOPEN;
    }
#अन्यथा
    यदि( 0!=म_भेद(zName, "local") && 0!=म_भेद(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      प्रत्यागम SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  यदि( pFile->aData==0 ){
    प्रत्यागम SQLITE_NOMEM;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णाङ्क kvvfsDelete(sqlite3_vfs *pVfs, ध्रुव वर्ण *zPath, पूर्णाङ्क dirSync){
  पूर्णाङ्क rc /* The JS impl can fail with OOM in argument conversion */;
  यदि( म_भेद(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }अन्यथा
  यदि( म_भेद(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  अन्यथा{
    rc = 0;
  }
  प्रत्यागम rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णाङ्क kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  ध्रुव वर्ण *zPath,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#यदि 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  ध्रुव वर्ण *z_कुंजी = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_कुंजी, 0, 0)>0;
#अन्यथा
  यदि( म_भेद(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }अन्यथा
  यदि( म_भेद(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }अन्यथा
  यदि( म_भेद(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }अन्यथा
  यदि( म_भेद(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }अन्यथा
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  प्रत्यागम SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णाङ्क kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  ध्रुव वर्ण *zPath,
  पूर्णाङ्क nOut,
  वर्ण *zOut
){
  माप_प्रकार nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = म_माप(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  यदि( nOut<nPath+1 ) nPath = nOut - 1;
  स_नकल(zOut, zPath, nPath);
  zOut[nPath] = 0;
  प्रत्यागम SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *kvvfsDlOpen(sqlite3_vfs *pVfs, ध्रुव वर्ण *zPath){
  प्रत्यागम 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
स्थिर पूर्णाङ्क kvvfsRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *zBufOut){
  स_रखो(zBufOut, 0, nByte);
  प्रत्यागम nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
स्थिर पूर्णाङ्क kvvfsSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  प्रत्यागम SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णाङ्क kvvfsCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  sqlite3_int64 i = 0;
  पूर्णाङ्क rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  प्रत्यागम rc;
}
#अन्तर्भाव <sys/time.h>
स्थिर पूर्णाङ्क kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  स्थिर ध्रुव sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  संरचना timeval sNow;
  (शून्य)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  प्रत्यागम SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#यदि SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
पूर्णाङ्क sqlite3_os_init(शून्य){
  प्रत्यागम sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
पूर्णाङ्क sqlite3_os_end(शून्य){
  प्रत्यागम SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#यदि SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
पूर्णाङ्क sqlite3KvvfsInit(शून्य){
  प्रत्यागम sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
