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
#ವ್ಯಾಖ್ಯಾನಿಸು etRADIX       0 /* non-decimal integer types.  %x %o */
#ವ್ಯಾಖ್ಯಾನಿಸು etFLOAT       1 /* Floating point.  %f */
#ವ್ಯಾಖ್ಯಾನಿಸು etEXP         2 /* Exponentional notation. %e and %E */
#ವ್ಯಾಖ್ಯಾನಿಸು etGENERIC     3 /* Floating or exponential, depending on exponent. %g */
#ವ್ಯಾಖ್ಯಾನಿಸು et_ಗಾತ್ರ        4 /* Return number of characters processed so far. %n */
#ವ್ಯಾಖ್ಯಾನಿಸು etSTRING      5 /* Strings. %s */
#ವ್ಯಾಖ್ಯಾನಿಸು etDYNSTRING   6 /* Dynamically allocated strings. %z */
#ವ್ಯಾಖ್ಯಾನಿಸು etPERCENT     7 /* Percent symbol. %% */
#ವ್ಯಾಖ್ಯಾನಿಸು etCHARX       8 /* Characters. %c */
/* The rest are extensions, not normally found in printf() */
#ವ್ಯಾಖ್ಯಾನಿಸು etESCAPE_q    9  /* Strings with '\'' doubled.  %q */
#ವ್ಯಾಖ್ಯಾನಿಸು etESCAPE_Q    10 /* Strings with '\'' doubled and enclosed in '',
                            NULL pointers replaced by SQL NULL.  %Q */
#ವ್ಯಾಖ್ಯಾನಿಸು etTOKEN       11 /* a pointer to a Token structure */
#ವ್ಯಾಖ್ಯಾನಿಸು etSRCITEM     12 /* a pointer to a SrcItem */
#ವ್ಯಾಖ್ಯಾನಿಸು et_ಸೂಚಕ     13 /* The %p conversion */
#ವ್ಯಾಖ್ಯಾನಿಸು etESCAPE_w    14 /* %w -> Strings with '\"' doubled */
#ವ್ಯಾಖ್ಯಾನಿಸು etORDINAL     15 /* %r -> 1st, 2nd, 3rd, 4th, etc.  English only */
#ವ್ಯಾಖ್ಯಾನಿಸು etDECIMAL     16 /* %d or %u, but not %x, %o */
#ವ್ಯಾಖ್ಯಾನಿಸು etESCAPE_j    17 /* %j -> JSON string literal w/o "..." */
#ವ್ಯಾಖ್ಯಾನಿಸು etESCAPE_J    18 /* %J -> JSON string literal with "..." */

#ವ್ಯಾಖ್ಯಾನಿಸು etINVALID     19 /* Any unrecognized conversion type */


/*
** An "etByte" is an 8-bit unsigned value.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ et_info {   /* Information about each format field */
  ಅಕ್ಷರ fmttype;            /* The format field code letter */
  etByte ತಳ;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte ಪೂರ್ವಪ್ರತ್ಯಯ;           /* Offset into aPrefix[] of the prefix string */
  ಅಕ್ಷರ iNxt;               /* Next with same hash, or 0 for end of chain */
} et_info;

/*
** Allowed values for et_info.flags
*/
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_SIGNED    1     /* True if the value to convert is signed */
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_STRING    4     /* Allow infinite precision */

/*
** The table is searched by hash.  In the case of %C where C is the character
** and that character has ASCII value j, then the hash is j%25.
**
** The order of the entries in fmtinfo[] and the hash chain was entered
** manually, but based on the output of the following TCL script:
*/
#ಆದರೆ 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  ಪರಿಶೀಲನೆ $c %c x
  ಗಣ_2 n($c) $x
}
ಗಣ_2 mx [llength [array ಹೆಸರುಗಳು n]]
puts "count: $mx"

ಗಣ_2 mx 25
puts "*********** mx=$mx ************"
ಗಾಗಿ {ಗಣ_2 r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array ಹೆಸರುಗಳು n] {
    ಆದರೆ {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#ಆದರೆ_ಅಂತ್ಯ /***** End of script ********/

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ aDigits[] = "0123456789ABCDEF0123456789abcdef";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ aHex[]    = "0123456789abcdef";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ a_ಪೂರ್ವಪ್ರತ್ಯಯ[] = "-x0\000X0";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ et_info fmtinfo[25] = {
  /*  0 */  {  'd', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  1 */  {  'e',  0, 1, etEXP,        30, 0,  0 },
  /*  2 */  {  'f',  0, 1, etFLOAT,      0,  0,  0 },
  /*  3 */  {  'g',  0, 1, etGENERIC,    30, 0,  0 },
  /*  4 */  {  'j',  0, 0, etESCAPE_j,   0,  0,  0 },  /* Hash: 6 */
  /*  5 */  {  'i', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  6 */  {  'Q',  0, 4, etESCAPE_Q,   0,  0,  4 },
  /*  7 */  {  'p', 16, 0, et_ಸೂಚಕ,    0,  1,  0 },  /* Hash: 12 */
  /*  8 */  {  'S',  0, 0, etSRCITEM,    0,  0,  0 },
  /*  9 */  {  'T',  0, 0, etTOKEN,      0,  0,  0 },
  /* 10 */  {  'n',  0, 0, et_ಗಾತ್ರ,       0,  0,  0 },
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
ಶೂನ್ಯ sqlite3_Str_Accum_ಗಣ_Error(StrAccum *p, u8 eError){
  ಪ್ರತಿಜ್ಞಾ( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  ಆದರೆ( p->mxAlloc ) sqlite3_str_reset(p);
  ಆದರೆ( eError==SQLITE_TOOBIG ) sqlite3_Error_ಗಮ್ಯ_Parser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ಸ್ಥಿರ sqlite3_int64 getIntArg(PrintfArguments *p){
  ಆದರೆ( p->nArg<=p->n_ಬಳಸಿದ ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_int64(p->apArg[p->n_ಬಳಸಿದ++]);
}
ಸ್ಥಿರ ದ್ವಿಗುಣ getDoubleArg(PrintfArguments *p){
  ಆದರೆ( p->nArg<=p->n_ಬಳಸಿದ ) ಹಿಂತಿರುಗಿ 0.0;
  ಹಿಂತಿರುಗಿ sqlite3_ಮೌಲ್ಯ_double(p->apArg[p->n_ಬಳಸಿದ++]);
}
ಸ್ಥಿರ ಅಕ್ಷರ *get_ಪಠ್ಯ_Arg(PrintfArguments *p){
  ಆದರೆ( p->nArg<=p->n_ಬಳಸಿದ ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p->apArg[p->n_ಬಳಸಿದ++]);
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
ಸ್ಥಿರ ಅಕ್ಷರ *printf_ತಾತ್ಕಾಲಿಕ_Buf(sqlite3_str *pAccum, sqlite3_int64 n){
  ಅಕ್ಷರ *z;
  ಆದರೆ( pAccum->accError ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3_Str_Accum_ಗಣ_Error(pAccum, SQLITE_TOOBIG);
    ಹಿಂತಿರುಗಿ 0;
  }
  z = sqlite3_malloc(n);
  ಆದರೆ( z==0 ){
    sqlite3_Str_Accum_ಗಣ_Error(pAccum, SQLITE_NOMEM);
  }
  ಹಿಂತಿರುಗಿ z;
}

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be something smaller, if desired.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_PRINT_BUF_ಗಾತ್ರ
# define SQLITE_PRINT_BUF_ಗಾತ್ರ 70
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು etBUFSIZE SQLITE_PRINT_BUF_ಗಾತ್ರ  /* Size of the output buffer */

/*
** Hard limit on the precision of floating-point conversions to
** the minimum of SQLITE_PRINTF_PRECISION_LIMIT and 100 million.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ
# ಆದರೆ SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ<100000000
#   define SQLITE_FP_PRECISION_ಎಲ್ಲೆ SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ
# ಇಲ್ಲದಿದ್ದರೆ
#   define SQLITE_FP_PRECISION_ಎಲ್ಲೆ 100000000
# endif
#ಇಲ್ಲದಿದ್ದರೆ
# define SQLITE_FP_PRECISION_ಎಲ್ಲೆ 100000000
#ಆದರೆ_ಅಂತ್ಯ

/* Forward reference */
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3StrAppend64(sqlite3_str *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, i64 N);
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, ಅಕ್ಷರ c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
ಶೂನ್ಯ sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  ಪೂರ್ಣಾಂಕ c;                     /* Next character in the format string */
  ಅಕ್ಷರ *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 ಉದ್ದಳತೆ;                /* Length of the field */
  ಪೂರ್ಣಾಂಕ idx;                   /* A general purpose loop counter */
  i64 ಅಗಲ;                 /* Width of the current field */
  etByte flag_leftjustify;   /* True if "-" flag is present */
  etByte flag_ಪೂರ್ವಪ್ರತ್ಯಯ;        /* '+' or ' ' or 0 for prefix */
  etByte flag_alternateform; /* True if "#" flag is present */
  etByte flag_altform2;      /* True if "!" flag is present */
  etByte flag_zeropad;       /* True if field width constant starts with zero */
  etByte flag_long;          /* 1 for the "l" flag, 2 for "ll", 0 by default */
  etByte ಮುಗಿದ;               /* Loop termination flag */
  etByte cThousand;          /* Thousands separator for %d and %u */
  etByte xtype = etINVALID;  /* Conversion paradigm */
  u8 bArgList;               /* True for SQLITE_PRINTF_SQLFUNC */
  ಅಕ್ಷರ ಪೂರ್ವಪ್ರತ್ಯಯ;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  ದ್ವಿಗುಣ realvalue;          /* Value for real types */
  ಸ್ಥಿರಾಂಕ et_info *infop;      /* Pointer to the appropriate info structure */
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;                /* Rendering buffer */
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು;                  /* Size of the rendering buffer */
  ಅಕ್ಷರ *zExtra = 0;          /* Malloced memory used by some conversion */
  ಪೂರ್ಣಾಂಕ ಘಾತಾಂಕ, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  ಅಕ್ಷರ buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  ಪ್ರತಿಜ್ಞಾ( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  ಆದರೆ( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    bArgList = 0;
  }
  ಗಾಗಿ(; (c=(*fmt))!=0; ++fmt){
    ಆದರೆ( c!='%' ){
      bufpt = (ಅಕ್ಷರ *)fmt;
#ಆದರೆ HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#ಇಲ್ಲದಿದ್ದರೆ
      fmt = strchr(fmt, '%');
      ಆದರೆ( fmt==0 ){
        fmt = bufpt + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(bufpt);
      }
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      ಆದರೆ( *fmt==0 ) ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      ನಿಲ್ಲಿಸು;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_ಪೂರ್ವಪ್ರತ್ಯಯ = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    ಮುಗಿದ = 0;
    ಅಗಲ = 0;
    flag_long = 0;
    precision = -1;
    ಮಾಡು{
      ಆಯ್ಕೆ( c ){
        ಸ್ಥಿತಿ '-':   flag_leftjustify = 1;     ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ '+':   flag_ಪೂರ್ವಪ್ರತ್ಯಯ = '+';        ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ ' ':   flag_ಪೂರ್ವಪ್ರತ್ಯಯ = ' ';        ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ '#':   flag_alternateform = 1;   ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ '!':   flag_altform2 = 1;        ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ '0':   flag_zeropad = 1;         ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ ',':   cThousand = ',';          ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:    ಮುಗಿದ = 1;                 ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 'l': {
          flag_long = 1;
          c = *++fmt;
          ಆದರೆ( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          ಮುಗಿದ = 1;
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ '1': ಸ್ಥಿತಿ '2': ಸ್ಥಿತಿ '3': ಸ್ಥಿತಿ '4': ಸ್ಥಿತಿ '5':
        ಸ್ಥಿತಿ '6': ಸ್ಥಿತಿ '7': ಸ್ಥಿತಿ '8': ಸ್ಥಿತಿ '9': {
          ಚಿಹ್ನೆರಹಿತ wx = c - '0';
          ಆಗ( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          ಅಗಲ = wx & 0x7fffffff;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ
          ಆದರೆ( ಅಗಲ>SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ ){
            ಅಗಲ = SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ;
          }
#ಆದರೆ_ಅಂತ್ಯ
          ಆದರೆ( c!='.' && c!='l' ){
            ಮುಗಿದ = 1;
          }ಇಲ್ಲದಿದ್ದರೆ{
            fmt--;
          }
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ '*': {
          ಆದರೆ( bArgList ){
            ಅಗಲ = (ಪೂರ್ಣಾಂಕ)getIntArg(pArgList);
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಅಗಲ = va_arg(ap,ಪೂರ್ಣಾಂಕ);
          }
          ಆದರೆ( ಅಗಲ<0 ){
            flag_leftjustify = 1;
            ಅಗಲ = ಅಗಲ >= -2147483647 ? -ಅಗಲ : 0;
          }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ
          ಆದರೆ( ಅಗಲ>SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ ){
            ಅಗಲ = SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ;
          }
#ಆದರೆ_ಅಂತ್ಯ
          ಆದರೆ( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            ಮುಗಿದ = 1;
          }
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ '.': {
          c = *++fmt;
          ಆದರೆ( c=='*' ){
            ಆದರೆ( bArgList ){
              precision = (ಪೂರ್ಣಾಂಕ)getIntArg(pArgList);
            }ಇಲ್ಲದಿದ್ದರೆ{
              precision = va_arg(ap,ಪೂರ್ಣಾಂಕ);
            }
            ಆದರೆ( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಚಿಹ್ನೆರಹಿತ px = 0;
            ಆಗ( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ
          ಆದರೆ( precision>SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ ){
            precision = SQLITE_PRINTF_PRECISION_ಎಲ್ಲೆ;
          }
#ಆದರೆ_ಅಂತ್ಯ
          ಆದರೆ( c=='l' ){
            --fmt;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಮುಗಿದ = 1;
          }
          ನಿಲ್ಲಿಸು;
        }
      }
    }ಆಗ( !ಮುಗಿದ && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    ಗಾಗಿ(idx=0; idx<Array_ಗಾತ್ರ(fmtinfo); idx++){
      ಆದರೆ( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        ನಿಲ್ಲಿಸು;
      }
    }
#ಇಲ್ಲದಿದ್ದರೆ
    /* Fast hash-table lookup */
    ಪ್ರತಿಜ್ಞಾ( Array_ಗಾತ್ರ(fmtinfo)==25 );
    idx = ((ಚಿಹ್ನೆರಹಿತ)c) % 25;
    ಆದರೆ( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }ಇಲ್ಲದಿದ್ದರೆ{
      infop = &fmtinfo[0];
      xtype = etINVALID;
    }
#ಆದರೆ_ಅಂತ್ಯ

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
    ಪ್ರತಿಜ್ಞಾ( ಅಗಲ>=0 );
    ಪ್ರತಿಜ್ಞಾ( precision>=(-1) );
    ಆಯ್ಕೆ( xtype ){
      ಸ್ಥಿತಿ et_ಸೂಚಕ:
        flag_long = ಗಾತ್ರ(ಅಕ್ಷರ*)==ಗಾತ್ರ(i64) ? 2 :
                     ಗಾತ್ರ(ಅಕ್ಷರ*)==ಗಾತ್ರ(ದೀರ್ಘ ಪೂರ್ಣಾಂಕ) ? 1 : 0;
        /* no break */ deliberate_fall_through
      ಸ್ಥಿತಿ etORDINAL:
      ಸ್ಥಿತಿ etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      ಸ್ಥಿತಿ etDECIMAL:
        ಆದರೆ( infop->flags & FLAG_SIGNED ){
          i64 v;
          ಆದರೆ( bArgList ){
            v = getIntArg(pArgList);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flag_long ){
            ಆದರೆ( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }ಇಲ್ಲದಿದ್ದರೆ{
              v = va_arg(ap,ದೀರ್ಘ ಪೂರ್ಣಾಂಕ);
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            v = va_arg(ap,ಪೂರ್ಣಾಂಕ);
          }
          ಆದರೆ( v<0 ){
            testcase( v==ಅತಿಚಿಕ್ಕ_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            ಪೂರ್ವಪ್ರತ್ಯಯ = '-';
          }ಇಲ್ಲದಿದ್ದರೆ{
            longvalue = v;
            ಪೂರ್ವಪ್ರತ್ಯಯ = flag_ಪೂರ್ವಪ್ರತ್ಯಯ;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಆದರೆ( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flag_long ){
            ಆದರೆ( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }ಇಲ್ಲದಿದ್ದರೆ{
              longvalue = va_arg(ap,ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಪೂರ್ಣಾಂಕ);
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            longvalue = va_arg(ap,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
          }
          ಪೂರ್ವಪ್ರತ್ಯಯ = 0;
        }

#ಆದರೆ WHERETRACE_ENABLED
        ಆದರೆ( xtype==et_ಸೂಚಕ && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ TREETRACE_ENABLED
        ಆದರೆ( xtype==et_ಸೂಚಕ && sqlite3_ವೃಕ್ಷ_Trace & 0x100000 ) longvalue = 0;
#ಆದರೆ_ಅಂತ್ಯ

        ಆದರೆ( longvalue==0 ) flag_alternateform = 0;
        ಆದರೆ( flag_zeropad && precision<ಅಗಲ-(ಪೂರ್ವಪ್ರತ್ಯಯ!=0) ){
          precision = ಅಗಲ-(ಪೂರ್ವಪ್ರತ್ಯಯ!=0);
        }
        ಆದರೆ( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          n_ಹೊರಹೋಗು = etBUFSIZE;
          z_ಹೊರಹೋಗು = buf;
        }ಇಲ್ಲದಿದ್ದರೆ{
          u64 n;
          n = (u64)precision + 10;
          ಆದರೆ( cThousand ) n += precision/3;
          z_ಹೊರಹೋಗು = zExtra = printf_ತಾತ್ಕಾಲಿಕ_Buf(pAccum, n);
          ಆದರೆ( z_ಹೊರಹೋಗು==0 ) ಹಿಂತಿರುಗಿ;
          n_ಹೊರಹೋಗು = (ಪೂರ್ಣಾಂಕ)n;
        }
        bufpt = &z_ಹೊರಹೋಗು[n_ಹೊರಹೋಗು-1];
        ಆದರೆ( xtype==etORDINAL ){
          ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ zOrd[] = "thstndrd";
          ಪೂರ್ಣಾಂಕ x = (ಪೂರ್ಣಾಂಕ)(longvalue % 10);
          ಆದರೆ( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cset = &aDigits[infop->charset];
          u8 ತಳ = infop->ತಳ;
          ಮಾಡು{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%ತಳ];
            longvalue = longvalue/ತಳ;
          }ಆಗ( longvalue>0 );
        }
        ಉದ್ದಳತೆ = &z_ಹೊರಹೋಗು[n_ಹೊರಹೋಗು-1] - bufpt;
        ಆದರೆ( precision>ಉದ್ದಳತೆ ){                         /* zero pad */
          i64 nn = precision-ಉದ್ದಳತೆ;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          ಉದ್ದಳತೆ = precision;
        }
        ಆದರೆ( cThousand ){
          i64 nn = (ಉದ್ದಳತೆ - 1)/3;  /* Number of "," to insert */
          i64 ix = (ಉದ್ದಳತೆ - 1)%3 + 1;
          ಪೂರ್ಣಾಂಕ ii;
          bufpt -= nn;
          ಗಾಗಿ(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            ಆದರೆ( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        ಆದರೆ( ಪೂರ್ವಪ್ರತ್ಯಯ ) *(--bufpt) = ಪೂರ್ವಪ್ರತ್ಯಯ;               /* Add sign */
        ಆದರೆ( flag_alternateform && infop->ಪೂರ್ವಪ್ರತ್ಯಯ ){      /* Add "0" or "0x" */
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pre;
          ಅಕ್ಷರ x;
          pre = &a_ಪೂರ್ವಪ್ರತ್ಯಯ[infop->ಪೂರ್ವಪ್ರತ್ಯಯ];
          ಗಾಗಿ(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        ಉದ್ದಳತೆ = &z_ಹೊರಹೋಗು[n_ಹೊರಹೋಗು-1] - bufpt;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ etFLOAT:
      ಸ್ಥಿತಿ etEXP:
      ಸ್ಥಿತಿ etGENERIC: {
        FpDecode s;
        ಪೂರ್ಣಾಂಕ i_ಸುತ್ತು;
        ಪೂರ್ಣಾಂಕ j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        ಆದರೆ( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }ಇಲ್ಲದಿದ್ದರೆ{
          realvalue = va_arg(ap,ದ್ವಿಗುಣ);
        }
        ಆದರೆ( precision<0 ) precision = 6;         /* Set default precision */
        ಆದರೆ( precision>SQLITE_FP_PRECISION_ಎಲ್ಲೆ ){
          precision = SQLITE_FP_PRECISION_ಎಲ್ಲೆ;
        }
        ಆದರೆ( xtype==etFLOAT ){
          i_ಸುತ್ತು = -precision;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( xtype==etGENERIC ){
          ಆದರೆ( precision==0 ) precision = 1;
          i_ಸುತ್ತು = precision;
        }ಇಲ್ಲದಿದ್ದರೆ{
          i_ಸುತ್ತು = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, i_ಸುತ್ತು, flag_altform2 ? 20 : 16);
        ಆದರೆ( s.isSpecial ){
          ಆದರೆ( s.isSpecial==2 ){
            ಆದರೆ( flag_zeropad ){
              bufpt = "null";
              ಉದ್ದಳತೆ = 4;
            }ಇಲ್ಲದಿದ್ದರೆ{
              bufpt = "NaN";
              ಉದ್ದಳತೆ = 3;
            }
            ನಿಲ್ಲಿಸು;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಸ್ಮರಣೆ_ನಕಲು(buf, "-Inf", 5);
            bufpt = buf;
            ಆದರೆ( s.sign=='-' ){
              /* no-op */
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flag_ಪೂರ್ವಪ್ರತ್ಯಯ ){
              buf[0] = flag_ಪೂರ್ವಪ್ರತ್ಯಯ;
            }ಇಲ್ಲದಿದ್ದರೆ{
              bufpt++;
            }
            ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(bufpt);
            ನಿಲ್ಲಿಸು;
          }
        }
        ಆದರೆ( s.sign=='-' ){
          ಆದರೆ( flag_alternateform
           && !flag_ಪೂರ್ವಪ್ರತ್ಯಯ
           && xtype==etFLOAT
           && s.iDP<=i_ಸುತ್ತು
          ){
            /* Suppress the minus sign if all of the following are true:
            **   *  The value displayed is zero
            **   *  The '#' flag is used
            **   *  The '+' flag is not used, and
            **   *  The format is %f
            */
            ಪೂರ್ವಪ್ರತ್ಯಯ = 0;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪೂರ್ವಪ್ರತ್ಯಯ = '-';
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪೂರ್ವಪ್ರತ್ಯಯ = flag_ಪೂರ್ವಪ್ರತ್ಯಯ;
        }

        ಘಾತಾಂಕ = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        ಆದರೆ( xtype==etGENERIC ){
          ಪ್ರತಿಜ್ಞಾ( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          ಆದರೆ( ಘಾತಾಂಕ<-4 || ಘಾತಾಂಕ>precision ){
            xtype = etEXP;
          }ಇಲ್ಲದಿದ್ದರೆ{
            precision = precision - ಘಾತಾಂಕ;
            xtype = etFLOAT;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          flag_rtz = flag_altform2;
        }
        ಆದರೆ( xtype==etEXP ){
          e2 = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)ಅಗಲ+10;
        ಆದರೆ( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        ಆದರೆ( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          ಆದರೆ( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            ಆದರೆ( bufpt==0 ){
              sqlite3_Str_Accum_ಗಣ_Error(pAccum, SQLITE_NOMEM);
              ಹಿಂತಿರುಗಿ;
            }
            zExtra = bufpt;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            ಅಗಲ = ಉದ್ದಳತೆ = 0;
            ನಿಲ್ಲಿಸು;
          }ಇಲ್ಲದಿದ್ದರೆ{
            bufpt = pAccum->z_ಪಠ್ಯ + pAccum->nChar;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          bufpt = pAccum->z_ಪಠ್ಯ + pAccum->nChar;
        }
        z_ಹೊರಹೋಗು = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        ಆದರೆ( ಪೂರ್ವಪ್ರತ್ಯಯ ){
          *(bufpt++) = ಪೂರ್ವಪ್ರತ್ಯಯ;
        }
        /* Digits prior to the decimal point */
        j = 0;
        ಪ್ರತಿಜ್ಞಾ( s.n>0 );
        ಆದರೆ( e2<0 ){
          *(bufpt++) = '0';
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( cThousand ){
          ಗಾಗಿ(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            ಆದರೆ( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          j = e2+1;
          ಆದರೆ( j>s.n ) j = s.n;
          ಸ್ಮರಣೆ_ನಕಲು(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          ಆದರೆ( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        ಆದರೆ( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        ಆದರೆ( e2<(-1) && precision>0 ){
          ಪೂರ್ಣಾಂಕ nn = -1-e2;
          ಆದರೆ( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        ಆದರೆ( precision>0 ){
          ಪೂರ್ಣಾಂಕ nn = s.n - j;
          ಆದರೆ( NEVER(nn>precision) ) nn = precision;
          ಆದರೆ( nn>0 ){
            ಸ್ಮರಣೆ_ನಕಲು(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          ಆದರೆ( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        ಆದರೆ( flag_rtz && flag_dp ){
          ಆಗ( bufpt[-1]=='0' ) *(--bufpt) = 0;
          ಪ್ರತಿಜ್ಞಾ( bufpt>z_ಹೊರಹೋಗು );
          ಆದರೆ( bufpt[-1]=='.' ){
            ಆದರೆ( flag_altform2 ){
              *(bufpt++) = '0';
            }ಇಲ್ಲದಿದ್ದರೆ{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        ಆದರೆ( xtype==etEXP ){
          ಘಾತಾಂಕ = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          ಆದರೆ( ಘಾತಾಂಕ<0 ){
            *(bufpt++) = '-'; ಘಾತಾಂಕ = -ಘಾತಾಂಕ;
          }ಇಲ್ಲದಿದ್ದರೆ{
            *(bufpt++) = '+';
          }
          ಆದರೆ( ಘಾತಾಂಕ>=100 ){
            *(bufpt++) = (ಅಕ್ಷರ)((ಘಾತಾಂಕ/100)+'0');        /* 100's digit */
            ಘಾತಾಂಕ %= 100;
          }
          *(bufpt++) = (ಅಕ್ಷರ)(ಘಾತಾಂಕ/10+'0');             /* 10's digit */
          *(bufpt++) = (ಅಕ್ಷರ)(ಘಾತಾಂಕ%10+'0');             /* 1's digit */
        }

        ಉದ್ದಳತೆ = bufpt - z_ಹೊರಹೋಗು;
        ಪ್ರತಿಜ್ಞಾ( ಉದ್ದಳತೆ <= szBufNeeded );
        ಆದರೆ( ಉದ್ದಳತೆ<ಅಗಲ ){
          i64 nPad = ಅಗಲ - ಉದ್ದಳತೆ;
          ಆದರೆ( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !flag_zeropad ){
            memmove(z_ಹೊರಹೋಗು+nPad, z_ಹೊರಹೋಗು, ಉದ್ದಳತೆ);
            memset(z_ಹೊರಹೋಗು, ' ', nPad);
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪೂರ್ಣಾಂಕ adj = ಪೂರ್ವಪ್ರತ್ಯಯ!=0;
            memmove(z_ಹೊರಹೋಗು+nPad+adj, z_ಹೊರಹೋಗು+adj, ಉದ್ದಳತೆ-adj);
            memset(z_ಹೊರಹೋಗು+adj, '0', nPad);
          }
          ಉದ್ದಳತೆ = ಅಗಲ;
        }

        ಆದರೆ( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          ಪ್ರತಿಜ್ಞಾ( pAccum->nChar + ಉದ್ದಳತೆ < SMXV(pAccum->nChar) );
          pAccum->nChar += ಉದ್ದಳತೆ;
          z_ಹೊರಹೋಗು[ಉದ್ದಳತೆ] = 0;
          ಮುಂದುವರಿ;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          ನಿಲ್ಲಿಸು;
        }
      }
      ಸ್ಥಿತಿ et_ಗಾತ್ರ:
        ಆದರೆ( !bArgList ){
          *(va_arg(ap,ಪೂರ್ಣಾಂಕ*)) = pAccum->nChar;
        }
        ಉದ್ದಳತೆ = ಅಗಲ = 0;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        ಉದ್ದಳತೆ = 1;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ etCHARX:
        ಆದರೆ( bArgList ){
          bufpt = get_ಪಠ್ಯ_Arg(pArgList);
          ಉದ್ದಳತೆ = 1;
          ಆದರೆ( bufpt ){
            buf[0] = c = *(bufpt++);
            ಆದರೆ( (c&0xc0)==0xc0 ){
              ಆಗ( ಉದ್ದಳತೆ<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[ಉದ್ದಳತೆ++] = *(bufpt++);
              }
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            buf[0] = 0;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ch = va_arg(ap,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
          ಉದ್ದಳತೆ = sqlite3AppendOneUtf8Character(buf, ch);
        }
        ಆದರೆ( precision>1 ){
          i64 n_ಪೂರ್ವ = 1;
          ಅಗಲ -= precision-1;
          ಆದರೆ( ಅಗಲ>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, ಅಗಲ-1, ' ');
            ಅಗಲ = 0;
          }
          sqlite3StrAppend64(pAccum, buf, ಉದ್ದಳತೆ);
          precision--;
          ಆಗ( precision > 1 ){
            i64 n_Copy_ಬೈಟುಗಳು;
            ಆದರೆ( n_ಪೂರ್ವ > precision-1 ) n_ಪೂರ್ವ = precision - 1;
            n_Copy_ಬೈಟುಗಳು = ಉದ್ದಳತೆ*n_ಪೂರ್ವ;
            ಆದರೆ( sqlite3StrAccumEnlargeIfNeeded(pAccum, n_Copy_ಬೈಟುಗಳು) ){
              ನಿಲ್ಲಿಸು;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->z_ಪಠ್ಯ[pAccum->nChar-n_Copy_ಬೈಟುಗಳು], n_Copy_ಬೈಟುಗಳು);
            precision -= n_ಪೂರ್ವ;
            n_ಪೂರ್ವ *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        ಹೋಗು adjust_ಅಗಲ_for_utf8;
      ಸ್ಥಿತಿ etSTRING:
      ಸ್ಥಿತಿ etDYNSTRING:
        ಆದರೆ( bArgList ){
          bufpt = get_ಪಠ್ಯ_Arg(pArgList);
          xtype = etSTRING;
        }ಇಲ್ಲದಿದ್ದರೆ{
          bufpt = va_arg(ap,ಅಕ್ಷರ*);
        }
        ಆದರೆ( bufpt==0 ){
          bufpt = "";
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( xtype==etDYNSTRING ){
          ಆದರೆ( pAccum->nChar==0
           && pAccum->mxAlloc
           && ಅಗಲ==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            ಪ್ರತಿಜ್ಞಾ( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->z_ಪಠ್ಯ = bufpt;
            pAccum->nAlloc = sqlite3_Db_Malloc_ಗಾತ್ರ(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            ಉದ್ದಳತೆ = 0;
            ನಿಲ್ಲಿಸು;
          }
          zExtra = bufpt;
        }
        ಆದರೆ( precision>=0 ){
          ಆದರೆ( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)bufpt;
            ಆಗ( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            ಉದ್ದಳತೆ = z - (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)bufpt;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಗಾಗಿ(ಉದ್ದಳತೆ=0; ಉದ್ದಳತೆ<precision && bufpt[ಉದ್ದಳತೆ]; ಉದ್ದಳತೆ++){}
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(bufpt);
        }
      adjust_ಅಗಲ_for_utf8:
        ಆದರೆ( flag_altform2 && ಅಗಲ>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = ಉದ್ದಳತೆ - 1;
          ಆಗ( ii>=0 ) ಆದರೆ( (bufpt[ii--] & 0xc0)==0x80 ) ಅಗಲ++;
        }
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      ಸ್ಥಿತಿ etESCAPE_J: {         /* %J: Generate a JSON string literal */
        ಅಕ್ಷರ *escarg;
        i64 i, j, px, i_ಆರಂಭ;
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ch;

        ಆದರೆ( bArgList ){
          escarg = get_ಪಠ್ಯ_Arg(pArgList);
        }ಇಲ್ಲದಿದ್ದರೆ{
          escarg = va_arg(ap,ಅಕ್ಷರ*);
        }
        i_ಆರಂಭ = sqlite3_str_ಉದ್ದಳತೆ(pAccum);
        ಆದರೆ( escarg==0 ){
          ಆದರೆ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಆದರೆ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          ಆದರೆ( px<0 ){
            px = 0x7fffffff;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            ಗಾಗಿ(i=0; i<px && escarg[i]; i++){
              ಆದರೆ( (escarg[i]&0xc0)==0x80 ) px++;
            }
            ಆದರೆ( i==px ){
              ಆಗ( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          ಗಾಗಿ(i=j=0; i<px; i++){
            ಆದರೆ( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              ಆದರೆ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              ಆದರೆ( ch==0 ) ನಿಲ್ಲಿಸು;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              ಆದರೆ( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ಇಲ್ಲದಿದ್ದರೆ{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          ಆದರೆ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          ಆದರೆ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        ಆದರೆ( ಅಗಲ>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_ಉದ್ದಳತೆ(pAccum) - i_ಆರಂಭ;
          sqlite3_int64 len = n;
          ಅಕ್ಷರ *zz;
          ಆದರೆ( flag_altform2 && n>0 ){
            zz = sqlite3_str_ಮೌಲ್ಯ(pAccum);
            ಗಾಗಿ(i=i_ಆರಂಭ; zz[i]; i++){
              ಆದರೆ( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          ಆದರೆ( ಅಗಲ>len ){
            sqlite3_int64 sp = ಅಗಲ-len;
            ಪ್ರತಿಜ್ಞಾ( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (ಪೂರ್ಣಾಂಕ)sp, ' ');
            ಆದರೆ( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_ಮೌಲ್ಯ(pAccum);
              zz += i_ಆರಂಭ;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        ಮುಂದುವರಿ;
      }
      ಸ್ಥಿತಿ etESCAPE_q:          /* %q: Escape ' characters */
      ಸ್ಥಿತಿ etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      ಸ್ಥಿತಿ etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        ಪೂರ್ಣಾಂಕ needQuote = 0;
        ಅಕ್ಷರ ch;
        ಅಕ್ಷರ *escarg;
        ಅಕ್ಷರ q;

        ಆದರೆ( bArgList ){
          escarg = get_ಪಠ್ಯ_Arg(pArgList);
        }ಇಲ್ಲದಿದ್ದರೆ{
          escarg = va_arg(ap,ಅಕ್ಷರ*);
        }
        ಆದರೆ( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        ಆದರೆ( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        ಗಾಗಿ(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          ಆದರೆ( ch==q )  n++;
          ಆದರೆ( flag_altform2 && (ch&0xc0)==0xc0 ){
            ಆಗ( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        ಆದರೆ( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 n_ಹಿಂಭಾಗ = 0;
          i64 nCtrl = 0;
          ಗಾಗಿ(k=0; k<i; k++){
            ಆದರೆ( escarg[k]=='\\' ){
              n_ಹಿಂಭಾಗ++;
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          ಆದರೆ( nCtrl || xtype==etESCAPE_q ){
            n += n_ಹಿಂಭಾಗ + 5*nCtrl;
            ಆದರೆ( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        ಆದರೆ( n>etBUFSIZE ){
          bufpt = zExtra = printf_ತಾತ್ಕಾಲಿಕ_Buf(pAccum, n);
          ಆದರೆ( bufpt==0 ) ಹಿಂತಿರುಗಿ;
        }ಇಲ್ಲದಿದ್ದರೆ{
          bufpt = buf;
        }
        j = 0;
        ಆದರೆ( needQuote ){
          ಆದರೆ( needQuote==2 ){
            ಸ್ಮರಣೆ_ನಕಲು(&bufpt[j], "unistr('", 8);
            j += 8;
          }ಇಲ್ಲದಿದ್ದರೆ{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        ಆದರೆ( flag_alternateform ){
          ಗಾಗಿ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ಆದರೆ( ch==q ){
              bufpt[j++] = ch;
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ch=='\\' ){
              bufpt[j++] = '\\';
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಗಾಗಿ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ಆದರೆ( ch==q ) bufpt[j++] = ch;
          }
        }
        ಆದರೆ( needQuote ){
          bufpt[j++] = '\'';
          ಆದರೆ( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        ಉದ್ದಳತೆ = j;
        ಹೋಗು adjust_ಅಗಲ_for_utf8;
      }
      ಸ್ಥಿತಿ etTOKEN: {
        ಆದರೆ( (pAccum->printfFlags & SQLITE_PRINTF_ಆಂತರಿಕ)==0 ) ಹಿಂತಿರುಗಿ;
        ಆದರೆ( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = va_arg(ap,ಅಭಿವ್ಯಕ್ತಿ*);
          ಆದರೆ( ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ) && ALWAYS(!ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Int_ಮೌಲ್ಯ)) ){
            sqlite3_str_appendall(pAccum, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
            sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(pAccum->db, p_ಅಭಿವ್ಯಕ್ತಿ);
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          ಪ್ರತಿಜ್ಞಾ( bArgList==0 );
          ಆದರೆ( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)pToken->z, pToken->n);
            sqlite3_ದಾಖಲೆ_Error_Byte_ಸ್ಥಾನಾಂತರ(pAccum->db, pToken->z);
          }
        }
        ಉದ್ದಳತೆ = ಅಗಲ = 0;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ etSRCITEM: {
        Src_ಅಂಶ *p_ಅಂಶ;
        ಆದರೆ( (pAccum->printfFlags & SQLITE_PRINTF_ಆಂತರಿಕ)==0 ) ಹಿಂತಿರುಗಿ;
        p_ಅಂಶ = va_arg(ap, Src_ಅಂಶ*);
        ಪ್ರತಿಜ್ಞಾ( bArgList==0 );
        ಆದರೆ( p_ಅಂಶ->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, p_ಅಂಶ->zAlias);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಂಶ->zName ){
          ಆದರೆ( p_ಅಂಶ->fg.fixedSchema==0
           && p_ಅಂಶ->fg.isSubquery==0
           && p_ಅಂಶ->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, p_ಅಂಶ->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, p_ಅಂಶ->zName);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಂಶ->zAlias ){
          sqlite3_str_appendall(pAccum, p_ಅಂಶ->zAlias);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ALWAYS(p_ಅಂಶ->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = p_ಅಂಶ->u4.pSubq->pSelect;
          ಪ್ರತಿಜ್ಞಾ( pSel!=0 ); 
          ಆದರೆ( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pSel->selFlags & SF_Multi_ಮೌಲ್ಯ ){
            ಪ್ರತಿಜ್ಞಾ( !p_ಅಂಶ->fg.isTabFunc && !p_ಅಂಶ->fg.is_ಸೂಚ್ಯಂಕಿತ_By );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                p_ಅಂಶ->u1.n_ಅಡ್ಡಸಾಲು);
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        ಉದ್ದಳತೆ = ಅಗಲ = 0;
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಪ್ರತಿಜ್ಞಾ( xtype==etINVALID );
        ಹಿಂತಿರುಗಿ;
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
    ಅಗಲ -= ಉದ್ದಳತೆ;
    ಆದರೆ( ಅಗಲ>0 ){
      ಆದರೆ( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, ಅಗಲ, ' ');
      sqlite3StrAppend64(pAccum, bufpt, ಉದ್ದಳತೆ);
      ಆದರೆ( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, ಅಗಲ, ' ');
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3StrAppend64(pAccum, bufpt, ಉದ್ದಳತೆ);
    }

    ಆದರೆ( zExtra ){
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
ಶೂನ್ಯ sqlite3_ದಾಖಲೆ_Error_Byte_ಸ್ಥಾನಾಂತರ(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಸ್ಥಿರಾಂಕ Parse *pParse;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ;
  ಪ್ರತಿಜ್ಞಾ( z!=0 );
  ಆದರೆ( NEVER(db==0) ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( db->err_Byte_ಸ್ಥಾನಾಂತರ!=(-2) ) ಹಿಂತಿರುಗಿ;
  pParse = db->pParse;
  ಆದರೆ( NEVER(pParse==0) ) ಹಿಂತಿರುಗಿ;
  z_ಪಠ್ಯ =pParse->z_ಬಾಲ;
  ಆದರೆ( NEVER(z_ಪಠ್ಯ==0) ) ಹಿಂತಿರುಗಿ;
  z_ಕೊನೆ = &z_ಪಠ್ಯ[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಪಠ್ಯ)];
  ಆದರೆ( SQLITE_WITHIN(z,z_ಪಠ್ಯ,z_ಕೊನೆ) ){
    db->err_Byte_ಸ್ಥಾನಾಂತರ = (ಪೂರ್ಣಾಂಕ)(z-z_ಪಠ್ಯ);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
ಶೂನ್ಯ sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆಗ( p_ಅಭಿವ್ಯಕ್ತಿ ){
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced|EP_TokenOnly) ) ಹಿಂತಿರುಗಿ;
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_OuterON|EP_InnerON) || p_ಅಭಿವ್ಯಕ್ತಿ->w.iOfst<=0 ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL) ) ಹಿಂತಿರುಗಿ;
  db->err_Byte_ಸ್ಥಾನಾಂತರ = p_ಅಭಿವ್ಯಕ್ತಿ->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
ಪೂರ್ಣಾಂಕ sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  ಅಕ್ಷರ *zNew;
  ಪ್ರತಿಜ್ಞಾ( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  ಆದರೆ( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( p->mxAlloc==0 ){
    sqlite3_Str_Accum_ಗಣ_Error(p, SQLITE_TOOBIG);
    ಹಿಂತಿರುಗಿ p->nAlloc - p->nChar - 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಅಕ್ಷರ *zOld = isMalloced(p) ? p->z_ಪಠ್ಯ : 0;
    i64 szNew = p->nChar + N + 1;
    ಆದರೆ( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    ಆದರೆ( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_ಗಣ_Error(p, SQLITE_TOOBIG);
      ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->nAlloc = (ಪೂರ್ಣಾಂಕ)szNew;
    }
    ಆದರೆ( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }ಇಲ್ಲದಿದ್ದರೆ{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    ಆದರೆ( zNew ){
      ಪ್ರತಿಜ್ಞಾ( p->z_ಪಠ್ಯ!=0 || p->nChar==0 );
      ಆದರೆ( !isMalloced(p) && p->nChar>0 ) ಸ್ಮರಣೆ_ನಕಲು(zNew, p->z_ಪಠ್ಯ, p->nChar);
      p->z_ಪಠ್ಯ = zNew;
      p->nAlloc = sqlite3_Db_Malloc_ಗಾತ್ರ(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_ಗಣ_Error(p, SQLITE_NOMEM);
      ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( N>=0 && N<=0x7fffffff );
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)N;
}

ಪೂರ್ಣಾಂಕ sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  ಆದರೆ( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  ಹಿಂತಿರುಗಿ p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
ಶೂನ್ಯ sqlite3_str_appendchar(sqlite3_str *p, ಪೂರ್ಣಾಂಕ N, ಅಕ್ಷರ c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  ಆದರೆ( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ಹಿಂತಿರುಗಿ;
  }
  ಆಗ( (N--)>0 ) p->z_ಪಠ್ಯ[p->nChar++] = c;
}
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, ಅಕ್ಷರ c){
  testcase( p->nChar + N > 0x7fffffff );
  ಆದರೆ( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ಹಿಂತಿರುಗಿ;
  }
  ಆಗ( (N--)>0 ) p->z_ಪಠ್ಯ[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ N){
  N = sqlite3StrAccumEnlarge(p, N);
  ಪ್ರತಿಜ್ಞಾ( z!=0 || N==0 );
  ಆದರೆ( N>0 ){
    ಸ್ಮರಣೆ_ನಕಲು(&p->z_ಪಠ್ಯ[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
ಶೂನ್ಯ sqlite3_str_append(sqlite3_str *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ N){
  ಪ್ರತಿಜ್ಞಾ( z!=0 || N==0 );
  ಪ್ರತಿಜ್ಞಾ( p->z_ಪಠ್ಯ!=0 || p->nChar==0 || p->accError );
  ಪ್ರತಿಜ್ಞಾ( N>=0 );
  ಪ್ರತಿಜ್ಞಾ( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ಆದರೆ( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( N ){
    ಪ್ರತಿಜ್ಞಾ( p->z_ಪಠ್ಯ );
    p->nChar += N;
    ಸ್ಮರಣೆ_ನಕಲು(&p->z_ಪಠ್ಯ[p->nChar-N], z, N);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3StrAppend64(sqlite3_str *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, i64 N){
  ಪ್ರತಿಜ್ಞಾ( z!=0 || N==0 );
  ಪ್ರತಿಜ್ಞಾ( p->z_ಪಠ್ಯ!=0 || p->nChar==0 || p->accError );
  ಪ್ರತಿಜ್ಞಾ( N>=0 );
  ಪ್ರತಿಜ್ಞಾ( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ಆದರೆ( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( N ){
    ಪ್ರತಿಜ್ಞಾ( p->z_ಪಠ್ಯ );
    p->nChar += N;
    ಸ್ಮರಣೆ_ನಕಲು(&p->z_ಪಠ್ಯ[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
ಶೂನ್ಯ sqlite3_str_appendall(sqlite3_str *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಅಕ್ಷರ *str_Accum_ಮುಕ್ತಾಯ_Realloc(StrAccum *p){
  ಅಕ್ಷರ *z_ಪಠ್ಯ;
  ಪ್ರತಿಜ್ಞಾ( p->mxAlloc>0 && !isMalloced(p) );
  z_ಪಠ್ಯ = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  ಆದರೆ( z_ಪಠ್ಯ ){
    ಸ್ಮರಣೆ_ನಕಲು(z_ಪಠ್ಯ, p->z_ಪಠ್ಯ, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Str_Accum_ಗಣ_Error(p, SQLITE_NOMEM);
  }
  p->z_ಪಠ್ಯ = z_ಪಠ್ಯ;
  ಹಿಂತಿರುಗಿ z_ಪಠ್ಯ;
}
ಅಕ್ಷರ *sqlite3_Str_Accum_ಮುಕ್ತಾಯ(StrAccum *p){
  ಆದರೆ( p->z_ಪಠ್ಯ ){
    p->z_ಪಠ್ಯ[p->nChar] = 0;
    ಆದರೆ( p->mxAlloc>0 && !isMalloced(p) ){
      ಹಿಂತಿರುಗಿ str_Accum_ಮುಕ್ತಾಯ_Realloc(p);
    }
  }
  ಹಿಂತಿರುಗಿ p->z_ಪಠ್ಯ;
}

/* Finalize a string created using sqlite3_str_new().
*/
ಅಕ್ಷರ *sqlite3_str_ಮುಕ್ತಾಯ(sqlite3_str *p){
  ಅಕ್ಷರ *z;
  ಆದರೆ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3_Str_Accum_ಮುಕ್ತಾಯ(p);
    sqlite3_free(p);
  }ಇಲ್ಲದಿದ್ದರೆ{
    z = 0;
  }
  ಹಿಂತಿರುಗಿ z;
}

/* Return any error code associated with p */
ಪೂರ್ಣಾಂಕ sqlite3_str_errcode(sqlite3_str *p){
  ಹಿಂತಿರುಗಿ p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
ಪೂರ್ಣಾಂಕ sqlite3_str_ಉದ್ದಳತೆ(sqlite3_str *p){
  ಹಿಂತಿರುಗಿ p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
ಶೂನ್ಯ sqlite3_str_truncate(sqlite3_str *p, ಪೂರ್ಣಾಂಕ N){
  ಆದರೆ( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->z_ಪಠ್ಯ[p->nChar] = 0;
  }
}

/* Return the current value for p */
ಅಕ್ಷರ *sqlite3_str_ಮೌಲ್ಯ(sqlite3_str *p){
  ಆದರೆ( p==0 || p->nChar==0 ) ಹಿಂತಿರುಗಿ 0;
  p->z_ಪಠ್ಯ[p->nChar] = 0;
  ಹಿಂತಿರುಗಿ p->z_ಪಠ್ಯ;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
ಶೂನ್ಯ sqlite3_str_reset(StrAccum *p){
  ಆದರೆ( isMalloced(p) ){
    sqlite3DbFree(p->db, p->z_ಪಠ್ಯ);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p==(sqlite3_str*)&sqlite3OomStr ){
    ಹಿಂತಿರುಗಿ;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->z_ಪಠ್ಯ = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
ಶೂನ್ಯ sqlite3_str_free(sqlite3_str *p){
  ಆದರೆ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
ಶೂನ್ಯ sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, ಅಕ್ಷರ *z_ತಳ, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ mx){
  p->z_ಪಠ್ಯ = z_ತಳ;
  p->db = db;
  p->nAlloc = n;
  p->mxAlloc = mx;
  p->nChar = 0;
  p->accError = 0;
  p->printfFlags = 0;
}

/* Allocate and initialize a new dynamic string object */
sqlite3_str *sqlite3_str_new(sqlite3 *db){
  sqlite3_str *p = sqlite3_malloc64(ಗಾತ್ರ(*p));
  ಆದರೆ( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ] : SQLITE_MAX_ಉದ್ದಳತೆ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
ಅಕ್ಷರ *sqlite3VMPrintf(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, va_list ap){
  ಅಕ್ಷರ *z;
  ಅಕ್ಷರ z_ತಳ[SQLITE_PRINT_BUF_ಗಾತ್ರ];
  StrAccum acc;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  sqlite3StrAccumInit(&acc, db, z_ತಳ, ಗಾತ್ರ(z_ತಳ),
                      db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ]);
  acc.printfFlags = SQLITE_PRINTF_ಆಂತರಿಕ;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc);
  ಆದರೆ( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ಹಿಂತಿರುಗಿ z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
ಅಕ್ಷರ *sqlite3MPrintf(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;
  ಅಕ್ಷರ *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ಹಿಂತಿರುಗಿ z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
ಅಕ್ಷರ *sqlite3_vmprintf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, va_list ap){
  ಅಕ್ಷರ *z;
  ಅಕ್ಷರ z_ತಳ[SQLITE_PRINT_BUF_ಗಾತ್ರ];
  StrAccum acc;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR  
  ಆದರೆ( zFormat==0 ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3StrAccumInit(&acc, 0, z_ತಳ, ಗಾತ್ರ(z_ತಳ), SQLITE_MAX_ಉದ್ದಳತೆ);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc);
  ಹಿಂತಿರುಗಿ z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
ಅಕ್ಷರ *sqlite3_mprintf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;
  ಅಕ್ಷರ *z;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  ಹಿಂತಿರುಗಿ z;
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
ಅಕ್ಷರ *sqlite3_vsnprintf(ಪೂರ್ಣಾಂಕ n, ಅಕ್ಷರ *zBuf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, va_list ap){
  StrAccum acc;
  ಆದರೆ( n<=0 ) ಹಿಂತಿರುಗಿ zBuf;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( zBuf==0 || zFormat==0 ) {
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಆದರೆ( zBuf ) zBuf[0] = 0;
    ಹಿಂತಿರುಗಿ zBuf;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  ಹಿಂತಿರುಗಿ zBuf;
}
ಅಕ್ಷರ *sqlite3_snprintf(ಪೂರ್ಣಾಂಕ n, ಅಕ್ಷರ *zBuf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  StrAccum acc;
  va_list ap;
  ಆದರೆ( n<=0 ) ಹಿಂತಿರುಗಿ zBuf;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( zBuf==0 || zFormat==0 ) {
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಆದರೆ( zBuf ) zBuf[0] = 0;
    ಹಿಂತಿರುಗಿ zBuf;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  ಹಿಂತಿರುಗಿ zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#ಆದರೆ defined(SQLITE_MAX_LOG_ಸಂದೇಶ) 
  /* Leave the definition as supplied */
#ಅಥವಾಆದರೆ SQLITE_PRINT_BUF_ಗಾತ್ರ*10>10000
# define SQLITE_MAX_LOG_ಸಂದೇಶ 10000
#ಇಲ್ಲದಿದ್ದರೆ
# define SQLITE_MAX_LOG_ಸಂದೇಶ (SQLITE_PRINT_BUF_ಗಾತ್ರ*10)
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಶೂನ್ಯ render_Log_ಸಂದೇಶ(ಪೂರ್ಣಾಂಕ i_Err_ಸಂಕೇತ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  ಅಕ್ಷರ z_ಸಂದೇಶ[SQLITE_MAX_LOG_ಸಂದೇಶ];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_ಸಂದೇಶ, ಗಾತ್ರ(z_ಸಂದೇಶ), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, i_Err_ಸಂಕೇತ,
                           sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
ಶೂನ್ಯ sqlite3_log(ಪೂರ್ಣಾಂಕ i_Err_ಸಂಕೇತ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;                             /* Vararg list */
  ಆದರೆ( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_ಸಂದೇಶ(i_Err_ಸಂಕೇತ, zFormat, ap);
    va_end(ap);
  }
}

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
ಶೂನ್ಯ sqlite3DebugPrintf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;
  StrAccum acc;
  ಅಕ್ಷರ zBuf[SQLITE_PRINT_BUF_ಗಾತ್ರ*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, ಗಾತ್ರ(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OS_TRACE_PROC
  {
    ಬಾಹ್ಯ ಶೂನ್ಯ SQLITE_OS_TRACE_PROC(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zBuf, ಪೂರ್ಣಾಂಕ nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, ಗಾತ್ರ(zBuf));
  }
#ಇಲ್ಲದಿದ್ದರೆ
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
ಶೂನ್ಯ sqlite3_str_appendf(StrAccum *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
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
ಅಕ್ಷರ *sqlite3RCStrRef(ಅಕ್ಷರ *z){
  RCStr *p = (RCStr*)z;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  p--;
  p->nRCRef++;
  ಹಿಂತಿರುಗಿ z;
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
  ಆದರೆ( p->nRCRef>=2 ){
    p->nRCRef--;
  }ಇಲ್ಲದಿದ್ದರೆ{
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
ಅಕ್ಷರ *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + ಗಾತ್ರ(*p) + 1 );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  p->nRCRef = 1;
  ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
ಅಕ್ಷರ *sqlite3RCStrResize(ಅಕ್ಷರ *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  p--;
  ಪ್ರತಿಜ್ಞಾ( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+ಗಾತ್ರ(RCStr)+1);
  ಆದರೆ( pNew==0 ){
    sqlite3_free(p);
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)&pNew[1];
  }
}
