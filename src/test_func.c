/*
** 2008 March 19
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing all sorts of SQLite interfaces.  This code
** implements new SQL functions used by the test scripts.
*/
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *testContextMalloc(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ nByte){
  ವರ್ಣ *z = sqlite3_malloc(nByte);
  ಯದಿ( !z && nByte>0 ){
    sqlite3_result_error_nomem(context);
  }
  ಪ್ರತ್ಯಾಗಮ z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
ಸ್ಥಿರ ಶೂನ್ಯ randStr(sqlite3_context *context, ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  ಪೂರ್ಣಾಙ್ಕ iMin, iMax, n, r, i;
  unsigned ವರ್ಣ zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  ಪ್ರತಿಜ್ಞಾ(argc==2);

  iMin = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  ಯದಿ( iMin<0 ) iMin = 0;
  ಯದಿ( iMin>=ಪರಿಮಾಣ(zBuf) ) iMin = ಪರಿಮಾಣ(zBuf)-1;
  iMax = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  ಯದಿ( iMax<iMin ) iMax = iMin;
  ಯದಿ( iMax>=ಪರಿಮಾಣ(zBuf) ) iMax = ಪರಿಮಾಣ(zBuf)-1;
  n = iMin;
  ಯದಿ( iMax>iMin ){
    sqlite3_randomness(ಪರಿಮಾಣ(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  ಪ್ರತಿಜ್ಞಾ( n<ಪರಿಮಾಣ(zBuf) );
  sqlite3_randomness(n, zBuf);
  ಪ್ರತಿ(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(ಪರಿಮಾಣ(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_result_text(context, (ವರ್ಣ*)zBuf, n, SQLITE_TRANSIENT);
}

/*
** The following two SQL functions are used to test returning a text
** result with a destructor. Function 'test_destructor' takes one argument
** and returns the same argument interpreted as TEXT. A destructor is
** passed with the sqlite3_result_text() call.
**
** SQL function 'test_destructor_count' returns the number of outstanding 
** allocations made by 'test_destructor';
**
** WARNING: Not threadsafe.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_destructor_ಎಣಿಕೆ_var = 0;
ಸ್ಥಿರ ಶೂನ್ಯ destructor(ಶೂನ್ಯ *p){
  ವರ್ಣ *zVal = (ವರ್ಣ *)p;
  ಪ್ರತಿಜ್ಞಾ(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_ಎಣಿಕೆ_var--;
}
ಸ್ಥಿರ ಶೂನ್ಯ test_destructor(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ವರ್ಣ *zVal;
  ಪೂರ್ಣಾಙ್ಕ len;
  
  test_destructor_ಎಣಿಕೆ_var++;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  len = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ಯದಿ( !zVal ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_ಮೌಲ್ಯ_text(argv[0]), len);
  sqlite3_result_text(pCtx, zVal, -1, destructor);
}
#ifndef SQLITE_OMIT_UTF16
ಸ್ಥಿರ ಶೂನ್ಯ test_destructor16(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ವರ್ಣ *zVal;
  ಪೂರ್ಣಾಙ್ಕ len;
  
  test_destructor_ಎಣಿಕೆ_var++;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಪ್ರತ್ಯಾಗಮ;
  len = sqlite3_ಮೌಲ್ಯ_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ಯದಿ( !zVal ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_ಮೌಲ್ಯ_text16(argv[0]), len);
  sqlite3_result_text16(pCtx, zVal, -1, destructor);
}
#endif
ಸ್ಥಿರ ಶೂನ್ಯ test_destructor_ಎಣಿಕೆ(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_result_int(pCtx, test_destructor_ಎಣಿಕೆ_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ifndef SQLITE_UNTESTABLE
ಶೂನ್ಯ sqlite3BeginBenignMalloc(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3EndBenignMalloc(ಶೂನ್ಯ);
#ಅನ್ಯಥಾ
  #define sqlite3BeginBenignMalloc()
  #define sqlite3EndBenignMalloc()
#endif
ಸ್ಥಿರ ಶೂನ್ಯ test_agg_errmsg16_step(sqlite3_context *a, ಪೂರ್ಣಾಙ್ಕ b,sqlite3_ಮೌಲ್ಯ **c){
}
ಸ್ಥಿರ ಶೂನ್ಯ test_agg_errmsg16_final(sqlite3_context *ctx){
#ifndef SQLITE_OMIT_UTF16
  ಧ್ರುವ ಶೂನ್ಯ *z;
  sqlite3 * db = sqlite3_context_db_handle(ctx);
  sqlite3_aggregate_context(ctx, 2048);
  z = sqlite3_errmsg16(db);
  sqlite3_result_text16(ctx, z, -1, SQLITE_TRANSIENT);
#endif
}

/*
** Routines for testing the sqlite3_get_auxdata() and sqlite3_set_auxdata()
** interface.
**
** The test_auxdata() SQL function attempts to register each of its arguments
** as auxiliary data.  If there are no prior registrations of aux data for
** that argument (meaning the argument is not a constant or this is its first
** call) then the result for that argument is 0.  If there is a prior
** registration, the result for that argument is 1.  The overall result
** is the individual argument results separated by spaces.
*/
ಸ್ಥಿರ ಶೂನ್ಯ free_test_auxdata(ಶೂನ್ಯ *p) {sqlite3_free(p);}
ಸ್ಥಿರ ಶೂನ್ಯ test_auxdata(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ವರ್ಣ *zRet = testContextMalloc(pCtx, nArg*2);
  ಯದಿ( !zRet ) ಪ್ರತ್ಯಾಗಮ;
  memset(zRet, 0, nArg*2);
  ಪ್ರತಿ(i=0; i<nArg; i++){
    ವರ್ಣ ಧ್ರುವ *z = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
    ಯದಿ( z ){
      ಪೂರ್ಣಾಙ್ಕ n;
      ವರ್ಣ *zAux = sqlite3_get_auxdata(pCtx, i);
      ಯದಿ( zAux ){
        zRet[i*2] = '1';
        ಪ್ರತಿಜ್ಞಾ( strcmp(zAux,z)==0 );
      }ಅನ್ಯಥಾ {
        zRet[i*2] = '0';
      }
      n = (ಪೂರ್ಣಾಙ್ಕ)strlen(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      ಯದಿ( zAux ){
        memcpy(zAux, z, n);
        sqlite3_set_auxdata(pCtx, i, zAux, free_test_auxdata);
      }
      zRet[i*2+1] = ' ';
    }
  }
  sqlite3_result_text(pCtx, zRet, 2*nArg-1, free_test_auxdata);
}

/*
** A function to test error reporting from user functions. This function
** returns a copy of its first argument as the error message.  If the
** second argument exists, it becomes the error code.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_error(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_result_error(pCtx, (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]), -1);
  ಯದಿ( nArg==2 ){
    sqlite3_result_error_code(pCtx, sqlite3_ಮೌಲ್ಯ_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
ಸ್ಥಿರ ಶೂನ್ಯ counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  ಪೂರ್ಣಾಙ್ಕ nArg,                /* Number of function arguments */
  sqlite3_ಮೌಲ್ಯ **argv     /* Values for all function arguments */
){
  ಪೂರ್ಣಾಙ್ಕ *pCounter = (ಪೂರ್ಣಾಙ್ಕ*)sqlite3_get_auxdata(pCtx, 0);
  ಯದಿ( pCounter==0 ){
    pCounter = sqlite3_malloc( ಪರಿಮಾಣ(*pCounter) );
    ಯದಿ( pCounter==0 ){
      sqlite3_result_error_nomem(pCtx);
      ಪ್ರತ್ಯಾಗಮ;
    }
    *pCounter = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
    sqlite3_set_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }ಅನ್ಯಥಾ{
    ++*pCounter;
  }
  sqlite3_result_int(pCtx, *pCounter);
}


/*
** This function takes two arguments.  It performance UTF-8/16 type
** conversions on the first argument then returns a copy of the second
** argument.
**
** This function is used in cases such as the following:
**
**      SELECT test_isolation(x,x) FROM t1;
**
** We want to verify that the type conversions that occur on the
** first argument do not invalidate the second argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_isolation(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3_ಮೌಲ್ಯ_text16(argv[0]);
  sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  sqlite3_ಮೌಲ್ಯ_text16(argv[0]);
  sqlite3_ಮೌಲ್ಯ_text(argv[0]);
#endif
  sqlite3_result_ಮೌಲ್ಯ(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_eval(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ಧ್ರುವ ವರ್ಣ *zSql;
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ zTag[] = "test_eval()";

  ಯದಿ( sqlite3_get_clientdata(db, zTag) ){
    sqlite3_result_error(pCtx, "nested test_eval()", -1);
    ಪ್ರತ್ಯಾಗಮ;
  }
  zSql = (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  sqlite3_set_clientdata(db, zTag, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3_step(pStmt);
    ಯದಿ( rc==SQLITE_ROW ){
      sqlite3_result_ಮೌಲ್ಯ(pCtx, sqlite3_column_ಮೌಲ್ಯ(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_set_clientdata(db, zTag, 0, 0);
  ಯದಿ( rc ){
    ವರ್ಣ *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_result_text(pCtx, zErr, -1, sqlite3_free);
    sqlite3_result_error_code(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ testHexChar(ವರ್ಣ c){
  ಯದಿ( c>='0' && c<='9' ){
    ಪ್ರತ್ಯಾಗಮ c - '0';
  }ಅನ್ಯಥಾ ಯದಿ( c>='a' && c<='f' ){
    ಪ್ರತ್ಯಾಗಮ c - 'a' + 10;
  }ಅನ್ಯಥಾ ಯದಿ( c>='A' && c<='F' ){
    ಪ್ರತ್ಯಾಗಮ c - 'A' + 10;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Convert hex to binary.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testHexToBin(ಧ್ರುವ ವರ್ಣ *zIn, ವರ್ಣ *zOut){
  ಯಾವತ್( zIn[0] && zIn[1] ){
    *(zOut++) = (testHexChar(zIn[0])<<4) + testHexChar(zIn[1]);
    zIn += 2;
  }
}

/*
**      hex_to_utf16be(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#ifndef SQLITE_OMIT_UTF16
ಸ್ಥಿರ ಶೂನ್ಯ testHexToUtf16be(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ n;
  ಧ್ರುವ ವರ್ಣ *zIn;
  ವರ್ಣ *zOut;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  n = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  zIn = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  ಯದಿ( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }ಅನ್ಯಥಾ{
    testHexToBin(zIn, zOut);
    sqlite3_result_text16be(pCtx, zOut, n/2, sqlite3_free);
  }
}
#endif

/*
**      hex_to_utf8(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
ಸ್ಥಿರ ಶೂನ್ಯ testHexToUtf8(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ n;
  ಧ್ರುವ ವರ್ಣ *zIn;
  ವರ್ಣ *zOut;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  n = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  zIn = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  ಯದಿ( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }ಅನ್ಯಥಾ{
    testHexToBin(zIn, zOut);
    sqlite3_result_text(pCtx, zOut, n/2, sqlite3_free);
  }
}

/*
**      hex_to_utf16le(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#ifndef SQLITE_OMIT_UTF16
ಸ್ಥಿರ ಶೂನ್ಯ testHexToUtf16le(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಙ್ಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ n;
  ಧ್ರುವ ವರ್ಣ *zIn;
  ವರ್ಣ *zOut;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  n = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  zIn = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  ಯದಿ( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }ಅನ್ಯಥಾ{
    testHexToBin(zIn, zOut);
    sqlite3_result_text16le(pCtx, zOut, n/2, sqlite3_free);
  }
}
#endif

/*
** SQL function:   real2hex(X)
**
** If argument X is a real number, then convert it into a string which is
** the big-endian hexadecimal representation of the ieee754 encoding of
** that number.  If X is not a real number, return NULL.
*/
ಸ್ಥಿರ ಶೂನ್ಯ real2hex(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಸಙ್ಘ {
    sqlite3_uint64 i;
    ದ್ವಿಗುಣ r;
    unsigned ವರ್ಣ x[8];
  } v;
  ವರ್ಣ zOut[20];
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  ಪ್ರತಿ(i=0; i<8; i++){
    ಯದಿ( bigEndian ){
      zOut[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }ಅನ್ಯಥಾ{
      zOut[14-i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[14-i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }
  }
  zOut[16] = 0;
  sqlite3_result_text(context, zOut, -1, SQLITE_TRANSIENT);
}

/*
**     test_extract(record, field)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as the first argument. The
** second argument is the index of the field within that record to
** extract and return.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_extract(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  ಪೂರ್ಣಾಙ್ಕ iIdx;                       /* Required field */
  ಪೂರ್ಣಾಙ್ಕ iCurrent = 0;               /* Current field */

  ಪ್ರತಿಜ್ಞಾ( argc==2 );
  pRec = (u8*)sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
  iIdx = sqlite3_ಮೌಲ್ಯ_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];

  ಪ್ರತಿ(iCurrent=0; pHdr<pEndHdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, ಪರಿಮಾಣ(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ಯದಿ( iCurrent==iIdx ){
      sqlite3_result_ಮೌಲ್ಯ(context, &mem);
    }

    ಯದಿ( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
  }
}

/*
**      test_decode(record)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as its only argument. It returns
** a tcl list (type SQLITE_TEXT) containing each of the values stored
** in the record.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_decode(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRet);

  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  pRec = (u8*)sqlite3_ಮೌಲ್ಯ_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];
  ಯಾವತ್( pHdr<pEndHdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, ಪರಿಮಾಣ(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ವಿಕಲ್ಪ( sqlite3_ಮೌಲ್ಯ_type(&mem) ){
      ಪಕ್ಷ SQLITE_TEXT:
        pVal = Tcl_NewStringObj((ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(&mem), -1);
        ಭಙ್ಗ;

      ಪಕ್ಷ SQLITE_BLOB: {
        ವರ್ಣ hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        ಪೂರ್ಣಾಙ್ಕ n = sqlite3_ಮೌಲ್ಯ_bytes(&mem);
        u8 *z = (u8*)sqlite3_ಮೌಲ್ಯ_blob(&mem);
        ಪೂರ್ಣಾಙ್ಕ i;
        pVal = Tcl_NewStringObj("x'", -1);
        ಪ್ರತಿ(i=0; i<n; i++){
          ವರ್ಣ hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_AppendStringsToObj(pVal, hex, 0);
        }
        Tcl_AppendStringsToObj(pVal, "'", 0);
        ಭಙ್ಗ;
      }

      ಪಕ್ಷ SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_ಮೌಲ್ಯ_double(&mem));
        ಭಙ್ಗ;

      ಪಕ್ಷ SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_ಮೌಲ್ಯ_int64(&mem));
        ಭಙ್ಗ;

      ಪಕ್ಷ SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        ಭಙ್ಗ;

      ಸ್ವಭಾವ:
        ಪ್ರತಿಜ್ಞಾ( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    ಯದಿ( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_result_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_zeroblob(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ nZero = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  sqlite3_result_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_getsubtype(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_result_int(context, (ಪೂರ್ಣಾಙ್ಕ)sqlite3_ಮೌಲ್ಯ_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_frombind(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_uint64 m = 0;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<argc && i<63; i++){
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_result_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_setsubtype(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_result_ಮೌಲ್ಯ(context, argv[0]);
  sqlite3_result_subtype(context, (unsigned ಪೂರ್ಣಾಙ್ಕ)sqlite3_ಮೌಲ್ಯ_int(argv[1]));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ registerTestFunctions(
  sqlite3 *db,
  ವರ್ಣ **pz_Err_ಸಂದೇಶ,
  ಧ್ರುವ sqlite3_api_routines *pThunk
){
  ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     signed ವರ್ಣ nArg;
     unsigned ಪೂರ್ಣಾಙ್ಕ eTextRep; /* 1: UTF-16.  0: UTF-8 */
     ಶೂನ್ಯ (*xFunc)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ifndef SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, testHexToUtf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, testHexToUtf16le},
#endif
    { "hex_to_utf8",           1, SQLITE_UTF8, testHexToUtf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_ಎಣಿಕೆ},
    { "test_auxdata",         -1, SQLITE_UTF8, test_auxdata},
    { "test_error",            1, SQLITE_UTF8, test_error},
    { "test_error",            2, SQLITE_UTF8, test_error},
    { "test_eval",             1, SQLITE_UTF8, test_eval},
    { "test_isolation",        2, SQLITE_UTF8, test_isolation},
    { "test_counter",          1, SQLITE_UTF8, counterFunc},
    { "real2hex",              1, SQLITE_UTF8, real2hex},
    { "test_decode",           1, SQLITE_UTF8, test_decode},
    { "test_extract",          2, SQLITE_UTF8, test_extract},
    { "test_zeroblob",  1, SQLITE_UTF8|SQLITE_DETERMINISTIC, test_zeroblob},
    { "test_getsubtype",       1, SQLITE_UTF8, test_getsubtype},
    { "test_setsubtype",       2, SQLITE_UTF8|SQLITE_RESULT_SUBTYPE,
                                               test_setsubtype},
    { "test_frombind",        -1, SQLITE_UTF8, test_frombind},
  };
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aFuncs)/ಪರಿಮಾಣ(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].eTextRep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_step, test_agg_errmsg16_final);
      
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI autoinstall_test_funcs(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Md5_Register(sqlite3 *, ವರ್ಣ **, ಧ್ರುವ sqlite3_api_routines *);
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))registerTestFunctions);
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))Md5_Register);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tStep(sqlite3_context *a, ಪೂರ್ಣಾಙ್ಕ b, sqlite3_ಮೌಲ್ಯ **c){}
ಸ್ಥಿರ ಶೂನ್ಯ tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI abuse_create_function(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp*, ಧ್ರುವ ವರ್ಣ*, sqlite3**);
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ mxArg;

  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep,tStep,tFinal);
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, tStep, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, 0, tFinal);
  ಯದಿ( rc!=SQLITE_MISUSE) ಗಚ್ಛ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, tStep, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, tStep, 0, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, tStep, 0, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, tStep, 0, 0);
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ abuse_err;

  /* This last function registration should actually work.  Generate
  ** a no-op function (that always returns NULL) and which has the
  ** maximum-length function name and the maximum number of parameters.
  */
  sqlite3_limit(db, SQLITE_LIMIT_FUNCTION_ARG, 1000000);
  mxArg = sqlite3_limit(db, SQLITE_LIMIT_FUNCTION_ARG, -1);
  rc = sqlite3_create_function(db, "nullx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       mxArg, SQLITE_UTF8, 0, tStep, 0, 0);
  ಯದಿ( rc!=SQLITE_OK ) ಗಚ್ಛ abuse_err;
                                
  ಪ್ರತ್ಯಾಗಮ TCL_OK;

abuse_err:
  Tcl_AppendResult(interp, "sqlite3_create_function abused test failed", 
                   (ವರ್ಣ*)0);
  ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
}


/*
** SQLite user defined function to use with matchinfo() to calculate the
** relevancy of an FTS match. The value returned is the relevancy score
** (a real value greater than or equal to zero). A larger value indicates 
** a more relevant document.
**
** The overall relevancy returned is the sum of the relevancies of each 
** column value in the FTS table. The relevancy of a column value is the
** sum of the following for each reportable phrase in the FTS query:
**
**   (<hit count> / <global hit count>) * <column weight>
**
** where <hit count> is the number of instances of the phrase in the
** column value of the current row and <global hit count> is the number
** of instances of the phrase in the same column of all rows in the FTS
** table. The <column weight> is a weighting factor assigned to each
** column by the caller (see below).
**
** The first argument to this function must be the return value of the FTS 
** matchinfo() function. Following this must be one argument for each column 
** of the FTS table containing a numeric weight factor for the corresponding 
** column. Example:
**
**     CREATE VIRTUAL TABLE documents USING fts3(title, content)
**
** The following query returns the docids of documents that match the full-text
** query <query> sorted from most to least relevant. When calculating
** relevance, query term instances in the 'title' column are given twice the
** weighting of those in the 'content' column.
**
**     SELECT docid FROM documents 
**     WHERE documents MATCH <query> 
**     ORDER BY rank(matchinfo(documents), 1.0, 0.5) DESC
*/
ಸ್ಥಿರ ಶೂನ್ಯ rankfunc(sqlite3_context *pCtx, ಪೂರ್ಣಾಙ್ಕ nVal, sqlite3_ಮೌಲ್ಯ **apVal){
  ಪೂರ್ಣಾಙ್ಕ *aMatchinfo;                /* Return value of matchinfo() */
  ಪೂರ್ಣಾಙ್ಕ nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  ಪೂರ್ಣಾಙ್ಕ nCol = 0;                   /* Number of columns in the table */
  ಪೂರ್ಣಾಙ್ಕ nPhrase = 0;                /* Number of phrases in the query */
  ಪೂರ್ಣಾಙ್ಕ iPhrase;                    /* Current phrase */
  ದ್ವಿಗುಣ score = 0.0;             /* Value to return */

  ಪ್ರತಿಜ್ಞಾ( ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  ಯದಿ( nVal<1 ) ಗಚ್ಛ wrong_ಸಂಖ್ಯೆ_args;
  aMatchinfo = (ಪೂರ್ಣಾಙ್ಕ*)sqlite3_ಮೌಲ್ಯ_blob(apVal[0]);
  nMatchinfo = sqlite3_ಮೌಲ್ಯ_bytes(apVal[0]) / ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
  ಯದಿ( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
  }
  ಯದಿ( nMatchinfo!=(2+3*nCol*nPhrase) ){
    sqlite3_result_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( nVal!=(1+nCol) ) ಗಚ್ಛ wrong_ಸಂಖ್ಯೆ_args;

  /* Iterate through each phrase in the users query. */
  ಪ್ರತಿ(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    ಪೂರ್ಣಾಙ್ಕ iCol;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    ಪೂರ್ಣಾಙ್ಕ *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
    ಪ್ರತಿ(iCol=0; iCol<nCol; iCol++){
      ಪೂರ್ಣಾಙ್ಕ n_Hit_ಎಣಿಕೆ = aPhraseinfo[3*iCol];
      ಪೂರ್ಣಾಙ್ಕ n_Global_Hit_ಎಣಿಕೆ = aPhraseinfo[3*iCol+1];
      ದ್ವಿಗುಣ weight = sqlite3_ಮೌಲ್ಯ_double(apVal[iCol+1]);
      ಯದಿ( n_Hit_ಎಣಿಕೆ>0 ){
        score += ((ದ್ವಿಗುಣ)n_Hit_ಎಣಿಕೆ / (ದ್ವಿಗುಣ)n_Global_Hit_ಎಣಿಕೆ) * weight;
      }
    }
  }

  sqlite3_result_double(pCtx, score);
  ಪ್ರತ್ಯಾಗಮ;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_ಸಂಖ್ಯೆ_args:
  sqlite3_result_error(pCtx, "wrong number of arguments to function rank()", -1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI install_fts3_rank_function(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp*, ಧ್ರುವ ವರ್ಣ*, sqlite3**);
  sqlite3 *db;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest_func_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ Md5_Register(sqlite3 *, ವರ್ಣ **, ಧ್ರುವ sqlite3_api_routines *);

  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))registerTestFunctions);
  sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))Md5_Register);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
