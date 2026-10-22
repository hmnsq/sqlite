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
#అన్తర్భావ "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#నిర్వచితమైతే SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
స్థిర శూన్య *sqlite3MemMalloc(పూర్ణాంకం nByte){ తిరిగి 0; }
స్థిర శూన్య sqlite3MemFree(శూన్య *p_పూర్వ){ తిరిగి; }
స్థిర శూన్య *sqlite3MemRealloc(శూన్య *p_పూర్వ, పూర్ణాంకం nByte){ తిరిగి 0; }
స్థిర పూర్ణాంకం sqlite3_Mem_కొలత(శూన్య *p_పూర్వ){ తిరిగి 0; }
స్థిర పూర్ణాంకం sqlite3MemRoundup(పూర్ణాంకం n){ తిరిగి n; }
స్థిర పూర్ణాంకం sqlite3MemInit(శూన్య *Not_వాడిన){ తిరిగి SQLITE_OK; }
స్థిర శూన్య sqlite3MemShutdown(శూన్య *Not_వాడిన){ తిరిగి; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
శూన్య sqlite3_Mem_సమితి_Default(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_కొలత,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#అయితే_ముగింపు /* SQLITE_ZERO_MALLOC */
