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
#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>

#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
સ્થિર શૂન્ય *testContextMalloc(sqlite3_context *context, પૂર્ણાંક nByte){
  અક્ષર *z = sqlite3_malloc(nByte);
  જો( !z && nByte>0 ){
    sqlite3_result_error_nomem(context);
  }
  પાછા z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
સ્થિર શૂન્ય randStr(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત **argv){
  સ્થિર અચલ unsigned અક્ષર zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  પૂર્ણાંક iMin, iMax, n, r, i;
  unsigned અક્ષર zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  પ્રતિજ્ઞા(argc==2);

  iMin = sqlite3_કિંમત_int(argv[0]);
  જો( iMin<0 ) iMin = 0;
  જો( iMin>=કદ(zBuf) ) iMin = કદ(zBuf)-1;
  iMax = sqlite3_કિંમત_int(argv[1]);
  જો( iMax<iMin ) iMax = iMin;
  જો( iMax>=કદ(zBuf) ) iMax = કદ(zBuf)-1;
  n = iMin;
  જો( iMax>iMin ){
    sqlite3_randomness(કદ(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  પ્રતિજ્ઞા( n<કદ(zBuf) );
  sqlite3_randomness(n, zBuf);
  માટે(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(કદ(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_result_text(context, (અક્ષર*)zBuf, n, SQLITE_TRANSIENT);
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
સ્થિર પૂર્ણાંક test_destructor_ગણતરી_var = 0;
સ્થિર શૂન્ય destructor(શૂન્ય *p){
  અક્ષર *zVal = (અક્ષર *)p;
  પ્રતિજ્ઞા(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_ગણતરી_var--;
}
સ્થિર શૂન્ય test_destructor(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  અક્ષર *zVal;
  પૂર્ણાંક len;
  
  test_destructor_ગણતરી_var++;
  પ્રતિજ્ઞા( nArg==1 );
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ) પાછા;
  len = sqlite3_કિંમત_bytes(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  જો( !zVal ){
    પાછા;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_કિંમત_text(argv[0]), len);
  sqlite3_result_text(pCtx, zVal, -1, destructor);
}
#ifndef SQLITE_OMIT_UTF16
સ્થિર શૂન્ય test_destructor16(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  અક્ષર *zVal;
  પૂર્ણાંક len;
  
  test_destructor_ગણતરી_var++;
  પ્રતિજ્ઞા( nArg==1 );
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ) પાછા;
  len = sqlite3_કિંમત_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  જો( !zVal ){
    પાછા;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  memcpy(zVal, sqlite3_કિંમત_text16(argv[0]), len);
  sqlite3_result_text16(pCtx, zVal, -1, destructor);
}
#endif
સ્થિર શૂન્ય test_destructor_ગણતરી(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  sqlite3_result_int(pCtx, test_destructor_ગણતરી_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#ifndef SQLITE_UNTESTABLE
શૂન્ય sqlite3BeginBenignMalloc(શૂન્ય);
શૂન્ય sqlite3EndBenignMalloc(શૂન્ય);
#નહીંતર
  #define sqlite3BeginBenignMalloc()
  #define sqlite3EndBenignMalloc()
#endif
સ્થિર શૂન્ય test_agg_errmsg16_step(sqlite3_context *a, પૂર્ણાંક b,sqlite3_કિંમત **c){
}
સ્થિર શૂન્ય test_agg_errmsg16_final(sqlite3_context *ctx){
#ifndef SQLITE_OMIT_UTF16
  અચલ શૂન્ય *z;
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
સ્થિર શૂન્ય free_test_auxdata(શૂન્ય *p) {sqlite3_free(p);}
સ્થિર શૂન્ય test_auxdata(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક i;
  અક્ષર *zRet = testContextMalloc(pCtx, nArg*2);
  જો( !zRet ) પાછા;
  memset(zRet, 0, nArg*2);
  માટે(i=0; i<nArg; i++){
    અક્ષર અચલ *z = (અક્ષર*)sqlite3_કિંમત_text(argv[i]);
    જો( z ){
      પૂર્ણાંક n;
      અક્ષર *zAux = sqlite3_get_auxdata(pCtx, i);
      જો( zAux ){
        zRet[i*2] = '1';
        પ્રતિજ્ઞા( strcmp(zAux,z)==0 );
      }નહીંતર {
        zRet[i*2] = '0';
      }
      n = (પૂર્ણાંક)strlen(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      જો( zAux ){
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
સ્થિર શૂન્ય test_error(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  sqlite3_result_error(pCtx, (અક્ષર*)sqlite3_કિંમત_text(argv[0]), -1);
  જો( nArg==2 ){
    sqlite3_result_error_code(pCtx, sqlite3_કિંમત_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
સ્થિર શૂન્ય counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  પૂર્ણાંક nArg,                /* Number of function arguments */
  sqlite3_કિંમત **argv     /* Values for all function arguments */
){
  પૂર્ણાંક *pCounter = (પૂર્ણાંક*)sqlite3_get_auxdata(pCtx, 0);
  જો( pCounter==0 ){
    pCounter = sqlite3_malloc( કદ(*pCounter) );
    જો( pCounter==0 ){
      sqlite3_result_error_nomem(pCtx);
      પાછા;
    }
    *pCounter = sqlite3_કિંમત_int(argv[0]);
    sqlite3_set_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }નહીંતર{
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
સ્થિર શૂન્ય test_isolation(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
#ifndef SQLITE_OMIT_UTF16
  sqlite3_કિંમત_text16(argv[0]);
  sqlite3_કિંમત_text(argv[0]);
  sqlite3_કિંમત_text16(argv[0]);
  sqlite3_કિંમત_text(argv[0]);
#endif
  sqlite3_result_કિંમત(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
સ્થિર શૂન્ય test_eval(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  sqlite3_stmt *pStmt;
  પૂર્ણાંક rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  અચલ અક્ષર *zSql;
  સ્થિર અચલ અક્ષર zTag[] = "test_eval()";

  જો( sqlite3_get_clientdata(db, zTag) ){
    sqlite3_result_error(pCtx, "nested test_eval()", -1);
    પાછા;
  }
  zSql = (અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  sqlite3_set_clientdata(db, zTag, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  જો( rc==SQLITE_OK ){
    rc = sqlite3_step(pStmt);
    જો( rc==SQLITE_ROW ){
      sqlite3_result_કિંમત(pCtx, sqlite3_column_કિંમત(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_set_clientdata(db, zTag, 0, 0);
  જો( rc ){
    અક્ષર *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_result_text(pCtx, zErr, -1, sqlite3_free);
    sqlite3_result_error_code(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
સ્થિર પૂર્ણાંક testHexChar(અક્ષર c){
  જો( c>='0' && c<='9' ){
    પાછા c - '0';
  }નહીંતર જો( c>='a' && c<='f' ){
    પાછા c - 'a' + 10;
  }નહીંતર જો( c>='A' && c<='F' ){
    પાછા c - 'A' + 10;
  }
  પાછા 0;
}

/*
** Convert hex to binary.
*/
સ્થિર શૂન્ય testHexToBin(અચલ અક્ષર *zIn, અક્ષર *zOut){
  જ્યારે( zIn[0] && zIn[1] ){
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
સ્થિર શૂન્ય testHexToUtf16be(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક n;
  અચલ અક્ષર *zIn;
  અક્ષર *zOut;
  પ્રતિજ્ઞા( nArg==1 );
  n = sqlite3_કિંમત_bytes(argv[0]);
  zIn = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  જો( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }નહીંતર{
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
સ્થિર શૂન્ય testHexToUtf8(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક n;
  અચલ અક્ષર *zIn;
  અક્ષર *zOut;
  પ્રતિજ્ઞા( nArg==1 );
  n = sqlite3_કિંમત_bytes(argv[0]);
  zIn = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  જો( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }નહીંતર{
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
સ્થિર શૂન્ય testHexToUtf16le(
  sqlite3_context *pCtx, 
  પૂર્ણાંક nArg,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક n;
  અચલ અક્ષર *zIn;
  અક્ષર *zOut;
  પ્રતિજ્ઞા( nArg==1 );
  n = sqlite3_કિંમત_bytes(argv[0]);
  zIn = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  zOut = sqlite3_malloc( n/2 );
  જો( zOut==0 ){
    sqlite3_result_error_nomem(pCtx);
  }નહીંતર{
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
સ્થિર શૂન્ય real2hex(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  સંઘ {
    sqlite3_uint64 i;
    દ્વિગુણ r;
    unsigned અક્ષર x[8];
  } v;
  અક્ષર zOut[20];
  પૂર્ણાંક i;
  પૂર્ણાંક bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_કિંમત_double(argv[0]);
  માટે(i=0; i<8; i++){
    જો( bigEndian ){
      zOut[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      zOut[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }નહીંતર{
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
સ્થિર શૂન્ય test_extract(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  પૂર્ણાંક iIdx;                       /* Required field */
  પૂર્ણાંક iCurrent = 0;               /* Current field */

  પ્રતિજ્ઞા( argc==2 );
  pRec = (u8*)sqlite3_કિંમત_blob(argv[0]);
  iIdx = sqlite3_કિંમત_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];

  માટે(iCurrent=0; pHdr<pEndHdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, કદ(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    જો( iCurrent==iIdx ){
      sqlite3_result_કિંમત(context, &mem);
    }

    જો( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
સ્થિર શૂન્ય test_decode(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *pEndHdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_ગણતરી(pRet);

  પ્રતિજ્ઞા( argc==1 );
  pRec = (u8*)sqlite3_કિંમત_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = pEndHdr = &pRec[nHdr];
  જ્યારે( pHdr<pEndHdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, કદ(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    પસંદગી( sqlite3_કિંમત_type(&mem) ){
      સ્થિતિ SQLITE_TEXT:
        pVal = Tcl_NewStringObj((અચલ અક્ષર*)sqlite3_કિંમત_text(&mem), -1);
        તોડો;

      સ્થિતિ SQLITE_BLOB: {
        અક્ષર hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        પૂર્ણાંક n = sqlite3_કિંમત_bytes(&mem);
        u8 *z = (u8*)sqlite3_કિંમત_blob(&mem);
        પૂર્ણાંક i;
        pVal = Tcl_NewStringObj("x'", -1);
        માટે(i=0; i<n; i++){
          અક્ષર hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_AppendStringsToObj(pVal, hex, 0);
        }
        Tcl_AppendStringsToObj(pVal, "'", 0);
        તોડો;
      }

      સ્થિતિ SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_કિંમત_double(&mem));
        તોડો;

      સ્થિતિ SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_કિંમત_int64(&mem));
        તોડો;

      સ્થિતિ SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        તોડો;

      મૂળભૂત:
        પ્રતિજ્ઞા( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    જો( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_result_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_ગણતરી(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
સ્થિર શૂન્ય test_zeroblob(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક nZero = sqlite3_કિંમત_int(argv[0]);
  sqlite3_result_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
સ્થિર શૂન્ય test_getsubtype(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3_result_int(context, (પૂર્ણાંક)sqlite3_કિંમત_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
સ્થિર શૂન્ય test_frombind(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3_uint64 m = 0;
  પૂર્ણાંક i;
  માટે(i=0; i<argc && i<63; i++){
    જો( sqlite3_કિંમત_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_result_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
સ્થિર શૂન્ય test_setsubtype(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3_result_કિંમત(context, argv[0]);
  sqlite3_result_subtype(context, (unsigned પૂર્ણાંક)sqlite3_કિંમત_int(argv[1]));
}

સ્થિર પૂર્ણાંક registerTestFunctions(
  sqlite3 *db,
  અક્ષર **pz_Err_સંદેશ,
  અચલ sqlite3_api_routines *pThunk
){
  સ્થિર અચલ સંરચના {
     અક્ષર *zName;
     signed અક્ષર nArg;
     unsigned પૂર્ણાંક eTextRep; /* 1: UTF-16.  0: UTF-8 */
     શૂન્ય (*xFunc)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#ifndef SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, testHexToUtf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, testHexToUtf16le},
#endif
    { "hex_to_utf8",           1, SQLITE_UTF8, testHexToUtf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_ગણતરી},
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
  પૂર્ણાંક i;

  માટે(i=0; i<કદ(aFuncs)/કદ(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].eTextRep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_step, test_agg_errmsg16_final);
      
  પાછા SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI autoinstall_test_funcs(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  બાહ્ય પૂર્ણાંક Md5_Register(sqlite3 *, અક્ષર **, અચલ sqlite3_api_routines *);
  પૂર્ણાંક rc = sqlite3_auto_extension((શૂન્ય(*)(શૂન્ય))registerTestFunctions);
  જો( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((શૂન્ય(*)(શૂન્ય))Md5_Register);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
સ્થિર શૂન્ય tStep(sqlite3_context *a, પૂર્ણાંક b, sqlite3_કિંમત **c){}
સ્થિર શૂન્ય tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI abuse_create_function(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp*, અચલ અક્ષર*, sqlite3**);
  sqlite3 *db;
  પૂર્ણાંક rc;
  પૂર્ણાંક mxArg;

  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep,tStep,tFinal);
  જો( rc!=SQLITE_MISUSE ) જાઓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, tStep, 0);
  જો( rc!=SQLITE_MISUSE ) જાઓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, tStep, 0, tFinal);
  જો( rc!=SQLITE_MISUSE) જાઓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  જો( rc!=SQLITE_MISUSE ) જાઓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, tStep, 0);
  જો( rc!=SQLITE_MISUSE ) જાઓ abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, tStep, 0, 0);
  જો( rc!=SQLITE_MISUSE ) જાઓ abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, tStep, 0, 0);
  જો( rc!=SQLITE_MISUSE ) જાઓ abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, tStep, 0, 0);
  જો( rc!=SQLITE_MISUSE ) જાઓ abuse_err;

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
  જો( rc!=SQLITE_OK ) જાઓ abuse_err;
                                
  પાછા TCL_OK;

abuse_err:
  Tcl_AppendResult(interp, "sqlite3_create_function abused test failed", 
                   (અક્ષર*)0);
  પાછા TCL_ERROR;
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
સ્થિર શૂન્ય rankfunc(sqlite3_context *pCtx, પૂર્ણાંક nVal, sqlite3_કિંમત **apVal){
  પૂર્ણાંક *aMatchinfo;                /* Return value of matchinfo() */
  પૂર્ણાંક nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  પૂર્ણાંક nCol = 0;                   /* Number of columns in the table */
  પૂર્ણાંક nPhrase = 0;                /* Number of phrases in the query */
  પૂર્ણાંક iPhrase;                    /* Current phrase */
  દ્વિગુણ score = 0.0;             /* Value to return */

  પ્રતિજ્ઞા( કદ(પૂર્ણાંક)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  જો( nVal<1 ) જાઓ wrong_સંખ્યા_args;
  aMatchinfo = (પૂર્ણાંક*)sqlite3_કિંમત_blob(apVal[0]);
  nMatchinfo = sqlite3_કિંમત_bytes(apVal[0]) / કદ(પૂર્ણાંક);
  જો( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    nCol = aMatchinfo[1];
  }
  જો( nMatchinfo!=(2+3*nCol*nPhrase) ){
    sqlite3_result_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    પાછા;
  }
  જો( nVal!=(1+nCol) ) જાઓ wrong_સંખ્યા_args;

  /* Iterate through each phrase in the users query. */
  માટે(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    પૂર્ણાંક iCol;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    પૂર્ણાંક *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
    માટે(iCol=0; iCol<nCol; iCol++){
      પૂર્ણાંક n_Hit_ગણતરી = aPhraseinfo[3*iCol];
      પૂર્ણાંક n_Global_Hit_ગણતરી = aPhraseinfo[3*iCol+1];
      દ્વિગુણ weight = sqlite3_કિંમત_double(apVal[iCol+1]);
      જો( n_Hit_ગણતરી>0 ){
        score += ((દ્વિગુણ)n_Hit_ગણતરી / (દ્વિગુણ)n_Global_Hit_ગણતરી) * weight;
      }
    }
  }

  sqlite3_result_double(pCtx, score);
  પાછા;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_સંખ્યા_args:
  sqlite3_result_error(pCtx, "wrong number of arguments to function rank()", -1);
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI install_fts3_rank_function(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp*, અચલ અક્ષર*, sqlite3**);
  sqlite3 *db;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }

  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  પાછા TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest_func_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  પૂર્ણાંક i;
  બાહ્ય પૂર્ણાંક Md5_Register(sqlite3 *, અક્ષર **, અચલ sqlite3_api_routines *);

  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((શૂન્ય(*)(શૂન્ય))registerTestFunctions);
  sqlite3_auto_extension((શૂન્ય(*)(શૂન્ય))Md5_Register);
  પાછા TCL_OK;
}
