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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

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
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ଅକ୍ଷର etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା et_info {   /* Information about each format field */
  ଅକ୍ଷର fmttype;            /* The format field code letter */
  etByte base;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte prefix;           /* Offset into aPrefix[] of the prefix string */
  ଅକ୍ଷର iNxt;               /* Next with same hash, or 0 for end of chain */
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
#ଯଦି 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  scan $c %c x
  set n($c) $x
}
set mx [llength [array names n]]
puts "count: $mx"

set mx 25
puts "*********** mx=$mx ************"
ପାଇଁ {set r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array names n] {
    ଯଦି {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#endif /***** End of script ********/

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର aDigits[] = "0123456789ABCDEF0123456789abcdef";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର aHex[]    = "0123456789abcdef";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର aPrefix[] = "-x0\000X0";
ସ୍ଥିର ଅଚଳ et_info fmtinfo[25] = {
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
ଶୂନ୍ୟ sqlite3StrAccumSetError(StrAccum *p, u8 eError){
  ପ୍ରତିଜ୍ଞା( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  ଯଦି( p->mxAlloc ) sqlite3_str_reset(p);
  ଯଦି( eError==SQLITE_TOOBIG ) sqlite3ErrorToParser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ସ୍ଥିର sqlite3_int64 getIntArg(PrintfArguments *p){
  ଯଦି( p->nArg<=p->nUsed ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_int64(p->apArg[p->nUsed++]);
}
ସ୍ଥିର ଦ୍ୱିଗୁଣ getDoubleArg(PrintfArguments *p){
  ଯଦି( p->nArg<=p->nUsed ) ଫେରନ୍ତୁ 0.0;
  ଫେରନ୍ତୁ sqlite3_ମୂଲ୍ୟ_double(p->apArg[p->nUsed++]);
}
ସ୍ଥିର ଅକ୍ଷର *getTextArg(PrintfArguments *p){
  ଯଦି( p->nArg<=p->nUsed ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(p->apArg[p->nUsed++]);
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
ସ୍ଥିର ଅକ୍ଷର *printfTempBuf(sqlite3_str *pAccum, sqlite3_int64 n){
  ଅକ୍ଷର *z;
  ଯଦି( pAccum->accError ) ଫେରନ୍ତୁ 0;
  ଯଦି( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3StrAccumSetError(pAccum, SQLITE_TOOBIG);
    ଫେରନ୍ତୁ 0;
  }
  z = sqlite3_malloc(n);
  ଯଦି( z==0 ){
    sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
  }
  ଫେରନ୍ତୁ z;
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
# ଯଦି SQLITE_PRINTF_PRECISION_LIMIT<100000000
#   define SQLITE_FP_PRECISION_LIMIT SQLITE_PRINTF_PRECISION_LIMIT
# ଅନ୍ୟଥା
#   define SQLITE_FP_PRECISION_LIMIT 100000000
# endif
#ଅନ୍ୟଥା
# define SQLITE_FP_PRECISION_LIMIT 100000000
#endif

/* Forward reference */
ସ୍ଥିର ଶୂନ୍ୟ sqlite3StrAppend64(sqlite3_str *p, ଅଚଳ ଅକ୍ଷର *z, i64 N);
ସ୍ଥିର ଶୂନ୍ୟ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, ଅକ୍ଷର c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
ଶୂନ୍ୟ sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ଅଚଳ ଅକ୍ଷର *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;                     /* Next character in the format string */
  ଅକ୍ଷର *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 length;                /* Length of the field */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;                   /* A general purpose loop counter */
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
  ଅକ୍ଷର prefix;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  ଦ୍ୱିଗୁଣ realvalue;          /* Value for real types */
  ଅଚଳ et_info *infop;      /* Pointer to the appropriate info structure */
  ଅକ୍ଷର *zOut;                /* Rendering buffer */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut;                  /* Size of the rendering buffer */
  ଅକ୍ଷର *zExtra = 0;          /* Malloced memory used by some conversion */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା exp, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  ଅକ୍ଷର buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  ପ୍ରତିଜ୍ଞା( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  ଯଦି( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }ଅନ୍ୟଥା{
    bArgList = 0;
  }
  ପାଇଁ(; (c=(*fmt))!=0; ++fmt){
    ଯଦି( c!='%' ){
      bufpt = (ଅକ୍ଷର *)fmt;
#ଯଦି HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#ଅନ୍ୟଥା
      fmt = strchr(fmt, '%');
      ଯଦି( fmt==0 ){
        fmt = bufpt + strlen(bufpt);
      }
#endif
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      ଯଦି( *fmt==0 ) ଭାଙ୍ଗ;
    }
    ଯଦି( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      ଭାଙ୍ଗ;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_prefix = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    done = 0;
    width = 0;
    flag_long = 0;
    precision = -1;
    କର{
      ବାଛ( c ){
        ସ୍ଥିତି '-':   flag_leftjustify = 1;     ଭାଙ୍ଗ;
        ସ୍ଥିତି '+':   flag_prefix = '+';        ଭାଙ୍ଗ;
        ସ୍ଥିତି ' ':   flag_prefix = ' ';        ଭାଙ୍ଗ;
        ସ୍ଥିତି '#':   flag_alternateform = 1;   ଭାଙ୍ଗ;
        ସ୍ଥିତି '!':   flag_altform2 = 1;        ଭାଙ୍ଗ;
        ସ୍ଥିତି '0':   flag_zeropad = 1;         ଭାଙ୍ଗ;
        ସ୍ଥିତି ',':   cThousand = ',';          ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:    done = 1;                 ଭାଙ୍ଗ;
        ସ୍ଥିତି 'l': {
          flag_long = 1;
          c = *++fmt;
          ଯଦି( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          done = 1;
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି '1': ସ୍ଥିତି '2': ସ୍ଥିତି '3': ସ୍ଥିତି '4': ସ୍ଥିତି '5':
        ସ୍ଥିତି '6': ସ୍ଥିତି '7': ସ୍ଥିତି '8': ସ୍ଥିତି '9': {
          unsigned wx = c - '0';
          ଯେତେବେଳେ( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          width = wx & 0x7fffffff;
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ଯଦି( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ଯଦି( c!='.' && c!='l' ){
            done = 1;
          }ଅନ୍ୟଥା{
            fmt--;
          }
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି '*': {
          ଯଦି( bArgList ){
            width = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)getIntArg(pArgList);
          }ଅନ୍ୟଥା{
            width = va_arg(ap,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
          }
          ଯଦି( width<0 ){
            flag_leftjustify = 1;
            width = width >= -2147483647 ? -width : 0;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ଯଦି( width>SQLITE_PRINTF_PRECISION_LIMIT ){
            width = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ଯଦି( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            done = 1;
          }
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି '.': {
          c = *++fmt;
          ଯଦି( c=='*' ){
            ଯଦି( bArgList ){
              precision = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)getIntArg(pArgList);
            }ଅନ୍ୟଥା{
              precision = va_arg(ap,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
            }
            ଯଦି( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }ଅନ୍ୟଥା{
            unsigned px = 0;
            ଯେତେବେଳେ( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#ifdef SQLITE_PRINTF_PRECISION_LIMIT
          ଯଦି( precision>SQLITE_PRINTF_PRECISION_LIMIT ){
            precision = SQLITE_PRINTF_PRECISION_LIMIT;
          }
#endif
          ଯଦି( c=='l' ){
            --fmt;
          }ଅନ୍ୟଥା{
            done = 1;
          }
          ଭାଙ୍ଗ;
        }
      }
    }ଯେତେବେଳେ( !done && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#ifdef SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    ପାଇଁ(idx=0; idx<ArraySize(fmtinfo); idx++){
      ଯଦି( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        ଭାଙ୍ଗ;
      }
    }
#ଅନ୍ୟଥା
    /* Fast hash-table lookup */
    ପ୍ରତିଜ୍ଞା( ArraySize(fmtinfo)==25 );
    idx = ((unsigned)c) % 25;
    ଯଦି( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }ଅନ୍ୟଥା{
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
    ପ୍ରତିଜ୍ଞା( width>=0 );
    ପ୍ରତିଜ୍ଞା( precision>=(-1) );
    ବାଛ( xtype ){
      ସ୍ଥିତି etPOINTER:
        flag_long = ଆକାର(ଅକ୍ଷର*)==ଆକାର(i64) ? 2 :
                     ଆକାର(ଅକ୍ଷର*)==ଆକାର(long ପୂର୍ଣ୍ଣସଂଖ୍ୟା) ? 1 : 0;
        /* no break */ deliberate_fall_through
      ସ୍ଥିତି etORDINAL:
      ସ୍ଥିତି etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      ସ୍ଥିତି etDECIMAL:
        ଯଦି( infop->flags & FLAG_SIGNED ){
          i64 v;
          ଯଦି( bArgList ){
            v = getIntArg(pArgList);
          }ଅନ୍ୟଥା ଯଦି( flag_long ){
            ଯଦି( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }ଅନ୍ୟଥା{
              v = va_arg(ap,long ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
            }
          }ଅନ୍ୟଥା{
            v = va_arg(ap,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
          }
          ଯଦି( v<0 ){
            testcase( v==SMALLEST_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            prefix = '-';
          }ଅନ୍ୟଥା{
            longvalue = v;
            prefix = flag_prefix;
          }
        }ଅନ୍ୟଥା{
          ଯଦି( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }ଅନ୍ୟଥା ଯଦି( flag_long ){
            ଯଦି( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }ଅନ୍ୟଥା{
              longvalue = va_arg(ap,unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
            }
          }ଅନ୍ୟଥା{
            longvalue = va_arg(ap,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
          }
          prefix = 0;
        }

#ଯଦି WHERETRACE_ENABLED
        ଯଦି( xtype==etPOINTER && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#endif
#ଯଦି TREETRACE_ENABLED
        ଯଦି( xtype==etPOINTER && sqlite3_ବୃକ୍ଷ_Trace & 0x100000 ) longvalue = 0;
#endif

        ଯଦି( longvalue==0 ) flag_alternateform = 0;
        ଯଦି( flag_zeropad && precision<width-(prefix!=0) ){
          precision = width-(prefix!=0);
        }
        ଯଦି( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          nOut = etBUFSIZE;
          zOut = buf;
        }ଅନ୍ୟଥା{
          u64 n;
          n = (u64)precision + 10;
          ଯଦି( cThousand ) n += precision/3;
          zOut = zExtra = printfTempBuf(pAccum, n);
          ଯଦି( zOut==0 ) ଫେରନ୍ତୁ;
          nOut = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
        }
        bufpt = &zOut[nOut-1];
        ଯଦି( xtype==etORDINAL ){
          ସ୍ଥିର ଅଚଳ ଅକ୍ଷର zOrd[] = "thstndrd";
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(longvalue % 10);
          ଯଦି( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ଅଚଳ ଅକ୍ଷର *cset = &aDigits[infop->charset];
          u8 base = infop->base;
          କର{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%base];
            longvalue = longvalue/base;
          }ଯେତେବେଳେ( longvalue>0 );
        }
        length = &zOut[nOut-1] - bufpt;
        ଯଦି( precision>length ){                         /* zero pad */
          i64 nn = precision-length;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          length = precision;
        }
        ଯଦି( cThousand ){
          i64 nn = (length - 1)/3;  /* Number of "," to insert */
          i64 ix = (length - 1)%3 + 1;
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
          bufpt -= nn;
          ପାଇଁ(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            ଯଦି( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        ଯଦି( prefix ) *(--bufpt) = prefix;               /* Add sign */
        ଯଦି( flag_alternateform && infop->prefix ){      /* Add "0" or "0x" */
          ଅଚଳ ଅକ୍ଷର *pre;
          ଅକ୍ଷର x;
          pre = &aPrefix[infop->prefix];
          ପାଇଁ(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        length = &zOut[nOut-1] - bufpt;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି etFLOAT:
      ସ୍ଥିତି etEXP:
      ସ୍ଥିତି etGENERIC: {
        FpDecode s;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRound;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        ଯଦି( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }ଅନ୍ୟଥା{
          realvalue = va_arg(ap,ଦ୍ୱିଗୁଣ);
        }
        ଯଦି( precision<0 ) precision = 6;         /* Set default precision */
        ଯଦି( precision>SQLITE_FP_PRECISION_LIMIT ){
          precision = SQLITE_FP_PRECISION_LIMIT;
        }
        ଯଦି( xtype==etFLOAT ){
          iRound = -precision;
        }ଅନ୍ୟଥା ଯଦି( xtype==etGENERIC ){
          ଯଦି( precision==0 ) precision = 1;
          iRound = precision;
        }ଅନ୍ୟଥା{
          iRound = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, iRound, flag_altform2 ? 20 : 16);
        ଯଦି( s.isSpecial ){
          ଯଦି( s.isSpecial==2 ){
            ଯଦି( flag_zeropad ){
              bufpt = "null";
              length = 4;
            }ଅନ୍ୟଥା{
              bufpt = "NaN";
              length = 3;
            }
            ଭାଙ୍ଗ;
          }ଅନ୍ୟଥା ଯଦି( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }ଅନ୍ୟଥା{
            memcpy(buf, "-Inf", 5);
            bufpt = buf;
            ଯଦି( s.sign=='-' ){
              /* no-op */
            }ଅନ୍ୟଥା ଯଦି( flag_prefix ){
              buf[0] = flag_prefix;
            }ଅନ୍ୟଥା{
              bufpt++;
            }
            length = strlen(bufpt);
            ଭାଙ୍ଗ;
          }
        }
        ଯଦି( s.sign=='-' ){
          ଯଦି( flag_alternateform
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
          }ଅନ୍ୟଥା{
            prefix = '-';
          }
        }ଅନ୍ୟଥା{
          prefix = flag_prefix;
        }

        exp = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        ଯଦି( xtype==etGENERIC ){
          ପ୍ରତିଜ୍ଞା( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          ଯଦି( exp<-4 || exp>precision ){
            xtype = etEXP;
          }ଅନ୍ୟଥା{
            precision = precision - exp;
            xtype = etFLOAT;
          }
        }ଅନ୍ୟଥା{
          flag_rtz = flag_altform2;
        }
        ଯଦି( xtype==etEXP ){
          e2 = 0;
        }ଅନ୍ୟଥା{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)width+10;
        ଯଦି( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        ଯଦି( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          ଯଦି( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            ଯଦି( bufpt==0 ){
              sqlite3StrAccumSetError(pAccum, SQLITE_NOMEM);
              ଫେରନ୍ତୁ;
            }
            zExtra = bufpt;
          }ଅନ୍ୟଥା ଯଦି( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            width = length = 0;
            ଭାଙ୍ଗ;
          }ଅନ୍ୟଥା{
            bufpt = pAccum->zText + pAccum->nChar;
          }
        }ଅନ୍ୟଥା{
          bufpt = pAccum->zText + pAccum->nChar;
        }
        zOut = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        ଯଦି( prefix ){
          *(bufpt++) = prefix;
        }
        /* Digits prior to the decimal point */
        j = 0;
        ପ୍ରତିଜ୍ଞା( s.n>0 );
        ଯଦି( e2<0 ){
          *(bufpt++) = '0';
        }ଅନ୍ୟଥା ଯଦି( cThousand ){
          ପାଇଁ(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            ଯଦି( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }ଅନ୍ୟଥା{
          j = e2+1;
          ଯଦି( j>s.n ) j = s.n;
          memcpy(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          ଯଦି( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        ଯଦି( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        ଯଦି( e2<(-1) && precision>0 ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nn = -1-e2;
          ଯଦି( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        ଯଦି( precision>0 ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nn = s.n - j;
          ଯଦି( NEVER(nn>precision) ) nn = precision;
          ଯଦି( nn>0 ){
            memcpy(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          ଯଦି( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        ଯଦି( flag_rtz && flag_dp ){
          ଯେତେବେଳେ( bufpt[-1]=='0' ) *(--bufpt) = 0;
          ପ୍ରତିଜ୍ଞା( bufpt>zOut );
          ଯଦି( bufpt[-1]=='.' ){
            ଯଦି( flag_altform2 ){
              *(bufpt++) = '0';
            }ଅନ୍ୟଥା{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        ଯଦି( xtype==etEXP ){
          exp = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          ଯଦି( exp<0 ){
            *(bufpt++) = '-'; exp = -exp;
          }ଅନ୍ୟଥା{
            *(bufpt++) = '+';
          }
          ଯଦି( exp>=100 ){
            *(bufpt++) = (ଅକ୍ଷର)((exp/100)+'0');        /* 100's digit */
            exp %= 100;
          }
          *(bufpt++) = (ଅକ୍ଷର)(exp/10+'0');             /* 10's digit */
          *(bufpt++) = (ଅକ୍ଷର)(exp%10+'0');             /* 1's digit */
        }

        length = bufpt - zOut;
        ପ୍ରତିଜ୍ଞା( length <= szBufNeeded );
        ଯଦି( length<width ){
          i64 nPad = width - length;
          ଯଦି( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }ଅନ୍ୟଥା ଯଦି( !flag_zeropad ){
            memmove(zOut+nPad, zOut, length);
            memset(zOut, ' ', nPad);
          }ଅନ୍ୟଥା{
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା adj = prefix!=0;
            memmove(zOut+nPad+adj, zOut+adj, length-adj);
            memset(zOut+adj, '0', nPad);
          }
          length = width;
        }

        ଯଦି( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          ପ୍ରତିଜ୍ଞା( pAccum->nChar + length < SMXV(pAccum->nChar) );
          pAccum->nChar += length;
          zOut[length] = 0;
          ଜାରି;
        }ଅନ୍ୟଥା{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          ଭାଙ୍ଗ;
        }
      }
      ସ୍ଥିତି etSIZE:
        ଯଦି( !bArgList ){
          *(va_arg(ap,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)) = pAccum->nChar;
        }
        length = width = 0;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        length = 1;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି etCHARX:
        ଯଦି( bArgList ){
          bufpt = getTextArg(pArgList);
          length = 1;
          ଯଦି( bufpt ){
            buf[0] = c = *(bufpt++);
            ଯଦି( (c&0xc0)==0xc0 ){
              ଯେତେବେଳେ( length<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[length++] = *(bufpt++);
              }
            }
          }ଅନ୍ୟଥା{
            buf[0] = 0;
          }
        }ଅନ୍ୟଥା{
          unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = va_arg(ap,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
          length = sqlite3AppendOneUtf8Character(buf, ch);
        }
        ଯଦି( precision>1 ){
          i64 nPrior = 1;
          width -= precision-1;
          ଯଦି( width>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, width-1, ' ');
            width = 0;
          }
          sqlite3StrAppend64(pAccum, buf, length);
          precision--;
          ଯେତେବେଳେ( precision > 1 ){
            i64 nCopyBytes;
            ଯଦି( nPrior > precision-1 ) nPrior = precision - 1;
            nCopyBytes = length*nPrior;
            ଯଦି( sqlite3StrAccumEnlargeIfNeeded(pAccum, nCopyBytes) ){
              ଭାଙ୍ଗ;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->zText[pAccum->nChar-nCopyBytes], nCopyBytes);
            precision -= nPrior;
            nPrior *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        ଯାଅ adjust_width_for_utf8;
      ସ୍ଥିତି etSTRING:
      ସ୍ଥିତି etDYNSTRING:
        ଯଦି( bArgList ){
          bufpt = getTextArg(pArgList);
          xtype = etSTRING;
        }ଅନ୍ୟଥା{
          bufpt = va_arg(ap,ଅକ୍ଷର*);
        }
        ଯଦି( bufpt==0 ){
          bufpt = "";
        }ଅନ୍ୟଥା ଯଦି( xtype==etDYNSTRING ){
          ଯଦି( pAccum->nChar==0
           && pAccum->mxAlloc
           && width==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            ପ୍ରତିଜ୍ଞା( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->zText = bufpt;
            pAccum->nAlloc = sqlite3DbMallocSize(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            length = 0;
            ଭାଙ୍ଗ;
          }
          zExtra = bufpt;
        }
        ଯଦି( precision>=0 ){
          ଯଦି( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            unsigned ଅକ୍ଷର *z = (unsigned ଅକ୍ଷର*)bufpt;
            ଯେତେବେଳେ( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            length = z - (unsigned ଅକ୍ଷର*)bufpt;
          }ଅନ୍ୟଥା{
            ପାଇଁ(length=0; length<precision && bufpt[length]; length++){}
          }
        }ଅନ୍ୟଥା{
          length = strlen(bufpt);
        }
      adjust_width_for_utf8:
        ଯଦି( flag_altform2 && width>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = length - 1;
          ଯେତେବେଳେ( ii>=0 ) ଯଦି( (bufpt[ii--] & 0xc0)==0x80 ) width++;
        }
        ଭାଙ୍ଗ;
      ସ୍ଥିତି etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      ସ୍ଥିତି etESCAPE_J: {         /* %J: Generate a JSON string literal */
        ଅକ୍ଷର *escarg;
        i64 i, j, px, iStart;
        unsigned ଅକ୍ଷର ch;

        ଯଦି( bArgList ){
          escarg = getTextArg(pArgList);
        }ଅନ୍ୟଥା{
          escarg = va_arg(ap,ଅକ୍ଷର*);
        }
        iStart = sqlite3_str_length(pAccum);
        ଯଦି( escarg==0 ){
          ଯଦି( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }ଅନ୍ୟଥା{
          ଯଦି( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          ଯଦି( px<0 ){
            px = 0x7fffffff;
          }ଅନ୍ୟଥା ଯଦି( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            ପାଇଁ(i=0; i<px && escarg[i]; i++){
              ଯଦି( (escarg[i]&0xc0)==0x80 ) px++;
            }
            ଯଦି( i==px ){
              ଯେତେବେଳେ( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          ପାଇଁ(i=j=0; i<px; i++){
            ଯଦି( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              ଯଦି( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              ଯଦି( ch==0 ) ଭାଙ୍ଗ;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              ଯଦି( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ଅନ୍ୟଥା ଯଦି( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ଅନ୍ୟଥା{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          ଯଦି( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          ଯଦି( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        ଯଦି( width>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_length(pAccum) - iStart;
          sqlite3_int64 len = n;
          ଅକ୍ଷର *zz;
          ଯଦି( flag_altform2 && n>0 ){
            zz = sqlite3_str_ମୂଲ୍ୟ(pAccum);
            ପାଇଁ(i=iStart; zz[i]; i++){
              ଯଦି( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          ଯଦି( width>len ){
            sqlite3_int64 sp = width-len;
            ପ୍ରତିଜ୍ଞା( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sp, ' ');
            ଯଦି( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_ମୂଲ୍ୟ(pAccum);
              zz += iStart;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        ଜାରି;
      }
      ସ୍ଥିତି etESCAPE_q:          /* %q: Escape ' characters */
      ସ୍ଥିତି etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      ସ୍ଥିତି etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା needQuote = 0;
        ଅକ୍ଷର ch;
        ଅକ୍ଷର *escarg;
        ଅକ୍ଷର q;

        ଯଦି( bArgList ){
          escarg = getTextArg(pArgList);
        }ଅନ୍ୟଥା{
          escarg = va_arg(ap,ଅକ୍ଷର*);
        }
        ଯଦି( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }ଅନ୍ୟଥା ଯଦି( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        ଯଦି( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }ଅନ୍ୟଥା{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        ପାଇଁ(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          ଯଦି( ch==q )  n++;
          ଯଦି( flag_altform2 && (ch&0xc0)==0xc0 ){
            ଯେତେବେଳେ( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        ଯଦି( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 nBack = 0;
          i64 nCtrl = 0;
          ପାଇଁ(k=0; k<i; k++){
            ଯଦି( escarg[k]=='\\' ){
              nBack++;
            }ଅନ୍ୟଥା ଯଦି( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          ଯଦି( nCtrl || xtype==etESCAPE_q ){
            n += nBack + 5*nCtrl;
            ଯଦି( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }ଅନ୍ୟଥା{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        ଯଦି( n>etBUFSIZE ){
          bufpt = zExtra = printfTempBuf(pAccum, n);
          ଯଦି( bufpt==0 ) ଫେରନ୍ତୁ;
        }ଅନ୍ୟଥା{
          bufpt = buf;
        }
        j = 0;
        ଯଦି( needQuote ){
          ଯଦି( needQuote==2 ){
            memcpy(&bufpt[j], "unistr('", 8);
            j += 8;
          }ଅନ୍ୟଥା{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        ଯଦି( flag_alternateform ){
          ପାଇଁ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ଯଦି( ch==q ){
              bufpt[j++] = ch;
            }ଅନ୍ୟଥା ଯଦି( ch=='\\' ){
              bufpt[j++] = '\\';
            }ଅନ୍ୟଥା ଯଦି( ((unsigned ଅକ୍ଷର)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }ଅନ୍ୟଥା{
          ପାଇଁ(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ଯଦି( ch==q ) bufpt[j++] = ch;
          }
        }
        ଯଦି( needQuote ){
          bufpt[j++] = '\'';
          ଯଦି( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        length = j;
        ଯାଅ adjust_width_for_utf8;
      }
      ସ୍ଥିତି etTOKEN: {
        ଯଦି( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ଫେରନ୍ତୁ;
        ଯଦି( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = va_arg(ap,ପଦସମଷ୍ଟି*);
          ଯଦି( ALWAYS(p_ପଦସମଷ୍ଟି) && ALWAYS(!ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_Int_ମୂଲ୍ୟ)) ){
            sqlite3_str_appendall(pAccum, (ଅଚଳ ଅକ୍ଷର*)p_ପଦସମଷ୍ଟି->u.zToken);
            sqlite3_Record_Error_Offset_Of_ପଦସମଷ୍ଟି(pAccum->db, p_ପଦସମଷ୍ଟି);
          }
        }ଅନ୍ୟଥା{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          ପ୍ରତିଜ୍ଞା( bArgList==0 );
          ଯଦି( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ଅଚଳ ଅକ୍ଷର*)pToken->z, pToken->n);
            sqlite3RecordErrorByteOffset(pAccum->db, pToken->z);
          }
        }
        length = width = 0;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି etSRCITEM: {
        SrcItem *pItem;
        ଯଦି( (pAccum->printfFlags & SQLITE_PRINTF_INTERNAL)==0 ) ଫେରନ୍ତୁ;
        pItem = va_arg(ap, SrcItem*);
        ପ୍ରତିଜ୍ଞା( bArgList==0 );
        ଯଦି( pItem->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }ଅନ୍ୟଥା ଯଦି( pItem->zName ){
          ଯଦି( pItem->fg.fixedSchema==0
           && pItem->fg.isSubquery==0
           && pItem->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, pItem->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, pItem->zName);
        }ଅନ୍ୟଥା ଯଦି( pItem->zAlias ){
          sqlite3_str_appendall(pAccum, pItem->zAlias);
        }ଅନ୍ୟଥା ଯଦି( ALWAYS(pItem->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = pItem->u4.pSubq->pSelect;
          ପ୍ରତିଜ୍ଞା( pSel!=0 ); 
          ଯଦି( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }ଅନ୍ୟଥା ଯଦି( pSel->selFlags & SF_Multi_ମୂଲ୍ୟ ){
            ପ୍ରତିଜ୍ଞା( !pItem->fg.isTabFunc && !pItem->fg.isIndexedBy );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                pItem->u1.nRow);
          }ଅନ୍ୟଥା{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        length = width = 0;
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        ପ୍ରତିଜ୍ଞା( xtype==etINVALID );
        ଫେରନ୍ତୁ;
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
    ଯଦି( width>0 ){
      ଯଦି( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
      sqlite3StrAppend64(pAccum, bufpt, length);
      ଯଦି( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, width, ' ');
    }ଅନ୍ୟଥା{
      sqlite3StrAppend64(pAccum, bufpt, length);
    }

    ଯଦି( zExtra ){
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
ଶୂନ୍ୟ sqlite3RecordErrorByteOffset(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *z){
  ଅଚଳ Parse *pParse;
  ଅଚଳ ଅକ୍ଷର *zText;
  ଅଚଳ ଅକ୍ଷର *zEnd;
  ପ୍ରତିଜ୍ଞା( z!=0 );
  ଯଦି( NEVER(db==0) ) ଫେରନ୍ତୁ;
  ଯଦି( db->errByteOffset!=(-2) ) ଫେରନ୍ତୁ;
  pParse = db->pParse;
  ଯଦି( NEVER(pParse==0) ) ଫେରନ୍ତୁ;
  zText =pParse->zTail;
  ଯଦି( NEVER(zText==0) ) ଫେରନ୍ତୁ;
  zEnd = &zText[strlen(zText)];
  ଯଦି( SQLITE_WITHIN(z,zText,zEnd) ){
    db->errByteOffset = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(z-zText);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
ଶୂନ୍ୟ sqlite3_Record_Error_Offset_Of_ପଦସମଷ୍ଟି(sqlite3 *db, ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯେତେବେଳେ( p_ପଦସମଷ୍ଟି ){
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Reduced|EP_TokenOnly) ) ଫେରନ୍ତୁ;
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_OuterON|EP_InnerON) || p_ପଦସମଷ୍ଟି->w.iOfst<=0 ){
      p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ବାମ;
    }ଅନ୍ୟଥା{
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( p_ପଦସମଷ୍ଟି==0 ) ଫେରନ୍ତୁ;
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FromDDL) ) ଫେରନ୍ତୁ;
  db->errByteOffset = p_ପଦସମଷ୍ଟି->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  ଅକ୍ଷର *zNew;
  ପ୍ରତିଜ୍ଞା( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  ଯଦି( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( p->mxAlloc==0 ){
    sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
    ଫେରନ୍ତୁ p->nAlloc - p->nChar - 1;
  }ଅନ୍ୟଥା{
    ଅକ୍ଷର *zOld = isMalloced(p) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    ଯଦି( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    ଯଦି( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_TOOBIG);
      ଫେରନ୍ତୁ 0;
    }ଅନ୍ୟଥା{
      p->nAlloc = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)szNew;
    }
    ଯଦି( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }ଅନ୍ୟଥା{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    ଯଦି( zNew ){
      ପ୍ରତିଜ୍ଞା( p->zText!=0 || p->nChar==0 );
      ଯଦି( !isMalloced(p) && p->nChar>0 ) memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }ଅନ୍ୟଥା{
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, SQLITE_NOMEM);
      ଫେରନ୍ତୁ 0;
    }
  }
  ପ୍ରତିଜ୍ଞା( N>=0 && N<=0x7fffffff );
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)N;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  ଯଦି( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  ଫେରନ୍ତୁ p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
ଶୂନ୍ୟ sqlite3_str_appendchar(sqlite3_str *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N, ଅକ୍ଷର c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  ଯଦି( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ଫେରନ୍ତୁ;
  }
  ଯେତେବେଳେ( (N--)>0 ) p->zText[p->nChar++] = c;
}
ସ୍ଥିର ଶୂନ୍ୟ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, ଅକ୍ଷର c){
  testcase( p->nChar + N > 0x7fffffff );
  ଯଦି( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    ଫେରନ୍ତୁ;
  }
  ଯେତେବେଳେ( (N--)>0 ) p->zText[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
ସ୍ଥିର ଶୂନ୍ୟ SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ଅଚଳ ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  N = sqlite3StrAccumEnlarge(p, N);
  ପ୍ରତିଜ୍ଞା( z!=0 || N==0 );
  ଯଦି( N>0 ){
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
ଶୂନ୍ୟ sqlite3_str_append(sqlite3_str *p, ଅଚଳ ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ପ୍ରତିଜ୍ଞା( z!=0 || N==0 );
  ପ୍ରତିଜ୍ଞା( p->zText!=0 || p->nChar==0 || p->accError );
  ପ୍ରତିଜ୍ଞା( N>=0 );
  ପ୍ରତିଜ୍ଞା( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ଯଦି( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ଅନ୍ୟଥା ଯଦି( N ){
    ପ୍ରତିଜ୍ଞା( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}
ସ୍ଥିର ଶୂନ୍ୟ sqlite3StrAppend64(sqlite3_str *p, ଅଚଳ ଅକ୍ଷର *z, i64 N){
  ପ୍ରତିଜ୍ଞା( z!=0 || N==0 );
  ପ୍ରତିଜ୍ଞା( p->zText!=0 || p->nChar==0 || p->accError );
  ପ୍ରତିଜ୍ଞା( N>=0 );
  ପ୍ରତିଜ୍ଞା( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ଯଦି( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ଅନ୍ୟଥା ଯଦି( N ){
    ପ୍ରତିଜ୍ଞା( p->zText );
    p->nChar += N;
    memcpy(&p->zText[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
ଶୂନ୍ୟ sqlite3_str_appendall(sqlite3_str *p, ଅଚଳ ଅକ୍ଷର *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
ସ୍ଥିର SQLITE_NOINLINE ଅକ୍ଷର *strAccumFinishRealloc(StrAccum *p){
  ଅକ୍ଷର *zText;
  ପ୍ରତିଜ୍ଞା( p->mxAlloc>0 && !isMalloced(p) );
  zText = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  ଯଦି( zText ){
    memcpy(zText, p->zText, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }ଅନ୍ୟଥା{
    sqlite3StrAccumSetError(p, SQLITE_NOMEM);
  }
  p->zText = zText;
  ଫେରନ୍ତୁ zText;
}
ଅକ୍ଷର *sqlite3StrAccumFinish(StrAccum *p){
  ଯଦି( p->zText ){
    p->zText[p->nChar] = 0;
    ଯଦି( p->mxAlloc>0 && !isMalloced(p) ){
      ଫେରନ୍ତୁ strAccumFinishRealloc(p);
    }
  }
  ଫେରନ୍ତୁ p->zText;
}

/* Finalize a string created using sqlite3_str_new().
*/
ଅକ୍ଷର *sqlite3_str_finish(sqlite3_str *p){
  ଅକ୍ଷର *z;
  ଯଦି( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3StrAccumFinish(p);
    sqlite3_free(p);
  }ଅନ୍ୟଥା{
    z = 0;
  }
  ଫେରନ୍ତୁ z;
}

/* Return any error code associated with p */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_str_errcode(sqlite3_str *p){
  ଫେରନ୍ତୁ p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_str_length(sqlite3_str *p){
  ଫେରନ୍ତୁ p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
ଶୂନ୍ୟ sqlite3_str_truncate(sqlite3_str *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଯଦି( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->zText[p->nChar] = 0;
  }
}

/* Return the current value for p */
ଅକ୍ଷର *sqlite3_str_ମୂଲ୍ୟ(sqlite3_str *p){
  ଯଦି( p==0 || p->nChar==0 ) ଫେରନ୍ତୁ 0;
  p->zText[p->nChar] = 0;
  ଫେରନ୍ତୁ p->zText;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
ଶୂନ୍ୟ sqlite3_str_reset(StrAccum *p){
  ଯଦି( isMalloced(p) ){
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }ଅନ୍ୟଥା ଯଦି( p==(sqlite3_str*)&sqlite3OomStr ){
    ଫେରନ୍ତୁ;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
ଶୂନ୍ୟ sqlite3_str_free(sqlite3_str *p){
  ଯଦି( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
ଶୂନ୍ୟ sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, ଅକ୍ଷର *zBase, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mx){
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
  sqlite3_str *p = sqlite3_malloc64(ଆକାର(*p));
  ଯଦି( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->aLimit[SQLITE_LIMIT_LENGTH] : SQLITE_MAX_LENGTH);
  }ଅନ୍ୟଥା{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  ଫେରନ୍ତୁ p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
ଅକ୍ଷର *sqlite3VMPrintf(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *zFormat, va_list ap){
  ଅକ୍ଷର *z;
  ଅକ୍ଷର zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;
  ପ୍ରତିଜ୍ଞା( db!=0 );
  sqlite3StrAccumInit(&acc, db, zBase, ଆକାର(zBase),
                      db->aLimit[SQLITE_LIMIT_LENGTH]);
  acc.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ଯଦି( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ଫେରନ୍ତୁ z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
ଅକ୍ଷର *sqlite3MPrintf(sqlite3 *db, ଅଚଳ ଅକ୍ଷର *zFormat, ...){
  va_list ap;
  ଅକ୍ଷର *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  ଫେରନ୍ତୁ z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
ଅକ୍ଷର *sqlite3_vmprintf(ଅଚଳ ଅକ୍ଷର *zFormat, va_list ap){
  ଅକ୍ଷର *z;
  ଅକ୍ଷର zBase[SQLITE_PRINT_BUF_SIZE];
  StrAccum acc;

#ifdef SQLITE_ENABLE_API_ARMOR  
  ଯଦି( zFormat==0 ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  sqlite3StrAccumInit(&acc, 0, zBase, ଆକାର(zBase), SQLITE_MAX_LENGTH);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  ଫେରନ୍ତୁ z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
ଅକ୍ଷର *sqlite3_mprintf(ଅଚଳ ଅକ୍ଷର *zFormat, ...){
  va_list ap;
  ଅକ୍ଷର *z;
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  ଫେରନ୍ତୁ z;
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
ଅକ୍ଷର *sqlite3_vsnprintf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ଅକ୍ଷର *zBuf, ଅଚଳ ଅକ୍ଷର *zFormat, va_list ap){
  StrAccum acc;
  ଯଦି( n<=0 ) ଫେରନ୍ତୁ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( zBuf==0 || zFormat==0 ) {
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଯଦି( zBuf ) zBuf[0] = 0;
    ଫେରନ୍ତୁ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  ଫେରନ୍ତୁ zBuf;
}
ଅକ୍ଷର *sqlite3_snprintf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ଅକ୍ଷର *zBuf, ଅଚଳ ଅକ୍ଷର *zFormat, ...){
  StrAccum acc;
  va_list ap;
  ଯଦି( n<=0 ) ଫେରନ୍ତୁ zBuf;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( zBuf==0 || zFormat==0 ) {
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଯଦି( zBuf ) zBuf[0] = 0;
    ଫେରନ୍ତୁ zBuf;
  }
#endif
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  ଫେରନ୍ତୁ zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#ଯଦି defined(SQLITE_MAX_LOG_MESSAGE) 
  /* Leave the definition as supplied */
#ନଚେତଯଦି SQLITE_PRINT_BUF_SIZE*10>10000
# define SQLITE_MAX_LOG_MESSAGE 10000
#ଅନ୍ୟଥା
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
ସ୍ଥିର ଶୂନ୍ୟ render_Log_ସନ୍ଦେଶ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା iErrCode, ଅଚଳ ଅକ୍ଷର *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  ଅକ୍ଷର z_ସନ୍ଦେଶ[SQLITE_MAX_LOG_MESSAGE];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_ସନ୍ଦେଶ, ଆକାର(z_ସନ୍ଦେଶ), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, iErrCode,
                           sqlite3StrAccumFinish(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
ଶୂନ୍ୟ sqlite3_log(ପୂର୍ଣ୍ଣସଂଖ୍ୟା iErrCode, ଅଚଳ ଅକ୍ଷର *zFormat, ...){
  va_list ap;                             /* Vararg list */
  ଯଦି( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_ସନ୍ଦେଶ(iErrCode, zFormat, ap);
    va_end(ap);
  }
}

#ଯଦି defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
ଶୂନ୍ୟ sqlite3DebugPrintf(ଅଚଳ ଅକ୍ଷର *zFormat, ...){
  va_list ap;
  StrAccum acc;
  ଅକ୍ଷର zBuf[SQLITE_PRINT_BUF_SIZE*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, ଆକାର(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3StrAccumFinish(&acc);
#ifdef SQLITE_OS_TRACE_PROC
  {
    ବାହ୍ୟ ଶୂନ୍ୟ SQLITE_OS_TRACE_PROC(ଅଚଳ ଅକ୍ଷର *zBuf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, ଆକାର(zBuf));
  }
#ଅନ୍ୟଥା
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#endif
}
#endif


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
ଶୂନ୍ୟ sqlite3_str_appendf(StrAccum *p, ଅଚଳ ଅକ୍ଷର *zFormat, ...){
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
ଅକ୍ଷର *sqlite3RCStrRef(ଅକ୍ଷର *z){
  RCStr *p = (RCStr*)z;
  ପ୍ରତିଜ୍ଞା( p!=0 );
  p--;
  p->nRCRef++;
  ଫେରନ୍ତୁ z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
ଶୂନ୍ୟ sqlite3RCStrUnref(ଶୂନ୍ୟ *z){
  RCStr *p = (RCStr*)z;
  ପ୍ରତିଜ୍ଞା( p!=0 );
  p--;
  ପ୍ରତିଜ୍ଞା( p->nRCRef>0 );
  ଯଦି( p->nRCRef>=2 ){
    p->nRCRef--;
  }ଅନ୍ୟଥା{
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
ଅକ୍ଷର *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + ଆକାର(*p) + 1 );
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  p->nRCRef = 1;
  ଫେରନ୍ତୁ (ଅକ୍ଷର*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
ଅକ୍ଷର *sqlite3RCStrResize(ଅକ୍ଷର *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  ପ୍ରତିଜ୍ଞା( p!=0 );
  p--;
  ପ୍ରତିଜ୍ଞା( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+ଆକାର(RCStr)+1);
  ଯଦି( pNew==0 ){
    sqlite3_free(p);
    ଫେରନ୍ତୁ 0;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ (ଅକ୍ଷର*)&pNew[1];
  }
}
