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
#ضمن "sqlite3.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>

#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
ساكن فراغ *testContextMalloc(sqlite3_context *context, صحيح nByte){
  حرف *z = sqlite3_malloc(nByte);
  إذا( !z && nByte>0 ){
    sqlite3_نتیجہ_error_nomem(context);
  }
  أرجع z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
ساكن فراغ randStr(sqlite3_context *context, صحيح argc, sqlite3_قدر **argv){
  ساكن ثابت بنانشانی حرف zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  صحيح iMin, iMax, n, r, i;
  بنانشانی حرف zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  أكد(argc==2);

  iMin = sqlite3_قدر_int(argv[0]);
  إذا( iMin<0 ) iMin = 0;
  إذا( iMin>=حجم(zBuf) ) iMin = حجم(zBuf)-1;
  iMax = sqlite3_قدر_int(argv[1]);
  إذا( iMax<iMin ) iMax = iMin;
  إذا( iMax>=حجم(zBuf) ) iMax = حجم(zBuf)-1;
  n = iMin;
  إذا( iMax>iMin ){
    sqlite3_randomness(حجم(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  أكد( n<حجم(zBuf) );
  sqlite3_randomness(n, zBuf);
  لكل(i=0; i<n; i++){
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
ساكن صحيح test_destructor_گنتی_var = 0;
ساكن فراغ destructor(فراغ *p){
  حرف *zVal = (حرف *)p;
  أكد(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_گنتی_var--;
}
ساكن فراغ test_destructor(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  حرف *zVal;
  صحيح len;
  
  test_destructor_گنتی_var++;
  أكد( nArg==1 );
  إذا( sqlite3_قدر_type(argv[0])==SQLITE_NULL ) أرجع;
  len = sqlite3_قدر_بائٹاں(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  إذا( !zVal ){
    أرجع;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  حافظہ_نقل(zVal, sqlite3_قدر_متن(argv[0]), len);
  sqlite3_نتیجہ_متن(pCtx, zVal, -1, destructor);
}
#جے_بنا_تعریف SQLITE_OMIT_UTF16
ساكن فراغ test_destructor16(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  حرف *zVal;
  صحيح len;
  
  test_destructor_گنتی_var++;
  أكد( nArg==1 );
  إذا( sqlite3_قدر_type(argv[0])==SQLITE_NULL ) أرجع;
  len = sqlite3_قدر_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  إذا( !zVal ){
    أرجع;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  حافظہ_نقل(zVal, sqlite3_قدر_text16(argv[0]), len);
  sqlite3_نتیجہ_text16(pCtx, zVal, -1, destructor);
}
#جے_انت
ساكن فراغ test_destructor_گنتی(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_int(pCtx, test_destructor_گنتی_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#جے_بنا_تعریف SQLITE_UNTESTABLE
فراغ sqlite3BeginBenignMalloc(فراغ);
فراغ sqlite3_سرا_Benign_Malloc(فراغ);
#وإلا
  #تعریف_کرو sqlite3BeginBenignMalloc()
  #تعریف_کرو sqlite3_سرا_Benign_Malloc()
#جے_انت
ساكن فراغ test_agg_errmsg16_قدم(sqlite3_context *a, صحيح b,sqlite3_قدر **c){
}
ساكن فراغ test_agg_errmsg16_final(sqlite3_context *ctx){
#جے_بنا_تعریف SQLITE_OMIT_UTF16
  ثابت فراغ *z;
  sqlite3 * db = sqlite3_context_db_handle(ctx);
  sqlite3_aggregate_context(ctx, 2048);
  z = sqlite3_errmsg16(db);
  sqlite3_نتیجہ_text16(ctx, z, -1, SQLITE_TRANSIENT);
#جے_انت
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
ساكن فراغ free_test_auxdata(فراغ *p) {sqlite3_free(p);}
ساكن فراغ test_auxdata(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  صحيح i;
  حرف *zRet = testContextMalloc(pCtx, nArg*2);
  إذا( !zRet ) أرجع;
  memset(zRet, 0, nArg*2);
  لكل(i=0; i<nArg; i++){
    حرف ثابت *z = (حرف*)sqlite3_قدر_متن(argv[i]);
    إذا( z ){
      صحيح n;
      حرف *zAux = sqlite3_get_auxdata(pCtx, i);
      إذا( zAux ){
        zRet[i*2] = '1';
        أكد( strcmp(zAux,z)==0 );
      }وإلا {
        zRet[i*2] = '0';
      }
      n = (صحيح)لڑی_لمبائی(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      إذا( zAux ){
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
ساكن فراغ test_error(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_error(pCtx, (حرف*)sqlite3_قدر_متن(argv[0]), -1);
  إذا( nArg==2 ){
    sqlite3_نتیجہ_error_کوڈ(pCtx, sqlite3_قدر_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
ساكن فراغ counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  صحيح nArg,                /* Number of function arguments */
  sqlite3_قدر **argv     /* Values for all function arguments */
){
  صحيح *pCounter = (صحيح*)sqlite3_get_auxdata(pCtx, 0);
  إذا( pCounter==0 ){
    pCounter = sqlite3_malloc( حجم(*pCounter) );
    إذا( pCounter==0 ){
      sqlite3_نتیجہ_error_nomem(pCtx);
      أرجع;
    }
    *pCounter = sqlite3_قدر_int(argv[0]);
    sqlite3_سیٹ_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }وإلا{
    ++*pCounter;
  }
  sqlite3_نتیجہ_int(pCtx, *pCounter);
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
ساكن فراغ test_isolation(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
#جے_بنا_تعریف SQLITE_OMIT_UTF16
  sqlite3_قدر_text16(argv[0]);
  sqlite3_قدر_متن(argv[0]);
  sqlite3_قدر_text16(argv[0]);
  sqlite3_قدر_متن(argv[0]);
#جے_انت
  sqlite3_نتیجہ_قدر(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
ساكن فراغ test_eval(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  sqlite3_stmt *pStmt;
  صحيح rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ثابت حرف *zSql;
  ساكن ثابت حرف z_ٹیگ[] = "test_eval()";

  إذا( sqlite3_get_clientdata(db, z_ٹیگ) ){
    sqlite3_نتیجہ_error(pCtx, "nested test_eval()", -1);
    أرجع;
  }
  zSql = (حرف*)sqlite3_قدر_متن(argv[0]);
  sqlite3_سیٹ_clientdata(db, z_ٹیگ, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  إذا( rc==SQLITE_OK ){
    rc = sqlite3_قدم(pStmt);
    إذا( rc==SQLITE_صف ){
      sqlite3_نتیجہ_قدر(pCtx, sqlite3_کالم_قدر(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_سیٹ_clientdata(db, z_ٹیگ, 0, 0);
  إذا( rc ){
    حرف *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_نتیجہ_متن(pCtx, zErr, -1, sqlite3_free);
    sqlite3_نتیجہ_error_کوڈ(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
ساكن صحيح testHexChar(حرف c){
  إذا( c>='0' && c<='9' ){
    أرجع c - '0';
  }وإلا إذا( c>='a' && c<='f' ){
    أرجع c - 'a' + 10;
  }وإلا إذا( c>='A' && c<='F' ){
    أرجع c - 'A' + 10;
  }
  أرجع 0;
}

/*
** Convert hex to binary.
*/
ساكن فراغ test_Hex_منزل_Bin(ثابت حرف *z_وچ, حرف *z_باہرلا){
  طالما( z_وچ[0] && z_وچ[1] ){
    *(z_باہرلا++) = (testHexChar(z_وچ[0])<<4) + testHexChar(z_وچ[1]);
    z_وچ += 2;
  }
}

/*
**      hex_to_utf16be(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#جے_بنا_تعریف SQLITE_OMIT_UTF16
ساكن فراغ test_Hex_منزل_Utf16be(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  صحيح n;
  ثابت حرف *z_وچ;
  حرف *z_باہرلا;
  أكد( nArg==1 );
  n = sqlite3_قدر_بائٹاں(argv[0]);
  z_وچ = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  z_باہرلا = sqlite3_malloc( n/2 );
  إذا( z_باہرلا==0 ){
    sqlite3_نتیجہ_error_nomem(pCtx);
  }وإلا{
    test_Hex_منزل_Bin(z_وچ, z_باہرلا);
    sqlite3_نتیجہ_text16be(pCtx, z_باہرلا, n/2, sqlite3_free);
  }
}
#جے_انت

/*
**      hex_to_utf8(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
ساكن فراغ test_Hex_منزل_Utf8(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  صحيح n;
  ثابت حرف *z_وچ;
  حرف *z_باہرلا;
  أكد( nArg==1 );
  n = sqlite3_قدر_بائٹاں(argv[0]);
  z_وچ = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  z_باہرلا = sqlite3_malloc( n/2 );
  إذا( z_باہرلا==0 ){
    sqlite3_نتیجہ_error_nomem(pCtx);
  }وإلا{
    test_Hex_منزل_Bin(z_وچ, z_باہرلا);
    sqlite3_نتیجہ_متن(pCtx, z_باہرلا, n/2, sqlite3_free);
  }
}

/*
**      hex_to_utf16le(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#جے_بنا_تعریف SQLITE_OMIT_UTF16
ساكن فراغ test_Hex_منزل_Utf16le(
  sqlite3_context *pCtx, 
  صحيح nArg,
  sqlite3_قدر **argv
){
  صحيح n;
  ثابت حرف *z_وچ;
  حرف *z_باہرلا;
  أكد( nArg==1 );
  n = sqlite3_قدر_بائٹاں(argv[0]);
  z_وچ = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  z_باہرلا = sqlite3_malloc( n/2 );
  إذا( z_باہرلا==0 ){
    sqlite3_نتیجہ_error_nomem(pCtx);
  }وإلا{
    test_Hex_منزل_Bin(z_وچ, z_باہرلا);
    sqlite3_نتیجہ_text16le(pCtx, z_باہرلا, n/2, sqlite3_free);
  }
}
#جے_انت

/*
** SQL function:   real2hex(X)
**
** If argument X is a real number, then convert it into a string which is
** the big-endian hexadecimal representation of the ieee754 encoding of
** that number.  If X is not a real number, return NULL.
*/
ساكن فراغ real2hex(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  اتحاد {
    sqlite3_uint64 i;
    مزدوج r;
    بنانشانی حرف x[8];
  } v;
  حرف z_باہرلا[20];
  صحيح i;
  صحيح bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_قدر_double(argv[0]);
  لكل(i=0; i<8; i++){
    إذا( bigEndian ){
      z_باہرلا[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_باہرلا[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }وإلا{
      z_باہرلا[14-i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_باہرلا[14-i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }
  }
  z_باہرلا[16] = 0;
  sqlite3_نتیجہ_متن(context, z_باہرلا, -1, SQLITE_TRANSIENT);
}

/*
**     test_extract(record, field)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as the first argument. The
** second argument is the index of the field within that record to
** extract and return.
*/
ساكن فراغ test_کڈھو(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_سرا_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  صحيح iIdx;                       /* Required field */
  صحيح iCurrent = 0;               /* Current field */

  أكد( argc==2 );
  pRec = (u8*)sqlite3_قدر_blob(argv[0]);
  iIdx = sqlite3_قدر_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_سرا_Hdr = &pRec[nHdr];

  لكل(iCurrent=0; pHdr<p_سرا_Hdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, حجم(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    إذا( iCurrent==iIdx ){
      sqlite3_نتیجہ_قدر(context, &mem);
    }

    إذا( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
ساكن فراغ test_decode(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_سرا_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pRet);

  أكد( argc==1 );
  pRec = (u8*)sqlite3_قدر_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_سرا_Hdr = &pRec[nHdr];
  طالما( pHdr<p_سرا_Hdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, حجم(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    بدل( sqlite3_قدر_type(&mem) ){
      حالة SQLITE_متن:
        pVal = Tcl_NewStringObj((ثابت حرف*)sqlite3_قدر_متن(&mem), -1);
        اكسر;

      حالة SQLITE_BLOB: {
        حرف hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        صحيح n = sqlite3_قدر_بائٹاں(&mem);
        u8 *z = (u8*)sqlite3_قدر_blob(&mem);
        صحيح i;
        pVal = Tcl_NewStringObj("x'", -1);
        لكل(i=0; i<n; i++){
          حرف hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_Append_لڑیاں_منزل_Obj(pVal, hex, 0);
        }
        Tcl_Append_لڑیاں_منزل_Obj(pVal, "'", 0);
        اكسر;
      }

      حالة SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_قدر_double(&mem));
        اكسر;

      حالة SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_قدر_int64(&mem));
        اكسر;

      حالة SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        اكسر;

      افتراضي:
        أكد( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    إذا( mem.szMalloc ){
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
ساكن فراغ test_zeroblob(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  صحيح nZero = sqlite3_قدر_int(argv[0]);
  sqlite3_نتیجہ_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
ساكن فراغ test_getsubtype(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_int(context, (صحيح)sqlite3_قدر_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
ساكن فراغ test_frombind(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  sqlite3_uint64 m = 0;
  صحيح i;
  لكل(i=0; i<argc && i<63; i++){
    إذا( sqlite3_قدر_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_نتیجہ_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
ساكن فراغ test_setsubtype(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_قدر(context, argv[0]);
  sqlite3_نتیجہ_subtype(context, (بنانشانی صحيح)sqlite3_قدر_int(argv[1]));
}

ساكن صحيح رجسٹر_Test_Functions(
  sqlite3 *db,
  حرف **pz_Err_پیغام,
  ثابت sqlite3_api_routines *pThunk
){
  ساكن ثابت بنية {
     حرف *zName;
     نشانیوالا حرف nArg;
     بنانشانی صحيح e_متن_Rep; /* 1: UTF-16.  0: UTF-8 */
     فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_قدر **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#جے_بنا_تعریف SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, test_Hex_منزل_Utf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, test_Hex_منزل_Utf16le},
#جے_انت
    { "hex_to_utf8",           1, SQLITE_UTF8, test_Hex_منزل_Utf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_گنتی},
    { "test_auxdata",         -1, SQLITE_UTF8, test_auxdata},
    { "test_error",            1, SQLITE_UTF8, test_error},
    { "test_error",            2, SQLITE_UTF8, test_error},
    { "test_eval",             1, SQLITE_UTF8, test_eval},
    { "test_isolation",        2, SQLITE_UTF8, test_isolation},
    { "test_counter",          1, SQLITE_UTF8, counterFunc},
    { "real2hex",              1, SQLITE_UTF8, real2hex},
    { "test_decode",           1, SQLITE_UTF8, test_decode},
    { "test_extract",          2, SQLITE_UTF8, test_کڈھو},
    { "test_zeroblob",  1, SQLITE_UTF8|SQLITE_DETERMINISTIC, test_zeroblob},
    { "test_getsubtype",       1, SQLITE_UTF8, test_getsubtype},
    { "test_setsubtype",       2, SQLITE_UTF8|SQLITE_نتیجہ_SUBTYPE,
                                               test_setsubtype},
    { "test_frombind",        -1, SQLITE_UTF8, test_frombind},
  };
  صحيح i;

  لكل(i=0; i<حجم(aFuncs)/حجم(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].e_متن_Rep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_قدم, test_agg_errmsg16_final);
      
  أرجع SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
ساكن صحيح SQLITE_TCLAPI autoinstall_test_funcs(
  فراغ * client_انکڑے,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  خارجي صحيح Md5_رجسٹر(sqlite3 *, حرف **, ثابت sqlite3_api_routines *);
  صحيح rc = sqlite3_auto_extension((فراغ(*)(فراغ))رجسٹر_Test_Functions);
  إذا( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((فراغ(*)(فراغ))Md5_رجسٹر);
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
ساكن فراغ t_قدم(sqlite3_context *a, صحيح b, sqlite3_قدر **c){}
ساكن فراغ tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
ساكن صحيح SQLITE_TCLAPI abuse_create_function(
  فراغ * client_انکڑے,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  خارجي صحيح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  sqlite3 *db;
  صحيح rc;
  صحيح mxArg;

  إذا( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_قدم,t_قدم,tFinal);
  إذا( rc!=SQLITE_MISUSE ) اذهب abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_قدم, t_قدم, 0);
  إذا( rc!=SQLITE_MISUSE ) اذهب abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_قدم, 0, tFinal);
  إذا( rc!=SQLITE_MISUSE) اذهب abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  إذا( rc!=SQLITE_MISUSE ) اذهب abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, t_قدم, 0);
  إذا( rc!=SQLITE_MISUSE ) اذهب abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, t_قدم, 0, 0);
  إذا( rc!=SQLITE_MISUSE ) اذهب abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, t_قدم, 0, 0);
  إذا( rc!=SQLITE_MISUSE ) اذهب abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, t_قدم, 0, 0);
  إذا( rc!=SQLITE_MISUSE ) اذهب abuse_err;

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
  إذا( rc!=SQLITE_OK ) اذهب abuse_err;
                                
  أرجع TCL_OK;

abuse_err:
  Tcl_Append_نتیجہ(interp, "sqlite3_create_function abused test failed", 
                   (حرف*)0);
  أرجع TCL_ERROR;
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
ساكن فراغ rankfunc(sqlite3_context *pCtx, صحيح nVal, sqlite3_قدر **apVal){
  صحيح *aMatchinfo;                /* Return value of matchinfo() */
  صحيح nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  صحيح n_خانہ = 0;                   /* Number of columns in the table */
  صحيح nPhrase = 0;                /* Number of phrases in the query */
  صحيح iPhrase;                    /* Current phrase */
  مزدوج score = 0.0;             /* Value to return */

  أكد( حجم(صحيح)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  إذا( nVal<1 ) اذهب wrong_نمبر_دلیلاں;
  aMatchinfo = (صحيح*)sqlite3_قدر_blob(apVal[0]);
  nMatchinfo = sqlite3_قدر_بائٹاں(apVal[0]) / حجم(صحيح);
  إذا( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    n_خانہ = aMatchinfo[1];
  }
  إذا( nMatchinfo!=(2+3*n_خانہ*nPhrase) ){
    sqlite3_نتیجہ_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    أرجع;
  }
  إذا( nVal!=(1+n_خانہ) ) اذهب wrong_نمبر_دلیلاں;

  /* Iterate through each phrase in the users query. */
  لكل(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    صحيح i_خانہ;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    صحيح *aPhraseinfo = &aMatchinfo[2 + iPhrase*n_خانہ*3];
    لكل(i_خانہ=0; i_خانہ<n_خانہ; i_خانہ++){
      صحيح n_کامیابی_گنتی = aPhraseinfo[3*i_خانہ];
      صحيح n_Global_کامیابی_گنتی = aPhraseinfo[3*i_خانہ+1];
      مزدوج بھار = sqlite3_قدر_double(apVal[i_خانہ+1]);
      إذا( n_کامیابی_گنتی>0 ){
        score += ((مزدوج)n_کامیابی_گنتی / (مزدوج)n_Global_کامیابی_گنتی) * بھار;
      }
    }
  }

  sqlite3_نتیجہ_double(pCtx, score);
  أرجع;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_نمبر_دلیلاں:
  sqlite3_نتیجہ_error(pCtx, "wrong number of arguments to function rank()", -1);
}

ساكن صحيح SQLITE_TCLAPI install_fts3_rank_function(
  فراغ * client_انکڑے,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  خارجي صحيح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  sqlite3 *db;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلاں(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }

  إذا( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  أرجع TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest_func_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  صحيح i;
  خارجي صحيح Md5_رجسٹر(sqlite3 *, حرف **, ثابت sqlite3_api_routines *);

  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((فراغ(*)(فراغ))رجسٹر_Test_Functions);
  sqlite3_auto_extension((فراغ(*)(فراغ))Md5_رجسٹر);
  أرجع TCL_OK;
}
