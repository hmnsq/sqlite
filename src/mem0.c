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
স্থির শূন্য *sqlite3MemMalloc(পূর্ণাঙ্ক nByte){ প্রত্যাগম 0; }
স্থির শূন্য sqlite3MemFree(শূন্য *pPrior){ প্রত্যাগম; }
স্থির শূন্য *sqlite3MemRealloc(শূন্য *pPrior, পূর্ণাঙ্ক nByte){ প্রত্যাগম 0; }
স্থির পূর্ণাঙ্ক sqlite3MemSize(শূন্য *pPrior){ প্রত্যাগম 0; }
স্থির পূর্ণাঙ্ক sqlite3MemRoundup(পূর্ণাঙ্ক n){ প্রত্যাগম n; }
স্থির পূর্ণাঙ্ক sqlite3MemInit(শূন্য *NotUsed){ প্রত্যাগম SQLITE_OK; }
স্থির শূন্য sqlite3MemShutdown(শূন্য *NotUsed){ প্রত্যাগম; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
শূন্য sqlite3MemSetDefault(শূন্য){
  স্থির ধ্রুব sqlite3_mem_methods defaultMethods = {
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
