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
#include "sqlite3.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sqliteInt.h"
#include "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
static void *testContextMalloc(sqlite3_context *context, ཧྲིལ nByte){
  var_na *z = sqlite3_malloc(nByte);
  གལ་སྲིད( !z && nByte>0 ){
    sqlite3_result_error_nomem(context);
  }
  ལོག z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
static void randStr(sqlite3_context *context, ཧྲིལ argc, sqlite3_value **argv){
  static const unsigned var_na zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  ཧྲིལ iMin, iMax, n, r, i;
  unsigned var_na zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  assert(argc==2);

  iMin = sqlite3_value_int(argv[0]);
  གལ་སྲིད( iMin<0 ) iMin = 0;
  གལ་སྲིད( iMin>=parimaa_na(zBuf) ) iMin = parimaa_na(zBuf)-1;
  iMax = sqlite3_value_int(argv[1]);
  གལ་སྲིད( iMax<iMin ) iMax = iMin;
  གལ་སྲིད( iMax>=parimaa_na(zBuf) ) iMax = parimaa_na(zBuf)-1;
  n = iMin;
  གལ་སྲིད( iMax>iMin ){
    sqlite3_randomness(parimaa_na(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  assert( n<parimaa_na(zBuf) );
  sqlite3_randomness(n, zBuf);
  དོན་ལུ(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(parimaa_na(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_result_text(context, (var_na*)zBuf, n, SQLITE_TRANSIENT);
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
static ཧྲིལ test_destructor_count_var = 0;
static void destructor(void *p){
  var_na *zVal = (var_na *)p;
  assert(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_count_var--;
}
static void test_destructor(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  var_na *zVal;
  ཧྲིལ len;
  
  test_destructor_count_var++;
  assert( nArg==1 );
  གལ་སྲིད( sqlite3_value_type(argv[0])==SQLITE_NULL ) ལོག;
  len = sqlite3_value_bytes(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  གལ་སྲིད( !zVal ){
    ལོག;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_value_text(argv[0]), len);
  sqlite3_result_text(pCtx, zVal, -1, destructor);
}
#ifndef SQLITE_OMIT_UTF16
static void test_destructor16(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  var_na *zVal;
  ཧྲིལ len;
  
  test_destructor_count_var++;
  assert( nArg==1 );
  གལ་སྲིད( sqlite3_value_type(argv[0])==SQLITE_NULL ) ལོག;
  len = sqlite3_value_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  གལ་སྲིད( !zVal ){
    ལོག;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_value_text16(argv[0]), len);
  sqlite3_result_text16(pCtx, zVal, -1, destructor);
}
#endif
static void test_destructor_count(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  sqlite3_result_int(pCtx, test_destructor_count_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ifndef SQLITE_UNTESTABLE
void sqlite3BeginBenignMalloc(void);
void sqlite3EndBenignMalloc(void);
#དེ་མིན
  #define sqlite3BeginBenignMalloc()
  #define sqlite3EndBenignMalloc()
#endif
static void test_agg_errmsg16_step(sqlite3_context *a, ཧྲིལ b,sqlite3_value **c){
}
static void test_agg_errmsg16_final(sqlite3_context *ctx){
#ifndef SQLITE_OMIT_UTF16
  const void *z;
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
static void free_test_auxdata(void *p) {sqlite3_free(p);}
static void test_auxdata(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  ཧྲིལ i;
  var_na *zRet = testContextMalloc(pCtx, nArg*2);
  གལ་སྲིད( !zRet ) ལོག;
  memset(zRet, 0, nArg*2);
  དོན་ལུ(i=0; i<nArg; i++){
    var_na const *z = (var_na*)sqlite3_value_text(argv[i]);
    གལ་སྲིད( z ){
      ཧྲིལ n;
      var_na *zAux = sqlite3_get_auxdata(pCtx, i);
      གལ་སྲིད( zAux ){
        zRet[i*2] = '1';
        assert( strcmp(zAux,z)==0 );
      }དེ་མིན {
        zRet[i*2] = '0';
      }
      n = (ཧྲིལ)strlen(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      གལ་སྲིད( zAux ){
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
static void test_error(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  sqlite3_result_error(pCtx, (var_na*)sqlite3_value_text(argv[0]), -1);
  གལ་སྲིད( nArg==2 ){
    sqlite3_result_error_code(pCtx, sqlite3_value_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
static void counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  ཧྲིལ nArg,                /* Number of function arguments */
  sqlite3_value **argv     /* Values for all function arguments */
){
  ཧྲིལ *pCounter = (ཧྲིལ*)sqlite3_get_auxdata(pCtx, 0);
  གལ་སྲིད( pCounter==0 ){
    pCounter = sqlite3_malloc( parimaa_na(*pCounter) );
    གལ་སྲིད( pCounter==0 ){
      sqlite3_result_error_nomem(pCtx);
      ལོག;
    }
    *pCounter = sqlite3_value_int(argv[0]);
    sqlite3_set_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }དེ་མིན{
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
static void test_isolation(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3_value_text16(argv[0]);
  sqlite3_value_text(argv[0]);
  sqlite3_value_text16(argv[0]);
  sqlite3_value_text(argv[0]);
#endif
  sqlite3_result_value(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
static void test_eval(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  sqlite3_stmt *pStmt;
  ཧྲིལ rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  const var_na *zSql;
  static const var_na zTag[] = "test_eval()";

  གལ་སྲིད( sqlite3_get_clientdata(db, zTag) ){
    sqlite3_result_error(pCtx, "nested test_eval()", -1);
    ལོག;
  }
  zSql = (var_na*)sqlite3_value_text(argv[0]);
  sqlite3_set_clientdata(db, zTag, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  གལ་སྲིད( rc==SQLITE_OK ){
    rc = sqlite3_step(pStmt);
    གལ་སྲིད( rc==SQLITE_ROW ){
      sqlite3_result_value(pCtx, sqlite3_column_value(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_set_clientdata(db, zTag, 0, 0);
  གལ་སྲིད( rc ){
    var_na *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_result_text(pCtx, zErr, -1, sqlite3_free);
    sqlite3_result_error_code(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
static ཧྲིལ testHexChar(var_na c){
  གལ་སྲིད( c>='0' && c<='9' ){
    ལོག c - '0';
  }དེ་མིན གལ་སྲིད( c>='a' && c<='f' ){
    ལོག c - 'a' + 10;
  }དེ་མིན གལ་སྲིད( c>='A' && c<='F' ){
    ལོག c - 'A' + 10;
  }
  ལོག 0;
}

/*
** Convert hex to binary.
*/
static void testHexToBin(const var_na *zIn, var_na *zOut){
  སྐབས( zIn[0] && zIn[1] ){
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
static void testHexToUtf16be(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  ཧྲིལ n;
  const var_na *zIn;
  var_na *zOut;
  assert( nArg==1 );
  n = sqlite3_value_bytes(argv[0]);
  zIn = (const var_na*)sqlite3_value_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  གལ་སྲིད( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }དེ་མིན{
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
static void testHexToUtf8(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  ཧྲིལ n;
  const var_na *zIn;
  var_na *zOut;
  assert( nArg==1 );
  n = sqlite3_value_bytes(argv[0]);
  zIn = (const var_na*)sqlite3_value_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  གལ་སྲིད( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }དེ་མིན{
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
static void testHexToUtf16le(
  sqlite3_context *pCtx, 
  ཧྲིལ nArg,
  sqlite3_value **argv
){
  ཧྲིལ n;
  const var_na *zIn;
  var_na *zOut;
  assert( nArg==1 );
  n = sqlite3_value_bytes(argv[0]);
  zIn = (const var_na*)sqlite3_value_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  གལ་སྲིད( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }དེ་མིན{
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
static void real2hex(
  sqlite3_context *context,
  ཧྲིལ argc,
  sqlite3_value **argv
){
  sa_ngha {
    sqlite3_uint64 i;
    dviguu_na r;
    unsigned var_na x[8];
  } v;
  var_na zOut[20];
  ཧྲིལ i;
  ཧྲིལ bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_value_double(argv[0]);
  དོན་ལུ(i=0; i<8; i++){
    གལ་སྲིད( bigEndian ){
      zOut[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }དེ་མིན{
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
static void test_extract(
  sqlite3_context *context,
  ཧྲིལ argc,
  sqlite3_value **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  ཧྲིལ iIdx;                       /* Required field */
  ཧྲིལ iCurrent = 0;               /* Current field */

  assert( argc==2 );
  pRec = (u8*)sqlite3_value_blob(argv[0]);
  iIdx = sqlite3_value_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];

  དོན་ལུ(iCurrent=0; pHdr<pEndHdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, parimaa_na(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    གལ་སྲིད( iCurrent==iIdx ){
      sqlite3_result_value(context, &mem);
    }

    གལ་སྲིད( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
static void test_decode(
  sqlite3_context *context,
  ཧྲིལ argc,
  sqlite3_value **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_IncrRefCount(pRet);

  assert( argc==1 );
  pRec = (u8*)sqlite3_value_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];
  སྐབས( pHdr<pEndHdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, parimaa_na(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    switch( sqlite3_value_type(&mem) ){
      pak_sa SQLITE_TEXT:
        pVal = Tcl_NewStringObj((const var_na*)sqlite3_value_text(&mem), -1);
        bha_nga;

      pak_sa SQLITE_BLOB: {
        var_na hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        ཧྲིལ n = sqlite3_value_bytes(&mem);
        u8 *z = (u8*)sqlite3_value_blob(&mem);
        ཧྲིལ i;
        pVal = Tcl_NewStringObj("x'", -1);
        དོན་ལུ(i=0; i<n; i++){
          var_na hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_AppendStringsToObj(pVal, hex, 0);
        }
        Tcl_AppendStringsToObj(pVal, "'", 0);
        bha_nga;
      }

      pak_sa SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_value_double(&mem));
        bha_nga;

      pak_sa SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_value_int64(&mem));
        bha_nga;

      pak_sa SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        bha_nga;

      default:
        assert( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    གལ་སྲིད( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_result_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_DecrRefCount(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
static void test_zeroblob(
  sqlite3_context *context,
  ཧྲིལ argc,
  sqlite3_value **argv
){
  ཧྲིལ nZero = sqlite3_value_int(argv[0]);
  sqlite3_result_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
static void test_getsubtype(
  sqlite3_context *context,
  ཧྲིལ argc,
  sqlite3_value **argv
){
  sqlite3_result_int(context, (ཧྲིལ)sqlite3_value_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
static void test_frombind(
  sqlite3_context *context,
  ཧྲིལ argc,
  sqlite3_value **argv
){
  sqlite3_uint64 m = 0;
  ཧྲིལ i;
  དོན་ལུ(i=0; i<argc && i<63; i++){
    གལ་སྲིད( sqlite3_value_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_result_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
static void test_setsubtype(
  sqlite3_context *context,
  ཧྲིལ argc,
  sqlite3_value **argv
){
  sqlite3_result_value(context, argv[0]);
  sqlite3_result_subtype(context, (unsigned ཧྲིལ)sqlite3_value_int(argv[1]));
}

static ཧྲིལ registerTestFunctions(
  sqlite3 *db,
  var_na **pzErrMsg,
  const sqlite3_api_routines *pThunk
){
  static const sa_nracanaa {
     var_na *zName;
     signed var_na nArg;
     unsigned ཧྲིལ eTextRep; /* 1: UTF-16.  0: UTF-8 */
     void (*xFunc)(sqlite3_context*,ཧྲིལ,sqlite3_value **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ifndef SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, testHexToUtf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, testHexToUtf16le},
#endif
    { "hex_to_utf8",           1, SQLITE_UTF8, testHexToUtf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_count},
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
  ཧྲིལ i;

  དོན་ལུ(i=0; i<parimaa_na(aFuncs)/parimaa_na(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].eTextRep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_step, test_agg_errmsg16_final);
      
  ལོག SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
static ཧྲིལ SQLITE_TCLAPI autoinstall_test_funcs(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  extern ཧྲིལ Md5_Register(sqlite3 *, var_na **, const sqlite3_api_routines *);
  ཧྲིལ rc = sqlite3_auto_extension((void(*)(void))registerTestFunctions);
  གལ་སྲིད( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((void(*)(void))Md5_Register);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ལོག TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
static void tStep(sqlite3_context *a, ཧྲིལ b, sqlite3_value **c){}
static void tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
static ཧྲིལ SQLITE_TCLAPI abuse_create_function(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  extern ཧྲིལ getDbPointer(Tcl_Interp*, const var_na*, sqlite3**);
  sqlite3 *db;
  ཧྲིལ rc;
  ཧྲིལ mxArg;

  གལ་སྲིད( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ལོག TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep,tStep,tFinal);
  གལ་སྲིད( rc!=SQLITE_MISUSE ) goto abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, tStep, 0);
  གལ་སྲིད( rc!=SQLITE_MISUSE ) goto abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, 0, tFinal);
  གལ་སྲིད( rc!=SQLITE_MISUSE) goto abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  གལ་སྲིད( rc!=SQLITE_MISUSE ) goto abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, tStep, 0);
  གལ་སྲིད( rc!=SQLITE_MISUSE ) goto abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, tStep, 0, 0);
  གལ་སྲིད( rc!=SQLITE_MISUSE ) goto abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, tStep, 0, 0);
  གལ་སྲིད( rc!=SQLITE_MISUSE ) goto abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, tStep, 0, 0);
  གལ་སྲིད( rc!=SQLITE_MISUSE ) goto abuse_err;

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
  གལ་སྲིད( rc!=SQLITE_OK ) goto abuse_err;
                                
  ལོག TCL_OK;

abuse_err:
  Tcl_AppendResult(interp, "sqlite3_create_function abused test failed", 
                   (var_na*)0);
  ལོག TCL_ERROR;
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
static void rankfunc(sqlite3_context *pCtx, ཧྲིལ nVal, sqlite3_value **apVal){
  ཧྲིལ *aMatchinfo;                /* Return value of matchinfo() */
  ཧྲིལ nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  ཧྲིལ nCol = 0;                   /* Number of columns in the table */
  ཧྲིལ nPhrase = 0;                /* Number of phrases in the query */
  ཧྲིལ iPhrase;                    /* Current phrase */
  dviguu_na score = 0.0;             /* Value to return */

  assert( parimaa_na(ཧྲིལ)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  གལ་སྲིད( nVal<1 ) goto wrong_number_args;
  aMatchinfo = (ཧྲིལ*)sqlite3_value_blob(apVal[0]);
  nMatchinfo = sqlite3_value_bytes(apVal[0]) / parimaa_na(ཧྲིལ);
  གལ་སྲིད( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
  }
  གལ་སྲིད( nMatchinfo!=(2+3*nCol*nPhrase) ){
    sqlite3_result_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    ལོག;
  }
  གལ་སྲིད( nVal!=(1+nCol) ) goto wrong_number_args;

  /* Iterate through each phrase in the users query. */
  དོན་ལུ(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    ཧྲིལ iCol;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    ཧྲིལ *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
    དོན་ལུ(iCol=0; iCol<nCol; iCol++){
      ཧྲིལ nHitCount = aPhraseinfo[3*iCol];
      ཧྲིལ nGlobalHitCount = aPhraseinfo[3*iCol+1];
      dviguu_na weight = sqlite3_value_double(apVal[iCol+1]);
      གལ་སྲིད( nHitCount>0 ){
        score += ((dviguu_na)nHitCount / (dviguu_na)nGlobalHitCount) * weight;
      }
    }
  }

  sqlite3_result_double(pCtx, score);
  ལོག;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_number_args:
  sqlite3_result_error(pCtx, "wrong number of arguments to function rank()", -1);
}

static ཧྲིལ SQLITE_TCLAPI install_fts3_rank_function(
  void * clientData,
  Tcl_Interp *interp,
  ཧྲིལ objc,
  Tcl_Obj *CONST objv[]
){
  extern ཧྲིལ getDbPointer(Tcl_Interp*, const var_na*, sqlite3**);
  sqlite3 *db;

  གལ་སྲིད( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ལོག TCL_ERROR;
  }

  གལ་སྲིད( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ལོག TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  ལོག TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
ཧྲིལ Sqlitetest_func_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  ཧྲིལ i;
  extern ཧྲིལ Md5_Register(sqlite3 *, var_na **, const sqlite3_api_routines *);

  དོན་ལུ(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((void(*)(void))registerTestFunctions);
  sqlite3_auto_extension((void(*)(void))Md5_Register);
  ལོག TCL_OK;
}
