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
#অন্তর্ভাব "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ifdef SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
স্থির শূন্য *sqlite3MemMalloc(পূর্ণসংখ্যা nByte){ ফেরত 0; }
স্থির শূন্য sqlite3MemFree(শূন্য *pPrior){ ফেরত; }
স্থির শূন্য *sqlite3MemRealloc(শূন্য *pPrior, পূর্ণসংখ্যা nByte){ ফেরত 0; }
স্থির পূর্ণসংখ্যা sqlite3_Mem_আকার(শূন্য *pPrior){ ফেরত 0; }
স্থির পূর্ণসংখ্যা sqlite3MemRoundup(পূর্ণসংখ্যা n){ ফেরত n; }
স্থির পূর্ণসংখ্যা sqlite3MemInit(শূন্য *NotUsed){ ফেরত SQLITE_OK; }
স্থির শূন্য sqlite3MemShutdown(শূন্য *NotUsed){ ফেরত; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
শূন্য sqlite3MemSetDefault(শূন্য){
  স্থির ধ্রুবক sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_আকার,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#endif /* SQLITE_ZERO_MALLOC */
