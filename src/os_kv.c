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
#जर SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#जर 0
#define SQLITE_KV_TRACE(X)  म_लिखो X
#अन्यथा
#define SQLITE_KV_TRACE(X)
#endif

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#जर 0
#define SQLITE_KV_LOG(X)  म_लिखो X
#अन्यथा
#define SQLITE_KV_LOG(X)
#endif

/*
** Forward declaration of objects used by this VFS implementation
*/
प्रकारव्याख्या संरचना KVVfsFile KVVfsFile;

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
  स्थिरांक अक्षर *zClass;             /* Storage class */
  पूर्णांक isJournal;                  /* True if this is a journal file */
  अचिन्हित पूर्णांक nJrnl;             /* Space allocated for aJrnl[] */
  अक्षर *aJrnl;                    /* Journal content */
  पूर्णांक szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  अक्षर *aData;                    /* Buffer to hold page data */
};
#define SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
स्थिर पूर्णांक kvvfsClose(sqlite3_file*);
स्थिर पूर्णांक kvvfsReadDb(sqlite3_file*, शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक kvvfsReadJrnl(sqlite3_file*, शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक kvvfsWriteDb(sqlite3_file*,स्थिरांक शून्य*,पूर्णांक iAmt, sqlite3_int64);
स्थिर पूर्णांक kvvfsWriteJrnl(sqlite3_file*,स्थिरांक शून्य*,पूर्णांक iAmt, sqlite3_int64);
स्थिर पूर्णांक kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णांक kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णांक kvvfsSyncDb(sqlite3_file*, पूर्णांक flags);
स्थिर पूर्णांक kvvfsSyncJrnl(sqlite3_file*, पूर्णांक flags);
स्थिर पूर्णांक kvvfsFileSizeDb(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णांक kvvfsFileSizeJrnl(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णांक kvvfsLock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक kvvfsUnlock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक kvvfsCheckReservedLock(sqlite3_file*, पूर्णांक *pResOut);
स्थिर पूर्णांक kvvfsFileControlDb(sqlite3_file*, पूर्णांक op, शून्य *pArg);
स्थिर पूर्णांक kvvfsFileControlJrnl(sqlite3_file*, पूर्णांक op, शून्य *pArg);
स्थिर पूर्णांक kvvfsSectorSize(sqlite3_file*);
स्थिर पूर्णांक kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
स्थिर पूर्णांक kvvfsOpen(sqlite3_vfs*, स्थिरांक अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
स्थिर पूर्णांक kvvfsDelete(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक syncDir);
स्थिर पूर्णांक kvvfsAccess(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक flags, पूर्णांक *);
स्थिर पूर्णांक kvvfsFullPathname(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक, अक्षर *zOut);
स्थिर शून्य *kvvfsDlOpen(sqlite3_vfs*, स्थिरांक अक्षर *zFilename);
स्थिर पूर्णांक kvvfsRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
स्थिर पूर्णांक kvvfsSleep(sqlite3_vfs*, पूर्णांक microseconds);
स्थिर पूर्णांक kvvfsCurrentTime(sqlite3_vfs*, द्विगुण*);
स्थिर पूर्णांक kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

स्थिर sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  आकार(KVVfsFile),              /* szOsFile */
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
स्थिर पूर्णांक kvrecordWrite(स्थिरांक अक्षर*, स्थिरांक अक्षर *z_किल्ली, स्थिरांक अक्षर *zData);
स्थिर पूर्णांक kvrecordDelete(स्थिरांक अक्षर*, स्थिरांक अक्षर *z_किल्ली);
स्थिर पूर्णांक kvrecordRead(स्थिरांक अक्षर*, स्थिरांक अक्षर *z_किल्ली, अक्षर *zBuf, पूर्णांक nBuf);
#endif
#ifndef KVRECORD_किल्ली_SZ
#define KVRECORD_किल्ली_SZ 32
#endif

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
स्थिर शून्य kvrecord_Make_किल्ली(
  स्थिरांक अक्षर *zClass,
  स्थिरांक अक्षर *z_किल्ली_In,
  अक्षर *z_किल्ली_Out
){
  प्रतिज्ञा( z_किल्ली_In );
  प्रतिज्ञा( z_किल्ली_Out );
  प्रतिज्ञा( zClass );
  sqlite3_snprintf(KVRECORD_किल्ली_SZ, z_किल्ली_Out, "kvvfs-%s-%s",
                   zClass, z_किल्ली_In);
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
स्थिर पूर्णांक kvrecordWrite(
  स्थिरांक अक्षर *zClass,
  स्थिरांक अक्षर *z_किल्ली,
  स्थिरांक अक्षर *zData
){
  खाता *fd;
  अक्षर zXKey[KVRECORD_किल्ली_SZ];
  kvrecord_Make_किल्ली(zClass, z_किल्ली, zXKey);
  fd = ख_खोलो(zXKey, "wb");
  जर( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (पूर्णांक)म_माप(zData), zData,
                 म_माप(zData)>50 ? "..." : ""));
    ख_माला_दो(zData, fd);
    ख_बंद(fd);
    परत 0;
  }अन्यथा{
    परत 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
स्थिर पूर्णांक kvrecordDelete(स्थिरांक अक्षर *zClass, स्थिरांक अक्षर *z_किल्ली){
  अक्षर zXKey[KVRECORD_किल्ली_SZ];
  kvrecord_Make_किल्ली(zClass, z_किल्ली, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  परत 0;
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
स्थिर पूर्णांक kvrecordRead(
  स्थिरांक अक्षर *zClass,
  स्थिरांक अक्षर *z_किल्ली,
  अक्षर *zBuf,
  पूर्णांक nBuf
){
  खाता *fd;
  संरचना stat buf;
  अक्षर zXKey[KVRECORD_किल्ली_SZ];
  kvrecord_Make_किल्ली(zClass, z_किल्ली, zXKey);
  जर( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    परत -1;
  }
  जर( nBuf<=0 ){
    परत (पूर्णांक)buf.st_size;
  }अन्यथा जर( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (पूर्णांक)buf.st_size));
    परत (पूर्णांक)buf.st_size;
  }
  जर( nBuf > buf.st_size + 1 ){
    nBuf = buf.st_size + 1;
  }
  fd = ख_खोलो(zXKey, "rb");
  जर( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    परत -1;
  }अन्यथा{
    sqlite3_int64 n = ख_पढ़ो(zBuf, 1, nBuf-1, fd);
    ख_बंद(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    परत (पूर्णांक)n;
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
प्रकारव्याख्या संरचना sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
संरचना sqlite3_kvvfs_methods {
  पूर्णांक (*xRcrdRead)(स्थिरांक अक्षर*, स्थिरांक अक्षर *z_किल्ली, अक्षर *zBuf, पूर्णांक nBuf);
  पूर्णांक (*xRcrdWrite)(स्थिरांक अक्षर*, स्थिरांक अक्षर *z_किल्ली, स्थिरांक अक्षर *zData);
  पूर्णांक (*xRcrdDelete)(स्थिरांक अक्षर*, स्थिरांक अक्षर *z_किल्ली);
  स्थिरांक पूर्णांक n_किल्ली_Size;
  स्थिरांक पूर्णांक nBufferSize;
#ifndef SQLITE_WASM
#  define MAYBE_CONST स्थिरांक
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
स्थिरांक
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
  .n_किल्ली_Size        = KVRECORD_किल्ली_SZ,
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
पूर्णांक kvvfsEncode(स्थिरांक अक्षर *aData, पूर्णांक nData, अक्षर *aOut){
  पूर्णांक i, j;
  स्थिरांक अचिन्हित अक्षर *a = (स्थिरांक अचिन्हित अक्षर*)aData;
  साठी(i=j=0; i<nData; i++){
    अचिन्हित अक्षर c = a[i];
    जर( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }अन्यथा{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      पूर्णांक k;
      साठी(k=1; i+k<nData && a[i+k]==0; k++){}
      i += k-1;
      पर्यंत( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  परत j;
}

स्थिर स्थिरांक चिन्हित अक्षर kvvfs_Hex_मूल्य[256] = {
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
पूर्णांक kvvfsDecode(स्थिरांक अक्षर *a, अक्षर *aOut, पूर्णांक nOut){
  पूर्णांक i, j;
  पूर्णांक c;
  स्थिरांक अचिन्हित अक्षर *aIn = (स्थिरांक अचिन्हित अक्षर*)a;
  i = 0;
  j = 0;
  पर्यंत( 1 ){
    c = kvvfs_Hex_मूल्य[aIn[i]];
    जर( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      जर( c==0 ) थांबा;
      पर्यंत( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        जर( n>nOut ) परत -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      जर( j+n>nOut ) परत -1 /* oversized/malformed input */;
      स_रखो(&aOut[j], 0, n);
      j += n;
      जर( c==0 || mult==1 ) थांबा; /* progress stalled if mult==1 */
    }अन्यथा जर( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_मूल्य[aIn[++i]];
      जर( c<0 ) परत -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }अन्यथा{
      परत -1;
    }
  }
  परत j;
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
  स्थिरांक अक्षर *zTxt,      /* Text encoding.  Zero-terminated */
  पूर्णांक nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  अचिन्हित पूर्णांक n = 0, mult;
  पूर्णांक c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  पर्यंत( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  जर( ' '!=zTxt[i++] ){
    /* Malformed input */
    परत;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  जर( pFile->aJrnl==0 ){
    परत;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  जर( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
स्थिर sqlite3_int64 kvvfsReadFileSize(KVVfsFile *pFile){
  अक्षर zData[50];
  zData[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", zData,
                                आकार(zData)-1);
  परत strtoll(zData, 0, 0);
}
स्थिर पूर्णांक kvvfsWriteFileSize(KVVfsFile *pFile, sqlite3_int64 sz){
  अक्षर zData[50];
  sqlite3_snprintf(आकार(zData), zData, "%lld", sz);
  परत sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", zData);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->aData);
  स_रखो(pFile, 0, आकार(*pFile));
  परत SQLITE_OK;
}

/*
** Read from the -journal file.
*/
स्थिर पूर्णांक kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  शून्य *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  प्रतिज्ञा( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  जर( pFile->aJrnl==0 ){
    पूर्णांक rc;
    पूर्णांक szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    अक्षर *aTxt;
    जर( szTxt<=4 ){
      परत SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    जर( aTxt==0 ) परत SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    जर( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    जर( rc ) परत rc;
    जर( pFile->aJrnl==0 ) परत SQLITE_IOERR;
  }
  जर( iOfst+iAmt>pFile->nJrnl ){
    परत SQLITE_IOERR_SHORT_READ;
  }
  स_नकल(zBuf, pFile->aJrnl+iOfst, iAmt);
  परत SQLITE_OK;
}

/*
** Read from the database file.
*/
स्थिर पूर्णांक kvvfsReadDb(
  sqlite3_file *pProtoFile,
  शून्य *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  अचिन्हित पूर्णांक pgno;
  पूर्णांक got, n;
  अक्षर z_किल्ली[30];
  अक्षर *aData = pFile->aData;
  प्रतिज्ञा( iOfst>=0 );
  प्रतिज्ञा( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  जर( iOfst+iAmt>=512 ){
    जर( (iOfst % iAmt)!=0 ){
      परत SQLITE_IOERR_READ;
    }
    जर( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      परत SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }अन्यथा{
    pgno = 1;
  }
  sqlite3_snprintf(आकार(z_किल्ली), z_किल्ली, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_किल्ली,
                                      aData, SQLITE_KVOS_SZ-1);
  जर( got<0 ){
    n = 0;
  }अन्यथा{
    aData[got] = 0;
    जर( iOfst+iAmt<512 ){
      पूर्णांक k = iOfst+iAmt;
      aData[k*2] = 0;
      n = kvvfsDecode(aData, &aData[2000], SQLITE_KVOS_SZ-2000);
      जर( n>=iOfst+iAmt ){
        स_नकल(zBuf, &aData[2000+iOfst], iAmt);
        n = iAmt;
      }अन्यथा{
        n = 0;
      }
    }अन्यथा{
      n = kvvfsDecode(aData, zBuf, iAmt);
    }
  }
  जर( n<iAmt ){
    स_रखो(zBuf+n, 0, iAmt-n);
    परत SQLITE_IOERR_SHORT_READ;
  }
  परत SQLITE_OK;
}


/*
** Write into the -journal file.
*/
स्थिर पूर्णांक kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  स्थिरांक शून्य *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  जर( iEnd>=0x10000000 ) परत SQLITE_FULL;
  जर( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    अक्षर *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    जर( aNew==0 ){
      परत SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    जर( pFile->nJrnl<iOfst ){
      स_रखो(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  स_नकल(pFile->aJrnl+iOfst, zBuf, iAmt);
  परत SQLITE_OK;
}

/*
** Write into the database file.
*/
स्थिर पूर्णांक kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  स्थिरांक शून्य *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  अचिन्हित पूर्णांक pgno;
  अक्षर z_किल्ली[30];
  अक्षर *aData = pFile->aData;
  पूर्णांक rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  प्रतिज्ञा( iAmt>=512 && iAmt<=65536 );
  प्रतिज्ञा( (iAmt & (iAmt-1))==0 );
  प्रतिज्ञा( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(आकार(z_किल्ली), z_किल्ली, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, aData);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_किल्ली, aData);
  जर( 0==rc ){
    जर( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  परत rc;
}

/*
** Truncate an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, size));
  प्रतिज्ञा( size==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  परत SQLITE_OK;
}
स्थिर पूर्णांक kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 size){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  जर( pFile->szDb>size
   && pFile->szPage>0
   && (size % pFile->szPage)==0
  ){
    अक्षर z_किल्ली[50];
    अचिन्हित पूर्णांक pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, size));
    pgno = 1 + size/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    पर्यंत( pgno<=pgnoMax ){
      sqlite3_snprintf(आकार(z_किल्ली), z_किल्ली, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_किल्ली);
      pgno++;
    }
    pFile->szDb = size;
    परत kvvfsWriteFileSize(pFile, size) ? SQLITE_IOERR : SQLITE_OK;
  }
  परत SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsSyncJrnl(sqlite3_file *pProtoFile, पूर्णांक flags){
  पूर्णांक i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  अक्षर *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  जर( pFile->nJrnl<=0 ){
    परत kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  जर( zOut==0 ){
    परत SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  करा{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }पर्यंत( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  परत i ? SQLITE_IOERR : SQLITE_OK;
}
स्थिर पूर्णांक kvvfsSyncDb(sqlite3_file *pProtoFile, पूर्णांक flags){
  परत SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsFileSizeJrnl(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *pSize = pFile->nJrnl;
  परत SQLITE_OK;
}
स्थिर पूर्णांक kvvfsFileSizeDb(sqlite3_file *pProtoFile, sqlite_int64 *pSize){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  जर( pFile->szDb>=0 ){
    *pSize = pFile->szDb;
  }अन्यथा{
    *pSize = kvvfsReadFileSize(pFile);
  }
  परत SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsLock(sqlite3_file *pProtoFile, पूर्णांक eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  प्रतिज्ञा( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  जर( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfsReadFileSize(pFile);
  }
  परत SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsUnlock(sqlite3_file *pProtoFile, पूर्णांक eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  प्रतिज्ञा( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  जर( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  परत SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsCheckReservedLock(sqlite3_file *pProtoFile, पूर्णांक *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  परत SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsFileControlJrnl(sqlite3_file *pProtoFile, पूर्णांक op, शून्य *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  परत SQLITE_NOTFOUND;
}
स्थिर पूर्णांक kvvfsFileControlDb(sqlite3_file *pProtoFile, पूर्णांक op, शून्य *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  जर( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    पूर्णांक rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    जर( pFile->szDb>0 && 0!=kvvfsWriteFileSize(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    परत rc;
  }
  परत SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsSectorSize(sqlite3_file *pFile){
  परत 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
स्थिर पूर्णांक kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  परत 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
स्थिर पूर्णांक kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  स्थिरांक अक्षर *zName,
  sqlite3_file *pProtoFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  जर( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  प्रतिज्ञा(!pFile->zClass);
  प्रतिज्ञा(!pFile->aData);
  प्रतिज्ञा(!pFile->aJrnl);
  प्रतिज्ञा(!pFile->nJrnl);
  प्रतिज्ञा(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  जर( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    जर( 0==म_भेद("session-journal",zName) ){
      pFile->zClass = "session";
    }अन्यथा जर( 0==म_भेद("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }अन्यथा{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  जर( !pFile->zClass ){
#ifdef SQLITE_WASM
    जर( म_माप(zName) >= (KVRECORD_किल्ली_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      परत SQLITE_CANTOPEN;
    }
#अन्यथा
    जर( 0!=म_भेद(zName, "local") && 0!=म_भेद(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      परत SQLITE_CANTOPEN;
    }
#endif
    pFile->zClass = zName;
  }
  pFile->aData = sqlite3_malloc64(SQLITE_KVOS_SZ);
  जर( pFile->aData==0 ){
    परत SQLITE_NOMEM;
  }
  परत SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णांक kvvfsDelete(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक rc /* The JS impl can fail with OOM in argument conversion */;
  जर( म_भेद(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }अन्यथा
  जर( म_भेद(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  अन्यथा{
    rc = 0;
  }
  परत rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णांक kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  स्थिरांक अक्षर *zPath,
  पूर्णांक flags,
  पूर्णांक *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#जर 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  स्थिरांक अक्षर *z_किल्ली = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_किल्ली, 0, 0)>0;
#अन्यथा
  जर( म_भेद(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }अन्यथा
  जर( म_भेद(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }अन्यथा
  जर( म_भेद(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }अन्यथा
  जर( म_भेद(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }अन्यथा
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#endif
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  परत SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णांक kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  स्थिरांक अक्षर *zPath,
  पूर्णांक nOut,
  अक्षर *zOut
){
  माप_प्रकार nPath;
#ifdef SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#endif
  nPath = म_माप(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  जर( nOut<nPath+1 ) nPath = nOut - 1;
  स_नकल(zOut, zPath, nPath);
  zOut[nPath] = 0;
  परत SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *kvvfsDlOpen(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath){
  परत 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
स्थिर पूर्णांक kvvfsRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  स_रखो(zBufOut, 0, nByte);
  परत nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
स्थिर पूर्णांक kvvfsSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  परत SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णांक kvvfsCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  sqlite3_int64 i = 0;
  पूर्णांक rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  परत rc;
}
#अन्तर्भाव <sys/time.h>
स्थिर पूर्णांक kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  स्थिर स्थिरांक sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  संरचना timeval sNow;
  (शून्य)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  परत SQLITE_OK;
}
#endif /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#जर SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
पूर्णांक sqlite3_os_init(शून्य){
  परत sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
पूर्णांक sqlite3_os_end(शून्य){
  परत SQLITE_OK;
}
#endif /* SQLITE_OS_KV */

#जर SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
पूर्णांक sqlite3KvvfsInit(शून्य){
  परत sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#endif
