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
ಸ್ಥಿರ ಶೂನ್ಯ *testContextMalloc(sqlite3_context *context, ಪೂರ್ಣಾಂಕ nByte){
  ಅಕ್ಷರ *z = sqlite3_malloc(nByte);
  ಆದರೆ( !z && nByte>0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
  }
  ಹಿಂತಿರುಗಿ z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
ಸ್ಥಿರ ಶೂನ್ಯ randStr(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  ಪೂರ್ಣಾಂಕ iMin, iMax, n, r, i;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  ಪ್ರತಿಜ್ಞಾ(argc==2);

  iMin = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  ಆದರೆ( iMin<0 ) iMin = 0;
  ಆದರೆ( iMin>=ಗಾತ್ರ(zBuf) ) iMin = ಗಾತ್ರ(zBuf)-1;
  iMax = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  ಆದರೆ( iMax<iMin ) iMax = iMin;
  ಆದರೆ( iMax>=ಗಾತ್ರ(zBuf) ) iMax = ಗಾತ್ರ(zBuf)-1;
  n = iMin;
  ಆದರೆ( iMax>iMin ){
    sqlite3_randomness(ಗಾತ್ರ(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  ಪ್ರತಿಜ್ಞಾ( n<ಗಾತ್ರ(zBuf) );
  sqlite3_randomness(n, zBuf);
  ಗಾಗಿ(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(ಗಾತ್ರ(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, (ಅಕ್ಷರ*)zBuf, n, SQLITE_TRANSIENT);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_destructor_ಎಣಿಕೆ_var = 0;
ಸ್ಥಿರ ಶೂನ್ಯ destructor(ಶೂನ್ಯ *p){
  ಅಕ್ಷರ *zVal = (ಅಕ್ಷರ *)p;
  ಪ್ರತಿಜ್ಞಾ(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_ಎಣಿಕೆ_var--;
}
ಸ್ಥಿರ ಶೂನ್ಯ test_destructor(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಅಕ್ಷರ *zVal;
  ಪೂರ್ಣಾಂಕ len;
  
  test_destructor_ಎಣಿಕೆ_var++;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  len = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ಆದರೆ( !zVal ){
    ಹಿಂತಿರುಗಿ;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  ಸ್ಮರಣೆ_ನಕಲು(zVal, sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]), len);
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pCtx, zVal, -1, destructor);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರ ಶೂನ್ಯ test_destructor16(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಅಕ್ಷರ *zVal;
  ಪೂರ್ಣಾಂಕ len;
  
  test_destructor_ಎಣಿಕೆ_var++;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ) ಹಿಂತಿರುಗಿ;
  len = sqlite3_ಮೌಲ್ಯ_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ಆದರೆ( !zVal ){
    ಹಿಂತಿರುಗಿ;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  ಸ್ಮರಣೆ_ನಕಲು(zVal, sqlite3_ಮೌಲ್ಯ_text16(argv[0]), len);
  sqlite3_ಫಲಿತಾಂಶ_text16(pCtx, zVal, -1, destructor);
}
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಶೂನ್ಯ test_destructor_ಎಣಿಕೆ(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_ಫಲಿತಾಂಶ_int(pCtx, test_destructor_ಎಣಿಕೆ_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
ಶೂನ್ಯ sqlite3BeginBenignMalloc(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3_ಕೊನೆ_Benign_Malloc(ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3BeginBenignMalloc()
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಕೊನೆ_Benign_Malloc()
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಶೂನ್ಯ test_agg_errmsg16_ಹೆಜ್ಜೆ(sqlite3_context *a, ಪೂರ್ಣಾಂಕ b,sqlite3_ಮೌಲ್ಯ **c){
}
ಸ್ಥಿರ ಶೂನ್ಯ test_agg_errmsg16_final(sqlite3_context *ctx){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z;
  sqlite3 * db = sqlite3_context_db_handle(ctx);
  sqlite3_aggregate_context(ctx, 2048);
  z = sqlite3_errmsg16(db);
  sqlite3_ಫಲಿತಾಂಶ_text16(ctx, z, -1, SQLITE_TRANSIENT);
#ಆದರೆ_ಅಂತ್ಯ
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
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ i;
  ಅಕ್ಷರ *zRet = testContextMalloc(pCtx, nArg*2);
  ಆದರೆ( !zRet ) ಹಿಂತಿರುಗಿ;
  memset(zRet, 0, nArg*2);
  ಗಾಗಿ(i=0; i<nArg; i++){
    ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *z = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
    ಆದರೆ( z ){
      ಪೂರ್ಣಾಂಕ n;
      ಅಕ್ಷರ *zAux = sqlite3_get_auxdata(pCtx, i);
      ಆದರೆ( zAux ){
        zRet[i*2] = '1';
        ಪ್ರತಿಜ್ಞಾ( strcmp(zAux,z)==0 );
      }ಇಲ್ಲದಿದ್ದರೆ {
        zRet[i*2] = '0';
      }
      n = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      ಆದರೆ( zAux ){
        ಸ್ಮರಣೆ_ನಕಲು(zAux, z, n);
        sqlite3_ಗಣ_auxdata(pCtx, i, zAux, free_test_auxdata);
      }
      zRet[i*2+1] = ' ';
    }
  }
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pCtx, zRet, 2*nArg-1, free_test_auxdata);
}

/*
** A function to test error reporting from user functions. This function
** returns a copy of its first argument as the error message.  If the
** second argument exists, it becomes the error code.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_error(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_ಫಲಿತಾಂಶ_error(pCtx, (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]), -1);
  ಆದರೆ( nArg==2 ){
    sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(pCtx, sqlite3_ಮೌಲ್ಯ_int(argv[1]));
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
  ಪೂರ್ಣಾಂಕ nArg,                /* Number of function arguments */
  sqlite3_ಮೌಲ್ಯ **argv     /* Values for all function arguments */
){
  ಪೂರ್ಣಾಂಕ *pCounter = (ಪೂರ್ಣಾಂಕ*)sqlite3_get_auxdata(pCtx, 0);
  ಆದರೆ( pCounter==0 ){
    pCounter = sqlite3_malloc( ಗಾತ್ರ(*pCounter) );
    ಆದರೆ( pCounter==0 ){
      sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
      ಹಿಂತಿರುಗಿ;
    }
    *pCounter = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
    sqlite3_ಗಣ_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ++*pCounter;
  }
  sqlite3_ಫಲಿತಾಂಶ_int(pCtx, *pCounter);
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
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  sqlite3_ಮೌಲ್ಯ_text16(argv[0]);
  sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  sqlite3_ಮೌಲ್ಯ_text16(argv[0]);
  sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_eval(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ z_ಟ್ಯಾಗ್[] = "test_eval()";

  ಆದರೆ( sqlite3_get_clientdata(db, z_ಟ್ಯಾಗ್) ){
    sqlite3_ಫಲಿತಾಂಶ_error(pCtx, "nested test_eval()", -1);
    ಹಿಂತಿರುಗಿ;
  }
  zSql = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  sqlite3_ಗಣ_clientdata(db, z_ಟ್ಯಾಗ್, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3_ಹೆಜ್ಜೆ(pStmt);
    ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
      sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(pCtx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_ಗಣ_clientdata(db, z_ಟ್ಯಾಗ್, 0, 0);
  ಆದರೆ( rc ){
    ಅಕ್ಷರ *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pCtx, zErr, -1, sqlite3_free);
    sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ testHexChar(ಅಕ್ಷರ c){
  ಆದರೆ( c>='0' && c<='9' ){
    ಹಿಂತಿರುಗಿ c - '0';
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c>='a' && c<='f' ){
    ಹಿಂತಿರುಗಿ c - 'a' + 10;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c>='A' && c<='F' ){
    ಹಿಂತಿರುಗಿ c - 'A' + 10;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Convert hex to binary.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_Hex_ಗಮ್ಯ_Bin(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ, ಅಕ್ಷರ *z_ಹೊರಹೋಗು){
  ಆಗ( z_ಒಳ[0] && z_ಒಳ[1] ){
    *(z_ಹೊರಹೋಗು++) = (testHexChar(z_ಒಳ[0])<<4) + testHexChar(z_ಒಳ[1]);
    z_ಒಳ += 2;
  }
}

/*
**      hex_to_utf16be(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರ ಶೂನ್ಯ test_Hex_ಗಮ್ಯ_Utf16be(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ n;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ;
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  z_ಹೊರಹೋಗು = sqlite3_malloc( n/2 );
  ಆದರೆ( z_ಹೊರಹೋಗು==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    test_Hex_ಗಮ್ಯ_Bin(z_ಒಳ, z_ಹೊರಹೋಗು);
    sqlite3_ಫಲಿತಾಂಶ_text16be(pCtx, z_ಹೊರಹೋಗು, n/2, sqlite3_free);
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/*
**      hex_to_utf8(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_Hex_ಗಮ್ಯ_Utf8(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ n;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ;
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  z_ಹೊರಹೋಗು = sqlite3_malloc( n/2 );
  ಆದರೆ( z_ಹೊರಹೋಗು==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    test_Hex_ಗಮ್ಯ_Bin(z_ಒಳ, z_ಹೊರಹೋಗು);
    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pCtx, z_ಹೊರಹೋಗು, n/2, sqlite3_free);
  }
}

/*
**      hex_to_utf16le(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಸ್ಥಿರ ಶೂನ್ಯ test_Hex_ಗಮ್ಯ_Utf16le(
  sqlite3_context *pCtx, 
  ಪೂರ್ಣಾಂಕ nArg,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ n;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ;
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;
  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  z_ಒಳ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  z_ಹೊರಹೋಗು = sqlite3_malloc( n/2 );
  ಆದರೆ( z_ಹೊರಹೋಗು==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(pCtx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    test_Hex_ಗಮ್ಯ_Bin(z_ಒಳ, z_ಹೊರಹೋಗು);
    sqlite3_ಫಲಿತಾಂಶ_text16le(pCtx, z_ಹೊರಹೋಗು, n/2, sqlite3_free);
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** SQL function:   real2hex(X)
**
** If argument X is a real number, then convert it into a string which is
** the big-endian hexadecimal representation of the ieee754 encoding of
** that number.  If X is not a real number, return NULL.
*/
ಸ್ಥಿರ ಶೂನ್ಯ real2hex(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಒಕ್ಕೂಟ {
    sqlite3_uint64 i;
    ದ್ವಿಗುಣ r;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ x[8];
  } v;
  ಅಕ್ಷರ z_ಹೊರಹೋಗು[20];
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_ಮೌಲ್ಯ_double(argv[0]);
  ಗಾಗಿ(i=0; i<8; i++){
    ಆದರೆ( bigEndian ){
      z_ಹೊರಹೋಗು[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_ಹೊರಹೋಗು[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }ಇಲ್ಲದಿದ್ದರೆ{
      z_ಹೊರಹೋಗು[14-i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_ಹೊರಹೋಗು[14-i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }
  }
  z_ಹೊರಹೋಗು[16] = 0;
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, z_ಹೊರಹೋಗು, -1, SQLITE_TRANSIENT);
}

/*
**     test_extract(record, field)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as the first argument. The
** second argument is the index of the field within that record to
** extract and return.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_ಹೊರತೆಗೆ(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_ಕೊನೆ_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  ಪೂರ್ಣಾಂಕ iIdx;                       /* Required field */
  ಪೂರ್ಣಾಂಕ iCurrent = 0;               /* Current field */

  ಪ್ರತಿಜ್ಞಾ( argc==2 );
  pRec = (u8*)sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
  iIdx = sqlite3_ಮೌಲ್ಯ_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_ಕೊನೆ_Hdr = &pRec[nHdr];

  ಗಾಗಿ(iCurrent=0; pHdr<p_ಕೊನೆ_Hdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, ಗಾತ್ರ(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ಆದರೆ( iCurrent==iIdx ){
      sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(context, &mem);
    }

    ಆದರೆ( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_ಕೊನೆ_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRet);

  ಪ್ರತಿಜ್ಞಾ( argc==1 );
  pRec = (u8*)sqlite3_ಮೌಲ್ಯ_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_ಕೊನೆ_Hdr = &pRec[nHdr];
  ಆಗ( pHdr<p_ಕೊನೆ_Hdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, ಗಾತ್ರ(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type(&mem) ){
      ಸ್ಥಿತಿ SQLITE_ಪಠ್ಯ:
        pVal = Tcl_NewStringObj((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(&mem), -1);
        ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ SQLITE_BLOB: {
        ಅಕ್ಷರ hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        ಪೂರ್ಣಾಂಕ n = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(&mem);
        u8 *z = (u8*)sqlite3_ಮೌಲ್ಯ_blob(&mem);
        ಪೂರ್ಣಾಂಕ i;
        pVal = Tcl_NewStringObj("x'", -1);
        ಗಾಗಿ(i=0; i<n; i++){
          ಅಕ್ಷರ hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_Append_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_Obj(pVal, hex, 0);
        }
        Tcl_Append_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_Obj(pVal, "'", 0);
        ನಿಲ್ಲಿಸು;
      }

      ಸ್ಥಿತಿ SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_ಮೌಲ್ಯ_double(&mem));
        ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_ಮೌಲ್ಯ_int64(&mem));
        ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        ನಿಲ್ಲಿಸು;

      ಪೂರ್ವನಿಯೋಜಿತ:
        ಪ್ರತಿಜ್ಞಾ( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    ಆದರೆ( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ nZero = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  sqlite3_ಫಲಿತಾಂಶ_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_getsubtype(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_ಫಲಿತಾಂಶ_int(context, (ಪೂರ್ಣಾಂಕ)sqlite3_ಮೌಲ್ಯ_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_frombind(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_uint64 m = 0;
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<argc && i<63; i++){
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_ಫಲಿತಾಂಶ_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
ಸ್ಥಿರ ಶೂನ್ಯ test_setsubtype(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(context, argv[0]);
  sqlite3_ಫಲಿತಾಂಶ_subtype(context, (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)sqlite3_ಮೌಲ್ಯ_int(argv[1]));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ರಿಜಿಸ್ಟರ್_Test_Functions(
  sqlite3 *db,
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ,
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pThunk
){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ nArg;
     ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ e_ಪಠ್ಯ_Rep; /* 1: UTF-16.  0: UTF-8 */
     ಶೂನ್ಯ (*xFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, test_Hex_ಗಮ್ಯ_Utf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, test_Hex_ಗಮ್ಯ_Utf16le},
#ಆದರೆ_ಅಂತ್ಯ
    { "hex_to_utf8",           1, SQLITE_UTF8, test_Hex_ಗಮ್ಯ_Utf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_ಎಣಿಕೆ},
    { "test_auxdata",         -1, SQLITE_UTF8, test_auxdata},
    { "test_error",            1, SQLITE_UTF8, test_error},
    { "test_error",            2, SQLITE_UTF8, test_error},
    { "test_eval",             1, SQLITE_UTF8, test_eval},
    { "test_isolation",        2, SQLITE_UTF8, test_isolation},
    { "test_counter",          1, SQLITE_UTF8, counterFunc},
    { "real2hex",              1, SQLITE_UTF8, real2hex},
    { "test_decode",           1, SQLITE_UTF8, test_decode},
    { "test_extract",          2, SQLITE_UTF8, test_ಹೊರತೆಗೆ},
    { "test_zeroblob",  1, SQLITE_UTF8|SQLITE_DETERMINISTIC, test_zeroblob},
    { "test_getsubtype",       1, SQLITE_UTF8, test_getsubtype},
    { "test_setsubtype",       2, SQLITE_UTF8|SQLITE_ಫಲಿತಾಂಶ_SUBTYPE,
                                               test_setsubtype},
    { "test_frombind",        -1, SQLITE_UTF8, test_frombind},
  };
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aFuncs)/ಗಾತ್ರ(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].e_ಪಠ್ಯ_Rep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_ಹೆಜ್ಜೆ, test_agg_errmsg16_final);
      
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI autoinstall_test_funcs(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ Md5_ರಿಜಿಸ್ಟರ್(sqlite3 *, ಅಕ್ಷರ **, ಸ್ಥಿರಾಂಕ sqlite3_api_routines *);
  ಪೂರ್ಣಾಂಕ rc = sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))ರಿಜಿಸ್ಟರ್_Test_Functions);
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))Md5_ರಿಜಿಸ್ಟರ್);
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ t_ಹೆಜ್ಜೆ(sqlite3_context *a, ಪೂರ್ಣಾಂಕ b, sqlite3_ಮೌಲ್ಯ **c){}
ಸ್ಥಿರ ಶೂನ್ಯ tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI abuse_create_function(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, sqlite3**);
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ mxArg;

  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_ಹೆಜ್ಜೆ,t_ಹೆಜ್ಜೆ,tFinal);
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_ಹೆಜ್ಜೆ, t_ಹೆಜ್ಜೆ, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_ಹೆಜ್ಜೆ, 0, tFinal);
  ಆದರೆ( rc!=SQLITE_MISUSE) ಹೋಗು abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, t_ಹೆಜ್ಜೆ, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, t_ಹೆಜ್ಜೆ, 0, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, t_ಹೆಜ್ಜೆ, 0, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, t_ಹೆಜ್ಜೆ, 0, 0);
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು abuse_err;

  /* This last function registration should actually work.  Generate
  ** a no-op function (that always returns NULL) and which has the
  ** maximum-length function name and the maximum number of parameters.
  */
  sqlite3_ಎಲ್ಲೆ(db, SQLITE_ಎಲ್ಲೆ_FUNCTION_ARG, 1000000);
  mxArg = sqlite3_ಎಲ್ಲೆ(db, SQLITE_ಎಲ್ಲೆ_FUNCTION_ARG, -1);
  rc = sqlite3_create_function(db, "nullx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       mxArg, SQLITE_UTF8, 0, t_ಹೆಜ್ಜೆ, 0, 0);
  ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು abuse_err;
                                
  ಹಿಂತಿರುಗಿ TCL_OK;

abuse_err:
  Tcl_Append_ಫಲಿತಾಂಶ(interp, "sqlite3_create_function abused test failed", 
                   (ಅಕ್ಷರ*)0);
  ಹಿಂತಿರುಗಿ TCL_ERROR;
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
ಸ್ಥಿರ ಶೂನ್ಯ rankfunc(sqlite3_context *pCtx, ಪೂರ್ಣಾಂಕ nVal, sqlite3_ಮೌಲ್ಯ **apVal){
  ಪೂರ್ಣಾಂಕ *aMatchinfo;                /* Return value of matchinfo() */
  ಪೂರ್ಣಾಂಕ nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = 0;                   /* Number of columns in the table */
  ಪೂರ್ಣಾಂಕ nPhrase = 0;                /* Number of phrases in the query */
  ಪೂರ್ಣಾಂಕ iPhrase;                    /* Current phrase */
  ದ್ವಿಗುಣ score = 0.0;             /* Value to return */

  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  ಆದರೆ( nVal<1 ) ಹೋಗು wrong_ಸಂಖ್ಯೆ_ವಾದಗಳು;
  aMatchinfo = (ಪೂರ್ಣಾಂಕ*)sqlite3_ಮೌಲ್ಯ_blob(apVal[0]);
  nMatchinfo = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(apVal[0]) / ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ);
  ಆದರೆ( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    n_ಲಂಬಸಾಲು = aMatchinfo[1];
  }
  ಆದರೆ( nMatchinfo!=(2+3*n_ಲಂಬಸಾಲು*nPhrase) ){
    sqlite3_ಫಲಿತಾಂಶ_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( nVal!=(1+n_ಲಂಬಸಾಲು) ) ಹೋಗು wrong_ಸಂಖ್ಯೆ_ವಾದಗಳು;

  /* Iterate through each phrase in the users query. */
  ಗಾಗಿ(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    ಪೂರ್ಣಾಂಕ *aPhraseinfo = &aMatchinfo[2 + iPhrase*n_ಲಂಬಸಾಲು*3];
    ಗಾಗಿ(i_ಲಂಬಸಾಲು=0; i_ಲಂಬಸಾಲು<n_ಲಂಬಸಾಲು; i_ಲಂಬಸಾಲು++){
      ಪೂರ್ಣಾಂಕ n_ದೊರಕಿತು_ಎಣಿಕೆ = aPhraseinfo[3*i_ಲಂಬಸಾಲು];
      ಪೂರ್ಣಾಂಕ n_Global_ದೊರಕಿತು_ಎಣಿಕೆ = aPhraseinfo[3*i_ಲಂಬಸಾಲು+1];
      ದ್ವಿಗುಣ ತೂಕ = sqlite3_ಮೌಲ್ಯ_double(apVal[i_ಲಂಬಸಾಲು+1]);
      ಆದರೆ( n_ದೊರಕಿತು_ಎಣಿಕೆ>0 ){
        score += ((ದ್ವಿಗುಣ)n_ದೊರಕಿತು_ಎಣಿಕೆ / (ದ್ವಿಗುಣ)n_Global_ದೊರಕಿತು_ಎಣಿಕೆ) * ತೂಕ;
      }
    }
  }

  sqlite3_ಫಲಿತಾಂಶ_double(pCtx, score);
  ಹಿಂತಿರುಗಿ;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_ಸಂಖ್ಯೆ_ವಾದಗಳು:
  sqlite3_ಫಲಿತಾಂಶ_error(pCtx, "wrong number of arguments to function rank()", -1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI install_fts3_rank_function(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, sqlite3**);
  sqlite3 *db;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest_func_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ Md5_ರಿಜಿಸ್ಟರ್(sqlite3 *, ಅಕ್ಷರ **, ಸ್ಥಿರಾಂಕ sqlite3_api_routines *);

  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))ರಿಜಿಸ್ಟರ್_Test_Functions);
  sqlite3_auto_extension((ಶೂನ್ಯ(*)(ಶೂನ್ಯ))Md5_ರಿಜಿಸ್ಟರ್);
  ಹಿಂತಿರುಗಿ TCL_OK;
}
