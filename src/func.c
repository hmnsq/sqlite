/*
** 2002 February 23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C-language implementations for many of the SQL
** functions of SQLite.  (Some function, and in particular the date and
** time functions, are implemented separately.)
*/
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <assert.h>
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
#అన్తర్భావ <math.h>
#అయితే_ముగింపు
#అన్తర్భావ "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
స్థిర CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  ప్రతిజ్ఞా( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  ప్రతిజ్ఞా( pOp->opcode==OP_CollSeq );
  ప్రతిజ్ఞా( pOp->p4type==P4_COLLSEQ );
  తిరిగి pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
స్థిర శూన్య sqlite3_Skip_సంచాయకం_ఎక్కించు(sqlite3_context *context){
  ప్రతిజ్ఞా( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
స్థిర శూన్య minmaxFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం i;
  పూర్ణాంకం ముసుగు;    /* 0 for min() or 0xffffffff for max() */
  పూర్ణాంకం i_ఉత్తమ;
  CollSeq *pColl;

  ప్రతిజ్ఞా( argc>1 );
  ముసుగు = sqlite3_user_దత్తాంశం(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  ప్రతిజ్ఞా( pColl );
  ప్రతిజ్ఞా( ముసుగు==-1 || ముసుగు==0 );
  i_ఉత్తమ = 0;
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ) తిరిగి;
  కొరకు(i=1; i<argc; i++){
    ఒకవేళ( sqlite3_విలువ_type(argv[i])==SQLITE_NULL ) తిరిగి;
    ఒకవేళ( (sqlite3MemCompare(argv[i_ఉత్తమ], argv[i], pColl)^ముసుగు)>=0 ){
      testcase( ముసుగు==0 );
      i_ఉత్తమ = i;
    }
  }
  sqlite3_ఫలితం_విలువ(context, argv[i_ఉత్తమ]);
}

/*
** Return the type of the argument.
*/
స్థిర శూన్య typeofFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  స్థిర స్థిరరాశి అక్షరం *azType[] = { "integer", "real", "text", "blob", "null" };
  పూర్ణాంకం i = sqlite3_విలువ_type(argv[0]) - 1;
  వాడని_PARAMETER(Not_వాడిన);
  ప్రతిజ్ఞా( i>=0 && i<Array_కొలత(azType) );
  ప్రతిజ్ఞా( SQLITE_INTEGER==1 );
  ప్రతిజ్ఞా( SQLITE_FLOAT==2 );
  ప్రతిజ్ఞా( SQLITE_పాఠ్యం==3 );
  ప్రతిజ్ఞా( SQLITE_BLOB==4 );
  ప్రతిజ్ఞా( SQLITE_NULL==5 );
  /* EVIDENCE-OF: R-01470-60482 The sqlite3_value_type(V) interface returns
  ** the datatype code for the initial datatype of the sqlite3_value object
  ** V. The returned value is one of SQLITE_INTEGER, SQLITE_FLOAT,
  ** SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL. */
  sqlite3_ఫలితం_పాఠ్యం(context, azType[i], -1, SQLITE_STATIC);
}

/* subtype(X)
**
** Return the subtype of X
*/
స్థిర శూన్య subtypeFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  వాడని_PARAMETER(argc);
  sqlite3_ఫలితం_int(context, sqlite3_విలువ_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
స్థిర శూన్య నిడివి_Func(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  ఎంపిక( sqlite3_విలువ_type(argv[0]) ){
    స్థితి SQLITE_BLOB:
    స్థితి SQLITE_INTEGER:
    స్థితి SQLITE_FLOAT: {
      sqlite3_ఫలితం_int(context, sqlite3_విలువ_బైట్లు(argv[0]));
      ఆపు;
    }
    స్థితి SQLITE_పాఠ్యం: {
      స్థిరరాశి గుర్తులేని అక్షరం *z = sqlite3_విలువ_పాఠ్యం(argv[0]);
      స్థిరరాశి గుర్తులేని అక్షరం *z0;
      ఒకవేళ( z==0 ) తిరిగి;
      z0 = z;
      అయితే( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        ఒకవేళ( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }లేకపోతే ఒకవేళ( z[0]==0 ){
          ఆపు;
        }లేకపోతే{
          z++;
          అయితే( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_ఫలితం_int(context, (పూర్ణాంకం)(z-z0));
      ఆపు;
    }
    అప్రమేయ: {
      sqlite3_ఫలితం_null(context);
      ఆపు;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
స్థిర శూన్య bytelengthFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  ఎంపిక( sqlite3_విలువ_type(argv[0]) ){
    స్థితి SQLITE_BLOB: {
      sqlite3_ఫలితం_int(context, sqlite3_విలువ_బైట్లు(argv[0]));
      ఆపు;
    }
    స్థితి SQLITE_INTEGER:
    స్థితి SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_ఫలితం_int64(context, sqlite3_విలువ_బైట్లు(argv[0])*m);
      ఆపు;
    }
    స్థితి SQLITE_పాఠ్యం: {
      ఒకవేళ( sqlite3_విలువ_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_ఫలితం_int(context, sqlite3_విలువ_బైట్లు(argv[0]));
      }లేకపోతే{
        sqlite3_ఫలితం_int(context, sqlite3_విలువ_bytes16(argv[0]));
      }
      ఆపు;
    }
    అప్రమేయ: {
      sqlite3_ఫలితం_null(context);
      ఆపు;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
స్థిర శూన్య absFunc(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  ఎంపిక( sqlite3_విలువ_type(argv[0]) ){
    స్థితి SQLITE_INTEGER: {
      i64 iVal = sqlite3_విలువ_int64(argv[0]);
      ఒకవేళ( iVal<0 ){
        ఒకవేళ( iVal==అతిచిన్న_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_ఫలితం_error(context, "integer overflow", -1);
          తిరిగి;
        }
        iVal = -iVal;
      }
      sqlite3_ఫలితం_int64(context, iVal);
      ఆపు;
    }
    స్థితి SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_ఫలితం_null(context);
      ఆపు;
    }
    అప్రమేయ: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      ద్విగుణ rVal = sqlite3_విలువ_double(argv[0]);
      ఒకవేళ( rVal<0 ) rVal = -rVal;
      sqlite3_ఫలితం_double(context, rVal);
      ఆపు;
    }
  }
}

/*
** Implementation of the instr() function.
**
** instr(haystack,needle) finds the first occurrence of needle
** in haystack and returns the number of previous characters plus 1,
** or 0 if needle does not occur within haystack.
**
** If both haystack and needle are BLOBs, then the result is one more than
** the number of bytes in haystack prior to the first occurrence of needle,
** or 0 if needle never occurs in haystack.
*/
స్థిర శూన్య instrFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి గుర్తులేని అక్షరం *zHaystack;
  స్థిరరాశి గుర్తులేని అక్షరం *zNeedle;
  పూర్ణాంకం nHaystack;
  పూర్ణాంకం nNeedle;
  పూర్ణాంకం typeHaystack, typeNeedle;
  పూర్ణాంకం N = 1;
  పూర్ణాంకం is_పాఠ్యం;
  గుర్తులేని అక్షరం మొదటి_Char;
  sqlite3_విలువ *pC1 = 0;
  sqlite3_విలువ *pC2 = 0;

  వాడని_PARAMETER(argc);
  typeHaystack = sqlite3_విలువ_type(argv[0]);
  typeNeedle = sqlite3_విలువ_type(argv[1]);
  ఒకవేళ( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) తిరిగి;
  nHaystack = sqlite3_విలువ_బైట్లు(argv[0]);
  nNeedle = sqlite3_విలువ_బైట్లు(argv[1]);
  ఒకవేళ( nNeedle>0 ){
    ఒకవేళ( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_విలువ_blob(argv[0]);
      zNeedle = sqlite3_విలువ_blob(argv[1]);
      is_పాఠ్యం = 0;
    }లేకపోతే ఒకవేళ( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_విలువ_పాఠ్యం(argv[0]);
      zNeedle = sqlite3_విలువ_పాఠ్యం(argv[1]);
      is_పాఠ్యం = 1;
    }లేకపోతే{
      pC1 = sqlite3_విలువ_dup(argv[0]);
      zHaystack = sqlite3_విలువ_పాఠ్యం(pC1);
      ఒకవేళ( zHaystack==0 ) వెళ్ళు చివర_Instr_OOM;
      nHaystack = sqlite3_విలువ_బైట్లు(pC1);
      pC2 = sqlite3_విలువ_dup(argv[1]);
      zNeedle = sqlite3_విలువ_పాఠ్యం(pC2);
      ఒకవేళ( zNeedle==0 ) వెళ్ళు చివర_Instr_OOM;
      nNeedle = sqlite3_విలువ_బైట్లు(pC2);
      is_పాఠ్యం = 1;
    }
    ఒకవేళ( zNeedle==0 || (nHaystack && zHaystack==0) ) వెళ్ళు చివర_Instr_OOM;
    మొదటి_Char = zNeedle[0];
    అయితే( nNeedle<=nHaystack
       && (zHaystack[0]!=మొదటి_Char || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      చేయి{
        nHaystack--;
        zHaystack++;
      }అయితే( is_పాఠ్యం && (zHaystack[0]&0xc0)==0x80 );
    }
    ఒకవేళ( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_ఫలితం_int(context, N);
చివర_Instr:
  sqlite3_విలువ_free(pC1);
  sqlite3_విలువ_free(pC2);
  తిరిగి;
చివర_Instr_OOM:
  sqlite3_ఫలితం_error_nomem(context);
  వెళ్ళు చివర_Instr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
స్థిర శూన్య printfFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  PrintfArguments x;
  StrAccum str;
  స్థిరరాశి అక్షరం *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  ఒకవేళ( argc>=1 && (zFormat = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.n_వాడిన = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->a_హద్దు[SQLITE_హద్దు_నిడివి]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3_str_appendf(&str, zFormat, &x);
    sqlite3_ఫలితం_str(context, &str, SQLITE_XFER);
  }
}

/* Return true if any of the eight bytes in word is NUL or non-ASCII. */
స్థిర SQLITE_INLINE పూర్ణాంకం కలిగియుంది_Nul_Or_Non_Ascii(u64 మాట){
  స్థిరరాశి u64 byteOnes = UINT64_C(0x0101010101010101);
  స్థిరరాశి u64 byte_ఎగువ_బిట్లు = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  తిరిగి ((మాట | (మాట - byteOnes)) & byte_ఎగువ_బిట్లు)!=0;
}

/*
** Implementation of the substr() function.
**
** substr(x,p1,p2)  returns p2 characters of x[] beginning with p1.
** p1 is 1-indexed.  So substr(x,1,1) returns the first character
** of x.  If x is text, then we actually count UTF-8 characters.
** If x is a blob, then we count bytes.
**
** If p1 is negative, then we begin abs(p1) from the end of x[].
**
** If p2 is negative, return the p2 characters preceding p1.
*/
స్థిర శూన్య substrFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి గుర్తులేని అక్షరం *z;
  స్థిరరాశి గుర్తులేని అక్షరం *z2;
  పూర్ణాంకం len;
  పూర్ణాంకం p0type;
  i64 p1, p2;

  ప్రతిజ్ఞా( argc==3 || argc==2 );
  p0type = sqlite3_విలువ_type(argv[0]);
  p1 = sqlite3_విలువ_int64(argv[1]);
  ఒకవేళ( p0type==SQLITE_BLOB ){
    len = sqlite3_విలువ_బైట్లు(argv[0]);
    z = sqlite3_విలువ_blob(argv[0]);
    ఒకవేళ( z==0 ) తిరిగి;
    ప్రతిజ్ఞా( len==sqlite3_విలువ_బైట్లు(argv[0]) );
  }లేకపోతే{
    z = sqlite3_విలువ_పాఠ్యం(argv[0]);
    ఒకవేళ( z==0 ) తిరిగి;
    len = 0;
    ఒకవేళ( p1<0 ){
      కొరకు(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  ఒకవేళ( argc==3 ){
    p2 = sqlite3_విలువ_int64(argv[2]);
    ఒకవేళ( p2==0 && sqlite3_విలువ_type(argv[2])==SQLITE_NULL ) తిరిగి;
  }లేకపోతే{
    p2 = అతిపెద్ద_INT64;
  }
  ఒకవేళ( p1==0 ){
#నిర్వచితమైతే SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#అయితే_ముగింపు
    ఒకవేళ( sqlite3_విలువ_type(argv[1])==SQLITE_NULL ) తిరిగి;
  }
  ఒకవేళ( p1<0 ){
    p1 += len;
    ఒకవేళ( p1<0 ){
      ఒకవేళ( p2<0 ){
        p2 = 0;
      }లేకపోతే{
        p2 += p1;
      }
      p1 = 0;
    }
  }లేకపోతే ఒకవేళ( p1>0 ){
    p1--;
  }లేకపోతే ఒకవేళ( p2>0 ){
    p2--;
  }
  ఒకవేళ( p2<0 ){
    ఒకవేళ( p2<-p1 ){
      p2 = p1;
    }లేకపోతే{
      p2 = -p2;
    }
    p1 -= p2;
  }
  ప్రతిజ్ఞా( p1>=0 && p2>=0 );
  ఒకవేళ( p0type!=SQLITE_BLOB ){
    ఒకవేళ( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      స్థిరరాశి గుర్తులేని అక్షరం *z_చివర = z + sqlite3_విలువ_బైట్లు(argv[0]);
      అయితే( p1>=8 && z_చివర-z>=8 ){
        u64 మాట;
        జ్ఞాపకం_నకలు(&మాట, z, పరిమాణం(మాట));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        ఒకవేళ( కలిగియుంది_Nul_Or_Non_Ascii(మాట) ){
          ఆపు;
        }
        z += 8;
        p1 -= 8;
      }
    }
    కొరకు( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      ఒకవేళ( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }లేకపోతే ఒకవేళ( z[0]==0 ){
        ఆపు;
      }లేకపోతే{
        చేయి{ z++; }అయితే( (z[0]&0xc0)==0x80 );
      }
    }
    కొరకు(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      ఒకవేళ( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }లేకపోతే ఒకవేళ( z2[0]==0 ){
        ఆపు;
      }లేకపోతే{
        చేయి{ z2++; }అయితే( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_ఫలితం_text64(context, (అక్షరం*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }లేకపోతే{
    ఒకవేళ( p1>=len ){
      p1 = p2 = 0;
    }లేకపోతే ఒకవేళ( p2>len-p1 ){
      p2 = len-p1;
      ప్రతిజ్ఞా( p2>0 );
    }
    sqlite3_ఫలితం_blob64(context, (అక్షరం*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
స్థిర శూన్య విడత_Func(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  i64 n = 0;
  ద్విగుణ r;
  అక్షరం *zBuf;
  ప్రతిజ్ఞా( argc==1 || argc==2 );
  ఒకవేళ( argc==2 ){
    ఒకవేళ( SQLITE_NULL==sqlite3_విలువ_type(argv[1]) ) తిరిగి;
    n = sqlite3_విలువ_int64(argv[1]);
    ఒకవేళ( n>30 ) n = 30;
    ఒకవేళ( n<0 ) n = 0;
  }
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ) తిరిగి;
  r = sqlite3_విలువ_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  ఒకవేళ( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }లేకపోతే ఒకవేళ( n==0 ){ 
    r = (ద్విగుణ)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }లేకపోతే{
    zBuf = sqlite3_mprintf("%!.*f",(పూర్ణాంకం)n,r);
    ఒకవేళ( zBuf==0 ){
      sqlite3_ఫలితం_error_nomem(context);
      తిరిగి;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_ఫలితం_double(context, r);
}
#అయితే_ముగింపు

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
స్థిర శూన్య *contextMalloc(sqlite3_context *context, i64 nByte){
  అక్షరం *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ప్రతిజ్ఞా( nByte>0 );
  testcase( nByte==db->a_హద్దు[SQLITE_హద్దు_నిడివి] );
  testcase( nByte==(i64)db->a_హద్దు[SQLITE_హద్దు_నిడివి]+1 );
  ఒకవేళ( nByte>db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
    sqlite3_ఫలితం_error_toobig(context);
    z = 0;
  }లేకపోతే{
    z = sqlite3Malloc(nByte);
    ఒకవేళ( !z ){
      sqlite3_ఫలితం_error_nomem(context);
    }
  }
  తిరిగి z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
స్థిర శూన్య పై_Func(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  అక్షరం *z1;
  స్థిరరాశి అక్షరం *z2;
  పూర్ణాంకం i, n;
  వాడని_PARAMETER(argc);
  z2 = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  n = sqlite3_విలువ_బైట్లు(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ప్రతిజ్ఞా( z2==(అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]) );
  ఒకవేళ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    ఒకవేళ( z1 ){
      కొరకు(i=0; i<n; i++){
        z1[i] = (అక్షరం)sqlite3Toupper(z2[i]);
      }
      sqlite3_ఫలితం_పాఠ్యం(context, z1, n, sqlite3_free);
    }
  }
}
స్థిర శూన్య క్రింది_Func(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  అక్షరం *z1;
  స్థిరరాశి అక్షరం *z2;
  పూర్ణాంకం i, n;
  వాడని_PARAMETER(argc);
  z2 = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  n = sqlite3_విలువ_బైట్లు(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  ప్రతిజ్ఞా( z2==(అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]) );
  ఒకవేళ( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    ఒకవేళ( z1 ){
      కొరకు(i=0; i<n; i++){
        z1[i] = sqlite3Tolower(z2[i]);
      }
      sqlite3_ఫలితం_పాఠ్యం(context, z1, n, sqlite3_free);
    }
  }
}

/*
** Some functions like COALESCE() and IFNULL() and UNLIKELY() are implemented
** as VDBE code so that unused argument values do not have to be computed.
** However, we still need some kind of function implementation for this
** routines in the function table.  The noopFunc macro provides this.
** noopFunc will never be called so it doesn't matter what the implementation
** is.  We might as well use the "version()" function as a substitute.
*/
#నిర్వచించు noopFunc సంస్కరణ_Func   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer. 
*/
స్థిర శూన్య randomFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  sqlite_int64 r;
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  sqlite3_randomness(పరిమాణం(r), &r);
  ఒకవేళ( r<0 ){
    /* We need to prevent a random number of 0x8000000000000000
    ** (or -9223372036854775808) since when you do abs() of that
    ** number of you get the same value back again.  To do this
    ** in a way that is testable, mask the sign bit off of negative
    ** values, resulting in a positive value.  Then take the
    ** 2s complement of that positive value.  The end result can
    ** therefore be no less than -9223372036854775807.
    */
    r = -(r & అతిపెద్ద_INT64);
  }
  sqlite3_ఫలితం_int64(context, r);
}

/*
** Implementation of randomblob(N).  Return a random blob
** that is N bytes long.
*/
స్థిర శూన్య randomBlob(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3_int64 n;
  గుర్తులేని అక్షరం *p;
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  n = sqlite3_విలువ_int64(argv[0]);
  ఒకవేళ( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  ఒకవేళ( p ){
    sqlite3_randomness(n, p);
    sqlite3_ఫలితం_blob(context, (అక్షరం*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
స్థిర శూన్య చివరి_చొప్పించు_rowid(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_ఫలితం_int64(context, sqlite3_చివరి_చొప్పించు_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
స్థిర శూన్య changes(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  sqlite3_ఫలితం_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
స్థిర శూన్య మొత్తం_changes(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  /* IMP: R-11217-42568 This function is a wrapper around the
  ** sqlite3_total_changes64() C/C++ interface. */
  sqlite3_ఫలితం_int64(context, sqlite3_మొత్తం_changes64(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
నిర్మాణం compareInfo {
  u8 సరిపోలిక_All;          /* "*" or "%" */
  u8 సరిపోలిక_One;          /* "?" or "_" */
  u8 సరిపోలిక_సమితి;          /* "[" or 0 */
  u8 noCase;            /* true to ignore case differences */
};

/*
** For LIKE and GLOB matching on EBCDIC machines, assume that every
** character is exactly one byte in size.  Also, provide the Utf8Read()
** macro for fast reading of the next character in the common case where
** the next character is ASCII.
*/
#ఒకవేళ defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#లేకపోతే
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#అయితే_ముగింపు

స్థిర స్థిరరాశి నిర్మాణం compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
స్థిర స్థిరరాశి నిర్మాణం compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
స్థిర స్థిరరాశి నిర్మాణం compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Possible error returns from patternMatch()
*/
#నిర్వచించు SQLITE_సరిపోలిక             0
#నిర్వచించు SQLITE_NOMATCH           1
#నిర్వచించు SQLITE_NOWILDCARDMATCH   2

/*
** Compare two UTF-8 strings for equality where the first string is
** a GLOB or LIKE expression.  Return values:
**
**    SQLITE_MATCH:            Match
**    SQLITE_NOMATCH:          No match
**    SQLITE_NOWILDCARDMATCH:  No match in spite of having * or % wildcards.
**
** Globbing rules:
**
**      '*'       Matches any sequence of zero or more characters.
**
**      '?'       Matches exactly one character.
**
**     [...]      Matches one character from the enclosed list of
**                characters.
**
**     [^...]     Matches one character not in the enclosed list.
**
** With the [...] and [^...] matching, a ']' character can be included
** in the list by making it the first character after '[' or '^'.  A
** range of characters can be specified using '-'.  Example:
** "[a-z]" matches any single lower-case letter.  To match a '-', make
** it the last character in the list.
**
** Like matching rules:
**
**      '%'       Matches any sequence of zero or more characters
**
***     '_'       Matches any one character
**
**      Ec        Where E is the "esc" character and c is any other
**                character, including '%', '_', and esc, match exactly c.
**
** The comments within this routine usually assume glob matching.
**
** This routine is usually quick, but can be N**2 in the worst case.
*/
స్థిర పూర్ణాంకం నమూనా_Compare(
  స్థిరరాశి u8 *z_నమూనా,              /* The glob pattern */
  స్థిరరాశి u8 *zString,               /* The string to compare against the glob */
  స్థిరరాశి నిర్మాణం compareInfo *pInfo, /* Information about how to do the compare */
  u32 సరిపోలిక_Other                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 సరిపోలిక_One = pInfo->సరిపోలిక_One;  /* "?" or "_" */
  u32 సరిపోలిక_All = pInfo->సరిపోలిక_All;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  స్థిరరాశి u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  అయితే( (c = Utf8Read(z_నమూనా))!=0 ){
    ఒకవేళ( c==సరిపోలిక_All ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      అయితే( (c=Utf8Read(z_నమూనా)) == సరిపోలిక_All
             || (c == సరిపోలిక_One && సరిపోలిక_One!=0) ){
        ఒకవేళ( c==సరిపోలిక_One && sqlite3Utf8Read(&zString)==0 ){
          తిరిగి SQLITE_NOWILDCARDMATCH;
        }
      }
      ఒకవేళ( c==0 ){
        తిరిగి SQLITE_సరిపోలిక;   /* "*" at the end of the pattern matches */
      }లేకపోతే ఒకవేళ( c==సరిపోలిక_Other ){
        ఒకవేళ( pInfo->సరిపోలిక_సమితి==0 ){
          c = sqlite3Utf8Read(&z_నమూనా);
          ఒకవేళ( c==0 ) తిరిగి SQLITE_NOWILDCARDMATCH;
        }లేకపోతే{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          ప్రతిజ్ఞా( సరిపోలిక_Other<0x80 );  /* '[' is a single-byte character */
          అయితే( *zString ){
            పూర్ణాంకం b_సరిపోలిక = నమూనా_Compare(&z_నమూనా[-1],zString,pInfo,సరిపోలిక_Other);
            ఒకవేళ( b_సరిపోలిక!=SQLITE_NOMATCH ) తిరిగి b_సరిపోలిక;
            SQLITE_SKIP_UTF8(zString);
          }
          తిరిగి SQLITE_NOWILDCARDMATCH;
        }
      }

      /* At this point variable c contains the first character of the
      ** pattern string past the "*".  Search in the input string for the
      ** first matching character and recursively continue the match from
      ** that point.
      **
      ** For a case-insensitive search, set variable cx to be the same as
      ** c but in the other case and search the input string for either
      ** c or cx.
      */
      ఒకవేళ( c<0x80 ){
        అక్షరం zStop[3];
        పూర్ణాంకం b_సరిపోలిక;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = z_నమూనా[0], ny = nx;
        ఒకవేళ( nx>=0x80 || nx==సరిపోలిక_All || nx==సరిపోలిక_One || nx==సరిపోలిక_Other ){
          nx = 0;
        }
        ఒకవేళ( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }లేకపోతే{
          zStop[0] = c;
          zStop[1] = 0;
        }
        అయితే(1){
          zString += strcspn((స్థిరరాశి అక్షరం*)zString, zStop);
          ఒకవేళ( zString[0]==0 ) ఆపు;
          zString++;
          ఒకవేళ( nx && zString[0]!=nx && zString[0]!=ny ) కొనసాగు;
          b_సరిపోలిక = నమూనా_Compare(z_నమూనా,zString,pInfo,సరిపోలిక_Other);
          ఒకవేళ( b_సరిపోలిక!=SQLITE_NOMATCH ) తిరిగి b_సరిపోలిక;
        }
      }లేకపోతే{
        పూర్ణాంకం b_సరిపోలిక;
        అయితే( (c2 = Utf8Read(zString))!=0 ){
          ఒకవేళ( c2!=c ) కొనసాగు;
          b_సరిపోలిక = నమూనా_Compare(z_నమూనా,zString,pInfo,సరిపోలిక_Other);
          ఒకవేళ( b_సరిపోలిక!=SQLITE_NOMATCH ) తిరిగి b_సరిపోలిక;
        }
      }
      తిరిగి SQLITE_NOWILDCARDMATCH;
    }
    ఒకవేళ( c==సరిపోలిక_Other ){
      ఒకవేళ( pInfo->సరిపోలిక_సమితి==0 ){
        c = sqlite3Utf8Read(&z_నమూనా);
        ఒకవేళ( c==0 ) తిరిగి SQLITE_NOMATCH;
        zEscaped = z_నమూనా;
      }లేకపోతే{
        u32 పూర్వ_c = 0;
        పూర్ణాంకం చూసిన = 0;
        పూర్ణాంకం invert = 0;
        c = sqlite3Utf8Read(&zString);
        ఒకవేళ( c==0 ) తిరిగి SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&z_నమూనా);
        ఒకవేళ( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&z_నమూనా);
        }
        ఒకవేళ( c2==']' ){
          ఒకవేళ( c==']' ) చూసిన = 1;
          c2 = sqlite3Utf8Read(&z_నమూనా);
        }
        అయితే( c2 && c2!=']' ){
          ఒకవేళ( c2=='-' && z_నమూనా[0]!=']' && z_నమూనా[0]!=0 && పూర్వ_c>0 ){
            c2 = sqlite3Utf8Read(&z_నమూనా);
            ఒకవేళ( c>=పూర్వ_c && c<=c2 ) చూసిన = 1;
            పూర్వ_c = 0;
          }లేకపోతే{
            ఒకవేళ( c==c2 ){
              చూసిన = 1;
            }
            పూర్వ_c = c2;
          }
          c2 = sqlite3Utf8Read(&z_నమూనా);
        }
        ఒకవేళ( c2==0 || (చూసిన ^ invert)==0 ){
          తిరిగి SQLITE_NOMATCH;
        }
        కొనసాగు;
      }
    }
    c2 = Utf8Read(zString);
    ఒకవేళ( c==c2 ) కొనసాగు;
    ఒకవేళ( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      కొనసాగు;
    }
    ఒకవేళ( c==సరిపోలిక_One && z_నమూనా!=zEscaped && c2!=0 ) కొనసాగు;
    తిరిగి SQLITE_NOMATCH;
  }
  తిరిగి *zString==0 ? SQLITE_సరిపోలిక : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
పూర్ణాంకం sqlite3_strglob(స్థిరరాశి అక్షరం *z_Glob_నమూనా, స్థిరరాశి అక్షరం *zString){
  ఒకవేళ( zString==0 ){
    తిరిగి z_Glob_నమూనా!=0;
  }లేకపోతే ఒకవేళ( z_Glob_నమూనా==0 ){
    తిరిగి 1;
  }లేకపోతే {
    తిరిగి నమూనా_Compare((u8*)z_Glob_నమూనా, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
పూర్ణాంకం sqlite3_strlike(స్థిరరాశి అక్షరం *z_నమూనా, స్థిరరాశి అక్షరం *zStr, గుర్తులేని పూర్ణాంకం esc){
  ఒకవేళ( zStr==0 ){
    తిరిగి z_నమూనా!=0;
  }లేకపోతే ఒకవేళ( z_నమూనా==0 ){
    తిరిగి 1;
  }లేకపోతే{
    తిరిగి నమూనా_Compare((u8*)z_నమూనా, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3_like_లెక్క = 0;
#అయితే_ముగింపు


/*
** Implementation of the like() SQL function.  This function implements
** the built-in LIKE operator.  The first argument to the function is the
** pattern and the second argument is the string.  So, the SQL statements:
**
**       A LIKE B
**
** is implemented as like(B,A).
**
** This same function (with a different compareInfo structure) computes
** the GLOB operator.
*/
స్థిర శూన్య likeFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి గుర్తులేని అక్షరం *zA, *zB;
  u32 escape;
  పూర్ణాంకం nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  నిర్మాణం compareInfo *pInfo = sqlite3_user_దత్తాంశం(context);
  నిర్మాణం compareInfo backupInfo;

#నిర్వచితమైతే SQLITE_LIKE_DOESNT_సరిపోలిక_BLOBS
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_BLOB
   || sqlite3_విలువ_type(argv[1])==SQLITE_BLOB
  ){
#నిర్వచితమైతే SQLITE_TEST
    sqlite3_like_లెక్క++;
#అయితే_ముగింపు
    sqlite3_ఫలితం_int(context, 0);
    తిరిగి;
  }
#అయితే_ముగింపు

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_విలువ_బైట్లు(argv[0]);
  testcase( nPat==db->a_హద్దు[SQLITE_హద్దు_LIKE_నమూనా_నిడివి] );
  testcase( nPat==db->a_హద్దు[SQLITE_హద్దు_LIKE_నమూనా_నిడివి]+1 );
  ఒకవేళ( nPat > db->a_హద్దు[SQLITE_హద్దు_LIKE_నమూనా_నిడివి] ){
    sqlite3_ఫలితం_error(context, "LIKE or GLOB pattern too complex", -1);
    తిరిగి;
  }
  ఒకవేళ( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    స్థిరరాశి గుర్తులేని అక్షరం *zEsc = sqlite3_విలువ_పాఠ్యం(argv[2]);
    ఒకవేళ( zEsc==0 ) తిరిగి;
    ఒకవేళ( sqlite3Utf8CharLen((అక్షరం*)zEsc, -1)!=1 ){
      sqlite3_ఫలితం_error(context,
          "ESCAPE expression must be a single character", -1);
      తిరిగి;
    }
    escape = sqlite3Utf8Read(&zEsc);
    ఒకవేళ( escape==pInfo->సరిపోలిక_All || escape==pInfo->సరిపోలిక_One ){
      జ్ఞాపకం_నకలు(&backupInfo, pInfo, పరిమాణం(backupInfo));
      pInfo = &backupInfo;
      ఒకవేళ( escape==pInfo->సరిపోలిక_All ) pInfo->సరిపోలిక_All = 0;
      ఒకవేళ( escape==pInfo->సరిపోలిక_One ) pInfo->సరిపోలిక_One = 0;
    }
  }లేకపోతే{
    escape = pInfo->సరిపోలిక_సమితి;
  }
  zB = sqlite3_విలువ_పాఠ్యం(argv[0]);
  zA = sqlite3_విలువ_పాఠ్యం(argv[1]);
  ఒకవేళ( zA && zB ){
#నిర్వచితమైతే SQLITE_TEST
    sqlite3_like_లెక్క++;
#అయితే_ముగింపు
    sqlite3_ఫలితం_int(context,
                      నమూనా_Compare(zB, zA, pInfo, escape)==SQLITE_సరిపోలిక);
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
స్థిర శూన్య nullifFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  వాడని_PARAMETER(Not_వాడిన);
  ఒకవేళ( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_ఫలితం_విలువ(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
స్థిర శూన్య సంస్కరణ_Func(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_ఫలితం_పాఠ్యం(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
స్థిర శూన్య sourceidFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_ఫలితం_పాఠ్యం(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
స్థిర శూన్య errlogFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  వాడని_PARAMETER(argc);
  వాడని_PARAMETER(context);
  sqlite3_log(sqlite3_విలువ_int(argv[0]), "%s", sqlite3_విలువ_పాఠ్యం(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPILEOPTION_DIAGS
స్థిర శూన్య compileoptionusedFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి అక్షరం *zOptName;
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  ఒకవేళ( (zOptName = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]))!=0 ){
    sqlite3_ఫలితం_int(context, sqlite3_compileoption_వాడిన(zOptName));
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function.
** The result is a string that identifies the compiler options
** used to build SQLite.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPILEOPTION_DIAGS
స్థిర శూన్య compileoptiongetFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  i64 n;
  స్థిరరాశి అక్షరం *zRes;
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_విలువ_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((పూర్ణాంకం)n) : 0;
  sqlite3_ఫలితం_పాఠ్యం(context, zRes, -1, SQLITE_STATIC);
}
#అయితే_ముగింపు /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
స్థిర స్థిరరాశి అక్షరం hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
శూన్య sqlite3_Quote_విలువ(StrAccum *pStr, sqlite3_విలువ *p_విలువ, పూర్ణాంకం bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  ప్రతిజ్ఞా( pStr!=0 && pStr->nChar==0 );

  ఎంపిక( sqlite3_విలువ_type(p_విలువ) ){
    స్థితి SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_విలువ_double(p_విలువ));
      ఆపు;
    }
    స్థితి SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_విలువ_int64(p_విలువ));
      ఆపు;
    }
    స్థితి SQLITE_BLOB: {
      అక్షరం స్థిరరాశి *zBlob = sqlite3_విలువ_blob(p_విలువ);
      i64 nBlob = sqlite3_విలువ_బైట్లు(p_విలువ);
      ప్రతిజ్ఞా( zBlob==sqlite3_విలువ_blob(p_విలువ) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      ఒకవేళ( pStr->accError==0 ){
        అక్షరం *z_పాఠ్యం = pStr->z_పాఠ్యం;
        పూర్ణాంకం i;
        కొరకు(i=0; i<nBlob; i++){
          z_పాఠ్యం[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          z_పాఠ్యం[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        z_పాఠ్యం[(nBlob*2)+2] = '\'';
        z_పాఠ్యం[(nBlob*2)+3] = '\0';
        z_పాఠ్యం[0] = 'X';
        z_పాఠ్యం[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      ఆపు;
    }
    స్థితి SQLITE_పాఠ్యం: {
      స్థిరరాశి గుర్తులేని అక్షరం *zArg = sqlite3_విలువ_పాఠ్యం(p_విలువ);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      ఆపు;
    }
    అప్రమేయ: {
      ప్రతిజ్ఞా( sqlite3_విలువ_type(p_విలువ)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      ఆపు;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
స్థిర పూర్ణాంకం isNHex(స్థిరరాశి అక్షరం *z, పూర్ణాంకం N, u32 *pVal){
  పూర్ణాంకం i;
  u32 v = 0;
  కొరకు(i=0; i<N; i++){
    ఒకవేళ( !sqlite3Isxdigit(z[i]) ) తిరిగి 0;
    v = (v<<4) + sqlite3_Hex_గమ్యం_Int(z[i]);
  }
  *pVal = v;
  తిరిగి 1;
}

/*
** Implementation of the UNISTR() function.
**
** This is intended to be a work-alike of the UNISTR() function in
** PostgreSQL.  Quoting from the PG documentation (PostgreSQL 17 -
** scraped on 2025-02-22):
**
**    Evaluate escaped Unicode characters in the argument. Unicode
**    characters can be specified as \XXXX (4 hexadecimal digits),
**    \+XXXXXX (6 hexadecimal digits), \uXXXX (4 hexadecimal digits),
**    or \UXXXXXXXX (8 hexadecimal digits). To specify a backslash,
**    write two backslashes. All other characters are taken literally.
*/
స్థిర శూన్య unistrFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  అక్షరం *z_బహిర్గతం;
  స్థిరరాశి అక్షరం *z_లోపల;
  పూర్ణాంకం n_లోపల;
  పూర్ణాంకం i, j, n;
  u32 v;

  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER( argc );
  z_లోపల = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  ఒకవేళ( z_లోపల==0 ) తిరిగి;
  n_లోపల = sqlite3_విలువ_బైట్లు(argv[0]);
  z_బహిర్గతం = sqlite3_malloc64(n_లోపల+1);
  ఒకవేళ( z_బహిర్గతం==0 ){
    sqlite3_ఫలితం_error_nomem(context);
    తిరిగి;
  }
  i = j = 0;
  అయితే( i<n_లోపల ){
    స్థిరరాశి అక్షరం *z = strchr(&z_లోపల[i],'\\');
    ఒకవేళ( z==0 ){
      n = n_లోపల - i;
      memmove(&z_బహిర్గతం[j], &z_లోపల[i], n);
      j += n;
      ఆపు;
    }
    n = z - &z_లోపల[i];
    ఒకవేళ( n>0 ){
      memmove(&z_బహిర్గతం[j], &z_లోపల[i], n);
      j += n;
      i += n;
    }
    ఒకవేళ( z_లోపల[i+1]=='\\' ){
      i += 2;
      z_బహిర్గతం[j++] = '\\';
    }లేకపోతే ఒకవేళ( sqlite3Isxdigit(z_లోపల[i+1]) ){
      ఒకవేళ( !isNHex(&z_లోపల[i+1], 4, &v) ) వెళ్ళు unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&z_బహిర్గతం[j], v);
    }లేకపోతే ఒకవేళ( z_లోపల[i+1]=='+' ){
      ఒకవేళ( !isNHex(&z_లోపల[i+2], 6, &v) ) వెళ్ళు unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&z_బహిర్గతం[j], v);
    }లేకపోతే ఒకవేళ( z_లోపల[i+1]=='u' ){
      ఒకవేళ( !isNHex(&z_లోపల[i+2], 4, &v) ) వెళ్ళు unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&z_బహిర్గతం[j], v);
    }లేకపోతే ఒకవేళ( z_లోపల[i+1]=='U' ){
      ఒకవేళ( !isNHex(&z_లోపల[i+2], 8, &v) ) వెళ్ళు unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&z_బహిర్గతం[j], v);
    }లేకపోతే{
      వెళ్ళు unistr_error;
    }
  }
  z_బహిర్గతం[j] = 0;
  sqlite3_ఫలితం_text64(context, z_బహిర్గతం, j, sqlite3_free, SQLITE_UTF8_ZT);
  తిరిగి;

unistr_error:
  sqlite3_free(z_బహిర్గతం);
  sqlite3_ఫలితం_error(context, "invalid Unicode escape", -1);
  తిరిగి;
}


/*
** Implementation of the QUOTE() function. 
**
** The quote(X) function returns the text of an SQL literal which is the
** value of its argument suitable for inclusion into an SQL statement.
** Strings are surrounded by single-quotes with escapes on interior quotes
** as needed. BLOBs are encoded as hexadecimal literals. Strings with
** embedded NUL characters cannot be represented as string literals in SQL
** and hence the returned string literal is truncated prior to the first NUL.
**
** If sqlite3_user_data() is non-zero, then the UNISTR_QUOTE() function is
** implemented instead.  The difference is that UNISTR_QUOTE() uses the
** UNISTR() function to escape control characters.
*/
స్థిర శూన్య quoteFunc(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->a_హద్దు[SQLITE_హద్దు_నిడివి]);
  sqlite3_Quote_విలువ(&str,argv[0],SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(context)));
  sqlite3_ఫలితం_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
స్థిర శూన్య unicodeFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి గుర్తులేని అక్షరం *z = sqlite3_విలువ_పాఠ్యం(argv[0]);
  (శూన్య)argc;
  ఒకవేళ( z && z[0] ) sqlite3_ఫలితం_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
స్థిర శూన్య charFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  గుర్తులేని అక్షరం *z, *z_బహిర్గతం;
  పూర్ణాంకం i;
  z_బహిర్గతం = z = sqlite3_malloc64( argc*4+1 );
  ఒకవేళ( z==0 ){
    sqlite3_ఫలితం_error_nomem(context);
    తిరిగి;
  }
  కొరకు(i=0; i<argc; i++){
    sqlite3_int64 x;
    గుర్తులేని c;
    x = sqlite3_విలువ_int64(argv[i]);
    ఒకవేళ( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (గుర్తులేని)(x & 0x1fffff);
    ఒకవేళ( c<0x00080 ){
      *z_బహిర్గతం++ = (u8)(c&0xFF);
    }లేకపోతే ఒకవేళ( c<0x00800 ){
      *z_బహిర్గతం++ = 0xC0 + (u8)((c>>6)&0x1F);
      *z_బహిర్గతం++ = 0x80 + (u8)(c & 0x3F);
    }లేకపోతే ఒకవేళ( c<0x10000 ){
      *z_బహిర్గతం++ = 0xE0 + (u8)((c>>12)&0x0F);
      *z_బహిర్గతం++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_బహిర్గతం++ = 0x80 + (u8)(c & 0x3F);
    }లేకపోతే{
      *z_బహిర్గతం++ = 0xF0 + (u8)((c>>18) & 0x07);
      *z_బహిర్గతం++ = 0x80 + (u8)((c>>12) & 0x3F);
      *z_బహిర్గతం++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_బహిర్గతం++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *z_బహిర్గతం = 0;
  sqlite3_ఫలితం_text64(context, (అక్షరం*)z, z_బహిర్గతం-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
స్థిర శూన్య hexFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం i, n;
  స్థిరరాశి గుర్తులేని అక్షరం *pBlob;
  అక్షరం *zHex, *z;
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  pBlob = sqlite3_విలువ_blob(argv[0]);
  n = sqlite3_విలువ_బైట్లు(argv[0]);
  ప్రతిజ్ఞా( pBlob==sqlite3_విలువ_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  ఒకవేళ( zHex ){
    కొరకు(i=0; i<n; i++, pBlob++){
      గుర్తులేని అక్షరం c = *pBlob;
      *(z++) = hexdigits[(c>>4)&0xf];
      *(z++) = hexdigits[c&0xf];
    }
    *z = 0;
    sqlite3_ఫలితం_text64(context, zHex, (u64)(z-zHex),
                          sqlite3_free, SQLITE_UTF8_ZT);
  }
}

/*
** Buffer zStr contains nStr bytes of utf-8 encoded text. Return 1 if zStr
** contains character ch, or 0 if it does not.
*/
స్థిర పూర్ణాంకం str_కలిగియుంది_Char(స్థిరరాశి u8 *zStr, పూర్ణాంకం nStr, u32 ch){
  స్థిరరాశి u8 *z_చివర = &zStr[nStr];
  స్థిరరాశి u8 *z = zStr;
  అయితే( z<z_చివర ){
    u32 tst = Utf8Read(z);
    ఒకవేళ( tst==ch ) తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** The unhex() function. This function may be invoked with either one or
** two arguments. In both cases the first argument is interpreted as text
** a text value containing a set of pairs of hexadecimal digits which are
** decoded and returned as a blob.
**
** If there is only a single argument, then it must consist only of an
** even number of hexadecimal digits. Otherwise, return NULL.
**
** Or, if there is a second argument, then any character that appears in
** the second argument is also allowed to appear between pairs of hexadecimal
** digits in the first argument. If any other character appears in the
** first argument, or if one of the allowed characters appears between
** two hexadecimal digits that make up a single byte, NULL is returned.
**
** The following expressions are all true:
**
**     unhex('ABCD')       IS x'ABCD'
**     unhex('AB CD')      IS NULL
**     unhex('AB CD', ' ') IS x'ABCD'
**     unhex('A BCD', ' ') IS NULL
*/
స్థిర శూన్య unhexFunc(
  sqlite3_context *pCtx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి u8 *zPass = (స్థిరరాశి u8*)"";
  పూర్ణాంకం nPass = 0;
  స్థిరరాశి u8 *zHex = sqlite3_విలువ_పాఠ్యం(argv[0]);
  పూర్ణాంకం nHex = sqlite3_విలువ_బైట్లు(argv[0]);
#నిర్వచితమైతే SQLITE_DEBUG
  స్థిరరాశి u8 *z_చివర = zHex ? &zHex[nHex] : 0;
#అయితే_ముగింపు
  u8 *pBlob = 0;
  u8 *p = 0;

  ప్రతిజ్ఞా( argc==1 || argc==2 );
  ఒకవేళ( argc==2 ){
    zPass = sqlite3_విలువ_పాఠ్యం(argv[1]);
    nPass = sqlite3_విలువ_బైట్లు(argv[1]);
  }
  ఒకవేళ( !zHex || !zPass ) తిరిగి;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  ఒకవేళ( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    అయితే( (c = *zHex)!=0x00 ){
      అయితే( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        ప్రతిజ్ఞా( zHex<=z_చివర );
        ఒకవేళ( !str_కలిగియుంది_Char(zPass, nPass, ch) ) వెళ్ళు unhex_null;
        c = *zHex;
        ఒకవేళ( c==0x00 ) వెళ్ళు unhex_పూర్తి;
      }
      zHex++;
      ప్రతిజ్ఞా( *z_చివర==0x00 );
      ప్రతిజ్ఞా( zHex<=z_చివర );
      d = *(zHex++);
      ఒకవేళ( !sqlite3Isxdigit(d) ) వెళ్ళు unhex_null;
      *(p++) = (sqlite3_Hex_గమ్యం_Int(c)<<4) | sqlite3_Hex_గమ్యం_Int(d);
    }
  }

 unhex_పూర్తి:
  sqlite3_ఫలితం_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  తిరిగి;

 unhex_null:
  sqlite3_free(pBlob);
  తిరిగి;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
స్థిర శూన్య zeroblobFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  i64 n;
  పూర్ణాంకం rc;
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  n = sqlite3_విలువ_int64(argv[0]);
  ఒకవేళ( n<0 ) n = 0;
  rc = sqlite3_ఫలితం_zeroblob64(context, n); /* IMP: R-00293-64994 */
  ఒకవేళ( rc ){
    sqlite3_ఫలితం_error_సంకేతం(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
స్థిర శూన్య replaceFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి గుర్తులేని అక్షరం *zStr;        /* The input string A */
  స్థిరరాశి గుర్తులేని అక్షరం *z_నమూనా;    /* The pattern string B */
  స్థిరరాశి గుర్తులేని అక్షరం *zRep;        /* The replacement string C */
  గుర్తులేని అక్షరం *z_బహిర్గతం;              /* The output */
  పూర్ణాంకం nStr;                /* Size of zStr */
  పూర్ణాంకం n_నమూనా;            /* Size of zPattern */
  పూర్ణాంకం nRep;                /* Size of zRep */
  i64 n_బహిర్గతం;                /* Maximum size of zOut */
  పూర్ణాంకం loop_హద్దు;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  గుర్తులేని cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  ప్రతిజ్ఞా( argc==3 );
  వాడని_PARAMETER(argc);
  zStr = sqlite3_విలువ_పాఠ్యం(argv[0]);
  ఒకవేళ( zStr==0 ) తిరిగి;
  nStr = sqlite3_విలువ_బైట్లు(argv[0]);
  ప్రతిజ్ఞా( zStr==sqlite3_విలువ_పాఠ్యం(argv[0]) );  /* No encoding change */
  z_నమూనా = sqlite3_విలువ_పాఠ్యం(argv[1]);
  ఒకవేళ( z_నమూనా==0 ){
    ప్రతిజ్ఞా( sqlite3_విలువ_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    తిరిగి;
  }
  ఒకవేళ( z_నమూనా[0]==0 ){
    ప్రతిజ్ఞా( sqlite3_విలువ_type(argv[1])!=SQLITE_NULL );
    sqlite3_ఫలితం_పాఠ్యం(context, (స్థిరరాశి అక్షరం*)zStr, nStr, SQLITE_TRANSIENT);
    తిరిగి;
  }
  n_నమూనా = sqlite3_విలువ_బైట్లు(argv[1]);
  ప్రతిజ్ఞా( z_నమూనా==sqlite3_విలువ_పాఠ్యం(argv[1]) );  /* No encoding change */
  zRep = sqlite3_విలువ_పాఠ్యం(argv[2]);
  ఒకవేళ( zRep==0 ) తిరిగి;
  nRep = sqlite3_విలువ_బైట్లు(argv[2]);
  ప్రతిజ్ఞా( zRep==sqlite3_విలువ_పాఠ్యం(argv[2]) );
  n_బహిర్గతం = nStr + 1;
  ప్రతిజ్ఞా( n_బహిర్గతం<SQLITE_MAX_నిడివి );
  z_బహిర్గతం = contextMalloc(context, n_బహిర్గతం);
  ఒకవేళ( z_బహిర్గతం==0 ){
    తిరిగి;
  }
  loop_హద్దు = nStr - n_నమూనా; 
  cntExpand = 0;
  కొరకు(i=j=0; i<=loop_హద్దు; i++){
    ఒకవేళ( zStr[i]!=z_నమూనా[0] || memcmp(&zStr[i], z_నమూనా, n_నమూనా) ){
      z_బహిర్గతం[j++] = zStr[i];
    }లేకపోతే{
      ఒకవేళ( nRep>n_నమూనా ){
        n_బహిర్గతం += nRep - n_నమూనా;
        testcase( n_బహిర్గతం-1==db->a_హద్దు[SQLITE_హద్దు_నిడివి] );
        testcase( n_బహిర్గతం-2==db->a_హద్దు[SQLITE_హద్దు_నిడివి] );
        ఒకవేళ( n_బహిర్గతం-1>db->a_హద్దు[SQLITE_హద్దు_నిడివి] ){
          sqlite3_ఫలితం_error_toobig(context);
          sqlite3_free(z_బహిర్గతం);
          తిరిగి;
        }
        cntExpand++;
        ఒకవేళ( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = z_బహిర్గతం;
          z_బహిర్గతం = sqlite3Realloc(z_బహిర్గతం, (పూర్ణాంకం)n_బహిర్గతం + (n_బహిర్గతం - nStr - 1));
          ఒకవేళ( z_బహిర్గతం==0 ){
            sqlite3_ఫలితం_error_nomem(context);
            sqlite3_free(zOld);
            తిరిగి;
          }
        }
      }
      జ్ఞాపకం_నకలు(&z_బహిర్గతం[j], zRep, nRep);
      j += nRep;
      i += n_నమూనా-1;
    }
  }
  ప్రతిజ్ఞా( j+nStr-i+1<=n_బహిర్గతం );
  జ్ఞాపకం_నకలు(&z_బహిర్గతం[j], &zStr[i], nStr-i);
  j += nStr - i;
  ప్రతిజ్ఞా( j<=n_బహిర్గతం );
  z_బహిర్గతం[j] = 0;
  sqlite3_ఫలితం_పాఠ్యం(context, (అక్షరం*)z_బహిర్గతం, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
స్థిర శూన్య trimFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి గుర్తులేని అక్షరం *z_లోపల;         /* Input string */
  స్థిరరాశి గుర్తులేని అక్షరం *z_Char_సమితి;    /* Set of characters to trim */
  గుర్తులేని పూర్ణాంకం n_లోపల;                 /* Number of bytes in input */
  పూర్ణాంకం flags;                        /* 1: trimleft  2: trimright  3: trim */
  పూర్ణాంకం i;                            /* Loop counter */
  గుర్తులేని పూర్ణాంకం *aLen = 0;           /* Length of each character in zCharSet */
  గుర్తులేని అక్షరం **azChar = 0;       /* Individual characters in zCharSet */
  పూర్ణాంకం nChar;                        /* Number of characters in zCharSet */

  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ){
    తిరిగి;
  }
  z_లోపల = sqlite3_విలువ_పాఠ్యం(argv[0]);
  ఒకవేళ( z_లోపల==0 ) తిరిగి;
  n_లోపల = (గుర్తులేని)sqlite3_విలువ_బైట్లు(argv[0]);
  ప్రతిజ్ఞా( z_లోపల==sqlite3_విలువ_పాఠ్యం(argv[0]) );
  ఒకవేళ( argc==1 ){
    స్థిర స్థిరరాశి గుర్తులేని lenOne[] = { 1 };
    స్థిర గుర్తులేని అక్షరం * స్థిరరాశి azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (గుర్తులేని*)lenOne;
    azChar = (గుర్తులేని అక్షరం **)azOne;
    z_Char_సమితి = 0;
  }లేకపోతే ఒకవేళ( (z_Char_సమితి = sqlite3_విలువ_పాఠ్యం(argv[1]))==0 ){
    తిరిగి;
  }లేకపోతే{
    స్థిరరాశి గుర్తులేని అక్షరం *z;
    కొరకు(z=z_Char_సమితి, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    ఒకవేళ( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(పరిమాణం(అక్షరం*)+పరిమాణం(గుర్తులేని)));
      ఒకవేళ( azChar==0 ){
        తిరిగి;
      }
      aLen = (గుర్తులేని*)&azChar[nChar];
      కొరకు(z=z_Char_సమితి, nChar=0; *z; nChar++){
        azChar[nChar] = (గుర్తులేని అక్షరం *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (గుర్తులేని)(z - azChar[nChar]);
      }
    }
  }
  ఒకవేళ( nChar>0 ){
    flags = SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(context));
    ఒకవేళ( flags & 1 ){
      అయితే( n_లోపల>0 ){
        గుర్తులేని పూర్ణాంకం len = 0;
        కొరకు(i=0; i<nChar; i++){
          len = aLen[i];
          ఒకవేళ( len<=n_లోపల && memcmp(z_లోపల, azChar[i], len)==0 ) ఆపు;
        }
        ఒకవేళ( i>=nChar ) ఆపు;
        z_లోపల += len;
        n_లోపల -= len;
      }
    }
    ఒకవేళ( flags & 2 ){
      అయితే( n_లోపల>0 ){
        గుర్తులేని పూర్ణాంకం len = 0;
        కొరకు(i=0; i<nChar; i++){
          len = aLen[i];
          ఒకవేళ( len<=n_లోపల && memcmp(&z_లోపల[n_లోపల-len],azChar[i],len)==0 ) ఆపు;
        }
        ఒకవేళ( i>=nChar ) ఆపు;
        n_లోపల -= len;
      }
    }
    ఒకవేళ( z_Char_సమితి ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_ఫలితం_పాఠ్యం(context, (అక్షరం*)z_లోపల, n_లోపల, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
స్థిర శూన్య concatFuncCore(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv,
  పూర్ణాంకం nSep,
  స్థిరరాశి అక్షరం *zSep
){
  i64 j, n = 0;
  పూర్ణాంకం i;
  పూర్ణాంకం bNotNull = 0;   /* True after at least NOT NULL argument seen */
  అక్షరం *z;
  కొరకు(i=0; i<argc; i++){
    n += sqlite3_విలువ_బైట్లు(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  ఒకవేళ( z==0 ){
    sqlite3_ఫలితం_error_nomem(context);
    తిరిగి;
  }
  j = 0;
  కొరకు(i=0; i<argc; i++){
    ఒకవేళ( sqlite3_విలువ_type(argv[i])!=SQLITE_NULL ){
      పూర్ణాంకం k = sqlite3_విలువ_బైట్లు(argv[i]);
      స్థిరరాశి అక్షరం *v = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
      ఒకవేళ( v!=0 ){
        ఒకవేళ( bNotNull && nSep>0 ){
          జ్ఞాపకం_నకలు(&z[j], zSep, nSep);
          j += nSep;
        }
        జ్ఞాపకం_నకలు(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;
  ప్రతిజ్ఞా( j<=n );
  sqlite3_ఫలితం_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
స్థిర శూన్య concatFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  concatFuncCore(context, argc, argv, 0, "");
}

/*
** The CONCAT_WS(separator, ...) function.
**
** Generate a string that is the concatenation of 2nd through the Nth
** argument.  Use the first argument (which must be non-NULL) as the
** separator.
*/
స్థిర శూన్య concatwsFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం nSep = sqlite3_విలువ_బైట్లు(argv[0]);
  స్థిరరాశి అక్షరం *zSep = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  ఒకవేళ( zSep==0 ) తిరిగి;
  concatFuncCore(context, argc-1, argv+1, nSep, zSep);
}


#నిర్వచితమైతే SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
/*
** The "unknown" function is automatically substituted in place of
** any unrecognized function name when doing an EXPLAIN or EXPLAIN QUERY PLAN
** when the SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION compile-time option is used.
** When the "sqlite3" command-line shell is built using this functionality,
** that allows an EXPLAIN or EXPLAIN QUERY PLAN for complex queries
** involving application-defined functions to be examined in a generic
** sqlite3 shell.
*/
స్థిర శూన్య unknownFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  /* no-op */
  (శూన్య)context;
  (శూన్య)argc;
  (శూన్య)argv;
}
#అయితే_ముగింపు /*SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION*/


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#నిర్వచితమైతే SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X.
*/
స్థిర శూన్య soundexFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  అక్షరం z_ఫలితం[8];
  స్థిరరాశి u8 *z_లోపల;
  పూర్ణాంకం i, j;
  స్థిర స్థిరరాశి గుర్తులేని అక్షరం i_సంకేతం[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  ప్రతిజ్ఞా( argc==1 );
  z_లోపల = (u8*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  ఒకవేళ( z_లోపల==0 ) z_లోపల = (u8*)"";
  కొరకు(i=0; z_లోపల[i] && !sqlite3Isalpha(z_లోపల[i]); i++){}
  ఒకవేళ( z_లోపల[i] ){
    u8 prevcode = i_సంకేతం[z_లోపల[i]&0x7f];
    z_ఫలితం[0] = sqlite3Toupper(z_లోపల[i]);
    కొరకు(j=1; j<4 && z_లోపల[i]; i++){
      పూర్ణాంకం సంకేతం_2 = i_సంకేతం[z_లోపల[i]&0x7f];
      ఒకవేళ( సంకేతం_2>0 ){
        ఒకవేళ( సంకేతం_2!=prevcode ){
          prevcode = సంకేతం_2;
          z_ఫలితం[j++] = సంకేతం_2 + '0';
        }
      }లేకపోతే{
        prevcode = 0;
      }
    }
    అయితే( j<4 ){
      z_ఫలితం[j++] = '0';
    }
    z_ఫలితం[j] = 0;
    sqlite3_ఫలితం_పాఠ్యం(context, z_ఫలితం, 4, SQLITE_TRANSIENT);
  }లేకపోతే{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_ఫలితం_పాఠ్యం(context, "?000", 4, SQLITE_STATIC);
  }
}
#అయితే_ముగింపు /* SQLITE_SOUNDEX */

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
స్థిర శూన్య oomFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
స్థిర శూన్య ఎక్కించు_Ext(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  స్థిరరాశి అక్షరం *zFile = (స్థిరరాశి అక్షరం *)sqlite3_విలువ_పాఠ్యం(argv[0]);
  స్థిరరాశి అక్షరం *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  అక్షరం *z_Err_సందేశం = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  ఒకవేళ( (db->flags & SQLITE_ఎక్కించు_Ext_Func)==0 ){
    sqlite3_ఫలితం_error(context, "not authorized", -1);
    తిరిగి;
  }

  ఒకవేళ( argc==2 ){
    zProc = (స్థిరరాశి అక్షరం *)sqlite3_విలువ_పాఠ్యం(argv[1]);
  }లేకపోతే{
    zProc = 0;
  }
  ఒకవేళ( zFile && sqlite3_ఎక్కించు_extension(db, zFile, zProc, &z_Err_సందేశం) ){
    sqlite3_ఫలితం_error(context, z_Err_సందేశం, -1);
    sqlite3_free(z_Err_సందేశం);
  }
}
#అయితే_ముగింపు


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
రకనిర్వచనం నిర్మాణం కూడిక_Ctx కూడిక_Ctx;
నిర్మాణం కూడిక_Ctx {
  ద్విగుణ r_కూడిక;      /* Running sum as as a double */
  ద్విగుణ rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
  i64 i_కూడిక;         /* Running sum as a signed integer */
  i64 cnt;          /* Number of elements summed */
  u8 approx;        /* True if any non-integer value was input to the sum */
  u8 ovrfl;         /* Integer overflow seen */
};

/*
** Do one step of the Kahan-Babushka-Neumaier summation.
**
** https://en.wikipedia.org/wiki/Kahan_summation_algorithm
**
** Variables are marked "volatile" to defeat c89 x86 floating point
** optimizations can mess up this algorithm.
*/
స్థిర శూన్య kahan_Babuska_Neumaier_అడుగు(
  అస్థిర కూడిక_Ctx *p_కూడిక,
  అస్థిర ద్విగుణ r
){
  అస్థిర ద్విగుణ s = p_కూడిక->r_కూడిక;
  అస్థిర ద్విగుణ t = s + r;
  ఒకవేళ( fabs(s) > fabs(r) ){
    p_కూడిక->rErr += (s - t) + r;
  }లేకపోతే{
    p_కూడిక->rErr += (r - t) + s;
  }
  p_కూడిక->r_కూడిక = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
స్థిర శూన్య kahan_Babuska_Neumaier_అడుగు_Int64(అస్థిర కూడిక_Ctx *p_కూడిక, i64 iVal){
  ఒకవేళ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahan_Babuska_Neumaier_అడుగు(p_కూడిక, iBig);
    kahan_Babuska_Neumaier_అడుగు(p_కూడిక, iSm);
  }లేకపోతే{
    kahan_Babuska_Neumaier_అడుగు(p_కూడిక, (ద్విగుణ)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
స్థిర శూన్య kahanBabuskaNeumaierInit(
  అస్థిర కూడిక_Ctx *p,
  i64 iVal
){
  ఒకవేళ( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->r_కూడిక = (ద్విగుణ)(iVal - iSm);
    p->rErr = (ద్విగుణ)iSm;
  }లేకపోతే{
    p->r_కూడిక = (ద్విగుణ)iVal;
    p->rErr = 0.0;
  }
}

/*
** Routines used to compute the sum, average, and total.
**
** The SUM() function follows the (broken) SQL standard which means
** that it returns NULL if it sums over no inputs.  TOTAL returns
** 0.0 in that case.  In addition, TOTAL always returns a float where
** SUM might return an integer if it never encounters a floating point
** value.  TOTAL never fails, but SUM might throw an exception if
** it overflows an integer.
*/
స్థిర శూన్య కూడిక_అడుగు(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  కూడిక_Ctx *p;
  పూర్ణాంకం type;
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, పరిమాణం(*p));
  type = sqlite3_విలువ_numeric_type(argv[0]);
  ఒకవేళ( p && type!=SQLITE_NULL ){
    p->cnt++;
    ఒకవేళ( p->approx==0 ){
      ఒకవేళ( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->i_కూడిక);
        p->approx = 1;
        kahan_Babuska_Neumaier_అడుగు(p, sqlite3_విలువ_double(argv[0]));
      }లేకపోతే{
        i64 x = p->i_కూడిక;
        ఒకవేళ( sqlite3_కూడు_Int64(&x, sqlite3_విలువ_int64(argv[0]))==0 ){
          p->i_కూడిక = x;
        }లేకపోతే{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->i_కూడిక);
          p->approx = 1;
          kahan_Babuska_Neumaier_అడుగు_Int64(p, sqlite3_విలువ_int64(argv[0]));
        }
      }
    }లేకపోతే{
      ఒకవేళ( type==SQLITE_INTEGER ){
        kahan_Babuska_Neumaier_అడుగు_Int64(p, sqlite3_విలువ_int64(argv[0]));
      }లేకపోతే{
        p->ovrfl = 0;
        kahan_Babuska_Neumaier_అడుగు(p, sqlite3_విలువ_double(argv[0]));
      }
    }
  }
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
స్థిర శూన్య కూడిక_Inverse(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ**argv){
  కూడిక_Ctx *p;
  పూర్ణాంకం type;
  ప్రతిజ్ఞా( argc==1 );
  వాడని_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, పరిమాణం(*p));
  type = sqlite3_విలువ_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  ఒకవేళ( ALWAYS(p) && type!=SQLITE_NULL ){
    ప్రతిజ్ఞా( p->cnt>0 );
    p->cnt--;
    ఒకవేళ( !p->approx ){
      i64 x = p->i_కూడిక;
      ఒకవేళ( sqlite3SubInt64(&x, sqlite3_విలువ_int64(argv[0]))==0 ){
        p->i_కూడిక = x;
        తిరిగి;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->i_కూడిక);
    }
    ఒకవేళ( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_విలువ_int64(argv[0]);
      ఒకవేళ( iVal!=అతిచిన్న_INT64 ){
        kahan_Babuska_Neumaier_అడుగు_Int64(p, -iVal);
      }లేకపోతే{
        kahan_Babuska_Neumaier_అడుగు_Int64(p, అతిపెద్ద_INT64);
        kahan_Babuska_Neumaier_అడుగు_Int64(p, 1);
      }       
    }లేకపోతే{
      kahan_Babuska_Neumaier_అడుగు(p, -sqlite3_విలువ_double(argv[0]));
    }
  }
}
#లేకపోతే
# define కూడిక_Inverse 0
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */
స్థిర శూన్య కూడిక_Finalize(sqlite3_context *context){
  కూడిక_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  ఒకవేళ( p && p->cnt>0 ){
    ఒకవేళ( p->approx ){
      ఒకవేళ( p->ovrfl ){
        sqlite3_ఫలితం_error(context,"integer overflow",-1);
      }లేకపోతే ఒకవేళ( !sqlite3_Is_అధికప్రవాహం(p->rErr) ){
        sqlite3_ఫలితం_double(context, p->r_కూడిక+p->rErr);
      }లేకపోతే{
        sqlite3_ఫలితం_double(context, p->r_కూడిక);
      }
    }లేకపోతే{
      sqlite3_ఫలితం_int64(context, p->i_కూడిక);
    }
  }
}
స్థిర శూన్య avgFinalize(sqlite3_context *context){
  కూడిక_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  ఒకవేళ( p && p->cnt>0 ){
    ద్విగుణ r;
    ఒకవేళ( p->approx ){
      r = p->r_కూడిక;
      ఒకవేళ( !sqlite3_Is_అధికప్రవాహం(p->rErr) ) r += p->rErr;
    }లేకపోతే{
      r = (ద్విగుణ)(p->i_కూడిక);
    }
    sqlite3_ఫలితం_double(context, r/(ద్విగుణ)p->cnt);
  }
}
స్థిర శూన్య మొత్తం_Finalize(sqlite3_context *context){
  కూడిక_Ctx *p;
  ద్విగుణ r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  ఒకవేళ( p ){
    ఒకవేళ( p->approx ){
      r = p->r_కూడిక;
      ఒకవేళ( !sqlite3_Is_అధికప్రవాహం(p->rErr) ) r += p->rErr;
    }లేకపోతే{
      r = (ద్విగుణ)(p->i_కూడిక);
    }
  }
  sqlite3_ఫలితం_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
రకనిర్వచనం నిర్మాణం లెక్క_Ctx లెక్క_Ctx;
నిర్మాణం లెక్క_Ctx {
  i64 n;
#నిర్వచితమైతే SQLITE_DEBUG
  పూర్ణాంకం bInverse;                   /* True if xInverse() ever called */
#అయితే_ముగింపు
};

/*
** Routines to implement the count() aggregate function.
*/
స్థిర శూన్య లెక్క_అడుగు(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  లెక్క_Ctx *p;
  p = sqlite3_aggregate_context(context, పరిమాణం(*p));
  ఒకవేళ( (argc==0 || SQLITE_NULL!=sqlite3_విలువ_type(argv[0])) && p ){
    p->n++;
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  ప్రతిజ్ఞా( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_లెక్క(context) );
#అయితే_ముగింపు
}  
స్థిర శూన్య లెక్క_Finalize(sqlite3_context *context){
  లెక్క_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_ఫలితం_int64(context, p ? p->n : 0);
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
స్థిర శూన్య లెక్క_Inverse(sqlite3_context *ctx, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  లెక్క_Ctx *p;
  p = sqlite3_aggregate_context(ctx, పరిమాణం(*p));
  /* p is always non-NULL since countStep() will have been called first */
  ఒకవేళ( (argc==0 || SQLITE_NULL!=sqlite3_విలువ_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#నిర్వచితమైతే SQLITE_DEBUG
    p->bInverse = 1;
#అయితే_ముగింపు
  }
}  
#లేకపోతే
# define లెక్క_Inverse 0
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
స్థిర శూన్య minmax_అడుగు(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *p_ఉత్తమ;
  వాడని_PARAMETER(Not_వాడిన);

  p_ఉత్తమ = (Mem *)sqlite3_aggregate_context(context, పరిమాణం(*p_ఉత్తమ));
  ఒకవేళ( !p_ఉత్తమ ) తిరిగి;

  ఒకవేళ( sqlite3_విలువ_type(pArg)==SQLITE_NULL ){
    ఒకవేళ( p_ఉత్తమ->flags ) sqlite3_Skip_సంచాయకం_ఎక్కించు(context);
  }లేకపోతే ఒకవేళ( p_ఉత్తమ->flags ){
    పూర్ణాంకం max;
    పూర్ణాంకం cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);
    /* This step function is used for both the min() and max() aggregates,
    ** the only difference between the two being that the sense of the
    ** comparison is inverted. For the max() aggregate, the
    ** sqlite3_user_data() function returns (void *)-1. For min() it
    ** returns (void *)db, where db is the sqlite3* database pointer.
    ** Therefore the next statement sets variable 'max' to 1 for the max()
    ** aggregate, or 0 for min().
    */
    max = sqlite3_user_దత్తాంశం(context)!=0;
    cmp = sqlite3MemCompare(p_ఉత్తమ, pArg, pColl);
    ఒకవేళ( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(p_ఉత్తమ, pArg);
    }లేకపోతే{
      sqlite3_Skip_సంచాయకం_ఎక్కించు(context);
    }
  }లేకపోతే{
    p_ఉత్తమ->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(p_ఉత్తమ, pArg);
  }
}
స్థిర శూన్య min_Max_విలువ_Finalize(sqlite3_context *context, పూర్ణాంకం b_విలువ){
  sqlite3_విలువ *pRes;
  pRes = (sqlite3_విలువ *)sqlite3_aggregate_context(context, 0);
  ఒకవేళ( pRes ){
    ఒకవేళ( pRes->flags ){
      sqlite3_ఫలితం_విలువ(context, pRes);
    }
    ఒకవేళ( b_విలువ==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
స్థిర శూన్య min_Max_విలువ(sqlite3_context *context){
  min_Max_విలువ_Finalize(context, 1);
}
#లేకపోతే
# define min_Max_విలువ 0
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */
స్థిర శూన్య minMaxFinalize(sqlite3_context *context){
  min_Max_విలువ_Finalize(context, 0);
}

/*
** group_concat(EXPR, ?SEPARATOR?)
** string_agg(EXPR, SEPARATOR)
**
** Content is accumulated in GroupConcatCtx.str with the SEPARATOR
** coming before the EXPR value, except for the first entry which
** omits the SEPARATOR.
**
** It is tragic that the SEPARATOR goes before the EXPR string.  The
** groupConcatInverse() implementation would have been easier if the
** SEPARATOR were appended after EXPR.  And the order is undocumented,
** so we could change it, in theory.  But the old behavior has been
** around for so long that we dare not, for fear of breaking something.
*/
రకనిర్వచనం నిర్మాణం {
  StrAccum str;          /* The accumulated concatenation */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  పూర్ణాంకం nAccum;            /* Number of strings presently concatenated */
  పూర్ణాంకం n_మొదటి_Sep_నిడివి;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  పూర్ణాంకం *pnSepLengths;
#అయితే_ముగింపు
} సమూహం_Concat_Ctx;

స్థిర శూన్య సమూహం_Concat_అడుగు(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  స్థిరరాశి అక్షరం *zVal;
  సమూహం_Concat_Ctx *pGCC;
  స్థిరరాశి అక్షరం *zSep;
  పూర్ణాంకం nVal, nSep;
  ప్రతిజ్ఞా( argc==1 || argc==2 );
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ) తిరిగి;
  pGCC = (సమూహం_Concat_Ctx*)sqlite3_aggregate_context(context, పరిమాణం(*pGCC));
  ఒకవేళ( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    పూర్ణాంకం మొదటి_పదం = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->a_హద్దు[SQLITE_హద్దు_నిడివి];
    ఒకవేళ( argc==1 ){
      ఒకవేళ( !మొదటి_పదం ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      లేకపోతే{
        pGCC->n_మొదటి_Sep_నిడివి = 1;
      }
#అయితే_ముగింపు
    }లేకపోతే ఒకవేళ( !మొదటి_పదం ){
      zSep = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);
      nSep = sqlite3_విలువ_బైట్లు(argv[1]);
      ఒకవేళ( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      లేకపోతే{
        nSep = 0;
      }
      ఒకవేళ( nSep != pGCC->n_మొదటి_Sep_నిడివి || pGCC->pnSepLengths != 0 ){
        పూర్ణాంకం *pnsl = pGCC->pnSepLengths;
        ఒకవేళ( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (పూర్ణాంకం*)sqlite3_malloc64((pGCC->nAccum+1) * పరిమాణం(పూర్ణాంకం));
          ఒకవేళ( pnsl!=0 ){
            పూర్ణాంకం i = 0, nA = pGCC->nAccum-1;
            అయితే( i<nA ) pnsl[i++] = pGCC->n_మొదటి_Sep_నిడివి;
          }
        }లేకపోతే{
          pnsl = (పూర్ణాంకం*)sqlite3_realloc64(pnsl, pGCC->nAccum * పరిమాణం(పూర్ణాంకం));
        }
        ఒకవేళ( pnsl!=0 ){
          ఒకవేళ( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }లేకపోతే{
          sqlite3_Str_Accum_సమితి_Error(&pGCC->str, SQLITE_NOMEM);
        }
      }
#అయితే_ముగింపు
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    లేకపోతే{
      pGCC->n_మొదటి_Sep_నిడివి = sqlite3_విలువ_బైట్లు(argv[1]);
    }
    pGCC->nAccum += 1;
#అయితే_ముగింపు
    zVal = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    nVal = sqlite3_విలువ_బైట్లు(argv[0]);
    ఒకవేళ( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
స్థిర శూన్య సమూహం_Concat_Inverse(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  సమూహం_Concat_Ctx *pGCC;
  ప్రతిజ్ఞా( argc==1 || argc==2 );
  (శూన్య)argc;  /* Suppress unused parameter warning */
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ) తిరిగి;
  pGCC = (సమూహం_Concat_Ctx*)sqlite3_aggregate_context(context, పరిమాణం(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  ఒకవేళ( ALWAYS(pGCC) ){
    పూర్ణాంకం nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (శూన్య)sqlite3_విలువ_పాఠ్యం(argv[0]);
    nVS = sqlite3_విలువ_బైట్లు(argv[0]);
    pGCC->nAccum -= 1;
    ఒకవేళ( pGCC->pnSepLengths!=0 ){
      ప్రతిజ్ఞా(pGCC->nAccum >= 0);
      ఒకవేళ( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*పరిమాణం(పూర్ణాంకం));
      }
    }లేకపోతే{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->n_మొదటి_Sep_నిడివి;
    }
    ఒకవేళ( nVS>=(పూర్ణాంకం)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }లేకపోతే{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.z_పాఠ్యం, &pGCC->str.z_పాఠ్యం[nVS], pGCC->str.nChar);
    }
    ఒకవేళ( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#లేకపోతే
# define సమూహం_Concat_Inverse 0
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */
స్థిర శూన్య సమూహం_Concat_Finalize(sqlite3_context *context){
  సమూహం_Concat_Ctx *pGCC
    = (సమూహం_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  ఒకవేళ( pGCC ){
    sqlite3_ఫలితం_str(context, &pGCC->str, SQLITE_XFER);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#అయితే_ముగింపు
  }
}
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
స్థిర శూన్య సమూహం_Concat_విలువ(sqlite3_context *context){
  సమూహం_Concat_Ctx *pGCC
    = (సమూహం_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  ఒకవేళ( pGCC && pGCC->nAccum>0 ){
    sqlite3_ఫలితం_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#లేకపోతే
# define సమూహం_Concat_విలువ 0
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
శూన్య sqlite3_రిజిస్టరు_చొప్పున_Connection_Builtin_Functions(sqlite3 *db){
  పూర్ణాంకం rc = sqlite3_overload_function(db, "MATCH", 2);
  ప్రతిజ్ఞా( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  ఒకవేళ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
శూన్య sqlite3_రిజిస్టరు_Like_Functions(sqlite3 *db, పూర్ణాంకం caseSensitive){
  FuncDef *pDef;
  నిర్మాణం compareInfo *pInfo;
  పూర్ణాంకం flags;
  పూర్ణాంకం nArg;
  ఒకవేళ( caseSensitive ){
    pInfo = (నిర్మాణం compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }లేకపోతే{
    pInfo = (నిర్మాణం compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  కొరకు(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3_కనుగొను_Function(db, "like", nArg, SQLITE_UTF8, 0);
    ప్రతిజ్ఞా( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
                       ** because the "like" SQL-function already exists */
    pDef->funcFlags |= flags;
    pDef->funcFlags &= ~SQLITE_FUNC_UNSAFE;
  }
}

/*
** pExpr points to an expression which implements a function.  If
** it is appropriate to apply the LIKE optimization to that function
** then set aWc[0] through aWc[2] to the wildcard characters and the
** escape character and then return TRUE.  If the function is not a
** LIKE-style function then return FALSE.
**
** The expression "a LIKE b ESCAPE c" is only considered a valid LIKE
** operator if c is a string literal that is exactly one byte in length.
** That one byte is stored in aWc[3].  aWc[3] is set to zero if there is
** no ESCAPE clause.
**
** *pIsNocase is set to true if uppercase and lowercase are equivalent for
** the function (default for LIKE).  If the function makes the distinction
** between uppercase and lowercase (as does GLOB) then *pIsNocase is set to
** false.
*/
పూర్ణాంకం sqlite3IsLikeFunction(sqlite3 *db, సమాసం *p_సమాసం, పూర్ణాంకం *pIsNocase, అక్షరం *aWc){
  FuncDef *pDef;
  పూర్ణాంకం n_సమాసం;
  ప్రతిజ్ఞా( p_సమాసం!=0 );
  ప్రతిజ్ఞా( p_సమాసం->op==TK_FUNCTION );
  ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
  ఒకవేళ( !p_సమాసం->x.pList ){
    తిరిగి 0;
  }
  n_సమాసం = p_సమాసం->x.pList->n_సమాసం;
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
  pDef = sqlite3_కనుగొను_Function(db, p_సమాసం->u.zToken, n_సమాసం, SQLITE_UTF8, 0);
#నిర్వచితమైతే SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  ఒకవేళ( pDef==0 ) తిరిగి 0;
#అయితే_ముగింపు
  ఒకవేళ( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    తిరిగి 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  జ్ఞాపకం_నకలు(aWc, pDef->p_User_దత్తాంశం, 3);
  ప్రతిజ్ఞా( (అక్షరం*)&likeInfoAlt == (అక్షరం*)&likeInfoAlt.సరిపోలిక_All );
  ప్రతిజ్ఞా( &((అక్షరం*)&likeInfoAlt)[1] == (అక్షరం*)&likeInfoAlt.సరిపోలిక_One );
  ప్రతిజ్ఞా( &((అక్షరం*)&likeInfoAlt)[2] == (అక్షరం*)&likeInfoAlt.సరిపోలిక_సమితి );

  ఒకవేళ( n_సమాసం<3 ){
    aWc[3] = 0;
  }లేకపోతే{
    సమాసం *pEscape = p_సమాసం->x.pList->a[2].p_సమాసం;
    అక్షరం *zEscape;
    ఒకవేళ( pEscape->op!=TK_STRING ) తిరిగి 0;
    ప్రతిజ్ఞా( !సమాసం_Has_Property(pEscape, EP_Int_విలువ) );
    zEscape = pEscape->u.zToken;
    ఒకవేళ( zEscape[0]==0 || zEscape[1]!=0 ) తిరిగి 0;
    ఒకవేళ( zEscape[0]==aWc[0] ) తిరిగి 0;
    ఒకవేళ( zEscape[0]==aWc[1] ) తిరిగి 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  తిరిగి 1;
}

/* Mathematical Constants */
#నిర్వచితం_కాకపోతే M_PI
# define M_PI   3.141592653589793238462643383279502884
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే M_LN10
# define M_LN10 2.302585092994045684017991454684364208
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే M_LN2
# define M_LN2  0.693147180559945309417232121458176568
#అయితే_ముగింపు


/* Extra math functions that require linking with -lm
*/
#నిర్వచితమైతే SQLITE_ENABLE_MATH_FUNCTIONS
/*
** Implementation SQL functions:
**
**   ceil(X)
**   ceiling(X)
**   floor(X)
**
** The sqlite3_user_data() pointer is a pointer to the libm implementation
** of the underlying C function.
*/
స్థిర శూన్య ceilingFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  ప్రతిజ్ఞా( argc==1 );
  ఎంపిక( sqlite3_విలువ_numeric_type(argv[0]) ){
    స్థితి SQLITE_INTEGER: {
       sqlite3_ఫలితం_int64(context, sqlite3_విలువ_int64(argv[0]));
       ఆపు;
    }
    స్థితి SQLITE_FLOAT: {
       ద్విగుణ (*x)(ద్విగుణ) = (ద్విగుణ(*)(ద్విగుణ))sqlite3_user_దత్తాంశం(context);
       sqlite3_ఫలితం_double(context, x(sqlite3_విలువ_double(argv[0])));
       ఆపు;
    }
    అప్రమేయ: {
       ఆపు;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
స్థిర ద్విగుణ xCeil(ద్విగుణ x){ తిరిగి ceil(x); }
స్థిర ద్విగుణ xFloor(ద్విగుణ x){ తిరిగి floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#ఒకవేళ defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*సంవర్గమానం(X))
#అయితే_ముగింపు
#ఒకవేళ defined(HAVE_LOG2) && HAVE_LOG2==0
# define log2(X) (1.442695040888963456*సంవర్గమానం(X))
#అయితే_ముగింపు


/*
** Implementation of SQL functions:
**
**   ln(X)       - natural logarithm
**   log(X)      - log X base 10
**   log10(X)    - log X base 10
**   log(B,X)    - log X base B
*/
స్థిర శూన్య logFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  ద్విగుణ x, b, ans;
  ప్రతిజ్ఞా( argc==1 || argc==2 );
  ఎంపిక( sqlite3_విలువ_numeric_type(argv[0]) ){
    స్థితి SQLITE_INTEGER:
    స్థితి SQLITE_FLOAT:
      x = sqlite3_విలువ_double(argv[0]);
      ఒకవేళ( x<=0.0 ) తిరిగి;
      ఆపు;
    అప్రమేయ:
      తిరిగి;
  }
  ఒకవేళ( argc==2 ){
    ఎంపిక( sqlite3_విలువ_numeric_type(argv[1]) ){
      స్థితి SQLITE_INTEGER:
      స్థితి SQLITE_FLOAT:
        b = సంవర్గమానం(x);
        ఒకవేళ( b<=0.0 ) తిరిగి;
        x = sqlite3_విలువ_double(argv[1]);
        ఒకవేళ( x<=0.0 ) తిరిగి;
        ఆపు;
     అప్రమేయ:
        తిరిగి;
    }
    ans = సంవర్గమానం(x)/b;
  }లేకపోతే{
    ఎంపిక( SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(context)) ){
      స్థితి 1:
        ans = log10(x);
        ఆపు;
      స్థితి 2:
        ans = log2(x);
        ఆపు;
      అప్రమేయ:
        ans = సంవర్గమానం(x);
        ఆపు;
    }
  }
  sqlite3_ఫలితం_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
స్థిర ద్విగుణ deg_గమ్యం_Rad(ద్విగుణ x){ తిరిగి x*(M_PI/180.0); }
స్థిర ద్విగుణ rad_గమ్యం_Deg(ద్విగుణ x){ తిరిగి x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
స్థిర శూన్య math1Func(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం type0;
  ద్విగుణ v0, ans;
  ద్విగుణ (*x)(ద్విగుణ);
  ప్రతిజ్ఞా( argc==1 );
  type0 = sqlite3_విలువ_numeric_type(argv[0]);
  ఒకవేళ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) తిరిగి;
  v0 = sqlite3_విలువ_double(argv[0]);
  x = (ద్విగుణ(*)(ద్విగుణ))sqlite3_user_దత్తాంశం(context);
  ans = x(v0);
  sqlite3_ఫలితం_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
స్థిర శూన్య math2Func(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం type0, type1;
  ద్విగుణ v0, v1, ans;
  ద్విగుణ (*x)(ద్విగుణ,ద్విగుణ);
  ప్రతిజ్ఞా( argc==2 );
  type0 = sqlite3_విలువ_numeric_type(argv[0]);
  ఒకవేళ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) తిరిగి;
  type1 = sqlite3_విలువ_numeric_type(argv[1]);
  ఒకవేళ( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) తిరిగి;
  v0 = sqlite3_విలువ_double(argv[0]);
  v1 = sqlite3_విలువ_double(argv[1]);
  x = (ద్విగుణ(*)(ద్విగుణ,ద్విగుణ))sqlite3_user_దత్తాంశం(context);
  ans = x(v0, v1);
  sqlite3_ఫలితం_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
స్థిర శూన్య piFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  ప్రతిజ్ఞా( argc==0 );
  (శూన్య)argv;
  sqlite3_ఫలితం_double(context, M_PI);
}

#అయితే_ముగింపు /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
స్థిర శూన్య signFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం type0;
  ద్విగుణ x;
  వాడని_PARAMETER(argc);
  ప్రతిజ్ఞా( argc==1 );
  type0 = sqlite3_విలువ_numeric_type(argv[0]);
  ఒకవేళ( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) తిరిగి;
  x = sqlite3_విలువ_double(argv[0]);
  sqlite3_ఫలితం_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#ఒకవేళ defined(SQLITE_ENABLE_PERCENTILE)
/***********************************************************************
** This section implements the percentile(Y,P) SQL function and similar.
** Requirements:
**
**   (1)  The percentile(Y,P) function is an aggregate function taking
**        exactly two arguments.
**
**   (2)  If the P argument to percentile(Y,P) is not the same for every
**        row in the aggregate then an error is thrown.  The word "same"
**        in the previous sentence means that the value differ by less
**        than 0.001.
**
**   (3)  If the P argument to percentile(Y,P) evaluates to anything other
**        than a number in the range of 0.0 to 100.0 inclusive then an
**        error is thrown.
**
**   (4)  If any Y argument to percentile(Y,P) evaluates to a value that
**        is not NULL and is not numeric then an error is thrown.
**
**   (5)  If any Y argument to percentile(Y,P) evaluates to plus or minus
**        infinity then an error is thrown.  (SQLite always interprets NaN
**        values as NULL.)
**
**   (6)  Both Y and P in percentile(Y,P) can be arbitrary expressions,
**        including CASE WHEN expressions.
**
**   (7)  The percentile(Y,P) aggregate is able to handle inputs of at least
**        one million (1,000,000) rows.
**
**   (8)  If there are no non-NULL values for Y, then percentile(Y,P)
**        returns NULL.
**
**   (9)  If there is exactly one non-NULL value for Y, the percentile(Y,P)
**        returns the one Y value.
**
**  (10)  If there N non-NULL values of Y where N is two or more and
**        the Y values are ordered from least to greatest and a graph is
**        drawn from 0 to N-1 such that the height of the graph at J is
**        the J-th Y value and such that straight lines are drawn between
**        adjacent Y values, then the percentile(Y,P) function returns
**        the height of the graph at P*(N-1)/100.
**
**  (11)  The percentile(Y,P) function always returns either a floating
**        point number or NULL.
**
**  (12)  The percentile(Y,P) is implemented as a single C99 source-code
**        file that compiles into a shared-library or DLL that can be loaded
**        into SQLite using the sqlite3_load_extension() interface.
**
**  (13)  A separate median(Y) function is the equivalent percentile(Y,50).
**
**  (14)  A separate percentile_cont(Y,P) function is equivalent to
**        percentile(Y,P/100.0).  In other words, the fraction value in
**        the second argument is in the range of 0 to 1 instead of 0 to 100.
**
**  (15)  A separate percentile_disc(Y,P) function is like
**        percentile_cont(Y,P) except that instead of returning the weighted
**        average of the nearest two input values, it returns the next lower
**        value.  So the percentile_disc(Y,P) will always return a value
**        that was one of the inputs.
**
**  (16)  All of median(), percentile(Y,P), percentile_cont(Y,P) and
**        percentile_disc(Y,P) can be used as window functions.
**
** Differences from standard SQL:
**
**  *  The percentile_cont(X,P) function is equivalent to the following in
**     standard SQL:
**
**         (percentile_cont(P) WITHIN GROUP (ORDER BY X))
**
**     The SQLite syntax is much more compact.  The standard SQL syntax
**     is also supported if SQLite is compiled with the
**     -DSQLITE_ENABLE_ORDERED_SET_AGGREGATES option.
**
**  *  No median(X) function exists in the SQL standard.  App developers
**     are expected to write "percentile_cont(0.5)WITHIN GROUP(ORDER BY X)".
**
**  *  No percentile(Y,P) function exists in the SQL standard.  Instead of
**     percential(Y,P), developers must write this:
**     "percentile_cont(P/100.0) WITHIN GROUP (ORDER BY Y)".  Note that
**     the fraction parameter to percentile() goes from 0 to 100 whereas
**     the fraction parameter in SQL standard percentile_cont() goes from
**     0 to 1.
**
** Implementation notes as of 2024-08-31:
**
**  *  The regular aggregate-function versions of these routines work
**     by accumulating all values in an array of doubles, then sorting
**     that array using quicksort before computing the answer. Thus
**     the runtime is O(NlogN) where N is the number of rows of input.
**
**  *  For the window-function versions of these routines, the array of
**     inputs is sorted as soon as the first value is computed.  Thereafter,
**     the array is kept in sorted order using an insert-sort.  This
**     results in O(N*K) performance where K is the size of the window.
**     One can imagine alternative implementations that give O(N*logN*logK)
**     performance, but they require more complex logic and data structures.
**     The developers have elected to keep the asymptotically slower
**     algorithm for now, for simplicity, under the theory that window
**     functions are seldom used and when they are, the window size K is
**     often small.  The developers might revisit that decision later,
**     should the need arise.
*/

/* The following object is the group context for a single percentile()
** aggregate.  Remember all input Y values until the very end.
** Those values are accumulated in the Percentile.a[] array.
*/
రకనిర్వచనం నిర్మాణం Percentile Percentile;
నిర్మాణం Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 n_వాడిన;           /* Number of slots actually used in a[] */
  అక్షరం bSorted;        /* True if a[] is already in sorted order */
  అక్షరం bKeepSorted;    /* True if advantageous to keep a[] sorted */
  అక్షరం bPctValid;      /* True if rPct is valid */
  ద్విగుణ rPct;         /* Fraction.  0.0 to 1.0 */
  ద్విగుణ *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
స్థిర పూర్ణాంకం percentIsInfinity(ద్విగుణ r){
  sqlite3_uint64 u;
  ప్రతిజ్ఞా( పరిమాణం(u)==పరిమాణం(r) );
  జ్ఞాపకం_నకలు(&u, &r, పరిమాణం(u));
  తిరిగి ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
స్థిర పూర్ణాంకం percent_Same_విలువ(ద్విగుణ a, ద్విగుణ b){
  a -= b;
  తిరిగి a>=-0.001 && a<=0.001;
}

/*
** Search p (which must have p->bSorted) looking for an entry with
** value y.  Return the index of that entry.
**
** If bExact is true, return -1 if the entry is not found.
**
** If bExact is false, return the index at which a new entry with
** value y should be insert in order to keep the values in sorted
** order.  The smallest return value in this case will be 0, and
** the largest return value will be p->nUsed.
*/
స్థిర i64 percent_ద్విమాన_శోధన(Percentile *p, ద్విగుణ y, పూర్ణాంకం bExact){
  i64 i_మొదటి = 0;                   /* First element of search range */
  i64 i_చివరి = (i64)p->n_వాడిన - 1;    /* Last element of search range */
  అయితే( i_చివరి>=i_మొదటి ){
    i64 iMid = (i_మొదటి+i_చివరి)/2;
    ద్విగుణ x = p->a[iMid];
    ఒకవేళ( x<y ){
      i_మొదటి = iMid + 1;
    }లేకపోతే ఒకవేళ( x>y ){
      i_చివరి = iMid - 1;
    }లేకపోతే{
      తిరిగి iMid;
    }
  }
  ఒకవేళ( bExact ) తిరిగి -1;
  తిరిగి i_మొదటి;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
స్థిర శూన్య percentError(sqlite3_context *pCtx, స్థిరరాశి అక్షరం *zFormat, ...){
  అక్షరం *zMsg1;
  అక్షరం *zMsg2;
  va_list ap;

  va_start(ap, zFormat);
  zMsg1 = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_ఫలితం_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

/*
** The "step" function for percentile(Y,P) is called once for each
** input row.
*/
స్థిర శూన్య percent_అడుగు(sqlite3_context *pCtx, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  Percentile *p;
  ద్విగుణ rPct;
  పూర్ణాంకం eType;
  ద్విగుణ y;
  ప్రతిజ్ఞా( argc==2 || argc==1 );

  ఒకవేళ( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }లేకపోతే{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    ద్విగుణ mxFrac = (SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_విలువ_numeric_type(argv[1]);
    rPct = sqlite3_విలువ_double(argv[1])/mxFrac;
    ఒకవేళ( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (ద్విగుణ)mxFrac);
      తిరిగి;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, పరిమాణం(*p));
  ఒకవేళ( p==0 ) తిరిగి;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  ఒకవేళ( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }లేకపోతే ఒకవేళ( !percent_Same_విలువ(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    తిరిగి;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_విలువ_type(argv[0]);
  ఒకవేళ( eType==SQLITE_NULL ) తిరిగి;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  ఒకవేళ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    తిరిగి;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_విలువ_double(argv[0]);
  ఒకవేళ( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    తిరిగి;
  }

  /* Allocate and store the Y */
  ఒకవేళ( p->n_వాడిన>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    ద్విగుణ *a = sqlite3_realloc64(p->a, పరిమాణం(ద్విగుణ)*n);
    ఒకవేళ( a==0 ){
      sqlite3_free(p->a);
      memset(p, 0, పరిమాణం(*p));
      sqlite3_ఫలితం_error_nomem(pCtx);
      తిరిగి;
    }
    p->nAlloc = n;
    p->a = a;
  }
  ఒకవేళ( p->n_వాడిన==0 ){
    p->a[p->n_వాడిన++] = y;
    p->bSorted = 1;
  }లేకపోతే ఒకవేళ( !p->bSorted || y>=p->a[p->n_వాడిన-1] ){
    p->a[p->n_వాడిన++] = y;
  }లేకపోతే ఒకవేళ( p->bKeepSorted ){
    i64 i;
    i = percent_ద్విమాన_శోధన(p, y, 0);
    ఒకవేళ( i<(పూర్ణాంకం)p->n_వాడిన ){
      memmove(&p->a[i+1], &p->a[i], (p->n_వాడిన-i)*పరిమాణం(p->a[0]));
    }
    p->a[i] = y;
    p->n_వాడిన++;
  }లేకపోతే{
    p->a[p->n_వాడిన++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#నిర్వచించు మార్పిడి_DOUBLE(X,Y)  {ద్విగుణ ttt=(X);(X)=(Y);(Y)=ttt;}

/*
** Sort an array of doubles.
**
** Algorithm: quicksort
**
** This is implemented separately rather than using the qsort() routine
** from the standard library because:
**
**    (1)  To avoid a dependency on qsort()
**    (2)  To avoid the function call to the comparison routine for each
**         comparison.
**
** If parameter iReq is non-negative, then the caller will only access
** elements a[iReq] and a[iReq+1] (if it exists) of the sorted array and
** so it is not necessary to position any other elements. Or if iReq is
** negative, then the final array must be fully sorted.
*/
స్థిర శూన్య percent_క్రమీకరించు(
  ద్విగుణ *a,                      /* Array to sort */
  గుర్తులేని పూర్ణాంకం n,                 /* Number of elements in array a[] */
  పూర్ణాంకం iReq                        /* Element caller cares about (or -ve) */
){
  పూర్ణాంకం iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  పూర్ణాంకం iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  పూర్ణాంకం i;         /* Loop counter */
  ద్విగుణ r_ఇరుసు; /* The pivot value */

  ప్రతిజ్ఞా( n>=2 );
  చేయి{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    ఒకవేళ( a[0]>a[n-1] ){
      మార్పిడి_DOUBLE(a[0],a[n-1])
    }
    ఒకవేళ( n==2 ) తిరిగి;
    iGt = n-1;
    i = n/2;
    ఒకవేళ( a[0]>a[i] ){
      మార్పిడి_DOUBLE(a[0],a[i])
    }లేకపోతే ఒకవేళ( a[i]>a[iGt] ){
      మార్పిడి_DOUBLE(a[i],a[iGt])
    }
    ఒకవేళ( n==3 ) తిరిగి;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    r_ఇరుసు = a[i];
    iLt = i = 1;
    చేయి{
      ఒకవేళ( a[i]<r_ఇరుసు ){
        ఒకవేళ( i>iLt ) మార్పిడి_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }లేకపోతే ఒకవేళ( a[i]>r_ఇరుసు ){
        చేయి{
          iGt--;
        }అయితే( iGt>i && a[iGt]>r_ఇరుసు );
        మార్పిడి_DOUBLE(a[i],a[iGt])
      }లేకపోతే{
        i++;
      }
    }అయితే( i<iGt );

    ప్రతిజ్ఞా( iLt>0 && iLt<iGt && (గుర్తులేని)iGt<n );
    testcase( iGt>iLt+1 );
    ప్రతిజ్ఞా( a[iLt]==r_ఇరుసు );
    ప్రతిజ్ఞా( a[iLt-1]<=r_ఇరుసు );
    ప్రతిజ్ఞా( a[iGt]>=r_ఇరుసు );
    ప్రతిజ్ఞా( a[iLt+1]>=r_ఇరుసు );

    ఒకవేళ( iReq>=0 ){
      /* In this case, the only elements that the caller requires sorted into 
      ** the correct positions are elements a[iReq] and a[iReq+1]. At this
      ** point we know that element a[iLt] is in the correct position and
      ** all elements smaller than a[iLt] are in the left-hand partition.
      ** So if (iReq<iLt), then it is only necessary to sort the left
      ** partition.
      **
      ** If (iReq>=iLt), then elements iReq and iReq+1 are either in the
      ** right partition or the equal partition (elements for which 
      ** iLt<=iElem<iGt). Therefore it is always sufficient to sort only
      ** the right partition in this case.  */
      ఒకవేళ( iReq<iLt ){
        n = iLt;
      }లేకపోతే{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }లేకపోతే{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      ఒకవేళ( iLt>(పూర్ణాంకం)(n/2) ){
        ఒకవేళ( n-iGt>=2 ) percent_క్రమీకరించు(a+iGt, n-iGt, -1);
        n = iLt;
      }లేకపోతే{
        ఒకవేళ( iLt>=2 ) percent_క్రమీకరించు(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }అయితే( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
స్థిర శూన్య percentInverse(sqlite3_context *pCtx,పూర్ణాంకం argc,sqlite3_విలువ **argv){
  Percentile *p;
  పూర్ణాంకం eType;
  ద్విగుణ y;
  i64 i;
  ప్రతిజ్ఞా( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, పరిమాణం(*p));
  ప్రతిజ్ఞా( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_విలువ_type(argv[0]);
  ఒకవేళ( eType==SQLITE_NULL ) తిరిగి;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  ఒకవేళ( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    తిరిగి;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_విలువ_double(argv[0]);
  ఒకవేళ( percentIsInfinity(y) ){
    తిరిగి;
  }
  ఒకవేళ( p->bSorted==0 ){
    ప్రతిజ్ఞా( p->n_వాడిన>1 );
    percent_క్రమీకరించు(p->a, p->n_వాడిన, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_ద్విమాన_శోధన(p, y, 1);
  ఒకవేళ( i>=0 ){
    p->n_వాడిన--;
    ఒకవేళ( i<(పూర్ణాంకం)p->n_వాడిన ){
      memmove(&p->a[i], &p->a[i+1], (p->n_వాడిన - i)*పరిమాణం(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
స్థిర శూన్య percentCompute(sqlite3_context *pCtx, పూర్ణాంకం bIsFinal){
  Percentile *p;
  పూర్ణాంకం settings = SQLITE_PTR_గమ్యం_INT(sqlite3_user_దత్తాంశం(pCtx))&1; /* Discrete? */
  గుర్తులేని i1, i2;
  ద్విగుణ v1, v2;
  ద్విగుణ ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  ఒకవేళ( p==0 ) తిరిగి;
  ఒకవేళ( p->a==0 ) తిరిగి;
  ఒకవేళ( p->n_వాడిన ){
    ix = p->rPct*(p->n_వాడిన-1);
    i1 = (గుర్తులేని)ix;
    ఒకవేళ( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      ప్రతిజ్ఞా( p->n_వాడిన>1 );
      percent_క్రమీకరించు(p->a, p->n_వాడిన, (bIsFinal ? (పూర్ణాంకం)i1 : -1));
      p->bSorted = 1;
    }
    ఒకవేళ( settings & 1 ){
      vx = p->a[i1];
    }లేకపోతే{
      i2 = ix==(ద్విగుణ)i1 || i1==p->n_వాడిన-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_ఫలితం_double(pCtx, vx);
  }
  ఒకవేళ( bIsFinal ){
    sqlite3_free(p->a);
    memset(p, 0, పరిమాణం(*p));
  }లేకపోతే{
    p->bKeepSorted = 1;
  }
}
స్థిర శూన్య percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
స్థిర శూన్య percent_విలువ(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#అయితే_ముగింపు /* SQLITE_ENABLE_PERCENTILE */

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
స్థిర శూన్య filestatFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  స్థిరరాశి అక్షరం *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  pBtree = sqlite3_Db_Name_గమ్యం_Btree(db, zDbName);
  ఒకవేళ( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    పూర్ణాంకం rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ప్రతిజ్ఞా( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    ఒకవేళ( sqlite3_str_errcode(pStr) ){
      sqlite3_ఫలితం_error_nomem(context);
    }లేకపోతే{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      ఒకవేళ( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      ఒకవేళ( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        ఒకవేళ( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_ఫలితం_str(context, pStr, SQLITE_ముగింపు);
    }
    sqlite3BtreeLeave(pBtree);
  }లేకపోతే{
    sqlite3_ఫలితం_పాఠ్యం(context, "{}", 2, SQLITE_STATIC);
  }
}
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
స్థిర శూన్య walstatFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  స్థిరరాశి అక్షరం *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  pBtree = sqlite3_Db_Name_గమ్యం_Btree(db, zDbName);
  ఒకవేళ( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    ప్రతిజ్ఞా( pPager!=0 );
    pStr = sqlite3_str_new(db);
    ఒకవేళ( sqlite3_str_errcode(pStr) ){
      sqlite3_ఫలితం_error_nomem(context);
    }లేకపోతే{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_ఫలితం_str(context, pStr, SQLITE_ముగింపు);
    }
    sqlite3BtreeLeave(pBtree);
  }లేకపోతే{
    sqlite3_ఫలితం_పాఠ్యం(context, "{}", 2, SQLITE_STATIC);
  }
}
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Implementation of fpdecode(x,y,z) function.
**
** x is a real number that is to be decoded.  y is the precision.
** z is the maximum real precision.  Return a string that shows the
** results of the sqlite3FpDecode() function.
**
** Used for testing and debugging only, specifically testing and debugging
** of the sqlite3FpDecode() function.  This SQL function does not appear
** in production builds.  This function is not an API and is subject to
** modification or removal in future versions of SQLite.
*/
స్థిర శూన్య fpdecodeFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  FpDecode s;
  ద్విగుణ x;
  పూర్ణాంకం y, z;
  అక్షరం zBuf[100];
  వాడని_PARAMETER(argc);
  ప్రతిజ్ఞా( argc==3 );
  x = sqlite3_విలువ_double(argv[0]);
  y = sqlite3_విలువ_int(argv[1]);
  z = sqlite3_విలువ_int(argv[2]);
  ఒకవేళ( z<=0 ) z = 1;
  ఒకవేళ( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  ఒకవేళ( s.isSpecial==2 ){
    sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "NaN");
  }లేకపోతే{
    sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
  }
  sqlite3_ఫలితం_పాఠ్యం(context, zBuf, -1, SQLITE_TRANSIENT);
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Implementation of parseuri(uri,flags) function.
**
** Required Arguments:
**    "uri"        The URI to parse.
**    "flags"      Bitmask of flags, as if to sqlite3_open_v2().
**
** Additional arguments beyond the first two make calls to
** sqlite3_uri_key() for integers and sqlite3_uri_parameter for
** anything else.
**
** The result is a string showing the results of calling sqlite3ParseUri().
**
** Used for testing and debugging only, specifically testing and debugging
** of the sqlite3ParseUri() function.  This SQL function does not appear
** in production builds.  This function is not an API and is subject to
** modification or removal in future versions of SQLite.
*/
స్థిర శూన్య parseuriFunc(
  sqlite3_context *ctx,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3_str *p_ఫలితం;
  స్థిరరాశి అక్షరం *zVfs;
  స్థిరరాశి అక్షరం *zUri;
  గుర్తులేని పూర్ణాంకం flgs;
  పూర్ణాంకం rc;
  sqlite3_vfs *pVfs = 0;
  అక్షరం *zFile = 0;
  అక్షరం *zErr = 0;

  ఒకవేళ( argc<2 ) తిరిగి;
  pVfs = sqlite3_vfs_కనుగొను(0);
  ప్రతిజ్ఞా( pVfs );
  zVfs = pVfs->zName;
  zUri = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  ఒకవేళ( zUri==0 ) తిరిగి;
  flgs = (గుర్తులేని పూర్ణాంకం)sqlite3_విలువ_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  p_ఫలితం = sqlite3_str_new(0);
  ఒకవేళ( !sqlite3_str_errcode(p_ఫలితం) ){
    పూర్ణాంకం i;
    sqlite3_str_appendf(p_ఫలితం, "rc=%d", rc);
    sqlite3_str_appendf(p_ఫలితం, ", flags=0x%x", flgs);
    sqlite3_str_appendf(p_ఫలితం, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(p_ఫలితం, ", err=%Q", zErr);
    sqlite3_str_appendf(p_ఫలితం, ", file=%Q", zFile);
    ఒకవేళ( zFile ){
      స్థిరరాశి అక్షరం *z = zFile;
      z += sqlite3Strlen30(z)+1;
      అయితే( z[0] ){
        sqlite3_str_appendf(p_ఫలితం, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      కొరకు(i=2; i<argc; i++){
        స్థిరరాశి అక్షరం *zArg;
        ఒకవేళ( sqlite3_విలువ_type(argv[i])==SQLITE_INTEGER ){
          పూర్ణాంకం k = sqlite3_విలువ_int(argv[i]);
          sqlite3_str_appendf(p_ఫలితం, ", '%d:%q'",k,sqlite3_uri_కీలకం(zFile, k));
        }లేకపోతే ఒకవేళ( (zArg = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]))!=0 ){
          sqlite3_str_appendf(p_ఫలితం, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }లేకపోతే{
          sqlite3_str_appendf(p_ఫలితం, ", NULL");
        }
      }
    }
  }
  sqlite3_ఫలితం_str(ctx, p_ఫలితం, SQLITE_ముగింపు);
  sqlite3_free_filename(zFile);
  sqlite3_free(zErr);
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
శూన్య sqlite3_రిజిస్టరు_Builtin_Functions(శూన్య){
  /*
  ** The following array holds FuncDef structures for all of the functions
  ** defined in this file.
  **
  ** The array cannot be constant since changes are made to the
  ** FuncDef.pHash elements at start-time.  The elements of this array
  ** are read-only after initialization is complete.
  **
  ** For peak efficiency, put the most frequently used function last.
  */
  స్థిర FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#ఒకవేళ !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(సూచిస్తుంది_nonnull_అడ్డువరుస, 2, INLINEFUNC_సూచిస్తుంది_nonnull_అడ్డువరుస, 0),
    TEST_FUNC(సమాసం_compare,        2, INLINEFUNC_సమాసం_compare,        0),
    TEST_FUNC(సమాసం_సూచిస్తుంది_సమాసం,   2, INLINEFUNC_సమాసం_సూచిస్తుంది_సమాసం,   0),
    TEST_FUNC(affinity,            1, INLINEFUNC_affinity,            0),
#అయితే_ముగింపు /* !defined(SQLITE_UNTESTABLE) */
/***** Regular functions *****/
#నిర్వచితమైతే SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
    SFUNCTION(ఎక్కించు_extension,    1, 0, 0, ఎక్కించు_Ext          ),
    SFUNCTION(ఎక్కించు_extension,    2, 0, 0, ఎక్కించు_Ext          ),
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_వాడిన,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#అయితే_ముగింపు /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    INLINE_FUNC(unlikely,        1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likelihood,      2, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likely,          1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
#నిర్వచితమైతే SQLITE_ENABLE_స్థానభ్రంశం_SQL_FUNC
    INLINE_FUNC(sqlite_స్థానభ్రంశం,   1, INLINEFUNC_sqlite_స్థానభ్రంశం, 0 ),
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    FUNCTION(sqlite_filestat,    1, 0, 0, filestatFunc     ),
    FUNCTION(sqlite_walstat,     1, 0, 0, walstatFunc      ),
#అయితే_ముగింపు
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -3, 0, 1, minmaxFunc       ),
    WAGGREGATE(min, 1, 0, 1, minmax_అడుగు, minMaxFinalize, min_Max_విలువ, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmax_అడుగు, minMaxFinalize, min_Max_విలువ, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(subtype,           1, 0, 0, subtypeFunc,
                                           SQLITE_FUNC_TYPEOF|SQLITE_SUBTYPE),
    FUNCTION2(నిడివి,            1, 0, 0, నిడివి_Func,  SQLITE_FUNC_నిడివి),
    FUNCTION2(octet_నిడివి,      1, 0, 0, bytelengthFunc,SQLITE_FUNC_BYTELEN),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(రాయి,            -1, 0, 0, printfFunc       ),
    FUNCTION(format,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(అక్షరం,              -1, 0, 0, charFunc         ),
    FUNCTION(నిరపేక్ష,                1, 0, 0, absFunc          ),
#నిర్వచితమైతే SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
    FUNCTION(oom,               -1, 0, 0, oomFunc          ),
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
    FUNCTION(విడత,              1, 0, 0, విడత_Func        ),
    FUNCTION(విడత,              2, 0, 0, విడత_Func        ),
#అయితే_ముగింపు
    FUNCTION(పై,              1, 0, 0, పై_Func        ),
    FUNCTION(క్రింది,              1, 0, 0, క్రింది_Func        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION(unhex,              1, 0, 0, unhexFunc        ),
    FUNCTION(unhex,              2, 0, 0, unhexFunc        ),
    FUNCTION(concat,            -3, 0, 0, concatFunc       ),
    FUNCTION(concat_ws,         -4, 0, 0, concatwsFunc     ),
    INLINE_FUNC(ifnull,          2, INLINEFUNC_coalesce, 0 ),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_సంస్కరణ,    0, 0, 0, సంస్కరణ_Func      ),
    DFUNCTION(sqlite_ఆధారం_id,  0, 0, 0, sourceidFunc     ),
    SFUNCTION(sqlite_log,        2, 0, 0, errlogFunc       ),
    FUNCTION(unistr,             1, 0, 0, unistrFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    FUNCTION(unistr_quote,       1, 1, 0, quoteFunc        ),
    VFUNCTION(చివరి_చొప్పించు_rowid, 0, 0, 0, చివరి_చొప్పించు_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(మొత్తం_changes,     0, 0, 0, మొత్తం_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    FUNCTION(substring,          2, 0, 0, substrFunc       ),
    FUNCTION(substring,          3, 0, 0, substrFunc       ),
    WAGGREGATE(కూడిక,   1,0,0, కూడిక_అడుగు, కూడిక_Finalize, కూడిక_Finalize, కూడిక_Inverse, 0),
    WAGGREGATE(మొత్తం, 1,0,0, కూడిక_అడుగు,మొత్తం_Finalize,మొత్తం_Finalize,కూడిక_Inverse, 0),
    WAGGREGATE(avg,   1,0,0, కూడిక_అడుగు, avgFinalize, avgFinalize, కూడిక_Inverse, 0),
    WAGGREGATE(లెక్క, 0,0,0, లెక్క_అడుగు,
        లెక్క_Finalize, లెక్క_Finalize, లెక్క_Inverse,
        SQLITE_FUNC_లెక్క|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(లెక్క, 1,0,0, లెక్క_అడుగు,
        లెక్క_Finalize, లెక్క_Finalize, లెక్క_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(సమూహం_concat, 1, 0, 0, సమూహం_Concat_అడుగు,
        సమూహం_Concat_Finalize, సమూహం_Concat_విలువ, సమూహం_Concat_Inverse, 0),
    WAGGREGATE(సమూహం_concat, 2, 0, 0, సమూహం_Concat_అడుగు,
        సమూహం_Concat_Finalize, సమూహం_Concat_విలువ, సమూహం_Concat_Inverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, సమూహం_Concat_అడుగు,
        సమూహం_Concat_Finalize, సమూహం_Concat_విలువ, సమూహం_Concat_Inverse, 0),

#నిర్వచితమైతే SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percent_అడుగు,
        percentFinal, percent_విలువ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percent_అడుగు,
        percentFinal, percent_విలువ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percent_అడుగు,
        percentFinal, percent_విలువ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percent_అడుగు,
        percentFinal, percent_విలువ, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#అయితే_ముగింపు /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#నిర్వచితమైతే SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#లేకపోతే
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    FUNCTION(unknown,           -1, 0, 0, unknownFunc      ),
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_MATH_FUNCTIONS
    MFUNCTION(ceil,              1, xCeil,     ceilingFunc ),
    MFUNCTION(ceiling,           1, xCeil,     ceilingFunc ),
    MFUNCTION(floor,             1, xFloor,    ceilingFunc ),
#ఒకవేళ SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(trunc,             1, trunc,     ceilingFunc ),
#అయితే_ముగింపు
    FUNCTION(ln,                 1, 0, 0,      logFunc     ),
    FUNCTION(సంవర్గమానం,                1, 1, 0,      logFunc     ),
    FUNCTION(log10,              1, 1, 0,      logFunc     ),
    FUNCTION(log2,               1, 2, 0,      logFunc     ),
    FUNCTION(సంవర్గమానం,                2, 0, 0,      logFunc     ),
    MFUNCTION(ఘాతాంకం,               1, ఘాతాంకం,       math1Func   ),
    MFUNCTION(ఘాతం,               2, ఘాతం,       math2Func   ),
    MFUNCTION(power,             2, ఘాతం,       math2Func   ),
    MFUNCTION(mod,               2, fmod,      math2Func   ),
    MFUNCTION(acos,              1, acos,      math1Func   ),
    MFUNCTION(asin,              1, asin,      math1Func   ),
    MFUNCTION(atan,              1, atan,      math1Func   ),
    MFUNCTION(atan2,             2, atan2,     math2Func   ),
    MFUNCTION(కోజ్యా,               1, కోజ్యా,       math1Func   ),
    MFUNCTION(జ్యా,               1, జ్యా,       math1Func   ),
    MFUNCTION(tan,               1, tan,       math1Func   ),
    MFUNCTION(cosh,              1, cosh,      math1Func   ),
    MFUNCTION(sinh,              1, sinh,      math1Func   ),
    MFUNCTION(tanh,              1, tanh,      math1Func   ),
#ఒకవేళ SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(acosh,             1, acosh,     math1Func   ),
    MFUNCTION(asinh,             1, asinh,     math1Func   ),
    MFUNCTION(atanh,             1, atanh,     math1Func   ),
#అయితే_ముగింపు
    MFUNCTION(వర్గమూలం,              1, వర్గమూలం,      math1Func   ),
    MFUNCTION(radians,           1, deg_గమ్యం_Rad,  math1Func   ),
    MFUNCTION(degrees,           1, rad_గమ్యం_Deg,  math1Func   ),
    MFUNCTION(pi,                0, 0,         piFunc      ),
#అయితే_ముగింపు /* SQLITE_ENABLE_MATH_FUNCTIONS */
    FUNCTION(sign,               1, 0, 0,      signFunc    ),
    INLINE_FUNC(coalesce,       -4, INLINEFUNC_coalesce, 0 ),
    INLINE_FUNC(iif,            -4, INLINEFUNC_iif,      0 ),
    INLINE_FUNC(ఒకవేళ,             -4, INLINEFUNC_iif,      0 ),
  };
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#అయితే_ముగింపు
  sqlite3_కిటికీ_Functions();
  sqlite3_రిజిస్టరు_Date_సమయం_Functions();
  sqlite3_రిజిస్టరు_Json_Functions();
  sqlite3_చొప్పించు_Builtin_Funcs(aBuiltinFunc, Array_కొలత(aBuiltinFunc));

#ఒకవేళ 0  /* Enable to print out how the built-in functions are hashed */
  {
    పూర్ణాంకం i;
    FuncDef *p;
    కొరకు(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      రాయి("FUNC-HASH %02d:", i);
      కొరకు(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        పూర్ణాంకం n = sqlite3Strlen30(p->zName);
        పూర్ణాంకం h = p->zName[0] + n;
        ప్రతిజ్ఞా( p->funcFlags & SQLITE_FUNC_BUILTIN );
        రాయి(" %s(%d)", p->zName, h);
      }
      రాయి("\n");
    }
  }
#అయితే_ముగింపు
}
