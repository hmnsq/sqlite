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
#شامل "sqliteInt.h"

/*
** Conversion types fall into various categories as defined by the
** following enumeration.
*/
#تعریف_کرو etRADIX       0 /* non-decimal integer types.  %x %o */
#تعریف_کرو etFLOAT       1 /* Floating point.  %f */
#تعریف_کرو etEXP         2 /* Exponentional notation. %e and %E */
#تعریف_کرو etGENERIC     3 /* Floating or exponential, depending on exponent. %g */
#تعریف_کرو et_سائز        4 /* Return number of characters processed so far. %n */
#تعریف_کرو etSTRING      5 /* Strings. %s */
#تعریف_کرو etDYNSTRING   6 /* Dynamically allocated strings. %z */
#تعریف_کرو etPERCENT     7 /* Percent symbol. %% */
#تعریف_کرو etCHARX       8 /* Characters. %c */
/* The rest are extensions, not normally found in printf() */
#تعریف_کرو etESCAPE_q    9  /* Strings with '\'' doubled.  %q */
#تعریف_کرو etESCAPE_Q    10 /* Strings with '\'' doubled and enclosed in '',
                            NULL pointers replaced by SQL NULL.  %Q */
#تعریف_کرو etTOKEN       11 /* a pointer to a Token structure */
#تعریف_کرو etSRCITEM     12 /* a pointer to a SrcItem */
#تعریف_کرو et_اشارہ_گر     13 /* The %p conversion */
#تعریف_کرو etESCAPE_w    14 /* %w -> Strings with '\"' doubled */
#تعریف_کرو etORDINAL     15 /* %r -> 1st, 2nd, 3rd, 4th, etc.  English only */
#تعریف_کرو etDECIMAL     16 /* %d or %u, but not %x, %o */
#تعریف_کرو etESCAPE_j    17 /* %j -> JSON string literal w/o "..." */
#تعریف_کرو etESCAPE_J    18 /* %J -> JSON string literal with "..." */

#تعریف_کرو etINVALID     19 /* Any unrecognized conversion type */


/*
** An "etByte" is an 8-bit unsigned value.
*/
قسمتعریف غیرعلامتی حرف etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
قسمتعریف ساخت et_info {   /* Information about each format field */
  حرف fmttype;            /* The format field code letter */
  etByte بنیاد;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte سابقہ;           /* Offset into aPrefix[] of the prefix string */
  حرف iNxt;               /* Next with same hash, or 0 for end of chain */
} et_info;

/*
** Allowed values for et_info.flags
*/
#تعریف_کرو FLAG_SIGNED    1     /* True if the value to convert is signed */
#تعریف_کرو FLAG_STRING    4     /* Allow infinite precision */

/*
** The table is searched by hash.  In the case of %C where C is the character
** and that character has ASCII value j, then the hash is j%25.
**
** The order of the entries in fmtinfo[] and the hash chain was entered
** manually, but based on the output of the following TCL script:
*/
#اگر 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  جائزہ $c %c x
  سیٹ_2 n($c) $x
}
سیٹ_2 mx [llength [array نام n]]
puts "count: $mx"

سیٹ_2 mx 25
puts "*********** mx=$mx ************"
برائے {سیٹ_2 r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array نام n] {
    اگر {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#اگر_ختم /***** End of script ********/

ساکن ثابت حرف aDigits[] = "0123456789ABCDEF0123456789abcdef";
ساکن ثابت حرف aHex[]    = "0123456789abcdef";
ساکن ثابت حرف a_سابقہ[] = "-x0\000X0";
ساکن ثابت et_info fmtinfo[25] = {
  /*  0 */  {  'd', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  1 */  {  'e',  0, 1, etEXP,        30, 0,  0 },
  /*  2 */  {  'f',  0, 1, etFLOAT,      0,  0,  0 },
  /*  3 */  {  'g',  0, 1, etGENERIC,    30, 0,  0 },
  /*  4 */  {  'j',  0, 0, etESCAPE_j,   0,  0,  0 },  /* Hash: 6 */
  /*  5 */  {  'i', 10, 1, etDECIMAL,    0,  0,  0 },
  /*  6 */  {  'Q',  0, 4, etESCAPE_Q,   0,  0,  4 },
  /*  7 */  {  'p', 16, 0, et_اشارہ_گر,    0,  1,  0 },  /* Hash: 12 */
  /*  8 */  {  'S',  0, 0, etSRCITEM,    0,  0,  0 },
  /*  9 */  {  'T',  0, 0, etTOKEN,      0,  0,  0 },
  /* 10 */  {  'n',  0, 0, et_سائز,       0,  0,  0 },
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
خالی sqlite3_Str_Accum_سیٹ_Error(StrAccum *p, u8 eError){
  أكد( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  اگر( p->mxAlloc ) sqlite3_str_reset(p);
  اگر( eError==SQLITE_TOOBIG ) sqlite3_Error_مقصد_Parser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ساکن sqlite3_int64 getIntArg(PrintfArguments *p){
  اگر( p->nArg<=p->n_مستعمل ) واپس 0;
  واپس sqlite3_قدر_int64(p->apArg[p->n_مستعمل++]);
}
ساکن دوہرا getDoubleArg(PrintfArguments *p){
  اگر( p->nArg<=p->n_مستعمل ) واپس 0.0;
  واپس sqlite3_قدر_double(p->apArg[p->n_مستعمل++]);
}
ساکن حرف *get_متن_Arg(PrintfArguments *p){
  اگر( p->nArg<=p->n_مستعمل ) واپس 0;
  واپس (حرف*)sqlite3_قدر_متن(p->apArg[p->n_مستعمل++]);
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
ساکن حرف *printf_عارضی_Buf(sqlite3_str *pAccum, sqlite3_int64 n){
  حرف *z;
  اگر( pAccum->accError ) واپس 0;
  اگر( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3_Str_Accum_سیٹ_Error(pAccum, SQLITE_TOOBIG);
    واپس 0;
  }
  z = sqlite3_malloc(n);
  اگر( z==0 ){
    sqlite3_Str_Accum_سیٹ_Error(pAccum, SQLITE_NOMEM);
  }
  واپس z;
}

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be something smaller, if desired.
*/
#اگر_غیر_تعریف_شدہ SQLITE_PRINT_BUF_سائز
# define SQLITE_PRINT_BUF_سائز 70
#اگر_ختم
#تعریف_کرو etBUFSIZE SQLITE_PRINT_BUF_سائز  /* Size of the output buffer */

/*
** Hard limit on the precision of floating-point conversions to
** the minimum of SQLITE_PRINTF_PRECISION_LIMIT and 100 million.
*/
#اگر_تعریف_شدہ SQLITE_PRINTF_PRECISION_حد_بندی
# اگر SQLITE_PRINTF_PRECISION_حد_بندی<100000000
#   define SQLITE_FP_PRECISION_حد_بندی SQLITE_PRINTF_PRECISION_حد_بندی
# ورنہ
#   define SQLITE_FP_PRECISION_حد_بندی 100000000
# endif
#ورنہ
# define SQLITE_FP_PRECISION_حد_بندی 100000000
#اگر_ختم

/* Forward reference */
ساکن خالی sqlite3StrAppend64(sqlite3_str *p, ثابت حرف *z, i64 N);
ساکن خالی sqlite3StrAppendchar64(sqlite3_str *p, i64 N, حرف c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
خالی sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ثابت حرف *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  صحیح c;                     /* Next character in the format string */
  حرف *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 لمبائی;                /* Length of the field */
  صحیح idx;                   /* A general purpose loop counter */
  i64 چوڑائی;                 /* Width of the current field */
  etByte flag_leftjustify;   /* True if "-" flag is present */
  etByte flag_سابقہ;        /* '+' or ' ' or 0 for prefix */
  etByte flag_alternateform; /* True if "#" flag is present */
  etByte flag_altform2;      /* True if "!" flag is present */
  etByte flag_zeropad;       /* True if field width constant starts with zero */
  etByte flag_long;          /* 1 for the "l" flag, 2 for "ll", 0 by default */
  etByte مکمل;               /* Loop termination flag */
  etByte cThousand;          /* Thousands separator for %d and %u */
  etByte xtype = etINVALID;  /* Conversion paradigm */
  u8 bArgList;               /* True for SQLITE_PRINTF_SQLFUNC */
  حرف سابقہ;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  دوہرا realvalue;          /* Value for real types */
  ثابت et_info *infop;      /* Pointer to the appropriate info structure */
  حرف *z_برآمد;                /* Rendering buffer */
  صحیح n_برآمد;                  /* Size of the rendering buffer */
  حرف *zExtra = 0;          /* Malloced memory used by some conversion */
  صحیح قوت_نما, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  حرف buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  أكد( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  اگر( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }ورنہ{
    bArgList = 0;
  }
  برائے(; (c=(*fmt))!=0; ++fmt){
    اگر( c!='%' ){
      bufpt = (حرف *)fmt;
#اگر HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#ورنہ
      fmt = strchr(fmt, '%');
      اگر( fmt==0 ){
        fmt = bufpt + لڑی_لمبائی(bufpt);
      }
#اگر_ختم
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      اگر( *fmt==0 ) توڑو;
    }
    اگر( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      توڑو;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_سابقہ = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    مکمل = 0;
    چوڑائی = 0;
    flag_long = 0;
    precision = -1;
    کرو{
      انتخاب( c ){
        حالت '-':   flag_leftjustify = 1;     توڑو;
        حالت '+':   flag_سابقہ = '+';        توڑو;
        حالت ' ':   flag_سابقہ = ' ';        توڑو;
        حالت '#':   flag_alternateform = 1;   توڑو;
        حالت '!':   flag_altform2 = 1;        توڑو;
        حالت '0':   flag_zeropad = 1;         توڑو;
        حالت ',':   cThousand = ',';          توڑو;
        طےشدہ:    مکمل = 1;                 توڑو;
        حالت 'l': {
          flag_long = 1;
          c = *++fmt;
          اگر( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          مکمل = 1;
          توڑو;
        }
        حالت '1': حالت '2': حالت '3': حالت '4': حالت '5':
        حالت '6': حالت '7': حالت '8': حالت '9': {
          غیرعلامتی wx = c - '0';
          جب( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          چوڑائی = wx & 0x7fffffff;
#اگر_تعریف_شدہ SQLITE_PRINTF_PRECISION_حد_بندی
          اگر( چوڑائی>SQLITE_PRINTF_PRECISION_حد_بندی ){
            چوڑائی = SQLITE_PRINTF_PRECISION_حد_بندی;
          }
#اگر_ختم
          اگر( c!='.' && c!='l' ){
            مکمل = 1;
          }ورنہ{
            fmt--;
          }
          توڑو;
        }
        حالت '*': {
          اگر( bArgList ){
            چوڑائی = (صحیح)getIntArg(pArgList);
          }ورنہ{
            چوڑائی = va_arg(ap,صحیح);
          }
          اگر( چوڑائی<0 ){
            flag_leftjustify = 1;
            چوڑائی = چوڑائی >= -2147483647 ? -چوڑائی : 0;
          }
#اگر_تعریف_شدہ SQLITE_PRINTF_PRECISION_حد_بندی
          اگر( چوڑائی>SQLITE_PRINTF_PRECISION_حد_بندی ){
            چوڑائی = SQLITE_PRINTF_PRECISION_حد_بندی;
          }
#اگر_ختم
          اگر( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            مکمل = 1;
          }
          توڑو;
        }
        حالت '.': {
          c = *++fmt;
          اگر( c=='*' ){
            اگر( bArgList ){
              precision = (صحیح)getIntArg(pArgList);
            }ورنہ{
              precision = va_arg(ap,صحیح);
            }
            اگر( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }ورنہ{
            غیرعلامتی px = 0;
            جب( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#اگر_تعریف_شدہ SQLITE_PRINTF_PRECISION_حد_بندی
          اگر( precision>SQLITE_PRINTF_PRECISION_حد_بندی ){
            precision = SQLITE_PRINTF_PRECISION_حد_بندی;
          }
#اگر_ختم
          اگر( c=='l' ){
            --fmt;
          }ورنہ{
            مکمل = 1;
          }
          توڑو;
        }
      }
    }جب( !مکمل && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#اگر_تعریف_شدہ SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    برائے(idx=0; idx<Array_سائز(fmtinfo); idx++){
      اگر( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        توڑو;
      }
    }
#ورنہ
    /* Fast hash-table lookup */
    أكد( Array_سائز(fmtinfo)==25 );
    idx = ((غیرعلامتی)c) % 25;
    اگر( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }ورنہ{
      infop = &fmtinfo[0];
      xtype = etINVALID;
    }
#اگر_ختم

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
    أكد( چوڑائی>=0 );
    أكد( precision>=(-1) );
    انتخاب( xtype ){
      حالت et_اشارہ_گر:
        flag_long = حجم(حرف*)==حجم(i64) ? 2 :
                     حجم(حرف*)==حجم(طویل صحیح) ? 1 : 0;
        /* no break */ deliberate_fall_through
      حالت etORDINAL:
      حالت etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      حالت etDECIMAL:
        اگر( infop->flags & FLAG_SIGNED ){
          i64 v;
          اگر( bArgList ){
            v = getIntArg(pArgList);
          }ورنہ اگر( flag_long ){
            اگر( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }ورنہ{
              v = va_arg(ap,طویل صحیح);
            }
          }ورنہ{
            v = va_arg(ap,صحیح);
          }
          اگر( v<0 ){
            testcase( v==سب_سے_چھوٹا_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            سابقہ = '-';
          }ورنہ{
            longvalue = v;
            سابقہ = flag_سابقہ;
          }
        }ورنہ{
          اگر( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }ورنہ اگر( flag_long ){
            اگر( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }ورنہ{
              longvalue = va_arg(ap,غیرعلامتی طویل صحیح);
            }
          }ورنہ{
            longvalue = va_arg(ap,غیرعلامتی صحیح);
          }
          سابقہ = 0;
        }

#اگر WHERETRACE_ENABLED
        اگر( xtype==et_اشارہ_گر && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#اگر_ختم
#اگر TREETRACE_ENABLED
        اگر( xtype==et_اشارہ_گر && sqlite3_درخت_Trace & 0x100000 ) longvalue = 0;
#اگر_ختم

        اگر( longvalue==0 ) flag_alternateform = 0;
        اگر( flag_zeropad && precision<چوڑائی-(سابقہ!=0) ){
          precision = چوڑائی-(سابقہ!=0);
        }
        اگر( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          n_برآمد = etBUFSIZE;
          z_برآمد = buf;
        }ورنہ{
          u64 n;
          n = (u64)precision + 10;
          اگر( cThousand ) n += precision/3;
          z_برآمد = zExtra = printf_عارضی_Buf(pAccum, n);
          اگر( z_برآمد==0 ) واپس;
          n_برآمد = (صحیح)n;
        }
        bufpt = &z_برآمد[n_برآمد-1];
        اگر( xtype==etORDINAL ){
          ساکن ثابت حرف zOrd[] = "thstndrd";
          صحیح x = (صحیح)(longvalue % 10);
          اگر( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ثابت حرف *cset = &aDigits[infop->charset];
          u8 بنیاد = infop->بنیاد;
          کرو{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%بنیاد];
            longvalue = longvalue/بنیاد;
          }جب( longvalue>0 );
        }
        لمبائی = &z_برآمد[n_برآمد-1] - bufpt;
        اگر( precision>لمبائی ){                         /* zero pad */
          i64 nn = precision-لمبائی;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          لمبائی = precision;
        }
        اگر( cThousand ){
          i64 nn = (لمبائی - 1)/3;  /* Number of "," to insert */
          i64 ix = (لمبائی - 1)%3 + 1;
          صحیح ii;
          bufpt -= nn;
          برائے(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            اگر( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        اگر( سابقہ ) *(--bufpt) = سابقہ;               /* Add sign */
        اگر( flag_alternateform && infop->سابقہ ){      /* Add "0" or "0x" */
          ثابت حرف *pre;
          حرف x;
          pre = &a_سابقہ[infop->سابقہ];
          برائے(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        لمبائی = &z_برآمد[n_برآمد-1] - bufpt;
        توڑو;
      حالت etFLOAT:
      حالت etEXP:
      حالت etGENERIC: {
        FpDecode s;
        صحیح i_باری;
        صحیح j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        اگر( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }ورنہ{
          realvalue = va_arg(ap,دوہرا);
        }
        اگر( precision<0 ) precision = 6;         /* Set default precision */
        اگر( precision>SQLITE_FP_PRECISION_حد_بندی ){
          precision = SQLITE_FP_PRECISION_حد_بندی;
        }
        اگر( xtype==etFLOAT ){
          i_باری = -precision;
        }ورنہ اگر( xtype==etGENERIC ){
          اگر( precision==0 ) precision = 1;
          i_باری = precision;
        }ورنہ{
          i_باری = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, i_باری, flag_altform2 ? 20 : 16);
        اگر( s.isSpecial ){
          اگر( s.isSpecial==2 ){
            اگر( flag_zeropad ){
              bufpt = "null";
              لمبائی = 4;
            }ورنہ{
              bufpt = "NaN";
              لمبائی = 3;
            }
            توڑو;
          }ورنہ اگر( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }ورنہ{
            حافظہ_نقل(buf, "-Inf", 5);
            bufpt = buf;
            اگر( s.sign=='-' ){
              /* no-op */
            }ورنہ اگر( flag_سابقہ ){
              buf[0] = flag_سابقہ;
            }ورنہ{
              bufpt++;
            }
            لمبائی = لڑی_لمبائی(bufpt);
            توڑو;
          }
        }
        اگر( s.sign=='-' ){
          اگر( flag_alternateform
           && !flag_سابقہ
           && xtype==etFLOAT
           && s.iDP<=i_باری
          ){
            /* Suppress the minus sign if all of the following are true:
            **   *  The value displayed is zero
            **   *  The '#' flag is used
            **   *  The '+' flag is not used, and
            **   *  The format is %f
            */
            سابقہ = 0;
          }ورنہ{
            سابقہ = '-';
          }
        }ورنہ{
          سابقہ = flag_سابقہ;
        }

        قوت_نما = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        اگر( xtype==etGENERIC ){
          أكد( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          اگر( قوت_نما<-4 || قوت_نما>precision ){
            xtype = etEXP;
          }ورنہ{
            precision = precision - قوت_نما;
            xtype = etFLOAT;
          }
        }ورنہ{
          flag_rtz = flag_altform2;
        }
        اگر( xtype==etEXP ){
          e2 = 0;
        }ورنہ{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)چوڑائی+10;
        اگر( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        اگر( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          اگر( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            اگر( bufpt==0 ){
              sqlite3_Str_Accum_سیٹ_Error(pAccum, SQLITE_NOMEM);
              واپس;
            }
            zExtra = bufpt;
          }ورنہ اگر( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            چوڑائی = لمبائی = 0;
            توڑو;
          }ورنہ{
            bufpt = pAccum->z_متن + pAccum->nChar;
          }
        }ورنہ{
          bufpt = pAccum->z_متن + pAccum->nChar;
        }
        z_برآمد = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        اگر( سابقہ ){
          *(bufpt++) = سابقہ;
        }
        /* Digits prior to the decimal point */
        j = 0;
        أكد( s.n>0 );
        اگر( e2<0 ){
          *(bufpt++) = '0';
        }ورنہ اگر( cThousand ){
          برائے(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            اگر( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }ورنہ{
          j = e2+1;
          اگر( j>s.n ) j = s.n;
          حافظہ_نقل(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          اگر( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        اگر( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        اگر( e2<(-1) && precision>0 ){
          صحیح nn = -1-e2;
          اگر( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        اگر( precision>0 ){
          صحیح nn = s.n - j;
          اگر( NEVER(nn>precision) ) nn = precision;
          اگر( nn>0 ){
            حافظہ_نقل(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          اگر( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        اگر( flag_rtz && flag_dp ){
          جب( bufpt[-1]=='0' ) *(--bufpt) = 0;
          أكد( bufpt>z_برآمد );
          اگر( bufpt[-1]=='.' ){
            اگر( flag_altform2 ){
              *(bufpt++) = '0';
            }ورنہ{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        اگر( xtype==etEXP ){
          قوت_نما = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          اگر( قوت_نما<0 ){
            *(bufpt++) = '-'; قوت_نما = -قوت_نما;
          }ورنہ{
            *(bufpt++) = '+';
          }
          اگر( قوت_نما>=100 ){
            *(bufpt++) = (حرف)((قوت_نما/100)+'0');        /* 100's digit */
            قوت_نما %= 100;
          }
          *(bufpt++) = (حرف)(قوت_نما/10+'0');             /* 10's digit */
          *(bufpt++) = (حرف)(قوت_نما%10+'0');             /* 1's digit */
        }

        لمبائی = bufpt - z_برآمد;
        أكد( لمبائی <= szBufNeeded );
        اگر( لمبائی<چوڑائی ){
          i64 nPad = چوڑائی - لمبائی;
          اگر( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }ورنہ اگر( !flag_zeropad ){
            memmove(z_برآمد+nPad, z_برآمد, لمبائی);
            memset(z_برآمد, ' ', nPad);
          }ورنہ{
            صحیح adj = سابقہ!=0;
            memmove(z_برآمد+nPad+adj, z_برآمد+adj, لمبائی-adj);
            memset(z_برآمد+adj, '0', nPad);
          }
          لمبائی = چوڑائی;
        }

        اگر( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          أكد( pAccum->nChar + لمبائی < SMXV(pAccum->nChar) );
          pAccum->nChar += لمبائی;
          z_برآمد[لمبائی] = 0;
          جاری;
        }ورنہ{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          توڑو;
        }
      }
      حالت et_سائز:
        اگر( !bArgList ){
          *(va_arg(ap,صحیح*)) = pAccum->nChar;
        }
        لمبائی = چوڑائی = 0;
        توڑو;
      حالت etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        لمبائی = 1;
        توڑو;
      حالت etCHARX:
        اگر( bArgList ){
          bufpt = get_متن_Arg(pArgList);
          لمبائی = 1;
          اگر( bufpt ){
            buf[0] = c = *(bufpt++);
            اگر( (c&0xc0)==0xc0 ){
              جب( لمبائی<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[لمبائی++] = *(bufpt++);
              }
            }
          }ورنہ{
            buf[0] = 0;
          }
        }ورنہ{
          غیرعلامتی صحیح ch = va_arg(ap,غیرعلامتی صحیح);
          لمبائی = sqlite3AppendOneUtf8Character(buf, ch);
        }
        اگر( precision>1 ){
          i64 n_پیشگی = 1;
          چوڑائی -= precision-1;
          اگر( چوڑائی>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, چوڑائی-1, ' ');
            چوڑائی = 0;
          }
          sqlite3StrAppend64(pAccum, buf, لمبائی);
          precision--;
          جب( precision > 1 ){
            i64 n_Copy_بائٹیں;
            اگر( n_پیشگی > precision-1 ) n_پیشگی = precision - 1;
            n_Copy_بائٹیں = لمبائی*n_پیشگی;
            اگر( sqlite3StrAccumEnlargeIfNeeded(pAccum, n_Copy_بائٹیں) ){
              توڑو;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->z_متن[pAccum->nChar-n_Copy_بائٹیں], n_Copy_بائٹیں);
            precision -= n_پیشگی;
            n_پیشگی *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        جاؤ adjust_چوڑائی_for_utf8;
      حالت etSTRING:
      حالت etDYNSTRING:
        اگر( bArgList ){
          bufpt = get_متن_Arg(pArgList);
          xtype = etSTRING;
        }ورنہ{
          bufpt = va_arg(ap,حرف*);
        }
        اگر( bufpt==0 ){
          bufpt = "";
        }ورنہ اگر( xtype==etDYNSTRING ){
          اگر( pAccum->nChar==0
           && pAccum->mxAlloc
           && چوڑائی==0
           && precision<0
           && pAccum->accError==0
          ){
            /* Special optimization for sqlite3_mprintf("%z..."):
            ** Extend an existing memory allocation rather than creating
            ** a new one. */
            أكد( (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );
            pAccum->z_متن = bufpt;
            pAccum->nAlloc = sqlite3_Db_Malloc_سائز(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (صحیح)لڑی_لمبائی(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            لمبائی = 0;
            توڑو;
          }
          zExtra = bufpt;
        }
        اگر( precision>=0 ){
          اگر( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            غیرعلامتی حرف *z = (غیرعلامتی حرف*)bufpt;
            جب( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            لمبائی = z - (غیرعلامتی حرف*)bufpt;
          }ورنہ{
            برائے(لمبائی=0; لمبائی<precision && bufpt[لمبائی]; لمبائی++){}
          }
        }ورنہ{
          لمبائی = لڑی_لمبائی(bufpt);
        }
      adjust_چوڑائی_for_utf8:
        اگر( flag_altform2 && چوڑائی>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = لمبائی - 1;
          جب( ii>=0 ) اگر( (bufpt[ii--] & 0xc0)==0x80 ) چوڑائی++;
        }
        توڑو;
      حالت etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      حالت etESCAPE_J: {         /* %J: Generate a JSON string literal */
        حرف *escarg;
        i64 i, j, px, i_آغاز;
        غیرعلامتی حرف ch;

        اگر( bArgList ){
          escarg = get_متن_Arg(pArgList);
        }ورنہ{
          escarg = va_arg(ap,حرف*);
        }
        i_آغاز = sqlite3_str_لمبائی(pAccum);
        اگر( escarg==0 ){
          اگر( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }ورنہ{
          اگر( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          اگر( px<0 ){
            px = 0x7fffffff;
          }ورنہ اگر( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            برائے(i=0; i<px && escarg[i]; i++){
              اگر( (escarg[i]&0xc0)==0x80 ) px++;
            }
            اگر( i==px ){
              جب( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          برائے(i=j=0; i<px; i++){
            اگر( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              اگر( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              اگر( ch==0 ) توڑو;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              اگر( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ورنہ اگر( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }ورنہ{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          اگر( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          اگر( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        اگر( چوڑائی>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_لمبائی(pAccum) - i_آغاز;
          sqlite3_int64 len = n;
          حرف *zz;
          اگر( flag_altform2 && n>0 ){
            zz = sqlite3_str_قدر(pAccum);
            برائے(i=i_آغاز; zz[i]; i++){
              اگر( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          اگر( چوڑائی>len ){
            sqlite3_int64 sp = چوڑائی-len;
            أكد( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (صحیح)sp, ' ');
            اگر( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_قدر(pAccum);
              zz += i_آغاز;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        جاری;
      }
      حالت etESCAPE_q:          /* %q: Escape ' characters */
      حالت etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      حالت etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        صحیح needQuote = 0;
        حرف ch;
        حرف *escarg;
        حرف q;

        اگر( bArgList ){
          escarg = get_متن_Arg(pArgList);
        }ورنہ{
          escarg = va_arg(ap,حرف*);
        }
        اگر( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }ورنہ اگر( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        اگر( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }ورنہ{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        برائے(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          اگر( ch==q )  n++;
          اگر( flag_altform2 && (ch&0xc0)==0xc0 ){
            جب( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        اگر( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 n_عقب = 0;
          i64 nCtrl = 0;
          برائے(k=0; k<i; k++){
            اگر( escarg[k]=='\\' ){
              n_عقب++;
            }ورنہ اگر( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          اگر( nCtrl || xtype==etESCAPE_q ){
            n += n_عقب + 5*nCtrl;
            اگر( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }ورنہ{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        اگر( n>etBUFSIZE ){
          bufpt = zExtra = printf_عارضی_Buf(pAccum, n);
          اگر( bufpt==0 ) واپس;
        }ورنہ{
          bufpt = buf;
        }
        j = 0;
        اگر( needQuote ){
          اگر( needQuote==2 ){
            حافظہ_نقل(&bufpt[j], "unistr('", 8);
            j += 8;
          }ورنہ{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        اگر( flag_alternateform ){
          برائے(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            اگر( ch==q ){
              bufpt[j++] = ch;
            }ورنہ اگر( ch=='\\' ){
              bufpt[j++] = '\\';
            }ورنہ اگر( ((غیرعلامتی حرف)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }ورنہ{
          برائے(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            اگر( ch==q ) bufpt[j++] = ch;
          }
        }
        اگر( needQuote ){
          bufpt[j++] = '\'';
          اگر( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        لمبائی = j;
        جاؤ adjust_چوڑائی_for_utf8;
      }
      حالت etTOKEN: {
        اگر( (pAccum->printfFlags & SQLITE_PRINTF_اندرونی)==0 ) واپس;
        اگر( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          اظہاریہ *p_اظہاریہ = va_arg(ap,اظہاریہ*);
          اگر( ALWAYS(p_اظہاریہ) && ALWAYS(!اظہاریہ_Has_Property(p_اظہاریہ,EP_Int_قدر)) ){
            sqlite3_str_appendall(pAccum, (ثابت حرف*)p_اظہاریہ->u.zToken);
            sqlite3_ریکارڈ_Error_انحراف_کا_اظہاریہ(pAccum->db, p_اظہاریہ);
          }
        }ورنہ{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          أكد( bArgList==0 );
          اگر( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ثابت حرف*)pToken->z, pToken->n);
            sqlite3_ریکارڈ_Error_Byte_انحراف(pAccum->db, pToken->z);
          }
        }
        لمبائی = چوڑائی = 0;
        توڑو;
      }
      حالت etSRCITEM: {
        Src_شے *p_شے;
        اگر( (pAccum->printfFlags & SQLITE_PRINTF_اندرونی)==0 ) واپس;
        p_شے = va_arg(ap, Src_شے*);
        أكد( bArgList==0 );
        اگر( p_شے->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, p_شے->zAlias);
        }ورنہ اگر( p_شے->zName ){
          اگر( p_شے->fg.fixedSchema==0
           && p_شے->fg.isSubquery==0
           && p_شے->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, p_شے->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, p_شے->zName);
        }ورنہ اگر( p_شے->zAlias ){
          sqlite3_str_appendall(pAccum, p_شے->zAlias);
        }ورنہ اگر( ALWAYS(p_شے->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = p_شے->u4.pSubq->pSelect;
          أكد( pSel!=0 ); 
          اگر( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }ورنہ اگر( pSel->selFlags & SF_Multi_قدر ){
            أكد( !p_شے->fg.isTabFunc && !p_شے->fg.is_اشاریہ_دار_By );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                p_شے->u1.n_صف);
          }ورنہ{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        لمبائی = چوڑائی = 0;
        توڑو;
      }
      طےشدہ: {
        أكد( xtype==etINVALID );
        واپس;
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
    چوڑائی -= لمبائی;
    اگر( چوڑائی>0 ){
      اگر( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, چوڑائی, ' ');
      sqlite3StrAppend64(pAccum, bufpt, لمبائی);
      اگر( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, چوڑائی, ' ');
    }ورنہ{
      sqlite3StrAppend64(pAccum, bufpt, لمبائی);
    }

    اگر( zExtra ){
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
خالی sqlite3_ریکارڈ_Error_Byte_انحراف(sqlite3 *db, ثابت حرف *z){
  ثابت Parse *pParse;
  ثابت حرف *z_متن;
  ثابت حرف *z_انتہا;
  أكد( z!=0 );
  اگر( NEVER(db==0) ) واپس;
  اگر( db->err_Byte_انحراف!=(-2) ) واپس;
  pParse = db->pParse;
  اگر( NEVER(pParse==0) ) واپس;
  z_متن =pParse->z_دم;
  اگر( NEVER(z_متن==0) ) واپس;
  z_انتہا = &z_متن[لڑی_لمبائی(z_متن)];
  اگر( SQLITE_WITHIN(z,z_متن,z_انتہا) ){
    db->err_Byte_انحراف = (صحیح)(z-z_متن);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
خالی sqlite3_ریکارڈ_Error_انحراف_کا_اظہاریہ(sqlite3 *db, ثابت اظہاریہ *p_اظہاریہ){
  جب( p_اظہاریہ ){
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Reduced|EP_TokenOnly) ) واپس;
    اگر( اظہاریہ_Has_Property(p_اظہاریہ,EP_OuterON|EP_InnerON) || p_اظہاریہ->w.iOfst<=0 ){
      p_اظہاریہ = p_اظہاریہ->p_بایاں;
    }ورنہ{
      توڑو;
    }
  }
  اگر( p_اظہاریہ==0 ) واپس;
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_FromDDL) ) واپس;
  db->err_Byte_انحراف = p_اظہاریہ->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
صحیح sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  حرف *zNew;
  أكد( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  اگر( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    واپس 0;
  }
  اگر( p->mxAlloc==0 ){
    sqlite3_Str_Accum_سیٹ_Error(p, SQLITE_TOOBIG);
    واپس p->nAlloc - p->nChar - 1;
  }ورنہ{
    حرف *zOld = isMalloced(p) ? p->z_متن : 0;
    i64 szNew = p->nChar + N + 1;
    اگر( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    اگر( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_سیٹ_Error(p, SQLITE_TOOBIG);
      واپس 0;
    }ورنہ{
      p->nAlloc = (صحیح)szNew;
    }
    اگر( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }ورنہ{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    اگر( zNew ){
      أكد( p->z_متن!=0 || p->nChar==0 );
      اگر( !isMalloced(p) && p->nChar>0 ) حافظہ_نقل(zNew, p->z_متن, p->nChar);
      p->z_متن = zNew;
      p->nAlloc = sqlite3_Db_Malloc_سائز(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }ورنہ{
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_سیٹ_Error(p, SQLITE_NOMEM);
      واپس 0;
    }
  }
  أكد( N>=0 && N<=0x7fffffff );
  واپس (صحیح)N;
}

صحیح sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  اگر( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  واپس p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
خالی sqlite3_str_appendchar(sqlite3_str *p, صحیح N, حرف c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  اگر( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    واپس;
  }
  جب( (N--)>0 ) p->z_متن[p->nChar++] = c;
}
ساکن خالی sqlite3StrAppendchar64(sqlite3_str *p, i64 N, حرف c){
  testcase( p->nChar + N > 0x7fffffff );
  اگر( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    واپس;
  }
  جب( (N--)>0 ) p->z_متن[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
ساکن خالی SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ثابت حرف *z, صحیح N){
  N = sqlite3StrAccumEnlarge(p, N);
  أكد( z!=0 || N==0 );
  اگر( N>0 ){
    حافظہ_نقل(&p->z_متن[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
خالی sqlite3_str_append(sqlite3_str *p, ثابت حرف *z, صحیح N){
  أكد( z!=0 || N==0 );
  أكد( p->z_متن!=0 || p->nChar==0 || p->accError );
  أكد( N>=0 );
  أكد( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  اگر( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ورنہ اگر( N ){
    أكد( p->z_متن );
    p->nChar += N;
    حافظہ_نقل(&p->z_متن[p->nChar-N], z, N);
  }
}
ساکن خالی sqlite3StrAppend64(sqlite3_str *p, ثابت حرف *z, i64 N){
  أكد( z!=0 || N==0 );
  أكد( p->z_متن!=0 || p->nChar==0 || p->accError );
  أكد( N>=0 );
  أكد( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  اگر( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }ورنہ اگر( N ){
    أكد( p->z_متن );
    p->nChar += N;
    حافظہ_نقل(&p->z_متن[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
خالی sqlite3_str_appendall(sqlite3_str *p, ثابت حرف *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
ساکن SQLITE_NOINLINE حرف *str_Accum_اختتام_Realloc(StrAccum *p){
  حرف *z_متن;
  أكد( p->mxAlloc>0 && !isMalloced(p) );
  z_متن = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  اگر( z_متن ){
    حافظہ_نقل(z_متن, p->z_متن, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }ورنہ{
    sqlite3_Str_Accum_سیٹ_Error(p, SQLITE_NOMEM);
  }
  p->z_متن = z_متن;
  واپس z_متن;
}
حرف *sqlite3_Str_Accum_اختتام(StrAccum *p){
  اگر( p->z_متن ){
    p->z_متن[p->nChar] = 0;
    اگر( p->mxAlloc>0 && !isMalloced(p) ){
      واپس str_Accum_اختتام_Realloc(p);
    }
  }
  واپس p->z_متن;
}

/* Finalize a string created using sqlite3_str_new().
*/
حرف *sqlite3_str_اختتام(sqlite3_str *p){
  حرف *z;
  اگر( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3_Str_Accum_اختتام(p);
    sqlite3_free(p);
  }ورنہ{
    z = 0;
  }
  واپس z;
}

/* Return any error code associated with p */
صحیح sqlite3_str_errcode(sqlite3_str *p){
  واپس p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
صحیح sqlite3_str_لمبائی(sqlite3_str *p){
  واپس p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
خالی sqlite3_str_truncate(sqlite3_str *p, صحیح N){
  اگر( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->z_متن[p->nChar] = 0;
  }
}

/* Return the current value for p */
حرف *sqlite3_str_قدر(sqlite3_str *p){
  اگر( p==0 || p->nChar==0 ) واپس 0;
  p->z_متن[p->nChar] = 0;
  واپس p->z_متن;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
خالی sqlite3_str_reset(StrAccum *p){
  اگر( isMalloced(p) ){
    sqlite3DbFree(p->db, p->z_متن);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }ورنہ اگر( p==(sqlite3_str*)&sqlite3OomStr ){
    واپس;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->z_متن = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
خالی sqlite3_str_free(sqlite3_str *p){
  اگر( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
خالی sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, حرف *z_بنیاد, صحیح n, صحیح mx){
  p->z_متن = z_بنیاد;
  p->db = db;
  p->nAlloc = n;
  p->mxAlloc = mx;
  p->nChar = 0;
  p->accError = 0;
  p->printfFlags = 0;
}

/* Allocate and initialize a new dynamic string object */
sqlite3_str *sqlite3_str_new(sqlite3 *db){
  sqlite3_str *p = sqlite3_malloc64(حجم(*p));
  اگر( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->a_حد_بندی[SQLITE_حد_بندی_لمبائی] : SQLITE_MAX_لمبائی);
  }ورنہ{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  واپس p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
حرف *sqlite3VMPrintf(sqlite3 *db, ثابت حرف *zFormat, va_list ap){
  حرف *z;
  حرف z_بنیاد[SQLITE_PRINT_BUF_سائز];
  StrAccum acc;
  أكد( db!=0 );
  sqlite3StrAccumInit(&acc, db, z_بنیاد, حجم(z_بنیاد),
                      db->a_حد_بندی[SQLITE_حد_بندی_لمبائی]);
  acc.printfFlags = SQLITE_PRINTF_اندرونی;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_اختتام(&acc);
  اگر( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  واپس z;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
حرف *sqlite3MPrintf(sqlite3 *db, ثابت حرف *zFormat, ...){
  va_list ap;
  حرف *z;
  va_start(ap, zFormat);
  z = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  واپس z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
حرف *sqlite3_vmprintf(ثابت حرف *zFormat, va_list ap){
  حرف *z;
  حرف z_بنیاد[SQLITE_PRINT_BUF_سائز];
  StrAccum acc;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR  
  اگر( zFormat==0 ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  sqlite3StrAccumInit(&acc, 0, z_بنیاد, حجم(z_بنیاد), SQLITE_MAX_لمبائی);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_اختتام(&acc);
  واپس z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
حرف *sqlite3_mprintf(ثابت حرف *zFormat, ...){
  va_list ap;
  حرف *z;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  واپس z;
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
حرف *sqlite3_vsnprintf(صحیح n, حرف *zBuf, ثابت حرف *zFormat, va_list ap){
  StrAccum acc;
  اگر( n<=0 ) واپس zBuf;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( zBuf==0 || zFormat==0 ) {
    (خالی)SQLITE_MISUSE_BKPT;
    اگر( zBuf ) zBuf[0] = 0;
    واپس zBuf;
  }
#اگر_ختم
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  واپس zBuf;
}
حرف *sqlite3_snprintf(صحیح n, حرف *zBuf, ثابت حرف *zFormat, ...){
  StrAccum acc;
  va_list ap;
  اگر( n<=0 ) واپس zBuf;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( zBuf==0 || zFormat==0 ) {
    (خالی)SQLITE_MISUSE_BKPT;
    اگر( zBuf ) zBuf[0] = 0;
    واپس zBuf;
  }
#اگر_ختم
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  واپس zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#اگر defined(SQLITE_MAX_LOG_پیغام) 
  /* Leave the definition as supplied */
#وگرنہ_اگر SQLITE_PRINT_BUF_سائز*10>10000
# define SQLITE_MAX_LOG_پیغام 10000
#ورنہ
# define SQLITE_MAX_LOG_پیغام (SQLITE_PRINT_BUF_سائز*10)
#اگر_ختم

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
ساکن خالی render_Log_پیغام(صحیح i_Err_رمز, ثابت حرف *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  حرف z_پیغام[SQLITE_MAX_LOG_پیغام];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_پیغام, حجم(z_پیغام), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, i_Err_رمز,
                           sqlite3_Str_Accum_اختتام(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
خالی sqlite3_log(صحیح i_Err_رمز, ثابت حرف *zFormat, ...){
  va_list ap;                             /* Vararg list */
  اگر( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_پیغام(i_Err_رمز, zFormat, ap);
    va_end(ap);
  }
}

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
خالی sqlite3DebugPrintf(ثابت حرف *zFormat, ...){
  va_list ap;
  StrAccum acc;
  حرف zBuf[SQLITE_PRINT_BUF_سائز*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3_Str_Accum_اختتام(&acc);
#اگر_تعریف_شدہ SQLITE_OS_TRACE_PROC
  {
    خارجی خالی SQLITE_OS_TRACE_PROC(ثابت حرف *zBuf, صحیح nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, حجم(zBuf));
  }
#ورنہ
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#اگر_ختم
}
#اگر_ختم


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
خالی sqlite3_str_appendf(StrAccum *p, ثابت حرف *zFormat, ...){
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
حرف *sqlite3RCStrRef(حرف *z){
  RCStr *p = (RCStr*)z;
  أكد( p!=0 );
  p--;
  p->nRCRef++;
  واپس z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
خالی sqlite3RCStrUnref(خالی *z){
  RCStr *p = (RCStr*)z;
  أكد( p!=0 );
  p--;
  أكد( p->nRCRef>0 );
  اگر( p->nRCRef>=2 ){
    p->nRCRef--;
  }ورنہ{
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
حرف *sqlite3RCStrNew(u64 N){
  RCStr *p = sqlite3_malloc64( N + حجم(*p) + 1 );
  اگر( p==0 ) واپس 0;
  p->nRCRef = 1;
  واپس (حرف*)&p[1];
}

/*
** Change the size of the string so that it is able to hold N bytes.
** The string might be reallocated, so return the new allocation.
*/
حرف *sqlite3RCStrResize(حرف *z, u64 N){
  RCStr *p = (RCStr*)z;
  RCStr *pNew;
  أكد( p!=0 );
  p--;
  أكد( p->nRCRef==1 );
  pNew = sqlite3_realloc64(p, N+حجم(RCStr)+1);
  اگر( pNew==0 ){
    sqlite3_free(p);
    واپس 0;
  }ورنہ{
    واپس (حرف*)&pNew[1];
  }
}
