/*
** The "printf" code that follows dates from the 1980's.  It is in
** the public domain. 
**
**************************************************************************
**
** This file contains code for a set of "printf"-like routines.  These
** routines format strings much like the printf() from the standard C
** library, though the implementation here has enhancements to support
** SQLite.
*/
#समावेश "sqliteInt.h"

/*
** Conversion types fall into various categories as defined by the
** following enumeration.
*/
#घोषणा etRADIX       0 /* non-decimal integer types.  %x %o */
#घोषणा etFLOAT       1 /* Floating point.  %f */
#घोषणा etEXP         2 /* Exponentional notation. %e and %E */
#घोषणा etGENERIC     3 /* Floating or exponential, depending on exponent. %g */
#घोषणा et_आकार        4 /* Return number of characters processed so far. %n */
#घोषणा etSTRING      5 /* Strings. %s */
#घोषणा etDYNSTRING   6 /* Dynamically allocated strings. %z */
#घोषणा etPERCENT     7 /* Percent symbol. %% */
#घोषणा etCHARX       8 /* Characters. %c */
/* The rest are extensions, not normally found in printf() */
#घोषणा etESCAPE_q    9  /* Strings with '\'' doubled.  %q */
#घोषणा etESCAPE_Q    10 /* Strings with '\'' doubled and enclosed in '',
                            NULL pointers replaced by SQL NULL.  %Q */
#घोषणा etTOKEN       11 /* a pointer to a Token structure */
#घोषणा etSRCITEM     12 /* a pointer to a SrcItem */
#घोषणा etPOINTER     13 /* The %p conversion */
#घोषणा etESCAPE_w    14 /* %w -> Strings with '\"' doubled */
#घोषणा etORDINAL     15 /* %r -> 1st, 2nd, 3rd, 4th, etc.  English only */
#घोषणा etDECIMAL     16 /* %d or %u, but not %x, %o */
#घोषणा etESCAPE_j    17 /* %j -> JSON string literal w/o "..." */
#घोषणा etESCAPE_J    18 /* %J -> JSON string literal with "..." */

#घोषणा etINVALID     19 /* Any unrecognized conversion type */


/*
** An "etByte" is an 8-bit unsigned value.
*/
प्रकार अचिन्हित अक्षर etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
प्रकार काष्ठा et_info {   /* Information about each format field */
  अक्षर fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  अक्षर iNxt;               /* Next with same hash, or 0 for end of chain */
} et_info;

/*
** Allowed values for et_info.flags
*/
#घोषणा FLAG_SIGNED    1     /* True if the value to convert is signed */
#घोषणा FLAG_STRING    4     /* Allow infinite precision */

/*
** The table is searched by hash.  In the case of %C where C is the character
** and that character has ASCII value j, then the hash is j%25.
**
** The order of the entries in fmtinfo[] and the hash chain was entered
** manually, but based on the output of the following TCL script:
*/
#अगर 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
माला_दो "count: $mx"

set mx 25
माला_दो "*********** mx=$mx ************"
क्रम {set r 0} {$r<$mx} {incr r} {
  माला_दो -nonewline [format %2d: $r]
  foreach c [array names n] {
    अगर {($n($c))%$mx==$r} {माला_दो -nonewline " $c"}
  }
  माला_दो ""
}
#पूर्ण_अगर /***** End of script ********/

अटल स्थिर अक्षर aDigits[] = "0123456789ABCDEF0123456789abcdef";
अटल स्थिर अक्षर aHex[]    = "0123456789abcdef";
अटल स्थिर अक्षर aPrefix[] = "-x0\000X0";
अटल स्थिर et_info fmtinfo[25] = {
  /*  0 */  {  'd', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  1 */  {  'e',  0, 1, etEXP,        30, 0,  0 },
  /*  2 */  {  'f',  0, 1, etFLOAT,      0,  0,  0 },
  /*  3 */  {  'g',  0, 1, etGENERIC,    30, 0,  0 },
  /*  4 */  {  'j',  0, 0, etESCAPE_j,   0,  0,  0 },  /* Hash: 6 */
  /*  5 */  {  'i', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  6 */  {  'Q',  0, 4, etESCAPE_Q,   0,  0,  4 },
  /*  7 */  {  'p', 16, 0, etPOINTER,    0,  1,  0 },  /* Hash: 12 */
  /*  8 */  {  'S',  0, 0, etSRCITEM,    0,  0,  0 },
  /*  9 */  {  'T',  0, 0, etTOKEN,      0,  0,  0 },
  /* 10 */  {  'n',  0, 0, et_आकार,       0,  0,  0 },
  /* 11 */  {  'o',  8, 0, etRADIX,      0,  2,  0 },
  /* 12 */  {  '%',  0, 0, etPERCENT,    0,  0,  7 },
  /* 13 */  {  'q',  0, 4, etESCAPE_q,   0,  0, 16 },
  /* 14 */  {  'r', 10, 1, etORDINAL,    0,  0,  0 },
  /* 15 */  {  's',  0, 4, etSTRING,     0,  0,  0 },
  /* 16 */  {  'X', 16, 0, etRADIX,      0,  4,  0 },  /* Hash: 13 */
  /* 17 */  {  'u', 10, 0, etDECIMAL,    0,  0,  0 },
  /* 18 */  {  'w',  0, 4, etESCAPE_w,   0,  0,  0 },  /* Hash: 19 */
  /* 19 */  {  'E',  0, 1, etEXP,        14, 0, 18 },
  /* 20 */  {  'x', 16, 0, etRADIX,      16, 1,  0 },
  /* 21 */  {  'G',  0, 1, etGENERIC,    14, 0,  0 },
  /* 22 */  {  'z',  0, 4, etDYNSTRING,  0,  0,  0 },
  /* 23 */  {  'J',  0, 0, etESCAPE_J,   0,  0,  0 },  /* Hash: 24 */
  /* 24 */  {  'c',  0, 0, etCHARX,      0,  0, 23 }
};

/* Additional Notes:
**
**    %S    Takes a pointer to SrcItem.  Shows name or database.name
**    %!S   Like %S but prefer the zName over the zAlias
*/

/*
** Set the StrAccum object to an error mode.
*/
व्योम sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  निश्चित( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  अगर( p->mxAlloc ) sqlite3_str_reset(p);
  अगर( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
अटल sqlite3_int64 getIntArg(PrintfArguments *p){
  अगर( p->nArg<=p->nUsed ) वापस 0;
  वापस sqlite3_मूल्य_int64(p->apArg[p->nUsed++]);
}
अटल द्विगुन getDoubleArg(PrintfArguments *p){
  अगर( p->nArg<=p->nUsed ) वापस 0.0;
  वापस sqlite3_मूल्य_double(p->apArg[p->nUsed++]);
}
अटल अक्षर *getTextArg(PrintfArguments *p){
  अगर( p->nArg<=p->nUsed ) वापस 0;
  वापस (अक्षर*)sqlite3_मूल्य_text(p->apArg[p->nUsed++]);
}

/*
** Allocate memory for a temporary buffer needed for printf rendering.
**
** If the requested size of the temp buffer is larger than the size
** of the output buffer in pAccum, then cause an SQLITE_TOOBIG error.
** Do the size check before the memory allocation to prevent rogue
** SQL from requesting large allocations using the precision or width
** field of the printf() function.
*/
अटल अक्षर *printf_अस्थायी_Buf(sqlite3_str *pAccum, sqlite3_int64 n){
  अक्षर *z;
  अगर( pAccum->accError ) वापस 0;
  अगर( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    वापस 0;
  }
  z = sqlite3_malloc(n);
  अगर( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  वापस z;
}

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be something smaller, if desired.
*/
#अगर_अघोषित SQLITE_PRINT_BUF_आकार
# define SQLITE_PRINT_BUF_आकार 70
#पूर्ण_अगर
#घोषणा etBUFSIZE SQLITE_PRINT_BUF_आकार  /* Size of the output buffer */

/*
** Hard limit on the precision of floating-point conversions to
** the minimum of SQLITE_PRINTF_PRECISION_LIMIT and 100 million.
*/
#अगर_घोषित SQLITE_PRINTF_PRECISION_LIMIT
# अगर SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# अन्यथा
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#अन्यथा
# define SQLITE_FP_PRECISION_LIMIT 100000000
#पूर्ण_अगर

/* Forward reference */
अटल व्योम sqlite3StrAppend64(sqlite3_str *p, स्थिर अक्षर *z, i64 N);
अटल व्योम sqlite3StrAppendchar64(sqlite3_str *p, i64 N, अक्षर c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
व्योम sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  स्थिर अक्षर *fmt,           /* Format string */
  बहु_सूची ap                 /* arguments */
){
  पूर्णांक c;                     /* Next character in the format string */
  अक्षर *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 लंबाई;                /* Length of the field */
  पूर्णांक idx;                   /* A general purpose loop counter */
  i64 width;                 /* Width of the current field */
  etByte flag_leftjustify;   /* True if "-" flag is present */
  etByte flag_prefix;        /* '+' or ' ' or 0 for prefix */
  etByte flag_alternateform; /* True if "#" flag is present */
  etByte flag_altform2;      /* True if "!" flag is present */
  etByte flag_zeropad;       /* True if field width constant starts with zero */
  etByte flag_long;          /* 1 for the "l" flag, 2 for "ll", 0 by default */
  etByte done;               /* Loop termination flag */
  etByte cThousand;          /* Thousands separator for %d and %u */
  etByte xtype = etINVALID;  /* Conversion paradigm */
  u8 bArgList;               /* True for SQLITE_PRINTF_SQLFUNC */
  अक्षर prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  द्विगुन realvalue;          /* Value for real types */
  स्थिर et_info *infop;      /* Pointer to the appropriate info structure */
  अक्षर *zOut;                /* Rendering buffer */
  पूर्णांक nOut;                  /* Size of the rendering buffer */
  अक्षर *zExtra = 0;          /* Malloced memory used by some conversion */
  पूर्णांक exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  अक्षर buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  निश्चित( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  अगर( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = बहु_तर्क(ap, PrintfArguments*);
    bArgList = 1;
  }अन्यथा{
    bArgList = 0;
  }
  क्रम(; (c=(*fmt))!=0; ++fmt){
    अगर( c!='%' ){
      bufpt = (अक्षर *)fmt;
#अगर HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#अन्यथा
      fmt = म_अक्षर(fmt, '%');
      अगर( fmt==0 ){
        fmt = bufpt + म_माप(bufpt);
      }
#पूर्ण_अगर
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      अगर( *fmt==0 ) अवरोध;
    }
    अगर( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      अवरोध;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    करो{
      चयन( c ){
        हाल '-':   flag_leftjustify = 1;     अवरोध;
        हाल '+':   flag_prefix = '+';        अवरोध;
        हाल ' ':   flag_prefix = ' ';        अवरोध;
        हाल '#':   flag_alternateform = 1;   अवरोध;
        हाल '!':   flag_altform2 = 1;        अवरोध;
        हाल '0':   flag_zeropad = 1;         अवरोध;
        हाल ',':   cThousand = ',';          अवरोध;
        शेष:    done = 1;                 अवरोध;
        हाल 'l': {
          flag_long = 1;
          c = *++fmt;
          अगर( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          अवरोध;
        }
        हाल '1': हाल '2': हाल '3': हाल '4': हाल '5':
        हाल '6': हाल '7': हाल '8': हाल '9': {
          अचिन्हित wx = c - '0';
          जबतक( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#अगर_घोषित SQLITE_PRINTF_PRECISION_LIMIT
          अगर( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#पूर्ण_अगर
          अगर( c!='.' && c!='l' ){
            done = 1;
          }अन्यथा{
            fmt--;
          }
          अवरोध;
        }
        हाल '*': {
          अगर( bArgList ){
            width = (पूर्णांक)getIntArg(pArgList);
          }अन्यथा{
            width = बहु_तर्क(ap,पूर्णांक);
          }
          अगर( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#अगर_घोषित SQLITE_PRINTF_PRECISION_LIMIT
          अगर( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#पूर्ण_अगर
          अगर( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          अवरोध;
        }
        हाल '.': {
          c = *++fmt;
          अगर( c=='*' ){
            अगर( bArgList ){
              precision = (पूर्णांक)getIntArg(pArgList);
            }अन्यथा{
              precision = बहु_तर्क(ap,पूर्णांक);
            }
            अगर( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }अन्यथा{
            अचिन्हित px = 0;
            जबतक( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#अगर_घोषित SQLITE_PRINTF_PRECISION_LIMIT
          अगर( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#पूर्ण_अगर
          अगर( c=='l' ){
            --fmt;
          }अन्यथा{
            done = 1;
          }
          अवरोध;
        }
      }
    }जबतक( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#अगर_घोषित SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    क्रम(idx=0; idx<Array_आकार(fmtinfo); idx++){
      अगर( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        अवरोध;
      }
    }
#अन्यथा
    /* Fast hash-table lookup */
    निश्चित( Array_आकार(fmtinfo)==25 );
    idx = ((अचिन्हित)c) % 25;
    अगर( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }अन्यथा{
      infop = &fmtinfo[0];
      xtype = etINVALID;
    }
#पूर्ण_अगर

    /*
    ** At this point, variables are initialized as follows:
    **
    **   flag_alternateform          TRUE if a '#' is present.
    **   flag_altform2               TRUE if a '!' is present.
    **   flag_prefix                 '+' or ' ' or zero
    **   flag_leftjustify            TRUE if a '-' is present or if the
    **                               field width was negative.
    **   flag_zeropad                TRUE if the width began with 0.
    **   flag_long                   1 for "l", 2 for "ll"
    **   width                       The specified field width.  This is
    **                               always non-negative.  Zero is the default.
    **   precision                   The specified precision.  The default
    **                               is -1.
    **   xtype                       The class of the conversion.
    **   infop                       Pointer to the appropriate info struct.
    */
    निश्चित( width>=0 );
    निश्चित( precision>=(-1) );
    चयन( xtype ){
      हाल etPOINTER:
        flag_long = माप(अक्षर*)==माप(i64) ? 2 :
                     माप(अक्षर*)==माप(दीर्घ पूर्णांक) ? 1 : 0;
        /* no break */ deliberate_fall_through
      हाल etORDINAL:
      हाल etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      हाल etDECIMAL:
        अगर( infop->flags & FLAG_SIGNED ){
          i64 v;
          अगर( bArgList ){
            v = getIntArg(pArgList);
          }अन्यथा अगर( flag_long ){
            अगर( flag_long==2 ){
              v = बहु_तर्क(ap,i64) ;
            }अन्यथा{
              v = बहु_तर्क(ap,दीर्घ पूर्णांक);
            }
          }अन्यथा{
            v = बहु_तर्क(ap,पूर्णांक);
          }
          अगर( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }अन्यथा{
            longvalue = v;
            prefix = flag_prefix;
          }
        }अन्यथा{
          अगर( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }अन्यथा अगर( flag_long ){
            अगर( flag_long==2 ){
              longvalue = बहु_तर्क(ap,u64);
            }अन्यथा{
              longvalue = बहु_तर्क(ap,अचिन्हित दीर्घ पूर्णांक);
            }
          }अन्यथा{
            longvalue = बहु_तर्क(ap,अचिन्हित पूर्णांक);
          }
          prefix = 0;
        }

#अगर WHERETRACE_ENABLED
        अगर( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#पूर्ण_अगर
#अगर TREETRACE_ENABLED
        अगर( xtype==etPOINTER && sqlite3_वृक्ष_Trace & 0x100000 ) longvalue = 0;
#पूर्ण_अगर

        अगर( longvalue==0 ) flag_alternateform = 0;
        अगर( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        अगर( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }अन्यथा{
          u64 n;
          n = (u64)precision + 10;
          अगर( cThousand ) n += precision/3;
          zOut = zExtra = printf_अस्थायी_Buf(pAccum, n);
          अगर( zOut==0 ) वापस;
          nOut = (पूर्णांक)n;
        }
        bufpt = &zOut[nOut-1];
        अगर( xtype==etORDINAL ){
          अटल स्थिर अक्षर zOrd[] = "thstndrd";
          पूर्णांक x = (पूर्णांक)(longvalue % 10);
          अगर( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          स्थिर अक्षर *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          करो{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }जबतक( longvalue>0 );
        }
        लंबाई = &zOut[nOut-1] - bufpt;
        अगर( precision>लंबाई ){                         /* zero pad */
          i64 nn = precision-लंबाई;
          bufpt -= nn;
          स_रखो(bufpt,'0',nn);
          लंबाई = precision;
        }
        अगर( cThousand ){
          i64 nn = (लंबाई - 1)/3;  /* Number of "," to insert */
          i64 ix = (लंबाई - 1)%3 + 1;
          पूर्णांक ii;
          bufpt -= nn;
          क्रम(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            अगर( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        अगर( prefix ) *(--bufpt) = prefix;               /* Add sign */
        अगर( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          स्थिर अक्षर *pre;
          अक्षर x;
          pre = &aPrefix[infop->prefix];
          क्रम(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        लंबाई = &zOut[nOut-1] - bufpt;
        अवरोध;
      हाल etFLOAT:
      हाल etEXP:
      हाल etGENERIC: {
        FpDecode s;
        पूर्णांक iRound;
        पूर्णांक j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        अगर( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }अन्यथा{
          realvalue = बहु_तर्क(ap,द्विगुन);
        }
        अगर( precision<0 ) precision = 6;         /* Set default precision */
        अगर( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        अगर( xtype==etFLOAT ){
          iRound = -precision;
        }अन्यथा अगर( xtype==etGENERIC ){
          अगर( precision==0 ) precision = 1;
          iRound = precision;
        }अन्यथा{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        अगर( s.isSpecial ){
          अगर( s.isSpecial==2 ){
            अगर( flag_zeropad ){
              bufpt = "null";
              लंबाई = 4;
            }अन्यथा{
              bufpt = "NaN";
              लंबाई = 3;
            }
            अवरोध;
          }अन्यथा अगर( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }अन्यथा{
            स_नकल(buf, "-Inf", 5);
            bufpt = buf;
            अगर( s.sign=='-' ){
              /* no-op */
            }अन्यथा अगर( flag_prefix ){
              buf[0] = flag_prefix;
            }अन्यथा{
              bufpt++;
            }
            लंबाई = म_माप(bufpt);
            अवरोध;
          }
        }
        अगर( s.sign=='-' ){
          अगर( flag_alternateform
           && !flag_prefix
           && xtype==etFLOAT
           && s.iDP<=iRound
          ){
            /* Suppress the minus sign if all of the following are true:
            **   *  The value displayed is zero
            **   *  The '#' flag is used
            **   *  The '+' flag is not used, and
            **   *  The format is %f
            */
            prefix = 0;
          }अन्यथा{
            prefix = '-';
          }
        }अन्यथा{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        अगर( xtype==etGENERIC ){
          निश्चित( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          अगर( exp<-4 || exp>precision ){
            xtype = etEXP;
          }अन्यथा{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }अन्यथा{
          flag_rtz = flag_altform2;
        }
        अगर( xtype==etEXP ){
          e2 = 0;
        }अन्यथा{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        अगर( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        अगर( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          अगर( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            अगर( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              वापस;
            }
            zExtra = bufpt;
          }अन्यथा अगर( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = लंबाई = 0;
            अवरोध;
          }अन्यथा{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }अन्यथा{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        अगर( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        निश्चित( s.n>0 );
        अगर( e2<0 ){
          *(bufpt++) = '0';
        }अन्यथा अगर( cThousand ){
          क्रम(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            अगर( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }अन्यथा{
          j = e2+1;
          अगर( j>s.n ) j = s.n;
          स_नकल(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          अगर( e2>=0 ){
            स_रखो(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        अगर( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        अगर( e2<(-1) && precision>0 ){
          पूर्णांक nn = -1-e2;
          अगर( nn>precision ) nn = precision;
          स_रखो(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        अगर( precision>0 ){
          पूर्णांक nn = s.n - j;
          अगर( NEVER(nn>precision) ) nn = precision;
          अगर( nn>0 ){
            स_नकल(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          अगर( precision>0 && !flag_rtz ){
            स_रखो(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        अगर( flag_rtz && flag_dp ){
          जबतक( bufpt[-1]=='0' ) *(--bufpt) = 0;
          निश्चित( bufpt>zOut );
          अगर( bufpt[-1]=='.' ){
            अगर( flag_altform2 ){
              *(bufpt++) = '0';
            }अन्यथा{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        अगर( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          अगर( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }अन्यथा{
            *(bufpt++) = '+';
          }
          अगर( exp>=100 ){
            *(bufpt++) = (अक्षर)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (अक्षर)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (अक्षर)(exp%10+'0');             /* 1's digit */
        }

        लंबाई = bufpt - zOut;
        निश्चित( लंबाई <= szBufNeeded );
        अगर( लंबाई<width ){
          i64 nPad = width - लंबाई;
          अगर( flag_leftjustify ){
            स_रखो(bufpt, ' ', nPad);
          }अन्यथा अगर( !flag_zeropad ){
            स_हटाओ(zOut+nPad, zOut, लंबाई);
            स_रखो(zOut, ' ', nPad);
          }अन्यथा{
            पूर्णांक adj = prefix!=0;
            स_हटाओ(zOut+nPad+adj, zOut+adj, लंबाई-adj);
            स_रखो(zOut+adj, '0', nPad);
          }
          लंबाई = width;
        }

        अगर( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          निश्चित( pAccum->nChar + लंबाई < SMXV(pAccum->nChar) );
          pAccum->nChar += लंबाई;
          zOut[लंबाई] = 0;
          जारी;
        }अन्यथा{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          अवरोध;
        }
      }
      हाल et_आकार:
        अगर( !bArgList ){
          *(बहु_तर्क(ap,पूर्णांक*)) = pAccum->nChar;
        }
        लंबाई = width = 0;
        अवरोध;
      हाल etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        लंबाई = 1;
        अवरोध;
      हाल etCHARX:
        अगर( bArgList ){
          bufpt = getTextArg(pArgList);
          लंबाई = 1;
          अगर( bufpt ){
            buf[0] = c = *(bufpt++);
            अगर( (c&0xc0)==0xc0 ){
              जबतक( लंबाई<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[लंबाई++] = *(bufpt++);
              }
            }
          }अन्यथा{
            buf[0] = 0;
          }
        }अन्यथा{
          अचिन्हित पूर्णांक ch = बहु_तर्क(ap,अचिन्हित पूर्णांक);
          लंबाई = sqlite3AppendOneUtf8Character(buf, ch);
        }
        अगर( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          अगर( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, लंबाई);
          precision--;
          जबतक( precision > 1 ){
            i64 nCopyBytes;
            अगर( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = लंबाई*nPrior;
            अगर( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              अवरोध;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        जाओ adjust_width_for_utf8;
      हाल etSTRING:
      हाल etDYNSTRING:
        अगर( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }अन्यथा{
          bufpt = बहु_तर्क(ap,अक्षर*);
        }
        अगर( bufpt==0 ){
          bufpt = "";
        }अन्यथा अगर( xtype==etDYNSTRING ){
          अगर( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            निश्चित( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3_Db_Malloc_आकार(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (पूर्णांक)म_माप(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            लंबाई = 0;
            अवरोध;
          }
          zExtra = bufpt;
        }
        अगर( precision>=0 ){
          अगर( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            अचिन्हित अक्षर *z = (अचिन्हित अक्षर*)bufpt;
            जबतक( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            लंबाई = z - (अचिन्हित अक्षर*)bufpt;
          }अन्यथा{
            क्रम(लंबाई=0; लंबाई<precision && bufpt[लंबाई]; लंबाई++){}
          }
        }अन्यथा{
          लंबाई = म_माप(bufpt);
        }
      adjust_width_for_utf8:
        अगर( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = लंबाई - 1;
          जबतक( ii>=0 ) अगर( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        अवरोध;
      हाल etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      हाल etESCAPE_J: {         /* %J: Generate a JSON string literal */
        अक्षर *escarg;
        i64 i, j, px, iStart;
        अचिन्हित अक्षर ch;

        अगर( bArgList ){
          escarg = getTextArg(pArgList);
        }अन्यथा{
          escarg = बहु_तर्क(ap,अक्षर*);
        }
        iStart = sqlite3_str_लंबाई(pAccum);
        अगर( escarg==0 ){
          अगर( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }अन्यथा{
          अगर( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          अगर( px<0 ){
            px = 0x7fffffff;
          }अन्यथा अगर( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            क्रम(i=0; i<px && escarg[i]; i++){
              अगर( (escarg[i]&0xc0)==0x80 ) px++;
            }
            अगर( i==px ){
              जबतक( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          क्रम(i=j=0; i<px; i++){
            अगर( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              अगर( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              अगर( ch==0 ) अवरोध;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              अगर( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }अन्यथा अगर( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }अन्यथा{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          अगर( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          अगर( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        अगर( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_लंबाई(pAccum) - iStart;
          sqlite3_int64 len = n;
          अक्षर *zz;
          अगर( flag_altform2 && n>0 ){
            zz = sqlite3_str_मूल्य(pAccum);
            क्रम(i=iStart; zz[i]; i++){
              अगर( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          अगर( width>len ){
            sqlite3_int64 sp = width-len;
            निश्चित( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (पूर्णांक)sp, ' ');
            अगर( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_मूल्य(pAccum);
              zz += iStart;
              स_हटाओ(zz+sp, zz, n);
              स_रखो(zz, ' ', sp);
            }
          }
        }
        जारी;
      }
      हाल etESCAPE_q:          /* %q: Escape ' characters */
      हाल etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      हाल etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        पूर्णांक needQuote = 0;
        अक्षर ch;
        अक्षर *escarg;
        अक्षर q;

        अगर( bArgList ){
          escarg = getTextArg(pArgList);
        }अन्यथा{
          escarg = बहु_तर्क(ap,अक्षर*);
        }
        अगर( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }अन्यथा अगर( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        अगर( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }अन्यथा{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        क्रम(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          अगर( ch==q )  n++;
          अगर( flag_altform2 && (ch&0xc0)==0xc0 ){
            जबतक( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        अगर( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          क्रम(k=0; k<i; k++){
            अगर( escarg[k]=='\\' ){
              nBack++;
            }अन्यथा अगर( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          अगर( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            अगर( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }अन्यथा{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        अगर( n>etBUFSIZE ){
          bufpt = zExtra = printf_अस्थायी_Buf(pAccum, n);
          अगर( bufpt==0 ) वापस;
        }अन्यथा{
          bufpt = buf;
        }
        j = 0;
        अगर( needQuote ){
          अगर( needQuote==2 ){
            स_नकल(&bufpt[j], "unistr('", 8);
            j += 8;
          }अन्यथा{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        अगर( flag_alternateform ){
          क्रम(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            अगर( ch==q ){
              bufpt[j++] = ch;
            }अन्यथा अगर( ch=='\\' ){
              bufpt[j++] = '\\';
            }अन्यथा अगर( ((अचिन्हित अक्षर)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }अन्यथा{
          क्रम(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            अगर( ch==q ) bufpt[j++] = ch;
          }
        }
        अगर( needQuote ){
          bufpt[j++] = '\'';
          अगर( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        लंबाई = j;
        जाओ adjust_width_for_utf8;
      }
      हाल etTOKEN: {
        अगर( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) वापस;
        अगर( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          व्यंजक *p_व्यंजक = बहु_तर्क(ap,व्यंजक*);
          अगर( ALWAYS(p_व्यंजक) && ALWAYS(!व्यंजक_Has_Property(p_व्यंजक,EP_Int_मूल्य)) ){
            sqlite3_str_appendall(pAccum, (स्थिर अक्षर*)p_व्यंजक->u.zToken);
            sqlite3_Record_Error_Offset_Of_व्यंजक(pAccum->db, p_व्यंजक);
          }
        }अन्यथा{
          /* %T means a Token pointer */
          Token *pToken = बहु_तर्क(ap, Token*);
          निश्चित( bArgList==0 );
          अगर( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (स्थिर अक्षर*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        लंबाई = width = 0;
        अवरोध;
      }
      हाल etSRCITEM: {
        SrcItem *pItem;
        अगर( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) वापस;
        pItem = बहु_तर्क(ap, SrcItem*);
        निश्चित( bArgList==0 );
        अगर( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }अन्यथा अगर( pItem->zName ){
          अगर( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }अन्यथा अगर( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }अन्यथा अगर( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          निश्चित( pSel!=0 ); 
          अगर( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }अन्यथा अगर( pSel->selFlags & SF_Multi_मूल्य ){
            निश्चित( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }अन्यथा{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        लंबाई = width = 0;
        अवरोध;
      }
      शेष: {
        निश्चित( xtype==etINVALID );
        वापस;
      }
    }/* End switch over the format type */
    /*
    ** The text of the conversion is pointed to by "bufpt" and is
    ** "length" characters long.  The field width is "width".  Do
    ** the output.  Both length and width are in bytes, not characters,
    ** at this point.  If the "!" flag was present on string conversions
    ** indicating that width and precision should be expressed in characters,
    ** then the values have been translated prior to reaching this point.
    */
    width -= लंबाई;
    अगर( width>0 ){
      अगर( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, लंबाई);
      अगर( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }अन्यथा{
      sqlite3StrAppend64(pAccum, bufpt, लंबाई);
    }

    अगर( zExtra ){
      sqlite3DbFree(pAccum->db, zExtra);
      zExtra = 0;
    }
  }/* End for loop over the format string */
} /* End of function */


/*
** The z string points to the first character of a token that is
** associated with an error.  If db does not already have an error
** byte offset recorded, try to compute the error byte offset for
** z and set the error byte offset in db.
*/
व्योम sqlite3RecordErrorByteOffset(sqlite3 *db, स्थिर अक्षर *z){
  स्थिर Parse *pParse;
  स्थिर अक्षर *zText;
  स्थिर अक्षर *zEnd;
  निश्चित( z!=0 );
  अगर( NEVER(db==0) ) वापस;
  अगर( db->errByteOffset!=(-2) ) वापस;
  pParse = db->pParse;
  अगर( NEVER(pParse==0) ) वापस;
  zText =pParse->z_पुच्छ;
  अगर( NEVER(zText==0) ) वापस;
  zEnd = &zText[म_माप(zText)];
  अगर( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (पूर्णांक)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
व्योम sqlite3_Record_Error_Offset_Of_व्यंजक(sqlite3 *db, स्थिर व्यंजक *p_व्यंजक){
  जबतक( p_व्यंजक ){
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Reduced|EP_TokenOnly) ) वापस;
    अगर( व्यंजक_Has_Property(p_व्यंजक,EP_OuterON|EP_InnerON) || p_व्यंजक->w.iOfst<=0 ){
      p_व्यंजक = p_व्यंजक->p_बायाँ;
    }अन्यथा{
      अवरोध;
    }
  }
  अगर( p_व्यंजक==0 ) वापस;
  अगर( व्यंजक_Has_Property(p_व्यंजक, EP_FromDDL) ) वापस;
  db->errByteOffset = p_व्यंजक->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
पूर्णांक sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  अक्षर *zNew;
  निश्चित( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  अगर( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    वापस 0;
  }
  अगर( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    वापस p->nAlloc - p->nChar - 1;
  }अन्यथा{
    अक्षर *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    अगर( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    अगर( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      वापस 0;
    }अन्यथा{
      p->nAlloc = (पूर्णांक)szNew;
    }
    अगर( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }अन्यथा{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    अगर( zNew ){
      निश्चित( p->zText!=0 || p->nChar==0 );
      अगर( !isMalloced(p) && p->nChar>0 ) स_नकल(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3_Db_Malloc_आकार(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }अन्यथा{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      वापस 0;
    }
  }
  निश्चित( N>=0 && N<=0x7fffffff );
  वापस (पूर्णांक)N;
}

पूर्णांक sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  अगर( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  वापस p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
व्योम sqlite3_str_appendchar(sqlite3_str *p, पूर्णांक N, अक्षर c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  अगर( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    वापस;
  }
  जबतक( (N--)>0 ) p->zText[p->nChar++] = c;
}
अटल व्योम sqlite3StrAppendchar64(sqlite3_str *p, i64 N, अक्षर c){
  testcase( p->nChar + N > 0x7fffffff );
  अगर( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    वापस;
  }
  जबतक( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
अटल व्योम SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, स्थिर अक्षर *z, पूर्णांक N){
  N = sqlite3StrAccumEnlarge(p, N);
  निश्चित( z!=0 || N==0 );
  अगर( N>0 ){
    स_नकल(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
व्योम sqlite3_str_append(sqlite3_str *p, स्थिर अक्षर *z, पूर्णांक N){
  निश्चित( z!=0 || N==0 );
  निश्चित( p->zText!=0 || p->nChar==0 || p->accError );
  निश्चित( N>=0 );
  निश्चित( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  अगर( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }अन्यथा अगर( N ){
    निश्चित( p->zText );
    p->nChar += N;
    स_नकल(&p->zText[p->nChar-N], z, N);
  }
}
अटल व्योम sqlite3StrAppend64(sqlite3_str *p, स्थिर अक्षर *z, i64 N){
  निश्चित( z!=0 || N==0 );
  निश्चित( p->zText!=0 || p->nChar==0 || p->accError );
  निश्चित( N>=0 );
  निश्चित( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  अगर( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }अन्यथा अगर( N ){
    निश्चित( p->zText );
    p->nChar += N;
    स_नकल(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
व्योम sqlite3_str_appendall(sqlite3_str *p, स्थिर अक्षर *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
अटल SQLITE_NOINLINE अक्षर *strAccumFinishRealloc(StrAccum *p){
  अक्षर *zText;
  निश्चित( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  अगर( zText ){
    स_नकल(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }अन्यथा{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  वापस zText;
}
अक्षर *sqlite3StrAccumFinish(StrAccum *p){
  अगर( p->zText ){
    p->zText[p->nChar] = 0;
    अगर( p->mxAlloc>0 && !isMalloced(p) ){
      वापस strAccumFinishRealloc(p);
    }
  }
  वापस p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
अक्षर *sqlite3_str_finish(sqlite3_str *p){
  अक्षर *z;
  अगर( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }अन्यथा{
    z = 0;
  }
  वापस z;
}

/* Return any error code associated with p */
पूर्णांक sqlite3_str_errcode(sqlite3_str *p){
  वापस p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
पूर्णांक sqlite3_str_लंबाई(sqlite3_str *p){
  वापस p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
व्योम sqlite3_str_truncate(sqlite3_str *p, पूर्णांक N){
  अगर( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
अक्षर *sqlite3_str_मूल्य(sqlite3_str *p){
  अगर( p==0 || p->nChar==0 ) वापस 0;
  p->zText[p->nChar] = 0;
  वापस p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
व्योम sqlite3_str_reset(StrAccum *p){
  अगर( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }अन्यथा अगर( p==(sqlite3_str*)&sqlite3OomStr ){
    वापस;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
व्योम sqlite3_str_free(sqlite3_str *p){
  अगर( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    sqlite3_str_reset(p);
    sqlite3_free(p);
  }
}

/*
** Initialize a string accumulator.
**
** p:     The accumulator to be initialized.
** db:    Pointer to a database connection.  May be NULL.  Lookaside
**        memory is used if not NULL. db->mallocFailed is set appropriately
**        when not NULL.
** zBase: An initial buffer.  May be NULL in which case the initial buffer
**        is malloced.
** n:     Size of zBase in bytes.  If total space requirements never exceed
**        n then no memory allocations ever occur.
** mx:    Maximum number of bytes to accumulate.  If mx==0 then no memory
**        allocations will ever occur.
*/
व्योम sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, अक्षर *zBase, पूर्णांक n, पूर्णांक mx){
  p->zText = zBase;
  p->db = db;
  p->nAlloc = n;
  p->mxAlloc = mx;
  p->nChar = 0;
  p->accError = 0;
  p->printfFlags = 0;
}

/* Allocate and initialize a new dynamic string object */
sqlite3_str *sqlite3_str_new(sqlite3 *db){
  sqlite3_str *p = sqlite3_malloc64(माप(*p));
  अगर( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_लंबाई] : SQLITE_MAX_लंबाई);
  }अन्यथा{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  वापस p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
अक्षर *sqlite3VMPrintf(sqlite3 *db, स्थिर अक्षर *zFormat, बहु_सूची ap){
  अक्षर *z;
  अक्षर zBase[SQLITE_PRINT_BUF_आकार];
  StrAccum acc;
  निश्चित( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, माप(zBase),
                      db->aLimit[SQLITE_LIMIT_लंबाई]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  अगर( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  वापस z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
अक्षर *sqlite3MPrintf(sqlite3 *db, स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  अक्षर *z;
  बहु_शुरू(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  बहु_पूर्ण(ap);
  वापस z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
अक्षर *sqlite3_vmprintf(स्थिर अक्षर *zFormat, बहु_सूची ap){
  अक्षर *z;
  अक्षर zBase[SQLITE_PRINT_BUF_आकार];
  StrAccum acc;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR  
  अगर( zFormat==0 ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  sqlite3StrAccumInit(&acc, 0, zBase, माप(zBase), SQLITE_MAX_लंबाई);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  वापस z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
अक्षर *sqlite3_mprintf(स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  अक्षर *z;
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  बहु_शुरू(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  बहु_पूर्ण(ap);
  वापस z;
}

/*
** sqlite3_snprintf() works like snprintf() except that it ignores the
** current locale settings.  This is important for SQLite because we
** are not able to use a "," as the decimal point in place of "." as
** specified by some locales.
**
** Oops:  The first two arguments of sqlite3_snprintf() are backwards
** from the snprintf() standard.  Unfortunately, it is too late to change
** this without breaking compatibility, so we just have to live with the
** mistake.
**
** sqlite3_vsnprintf() is the varargs version.
*/
अक्षर *sqlite3_vsnprintf(पूर्णांक n, अक्षर *zBuf, स्थिर अक्षर *zFormat, बहु_सूची ap){
  StrAccum acc;
  अगर( n<=0 ) वापस zBuf;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( zBuf==0 || zFormat==0 ) {
    (व्योम)SQLITE_MISUSE_BKPT;
    अगर( zBuf ) zBuf[0] = 0;
    वापस zBuf;
  }
#पूर्ण_अगर
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  वापस zBuf;
}
अक्षर *sqlite3_snprintf(पूर्णांक n, अक्षर *zBuf, स्थिर अक्षर *zFormat, ...){
  StrAccum acc;
  बहु_सूची ap;
  अगर( n<=0 ) वापस zBuf;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( zBuf==0 || zFormat==0 ) {
    (व्योम)SQLITE_MISUSE_BKPT;
    अगर( zBuf ) zBuf[0] = 0;
    वापस zBuf;
  }
#पूर्ण_अगर
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  बहु_शुरू(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  बहु_पूर्ण(ap);
  zBuf[acc.nChar] = 0;
  वापस zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#अगर defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#या_अगर SQLITE_PRINT_BUF_आकार*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#अन्यथा
# define SQLITE_MAX_LOG_MESSAGE (SQLITE_PRINT_BUF_आकार*10)
#पूर्ण_अगर

/*
** This is the routine that actually formats the sqlite3_log() message.
** We house it in a separate routine from sqlite3_log() to avoid using
** stack space on small-stack systems when logging is disabled.
**
** sqlite3_log() must render into a static buffer.  It cannot dynamically
** allocate memory because it might be called while the memory allocator
** mutex is held.
**
** sqlite3_str_vappendf() might ask for *temporary* memory allocations for
** certain format characters (%q) or for very large precisions or widths.
** Care must be taken that any sqlite3_log() calls that occur while the
** memory mutex is held do not use these mechanisms.
*/
अटल व्योम render_Log_संदेश(पूर्णांक iErrCode, स्थिर अक्षर *zFormat, बहु_सूची ap){
  StrAccum acc;                          /* String accumulator */
  अक्षर z_संदेश[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_संदेश, माप(z_संदेश), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
व्योम sqlite3_log(पूर्णांक iErrCode, स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;                             /* Vararg list */
  अगर( sqlite3GlobalConfig.xLog ){
    बहु_शुरू(ap, zFormat);
    render_Log_संदेश(iErrCode, zFormat, ap);
    बहु_पूर्ण(ap);
  }
}

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
व्योम sqlite3DebugPrintf(स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  StrAccum acc;
  अक्षर zBuf[SQLITE_PRINT_BUF_आकार*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, माप(zBuf), 0);
  बहु_शुरू(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  बहु_पूर्ण(ap);
  sqlite3StrAccumFinish(&acc);
#अगर_घोषित SQLITE_OS_TRACE_PROC
  {
    बाह्य व्योम SQLITE_OS_TRACE_PROC(स्थिर अक्षर *zBuf, पूर्णांक nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, माप(zBuf));
  }
#अन्यथा
  ख_लिखो(मानक_निकास,"%s", zBuf);
  ख_साफ(मानक_निकास);
#पूर्ण_अगर
}
#पूर्ण_अगर


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
व्योम sqlite3_str_appendf(StrAccum *p, स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  बहु_शुरू(ap,zFormat);
  sqlite3_str_vappendf(p, zFormat, ap);
  बहु_पूर्ण(ap);
}


/*****************************************************************************
** Reference counted string/blob storage
*****************************************************************************/

/*
** Increase the reference count of the string by one.
**
** The input parameter is returned.
*/
अक्षर *sqlite3RCStrRef(अक्षर *z){
  RCStr *p = (RCStr*)z;
  निश्चित( p!=0 );
  p--;
  p->nRCRef++;
  वापस z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
व्योम sqlite3RCStrUnref(व्योम *z){
  RCStr *p = (RCStr*)z;
  निश्चित( p!=0 );
  p--;
  निश्चित( p->nRCRef>0 );
  अगर( p->nRCRef>=2 ){
    p->nRCRef--;
  }अन्यथा{
    sqlite3_free(p);
  }
}

/*
** Create a new string that is capable of holding N bytes of text, not counting
** the zero byte at the end.  The string is uninitialized.
**
** The reference count is initially 1.  Call sqlite3RCStrUnref() to free the
** newly allocated string.
**
** This routine returns 0 on an OOM.
*/
अक्षर *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + माप(*p) + 1 );
  अगर( p==0 ) वापस 0;
  p->nRCRef = 1;
  वापस (अक्षर*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
अक्षर *sqlite3RCStrResize(अक्षर *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  निश्चित( p!=0 );
  p--;
  निश्चित( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+माप(RCStr)+1);
  अगर( pNew==0 ){
    sqlite3_free(p);
    वापस 0;
  }अन्यथा{
    वापस (अक्षर*)&pNew[1];
  }
}
