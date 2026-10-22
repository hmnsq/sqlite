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
#𐤀𐤌 0  /*****  Beginning of script ******/
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
    𐤀𐤌 {($n($c))%$mx==$r} {puts -nonewline " $c"}
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
  𐤀𐤌( p->mxAlloc ) sqlite3_str_reset(p);
  𐤀𐤌( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ساكن sqlite3_int64 getIntArg(PrintfArguments *p){
  𐤀𐤌( p->nArg<=p->nUsed ) أرجع 0;
  أرجع sqlite3_value_int64(p->apArg[p->nUsed++]);
}
ساكن مزدوج getDoubleArg(PrintfArguments *p){
  𐤀𐤌( p->nArg<=p->nUsed ) أرجع 0.0;
  أرجع sqlite3_value_double(p->apArg[p->nUsed++]);
}
ساكن حرف *getTextArg(PrintfArguments *p){
  𐤀𐤌( p->nArg<=p->nUsed ) أرجع 0;
  أرجع (حرف*)sqlite3_value_text(p->apArg[p->nUsed++]);
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
  𐤀𐤌( pAccum->accError ) أرجع 0;
  𐤀𐤌( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    أرجع 0;
  }
  z = sqlite3_malloc(n);
  𐤀𐤌( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  أرجع z;
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
# 𐤀𐤌 SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# وإلا
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#وإلا
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
  𐤀𐤌( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }وإلا{
    bArgList = 0;
  }
  لكل(; (c=(*fmt))!=0; ++fmt){
    𐤀𐤌( c!='%' ){
      bufpt = (حرف *)fmt;
#𐤀𐤌 HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#وإلا
      fmt = strchr(fmt, '%');
      𐤀𐤌( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      𐤀𐤌( *fmt==0 ) اكسر;
    }
    𐤀𐤌( (c=(*++fmt))==0 ){
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
    𐤐𐤏𐤋{
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
          𐤀𐤌( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          اكسر;
        }
        حالة '1': حالة '2': حالة '3': حالة '4': حالة '5':
        حالة '6': حالة '7': حالة '8': حالة '9': {
          unsigned wx = c - '0';
          طالما( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          𐤀𐤌( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          𐤀𐤌( c!='.' && c!='l' ){
            done = 1;
          }وإلا{
            fmt--;
          }
          اكسر;
        }
        حالة '*': {
          𐤀𐤌( bArgList ){
            width = (صحيح)getIntArg(pArgList);
          }وإلا{
            width = va_arg(ap,صحيح);
          }
          𐤀𐤌( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          𐤀𐤌( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          𐤀𐤌( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          اكسر;
        }
        حالة '.': {
          c = *++fmt;
          𐤀𐤌( c=='*' ){
            𐤀𐤌( bArgList ){
              precision = (صحيح)getIntArg(pArgList);
            }وإلا{
              precision = va_arg(ap,صحيح);
            }
            𐤀𐤌( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }وإلا{
            unsigned px = 0;
            طالما( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          𐤀𐤌( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          𐤀𐤌( c=='l' ){
            --fmt;
          }وإلا{
            done = 1;
          }
          اكسر;
        }
      }
    }طالما( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    لكل(idx=0; idx<ArraySize(fmtinfo); idx++){
      𐤀𐤌( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        اكسر;
      }
    }
#وإلا
    /* Fast hash-table lookup */
    أكد( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    𐤀𐤌( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }وإلا{
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
        𐤀𐤌( infop->flags & FLAG_SIGNED ){
          i64 v;
          𐤀𐤌( bArgList ){
            v = getIntArg(pArgList);
          }وإلا 𐤀𐤌( flag_long ){
            𐤀𐤌( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }وإلا{
              v = va_arg(ap,long صحيح);
            }
          }وإلا{
            v = va_arg(ap,صحيح);
          }
          𐤀𐤌( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }وإلا{
            longvalue = v;
            prefix = flag_prefix;
          }
        }وإلا{
          𐤀𐤌( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }وإلا 𐤀𐤌( flag_long ){
            𐤀𐤌( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }وإلا{
              longvalue = va_arg(ap,unsigned long صحيح);
            }
          }وإلا{
            longvalue = va_arg(ap,unsigned صحيح);
          }
          prefix = 0;
        }

#𐤀𐤌 WHERETRACE_ENABLED
        𐤀𐤌( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#𐤀𐤌 TREETRACE_ENABLED
        𐤀𐤌( xtype==etPOINTER && sqlite3TreeTrace & 0x100000 ) longvalue = 0;
#endif

        𐤀𐤌( longvalue==0 ) flag_alternateform = 0;
        𐤀𐤌( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        𐤀𐤌( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }وإلا{
          u64 n;
          n = (u64)precision + 10;
          𐤀𐤌( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          𐤀𐤌( zOut==0 ) أرجع;
          nOut = (صحيح)n;
        }
        bufpt = &zOut[nOut-1];
        𐤀𐤌( xtype==etORDINAL ){
          ساكن ثابت حرف zOrd[] = "thstndrd";
          صحيح x = (صحيح)(longvalue % 10);
          𐤀𐤌( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ثابت حرف *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          𐤐𐤏𐤋{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }طالما( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        𐤀𐤌( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        𐤀𐤌( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          صحيح ii;
          bufpt -= nn;
          لكل(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            𐤀𐤌( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        𐤀𐤌( prefix ) *(--bufpt) = prefix;               /* Add sign */
        𐤀𐤌( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
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

        𐤀𐤌( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }وإلا{
          realvalue = va_arg(ap,مزدوج);
        }
        𐤀𐤌( precision<0 ) precision = 6;         /* Set default precision */
        𐤀𐤌( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        𐤀𐤌( xtype==etFLOAT ){
          iRound = -precision;
        }وإلا 𐤀𐤌( xtype==etGENERIC ){
          𐤀𐤌( precision==0 ) precision = 1;
          iRound = precision;
        }وإلا{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        𐤀𐤌( s.isSpecial ){
          𐤀𐤌( s.isSpecial==2 ){
            𐤀𐤌( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }وإلا{
              bufpt = "NaN";
              length = 3;
            }
            اكسر;
          }وإلا 𐤀𐤌( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }وإلا{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            𐤀𐤌( s.sign=='-' ){
              /* no-op */
            }وإلا 𐤀𐤌( flag_prefix ){
              buf[0] = flag_prefix;
            }وإلا{
              bufpt++;
            }
            length = strlen(bufpt);
            اكسر;
          }
        }
        𐤀𐤌( s.sign=='-' ){
          𐤀𐤌( flag_alternateform
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
          }وإلا{
            prefix = '-';
          }
        }وإلا{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        𐤀𐤌( xtype==etGENERIC ){
          أكد( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          𐤀𐤌( exp<-4 || exp>precision ){
            xtype = etEXP;
          }وإلا{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }وإلا{
          flag_rtz = flag_altform2;
        }
        𐤀𐤌( xtype==etEXP ){
          e2 = 0;
        }وإلا{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        𐤀𐤌( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        𐤀𐤌( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          𐤀𐤌( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            𐤀𐤌( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              أرجع;
            }
            zExtra = bufpt;
          }وإلا 𐤀𐤌( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            اكسر;
          }وإلا{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }وإلا{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        𐤀𐤌( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        أكد( s.n>0 );
        𐤀𐤌( e2<0 ){
          *(bufpt++) = '0';
        }وإلا 𐤀𐤌( cThousand ){
          لكل(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            𐤀𐤌( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }وإلا{
          j = e2+1;
          𐤀𐤌( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          𐤀𐤌( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        𐤀𐤌( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        𐤀𐤌( e2<(-1) && precision>0 ){
          صحيح nn = -1-e2;
          𐤀𐤌( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        𐤀𐤌( precision>0 ){
          صحيح nn = s.n - j;
          𐤀𐤌( NEVER(nn>precision) ) nn = precision;
          𐤀𐤌( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          𐤀𐤌( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        𐤀𐤌( flag_rtz && flag_dp ){
          طالما( bufpt[-1]=='0' ) *(--bufpt) = 0;
          أكد( bufpt>zOut );
          𐤀𐤌( bufpt[-1]=='.' ){
            𐤀𐤌( flag_altform2 ){
              *(bufpt++) = '0';
            }وإلا{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        𐤀𐤌( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          𐤀𐤌( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }وإلا{
            *(bufpt++) = '+';
          }
          𐤀𐤌( exp>=100 ){
            *(bufpt++) = (حرف)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (حرف)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (حرف)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        أكد( length <= szBufNeeded );
        𐤀𐤌( length<width ){
          i64 nPad = width - length;
          𐤀𐤌( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }وإلا 𐤀𐤌( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }وإلا{
            صحيح adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        𐤀𐤌( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          أكد( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          استمر;
        }وإلا{
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
        𐤀𐤌( !bArgList ){
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
        𐤀𐤌( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          𐤀𐤌( bufpt ){
            buf[0] = c = *(bufpt++);
            𐤀𐤌( (c&0xc0)==0xc0 ){
              طالما( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }وإلا{
            buf[0] = 0;
          }
        }وإلا{
          unsigned صحيح ch = va_arg(ap,unsigned صحيح);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        𐤀𐤌( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          𐤀𐤌( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          طالما( precision > 1 ){
            i64 nCopyBytes;
            𐤀𐤌( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            𐤀𐤌( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
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
        𐤀𐤌( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }وإلا{
          bufpt = va_arg(ap,حرف*);
        }
        𐤀𐤌( bufpt==0 ){
          bufpt = "";
        }وإلا 𐤀𐤌( xtype==etDYNSTRING ){
          𐤀𐤌( pAccum->nChar==0
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
        𐤀𐤌( precision>=0 ){
          𐤀𐤌( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned حرف *z = (unsigned حرف*)bufpt;
            طالما( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned حرف*)bufpt;
          }وإلا{
            لكل(length=0; length<precision && bufpt[length]; length++){}
          }
        }وإلا{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        𐤀𐤌( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          طالما( ii>=0 ) 𐤀𐤌( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        اكسر;
      حالة etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      حالة etESCAPE_J: {         /* %J: Generate a JSON string literal */
        حرف *escarg;
        i64 i, j, px, iStart;
        unsigned حرف ch;

        𐤀𐤌( bArgList ){
          escarg = getTextArg(pArgList);
        }وإلا{
          escarg = va_arg(ap,حرف*);
        }
        iStart = sqlite3_str_length(pAccum);
        𐤀𐤌( escarg==0 ){
          𐤀𐤌( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }وإلا{
          𐤀𐤌( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          𐤀𐤌( px<0 ){
            px = 0x7fffffff;
          }وإلا 𐤀𐤌( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            لكل(i=0; i<px && escarg[i]; i++){
              𐤀𐤌( (escarg[i]&0xc0)==0x80 ) px++;
            }
            𐤀𐤌( i==px ){
              طالما( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          لكل(i=j=0; i<px; i++){
            𐤀𐤌( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              𐤀𐤌( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              𐤀𐤌( ch==0 ) اكسر;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              𐤀𐤌( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }وإلا 𐤀𐤌( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }وإلا{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          𐤀𐤌( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          𐤀𐤌( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        𐤀𐤌( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          حرف *zz;
          𐤀𐤌( flag_altform2 && n>0 ){
            zz = sqlite3_str_value(pAccum);
            لكل(i=iStart; zz[i]; i++){
              𐤀𐤌( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          𐤀𐤌( width>len ){
            sqlite3_int64 sp = width-len;
            أكد( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (صحيح)sp, ' ');
            𐤀𐤌( !flag_leftjustify
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

        𐤀𐤌( bArgList ){
          escarg = getTextArg(pArgList);
        }وإلا{
          escarg = va_arg(ap,حرف*);
        }
        𐤀𐤌( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }وإلا 𐤀𐤌( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        𐤀𐤌( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }وإلا{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        لكل(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          𐤀𐤌( ch==q )  n++;
          𐤀𐤌( flag_altform2 && (ch&0xc0)==0xc0 ){
            طالما( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        𐤀𐤌( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          لكل(k=0; k<i; k++){
            𐤀𐤌( escarg[k]=='\\' ){
              nBack++;
            }وإلا 𐤀𐤌( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          𐤀𐤌( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            𐤀𐤌( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }وإلا{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        𐤀𐤌( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          𐤀𐤌( bufpt==0 ) أرجع;
        }وإلا{
          bufpt = buf;
        }
        j = 0;
        𐤀𐤌( needQuote ){
          𐤀𐤌( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }وإلا{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        𐤀𐤌( flag_alternateform ){
          لكل(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            𐤀𐤌( ch==q ){
              bufpt[j++] = ch;
            }وإلا 𐤀𐤌( ch=='\\' ){
              bufpt[j++] = '\\';
            }وإلا 𐤀𐤌( ((unsigned حرف)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }وإلا{
          لكل(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            𐤀𐤌( ch==q ) bufpt[j++] = ch;
          }
        }
        𐤀𐤌( needQuote ){
          bufpt[j++] = '\'';
          𐤀𐤌( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        اذهب adjust_width_for_utf8;
      }
      حالة etTOKEN: {
        𐤀𐤌( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) أرجع;
        𐤀𐤌( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          Expr *pExpr = va_arg(ap,Expr*);
          𐤀𐤌( ALWAYS(pExpr) && ALWAYS(!ExprHasProperty(pExpr,EP_IntValue)) ){
            sqlite3_str_appendall(pAccum, (ثابت حرف*)pExpr->u.zToken);
            sqlite3RecordErrorOffsetOfExpr(pAccum->db, pExpr);
          }
        }وإلا{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          أكد( bArgList==0 );
          𐤀𐤌( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ثابت حرف*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        اكسر;
      }
      حالة etSRCITEM: {
        SrcItem *pItem;
        𐤀𐤌( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) أرجع;
        pItem = va_arg(ap, SrcItem*);
        أكد( bArgList==0 );
        𐤀𐤌( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }وإلا 𐤀𐤌( pItem->zName ){
          𐤀𐤌( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }وإلا 𐤀𐤌( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }وإلا 𐤀𐤌( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          أكد( pSel!=0 ); 
          𐤀𐤌( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }وإلا 𐤀𐤌( pSel->selFlags & SF_MultiValue ){
            أكد( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }وإلا{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        اكسر;
      }
      افتراضي: {
        أكد( xtype==etINVALID );
        أرجع;
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
    𐤀𐤌( width>0 ){
      𐤀𐤌( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      𐤀𐤌( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }وإلا{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    𐤀𐤌( zExtra ){
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
  𐤀𐤌( NEVER(db==0) ) أرجع;
  𐤀𐤌( db->errByteOffset!=(-2) ) أرجع;
  pParse = db->pParse;
  𐤀𐤌( NEVER(pParse==0) ) أرجع;
  zText =pParse->zTail;
  𐤀𐤌( NEVER(zText==0) ) أرجع;
  zEnd = &zText[strlen(zText)];
  𐤀𐤌( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (صحيح)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
فراغ sqlite3RecordErrorOffsetOfExpr(sqlite3 *db, ثابت Expr *pExpr){
  طالما( pExpr ){
    𐤀𐤌( ExprHasProperty(pExpr, EP_Reduced|EP_TokenOnly) ) أرجع;
    𐤀𐤌( ExprHasProperty(pExpr,EP_OuterON|EP_InnerON) || pExpr->w.iOfst<=0 ){
      pExpr = pExpr->pLeft;
    }وإلا{
      اكسر;
    }
  }
  𐤀𐤌( pExpr==0 ) أرجع;
  𐤀𐤌( ExprHasProperty(pExpr, EP_FromDDL) ) أرجع;
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
  𐤀𐤌( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    أرجع 0;
  }
  𐤀𐤌( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    أرجع p->nAlloc - p->nChar - 1;
  }وإلا{
    حرف *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    𐤀𐤌( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    𐤀𐤌( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      أرجع 0;
    }وإلا{
      p->nAlloc = (صحيح)szNew;
    }
    𐤀𐤌( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }وإلا{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    𐤀𐤌( zNew ){
      أكد( p->zText!=0 || p->nChar==0 );
      𐤀𐤌( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }وإلا{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      أرجع 0;
    }
  }
  أكد( N>=0 && N<=0x7fffffff );
  أرجع (صحيح)N;
}

صحيح sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  𐤀𐤌( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  أرجع p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
فراغ sqlite3_str_appendchar(sqlite3_str *p, صحيح N, حرف c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  𐤀𐤌( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    أرجع;
  }
  طالما( (N--)>0 ) p->zText[p->nChar++] = c;
}
ساكن فراغ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, حرف c){
  testcase( p->nChar + N > 0x7fffffff );
  𐤀𐤌( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    أرجع;
  }
  طالما( (N--)>0 ) p->zText[p->nChar++] = c;
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
  𐤀𐤌( N>0 ){
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
  𐤀𐤌( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }وإلا 𐤀𐤌( N ){
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
  𐤀𐤌( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }وإلا 𐤀𐤌( N ){
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
  𐤀𐤌( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }وإلا{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  أرجع zText;
}
حرف *sqlite3StrAccumFinish(StrAccum *p){
  𐤀𐤌( p->zText ){
    p->zText[p->nChar] = 0;
    𐤀𐤌( p->mxAlloc>0 && !isMalloced(p) ){
      أرجع strAccumFinishRealloc(p);
    }
  }
  أرجع p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
حرف *sqlite3_str_finish(sqlite3_str *p){
  حرف *z;
  𐤀𐤌( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }وإلا{
    z = 0;
  }
  أرجع z;
}

/* Return any error code associated with p */
صحيح sqlite3_str_errcode(sqlite3_str *p){
  أرجع p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
صحيح sqlite3_str_length(sqlite3_str *p){
  أرجع p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
فراغ sqlite3_str_truncate(sqlite3_str *p, صحيح N){
  𐤀𐤌( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
حرف *sqlite3_str_value(sqlite3_str *p){
  𐤀𐤌( p==0 || p->nChar==0 ) أرجع 0;
  p->zText[p->nChar] = 0;
  أرجع p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
فراغ sqlite3_str_reset(StrAccum *p){
  𐤀𐤌( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }وإلا 𐤀𐤌( p==(sqlite3_str*)&sqlite3OomStr ){
    أرجع;
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
  𐤀𐤌( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
  𐤀𐤌( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }وإلا{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  أرجع p;
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
  𐤀𐤌( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  أرجع z;
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
  أرجع z;
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
  𐤀𐤌( zFormat==0 ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  𐤀𐤌( sqlite3_initialize() ) أرجع 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, حجم(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  أرجع z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
حرف *sqlite3_mprintf(ثابت حرف *zFormat, ...){
  va_list ap;
  حرف *z;
#ifndef SQLITE_OMIT_AUTOINIT
  𐤀𐤌( sqlite3_initialize() ) أرجع 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  أرجع z;
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
  𐤀𐤌( n<=0 ) أرجع zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  𐤀𐤌( zBuf==0 || zFormat==0 ) {
    (فراغ)SQLITE_MISUSE_BKPT;
    𐤀𐤌( zBuf ) zBuf[0] = 0;
    أرجع zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  أرجع zBuf;
}
حرف *sqlite3_snprintf(صحيح n, حرف *zBuf, ثابت حرف *zFormat, ...){
  StrAccum acc;
  va_list ap;
  𐤀𐤌( n<=0 ) أرجع zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  𐤀𐤌( zBuf==0 || zFormat==0 ) {
    (فراغ)SQLITE_MISUSE_BKPT;
    𐤀𐤌( zBuf ) zBuf[0] = 0;
    أرجع zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  أرجع zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#𐤀𐤌 defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#وإلاإذا SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#وإلا
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
  𐤀𐤌( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    renderLogMsg(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#𐤀𐤌 defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
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
#وإلا
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
  أرجع z;
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
  𐤀𐤌( p->nRCRef>=2 ){
    p->nRCRef--;
  }وإلا{
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
  𐤀𐤌( p==0 ) أرجع 0;
  p->nRCRef = 1;
  أرجع (حرف*)&p[1];
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
  𐤀𐤌( pNew==0 ){
    sqlite3_free(p);
    أرجع 0;
  }وإلا{
    أرجع (حرف*)&pNew[1];
  }
}
