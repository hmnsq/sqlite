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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
ਸਥਿਰ ਖਾਲੀ *testContextMalloc(sqlite3_context *context, ਪੂਰਨ nByte){
  ਅੱਖਰ *z = sqlite3_malloc(nByte);
  ਜੇ( !z && nByte>0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(context);
  }
  ਵਾਪਸ z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
ਸਥਿਰ ਖਾਲੀ randStr(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  ਪੂਰਨ iMin, iMax, n, r, i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ(argc==2);

  iMin = sqlite3_ਮੁੱਲ_int(argv[0]);
  ਜੇ( iMin<0 ) iMin = 0;
  ਜੇ( iMin>=ਆਕਾਰ(zBuf) ) iMin = ਆਕਾਰ(zBuf)-1;
  iMax = sqlite3_ਮੁੱਲ_int(argv[1]);
  ਜੇ( iMax<iMin ) iMax = iMin;
  ਜੇ( iMax>=ਆਕਾਰ(zBuf) ) iMax = ਆਕਾਰ(zBuf)-1;
  n = iMin;
  ਜੇ( iMax>iMin ){
    sqlite3_randomness(ਆਕਾਰ(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n<ਆਕਾਰ(zBuf) );
  sqlite3_randomness(n, zBuf);
  ਲਈ(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(ਆਕਾਰ(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, (ਅੱਖਰ*)zBuf, n, SQLITE_TRANSIENT);
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
ਸਥਿਰ ਪੂਰਨ test_destructor_ਗਿਣਤੀ_var = 0;
ਸਥਿਰ ਖਾਲੀ destructor(ਖਾਲੀ *p){
  ਅੱਖਰ *zVal = (ਅੱਖਰ *)p;
  ਪ੍ਰਤਿਜ੍ਞਾ(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_ਗਿਣਤੀ_var--;
}
ਸਥਿਰ ਖਾਲੀ test_destructor(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  ਅੱਖਰ *zVal;
  ਪੂਰਨ len;
  
  test_destructor_ਗਿਣਤੀ_var++;
  ਪ੍ਰਤਿਜ੍ਞਾ( nArg==1 );
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ) ਵਾਪਸ;
  len = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ਜੇ( !zVal ){
    ਵਾਪਸ;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zVal, sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]), len);
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(pCtx, zVal, -1, destructor);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਸਥਿਰ ਖਾਲੀ test_destructor16(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  ਅੱਖਰ *zVal;
  ਪੂਰਨ len;
  
  test_destructor_ਗਿਣਤੀ_var++;
  ਪ੍ਰਤਿਜ੍ਞਾ( nArg==1 );
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ) ਵਾਪਸ;
  len = sqlite3_ਮੁੱਲ_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ਜੇ( !zVal ){
    ਵਾਪਸ;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zVal, sqlite3_ਮੁੱਲ_text16(argv[0]), len);
  sqlite3_ਨਤੀਜਾ_text16(pCtx, zVal, -1, destructor);
}
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਖਾਲੀ test_destructor_ਗਿਣਤੀ(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_ਨਤੀਜਾ_int(pCtx, test_destructor_ਗਿਣਤੀ_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
ਖਾਲੀ sqlite3BeginBenignMalloc(ਖਾਲੀ);
ਖਾਲੀ sqlite3_ਸਿਰਾ_Benign_Malloc(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3BeginBenignMalloc()
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸਿਰਾ_Benign_Malloc()
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਖਾਲੀ test_agg_errmsg16_ਕਦਮ(sqlite3_context *a, ਪੂਰਨ b,sqlite3_ਮੁੱਲ **c){
}
ਸਥਿਰ ਖਾਲੀ test_agg_errmsg16_final(sqlite3_context *ctx){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  ਅਚਲ ਖਾਲੀ *z;
  sqlite3 * db = sqlite3_context_db_handle(ctx);
  sqlite3_aggregate_context(ctx, 2048);
  z = sqlite3_errmsg16(db);
  sqlite3_ਨਤੀਜਾ_text16(ctx, z, -1, SQLITE_TRANSIENT);
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਖਾਲੀ free_test_auxdata(ਖਾਲੀ *p) {sqlite3_free(p);}
ਸਥਿਰ ਖਾਲੀ test_auxdata(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ i;
  ਅੱਖਰ *zRet = testContextMalloc(pCtx, nArg*2);
  ਜੇ( !zRet ) ਵਾਪਸ;
  memset(zRet, 0, nArg*2);
  ਲਈ(i=0; i<nArg; i++){
    ਅੱਖਰ ਅਚਲ *z = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
    ਜੇ( z ){
      ਪੂਰਨ n;
      ਅੱਖਰ *zAux = sqlite3_get_auxdata(pCtx, i);
      ਜੇ( zAux ){
        zRet[i*2] = '1';
        ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(zAux,z)==0 );
      }ਨਹੀਂਤਾਂ {
        zRet[i*2] = '0';
      }
      n = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      ਜੇ( zAux ){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(zAux, z, n);
        sqlite3_ਸੈੱਟ_auxdata(pCtx, i, zAux, free_test_auxdata);
      }
      zRet[i*2+1] = ' ';
    }
  }
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(pCtx, zRet, 2*nArg-1, free_test_auxdata);
}

/*
** A function to test error reporting from user functions. This function
** returns a copy of its first argument as the error message.  If the
** second argument exists, it becomes the error code.
*/
ਸਥਿਰ ਖਾਲੀ test_error(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_ਨਤੀਜਾ_error(pCtx, (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]), -1);
  ਜੇ( nArg==2 ){
    sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(pCtx, sqlite3_ਮੁੱਲ_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
ਸਥਿਰ ਖਾਲੀ counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  ਪੂਰਨ nArg,                /* Number of function arguments */
  sqlite3_ਮੁੱਲ **argv     /* Values for all function arguments */
){
  ਪੂਰਨ *pCounter = (ਪੂਰਨ*)sqlite3_get_auxdata(pCtx, 0);
  ਜੇ( pCounter==0 ){
    pCounter = sqlite3_malloc( ਆਕਾਰ(*pCounter) );
    ਜੇ( pCounter==0 ){
      sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
      ਵਾਪਸ;
    }
    *pCounter = sqlite3_ਮੁੱਲ_int(argv[0]);
    sqlite3_ਸੈੱਟ_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }ਨਹੀਂਤਾਂ{
    ++*pCounter;
  }
  sqlite3_ਨਤੀਜਾ_int(pCtx, *pCounter);
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
ਸਥਿਰ ਖਾਲੀ test_isolation(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  sqlite3_ਮੁੱਲ_text16(argv[0]);
  sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  sqlite3_ਮੁੱਲ_text16(argv[0]);
  sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
#ਜੇ_ਅੰਤ
  sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
ਸਥਿਰ ਖਾਲੀ test_eval(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_stmt *pStmt;
  ਪੂਰਨ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ਅਚਲ ਅੱਖਰ *zSql;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ z_ਟੈਗ[] = "test_eval()";

  ਜੇ( sqlite3_get_clientdata(db, z_ਟੈਗ) ){
    sqlite3_ਨਤੀਜਾ_error(pCtx, "nested test_eval()", -1);
    ਵਾਪਸ;
  }
  zSql = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  sqlite3_ਸੈੱਟ_clientdata(db, z_ਟੈਗ, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3_ਕਦਮ(pStmt);
    ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
      sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(pCtx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_ਸੈੱਟ_clientdata(db, z_ਟੈਗ, 0, 0);
  ਜੇ( rc ){
    ਅੱਖਰ *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(pCtx, zErr, -1, sqlite3_free);
    sqlite3_ਨਤੀਜਾ_error_ਕੋਡ(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
ਸਥਿਰ ਪੂਰਨ testHexChar(ਅੱਖਰ c){
  ਜੇ( c>='0' && c<='9' ){
    ਵਾਪਸ c - '0';
  }ਨਹੀਂਤਾਂ ਜੇ( c>='a' && c<='f' ){
    ਵਾਪਸ c - 'a' + 10;
  }ਨਹੀਂਤਾਂ ਜੇ( c>='A' && c<='F' ){
    ਵਾਪਸ c - 'A' + 10;
  }
  ਵਾਪਸ 0;
}

/*
** Convert hex to binary.
*/
ਸਥਿਰ ਖਾਲੀ test_Hex_ਮੰਜ਼ਿਲ_Bin(ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ, ਅੱਖਰ *z_ਬਾਹਰਲਾ){
  ਜਦੋਂ( z_ਵਿੱਚ[0] && z_ਵਿੱਚ[1] ){
    *(z_ਬਾਹਰਲਾ++) = (testHexChar(z_ਵਿੱਚ[0])<<4) + testHexChar(z_ਵਿੱਚ[1]);
    z_ਵਿੱਚ += 2;
  }
}

/*
**      hex_to_utf16be(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਸਥਿਰ ਖਾਲੀ test_Hex_ਮੰਜ਼ਿਲ_Utf16be(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ n;
  ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ;
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( nArg==1 );
  n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  z_ਵਿੱਚ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  z_ਬਾਹਰਲਾ = sqlite3_malloc( n/2 );
  ਜੇ( z_ਬਾਹਰਲਾ==0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
  }ਨਹੀਂਤਾਂ{
    test_Hex_ਮੰਜ਼ਿਲ_Bin(z_ਵਿੱਚ, z_ਬਾਹਰਲਾ);
    sqlite3_ਨਤੀਜਾ_text16be(pCtx, z_ਬਾਹਰਲਾ, n/2, sqlite3_free);
  }
}
#ਜੇ_ਅੰਤ

/*
**      hex_to_utf8(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
ਸਥਿਰ ਖਾਲੀ test_Hex_ਮੰਜ਼ਿਲ_Utf8(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ n;
  ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ;
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( nArg==1 );
  n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  z_ਵਿੱਚ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  z_ਬਾਹਰਲਾ = sqlite3_malloc( n/2 );
  ਜੇ( z_ਬਾਹਰਲਾ==0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
  }ਨਹੀਂਤਾਂ{
    test_Hex_ਮੰਜ਼ਿਲ_Bin(z_ਵਿੱਚ, z_ਬਾਹਰਲਾ);
    sqlite3_ਨਤੀਜਾ_ਪਾਠ(pCtx, z_ਬਾਹਰਲਾ, n/2, sqlite3_free);
  }
}

/*
**      hex_to_utf16le(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਸਥਿਰ ਖਾਲੀ test_Hex_ਮੰਜ਼ਿਲ_Utf16le(
  sqlite3_context *pCtx, 
  ਪੂਰਨ nArg,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ n;
  ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ;
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( nArg==1 );
  n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  z_ਵਿੱਚ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  z_ਬਾਹਰਲਾ = sqlite3_malloc( n/2 );
  ਜੇ( z_ਬਾਹਰਲਾ==0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(pCtx);
  }ਨਹੀਂਤਾਂ{
    test_Hex_ਮੰਜ਼ਿਲ_Bin(z_ਵਿੱਚ, z_ਬਾਹਰਲਾ);
    sqlite3_ਨਤੀਜਾ_text16le(pCtx, z_ਬਾਹਰਲਾ, n/2, sqlite3_free);
  }
}
#ਜੇ_ਅੰਤ

/*
** SQL function:   real2hex(X)
**
** If argument X is a real number, then convert it into a string which is
** the big-endian hexadecimal representation of the ieee754 encoding of
** that number.  If X is not a real number, return NULL.
*/
ਸਥਿਰ ਖਾਲੀ real2hex(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਸੰਘ {
    sqlite3_uint64 i;
    ਦੁੱਗਣਾ r;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ x[8];
  } v;
  ਅੱਖਰ z_ਬਾਹਰਲਾ[20];
  ਪੂਰਨ i;
  ਪੂਰਨ bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_ਮੁੱਲ_double(argv[0]);
  ਲਈ(i=0; i<8; i++){
    ਜੇ( bigEndian ){
      z_ਬਾਹਰਲਾ[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_ਬਾਹਰਲਾ[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }ਨਹੀਂਤਾਂ{
      z_ਬਾਹਰਲਾ[14-i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_ਬਾਹਰਲਾ[14-i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }
  }
  z_ਬਾਹਰਲਾ[16] = 0;
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, z_ਬਾਹਰਲਾ, -1, SQLITE_TRANSIENT);
}

/*
**     test_extract(record, field)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as the first argument. The
** second argument is the index of the field within that record to
** extract and return.
*/
ਸਥਿਰ ਖਾਲੀ test_ਕੱਢੋ(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_ਸਿਰਾ_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  ਪੂਰਨ iIdx;                       /* Required field */
  ਪੂਰਨ iCurrent = 0;               /* Current field */

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==2 );
  pRec = (u8*)sqlite3_ਮੁੱਲ_blob(argv[0]);
  iIdx = sqlite3_ਮੁੱਲ_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_ਸਿਰਾ_Hdr = &pRec[nHdr];

  ਲਈ(iCurrent=0; pHdr<p_ਸਿਰਾ_Hdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, ਆਕਾਰ(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ਜੇ( iCurrent==iIdx ){
      sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(context, &mem);
    }

    ਜੇ( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
ਸਥਿਰ ਖਾਲੀ test_decode(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_ਸਿਰਾ_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ਗਿਣਤੀ(pRet);

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
  pRec = (u8*)sqlite3_ਮੁੱਲ_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_ਸਿਰਾ_Hdr = &pRec[nHdr];
  ਜਦੋਂ( pHdr<p_ਸਿਰਾ_Hdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, ਆਕਾਰ(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ਚੋਣ( sqlite3_ਮੁੱਲ_type(&mem) ){
      ਹਾਲਤ SQLITE_ਪਾਠ:
        pVal = Tcl_NewStringObj((ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(&mem), -1);
        ਰੋਕੋ;

      ਹਾਲਤ SQLITE_BLOB: {
        ਅੱਖਰ hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        ਪੂਰਨ n = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(&mem);
        u8 *z = (u8*)sqlite3_ਮੁੱਲ_blob(&mem);
        ਪੂਰਨ i;
        pVal = Tcl_NewStringObj("x'", -1);
        ਲਈ(i=0; i<n; i++){
          ਅੱਖਰ hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_Append_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_Obj(pVal, hex, 0);
        }
        Tcl_Append_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_Obj(pVal, "'", 0);
        ਰੋਕੋ;
      }

      ਹਾਲਤ SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_ਮੁੱਲ_double(&mem));
        ਰੋਕੋ;

      ਹਾਲਤ SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_ਮੁੱਲ_int64(&mem));
        ਰੋਕੋ;

      ਹਾਲਤ SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        ਰੋਕੋ;

      ਮੂਲ:
        ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    ਜੇ( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
ਸਥਿਰ ਖਾਲੀ test_zeroblob(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ nZero = sqlite3_ਮੁੱਲ_int(argv[0]);
  sqlite3_ਨਤੀਜਾ_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
ਸਥਿਰ ਖਾਲੀ test_getsubtype(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_ਨਤੀਜਾ_int(context, (ਪੂਰਨ)sqlite3_ਮੁੱਲ_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
ਸਥਿਰ ਖਾਲੀ test_frombind(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_uint64 m = 0;
  ਪੂਰਨ i;
  ਲਈ(i=0; i<argc && i<63; i++){
    ਜੇ( sqlite3_ਮੁੱਲ_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_ਨਤੀਜਾ_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
ਸਥਿਰ ਖਾਲੀ test_setsubtype(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(context, argv[0]);
  sqlite3_ਨਤੀਜਾ_subtype(context, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)sqlite3_ਮੁੱਲ_int(argv[1]));
}

ਸਥਿਰ ਪੂਰਨ ਰਜਿਸਟਰ_Test_Functions(
  sqlite3 *db,
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ,
  ਅਚਲ sqlite3_api_routines *pThunk
){
  ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ nArg;
     ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ e_ਪਾਠ_Rep; /* 1: UTF-16.  0: UTF-8 */
     ਖਾਲੀ (*xFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, test_Hex_ਮੰਜ਼ਿਲ_Utf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, test_Hex_ਮੰਜ਼ਿਲ_Utf16le},
#ਜੇ_ਅੰਤ
    { "hex_to_utf8",           1, SQLITE_UTF8, test_Hex_ਮੰਜ਼ਿਲ_Utf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_ਗਿਣਤੀ},
    { "test_auxdata",         -1, SQLITE_UTF8, test_auxdata},
    { "test_error",            1, SQLITE_UTF8, test_error},
    { "test_error",            2, SQLITE_UTF8, test_error},
    { "test_eval",             1, SQLITE_UTF8, test_eval},
    { "test_isolation",        2, SQLITE_UTF8, test_isolation},
    { "test_counter",          1, SQLITE_UTF8, counterFunc},
    { "real2hex",              1, SQLITE_UTF8, real2hex},
    { "test_decode",           1, SQLITE_UTF8, test_decode},
    { "test_extract",          2, SQLITE_UTF8, test_ਕੱਢੋ},
    { "test_zeroblob",  1, SQLITE_UTF8|SQLITE_DETERMINISTIC, test_zeroblob},
    { "test_getsubtype",       1, SQLITE_UTF8, test_getsubtype},
    { "test_setsubtype",       2, SQLITE_UTF8|SQLITE_ਨਤੀਜਾ_SUBTYPE,
                                               test_setsubtype},
    { "test_frombind",        -1, SQLITE_UTF8, test_frombind},
  };
  ਪੂਰਨ i;

  ਲਈ(i=0; i<ਆਕਾਰ(aFuncs)/ਆਕਾਰ(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].e_ਪਾਠ_Rep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_ਕਦਮ, test_agg_errmsg16_final);
      
  ਵਾਪਸ SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI autoinstall_test_funcs(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਬਾਹਰੀ ਪੂਰਨ Md5_ਰਜਿਸਟਰ(sqlite3 *, ਅੱਖਰ **, ਅਚਲ sqlite3_api_routines *);
  ਪੂਰਨ rc = sqlite3_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))ਰਜਿਸਟਰ_Test_Functions);
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))Md5_ਰਜਿਸਟਰ);
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
ਸਥਿਰ ਖਾਲੀ t_ਕਦਮ(sqlite3_context *a, ਪੂਰਨ b, sqlite3_ਮੁੱਲ **c){}
ਸਥਿਰ ਖਾਲੀ tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI abuse_create_function(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp*, ਅਚਲ ਅੱਖਰ*, sqlite3**);
  sqlite3 *db;
  ਪੂਰਨ rc;
  ਪੂਰਨ mxArg;

  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_ਕਦਮ,t_ਕਦਮ,tFinal);
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_ਕਦਮ, t_ਕਦਮ, 0);
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_ਕਦਮ, 0, tFinal);
  ਜੇ( rc!=SQLITE_MISUSE) ਜਾਓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, t_ਕਦਮ, 0);
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, t_ਕਦਮ, 0, 0);
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, t_ਕਦਮ, 0, 0);
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, t_ਕਦਮ, 0, 0);
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ abuse_err;

  /* This last function registration should actually work.  Generate
  ** a no-op function (that always returns NULL) and which has the
  ** maximum-length function name and the maximum number of parameters.
  */
  sqlite3_ਹੱਦਬੰਦੀ(db, SQLITE_ਹੱਦਬੰਦੀ_FUNCTION_ARG, 1000000);
  mxArg = sqlite3_ਹੱਦਬੰਦੀ(db, SQLITE_ਹੱਦਬੰਦੀ_FUNCTION_ARG, -1);
  rc = sqlite3_create_function(db, "nullx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       mxArg, SQLITE_UTF8, 0, t_ਕਦਮ, 0, 0);
  ਜੇ( rc!=SQLITE_OK ) ਜਾਓ abuse_err;
                                
  ਵਾਪਸ TCL_OK;

abuse_err:
  Tcl_Append_ਨਤੀਜਾ(interp, "sqlite3_create_function abused test failed", 
                   (ਅੱਖਰ*)0);
  ਵਾਪਸ TCL_ERROR;
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
ਸਥਿਰ ਖਾਲੀ rankfunc(sqlite3_context *pCtx, ਪੂਰਨ nVal, sqlite3_ਮੁੱਲ **apVal){
  ਪੂਰਨ *aMatchinfo;                /* Return value of matchinfo() */
  ਪੂਰਨ nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  ਪੂਰਨ n_ਖਾਨਾ = 0;                   /* Number of columns in the table */
  ਪੂਰਨ nPhrase = 0;                /* Number of phrases in the query */
  ਪੂਰਨ iPhrase;                    /* Current phrase */
  ਦੁੱਗਣਾ score = 0.0;             /* Value to return */

  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(ਪੂਰਨ)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  ਜੇ( nVal<1 ) ਜਾਓ wrong_ਸੰਖਿਆ_ਦਲੀਲਾਂ;
  aMatchinfo = (ਪੂਰਨ*)sqlite3_ਮੁੱਲ_blob(apVal[0]);
  nMatchinfo = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(apVal[0]) / ਆਕਾਰ(ਪੂਰਨ);
  ਜੇ( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    n_ਖਾਨਾ = aMatchinfo[1];
  }
  ਜੇ( nMatchinfo!=(2+3*n_ਖਾਨਾ*nPhrase) ){
    sqlite3_ਨਤੀਜਾ_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    ਵਾਪਸ;
  }
  ਜੇ( nVal!=(1+n_ਖਾਨਾ) ) ਜਾਓ wrong_ਸੰਖਿਆ_ਦਲੀਲਾਂ;

  /* Iterate through each phrase in the users query. */
  ਲਈ(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    ਪੂਰਨ i_ਖਾਨਾ;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    ਪੂਰਨ *aPhraseinfo = &aMatchinfo[2 + iPhrase*n_ਖਾਨਾ*3];
    ਲਈ(i_ਖਾਨਾ=0; i_ਖਾਨਾ<n_ਖਾਨਾ; i_ਖਾਨਾ++){
      ਪੂਰਨ n_ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ = aPhraseinfo[3*i_ਖਾਨਾ];
      ਪੂਰਨ n_Global_ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ = aPhraseinfo[3*i_ਖਾਨਾ+1];
      ਦੁੱਗਣਾ ਭਾਰ = sqlite3_ਮੁੱਲ_double(apVal[i_ਖਾਨਾ+1]);
      ਜੇ( n_ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ>0 ){
        score += ((ਦੁੱਗਣਾ)n_ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ / (ਦੁੱਗਣਾ)n_Global_ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ) * ਭਾਰ;
      }
    }
  }

  sqlite3_ਨਤੀਜਾ_double(pCtx, score);
  ਵਾਪਸ;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_ਸੰਖਿਆ_ਦਲੀਲਾਂ:
  sqlite3_ਨਤੀਜਾ_error(pCtx, "wrong number of arguments to function rank()", -1);
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI install_fts3_rank_function(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp*, ਅਚਲ ਅੱਖਰ*, sqlite3**);
  sqlite3 *db;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  ਵਾਪਸ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest_func_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  ਪੂਰਨ i;
  ਬਾਹਰੀ ਪੂਰਨ Md5_ਰਜਿਸਟਰ(sqlite3 *, ਅੱਖਰ **, ਅਚਲ sqlite3_api_routines *);

  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))ਰਜਿਸਟਰ_Test_Functions);
  sqlite3_auto_extension((ਖਾਲੀ(*)(ਖਾਲੀ))Md5_ਰਜਿਸਟਰ);
  ਵਾਪਸ TCL_OK;
}
