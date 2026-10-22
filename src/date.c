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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ <time.h>

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DATETIME_FUNCS

/*
** The MSVC CRT on Windows CE may not have a localtime() function.
** So declare a substitute.  The substitute function itself is
** defined in "os_win.c".
*/
#ఒకవేళ !defined(SQLITE_OMIT_LOCALTIME) && defined(_WIN32_WCE) && \
    (!defined(SQLITE_MSVC_LOCALTIME_API) || !SQLITE_MSVC_LOCALTIME_API)
నిర్మాణం tm *__cdecl localtime(స్థిరరాశి సమయం_t *);
#అయితే_ముగింపు

/*
** A structure for holding a single date and time.
*/
రకనిర్వచనం నిర్మాణం Date_సమయం Date_సమయం;
నిర్మాణం Date_సమయం {
  sqlite3_int64 iJD;  /* The julian day number times 86400000 */
  పూర్ణాంకం Y, M, D;        /* Year, month, and day */
  పూర్ణాంకం h, m;           /* Hour and minutes */
  పూర్ణాంకం tz;             /* Timezone offset in minutes */
  ద్విగుణ s;           /* Seconds */
  అక్షరం validJD;       /* True (1) if iJD is valid */
  అక్షరం validYMD;      /* True (1) if Y,M,D are valid */
  అక్షరం validHMS;      /* True (1) if h,m,s are valid */
  అక్షరం nFloor;            /* Days to implement "floor" */
  గుర్తులేని rawS      : 1; /* Raw numeric value stored in s */
  గుర్తులేని isError   : 1; /* An overflow has occurred */
  గుర్తులేని useSubsec : 1; /* Display subsecond precision */
  గుర్తులేని isUtc     : 1; /* Time is known to be UTC */
  గుర్తులేని isLocal   : 1; /* Time is known to be localtime */
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
స్థిర పూర్ణాంకం getDigits(స్థిరరాశి అక్షరం *zDate, స్థిరరాశి అక్షరం *zFormat, ...){
  /* The aMx[] array translates the 3rd character of each format
  ** spec into a max size:    a   b   c   d   e      f */
  స్థిర స్థిరరాశి u16 aMx[] = { 12, 14, 24, 31, 59, 14712 };
  va_list ap;
  పూర్ణాంకం cnt = 0;
  అక్షరం తదుపరి_C;
  va_start(ap, zFormat);
  చేయి{
    అక్షరం N = zFormat[0] - '0';
    అక్షరం min = zFormat[1] - '0';
    పూర్ణాంకం val = 0;
    u16 max;

    ప్రతిజ్ఞా( zFormat[2]>='a' && zFormat[2]<='f' );
    max = aMx[zFormat[2] - 'a'];
    తదుపరి_C = zFormat[3];
    val = 0;
    అయితే( N-- ){
      ఒకవేళ( !sqlite3Isdigit(*zDate) ){
        వెళ్ళు చివర_get_Digits;
      }
      val = val*10 + *zDate - '0';
      zDate++;
    }
    ఒకవేళ( val<(పూర్ణాంకం)min || val>(పూర్ణాంకం)max || (తదుపరి_C!=0 && తదుపరి_C!=*zDate) ){
      వెళ్ళు చివర_get_Digits;
    }
    *va_arg(ap,పూర్ణాంకం*) = val;
    zDate++;
    cnt++;
    zFormat += 4;
  }అయితే( తదుపరి_C );
చివర_get_Digits:
  va_end(ap);
  తిరిగి cnt;
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
స్థిర పూర్ణాంకం parseTimezone(స్థిరరాశి అక్షరం *zDate, Date_సమయం *p){
  పూర్ణాంకం sgn = 0;
  పూర్ణాంకం nHr, nMn;
  పూర్ణాంకం c;
  అయితే( sqlite3Isspace(*zDate) ){ zDate++; }
  p->tz = 0;
  c = *zDate;
  ఒకవేళ( c=='-' ){
    sgn = -1;
  }లేకపోతే ఒకవేళ( c=='+' ){
    sgn = +1;
  }లేకపోతే ఒకవేళ( c=='Z' || c=='z' ){
    zDate++;
    p->isLocal = 0;
    p->isUtc = 1;
    వెళ్ళు zulu_సమయం;
  }లేకపోతే{
    తిరిగి c!=0;
  }
  zDate++;
  ఒకవేళ( getDigits(zDate, "20b:20e", &nHr, &nMn)!=2 ){
    తిరిగి 1;
  }
  zDate += 5;
  p->tz = sgn*(nMn + nHr*60);
  ఒకవేళ( p->tz==0 ){   /* Forum post 2025-09-17T10:12:14z */
    p->isLocal = 0;
    p->isUtc = 1;
  }
zulu_సమయం:
  అయితే( sqlite3Isspace(*zDate) ){ zDate++; }
  తిరిగి *zDate!=0;
}

/*
** Parse times of the form HH:MM or HH:MM:SS or HH:MM:SS.FFFF.
** The HH, MM, and SS must each be exactly 2 digits.  The
** fractional seconds FFFF can be one or more digits.
**
** Return 1 if there is a parsing error and 0 on success.
*/
స్థిర పూర్ణాంకం parseHhMmSs(స్థిరరాశి అక్షరం *zDate, Date_సమయం *p){
  పూర్ణాంకం h, m, s;
  ద్విగుణ ms = 0.0;
  ఒకవేళ( getDigits(zDate, "20c:20e", &h, &m)!=2 ){
    తిరిగి 1;
  }
  zDate += 5;
  ఒకవేళ( *zDate==':' ){
    zDate++;
    ఒకవేళ( getDigits(zDate, "20e", &s)!=1 ){
      తిరిగి 1;
    }
    zDate += 2;
    ఒకవేళ( *zDate=='.' && sqlite3Isdigit(zDate[1]) ){
      ద్విగుణ rScale = 1.0;
      zDate++;
      అయితే( sqlite3Isdigit(*zDate) ){
        ms = ms*10.0 + *zDate - '0';
        rScale *= 10.0;
        zDate++;
      }
      ms /= rScale;
      /* Truncate to avoid problems with sub-milliseconds
      ** rounding. https://sqlite.org/forum/forumpost/766a2c9231 */
      ఒకవేళ( ms>0.999 ) ms = 0.999;
    }
  }లేకపోతే{
    s = 0;
  }
  ఒకవేళ( h==24 && (m!=0 || s!=0 || ms!=0.0) ) తిరిగి 1;
  p->validJD = 0;
  p->rawS = 0;
  p->validHMS = 1;
  p->h = h;
  p->m = m;
  p->s = s + ms;
  ఒకవేళ( parseTimezone(zDate, p) ) తిరిగి 1;
  తిరిగి 0;
}

/*
** Put the DateTime object into its error state.
*/
స్థిర శూన్య datetimeError(Date_సమయం *p){
  memset(p, 0, పరిమాణం(*p));
  p->isError = 1;
}

/*
** Convert from YYYY-MM-DD HH:MM:SS to julian day.  We always assume
** that the YYYY-MM-DD is according to the Gregorian calendar.
**
** Reference:  Meeus page 61
*/
స్థిర శూన్య computeJD(Date_సమయం *p){
  పూర్ణాంకం Y, M, D, A, B, X1, X2;

  ఒకవేళ( p->validJD ) తిరిగి;
  ఒకవేళ( p->validYMD ){
    Y = p->Y;
    M = p->M;
    D = p->D;
  }లేకపోతే{
    Y = 2000;  /* If no YMD specified, assume 2000-Jan-01 */
    M = 1;
    D = 1;
    ఒకవేళ( p->validHMS && p->h==24 ){
      p->h = 0;
      D = 2;
    }
  }
  ఒకవేళ( Y<-4713 || Y>9999 || p->rawS ){
    datetimeError(p);
    తిరిగి;
  }
  ఒకవేళ( M<=2 ){
    Y--;
    M += 12;
  }
  A = (Y+4800)/100;
  B = 38 - A + (A/4);
  X1 = 36525*(Y+4716)/100;
  X2 = 306001*(M+1)/10000;
  p->iJD = (sqlite3_int64)((X1 + X2 + D + B - 1524.5 ) * 86400000);
  p->validJD = 1;
  ఒకవేళ( p->validHMS ){
    p->iJD += p->h*3600000 + p->m*60000 + (sqlite3_int64)(p->s*1000 + 0.5);
    ఒకవేళ( p->tz ){
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
స్థిర పూర్ణాంకం days_యొక్క_Month(పూర్ణాంకం Y, పూర్ణాంకం M){
  ఒకవేళ( (1<<M) & 0x15aa ){
    తిరిగి 31;
  }లేకపోతే ఒకవేళ( M!=2 ){
    తిరిగి 30;
  }లేకపోతే ఒకవేళ( Y%4==0 && (Y%100!=0 || Y%400==0) ){
    తిరిగి 29;
  }లేకపోతే{
    తిరిగి 28;
  }
}

/*
** Given the YYYY-MM-DD information current in p, determine if there
** is day-of-month overflow and set nFloor to the number of days that
** would need to be subtracted from the date in order to bring the
** date back to the end of the month.
*/
స్థిర శూన్య computeFloor(Date_సమయం *p){
  పూర్ణాంకం mxD;         /* Maximum day number for the current month */
  ప్రతిజ్ఞా( p->validYMD || p->isError );
  ప్రతిజ్ఞా( p->D>=0 && p->D<=32 );
  ప్రతిజ్ఞా( p->M>=0 && p->M<=12 );
  mxD = days_యొక్క_Month(p->Y, p->M);
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
స్థిర పూర్ణాంకం parseYyyyMmDd(స్థిరరాశి అక్షరం *zDate, Date_సమయం *p){
  పూర్ణాంకం Y, M, D, neg;

  ఒకవేళ( zDate[0]=='-' ){
    zDate++;
    neg = 1;
  }లేకపోతే{
    neg = 0;
  }
  ఒకవేళ( getDigits(zDate, "40f-21a-21d", &Y, &M, &D)!=3 ){
    తిరిగి 1;
  }
  ఒకవేళ( neg ) Y = -Y;
  zDate += 10;
  అయితే( sqlite3Isspace(*zDate) || 'T'==*(u8*)zDate ){ zDate++; }
  ఒకవేళ( parseHhMmSs(zDate, p)==0 ){
    ఒకవేళ( p->h==24 ){
      పూర్ణాంకం mxD = days_యొక్క_Month(Y,M);
      p->h = 0;
      D++;
      ఒకవేళ( D>mxD ){
        D -= mxD;
        M++;
        ఒకవేళ( M>12 ){
          M = 1;
          Y++;
        }
      }
    }
  }లేకపోతే ఒకవేళ( *zDate==0 ){
    p->validHMS = 0;
  }లేకపోతే{
    తిరిగి 1;
  }
  p->validJD = 0;
  p->validYMD = 1;
  p->Y = Y;
  p->M = M;
  p->D = D;
  computeFloor(p);
  ఒకవేళ( p->tz ){
    computeJD(p);
  }
  తిరిగి 0;
}


స్థిర శూన్య clearYMD_HMS_TZ(Date_సమయం *p);  /* Forward declaration */

/*
** Set the time to the current time reported by the VFS.
**
** Return the number of errors.
*/
స్థిర పూర్ణాంకం సమితి_Date_సమయం_గమ్యం_Current(sqlite3_context *context, Date_సమయం *p){
  p->iJD = sqlite3_Stmt_Current_సమయం(context);
  ఒకవేళ( p->iJD>0 ){
    p->validJD = 1;
    p->isUtc = 1;
    p->isLocal = 0;
    clearYMD_HMS_TZ(p);
    తిరిగి 0;
  }లేకపోతే{
    తిరిగి 1;
  }
}

/*
** Input "r" is a numeric quantity which might be a julian day number,
** or the number of seconds since 1970.  If the value if r is within
** range of a julian day number, install it as such and set validJD.
** If the value is a valid unix timestamp, put it in p->s and set p->rawS.
*/
స్థిర శూన్య సమితి_Raw_Date_సంఖ్య(Date_సమయం *p, ద్విగుణ r){
  p->s = r;
  p->rawS = 1;
  ఒకవేళ( r>=0.0 && r<5373484.5 ){
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
స్థిర పూర్ణాంకం parse_Date_Or_సమయం(
  sqlite3_context *context, 
  స్థిరరాశి అక్షరం *zDate, 
  Date_సమయం *p
){
  ద్విగుణ r;
  ఒకవేళ( parseYyyyMmDd(zDate,p)==0 ){
    తిరిగి 0;
  }లేకపోతే ఒకవేళ( parseHhMmSs(zDate, p)==0 ){
    తిరిగి 0;
  }లేకపోతే ఒకవేళ( sqlite3StrICmp(zDate,"now")==0 && sqlite3_Not_శుద్ధ_Func(context) ){
    తిరిగి సమితి_Date_సమయం_గమ్యం_Current(context, p);
  }లేకపోతే ఒకవేళ( sqlite3AtoF(zDate, &r)>0 ){
    సమితి_Raw_Date_సంఖ్య(p, r);
    తిరిగి 0;
  }లేకపోతే ఒకవేళ( (sqlite3StrICmp(zDate,"subsec")==0
             || sqlite3StrICmp(zDate,"subsecond")==0)
           && sqlite3_Not_శుద్ధ_Func(context) ){
    p->useSubsec = 1;
    తిరిగి సమితి_Date_సమయం_గమ్యం_Current(context, p);
  }
  తిరిగి 1;
}

/* The julian day number for 9999-12-31 23:59:59.999 is 5373484.4999999.
** Multiplying this by 86400000 gives 464269060799999 as the maximum value
** for DateTime.iJD.
**
** But some older compilers (ex: gcc 4.2.1 on older Macs) cannot deal with 
** such a large integer literal, so we have to encode it.
*/
#నిర్వచించు INT_464269060799999  ((((i64)0x1a640)<<32)|0x1072fdff)

/*
** Return TRUE if the given julian day number is within range.
**
** The input is the JulianDay times 86400000.
*/
స్థిర పూర్ణాంకం validJulianDay(sqlite3_int64 iJD){
  తిరిగి iJD>=0 && iJD<=INT_464269060799999;
}

/*
** Compute the Year, Month, and Day from the julian day number.
*/
స్థిర శూన్య computeYMD(Date_సమయం *p){
  పూర్ణాంకం Z, alpha, A, B, C, D, E, X1;
  ఒకవేళ( p->validYMD ) తిరిగి;
  ఒకవేళ( !p->validJD ){
    p->Y = 2000;
    p->M = 1;
    p->D = 1;
  }లేకపోతే ఒకవేళ( !validJulianDay(p->iJD) ){
    datetimeError(p);
    తిరిగి;
  }లేకపోతే{
    Z = (పూర్ణాంకం)((p->iJD + 43200000)/86400000);
    alpha = (పూర్ణాంకం)((Z + 32044.75)/36524.25) - 52;
    A = Z + 1 + alpha - ((alpha+100)/4) + 25;
    B = A + 1524;
    C = (పూర్ణాంకం)((B - 122.1)/365.25);
    D = (36525*(C&32767))/100;
    E = (పూర్ణాంకం)((B-D)/30.6001);
    X1 = (పూర్ణాంకం)(30.6001*E);
    p->D = B - D - X1;
    p->M = E<14 ? E-1 : E-13;
    p->Y = p->M>2 ? C - 4716 : C - 4715;
  }
  p->validYMD = 1;
}

/*
** Compute the Hour, Minute, and Seconds from the julian day number.
*/
స్థిర శూన్య computeHMS(Date_సమయం *p){
  పూర్ణాంకం day_ms, day_min; /* milliseconds, minutes into the day */
  ఒకవేళ( p->validHMS ) తిరిగి;
  computeJD(p);
  day_ms = (పూర్ణాంకం)((p->iJD + 43200000) % 86400000);
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
స్థిర శూన్య computeYMD_HMS(Date_సమయం *p){
  computeYMD(p);
  computeHMS(p);
}

/*
** Clear the YMD and HMS and the TZ
*/
స్థిర శూన్య clearYMD_HMS_TZ(Date_సమయం *p){
  p->validYMD = 0;
  p->validHMS = 0;
  p->tz = 0;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_LOCALTIME
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
#ఒకవేళ !HAVE_LOCALTIME_R && !HAVE_LOCALTIME_S \
    && defined(_MSC_VER) && defined(_CRT_INSECURE_DEPRECATE)
#నిర్వచనం_రద్దు  HAVE_LOCALTIME_S
#నిర్వచించు HAVE_LOCALTIME_S 1
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం osLocaltime(సమయం_t *t, నిర్మాణం tm *pTm){
  పూర్ణాంకం rc;
#ఒకవేళ !HAVE_LOCALTIME_R && !HAVE_LOCALTIME_S
  నిర్మాణం tm *pX;
#ఒకవేళ SQLITE_THREADSAFE>0
  sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#అయితే_ముగింపు
  sqlite3_mutex_enter(mutex);
  pX = localtime(t);
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
  ఒకవేళ( sqlite3GlobalConfig.bLocaltimeFault ){
    ఒకవేళ( sqlite3GlobalConfig.xAltLocaltime!=0
     && 0==sqlite3GlobalConfig.xAltLocaltime((స్థిరరాశి శూన్య*)t,(శూన్య*)pTm)
    ){
      pX = pTm;
    }లేకపోతే{
      pX = 0;
    }
  }
#అయితే_ముగింపు
  ఒకవేళ( pX ) *pTm = *pX;
#ఒకవేళ SQLITE_THREADSAFE>0
  sqlite3_mutex_leave(mutex);
#అయితే_ముగింపు
  rc = pX==0;
#లేకపోతే
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
  ఒకవేళ( sqlite3GlobalConfig.bLocaltimeFault ){
    ఒకవేళ( sqlite3GlobalConfig.xAltLocaltime!=0 ){
      తిరిగి sqlite3GlobalConfig.xAltLocaltime((స్థిరరాశి శూన్య*)t,(శూన్య*)pTm);
    }లేకపోతే{
      తిరిగి 1;
    }
  }
#అయితే_ముగింపు
#ఒకవేళ HAVE_LOCALTIME_R
  rc = localtime_r(t, pTm)==0;
#లేకపోతే
  rc = localtime_s(pTm, t);
#అయితే_ముగింపు /* HAVE_LOCALTIME_R */
#అయితే_ముగింపు /* HAVE_LOCALTIME_R || HAVE_LOCALTIME_S */
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_LOCALTIME */


#నిర్వచితం_కాకపోతే SQLITE_OMIT_LOCALTIME
/*
** Assuming the input DateTime is UTC, move it to its localtime equivalent.
*/
స్థిర పూర్ణాంకం గమ్యం_Localtime(
  Date_సమయం *p,                   /* Date at which to calculate offset */
  sqlite3_context *pCtx          /* Write error here if one occurs */
){
  సమయం_t t;
  నిర్మాణం tm sLocal;
  పూర్ణాంకం iYearDiff;

  /* Initialize the contents of sLocal to avoid a compiler warning. */
  memset(&sLocal, 0, పరిమాణం(sLocal));

  computeJD(p);
  ఒకవేళ( p->iJD<2108667600*(i64)100000 /* 1970-01-01 */
   || p->iJD>2130141456*(i64)100000 /* 2038-01-18 */
  ){
    /* EVIDENCE-OF: R-55269-29598 The localtime_r() C function normally only
    ** works for years between 1970 and 2037. For dates outside this range,
    ** SQLite attempts to map the year into an equivalent year within this
    ** range, do the calculation, then map the year back.
    */
    Date_సమయం x = *p;
    computeYMD_HMS(&x);
    iYearDiff = (2000 + x.Y%4) - x.Y;
    x.Y += iYearDiff;
    x.validJD = 0;
    computeJD(&x);
    t = (సమయం_t)(x.iJD/1000 -  21086676*(i64)10000);
  }లేకపోతే{
    iYearDiff = 0;
    t = (సమయం_t)(p->iJD/1000 -  21086676*(i64)10000);
  }
  ఒకవేళ( osLocaltime(&t, &sLocal) ){
    sqlite3_ఫలితం_error(pCtx, "local time unavailable", -1);
    తిరిగి SQLITE_ERROR;
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
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_OMIT_LOCALTIME */

/*
** The following table defines various date transformations of the form
**
**            'NNN days'
**
** Where NNN is an arbitrary floating-point number and "days" can be one
** of several units of time.
*/
స్థిర స్థిరరాశి నిర్మాణం {
  u8 nName;           /* Length of the name */
  అక్షరం zName[7];      /* Name of the transformation */
  దశాంశం r_హద్దు;       /* Maximum NNN value for this transform */
  దశాంశం rXform;       /* Constant used for this transform */
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
స్థిర శూన్య autoAdjustDate(Date_సమయం *p){
  ఒకవేళ( !p->rawS || p->validJD ){
    p->rawS = 0;
  }లేకపోతే ఒకవేళ( p->s>=-21086676*(i64)10000        /* -4713-11-24 12:00:00 */
         && p->s<=(25340230*(i64)10000)+799   /*  9999-12-31 23:59:59 */
  ){
    ద్విగుణ r = p->s*1000.0 + 210866760000000.0;
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
స్థిర పూర్ణాంకం parseModifier(
  sqlite3_context *pCtx,      /* Function context */
  స్థిరరాశి అక్షరం *z,              /* The text of the modifier */
  పూర్ణాంకం n,                      /* Length of zMod in bytes */
  Date_సమయం *p,                /* The date/time value to be modified */
  పూర్ణాంకం idx                     /* Parameter index of the modifier */
){
  పూర్ణాంకం rc = 1;
  ద్విగుణ r;
  ఎంపిక(sqlite3_పై_గమ్యం_క్రింది[(u8)z[0]] ){
    స్థితి 'a': {
      /*
      **    auto
      **
      ** If rawS is available, then interpret as a julian day number, or
      ** a unix timestamp, depending on its magnitude.
      */
      ఒకవేళ( sqlite3_stricmp(z, "auto")==0 ){
        ఒకవేళ( idx>1 ) తిరిగి 1; /* IMP: R-33611-57934 */
        autoAdjustDate(p);
        rc = 0;
      }
      ఆపు;
    }
    స్థితి 'c': {
      /*
      **    ceiling
      **
      ** Resolve day-of-month overflow by rolling forward into the next
      ** month.  As this is the default action, this modifier is really
      ** a no-op that is only included for symmetry.  See "floor".
      */
      ఒకవేళ( sqlite3_stricmp(z, "ceiling")==0 ){
        computeJD(p);
        clearYMD_HMS_TZ(p);
        rc = 0;
        p->nFloor = 0;
      }
      ఆపు;
    }
    స్థితి 'e': {
      /*
      **    end of day
      **    end of month
      **    end of year
      **
      ** Move the date forwards to the last millisecond of the current
      ** day, month or year.
      */
      ఒకవేళ( sqlite3_strnicmp(z, "end of ", 7)!=0 ){
        ఆపు;
      }        
      ఒకవేళ( !p->validJD && !p->validYMD && !p->validHMS ) ఆపు;
      z += 7;
      computeYMD(p);
      p->validHMS = 1;
      p->h = 23;
      p->m = 59;
      p->s = 59.999;
      p->rawS = 0;
      p->tz = 0;
      p->validJD = 0;
      ఒకవేళ( sqlite3_stricmp(z,"month")==0 ){
        p->D = 1;
        p->M++;
        ఒకవేళ( p->M>12 ){
          p->Y++;
          p->M = 1;
        }
        computeFloor(p);
        computeJD(p);
        p->iJD -= (p->nFloor+1)*86400000;
        clearYMD_HMS_TZ(p);
        rc = 0;
      }లేకపోతే ఒకవేళ( sqlite3_stricmp(z,"year")==0 ){
        p->M = 12;
        p->D = 31;
        rc = 0;
      }లేకపోతే ఒకవేళ( sqlite3_stricmp(z,"day")==0 ){
        rc = 0;
      }
      ఆపు;
    }
    స్థితి 'f': {
      /*
      **    floor
      **
      ** Resolve day-of-month overflow by rolling back to the end of the
      ** previous month.
      */
      ఒకవేళ( sqlite3_stricmp(z, "floor")==0 ){
        computeJD(p);
        p->iJD -= p->nFloor*86400000;
        clearYMD_HMS_TZ(p);
        rc = 0;
      }
      ఆపు;
    }
    స్థితి 'j': {
      /*
      **    julianday
      **
      ** Always interpret the prior number as a julian-day value.  If this
      ** is not the first modifier, or if the prior argument is not a numeric
      ** value in the allowed range of julian day numbers understood by
      ** SQLite (0..5373484.5) then the result will be NULL.
      */
      ఒకవేళ( sqlite3_stricmp(z, "julianday")==0 ){
        ఒకవేళ( idx>1 ) తిరిగి 1;  /* IMP: R-31176-64601 */
        ఒకవేళ( p->validJD && p->rawS ){
          rc = 0;
          p->rawS = 0;
        }
      }
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_LOCALTIME
    స్థితి 'l': {
      /*    localtime
      **
      ** Assuming the current time value is UTC (a.k.a. GMT), shift it to
      ** show local time.
      */
      ఒకవేళ( sqlite3_stricmp(z, "localtime")==0 && sqlite3_Not_శుద్ధ_Func(pCtx) ){
        rc = p->isLocal ? SQLITE_OK : గమ్యం_Localtime(p, pCtx);
        p->isUtc = 0;
        p->isLocal = 1;
      }
      ఆపు;
    }
#అయితే_ముగింపు
    స్థితి 'u': {
      /*
      **    unixepoch
      **
      ** Treat the current value of p->s as the number of
      ** seconds since 1970.  Convert to a real julian day number.
      */
      ఒకవేళ( sqlite3_stricmp(z, "unixepoch")==0 && p->rawS ){
        ఒకవేళ( idx>1 ) తిరిగి 1;  /* IMP: R-49255-55373 */
        r = p->s*1000.0 + 210866760000000.0;
        ఒకవేళ( r>=0.0 && r<464269060800000.0 ){
          clearYMD_HMS_TZ(p);
          p->iJD = (sqlite3_int64)(r + 0.5);
          p->validJD = 1;
          p->rawS = 0;
          rc = 0;
        }
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_LOCALTIME
      లేకపోతే ఒకవేళ( sqlite3_stricmp(z, "utc")==0 && sqlite3_Not_శుద్ధ_Func(pCtx) ){
        ఒకవేళ( p->isUtc==0 ){
          i64 iOrigJD;              /* Original localtime */
          i64 iGuess;               /* Guess at the corresponding utc time */
          పూర్ణాంకం cnt = 0;              /* Safety to prevent infinite loop */
          i64 iErr;                 /* Guess is off by this much */

          computeJD(p);
          iGuess = iOrigJD = p->iJD;
          iErr = 0;
          చేయి{
            Date_సమయం new;
            memset(&new, 0, పరిమాణం(new));
            iGuess -= iErr;
            new.iJD = iGuess;
            new.validJD = 1;
            rc = గమ్యం_Localtime(&new, pCtx);
            ఒకవేళ( rc ) తిరిగి rc;
            computeJD(&new);
            iErr = new.iJD - iOrigJD;
          }అయితే( iErr && cnt++<3 );
          memset(p, 0, పరిమాణం(*p));
          p->iJD = iGuess;
          p->validJD = 1;
          p->isUtc = 1;
          p->isLocal = 0;
        }
        rc = SQLITE_OK;
      }
#అయితే_ముగింపు
      ఆపు;
    }
    స్థితి 'w': {
      /*
      **    weekday N
      **    weekday -N
      **
      ** Move the date forward (for N>=0) or backward (for N<=-0) to the
      ** same time on the next/previous occurrence of weekday abs(N)
      ** where 0==Sunday, 1==Monday, and so forth.  If the date is already
      ** on the appropriate weekday, this is a no-op.
      */
      ఒకవేళ( sqlite3_strnicmp(z, "weekday ", 8)==0
       && sqlite3AtoF(&z[8], &r)>0
       && r>=-6.0 && r<=6.0
       && sqlite3RealSameAsInt(r,(i64)(n=(పూర్ణాంకం)r))
      ){
        sqlite3_int64 Z;
        computeYMD_HMS(p);
        p->tz = 0;
        p->validJD = 0;
        computeJD(p);
        Z = ((p->iJD + 129600000)/86400000) % 7;
        ఒకవేళ( n<0 ) n = -n;
        ఒకవేళ( Z!=n ){
          ఒకవేళ( Z>n ) Z -= 7;
          p->iJD += (n - Z)*86400000;
          ఒకవేళ( strchr(z+8,'-')!=0 ) p->iJD -= 7*86400000;
        }
        clearYMD_HMS_TZ(p);
        rc = 0;
      }
      ఆపు;
    }
    స్థితి 's': {
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
      ఒకవేళ( sqlite3_strnicmp(z, "start of ", 9)!=0 ){
        ఒకవేళ( sqlite3_stricmp(z, "subsec")==0
         || sqlite3_stricmp(z, "subsecond")==0
        ){
          p->useSubsec = 1;
          rc = 0;
        }
        ఆపు;
      }        
      ఒకవేళ( !p->validJD && !p->validYMD && !p->validHMS ) ఆపు;
      z += 9;
      computeYMD(p);
      p->validHMS = 1;
      p->h = p->m = 0;
      p->s = 0.0;
      p->rawS = 0;
      p->tz = 0;
      p->validJD = 0;
      ఒకవేళ( sqlite3_stricmp(z,"month")==0 ){
        p->D = 1;
        rc = 0;
      }లేకపోతే ఒకవేళ( sqlite3_stricmp(z,"year")==0 ){
        p->M = 1;
        p->D = 1;
        rc = 0;
      }లేకపోతే ఒకవేళ( sqlite3_stricmp(z,"day")==0 ){
        rc = 0;
      }
      ఆపు;
    }
    స్థితి '+':
    స్థితి '-':
    స్థితి '0':
    స్థితి '1':
    స్థితి '2':
    స్థితి '3':
    స్థితి '4':
    స్థితి '5':
    స్థితి '6':
    స్థితి '7':
    స్థితి '8':
    స్థితి '9': {
      ద్విగుణ rRounder;
      పూర్ణాంకం i, rx;
      పూర్ణాంకం Y,M,D,h,m,x;
      స్థిరరాశి అక్షరం *z2 = z;
      అక్షరం *zCopy;
      sqlite3 *db = sqlite3_context_db_handle(pCtx);
      అక్షరం z0 = z[0];
      కొరకు(n=1; z[n]; n++){
        ఒకవేళ( z[n]==':' ) ఆపు;
        ఒకవేళ( sqlite3Isspace(z[n]) ) ఆపు;
        ఒకవేళ( z[n]=='-' ){
          ఒకవేళ( n==5 && getDigits(&z[1], "40f", &Y)==1 ) ఆపు;
          ఒకవేళ( n==6 && getDigits(&z[1], "50f", &Y)==1 ) ఆపు;
        }
      }
      zCopy = sqlite3DbStrNDup(db, z, n);
      ఒకవేళ( zCopy==0 ) ఆపు;
      rx = sqlite3AtoF(zCopy, &r)<=0;
      sqlite3DbFree(db, zCopy);
      ఒకవేళ( rx ){
        ప్రతిజ్ఞా( rc==1 );
        ఆపు;
      }
      ఒకవేళ( z[n]=='-' ){
        /* A modifier of the form (+|-)YYYY-MM-DD adds or subtracts the
        ** specified number of years, months, and days.  MM is limited to
        ** the range 0-11 and DD is limited to 0-30.
        */
        ఒకవేళ( z0!='+' && z0!='-' ) ఆపు;  /* Must start with +/- */
        ఒకవేళ( n==5 ){
          ఒకవేళ( getDigits(&z[1], "40f-20a-20d", &Y, &M, &D)!=3 ) ఆపు;
        }లేకపోతే{
          ప్రతిజ్ఞా( n==6 );
          ఒకవేళ( getDigits(&z[1], "50f-20a-20d", &Y, &M, &D)!=3 ) ఆపు;
          z++;
        }
        ఒకవేళ( M>=12 ) ఆపు;                   /* M range 0..11 */
        ఒకవేళ( D>=31 ) ఆపు;                   /* D range 0..30 */
        computeYMD_HMS(p);
        p->validJD = 0;
        ఒకవేళ( z0=='-' ){
          p->Y -= Y;
          p->M -= M;
          D = -D;
        }లేకపోతే{
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
        ఒకవేళ( z[11]==0 ){
          rc = 0;
          ఆపు;
        }
        ఒకవేళ( sqlite3Isspace(z[11])
         && getDigits(&z[12], "20c:20e", &h, &m)==2
        ){
          z2 = &z[12];
          n = 2;
        }లేకపోతే{
          ఆపు;
        }
      }
      ఒకవేళ( z2[n]==':' ){
        /* A modifier of the form (+|-)HH:MM:SS.FFF adds (or subtracts) the
        ** specified number of hours, minutes, seconds, and fractional seconds
        ** to the time.  The ".FFF" may be omitted.  The ":SS.FFF" may be
        ** omitted.
        */

        Date_సమయం tx;
        sqlite3_int64 day;
        ఒకవేళ( !sqlite3Isdigit(*z2) ) z2++;
        memset(&tx, 0, పరిమాణం(tx));
        ఒకవేళ( parseHhMmSs(z2, &tx) ) ఆపు;
        computeJD(&tx);
        tx.iJD -= 43200000;
        day = tx.iJD/86400000;
        tx.iJD -= day*86400000;
        ఒకవేళ( z0=='-' ) tx.iJD = -tx.iJD;
        computeJD(p);
        clearYMD_HMS_TZ(p);
        p->iJD += tx.iJD;
        rc = 0;
        ఆపు;
      }

      /* If control reaches this point, it means the transformation is
      ** one of the forms like "+NNN days".  */
      z += n;
      అయితే( sqlite3Isspace(*z) ) z++;
      n = sqlite3Strlen30(z);
      ఒకవేళ( n<3 || n>10 ) ఆపు;
      ఒకవేళ( sqlite3_పై_గమ్యం_క్రింది[(u8)z[n-1]]=='s' ) n--;
      computeJD(p);
      ప్రతిజ్ఞా( rc==1 );
      rRounder = r<0 ? -0.5 : +0.5;
      p->nFloor = 0;
      కొరకు(i=0; i<Array_కొలత(aXformType); i++){
        ఒకవేళ( aXformType[i].nName==n
         && sqlite3_strnicmp(aXformType[i].zName, z, n)==0
         && r>-aXformType[i].r_హద్దు && r<aXformType[i].r_హద్దు
        ){
          ఎంపిక( i ){
            స్థితి 4: { /* Special processing to add months */
              ప్రతిజ్ఞా( strcmp(aXformType[4].zName,"month")==0 );
              computeYMD_HMS(p);
              p->M += (పూర్ణాంకం)r;
              x = p->M>0 ? (p->M-1)/12 : (p->M-12)/12;
              p->Y += x;
              p->M -= x*12;
              computeFloor(p);
              p->validJD = 0;
              r -= (పూర్ణాంకం)r;
              ఆపు;
            }
            స్థితి 5: { /* Special processing to add years */
              పూర్ణాంకం y = (పూర్ణాంకం)r;
              ప్రతిజ్ఞా( strcmp(aXformType[5].zName,"year")==0 );
              computeYMD_HMS(p);
              ప్రతిజ్ఞా( p->M>=0 && p->M<=12 );
              p->Y += y;
              computeFloor(p);
              p->validJD = 0;
              r -= (పూర్ణాంకం)r;
              ఆపు;
            }
          }
          computeJD(p);
          p->iJD += (sqlite3_int64)(r*1000.0*aXformType[i].rXform + rRounder);
          rc = 0;
          ఆపు;
        }
      }
      clearYMD_HMS_TZ(p);
      ఆపు;
    }
    అప్రమేయ: {
      ఆపు;
    }
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం isDate(
  sqlite3_context *context, 
  పూర్ణాంకం argc, 
  sqlite3_విలువ **argv, 
  Date_సమయం *p
){
  పూర్ణాంకం i, n;
  స్థిరరాశి గుర్తులేని అక్షరం *z;
  పూర్ణాంకం eType;
  memset(p, 0, పరిమాణం(*p));
  ఒకవేళ( argc==0 ){
    ఒకవేళ( !sqlite3_Not_శుద్ధ_Func(context) ) తిరిగి 1;
    తిరిగి సమితి_Date_సమయం_గమ్యం_Current(context, p);
  }
  ఒకవేళ( (eType = sqlite3_విలువ_type(argv[0]))==SQLITE_FLOAT
                   || eType==SQLITE_INTEGER ){
    సమితి_Raw_Date_సంఖ్య(p, sqlite3_విలువ_double(argv[0]));
  }లేకపోతే{
    z = sqlite3_విలువ_పాఠ్యం(argv[0]);
    ఒకవేళ( !z || parse_Date_Or_సమయం(context, (అక్షరం*)z, p) ){
      తిరిగి 1;
    }
  }
  కొరకు(i=1; i<argc; i++){
    z = sqlite3_విలువ_పాఠ్యం(argv[i]);
    n = sqlite3_విలువ_బైట్లు(argv[i]);
    ఒకవేళ( z==0 || parseModifier(context, (అక్షరం*)z, n, p, i) ) తిరిగి 1;
  }
  computeJD(p);
  ఒకవేళ( p->isError || !validJulianDay(p->iJD) ) తిరిగి 1;
  ఒకవేళ( argc==1 && p->validYMD && p->D>28 ){
    /* Make sure a YYYY-MM-DD is normalized.
    ** Example: 2023-02-31 -> 2023-03-03 */
    ప్రతిజ్ఞా( p->validJD );
    p->validYMD = 0;  
  }
  తిరిగి 0;
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
స్థిర శూన్య juliandayFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  Date_సమయం x;
  ఒకవేళ( isDate(context, argc, argv, &x)==0 ){
    computeJD(&x);
    sqlite3_ఫలితం_double(context, x.iJD/86400000.0);
  }
}

/*
**    unixepoch( TIMESTRING, MOD, MOD, ...)
**
** Return the number of seconds (including fractional seconds) since
** the unix epoch of 1970-01-01 00:00:00 GMT.
*/
స్థిర శూన్య unixepochFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  Date_సమయం x;
  ఒకవేళ( isDate(context, argc, argv, &x)==0 ){
    computeJD(&x);
    ఒకవేళ( x.useSubsec ){
      sqlite3_ఫలితం_double(context, (x.iJD - 21086676*(i64)10000000)/1000.0);
    }లేకపోతే{
      sqlite3_ఫలితం_int64(context, x.iJD/1000 - 21086676*(i64)10000);
    }
  }
}

/*
**    datetime( TIMESTRING, MOD, MOD, ...)
**
** Return YYYY-MM-DD HH:MM:SS
*/
స్థిర శూన్య datetimeFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  Date_సమయం x;
  ఒకవేళ( isDate(context, argc, argv, &x)==0 ){
    పూర్ణాంకం Y, s, n;
    అక్షరం zBuf[32];
    computeYMD_HMS(&x);
    Y = x.Y;
    ఒకవేళ( Y<0 ) Y = -Y;
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
    ఒకవేళ( x.useSubsec ){
      s = (పూర్ణాంకం)(1000.0*x.s + 0.5);
      zBuf[18] = '0' + (s/10000)%10;
      zBuf[19] = '0' + (s/1000)%10;
      zBuf[20] = '.';
      zBuf[21] = '0' + (s/100)%10;
      zBuf[22] = '0' + (s/10)%10;
      zBuf[23] = '0' + (s)%10;
      zBuf[24] = 0;
      n = 24;
    }లేకపోతే{
      s = (పూర్ణాంకం)x.s;
      zBuf[18] = '0' + (s/10)%10;
      zBuf[19] = '0' + (s)%10;
      zBuf[20] = 0;
      n = 20;
    }
    ఒకవేళ( x.Y<0 ){
      zBuf[0] = '-';
      sqlite3_ఫలితం_పాఠ్యం(context, zBuf, n, SQLITE_TRANSIENT);
    }లేకపోతే{
      sqlite3_ఫలితం_పాఠ్యం(context, &zBuf[1], n-1, SQLITE_TRANSIENT);
    }
  }
}

/*
**    time( TIMESTRING, MOD, MOD, ...)
**
** Return HH:MM:SS
*/
స్థిర శూన్య సమయం_Func(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  Date_సమయం x;
  ఒకవేళ( isDate(context, argc, argv, &x)==0 ){
    పూర్ణాంకం s, n;
    అక్షరం zBuf[16];
    computeHMS(&x);
    zBuf[0] = '0' + (x.h/10)%10;
    zBuf[1] = '0' + (x.h)%10;
    zBuf[2] = ':';
    zBuf[3] = '0' + (x.m/10)%10;
    zBuf[4] = '0' + (x.m)%10;
    zBuf[5] = ':';
    ఒకవేళ( x.useSubsec ){
      s = (పూర్ణాంకం)(1000.0*x.s + 0.5);
      zBuf[6] = '0' + (s/10000)%10;
      zBuf[7] = '0' + (s/1000)%10;
      zBuf[8] = '.';
      zBuf[9] = '0' + (s/100)%10;
      zBuf[10] = '0' + (s/10)%10;
      zBuf[11] = '0' + (s)%10;
      zBuf[12] = 0;
      n = 12;
    }లేకపోతే{
      s = (పూర్ణాంకం)x.s;
      zBuf[6] = '0' + (s/10)%10;
      zBuf[7] = '0' + (s)%10;
      zBuf[8] = 0;
      n = 8;
    }
    sqlite3_ఫలితం_పాఠ్యం(context, zBuf, n, SQLITE_TRANSIENT);
  }
}

/*
**    date( TIMESTRING, MOD, MOD, ...)
**
** Return YYYY-MM-DD
*/
స్థిర శూన్య dateFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  Date_సమయం x;
  ఒకవేళ( isDate(context, argc, argv, &x)==0 ){
    పూర్ణాంకం Y;
    అక్షరం zBuf[16];
    computeYMD(&x);
    Y = x.Y;
    ఒకవేళ( Y<0 ) Y = -Y;
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
    ఒకవేళ( x.Y<0 ){
      zBuf[0] = '-';
      sqlite3_ఫలితం_పాఠ్యం(context, zBuf, 11, SQLITE_TRANSIENT);
    }లేకపోతే{
      sqlite3_ఫలితం_పాఠ్యం(context, &zBuf[1], 10, SQLITE_TRANSIENT);
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
స్థిర పూర్ణాంకం daysAfterJan01(Date_సమయం *pDate){
  Date_సమయం jan01 = *pDate;
  ప్రతిజ్ఞా( jan01.validYMD );
  ప్రతిజ్ఞా( jan01.validHMS );
  ప్రతిజ్ఞా( pDate->validJD );
  jan01.validJD = 0;
  jan01.M = 1;
  jan01.D = 1;
  computeJD(&jan01);
  తిరిగి (పూర్ణాంకం)((pDate->iJD-jan01.iJD+43200000)/86400000);
}

/*
** Return the number of days after the most recent Monday.
**
** In other words, return the day of the week according
** to this code:
**
**   0=Monday, 1=Tuesday, 2=Wednesday, ..., 6=Sunday.
*/
స్థిర పూర్ణాంకం daysAfterMonday(Date_సమయం *pDate){
  ప్రతిజ్ఞా( pDate->validJD );
  తిరిగి (పూర్ణాంకం)((pDate->iJD+43200000)/86400000) % 7;
}

/*
** Return the number of days after the most recent Sunday.
**
** In other words, return the day of the week according
** to this code:
**
**   0=Sunday, 1=Monday, 2=Tuesday, ..., 6=Saturday
*/
స్థిర పూర్ణాంకం daysAfterSunday(Date_సమయం *pDate){
  ప్రతిజ్ఞా( pDate->validJD );
  తిరిగి (పూర్ణాంకం)((pDate->iJD+129600000)/86400000) % 7;
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
స్థిర శూన్య strftimeFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  Date_సమయం x;
  size_t i,j;
  sqlite3 *db;
  స్థిరరాశి అక్షరం *zFmt;
  sqlite3_str *pRes;


  ఒకవేళ( argc==0 ) తిరిగి;
  zFmt = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  ఒకవేళ( zFmt==0 || isDate(context, argc-1, argv+1, &x) ) తిరిగి;
  db = sqlite3_context_db_handle(context);
  pRes = sqlite3_str_new(db);

  computeJD(&x);
  computeYMD_HMS(&x);
  కొరకు(i=j=0; zFmt[i]; i++){
    అక్షరం cf;
    ఒకవేళ( zFmt[i]!='%' ) కొనసాగు;
    ఒకవేళ( j<i ) sqlite3_str_append(pRes, zFmt+j, (పూర్ణాంకం)(i-j));
    i++;
    j = i + 1;
    cf = zFmt[i];
    ఎంపిక( cf ){
      స్థితి 'd':  /* Fall thru */
      స్థితి 'e': {
        sqlite3_str_appendf(pRes, cf=='d' ? "%02d" : "%2d", x.D);
        ఆపు;
      }
      స్థితి 'f': {  /* Fractional seconds.  (Non-standard) */
        ద్విగుణ s = x.s;
        ఒకవేళ( NEVER(s>59.999) ) s = 59.999;
        sqlite3_str_appendf(pRes, "%06.3f", s);
        ఆపు;
      }
      స్థితి 'F': {
        sqlite3_str_appendf(pRes, "%04d-%02d-%02d", x.Y, x.M, x.D);
        ఆపు;
      }
      స్థితి 'G': /* Fall thru */
      స్థితి 'g': {
        Date_సమయం y = x;
        ప్రతిజ్ఞా( y.validJD );
        /* Move y so that it is the Thursday in the same week as x */
        y.iJD += (3 - daysAfterMonday(&x))*86400000;
        y.validYMD = 0;
        computeYMD(&y);
        ఒకవేళ( cf=='g' ){
          sqlite3_str_appendf(pRes, "%02d", y.Y%100);
        }లేకపోతే{
          sqlite3_str_appendf(pRes, "%04d", y.Y);
        }
        ఆపు;
      }
      స్థితి 'H':
      స్థితి 'k': {
        sqlite3_str_appendf(pRes, cf=='H' ? "%02d" : "%2d", x.h);
        ఆపు;
      }
      స్థితి 'I': /* Fall thru */
      స్థితి 'l': {
        పూర్ణాంకం h = x.h;
        ఒకవేళ( h>12 ) h -= 12;
        ఒకవేళ( h==0 ) h = 12;
        sqlite3_str_appendf(pRes, cf=='I' ? "%02d" : "%2d", h);
        ఆపు;
      }
      స్థితి 'j': {  /* Day of year.  Jan01==1, Jan02==2, and so forth */
        sqlite3_str_appendf(pRes,"%03d",daysAfterJan01(&x)+1);
        ఆపు;
      }
      స్థితి 'J': {  /* Julian day number.  (Non-standard) */
        sqlite3_str_appendf(pRes,"%.16g",x.iJD/86400000.0);
        ఆపు;
      }
      స్థితి 'm': {
        sqlite3_str_appendf(pRes,"%02d",x.M);
        ఆపు;
      }
      స్థితి 'M': {
        sqlite3_str_appendf(pRes,"%02d",x.m);
        ఆపు;
      }
      స్థితి 'p': /* Fall thru */
      స్థితి 'P': {
        ఒకవేళ( x.h>=12 ){
          sqlite3_str_append(pRes, cf=='p' ? "PM" : "pm", 2);
        }లేకపోతే{
          sqlite3_str_append(pRes, cf=='p' ? "AM" : "am", 2);
        }
        ఆపు;
      }
      స్థితి 'R': {
        sqlite3_str_appendf(pRes, "%02d:%02d", x.h, x.m);
        ఆపు;
      }
      స్థితి 's': {
        ఒకవేళ( x.useSubsec ){
          sqlite3_str_appendf(pRes,"%.3f",
                (x.iJD - 21086676*(i64)10000000)/1000.0);
        }లేకపోతే{
          i64 iS = (i64)(x.iJD/1000 - 21086676*(i64)10000);
          sqlite3_str_appendf(pRes,"%lld",iS);
        }
        ఆపు;
      }
      స్థితి 'S': {
        sqlite3_str_appendf(pRes,"%02d",(పూర్ణాంకం)x.s);
        ఆపు;
      }
      స్థితి 'T': {
        sqlite3_str_appendf(pRes,"%02d:%02d:%02d", x.h, x.m, (పూర్ణాంకం)x.s);
        ఆపు;
      }
      స్థితి 'u':    /* Day of week.  1 to 7.  Monday==1, Sunday==7 */
      స్థితి 'w': {  /* Day of week.  0 to 6.  Sunday==0, Monday==1 */
        అక్షరం c = (అక్షరం)daysAfterSunday(&x) + '0';
        ఒకవేళ( c=='0' && cf=='u' ) c = '7';
        sqlite3_str_appendchar(pRes, 1, c);
        ఆపు;
      }
      స్థితి 'U': {  /* Week num. 00-53. First Sun of the year is week 01 */
        sqlite3_str_appendf(pRes,"%02d",
              (daysAfterJan01(&x)-daysAfterSunday(&x)+7)/7);
        ఆపు;
      }
      స్థితి 'V': {  /* Week num. 01-53. First week with a Thur is week 01 */
        Date_సమయం y = x;
        /* Adjust y so that is the Thursday in the same week as x */
        ప్రతిజ్ఞా( y.validJD );
        y.iJD += (3 - daysAfterMonday(&x))*86400000;
        y.validYMD = 0;
        computeYMD(&y);
        sqlite3_str_appendf(pRes,"%02d", daysAfterJan01(&y)/7+1);
        ఆపు;
      }
      స్థితి 'W': {  /* Week num. 00-53. First Mon of the year is week 01 */
        sqlite3_str_appendf(pRes,"%02d",
           (daysAfterJan01(&x)-daysAfterMonday(&x)+7)/7);
        ఆపు;
      }
      స్థితి 'Y': {
        sqlite3_str_appendf(pRes,"%04d",x.Y);
        ఆపు;
      }
      స్థితి '%': {
        sqlite3_str_appendchar(pRes, 1, '%');
        ఆపు;
      }
      అప్రమేయ: {
        sqlite3_str_free(pRes);
        తిరిగి;
      }
    }
  }
  ఒకవేళ( j<i ) sqlite3_str_append(pRes, zFmt+j, (పూర్ణాంకం)(i-j));
  sqlite3_ఫలితం_str(context, pRes, SQLITE_ముగింపు);
}

/*
** current_time()
**
** This function returns the same value as time('now').
*/
స్థిర శూన్య ctimeFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  సమయం_Func(context, 0, 0);
}

/*
** current_date()
**
** This function returns the same value as date('now').
*/
స్థిర శూన్య cdateFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
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
స్థిర శూన్య timediffFunc(
  sqlite3_context *context,
  పూర్ణాంకం NotUsed1,
  sqlite3_విలువ **argv
){
  అక్షరం sign;
  పూర్ణాంకం Y, M;
  Date_సమయం d1, d2;
  sqlite3_str sRes;
  వాడని_PARAMETER(NotUsed1);
  ఒకవేళ( isDate(context, 1, &argv[0], &d1) ) తిరిగి;
  ఒకవేళ( isDate(context, 1, &argv[1], &d2) ) తిరిగి;
  computeYMD_HMS(&d1);
  computeYMD_HMS(&d2);
  ఒకవేళ( d1.iJD>=d2.iJD ){
    sign = '+';
    Y = d1.Y - d2.Y;
    ఒకవేళ( Y ){
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d1.M - d2.M;
    ఒకవేళ( M<0 ){
      Y--;
      M += 12;
    }
    ఒకవేళ( M!=0 ){
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    అయితే( d1.iJD<d2.iJD ){
      M--;
      ఒకవేళ( M<0 ){
        M = 11;
        Y--;
      }
      d2.M--;
      ఒకవేళ( d2.M<1 ){
        d2.M = 12;
        d2.Y--;
      }
      d2.validJD = 0;
      computeJD(&d2);
    }
    d1.iJD -= d2.iJD;
    d1.iJD += (u64)1486995408 * (u64)100000;
  }లేకపోతే /* d1<d2 */{
    sign = '-';
    Y = d2.Y - d1.Y;
    ఒకవేళ( Y ){
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d2.M - d1.M;
    ఒకవేళ( M<0 ){
      Y--;
      M += 12;
    }
    ఒకవేళ( M!=0 ){
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    అయితే( d1.iJD>d2.iJD ){
      M--;
      ఒకవేళ( M<0 ){
        M = 11;
        Y--;
      }
      d2.M++;
      ఒకవేళ( d2.M>12 ){
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
  sqlite3_ఫలితం_str(context, &sRes, SQLITE_XFER);
}


/*
** current_timestamp()
**
** This function returns the same value as datetime('now').
*/
స్థిర శూన్య ctimestampFunc(
  sqlite3_context *context,
  పూర్ణాంకం Not_వాడిన,
  sqlite3_విలువ **NotUsed2
){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  datetimeFunc(context, 0, 0);
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_DATETIME_FUNCS) */

#నిర్వచితమైతే SQLITE_OMIT_DATETIME_FUNCS
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
స్థిర శూన్య current_సమయం_Func(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  సమయం_t t;
  అక్షరం *zFormat = (అక్షరం *)sqlite3_user_దత్తాంశం(context);
  sqlite3_int64 iT;
  నిర్మాణం tm *pTm;
  నిర్మాణం tm sNow;
  అక్షరం zBuf[20];

  వాడని_PARAMETER(argc);
  వాడని_PARAMETER(argv);

  iT = sqlite3_Stmt_Current_సమయం(context);
  ఒకవేళ( iT<=0 ) తిరిగి;
  t = iT/1000 - 10000*(sqlite3_int64)21086676;
#ఒకవేళ HAVE_GMTIME_R
  pTm = gmtime_r(&t, &sNow);
#లేకపోతే
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
  pTm = gmtime(&t);
  ఒకవేళ( pTm ) జ్ఞాపకం_నకలు(&sNow, pTm, పరిమాణం(sNow));
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
#అయితే_ముగింపు
  ఒకవేళ( pTm ){
    strftime(zBuf, 20, zFormat, &sNow);
    sqlite3_ఫలితం_పాఠ్యం(context, zBuf, -1, SQLITE_TRANSIENT);
  }
}
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_DATETIME_FUNCS) && defined(SQLITE_DEBUG)
/*
**   datedebug(...)
**
** This routine returns JSON that describes the internal DateTime object.
** Used for debugging and testing only.  Subject to change.
*/
స్థిర శూన్య datedebugFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  Date_సమయం x;
  ఒకవేళ( isDate(context, argc, argv, &x)==0 ){
    అక్షరం *zJson;
    zJson = sqlite3_mprintf(
      "{iJD:%lld,Y:%d,M:%d,D:%d,h:%d,m:%d,tz:%d,"
      "s:%.3f,validJD:%d,validYMD:%d,validHMS:%d,"
      "nFloor:%d,rawS:%d,isError:%d,useSubsec:%d,"
      "isUtc:%d,isLocal:%d}",
      x.iJD, x.Y, x.M, x.D, x.h, x.m, x.tz,
      x.s, x.validJD, x.validYMD, x.validHMS,
      x.nFloor, x.rawS, x.isError, x.useSubsec,
      x.isUtc, x.isLocal);
    sqlite3_ఫలితం_పాఠ్యం(context, zJson, -1, sqlite3_free);
  }
}
#అయితే_ముగింపు /* !SQLITE_OMIT_DATETIME_FUNCS && SQLITE_DEBUG */


/*
** This function registered all of the above C functions as SQL
** functions.  This should be the only routine in this file with
** external linkage.
*/
శూన్య sqlite3_రిజిస్టరు_Date_సమయం_Functions(శూన్య){
  స్థిర FuncDef a_Date_సమయం_Funcs[] = {
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DATETIME_FUNCS
    శుద్ధ_DATE(julianday,        -1, 0, 0, juliandayFunc ),
    శుద్ధ_DATE(unixepoch,        -1, 0, 0, unixepochFunc ),
    శుద్ధ_DATE(date,             -1, 0, 0, dateFunc      ),
    శుద్ధ_DATE(సమయం,             -1, 0, 0, సమయం_Func      ),
    శుద్ధ_DATE(datetime,         -1, 0, 0, datetimeFunc  ),
    శుద్ధ_DATE(strftime,         -1, 0, 0, strftimeFunc  ),
    శుద్ధ_DATE(timediff,          2, 0, 0, timediffFunc  ),
#నిర్వచితమైతే SQLITE_DEBUG
    శుద్ధ_DATE(datedebug,        -1, 0, 0, datedebugFunc ),
#అయితే_ముగింపు
    DFUNCTION(current_సమయం,      0, 0, 0, ctimeFunc     ),
    DFUNCTION(current_timestamp, 0, 0, 0, ctimestampFunc),
    DFUNCTION(current_date,      0, 0, 0, cdateFunc     ),
#లేకపోతే
    STR_FUNCTION(current_సమయం,      0, "%H:%M:%S",          0, current_సమయం_Func),
    STR_FUNCTION(current_date,      0, "%Y-%m-%d",          0, current_సమయం_Func),
    STR_FUNCTION(current_timestamp, 0, "%Y-%m-%d %H:%M:%S", 0, current_సమయం_Func),
#అయితే_ముగింపు
  };
  sqlite3_చొప్పించు_Builtin_Funcs(a_Date_సమయం_Funcs, Array_కొలత(a_Date_సమయం_Funcs));
}
