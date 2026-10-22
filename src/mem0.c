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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
ਸਥਿਰ ਖਾਲੀ *sqlite3MemMalloc(ਪੂਰਨ nByte){ ਵਾਪਸ 0; }
ਸਥਿਰ ਖਾਲੀ sqlite3MemFree(ਖਾਲੀ *p_ਪੂਰਵ){ ਵਾਪਸ; }
ਸਥਿਰ ਖਾਲੀ *sqlite3MemRealloc(ਖਾਲੀ *p_ਪੂਰਵ, ਪੂਰਨ nByte){ ਵਾਪਸ 0; }
ਸਥਿਰ ਪੂਰਨ sqlite3_Mem_ਮਾਪ(ਖਾਲੀ *p_ਪੂਰਵ){ ਵਾਪਸ 0; }
ਸਥਿਰ ਪੂਰਨ sqlite3MemRoundup(ਪੂਰਨ n){ ਵਾਪਸ n; }
ਸਥਿਰ ਪੂਰਨ sqlite3MemInit(ਖਾਲੀ *Not_ਵਰਤਿਆ){ ਵਾਪਸ SQLITE_OK; }
ਸਥਿਰ ਖਾਲੀ sqlite3MemShutdown(ਖਾਲੀ *Not_ਵਰਤਿਆ){ ਵਾਪਸ; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ਖਾਲੀ sqlite3_Mem_ਸੈੱਟ_Default(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_ਮਾਪ,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#ਜੇ_ਅੰਤ /* SQLITE_ZERO_MALLOC */
