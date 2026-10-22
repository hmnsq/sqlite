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
#समावेश "sqliteInt.h"
#समावेश <stdlib.h>
#समावेश <assert.h>
#समावेश <time.h>

#अगर_अघोषित SQLITE_OMIT_DATETIME_FUNCS

/*
** The MSVC CRT on Windows CE may not have a localtime() function.
** So declare a substitute.  The substitute function itself is
** defined in "os_win.c".
*/
#अगर !defined(SQLITE_OMIT_LOCALTIME) && defined(_WIN32_WCE) && \
    (!defined(SQLITE_MSVC_LOCALTIME_API) || !SQLITE_MSVC_LOCALTIME_API)
काष्ठा पंचांग *__cdecl स_स्थानीय(स्थिर समय_प्रकार *);
#पूर्ण_अगर

/*
** A structure for holding a single date and time.
*/
प्रकार काष्ठा DateTime DateTime;
काष्ठा DateTime {
  sqlite3_int64 iJD;  /* The julian day number times 86400000 */
  पूर्णांक Y, M, D;        /* Year, month, and day */
  पूर्णांक h, m;           /* Hour and minutes */
  पूर्णांक tz;             /* Timezone offset in minutes */
  द्विगुन s;           /* Seconds */
  अक्षर validJD;       /* True (1) if iJD is valid */
  अक्षर validYMD;      /* True (1) if Y,M,D are valid */
  अक्षर validHMS;      /* True (1) if h,m,s are valid */
  अक्षर nFloor;            /* Days to implement "floor" */
  अचिन्हित rawS      : 1; /* Raw numeric value stored in s */
  अचिन्हित isError   : 1; /* An overflow has occurred */
  अचिन्हित useSubsec : 1; /* Display subsecond precision */
  अचिन्हित isUtc     : 1; /* Time is known to be UTC */
  अचिन्हित isLocal   : 1; /* Time is known to be localtime */
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
अटल पूर्णांक getDigits(स्थिर अक्षर *zDate, स्थिर अक्षर *zFormat, ...){
  /* The aMx[] array translates the 3rd character of each format
  ** spec into a max size:    a   b   c   d   e      f */
  अटल स्थिर u16 aMx[] = { 12, 14, 24, 31, 59, 14712 };
  बहु_सूची ap;
  पूर्णांक cnt = 0;
  अक्षर अग्र_C;
  बहु_शुरू(ap, zFormat);
  करो{
    अक्षर N = zFormat[0] - '0';
    अक्षर min = zFormat[1] - '0';
    पूर्णांक val = 0;
    u16 max;

    निश्चित( zFormat[2]>='a' && zFormat[2]<='f' );
    max = aMx[zFormat[2] - 'a'];
    अग्र_C = zFormat[3];
    val = 0;
    जबतक( N-- ){
      अगर( !sqlite3Isdigit(*zDate) ){
        जाओ end_getDigits;
      }
      val = val*10 + *zDate - '0';
      zDate++;
    }
    अगर( val<(पूर्णांक)min || val>(पूर्णांक)max || (अग्र_C!=0 && अग्र_C!=*zDate) ){
      जाओ end_getDigits;
    }
    *बहु_तर्क(ap,पूर्णांक*) = val;
    zDate++;
    cnt++;
    zFormat += 4;
  }जबतक( अग्र_C );
end_getDigits:
  बहु_पूर्ण(ap);
  वापस cnt;
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
अटल पूर्णांक parseTimezone(स्थिर अक्षर *zDate, DateTime *p){
  पूर्णांक sgn = 0;
  पूर्णांक nHr, nMn;
  पूर्णांक c;
  जबतक( sqlite3Isspace(*zDate) ){ zDate++; }
  p->tz = 0;
  c = *zDate;
  अगर( c=='-' ){
    sgn = -1;
  }अन्यथा अगर( c=='+' ){
    sgn = +1;
  }अन्यथा अगर( c=='Z' || c=='z' ){
    zDate++;
    p->isLocal = 0;
    p->isUtc = 1;
    जाओ zulu_time;
  }अन्यथा{
    वापस c!=0;
  }
  zDate++;
  अगर( getDigits(zDate, "20b:20e", &nHr, &nMn)!=2 ){
    वापस 1;
  }
  zDate += 5;
  p->tz = sgn*(nMn + nHr*60);
  अगर( p->tz==0 ){   /* Forum post 2025-09-17T10:12:14z */
    p->isLocal = 0;
    p->isUtc = 1;
  }
zulu_time:
  जबतक( sqlite3Isspace(*zDate) ){ zDate++; }
  वापस *zDate!=0;
}

/*
** Parse times of the form HH:MM or HH:MM:SS or HH:MM:SS.FFFF.
** The HH, MM, and SS must each be exactly 2 digits.  The
** fractional seconds FFFF can be one or more digits.
**
** Return 1 if there is a parsing error and 0 on success.
*/
अटल पूर्णांक parseHhMmSs(स्थिर अक्षर *zDate, DateTime *p){
  पूर्णांक h, m, s;
  द्विगुन ms = 0.0;
  अगर( getDigits(zDate, "20c:20e", &h, &m)!=2 ){
    वापस 1;
  }
  zDate += 5;
  अगर( *zDate==':' ){
    zDate++;
    अगर( getDigits(zDate, "20e", &s)!=1 ){
      वापस 1;
    }
    zDate += 2;
    अगर( *zDate=='.' && sqlite3Isdigit(zDate[1]) ){
      द्विगुन rScale = 1.0;
      zDate++;
      जबतक( sqlite3Isdigit(*zDate) ){
        ms = ms*10.0 + *zDate - '0';
        rScale *= 10.0;
        zDate++;
      }
      ms /= rScale;
      /* Truncate to avoid problems with sub-milliseconds
      ** rounding. https://sqlite.org/forum/forumpost/766a2c9231 */
      अगर( ms>0.999 ) ms = 0.999;
    }
  }अन्यथा{
    s = 0;
  }
  अगर( h==24 && (m!=0 || s!=0 || ms!=0.0) ) वापस 1;
  p->validJD = 0;
  p->rawS = 0;
  p->validHMS = 1;
  p->h = h;
  p->m = m;
  p->s = s + ms;
  अगर( parseTimezone(zDate, p) ) वापस 1;
  वापस 0;
}

/*
** Put the DateTime object into its error state.
*/
अटल व्योम datetimeError(DateTime *p){
  स_रखो(p, 0, माप(*p));
  p->isError = 1;
}

/*
** Convert from YYYY-MM-DD HH:MM:SS to julian day.  We always assume
** that the YYYY-MM-DD is according to the Gregorian calendar.
**
** Reference:  Meeus page 61
*/
अटल व्योम computeJD(DateTime *p){
  पूर्णांक Y, M, D, A, B, X1, X2;

  अगर( p->validJD ) वापस;
  अगर( p->validYMD ){
    Y = p->Y;
    M = p->M;
    D = p->D;
  }अन्यथा{
    Y = 2000;  /* If no YMD specified, assume 2000-Jan-01 */
    M = 1;
    D = 1;
    अगर( p->validHMS && p->h==24 ){
      p->h = 0;
      D = 2;
    }
  }
  अगर( Y<-4713 || Y>9999 || p->rawS ){
    datetimeError(p);
    वापस;
  }
  अगर( M<=2 ){
    Y--;
    M += 12;
  }
  A = (Y+4800)/100;
  B = 38 - A + (A/4);
  X1 = 36525*(Y+4716)/100;
  X2 = 306001*(M+1)/10000;
  p->iJD = (sqlite3_int64)((X1 + X2 + D + B - 1524.5 ) * 86400000);
  p->validJD = 1;
  अगर( p->validHMS ){
    p->iJD += p->h*3600000 + p->m*60000 + (sqlite3_int64)(p->s*1000 + 0.5);
    अगर( p->tz ){
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
अटल पूर्णांक daysOfMonth(पूर्णांक Y, पूर्णांक M){
  अगर( (1<<M) & 0x15aa ){
    वापस 31;
  }अन्यथा अगर( M!=2 ){
    वापस 30;
  }अन्यथा अगर( Y%4==0 && (Y%100!=0 || Y%400==0) ){
    वापस 29;
  }अन्यथा{
    वापस 28;
  }
}

/*
** Given the YYYY-MM-DD information current in p, determine if there
** is day-of-month overflow and set nFloor to the number of days that
** would need to be subtracted from the date in order to bring the
** date back to the end of the month.
*/
अटल व्योम computeFloor(DateTime *p){
  पूर्णांक mxD;         /* Maximum day number for the current month */
  निश्चित( p->validYMD || p->isError );
  निश्चित( p->D>=0 && p->D<=32 );
  निश्चित( p->M>=0 && p->M<=12 );
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
अटल पूर्णांक parseYyyyMmDd(स्थिर अक्षर *zDate, DateTime *p){
  पूर्णांक Y, M, D, neg;

  अगर( zDate[0]=='-' ){
    zDate++;
    neg = 1;
  }अन्यथा{
    neg = 0;
  }
  अगर( getDigits(zDate, "40f-21a-21d", &Y, &M, &D)!=3 ){
    वापस 1;
  }
  अगर( neg ) Y = -Y;
  zDate += 10;
  जबतक( sqlite3Isspace(*zDate) || 'T'==*(u8*)zDate ){ zDate++; }
  अगर( parseHhMmSs(zDate, p)==0 ){
    अगर( p->h==24 ){
      पूर्णांक mxD = daysOfMonth(Y,M);
      p->h = 0;
      D++;
      अगर( D>mxD ){
        D -= mxD;
        M++;
        अगर( M>12 ){
          M = 1;
          Y++;
        }
      }
    }
  }अन्यथा अगर( *zDate==0 ){
    p->validHMS = 0;
  }अन्यथा{
    वापस 1;
  }
  p->validJD = 0;
  p->validYMD = 1;
  p->Y = Y;
  p->M = M;
  p->D = D;
  computeFloor(p);
  अगर( p->tz ){
    computeJD(p);
  }
  वापस 0;
}


अटल व्योम clearYMD_HMS_TZ(DateTime *p);  /* Forward declaration */

/*
** Set the time to the current time reported by the VFS.
**
** Return the number of errors.
*/
अटल पूर्णांक setDateTimeToCurrent(sqlite3_context *context, DateTime *p){
  p->iJD = sqlite3StmtCurrentTime(context);
  अगर( p->iJD>0 ){
    p->validJD = 1;
    p->isUtc = 1;
    p->isLocal = 0;
    clearYMD_HMS_TZ(p);
    वापस 0;
  }अन्यथा{
    वापस 1;
  }
}

/*
** Input "r" is a numeric quantity which might be a julian day number,
** or the number of seconds since 1970.  If the value if r is within
** range of a julian day number, install it as such and set validJD.
** If the value is a valid unix timestamp, put it in p->s and set p->rawS.
*/
अटल व्योम set_Raw_Date_संख्या(DateTime *p, द्विगुन r){
  p->s = r;
  p->rawS = 1;
  अगर( r>=0.0 && r<5373484.5 ){
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
अटल पूर्णांक parseDateOrTime(
  sqlite3_context *context, 
  स्थिर अक्षर *zDate, 
  DateTime *p
){
  द्विगुन r;
  अगर( parseYyyyMmDd(zDate,p)==0 ){
    वापस 0;
  }अन्यथा अगर( parseHhMmSs(zDate, p)==0 ){
    वापस 0;
  }अन्यथा अगर( sqlite3StrICmp(zDate,"now")==0 && sqlite3NotPureFunc(context) ){
    वापस setDateTimeToCurrent(context, p);
  }अन्यथा अगर( sqlite3AtoF(zDate, &r)>0 ){
    set_Raw_Date_संख्या(p, r);
    वापस 0;
  }अन्यथा अगर( (sqlite3StrICmp(zDate,"subsec")==0
             || sqlite3StrICmp(zDate,"subsecond")==0)
           && sqlite3NotPureFunc(context) ){
    p->useSubsec = 1;
    वापस setDateTimeToCurrent(context, p);
  }
  वापस 1;
}

/* The julian day number for 9999-12-31 23:59:59.999 is 5373484.4999999.
** Multiplying this by 86400000 gives 464269060799999 as the maximum value
** for DateTime.iJD.
**
** But some older compilers (ex: gcc 4.2.1 on older Macs) cannot deal with 
** such a large integer literal, so we have to encode it.
*/
#घोषणा INT_464269060799999  ((((i64)0x1a640)<<32)|0x1072fdff)

/*
** Return TRUE if the given julian day number is within range.
**
** The input is the JulianDay times 86400000.
*/
अटल पूर्णांक validJulianDay(sqlite3_int64 iJD){
  वापस iJD>=0 && iJD<=INT_464269060799999;
}

/*
** Compute the Year, Month, and Day from the julian day number.
*/
अटल व्योम computeYMD(DateTime *p){
  पूर्णांक Z, alpha, A, B, C, D, E, X1;
  अगर( p->validYMD ) वापस;
  अगर( !p->validJD ){
    p->Y = 2000;
    p->M = 1;
    p->D = 1;
  }अन्यथा अगर( !validJulianDay(p->iJD) ){
    datetimeError(p);
    वापस;
  }अन्यथा{
    Z = (पूर्णांक)((p->iJD + 43200000)/86400000);
    alpha = (पूर्णांक)((Z + 32044.75)/36524.25) - 52;
    A = Z + 1 + alpha - ((alpha+100)/4) + 25;
    B = A + 1524;
    C = (पूर्णांक)((B - 122.1)/365.25);
    D = (36525*(C&32767))/100;
    E = (पूर्णांक)((B-D)/30.6001);
    X1 = (पूर्णांक)(30.6001*E);
    p->D = B - D - X1;
    p->M = E<14 ? E-1 : E-13;
    p->Y = p->M>2 ? C - 4716 : C - 4715;
  }
  p->validYMD = 1;
}

/*
** Compute the Hour, Minute, and Seconds from the julian day number.
*/
अटल व्योम computeHMS(DateTime *p){
  पूर्णांक day_ms, day_min; /* milliseconds, minutes into the day */
  अगर( p->validHMS ) वापस;
  computeJD(p);
  day_ms = (पूर्णांक)((p->iJD + 43200000) % 86400000);
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
अटल व्योम computeYMD_HMS(DateTime *p){
  computeYMD(p);
  computeHMS(p);
}

/*
** Clear the YMD and HMS and the TZ
*/
अटल व्योम clearYMD_HMS_TZ(DateTime *p){
  p->validYMD = 0;
  p->validHMS = 0;
  p->tz = 0;
}

#अगर_अघोषित SQLITE_OMIT_LOCALTIME
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
#अगर !HAVE_LOCALTIME_R && !HAVE_LOCALTIME_S \
    && defined(_MSC_VER) && defined(_CRT_INSECURE_DEPRECATE)
#अघोषित  HAVE_LOCALTIME_S
#घोषणा HAVE_LOCALTIME_S 1
#पूर्ण_अगर

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
अटल पूर्णांक osLocaltime(समय_प्रकार *t, काष्ठा पंचांग *pTm){
  पूर्णांक rc;
#अगर !HAVE_LOCALTIME_R && !HAVE_LOCALTIME_S
  काष्ठा पंचांग *pX;
#अगर SQLITE_THREADSAFE>0
  sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#पूर्ण_अगर
  sqlite3_mutex_enter(mutex);
  pX = स_स्थानीय(t);
#अगर_अघोषित SQLITE_UNTESTABLE
  अगर( sqlite3GlobalConfig.bLocaltimeFault ){
    अगर( sqlite3GlobalConfig.xAltLocaltime!=0
     && 0==sqlite3GlobalConfig.xAltLocaltime((स्थिर व्योम*)t,(व्योम*)pTm)
    ){
      pX = pTm;
    }अन्यथा{
      pX = 0;
    }
  }
#पूर्ण_अगर
  अगर( pX ) *pTm = *pX;
#अगर SQLITE_THREADSAFE>0
  sqlite3_mutex_leave(mutex);
#पूर्ण_अगर
  rc = pX==0;
#अन्यथा
#अगर_अघोषित SQLITE_UNTESTABLE
  अगर( sqlite3GlobalConfig.bLocaltimeFault ){
    अगर( sqlite3GlobalConfig.xAltLocaltime!=0 ){
      वापस sqlite3GlobalConfig.xAltLocaltime((स्थिर व्योम*)t,(व्योम*)pTm);
    }अन्यथा{
      वापस 1;
    }
  }
#पूर्ण_अगर
#अगर HAVE_LOCALTIME_R
  rc = localtime_r(t, pTm)==0;
#अन्यथा
  rc = localtime_s(pTm, t);
#पूर्ण_अगर /* HAVE_LOCALTIME_R */
#पूर्ण_अगर /* HAVE_LOCALTIME_R || HAVE_LOCALTIME_S */
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_OMIT_LOCALTIME */


#अगर_अघोषित SQLITE_OMIT_LOCALTIME
/*
** Assuming the input DateTime is UTC, move it to its localtime equivalent.
*/
अटल पूर्णांक toLocaltime(
  DateTime *p,                   /* Date at which to calculate offset */
  sqlite3_context *pCtx          /* Write error here if one occurs */
){
  समय_प्रकार t;
  काष्ठा पंचांग sLocal;
  पूर्णांक iYearDiff;

  /* Initialize the contents of sLocal to avoid a compiler warning. */
  स_रखो(&sLocal, 0, माप(sLocal));

  computeJD(p);
  अगर( p->iJD<2108667600*(i64)100000 /* 1970-01-01 */
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
    t = (समय_प्रकार)(x.iJD/1000 -  21086676*(i64)10000);
  }अन्यथा{
    iYearDiff = 0;
    t = (समय_प्रकार)(p->iJD/1000 -  21086676*(i64)10000);
  }
  अगर( osLocaltime(&t, &sLocal) ){
    sqlite3_परिणाम_error(pCtx, "local time unavailable", -1);
    वापस SQLITE_ERROR;
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
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_OMIT_LOCALTIME */

/*
** The following table defines various date transformations of the form
**
**            'NNN days'
**
** Where NNN is an arbitrary floating-point number and "days" can be one
** of several units of time.
*/
अटल स्थिर काष्ठा {
  u8 nName;           /* Length of the name */
  अक्षर zName[7];      /* Name of the transformation */
  भग्न rLimit;       /* Maximum NNN value for this transform */
  भग्न rXform;       /* Constant used for this transform */
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
अटल व्योम autoAdjustDate(DateTime *p){
  अगर( !p->rawS || p->validJD ){
    p->rawS = 0;
  }अन्यथा अगर( p->s>=-21086676*(i64)10000        /* -4713-11-24 12:00:00 */
         && p->s<=(25340230*(i64)10000)+799   /*  9999-12-31 23:59:59 */
  ){
    द्विगुन r = p->s*1000.0 + 210866760000000.0;
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
अटल पूर्णांक parseModifier(
  sqlite3_context *pCtx,      /* Function context */
  स्थिर अक्षर *z,              /* The text of the modifier */
  पूर्णांक n,                      /* Length of zMod in bytes */
  DateTime *p,                /* The date/time value to be modified */
  पूर्णांक idx                     /* Parameter index of the modifier */
){
  पूर्णांक rc = 1;
  द्विगुन r;
  चयन(sqlite3UpperToLower[(u8)z[0]] ){
    हाल 'a': {
      /*
      **    auto
      **
      ** If rawS is available, then interpret as a julian day number, or
      ** a unix timestamp, depending on its magnitude.
      */
      अगर( sqlite3_stricmp(z, "auto")==0 ){
        अगर( idx>1 ) वापस 1; /* IMP: R-33611-57934 */
        autoAdjustDate(p);
        rc = 0;
      }
      अवरोध;
    }
    हाल 'c': {
      /*
      **    ceiling
      **
      ** Resolve day-of-month overflow by rolling forward into the next
      ** month.  As this is the default action, this modifier is really
      ** a no-op that is only included for symmetry.  See "floor".
      */
      अगर( sqlite3_stricmp(z, "ceiling")==0 ){
        computeJD(p);
        clearYMD_HMS_TZ(p);
        rc = 0;
        p->nFloor = 0;
      }
      अवरोध;
    }
    हाल 'e': {
      /*
      **    end of day
      **    end of month
      **    end of year
      **
      ** Move the date forwards to the last millisecond of the current
      ** day, month or year.
      */
      अगर( sqlite3_strnicmp(z, "end of ", 7)!=0 ){
        अवरोध;
      }        
      अगर( !p->validJD && !p->validYMD && !p->validHMS ) अवरोध;
      z += 7;
      computeYMD(p);
      p->validHMS = 1;
      p->h = 23;
      p->m = 59;
      p->s = 59.999;
      p->rawS = 0;
      p->tz = 0;
      p->validJD = 0;
      अगर( sqlite3_stricmp(z,"month")==0 ){
        p->D = 1;
        p->M++;
        अगर( p->M>12 ){
          p->Y++;
          p->M = 1;
        }
        computeFloor(p);
        computeJD(p);
        p->iJD -= (p->nFloor+1)*86400000;
        clearYMD_HMS_TZ(p);
        rc = 0;
      }अन्यथा अगर( sqlite3_stricmp(z,"year")==0 ){
        p->M = 12;
        p->D = 31;
        rc = 0;
      }अन्यथा अगर( sqlite3_stricmp(z,"day")==0 ){
        rc = 0;
      }
      अवरोध;
    }
    हाल 'f': {
      /*
      **    floor
      **
      ** Resolve day-of-month overflow by rolling back to the end of the
      ** previous month.
      */
      अगर( sqlite3_stricmp(z, "floor")==0 ){
        computeJD(p);
        p->iJD -= p->nFloor*86400000;
        clearYMD_HMS_TZ(p);
        rc = 0;
      }
      अवरोध;
    }
    हाल 'j': {
      /*
      **    julianday
      **
      ** Always interpret the prior number as a julian-day value.  If this
      ** is not the first modifier, or if the prior argument is not a numeric
      ** value in the allowed range of julian day numbers understood by
      ** SQLite (0..5373484.5) then the result will be NULL.
      */
      अगर( sqlite3_stricmp(z, "julianday")==0 ){
        अगर( idx>1 ) वापस 1;  /* IMP: R-31176-64601 */
        अगर( p->validJD && p->rawS ){
          rc = 0;
          p->rawS = 0;
        }
      }
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_LOCALTIME
    हाल 'l': {
      /*    localtime
      **
      ** Assuming the current time value is UTC (a.k.a. GMT), shift it to
      ** show local time.
      */
      अगर( sqlite3_stricmp(z, "localtime")==0 && sqlite3NotPureFunc(pCtx) ){
        rc = p->isLocal ? SQLITE_OK : toLocaltime(p, pCtx);
        p->isUtc = 0;
        p->isLocal = 1;
      }
      अवरोध;
    }
#पूर्ण_अगर
    हाल 'u': {
      /*
      **    unixepoch
      **
      ** Treat the current value of p->s as the number of
      ** seconds since 1970.  Convert to a real julian day number.
      */
      अगर( sqlite3_stricmp(z, "unixepoch")==0 && p->rawS ){
        अगर( idx>1 ) वापस 1;  /* IMP: R-49255-55373 */
        r = p->s*1000.0 + 210866760000000.0;
        अगर( r>=0.0 && r<464269060800000.0 ){
          clearYMD_HMS_TZ(p);
          p->iJD = (sqlite3_int64)(r + 0.5);
          p->validJD = 1;
          p->rawS = 0;
          rc = 0;
        }
      }
#अगर_अघोषित SQLITE_OMIT_LOCALTIME
      अन्यथा अगर( sqlite3_stricmp(z, "utc")==0 && sqlite3NotPureFunc(pCtx) ){
        अगर( p->isUtc==0 ){
          i64 iOrigJD;              /* Original localtime */
          i64 iGuess;               /* Guess at the corresponding utc time */
          पूर्णांक cnt = 0;              /* Safety to prevent infinite loop */
          i64 iErr;                 /* Guess is off by this much */

          computeJD(p);
          iGuess = iOrigJD = p->iJD;
          iErr = 0;
          करो{
            DateTime new;
            स_रखो(&new, 0, माप(new));
            iGuess -= iErr;
            new.iJD = iGuess;
            new.validJD = 1;
            rc = toLocaltime(&new, pCtx);
            अगर( rc ) वापस rc;
            computeJD(&new);
            iErr = new.iJD - iOrigJD;
          }जबतक( iErr && cnt++<3 );
          स_रखो(p, 0, माप(*p));
          p->iJD = iGuess;
          p->validJD = 1;
          p->isUtc = 1;
          p->isLocal = 0;
        }
        rc = SQLITE_OK;
      }
#पूर्ण_अगर
      अवरोध;
    }
    हाल 'w': {
      /*
      **    weekday N
      **    weekday -N
      **
      ** Move the date forward (for N>=0) or backward (for N<=-0) to the
      ** same time on the next/previous occurrence of weekday abs(N)
      ** where 0==Sunday, 1==Monday, and so forth.  If the date is already
      ** on the appropriate weekday, this is a no-op.
      */
      अगर( sqlite3_strnicmp(z, "weekday ", 8)==0
       && sqlite3AtoF(&z[8], &r)>0
       && r>=-6.0 && r<=6.0
       && sqlite3RealSameAsInt(r,(i64)(n=(पूर्णांक)r))
      ){
        sqlite3_int64 Z;
        computeYMD_HMS(p);
        p->tz = 0;
        p->validJD = 0;
        computeJD(p);
        Z = ((p->iJD + 129600000)/86400000) % 7;
        अगर( n<0 ) n = -n;
        अगर( Z!=n ){
          अगर( Z>n ) Z -= 7;
          p->iJD += (n - Z)*86400000;
          अगर( म_अक्षर(z+8,'-')!=0 ) p->iJD -= 7*86400000;
        }
        clearYMD_HMS_TZ(p);
        rc = 0;
      }
      अवरोध;
    }
    हाल 's': {
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
      अगर( sqlite3_strnicmp(z, "start of ", 9)!=0 ){
        अगर( sqlite3_stricmp(z, "subsec")==0
         || sqlite3_stricmp(z, "subsecond")==0
        ){
          p->useSubsec = 1;
          rc = 0;
        }
        अवरोध;
      }        
      अगर( !p->validJD && !p->validYMD && !p->validHMS ) अवरोध;
      z += 9;
      computeYMD(p);
      p->validHMS = 1;
      p->h = p->m = 0;
      p->s = 0.0;
      p->rawS = 0;
      p->tz = 0;
      p->validJD = 0;
      अगर( sqlite3_stricmp(z,"month")==0 ){
        p->D = 1;
        rc = 0;
      }अन्यथा अगर( sqlite3_stricmp(z,"year")==0 ){
        p->M = 1;
        p->D = 1;
        rc = 0;
      }अन्यथा अगर( sqlite3_stricmp(z,"day")==0 ){
        rc = 0;
      }
      अवरोध;
    }
    हाल '+':
    हाल '-':
    हाल '0':
    हाल '1':
    हाल '2':
    हाल '3':
    हाल '4':
    हाल '5':
    हाल '6':
    हाल '7':
    हाल '8':
    हाल '9': {
      द्विगुन rRounder;
      पूर्णांक i, rx;
      पूर्णांक Y,M,D,h,m,x;
      स्थिर अक्षर *z2 = z;
      अक्षर *zCopy;
      sqlite3 *db = sqlite3_context_db_handle(pCtx);
      अक्षर z0 = z[0];
      क्रम(n=1; z[n]; n++){
        अगर( z[n]==':' ) अवरोध;
        अगर( sqlite3Isspace(z[n]) ) अवरोध;
        अगर( z[n]=='-' ){
          अगर( n==5 && getDigits(&z[1], "40f", &Y)==1 ) अवरोध;
          अगर( n==6 && getDigits(&z[1], "50f", &Y)==1 ) अवरोध;
        }
      }
      zCopy = sqlite3DbStrNDup(db, z, n);
      अगर( zCopy==0 ) अवरोध;
      rx = sqlite3AtoF(zCopy, &r)<=0;
      sqlite3DbFree(db, zCopy);
      अगर( rx ){
        निश्चित( rc==1 );
        अवरोध;
      }
      अगर( z[n]=='-' ){
        /* A modifier of the form (+|-)YYYY-MM-DD adds or subtracts the
        ** specified number of years, months, and days.  MM is limited to
        ** the range 0-11 and DD is limited to 0-30.
        */
        अगर( z0!='+' && z0!='-' ) अवरोध;  /* Must start with +/- */
        अगर( n==5 ){
          अगर( getDigits(&z[1], "40f-20a-20d", &Y, &M, &D)!=3 ) अवरोध;
        }अन्यथा{
          निश्चित( n==6 );
          अगर( getDigits(&z[1], "50f-20a-20d", &Y, &M, &D)!=3 ) अवरोध;
          z++;
        }
        अगर( M>=12 ) अवरोध;                   /* M range 0..11 */
        अगर( D>=31 ) अवरोध;                   /* D range 0..30 */
        computeYMD_HMS(p);
        p->validJD = 0;
        अगर( z0=='-' ){
          p->Y -= Y;
          p->M -= M;
          D = -D;
        }अन्यथा{
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
        अगर( z[11]==0 ){
          rc = 0;
          अवरोध;
        }
        अगर( sqlite3Isspace(z[11])
         && getDigits(&z[12], "20c:20e", &h, &m)==2
        ){
          z2 = &z[12];
          n = 2;
        }अन्यथा{
          अवरोध;
        }
      }
      अगर( z2[n]==':' ){
        /* A modifier of the form (+|-)HH:MM:SS.FFF adds (or subtracts) the
        ** specified number of hours, minutes, seconds, and fractional seconds
        ** to the time.  The ".FFF" may be omitted.  The ":SS.FFF" may be
        ** omitted.
        */

        DateTime tx;
        sqlite3_int64 day;
        अगर( !sqlite3Isdigit(*z2) ) z2++;
        स_रखो(&tx, 0, माप(tx));
        अगर( parseHhMmSs(z2, &tx) ) अवरोध;
        computeJD(&tx);
        tx.iJD -= 43200000;
        day = tx.iJD/86400000;
        tx.iJD -= day*86400000;
        अगर( z0=='-' ) tx.iJD = -tx.iJD;
        computeJD(p);
        clearYMD_HMS_TZ(p);
        p->iJD += tx.iJD;
        rc = 0;
        अवरोध;
      }

      /* If control reaches this point, it means the transformation is
      ** one of the forms like "+NNN days".  */
      z += n;
      जबतक( sqlite3Isspace(*z) ) z++;
      n = sqlite3Strlen30(z);
      अगर( n<3 || n>10 ) अवरोध;
      अगर( sqlite3UpperToLower[(u8)z[n-1]]=='s' ) n--;
      computeJD(p);
      निश्चित( rc==1 );
      rRounder = r<0 ? -0.5 : +0.5;
      p->nFloor = 0;
      क्रम(i=0; i<Array_आकार(aXformType); i++){
        अगर( aXformType[i].nName==n
         && sqlite3_strnicmp(aXformType[i].zName, z, n)==0
         && r>-aXformType[i].rLimit && r<aXformType[i].rLimit
        ){
          चयन( i ){
            हाल 4: { /* Special processing to add months */
              निश्चित( म_भेद(aXformType[4].zName,"month")==0 );
              computeYMD_HMS(p);
              p->M += (पूर्णांक)r;
              x = p->M>0 ? (p->M-1)/12 : (p->M-12)/12;
              p->Y += x;
              p->M -= x*12;
              computeFloor(p);
              p->validJD = 0;
              r -= (पूर्णांक)r;
              अवरोध;
            }
            हाल 5: { /* Special processing to add years */
              पूर्णांक y = (पूर्णांक)r;
              निश्चित( म_भेद(aXformType[5].zName,"year")==0 );
              computeYMD_HMS(p);
              निश्चित( p->M>=0 && p->M<=12 );
              p->Y += y;
              computeFloor(p);
              p->validJD = 0;
              r -= (पूर्णांक)r;
              अवरोध;
            }
          }
          computeJD(p);
          p->iJD += (sqlite3_int64)(r*1000.0*aXformType[i].rXform + rRounder);
          rc = 0;
          अवरोध;
        }
      }
      clearYMD_HMS_TZ(p);
      अवरोध;
    }
    शेष: {
      अवरोध;
    }
  }
  वापस rc;
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
अटल पूर्णांक isDate(
  sqlite3_context *context, 
  पूर्णांक argc, 
  sqlite3_मूल्य **argv, 
  DateTime *p
){
  पूर्णांक i, n;
  स्थिर अचिन्हित अक्षर *z;
  पूर्णांक eType;
  स_रखो(p, 0, माप(*p));
  अगर( argc==0 ){
    अगर( !sqlite3NotPureFunc(context) ) वापस 1;
    वापस setDateTimeToCurrent(context, p);
  }
  अगर( (eType = sqlite3_मूल्य_type(argv[0]))==SQLITE_FLOAT
                   || eType==SQLITE_INTEGER ){
    set_Raw_Date_संख्या(p, sqlite3_मूल्य_double(argv[0]));
  }अन्यथा{
    z = sqlite3_मूल्य_text(argv[0]);
    अगर( !z || parseDateOrTime(context, (अक्षर*)z, p) ){
      वापस 1;
    }
  }
  क्रम(i=1; i<argc; i++){
    z = sqlite3_मूल्य_text(argv[i]);
    n = sqlite3_मूल्य_bytes(argv[i]);
    अगर( z==0 || parseModifier(context, (अक्षर*)z, n, p, i) ) वापस 1;
  }
  computeJD(p);
  अगर( p->isError || !validJulianDay(p->iJD) ) वापस 1;
  अगर( argc==1 && p->validYMD && p->D>28 ){
    /* Make sure a YYYY-MM-DD is normalized.
    ** Example: 2023-02-31 -> 2023-03-03 */
    निश्चित( p->validJD );
    p->validYMD = 0;  
  }
  वापस 0;
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
अटल व्योम juliandayFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  DateTime x;
  अगर( isDate(context, argc, argv, &x)==0 ){
    computeJD(&x);
    sqlite3_परिणाम_double(context, x.iJD/86400000.0);
  }
}

/*
**    unixepoch( TIMESTRING, MOD, MOD, ...)
**
** Return the number of seconds (including fractional seconds) since
** the unix epoch of 1970-01-01 00:00:00 GMT.
*/
अटल व्योम unixepochFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  DateTime x;
  अगर( isDate(context, argc, argv, &x)==0 ){
    computeJD(&x);
    अगर( x.useSubsec ){
      sqlite3_परिणाम_double(context, (x.iJD - 21086676*(i64)10000000)/1000.0);
    }अन्यथा{
      sqlite3_परिणाम_int64(context, x.iJD/1000 - 21086676*(i64)10000);
    }
  }
}

/*
**    datetime( TIMESTRING, MOD, MOD, ...)
**
** Return YYYY-MM-DD HH:MM:SS
*/
अटल व्योम datetimeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  DateTime x;
  अगर( isDate(context, argc, argv, &x)==0 ){
    पूर्णांक Y, s, n;
    अक्षर zBuf[32];
    computeYMD_HMS(&x);
    Y = x.Y;
    अगर( Y<0 ) Y = -Y;
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
    अगर( x.useSubsec ){
      s = (पूर्णांक)(1000.0*x.s + 0.5);
      zBuf[18] = '0' + (s/10000)%10;
      zBuf[19] = '0' + (s/1000)%10;
      zBuf[20] = '.';
      zBuf[21] = '0' + (s/100)%10;
      zBuf[22] = '0' + (s/10)%10;
      zBuf[23] = '0' + (s)%10;
      zBuf[24] = 0;
      n = 24;
    }अन्यथा{
      s = (पूर्णांक)x.s;
      zBuf[18] = '0' + (s/10)%10;
      zBuf[19] = '0' + (s)%10;
      zBuf[20] = 0;
      n = 20;
    }
    अगर( x.Y<0 ){
      zBuf[0] = '-';
      sqlite3_परिणाम_text(context, zBuf, n, SQLITE_TRANSIENT);
    }अन्यथा{
      sqlite3_परिणाम_text(context, &zBuf[1], n-1, SQLITE_TRANSIENT);
    }
  }
}

/*
**    time( TIMESTRING, MOD, MOD, ...)
**
** Return HH:MM:SS
*/
अटल व्योम timeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  DateTime x;
  अगर( isDate(context, argc, argv, &x)==0 ){
    पूर्णांक s, n;
    अक्षर zBuf[16];
    computeHMS(&x);
    zBuf[0] = '0' + (x.h/10)%10;
    zBuf[1] = '0' + (x.h)%10;
    zBuf[2] = ':';
    zBuf[3] = '0' + (x.m/10)%10;
    zBuf[4] = '0' + (x.m)%10;
    zBuf[5] = ':';
    अगर( x.useSubsec ){
      s = (पूर्णांक)(1000.0*x.s + 0.5);
      zBuf[6] = '0' + (s/10000)%10;
      zBuf[7] = '0' + (s/1000)%10;
      zBuf[8] = '.';
      zBuf[9] = '0' + (s/100)%10;
      zBuf[10] = '0' + (s/10)%10;
      zBuf[11] = '0' + (s)%10;
      zBuf[12] = 0;
      n = 12;
    }अन्यथा{
      s = (पूर्णांक)x.s;
      zBuf[6] = '0' + (s/10)%10;
      zBuf[7] = '0' + (s)%10;
      zBuf[8] = 0;
      n = 8;
    }
    sqlite3_परिणाम_text(context, zBuf, n, SQLITE_TRANSIENT);
  }
}

/*
**    date( TIMESTRING, MOD, MOD, ...)
**
** Return YYYY-MM-DD
*/
अटल व्योम dateFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  DateTime x;
  अगर( isDate(context, argc, argv, &x)==0 ){
    पूर्णांक Y;
    अक्षर zBuf[16];
    computeYMD(&x);
    Y = x.Y;
    अगर( Y<0 ) Y = -Y;
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
    अगर( x.Y<0 ){
      zBuf[0] = '-';
      sqlite3_परिणाम_text(context, zBuf, 11, SQLITE_TRANSIENT);
    }अन्यथा{
      sqlite3_परिणाम_text(context, &zBuf[1], 10, SQLITE_TRANSIENT);
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
अटल पूर्णांक daysAfterJan01(DateTime *pDate){
  DateTime jan01 = *pDate;
  निश्चित( jan01.validYMD );
  निश्चित( jan01.validHMS );
  निश्चित( pDate->validJD );
  jan01.validJD = 0;
  jan01.M = 1;
  jan01.D = 1;
  computeJD(&jan01);
  वापस (पूर्णांक)((pDate->iJD-jan01.iJD+43200000)/86400000);
}

/*
** Return the number of days after the most recent Monday.
**
** In other words, return the day of the week according
** to this code:
**
**   0=Monday, 1=Tuesday, 2=Wednesday, ..., 6=Sunday.
*/
अटल पूर्णांक daysAfterMonday(DateTime *pDate){
  निश्चित( pDate->validJD );
  वापस (पूर्णांक)((pDate->iJD+43200000)/86400000) % 7;
}

/*
** Return the number of days after the most recent Sunday.
**
** In other words, return the day of the week according
** to this code:
**
**   0=Sunday, 1=Monday, 2=Tuesday, ..., 6=Saturday
*/
अटल पूर्णांक daysAfterSunday(DateTime *pDate){
  निश्चित( pDate->validJD );
  वापस (पूर्णांक)((pDate->iJD+129600000)/86400000) % 7;
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
अटल व्योम strftimeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  DateTime x;
  माप_प्रकार i,j;
  sqlite3 *db;
  स्थिर अक्षर *zFmt;
  sqlite3_str *pRes;


  अगर( argc==0 ) वापस;
  zFmt = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  अगर( zFmt==0 || isDate(context, argc-1, argv+1, &x) ) वापस;
  db = sqlite3_context_db_handle(context);
  pRes = sqlite3_str_new(db);

  computeJD(&x);
  computeYMD_HMS(&x);
  क्रम(i=j=0; zFmt[i]; i++){
    अक्षर cf;
    अगर( zFmt[i]!='%' ) जारी;
    अगर( j<i ) sqlite3_str_append(pRes, zFmt+j, (पूर्णांक)(i-j));
    i++;
    j = i + 1;
    cf = zFmt[i];
    चयन( cf ){
      हाल 'd':  /* Fall thru */
      हाल 'e': {
        sqlite3_str_appendf(pRes, cf=='d' ? "%02d" : "%2d", x.D);
        अवरोध;
      }
      हाल 'f': {  /* Fractional seconds.  (Non-standard) */
        द्विगुन s = x.s;
        अगर( NEVER(s>59.999) ) s = 59.999;
        sqlite3_str_appendf(pRes, "%06.3f", s);
        अवरोध;
      }
      हाल 'F': {
        sqlite3_str_appendf(pRes, "%04d-%02d-%02d", x.Y, x.M, x.D);
        अवरोध;
      }
      हाल 'G': /* Fall thru */
      हाल 'g': {
        DateTime y = x;
        निश्चित( y.validJD );
        /* Move y so that it is the Thursday in the same week as x */
        y.iJD += (3 - daysAfterMonday(&x))*86400000;
        y.validYMD = 0;
        computeYMD(&y);
        अगर( cf=='g' ){
          sqlite3_str_appendf(pRes, "%02d", y.Y%100);
        }अन्यथा{
          sqlite3_str_appendf(pRes, "%04d", y.Y);
        }
        अवरोध;
      }
      हाल 'H':
      हाल 'k': {
        sqlite3_str_appendf(pRes, cf=='H' ? "%02d" : "%2d", x.h);
        अवरोध;
      }
      हाल 'I': /* Fall thru */
      हाल 'l': {
        पूर्णांक h = x.h;
        अगर( h>12 ) h -= 12;
        अगर( h==0 ) h = 12;
        sqlite3_str_appendf(pRes, cf=='I' ? "%02d" : "%2d", h);
        अवरोध;
      }
      हाल 'j': {  /* Day of year.  Jan01==1, Jan02==2, and so forth */
        sqlite3_str_appendf(pRes,"%03d",daysAfterJan01(&x)+1);
        अवरोध;
      }
      हाल 'J': {  /* Julian day number.  (Non-standard) */
        sqlite3_str_appendf(pRes,"%.16g",x.iJD/86400000.0);
        अवरोध;
      }
      हाल 'm': {
        sqlite3_str_appendf(pRes,"%02d",x.M);
        अवरोध;
      }
      हाल 'M': {
        sqlite3_str_appendf(pRes,"%02d",x.m);
        अवरोध;
      }
      हाल 'p': /* Fall thru */
      हाल 'P': {
        अगर( x.h>=12 ){
          sqlite3_str_append(pRes, cf=='p' ? "PM" : "pm", 2);
        }अन्यथा{
          sqlite3_str_append(pRes, cf=='p' ? "AM" : "am", 2);
        }
        अवरोध;
      }
      हाल 'R': {
        sqlite3_str_appendf(pRes, "%02d:%02d", x.h, x.m);
        अवरोध;
      }
      हाल 's': {
        अगर( x.useSubsec ){
          sqlite3_str_appendf(pRes,"%.3f",
                (x.iJD - 21086676*(i64)10000000)/1000.0);
        }अन्यथा{
          i64 iS = (i64)(x.iJD/1000 - 21086676*(i64)10000);
          sqlite3_str_appendf(pRes,"%lld",iS);
        }
        अवरोध;
      }
      हाल 'S': {
        sqlite3_str_appendf(pRes,"%02d",(पूर्णांक)x.s);
        अवरोध;
      }
      हाल 'T': {
        sqlite3_str_appendf(pRes,"%02d:%02d:%02d", x.h, x.m, (पूर्णांक)x.s);
        अवरोध;
      }
      हाल 'u':    /* Day of week.  1 to 7.  Monday==1, Sunday==7 */
      हाल 'w': {  /* Day of week.  0 to 6.  Sunday==0, Monday==1 */
        अक्षर c = (अक्षर)daysAfterSunday(&x) + '0';
        अगर( c=='0' && cf=='u' ) c = '7';
        sqlite3_str_appendchar(pRes, 1, c);
        अवरोध;
      }
      हाल 'U': {  /* Week num. 00-53. First Sun of the year is week 01 */
        sqlite3_str_appendf(pRes,"%02d",
              (daysAfterJan01(&x)-daysAfterSunday(&x)+7)/7);
        अवरोध;
      }
      हाल 'V': {  /* Week num. 01-53. First week with a Thur is week 01 */
        DateTime y = x;
        /* Adjust y so that is the Thursday in the same week as x */
        निश्चित( y.validJD );
        y.iJD += (3 - daysAfterMonday(&x))*86400000;
        y.validYMD = 0;
        computeYMD(&y);
        sqlite3_str_appendf(pRes,"%02d", daysAfterJan01(&y)/7+1);
        अवरोध;
      }
      हाल 'W': {  /* Week num. 00-53. First Mon of the year is week 01 */
        sqlite3_str_appendf(pRes,"%02d",
           (daysAfterJan01(&x)-daysAfterMonday(&x)+7)/7);
        अवरोध;
      }
      हाल 'Y': {
        sqlite3_str_appendf(pRes,"%04d",x.Y);
        अवरोध;
      }
      हाल '%': {
        sqlite3_str_appendchar(pRes, 1, '%');
        अवरोध;
      }
      शेष: {
        sqlite3_str_free(pRes);
        वापस;
      }
    }
  }
  अगर( j<i ) sqlite3_str_append(pRes, zFmt+j, (पूर्णांक)(i-j));
  sqlite3_परिणाम_str(context, pRes, SQLITE_FINISH);
}

/*
** current_time()
**
** This function returns the same value as time('now').
*/
अटल व्योम ctimeFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  timeFunc(context, 0, 0);
}

/*
** current_date()
**
** This function returns the same value as date('now').
*/
अटल व्योम cdateFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
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
अटल व्योम timediffFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed1,
  sqlite3_मूल्य **argv
){
  अक्षर sign;
  पूर्णांक Y, M;
  DateTime d1, d2;
  sqlite3_str sRes;
  UNUSED_PARAMETER(NotUsed1);
  अगर( isDate(context, 1, &argv[0], &d1) ) वापस;
  अगर( isDate(context, 1, &argv[1], &d2) ) वापस;
  computeYMD_HMS(&d1);
  computeYMD_HMS(&d2);
  अगर( d1.iJD>=d2.iJD ){
    sign = '+';
    Y = d1.Y - d2.Y;
    अगर( Y ){
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d1.M - d2.M;
    अगर( M<0 ){
      Y--;
      M += 12;
    }
    अगर( M!=0 ){
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    जबतक( d1.iJD<d2.iJD ){
      M--;
      अगर( M<0 ){
        M = 11;
        Y--;
      }
      d2.M--;
      अगर( d2.M<1 ){
        d2.M = 12;
        d2.Y--;
      }
      d2.validJD = 0;
      computeJD(&d2);
    }
    d1.iJD -= d2.iJD;
    d1.iJD += (u64)1486995408 * (u64)100000;
  }अन्यथा /* d1<d2 */{
    sign = '-';
    Y = d2.Y - d1.Y;
    अगर( Y ){
      d2.Y = d1.Y;
      d2.validJD = 0;
      computeJD(&d2);
    }
    M = d2.M - d1.M;
    अगर( M<0 ){
      Y--;
      M += 12;
    }
    अगर( M!=0 ){
      d2.M = d1.M;
      d2.validJD = 0;
      computeJD(&d2);
    }
    जबतक( d1.iJD>d2.iJD ){
      M--;
      अगर( M<0 ){
        M = 11;
        Y--;
      }
      d2.M++;
      अगर( d2.M>12 ){
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
  sqlite3_परिणाम_str(context, &sRes, SQLITE_XFER);
}


/*
** current_timestamp()
**
** This function returns the same value as datetime('now').
*/
अटल व्योम ctimestampFunc(
  sqlite3_context *context,
  पूर्णांक NotUsed,
  sqlite3_मूल्य **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  datetimeFunc(context, 0, 0);
}
#पूर्ण_अगर /* !defined(SQLITE_OMIT_DATETIME_FUNCS) */

#अगर_घोषित SQLITE_OMIT_DATETIME_FUNCS
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
अटल व्योम currentTimeFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  समय_प्रकार t;
  अक्षर *zFormat = (अक्षर *)sqlite3_user_तथ्य(context);
  sqlite3_int64 iT;
  काष्ठा पंचांग *pTm;
  काष्ठा पंचांग sNow;
  अक्षर zBuf[20];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);

  iT = sqlite3StmtCurrentTime(context);
  अगर( iT<=0 ) वापस;
  t = iT/1000 - 10000*(sqlite3_int64)21086676;
#अगर HAVE_GMTIME_R
  pTm = gmtime_r(&t, &sNow);
#अन्यथा
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
  pTm = स_जमट(&t);
  अगर( pTm ) स_नकल(&sNow, pTm, माप(sNow));
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
#पूर्ण_अगर
  अगर( pTm ){
    स_माला(zBuf, 20, zFormat, &sNow);
    sqlite3_परिणाम_text(context, zBuf, -1, SQLITE_TRANSIENT);
  }
}
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_DATETIME_FUNCS) && defined(SQLITE_DEBUG)
/*
**   datedebug(...)
**
** This routine returns JSON that describes the internal DateTime object.
** Used for debugging and testing only.  Subject to change.
*/
अटल व्योम datedebugFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  DateTime x;
  अगर( isDate(context, argc, argv, &x)==0 ){
    अक्षर *zJson;
    zJson = sqlite3_mprintf(
      "{iJD:%lld,Y:%d,M:%d,D:%d,h:%d,m:%d,tz:%d,"
      "s:%.3f,validJD:%d,validYMD:%d,validHMS:%d,"
      "nFloor:%d,rawS:%d,isError:%d,useSubsec:%d,"
      "isUtc:%d,isLocal:%d}",
      x.iJD, x.Y, x.M, x.D, x.h, x.m, x.tz,
      x.s, x.validJD, x.validYMD, x.validHMS,
      x.nFloor, x.rawS, x.isError, x.useSubsec,
      x.isUtc, x.isLocal);
    sqlite3_परिणाम_text(context, zJson, -1, sqlite3_free);
  }
}
#पूर्ण_अगर /* !SQLITE_OMIT_DATETIME_FUNCS && SQLITE_DEBUG */


/*
** This function registered all of the above C functions as SQL
** functions.  This should be the only routine in this file with
** external linkage.
*/
व्योम sqlite3RegisterDateTimeFunctions(व्योम){
  अटल FuncDef aDateTimeFuncs[] = {
#अगर_अघोषित SQLITE_OMIT_DATETIME_FUNCS
    PURE_DATE(julianday,        -1, 0, 0, juliandayFunc ),
    PURE_DATE(unixepoch,        -1, 0, 0, unixepochFunc ),
    PURE_DATE(date,             -1, 0, 0, dateFunc      ),
    PURE_DATE(समय,             -1, 0, 0, timeFunc      ),
    PURE_DATE(datetime,         -1, 0, 0, datetimeFunc  ),
    PURE_DATE(स_माला,         -1, 0, 0, strftimeFunc  ),
    PURE_DATE(timediff,          2, 0, 0, timediffFunc  ),
#अगर_घोषित SQLITE_DEBUG
    PURE_DATE(datedebug,        -1, 0, 0, datedebugFunc ),
#पूर्ण_अगर
    DFUNCTION(current_time,      0, 0, 0, ctimeFunc     ),
    DFUNCTION(current_timestamp, 0, 0, 0, ctimestampFunc),
    DFUNCTION(current_date,      0, 0, 0, cdateFunc     ),
#अन्यथा
    STR_FUNCTION(current_time,      0, "%H:%M:%S",          0, currentTimeFunc),
    STR_FUNCTION(current_date,      0, "%Y-%m-%d",          0, currentTimeFunc),
    STR_FUNCTION(current_timestamp, 0, "%Y-%m-%d %H:%M:%S", 0, currentTimeFunc),
#पूर्ण_अगर
  };
  sqlite3_जोड़ो_Builtin_Funcs(aDateTimeFuncs, Array_आकार(aDateTimeFuncs));
}
