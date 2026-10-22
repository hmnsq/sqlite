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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ifdef SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
ସ୍ଥିର ଶୂନ୍ୟ *sqlite3MemMalloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){ ଫେରନ୍ତୁ 0; }
ସ୍ଥିର ଶୂନ୍ୟ sqlite3MemFree(ଶୂନ୍ୟ *pPrior){ ଫେରନ୍ତୁ; }
ସ୍ଥିର ଶୂନ୍ୟ *sqlite3MemRealloc(ଶୂନ୍ୟ *pPrior, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){ ଫେରନ୍ତୁ 0; }
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3MemSize(ଶୂନ୍ୟ *pPrior){ ଫେରନ୍ତୁ 0; }
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3MemRoundup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){ ଫେରନ୍ତୁ n; }
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3MemInit(ଶୂନ୍ୟ *NotUsed){ ଫେରନ୍ତୁ SQLITE_OK; }
ସ୍ଥିର ଶୂନ୍ୟ sqlite3MemShutdown(ଶୂନ୍ୟ *NotUsed){ ଫେରନ୍ତୁ; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ଶୂନ୍ୟ sqlite3MemSetDefault(ଶୂନ୍ୟ){
  ସ୍ଥିର ଅଚଳ sqlite3_mem_methods defaultMethods = {
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
