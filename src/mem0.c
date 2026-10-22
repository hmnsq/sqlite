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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ZERO_MALLOC

/*
** No-op versions of all memory allocation routines
*/
ಸ್ಥಿರ ಶೂನ್ಯ *sqlite3MemMalloc(ಪೂರ್ಣಾಂಕ nByte){ ಹಿಂತಿರುಗಿ 0; }
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MemFree(ಶೂನ್ಯ *p_ಪೂರ್ವ){ ಹಿಂತಿರುಗಿ; }
ಸ್ಥಿರ ಶೂನ್ಯ *sqlite3MemRealloc(ಶೂನ್ಯ *p_ಪೂರ್ವ, ಪೂರ್ಣಾಂಕ nByte){ ಹಿಂತಿರುಗಿ 0; }
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_Mem_ಗಾತ್ರ(ಶೂನ್ಯ *p_ಪೂರ್ವ){ ಹಿಂತಿರುಗಿ 0; }
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3MemRoundup(ಪೂರ್ಣಾಂಕ n){ ಹಿಂತಿರುಗಿ n; }
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3MemInit(ಶೂನ್ಯ *Not_ಬಳಸಿದ){ ಹಿಂತಿರುಗಿ SQLITE_OK; }
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MemShutdown(ಶೂನ್ಯ *Not_ಬಳಸಿದ){ ಹಿಂತಿರುಗಿ; }

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ಶೂನ್ಯ sqlite3_Mem_ಗಣ_Default(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_ಗಾತ್ರ,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ZERO_MALLOC */
