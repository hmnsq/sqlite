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
#include "sqliteInt.h"

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
typedef unsigned var_na etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
typedef sa_nracanaa et_info {   /* Information about each format field */
  var_na fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  var_na iNxt;               /* Next with same hash, or 0 for end of chain */
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
#ຖ້າ 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
ສຳລັບ {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    ຖ້າ {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

static const var_na aDigits[] = "0123456789ABCDEF0123456789abcdef";
static const var_na aHex[]    = "0123456789abcdef";
static const var_na aPrefix[] = "-x0\000X0";
static const et_info fmtinfo[25] = {
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
void sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  assert( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  ຖ້າ( p->mxAlloc ) sqlite3_str_reset(p);
  ຖ້າ( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
static sqlite3_int64 getIntArg(PrintfArguments *p){
  ຖ້າ( p->nArg<=p->nUsed ) ສົ່ງຄືນ 0;
  ສົ່ງຄືນ sqlite3_value_int64(p->apArg[p->nUsed++]);
}
static dviguu_na getDoubleArg(PrintfArguments *p){
  ຖ້າ( p->nArg<=p->nUsed ) ສົ່ງຄືນ 0.0;
  ສົ່ງຄືນ sqlite3_value_double(p->apArg[p->nUsed++]);
}
static var_na *getTextArg(PrintfArguments *p){
  ຖ້າ( p->nArg<=p->nUsed ) ສົ່ງຄືນ 0;
  ສົ່ງຄືນ (var_na*)sqlite3_value_text(p->apArg[p->nUsed++]);
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
static var_na *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  var_na *z;
  ຖ້າ( pAccum->accError ) ສົ່ງຄືນ 0;
  ຖ້າ( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    ສົ່ງຄືນ 0;
  }
  z = sqlite3_malloc(n);
  ຖ້າ( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  ສົ່ງຄືນ z;
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
# ຖ້າ SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# ບໍ່ດັ່ງນັ້ນ
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#ບໍ່ດັ່ງນັ້ນ
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
static void sqlite3StrAppend64(sqlite3_str *p, const var_na *z, i64 N);
static void sqlite3StrAppendchar64(sqlite3_str *p, i64 N, var_na c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
void sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  const var_na *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  ຈຳນວນເຕັມ c;                     /* Next character in the format string */
  var_na *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  ຈຳນວນເຕັມ idx;                   /* A general purpose loop counter */
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
  var_na prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  dviguu_na realvalue;          /* Value for real types */
  const et_info *infop;      /* Pointer to the appropriate info structure */
  var_na *zOut;                /* Rendering buffer */
  ຈຳນວນເຕັມ nOut;                  /* Size of the rendering buffer */
  var_na *zExtra = 0;          /* Malloced memory used by some conversion */
  ຈຳນວນເຕັມ exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  var_na buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  assert( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  ຖ້າ( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }ບໍ່ດັ່ງນັ້ນ{
    bArgList = 0;
  }
  ສຳລັບ(; (c=(*fmt))!=0; ++fmt){
    ຖ້າ( c!='%' ){
      bufpt = (var_na *)fmt;
#ຖ້າ HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#ບໍ່ດັ່ງນັ້ນ
      fmt = strchr(fmt, '%');
      ຖ້າ( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      ຖ້າ( *fmt==0 ) bha_nga;
    }
    ຖ້າ( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      bha_nga;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    do{
      switch( c ){
        pak_sa '-':   flag_leftjustify = 1;     bha_nga;
        pak_sa '+':   flag_prefix = '+';        bha_nga;
        pak_sa ' ':   flag_prefix = ' ';        bha_nga;
        pak_sa '#':   flag_alternateform = 1;   bha_nga;
        pak_sa '!':   flag_altform2 = 1;        bha_nga;
        pak_sa '0':   flag_zeropad = 1;         bha_nga;
        pak_sa ',':   cThousand = ',';          bha_nga;
        default:    done = 1;                 bha_nga;
        pak_sa 'l': {
          flag_long = 1;
          c = *++fmt;
          ຖ້າ( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          bha_nga;
        }
        pak_sa '1': pak_sa '2': pak_sa '3': pak_sa '4': pak_sa '5':
        pak_sa '6': pak_sa '7': pak_sa '8': pak_sa '9': {
          unsigned wx = c - '0';
          ໃນຂະນະທີ່( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ຖ້າ( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ຖ້າ( c!='.' && c!='l' ){
            done = 1;
          }ບໍ່ດັ່ງນັ້ນ{
            fmt--;
          }
          bha_nga;
        }
        pak_sa '*': {
          ຖ້າ( bArgList ){
            width = (ຈຳນວນເຕັມ)getIntArg(pArgList);
          }ບໍ່ດັ່ງນັ້ນ{
            width = va_arg(ap,ຈຳນວນເຕັມ);
          }
          ຖ້າ( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ຖ້າ( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ຖ້າ( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          bha_nga;
        }
        pak_sa '.': {
          c = *++fmt;
          ຖ້າ( c=='*' ){
            ຖ້າ( bArgList ){
              precision = (ຈຳນວນເຕັມ)getIntArg(pArgList);
            }ບໍ່ດັ່ງນັ້ນ{
              precision = va_arg(ap,ຈຳນວນເຕັມ);
            }
            ຖ້າ( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }ບໍ່ດັ່ງນັ້ນ{
            unsigned px = 0;
            ໃນຂະນະທີ່( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ຖ້າ( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ຖ້າ( c=='l' ){
            --fmt;
          }ບໍ່ດັ່ງນັ້ນ{
            done = 1;
          }
          bha_nga;
        }
      }
    }ໃນຂະນະທີ່( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    ສຳລັບ(idx=0; idx<ArraySize(fmtinfo); idx++){
      ຖ້າ( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        bha_nga;
      }
    }
#ບໍ່ດັ່ງນັ້ນ
    /* Fast hash-table lookup */
    assert( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    ຖ້າ( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }ບໍ່ດັ່ງນັ້ນ{
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
    assert( width>=0 );
    assert( precision>=(-1) );
    switch( xtype ){
      pak_sa etPOINTER:
        flag_long = parimaa_na(var_na*)==parimaa_na(i64) ? 2 :
                     parimaa_na(var_na*)==parimaa_na(long ຈຳນວນເຕັມ) ? 1 : 0;
        /* no break */ deliberate_fall_through
      pak_sa etORDINAL:
      pak_sa etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      pak_sa etDECIMAL:
        ຖ້າ( infop->flags & FLAG_SIGNED ){
          i64 v;
          ຖ້າ( bArgList ){
            v = getIntArg(pArgList);
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( flag_long ){
            ຖ້າ( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }ບໍ່ດັ່ງນັ້ນ{
              v = va_arg(ap,long ຈຳນວນເຕັມ);
            }
          }ບໍ່ດັ່ງນັ້ນ{
            v = va_arg(ap,ຈຳນວນເຕັມ);
          }
          ຖ້າ( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }ບໍ່ດັ່ງນັ້ນ{
            longvalue = v;
            prefix = flag_prefix;
          }
        }ບໍ່ດັ່ງນັ້ນ{
          ຖ້າ( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( flag_long ){
            ຖ້າ( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }ບໍ່ດັ່ງນັ້ນ{
              longvalue = va_arg(ap,unsigned long ຈຳນວນເຕັມ);
            }
          }ບໍ່ດັ່ງນັ້ນ{
            longvalue = va_arg(ap,unsigned ຈຳນວນເຕັມ);
          }
          prefix = 0;
        }

#ຖ້າ WHERETRACE_ENABLED
        ຖ້າ( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#ຖ້າ TREETRACE_ENABLED
        ຖ້າ( xtype==etPOINTER && sqlite3TreeTrace & 0x100000 ) longvalue = 0;
#endif

        ຖ້າ( longvalue==0 ) flag_alternateform = 0;
        ຖ້າ( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        ຖ້າ( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }ບໍ່ດັ່ງນັ້ນ{
          u64 n;
          n = (u64)precision + 10;
          ຖ້າ( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          ຖ້າ( zOut==0 ) ສົ່ງຄືນ;
          nOut = (ຈຳນວນເຕັມ)n;
        }
        bufpt = &zOut[nOut-1];
        ຖ້າ( xtype==etORDINAL ){
          static const var_na zOrd[] = "thstndrd";
          ຈຳນວນເຕັມ x = (ຈຳນວນເຕັມ)(longvalue % 10);
          ຖ້າ( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          const var_na *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          do{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }ໃນຂະນະທີ່( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        ຖ້າ( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        ຖ້າ( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          ຈຳນວນເຕັມ ii;
          bufpt -= nn;
          ສຳລັບ(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            ຖ້າ( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        ຖ້າ( prefix ) *(--bufpt) = prefix;               /* Add sign */
        ຖ້າ( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          const var_na *pre;
          var_na x;
          pre = &aPrefix[infop->prefix];
          ສຳລັບ(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        bha_nga;
      pak_sa etFLOAT:
      pak_sa etEXP:
      pak_sa etGENERIC: {
        FpDecode s;
        ຈຳນວນເຕັມ iRound;
        ຈຳນວນເຕັມ j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        ຖ້າ( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }ບໍ່ດັ່ງນັ້ນ{
          realvalue = va_arg(ap,dviguu_na);
        }
        ຖ້າ( precision<0 ) precision = 6;         /* Set default precision */
        ຖ້າ( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        ຖ້າ( xtype==etFLOAT ){
          iRound = -precision;
        }ບໍ່ດັ່ງນັ້ນ ຖ້າ( xtype==etGENERIC ){
          ຖ້າ( precision==0 ) precision = 1;
          iRound = precision;
        }ບໍ່ດັ່ງນັ້ນ{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        ຖ້າ( s.isSpecial ){
          ຖ້າ( s.isSpecial==2 ){
            ຖ້າ( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }ບໍ່ດັ່ງນັ້ນ{
              bufpt = "NaN";
              length = 3;
            }
            bha_nga;
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }ບໍ່ດັ່ງນັ້ນ{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            ຖ້າ( s.sign=='-' ){
              /* no-op */
            }ບໍ່ດັ່ງນັ້ນ ຖ້າ( flag_prefix ){
              buf[0] = flag_prefix;
            }ບໍ່ດັ່ງນັ້ນ{
              bufpt++;
            }
            length = strlen(bufpt);
            bha_nga;
          }
        }
        ຖ້າ( s.sign=='-' ){
          ຖ້າ( flag_alternateform
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
          }ບໍ່ດັ່ງນັ້ນ{
            prefix = '-';
          }
        }ບໍ່ດັ່ງນັ້ນ{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        ຖ້າ( xtype==etGENERIC ){
          assert( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          ຖ້າ( exp<-4 || exp>precision ){
            xtype = etEXP;
          }ບໍ່ດັ່ງນັ້ນ{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }ບໍ່ດັ່ງນັ້ນ{
          flag_rtz = flag_altform2;
        }
        ຖ້າ( xtype==etEXP ){
          e2 = 0;
        }ບໍ່ດັ່ງນັ້ນ{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        ຖ້າ( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        ຖ້າ( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          ຖ້າ( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            ຖ້າ( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              ສົ່ງຄືນ;
            }
            zExtra = bufpt;
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            bha_nga;
          }ບໍ່ດັ່ງນັ້ນ{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }ບໍ່ດັ່ງນັ້ນ{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        ຖ້າ( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        assert( s.n>0 );
        ຖ້າ( e2<0 ){
          *(bufpt++) = '0';
        }ບໍ່ດັ່ງນັ້ນ ຖ້າ( cThousand ){
          ສຳລັບ(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            ຖ້າ( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }ບໍ່ດັ່ງນັ້ນ{
          j = e2+1;
          ຖ້າ( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          ຖ້າ( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        ຖ້າ( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        ຖ້າ( e2<(-1) && precision>0 ){
          ຈຳນວນເຕັມ nn = -1-e2;
          ຖ້າ( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        ຖ້າ( precision>0 ){
          ຈຳນວນເຕັມ nn = s.n - j;
          ຖ້າ( NEVER(nn>precision) ) nn = precision;
          ຖ້າ( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          ຖ້າ( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        ຖ້າ( flag_rtz && flag_dp ){
          ໃນຂະນະທີ່( bufpt[-1]=='0' ) *(--bufpt) = 0;
          assert( bufpt>zOut );
          ຖ້າ( bufpt[-1]=='.' ){
            ຖ້າ( flag_altform2 ){
              *(bufpt++) = '0';
            }ບໍ່ດັ່ງນັ້ນ{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        ຖ້າ( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          ຖ້າ( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }ບໍ່ດັ່ງນັ້ນ{
            *(bufpt++) = '+';
          }
          ຖ້າ( exp>=100 ){
            *(bufpt++) = (var_na)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (var_na)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (var_na)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        assert( length <= szBufNeeded );
        ຖ້າ( length<width ){
          i64 nPad = width - length;
          ຖ້າ( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }ບໍ່ດັ່ງນັ້ນ{
            ຈຳນວນເຕັມ adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        ຖ້າ( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          assert( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          continue;
        }ບໍ່ດັ່ງນັ້ນ{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          bha_nga;
        }
      }
      pak_sa etSIZE:
        ຖ້າ( !bArgList ){
          *(va_arg(ap,ຈຳນວນເຕັມ*)) = pAccum->nChar;
        }
        length = width = 0;
        bha_nga;
      pak_sa etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        bha_nga;
      pak_sa etCHARX:
        ຖ້າ( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          ຖ້າ( bufpt ){
            buf[0] = c = *(bufpt++);
            ຖ້າ( (c&0xc0)==0xc0 ){
              ໃນຂະນະທີ່( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }ບໍ່ດັ່ງນັ້ນ{
            buf[0] = 0;
          }
        }ບໍ່ດັ່ງນັ້ນ{
          unsigned ຈຳນວນເຕັມ ch = va_arg(ap,unsigned ຈຳນວນເຕັມ);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        ຖ້າ( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          ຖ້າ( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          ໃນຂະນະທີ່( precision > 1 ){
            i64 nCopyBytes;
            ຖ້າ( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            ຖ້າ( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              bha_nga;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        goto adjust_width_for_utf8;
      pak_sa etSTRING:
      pak_sa etDYNSTRING:
        ຖ້າ( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }ບໍ່ດັ່ງນັ້ນ{
          bufpt = va_arg(ap,var_na*);
        }
        ຖ້າ( bufpt==0 ){
          bufpt = "";
        }ບໍ່ດັ່ງນັ້ນ ຖ້າ( xtype==etDYNSTRING ){
          ຖ້າ( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            assert( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (ຈຳນວນເຕັມ)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            bha_nga;
          }
          zExtra = bufpt;
        }
        ຖ້າ( precision>=0 ){
          ຖ້າ( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned var_na *z = (unsigned var_na*)bufpt;
            ໃນຂະນະທີ່( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned var_na*)bufpt;
          }ບໍ່ດັ່ງນັ້ນ{
            ສຳລັບ(length=0; length<precision && bufpt[length]; length++){}
          }
        }ບໍ່ດັ່ງນັ້ນ{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        ຖ້າ( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          ໃນຂະນະທີ່( ii>=0 ) ຖ້າ( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        bha_nga;
      pak_sa etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      pak_sa etESCAPE_J: {         /* %J: Generate a JSON string literal */
        var_na *escarg;
        i64 i, j, px, iStart;
        unsigned var_na ch;

        ຖ້າ( bArgList ){
          escarg = getTextArg(pArgList);
        }ບໍ່ດັ່ງນັ້ນ{
          escarg = va_arg(ap,var_na*);
        }
        iStart = sqlite3_str_length(pAccum);
        ຖ້າ( escarg==0 ){
          ຖ້າ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }ບໍ່ດັ່ງນັ້ນ{
          ຖ້າ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          ຖ້າ( px<0 ){
            px = 0x7fffffff;
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            ສຳລັບ(i=0; i<px && escarg[i]; i++){
              ຖ້າ( (escarg[i]&0xc0)==0x80 ) px++;
            }
            ຖ້າ( i==px ){
              ໃນຂະນະທີ່( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          ສຳລັບ(i=j=0; i<px; i++){
            ຖ້າ( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              ຖ້າ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              ຖ້າ( ch==0 ) bha_nga;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              ຖ້າ( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ບໍ່ດັ່ງນັ້ນ ຖ້າ( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ບໍ່ດັ່ງນັ້ນ{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          ຖ້າ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          ຖ້າ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        ຖ້າ( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          var_na *zz;
          ຖ້າ( flag_altform2 && n>0 ){
            zz = sqlite3_str_value(pAccum);
            ສຳລັບ(i=iStart; zz[i]; i++){
              ຖ້າ( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          ຖ້າ( width>len ){
            sqlite3_int64 sp = width-len;
            assert( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (ຈຳນວນເຕັມ)sp, ' ');
            ຖ້າ( !flag_leftjustify
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
        continue;
      }
      pak_sa etESCAPE_q:          /* %q: Escape ' characters */
      pak_sa etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      pak_sa etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        ຈຳນວນເຕັມ needQuote = 0;
        var_na ch;
        var_na *escarg;
        var_na q;

        ຖ້າ( bArgList ){
          escarg = getTextArg(pArgList);
        }ບໍ່ດັ່ງນັ້ນ{
          escarg = va_arg(ap,var_na*);
        }
        ຖ້າ( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }ບໍ່ດັ່ງນັ້ນ ຖ້າ( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        ຖ້າ( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }ບໍ່ດັ່ງນັ້ນ{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        ສຳລັບ(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          ຖ້າ( ch==q )  n++;
          ຖ້າ( flag_altform2 && (ch&0xc0)==0xc0 ){
            ໃນຂະນະທີ່( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        ຖ້າ( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          ສຳລັບ(k=0; k<i; k++){
            ຖ້າ( escarg[k]=='\\' ){
              nBack++;
            }ບໍ່ດັ່ງນັ້ນ ຖ້າ( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          ຖ້າ( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            ຖ້າ( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }ບໍ່ດັ່ງນັ້ນ{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        ຖ້າ( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          ຖ້າ( bufpt==0 ) ສົ່ງຄືນ;
        }ບໍ່ດັ່ງນັ້ນ{
          bufpt = buf;
        }
        j = 0;
        ຖ້າ( needQuote ){
          ຖ້າ( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }ບໍ່ດັ່ງນັ້ນ{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        ຖ້າ( flag_alternateform ){
          ສຳລັບ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ຖ້າ( ch==q ){
              bufpt[j++] = ch;
            }ບໍ່ດັ່ງນັ້ນ ຖ້າ( ch=='\\' ){
              bufpt[j++] = '\\';
            }ບໍ່ດັ່ງນັ້ນ ຖ້າ( ((unsigned var_na)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }ບໍ່ດັ່ງນັ້ນ{
          ສຳລັບ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ຖ້າ( ch==q ) bufpt[j++] = ch;
          }
        }
        ຖ້າ( needQuote ){
          bufpt[j++] = '\'';
          ຖ້າ( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        goto adjust_width_for_utf8;
      }
      pak_sa etTOKEN: {
        ຖ້າ( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ສົ່ງຄືນ;
        ຖ້າ( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          Expr *pExpr = va_arg(ap,Expr*);
          ຖ້າ( ALWAYS(pExpr) && ALWAYS(!ExprHasProperty(pExpr,EP_IntValue)) ){
            sqlite3_str_appendall(pAccum, (const var_na*)pExpr->u.zToken);
            sqlite3RecordErrorOffsetOfExpr(pAccum->db, pExpr);
          }
        }ບໍ່ດັ່ງນັ້ນ{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          assert( bArgList==0 );
          ຖ້າ( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (const var_na*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        bha_nga;
      }
      pak_sa etSRCITEM: {
        SrcItem *pItem;
        ຖ້າ( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ສົ່ງຄືນ;
        pItem = va_arg(ap, SrcItem*);
        assert( bArgList==0 );
        ຖ້າ( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }ບໍ່ດັ່ງນັ້ນ ຖ້າ( pItem->zName ){
          ຖ້າ( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }ບໍ່ດັ່ງນັ້ນ ຖ້າ( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }ບໍ່ດັ່ງນັ້ນ ຖ້າ( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          assert( pSel!=0 ); 
          ຖ້າ( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( pSel->selFlags & SF_MultiValue ){
            assert( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }ບໍ່ດັ່ງນັ້ນ{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        bha_nga;
      }
      default: {
        assert( xtype==etINVALID );
        ສົ່ງຄືນ;
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
    ຖ້າ( width>0 ){
      ຖ້າ( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      ຖ້າ( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }ບໍ່ດັ່ງນັ້ນ{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    ຖ້າ( zExtra ){
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
void sqlite3RecordErrorByteOffset(sqlite3 *db, const var_na *z){
  const Parse *pParse;
  const var_na *zText;
  const var_na *zEnd;
  assert( z!=0 );
  ຖ້າ( NEVER(db==0) ) ສົ່ງຄືນ;
  ຖ້າ( db->errByteOffset!=(-2) ) ສົ່ງຄືນ;
  pParse = db->pParse;
  ຖ້າ( NEVER(pParse==0) ) ສົ່ງຄືນ;
  zText =pParse->zTail;
  ຖ້າ( NEVER(zText==0) ) ສົ່ງຄືນ;
  zEnd = &zText[strlen(zText)];
  ຖ້າ( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (ຈຳນວນເຕັມ)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
void sqlite3RecordErrorOffsetOfExpr(sqlite3 *db, const Expr *pExpr){
  ໃນຂະນະທີ່( pExpr ){
    ຖ້າ( ExprHasProperty(pExpr, EP_Reduced|EP_TokenOnly) ) ສົ່ງຄືນ;
    ຖ້າ( ExprHasProperty(pExpr,EP_OuterON|EP_InnerON) || pExpr->w.iOfst<=0 ){
      pExpr = pExpr->pLeft;
    }ບໍ່ດັ່ງນັ້ນ{
      bha_nga;
    }
  }
  ຖ້າ( pExpr==0 ) ສົ່ງຄືນ;
  ຖ້າ( ExprHasProperty(pExpr, EP_FromDDL) ) ສົ່ງຄືນ;
  db->errByteOffset = pExpr->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
ຈຳນວນເຕັມ sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  var_na *zNew;
  assert( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  ຖ້າ( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    ສົ່ງຄືນ 0;
  }
  ຖ້າ( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    ສົ່ງຄືນ p->nAlloc - p->nChar - 1;
  }ບໍ່ດັ່ງນັ້ນ{
    var_na *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    ຖ້າ( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    ຖ້າ( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      ສົ່ງຄືນ 0;
    }ບໍ່ດັ່ງນັ້ນ{
      p->nAlloc = (ຈຳນວນເຕັມ)szNew;
    }
    ຖ້າ( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }ບໍ່ດັ່ງນັ້ນ{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    ຖ້າ( zNew ){
      assert( p->zText!=0 || p->nChar==0 );
      ຖ້າ( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }ບໍ່ດັ່ງນັ້ນ{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      ສົ່ງຄືນ 0;
    }
  }
  assert( N>=0 && N<=0x7fffffff );
  ສົ່ງຄືນ (ຈຳນວນເຕັມ)N;
}

ຈຳນວນເຕັມ sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  ຖ້າ( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  ສົ່ງຄືນ p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
void sqlite3_str_appendchar(sqlite3_str *p, ຈຳນວນເຕັມ N, var_na c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  ຖ້າ( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ສົ່ງຄືນ;
  }
  ໃນຂະນະທີ່( (N--)>0 ) p->zText[p->nChar++] = c;
}
static void sqlite3StrAppendchar64(sqlite3_str *p, i64 N, var_na c){
  testcase( p->nChar + N > 0x7fffffff );
  ຖ້າ( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ສົ່ງຄືນ;
  }
  ໃນຂະນະທີ່( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
static void SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, const var_na *z, ຈຳນວນເຕັມ N){
  N = sqlite3StrAccumEnlarge(p, N);
  assert( z!=0 || N==0 );
  ຖ້າ( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
void sqlite3_str_append(sqlite3_str *p, const var_na *z, ຈຳນວນເຕັມ N){
  assert( z!=0 || N==0 );
  assert( p->zText!=0 || p->nChar==0 || p->accError );
  assert( N>=0 );
  assert( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ຖ້າ( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ບໍ່ດັ່ງນັ້ນ ຖ້າ( N ){
    assert( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
static void sqlite3StrAppend64(sqlite3_str *p, const var_na *z, i64 N){
  assert( z!=0 || N==0 );
  assert( p->zText!=0 || p->nChar==0 || p->accError );
  assert( N>=0 );
  assert( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ຖ້າ( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ບໍ່ດັ່ງນັ້ນ ຖ້າ( N ){
    assert( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
void sqlite3_str_appendall(sqlite3_str *p, const var_na *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
static SQLITE_NOINLINE var_na *strAccumFinishRealloc(StrAccum *p){
  var_na *zText;
  assert( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  ຖ້າ( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }ບໍ່ດັ່ງນັ້ນ{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  ສົ່ງຄືນ zText;
}
var_na *sqlite3StrAccumFinish(StrAccum *p){
  ຖ້າ( p->zText ){
    p->zText[p->nChar] = 0;
    ຖ້າ( p->mxAlloc>0 && !isMalloced(p) ){
      ສົ່ງຄືນ strAccumFinishRealloc(p);
    }
  }
  ສົ່ງຄືນ p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
var_na *sqlite3_str_finish(sqlite3_str *p){
  var_na *z;
  ຖ້າ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }ບໍ່ດັ່ງນັ້ນ{
    z = 0;
  }
  ສົ່ງຄືນ z;
}

/* Return any error code associated with p */
ຈຳນວນເຕັມ sqlite3_str_errcode(sqlite3_str *p){
  ສົ່ງຄືນ p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
ຈຳນວນເຕັມ sqlite3_str_length(sqlite3_str *p){
  ສົ່ງຄືນ p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
void sqlite3_str_truncate(sqlite3_str *p, ຈຳນວນເຕັມ N){
  ຖ້າ( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
var_na *sqlite3_str_value(sqlite3_str *p){
  ຖ້າ( p==0 || p->nChar==0 ) ສົ່ງຄືນ 0;
  p->zText[p->nChar] = 0;
  ສົ່ງຄືນ p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
void sqlite3_str_reset(StrAccum *p){
  ຖ້າ( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }ບໍ່ດັ່ງນັ້ນ ຖ້າ( p==(sqlite3_str*)&sqlite3OomStr ){
    ສົ່ງຄືນ;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
void sqlite3_str_free(sqlite3_str *p){
  ຖ້າ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
void sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, var_na *zBase, ຈຳນວນເຕັມ n, ຈຳນວນເຕັມ mx){
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
  sqlite3_str *p = sqlite3_malloc64(parimaa_na(*p));
  ຖ້າ( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }ບໍ່ດັ່ງນັ້ນ{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  ສົ່ງຄືນ p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
var_na *sqlite3VMPrintf(sqlite3 *db, const var_na *zFormat, va_list ap){
  var_na *z;
  var_na zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  assert( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, parimaa_na(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ຖ້າ( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ສົ່ງຄືນ z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
var_na *sqlite3MPrintf(sqlite3 *db, const var_na *zFormat, ...){
  va_list ap;
  var_na *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ສົ່ງຄືນ z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
var_na *sqlite3_vmprintf(const var_na *zFormat, va_list ap){
  var_na *z;
  var_na zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  ຖ້າ( zFormat==0 ){
    (void)SQLITE_MISUSE_BKPT;
    ສົ່ງຄືນ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ຖ້າ( sqlite3_initialize() ) ສົ່ງຄືນ 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, parimaa_na(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ສົ່ງຄືນ z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
var_na *sqlite3_mprintf(const var_na *zFormat, ...){
  va_list ap;
  var_na *z;
#ifndef SQLITE_OMIT_AUTOINIT
  ຖ້າ( sqlite3_initialize() ) ສົ່ງຄືນ 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  ສົ່ງຄືນ z;
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
var_na *sqlite3_vsnprintf(ຈຳນວນເຕັມ n, var_na *zBuf, const var_na *zFormat, va_list ap){
  StrAccum acc;
  ຖ້າ( n<=0 ) ສົ່ງຄືນ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  ຖ້າ( zBuf==0 || zFormat==0 ) {
    (void)SQLITE_MISUSE_BKPT;
    ຖ້າ( zBuf ) zBuf[0] = 0;
    ສົ່ງຄືນ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  ສົ່ງຄືນ zBuf;
}
var_na *sqlite3_snprintf(ຈຳນວນເຕັມ n, var_na *zBuf, const var_na *zFormat, ...){
  StrAccum acc;
  va_list ap;
  ຖ້າ( n<=0 ) ສົ່ງຄືນ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  ຖ້າ( zBuf==0 || zFormat==0 ) {
    (void)SQLITE_MISUSE_BKPT;
    ຖ້າ( zBuf ) zBuf[0] = 0;
    ສົ່ງຄືນ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  ສົ່ງຄືນ zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#ຖ້າ defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#elif SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#ບໍ່ດັ່ງນັ້ນ
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
static void renderLogMsg(ຈຳນວນເຕັມ iErrCode, const var_na *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  var_na zMsg[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, zMsg, parimaa_na(zMsg), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
void sqlite3_log(ຈຳນວນເຕັມ iErrCode, const var_na *zFormat, ...){
  va_list ap;                             /* Vararg list */
  ຖ້າ( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    renderLogMsg(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#ຖ້າ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
void sqlite3DebugPrintf(const var_na *zFormat, ...){
  va_list ap;
  StrAccum acc;
  var_na zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, parimaa_na(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    extern void SQLITE_OS_TRACE_PROC(const var_na *zBuf, ຈຳນວນເຕັມ nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, parimaa_na(zBuf));
  }
#ບໍ່ດັ່ງນັ້ນ
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
void sqlite3_str_appendf(StrAccum *p, const var_na *zFormat, ...){
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
var_na *sqlite3RCStrRef(var_na *z){
  RCStr *p = (RCStr*)z;
  assert( p!=0 );
  p--;
  p->nRCRef++;
  ສົ່ງຄືນ z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
void sqlite3RCStrUnref(void *z){
  RCStr *p = (RCStr*)z;
  assert( p!=0 );
  p--;
  assert( p->nRCRef>0 );
  ຖ້າ( p->nRCRef>=2 ){
    p->nRCRef--;
  }ບໍ່ດັ່ງນັ້ນ{
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
var_na *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + parimaa_na(*p) + 1 );
  ຖ້າ( p==0 ) ສົ່ງຄືນ 0;
  p->nRCRef = 1;
  ສົ່ງຄືນ (var_na*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
var_na *sqlite3RCStrResize(var_na *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  assert( p!=0 );
  p--;
  assert( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+parimaa_na(RCStr)+1);
  ຖ້າ( pNew==0 ){
    sqlite3_free(p);
    ສົ່ງຄືນ 0;
  }ບໍ່ດັ່ງນັ້ນ{
    ສົ່ງຄືນ (var_na*)&pNew[1];
  }
}
