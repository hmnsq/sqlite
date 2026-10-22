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
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>

#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

/*
** Allocate nByte bytes of space using sqlite3_malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed.
*/
స్థిర శూన్య *testContextMalloc(sqlite3_context *context, పూర్ణాంకం nByte){
  అక్షరం *z = sqlite3_malloc(nByte);
  ఒకవేళ( !z && nByte>0 ){
    sqlite3_ఫలితం_error_nomem(context);
  }
  తిరిగి z;
}

/*
** This function generates a string of random characters.  Used for
** generating test data.
*/
స్థిర శూన్య randStr(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  స్థిర స్థిరరాశి గుర్తులేని అక్షరం zSrc[] = 
     "abcdefghijklmnopqrstuvwxyz"
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     "0123456789"
     ".-!,:*^+=_|?/<> ";
  పూర్ణాంకం iMin, iMax, n, r, i;
  గుర్తులేని అక్షరం zBuf[1000];

  /* It used to be possible to call randstr() with any number of arguments,
  ** but now it is registered with SQLite as requiring exactly 2.
  */
  ప్రతిజ్ఞా(argc==2);

  iMin = sqlite3_విలువ_int(argv[0]);
  ఒకవేళ( iMin<0 ) iMin = 0;
  ఒకవేళ( iMin>=పరిమాణం(zBuf) ) iMin = పరిమాణం(zBuf)-1;
  iMax = sqlite3_విలువ_int(argv[1]);
  ఒకవేళ( iMax<iMin ) iMax = iMin;
  ఒకవేళ( iMax>=పరిమాణం(zBuf) ) iMax = పరిమాణం(zBuf)-1;
  n = iMin;
  ఒకవేళ( iMax>iMin ){
    sqlite3_randomness(పరిమాణం(r), &r);
    r &= 0x7fffffff;
    n += r%(iMax + 1 - iMin);
  }
  ప్రతిజ్ఞా( n<పరిమాణం(zBuf) );
  sqlite3_randomness(n, zBuf);
  కొరకు(i=0; i<n; i++){
    zBuf[i] = zSrc[zBuf[i]%(పరిమాణం(zSrc)-1)];
  }
  zBuf[n] = 0;
  sqlite3_ఫలితం_పాఠ్యం(context, (అక్షరం*)zBuf, n, SQLITE_TRANSIENT);
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
స్థిర పూర్ణాంకం test_destructor_లెక్క_var = 0;
స్థిర శూన్య destructor(శూన్య *p){
  అక్షరం *zVal = (అక్షరం *)p;
  ప్రతిజ్ఞా(zVal);
  zVal--;
  sqlite3_free(zVal);
  test_destructor_లెక్క_var--;
}
స్థిర శూన్య test_destructor(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  అక్షరం *zVal;
  పూర్ణాంకం len;
  
  test_destructor_లెక్క_var++;
  ప్రతిజ్ఞా( nArg==1 );
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ) తిరిగి;
  len = sqlite3_విలువ_బైట్లు(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ఒకవేళ( !zVal ){
    తిరిగి;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  జ్ఞాపకం_నకలు(zVal, sqlite3_విలువ_పాఠ్యం(argv[0]), len);
  sqlite3_ఫలితం_పాఠ్యం(pCtx, zVal, -1, destructor);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిర శూన్య test_destructor16(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  అక్షరం *zVal;
  పూర్ణాంకం len;
  
  test_destructor_లెక్క_var++;
  ప్రతిజ్ఞా( nArg==1 );
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ) తిరిగి;
  len = sqlite3_విలువ_bytes16(argv[0]); 
  zVal = testContextMalloc(pCtx, len+3);
  ఒకవేళ( !zVal ){
    తిరిగి;
  }
  zVal[len+1] = 0;
  zVal[len+2] = 0;
  zVal++;
  జ్ఞాపకం_నకలు(zVal, sqlite3_విలువ_text16(argv[0]), len);
  sqlite3_ఫలితం_text16(pCtx, zVal, -1, destructor);
}
#అయితే_ముగింపు
స్థిర శూన్య test_destructor_లెక్క(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  sqlite3_ఫలితం_int(pCtx, test_destructor_లెక్క_var);
}

/*
** The following aggregate function, test_agg_errmsg16(), takes zero 
** arguments. It returns the text value returned by the sqlite3_errmsg16()
** API function.
*/
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
శూన్య sqlite3BeginBenignMalloc(శూన్య);
శూన్య sqlite3_చివర_Benign_Malloc(శూన్య);
#లేకపోతే
  #నిర్వచించు sqlite3BeginBenignMalloc()
  #నిర్వచించు sqlite3_చివర_Benign_Malloc()
#అయితే_ముగింపు
స్థిర శూన్య test_agg_errmsg16_అడుగు(sqlite3_context *a, పూర్ణాంకం b,sqlite3_విలువ **c){
}
స్థిర శూన్య test_agg_errmsg16_final(sqlite3_context *ctx){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  స్థిరరాశి శూన్య *z;
  sqlite3 * db = sqlite3_context_db_handle(ctx);
  sqlite3_aggregate_context(ctx, 2048);
  z = sqlite3_errmsg16(db);
  sqlite3_ఫలితం_text16(ctx, z, -1, SQLITE_TRANSIENT);
#అయితే_ముగింపు
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
స్థిర శూన్య free_test_auxdata(శూన్య *p) {sqlite3_free(p);}
స్థిర శూన్య test_auxdata(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం i;
  అక్షరం *zRet = testContextMalloc(pCtx, nArg*2);
  ఒకవేళ( !zRet ) తిరిగి;
  memset(zRet, 0, nArg*2);
  కొరకు(i=0; i<nArg; i++){
    అక్షరం స్థిరరాశి *z = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
    ఒకవేళ( z ){
      పూర్ణాంకం n;
      అక్షరం *zAux = sqlite3_get_auxdata(pCtx, i);
      ఒకవేళ( zAux ){
        zRet[i*2] = '1';
        ప్రతిజ్ఞా( strcmp(zAux,z)==0 );
      }లేకపోతే {
        zRet[i*2] = '0';
      }
      n = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z) + 1;
      zAux = testContextMalloc(pCtx, n);
      ఒకవేళ( zAux ){
        జ్ఞాపకం_నకలు(zAux, z, n);
        sqlite3_సమితి_auxdata(pCtx, i, zAux, free_test_auxdata);
      }
      zRet[i*2+1] = ' ';
    }
  }
  sqlite3_ఫలితం_పాఠ్యం(pCtx, zRet, 2*nArg-1, free_test_auxdata);
}

/*
** A function to test error reporting from user functions. This function
** returns a copy of its first argument as the error message.  If the
** second argument exists, it becomes the error code.
*/
స్థిర శూన్య test_error(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  sqlite3_ఫలితం_error(pCtx, (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]), -1);
  ఒకవేళ( nArg==2 ){
    sqlite3_ఫలితం_error_సంకేతం(pCtx, sqlite3_విలువ_int(argv[1]));
  }
}

/*
** Implementation of the counter(X) function.  If X is an integer
** constant, then the first invocation will return X.  The second X+1.
** and so forth.  Can be used (for example) to provide a sequence number
** in a result set.
*/
స్థిర శూన్య counterFunc(
  sqlite3_context *pCtx,   /* Function context */
  పూర్ణాంకం nArg,                /* Number of function arguments */
  sqlite3_విలువ **argv     /* Values for all function arguments */
){
  పూర్ణాంకం *pCounter = (పూర్ణాంకం*)sqlite3_get_auxdata(pCtx, 0);
  ఒకవేళ( pCounter==0 ){
    pCounter = sqlite3_malloc( పరిమాణం(*pCounter) );
    ఒకవేళ( pCounter==0 ){
      sqlite3_ఫలితం_error_nomem(pCtx);
      తిరిగి;
    }
    *pCounter = sqlite3_విలువ_int(argv[0]);
    sqlite3_సమితి_auxdata(pCtx, 0, pCounter, sqlite3_free);
  }లేకపోతే{
    ++*pCounter;
  }
  sqlite3_ఫలితం_int(pCtx, *pCounter);
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
స్థిర శూన్య test_isolation(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  sqlite3_విలువ_text16(argv[0]);
  sqlite3_విలువ_పాఠ్యం(argv[0]);
  sqlite3_విలువ_text16(argv[0]);
  sqlite3_విలువ_పాఠ్యం(argv[0]);
#అయితే_ముగింపు
  sqlite3_ఫలితం_విలువ(pCtx, argv[1]);
}

/*
** Invoke an SQL statement recursively.  The function result is the 
** first column of the first row of the result set.
*/
స్థిర శూన్య test_eval(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  sqlite3_stmt *pStmt;
  పూర్ణాంకం rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);
  స్థిరరాశి అక్షరం *zSql;
  స్థిర స్థిరరాశి అక్షరం z_ట్యాగ్[] = "test_eval()";

  ఒకవేళ( sqlite3_get_clientdata(db, z_ట్యాగ్) ){
    sqlite3_ఫలితం_error(pCtx, "nested test_eval()", -1);
    తిరిగి;
  }
  zSql = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  sqlite3_సమితి_clientdata(db, z_ట్యాగ్, &rc, 0);
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3_అడుగు(pStmt);
    ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
      sqlite3_ఫలితం_విలువ(pCtx, sqlite3_స్తంభం_విలువ(pStmt, 0));
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqlite3_సమితి_clientdata(db, z_ట్యాగ్, 0, 0);
  ఒకవేళ( rc ){
    అక్షరం *zErr;
    zErr = sqlite3_mprintf("sqlite3_prepare_v2() error: %s",sqlite3_errmsg(db));
    sqlite3_ఫలితం_పాఠ్యం(pCtx, zErr, -1, sqlite3_free);
    sqlite3_ఫలితం_error_సంకేతం(pCtx, rc);
  }
}


/*
** convert one character from hex to binary
*/
స్థిర పూర్ణాంకం testHexChar(అక్షరం c){
  ఒకవేళ( c>='0' && c<='9' ){
    తిరిగి c - '0';
  }లేకపోతే ఒకవేళ( c>='a' && c<='f' ){
    తిరిగి c - 'a' + 10;
  }లేకపోతే ఒకవేళ( c>='A' && c<='F' ){
    తిరిగి c - 'A' + 10;
  }
  తిరిగి 0;
}

/*
** Convert hex to binary.
*/
స్థిర శూన్య test_Hex_గమ్యం_Bin(స్థిరరాశి అక్షరం *z_లోపల, అక్షరం *z_బహిర్గతం){
  అయితే( z_లోపల[0] && z_లోపల[1] ){
    *(z_బహిర్గతం++) = (testHexChar(z_లోపల[0])<<4) + testHexChar(z_లోపల[1]);
    z_లోపల += 2;
  }
}

/*
**      hex_to_utf16be(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిర శూన్య test_Hex_గమ్యం_Utf16be(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం n;
  స్థిరరాశి అక్షరం *z_లోపల;
  అక్షరం *z_బహిర్గతం;
  ప్రతిజ్ఞా( nArg==1 );
  n = sqlite3_విలువ_బైట్లు(argv[0]);
  z_లోపల = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  z_బహిర్గతం = sqlite3_malloc( n/2 );
  ఒకవేళ( z_బహిర్గతం==0 ){
    sqlite3_ఫలితం_error_nomem(pCtx);
  }లేకపోతే{
    test_Hex_గమ్యం_Bin(z_లోపల, z_బహిర్గతం);
    sqlite3_ఫలితం_text16be(pCtx, z_బహిర్గతం, n/2, sqlite3_free);
  }
}
#అయితే_ముగింపు

/*
**      hex_to_utf8(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
స్థిర శూన్య test_Hex_గమ్యం_Utf8(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం n;
  స్థిరరాశి అక్షరం *z_లోపల;
  అక్షరం *z_బహిర్గతం;
  ప్రతిజ్ఞా( nArg==1 );
  n = sqlite3_విలువ_బైట్లు(argv[0]);
  z_లోపల = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  z_బహిర్గతం = sqlite3_malloc( n/2 );
  ఒకవేళ( z_బహిర్గతం==0 ){
    sqlite3_ఫలితం_error_nomem(pCtx);
  }లేకపోతే{
    test_Hex_గమ్యం_Bin(z_లోపల, z_బహిర్గతం);
    sqlite3_ఫలితం_పాఠ్యం(pCtx, z_బహిర్గతం, n/2, sqlite3_free);
  }
}

/*
**      hex_to_utf16le(HEX)
**
** Convert the input string from HEX into binary.  Then return the
** result using sqlite3_result_text16le().
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
స్థిర శూన్య test_Hex_గమ్యం_Utf16le(
  sqlite3_context *pCtx, 
  పూర్ణాంకం nArg,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం n;
  స్థిరరాశి అక్షరం *z_లోపల;
  అక్షరం *z_బహిర్గతం;
  ప్రతిజ్ఞా( nArg==1 );
  n = sqlite3_విలువ_బైట్లు(argv[0]);
  z_లోపల = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  z_బహిర్గతం = sqlite3_malloc( n/2 );
  ఒకవేళ( z_బహిర్గతం==0 ){
    sqlite3_ఫలితం_error_nomem(pCtx);
  }లేకపోతే{
    test_Hex_గమ్యం_Bin(z_లోపల, z_బహిర్గతం);
    sqlite3_ఫలితం_text16le(pCtx, z_బహిర్గతం, n/2, sqlite3_free);
  }
}
#అయితే_ముగింపు

/*
** SQL function:   real2hex(X)
**
** If argument X is a real number, then convert it into a string which is
** the big-endian hexadecimal representation of the ieee754 encoding of
** that number.  If X is not a real number, return NULL.
*/
స్థిర శూన్య real2hex(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  సంఘం {
    sqlite3_uint64 i;
    ద్విగుణ r;
    గుర్తులేని అక్షరం x[8];
  } v;
  అక్షరం z_బహిర్గతం[20];
  పూర్ణాంకం i;
  పూర్ణాంకం bigEndian;
  v.i = 1;
  bigEndian = v.x[0]==0;
  v.r = sqlite3_విలువ_double(argv[0]);
  కొరకు(i=0; i<8; i++){
    ఒకవేళ( bigEndian ){
      z_బహిర్గతం[i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_బహిర్గతం[i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }లేకపోతే{
      z_బహిర్గతం[14-i*2]   = "0123456789abcdef"[v.x[i]>>4];
      z_బహిర్గతం[14-i*2+1] = "0123456789abcdef"[v.x[i]&0xf];
    }
  }
  z_బహిర్గతం[16] = 0;
  sqlite3_ఫలితం_పాఠ్యం(context, z_బహిర్గతం, -1, SQLITE_TRANSIENT);
}

/*
**     test_extract(record, field)
**
** This function implements an SQL user-function that accepts a blob
** containing a formatted database record as the first argument. The
** second argument is the index of the field within that record to
** extract and return.
*/
స్థిర శూన్య test_వెలికితీయి(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_చివర_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  పూర్ణాంకం iIdx;                       /* Required field */
  పూర్ణాంకం iCurrent = 0;               /* Current field */

  ప్రతిజ్ఞా( argc==2 );
  pRec = (u8*)sqlite3_విలువ_blob(argv[0]);
  iIdx = sqlite3_విలువ_int(argv[1]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_చివర_Hdr = &pRec[nHdr];

  కొరకు(iCurrent=0; pHdr<p_చివర_Hdr && iCurrent<=iIdx; iCurrent++){
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, పరిమాణం(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ఒకవేళ( iCurrent==iIdx ){
      sqlite3_ఫలితం_విలువ(context, &mem);
    }

    ఒకవేళ( mem.szMalloc ) sqlite3DbFree(db, mem.zMalloc);
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
స్థిర శూన్య test_decode(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  u8 *pRec;
  u8 *p_చివర_Hdr;                    /* Points to one byte past record header */
  u8 *pHdr;                       /* Current point in record header */
  u8 *pBody;                      /* Current point in record data */
  u64 nHdr;                       /* Bytes in record header */
  Tcl_Obj *pRet;                  /* Return value */

  pRet = Tcl_NewObj();
  Tcl_Incr_Ref_లెక్క(pRet);

  ప్రతిజ్ఞా( argc==1 );
  pRec = (u8*)sqlite3_విలువ_blob(argv[0]);

  pHdr = pRec + sqlite3GetVarint(pRec, &nHdr);
  pBody = p_చివర_Hdr = &pRec[nHdr];
  అయితే( pHdr<p_చివర_Hdr ){
    Tcl_Obj *pVal = 0;
    u64 iSerialType;
    Mem mem;

    memset(&mem, 0, పరిమాణం(mem));
    mem.db = db;
    mem.enc = ENC(db);
    pHdr += sqlite3GetVarint(pHdr, &iSerialType);
    sqlite3VdbeSerialGet(pBody, (u32)iSerialType, &mem);
    pBody += sqlite3VdbeSerialTypeLen((u32)iSerialType);

    ఎంపిక( sqlite3_విలువ_type(&mem) ){
      స్థితి SQLITE_పాఠ్యం:
        pVal = Tcl_NewStringObj((స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(&mem), -1);
        ఆపు;

      స్థితి SQLITE_BLOB: {
        అక్షరం hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7',
          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };
        పూర్ణాంకం n = sqlite3_విలువ_బైట్లు(&mem);
        u8 *z = (u8*)sqlite3_విలువ_blob(&mem);
        పూర్ణాంకం i;
        pVal = Tcl_NewStringObj("x'", -1);
        కొరకు(i=0; i<n; i++){
          అక్షరం hex[3];
          hex[0] = hexdigit[((z[i] >> 4) & 0x0F)];
          hex[1] = hexdigit[(z[i] & 0x0F)];
          hex[2] = '\0';
          Tcl_Append_అక్షరశ్రేణులు_గమ్యం_Obj(pVal, hex, 0);
        }
        Tcl_Append_అక్షరశ్రేణులు_గమ్యం_Obj(pVal, "'", 0);
        ఆపు;
      }

      స్థితి SQLITE_FLOAT:
        pVal = Tcl_NewDoubleObj(sqlite3_విలువ_double(&mem));
        ఆపు;

      స్థితి SQLITE_INTEGER:
        pVal = Tcl_NewWideIntObj(sqlite3_విలువ_int64(&mem));
        ఆపు;

      స్థితి SQLITE_NULL:
        pVal = Tcl_NewStringObj("NULL", -1);
        ఆపు;

      అప్రమేయ:
        ప్రతిజ్ఞా( 0 );
    }

    Tcl_ListObjAppendElement(0, pRet, pVal);

    ఒకవేళ( mem.szMalloc ){
      sqlite3DbFree(db, mem.zMalloc);
    }
  }

  sqlite3_ఫలితం_పాఠ్యం(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  Tcl_Decr_Ref_లెక్క(pRet);
}

/*
**       test_zeroblob(N)
**
** The implementation of scalar SQL function "test_zeroblob()". This is
** similar to the built-in zeroblob() function, except that it does not
** check that the integer parameter is within range before passing it
** to sqlite3_result_zeroblob().
*/
స్థిర శూన్య test_zeroblob(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం nZero = sqlite3_విలువ_int(argv[0]);
  sqlite3_ఫలితం_zeroblob(context, nZero);
}

/*         test_getsubtype(V)
**
** Return the subtype for value V.
*/
స్థిర శూన్య test_getsubtype(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3_ఫలితం_int(context, (పూర్ణాంకం)sqlite3_విలువ_subtype(argv[0]));
}

/*         test_frombind(A,B,C,...)
**
** Return an integer bitmask that has a bit set for every argument
** (up to the first 63 arguments) that originates from a bind a parameter.
*/
స్థిర శూన్య test_frombind(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3_uint64 m = 0;
  పూర్ణాంకం i;
  కొరకు(i=0; i<argc && i<63; i++){
    ఒకవేళ( sqlite3_విలువ_frombind(argv[i]) ) m |= ((sqlite3_uint64)1)<<i;
  }
  sqlite3_ఫలితం_int64(context, (sqlite3_int64)m);
}

/*         test_setsubtype(V, T)
**
** Return the value V with its subtype changed to T
*/
స్థిర శూన్య test_setsubtype(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3_ఫలితం_విలువ(context, argv[0]);
  sqlite3_ఫలితం_subtype(context, (గుర్తులేని పూర్ణాంకం)sqlite3_విలువ_int(argv[1]));
}

స్థిర పూర్ణాంకం రిజిస్టరు_Test_Functions(
  sqlite3 *db,
  అక్షరం **pz_Err_సందేశం,
  స్థిరరాశి sqlite3_api_routines *pThunk
){
  స్థిర స్థిరరాశి నిర్మాణం {
     అక్షరం *zName;
     గుర్తుగల అక్షరం nArg;
     గుర్తులేని పూర్ణాంకం e_పాఠ్యం_Rep; /* 1: UTF-16.  0: UTF-8 */
     శూన్య (*xFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **);
  } aFuncs[] = {
    { "randstr",               2, SQLITE_UTF8, randStr    },
    { "test_destructor",       1, SQLITE_UTF8, test_destructor},
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
    { "test_destructor16",     1, SQLITE_UTF8, test_destructor16},
    { "hex_to_utf16be",        1, SQLITE_UTF8, test_Hex_గమ్యం_Utf16be},
    { "hex_to_utf16le",        1, SQLITE_UTF8, test_Hex_గమ్యం_Utf16le},
#అయితే_ముగింపు
    { "hex_to_utf8",           1, SQLITE_UTF8, test_Hex_గమ్యం_Utf8},
    { "test_destructor_count", 0, SQLITE_UTF8, test_destructor_లెక్క},
    { "test_auxdata",         -1, SQLITE_UTF8, test_auxdata},
    { "test_error",            1, SQLITE_UTF8, test_error},
    { "test_error",            2, SQLITE_UTF8, test_error},
    { "test_eval",             1, SQLITE_UTF8, test_eval},
    { "test_isolation",        2, SQLITE_UTF8, test_isolation},
    { "test_counter",          1, SQLITE_UTF8, counterFunc},
    { "real2hex",              1, SQLITE_UTF8, real2hex},
    { "test_decode",           1, SQLITE_UTF8, test_decode},
    { "test_extract",          2, SQLITE_UTF8, test_వెలికితీయి},
    { "test_zeroblob",  1, SQLITE_UTF8|SQLITE_DETERMINISTIC, test_zeroblob},
    { "test_getsubtype",       1, SQLITE_UTF8, test_getsubtype},
    { "test_setsubtype",       2, SQLITE_UTF8|SQLITE_ఫలితం_SUBTYPE,
                                               test_setsubtype},
    { "test_frombind",        -1, SQLITE_UTF8, test_frombind},
  };
  పూర్ణాంకం i;

  కొరకు(i=0; i<పరిమాణం(aFuncs)/పరిమాణం(aFuncs[0]); i++){
    sqlite3_create_function(db, aFuncs[i].zName, aFuncs[i].nArg,
        aFuncs[i].e_పాఠ్యం_Rep, 0, aFuncs[i].xFunc, 0, 0);
  }

  sqlite3_create_function(db, "test_agg_errmsg16", 0, SQLITE_ANY, 0, 0, 
      test_agg_errmsg16_అడుగు, test_agg_errmsg16_final);
      
  తిరిగి SQLITE_OK;
}

/*
** TCLCMD:  autoinstall_test_functions
**
** Invoke this TCL command to use sqlite3_auto_extension() to cause
** the standard set of test functions to be loaded into each new
** database connection.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI autoinstall_test_funcs(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  బాహ్య పూర్ణాంకం Md5_రిజిస్టరు(sqlite3 *, అక్షరం **, స్థిరరాశి sqlite3_api_routines *);
  పూర్ణాంకం rc = sqlite3_auto_extension((శూన్య(*)(శూన్య))రిజిస్టరు_Test_Functions);
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3_auto_extension((శూన్య(*)(శూన్య))Md5_రిజిస్టరు);
  }
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** A bogus step function and finalizer function.
*/
స్థిర శూన్య t_అడుగు(sqlite3_context *a, పూర్ణాంకం b, sqlite3_విలువ **c){}
స్థిర శూన్య tFinal(sqlite3_context *a){}


/*
** tclcmd:  abuse_create_function
**
** Make various calls to sqlite3_create_function that do not have valid
** parameters.  Verify that the error condition is detected and reported.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI abuse_create_function(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp*, స్థిరరాశి అక్షరం*, sqlite3**);
  sqlite3 *db;
  పూర్ణాంకం rc;
  పూర్ణాంకం mxArg;

  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_అడుగు,t_అడుగు,tFinal);
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_అడుగు, t_అడుగు, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, t_అడుగు, 0, tFinal);
  ఒకవేళ( rc!=SQLITE_MISUSE) వెళ్ళు abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, 0, tFinal);
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు abuse_err;

  rc = sqlite3_create_function(db, "tx", 1, SQLITE_UTF8, 0, 0, t_అడుగు, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు abuse_err;

  rc = sqlite3_create_function(db, "tx", -2, SQLITE_UTF8, 0, t_అడుగు, 0, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు abuse_err;

  rc = sqlite3_create_function(db, "tx", 32768, SQLITE_UTF8, 0, t_అడుగు, 0, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు abuse_err;

  rc = sqlite3_create_function(db, "funcxx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       1, SQLITE_UTF8, 0, t_అడుగు, 0, 0);
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు abuse_err;

  /* This last function registration should actually work.  Generate
  ** a no-op function (that always returns NULL) and which has the
  ** maximum-length function name and the maximum number of parameters.
  */
  sqlite3_హద్దు(db, SQLITE_హద్దు_FUNCTION_ARG, 1000000);
  mxArg = sqlite3_హద్దు(db, SQLITE_హద్దు_FUNCTION_ARG, -1);
  rc = sqlite3_create_function(db, "nullx"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789"
       "_123456789_123456789_123456789_123456789_123456789",
       mxArg, SQLITE_UTF8, 0, t_అడుగు, 0, 0);
  ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు abuse_err;
                                
  తిరిగి TCL_OK;

abuse_err:
  Tcl_Append_ఫలితం(interp, "sqlite3_create_function abused test failed", 
                   (అక్షరం*)0);
  తిరిగి TCL_ERROR;
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
స్థిర శూన్య rankfunc(sqlite3_context *pCtx, పూర్ణాంకం nVal, sqlite3_విలువ **apVal){
  పూర్ణాంకం *aMatchinfo;                /* Return value of matchinfo() */
  పూర్ణాంకం nMatchinfo;                 /* Number of elements in aMatchinfo[] */
  పూర్ణాంకం n_నిలువు = 0;                   /* Number of columns in the table */
  పూర్ణాంకం nPhrase = 0;                /* Number of phrases in the query */
  పూర్ణాంకం iPhrase;                    /* Current phrase */
  ద్విగుణ score = 0.0;             /* Value to return */

  ప్రతిజ్ఞా( పరిమాణం(పూర్ణాంకం)==4 );

  /* Check that the number of arguments passed to this function is correct.
  ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
  ** of unsigned integer values returned by FTS function matchinfo. Set
  ** nPhrase to contain the number of reportable phrases in the users full-text
  ** query, and nCol to the number of columns in the table. Then check that the
  ** size of the matchinfo blob is as expected. Return an error if it is not.
  */
  ఒకవేళ( nVal<1 ) వెళ్ళు wrong_సంఖ్య_వాదనలు;
  aMatchinfo = (పూర్ణాంకం*)sqlite3_విలువ_blob(apVal[0]);
  nMatchinfo = sqlite3_విలువ_బైట్లు(apVal[0]) / పరిమాణం(పూర్ణాంకం);
  ఒకవేళ( nMatchinfo>=2 ){
    nPhrase = aMatchinfo[0];
    n_నిలువు = aMatchinfo[1];
  }
  ఒకవేళ( nMatchinfo!=(2+3*n_నిలువు*nPhrase) ){
    sqlite3_ఫలితం_error(pCtx,
        "invalid matchinfo blob passed to function rank()", -1);
    తిరిగి;
  }
  ఒకవేళ( nVal!=(1+n_నిలువు) ) వెళ్ళు wrong_సంఖ్య_వాదనలు;

  /* Iterate through each phrase in the users query. */
  కొరకు(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    పూర్ణాంకం i_నిలువు;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
    ** increment the relevancy score by:
    **
    **   (<hit count> / <global hit count>) * <column weight>
    **
    ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
    ** the hit count and global hit counts for each column are found in 
    ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
    */
    పూర్ణాంకం *aPhraseinfo = &aMatchinfo[2 + iPhrase*n_నిలువు*3];
    కొరకు(i_నిలువు=0; i_నిలువు<n_నిలువు; i_నిలువు++){
      పూర్ణాంకం n_తగిలింది_లెక్క = aPhraseinfo[3*i_నిలువు];
      పూర్ణాంకం n_Global_తగిలింది_లెక్క = aPhraseinfo[3*i_నిలువు+1];
      ద్విగుణ బరువు = sqlite3_విలువ_double(apVal[i_నిలువు+1]);
      ఒకవేళ( n_తగిలింది_లెక్క>0 ){
        score += ((ద్విగుణ)n_తగిలింది_లెక్క / (ద్విగుణ)n_Global_తగిలింది_లెక్క) * బరువు;
      }
    }
  }

  sqlite3_ఫలితం_double(pCtx, score);
  తిరిగి;

  /* Jump here if the wrong number of arguments are passed to this function */
wrong_సంఖ్య_వాదనలు:
  sqlite3_ఫలితం_error(pCtx, "wrong number of arguments to function rank()", -1);
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI install_fts3_rank_function(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp*, స్థిరరాశి అక్షరం*, sqlite3**);
  sqlite3 *db;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  sqlite3_create_function(db, "rank", -1, SQLITE_UTF8, 0, rankfunc, 0, 0);
  తిరిగి TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest_func_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "autoinstall_test_functions",    autoinstall_test_funcs },
     { "abuse_create_function",         abuse_create_function  },
     { "install_fts3_rank_function",    install_fts3_rank_function  },
  };
  పూర్ణాంకం i;
  బాహ్య పూర్ణాంకం Md5_రిజిస్టరు(sqlite3 *, అక్షరం **, స్థిరరాశి sqlite3_api_routines *);

  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  sqlite3_initialize();
  sqlite3_auto_extension((శూన్య(*)(శూన్య))రిజిస్టరు_Test_Functions);
  sqlite3_auto_extension((శూన్య(*)(శూన్య))Md5_రిజిస్టరు);
  తిరిగి TCL_OK;
}
