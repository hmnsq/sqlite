/*
** 2007 May 7
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
** This file contains code used for testing the SQLite system.
** None of the code in this file goes into a deliverable build.
** 
** The focus of this file is providing the TCL testing layer
** access to compile-time constants.
*/

#شامل "sqliteLimit.h"

#شامل "sqliteInt.h"
#اگر SQLITE_OS_WIN
#  include "os_win.h"
#اگر_ختم

#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>

/*
** Macro to stringify the results of the evaluation a pre-processor
** macro. i.e. so that STRINGVALUE(SQLITE_NOMEM) -> "7".
*/
#تعریف_کرو STRINGVALUE2(x) #x
#تعریف_کرو STRINGVALUE(x) STRINGVALUE2(x)

/*
** This routine sets entries in the global ::sqlite_options() array variable
** according to the compile-time configuration of the database.  Test
** procedures use this to determine when tests should be omitted.
*/
ساکن خالی سیٹ_options(Tcl_Interp *interp){
#اگر HAVE_MALLOC_USABLE_سائز
  Tcl_سیٹ_Var2(interp, "sqlite_options", "malloc_usable_size", "1",
              TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "malloc_usable_size", "0",
              TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_32_ثنائی_ہندسہ_ROWID
  Tcl_سیٹ_Var2(interp, "sqlite_options", "rowid32", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "rowid32", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
  Tcl_سیٹ_Var2(
      interp, "sqlite_options", "allow_rowid_in_view", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(
      interp, "sqlite_options", "allow_rowid_in_view", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_ENABLE_CARRAY)
  Tcl_سیٹ_Var2(interp, "sqlite_options","carray","1",TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options","carray","0",TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_CASE_SENSITIVE_LIKE
  Tcl_سیٹ_Var2(interp, "sqlite_options","casesensitivelike","1",TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options","casesensitivelike","0",TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SLOWDOWN_ضربی_عدد
  Tcl_سیٹ_Var2(interp, "sqlite_options","configslower",
              STRINGVALUE(CONFIG_SLOWDOWN_ضربی_عدد),TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options","configslower","1.0",TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر !SQLITE_OS_WINCE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "curdir", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "curdir", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_WIN32_MALLOC
  Tcl_سیٹ_Var2(interp, "sqlite_options", "win32malloc", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "win32malloc", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
  Tcl_سیٹ_Var2(interp, "sqlite_options", "debug", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "debug", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEFAULT_CKPTFULLFSYNC
  Tcl_سیٹ_Var2(interp, "sqlite_options", "default_ckptfullfsync", 
              SQLITE_DEFAULT_CKPTFULLFSYNC ? "1" : "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "default_ckptfullfsync", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_براہ_راست_حد_سے_تجاوز_READ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "direct_read", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "direct_read", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DISABLE_DIRSYNC
  Tcl_سیٹ_Var2(interp, "sqlite_options", "dirsync", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "dirsync", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DISABLE_LFS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "lfs", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "lfs", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DISABLE_PAGECACHE_حد_سے_تجاوز_STATS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "pagecache_overflow_stats","0",TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "pagecache_overflow_stats","1",TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر SQLITE_MAX_MMAP_سائز>0
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mmap", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mmap", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

  Tcl_سیٹ_Var2(interp, "sqlite_options", "worker_threads", 
      STRINGVALUE(SQLITE_MAX_کارکن_دھاگے), TCL_GLOBAL_ONLY
  );

#اگر_تعریف_شدہ SQLITE_MEMDEBUG 
  Tcl_سیٹ_Var2(interp, "sqlite_options", "memdebug", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "memdebug", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_8_3_نام
  Tcl_سیٹ_Var2(interp, "sqlite_options", "8_3_names", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "8_3_names", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_CURSOR_HINTS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "cursorhints", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "cursorhints", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_HIDDEN_COLUMNS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "hiddencolumns", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "hiddencolumns", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DESERIALIZE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "deserialize", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "deserialize", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_MATH_FUNCTIONS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mathlib", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mathlib", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS3
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mem3", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mem3", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS5
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mem5", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mem5", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_انحراف_SQL_FUNC
  Tcl_سیٹ_Var2(interp, "sqlite_options", "offset_sql_func","1",TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "offset_sql_func","0",TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_ORDERED_سیٹ_AGGREGATES
  Tcl_سیٹ_Var2(interp, "sqlite_options",
                      "ordered_set_aggregates","1",TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options",
                      "ordered_set_aggregates","0",TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
  Tcl_سیٹ_Var2(interp, "sqlite_options", "preupdate", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "preupdate", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_SNAPSHOT
  Tcl_سیٹ_Var2(interp, "sqlite_options", "snapshot", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "snapshot", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_MUTEX_OMIT
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mutex", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mutex", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_MUTEX_NOOP
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mutex_noop", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "mutex_noop", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_ALTERTABLE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "altertable", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "altertable", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_ANALYZE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "analyze", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "analyze", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  Tcl_سیٹ_Var2(interp, "sqlite_options", "api_armor", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "api_armor", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_ATOMIC_WRITE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "atomicwrite", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "atomicwrite", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_GEOPOLY
  Tcl_سیٹ_Var2(interp, "sqlite_options", "geopoly", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "geopoly", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_JSON
  Tcl_سیٹ_Var2(interp, "sqlite_options", "json1", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "json1", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

  Tcl_سیٹ_Var2(interp, "sqlite_options", "has_codec", "0", TCL_GLOBAL_ONLY);

#اگر_تعریف_شدہ SQLITE_LIKE_DOESNT_مطابقت_BLOBS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "like_match_blobs", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "like_match_blobs", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_ATTACH
  Tcl_سیٹ_Var2(interp, "sqlite_options", "attach", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "attach", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  Tcl_سیٹ_Var2(interp, "sqlite_options", "auth", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "auth", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_AUTOINCREMENT
  Tcl_سیٹ_Var2(interp, "sqlite_options", "autoinc", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "autoinc", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_AUTOMATIC_اشاریہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "autoindex", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "autoindex", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_AUTORESET
  Tcl_سیٹ_Var2(interp, "sqlite_options", "autoreset", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "autoreset", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  Tcl_سیٹ_Var2(interp, "sqlite_options", "autovacuum", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "autovacuum", "1", TCL_GLOBAL_ONLY);
#اگر_ختم /* SQLITE_OMIT_AUTOVACUUM */
#اگر !defined(SQLITE_DEFAULT_AUTOVACUUM)
  Tcl_سیٹ_Var2(interp,"sqlite_options","default_autovacuum","0",TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "default_autovacuum", 
      STRINGVALUE(SQLITE_DEFAULT_AUTOVACUUM), TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_BETWEEN_OPTIMIZATION
  Tcl_سیٹ_Var2(interp, "sqlite_options", "between_opt", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "between_opt", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_UNTESTABLE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "builtin_test", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "builtin_test", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_BLOB_LITERAL
  Tcl_سیٹ_Var2(interp, "sqlite_options", "bloblit", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "bloblit", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_CAST
  Tcl_سیٹ_Var2(interp, "sqlite_options", "cast", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "cast", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_CHECK
  Tcl_سیٹ_Var2(interp, "sqlite_options", "check", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "check", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_CTE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "cte", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "cte", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA
  Tcl_سیٹ_Var2(interp, "sqlite_options", "columnmetadata", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "columnmetadata", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_ORDEREDSETFUNC
  Tcl_سیٹ_Var2(interp, "sqlite_options", "ordered_set_funcs", "1",
              TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "ordered_set_funcs", "0",
              TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_OVERSIZE_CELL_CHECK
  Tcl_سیٹ_Var2(interp, "sqlite_options", "oversize_cell_check", "1",
              TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "oversize_cell_check", "0",
              TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_COMPILEOPTION_DIAGS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "compileoption_diags", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "compileoption_diags", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_COMPLETE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "complete", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "complete", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_COMPOUND_SELECT
  Tcl_سیٹ_Var2(interp, "sqlite_options", "compound", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "compound", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

  Tcl_سیٹ_Var2(interp, "sqlite_options", "conflict", "1", TCL_GLOBAL_ONLY);
  Tcl_سیٹ_Var2(interp, "sqlite_options", "crashtest", "1", TCL_GLOBAL_ONLY);

#اگر_تعریف_شدہ SQLITE_OMIT_DATETIME_FUNCS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "datetime", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "datetime", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_DECLTYPE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "decltype", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "decltype", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
  Tcl_سیٹ_Var2(interp, "sqlite_options", "deprecated", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "deprecated", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_DISKIO
  Tcl_سیٹ_Var2(interp, "sqlite_options", "diskio", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "diskio", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
  Tcl_سیٹ_Var2(interp, "sqlite_options", "explain", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "explain", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "floatingpoint", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "floatingpoint", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_FOREIGN_کلید
  Tcl_سیٹ_Var2(interp, "sqlite_options", "foreignkey", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "foreignkey", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_FTS3
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fts3", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fts3", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_FTS5
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fts5", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fts5", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_ENABLE_FTS3) && !defined(SQLITE_DISABLE_FTS3_UNICODE)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fts3_unicode", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fts3_unicode", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DISABLE_FTS4_DEFERRED
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fts4_deferred", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fts4_deferred", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_GET_جدول
  Tcl_سیٹ_Var2(interp, "sqlite_options", "gettable", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "gettable", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_ICU
  Tcl_سیٹ_Var2(interp, "sqlite_options", "icu", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "icu", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_ICU_COLLATIONS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "icu_collations", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "icu_collations", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
  Tcl_سیٹ_Var2(interp, "sqlite_options", "incrblob", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "incrblob", "1", TCL_GLOBAL_ONLY);
#اگر_ختم /* SQLITE_OMIT_AUTOVACUUM */

#اگر_تعریف_شدہ SQLITE_OMIT_INTEGRITY_CHECK
  Tcl_سیٹ_Var2(interp, "sqlite_options", "integrityck", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "integrityck", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_DEFAULT_FILE_FORMAT) && SQLITE_DEFAULT_FILE_FORMAT==1
  Tcl_سیٹ_Var2(interp, "sqlite_options", "legacyformat", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "legacyformat", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_LIKE_OPTIMIZATION
  Tcl_سیٹ_Var2(interp, "sqlite_options", "like_opt", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "like_opt", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
  Tcl_سیٹ_Var2(interp, "sqlite_options", "load_ext", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "load_ext", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_LOCALTIME
  Tcl_سیٹ_Var2(interp, "sqlite_options", "localtime", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "localtime", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_LOOKASIDE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "lookaside", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "lookaside", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_MEMORYDB
  Tcl_سیٹ_Var2(interp, "sqlite_options", "memorydb", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "memorydb", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
  Tcl_سیٹ_Var2(interp, "sqlite_options", "memorymanage", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "memorymanage", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

Tcl_سیٹ_Var2(interp, "sqlite_options", "mergesort", "1", TCL_GLOBAL_ONLY);

#اگر_تعریف_شدہ SQLITE_ENABLE_NULL_TRIM
  Tcl_سیٹ_Var2(interp, "sqlite_options", "null_trim", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "null_trim", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_OR_OPTIMIZATION
  Tcl_سیٹ_Var2(interp, "sqlite_options", "or_opt", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "or_opt", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_RBU
  Tcl_سیٹ_Var2(interp, "sqlite_options", "rbu", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "rbu", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_PAGER_PRAGMAS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "pager_pragmas", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "pager_pragmas", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_OMIT_PRAGMA) || defined(SQLITE_OMIT_FLAG_PRAGMAS)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "pragma", "0", TCL_GLOBAL_ONLY);
  Tcl_سیٹ_Var2(interp, "sqlite_options", "integrityck", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "pragma", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_پیش_رفت_CALLBACK
  Tcl_سیٹ_Var2(interp, "sqlite_options", "progress", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "progress", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_REINDEX
  Tcl_سیٹ_Var2(interp, "sqlite_options", "reindex", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "reindex", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_RTREE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "rtree", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "rtree", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_RTREE_INT_ONLY
  Tcl_سیٹ_Var2(interp, "sqlite_options", "rtree_int_only", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "rtree_int_only", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_SCHEMA_PRAGMAS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "schema_pragmas", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "schema_pragmas", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_SCHEMA_نسخہ_PRAGMAS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "schema_version", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "schema_version", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "session", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "session", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  Tcl_سیٹ_Var2(interp, "sqlite_options", "stat4", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "stat4", "0", TCL_GLOBAL_ONLY);
#اگر_ختم
#اگر defined(SQLITE_ENABLE_STMTVTAB) && !defined(SQLITE_OMIT_VIRTUALTABLE)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "stmtvtab", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "stmtvtab", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "scanstatus", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "scanstatus", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  اگر defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  ورنہ
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#اگر_ختم
#اگر SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
  Tcl_سیٹ_Var2(interp,"sqlite_options","lock_proxy_pragmas","1",TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp,"sqlite_options","lock_proxy_pragmas","0",TCL_GLOBAL_ONLY);
#اگر_ختم
#اگر defined(SQLITE_PREFER_PROXY_LOCKING) && defined(__APPLE__)
  Tcl_سیٹ_Var2(interp,"sqlite_options","prefer_proxy_locking","1",TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp,"sqlite_options","prefer_proxy_locking","0",TCL_GLOBAL_ONLY);
#اگر_ختم
    
    
#اگر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "shared_cache", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "shared_cache", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
  Tcl_سیٹ_Var2(interp, "sqlite_options", "subquery", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "subquery", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_TCL_VARIABLE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "tclvar", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "tclvar", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

  Tcl_سیٹ_Var2(interp, "sqlite_options", "threadsafe", 
      SQLITE_THREADSAFE ? "1" : "0", TCL_GLOBAL_ONLY);
  Tcl_سیٹ_Var2(interp, "sqlite_options", "threadsafe1", 
      SQLITE_THREADSAFE==1 ? "1" : "0", TCL_GLOBAL_ONLY);
  Tcl_سیٹ_Var2(interp, "sqlite_options", "threadsafe2", 
      SQLITE_THREADSAFE==2 ? "1" : "0", TCL_GLOBAL_ONLY);
  أكد( sqlite3_threadsafe()==SQLITE_THREADSAFE );

#اگر_تعریف_شدہ SQLITE_OMIT_TEMPDB
  Tcl_سیٹ_Var2(interp, "sqlite_options", "tempdb", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "tempdb", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_TRACE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "trace", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "trace", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_THREAD_MISUSE_WARNINGS
  Tcl_سیٹ_Var2(interp, "sqlite_options", "thread_misuse_warnings",
                                        "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "thread_misuse_warnings",
                                        "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_TRIGGER
  Tcl_سیٹ_Var2(interp, "sqlite_options", "trigger", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "trigger", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_TRUNCATE_OPTIMIZATION
  Tcl_سیٹ_Var2(interp, "sqlite_options", "truncate_opt", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "truncate_opt", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_UTF16
  Tcl_سیٹ_Var2(interp, "sqlite_options", "utf16", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "utf16", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_OMIT_VACUUM) || defined(SQLITE_OMIT_ATTACH)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "vacuum", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "vacuum", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_VIEW
  Tcl_سیٹ_Var2(interp, "sqlite_options", "view", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "view", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "vtab", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "vtab", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_WAL
  Tcl_سیٹ_Var2(interp, "sqlite_options", "wal", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "wal", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_WSD
  Tcl_سیٹ_Var2(interp, "sqlite_options", "wsd", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "wsd", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_ENABLE_UPDATE_DELETE_حد_بندی) && !defined(SQLITE_OMIT_SUBQUERY)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "update_delete_limit", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "update_delete_limit", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "unlock_notify", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "unlock_notify", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_FAST_SECURE_DELETE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fast_secure_delete", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "fast_secure_delete", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_SECURE_DELETE
  Tcl_سیٹ_Var2(interp, "sqlite_options", "secure_delete", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "secure_delete", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_MULTIPLEX_EXT_OVWR
  Tcl_سیٹ_Var2(interp, "sqlite_options", "multiplex_ext_overwrite", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "multiplex_ext_overwrite", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ YYTRACKMAXSTACKDEPTH
  Tcl_سیٹ_Var2(interp, "sqlite_options", "yytrackmaxstackdepth", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "yytrackmaxstackdepth", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_SQLLOG
  Tcl_سیٹ_Var2(interp, "sqlite_options", "sqllog", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "sqllog", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_URI_00_ERROR
  Tcl_سیٹ_Var2(interp, "sqlite_options", "uri_00_error", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "uri_00_error", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر defined(SQLITE_ENABLE_NORMALIZE)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "normalize", "1", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "normalize", "0", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  Tcl_سیٹ_Var2(interp, "sqlite_options", "windowfunc", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "windowfunc", "1", TCL_GLOBAL_ONLY);
#اگر_ختم

#اگر !defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  Tcl_سیٹ_Var2(interp, "sqlite_options", "setlk_timeout", "0", TCL_GLOBAL_ONLY);
#ورنہ
  Tcl_سیٹ_Var2(interp, "sqlite_options", "setlk_timeout", 
      STRINGVALUE(SQLITE_ENABLE_SETLK_TIMEOUT), TCL_GLOBAL_ONLY);
#اگر_ختم

#تعریف_کرو LINKVAR(x) { \
    ساکن ثابت صحیح cv_ ## x = SQLITE_ ## x; \
    Tcl_LinkVar(interp, "SQLITE_" #x, (حرف *)&(cv_ ## x), \
                TCL_LINK_INT | TCL_LINK_READ_ONLY); }

  LINKVAR( MAX_لمبائی );
  LINKVAR( MAX_عمود );
  LINKVAR( MAX_SQL_لمبائی );
  LINKVAR( MAX_اظہاریہ_گہرائی );
  LINKVAR( MAX_COMPOUND_SELECT );
  LINKVAR( MAX_VDBE_OP );
  LINKVAR( MAX_FUNCTION_ARG );
  LINKVAR( MAX_VARIABLE_نمبر );
  LINKVAR( MAX_صفحہ_سائز );
  LINKVAR( MAX_صفحہ_گنتی );
  LINKVAR( MAX_LIKE_نمونہ_لمبائی );
  LINKVAR( MAX_TRIGGER_گہرائی );
  LINKVAR( DEFAULT_CACHE_سائز );
  LINKVAR( DEFAULT_صفحہ_سائز );
  LINKVAR( DEFAULT_FILE_FORMAT );
  LINKVAR( DEFAULT_SYNCHRONOUS );
  LINKVAR( DEFAULT_WAL_SYNCHRONOUS );
  LINKVAR( MAX_ATTACHED );
  LINKVAR( MAX_DEFAULT_صفحہ_سائز );
  LINKVAR( MAX_کارکن_دھاگے );
  LINKVAR( MAX_SCHEMA );
  LINKVAR( MAX_TRIGGER_اقدام );

  {
    ساکن ثابت صحیح cv_عارضی_STORE = SQLITE_عارضی_STORE;
    Tcl_LinkVar(interp, "TEMP_STORE", (حرف *)&(cv_عارضی_STORE),
                TCL_LINK_INT | TCL_LINK_READ_ONLY);
  }

#اگر_تعریف_شدہ _MSC_VER
  {
    ساکن ثابت صحیح cv__MSC_VER = 1;
    Tcl_LinkVar(interp, "_MSC_VER", (حرف *)&(cv__MSC_VER),
                TCL_LINK_INT | TCL_LINK_READ_ONLY);
  }
#اگر_ختم
#اگر_تعریف_شدہ __GNUC__
  {
    ساکن ثابت صحیح cv___GNUC__ = 1;
    Tcl_LinkVar(interp, "__GNUC__", (حرف *)&(cv___GNUC__),
                TCL_LINK_INT | TCL_LINK_READ_ONLY);
  }
#اگر_ختم
}


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqliteconfig_Init(Tcl_Interp *interp){
  سیٹ_options(interp);
  واپس TCL_OK;
}
