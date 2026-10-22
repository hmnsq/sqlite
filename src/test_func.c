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
#समावेश "sqlite3.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>

#समावेश "sqliteInt.h"
#समावेश "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
अटल व्योम *testContextMalloc(sqlite3_context *context, पूर्णांक nByte){
  अक्षर *z = sqlite3_malloc(nByte);
  अगर( !z && nByte>0 ){
    sqlite3_परिणाम_error_nomem(context);
  }
  वापस z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
अटल व्योम randStr(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  अटल स्थिर अचिन्हित अक्षर zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  पूर्णांक iMin, iMax, n, r, i;
  अचिन्हित अक्षर zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  निश्चित(argc==2);

  iMin = sqlite3_मूल्य_int(argv[0]);
  अगर( iMin<0 ) iMin = 0;
  अगर( iMin>=माप(zBuf) ) iMin = माप(zBuf)-1;
  iMax = sqlite3_मूल्य_int(argv[1]);
  अगर( iMax<iMin ) iMax = iMin;
  अगर( iMax>=माप(zBuf) ) iMax = माप(zBuf)-1;
  n = iMin;
  अगर( iMax>iMin ){
    sqlite3_randomness(माप(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  निश्चित( n<माप(zBuf) );
  sqlite3_randomness(n, zBuf);
  क्रम(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(माप(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_परिणाम_text(context, (अक्षर*)zBuf, n, SQLITE_TRANSIENT);
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
अटल पूर्णांक test_destructor_गिनती_var = 0;
अटल व्योम destructor(व्योम *p){
  अक्षर *zVal = (अक्षर *)p;
  निश्चित(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_गिनती_var--;
}
अटल व्योम test_destructor(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  अक्षर *zVal;
  पूर्णांक len;
  
  test_destructor_गिनती_var++;
  निश्चित( nArg==1 );
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ) वापस;
  len = sqlite3_मूल्य_bytes(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  अगर( !zVal ){
    वापस;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  स_नकल(zVal, sqlite3_मूल्य_text(argv[0]), len);
  sqlite3_परिणाम_text(pCtx, zVal, -1, destructor);
}
#अगर_अघोषित SQLITE_OMIT_UTF16
अटल व्योम test_destructor16(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  अक्षर *zVal;
  पूर्णांक len;
  
  test_destructor_गिनती_var++;
  निश्चित( nArg==1 );
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ) वापस;
  len = sqlite3_मूल्य_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  अगर( !zVal ){
    वापस;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  स_नकल(zVal, sqlite3_मूल्य_text16(argv[0]), len);
  sqlite3_परिणाम_text16(pCtx, zVal, -1, destructor);
}
#पूर्ण_अगर
अटल व्योम test_destructor_गिनती(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  sqlite3_परिणाम_int(pCtx, test_destructor_गिनती_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#अगर_अघोषित SQLITE_UNTESTABLE
व्योम sqlite3BeginBenignMalloc(व्योम);
व्योम sqlite3EndBenignMalloc(व्योम);
#अन्यथा
  #घोषणा sqlite3BeginBenignMalloc()
  #घोषणा sqlite3EndBenignMalloc()
#पूर्ण_अगर
अटल व्योम test_agg_errmsg16_step(sqlite3_context *a, पूर्णांक b,sqlite3_मूल्य **c){
}
अटल व्योम test_agg_errmsg16_final(sqlite3_context *ctx){
#अगर_अघोषित SQLITE_OMIT_UTF16
  स्थिर व्योम *z;
  sqlite3 * db = sqlite3_context_db_handle(ctx);
  sqlite3_aggregate_context(ctx, 2048);
  z = sqlite3_errmsg16(db);
  sqlite3_परिणाम_text16(ctx, z, -1, SQLITE_TRANSIENT);
#पूर्ण_अगर
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
अटल व्योम free_test_auxdata(व्योम *p) {sqlite3_free(p);}
अटल व्योम test_auxdata(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;
  अक्षर *zRet = testContextMalloc(pCtx, nArg*2);
  अगर( !zRet ) वापस;
  स_रखो(zRet, 0, nArg*2);
  क्रम(i=0; i<nArg; i++){
    अक्षर स्थिर *z = (अक्षर*)sqlite3_मूल्य_text(argv[i]);
    अगर( z ){
      पूर्णांक n;
      अक्षर *zAux = sqlite3_get_auxdata(pCtx, i);
      अगर( zAux ){
        zRet[i*2] = '1';
        निश्चित( म_भेद(zAux,z)==0 );
      }अन्यथा {
        zRet[i*2] = '0';
      }
      n = (पूर्णांक)म_माप(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      अगर( zAux ){
        स_नकल(zAux, z, n);
        sqlite3_set_auxdata(pCtx, i, zAux, free_test_auxdata);
      }
      zRet[i*2+1] = ' ';
    }
  }
  sqlite3_परिणाम_text(pCtx, zRet, 2*nArg-1, free_test_auxdata);
}

/*
** A function to test error reporting from user functions. This function
** returns a copy of its first argument as the error message.  If the
** second argument exists, it becomes the error code.
*/
अटल व्योम test_error(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  sqlite3_परिणाम_error(pCtx, (अक्षर*)sqlite3_मूल्य_text(argv[0]), -1);
  अगर( nArg==2 ){
    sqlite3_परिणाम_error_code(pCtx, sqlite3_मूल्य_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
अटल व्योम गणक_Func(
  sqlite3_context *pCtx,   /* Function context */
  पूर्णांक nArg,                /* Number of function arguments */
  sqlite3_मूल्य **argv     /* Values for all function arguments */
){
  पूर्णांक *p_गणक = (पूर्णांक*)sqlite3_get_auxdata(pCtx, 0);
  अगर( p_गणक==0 ){
    p_गणक = sqlite3_malloc( माप(*p_गणक) );
    अगर( p_गणक==0 ){
      sqlite3_परिणाम_error_nomem(pCtx);
      वापस;
    }
    *p_गणक = sqlite3_मूल्य_int(argv[0]);
    sqlite3_set_auxdata(pCtx, 0, p_गणक, sqlite3_free);
  }अन्यथा{
    ++*p_गणक;
  }
  sqlite3_परिणाम_int(pCtx, *p_गणक);
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
अटल व्योम test_isolation(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
#अगर_अघोषित SQLITE_OMIT_UTF16
  sqlite3_मूल्य_text16(argv[0]);
  sqlite3_मूल्य_text(argv[0]);
  sqlite3_मूल्य_text16(argv[0]);
  sqlite3_मूल्य_text(argv[0]);
#पूर्ण_अगर
  sqlite3_परिणाम_मूल्य(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
अटल व्योम test_eval(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  sqlite3_stmt *pStmt;
  पूर्णांक rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  स्थिर अक्षर *zSql;
  अटल स्थिर अक्षर zTag[] = "test_eval()";

  अगर( sqlite3_get_clientdata(db, zTag) ){
    sqlite3_परिणाम_error(pCtx, "nested test_eval()", -1);
    वापस;
  }
  zSql = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
  sqlite3_set_clientdata(db, zTag, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  अगर( rc==SQLITE_OK ){
    rc = sqlite3_step(pStmt);
    अगर( rc==SQLITE_ROW ){
      sqlite3_परिणाम_मूल्य(pCtx, sqlite3_column_मूल्य(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_set_clientdata(db, zTag, 0, 0);
  अगर( rc ){
    अक्षर *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_परिणाम_text(pCtx, zErr, -1, sqlite3_free);
    sqlite3_परिणाम_error_code(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
अटल पूर्णांक testHexChar(अक्षर c){
  अगर( c>='0' && c<='9' ){
    वापस c - '0';
  }अन्यथा अगर( c>='a' && c<='f' ){
    वापस c - 'a' + 10;
  }अन्यथा अगर( c>='A' && c<='F' ){
    वापस c - 'A' + 10;
  }
  वापस 0;
}

/*
** Convert hex to binary.
*/
अटल व्योम testHexToBin(स्थिर अक्षर *zIn, अक्षर *zOut){
  जबतक( zIn[0] && zIn[1] ){
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
#अगर_अघोषित SQLITE_OMIT_UTF16
अटल व्योम testHexToUtf16be(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  पूर्णांक n;
  स्थिर अक्षर *zIn;
  अक्षर *zOut;
  निश्चित( nArg==1 );
  n = sqlite3_मूल्य_bytes(argv[0]);
  zIn = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  अगर( zOut==0 ){
    sqlite3_परिणाम_error_nomem(pCtx);
  }अन्यथा{
    testHexToBin(zIn, zOut);
    sqlite3_परिणाम_text16be(pCtx, zOut, n/2, sqlite3_free);
  }
}
#पूर्ण_अगर

/*
**      hex_to_utf8(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
अटल व्योम testHexToUtf8(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  पूर्णांक n;
  स्थिर अक्षर *zIn;
  अक्षर *zOut;
  निश्चित( nArg==1 );
  n = sqlite3_मूल्य_bytes(argv[0]);
  zIn = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  अगर( zOut==0 ){
    sqlite3_परिणाम_error_nomem(pCtx);
  }अन्यथा{
    testHexToBin(zIn, zOut);
    sqlite3_परिणाम_text(pCtx, zOut, n/2, sqlite3_free);
  }
}

/*
**      hex_to_utf16le(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#अगर_अघोषित SQLITE_OMIT_UTF16
अटल व्योम testHexToUtf16le(
  sqlite3_context *pCtx, 
  पूर्णांक nArg,
  sqlite3_मूल्य **argv
){
  पूर्णांक n;
  स्थिर अक्षर *zIn;
  अक्षर *zOut;
  निश्चित( nArg==1 );
  n = sqlite3_मूल्य_bytes(argv[0]);
  zIn = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  अगर( zOut==0 ){
    sqlite3_परिणाम_error_nomem(pCtx);
  }अन्यथा{
    testHexToBin(zIn, zOut);
    sqlite3_परिणाम_text16le(pCtx, zOut, n/2, sqlite3_free);
  }
}
#पूर्ण_अगर

/*
** SQL function:   real2hex(X)
**
** If argument X is a real number, then convert it into a string which is
** the big-endian hexadecimal representation of the ieee754 encoding of
** that number.  If X is not a real number, return NULL.
*/
अटल व्योम real2hex(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  जोड़ {
    sqlite3_uint64 i;
    द्विगुन r;
    अचिन्हित अक्षर x[8];
  } v;
  अक्षर zOut[20];
  पूर्णांक i;
  पूर्णांक bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_मूल्य_double(argv[0]);
  क्रम(i=0; i<8; i++){
    अगर( bigEndian ){
      zOut[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }अन्यथा{
      zOut[14-i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[14-i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }
  }
  zOut[16] = 0;
  sqlite3_परिणाम_text(context, zOut, -1, SQLITE_TRANSIENT);
}

/*
**     test_extract(record, field)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as the first argument. The
** second argument is the index of the field within that record to
** extract and return.
*/
अटल व्योम test_extract(
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

  निश्चित( argc==2 );
  pRec = (u8*)sqlite3_मूल्य_blob(argv[0]);
  iIdx = sqlite3_मूल्य_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];

  क्रम(iCurrent=0; pHdr<pEndHdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    स_रखो(&mem, 0, माप(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    अगर( iCurrent==iIdx ){
      sqlite3_परिणाम_मूल्य(context, &mem);
    }

    अगर( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
अटल व्योम test_decode(
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
  Tcl_Incr_Ref_गिनती(pRet);

  निश्चित( argc==1 );
  pRec = (u8*)sqlite3_मूल्य_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];
  जबतक( pHdr<pEndHdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    स_रखो(&mem, 0, माप(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    चयन( sqlite3_मूल्य_type(&mem) ){
      हाल SQLITE_TEXT:
        pVal = Tcl_NewStringObj((स्थिर अक्षर*)sqlite3_मूल्य_text(&mem), -1);
        अवरोध;

      हाल SQLITE_BLOB: {
        अक्षर hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        पूर्णांक n = sqlite3_मूल्य_bytes(&mem);
        u8 *z = (u8*)sqlite3_मूल्य_blob(&mem);
        पूर्णांक i;
        pVal = Tcl_NewStringObj("x'", -1);
        क्रम(i=0; i<n; i++){
          अक्षर hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_AppendStringsToObj(pVal, hex, 0);
        }
        Tcl_AppendStringsToObj(pVal, "'", 0);
        अवरोध;
      }

      हाल SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_मूल्य_double(&mem));
        अवरोध;

      हाल SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_मूल्य_int64(&mem));
        अवरोध;

      हाल SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        अवरोध;

      शेष:
        निश्चित( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    अगर( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_परिणाम_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_गिनती(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
अटल व्योम test_zeroblob(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक nZero = sqlite3_मूल्य_int(argv[0]);
  sqlite3_परिणाम_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
अटल व्योम test_getsubtype(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_परिणाम_int(context, (पूर्णांक)sqlite3_मूल्य_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
अटल व्योम test_frombind(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_uint64 m = 0;
  पूर्णांक i;
  क्रम(i=0; i<argc && i<63; i++){
    अगर( sqlite3_मूल्य_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_परिणाम_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
अटल व्योम test_setsubtype(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_परिणाम_मूल्य(context, argv[0]);
  sqlite3_परिणाम_subtype(context, (अचिन्हित पूर्णांक)sqlite3_मूल्य_int(argv[1]));
}

अटल पूर्णांक registerTestFunctions(
  sqlite3 *db,
  अक्षर **pz_Err_संदेश,
  स्थिर sqlite3_api_routines *pThunk
){
  अटल स्थिर काष्ठा {
     अक्षर *zName;
     चिन्हित अक्षर nArg;
     अचिन्हित पूर्णांक eTextRep; /* 1: UTF-16.  0: UTF-8 */
     व्योम (*xFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#अगर_अघोषित SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, testHexToUtf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, testHexToUtf16le},
#पूर्ण_अगर
    { "hex_to_utf8",           1, SQLITE_UTF8, testHexToUtf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_गिनती},
    { "test_auxdata",         -1, SQLITE_UTF8, test_auxdata},
    { "test_error",            1, SQLITE_UTF8, test_error},
    { "test_error",            2, SQLITE_UTF8, test_error},
    { "test_eval",             1, SQLITE_UTF8, test_eval},
    { "test_isolation",        2, SQLITE_UTF8, test_isolation},
    { "test_counter",          1, SQLITE_UTF8, गणक_Func},
    { "real2hex",              1, SQLITE_UTF8, real2hex},
    { "test_decode",           1, SQLITE_UTF8, test_decode},
    { "test_extract",          2, SQLITE_UTF8, test_extract},
    { "test_zeroblob",  1, SQLITE_UTF8|SQLITE_DETERMINISTIC, test_zeroblob},
    { "test_getsubtype",       1, SQLITE_UTF8, test_getsubtype},
    { "test_setsubtype",       2, SQLITE_UTF8|SQLITE_परिणाम_SUBTYPE,
                                               test_setsubtype},
    { "test_frombind",        -1, SQLITE_UTF8, test_frombind},
  };
  पूर्णांक i;

  क्रम(i=0; i<माप(aFuncs)/माप(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].eTextRep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_step, test_agg_errmsg16_final);
      
  वापस SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
अटल पूर्णांक SQLITE_TCLAPI autoinstall_test_funcs(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णांक Md5_Register(sqlite3 *, अक्षर **, स्थिर sqlite3_api_routines *);
  पूर्णांक rc = sqlite3_auto_extension((व्योम(*)(व्योम))registerTestFunctions);
  अगर( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((व्योम(*)(व्योम))Md5_Register);
  }
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
अटल व्योम tStep(sqlite3_context *a, पूर्णांक b, sqlite3_मूल्य **c){}
अटल व्योम tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
अटल पूर्णांक SQLITE_TCLAPI abuse_create_function(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिर अक्षर*, sqlite3**);
  sqlite3 *db;
  पूर्णांक rc;
  पूर्णांक mxArg;

  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep,tStep,tFinal);
  अगर( rc!=SQLITE_MISUSE ) जाओ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, tStep, 0);
  अगर( rc!=SQLITE_MISUSE ) जाओ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, 0, tFinal);
  अगर( rc!=SQLITE_MISUSE) जाओ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  अगर( rc!=SQLITE_MISUSE ) जाओ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, tStep, 0);
  अगर( rc!=SQLITE_MISUSE ) जाओ abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, tStep, 0, 0);
  अगर( rc!=SQLITE_MISUSE ) जाओ abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, tStep, 0, 0);
  अगर( rc!=SQLITE_MISUSE ) जाओ abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, tStep, 0, 0);
  अगर( rc!=SQLITE_MISUSE ) जाओ abuse_err;

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
  अगर( rc!=SQLITE_OK ) जाओ abuse_err;
                                
  वापस TCL_OK;

abuse_err:
  Tcl_Append_परिणाम(interp, "sqlite3_create_function abused test failed", 
                   (अक्षर*)0);
  वापस TCL_ERROR;
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
अटल व्योम rankfunc(sqlite3_context *pCtx, पूर्णांक nVal, sqlite3_मूल्य **apVal){
  पूर्णांक *aMatchinfo;                /* Return value of matchinfo() */
  पूर्णांक nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  पूर्णांक nCol = 0;                   /* Number of columns in the table */
  पूर्णांक nPhrase = 0;                /* Number of phrases in the query */
  पूर्णांक iPhrase;                    /* Current phrase */
  द्विगुन score = 0.0;             /* Value to return */

  निश्चित( माप(पूर्णांक)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  अगर( nVal<1 ) जाओ wrong_संख्या_args;
  aMatchinfo = (पूर्णांक*)sqlite3_मूल्य_blob(apVal[0]);
  nMatchinfo = sqlite3_मूल्य_bytes(apVal[0]) / माप(पूर्णांक);
  अगर( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
  }
  अगर( nMatchinfo!=(2+3*nCol*nPhrase) ){
    sqlite3_परिणाम_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    वापस;
  }
  अगर( nVal!=(1+nCol) ) जाओ wrong_संख्या_args;

  /* Iterate through each phrase in the users query. */
  क्रम(iPhrase=0; iPhrase<nPhrase; iPhrase++){
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
    क्रम(iCol=0; iCol<nCol; iCol++){
      पूर्णांक n_Hit_गिनती = aPhraseinfo[3*iCol];
      पूर्णांक n_Global_Hit_गिनती = aPhraseinfo[3*iCol+1];
      द्विगुन weight = sqlite3_मूल्य_double(apVal[iCol+1]);
      अगर( n_Hit_गिनती>0 ){
        score += ((द्विगुन)n_Hit_गिनती / (द्विगुन)n_Global_Hit_गिनती) * weight;
      }
    }
  }

  sqlite3_परिणाम_double(pCtx, score);
  वापस;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_संख्या_args:
  sqlite3_परिणाम_error(pCtx, "wrong number of arguments to function rank()", -1);
}

अटल पूर्णांक SQLITE_TCLAPI install_fts3_rank_function(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिर अक्षर*, sqlite3**);
  sqlite3 *db;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }

  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  वापस TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_func_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  पूर्णांक i;
  बाह्य पूर्णांक Md5_Register(sqlite3 *, अक्षर **, स्थिर sqlite3_api_routines *);

  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((व्योम(*)(व्योम))registerTestFunctions);
  sqlite3_auto_extension((व्योम(*)(व्योम))Md5_Register);
  वापस TCL_OK;
}
