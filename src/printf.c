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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

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
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ವರ್ಣ etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ et_info {   /* Information about each format field */
  ವರ್ಣ fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  ವರ್ಣ iNxt;               /* Next with same hash, or 0 for end of chain */
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
#ಯದಿ 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
ಪ್ರತಿ {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    ಯದಿ {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ aDigits[] = "0123456789ABCDEF0123456789abcdef";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ aHex[]    = "0123456789abcdef";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ aPrefix[] = "-x0\000X0";
ಸ್ಥಿರ ಧ್ರುವ et_info fmtinfo[25] = {
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
ಶೂನ್ಯ sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  ಪ್ರತಿಜ್ಞಾ( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  ಯದಿ( p->mxAlloc ) sqlite3_str_reset(p);
  ಯದಿ( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ಸ್ಥಿರ sqlite3_int64 getIntArg(PrintfArguments *p){
  ಯದಿ( p->nArg<=p->nUsed ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_int64(p->apArg[p->nUsed++]);
}
ಸ್ಥಿರ ದ್ವಿಗುಣ getDoubleArg(PrintfArguments *p){
  ಯದಿ( p->nArg<=p->nUsed ) ಪ್ರತ್ಯಾಗಮ 0.0;
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಮೌಲ್ಯ_double(p->apArg[p->nUsed++]);
}
ಸ್ಥಿರ ವರ್ಣ *getTextArg(PrintfArguments *p){
  ಯದಿ( p->nArg<=p->nUsed ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(p->apArg[p->nUsed++]);
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
ಸ್ಥಿರ ವರ್ಣ *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  ವರ್ಣ *z;
  ಯದಿ( pAccum->accError ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  z = sqlite3_malloc(n);
  ಯದಿ( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  ಪ್ರತ್ಯಾಗಮ z;
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
# ಯದಿ SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# ಅನ್ಯಥಾ
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#ಅನ್ಯಥಾ
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3StrAppend64(sqlite3_str *p, ಧ್ರುವ ವರ್ಣ *z, i64 N);
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, ವರ್ಣ c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
ಶೂನ್ಯ sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ಧ್ರುವ ವರ್ಣ *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  ಪೂರ್ಣಾಙ್ಕ c;                     /* Next character in the format string */
  ವರ್ಣ *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  ಪೂರ್ಣಾಙ್ಕ idx;                   /* A general purpose loop counter */
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
  ವರ್ಣ prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  ದ್ವಿಗುಣ realvalue;          /* Value for real types */
  ಧ್ರುವ et_info *infop;      /* Pointer to the appropriate info structure */
  ವರ್ಣ *zOut;                /* Rendering buffer */
  ಪೂರ್ಣಾಙ್ಕ nOut;                  /* Size of the rendering buffer */
  ವರ್ಣ *zExtra = 0;          /* Malloced memory used by some conversion */
  ಪೂರ್ಣಾಙ್ಕ exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  ವರ್ಣ buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  ಪ್ರತಿಜ್ಞಾ( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  ಯದಿ( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }ಅನ್ಯಥಾ{
    bArgList = 0;
  }
  ಪ್ರತಿ(; (c=(*fmt))!=0; ++fmt){
    ಯದಿ( c!='%' ){
      bufpt = (ವರ್ಣ *)fmt;
#ಯದಿ HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#ಅನ್ಯಥಾ
      fmt = strchr(fmt, '%');
      ಯದಿ( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      ಯದಿ( *fmt==0 ) ಭಙ್ಗ;
    }
    ಯದಿ( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      ಭಙ್ಗ;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    ಕುರು{
      ವಿಕಲ್ಪ( c ){
        ಪಕ್ಷ '-':   flag_leftjustify = 1;     ಭಙ್ಗ;
        ಪಕ್ಷ '+':   flag_prefix = '+';        ಭಙ್ಗ;
        ಪಕ್ಷ ' ':   flag_prefix = ' ';        ಭಙ್ಗ;
        ಪಕ್ಷ '#':   flag_alternateform = 1;   ಭಙ್ಗ;
        ಪಕ್ಷ '!':   flag_altform2 = 1;        ಭಙ್ಗ;
        ಪಕ್ಷ '0':   flag_zeropad = 1;         ಭಙ್ಗ;
        ಪಕ್ಷ ',':   cThousand = ',';          ಭಙ್ಗ;
        ಸ್ವಭಾವ:    done = 1;                 ಭಙ್ಗ;
        ಪಕ್ಷ 'l': {
          flag_long = 1;
          c = *++fmt;
          ಯದಿ( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ '1': ಪಕ್ಷ '2': ಪಕ್ಷ '3': ಪಕ್ಷ '4': ಪಕ್ಷ '5':
        ಪಕ್ಷ '6': ಪಕ್ಷ '7': ಪಕ್ಷ '8': ಪಕ್ಷ '9': {
          unsigned wx = c - '0';
          ಯಾವತ್( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ಯದಿ( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ಯದಿ( c!='.' && c!='l' ){
            done = 1;
          }ಅನ್ಯಥಾ{
            fmt--;
          }
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ '*': {
          ಯದಿ( bArgList ){
            width = (ಪೂರ್ಣಾಙ್ಕ)getIntArg(pArgList);
          }ಅನ್ಯಥಾ{
            width = va_arg(ap,ಪೂರ್ಣಾಙ್ಕ);
          }
          ಯದಿ( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ಯದಿ( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ಯದಿ( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ '.': {
          c = *++fmt;
          ಯದಿ( c=='*' ){
            ಯದಿ( bArgList ){
              precision = (ಪೂರ್ಣಾಙ್ಕ)getIntArg(pArgList);
            }ಅನ್ಯಥಾ{
              precision = va_arg(ap,ಪೂರ್ಣಾಙ್ಕ);
            }
            ಯದಿ( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }ಅನ್ಯಥಾ{
            unsigned px = 0;
            ಯಾವತ್( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ಯದಿ( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ಯದಿ( c=='l' ){
            --fmt;
          }ಅನ್ಯಥಾ{
            done = 1;
          }
          ಭಙ್ಗ;
        }
      }
    }ಯಾವತ್( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    ಪ್ರತಿ(idx=0; idx<ArraySize(fmtinfo); idx++){
      ಯದಿ( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        ಭಙ್ಗ;
      }
    }
#ಅನ್ಯಥಾ
    /* Fast hash-table lookup */
    ಪ್ರತಿಜ್ಞಾ( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    ಯದಿ( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }ಅನ್ಯಥಾ{
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
    ಪ್ರತಿಜ್ಞಾ( width>=0 );
    ಪ್ರತಿಜ್ಞಾ( precision>=(-1) );
    ವಿಕಲ್ಪ( xtype ){
      ಪಕ್ಷ etPOINTER:
        flag_long = ಪರಿಮಾಣ(ವರ್ಣ*)==ಪರಿಮಾಣ(i64) ? 2 :
                     ಪರಿಮಾಣ(ವರ್ಣ*)==ಪರಿಮಾಣ(long ಪೂರ್ಣಾಙ್ಕ) ? 1 : 0;
        /* no break */ deliberate_fall_through
      ಪಕ್ಷ etORDINAL:
      ಪಕ್ಷ etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      ಪಕ್ಷ etDECIMAL:
        ಯದಿ( infop->flags & FLAG_SIGNED ){
          i64 v;
          ಯದಿ( bArgList ){
            v = getIntArg(pArgList);
          }ಅನ್ಯಥಾ ಯದಿ( flag_long ){
            ಯದಿ( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }ಅನ್ಯಥಾ{
              v = va_arg(ap,long ಪೂರ್ಣಾಙ್ಕ);
            }
          }ಅನ್ಯಥಾ{
            v = va_arg(ap,ಪೂರ್ಣಾಙ್ಕ);
          }
          ಯದಿ( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }ಅನ್ಯಥಾ{
            longvalue = v;
            prefix = flag_prefix;
          }
        }ಅನ್ಯಥಾ{
          ಯದಿ( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }ಅನ್ಯಥಾ ಯದಿ( flag_long ){
            ಯದಿ( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }ಅನ್ಯಥಾ{
              longvalue = va_arg(ap,unsigned long ಪೂರ್ಣಾಙ್ಕ);
            }
          }ಅನ್ಯಥಾ{
            longvalue = va_arg(ap,unsigned ಪೂರ್ಣಾಙ್ಕ);
          }
          prefix = 0;
        }

#ಯದಿ WHERETRACE_ENABLED
        ಯದಿ( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#ಯದಿ TREETRACE_ENABLED
        ಯದಿ( xtype==etPOINTER && sqlite3_ವೃಕ್ಷ_Trace & 0x100000 ) longvalue = 0;
#endif

        ಯದಿ( longvalue==0 ) flag_alternateform = 0;
        ಯದಿ( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        ಯದಿ( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }ಅನ್ಯಥಾ{
          u64 n;
          n = (u64)precision + 10;
          ಯದಿ( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          ಯದಿ( zOut==0 ) ಪ್ರತ್ಯಾಗಮ;
          nOut = (ಪೂರ್ಣಾಙ್ಕ)n;
        }
        bufpt = &zOut[nOut-1];
        ಯದಿ( xtype==etORDINAL ){
          ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ zOrd[] = "thstndrd";
          ಪೂರ್ಣಾಙ್ಕ x = (ಪೂರ್ಣಾಙ್ಕ)(longvalue % 10);
          ಯದಿ( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ಧ್ರುವ ವರ್ಣ *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          ಕುರು{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }ಯಾವತ್( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        ಯದಿ( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        ಯದಿ( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          ಪೂರ್ಣಾಙ್ಕ ii;
          bufpt -= nn;
          ಪ್ರತಿ(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            ಯದಿ( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        ಯದಿ( prefix ) *(--bufpt) = prefix;               /* Add sign */
        ಯದಿ( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          ಧ್ರುವ ವರ್ಣ *pre;
          ವರ್ಣ x;
          pre = &aPrefix[infop->prefix];
          ಪ್ರತಿ(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        ಭಙ್ಗ;
      ಪಕ್ಷ etFLOAT:
      ಪಕ್ಷ etEXP:
      ಪಕ್ಷ etGENERIC: {
        FpDecode s;
        ಪೂರ್ಣಾಙ್ಕ iRound;
        ಪೂರ್ಣಾಙ್ಕ j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        ಯದಿ( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }ಅನ್ಯಥಾ{
          realvalue = va_arg(ap,ದ್ವಿಗುಣ);
        }
        ಯದಿ( precision<0 ) precision = 6;         /* Set default precision */
        ಯದಿ( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        ಯದಿ( xtype==etFLOAT ){
          iRound = -precision;
        }ಅನ್ಯಥಾ ಯದಿ( xtype==etGENERIC ){
          ಯದಿ( precision==0 ) precision = 1;
          iRound = precision;
        }ಅನ್ಯಥಾ{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        ಯದಿ( s.isSpecial ){
          ಯದಿ( s.isSpecial==2 ){
            ಯದಿ( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }ಅನ್ಯಥಾ{
              bufpt = "NaN";
              length = 3;
            }
            ಭಙ್ಗ;
          }ಅನ್ಯಥಾ ಯದಿ( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }ಅನ್ಯಥಾ{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            ಯದಿ( s.sign=='-' ){
              /* no-op */
            }ಅನ್ಯಥಾ ಯದಿ( flag_prefix ){
              buf[0] = flag_prefix;
            }ಅನ್ಯಥಾ{
              bufpt++;
            }
            length = strlen(bufpt);
            ಭಙ್ಗ;
          }
        }
        ಯದಿ( s.sign=='-' ){
          ಯದಿ( flag_alternateform
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
          }ಅನ್ಯಥಾ{
            prefix = '-';
          }
        }ಅನ್ಯಥಾ{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        ಯದಿ( xtype==etGENERIC ){
          ಪ್ರತಿಜ್ಞಾ( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          ಯದಿ( exp<-4 || exp>precision ){
            xtype = etEXP;
          }ಅನ್ಯಥಾ{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }ಅನ್ಯಥಾ{
          flag_rtz = flag_altform2;
        }
        ಯದಿ( xtype==etEXP ){
          e2 = 0;
        }ಅನ್ಯಥಾ{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        ಯದಿ( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        ಯದಿ( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          ಯದಿ( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            ಯದಿ( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              ಪ್ರತ್ಯಾಗಮ;
            }
            zExtra = bufpt;
          }ಅನ್ಯಥಾ ಯದಿ( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            ಭಙ್ಗ;
          }ಅನ್ಯಥಾ{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }ಅನ್ಯಥಾ{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        ಯದಿ( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        ಪ್ರತಿಜ್ಞಾ( s.n>0 );
        ಯದಿ( e2<0 ){
          *(bufpt++) = '0';
        }ಅನ್ಯಥಾ ಯದಿ( cThousand ){
          ಪ್ರತಿ(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            ಯದಿ( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }ಅನ್ಯಥಾ{
          j = e2+1;
          ಯದಿ( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          ಯದಿ( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        ಯದಿ( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        ಯದಿ( e2<(-1) && precision>0 ){
          ಪೂರ್ಣಾಙ್ಕ nn = -1-e2;
          ಯದಿ( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        ಯದಿ( precision>0 ){
          ಪೂರ್ಣಾಙ್ಕ nn = s.n - j;
          ಯದಿ( NEVER(nn>precision) ) nn = precision;
          ಯದಿ( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          ಯದಿ( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        ಯದಿ( flag_rtz && flag_dp ){
          ಯಾವತ್( bufpt[-1]=='0' ) *(--bufpt) = 0;
          ಪ್ರತಿಜ್ಞಾ( bufpt>zOut );
          ಯದಿ( bufpt[-1]=='.' ){
            ಯದಿ( flag_altform2 ){
              *(bufpt++) = '0';
            }ಅನ್ಯಥಾ{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        ಯದಿ( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          ಯದಿ( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }ಅನ್ಯಥಾ{
            *(bufpt++) = '+';
          }
          ಯದಿ( exp>=100 ){
            *(bufpt++) = (ವರ್ಣ)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (ವರ್ಣ)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (ವರ್ಣ)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        ಪ್ರತಿಜ್ಞಾ( length <= szBufNeeded );
        ಯದಿ( length<width ){
          i64 nPad = width - length;
          ಯದಿ( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }ಅನ್ಯಥಾ ಯದಿ( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }ಅನ್ಯಥಾ{
            ಪೂರ್ಣಾಙ್ಕ adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        ಯದಿ( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          ಪ್ರತಿಜ್ಞಾ( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          ಅನುವರ್ತ;
        }ಅನ್ಯಥಾ{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          ಭಙ್ಗ;
        }
      }
      ಪಕ್ಷ etSIZE:
        ಯದಿ( !bArgList ){
          *(va_arg(ap,ಪೂರ್ಣಾಙ್ಕ*)) = pAccum->nChar;
        }
        length = width = 0;
        ಭಙ್ಗ;
      ಪಕ್ಷ etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        ಭಙ್ಗ;
      ಪಕ್ಷ etCHARX:
        ಯದಿ( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          ಯದಿ( bufpt ){
            buf[0] = c = *(bufpt++);
            ಯದಿ( (c&0xc0)==0xc0 ){
              ಯಾವತ್( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }ಅನ್ಯಥಾ{
            buf[0] = 0;
          }
        }ಅನ್ಯಥಾ{
          unsigned ಪೂರ್ಣಾಙ್ಕ ch = va_arg(ap,unsigned ಪೂರ್ಣಾಙ್ಕ);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        ಯದಿ( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          ಯದಿ( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          ಯಾವತ್( precision > 1 ){
            i64 nCopyBytes;
            ಯದಿ( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            ಯದಿ( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              ಭಙ್ಗ;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        ಗಚ್ಛ adjust_width_for_utf8;
      ಪಕ್ಷ etSTRING:
      ಪಕ್ಷ etDYNSTRING:
        ಯದಿ( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }ಅನ್ಯಥಾ{
          bufpt = va_arg(ap,ವರ್ಣ*);
        }
        ಯದಿ( bufpt==0 ){
          bufpt = "";
        }ಅನ್ಯಥಾ ಯದಿ( xtype==etDYNSTRING ){
          ಯದಿ( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            ಪ್ರತಿಜ್ಞಾ( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (ಪೂರ್ಣಾಙ್ಕ)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            ಭಙ್ಗ;
          }
          zExtra = bufpt;
        }
        ಯದಿ( precision>=0 ){
          ಯದಿ( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned ವರ್ಣ *z = (unsigned ವರ್ಣ*)bufpt;
            ಯಾವತ್( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned ವರ್ಣ*)bufpt;
          }ಅನ್ಯಥಾ{
            ಪ್ರತಿ(length=0; length<precision && bufpt[length]; length++){}
          }
        }ಅನ್ಯಥಾ{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        ಯದಿ( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          ಯಾವತ್( ii>=0 ) ಯದಿ( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        ಭಙ್ಗ;
      ಪಕ್ಷ etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      ಪಕ್ಷ etESCAPE_J: {         /* %J: Generate a JSON string literal */
        ವರ್ಣ *escarg;
        i64 i, j, px, iStart;
        unsigned ವರ್ಣ ch;

        ಯದಿ( bArgList ){
          escarg = getTextArg(pArgList);
        }ಅನ್ಯಥಾ{
          escarg = va_arg(ap,ವರ್ಣ*);
        }
        iStart = sqlite3_str_length(pAccum);
        ಯದಿ( escarg==0 ){
          ಯದಿ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }ಅನ್ಯಥಾ{
          ಯದಿ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          ಯದಿ( px<0 ){
            px = 0x7fffffff;
          }ಅನ್ಯಥಾ ಯದಿ( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            ಪ್ರತಿ(i=0; i<px && escarg[i]; i++){
              ಯದಿ( (escarg[i]&0xc0)==0x80 ) px++;
            }
            ಯದಿ( i==px ){
              ಯಾವತ್( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          ಪ್ರತಿ(i=j=0; i<px; i++){
            ಯದಿ( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              ಯದಿ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              ಯದಿ( ch==0 ) ಭಙ್ಗ;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              ಯದಿ( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ಅನ್ಯಥಾ ಯದಿ( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ಅನ್ಯಥಾ{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          ಯದಿ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          ಯದಿ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        ಯದಿ( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          ವರ್ಣ *zz;
          ಯದಿ( flag_altform2 && n>0 ){
            zz = sqlite3_str_ಮೌಲ್ಯ(pAccum);
            ಪ್ರತಿ(i=iStart; zz[i]; i++){
              ಯದಿ( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          ಯದಿ( width>len ){
            sqlite3_int64 sp = width-len;
            ಪ್ರತಿಜ್ಞಾ( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (ಪೂರ್ಣಾಙ್ಕ)sp, ' ');
            ಯದಿ( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_ಮೌಲ್ಯ(pAccum);
              zz += iStart;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        ಅನುವರ್ತ;
      }
      ಪಕ್ಷ etESCAPE_q:          /* %q: Escape ' characters */
      ಪಕ್ಷ etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      ಪಕ್ಷ etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        ಪೂರ್ಣಾಙ್ಕ needQuote = 0;
        ವರ್ಣ ch;
        ವರ್ಣ *escarg;
        ವರ್ಣ q;

        ಯದಿ( bArgList ){
          escarg = getTextArg(pArgList);
        }ಅನ್ಯಥಾ{
          escarg = va_arg(ap,ವರ್ಣ*);
        }
        ಯದಿ( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }ಅನ್ಯಥಾ ಯದಿ( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        ಯದಿ( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }ಅನ್ಯಥಾ{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        ಪ್ರತಿ(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          ಯದಿ( ch==q )  n++;
          ಯದಿ( flag_altform2 && (ch&0xc0)==0xc0 ){
            ಯಾವತ್( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        ಯದಿ( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          ಪ್ರತಿ(k=0; k<i; k++){
            ಯದಿ( escarg[k]=='\\' ){
              nBack++;
            }ಅನ್ಯಥಾ ಯದಿ( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          ಯದಿ( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            ಯದಿ( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }ಅನ್ಯಥಾ{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        ಯದಿ( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          ಯದಿ( bufpt==0 ) ಪ್ರತ್ಯಾಗಮ;
        }ಅನ್ಯಥಾ{
          bufpt = buf;
        }
        j = 0;
        ಯದಿ( needQuote ){
          ಯದಿ( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }ಅನ್ಯಥಾ{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        ಯದಿ( flag_alternateform ){
          ಪ್ರತಿ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ಯದಿ( ch==q ){
              bufpt[j++] = ch;
            }ಅನ್ಯಥಾ ಯದಿ( ch=='\\' ){
              bufpt[j++] = '\\';
            }ಅನ್ಯಥಾ ಯದಿ( ((unsigned ವರ್ಣ)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }ಅನ್ಯಥಾ{
          ಪ್ರತಿ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ಯದಿ( ch==q ) bufpt[j++] = ch;
          }
        }
        ಯದಿ( needQuote ){
          bufpt[j++] = '\'';
          ಯದಿ( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        ಗಚ್ಛ adjust_width_for_utf8;
      }
      ಪಕ್ಷ etTOKEN: {
        ಯದಿ( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ಪ್ರತ್ಯಾಗಮ;
        ಯದಿ( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = va_arg(ap,ಅಭಿವ್ಯಕ್ತಿ*);
          ಯದಿ( ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ) && ALWAYS(!ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Int_ಮೌಲ್ಯ)) ){
            sqlite3_str_appendall(pAccum, (ಧ್ರುವ ವರ್ಣ*)p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
            sqlite3_Record_Error_Offset_Of_ಅಭಿವ್ಯಕ್ತಿ(pAccum->db, p_ಅಭಿವ್ಯಕ್ತಿ);
          }
        }ಅನ್ಯಥಾ{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          ಪ್ರತಿಜ್ಞಾ( bArgList==0 );
          ಯದಿ( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ಧ್ರುವ ವರ್ಣ*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ etSRCITEM: {
        SrcItem *pItem;
        ಯದಿ( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ಪ್ರತ್ಯಾಗಮ;
        pItem = va_arg(ap, SrcItem*);
        ಪ್ರತಿಜ್ಞಾ( bArgList==0 );
        ಯದಿ( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }ಅನ್ಯಥಾ ಯದಿ( pItem->zName ){
          ಯದಿ( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }ಅನ್ಯಥಾ ಯದಿ( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }ಅನ್ಯಥಾ ಯದಿ( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          ಪ್ರತಿಜ್ಞಾ( pSel!=0 ); 
          ಯದಿ( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }ಅನ್ಯಥಾ ಯದಿ( pSel->selFlags & SF_Multi_ಮೌಲ್ಯ ){
            ಪ್ರತಿಜ್ಞಾ( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }ಅನ್ಯಥಾ{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ: {
        ಪ್ರತಿಜ್ಞಾ( xtype==etINVALID );
        ಪ್ರತ್ಯಾಗಮ;
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
    ಯದಿ( width>0 ){
      ಯದಿ( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      ಯದಿ( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }ಅನ್ಯಥಾ{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    ಯದಿ( zExtra ){
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
ಶೂನ್ಯ sqlite3RecordErrorByteOffset(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *z){
  ಧ್ರುವ Parse *pParse;
  ಧ್ರುವ ವರ್ಣ *zText;
  ಧ್ರುವ ವರ್ಣ *zEnd;
  ಪ್ರತಿಜ್ಞಾ( z!=0 );
  ಯದಿ( NEVER(db==0) ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( db->errByteOffset!=(-2) ) ಪ್ರತ್ಯಾಗಮ;
  pParse = db->pParse;
  ಯದಿ( NEVER(pParse==0) ) ಪ್ರತ್ಯಾಗಮ;
  zText =pParse->zTail;
  ಯದಿ( NEVER(zText==0) ) ಪ್ರತ್ಯಾಗಮ;
  zEnd = &zText[strlen(zText)];
  ಯದಿ( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (ಪೂರ್ಣಾಙ್ಕ)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
ಶೂನ್ಯ sqlite3_Record_Error_Offset_Of_ಅಭಿವ್ಯಕ್ತಿ(sqlite3 *db, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯಾವತ್( p_ಅಭಿವ್ಯಕ್ತಿ ){
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced|EP_TokenOnly) ) ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_OuterON|EP_InnerON) || p_ಅಭಿವ್ಯಕ್ತಿ->w.iOfst<=0 ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
    }ಅನ್ಯಥಾ{
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL) ) ಪ್ರತ್ಯಾಗಮ;
  db->errByteOffset = p_ಅಭಿವ್ಯಕ್ತಿ->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  ವರ್ಣ *zNew;
  ಪ್ರತಿಜ್ಞಾ( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  ಯದಿ( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    ಪ್ರತ್ಯಾಗಮ p->nAlloc - p->nChar - 1;
  }ಅನ್ಯಥಾ{
    ವರ್ಣ *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    ಯದಿ( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    ಯದಿ( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      ಪ್ರತ್ಯಾಗಮ 0;
    }ಅನ್ಯಥಾ{
      p->nAlloc = (ಪೂರ್ಣಾಙ್ಕ)szNew;
    }
    ಯದಿ( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }ಅನ್ಯಥಾ{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    ಯದಿ( zNew ){
      ಪ್ರತಿಜ್ಞಾ( p->zText!=0 || p->nChar==0 );
      ಯದಿ( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }ಅನ್ಯಥಾ{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( N>=0 && N<=0x7fffffff );
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)N;
}

ಪೂರ್ಣಾಙ್ಕ sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  ಯದಿ( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  ಪ್ರತ್ಯಾಗಮ p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
ಶೂನ್ಯ sqlite3_str_appendchar(sqlite3_str *p, ಪೂರ್ಣಾಙ್ಕ N, ವರ್ಣ c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  ಯದಿ( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯಾವತ್( (N--)>0 ) p->zText[p->nChar++] = c;
}
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, ವರ್ಣ c){
  testcase( p->nChar + N > 0x7fffffff );
  ಯದಿ( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯಾವತ್( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ಧ್ರುವ ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ N){
  N = sqlite3StrAccumEnlarge(p, N);
  ಪ್ರತಿಜ್ಞಾ( z!=0 || N==0 );
  ಯದಿ( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
ಶೂನ್ಯ sqlite3_str_append(sqlite3_str *p, ಧ್ರುವ ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ N){
  ಪ್ರತಿಜ್ಞಾ( z!=0 || N==0 );
  ಪ್ರತಿಜ್ಞಾ( p->zText!=0 || p->nChar==0 || p->accError );
  ಪ್ರತಿಜ್ಞಾ( N>=0 );
  ಪ್ರತಿಜ್ಞಾ( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ಯದಿ( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ಅನ್ಯಥಾ ಯದಿ( N ){
    ಪ್ರತಿಜ್ಞಾ( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3StrAppend64(sqlite3_str *p, ಧ್ರುವ ವರ್ಣ *z, i64 N){
  ಪ್ರತಿಜ್ಞಾ( z!=0 || N==0 );
  ಪ್ರತಿಜ್ಞಾ( p->zText!=0 || p->nChar==0 || p->accError );
  ಪ್ರತಿಜ್ಞಾ( N>=0 );
  ಪ್ರತಿಜ್ಞಾ( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ಯದಿ( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ಅನ್ಯಥಾ ಯದಿ( N ){
    ಪ್ರತಿಜ್ಞಾ( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
ಶೂನ್ಯ sqlite3_str_appendall(sqlite3_str *p, ಧ್ರುವ ವರ್ಣ *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ವರ್ಣ *strAccumFinishRealloc(StrAccum *p){
  ವರ್ಣ *zText;
  ಪ್ರತಿಜ್ಞಾ( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  ಯದಿ( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }ಅನ್ಯಥಾ{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  ಪ್ರತ್ಯಾಗಮ zText;
}
ವರ್ಣ *sqlite3StrAccumFinish(StrAccum *p){
  ಯದಿ( p->zText ){
    p->zText[p->nChar] = 0;
    ಯದಿ( p->mxAlloc>0 && !isMalloced(p) ){
      ಪ್ರತ್ಯಾಗಮ strAccumFinishRealloc(p);
    }
  }
  ಪ್ರತ್ಯಾಗಮ p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
ವರ್ಣ *sqlite3_str_finish(sqlite3_str *p){
  ವರ್ಣ *z;
  ಯದಿ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }ಅನ್ಯಥಾ{
    z = 0;
  }
  ಪ್ರತ್ಯಾಗಮ z;
}

/* Return any error code associated with p */
ಪೂರ್ಣಾಙ್ಕ sqlite3_str_errcode(sqlite3_str *p){
  ಪ್ರತ್ಯಾಗಮ p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
ಪೂರ್ಣಾಙ್ಕ sqlite3_str_length(sqlite3_str *p){
  ಪ್ರತ್ಯಾಗಮ p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
ಶೂನ್ಯ sqlite3_str_truncate(sqlite3_str *p, ಪೂರ್ಣಾಙ್ಕ N){
  ಯದಿ( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
ವರ್ಣ *sqlite3_str_ಮೌಲ್ಯ(sqlite3_str *p){
  ಯದಿ( p==0 || p->nChar==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  p->zText[p->nChar] = 0;
  ಪ್ರತ್ಯಾಗಮ p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
ಶೂನ್ಯ sqlite3_str_reset(StrAccum *p){
  ಯದಿ( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }ಅನ್ಯಥಾ ಯದಿ( p==(sqlite3_str*)&sqlite3OomStr ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
ಶೂನ್ಯ sqlite3_str_free(sqlite3_str *p){
  ಯದಿ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
ಶೂನ್ಯ sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, ವರ್ಣ *zBase, ಪೂರ್ಣಾಙ್ಕ n, ಪೂರ್ಣಾಙ್ಕ mx){
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
  sqlite3_str *p = sqlite3_malloc64(ಪರಿಮಾಣ(*p));
  ಯದಿ( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }ಅನ್ಯಥಾ{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
ವರ್ಣ *sqlite3VMPrintf(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zFormat, va_list ap){
  ವರ್ಣ *z;
  ವರ್ಣ zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, ಪರಿಮಾಣ(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ಯದಿ( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ಪ್ರತ್ಯಾಗಮ z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
ವರ್ಣ *sqlite3MPrintf(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zFormat, ...){
  va_list ap;
  ವರ್ಣ *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ಪ್ರತ್ಯಾಗಮ z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
ವರ್ಣ *sqlite3_vmprintf(ಧ್ರುವ ವರ್ಣ *zFormat, va_list ap){
  ವರ್ಣ *z;
  ವರ್ಣ zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  ಯದಿ( zFormat==0 ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ಯದಿ( sqlite3_initialize() ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, ಪರಿಮಾಣ(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ಪ್ರತ್ಯಾಗಮ z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
ವರ್ಣ *sqlite3_mprintf(ಧ್ರುವ ವರ್ಣ *zFormat, ...){
  va_list ap;
  ವರ್ಣ *z;
#ifndef SQLITE_OMIT_AUTOINIT
  ಯದಿ( sqlite3_initialize() ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  ಪ್ರತ್ಯಾಗಮ z;
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
ವರ್ಣ *sqlite3_vsnprintf(ಪೂರ್ಣಾಙ್ಕ n, ವರ್ಣ *zBuf, ಧ್ರುವ ವರ್ಣ *zFormat, va_list ap){
  StrAccum acc;
  ಯದಿ( n<=0 ) ಪ್ರತ್ಯಾಗಮ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( zBuf==0 || zFormat==0 ) {
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಯದಿ( zBuf ) zBuf[0] = 0;
    ಪ್ರತ್ಯಾಗಮ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  ಪ್ರತ್ಯಾಗಮ zBuf;
}
ವರ್ಣ *sqlite3_snprintf(ಪೂರ್ಣಾಙ್ಕ n, ವರ್ಣ *zBuf, ಧ್ರುವ ವರ್ಣ *zFormat, ...){
  StrAccum acc;
  va_list ap;
  ಯದಿ( n<=0 ) ಪ್ರತ್ಯಾಗಮ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( zBuf==0 || zFormat==0 ) {
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಯದಿ( zBuf ) zBuf[0] = 0;
    ಪ್ರತ್ಯಾಗಮ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  ಪ್ರತ್ಯಾಗಮ zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#ಯದಿ defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#ನೋಚೇದ್ಯದಿ SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#ಅನ್ಯಥಾ
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
ಸ್ಥಿರ ಶೂನ್ಯ render_Log_ಸಂದೇಶ(ಪೂರ್ಣಾಙ್ಕ iErrCode, ಧ್ರುವ ವರ್ಣ *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  ವರ್ಣ z_ಸಂದೇಶ[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_ಸಂದೇಶ, ಪರಿಮಾಣ(z_ಸಂದೇಶ), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
ಶೂನ್ಯ sqlite3_log(ಪೂರ್ಣಾಙ್ಕ iErrCode, ಧ್ರುವ ವರ್ಣ *zFormat, ...){
  va_list ap;                             /* Vararg list */
  ಯದಿ( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_ಸಂದೇಶ(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#ಯದಿ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
ಶೂನ್ಯ sqlite3DebugPrintf(ಧ್ರುವ ವರ್ಣ *zFormat, ...){
  va_list ap;
  StrAccum acc;
  ವರ್ಣ zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, ಪರಿಮಾಣ(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    ಬಾಹ್ಯ ಶೂನ್ಯ SQLITE_OS_TRACE_PROC(ಧ್ರುವ ವರ್ಣ *zBuf, ಪೂರ್ಣಾಙ್ಕ nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, ಪರಿಮಾಣ(zBuf));
  }
#ಅನ್ಯಥಾ
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
ಶೂನ್ಯ sqlite3_str_appendf(StrAccum *p, ಧ್ರುವ ವರ್ಣ *zFormat, ...){
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
ವರ್ಣ *sqlite3RCStrRef(ವರ್ಣ *z){
  RCStr *p = (RCStr*)z;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  p--;
  p->nRCRef++;
  ಪ್ರತ್ಯಾಗಮ z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
ಶೂನ್ಯ sqlite3RCStrUnref(ಶೂನ್ಯ *z){
  RCStr *p = (RCStr*)z;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  p--;
  ಪ್ರತಿಜ್ಞಾ( p->nRCRef>0 );
  ಯದಿ( p->nRCRef>=2 ){
    p->nRCRef--;
  }ಅನ್ಯಥಾ{
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
ವರ್ಣ *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + ಪರಿಮಾಣ(*p) + 1 );
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  p->nRCRef = 1;
  ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
ವರ್ಣ *sqlite3RCStrResize(ವರ್ಣ *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  p--;
  ಪ್ರತಿಜ್ಞಾ( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+ಪರಿಮಾಣ(RCStr)+1);
  ಯದಿ( pNew==0 ){
    sqlite3_free(p);
    ಪ್ರತ್ಯಾಗಮ 0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ*)&pNew[1];
  }
}
