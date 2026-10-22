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
#شامل "sqlite3.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>

#شامل "sqliteInt.h"
#شامل "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
ایستا تهی *testContextMalloc(sqlite3_context *context, صحیح nByte){
  نویسه *z = sqlite3_malloc(nByte);
  اگر( !z && nByte>0 ){
    sqlite3_result_error_nomem(context);
  }
  بازگشت z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
ایستا تهی randStr(sqlite3_context *context, صحیح argc, sqlite3_مقدار **argv){
  ایستا ثابت unsigned نویسه zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  صحیح iMin, iMax, n, r, i;
  unsigned نویسه zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  أكد(argc==2);

  iMin = sqlite3_مقدار_int(argv[0]);
  اگر( iMin<0 ) iMin = 0;
  اگر( iMin>=اندازه(zBuf) ) iMin = اندازه(zBuf)-1;
  iMax = sqlite3_مقدار_int(argv[1]);
  اگر( iMax<iMin ) iMax = iMin;
  اگر( iMax>=اندازه(zBuf) ) iMax = اندازه(zBuf)-1;
  n = iMin;
  اگر( iMax>iMin ){
    sqlite3_randomness(اندازه(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  أكد( n<اندازه(zBuf) );
  sqlite3_randomness(n, zBuf);
  برای(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(اندازه(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_result_text(context, (نویسه*)zBuf, n, SQLITE_TRANSIENT);
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
ایستا صحیح test_destructor_شمار_var = 0;
ایستا تهی destructor(تهی *p){
  نویسه *zVal = (نویسه *)p;
  أكد(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_شمار_var--;
}
ایستا تهی test_destructor(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  نویسه *zVal;
  صحیح len;
  
  test_destructor_شمار_var++;
  أكد( nArg==1 );
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) بازگشت;
  len = sqlite3_مقدار_bytes(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  اگر( !zVal ){
    بازگشت;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_مقدار_text(argv[0]), len);
  sqlite3_result_text(pCtx, zVal, -1, destructor);
}
#ifndef SQLITE_OMIT_UTF16
ایستا تهی test_destructor16(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  نویسه *zVal;
  صحیح len;
  
  test_destructor_شمار_var++;
  أكد( nArg==1 );
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ) بازگشت;
  len = sqlite3_مقدار_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  اگر( !zVal ){
    بازگشت;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_مقدار_text16(argv[0]), len);
  sqlite3_result_text16(pCtx, zVal, -1, destructor);
}
#endif
ایستا تهی test_destructor_شمار(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  sqlite3_result_int(pCtx, test_destructor_شمار_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ifndef SQLITE_UNTESTABLE
تهی sqlite3BeginBenignMalloc(تهی);
تهی sqlite3EndBenignMalloc(تهی);
#وگرنه
  #define sqlite3BeginBenignMalloc()
  #define sqlite3EndBenignMalloc()
#endif
ایستا تهی test_agg_errmsg16_step(sqlite3_context *a, صحیح b,sqlite3_مقدار **c){
}
ایستا تهی test_agg_errmsg16_final(sqlite3_context *ctx){
#ifndef SQLITE_OMIT_UTF16
  ثابت تهی *z;
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
ایستا تهی free_test_auxdata(تهی *p) {sqlite3_free(p);}
ایستا تهی test_auxdata(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  صحیح i;
  نویسه *zRet = testContextMalloc(pCtx, nArg*2);
  اگر( !zRet ) بازگشت;
  memset(zRet, 0, nArg*2);
  برای(i=0; i<nArg; i++){
    نویسه ثابت *z = (نویسه*)sqlite3_مقدار_text(argv[i]);
    اگر( z ){
      صحیح n;
      نویسه *zAux = sqlite3_get_auxdata(pCtx, i);
      اگر( zAux ){
        zRet[i*2] = '1';
        أكد( strcmp(zAux,z)==0 );
      }وگرنه {
        zRet[i*2] = '0';
      }
      n = (صحیح)strlen(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      اگر( zAux ){
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
ایستا تهی test_error(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  sqlite3_result_error(pCtx, (نویسه*)sqlite3_مقدار_text(argv[0]), -1);
  اگر( nArg==2 ){
    sqlite3_result_error_code(pCtx, sqlite3_مقدار_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
ایستا تهی counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  صحیح nArg,                /* Number of function arguments */
  sqlite3_مقدار **argv     /* Values for all function arguments */
){
  صحیح *pCounter = (صحیح*)sqlite3_get_auxdata(pCtx, 0);
  اگر( pCounter==0 ){
    pCounter = sqlite3_malloc( اندازه(*pCounter) );
    اگر( pCounter==0 ){
      sqlite3_result_error_nomem(pCtx);
      بازگشت;
    }
    *pCounter = sqlite3_مقدار_int(argv[0]);
    sqlite3_set_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }وگرنه{
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
ایستا تهی test_isolation(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3_مقدار_text16(argv[0]);
  sqlite3_مقدار_text(argv[0]);
  sqlite3_مقدار_text16(argv[0]);
  sqlite3_مقدار_text(argv[0]);
#endif
  sqlite3_result_مقدار(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
ایستا تهی test_eval(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  sqlite3_stmt *pStmt;
  صحیح rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ثابت نویسه *zSql;
  ایستا ثابت نویسه zTag[] = "test_eval()";

  اگر( sqlite3_get_clientdata(db, zTag) ){
    sqlite3_result_error(pCtx, "nested test_eval()", -1);
    بازگشت;
  }
  zSql = (نویسه*)sqlite3_مقدار_text(argv[0]);
  sqlite3_set_clientdata(db, zTag, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3_step(pStmt);
    اگر( rc==SQLITE_ROW ){
      sqlite3_result_مقدار(pCtx, sqlite3_column_مقدار(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_set_clientdata(db, zTag, 0, 0);
  اگر( rc ){
    نویسه *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_result_text(pCtx, zErr, -1, sqlite3_free);
    sqlite3_result_error_code(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
ایستا صحیح testHexChar(نویسه c){
  اگر( c>='0' && c<='9' ){
    بازگشت c - '0';
  }وگرنه اگر( c>='a' && c<='f' ){
    بازگشت c - 'a' + 10;
  }وگرنه اگر( c>='A' && c<='F' ){
    بازگشت c - 'A' + 10;
  }
  بازگشت 0;
}

/*
** Convert hex to binary.
*/
ایستا تهی testHexToBin(ثابت نویسه *zIn, نویسه *zOut){
  وقتی( zIn[0] && zIn[1] ){
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
ایستا تهی testHexToUtf16be(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  صحیح n;
  ثابت نویسه *zIn;
  نویسه *zOut;
  أكد( nArg==1 );
  n = sqlite3_مقدار_bytes(argv[0]);
  zIn = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  اگر( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }وگرنه{
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
ایستا تهی testHexToUtf8(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  صحیح n;
  ثابت نویسه *zIn;
  نویسه *zOut;
  أكد( nArg==1 );
  n = sqlite3_مقدار_bytes(argv[0]);
  zIn = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  اگر( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }وگرنه{
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
ایستا تهی testHexToUtf16le(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_مقدار **argv
){
  صحیح n;
  ثابت نویسه *zIn;
  نویسه *zOut;
  أكد( nArg==1 );
  n = sqlite3_مقدار_bytes(argv[0]);
  zIn = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  اگر( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }وگرنه{
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
ایستا تهی real2hex(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  اجتماع {
    sqlite3_uint64 i;
    دوگانه r;
    unsigned نویسه x[8];
  } v;
  نویسه zOut[20];
  صحیح i;
  صحیح bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_مقدار_double(argv[0]);
  برای(i=0; i<8; i++){
    اگر( bigEndian ){
      zOut[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }وگرنه{
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
ایستا تهی test_extract(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  صحیح iIdx;                       /* Required field */
  صحیح iCurrent = 0;               /* Current field */

  أكد( argc==2 );
  pRec = (u8*)sqlite3_مقدار_blob(argv[0]);
  iIdx = sqlite3_مقدار_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];

  برای(iCurrent=0; pHdr<pEndHdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, اندازه(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    اگر( iCurrent==iIdx ){
      sqlite3_result_مقدار(context, &mem);
    }

    اگر( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
ایستا تهی test_decode(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_شمار(pRet);

  أكد( argc==1 );
  pRec = (u8*)sqlite3_مقدار_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];
  وقتی( pHdr<pEndHdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, اندازه(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    انتخاب( sqlite3_مقدار_type(&mem) ){
      حالت SQLITE_TEXT:
        pVal = Tcl_NewStringObj((ثابت نویسه*)sqlite3_مقدار_text(&mem), -1);
        بشکن;

      حالت SQLITE_BLOB: {
        نویسه hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        صحیح n = sqlite3_مقدار_bytes(&mem);
        u8 *z = (u8*)sqlite3_مقدار_blob(&mem);
        صحیح i;
        pVal = Tcl_NewStringObj("x'", -1);
        برای(i=0; i<n; i++){
          نویسه hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_AppendStringsToObj(pVal, hex, 0);
        }
        Tcl_AppendStringsToObj(pVal, "'", 0);
        بشکن;
      }

      حالت SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_مقدار_double(&mem));
        بشکن;

      حالت SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_مقدار_int64(&mem));
        بشکن;

      حالت SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        بشکن;

      پیشفرض:
        أكد( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    اگر( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_result_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_شمار(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
ایستا تهی test_zeroblob(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح nZero = sqlite3_مقدار_int(argv[0]);
  sqlite3_result_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
ایستا تهی test_getsubtype(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3_result_int(context, (صحیح)sqlite3_مقدار_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
ایستا تهی test_frombind(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3_uint64 m = 0;
  صحیح i;
  برای(i=0; i<argc && i<63; i++){
    اگر( sqlite3_مقدار_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_result_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
ایستا تهی test_setsubtype(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3_result_مقدار(context, argv[0]);
  sqlite3_result_subtype(context, (unsigned صحیح)sqlite3_مقدار_int(argv[1]));
}

ایستا صحیح registerTestFunctions(
  sqlite3 *db,
  نویسه **pz_Err_پیام,
  ثابت sqlite3_api_routines *pThunk
){
  ایستا ثابت ساختار {
     نویسه *zName;
     signed نویسه nArg;
     unsigned صحیح eTextRep; /* 1: UTF-16.  0: UTF-8 */
     تهی (*xFunc)(sqlite3_context*,صحیح,sqlite3_مقدار **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ifndef SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, testHexToUtf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, testHexToUtf16le},
#endif
    { "hex_to_utf8",           1, SQLITE_UTF8, testHexToUtf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_شمار},
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
  صحیح i;

  برای(i=0; i<اندازه(aFuncs)/اندازه(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].eTextRep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_step, test_agg_errmsg16_final);
      
  بازگشت SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
ایستا صحیح SQLITE_TCLAPI autoinstall_test_funcs(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  بیرونی صحیح Md5_Register(sqlite3 *, نویسه **, ثابت sqlite3_api_routines *);
  صحیح rc = sqlite3_auto_extension((تهی(*)(تهی))registerTestFunctions);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((تهی(*)(تهی))Md5_Register);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
ایستا تهی tStep(sqlite3_context *a, صحیح b, sqlite3_مقدار **c){}
ایستا تهی tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
ایستا صحیح SQLITE_TCLAPI abuse_create_function(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  بیرونی صحیح getDbPointer(Tcl_Interp*, ثابت نویسه*, sqlite3**);
  sqlite3 *db;
  صحیح rc;
  صحیح mxArg;

  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep,tStep,tFinal);
  اگر( rc!=SQLITE_MISUSE ) برو abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, tStep, 0);
  اگر( rc!=SQLITE_MISUSE ) برو abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, 0, tFinal);
  اگر( rc!=SQLITE_MISUSE) برو abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  اگر( rc!=SQLITE_MISUSE ) برو abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, tStep, 0);
  اگر( rc!=SQLITE_MISUSE ) برو abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, tStep, 0, 0);
  اگر( rc!=SQLITE_MISUSE ) برو abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, tStep, 0, 0);
  اگر( rc!=SQLITE_MISUSE ) برو abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, tStep, 0, 0);
  اگر( rc!=SQLITE_MISUSE ) برو abuse_err;

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
  اگر( rc!=SQLITE_OK ) برو abuse_err;
                                
  بازگشت TCL_OK;

abuse_err:
  Tcl_AppendResult(interp, "sqlite3_create_function abused test failed", 
                   (نویسه*)0);
  بازگشت TCL_ERROR;
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
ایستا تهی rankfunc(sqlite3_context *pCtx, صحیح nVal, sqlite3_مقدار **apVal){
  صحیح *aMatchinfo;                /* Return value of matchinfo() */
  صحیح nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  صحیح nCol = 0;                   /* Number of columns in the table */
  صحیح nPhrase = 0;                /* Number of phrases in the query */
  صحیح iPhrase;                    /* Current phrase */
  دوگانه score = 0.0;             /* Value to return */

  أكد( اندازه(صحیح)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  اگر( nVal<1 ) برو wrong_عدد_args;
  aMatchinfo = (صحیح*)sqlite3_مقدار_blob(apVal[0]);
  nMatchinfo = sqlite3_مقدار_bytes(apVal[0]) / اندازه(صحیح);
  اگر( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
  }
  اگر( nMatchinfo!=(2+3*nCol*nPhrase) ){
    sqlite3_result_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    بازگشت;
  }
  اگر( nVal!=(1+nCol) ) برو wrong_عدد_args;

  /* Iterate through each phrase in the users query. */
  برای(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    صحیح iCol;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    صحیح *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
    برای(iCol=0; iCol<nCol; iCol++){
      صحیح n_Hit_شمار = aPhraseinfo[3*iCol];
      صحیح n_Global_Hit_شمار = aPhraseinfo[3*iCol+1];
      دوگانه weight = sqlite3_مقدار_double(apVal[iCol+1]);
      اگر( n_Hit_شمار>0 ){
        score += ((دوگانه)n_Hit_شمار / (دوگانه)n_Global_Hit_شمار) * weight;
      }
    }
  }

  sqlite3_result_double(pCtx, score);
  بازگشت;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_عدد_args:
  sqlite3_result_error(pCtx, "wrong number of arguments to function rank()", -1);
}

ایستا صحیح SQLITE_TCLAPI install_fts3_rank_function(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  بیرونی صحیح getDbPointer(Tcl_Interp*, ثابت نویسه*, sqlite3**);
  sqlite3 *db;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }

  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  بازگشت TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_func_Init(Tcl_Interp *interp){
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  صحیح i;
  بیرونی صحیح Md5_Register(sqlite3 *, نویسه **, ثابت sqlite3_api_routines *);

  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((تهی(*)(تهی))registerTestFunctions);
  sqlite3_auto_extension((تهی(*)(تهی))Md5_Register);
  بازگشت TCL_OK;
}
