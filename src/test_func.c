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
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
ସ୍ଥିର ଶୂନ୍ୟ *testContextMalloc(sqlite3_context *context, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){
  ଅକ୍ଷର *z = sqlite3_malloc(nByte);
  ଯଦି( !z && nByte>0 ){
    sqlite3_result_error_nomem(context);
  }
  ଫେରନ୍ତୁ z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
ସ୍ଥିର ଶୂନ୍ୟ randStr(sqlite3_context *context, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv){
  ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iMin, iMax, n, r, i;
  unsigned ଅକ୍ଷର zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  ପ୍ରତିଜ୍ଞା(argc==2);

  iMin = sqlite3_ମୂଲ୍ୟ_int(argv[0]);
  ଯଦି( iMin<0 ) iMin = 0;
  ଯଦି( iMin>=ଆକାର(zBuf) ) iMin = ଆକାର(zBuf)-1;
  iMax = sqlite3_ମୂଲ୍ୟ_int(argv[1]);
  ଯଦି( iMax<iMin ) iMax = iMin;
  ଯଦି( iMax>=ଆକାର(zBuf) ) iMax = ଆକାର(zBuf)-1;
  n = iMin;
  ଯଦି( iMax>iMin ){
    sqlite3_randomness(ଆକାର(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  ପ୍ରତିଜ୍ଞା( n<ଆକାର(zBuf) );
  sqlite3_randomness(n, zBuf);
  ପାଇଁ(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(ଆକାର(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_result_text(context, (ଅକ୍ଷର*)zBuf, n, SQLITE_TRANSIENT);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_destructor_ଗଣନା_var = 0;
ସ୍ଥିର ଶୂନ୍ୟ destructor(ଶୂନ୍ୟ *p){
  ଅକ୍ଷର *zVal = (ଅକ୍ଷର *)p;
  ପ୍ରତିଜ୍ଞା(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_ଗଣନା_var--;
}
ସ୍ଥିର ଶୂନ୍ୟ test_destructor(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଅକ୍ଷର *zVal;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
  
  test_destructor_ଗଣନା_var++;
  ପ୍ରତିଜ୍ଞା( nArg==1 );
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[0])==SQLITE_NULL ) ଫେରନ୍ତୁ;
  len = sqlite3_ମୂଲ୍ୟ_bytes(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ଯଦି( !zVal ){
    ଫେରନ୍ତୁ;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_ମୂଲ୍ୟ_text(argv[0]), len);
  sqlite3_result_text(pCtx, zVal, -1, destructor);
}
#ifndef SQLITE_OMIT_UTF16
ସ୍ଥିର ଶୂନ୍ୟ test_destructor16(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ଅକ୍ଷର *zVal;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
  
  test_destructor_ଗଣନା_var++;
  ପ୍ରତିଜ୍ଞା( nArg==1 );
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[0])==SQLITE_NULL ) ଫେରନ୍ତୁ;
  len = sqlite3_ମୂଲ୍ୟ_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ଯଦି( !zVal ){
    ଫେରନ୍ତୁ;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_ମୂଲ୍ୟ_text16(argv[0]), len);
  sqlite3_result_text16(pCtx, zVal, -1, destructor);
}
#endif
ସ୍ଥିର ଶୂନ୍ୟ test_destructor_ଗଣନା(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3_result_int(pCtx, test_destructor_ଗଣନା_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ifndef SQLITE_UNTESTABLE
ଶୂନ୍ୟ sqlite3BeginBenignMalloc(ଶୂନ୍ୟ);
ଶୂନ୍ୟ sqlite3EndBenignMalloc(ଶୂନ୍ୟ);
#ଅନ୍ୟଥା
  #define sqlite3BeginBenignMalloc()
  #define sqlite3EndBenignMalloc()
#endif
ସ୍ଥିର ଶୂନ୍ୟ test_agg_errmsg16_step(sqlite3_context *a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b,sqlite3_ମୂଲ୍ୟ **c){
}
ସ୍ଥିର ଶୂନ୍ୟ test_agg_errmsg16_final(sqlite3_context *ctx){
#ifndef SQLITE_OMIT_UTF16
  ଅଚଳ ଶୂନ୍ୟ *z;
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
ସ୍ଥିର ଶୂନ୍ୟ free_test_auxdata(ଶୂନ୍ୟ *p) {sqlite3_free(p);}
ସ୍ଥିର ଶୂନ୍ୟ test_auxdata(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅକ୍ଷର *zRet = testContextMalloc(pCtx, nArg*2);
  ଯଦି( !zRet ) ଫେରନ୍ତୁ;
  memset(zRet, 0, nArg*2);
  ପାଇଁ(i=0; i<nArg; i++){
    ଅକ୍ଷର ଅଚଳ *z = (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
    ଯଦି( z ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
      ଅକ୍ଷର *zAux = sqlite3_get_auxdata(pCtx, i);
      ଯଦି( zAux ){
        zRet[i*2] = '1';
        ପ୍ରତିଜ୍ଞା( strcmp(zAux,z)==0 );
      }ଅନ୍ୟଥା {
        zRet[i*2] = '0';
      }
      n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      ଯଦି( zAux ){
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
ସ୍ଥିର ଶୂନ୍ୟ test_error(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3_result_error(pCtx, (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]), -1);
  ଯଦି( nArg==2 ){
    sqlite3_result_error_code(pCtx, sqlite3_ମୂଲ୍ୟ_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
ସ୍ଥିର ଶୂନ୍ୟ counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,                /* Number of function arguments */
  sqlite3_ମୂଲ୍ୟ **argv     /* Values for all function arguments */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pCounter = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)sqlite3_get_auxdata(pCtx, 0);
  ଯଦି( pCounter==0 ){
    pCounter = sqlite3_malloc( ଆକାର(*pCounter) );
    ଯଦି( pCounter==0 ){
      sqlite3_result_error_nomem(pCtx);
      ଫେରନ୍ତୁ;
    }
    *pCounter = sqlite3_ମୂଲ୍ୟ_int(argv[0]);
    sqlite3_set_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ test_isolation(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3_ମୂଲ୍ୟ_text16(argv[0]);
  sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  sqlite3_ମୂଲ୍ୟ_text16(argv[0]);
  sqlite3_ମୂଲ୍ୟ_text(argv[0]);
#endif
  sqlite3_result_ମୂଲ୍ୟ(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
ସ୍ଥିର ଶୂନ୍ୟ test_eval(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3_stmt *pStmt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ଅଚଳ ଅକ୍ଷର *zSql;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର zTag[] = "test_eval()";

  ଯଦି( sqlite3_get_clientdata(db, zTag) ){
    sqlite3_result_error(pCtx, "nested test_eval()", -1);
    ଫେରନ୍ତୁ;
  }
  zSql = (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  sqlite3_set_clientdata(db, zTag, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3_step(pStmt);
    ଯଦି( rc==SQLITE_ROW ){
      sqlite3_result_ମୂଲ୍ୟ(pCtx, sqlite3_column_ମୂଲ୍ୟ(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_set_clientdata(db, zTag, 0, 0);
  ଯଦି( rc ){
    ଅକ୍ଷର *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_result_text(pCtx, zErr, -1, sqlite3_free);
    sqlite3_result_error_code(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା testHexChar(ଅକ୍ଷର c){
  ଯଦି( c>='0' && c<='9' ){
    ଫେରନ୍ତୁ c - '0';
  }ଅନ୍ୟଥା ଯଦି( c>='a' && c<='f' ){
    ଫେରନ୍ତୁ c - 'a' + 10;
  }ଅନ୍ୟଥା ଯଦି( c>='A' && c<='F' ){
    ଫେରନ୍ତୁ c - 'A' + 10;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Convert hex to binary.
*/
ସ୍ଥିର ଶୂନ୍ୟ testHexToBin(ଅଚଳ ଅକ୍ଷର *zIn, ଅକ୍ଷର *zOut){
  ଯେତେବେଳେ( zIn[0] && zIn[1] ){
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
ସ୍ଥିର ଶୂନ୍ୟ testHexToUtf16be(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ଅଚଳ ଅକ୍ଷର *zIn;
  ଅକ୍ଷର *zOut;
  ପ୍ରତିଜ୍ଞା( nArg==1 );
  n = sqlite3_ମୂଲ୍ୟ_bytes(argv[0]);
  zIn = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  ଯଦି( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ testHexToUtf8(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ଅଚଳ ଅକ୍ଷର *zIn;
  ଅକ୍ଷର *zOut;
  ପ୍ରତିଜ୍ଞା( nArg==1 );
  n = sqlite3_ମୂଲ୍ୟ_bytes(argv[0]);
  zIn = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  ଯଦି( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ testHexToUtf16le(
  sqlite3_context *pCtx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ଅଚଳ ଅକ୍ଷର *zIn;
  ଅକ୍ଷର *zOut;
  ପ୍ରତିଜ୍ଞା( nArg==1 );
  n = sqlite3_ମୂଲ୍ୟ_bytes(argv[0]);
  zIn = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  ଯଦି( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ real2hex(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ସଂଘ {
    sqlite3_uint64 i;
    ଦ୍ୱିଗୁଣ r;
    unsigned ଅକ୍ଷର x[8];
  } v;
  ଅକ୍ଷର zOut[20];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_ମୂଲ୍ୟ_double(argv[0]);
  ପାଇଁ(i=0; i<8; i++){
    ଯଦି( bigEndian ){
      zOut[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ test_extract(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdx;                       /* Required field */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCurrent = 0;               /* Current field */

  ପ୍ରତିଜ୍ଞା( argc==2 );
  pRec = (u8*)sqlite3_ମୂଲ୍ୟ_blob(argv[0]);
  iIdx = sqlite3_ମୂଲ୍ୟ_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];

  ପାଇଁ(iCurrent=0; pHdr<pEndHdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, ଆକାର(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ଯଦି( iCurrent==iIdx ){
      sqlite3_result_ମୂଲ୍ୟ(context, &mem);
    }

    ଯଦି( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
ସ୍ଥିର ଶୂନ୍ୟ test_decode(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ଗଣନା(pRet);

  ପ୍ରତିଜ୍ଞା( argc==1 );
  pRec = (u8*)sqlite3_ମୂଲ୍ୟ_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];
  ଯେତେବେଳେ( pHdr<pEndHdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, ଆକାର(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ବାଛ( sqlite3_ମୂଲ୍ୟ_type(&mem) ){
      ସ୍ଥିତି SQLITE_TEXT:
        pVal = Tcl_NewStringObj((ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(&mem), -1);
        ଭାଙ୍ଗ;

      ସ୍ଥିତି SQLITE_BLOB: {
        ଅକ୍ଷର hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = sqlite3_ମୂଲ୍ୟ_bytes(&mem);
        u8 *z = (u8*)sqlite3_ମୂଲ୍ୟ_blob(&mem);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        pVal = Tcl_NewStringObj("x'", -1);
        ପାଇଁ(i=0; i<n; i++){
          ଅକ୍ଷର hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_AppendStringsToObj(pVal, hex, 0);
        }
        Tcl_AppendStringsToObj(pVal, "'", 0);
        ଭାଙ୍ଗ;
      }

      ସ୍ଥିତି SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_ମୂଲ୍ୟ_double(&mem));
        ଭାଙ୍ଗ;

      ସ୍ଥିତି SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_ମୂଲ୍ୟ_int64(&mem));
        ଭାଙ୍ଗ;

      ସ୍ଥିତି SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        ଭାଙ୍ଗ;

      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ପ୍ରତିଜ୍ଞା( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    ଯଦି( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_result_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_ଗଣନା(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
ସ୍ଥିର ଶୂନ୍ୟ test_zeroblob(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nZero = sqlite3_ମୂଲ୍ୟ_int(argv[0]);
  sqlite3_result_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
ସ୍ଥିର ଶୂନ୍ୟ test_getsubtype(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3_result_int(context, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sqlite3_ମୂଲ୍ୟ_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
ସ୍ଥିର ଶୂନ୍ୟ test_frombind(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3_uint64 m = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<argc && i<63; i++){
    ଯଦି( sqlite3_ମୂଲ୍ୟ_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_result_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
ସ୍ଥିର ଶୂନ୍ୟ test_setsubtype(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3_result_ମୂଲ୍ୟ(context, argv[0]);
  sqlite3_result_subtype(context, (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sqlite3_ମୂଲ୍ୟ_int(argv[1]));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା registerTestFunctions(
  sqlite3 *db,
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ,
  ଅଚଳ sqlite3_api_routines *pThunk
){
  ସ୍ଥିର ଅଚଳ ସଂରଚନା {
     ଅକ୍ଷର *zName;
     signed ଅକ୍ଷର nArg;
     unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା eTextRep; /* 1: UTF-16.  0: UTF-8 */
     ଶୂନ୍ୟ (*xFunc)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ifndef SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, testHexToUtf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, testHexToUtf16le},
#endif
    { "hex_to_utf8",           1, SQLITE_UTF8, testHexToUtf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_ଗଣନା},
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; i<ଆକାର(aFuncs)/ଆକାର(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].eTextRep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_step, test_agg_errmsg16_final);
      
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI autoinstall_test_funcs(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Md5_Register(sqlite3 *, ଅକ୍ଷର **, ଅଚଳ sqlite3_api_routines *);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))registerTestFunctions);
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))Md5_Register);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
ସ୍ଥିର ଶୂନ୍ୟ tStep(sqlite3_context *a, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b, sqlite3_ମୂଲ୍ୟ **c){}
ସ୍ଥିର ଶୂନ୍ୟ tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI abuse_create_function(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp*, ଅଚଳ ଅକ୍ଷର*, sqlite3**);
  sqlite3 *db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mxArg;

  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep,tStep,tFinal);
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, tStep, 0);
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, 0, tFinal);
  ଯଦି( rc!=SQLITE_MISUSE) ଯାଅ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, tStep, 0);
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, tStep, 0, 0);
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, tStep, 0, 0);
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, tStep, 0, 0);
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ abuse_err;

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
  ଯଦି( rc!=SQLITE_OK ) ଯାଅ abuse_err;
                                
  ଫେରନ୍ତୁ TCL_OK;

abuse_err:
  Tcl_AppendResult(interp, "sqlite3_create_function abused test failed", 
                   (ଅକ୍ଷର*)0);
  ଫେରନ୍ତୁ TCL_ERROR;
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
ସ୍ଥିର ଶୂନ୍ୟ rankfunc(sqlite3_context *pCtx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVal, sqlite3_ମୂଲ୍ୟ **apVal){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aMatchinfo;                /* Return value of matchinfo() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol = 0;                   /* Number of columns in the table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPhrase = 0;                /* Number of phrases in the query */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPhrase;                    /* Current phrase */
  ଦ୍ୱିଗୁଣ score = 0.0;             /* Value to return */

  ପ୍ରତିଜ୍ଞା( ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  ଯଦି( nVal<1 ) ଯାଅ wrong_ସଂଖ୍ୟା_args;
  aMatchinfo = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)sqlite3_ମୂଲ୍ୟ_blob(apVal[0]);
  nMatchinfo = sqlite3_ମୂଲ୍ୟ_bytes(apVal[0]) / ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଯଦି( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
  }
  ଯଦି( nMatchinfo!=(2+3*nCol*nPhrase) ){
    sqlite3_result_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    ଫେରନ୍ତୁ;
  }
  ଯଦି( nVal!=(1+nCol) ) ଯାଅ wrong_ସଂଖ୍ୟା_args;

  /* Iterate through each phrase in the users query. */
  ପାଇଁ(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
    ପାଇଁ(iCol=0; iCol<nCol; iCol++){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_Hit_ଗଣନା = aPhraseinfo[3*iCol];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_Global_Hit_ଗଣନା = aPhraseinfo[3*iCol+1];
      ଦ୍ୱିଗୁଣ weight = sqlite3_ମୂଲ୍ୟ_double(apVal[iCol+1]);
      ଯଦି( n_Hit_ଗଣନା>0 ){
        score += ((ଦ୍ୱିଗୁଣ)n_Hit_ଗଣନା / (ଦ୍ୱିଗୁଣ)n_Global_Hit_ଗଣନା) * weight;
      }
    }
  }

  sqlite3_result_double(pCtx, score);
  ଫେରନ୍ତୁ;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_ସଂଖ୍ୟା_args:
  sqlite3_result_error(pCtx, "wrong number of arguments to function rank()", -1);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI install_fts3_rank_function(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp*, ଅଚଳ ଅକ୍ଷର*, sqlite3**);
  sqlite3 *db;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_func_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା Md5_Register(sqlite3 *, ଅକ୍ଷର **, ଅଚଳ sqlite3_api_routines *);

  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))registerTestFunctions);
  sqlite3_auto_extension((ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))Md5_Register);
  ଫେରନ୍ତୁ TCL_OK;
}
