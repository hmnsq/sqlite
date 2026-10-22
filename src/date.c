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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <time.h>

#ifndef SQLITE_OMIT_DATETIME_FUNCS

/*
** The MSVC CRT on Windows CE may not have a localtime() function.
** So declare a substitute.  The substitute function itself is
** defined in "os_win.c".
*/
#যদি !defined(SQLITE_OMIT_LOCALTIME) && defined(_WIN32_WCE) && \
    (!defined(SQLITE_MSVC_LOCALTIME_API) || !SQLITE_MSVC_LOCALTIME_API)
কাঠামো tm *__cdecl localtime(ধ্রুবক time_t *);
#endif

/*
** A structure for holding a single date and time.
*/
প্রকারলেখ কাঠামো DateTime DateTime;
কাঠামো DateTime {
  sqlite3_int64 iJD;  /* The julian day number times 86400000 */
  পূর্ণসংখ্যা Y, M, D;        /* Year, month, and day */
  পূর্ণসংখ্যা h, m;           /* Hour and minutes */
  পূর্ণসংখ্যা tz;             /* Timezone offset in minutes */
  দ্বিগুণ s;           /* Seconds */
  অক্ষর validJD;       /* True (1) if iJD is valid */
  অক্ষর validYMD;      /* True (1) if Y,M,D are valid */
  অক্ষর validHMS;      /* True (1) if h,m,s are valid */
  অক্ষর nFloor;            /* Days to implement "floor" */
  unsigned rawS      : 1; /* Raw numeric value stored in s */
  unsigned isError   : 1; /* An overflow has occurred */
  unsigned useSubsec : 1; /* Display subsecond precision */
  unsigned isUtc     : 1; /* Time is known to be UTC */
  unsigned isLocal   : 1; /* Time is known to be localtime */
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
স্থির পূর্ণসংখ্যা getDigits(ধ্রুবক অক্ষর *zDate, ধ্রুবক অক্ষর *zFormat, ...){
  /* The aMx[] array translates the 3rd character of each format
  ** spec into a max size:    a   b   c   d   e      f */
  স্থির ধ্রুবক u16 aMx[] = { 12, 14, 24, 31, 59, 14712 };
  va_list ap;
  পূর্ণসংখ্যা cnt = 0;
  অক্ষর পরবর্তী_C;
  va_start(ap, zFormat);
  করো{
    অক্ষর N = zFormat[0] - '0';
    অক্ষর min = zFormat[1] - '0';
    পূর্ণসংখ্যা val = 0;
    u16 max;

    প্রতিজ্ঞা( zFormat[2]>='a' && zFormat[2]<='f' );
    max = aMx[zFormat[2] - 'a'];
    পরবর্তী_C = zFormat[3];
    val = 0;
    যখন( N-- ){
      যদি( !sqlite3Isdigit(*zDate) ){
        যাও end_getDigits;
      }
      val = val*10 + *zDate - '0';
      zDate++;
    }
    যদি( val<(পূর্ণসংখ্যা)min || val>(পূর্ণসংখ্যা)max || (পরবর্তী_C!=0 && পরবর্তী_C!=*zDate) ){
      যাও end_getDigits;
    }
    *va_arg(ap,পূর্ণসংখ্যা*) = val;
    zDate++;
    cnt++;
    zFormat += 4;
  }যখন( পরবর্তী_C );
end_getDigits:
  va_end(ap);
  ফেরত cnt;
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
স্থির পূর্ণসংখ্যা parseTimezone(ধ্রুবক অক্ষর *zDate, DateTime *p){
  পূর্ণসংখ্যা sgn = 0;
  পূর্ণসংখ্যা nHr, nMn;
  পূর্ণসংখ্যা c;
  যখন( sqlite3Isspace(*zDate) ){ zDate++; }
  p->tz = 0;
  c = *zDate;
  যদি( c=='-' ){
    sgn = -1;
  }অন্যথা যদি( c=='+' ){
    sgn = +1;
  }অন্যথা যদি( c=='Z' || c=='z' ){
    zDate++;
    p->isLocal = 0;
    p->isUtc = 1;
    যাও zulu_time;
  }অন্যথা{
    ফেরত c!=0;
  }
  zDate++;
  যদি( getDigits(zDate, "20b:20e", &nHr, &nMn)!=2 ){
    ফেরত 1;
  }
  zDate += 5;
  p->tz = sgn*(nMn + nHr*60);
  যদি( p->tz==0 ){   /* Forum post 2025-09-17T10:12:14z */
    p->isLocal = 0;
    p->isUtc = 1;
  }
zulu_time:
  যখন( sqlite3Isspace(*zDate) ){ zDate++; }
  ফেরত *zDate!=0;
}

/*
** Parse times of the form HH:MM or HH:MM:SS or HH:MM:SS.FFFF.
** The HH, MM, and SS must each be exactly 2 digits.  The
** fractional seconds FFFF can be one or more digits.
**
** Return 1 if there is a parsing error and 0 on success.
*/
স্থির পূর্ণসংখ্যা parseHhMmSs(ধ্রুবক অক্ষর *zDate, DateTime *p){
  পূর্ণসংখ্যা h, m, s;
  দ্বিগুণ ms = 0.0;
  যদি( getDigits(zDate, "20c:20e", &h, &m)!=2 ){
    ফেরত 1;
  }
  zDate += 5;
  যদি( *zDate==':' ){
    zDate++;
    যদি( getDigits(zDate, "20e", &s)!=1 ){
      ফেরত 1;
    }
    zDate += 2;
    যদি( *zDate=='.' && sqlite3Isdigit(zDate[1]) ){
      দ্বিগুণ rScale = 1.0;
      zDate++;
      যখন( sqlite3Isdigit(*zDate) ){
        ms = ms*10.0 + *zDate - '0';
        rScale *= 10.0;
        zDate++;
      }
      ms /= rScale;
      /* Truncate to avoid problems with sub-milliseconds
      ** rounding. https://sqlite.org/forum/forumpost/766a2c9231 */
      যদি( ms>0.999 ) ms = 0.999;
    }
  }অন্যথা{
    s = 0;
  }
  যদি( h==24 && (m!=0 || s!=0 || ms!=0.0) ) ফেরত 1;
  p->validJD = 0;
  p->rawS = 0;
  p->validHMS = 1;
  p->h = h;
  p->m = m;
  p->s = s + ms;
  যদি( parseTimezone(zDate, p) ) ফেরত 1;
  ফেরত 0;
}

/*
** Put the DateTime object into its error state.
*/
স্থির শূন্য datetimeError(DateTime *p){
  memset(p, 0, আকার(*p));
  p->isError = 1;
}

/*
** Convert from YYYY-MM-DD HH:MM:SS to julian day.  We always assume
** that the YYYY-MM-DD is according to the Gregorian calendar.
**
** Reference:  Meeus page 61
*/
স্থির শূন্য computeJD(DateTime *p){
  পূর্ণসংখ্যা Y, M, D, A, B, X1, X2;

  যদি( p->validJD ) ফেরত;
  যদি( p->validYMD ){
    Y = p->Y;
    M = p->M;
    D = p->D;
  }অন্যথা{
    Y = 2000;  /* If no YMD specified, assume 2000-Jan-01 */
    M = 1;
    D = 1;
    যদি( p->validHMS && p->h==24 ){
      p->h = 0;
      D = 2;
    }
  }
  যদি( Y<-4713 || Y>9999 || p->rawS ){
    datetimeError(p);
    ফেরত;
  }
  যদি( M<=2 ){
    Y--;
    M += 12;
  }
  A = (Y+4800)/100;
  B = 38 - A + (A/4);
  X1 = 36525*(Y+4716)/100;
  X2 = 306001*(M+1)/10000;
  p->iJD = (sqlite3_int64)((X1 + X2 + D + B - 1524.5 ) * 86400000);
  p->validJD = 1;
  যদি( p->validHMS ){
    p->iJD += p->h*3600000 + p->m*60000 + (sqlite3_int64)(p->s*1000 + 0.5);
    যদি( p->tz ){
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
স্থির পূর্ণসংখ্যা daysOfMonth(পূর্ণসংখ্যা Y, পূর্ণসংখ্যা M){
  যদি( (1<<M) & 0x15aa ){
    ফেরত 31;
  }অন্যথা যদি( M!=2 ){
    ফেরত 30;
  }অন্যথা যদি( Y%4==0 && (Y%100!=0 || Y%400==0) ){
    ফেরত 29;
  }অন্যথা{
    ফেরত 28;
  }
}

/*
** Given the YYYY-MM-DD information current in p, determine if there
** is day-of-month overflow and set nFloor to the number of days that
** would need to be subtracted from the date in order to bring the
** date back to the end of the month.
*/
স্থির শূন্য computeFloor(DateTime *p){
  পূর্ণসংখ্যা mxD;         /* Maximum day number for the current month */
  প্রতিজ্ঞা( p->validYMD || p->isError );
  প্রতিজ্ঞা( p->D>=0 && p->D<=32 );
  প্রতিজ্ঞা( p->M>=0 && p->M<=12 );
  mxD = daysOfMonth(p->Y, p->M);
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
স্থির পূর্ণসংখ্যা parseYyyyMmDd(ধ্রুবক অক্ষর *zDate, DateTime *p){
  পূর্ণসংখ্যা Y, M, D, neg;

  যদি( zDate[0]=='-' ){
    zDate++;
    neg = 1;
  }অন্যথা{
    neg = 0;
  }
  যদি( getDigits(zDate, "40f-21a-21d", &Y, &M, &D)!=3 ){
    ফেরত 1;
  }
  যদি( neg ) Y = -Y;
  zDate += 10;
  যখন( sqlite3Isspace(*zDate) || 'T'==*(u8*)zDate ){ zDate++; }
  যদি( parseHhMmSs(zDate, p)==0 ){
    যদি( p->h==24 ){
      পূর্ণসংখ্যা mxD = daysOfMonth(Y,M);
      p->h = 0;
      D++;
      যদি( D>mxD ){
        D -= mxD;
        M++;
        যদি( M>12 ){
          M = 1;
          Y++;
        }
      }
    }
  }অন্যথা যদি( *zDate==0 ){
    p->validHMS = 0;
  }অন্যথা{
    ফেরত 1;
  }
  p->validJD = 0;
  p->validYMD = 1;
  p->Y = Y;
  p->M = M;
  p->D = D;
  computeFloor(p);
  যদি( p->tz ){
    computeJD(p);
  }
  ফেরত 0;
}


স্থির শূন্য clearYMD_HMS_TZ(DateTime *p);  /* Forward declaration */

/*
** Set the time to the current time reported by the VFS.
**
** Return the number of errors.
*/
স্থির পূর্ণসংখ্যা setDateTimeToCurrent(sqlite3_context *context, DateTime *p){
  p->iJD = sqlite3StmtCurrentTime(context);
  যদি( p->iJD>0 ){
    p->validJD = 1;
    p->isUtc = 1;
    p->isLocal = 0;
    clearYMD_HMS_TZ(p);
    ফেরত 0;
  }অন্যথা{
    ফেরত 1;
  }
}

/*
** Input "r" is a numeric quantity which might be a julian day number,
** or the number of seconds since 1970.  If the value if r is within
** range of a julian day number, install it as such and set validJD.
** If the value is a valid unix timestamp, put it in p->s and set p->rawS.
*/
স্থির শূন্য set_Raw_Date_সংখ্যা(DateTime *p, দ্বিগুণ r){
  p->s = r;
  p->rawS = 1;
  যদি( r>=0.0 && r<5373484.5 ){
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
স্থির পূর্ণসংখ্যা parseDateOrTime(
  sqlite3_context *context, 
  ধ্রুবক অক্ষর *zDate, 
  DateTime *p
){
  দ্বিগুণ r;
  যদি( parseYyyyMmDd(zDate,p)==0 ){
    ফেরত 0;
  }অন্যথা যদি( parseHhMmSs(zDate, p)==0 ){
    ফেরত 0;
  }অন্যথা যদি( sqlite3StrICmp(zDate,"now")==0 && sqlite3NotPureFunc(context) ){
    ফেরত setDateTimeToCurrent(context, p);
  }অন্যথা যদি( sqlite3AtoF(zDate, &r)>0 ){
    set_Raw_Date_সংখ্যা(p, r);
    ফেরত 0;
  }অন্যথা যদি( (sqlite3StrICmp(zDate,"subsec")==0
             || sqlite3StrICmp(zDate,"subsecond")==0)
           && sqlite3NotPureFunc(context) ){
    p->useSubsec = 1;
    ফেরত setDateTimeToCurrent(context, p);
  }
  ফেরত 1;
}

/* The julian day number for 9999-12-31 23:59:59.999 is 5373484.4999999.
** Multiplying this by 86400000 gives 464269060799999 as the maximum value
** for DateTime.iJD.
**
** But some older compilers (ex: gcc 4.2.1 on older Macs) cannot deal with 
** such a large integer literal, so we have to encode it.
*/
#define INT_464269060799999  ((((i64)0x1a640)<<32)|0x1072fdff)

/*
** Return TRUE if the given julian day number is within range.
**
** The input is the JulianDay times 86400000.
*/
স্থির পূর্ণসংখ্যা validJulianDay(sqlite3_int64 iJD){
  ফেরত iJD>=0 && iJD<=INT_464269060799999;
}

/*
** Compute the Year, Month, and Day from the julian day number.
*/
স্থির শূন্য computeYMD(DateTime *p){
  পূর্ণসংখ্যা Z, alpha, A, B, C, D, E, X1;
  যদি( p->validYMD ) ফেরত;
  যদি( !p->validJD ){
    p->Y = 2000;
    p->M = 1;
    p->D = 1;
  }অন্যথা যদি( !validJulianDay(p->iJD) ){
    datetimeError(p);
    ফেরত;
  }অন্যথা{
    Z = (পূর্ণসংখ্যা)((p->iJD + 43200000)/86400000);
    alpha = (পূর্ণসংখ্যা)((Z + 32044.75)/36524.25) - 52;
    A = Z + 1 + alpha - ((alpha+100)/4) + 25;
    B = A + 1524;
    C = (পূর্ণসংখ্যা)((B - 122.1)/365.25);
    D = (36525*(C&32767))/100;
    E = (পূর্ণসংখ্যা)((B-D)/30.6001);
    X1 = (পূর্ণসংখ্যা)(30.6001*E);
    p->D = B - D - X1;
    p->M = E<14 ? E-1 : E-13;
    p->Y = p->M>2 ? C - 4716 : C - 4715;
  }
  p->validYMD = 1;
}

/*
** Compute the Hour, Minute, and Seconds from the julian day number.
*/
স্থির শূন্য computeHMS(DateTime *p){
  পূর্ণসংখ্যা day_ms, day_min; /* milliseconds, minutes into the day */
  যদি( p->validHMS ) ফেরত;
  computeJD(p);
  day_ms = (পূর্ণসংখ্যা)((p->iJD + 43200000) % 86400000);
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
স্থির শূন্য computeYMD_HMS(DateTime *p){
  computeYMD(p);
  computeHMS(p);
}

/*
** Clear the YMD and HMS and the TZ
*/
স্থির শূন্য clearYMD_HMS_TZ(DateTime *p){
  p->validYMD = 0;
  p->validHMS = 0;
  p->tz = 0;
}

#ifndef SQLITE_OMIT_LOCALTIME
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
#যদি !HAVE_LOCALTIME_R && !HAVE_LOCALTIME_S \
    && defined(_MSC_VER) && defined(_CRT_INSECURE_DEPRECATE)
#undef  HAVE_LOCALTIME_S
#define HAVE_LOCALTIME_S 1
#endif

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
স্থির পূর্ণসংখ্যা osLocaltime(time_t *t, কাঠামো tm *pTm){
  পূর্ণসংখ্যা rc;
#যদি !HAVE_LOCALTIME_R && !HAVE_LOCALTIME_S
  কাঠামো tm *pX;
#যদি SQLITE_THREADSAFE>0
  sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
  sqlite3_mutex_enter(mutex);
  pX = localtime(t);
#ifndef SQLITE_UNTESTABLE
  যদি( sqlite3GlobalConfig.bLocaltimeFault ){
    যদি( sqlite3GlobalConfig.xAltLocaltime!=0
     && 0==sqlite3GlobalConfig.xAltLocaltime((ধ্রুবক শূন্য*)t,(শূন্য*)pTm)
    ){
      pX = pTm;
    }অন্যথা{
      pX = 0;
    }
  }
#endif
  যদি( pX ) *pTm = *pX;
#যদি SQLITE_THREADSAFE>0
  sqlite3_mutex_leave(mutex);
#endif
  rc = pX==0;
#অন্যথা
#ifndef SQLITE_UNTESTABLE
  যদি( sqlite3GlobalConfig.bLocaltimeFault ){
    যদি( sqlite3GlobalConfig.xAltLocaltime!=0 ){
      ফেরত sqlite3GlobalConfig.xAltLocaltime((ধ্রুবক শূন্য*)t,(শূন্য*)pTm);
    }অন্যথা{
      ফেরত 1;
    }
  }
#endif
#যদি HAVE_LOCALTIME_R
  rc = localtime_r(t, pTm)==0;
#অন্যথা
  rc = localtime_s(pTm, t);
#endif /* HAVE_LOCALTIME_R */
#endif /* HAVE_LOCALTIME_R || HAVE_LOCALTIME_S */
  ফেরত rc;
}
#endif /* SQLITE_OMIT_LOCALTIME */


#ifndef SQLITE_OMIT_LOCALTIME
/*
** Assuming the input DateTime is UTC, move it to its localtime equivalent.
*/
স্থির পূর্ণসংখ্যা toLocaltime(
  DateTime *p,                   /* Date at which to calculate offset */
  sqlite3_context *pCtx          /* Write error here if one occurs */
){
  time_t t;
  কাঠামো tm sLocal;
  পূর্ণসংখ্যা iYearDiff;

  /* Initialize the contents of sLocal to avoid a compiler warning. */
  memset(&sLocal, 0, আকার(sLocal));

  computeJD(p);
  যদি( p->iJD<2108667600*(i64)100000 /* 1970-01-01 */
   || p->iJD>2130141456*(i64)100000 /* 2038-01-18 */
  ){
    /* EVIDENCE-OF: R-55269-29598 The localtime_r() C function normally only
    ** works for years between 1970 and 2037. For dates outside this range,
    ** SQLite attempts to map the year into an equivalent year within this
    ** range, do the calculation, then map the year back.
    */
    DateTime x = *p;
    computeYMD_HMS(&x);
    iYearDiff = (2000 + x.Y%4) - x.Y;
    x.Y += iYearDiff;
    x.validJD = 0;
    computeJD(&x);
    t = (time_t)(x.iJD/1000 -  21086676*(i64)10000);
  }অন্যথা{
    iYearDiff = 0;
    t = (time_t)(p->iJD/1000 -  21086676*(i64)10000);
  }
  যদি( osLocaltime(&t, &sLocal) ){
    sqlite3_ফলাফল_error(pCtx, "local time unavailable", -1);
    ফেরত SQLITE_ERROR;
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
  ফেরত SQLITE_OK;
}
#endif /* SQLITE_OMIT_LOCALTIME */

/*
** The following table defines various date transformations of the form
**
**            'NNN days'
**
** Where NNN is an arbitrary floating-point number and "days" can be one
** of several units of time.
*/
স্থির ধ্রুবক কাঠামো {
  u8 nName;           /* Length of the name */
  অক্ষর zName[7];      /* Name of the transformation */
  দশমিক rLimit;       /* Maximum NNN value for this transform */
  দশমিক rXform;       /* Constant used for this transform */
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
স্থির শূন্য autoAdjustDate(DateTime *p){
  যদি( !p->rawS || p->validJD ){
    p->rawS = 0;
  }অন্যথা যদি( p->s>=-21086676*(i64)10000        /* -4713-11-24 12:00:00 */
         && p->s<=(25340230*(i64)10000)+799   /*  9999-12-31 23:59:59 */
  ){
    দ্বিগুণ r = p->s*1000.0 + 210866760000000.0;
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
স্থির পূর্ণসংখ্যা parseModifier(
  sqlite3_context *pCtx,      /* Function context */
  ধ্রুবক অক্ষর *z,              /* The text of the modifier */
  পূর্ণসংখ্যা n,                      /* Length of zMod in bytes */
  DateTime *p,                /* The date/time value to be modified */
  পূর্ণসংখ্যা idx                     /* Parameter index of the modifier */
){
  পূর্ণসংখ্যা rc = 1;
  দ্বিগুণ r;
  নির্বাচন(sqlite3UpperToLower[(u8)z[0]] ){
    ক্ষেত্র 'a': {
      /*
      **    auto
      **
      ** If rawS is available, then interpret as a julian day number, or
      ** a unix timestamp, depending on its magnitude.
      */
      যদি( sqlite3_stricmp(z, "auto")==0 ){
        যদি( idx>1 ) ফেরত 1; /* IMP: R-33611-57934 */
        autoAdjustDate(p);
        rc = 0;
      }
      বিরতি;
    }
    ক্ষেত্র 'c': {
      /*
      **    ceiling
      **
      ** Resolve day-of-month overflow by rolling forward into the next
      ** month.  As this is the default action, this modifier is really
      ** a no-op that is only included for symmetry.  See "floor".
      */
      যদি( sqlite3_stricmp(z, "ceiling")==0 ){
        computeJD(p);
        clearYMD_HMS_TZ(p);
        rc = 0;
        p->nFloor = 0;
      }
      বিরতি;
    }
    ক্ষেত্র 'e': {
      /*
      **    end of day
      **    end of month
      **    end of year
      **
      ** Move the date forwards to the last millisecond of the current
      ** day, month or year.
      */
      যদি( sqlite3_strnicmp(z, "end of ", 7)!=0 ){
        বিরতি;
      }        
      যদি( !p->validJD && !p->validYMD && !p->validHMS ) বিরতি;
      z += 7;
      computeYMD(p);
      p->validHMS = 1;
      p->h = 23;
      p->m = 59;
      p->s = 59.999;
      p->rawS = 0;
      p->tz = 0;
      p->validJD = 0;
      যদি( sqlite3_stricmp(z,"month")==0 ){
        p->D = 1;
        p->M++;
        যদি( p->M>12 ){
          p->Y++;
          p->M = 1;
        }
        computeFloor(p);
        computeJD(p);
        p->iJD -= (p->nFloor+1)*86400000;
        clearYMD_HMS_TZ(p);
        rc = 0;
      }অন্যথা যদি( sqlite3_stricmp(z,"year")==0 ){
        p->M = 12;
        p->D = 31;
        rc = 0;
      }অন্যথা যদি( sqlite3_stricmp(z,"day")==0 ){
        rc = 0;
      }
      বিরতি;
    }
    ক্ষেত্র 'f': {
      /*
      **    floor
      **
      ** Resolve day-of-month overflow by rolling back to the end of the
      ** previous month.
      */
      যদি( sqlite3_stricmp(z, "floor")==0 ){
        computeJD(p);
        p->iJD -= p->nFloor*86400000;
        clearYMD_HMS_TZ(p);
        rc = 0;
      }
      বিরতি;
    }
    ক্ষেত্র 'j': {
      /*
      **    julianday
      **
      ** Always interpret the prior number as a julian-day value.  If this
      ** is not the first modifier, or if the prior argument is not a numeric
      ** value in the allowed range of julian day numbers understood by
      ** SQLite (0..5373484.5) then the result will be NULL.
      */
      যদি( sqlite3_stricmp(z, "julianday")==0 ){
        যদি( idx>1 ) ফেরত 1;  /* IMP: R-31176-64601 */
        যদি( p->validJD && p->rawS ){
          rc = 0;
          p->rawS = 0;
        }
      }
      বিরতি;
    }
#ifndef SQLITE_OMIT_LOCALTIME
    ক্ষেত্র 'l': {
      /*    localtime
      **
      ** Assuming the current time value is UTC (a.k.a. GMT), shift it to
      ** show local time.
      */
      যদি( sqlite3_stricmp(z, "localtime")==0 && sqlite3NotPureFunc(pCtx) ){
        rc = p->isLocal ? SQLITE_OK : toLocaltime(p, pCtx);
        p->isUtc = 0;
        p->isLocal = 1;
      }
      বিরতি;
    }
#endif
    ক্ষেত্র 'u': {
      /*
      **    unixepoch
      **
      ** Treat the current value of p->s as the number of
      ** seconds since 1970.  Convert to a real julian day number.
      */
      যদি( sqlite3_stricmp(z, "unixepoch")==0 && p->rawS ){
        যদি( idx>1 ) ফেরত 1;  /* IMP: R-49255-55373 */
        r = p->s*1000.0 + 210866760000000.0;
        যদি( r>=0.0 && r<464269060800000.0 ){
          clearYMD_HMS_TZ(p);
          p->iJD = (sqlite3_int64)(r + 0.5);
          p->validJD = 1;
          p->rawS = 0;
          rc = 0;
        }
      }
#ifndef SQLITE_OMIT_LOCALTIME
      অন্যথা যদি( sqlite3_stricmp(z, "utc")==0 && sqlite3NotPureFunc(pCtx) ){
        যদি( p->isUtc==0 ){
          i64 iOrigJD;              /* Original localtime */
          i64 iGuess;               /* Guess at the corresponding utc time */
          পূর্ণসংখ্যা cnt = 0;              /* Safety to prevent infinite loop */
          i64 iErr;                 /* Guess is off by this much */

          computeJD(p);
          iGuess = iOrigJD = p->iJD;
          iErr = 0;
          করো{
            DateTime new;
            memset(&new, 0, আকার(new));
            iGuess -= iErr;
            new.iJD = iGuess;
            new.validJD = 1;
            rc = toLocaltime(&new, pCtx);
            যদি( rc ) ফেরত rc;
            computeJD(&new);
            iErr = new.iJD - iOrigJD;
          }যখন( iErr && cnt++<3 );
          memset(p, 0, আকার(*p));
          p->iJD = iGuess;
          p->validJD = 1;
          p->isUtc = 1;
          p->isLocal = 0;
        }
        rc = SQLITE_OK;
      }
#endif
      বিরতি;
    }
    ক্ষেত্র 'w': {
      /*
      **    weekday N
      **    weekday -N
      **
      ** Move the date forward (for N>=0) or backward (for N<=-0) to the
      ** same time on the next/previous occurrence of weekday abs(N)
      ** where 0==Sunday, 1==Monday, and so forth.  If the date is already
      ** on the appropriate weekday, this is a no-op.
      */
      যদি( sqlite3_strnicmp(z, "weekday ", 8)==0
       && sqlite3AtoF(&z[8], &r)>0
       && r>=-6.0 && r<=6.0
       && sqlite3RealSameAsInt(r,(i64)(n=(পূর্ণসংখ্যা)r))
      ){
        sqlite3_int64 Z;
        computeYMD_HMS(p);
        p->tz = 0;
        p->validJD = 0;
        computeJD(p);
        Z = ((p->iJD + 129600000)/86400000) % 7;
        যদি( n<0 ) n = -n;
        যদি( Z!=n ){
          যদি( Z>n ) Z -= 7;
          p->iJD += (n - Z)*86400000;
          যদি( strchr(z+8,'-')!=0 ) p->iJD -= 7*86400000;
        }
        clearYMD_HMS_TZ(p);
        rc = 0;
      }
      বিরতি;
    }
    ক্ষেত্র 's': {
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
      যদি( sqlite3_strnicmp(z, "start of ", 9)!=0 ){
        যদি( sqlite3_stricmp(z, "subsec")==0
         || sqlite3_stricmp(z, "subsecond")==0
        ){
          p->useSubsec = 1;
          rc = 0;
        }
        বিরতি;
      }        
      যদি( !p->validJD && !p->validYMD && !p->validHMS ) বিরতি;
      z += 9;
      computeYMD(p);
      p->validHMS = 1;
      p->h = p->m = 0;
      p->s = 0.0;
      p->rawS = 0;
      p->tz = 0;
      p->validJD = 0;
      যদি( sqlite3_stricmp(z,"month")==0 ){
        p->D = 1;
        rc = 0;
      }অন্যথা যদি( sqlite3_stricmp(z,"year")==0 ){
        p->M = 1;
        p->D = 1;
        rc = 0;
      }অন্যথা যদি( sqlite3_stricmp(z,"day")==0 ){
        rc = 0;
      }
      বিরতি;
    }
    ক্ষেত্র '+':
    ক্ষেত্র '-':
    ক্ষেত্র '0':
    ক্ষেত্র '1':
    ক্ষেত্র '2':
    ক্ষেত্র '3':
    ক্ষেত্র '4':
    ক্ষেত্র '5':
    ক্ষেত্র '6':
    ক্ষেত্র '7':
    ক্ষেত্র '8':
    ক্ষেত্র '9': {
      দ্বিগুণ rRounder;
      পূর্ণসংখ্যা i, rx;
      পূর্ণসংখ্যা Y,M,D,h,m,x;
      ধ্রুবক অক্ষর *z2 = z;
      অক্ষর *zCopy;
      sqlite3 *db = sqlite3_context_db_handle(pCtx);
      অক্ষর z0 = z[0];
      জন্য(n=1; z[n]; n++){
        যদি( z[n]==':' ) বিরতি;
        যদি( sqlite3Isspace(z[n]) ) বিরতি;
        যদি( z[n]=='-' ){
          যদি( n==5 && getDigits(&z[1], "40f", &Y)==1 ) বিরতি;
          যদি( n==6 && getDigits(&z[1], "50f", &Y)==1 ) বিরতি;
        }
      }
      zCopy = sqlite3DbStrNDup(db, z, n);
      যদি( zCopy==0 ) বিরতি;
      rx = sqlite3AtoF(zCopy, &r)<=0;
      sqlite3DbFree(db, zCopy);
      যদি( rx ){
        প্রতিজ্ঞা( rc==1 );
        বিরতি;
      }
      যদি( z[n]=='-' ){
        /* A modifier of the form (+|-)YYYY-MM-DD adds or subtracts the
        ** specified number of years, months, and days.  MM is limited to
        ** the range 0-11 and DD is limited to 0-30.
        */
        যদি( z0!='+' && z0!='-' ) বিরতি;  /* Must start with +/- */
        যদি( n==5 ){
          যদি( getDigits(&z[1], "40f-20a-20d", &Y, &M, &D)!=3 ) বিরতি;
        }অন্যথা{
          প্রতিজ্ঞা( n==6 );
          যদি( getDigits(&z[1], "50f-20a-20d", &Y, &M, &D)!=3 ) বিরতি;
          z++;
        }
        যদি( M>=12 ) বিরতি;                   /* M range 0..11 */
        যদি( D>=31 ) বিরতি;                   /* D range 0..30 */
        computeYMD_HMS(p);
        p->validJD = 0;
        যদি( z0=='-' ){
          p->Y -= Y;
          p->M -= M;
          D = -D;
        }অন্যথা{
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
        যদি( z[11]==0 ){
          rc = 0;
          বিরতি;
        }
        যদি( sqlite3Isspace(z[11])
         && getDigits(&z[12], "20c:20e", &h, &m)==2
        ){
          z2 = &z[12];
          n = 2;
        }অন্যথা{
          বিরতি;
        }
      }
      যদি( z2[n]==':' ){
        /* A modifier of the form (+|-)HH:MM:SS.FFF adds (or subtracts) the
        ** specified number of hours, minutes, seconds, and fractional seconds
        ** to the time.  The ".FFF" may be omitted.  The ":SS.FFF" may be
        ** omitted.
        */

        DateTime tx;
        sqlite3_int64 day;
        যদি( !sqlite3Isdigit(*z2) ) z2++;
        memset(&tx, 0, আকার(tx));
        যদি( parseHhMmSs(z2, &tx) ) বিরতি;
        computeJD(&tx);
        tx.iJD -= 43200000;
        day = tx.iJD/86400000;
        tx.iJD -= day*86400000;
        যদি( z0=='-' ) tx.iJD = -tx.iJD;
        computeJD(p);
        clearYMD_HMS_TZ(p);
        p->iJD += tx.iJD;
        rc = 0;
        বিরতি;
      }

      /* If control reaches this point, it means the transformation is
      ** one of the forms like "+NNN days".  */
      z += n;
      যখন( sqlite3Isspace(*z) ) z++;
      n = sqlite3Strlen30(z);
      যদি( n<3 || n>10 ) বিরতি;
      যদি( sqlite3UpperToLower[(u8)z[n-1]]=='s' ) n--;
      computeJD(p);
      প্রতিজ্ঞা( rc==1 );
      rRounder = r<0 ? -0.5 : +0.5;
      p->nFloor = 0;
      জন্য(i=0; i<Array_আকার(aXformType); i++){
        যদি( aXformType[i].nName==n
         && sqlite3_strnicmp(aXformType[i].zName, z, n)==0
         && r>-aXformType[i].rLimit && r<aXformType[i].rLimit
        ){
          নির্বাচন( i ){
            ক্ষেত্র 4: { /* Special processing to add months */
              প্রতিজ্ঞা( strcmp(aXformType[4].zName,"month")==0 );
              computeYMD_HMS(p);
              p->M += (পূর্ণসংখ্যা)r;
              x = p->M>0 ? (p->M-1)/12 : (p->M-12)/12;
              p->Y += x;
              p->M -= x*12;
              computeFloor(p);
              p->validJD = 0;
              r -= (পূর্ণসংখ্যা)r;
              বিরতি;
            }
            ক্ষেত্র 5: { /* Special processing to add years */
              পূর্ণসংখ্যা y = (পূর্ণসংখ্যা)r;
              প্রতিজ্ঞা( strcmp(aXformType[5].zName,"year")==0 );
              computeYMD_HMS(p);
              প্রতিজ্ঞা( p->M>=0 && p->M<=12 );
              p->Y += y;
              computeFloor(p);
              p->validJD = 0;
              r -= (পূর্ণসংখ্যা)r;
              বিরতি;
            }
          }
          computeJD(p);
          p->iJD += (sqlite3_int64)(r*1000.0*aXformType[i].rXform + rRounder);
          rc = 0;
          বিরতি;
        }
      }
      clearYMD_HMS_TZ(p);
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      বিরতি;
    }
  }
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা isDate(
  sqlite3_context *context, 
  পূর্ণসংখ্যা argc, 
  sqlite3_মান **argv, 
  DateTime *p
){
  পূর্ণসংখ্যা i, n;
  ধ্রুবক unsigned অক্ষর *z;
  পূর্ণসংখ্যা eType;
  memset(p, 0, আকার(*p));
  যদি( argc==0 ){
    যদি( !sqlite3NotPureFunc(context) ) ফেরত 1;
    ফেরত setDateTimeToCurrent(context, p);
  }
  যদি( (eType = sqlite3_মান_type(argv[0]))==SQLITE_FLOAT
                   || eType==SQLITE_INTEGER ){
    set_Raw_Date_সংখ্যা(p, sqlite3_মান_double(argv[0]));
  }অন্যথা{
    z = sqlite3_মান_text(argv[0]);
    যদি( !z || parseDateOrTime(context, (অক্ষর*)z, p) ){
      ফেরত 1;
    }
  }
  জন্য(i=1; i<argc; i++){
    z = sqlite3_মান_text(argv[i]);
    n = sqlite3_মান_bytes(argv[i]);
    যদি( z==0 || parseModifier(context, (অক্ষর*)z, n, p, i) ) ফেরত 1;
  }
  computeJD(p);
  যদি( p->isError || !validJulianDay(p->iJD) ) ফেরত 1;
  যদি( argc==1 && p->validYMD && p->D>28 ){
    /* Make sure a YYYY-MM-DD is normalized.
    ** Example: 2023-02-31 -> 2023-03-03 */
    প্রতিজ্ঞা( p->validJD );
    p->validYMD = 0;  
  }
  ফেরত 0;
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
স্থির শূন্য juliandayFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  DateTime x;
  যদি( isDate(context, argc, argv, &x)==0 ){
    computeJD(&x);
    sqlite3_ফলাফল_double(context, x.iJD/86400000.0);
  }
}

/*
**    unixepoch( TIMESTRING, MOD, MOD, ...)
**
** Return the number of seconds (including fractional seconds) since
** the unix epoch of 1970-01-01 00:00:00 GMT.
*/
স্থির শূন্য unixepochFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  DateTime x;
  যদি( isDate(context, argc, argv, &x)==0 ){
    computeJD(&x);
    যদি( x.useSubsec ){
      sqlite3_ফলাফল_double(context, (x.iJD - 21086676*(i64)10000000)/1000.0);
    }অন্যথা{
      sqlite3_ফলাফল_int64(context, x.iJD/1000 - 21086676*(i64)10000);
    }
  }
}

/*
**    datetime( TIMESTRING, MOD, MOD, ...)
**
** Return YYYY-MM-DD HH:MM:SS
*/
স্থির শূন্য datetimeFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  DateTime x;
  যদি( isDate(context, argc, argv, &x)==0 ){
    পূর্ণসংখ্যা Y, s, n;
    অক্ষর zBuf[32];
    computeYMD_HMS(&x);
    Y = x.Y;
    যদি( Y<0 ) Y = -Y;
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
    যদি( x.useSubsec ){
      s = (পূর্ণসংখ্যা)(1000.0*x.s + 0.5);
      zBuf[18] = '0' + (s/10000)%10;
      zBuf[19] = '0' + (s/1000)%10;
      zBuf[20] = '.';
      zBuf[21] = '0' + (s/100)%10;
      zBuf[22] = '0' + (s/10)%10;
      zBuf[23] = '0' + (s)%10;
      zBuf[24] = 0;
      n = 24;
    }অন্যথা{
      s = (পূর্ণসংখ্যা)x.s;
      zBuf[18] = '0' + (s/10)%10;
      zBuf[19] = '0' + (s)%10;
      zBuf[20] = 0;
      n = 20;
    }
    যদি( x.Y<0 ){
      zBuf[0] = '-';
      sqlite3_ফলাফল_text(context, zBuf, n, SQLITE_TRANSIENT);
    }অন্যথা{
      sqlite3_ফলাফল_text(context, &zBuf[1], n-1, SQLITE_TRANSIENT);
    }
  }
}

/*
**    time( TIMESTRING, MOD, MOD, ...)
**
** Return HH:MM:SS
*/
স্থির শূন্য timeFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  DateTime x;
  যদি( isDate(context, argc, argv, &x)==0 ){
    পূর্ণসংখ্যা s, n;
    অক্ষর zBuf[16];
    computeHMS(&x);
    zBuf[0] = '0' + (x.h/10)%10;
    zBuf[1] = '0' + (x.h)%10;
    zBuf[2] = ':';
    zBuf[3] = '0' + (x.m/10)%10;
    zBuf[4] = '0' + (x.m)%10;
    zBuf[5] = ':';
    যদি( x.useSubsec ){
      s = (পূর্ণসংখ্যা)(1000.0*x.s + 0.5);
      zBuf[6] = '0' + (s/10000)%10;
      zBuf[7] = '0' + (s/1000)%10;
      zBuf[8] = '.';
      zBuf[9] = '0' + (s/100)%10;
      zBuf[10] = '0' + (s/10)%10;
      zBuf[11] = '0' + (s)%10;
      zBuf[12] = 0;
      n = 12;
    }অন্যথা{
      s = (পূর্ণসংখ্যা)x.s;
      zBuf[6] = '0' + (s/10)%10;
      zBuf[7] = '0' + (s)%10;
      zBuf[8] = 0;
      n = 8;
    }
    sqlite3_ফলাফল_text(context, zBuf, n, SQLITE_TRANSIENT);
  }
}

/*
**    date( TIMESTRING, MOD, MOD, ...)
**
** Return YYYY-MM-DD
*/
স্থির শূন্য dateFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  DateTime x;
  যদি( isDate(context, argc, argv, &x)==0 ){
    পূর্ণসংখ্যা Y;
    অক্ষর zBuf[16];
    computeYMD(&x);
    Y = x.Y;
    যদি( Y<0 ) Y = -Y;
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
    যদি( x.Y<0 ){
      zBuf[0] = '-';
      sqlite3_ফলাফল_text(context, zBuf, 11, SQLITE_TRANSIENT);
    }অন্যথা{
      sqlite3_ফলাফল_text(context, &zBuf[1], 10, SQLITE_TRANSIENT);
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
স্থির পূর্ণসংখ্যা daysAfterJan01(DateTime *pDate){
  DateTime jan01 = *pDate;
  প্রতিজ্ঞা( jan01.validYMD );
  প্রতিজ্ঞা( jan01.validHMS );
  প্রতিজ্ঞা( pDate->validJD );
  jan01.validJD = 0;
  jan01.M = 1;
  jan01.D = 1;
  computeJD(&jan01);
  ফেরত (পূর্ণসংখ্যা)((pDate->iJD-jan01.iJD+43200000)/86400000);
}

/*
** Return the number of days after the most recent Monday.
**
** In other words, return the day of the week according
** to this code:
**
**   0=Monday, 1=Tuesday, 2=Wednesday, ..., 6=Sunday.
*/
স্থির পূর্ণসংখ্যা daysAfterMonday(DateTime *pDate){
  প্রতিজ্ঞা( pDate->validJD );
  ফেরত (পূর্ণসংখ্যা)((pDate->iJD+43200000)/86400000) % 7;
}

/*
** Return the number of days after the most recent Sunday.
**
** In other words, return the day of the week according
** to this code:
**
**   0=Sunday, 1=Monday, 2=Tuesday, ..., 6=Saturday
*/
স্থির পূর্ণসংখ্যা daysAfterSunday(DateTime *pDate){
  প্রতিজ্ঞা( pDate->validJD );
  ফেরত (পূর্ণসংখ্যা)((pDate->iJD+129600000)/86400000) % 7;
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
স্থির শূন্য strftimeFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  DateTime x;
  size_t i,j;
  sqlite3 *db;
  ধ্রুবক অক্ষর *zFmt;
  sqlite3_str *pRes;


  যদি( argc==0 ) ফেরত;
  zFmt = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
  যদি( zFmt==0 || isDate(context, argc-1, argv+1, &x) ) ফেরত;
  db = sqlite3_context_db_handle(context);
  pRes = sqlite3_str_new(db);

  computeJD(&x);
  computeYMD_HMS(&x);
  জন্য(i=j=0; zFmt[i]; i++){
    অক্ষর cf;
    যদি( zFmt[i]!='%' ) চালু;
    যদি( j<i ) sqlite3_str_append(pRes, zFmt+j, (পূর্ণসংখ্যা)(i-j));
    i++;
    j = i + 1;
    cf = zFmt[i];
    নির্বাচন( cf ){
      ক্ষেত্র 'd':  /* Fall thru */
      ক্ষেত্র 'e': {
        sqlite3_str_appendf(pRes, cf=='d' ? "%02d" : "%2d", x.D);
        বিরতি;
      }
      ক্ষেত্র 'f': {  /* Fractional seconds.  (Non-standard) */
        দ্বিগুণ s = x.s;
        যদি( NEVER(s>59.999) ) s = 59.999;
        sqlite3_str_appendf(pRes, "%06.3f", s);
        বিরতি;
      }
      ক্ষেত্র 'F': {
        sqlite3_str_appendf(pRes, "%04d-%02d-%02d", x.Y, x.M, x.D);
        বিরতি;
      }
      ক্ষেত্র 'G': /* Fall thru */
      ক্ষেত্র 'g': {
        DateTime y = x;
        প্রতিজ্ঞা( y.validJD );
        /* Move y so that it is the Thursday in the same week as x */
        y.iJD += (3 - daysAfterMonday(&x))*86400000;
        y.validYMD = 0;
        computeYMD(&y);
        যদি( cf=='g' ){
          sqlite3_str_appendf(pRes, "%02d", y.Y%100);
        }অন্যথা{
          sqlite3_str_appendf(pRes, "%04d", y.Y);
        }
        বিরতি;
      }
      ক্ষেত্র 'H':
      ক্ষেত্র 'k': {
        sqlite3_str_appendf(pRes, cf=='H' ? "%02d" : "%2d", x.h);
        বিরতি;
      }
      ক্ষেত্র 'I': /* Fall thru */
      ক্ষেত্র 'l': {
        পূর্ণসংখ্যা h = x.h;
        যদি( h>12 ) h -= 12;
        যদি( h==0 ) h = 12;
        sqlite3_str_appendf(pRes, cf=='I' ? "%02d" : "%2d", h);
        বিরতি;
      }
      ক্ষেত্র 'j': {  /* Day of year.  Jan01==1, Jan02==2, and so forth */
        sqlite3_str_appendf(pRes,"%03d",daysAfterJan01(&x)+1);
        বিরতি;
      }
      ক্ষেত্র 'J': {  /* Julian day number.  (Non-standard) */
        sqlite3_str_appendf(pRes,"%.16g",x.iJD/86400000.0);
        বিরতি;
      }
      ক্ষেত্র 'm': {
        sqlite3_str_appendf(pRes,"%02d",x.M);
        বিরতি;
      }
      ক্ষেত্র 'M': {
        sqlite3_str_appendf(pRes,"%02d",x.m);
        বিরতি;
      }
      ক্ষেত্র 'p': /* Fall thru */
      ক্ষেত্র 'P': {
        যদি( x.h>=12 ){
          sqlite3_str_append(pRes, cf=='p' ? "PM" : "pm", 2);
        }অন্যথা{
          sqlite3_str_append(pRes, cf=='p' ? "AM" : "am", 2);
        }
        বিরতি;
      }
      ক্ষেত্র 'R': {
        sqlite3_str_appendf(pRes, "%02d:%02d", x.h, x.m);
        বিরতি;
      }
      ক্ষেত্র 's': {
        যদি( x.useSubsec ){
          sqlite3_str_appendf(pRes,"%.3f",
                (x.iJD - 21086676*(i64)10000000)/1000.0);
        }অন্যথা{
          i64 iS = (i64)(x.iJD/1000 - 21086676*(i64)10000);
          sqlite3_str_appendf(pRes,"%lld",iS);
        }
        বিরতি;
      }
      ক্ষেত্র 'S': {
        sqlite3_str_appendf(pRes,"%02d",(পূর্ণসংখ্যা)x.s);
        বিরতি;
      }
      ক্ষেত্র 'T': {
        sqlite3_str_appendf(pRes,"%02d:%02d:%02d", x.h, x.m, (পূর্ণসংখ্যা)x.s);
        বিরতি;
      }
      ক্ষেত্র 'u':    /* Day of week.  1 to 7.  Monday==1, Sunday==7 */
      ক্ষেত্র 'w': {  /* Day of week.  0 to 6.  Sunday==0, Monday==1 */
        অক্ষর c = (অক্ষর)daysAfterSunday(&x) + '0';
        যদি( c=='0' && cf=='u' ) c = '7';
        sqlite3_str_appendchar(pRes, 1, c);
        বিরতি;
      }
      ক্ষেত্র 'U': {  /* Week num. 00-53. First Sun of the year is week 01 */
        sqlite3_str_appendf(pRes,"%02d",
              (daysAfterJan01(&x)-daysAfterSunday(&x)+7)/7);
        বিরতি;
      }
      ক্ষেত্র 'V': {  /* Week num. 01-53. First week with a Thur is week 01 */
        DateTime y = x;
        /* Adjust y so that is the Thursday in the same week as x */
        প্রতিজ্ঞা( y.validJD );
        y.iJD += (3 - daysAfterMonday(&x))*86400000;
        y.validYMD = 0;
        computeYMD(&y);
        sqlite3_str_appendf(pRes,"%02d", daysAfterJan01(&y)/7+1);
        বিরতি;
      }
      ক্ষেত্র 'W': {  /* Week num. 00-53. First Mon of the year is week 01 */
        sqlite3_str_appendf(pRes,"%02d",
           (daysAfterJan01(&x)-daysAfterMonday(&x)+7)/7);
        বিরতি;
      }
      ক্ষেত্র 'Y': {
        sqlite3_str_appendf(pRes,"%04d",x.Y);
        বিরতি;
      }
      ক্ষেত্র '%': {
        sqlite3_str_appendchar(pRes, 1, '%');
        বিরতি;
      }
      পূর্বনির্ধারিত: {
        sqlite3_str_free(pRes);
        ফেরত;
      }
    }
  }
  যদি( j<i ) sqlite3_str_append(pRes, zFmt+j, (পূর্ণসংখ্যা)(i-j));
  sqlite3_ফলাফল_str(context, pRes, SQLITE_FINISH);
}

/*
** current_time()
**
** This function returns the same value as time('now').
*/
স্থির শূন্য ctimeFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা NotUsed,
  sqlite3_মান **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  timeFunc(context, 0, 0);
}

/*
** current_date()
**
** This function returns the same value as date('now').
*/
স্থির শূন্য cdateFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা NotUsed,
  sqlite3_মান **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
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
স্থির শূন্য timediffFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা NotUsed1,
  sqlite3_মান **argv
){
  অক্ষর sign;
  পূর্ণসংখ্যা Y, M;
  DateTime d1, d2;
  sqlite3_str sRes;
  UNUSED_PARAMETER(NotUsed1);
  যদি( isDate(context, 1, &argv[0], &d1) ) ফেরত;
  যদি( isDate(context, 1, &argv[1], &d2) ) ফেরত;
  computeYMD_HMS(&d1);
  computeYMD_HMS(&d2);
  যদি( d1.iJD>=d2.iJD ){
    sign = '+';
    Y = d1.Y - d2.Y;
    যদি( Y ){
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d1.M - d2.M;
    যদি( M<0 ){
      Y--;
      M += 12;
    }
    যদি( M!=0 ){
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    যখন( d1.iJD<d2.iJD ){
      M--;
      যদি( M<0 ){
        M = 11;
        Y--;
      }
      d2.M--;
      যদি( d2.M<1 ){
        d2.M = 12;
        d2.Y--;
      }
      d2.validJD = 0;
      computeJD(&d2);
    }
    d1.iJD -= d2.iJD;
    d1.iJD += (u64)1486995408 * (u64)100000;
  }অন্যথা /* d1<d2 */{
    sign = '-';
    Y = d2.Y - d1.Y;
    যদি( Y ){
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d2.M - d1.M;
    যদি( M<0 ){
      Y--;
      M += 12;
    }
    যদি( M!=0 ){
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    যখন( d1.iJD>d2.iJD ){
      M--;
      যদি( M<0 ){
        M = 11;
        Y--;
      }
      d2.M++;
      যদি( d2.M>12 ){
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
  sqlite3_ফলাফল_str(context, &sRes, SQLITE_XFER);
}


/*
** current_timestamp()
**
** This function returns the same value as datetime('now').
*/
স্থির শূন্য ctimestampFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা NotUsed,
  sqlite3_মান **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  datetimeFunc(context, 0, 0);
}
#endif /* !defined(SQLITE_OMIT_DATETIME_FUNCS) */

#ifdef SQLITE_OMIT_DATETIME_FUNCS
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
স্থির শূন্য currentTimeFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  time_t t;
  অক্ষর *zFormat = (অক্ষর *)sqlite3_user_তথ্য(context);
  sqlite3_int64 iT;
  কাঠামো tm *pTm;
  কাঠামো tm sNow;
  অক্ষর zBuf[20];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);

  iT = sqlite3StmtCurrentTime(context);
  যদি( iT<=0 ) ফেরত;
  t = iT/1000 - 10000*(sqlite3_int64)21086676;
#যদি HAVE_GMTIME_R
  pTm = gmtime_r(&t, &sNow);
#অন্যথা
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
  pTm = gmtime(&t);
  যদি( pTm ) memcpy(&sNow, pTm, আকার(sNow));
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
#endif
  যদি( pTm ){
    strftime(zBuf, 20, zFormat, &sNow);
    sqlite3_ফলাফল_text(context, zBuf, -1, SQLITE_TRANSIENT);
  }
}
#endif

#যদি !defined(SQLITE_OMIT_DATETIME_FUNCS) && defined(SQLITE_DEBUG)
/*
**   datedebug(...)
**
** This routine returns JSON that describes the internal DateTime object.
** Used for debugging and testing only.  Subject to change.
*/
স্থির শূন্য datedebugFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  DateTime x;
  যদি( isDate(context, argc, argv, &x)==0 ){
    অক্ষর *zJson;
    zJson = sqlite3_mprintf(
      "{iJD:%lld,Y:%d,M:%d,D:%d,h:%d,m:%d,tz:%d,"
      "s:%.3f,validJD:%d,validYMD:%d,validHMS:%d,"
      "nFloor:%d,rawS:%d,isError:%d,useSubsec:%d,"
      "isUtc:%d,isLocal:%d}",
      x.iJD, x.Y, x.M, x.D, x.h, x.m, x.tz,
      x.s, x.validJD, x.validYMD, x.validHMS,
      x.nFloor, x.rawS, x.isError, x.useSubsec,
      x.isUtc, x.isLocal);
    sqlite3_ফলাফল_text(context, zJson, -1, sqlite3_free);
  }
}
#endif /* !SQLITE_OMIT_DATETIME_FUNCS && SQLITE_DEBUG */


/*
** This function registered all of the above C functions as SQL
** functions.  This should be the only routine in this file with
** external linkage.
*/
শূন্য sqlite3RegisterDateTimeFunctions(শূন্য){
  স্থির FuncDef aDateTimeFuncs[] = {
#ifndef SQLITE_OMIT_DATETIME_FUNCS
    PURE_DATE(julianday,        -1, 0, 0, juliandayFunc ),
    PURE_DATE(unixepoch,        -1, 0, 0, unixepochFunc ),
    PURE_DATE(date,             -1, 0, 0, dateFunc      ),
    PURE_DATE(time,             -1, 0, 0, timeFunc      ),
    PURE_DATE(datetime,         -1, 0, 0, datetimeFunc  ),
    PURE_DATE(strftime,         -1, 0, 0, strftimeFunc  ),
    PURE_DATE(timediff,          2, 0, 0, timediffFunc  ),
#ifdef SQLITE_DEBUG
    PURE_DATE(datedebug,        -1, 0, 0, datedebugFunc ),
#endif
    DFUNCTION(current_time,      0, 0, 0, ctimeFunc     ),
    DFUNCTION(current_timestamp, 0, 0, 0, ctimestampFunc),
    DFUNCTION(current_date,      0, 0, 0, cdateFunc     ),
#অন্যথা
    STR_FUNCTION(current_time,      0, "%H:%M:%S",          0, currentTimeFunc),
    STR_FUNCTION(current_date,      0, "%Y-%m-%d",          0, currentTimeFunc),
    STR_FUNCTION(current_timestamp, 0, "%Y-%m-%d %H:%M:%S", 0, currentTimeFunc),
#endif
  };
  sqlite3_ঢোকাও_Builtin_Funcs(aDateTimeFuncs, Array_আকার(aDateTimeFuncs));
}
