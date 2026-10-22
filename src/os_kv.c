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
#समावेश <sqliteInt.h>
#अगर SQLITE_OS_KV || (SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL))

/*****************************************************************************
** Debugging logic
*/

/* SQLITE_KV_TRACE() is used for tracing calls to kvrecord routines. */
#अगर 0
#घोषणा SQLITE_KV_TRACE(X)  म_लिखो X
#अन्यथा
#घोषणा SQLITE_KV_TRACE(X)
#पूर्ण_अगर

/* SQLITE_KV_LOG() is used for tracing calls to the VFS interface */
#अगर 0
#घोषणा SQLITE_KV_LOG(X)  म_लिखो X
#अन्यथा
#घोषणा SQLITE_KV_LOG(X)
#पूर्ण_अगर

/*
** Forward declaration of objects used by this VFS implementation
*/
प्रकार काष्ठा KVVfsFile KVVfsFile;

/* A single open file.  There are only two files represented by this
** VFS - the database and the rollback journal.
**
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
काष्ठा KVVfsFile {
  sqlite3_file base;              /* IO methods */
  स्थिर अक्षर *zClass;             /* Storage class */
  पूर्णांक isJournal;                  /* True if this is a journal file */
  अचिन्हित पूर्णांक nJrnl;             /* Space allocated for aJrnl[] */
  अक्षर *aJrnl;                    /* Journal content */
  पूर्णांक szPage;                     /* Last known page size */
  sqlite3_int64 szDb;             /* Database file size.  -1 means unknown */
  अक्षर *a_तथ्य;                    /* Buffer to hold page data */
};
#घोषणा SQLITE_KVOS_SZ 133073

/*
** Methods for KVVfsFile
*/
अटल पूर्णांक kvvfsClose(sqlite3_file*);
अटल पूर्णांक kvvfsReadDb(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक kvvfsReadJrnl(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक kvvfsWriteDb(sqlite3_file*,स्थिर व्योम*,पूर्णांक iAmt, sqlite3_int64);
अटल पूर्णांक kvvfsWriteJrnl(sqlite3_file*,स्थिर व्योम*,पूर्णांक iAmt, sqlite3_int64);
अटल पूर्णांक kvvfsTruncateDb(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक kvvfsTruncateJrnl(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक kvvfsSyncDb(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक kvvfsSyncJrnl(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक kvvfs_File_आकार_Db(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक kvvfs_File_आकार_Jrnl(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक kvvfsLock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक kvvfsUnlock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक kvvfsCheckReservedLock(sqlite3_file*, पूर्णांक *pResOut);
अटल पूर्णांक kvvfsFileControlDb(sqlite3_file*, पूर्णांक op, व्योम *pArg);
अटल पूर्णांक kvvfsFileControlJrnl(sqlite3_file*, पूर्णांक op, व्योम *pArg);
अटल पूर्णांक kvvfs_Sector_आकार(sqlite3_file*);
अटल पूर्णांक kvvfsDeviceCharacteristics(sqlite3_file*);

/*
** Methods for sqlite3_vfs
*/
अटल पूर्णांक kvvfsOpen(sqlite3_vfs*, स्थिर अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
अटल पूर्णांक kvvfsDelete(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक syncDir);
अटल पूर्णांक kvvfsAccess(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक flags, पूर्णांक *);
अटल पूर्णांक kvvfsFullPathname(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक, अक्षर *zOut);
अटल व्योम *kvvfsDlOpen(sqlite3_vfs*, स्थिर अक्षर *zFilename);
अटल पूर्णांक kvvfsRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
अटल पूर्णांक kvvfsSleep(sqlite3_vfs*, पूर्णांक microseconds);
अटल पूर्णांक kvvfsCurrentTime(sqlite3_vfs*, द्विगुन*);
अटल पूर्णांक kvvfsCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

अटल sqlite3_vfs sqlite3OsKvvfsObject = {
  2,                              /* iVersion */
  माप(KVVfsFile),              /* szOsFile */
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
अटल sqlite3_io_methods kvvfs_db_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadDb,                    /* xRead */
  kvvfsWriteDb,                   /* xWrite */
  kvvfsTruncateDb,                /* xTruncate */
  kvvfsSyncDb,                    /* xSync */
  kvvfs_File_आकार_Db,                /* xFileSize */
  kvvfsLock,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfsCheckReservedLock,         /* xCheckReservedLock */
  kvvfsFileControlDb,             /* xFileControl */
  kvvfs_Sector_आकार,                /* xSectorSize */
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
अटल sqlite3_io_methods kvvfs_jrnl_io_methods = {
  1,                              /* iVersion */
  kvvfsClose,                     /* xClose */
  kvvfsReadJrnl,                  /* xRead */
  kvvfsWriteJrnl,                 /* xWrite */
  kvvfsTruncateJrnl,              /* xTruncate */
  kvvfsSyncJrnl,                  /* xSync */
  kvvfs_File_आकार_Jrnl,              /* xFileSize */
  kvvfsLock,                      /* xLock */
  kvvfsUnlock,                    /* xUnlock */
  kvvfsCheckReservedLock,         /* xCheckReservedLock */
  kvvfsFileControlJrnl,           /* xFileControl */
  kvvfs_Sector_आकार,                /* xSectorSize */
  kvvfsDeviceCharacteristics,     /* xDeviceCharacteristics */
  0,                              /* xShmMap */
  0,                              /* xShmLock */
  0,                              /* xShmBarrier */
  0,                              /* xShmUnmap */
  0,                              /* xFetch */
  0                               /* xUnfetch */
};

/****** Storage subsystem **************************************************/
#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <unistd.h>

/* Forward declarations for the low-level storage engine
*/
#अगर_अघोषित SQLITE_WASM
/* In WASM builds these are implemented in JS. */
अटल पूर्णांक kvrecordWrite(स्थिर अक्षर*, स्थिर अक्षर *z_कुंजी, स्थिर अक्षर *z_तथ्य);
अटल पूर्णांक kvrecordDelete(स्थिर अक्षर*, स्थिर अक्षर *z_कुंजी);
अटल पूर्णांक kvrecordRead(स्थिर अक्षर*, स्थिर अक्षर *z_कुंजी, अक्षर *zBuf, पूर्णांक nBuf);
#पूर्ण_अगर
#अगर_अघोषित KVRECORD_कुंजी_SZ
#घोषणा KVRECORD_कुंजी_SZ 32
#पूर्ण_अगर

/* Expand the key name with an appropriate prefix and put the result
** in zKeyOut[].  The zKeyOut[] buffer is assumed to hold at least
** KVRECORD_KEY_SZ bytes.
*/
अटल व्योम kvrecord_Make_कुंजी(
  स्थिर अक्षर *zClass,
  स्थिर अक्षर *z_कुंजी_In,
  अक्षर *z_कुंजी_Out
){
  निश्चित( z_कुंजी_In );
  निश्चित( z_कुंजी_Out );
  निश्चित( zClass );
  sqlite3_snprintf(KVRECORD_कुंजी_SZ, z_कुंजी_Out, "kvvfs-%s-%s",
                   zClass, z_कुंजी_In);
}

#अगर_अघोषित SQLITE_WASM
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
अटल पूर्णांक kvrecordWrite(
  स्थिर अक्षर *zClass,
  स्थिर अक्षर *z_कुंजी,
  स्थिर अक्षर *z_तथ्य
){
  खाता *fd;
  अक्षर zXKey[KVRECORD_कुंजी_SZ];
  kvrecord_Make_कुंजी(zClass, z_कुंजी, zXKey);
  fd = ख_खोलो(zXKey, "wb");
  अगर( fd ){
    SQLITE_KV_TRACE(("KVVFS-WRITE  %-15s (%d) %.50s%s\n", zXKey,
                 (पूर्णांक)म_माप(z_तथ्य), z_तथ्य,
                 म_माप(z_तथ्य)>50 ? "..." : ""));
    ख_माला_दो(z_तथ्य, fd);
    ख_बंद(fd);
    वापस 0;
  }अन्यथा{
    वापस 1;
  }
}

/* Delete a key (with its corresponding data) from the key/value
** namespace given by zClass.  If the key does not previously exist,
** this routine is a no-op.
*/
अटल पूर्णांक kvrecordDelete(स्थिर अक्षर *zClass, स्थिर अक्षर *z_कुंजी){
  अक्षर zXKey[KVRECORD_कुंजी_SZ];
  kvrecord_Make_कुंजी(zClass, z_कुंजी, zXKey);
  unlink(zXKey);
  SQLITE_KV_TRACE(("KVVFS-DELETE %-15s\n", zXKey));
  वापस 0;
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
अटल पूर्णांक kvrecordRead(
  स्थिर अक्षर *zClass,
  स्थिर अक्षर *z_कुंजी,
  अक्षर *zBuf,
  पूर्णांक nBuf
){
  खाता *fd;
  काष्ठा stat buf;
  अक्षर zXKey[KVRECORD_कुंजी_SZ];
  kvrecord_Make_कुंजी(zClass, z_कुंजी, zXKey);
  अगर( access(zXKey, R_OK)!=0
   || stat(zXKey, &buf)!=0
   || !S_ISREG(buf.st_mode)
  ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    वापस -1;
  }
  अगर( nBuf<=0 ){
    वापस (पूर्णांक)buf.st_आकार;
  }अन्यथा अगर( nBuf==1 ){
    zBuf[0] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%d)\n", zXKey,
                 (पूर्णांक)buf.st_आकार));
    वापस (पूर्णांक)buf.st_आकार;
  }
  अगर( nBuf > buf.st_आकार + 1 ){
    nBuf = buf.st_आकार + 1;
  }
  fd = ख_खोलो(zXKey, "rb");
  अगर( fd==0 ){
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (-1)\n", zXKey));
    वापस -1;
  }अन्यथा{
    sqlite3_int64 n = ख_पढ़ो(zBuf, 1, nBuf-1, fd);
    ख_बंद(fd);
    zBuf[n] = 0;
    SQLITE_KV_TRACE(("KVVFS-READ   %-15s (%lld) %.50s%s\n", zXKey,
                 n, zBuf, n>50 ? "..." : ""));
    वापस (पूर्णांक)n;
  }
}
#पूर्ण_अगर /* #ifndef SQLITE_WASM */


/*
** An internal level of indirection which enables us to replace the
** kvvfs i/o methods with JavaScript implementations in WASM builds.
** Maintenance reminder: if this struct changes in any way, the JSON
** rendering of its structure must be updated in
** sqlite3-wasm.c:sqlite3__wasm_enum_json(). There are no binary
** compatibility concerns, so it does not need an iVersion member.
*/
प्रकार काष्ठा sqlite3_kvvfs_methods sqlite3_kvvfs_methods;
काष्ठा sqlite3_kvvfs_methods {
  पूर्णांक (*xRcrdRead)(स्थिर अक्षर*, स्थिर अक्षर *z_कुंजी, अक्षर *zBuf, पूर्णांक nBuf);
  पूर्णांक (*xRcrdWrite)(स्थिर अक्षर*, स्थिर अक्षर *z_कुंजी, स्थिर अक्षर *z_तथ्य);
  पूर्णांक (*xRcrdDelete)(स्थिर अक्षर*, स्थिर अक्षर *z_कुंजी);
  स्थिर पूर्णांक n_कुंजी_आकार;
  स्थिर पूर्णांक n_बफर_आकार;
#अगर_अघोषित SQLITE_WASM
#  define MAYBE_CONST स्थिर
#अन्यथा
#  define MAYBE_CONST
#पूर्ण_अगर
  MAYBE_CONST sqlite3_vfs * pVfs;
  MAYBE_CONST sqlite3_io_methods *pIoDb;
  MAYBE_CONST sqlite3_io_methods *pIoJrnl;
#अघोषित MAYBE_CONST
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
#अगर_अघोषित SQLITE_WASM
स्थिर
#पूर्ण_अगर
sqlite3_kvvfs_methods sqlite3KvvfsMethods = {
#अगर_अघोषित SQLITE_WASM
  .xRcrdRead       = kvrecordRead,
  .xRcrdWrite      = kvrecordWrite,
  .xRcrdDelete     = kvrecordDelete,
#अन्यथा
  .xRcrdRead       = 0,
  .xRcrdWrite      = 0,
  .xRcrdDelete     = 0,
#पूर्ण_अगर
  .n_कुंजी_आकार        = KVRECORD_कुंजी_SZ,
  .n_बफर_आकार     = SQLITE_KVOS_SZ,
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
#अगर_अघोषित SQLITE_WASM
अटल
#पूर्ण_अगर
पूर्णांक kvvfsEncode(स्थिर अक्षर *a_तथ्य, पूर्णांक n_तथ्य, अक्षर *aOut){
  पूर्णांक i, j;
  स्थिर अचिन्हित अक्षर *a = (स्थिर अचिन्हित अक्षर*)a_तथ्य;
  क्रम(i=j=0; i<n_तथ्य; i++){
    अचिन्हित अक्षर c = a[i];
    अगर( c!=0 ){
      aOut[j++] = "0123456789ABCDEF"[c>>4];
      aOut[j++] = "0123456789ABCDEF"[c&0xf];
    }अन्यथा{
      /* A sequence of 1 or more zeros is stored as a little-endian
      ** base-26 number using a..z as the digits. So one zero is "b".
      ** Two zeros is "c". 25 zeros is "z", 26 zeros is "ab", 27 is "bb",
      ** and so forth.
      */
      पूर्णांक k;
      क्रम(k=1; i+k<n_तथ्य && a[i+k]==0; k++){}
      i += k-1;
      जबतक( k>0 ){
        aOut[j++] = 'a'+(k%26);
        k /= 26;
      }
    }
  }
  aOut[j] = 0;
  वापस j;
}

अटल स्थिर चिन्हित अक्षर kvvfs_Hex_मूल्य[256] = {
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
#अगर_अघोषित SQLITE_WASM
अटल
#पूर्ण_अगर
पूर्णांक kvvfsDecode(स्थिर अक्षर *a, अक्षर *aOut, पूर्णांक nOut){
  पूर्णांक i, j;
  पूर्णांक c;
  स्थिर अचिन्हित अक्षर *aIn = (स्थिर अचिन्हित अक्षर*)a;
  i = 0;
  j = 0;
  जबतक( 1 ){
    c = kvvfs_Hex_मूल्य[aIn[i]];
    अगर( c<0 ){
      sqlite3_int64 n = 0;
      sqlite3_int64 mult = 1;
      c = aIn[i];
      अगर( c==0 ) अवरोध;
      जबतक( c>='a' && c<='z' ){
        n += (c - 'a')*mult;
        अगर( n>nOut ) वापस -1 /* oversized/malformed input */;
        mult *= 26;
        c = aIn[++i];
      }
      अगर( j+n>nOut ) वापस -1 /* oversized/malformed input */;
      स_रखो(&aOut[j], 0, n);
      j += n;
      अगर( c==0 || mult==1 ) अवरोध; /* progress stalled if mult==1 */
    }अन्यथा अगर( j<nOut ){
      aOut[j] = c<<4;
      c = kvvfs_Hex_मूल्य[aIn[++i]];
      अगर( c<0 ) वापस -1 /* hex bytes are always in pairs */;
      aOut[j++] += c;
      i++;
    }अन्यथा{
      वापस -1;
    }
  }
  वापस j;
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
अटल व्योम kvvfsDecodeJournal(
  KVVfsFile *pFile,      /* Store decoding in pFile->aJrnl */
  स्थिर अक्षर *zTxt,      /* Text encoding.  Zero-terminated */
  पूर्णांक nTxt               /* Bytes in zTxt, excluding zero terminator */
){
  अचिन्हित पूर्णांक n = 0, mult;
  पूर्णांक c, i;
  i = 0;
  mult = 1;
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  जबतक( (c = zTxt[i])>='a' && c<='z' ){
    n += (c - 'a')*mult;
    mult *= 26;
    ++i;
  }
  अगर( ' '!=zTxt[i++] ){
    /* Malformed input */
    वापस;
  }
  pFile->aJrnl = sqlite3_malloc64( n );
  अगर( pFile->aJrnl==0 ){
    वापस;
  }
  pFile->nJrnl = n;
  n = kvvfsDecode(zTxt+i, pFile->aJrnl, pFile->nJrnl);
  अगर( n<pFile->nJrnl ){
    sqlite3_free(pFile->aJrnl);
    pFile->aJrnl = 0;
    pFile->nJrnl = 0;
  }
}

/*
** Read or write the "sz" element, containing the database file size.
*/
अटल sqlite3_int64 kvvfs_Read_File_आकार(KVVfsFile *pFile){
  अक्षर z_तथ्य[50];
  z_तथ्य[0] = 0;
  sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "sz", z_तथ्य,
                                माप(z_तथ्य)-1);
  वापस strtoll(z_तथ्य, 0, 0);
}
अटल पूर्णांक kvvfs_Write_File_आकार(KVVfsFile *pFile, sqlite3_int64 sz){
  अक्षर z_तथ्य[50];
  sqlite3_snprintf(माप(z_तथ्य), z_तथ्य, "%lld", sz);
  वापस sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "sz", z_तथ्य);
}

/****** sqlite3_io_methods methods ******************************************/

/*
** Close an kvvfs-file.
*/
अटल पूर्णांक kvvfsClose(sqlite3_file *pProtoFile){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;

  SQLITE_KV_LOG(("xClose %s %s\n", pFile->zClass,
             pFile->isJournal ? "journal" : "db"));
  sqlite3_free(pFile->aJrnl);
  sqlite3_free(pFile->a_तथ्य);
  स_रखो(pFile, 0, माप(*pFile));
  वापस SQLITE_OK;
}

/*
** Read from the -journal file.
*/
अटल पूर्णांक kvvfsReadJrnl(
  sqlite3_file *pProtoFile,
  व्योम *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  निश्चित( pFile->isJournal );
  SQLITE_KV_LOG(("xRead('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  अगर( pFile->aJrnl==0 ){
    पूर्णांक rc;
    पूर्णांक szTxt = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                              0, 0);
    अक्षर *aTxt;
    अगर( szTxt<=4 ){
      वापस SQLITE_IOERR;
    }
    aTxt = sqlite3_malloc64( szTxt+1 );
    अगर( aTxt==0 ) वापस SQLITE_NOMEM;
    rc = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, "jrnl",
                                       aTxt, szTxt+1);
    अगर( rc>=0 ){
      kvvfsDecodeJournal(pFile, aTxt, szTxt);
      rc = 0;
    }
    sqlite3_free(aTxt);
    अगर( rc ) वापस rc;
    अगर( pFile->aJrnl==0 ) वापस SQLITE_IOERR;
  }
  अगर( iOfst+iAmt>pFile->nJrnl ){
    वापस SQLITE_IOERR_SHORT_READ;
  }
  स_नकल(zBuf, pFile->aJrnl+iOfst, iAmt);
  वापस SQLITE_OK;
}

/*
** Read from the database file.
*/
अटल पूर्णांक kvvfsReadDb(
  sqlite3_file *pProtoFile,
  व्योम *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  अचिन्हित पूर्णांक pgno;
  पूर्णांक got, n;
  अक्षर z_कुंजी[30];
  अक्षर *a_तथ्य = pFile->a_तथ्य;
  निश्चित( iOfst>=0 );
  निश्चित( iAmt>=0 );
  SQLITE_KV_LOG(("xRead('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  अगर( iOfst+iAmt>=512 ){
    अगर( (iOfst % iAmt)!=0 ){
      वापस SQLITE_IOERR_READ;
    }
    अगर( (iAmt & (iAmt-1))!=0 || iAmt<512 || iAmt>65536 ){
      वापस SQLITE_IOERR_READ;
    }
    pFile->szPage = iAmt;
    pgno = 1 + iOfst/iAmt;
  }अन्यथा{
    pgno = 1;
  }
  sqlite3_snprintf(माप(z_कुंजी), z_कुंजी, "%u", pgno);
  got = sqlite3KvvfsMethods.xRcrdRead(pFile->zClass, z_कुंजी,
                                      a_तथ्य, SQLITE_KVOS_SZ-1);
  अगर( got<0 ){
    n = 0;
  }अन्यथा{
    a_तथ्य[got] = 0;
    अगर( iOfst+iAmt<512 ){
      पूर्णांक k = iOfst+iAmt;
      a_तथ्य[k*2] = 0;
      n = kvvfsDecode(a_तथ्य, &a_तथ्य[2000], SQLITE_KVOS_SZ-2000);
      अगर( n>=iOfst+iAmt ){
        स_नकल(zBuf, &a_तथ्य[2000+iOfst], iAmt);
        n = iAmt;
      }अन्यथा{
        n = 0;
      }
    }अन्यथा{
      n = kvvfsDecode(a_तथ्य, zBuf, iAmt);
    }
  }
  अगर( n<iAmt ){
    स_रखो(zBuf+n, 0, iAmt-n);
    वापस SQLITE_IOERR_SHORT_READ;
  }
  वापस SQLITE_OK;
}


/*
** Write into the -journal file.
*/
अटल पूर्णांक kvvfsWriteJrnl(
  sqlite3_file *pProtoFile,
  स्थिर व्योम *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  sqlite3_int64 iEnd = iOfst+iAmt;
  SQLITE_KV_LOG(("xWrite('%s-journal',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  अगर( iEnd>=0x10000000 ) वापस SQLITE_FULL;
  अगर( pFile->aJrnl==0 || pFile->nJrnl<iEnd ){
    अक्षर *aNew = sqlite3_realloc(pFile->aJrnl, iEnd);
    अगर( aNew==0 ){
      वापस SQLITE_IOERR_NOMEM;
    }
    pFile->aJrnl = aNew;
    अगर( pFile->nJrnl<iOfst ){
      स_रखो(pFile->aJrnl+pFile->nJrnl, 0, iOfst-pFile->nJrnl);
    }
    pFile->nJrnl = iEnd;
  }
  स_नकल(pFile->aJrnl+iOfst, zBuf, iAmt);
  वापस SQLITE_OK;
}

/*
** Write into the database file.
*/
अटल पूर्णांक kvvfsWriteDb(
  sqlite3_file *pProtoFile,
  स्थिर व्योम *zBuf,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  अचिन्हित पूर्णांक pgno;
  अक्षर z_कुंजी[30];
  अक्षर *a_तथ्य = pFile->a_तथ्य;
  पूर्णांक rc;
  SQLITE_KV_LOG(("xWrite('%s-db',%d,%lld)\n", pFile->zClass, iAmt, iOfst));
  निश्चित( iAmt>=512 && iAmt<=65536 );
  निश्चित( (iAmt & (iAmt-1))==0 );
  निश्चित( pFile->szPage<0 || pFile->szPage==iAmt );
  pFile->szPage = iAmt;
  pgno = 1 + iOfst/iAmt;
  sqlite3_snprintf(माप(z_कुंजी), z_कुंजी, "%u", pgno);
  kvvfsEncode(zBuf, iAmt, a_तथ्य);
  rc = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, z_कुंजी, a_तथ्य);
  अगर( 0==rc ){
    अगर( iOfst+iAmt > pFile->szDb ){
      pFile->szDb = iOfst + iAmt;
    }
  }
  वापस rc;
}

/*
** Truncate an kvvfs-file.
*/
अटल पूर्णांक kvvfsTruncateJrnl(sqlite3_file *pProtoFile, sqlite_int64 आकार){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xTruncate('%s-journal',%lld)\n", pFile->zClass, आकार));
  निश्चित( आकार==0 );
  sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, "jrnl");
  sqlite3_free(pFile->aJrnl);
  pFile->aJrnl = 0;
  pFile->nJrnl = 0;
  वापस SQLITE_OK;
}
अटल पूर्णांक kvvfsTruncateDb(sqlite3_file *pProtoFile, sqlite_int64 आकार){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  अगर( pFile->szDb>आकार
   && pFile->szPage>0
   && (आकार % pFile->szPage)==0
  ){
    अक्षर z_कुंजी[50];
    अचिन्हित पूर्णांक pgno, pgnoMax;
    SQLITE_KV_LOG(("xTruncate('%s-db',%lld)\n", pFile->zClass, आकार));
    pgno = 1 + आकार/pFile->szPage;
    pgnoMax = 2 + pFile->szDb/pFile->szPage;
    जबतक( pgno<=pgnoMax ){
      sqlite3_snprintf(माप(z_कुंजी), z_कुंजी, "%u", pgno);
      sqlite3KvvfsMethods.xRcrdDelete(pFile->zClass, z_कुंजी);
      pgno++;
    }
    pFile->szDb = आकार;
    वापस kvvfs_Write_File_आकार(pFile, आकार) ? SQLITE_IOERR : SQLITE_OK;
  }
  वापस SQLITE_IOERR;
}

/*
** Sync an kvvfs-file.
*/
अटल पूर्णांक kvvfsSyncJrnl(sqlite3_file *pProtoFile, पूर्णांक flags){
  पूर्णांक i, n;
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  अक्षर *zOut;
  SQLITE_KV_LOG(("xSync('%s-journal')\n", pFile->zClass));
  अगर( pFile->nJrnl<=0 ){
    वापस kvvfsTruncateJrnl(pProtoFile, 0);
  }
  zOut = sqlite3_malloc64( pFile->nJrnl*2 + 50 );
  अगर( zOut==0 ){
    वापस SQLITE_IOERR_NOMEM;
  }
  n = pFile->nJrnl;
  i = 0;
  करो{
    zOut[i++] = 'a' + (n%26);
    n /= 26;
  }जबतक( n>0 );
  zOut[i++] = ' ';
  kvvfsEncode(pFile->aJrnl, pFile->nJrnl, &zOut[i]);
  i = sqlite3KvvfsMethods.xRcrdWrite(pFile->zClass, "jrnl", zOut);
  sqlite3_free(zOut);
  वापस i ? SQLITE_IOERR : SQLITE_OK;
}
अटल पूर्णांक kvvfsSyncDb(sqlite3_file *pProtoFile, पूर्णांक flags){
  वापस SQLITE_OK;
}

/*
** Return the current file-size of an kvvfs-file.
*/
अटल पूर्णांक kvvfs_File_आकार_Jrnl(sqlite3_file *pProtoFile, sqlite_int64 *p_आकार){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-journal')\n", pFile->zClass));
  *p_आकार = pFile->nJrnl;
  वापस SQLITE_OK;
}
अटल पूर्णांक kvvfs_File_आकार_Db(sqlite3_file *pProtoFile, sqlite_int64 *p_आकार){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  SQLITE_KV_LOG(("xFileSize('%s-db')\n", pFile->zClass));
  अगर( pFile->szDb>=0 ){
    *p_आकार = pFile->szDb;
  }अन्यथा{
    *p_आकार = kvvfs_Read_File_आकार(pFile);
  }
  वापस SQLITE_OK;
}

/*
** Lock an kvvfs-file.
*/
अटल पूर्णांक kvvfsLock(sqlite3_file *pProtoFile, पूर्णांक eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  निश्चित( !pFile->isJournal );
  SQLITE_KV_LOG(("xLock(%s,%d)\n", pFile->zClass, eLock));

  अगर( eLock!=SQLITE_LOCK_NONE ){
    pFile->szDb = kvvfs_Read_File_आकार(pFile);
  }
  वापस SQLITE_OK;
}

/*
** Unlock an kvvfs-file.
*/
अटल पूर्णांक kvvfsUnlock(sqlite3_file *pProtoFile, पूर्णांक eLock){
  KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
  निश्चित( !pFile->isJournal );
  SQLITE_KV_LOG(("xUnlock(%s,%d)\n", pFile->zClass, eLock));
  अगर( eLock==SQLITE_LOCK_NONE ){
    pFile->szDb = -1;
  }
  वापस SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on an kvvfs-file.
*/
अटल पूर्णांक kvvfsCheckReservedLock(sqlite3_file *pProtoFile, पूर्णांक *pResOut){
  SQLITE_KV_LOG(("xCheckReservedLock\n"));
  *pResOut = 0;
  वापस SQLITE_OK;
}

/*
** File control method. For custom operations on an kvvfs-file.
*/
अटल पूर्णांक kvvfsFileControlJrnl(sqlite3_file *pProtoFile, पूर्णांक op, व्योम *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on journal\n", op));
  वापस SQLITE_NOTFOUND;
}
अटल पूर्णांक kvvfsFileControlDb(sqlite3_file *pProtoFile, पूर्णांक op, व्योम *pArg){
  SQLITE_KV_LOG(("xFileControl(%d) on database\n", op));
  अगर( op==SQLITE_FCNTL_SYNC ){
    KVVfsFile *pFile = (KVVfsFile *)pProtoFile;
    पूर्णांक rc = SQLITE_OK;
    SQLITE_KV_LOG(("xSync('%s-db')\n", pFile->zClass));
    अगर( pFile->szDb>0 && 0!=kvvfs_Write_File_आकार(pFile, pFile->szDb) ){
      rc = SQLITE_IOERR;
    }
    वापस rc;
  }
  वापस SQLITE_NOTFOUND;
}

/*
** Return the sector-size in bytes for an kvvfs-file.
*/
अटल पूर्णांक kvvfs_Sector_आकार(sqlite3_file *pFile){
  वापस 512;
}

/*
** Return the device characteristic flags supported by an kvvfs-file.
*/
अटल पूर्णांक kvvfsDeviceCharacteristics(sqlite3_file *pProtoFile){
  वापस 0;
}

/****** sqlite3_vfs methods *************************************************/

/*
** Open an kvvfs file handle.
*/
अटल पूर्णांक kvvfsOpen(
  sqlite3_vfs *pProtoVfs,
  स्थिर अक्षर *zName,
  sqlite3_file *pProtoFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  KVVfsFile *pFile = (KVVfsFile*)pProtoFile;
  अगर( zName==0 ) zName = "";
  SQLITE_KV_LOG(("xOpen(\"%s\")\n", zName));
  निश्चित(!pFile->zClass);
  निश्चित(!pFile->a_तथ्य);
  निश्चित(!pFile->aJrnl);
  निश्चित(!pFile->nJrnl);
  निश्चित(!pFile->base.pMethods);
  pFile->szPage = -1;
  pFile->szDb = -1;
  अगर( 0==sqlite3_strglob("*-journal", zName) ){
    pFile->isJournal = 1;
    pFile->base.pMethods = &kvvfs_jrnl_io_methods;
    अगर( 0==म_भेद("session-journal",zName) ){
      pFile->zClass = "session";
    }अन्यथा अगर( 0==म_भेद("local-journal",zName) ){
      pFile->zClass = "local";
    }
  }अन्यथा{
    pFile->isJournal = 0;
    pFile->base.pMethods = &kvvfs_db_io_methods;
  }
  अगर( !pFile->zClass ){
#अगर_घोषित SQLITE_WASM
    अगर( म_माप(zName) >= (KVRECORD_कुंजी_SZ
                          - 6 /* "kvvfs-" */
                          - 11 /* "-NNNNNNNNNNN" */) ){
      वापस SQLITE_CANTOPEN;
    }
#अन्यथा
    अगर( 0!=म_भेद(zName, "local") && 0!=म_भेद(zName, "session") ){
      /* Historical naming restriction which journaling depends on. */
      वापस SQLITE_CANTOPEN;
    }
#पूर्ण_अगर
    pFile->zClass = zName;
  }
  pFile->a_तथ्य = sqlite3_malloc64(SQLITE_KVOS_SZ);
  अगर( pFile->a_तथ्य==0 ){
    वापस SQLITE_NOMEM;
  }
  वापस SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
अटल पूर्णांक kvvfsDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक rc /* The JS impl can fail with OOM in argument conversion */;
  अगर( म_भेद(zPath, "local-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("local", "jrnl");
  }अन्यथा
  अगर( म_भेद(zPath, "session-journal")==0 ){
    rc = sqlite3KvvfsMethods.xRcrdDelete("session", "jrnl");
  }
  अन्यथा{
    rc = 0;
  }
  वापस rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
अटल पूर्णांक kvvfsAccess(
  sqlite3_vfs *pProtoVfs,
  स्थिर अक्षर *zPath,
  पूर्णांक flags,
  पूर्णांक *pResOut
){
  SQLITE_KV_LOG(("xAccess(\"%s\")\n", zPath));
#अगर 0 && defined(SQLITE_WASM)
  /*
  ** This is not having the desired effect in the JS bindings.
  ** It's ostensibly the same logic as the #else block, but
  ** it's not behaving that way.
  **
  ** In JS we map all zPaths to Storage objects, and -journal files
  ** are mapped to the storage for the main db (which is is exactly
  ** what the mapping of "local-journal" -> "local" is doing).
  */
  स्थिर अक्षर *z_कुंजी = (0==sqlite3_strglob("*-journal", zPath))
    ? "jrnl" : "sz";
  *pResOut =
    sqlite3KvvfsMethods.xRcrdRead(zPath, z_कुंजी, 0, 0)>0;
#अन्यथा
  अगर( म_भेद(zPath, "local-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "jrnl", 0, 0)>0;
  }अन्यथा
  अगर( म_भेद(zPath, "session-journal")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "jrnl", 0, 0)>0;
  }अन्यथा
  अगर( म_भेद(zPath, "local")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("local", "sz", 0, 0)>0;
  }अन्यथा
  अगर( म_भेद(zPath, "session")==0 ){
    *pResOut =
      sqlite3KvvfsMethods.xRcrdRead("session", "sz", 0, 0)>0;
  }अन्यथा
  {
    *pResOut = 0;
  }
  /*all current JS tests avoid triggering: assert( *pResOut == 0 ); */
#पूर्ण_अगर
  SQLITE_KV_LOG(("xAccess returns %d\n",*pResOut));
  वापस SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
अटल पूर्णांक kvvfsFullPathname(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zPath,
  पूर्णांक nOut,
  अक्षर *zOut
){
  माप_प्रकार nPath;
#अगर_घोषित SQLITE_OS_KV_ALWAYS_LOCAL
  zPath = "local";
#पूर्ण_अगर
  nPath = म_माप(zPath);
  SQLITE_KV_LOG(("xFullPathname(\"%s\")\n", zPath));
  अगर( nOut<nPath+1 ) nPath = nOut - 1;
  स_नकल(zOut, zPath, nPath);
  zOut[nPath] = 0;
  वापस SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
अटल व्योम *kvvfsDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  वापस 0;
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
अटल पूर्णांक kvvfsRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  स_रखो(zBufOut, 0, nByte);
  वापस nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
अटल पूर्णांक kvvfsSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  वापस SQLITE_OK;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
अटल पूर्णांक kvvfsCurrentTime(sqlite3_vfs *pVfs, द्विगुन *pTimeOut){
  sqlite3_int64 i = 0;
  पूर्णांक rc;
  rc = kvvfsCurrentTimeInt64(0, &i);
  *pTimeOut = i/86400000.0;
  वापस rc;
}
#समावेश <sys/time.h>
अटल पूर्णांक kvvfsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut){
  अटल स्थिर sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  काष्ठा timeval sNow;
  (व्योम)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *pTimeOut = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_OS_KV || SQLITE_OS_UNIX */

#अगर SQLITE_OS_KV
/*
** This routine is called initialize the KV-vfs as the default VFS.
*/
पूर्णांक sqlite3_os_init(व्योम){
  वापस sqlite3_vfs_register(&sqlite3OsKvvfsObject, 1);
}
पूर्णांक sqlite3_os_end(व्योम){
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_OS_KV */

#अगर SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
पूर्णांक sqlite3KvvfsInit(व्योम){
  वापस sqlite3_vfs_register(&sqlite3OsKvvfsObject, 0);
}
#पूर्ण_अगर
