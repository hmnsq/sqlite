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
#ضمن "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ifdef SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
ساكن فراغ *sqlite3MemMalloc(صحيح nByte){ ܦܢܐ 0; }
ساكن فراغ sqlite3MemFree(فراغ *pPrior){ ܦܢܐ; }
ساكن فراغ *sqlite3MemRealloc(فراغ *pPrior, صحيح nByte){ ܦܢܐ 0; }
ساكن صحيح sqlite3MemSize(فراغ *pPrior){ ܦܢܐ 0; }
ساكن صحيح sqlite3MemRoundup(صحيح n){ ܦܢܐ n; }
ساكن صحيح sqlite3MemInit(فراغ *NotUsed){ ܦܢܐ SQLITE_OK; }
ساكن فراغ sqlite3MemShutdown(فراغ *NotUsed){ ܦܢܐ; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
فراغ sqlite3MemSetDefault(فراغ){
  ساكن ثابت sqlite3_mem_methods defaultMethods = {
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
