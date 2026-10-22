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
#అన్తర్భావ "sqliteInt.h"

/*
** Conversion types fall into various categories as defined by the
** following enumeration.
*/
#నిర్వచించు etRADIX       0 /* non-decimal integer types.  %x %o */
#నిర్వచించు etFLOAT       1 /* Floating point.  %f */
#నిర్వచించు etEXP         2 /* Exponentional notation. %e and %E */
#నిర్వచించు etGENERIC     3 /* Floating or exponential, depending on exponent. %g */
#నిర్వచించు et_కొలత        4 /* Return number of characters processed so far. %n */
#నిర్వచించు etSTRING      5 /* Strings. %s */
#నిర్వచించు etDYNSTRING   6 /* Dynamically allocated strings. %z */
#నిర్వచించు etPERCENT     7 /* Percent symbol. %% */
#నిర్వచించు etCHARX       8 /* Characters. %c */
/* The rest are extensions, not normally found in printf() */
#నిర్వచించు etESCAPE_q    9  /* Strings with '\'' doubled.  %q */
#నిర్వచించు etESCAPE_Q    10 /* Strings with '\'' doubled and enclosed in '',
                            NULL pointers replaced by SQL NULL.  %Q */
#నిర్వచించు etTOKEN       11 /* a pointer to a Token structure */
#నిర్వచించు etSRCITEM     12 /* a pointer to a SrcItem */
#నిర్వచించు et_సూచిక     13 /* The %p conversion */
#నిర్వచించు etESCAPE_w    14 /* %w -> Strings with '\"' doubled */
#నిర్వచించు etORDINAL     15 /* %r -> 1st, 2nd, 3rd, 4th, etc.  English only */
#నిర్వచించు etDECIMAL     16 /* %d or %u, but not %x, %o */
#నిర్వచించు etESCAPE_j    17 /* %j -> JSON string literal w/o "..." */
#నిర్వచించు etESCAPE_J    18 /* %J -> JSON string literal with "..." */

#నిర్వచించు etINVALID     19 /* Any unrecognized conversion type */


/*
** An "etByte" is an 8-bit unsigned value.
*/
రకనిర్వచనం గుర్తులేని అక్షరం etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
రకనిర్వచనం నిర్మాణం et_info {   /* Information about each format field */
  అక్షరం fmttype;            /* The format field code letter */
  etByte పునాది;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte ఉపసర్గ;           /* Offset into aPrefix[] of the prefix string */
  అక్షరం iNxt;               /* Next with same hash, or 0 for end of chain */
} et_info;

/*
** Allowed values for et_info.flags
*/
#నిర్వచించు FLAG_SIGNED    1     /* True if the value to convert is signed */
#నిర్వచించు FLAG_STRING    4     /* Allow infinite precision */

/*
** The table is searched by hash.  In the case of %C where C is the character
** and that character has ASCII value j, then the hash is j%25.
**
** The order of the entries in fmtinfo[] and the hash chain was entered
** manually, but based on the output of the following TCL script:
*/
#ఒకవేళ 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  పరిశీలన $c %c x
  సమితి_2 n($c) $x
}
సమితి_2 mx [llength [array పేర్లు n]]
puts "count: $mx"

సమితి_2 mx 25
puts "*********** mx=$mx ************"
కొరకు {సమితి_2 r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array పేర్లు n] {
    ఒకవేళ {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#అయితే_ముగింపు /***** End of script ********/

స్థిర స్థిరరాశి అక్షరం aDigits[] = "0123456789ABCDEF0123456789abcdef";
స్థిర స్థిరరాశి అక్షరం aHex[]    = "0123456789abcdef";
స్థిర స్థిరరాశి అక్షరం a_ఉపసర్గ[] = "-x0\000X0";
స్థిర స్థిరరాశి et_info fmtinfo[25] = {
  /*  0 */  {  'd', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  1 */  {  'e',  0, 1, etEXP,        30, 0,  0 },
  /*  2 */  {  'f',  0, 1, etFLOAT,      0,  0,  0 },
  /*  3 */  {  'g',  0, 1, etGENERIC,    30, 0,  0 },
  /*  4 */  {  'j',  0, 0, etESCAPE_j,   0,  0,  0 },  /* Hash: 6 */
  /*  5 */  {  'i', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  6 */  {  'Q',  0, 4, etESCAPE_Q,   0,  0,  4 },
  /*  7 */  {  'p', 16, 0, et_సూచిక,    0,  1,  0 },  /* Hash: 12 */
  /*  8 */  {  'S',  0, 0, etSRCITEM,    0,  0,  0 },
  /*  9 */  {  'T',  0, 0, etTOKEN,      0,  0,  0 },
  /* 10 */  {  'n',  0, 0, et_కొలత,       0,  0,  0 },
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
శూన్య sqlite3_Str_Accum_సమితి_Error(StrAccum *p, u8 eError){
  ప్రతిజ్ఞా( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  ఒకవేళ( p->mxAlloc ) sqlite3_str_reset(p);
  ఒకవేళ( eError==SQLITE_TOOBIG ) sqlite3_Error_గమ్యం_Parser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
స్థిర sqlite3_int64 getIntArg(PrintfArguments *p){
  ఒకవేళ( p->nArg<=p->n_వాడిన ) తిరిగి 0;
  తిరిగి sqlite3_విలువ_int64(p->apArg[p->n_వాడిన++]);
}
స్థిర ద్విగుణ getDoubleArg(PrintfArguments *p){
  ఒకవేళ( p->nArg<=p->n_వాడిన ) తిరిగి 0.0;
  తిరిగి sqlite3_విలువ_double(p->apArg[p->n_వాడిన++]);
}
స్థిర అక్షరం *get_పాఠ్యం_Arg(PrintfArguments *p){
  ఒకవేళ( p->nArg<=p->n_వాడిన ) తిరిగి 0;
  తిరిగి (అక్షరం*)sqlite3_విలువ_పాఠ్యం(p->apArg[p->n_వాడిన++]);
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
స్థిర అక్షరం *printf_తాత్కాలిక_Buf(sqlite3_str *pAccum, sqlite3_int64 n){
  అక్షరం *z;
  ఒకవేళ( pAccum->accError ) తిరిగి 0;
  ఒకవేళ( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3_Str_Accum_సమితి_Error(pAccum, SQLITE_TOOBIG);
    తిరిగి 0;
  }
  z = sqlite3_malloc(n);
  ఒకవేళ( z==0 ){
    sqlite3_Str_Accum_సమితి_Error(pAccum, SQLITE_NOMEM);
  }
  తిరిగి z;
}

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be something smaller, if desired.
*/
#నిర్వచితం_కాకపోతే SQLITE_PRINT_BUF_కొలత
# define SQLITE_PRINT_BUF_కొలత 70
#అయితే_ముగింపు
#నిర్వచించు etBUFSIZE SQLITE_PRINT_BUF_కొలత  /* Size of the output buffer */

/*
** Hard limit on the precision of floating-point conversions to
** the minimum of SQLITE_PRINTF_PRECISION_LIMIT and 100 million.
*/
#నిర్వచితమైతే SQLITE_PRINTF_PRECISION_హద్దు
# ఒకవేళ SQLITE_PRINTF_PRECISION_హద్దు<100000000
#   define SQLITE_FP_PRECISION_హద్దు SQLITE_PRINTF_PRECISION_హద్దు
# లేకపోతే
#   define SQLITE_FP_PRECISION_హద్దు 100000000
# endif
#లేకపోతే
# define SQLITE_FP_PRECISION_హద్దు 100000000
#అయితే_ముగింపు

/* Forward reference */
స్థిర శూన్య sqlite3StrAppend64(sqlite3_str *p, స్థిరరాశి అక్షరం *z, i64 N);
స్థిర శూన్య sqlite3StrAppendchar64(sqlite3_str *p, i64 N, అక్షరం c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
శూన్య sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  స్థిరరాశి అక్షరం *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  పూర్ణాంకం c;                     /* Next character in the format string */
  అక్షరం *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 నిడివి;                /* Length of the field */
  పూర్ణాంకం idx;                   /* A general purpose loop counter */
  i64 వెడల్పు;                 /* Width of the current field */
  etByte flag_leftjustify;   /* True if "-" flag is present */
  etByte flag_ఉపసర్గ;        /* '+' or ' ' or 0 for prefix */
  etByte flag_alternateform; /* True if "#" flag is present */
  etByte flag_altform2;      /* True if "!" flag is present */
  etByte flag_zeropad;       /* True if field width constant starts with zero */
  etByte flag_long;          /* 1 for the "l" flag, 2 for "ll", 0 by default */
  etByte పూర్తి;               /* Loop termination flag */
  etByte cThousand;          /* Thousands separator for %d and %u */
  etByte xtype = etINVALID;  /* Conversion paradigm */
  u8 bArgList;               /* True for SQLITE_PRINTF_SQLFUNC */
  అక్షరం ఉపసర్గ;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  ద్విగుణ realvalue;          /* Value for real types */
  స్థిరరాశి et_info *infop;      /* Pointer to the appropriate info structure */
  అక్షరం *z_బహిర్గతం;                /* Rendering buffer */
  పూర్ణాంకం n_బహిర్గతం;                  /* Size of the rendering buffer */
  అక్షరం *zExtra = 0;          /* Malloced memory used by some conversion */
  పూర్ణాంకం ఘాతాంకం, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  అక్షరం buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  ప్రతిజ్ఞా( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  ఒకవేళ( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }లేకపోతే{
    bArgList = 0;
  }
  కొరకు(; (c=(*fmt))!=0; ++fmt){
    ఒకవేళ( c!='%' ){
      bufpt = (అక్షరం *)fmt;
#ఒకవేళ HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#లేకపోతే
      fmt = strchr(fmt, '%');
      ఒకవేళ( fmt==0 ){
        fmt = bufpt + అక్షరశ్రేణి_పొడవు(bufpt);
      }
#అయితే_ముగింపు
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      ఒకవేళ( *fmt==0 ) ఆపు;
    }
    ఒకవేళ( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      ఆపు;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_ఉపసర్గ = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    పూర్తి = 0;
    వెడల్పు = 0;
    flag_long = 0;
    precision = -1;
    చేయి{
      ఎంపిక( c ){
        స్థితి '-':   flag_leftjustify = 1;     ఆపు;
        స్థితి '+':   flag_ఉపసర్గ = '+';        ఆపు;
        స్థితి ' ':   flag_ఉపసర్గ = ' ';        ఆపు;
        స్థితి '#':   flag_alternateform = 1;   ఆపు;
        స్థితి '!':   flag_altform2 = 1;        ఆపు;
        స్థితి '0':   flag_zeropad = 1;         ఆపు;
        స్థితి ',':   cThousand = ',';          ఆపు;
        అప్రమేయ:    పూర్తి = 1;                 ఆపు;
        స్థితి 'l': {
          flag_long = 1;
          c = *++fmt;
          ఒకవేళ( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          పూర్తి = 1;
          ఆపు;
        }
        స్థితి '1': స్థితి '2': స్థితి '3': స్థితి '4': స్థితి '5':
        స్థితి '6': స్థితి '7': స్థితి '8': స్థితి '9': {
          గుర్తులేని wx = c - '0';
          అయితే( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          వెడల్పు = wx & 0x7fffffff;
#నిర్వచితమైతే SQLITE_PRINTF_PRECISION_హద్దు
          ఒకవేళ( వెడల్పు>SQLITE_PRINTF_PRECISION_హద్దు ){
            వెడల్పు = SQLITE_PRINTF_PRECISION_హద్దు;
          }
#అయితే_ముగింపు
          ఒకవేళ( c!='.' && c!='l' ){
            పూర్తి = 1;
          }లేకపోతే{
            fmt--;
          }
          ఆపు;
        }
        స్థితి '*': {
          ఒకవేళ( bArgList ){
            వెడల్పు = (పూర్ణాంకం)getIntArg(pArgList);
          }లేకపోతే{
            వెడల్పు = va_arg(ap,పూర్ణాంకం);
          }
          ఒకవేళ( వెడల్పు<0 ){
            flag_leftjustify = 1;
            వెడల్పు = వెడల్పు >= -2147483647 ? -వెడల్పు : 0;
          }
#నిర్వచితమైతే SQLITE_PRINTF_PRECISION_హద్దు
          ఒకవేళ( వెడల్పు>SQLITE_PRINTF_PRECISION_హద్దు ){
            వెడల్పు = SQLITE_PRINTF_PRECISION_హద్దు;
          }
#అయితే_ముగింపు
          ఒకవేళ( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            పూర్తి = 1;
          }
          ఆపు;
        }
        స్థితి '.': {
          c = *++fmt;
          ఒకవేళ( c=='*' ){
            ఒకవేళ( bArgList ){
              precision = (పూర్ణాంకం)getIntArg(pArgList);
            }లేకపోతే{
              precision = va_arg(ap,పూర్ణాంకం);
            }
            ఒకవేళ( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }లేకపోతే{
            గుర్తులేని px = 0;
            అయితే( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#నిర్వచితమైతే SQLITE_PRINTF_PRECISION_హద్దు
          ఒకవేళ( precision>SQLITE_PRINTF_PRECISION_హద్దు ){
            precision = SQLITE_PRINTF_PRECISION_హద్దు;
          }
#అయితే_ముగింపు
          ఒకవేళ( c=='l' ){
            --fmt;
          }లేకపోతే{
            పూర్తి = 1;
          }
          ఆపు;
        }
      }
    }అయితే( !పూర్తి && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#నిర్వచితమైతే SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    కొరకు(idx=0; idx<Array_కొలత(fmtinfo); idx++){
      ఒకవేళ( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        ఆపు;
      }
    }
#లేకపోతే
    /* Fast hash-table lookup */
    ప్రతిజ్ఞా( Array_కొలత(fmtinfo)==25 );
    idx = ((గుర్తులేని)c) % 25;
    ఒకవేళ( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }లేకపోతే{
      infop = &fmtinfo[0];
      xtype = etINVALID;
    }
#అయితే_ముగింపు

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
    ప్రతిజ్ఞా( వెడల్పు>=0 );
    ప్రతిజ్ఞా( precision>=(-1) );
    ఎంపిక( xtype ){
      స్థితి et_సూచిక:
        flag_long = పరిమాణం(అక్షరం*)==పరిమాణం(i64) ? 2 :
                     పరిమాణం(అక్షరం*)==పరిమాణం(దీర్ఘ పూర్ణాంకం) ? 1 : 0;
        /* no break */ deliberate_fall_through
      స్థితి etORDINAL:
      స్థితి etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      స్థితి etDECIMAL:
        ఒకవేళ( infop->flags & FLAG_SIGNED ){
          i64 v;
          ఒకవేళ( bArgList ){
            v = getIntArg(pArgList);
          }లేకపోతే ఒకవేళ( flag_long ){
            ఒకవేళ( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }లేకపోతే{
              v = va_arg(ap,దీర్ఘ పూర్ణాంకం);
            }
          }లేకపోతే{
            v = va_arg(ap,పూర్ణాంకం);
          }
          ఒకవేళ( v<0 ){
            testcase( v==అతిచిన్న_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            ఉపసర్గ = '-';
          }లేకపోతే{
            longvalue = v;
            ఉపసర్గ = flag_ఉపసర్గ;
          }
        }లేకపోతే{
          ఒకవేళ( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }లేకపోతే ఒకవేళ( flag_long ){
            ఒకవేళ( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }లేకపోతే{
              longvalue = va_arg(ap,గుర్తులేని దీర్ఘ పూర్ణాంకం);
            }
          }లేకపోతే{
            longvalue = va_arg(ap,గుర్తులేని పూర్ణాంకం);
          }
          ఉపసర్గ = 0;
        }

#ఒకవేళ WHERETRACE_ENABLED
        ఒకవేళ( xtype==et_సూచిక && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#అయితే_ముగింపు
#ఒకవేళ TREETRACE_ENABLED
        ఒకవేళ( xtype==et_సూచిక && sqlite3_వృక్షం_Trace & 0x100000 ) longvalue = 0;
#అయితే_ముగింపు

        ఒకవేళ( longvalue==0 ) flag_alternateform = 0;
        ఒకవేళ( flag_zeropad && precision<వెడల్పు-(ఉపసర్గ!=0) ){
          precision = వెడల్పు-(ఉపసర్గ!=0);
        }
        ఒకవేళ( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          n_బహిర్గతం = etBUFSIZE;
          z_బహిర్గతం = buf;
        }లేకపోతే{
          u64 n;
          n = (u64)precision + 10;
          ఒకవేళ( cThousand ) n += precision/3;
          z_బహిర్గతం = zExtra = printf_తాత్కాలిక_Buf(pAccum, n);
          ఒకవేళ( z_బహిర్గతం==0 ) తిరిగి;
          n_బహిర్గతం = (పూర్ణాంకం)n;
        }
        bufpt = &z_బహిర్గతం[n_బహిర్గతం-1];
        ఒకవేళ( xtype==etORDINAL ){
          స్థిర స్థిరరాశి అక్షరం zOrd[] = "thstndrd";
          పూర్ణాంకం x = (పూర్ణాంకం)(longvalue % 10);
          ఒకవేళ( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          స్థిరరాశి అక్షరం *cset = &aDigits[infop->charset];
          u8 పునాది = infop->పునాది;
          చేయి{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%పునాది];
            longvalue = longvalue/పునాది;
          }అయితే( longvalue>0 );
        }
        నిడివి = &z_బహిర్గతం[n_బహిర్గతం-1] - bufpt;
        ఒకవేళ( precision>నిడివి ){                         /* zero pad */
          i64 nn = precision-నిడివి;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          నిడివి = precision;
        }
        ఒకవేళ( cThousand ){
          i64 nn = (నిడివి - 1)/3;  /* Number of "," to insert */
          i64 ix = (నిడివి - 1)%3 + 1;
          పూర్ణాంకం ii;
          bufpt -= nn;
          కొరకు(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            ఒకవేళ( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        ఒకవేళ( ఉపసర్గ ) *(--bufpt) = ఉపసర్గ;               /* Add sign */
        ఒకవేళ( flag_alternateform && infop->ఉపసర్గ ){      /* Add "0" or "0x" */
          స్థిరరాశి అక్షరం *pre;
          అక్షరం x;
          pre = &a_ఉపసర్గ[infop->ఉపసర్గ];
          కొరకు(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        నిడివి = &z_బహిర్గతం[n_బహిర్గతం-1] - bufpt;
        ఆపు;
      స్థితి etFLOAT:
      స్థితి etEXP:
      స్థితి etGENERIC: {
        FpDecode s;
        పూర్ణాంకం i_విడత;
        పూర్ణాంకం j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        ఒకవేళ( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }లేకపోతే{
          realvalue = va_arg(ap,ద్విగుణ);
        }
        ఒకవేళ( precision<0 ) precision = 6;         /* Set default precision */
        ఒకవేళ( precision>SQLITE_FP_PRECISION_హద్దు ){
          precision = SQLITE_FP_PRECISION_హద్దు;
        }
        ఒకవేళ( xtype==etFLOAT ){
          i_విడత = -precision;
        }లేకపోతే ఒకవేళ( xtype==etGENERIC ){
          ఒకవేళ( precision==0 ) precision = 1;
          i_విడత = precision;
        }లేకపోతే{
          i_విడత = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, i_విడత, flag_altform2 ? 20 : 16);
        ఒకవేళ( s.isSpecial ){
          ఒకవేళ( s.isSpecial==2 ){
            ఒకవేళ( flag_zeropad ){
              bufpt = "null";
              నిడివి = 4;
            }లేకపోతే{
              bufpt = "NaN";
              నిడివి = 3;
            }
            ఆపు;
          }లేకపోతే ఒకవేళ( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }లేకపోతే{
            జ్ఞాపకం_నకలు(buf, "-Inf", 5);
            bufpt = buf;
            ఒకవేళ( s.sign=='-' ){
              /* no-op */
            }లేకపోతే ఒకవేళ( flag_ఉపసర్గ ){
              buf[0] = flag_ఉపసర్గ;
            }లేకపోతే{
              bufpt++;
            }
            నిడివి = అక్షరశ్రేణి_పొడవు(bufpt);
            ఆపు;
          }
        }
        ఒకవేళ( s.sign=='-' ){
          ఒకవేళ( flag_alternateform
           && !flag_ఉపసర్గ
           && xtype==etFLOAT
           && s.iDP<=i_విడత
          ){
            /* Suppress the minus sign if all of the following are true:
            **   *  The value displayed is zero
            **   *  The '#' flag is used
            **   *  The '+' flag is not used, and
            **   *  The format is %f
            */
            ఉపసర్గ = 0;
          }లేకపోతే{
            ఉపసర్గ = '-';
          }
        }లేకపోతే{
          ఉపసర్గ = flag_ఉపసర్గ;
        }

        ఘాతాంకం = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        ఒకవేళ( xtype==etGENERIC ){
          ప్రతిజ్ఞా( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          ఒకవేళ( ఘాతాంకం<-4 || ఘాతాంకం>precision ){
            xtype = etEXP;
          }లేకపోతే{
            precision = precision - ఘాతాంకం;
            xtype = etFLOAT;
          }
        }లేకపోతే{
          flag_rtz = flag_altform2;
        }
        ఒకవేళ( xtype==etEXP ){
          e2 = 0;
        }లేకపోతే{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)వెడల్పు+10;
        ఒకవేళ( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        ఒకవేళ( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          ఒకవేళ( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            ఒకవేళ( bufpt==0 ){
              sqlite3_Str_Accum_సమితి_Error(pAccum, SQLITE_NOMEM);
              తిరిగి;
            }
            zExtra = bufpt;
          }లేకపోతే ఒకవేళ( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            వెడల్పు = నిడివి = 0;
            ఆపు;
          }లేకపోతే{
            bufpt = pAccum->z_పాఠ్యం + pAccum->nChar;
          }
        }లేకపోతే{
          bufpt = pAccum->z_పాఠ్యం + pAccum->nChar;
        }
        z_బహిర్గతం = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        ఒకవేళ( ఉపసర్గ ){
          *(bufpt++) = ఉపసర్గ;
        }
        /* Digits prior to the decimal point */
        j = 0;
        ప్రతిజ్ఞా( s.n>0 );
        ఒకవేళ( e2<0 ){
          *(bufpt++) = '0';
        }లేకపోతే ఒకవేళ( cThousand ){
          కొరకు(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            ఒకవేళ( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }లేకపోతే{
          j = e2+1;
          ఒకవేళ( j>s.n ) j = s.n;
          జ్ఞాపకం_నకలు(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          ఒకవేళ( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        ఒకవేళ( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        ఒకవేళ( e2<(-1) && precision>0 ){
          పూర్ణాంకం nn = -1-e2;
          ఒకవేళ( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        ఒకవేళ( precision>0 ){
          పూర్ణాంకం nn = s.n - j;
          ఒకవేళ( NEVER(nn>precision) ) nn = precision;
          ఒకవేళ( nn>0 ){
            జ్ఞాపకం_నకలు(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          ఒకవేళ( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        ఒకవేళ( flag_rtz && flag_dp ){
          అయితే( bufpt[-1]=='0' ) *(--bufpt) = 0;
          ప్రతిజ్ఞా( bufpt>z_బహిర్గతం );
          ఒకవేళ( bufpt[-1]=='.' ){
            ఒకవేళ( flag_altform2 ){
              *(bufpt++) = '0';
            }లేకపోతే{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        ఒకవేళ( xtype==etEXP ){
          ఘాతాంకం = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          ఒకవేళ( ఘాతాంకం<0 ){
            *(bufpt++) = '-'; ఘాతాంకం = -ఘాతాంకం;
          }లేకపోతే{
            *(bufpt++) = '+';
          }
          ఒకవేళ( ఘాతాంకం>=100 ){
            *(bufpt++) = (అక్షరం)((ఘాతాంకం/100)+'0');        /* 100's digit */
            ఘాతాంకం %= 100;
          }
          *(bufpt++) = (అక్షరం)(ఘాతాంకం/10+'0');             /* 10's digit */
          *(bufpt++) = (అక్షరం)(ఘాతాంకం%10+'0');             /* 1's digit */
        }

        నిడివి = bufpt - z_బహిర్గతం;
        ప్రతిజ్ఞా( నిడివి <= szBufNeeded );
        ఒకవేళ( నిడివి<వెడల్పు ){
          i64 nPad = వెడల్పు - నిడివి;
          ఒకవేళ( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }లేకపోతే ఒకవేళ( !flag_zeropad ){
            memmove(z_బహిర్గతం+nPad, z_బహిర్గతం, నిడివి);
            memset(z_బహిర్గతం, ' ', nPad);
          }లేకపోతే{
            పూర్ణాంకం adj = ఉపసర్గ!=0;
            memmove(z_బహిర్గతం+nPad+adj, z_బహిర్గతం+adj, నిడివి-adj);
            memset(z_బహిర్గతం+adj, '0', nPad);
          }
          నిడివి = వెడల్పు;
        }

        ఒకవేళ( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          ప్రతిజ్ఞా( pAccum->nChar + నిడివి < SMXV(pAccum->nChar) );
          pAccum->nChar += నిడివి;
          z_బహిర్గతం[నిడివి] = 0;
          కొనసాగు;
        }లేకపోతే{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          ఆపు;
        }
      }
      స్థితి et_కొలత:
        ఒకవేళ( !bArgList ){
          *(va_arg(ap,పూర్ణాంకం*)) = pAccum->nChar;
        }
        నిడివి = వెడల్పు = 0;
        ఆపు;
      స్థితి etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        నిడివి = 1;
        ఆపు;
      స్థితి etCHARX:
        ఒకవేళ( bArgList ){
          bufpt = get_పాఠ్యం_Arg(pArgList);
          నిడివి = 1;
          ఒకవేళ( bufpt ){
            buf[0] = c = *(bufpt++);
            ఒకవేళ( (c&0xc0)==0xc0 ){
              అయితే( నిడివి<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[నిడివి++] = *(bufpt++);
              }
            }
          }లేకపోతే{
            buf[0] = 0;
          }
        }లేకపోతే{
          గుర్తులేని పూర్ణాంకం ch = va_arg(ap,గుర్తులేని పూర్ణాంకం);
          నిడివి = sqlite3AppendOneUtf8Character(buf, ch);
        }
        ఒకవేళ( precision>1 ){
          i64 n_పూర్వ = 1;
          వెడల్పు -= precision-1;
          ఒకవేళ( వెడల్పు>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, వెడల్పు-1, ' ');
            వెడల్పు = 0;
          }
          sqlite3StrAppend64(pAccum, buf, నిడివి);
          precision--;
          అయితే( precision > 1 ){
            i64 n_Copy_బైట్లు;
            ఒకవేళ( n_పూర్వ > precision-1 ) n_పూర్వ = precision - 1;
            n_Copy_బైట్లు = నిడివి*n_పూర్వ;
            ఒకవేళ( sqlite3StrAccumEnlargeIfNeeded(pAccum, n_Copy_బైట్లు) ){
              ఆపు;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->z_పాఠ్యం[pAccum->nChar-n_Copy_బైట్లు], n_Copy_బైట్లు);
            precision -= n_పూర్వ;
            n_పూర్వ *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        వెళ్ళు adjust_వెడల్పు_for_utf8;
      స్థితి etSTRING:
      స్థితి etDYNSTRING:
        ఒకవేళ( bArgList ){
          bufpt = get_పాఠ్యం_Arg(pArgList);
          xtype = etSTRING;
        }లేకపోతే{
          bufpt = va_arg(ap,అక్షరం*);
        }
        ఒకవేళ( bufpt==0 ){
          bufpt = "";
        }లేకపోతే ఒకవేళ( xtype==etDYNSTRING ){
          ఒకవేళ( pAccum->nChar==0
           && pAccum->mxAlloc
           && వెడల్పు==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            ప్రతిజ్ఞా( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->z_పాఠ్యం = bufpt;
            pAccum->nAlloc = sqlite3_Db_Malloc_కొలత(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            నిడివి = 0;
            ఆపు;
          }
          zExtra = bufpt;
        }
        ఒకవేళ( precision>=0 ){
          ఒకవేళ( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            గుర్తులేని అక్షరం *z = (గుర్తులేని అక్షరం*)bufpt;
            అయితే( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            నిడివి = z - (గుర్తులేని అక్షరం*)bufpt;
          }లేకపోతే{
            కొరకు(నిడివి=0; నిడివి<precision && bufpt[నిడివి]; నిడివి++){}
          }
        }లేకపోతే{
          నిడివి = అక్షరశ్రేణి_పొడవు(bufpt);
        }
      adjust_వెడల్పు_for_utf8:
        ఒకవేళ( flag_altform2 && వెడల్పు>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = నిడివి - 1;
          అయితే( ii>=0 ) ఒకవేళ( (bufpt[ii--] & 0xc0)==0x80 ) వెడల్పు++;
        }
        ఆపు;
      స్థితి etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      స్థితి etESCAPE_J: {         /* %J: Generate a JSON string literal */
        అక్షరం *escarg;
        i64 i, j, px, i_ప్రారంభం;
        గుర్తులేని అక్షరం ch;

        ఒకవేళ( bArgList ){
          escarg = get_పాఠ్యం_Arg(pArgList);
        }లేకపోతే{
          escarg = va_arg(ap,అక్షరం*);
        }
        i_ప్రారంభం = sqlite3_str_నిడివి(pAccum);
        ఒకవేళ( escarg==0 ){
          ఒకవేళ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }లేకపోతే{
          ఒకవేళ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          ఒకవేళ( px<0 ){
            px = 0x7fffffff;
          }లేకపోతే ఒకవేళ( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            కొరకు(i=0; i<px && escarg[i]; i++){
              ఒకవేళ( (escarg[i]&0xc0)==0x80 ) px++;
            }
            ఒకవేళ( i==px ){
              అయితే( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          కొరకు(i=j=0; i<px; i++){
            ఒకవేళ( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              ఒకవేళ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              ఒకవేళ( ch==0 ) ఆపు;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              ఒకవేళ( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }లేకపోతే ఒకవేళ( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }లేకపోతే{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          ఒకవేళ( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          ఒకవేళ( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        ఒకవేళ( వెడల్పు>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_నిడివి(pAccum) - i_ప్రారంభం;
          sqlite3_int64 len = n;
          అక్షరం *zz;
          ఒకవేళ( flag_altform2 && n>0 ){
            zz = sqlite3_str_విలువ(pAccum);
            కొరకు(i=i_ప్రారంభం; zz[i]; i++){
              ఒకవేళ( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          ఒకవేళ( వెడల్పు>len ){
            sqlite3_int64 sp = వెడల్పు-len;
            ప్రతిజ్ఞా( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (పూర్ణాంకం)sp, ' ');
            ఒకవేళ( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_విలువ(pAccum);
              zz += i_ప్రారంభం;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        కొనసాగు;
      }
      స్థితి etESCAPE_q:          /* %q: Escape ' characters */
      స్థితి etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      స్థితి etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        పూర్ణాంకం needQuote = 0;
        అక్షరం ch;
        అక్షరం *escarg;
        అక్షరం q;

        ఒకవేళ( bArgList ){
          escarg = get_పాఠ్యం_Arg(pArgList);
        }లేకపోతే{
          escarg = va_arg(ap,అక్షరం*);
        }
        ఒకవేళ( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }లేకపోతే ఒకవేళ( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        ఒకవేళ( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }లేకపోతే{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        కొరకు(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          ఒకవేళ( ch==q )  n++;
          ఒకవేళ( flag_altform2 && (ch&0xc0)==0xc0 ){
            అయితే( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        ఒకవేళ( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 n_వెనుక = 0;
          i64 nCtrl = 0;
          కొరకు(k=0; k<i; k++){
            ఒకవేళ( escarg[k]=='\\' ){
              n_వెనుక++;
            }లేకపోతే ఒకవేళ( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          ఒకవేళ( nCtrl || xtype==etESCAPE_q ){
            n += n_వెనుక + 5*nCtrl;
            ఒకవేళ( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }లేకపోతే{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        ఒకవేళ( n>etBUFSIZE ){
          bufpt = zExtra = printf_తాత్కాలిక_Buf(pAccum, n);
          ఒకవేళ( bufpt==0 ) తిరిగి;
        }లేకపోతే{
          bufpt = buf;
        }
        j = 0;
        ఒకవేళ( needQuote ){
          ఒకవేళ( needQuote==2 ){
            జ్ఞాపకం_నకలు(&bufpt[j], "unistr('", 8);
            j += 8;
          }లేకపోతే{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        ఒకవేళ( flag_alternateform ){
          కొరకు(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ఒకవేళ( ch==q ){
              bufpt[j++] = ch;
            }లేకపోతే ఒకవేళ( ch=='\\' ){
              bufpt[j++] = '\\';
            }లేకపోతే ఒకవేళ( ((గుర్తులేని అక్షరం)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }లేకపోతే{
          కొరకు(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            ఒకవేళ( ch==q ) bufpt[j++] = ch;
          }
        }
        ఒకవేళ( needQuote ){
          bufpt[j++] = '\'';
          ఒకవేళ( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        నిడివి = j;
        వెళ్ళు adjust_వెడల్పు_for_utf8;
      }
      స్థితి etTOKEN: {
        ఒకవేళ( (pAccum->printfFlags & SQLITE_PRINTF_అంతర్గత)==0 ) తిరిగి;
        ఒకవేళ( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          సమాసం *p_సమాసం = va_arg(ap,సమాసం*);
          ఒకవేళ( ALWAYS(p_సమాసం) && ALWAYS(!సమాసం_Has_Property(p_సమాసం,EP_Int_విలువ)) ){
            sqlite3_str_appendall(pAccum, (స్థిరరాశి అక్షరం*)p_సమాసం->u.zToken);
            sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(pAccum->db, p_సమాసం);
          }
        }లేకపోతే{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          ప్రతిజ్ఞా( bArgList==0 );
          ఒకవేళ( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (స్థిరరాశి అక్షరం*)pToken->z, pToken->n);
            sqlite3_రికార్డు_Error_Byte_స్థానభ్రంశం(pAccum->db, pToken->z);
          }
        }
        నిడివి = వెడల్పు = 0;
        ఆపు;
      }
      స్థితి etSRCITEM: {
        Src_అంశం *p_అంశం;
        ఒకవేళ( (pAccum->printfFlags & SQLITE_PRINTF_అంతర్గత)==0 ) తిరిగి;
        p_అంశం = va_arg(ap, Src_అంశం*);
        ప్రతిజ్ఞా( bArgList==0 );
        ఒకవేళ( p_అంశం->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, p_అంశం->zAlias);
        }లేకపోతే ఒకవేళ( p_అంశం->zName ){
          ఒకవేళ( p_అంశం->fg.fixedSchema==0
           && p_అంశం->fg.isSubquery==0
           && p_అంశం->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, p_అంశం->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, p_అంశం->zName);
        }లేకపోతే ఒకవేళ( p_అంశం->zAlias ){
          sqlite3_str_appendall(pAccum, p_అంశం->zAlias);
        }లేకపోతే ఒకవేళ( ALWAYS(p_అంశం->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = p_అంశం->u4.pSubq->pSelect;
          ప్రతిజ్ఞా( pSel!=0 ); 
          ఒకవేళ( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }లేకపోతే ఒకవేళ( pSel->selFlags & SF_Multi_విలువ ){
            ప్రతిజ్ఞా( !p_అంశం->fg.isTabFunc && !p_అంశం->fg.is_సూచీకృత_By );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                p_అంశం->u1.n_అడ్డువరుస);
          }లేకపోతే{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        నిడివి = వెడల్పు = 0;
        ఆపు;
      }
      అప్రమేయ: {
        ప్రతిజ్ఞా( xtype==etINVALID );
        తిరిగి;
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
    వెడల్పు -= నిడివి;
    ఒకవేళ( వెడల్పు>0 ){
      ఒకవేళ( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, వెడల్పు, ' ');
      sqlite3StrAppend64(pAccum, bufpt, నిడివి);
      ఒకవేళ( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, వెడల్పు, ' ');
    }లేకపోతే{
      sqlite3StrAppend64(pAccum, bufpt, నిడివి);
    }

    ఒకవేళ( zExtra ){
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
శూన్య sqlite3_రికార్డు_Error_Byte_స్థానభ్రంశం(sqlite3 *db, స్థిరరాశి అక్షరం *z){
  స్థిరరాశి Parse *pParse;
  స్థిరరాశి అక్షరం *z_పాఠ్యం;
  స్థిరరాశి అక్షరం *z_చివర;
  ప్రతిజ్ఞా( z!=0 );
  ఒకవేళ( NEVER(db==0) ) తిరిగి;
  ఒకవేళ( db->err_Byte_స్థానభ్రంశం!=(-2) ) తిరిగి;
  pParse = db->pParse;
  ఒకవేళ( NEVER(pParse==0) ) తిరిగి;
  z_పాఠ్యం =pParse->z_తోక;
  ఒకవేళ( NEVER(z_పాఠ్యం==0) ) తిరిగి;
  z_చివర = &z_పాఠ్యం[అక్షరశ్రేణి_పొడవు(z_పాఠ్యం)];
  ఒకవేళ( SQLITE_WITHIN(z,z_పాఠ్యం,z_చివర) ){
    db->err_Byte_స్థానభ్రంశం = (పూర్ణాంకం)(z-z_పాఠ్యం);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
శూన్య sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(sqlite3 *db, స్థిరరాశి సమాసం *p_సమాసం){
  అయితే( p_సమాసం ){
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Reduced|EP_TokenOnly) ) తిరిగి;
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం,EP_OuterON|EP_InnerON) || p_సమాసం->w.iOfst<=0 ){
      p_సమాసం = p_సమాసం->p_ఎడమ;
    }లేకపోతే{
      ఆపు;
    }
  }
  ఒకవేళ( p_సమాసం==0 ) తిరిగి;
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_FromDDL) ) తిరిగి;
  db->err_Byte_స్థానభ్రంశం = p_సమాసం->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
పూర్ణాంకం sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  అక్షరం *zNew;
  ప్రతిజ్ఞా( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  ఒకవేళ( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    తిరిగి 0;
  }
  ఒకవేళ( p->mxAlloc==0 ){
    sqlite3_Str_Accum_సమితి_Error(p, SQLITE_TOOBIG);
    తిరిగి p->nAlloc - p->nChar - 1;
  }లేకపోతే{
    అక్షరం *zOld = isMalloced(p) ? p->z_పాఠ్యం : 0;
    i64 szNew = p->nChar + N + 1;
    ఒకవేళ( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    ఒకవేళ( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_సమితి_Error(p, SQLITE_TOOBIG);
      తిరిగి 0;
    }లేకపోతే{
      p->nAlloc = (పూర్ణాంకం)szNew;
    }
    ఒకవేళ( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }లేకపోతే{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    ఒకవేళ( zNew ){
      ప్రతిజ్ఞా( p->z_పాఠ్యం!=0 || p->nChar==0 );
      ఒకవేళ( !isMalloced(p) && p->nChar>0 ) జ్ఞాపకం_నకలు(zNew, p->z_పాఠ్యం, p->nChar);
      p->z_పాఠ్యం = zNew;
      p->nAlloc = sqlite3_Db_Malloc_కొలత(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }లేకపోతే{
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_సమితి_Error(p, SQLITE_NOMEM);
      తిరిగి 0;
    }
  }
  ప్రతిజ్ఞా( N>=0 && N<=0x7fffffff );
  తిరిగి (పూర్ణాంకం)N;
}

పూర్ణాంకం sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  ఒకవేళ( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  తిరిగి p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
శూన్య sqlite3_str_appendchar(sqlite3_str *p, పూర్ణాంకం N, అక్షరం c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  ఒకవేళ( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    తిరిగి;
  }
  అయితే( (N--)>0 ) p->z_పాఠ్యం[p->nChar++] = c;
}
స్థిర శూన్య sqlite3StrAppendchar64(sqlite3_str *p, i64 N, అక్షరం c){
  testcase( p->nChar + N > 0x7fffffff );
  ఒకవేళ( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    తిరిగి;
  }
  అయితే( (N--)>0 ) p->z_పాఠ్యం[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
స్థిర శూన్య SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, స్థిరరాశి అక్షరం *z, పూర్ణాంకం N){
  N = sqlite3StrAccumEnlarge(p, N);
  ప్రతిజ్ఞా( z!=0 || N==0 );
  ఒకవేళ( N>0 ){
    జ్ఞాపకం_నకలు(&p->z_పాఠ్యం[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
శూన్య sqlite3_str_append(sqlite3_str *p, స్థిరరాశి అక్షరం *z, పూర్ణాంకం N){
  ప్రతిజ్ఞా( z!=0 || N==0 );
  ప్రతిజ్ఞా( p->z_పాఠ్యం!=0 || p->nChar==0 || p->accError );
  ప్రతిజ్ఞా( N>=0 );
  ప్రతిజ్ఞా( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ఒకవేళ( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }లేకపోతే ఒకవేళ( N ){
    ప్రతిజ్ఞా( p->z_పాఠ్యం );
    p->nChar += N;
    జ్ఞాపకం_నకలు(&p->z_పాఠ్యం[p->nChar-N], z, N);
  }
}
స్థిర శూన్య sqlite3StrAppend64(sqlite3_str *p, స్థిరరాశి అక్షరం *z, i64 N){
  ప్రతిజ్ఞా( z!=0 || N==0 );
  ప్రతిజ్ఞా( p->z_పాఠ్యం!=0 || p->nChar==0 || p->accError );
  ప్రతిజ్ఞా( N>=0 );
  ప్రతిజ్ఞా( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  ఒకవేళ( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }లేకపోతే ఒకవేళ( N ){
    ప్రతిజ్ఞా( p->z_పాఠ్యం );
    p->nChar += N;
    జ్ఞాపకం_నకలు(&p->z_పాఠ్యం[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
శూన్య sqlite3_str_appendall(sqlite3_str *p, స్థిరరాశి అక్షరం *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
స్థిర SQLITE_NOINLINE అక్షరం *str_Accum_ముగింపు_Realloc(StrAccum *p){
  అక్షరం *z_పాఠ్యం;
  ప్రతిజ్ఞా( p->mxAlloc>0 && !isMalloced(p) );
  z_పాఠ్యం = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  ఒకవేళ( z_పాఠ్యం ){
    జ్ఞాపకం_నకలు(z_పాఠ్యం, p->z_పాఠ్యం, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }లేకపోతే{
    sqlite3_Str_Accum_సమితి_Error(p, SQLITE_NOMEM);
  }
  p->z_పాఠ్యం = z_పాఠ్యం;
  తిరిగి z_పాఠ్యం;
}
అక్షరం *sqlite3_Str_Accum_ముగింపు(StrAccum *p){
  ఒకవేళ( p->z_పాఠ్యం ){
    p->z_పాఠ్యం[p->nChar] = 0;
    ఒకవేళ( p->mxAlloc>0 && !isMalloced(p) ){
      తిరిగి str_Accum_ముగింపు_Realloc(p);
    }
  }
  తిరిగి p->z_పాఠ్యం;
}

/* Finalize a string created using sqlite3_str_new().
*/
అక్షరం *sqlite3_str_ముగింపు(sqlite3_str *p){
  అక్షరం *z;
  ఒకవేళ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3_Str_Accum_ముగింపు(p);
    sqlite3_free(p);
  }లేకపోతే{
    z = 0;
  }
  తిరిగి z;
}

/* Return any error code associated with p */
పూర్ణాంకం sqlite3_str_errcode(sqlite3_str *p){
  తిరిగి p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
పూర్ణాంకం sqlite3_str_నిడివి(sqlite3_str *p){
  తిరిగి p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
శూన్య sqlite3_str_truncate(sqlite3_str *p, పూర్ణాంకం N){
  ఒకవేళ( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->z_పాఠ్యం[p->nChar] = 0;
  }
}

/* Return the current value for p */
అక్షరం *sqlite3_str_విలువ(sqlite3_str *p){
  ఒకవేళ( p==0 || p->nChar==0 ) తిరిగి 0;
  p->z_పాఠ్యం[p->nChar] = 0;
  తిరిగి p->z_పాఠ్యం;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
శూన్య sqlite3_str_reset(StrAccum *p){
  ఒకవేళ( isMalloced(p) ){
    sqlite3DbFree(p->db, p->z_పాఠ్యం);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }లేకపోతే ఒకవేళ( p==(sqlite3_str*)&sqlite3OomStr ){
    తిరిగి;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->z_పాఠ్యం = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
శూన్య sqlite3_str_free(sqlite3_str *p){
  ఒకవేళ( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
శూన్య sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, అక్షరం *z_పునాది, పూర్ణాంకం n, పూర్ణాంకం mx){
  p->z_పాఠ్యం = z_పునాది;
  p->db = db;
  p->nAlloc = n;
  p->mxAlloc = mx;
  p->nChar = 0;
  p->accError = 0;
  p->printfFlags = 0;
}

/* Allocate and initialize a new dynamic string object */
sqlite3_str *sqlite3_str_new(sqlite3 *db){
  sqlite3_str *p = sqlite3_malloc64(పరిమాణం(*p));
  ఒకవేళ( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->a_హద్దు[SQLITE_హద్దు_నిడివి] : SQLITE_MAX_నిడివి);
  }లేకపోతే{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  తిరిగి p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
అక్షరం *sqlite3VMPrintf(sqlite3 *db, స్థిరరాశి అక్షరం *zFormat, va_list ap){
  అక్షరం *z;
  అక్షరం z_పునాది[SQLITE_PRINT_BUF_కొలత];
  StrAccum acc;
  ప్రతిజ్ఞా( db!=0 );
  sqlite3StrAccumInit(&acc, db, z_పునాది, పరిమాణం(z_పునాది),
                      db->a_హద్దు[SQLITE_హద్దు_నిడివి]);
  acc.printfFlags = SQLITE_PRINTF_అంతర్గత;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_ముగింపు(&acc);
  ఒకవేళ( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  తిరిగి z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
అక్షరం *sqlite3MPrintf(sqlite3 *db, స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;
  అక్షరం *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  తిరిగి z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
అక్షరం *sqlite3_vmprintf(స్థిరరాశి అక్షరం *zFormat, va_list ap){
  అక్షరం *z;
  అక్షరం z_పునాది[SQLITE_PRINT_BUF_కొలత];
  StrAccum acc;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR  
  ఒకవేళ( zFormat==0 ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  sqlite3StrAccumInit(&acc, 0, z_పునాది, పరిమాణం(z_పునాది), SQLITE_MAX_నిడివి);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_ముగింపు(&acc);
  తిరిగి z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
అక్షరం *sqlite3_mprintf(స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;
  అక్షరం *z;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  తిరిగి z;
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
అక్షరం *sqlite3_vsnprintf(పూర్ణాంకం n, అక్షరం *zBuf, స్థిరరాశి అక్షరం *zFormat, va_list ap){
  StrAccum acc;
  ఒకవేళ( n<=0 ) తిరిగి zBuf;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( zBuf==0 || zFormat==0 ) {
    (శూన్య)SQLITE_MISUSE_BKPT;
    ఒకవేళ( zBuf ) zBuf[0] = 0;
    తిరిగి zBuf;
  }
#అయితే_ముగింపు
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  తిరిగి zBuf;
}
అక్షరం *sqlite3_snprintf(పూర్ణాంకం n, అక్షరం *zBuf, స్థిరరాశి అక్షరం *zFormat, ...){
  StrAccum acc;
  va_list ap;
  ఒకవేళ( n<=0 ) తిరిగి zBuf;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( zBuf==0 || zFormat==0 ) {
    (శూన్య)SQLITE_MISUSE_BKPT;
    ఒకవేళ( zBuf ) zBuf[0] = 0;
    తిరిగి zBuf;
  }
#అయితే_ముగింపు
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  తిరిగి zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#ఒకవేళ defined(SQLITE_MAX_LOG_సందేశం) 
  /* Leave the definition as supplied */
#లేదావేళ SQLITE_PRINT_BUF_కొలత*10>10000
# define SQLITE_MAX_LOG_సందేశం 10000
#లేకపోతే
# define SQLITE_MAX_LOG_సందేశం (SQLITE_PRINT_BUF_కొలత*10)
#అయితే_ముగింపు

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
స్థిర శూన్య render_Log_సందేశం(పూర్ణాంకం i_Err_సంకేతం, స్థిరరాశి అక్షరం *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  అక్షరం z_సందేశం[SQLITE_MAX_LOG_సందేశం];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_సందేశం, పరిమాణం(z_సందేశం), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, i_Err_సంకేతం,
                           sqlite3_Str_Accum_ముగింపు(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
శూన్య sqlite3_log(పూర్ణాంకం i_Err_సంకేతం, స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;                             /* Vararg list */
  ఒకవేళ( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_సందేశం(i_Err_సంకేతం, zFormat, ap);
    va_end(ap);
  }
}

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
శూన్య sqlite3DebugPrintf(స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;
  StrAccum acc;
  అక్షరం zBuf[SQLITE_PRINT_BUF_కొలత*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, పరిమాణం(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3_Str_Accum_ముగింపు(&acc);
#నిర్వచితమైతే SQLITE_OS_TRACE_PROC
  {
    బాహ్య శూన్య SQLITE_OS_TRACE_PROC(స్థిరరాశి అక్షరం *zBuf, పూర్ణాంకం nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, పరిమాణం(zBuf));
  }
#లేకపోతే
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#అయితే_ముగింపు
}
#అయితే_ముగింపు


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
శూన్య sqlite3_str_appendf(StrAccum *p, స్థిరరాశి అక్షరం *zFormat, ...){
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
అక్షరం *sqlite3RCStrRef(అక్షరం *z){
  RCStr *p = (RCStr*)z;
  ప్రతిజ్ఞా( p!=0 );
  p--;
  p->nRCRef++;
  తిరిగి z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
శూన్య sqlite3RCStrUnref(శూన్య *z){
  RCStr *p = (RCStr*)z;
  ప్రతిజ్ఞా( p!=0 );
  p--;
  ప్రతిజ్ఞా( p->nRCRef>0 );
  ఒకవేళ( p->nRCRef>=2 ){
    p->nRCRef--;
  }లేకపోతే{
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
అక్షరం *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + పరిమాణం(*p) + 1 );
  ఒకవేళ( p==0 ) తిరిగి 0;
  p->nRCRef = 1;
  తిరిగి (అక్షరం*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
అక్షరం *sqlite3RCStrResize(అక్షరం *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  ప్రతిజ్ఞా( p!=0 );
  p--;
  ప్రతిజ్ఞా( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+పరిమాణం(RCStr)+1);
  ఒకవేళ( pNew==0 ){
    sqlite3_free(p);
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి (అక్షరం*)&pNew[1];
  }
}
