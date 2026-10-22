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
#شامل "sqliteInt.h"

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
تعریفنوع unsigned نویسه etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
تعریفنوع ساختار et_info {   /* Information about each format field */
  نویسه fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  نویسه iNxt;               /* Next with same hash, or 0 for end of chain */
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
#اگر 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
برای {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    اگر {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

ایستا ثابت نویسه aDigits[] = "0123456789ABCDEF0123456789abcdef";
ایستا ثابت نویسه aHex[]    = "0123456789abcdef";
ایستا ثابت نویسه aPrefix[] = "-x0\000X0";
ایستا ثابت et_info fmtinfo[25] = {
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
تهی sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  أكد( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  اگر( p->mxAlloc ) sqlite3_str_reset(p);
  اگر( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ایستا sqlite3_int64 getIntArg(PrintfArguments *p){
  اگر( p->nArg<=p->nUsed ) بازگشت 0;
  بازگشت sqlite3_مقدار_int64(p->apArg[p->nUsed++]);
}
ایستا دوگانه getDoubleArg(PrintfArguments *p){
  اگر( p->nArg<=p->nUsed ) بازگشت 0.0;
  بازگشت sqlite3_مقدار_double(p->apArg[p->nUsed++]);
}
ایستا نویسه *getTextArg(PrintfArguments *p){
  اگر( p->nArg<=p->nUsed ) بازگشت 0;
  بازگشت (نویسه*)sqlite3_مقدار_text(p->apArg[p->nUsed++]);
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
ایستا نویسه *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  نویسه *z;
  اگر( pAccum->accError ) بازگشت 0;
  اگر( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    بازگشت 0;
  }
  z = sqlite3_malloc(n);
  اگر( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  بازگشت z;
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
# اگر SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# وگرنه
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#وگرنه
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
ایستا تهی sqlite3StrAppend64(sqlite3_str *p, ثابت نویسه *z, i64 N);
ایستا تهی sqlite3StrAppendchar64(sqlite3_str *p, i64 N, نویسه c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
تهی sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ثابت نویسه *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  صحیح c;                     /* Next character in the format string */
  نویسه *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  صحیح idx;                   /* A general purpose loop counter */
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
  نویسه prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  دوگانه realvalue;          /* Value for real types */
  ثابت et_info *infop;      /* Pointer to the appropriate info structure */
  نویسه *zOut;                /* Rendering buffer */
  صحیح nOut;                  /* Size of the rendering buffer */
  نویسه *zExtra = 0;          /* Malloced memory used by some conversion */
  صحیح exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  نویسه buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  أكد( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  اگر( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }وگرنه{
    bArgList = 0;
  }
  برای(; (c=(*fmt))!=0; ++fmt){
    اگر( c!='%' ){
      bufpt = (نویسه *)fmt;
#اگر HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#وگرنه
      fmt = strchr(fmt, '%');
      اگر( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      اگر( *fmt==0 ) بشکن;
    }
    اگر( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      بشکن;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    انجامبده{
      انتخاب( c ){
        حالت '-':   flag_leftjustify = 1;     بشکن;
        حالت '+':   flag_prefix = '+';        بشکن;
        حالت ' ':   flag_prefix = ' ';        بشکن;
        حالت '#':   flag_alternateform = 1;   بشکن;
        حالت '!':   flag_altform2 = 1;        بشکن;
        حالت '0':   flag_zeropad = 1;         بشکن;
        حالت ',':   cThousand = ',';          بشکن;
        پیشفرض:    done = 1;                 بشکن;
        حالت 'l': {
          flag_long = 1;
          c = *++fmt;
          اگر( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          بشکن;
        }
        حالت '1': حالت '2': حالت '3': حالت '4': حالت '5':
        حالت '6': حالت '7': حالت '8': حالت '9': {
          unsigned wx = c - '0';
          وقتی( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          اگر( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          اگر( c!='.' && c!='l' ){
            done = 1;
          }وگرنه{
            fmt--;
          }
          بشکن;
        }
        حالت '*': {
          اگر( bArgList ){
            width = (صحیح)getIntArg(pArgList);
          }وگرنه{
            width = va_arg(ap,صحیح);
          }
          اگر( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          اگر( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          اگر( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          بشکن;
        }
        حالت '.': {
          c = *++fmt;
          اگر( c=='*' ){
            اگر( bArgList ){
              precision = (صحیح)getIntArg(pArgList);
            }وگرنه{
              precision = va_arg(ap,صحیح);
            }
            اگر( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }وگرنه{
            unsigned px = 0;
            وقتی( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          اگر( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          اگر( c=='l' ){
            --fmt;
          }وگرنه{
            done = 1;
          }
          بشکن;
        }
      }
    }وقتی( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    برای(idx=0; idx<ArraySize(fmtinfo); idx++){
      اگر( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        بشکن;
      }
    }
#وگرنه
    /* Fast hash-table lookup */
    أكد( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    اگر( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }وگرنه{
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
    أكد( width>=0 );
    أكد( precision>=(-1) );
    انتخاب( xtype ){
      حالت etPOINTER:
        flag_long = اندازه(نویسه*)==اندازه(i64) ? 2 :
                     اندازه(نویسه*)==اندازه(long صحیح) ? 1 : 0;
        /* no break */ deliberate_fall_through
      حالت etORDINAL:
      حالت etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      حالت etDECIMAL:
        اگر( infop->flags & FLAG_SIGNED ){
          i64 v;
          اگر( bArgList ){
            v = getIntArg(pArgList);
          }وگرنه اگر( flag_long ){
            اگر( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }وگرنه{
              v = va_arg(ap,long صحیح);
            }
          }وگرنه{
            v = va_arg(ap,صحیح);
          }
          اگر( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }وگرنه{
            longvalue = v;
            prefix = flag_prefix;
          }
        }وگرنه{
          اگر( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }وگرنه اگر( flag_long ){
            اگر( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }وگرنه{
              longvalue = va_arg(ap,unsigned long صحیح);
            }
          }وگرنه{
            longvalue = va_arg(ap,unsigned صحیح);
          }
          prefix = 0;
        }

#اگر WHERETRACE_ENABLED
        اگر( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#اگر TREETRACE_ENABLED
        اگر( xtype==etPOINTER && sqlite3_درخت_Trace & 0x100000 ) longvalue = 0;
#endif

        اگر( longvalue==0 ) flag_alternateform = 0;
        اگر( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        اگر( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }وگرنه{
          u64 n;
          n = (u64)precision + 10;
          اگر( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          اگر( zOut==0 ) بازگشت;
          nOut = (صحیح)n;
        }
        bufpt = &zOut[nOut-1];
        اگر( xtype==etORDINAL ){
          ایستا ثابت نویسه zOrd[] = "thstndrd";
          صحیح x = (صحیح)(longvalue % 10);
          اگر( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ثابت نویسه *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          انجامبده{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }وقتی( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        اگر( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        اگر( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          صحیح ii;
          bufpt -= nn;
          برای(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            اگر( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        اگر( prefix ) *(--bufpt) = prefix;               /* Add sign */
        اگر( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          ثابت نویسه *pre;
          نویسه x;
          pre = &aPrefix[infop->prefix];
          برای(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        بشکن;
      حالت etFLOAT:
      حالت etEXP:
      حالت etGENERIC: {
        FpDecode s;
        صحیح iRound;
        صحیح j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        اگر( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }وگرنه{
          realvalue = va_arg(ap,دوگانه);
        }
        اگر( precision<0 ) precision = 6;         /* Set default precision */
        اگر( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        اگر( xtype==etFLOAT ){
          iRound = -precision;
        }وگرنه اگر( xtype==etGENERIC ){
          اگر( precision==0 ) precision = 1;
          iRound = precision;
        }وگرنه{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        اگر( s.isSpecial ){
          اگر( s.isSpecial==2 ){
            اگر( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }وگرنه{
              bufpt = "NaN";
              length = 3;
            }
            بشکن;
          }وگرنه اگر( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }وگرنه{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            اگر( s.sign=='-' ){
              /* no-op */
            }وگرنه اگر( flag_prefix ){
              buf[0] = flag_prefix;
            }وگرنه{
              bufpt++;
            }
            length = strlen(bufpt);
            بشکن;
          }
        }
        اگر( s.sign=='-' ){
          اگر( flag_alternateform
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
          }وگرنه{
            prefix = '-';
          }
        }وگرنه{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        اگر( xtype==etGENERIC ){
          أكد( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          اگر( exp<-4 || exp>precision ){
            xtype = etEXP;
          }وگرنه{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }وگرنه{
          flag_rtz = flag_altform2;
        }
        اگر( xtype==etEXP ){
          e2 = 0;
        }وگرنه{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        اگر( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        اگر( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          اگر( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            اگر( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              بازگشت;
            }
            zExtra = bufpt;
          }وگرنه اگر( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            بشکن;
          }وگرنه{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }وگرنه{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        اگر( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        أكد( s.n>0 );
        اگر( e2<0 ){
          *(bufpt++) = '0';
        }وگرنه اگر( cThousand ){
          برای(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            اگر( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }وگرنه{
          j = e2+1;
          اگر( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          اگر( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        اگر( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        اگر( e2<(-1) && precision>0 ){
          صحیح nn = -1-e2;
          اگر( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        اگر( precision>0 ){
          صحیح nn = s.n - j;
          اگر( NEVER(nn>precision) ) nn = precision;
          اگر( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          اگر( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        اگر( flag_rtz && flag_dp ){
          وقتی( bufpt[-1]=='0' ) *(--bufpt) = 0;
          أكد( bufpt>zOut );
          اگر( bufpt[-1]=='.' ){
            اگر( flag_altform2 ){
              *(bufpt++) = '0';
            }وگرنه{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        اگر( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          اگر( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }وگرنه{
            *(bufpt++) = '+';
          }
          اگر( exp>=100 ){
            *(bufpt++) = (نویسه)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (نویسه)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (نویسه)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        أكد( length <= szBufNeeded );
        اگر( length<width ){
          i64 nPad = width - length;
          اگر( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }وگرنه اگر( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }وگرنه{
            صحیح adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        اگر( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          أكد( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          ادامه;
        }وگرنه{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          بشکن;
        }
      }
      حالت etSIZE:
        اگر( !bArgList ){
          *(va_arg(ap,صحیح*)) = pAccum->nChar;
        }
        length = width = 0;
        بشکن;
      حالت etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        بشکن;
      حالت etCHARX:
        اگر( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          اگر( bufpt ){
            buf[0] = c = *(bufpt++);
            اگر( (c&0xc0)==0xc0 ){
              وقتی( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }وگرنه{
            buf[0] = 0;
          }
        }وگرنه{
          unsigned صحیح ch = va_arg(ap,unsigned صحیح);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        اگر( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          اگر( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          وقتی( precision > 1 ){
            i64 nCopyBytes;
            اگر( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            اگر( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              بشکن;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        برو adjust_width_for_utf8;
      حالت etSTRING:
      حالت etDYNSTRING:
        اگر( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }وگرنه{
          bufpt = va_arg(ap,نویسه*);
        }
        اگر( bufpt==0 ){
          bufpt = "";
        }وگرنه اگر( xtype==etDYNSTRING ){
          اگر( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            أكد( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (صحیح)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            بشکن;
          }
          zExtra = bufpt;
        }
        اگر( precision>=0 ){
          اگر( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned نویسه *z = (unsigned نویسه*)bufpt;
            وقتی( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned نویسه*)bufpt;
          }وگرنه{
            برای(length=0; length<precision && bufpt[length]; length++){}
          }
        }وگرنه{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        اگر( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          وقتی( ii>=0 ) اگر( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        بشکن;
      حالت etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      حالت etESCAPE_J: {         /* %J: Generate a JSON string literal */
        نویسه *escarg;
        i64 i, j, px, iStart;
        unsigned نویسه ch;

        اگر( bArgList ){
          escarg = getTextArg(pArgList);
        }وگرنه{
          escarg = va_arg(ap,نویسه*);
        }
        iStart = sqlite3_str_length(pAccum);
        اگر( escarg==0 ){
          اگر( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }وگرنه{
          اگر( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          اگر( px<0 ){
            px = 0x7fffffff;
          }وگرنه اگر( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            برای(i=0; i<px && escarg[i]; i++){
              اگر( (escarg[i]&0xc0)==0x80 ) px++;
            }
            اگر( i==px ){
              وقتی( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          برای(i=j=0; i<px; i++){
            اگر( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              اگر( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              اگر( ch==0 ) بشکن;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              اگر( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }وگرنه اگر( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }وگرنه{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          اگر( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          اگر( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        اگر( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          نویسه *zz;
          اگر( flag_altform2 && n>0 ){
            zz = sqlite3_str_مقدار(pAccum);
            برای(i=iStart; zz[i]; i++){
              اگر( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          اگر( width>len ){
            sqlite3_int64 sp = width-len;
            أكد( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (صحیح)sp, ' ');
            اگر( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_مقدار(pAccum);
              zz += iStart;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        ادامه;
      }
      حالت etESCAPE_q:          /* %q: Escape ' characters */
      حالت etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      حالت etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        صحیح needQuote = 0;
        نویسه ch;
        نویسه *escarg;
        نویسه q;

        اگر( bArgList ){
          escarg = getTextArg(pArgList);
        }وگرنه{
          escarg = va_arg(ap,نویسه*);
        }
        اگر( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }وگرنه اگر( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        اگر( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }وگرنه{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        برای(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          اگر( ch==q )  n++;
          اگر( flag_altform2 && (ch&0xc0)==0xc0 ){
            وقتی( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        اگر( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          برای(k=0; k<i; k++){
            اگر( escarg[k]=='\\' ){
              nBack++;
            }وگرنه اگر( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          اگر( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            اگر( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }وگرنه{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        اگر( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          اگر( bufpt==0 ) بازگشت;
        }وگرنه{
          bufpt = buf;
        }
        j = 0;
        اگر( needQuote ){
          اگر( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }وگرنه{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        اگر( flag_alternateform ){
          برای(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            اگر( ch==q ){
              bufpt[j++] = ch;
            }وگرنه اگر( ch=='\\' ){
              bufpt[j++] = '\\';
            }وگرنه اگر( ((unsigned نویسه)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }وگرنه{
          برای(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            اگر( ch==q ) bufpt[j++] = ch;
          }
        }
        اگر( needQuote ){
          bufpt[j++] = '\'';
          اگر( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        برو adjust_width_for_utf8;
      }
      حالت etTOKEN: {
        اگر( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) بازگشت;
        اگر( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          عبارت *p_عبارت = va_arg(ap,عبارت*);
          اگر( ALWAYS(p_عبارت) && ALWAYS(!عبارت_Has_Property(p_عبارت,EP_Int_مقدار)) ){
            sqlite3_str_appendall(pAccum, (ثابت نویسه*)p_عبارت->u.zToken);
            sqlite3_Record_Error_Offset_Of_عبارت(pAccum->db, p_عبارت);
          }
        }وگرنه{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          أكد( bArgList==0 );
          اگر( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ثابت نویسه*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        بشکن;
      }
      حالت etSRCITEM: {
        SrcItem *pItem;
        اگر( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) بازگشت;
        pItem = va_arg(ap, SrcItem*);
        أكد( bArgList==0 );
        اگر( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }وگرنه اگر( pItem->zName ){
          اگر( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }وگرنه اگر( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }وگرنه اگر( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          أكد( pSel!=0 ); 
          اگر( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }وگرنه اگر( pSel->selFlags & SF_Multi_مقدار ){
            أكد( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }وگرنه{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        بشکن;
      }
      پیشفرض: {
        أكد( xtype==etINVALID );
        بازگشت;
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
    اگر( width>0 ){
      اگر( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      اگر( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }وگرنه{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    اگر( zExtra ){
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
تهی sqlite3RecordErrorByteOffset(sqlite3 *db, ثابت نویسه *z){
  ثابت Parse *pParse;
  ثابت نویسه *zText;
  ثابت نویسه *zEnd;
  أكد( z!=0 );
  اگر( NEVER(db==0) ) بازگشت;
  اگر( db->errByteOffset!=(-2) ) بازگشت;
  pParse = db->pParse;
  اگر( NEVER(pParse==0) ) بازگشت;
  zText =pParse->zTail;
  اگر( NEVER(zText==0) ) بازگشت;
  zEnd = &zText[strlen(zText)];
  اگر( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (صحیح)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
تهی sqlite3_Record_Error_Offset_Of_عبارت(sqlite3 *db, ثابت عبارت *p_عبارت){
  وقتی( p_عبارت ){
    اگر( عبارت_Has_Property(p_عبارت, EP_Reduced|EP_TokenOnly) ) بازگشت;
    اگر( عبارت_Has_Property(p_عبارت,EP_OuterON|EP_InnerON) || p_عبارت->w.iOfst<=0 ){
      p_عبارت = p_عبارت->p_چپ;
    }وگرنه{
      بشکن;
    }
  }
  اگر( p_عبارت==0 ) بازگشت;
  اگر( عبارت_Has_Property(p_عبارت, EP_FromDDL) ) بازگشت;
  db->errByteOffset = p_عبارت->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
صحیح sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  نویسه *zNew;
  أكد( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  اگر( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    بازگشت 0;
  }
  اگر( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    بازگشت p->nAlloc - p->nChar - 1;
  }وگرنه{
    نویسه *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    اگر( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    اگر( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      بازگشت 0;
    }وگرنه{
      p->nAlloc = (صحیح)szNew;
    }
    اگر( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }وگرنه{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    اگر( zNew ){
      أكد( p->zText!=0 || p->nChar==0 );
      اگر( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }وگرنه{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      بازگشت 0;
    }
  }
  أكد( N>=0 && N<=0x7fffffff );
  بازگشت (صحیح)N;
}

صحیح sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  اگر( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  بازگشت p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
تهی sqlite3_str_appendchar(sqlite3_str *p, صحیح N, نویسه c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  اگر( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    بازگشت;
  }
  وقتی( (N--)>0 ) p->zText[p->nChar++] = c;
}
ایستا تهی sqlite3StrAppendchar64(sqlite3_str *p, i64 N, نویسه c){
  testcase( p->nChar + N > 0x7fffffff );
  اگر( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    بازگشت;
  }
  وقتی( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
ایستا تهی SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ثابت نویسه *z, صحیح N){
  N = sqlite3StrAccumEnlarge(p, N);
  أكد( z!=0 || N==0 );
  اگر( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
تهی sqlite3_str_append(sqlite3_str *p, ثابت نویسه *z, صحیح N){
  أكد( z!=0 || N==0 );
  أكد( p->zText!=0 || p->nChar==0 || p->accError );
  أكد( N>=0 );
  أكد( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  اگر( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }وگرنه اگر( N ){
    أكد( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
ایستا تهی sqlite3StrAppend64(sqlite3_str *p, ثابت نویسه *z, i64 N){
  أكد( z!=0 || N==0 );
  أكد( p->zText!=0 || p->nChar==0 || p->accError );
  أكد( N>=0 );
  أكد( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  اگر( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }وگرنه اگر( N ){
    أكد( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
تهی sqlite3_str_appendall(sqlite3_str *p, ثابت نویسه *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
ایستا SQLITE_NOINLINE نویسه *strAccumFinishRealloc(StrAccum *p){
  نویسه *zText;
  أكد( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  اگر( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }وگرنه{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  بازگشت zText;
}
نویسه *sqlite3StrAccumFinish(StrAccum *p){
  اگر( p->zText ){
    p->zText[p->nChar] = 0;
    اگر( p->mxAlloc>0 && !isMalloced(p) ){
      بازگشت strAccumFinishRealloc(p);
    }
  }
  بازگشت p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
نویسه *sqlite3_str_finish(sqlite3_str *p){
  نویسه *z;
  اگر( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }وگرنه{
    z = 0;
  }
  بازگشت z;
}

/* Return any error code associated with p */
صحیح sqlite3_str_errcode(sqlite3_str *p){
  بازگشت p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
صحیح sqlite3_str_length(sqlite3_str *p){
  بازگشت p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
تهی sqlite3_str_truncate(sqlite3_str *p, صحیح N){
  اگر( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
نویسه *sqlite3_str_مقدار(sqlite3_str *p){
  اگر( p==0 || p->nChar==0 ) بازگشت 0;
  p->zText[p->nChar] = 0;
  بازگشت p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
تهی sqlite3_str_reset(StrAccum *p){
  اگر( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }وگرنه اگر( p==(sqlite3_str*)&sqlite3OomStr ){
    بازگشت;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
تهی sqlite3_str_free(sqlite3_str *p){
  اگر( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
تهی sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, نویسه *zBase, صحیح n, صحیح mx){
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
  sqlite3_str *p = sqlite3_malloc64(اندازه(*p));
  اگر( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }وگرنه{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  بازگشت p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
نویسه *sqlite3VMPrintf(sqlite3 *db, ثابت نویسه *zFormat, va_list ap){
  نویسه *z;
  نویسه zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  أكد( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, اندازه(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  اگر( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  بازگشت z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
نویسه *sqlite3MPrintf(sqlite3 *db, ثابت نویسه *zFormat, ...){
  va_list ap;
  نویسه *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  بازگشت z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
نویسه *sqlite3_vmprintf(ثابت نویسه *zFormat, va_list ap){
  نویسه *z;
  نویسه zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  اگر( zFormat==0 ){
    (تهی)SQLITE_MISUSE_BKPT;
    بازگشت 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) بازگشت 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, اندازه(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  بازگشت z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
نویسه *sqlite3_mprintf(ثابت نویسه *zFormat, ...){
  va_list ap;
  نویسه *z;
#ifndef SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) بازگشت 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  بازگشت z;
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
نویسه *sqlite3_vsnprintf(صحیح n, نویسه *zBuf, ثابت نویسه *zFormat, va_list ap){
  StrAccum acc;
  اگر( n<=0 ) بازگشت zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( zBuf==0 || zFormat==0 ) {
    (تهی)SQLITE_MISUSE_BKPT;
    اگر( zBuf ) zBuf[0] = 0;
    بازگشت zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  بازگشت zBuf;
}
نویسه *sqlite3_snprintf(صحیح n, نویسه *zBuf, ثابت نویسه *zFormat, ...){
  StrAccum acc;
  va_list ap;
  اگر( n<=0 ) بازگشت zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( zBuf==0 || zFormat==0 ) {
    (تهی)SQLITE_MISUSE_BKPT;
    اگر( zBuf ) zBuf[0] = 0;
    بازگشت zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  بازگشت zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#اگر defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#وگرنهاگر SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#وگرنه
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
ایستا تهی render_Log_پیام(صحیح iErrCode, ثابت نویسه *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  نویسه z_پیام[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_پیام, اندازه(z_پیام), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
تهی sqlite3_log(صحیح iErrCode, ثابت نویسه *zFormat, ...){
  va_list ap;                             /* Vararg list */
  اگر( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_پیام(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
تهی sqlite3DebugPrintf(ثابت نویسه *zFormat, ...){
  va_list ap;
  StrAccum acc;
  نویسه zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, اندازه(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    بیرونی تهی SQLITE_OS_TRACE_PROC(ثابت نویسه *zBuf, صحیح nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, اندازه(zBuf));
  }
#وگرنه
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
تهی sqlite3_str_appendf(StrAccum *p, ثابت نویسه *zFormat, ...){
  va_list ap;
  va_start(ap,zFormat);
  sqlite3_str_vappendf(p, zFormat, ap);
  va_end(ap);
}


/*****************************************************************************
** Reference counted string/blob storage
*****************************************************************************/

/*
** Increase the reference count of the string by one.
**
** The input parameter is returned.
*/
نویسه *sqlite3RCStrRef(نویسه *z){
  RCStr *p = (RCStr*)z;
  أكد( p!=0 );
  p--;
  p->nRCRef++;
  بازگشت z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
تهی sqlite3RCStrUnref(تهی *z){
  RCStr *p = (RCStr*)z;
  أكد( p!=0 );
  p--;
  أكد( p->nRCRef>0 );
  اگر( p->nRCRef>=2 ){
    p->nRCRef--;
  }وگرنه{
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
نویسه *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + اندازه(*p) + 1 );
  اگر( p==0 ) بازگشت 0;
  p->nRCRef = 1;
  بازگشت (نویسه*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
نویسه *sqlite3RCStrResize(نویسه *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  أكد( p!=0 );
  p--;
  أكد( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+اندازه(RCStr)+1);
  اگر( pNew==0 ){
    sqlite3_free(p);
    بازگشت 0;
  }وگرنه{
    بازگشت (نویسه*)&pNew[1];
  }
}
