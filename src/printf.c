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
#अन्तर्भाव "sqliteInt.h"

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
प्रकारनिर्देश unsigned वर्ण etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
प्रकारनिर्देश संरचना et_info {   /* Information about each format field */
  वर्ण fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  वर्ण iNxt;               /* Next with same hash, or 0 for end of chain */
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
#सचे 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
अत्थाय {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    सचे {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

स्थिर ध्रुव वर्ण aDigits[] = "0123456789ABCDEF0123456789abcdef";
स्थिर ध्रुव वर्ण aHex[]    = "0123456789abcdef";
स्थिर ध्रुव वर्ण aPrefix[] = "-x0\000X0";
स्थिर ध्रुव et_info fmtinfo[25] = {
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
शून्य sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  प्रतिज्ञा( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  सचे( p->mxAlloc ) sqlite3_str_reset(p);
  सचे( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
स्थिर sqlite3_int64 getIntArg(PrintfArguments *p){
  सचे( p->nArg<=p->nUsed ) पटिनिवत्त 0;
  पटिनिवत्त sqlite3_मानम्_int64(p->apArg[p->nUsed++]);
}
स्थिर द्विगुण getDoubleArg(PrintfArguments *p){
  सचे( p->nArg<=p->nUsed ) पटिनिवत्त 0.0;
  पटिनिवत्त sqlite3_मानम्_double(p->apArg[p->nUsed++]);
}
स्थिर वर्ण *getTextArg(PrintfArguments *p){
  सचे( p->nArg<=p->nUsed ) पटिनिवत्त 0;
  पटिनिवत्त (वर्ण*)sqlite3_मानम्_text(p->apArg[p->nUsed++]);
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
स्थिर वर्ण *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  वर्ण *z;
  सचे( pAccum->accError ) पटिनिवत्त 0;
  सचे( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    पटिनिवत्त 0;
  }
  z = sqlite3_malloc(n);
  सचे( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  पटिनिवत्त z;
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
# सचे SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# अञ्ञथा
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#अञ्ञथा
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
स्थिर शून्य sqlite3StrAppend64(sqlite3_str *p, ध्रुव वर्ण *z, i64 N);
स्थिर शून्य sqlite3StrAppendchar64(sqlite3_str *p, i64 N, वर्ण c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
शून्य sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ध्रुव वर्ण *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  पूर्णाङ्क c;                     /* Next character in the format string */
  वर्ण *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  पूर्णाङ्क idx;                   /* A general purpose loop counter */
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
  वर्ण prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  द्विगुण realvalue;          /* Value for real types */
  ध्रुव et_info *infop;      /* Pointer to the appropriate info structure */
  वर्ण *zOut;                /* Rendering buffer */
  पूर्णाङ्क nOut;                  /* Size of the rendering buffer */
  वर्ण *zExtra = 0;          /* Malloced memory used by some conversion */
  पूर्णाङ्क exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  वर्ण buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  प्रतिज्ञा( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  सचे( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }अञ्ञथा{
    bArgList = 0;
  }
  अत्थाय(; (c=(*fmt))!=0; ++fmt){
    सचे( c!='%' ){
      bufpt = (वर्ण *)fmt;
#सचे HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#अञ्ञथा
      fmt = strchr(fmt, '%');
      सचे( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      सचे( *fmt==0 ) भङ्ग;
    }
    सचे( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      भङ्ग;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    करोहि{
      विकल्प( c ){
        पक्ष '-':   flag_leftjustify = 1;     भङ्ग;
        पक्ष '+':   flag_prefix = '+';        भङ्ग;
        पक्ष ' ':   flag_prefix = ' ';        भङ्ग;
        पक्ष '#':   flag_alternateform = 1;   भङ्ग;
        पक्ष '!':   flag_altform2 = 1;        भङ्ग;
        पक्ष '0':   flag_zeropad = 1;         भङ्ग;
        पक्ष ',':   cThousand = ',';          भङ्ग;
        स्वभाव:    done = 1;                 भङ्ग;
        पक्ष 'l': {
          flag_long = 1;
          c = *++fmt;
          सचे( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          भङ्ग;
        }
        पक्ष '1': पक्ष '2': पक्ष '3': पक्ष '4': पक्ष '5':
        पक्ष '6': पक्ष '7': पक्ष '8': पक्ष '9': {
          unsigned wx = c - '0';
          याव( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          सचे( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          सचे( c!='.' && c!='l' ){
            done = 1;
          }अञ्ञथा{
            fmt--;
          }
          भङ्ग;
        }
        पक्ष '*': {
          सचे( bArgList ){
            width = (पूर्णाङ्क)getIntArg(pArgList);
          }अञ्ञथा{
            width = va_arg(ap,पूर्णाङ्क);
          }
          सचे( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          सचे( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          सचे( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          भङ्ग;
        }
        पक्ष '.': {
          c = *++fmt;
          सचे( c=='*' ){
            सचे( bArgList ){
              precision = (पूर्णाङ्क)getIntArg(pArgList);
            }अञ्ञथा{
              precision = va_arg(ap,पूर्णाङ्क);
            }
            सचे( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }अञ्ञथा{
            unsigned px = 0;
            याव( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          सचे( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          सचे( c=='l' ){
            --fmt;
          }अञ्ञथा{
            done = 1;
          }
          भङ्ग;
        }
      }
    }याव( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    अत्थाय(idx=0; idx<ArraySize(fmtinfo); idx++){
      सचे( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        भङ्ग;
      }
    }
#अञ्ञथा
    /* Fast hash-table lookup */
    प्रतिज्ञा( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    सचे( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }अञ्ञथा{
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
    प्रतिज्ञा( width>=0 );
    प्रतिज्ञा( precision>=(-1) );
    विकल्प( xtype ){
      पक्ष etPOINTER:
        flag_long = परिमाण(वर्ण*)==परिमाण(i64) ? 2 :
                     परिमाण(वर्ण*)==परिमाण(long पूर्णाङ्क) ? 1 : 0;
        /* no break */ deliberate_fall_through
      पक्ष etORDINAL:
      पक्ष etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      पक्ष etDECIMAL:
        सचे( infop->flags & FLAG_SIGNED ){
          i64 v;
          सचे( bArgList ){
            v = getIntArg(pArgList);
          }अञ्ञथा सचे( flag_long ){
            सचे( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }अञ्ञथा{
              v = va_arg(ap,long पूर्णाङ्क);
            }
          }अञ्ञथा{
            v = va_arg(ap,पूर्णाङ्क);
          }
          सचे( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }अञ्ञथा{
            longvalue = v;
            prefix = flag_prefix;
          }
        }अञ्ञथा{
          सचे( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }अञ्ञथा सचे( flag_long ){
            सचे( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }अञ्ञथा{
              longvalue = va_arg(ap,unsigned long पूर्णाङ्क);
            }
          }अञ्ञथा{
            longvalue = va_arg(ap,unsigned पूर्णाङ्क);
          }
          prefix = 0;
        }

#सचे WHERETRACE_ENABLED
        सचे( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#सचे TREETRACE_ENABLED
        सचे( xtype==etPOINTER && sqlite3_वृक्षः_Trace & 0x100000 ) longvalue = 0;
#endif

        सचे( longvalue==0 ) flag_alternateform = 0;
        सचे( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        सचे( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }अञ्ञथा{
          u64 n;
          n = (u64)precision + 10;
          सचे( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          सचे( zOut==0 ) पटिनिवत्त;
          nOut = (पूर्णाङ्क)n;
        }
        bufpt = &zOut[nOut-1];
        सचे( xtype==etORDINAL ){
          स्थिर ध्रुव वर्ण zOrd[] = "thstndrd";
          पूर्णाङ्क x = (पूर्णाङ्क)(longvalue % 10);
          सचे( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ध्रुव वर्ण *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          करोहि{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }याव( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        सचे( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        सचे( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          पूर्णाङ्क ii;
          bufpt -= nn;
          अत्थाय(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            सचे( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        सचे( prefix ) *(--bufpt) = prefix;               /* Add sign */
        सचे( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          ध्रुव वर्ण *pre;
          वर्ण x;
          pre = &aPrefix[infop->prefix];
          अत्थाय(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        भङ्ग;
      पक्ष etFLOAT:
      पक्ष etEXP:
      पक्ष etGENERIC: {
        FpDecode s;
        पूर्णाङ्क iRound;
        पूर्णाङ्क j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        सचे( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }अञ्ञथा{
          realvalue = va_arg(ap,द्विगुण);
        }
        सचे( precision<0 ) precision = 6;         /* Set default precision */
        सचे( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        सचे( xtype==etFLOAT ){
          iRound = -precision;
        }अञ्ञथा सचे( xtype==etGENERIC ){
          सचे( precision==0 ) precision = 1;
          iRound = precision;
        }अञ्ञथा{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        सचे( s.isSpecial ){
          सचे( s.isSpecial==2 ){
            सचे( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }अञ्ञथा{
              bufpt = "NaN";
              length = 3;
            }
            भङ्ग;
          }अञ्ञथा सचे( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }अञ्ञथा{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            सचे( s.sign=='-' ){
              /* no-op */
            }अञ्ञथा सचे( flag_prefix ){
              buf[0] = flag_prefix;
            }अञ्ञथा{
              bufpt++;
            }
            length = strlen(bufpt);
            भङ्ग;
          }
        }
        सचे( s.sign=='-' ){
          सचे( flag_alternateform
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
          }अञ्ञथा{
            prefix = '-';
          }
        }अञ्ञथा{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        सचे( xtype==etGENERIC ){
          प्रतिज्ञा( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          सचे( exp<-4 || exp>precision ){
            xtype = etEXP;
          }अञ्ञथा{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }अञ्ञथा{
          flag_rtz = flag_altform2;
        }
        सचे( xtype==etEXP ){
          e2 = 0;
        }अञ्ञथा{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        सचे( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        सचे( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          सचे( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            सचे( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              पटिनिवत्त;
            }
            zExtra = bufpt;
          }अञ्ञथा सचे( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            भङ्ग;
          }अञ्ञथा{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }अञ्ञथा{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        सचे( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        प्रतिज्ञा( s.n>0 );
        सचे( e2<0 ){
          *(bufpt++) = '0';
        }अञ्ञथा सचे( cThousand ){
          अत्थाय(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            सचे( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }अञ्ञथा{
          j = e2+1;
          सचे( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          सचे( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        सचे( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        सचे( e2<(-1) && precision>0 ){
          पूर्णाङ्क nn = -1-e2;
          सचे( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        सचे( precision>0 ){
          पूर्णाङ्क nn = s.n - j;
          सचे( NEVER(nn>precision) ) nn = precision;
          सचे( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          सचे( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        सचे( flag_rtz && flag_dp ){
          याव( bufpt[-1]=='0' ) *(--bufpt) = 0;
          प्रतिज्ञा( bufpt>zOut );
          सचे( bufpt[-1]=='.' ){
            सचे( flag_altform2 ){
              *(bufpt++) = '0';
            }अञ्ञथा{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        सचे( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          सचे( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }अञ्ञथा{
            *(bufpt++) = '+';
          }
          सचे( exp>=100 ){
            *(bufpt++) = (वर्ण)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (वर्ण)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (वर्ण)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        प्रतिज्ञा( length <= szBufNeeded );
        सचे( length<width ){
          i64 nPad = width - length;
          सचे( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }अञ्ञथा सचे( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }अञ्ञथा{
            पूर्णाङ्क adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        सचे( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          प्रतिज्ञा( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          अनुवर्त;
        }अञ्ञथा{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          भङ्ग;
        }
      }
      पक्ष etSIZE:
        सचे( !bArgList ){
          *(va_arg(ap,पूर्णाङ्क*)) = pAccum->nChar;
        }
        length = width = 0;
        भङ्ग;
      पक्ष etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        भङ्ग;
      पक्ष etCHARX:
        सचे( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          सचे( bufpt ){
            buf[0] = c = *(bufpt++);
            सचे( (c&0xc0)==0xc0 ){
              याव( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }अञ्ञथा{
            buf[0] = 0;
          }
        }अञ्ञथा{
          unsigned पूर्णाङ्क ch = va_arg(ap,unsigned पूर्णाङ्क);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        सचे( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          सचे( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          याव( precision > 1 ){
            i64 nCopyBytes;
            सचे( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            सचे( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              भङ्ग;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        गच्छ adjust_width_for_utf8;
      पक्ष etSTRING:
      पक्ष etDYNSTRING:
        सचे( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }अञ्ञथा{
          bufpt = va_arg(ap,वर्ण*);
        }
        सचे( bufpt==0 ){
          bufpt = "";
        }अञ्ञथा सचे( xtype==etDYNSTRING ){
          सचे( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            प्रतिज्ञा( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (पूर्णाङ्क)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            भङ्ग;
          }
          zExtra = bufpt;
        }
        सचे( precision>=0 ){
          सचे( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned वर्ण *z = (unsigned वर्ण*)bufpt;
            याव( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned वर्ण*)bufpt;
          }अञ्ञथा{
            अत्थाय(length=0; length<precision && bufpt[length]; length++){}
          }
        }अञ्ञथा{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        सचे( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          याव( ii>=0 ) सचे( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        भङ्ग;
      पक्ष etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      पक्ष etESCAPE_J: {         /* %J: Generate a JSON string literal */
        वर्ण *escarg;
        i64 i, j, px, iStart;
        unsigned वर्ण ch;

        सचे( bArgList ){
          escarg = getTextArg(pArgList);
        }अञ्ञथा{
          escarg = va_arg(ap,वर्ण*);
        }
        iStart = sqlite3_str_length(pAccum);
        सचे( escarg==0 ){
          सचे( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }अञ्ञथा{
          सचे( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          सचे( px<0 ){
            px = 0x7fffffff;
          }अञ्ञथा सचे( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            अत्थाय(i=0; i<px && escarg[i]; i++){
              सचे( (escarg[i]&0xc0)==0x80 ) px++;
            }
            सचे( i==px ){
              याव( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          अत्थाय(i=j=0; i<px; i++){
            सचे( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              सचे( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              सचे( ch==0 ) भङ्ग;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              सचे( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }अञ्ञथा सचे( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }अञ्ञथा{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          सचे( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          सचे( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        सचे( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          वर्ण *zz;
          सचे( flag_altform2 && n>0 ){
            zz = sqlite3_str_मानम्(pAccum);
            अत्थाय(i=iStart; zz[i]; i++){
              सचे( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          सचे( width>len ){
            sqlite3_int64 sp = width-len;
            प्रतिज्ञा( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (पूर्णाङ्क)sp, ' ');
            सचे( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_मानम्(pAccum);
              zz += iStart;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        अनुवर्त;
      }
      पक्ष etESCAPE_q:          /* %q: Escape ' characters */
      पक्ष etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      पक्ष etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        पूर्णाङ्क needQuote = 0;
        वर्ण ch;
        वर्ण *escarg;
        वर्ण q;

        सचे( bArgList ){
          escarg = getTextArg(pArgList);
        }अञ्ञथा{
          escarg = va_arg(ap,वर्ण*);
        }
        सचे( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }अञ्ञथा सचे( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        सचे( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }अञ्ञथा{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        अत्थाय(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          सचे( ch==q )  n++;
          सचे( flag_altform2 && (ch&0xc0)==0xc0 ){
            याव( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        सचे( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          अत्थाय(k=0; k<i; k++){
            सचे( escarg[k]=='\\' ){
              nBack++;
            }अञ्ञथा सचे( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          सचे( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            सचे( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }अञ्ञथा{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        सचे( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          सचे( bufpt==0 ) पटिनिवत्त;
        }अञ्ञथा{
          bufpt = buf;
        }
        j = 0;
        सचे( needQuote ){
          सचे( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }अञ्ञथा{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        सचे( flag_alternateform ){
          अत्थाय(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            सचे( ch==q ){
              bufpt[j++] = ch;
            }अञ्ञथा सचे( ch=='\\' ){
              bufpt[j++] = '\\';
            }अञ्ञथा सचे( ((unsigned वर्ण)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }अञ्ञथा{
          अत्थाय(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            सचे( ch==q ) bufpt[j++] = ch;
          }
        }
        सचे( needQuote ){
          bufpt[j++] = '\'';
          सचे( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        गच्छ adjust_width_for_utf8;
      }
      पक्ष etTOKEN: {
        सचे( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) पटिनिवत्त;
        सचे( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          व्यञ्जकम् *p_व्यञ्जकम् = va_arg(ap,व्यञ्जकम्*);
          सचे( ALWAYS(p_व्यञ्जकम्) && ALWAYS(!व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_Int_मानम्)) ){
            sqlite3_str_appendall(pAccum, (ध्रुव वर्ण*)p_व्यञ्जकम्->u.zToken);
            sqlite3_Record_Error_Offset_Of_व्यञ्जकम्(pAccum->db, p_व्यञ्जकम्);
          }
        }अञ्ञथा{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          प्रतिज्ञा( bArgList==0 );
          सचे( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ध्रुव वर्ण*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        भङ्ग;
      }
      पक्ष etSRCITEM: {
        SrcItem *pItem;
        सचे( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) पटिनिवत्त;
        pItem = va_arg(ap, SrcItem*);
        प्रतिज्ञा( bArgList==0 );
        सचे( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }अञ्ञथा सचे( pItem->zName ){
          सचे( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }अञ्ञथा सचे( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }अञ्ञथा सचे( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          प्रतिज्ञा( pSel!=0 ); 
          सचे( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }अञ्ञथा सचे( pSel->selFlags & SF_Multi_मानम् ){
            प्रतिज्ञा( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }अञ्ञथा{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        भङ्ग;
      }
      स्वभाव: {
        प्रतिज्ञा( xtype==etINVALID );
        पटिनिवत्त;
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
    सचे( width>0 ){
      सचे( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      सचे( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }अञ्ञथा{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    सचे( zExtra ){
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
शून्य sqlite3RecordErrorByteOffset(sqlite3 *db, ध्रुव वर्ण *z){
  ध्रुव Parse *pParse;
  ध्रुव वर्ण *zText;
  ध्रुव वर्ण *zEnd;
  प्रतिज्ञा( z!=0 );
  सचे( NEVER(db==0) ) पटिनिवत्त;
  सचे( db->errByteOffset!=(-2) ) पटिनिवत्त;
  pParse = db->pParse;
  सचे( NEVER(pParse==0) ) पटिनिवत्त;
  zText =pParse->zTail;
  सचे( NEVER(zText==0) ) पटिनिवत्त;
  zEnd = &zText[strlen(zText)];
  सचे( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (पूर्णाङ्क)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
शून्य sqlite3_Record_Error_Offset_Of_व्यञ्जकम्(sqlite3 *db, ध्रुव व्यञ्जकम् *p_व्यञ्जकम्){
  याव( p_व्यञ्जकम् ){
    सचे( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Reduced|EP_TokenOnly) ) पटिनिवत्त;
    सचे( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_OuterON|EP_InnerON) || p_व्यञ्जकम्->w.iOfst<=0 ){
      p_व्यञ्जकम् = p_व्यञ्जकम्->p_वामः;
    }अञ्ञथा{
      भङ्ग;
    }
  }
  सचे( p_व्यञ्जकम्==0 ) पटिनिवत्त;
  सचे( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FromDDL) ) पटिनिवत्त;
  db->errByteOffset = p_व्यञ्जकम्->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
पूर्णाङ्क sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  वर्ण *zNew;
  प्रतिज्ञा( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  सचे( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    पटिनिवत्त 0;
  }
  सचे( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    पटिनिवत्त p->nAlloc - p->nChar - 1;
  }अञ्ञथा{
    वर्ण *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    सचे( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    सचे( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      पटिनिवत्त 0;
    }अञ्ञथा{
      p->nAlloc = (पूर्णाङ्क)szNew;
    }
    सचे( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }अञ्ञथा{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    सचे( zNew ){
      प्रतिज्ञा( p->zText!=0 || p->nChar==0 );
      सचे( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }अञ्ञथा{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      पटिनिवत्त 0;
    }
  }
  प्रतिज्ञा( N>=0 && N<=0x7fffffff );
  पटिनिवत्त (पूर्णाङ्क)N;
}

पूर्णाङ्क sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  सचे( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  पटिनिवत्त p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
शून्य sqlite3_str_appendchar(sqlite3_str *p, पूर्णाङ्क N, वर्ण c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  सचे( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    पटिनिवत्त;
  }
  याव( (N--)>0 ) p->zText[p->nChar++] = c;
}
स्थिर शून्य sqlite3StrAppendchar64(sqlite3_str *p, i64 N, वर्ण c){
  testcase( p->nChar + N > 0x7fffffff );
  सचे( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    पटिनिवत्त;
  }
  याव( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
स्थिर शून्य SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ध्रुव वर्ण *z, पूर्णाङ्क N){
  N = sqlite3StrAccumEnlarge(p, N);
  प्रतिज्ञा( z!=0 || N==0 );
  सचे( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
शून्य sqlite3_str_append(sqlite3_str *p, ध्रुव वर्ण *z, पूर्णाङ्क N){
  प्रतिज्ञा( z!=0 || N==0 );
  प्रतिज्ञा( p->zText!=0 || p->nChar==0 || p->accError );
  प्रतिज्ञा( N>=0 );
  प्रतिज्ञा( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  सचे( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }अञ्ञथा सचे( N ){
    प्रतिज्ञा( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
स्थिर शून्य sqlite3StrAppend64(sqlite3_str *p, ध्रुव वर्ण *z, i64 N){
  प्रतिज्ञा( z!=0 || N==0 );
  प्रतिज्ञा( p->zText!=0 || p->nChar==0 || p->accError );
  प्रतिज्ञा( N>=0 );
  प्रतिज्ञा( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  सचे( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }अञ्ञथा सचे( N ){
    प्रतिज्ञा( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
शून्य sqlite3_str_appendall(sqlite3_str *p, ध्रुव वर्ण *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
स्थिर SQLITE_NOINLINE वर्ण *strAccumFinishRealloc(StrAccum *p){
  वर्ण *zText;
  प्रतिज्ञा( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  सचे( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }अञ्ञथा{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  पटिनिवत्त zText;
}
वर्ण *sqlite3StrAccumFinish(StrAccum *p){
  सचे( p->zText ){
    p->zText[p->nChar] = 0;
    सचे( p->mxAlloc>0 && !isMalloced(p) ){
      पटिनिवत्त strAccumFinishRealloc(p);
    }
  }
  पटिनिवत्त p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
वर्ण *sqlite3_str_finish(sqlite3_str *p){
  वर्ण *z;
  सचे( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }अञ्ञथा{
    z = 0;
  }
  पटिनिवत्त z;
}

/* Return any error code associated with p */
पूर्णाङ्क sqlite3_str_errcode(sqlite3_str *p){
  पटिनिवत्त p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
पूर्णाङ्क sqlite3_str_length(sqlite3_str *p){
  पटिनिवत्त p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
शून्य sqlite3_str_truncate(sqlite3_str *p, पूर्णाङ्क N){
  सचे( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
वर्ण *sqlite3_str_मानम्(sqlite3_str *p){
  सचे( p==0 || p->nChar==0 ) पटिनिवत्त 0;
  p->zText[p->nChar] = 0;
  पटिनिवत्त p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
शून्य sqlite3_str_reset(StrAccum *p){
  सचे( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }अञ्ञथा सचे( p==(sqlite3_str*)&sqlite3OomStr ){
    पटिनिवत्त;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
शून्य sqlite3_str_free(sqlite3_str *p){
  सचे( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
शून्य sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, वर्ण *zBase, पूर्णाङ्क n, पूर्णाङ्क mx){
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
  sqlite3_str *p = sqlite3_malloc64(परिमाण(*p));
  सचे( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }अञ्ञथा{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  पटिनिवत्त p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
वर्ण *sqlite3VMPrintf(sqlite3 *db, ध्रुव वर्ण *zFormat, va_list ap){
  वर्ण *z;
  वर्ण zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  प्रतिज्ञा( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, परिमाण(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  सचे( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  पटिनिवत्त z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
वर्ण *sqlite3MPrintf(sqlite3 *db, ध्रुव वर्ण *zFormat, ...){
  va_list ap;
  वर्ण *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  पटिनिवत्त z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
वर्ण *sqlite3_vmprintf(ध्रुव वर्ण *zFormat, va_list ap){
  वर्ण *z;
  वर्ण zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  सचे( zFormat==0 ){
    (शून्य)SQLITE_MISUSE_BKPT;
    पटिनिवत्त 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  सचे( sqlite3_initialize() ) पटिनिवत्त 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, परिमाण(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  पटिनिवत्त z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
वर्ण *sqlite3_mprintf(ध्रुव वर्ण *zFormat, ...){
  va_list ap;
  वर्ण *z;
#ifndef SQLITE_OMIT_AUTOINIT
  सचे( sqlite3_initialize() ) पटिनिवत्त 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  पटिनिवत्त z;
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
वर्ण *sqlite3_vsnprintf(पूर्णाङ्क n, वर्ण *zBuf, ध्रुव वर्ण *zFormat, va_list ap){
  StrAccum acc;
  सचे( n<=0 ) पटिनिवत्त zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  सचे( zBuf==0 || zFormat==0 ) {
    (शून्य)SQLITE_MISUSE_BKPT;
    सचे( zBuf ) zBuf[0] = 0;
    पटिनिवत्त zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  पटिनिवत्त zBuf;
}
वर्ण *sqlite3_snprintf(पूर्णाङ्क n, वर्ण *zBuf, ध्रुव वर्ण *zFormat, ...){
  StrAccum acc;
  va_list ap;
  सचे( n<=0 ) पटिनिवत्त zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  सचे( zBuf==0 || zFormat==0 ) {
    (शून्य)SQLITE_MISUSE_BKPT;
    सचे( zBuf ) zBuf[0] = 0;
    पटिनिवत्त zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  पटिनिवत्त zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#सचे defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#नोचेद्यदि SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#अञ्ञथा
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
स्थिर शून्य render_Log_सन्देशः(पूर्णाङ्क iErrCode, ध्रुव वर्ण *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  वर्ण z_सन्देशः[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_सन्देशः, परिमाण(z_सन्देशः), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
शून्य sqlite3_log(पूर्णाङ्क iErrCode, ध्रुव वर्ण *zFormat, ...){
  va_list ap;                             /* Vararg list */
  सचे( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_सन्देशः(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#सचे defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
शून्य sqlite3DebugPrintf(ध्रुव वर्ण *zFormat, ...){
  va_list ap;
  StrAccum acc;
  वर्ण zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, परिमाण(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    बाह्य शून्य SQLITE_OS_TRACE_PROC(ध्रुव वर्ण *zBuf, पूर्णाङ्क nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, परिमाण(zBuf));
  }
#अञ्ञथा
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
शून्य sqlite3_str_appendf(StrAccum *p, ध्रुव वर्ण *zFormat, ...){
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
वर्ण *sqlite3RCStrRef(वर्ण *z){
  RCStr *p = (RCStr*)z;
  प्रतिज्ञा( p!=0 );
  p--;
  p->nRCRef++;
  पटिनिवत्त z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
शून्य sqlite3RCStrUnref(शून्य *z){
  RCStr *p = (RCStr*)z;
  प्रतिज्ञा( p!=0 );
  p--;
  प्रतिज्ञा( p->nRCRef>0 );
  सचे( p->nRCRef>=2 ){
    p->nRCRef--;
  }अञ्ञथा{
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
वर्ण *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + परिमाण(*p) + 1 );
  सचे( p==0 ) पटिनिवत्त 0;
  p->nRCRef = 1;
  पटिनिवत्त (वर्ण*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
वर्ण *sqlite3RCStrResize(वर्ण *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  प्रतिज्ञा( p!=0 );
  p--;
  प्रतिज्ञा( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+परिमाण(RCStr)+1);
  सचे( pNew==0 ){
    sqlite3_free(p);
    पटिनिवत्त 0;
  }अञ्ञथा{
    पटिनिवत्त (वर्ण*)&pNew[1];
  }
}
