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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <assert.h>
#यदि_अपरिभाषितम् SQLITE_OMIT_FLOATING_POINT
#अन्तर्भाव <math.h>
#यदि_अन्तः
#अन्तर्भाव "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
स्थिर CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  प्रतिज्ञा( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  प्रतिज्ञा( pOp->opcode==OP_CollSeq );
  प्रतिज्ञा( pOp->p4type==P4_COLLSEQ );
  प्रत्यागम pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
स्थिर शून्य sqlite3_Skip_सञ्चायकः_आरोपय(sqlite3_context *context){
  प्रतिज्ञा( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
स्थिर शून्य minmaxFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क i;
  पूर्णाङ्क आवरणम्;    /* 0 for min() or 0xffffffff for max() */
  पूर्णाङ्क iBest;
  CollSeq *pColl;

  प्रतिज्ञा( argc>1 );
  आवरणम् = sqlite3_user_दत्त(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  प्रतिज्ञा( pColl );
  प्रतिज्ञा( आवरणम्==-1 || आवरणम्==0 );
  iBest = 0;
  यदि( sqlite3_मानम्_type(argv[0])==SQLITE_NULL ) प्रत्यागम;
  प्रति(i=1; i<argc; i++){
    यदि( sqlite3_मानम्_type(argv[i])==SQLITE_NULL ) प्रत्यागम;
    यदि( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^आवरणम्)>=0 ){
      testcase( आवरणम्==0 );
      iBest = i;
    }
  }
  sqlite3_परिणाम_मानम्(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
स्थिर शून्य typeofFunc(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **argv
){
  स्थिर ध्रुव वर्ण *azType[] = { "integer", "real", "text", "blob", "null" };
  पूर्णाङ्क i = sqlite3_मानम्_type(argv[0]) - 1;
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  प्रतिज्ञा( i>=0 && i<Array_आकार(azType) );
  प्रतिज्ञा( SQLITE_INTEGER==1 );
  प्रतिज्ञा( SQLITE_FLOAT==2 );
  प्रतिज्ञा( SQLITE_TEXT==3 );
  प्रतिज्ञा( SQLITE_BLOB==4 );
  प्रतिज्ञा( SQLITE_NULL==5 );
  /* EVIDENCE-OF: R-01470-60482 The sqlite3_value_type(V) interface returns
  ** the datatype code for the initial datatype of the sqlite3_value object
  ** V. The returned value is one of SQLITE_INTEGER, SQLITE_FLOAT,
  ** SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL. */
  sqlite3_परिणाम_text(context, azType[i], -1, SQLITE_STATIC);
}

/* subtype(X)
**
** Return the subtype of X
*/
स्थिर शून्य subtypeFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  अप्रयुक्तम्_PARAMETER(argc);
  sqlite3_परिणाम_int(context, sqlite3_मानम्_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
स्थिर शून्य दीर्घता_Func(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  विकल्प( sqlite3_मानम्_type(argv[0]) ){
    पक्ष SQLITE_BLOB:
    पक्ष SQLITE_INTEGER:
    पक्ष SQLITE_FLOAT: {
      sqlite3_परिणाम_int(context, sqlite3_मानम्_अष्टकानि(argv[0]));
      भङ्ग;
    }
    पक्ष SQLITE_TEXT: {
      ध्रुव अचिह्नित वर्ण *z = sqlite3_मानम्_text(argv[0]);
      ध्रुव अचिह्नित वर्ण *z0;
      यदि( z==0 ) प्रत्यागम;
      z0 = z;
      यावत्( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        यदि( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }अन्यथा यदि( z[0]==0 ){
          भङ्ग;
        }अन्यथा{
          z++;
          यावत्( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_परिणाम_int(context, (पूर्णाङ्क)(z-z0));
      भङ्ग;
    }
    स्वभाव: {
      sqlite3_परिणाम_null(context);
      भङ्ग;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
स्थिर शून्य bytelengthFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  विकल्प( sqlite3_मानम्_type(argv[0]) ){
    पक्ष SQLITE_BLOB: {
      sqlite3_परिणाम_int(context, sqlite3_मानम्_अष्टकानि(argv[0]));
      भङ्ग;
    }
    पक्ष SQLITE_INTEGER:
    पक्ष SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_परिणाम_int64(context, sqlite3_मानम्_अष्टकानि(argv[0])*m);
      भङ्ग;
    }
    पक्ष SQLITE_TEXT: {
      यदि( sqlite3_मानम्_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_परिणाम_int(context, sqlite3_मानम्_अष्टकानि(argv[0]));
      }अन्यथा{
        sqlite3_परिणाम_int(context, sqlite3_मानम्_bytes16(argv[0]));
      }
      भङ्ग;
    }
    स्वभाव: {
      sqlite3_परिणाम_null(context);
      भङ्ग;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
स्थिर शून्य absFunc(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  विकल्प( sqlite3_मानम्_type(argv[0]) ){
    पक्ष SQLITE_INTEGER: {
      i64 iVal = sqlite3_मानम्_int64(argv[0]);
      यदि( iVal<0 ){
        यदि( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_परिणाम_error(context, "integer overflow", -1);
          प्रत्यागम;
        }
        iVal = -iVal;
      }
      sqlite3_परिणाम_int64(context, iVal);
      भङ्ग;
    }
    पक्ष SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_परिणाम_null(context);
      भङ्ग;
    }
    स्वभाव: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      द्विगुण rVal = sqlite3_मानम्_double(argv[0]);
      यदि( rVal<0 ) rVal = -rVal;
      sqlite3_परिणाम_double(context, rVal);
      भङ्ग;
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
स्थिर शून्य instrFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव अचिह्नित वर्ण *zHaystack;
  ध्रुव अचिह्नित वर्ण *zNeedle;
  पूर्णाङ्क nHaystack;
  पूर्णाङ्क nNeedle;
  पूर्णाङ्क typeHaystack, typeNeedle;
  पूर्णाङ्क N = 1;
  पूर्णाङ्क isText;
  अचिह्नित वर्ण firstChar;
  sqlite3_मानम् *pC1 = 0;
  sqlite3_मानम् *pC2 = 0;

  अप्रयुक्तम्_PARAMETER(argc);
  typeHaystack = sqlite3_मानम्_type(argv[0]);
  typeNeedle = sqlite3_मानम्_type(argv[1]);
  यदि( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) प्रत्यागम;
  nHaystack = sqlite3_मानम्_अष्टकानि(argv[0]);
  nNeedle = sqlite3_मानम्_अष्टकानि(argv[1]);
  यदि( nNeedle>0 ){
    यदि( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_मानम्_blob(argv[0]);
      zNeedle = sqlite3_मानम्_blob(argv[1]);
      isText = 0;
    }अन्यथा यदि( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_मानम्_text(argv[0]);
      zNeedle = sqlite3_मानम्_text(argv[1]);
      isText = 1;
    }अन्यथा{
      pC1 = sqlite3_मानम्_dup(argv[0]);
      zHaystack = sqlite3_मानम्_text(pC1);
      यदि( zHaystack==0 ) गच्छ अन्तः_Instr_OOM;
      nHaystack = sqlite3_मानम्_अष्टकानि(pC1);
      pC2 = sqlite3_मानम्_dup(argv[1]);
      zNeedle = sqlite3_मानम्_text(pC2);
      यदि( zNeedle==0 ) गच्छ अन्तः_Instr_OOM;
      nNeedle = sqlite3_मानम्_अष्टकानि(pC2);
      isText = 1;
    }
    यदि( zNeedle==0 || (nHaystack && zHaystack==0) ) गच्छ अन्तः_Instr_OOM;
    firstChar = zNeedle[0];
    यावत्( nNeedle<=nHaystack
       && (zHaystack[0]!=firstChar || memcmp(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      कुरु{
        nHaystack--;
        zHaystack++;
      }यावत्( isText && (zHaystack[0]&0xc0)==0x80 );
    }
    यदि( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_परिणाम_int(context, N);
अन्तः_Instr:
  sqlite3_मानम्_free(pC1);
  sqlite3_मानम्_free(pC2);
  प्रत्यागम;
अन्तः_Instr_OOM:
  sqlite3_परिणाम_error_nomem(context);
  गच्छ अन्तः_Instr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
स्थिर शून्य printfFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  PrintfArguments x;
  StrAccum str;
  ध्रुव वर्ण *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  यदि( argc>=1 && (zFormat = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.n_प्रयुक्तम् = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->a_सीमा[SQLITE_सीमा_दीर्घता]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3_str_appendf(&str, zFormat, &x);
    sqlite3_परिणाम_str(context, &str, SQLITE_XFER);
  }
}

/* Return true if any of the eight bytes in word is NUL or non-ASCII. */
स्थिर SQLITE_INLINE पूर्णाङ्क अन्तर्भवति_Nul_Or_Non_Ascii(u64 word){
  ध्रुव u64 byteOnes = UINT64_C(0x0101010101010101);
  ध्रुव u64 byte_उच्चः_द्वयङ्काः = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  प्रत्यागम ((word | (word - byteOnes)) & byte_उच्चः_द्वयङ्काः)!=0;
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
स्थिर शून्य substrFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव अचिह्नित वर्ण *z;
  ध्रुव अचिह्नित वर्ण *z2;
  पूर्णाङ्क len;
  पूर्णाङ्क p0type;
  i64 p1, p2;

  प्रतिज्ञा( argc==3 || argc==2 );
  p0type = sqlite3_मानम्_type(argv[0]);
  p1 = sqlite3_मानम्_int64(argv[1]);
  यदि( p0type==SQLITE_BLOB ){
    len = sqlite3_मानम्_अष्टकानि(argv[0]);
    z = sqlite3_मानम्_blob(argv[0]);
    यदि( z==0 ) प्रत्यागम;
    प्रतिज्ञा( len==sqlite3_मानम्_अष्टकानि(argv[0]) );
  }अन्यथा{
    z = sqlite3_मानम्_text(argv[0]);
    यदि( z==0 ) प्रत्यागम;
    len = 0;
    यदि( p1<0 ){
      प्रति(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  यदि( argc==3 ){
    p2 = sqlite3_मानम्_int64(argv[2]);
    यदि( p2==0 && sqlite3_मानम्_type(argv[2])==SQLITE_NULL ) प्रत्यागम;
  }अन्यथा{
    p2 = LARGEST_INT64;
  }
  यदि( p1==0 ){
#यदि_परिभाषितम् SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#यदि_अन्तः
    यदि( sqlite3_मानम्_type(argv[1])==SQLITE_NULL ) प्रत्यागम;
  }
  यदि( p1<0 ){
    p1 += len;
    यदि( p1<0 ){
      यदि( p2<0 ){
        p2 = 0;
      }अन्यथा{
        p2 += p1;
      }
      p1 = 0;
    }
  }अन्यथा यदि( p1>0 ){
    p1--;
  }अन्यथा यदि( p2>0 ){
    p2--;
  }
  यदि( p2<0 ){
    यदि( p2<-p1 ){
      p2 = p1;
    }अन्यथा{
      p2 = -p2;
    }
    p1 -= p2;
  }
  प्रतिज्ञा( p1>=0 && p2>=0 );
  यदि( p0type!=SQLITE_BLOB ){
    यदि( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      ध्रुव अचिह्नित वर्ण *z_अन्तः = z + sqlite3_मानम्_अष्टकानि(argv[0]);
      यावत्( p1>=8 && z_अन्तः-z>=8 ){
        u64 word;
        स्मृतिं_प्रतिलिख(&word, z, परिमाण(word));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        यदि( अन्तर्भवति_Nul_Or_Non_Ascii(word) ){
          भङ्ग;
        }
        z += 8;
        p1 -= 8;
      }
    }
    प्रति( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      यदि( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }अन्यथा यदि( z[0]==0 ){
        भङ्ग;
      }अन्यथा{
        कुरु{ z++; }यावत्( (z[0]&0xc0)==0x80 );
      }
    }
    प्रति(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      यदि( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }अन्यथा यदि( z2[0]==0 ){
        भङ्ग;
      }अन्यथा{
        कुरु{ z2++; }यावत्( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_परिणाम_text64(context, (वर्ण*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }अन्यथा{
    यदि( p1>=len ){
      p1 = p2 = 0;
    }अन्यथा यदि( p2>len-p1 ){
      p2 = len-p1;
      प्रतिज्ञा( p2>0 );
    }
    sqlite3_परिणाम_blob64(context, (वर्ण*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_FLOATING_POINT
स्थिर शून्य आवर्तनम्_Func(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  i64 n = 0;
  द्विगुण r;
  वर्ण *zBuf;
  प्रतिज्ञा( argc==1 || argc==2 );
  यदि( argc==2 ){
    यदि( SQLITE_NULL==sqlite3_मानम्_type(argv[1]) ) प्रत्यागम;
    n = sqlite3_मानम्_int64(argv[1]);
    यदि( n>30 ) n = 30;
    यदि( n<0 ) n = 0;
  }
  यदि( sqlite3_मानम्_type(argv[0])==SQLITE_NULL ) प्रत्यागम;
  r = sqlite3_मानम्_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  यदि( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }अन्यथा यदि( n==0 ){ 
    r = (द्विगुण)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }अन्यथा{
    zBuf = sqlite3_mprintf("%!.*f",(पूर्णाङ्क)n,r);
    यदि( zBuf==0 ){
      sqlite3_परिणाम_error_nomem(context);
      प्रत्यागम;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_परिणाम_double(context, r);
}
#यदि_अन्तः

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
स्थिर शून्य *contextMalloc(sqlite3_context *context, i64 nByte){
  वर्ण *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  प्रतिज्ञा( nByte>0 );
  testcase( nByte==db->a_सीमा[SQLITE_सीमा_दीर्घता] );
  testcase( nByte==(i64)db->a_सीमा[SQLITE_सीमा_दीर्घता]+1 );
  यदि( nByte>db->a_सीमा[SQLITE_सीमा_दीर्घता] ){
    sqlite3_परिणाम_error_toobig(context);
    z = 0;
  }अन्यथा{
    z = sqlite3Malloc(nByte);
    यदि( !z ){
      sqlite3_परिणाम_error_nomem(context);
    }
  }
  प्रत्यागम z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
स्थिर शून्य ऊर्ध्वस्थम्_Func(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  वर्ण *z1;
  ध्रुव वर्ण *z2;
  पूर्णाङ्क i, n;
  अप्रयुक्तम्_PARAMETER(argc);
  z2 = (वर्ण*)sqlite3_मानम्_text(argv[0]);
  n = sqlite3_मानम्_अष्टकानि(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  प्रतिज्ञा( z2==(वर्ण*)sqlite3_मानम्_text(argv[0]) );
  यदि( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    यदि( z1 ){
      प्रति(i=0; i<n; i++){
        z1[i] = (वर्ण)sqlite3Toupper(z2[i]);
      }
      sqlite3_परिणाम_text(context, z1, n, sqlite3_free);
    }
  }
}
स्थिर शून्य अधस्तनम्_Func(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  वर्ण *z1;
  ध्रुव वर्ण *z2;
  पूर्णाङ्क i, n;
  अप्रयुक्तम्_PARAMETER(argc);
  z2 = (वर्ण*)sqlite3_मानम्_text(argv[0]);
  n = sqlite3_मानम्_अष्टकानि(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  प्रतिज्ञा( z2==(वर्ण*)sqlite3_मानम्_text(argv[0]) );
  यदि( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    यदि( z1 ){
      प्रति(i=0; i<n; i++){
        z1[i] = sqlite3Tolower(z2[i]);
      }
      sqlite3_परिणाम_text(context, z1, n, sqlite3_free);
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
#परिभाषय noopFunc संस्करणम्_Func   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer. 
*/
स्थिर शून्य randomFunc(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **NotUsed2
){
  sqlite_int64 r;
  अप्रयुक्तम्_PARAMETER2(Not_प्रयुक्तम्, NotUsed2);
  sqlite3_randomness(परिमाण(r), &r);
  यदि( r<0 ){
    /* We need to prevent a random number of 0x8000000000000000
    ** (or -9223372036854775808) since when you do abs() of that
    ** number of you get the same value back again.  To do this
    ** in a way that is testable, mask the sign bit off of negative
    ** values, resulting in a positive value.  Then take the
    ** 2s complement of that positive value.  The end result can
    ** therefore be no less than -9223372036854775807.
    */
    r = -(r & LARGEST_INT64);
  }
  sqlite3_परिणाम_int64(context, r);
}

/*
** Implementation of randomblob(N).  Return a random blob
** that is N bytes long.
*/
स्थिर शून्य randomBlob(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  sqlite3_int64 n;
  अचिह्नित वर्ण *p;
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  n = sqlite3_मानम्_int64(argv[0]);
  यदि( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  यदि( p ){
    sqlite3_randomness(n, p);
    sqlite3_परिणाम_blob(context, (वर्ण*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
स्थिर शून्य चरमः_निवेशय_rowid(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  अप्रयुक्तम्_PARAMETER2(Not_प्रयुक्तम्, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_परिणाम_int64(context, sqlite3_चरमः_निवेशय_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
स्थिर शून्य changes(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  अप्रयुक्तम्_PARAMETER2(Not_प्रयुक्तम्, NotUsed2);
  sqlite3_परिणाम_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
स्थिर शून्य योग_changes(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  अप्रयुक्तम्_PARAMETER2(Not_प्रयुक्तम्, NotUsed2);
  /* IMP: R-11217-42568 This function is a wrapper around the
  ** sqlite3_total_changes64() C/C++ interface. */
  sqlite3_परिणाम_int64(context, sqlite3_योग_changes64(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
संरचना compareInfo {
  u8 मेलनम्_All;          /* "*" or "%" */
  u8 मेलनम्_One;          /* "?" or "_" */
  u8 मेलनम्_समुच्चयः;          /* "[" or 0 */
  u8 noCase;            /* true to ignore case differences */
};

/*
** For LIKE and GLOB matching on EBCDIC machines, assume that every
** character is exactly one byte in size.  Also, provide the Utf8Read()
** macro for fast reading of the next character in the common case where
** the next character is ASCII.
*/
#यदि defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#अन्यथा
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#यदि_अन्तः

स्थिर ध्रुव संरचना compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
स्थिर ध्रुव संरचना compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
स्थिर ध्रुव संरचना compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Possible error returns from patternMatch()
*/
#परिभाषय SQLITE_मेलनम्             0
#परिभाषय SQLITE_NOMATCH           1
#परिभाषय SQLITE_NOWILDCARDMATCH   2

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
स्थिर पूर्णाङ्क आदर्शः_Compare(
  ध्रुव u8 *z_आदर्शः,              /* The glob pattern */
  ध्रुव u8 *zString,               /* The string to compare against the glob */
  ध्रुव संरचना compareInfo *pInfo, /* Information about how to do the compare */
  u32 मेलनम्_Other                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 मेलनम्_One = pInfo->मेलनम्_One;  /* "?" or "_" */
  u32 मेलनम्_All = pInfo->मेलनम्_All;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  ध्रुव u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  यावत्( (c = Utf8Read(z_आदर्शः))!=0 ){
    यदि( c==मेलनम्_All ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      यावत्( (c=Utf8Read(z_आदर्शः)) == मेलनम्_All
             || (c == मेलनम्_One && मेलनम्_One!=0) ){
        यदि( c==मेलनम्_One && sqlite3Utf8Read(&zString)==0 ){
          प्रत्यागम SQLITE_NOWILDCARDMATCH;
        }
      }
      यदि( c==0 ){
        प्रत्यागम SQLITE_मेलनम्;   /* "*" at the end of the pattern matches */
      }अन्यथा यदि( c==मेलनम्_Other ){
        यदि( pInfo->मेलनम्_समुच्चयः==0 ){
          c = sqlite3Utf8Read(&z_आदर्शः);
          यदि( c==0 ) प्रत्यागम SQLITE_NOWILDCARDMATCH;
        }अन्यथा{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          प्रतिज्ञा( मेलनम्_Other<0x80 );  /* '[' is a single-byte character */
          यावत्( *zString ){
            पूर्णाङ्क b_मेलनम् = आदर्शः_Compare(&z_आदर्शः[-1],zString,pInfo,मेलनम्_Other);
            यदि( b_मेलनम्!=SQLITE_NOMATCH ) प्रत्यागम b_मेलनम्;
            SQLITE_SKIP_UTF8(zString);
          }
          प्रत्यागम SQLITE_NOWILDCARDMATCH;
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
      यदि( c<0x80 ){
        वर्ण zStop[3];
        पूर्णाङ्क b_मेलनम्;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = z_आदर्शः[0], ny = nx;
        यदि( nx>=0x80 || nx==मेलनम्_All || nx==मेलनम्_One || nx==मेलनम्_Other ){
          nx = 0;
        }
        यदि( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }अन्यथा{
          zStop[0] = c;
          zStop[1] = 0;
        }
        यावत्(1){
          zString += strcspn((ध्रुव वर्ण*)zString, zStop);
          यदि( zString[0]==0 ) भङ्ग;
          zString++;
          यदि( nx && zString[0]!=nx && zString[0]!=ny ) अनुवर्त;
          b_मेलनम् = आदर्शः_Compare(z_आदर्शः,zString,pInfo,मेलनम्_Other);
          यदि( b_मेलनम्!=SQLITE_NOMATCH ) प्रत्यागम b_मेलनम्;
        }
      }अन्यथा{
        पूर्णाङ्क b_मेलनम्;
        यावत्( (c2 = Utf8Read(zString))!=0 ){
          यदि( c2!=c ) अनुवर्त;
          b_मेलनम् = आदर्शः_Compare(z_आदर्शः,zString,pInfo,मेलनम्_Other);
          यदि( b_मेलनम्!=SQLITE_NOMATCH ) प्रत्यागम b_मेलनम्;
        }
      }
      प्रत्यागम SQLITE_NOWILDCARDMATCH;
    }
    यदि( c==मेलनम्_Other ){
      यदि( pInfo->मेलनम्_समुच्चयः==0 ){
        c = sqlite3Utf8Read(&z_आदर्शः);
        यदि( c==0 ) प्रत्यागम SQLITE_NOMATCH;
        zEscaped = z_आदर्शः;
      }अन्यथा{
        u32 प्राक्_c = 0;
        पूर्णाङ्क आलोकितम् = 0;
        पूर्णाङ्क invert = 0;
        c = sqlite3Utf8Read(&zString);
        यदि( c==0 ) प्रत्यागम SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&z_आदर्शः);
        यदि( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&z_आदर्शः);
        }
        यदि( c2==']' ){
          यदि( c==']' ) आलोकितम् = 1;
          c2 = sqlite3Utf8Read(&z_आदर्शः);
        }
        यावत्( c2 && c2!=']' ){
          यदि( c2=='-' && z_आदर्शः[0]!=']' && z_आदर्शः[0]!=0 && प्राक्_c>0 ){
            c2 = sqlite3Utf8Read(&z_आदर्शः);
            यदि( c>=प्राक्_c && c<=c2 ) आलोकितम् = 1;
            प्राक्_c = 0;
          }अन्यथा{
            यदि( c==c2 ){
              आलोकितम् = 1;
            }
            प्राक्_c = c2;
          }
          c2 = sqlite3Utf8Read(&z_आदर्शः);
        }
        यदि( c2==0 || (आलोकितम् ^ invert)==0 ){
          प्रत्यागम SQLITE_NOMATCH;
        }
        अनुवर्त;
      }
    }
    c2 = Utf8Read(zString);
    यदि( c==c2 ) अनुवर्त;
    यदि( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      अनुवर्त;
    }
    यदि( c==मेलनम्_One && z_आदर्शः!=zEscaped && c2!=0 ) अनुवर्त;
    प्रत्यागम SQLITE_NOMATCH;
  }
  प्रत्यागम *zString==0 ? SQLITE_मेलनम् : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
पूर्णाङ्क sqlite3_strglob(ध्रुव वर्ण *z_Glob_आदर्शः, ध्रुव वर्ण *zString){
  यदि( zString==0 ){
    प्रत्यागम z_Glob_आदर्शः!=0;
  }अन्यथा यदि( z_Glob_आदर्शः==0 ){
    प्रत्यागम 1;
  }अन्यथा {
    प्रत्यागम आदर्शः_Compare((u8*)z_Glob_आदर्शः, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
पूर्णाङ्क sqlite3_strlike(ध्रुव वर्ण *z_आदर्शः, ध्रुव वर्ण *zStr, अचिह्नित पूर्णाङ्क esc){
  यदि( zStr==0 ){
    प्रत्यागम z_आदर्शः!=0;
  }अन्यथा यदि( z_आदर्शः==0 ){
    प्रत्यागम 1;
  }अन्यथा{
    प्रत्यागम आदर्शः_Compare((u8*)z_आदर्शः, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#यदि_परिभाषितम् SQLITE_TEST
पूर्णाङ्क sqlite3_like_गणनम् = 0;
#यदि_अन्तः


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
स्थिर शून्य likeFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव अचिह्नित वर्ण *zA, *zB;
  u32 escape;
  पूर्णाङ्क nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  संरचना compareInfo *pInfo = sqlite3_user_दत्त(context);
  संरचना compareInfo backupInfo;

#यदि_परिभाषितम् SQLITE_LIKE_DOESNT_मेलनम्_BLOBS
  यदि( sqlite3_मानम्_type(argv[0])==SQLITE_BLOB
   || sqlite3_मानम्_type(argv[1])==SQLITE_BLOB
  ){
#यदि_परिभाषितम् SQLITE_TEST
    sqlite3_like_गणनम्++;
#यदि_अन्तः
    sqlite3_परिणाम_int(context, 0);
    प्रत्यागम;
  }
#यदि_अन्तः

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_मानम्_अष्टकानि(argv[0]);
  testcase( nPat==db->a_सीमा[SQLITE_सीमा_LIKE_आदर्शः_दीर्घता] );
  testcase( nPat==db->a_सीमा[SQLITE_सीमा_LIKE_आदर्शः_दीर्घता]+1 );
  यदि( nPat > db->a_सीमा[SQLITE_सीमा_LIKE_आदर्शः_दीर्घता] ){
    sqlite3_परिणाम_error(context, "LIKE or GLOB pattern too complex", -1);
    प्रत्यागम;
  }
  यदि( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    ध्रुव अचिह्नित वर्ण *zEsc = sqlite3_मानम्_text(argv[2]);
    यदि( zEsc==0 ) प्रत्यागम;
    यदि( sqlite3Utf8CharLen((वर्ण*)zEsc, -1)!=1 ){
      sqlite3_परिणाम_error(context,
          "ESCAPE expression must be a single character", -1);
      प्रत्यागम;
    }
    escape = sqlite3Utf8Read(&zEsc);
    यदि( escape==pInfo->मेलनम्_All || escape==pInfo->मेलनम्_One ){
      स्मृतिं_प्रतिलिख(&backupInfo, pInfo, परिमाण(backupInfo));
      pInfo = &backupInfo;
      यदि( escape==pInfo->मेलनम्_All ) pInfo->मेलनम्_All = 0;
      यदि( escape==pInfo->मेलनम्_One ) pInfo->मेलनम्_One = 0;
    }
  }अन्यथा{
    escape = pInfo->मेलनम्_समुच्चयः;
  }
  zB = sqlite3_मानम्_text(argv[0]);
  zA = sqlite3_मानम्_text(argv[1]);
  यदि( zA && zB ){
#यदि_परिभाषितम् SQLITE_TEST
    sqlite3_like_गणनम्++;
#यदि_अन्तः
    sqlite3_परिणाम_int(context,
                      आदर्शः_Compare(zB, zA, pInfo, escape)==SQLITE_मेलनम्);
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
स्थिर शून्य nullifFunc(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  यदि( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_परिणाम_मानम्(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
स्थिर शून्य संस्करणम्_Func(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **NotUsed2
){
  अप्रयुक्तम्_PARAMETER2(Not_प्रयुक्तम्, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_परिणाम_text(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
स्थिर शून्य sourceidFunc(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **NotUsed2
){
  अप्रयुक्तम्_PARAMETER2(Not_प्रयुक्तम्, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_परिणाम_text(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
स्थिर शून्य errlogFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  अप्रयुक्तम्_PARAMETER(argc);
  अप्रयुक्तम्_PARAMETER(context);
  sqlite3_log(sqlite3_मानम्_int(argv[0]), "%s", sqlite3_मानम्_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_COMPILEOPTION_DIAGS
स्थिर शून्य compileoptionusedFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव वर्ण *zOptName;
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  यदि( (zOptName = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]))!=0 ){
    sqlite3_परिणाम_int(context, sqlite3_compileoption_प्रयुक्तम्(zOptName));
  }
}
#यदि_अन्तः /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function.
** The result is a string that identifies the compiler options
** used to build SQLite.
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_COMPILEOPTION_DIAGS
स्थिर शून्य compileoptiongetFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  i64 n;
  ध्रुव वर्ण *zRes;
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_मानम्_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((पूर्णाङ्क)n) : 0;
  sqlite3_परिणाम_text(context, zRes, -1, SQLITE_STATIC);
}
#यदि_अन्तः /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
स्थिर ध्रुव वर्ण hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
शून्य sqlite3_Quote_मानम्(StrAccum *pStr, sqlite3_मानम् *p_मानम्, पूर्णाङ्क bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  प्रतिज्ञा( pStr!=0 && pStr->nChar==0 );

  विकल्प( sqlite3_मानम्_type(p_मानम्) ){
    पक्ष SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_मानम्_double(p_मानम्));
      भङ्ग;
    }
    पक्ष SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_मानम्_int64(p_मानम्));
      भङ्ग;
    }
    पक्ष SQLITE_BLOB: {
      वर्ण ध्रुव *zBlob = sqlite3_मानम्_blob(p_मानम्);
      i64 nBlob = sqlite3_मानम्_अष्टकानि(p_मानम्);
      प्रतिज्ञा( zBlob==sqlite3_मानम्_blob(p_मानम्) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      यदि( pStr->accError==0 ){
        वर्ण *zText = pStr->zText;
        पूर्णाङ्क i;
        प्रति(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      भङ्ग;
    }
    पक्ष SQLITE_TEXT: {
      ध्रुव अचिह्नित वर्ण *zArg = sqlite3_मानम्_text(p_मानम्);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      भङ्ग;
    }
    स्वभाव: {
      प्रतिज्ञा( sqlite3_मानम्_type(p_मानम्)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      भङ्ग;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
स्थिर पूर्णाङ्क isNHex(ध्रुव वर्ण *z, पूर्णाङ्क N, u32 *pVal){
  पूर्णाङ्क i;
  u32 v = 0;
  प्रति(i=0; i<N; i++){
    यदि( !sqlite3Isxdigit(z[i]) ) प्रत्यागम 0;
    v = (v<<4) + sqlite3_Hex_गन्तव्यम्_Int(z[i]);
  }
  *pVal = v;
  प्रत्यागम 1;
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
स्थिर शून्य unistrFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  वर्ण *z_बहिर्गमनम्;
  ध्रुव वर्ण *z_अन्तर्;
  पूर्णाङ्क n_अन्तर्;
  पूर्णाङ्क i, j, n;
  u32 v;

  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER( argc );
  z_अन्तर् = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  यदि( z_अन्तर्==0 ) प्रत्यागम;
  n_अन्तर् = sqlite3_मानम्_अष्टकानि(argv[0]);
  z_बहिर्गमनम् = sqlite3_malloc64(n_अन्तर्+1);
  यदि( z_बहिर्गमनम्==0 ){
    sqlite3_परिणाम_error_nomem(context);
    प्रत्यागम;
  }
  i = j = 0;
  यावत्( i<n_अन्तर् ){
    ध्रुव वर्ण *z = strchr(&z_अन्तर्[i],'\\');
    यदि( z==0 ){
      n = n_अन्तर् - i;
      memmove(&z_बहिर्गमनम्[j], &z_अन्तर्[i], n);
      j += n;
      भङ्ग;
    }
    n = z - &z_अन्तर्[i];
    यदि( n>0 ){
      memmove(&z_बहिर्गमनम्[j], &z_अन्तर्[i], n);
      j += n;
      i += n;
    }
    यदि( z_अन्तर्[i+1]=='\\' ){
      i += 2;
      z_बहिर्गमनम्[j++] = '\\';
    }अन्यथा यदि( sqlite3Isxdigit(z_अन्तर्[i+1]) ){
      यदि( !isNHex(&z_अन्तर्[i+1], 4, &v) ) गच्छ unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&z_बहिर्गमनम्[j], v);
    }अन्यथा यदि( z_अन्तर्[i+1]=='+' ){
      यदि( !isNHex(&z_अन्तर्[i+2], 6, &v) ) गच्छ unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&z_बहिर्गमनम्[j], v);
    }अन्यथा यदि( z_अन्तर्[i+1]=='u' ){
      यदि( !isNHex(&z_अन्तर्[i+2], 4, &v) ) गच्छ unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&z_बहिर्गमनम्[j], v);
    }अन्यथा यदि( z_अन्तर्[i+1]=='U' ){
      यदि( !isNHex(&z_अन्तर्[i+2], 8, &v) ) गच्छ unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&z_बहिर्गमनम्[j], v);
    }अन्यथा{
      गच्छ unistr_error;
    }
  }
  z_बहिर्गमनम्[j] = 0;
  sqlite3_परिणाम_text64(context, z_बहिर्गमनम्, j, sqlite3_free, SQLITE_UTF8_ZT);
  प्रत्यागम;

unistr_error:
  sqlite3_free(z_बहिर्गमनम्);
  sqlite3_परिणाम_error(context, "invalid Unicode escape", -1);
  प्रत्यागम;
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
स्थिर शून्य quoteFunc(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->a_सीमा[SQLITE_सीमा_दीर्घता]);
  sqlite3_Quote_मानम्(&str,argv[0],SQLITE_PTR_गन्तव्यम्_INT(sqlite3_user_दत्त(context)));
  sqlite3_परिणाम_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
स्थिर शून्य unicodeFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव अचिह्नित वर्ण *z = sqlite3_मानम्_text(argv[0]);
  (शून्य)argc;
  यदि( z && z[0] ) sqlite3_परिणाम_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
स्थिर शून्य charFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  अचिह्नित वर्ण *z, *z_बहिर्गमनम्;
  पूर्णाङ्क i;
  z_बहिर्गमनम् = z = sqlite3_malloc64( argc*4+1 );
  यदि( z==0 ){
    sqlite3_परिणाम_error_nomem(context);
    प्रत्यागम;
  }
  प्रति(i=0; i<argc; i++){
    sqlite3_int64 x;
    अचिह्नित c;
    x = sqlite3_मानम्_int64(argv[i]);
    यदि( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (अचिह्नित)(x & 0x1fffff);
    यदि( c<0x00080 ){
      *z_बहिर्गमनम्++ = (u8)(c&0xFF);
    }अन्यथा यदि( c<0x00800 ){
      *z_बहिर्गमनम्++ = 0xC0 + (u8)((c>>6)&0x1F);
      *z_बहिर्गमनम्++ = 0x80 + (u8)(c & 0x3F);
    }अन्यथा यदि( c<0x10000 ){
      *z_बहिर्गमनम्++ = 0xE0 + (u8)((c>>12)&0x0F);
      *z_बहिर्गमनम्++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_बहिर्गमनम्++ = 0x80 + (u8)(c & 0x3F);
    }अन्यथा{
      *z_बहिर्गमनम्++ = 0xF0 + (u8)((c>>18) & 0x07);
      *z_बहिर्गमनम्++ = 0x80 + (u8)((c>>12) & 0x3F);
      *z_बहिर्गमनम्++ = 0x80 + (u8)((c>>6) & 0x3F);
      *z_बहिर्गमनम्++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *z_बहिर्गमनम् = 0;
  sqlite3_परिणाम_text64(context, (वर्ण*)z, z_बहिर्गमनम्-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
स्थिर शून्य hexFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क i, n;
  ध्रुव अचिह्नित वर्ण *pBlob;
  वर्ण *zHex, *z;
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  pBlob = sqlite3_मानम्_blob(argv[0]);
  n = sqlite3_मानम्_अष्टकानि(argv[0]);
  प्रतिज्ञा( pBlob==sqlite3_मानम्_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  यदि( zHex ){
    प्रति(i=0; i<n; i++, pBlob++){
      अचिह्नित वर्ण c = *pBlob;
      *(z++) = hexdigits[(c>>4)&0xf];
      *(z++) = hexdigits[c&0xf];
    }
    *z = 0;
    sqlite3_परिणाम_text64(context, zHex, (u64)(z-zHex),
                          sqlite3_free, SQLITE_UTF8_ZT);
  }
}

/*
** Buffer zStr contains nStr bytes of utf-8 encoded text. Return 1 if zStr
** contains character ch, or 0 if it does not.
*/
स्थिर पूर्णाङ्क str_अन्तर्भवति_Char(ध्रुव u8 *zStr, पूर्णाङ्क nStr, u32 ch){
  ध्रुव u8 *z_अन्तः = &zStr[nStr];
  ध्रुव u8 *z = zStr;
  यावत्( z<z_अन्तः ){
    u32 tst = Utf8Read(z);
    यदि( tst==ch ) प्रत्यागम 1;
  }
  प्रत्यागम 0;
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
स्थिर शून्य unhexFunc(
  sqlite3_context *pCtx,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव u8 *zPass = (ध्रुव u8*)"";
  पूर्णाङ्क nPass = 0;
  ध्रुव u8 *zHex = sqlite3_मानम्_text(argv[0]);
  पूर्णाङ्क nHex = sqlite3_मानम्_अष्टकानि(argv[0]);
#यदि_परिभाषितम् SQLITE_DEBUG
  ध्रुव u8 *z_अन्तः = zHex ? &zHex[nHex] : 0;
#यदि_अन्तः
  u8 *pBlob = 0;
  u8 *p = 0;

  प्रतिज्ञा( argc==1 || argc==2 );
  यदि( argc==2 ){
    zPass = sqlite3_मानम्_text(argv[1]);
    nPass = sqlite3_मानम्_अष्टकानि(argv[1]);
  }
  यदि( !zHex || !zPass ) प्रत्यागम;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  यदि( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    यावत्( (c = *zHex)!=0x00 ){
      यावत्( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        प्रतिज्ञा( zHex<=z_अन्तः );
        यदि( !str_अन्तर्भवति_Char(zPass, nPass, ch) ) गच्छ unhex_null;
        c = *zHex;
        यदि( c==0x00 ) गच्छ unhex_समाप्तम्;
      }
      zHex++;
      प्रतिज्ञा( *z_अन्तः==0x00 );
      प्रतिज्ञा( zHex<=z_अन्तः );
      d = *(zHex++);
      यदि( !sqlite3Isxdigit(d) ) गच्छ unhex_null;
      *(p++) = (sqlite3_Hex_गन्तव्यम्_Int(c)<<4) | sqlite3_Hex_गन्तव्यम्_Int(d);
    }
  }

 unhex_समाप्तम्:
  sqlite3_परिणाम_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  प्रत्यागम;

 unhex_null:
  sqlite3_free(pBlob);
  प्रत्यागम;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
स्थिर शून्य zeroblobFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  i64 n;
  पूर्णाङ्क rc;
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  n = sqlite3_मानम्_int64(argv[0]);
  यदि( n<0 ) n = 0;
  rc = sqlite3_परिणाम_zeroblob64(context, n); /* IMP: R-00293-64994 */
  यदि( rc ){
    sqlite3_परिणाम_error_संहिता(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
स्थिर शून्य replaceFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव अचिह्नित वर्ण *zStr;        /* The input string A */
  ध्रुव अचिह्नित वर्ण *z_आदर्शः;    /* The pattern string B */
  ध्रुव अचिह्नित वर्ण *zRep;        /* The replacement string C */
  अचिह्नित वर्ण *z_बहिर्गमनम्;              /* The output */
  पूर्णाङ्क nStr;                /* Size of zStr */
  पूर्णाङ्क n_आदर्शः;            /* Size of zPattern */
  पूर्णाङ्क nRep;                /* Size of zRep */
  i64 n_बहिर्गमनम्;                /* Maximum size of zOut */
  पूर्णाङ्क loop_सीमा;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  अचिह्नित cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  प्रतिज्ञा( argc==3 );
  अप्रयुक्तम्_PARAMETER(argc);
  zStr = sqlite3_मानम्_text(argv[0]);
  यदि( zStr==0 ) प्रत्यागम;
  nStr = sqlite3_मानम्_अष्टकानि(argv[0]);
  प्रतिज्ञा( zStr==sqlite3_मानम्_text(argv[0]) );  /* No encoding change */
  z_आदर्शः = sqlite3_मानम्_text(argv[1]);
  यदि( z_आदर्शः==0 ){
    प्रतिज्ञा( sqlite3_मानम्_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    प्रत्यागम;
  }
  यदि( z_आदर्शः[0]==0 ){
    प्रतिज्ञा( sqlite3_मानम्_type(argv[1])!=SQLITE_NULL );
    sqlite3_परिणाम_text(context, (ध्रुव वर्ण*)zStr, nStr, SQLITE_TRANSIENT);
    प्रत्यागम;
  }
  n_आदर्शः = sqlite3_मानम्_अष्टकानि(argv[1]);
  प्रतिज्ञा( z_आदर्शः==sqlite3_मानम्_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_मानम्_text(argv[2]);
  यदि( zRep==0 ) प्रत्यागम;
  nRep = sqlite3_मानम्_अष्टकानि(argv[2]);
  प्रतिज्ञा( zRep==sqlite3_मानम्_text(argv[2]) );
  n_बहिर्गमनम् = nStr + 1;
  प्रतिज्ञा( n_बहिर्गमनम्<SQLITE_MAX_दीर्घता );
  z_बहिर्गमनम् = contextMalloc(context, n_बहिर्गमनम्);
  यदि( z_बहिर्गमनम्==0 ){
    प्रत्यागम;
  }
  loop_सीमा = nStr - n_आदर्शः; 
  cntExpand = 0;
  प्रति(i=j=0; i<=loop_सीमा; i++){
    यदि( zStr[i]!=z_आदर्शः[0] || memcmp(&zStr[i], z_आदर्शः, n_आदर्शः) ){
      z_बहिर्गमनम्[j++] = zStr[i];
    }अन्यथा{
      यदि( nRep>n_आदर्शः ){
        n_बहिर्गमनम् += nRep - n_आदर्शः;
        testcase( n_बहिर्गमनम्-1==db->a_सीमा[SQLITE_सीमा_दीर्घता] );
        testcase( n_बहिर्गमनम्-2==db->a_सीमा[SQLITE_सीमा_दीर्घता] );
        यदि( n_बहिर्गमनम्-1>db->a_सीमा[SQLITE_सीमा_दीर्घता] ){
          sqlite3_परिणाम_error_toobig(context);
          sqlite3_free(z_बहिर्गमनम्);
          प्रत्यागम;
        }
        cntExpand++;
        यदि( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = z_बहिर्गमनम्;
          z_बहिर्गमनम् = sqlite3Realloc(z_बहिर्गमनम्, (पूर्णाङ्क)n_बहिर्गमनम् + (n_बहिर्गमनम् - nStr - 1));
          यदि( z_बहिर्गमनम्==0 ){
            sqlite3_परिणाम_error_nomem(context);
            sqlite3_free(zOld);
            प्रत्यागम;
          }
        }
      }
      स्मृतिं_प्रतिलिख(&z_बहिर्गमनम्[j], zRep, nRep);
      j += nRep;
      i += n_आदर्शः-1;
    }
  }
  प्रतिज्ञा( j+nStr-i+1<=n_बहिर्गमनम् );
  स्मृतिं_प्रतिलिख(&z_बहिर्गमनम्[j], &zStr[i], nStr-i);
  j += nStr - i;
  प्रतिज्ञा( j<=n_बहिर्गमनम् );
  z_बहिर्गमनम्[j] = 0;
  sqlite3_परिणाम_text(context, (वर्ण*)z_बहिर्गमनम्, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
स्थिर शून्य trimFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव अचिह्नित वर्ण *z_अन्तर्;         /* Input string */
  ध्रुव अचिह्नित वर्ण *z_Char_समुच्चयः;    /* Set of characters to trim */
  अचिह्नित पूर्णाङ्क n_अन्तर्;                 /* Number of bytes in input */
  पूर्णाङ्क flags;                        /* 1: trimleft  2: trimright  3: trim */
  पूर्णाङ्क i;                            /* Loop counter */
  अचिह्नित पूर्णाङ्क *aLen = 0;           /* Length of each character in zCharSet */
  अचिह्नित वर्ण **azChar = 0;       /* Individual characters in zCharSet */
  पूर्णाङ्क nChar;                        /* Number of characters in zCharSet */

  यदि( sqlite3_मानम्_type(argv[0])==SQLITE_NULL ){
    प्रत्यागम;
  }
  z_अन्तर् = sqlite3_मानम्_text(argv[0]);
  यदि( z_अन्तर्==0 ) प्रत्यागम;
  n_अन्तर् = (अचिह्नित)sqlite3_मानम्_अष्टकानि(argv[0]);
  प्रतिज्ञा( z_अन्तर्==sqlite3_मानम्_text(argv[0]) );
  यदि( argc==1 ){
    स्थिर ध्रुव अचिह्नित lenOne[] = { 1 };
    स्थिर अचिह्नित वर्ण * ध्रुव azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (अचिह्नित*)lenOne;
    azChar = (अचिह्नित वर्ण **)azOne;
    z_Char_समुच्चयः = 0;
  }अन्यथा यदि( (z_Char_समुच्चयः = sqlite3_मानम्_text(argv[1]))==0 ){
    प्रत्यागम;
  }अन्यथा{
    ध्रुव अचिह्नित वर्ण *z;
    प्रति(z=z_Char_समुच्चयः, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    यदि( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(परिमाण(वर्ण*)+परिमाण(अचिह्नित)));
      यदि( azChar==0 ){
        प्रत्यागम;
      }
      aLen = (अचिह्नित*)&azChar[nChar];
      प्रति(z=z_Char_समुच्चयः, nChar=0; *z; nChar++){
        azChar[nChar] = (अचिह्नित वर्ण *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (अचिह्नित)(z - azChar[nChar]);
      }
    }
  }
  यदि( nChar>0 ){
    flags = SQLITE_PTR_गन्तव्यम्_INT(sqlite3_user_दत्त(context));
    यदि( flags & 1 ){
      यावत्( n_अन्तर्>0 ){
        अचिह्नित पूर्णाङ्क len = 0;
        प्रति(i=0; i<nChar; i++){
          len = aLen[i];
          यदि( len<=n_अन्तर् && memcmp(z_अन्तर्, azChar[i], len)==0 ) भङ्ग;
        }
        यदि( i>=nChar ) भङ्ग;
        z_अन्तर् += len;
        n_अन्तर् -= len;
      }
    }
    यदि( flags & 2 ){
      यावत्( n_अन्तर्>0 ){
        अचिह्नित पूर्णाङ्क len = 0;
        प्रति(i=0; i<nChar; i++){
          len = aLen[i];
          यदि( len<=n_अन्तर् && memcmp(&z_अन्तर्[n_अन्तर्-len],azChar[i],len)==0 ) भङ्ग;
        }
        यदि( i>=nChar ) भङ्ग;
        n_अन्तर् -= len;
      }
    }
    यदि( z_Char_समुच्चयः ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_परिणाम_text(context, (वर्ण*)z_अन्तर्, n_अन्तर्, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
स्थिर शून्य concatFuncCore(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv,
  पूर्णाङ्क nSep,
  ध्रुव वर्ण *zSep
){
  i64 j, n = 0;
  पूर्णाङ्क i;
  पूर्णाङ्क bNotNull = 0;   /* True after at least NOT NULL argument seen */
  वर्ण *z;
  प्रति(i=0; i<argc; i++){
    n += sqlite3_मानम्_अष्टकानि(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  यदि( z==0 ){
    sqlite3_परिणाम_error_nomem(context);
    प्रत्यागम;
  }
  j = 0;
  प्रति(i=0; i<argc; i++){
    यदि( sqlite3_मानम्_type(argv[i])!=SQLITE_NULL ){
      पूर्णाङ्क k = sqlite3_मानम्_अष्टकानि(argv[i]);
      ध्रुव वर्ण *v = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[i]);
      यदि( v!=0 ){
        यदि( bNotNull && nSep>0 ){
          स्मृतिं_प्रतिलिख(&z[j], zSep, nSep);
          j += nSep;
        }
        स्मृतिं_प्रतिलिख(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;
  प्रतिज्ञा( j<=n );
  sqlite3_परिणाम_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
स्थिर शून्य concatFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
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
स्थिर शून्य concatwsFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क nSep = sqlite3_मानम्_अष्टकानि(argv[0]);
  ध्रुव वर्ण *zSep = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  यदि( zSep==0 ) प्रत्यागम;
  concatFuncCore(context, argc-1, argv+1, nSep, zSep);
}


#यदि_परिभाषितम् SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
/*
** The "unknown" function is automatically substituted in place of
** any unrecognized function name when doing an EXPLAIN or EXPLAIN QUERY PLAN
** when the SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION compile-time option is used.
** When the "sqlite3" command-line shell is built using this functionality,
** that allows an EXPLAIN or EXPLAIN QUERY PLAN for complex queries
** involving application-defined functions to be examined in a generic
** sqlite3 shell.
*/
स्थिर शून्य unknownFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  /* no-op */
  (शून्य)context;
  (शून्य)argc;
  (शून्य)argv;
}
#यदि_अन्तः /*SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION*/


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#यदि_परिभाषितम् SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X.
*/
स्थिर शून्य soundexFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  वर्ण z_परिणाम[8];
  ध्रुव u8 *z_अन्तर्;
  पूर्णाङ्क i, j;
  स्थिर ध्रुव अचिह्नित वर्ण i_संहिता[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  प्रतिज्ञा( argc==1 );
  z_अन्तर् = (u8*)sqlite3_मानम्_text(argv[0]);
  यदि( z_अन्तर्==0 ) z_अन्तर् = (u8*)"";
  प्रति(i=0; z_अन्तर्[i] && !sqlite3Isalpha(z_अन्तर्[i]); i++){}
  यदि( z_अन्तर्[i] ){
    u8 prevcode = i_संहिता[z_अन्तर्[i]&0x7f];
    z_परिणाम[0] = sqlite3Toupper(z_अन्तर्[i]);
    प्रति(j=1; j<4 && z_अन्तर्[i]; i++){
      पूर्णाङ्क संहिता = i_संहिता[z_अन्तर्[i]&0x7f];
      यदि( संहिता>0 ){
        यदि( संहिता!=prevcode ){
          prevcode = संहिता;
          z_परिणाम[j++] = संहिता + '0';
        }
      }अन्यथा{
        prevcode = 0;
      }
    }
    यावत्( j<4 ){
      z_परिणाम[j++] = '0';
    }
    z_परिणाम[j] = 0;
    sqlite3_परिणाम_text(context, z_परिणाम, 4, SQLITE_TRANSIENT);
  }अन्यथा{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_परिणाम_text(context, "?000", 4, SQLITE_STATIC);
  }
}
#यदि_अन्तः /* SQLITE_SOUNDEX */

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
स्थिर शून्य oomFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#यदि_अन्तः /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
स्थिर शून्य आरोपय_Ext(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  ध्रुव वर्ण *zFile = (ध्रुव वर्ण *)sqlite3_मानम्_text(argv[0]);
  ध्रुव वर्ण *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  वर्ण *z_Err_सन्देशः = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  यदि( (db->flags & SQLITE_आरोपय_Ext_Func)==0 ){
    sqlite3_परिणाम_error(context, "not authorized", -1);
    प्रत्यागम;
  }

  यदि( argc==2 ){
    zProc = (ध्रुव वर्ण *)sqlite3_मानम्_text(argv[1]);
  }अन्यथा{
    zProc = 0;
  }
  यदि( zFile && sqlite3_आरोपय_extension(db, zFile, zProc, &z_Err_सन्देशः) ){
    sqlite3_परिणाम_error(context, z_Err_सन्देशः, -1);
    sqlite3_free(z_Err_सन्देशः);
  }
}
#यदि_अन्तः


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
प्रकारनिर्देश संरचना योग_Ctx योग_Ctx;
संरचना योग_Ctx {
  द्विगुण r_योग;      /* Running sum as as a double */
  द्विगुण rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
  i64 i_योग;         /* Running sum as a signed integer */
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
स्थिर शून्य kahan_Babuska_Neumaier_चरणः(
  चञ्चल योग_Ctx *p_योग,
  चञ्चल द्विगुण r
){
  चञ्चल द्विगुण s = p_योग->r_योग;
  चञ्चल द्विगुण t = s + r;
  यदि( fabs(s) > fabs(r) ){
    p_योग->rErr += (s - t) + r;
  }अन्यथा{
    p_योग->rErr += (r - t) + s;
  }
  p_योग->r_योग = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
स्थिर शून्य kahan_Babuska_Neumaier_चरणः_Int64(चञ्चल योग_Ctx *p_योग, i64 iVal){
  यदि( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahan_Babuska_Neumaier_चरणः(p_योग, iBig);
    kahan_Babuska_Neumaier_चरणः(p_योग, iSm);
  }अन्यथा{
    kahan_Babuska_Neumaier_चरणः(p_योग, (द्विगुण)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
स्थिर शून्य kahanBabuskaNeumaierInit(
  चञ्चल योग_Ctx *p,
  i64 iVal
){
  यदि( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->r_योग = (द्विगुण)(iVal - iSm);
    p->rErr = (द्विगुण)iSm;
  }अन्यथा{
    p->r_योग = (द्विगुण)iVal;
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
स्थिर शून्य योग_चरणः(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  योग_Ctx *p;
  पूर्णाङ्क type;
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, परिमाण(*p));
  type = sqlite3_मानम्_numeric_type(argv[0]);
  यदि( p && type!=SQLITE_NULL ){
    p->cnt++;
    यदि( p->approx==0 ){
      यदि( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->i_योग);
        p->approx = 1;
        kahan_Babuska_Neumaier_चरणः(p, sqlite3_मानम्_double(argv[0]));
      }अन्यथा{
        i64 x = p->i_योग;
        यदि( sqlite3_सङ्कलय_Int64(&x, sqlite3_मानम्_int64(argv[0]))==0 ){
          p->i_योग = x;
        }अन्यथा{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->i_योग);
          p->approx = 1;
          kahan_Babuska_Neumaier_चरणः_Int64(p, sqlite3_मानम्_int64(argv[0]));
        }
      }
    }अन्यथा{
      यदि( type==SQLITE_INTEGER ){
        kahan_Babuska_Neumaier_चरणः_Int64(p, sqlite3_मानम्_int64(argv[0]));
      }अन्यथा{
        p->ovrfl = 0;
        kahan_Babuska_Neumaier_चरणः(p, sqlite3_मानम्_double(argv[0]));
      }
    }
  }
}
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
स्थिर शून्य योग_Inverse(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम्**argv){
  योग_Ctx *p;
  पूर्णाङ्क type;
  प्रतिज्ञा( argc==1 );
  अप्रयुक्तम्_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, परिमाण(*p));
  type = sqlite3_मानम्_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  यदि( ALWAYS(p) && type!=SQLITE_NULL ){
    प्रतिज्ञा( p->cnt>0 );
    p->cnt--;
    यदि( !p->approx ){
      i64 x = p->i_योग;
      यदि( sqlite3SubInt64(&x, sqlite3_मानम्_int64(argv[0]))==0 ){
        p->i_योग = x;
        प्रत्यागम;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->i_योग);
    }
    यदि( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_मानम्_int64(argv[0]);
      यदि( iVal!=SMALLEST_INT64 ){
        kahan_Babuska_Neumaier_चरणः_Int64(p, -iVal);
      }अन्यथा{
        kahan_Babuska_Neumaier_चरणः_Int64(p, LARGEST_INT64);
        kahan_Babuska_Neumaier_चरणः_Int64(p, 1);
      }       
    }अन्यथा{
      kahan_Babuska_Neumaier_चरणः(p, -sqlite3_मानम्_double(argv[0]));
    }
  }
}
#अन्यथा
# define योग_Inverse 0
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */
स्थिर शून्य योग_Finalize(sqlite3_context *context){
  योग_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  यदि( p && p->cnt>0 ){
    यदि( p->approx ){
      यदि( p->ovrfl ){
        sqlite3_परिणाम_error(context,"integer overflow",-1);
      }अन्यथा यदि( !sqlite3_Is_अतिप्रवाहः(p->rErr) ){
        sqlite3_परिणाम_double(context, p->r_योग+p->rErr);
      }अन्यथा{
        sqlite3_परिणाम_double(context, p->r_योग);
      }
    }अन्यथा{
      sqlite3_परिणाम_int64(context, p->i_योग);
    }
  }
}
स्थिर शून्य avgFinalize(sqlite3_context *context){
  योग_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  यदि( p && p->cnt>0 ){
    द्विगुण r;
    यदि( p->approx ){
      r = p->r_योग;
      यदि( !sqlite3_Is_अतिप्रवाहः(p->rErr) ) r += p->rErr;
    }अन्यथा{
      r = (द्विगुण)(p->i_योग);
    }
    sqlite3_परिणाम_double(context, r/(द्विगुण)p->cnt);
  }
}
स्थिर शून्य योग_Finalize_2(sqlite3_context *context){
  योग_Ctx *p;
  द्विगुण r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  यदि( p ){
    यदि( p->approx ){
      r = p->r_योग;
      यदि( !sqlite3_Is_अतिप्रवाहः(p->rErr) ) r += p->rErr;
    }अन्यथा{
      r = (द्विगुण)(p->i_योग);
    }
  }
  sqlite3_परिणाम_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
प्रकारनिर्देश संरचना गणनम्_Ctx गणनम्_Ctx;
संरचना गणनम्_Ctx {
  i64 n;
#यदि_परिभाषितम् SQLITE_DEBUG
  पूर्णाङ्क bInverse;                   /* True if xInverse() ever called */
#यदि_अन्तः
};

/*
** Routines to implement the count() aggregate function.
*/
स्थिर शून्य गणनम्_चरणः(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  गणनम्_Ctx *p;
  p = sqlite3_aggregate_context(context, परिमाण(*p));
  यदि( (argc==0 || SQLITE_NULL!=sqlite3_मानम्_type(argv[0])) && p ){
    p->n++;
  }

#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  प्रतिज्ञा( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_गणनम्(context) );
#यदि_अन्तः
}  
स्थिर शून्य गणनम्_Finalize(sqlite3_context *context){
  गणनम्_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_परिणाम_int64(context, p ? p->n : 0);
}
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
स्थिर शून्य गणनम्_Inverse(sqlite3_context *ctx, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  गणनम्_Ctx *p;
  p = sqlite3_aggregate_context(ctx, परिमाण(*p));
  /* p is always non-NULL since countStep() will have been called first */
  यदि( (argc==0 || SQLITE_NULL!=sqlite3_मानम्_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#यदि_परिभाषितम् SQLITE_DEBUG
    p->bInverse = 1;
#यदि_अन्तः
  }
}  
#अन्यथा
# define गणनम्_Inverse 0
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
स्थिर शून्य minmax_चरणः(
  sqlite3_context *context,
  पूर्णाङ्क Not_प्रयुक्तम्,
  sqlite3_मानम् **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);

  pBest = (Mem *)sqlite3_aggregate_context(context, परिमाण(*pBest));
  यदि( !pBest ) प्रत्यागम;

  यदि( sqlite3_मानम्_type(pArg)==SQLITE_NULL ){
    यदि( pBest->flags ) sqlite3_Skip_सञ्चायकः_आरोपय(context);
  }अन्यथा यदि( pBest->flags ){
    पूर्णाङ्क max;
    पूर्णाङ्क cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);
    /* This step function is used for both the min() and max() aggregates,
    ** the only difference between the two being that the sense of the
    ** comparison is inverted. For the max() aggregate, the
    ** sqlite3_user_data() function returns (void *)-1. For min() it
    ** returns (void *)db, where db is the sqlite3* database pointer.
    ** Therefore the next statement sets variable 'max' to 1 for the max()
    ** aggregate, or 0 for min().
    */
    max = sqlite3_user_दत्त(context)!=0;
    cmp = sqlite3MemCompare(pBest, pArg, pColl);
    यदि( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }अन्यथा{
      sqlite3_Skip_सञ्चायकः_आरोपय(context);
    }
  }अन्यथा{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
स्थिर शून्य min_Max_मानम्_Finalize(sqlite3_context *context, पूर्णाङ्क b_मानम्){
  sqlite3_मानम् *pRes;
  pRes = (sqlite3_मानम् *)sqlite3_aggregate_context(context, 0);
  यदि( pRes ){
    यदि( pRes->flags ){
      sqlite3_परिणाम_मानम्(context, pRes);
    }
    यदि( b_मानम्==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
स्थिर शून्य min_Max_मानम्(sqlite3_context *context){
  min_Max_मानम्_Finalize(context, 1);
}
#अन्यथा
# define min_Max_मानम् 0
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */
स्थिर शून्य minMaxFinalize(sqlite3_context *context){
  min_Max_मानम्_Finalize(context, 0);
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
प्रकारनिर्देश संरचना {
  StrAccum str;          /* The accumulated concatenation */
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  पूर्णाङ्क nAccum;            /* Number of strings presently concatenated */
  पूर्णाङ्क n_First_Sep_दीर्घता;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  पूर्णाङ्क *pnSepLengths;
#यदि_अन्तः
} गणः_Concat_Ctx;

स्थिर शून्य गणः_Concat_चरणः(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  ध्रुव वर्ण *zVal;
  गणः_Concat_Ctx *pGCC;
  ध्रुव वर्ण *zSep;
  पूर्णाङ्क nVal, nSep;
  प्रतिज्ञा( argc==1 || argc==2 );
  यदि( sqlite3_मानम्_type(argv[0])==SQLITE_NULL ) प्रत्यागम;
  pGCC = (गणः_Concat_Ctx*)sqlite3_aggregate_context(context, परिमाण(*pGCC));
  यदि( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    पूर्णाङ्क first_पदांशः = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->a_सीमा[SQLITE_सीमा_दीर्घता];
    यदि( argc==1 ){
      यदि( !first_पदांशः ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
      अन्यथा{
        pGCC->n_First_Sep_दीर्घता = 1;
      }
#यदि_अन्तः
    }अन्यथा यदि( !first_पदांशः ){
      zSep = (वर्ण*)sqlite3_मानम्_text(argv[1]);
      nSep = sqlite3_मानम्_अष्टकानि(argv[1]);
      यदि( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
      अन्यथा{
        nSep = 0;
      }
      यदि( nSep != pGCC->n_First_Sep_दीर्घता || pGCC->pnSepLengths != 0 ){
        पूर्णाङ्क *pnsl = pGCC->pnSepLengths;
        यदि( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (पूर्णाङ्क*)sqlite3_malloc64((pGCC->nAccum+1) * परिमाण(पूर्णाङ्क));
          यदि( pnsl!=0 ){
            पूर्णाङ्क i = 0, nA = pGCC->nAccum-1;
            यावत्( i<nA ) pnsl[i++] = pGCC->n_First_Sep_दीर्घता;
          }
        }अन्यथा{
          pnsl = (पूर्णाङ्क*)sqlite3_realloc64(pnsl, pGCC->nAccum * परिमाण(पूर्णाङ्क));
        }
        यदि( pnsl!=0 ){
          यदि( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }अन्यथा{
          sqlite3_Str_Accum_समुच्चयः_Error(&pGCC->str, SQLITE_NOMEM);
        }
      }
#यदि_अन्तः
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    अन्यथा{
      pGCC->n_First_Sep_दीर्घता = sqlite3_मानम्_अष्टकानि(argv[1]);
    }
    pGCC->nAccum += 1;
#यदि_अन्तः
    zVal = (वर्ण*)sqlite3_मानम्_text(argv[0]);
    nVal = sqlite3_मानम्_अष्टकानि(argv[0]);
    यदि( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
स्थिर शून्य गणः_Concat_Inverse(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  गणः_Concat_Ctx *pGCC;
  प्रतिज्ञा( argc==1 || argc==2 );
  (शून्य)argc;  /* Suppress unused parameter warning */
  यदि( sqlite3_मानम्_type(argv[0])==SQLITE_NULL ) प्रत्यागम;
  pGCC = (गणः_Concat_Ctx*)sqlite3_aggregate_context(context, परिमाण(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  यदि( ALWAYS(pGCC) ){
    पूर्णाङ्क nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (शून्य)sqlite3_मानम्_text(argv[0]);
    nVS = sqlite3_मानम्_अष्टकानि(argv[0]);
    pGCC->nAccum -= 1;
    यदि( pGCC->pnSepLengths!=0 ){
      प्रतिज्ञा(pGCC->nAccum >= 0);
      यदि( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        memmove(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*परिमाण(पूर्णाङ्क));
      }
    }अन्यथा{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->n_First_Sep_दीर्घता;
    }
    यदि( nVS>=(पूर्णाङ्क)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }अन्यथा{
      pGCC->str.nChar -= nVS;
      memmove(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    यदि( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#अन्यथा
# define गणः_Concat_Inverse 0
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */
स्थिर शून्य गणः_Concat_Finalize(sqlite3_context *context){
  गणः_Concat_Ctx *pGCC
    = (गणः_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  यदि( pGCC ){
    sqlite3_परिणाम_str(context, &pGCC->str, SQLITE_XFER);
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#यदि_अन्तः
  }
}
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
स्थिर शून्य गणः_Concat_मानम्(sqlite3_context *context){
  गणः_Concat_Ctx *pGCC
    = (गणः_Concat_Ctx*)sqlite3_aggregate_context(context, 0);
  यदि( pGCC && pGCC->nAccum>0 ){
    sqlite3_परिणाम_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#अन्यथा
# define गणः_Concat_मानम् 0
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
शून्य sqlite3_पञ्जिकाकोशः_प्रत्येकम्_Connection_Builtin_Functions(sqlite3 *db){
  पूर्णाङ्क rc = sqlite3_overload_function(db, "MATCH", 2);
  प्रतिज्ञा( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  यदि( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
शून्य sqlite3_पञ्जिकाकोशः_Like_Functions(sqlite3 *db, पूर्णाङ्क caseSensitive){
  FuncDef *pDef;
  संरचना compareInfo *pInfo;
  पूर्णाङ्क flags;
  पूर्णाङ्क nArg;
  यदि( caseSensitive ){
    pInfo = (संरचना compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }अन्यथा{
    pInfo = (संरचना compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  प्रति(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);
    प्रतिज्ञा( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
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
पूर्णाङ्क sqlite3IsLikeFunction(sqlite3 *db, व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क *pIsNocase, वर्ण *aWc){
  FuncDef *pDef;
  पूर्णाङ्क n_व्यञ्जकम्;
  प्रतिज्ञा( p_व्यञ्जकम्!=0 );
  प्रतिज्ञा( p_व्यञ्जकम्->op==TK_FUNCTION );
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
  यदि( !p_व्यञ्जकम्->x.pList ){
    प्रत्यागम 0;
  }
  n_व्यञ्जकम् = p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्;
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
  pDef = sqlite3FindFunction(db, p_व्यञ्जकम्->u.zToken, n_व्यञ्जकम्, SQLITE_UTF8, 0);
#यदि_परिभाषितम् SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  यदि( pDef==0 ) प्रत्यागम 0;
#यदि_अन्तः
  यदि( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    प्रत्यागम 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  स्मृतिं_प्रतिलिख(aWc, pDef->p_User_दत्त, 3);
  प्रतिज्ञा( (वर्ण*)&likeInfoAlt == (वर्ण*)&likeInfoAlt.मेलनम्_All );
  प्रतिज्ञा( &((वर्ण*)&likeInfoAlt)[1] == (वर्ण*)&likeInfoAlt.मेलनम्_One );
  प्रतिज्ञा( &((वर्ण*)&likeInfoAlt)[2] == (वर्ण*)&likeInfoAlt.मेलनम्_समुच्चयः );

  यदि( n_व्यञ्जकम्<3 ){
    aWc[3] = 0;
  }अन्यथा{
    व्यञ्जकम् *pEscape = p_व्यञ्जकम्->x.pList->a[2].p_व्यञ्जकम्;
    वर्ण *zEscape;
    यदि( pEscape->op!=TK_STRING ) प्रत्यागम 0;
    प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pEscape, EP_Int_मानम्) );
    zEscape = pEscape->u.zToken;
    यदि( zEscape[0]==0 || zEscape[1]!=0 ) प्रत्यागम 0;
    यदि( zEscape[0]==aWc[0] ) प्रत्यागम 0;
    यदि( zEscape[0]==aWc[1] ) प्रत्यागम 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  प्रत्यागम 1;
}

/* Mathematical Constants */
#यदि_अपरिभाषितम् M_PI
# define M_PI   3.141592653589793238462643383279502884
#यदि_अन्तः
#यदि_अपरिभाषितम् M_LN10
# define M_LN10 2.302585092994045684017991454684364208
#यदि_अन्तः
#यदि_अपरिभाषितम् M_LN2
# define M_LN2  0.693147180559945309417232121458176568
#यदि_अन्तः


/* Extra math functions that require linking with -lm
*/
#यदि_परिभाषितम् SQLITE_ENABLE_MATH_FUNCTIONS
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
स्थिर शून्य ceilingFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  प्रतिज्ञा( argc==1 );
  विकल्प( sqlite3_मानम्_numeric_type(argv[0]) ){
    पक्ष SQLITE_INTEGER: {
       sqlite3_परिणाम_int64(context, sqlite3_मानम्_int64(argv[0]));
       भङ्ग;
    }
    पक्ष SQLITE_FLOAT: {
       द्विगुण (*x)(द्विगुण) = (द्विगुण(*)(द्विगुण))sqlite3_user_दत्त(context);
       sqlite3_परिणाम_double(context, x(sqlite3_मानम्_double(argv[0])));
       भङ्ग;
    }
    स्वभाव: {
       भङ्ग;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
स्थिर द्विगुण xCeil(द्विगुण x){ प्रत्यागम ceil(x); }
स्थिर द्विगुण xFloor(द्विगुण x){ प्रत्यागम floor(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#यदि defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*लघुगणकः(X))
#यदि_अन्तः
#यदि defined(HAVE_LOG2) && HAVE_LOG2==0
# define log2(X) (1.442695040888963456*लघुगणकः(X))
#यदि_अन्तः


/*
** Implementation of SQL functions:
**
**   ln(X)       - natural logarithm
**   log(X)      - log X base 10
**   log10(X)    - log X base 10
**   log(B,X)    - log X base B
*/
स्थिर शून्य logFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  द्विगुण x, b, ans;
  प्रतिज्ञा( argc==1 || argc==2 );
  विकल्प( sqlite3_मानम्_numeric_type(argv[0]) ){
    पक्ष SQLITE_INTEGER:
    पक्ष SQLITE_FLOAT:
      x = sqlite3_मानम्_double(argv[0]);
      यदि( x<=0.0 ) प्रत्यागम;
      भङ्ग;
    स्वभाव:
      प्रत्यागम;
  }
  यदि( argc==2 ){
    विकल्प( sqlite3_मानम्_numeric_type(argv[1]) ){
      पक्ष SQLITE_INTEGER:
      पक्ष SQLITE_FLOAT:
        b = लघुगणकः(x);
        यदि( b<=0.0 ) प्रत्यागम;
        x = sqlite3_मानम्_double(argv[1]);
        यदि( x<=0.0 ) प्रत्यागम;
        भङ्ग;
     स्वभाव:
        प्रत्यागम;
    }
    ans = लघुगणकः(x)/b;
  }अन्यथा{
    विकल्प( SQLITE_PTR_गन्तव्यम्_INT(sqlite3_user_दत्त(context)) ){
      पक्ष 1:
        ans = log10(x);
        भङ्ग;
      पक्ष 2:
        ans = log2(x);
        भङ्ग;
      स्वभाव:
        ans = लघुगणकः(x);
        भङ्ग;
    }
  }
  sqlite3_परिणाम_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
स्थिर द्विगुण deg_गन्तव्यम्_Rad(द्विगुण x){ प्रत्यागम x*(M_PI/180.0); }
स्थिर द्विगुण rad_गन्तव्यम्_Deg(द्विगुण x){ प्रत्यागम x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
स्थिर शून्य math1Func(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क type0;
  द्विगुण v0, ans;
  द्विगुण (*x)(द्विगुण);
  प्रतिज्ञा( argc==1 );
  type0 = sqlite3_मानम्_numeric_type(argv[0]);
  यदि( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) प्रत्यागम;
  v0 = sqlite3_मानम्_double(argv[0]);
  x = (द्विगुण(*)(द्विगुण))sqlite3_user_दत्त(context);
  ans = x(v0);
  sqlite3_परिणाम_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
स्थिर शून्य math2Func(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क type0, type1;
  द्विगुण v0, v1, ans;
  द्विगुण (*x)(द्विगुण,द्विगुण);
  प्रतिज्ञा( argc==2 );
  type0 = sqlite3_मानम्_numeric_type(argv[0]);
  यदि( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) प्रत्यागम;
  type1 = sqlite3_मानम्_numeric_type(argv[1]);
  यदि( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) प्रत्यागम;
  v0 = sqlite3_मानम्_double(argv[0]);
  v1 = sqlite3_मानम्_double(argv[1]);
  x = (द्विगुण(*)(द्विगुण,द्विगुण))sqlite3_user_दत्त(context);
  ans = x(v0, v1);
  sqlite3_परिणाम_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
स्थिर शून्य piFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  प्रतिज्ञा( argc==0 );
  (शून्य)argv;
  sqlite3_परिणाम_double(context, M_PI);
}

#यदि_अन्तः /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
स्थिर शून्य signFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क type0;
  द्विगुण x;
  अप्रयुक्तम्_PARAMETER(argc);
  प्रतिज्ञा( argc==1 );
  type0 = sqlite3_मानम्_numeric_type(argv[0]);
  यदि( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) प्रत्यागम;
  x = sqlite3_मानम्_double(argv[0]);
  sqlite3_परिणाम_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#यदि defined(SQLITE_ENABLE_PERCENTILE)
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
प्रकारनिर्देश संरचना Percentile Percentile;
संरचना Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 n_प्रयुक्तम्;           /* Number of slots actually used in a[] */
  वर्ण bSorted;        /* True if a[] is already in sorted order */
  वर्ण bKeepSorted;    /* True if advantageous to keep a[] sorted */
  वर्ण bPctValid;      /* True if rPct is valid */
  द्विगुण rPct;         /* Fraction.  0.0 to 1.0 */
  द्विगुण *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
स्थिर पूर्णाङ्क percentIsInfinity(द्विगुण r){
  sqlite3_uint64 u;
  प्रतिज्ञा( परिमाण(u)==परिमाण(r) );
  स्मृतिं_प्रतिलिख(&u, &r, परिमाण(u));
  प्रत्यागम ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
स्थिर पूर्णाङ्क percent_Same_मानम्(द्विगुण a, द्विगुण b){
  a -= b;
  प्रत्यागम a>=-0.001 && a<=0.001;
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
स्थिर i64 percent_द्विमान_अन्वेषणम्(Percentile *p, द्विगुण y, पूर्णाङ्क bExact){
  i64 iFirst = 0;                   /* First element of search range */
  i64 i_चरमः = (i64)p->n_प्रयुक्तम् - 1;    /* Last element of search range */
  यावत्( i_चरमः>=iFirst ){
    i64 iMid = (iFirst+i_चरमः)/2;
    द्विगुण x = p->a[iMid];
    यदि( x<y ){
      iFirst = iMid + 1;
    }अन्यथा यदि( x>y ){
      i_चरमः = iMid - 1;
    }अन्यथा{
      प्रत्यागम iMid;
    }
  }
  यदि( bExact ) प्रत्यागम -1;
  प्रत्यागम iFirst;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
स्थिर शून्य percentError(sqlite3_context *pCtx, ध्रुव वर्ण *zFormat, ...){
  वर्ण *zMsg1;
  वर्ण *zMsg2;
  va_list ap;

  va_start(ap, zFormat);
  zMsg1 = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_परिणाम_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

/*
** The "step" function for percentile(Y,P) is called once for each
** input row.
*/
स्थिर शून्य percent_चरणः(sqlite3_context *pCtx, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  Percentile *p;
  द्विगुण rPct;
  पूर्णाङ्क eType;
  द्विगुण y;
  प्रतिज्ञा( argc==2 || argc==1 );

  यदि( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }अन्यथा{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    द्विगुण mxFrac = (SQLITE_PTR_गन्तव्यम्_INT(sqlite3_user_दत्त(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_मानम्_numeric_type(argv[1]);
    rPct = sqlite3_मानम्_double(argv[1])/mxFrac;
    यदि( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (द्विगुण)mxFrac);
      प्रत्यागम;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, परिमाण(*p));
  यदि( p==0 ) प्रत्यागम;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  यदि( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }अन्यथा यदि( !percent_Same_मानम्(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    प्रत्यागम;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_मानम्_type(argv[0]);
  यदि( eType==SQLITE_NULL ) प्रत्यागम;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  यदि( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    प्रत्यागम;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_मानम्_double(argv[0]);
  यदि( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    प्रत्यागम;
  }

  /* Allocate and store the Y */
  यदि( p->n_प्रयुक्तम्>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    द्विगुण *a = sqlite3_realloc64(p->a, परिमाण(द्विगुण)*n);
    यदि( a==0 ){
      sqlite3_free(p->a);
      स्मृतिं_पूरय(p, 0, परिमाण(*p));
      sqlite3_परिणाम_error_nomem(pCtx);
      प्रत्यागम;
    }
    p->nAlloc = n;
    p->a = a;
  }
  यदि( p->n_प्रयुक्तम्==0 ){
    p->a[p->n_प्रयुक्तम्++] = y;
    p->bSorted = 1;
  }अन्यथा यदि( !p->bSorted || y>=p->a[p->n_प्रयुक्तम्-1] ){
    p->a[p->n_प्रयुक्तम्++] = y;
  }अन्यथा यदि( p->bKeepSorted ){
    i64 i;
    i = percent_द्विमान_अन्वेषणम्(p, y, 0);
    यदि( i<(पूर्णाङ्क)p->n_प्रयुक्तम् ){
      memmove(&p->a[i+1], &p->a[i], (p->n_प्रयुक्तम्-i)*परिमाण(p->a[0]));
    }
    p->a[i] = y;
    p->n_प्रयुक्तम्++;
  }अन्यथा{
    p->a[p->n_प्रयुक्तम्++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#परिभाषय SWAP_DOUBLE(X,Y)  {द्विगुण ttt=(X);(X)=(Y);(Y)=ttt;}

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
स्थिर शून्य percent_क्रमय(
  द्विगुण *a,                      /* Array to sort */
  अचिह्नित पूर्णाङ्क n,                 /* Number of elements in array a[] */
  पूर्णाङ्क iReq                        /* Element caller cares about (or -ve) */
){
  पूर्णाङ्क iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  पूर्णाङ्क iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  पूर्णाङ्क i;         /* Loop counter */
  द्विगुण rPivot; /* The pivot value */

  प्रतिज्ञा( n>=2 );
  कुरु{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    यदि( a[0]>a[n-1] ){
      SWAP_DOUBLE(a[0],a[n-1])
    }
    यदि( n==2 ) प्रत्यागम;
    iGt = n-1;
    i = n/2;
    यदि( a[0]>a[i] ){
      SWAP_DOUBLE(a[0],a[i])
    }अन्यथा यदि( a[i]>a[iGt] ){
      SWAP_DOUBLE(a[i],a[iGt])
    }
    यदि( n==3 ) प्रत्यागम;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    rPivot = a[i];
    iLt = i = 1;
    कुरु{
      यदि( a[i]<rPivot ){
        यदि( i>iLt ) SWAP_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }अन्यथा यदि( a[i]>rPivot ){
        कुरु{
          iGt--;
        }यावत्( iGt>i && a[iGt]>rPivot );
        SWAP_DOUBLE(a[i],a[iGt])
      }अन्यथा{
        i++;
      }
    }यावत्( i<iGt );

    प्रतिज्ञा( iLt>0 && iLt<iGt && (अचिह्नित)iGt<n );
    testcase( iGt>iLt+1 );
    प्रतिज्ञा( a[iLt]==rPivot );
    प्रतिज्ञा( a[iLt-1]<=rPivot );
    प्रतिज्ञा( a[iGt]>=rPivot );
    प्रतिज्ञा( a[iLt+1]>=rPivot );

    यदि( iReq>=0 ){
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
      यदि( iReq<iLt ){
        n = iLt;
      }अन्यथा{
        a += iGt;
        n -= iGt;
        iReq = MAX(0, iReq-iGt);
      }
    }अन्यथा{
      /* Recurse on the smaller partition only.  The smaller partition
      ** will hold n/2 or fewer entries, which assures that the stack
      ** depth will not exceed O(log(n)), even for pathological cases.
      ** Loop without recursion for the larger partition. */
      यदि( iLt>(पूर्णाङ्क)(n/2) ){
        यदि( n-iGt>=2 ) percent_क्रमय(a+iGt, n-iGt, -1);
        n = iLt;
      }अन्यथा{
        यदि( iLt>=2 ) percent_क्रमय(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }यावत्( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
स्थिर शून्य percentInverse(sqlite3_context *pCtx,पूर्णाङ्क argc,sqlite3_मानम् **argv){
  Percentile *p;
  पूर्णाङ्क eType;
  द्विगुण y;
  i64 i;
  प्रतिज्ञा( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, परिमाण(*p));
  प्रतिज्ञा( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_मानम्_type(argv[0]);
  यदि( eType==SQLITE_NULL ) प्रत्यागम;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  यदि( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    प्रत्यागम;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_मानम्_double(argv[0]);
  यदि( percentIsInfinity(y) ){
    प्रत्यागम;
  }
  यदि( p->bSorted==0 ){
    प्रतिज्ञा( p->n_प्रयुक्तम्>1 );
    percent_क्रमय(p->a, p->n_प्रयुक्तम्, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_द्विमान_अन्वेषणम्(p, y, 1);
  यदि( i>=0 ){
    p->n_प्रयुक्तम्--;
    यदि( i<(पूर्णाङ्क)p->n_प्रयुक्तम् ){
      memmove(&p->a[i], &p->a[i+1], (p->n_प्रयुक्तम् - i)*परिमाण(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
स्थिर शून्य percentCompute(sqlite3_context *pCtx, पूर्णाङ्क bIsFinal){
  Percentile *p;
  पूर्णाङ्क settings = SQLITE_PTR_गन्तव्यम्_INT(sqlite3_user_दत्त(pCtx))&1; /* Discrete? */
  अचिह्नित i1, i2;
  द्विगुण v1, v2;
  द्विगुण ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  यदि( p==0 ) प्रत्यागम;
  यदि( p->a==0 ) प्रत्यागम;
  यदि( p->n_प्रयुक्तम् ){
    ix = p->rPct*(p->n_प्रयुक्तम्-1);
    i1 = (अचिह्नित)ix;
    यदि( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      प्रतिज्ञा( p->n_प्रयुक्तम्>1 );
      percent_क्रमय(p->a, p->n_प्रयुक्तम्, (bIsFinal ? (पूर्णाङ्क)i1 : -1));
      p->bSorted = 1;
    }
    यदि( settings & 1 ){
      vx = p->a[i1];
    }अन्यथा{
      i2 = ix==(द्विगुण)i1 || i1==p->n_प्रयुक्तम्-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_परिणाम_double(pCtx, vx);
  }
  यदि( bIsFinal ){
    sqlite3_free(p->a);
    स्मृतिं_पूरय(p, 0, परिमाण(*p));
  }अन्यथा{
    p->bKeepSorted = 1;
  }
}
स्थिर शून्य percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
स्थिर शून्य percent_मानम्(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#यदि_अन्तः /* SQLITE_ENABLE_PERCENTILE */

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
स्थिर शून्य filestatFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ध्रुव वर्ण *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  pBtree = sqlite3_Db_Name_गन्तव्यम्_Btree(db, zDbName);
  यदि( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    पूर्णाङ्क rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    प्रतिज्ञा( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    यदि( sqlite3_str_errcode(pStr) ){
      sqlite3_परिणाम_error_nomem(context);
    }अन्यथा{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      यदि( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      यदि( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        यदि( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_परिणाम_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }अन्यथा{
    sqlite3_परिणाम_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#यदि_अन्तः /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
स्थिर शून्य walstatFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  ध्रुव वर्ण *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  pBtree = sqlite3_Db_Name_गन्तव्यम्_Btree(db, zDbName);
  यदि( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    प्रतिज्ञा( pPager!=0 );
    pStr = sqlite3_str_new(db);
    यदि( sqlite3_str_errcode(pStr) ){
      sqlite3_परिणाम_error_nomem(context);
    }अन्यथा{
      sqlite3_str_append(pStr, "{", 1);
      sqlite3PagerWalStat(pPager, pStr);
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_परिणाम_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }अन्यथा{
    sqlite3_परिणाम_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#यदि_अन्तः /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#यदि_परिभाषितम् SQLITE_DEBUG
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
स्थिर शून्य fpdecodeFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  FpDecode s;
  द्विगुण x;
  पूर्णाङ्क y, z;
  वर्ण zBuf[100];
  अप्रयुक्तम्_PARAMETER(argc);
  प्रतिज्ञा( argc==3 );
  x = sqlite3_मानम्_double(argv[0]);
  y = sqlite3_मानम्_int(argv[1]);
  z = sqlite3_मानम्_int(argv[2]);
  यदि( z<=0 ) z = 1;
  यदि( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  यदि( s.isSpecial==2 ){
    sqlite3_snprintf(परिमाण(zBuf), zBuf, "NaN");
  }अन्यथा{
    sqlite3_snprintf(परिमाण(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
  }
  sqlite3_परिणाम_text(context, zBuf, -1, SQLITE_TRANSIENT);
}
#यदि_अन्तः /* SQLITE_DEBUG */

#यदि_परिभाषितम् SQLITE_DEBUG
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
स्थिर शून्य parseuriFunc(
  sqlite3_context *ctx,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  sqlite3_str *p_परिणाम;
  ध्रुव वर्ण *zVfs;
  ध्रुव वर्ण *zUri;
  अचिह्नित पूर्णाङ्क flgs;
  पूर्णाङ्क rc;
  sqlite3_vfs *pVfs = 0;
  वर्ण *zFile = 0;
  वर्ण *zErr = 0;

  यदि( argc<2 ) प्रत्यागम;
  pVfs = sqlite3_vfs_find(0);
  प्रतिज्ञा( pVfs );
  zVfs = pVfs->zName;
  zUri = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
  यदि( zUri==0 ) प्रत्यागम;
  flgs = (अचिह्नित पूर्णाङ्क)sqlite3_मानम्_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  p_परिणाम = sqlite3_str_new(0);
  यदि( !sqlite3_str_errcode(p_परिणाम) ){
    पूर्णाङ्क i;
    sqlite3_str_appendf(p_परिणाम, "rc=%d", rc);
    sqlite3_str_appendf(p_परिणाम, ", flags=0x%x", flgs);
    sqlite3_str_appendf(p_परिणाम, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(p_परिणाम, ", err=%Q", zErr);
    sqlite3_str_appendf(p_परिणाम, ", file=%Q", zFile);
    यदि( zFile ){
      ध्रुव वर्ण *z = zFile;
      z += sqlite3Strlen30(z)+1;
      यावत्( z[0] ){
        sqlite3_str_appendf(p_परिणाम, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      प्रति(i=2; i<argc; i++){
        ध्रुव वर्ण *zArg;
        यदि( sqlite3_मानम्_type(argv[i])==SQLITE_INTEGER ){
          पूर्णाङ्क k = sqlite3_मानम्_int(argv[i]);
          sqlite3_str_appendf(p_परिणाम, ", '%d:%q'",k,sqlite3_uri_कुञ्चिका(zFile, k));
        }अन्यथा यदि( (zArg = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[i]))!=0 ){
          sqlite3_str_appendf(p_परिणाम, ", '%q:%q'",
                 zArg, sqlite3_uri_parameter(zFile,zArg));
        }अन्यथा{
          sqlite3_str_appendf(p_परिणाम, ", NULL");
        }
      }
    }
  }
  sqlite3_परिणाम_str(ctx, p_परिणाम, SQLITE_FINISH);
  sqlite3_free_filename(zFile);
  sqlite3_free(zErr);
}
#यदि_अन्तः /* SQLITE_DEBUG */

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
शून्य sqlite3_पञ्जिकाकोशः_Builtin_Functions(शून्य){
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
  स्थिर FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#यदि !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(उपपादयति_nonnull_row, 2, INLINEFUNC_उपपादयति_nonnull_row, 0),
    TEST_FUNC(व्यञ्जकम्_compare,        2, INLINEFUNC_व्यञ्जकम्_compare,        0),
    TEST_FUNC(व्यञ्जकम्_उपपादयति_व्यञ्जकम्,   2, INLINEFUNC_व्यञ्जकम्_उपपादयति_व्यञ्जकम्,   0),
    TEST_FUNC(affinity,            1, INLINEFUNC_affinity,            0),
#यदि_अन्तः /* !defined(SQLITE_UNTESTABLE) */
/***** Regular functions *****/
#यदि_परिभाषितम् SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
    SFUNCTION(आरोपय_extension,    1, 0, 0, आरोपय_Ext          ),
    SFUNCTION(आरोपय_extension,    2, 0, 0, आरोपय_Ext          ),
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_प्रयुक्तम्,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#यदि_अन्तः /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    INLINE_FUNC(unlikely,        1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likelihood,      2, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likely,          1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
#यदि_परिभाषितम् SQLITE_ENABLE_विस्थापनम्_SQL_FUNC
    INLINE_FUNC(sqlite_विस्थापनम्,   1, INLINEFUNC_sqlite_विस्थापनम्, 0 ),
#यदि_अन्तः
#यदि defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    FUNCTION(sqlite_filestat,    1, 0, 0, filestatFunc     ),
    FUNCTION(sqlite_walstat,     1, 0, 0, walstatFunc      ),
#यदि_अन्तः
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -3, 0, 1, minmaxFunc       ),
    WAGGREGATE(min, 1, 0, 1, minmax_चरणः, minMaxFinalize, min_Max_मानम्, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmax_चरणः, minMaxFinalize, min_Max_मानम्, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(subtype,           1, 0, 0, subtypeFunc,
                                           SQLITE_FUNC_TYPEOF|SQLITE_SUBTYPE),
    FUNCTION2(दीर्घता,            1, 0, 0, दीर्घता_Func,  SQLITE_FUNC_दीर्घता),
    FUNCTION2(octet_दीर्घता,      1, 0, 0, bytelengthFunc,SQLITE_FUNC_BYTELEN),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(प्रारूप_लिख,            -1, 0, 0, printfFunc       ),
    FUNCTION(format,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(वर्ण,              -1, 0, 0, charFunc         ),
    FUNCTION(निरपेक्षम्,                1, 0, 0, absFunc          ),
#यदि_परिभाषितम् SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#यदि_अन्तः
#यदि defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
    FUNCTION(oom,               -1, 0, 0, oomFunc          ),
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_FLOATING_POINT
    FUNCTION(आवर्तनम्,              1, 0, 0, आवर्तनम्_Func        ),
    FUNCTION(आवर्तनम्,              2, 0, 0, आवर्तनम्_Func        ),
#यदि_अन्तः
    FUNCTION(ऊर्ध्वस्थम्,              1, 0, 0, ऊर्ध्वस्थम्_Func        ),
    FUNCTION(अधस्तनम्,              1, 0, 0, अधस्तनम्_Func        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION(unhex,              1, 0, 0, unhexFunc        ),
    FUNCTION(unhex,              2, 0, 0, unhexFunc        ),
    FUNCTION(concat,            -3, 0, 0, concatFunc       ),
    FUNCTION(concat_ws,         -4, 0, 0, concatwsFunc     ),
    INLINE_FUNC(ifnull,          2, INLINEFUNC_coalesce, 0 ),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_संस्करणम्,    0, 0, 0, संस्करणम्_Func      ),
    DFUNCTION(sqlite_source_id,  0, 0, 0, sourceidFunc     ),
    SFUNCTION(sqlite_log,        2, 0, 0, errlogFunc       ),
    FUNCTION(unistr,             1, 0, 0, unistrFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    FUNCTION(unistr_quote,       1, 1, 0, quoteFunc        ),
    VFUNCTION(चरमः_निवेशय_rowid, 0, 0, 0, चरमः_निवेशय_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(योग_changes,     0, 0, 0, योग_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    FUNCTION(substring,          2, 0, 0, substrFunc       ),
    FUNCTION(substring,          3, 0, 0, substrFunc       ),
    WAGGREGATE(योग,   1,0,0, योग_चरणः, योग_Finalize, योग_Finalize, योग_Inverse, 0),
    WAGGREGATE(योग_2, 1,0,0, योग_चरणः,योग_Finalize_2,योग_Finalize_2,योग_Inverse, 0),
    WAGGREGATE(avg,   1,0,0, योग_चरणः, avgFinalize, avgFinalize, योग_Inverse, 0),
    WAGGREGATE(गणनम्, 0,0,0, गणनम्_चरणः,
        गणनम्_Finalize, गणनम्_Finalize, गणनम्_Inverse,
        SQLITE_FUNC_गणनम्|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(गणनम्, 1,0,0, गणनम्_चरणः,
        गणनम्_Finalize, गणनम्_Finalize, गणनम्_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(गणः_concat, 1, 0, 0, गणः_Concat_चरणः,
        गणः_Concat_Finalize, गणः_Concat_मानम्, गणः_Concat_Inverse, 0),
    WAGGREGATE(गणः_concat, 2, 0, 0, गणः_Concat_चरणः,
        गणः_Concat_Finalize, गणः_Concat_मानम्, गणः_Concat_Inverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, गणः_Concat_चरणः,
        गणः_Concat_Finalize, गणः_Concat_मानम्, गणः_Concat_Inverse, 0),

#यदि_परिभाषितम् SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percent_चरणः,
        percentFinal, percent_मानम्, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percent_चरणः,
        percentFinal, percent_मानम्, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percent_चरणः,
        percentFinal, percent_मानम्, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percent_चरणः,
        percentFinal, percent_मानम्, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#यदि_अन्तः /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#यदि_परिभाषितम् SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#अन्यथा
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    FUNCTION(unknown,           -1, 0, 0, unknownFunc      ),
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_ENABLE_MATH_FUNCTIONS
    MFUNCTION(ceil,              1, xCeil,     ceilingFunc ),
    MFUNCTION(ceiling,           1, xCeil,     ceilingFunc ),
    MFUNCTION(floor,             1, xFloor,    ceilingFunc ),
#यदि SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(trunc,             1, trunc,     ceilingFunc ),
#यदि_अन्तः
    FUNCTION(ln,                 1, 0, 0,      logFunc     ),
    FUNCTION(लघुगणकः,                1, 1, 0,      logFunc     ),
    FUNCTION(log10,              1, 1, 0,      logFunc     ),
    FUNCTION(log2,               1, 2, 0,      logFunc     ),
    FUNCTION(लघुगणकः,                2, 0, 0,      logFunc     ),
    MFUNCTION(घातांकः,               1, घातांकः,       math1Func   ),
    MFUNCTION(घातः,               2, घातः,       math2Func   ),
    MFUNCTION(power,             2, घातः,       math2Func   ),
    MFUNCTION(mod,               2, fmod,      math2Func   ),
    MFUNCTION(acos,              1, acos,      math1Func   ),
    MFUNCTION(asin,              1, asin,      math1Func   ),
    MFUNCTION(atan,              1, atan,      math1Func   ),
    MFUNCTION(atan2,             2, atan2,     math2Func   ),
    MFUNCTION(कोटिज्या,               1, कोटिज्या,       math1Func   ),
    MFUNCTION(ज्या,               1, ज्या,       math1Func   ),
    MFUNCTION(tan,               1, tan,       math1Func   ),
    MFUNCTION(cosh,              1, cosh,      math1Func   ),
    MFUNCTION(sinh,              1, sinh,      math1Func   ),
    MFUNCTION(tanh,              1, tanh,      math1Func   ),
#यदि SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(acosh,             1, acosh,     math1Func   ),
    MFUNCTION(asinh,             1, asinh,     math1Func   ),
    MFUNCTION(atanh,             1, atanh,     math1Func   ),
#यदि_अन्तः
    MFUNCTION(वर्गमूलम्,              1, वर्गमूलम्,      math1Func   ),
    MFUNCTION(radians,           1, deg_गन्तव्यम्_Rad,  math1Func   ),
    MFUNCTION(degrees,           1, rad_गन्तव्यम्_Deg,  math1Func   ),
    MFUNCTION(pi,                0, 0,         piFunc      ),
#यदि_अन्तः /* SQLITE_ENABLE_MATH_FUNCTIONS */
    FUNCTION(sign,               1, 0, 0,      signFunc    ),
    INLINE_FUNC(coalesce,       -4, INLINEFUNC_coalesce, 0 ),
    INLINE_FUNC(iif,            -4, INLINEFUNC_iif,      0 ),
    INLINE_FUNC(यदि,             -4, INLINEFUNC_iif,      0 ),
  };
#यदि_अपरिभाषितम् SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#यदि_अन्तः
  sqlite3_गवाक्षः_Functions();
  sqlite3_पञ्जिकाकोशः_Date_Time_Functions();
  sqlite3_पञ्जिकाकोशः_Json_Functions();
  sqlite3_निवेशय_Builtin_Funcs(aBuiltinFunc, Array_आकार(aBuiltinFunc));

#यदि 0  /* Enable to print out how the built-in functions are hashed */
  {
    पूर्णाङ्क i;
    FuncDef *p;
    प्रति(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      प्रारूप_लिख("FUNC-HASH %02d:", i);
      प्रति(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        पूर्णाङ्क n = sqlite3Strlen30(p->zName);
        पूर्णाङ्क h = p->zName[0] + n;
        प्रतिज्ञा( p->funcFlags & SQLITE_FUNC_BUILTIN );
        प्रारूप_लिख(" %s(%d)", p->zName, h);
      }
      प्रारूप_लिख("\n");
    }
  }
#यदि_अन्तः
}
