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
#অন্তর্ভাব "sqliteInt.h"

/*
** Conversion types fall into various categories as defined by the
** following enumeration.
*/
#define etRADIX       0 /* non-decimal integer types.  %x %o */
#define etFLOAT       1 /* Floating point.  %f */
#define etEXP         2 /* Exponentional notation. %e and %E */
#define etGENERIC     3 /* Floating or exponential, depending on exponent. %g */
#define et_আকার        4 /* Return number of characters processed so far. %n */
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
প্রকারলেখ unsigned অক্ষর etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
প্রকারলেখ কাঠামো et_info {   /* Information about each format field */
  অক্ষর fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  অক্ষর iNxt;               /* Next with same hash, or 0 for end of chain */
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
#যদি 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
জন্য {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    যদি {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

স্থির ধ্রুবক অক্ষর aDigits[] = "0123456789ABCDEF0123456789abcdef";
স্থির ধ্রুবক অক্ষর aHex[]    = "0123456789abcdef";
স্থির ধ্রুবক অক্ষর aPrefix[] = "-x0\000X0";
স্থির ধ্রুবক et_info fmtinfo[25] = {
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
  /* 10 */  {  'n',  0, 0, et_আকার,       0,  0,  0 },
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
শূন্য sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  প্রতিজ্ঞা( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  যদি( p->mxAlloc ) sqlite3_str_reset(p);
  যদি( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
স্থির sqlite3_int64 getIntArg(PrintfArguments *p){
  যদি( p->nArg<=p->nUsed ) ফেরত 0;
  ফেরত sqlite3_মান_int64(p->apArg[p->nUsed++]);
}
স্থির দ্বিগুণ getDoubleArg(PrintfArguments *p){
  যদি( p->nArg<=p->nUsed ) ফেরত 0.0;
  ফেরত sqlite3_মান_double(p->apArg[p->nUsed++]);
}
স্থির অক্ষর *getTextArg(PrintfArguments *p){
  যদি( p->nArg<=p->nUsed ) ফেরত 0;
  ফেরত (অক্ষর*)sqlite3_মান_text(p->apArg[p->nUsed++]);
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
স্থির অক্ষর *printf_অস্থায়ী_Buf(sqlite3_str *pAccum, sqlite3_int64 n){
  অক্ষর *z;
  যদি( pAccum->accError ) ফেরত 0;
  যদি( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    ফেরত 0;
  }
  z = sqlite3_malloc(n);
  যদি( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  ফেরত z;
}

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be something smaller, if desired.
*/
#ifndef SQLITE_PRINT_BUF_আকার
# define SQLITE_PRINT_BUF_আকার 70
#endif
#define etBUFSIZE SQLITE_PRINT_BUF_আকার  /* Size of the output buffer */

/*
** Hard limit on the precision of floating-point conversions to
** the minimum of SQLITE_PRINTF_PRECISION_LIMIT and 100 million.
*/
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
# যদি SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# অন্যথা
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#অন্যথা
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
স্থির শূন্য sqlite3StrAppend64(sqlite3_str *p, ধ্রুবক অক্ষর *z, i64 N);
স্থির শূন্য sqlite3StrAppendchar64(sqlite3_str *p, i64 N, অক্ষর c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
শূন্য sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ধ্রুবক অক্ষর *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  পূর্ণসংখ্যা c;                     /* Next character in the format string */
  অক্ষর *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 দৈর্ঘ্য;                /* Length of the field */
  পূর্ণসংখ্যা idx;                   /* A general purpose loop counter */
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
  অক্ষর prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  দ্বিগুণ realvalue;          /* Value for real types */
  ধ্রুবক et_info *infop;      /* Pointer to the appropriate info structure */
  অক্ষর *zOut;                /* Rendering buffer */
  পূর্ণসংখ্যা nOut;                  /* Size of the rendering buffer */
  অক্ষর *zExtra = 0;          /* Malloced memory used by some conversion */
  পূর্ণসংখ্যা exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  অক্ষর buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  প্রতিজ্ঞা( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  যদি( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }অন্যথা{
    bArgList = 0;
  }
  জন্য(; (c=(*fmt))!=0; ++fmt){
    যদি( c!='%' ){
      bufpt = (অক্ষর *)fmt;
#যদি HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#অন্যথা
      fmt = strchr(fmt, '%');
      যদি( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      যদি( *fmt==0 ) বিরতি;
    }
    যদি( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      বিরতি;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    করো{
      নির্বাচন( c ){
        ক্ষেত্র '-':   flag_leftjustify = 1;     বিরতি;
        ক্ষেত্র '+':   flag_prefix = '+';        বিরতি;
        ক্ষেত্র ' ':   flag_prefix = ' ';        বিরতি;
        ক্ষেত্র '#':   flag_alternateform = 1;   বিরতি;
        ক্ষেত্র '!':   flag_altform2 = 1;        বিরতি;
        ক্ষেত্র '0':   flag_zeropad = 1;         বিরতি;
        ক্ষেত্র ',':   cThousand = ',';          বিরতি;
        পূর্বনির্ধারিত:    done = 1;                 বিরতি;
        ক্ষেত্র 'l': {
          flag_long = 1;
          c = *++fmt;
          যদি( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          বিরতি;
        }
        ক্ষেত্র '1': ক্ষেত্র '2': ক্ষেত্র '3': ক্ষেত্র '4': ক্ষেত্র '5':
        ক্ষেত্র '6': ক্ষেত্র '7': ক্ষেত্র '8': ক্ষেত্র '9': {
          unsigned wx = c - '0';
          যখন( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          যদি( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          যদি( c!='.' && c!='l' ){
            done = 1;
          }অন্যথা{
            fmt--;
          }
          বিরতি;
        }
        ক্ষেত্র '*': {
          যদি( bArgList ){
            width = (পূর্ণসংখ্যা)getIntArg(pArgList);
          }অন্যথা{
            width = va_arg(ap,পূর্ণসংখ্যা);
          }
          যদি( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          যদি( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          যদি( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          বিরতি;
        }
        ক্ষেত্র '.': {
          c = *++fmt;
          যদি( c=='*' ){
            যদি( bArgList ){
              precision = (পূর্ণসংখ্যা)getIntArg(pArgList);
            }অন্যথা{
              precision = va_arg(ap,পূর্ণসংখ্যা);
            }
            যদি( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }অন্যথা{
            unsigned px = 0;
            যখন( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          যদি( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          যদি( c=='l' ){
            --fmt;
          }অন্যথা{
            done = 1;
          }
          বিরতি;
        }
      }
    }যখন( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    জন্য(idx=0; idx<Array_আকার(fmtinfo); idx++){
      যদি( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        বিরতি;
      }
    }
#অন্যথা
    /* Fast hash-table lookup */
    প্রতিজ্ঞা( Array_আকার(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    যদি( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }অন্যথা{
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
    প্রতিজ্ঞা( width>=0 );
    প্রতিজ্ঞা( precision>=(-1) );
    নির্বাচন( xtype ){
      ক্ষেত্র etPOINTER:
        flag_long = আকার(অক্ষর*)==আকার(i64) ? 2 :
                     আকার(অক্ষর*)==আকার(long পূর্ণসংখ্যা) ? 1 : 0;
        /* no break */ deliberate_fall_through
      ক্ষেত্র etORDINAL:
      ক্ষেত্র etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      ক্ষেত্র etDECIMAL:
        যদি( infop->flags & FLAG_SIGNED ){
          i64 v;
          যদি( bArgList ){
            v = getIntArg(pArgList);
          }অন্যথা যদি( flag_long ){
            যদি( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }অন্যথা{
              v = va_arg(ap,long পূর্ণসংখ্যা);
            }
          }অন্যথা{
            v = va_arg(ap,পূর্ণসংখ্যা);
          }
          যদি( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }অন্যথা{
            longvalue = v;
            prefix = flag_prefix;
          }
        }অন্যথা{
          যদি( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }অন্যথা যদি( flag_long ){
            যদি( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }অন্যথা{
              longvalue = va_arg(ap,unsigned long পূর্ণসংখ্যা);
            }
          }অন্যথা{
            longvalue = va_arg(ap,unsigned পূর্ণসংখ্যা);
          }
          prefix = 0;
        }

#যদি WHERETRACE_ENABLED
        যদি( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#যদি TREETRACE_ENABLED
        যদি( xtype==etPOINTER && sqlite3_বৃক্ষ_Trace & 0x100000 ) longvalue = 0;
#endif

        যদি( longvalue==0 ) flag_alternateform = 0;
        যদি( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        যদি( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }অন্যথা{
          u64 n;
          n = (u64)precision + 10;
          যদি( cThousand ) n += precision/3;
          zOut = zExtra = printf_অস্থায়ী_Buf(pAccum, n);
          যদি( zOut==0 ) ফেরত;
          nOut = (পূর্ণসংখ্যা)n;
        }
        bufpt = &zOut[nOut-1];
        যদি( xtype==etORDINAL ){
          স্থির ধ্রুবক অক্ষর zOrd[] = "thstndrd";
          পূর্ণসংখ্যা x = (পূর্ণসংখ্যা)(longvalue % 10);
          যদি( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ধ্রুবক অক্ষর *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          করো{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }যখন( longvalue>0 );
        }
        দৈর্ঘ্য = &zOut[nOut-1] - bufpt;
        যদি( precision>দৈর্ঘ্য ){                         /* zero pad */
          i64 nn = precision-দৈর্ঘ্য;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          দৈর্ঘ্য = precision;
        }
        যদি( cThousand ){
          i64 nn = (দৈর্ঘ্য - 1)/3;  /* Number of "," to insert */
          i64 ix = (দৈর্ঘ্য - 1)%3 + 1;
          পূর্ণসংখ্যা ii;
          bufpt -= nn;
          জন্য(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            যদি( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        যদি( prefix ) *(--bufpt) = prefix;               /* Add sign */
        যদি( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          ধ্রুবক অক্ষর *pre;
          অক্ষর x;
          pre = &aPrefix[infop->prefix];
          জন্য(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        দৈর্ঘ্য = &zOut[nOut-1] - bufpt;
        বিরতি;
      ক্ষেত্র etFLOAT:
      ক্ষেত্র etEXP:
      ক্ষেত্র etGENERIC: {
        FpDecode s;
        পূর্ণসংখ্যা iRound;
        পূর্ণসংখ্যা j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        যদি( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }অন্যথা{
          realvalue = va_arg(ap,দ্বিগুণ);
        }
        যদি( precision<0 ) precision = 6;         /* Set default precision */
        যদি( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        যদি( xtype==etFLOAT ){
          iRound = -precision;
        }অন্যথা যদি( xtype==etGENERIC ){
          যদি( precision==0 ) precision = 1;
          iRound = precision;
        }অন্যথা{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        যদি( s.isSpecial ){
          যদি( s.isSpecial==2 ){
            যদি( flag_zeropad ){
              bufpt = "null";
              দৈর্ঘ্য = 4;
            }অন্যথা{
              bufpt = "NaN";
              দৈর্ঘ্য = 3;
            }
            বিরতি;
          }অন্যথা যদি( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }অন্যথা{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            যদি( s.sign=='-' ){
              /* no-op */
            }অন্যথা যদি( flag_prefix ){
              buf[0] = flag_prefix;
            }অন্যথা{
              bufpt++;
            }
            দৈর্ঘ্য = strlen(bufpt);
            বিরতি;
          }
        }
        যদি( s.sign=='-' ){
          যদি( flag_alternateform
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
          }অন্যথা{
            prefix = '-';
          }
        }অন্যথা{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        যদি( xtype==etGENERIC ){
          প্রতিজ্ঞা( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          যদি( exp<-4 || exp>precision ){
            xtype = etEXP;
          }অন্যথা{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }অন্যথা{
          flag_rtz = flag_altform2;
        }
        যদি( xtype==etEXP ){
          e2 = 0;
        }অন্যথা{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        যদি( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        যদি( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          যদি( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            যদি( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              ফেরত;
            }
            zExtra = bufpt;
          }অন্যথা যদি( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = দৈর্ঘ্য = 0;
            বিরতি;
          }অন্যথা{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }অন্যথা{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        যদি( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        প্রতিজ্ঞা( s.n>0 );
        যদি( e2<0 ){
          *(bufpt++) = '0';
        }অন্যথা যদি( cThousand ){
          জন্য(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            যদি( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }অন্যথা{
          j = e2+1;
          যদি( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          যদি( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        যদি( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        যদি( e2<(-1) && precision>0 ){
          পূর্ণসংখ্যা nn = -1-e2;
          যদি( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        যদি( precision>0 ){
          পূর্ণসংখ্যা nn = s.n - j;
          যদি( NEVER(nn>precision) ) nn = precision;
          যদি( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          যদি( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        যদি( flag_rtz && flag_dp ){
          যখন( bufpt[-1]=='0' ) *(--bufpt) = 0;
          প্রতিজ্ঞা( bufpt>zOut );
          যদি( bufpt[-1]=='.' ){
            যদি( flag_altform2 ){
              *(bufpt++) = '0';
            }অন্যথা{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        যদি( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          যদি( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }অন্যথা{
            *(bufpt++) = '+';
          }
          যদি( exp>=100 ){
            *(bufpt++) = (অক্ষর)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (অক্ষর)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (অক্ষর)(exp%10+'0');             /* 1's digit */
        }

        দৈর্ঘ্য = bufpt - zOut;
        প্রতিজ্ঞা( দৈর্ঘ্য <= szBufNeeded );
        যদি( দৈর্ঘ্য<width ){
          i64 nPad = width - দৈর্ঘ্য;
          যদি( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }অন্যথা যদি( !flag_zeropad ){
            memmove(zOut+nPad, zOut, দৈর্ঘ্য);
            memset(zOut, ' ', nPad);
          }অন্যথা{
            পূর্ণসংখ্যা adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, দৈর্ঘ্য-adj);
            memset(zOut+adj, '0', nPad);
          }
          দৈর্ঘ্য = width;
        }

        যদি( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          প্রতিজ্ঞা( pAccum->nChar + দৈর্ঘ্য < SMXV(pAccum->nChar) );
          pAccum->nChar += দৈর্ঘ্য;
          zOut[দৈর্ঘ্য] = 0;
          চালু;
        }অন্যথা{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          বিরতি;
        }
      }
      ক্ষেত্র et_আকার:
        যদি( !bArgList ){
          *(va_arg(ap,পূর্ণসংখ্যা*)) = pAccum->nChar;
        }
        দৈর্ঘ্য = width = 0;
        বিরতি;
      ক্ষেত্র etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        দৈর্ঘ্য = 1;
        বিরতি;
      ক্ষেত্র etCHARX:
        যদি( bArgList ){
          bufpt = getTextArg(pArgList);
          দৈর্ঘ্য = 1;
          যদি( bufpt ){
            buf[0] = c = *(bufpt++);
            যদি( (c&0xc0)==0xc0 ){
              যখন( দৈর্ঘ্য<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[দৈর্ঘ্য++] = *(bufpt++);
              }
            }
          }অন্যথা{
            buf[0] = 0;
          }
        }অন্যথা{
          unsigned পূর্ণসংখ্যা ch = va_arg(ap,unsigned পূর্ণসংখ্যা);
          দৈর্ঘ্য = sqlite3AppendOneUtf8Character(buf, ch);
        }
        যদি( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          যদি( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, দৈর্ঘ্য);
          precision--;
          যখন( precision > 1 ){
            i64 nCopyBytes;
            যদি( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = দৈর্ঘ্য*nPrior;
            যদি( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              বিরতি;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        যাও adjust_width_for_utf8;
      ক্ষেত্র etSTRING:
      ক্ষেত্র etDYNSTRING:
        যদি( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }অন্যথা{
          bufpt = va_arg(ap,অক্ষর*);
        }
        যদি( bufpt==0 ){
          bufpt = "";
        }অন্যথা যদি( xtype==etDYNSTRING ){
          যদি( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            প্রতিজ্ঞা( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3_Db_Malloc_আকার(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (পূর্ণসংখ্যা)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            দৈর্ঘ্য = 0;
            বিরতি;
          }
          zExtra = bufpt;
        }
        যদি( precision>=0 ){
          যদি( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned অক্ষর *z = (unsigned অক্ষর*)bufpt;
            যখন( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            দৈর্ঘ্য = z - (unsigned অক্ষর*)bufpt;
          }অন্যথা{
            জন্য(দৈর্ঘ্য=0; দৈর্ঘ্য<precision && bufpt[দৈর্ঘ্য]; দৈর্ঘ্য++){}
          }
        }অন্যথা{
          দৈর্ঘ্য = strlen(bufpt);
        }
      adjust_width_for_utf8:
        যদি( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = দৈর্ঘ্য - 1;
          যখন( ii>=0 ) যদি( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        বিরতি;
      ক্ষেত্র etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      ক্ষেত্র etESCAPE_J: {         /* %J: Generate a JSON string literal */
        অক্ষর *escarg;
        i64 i, j, px, iStart;
        unsigned অক্ষর ch;

        যদি( bArgList ){
          escarg = getTextArg(pArgList);
        }অন্যথা{
          escarg = va_arg(ap,অক্ষর*);
        }
        iStart = sqlite3_str_দৈর্ঘ্য(pAccum);
        যদি( escarg==0 ){
          যদি( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }অন্যথা{
          যদি( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          যদি( px<0 ){
            px = 0x7fffffff;
          }অন্যথা যদি( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            জন্য(i=0; i<px && escarg[i]; i++){
              যদি( (escarg[i]&0xc0)==0x80 ) px++;
            }
            যদি( i==px ){
              যখন( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          জন্য(i=j=0; i<px; i++){
            যদি( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              যদি( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              যদি( ch==0 ) বিরতি;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              যদি( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }অন্যথা যদি( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }অন্যথা{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          যদি( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          যদি( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        যদি( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_দৈর্ঘ্য(pAccum) - iStart;
          sqlite3_int64 len = n;
          অক্ষর *zz;
          যদি( flag_altform2 && n>0 ){
            zz = sqlite3_str_মান(pAccum);
            জন্য(i=iStart; zz[i]; i++){
              যদি( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          যদি( width>len ){
            sqlite3_int64 sp = width-len;
            প্রতিজ্ঞা( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (পূর্ণসংখ্যা)sp, ' ');
            যদি( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_মান(pAccum);
              zz += iStart;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        চালু;
      }
      ক্ষেত্র etESCAPE_q:          /* %q: Escape ' characters */
      ক্ষেত্র etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      ক্ষেত্র etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        পূর্ণসংখ্যা needQuote = 0;
        অক্ষর ch;
        অক্ষর *escarg;
        অক্ষর q;

        যদি( bArgList ){
          escarg = getTextArg(pArgList);
        }অন্যথা{
          escarg = va_arg(ap,অক্ষর*);
        }
        যদি( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }অন্যথা যদি( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        যদি( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }অন্যথা{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        জন্য(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          যদি( ch==q )  n++;
          যদি( flag_altform2 && (ch&0xc0)==0xc0 ){
            যখন( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        যদি( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          জন্য(k=0; k<i; k++){
            যদি( escarg[k]=='\\' ){
              nBack++;
            }অন্যথা যদি( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          যদি( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            যদি( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }অন্যথা{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        যদি( n>etBUFSIZE ){
          bufpt = zExtra = printf_অস্থায়ী_Buf(pAccum, n);
          যদি( bufpt==0 ) ফেরত;
        }অন্যথা{
          bufpt = buf;
        }
        j = 0;
        যদি( needQuote ){
          যদি( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }অন্যথা{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        যদি( flag_alternateform ){
          জন্য(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            যদি( ch==q ){
              bufpt[j++] = ch;
            }অন্যথা যদি( ch=='\\' ){
              bufpt[j++] = '\\';
            }অন্যথা যদি( ((unsigned অক্ষর)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }অন্যথা{
          জন্য(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            যদি( ch==q ) bufpt[j++] = ch;
          }
        }
        যদি( needQuote ){
          bufpt[j++] = '\'';
          যদি( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        দৈর্ঘ্য = j;
        যাও adjust_width_for_utf8;
      }
      ক্ষেত্র etTOKEN: {
        যদি( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ফেরত;
        যদি( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          রাশিমালা *p_রাশিমালা = va_arg(ap,রাশিমালা*);
          যদি( ALWAYS(p_রাশিমালা) && ALWAYS(!রাশিমালা_Has_Property(p_রাশিমালা,EP_Int_মান)) ){
            sqlite3_str_appendall(pAccum, (ধ্রুবক অক্ষর*)p_রাশিমালা->u.zToken);
            sqlite3_Record_Error_Offset_Of_রাশিমালা(pAccum->db, p_রাশিমালা);
          }
        }অন্যথা{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          প্রতিজ্ঞা( bArgList==0 );
          যদি( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ধ্রুবক অক্ষর*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        দৈর্ঘ্য = width = 0;
        বিরতি;
      }
      ক্ষেত্র etSRCITEM: {
        SrcItem *pItem;
        যদি( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ফেরত;
        pItem = va_arg(ap, SrcItem*);
        প্রতিজ্ঞা( bArgList==0 );
        যদি( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }অন্যথা যদি( pItem->zName ){
          যদি( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }অন্যথা যদি( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }অন্যথা যদি( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          প্রতিজ্ঞা( pSel!=0 ); 
          যদি( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }অন্যথা যদি( pSel->selFlags & SF_Multi_মান ){
            প্রতিজ্ঞা( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }অন্যথা{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        দৈর্ঘ্য = width = 0;
        বিরতি;
      }
      পূর্বনির্ধারিত: {
        প্রতিজ্ঞা( xtype==etINVALID );
        ফেরত;
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
    width -= দৈর্ঘ্য;
    যদি( width>0 ){
      যদি( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, দৈর্ঘ্য);
      যদি( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }অন্যথা{
      sqlite3StrAppend64(pAccum, bufpt, দৈর্ঘ্য);
    }

    যদি( zExtra ){
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
শূন্য sqlite3RecordErrorByteOffset(sqlite3 *db, ধ্রুবক অক্ষর *z){
  ধ্রুবক Parse *pParse;
  ধ্রুবক অক্ষর *zText;
  ধ্রুবক অক্ষর *zEnd;
  প্রতিজ্ঞা( z!=0 );
  যদি( NEVER(db==0) ) ফেরত;
  যদি( db->errByteOffset!=(-2) ) ফেরত;
  pParse = db->pParse;
  যদি( NEVER(pParse==0) ) ফেরত;
  zText =pParse->z_লেজ;
  যদি( NEVER(zText==0) ) ফেরত;
  zEnd = &zText[strlen(zText)];
  যদি( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (পূর্ণসংখ্যা)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
শূন্য sqlite3_Record_Error_Offset_Of_রাশিমালা(sqlite3 *db, ধ্রুবক রাশিমালা *p_রাশিমালা){
  যখন( p_রাশিমালা ){
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Reduced|EP_TokenOnly) ) ফেরত;
    যদি( রাশিমালা_Has_Property(p_রাশিমালা,EP_OuterON|EP_InnerON) || p_রাশিমালা->w.iOfst<=0 ){
      p_রাশিমালা = p_রাশিমালা->p_বাম;
    }অন্যথা{
      বিরতি;
    }
  }
  যদি( p_রাশিমালা==0 ) ফেরত;
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_FromDDL) ) ফেরত;
  db->errByteOffset = p_রাশিমালা->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
পূর্ণসংখ্যা sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  অক্ষর *zNew;
  প্রতিজ্ঞা( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  যদি( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    ফেরত 0;
  }
  যদি( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    ফেরত p->nAlloc - p->nChar - 1;
  }অন্যথা{
    অক্ষর *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    যদি( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    যদি( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      ফেরত 0;
    }অন্যথা{
      p->nAlloc = (পূর্ণসংখ্যা)szNew;
    }
    যদি( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }অন্যথা{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    যদি( zNew ){
      প্রতিজ্ঞা( p->zText!=0 || p->nChar==0 );
      যদি( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3_Db_Malloc_আকার(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }অন্যথা{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      ফেরত 0;
    }
  }
  প্রতিজ্ঞা( N>=0 && N<=0x7fffffff );
  ফেরত (পূর্ণসংখ্যা)N;
}

পূর্ণসংখ্যা sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  যদি( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  ফেরত p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
শূন্য sqlite3_str_appendchar(sqlite3_str *p, পূর্ণসংখ্যা N, অক্ষর c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  যদি( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ফেরত;
  }
  যখন( (N--)>0 ) p->zText[p->nChar++] = c;
}
স্থির শূন্য sqlite3StrAppendchar64(sqlite3_str *p, i64 N, অক্ষর c){
  testcase( p->nChar + N > 0x7fffffff );
  যদি( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ফেরত;
  }
  যখন( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
স্থির শূন্য SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ধ্রুবক অক্ষর *z, পূর্ণসংখ্যা N){
  N = sqlite3StrAccumEnlarge(p, N);
  প্রতিজ্ঞা( z!=0 || N==0 );
  যদি( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
শূন্য sqlite3_str_append(sqlite3_str *p, ধ্রুবক অক্ষর *z, পূর্ণসংখ্যা N){
  প্রতিজ্ঞা( z!=0 || N==0 );
  প্রতিজ্ঞা( p->zText!=0 || p->nChar==0 || p->accError );
  প্রতিজ্ঞা( N>=0 );
  প্রতিজ্ঞা( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  যদি( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }অন্যথা যদি( N ){
    প্রতিজ্ঞা( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
স্থির শূন্য sqlite3StrAppend64(sqlite3_str *p, ধ্রুবক অক্ষর *z, i64 N){
  প্রতিজ্ঞা( z!=0 || N==0 );
  প্রতিজ্ঞা( p->zText!=0 || p->nChar==0 || p->accError );
  প্রতিজ্ঞা( N>=0 );
  প্রতিজ্ঞা( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  যদি( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }অন্যথা যদি( N ){
    প্রতিজ্ঞা( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
শূন্য sqlite3_str_appendall(sqlite3_str *p, ধ্রুবক অক্ষর *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
স্থির SQLITE_NOINLINE অক্ষর *strAccumFinishRealloc(StrAccum *p){
  অক্ষর *zText;
  প্রতিজ্ঞা( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  যদি( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }অন্যথা{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  ফেরত zText;
}
অক্ষর *sqlite3StrAccumFinish(StrAccum *p){
  যদি( p->zText ){
    p->zText[p->nChar] = 0;
    যদি( p->mxAlloc>0 && !isMalloced(p) ){
      ফেরত strAccumFinishRealloc(p);
    }
  }
  ফেরত p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
অক্ষর *sqlite3_str_finish(sqlite3_str *p){
  অক্ষর *z;
  যদি( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }অন্যথা{
    z = 0;
  }
  ফেরত z;
}

/* Return any error code associated with p */
পূর্ণসংখ্যা sqlite3_str_errcode(sqlite3_str *p){
  ফেরত p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
পূর্ণসংখ্যা sqlite3_str_দৈর্ঘ্য(sqlite3_str *p){
  ফেরত p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
শূন্য sqlite3_str_truncate(sqlite3_str *p, পূর্ণসংখ্যা N){
  যদি( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
অক্ষর *sqlite3_str_মান(sqlite3_str *p){
  যদি( p==0 || p->nChar==0 ) ফেরত 0;
  p->zText[p->nChar] = 0;
  ফেরত p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
শূন্য sqlite3_str_reset(StrAccum *p){
  যদি( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }অন্যথা যদি( p==(sqlite3_str*)&sqlite3OomStr ){
    ফেরত;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
শূন্য sqlite3_str_free(sqlite3_str *p){
  যদি( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
শূন্য sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, অক্ষর *zBase, পূর্ণসংখ্যা n, পূর্ণসংখ্যা mx){
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
  sqlite3_str *p = sqlite3_malloc64(আকার(*p));
  যদি( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_দৈর্ঘ্য] : SQLITE_MAX_দৈর্ঘ্য);
  }অন্যথা{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  ফেরত p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
অক্ষর *sqlite3VMPrintf(sqlite3 *db, ধ্রুবক অক্ষর *zFormat, va_list ap){
  অক্ষর *z;
  অক্ষর zBase[SQLITE_PRINT_BUF_আকার];
  StrAccum acc;
  প্রতিজ্ঞা( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, আকার(zBase),
                      db->aLimit[SQLITE_LIMIT_দৈর্ঘ্য]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  যদি( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ফেরত z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
অক্ষর *sqlite3MPrintf(sqlite3 *db, ধ্রুবক অক্ষর *zFormat, ...){
  va_list ap;
  অক্ষর *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ফেরত z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
অক্ষর *sqlite3_vmprintf(ধ্রুবক অক্ষর *zFormat, va_list ap){
  অক্ষর *z;
  অক্ষর zBase[SQLITE_PRINT_BUF_আকার];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  যদি( zFormat==0 ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ফেরত 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, আকার(zBase), SQLITE_MAX_দৈর্ঘ্য);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ফেরত z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
অক্ষর *sqlite3_mprintf(ধ্রুবক অক্ষর *zFormat, ...){
  va_list ap;
  অক্ষর *z;
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ফেরত 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  ফেরত z;
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
অক্ষর *sqlite3_vsnprintf(পূর্ণসংখ্যা n, অক্ষর *zBuf, ধ্রুবক অক্ষর *zFormat, va_list ap){
  StrAccum acc;
  যদি( n<=0 ) ফেরত zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( zBuf==0 || zFormat==0 ) {
    (শূন্য)SQLITE_MISUSE_BKPT;
    যদি( zBuf ) zBuf[0] = 0;
    ফেরত zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  ফেরত zBuf;
}
অক্ষর *sqlite3_snprintf(পূর্ণসংখ্যা n, অক্ষর *zBuf, ধ্রুবক অক্ষর *zFormat, ...){
  StrAccum acc;
  va_list ap;
  যদি( n<=0 ) ফেরত zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( zBuf==0 || zFormat==0 ) {
    (শূন্য)SQLITE_MISUSE_BKPT;
    যদি( zBuf ) zBuf[0] = 0;
    ফেরত zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  ফেরত zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#যদি defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#নতুবা_যদি SQLITE_PRINT_BUF_আকার*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#অন্যথা
# define SQLITE_MAX_LOG_MESSAGE (SQLITE_PRINT_BUF_আকার*10)
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
স্থির শূন্য render_Log_বার্তা(পূর্ণসংখ্যা iErrCode, ধ্রুবক অক্ষর *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  অক্ষর z_বার্তা[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_বার্তা, আকার(z_বার্তা), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
শূন্য sqlite3_log(পূর্ণসংখ্যা iErrCode, ধ্রুবক অক্ষর *zFormat, ...){
  va_list ap;                             /* Vararg list */
  যদি( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_বার্তা(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
শূন্য sqlite3DebugPrintf(ধ্রুবক অক্ষর *zFormat, ...){
  va_list ap;
  StrAccum acc;
  অক্ষর zBuf[SQLITE_PRINT_BUF_আকার*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, আকার(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    বাহ্যিক শূন্য SQLITE_OS_TRACE_PROC(ধ্রুবক অক্ষর *zBuf, পূর্ণসংখ্যা nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, আকার(zBuf));
  }
#অন্যথা
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
শূন্য sqlite3_str_appendf(StrAccum *p, ধ্রুবক অক্ষর *zFormat, ...){
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
অক্ষর *sqlite3RCStrRef(অক্ষর *z){
  RCStr *p = (RCStr*)z;
  প্রতিজ্ঞা( p!=0 );
  p--;
  p->nRCRef++;
  ফেরত z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
শূন্য sqlite3RCStrUnref(শূন্য *z){
  RCStr *p = (RCStr*)z;
  প্রতিজ্ঞা( p!=0 );
  p--;
  প্রতিজ্ঞা( p->nRCRef>0 );
  যদি( p->nRCRef>=2 ){
    p->nRCRef--;
  }অন্যথা{
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
অক্ষর *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + আকার(*p) + 1 );
  যদি( p==0 ) ফেরত 0;
  p->nRCRef = 1;
  ফেরত (অক্ষর*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
অক্ষর *sqlite3RCStrResize(অক্ষর *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  প্রতিজ্ঞা( p!=0 );
  p--;
  প্রতিজ্ঞা( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+আকার(RCStr)+1);
  যদি( pNew==0 ){
    sqlite3_free(p);
    ফেরত 0;
  }অন্যথা{
    ফেরত (অক্ষর*)&pNew[1];
  }
}
