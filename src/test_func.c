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
#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>

#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
স্থির শূন্য *testContextMalloc(sqlite3_context *context, পূর্ণসংখ্যা nByte){
  অক্ষর *z = sqlite3_malloc(nByte);
  যদি( !z && nByte>0 ){
    sqlite3_ফলাফল_error_nomem(context);
  }
  ফেরত z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
স্থির শূন্য randStr(sqlite3_context *context, পূর্ণসংখ্যা argc, sqlite3_মান **argv){
  স্থির ধ্রুবক unsigned অক্ষর zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  পূর্ণসংখ্যা iMin, iMax, n, r, i;
  unsigned অক্ষর zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  প্রতিজ্ঞা(argc==2);

  iMin = sqlite3_মান_int(argv[0]);
  যদি( iMin<0 ) iMin = 0;
  যদি( iMin>=আকার(zBuf) ) iMin = আকার(zBuf)-1;
  iMax = sqlite3_মান_int(argv[1]);
  যদি( iMax<iMin ) iMax = iMin;
  যদি( iMax>=আকার(zBuf) ) iMax = আকার(zBuf)-1;
  n = iMin;
  যদি( iMax>iMin ){
    sqlite3_randomness(আকার(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  প্রতিজ্ঞা( n<আকার(zBuf) );
  sqlite3_randomness(n, zBuf);
  জন্য(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(আকার(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_ফলাফল_text(context, (অক্ষর*)zBuf, n, SQLITE_TRANSIENT);
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
স্থির পূর্ণসংখ্যা test_destructor_গুনতি_var = 0;
স্থির শূন্য destructor(শূন্য *p){
  অক্ষর *zVal = (অক্ষর *)p;
  প্রতিজ্ঞা(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_গুনতি_var--;
}
স্থির শূন্য test_destructor(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  অক্ষর *zVal;
  পূর্ণসংখ্যা len;
  
  test_destructor_গুনতি_var++;
  প্রতিজ্ঞা( nArg==1 );
  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ) ফেরত;
  len = sqlite3_মান_bytes(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  যদি( !zVal ){
    ফেরত;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_মান_text(argv[0]), len);
  sqlite3_ফলাফল_text(pCtx, zVal, -1, destructor);
}
#ifndef SQLITE_OMIT_UTF16
স্থির শূন্য test_destructor16(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  অক্ষর *zVal;
  পূর্ণসংখ্যা len;
  
  test_destructor_গুনতি_var++;
  প্রতিজ্ঞা( nArg==1 );
  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ) ফেরত;
  len = sqlite3_মান_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  যদি( !zVal ){
    ফেরত;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_মান_text16(argv[0]), len);
  sqlite3_ফলাফল_text16(pCtx, zVal, -1, destructor);
}
#endif
স্থির শূন্য test_destructor_গুনতি(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  sqlite3_ফলাফল_int(pCtx, test_destructor_গুনতি_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ifndef SQLITE_UNTESTABLE
শূন্য sqlite3BeginBenignMalloc(শূন্য);
শূন্য sqlite3EndBenignMalloc(শূন্য);
#অন্যথা
  #define sqlite3BeginBenignMalloc()
  #define sqlite3EndBenignMalloc()
#endif
স্থির শূন্য test_agg_errmsg16_step(sqlite3_context *a, পূর্ণসংখ্যা b,sqlite3_মান **c){
}
স্থির শূন্য test_agg_errmsg16_final(sqlite3_context *ctx){
#ifndef SQLITE_OMIT_UTF16
  ধ্রুবক শূন্য *z;
  sqlite3 * db = sqlite3_context_db_handle(ctx);
  sqlite3_aggregate_context(ctx, 2048);
  z = sqlite3_errmsg16(db);
  sqlite3_ফলাফল_text16(ctx, z, -1, SQLITE_TRANSIENT);
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
স্থির শূন্য free_test_auxdata(শূন্য *p) {sqlite3_free(p);}
স্থির শূন্য test_auxdata(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা i;
  অক্ষর *zRet = testContextMalloc(pCtx, nArg*2);
  যদি( !zRet ) ফেরত;
  memset(zRet, 0, nArg*2);
  জন্য(i=0; i<nArg; i++){
    অক্ষর ধ্রুবক *z = (অক্ষর*)sqlite3_মান_text(argv[i]);
    যদি( z ){
      পূর্ণসংখ্যা n;
      অক্ষর *zAux = sqlite3_get_auxdata(pCtx, i);
      যদি( zAux ){
        zRet[i*2] = '1';
        প্রতিজ্ঞা( strcmp(zAux,z)==0 );
      }অন্যথা {
        zRet[i*2] = '0';
      }
      n = (পূর্ণসংখ্যা)strlen(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      যদি( zAux ){
        memcpy(zAux, z, n);
        sqlite3_set_auxdata(pCtx, i, zAux, free_test_auxdata);
      }
      zRet[i*2+1] = ' ';
    }
  }
  sqlite3_ফলাফল_text(pCtx, zRet, 2*nArg-1, free_test_auxdata);
}

/*
** A function to test error reporting from user functions. This function
** returns a copy of its first argument as the error message.  If the
** second argument exists, it becomes the error code.
*/
স্থির শূন্য test_error(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  sqlite3_ফলাফল_error(pCtx, (অক্ষর*)sqlite3_মান_text(argv[0]), -1);
  যদি( nArg==2 ){
    sqlite3_ফলাফল_error_code(pCtx, sqlite3_মান_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
স্থির শূন্য গণক_Func(
  sqlite3_context *pCtx,   /* Function context */
  পূর্ণসংখ্যা nArg,                /* Number of function arguments */
  sqlite3_মান **argv     /* Values for all function arguments */
){
  পূর্ণসংখ্যা *p_গণক = (পূর্ণসংখ্যা*)sqlite3_get_auxdata(pCtx, 0);
  যদি( p_গণক==0 ){
    p_গণক = sqlite3_malloc( আকার(*p_গণক) );
    যদি( p_গণক==0 ){
      sqlite3_ফলাফল_error_nomem(pCtx);
      ফেরত;
    }
    *p_গণক = sqlite3_মান_int(argv[0]);
    sqlite3_set_auxdata(pCtx, 0, p_গণক, sqlite3_free);
  }অন্যথা{
    ++*p_গণক;
  }
  sqlite3_ফলাফল_int(pCtx, *p_গণক);
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
স্থির শূন্য test_isolation(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3_মান_text16(argv[0]);
  sqlite3_মান_text(argv[0]);
  sqlite3_মান_text16(argv[0]);
  sqlite3_মান_text(argv[0]);
#endif
  sqlite3_ফলাফল_মান(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
স্থির শূন্য test_eval(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  sqlite3_stmt *pStmt;
  পূর্ণসংখ্যা rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  ধ্রুবক অক্ষর *zSql;
  স্থির ধ্রুবক অক্ষর zTag[] = "test_eval()";

  যদি( sqlite3_get_clientdata(db, zTag) ){
    sqlite3_ফলাফল_error(pCtx, "nested test_eval()", -1);
    ফেরত;
  }
  zSql = (অক্ষর*)sqlite3_মান_text(argv[0]);
  sqlite3_set_clientdata(db, zTag, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  যদি( rc==SQLITE_OK ){
    rc = sqlite3_step(pStmt);
    যদি( rc==SQLITE_ROW ){
      sqlite3_ফলাফল_মান(pCtx, sqlite3_column_মান(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_set_clientdata(db, zTag, 0, 0);
  যদি( rc ){
    অক্ষর *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_ফলাফল_text(pCtx, zErr, -1, sqlite3_free);
    sqlite3_ফলাফল_error_code(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
স্থির পূর্ণসংখ্যা testHexChar(অক্ষর c){
  যদি( c>='0' && c<='9' ){
    ফেরত c - '0';
  }অন্যথা যদি( c>='a' && c<='f' ){
    ফেরত c - 'a' + 10;
  }অন্যথা যদি( c>='A' && c<='F' ){
    ফেরত c - 'A' + 10;
  }
  ফেরত 0;
}

/*
** Convert hex to binary.
*/
স্থির শূন্য testHexToBin(ধ্রুবক অক্ষর *zIn, অক্ষর *zOut){
  যখন( zIn[0] && zIn[1] ){
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
স্থির শূন্য testHexToUtf16be(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা n;
  ধ্রুবক অক্ষর *zIn;
  অক্ষর *zOut;
  প্রতিজ্ঞা( nArg==1 );
  n = sqlite3_মান_bytes(argv[0]);
  zIn = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  যদি( zOut==0 ){
    sqlite3_ফলাফল_error_nomem(pCtx);
  }অন্যথা{
    testHexToBin(zIn, zOut);
    sqlite3_ফলাফল_text16be(pCtx, zOut, n/2, sqlite3_free);
  }
}
#endif

/*
**      hex_to_utf8(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
স্থির শূন্য testHexToUtf8(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা n;
  ধ্রুবক অক্ষর *zIn;
  অক্ষর *zOut;
  প্রতিজ্ঞা( nArg==1 );
  n = sqlite3_মান_bytes(argv[0]);
  zIn = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  যদি( zOut==0 ){
    sqlite3_ফলাফল_error_nomem(pCtx);
  }অন্যথা{
    testHexToBin(zIn, zOut);
    sqlite3_ফলাফল_text(pCtx, zOut, n/2, sqlite3_free);
  }
}

/*
**      hex_to_utf16le(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#ifndef SQLITE_OMIT_UTF16
স্থির শূন্য testHexToUtf16le(
  sqlite3_context *pCtx, 
  পূর্ণসংখ্যা nArg,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা n;
  ধ্রুবক অক্ষর *zIn;
  অক্ষর *zOut;
  প্রতিজ্ঞা( nArg==1 );
  n = sqlite3_মান_bytes(argv[0]);
  zIn = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  যদি( zOut==0 ){
    sqlite3_ফলাফল_error_nomem(pCtx);
  }অন্যথা{
    testHexToBin(zIn, zOut);
    sqlite3_ফলাফল_text16le(pCtx, zOut, n/2, sqlite3_free);
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
স্থির শূন্য real2hex(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  সংঘ {
    sqlite3_uint64 i;
    দ্বিগুণ r;
    unsigned অক্ষর x[8];
  } v;
  অক্ষর zOut[20];
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_মান_double(argv[0]);
  জন্য(i=0; i<8; i++){
    যদি( bigEndian ){
      zOut[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }অন্যথা{
      zOut[14-i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[14-i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }
  }
  zOut[16] = 0;
  sqlite3_ফলাফল_text(context, zOut, -1, SQLITE_TRANSIENT);
}

/*
**     test_extract(record, field)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as the first argument. The
** second argument is the index of the field within that record to
** extract and return.
*/
স্থির শূন্য test_extract(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  পূর্ণসংখ্যা iIdx;                       /* Required field */
  পূর্ণসংখ্যা iCurrent = 0;               /* Current field */

  প্রতিজ্ঞা( argc==2 );
  pRec = (u8*)sqlite3_মান_blob(argv[0]);
  iIdx = sqlite3_মান_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];

  জন্য(iCurrent=0; pHdr<pEndHdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, আকার(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    যদি( iCurrent==iIdx ){
      sqlite3_ফলাফল_মান(context, &mem);
    }

    যদি( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
স্থির শূন্য test_decode(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pRet);

  প্রতিজ্ঞা( argc==1 );
  pRec = (u8*)sqlite3_মান_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];
  যখন( pHdr<pEndHdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, আকার(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    নির্বাচন( sqlite3_মান_type(&mem) ){
      ক্ষেত্র SQLITE_TEXT:
        pVal = Tcl_NewStringObj((ধ্রুবক অক্ষর*)sqlite3_মান_text(&mem), -1);
        বিরতি;

      ক্ষেত্র SQLITE_BLOB: {
        অক্ষর hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        পূর্ণসংখ্যা n = sqlite3_মান_bytes(&mem);
        u8 *z = (u8*)sqlite3_মান_blob(&mem);
        পূর্ণসংখ্যা i;
        pVal = Tcl_NewStringObj("x'", -1);
        জন্য(i=0; i<n; i++){
          অক্ষর hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_AppendStringsToObj(pVal, hex, 0);
        }
        Tcl_AppendStringsToObj(pVal, "'", 0);
        বিরতি;
      }

      ক্ষেত্র SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_মান_double(&mem));
        বিরতি;

      ক্ষেত্র SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_মান_int64(&mem));
        বিরতি;

      ক্ষেত্র SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        বিরতি;

      পূর্বনির্ধারিত:
        প্রতিজ্ঞা( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    যদি( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_ফলাফল_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_গুনতি(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
স্থির শূন্য test_zeroblob(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা nZero = sqlite3_মান_int(argv[0]);
  sqlite3_ফলাফল_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
স্থির শূন্য test_getsubtype(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  sqlite3_ফলাফল_int(context, (পূর্ণসংখ্যা)sqlite3_মান_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
স্থির শূন্য test_frombind(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  sqlite3_uint64 m = 0;
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<argc && i<63; i++){
    যদি( sqlite3_মান_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_ফলাফল_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
স্থির শূন্য test_setsubtype(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  sqlite3_ফলাফল_মান(context, argv[0]);
  sqlite3_ফলাফল_subtype(context, (unsigned পূর্ণসংখ্যা)sqlite3_মান_int(argv[1]));
}

স্থির পূর্ণসংখ্যা registerTestFunctions(
  sqlite3 *db,
  অক্ষর **pz_Err_বার্তা,
  ধ্রুবক sqlite3_api_routines *pThunk
){
  স্থির ধ্রুবক কাঠামো {
     অক্ষর *zName;
     signed অক্ষর nArg;
     unsigned পূর্ণসংখ্যা eTextRep; /* 1: UTF-16.  0: UTF-8 */
     শূন্য (*xFunc)(sqlite3_context*,পূর্ণসংখ্যা,sqlite3_মান **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ifndef SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, testHexToUtf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, testHexToUtf16le},
#endif
    { "hex_to_utf8",           1, SQLITE_UTF8, testHexToUtf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_গুনতি},
    { "test_auxdata",         -1, SQLITE_UTF8, test_auxdata},
    { "test_error",            1, SQLITE_UTF8, test_error},
    { "test_error",            2, SQLITE_UTF8, test_error},
    { "test_eval",             1, SQLITE_UTF8, test_eval},
    { "test_isolation",        2, SQLITE_UTF8, test_isolation},
    { "test_counter",          1, SQLITE_UTF8, গণক_Func},
    { "real2hex",              1, SQLITE_UTF8, real2hex},
    { "test_decode",           1, SQLITE_UTF8, test_decode},
    { "test_extract",          2, SQLITE_UTF8, test_extract},
    { "test_zeroblob",  1, SQLITE_UTF8|SQLITE_DETERMINISTIC, test_zeroblob},
    { "test_getsubtype",       1, SQLITE_UTF8, test_getsubtype},
    { "test_setsubtype",       2, SQLITE_UTF8|SQLITE_ফলাফল_SUBTYPE,
                                               test_setsubtype},
    { "test_frombind",        -1, SQLITE_UTF8, test_frombind},
  };
  পূর্ণসংখ্যা i;

  জন্য(i=0; i<আকার(aFuncs)/আকার(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].eTextRep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_step, test_agg_errmsg16_final);
      
  ফেরত SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI autoinstall_test_funcs(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  বাহ্যিক পূর্ণসংখ্যা Md5_Register(sqlite3 *, অক্ষর **, ধ্রুবক sqlite3_api_routines *);
  পূর্ণসংখ্যা rc = sqlite3_auto_extension((শূন্য(*)(শূন্য))registerTestFunctions);
  যদি( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((শূন্য(*)(শূন্য))Md5_Register);
  }
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(rc));
  ফেরত TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
স্থির শূন্য tStep(sqlite3_context *a, পূর্ণসংখ্যা b, sqlite3_মান **c){}
স্থির শূন্য tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI abuse_create_function(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp*, ধ্রুবক অক্ষর*, sqlite3**);
  sqlite3 *db;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা mxArg;

  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep,tStep,tFinal);
  যদি( rc!=SQLITE_MISUSE ) যাও abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, tStep, 0);
  যদি( rc!=SQLITE_MISUSE ) যাও abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, 0, tFinal);
  যদি( rc!=SQLITE_MISUSE) যাও abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  যদি( rc!=SQLITE_MISUSE ) যাও abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, tStep, 0);
  যদি( rc!=SQLITE_MISUSE ) যাও abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, tStep, 0, 0);
  যদি( rc!=SQLITE_MISUSE ) যাও abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, tStep, 0, 0);
  যদি( rc!=SQLITE_MISUSE ) যাও abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, tStep, 0, 0);
  যদি( rc!=SQLITE_MISUSE ) যাও abuse_err;

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
  যদি( rc!=SQLITE_OK ) যাও abuse_err;
                                
  ফেরত TCL_OK;

abuse_err:
  Tcl_Append_ফলাফল(interp, "sqlite3_create_function abused test failed", 
                   (অক্ষর*)0);
  ফেরত TCL_ERROR;
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
স্থির শূন্য rankfunc(sqlite3_context *pCtx, পূর্ণসংখ্যা nVal, sqlite3_মান **apVal){
  পূর্ণসংখ্যা *aMatchinfo;                /* Return value of matchinfo() */
  পূর্ণসংখ্যা nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  পূর্ণসংখ্যা nCol = 0;                   /* Number of columns in the table */
  পূর্ণসংখ্যা nPhrase = 0;                /* Number of phrases in the query */
  পূর্ণসংখ্যা iPhrase;                    /* Current phrase */
  দ্বিগুণ score = 0.0;             /* Value to return */

  প্রতিজ্ঞা( আকার(পূর্ণসংখ্যা)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  যদি( nVal<1 ) যাও wrong_সংখ্যা_args;
  aMatchinfo = (পূর্ণসংখ্যা*)sqlite3_মান_blob(apVal[0]);
  nMatchinfo = sqlite3_মান_bytes(apVal[0]) / আকার(পূর্ণসংখ্যা);
  যদি( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
  }
  যদি( nMatchinfo!=(2+3*nCol*nPhrase) ){
    sqlite3_ফলাফল_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    ফেরত;
  }
  যদি( nVal!=(1+nCol) ) যাও wrong_সংখ্যা_args;

  /* Iterate through each phrase in the users query. */
  জন্য(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    পূর্ণসংখ্যা iCol;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    পূর্ণসংখ্যা *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
    জন্য(iCol=0; iCol<nCol; iCol++){
      পূর্ণসংখ্যা n_Hit_গুনতি = aPhraseinfo[3*iCol];
      পূর্ণসংখ্যা n_Global_Hit_গুনতি = aPhraseinfo[3*iCol+1];
      দ্বিগুণ weight = sqlite3_মান_double(apVal[iCol+1]);
      যদি( n_Hit_গুনতি>0 ){
        score += ((দ্বিগুণ)n_Hit_গুনতি / (দ্বিগুণ)n_Global_Hit_গুনতি) * weight;
      }
    }
  }

  sqlite3_ফলাফল_double(pCtx, score);
  ফেরত;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_সংখ্যা_args:
  sqlite3_ফলাফল_error(pCtx, "wrong number of arguments to function rank()", -1);
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI install_fts3_rank_function(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp*, ধ্রুবক অক্ষর*, sqlite3**);
  sqlite3 *db;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }

  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  ফেরত TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest_func_Init(Tcl_Interp *interp){
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  পূর্ণসংখ্যা i;
  বাহ্যিক পূর্ণসংখ্যা Md5_Register(sqlite3 *, অক্ষর **, ধ্রুবক sqlite3_api_routines *);

  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((শূন্য(*)(শূন্য))registerTestFunctions);
  sqlite3_auto_extension((শূন্য(*)(শূন্য))Md5_Register);
  ফেরত TCL_OK;
}
