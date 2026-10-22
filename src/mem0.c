/*
** 2008 October 28
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains a no-op memory allocation drivers for use when
** SQLITE_ZERO_MALLOC is defined.  The allocation drivers implemented
** here always fail.  SQLite will not operate with these drivers.  These
** are merely placeholders.  Real drivers must be substituted using
** sqlite3_config() before SQLite will operate.
*/
#अन्तर्भाव "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ifdef SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
स्थिर शून्य *sqlite3MemMalloc(पूर्णाङ्क nByte){ प्रत्यागम 0; }
स्थिर शून्य sqlite3MemFree(शून्य *pPrior){ प्रत्यागम; }
स्थिर शून्य *sqlite3MemRealloc(शून्य *pPrior, पूर्णाङ्क nByte){ प्रत्यागम 0; }
स्थिर पूर्णाङ्क sqlite3MemSize(शून्य *pPrior){ प्रत्यागम 0; }
स्थिर पूर्णाङ्क sqlite3MemRoundup(पूर्णाङ्क n){ प्रत्यागम n; }
स्थिर पूर्णाङ्क sqlite3MemInit(शून्य *NotUsed){ प्रत्यागम SQLITE_OK; }
स्थिर शून्य sqlite3MemShutdown(शून्य *NotUsed){ प्रत्यागम; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
शून्य sqlite3MemSetDefault(शून्य){
  स्थिर ध्रुव sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3MemSize,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#endif /* SQLITE_ZERO_MALLOC */
