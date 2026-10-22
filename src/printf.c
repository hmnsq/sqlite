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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** Conversion types fall into various categories as defined by the
** following enumeration.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etRADIX       0 /* non-decimal integer types.  %x %o */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etFLOAT       1 /* Floating point.  %f */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etEXP         2 /* Exponentional notation. %e and %E */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etGENERIC     3 /* Floating or exponential, depending on exponent. %g */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ et_ਮਾਪ        4 /* Return number of characters processed so far. %n */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etSTRING      5 /* Strings. %s */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etDYNSTRING   6 /* Dynamically allocated strings. %z */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etPERCENT     7 /* Percent symbol. %% */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etCHARX       8 /* Characters. %c */
/* The rest are extensions, not normally found in printf() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etESCAPE_q    9  /* Strings with '\'' doubled.  %q */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etESCAPE_Q    10 /* Strings with '\'' doubled and enclosed in '',
                            NULL pointers replaced by SQL NULL.  %Q */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etTOKEN       11 /* a pointer to a Token structure */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etSRCITEM     12 /* a pointer to a SrcItem */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ et_ਸੂਚਕ     13 /* The %p conversion */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etESCAPE_w    14 /* %w -> Strings with '\"' doubled */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etORDINAL     15 /* %r -> 1st, 2nd, 3rd, 4th, etc.  English only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etDECIMAL     16 /* %d or %u, but not %x, %o */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etESCAPE_j    17 /* %j -> JSON string literal w/o "..." */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etESCAPE_J    18 /* %J -> JSON string literal with "..." */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etINVALID     19 /* Any unrecognized conversion type */


/*
** An "etByte" is an 8-bit unsigned value.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ et_info {   /* Information about each format field */
  ਅੱਖਰ fmttype;            /* The format field code letter */
  etByte ਅਧਾਰ;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte ਅਗੇਤਰ;           /* Offset into aPrefix[] of the prefix string */
  ਅੱਖਰ iNxt;               /* Next with same hash, or 0 for end of chain */
} et_info;

/*
** Allowed values for et_info.flags
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_SIGNED    1     /* True if the value to convert is signed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_STRING    4     /* Allow infinite precision */

/*
** The table is searched by hash.  In the case of %C where C is the character
** and that character has ASCII value j, then the hash is j%25.
**
** The order of the entries in fmtinfo[] and the hash chain was entered
** manually, but based on the output of the following TCL script:
*/
#ਜੇ 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  ਜਾਂਚ $c %c x
  ਸੈੱਟ_2 n($c) $x
}
ਸੈੱਟ_2 mx [llength [array ਨਾਂ n]]
puts "count: $mx"

ਸੈੱਟ_2 mx 25
puts "*********** mx=$mx ************"
ਲਈ {ਸੈੱਟ_2 r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array ਨਾਂ n] {
    ਜੇ {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#ਜੇ_ਅੰਤ /***** End of script ********/

ਸਥਿਰ ਅਚਲ ਅੱਖਰ aDigits[] = "0123456789ABCDEF0123456789abcdef";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ aHex[]    = "0123456789abcdef";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ a_ਅਗੇਤਰ[] = "-x0\000X0";
ਸਥਿਰ ਅਚਲ et_info fmtinfo[25] = {
  /*  0 */  {  'd', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  1 */  {  'e',  0, 1, etEXP,        30, 0,  0 },
  /*  2 */  {  'f',  0, 1, etFLOAT,      0,  0,  0 },
  /*  3 */  {  'g',  0, 1, etGENERIC,    30, 0,  0 },
  /*  4 */  {  'j',  0, 0, etESCAPE_j,   0,  0,  0 },  /* Hash: 6 */
  /*  5 */  {  'i', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  6 */  {  'Q',  0, 4, etESCAPE_Q,   0,  0,  4 },
  /*  7 */  {  'p', 16, 0, et_ਸੂਚਕ,    0,  1,  0 },  /* Hash: 12 */
  /*  8 */  {  'S',  0, 0, etSRCITEM,    0,  0,  0 },
  /*  9 */  {  'T',  0, 0, etTOKEN,      0,  0,  0 },
  /* 10 */  {  'n',  0, 0, et_ਮਾਪ,       0,  0,  0 },
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
ਖਾਲੀ sqlite3_Str_Accum_ਸੈੱਟ_Error(StrAccum *p, u8 eError){
  ਪ੍ਰਤਿਜ੍ਞਾ( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  ਜੇ( p->mxAlloc ) sqlite3_str_reset(p);
  ਜੇ( eError==SQLITE_TOOBIG ) sqlite3_Error_ਮੰਜ਼ਿਲ_Parser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ਸਥਿਰ sqlite3_int64 getIntArg(PrintfArguments *p){
  ਜੇ( p->nArg<=p->n_ਵਰਤਿਆ ) ਵਾਪਸ 0;
  ਵਾਪਸ sqlite3_ਮੁੱਲ_int64(p->apArg[p->n_ਵਰਤਿਆ++]);
}
ਸਥਿਰ ਦੁੱਗਣਾ getDoubleArg(PrintfArguments *p){
  ਜੇ( p->nArg<=p->n_ਵਰਤਿਆ ) ਵਾਪਸ 0.0;
  ਵਾਪਸ sqlite3_ਮੁੱਲ_double(p->apArg[p->n_ਵਰਤਿਆ++]);
}
ਸਥਿਰ ਅੱਖਰ *get_ਪਾਠ_Arg(PrintfArguments *p){
  ਜੇ( p->nArg<=p->n_ਵਰਤਿਆ ) ਵਾਪਸ 0;
  ਵਾਪਸ (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(p->apArg[p->n_ਵਰਤਿਆ++]);
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
ਸਥਿਰ ਅੱਖਰ *printf_ਆਰਜ਼ੀ_Buf(sqlite3_str *pAccum, sqlite3_int64 n){
  ਅੱਖਰ *z;
  ਜੇ( pAccum->accError ) ਵਾਪਸ 0;
  ਜੇ( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3_Str_Accum_ਸੈੱਟ_Error(pAccum, SQLITE_TOOBIG);
    ਵਾਪਸ 0;
  }
  z = sqlite3_malloc(n);
  ਜੇ( z==0 ){
    sqlite3_Str_Accum_ਸੈੱਟ_Error(pAccum, SQLITE_NOMEM);
  }
  ਵਾਪਸ z;
}

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be something smaller, if desired.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_PRINT_BUF_ਮਾਪ
# define SQLITE_PRINT_BUF_ਮਾਪ 70
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ etBUFSIZE SQLITE_PRINT_BUF_ਮਾਪ  /* Size of the output buffer */

/*
** Hard limit on the precision of floating-point conversions to
** the minimum of SQLITE_PRINTF_PRECISION_LIMIT and 100 million.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ
# ਜੇ SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ<100000000
#   define SQLITE_FP_PRECISION_ਹੱਦਬੰਦੀ SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ
# ਨਹੀਂਤਾਂ
#   define SQLITE_FP_PRECISION_ਹੱਦਬੰਦੀ 100000000
# endif
#ਨਹੀਂਤਾਂ
# define SQLITE_FP_PRECISION_ਹੱਦਬੰਦੀ 100000000
#ਜੇ_ਅੰਤ

/* Forward reference */
ਸਥਿਰ ਖਾਲੀ sqlite3StrAppend64(sqlite3_str *p, ਅਚਲ ਅੱਖਰ *z, i64 N);
ਸਥਿਰ ਖਾਲੀ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, ਅੱਖਰ c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
ਖਾਲੀ sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ਅਚਲ ਅੱਖਰ *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  ਪੂਰਨ c;                     /* Next character in the format string */
  ਅੱਖਰ *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 ਲੰਮਾਈ;                /* Length of the field */
  ਪੂਰਨ idx;                   /* A general purpose loop counter */
  i64 ਚੌੜਾਈ;                 /* Width of the current field */
  etByte flag_leftjustify;   /* True if "-" flag is present */
  etByte flag_ਅਗੇਤਰ;        /* '+' or ' ' or 0 for prefix */
  etByte flag_alternateform; /* True if "#" flag is present */
  etByte flag_altform2;      /* True if "!" flag is present */
  etByte flag_zeropad;       /* True if field width constant starts with zero */
  etByte flag_long;          /* 1 for the "l" flag, 2 for "ll", 0 by default */
  etByte ਪੂਰਾ;               /* Loop termination flag */
  etByte cThousand;          /* Thousands separator for %d and %u */
  etByte xtype = etINVALID;  /* Conversion paradigm */
  u8 bArgList;               /* True for SQLITE_PRINTF_SQLFUNC */
  ਅੱਖਰ ਅਗੇਤਰ;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  ਦੁੱਗਣਾ realvalue;          /* Value for real types */
  ਅਚਲ et_info *infop;      /* Pointer to the appropriate info structure */
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;                /* Rendering buffer */
  ਪੂਰਨ n_ਬਾਹਰਲਾ;                  /* Size of the rendering buffer */
  ਅੱਖਰ *zExtra = 0;          /* Malloced memory used by some conversion */
  ਪੂਰਨ ਘਾਤਾਂਕ, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  ਅੱਖਰ buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  ਪ੍ਰਤਿਜ੍ਞਾ( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  ਜੇ( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }ਨਹੀਂਤਾਂ{
    bArgList = 0;
  }
  ਲਈ(; (c=(*fmt))!=0; ++fmt){
    ਜੇ( c!='%' ){
      bufpt = (ਅੱਖਰ *)fmt;
#ਜੇ HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#ਨਹੀਂਤਾਂ
      fmt = strchr(fmt, '%');
      ਜੇ( fmt==0 ){
        fmt = bufpt + ਲੜੀ_ਲੰਬਾਈ(bufpt);
      }
#ਜੇ_ਅੰਤ
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      ਜੇ( *fmt==0 ) ਰੋਕੋ;
    }
    ਜੇ( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      ਰੋਕੋ;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_ਅਗੇਤਰ = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    ਪੂਰਾ = 0;
    ਚੌੜਾਈ = 0;
    flag_long = 0;
    precision = -1;
    ਕਰੋ{
      ਚੋਣ( c ){
        ਹਾਲਤ '-':   flag_leftjustify = 1;     ਰੋਕੋ;
        ਹਾਲਤ '+':   flag_ਅਗੇਤਰ = '+';        ਰੋਕੋ;
        ਹਾਲਤ ' ':   flag_ਅਗੇਤਰ = ' ';        ਰੋਕੋ;
        ਹਾਲਤ '#':   flag_alternateform = 1;   ਰੋਕੋ;
        ਹਾਲਤ '!':   flag_altform2 = 1;        ਰੋਕੋ;
        ਹਾਲਤ '0':   flag_zeropad = 1;         ਰੋਕੋ;
        ਹਾਲਤ ',':   cThousand = ',';          ਰੋਕੋ;
        ਮੂਲ:    ਪੂਰਾ = 1;                 ਰੋਕੋ;
        ਹਾਲਤ 'l': {
          flag_long = 1;
          c = *++fmt;
          ਜੇ( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          ਪੂਰਾ = 1;
          ਰੋਕੋ;
        }
        ਹਾਲਤ '1': ਹਾਲਤ '2': ਹਾਲਤ '3': ਹਾਲਤ '4': ਹਾਲਤ '5':
        ਹਾਲਤ '6': ਹਾਲਤ '7': ਹਾਲਤ '8': ਹਾਲਤ '9': {
          ਬਿਨਾਨਿਸ਼ਾਨੀ wx = c - '0';
          ਜਦੋਂ( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          ਚੌੜਾਈ = wx & 0x7fffffff;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ
          ਜੇ( ਚੌੜਾਈ>SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ ){
            ਚੌੜਾਈ = SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ;
          }
#ਜੇ_ਅੰਤ
          ਜੇ( c!='.' && c!='l' ){
            ਪੂਰਾ = 1;
          }ਨਹੀਂਤਾਂ{
            fmt--;
          }
          ਰੋਕੋ;
        }
        ਹਾਲਤ '*': {
          ਜੇ( bArgList ){
            ਚੌੜਾਈ = (ਪੂਰਨ)getIntArg(pArgList);
          }ਨਹੀਂਤਾਂ{
            ਚੌੜਾਈ = va_arg(ap,ਪੂਰਨ);
          }
          ਜੇ( ਚੌੜਾਈ<0 ){
            flag_leftjustify = 1;
            ਚੌੜਾਈ = ਚੌੜਾਈ >= -2147483647 ? -ਚੌੜਾਈ : 0;
          }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ
          ਜੇ( ਚੌੜਾਈ>SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ ){
            ਚੌੜਾਈ = SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ;
          }
#ਜੇ_ਅੰਤ
          ਜੇ( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            ਪੂਰਾ = 1;
          }
          ਰੋਕੋ;
        }
        ਹਾਲਤ '.': {
          c = *++fmt;
          ਜੇ( c=='*' ){
            ਜੇ( bArgList ){
              precision = (ਪੂਰਨ)getIntArg(pArgList);
            }ਨਹੀਂਤਾਂ{
              precision = va_arg(ap,ਪੂਰਨ);
            }
            ਜੇ( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }ਨਹੀਂਤਾਂ{
            ਬਿਨਾਨਿਸ਼ਾਨੀ px = 0;
            ਜਦੋਂ( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ
          ਜੇ( precision>SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ ){
            precision = SQLITE_PRINTF_PRECISION_ਹੱਦਬੰਦੀ;
          }
#ਜੇ_ਅੰਤ
          ਜੇ( c=='l' ){
            --fmt;
          }ਨਹੀਂਤਾਂ{
            ਪੂਰਾ = 1;
          }
          ਰੋਕੋ;
        }
      }
    }ਜਦੋਂ( !ਪੂਰਾ && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    ਲਈ(idx=0; idx<Array_ਮਾਪ(fmtinfo); idx++){
      ਜੇ( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        ਰੋਕੋ;
      }
    }
#ਨਹੀਂਤਾਂ
    /* Fast hash-table lookup */
    ਪ੍ਰਤਿਜ੍ਞਾ( Array_ਮਾਪ(fmtinfo)==25 );
    idx = ((ਬਿਨਾਨਿਸ਼ਾਨੀ)c) % 25;
    ਜੇ( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }ਨਹੀਂਤਾਂ{
      infop = &fmtinfo[0];
      xtype = etINVALID;
    }
#ਜੇ_ਅੰਤ

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
    ਪ੍ਰਤਿਜ੍ਞਾ( ਚੌੜਾਈ>=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( precision>=(-1) );
    ਚੋਣ( xtype ){
      ਹਾਲਤ et_ਸੂਚਕ:
        flag_long = ਆਕਾਰ(ਅੱਖਰ*)==ਆਕਾਰ(i64) ? 2 :
                     ਆਕਾਰ(ਅੱਖਰ*)==ਆਕਾਰ(ਲੰਮਾ ਪੂਰਨ) ? 1 : 0;
        /* no break */ deliberate_fall_through
      ਹਾਲਤ etORDINAL:
      ਹਾਲਤ etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      ਹਾਲਤ etDECIMAL:
        ਜੇ( infop->flags & FLAG_SIGNED ){
          i64 v;
          ਜੇ( bArgList ){
            v = getIntArg(pArgList);
          }ਨਹੀਂਤਾਂ ਜੇ( flag_long ){
            ਜੇ( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }ਨਹੀਂਤਾਂ{
              v = va_arg(ap,ਲੰਮਾ ਪੂਰਨ);
            }
          }ਨਹੀਂਤਾਂ{
            v = va_arg(ap,ਪੂਰਨ);
          }
          ਜੇ( v<0 ){
            testcase( v==ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            ਅਗੇਤਰ = '-';
          }ਨਹੀਂਤਾਂ{
            longvalue = v;
            ਅਗੇਤਰ = flag_ਅਗੇਤਰ;
          }
        }ਨਹੀਂਤਾਂ{
          ਜੇ( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }ਨਹੀਂਤਾਂ ਜੇ( flag_long ){
            ਜੇ( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }ਨਹੀਂਤਾਂ{
              longvalue = va_arg(ap,ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ);
            }
          }ਨਹੀਂਤਾਂ{
            longvalue = va_arg(ap,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
          }
          ਅਗੇਤਰ = 0;
        }

#ਜੇ WHERETRACE_ENABLED
        ਜੇ( xtype==et_ਸੂਚਕ && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#ਜੇ_ਅੰਤ
#ਜੇ TREETRACE_ENABLED
        ਜੇ( xtype==et_ਸੂਚਕ && sqlite3_ਰੁੱਖ_Trace & 0x100000 ) longvalue = 0;
#ਜੇ_ਅੰਤ

        ਜੇ( longvalue==0 ) flag_alternateform = 0;
        ਜੇ( flag_zeropad && precision<ਚੌੜਾਈ-(ਅਗੇਤਰ!=0) ){
          precision = ਚੌੜਾਈ-(ਅਗੇਤਰ!=0);
        }
        ਜੇ( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          n_ਬਾਹਰਲਾ = etBUFSIZE;
          z_ਬਾਹਰਲਾ = buf;
        }ਨਹੀਂਤਾਂ{
          u64 n;
          n = (u64)precision + 10;
          ਜੇ( cThousand ) n += precision/3;
          z_ਬਾਹਰਲਾ = zExtra = printf_ਆਰਜ਼ੀ_Buf(pAccum, n);
          ਜੇ( z_ਬਾਹਰਲਾ==0 ) ਵਾਪਸ;
          n_ਬਾਹਰਲਾ = (ਪੂਰਨ)n;
        }
        bufpt = &z_ਬਾਹਰਲਾ[n_ਬਾਹਰਲਾ-1];
        ਜੇ( xtype==etORDINAL ){
          ਸਥਿਰ ਅਚਲ ਅੱਖਰ zOrd[] = "thstndrd";
          ਪੂਰਨ x = (ਪੂਰਨ)(longvalue % 10);
          ਜੇ( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ਅਚਲ ਅੱਖਰ *cset = &aDigits[infop->charset];
          u8 ਅਧਾਰ = infop->ਅਧਾਰ;
          ਕਰੋ{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%ਅਧਾਰ];
            longvalue = longvalue/ਅਧਾਰ;
          }ਜਦੋਂ( longvalue>0 );
        }
        ਲੰਮਾਈ = &z_ਬਾਹਰਲਾ[n_ਬਾਹਰਲਾ-1] - bufpt;
        ਜੇ( precision>ਲੰਮਾਈ ){                         /* zero pad */
          i64 nn = precision-ਲੰਮਾਈ;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          ਲੰਮਾਈ = precision;
        }
        ਜੇ( cThousand ){
          i64 nn = (ਲੰਮਾਈ - 1)/3;  /* Number of "," to insert */
          i64 ix = (ਲੰਮਾਈ - 1)%3 + 1;
          ਪੂਰਨ ii;
          bufpt -= nn;
          ਲਈ(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            ਜੇ( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        ਜੇ( ਅਗੇਤਰ ) *(--bufpt) = ਅਗੇਤਰ;               /* Add sign */
        ਜੇ( flag_alternateform && infop->ਅਗੇਤਰ ){      /* Add "0" or "0x" */
          ਅਚਲ ਅੱਖਰ *pre;
          ਅੱਖਰ x;
          pre = &a_ਅਗੇਤਰ[infop->ਅਗੇਤਰ];
          ਲਈ(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        ਲੰਮਾਈ = &z_ਬਾਹਰਲਾ[n_ਬਾਹਰਲਾ-1] - bufpt;
        ਰੋਕੋ;
      ਹਾਲਤ etFLOAT:
      ਹਾਲਤ etEXP:
      ਹਾਲਤ etGENERIC: {
        FpDecode s;
        ਪੂਰਨ i_ਗੇੜਾ;
        ਪੂਰਨ j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        ਜੇ( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }ਨਹੀਂਤਾਂ{
          realvalue = va_arg(ap,ਦੁੱਗਣਾ);
        }
        ਜੇ( precision<0 ) precision = 6;         /* Set default precision */
        ਜੇ( precision>SQLITE_FP_PRECISION_ਹੱਦਬੰਦੀ ){
          precision = SQLITE_FP_PRECISION_ਹੱਦਬੰਦੀ;
        }
        ਜੇ( xtype==etFLOAT ){
          i_ਗੇੜਾ = -precision;
        }ਨਹੀਂਤਾਂ ਜੇ( xtype==etGENERIC ){
          ਜੇ( precision==0 ) precision = 1;
          i_ਗੇੜਾ = precision;
        }ਨਹੀਂਤਾਂ{
          i_ਗੇੜਾ = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, i_ਗੇੜਾ, flag_altform2 ? 20 : 16);
        ਜੇ( s.isSpecial ){
          ਜੇ( s.isSpecial==2 ){
            ਜੇ( flag_zeropad ){
              bufpt = "null";
              ਲੰਮਾਈ = 4;
            }ਨਹੀਂਤਾਂ{
              bufpt = "NaN";
              ਲੰਮਾਈ = 3;
            }
            ਰੋਕੋ;
          }ਨਹੀਂਤਾਂ ਜੇ( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }ਨਹੀਂਤਾਂ{
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, "-Inf", 5);
            bufpt = buf;
            ਜੇ( s.sign=='-' ){
              /* no-op */
            }ਨਹੀਂਤਾਂ ਜੇ( flag_ਅਗੇਤਰ ){
              buf[0] = flag_ਅਗੇਤਰ;
            }ਨਹੀਂਤਾਂ{
              bufpt++;
            }
            ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(bufpt);
            ਰੋਕੋ;
          }
        }
        ਜੇ( s.sign=='-' ){
          ਜੇ( flag_alternateform
           && !flag_ਅਗੇਤਰ
           && xtype==etFLOAT
           && s.iDP<=i_ਗੇੜਾ
          ){
            /* Suppress the minus sign if all of the following are true:
            **   *  The value displayed is zero
            **   *  The '#' flag is used
            **   *  The '+' flag is not used, and
            **   *  The format is %f
            */
            ਅਗੇਤਰ = 0;
          }ਨਹੀਂਤਾਂ{
            ਅਗੇਤਰ = '-';
          }
        }ਨਹੀਂਤਾਂ{
          ਅਗੇਤਰ = flag_ਅਗੇਤਰ;
        }

        ਘਾਤਾਂਕ = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        ਜੇ( xtype==etGENERIC ){
          ਪ੍ਰਤਿਜ੍ਞਾ( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          ਜੇ( ਘਾਤਾਂਕ<-4 || ਘਾਤਾਂਕ>precision ){
            xtype = etEXP;
          }ਨਹੀਂਤਾਂ{
            precision = precision - ਘਾਤਾਂਕ;
            xtype = etFLOAT;
          }
        }ਨਹੀਂਤਾਂ{
          flag_rtz = flag_altform2;
        }
        ਜੇ( xtype==etEXP ){
          e2 = 0;
        }ਨਹੀਂਤਾਂ{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)ਚੌੜਾਈ+10;
        ਜੇ( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        ਜੇ( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          ਜੇ( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            ਜੇ( bufpt==0 ){
              sqlite3_Str_Accum_ਸੈੱਟ_Error(pAccum, SQLITE_NOMEM);
              ਵਾਪਸ;
            }
            zExtra = bufpt;
          }ਨਹੀਂਤਾਂ ਜੇ( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            ਚੌੜਾਈ = ਲੰਮਾਈ = 0;
            ਰੋਕੋ;
          }ਨਹੀਂਤਾਂ{
            bufpt = pAccum->z_ਪਾਠ + pAccum->nChar;
          }
        }ਨਹੀਂਤਾਂ{
          bufpt = pAccum->z_ਪਾਠ + pAccum->nChar;
        }
        z_ਬਾਹਰਲਾ = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        ਜੇ( ਅਗੇਤਰ ){
          *(bufpt++) = ਅਗੇਤਰ;
        }
        /* Digits prior to the decimal point */
        j = 0;
        ਪ੍ਰਤਿਜ੍ਞਾ( s.n>0 );
        ਜੇ( e2<0 ){
          *(bufpt++) = '0';
        }ਨਹੀਂਤਾਂ ਜੇ( cThousand ){
          ਲਈ(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            ਜੇ( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }ਨਹੀਂਤਾਂ{
          j = e2+1;
          ਜੇ( j>s.n ) j = s.n;
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          ਜੇ( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        ਜੇ( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        ਜੇ( e2<(-1) && precision>0 ){
          ਪੂਰਨ nn = -1-e2;
          ਜੇ( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        ਜੇ( precision>0 ){
          ਪੂਰਨ nn = s.n - j;
          ਜੇ( NEVER(nn>precision) ) nn = precision;
          ਜੇ( nn>0 ){
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          ਜੇ( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        ਜੇ( flag_rtz && flag_dp ){
          ਜਦੋਂ( bufpt[-1]=='0' ) *(--bufpt) = 0;
          ਪ੍ਰਤਿਜ੍ਞਾ( bufpt>z_ਬਾਹਰਲਾ );
          ਜੇ( bufpt[-1]=='.' ){
            ਜੇ( flag_altform2 ){
              *(bufpt++) = '0';
            }ਨਹੀਂਤਾਂ{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        ਜੇ( xtype==etEXP ){
          ਘਾਤਾਂਕ = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          ਜੇ( ਘਾਤਾਂਕ<0 ){
            *(bufpt++) = '-'; ਘਾਤਾਂਕ = -ਘਾਤਾਂਕ;
          }ਨਹੀਂਤਾਂ{
            *(bufpt++) = '+';
          }
          ਜੇ( ਘਾਤਾਂਕ>=100 ){
            *(bufpt++) = (ਅੱਖਰ)((ਘਾਤਾਂਕ/100)+'0');        /* 100's digit */
            ਘਾਤਾਂਕ %= 100;
          }
          *(bufpt++) = (ਅੱਖਰ)(ਘਾਤਾਂਕ/10+'0');             /* 10's digit */
          *(bufpt++) = (ਅੱਖਰ)(ਘਾਤਾਂਕ%10+'0');             /* 1's digit */
        }

        ਲੰਮਾਈ = bufpt - z_ਬਾਹਰਲਾ;
        ਪ੍ਰਤਿਜ੍ਞਾ( ਲੰਮਾਈ <= szBufNeeded );
        ਜੇ( ਲੰਮਾਈ<ਚੌੜਾਈ ){
          i64 nPad = ਚੌੜਾਈ - ਲੰਮਾਈ;
          ਜੇ( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }ਨਹੀਂਤਾਂ ਜੇ( !flag_zeropad ){
            memmove(z_ਬਾਹਰਲਾ+nPad, z_ਬਾਹਰਲਾ, ਲੰਮਾਈ);
            memset(z_ਬਾਹਰਲਾ, ' ', nPad);
          }ਨਹੀਂਤਾਂ{
            ਪੂਰਨ adj = ਅਗੇਤਰ!=0;
            memmove(z_ਬਾਹਰਲਾ+nPad+adj, z_ਬਾਹਰਲਾ+adj, ਲੰਮਾਈ-adj);
            memset(z_ਬਾਹਰਲਾ+adj, '0', nPad);
          }
          ਲੰਮਾਈ = ਚੌੜਾਈ;
        }

        ਜੇ( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          ਪ੍ਰਤਿਜ੍ਞਾ( pAccum->nChar + ਲੰਮਾਈ < SMXV(pAccum->nChar) );
          pAccum->nChar += ਲੰਮਾਈ;
          z_ਬਾਹਰਲਾ[ਲੰਮਾਈ] = 0;
          ਜਾਰੀ;
        }ਨਹੀਂਤਾਂ{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          ਰੋਕੋ;
        }
      }
      ਹਾਲਤ et_ਮਾਪ:
        ਜੇ( !bArgList ){
          *(va_arg(ap,ਪੂਰਨ*)) = pAccum->nChar;
        }
        ਲੰਮਾਈ = ਚੌੜਾਈ = 0;
        ਰੋਕੋ;
      ਹਾਲਤ etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        ਲੰਮਾਈ = 1;
        ਰੋਕੋ;
      ਹਾਲਤ etCHARX:
        ਜੇ( bArgList ){
          bufpt = get_ਪਾਠ_Arg(pArgList);
          ਲੰਮਾਈ = 1;
          ਜੇ( bufpt ){
            buf[0] = c = *(bufpt++);
            ਜੇ( (c&0xc0)==0xc0 ){
              ਜਦੋਂ( ਲੰਮਾਈ<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[ਲੰਮਾਈ++] = *(bufpt++);
              }
            }
          }ਨਹੀਂਤਾਂ{
            buf[0] = 0;
          }
        }ਨਹੀਂਤਾਂ{
          ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ch = va_arg(ap,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
          ਲੰਮਾਈ = sqlite3AppendOneUtf8Character(buf, ch);
        }
        ਜੇ( precision>1 ){
          i64 n_ਪੂਰਵ = 1;
          ਚੌੜਾਈ -= precision-1;
          ਜੇ( ਚੌੜਾਈ>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, ਚੌੜਾਈ-1, ' ');
            ਚੌੜਾਈ = 0;
          }
          sqlite3StrAppend64(pAccum, buf, ਲੰਮਾਈ);
          precision--;
          ਜਦੋਂ( precision > 1 ){
            i64 n_Copy_ਬਾਈਟਾਂ;
            ਜੇ( n_ਪੂਰਵ > precision-1 ) n_ਪੂਰਵ = precision - 1;
            n_Copy_ਬਾਈਟਾਂ = ਲੰਮਾਈ*n_ਪੂਰਵ;
            ਜੇ( sqlite3StrAccumEnlargeIfNeeded(pAccum, n_Copy_ਬਾਈਟਾਂ) ){
              ਰੋਕੋ;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->z_ਪਾਠ[pAccum->nChar-n_Copy_ਬਾਈਟਾਂ], n_Copy_ਬਾਈਟਾਂ);
            precision -= n_ਪੂਰਵ;
            n_ਪੂਰਵ *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        ਜਾਓ adjust_ਚੌੜਾਈ_for_utf8;
      ਹਾਲਤ etSTRING:
      ਹਾਲਤ etDYNSTRING:
        ਜੇ( bArgList ){
          bufpt = get_ਪਾਠ_Arg(pArgList);
          xtype = etSTRING;
        }ਨਹੀਂਤਾਂ{
          bufpt = va_arg(ap,ਅੱਖਰ*);
        }
        ਜੇ( bufpt==0 ){
          bufpt = "";
        }ਨਹੀਂਤਾਂ ਜੇ( xtype==etDYNSTRING ){
          ਜੇ( pAccum->nChar==0
           && pAccum->mxAlloc
           && ਚੌੜਾਈ==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            ਪ੍ਰਤਿਜ੍ਞਾ( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->z_ਪਾਠ = bufpt;
            pAccum->nAlloc = sqlite3_Db_Malloc_ਮਾਪ(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            ਲੰਮਾਈ = 0;
            ਰੋਕੋ;
          }
          zExtra = bufpt;
        }
        ਜੇ( precision>=0 ){
          ਜੇ( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)bufpt;
            ਜਦੋਂ( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            ਲੰਮਾਈ = z - (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)bufpt;
          }ਨਹੀਂਤਾਂ{
            ਲਈ(ਲੰਮਾਈ=0; ਲੰਮਾਈ<precision && bufpt[ਲੰਮਾਈ]; ਲੰਮਾਈ++){}
          }
        }ਨਹੀਂਤਾਂ{
          ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(bufpt);
        }
      adjust_ਚੌੜਾਈ_for_utf8:
        ਜੇ( flag_altform2 && ਚੌੜਾਈ>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = ਲੰਮਾਈ - 1;
          ਜਦੋਂ( ii>=0 ) ਜੇ( (bufpt[ii--] & 0xc0)==0x80 ) ਚੌੜਾਈ++;
        }
        ਰੋਕੋ;
      ਹਾਲਤ etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      ਹਾਲਤ etESCAPE_J: {         /* %J: Generate a JSON string literal */
        ਅੱਖਰ *escarg;
        i64 i, j, px, i_ਸ਼ੁਰੂ;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ch;

        ਜੇ( bArgList ){
          escarg = get_ਪਾਠ_Arg(pArgList);
        }ਨਹੀਂਤਾਂ{
          escarg = va_arg(ap,ਅੱਖਰ*);
        }
        i_ਸ਼ੁਰੂ = sqlite3_str_ਲੰਮਾਈ(pAccum);
        ਜੇ( escarg==0 ){
          ਜੇ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }ਨਹੀਂਤਾਂ{
          ਜੇ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          ਜੇ( px<0 ){
            px = 0x7fffffff;
          }ਨਹੀਂਤਾਂ ਜੇ( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            ਲਈ(i=0; i<px && escarg[i]; i++){
              ਜੇ( (escarg[i]&0xc0)==0x80 ) px++;
            }
            ਜੇ( i==px ){
              ਜਦੋਂ( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          ਲਈ(i=j=0; i<px; i++){
            ਜੇ( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              ਜੇ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              ਜੇ( ch==0 ) ਰੋਕੋ;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              ਜੇ( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ਨਹੀਂਤਾਂ ਜੇ( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ਨਹੀਂਤਾਂ{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          ਜੇ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          ਜੇ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        ਜੇ( ਚੌੜਾਈ>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_ਲੰਮਾਈ(pAccum) - i_ਸ਼ੁਰੂ;
          sqlite3_int64 len = n;
          ਅੱਖਰ *zz;
          ਜੇ( flag_altform2 && n>0 ){
            zz = sqlite3_str_ਮੁੱਲ(pAccum);
            ਲਈ(i=i_ਸ਼ੁਰੂ; zz[i]; i++){
              ਜੇ( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          ਜੇ( ਚੌੜਾਈ>len ){
            sqlite3_int64 sp = ਚੌੜਾਈ-len;
            ਪ੍ਰਤਿਜ੍ਞਾ( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (ਪੂਰਨ)sp, ' ');
            ਜੇ( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_ਮੁੱਲ(pAccum);
              zz += i_ਸ਼ੁਰੂ;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        ਜਾਰੀ;
      }
      ਹਾਲਤ etESCAPE_q:          /* %q: Escape ' characters */
      ਹਾਲਤ etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      ਹਾਲਤ etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        ਪੂਰਨ needQuote = 0;
        ਅੱਖਰ ch;
        ਅੱਖਰ *escarg;
        ਅੱਖਰ q;

        ਜੇ( bArgList ){
          escarg = get_ਪਾਠ_Arg(pArgList);
        }ਨਹੀਂਤਾਂ{
          escarg = va_arg(ap,ਅੱਖਰ*);
        }
        ਜੇ( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }ਨਹੀਂਤਾਂ ਜੇ( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        ਜੇ( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }ਨਹੀਂਤਾਂ{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        ਲਈ(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          ਜੇ( ch==q )  n++;
          ਜੇ( flag_altform2 && (ch&0xc0)==0xc0 ){
            ਜਦੋਂ( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        ਜੇ( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 n_ਪਿੱਠ = 0;
          i64 nCtrl = 0;
          ਲਈ(k=0; k<i; k++){
            ਜੇ( escarg[k]=='\\' ){
              n_ਪਿੱਠ++;
            }ਨਹੀਂਤਾਂ ਜੇ( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          ਜੇ( nCtrl || xtype==etESCAPE_q ){
            n += n_ਪਿੱਠ + 5*nCtrl;
            ਜੇ( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }ਨਹੀਂਤਾਂ{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        ਜੇ( n>etBUFSIZE ){
          bufpt = zExtra = printf_ਆਰਜ਼ੀ_Buf(pAccum, n);
          ਜੇ( bufpt==0 ) ਵਾਪਸ;
        }ਨਹੀਂਤਾਂ{
          bufpt = buf;
        }
        j = 0;
        ਜੇ( needQuote ){
          ਜੇ( needQuote==2 ){
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(&bufpt[j], "unistr('", 8);
            j += 8;
          }ਨਹੀਂਤਾਂ{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        ਜੇ( flag_alternateform ){
          ਲਈ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ਜੇ( ch==q ){
              bufpt[j++] = ch;
            }ਨਹੀਂਤਾਂ ਜੇ( ch=='\\' ){
              bufpt[j++] = '\\';
            }ਨਹੀਂਤਾਂ ਜੇ( ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }ਨਹੀਂਤਾਂ{
          ਲਈ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ਜੇ( ch==q ) bufpt[j++] = ch;
          }
        }
        ਜੇ( needQuote ){
          bufpt[j++] = '\'';
          ਜੇ( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        ਲੰਮਾਈ = j;
        ਜਾਓ adjust_ਚੌੜਾਈ_for_utf8;
      }
      ਹਾਲਤ etTOKEN: {
        ਜੇ( (pAccum->printfFlags & SQLITE_PRINTF_ਅੰਦਰੂਨੀ)==0 ) ਵਾਪਸ;
        ਜੇ( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = va_arg(ap,ਵਿਅੰਜਕ*);
          ਜੇ( ALWAYS(p_ਵਿਅੰਜਕ) && ALWAYS(!ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_Int_ਮੁੱਲ)) ){
            sqlite3_str_appendall(pAccum, (ਅਚਲ ਅੱਖਰ*)p_ਵਿਅੰਜਕ->u.zToken);
            sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(pAccum->db, p_ਵਿਅੰਜਕ);
          }
        }ਨਹੀਂਤਾਂ{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          ਪ੍ਰਤਿਜ੍ਞਾ( bArgList==0 );
          ਜੇ( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ਅਚਲ ਅੱਖਰ*)pToken->z, pToken->n);
            sqlite3_ਰਿਕਾਰਡ_Error_Byte_ਵਿੱਥ(pAccum->db, pToken->z);
          }
        }
        ਲੰਮਾਈ = ਚੌੜਾਈ = 0;
        ਰੋਕੋ;
      }
      ਹਾਲਤ etSRCITEM: {
        Src_ਵਸਤੂ *p_ਵਸਤੂ;
        ਜੇ( (pAccum->printfFlags & SQLITE_PRINTF_ਅੰਦਰੂਨੀ)==0 ) ਵਾਪਸ;
        p_ਵਸਤੂ = va_arg(ap, Src_ਵਸਤੂ*);
        ਪ੍ਰਤਿਜ੍ਞਾ( bArgList==0 );
        ਜੇ( p_ਵਸਤੂ->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, p_ਵਸਤੂ->zAlias);
        }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->zName ){
          ਜੇ( p_ਵਸਤੂ->fg.fixedSchema==0
           && p_ਵਸਤੂ->fg.isSubquery==0
           && p_ਵਸਤੂ->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, p_ਵਸਤੂ->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, p_ਵਸਤੂ->zName);
        }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->zAlias ){
          sqlite3_str_appendall(pAccum, p_ਵਸਤੂ->zAlias);
        }ਨਹੀਂਤਾਂ ਜੇ( ALWAYS(p_ਵਸਤੂ->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = p_ਵਸਤੂ->u4.pSubq->pSelect;
          ਪ੍ਰਤਿਜ੍ਞਾ( pSel!=0 ); 
          ਜੇ( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }ਨਹੀਂਤਾਂ ਜੇ( pSel->selFlags & SF_Multi_ਮੁੱਲ ){
            ਪ੍ਰਤਿਜ੍ਞਾ( !p_ਵਸਤੂ->fg.isTabFunc && !p_ਵਸਤੂ->fg.is_ਸੂਚਕਾਂਕਿਤ_By );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                p_ਵਸਤੂ->u1.n_ਪੰਕਤੀ);
          }ਨਹੀਂਤਾਂ{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        ਲੰਮਾਈ = ਚੌੜਾਈ = 0;
        ਰੋਕੋ;
      }
      ਮੂਲ: {
        ਪ੍ਰਤਿਜ੍ਞਾ( xtype==etINVALID );
        ਵਾਪਸ;
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
    ਚੌੜਾਈ -= ਲੰਮਾਈ;
    ਜੇ( ਚੌੜਾਈ>0 ){
      ਜੇ( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, ਚੌੜਾਈ, ' ');
      sqlite3StrAppend64(pAccum, bufpt, ਲੰਮਾਈ);
      ਜੇ( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, ਚੌੜਾਈ, ' ');
    }ਨਹੀਂਤਾਂ{
      sqlite3StrAppend64(pAccum, bufpt, ਲੰਮਾਈ);
    }

    ਜੇ( zExtra ){
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
ਖਾਲੀ sqlite3_ਰਿਕਾਰਡ_Error_Byte_ਵਿੱਥ(sqlite3 *db, ਅਚਲ ਅੱਖਰ *z){
  ਅਚਲ Parse *pParse;
  ਅਚਲ ਅੱਖਰ *z_ਪਾਠ;
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( z!=0 );
  ਜੇ( NEVER(db==0) ) ਵਾਪਸ;
  ਜੇ( db->err_Byte_ਵਿੱਥ!=(-2) ) ਵਾਪਸ;
  pParse = db->pParse;
  ਜੇ( NEVER(pParse==0) ) ਵਾਪਸ;
  z_ਪਾਠ =pParse->z_ਪੂਛ;
  ਜੇ( NEVER(z_ਪਾਠ==0) ) ਵਾਪਸ;
  z_ਸਿਰਾ = &z_ਪਾਠ[ਲੜੀ_ਲੰਬਾਈ(z_ਪਾਠ)];
  ਜੇ( SQLITE_WITHIN(z,z_ਪਾਠ,z_ਸਿਰਾ) ){
    db->err_Byte_ਵਿੱਥ = (ਪੂਰਨ)(z-z_ਪਾਠ);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
ਖਾਲੀ sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(sqlite3 *db, ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜਦੋਂ( p_ਵਿਅੰਜਕ ){
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Reduced|EP_TokenOnly) ) ਵਾਪਸ;
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_OuterON|EP_InnerON) || p_ਵਿਅੰਜਕ->w.iOfst<=0 ){
      p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
    }ਨਹੀਂਤਾਂ{
      ਰੋਕੋ;
    }
  }
  ਜੇ( p_ਵਿਅੰਜਕ==0 ) ਵਾਪਸ;
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_FromDDL) ) ਵਾਪਸ;
  db->err_Byte_ਵਿੱਥ = p_ਵਿਅੰਜਕ->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
ਪੂਰਨ sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  ਅੱਖਰ *zNew;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  ਜੇ( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    ਵਾਪਸ 0;
  }
  ਜੇ( p->mxAlloc==0 ){
    sqlite3_Str_Accum_ਸੈੱਟ_Error(p, SQLITE_TOOBIG);
    ਵਾਪਸ p->nAlloc - p->nChar - 1;
  }ਨਹੀਂਤਾਂ{
    ਅੱਖਰ *zOld = isMalloced(p) ? p->z_ਪਾਠ : 0;
    i64 szNew = p->nChar + N + 1;
    ਜੇ( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    ਜੇ( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_ਸੈੱਟ_Error(p, SQLITE_TOOBIG);
      ਵਾਪਸ 0;
    }ਨਹੀਂਤਾਂ{
      p->nAlloc = (ਪੂਰਨ)szNew;
    }
    ਜੇ( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }ਨਹੀਂਤਾਂ{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    ਜੇ( zNew ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->z_ਪਾਠ!=0 || p->nChar==0 );
      ਜੇ( !isMalloced(p) && p->nChar>0 ) ਸਮ੍ਰਿਤੀ_ਨਕਲ(zNew, p->z_ਪਾਠ, p->nChar);
      p->z_ਪਾਠ = zNew;
      p->nAlloc = sqlite3_Db_Malloc_ਮਾਪ(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }ਨਹੀਂਤਾਂ{
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_ਸੈੱਟ_Error(p, SQLITE_NOMEM);
      ਵਾਪਸ 0;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( N>=0 && N<=0x7fffffff );
  ਵਾਪਸ (ਪੂਰਨ)N;
}

ਪੂਰਨ sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  ਜੇ( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  ਵਾਪਸ p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
ਖਾਲੀ sqlite3_str_appendchar(sqlite3_str *p, ਪੂਰਨ N, ਅੱਖਰ c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  ਜੇ( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ਵਾਪਸ;
  }
  ਜਦੋਂ( (N--)>0 ) p->z_ਪਾਠ[p->nChar++] = c;
}
ਸਥਿਰ ਖਾਲੀ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, ਅੱਖਰ c){
  testcase( p->nChar + N > 0x7fffffff );
  ਜੇ( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ਵਾਪਸ;
  }
  ਜਦੋਂ( (N--)>0 ) p->z_ਪਾਠ[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
ਸਥਿਰ ਖਾਲੀ SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ N){
  N = sqlite3StrAccumEnlarge(p, N);
  ਪ੍ਰਤਿਜ੍ਞਾ( z!=0 || N==0 );
  ਜੇ( N>0 ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->z_ਪਾਠ[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
ਖਾਲੀ sqlite3_str_append(sqlite3_str *p, ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ N){
  ਪ੍ਰਤਿਜ੍ਞਾ( z!=0 || N==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->z_ਪਾਠ!=0 || p->nChar==0 || p->accError );
  ਪ੍ਰਤਿਜ੍ਞਾ( N>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ਜੇ( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ਨਹੀਂਤਾਂ ਜੇ( N ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->z_ਪਾਠ );
    p->nChar += N;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->z_ਪਾਠ[p->nChar-N], z, N);
  }
}
ਸਥਿਰ ਖਾਲੀ sqlite3StrAppend64(sqlite3_str *p, ਅਚਲ ਅੱਖਰ *z, i64 N){
  ਪ੍ਰਤਿਜ੍ਞਾ( z!=0 || N==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->z_ਪਾਠ!=0 || p->nChar==0 || p->accError );
  ਪ੍ਰਤਿਜ੍ਞਾ( N>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ਜੇ( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ਨਹੀਂਤਾਂ ਜੇ( N ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->z_ਪਾਠ );
    p->nChar += N;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->z_ਪਾਠ[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
ਖਾਲੀ sqlite3_str_appendall(sqlite3_str *p, ਅਚਲ ਅੱਖਰ *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
ਸਥਿਰ SQLITE_NOINLINE ਅੱਖਰ *str_Accum_ਅੰਤ_Realloc(StrAccum *p){
  ਅੱਖਰ *z_ਪਾਠ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->mxAlloc>0 && !isMalloced(p) );
  z_ਪਾਠ = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  ਜੇ( z_ਪਾਠ ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_ਪਾਠ, p->z_ਪਾਠ, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }ਨਹੀਂਤਾਂ{
    sqlite3_Str_Accum_ਸੈੱਟ_Error(p, SQLITE_NOMEM);
  }
  p->z_ਪਾਠ = z_ਪਾਠ;
  ਵਾਪਸ z_ਪਾਠ;
}
ਅੱਖਰ *sqlite3_Str_Accum_ਅੰਤ(StrAccum *p){
  ਜੇ( p->z_ਪਾਠ ){
    p->z_ਪਾਠ[p->nChar] = 0;
    ਜੇ( p->mxAlloc>0 && !isMalloced(p) ){
      ਵਾਪਸ str_Accum_ਅੰਤ_Realloc(p);
    }
  }
  ਵਾਪਸ p->z_ਪਾਠ;
}

/* Finalize a string created using sqlite3_str_new().
*/
ਅੱਖਰ *sqlite3_str_ਅੰਤ(sqlite3_str *p){
  ਅੱਖਰ *z;
  ਜੇ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3_Str_Accum_ਅੰਤ(p);
    sqlite3_free(p);
  }ਨਹੀਂਤਾਂ{
    z = 0;
  }
  ਵਾਪਸ z;
}

/* Return any error code associated with p */
ਪੂਰਨ sqlite3_str_errcode(sqlite3_str *p){
  ਵਾਪਸ p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
ਪੂਰਨ sqlite3_str_ਲੰਮਾਈ(sqlite3_str *p){
  ਵਾਪਸ p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
ਖਾਲੀ sqlite3_str_truncate(sqlite3_str *p, ਪੂਰਨ N){
  ਜੇ( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->z_ਪਾਠ[p->nChar] = 0;
  }
}

/* Return the current value for p */
ਅੱਖਰ *sqlite3_str_ਮੁੱਲ(sqlite3_str *p){
  ਜੇ( p==0 || p->nChar==0 ) ਵਾਪਸ 0;
  p->z_ਪਾਠ[p->nChar] = 0;
  ਵਾਪਸ p->z_ਪਾਠ;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
ਖਾਲੀ sqlite3_str_reset(StrAccum *p){
  ਜੇ( isMalloced(p) ){
    sqlite3DbFree(p->db, p->z_ਪਾਠ);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }ਨਹੀਂਤਾਂ ਜੇ( p==(sqlite3_str*)&sqlite3OomStr ){
    ਵਾਪਸ;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->z_ਪਾਠ = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
ਖਾਲੀ sqlite3_str_free(sqlite3_str *p){
  ਜੇ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
ਖਾਲੀ sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, ਅੱਖਰ *z_ਅਧਾਰ, ਪੂਰਨ n, ਪੂਰਨ mx){
  p->z_ਪਾਠ = z_ਅਧਾਰ;
  p->db = db;
  p->nAlloc = n;
  p->mxAlloc = mx;
  p->nChar = 0;
  p->accError = 0;
  p->printfFlags = 0;
}

/* Allocate and initialize a new dynamic string object */
sqlite3_str *sqlite3_str_new(sqlite3 *db){
  sqlite3_str *p = sqlite3_malloc64(ਆਕਾਰ(*p));
  ਜੇ( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ] : SQLITE_MAX_ਲੰਮਾਈ);
  }ਨਹੀਂਤਾਂ{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  ਵਾਪਸ p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
ਅੱਖਰ *sqlite3VMPrintf(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zFormat, va_list ap){
  ਅੱਖਰ *z;
  ਅੱਖਰ z_ਅਧਾਰ[SQLITE_PRINT_BUF_ਮਾਪ];
  StrAccum acc;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  sqlite3StrAccumInit(&acc, db, z_ਅਧਾਰ, ਆਕਾਰ(z_ਅਧਾਰ),
                      db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ]);
  acc.printfFlags = SQLITE_PRINTF_ਅੰਦਰੂਨੀ;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_ਅੰਤ(&acc);
  ਜੇ( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ਵਾਪਸ z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
ਅੱਖਰ *sqlite3MPrintf(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;
  ਅੱਖਰ *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ਵਾਪਸ z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
ਅੱਖਰ *sqlite3_vmprintf(ਅਚਲ ਅੱਖਰ *zFormat, va_list ap){
  ਅੱਖਰ *z;
  ਅੱਖਰ z_ਅਧਾਰ[SQLITE_PRINT_BUF_ਮਾਪ];
  StrAccum acc;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR  
  ਜੇ( zFormat==0 ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  sqlite3StrAccumInit(&acc, 0, z_ਅਧਾਰ, ਆਕਾਰ(z_ਅਧਾਰ), SQLITE_MAX_ਲੰਮਾਈ);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_ਅੰਤ(&acc);
  ਵਾਪਸ z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
ਅੱਖਰ *sqlite3_mprintf(ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;
  ਅੱਖਰ *z;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  ਵਾਪਸ z;
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
ਅੱਖਰ *sqlite3_vsnprintf(ਪੂਰਨ n, ਅੱਖਰ *zBuf, ਅਚਲ ਅੱਖਰ *zFormat, va_list ap){
  StrAccum acc;
  ਜੇ( n<=0 ) ਵਾਪਸ zBuf;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( zBuf==0 || zFormat==0 ) {
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਜੇ( zBuf ) zBuf[0] = 0;
    ਵਾਪਸ zBuf;
  }
#ਜੇ_ਅੰਤ
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  ਵਾਪਸ zBuf;
}
ਅੱਖਰ *sqlite3_snprintf(ਪੂਰਨ n, ਅੱਖਰ *zBuf, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  StrAccum acc;
  va_list ap;
  ਜੇ( n<=0 ) ਵਾਪਸ zBuf;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( zBuf==0 || zFormat==0 ) {
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਜੇ( zBuf ) zBuf[0] = 0;
    ਵਾਪਸ zBuf;
  }
#ਜੇ_ਅੰਤ
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  ਵਾਪਸ zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#ਜੇ defined(SQLITE_MAX_LOG_ਸੁਨੇਹਾ) 
  /* Leave the definition as supplied */
#ਜਾਂਜੇ SQLITE_PRINT_BUF_ਮਾਪ*10>10000
# define SQLITE_MAX_LOG_ਸੁਨੇਹਾ 10000
#ਨਹੀਂਤਾਂ
# define SQLITE_MAX_LOG_ਸੁਨੇਹਾ (SQLITE_PRINT_BUF_ਮਾਪ*10)
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਖਾਲੀ render_Log_ਸੁਨੇਹਾ(ਪੂਰਨ i_Err_ਕੋਡ, ਅਚਲ ਅੱਖਰ *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  ਅੱਖਰ z_ਸੁਨੇਹਾ[SQLITE_MAX_LOG_ਸੁਨੇਹਾ];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_ਸੁਨੇਹਾ, ਆਕਾਰ(z_ਸੁਨੇਹਾ), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, i_Err_ਕੋਡ,
                           sqlite3_Str_Accum_ਅੰਤ(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
ਖਾਲੀ sqlite3_log(ਪੂਰਨ i_Err_ਕੋਡ, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;                             /* Vararg list */
  ਜੇ( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_ਸੁਨੇਹਾ(i_Err_ਕੋਡ, zFormat, ap);
    va_end(ap);
  }
}

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
ਖਾਲੀ sqlite3DebugPrintf(ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;
  StrAccum acc;
  ਅੱਖਰ zBuf[SQLITE_PRINT_BUF_ਮਾਪ*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, ਆਕਾਰ(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3_Str_Accum_ਅੰਤ(&acc);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OS_TRACE_PROC
  {
    ਬਾਹਰੀ ਖਾਲੀ SQLITE_OS_TRACE_PROC(ਅਚਲ ਅੱਖਰ *zBuf, ਪੂਰਨ nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, ਆਕਾਰ(zBuf));
  }
#ਨਹੀਂਤਾਂ
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
ਖਾਲੀ sqlite3_str_appendf(StrAccum *p, ਅਚਲ ਅੱਖਰ *zFormat, ...){
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
ਅੱਖਰ *sqlite3RCStrRef(ਅੱਖਰ *z){
  RCStr *p = (RCStr*)z;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  p--;
  p->nRCRef++;
  ਵਾਪਸ z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
ਖਾਲੀ sqlite3RCStrUnref(ਖਾਲੀ *z){
  RCStr *p = (RCStr*)z;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  p--;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRCRef>0 );
  ਜੇ( p->nRCRef>=2 ){
    p->nRCRef--;
  }ਨਹੀਂਤਾਂ{
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
ਅੱਖਰ *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + ਆਕਾਰ(*p) + 1 );
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  p->nRCRef = 1;
  ਵਾਪਸ (ਅੱਖਰ*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
ਅੱਖਰ *sqlite3RCStrResize(ਅੱਖਰ *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  p--;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+ਆਕਾਰ(RCStr)+1);
  ਜੇ( pNew==0 ){
    sqlite3_free(p);
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ (ਅੱਖਰ*)&pNew[1];
  }
}
