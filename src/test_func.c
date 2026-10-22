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
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
स्थिर शून्य *testContextMalloc(sqlite3_context *context, पूर्णांक nByte){
  अक्षर *z = sqlite3_malloc(nByte);
  जर( !z && nByte>0 ){
    sqlite3_result_error_nomem(context);
  }
  परत z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
स्थिर शून्य randStr(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  स्थिर स्थिरांक अचिन्हित अक्षर zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  पूर्णांक iMin, iMax, n, r, i;
  अचिन्हित अक्षर zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  प्रतिज्ञा(argc==2);

  iMin = sqlite3_मूल्य_int(argv[0]);
  जर( iMin<0 ) iMin = 0;
  जर( iMin>=आकार(zBuf) ) iMin = आकार(zBuf)-1;
  iMax = sqlite3_मूल्य_int(argv[1]);
  जर( iMax<iMin ) iMax = iMin;
  जर( iMax>=आकार(zBuf) ) iMax = आकार(zBuf)-1;
  n = iMin;
  जर( iMax>iMin ){
    sqlite3_randomness(आकार(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  प्रतिज्ञा( n<आकार(zBuf) );
  sqlite3_randomness(n, zBuf);
  साठी(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(आकार(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_result_text(context, (अक्षर*)zBuf, n, SQLITE_TRANSIENT);
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
स्थिर पूर्णांक test_destructor_मोजणी_var = 0;
स्थिर शून्य destructor(शून्य *p){
  अक्षर *zVal = (अक्षर *)p;
  प्रतिज्ञा(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_मोजणी_var--;
}
स्थिर शून्य test_destructor(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  अक्षर *zVal;
  पूर्णांक len;
  
  test_destructor_मोजणी_var++;
  प्रतिज्ञा( nArg==1 );
  जर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ) परत;
  len = sqlite3_मूल्य_bytes(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  जर( !zVal ){
    परत;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  स_नकल(zVal, sqlite3_मूल्य_text(argv[0]), len);
  sqlite3_result_text(pCtx, zVal, -1, destructor);
}
#ifndef SQLITE_OMIT_UTF16
स्थिर शून्य test_destructor16(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  अक्षर *zVal;
  पूर्णांक len;
  
  test_destructor_मोजणी_var++;
  प्रतिज्ञा( nArg==1 );
  जर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ) परत;
  len = sqlite3_मूल्य_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  जर( !zVal ){
    परत;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  स_नकल(zVal, sqlite3_मूल्य_text16(argv[0]), len);
  sqlite3_result_text16(pCtx, zVal, -1, destructor);
}
#endif
स्थिर शून्य test_destructor_मोजणी(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  sqlite3_result_int(pCtx, test_destructor_मोजणी_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ifndef SQLITE_UNTESTABLE
शून्य sqlite3BeginBenignMalloc(शून्य);
शून्य sqlite3EndBenignMalloc(शून्य);
#अन्यथा
  #define sqlite3BeginBenignMalloc()
  #define sqlite3EndBenignMalloc()
#endif
स्थिर शून्य test_agg_errmsg16_step(sqlite3_context *a, पूर्णांक b,sqlite3_मूल्य **c){
}
स्थिर शून्य test_agg_errmsg16_final(sqlite3_context *ctx){
#ifndef SQLITE_OMIT_UTF16
  स्थिरांक शून्य *z;
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
स्थिर शून्य free_test_auxdata(शून्य *p) {sqlite3_free(p);}
स्थिर शून्य test_auxdata(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;
  अक्षर *zRet = testContextMalloc(pCtx, nArg*2);
  जर( !zRet ) परत;
  स_रखो(zRet, 0, nArg*2);
  साठी(i=0; i<nArg; i++){
    अक्षर स्थिरांक *z = (अक्षर*)sqlite3_मूल्य_text(argv[i]);
    जर( z ){
      पूर्णांक n;
      अक्षर *zAux = sqlite3_get_auxdata(pCtx, i);
      जर( zAux ){
        zRet[i*2] = '1';
        प्रतिज्ञा( म_भेद(zAux,z)==0 );
      }अन्यथा {
        zRet[i*2] = '0';
      }
      n = (पूर्णांक)म_माप(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      जर( zAux ){
        स_नकल(zAux, z, n);
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
स्थिर शून्य test_error(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  sqlite3_result_error(pCtx, (अक्षर*)sqlite3_मूल्य_text(argv[0]), -1);
  जर( nArg==2 ){
    sqlite3_result_error_code(pCtx, sqlite3_मूल्य_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
स्थिर शून्य counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  पूर्णांक nArg,                /* Number of function arguments */
  sqlite3_मूल्य **argv     /* Values for all function arguments */
){
  पूर्णांक *pCounter = (पूर्णांक*)sqlite3_get_auxdata(pCtx, 0);
  जर( pCounter==0 ){
    pCounter = sqlite3_malloc( आकार(*pCounter) );
    जर( pCounter==0 ){
      sqlite3_result_error_nomem(pCtx);
      परत;
    }
    *pCounter = sqlite3_मूल्य_int(argv[0]);
    sqlite3_set_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }अन्यथा{
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
स्थिर शून्य test_isolation(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3_मूल्य_text16(argv[0]);
  sqlite3_मूल्य_text(argv[0]);
  sqlite3_मूल्य_text16(argv[0]);
  sqlite3_मूल्य_text(argv[0]);
#endif
  sqlite3_result_मूल्य(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
स्थिर शून्य test_eval(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  sqlite3_stmt *pStmt;
  पूर्णांक rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  स्थिरांक अक्षर *zSql;
  स्थिर स्थिरांक अक्षर zTag[] = "test_eval()";

  जर( sqlite3_get_clientdata(db, zTag) ){
    sqlite3_result_error(pCtx, "nested test_eval()", -1);
    परत;
  }
  zSql = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
  sqlite3_set_clientdata(db, zTag, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  जर( rc==SQLITE_OK ){
    rc = sqlite3_step(pStmt);
    जर( rc==SQLITE_ROW ){
      sqlite3_result_मूल्य(pCtx, sqlite3_column_मूल्य(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_set_clientdata(db, zTag, 0, 0);
  जर( rc ){
    अक्षर *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_result_text(pCtx, zErr, -1, sqlite3_free);
    sqlite3_result_error_code(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
स्थिर पूर्णांक testHexChar(अक्षर c){
  जर( c>='0' && c<='9' ){
    परत c - '0';
  }अन्यथा जर( c>='a' && c<='f' ){
    परत c - 'a' + 10;
  }अन्यथा जर( c>='A' && c<='F' ){
    परत c - 'A' + 10;
  }
  परत 0;
}

/*
** Convert hex to binary.
*/
स्थिर शून्य testHexToBin(स्थिरांक अक्षर *zIn, अक्षर *zOut){
  पर्यंत( zIn[0] && zIn[1] ){
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
स्थिर शून्य testHexToUtf16be(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  पूर्णांक n;
  स्थिरांक अक्षर *zIn;
  अक्षर *zOut;
  प्रतिज्ञा( nArg==1 );
  n = sqlite3_मूल्य_bytes(argv[0]);
  zIn = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  जर( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }अन्यथा{
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
स्थिर शून्य testHexToUtf8(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  पूर्णांक n;
  स्थिरांक अक्षर *zIn;
  अक्षर *zOut;
  प्रतिज्ञा( nArg==1 );
  n = sqlite3_मूल्य_bytes(argv[0]);
  zIn = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  जर( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }अन्यथा{
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
स्थिर शून्य testHexToUtf16le(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  पूर्णांक n;
  स्थिरांक अक्षर *zIn;
  अक्षर *zOut;
  प्रतिज्ञा( nArg==1 );
  n = sqlite3_मूल्य_bytes(argv[0]);
  zIn = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  जर( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }अन्यथा{
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
स्थिर शून्य real2hex(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  संघ {
    sqlite3_uint64 i;
    द्विगुण r;
    अचिन्हित अक्षर x[8];
  } v;
  अक्षर zOut[20];
  पूर्णांक i;
  पूर्णांक bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_मूल्य_double(argv[0]);
  साठी(i=0; i<8; i++){
    जर( bigEndian ){
      zOut[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }अन्यथा{
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
स्थिर शून्य test_extract(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  पूर्णांक iIdx;                       /* Required field */
  पूर्णांक iCurrent = 0;               /* Current field */

  प्रतिज्ञा( argc==2 );
  pRec = (u8*)sqlite3_मूल्य_blob(argv[0]);
  iIdx = sqlite3_मूल्य_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];

  साठी(iCurrent=0; pHdr<pEndHdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    स_रखो(&mem, 0, आकार(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    जर( iCurrent==iIdx ){
      sqlite3_result_मूल्य(context, &mem);
    }

    जर( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
स्थिर शून्य test_decode(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_मोजणी(pRet);

  प्रतिज्ञा( argc==1 );
  pRec = (u8*)sqlite3_मूल्य_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];
  पर्यंत( pHdr<pEndHdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    स_रखो(&mem, 0, आकार(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    निवड( sqlite3_मूल्य_type(&mem) ){
      स्थिती SQLITE_TEXT:
        pVal = Tcl_NewStringObj((स्थिरांक अक्षर*)sqlite3_मूल्य_text(&mem), -1);
        थांबा;

      स्थिती SQLITE_BLOB: {
        अक्षर hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        पूर्णांक n = sqlite3_मूल्य_bytes(&mem);
        u8 *z = (u8*)sqlite3_मूल्य_blob(&mem);
        पूर्णांक i;
        pVal = Tcl_NewStringObj("x'", -1);
        साठी(i=0; i<n; i++){
          अक्षर hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_AppendStringsToObj(pVal, hex, 0);
        }
        Tcl_AppendStringsToObj(pVal, "'", 0);
        थांबा;
      }

      स्थिती SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_मूल्य_double(&mem));
        थांबा;

      स्थिती SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_मूल्य_int64(&mem));
        थांबा;

      स्थिती SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        थांबा;

      पूर्वनिर्धारित:
        प्रतिज्ञा( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    जर( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_result_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_मोजणी(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
स्थिर शून्य test_zeroblob(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक nZero = sqlite3_मूल्य_int(argv[0]);
  sqlite3_result_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
स्थिर शून्य test_getsubtype(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_result_int(context, (पूर्णांक)sqlite3_मूल्य_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
स्थिर शून्य test_frombind(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_uint64 m = 0;
  पूर्णांक i;
  साठी(i=0; i<argc && i<63; i++){
    जर( sqlite3_मूल्य_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_result_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
स्थिर शून्य test_setsubtype(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_result_मूल्य(context, argv[0]);
  sqlite3_result_subtype(context, (अचिन्हित पूर्णांक)sqlite3_मूल्य_int(argv[1]));
}

स्थिर पूर्णांक registerTestFunctions(
  sqlite3 *db,
  अक्षर **pz_Err_संदेश,
  स्थिरांक sqlite3_api_routines *pThunk
){
  स्थिर स्थिरांक संरचना {
     अक्षर *zName;
     चिन्हित अक्षर nArg;
     अचिन्हित पूर्णांक eTextRep; /* 1: UTF-16.  0: UTF-8 */
     शून्य (*xFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ifndef SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, testHexToUtf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, testHexToUtf16le},
#endif
    { "hex_to_utf8",           1, SQLITE_UTF8, testHexToUtf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_मोजणी},
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
  पूर्णांक i;

  साठी(i=0; i<आकार(aFuncs)/आकार(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].eTextRep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_step, test_agg_errmsg16_final);
      
  परत SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
स्थिर पूर्णांक SQLITE_TCLAPI autoinstall_test_funcs(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णांक Md5_Register(sqlite3 *, अक्षर **, स्थिरांक sqlite3_api_routines *);
  पूर्णांक rc = sqlite3_auto_extension((शून्य(*)(शून्य))registerTestFunctions);
  जर( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((शून्य(*)(शून्य))Md5_Register);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  परत TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
स्थिर शून्य tStep(sqlite3_context *a, पूर्णांक b, sqlite3_मूल्य **c){}
स्थिर शून्य tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
स्थिर पूर्णांक SQLITE_TCLAPI abuse_create_function(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिरांक अक्षर*, sqlite3**);
  sqlite3 *db;
  पूर्णांक rc;
  पूर्णांक mxArg;

  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep,tStep,tFinal);
  जर( rc!=SQLITE_MISUSE ) जा abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, tStep, 0);
  जर( rc!=SQLITE_MISUSE ) जा abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, 0, tFinal);
  जर( rc!=SQLITE_MISUSE) जा abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  जर( rc!=SQLITE_MISUSE ) जा abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, tStep, 0);
  जर( rc!=SQLITE_MISUSE ) जा abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, tStep, 0, 0);
  जर( rc!=SQLITE_MISUSE ) जा abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, tStep, 0, 0);
  जर( rc!=SQLITE_MISUSE ) जा abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, tStep, 0, 0);
  जर( rc!=SQLITE_MISUSE ) जा abuse_err;

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
  जर( rc!=SQLITE_OK ) जा abuse_err;
                                
  परत TCL_OK;

abuse_err:
  Tcl_AppendResult(interp, "sqlite3_create_function abused test failed", 
                   (अक्षर*)0);
  परत TCL_ERROR;
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
स्थिर शून्य rankfunc(sqlite3_context *pCtx, पूर्णांक nVal, sqlite3_मूल्य **apVal){
  पूर्णांक *aMatchinfo;                /* Return value of matchinfo() */
  पूर्णांक nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  पूर्णांक nCol = 0;                   /* Number of columns in the table */
  पूर्णांक nPhrase = 0;                /* Number of phrases in the query */
  पूर्णांक iPhrase;                    /* Current phrase */
  द्विगुण score = 0.0;             /* Value to return */

  प्रतिज्ञा( आकार(पूर्णांक)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  जर( nVal<1 ) जा wrong_संख्या_args;
  aMatchinfo = (पूर्णांक*)sqlite3_मूल्य_blob(apVal[0]);
  nMatchinfo = sqlite3_मूल्य_bytes(apVal[0]) / आकार(पूर्णांक);
  जर( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
  }
  जर( nMatchinfo!=(2+3*nCol*nPhrase) ){
    sqlite3_result_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    परत;
  }
  जर( nVal!=(1+nCol) ) जा wrong_संख्या_args;

  /* Iterate through each phrase in the users query. */
  साठी(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    पूर्णांक iCol;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    पूर्णांक *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
    साठी(iCol=0; iCol<nCol; iCol++){
      पूर्णांक n_Hit_मोजणी = aPhraseinfo[3*iCol];
      पूर्णांक n_Global_Hit_मोजणी = aPhraseinfo[3*iCol+1];
      द्विगुण weight = sqlite3_मूल्य_double(apVal[iCol+1]);
      जर( n_Hit_मोजणी>0 ){
        score += ((द्विगुण)n_Hit_मोजणी / (द्विगुण)n_Global_Hit_मोजणी) * weight;
      }
    }
  }

  sqlite3_result_double(pCtx, score);
  परत;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_संख्या_args:
  sqlite3_result_error(pCtx, "wrong number of arguments to function rank()", -1);
}

स्थिर पूर्णांक SQLITE_TCLAPI install_fts3_rank_function(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिरांक अक्षर*, sqlite3**);
  sqlite3 *db;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    परत TCL_ERROR;
  }

  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  परत TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_func_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  पूर्णांक i;
  बाह्य पूर्णांक Md5_Register(sqlite3 *, अक्षर **, स्थिरांक sqlite3_api_routines *);

  साठी(i=0; i<आकार(aObjCmd)/आकार(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((शून्य(*)(शून्य))registerTestFunctions);
  sqlite3_auto_extension((शून्य(*)(शून्य))Md5_Register);
  परत TCL_OK;
}
