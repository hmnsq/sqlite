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
#അന്തര്ഭാവ "sqliteInt.h"

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
തരനിർവചനം unsigned അക്ഷരം etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
തരനിർവചനം ഘടന et_info {   /* Information about each format field */
  അക്ഷരം fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  അക്ഷരം iNxt;               /* Next with same hash, or 0 for end of chain */
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
#എങ്കിൽ 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
വേണ്ടി {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    എങ്കിൽ {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

സ്ഥിതം സ്ഥിരം അക്ഷരം aDigits[] = "0123456789ABCDEF0123456789abcdef";
സ്ഥിതം സ്ഥിരം അക്ഷരം aHex[]    = "0123456789abcdef";
സ്ഥിതം സ്ഥിരം അക്ഷരം aPrefix[] = "-x0\000X0";
സ്ഥിതം സ്ഥിരം et_info fmtinfo[25] = {
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
ശൂന്യം sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  പ്രതിജ്ഞാ( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  എങ്കിൽ( p->mxAlloc ) sqlite3_str_reset(p);
  എങ്കിൽ( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
സ്ഥിതം sqlite3_int64 getIntArg(PrintfArguments *p){
  എങ്കിൽ( p->nArg<=p->nUsed ) തിരികെ 0;
  തിരികെ sqlite3_മൂല്യം_int64(p->apArg[p->nUsed++]);
}
സ്ഥിതം ഇരട്ട getDoubleArg(PrintfArguments *p){
  എങ്കിൽ( p->nArg<=p->nUsed ) തിരികെ 0.0;
  തിരികെ sqlite3_മൂല്യം_double(p->apArg[p->nUsed++]);
}
സ്ഥിതം അക്ഷരം *getTextArg(PrintfArguments *p){
  എങ്കിൽ( p->nArg<=p->nUsed ) തിരികെ 0;
  തിരികെ (അക്ഷരം*)sqlite3_മൂല്യം_text(p->apArg[p->nUsed++]);
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
സ്ഥിതം അക്ഷരം *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  അക്ഷരം *z;
  എങ്കിൽ( pAccum->accError ) തിരികെ 0;
  എങ്കിൽ( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    തിരികെ 0;
  }
  z = sqlite3_malloc(n);
  എങ്കിൽ( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  തിരികെ z;
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
# എങ്കിൽ SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# അല്ലെങ്കിൽ
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#അല്ലെങ്കിൽ
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
സ്ഥിതം ശൂന്യം sqlite3StrAppend64(sqlite3_str *p, സ്ഥിരം അക്ഷരം *z, i64 N);
സ്ഥിതം ശൂന്യം sqlite3StrAppendchar64(sqlite3_str *p, i64 N, അക്ഷരം c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
ശൂന്യം sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  സ്ഥിരം അക്ഷരം *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  പൂർണ്ണസംഖ്യ c;                     /* Next character in the format string */
  അക്ഷരം *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  പൂർണ്ണസംഖ്യ idx;                   /* A general purpose loop counter */
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
  അക്ഷരം prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  ഇരട്ട realvalue;          /* Value for real types */
  സ്ഥിരം et_info *infop;      /* Pointer to the appropriate info structure */
  അക്ഷരം *zOut;                /* Rendering buffer */
  പൂർണ്ണസംഖ്യ nOut;                  /* Size of the rendering buffer */
  അക്ഷരം *zExtra = 0;          /* Malloced memory used by some conversion */
  പൂർണ്ണസംഖ്യ exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  അക്ഷരം buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  പ്രതിജ്ഞാ( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  എങ്കിൽ( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }അല്ലെങ്കിൽ{
    bArgList = 0;
  }
  വേണ്ടി(; (c=(*fmt))!=0; ++fmt){
    എങ്കിൽ( c!='%' ){
      bufpt = (അക്ഷരം *)fmt;
#എങ്കിൽ HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#അല്ലെങ്കിൽ
      fmt = strchr(fmt, '%');
      എങ്കിൽ( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      എങ്കിൽ( *fmt==0 ) നിർത്തുക;
    }
    എങ്കിൽ( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      നിർത്തുക;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    ചെയ്യുക{
      തിരഞ്ഞെടുപ്പ്( c ){
        അവസ്ഥ '-':   flag_leftjustify = 1;     നിർത്തുക;
        അവസ്ഥ '+':   flag_prefix = '+';        നിർത്തുക;
        അവസ്ഥ ' ':   flag_prefix = ' ';        നിർത്തുക;
        അവസ്ഥ '#':   flag_alternateform = 1;   നിർത്തുക;
        അവസ്ഥ '!':   flag_altform2 = 1;        നിർത്തുക;
        അവസ്ഥ '0':   flag_zeropad = 1;         നിർത്തുക;
        അവസ്ഥ ',':   cThousand = ',';          നിർത്തുക;
        സ്ഥിരസ്ഥിതി:    done = 1;                 നിർത്തുക;
        അവസ്ഥ 'l': {
          flag_long = 1;
          c = *++fmt;
          എങ്കിൽ( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          നിർത്തുക;
        }
        അവസ്ഥ '1': അവസ്ഥ '2': അവസ്ഥ '3': അവസ്ഥ '4': അവസ്ഥ '5':
        അവസ്ഥ '6': അവസ്ഥ '7': അവസ്ഥ '8': അവസ്ഥ '9': {
          unsigned wx = c - '0';
          ആകുമ്പോൾ( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          എങ്കിൽ( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          എങ്കിൽ( c!='.' && c!='l' ){
            done = 1;
          }അല്ലെങ്കിൽ{
            fmt--;
          }
          നിർത്തുക;
        }
        അവസ്ഥ '*': {
          എങ്കിൽ( bArgList ){
            width = (പൂർണ്ണസംഖ്യ)getIntArg(pArgList);
          }അല്ലെങ്കിൽ{
            width = va_arg(ap,പൂർണ്ണസംഖ്യ);
          }
          എങ്കിൽ( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          എങ്കിൽ( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          എങ്കിൽ( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          നിർത്തുക;
        }
        അവസ്ഥ '.': {
          c = *++fmt;
          എങ്കിൽ( c=='*' ){
            എങ്കിൽ( bArgList ){
              precision = (പൂർണ്ണസംഖ്യ)getIntArg(pArgList);
            }അല്ലെങ്കിൽ{
              precision = va_arg(ap,പൂർണ്ണസംഖ്യ);
            }
            എങ്കിൽ( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }അല്ലെങ്കിൽ{
            unsigned px = 0;
            ആകുമ്പോൾ( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          എങ്കിൽ( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          എങ്കിൽ( c=='l' ){
            --fmt;
          }അല്ലെങ്കിൽ{
            done = 1;
          }
          നിർത്തുക;
        }
      }
    }ആകുമ്പോൾ( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    വേണ്ടി(idx=0; idx<ArraySize(fmtinfo); idx++){
      എങ്കിൽ( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        നിർത്തുക;
      }
    }
#അല്ലെങ്കിൽ
    /* Fast hash-table lookup */
    പ്രതിജ്ഞാ( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    എങ്കിൽ( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }അല്ലെങ്കിൽ{
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
    പ്രതിജ്ഞാ( width>=0 );
    പ്രതിജ്ഞാ( precision>=(-1) );
    തിരഞ്ഞെടുപ്പ്( xtype ){
      അവസ്ഥ etPOINTER:
        flag_long = വലുപ്പം(അക്ഷരം*)==വലുപ്പം(i64) ? 2 :
                     വലുപ്പം(അക്ഷരം*)==വലുപ്പം(long പൂർണ്ണസംഖ്യ) ? 1 : 0;
        /* no break */ deliberate_fall_through
      അവസ്ഥ etORDINAL:
      അവസ്ഥ etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      അവസ്ഥ etDECIMAL:
        എങ്കിൽ( infop->flags & FLAG_SIGNED ){
          i64 v;
          എങ്കിൽ( bArgList ){
            v = getIntArg(pArgList);
          }അല്ലെങ്കിൽ എങ്കിൽ( flag_long ){
            എങ്കിൽ( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }അല്ലെങ്കിൽ{
              v = va_arg(ap,long പൂർണ്ണസംഖ്യ);
            }
          }അല്ലെങ്കിൽ{
            v = va_arg(ap,പൂർണ്ണസംഖ്യ);
          }
          എങ്കിൽ( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }അല്ലെങ്കിൽ{
            longvalue = v;
            prefix = flag_prefix;
          }
        }അല്ലെങ്കിൽ{
          എങ്കിൽ( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }അല്ലെങ്കിൽ എങ്കിൽ( flag_long ){
            എങ്കിൽ( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }അല്ലെങ്കിൽ{
              longvalue = va_arg(ap,unsigned long പൂർണ്ണസംഖ്യ);
            }
          }അല്ലെങ്കിൽ{
            longvalue = va_arg(ap,unsigned പൂർണ്ണസംഖ്യ);
          }
          prefix = 0;
        }

#എങ്കിൽ WHERETRACE_ENABLED
        എങ്കിൽ( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#എങ്കിൽ TREETRACE_ENABLED
        എങ്കിൽ( xtype==etPOINTER && sqlite3_വൃക്ഷം_Trace & 0x100000 ) longvalue = 0;
#endif

        എങ്കിൽ( longvalue==0 ) flag_alternateform = 0;
        എങ്കിൽ( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        എങ്കിൽ( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }അല്ലെങ്കിൽ{
          u64 n;
          n = (u64)precision + 10;
          എങ്കിൽ( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          എങ്കിൽ( zOut==0 ) തിരികെ;
          nOut = (പൂർണ്ണസംഖ്യ)n;
        }
        bufpt = &zOut[nOut-1];
        എങ്കിൽ( xtype==etORDINAL ){
          സ്ഥിതം സ്ഥിരം അക്ഷരം zOrd[] = "thstndrd";
          പൂർണ്ണസംഖ്യ x = (പൂർണ്ണസംഖ്യ)(longvalue % 10);
          എങ്കിൽ( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          സ്ഥിരം അക്ഷരം *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          ചെയ്യുക{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }ആകുമ്പോൾ( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        എങ്കിൽ( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        എങ്കിൽ( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          പൂർണ്ണസംഖ്യ ii;
          bufpt -= nn;
          വേണ്ടി(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            എങ്കിൽ( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        എങ്കിൽ( prefix ) *(--bufpt) = prefix;               /* Add sign */
        എങ്കിൽ( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          സ്ഥിരം അക്ഷരം *pre;
          അക്ഷരം x;
          pre = &aPrefix[infop->prefix];
          വേണ്ടി(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        നിർത്തുക;
      അവസ്ഥ etFLOAT:
      അവസ്ഥ etEXP:
      അവസ്ഥ etGENERIC: {
        FpDecode s;
        പൂർണ്ണസംഖ്യ iRound;
        പൂർണ്ണസംഖ്യ j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        എങ്കിൽ( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }അല്ലെങ്കിൽ{
          realvalue = va_arg(ap,ഇരട്ട);
        }
        എങ്കിൽ( precision<0 ) precision = 6;         /* Set default precision */
        എങ്കിൽ( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        എങ്കിൽ( xtype==etFLOAT ){
          iRound = -precision;
        }അല്ലെങ്കിൽ എങ്കിൽ( xtype==etGENERIC ){
          എങ്കിൽ( precision==0 ) precision = 1;
          iRound = precision;
        }അല്ലെങ്കിൽ{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        എങ്കിൽ( s.isSpecial ){
          എങ്കിൽ( s.isSpecial==2 ){
            എങ്കിൽ( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }അല്ലെങ്കിൽ{
              bufpt = "NaN";
              length = 3;
            }
            നിർത്തുക;
          }അല്ലെങ്കിൽ എങ്കിൽ( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }അല്ലെങ്കിൽ{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            എങ്കിൽ( s.sign=='-' ){
              /* no-op */
            }അല്ലെങ്കിൽ എങ്കിൽ( flag_prefix ){
              buf[0] = flag_prefix;
            }അല്ലെങ്കിൽ{
              bufpt++;
            }
            length = strlen(bufpt);
            നിർത്തുക;
          }
        }
        എങ്കിൽ( s.sign=='-' ){
          എങ്കിൽ( flag_alternateform
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
          }അല്ലെങ്കിൽ{
            prefix = '-';
          }
        }അല്ലെങ്കിൽ{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        എങ്കിൽ( xtype==etGENERIC ){
          പ്രതിജ്ഞാ( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          എങ്കിൽ( exp<-4 || exp>precision ){
            xtype = etEXP;
          }അല്ലെങ്കിൽ{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }അല്ലെങ്കിൽ{
          flag_rtz = flag_altform2;
        }
        എങ്കിൽ( xtype==etEXP ){
          e2 = 0;
        }അല്ലെങ്കിൽ{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        എങ്കിൽ( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        എങ്കിൽ( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          എങ്കിൽ( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            എങ്കിൽ( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              തിരികെ;
            }
            zExtra = bufpt;
          }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            നിർത്തുക;
          }അല്ലെങ്കിൽ{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }അല്ലെങ്കിൽ{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        എങ്കിൽ( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        പ്രതിജ്ഞാ( s.n>0 );
        എങ്കിൽ( e2<0 ){
          *(bufpt++) = '0';
        }അല്ലെങ്കിൽ എങ്കിൽ( cThousand ){
          വേണ്ടി(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            എങ്കിൽ( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }അല്ലെങ്കിൽ{
          j = e2+1;
          എങ്കിൽ( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          എങ്കിൽ( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        എങ്കിൽ( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        എങ്കിൽ( e2<(-1) && precision>0 ){
          പൂർണ്ണസംഖ്യ nn = -1-e2;
          എങ്കിൽ( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        എങ്കിൽ( precision>0 ){
          പൂർണ്ണസംഖ്യ nn = s.n - j;
          എങ്കിൽ( NEVER(nn>precision) ) nn = precision;
          എങ്കിൽ( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          എങ്കിൽ( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        എങ്കിൽ( flag_rtz && flag_dp ){
          ആകുമ്പോൾ( bufpt[-1]=='0' ) *(--bufpt) = 0;
          പ്രതിജ്ഞാ( bufpt>zOut );
          എങ്കിൽ( bufpt[-1]=='.' ){
            എങ്കിൽ( flag_altform2 ){
              *(bufpt++) = '0';
            }അല്ലെങ്കിൽ{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        എങ്കിൽ( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          എങ്കിൽ( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }അല്ലെങ്കിൽ{
            *(bufpt++) = '+';
          }
          എങ്കിൽ( exp>=100 ){
            *(bufpt++) = (അക്ഷരം)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (അക്ഷരം)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (അക്ഷരം)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        പ്രതിജ്ഞാ( length <= szBufNeeded );
        എങ്കിൽ( length<width ){
          i64 nPad = width - length;
          എങ്കിൽ( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }അല്ലെങ്കിൽ എങ്കിൽ( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }അല്ലെങ്കിൽ{
            പൂർണ്ണസംഖ്യ adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        എങ്കിൽ( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          പ്രതിജ്ഞാ( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          തുടരുക;
        }അല്ലെങ്കിൽ{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          നിർത്തുക;
        }
      }
      അവസ്ഥ etSIZE:
        എങ്കിൽ( !bArgList ){
          *(va_arg(ap,പൂർണ്ണസംഖ്യ*)) = pAccum->nChar;
        }
        length = width = 0;
        നിർത്തുക;
      അവസ്ഥ etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        നിർത്തുക;
      അവസ്ഥ etCHARX:
        എങ്കിൽ( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          എങ്കിൽ( bufpt ){
            buf[0] = c = *(bufpt++);
            എങ്കിൽ( (c&0xc0)==0xc0 ){
              ആകുമ്പോൾ( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }അല്ലെങ്കിൽ{
            buf[0] = 0;
          }
        }അല്ലെങ്കിൽ{
          unsigned പൂർണ്ണസംഖ്യ ch = va_arg(ap,unsigned പൂർണ്ണസംഖ്യ);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        എങ്കിൽ( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          എങ്കിൽ( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          ആകുമ്പോൾ( precision > 1 ){
            i64 nCopyBytes;
            എങ്കിൽ( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            എങ്കിൽ( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              നിർത്തുക;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        പോകുക adjust_width_for_utf8;
      അവസ്ഥ etSTRING:
      അവസ്ഥ etDYNSTRING:
        എങ്കിൽ( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }അല്ലെങ്കിൽ{
          bufpt = va_arg(ap,അക്ഷരം*);
        }
        എങ്കിൽ( bufpt==0 ){
          bufpt = "";
        }അല്ലെങ്കിൽ എങ്കിൽ( xtype==etDYNSTRING ){
          എങ്കിൽ( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            പ്രതിജ്ഞാ( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (പൂർണ്ണസംഖ്യ)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            നിർത്തുക;
          }
          zExtra = bufpt;
        }
        എങ്കിൽ( precision>=0 ){
          എങ്കിൽ( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned അക്ഷരം *z = (unsigned അക്ഷരം*)bufpt;
            ആകുമ്പോൾ( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned അക്ഷരം*)bufpt;
          }അല്ലെങ്കിൽ{
            വേണ്ടി(length=0; length<precision && bufpt[length]; length++){}
          }
        }അല്ലെങ്കിൽ{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        എങ്കിൽ( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          ആകുമ്പോൾ( ii>=0 ) എങ്കിൽ( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        നിർത്തുക;
      അവസ്ഥ etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      അവസ്ഥ etESCAPE_J: {         /* %J: Generate a JSON string literal */
        അക്ഷരം *escarg;
        i64 i, j, px, iStart;
        unsigned അക്ഷരം ch;

        എങ്കിൽ( bArgList ){
          escarg = getTextArg(pArgList);
        }അല്ലെങ്കിൽ{
          escarg = va_arg(ap,അക്ഷരം*);
        }
        iStart = sqlite3_str_length(pAccum);
        എങ്കിൽ( escarg==0 ){
          എങ്കിൽ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }അല്ലെങ്കിൽ{
          എങ്കിൽ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          എങ്കിൽ( px<0 ){
            px = 0x7fffffff;
          }അല്ലെങ്കിൽ എങ്കിൽ( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            വേണ്ടി(i=0; i<px && escarg[i]; i++){
              എങ്കിൽ( (escarg[i]&0xc0)==0x80 ) px++;
            }
            എങ്കിൽ( i==px ){
              ആകുമ്പോൾ( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          വേണ്ടി(i=j=0; i<px; i++){
            എങ്കിൽ( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              എങ്കിൽ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              എങ്കിൽ( ch==0 ) നിർത്തുക;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              എങ്കിൽ( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }അല്ലെങ്കിൽ എങ്കിൽ( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }അല്ലെങ്കിൽ{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          എങ്കിൽ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          എങ്കിൽ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        എങ്കിൽ( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          അക്ഷരം *zz;
          എങ്കിൽ( flag_altform2 && n>0 ){
            zz = sqlite3_str_മൂല്യം(pAccum);
            വേണ്ടി(i=iStart; zz[i]; i++){
              എങ്കിൽ( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          എങ്കിൽ( width>len ){
            sqlite3_int64 sp = width-len;
            പ്രതിജ്ഞാ( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (പൂർണ്ണസംഖ്യ)sp, ' ');
            എങ്കിൽ( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_മൂല്യം(pAccum);
              zz += iStart;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        തുടരുക;
      }
      അവസ്ഥ etESCAPE_q:          /* %q: Escape ' characters */
      അവസ്ഥ etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      അവസ്ഥ etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        പൂർണ്ണസംഖ്യ needQuote = 0;
        അക്ഷരം ch;
        അക്ഷരം *escarg;
        അക്ഷരം q;

        എങ്കിൽ( bArgList ){
          escarg = getTextArg(pArgList);
        }അല്ലെങ്കിൽ{
          escarg = va_arg(ap,അക്ഷരം*);
        }
        എങ്കിൽ( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }അല്ലെങ്കിൽ എങ്കിൽ( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        എങ്കിൽ( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }അല്ലെങ്കിൽ{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        വേണ്ടി(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          എങ്കിൽ( ch==q )  n++;
          എങ്കിൽ( flag_altform2 && (ch&0xc0)==0xc0 ){
            ആകുമ്പോൾ( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        എങ്കിൽ( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          വേണ്ടി(k=0; k<i; k++){
            എങ്കിൽ( escarg[k]=='\\' ){
              nBack++;
            }അല്ലെങ്കിൽ എങ്കിൽ( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          എങ്കിൽ( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            എങ്കിൽ( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }അല്ലെങ്കിൽ{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        എങ്കിൽ( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          എങ്കിൽ( bufpt==0 ) തിരികെ;
        }അല്ലെങ്കിൽ{
          bufpt = buf;
        }
        j = 0;
        എങ്കിൽ( needQuote ){
          എങ്കിൽ( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }അല്ലെങ്കിൽ{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        എങ്കിൽ( flag_alternateform ){
          വേണ്ടി(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            എങ്കിൽ( ch==q ){
              bufpt[j++] = ch;
            }അല്ലെങ്കിൽ എങ്കിൽ( ch=='\\' ){
              bufpt[j++] = '\\';
            }അല്ലെങ്കിൽ എങ്കിൽ( ((unsigned അക്ഷരം)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }അല്ലെങ്കിൽ{
          വേണ്ടി(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            എങ്കിൽ( ch==q ) bufpt[j++] = ch;
          }
        }
        എങ്കിൽ( needQuote ){
          bufpt[j++] = '\'';
          എങ്കിൽ( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        പോകുക adjust_width_for_utf8;
      }
      അവസ്ഥ etTOKEN: {
        എങ്കിൽ( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) തിരികെ;
        എങ്കിൽ( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          വ്യഞ്ജകം *p_വ്യഞ്ജകം = va_arg(ap,വ്യഞ്ജകം*);
          എങ്കിൽ( ALWAYS(p_വ്യഞ്ജകം) && ALWAYS(!വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_Int_മൂല്യം)) ){
            sqlite3_str_appendall(pAccum, (സ്ഥിരം അക്ഷരം*)p_വ്യഞ്ജകം->u.zToken);
            sqlite3_Record_Error_Offset_Of_വ്യഞ്ജകം(pAccum->db, p_വ്യഞ്ജകം);
          }
        }അല്ലെങ്കിൽ{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          പ്രതിജ്ഞാ( bArgList==0 );
          എങ്കിൽ( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (സ്ഥിരം അക്ഷരം*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        നിർത്തുക;
      }
      അവസ്ഥ etSRCITEM: {
        SrcItem *pItem;
        എങ്കിൽ( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) തിരികെ;
        pItem = va_arg(ap, SrcItem*);
        പ്രതിജ്ഞാ( bArgList==0 );
        എങ്കിൽ( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }അല്ലെങ്കിൽ എങ്കിൽ( pItem->zName ){
          എങ്കിൽ( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }അല്ലെങ്കിൽ എങ്കിൽ( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }അല്ലെങ്കിൽ എങ്കിൽ( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          പ്രതിജ്ഞാ( pSel!=0 ); 
          എങ്കിൽ( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }അല്ലെങ്കിൽ എങ്കിൽ( pSel->selFlags & SF_Multi_മൂല്യം ){
            പ്രതിജ്ഞാ( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }അല്ലെങ്കിൽ{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: {
        പ്രതിജ്ഞാ( xtype==etINVALID );
        തിരികെ;
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
    എങ്കിൽ( width>0 ){
      എങ്കിൽ( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      എങ്കിൽ( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }അല്ലെങ്കിൽ{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    എങ്കിൽ( zExtra ){
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
ശൂന്യം sqlite3RecordErrorByteOffset(sqlite3 *db, സ്ഥിരം അക്ഷരം *z){
  സ്ഥിരം Parse *pParse;
  സ്ഥിരം അക്ഷരം *zText;
  സ്ഥിരം അക്ഷരം *zEnd;
  പ്രതിജ്ഞാ( z!=0 );
  എങ്കിൽ( NEVER(db==0) ) തിരികെ;
  എങ്കിൽ( db->errByteOffset!=(-2) ) തിരികെ;
  pParse = db->pParse;
  എങ്കിൽ( NEVER(pParse==0) ) തിരികെ;
  zText =pParse->zTail;
  എങ്കിൽ( NEVER(zText==0) ) തിരികെ;
  zEnd = &zText[strlen(zText)];
  എങ്കിൽ( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (പൂർണ്ണസംഖ്യ)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
ശൂന്യം sqlite3_Record_Error_Offset_Of_വ്യഞ്ജകം(sqlite3 *db, സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  ആകുമ്പോൾ( p_വ്യഞ്ജകം ){
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Reduced|EP_TokenOnly) ) തിരികെ;
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_OuterON|EP_InnerON) || p_വ്യഞ്ജകം->w.iOfst<=0 ){
      p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
    }അല്ലെങ്കിൽ{
      നിർത്തുക;
    }
  }
  എങ്കിൽ( p_വ്യഞ്ജകം==0 ) തിരികെ;
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_FromDDL) ) തിരികെ;
  db->errByteOffset = p_വ്യഞ്ജകം->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
പൂർണ്ണസംഖ്യ sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  അക്ഷരം *zNew;
  പ്രതിജ്ഞാ( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  എങ്കിൽ( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    തിരികെ 0;
  }
  എങ്കിൽ( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    തിരികെ p->nAlloc - p->nChar - 1;
  }അല്ലെങ്കിൽ{
    അക്ഷരം *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    എങ്കിൽ( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    എങ്കിൽ( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      തിരികെ 0;
    }അല്ലെങ്കിൽ{
      p->nAlloc = (പൂർണ്ണസംഖ്യ)szNew;
    }
    എങ്കിൽ( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }അല്ലെങ്കിൽ{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    എങ്കിൽ( zNew ){
      പ്രതിജ്ഞാ( p->zText!=0 || p->nChar==0 );
      എങ്കിൽ( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }അല്ലെങ്കിൽ{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      തിരികെ 0;
    }
  }
  പ്രതിജ്ഞാ( N>=0 && N<=0x7fffffff );
  തിരികെ (പൂർണ്ണസംഖ്യ)N;
}

പൂർണ്ണസംഖ്യ sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  എങ്കിൽ( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  തിരികെ p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
ശൂന്യം sqlite3_str_appendchar(sqlite3_str *p, പൂർണ്ണസംഖ്യ N, അക്ഷരം c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  എങ്കിൽ( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    തിരികെ;
  }
  ആകുമ്പോൾ( (N--)>0 ) p->zText[p->nChar++] = c;
}
സ്ഥിതം ശൂന്യം sqlite3StrAppendchar64(sqlite3_str *p, i64 N, അക്ഷരം c){
  testcase( p->nChar + N > 0x7fffffff );
  എങ്കിൽ( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    തിരികെ;
  }
  ആകുമ്പോൾ( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
സ്ഥിതം ശൂന്യം SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, സ്ഥിരം അക്ഷരം *z, പൂർണ്ണസംഖ്യ N){
  N = sqlite3StrAccumEnlarge(p, N);
  പ്രതിജ്ഞാ( z!=0 || N==0 );
  എങ്കിൽ( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
ശൂന്യം sqlite3_str_append(sqlite3_str *p, സ്ഥിരം അക്ഷരം *z, പൂർണ്ണസംഖ്യ N){
  പ്രതിജ്ഞാ( z!=0 || N==0 );
  പ്രതിജ്ഞാ( p->zText!=0 || p->nChar==0 || p->accError );
  പ്രതിജ്ഞാ( N>=0 );
  പ്രതിജ്ഞാ( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  എങ്കിൽ( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }അല്ലെങ്കിൽ എങ്കിൽ( N ){
    പ്രതിജ്ഞാ( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
സ്ഥിതം ശൂന്യം sqlite3StrAppend64(sqlite3_str *p, സ്ഥിരം അക്ഷരം *z, i64 N){
  പ്രതിജ്ഞാ( z!=0 || N==0 );
  പ്രതിജ്ഞാ( p->zText!=0 || p->nChar==0 || p->accError );
  പ്രതിജ്ഞാ( N>=0 );
  പ്രതിജ്ഞാ( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  എങ്കിൽ( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }അല്ലെങ്കിൽ എങ്കിൽ( N ){
    പ്രതിജ്ഞാ( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
ശൂന്യം sqlite3_str_appendall(sqlite3_str *p, സ്ഥിരം അക്ഷരം *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
സ്ഥിതം SQLITE_NOINLINE അക്ഷരം *strAccumFinishRealloc(StrAccum *p){
  അക്ഷരം *zText;
  പ്രതിജ്ഞാ( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  എങ്കിൽ( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }അല്ലെങ്കിൽ{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  തിരികെ zText;
}
അക്ഷരം *sqlite3StrAccumFinish(StrAccum *p){
  എങ്കിൽ( p->zText ){
    p->zText[p->nChar] = 0;
    എങ്കിൽ( p->mxAlloc>0 && !isMalloced(p) ){
      തിരികെ strAccumFinishRealloc(p);
    }
  }
  തിരികെ p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
അക്ഷരം *sqlite3_str_finish(sqlite3_str *p){
  അക്ഷരം *z;
  എങ്കിൽ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }അല്ലെങ്കിൽ{
    z = 0;
  }
  തിരികെ z;
}

/* Return any error code associated with p */
പൂർണ്ണസംഖ്യ sqlite3_str_errcode(sqlite3_str *p){
  തിരികെ p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
പൂർണ്ണസംഖ്യ sqlite3_str_length(sqlite3_str *p){
  തിരികെ p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
ശൂന്യം sqlite3_str_truncate(sqlite3_str *p, പൂർണ്ണസംഖ്യ N){
  എങ്കിൽ( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
അക്ഷരം *sqlite3_str_മൂല്യം(sqlite3_str *p){
  എങ്കിൽ( p==0 || p->nChar==0 ) തിരികെ 0;
  p->zText[p->nChar] = 0;
  തിരികെ p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
ശൂന്യം sqlite3_str_reset(StrAccum *p){
  എങ്കിൽ( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }അല്ലെങ്കിൽ എങ്കിൽ( p==(sqlite3_str*)&sqlite3OomStr ){
    തിരികെ;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
ശൂന്യം sqlite3_str_free(sqlite3_str *p){
  എങ്കിൽ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
ശൂന്യം sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, അക്ഷരം *zBase, പൂർണ്ണസംഖ്യ n, പൂർണ്ണസംഖ്യ mx){
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
  sqlite3_str *p = sqlite3_malloc64(വലുപ്പം(*p));
  എങ്കിൽ( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }അല്ലെങ്കിൽ{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  തിരികെ p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
അക്ഷരം *sqlite3VMPrintf(sqlite3 *db, സ്ഥിരം അക്ഷരം *zFormat, va_list ap){
  അക്ഷരം *z;
  അക്ഷരം zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  പ്രതിജ്ഞാ( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, വലുപ്പം(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  എങ്കിൽ( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  തിരികെ z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
അക്ഷരം *sqlite3MPrintf(sqlite3 *db, സ്ഥിരം അക്ഷരം *zFormat, ...){
  va_list ap;
  അക്ഷരം *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  തിരികെ z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
അക്ഷരം *sqlite3_vmprintf(സ്ഥിരം അക്ഷരം *zFormat, va_list ap){
  അക്ഷരം *z;
  അക്ഷരം zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  എങ്കിൽ( zFormat==0 ){
    (ശൂന്യം)SQLITE_MISUSE_BKPT;
    തിരികെ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  എങ്കിൽ( sqlite3_initialize() ) തിരികെ 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, വലുപ്പം(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  തിരികെ z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
അക്ഷരം *sqlite3_mprintf(സ്ഥിരം അക്ഷരം *zFormat, ...){
  va_list ap;
  അക്ഷരം *z;
#ifndef SQLITE_OMIT_AUTOINIT
  എങ്കിൽ( sqlite3_initialize() ) തിരികെ 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  തിരികെ z;
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
അക്ഷരം *sqlite3_vsnprintf(പൂർണ്ണസംഖ്യ n, അക്ഷരം *zBuf, സ്ഥിരം അക്ഷരം *zFormat, va_list ap){
  StrAccum acc;
  എങ്കിൽ( n<=0 ) തിരികെ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( zBuf==0 || zFormat==0 ) {
    (ശൂന്യം)SQLITE_MISUSE_BKPT;
    എങ്കിൽ( zBuf ) zBuf[0] = 0;
    തിരികെ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  തിരികെ zBuf;
}
അക്ഷരം *sqlite3_snprintf(പൂർണ്ണസംഖ്യ n, അക്ഷരം *zBuf, സ്ഥിരം അക്ഷരം *zFormat, ...){
  StrAccum acc;
  va_list ap;
  എങ്കിൽ( n<=0 ) തിരികെ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( zBuf==0 || zFormat==0 ) {
    (ശൂന്യം)SQLITE_MISUSE_BKPT;
    എങ്കിൽ( zBuf ) zBuf[0] = 0;
    തിരികെ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  തിരികെ zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#എങ്കിൽ defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#അല്ലെങ്കിലെങ്കിൽ SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#അല്ലെങ്കിൽ
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
സ്ഥിതം ശൂന്യം render_Log_സന്ദേശം(പൂർണ്ണസംഖ്യ iErrCode, സ്ഥിരം അക്ഷരം *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  അക്ഷരം z_സന്ദേശം[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_സന്ദേശം, വലുപ്പം(z_സന്ദേശം), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
ശൂന്യം sqlite3_log(പൂർണ്ണസംഖ്യ iErrCode, സ്ഥിരം അക്ഷരം *zFormat, ...){
  va_list ap;                             /* Vararg list */
  എങ്കിൽ( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_സന്ദേശം(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
ശൂന്യം sqlite3DebugPrintf(സ്ഥിരം അക്ഷരം *zFormat, ...){
  va_list ap;
  StrAccum acc;
  അക്ഷരം zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, വലുപ്പം(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    ബാഹ്യ ശൂന്യം SQLITE_OS_TRACE_PROC(സ്ഥിരം അക്ഷരം *zBuf, പൂർണ്ണസംഖ്യ nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, വലുപ്പം(zBuf));
  }
#അല്ലെങ്കിൽ
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
ശൂന്യം sqlite3_str_appendf(StrAccum *p, സ്ഥിരം അക്ഷരം *zFormat, ...){
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
അക്ഷരം *sqlite3RCStrRef(അക്ഷരം *z){
  RCStr *p = (RCStr*)z;
  പ്രതിജ്ഞാ( p!=0 );
  p--;
  p->nRCRef++;
  തിരികെ z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
ശൂന്യം sqlite3RCStrUnref(ശൂന്യം *z){
  RCStr *p = (RCStr*)z;
  പ്രതിജ്ഞാ( p!=0 );
  p--;
  പ്രതിജ്ഞാ( p->nRCRef>0 );
  എങ്കിൽ( p->nRCRef>=2 ){
    p->nRCRef--;
  }അല്ലെങ്കിൽ{
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
അക്ഷരം *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + വലുപ്പം(*p) + 1 );
  എങ്കിൽ( p==0 ) തിരികെ 0;
  p->nRCRef = 1;
  തിരികെ (അക്ഷരം*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
അക്ഷരം *sqlite3RCStrResize(അക്ഷരം *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  പ്രതിജ്ഞാ( p!=0 );
  p--;
  പ്രതിജ്ഞാ( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+വലുപ്പം(RCStr)+1);
  എങ്കിൽ( pNew==0 ){
    sqlite3_free(p);
    തിരികെ 0;
  }അല്ലെങ്കിൽ{
    തിരികെ (അക്ഷരം*)&pNew[1];
  }
}
