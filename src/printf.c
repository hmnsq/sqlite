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
#གལ་སྲིད 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
དོན་ལུ {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    གལ་སྲིད {($n($c))%$mx==$r} {puts -nonewline " $c"}
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
  གལ་སྲིད( p->mxAlloc ) sqlite3_str_reset(p);
  གལ་སྲིད( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
static sqlite3_int64 getIntArg(PrintfArguments *p){
  གལ་སྲིད( p->nArg<=p->nUsed ) ལོག 0;
  ལོག sqlite3_value_int64(p->apArg[p->nUsed++]);
}
static dviguu_na getDoubleArg(PrintfArguments *p){
  གལ་སྲིད( p->nArg<=p->nUsed ) ལོག 0.0;
  ལོག sqlite3_value_double(p->apArg[p->nUsed++]);
}
static var_na *getTextArg(PrintfArguments *p){
  གལ་སྲིད( p->nArg<=p->nUsed ) ལོག 0;
  ལོག (var_na*)sqlite3_value_text(p->apArg[p->nUsed++]);
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
  གལ་སྲིད( pAccum->accError ) ལོག 0;
  གལ་སྲིད( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    ལོག 0;
  }
  z = sqlite3_malloc(n);
  གལ་སྲིད( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  ལོག z;
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
# གལ་སྲིད SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# དེ་མིན
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#དེ་མིན
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
  ཧྲིལ c;                     /* Next character in the format string */
  var_na *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  ཧྲིལ idx;                   /* A general purpose loop counter */
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
  ཧྲིལ nOut;                  /* Size of the rendering buffer */
  var_na *zExtra = 0;          /* Malloced memory used by some conversion */
  ཧྲིལ exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  var_na buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  assert( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  གལ་སྲིད( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }དེ་མིན{
    bArgList = 0;
  }
  དོན་ལུ(; (c=(*fmt))!=0; ++fmt){
    གལ་སྲིད( c!='%' ){
      bufpt = (var_na *)fmt;
#གལ་སྲིད HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#དེ་མིན
      fmt = strchr(fmt, '%');
      གལ་སྲིད( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      གལ་སྲིད( *fmt==0 ) bha_nga;
    }
    གལ་སྲིད( (c=(*++fmt))==0 ){
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
          གལ་སྲིད( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          bha_nga;
        }
        pak_sa '1': pak_sa '2': pak_sa '3': pak_sa '4': pak_sa '5':
        pak_sa '6': pak_sa '7': pak_sa '8': pak_sa '9': {
          unsigned wx = c - '0';
          སྐབས( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          གལ་སྲིད( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          གལ་སྲིད( c!='.' && c!='l' ){
            done = 1;
          }དེ་མིན{
            fmt--;
          }
          bha_nga;
        }
        pak_sa '*': {
          གལ་སྲིད( bArgList ){
            width = (ཧྲིལ)getIntArg(pArgList);
          }དེ་མིན{
            width = va_arg(ap,ཧྲིལ);
          }
          གལ་སྲིད( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          གལ་སྲིད( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          གལ་སྲིད( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          bha_nga;
        }
        pak_sa '.': {
          c = *++fmt;
          གལ་སྲིད( c=='*' ){
            གལ་སྲིད( bArgList ){
              precision = (ཧྲིལ)getIntArg(pArgList);
            }དེ་མིན{
              precision = va_arg(ap,ཧྲིལ);
            }
            གལ་སྲིད( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }དེ་མིན{
            unsigned px = 0;
            སྐབས( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          གལ་སྲིད( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          གལ་སྲིད( c=='l' ){
            --fmt;
          }དེ་མིན{
            done = 1;
          }
          bha_nga;
        }
      }
    }སྐབས( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    དོན་ལུ(idx=0; idx<ArraySize(fmtinfo); idx++){
      གལ་སྲིད( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        bha_nga;
      }
    }
#དེ་མིན
    /* Fast hash-table lookup */
    assert( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    གལ་སྲིད( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }དེ་མིན{
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
                     parimaa_na(var_na*)==parimaa_na(long ཧྲིལ) ? 1 : 0;
        /* no break */ deliberate_fall_through
      pak_sa etORDINAL:
      pak_sa etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      pak_sa etDECIMAL:
        གལ་སྲིད( infop->flags & FLAG_SIGNED ){
          i64 v;
          གལ་སྲིད( bArgList ){
            v = getIntArg(pArgList);
          }དེ་མིན གལ་སྲིད( flag_long ){
            གལ་སྲིད( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }དེ་མིན{
              v = va_arg(ap,long ཧྲིལ);
            }
          }དེ་མིན{
            v = va_arg(ap,ཧྲིལ);
          }
          གལ་སྲིད( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }དེ་མིན{
            longvalue = v;
            prefix = flag_prefix;
          }
        }དེ་མིན{
          གལ་སྲིད( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }དེ་མིན གལ་སྲིད( flag_long ){
            གལ་སྲིད( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }དེ་མིན{
              longvalue = va_arg(ap,unsigned long ཧྲིལ);
            }
          }དེ་མིན{
            longvalue = va_arg(ap,unsigned ཧྲིལ);
          }
          prefix = 0;
        }

#གལ་སྲིད WHERETRACE_ENABLED
        གལ་སྲིད( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#གལ་སྲིད TREETRACE_ENABLED
        གལ་སྲིད( xtype==etPOINTER && sqlite3TreeTrace & 0x100000 ) longvalue = 0;
#endif

        གལ་སྲིད( longvalue==0 ) flag_alternateform = 0;
        གལ་སྲིད( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        གལ་སྲིད( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }དེ་མིན{
          u64 n;
          n = (u64)precision + 10;
          གལ་སྲིད( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          གལ་སྲིད( zOut==0 ) ལོག;
          nOut = (ཧྲིལ)n;
        }
        bufpt = &zOut[nOut-1];
        གལ་སྲིད( xtype==etORDINAL ){
          static const var_na zOrd[] = "thstndrd";
          ཧྲིལ x = (ཧྲིལ)(longvalue % 10);
          གལ་སྲིད( x>=4 || (longvalue/10)%10==1 ){
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
          }སྐབས( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        གལ་སྲིད( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        གལ་སྲིད( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          ཧྲིལ ii;
          bufpt -= nn;
          དོན་ལུ(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            གལ་སྲིད( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        གལ་སྲིད( prefix ) *(--bufpt) = prefix;               /* Add sign */
        གལ་སྲིད( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          const var_na *pre;
          var_na x;
          pre = &aPrefix[infop->prefix];
          དོན་ལུ(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        bha_nga;
      pak_sa etFLOAT:
      pak_sa etEXP:
      pak_sa etGENERIC: {
        FpDecode s;
        ཧྲིལ iRound;
        ཧྲིལ j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        གལ་སྲིད( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }དེ་མིན{
          realvalue = va_arg(ap,dviguu_na);
        }
        གལ་སྲིད( precision<0 ) precision = 6;         /* Set default precision */
        གལ་སྲིད( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        གལ་སྲིད( xtype==etFLOAT ){
          iRound = -precision;
        }དེ་མིན གལ་སྲིད( xtype==etGENERIC ){
          གལ་སྲིད( precision==0 ) precision = 1;
          iRound = precision;
        }དེ་མིན{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        གལ་སྲིད( s.isSpecial ){
          གལ་སྲིད( s.isSpecial==2 ){
            གལ་སྲིད( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }དེ་མིན{
              bufpt = "NaN";
              length = 3;
            }
            bha_nga;
          }དེ་མིན གལ་སྲིད( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }དེ་མིན{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            གལ་སྲིད( s.sign=='-' ){
              /* no-op */
            }དེ་མིན གལ་སྲིད( flag_prefix ){
              buf[0] = flag_prefix;
            }དེ་མིན{
              bufpt++;
            }
            length = strlen(bufpt);
            bha_nga;
          }
        }
        གལ་སྲིད( s.sign=='-' ){
          གལ་སྲིད( flag_alternateform
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
          }དེ་མིན{
            prefix = '-';
          }
        }དེ་མིན{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        གལ་སྲིད( xtype==etGENERIC ){
          assert( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          གལ་སྲིད( exp<-4 || exp>precision ){
            xtype = etEXP;
          }དེ་མིན{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }དེ་མིན{
          flag_rtz = flag_altform2;
        }
        གལ་སྲིད( xtype==etEXP ){
          e2 = 0;
        }དེ་མིན{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        གལ་སྲིད( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        གལ་སྲིད( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          གལ་སྲིད( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            གལ་སྲིད( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              ལོག;
            }
            zExtra = bufpt;
          }དེ་མིན གལ་སྲིད( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            bha_nga;
          }དེ་མིན{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }དེ་མིན{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        གལ་སྲིད( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        assert( s.n>0 );
        གལ་སྲིད( e2<0 ){
          *(bufpt++) = '0';
        }དེ་མིན གལ་སྲིད( cThousand ){
          དོན་ལུ(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            གལ་སྲིད( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }དེ་མིན{
          j = e2+1;
          གལ་སྲིད( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          གལ་སྲིད( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        གལ་སྲིད( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        གལ་སྲིད( e2<(-1) && precision>0 ){
          ཧྲིལ nn = -1-e2;
          གལ་སྲིད( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        གལ་སྲིད( precision>0 ){
          ཧྲིལ nn = s.n - j;
          གལ་སྲིད( NEVER(nn>precision) ) nn = precision;
          གལ་སྲིད( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          གལ་སྲིད( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        གལ་སྲིད( flag_rtz && flag_dp ){
          སྐབས( bufpt[-1]=='0' ) *(--bufpt) = 0;
          assert( bufpt>zOut );
          གལ་སྲིད( bufpt[-1]=='.' ){
            གལ་སྲིད( flag_altform2 ){
              *(bufpt++) = '0';
            }དེ་མིན{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        གལ་སྲིད( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          གལ་སྲིད( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }དེ་མིན{
            *(bufpt++) = '+';
          }
          གལ་སྲིད( exp>=100 ){
            *(bufpt++) = (var_na)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (var_na)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (var_na)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        assert( length <= szBufNeeded );
        གལ་སྲིད( length<width ){
          i64 nPad = width - length;
          གལ་སྲིད( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }དེ་མིན གལ་སྲིད( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }དེ་མིན{
            ཧྲིལ adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        གལ་སྲིད( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          assert( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          continue;
        }དེ་མིན{
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
        གལ་སྲིད( !bArgList ){
          *(va_arg(ap,ཧྲིལ*)) = pAccum->nChar;
        }
        length = width = 0;
        bha_nga;
      pak_sa etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        bha_nga;
      pak_sa etCHARX:
        གལ་སྲིད( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          གལ་སྲིད( bufpt ){
            buf[0] = c = *(bufpt++);
            གལ་སྲིད( (c&0xc0)==0xc0 ){
              སྐབས( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }དེ་མིན{
            buf[0] = 0;
          }
        }དེ་མིན{
          unsigned ཧྲིལ ch = va_arg(ap,unsigned ཧྲིལ);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        གལ་སྲིད( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          གལ་སྲིད( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          སྐབས( precision > 1 ){
            i64 nCopyBytes;
            གལ་སྲིད( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            གལ་སྲིད( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
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
        གལ་སྲིད( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }དེ་མིན{
          bufpt = va_arg(ap,var_na*);
        }
        གལ་སྲིད( bufpt==0 ){
          bufpt = "";
        }དེ་མིན གལ་སྲིད( xtype==etDYNSTRING ){
          གལ་སྲིད( pAccum->nChar==0
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
            pAccum->nChar = 0x7fffffff & (ཧྲིལ)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            bha_nga;
          }
          zExtra = bufpt;
        }
        གལ་སྲིད( precision>=0 ){
          གལ་སྲིད( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned var_na *z = (unsigned var_na*)bufpt;
            སྐབས( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned var_na*)bufpt;
          }དེ་མིན{
            དོན་ལུ(length=0; length<precision && bufpt[length]; length++){}
          }
        }དེ་མིན{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        གལ་སྲིད( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          སྐབས( ii>=0 ) གལ་སྲིད( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        bha_nga;
      pak_sa etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      pak_sa etESCAPE_J: {         /* %J: Generate a JSON string literal */
        var_na *escarg;
        i64 i, j, px, iStart;
        unsigned var_na ch;

        གལ་སྲིད( bArgList ){
          escarg = getTextArg(pArgList);
        }དེ་མིན{
          escarg = va_arg(ap,var_na*);
        }
        iStart = sqlite3_str_length(pAccum);
        གལ་སྲིད( escarg==0 ){
          གལ་སྲིད( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }དེ་མིན{
          གལ་སྲིད( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          གལ་སྲིད( px<0 ){
            px = 0x7fffffff;
          }དེ་མིན གལ་སྲིད( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            དོན་ལུ(i=0; i<px && escarg[i]; i++){
              གལ་སྲིད( (escarg[i]&0xc0)==0x80 ) px++;
            }
            གལ་སྲིད( i==px ){
              སྐབས( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          དོན་ལུ(i=j=0; i<px; i++){
            གལ་སྲིད( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              གལ་སྲིད( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              གལ་སྲིད( ch==0 ) bha_nga;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              གལ་སྲིད( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }དེ་མིན གལ་སྲིད( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }དེ་མིན{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          གལ་སྲིད( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          གལ་སྲིད( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        གལ་སྲིད( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          var_na *zz;
          གལ་སྲིད( flag_altform2 && n>0 ){
            zz = sqlite3_str_value(pAccum);
            དོན་ལུ(i=iStart; zz[i]; i++){
              གལ་སྲིད( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          གལ་སྲིད( width>len ){
            sqlite3_int64 sp = width-len;
            assert( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (ཧྲིལ)sp, ' ');
            གལ་སྲིད( !flag_leftjustify
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
        ཧྲིལ needQuote = 0;
        var_na ch;
        var_na *escarg;
        var_na q;

        གལ་སྲིད( bArgList ){
          escarg = getTextArg(pArgList);
        }དེ་མིན{
          escarg = va_arg(ap,var_na*);
        }
        གལ་སྲིད( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }དེ་མིན གལ་སྲིད( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        གལ་སྲིད( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }དེ་མིན{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        དོན་ལུ(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          གལ་སྲིད( ch==q )  n++;
          གལ་སྲིད( flag_altform2 && (ch&0xc0)==0xc0 ){
            སྐབས( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        གལ་སྲིད( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          དོན་ལུ(k=0; k<i; k++){
            གལ་སྲིད( escarg[k]=='\\' ){
              nBack++;
            }དེ་མིན གལ་སྲིད( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          གལ་སྲིད( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            གལ་སྲིད( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }དེ་མིན{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        གལ་སྲིད( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          གལ་སྲིད( bufpt==0 ) ལོག;
        }དེ་མིན{
          bufpt = buf;
        }
        j = 0;
        གལ་སྲིད( needQuote ){
          གལ་སྲིད( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }དེ་མིན{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        གལ་སྲིད( flag_alternateform ){
          དོན་ལུ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            གལ་སྲིད( ch==q ){
              bufpt[j++] = ch;
            }དེ་མིན གལ་སྲིད( ch=='\\' ){
              bufpt[j++] = '\\';
            }དེ་མིན གལ་སྲིད( ((unsigned var_na)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }དེ་མིན{
          དོན་ལུ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            གལ་སྲིད( ch==q ) bufpt[j++] = ch;
          }
        }
        གལ་སྲིད( needQuote ){
          bufpt[j++] = '\'';
          གལ་སྲིད( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        goto adjust_width_for_utf8;
      }
      pak_sa etTOKEN: {
        གལ་སྲིད( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ལོག;
        གལ་སྲིད( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          Expr *pExpr = va_arg(ap,Expr*);
          གལ་སྲིད( ALWAYS(pExpr) && ALWAYS(!ExprHasProperty(pExpr,EP_IntValue)) ){
            sqlite3_str_appendall(pAccum, (const var_na*)pExpr->u.zToken);
            sqlite3RecordErrorOffsetOfExpr(pAccum->db, pExpr);
          }
        }དེ་མིན{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          assert( bArgList==0 );
          གལ་སྲིད( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (const var_na*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        bha_nga;
      }
      pak_sa etSRCITEM: {
        SrcItem *pItem;
        གལ་སྲིད( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ལོག;
        pItem = va_arg(ap, SrcItem*);
        assert( bArgList==0 );
        གལ་སྲིད( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }དེ་མིན གལ་སྲིད( pItem->zName ){
          གལ་སྲིད( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }དེ་མིན གལ་སྲིད( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }དེ་མིན གལ་སྲིད( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          assert( pSel!=0 ); 
          གལ་སྲིད( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }དེ་མིན གལ་སྲིད( pSel->selFlags & SF_MultiValue ){
            assert( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }དེ་མིན{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        bha_nga;
      }
      default: {
        assert( xtype==etINVALID );
        ལོག;
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
    གལ་སྲིད( width>0 ){
      གལ་སྲིད( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      གལ་སྲིད( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }དེ་མིན{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    གལ་སྲིད( zExtra ){
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
  གལ་སྲིད( NEVER(db==0) ) ལོག;
  གལ་སྲིད( db->errByteOffset!=(-2) ) ལོག;
  pParse = db->pParse;
  གལ་སྲིད( NEVER(pParse==0) ) ལོག;
  zText =pParse->zTail;
  གལ་སྲིད( NEVER(zText==0) ) ལོག;
  zEnd = &zText[strlen(zText)];
  གལ་སྲིད( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (ཧྲིལ)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
void sqlite3RecordErrorOffsetOfExpr(sqlite3 *db, const Expr *pExpr){
  སྐབས( pExpr ){
    གལ་སྲིད( ExprHasProperty(pExpr, EP_Reduced|EP_TokenOnly) ) ལོག;
    གལ་སྲིད( ExprHasProperty(pExpr,EP_OuterON|EP_InnerON) || pExpr->w.iOfst<=0 ){
      pExpr = pExpr->pLeft;
    }དེ་མིན{
      bha_nga;
    }
  }
  གལ་སྲིད( pExpr==0 ) ལོག;
  གལ་སྲིད( ExprHasProperty(pExpr, EP_FromDDL) ) ལོག;
  db->errByteOffset = pExpr->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
ཧྲིལ sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  var_na *zNew;
  assert( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  གལ་སྲིད( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    ལོག 0;
  }
  གལ་སྲིད( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    ལོག p->nAlloc - p->nChar - 1;
  }དེ་མིན{
    var_na *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    གལ་སྲིད( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    གལ་སྲིད( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      ལོག 0;
    }དེ་མིན{
      p->nAlloc = (ཧྲིལ)szNew;
    }
    གལ་སྲིད( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }དེ་མིན{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    གལ་སྲིད( zNew ){
      assert( p->zText!=0 || p->nChar==0 );
      གལ་སྲིད( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }དེ་མིན{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      ལོག 0;
    }
  }
  assert( N>=0 && N<=0x7fffffff );
  ལོག (ཧྲིལ)N;
}

ཧྲིལ sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  གལ་སྲིད( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  ལོག p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
void sqlite3_str_appendchar(sqlite3_str *p, ཧྲིལ N, var_na c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  གལ་སྲིད( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ལོག;
  }
  སྐབས( (N--)>0 ) p->zText[p->nChar++] = c;
}
static void sqlite3StrAppendchar64(sqlite3_str *p, i64 N, var_na c){
  testcase( p->nChar + N > 0x7fffffff );
  གལ་སྲིད( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ལོག;
  }
  སྐབས( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
static void SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, const var_na *z, ཧྲིལ N){
  N = sqlite3StrAccumEnlarge(p, N);
  assert( z!=0 || N==0 );
  གལ་སྲིད( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
void sqlite3_str_append(sqlite3_str *p, const var_na *z, ཧྲིལ N){
  assert( z!=0 || N==0 );
  assert( p->zText!=0 || p->nChar==0 || p->accError );
  assert( N>=0 );
  assert( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  གལ་སྲིད( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }དེ་མིན གལ་སྲིད( N ){
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
  གལ་སྲིད( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }དེ་མིན གལ་སྲིད( N ){
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
  གལ་སྲིད( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }དེ་མིན{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  ལོག zText;
}
var_na *sqlite3StrAccumFinish(StrAccum *p){
  གལ་སྲིད( p->zText ){
    p->zText[p->nChar] = 0;
    གལ་སྲིད( p->mxAlloc>0 && !isMalloced(p) ){
      ལོག strAccumFinishRealloc(p);
    }
  }
  ལོག p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
var_na *sqlite3_str_finish(sqlite3_str *p){
  var_na *z;
  གལ་སྲིད( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }དེ་མིན{
    z = 0;
  }
  ལོག z;
}

/* Return any error code associated with p */
ཧྲིལ sqlite3_str_errcode(sqlite3_str *p){
  ལོག p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
ཧྲིལ sqlite3_str_length(sqlite3_str *p){
  ལོག p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
void sqlite3_str_truncate(sqlite3_str *p, ཧྲིལ N){
  གལ་སྲིད( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
var_na *sqlite3_str_value(sqlite3_str *p){
  གལ་སྲིད( p==0 || p->nChar==0 ) ལོག 0;
  p->zText[p->nChar] = 0;
  ལོག p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
void sqlite3_str_reset(StrAccum *p){
  གལ་སྲིད( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }དེ་མིན གལ་སྲིད( p==(sqlite3_str*)&sqlite3OomStr ){
    ལོག;
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
  གལ་སྲིད( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
void sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, var_na *zBase, ཧྲིལ n, ཧྲིལ mx){
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
  གལ་སྲིད( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }དེ་མིན{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  ལོག p;
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
  གལ་སྲིད( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ལོག z;
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
  ལོག z;
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
  གལ་སྲིད( zFormat==0 ){
    (void)SQLITE_MISUSE_BKPT;
    ལོག 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  གལ་སྲིད( sqlite3_initialize() ) ལོག 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, parimaa_na(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ལོག z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
var_na *sqlite3_mprintf(const var_na *zFormat, ...){
  va_list ap;
  var_na *z;
#ifndef SQLITE_OMIT_AUTOINIT
  གལ་སྲིད( sqlite3_initialize() ) ལོག 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  ལོག z;
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
var_na *sqlite3_vsnprintf(ཧྲིལ n, var_na *zBuf, const var_na *zFormat, va_list ap){
  StrAccum acc;
  གལ་སྲིད( n<=0 ) ལོག zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( zBuf==0 || zFormat==0 ) {
    (void)SQLITE_MISUSE_BKPT;
    གལ་སྲིད( zBuf ) zBuf[0] = 0;
    ལོག zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  ལོག zBuf;
}
var_na *sqlite3_snprintf(ཧྲིལ n, var_na *zBuf, const var_na *zFormat, ...){
  StrAccum acc;
  va_list ap;
  གལ་སྲིད( n<=0 ) ལོག zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( zBuf==0 || zFormat==0 ) {
    (void)SQLITE_MISUSE_BKPT;
    གལ་སྲིད( zBuf ) zBuf[0] = 0;
    ལོག zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  ལོག zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#གལ་སྲིད defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#elif SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#དེ་མིན
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
static void renderLogMsg(ཧྲིལ iErrCode, const var_na *zFormat, va_list ap){
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
void sqlite3_log(ཧྲིལ iErrCode, const var_na *zFormat, ...){
  va_list ap;                             /* Vararg list */
  གལ་སྲིད( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    renderLogMsg(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#གལ་སྲིད defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
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
    extern void SQLITE_OS_TRACE_PROC(const var_na *zBuf, ཧྲིལ nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, parimaa_na(zBuf));
  }
#དེ་མིན
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
  ལོག z;
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
  གལ་སྲིད( p->nRCRef>=2 ){
    p->nRCRef--;
  }དེ་མིན{
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
  གལ་སྲིད( p==0 ) ལོག 0;
  p->nRCRef = 1;
  ལོག (var_na*)&p[1];
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
  གལ་སྲིད( pNew==0 ){
    sqlite3_free(p);
    ལོག 0;
  }དེ་མིན{
    ལོག (var_na*)&pNew[1];
  }
}
