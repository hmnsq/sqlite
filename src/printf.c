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
#अन्तर्भाव "sqliteInt.h"

/*
** Conversion types fall into various categories as defined by the
** following enumeration.
*/
#define etRADIX       0 /* non-decimal integer types.  %x %o */
#define etFLOAT       1 /* Floating point.  %f */
#define etEXP         2 /* Exponentional notation. %e and %E */
#define etGENERIC     3 /* Floating or exponential, depending on exponent. %g */
#define etSIZE        4 /* Return number of characters processed so far. %n */
#define etSTRING      5 /* Strings. %s */
#define etDYNSTRING   6 /* Dynamically allocated strings. %z */
#define etPERCENT     7 /* Percent symbol. %% */
#define etCHARX       8 /* Characters. %c */
/* The rest are extensions, not normally found in printf() */
#define etESCAPE_q    9  /* Strings with '\'' doubled.  %q */
#define etESCAPE_Q    10 /* Strings with '\'' doubled and enclosed in '',
                            NULL pointers replaced by SQL NULL.  %Q */
#define etTOKEN       11 /* a pointer to a Token structure */
#define etSRCITEM     12 /* a pointer to a SrcItem */
#define etPOINTER     13 /* The %p conversion */
#define etESCAPE_w    14 /* %w -> Strings with '\"' doubled */
#define etORDINAL     15 /* %r -> 1st, 2nd, 3rd, 4th, etc.  English only */
#define etDECIMAL     16 /* %d or %u, but not %x, %o */
#define etESCAPE_j    17 /* %j -> JSON string literal w/o "..." */
#define etESCAPE_J    18 /* %J -> JSON string literal with "..." */

#define etINVALID     19 /* Any unrecognized conversion type */


/*
** An "etByte" is an 8-bit unsigned value.
*/
प्रकारव्याख्या अचिन्हित अक्षर etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
प्रकारव्याख्या संरचना et_info {   /* Information about each format field */
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
#define FLAG_SIGNED    1     /* True if the value to convert is signed */
#define FLAG_STRING    4     /* Allow infinite precision */

/*
** The table is searched by hash.  In the case of %C where C is the character
** and that character has ASCII value j, then the hash is j%25.
**
** The order of the entries in fmtinfo[] and the hash chain was entered
** manually, but based on the output of the following TCL script:
*/
#जर 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
माला_दो "count: $mx"

set mx 25
माला_दो "*********** mx=$mx ************"
साठी {set r 0} {$r<$mx} {incr r} {
  माला_दो -nonewline [format %2d: $r]
  foreach c [array names n] {
    जर {($n($c))%$mx==$r} {माला_दो -nonewline " $c"}
  }
  माला_दो ""
}
#endif /***** End of script ********/

स्थिर स्थिरांक अक्षर aDigits[] = "0123456789ABCDEF0123456789abcdef";
स्थिर स्थिरांक अक्षर aHex[]    = "0123456789abcdef";
स्थिर स्थिरांक अक्षर aPrefix[] = "-x0\000X0";
स्थिर स्थिरांक et_info fmtinfo[25] = {
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
  /* 10 */  {  'n',  0, 0, etSIZE,       0,  0,  0 },
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
शून्य sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  प्रतिज्ञा( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  जर( p->mxAlloc ) sqlite3_str_reset(p);
  जर( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
स्थिर sqlite3_int64 getIntArg(PrintfArguments *p){
  जर( p->nArg<=p->nUsed ) परत 0;
  परत sqlite3_मूल्य_int64(p->apArg[p->nUsed++]);
}
स्थिर द्विगुण getDoubleArg(PrintfArguments *p){
  जर( p->nArg<=p->nUsed ) परत 0.0;
  परत sqlite3_मूल्य_double(p->apArg[p->nUsed++]);
}
स्थिर अक्षर *getTextArg(PrintfArguments *p){
  जर( p->nArg<=p->nUsed ) परत 0;
  परत (अक्षर*)sqlite3_मूल्य_text(p->apArg[p->nUsed++]);
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
स्थिर अक्षर *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  अक्षर *z;
  जर( pAccum->accError ) परत 0;
  जर( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    परत 0;
  }
  z = sqlite3_malloc(n);
  जर( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  परत z;
}

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be something smaller, if desired.
*/
#ifndef SQLITE_PRINT_BUF_SIZE
# define SQLITE_PRINT_BUF_SIZE 70
#endif
#define etBUFSIZE SQLITE_PRINT_BUF_SIZE  /* Size of the output buffer */

/*
** Hard limit on the precision of floating-point conversions to
** the minimum of SQLITE_PRINTF_PRECISION_LIMIT and 100 million.
*/
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
# जर SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# अन्यथा
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#अन्यथा
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
स्थिर शून्य sqlite3StrAppend64(sqlite3_str *p, स्थिरांक अक्षर *z, i64 N);
स्थिर शून्य sqlite3StrAppendchar64(sqlite3_str *p, i64 N, अक्षर c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
शून्य sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  स्थिरांक अक्षर *fmt,           /* Format string */
  बहु_सूची ap                 /* arguments */
){
  पूर्णांक c;                     /* Next character in the format string */
  अक्षर *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
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
  द्विगुण realvalue;          /* Value for real types */
  स्थिरांक et_info *infop;      /* Pointer to the appropriate info structure */
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
  प्रतिज्ञा( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  जर( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = बहु_तर्क(ap, PrintfArguments*);
    bArgList = 1;
  }अन्यथा{
    bArgList = 0;
  }
  साठी(; (c=(*fmt))!=0; ++fmt){
    जर( c!='%' ){
      bufpt = (अक्षर *)fmt;
#जर HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#अन्यथा
      fmt = म_अक्षर(fmt, '%');
      जर( fmt==0 ){
        fmt = bufpt + म_माप(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      जर( *fmt==0 ) थांबा;
    }
    जर( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      थांबा;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    करा{
      निवड( c ){
        स्थिती '-':   flag_leftjustify = 1;     थांबा;
        स्थिती '+':   flag_prefix = '+';        थांबा;
        स्थिती ' ':   flag_prefix = ' ';        थांबा;
        स्थिती '#':   flag_alternateform = 1;   थांबा;
        स्थिती '!':   flag_altform2 = 1;        थांबा;
        स्थिती '0':   flag_zeropad = 1;         थांबा;
        स्थिती ',':   cThousand = ',';          थांबा;
        पूर्वनिर्धारित:    done = 1;                 थांबा;
        स्थिती 'l': {
          flag_long = 1;
          c = *++fmt;
          जर( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          थांबा;
        }
        स्थिती '1': स्थिती '2': स्थिती '3': स्थिती '4': स्थिती '5':
        स्थिती '6': स्थिती '7': स्थिती '8': स्थिती '9': {
          अचिन्हित wx = c - '0';
          पर्यंत( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          जर( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          जर( c!='.' && c!='l' ){
            done = 1;
          }अन्यथा{
            fmt--;
          }
          थांबा;
        }
        स्थिती '*': {
          जर( bArgList ){
            width = (पूर्णांक)getIntArg(pArgList);
          }अन्यथा{
            width = बहु_तर्क(ap,पूर्णांक);
          }
          जर( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          जर( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          जर( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          थांबा;
        }
        स्थिती '.': {
          c = *++fmt;
          जर( c=='*' ){
            जर( bArgList ){
              precision = (पूर्णांक)getIntArg(pArgList);
            }अन्यथा{
              precision = बहु_तर्क(ap,पूर्णांक);
            }
            जर( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }अन्यथा{
            अचिन्हित px = 0;
            पर्यंत( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          जर( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          जर( c=='l' ){
            --fmt;
          }अन्यथा{
            done = 1;
          }
          थांबा;
        }
      }
    }पर्यंत( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    साठी(idx=0; idx<ArraySize(fmtinfo); idx++){
      जर( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        थांबा;
      }
    }
#अन्यथा
    /* Fast hash-table lookup */
    प्रतिज्ञा( ArraySize(fmtinfo)==25 );
    idx = ((अचिन्हित)c) % 25;
    जर( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }अन्यथा{
      infop = &fmtinfo[0];
      xtype = etINVALID;
    }
#endif

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
    प्रतिज्ञा( width>=0 );
    प्रतिज्ञा( precision>=(-1) );
    निवड( xtype ){
      स्थिती etPOINTER:
        flag_long = आकार(अक्षर*)==आकार(i64) ? 2 :
                     आकार(अक्षर*)==आकार(दीर्घ पूर्णांक) ? 1 : 0;
        /* no break */ deliberate_fall_through
      स्थिती etORDINAL:
      स्थिती etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      स्थिती etDECIMAL:
        जर( infop->flags & FLAG_SIGNED ){
          i64 v;
          जर( bArgList ){
            v = getIntArg(pArgList);
          }अन्यथा जर( flag_long ){
            जर( flag_long==2 ){
              v = बहु_तर्क(ap,i64) ;
            }अन्यथा{
              v = बहु_तर्क(ap,दीर्घ पूर्णांक);
            }
          }अन्यथा{
            v = बहु_तर्क(ap,पूर्णांक);
          }
          जर( v<0 ){
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
          जर( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }अन्यथा जर( flag_long ){
            जर( flag_long==2 ){
              longvalue = बहु_तर्क(ap,u64);
            }अन्यथा{
              longvalue = बहु_तर्क(ap,अचिन्हित दीर्घ पूर्णांक);
            }
          }अन्यथा{
            longvalue = बहु_तर्क(ap,अचिन्हित पूर्णांक);
          }
          prefix = 0;
        }

#जर WHERETRACE_ENABLED
        जर( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#जर TREETRACE_ENABLED
        जर( xtype==etPOINTER && sqlite3_वृक्ष_Trace & 0x100000 ) longvalue = 0;
#endif

        जर( longvalue==0 ) flag_alternateform = 0;
        जर( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        जर( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }अन्यथा{
          u64 n;
          n = (u64)precision + 10;
          जर( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          जर( zOut==0 ) परत;
          nOut = (पूर्णांक)n;
        }
        bufpt = &zOut[nOut-1];
        जर( xtype==etORDINAL ){
          स्थिर स्थिरांक अक्षर zOrd[] = "thstndrd";
          पूर्णांक x = (पूर्णांक)(longvalue % 10);
          जर( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          स्थिरांक अक्षर *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          करा{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }पर्यंत( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        जर( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          स_रखो(bufpt,'0',nn);
          length = precision;
        }
        जर( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          पूर्णांक ii;
          bufpt -= nn;
          साठी(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            जर( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        जर( prefix ) *(--bufpt) = prefix;               /* Add sign */
        जर( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          स्थिरांक अक्षर *pre;
          अक्षर x;
          pre = &aPrefix[infop->prefix];
          साठी(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        थांबा;
      स्थिती etFLOAT:
      स्थिती etEXP:
      स्थिती etGENERIC: {
        FpDecode s;
        पूर्णांक iRound;
        पूर्णांक j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        जर( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }अन्यथा{
          realvalue = बहु_तर्क(ap,द्विगुण);
        }
        जर( precision<0 ) precision = 6;         /* Set default precision */
        जर( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        जर( xtype==etFLOAT ){
          iRound = -precision;
        }अन्यथा जर( xtype==etGENERIC ){
          जर( precision==0 ) precision = 1;
          iRound = precision;
        }अन्यथा{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        जर( s.isSpecial ){
          जर( s.isSpecial==2 ){
            जर( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }अन्यथा{
              bufpt = "NaN";
              length = 3;
            }
            थांबा;
          }अन्यथा जर( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }अन्यथा{
            स_नकल(buf, "-Inf", 5);
            bufpt = buf;
            जर( s.sign=='-' ){
              /* no-op */
            }अन्यथा जर( flag_prefix ){
              buf[0] = flag_prefix;
            }अन्यथा{
              bufpt++;
            }
            length = म_माप(bufpt);
            थांबा;
          }
        }
        जर( s.sign=='-' ){
          जर( flag_alternateform
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
        जर( xtype==etGENERIC ){
          प्रतिज्ञा( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          जर( exp<-4 || exp>precision ){
            xtype = etEXP;
          }अन्यथा{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }अन्यथा{
          flag_rtz = flag_altform2;
        }
        जर( xtype==etEXP ){
          e2 = 0;
        }अन्यथा{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        जर( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        जर( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          जर( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            जर( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              परत;
            }
            zExtra = bufpt;
          }अन्यथा जर( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            थांबा;
          }अन्यथा{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }अन्यथा{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        जर( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        प्रतिज्ञा( s.n>0 );
        जर( e2<0 ){
          *(bufpt++) = '0';
        }अन्यथा जर( cThousand ){
          साठी(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            जर( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }अन्यथा{
          j = e2+1;
          जर( j>s.n ) j = s.n;
          स_नकल(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          जर( e2>=0 ){
            स_रखो(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        जर( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        जर( e2<(-1) && precision>0 ){
          पूर्णांक nn = -1-e2;
          जर( nn>precision ) nn = precision;
          स_रखो(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        जर( precision>0 ){
          पूर्णांक nn = s.n - j;
          जर( NEVER(nn>precision) ) nn = precision;
          जर( nn>0 ){
            स_नकल(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          जर( precision>0 && !flag_rtz ){
            स_रखो(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        जर( flag_rtz && flag_dp ){
          पर्यंत( bufpt[-1]=='0' ) *(--bufpt) = 0;
          प्रतिज्ञा( bufpt>zOut );
          जर( bufpt[-1]=='.' ){
            जर( flag_altform2 ){
              *(bufpt++) = '0';
            }अन्यथा{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        जर( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          जर( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }अन्यथा{
            *(bufpt++) = '+';
          }
          जर( exp>=100 ){
            *(bufpt++) = (अक्षर)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (अक्षर)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (अक्षर)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        प्रतिज्ञा( length <= szBufNeeded );
        जर( length<width ){
          i64 nPad = width - length;
          जर( flag_leftjustify ){
            स_रखो(bufpt, ' ', nPad);
          }अन्यथा जर( !flag_zeropad ){
            स_हटाओ(zOut+nPad, zOut, length);
            स_रखो(zOut, ' ', nPad);
          }अन्यथा{
            पूर्णांक adj = prefix!=0;
            स_हटाओ(zOut+nPad+adj, zOut+adj, length-adj);
            स_रखो(zOut+adj, '0', nPad);
          }
          length = width;
        }

        जर( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          प्रतिज्ञा( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          सुरूठेवा;
        }अन्यथा{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          थांबा;
        }
      }
      स्थिती etSIZE:
        जर( !bArgList ){
          *(बहु_तर्क(ap,पूर्णांक*)) = pAccum->nChar;
        }
        length = width = 0;
        थांबा;
      स्थिती etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        थांबा;
      स्थिती etCHARX:
        जर( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          जर( bufpt ){
            buf[0] = c = *(bufpt++);
            जर( (c&0xc0)==0xc0 ){
              पर्यंत( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }अन्यथा{
            buf[0] = 0;
          }
        }अन्यथा{
          अचिन्हित पूर्णांक ch = बहु_तर्क(ap,अचिन्हित पूर्णांक);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        जर( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          जर( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          पर्यंत( precision > 1 ){
            i64 nCopyBytes;
            जर( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            जर( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              थांबा;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        जा adjust_width_for_utf8;
      स्थिती etSTRING:
      स्थिती etDYNSTRING:
        जर( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }अन्यथा{
          bufpt = बहु_तर्क(ap,अक्षर*);
        }
        जर( bufpt==0 ){
          bufpt = "";
        }अन्यथा जर( xtype==etDYNSTRING ){
          जर( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            प्रतिज्ञा( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (पूर्णांक)म_माप(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            थांबा;
          }
          zExtra = bufpt;
        }
        जर( precision>=0 ){
          जर( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            अचिन्हित अक्षर *z = (अचिन्हित अक्षर*)bufpt;
            पर्यंत( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (अचिन्हित अक्षर*)bufpt;
          }अन्यथा{
            साठी(length=0; length<precision && bufpt[length]; length++){}
          }
        }अन्यथा{
          length = म_माप(bufpt);
        }
      adjust_width_for_utf8:
        जर( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          पर्यंत( ii>=0 ) जर( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        थांबा;
      स्थिती etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      स्थिती etESCAPE_J: {         /* %J: Generate a JSON string literal */
        अक्षर *escarg;
        i64 i, j, px, iStart;
        अचिन्हित अक्षर ch;

        जर( bArgList ){
          escarg = getTextArg(pArgList);
        }अन्यथा{
          escarg = बहु_तर्क(ap,अक्षर*);
        }
        iStart = sqlite3_str_length(pAccum);
        जर( escarg==0 ){
          जर( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }अन्यथा{
          जर( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          जर( px<0 ){
            px = 0x7fffffff;
          }अन्यथा जर( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            साठी(i=0; i<px && escarg[i]; i++){
              जर( (escarg[i]&0xc0)==0x80 ) px++;
            }
            जर( i==px ){
              पर्यंत( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          साठी(i=j=0; i<px; i++){
            जर( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              जर( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              जर( ch==0 ) थांबा;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              जर( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }अन्यथा जर( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }अन्यथा{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          जर( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          जर( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        जर( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          अक्षर *zz;
          जर( flag_altform2 && n>0 ){
            zz = sqlite3_str_मूल्य(pAccum);
            साठी(i=iStart; zz[i]; i++){
              जर( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          जर( width>len ){
            sqlite3_int64 sp = width-len;
            प्रतिज्ञा( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (पूर्णांक)sp, ' ');
            जर( !flag_leftjustify
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
        सुरूठेवा;
      }
      स्थिती etESCAPE_q:          /* %q: Escape ' characters */
      स्थिती etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      स्थिती etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        पूर्णांक needQuote = 0;
        अक्षर ch;
        अक्षर *escarg;
        अक्षर q;

        जर( bArgList ){
          escarg = getTextArg(pArgList);
        }अन्यथा{
          escarg = बहु_तर्क(ap,अक्षर*);
        }
        जर( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }अन्यथा जर( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        जर( xtype==etESCAPE_w ){
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
        साठी(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          जर( ch==q )  n++;
          जर( flag_altform2 && (ch&0xc0)==0xc0 ){
            पर्यंत( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        जर( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          साठी(k=0; k<i; k++){
            जर( escarg[k]=='\\' ){
              nBack++;
            }अन्यथा जर( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          जर( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            जर( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }अन्यथा{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        जर( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          जर( bufpt==0 ) परत;
        }अन्यथा{
          bufpt = buf;
        }
        j = 0;
        जर( needQuote ){
          जर( needQuote==2 ){
            स_नकल(&bufpt[j], "unistr('", 8);
            j += 8;
          }अन्यथा{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        जर( flag_alternateform ){
          साठी(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            जर( ch==q ){
              bufpt[j++] = ch;
            }अन्यथा जर( ch=='\\' ){
              bufpt[j++] = '\\';
            }अन्यथा जर( ((अचिन्हित अक्षर)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }अन्यथा{
          साठी(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            जर( ch==q ) bufpt[j++] = ch;
          }
        }
        जर( needQuote ){
          bufpt[j++] = '\'';
          जर( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        जा adjust_width_for_utf8;
      }
      स्थिती etTOKEN: {
        जर( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) परत;
        जर( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          अभिव्यक्ती *p_अभिव्यक्ती = बहु_तर्क(ap,अभिव्यक्ती*);
          जर( ALWAYS(p_अभिव्यक्ती) && ALWAYS(!अभिव्यक्ती_Has_Property(p_अभिव्यक्ती,EP_Int_मूल्य)) ){
            sqlite3_str_appendall(pAccum, (स्थिरांक अक्षर*)p_अभिव्यक्ती->u.zToken);
            sqlite3_Record_Error_Offset_Of_अभिव्यक्ती(pAccum->db, p_अभिव्यक्ती);
          }
        }अन्यथा{
          /* %T means a Token pointer */
          Token *pToken = बहु_तर्क(ap, Token*);
          प्रतिज्ञा( bArgList==0 );
          जर( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (स्थिरांक अक्षर*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        थांबा;
      }
      स्थिती etSRCITEM: {
        SrcItem *pItem;
        जर( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) परत;
        pItem = बहु_तर्क(ap, SrcItem*);
        प्रतिज्ञा( bArgList==0 );
        जर( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }अन्यथा जर( pItem->zName ){
          जर( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }अन्यथा जर( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }अन्यथा जर( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          प्रतिज्ञा( pSel!=0 ); 
          जर( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }अन्यथा जर( pSel->selFlags & SF_Multi_मूल्य ){
            प्रतिज्ञा( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }अन्यथा{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        थांबा;
      }
      पूर्वनिर्धारित: {
        प्रतिज्ञा( xtype==etINVALID );
        परत;
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
    width -= length;
    जर( width>0 ){
      जर( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      जर( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }अन्यथा{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    जर( zExtra ){
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
शून्य sqlite3RecordErrorByteOffset(sqlite3 *db, स्थिरांक अक्षर *z){
  स्थिरांक Parse *pParse;
  स्थिरांक अक्षर *zText;
  स्थिरांक अक्षर *zEnd;
  प्रतिज्ञा( z!=0 );
  जर( NEVER(db==0) ) परत;
  जर( db->errByteOffset!=(-2) ) परत;
  pParse = db->pParse;
  जर( NEVER(pParse==0) ) परत;
  zText =pParse->zTail;
  जर( NEVER(zText==0) ) परत;
  zEnd = &zText[म_माप(zText)];
  जर( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (पूर्णांक)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
शून्य sqlite3_Record_Error_Offset_Of_अभिव्यक्ती(sqlite3 *db, स्थिरांक अभिव्यक्ती *p_अभिव्यक्ती){
  पर्यंत( p_अभिव्यक्ती ){
    जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Reduced|EP_TokenOnly) ) परत;
    जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती,EP_OuterON|EP_InnerON) || p_अभिव्यक्ती->w.iOfst<=0 ){
      p_अभिव्यक्ती = p_अभिव्यक्ती->p_डावा;
    }अन्यथा{
      थांबा;
    }
  }
  जर( p_अभिव्यक्ती==0 ) परत;
  जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_FromDDL) ) परत;
  db->errByteOffset = p_अभिव्यक्ती->w.iOfst;
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
  प्रतिज्ञा( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  जर( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    परत 0;
  }
  जर( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    परत p->nAlloc - p->nChar - 1;
  }अन्यथा{
    अक्षर *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    जर( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    जर( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      परत 0;
    }अन्यथा{
      p->nAlloc = (पूर्णांक)szNew;
    }
    जर( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }अन्यथा{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    जर( zNew ){
      प्रतिज्ञा( p->zText!=0 || p->nChar==0 );
      जर( !isMalloced(p) && p->nChar>0 ) स_नकल(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }अन्यथा{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      परत 0;
    }
  }
  प्रतिज्ञा( N>=0 && N<=0x7fffffff );
  परत (पूर्णांक)N;
}

पूर्णांक sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  जर( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  परत p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
शून्य sqlite3_str_appendchar(sqlite3_str *p, पूर्णांक N, अक्षर c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  जर( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    परत;
  }
  पर्यंत( (N--)>0 ) p->zText[p->nChar++] = c;
}
स्थिर शून्य sqlite3StrAppendchar64(sqlite3_str *p, i64 N, अक्षर c){
  testcase( p->nChar + N > 0x7fffffff );
  जर( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    परत;
  }
  पर्यंत( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
स्थिर शून्य SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, स्थिरांक अक्षर *z, पूर्णांक N){
  N = sqlite3StrAccumEnlarge(p, N);
  प्रतिज्ञा( z!=0 || N==0 );
  जर( N>0 ){
    स_नकल(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
शून्य sqlite3_str_append(sqlite3_str *p, स्थिरांक अक्षर *z, पूर्णांक N){
  प्रतिज्ञा( z!=0 || N==0 );
  प्रतिज्ञा( p->zText!=0 || p->nChar==0 || p->accError );
  प्रतिज्ञा( N>=0 );
  प्रतिज्ञा( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  जर( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }अन्यथा जर( N ){
    प्रतिज्ञा( p->zText );
    p->nChar += N;
    स_नकल(&p->zText[p->nChar-N], z, N);
  }
}
स्थिर शून्य sqlite3StrAppend64(sqlite3_str *p, स्थिरांक अक्षर *z, i64 N){
  प्रतिज्ञा( z!=0 || N==0 );
  प्रतिज्ञा( p->zText!=0 || p->nChar==0 || p->accError );
  प्रतिज्ञा( N>=0 );
  प्रतिज्ञा( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  जर( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }अन्यथा जर( N ){
    प्रतिज्ञा( p->zText );
    p->nChar += N;
    स_नकल(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
शून्य sqlite3_str_appendall(sqlite3_str *p, स्थिरांक अक्षर *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
स्थिर SQLITE_NOINLINE अक्षर *strAccumFinishRealloc(StrAccum *p){
  अक्षर *zText;
  प्रतिज्ञा( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  जर( zText ){
    स_नकल(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }अन्यथा{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  परत zText;
}
अक्षर *sqlite3StrAccumFinish(StrAccum *p){
  जर( p->zText ){
    p->zText[p->nChar] = 0;
    जर( p->mxAlloc>0 && !isMalloced(p) ){
      परत strAccumFinishRealloc(p);
    }
  }
  परत p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
अक्षर *sqlite3_str_finish(sqlite3_str *p){
  अक्षर *z;
  जर( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }अन्यथा{
    z = 0;
  }
  परत z;
}

/* Return any error code associated with p */
पूर्णांक sqlite3_str_errcode(sqlite3_str *p){
  परत p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
पूर्णांक sqlite3_str_length(sqlite3_str *p){
  परत p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
शून्य sqlite3_str_truncate(sqlite3_str *p, पूर्णांक N){
  जर( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
अक्षर *sqlite3_str_मूल्य(sqlite3_str *p){
  जर( p==0 || p->nChar==0 ) परत 0;
  p->zText[p->nChar] = 0;
  परत p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
शून्य sqlite3_str_reset(StrAccum *p){
  जर( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }अन्यथा जर( p==(sqlite3_str*)&sqlite3OomStr ){
    परत;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
शून्य sqlite3_str_free(sqlite3_str *p){
  जर( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
शून्य sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, अक्षर *zBase, पूर्णांक n, पूर्णांक mx){
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
  sqlite3_str *p = sqlite3_malloc64(आकार(*p));
  जर( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }अन्यथा{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  परत p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
अक्षर *sqlite3VMPrintf(sqlite3 *db, स्थिरांक अक्षर *zFormat, बहु_सूची ap){
  अक्षर *z;
  अक्षर zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  प्रतिज्ञा( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, आकार(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  जर( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  परत z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
अक्षर *sqlite3MPrintf(sqlite3 *db, स्थिरांक अक्षर *zFormat, ...){
  बहु_सूची ap;
  अक्षर *z;
  बहु_शुरू(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  बहु_पूर्ण(ap);
  परत z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
अक्षर *sqlite3_vmprintf(स्थिरांक अक्षर *zFormat, बहु_सूची ap){
  अक्षर *z;
  अक्षर zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  जर( zFormat==0 ){
    (शून्य)SQLITE_MISUSE_BKPT;
    परत 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  जर( sqlite3_initialize() ) परत 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, आकार(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  परत z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
अक्षर *sqlite3_mprintf(स्थिरांक अक्षर *zFormat, ...){
  बहु_सूची ap;
  अक्षर *z;
#ifndef SQLITE_OMIT_AUTOINIT
  जर( sqlite3_initialize() ) परत 0;
#endif
  बहु_शुरू(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  बहु_पूर्ण(ap);
  परत z;
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
अक्षर *sqlite3_vsnprintf(पूर्णांक n, अक्षर *zBuf, स्थिरांक अक्षर *zFormat, बहु_सूची ap){
  StrAccum acc;
  जर( n<=0 ) परत zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( zBuf==0 || zFormat==0 ) {
    (शून्य)SQLITE_MISUSE_BKPT;
    जर( zBuf ) zBuf[0] = 0;
    परत zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  परत zBuf;
}
अक्षर *sqlite3_snprintf(पूर्णांक n, अक्षर *zBuf, स्थिरांक अक्षर *zFormat, ...){
  StrAccum acc;
  बहु_सूची ap;
  जर( n<=0 ) परत zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  जर( zBuf==0 || zFormat==0 ) {
    (शून्य)SQLITE_MISUSE_BKPT;
    जर( zBuf ) zBuf[0] = 0;
    परत zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  बहु_शुरू(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  बहु_पूर्ण(ap);
  zBuf[acc.nChar] = 0;
  परत zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#जर defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#किंवाजर SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#अन्यथा
# define SQLITE_MAX_LOG_MESSAGE (SQLITE_PRINT_BUF_SIZE*10)
#endif

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
स्थिर शून्य render_Log_संदेश(पूर्णांक iErrCode, स्थिरांक अक्षर *zFormat, बहु_सूची ap){
  StrAccum acc;                          /* String accumulator */
  अक्षर z_संदेश[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_संदेश, आकार(z_संदेश), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
शून्य sqlite3_log(पूर्णांक iErrCode, स्थिरांक अक्षर *zFormat, ...){
  बहु_सूची ap;                             /* Vararg list */
  जर( sqlite3GlobalConfig.xLog ){
    बहु_शुरू(ap, zFormat);
    render_Log_संदेश(iErrCode, zFormat, ap);
    बहु_पूर्ण(ap);
  }
}

#जर defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
शून्य sqlite3DebugPrintf(स्थिरांक अक्षर *zFormat, ...){
  बहु_सूची ap;
  StrAccum acc;
  अक्षर zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, आकार(zBuf), 0);
  बहु_शुरू(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  बहु_पूर्ण(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    बाह्य शून्य SQLITE_OS_TRACE_PROC(स्थिरांक अक्षर *zBuf, पूर्णांक nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, आकार(zBuf));
  }
#अन्यथा
  ख_लिखो(मानक_निकास,"%s", zBuf);
  ख_साफ(मानक_निकास);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
शून्य sqlite3_str_appendf(StrAccum *p, स्थिरांक अक्षर *zFormat, ...){
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
  प्रतिज्ञा( p!=0 );
  p--;
  p->nRCRef++;
  परत z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
शून्य sqlite3RCStrUnref(शून्य *z){
  RCStr *p = (RCStr*)z;
  प्रतिज्ञा( p!=0 );
  p--;
  प्रतिज्ञा( p->nRCRef>0 );
  जर( p->nRCRef>=2 ){
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
  RCStr *p = sqlite3_malloc64( N + आकार(*p) + 1 );
  जर( p==0 ) परत 0;
  p->nRCRef = 1;
  परत (अक्षर*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
अक्षर *sqlite3RCStrResize(अक्षर *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  प्रतिज्ञा( p!=0 );
  p--;
  प्रतिज्ञा( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+आकार(RCStr)+1);
  जर( pNew==0 ){
    sqlite3_free(p);
    परत 0;
  }अन्यथा{
    परत (अक्षर*)&pNew[1];
  }
}
