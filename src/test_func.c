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
ساکن خالی *testContextMalloc(sqlite3_context *context, صحیح nByte){
  حرف *z = sqlite3_malloc(nByte);
  اگر( !z && nByte>0 ){
    sqlite3_نتیجہ_error_nomem(context);
  }
  واپس z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
ساکن خالی randStr(sqlite3_context *context, صحیح argc, sqlite3_قدر **argv){
  ساکن ثابت غیرعلامتی حرف zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  صحیح iMin, iMax, n, r, i;
  غیرعلامتی حرف zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  أكد(argc==2);

  iMin = sqlite3_قدر_int(argv[0]);
  اگر( iMin<0 ) iMin = 0;
  اگر( iMin>=حجم(zBuf) ) iMin = حجم(zBuf)-1;
  iMax = sqlite3_قدر_int(argv[1]);
  اگر( iMax<iMin ) iMax = iMin;
  اگر( iMax>=حجم(zBuf) ) iMax = حجم(zBuf)-1;
  n = iMin;
  اگر( iMax>iMin ){
    sqlite3_randomness(حجم(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  أكد( n<حجم(zBuf) );
  sqlite3_randomness(n, zBuf);
  برائے(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(حجم(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_نتیجہ_متن(context, (حرف*)zBuf, n, SQLITE_TRANSIENT);
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
ساکن صحیح test_destructor_گنتی_var = 0;
ساکن خالی destructor(خالی *p){
  حرف *zVal = (حرف *)p;
  أكد(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_گنتی_var--;
}
ساکن خالی test_destructor(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  حرف *zVal;
  صحیح len;
  
  test_destructor_گنتی_var++;
  أكد( nArg==1 );
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ) واپس;
  len = sqlite3_قدر_بائٹیں(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  اگر( !zVal ){
    واپس;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  حافظہ_نقل(zVal, sqlite3_قدر_متن(argv[0]), len);
  sqlite3_نتیجہ_متن(pCtx, zVal, -1, destructor);
}
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ساکن خالی test_destructor16(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  حرف *zVal;
  صحیح len;
  
  test_destructor_گنتی_var++;
  أكد( nArg==1 );
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ) واپس;
  len = sqlite3_قدر_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  اگر( !zVal ){
    واپس;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  حافظہ_نقل(zVal, sqlite3_قدر_text16(argv[0]), len);
  sqlite3_نتیجہ_text16(pCtx, zVal, -1, destructor);
}
#اگر_ختم
ساکن خالی test_destructor_گنتی(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_int(pCtx, test_destructor_گنتی_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#اگر_غیر_تعریف_شدہ SQLITE_UNTESTABLE
خالی sqlite3BeginBenignMalloc(خالی);
خالی sqlite3_انتہا_Benign_Malloc(خالی);
#ورنہ
  #تعریف_کرو sqlite3BeginBenignMalloc()
  #تعریف_کرو sqlite3_انتہا_Benign_Malloc()
#اگر_ختم
ساکن خالی test_agg_errmsg16_قدم(sqlite3_context *a, صحیح b,sqlite3_قدر **c){
}
ساکن خالی test_agg_errmsg16_final(sqlite3_context *ctx){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  ثابت خالی *z;
  sqlite3 * db = sqlite3_context_db_handle(ctx);
  sqlite3_aggregate_context(ctx, 2048);
  z = sqlite3_errmsg16(db);
  sqlite3_نتیجہ_text16(ctx, z, -1, SQLITE_TRANSIENT);
#اگر_ختم
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
ساکن خالی free_test_auxdata(خالی *p) {sqlite3_free(p);}
ساکن خالی test_auxdata(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  صحیح i;
  حرف *zRet = testContextMalloc(pCtx, nArg*2);
  اگر( !zRet ) واپس;
  memset(zRet, 0, nArg*2);
  برائے(i=0; i<nArg; i++){
    حرف ثابت *z = (حرف*)sqlite3_قدر_متن(argv[i]);
    اگر( z ){
      صحیح n;
      حرف *zAux = sqlite3_get_auxdata(pCtx, i);
      اگر( zAux ){
        zRet[i*2] = '1';
        أكد( strcmp(zAux,z)==0 );
      }ورنہ {
        zRet[i*2] = '0';
      }
      n = (صحیح)لڑی_لمبائی(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      اگر( zAux ){
        حافظہ_نقل(zAux, z, n);
        sqlite3_سیٹ_auxdata(pCtx, i, zAux, free_test_auxdata);
      }
      zRet[i*2+1] = ' ';
    }
  }
  sqlite3_نتیجہ_متن(pCtx, zRet, 2*nArg-1, free_test_auxdata);
}

/*
** A function to test error reporting from user functions. This function
** returns a copy of its first argument as the error message.  If the
** second argument exists, it becomes the error code.
*/
ساکن خالی test_error(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_error(pCtx, (حرف*)sqlite3_قدر_متن(argv[0]), -1);
  اگر( nArg==2 ){
    sqlite3_نتیجہ_error_رمز(pCtx, sqlite3_قدر_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
ساکن خالی شمار_Func(
  sqlite3_context *pCtx,   /* Function context */
  صحیح nArg,                /* Number of function arguments */
  sqlite3_قدر **argv     /* Values for all function arguments */
){
  صحیح *p_شمار = (صحیح*)sqlite3_get_auxdata(pCtx, 0);
  اگر( p_شمار==0 ){
    p_شمار = sqlite3_malloc( حجم(*p_شمار) );
    اگر( p_شمار==0 ){
      sqlite3_نتیجہ_error_nomem(pCtx);
      واپس;
    }
    *p_شمار = sqlite3_قدر_int(argv[0]);
    sqlite3_سیٹ_auxdata(pCtx, 0, p_شمار, sqlite3_free);
  }ورنہ{
    ++*p_شمار;
  }
  sqlite3_نتیجہ_int(pCtx, *p_شمار);
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
ساکن خالی test_isolation(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  sqlite3_قدر_text16(argv[0]);
  sqlite3_قدر_متن(argv[0]);
  sqlite3_قدر_text16(argv[0]);
  sqlite3_قدر_متن(argv[0]);
#اگر_ختم
  sqlite3_نتیجہ_قدر(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
ساکن خالی test_eval(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  sqlite3_stmt *pStmt;
  صحیح rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ثابت حرف *zSql;
  ساکن ثابت حرف z_لیبل[] = "test_eval()";

  اگر( sqlite3_get_clientdata(db, z_لیبل) ){
    sqlite3_نتیجہ_error(pCtx, "nested test_eval()", -1);
    واپس;
  }
  zSql = (حرف*)sqlite3_قدر_متن(argv[0]);
  sqlite3_سیٹ_clientdata(db, z_لیبل, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3_قدم(pStmt);
    اگر( rc==SQLITE_صف ){
      sqlite3_نتیجہ_قدر(pCtx, sqlite3_عمود_قدر(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_سیٹ_clientdata(db, z_لیبل, 0, 0);
  اگر( rc ){
    حرف *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_نتیجہ_متن(pCtx, zErr, -1, sqlite3_free);
    sqlite3_نتیجہ_error_رمز(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
ساکن صحیح testHexChar(حرف c){
  اگر( c>='0' && c<='9' ){
    واپس c - '0';
  }ورنہ اگر( c>='a' && c<='f' ){
    واپس c - 'a' + 10;
  }ورنہ اگر( c>='A' && c<='F' ){
    واپس c - 'A' + 10;
  }
  واپس 0;
}

/*
** Convert hex to binary.
*/
ساکن خالی test_Hex_مقصد_Bin(ثابت حرف *z_اندر, حرف *z_برآمد){
  جب( z_اندر[0] && z_اندر[1] ){
    *(z_برآمد++) = (testHexChar(z_اندر[0])<<4) + testHexChar(z_اندر[1]);
    z_اندر += 2;
  }
}

/*
**      hex_to_utf16be(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ساکن خالی test_Hex_مقصد_Utf16be(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  صحیح n;
  ثابت حرف *z_اندر;
  حرف *z_برآمد;
  أكد( nArg==1 );
  n = sqlite3_قدر_بائٹیں(argv[0]);
  z_اندر = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  z_برآمد = sqlite3_malloc( n/2 );
  اگر( z_برآمد==0 ){
    sqlite3_نتیجہ_error_nomem(pCtx);
  }ورنہ{
    test_Hex_مقصد_Bin(z_اندر, z_برآمد);
    sqlite3_نتیجہ_text16be(pCtx, z_برآمد, n/2, sqlite3_free);
  }
}
#اگر_ختم

/*
**      hex_to_utf8(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
ساکن خالی test_Hex_مقصد_Utf8(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  صحیح n;
  ثابت حرف *z_اندر;
  حرف *z_برآمد;
  أكد( nArg==1 );
  n = sqlite3_قدر_بائٹیں(argv[0]);
  z_اندر = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  z_برآمد = sqlite3_malloc( n/2 );
  اگر( z_برآمد==0 ){
    sqlite3_نتیجہ_error_nomem(pCtx);
  }ورنہ{
    test_Hex_مقصد_Bin(z_اندر, z_برآمد);
    sqlite3_نتیجہ_متن(pCtx, z_برآمد, n/2, sqlite3_free);
  }
}

/*
**      hex_to_utf16le(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
ساکن خالی test_Hex_مقصد_Utf16le(
  sqlite3_context *pCtx, 
  صحیح nArg,
  sqlite3_قدر **argv
){
  صحیح n;
  ثابت حرف *z_اندر;
  حرف *z_برآمد;
  أكد( nArg==1 );
  n = sqlite3_قدر_بائٹیں(argv[0]);
  z_اندر = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  z_برآمد = sqlite3_malloc( n/2 );
  اگر( z_برآمد==0 ){
    sqlite3_نتیجہ_error_nomem(pCtx);
  }ورنہ{
    test_Hex_مقصد_Bin(z_اندر, z_برآمد);
    sqlite3_نتیجہ_text16le(pCtx, z_برآمد, n/2, sqlite3_free);
  }
}
#اگر_ختم

/*
** SQL function:   real2hex(X)
**
** If argument X is a real number, then convert it into a string which is
** the big-endian hexadecimal representation of the ieee754 encoding of
** that number.  If X is not a real number, return NULL.
*/
ساکن خالی real2hex(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  اتحاد {
    sqlite3_uint64 i;
    دوہرا r;
    غیرعلامتی حرف x[8];
  } v;
  حرف z_برآمد[20];
  صحیح i;
  صحیح bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_قدر_double(argv[0]);
  برائے(i=0; i<8; i++){
    اگر( bigEndian ){
      z_برآمد[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_برآمد[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }ورنہ{
      z_برآمد[14-i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_برآمد[14-i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }
  }
  z_برآمد[16] = 0;
  sqlite3_نتیجہ_متن(context, z_برآمد, -1, SQLITE_TRANSIENT);
}

/*
**     test_extract(record, field)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as the first argument. The
** second argument is the index of the field within that record to
** extract and return.
*/
ساکن خالی test_نکالو(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_انتہا_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  صحیح iIdx;                       /* Required field */
  صحیح iCurrent = 0;               /* Current field */

  أكد( argc==2 );
  pRec = (u8*)sqlite3_قدر_blob(argv[0]);
  iIdx = sqlite3_قدر_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_انتہا_Hdr = &pRec[nHdr];

  برائے(iCurrent=0; pHdr<p_انتہا_Hdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, حجم(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    اگر( iCurrent==iIdx ){
      sqlite3_نتیجہ_قدر(context, &mem);
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
ساکن خالی test_decode(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_انتہا_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pRet);

  أكد( argc==1 );
  pRec = (u8*)sqlite3_قدر_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_انتہا_Hdr = &pRec[nHdr];
  جب( pHdr<p_انتہا_Hdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, حجم(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    انتخاب( sqlite3_قدر_type(&mem) ){
      حالت SQLITE_متن:
        pVal = Tcl_NewStringObj((ثابت حرف*)sqlite3_قدر_متن(&mem), -1);
        توڑو;

      حالت SQLITE_BLOB: {
        حرف hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        صحیح n = sqlite3_قدر_بائٹیں(&mem);
        u8 *z = (u8*)sqlite3_قدر_blob(&mem);
        صحیح i;
        pVal = Tcl_NewStringObj("x'", -1);
        برائے(i=0; i<n; i++){
          حرف hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_Append_لڑیاں_مقصد_Obj(pVal, hex, 0);
        }
        Tcl_Append_لڑیاں_مقصد_Obj(pVal, "'", 0);
        توڑو;
      }

      حالت SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_قدر_double(&mem));
        توڑو;

      حالت SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_قدر_int64(&mem));
        توڑو;

      حالت SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        توڑو;

      طےشدہ:
        أكد( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    اگر( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_نتیجہ_متن(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_گنتی(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
ساکن خالی test_zeroblob(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح nZero = sqlite3_قدر_int(argv[0]);
  sqlite3_نتیجہ_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
ساکن خالی test_getsubtype(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_int(context, (صحیح)sqlite3_قدر_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
ساکن خالی test_frombind(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3_uint64 m = 0;
  صحیح i;
  برائے(i=0; i<argc && i<63; i++){
    اگر( sqlite3_قدر_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_نتیجہ_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
ساکن خالی test_setsubtype(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_قدر(context, argv[0]);
  sqlite3_نتیجہ_subtype(context, (غیرعلامتی صحیح)sqlite3_قدر_int(argv[1]));
}

ساکن صحیح رجسٹر_Test_Functions(
  sqlite3 *db,
  حرف **pz_Err_پیغام,
  ثابت sqlite3_api_routines *pThunk
){
  ساکن ثابت ساخت {
     حرف *zName;
     علامتی حرف nArg;
     غیرعلامتی صحیح e_متن_Rep; /* 1: UTF-16.  0: UTF-8 */
     خالی (*xFunc)(sqlite3_context*,صحیح,sqlite3_قدر **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, test_Hex_مقصد_Utf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, test_Hex_مقصد_Utf16le},
#اگر_ختم
    { "hex_to_utf8",           1, SQLITE_UTF8, test_Hex_مقصد_Utf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_گنتی},
    { "test_auxdata",         -1, SQLITE_UTF8, test_auxdata},
    { "test_error",            1, SQLITE_UTF8, test_error},
    { "test_error",            2, SQLITE_UTF8, test_error},
    { "test_eval",             1, SQLITE_UTF8, test_eval},
    { "test_isolation",        2, SQLITE_UTF8, test_isolation},
    { "test_counter",          1, SQLITE_UTF8, شمار_Func},
    { "real2hex",              1, SQLITE_UTF8, real2hex},
    { "test_decode",           1, SQLITE_UTF8, test_decode},
    { "test_extract",          2, SQLITE_UTF8, test_نکالو},
    { "test_zeroblob",  1, SQLITE_UTF8|SQLITE_DETERMINISTIC, test_zeroblob},
    { "test_getsubtype",       1, SQLITE_UTF8, test_getsubtype},
    { "test_setsubtype",       2, SQLITE_UTF8|SQLITE_نتیجہ_SUBTYPE,
                                               test_setsubtype},
    { "test_frombind",        -1, SQLITE_UTF8, test_frombind},
  };
  صحیح i;

  برائے(i=0; i<حجم(aFuncs)/حجم(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].e_متن_Rep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_قدم, test_agg_errmsg16_final);
      
  واپس SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
ساکن صحیح SQLITE_TCLAPI autoinstall_test_funcs(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خارجی صحیح Md5_رجسٹر(sqlite3 *, حرف **, ثابت sqlite3_api_routines *);
  صحیح rc = sqlite3_auto_extension((خالی(*)(خالی))رجسٹر_Test_Functions);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((خالی(*)(خالی))Md5_رجسٹر);
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
ساکن خالی t_قدم(sqlite3_context *a, صحیح b, sqlite3_قدر **c){}
ساکن خالی tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
ساکن صحیح SQLITE_TCLAPI abuse_create_function(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  sqlite3 *db;
  صحیح rc;
  صحیح mxArg;

  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_قدم,t_قدم,tFinal);
  اگر( rc!=SQLITE_MISUSE ) جاؤ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_قدم, t_قدم, 0);
  اگر( rc!=SQLITE_MISUSE ) جاؤ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_قدم, 0, tFinal);
  اگر( rc!=SQLITE_MISUSE) جاؤ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  اگر( rc!=SQLITE_MISUSE ) جاؤ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, t_قدم, 0);
  اگر( rc!=SQLITE_MISUSE ) جاؤ abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, t_قدم, 0, 0);
  اگر( rc!=SQLITE_MISUSE ) جاؤ abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, t_قدم, 0, 0);
  اگر( rc!=SQLITE_MISUSE ) جاؤ abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, t_قدم, 0, 0);
  اگر( rc!=SQLITE_MISUSE ) جاؤ abuse_err;

  /* This last function registration should actually work.  Generate
  ** a no-op function (that always returns NULL) and which has the
  ** maximum-length function name and the maximum number of parameters.
  */
  sqlite3_حد_بندی(db, SQLITE_حد_بندی_FUNCTION_ARG, 1000000);
  mxArg = sqlite3_حد_بندی(db, SQLITE_حد_بندی_FUNCTION_ARG, -1);
  rc = sqlite3_create_function(db, "nullx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       mxArg, SQLITE_UTF8, 0, t_قدم, 0, 0);
  اگر( rc!=SQLITE_OK ) جاؤ abuse_err;
                                
  واپس TCL_OK;

abuse_err:
  Tcl_Append_نتیجہ(interp, "sqlite3_create_function abused test failed", 
                   (حرف*)0);
  واپس TCL_ERROR;
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
ساکن خالی rankfunc(sqlite3_context *pCtx, صحیح nVal, sqlite3_قدر **apVal){
  صحیح *aMatchinfo;                /* Return value of matchinfo() */
  صحیح nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  صحیح n_ستون = 0;                   /* Number of columns in the table */
  صحیح nPhrase = 0;                /* Number of phrases in the query */
  صحیح iPhrase;                    /* Current phrase */
  دوہرا score = 0.0;             /* Value to return */

  أكد( حجم(صحیح)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  اگر( nVal<1 ) جاؤ wrong_نمبر_دلائل;
  aMatchinfo = (صحیح*)sqlite3_قدر_blob(apVal[0]);
  nMatchinfo = sqlite3_قدر_بائٹیں(apVal[0]) / حجم(صحیح);
  اگر( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    n_ستون = aMatchinfo[1];
  }
  اگر( nMatchinfo!=(2+3*n_ستون*nPhrase) ){
    sqlite3_نتیجہ_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    واپس;
  }
  اگر( nVal!=(1+n_ستون) ) جاؤ wrong_نمبر_دلائل;

  /* Iterate through each phrase in the users query. */
  برائے(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    صحیح i_ستون;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    صحیح *aPhraseinfo = &aMatchinfo[2 + iPhrase*n_ستون*3];
    برائے(i_ستون=0; i_ستون<n_ستون; i_ستون++){
      صحیح n_کامیابی_گنتی = aPhraseinfo[3*i_ستون];
      صحیح n_Global_کامیابی_گنتی = aPhraseinfo[3*i_ستون+1];
      دوہرا وزن = sqlite3_قدر_double(apVal[i_ستون+1]);
      اگر( n_کامیابی_گنتی>0 ){
        score += ((دوہرا)n_کامیابی_گنتی / (دوہرا)n_Global_کامیابی_گنتی) * وزن;
      }
    }
  }

  sqlite3_نتیجہ_double(pCtx, score);
  واپس;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_نمبر_دلائل:
  sqlite3_نتیجہ_error(pCtx, "wrong number of arguments to function rank()", -1);
}

ساکن صحیح SQLITE_TCLAPI install_fts3_rank_function(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  sqlite3 *db;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }

  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  واپس TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_func_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  صحیح i;
  خارجی صحیح Md5_رجسٹر(sqlite3 *, حرف **, ثابت sqlite3_api_routines *);

  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((خالی(*)(خالی))رجسٹر_Test_Functions);
  sqlite3_auto_extension((خالی(*)(خالی))Md5_رجسٹر);
  واپس TCL_OK;
}
