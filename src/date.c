/*
** 2003 October 31
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C functions that implement date and time
** functions for SQLite.  
**
** There is only one exported symbol in this file - the function
** sqlite3RegisterDateTimeFunctions() found at the bottom of the file.
** All other code has file scope.
**
** SQLite processes all times and dates as julian day numbers.  The
** dates and times are stored as the number of days since noon
** in Greenwich on November 24, 4714 B.C. according to the Gregorian
** calendar system. 
**
** 1970-01-01 00:00:00 is JD 2440587.5
** 2000-01-01 00:00:00 is JD 2451544.5
**
** This implementation requires years to be expressed as a 4-digit number
** which means that only dates between 0000-01-01 and 9999-12-31 can
** be represented, even though julian day numbers allow a much wider
** range of dates.
**
** The Gregorian calendar system is used for all dates and times,
** even those that predate the Gregorian calendar.  Historians usually
** use the julian calendar for dates prior to 1582-10-15 and for some
** dates afterwards, depending on locale.  Beware of this difference.
**
** The conversion algorithms are implemented based on descriptions
** in the following text:
**
**      Jean Meeus
**      Astronomical Algorithms, 2nd Edition, 1998
**      ISBN 0-943396-61-1
**      Willmann-Bell, Inc
**      Richmond, Virginia (USA)
*/
#شامل "sqliteInt.h"
#شامل <stdlib.h>
#شامل <assert.h>
#شامل <time.h>

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DATETIME_FUNCS

/*
** The MSVC CRT on Windows CE may not have a localtime() function.
** So declare a substitute.  The substitute function itself is
** defined in "os_win.c".
*/
#اگر !defined(SQLITE_OMIT_LOCALTIME) && defined(_WIN32_WCE) && \
    (!defined(SQLITE_MSVC_LOCALTIME_API) || !SQLITE_MSVC_LOCALTIME_API)
ساخت tm *__cdecl localtime(ثابت وقت_t *);
#اگر_ختم

/*
** A structure for holding a single date and time.
*/
قسمتعریف ساخت Date_وقت Date_وقت;
ساخت Date_وقت {
  sqlite3_int64 iJD;  /* The julian day number times 86400000 */
  صحیح Y, M, D;        /* Year, month, and day */
  صحیح h, m;           /* Hour and minutes */
  صحیح tz;             /* Timezone offset in minutes */
  دوہرا s;           /* Seconds */
  حرف validJD;       /* True (1) if iJD is valid */
  حرف validYMD;      /* True (1) if Y,M,D are valid */
  حرف validHMS;      /* True (1) if h,m,s are valid */
  حرف nFloor;            /* Days to implement "floor" */
  غیرعلامتی rawS      : 1; /* Raw numeric value stored in s */
  غیرعلامتی isError   : 1; /* An overflow has occurred */
  غیرعلامتی useSubsec : 1; /* Display subsecond precision */
  غیرعلامتی isUtc     : 1; /* Time is known to be UTC */
  غیرعلامتی isLocal   : 1; /* Time is known to be localtime */
};


/*
** Convert zDate into one or more integers according to the conversion
** specifier zFormat.
**
** zFormat[] contains 4 characters for each integer converted, except for
** the last integer which is specified by three characters.  The meaning
** of a four-character format specifiers ABCD is:
**
**    A:   number of digits to convert.  Always "2" or "4".
**    B:   minimum value.  Always "0" or "1".
**    C:   maximum value, decoded as:
**           a:  12
**           b:  14
**           c:  24
**           d:  31
**           e:  59
**           f:  9999
**    D:   the separator character, or \000 to indicate this is the
**         last number to convert.
**
** Example:  To translate an ISO-8601 date YYYY-MM-DD, the format would
** be "40f-21a-20c".  The "40f-" indicates the 4-digit year followed by "-".
** The "21a-" indicates the 2-digit month followed by "-".  The "20c" indicates
** the 2-digit day which is the last integer in the set.
**
** The function returns the number of successful conversions.
*/
ساکن صحیح getDigits(ثابت حرف *zDate, ثابت حرف *zFormat, ...){
  /* The aMx[] array translates the 3rd character of each format
  ** spec into a max size:    a   b   c   d   e      f */
  ساکن ثابت u16 aMx[] = { 12, 14, 24, 31, 59, 14712 };
  va_list ap;
  صحیح cnt = 0;
  حرف اگلا_C;
  va_start(ap, zFormat);
  کرو{
    حرف N = zFormat[0] - '0';
    حرف min = zFormat[1] - '0';
    صحیح val = 0;
    u16 max;

    أكد( zFormat[2]>='a' && zFormat[2]<='f' );
    max = aMx[zFormat[2] - 'a'];
    اگلا_C = zFormat[3];
    val = 0;
    جب( N-- ){
      اگر( !sqlite3Isdigit(*zDate) ){
        جاؤ انتہا_get_Digits;
      }
      val = val*10 + *zDate - '0';
      zDate++;
    }
    اگر( val<(صحیح)min || val>(صحیح)max || (اگلا_C!=0 && اگلا_C!=*zDate) ){
      جاؤ انتہا_get_Digits;
    }
    *va_arg(ap,صحیح*) = val;
    zDate++;
    cnt++;
    zFormat += 4;
  }جب( اگلا_C );
انتہا_get_Digits:
  va_end(ap);
  واپس cnt;
}

/*
** Parse a timezone extension on the end of a date-time.
** The extension is of the form:
**
**        (+/-)HH:MM
**
** Or the "zulu" notation:
**
**        Z
**
** If the parse is successful, write the number of minutes
** of change in p->tz and return 0.  If a parser error occurs,
** return non-zero.
**
** A missing specifier is not considered an error.
*/
ساکن صحیح parseTimezone(ثابت حرف *zDate, Date_وقت *p){
  صحیح sgn = 0;
  صحیح nHr, nMn;
  صحیح c;
  جب( sqlite3Isspace(*zDate) ){ zDate++; }
  p->tz = 0;
  c = *zDate;
  اگر( c=='-' ){
    sgn = -1;
  }ورنہ اگر( c=='+' ){
    sgn = +1;
  }ورنہ اگر( c=='Z' || c=='z' ){
    zDate++;
    p->isLocal = 0;
    p->isUtc = 1;
    جاؤ zulu_وقت;
  }ورنہ{
    واپس c!=0;
  }
  zDate++;
  اگر( getDigits(zDate, "20b:20e", &nHr, &nMn)!=2 ){
    واپس 1;
  }
  zDate += 5;
  p->tz = sgn*(nMn + nHr*60);
  اگر( p->tz==0 ){   /* Forum post 2025-09-17T10:12:14z */
    p->isLocal = 0;
    p->isUtc = 1;
  }
zulu_وقت:
  جب( sqlite3Isspace(*zDate) ){ zDate++; }
  واپس *zDate!=0;
}

/*
** Parse times of the form HH:MM or HH:MM:SS or HH:MM:SS.FFFF.
** The HH, MM, and SS must each be exactly 2 digits.  The
** fractional seconds FFFF can be one or more digits.
**
** Return 1 if there is a parsing error and 0 on success.
*/
ساکن صحیح parseHhMmSs(ثابت حرف *zDate, Date_وقت *p){
  صحیح h, m, s;
  دوہرا ms = 0.0;
  اگر( getDigits(zDate, "20c:20e", &h, &m)!=2 ){
    واپس 1;
  }
  zDate += 5;
  اگر( *zDate==':' ){
    zDate++;
    اگر( getDigits(zDate, "20e", &s)!=1 ){
      واپس 1;
    }
    zDate += 2;
    اگر( *zDate=='.' && sqlite3Isdigit(zDate[1]) ){
      دوہرا rScale = 1.0;
      zDate++;
      جب( sqlite3Isdigit(*zDate) ){
        ms = ms*10.0 + *zDate - '0';
        rScale *= 10.0;
        zDate++;
      }
      ms /= rScale;
      /* Truncate to avoid problems with sub-milliseconds
      ** rounding. https://sqlite.org/forum/forumpost/766a2c9231 */
      اگر( ms>0.999 ) ms = 0.999;
    }
  }ورنہ{
    s = 0;
  }
  اگر( h==24 && (m!=0 || s!=0 || ms!=0.0) ) واپس 1;
  p->validJD = 0;
  p->rawS = 0;
  p->validHMS = 1;
  p->h = h;
  p->m = m;
  p->s = s + ms;
  اگر( parseTimezone(zDate, p) ) واپس 1;
  واپس 0;
}

/*
** Put the DateTime object into its error state.
*/
ساکن خالی datetimeError(Date_وقت *p){
  memset(p, 0, حجم(*p));
  p->isError = 1;
}

/*
** Convert from YYYY-MM-DD HH:MM:SS to julian day.  We always assume
** that the YYYY-MM-DD is according to the Gregorian calendar.
**
** Reference:  Meeus page 61
*/
ساکن خالی computeJD(Date_وقت *p){
  صحیح Y, M, D, A, B, X1, X2;

  اگر( p->validJD ) واپس;
  اگر( p->validYMD ){
    Y = p->Y;
    M = p->M;
    D = p->D;
  }ورنہ{
    Y = 2000;  /* If no YMD specified, assume 2000-Jan-01 */
    M = 1;
    D = 1;
    اگر( p->validHMS && p->h==24 ){
      p->h = 0;
      D = 2;
    }
  }
  اگر( Y<-4713 || Y>9999 || p->rawS ){
    datetimeError(p);
    واپس;
  }
  اگر( M<=2 ){
    Y--;
    M += 12;
  }
  A = (Y+4800)/100;
  B = 38 - A + (A/4);
  X1 = 36525*(Y+4716)/100;
  X2 = 306001*(M+1)/10000;
  p->iJD = (sqlite3_int64)((X1 + X2 + D + B - 1524.5 ) * 86400000);
  p->validJD = 1;
  اگر( p->validHMS ){
    p->iJD += p->h*3600000 + p->m*60000 + (sqlite3_int64)(p->s*1000 + 0.5);
    اگر( p->tz ){
      p->iJD -= p->tz*60000;
      p->validYMD = 0;
      p->validHMS = 0;
      p->tz = 0;
      p->isUtc = 1;
      p->isLocal = 0;
    }
  }
}

/*
** Return the number of dates in a specific month.
*/
ساکن صحیح days_کا_Month(صحیح Y, صحیح M){
  اگر( (1<<M) & 0x15aa ){
    واپس 31;
  }ورنہ اگر( M!=2 ){
    واپس 30;
  }ورنہ اگر( Y%4==0 && (Y%100!=0 || Y%400==0) ){
    واپس 29;
  }ورنہ{
    واپس 28;
  }
}

/*
** Given the YYYY-MM-DD information current in p, determine if there
** is day-of-month overflow and set nFloor to the number of days that
** would need to be subtracted from the date in order to bring the
** date back to the end of the month.
*/
ساکن خالی computeFloor(Date_وقت *p){
  صحیح mxD;         /* Maximum day number for the current month */
  أكد( p->validYMD || p->isError );
  أكد( p->D>=0 && p->D<=32 );
  أكد( p->M>=0 && p->M<=12 );
  mxD = days_کا_Month(p->Y, p->M);
  p->nFloor = p->D>mxD ? p->D - mxD : 0;
}

/*
** Parse dates of the form
**
**     YYYY-MM-DD HH:MM:SS.FFF
**     YYYY-MM-DD HH:MM:SS
**     YYYY-MM-DD HH:MM
**     YYYY-MM-DD
**
** Write the result into the DateTime structure and return 0
** on success and 1 if the input string is not a well-formed
** date.
*/
ساکن صحیح parseYyyyMmDd(ثابت حرف *zDate, Date_وقت *p){
  صحیح Y, M, D, neg;

  اگر( zDate[0]=='-' ){
    zDate++;
    neg = 1;
  }ورنہ{
    neg = 0;
  }
  اگر( getDigits(zDate, "40f-21a-21d", &Y, &M, &D)!=3 ){
    واپس 1;
  }
  اگر( neg ) Y = -Y;
  zDate += 10;
  جب( sqlite3Isspace(*zDate) || 'T'==*(u8*)zDate ){ zDate++; }
  اگر( parseHhMmSs(zDate, p)==0 ){
    اگر( p->h==24 ){
      صحیح mxD = days_کا_Month(Y,M);
      p->h = 0;
      D++;
      اگر( D>mxD ){
        D -= mxD;
        M++;
        اگر( M>12 ){
          M = 1;
          Y++;
        }
      }
    }
  }ورنہ اگر( *zDate==0 ){
    p->validHMS = 0;
  }ورنہ{
    واپس 1;
  }
  p->validJD = 0;
  p->validYMD = 1;
  p->Y = Y;
  p->M = M;
  p->D = D;
  computeFloor(p);
  اگر( p->tz ){
    computeJD(p);
  }
  واپس 0;
}


ساکن خالی clearYMD_HMS_TZ(Date_وقت *p);  /* Forward declaration */

/*
** Set the time to the current time reported by the VFS.
**
** Return the number of errors.
*/
ساکن صحیح سیٹ_Date_وقت_مقصد_Current(sqlite3_context *context, Date_وقت *p){
  p->iJD = sqlite3_Stmt_Current_وقت(context);
  اگر( p->iJD>0 ){
    p->validJD = 1;
    p->isUtc = 1;
    p->isLocal = 0;
    clearYMD_HMS_TZ(p);
    واپس 0;
  }ورنہ{
    واپس 1;
  }
}

/*
** Input "r" is a numeric quantity which might be a julian day number,
** or the number of seconds since 1970.  If the value if r is within
** range of a julian day number, install it as such and set validJD.
** If the value is a valid unix timestamp, put it in p->s and set p->rawS.
*/
ساکن خالی سیٹ_Raw_Date_نمبر(Date_وقت *p, دوہرا r){
  p->s = r;
  p->rawS = 1;
  اگر( r>=0.0 && r<5373484.5 ){
    p->iJD = (sqlite3_int64)(r*86400000.0 + 0.5);
    p->validJD = 1;
  }
}

/*
** Attempt to parse the given string into a julian day number.  Return
** the number of errors.
**
** The following are acceptable forms for the input string:
**
**      YYYY-MM-DD HH:MM:SS.FFF  +/-HH:MM
**      DDDD.DD 
**      now
**
** In the first form, the +/-HH:MM is always optional.  The fractional
** seconds extension (the ".FFF") is optional.  The seconds portion
** (":SS.FFF") is option.  The year and date can be omitted as long
** as there is a time string.  The time string can be omitted as long
** as there is a year and date.
*/
ساکن صحیح parse_Date_Or_وقت(
  sqlite3_context *context, 
  ثابت حرف *zDate, 
  Date_وقت *p
){
  دوہرا r;
  اگر( parseYyyyMmDd(zDate,p)==0 ){
    واپس 0;
  }ورنہ اگر( parseHhMmSs(zDate, p)==0 ){
    واپس 0;
  }ورنہ اگر( sqlite3StrICmp(zDate,"now")==0 && sqlite3_Not_خالص_Func(context) ){
    واپس سیٹ_Date_وقت_مقصد_Current(context, p);
  }ورنہ اگر( sqlite3AtoF(zDate, &r)>0 ){
    سیٹ_Raw_Date_نمبر(p, r);
    واپس 0;
  }ورنہ اگر( (sqlite3StrICmp(zDate,"subsec")==0
             || sqlite3StrICmp(zDate,"subsecond")==0)
           && sqlite3_Not_خالص_Func(context) ){
    p->useSubsec = 1;
    واپس سیٹ_Date_وقت_مقصد_Current(context, p);
  }
  واپس 1;
}

/* The julian day number for 9999-12-31 23:59:59.999 is 5373484.4999999.
** Multiplying this by 86400000 gives 464269060799999 as the maximum value
** for DateTime.iJD.
**
** But some older compilers (ex: gcc 4.2.1 on older Macs) cannot deal with 
** such a large integer literal, so we have to encode it.
*/
#تعریف_کرو INT_464269060799999  ((((i64)0x1a640)<<32)|0x1072fdff)

/*
** Return TRUE if the given julian day number is within range.
**
** The input is the JulianDay times 86400000.
*/
ساکن صحیح validJulianDay(sqlite3_int64 iJD){
  واپس iJD>=0 && iJD<=INT_464269060799999;
}

/*
** Compute the Year, Month, and Day from the julian day number.
*/
ساکن خالی computeYMD(Date_وقت *p){
  صحیح Z, alpha, A, B, C, D, E, X1;
  اگر( p->validYMD ) واپس;
  اگر( !p->validJD ){
    p->Y = 2000;
    p->M = 1;
    p->D = 1;
  }ورنہ اگر( !validJulianDay(p->iJD) ){
    datetimeError(p);
    واپس;
  }ورنہ{
    Z = (صحیح)((p->iJD + 43200000)/86400000);
    alpha = (صحیح)((Z + 32044.75)/36524.25) - 52;
    A = Z + 1 + alpha - ((alpha+100)/4) + 25;
    B = A + 1524;
    C = (صحیح)((B - 122.1)/365.25);
    D = (36525*(C&32767))/100;
    E = (صحیح)((B-D)/30.6001);
    X1 = (صحیح)(30.6001*E);
    p->D = B - D - X1;
    p->M = E<14 ? E-1 : E-13;
    p->Y = p->M>2 ? C - 4716 : C - 4715;
  }
  p->validYMD = 1;
}

/*
** Compute the Hour, Minute, and Seconds from the julian day number.
*/
ساکن خالی computeHMS(Date_وقت *p){
  صحیح day_ms, day_min; /* milliseconds, minutes into the day */
  اگر( p->validHMS ) واپس;
  computeJD(p);
  day_ms = (صحیح)((p->iJD + 43200000) % 86400000);
  p->s = (day_ms % 60000)/1000.0;
  day_min = day_ms/60000;
  p->m = day_min % 60;
  p->h = day_min / 60;
  p->rawS = 0;
  p->validHMS = 1;
}

/*
** Compute both YMD and HMS
*/
ساکن خالی computeYMD_HMS(Date_وقت *p){
  computeYMD(p);
  computeHMS(p);
}

/*
** Clear the YMD and HMS and the TZ
*/
ساکن خالی clearYMD_HMS_TZ(Date_وقت *p){
  p->validYMD = 0;
  p->validHMS = 0;
  p->tz = 0;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LOCALTIME
/*
** On recent Windows platforms, the localtime_s() function is available
** as part of the "Secure CRT". It is essentially equivalent to 
** localtime_r() available under most POSIX platforms, except that the 
** order of the parameters is reversed.
**
** See http://msdn.microsoft.com/en-us/library/a442x3ye(VS.80).aspx.
**
** If the user has not indicated to use localtime_r() or localtime_s()
** already, check for an MSVC build environment that provides 
** localtime_s().
*/
#اگر !HAVE_LOCALTIME_R && !HAVE_LOCALTIME_S \
    && defined(_MSC_VER) && defined(_CRT_INSECURE_DEPRECATE)
#تعریف_منسوخ  HAVE_LOCALTIME_S
#تعریف_کرو HAVE_LOCALTIME_S 1
#اگر_ختم

/*
** The following routine implements the rough equivalent of localtime_r()
** using whatever operating-system specific localtime facility that
** is available.  This routine returns 0 on success and
** non-zero on any kind of error.
**
** If the sqlite3GlobalConfig.bLocaltimeFault variable is non-zero then this
** routine will always fail.  If bLocaltimeFault is nonzero and
** sqlite3GlobalConfig.xAltLocaltime is not NULL, then xAltLocaltime() is
** invoked in place of the OS-defined localtime() function.
**
** EVIDENCE-OF: R-62172-00036 In this implementation, the standard C
** library function localtime_r() is used to assist in the calculation of
** local time.
*/
ساکن صحیح osLocaltime(وقت_t *t, ساخت tm *pTm){
  صحیح rc;
#اگر !HAVE_LOCALTIME_R && !HAVE_LOCALTIME_S
  ساخت tm *pX;
#اگر SQLITE_THREADSAFE>0
  sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#اگر_ختم
  sqlite3_mutex_enter(mutex);
  pX = localtime(t);
#اگر_غیر_تعریف_شدہ SQLITE_UNTESTABLE
  اگر( sqlite3GlobalConfig.bLocaltimeFault ){
    اگر( sqlite3GlobalConfig.xAltLocaltime!=0
     && 0==sqlite3GlobalConfig.xAltLocaltime((ثابت خالی*)t,(خالی*)pTm)
    ){
      pX = pTm;
    }ورنہ{
      pX = 0;
    }
  }
#اگر_ختم
  اگر( pX ) *pTm = *pX;
#اگر SQLITE_THREADSAFE>0
  sqlite3_mutex_leave(mutex);
#اگر_ختم
  rc = pX==0;
#ورنہ
#اگر_غیر_تعریف_شدہ SQLITE_UNTESTABLE
  اگر( sqlite3GlobalConfig.bLocaltimeFault ){
    اگر( sqlite3GlobalConfig.xAltLocaltime!=0 ){
      واپس sqlite3GlobalConfig.xAltLocaltime((ثابت خالی*)t,(خالی*)pTm);
    }ورنہ{
      واپس 1;
    }
  }
#اگر_ختم
#اگر HAVE_LOCALTIME_R
  rc = localtime_r(t, pTm)==0;
#ورنہ
  rc = localtime_s(pTm, t);
#اگر_ختم /* HAVE_LOCALTIME_R */
#اگر_ختم /* HAVE_LOCALTIME_R || HAVE_LOCALTIME_S */
  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_LOCALTIME */


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LOCALTIME
/*
** Assuming the input DateTime is UTC, move it to its localtime equivalent.
*/
ساکن صحیح مقصد_Localtime(
  Date_وقت *p,                   /* Date at which to calculate offset */
  sqlite3_context *pCtx          /* Write error here if one occurs */
){
  وقت_t t;
  ساخت tm sLocal;
  صحیح iYearDiff;

  /* Initialize the contents of sLocal to avoid a compiler warning. */
  memset(&sLocal, 0, حجم(sLocal));

  computeJD(p);
  اگر( p->iJD<2108667600*(i64)100000 /* 1970-01-01 */
   || p->iJD>2130141456*(i64)100000 /* 2038-01-18 */
  ){
    /* EVIDENCE-OF: R-55269-29598 The localtime_r() C function normally only
    ** works for years between 1970 and 2037. For dates outside this range,
    ** SQLite attempts to map the year into an equivalent year within this
    ** range, do the calculation, then map the year back.
    */
    Date_وقت x = *p;
    computeYMD_HMS(&x);
    iYearDiff = (2000 + x.Y%4) - x.Y;
    x.Y += iYearDiff;
    x.validJD = 0;
    computeJD(&x);
    t = (وقت_t)(x.iJD/1000 -  21086676*(i64)10000);
  }ورنہ{
    iYearDiff = 0;
    t = (وقت_t)(p->iJD/1000 -  21086676*(i64)10000);
  }
  اگر( osLocaltime(&t, &sLocal) ){
    sqlite3_نتیجہ_error(pCtx, "local time unavailable", -1);
    واپس SQLITE_ERROR;
  }
  p->Y = sLocal.tm_year + 1900 - iYearDiff;
  p->M = sLocal.tm_mon + 1;
  p->D = sLocal.tm_mday;
  p->h = sLocal.tm_hour;
  p->m = sLocal.tm_min;
  p->s = sLocal.tm_sec + (p->iJD%1000)*0.001;
  p->validYMD = 1;
  p->validHMS = 1;
  p->validJD = 0;
  p->rawS = 0;
  p->tz = 0;
  p->isError = 0;
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_OMIT_LOCALTIME */

/*
** The following table defines various date transformations of the form
**
**            'NNN days'
**
** Where NNN is an arbitrary floating-point number and "days" can be one
** of several units of time.
*/
ساکن ثابت ساخت {
  u8 nName;           /* Length of the name */
  حرف zName[7];      /* Name of the transformation */
  اعشاری r_حد_بندی;       /* Maximum NNN value for this transform */
  اعشاری rXform;       /* Constant used for this transform */
} aXformType[] = {
  /* 0 */ { 6, "second",   4.6427e+14,         1.0  },
  /* 1 */ { 6, "minute",   7.7379e+12,        60.0  },
  /* 2 */ { 4, "hour",     1.2897e+11,      3600.0  },
  /* 3 */ { 3, "day",      5373485.0,      86400.0  },
  /* 4 */ { 5, "month",    176546.0,     2592000.0  },
  /* 5 */ { 4, "year",     14713.0,     31536000.0  },
};

/*
** If the DateTime p is raw number, try to figure out if it is
** a julian day number of a unix timestamp.  Set the p value
** appropriately.
*/
ساکن خالی autoAdjustDate(Date_وقت *p){
  اگر( !p->rawS || p->validJD ){
    p->rawS = 0;
  }ورنہ اگر( p->s>=-21086676*(i64)10000        /* -4713-11-24 12:00:00 */
         && p->s<=(25340230*(i64)10000)+799   /*  9999-12-31 23:59:59 */
  ){
    دوہرا r = p->s*1000.0 + 210866760000000.0;
    clearYMD_HMS_TZ(p);
    p->iJD = (sqlite3_int64)(r + 0.5);
    p->validJD = 1;
    p->rawS = 0;
  }
}

/*
** Process a modifier to a date-time stamp.  The modifiers are
** as follows:
**
**     NNN days
**     NNN hours
**     NNN minutes
**     NNN.NNNN seconds
**     NNN months
**     NNN years
**     +/-YYYY-MM-DD HH:MM:SS.SSS
**     ceiling
**     floor
**     start of month
**     start of year
**     start of week
**     start of day
**     weekday N
**     unixepoch
**     auto
**     localtime
**     utc
**     subsec
**     subsecond
**
** Return 0 on success and 1 if there is any kind of error. If the error
** is in a system call (i.e. localtime()), then an error message is written
** to context pCtx. If the error is an unrecognized modifier, no error is
** written to pCtx.
*/
ساکن صحیح parseModifier(
  sqlite3_context *pCtx,      /* Function context */
  ثابت حرف *z,              /* The text of the modifier */
  صحیح n,                      /* Length of zMod in bytes */
  Date_وقت *p,                /* The date/time value to be modified */
  صحیح idx                     /* Parameter index of the modifier */
){
  صحیح rc = 1;
  دوہرا r;
  انتخاب(sqlite3_بالائی_مقصد_زیریں[(u8)z[0]] ){
    حالت 'a': {
      /*
      **    auto
      **
      ** If rawS is available, then interpret as a julian day number, or
      ** a unix timestamp, depending on its magnitude.
      */
      اگر( sqlite3_stricmp(z, "auto")==0 ){
        اگر( idx>1 ) واپس 1; /* IMP: R-33611-57934 */
        autoAdjustDate(p);
        rc = 0;
      }
      توڑو;
    }
    حالت 'c': {
      /*
      **    ceiling
      **
      ** Resolve day-of-month overflow by rolling forward into the next
      ** month.  As this is the default action, this modifier is really
      ** a no-op that is only included for symmetry.  See "floor".
      */
      اگر( sqlite3_stricmp(z, "ceiling")==0 ){
        computeJD(p);
        clearYMD_HMS_TZ(p);
        rc = 0;
        p->nFloor = 0;
      }
      توڑو;
    }
    حالت 'e': {
      /*
      **    end of day
      **    end of month
      **    end of year
      **
      ** Move the date forwards to the last millisecond of the current
      ** day, month or year.
      */
      اگر( sqlite3_strnicmp(z, "end of ", 7)!=0 ){
        توڑو;
      }        
      اگر( !p->validJD && !p->validYMD && !p->validHMS ) توڑو;
      z += 7;
      computeYMD(p);
      p->validHMS = 1;
      p->h = 23;
      p->m = 59;
      p->s = 59.999;
      p->rawS = 0;
      p->tz = 0;
      p->validJD = 0;
      اگر( sqlite3_stricmp(z,"month")==0 ){
        p->D = 1;
        p->M++;
        اگر( p->M>12 ){
          p->Y++;
          p->M = 1;
        }
        computeFloor(p);
        computeJD(p);
        p->iJD -= (p->nFloor+1)*86400000;
        clearYMD_HMS_TZ(p);
        rc = 0;
      }ورنہ اگر( sqlite3_stricmp(z,"year")==0 ){
        p->M = 12;
        p->D = 31;
        rc = 0;
      }ورنہ اگر( sqlite3_stricmp(z,"day")==0 ){
        rc = 0;
      }
      توڑو;
    }
    حالت 'f': {
      /*
      **    floor
      **
      ** Resolve day-of-month overflow by rolling back to the end of the
      ** previous month.
      */
      اگر( sqlite3_stricmp(z, "floor")==0 ){
        computeJD(p);
        p->iJD -= p->nFloor*86400000;
        clearYMD_HMS_TZ(p);
        rc = 0;
      }
      توڑو;
    }
    حالت 'j': {
      /*
      **    julianday
      **
      ** Always interpret the prior number as a julian-day value.  If this
      ** is not the first modifier, or if the prior argument is not a numeric
      ** value in the allowed range of julian day numbers understood by
      ** SQLite (0..5373484.5) then the result will be NULL.
      */
      اگر( sqlite3_stricmp(z, "julianday")==0 ){
        اگر( idx>1 ) واپس 1;  /* IMP: R-31176-64601 */
        اگر( p->validJD && p->rawS ){
          rc = 0;
          p->rawS = 0;
        }
      }
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LOCALTIME
    حالت 'l': {
      /*    localtime
      **
      ** Assuming the current time value is UTC (a.k.a. GMT), shift it to
      ** show local time.
      */
      اگر( sqlite3_stricmp(z, "localtime")==0 && sqlite3_Not_خالص_Func(pCtx) ){
        rc = p->isLocal ? SQLITE_OK : مقصد_Localtime(p, pCtx);
        p->isUtc = 0;
        p->isLocal = 1;
      }
      توڑو;
    }
#اگر_ختم
    حالت 'u': {
      /*
      **    unixepoch
      **
      ** Treat the current value of p->s as the number of
      ** seconds since 1970.  Convert to a real julian day number.
      */
      اگر( sqlite3_stricmp(z, "unixepoch")==0 && p->rawS ){
        اگر( idx>1 ) واپس 1;  /* IMP: R-49255-55373 */
        r = p->s*1000.0 + 210866760000000.0;
        اگر( r>=0.0 && r<464269060800000.0 ){
          clearYMD_HMS_TZ(p);
          p->iJD = (sqlite3_int64)(r + 0.5);
          p->validJD = 1;
          p->rawS = 0;
          rc = 0;
        }
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_LOCALTIME
      ورنہ اگر( sqlite3_stricmp(z, "utc")==0 && sqlite3_Not_خالص_Func(pCtx) ){
        اگر( p->isUtc==0 ){
          i64 iOrigJD;              /* Original localtime */
          i64 iGuess;               /* Guess at the corresponding utc time */
          صحیح cnt = 0;              /* Safety to prevent infinite loop */
          i64 iErr;                 /* Guess is off by this much */

          computeJD(p);
          iGuess = iOrigJD = p->iJD;
          iErr = 0;
          کرو{
            Date_وقت new;
            memset(&new, 0, حجم(new));
            iGuess -= iErr;
            new.iJD = iGuess;
            new.validJD = 1;
            rc = مقصد_Localtime(&new, pCtx);
            اگر( rc ) واپس rc;
            computeJD(&new);
            iErr = new.iJD - iOrigJD;
          }جب( iErr && cnt++<3 );
          memset(p, 0, حجم(*p));
          p->iJD = iGuess;
          p->validJD = 1;
          p->isUtc = 1;
          p->isLocal = 0;
        }
        rc = SQLITE_OK;
      }
#اگر_ختم
      توڑو;
    }
    حالت 'w': {
      /*
      **    weekday N
      **    weekday -N
      **
      ** Move the date forward (for N>=0) or backward (for N<=-0) to the
      ** same time on the next/previous occurrence of weekday abs(N)
      ** where 0==Sunday, 1==Monday, and so forth.  If the date is already
      ** on the appropriate weekday, this is a no-op.
      */
      اگر( sqlite3_strnicmp(z, "weekday ", 8)==0
       && sqlite3AtoF(&z[8], &r)>0
       && r>=-6.0 && r<=6.0
       && sqlite3RealSameAsInt(r,(i64)(n=(صحیح)r))
      ){
        sqlite3_int64 Z;
        computeYMD_HMS(p);
        p->tz = 0;
        p->validJD = 0;
        computeJD(p);
        Z = ((p->iJD + 129600000)/86400000) % 7;
        اگر( n<0 ) n = -n;
        اگر( Z!=n ){
          اگر( Z>n ) Z -= 7;
          p->iJD += (n - Z)*86400000;
          اگر( strchr(z+8,'-')!=0 ) p->iJD -= 7*86400000;
        }
        clearYMD_HMS_TZ(p);
        rc = 0;
      }
      توڑو;
    }
    حالت 's': {
      /*
      **    start of day
      **    start of month
      **    start of year
      **
      ** Move the date backwards to the beginning of the current day,
      ** or month or year.
      **
      **    subsecond
      **    subsec
      **
      ** Show subsecond precision in the output of datetime() and
      ** unixepoch() and strftime('%s').
      */
      اگر( sqlite3_strnicmp(z, "start of ", 9)!=0 ){
        اگر( sqlite3_stricmp(z, "subsec")==0
         || sqlite3_stricmp(z, "subsecond")==0
        ){
          p->useSubsec = 1;
          rc = 0;
        }
        توڑو;
      }        
      اگر( !p->validJD && !p->validYMD && !p->validHMS ) توڑو;
      z += 9;
      computeYMD(p);
      p->validHMS = 1;
      p->h = p->m = 0;
      p->s = 0.0;
      p->rawS = 0;
      p->tz = 0;
      p->validJD = 0;
      اگر( sqlite3_stricmp(z,"month")==0 ){
        p->D = 1;
        rc = 0;
      }ورنہ اگر( sqlite3_stricmp(z,"year")==0 ){
        p->M = 1;
        p->D = 1;
        rc = 0;
      }ورنہ اگر( sqlite3_stricmp(z,"day")==0 ){
        rc = 0;
      }
      توڑو;
    }
    حالت '+':
    حالت '-':
    حالت '0':
    حالت '1':
    حالت '2':
    حالت '3':
    حالت '4':
    حالت '5':
    حالت '6':
    حالت '7':
    حالت '8':
    حالت '9': {
      دوہرا rRounder;
      صحیح i, rx;
      صحیح Y,M,D,h,m,x;
      ثابت حرف *z2 = z;
      حرف *zCopy;
      sqlite3 *db = sqlite3_context_db_handle(pCtx);
      حرف z0 = z[0];
      برائے(n=1; z[n]; n++){
        اگر( z[n]==':' ) توڑو;
        اگر( sqlite3Isspace(z[n]) ) توڑو;
        اگر( z[n]=='-' ){
          اگر( n==5 && getDigits(&z[1], "40f", &Y)==1 ) توڑو;
          اگر( n==6 && getDigits(&z[1], "50f", &Y)==1 ) توڑو;
        }
      }
      zCopy = sqlite3DbStrNDup(db, z, n);
      اگر( zCopy==0 ) توڑو;
      rx = sqlite3AtoF(zCopy, &r)<=0;
      sqlite3DbFree(db, zCopy);
      اگر( rx ){
        أكد( rc==1 );
        توڑو;
      }
      اگر( z[n]=='-' ){
        /* A modifier of the form (+|-)YYYY-MM-DD adds or subtracts the
        ** specified number of years, months, and days.  MM is limited to
        ** the range 0-11 and DD is limited to 0-30.
        */
        اگر( z0!='+' && z0!='-' ) توڑو;  /* Must start with +/- */
        اگر( n==5 ){
          اگر( getDigits(&z[1], "40f-20a-20d", &Y, &M, &D)!=3 ) توڑو;
        }ورنہ{
          أكد( n==6 );
          اگر( getDigits(&z[1], "50f-20a-20d", &Y, &M, &D)!=3 ) توڑو;
          z++;
        }
        اگر( M>=12 ) توڑو;                   /* M range 0..11 */
        اگر( D>=31 ) توڑو;                   /* D range 0..30 */
        computeYMD_HMS(p);
        p->validJD = 0;
        اگر( z0=='-' ){
          p->Y -= Y;
          p->M -= M;
          D = -D;
        }ورنہ{
          p->Y += Y;
          p->M += M;
        }
        x = p->M>0 ? (p->M-1)/12 : (p->M-12)/12;
        p->Y += x;
        p->M -= x*12;
        computeFloor(p);
        computeJD(p);
        p->validHMS = 0;
        p->validYMD = 0;
        p->iJD += (i64)D*86400000;
        اگر( z[11]==0 ){
          rc = 0;
          توڑو;
        }
        اگر( sqlite3Isspace(z[11])
         && getDigits(&z[12], "20c:20e", &h, &m)==2
        ){
          z2 = &z[12];
          n = 2;
        }ورنہ{
          توڑو;
        }
      }
      اگر( z2[n]==':' ){
        /* A modifier of the form (+|-)HH:MM:SS.FFF adds (or subtracts) the
        ** specified number of hours, minutes, seconds, and fractional seconds
        ** to the time.  The ".FFF" may be omitted.  The ":SS.FFF" may be
        ** omitted.
        */

        Date_وقت tx;
        sqlite3_int64 day;
        اگر( !sqlite3Isdigit(*z2) ) z2++;
        memset(&tx, 0, حجم(tx));
        اگر( parseHhMmSs(z2, &tx) ) توڑو;
        computeJD(&tx);
        tx.iJD -= 43200000;
        day = tx.iJD/86400000;
        tx.iJD -= day*86400000;
        اگر( z0=='-' ) tx.iJD = -tx.iJD;
        computeJD(p);
        clearYMD_HMS_TZ(p);
        p->iJD += tx.iJD;
        rc = 0;
        توڑو;
      }

      /* If control reaches this point, it means the transformation is
      ** one of the forms like "+NNN days".  */
      z += n;
      جب( sqlite3Isspace(*z) ) z++;
      n = sqlite3Strlen30(z);
      اگر( n<3 || n>10 ) توڑو;
      اگر( sqlite3_بالائی_مقصد_زیریں[(u8)z[n-1]]=='s' ) n--;
      computeJD(p);
      أكد( rc==1 );
      rRounder = r<0 ? -0.5 : +0.5;
      p->nFloor = 0;
      برائے(i=0; i<Array_سائز(aXformType); i++){
        اگر( aXformType[i].nName==n
         && sqlite3_strnicmp(aXformType[i].zName, z, n)==0
         && r>-aXformType[i].r_حد_بندی && r<aXformType[i].r_حد_بندی
        ){
          انتخاب( i ){
            حالت 4: { /* Special processing to add months */
              أكد( strcmp(aXformType[4].zName,"month")==0 );
              computeYMD_HMS(p);
              p->M += (صحیح)r;
              x = p->M>0 ? (p->M-1)/12 : (p->M-12)/12;
              p->Y += x;
              p->M -= x*12;
              computeFloor(p);
              p->validJD = 0;
              r -= (صحیح)r;
              توڑو;
            }
            حالت 5: { /* Special processing to add years */
              صحیح y = (صحیح)r;
              أكد( strcmp(aXformType[5].zName,"year")==0 );
              computeYMD_HMS(p);
              أكد( p->M>=0 && p->M<=12 );
              p->Y += y;
              computeFloor(p);
              p->validJD = 0;
              r -= (صحیح)r;
              توڑو;
            }
          }
          computeJD(p);
          p->iJD += (sqlite3_int64)(r*1000.0*aXformType[i].rXform + rRounder);
          rc = 0;
          توڑو;
        }
      }
      clearYMD_HMS_TZ(p);
      توڑو;
    }
    طےشدہ: {
      توڑو;
    }
  }
  واپس rc;
}

/*
** Process time function arguments.  argv[0] is a date-time stamp.
** argv[1] and following are modifiers.  Parse them all and write
** the resulting time into the DateTime structure p.  Return 0
** on success and 1 if there are any errors.
**
** If there are zero parameters (if even argv[0] is undefined)
** then assume a default value of "now" for argv[0].
*/
ساکن صحیح isDate(
  sqlite3_context *context, 
  صحیح argc, 
  sqlite3_قدر **argv, 
  Date_وقت *p
){
  صحیح i, n;
  ثابت غیرعلامتی حرف *z;
  صحیح eType;
  memset(p, 0, حجم(*p));
  اگر( argc==0 ){
    اگر( !sqlite3_Not_خالص_Func(context) ) واپس 1;
    واپس سیٹ_Date_وقت_مقصد_Current(context, p);
  }
  اگر( (eType = sqlite3_قدر_type(argv[0]))==SQLITE_FLOAT
                   || eType==SQLITE_INTEGER ){
    سیٹ_Raw_Date_نمبر(p, sqlite3_قدر_double(argv[0]));
  }ورنہ{
    z = sqlite3_قدر_متن(argv[0]);
    اگر( !z || parse_Date_Or_وقت(context, (حرف*)z, p) ){
      واپس 1;
    }
  }
  برائے(i=1; i<argc; i++){
    z = sqlite3_قدر_متن(argv[i]);
    n = sqlite3_قدر_بائٹیں(argv[i]);
    اگر( z==0 || parseModifier(context, (حرف*)z, n, p, i) ) واپس 1;
  }
  computeJD(p);
  اگر( p->isError || !validJulianDay(p->iJD) ) واپس 1;
  اگر( argc==1 && p->validYMD && p->D>28 ){
    /* Make sure a YYYY-MM-DD is normalized.
    ** Example: 2023-02-31 -> 2023-03-03 */
    أكد( p->validJD );
    p->validYMD = 0;  
  }
  واپس 0;
}


/*
** The following routines implement the various date and time functions
** of SQLite.
*/

/*
**    julianday( TIMESTRING, MOD, MOD, ...)
**
** Return the julian day number of the date specified in the arguments
*/
ساکن خالی juliandayFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  Date_وقت x;
  اگر( isDate(context, argc, argv, &x)==0 ){
    computeJD(&x);
    sqlite3_نتیجہ_double(context, x.iJD/86400000.0);
  }
}

/*
**    unixepoch( TIMESTRING, MOD, MOD, ...)
**
** Return the number of seconds (including fractional seconds) since
** the unix epoch of 1970-01-01 00:00:00 GMT.
*/
ساکن خالی unixepochFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  Date_وقت x;
  اگر( isDate(context, argc, argv, &x)==0 ){
    computeJD(&x);
    اگر( x.useSubsec ){
      sqlite3_نتیجہ_double(context, (x.iJD - 21086676*(i64)10000000)/1000.0);
    }ورنہ{
      sqlite3_نتیجہ_int64(context, x.iJD/1000 - 21086676*(i64)10000);
    }
  }
}

/*
**    datetime( TIMESTRING, MOD, MOD, ...)
**
** Return YYYY-MM-DD HH:MM:SS
*/
ساکن خالی datetimeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  Date_وقت x;
  اگر( isDate(context, argc, argv, &x)==0 ){
    صحیح Y, s, n;
    حرف zBuf[32];
    computeYMD_HMS(&x);
    Y = x.Y;
    اگر( Y<0 ) Y = -Y;
    zBuf[1] = '0' + (Y/1000)%10;
    zBuf[2] = '0' + (Y/100)%10;
    zBuf[3] = '0' + (Y/10)%10;
    zBuf[4] = '0' + (Y)%10;
    zBuf[5] = '-';
    zBuf[6] = '0' + (x.M/10)%10;
    zBuf[7] = '0' + (x.M)%10;
    zBuf[8] = '-';
    zBuf[9] = '0' + (x.D/10)%10;
    zBuf[10] = '0' + (x.D)%10;
    zBuf[11] = ' ';
    zBuf[12] = '0' + (x.h/10)%10;
    zBuf[13] = '0' + (x.h)%10;
    zBuf[14] = ':';
    zBuf[15] = '0' + (x.m/10)%10;
    zBuf[16] = '0' + (x.m)%10;
    zBuf[17] = ':';
    اگر( x.useSubsec ){
      s = (صحیح)(1000.0*x.s + 0.5);
      zBuf[18] = '0' + (s/10000)%10;
      zBuf[19] = '0' + (s/1000)%10;
      zBuf[20] = '.';
      zBuf[21] = '0' + (s/100)%10;
      zBuf[22] = '0' + (s/10)%10;
      zBuf[23] = '0' + (s)%10;
      zBuf[24] = 0;
      n = 24;
    }ورنہ{
      s = (صحیح)x.s;
      zBuf[18] = '0' + (s/10)%10;
      zBuf[19] = '0' + (s)%10;
      zBuf[20] = 0;
      n = 20;
    }
    اگر( x.Y<0 ){
      zBuf[0] = '-';
      sqlite3_نتیجہ_متن(context, zBuf, n, SQLITE_TRANSIENT);
    }ورنہ{
      sqlite3_نتیجہ_متن(context, &zBuf[1], n-1, SQLITE_TRANSIENT);
    }
  }
}

/*
**    time( TIMESTRING, MOD, MOD, ...)
**
** Return HH:MM:SS
*/
ساکن خالی وقت_Func(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  Date_وقت x;
  اگر( isDate(context, argc, argv, &x)==0 ){
    صحیح s, n;
    حرف zBuf[16];
    computeHMS(&x);
    zBuf[0] = '0' + (x.h/10)%10;
    zBuf[1] = '0' + (x.h)%10;
    zBuf[2] = ':';
    zBuf[3] = '0' + (x.m/10)%10;
    zBuf[4] = '0' + (x.m)%10;
    zBuf[5] = ':';
    اگر( x.useSubsec ){
      s = (صحیح)(1000.0*x.s + 0.5);
      zBuf[6] = '0' + (s/10000)%10;
      zBuf[7] = '0' + (s/1000)%10;
      zBuf[8] = '.';
      zBuf[9] = '0' + (s/100)%10;
      zBuf[10] = '0' + (s/10)%10;
      zBuf[11] = '0' + (s)%10;
      zBuf[12] = 0;
      n = 12;
    }ورنہ{
      s = (صحیح)x.s;
      zBuf[6] = '0' + (s/10)%10;
      zBuf[7] = '0' + (s)%10;
      zBuf[8] = 0;
      n = 8;
    }
    sqlite3_نتیجہ_متن(context, zBuf, n, SQLITE_TRANSIENT);
  }
}

/*
**    date( TIMESTRING, MOD, MOD, ...)
**
** Return YYYY-MM-DD
*/
ساکن خالی dateFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  Date_وقت x;
  اگر( isDate(context, argc, argv, &x)==0 ){
    صحیح Y;
    حرف zBuf[16];
    computeYMD(&x);
    Y = x.Y;
    اگر( Y<0 ) Y = -Y;
    zBuf[1] = '0' + (Y/1000)%10;
    zBuf[2] = '0' + (Y/100)%10;
    zBuf[3] = '0' + (Y/10)%10;
    zBuf[4] = '0' + (Y)%10;
    zBuf[5] = '-';
    zBuf[6] = '0' + (x.M/10)%10;
    zBuf[7] = '0' + (x.M)%10;
    zBuf[8] = '-';
    zBuf[9] = '0' + (x.D/10)%10;
    zBuf[10] = '0' + (x.D)%10;
    zBuf[11] = 0;
    اگر( x.Y<0 ){
      zBuf[0] = '-';
      sqlite3_نتیجہ_متن(context, zBuf, 11, SQLITE_TRANSIENT);
    }ورنہ{
      sqlite3_نتیجہ_متن(context, &zBuf[1], 10, SQLITE_TRANSIENT);
    }
  }
}

/*
** Compute the number of days after the most recent January 1.
**
** In other words, compute the zero-based day number for the
** current year:
**
**   Jan01 = 0,  Jan02 = 1, ..., Jan31 = 30, Feb01 = 31, ...
**   Dec31 = 364 or 365.
*/
ساکن صحیح daysAfterJan01(Date_وقت *pDate){
  Date_وقت jan01 = *pDate;
  أكد( jan01.validYMD );
  أكد( jan01.validHMS );
  أكد( pDate->validJD );
  jan01.validJD = 0;
  jan01.M = 1;
  jan01.D = 1;
  computeJD(&jan01);
  واپس (صحیح)((pDate->iJD-jan01.iJD+43200000)/86400000);
}

/*
** Return the number of days after the most recent Monday.
**
** In other words, return the day of the week according
** to this code:
**
**   0=Monday, 1=Tuesday, 2=Wednesday, ..., 6=Sunday.
*/
ساکن صحیح daysAfterMonday(Date_وقت *pDate){
  أكد( pDate->validJD );
  واپس (صحیح)((pDate->iJD+43200000)/86400000) % 7;
}

/*
** Return the number of days after the most recent Sunday.
**
** In other words, return the day of the week according
** to this code:
**
**   0=Sunday, 1=Monday, 2=Tuesday, ..., 6=Saturday
*/
ساکن صحیح daysAfterSunday(Date_وقت *pDate){
  أكد( pDate->validJD );
  واپس (صحیح)((pDate->iJD+129600000)/86400000) % 7;
}

/*
**    strftime( FORMAT, TIMESTRING, MOD, MOD, ...)
**
** Return a string described by FORMAT.  Conversions as follows:
**
**   %d  day of month  01-31
**   %e  day of month  1-31
**   %f  ** fractional seconds  SS.SSS
**   %F  ISO date.  YYYY-MM-DD
**   %G  ISO year corresponding to %V 0000-9999.
**   %g  2-digit ISO year corresponding to %V 00-99
**   %H  hour 00-24
**   %k  hour  0-24  (leading zero converted to space)
**   %I  hour 01-12
**   %j  day of year 001-366
**   %J  ** julian day number
**   %l  hour  1-12  (leading zero converted to space)
**   %m  month 01-12
**   %M  minute 00-59
**   %p  "AM" or "PM"
**   %P  "am" or "pm"
**   %R  time as HH:MM
**   %s  seconds since 1970-01-01
**   %S  seconds 00-59
**   %T  time as HH:MM:SS
**   %u  day of week 1-7  Monday==1, Sunday==7
**   %w  day of week 0-6  Sunday==0, Monday==1
**   %U  week of year 00-53  (First Sunday is start of week 01)
**   %V  week of year 01-53  (First week containing Thursday is week 01)
**   %W  week of year 00-53  (First Monday is start of week 01)
**   %Y  year 0000-9999
**   %%  %
*/
ساکن خالی strftimeFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  Date_وقت x;
  size_t i,j;
  sqlite3 *db;
  ثابت حرف *zFmt;
  sqlite3_str *pRes;


  اگر( argc==0 ) واپس;
  zFmt = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  اگر( zFmt==0 || isDate(context, argc-1, argv+1, &x) ) واپس;
  db = sqlite3_context_db_handle(context);
  pRes = sqlite3_str_new(db);

  computeJD(&x);
  computeYMD_HMS(&x);
  برائے(i=j=0; zFmt[i]; i++){
    حرف cf;
    اگر( zFmt[i]!='%' ) جاری;
    اگر( j<i ) sqlite3_str_append(pRes, zFmt+j, (صحیح)(i-j));
    i++;
    j = i + 1;
    cf = zFmt[i];
    انتخاب( cf ){
      حالت 'd':  /* Fall thru */
      حالت 'e': {
        sqlite3_str_appendf(pRes, cf=='d' ? "%02d" : "%2d", x.D);
        توڑو;
      }
      حالت 'f': {  /* Fractional seconds.  (Non-standard) */
        دوہرا s = x.s;
        اگر( NEVER(s>59.999) ) s = 59.999;
        sqlite3_str_appendf(pRes, "%06.3f", s);
        توڑو;
      }
      حالت 'F': {
        sqlite3_str_appendf(pRes, "%04d-%02d-%02d", x.Y, x.M, x.D);
        توڑو;
      }
      حالت 'G': /* Fall thru */
      حالت 'g': {
        Date_وقت y = x;
        أكد( y.validJD );
        /* Move y so that it is the Thursday in the same week as x */
        y.iJD += (3 - daysAfterMonday(&x))*86400000;
        y.validYMD = 0;
        computeYMD(&y);
        اگر( cf=='g' ){
          sqlite3_str_appendf(pRes, "%02d", y.Y%100);
        }ورنہ{
          sqlite3_str_appendf(pRes, "%04d", y.Y);
        }
        توڑو;
      }
      حالت 'H':
      حالت 'k': {
        sqlite3_str_appendf(pRes, cf=='H' ? "%02d" : "%2d", x.h);
        توڑو;
      }
      حالت 'I': /* Fall thru */
      حالت 'l': {
        صحیح h = x.h;
        اگر( h>12 ) h -= 12;
        اگر( h==0 ) h = 12;
        sqlite3_str_appendf(pRes, cf=='I' ? "%02d" : "%2d", h);
        توڑو;
      }
      حالت 'j': {  /* Day of year.  Jan01==1, Jan02==2, and so forth */
        sqlite3_str_appendf(pRes,"%03d",daysAfterJan01(&x)+1);
        توڑو;
      }
      حالت 'J': {  /* Julian day number.  (Non-standard) */
        sqlite3_str_appendf(pRes,"%.16g",x.iJD/86400000.0);
        توڑو;
      }
      حالت 'm': {
        sqlite3_str_appendf(pRes,"%02d",x.M);
        توڑو;
      }
      حالت 'M': {
        sqlite3_str_appendf(pRes,"%02d",x.m);
        توڑو;
      }
      حالت 'p': /* Fall thru */
      حالت 'P': {
        اگر( x.h>=12 ){
          sqlite3_str_append(pRes, cf=='p' ? "PM" : "pm", 2);
        }ورنہ{
          sqlite3_str_append(pRes, cf=='p' ? "AM" : "am", 2);
        }
        توڑو;
      }
      حالت 'R': {
        sqlite3_str_appendf(pRes, "%02d:%02d", x.h, x.m);
        توڑو;
      }
      حالت 's': {
        اگر( x.useSubsec ){
          sqlite3_str_appendf(pRes,"%.3f",
                (x.iJD - 21086676*(i64)10000000)/1000.0);
        }ورنہ{
          i64 iS = (i64)(x.iJD/1000 - 21086676*(i64)10000);
          sqlite3_str_appendf(pRes,"%lld",iS);
        }
        توڑو;
      }
      حالت 'S': {
        sqlite3_str_appendf(pRes,"%02d",(صحیح)x.s);
        توڑو;
      }
      حالت 'T': {
        sqlite3_str_appendf(pRes,"%02d:%02d:%02d", x.h, x.m, (صحیح)x.s);
        توڑو;
      }
      حالت 'u':    /* Day of week.  1 to 7.  Monday==1, Sunday==7 */
      حالت 'w': {  /* Day of week.  0 to 6.  Sunday==0, Monday==1 */
        حرف c = (حرف)daysAfterSunday(&x) + '0';
        اگر( c=='0' && cf=='u' ) c = '7';
        sqlite3_str_appendchar(pRes, 1, c);
        توڑو;
      }
      حالت 'U': {  /* Week num. 00-53. First Sun of the year is week 01 */
        sqlite3_str_appendf(pRes,"%02d",
              (daysAfterJan01(&x)-daysAfterSunday(&x)+7)/7);
        توڑو;
      }
      حالت 'V': {  /* Week num. 01-53. First week with a Thur is week 01 */
        Date_وقت y = x;
        /* Adjust y so that is the Thursday in the same week as x */
        أكد( y.validJD );
        y.iJD += (3 - daysAfterMonday(&x))*86400000;
        y.validYMD = 0;
        computeYMD(&y);
        sqlite3_str_appendf(pRes,"%02d", daysAfterJan01(&y)/7+1);
        توڑو;
      }
      حالت 'W': {  /* Week num. 00-53. First Mon of the year is week 01 */
        sqlite3_str_appendf(pRes,"%02d",
           (daysAfterJan01(&x)-daysAfterMonday(&x)+7)/7);
        توڑو;
      }
      حالت 'Y': {
        sqlite3_str_appendf(pRes,"%04d",x.Y);
        توڑو;
      }
      حالت '%': {
        sqlite3_str_appendchar(pRes, 1, '%');
        توڑو;
      }
      طےشدہ: {
        sqlite3_str_free(pRes);
        واپس;
      }
    }
  }
  اگر( j<i ) sqlite3_str_append(pRes, zFmt+j, (صحیح)(i-j));
  sqlite3_نتیجہ_str(context, pRes, SQLITE_اختتام);
}

/*
** current_time()
**
** This function returns the same value as time('now').
*/
ساکن خالی ctimeFunc(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  وقت_Func(context, 0, 0);
}

/*
** current_date()
**
** This function returns the same value as date('now').
*/
ساکن خالی cdateFunc(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  dateFunc(context, 0, 0);
}

/*
** timediff(DATE1, DATE2)
**
** Return the amount of time that must be added to DATE2 in order to
** convert it into DATE2.  The time difference format is:
**
**     +YYYY-MM-DD HH:MM:SS.SSS
**
** The initial "+" becomes "-" if DATE1 occurs before DATE2.  For
** date/time values A and B, the following invariant should hold:
**
**     datetime(A) == (datetime(B, timediff(A,B))
**
** Both DATE arguments must be either a julian day number, or an
** ISO-8601 string.  The unix timestamps are not supported by this
** routine.
*/
ساکن خالی timediffFunc(
  sqlite3_context *context,
  صحیح NotUsed1,
  sqlite3_قدر **argv
){
  حرف sign;
  صحیح Y, M;
  Date_وقت d1, d2;
  sqlite3_str sRes;
  غیر_مستعمل_PARAMETER(NotUsed1);
  اگر( isDate(context, 1, &argv[0], &d1) ) واپس;
  اگر( isDate(context, 1, &argv[1], &d2) ) واپس;
  computeYMD_HMS(&d1);
  computeYMD_HMS(&d2);
  اگر( d1.iJD>=d2.iJD ){
    sign = '+';
    Y = d1.Y - d2.Y;
    اگر( Y ){
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d1.M - d2.M;
    اگر( M<0 ){
      Y--;
      M += 12;
    }
    اگر( M!=0 ){
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    جب( d1.iJD<d2.iJD ){
      M--;
      اگر( M<0 ){
        M = 11;
        Y--;
      }
      d2.M--;
      اگر( d2.M<1 ){
        d2.M = 12;
        d2.Y--;
      }
      d2.validJD = 0;
      computeJD(&d2);
    }
    d1.iJD -= d2.iJD;
    d1.iJD += (u64)1486995408 * (u64)100000;
  }ورنہ /* d1<d2 */{
    sign = '-';
    Y = d2.Y - d1.Y;
    اگر( Y ){
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d2.M - d1.M;
    اگر( M<0 ){
      Y--;
      M += 12;
    }
    اگر( M!=0 ){
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    جب( d1.iJD>d2.iJD ){
      M--;
      اگر( M<0 ){
        M = 11;
        Y--;
      }
      d2.M++;
      اگر( d2.M>12 ){
        d2.M = 1;
        d2.Y++;
      }
      d2.validJD = 0;
      computeJD(&d2);
    }
    d1.iJD = d2.iJD - d1.iJD;
    d1.iJD += (u64)1486995408 * (u64)100000;
  }
  clearYMD_HMS_TZ(&d1);
  computeYMD_HMS(&d1);
  sqlite3StrAccumInit(&sRes, 0, 0, 0, 100);
  sqlite3_str_appendf(&sRes, "%c%04d-%02d-%02d %02d:%02d:%06.3f",
       sign, Y, M, d1.D-1, d1.h, d1.m, d1.s);
  sqlite3_نتیجہ_str(context, &sRes, SQLITE_XFER);
}


/*
** current_timestamp()
**
** This function returns the same value as datetime('now').
*/
ساکن خالی ctimestampFunc(
  sqlite3_context *context,
  صحیح Not_مستعمل,
  sqlite3_قدر **NotUsed2
){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  datetimeFunc(context, 0, 0);
}
#اگر_ختم /* !defined(SQLITE_OMIT_DATETIME_FUNCS) */

#اگر_تعریف_شدہ SQLITE_OMIT_DATETIME_FUNCS
/*
** If the library is compiled to omit the full-scale date and time
** handling (to get a smaller binary), the following minimal version
** of the functions current_time(), current_date() and current_timestamp()
** are included instead. This is to support column declarations that
** include "DEFAULT CURRENT_TIME" etc.
**
** This function uses the C-library functions time(), gmtime()
** and strftime(). The format string to pass to strftime() is supplied
** as the user-data for the function.
*/
ساکن خالی current_وقت_Func(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  وقت_t t;
  حرف *zFormat = (حرف *)sqlite3_user_ڈیٹا(context);
  sqlite3_int64 iT;
  ساخت tm *pTm;
  ساخت tm sNow;
  حرف zBuf[20];

  غیر_مستعمل_PARAMETER(argc);
  غیر_مستعمل_PARAMETER(argv);

  iT = sqlite3_Stmt_Current_وقت(context);
  اگر( iT<=0 ) واپس;
  t = iT/1000 - 10000*(sqlite3_int64)21086676;
#اگر HAVE_GMTIME_R
  pTm = gmtime_r(&t, &sNow);
#ورنہ
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
  pTm = gmtime(&t);
  اگر( pTm ) حافظہ_نقل(&sNow, pTm, حجم(sNow));
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
#اگر_ختم
  اگر( pTm ){
    strftime(zBuf, 20, zFormat, &sNow);
    sqlite3_نتیجہ_متن(context, zBuf, -1, SQLITE_TRANSIENT);
  }
}
#اگر_ختم

#اگر !defined(SQLITE_OMIT_DATETIME_FUNCS) && defined(SQLITE_DEBUG)
/*
**   datedebug(...)
**
** This routine returns JSON that describes the internal DateTime object.
** Used for debugging and testing only.  Subject to change.
*/
ساکن خالی datedebugFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  Date_وقت x;
  اگر( isDate(context, argc, argv, &x)==0 ){
    حرف *zJson;
    zJson = sqlite3_mprintf(
      "{iJD:%lld,Y:%d,M:%d,D:%d,h:%d,m:%d,tz:%d,"
      "s:%.3f,validJD:%d,validYMD:%d,validHMS:%d,"
      "nFloor:%d,rawS:%d,isError:%d,useSubsec:%d,"
      "isUtc:%d,isLocal:%d}",
      x.iJD, x.Y, x.M, x.D, x.h, x.m, x.tz,
      x.s, x.validJD, x.validYMD, x.validHMS,
      x.nFloor, x.rawS, x.isError, x.useSubsec,
      x.isUtc, x.isLocal);
    sqlite3_نتیجہ_متن(context, zJson, -1, sqlite3_free);
  }
}
#اگر_ختم /* !SQLITE_OMIT_DATETIME_FUNCS && SQLITE_DEBUG */


/*
** This function registered all of the above C functions as SQL
** functions.  This should be the only routine in this file with
** external linkage.
*/
خالی sqlite3_رجسٹر_Date_وقت_Functions(خالی){
  ساکن FuncDef a_Date_وقت_Funcs[] = {
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DATETIME_FUNCS
    خالص_DATE(julianday,        -1, 0, 0, juliandayFunc ),
    خالص_DATE(unixepoch,        -1, 0, 0, unixepochFunc ),
    خالص_DATE(date,             -1, 0, 0, dateFunc      ),
    خالص_DATE(وقت,             -1, 0, 0, وقت_Func      ),
    خالص_DATE(datetime,         -1, 0, 0, datetimeFunc  ),
    خالص_DATE(strftime,         -1, 0, 0, strftimeFunc  ),
    خالص_DATE(timediff,          2, 0, 0, timediffFunc  ),
#اگر_تعریف_شدہ SQLITE_DEBUG
    خالص_DATE(datedebug,        -1, 0, 0, datedebugFunc ),
#اگر_ختم
    DFUNCTION(current_وقت,      0, 0, 0, ctimeFunc     ),
    DFUNCTION(current_timestamp, 0, 0, 0, ctimestampFunc),
    DFUNCTION(current_date,      0, 0, 0, cdateFunc     ),
#ورنہ
    STR_FUNCTION(current_وقت,      0, "%H:%M:%S",          0, current_وقت_Func),
    STR_FUNCTION(current_date,      0, "%Y-%m-%d",          0, current_وقت_Func),
    STR_FUNCTION(current_timestamp, 0, "%Y-%m-%d %H:%M:%S", 0, current_وقت_Func),
#اگر_ختم
  };
  sqlite3_داخل_کرو_Builtin_Funcs(a_Date_وقت_Funcs, Array_سائز(a_Date_وقت_Funcs));
}
