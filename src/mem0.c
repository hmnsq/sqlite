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
#شامل "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#اگر_تعریف_شدہ SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
ساکن خالی *sqlite3MemMalloc(صحیح nByte){ واپس 0; }
ساکن خالی sqlite3MemFree(خالی *p_پیشگی){ واپس; }
ساکن خالی *sqlite3MemRealloc(خالی *p_پیشگی, صحیح nByte){ واپس 0; }
ساکن صحیح sqlite3_Mem_سائز(خالی *p_پیشگی){ واپس 0; }
ساکن صحیح sqlite3MemRoundup(صحیح n){ واپس n; }
ساکن صحیح sqlite3MemInit(خالی *Not_مستعمل){ واپس SQLITE_OK; }
ساکن خالی sqlite3MemShutdown(خالی *Not_مستعمل){ واپس; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
خالی sqlite3_Mem_سیٹ_Default(خالی){
  ساکن ثابت sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_سائز,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#اگر_ختم /* SQLITE_ZERO_MALLOC */
