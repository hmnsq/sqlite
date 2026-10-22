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
#அந்தர்பாவ "sqliteInt.h"

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
வகைவரை unsigned எழுத்து etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
வகைவரை கட்டமைப்பு et_info {   /* Information about each format field */
  எழுத்து fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  எழுத்து iNxt;               /* Next with same hash, or 0 for end of chain */
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
#எனில் 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
க்கு {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    எனில் {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

நிலையான மாறிலி எழுத்து aDigits[] = "0123456789ABCDEF0123456789abcdef";
நிலையான மாறிலி எழுத்து aHex[]    = "0123456789abcdef";
நிலையான மாறிலி எழுத்து aPrefix[] = "-x0\000X0";
நிலையான மாறிலி et_info fmtinfo[25] = {
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
வெறுமை sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  ப்ரதிஜ்ஞா( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  எனில்( p->mxAlloc ) sqlite3_str_reset(p);
  எனில்( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
நிலையான sqlite3_int64 getIntArg(PrintfArguments *p){
  எனில்( p->nArg<=p->nUsed ) திரும்பு 0;
  திரும்பு sqlite3_மதிப்பு_int64(p->apArg[p->nUsed++]);
}
நிலையான இரட்டை getDoubleArg(PrintfArguments *p){
  எனில்( p->nArg<=p->nUsed ) திரும்பு 0.0;
  திரும்பு sqlite3_மதிப்பு_double(p->apArg[p->nUsed++]);
}
நிலையான எழுத்து *getTextArg(PrintfArguments *p){
  எனில்( p->nArg<=p->nUsed ) திரும்பு 0;
  திரும்பு (எழுத்து*)sqlite3_மதிப்பு_text(p->apArg[p->nUsed++]);
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
நிலையான எழுத்து *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  எழுத்து *z;
  எனில்( pAccum->accError ) திரும்பு 0;
  எனில்( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    திரும்பு 0;
  }
  z = sqlite3_malloc(n);
  எனில்( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  திரும்பு z;
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
# எனில் SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# இல்லையெனில்
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#இல்லையெனில்
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
நிலையான வெறுமை sqlite3StrAppend64(sqlite3_str *p, மாறிலி எழுத்து *z, i64 N);
நிலையான வெறுமை sqlite3StrAppendchar64(sqlite3_str *p, i64 N, எழுத்து c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
வெறுமை sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  மாறிலி எழுத்து *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  முழு c;                     /* Next character in the format string */
  எழுத்து *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  முழு idx;                   /* A general purpose loop counter */
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
  எழுத்து prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  இரட்டை realvalue;          /* Value for real types */
  மாறிலி et_info *infop;      /* Pointer to the appropriate info structure */
  எழுத்து *zOut;                /* Rendering buffer */
  முழு nOut;                  /* Size of the rendering buffer */
  எழுத்து *zExtra = 0;          /* Malloced memory used by some conversion */
  முழு exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  எழுத்து buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  ப்ரதிஜ்ஞா( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  எனில்( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }இல்லையெனில்{
    bArgList = 0;
  }
  க்கு(; (c=(*fmt))!=0; ++fmt){
    எனில்( c!='%' ){
      bufpt = (எழுத்து *)fmt;
#எனில் HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#இல்லையெனில்
      fmt = strchr(fmt, '%');
      எனில்( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      எனில்( *fmt==0 ) நிறுத்து;
    }
    எனில்( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      நிறுத்து;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    செய்{
      தேர்வு( c ){
        நிலை '-':   flag_leftjustify = 1;     நிறுத்து;
        நிலை '+':   flag_prefix = '+';        நிறுத்து;
        நிலை ' ':   flag_prefix = ' ';        நிறுத்து;
        நிலை '#':   flag_alternateform = 1;   நிறுத்து;
        நிலை '!':   flag_altform2 = 1;        நிறுத்து;
        நிலை '0':   flag_zeropad = 1;         நிறுத்து;
        நிலை ',':   cThousand = ',';          நிறுத்து;
        இயல்பு:    done = 1;                 நிறுத்து;
        நிலை 'l': {
          flag_long = 1;
          c = *++fmt;
          எனில்( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          நிறுத்து;
        }
        நிலை '1': நிலை '2': நிலை '3': நிலை '4': நிலை '5':
        நிலை '6': நிலை '7': நிலை '8': நிலை '9': {
          unsigned wx = c - '0';
          போது( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          எனில்( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          எனில்( c!='.' && c!='l' ){
            done = 1;
          }இல்லையெனில்{
            fmt--;
          }
          நிறுத்து;
        }
        நிலை '*': {
          எனில்( bArgList ){
            width = (முழு)getIntArg(pArgList);
          }இல்லையெனில்{
            width = va_arg(ap,முழு);
          }
          எனில்( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          எனில்( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          எனில்( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          நிறுத்து;
        }
        நிலை '.': {
          c = *++fmt;
          எனில்( c=='*' ){
            எனில்( bArgList ){
              precision = (முழு)getIntArg(pArgList);
            }இல்லையெனில்{
              precision = va_arg(ap,முழு);
            }
            எனில்( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }இல்லையெனில்{
            unsigned px = 0;
            போது( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          எனில்( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          எனில்( c=='l' ){
            --fmt;
          }இல்லையெனில்{
            done = 1;
          }
          நிறுத்து;
        }
      }
    }போது( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    க்கு(idx=0; idx<ArraySize(fmtinfo); idx++){
      எனில்( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        நிறுத்து;
      }
    }
#இல்லையெனில்
    /* Fast hash-table lookup */
    ப்ரதிஜ்ஞா( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    எனில்( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }இல்லையெனில்{
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
    ப்ரதிஜ்ஞா( width>=0 );
    ப்ரதிஜ்ஞா( precision>=(-1) );
    தேர்வு( xtype ){
      நிலை etPOINTER:
        flag_long = அளவு(எழுத்து*)==அளவு(i64) ? 2 :
                     அளவு(எழுத்து*)==அளவு(long முழு) ? 1 : 0;
        /* no break */ deliberate_fall_through
      நிலை etORDINAL:
      நிலை etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      நிலை etDECIMAL:
        எனில்( infop->flags & FLAG_SIGNED ){
          i64 v;
          எனில்( bArgList ){
            v = getIntArg(pArgList);
          }இல்லையெனில் எனில்( flag_long ){
            எனில்( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }இல்லையெனில்{
              v = va_arg(ap,long முழு);
            }
          }இல்லையெனில்{
            v = va_arg(ap,முழு);
          }
          எனில்( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }இல்லையெனில்{
            longvalue = v;
            prefix = flag_prefix;
          }
        }இல்லையெனில்{
          எனில்( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }இல்லையெனில் எனில்( flag_long ){
            எனில்( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }இல்லையெனில்{
              longvalue = va_arg(ap,unsigned long முழு);
            }
          }இல்லையெனில்{
            longvalue = va_arg(ap,unsigned முழு);
          }
          prefix = 0;
        }

#எனில் WHERETRACE_ENABLED
        எனில்( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#எனில் TREETRACE_ENABLED
        எனில்( xtype==etPOINTER && sqlite3_மரம்_Trace & 0x100000 ) longvalue = 0;
#endif

        எனில்( longvalue==0 ) flag_alternateform = 0;
        எனில்( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        எனில்( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }இல்லையெனில்{
          u64 n;
          n = (u64)precision + 10;
          எனில்( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          எனில்( zOut==0 ) திரும்பு;
          nOut = (முழு)n;
        }
        bufpt = &zOut[nOut-1];
        எனில்( xtype==etORDINAL ){
          நிலையான மாறிலி எழுத்து zOrd[] = "thstndrd";
          முழு x = (முழு)(longvalue % 10);
          எனில்( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          மாறிலி எழுத்து *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          செய்{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }போது( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        எனில்( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        எனில்( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          முழு ii;
          bufpt -= nn;
          க்கு(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            எனில்( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        எனில்( prefix ) *(--bufpt) = prefix;               /* Add sign */
        எனில்( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          மாறிலி எழுத்து *pre;
          எழுத்து x;
          pre = &aPrefix[infop->prefix];
          க்கு(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        நிறுத்து;
      நிலை etFLOAT:
      நிலை etEXP:
      நிலை etGENERIC: {
        FpDecode s;
        முழு iRound;
        முழு j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        எனில்( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }இல்லையெனில்{
          realvalue = va_arg(ap,இரட்டை);
        }
        எனில்( precision<0 ) precision = 6;         /* Set default precision */
        எனில்( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        எனில்( xtype==etFLOAT ){
          iRound = -precision;
        }இல்லையெனில் எனில்( xtype==etGENERIC ){
          எனில்( precision==0 ) precision = 1;
          iRound = precision;
        }இல்லையெனில்{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        எனில்( s.isSpecial ){
          எனில்( s.isSpecial==2 ){
            எனில்( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }இல்லையெனில்{
              bufpt = "NaN";
              length = 3;
            }
            நிறுத்து;
          }இல்லையெனில் எனில்( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }இல்லையெனில்{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            எனில்( s.sign=='-' ){
              /* no-op */
            }இல்லையெனில் எனில்( flag_prefix ){
              buf[0] = flag_prefix;
            }இல்லையெனில்{
              bufpt++;
            }
            length = strlen(bufpt);
            நிறுத்து;
          }
        }
        எனில்( s.sign=='-' ){
          எனில்( flag_alternateform
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
          }இல்லையெனில்{
            prefix = '-';
          }
        }இல்லையெனில்{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        எனில்( xtype==etGENERIC ){
          ப்ரதிஜ்ஞா( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          எனில்( exp<-4 || exp>precision ){
            xtype = etEXP;
          }இல்லையெனில்{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }இல்லையெனில்{
          flag_rtz = flag_altform2;
        }
        எனில்( xtype==etEXP ){
          e2 = 0;
        }இல்லையெனில்{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        எனில்( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        எனில்( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          எனில்( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            எனில்( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              திரும்பு;
            }
            zExtra = bufpt;
          }இல்லையெனில் எனில்( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            நிறுத்து;
          }இல்லையெனில்{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }இல்லையெனில்{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        எனில்( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        ப்ரதிஜ்ஞா( s.n>0 );
        எனில்( e2<0 ){
          *(bufpt++) = '0';
        }இல்லையெனில் எனில்( cThousand ){
          க்கு(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            எனில்( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }இல்லையெனில்{
          j = e2+1;
          எனில்( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          எனில்( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        எனில்( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        எனில்( e2<(-1) && precision>0 ){
          முழு nn = -1-e2;
          எனில்( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        எனில்( precision>0 ){
          முழு nn = s.n - j;
          எனில்( NEVER(nn>precision) ) nn = precision;
          எனில்( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          எனில்( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        எனில்( flag_rtz && flag_dp ){
          போது( bufpt[-1]=='0' ) *(--bufpt) = 0;
          ப்ரதிஜ்ஞா( bufpt>zOut );
          எனில்( bufpt[-1]=='.' ){
            எனில்( flag_altform2 ){
              *(bufpt++) = '0';
            }இல்லையெனில்{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        எனில்( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          எனில்( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }இல்லையெனில்{
            *(bufpt++) = '+';
          }
          எனில்( exp>=100 ){
            *(bufpt++) = (எழுத்து)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (எழுத்து)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (எழுத்து)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        ப்ரதிஜ்ஞா( length <= szBufNeeded );
        எனில்( length<width ){
          i64 nPad = width - length;
          எனில்( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }இல்லையெனில் எனில்( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }இல்லையெனில்{
            முழு adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        எனில்( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          ப்ரதிஜ்ஞா( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          தொடர்;
        }இல்லையெனில்{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          நிறுத்து;
        }
      }
      நிலை etSIZE:
        எனில்( !bArgList ){
          *(va_arg(ap,முழு*)) = pAccum->nChar;
        }
        length = width = 0;
        நிறுத்து;
      நிலை etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        நிறுத்து;
      நிலை etCHARX:
        எனில்( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          எனில்( bufpt ){
            buf[0] = c = *(bufpt++);
            எனில்( (c&0xc0)==0xc0 ){
              போது( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }இல்லையெனில்{
            buf[0] = 0;
          }
        }இல்லையெனில்{
          unsigned முழு ch = va_arg(ap,unsigned முழு);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        எனில்( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          எனில்( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          போது( precision > 1 ){
            i64 nCopyBytes;
            எனில்( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            எனில்( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              நிறுத்து;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        செல் adjust_width_for_utf8;
      நிலை etSTRING:
      நிலை etDYNSTRING:
        எனில்( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }இல்லையெனில்{
          bufpt = va_arg(ap,எழுத்து*);
        }
        எனில்( bufpt==0 ){
          bufpt = "";
        }இல்லையெனில் எனில்( xtype==etDYNSTRING ){
          எனில்( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            ப்ரதிஜ்ஞா( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (முழு)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            நிறுத்து;
          }
          zExtra = bufpt;
        }
        எனில்( precision>=0 ){
          எனில்( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned எழுத்து *z = (unsigned எழுத்து*)bufpt;
            போது( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned எழுத்து*)bufpt;
          }இல்லையெனில்{
            க்கு(length=0; length<precision && bufpt[length]; length++){}
          }
        }இல்லையெனில்{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        எனில்( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          போது( ii>=0 ) எனில்( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        நிறுத்து;
      நிலை etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      நிலை etESCAPE_J: {         /* %J: Generate a JSON string literal */
        எழுத்து *escarg;
        i64 i, j, px, iStart;
        unsigned எழுத்து ch;

        எனில்( bArgList ){
          escarg = getTextArg(pArgList);
        }இல்லையெனில்{
          escarg = va_arg(ap,எழுத்து*);
        }
        iStart = sqlite3_str_length(pAccum);
        எனில்( escarg==0 ){
          எனில்( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }இல்லையெனில்{
          எனில்( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          எனில்( px<0 ){
            px = 0x7fffffff;
          }இல்லையெனில் எனில்( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            க்கு(i=0; i<px && escarg[i]; i++){
              எனில்( (escarg[i]&0xc0)==0x80 ) px++;
            }
            எனில்( i==px ){
              போது( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          க்கு(i=j=0; i<px; i++){
            எனில்( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              எனில்( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              எனில்( ch==0 ) நிறுத்து;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              எனில்( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }இல்லையெனில் எனில்( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }இல்லையெனில்{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          எனில்( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          எனில்( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        எனில்( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          எழுத்து *zz;
          எனில்( flag_altform2 && n>0 ){
            zz = sqlite3_str_மதிப்பு(pAccum);
            க்கு(i=iStart; zz[i]; i++){
              எனில்( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          எனில்( width>len ){
            sqlite3_int64 sp = width-len;
            ப்ரதிஜ்ஞா( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (முழு)sp, ' ');
            எனில்( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_மதிப்பு(pAccum);
              zz += iStart;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        தொடர்;
      }
      நிலை etESCAPE_q:          /* %q: Escape ' characters */
      நிலை etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      நிலை etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        முழு needQuote = 0;
        எழுத்து ch;
        எழுத்து *escarg;
        எழுத்து q;

        எனில்( bArgList ){
          escarg = getTextArg(pArgList);
        }இல்லையெனில்{
          escarg = va_arg(ap,எழுத்து*);
        }
        எனில்( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }இல்லையெனில் எனில்( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        எனில்( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }இல்லையெனில்{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        க்கு(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          எனில்( ch==q )  n++;
          எனில்( flag_altform2 && (ch&0xc0)==0xc0 ){
            போது( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        எனில்( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          க்கு(k=0; k<i; k++){
            எனில்( escarg[k]=='\\' ){
              nBack++;
            }இல்லையெனில் எனில்( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          எனில்( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            எனில்( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }இல்லையெனில்{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        எனில்( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          எனில்( bufpt==0 ) திரும்பு;
        }இல்லையெனில்{
          bufpt = buf;
        }
        j = 0;
        எனில்( needQuote ){
          எனில்( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }இல்லையெனில்{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        எனில்( flag_alternateform ){
          க்கு(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            எனில்( ch==q ){
              bufpt[j++] = ch;
            }இல்லையெனில் எனில்( ch=='\\' ){
              bufpt[j++] = '\\';
            }இல்லையெனில் எனில்( ((unsigned எழுத்து)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }இல்லையெனில்{
          க்கு(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            எனில்( ch==q ) bufpt[j++] = ch;
          }
        }
        எனில்( needQuote ){
          bufpt[j++] = '\'';
          எனில்( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        செல் adjust_width_for_utf8;
      }
      நிலை etTOKEN: {
        எனில்( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) திரும்பு;
        எனில்( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          கோவை *p_கோவை = va_arg(ap,கோவை*);
          எனில்( ALWAYS(p_கோவை) && ALWAYS(!கோவை_Has_Property(p_கோவை,EP_Int_மதிப்பு)) ){
            sqlite3_str_appendall(pAccum, (மாறிலி எழுத்து*)p_கோவை->u.zToken);
            sqlite3_Record_Error_Offset_Of_கோவை(pAccum->db, p_கோவை);
          }
        }இல்லையெனில்{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          ப்ரதிஜ்ஞா( bArgList==0 );
          எனில்( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (மாறிலி எழுத்து*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        நிறுத்து;
      }
      நிலை etSRCITEM: {
        SrcItem *pItem;
        எனில்( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) திரும்பு;
        pItem = va_arg(ap, SrcItem*);
        ப்ரதிஜ்ஞா( bArgList==0 );
        எனில்( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }இல்லையெனில் எனில்( pItem->zName ){
          எனில்( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }இல்லையெனில் எனில்( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }இல்லையெனில் எனில்( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          ப்ரதிஜ்ஞா( pSel!=0 ); 
          எனில்( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }இல்லையெனில் எனில்( pSel->selFlags & SF_Multi_மதிப்பு ){
            ப்ரதிஜ்ஞா( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }இல்லையெனில்{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        நிறுத்து;
      }
      இயல்பு: {
        ப்ரதிஜ்ஞா( xtype==etINVALID );
        திரும்பு;
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
    எனில்( width>0 ){
      எனில்( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      எனில்( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }இல்லையெனில்{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    எனில்( zExtra ){
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
வெறுமை sqlite3RecordErrorByteOffset(sqlite3 *db, மாறிலி எழுத்து *z){
  மாறிலி Parse *pParse;
  மாறிலி எழுத்து *zText;
  மாறிலி எழுத்து *zEnd;
  ப்ரதிஜ்ஞா( z!=0 );
  எனில்( NEVER(db==0) ) திரும்பு;
  எனில்( db->errByteOffset!=(-2) ) திரும்பு;
  pParse = db->pParse;
  எனில்( NEVER(pParse==0) ) திரும்பு;
  zText =pParse->zTail;
  எனில்( NEVER(zText==0) ) திரும்பு;
  zEnd = &zText[strlen(zText)];
  எனில்( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (முழு)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
வெறுமை sqlite3_Record_Error_Offset_Of_கோவை(sqlite3 *db, மாறிலி கோவை *p_கோவை){
  போது( p_கோவை ){
    எனில்( கோவை_Has_Property(p_கோவை, EP_Reduced|EP_TokenOnly) ) திரும்பு;
    எனில்( கோவை_Has_Property(p_கோவை,EP_OuterON|EP_InnerON) || p_கோவை->w.iOfst<=0 ){
      p_கோவை = p_கோவை->p_இடது;
    }இல்லையெனில்{
      நிறுத்து;
    }
  }
  எனில்( p_கோவை==0 ) திரும்பு;
  எனில்( கோவை_Has_Property(p_கோவை, EP_FromDDL) ) திரும்பு;
  db->errByteOffset = p_கோவை->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
முழு sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  எழுத்து *zNew;
  ப்ரதிஜ்ஞா( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  எனில்( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    திரும்பு 0;
  }
  எனில்( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    திரும்பு p->nAlloc - p->nChar - 1;
  }இல்லையெனில்{
    எழுத்து *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    எனில்( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    எனில்( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      திரும்பு 0;
    }இல்லையெனில்{
      p->nAlloc = (முழு)szNew;
    }
    எனில்( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }இல்லையெனில்{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    எனில்( zNew ){
      ப்ரதிஜ்ஞா( p->zText!=0 || p->nChar==0 );
      எனில்( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }இல்லையெனில்{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      திரும்பு 0;
    }
  }
  ப்ரதிஜ்ஞா( N>=0 && N<=0x7fffffff );
  திரும்பு (முழு)N;
}

முழு sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  எனில்( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  திரும்பு p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
வெறுமை sqlite3_str_appendchar(sqlite3_str *p, முழு N, எழுத்து c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  எனில்( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    திரும்பு;
  }
  போது( (N--)>0 ) p->zText[p->nChar++] = c;
}
நிலையான வெறுமை sqlite3StrAppendchar64(sqlite3_str *p, i64 N, எழுத்து c){
  testcase( p->nChar + N > 0x7fffffff );
  எனில்( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    திரும்பு;
  }
  போது( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
நிலையான வெறுமை SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, மாறிலி எழுத்து *z, முழு N){
  N = sqlite3StrAccumEnlarge(p, N);
  ப்ரதிஜ்ஞா( z!=0 || N==0 );
  எனில்( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
வெறுமை sqlite3_str_append(sqlite3_str *p, மாறிலி எழுத்து *z, முழு N){
  ப்ரதிஜ்ஞா( z!=0 || N==0 );
  ப்ரதிஜ்ஞா( p->zText!=0 || p->nChar==0 || p->accError );
  ப்ரதிஜ்ஞா( N>=0 );
  ப்ரதிஜ்ஞா( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  எனில்( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }இல்லையெனில் எனில்( N ){
    ப்ரதிஜ்ஞா( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
நிலையான வெறுமை sqlite3StrAppend64(sqlite3_str *p, மாறிலி எழுத்து *z, i64 N){
  ப்ரதிஜ்ஞா( z!=0 || N==0 );
  ப்ரதிஜ்ஞா( p->zText!=0 || p->nChar==0 || p->accError );
  ப்ரதிஜ்ஞா( N>=0 );
  ப்ரதிஜ்ஞா( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  எனில்( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }இல்லையெனில் எனில்( N ){
    ப்ரதிஜ்ஞா( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
வெறுமை sqlite3_str_appendall(sqlite3_str *p, மாறிலி எழுத்து *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
நிலையான SQLITE_NOINLINE எழுத்து *strAccumFinishRealloc(StrAccum *p){
  எழுத்து *zText;
  ப்ரதிஜ்ஞா( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  எனில்( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }இல்லையெனில்{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  திரும்பு zText;
}
எழுத்து *sqlite3StrAccumFinish(StrAccum *p){
  எனில்( p->zText ){
    p->zText[p->nChar] = 0;
    எனில்( p->mxAlloc>0 && !isMalloced(p) ){
      திரும்பு strAccumFinishRealloc(p);
    }
  }
  திரும்பு p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
எழுத்து *sqlite3_str_finish(sqlite3_str *p){
  எழுத்து *z;
  எனில்( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }இல்லையெனில்{
    z = 0;
  }
  திரும்பு z;
}

/* Return any error code associated with p */
முழு sqlite3_str_errcode(sqlite3_str *p){
  திரும்பு p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
முழு sqlite3_str_length(sqlite3_str *p){
  திரும்பு p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
வெறுமை sqlite3_str_truncate(sqlite3_str *p, முழு N){
  எனில்( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
எழுத்து *sqlite3_str_மதிப்பு(sqlite3_str *p){
  எனில்( p==0 || p->nChar==0 ) திரும்பு 0;
  p->zText[p->nChar] = 0;
  திரும்பு p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
வெறுமை sqlite3_str_reset(StrAccum *p){
  எனில்( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }இல்லையெனில் எனில்( p==(sqlite3_str*)&sqlite3OomStr ){
    திரும்பு;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
வெறுமை sqlite3_str_free(sqlite3_str *p){
  எனில்( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
வெறுமை sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, எழுத்து *zBase, முழு n, முழு mx){
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
  sqlite3_str *p = sqlite3_malloc64(அளவு(*p));
  எனில்( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }இல்லையெனில்{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  திரும்பு p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
எழுத்து *sqlite3VMPrintf(sqlite3 *db, மாறிலி எழுத்து *zFormat, va_list ap){
  எழுத்து *z;
  எழுத்து zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  ப்ரதிஜ்ஞா( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, அளவு(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  எனில்( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  திரும்பு z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
எழுத்து *sqlite3MPrintf(sqlite3 *db, மாறிலி எழுத்து *zFormat, ...){
  va_list ap;
  எழுத்து *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  திரும்பு z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
எழுத்து *sqlite3_vmprintf(மாறிலி எழுத்து *zFormat, va_list ap){
  எழுத்து *z;
  எழுத்து zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  எனில்( zFormat==0 ){
    (வெறுமை)SQLITE_MISUSE_BKPT;
    திரும்பு 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  எனில்( sqlite3_initialize() ) திரும்பு 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, அளவு(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  திரும்பு z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
எழுத்து *sqlite3_mprintf(மாறிலி எழுத்து *zFormat, ...){
  va_list ap;
  எழுத்து *z;
#ifndef SQLITE_OMIT_AUTOINIT
  எனில்( sqlite3_initialize() ) திரும்பு 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  திரும்பு z;
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
எழுத்து *sqlite3_vsnprintf(முழு n, எழுத்து *zBuf, மாறிலி எழுத்து *zFormat, va_list ap){
  StrAccum acc;
  எனில்( n<=0 ) திரும்பு zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( zBuf==0 || zFormat==0 ) {
    (வெறுமை)SQLITE_MISUSE_BKPT;
    எனில்( zBuf ) zBuf[0] = 0;
    திரும்பு zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  திரும்பு zBuf;
}
எழுத்து *sqlite3_snprintf(முழு n, எழுத்து *zBuf, மாறிலி எழுத்து *zFormat, ...){
  StrAccum acc;
  va_list ap;
  எனில்( n<=0 ) திரும்பு zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( zBuf==0 || zFormat==0 ) {
    (வெறுமை)SQLITE_MISUSE_BKPT;
    எனில்( zBuf ) zBuf[0] = 0;
    திரும்பு zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  திரும்பு zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#எனில் defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#அல்லதெனில் SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#இல்லையெனில்
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
நிலையான வெறுமை render_Log_செய்தி(முழு iErrCode, மாறிலி எழுத்து *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  எழுத்து z_செய்தி[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_செய்தி, அளவு(z_செய்தி), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
வெறுமை sqlite3_log(முழு iErrCode, மாறிலி எழுத்து *zFormat, ...){
  va_list ap;                             /* Vararg list */
  எனில்( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_செய்தி(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#எனில் defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
வெறுமை sqlite3DebugPrintf(மாறிலி எழுத்து *zFormat, ...){
  va_list ap;
  StrAccum acc;
  எழுத்து zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, அளவு(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    வெளி வெறுமை SQLITE_OS_TRACE_PROC(மாறிலி எழுத்து *zBuf, முழு nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, அளவு(zBuf));
  }
#இல்லையெனில்
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
வெறுமை sqlite3_str_appendf(StrAccum *p, மாறிலி எழுத்து *zFormat, ...){
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
எழுத்து *sqlite3RCStrRef(எழுத்து *z){
  RCStr *p = (RCStr*)z;
  ப்ரதிஜ்ஞா( p!=0 );
  p--;
  p->nRCRef++;
  திரும்பு z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
வெறுமை sqlite3RCStrUnref(வெறுமை *z){
  RCStr *p = (RCStr*)z;
  ப்ரதிஜ்ஞா( p!=0 );
  p--;
  ப்ரதிஜ்ஞா( p->nRCRef>0 );
  எனில்( p->nRCRef>=2 ){
    p->nRCRef--;
  }இல்லையெனில்{
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
எழுத்து *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + அளவு(*p) + 1 );
  எனில்( p==0 ) திரும்பு 0;
  p->nRCRef = 1;
  திரும்பு (எழுத்து*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
எழுத்து *sqlite3RCStrResize(எழுத்து *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  ப்ரதிஜ்ஞா( p!=0 );
  p--;
  ப்ரதிஜ்ஞா( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+அளவு(RCStr)+1);
  எனில்( pNew==0 ){
    sqlite3_free(p);
    திரும்பு 0;
  }இல்லையெனில்{
    திரும்பு (எழுத்து*)&pNew[1];
  }
}
