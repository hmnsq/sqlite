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
#ضمن "sqliteInt.h"

/*
** Conversion types fall into various categories as defined by the
** following enumeration.
*/
#تعریف_کرو etRADIX       0 /* non-decimal integer types.  %x %o */
#تعریف_کرو etFLOAT       1 /* Floating point.  %f */
#تعریف_کرو etEXP         2 /* Exponentional notation. %e and %E */
#تعریف_کرو etGENERIC     3 /* Floating or exponential, depending on exponent. %g */
#تعریف_کرو et_ناپ        4 /* Return number of characters processed so far. %n */
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
تعريفنوع بنانشانی حرف etByte;

/*
** Each builtin conversion character (ex: the 'd' in "%d") is described
** by an instance of the following structure
*/
تعريفنوع بنية et_info {   /* Information about each format field */
  حرف fmttype;            /* The format field code letter */
  etByte ادھار;             /* The base for radix conversion */
  etByte flags;            /* One or more of FLAG_ constants below */
  etByte type;             /* Conversion paradigm */
  etByte charset;          /* Offset into aDigits[] of the digits string */
  etByte اگیتر;           /* Offset into aPrefix[] of the prefix string */
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
#إذا 0  /*****  Beginning of script ******/
foreach c {d s g z q Q w c o u x X f e E G i n % p T S r J j} {
  جانچ $c %c x
  سیٹ_2 n($c) $x
}
سیٹ_2 mx [llength [array ناں n]]
puts "count: $mx"

سیٹ_2 mx 25
puts "*********** mx=$mx ************"
لكل {سیٹ_2 r 0} {$r<$mx} {incr r} {
  puts -nonewline [format %2d: $r]
  foreach c [array ناں n] {
    إذا {($n($c))%$mx==$r} {puts -nonewline " $c"}
  }
  puts ""
}
#جے_انت /***** End of script ********/

ساكن ثابت حرف aDigits[] = "0123456789ABCDEF0123456789abcdef";
ساكن ثابت حرف aHex[]    = "0123456789abcdef";
ساكن ثابت حرف a_اگیتر[] = "-x0\000X0";
ساكن ثابت et_info fmtinfo[25] = {
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
  /* 10 */  {  'n',  0, 0, et_ناپ,       0,  0,  0 },
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
فراغ sqlite3_Str_Accum_سیٹ_Error(StrAccum *p, u8 eError){
  أكد( eError==SQLITE_NOMEM || eError==SQLITE_TOOBIG );
  p->accError = eError;
  إذا( p->mxAlloc ) sqlite3_str_reset(p);
  إذا( eError==SQLITE_TOOBIG ) sqlite3_Error_منزل_Parser(p->db, eError);
}

/*
** Extra argument values from a PrintfArguments object
*/
ساكن sqlite3_int64 getIntArg(PrintfArguments *p){
  إذا( p->nArg<=p->n_ورتیا ) أرجع 0;
  أرجع sqlite3_قدر_int64(p->apArg[p->n_ورتیا++]);
}
ساكن مزدوج getDoubleArg(PrintfArguments *p){
  إذا( p->nArg<=p->n_ورتیا ) أرجع 0.0;
  أرجع sqlite3_قدر_double(p->apArg[p->n_ورتیا++]);
}
ساكن حرف *get_متن_Arg(PrintfArguments *p){
  إذا( p->nArg<=p->n_ورتیا ) أرجع 0;
  أرجع (حرف*)sqlite3_قدر_متن(p->apArg[p->n_ورتیا++]);
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
ساكن حرف *printf_عارضی_Buf(sqlite3_str *pAccum, sqlite3_int64 n){
  حرف *z;
  إذا( pAccum->accError ) أرجع 0;
  إذا( n>pAccum->nAlloc && n>pAccum->mxAlloc ){
    sqlite3_Str_Accum_سیٹ_Error(pAccum, SQLITE_TOOBIG);
    أرجع 0;
  }
  z = sqlite3_malloc(n);
  إذا( z==0 ){
    sqlite3_Str_Accum_سیٹ_Error(pAccum, SQLITE_NOMEM);
  }
  أرجع z;
}

/*
** On machines with a small stack size, you can redefine the
** SQLITE_PRINT_BUF_SIZE to be something smaller, if desired.
*/
#جے_بنا_تعریف SQLITE_PRINT_BUF_ناپ
# define SQLITE_PRINT_BUF_ناپ 70
#جے_انت
#تعریف_کرو etBUFSIZE SQLITE_PRINT_BUF_ناپ  /* Size of the output buffer */

/*
** Hard limit on the precision of floating-point conversions to
** the minimum of SQLITE_PRINTF_PRECISION_LIMIT and 100 million.
*/
#جے_تعریف_شدہ SQLITE_PRINTF_PRECISION_حد_بندی
# إذا SQLITE_PRINTF_PRECISION_حد_بندی<100000000
#   define SQLITE_FP_PRECISION_حد_بندی SQLITE_PRINTF_PRECISION_حد_بندی
# وإلا
#   define SQLITE_FP_PRECISION_حد_بندی 100000000
# endif
#وإلا
# define SQLITE_FP_PRECISION_حد_بندی 100000000
#جے_انت

/* Forward reference */
ساكن فراغ sqlite3StrAppend64(sqlite3_str *p, ثابت حرف *z, i64 N);
ساكن فراغ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, حرف c);

/*
** Render a string given by "fmt" into the StrAccum object.
*/
فراغ sqlite3_str_vappendf(
  sqlite3_str *pAccum,       /* Accumulate results here */
  ثابت حرف *fmt,           /* Format string */
  va_list ap                 /* arguments */
){
  صحيح c;                     /* Next character in the format string */
  حرف *bufpt;               /* Pointer to the conversion buffer */
  i64 precision;             /* Precision of the current field */
  i64 لمائی;                /* Length of the field */
  صحيح idx;                   /* A general purpose loop counter */
  i64 چوڑائی;                 /* Width of the current field */
  etByte flag_leftjustify;   /* True if "-" flag is present */
  etByte flag_اگیتر;        /* '+' or ' ' or 0 for prefix */
  etByte flag_alternateform; /* True if "#" flag is present */
  etByte flag_altform2;      /* True if "!" flag is present */
  etByte flag_zeropad;       /* True if field width constant starts with zero */
  etByte flag_long;          /* 1 for the "l" flag, 2 for "ll", 0 by default */
  etByte پورا;               /* Loop termination flag */
  etByte cThousand;          /* Thousands separator for %d and %u */
  etByte xtype = etINVALID;  /* Conversion paradigm */
  u8 bArgList;               /* True for SQLITE_PRINTF_SQLFUNC */
  حرف اگیتر;               /* Prefix character.  "+" or "-" or " " or '\0'. */
  sqlite_uint64 longvalue;   /* Value for integer types */
  مزدوج realvalue;          /* Value for real types */
  ثابت et_info *infop;      /* Pointer to the appropriate info structure */
  حرف *z_باہرلا;                /* Rendering buffer */
  صحيح n_باہرلا;                  /* Size of the rendering buffer */
  حرف *zExtra = 0;          /* Malloced memory used by some conversion */
  صحيح قوت_نما, e2;               /* exponent of real numbers */
  etByte flag_dp;            /* True if decimal point should be shown */
  etByte flag_rtz;           /* True if trailing zeros should be removed */

  PrintfArguments *pArgList = 0; /* Arguments for SQLITE_PRINTF_SQLFUNC */
  حرف buf[etBUFSIZE];       /* Conversion buffer */

  /* pAccum never starts out with an empty buffer that was obtained from 
  ** malloc().  This precondition is required by the mprintf("%z...")
  ** optimization. */
  أكد( pAccum->nChar>0 || (pAccum->printfFlags&SQLITE_PRINTF_MALLOCED)==0 );

  bufpt = 0;
  إذا( (pAccum->printfFlags & SQLITE_PRINTF_SQLFUNC)!=0 ){
    pArgList = va_arg(ap, PrintfArguments*);
    bArgList = 1;
  }وإلا{
    bArgList = 0;
  }
  لكل(; (c=(*fmt))!=0; ++fmt){
    إذا( c!='%' ){
      bufpt = (حرف *)fmt;
#إذا HAVE_STRCHRNUL
      fmt = strchrnul(fmt, '%');
#وإلا
      fmt = strchr(fmt, '%');
      إذا( fmt==0 ){
        fmt = bufpt + لڑی_لمبائی(bufpt);
      }
#جے_انت
      sqlite3StrAppend64(pAccum, bufpt, fmt - bufpt);
      إذا( *fmt==0 ) اكسر;
    }
    إذا( (c=(*++fmt))==0 ){
      sqlite3_str_append(pAccum, "%", 1);
      اكسر;
    }
    /* Find out what flags are present */
    flag_leftjustify = flag_اگیتر = cThousand =
     flag_alternateform = flag_altform2 = flag_zeropad = 0;
    پورا = 0;
    چوڑائی = 0;
    flag_long = 0;
    precision = -1;
    افعل{
      بدل( c ){
        حالة '-':   flag_leftjustify = 1;     اكسر;
        حالة '+':   flag_اگیتر = '+';        اكسر;
        حالة ' ':   flag_اگیتر = ' ';        اكسر;
        حالة '#':   flag_alternateform = 1;   اكسر;
        حالة '!':   flag_altform2 = 1;        اكسر;
        حالة '0':   flag_zeropad = 1;         اكسر;
        حالة ',':   cThousand = ',';          اكسر;
        افتراضي:    پورا = 1;                 اكسر;
        حالة 'l': {
          flag_long = 1;
          c = *++fmt;
          إذا( c=='l' ){
            c = *++fmt;
            flag_long = 2;
          }
          پورا = 1;
          اكسر;
        }
        حالة '1': حالة '2': حالة '3': حالة '4': حالة '5':
        حالة '6': حالة '7': حالة '8': حالة '9': {
          بنانشانی wx = c - '0';
          طالما( (c = *++fmt)>='0' && c<='9' ){
            wx = wx*10 + c - '0';
          }
          testcase( wx>0x7fffffff );
          چوڑائی = wx & 0x7fffffff;
#جے_تعریف_شدہ SQLITE_PRINTF_PRECISION_حد_بندی
          إذا( چوڑائی>SQLITE_PRINTF_PRECISION_حد_بندی ){
            چوڑائی = SQLITE_PRINTF_PRECISION_حد_بندی;
          }
#جے_انت
          إذا( c!='.' && c!='l' ){
            پورا = 1;
          }وإلا{
            fmt--;
          }
          اكسر;
        }
        حالة '*': {
          إذا( bArgList ){
            چوڑائی = (صحيح)getIntArg(pArgList);
          }وإلا{
            چوڑائی = va_arg(ap,صحيح);
          }
          إذا( چوڑائی<0 ){
            flag_leftjustify = 1;
            چوڑائی = چوڑائی >= -2147483647 ? -چوڑائی : 0;
          }
#جے_تعریف_شدہ SQLITE_PRINTF_PRECISION_حد_بندی
          إذا( چوڑائی>SQLITE_PRINTF_PRECISION_حد_بندی ){
            چوڑائی = SQLITE_PRINTF_PRECISION_حد_بندی;
          }
#جے_انت
          إذا( (c = fmt[1])!='.' && c!='l' ){
            c = *++fmt;
            پورا = 1;
          }
          اكسر;
        }
        حالة '.': {
          c = *++fmt;
          إذا( c=='*' ){
            إذا( bArgList ){
              precision = (صحيح)getIntArg(pArgList);
            }وإلا{
              precision = va_arg(ap,صحيح);
            }
            إذا( precision<0 ){
              precision = precision >= -2147483647 ? -precision : -1;
            }
            c = *++fmt;
          }وإلا{
            بنانشانی px = 0;
            طالما( c>='0' && c<='9' ){
              px = px*10 + c - '0';
              c = *++fmt;
            }
            testcase( px>0x7fffffff );
            precision = px & 0x7fffffff;
          }
#جے_تعریف_شدہ SQLITE_PRINTF_PRECISION_حد_بندی
          إذا( precision>SQLITE_PRINTF_PRECISION_حد_بندی ){
            precision = SQLITE_PRINTF_PRECISION_حد_بندی;
          }
#جے_انت
          إذا( c=='l' ){
            --fmt;
          }وإلا{
            پورا = 1;
          }
          اكسر;
        }
      }
    }طالما( !پورا && (c=(*++fmt))!=0 );

    /* Fetch the info entry for the field */
#جے_تعریف_شدہ SQLITE_EBCDIC
    /* The hash table only works for ASCII.  For EBCDIC, we need to do
    ** a linear search of the table */
    infop = &fmtinfo[0];
    xtype = etINVALID;
    لكل(idx=0; idx<Array_ناپ(fmtinfo); idx++){
      إذا( c==fmtinfo[idx].fmttype ){
        infop = &fmtinfo[idx];
        xtype = infop->type;
        اكسر;
      }
    }
#وإلا
    /* Fast hash-table lookup */
    أكد( Array_ناپ(fmtinfo)==25 );
    idx = ((بنانشانی)c) % 25;
    إذا( fmtinfo[idx].fmttype==c
     || fmtinfo[idx = fmtinfo[idx].iNxt].fmttype==c
    ){
      infop = &fmtinfo[idx];
      xtype = infop->type;
    }وإلا{
      infop = &fmtinfo[0];
      xtype = etINVALID;
    }
#جے_انت

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
    بدل( xtype ){
      حالة et_اشارہ_گر:
        flag_long = حجم(حرف*)==حجم(i64) ? 2 :
                     حجم(حرف*)==حجم(لما صحيح) ? 1 : 0;
        /* no break */ deliberate_fall_through
      حالة etORDINAL:
      حالة etRADIX:      
        cThousand = 0;
        /* no break */ deliberate_fall_through
      حالة etDECIMAL:
        إذا( infop->flags & FLAG_SIGNED ){
          i64 v;
          إذا( bArgList ){
            v = getIntArg(pArgList);
          }وإلا إذا( flag_long ){
            إذا( flag_long==2 ){
              v = va_arg(ap,i64) ;
            }وإلا{
              v = va_arg(ap,لما صحيح);
            }
          }وإلا{
            v = va_arg(ap,صحيح);
          }
          إذا( v<0 ){
            testcase( v==سبھ_توں_چھوٹا_INT64 );
            testcase( v==(-1) );
            longvalue = ~v;
            longvalue++;
            اگیتر = '-';
          }وإلا{
            longvalue = v;
            اگیتر = flag_اگیتر;
          }
        }وإلا{
          إذا( bArgList ){
            longvalue = (u64)getIntArg(pArgList);
          }وإلا إذا( flag_long ){
            إذا( flag_long==2 ){
              longvalue = va_arg(ap,u64);
            }وإلا{
              longvalue = va_arg(ap,بنانشانی لما صحيح);
            }
          }وإلا{
            longvalue = va_arg(ap,بنانشانی صحيح);
          }
          اگیتر = 0;
        }

#إذا WHERETRACE_ENABLED
        إذا( xtype==et_اشارہ_گر && sqlite3WhereTrace & 0x100000 ) longvalue = 0;
#جے_انت
#إذا TREETRACE_ENABLED
        إذا( xtype==et_اشارہ_گر && sqlite3_درخت_Trace & 0x100000 ) longvalue = 0;
#جے_انت

        إذا( longvalue==0 ) flag_alternateform = 0;
        إذا( flag_zeropad && precision<چوڑائی-(اگیتر!=0) ){
          precision = چوڑائی-(اگیتر!=0);
        }
        إذا( precision<etBUFSIZE-10-etBUFSIZE/3 ){
          n_باہرلا = etBUFSIZE;
          z_باہرلا = buf;
        }وإلا{
          u64 n;
          n = (u64)precision + 10;
          إذا( cThousand ) n += precision/3;
          z_باہرلا = zExtra = printf_عارضی_Buf(pAccum, n);
          إذا( z_باہرلا==0 ) أرجع;
          n_باہرلا = (صحيح)n;
        }
        bufpt = &z_باہرلا[n_باہرلا-1];
        إذا( xtype==etORDINAL ){
          ساكن ثابت حرف zOrd[] = "thstndrd";
          صحيح x = (صحيح)(longvalue % 10);
          إذا( x>=4 || (longvalue/10)%10==1 ){
            x = 0;
          }
          *(--bufpt) = zOrd[x*2+1];
          *(--bufpt) = zOrd[x*2];
        }
        {
          ثابت حرف *cset = &aDigits[infop->charset];
          u8 ادھار = infop->ادھار;
          افعل{                                           /* Convert to ascii */
            *(--bufpt) = cset[longvalue%ادھار];
            longvalue = longvalue/ادھار;
          }طالما( longvalue>0 );
        }
        لمائی = &z_باہرلا[n_باہرلا-1] - bufpt;
        إذا( precision>لمائی ){                         /* zero pad */
          i64 nn = precision-لمائی;
          bufpt -= nn;
          memset(bufpt,'0',nn);
          لمائی = precision;
        }
        إذا( cThousand ){
          i64 nn = (لمائی - 1)/3;  /* Number of "," to insert */
          i64 ix = (لمائی - 1)%3 + 1;
          صحيح ii;
          bufpt -= nn;
          لكل(ii=0; nn>0; ii++){
            bufpt[ii] = bufpt[ii+nn];
            ix--;
            إذا( ix==0 ){
              bufpt[++ii] = cThousand;
              nn--;
              ix = 3;
            }
          }
        }
        إذا( اگیتر ) *(--bufpt) = اگیتر;               /* Add sign */
        إذا( flag_alternateform && infop->اگیتر ){      /* Add "0" or "0x" */
          ثابت حرف *pre;
          حرف x;
          pre = &a_اگیتر[infop->اگیتر];
          لكل(; (x=(*pre))!=0; pre++) *(--bufpt) = x;
        }
        لمائی = &z_باہرلا[n_باہرلا-1] - bufpt;
        اكسر;
      حالة etFLOAT:
      حالة etEXP:
      حالة etGENERIC: {
        FpDecode s;
        صحيح i_گیڑا;
        صحيح j;
        i64 szBufNeeded;       /* Size needed to hold the output */

        إذا( bArgList ){
          realvalue = getDoubleArg(pArgList);
        }وإلا{
          realvalue = va_arg(ap,مزدوج);
        }
        إذا( precision<0 ) precision = 6;         /* Set default precision */
        إذا( precision>SQLITE_FP_PRECISION_حد_بندی ){
          precision = SQLITE_FP_PRECISION_حد_بندی;
        }
        إذا( xtype==etFLOAT ){
          i_گیڑا = -precision;
        }وإلا إذا( xtype==etGENERIC ){
          إذا( precision==0 ) precision = 1;
          i_گیڑا = precision;
        }وإلا{
          i_گیڑا = precision+1;
        }
        sqlite3FpDecode(&s, realvalue, i_گیڑا, flag_altform2 ? 20 : 16);
        إذا( s.isSpecial ){
          إذا( s.isSpecial==2 ){
            إذا( flag_zeropad ){
              bufpt = "null";
              لمائی = 4;
            }وإلا{
              bufpt = "NaN";
              لمائی = 3;
            }
            اكسر;
          }وإلا إذا( flag_zeropad ){
            s.z[0] = '9';
            s.iDP = 1000;
            s.n = 1;
          }وإلا{
            حافظہ_نقل(buf, "-Inf", 5);
            bufpt = buf;
            إذا( s.sign=='-' ){
              /* no-op */
            }وإلا إذا( flag_اگیتر ){
              buf[0] = flag_اگیتر;
            }وإلا{
              bufpt++;
            }
            لمائی = لڑی_لمبائی(bufpt);
            اكسر;
          }
        }
        إذا( s.sign=='-' ){
          إذا( flag_alternateform
           && !flag_اگیتر
           && xtype==etFLOAT
           && s.iDP<=i_گیڑا
          ){
            /* Suppress the minus sign if all of the following are true:
            **   *  The value displayed is zero
            **   *  The '#' flag is used
            **   *  The '+' flag is not used, and
            **   *  The format is %f
            */
            اگیتر = 0;
          }وإلا{
            اگیتر = '-';
          }
        }وإلا{
          اگیتر = flag_اگیتر;
        }

        قوت_نما = s.iDP-1;

        /*
        ** If the field type is etGENERIC, then convert to either etEXP
        ** or etFLOAT, as appropriate.
        */
        إذا( xtype==etGENERIC ){
          أكد( precision>0 );
          precision--;
          flag_rtz = !flag_alternateform;
          إذا( قوت_نما<-4 || قوت_نما>precision ){
            xtype = etEXP;
          }وإلا{
            precision = precision - قوت_نما;
            xtype = etFLOAT;
          }
        }وإلا{
          flag_rtz = flag_altform2;
        }
        إذا( xtype==etEXP ){
          e2 = 0;
        }وإلا{
          e2 = s.iDP - 1;
        }

        szBufNeeded = MAX(e2,0)+(i64)precision+(i64)چوڑائی+10;
        إذا( cThousand && e2>0 ) szBufNeeded += (e2+2)/3;
        إذا( szBufNeeded + pAccum->nChar >= pAccum->nAlloc ){
          إذا( pAccum->mxAlloc==0 && pAccum->accError==0 ){
            /* Unable to allocate space in pAccum, perhaps because it
            ** is coming from sqlite3_snprintf() or similar.  We'll have
            ** to render into temporary space and the memcpy() it over. */
            bufpt = sqlite3_malloc64(szBufNeeded);
            إذا( bufpt==0 ){
              sqlite3_Str_Accum_سیٹ_Error(pAccum, SQLITE_NOMEM);
              أرجع;
            }
            zExtra = bufpt;
          }وإلا إذا( sqlite3StrAccumEnlarge(pAccum, szBufNeeded)<szBufNeeded ){
            چوڑائی = لمائی = 0;
            اكسر;
          }وإلا{
            bufpt = pAccum->z_متن + pAccum->nChar;
          }
        }وإلا{
          bufpt = pAccum->z_متن + pAccum->nChar;
        }
        z_باہرلا = bufpt;

        flag_dp = (precision>0 ?1:0) | flag_alternateform | flag_altform2;
        /* The sign in front of the number */
        إذا( اگیتر ){
          *(bufpt++) = اگیتر;
        }
        /* Digits prior to the decimal point */
        j = 0;
        أكد( s.n>0 );
        إذا( e2<0 ){
          *(bufpt++) = '0';
        }وإلا إذا( cThousand ){
          لكل(; e2>=0; e2--){
            *(bufpt++) = j<s.n ? s.z[j++] : '0';
            إذا( (e2%3)==0 && e2>1 ) *(bufpt++) = ',';
          }
        }وإلا{
          j = e2+1;
          إذا( j>s.n ) j = s.n;
          حافظہ_نقل(bufpt, s.z, j);
          bufpt += j;
          e2 -= j;
          إذا( e2>=0 ){
            memset(bufpt, '0', e2+1);
            bufpt += e2+1;
            e2 = -1;
          }
        }
        /* The decimal point */
        إذا( flag_dp ){
          *(bufpt++) = '.';
        }
        /* "0" digits after the decimal point but before the first
        ** significant digit of the number */
        إذا( e2<(-1) && precision>0 ){
          صحيح nn = -1-e2;
          إذا( nn>precision ) nn = precision;
          memset(bufpt, '0', nn);
          bufpt += nn;
          precision -= nn;
        }
        /* Significant digits after the decimal point */
        إذا( precision>0 ){
          صحيح nn = s.n - j;
          إذا( NEVER(nn>precision) ) nn = precision;
          إذا( nn>0 ){
            حافظہ_نقل(bufpt, s.z+j, nn);
            bufpt += nn;
            precision -= nn;
          }
          إذا( precision>0 && !flag_rtz ){
            memset(bufpt, '0', precision);
            bufpt += precision;
          }
        }
        /* Remove trailing zeros and the "." if no digits follow the "." */
        إذا( flag_rtz && flag_dp ){
          طالما( bufpt[-1]=='0' ) *(--bufpt) = 0;
          أكد( bufpt>z_باہرلا );
          إذا( bufpt[-1]=='.' ){
            إذا( flag_altform2 ){
              *(bufpt++) = '0';
            }وإلا{
              *(--bufpt) = 0;
            }
          }
        }
        /* Add the "eNNN" suffix */
        إذا( xtype==etEXP ){
          قوت_نما = s.iDP - 1;
          *(bufpt++) = aDigits[infop->charset];
          إذا( قوت_نما<0 ){
            *(bufpt++) = '-'; قوت_نما = -قوت_نما;
          }وإلا{
            *(bufpt++) = '+';
          }
          إذا( قوت_نما>=100 ){
            *(bufpt++) = (حرف)((قوت_نما/100)+'0');        /* 100's digit */
            قوت_نما %= 100;
          }
          *(bufpt++) = (حرف)(قوت_نما/10+'0');             /* 10's digit */
          *(bufpt++) = (حرف)(قوت_نما%10+'0');             /* 1's digit */
        }

        لمائی = bufpt - z_باہرلا;
        أكد( لمائی <= szBufNeeded );
        إذا( لمائی<چوڑائی ){
          i64 nPad = چوڑائی - لمائی;
          إذا( flag_leftjustify ){
            memset(bufpt, ' ', nPad);
          }وإلا إذا( !flag_zeropad ){
            memmove(z_باہرلا+nPad, z_باہرلا, لمائی);
            memset(z_باہرلا, ' ', nPad);
          }وإلا{
            صحيح adj = اگیتر!=0;
            memmove(z_باہرلا+nPad+adj, z_باہرلا+adj, لمائی-adj);
            memset(z_باہرلا+adj, '0', nPad);
          }
          لمائی = چوڑائی;
        }

        إذا( zExtra==0 ){
          /* The result is being rendered directory into pAccum.  This
          ** is the common and fast case */
          أكد( pAccum->nChar + لمائی < SMXV(pAccum->nChar) );
          pAccum->nChar += لمائی;
          z_باہرلا[لمائی] = 0;
          استمر;
        }وإلا{
          /* We were unable to render directly into pAccum because we
          ** couldn't allocate sufficient memory.  We need to memcpy()
          ** the rendering (or some prefix thereof) into the output
          ** buffer. */
          bufpt[0] = 0;
          bufpt = zExtra;
          اكسر;
        }
      }
      حالة et_ناپ:
        إذا( !bArgList ){
          *(va_arg(ap,صحيح*)) = pAccum->nChar;
        }
        لمائی = چوڑائی = 0;
        اكسر;
      حالة etPERCENT:
        buf[0] = '%';
        bufpt = buf;
        لمائی = 1;
        اكسر;
      حالة etCHARX:
        إذا( bArgList ){
          bufpt = get_متن_Arg(pArgList);
          لمائی = 1;
          إذا( bufpt ){
            buf[0] = c = *(bufpt++);
            إذا( (c&0xc0)==0xc0 ){
              طالما( لمائی<4 && (bufpt[0]&0xc0)==0x80 ){
                buf[لمائی++] = *(bufpt++);
              }
            }
          }وإلا{
            buf[0] = 0;
          }
        }وإلا{
          بنانشانی صحيح ch = va_arg(ap,بنانشانی صحيح);
          لمائی = sqlite3AppendOneUtf8Character(buf, ch);
        }
        إذا( precision>1 ){
          i64 n_پہلوں = 1;
          چوڑائی -= precision-1;
          إذا( چوڑائی>1 && !flag_leftjustify ){
            sqlite3StrAppendchar64(pAccum, چوڑائی-1, ' ');
            چوڑائی = 0;
          }
          sqlite3StrAppend64(pAccum, buf, لمائی);
          precision--;
          طالما( precision > 1 ){
            i64 n_Copy_بائٹاں;
            إذا( n_پہلوں > precision-1 ) n_پہلوں = precision - 1;
            n_Copy_بائٹاں = لمائی*n_پہلوں;
            إذا( sqlite3StrAccumEnlargeIfNeeded(pAccum, n_Copy_بائٹاں) ){
              اكسر;
            }
            sqlite3_str_append(pAccum,
                 &pAccum->z_متن[pAccum->nChar-n_Copy_بائٹاں], n_Copy_بائٹاں);
            precision -= n_پہلوں;
            n_پہلوں *= 2;
          }
        }
        bufpt = buf;
        flag_altform2 = 1;
        اذهب adjust_چوڑائی_for_utf8;
      حالة etSTRING:
      حالة etDYNSTRING:
        إذا( bArgList ){
          bufpt = get_متن_Arg(pArgList);
          xtype = etSTRING;
        }وإلا{
          bufpt = va_arg(ap,حرف*);
        }
        إذا( bufpt==0 ){
          bufpt = "";
        }وإلا إذا( xtype==etDYNSTRING ){
          إذا( pAccum->nChar==0
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
            pAccum->nAlloc = sqlite3_Db_Malloc_ناپ(pAccum->db, bufpt);
            pAccum->nChar = 0x7fffffff & (صحيح)لڑی_لمبائی(bufpt);
            pAccum->printfFlags |= SQLITE_PRINTF_MALLOCED;
            لمائی = 0;
            اكسر;
          }
          zExtra = bufpt;
        }
        إذا( precision>=0 ){
          إذا( flag_altform2 ){
            /* Set length to the number of bytes needed in order to display
            ** precision characters */
            بنانشانی حرف *z = (بنانشانی حرف*)bufpt;
            طالما( precision-- > 0 && z[0] ){
              SQLITE_SKIP_UTF8(z);
            }
            لمائی = z - (بنانشانی حرف*)bufpt;
          }وإلا{
            لكل(لمائی=0; لمائی<precision && bufpt[لمائی]; لمائی++){}
          }
        }وإلا{
          لمائی = لڑی_لمبائی(bufpt);
        }
      adjust_چوڑائی_for_utf8:
        إذا( flag_altform2 && چوڑائی>0 ){
          /* Adjust width to account for extra bytes in UTF-8 characters */
          i64 ii = لمائی - 1;
          طالما( ii>=0 ) إذا( (bufpt[ii--] & 0xc0)==0x80 ) چوڑائی++;
        }
        اكسر;
      حالة etESCAPE_j:           /* %j: JSON string literal w/o "..." */
      حالة etESCAPE_J: {         /* %J: Generate a JSON string literal */
        حرف *escarg;
        i64 i, j, px, i_شروع;
        بنانشانی حرف ch;

        إذا( bArgList ){
          escarg = get_متن_Arg(pArgList);
        }وإلا{
          escarg = va_arg(ap,حرف*);
        }
        i_شروع = sqlite3_str_لمائی(pAccum);
        إذا( escarg==0 ){
          إذا( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "null", 4);
        }وإلا{
          إذا( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
          px = precision;
          testcase( px==0 );
          إذا( px<0 ){
            px = 0x7fffffff;
          }وإلا إذا( flag_altform2 ){
            /* Convert precision from code-points to bytes */
            لكل(i=0; i<px && escarg[i]; i++){
              إذا( (escarg[i]&0xc0)==0x80 ) px++;
            }
            إذا( i==px ){
              طالما( (escarg[px]&0xc0)==0x80 ) px++;
            }
          }
          لكل(i=j=0; i<px; i++){
            إذا( (ch = ((u8*)escarg)[i])<=0x1f || ch=='"' || ch=='\\' ){
              إذا( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
              j = i+1;
              إذا( ch==0 ) اكسر;
              sqlite3_str_appendchar(pAccum, 1, '\\');
              إذا( ch>0x1f ){
                sqlite3_str_appendchar(pAccum, 1, ch);
              }وإلا إذا( ((1u<<ch)&0x3700)!=0 ){
                ch = "btn?fr"[ch-8];
                sqlite3_str_appendchar(pAccum, 1, ch);
              }وإلا{
                sqlite3_str_append(pAccum, "u00", 3);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch>>4]);
                sqlite3_str_appendchar(pAccum, 1, aHex[ch&0xf]);
              }
            }
          }
          إذا( j<i ) sqlite3StrAppend64(pAccum, &escarg[j], i-j);
          إذا( xtype==etESCAPE_J ) sqlite3_str_append(pAccum, "\"", 1);
        }
        إذا( چوڑائی>0 && sqlite3_str_errcode(pAccum)==SQLITE_OK ){
          sqlite3_int64 n = sqlite3_str_لمائی(pAccum) - i_شروع;
          sqlite3_int64 len = n;
          حرف *zz;
          إذا( flag_altform2 && n>0 ){
            zz = sqlite3_str_قدر(pAccum);
            لكل(i=i_شروع; zz[i]; i++){
              إذا( (zz[i]&0xc0)==0x80 ) len--;
            }
          }
          إذا( چوڑائی>len ){
            sqlite3_int64 sp = چوڑائی-len;
            أكد( sp>0 && sp<0x7fffffff );
            sqlite3StrAppendchar64(pAccum, (صحيح)sp, ' ');
            إذا( !flag_leftjustify
             && n>0
             && sqlite3_str_errcode(pAccum)==0
            ){
              zz = sqlite3_str_قدر(pAccum);
              zz += i_شروع;
              memmove(zz+sp, zz, n);
              memset(zz, ' ', sp);
            }
          }
        }
        استمر;
      }
      حالة etESCAPE_q:          /* %q: Escape ' characters */
      حالة etESCAPE_Q:          /* %Q: Escape ' and enclose in '...' */
      حالة etESCAPE_w: {        /* %w: Escape " characters */
        i64 i, j, k, n;
        صحيح needQuote = 0;
        حرف ch;
        حرف *escarg;
        حرف q;

        إذا( bArgList ){
          escarg = get_متن_Arg(pArgList);
        }وإلا{
          escarg = va_arg(ap,حرف*);
        }
        إذا( escarg==0 ){
          escarg = (xtype==etESCAPE_Q ? "NULL" : "(NULL)");
        }وإلا إذا( xtype==etESCAPE_Q ){
          needQuote = 1;
        }
        إذا( xtype==etESCAPE_w ){
          q = '"';
          flag_alternateform = 0;
        }وإلا{
          q = '\'';
        }
        /* For %q, %Q, and %w, the precision is the number of bytes (or
        ** characters if the ! flags is present) to use from the input.
        ** Because of the extra quoting characters inserted, the number
        ** of output characters may be larger than the precision.
        */
        k = precision;
        لكل(i=n=0; k!=0 && (ch=escarg[i])!=0; i++, k--){
          إذا( ch==q )  n++;
          إذا( flag_altform2 && (ch&0xc0)==0xc0 ){
            طالما( (escarg[i+1]&0xc0)==0x80 ){ i++; }
          }
        }
        إذا( flag_alternateform ){
          /* For %#q, do unistr()-style backslash escapes for
          ** all control characters, and for backslash itself.
          ** For %#Q, do the same but only if there is at least
          ** one control character. */
          i64 n_پٹھ = 0;
          i64 nCtrl = 0;
          لكل(k=0; k<i; k++){
            إذا( escarg[k]=='\\' ){
              n_پٹھ++;
            }وإلا إذا( ((u8*)escarg)[k]<=0x1f ){
              nCtrl++;
            }
          }
          إذا( nCtrl || xtype==etESCAPE_q ){
            n += n_پٹھ + 5*nCtrl;
            إذا( xtype==etESCAPE_Q ){
              n += 10;
              needQuote = 2;
            }
          }وإلا{
            flag_alternateform = 0;
          }
        }
        n += i + 3;
        إذا( n>etBUFSIZE ){
          bufpt = zExtra = printf_عارضی_Buf(pAccum, n);
          إذا( bufpt==0 ) أرجع;
        }وإلا{
          bufpt = buf;
        }
        j = 0;
        إذا( needQuote ){
          إذا( needQuote==2 ){
            حافظہ_نقل(&bufpt[j], "unistr('", 8);
            j += 8;
          }وإلا{
            bufpt[j++] = '\'';
          }
        }
        k = i;
        إذا( flag_alternateform ){
          لكل(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            إذا( ch==q ){
              bufpt[j++] = ch;
            }وإلا إذا( ch=='\\' ){
              bufpt[j++] = '\\';
            }وإلا إذا( ((بنانشانی حرف)ch)<=0x1f ){
              bufpt[j-1] = '\\';
              bufpt[j++] = 'u';
              bufpt[j++] = '0';
              bufpt[j++] = '0';
              bufpt[j++] = ch>=0x10 ? '1' : '0';
              bufpt[j++] = aHex[ch&0xf];
            }
          }
        }وإلا{
          لكل(i=0; i<k; i++){
            bufpt[j++] = ch = escarg[i];
            إذا( ch==q ) bufpt[j++] = ch;
          }
        }
        إذا( needQuote ){
          bufpt[j++] = '\'';
          إذا( needQuote==2 ) bufpt[j++] = ')';
        }
        bufpt[j] = 0;
        لمائی = j;
        اذهب adjust_چوڑائی_for_utf8;
      }
      حالة etTOKEN: {
        إذا( (pAccum->printfFlags & SQLITE_PRINTF_اندرونی)==0 ) أرجع;
        إذا( flag_alternateform ){
          /* %#T means an Expr pointer that uses Expr.u.zToken */
          اظہاریہ *p_اظہاریہ = va_arg(ap,اظہاریہ*);
          إذا( ALWAYS(p_اظہاریہ) && ALWAYS(!اظہاریہ_Has_Property(p_اظہاریہ,EP_Int_قدر)) ){
            sqlite3_str_appendall(pAccum, (ثابت حرف*)p_اظہاریہ->u.zToken);
            sqlite3_ریکارڈ_Error_وتھ_دا_اظہاریہ(pAccum->db, p_اظہاریہ);
          }
        }وإلا{
          /* %T means a Token pointer */
          Token *pToken = va_arg(ap, Token*);
          أكد( bArgList==0 );
          إذا( pToken && pToken->n ){
            sqlite3_str_append(pAccum, (ثابت حرف*)pToken->z, pToken->n);
            sqlite3_ریکارڈ_Error_Byte_وتھ(pAccum->db, pToken->z);
          }
        }
        لمائی = چوڑائی = 0;
        اكسر;
      }
      حالة etSRCITEM: {
        Src_شے *p_شے;
        إذا( (pAccum->printfFlags & SQLITE_PRINTF_اندرونی)==0 ) أرجع;
        p_شے = va_arg(ap, Src_شے*);
        أكد( bArgList==0 );
        إذا( p_شے->zAlias && !flag_altform2 ){
          sqlite3_str_appendall(pAccum, p_شے->zAlias);
        }وإلا إذا( p_شے->zName ){
          إذا( p_شے->fg.fixedSchema==0
           && p_شے->fg.isSubquery==0
           && p_شے->u4.zDatabase!=0
          ){
            sqlite3_str_appendall(pAccum, p_شے->u4.zDatabase);
            sqlite3_str_append(pAccum, ".", 1);
          }
          sqlite3_str_appendall(pAccum, p_شے->zName);
        }وإلا إذا( p_شے->zAlias ){
          sqlite3_str_appendall(pAccum, p_شے->zAlias);
        }وإلا إذا( ALWAYS(p_شے->fg.isSubquery) ){/* Because of tag-20240424-1 */
          Select *pSel = p_شے->u4.pSubq->pSelect;
          أكد( pSel!=0 ); 
          إذا( pSel->selFlags & SF_NestedFrom ){
            sqlite3_str_appendf(pAccum, "(join-%u)", pSel->selId);
          }وإلا إذا( pSel->selFlags & SF_Multi_قدر ){
            أكد( !p_شے->fg.isTabFunc && !p_شے->fg.is_اشاریہ_دار_By );
            sqlite3_str_appendf(pAccum, "%u-ROW VALUES CLAUSE",
                                p_شے->u1.n_صف);
          }وإلا{
            sqlite3_str_appendf(pAccum, "(subquery-%u)", pSel->selId);
          }
        }
        لمائی = چوڑائی = 0;
        اكسر;
      }
      افتراضي: {
        أكد( xtype==etINVALID );
        أرجع;
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
    چوڑائی -= لمائی;
    إذا( چوڑائی>0 ){
      إذا( !flag_leftjustify ) sqlite3StrAppendchar64(pAccum, چوڑائی, ' ');
      sqlite3StrAppend64(pAccum, bufpt, لمائی);
      إذا( flag_leftjustify ) sqlite3StrAppendchar64(pAccum, چوڑائی, ' ');
    }وإلا{
      sqlite3StrAppend64(pAccum, bufpt, لمائی);
    }

    إذا( zExtra ){
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
فراغ sqlite3_ریکارڈ_Error_Byte_وتھ(sqlite3 *db, ثابت حرف *z){
  ثابت Parse *pParse;
  ثابت حرف *z_متن;
  ثابت حرف *z_سرا;
  أكد( z!=0 );
  إذا( NEVER(db==0) ) أرجع;
  إذا( db->err_Byte_وتھ!=(-2) ) أرجع;
  pParse = db->pParse;
  إذا( NEVER(pParse==0) ) أرجع;
  z_متن =pParse->z_پوچھ;
  إذا( NEVER(z_متن==0) ) أرجع;
  z_سرا = &z_متن[لڑی_لمبائی(z_متن)];
  إذا( SQLITE_WITHIN(z,z_متن,z_سرا) ){
    db->err_Byte_وتھ = (صحيح)(z-z_متن);
  }
}

/*
** If pExpr has a byte offset for the start of a token, record that as
** as the error offset.
*/
فراغ sqlite3_ریکارڈ_Error_وتھ_دا_اظہاریہ(sqlite3 *db, ثابت اظہاریہ *p_اظہاریہ){
  طالما( p_اظہاریہ ){
    إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_Reduced|EP_TokenOnly) ) أرجع;
    إذا( اظہاریہ_Has_Property(p_اظہاریہ,EP_OuterON|EP_InnerON) || p_اظہاریہ->w.iOfst<=0 ){
      p_اظہاریہ = p_اظہاریہ->p_بایاں;
    }وإلا{
      اكسر;
    }
  }
  إذا( p_اظہاریہ==0 ) أرجع;
  إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_FromDDL) ) أرجع;
  db->err_Byte_وتھ = p_اظہاریہ->w.iOfst;
}

/*
** Enlarge the memory allocation on a StrAccum object so that it is
** able to accept at least N more bytes of text.
**
** Return the number of bytes of text that StrAccum is able to accept
** after the attempted enlargement.  The value returned might be zero.
*/
صحيح sqlite3StrAccumEnlarge(StrAccum *p, i64 N){
  حرف *zNew;
  أكد( p->nChar+N >= p->nAlloc ); /* Only called if really needed */
  إذا( p->accError ){
    testcase(p->accError==SQLITE_TOOBIG);
    testcase(p->accError==SQLITE_NOMEM);
    أرجع 0;
  }
  إذا( p->mxAlloc==0 ){
    sqlite3_Str_Accum_سیٹ_Error(p, SQLITE_TOOBIG);
    أرجع p->nAlloc - p->nChar - 1;
  }وإلا{
    حرف *zOld = isMalloced(p) ? p->z_متن : 0;
    i64 szNew = p->nChar + N + 1;
    إذا( szNew+p->nChar<=p->mxAlloc ){
      /* Force exponential buffer size growth as long as it does not overflow,
      ** to avoid having to call this routine too often */
      szNew += p->nChar;
    }
    إذا( szNew > p->mxAlloc ){
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_سیٹ_Error(p, SQLITE_TOOBIG);
      أرجع 0;
    }وإلا{
      p->nAlloc = (صحيح)szNew;
    }
    إذا( p->db ){
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    }وإلا{
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    إذا( zNew ){
      أكد( p->z_متن!=0 || p->nChar==0 );
      إذا( !isMalloced(p) && p->nChar>0 ) حافظہ_نقل(zNew, p->z_متن, p->nChar);
      p->z_متن = zNew;
      p->nAlloc = sqlite3_Db_Malloc_ناپ(p->db, zNew);
      p->printfFlags |= SQLITE_PRINTF_MALLOCED;
    }وإلا{
      sqlite3_str_reset(p);
      sqlite3_Str_Accum_سیٹ_Error(p, SQLITE_NOMEM);
      أرجع 0;
    }
  }
  أكد( N>=0 && N<=0x7fffffff );
  أرجع (صحيح)N;
}

صحيح sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N){
  إذا( N + p->nChar >= p->nAlloc ){
    sqlite3StrAccumEnlarge(p, N);
  }
  أرجع p->accError;
}

/*
** Append N copies of character c to the given string buffer.
*/
فراغ sqlite3_str_appendchar(sqlite3_str *p, صحيح N, حرف c){
  testcase( p->nChar + (i64)N > 0x7fffffff );
  إذا( p->nChar+(i64)N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    أرجع;
  }
  طالما( (N--)>0 ) p->z_متن[p->nChar++] = c;
}
ساكن فراغ sqlite3StrAppendchar64(sqlite3_str *p, i64 N, حرف c){
  testcase( p->nChar + N > 0x7fffffff );
  إذا( p->nChar+N >= p->nAlloc && (N = sqlite3StrAccumEnlarge(p, N))<=0 ){
    أرجع;
  }
  طالما( (N--)>0 ) p->z_متن[p->nChar++] = c;
}

/*
** The StrAccum "p" is not large enough to accept N new bytes of z[].
** So enlarge if first, then do the append.
**
** This is a helper routine to sqlite3_str_append() that does special-case
** work (enlarging the buffer) using tail recursion, so that the
** sqlite3_str_append() routine can use fast calling semantics.
*/
ساكن فراغ SQLITE_NOINLINE enlargeAndAppend(StrAccum *p, ثابت حرف *z, صحيح N){
  N = sqlite3StrAccumEnlarge(p, N);
  أكد( z!=0 || N==0 );
  إذا( N>0 ){
    حافظہ_نقل(&p->z_متن[p->nChar], z, N);
    p->nChar += N;
  }
}

/*
** Append N bytes of text from z to the StrAccum object.  Increase the
** size of the memory allocation for StrAccum if necessary.
*/
فراغ sqlite3_str_append(sqlite3_str *p, ثابت حرف *z, صحيح N){
  أكد( z!=0 || N==0 );
  أكد( p->z_متن!=0 || p->nChar==0 || p->accError );
  أكد( N>=0 );
  أكد( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  إذا( p->nChar+N >= p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }وإلا إذا( N ){
    أكد( p->z_متن );
    p->nChar += N;
    حافظہ_نقل(&p->z_متن[p->nChar-N], z, N);
  }
}
ساكن فراغ sqlite3StrAppend64(sqlite3_str *p, ثابت حرف *z, i64 N){
  أكد( z!=0 || N==0 );
  أكد( p->z_متن!=0 || p->nChar==0 || p->accError );
  أكد( N>=0 );
  أكد( p->accError==0 || p->nAlloc==0 || p->mxAlloc==0 );
  إذا( p->nChar+N >= (i64)p->nAlloc ){
    enlargeAndAppend(p,z,N);
  }وإلا إذا( N ){
    أكد( p->z_متن );
    p->nChar += N;
    حافظہ_نقل(&p->z_متن[p->nChar-N], z, N);
  }
}


/*
** Append the complete text of zero-terminated string z[] to the p string.
*/
فراغ sqlite3_str_appendall(sqlite3_str *p, ثابت حرف *z){
  sqlite3_str_append(p, z, sqlite3Strlen30(z));
}


/*
** Finish off a string by making sure it is zero-terminated.
** Return a pointer to the resulting string.  Return a NULL
** pointer if any kind of error was encountered.
*/
ساكن SQLITE_NOINLINE حرف *str_Accum_انت_Realloc(StrAccum *p){
  حرف *z_متن;
  أكد( p->mxAlloc>0 && !isMalloced(p) );
  z_متن = sqlite3DbMallocRaw(p->db, 1+(u64)p->nChar );
  إذا( z_متن ){
    حافظہ_نقل(z_متن, p->z_متن, p->nChar+1);
    p->printfFlags |= SQLITE_PRINTF_MALLOCED;
  }وإلا{
    sqlite3_Str_Accum_سیٹ_Error(p, SQLITE_NOMEM);
  }
  p->z_متن = z_متن;
  أرجع z_متن;
}
حرف *sqlite3_Str_Accum_انت(StrAccum *p){
  إذا( p->z_متن ){
    p->z_متن[p->nChar] = 0;
    إذا( p->mxAlloc>0 && !isMalloced(p) ){
      أرجع str_Accum_انت_Realloc(p);
    }
  }
  أرجع p->z_متن;
}

/* Finalize a string created using sqlite3_str_new().
*/
حرف *sqlite3_str_انت(sqlite3_str *p){
  حرف *z;
  إذا( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
    z = sqlite3_Str_Accum_انت(p);
    sqlite3_free(p);
  }وإلا{
    z = 0;
  }
  أرجع z;
}

/* Return any error code associated with p */
صحيح sqlite3_str_errcode(sqlite3_str *p){
  أرجع p ? p->accError : SQLITE_NOMEM;
}

/* Return the current length of p in bytes */
صحيح sqlite3_str_لمائی(sqlite3_str *p){
  أرجع p ? p->nChar : 0;
}

/* Truncate the text of the string to be no more than N bytes. */
فراغ sqlite3_str_truncate(sqlite3_str *p, صحيح N){
  إذا( p!=0 && N>=0 && (u32)N<p->nChar ){
    p->nChar = N;
    p->z_متن[p->nChar] = 0;
  }
}

/* Return the current value for p */
حرف *sqlite3_str_قدر(sqlite3_str *p){
  إذا( p==0 || p->nChar==0 ) أرجع 0;
  p->z_متن[p->nChar] = 0;
  أرجع p->z_متن;
}

/*
** Reset an StrAccum string.  Reclaim all malloced memory.
*/
فراغ sqlite3_str_reset(StrAccum *p){
  إذا( isMalloced(p) ){
    sqlite3DbFree(p->db, p->z_متن);
    p->printfFlags &= ~SQLITE_PRINTF_MALLOCED;
  }وإلا إذا( p==(sqlite3_str*)&sqlite3OomStr ){
    أرجع;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->z_متن = 0;
}

/*
** Destroy a dynamically allocate sqlite3_str object and all
** of its content, all in one call.
*/
فراغ sqlite3_str_free(sqlite3_str *p){
  إذا( p!=0 && p!=(sqlite3_str*)&sqlite3OomStr ){
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
فراغ sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, حرف *z_ادھار, صحيح n, صحيح mx){
  p->z_متن = z_ادھار;
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
  إذا( p ){
    sqlite3StrAccumInit(p, 0, 0, 0,
            db ? db->a_حد_بندی[SQLITE_حد_بندی_لمائی] : SQLITE_MAX_لمائی);
  }وإلا{
    p = (sqlite3_str*)&sqlite3OomStr;
  }
  أرجع p;
}

/*
** Print into memory obtained from sqliteMalloc().  Use the internal
** %-conversion extensions.
*/
حرف *sqlite3VMPrintf(sqlite3 *db, ثابت حرف *zFormat, va_list ap){
  حرف *z;
  حرف z_ادھار[SQLITE_PRINT_BUF_ناپ];
  StrAccum acc;
  أكد( db!=0 );
  sqlite3StrAccumInit(&acc, db, z_ادھار, حجم(z_ادھار),
                      db->a_حد_بندی[SQLITE_حد_بندی_لمائی]);
  acc.printfFlags = SQLITE_PRINTF_اندرونی;
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_انت(&acc);
  إذا( acc.accError==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  أرجع z;
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
  أرجع z;
}

/*
** Print into memory obtained from sqlite3_malloc().  Omit the internal
** %-conversion extensions.
*/
حرف *sqlite3_vmprintf(ثابت حرف *zFormat, va_list ap){
  حرف *z;
  حرف z_ادھار[SQLITE_PRINT_BUF_ناپ];
  StrAccum acc;

#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR  
  إذا( zFormat==0 ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جے_انت
#جے_بنا_تعریف SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جے_انت
  sqlite3StrAccumInit(&acc, 0, z_ادھار, حجم(z_ادھار), SQLITE_MAX_لمائی);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3_Str_Accum_انت(&acc);
  أرجع z;
}

/*
** Print into memory obtained from sqlite3_malloc()().  Omit the internal
** %-conversion extensions.
*/
حرف *sqlite3_mprintf(ثابت حرف *zFormat, ...){
  va_list ap;
  حرف *z;
#جے_بنا_تعریف SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جے_انت
  va_start(ap, zFormat);
  z = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  أرجع z;
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
حرف *sqlite3_vsnprintf(صحيح n, حرف *zBuf, ثابت حرف *zFormat, va_list ap){
  StrAccum acc;
  إذا( n<=0 ) أرجع zBuf;
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( zBuf==0 || zFormat==0 ) {
    (فراغ)SQLITE_MISUSE_BKPT;
    إذا( zBuf ) zBuf[0] = 0;
    أرجع zBuf;
  }
#جے_انت
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  أرجع zBuf;
}
حرف *sqlite3_snprintf(صحيح n, حرف *zBuf, ثابت حرف *zFormat, ...){
  StrAccum acc;
  va_list ap;
  إذا( n<=0 ) أرجع zBuf;
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( zBuf==0 || zFormat==0 ) {
    (فراغ)SQLITE_MISUSE_BKPT;
    إذا( zBuf ) zBuf[0] = 0;
    أرجع zBuf;
  }
#جے_انت
  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  zBuf[acc.nChar] = 0;
  أرجع zBuf;
}

/* Maximum size of an sqlite3_log() message. */
#إذا defined(SQLITE_MAX_LOG_سنیہا) 
  /* Leave the definition as supplied */
#وإلاإذا SQLITE_PRINT_BUF_ناپ*10>10000
# define SQLITE_MAX_LOG_سنیہا 10000
#وإلا
# define SQLITE_MAX_LOG_سنیہا (SQLITE_PRINT_BUF_ناپ*10)
#جے_انت

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
ساكن فراغ render_Log_پیغام(صحيح i_Err_کوڈ, ثابت حرف *zFormat, va_list ap){
  StrAccum acc;                          /* String accumulator */
  حرف z_پیغام[SQLITE_MAX_LOG_سنیہا];     /* Complete log message */

  sqlite3StrAccumInit(&acc, 0, z_پیغام, حجم(z_پیغام), 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  sqlite3GlobalConfig.xLog(sqlite3GlobalConfig.pLogArg, i_Err_کوڈ,
                           sqlite3_Str_Accum_انت(&acc));
}

/*
** Format and write a message to the log if logging is enabled.
*/
فراغ sqlite3_log(صحيح i_Err_کوڈ, ثابت حرف *zFormat, ...){
  va_list ap;                             /* Vararg list */
  إذا( sqlite3GlobalConfig.xLog ){
    va_start(ap, zFormat);
    render_Log_پیغام(i_Err_کوڈ, zFormat, ap);
    va_end(ap);
  }
}

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
/*
** A version of printf() that understands %lld.  Used for debugging.
** The printf() built into some versions of windows does not understand %lld
** and segfaults if you give it a long long int.
*/
فراغ sqlite3DebugPrintf(ثابت حرف *zFormat, ...){
  va_list ap;
  StrAccum acc;
  حرف zBuf[SQLITE_PRINT_BUF_ناپ*10];
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  va_start(ap,zFormat);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  va_end(ap);
  sqlite3_Str_Accum_انت(&acc);
#جے_تعریف_شدہ SQLITE_OS_TRACE_PROC
  {
    خارجي فراغ SQLITE_OS_TRACE_PROC(ثابت حرف *zBuf, صحيح nBuf);
    SQLITE_OS_TRACE_PROC(zBuf, حجم(zBuf));
  }
#وإلا
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
#جے_انت
}
#جے_انت


/*
** variable-argument wrapper around sqlite3_str_vappendf(). The bFlags argument
** can contain the bit SQLITE_PRINTF_INTERNAL enable internal formats.
*/
فراغ sqlite3_str_appendf(StrAccum *p, ثابت حرف *zFormat, ...){
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
  أرجع z;
}

/*
** Decrease the reference count by one.  Free the string when the
** reference count reaches zero.
*/
فراغ sqlite3RCStrUnref(فراغ *z){
  RCStr *p = (RCStr*)z;
  أكد( p!=0 );
  p--;
  أكد( p->nRCRef>0 );
  إذا( p->nRCRef>=2 ){
    p->nRCRef--;
  }وإلا{
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
  إذا( p==0 ) أرجع 0;
  p->nRCRef = 1;
  أرجع (حرف*)&p[1];
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
  إذا( pNew==0 ){
    sqlite3_free(p);
    أرجع 0;
  }وإلا{
    أرجع (حرف*)&pNew[1];
  }
}
