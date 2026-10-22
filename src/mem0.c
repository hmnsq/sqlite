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
#समावेश "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#अगर_घोषित SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
अटल व्योम *sqlite3MemMalloc(पूर्णांक nByte){ वापस 0; }
अटल व्योम sqlite3MemFree(व्योम *pPrior){ वापस; }
अटल व्योम *sqlite3MemRealloc(व्योम *pPrior, पूर्णांक nByte){ वापस 0; }
अटल पूर्णांक sqlite3_Mem_आकार(व्योम *pPrior){ वापस 0; }
अटल पूर्णांक sqlite3MemRoundup(पूर्णांक n){ वापस n; }
अटल पूर्णांक sqlite3MemInit(व्योम *NotUsed){ वापस SQLITE_OK; }
अटल व्योम sqlite3MemShutdown(व्योम *NotUsed){ वापस; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
व्योम sqlite3MemSetDefault(व्योम){
  अटल स्थिर sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_आकार,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#पूर्ण_अगर /* SQLITE_ZERO_MALLOC */
