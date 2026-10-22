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
#समावेश "sqliteInt.h"
#समावेश <stdlib.h>
#समावेश <assert.h>
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
#समावेश <math.h>
#पूर्ण_अगर
#समावेश "vdbeInt.h"

/*
** Return the collating function associated with a function.
*/
अटल CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  निश्चित( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  निश्चित( pOp->opcode==OP_CollSeq );
  निश्चित( pOp->p4type==P4_COLLSEQ );
  वापस pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
अटल व्योम sqlite3SkipAccumulatorLoad(sqlite3_context *context){
  निश्चित( context->isError<=0 );
  context->isError = -1;
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
अटल व्योम minmaxFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;
  पूर्णांक mask;    /* 0 for min() or 0xffffffff for max() */
  पूर्णांक iBest;
  CollSeq *pColl;

  निश्चित( argc>1 );
  mask = sqlite3_user_तथ्य(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  निश्चित( pColl );
  निश्चित( mask==-1 || mask==0 );
  iBest = 0;
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ) वापस;
  क्रम(i=1; i<argc; i++){
    अगर( sqlite3_मूल्य_type(argv[i])==SQLITE_NULL ) वापस;
    अगर( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^mask)>=0 ){
      testcase( mask==0 );
      iBest = i;
    }
  }
  sqlite3_परिणाम_मूल्य(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
अटल व्योम typeofFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  अटल स्थिर अक्षर *azType[] = { "integer", "real", "text", "blob", "null" };
  पूर्णांक i = sqlite3_मूल्य_type(argv[0]) - 1;
  UNUSED_PARAMETER(NotUsed);
  निश्चित( i>=0 && i<Array_आकार(azType) );
  निश्चित( SQLITE_INTEGER==1 );
  निश्चित( SQLITE_FLOAT==2 );
  निश्चित( SQLITE_TEXT==3 );
  निश्चित( SQLITE_BLOB==4 );
  निश्चित( SQLITE_NULL==5 );
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
अटल व्योम subtypeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  UNUSED_PARAMETER(argc);
  sqlite3_परिणाम_int(context, sqlite3_मूल्य_subtype(argv[0]));
}

/*
** Implementation of the length() function
*/
अटल व्योम लंबाई_Func(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  चयन( sqlite3_मूल्य_type(argv[0]) ){
    हाल SQLITE_BLOB:
    हाल SQLITE_INTEGER:
    हाल SQLITE_FLOAT: {
      sqlite3_परिणाम_int(context, sqlite3_मूल्य_bytes(argv[0]));
      अवरोध;
    }
    हाल SQLITE_TEXT: {
      स्थिर अचिन्हित अक्षर *z = sqlite3_मूल्य_text(argv[0]);
      स्थिर अचिन्हित अक्षर *z0;
      अगर( z==0 ) वापस;
      z0 = z;
      जबतक( 1 /*exit-by-break*/ ){
                      /*  vvvvvv----  See tag-20260418-01 */
        अगर( (u8)(z[0]-1)<(0x80-1) ){
          z++;
        }अन्यथा अगर( z[0]==0 ){
          अवरोध;
        }अन्यथा{
          z++;
          जबतक( (z[0]&0xc0)==0x80 ){ z++; z0++; }
        }
      }
      sqlite3_परिणाम_int(context, (पूर्णांक)(z-z0));
      अवरोध;
    }
    शेष: {
      sqlite3_परिणाम_null(context);
      अवरोध;
    }
  }
}

/*
** Implementation of the octet_length() function
*/
अटल व्योम bytelengthFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  चयन( sqlite3_मूल्य_type(argv[0]) ){
    हाल SQLITE_BLOB: {
      sqlite3_परिणाम_int(context, sqlite3_मूल्य_bytes(argv[0]));
      अवरोध;
    }
    हाल SQLITE_INTEGER:
    हाल SQLITE_FLOAT: {
      i64 m = sqlite3_context_db_handle(context)->enc<=SQLITE_UTF8 ? 1 : 2;
      sqlite3_परिणाम_int64(context, sqlite3_मूल्य_bytes(argv[0])*m);
      अवरोध;
    }
    हाल SQLITE_TEXT: {
      अगर( sqlite3_मूल्य_encoding(argv[0])<=SQLITE_UTF8 ){
        sqlite3_परिणाम_int(context, sqlite3_मूल्य_bytes(argv[0]));
      }अन्यथा{
        sqlite3_परिणाम_int(context, sqlite3_मूल्य_bytes16(argv[0]));
      }
      अवरोध;
    }
    शेष: {
      sqlite3_परिणाम_null(context);
      अवरोध;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X.
*/
अटल व्योम absFunc(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  चयन( sqlite3_मूल्य_type(argv[0]) ){
    हाल SQLITE_INTEGER: {
      i64 iVal = sqlite3_मूल्य_int64(argv[0]);
      अगर( iVal<0 ){
        अगर( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_परिणाम_error(context, "integer overflow", -1);
          वापस;
        }
        iVal = -iVal;
      }
      sqlite3_परिणाम_int64(context, iVal);
      अवरोध;
    }
    हाल SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_परिणाम_null(context);
      अवरोध;
    }
    शेष: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      द्विगुन rVal = sqlite3_मूल्य_double(argv[0]);
      अगर( rVal<0 ) rVal = -rVal;
      sqlite3_परिणाम_double(context, rVal);
      अवरोध;
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
अटल व्योम instrFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर अचिन्हित अक्षर *zHaystack;
  स्थिर अचिन्हित अक्षर *zNeedle;
  पूर्णांक nHaystack;
  पूर्णांक nNeedle;
  पूर्णांक typeHaystack, typeNeedle;
  पूर्णांक N = 1;
  पूर्णांक isText;
  अचिन्हित अक्षर firstChar;
  sqlite3_मूल्य *pC1 = 0;
  sqlite3_मूल्य *pC2 = 0;

  UNUSED_PARAMETER(argc);
  typeHaystack = sqlite3_मूल्य_type(argv[0]);
  typeNeedle = sqlite3_मूल्य_type(argv[1]);
  अगर( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) वापस;
  nHaystack = sqlite3_मूल्य_bytes(argv[0]);
  nNeedle = sqlite3_मूल्य_bytes(argv[1]);
  अगर( nNeedle>0 ){
    अगर( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
      zHaystack = sqlite3_मूल्य_blob(argv[0]);
      zNeedle = sqlite3_मूल्य_blob(argv[1]);
      isText = 0;
    }अन्यथा अगर( typeHaystack!=SQLITE_BLOB && typeNeedle!=SQLITE_BLOB ){
      zHaystack = sqlite3_मूल्य_text(argv[0]);
      zNeedle = sqlite3_मूल्य_text(argv[1]);
      isText = 1;
    }अन्यथा{
      pC1 = sqlite3_मूल्य_dup(argv[0]);
      zHaystack = sqlite3_मूल्य_text(pC1);
      अगर( zHaystack==0 ) जाओ endInstrOOM;
      nHaystack = sqlite3_मूल्य_bytes(pC1);
      pC2 = sqlite3_मूल्य_dup(argv[1]);
      zNeedle = sqlite3_मूल्य_text(pC2);
      अगर( zNeedle==0 ) जाओ endInstrOOM;
      nNeedle = sqlite3_मूल्य_bytes(pC2);
      isText = 1;
    }
    अगर( zNeedle==0 || (nHaystack && zHaystack==0) ) जाओ endInstrOOM;
    firstChar = zNeedle[0];
    जबतक( nNeedle<=nHaystack
       && (zHaystack[0]!=firstChar || स_भेद(zHaystack, zNeedle, nNeedle)!=0)
    ){
      N++;
      करो{
        nHaystack--;
        zHaystack++;
      }जबतक( isText && (zHaystack[0]&0xc0)==0x80 );
    }
    अगर( nNeedle>nHaystack ) N = 0;
  }
  sqlite3_परिणाम_int(context, N);
endInstr:
  sqlite3_मूल्य_free(pC1);
  sqlite3_मूल्य_free(pC2);
  वापस;
endInstrOOM:
  sqlite3_परिणाम_error_nomem(context);
  जाओ endInstr;
}

/*
** Implementation of the printf() (a.k.a. format()) SQL function.
*/
अटल व्योम printfFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  PrintfArguments x;
  StrAccum str;
  स्थिर अक्षर *zFormat;
  sqlite3 *db = sqlite3_context_db_handle(context);

  अगर( argc>=1 && (zFormat = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.nUsed = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_लंबाई]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3_str_appendf(&str, zFormat, &x);
    sqlite3_परिणाम_str(context, &str, SQLITE_XFER);
  }
}

/* Return true if any of the eight bytes in word is NUL or non-ASCII. */
अटल SQLITE_INLINE पूर्णांक containsNulOrNonAscii(u64 word){
  स्थिर u64 byteOnes = UINT64_C(0x0101010101010101);
  स्थिर u64 byte_ऊपरी_Bits = UINT64_C(0x8080808080808080);
  /* NUL causes a borrow into a high bit; non-ASCII already has one set. */
  वापस ((word | (word - byteOnes)) & byte_ऊपरी_Bits)!=0;
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
अटल व्योम substrFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर अचिन्हित अक्षर *z;
  स्थिर अचिन्हित अक्षर *z2;
  पूर्णांक len;
  पूर्णांक p0type;
  i64 p1, p2;

  निश्चित( argc==3 || argc==2 );
  p0type = sqlite3_मूल्य_type(argv[0]);
  p1 = sqlite3_मूल्य_int64(argv[1]);
  अगर( p0type==SQLITE_BLOB ){
    len = sqlite3_मूल्य_bytes(argv[0]);
    z = sqlite3_मूल्य_blob(argv[0]);
    अगर( z==0 ) वापस;
    निश्चित( len==sqlite3_मूल्य_bytes(argv[0]) );
  }अन्यथा{
    z = sqlite3_मूल्य_text(argv[0]);
    अगर( z==0 ) वापस;
    len = 0;
    अगर( p1<0 ){
      क्रम(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
  अगर( argc==3 ){
    p2 = sqlite3_मूल्य_int64(argv[2]);
    अगर( p2==0 && sqlite3_मूल्य_type(argv[2])==SQLITE_NULL ) वापस;
  }अन्यथा{
    p2 = LARGEST_INT64;
  }
  अगर( p1==0 ){
#अगर_घोषित SQLITE_SUBSTR_COMPATIBILITY
    /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
    ** as substr(X,1,N) - it returns the first N characters of X.  This
    ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
    ** from 2009-02-02 for compatibility of applications that exploited the
    ** old buggy behavior. */
    p1 = 1; /* <rdar://problem/6778339> */
#पूर्ण_अगर
    अगर( sqlite3_मूल्य_type(argv[1])==SQLITE_NULL ) वापस;
  }
  अगर( p1<0 ){
    p1 += len;
    अगर( p1<0 ){
      अगर( p2<0 ){
        p2 = 0;
      }अन्यथा{
        p2 += p1;
      }
      p1 = 0;
    }
  }अन्यथा अगर( p1>0 ){
    p1--;
  }अन्यथा अगर( p2>0 ){
    p2--;
  }
  अगर( p2<0 ){
    अगर( p2<-p1 ){
      p2 = p1;
    }अन्यथा{
      p2 = -p2;
    }
    p1 -= p2;
  }
  निश्चित( p1>=0 && p2>=0 );
  अगर( p0type!=SQLITE_BLOB ){
    अगर( p1>=8 ){
      /* Skip 8 characters at a time while all 8 bytes are in 0x01..0x7f */
      स्थिर अचिन्हित अक्षर *zEnd = z + sqlite3_मूल्य_bytes(argv[0]);
      जबतक( p1>=8 && zEnd-z>=8 ){
        u64 word;
        स_नकल(&word, z, माप(word));
        /* Handle NUL and UTF-8 with the scalar loop below. */
        अगर( containsNulOrNonAscii(word) ){
          अवरोध;
        }
        z += 8;
        p1 -= 8;
      }
    }
    क्रम( ; p1>0; p1--){
                    /*  vvvvvv----  See tag-20260418-01 */
      अगर( (u8)(z[0]-1)<(0x80-1) ){
        z++;
      }अन्यथा अगर( z[0]==0 ){
        अवरोध;
      }अन्यथा{
        करो{ z++; }जबतक( (z[0]&0xc0)==0x80 );
      }
    }
    क्रम(z2=z; p2>0; p2--){
                     /*  vvvvvv----  See tag-20260418-01 */
      अगर( (u8)(z2[0]-1)<(0x80-1) ){
        z2++;
      }अन्यथा अगर( z2[0]==0 ){
        अवरोध;
      }अन्यथा{
        करो{ z2++; }जबतक( (z2[0]&0xc0)==0x80 );
      }
    }
    sqlite3_परिणाम_text64(context, (अक्षर*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }अन्यथा{
    अगर( p1>=len ){
      p1 = p2 = 0;
    }अन्यथा अगर( p2>len-p1 ){
      p2 = len-p1;
      निश्चित( p2>0 );
    }
    sqlite3_परिणाम_blob64(context, (अक्षर*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
अटल व्योम roundFunc(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  i64 n = 0;
  द्विगुन r;
  अक्षर *zBuf;
  निश्चित( argc==1 || argc==2 );
  अगर( argc==2 ){
    अगर( SQLITE_NULL==sqlite3_मूल्य_type(argv[1]) ) वापस;
    n = sqlite3_मूल्य_int64(argv[1]);
    अगर( n>30 ) n = 30;
    अगर( n<0 ) n = 0;
  }
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ) वापस;
  r = sqlite3_मूल्य_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  अगर( r<-4503599627370496.0 || r>+4503599627370496.0 ){
    /* The value has no fractional part so there is nothing to round */
  }अन्यथा अगर( n==0 ){ 
    r = (द्विगुन)((sqlite_int64)(r+(r<0?-0.5:+0.5)));
  }अन्यथा{
    zBuf = sqlite3_mprintf("%!.*f",(पूर्णांक)n,r);
    अगर( zBuf==0 ){
      sqlite3_परिणाम_error_nomem(context);
      वापस;
    }
    sqlite3AtoF(zBuf, &r);
    sqlite3_free(zBuf);
  }
  sqlite3_परिणाम_double(context, r);
}
#पूर्ण_अगर

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
अटल व्योम *contextMalloc(sqlite3_context *context, i64 nByte){
  अक्षर *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  निश्चित( nByte>0 );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_लंबाई] );
  testcase( nByte==(i64)db->aLimit[SQLITE_LIMIT_लंबाई]+1 );
  अगर( nByte>db->aLimit[SQLITE_LIMIT_लंबाई] ){
    sqlite3_परिणाम_error_toobig(context);
    z = 0;
  }अन्यथा{
    z = sqlite3Malloc(nByte);
    अगर( !z ){
      sqlite3_परिणाम_error_nomem(context);
    }
  }
  वापस z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
अटल व्योम upperFunc(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  अक्षर *z1;
  स्थिर अक्षर *z2;
  पूर्णांक i, n;
  UNUSED_PARAMETER(argc);
  z2 = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
  n = sqlite3_मूल्य_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  निश्चित( z2==(अक्षर*)sqlite3_मूल्य_text(argv[0]) );
  अगर( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    अगर( z1 ){
      क्रम(i=0; i<n; i++){
        z1[i] = (अक्षर)sqlite3Toupper(z2[i]);
      }
      sqlite3_परिणाम_text(context, z1, n, sqlite3_free);
    }
  }
}
अटल व्योम lowerFunc(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  अक्षर *z1;
  स्थिर अक्षर *z2;
  पूर्णांक i, n;
  UNUSED_PARAMETER(argc);
  z2 = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
  n = sqlite3_मूल्य_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  निश्चित( z2==(अक्षर*)sqlite3_मूल्य_text(argv[0]) );
  अगर( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    अगर( z1 ){
      क्रम(i=0; i<n; i++){
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
#घोषणा noopFunc versionFunc   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer. 
*/
अटल व्योम randomFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
){
  sqlite_int64 r;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_randomness(माप(r), &r);
  अगर( r<0 ){
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
अटल व्योम randomBlob(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_int64 n;
  अचिन्हित अक्षर *p;
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_मूल्य_int64(argv[0]);
  अगर( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  अगर( p ){
    sqlite3_randomness(n, p);
    sqlite3_परिणाम_blob(context, (अक्षर*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
अटल व्योम last_जोड़ो_rowid(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_परिणाम_int64(context, sqlite3_last_जोड़ो_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-32760-32347 The changes() SQL function is a wrapper
** around the sqlite3_changes64() C/C++ function and hence follows the
** same rules for counting changes.
*/
अटल व्योम changes(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_परिणाम_int64(context, sqlite3_changes64(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes64() API function.
*/
अटल व्योम योग_changes(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-11217-42568 This function is a wrapper around the
  ** sqlite3_total_changes64() C/C++ interface. */
  sqlite3_परिणाम_int64(context, sqlite3_योग_changes64(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
काष्ठा compareInfo {
  u8 matchAll;          /* "*" or "%" */
  u8 matchOne;          /* "?" or "_" */
  u8 matchSet;          /* "[" or 0 */
  u8 noCase;            /* true to ignore case differences */
};

/*
** For LIKE and GLOB matching on EBCDIC machines, assume that every
** character is exactly one byte in size.  Also, provide the Utf8Read()
** macro for fast reading of the next character in the common case where
** the next character is ASCII.
*/
#अगर defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#अन्यथा
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#पूर्ण_अगर

अटल स्थिर काष्ठा compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
अटल स्थिर काष्ठा compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
अटल स्थिर काष्ठा compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Possible error returns from patternMatch()
*/
#घोषणा SQLITE_MATCH             0
#घोषणा SQLITE_NOMATCH           1
#घोषणा SQLITE_NOWILDCARDMATCH   2

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
अटल पूर्णांक patternCompare(
  स्थिर u8 *zPattern,              /* The glob pattern */
  स्थिर u8 *zString,               /* The string to compare against the glob */
  स्थिर काष्ठा compareInfo *pInfo, /* Information about how to do the compare */
  u32 matchOther                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 matchOne = pInfo->matchOne;  /* "?" or "_" */
  u32 matchAll = pInfo->matchAll;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  स्थिर u8 *zEscaped = 0;          /* One past the last escaped input char */
 
  जबतक( (c = Utf8Read(zPattern))!=0 ){
    अगर( c==matchAll ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      जबतक( (c=Utf8Read(zPattern)) == matchAll
             || (c == matchOne && matchOne!=0) ){
        अगर( c==matchOne && sqlite3Utf8Read(&zString)==0 ){
          वापस SQLITE_NOWILDCARDMATCH;
        }
      }
      अगर( c==0 ){
        वापस SQLITE_MATCH;   /* "*" at the end of the pattern matches */
      }अन्यथा अगर( c==matchOther ){
        अगर( pInfo->matchSet==0 ){
          c = sqlite3Utf8Read(&zPattern);
          अगर( c==0 ) वापस SQLITE_NOWILDCARDMATCH;
        }अन्यथा{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          निश्चित( matchOther<0x80 );  /* '[' is a single-byte character */
          जबतक( *zString ){
            पूर्णांक bMatch = patternCompare(&zPattern[-1],zString,pInfo,matchOther);
            अगर( bMatch!=SQLITE_NOMATCH ) वापस bMatch;
            SQLITE_SKIP_UTF8(zString);
          }
          वापस SQLITE_NOWILDCARDMATCH;
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
      अगर( c<0x80 ){
        अक्षर zStop[3];
        पूर्णांक bMatch;
        /* If the next pattern character is an ordinary ASCII character,
        ** then a candidate position can only match if the next input
        ** character is nx or ny.  Check that inline, to avoid most of the
        ** recursive calls.  nx==0 disables this check. */
        u8 nx = zPattern[0], ny = nx;
        अगर( nx>=0x80 || nx==matchAll || nx==matchOne || nx==matchOther ){
          nx = 0;
        }
        अगर( noCase ){
          zStop[0] = sqlite3Toupper(c);
          zStop[1] = sqlite3Tolower(c);
          zStop[2] = 0;
          nx = sqlite3Toupper(nx);
          ny = sqlite3Tolower(ny);
        }अन्यथा{
          zStop[0] = c;
          zStop[1] = 0;
        }
        जबतक(1){
          zString += म_खोज((स्थिर अक्षर*)zString, zStop);
          अगर( zString[0]==0 ) अवरोध;
          zString++;
          अगर( nx && zString[0]!=nx && zString[0]!=ny ) जारी;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          अगर( bMatch!=SQLITE_NOMATCH ) वापस bMatch;
        }
      }अन्यथा{
        पूर्णांक bMatch;
        जबतक( (c2 = Utf8Read(zString))!=0 ){
          अगर( c2!=c ) जारी;
          bMatch = patternCompare(zPattern,zString,pInfo,matchOther);
          अगर( bMatch!=SQLITE_NOMATCH ) वापस bMatch;
        }
      }
      वापस SQLITE_NOWILDCARDMATCH;
    }
    अगर( c==matchOther ){
      अगर( pInfo->matchSet==0 ){
        c = sqlite3Utf8Read(&zPattern);
        अगर( c==0 ) वापस SQLITE_NOMATCH;
        zEscaped = zPattern;
      }अन्यथा{
        u32 prior_c = 0;
        पूर्णांक seen = 0;
        पूर्णांक invert = 0;
        c = sqlite3Utf8Read(&zString);
        अगर( c==0 ) वापस SQLITE_NOMATCH;
        c2 = sqlite3Utf8Read(&zPattern);
        अगर( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        अगर( c2==']' ){
          अगर( c==']' ) seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        जबतक( c2 && c2!=']' ){
          अगर( c2=='-' && zPattern[0]!=']' && zPattern[0]!=0 && prior_c>0 ){
            c2 = sqlite3Utf8Read(&zPattern);
            अगर( c>=prior_c && c<=c2 ) seen = 1;
            prior_c = 0;
          }अन्यथा{
            अगर( c==c2 ){
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        अगर( c2==0 || (seen ^ invert)==0 ){
          वापस SQLITE_NOMATCH;
        }
        जारी;
      }
    }
    c2 = Utf8Read(zString);
    अगर( c==c2 ) जारी;
    अगर( noCase  && sqlite3Tolower(c)==sqlite3Tolower(c2) && c<0x80 && c2<0x80 ){
      जारी;
    }
    अगर( c==matchOne && zPattern!=zEscaped && c2!=0 ) जारी;
    वापस SQLITE_NOMATCH;
  }
  वापस *zString==0 ? SQLITE_MATCH : SQLITE_NOMATCH;
}

/*
** The sqlite3_strglob() interface.  Return 0 on a match (like strcmp()) and
** non-zero if there is no match.
*/
पूर्णांक sqlite3_strglob(स्थिर अक्षर *zGlobPattern, स्थिर अक्षर *zString){
  अगर( zString==0 ){
    वापस zGlobPattern!=0;
  }अन्यथा अगर( zGlobPattern==0 ){
    वापस 1;
  }अन्यथा {
    वापस patternCompare((u8*)zGlobPattern, (u8*)zString, &globInfo, '[');
  }
}

/*
** The sqlite3_strlike() interface.  Return 0 on a match and non-zero for
** a miss - like strcmp().
*/
पूर्णांक sqlite3_strlike(स्थिर अक्षर *zPattern, स्थिर अक्षर *zStr, अचिन्हित पूर्णांक esc){
  अगर( zStr==0 ){
    वापस zPattern!=0;
  }अन्यथा अगर( zPattern==0 ){
    वापस 1;
  }अन्यथा{
    वापस patternCompare((u8*)zPattern, (u8*)zStr, &likeInfoNorm, esc);
  }
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#अगर_घोषित SQLITE_TEST
पूर्णांक sqlite3_like_गिनती = 0;
#पूर्ण_अगर


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
अटल व्योम likeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर अचिन्हित अक्षर *zA, *zB;
  u32 escape;
  पूर्णांक nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  काष्ठा compareInfo *pInfo = sqlite3_user_तथ्य(context);
  काष्ठा compareInfo backupInfo;

#अगर_घोषित SQLITE_LIKE_DOESNT_MATCH_BLOBS
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_BLOB
   || sqlite3_मूल्य_type(argv[1])==SQLITE_BLOB
  ){
#अगर_घोषित SQLITE_TEST
    sqlite3_like_गिनती++;
#पूर्ण_अगर
    sqlite3_परिणाम_int(context, 0);
    वापस;
  }
#पूर्ण_अगर

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_मूल्य_bytes(argv[0]);
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_लंबाई] );
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_लंबाई]+1 );
  अगर( nPat > db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_लंबाई] ){
    sqlite3_परिणाम_error(context, "LIKE or GLOB pattern too complex", -1);
    वापस;
  }
  अगर( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    स्थिर अचिन्हित अक्षर *zEsc = sqlite3_मूल्य_text(argv[2]);
    अगर( zEsc==0 ) वापस;
    अगर( sqlite3Utf8CharLen((अक्षर*)zEsc, -1)!=1 ){
      sqlite3_परिणाम_error(context,
          "ESCAPE expression must be a single character", -1);
      वापस;
    }
    escape = sqlite3Utf8Read(&zEsc);
    अगर( escape==pInfo->matchAll || escape==pInfo->matchOne ){
      स_नकल(&backupInfo, pInfo, माप(backupInfo));
      pInfo = &backupInfo;
      अगर( escape==pInfo->matchAll ) pInfo->matchAll = 0;
      अगर( escape==pInfo->matchOne ) pInfo->matchOne = 0;
    }
  }अन्यथा{
    escape = pInfo->matchSet;
  }
  zB = sqlite3_मूल्य_text(argv[0]);
  zA = sqlite3_मूल्य_text(argv[1]);
  अगर( zA && zB ){
#अगर_घोषित SQLITE_TEST
    sqlite3_like_गिनती++;
#पूर्ण_अगर
    sqlite3_परिणाम_int(context,
                      patternCompare(zB, zA, pInfo, escape)==SQLITE_MATCH);
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
अटल व्योम nullifFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  UNUSED_PARAMETER(NotUsed);
  अगर( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_परिणाम_मूल्य(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
अटल व्योम versionFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_परिणाम_text(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
अटल व्योम sourceidFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_परिणाम_text(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
अटल व्योम errlogFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(context);
  sqlite3_log(sqlite3_मूल्य_int(argv[0]), "%s", sqlite3_मूल्य_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#अगर_अघोषित SQLITE_OMIT_COMPILEOPTION_DIAGS
अटल व्योम compileoptionusedFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर अक्षर *zOptName;
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  अगर( (zOptName = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]))!=0 ){
    sqlite3_परिणाम_int(context, sqlite3_compileoption_used(zOptName));
  }
}
#पूर्ण_अगर /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function.
** The result is a string that identifies the compiler options
** used to build SQLite.
*/
#अगर_अघोषित SQLITE_OMIT_COMPILEOPTION_DIAGS
अटल व्योम compileoptiongetFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  i64 n;
  स्थिर अक्षर *zRes;
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_मूल्य_int64(argv[0]);
  /* Use a negative lower bound to assist testing of the
  ** sqlite3_compileoption_get() C-language interface and its handling
  ** of negative inputs.
  **         vvvvv */
  zRes = (n>=(-10) && n<10000) ? sqlite3_compileoption_get((पूर्णांक)n) : 0;
  sqlite3_परिणाम_text(context, zRes, -1, SQLITE_STATIC);
}
#पूर्ण_अगर /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
अटल स्थिर अक्षर hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*
** Append to pStr text that is the SQL literal representation of the
** value contained in pValue.
*/
व्योम sqlite3_Quote_मूल्य(StrAccum *pStr, sqlite3_मूल्य *p_मूल्य, पूर्णांक bEscape){
  /* As currently implemented, the string must be initially empty.
  ** we might relax this requirement in the future, but that will
  ** require enhancements to the implementation. */
  निश्चित( pStr!=0 && pStr->nChar==0 );

  चयन( sqlite3_मूल्य_type(p_मूल्य) ){
    हाल SQLITE_FLOAT: {
                             /*    ,---  Show infinity as 9.0e+999
                             **    |   
                             **    | ,--- 17 precision guarantees round-trip
                             **    v v                                       */
      sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_मूल्य_double(p_मूल्य));
      अवरोध;
    }
    हाल SQLITE_INTEGER: {
      sqlite3_str_appendf(pStr, "%lld", sqlite3_मूल्य_int64(p_मूल्य));
      अवरोध;
    }
    हाल SQLITE_BLOB: {
      अक्षर स्थिर *zBlob = sqlite3_मूल्य_blob(p_मूल्य);
      i64 nBlob = sqlite3_मूल्य_bytes(p_मूल्य);
      निश्चित( zBlob==sqlite3_मूल्य_blob(p_मूल्य) ); /* No encoding change */
      sqlite3StrAccumEnlarge(pStr, nBlob*2 + 4);
      अगर( pStr->accError==0 ){
        अक्षर *zText = pStr->zText;
        पूर्णांक i;
        क्रम(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        pStr->nChar = nBlob*2 + 3;
      }
      अवरोध;
    }
    हाल SQLITE_TEXT: {
      स्थिर अचिन्हित अक्षर *zArg = sqlite3_मूल्य_text(p_मूल्य);
      sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
      अवरोध;
    }
    शेष: {
      निश्चित( sqlite3_मूल्य_type(p_मूल्य)==SQLITE_NULL );
      sqlite3_str_append(pStr, "NULL", 4);
      अवरोध;
    }
  }
}

/*
** Return true if z[] begins with N hexadecimal digits, and write
** a decoding of those digits into *pVal.  Or return false if any
** one of the first N characters in z[] is not a hexadecimal digit.
*/
अटल पूर्णांक isNHex(स्थिर अक्षर *z, पूर्णांक N, u32 *pVal){
  पूर्णांक i;
  u32 v = 0;
  क्रम(i=0; i<N; i++){
    अगर( !sqlite3Isxdigit(z[i]) ) वापस 0;
    v = (v<<4) + sqlite3HexToInt(z[i]);
  }
  *pVal = v;
  वापस 1;
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
अटल व्योम unistrFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  अक्षर *zOut;
  स्थिर अक्षर *zIn;
  पूर्णांक nIn;
  पूर्णांक i, j, n;
  u32 v;

  निश्चित( argc==1 );
  UNUSED_PARAMETER( argc );
  zIn = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  अगर( zIn==0 ) वापस;
  nIn = sqlite3_मूल्य_bytes(argv[0]);
  zOut = sqlite3_malloc64(nIn+1);
  अगर( zOut==0 ){
    sqlite3_परिणाम_error_nomem(context);
    वापस;
  }
  i = j = 0;
  जबतक( i<nIn ){
    स्थिर अक्षर *z = म_अक्षर(&zIn[i],'\\');
    अगर( z==0 ){
      n = nIn - i;
      स_हटाओ(&zOut[j], &zIn[i], n);
      j += n;
      अवरोध;
    }
    n = z - &zIn[i];
    अगर( n>0 ){
      स_हटाओ(&zOut[j], &zIn[i], n);
      j += n;
      i += n;
    }
    अगर( zIn[i+1]=='\\' ){
      i += 2;
      zOut[j++] = '\\';
    }अन्यथा अगर( sqlite3Isxdigit(zIn[i+1]) ){
      अगर( !isNHex(&zIn[i+1], 4, &v) ) जाओ unistr_error;
      i += 5;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }अन्यथा अगर( zIn[i+1]=='+' ){
      अगर( !isNHex(&zIn[i+2], 6, &v) ) जाओ unistr_error;
      i += 8;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }अन्यथा अगर( zIn[i+1]=='u' ){
      अगर( !isNHex(&zIn[i+2], 4, &v) ) जाओ unistr_error;
      i += 6;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }अन्यथा अगर( zIn[i+1]=='U' ){
      अगर( !isNHex(&zIn[i+2], 8, &v) ) जाओ unistr_error;
      i += 10;
      j += sqlite3AppendOneUtf8Character(&zOut[j], v);
    }अन्यथा{
      जाओ unistr_error;
    }
  }
  zOut[j] = 0;
  sqlite3_परिणाम_text64(context, zOut, j, sqlite3_free, SQLITE_UTF8_ZT);
  वापस;

unistr_error:
  sqlite3_free(zOut);
  sqlite3_परिणाम_error(context, "invalid Unicode escape", -1);
  वापस;
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
अटल व्योम quoteFunc(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  sqlite3_str str;
  sqlite3 *db = sqlite3_context_db_handle(context);
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_लंबाई]);
  sqlite3_Quote_मूल्य(&str,argv[0],SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(context)));
  sqlite3_परिणाम_str(context, &str, SQLITE_XFER);
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string.
*/
अटल व्योम unicodeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर अचिन्हित अक्षर *z = sqlite3_मूल्य_text(argv[0]);
  (व्योम)argc;
  अगर( z && z[0] ) sqlite3_परिणाम_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
अटल व्योम charFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  अचिन्हित अक्षर *z, *zOut;
  पूर्णांक i;
  zOut = z = sqlite3_malloc64( argc*4+1 );
  अगर( z==0 ){
    sqlite3_परिणाम_error_nomem(context);
    वापस;
  }
  क्रम(i=0; i<argc; i++){
    sqlite3_int64 x;
    अचिन्हित c;
    x = sqlite3_मूल्य_int64(argv[i]);
    अगर( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (अचिन्हित)(x & 0x1fffff);
    अगर( c<0x00080 ){
      *zOut++ = (u8)(c&0xFF);
    }अन्यथा अगर( c<0x00800 ){
      *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }अन्यथा अगर( c<0x10000 ){
      *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }अन्यथा{
      *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);
      *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  *zOut = 0;
  sqlite3_परिणाम_text64(context, (अक्षर*)z, zOut-z,sqlite3_free,SQLITE_UTF8_ZT);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
अटल व्योम hexFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक i, n;
  स्थिर अचिन्हित अक्षर *pBlob;
  अक्षर *zHex, *z;
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  pBlob = sqlite3_मूल्य_blob(argv[0]);
  n = sqlite3_मूल्य_bytes(argv[0]);
  निश्चित( pBlob==sqlite3_मूल्य_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  अगर( zHex ){
    क्रम(i=0; i<n; i++, pBlob++){
      अचिन्हित अक्षर c = *pBlob;
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
अटल पूर्णांक strContainsChar(स्थिर u8 *zStr, पूर्णांक nStr, u32 ch){
  स्थिर u8 *zEnd = &zStr[nStr];
  स्थिर u8 *z = zStr;
  जबतक( z<zEnd ){
    u32 tst = Utf8Read(z);
    अगर( tst==ch ) वापस 1;
  }
  वापस 0;
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
अटल व्योम unhexFunc(
  sqlite3_context *pCtx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर u8 *zPass = (स्थिर u8*)"";
  पूर्णांक nPass = 0;
  स्थिर u8 *zHex = sqlite3_मूल्य_text(argv[0]);
  पूर्णांक nHex = sqlite3_मूल्य_bytes(argv[0]);
#अगर_घोषित SQLITE_DEBUG
  स्थिर u8 *zEnd = zHex ? &zHex[nHex] : 0;
#पूर्ण_अगर
  u8 *pBlob = 0;
  u8 *p = 0;

  निश्चित( argc==1 || argc==2 );
  अगर( argc==2 ){
    zPass = sqlite3_मूल्य_text(argv[1]);
    nPass = sqlite3_मूल्य_bytes(argv[1]);
  }
  अगर( !zHex || !zPass ) वापस;

  p = pBlob = contextMalloc(pCtx, (nHex/2)+1);
  अगर( pBlob ){
    u8 c;                         /* Most significant digit of next byte */
    u8 d;                         /* Least significant digit of next byte */

    जबतक( (c = *zHex)!=0x00 ){
      जबतक( !sqlite3Isxdigit(c) ){
        u32 ch = Utf8Read(zHex);
        निश्चित( zHex<=zEnd );
        अगर( !strContainsChar(zPass, nPass, ch) ) जाओ unhex_null;
        c = *zHex;
        अगर( c==0x00 ) जाओ unhex_done;
      }
      zHex++;
      निश्चित( *zEnd==0x00 );
      निश्चित( zHex<=zEnd );
      d = *(zHex++);
      अगर( !sqlite3Isxdigit(d) ) जाओ unhex_null;
      *(p++) = (sqlite3HexToInt(c)<<4) | sqlite3HexToInt(d);
    }
  }

 unhex_done:
  sqlite3_परिणाम_blob(pCtx, pBlob, (p - pBlob), sqlite3_free);
  वापस;

 unhex_null:
  sqlite3_free(pBlob);
  वापस;
}


/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
अटल व्योम zeroblobFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  i64 n;
  पूर्णांक rc;
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_मूल्य_int64(argv[0]);
  अगर( n<0 ) n = 0;
  rc = sqlite3_परिणाम_zeroblob64(context, n); /* IMP: R-00293-64994 */
  अगर( rc ){
    sqlite3_परिणाम_error_code(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
अटल व्योम replaceFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर अचिन्हित अक्षर *zStr;        /* The input string A */
  स्थिर अचिन्हित अक्षर *zPattern;    /* The pattern string B */
  स्थिर अचिन्हित अक्षर *zRep;        /* The replacement string C */
  अचिन्हित अक्षर *zOut;              /* The output */
  पूर्णांक nStr;                /* Size of zStr */
  पूर्णांक nPattern;            /* Size of zPattern */
  पूर्णांक nRep;                /* Size of zRep */
  i64 nOut;                /* Maximum size of zOut */
  पूर्णांक loopLimit;           /* Last zStr[] that might match zPattern[] */
  i64 i, j;                /* Loop counters */
  अचिन्हित cntExpand;      /* Number zOut expansions */
  sqlite3 *db = sqlite3_context_db_handle(context);

  निश्चित( argc==3 );
  UNUSED_PARAMETER(argc);
  zStr = sqlite3_मूल्य_text(argv[0]);
  अगर( zStr==0 ) वापस;
  nStr = sqlite3_मूल्य_bytes(argv[0]);
  निश्चित( zStr==sqlite3_मूल्य_text(argv[0]) );  /* No encoding change */
  zPattern = sqlite3_मूल्य_text(argv[1]);
  अगर( zPattern==0 ){
    निश्चित( sqlite3_मूल्य_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    वापस;
  }
  अगर( zPattern[0]==0 ){
    निश्चित( sqlite3_मूल्य_type(argv[1])!=SQLITE_NULL );
    sqlite3_परिणाम_text(context, (स्थिर अक्षर*)zStr, nStr, SQLITE_TRANSIENT);
    वापस;
  }
  nPattern = sqlite3_मूल्य_bytes(argv[1]);
  निश्चित( zPattern==sqlite3_मूल्य_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_मूल्य_text(argv[2]);
  अगर( zRep==0 ) वापस;
  nRep = sqlite3_मूल्य_bytes(argv[2]);
  निश्चित( zRep==sqlite3_मूल्य_text(argv[2]) );
  nOut = nStr + 1;
  निश्चित( nOut<SQLITE_MAX_लंबाई );
  zOut = contextMalloc(context, nOut);
  अगर( zOut==0 ){
    वापस;
  }
  loopLimit = nStr - nPattern; 
  cntExpand = 0;
  क्रम(i=j=0; i<=loopLimit; i++){
    अगर( zStr[i]!=zPattern[0] || स_भेद(&zStr[i], zPattern, nPattern) ){
      zOut[j++] = zStr[i];
    }अन्यथा{
      अगर( nRep>nPattern ){
        nOut += nRep - nPattern;
        testcase( nOut-1==db->aLimit[SQLITE_LIMIT_लंबाई] );
        testcase( nOut-2==db->aLimit[SQLITE_LIMIT_लंबाई] );
        अगर( nOut-1>db->aLimit[SQLITE_LIMIT_लंबाई] ){
          sqlite3_परिणाम_error_toobig(context);
          sqlite3_free(zOut);
          वापस;
        }
        cntExpand++;
        अगर( (cntExpand&(cntExpand-1))==0 ){
          /* Grow the size of the output buffer only on substitutions
          ** whose index is a power of two: 1, 2, 4, 8, 16, 32, ... */
          u8 *zOld;
          zOld = zOut;
          zOut = sqlite3Realloc(zOut, (पूर्णांक)nOut + (nOut - nStr - 1));
          अगर( zOut==0 ){
            sqlite3_परिणाम_error_nomem(context);
            sqlite3_free(zOld);
            वापस;
          }
        }
      }
      स_नकल(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern-1;
    }
  }
  निश्चित( j+nStr-i+1<=nOut );
  स_नकल(&zOut[j], &zStr[i], nStr-i);
  j += nStr - i;
  निश्चित( j<=nOut );
  zOut[j] = 0;
  sqlite3_परिणाम_text(context, (अक्षर*)zOut, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
अटल व्योम trimFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर अचिन्हित अक्षर *zIn;         /* Input string */
  स्थिर अचिन्हित अक्षर *zCharSet;    /* Set of characters to trim */
  अचिन्हित पूर्णांक nIn;                 /* Number of bytes in input */
  पूर्णांक flags;                        /* 1: trimleft  2: trimright  3: trim */
  पूर्णांक i;                            /* Loop counter */
  अचिन्हित पूर्णांक *aLen = 0;           /* Length of each character in zCharSet */
  अचिन्हित अक्षर **azChar = 0;       /* Individual characters in zCharSet */
  पूर्णांक nChar;                        /* Number of characters in zCharSet */

  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ){
    वापस;
  }
  zIn = sqlite3_मूल्य_text(argv[0]);
  अगर( zIn==0 ) वापस;
  nIn = (अचिन्हित)sqlite3_मूल्य_bytes(argv[0]);
  निश्चित( zIn==sqlite3_मूल्य_text(argv[0]) );
  अगर( argc==1 ){
    अटल स्थिर अचिन्हित lenOne[] = { 1 };
    अटल अचिन्हित अक्षर * स्थिर azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (अचिन्हित*)lenOne;
    azChar = (अचिन्हित अक्षर **)azOne;
    zCharSet = 0;
  }अन्यथा अगर( (zCharSet = sqlite3_मूल्य_text(argv[1]))==0 ){
    वापस;
  }अन्यथा{
    स्थिर अचिन्हित अक्षर *z;
    क्रम(z=zCharSet, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    अगर( nChar>0 ){
      azChar = contextMalloc(context,
                     ((i64)nChar)*(माप(अक्षर*)+माप(अचिन्हित)));
      अगर( azChar==0 ){
        वापस;
      }
      aLen = (अचिन्हित*)&azChar[nChar];
      क्रम(z=zCharSet, nChar=0; *z; nChar++){
        azChar[nChar] = (अचिन्हित अक्षर *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (अचिन्हित)(z - azChar[nChar]);
      }
    }
  }
  अगर( nChar>0 ){
    flags = SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(context));
    अगर( flags & 1 ){
      जबतक( nIn>0 ){
        अचिन्हित पूर्णांक len = 0;
        क्रम(i=0; i<nChar; i++){
          len = aLen[i];
          अगर( len<=nIn && स_भेद(zIn, azChar[i], len)==0 ) अवरोध;
        }
        अगर( i>=nChar ) अवरोध;
        zIn += len;
        nIn -= len;
      }
    }
    अगर( flags & 2 ){
      जबतक( nIn>0 ){
        अचिन्हित पूर्णांक len = 0;
        क्रम(i=0; i<nChar; i++){
          len = aLen[i];
          अगर( len<=nIn && स_भेद(&zIn[nIn-len],azChar[i],len)==0 ) अवरोध;
        }
        अगर( i>=nChar ) अवरोध;
        nIn -= len;
      }
    }
    अगर( zCharSet ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_परिणाम_text(context, (अक्षर*)zIn, nIn, SQLITE_TRANSIENT);
}

/* The core implementation of the CONCAT(...) and CONCAT_WS(SEP,...)
** functions.
**
** Return a string value that is the concatenation of all non-null
** entries in argv[].  Use zSep as the separator.
*/
अटल व्योम concatFuncCore(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv,
  पूर्णांक nSep,
  स्थिर अक्षर *zSep
){
  i64 j, n = 0;
  पूर्णांक i;
  पूर्णांक bNotNull = 0;   /* True after at least NOT NULL argument seen */
  अक्षर *z;
  क्रम(i=0; i<argc; i++){
    n += sqlite3_मूल्य_bytes(argv[i]);
  }
  n += (argc-1)*(i64)nSep;
  z = sqlite3_malloc64(n+1);
  अगर( z==0 ){
    sqlite3_परिणाम_error_nomem(context);
    वापस;
  }
  j = 0;
  क्रम(i=0; i<argc; i++){
    अगर( sqlite3_मूल्य_type(argv[i])!=SQLITE_NULL ){
      पूर्णांक k = sqlite3_मूल्य_bytes(argv[i]);
      स्थिर अक्षर *v = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
      अगर( v!=0 ){
        अगर( bNotNull && nSep>0 ){
          स_नकल(&z[j], zSep, nSep);
          j += nSep;
        }
        स_नकल(&z[j], v, k);
        j += k;
        bNotNull = 1;
      }
    }
  }
  z[j] = 0;
  निश्चित( j<=n );
  sqlite3_परिणाम_text64(context, z, j, sqlite3_free, SQLITE_UTF8_ZT);
}

/*
** The CONCAT(...) function.  Generate a string result that is the
** concatentation of all non-null arguments.
*/
अटल व्योम concatFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
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
अटल व्योम concatwsFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक nSep = sqlite3_मूल्य_bytes(argv[0]);
  स्थिर अक्षर *zSep = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  अगर( zSep==0 ) वापस;
  concatFuncCore(context, argc-1, argv+1, nSep, zSep);
}


#अगर_घोषित SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
/*
** The "unknown" function is automatically substituted in place of
** any unrecognized function name when doing an EXPLAIN or EXPLAIN QUERY PLAN
** when the SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION compile-time option is used.
** When the "sqlite3" command-line shell is built using this functionality,
** that allows an EXPLAIN or EXPLAIN QUERY PLAN for complex queries
** involving application-defined functions to be examined in a generic
** sqlite3 shell.
*/
अटल व्योम unknownFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  /* no-op */
  (व्योम)context;
  (व्योम)argc;
  (व्योम)argv;
}
#पूर्ण_अगर /*SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION*/


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#अगर_घोषित SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X.
*/
अटल व्योम soundexFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  अक्षर z_परिणाम[8];
  स्थिर u8 *zIn;
  पूर्णांक i, j;
  अटल स्थिर अचिन्हित अक्षर iCode[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  निश्चित( argc==1 );
  zIn = (u8*)sqlite3_मूल्य_text(argv[0]);
  अगर( zIn==0 ) zIn = (u8*)"";
  क्रम(i=0; zIn[i] && !sqlite3Isalpha(zIn[i]); i++){}
  अगर( zIn[i] ){
    u8 prevcode = iCode[zIn[i]&0x7f];
    z_परिणाम[0] = sqlite3Toupper(zIn[i]);
    क्रम(j=1; j<4 && zIn[i]; i++){
      पूर्णांक code = iCode[zIn[i]&0x7f];
      अगर( code>0 ){
        अगर( code!=prevcode ){
          prevcode = code;
          z_परिणाम[j++] = code + '0';
        }
      }अन्यथा{
        prevcode = 0;
      }
    }
    जबतक( j<4 ){
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
#पूर्ण_अगर /* SQLITE_SOUNDEX */

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
/*
** This SQL function just invokes sqlite3OomFault() and then returns
** NULL.  Used for testing only.
*/
अटल व्योम oomFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  sqlite3OomFault(db);
}
#पूर्ण_अगर /* SQLITE_DEBUG || SQLITE_BUILTIN_OOM_FUNCTION */

#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
अटल व्योम loadExt(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  स्थिर अक्षर *zFile = (स्थिर अक्षर *)sqlite3_मूल्य_text(argv[0]);
  स्थिर अक्षर *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  अक्षर *z_Err_संदेश = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  अगर( (db->flags & SQLITE_LoadExtFunc)==0 ){
    sqlite3_परिणाम_error(context, "not authorized", -1);
    वापस;
  }

  अगर( argc==2 ){
    zProc = (स्थिर अक्षर *)sqlite3_मूल्य_text(argv[1]);
  }अन्यथा{
    zProc = 0;
  }
  अगर( zFile && sqlite3_load_extension(db, zFile, zProc, &z_Err_संदेश) ){
    sqlite3_परिणाम_error(context, z_Err_संदेश, -1);
    sqlite3_free(z_Err_संदेश);
  }
}
#पूर्ण_अगर


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
प्रकार काष्ठा योग_Ctx योग_Ctx;
काष्ठा योग_Ctx {
  द्विगुन r_योग;      /* Running sum as as a double */
  द्विगुन rErr;      /* Error term for Kahan-Babushka-Neumaier summation */
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
अटल व्योम kahanBabuskaNeumaierStep(
  अस्थिर योग_Ctx *p_योग,
  अस्थिर द्विगुन r
){
  अस्थिर द्विगुन s = p_योग->r_योग;
  अस्थिर द्विगुन t = s + r;
  अगर( भ_असल(s) > भ_असल(r) ){
    p_योग->rErr += (s - t) + r;
  }अन्यथा{
    p_योग->rErr += (r - t) + s;
  }
  p_योग->r_योग = t;
}

/*
** Add a (possibly large) integer to the running sum.
*/
अटल व्योम kahanBabuskaNeumaierStepInt64(अस्थिर योग_Ctx *p_योग, i64 iVal){
  अगर( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iBig, iSm;
    iSm = iVal % 16384;
    iBig = iVal - iSm;
    kahanBabuskaNeumaierStep(p_योग, iBig);
    kahanBabuskaNeumaierStep(p_योग, iSm);
  }अन्यथा{
    kahanBabuskaNeumaierStep(p_योग, (द्विगुन)iVal);
  }
}

/*
** Initialize the Kahan-Babaska-Neumaier sum from a 64-bit integer
*/
अटल व्योम kahanBabuskaNeumaierInit(
  अस्थिर योग_Ctx *p,
  i64 iVal
){
  अगर( iVal<=-4503599627370496LL || iVal>=+4503599627370496LL ){
    i64 iSm = iVal % 16384;
    p->r_योग = (द्विगुन)(iVal - iSm);
    p->rErr = (द्विगुन)iSm;
  }अन्यथा{
    p->r_योग = (द्विगुन)iVal;
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
अटल व्योम योग_Step(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  योग_Ctx *p;
  पूर्णांक type;
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, माप(*p));
  type = sqlite3_मूल्य_numeric_type(argv[0]);
  अगर( p && type!=SQLITE_NULL ){
    p->cnt++;
    अगर( p->approx==0 ){
      अगर( type!=SQLITE_INTEGER ){
        kahanBabuskaNeumaierInit(p, p->i_योग);
        p->approx = 1;
        kahanBabuskaNeumaierStep(p, sqlite3_मूल्य_double(argv[0]));
      }अन्यथा{
        i64 x = p->i_योग;
        अगर( sqlite3AddInt64(&x, sqlite3_मूल्य_int64(argv[0]))==0 ){
          p->i_योग = x;
        }अन्यथा{
          p->ovrfl = 1;
          kahanBabuskaNeumaierInit(p, p->i_योग);
          p->approx = 1;
          kahanBabuskaNeumaierStepInt64(p, sqlite3_मूल्य_int64(argv[0]));
        }
      }
    }अन्यथा{
      अगर( type==SQLITE_INTEGER ){
        kahanBabuskaNeumaierStepInt64(p, sqlite3_मूल्य_int64(argv[0]));
      }अन्यथा{
        p->ovrfl = 0;
        kahanBabuskaNeumaierStep(p, sqlite3_मूल्य_double(argv[0]));
      }
    }
  }
}
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
अटल व्योम योग_Inverse(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य**argv){
  योग_Ctx *p;
  पूर्णांक type;
  निश्चित( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, माप(*p));
  type = sqlite3_मूल्य_numeric_type(argv[0]);
  /* p is always non-NULL because sumStep() will have been called first
  ** to initialize it */
  अगर( ALWAYS(p) && type!=SQLITE_NULL ){
    निश्चित( p->cnt>0 );
    p->cnt--;
    अगर( !p->approx ){
      i64 x = p->i_योग;
      अगर( sqlite3SubInt64(&x, sqlite3_मूल्य_int64(argv[0]))==0 ){
        p->i_योग = x;
        वापस;
      }
      p->ovrfl = 1;
      p->approx = 1;
      kahanBabuskaNeumaierInit(p, p->i_योग);
    }
    अगर( type==SQLITE_INTEGER ){
      i64 iVal = sqlite3_मूल्य_int64(argv[0]);
      अगर( iVal!=SMALLEST_INT64 ){
        kahanBabuskaNeumaierStepInt64(p, -iVal);
      }अन्यथा{
        kahanBabuskaNeumaierStepInt64(p, LARGEST_INT64);
        kahanBabuskaNeumaierStepInt64(p, 1);
      }       
    }अन्यथा{
      kahanBabuskaNeumaierStep(p, -sqlite3_मूल्य_double(argv[0]));
    }
  }
}
#अन्यथा
# define योग_Inverse 0
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */
अटल व्योम योग_Finalize(sqlite3_context *context){
  योग_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  अगर( p && p->cnt>0 ){
    अगर( p->approx ){
      अगर( p->ovrfl ){
        sqlite3_परिणाम_error(context,"integer overflow",-1);
      }अन्यथा अगर( !sqlite3IsOverflow(p->rErr) ){
        sqlite3_परिणाम_double(context, p->r_योग+p->rErr);
      }अन्यथा{
        sqlite3_परिणाम_double(context, p->r_योग);
      }
    }अन्यथा{
      sqlite3_परिणाम_int64(context, p->i_योग);
    }
  }
}
अटल व्योम avgFinalize(sqlite3_context *context){
  योग_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  अगर( p && p->cnt>0 ){
    द्विगुन r;
    अगर( p->approx ){
      r = p->r_योग;
      अगर( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }अन्यथा{
      r = (द्विगुन)(p->i_योग);
    }
    sqlite3_परिणाम_double(context, r/(द्विगुन)p->cnt);
  }
}
अटल व्योम योग_Finalize_2(sqlite3_context *context){
  योग_Ctx *p;
  द्विगुन r = 0.0;
  p = sqlite3_aggregate_context(context, 0);
  अगर( p ){
    अगर( p->approx ){
      r = p->r_योग;
      अगर( !sqlite3IsOverflow(p->rErr) ) r += p->rErr;
    }अन्यथा{
      r = (द्विगुन)(p->i_योग);
    }
  }
  sqlite3_परिणाम_double(context, r);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
प्रकार काष्ठा गिनती_Ctx गिनती_Ctx;
काष्ठा गिनती_Ctx {
  i64 n;
#अगर_घोषित SQLITE_DEBUG
  पूर्णांक bInverse;                   /* True if xInverse() ever called */
#पूर्ण_अगर
};

/*
** Routines to implement the count() aggregate function.
*/
अटल व्योम गिनती_Step(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  गिनती_Ctx *p;
  p = sqlite3_aggregate_context(context, माप(*p));
  अगर( (argc==0 || SQLITE_NULL!=sqlite3_मूल्य_type(argv[0])) && p ){
    p->n++;
  }

#अगर_अघोषित SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  निश्चित( argc==1 || p==0 || p->n>0x7fffffff || p->bInverse
          || p->n==sqlite3_aggregate_गिनती(context) );
#पूर्ण_अगर
}  
अटल व्योम गिनती_Finalize(sqlite3_context *context){
  गिनती_Ctx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_परिणाम_int64(context, p ? p->n : 0);
}
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
अटल व्योम गिनती_Inverse(sqlite3_context *ctx, पूर्णांक argc, sqlite3_मूल्य **argv){
  गिनती_Ctx *p;
  p = sqlite3_aggregate_context(ctx, माप(*p));
  /* p is always non-NULL since countStep() will have been called first */
  अगर( (argc==0 || SQLITE_NULL!=sqlite3_मूल्य_type(argv[0])) && ALWAYS(p) ){
    p->n--;
#अगर_घोषित SQLITE_DEBUG
    p->bInverse = 1;
#पूर्ण_अगर
  }
}  
#अन्यथा
# define गिनती_Inverse 0
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */

/*
** Routines to implement min() and max() aggregate functions.
*/
अटल व्योम minmaxStep(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  UNUSED_PARAMETER(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, माप(*pBest));
  अगर( !pBest ) वापस;

  अगर( sqlite3_मूल्य_type(pArg)==SQLITE_NULL ){
    अगर( pBest->flags ) sqlite3SkipAccumulatorLoad(context);
  }अन्यथा अगर( pBest->flags ){
    पूर्णांक max;
    पूर्णांक cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);
    /* This step function is used for both the min() and max() aggregates,
    ** the only difference between the two being that the sense of the
    ** comparison is inverted. For the max() aggregate, the
    ** sqlite3_user_data() function returns (void *)-1. For min() it
    ** returns (void *)db, where db is the sqlite3* database pointer.
    ** Therefore the next statement sets variable 'max' to 1 for the max()
    ** aggregate, or 0 for min().
    */
    max = sqlite3_user_तथ्य(context)!=0;
    cmp = sqlite3MemCompare(pBest, pArg, pColl);
    अगर( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }अन्यथा{
      sqlite3SkipAccumulatorLoad(context);
    }
  }अन्यथा{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
अटल व्योम min_Max_मूल्य_Finalize(sqlite3_context *context, पूर्णांक b_मूल्य){
  sqlite3_मूल्य *pRes;
  pRes = (sqlite3_मूल्य *)sqlite3_aggregate_context(context, 0);
  अगर( pRes ){
    अगर( pRes->flags ){
      sqlite3_परिणाम_मूल्य(context, pRes);
    }
    अगर( b_मूल्य==0 ) sqlite3VdbeMemRelease(pRes);
  }
}
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
अटल व्योम min_Max_मूल्य(sqlite3_context *context){
  min_Max_मूल्य_Finalize(context, 1);
}
#अन्यथा
# define min_Max_मूल्य 0
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */
अटल व्योम minMaxFinalize(sqlite3_context *context){
  min_Max_मूल्य_Finalize(context, 0);
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
प्रकार काष्ठा {
  StrAccum str;          /* The accumulated concatenation */
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
  पूर्णांक nAccum;            /* Number of strings presently concatenated */
  पूर्णांक n_First_Sep_लंबाई;   /* Used to detect separator length change */
  /* If pnSepLengths!=0, refs an array of inter-string separator lengths,
  ** stored as actually incorporated into presently accumulated result.
  ** (Hence, its slots in use number nAccum-1 between method calls.)
  ** If pnSepLengths==0, nFirstSepLength is the length used throughout.
  */
  पूर्णांक *pnSepLengths;
#पूर्ण_अगर
} GroupConcatCtx;

अटल व्योम groupConcatStep(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  स्थिर अक्षर *zVal;
  GroupConcatCtx *pGCC;
  स्थिर अक्षर *zSep;
  पूर्णांक nVal, nSep;
  निश्चित( argc==1 || argc==2 );
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ) वापस;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, माप(*pGCC));
  अगर( pGCC ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    पूर्णांक firstTerm = pGCC->str.mxAlloc==0;
    pGCC->str.mxAlloc = db->aLimit[SQLITE_LIMIT_लंबाई];
    अगर( argc==1 ){
      अगर( !firstTerm ){
        sqlite3_str_appendchar(&pGCC->str, 1, ',');
      }
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      अन्यथा{
        pGCC->n_First_Sep_लंबाई = 1;
      }
#पूर्ण_अगर
    }अन्यथा अगर( !firstTerm ){
      zSep = (अक्षर*)sqlite3_मूल्य_text(argv[1]);
      nSep = sqlite3_मूल्य_bytes(argv[1]);
      अगर( zSep ){
        sqlite3_str_append(&pGCC->str, zSep, nSep);
      }
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      अन्यथा{
        nSep = 0;
      }
      अगर( nSep != pGCC->n_First_Sep_लंबाई || pGCC->pnSepLengths != 0 ){
        पूर्णांक *pnsl = pGCC->pnSepLengths;
        अगर( pnsl == 0 ){
          /* First separator length variation seen, start tracking them. */
          pnsl = (पूर्णांक*)sqlite3_malloc64((pGCC->nAccum+1) * माप(पूर्णांक));
          अगर( pnsl!=0 ){
            पूर्णांक i = 0, nA = pGCC->nAccum-1;
            जबतक( i<nA ) pnsl[i++] = pGCC->n_First_Sep_लंबाई;
          }
        }अन्यथा{
          pnsl = (पूर्णांक*)sqlite3_realloc64(pnsl, pGCC->nAccum * माप(पूर्णांक));
        }
        अगर( pnsl!=0 ){
          अगर( ALWAYS(pGCC->nAccum>0) ){
            pnsl[pGCC->nAccum-1] = nSep;
          }
          pGCC->pnSepLengths = pnsl;
        }अन्यथा{
          sqlite3StrAccumSetError(&pGCC->str, SQLITE_NOMEM);
        }
      }
#पूर्ण_अगर
    }
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
    अन्यथा{
      pGCC->n_First_Sep_लंबाई = sqlite3_मूल्य_bytes(argv[1]);
    }
    pGCC->nAccum += 1;
#पूर्ण_अगर
    zVal = (अक्षर*)sqlite3_मूल्य_text(argv[0]);
    nVal = sqlite3_मूल्य_bytes(argv[0]);
    अगर( zVal ) sqlite3_str_append(&pGCC->str, zVal, nVal);
  }
}

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
अटल व्योम groupConcatInverse(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  GroupConcatCtx *pGCC;
  निश्चित( argc==1 || argc==2 );
  (व्योम)argc;  /* Suppress unused parameter warning */
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ) वापस;
  pGCC = (GroupConcatCtx*)sqlite3_aggregate_context(context, माप(*pGCC));
  /* pGCC is always non-NULL since groupConcatStep() will have always
  ** run first to initialize it */
  अगर( ALWAYS(pGCC) ){
    पूर्णांक nVS;  /* Number of characters to remove */
    /* Must call sqlite3_value_text() to convert the argument into text prior
    ** to invoking sqlite3_value_bytes(), in case the text encoding is UTF16 */
    (व्योम)sqlite3_मूल्य_text(argv[0]);
    nVS = sqlite3_मूल्य_bytes(argv[0]);
    pGCC->nAccum -= 1;
    अगर( pGCC->pnSepLengths!=0 ){
      निश्चित(pGCC->nAccum >= 0);
      अगर( pGCC->nAccum>0 ){
        nVS += *pGCC->pnSepLengths;
        स_हटाओ(pGCC->pnSepLengths, pGCC->pnSepLengths+1,
               (pGCC->nAccum-1)*माप(पूर्णांक));
      }
    }अन्यथा{
      /* If removing single accumulated string, harmlessly over-do. */
      nVS += pGCC->n_First_Sep_लंबाई;
    }
    अगर( nVS>=(पूर्णांक)pGCC->str.nChar ){
      pGCC->str.nChar = 0;
    }अन्यथा{
      pGCC->str.nChar -= nVS;
      स_हटाओ(pGCC->str.zText, &pGCC->str.zText[nVS], pGCC->str.nChar);
    }
    अगर( pGCC->nAccum==0 ){
      pGCC->str.mxAlloc = 0;
      sqlite3_free(pGCC->pnSepLengths);
      pGCC->pnSepLengths = 0;
    }
  }
}
#अन्यथा
# define groupConcatInverse 0
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */
अटल व्योम groupConcatFinalize(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  अगर( pGCC ){
    sqlite3_परिणाम_str(context, &pGCC->str, SQLITE_XFER);
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
    sqlite3_free(pGCC->pnSepLengths);
#पूर्ण_अगर
  }
}
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
अटल व्योम group_Concat_मूल्य(sqlite3_context *context){
  GroupConcatCtx *pGCC
    = (GroupConcatCtx*)sqlite3_aggregate_context(context, 0);
  अगर( pGCC && pGCC->nAccum>0 ){
    sqlite3_परिणाम_str(context, &pGCC->str, SQLITE_COPY);
  }
}
#अन्यथा
# define group_Concat_मूल्य 0
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
व्योम sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db){
  पूर्णांक rc = sqlite3_overload_function(db, "MATCH", 2);
  निश्चित( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  अगर( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Re-register the built-in LIKE functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.
*/
व्योम sqlite3RegisterLikeFunctions(sqlite3 *db, पूर्णांक caseSensitive){
  FuncDef *pDef;
  काष्ठा compareInfo *pInfo;
  पूर्णांक flags;
  पूर्णांक nArg;
  अगर( caseSensitive ){
    pInfo = (काष्ठा compareInfo*)&likeInfoAlt;
    flags = SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE;
  }अन्यथा{
    pInfo = (काष्ठा compareInfo*)&likeInfoNorm;
    flags = SQLITE_FUNC_LIKE;
  }
  क्रम(nArg=2; nArg<=3; nArg++){
    sqlite3CreateFunc(db, "like", nArg, SQLITE_UTF8, pInfo, likeFunc, 
                      0, 0, 0, 0, 0);
    pDef = sqlite3FindFunction(db, "like", nArg, SQLITE_UTF8, 0);
    निश्चित( pDef!=0 ); /* The sqlite3CreateFunc() call above cannot fail
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
पूर्णांक sqlite3IsLikeFunction(sqlite3 *db, व्यंजक *p_व्यंजक, पूर्णांक *pIsNocase, अक्षर *aWc){
  FuncDef *pDef;
  पूर्णांक n_व्यंजक;
  निश्चित( p_व्यंजक!=0 );
  निश्चित( p_व्यंजक->op==TK_FUNCTION );
  निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
  अगर( !p_व्यंजक->x.pList ){
    वापस 0;
  }
  n_व्यंजक = p_व्यंजक->x.pList->n_व्यंजक;
  निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
  pDef = sqlite3FindFunction(db, p_व्यंजक->u.zToken, n_व्यंजक, SQLITE_UTF8, 0);
#अगर_घोषित SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
  अगर( pDef==0 ) वापस 0;
#पूर्ण_अगर
  अगर( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    वापस 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  स_नकल(aWc, pDef->p_User_तथ्य, 3);
  निश्चित( (अक्षर*)&likeInfoAlt == (अक्षर*)&likeInfoAlt.matchAll );
  निश्चित( &((अक्षर*)&likeInfoAlt)[1] == (अक्षर*)&likeInfoAlt.matchOne );
  निश्चित( &((अक्षर*)&likeInfoAlt)[2] == (अक्षर*)&likeInfoAlt.matchSet );

  अगर( n_व्यंजक<3 ){
    aWc[3] = 0;
  }अन्यथा{
    व्यंजक *pEscape = p_व्यंजक->x.pList->a[2].p_व्यंजक;
    अक्षर *zEscape;
    अगर( pEscape->op!=TK_STRING ) वापस 0;
    निश्चित( !व्यंजक_Has_Property(pEscape, EP_Int_मूल्य) );
    zEscape = pEscape->u.zToken;
    अगर( zEscape[0]==0 || zEscape[1]!=0 ) वापस 0;
    अगर( zEscape[0]==aWc[0] ) वापस 0;
    अगर( zEscape[0]==aWc[1] ) वापस 0;
    aWc[3] = zEscape[0];
  }

  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  वापस 1;
}

/* Mathematical Constants */
#अगर_अघोषित M_PI
# define M_PI   3.141592653589793238462643383279502884
#पूर्ण_अगर
#अगर_अघोषित M_LN10
# define M_LN10 2.302585092994045684017991454684364208
#पूर्ण_अगर
#अगर_अघोषित M_LN2
# define M_LN2  0.693147180559945309417232121458176568
#पूर्ण_अगर


/* Extra math functions that require linking with -lm
*/
#अगर_घोषित SQLITE_ENABLE_MATH_FUNCTIONS
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
अटल व्योम ceilingFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  निश्चित( argc==1 );
  चयन( sqlite3_मूल्य_numeric_type(argv[0]) ){
    हाल SQLITE_INTEGER: {
       sqlite3_परिणाम_int64(context, sqlite3_मूल्य_int64(argv[0]));
       अवरोध;
    }
    हाल SQLITE_FLOAT: {
       द्विगुन (*x)(द्विगुन) = (द्विगुन(*)(द्विगुन))sqlite3_user_तथ्य(context);
       sqlite3_परिणाम_double(context, x(sqlite3_मूल्य_double(argv[0])));
       अवरोध;
    }
    शेष: {
       अवरोध;
    }
  }
}

/*
** On some systems, ceil() and floor() are intrinsic function.  You are
** unable to take a pointer to these functions.  Hence, we here wrap them
** in our own actual functions.
*/
अटल द्विगुन xCeil(द्विगुन x){ वापस उच्चमान(x); }
अटल द्विगुन xFloor(द्विगुन x){ वापस न्यूनमान(x); }

/*
** Some systems do not have log2() and log10() in their standard math
** libraries.
*/
#अगर defined(HAVE_LOG10) && HAVE_LOG10==0
# define log10(X) (0.4342944819032517867*log(X))
#पूर्ण_अगर
#अगर defined(HAVE_LOG2) && HAVE_LOG2==0
# define log2(X) (1.442695040888963456*log(X))
#पूर्ण_अगर


/*
** Implementation of SQL functions:
**
**   ln(X)       - natural logarithm
**   log(X)      - log X base 10
**   log10(X)    - log X base 10
**   log(B,X)    - log X base B
*/
अटल व्योम logFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  द्विगुन x, b, ans;
  निश्चित( argc==1 || argc==2 );
  चयन( sqlite3_मूल्य_numeric_type(argv[0]) ){
    हाल SQLITE_INTEGER:
    हाल SQLITE_FLOAT:
      x = sqlite3_मूल्य_double(argv[0]);
      अगर( x<=0.0 ) वापस;
      अवरोध;
    शेष:
      वापस;
  }
  अगर( argc==2 ){
    चयन( sqlite3_मूल्य_numeric_type(argv[1]) ){
      हाल SQLITE_INTEGER:
      हाल SQLITE_FLOAT:
        b = log(x);
        अगर( b<=0.0 ) वापस;
        x = sqlite3_मूल्य_double(argv[1]);
        अगर( x<=0.0 ) वापस;
        अवरोध;
     शेष:
        वापस;
    }
    ans = log(x)/b;
  }अन्यथा{
    चयन( SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(context)) ){
      हाल 1:
        ans = log10(x);
        अवरोध;
      हाल 2:
        ans = log2(x);
        अवरोध;
      शेष:
        ans = log(x);
        अवरोध;
    }
  }
  sqlite3_परिणाम_double(context, ans);
}

/*
** Functions to converts degrees to radians and radians to degrees.
*/
अटल द्विगुन degToRad(द्विगुन x){ वापस x*(M_PI/180.0); }
अटल द्विगुन radToDeg(द्विगुन x){ वापस x*(180.0/M_PI); }

/*
** Implementation of 1-argument SQL math functions:
**
**   exp(X)  - Compute e to the X-th power
*/
अटल व्योम math1Func(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक type0;
  द्विगुन v0, ans;
  द्विगुन (*x)(द्विगुन);
  निश्चित( argc==1 );
  type0 = sqlite3_मूल्य_numeric_type(argv[0]);
  अगर( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) वापस;
  v0 = sqlite3_मूल्य_double(argv[0]);
  x = (द्विगुन(*)(द्विगुन))sqlite3_user_तथ्य(context);
  ans = x(v0);
  sqlite3_परिणाम_double(context, ans);
}

/*
** Implementation of 2-argument SQL math functions:
**
**   power(X,Y)  - Compute X to the Y-th power
*/
अटल व्योम math2Func(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक type0, type1;
  द्विगुन v0, v1, ans;
  द्विगुन (*x)(द्विगुन,द्विगुन);
  निश्चित( argc==2 );
  type0 = sqlite3_मूल्य_numeric_type(argv[0]);
  अगर( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) वापस;
  type1 = sqlite3_मूल्य_numeric_type(argv[1]);
  अगर( type1!=SQLITE_INTEGER && type1!=SQLITE_FLOAT ) वापस;
  v0 = sqlite3_मूल्य_double(argv[0]);
  v1 = sqlite3_मूल्य_double(argv[1]);
  x = (द्विगुन(*)(द्विगुन,द्विगुन))sqlite3_user_तथ्य(context);
  ans = x(v0, v1);
  sqlite3_परिणाम_double(context, ans);
}

/*
** Implementation of 0-argument pi() function.
*/
अटल व्योम piFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  निश्चित( argc==0 );
  (व्योम)argv;
  sqlite3_परिणाम_double(context, M_PI);
}

#पूर्ण_अगर /* SQLITE_ENABLE_MATH_FUNCTIONS */

/*
** Implementation of sign(X) function.
*/
अटल व्योम signFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक type0;
  द्विगुन x;
  UNUSED_PARAMETER(argc);
  निश्चित( argc==1 );
  type0 = sqlite3_मूल्य_numeric_type(argv[0]);
  अगर( type0!=SQLITE_INTEGER && type0!=SQLITE_FLOAT ) वापस;
  x = sqlite3_मूल्य_double(argv[0]);
  sqlite3_परिणाम_int(context, x<0.0 ? -1 : x>0.0 ? +1 : 0);
}

#अगर defined(SQLITE_ENABLE_PERCENTILE)
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
प्रकार काष्ठा Percentile Percentile;
काष्ठा Percentile {
  u64 nAlloc;          /* Number of slots allocated for a[] */
  u64 nUsed;           /* Number of slots actually used in a[] */
  अक्षर bSorted;        /* True if a[] is already in sorted order */
  अक्षर bKeepSorted;    /* True if advantageous to keep a[] sorted */
  अक्षर bPctValid;      /* True if rPct is valid */
  द्विगुन rPct;         /* Fraction.  0.0 to 1.0 */
  द्विगुन *a;           /* Array of Y values */
};

/*
** Return TRUE if the input floating-point number is an infinity.
*/
अटल पूर्णांक percentIsInfinity(द्विगुन r){
  sqlite3_uint64 u;
  निश्चित( माप(u)==माप(r) );
  स_नकल(&u, &r, माप(u));
  वापस ((u>>52)&0x7ff)==0x7ff;
}

/*
** Return TRUE if two doubles differ by 0.001 or less.
*/
अटल पूर्णांक percent_Same_मूल्य(द्विगुन a, द्विगुन b){
  a -= b;
  वापस a>=-0.001 && a<=0.001;
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
अटल i64 percent_द्विआधारी_खोज(Percentile *p, द्विगुन y, पूर्णांक bExact){
  i64 iFirst = 0;                   /* First element of search range */
  i64 iLast = (i64)p->nUsed - 1;    /* Last element of search range */
  जबतक( iLast>=iFirst ){
    i64 iMid = (iFirst+iLast)/2;
    द्विगुन x = p->a[iMid];
    अगर( x<y ){
      iFirst = iMid + 1;
    }अन्यथा अगर( x>y ){
      iLast = iMid - 1;
    }अन्यथा{
      वापस iMid;
    }
  }
  अगर( bExact ) वापस -1;
  वापस iFirst;
}

/*
** Generate an error for a percentile function.
**
** The error format string must have exactly one occurrence of "%%s()"
** (with two '%' characters).  That substring will be replaced by the name
** of the function.
*/
अटल व्योम percentError(sqlite3_context *pCtx, स्थिर अक्षर *zFormat, ...){
  अक्षर *zMsg1;
  अक्षर *zMsg2;
  बहु_सूची ap;

  बहु_शुरू(ap, zFormat);
  zMsg1 = sqlite3_vmprintf(zFormat, ap);
  बहु_पूर्ण(ap);
  zMsg2 = zMsg1 ? sqlite3_mprintf(zMsg1, sqlite3VdbeFuncName(pCtx)) : 0;
  sqlite3_परिणाम_error(pCtx, zMsg2, -1);
  sqlite3_free(zMsg1);
  sqlite3_free(zMsg2);
}

/*
** The "step" function for percentile(Y,P) is called once for each
** input row.
*/
अटल व्योम percentStep(sqlite3_context *pCtx, पूर्णांक argc, sqlite3_मूल्य **argv){
  Percentile *p;
  द्विगुन rPct;
  पूर्णांक eType;
  द्विगुन y;
  निश्चित( argc==2 || argc==1 );

  अगर( argc==1 ){
    /* Requirement 13:  median(Y) is the same as percentile(Y,50). */
    rPct = 0.5;
  }अन्यथा{
    /* P must be a number between 0 and 100 for percentile() or between
    ** 0.0 and 1.0 for percentile_cont() and percentile_disc().
    **
    ** The user-data is an integer which is 10 times the upper bound.
    */
    द्विगुन mxFrac = (SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(pCtx))&2)? 100.0 : 1.0;
    eType = sqlite3_मूल्य_numeric_type(argv[1]);
    rPct = sqlite3_मूल्य_double(argv[1])/mxFrac;
    अगर( (eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT)
     || rPct<0.0 || rPct>1.0
    ){
      percentError(pCtx, "the fraction argument to %%s()"
                        " is not between 0.0 and %.1f",
                        (द्विगुन)mxFrac);
      वापस;
    }
  }

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, माप(*p));
  अगर( p==0 ) वापस;

  /* Remember the P value.  Throw an error if the P value is different
  ** from any prior row, per Requirement (2). */
  अगर( !p->bPctValid ){
    p->rPct = rPct;
    p->bPctValid = 1;
  }अन्यथा अगर( !percent_Same_मूल्य(p->rPct,rPct) ){
    percentError(pCtx, "the fraction argument to %%s()"
                      " is not the same for all input rows");
    वापस;
  }

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_मूल्य_type(argv[0]);
  अगर( eType==SQLITE_NULL ) वापस;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  अगर( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    percentError(pCtx, "input to %%s() is not numeric");
    वापस;
  }

  /* Throw an error if the Y value is infinity or NaN */
  y = sqlite3_मूल्य_double(argv[0]);
  अगर( percentIsInfinity(y) ){
    percentError(pCtx, "Inf input to %%s()");
    वापस;
  }

  /* Allocate and store the Y */
  अगर( p->nUsed>=p->nAlloc ){
    u64 n = p->nAlloc*2 + 250;
    द्विगुन *a = sqlite3_realloc64(p->a, माप(द्विगुन)*n);
    अगर( a==0 ){
      sqlite3_free(p->a);
      स_रखो(p, 0, माप(*p));
      sqlite3_परिणाम_error_nomem(pCtx);
      वापस;
    }
    p->nAlloc = n;
    p->a = a;
  }
  अगर( p->nUsed==0 ){
    p->a[p->nUsed++] = y;
    p->bSorted = 1;
  }अन्यथा अगर( !p->bSorted || y>=p->a[p->nUsed-1] ){
    p->a[p->nUsed++] = y;
  }अन्यथा अगर( p->bKeepSorted ){
    i64 i;
    i = percent_द्विआधारी_खोज(p, y, 0);
    अगर( i<(पूर्णांक)p->nUsed ){
      स_हटाओ(&p->a[i+1], &p->a[i], (p->nUsed-i)*माप(p->a[0]));
    }
    p->a[i] = y;
    p->nUsed++;
  }अन्यथा{
    p->a[p->nUsed++] = y;
    p->bSorted = 0;
  }
}

/*
** Interchange two doubles.
*/
#घोषणा SWAP_DOUBLE(X,Y)  {द्विगुन ttt=(X);(X)=(Y);(Y)=ttt;}

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
अटल व्योम percentSort(
  द्विगुन *a,                      /* Array to sort */
  अचिन्हित पूर्णांक n,                 /* Number of elements in array a[] */
  पूर्णांक iReq                        /* Element caller cares about (or -ve) */
){
  पूर्णांक iLt;       /* Entries before a[iLt] are less than or equal to rPivot */
  पूर्णांक iGt;       /* Entries a[iGt] and after are greater or equal to rPivot */
  पूर्णांक i;         /* Loop counter */
  द्विगुन rPivot; /* The pivot value */

  निश्चित( n>=2 );
  करो{
    /* Put the first, middle, and last elements in sorted order.
    ** After doing so, return immediately if the array contains
    ** three or fewer elements as there is nothing more to do.
    */
    अगर( a[0]>a[n-1] ){
      SWAP_DOUBLE(a[0],a[n-1])
    }
    अगर( n==2 ) वापस;
    iGt = n-1;
    i = n/2;
    अगर( a[0]>a[i] ){
      SWAP_DOUBLE(a[0],a[i])
    }अन्यथा अगर( a[i]>a[iGt] ){
      SWAP_DOUBLE(a[i],a[iGt])
    }
    अगर( n==3 ) वापस;

    /* Take the value of the middle element as the pivot.  Shuffle
    ** values around so that all elements less than the pivot come
    ** before all elements greater than the pivot.
    */
    rPivot = a[i];
    iLt = i = 1;
    करो{
      अगर( a[i]<rPivot ){
        अगर( i>iLt ) SWAP_DOUBLE(a[i],a[iLt])
        iLt++;
        i++;
      }अन्यथा अगर( a[i]>rPivot ){
        करो{
          iGt--;
        }जबतक( iGt>i && a[iGt]>rPivot );
        SWAP_DOUBLE(a[i],a[iGt])
      }अन्यथा{
        i++;
      }
    }जबतक( i<iGt );

    निश्चित( iLt>0 && iLt<iGt && (अचिन्हित)iGt<n );
    testcase( iGt>iLt+1 );
    निश्चित( a[iLt]==rPivot );
    निश्चित( a[iLt-1]<=rPivot );
    निश्चित( a[iGt]>=rPivot );
    निश्चित( a[iLt+1]>=rPivot );

    अगर( iReq>=0 ){
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
      अगर( iReq<iLt ){
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
      अगर( iLt>(पूर्णांक)(n/2) ){
        अगर( n-iGt>=2 ) percentSort(a+iGt, n-iGt, -1);
        n = iLt;
      }अन्यथा{
        अगर( iLt>=2 ) percentSort(a, iLt, -1);
        a += iGt;
        n -= iGt;
      }
    }
  }जबतक( n>=2 );
}

/*
** The "inverse" function for percentile(Y,P) is called to remove a
** row that was previously inserted by "step".
*/
अटल व्योम percentInverse(sqlite3_context *pCtx,पूर्णांक argc,sqlite3_मूल्य **argv){
  Percentile *p;
  पूर्णांक eType;
  द्विगुन y;
  i64 i;
  निश्चित( argc==2 || argc==1 );

  /* Allocate the session context. */
  p = (Percentile*)sqlite3_aggregate_context(pCtx, माप(*p));
  निश्चित( p!=0 );

  /* Ignore rows for which Y is NULL */
  eType = sqlite3_मूल्य_type(argv[0]);
  अगर( eType==SQLITE_NULL ) वापस;

  /* If not NULL, then Y must be numeric.  Otherwise throw an error.
  ** Requirement 4 */
  अगर( eType!=SQLITE_INTEGER && eType!=SQLITE_FLOAT ){
    वापस;
  }

  /* Ignore the Y value if it is infinity or NaN */
  y = sqlite3_मूल्य_double(argv[0]);
  अगर( percentIsInfinity(y) ){
    वापस;
  }
  अगर( p->bSorted==0 ){
    निश्चित( p->nUsed>1 );
    percentSort(p->a, p->nUsed, -1);
    p->bSorted = 1;
  }
  p->bKeepSorted = 1;

  /* Find and remove the row */
  i = percent_द्विआधारी_खोज(p, y, 1);
  अगर( i>=0 ){
    p->nUsed--;
    अगर( i<(पूर्णांक)p->nUsed ){
      स_हटाओ(&p->a[i], &p->a[i+1], (p->nUsed - i)*माप(p->a[0]));
    }
  }
}

/*
** Compute the final output of percentile().  Clean up all allocated
** memory if and only if bIsFinal is true.
*/
अटल व्योम percentCompute(sqlite3_context *pCtx, पूर्णांक bIsFinal){
  Percentile *p;
  पूर्णांक settings = SQLITE_PTR_TO_INT(sqlite3_user_तथ्य(pCtx))&1; /* Discrete? */
  अचिन्हित i1, i2;
  द्विगुन v1, v2;
  द्विगुन ix, vx;
  p = (Percentile*)sqlite3_aggregate_context(pCtx, 0);
  अगर( p==0 ) वापस;
  अगर( p->a==0 ) वापस;
  अगर( p->nUsed ){
    ix = p->rPct*(p->nUsed-1);
    i1 = (अचिन्हित)ix;
    अगर( p->bSorted==0 ){
      /* In cases where bIsFinal is non-zero, setting Percentile.bSorted 
      ** after the percentSort() call here is not technically correct, as 
      ** the array is not fully sorted. But in this case the object will be 
      ** freed below anyway, so it doesn't matter.  */
      निश्चित( p->nUsed>1 );
      percentSort(p->a, p->nUsed, (bIsFinal ? (पूर्णांक)i1 : -1));
      p->bSorted = 1;
    }
    अगर( settings & 1 ){
      vx = p->a[i1];
    }अन्यथा{
      i2 = ix==(द्विगुन)i1 || i1==p->nUsed-1 ? i1 : i1+1;
      v1 = p->a[i1];
      v2 = p->a[i2];
      vx = v1 + (v2-v1)*(ix-i1);
    }
    sqlite3_परिणाम_double(pCtx, vx);
  }
  अगर( bIsFinal ){
    sqlite3_free(p->a);
    स_रखो(p, 0, माप(*p));
  }अन्यथा{
    p->bKeepSorted = 1;
  }
}
अटल व्योम percentFinal(sqlite3_context *pCtx){
  percentCompute(pCtx, 1);
}
अटल व्योम percent_मूल्य(sqlite3_context *pCtx){
  percentCompute(pCtx, 0);
}
/****** End of percentile family of functions ******/
#पूर्ण_अगर /* SQLITE_ENABLE_PERCENTILE */

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Implementation of sqlite_filestat(SCHEMA).
**
** Return JSON text that describes low-level debug/diagnostic information
** about the sqlite3_file object associated with SCHEMA.
*/
अटल व्योम filestatFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  स्थिर अक्षर *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  अगर( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    पूर्णांक rc;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    निश्चित( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    pStr = sqlite3_str_new(db);
    अगर( sqlite3_str_errcode(pStr) ){
      sqlite3_परिणाम_error_nomem(context);
    }अन्यथा{
      sqlite3_str_append(pStr, "{\"db\":", 6);
      rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
      अगर( rc ) sqlite3_str_append(pStr, "null", 4);
      fd = sqlite3PagerJrnlFile(pPager);
      अगर( fd && fd->pMethods!=0 ){
        sqlite3_str_appendall(pStr, ",\"journal\":");
        rc = sqlite3OsFileControl(fd, SQLITE_FCNTL_FILESTAT, pStr);
        अगर( rc ) sqlite3_str_append(pStr, "null", 4);
      }
      sqlite3_str_append(pStr, "}", 1);
      sqlite3_परिणाम_str(context, pStr, SQLITE_FINISH);
    }
    sqlite3BtreeLeave(pBtree);
  }अन्यथा{
    sqlite3_परिणाम_text(context, "{}", 2, SQLITE_STATIC);
  }
}
#पूर्ण_अगर /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Implementation of sqlite_walstat(SCHEMA).
**
** Return JSON text that describes the current state of a WAL file.
** This function is for debugging and analysis only.  It is not part
** of production builds.  This function is not part of the SQLite API
** and is likely to change or be removed in future versions of SQLite.
*/
अटल व्योम walstatFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  स्थिर अक्षर *zDbName;
  sqlite3_str *pStr;
  Btree *pBtree;

  zDbName = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  अगर( pBtree ){
    Pager *pPager;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    निश्चित( pPager!=0 );
    pStr = sqlite3_str_new(db);
    अगर( sqlite3_str_errcode(pStr) ){
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
#पूर्ण_अगर /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#अगर_घोषित SQLITE_DEBUG
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
अटल व्योम fpdecodeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  FpDecode s;
  द्विगुन x;
  पूर्णांक y, z;
  अक्षर zBuf[100];
  UNUSED_PARAMETER(argc);
  निश्चित( argc==3 );
  x = sqlite3_मूल्य_double(argv[0]);
  y = sqlite3_मूल्य_int(argv[1]);
  z = sqlite3_मूल्य_int(argv[2]);
  अगर( z<=0 ) z = 1;
  अगर( y<-100000000 ) y = -100000000;  /* Same limit as printf() */
  sqlite3FpDecode(&s, x, y, z);
  अगर( s.isSpecial==2 ){
    sqlite3_snprintf(माप(zBuf), zBuf, "NaN");
  }अन्यथा{
    sqlite3_snprintf(माप(zBuf), zBuf, "%c%.*s/%d", s.sign, s.n, s.z, s.iDP);
  }
  sqlite3_परिणाम_text(context, zBuf, -1, SQLITE_TRANSIENT);
}
#पूर्ण_अगर /* SQLITE_DEBUG */

#अगर_घोषित SQLITE_DEBUG
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
अटल व्योम parseuriFunc(
  sqlite3_context *ctx,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_str *p_परिणाम;
  स्थिर अक्षर *zVfs;
  स्थिर अक्षर *zUri;
  अचिन्हित पूर्णांक flgs;
  पूर्णांक rc;
  sqlite3_vfs *pVfs = 0;
  अक्षर *zFile = 0;
  अक्षर *zErr = 0;

  अगर( argc<2 ) वापस;
  pVfs = sqlite3_vfs_find(0);
  निश्चित( pVfs );
  zVfs = pVfs->zName;
  zUri = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  अगर( zUri==0 ) वापस;
  flgs = (अचिन्हित पूर्णांक)sqlite3_मूल्य_int(argv[1]);
  rc = sqlite3ParseUri(zVfs, zUri, &flgs, &pVfs, &zFile, &zErr);
  p_परिणाम = sqlite3_str_new(0);
  अगर( !sqlite3_str_errcode(p_परिणाम) ){
    पूर्णांक i;
    sqlite3_str_appendf(p_परिणाम, "rc=%d", rc);
    sqlite3_str_appendf(p_परिणाम, ", flags=0x%x", flgs);
    sqlite3_str_appendf(p_परिणाम, ", vfs=%Q", pVfs ? pVfs->zName: 0);
    sqlite3_str_appendf(p_परिणाम, ", err=%Q", zErr);
    sqlite3_str_appendf(p_परिणाम, ", file=%Q", zFile);
    अगर( zFile ){
      स्थिर अक्षर *z = zFile;
      z += sqlite3Strlen30(z)+1;
      जबतक( z[0] ){
        sqlite3_str_appendf(p_परिणाम, ", %Q", z);
        z += sqlite3Strlen30(z)+1;
      }
      क्रम(i=2; i<argc; i++){
        स्थिर अक्षर *zArg;
        अगर( sqlite3_मूल्य_type(argv[i])==SQLITE_INTEGER ){
          पूर्णांक k = sqlite3_मूल्य_int(argv[i]);
          sqlite3_str_appendf(p_परिणाम, ", '%d:%q'",k,sqlite3_uri_कुंजी(zFile, k));
        }अन्यथा अगर( (zArg = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]))!=0 ){
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
#पूर्ण_अगर /* SQLITE_DEBUG */

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
व्योम sqlite3RegisterBuiltinFunctions(व्योम){
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
  अटल FuncDef aBuiltinFunc[] = {
/***** Functions only available with SQLITE_TESTCTRL_INTERNAL_FUNCTIONS *****/
#अगर !defined(SQLITE_UNTESTABLE)
    TEST_FUNC(implies_nonnull_row, 2, INLINEFUNC_implies_nonnull_row, 0),
    TEST_FUNC(व्यंजक_compare,        2, INLINEFUNC_व्यंजक_compare,        0),
    TEST_FUNC(व्यंजक_implies_व्यंजक,   2, INLINEFUNC_व्यंजक_implies_व्यंजक,   0),
    TEST_FUNC(affinity,            1, INLINEFUNC_affinity,            0),
#पूर्ण_अगर /* !defined(SQLITE_UNTESTABLE) */
/***** Regular functions *****/
#अगर_घोषित SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
    SFUNCTION(load_extension,    1, 0, 0, loadExt          ),
    SFUNCTION(load_extension,    2, 0, 0, loadExt          ),
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_used,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#पूर्ण_अगर /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    INLINE_FUNC(unlikely,        1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likelihood,      2, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
    INLINE_FUNC(likely,          1, INLINEFUNC_unlikely, SQLITE_FUNC_UNLIKELY),
#अगर_घोषित SQLITE_ENABLE_OFFSET_SQL_FUNC
    INLINE_FUNC(sqlite_offset,   1, INLINEFUNC_sqlite_offset, 0 ),
#पूर्ण_अगर
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    FUNCTION(sqlite_filestat,    1, 0, 0, filestatFunc     ),
    FUNCTION(sqlite_walstat,     1, 0, 0, walstatFunc      ),
#पूर्ण_अगर
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -3, 0, 1, minmaxFunc       ),
    WAGGREGATE(min, 1, 0, 1, minmaxStep, minMaxFinalize, min_Max_मूल्य, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION(max,               -3, 1, 1, minmaxFunc       ),
    WAGGREGATE(max, 1, 1, 1, minmaxStep, minMaxFinalize, min_Max_मूल्य, 0,
                                 SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(subtype,           1, 0, 0, subtypeFunc,
                                           SQLITE_FUNC_TYPEOF|SQLITE_SUBTYPE),
    FUNCTION2(लंबाई,            1, 0, 0, लंबाई_Func,  SQLITE_FUNC_लंबाई),
    FUNCTION2(octet_लंबाई,      1, 0, 0, bytelengthFunc,SQLITE_FUNC_BYTELEN),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(म_लिखो,            -1, 0, 0, printfFunc       ),
    FUNCTION(format,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(अक्षर,              -1, 0, 0, charFunc         ),
    FUNCTION(असल,                1, 0, 0, absFunc          ),
#अगर_घोषित SQLITE_DEBUG
    FUNCTION(fpdecode,           3, 0, 0, fpdecodeFunc     ),
    FUNCTION(parseuri,          -1, 0, 0, parseuriFunc     ),
#पूर्ण_अगर
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_BUILTIN_OOM_FUNCTION)
    FUNCTION(oom,               -1, 0, 0, oomFunc          ),
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
    FUNCTION(round,              1, 0, 0, roundFunc        ),
    FUNCTION(round,              2, 0, 0, roundFunc        ),
#पूर्ण_अगर
    FUNCTION(upper,              1, 0, 0, upperFunc        ),
    FUNCTION(lower,              1, 0, 0, lowerFunc        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION(unhex,              1, 0, 0, unhexFunc        ),
    FUNCTION(unhex,              2, 0, 0, unhexFunc        ),
    FUNCTION(concat,            -3, 0, 0, concatFunc       ),
    FUNCTION(concat_ws,         -4, 0, 0, concatwsFunc     ),
    INLINE_FUNC(ifnull,          2, INLINEFUNC_coalesce, 0 ),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_version,    0, 0, 0, versionFunc      ),
    DFUNCTION(sqlite_source_id,  0, 0, 0, sourceidFunc     ),
    SFUNCTION(sqlite_log,        2, 0, 0, errlogFunc       ),
    FUNCTION(unistr,             1, 0, 0, unistrFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    FUNCTION(unistr_quote,       1, 1, 0, quoteFunc        ),
    VFUNCTION(last_जोड़ो_rowid, 0, 0, 0, last_जोड़ो_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(योग_changes,     0, 0, 0, योग_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    FUNCTION(substring,          2, 0, 0, substrFunc       ),
    FUNCTION(substring,          3, 0, 0, substrFunc       ),
    WAGGREGATE(योग,   1,0,0, योग_Step, योग_Finalize, योग_Finalize, योग_Inverse, 0),
    WAGGREGATE(योग_2, 1,0,0, योग_Step,योग_Finalize_2,योग_Finalize_2,योग_Inverse, 0),
    WAGGREGATE(avg,   1,0,0, योग_Step, avgFinalize, avgFinalize, योग_Inverse, 0),
    WAGGREGATE(गिनती, 0,0,0, गिनती_Step,
        गिनती_Finalize, गिनती_Finalize, गिनती_Inverse,
        SQLITE_FUNC_गिनती|SQLITE_FUNC_ANYORDER  ),
    WAGGREGATE(गिनती, 1,0,0, गिनती_Step,
        गिनती_Finalize, गिनती_Finalize, गिनती_Inverse, SQLITE_FUNC_ANYORDER ),
    WAGGREGATE(group_concat, 1, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_मूल्य, groupConcatInverse, 0),
    WAGGREGATE(group_concat, 2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_मूल्य, groupConcatInverse, 0),
    WAGGREGATE(string_agg,   2, 0, 0, groupConcatStep,
        groupConcatFinalize, group_Concat_मूल्य, groupConcatInverse, 0),

#अगर_घोषित SQLITE_ENABLE_PERCENTILE
    WAGGREGATE(median,          1,   0,0, percentStep,
        percentFinal, percent_मूल्य, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile,      2, 0x2,0, percentStep,
        percentFinal, percent_मूल्य, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_cont, 2,   0,0, percentStep,
        percentFinal, percent_मूल्य, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
    WAGGREGATE(percentile_disc, 2, 0x1,0, percentStep,
        percentFinal, percent_मूल्य, percentInverse,
        SQLITE_INNOCUOUS|SQLITE_SELFORDER1),
#पूर्ण_अगर /* SQLITE_ENABLE_PERCENTILE */
 
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#अगर_घोषित SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
#अन्यथा
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
    FUNCTION(unknown,           -1, 0, 0, unknownFunc      ),
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_MATH_FUNCTIONS
    MFUNCTION(उच्चमान,              1, xCeil,     ceilingFunc ),
    MFUNCTION(ceiling,           1, xCeil,     ceilingFunc ),
    MFUNCTION(न्यूनमान,             1, xFloor,    ceilingFunc ),
#अगर SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(trunc,             1, trunc,     ceilingFunc ),
#पूर्ण_अगर
    FUNCTION(ln,                 1, 0, 0,      logFunc     ),
    FUNCTION(log,                1, 1, 0,      logFunc     ),
    FUNCTION(log10,              1, 1, 0,      logFunc     ),
    FUNCTION(log2,               1, 2, 0,      logFunc     ),
    FUNCTION(log,                2, 0, 0,      logFunc     ),
    MFUNCTION(exp,               1, exp,       math1Func   ),
    MFUNCTION(घात,               2, घात,       math2Func   ),
    MFUNCTION(power,             2, घात,       math2Func   ),
    MFUNCTION(mod,               2, भ_शेष,      math2Func   ),
    MFUNCTION(acos,              1, acos,      math1Func   ),
    MFUNCTION(asin,              1, asin,      math1Func   ),
    MFUNCTION(atan,              1, atan,      math1Func   ),
    MFUNCTION(atan2,             2, atan2,     math2Func   ),
    MFUNCTION(cos,               1, cos,       math1Func   ),
    MFUNCTION(sin,               1, sin,       math1Func   ),
    MFUNCTION(tan,               1, tan,       math1Func   ),
    MFUNCTION(cosh,              1, cosh,      math1Func   ),
    MFUNCTION(sinh,              1, sinh,      math1Func   ),
    MFUNCTION(tanh,              1, tanh,      math1Func   ),
#अगर SQLITE_HAVE_C99_MATH_FUNCS
    MFUNCTION(acosh,             1, acosh,     math1Func   ),
    MFUNCTION(asinh,             1, asinh,     math1Func   ),
    MFUNCTION(atanh,             1, atanh,     math1Func   ),
#पूर्ण_अगर
    MFUNCTION(वर्ग_मूल,              1, वर्ग_मूल,      math1Func   ),
    MFUNCTION(radians,           1, degToRad,  math1Func   ),
    MFUNCTION(degrees,           1, radToDeg,  math1Func   ),
    MFUNCTION(pi,                0, 0,         piFunc      ),
#पूर्ण_अगर /* SQLITE_ENABLE_MATH_FUNCTIONS */
    FUNCTION(sign,               1, 0, 0,      signFunc    ),
    INLINE_FUNC(coalesce,       -4, INLINEFUNC_coalesce, 0 ),
    INLINE_FUNC(iif,            -4, INLINEFUNC_iif,      0 ),
    INLINE_FUNC(अगर,             -4, INLINEFUNC_iif,      0 ),
  };
#अगर_अघोषित SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#पूर्ण_अगर
  sqlite3WindowFunctions();
  sqlite3RegisterDateTimeFunctions();
  sqlite3RegisterJsonFunctions();
  sqlite3_जोड़ो_Builtin_Funcs(aBuiltinFunc, Array_आकार(aBuiltinFunc));

#अगर 0  /* Enable to print out how the built-in functions are hashed */
  {
    पूर्णांक i;
    FuncDef *p;
    क्रम(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      म_लिखो("FUNC-HASH %02d:", i);
      क्रम(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        पूर्णांक n = sqlite3Strlen30(p->zName);
        पूर्णांक h = p->zName[0] + n;
        निश्चित( p->funcFlags & SQLITE_FUNC_BUILTIN );
        म_लिखो(" %s(%d)", p->zName, h);
      }
      म_लिखो("\n");
    }
  }
#पूर्ण_अगर
}
