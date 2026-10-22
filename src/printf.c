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
#ضمن "sqliteInt.h"

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
تعريفنوع unsigned حرف etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
تعريفنوع بنية et_info {   /* Information about each format field */
  حرف fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  حرف iNxt;               /* Next with same hash, or 0 for end of chain */
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
#ܐܢ 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
لكل {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    ܐܢ {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

ساكن ثابت حرف aDigits[] = "0123456789ABCDEF0123456789abcdef";
ساكن ثابت حرف aHex[]    = "0123456789abcdef";
ساكن ثابت حرف aPrefix[] = "-x0\000X0";
ساكن ثابت et_info fmtinfo[25] = {
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
فراغ sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  أكد( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  ܐܢ( p->mxAlloc ) sqlite3_str_reset(p);
  ܐܢ( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ساكن sqlite3_int64 getIntArg(PrintfArguments *p){
  ܐܢ( p->nArg<=p->nUsed ) ܦܢܐ 0;
  ܦܢܐ sqlite3_value_int64(p->apArg[p->nUsed++]);
}
ساكن مزدوج getDoubleArg(PrintfArguments *p){
  ܐܢ( p->nArg<=p->nUsed ) ܦܢܐ 0.0;
  ܦܢܐ sqlite3_value_double(p->apArg[p->nUsed++]);
}
ساكن حرف *getTextArg(PrintfArguments *p){
  ܐܢ( p->nArg<=p->nUsed ) ܦܢܐ 0;
  ܦܢܐ (حرف*)sqlite3_value_text(p->apArg[p->nUsed++]);
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
ساكن حرف *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  حرف *z;
  ܐܢ( pAccum->accError ) ܦܢܐ 0;
  ܐܢ( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    ܦܢܐ 0;
  }
  z = sqlite3_malloc(n);
  ܐܢ( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  ܦܢܐ z;
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
# ܐܢ SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# ܐܚܪܢܐ
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#ܐܚܪܢܐ
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
ساكن فراغ sqlite3StrAppend64(sqlite3_str *p, ثابت حرف *z, i64 N);
ساكن فراغ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, حرف c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
فراغ sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ثابت حرف *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  صحيح c;                     /* Next character in the format string */
  حرف *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  صحيح idx;                   /* A general purpose loop counter */
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
  حرف prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  مزدوج realvalue;          /* Value for real types */
  ثابت et_info *infop;      /* Pointer to the appropriate info structure */
  حرف *zOut;                /* Rendering buffer */
  صحيح nOut;                  /* Size of the rendering buffer */
  حرف *zExtra = 0;          /* Malloced memory used by some conversion */
  صحيح exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  حرف buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  أكد( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  ܐܢ( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }ܐܚܪܢܐ{
    bArgList = 0;
  }
  لكل(; (c=(*fmt))!=0; ++fmt){
    ܐܢ( c!='%' ){
      bufpt = (حرف *)fmt;
#ܐܢ HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#ܐܚܪܢܐ
      fmt = strchr(fmt, '%');
      ܐܢ( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      ܐܢ( *fmt==0 ) اكسر;
    }
    ܐܢ( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      اكسر;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    ܥܒܕ{
      بدل( c ){
        حالة '-':   flag_leftjustify = 1;     اكسر;
        حالة '+':   flag_prefix = '+';        اكسر;
        حالة ' ':   flag_prefix = ' ';        اكسر;
        حالة '#':   flag_alternateform = 1;   اكسر;
        حالة '!':   flag_altform2 = 1;        اكسر;
        حالة '0':   flag_zeropad = 1;         اكسر;
        حالة ',':   cThousand = ',';          اكسر;
        افتراضي:    done = 1;                 اكسر;
        حالة 'l': {
          flag_long = 1;
          c = *++fmt;
          ܐܢ( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          اكسر;
        }
        حالة '1': حالة '2': حالة '3': حالة '4': حالة '5':
        حالة '6': حالة '7': حالة '8': حالة '9': {
          unsigned wx = c - '0';
          ܟܡܐ( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ܐܢ( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ܐܢ( c!='.' && c!='l' ){
            done = 1;
          }ܐܚܪܢܐ{
            fmt--;
          }
          اكسر;
        }
        حالة '*': {
          ܐܢ( bArgList ){
            width = (صحيح)getIntArg(pArgList);
          }ܐܚܪܢܐ{
            width = va_arg(ap,صحيح);
          }
          ܐܢ( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ܐܢ( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ܐܢ( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          اكسر;
        }
        حالة '.': {
          c = *++fmt;
          ܐܢ( c=='*' ){
            ܐܢ( bArgList ){
              precision = (صحيح)getIntArg(pArgList);
            }ܐܚܪܢܐ{
              precision = va_arg(ap,صحيح);
            }
            ܐܢ( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }ܐܚܪܢܐ{
            unsigned px = 0;
            ܟܡܐ( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ܐܢ( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ܐܢ( c=='l' ){
            --fmt;
          }ܐܚܪܢܐ{
            done = 1;
          }
          اكسر;
        }
      }
    }ܟܡܐ( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    لكل(idx=0; idx<ArraySize(fmtinfo); idx++){
      ܐܢ( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        اكسر;
      }
    }
#ܐܚܪܢܐ
    /* Fast hash-table lookup */
    أكد( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    ܐܢ( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }ܐܚܪܢܐ{
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
    بدل( xtype ){
      حالة etPOINTER:
        flag_long = حجم(حرف*)==حجم(i64) ? 2 :
                     حجم(حرف*)==حجم(long صحيح) ? 1 : 0;
        /* no break */ deliberate_fall_through
      حالة etORDINAL:
      حالة etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      حالة etDECIMAL:
        ܐܢ( infop->flags & FLAG_SIGNED ){
          i64 v;
          ܐܢ( bArgList ){
            v = getIntArg(pArgList);
          }ܐܚܪܢܐ ܐܢ( flag_long ){
            ܐܢ( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }ܐܚܪܢܐ{
              v = va_arg(ap,long صحيح);
            }
          }ܐܚܪܢܐ{
            v = va_arg(ap,صحيح);
          }
          ܐܢ( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }ܐܚܪܢܐ{
            longvalue = v;
            prefix = flag_prefix;
          }
        }ܐܚܪܢܐ{
          ܐܢ( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }ܐܚܪܢܐ ܐܢ( flag_long ){
            ܐܢ( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }ܐܚܪܢܐ{
              longvalue = va_arg(ap,unsigned long صحيح);
            }
          }ܐܚܪܢܐ{
            longvalue = va_arg(ap,unsigned صحيح);
          }
          prefix = 0;
        }

#ܐܢ WHERETRACE_ENABLED
        ܐܢ( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#ܐܢ TREETRACE_ENABLED
        ܐܢ( xtype==etPOINTER && sqlite3TreeTrace & 0x100000 ) longvalue = 0;
#endif

        ܐܢ( longvalue==0 ) flag_alternateform = 0;
        ܐܢ( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        ܐܢ( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }ܐܚܪܢܐ{
          u64 n;
          n = (u64)precision + 10;
          ܐܢ( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          ܐܢ( zOut==0 ) ܦܢܐ;
          nOut = (صحيح)n;
        }
        bufpt = &zOut[nOut-1];
        ܐܢ( xtype==etORDINAL ){
          ساكن ثابت حرف zOrd[] = "thstndrd";
          صحيح x = (صحيح)(longvalue % 10);
          ܐܢ( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ثابت حرف *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          ܥܒܕ{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }ܟܡܐ( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        ܐܢ( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        ܐܢ( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          صحيح ii;
          bufpt -= nn;
          لكل(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            ܐܢ( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        ܐܢ( prefix ) *(--bufpt) = prefix;               /* Add sign */
        ܐܢ( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          ثابت حرف *pre;
          حرف x;
          pre = &aPrefix[infop->prefix];
          لكل(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        اكسر;
      حالة etFLOAT:
      حالة etEXP:
      حالة etGENERIC: {
        FpDecode s;
        صحيح iRound;
        صحيح j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        ܐܢ( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }ܐܚܪܢܐ{
          realvalue = va_arg(ap,مزدوج);
        }
        ܐܢ( precision<0 ) precision = 6;         /* Set default precision */
        ܐܢ( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        ܐܢ( xtype==etFLOAT ){
          iRound = -precision;
        }ܐܚܪܢܐ ܐܢ( xtype==etGENERIC ){
          ܐܢ( precision==0 ) precision = 1;
          iRound = precision;
        }ܐܚܪܢܐ{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        ܐܢ( s.isSpecial ){
          ܐܢ( s.isSpecial==2 ){
            ܐܢ( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }ܐܚܪܢܐ{
              bufpt = "NaN";
              length = 3;
            }
            اكسر;
          }ܐܚܪܢܐ ܐܢ( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }ܐܚܪܢܐ{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            ܐܢ( s.sign=='-' ){
              /* no-op */
            }ܐܚܪܢܐ ܐܢ( flag_prefix ){
              buf[0] = flag_prefix;
            }ܐܚܪܢܐ{
              bufpt++;
            }
            length = strlen(bufpt);
            اكسر;
          }
        }
        ܐܢ( s.sign=='-' ){
          ܐܢ( flag_alternateform
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
          }ܐܚܪܢܐ{
            prefix = '-';
          }
        }ܐܚܪܢܐ{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        ܐܢ( xtype==etGENERIC ){
          أكد( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          ܐܢ( exp<-4 || exp>precision ){
            xtype = etEXP;
          }ܐܚܪܢܐ{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }ܐܚܪܢܐ{
          flag_rtz = flag_altform2;
        }
        ܐܢ( xtype==etEXP ){
          e2 = 0;
        }ܐܚܪܢܐ{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        ܐܢ( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        ܐܢ( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          ܐܢ( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            ܐܢ( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              ܦܢܐ;
            }
            zExtra = bufpt;
          }ܐܚܪܢܐ ܐܢ( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            اكسر;
          }ܐܚܪܢܐ{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }ܐܚܪܢܐ{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        ܐܢ( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        أكد( s.n>0 );
        ܐܢ( e2<0 ){
          *(bufpt++) = '0';
        }ܐܚܪܢܐ ܐܢ( cThousand ){
          لكل(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            ܐܢ( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }ܐܚܪܢܐ{
          j = e2+1;
          ܐܢ( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          ܐܢ( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        ܐܢ( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        ܐܢ( e2<(-1) && precision>0 ){
          صحيح nn = -1-e2;
          ܐܢ( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        ܐܢ( precision>0 ){
          صحيح nn = s.n - j;
          ܐܢ( NEVER(nn>precision) ) nn = precision;
          ܐܢ( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          ܐܢ( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        ܐܢ( flag_rtz && flag_dp ){
          ܟܡܐ( bufpt[-1]=='0' ) *(--bufpt) = 0;
          أكد( bufpt>zOut );
          ܐܢ( bufpt[-1]=='.' ){
            ܐܢ( flag_altform2 ){
              *(bufpt++) = '0';
            }ܐܚܪܢܐ{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        ܐܢ( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          ܐܢ( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }ܐܚܪܢܐ{
            *(bufpt++) = '+';
          }
          ܐܢ( exp>=100 ){
            *(bufpt++) = (حرف)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (حرف)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (حرف)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        أكد( length <= szBufNeeded );
        ܐܢ( length<width ){
          i64 nPad = width - length;
          ܐܢ( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }ܐܚܪܢܐ ܐܢ( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }ܐܚܪܢܐ{
            صحيح adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        ܐܢ( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          أكد( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          استمر;
        }ܐܚܪܢܐ{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          اكسر;
        }
      }
      حالة etSIZE:
        ܐܢ( !bArgList ){
          *(va_arg(ap,صحيح*)) = pAccum->nChar;
        }
        length = width = 0;
        اكسر;
      حالة etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        اكسر;
      حالة etCHARX:
        ܐܢ( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          ܐܢ( bufpt ){
            buf[0] = c = *(bufpt++);
            ܐܢ( (c&0xc0)==0xc0 ){
              ܟܡܐ( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }ܐܚܪܢܐ{
            buf[0] = 0;
          }
        }ܐܚܪܢܐ{
          unsigned صحيح ch = va_arg(ap,unsigned صحيح);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        ܐܢ( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          ܐܢ( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          ܟܡܐ( precision > 1 ){
            i64 nCopyBytes;
            ܐܢ( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            ܐܢ( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              اكسر;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        اذهب adjust_width_for_utf8;
      حالة etSTRING:
      حالة etDYNSTRING:
        ܐܢ( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }ܐܚܪܢܐ{
          bufpt = va_arg(ap,حرف*);
        }
        ܐܢ( bufpt==0 ){
          bufpt = "";
        }ܐܚܪܢܐ ܐܢ( xtype==etDYNSTRING ){
          ܐܢ( pAccum->nChar==0
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
            pAccum->nChar = 0x7fffffff & (صحيح)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            اكسر;
          }
          zExtra = bufpt;
        }
        ܐܢ( precision>=0 ){
          ܐܢ( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned حرف *z = (unsigned حرف*)bufpt;
            ܟܡܐ( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned حرف*)bufpt;
          }ܐܚܪܢܐ{
            لكل(length=0; length<precision && bufpt[length]; length++){}
          }
        }ܐܚܪܢܐ{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        ܐܢ( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          ܟܡܐ( ii>=0 ) ܐܢ( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        اكسر;
      حالة etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      حالة etESCAPE_J: {         /* %J: Generate a JSON string literal */
        حرف *escarg;
        i64 i, j, px, iStart;
        unsigned حرف ch;

        ܐܢ( bArgList ){
          escarg = getTextArg(pArgList);
        }ܐܚܪܢܐ{
          escarg = va_arg(ap,حرف*);
        }
        iStart = sqlite3_str_length(pAccum);
        ܐܢ( escarg==0 ){
          ܐܢ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }ܐܚܪܢܐ{
          ܐܢ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          ܐܢ( px<0 ){
            px = 0x7fffffff;
          }ܐܚܪܢܐ ܐܢ( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            لكل(i=0; i<px && escarg[i]; i++){
              ܐܢ( (escarg[i]&0xc0)==0x80 ) px++;
            }
            ܐܢ( i==px ){
              ܟܡܐ( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          لكل(i=j=0; i<px; i++){
            ܐܢ( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              ܐܢ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              ܐܢ( ch==0 ) اكسر;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              ܐܢ( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ܐܚܪܢܐ ܐܢ( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ܐܚܪܢܐ{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          ܐܢ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          ܐܢ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        ܐܢ( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          حرف *zz;
          ܐܢ( flag_altform2 && n>0 ){
            zz = sqlite3_str_value(pAccum);
            لكل(i=iStart; zz[i]; i++){
              ܐܢ( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          ܐܢ( width>len ){
            sqlite3_int64 sp = width-len;
            أكد( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (صحيح)sp, ' ');
            ܐܢ( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_value(pAccum);
              zz += iStart;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        استمر;
      }
      حالة etESCAPE_q:          /* %q: Escape ' characters */
      حالة etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      حالة etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        صحيح needQuote = 0;
        حرف ch;
        حرف *escarg;
        حرف q;

        ܐܢ( bArgList ){
          escarg = getTextArg(pArgList);
        }ܐܚܪܢܐ{
          escarg = va_arg(ap,حرف*);
        }
        ܐܢ( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }ܐܚܪܢܐ ܐܢ( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        ܐܢ( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }ܐܚܪܢܐ{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        لكل(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          ܐܢ( ch==q )  n++;
          ܐܢ( flag_altform2 && (ch&0xc0)==0xc0 ){
            ܟܡܐ( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        ܐܢ( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          لكل(k=0; k<i; k++){
            ܐܢ( escarg[k]=='\\' ){
              nBack++;
            }ܐܚܪܢܐ ܐܢ( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          ܐܢ( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            ܐܢ( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }ܐܚܪܢܐ{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        ܐܢ( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          ܐܢ( bufpt==0 ) ܦܢܐ;
        }ܐܚܪܢܐ{
          bufpt = buf;
        }
        j = 0;
        ܐܢ( needQuote ){
          ܐܢ( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }ܐܚܪܢܐ{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        ܐܢ( flag_alternateform ){
          لكل(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ܐܢ( ch==q ){
              bufpt[j++] = ch;
            }ܐܚܪܢܐ ܐܢ( ch=='\\' ){
              bufpt[j++] = '\\';
            }ܐܚܪܢܐ ܐܢ( ((unsigned حرف)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }ܐܚܪܢܐ{
          لكل(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ܐܢ( ch==q ) bufpt[j++] = ch;
          }
        }
        ܐܢ( needQuote ){
          bufpt[j++] = '\'';
          ܐܢ( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        اذهب adjust_width_for_utf8;
      }
      حالة etTOKEN: {
        ܐܢ( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ܦܢܐ;
        ܐܢ( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          Expr *pExpr = va_arg(ap,Expr*);
          ܐܢ( ALWAYS(pExpr) && ALWAYS(!ExprHasProperty(pExpr,EP_IntValue)) ){
            sqlite3_str_appendall(pAccum, (ثابت حرف*)pExpr->u.zToken);
            sqlite3RecordErrorOffsetOfExpr(pAccum->db, pExpr);
          }
        }ܐܚܪܢܐ{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          أكد( bArgList==0 );
          ܐܢ( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ثابت حرف*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        اكسر;
      }
      حالة etSRCITEM: {
        SrcItem *pItem;
        ܐܢ( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ܦܢܐ;
        pItem = va_arg(ap, SrcItem*);
        أكد( bArgList==0 );
        ܐܢ( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }ܐܚܪܢܐ ܐܢ( pItem->zName ){
          ܐܢ( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }ܐܚܪܢܐ ܐܢ( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }ܐܚܪܢܐ ܐܢ( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          أكد( pSel!=0 ); 
          ܐܢ( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }ܐܚܪܢܐ ܐܢ( pSel->selFlags & SF_MultiValue ){
            أكد( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }ܐܚܪܢܐ{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        اكسر;
      }
      افتراضي: {
        أكد( xtype==etINVALID );
        ܦܢܐ;
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
    ܐܢ( width>0 ){
      ܐܢ( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      ܐܢ( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }ܐܚܪܢܐ{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    ܐܢ( zExtra ){
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
فراغ sqlite3RecordErrorByteOffset(sqlite3 *db, ثابت حرف *z){
  ثابت Parse *pParse;
  ثابت حرف *zText;
  ثابت حرف *zEnd;
  أكد( z!=0 );
  ܐܢ( NEVER(db==0) ) ܦܢܐ;
  ܐܢ( db->errByteOffset!=(-2) ) ܦܢܐ;
  pParse = db->pParse;
  ܐܢ( NEVER(pParse==0) ) ܦܢܐ;
  zText =pParse->zTail;
  ܐܢ( NEVER(zText==0) ) ܦܢܐ;
  zEnd = &zText[strlen(zText)];
  ܐܢ( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (صحيح)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
فراغ sqlite3RecordErrorOffsetOfExpr(sqlite3 *db, ثابت Expr *pExpr){
  ܟܡܐ( pExpr ){
    ܐܢ( ExprHasProperty(pExpr, EP_Reduced|EP_TokenOnly) ) ܦܢܐ;
    ܐܢ( ExprHasProperty(pExpr,EP_OuterON|EP_InnerON) || pExpr->w.iOfst<=0 ){
      pExpr = pExpr->pLeft;
    }ܐܚܪܢܐ{
      اكسر;
    }
  }
  ܐܢ( pExpr==0 ) ܦܢܐ;
  ܐܢ( ExprHasProperty(pExpr, EP_FromDDL) ) ܦܢܐ;
  db->errByteOffset = pExpr->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
صحيح sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  حرف *zNew;
  أكد( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  ܐܢ( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    ܦܢܐ 0;
  }
  ܐܢ( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    ܦܢܐ p->nAlloc - p->nChar - 1;
  }ܐܚܪܢܐ{
    حرف *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    ܐܢ( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    ܐܢ( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      ܦܢܐ 0;
    }ܐܚܪܢܐ{
      p->nAlloc = (صحيح)szNew;
    }
    ܐܢ( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }ܐܚܪܢܐ{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    ܐܢ( zNew ){
      أكد( p->zText!=0 || p->nChar==0 );
      ܐܢ( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }ܐܚܪܢܐ{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      ܦܢܐ 0;
    }
  }
  أكد( N>=0 && N<=0x7fffffff );
  ܦܢܐ (صحيح)N;
}

صحيح sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  ܐܢ( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  ܦܢܐ p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
فراغ sqlite3_str_appendchar(sqlite3_str *p, صحيح N, حرف c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  ܐܢ( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ܦܢܐ;
  }
  ܟܡܐ( (N--)>0 ) p->zText[p->nChar++] = c;
}
ساكن فراغ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, حرف c){
  testcase( p->nChar + N > 0x7fffffff );
  ܐܢ( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ܦܢܐ;
  }
  ܟܡܐ( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
ساكن فراغ SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ثابت حرف *z, صحيح N){
  N = sqlite3StrAccumEnlarge(p, N);
  أكد( z!=0 || N==0 );
  ܐܢ( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
فراغ sqlite3_str_append(sqlite3_str *p, ثابت حرف *z, صحيح N){
  أكد( z!=0 || N==0 );
  أكد( p->zText!=0 || p->nChar==0 || p->accError );
  أكد( N>=0 );
  أكد( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ܐܢ( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ܐܚܪܢܐ ܐܢ( N ){
    أكد( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
ساكن فراغ sqlite3StrAppend64(sqlite3_str *p, ثابت حرف *z, i64 N){
  أكد( z!=0 || N==0 );
  أكد( p->zText!=0 || p->nChar==0 || p->accError );
  أكد( N>=0 );
  أكد( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ܐܢ( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ܐܚܪܢܐ ܐܢ( N ){
    أكد( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
فراغ sqlite3_str_appendall(sqlite3_str *p, ثابت حرف *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
ساكن SQLITE_NOINLINE حرف *strAccumFinishRealloc(StrAccum *p){
  حرف *zText;
  أكد( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  ܐܢ( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }ܐܚܪܢܐ{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  ܦܢܐ zText;
}
حرف *sqlite3StrAccumFinish(StrAccum *p){
  ܐܢ( p->zText ){
    p->zText[p->nChar] = 0;
    ܐܢ( p->mxAlloc>0 && !isMalloced(p) ){
      ܦܢܐ strAccumFinishRealloc(p);
    }
  }
  ܦܢܐ p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
حرف *sqlite3_str_finish(sqlite3_str *p){
  حرف *z;
  ܐܢ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }ܐܚܪܢܐ{
    z = 0;
  }
  ܦܢܐ z;
}

/* Return any error code associated with p */
صحيح sqlite3_str_errcode(sqlite3_str *p){
  ܦܢܐ p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
صحيح sqlite3_str_length(sqlite3_str *p){
  ܦܢܐ p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
فراغ sqlite3_str_truncate(sqlite3_str *p, صحيح N){
  ܐܢ( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
حرف *sqlite3_str_value(sqlite3_str *p){
  ܐܢ( p==0 || p->nChar==0 ) ܦܢܐ 0;
  p->zText[p->nChar] = 0;
  ܦܢܐ p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
فراغ sqlite3_str_reset(StrAccum *p){
  ܐܢ( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }ܐܚܪܢܐ ܐܢ( p==(sqlite3_str*)&sqlite3OomStr ){
    ܦܢܐ;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
فراغ sqlite3_str_free(sqlite3_str *p){
  ܐܢ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
فراغ sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, حرف *zBase, صحيح n, صحيح mx){
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
  sqlite3_str *p = sqlite3_malloc64(حجم(*p));
  ܐܢ( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }ܐܚܪܢܐ{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  ܦܢܐ p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
حرف *sqlite3VMPrintf(sqlite3 *db, ثابت حرف *zFormat, va_list ap){
  حرف *z;
  حرف zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  أكد( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, حجم(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ܐܢ( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ܦܢܐ z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
حرف *sqlite3MPrintf(sqlite3 *db, ثابت حرف *zFormat, ...){
  va_list ap;
  حرف *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ܦܢܐ z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
حرف *sqlite3_vmprintf(ثابت حرف *zFormat, va_list ap){
  حرف *z;
  حرف zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  ܐܢ( zFormat==0 ){
    (فراغ)SQLITE_MISUSE_BKPT;
    ܦܢܐ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ܐܢ( sqlite3_initialize() ) ܦܢܐ 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, حجم(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ܦܢܐ z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
حرف *sqlite3_mprintf(ثابت حرف *zFormat, ...){
  va_list ap;
  حرف *z;
#ifndef SQLITE_OMIT_AUTOINIT
  ܐܢ( sqlite3_initialize() ) ܦܢܐ 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  ܦܢܐ z;
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
حرف *sqlite3_vsnprintf(صحيح n, حرف *zBuf, ثابت حرف *zFormat, va_list ap){
  StrAccum acc;
  ܐܢ( n<=0 ) ܦܢܐ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  ܐܢ( zBuf==0 || zFormat==0 ) {
    (فراغ)SQLITE_MISUSE_BKPT;
    ܐܢ( zBuf ) zBuf[0] = 0;
    ܦܢܐ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  ܦܢܐ zBuf;
}
حرف *sqlite3_snprintf(صحيح n, حرف *zBuf, ثابت حرف *zFormat, ...){
  StrAccum acc;
  va_list ap;
  ܐܢ( n<=0 ) ܦܢܐ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  ܐܢ( zBuf==0 || zFormat==0 ) {
    (فراغ)SQLITE_MISUSE_BKPT;
    ܐܢ( zBuf ) zBuf[0] = 0;
    ܦܢܐ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  ܦܢܐ zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#ܐܢ defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#وإلاإذا SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#ܐܚܪܢܐ
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
ساكن فراغ renderLogMsg(صحيح iErrCode, ثابت حرف *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  حرف zMsg[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, zMsg, حجم(zMsg), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
فراغ sqlite3_log(صحيح iErrCode, ثابت حرف *zFormat, ...){
  va_list ap;                             /* Vararg list */
  ܐܢ( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    renderLogMsg(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#ܐܢ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
فراغ sqlite3DebugPrintf(ثابت حرف *zFormat, ...){
  va_list ap;
  StrAccum acc;
  حرف zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    خارجي فراغ SQLITE_OS_TRACE_PROC(ثابت حرف *zBuf, صحيح nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, حجم(zBuf));
  }
#ܐܚܪܢܐ
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
فراغ sqlite3_str_appendf(StrAccum *p, ثابت حرف *zFormat, ...){
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
حرف *sqlite3RCStrRef(حرف *z){
  RCStr *p = (RCStr*)z;
  أكد( p!=0 );
  p--;
  p->nRCRef++;
  ܦܢܐ z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
فراغ sqlite3RCStrUnref(فراغ *z){
  RCStr *p = (RCStr*)z;
  أكد( p!=0 );
  p--;
  أكد( p->nRCRef>0 );
  ܐܢ( p->nRCRef>=2 ){
    p->nRCRef--;
  }ܐܚܪܢܐ{
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
حرف *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + حجم(*p) + 1 );
  ܐܢ( p==0 ) ܦܢܐ 0;
  p->nRCRef = 1;
  ܦܢܐ (حرف*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
حرف *sqlite3RCStrResize(حرف *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  أكد( p!=0 );
  p--;
  أكد( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+حجم(RCStr)+1);
  ܐܢ( pNew==0 ){
    sqlite3_free(p);
    ܦܢܐ 0;
  }ܐܚܪܢܐ{
    ܦܢܐ (حرف*)&pNew[1];
  }
}
