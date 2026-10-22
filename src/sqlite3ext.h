/*
** 2006 June 7
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This header file defines the SQLite interface for use by
** shared libraries that want to be imported as extensions into
** an SQLite instance.  Shared libraries that intend to be loaded
** as extensions by SQLite should #include this file instead of 
** sqlite3.h.
*/
#ifndef SQLITE3EXT_H
#define SQLITE3EXT_H
#অন্তর্ভাৱ "sqlite3.h"

/*
** The following structure holds pointers to all of the SQLite API
** routines.
**
** WARNING:  In order to maintain backwards compatibility, add new
** interfaces to the end of this structure only.  If you insert new
** interfaces in the middle of this structure, then older different
** versions of SQLite will not be able to load each other's shared
** libraries!
*/
সংরচনা sqlite3_api_routines {
  শূন্য * (*aggregate_context)(sqlite3_context*,পূর্ণাঙ্ক nBytes);
  পূর্ণাঙ্ক  (*aggregate_গুনতি)(sqlite3_context*);
  পূর্ণাঙ্ক  (*bind_blob)(sqlite3_stmt*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক n,শূন্য(*)(শূন্য*));
  পূর্ণাঙ্ক  (*bind_double)(sqlite3_stmt*,পূর্ণাঙ্ক,দ্ৱিগুণ);
  পূর্ণাঙ্ক  (*bind_int)(sqlite3_stmt*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক  (*bind_int64)(sqlite3_stmt*,পূর্ণাঙ্ক,sqlite_int64);
  পূর্ণাঙ্ক  (*bind_null)(sqlite3_stmt*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক  (*bind_parameter_গুনতি)(sqlite3_stmt*);
  পূর্ণাঙ্ক  (*bind_parameter_index)(sqlite3_stmt*,ধ্রুৱ ৱর্ণ*zName);
  ধ্রুৱ ৱর্ণ * (*bind_parameter_name)(sqlite3_stmt*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক  (*bind_text)(sqlite3_stmt*,পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক n,শূন্য(*)(শূন্য*));
  পূর্ণাঙ্ক  (*bind_text16)(sqlite3_stmt*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,শূন্য(*)(শূন্য*));
  পূর্ণাঙ্ক  (*bind_মান)(sqlite3_stmt*,পূর্ণাঙ্ক,ধ্রুৱ sqlite3_মান*);
  পূর্ণাঙ্ক  (*busy_handler)(sqlite3*,পূর্ণাঙ্ক(*)(শূন্য*,পূর্ণাঙ্ক),শূন্য*);
  পূর্ণাঙ্ক  (*busy_timeout)(sqlite3*,পূর্ণাঙ্ক ms);
  পূর্ণাঙ্ক  (*changes)(sqlite3*);
  পূর্ণাঙ্ক  (*close)(sqlite3*);
  পূর্ণাঙ্ক  (*collation_needed)(sqlite3*,শূন্য*,শূন্য(*)(শূন্য*,sqlite3*,
                           পূর্ণাঙ্ক eTextRep,ধ্রুৱ ৱর্ণ*));
  পূর্ণাঙ্ক  (*collation_needed16)(sqlite3*,শূন্য*,শূন্য(*)(শূন্য*,sqlite3*,
                             পূর্ণাঙ্ক eTextRep,ধ্রুৱ শূন্য*));
  ধ্রুৱ শূন্য * (*column_blob)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  পূর্ণাঙ্ক  (*column_bytes)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  পূর্ণাঙ্ক  (*column_bytes16)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  পূর্ণাঙ্ক  (*column_গুনতি)(sqlite3_stmt*pStmt);
  ধ্রুৱ ৱর্ণ * (*column_database_name)(sqlite3_stmt*,পূর্ণাঙ্ক);
  ধ্রুৱ শূন্য * (*column_database_name16)(sqlite3_stmt*,পূর্ণাঙ্ক);
  ধ্রুৱ ৱর্ণ * (*column_decltype)(sqlite3_stmt*,পূর্ণাঙ্ক i);
  ধ্রুৱ শূন্য * (*column_decltype16)(sqlite3_stmt*,পূর্ণাঙ্ক);
  দ্ৱিগুণ  (*column_double)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  পূর্ণাঙ্ক  (*column_int)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  ধ্রুৱ ৱর্ণ * (*column_name)(sqlite3_stmt*,পূর্ণাঙ্ক);
  ধ্রুৱ শূন্য * (*column_name16)(sqlite3_stmt*,পূর্ণাঙ্ক);
  ধ্রুৱ ৱর্ণ * (*column_origin_name)(sqlite3_stmt*,পূর্ণাঙ্ক);
  ধ্রুৱ শূন্য * (*column_origin_name16)(sqlite3_stmt*,পূর্ণাঙ্ক);
  ধ্রুৱ ৱর্ণ * (*column_table_name)(sqlite3_stmt*,পূর্ণাঙ্ক);
  ধ্রুৱ শূন্য * (*column_table_name16)(sqlite3_stmt*,পূর্ণাঙ্ক);
  ধ্রুৱ unsigned ৱর্ণ * (*column_text)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  ধ্রুৱ শূন্য * (*column_text16)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  পূর্ণাঙ্ক  (*column_type)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  sqlite3_মান* (*column_মান)(sqlite3_stmt*,পূর্ণাঙ্ক iCol);
  শূন্য * (*commit_hook)(sqlite3*,পূর্ণাঙ্ক(*)(শূন্য*),শূন্য*);
  পূর্ণাঙ্ক  (*complete)(ধ্রুৱ ৱর্ণ*sql);
  পূর্ণাঙ্ক  (*complete16)(ধ্রুৱ শূন্য*sql);
  পূর্ণাঙ্ক  (*create_collation)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,শূন্য*,
                           পূর্ণাঙ্ক(*)(শূন্য*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*));
  পূর্ণাঙ্ক  (*create_collation16)(sqlite3*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,শূন্য*,
                             পূর্ণাঙ্ক(*)(শূন্য*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*));
  পূর্ণাঙ্ক  (*create_function)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,শূন্য*,
                          শূন্য (*xFunc)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**),
                          শূন্য (*xStep)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**),
                          শূন্য (*xFinal)(sqlite3_context*));
  পূর্ণাঙ্ক  (*create_function16)(sqlite3*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,শূন্য*,
                            শূন্য (*xFunc)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**),
                            শূন্য (*xStep)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**),
                            শূন্য (*xFinal)(sqlite3_context*));
  পূর্ণাঙ্ক (*create_module)(sqlite3*,ধ্রুৱ ৱর্ণ*,ধ্রুৱ sqlite3_module*,শূন্য*);
  পূর্ণাঙ্ক  (*data_গুনতি)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  পূর্ণাঙ্ক (*declare_vtab)(sqlite3*,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক  (*enable_shared_cache)(পূর্ণাঙ্ক);
  পূর্ণাঙ্ক  (*errcode)(sqlite3*db);
  ধ্রুৱ ৱর্ণ * (*errmsg)(sqlite3*);
  ধ্রুৱ শূন্য * (*errmsg16)(sqlite3*);
  পূর্ণাঙ্ক  (*exec)(sqlite3*,ধ্রুৱ ৱর্ণ*,sqlite3_callback,শূন্য*,ৱর্ণ**);
  পূর্ণাঙ্ক  (*expired)(sqlite3_stmt*);
  পূর্ণাঙ্ক  (*finalize)(sqlite3_stmt*pStmt);
  শূন্য  (*মুক্ত)(শূন্য*);
  শূন্য  (*free_table)(ৱর্ণ**result);
  পূর্ণাঙ্ক  (*get_autocommit)(sqlite3*);
  শূন্য * (*get_auxdata)(sqlite3_context*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক  (*get_table)(sqlite3*,ধ্রুৱ ৱর্ণ*,ৱর্ণ***,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*,ৱর্ণ**);
  পূর্ণাঙ্ক  (*global_recover)(শূন্য);
  শূন্য  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_ঢোকাও_rowid)(sqlite3*);
  ধ্রুৱ ৱর্ণ * (*libversion)(শূন্য);
  পূর্ণাঙ্ক  (*libversion_সংখ্যা)(শূন্য);
  শূন্য *(*malloc)(পূর্ণাঙ্ক);
  ৱর্ণ *(SQLITE_CDECL*mprintf)(ধ্রুৱ ৱর্ণ*,...);
  পূর্ণাঙ্ক  (*open)(ধ্রুৱ ৱর্ণ*,sqlite3**);
  পূর্ণাঙ্ক  (*open16)(ধ্রুৱ শূন্য*,sqlite3**);
  পূর্ণাঙ্ক  (*prepare)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,sqlite3_stmt**,ধ্রুৱ ৱর্ণ**);
  পূর্ণাঙ্ক  (*prepare16)(sqlite3*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,sqlite3_stmt**,ধ্রুৱ শূন্য**);
  শূন্য * (*profile)(sqlite3*,শূন্য(*)(শূন্য*,ধ্রুৱ ৱর্ণ*,sqlite_uint64),শূন্য*);
  শূন্য  (*progress_handler)(sqlite3*,পূর্ণাঙ্ক,পূর্ণাঙ্ক(*)(শূন্য*),শূন্য*);
  শূন্য *(*realloc)(শূন্য*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক  (*reset)(sqlite3_stmt*pStmt);
  শূন্য  (*result_blob)(sqlite3_context*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,শূন্য(*)(শূন্য*));
  শূন্য  (*result_double)(sqlite3_context*,দ্ৱিগুণ);
  শূন্য  (*result_error)(sqlite3_context*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক);
  শূন্য  (*result_error16)(sqlite3_context*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক);
  শূন্য  (*result_int)(sqlite3_context*,পূর্ণাঙ্ক);
  শূন্য  (*result_int64)(sqlite3_context*,sqlite_int64);
  শূন্য  (*result_null)(sqlite3_context*);
  শূন্য  (*result_text)(sqlite3_context*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,শূন্য(*)(শূন্য*));
  শূন্য  (*result_text16)(sqlite3_context*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,শূন্য(*)(শূন্য*));
  শূন্য  (*result_text16be)(sqlite3_context*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,শূন্য(*)(শূন্য*));
  শূন্য  (*result_text16le)(sqlite3_context*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,শূন্য(*)(শূন্য*));
  শূন্য  (*result_মান)(sqlite3_context*,sqlite3_মান*);
  শূন্য *(*rollback_hook)(sqlite3*,শূন্য(*)(শূন্য*),শূন্য*);
  পূর্ণাঙ্ক  (*set_authorizer)(sqlite3*,পূর্ণাঙ্ক(*)(শূন্য*,পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,
                         ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*),শূন্য*);
  শূন্য  (*set_auxdata)(sqlite3_context*,পূর্ণাঙ্ক,শূন্য*,শূন্য (*)(শূন্য*));
  ৱর্ণ *(SQLITE_CDECL*xsnprintf)(পূর্ণাঙ্ক,ৱর্ণ*,ধ্রুৱ ৱর্ণ*,...);
  পূর্ণাঙ্ক  (*step)(sqlite3_stmt*);
  পূর্ণাঙ্ক  (*table_column_metadata)(sqlite3*,ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,
                                ৱর্ণ ধ্রুৱ**,ৱর্ণ ধ্রুৱ**,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*);
  শূন্য  (*thread_cleanup)(শূন্য);
  পূর্ণাঙ্ক  (*total_changes)(sqlite3*);
  শূন্য * (*trace)(sqlite3*,শূন্য(*xTrace)(শূন্য*,ধ্রুৱ ৱর্ণ*),শূন্য*);
  পূর্ণাঙ্ক  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  শূন্য * (*update_hook)(sqlite3*,শূন্য(*)(শূন্য*,পূর্ণাঙ্ক ,ৱর্ণ ধ্রুৱ*,ৱর্ণ ধ্রুৱ*,
                                         sqlite_int64),শূন্য*);
  শূন্য * (*user_data)(sqlite3_context*);
  ধ্রুৱ শূন্য * (*মান_blob)(sqlite3_মান*);
  পূর্ণাঙ্ক  (*মান_bytes)(sqlite3_মান*);
  পূর্ণাঙ্ক  (*মান_bytes16)(sqlite3_মান*);
  দ্ৱিগুণ  (*মান_double)(sqlite3_মান*);
  পূর্ণাঙ্ক  (*মান_int)(sqlite3_মান*);
  sqlite_int64  (*মান_int64)(sqlite3_মান*);
  পূর্ণাঙ্ক  (*মান_numeric_type)(sqlite3_মান*);
  ধ্রুৱ unsigned ৱর্ণ * (*মান_text)(sqlite3_মান*);
  ধ্রুৱ শূন্য * (*মান_text16)(sqlite3_মান*);
  ধ্রুৱ শূন্য * (*মান_text16be)(sqlite3_মান*);
  ধ্রুৱ শূন্য * (*মান_text16le)(sqlite3_মান*);
  পূর্ণাঙ্ক  (*মান_type)(sqlite3_মান*);
  ৱর্ণ *(*vmprintf)(ধ্রুৱ ৱর্ণ*,va_list);
  /* Added ??? */
  পূর্ণাঙ্ক (*overload_function)(sqlite3*, ধ্রুৱ ৱর্ণ *zFuncName, পূর্ণাঙ্ক nArg);
  /* Added by 3.3.13 */
  পূর্ণাঙ্ক (*prepare_v2)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,sqlite3_stmt**,ধ্রুৱ ৱর্ণ**);
  পূর্ণাঙ্ক (*prepare16_v2)(sqlite3*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,sqlite3_stmt**,ধ্রুৱ শূন্য**);
  পূর্ণাঙ্ক (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  পূর্ণাঙ্ক (*create_module_v2)(sqlite3*,ধ্রুৱ ৱর্ণ*,ধ্রুৱ sqlite3_module*,শূন্য*,
                          শূন্য (*xDestroy)(শূন্য *));
  /* Added by 3.5.0 */
  পূর্ণাঙ্ক (*bind_zeroblob)(sqlite3_stmt*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*blob_bytes)(sqlite3_blob*);
  পূর্ণাঙ্ক (*blob_close)(sqlite3_blob*);
  পূর্ণাঙ্ক (*blob_open)(sqlite3*,ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,sqlite3_int64,
                   পূর্ণাঙ্ক,sqlite3_blob**);
  পূর্ণাঙ্ক (*blob_read)(sqlite3_blob*,শূন্য*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*blob_write)(sqlite3_blob*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*create_collation_v2)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,শূন্য*,
                             পূর্ণাঙ্ক(*)(শূন্য*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*),
                             শূন্য(*)(শূন্য*));
  পূর্ণাঙ্ক (*file_control)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,শূন্য*);
  sqlite3_int64 (*memory_highwater)(পূর্ণাঙ্ক);
  sqlite3_int64 (*memory_used)(শূন্য);
  sqlite3_mutex *(*mutex_alloc)(পূর্ণাঙ্ক);
  শূন্য (*mutex_enter)(sqlite3_mutex*);
  শূন্য (*mutex_free)(sqlite3_mutex*);
  শূন্য (*mutex_leave)(sqlite3_mutex*);
  পূর্ণাঙ্ক (*mutex_try)(sqlite3_mutex*);
  পূর্ণাঙ্ক (*open_v2)(ধ্রুৱ ৱর্ণ*,sqlite3**,পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*release_memory)(পূর্ণাঙ্ক);
  শূন্য (*result_error_nomem)(sqlite3_context*);
  শূন্য (*result_error_toobig)(sqlite3_context*);
  পূর্ণাঙ্ক (*sleep)(পূর্ণাঙ্ক);
  শূন্য (*soft_heap_limit)(পূর্ণাঙ্ক);
  sqlite3_vfs *(*vfs_find)(ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*vfs_register)(sqlite3_vfs*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*vfs_unregister)(sqlite3_vfs*);
  পূর্ণাঙ্ক (*xthreadsafe)(শূন্য);
  শূন্য (*result_zeroblob)(sqlite3_context*,পূর্ণাঙ্ক);
  শূন্য (*result_error_code)(sqlite3_context*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (SQLITE_CDECL*test_control)(পূর্ণাঙ্ক, ...);
  শূন্য (*randomness)(পূর্ণাঙ্ক,শূন্য*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  পূর্ণাঙ্ক (*extended_result_codes)(sqlite3*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*limit)(sqlite3*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
  sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
  ধ্রুৱ ৱর্ণ *(*sql)(sqlite3_stmt*);
  পূর্ণাঙ্ক (*status)(পূর্ণাঙ্ক,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ধ্রুৱ ৱর্ণ*,sqlite3*,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*backup_pagecount)(sqlite3_backup*);
  পূর্ণাঙ্ক (*backup_remaining)(sqlite3_backup*);
  পূর্ণাঙ্ক (*backup_step)(sqlite3_backup*,পূর্ণাঙ্ক);
  ধ্রুৱ ৱর্ণ *(*compileoption_get)(পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*compileoption_used)(ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*create_function_v2)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,শূন্য*,
                            শূন্য (*xFunc)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**),
                            শূন্য (*xStep)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**),
                            শূন্য (*xFinal)(sqlite3_context*),
                            শূন্য(*xDestroy)(শূন্য*));
  পূর্ণাঙ্ক (SQLITE_CDECL*db_config)(sqlite3*,পূর্ণাঙ্ক,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  পূর্ণাঙ্ক (*db_status)(sqlite3*,পূর্ণাঙ্ক,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*extended_errcode)(sqlite3*);
  শূন্য (SQLITE_CDECL*log)(পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  ধ্রুৱ ৱর্ণ *(*sourceid)(শূন্য);
  পূর্ণাঙ্ক (*stmt_status)(sqlite3_stmt*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*strnicmp)(ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*unlock_notify)(sqlite3*,শূন্য(*)(শূন্য**,পূর্ণাঙ্ক),শূন্য*);
  পূর্ণাঙ্ক (*wal_autocheckpoint)(sqlite3*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*wal_checkpoint)(sqlite3*,ধ্রুৱ ৱর্ণ*);
  শূন্য *(*wal_hook)(sqlite3*,পূর্ণাঙ্ক(*)(শূন্য*,sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক),শূন্য*);
  পূর্ণাঙ্ক (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  পূর্ণাঙ্ক (SQLITE_CDECL*vtab_config)(sqlite3*,পূর্ণাঙ্ক op,...);
  পূর্ণাঙ্ক (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  পূর্ণাঙ্ক (*close_v2)(sqlite3*);
  ধ্রুৱ ৱর্ণ *(*db_filename)(sqlite3*,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*db_readonly)(sqlite3*,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*db_release_memory)(sqlite3*);
  ধ্রুৱ ৱর্ণ *(*errstr)(পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*stmt_busy)(sqlite3_stmt*);
  পূর্ণাঙ্ক (*stmt_readonly)(sqlite3_stmt*);
  পূর্ণাঙ্ক (*stricmp)(ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*uri_boolean)(ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক);
  sqlite3_int64 (*uri_int64)(ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,sqlite3_int64);
  ধ্রুৱ ৱর্ণ *(*uri_parameter)(ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*);
  ৱর্ণ *(*xvsnprintf)(পূর্ণাঙ্ক,ৱর্ণ*,ধ্রুৱ ৱর্ণ*,va_list);
  পূর্ণাঙ্ক (*wal_checkpoint_v2)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*);
  /* Version 3.8.7 and later */
  পূর্ণাঙ্ক (*auto_extension)(শূন্য(*)(শূন্য));
  পূর্ণাঙ্ক (*bind_blob64)(sqlite3_stmt*,পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,sqlite3_uint64,
                     শূন্য(*)(শূন্য*));
  পূর্ণাঙ্ক (*bind_text64)(sqlite3_stmt*,পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ*,sqlite3_uint64,
                      শূন্য(*)(শূন্য*),unsigned ৱর্ণ);
  পূর্ণাঙ্ক (*cancel_auto_extension)(শূন্য(*)(শূন্য));
  পূর্ণাঙ্ক (*load_extension)(sqlite3*,ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,ৱর্ণ**);
  শূন্য *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(শূন্য*);
  শূন্য *(*realloc64)(শূন্য*,sqlite3_uint64);
  শূন্য (*reset_auto_extension)(শূন্য);
  শূন্য (*result_blob64)(sqlite3_context*,ধ্রুৱ শূন্য*,sqlite3_uint64,
                        শূন্য(*)(শূন্য*));
  শূন্য (*result_text64)(sqlite3_context*,ধ্রুৱ ৱর্ণ*,sqlite3_uint64,
                         শূন্য(*)(শূন্য*), unsigned ৱর্ণ);
  পূর্ণাঙ্ক (*strglob)(ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*);
  /* Version 3.8.11 and later */
  sqlite3_মান *(*মান_dup)(ধ্রুৱ sqlite3_মান*);
  শূন্য (*মান_free)(sqlite3_মান*);
  পূর্ণাঙ্ক (*result_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  পূর্ণাঙ্ক (*bind_zeroblob64)(sqlite3_stmt*, পূর্ণাঙ্ক, sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned পূর্ণাঙ্ক (*মান_subtype)(sqlite3_মান*);
  শূন্য (*result_subtype)(sqlite3_context*,unsigned পূর্ণাঙ্ক);
  /* Version 3.10.0 and later */
  পূর্ণাঙ্ক (*status64)(পূর্ণাঙ্ক,sqlite3_int64*,sqlite3_int64*,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*strlike)(ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,unsigned পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  পূর্ণাঙ্ক (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  পূর্ণাঙ্ক (*trace_v2)(sqlite3*,unsigned,পূর্ণাঙ্ক(*)(unsigned,শূন্য*,শূন্য*,শূন্য*),শূন্য*);
  ৱর্ণ *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  শূন্য (*set_last_ঢোকাও_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  পূর্ণাঙ্ক (*prepare_v3)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,unsigned পূর্ণাঙ্ক,
                    sqlite3_stmt**,ধ্রুৱ ৱর্ণ**);
  পূর্ণাঙ্ক (*prepare16_v3)(sqlite3*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক,unsigned পূর্ণাঙ্ক,
                      sqlite3_stmt**,ধ্রুৱ শূন্য**);
  পূর্ণাঙ্ক (*bind_pointer)(sqlite3_stmt*,পূর্ণাঙ্ক,শূন্য*,ধ্রুৱ ৱর্ণ*,শূন্য(*)(শূন্য*));
  শূন্য (*result_pointer)(sqlite3_context*,শূন্য*,ধ্রুৱ ৱর্ণ*,শূন্য(*)(শূন্য*));
  শূন্য *(*মান_pointer)(sqlite3_মান*,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*vtab_nochange)(sqlite3_context*);
  পূর্ণাঙ্ক (*মান_nochange)(sqlite3_মান*);
  ধ্রুৱ ৱর্ণ *(*vtab_collation)(sqlite3_index_info*,পূর্ণাঙ্ক);
  /* Version 3.24.0 and later */
  পূর্ণাঙ্ক (*keyword_গুনতি)(শূন্য);
  পূর্ণাঙ্ক (*keyword_name)(পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ**,পূর্ণাঙ্ক*);
  পূর্ণাঙ্ক (*keyword_check)(ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক);
  sqlite3_str *(*str_new)(sqlite3*);
  ৱর্ণ *(*str_finish)(sqlite3_str*);
  শূন্য (SQLITE_CDECL*str_appendf)(sqlite3_str*, ধ্রুৱ ৱর্ণ *zFormat, ...);
  শূন্য (*str_vappendf)(sqlite3_str*, ধ্রুৱ ৱর্ণ *zFormat, va_list);
  শূন্য (*str_append)(sqlite3_str*, ধ্রুৱ ৱর্ণ *zIn, পূর্ণাঙ্ক N);
  শূন্য (*str_appendall)(sqlite3_str*, ধ্রুৱ ৱর্ণ *zIn);
  শূন্য (*str_appendchar)(sqlite3_str*, পূর্ণাঙ্ক N, ৱর্ণ C);
  শূন্য (*str_reset)(sqlite3_str*);
  পূর্ণাঙ্ক (*str_errcode)(sqlite3_str*);
  পূর্ণাঙ্ক (*str_length)(sqlite3_str*);
  ৱর্ণ *(*str_মান)(sqlite3_str*);
  /* Version 3.25.0 and later */
  পূর্ণাঙ্ক (*create_window_function)(sqlite3*,ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,শূন্য*,
                            শূন্য (*xStep)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**),
                            শূন্য (*xFinal)(sqlite3_context*),
                            শূন্য (*x_মান)(sqlite3_context*),
                            শূন্য (*xInv)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**),
                            শূন্য(*xDestroy)(শূন্য*));
  /* Version 3.26.0 and later */
  ধ্রুৱ ৱর্ণ *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  পূর্ণাঙ্ক (*stmt_isexplain)(sqlite3_stmt*);
  পূর্ণাঙ্ক (*মান_frombind)(sqlite3_মান*);
  /* Version 3.30.0 and later */
  পূর্ণাঙ্ক (*drop_modules)(sqlite3*,ধ্রুৱ ৱর্ণ**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  ধ্রুৱ ৱর্ণ *(*uri_চাবি)(ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক);
  ধ্রুৱ ৱর্ণ *(*filename_database)(ধ্রুৱ ৱর্ণ*);
  ধ্রুৱ ৱর্ণ *(*filename_journal)(ধ্রুৱ ৱর্ণ*);
  ধ্রুৱ ৱর্ণ *(*filename_wal)(ধ্রুৱ ৱর্ণ*);
  /* Version 3.32.0 and later */
  ধ্রুৱ ৱর্ণ *(*create_filename)(ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,ধ্রুৱ ৱর্ণ*,
                           পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ**);
  শূন্য (*free_filename)(ধ্রুৱ ৱর্ণ*);
  sqlite3_file *(*database_file_object)(ধ্রুৱ ৱর্ণ*);
  /* Version 3.34.0 and later */
  পূর্ণাঙ্ক (*txn_state)(sqlite3*,ধ্রুৱ ৱর্ণ*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*total_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  পূর্ণাঙ্ক (*autovacuum_pages)(sqlite3*,
     unsigned পূর্ণাঙ্ক(*)(শূন্য*,ধ্রুৱ ৱর্ণ*,unsigned পূর্ণাঙ্ক,unsigned পূর্ণাঙ্ক,unsigned পূর্ণাঙ্ক),
     শূন্য*, শূন্য(*)(শূন্য*));
  /* Version 3.38.0 and later */
  পূর্ণাঙ্ক (*error_offset)(sqlite3*);
  পূর্ণাঙ্ক (*vtab_rhs_মান)(sqlite3_index_info*,পূর্ণাঙ্ক,sqlite3_মান**);
  পূর্ণাঙ্ক (*vtab_distinct)(sqlite3_index_info*);
  পূর্ণাঙ্ক (*vtab_in)(sqlite3_index_info*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
  পূর্ণাঙ্ক (*vtab_in_first)(sqlite3_মান*,sqlite3_মান**);
  পূর্ণাঙ্ক (*vtab_in_next)(sqlite3_মান*,sqlite3_মান**);
  /* Version 3.39.0 and later */
  পূর্ণাঙ্ক (*deserialize)(sqlite3*,ধ্রুৱ ৱর্ণ*,unsigned ৱর্ণ*,
                     sqlite3_int64,sqlite3_int64,unsigned);
  unsigned ৱর্ণ *(*serialize)(sqlite3*,ধ্রুৱ ৱর্ণ *,sqlite3_int64*,
                              unsigned পূর্ণাঙ্ক);
  ধ্রুৱ ৱর্ণ *(*db_name)(sqlite3*,পূর্ণাঙ্ক);
  /* Version 3.40.0 and later */
  পূর্ণাঙ্ক (*মান_encoding)(sqlite3_মান*);
  /* Version 3.41.0 and later */
  পূর্ণাঙ্ক (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  পূর্ণাঙ্ক (*stmt_explain)(sqlite3_stmt*,পূর্ণাঙ্ক);
  /* Version 3.44.0 and later */
  শূন্য *(*get_clientdata)(sqlite3*,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*set_clientdata)(sqlite3*, ধ্রুৱ ৱর্ণ*, শূন্য*, শূন্য(*)(শূন্য*));
  /* Version 3.50.0 and later */
  পূর্ণাঙ্ক (*setlk_timeout)(sqlite3*,পূর্ণাঙ্ক,পূর্ণাঙ্ক);
  /* Version 3.51.0 and later */
  পূর্ণাঙ্ক (*set_errmsg)(sqlite3*,পূর্ণাঙ্ক,ধ্রুৱ ৱর্ণ*);
  পূর্ণাঙ্ক (*db_status64)(sqlite3*,পূর্ণাঙ্ক,sqlite3_int64*,sqlite3_int64*,পূর্ণাঙ্ক);
  /* Version 3.52.0 and later */
  শূন্য (*str_truncate)(sqlite3_str*,পূর্ণাঙ্ক);
  শূন্য (*str_free)(sqlite3_str*);
  পূর্ণাঙ্ক (*carray_bind)(sqlite3_stmt*,পূর্ণাঙ্ক,শূন্য*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,শূন্য(*)(শূন্য*));
  পূর্ণাঙ্ক (*carray_bind_v2)(sqlite3_stmt*,পূর্ণাঙ্ক,শূন্য*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,শূন্য(*)(শূন্য*),শূন্য*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ধ্রুৱ ৱর্ণ*);
  শূন্য (*result_str)(sqlite3_context*,sqlite3_str*,পূর্ণাঙ্ক);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
প্রকারনির্দেশ পূর্ণাঙ্ক (*sqlite3_loadext_entry)(
  sqlite3 *db,                       /* Handle to the database. */
  ৱর্ণ **pz_Err_বাৰ্তা,                   /* Used to set error string on failure. */
  ধ্রুৱ sqlite3_api_routines *pThunk /* Extension API function pointers. */
);

/*
** The following macros redefine the API routines so that they are
** redirected through the global sqlite3_api structure.
**
** This header file is also used by the loadext.c source file
** (part of the main SQLite library - not an extension) so that
** it can get access to the sqlite3_api_routines structure
** definition.  But the main library does not want to redefine
** the API.  So the redefinition macros are only valid if the
** SQLITE_CORE macros is undefined.
*/
#যদি !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_aggregate_গুনতি        sqlite3_api->aggregate_গুনতি
#endif
#define sqlite3_bind_blob              sqlite3_api->bind_blob
#define sqlite3_bind_double            sqlite3_api->bind_double
#define sqlite3_bind_int               sqlite3_api->bind_int
#define sqlite3_bind_int64             sqlite3_api->bind_int64
#define sqlite3_bind_null              sqlite3_api->bind_null
#define sqlite3_bind_parameter_গুনতি   sqlite3_api->bind_parameter_গুনতি
#define sqlite3_bind_parameter_index   sqlite3_api->bind_parameter_index
#define sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#define sqlite3_bind_text              sqlite3_api->bind_text
#define sqlite3_bind_text16            sqlite3_api->bind_text16
#define sqlite3_bind_মান             sqlite3_api->bind_মান
#define sqlite3_busy_handler           sqlite3_api->busy_handler
#define sqlite3_busy_timeout           sqlite3_api->busy_timeout
#define sqlite3_changes                sqlite3_api->changes
#define sqlite3_close                  sqlite3_api->close
#define sqlite3_collation_needed       sqlite3_api->collation_needed
#define sqlite3_collation_needed16     sqlite3_api->collation_needed16
#define sqlite3_column_blob            sqlite3_api->column_blob
#define sqlite3_column_bytes           sqlite3_api->column_bytes
#define sqlite3_column_bytes16         sqlite3_api->column_bytes16
#define sqlite3_column_গুনতি           sqlite3_api->column_গুনতি
#define sqlite3_column_database_name   sqlite3_api->column_database_name
#define sqlite3_column_database_name16 sqlite3_api->column_database_name16
#define sqlite3_column_decltype        sqlite3_api->column_decltype
#define sqlite3_column_decltype16      sqlite3_api->column_decltype16
#define sqlite3_column_double          sqlite3_api->column_double
#define sqlite3_column_int             sqlite3_api->column_int
#define sqlite3_column_int64           sqlite3_api->column_int64
#define sqlite3_column_name            sqlite3_api->column_name
#define sqlite3_column_name16          sqlite3_api->column_name16
#define sqlite3_column_origin_name     sqlite3_api->column_origin_name
#define sqlite3_column_origin_name16   sqlite3_api->column_origin_name16
#define sqlite3_column_table_name      sqlite3_api->column_table_name
#define sqlite3_column_table_name16    sqlite3_api->column_table_name16
#define sqlite3_column_text            sqlite3_api->column_text
#define sqlite3_column_text16          sqlite3_api->column_text16
#define sqlite3_column_type            sqlite3_api->column_type
#define sqlite3_column_মান           sqlite3_api->column_মান
#define sqlite3_commit_hook            sqlite3_api->commit_hook
#define sqlite3_complete               sqlite3_api->complete
#define sqlite3_complete16             sqlite3_api->complete16
#define sqlite3_create_collation       sqlite3_api->create_collation
#define sqlite3_create_collation16     sqlite3_api->create_collation16
#define sqlite3_create_function        sqlite3_api->create_function
#define sqlite3_create_function16      sqlite3_api->create_function16
#define sqlite3_create_module          sqlite3_api->create_module
#define sqlite3_create_module_v2       sqlite3_api->create_module_v2
#define sqlite3_data_গুনতি             sqlite3_api->data_গুনতি
#define sqlite3_db_handle              sqlite3_api->db_handle
#define sqlite3_declare_vtab           sqlite3_api->declare_vtab
#define sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#define sqlite3_errcode                sqlite3_api->errcode
#define sqlite3_errmsg                 sqlite3_api->errmsg
#define sqlite3_errmsg16               sqlite3_api->errmsg16
#define sqlite3_exec                   sqlite3_api->exec
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_expired                sqlite3_api->expired
#endif
#define sqlite3_finalize               sqlite3_api->finalize
#define sqlite3_free                   sqlite3_api->মুক্ত
#define sqlite3_free_table             sqlite3_api->free_table
#define sqlite3_get_autocommit         sqlite3_api->get_autocommit
#define sqlite3_get_auxdata            sqlite3_api->get_auxdata
#define sqlite3_get_table              sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_global_recover         sqlite3_api->global_recover
#endif
#define sqlite3_interrupt              sqlite3_api->interruptx
#define sqlite3_last_ঢোকাও_rowid      sqlite3_api->last_ঢোকাও_rowid
#define sqlite3_libversion             sqlite3_api->libversion
#define sqlite3_libversion_সংখ্যা      sqlite3_api->libversion_সংখ্যা
#define sqlite3_malloc                 sqlite3_api->malloc
#define sqlite3_mprintf                sqlite3_api->mprintf
#define sqlite3_open                   sqlite3_api->open
#define sqlite3_open16                 sqlite3_api->open16
#define sqlite3_prepare                sqlite3_api->prepare
#define sqlite3_prepare16              sqlite3_api->prepare16
#define sqlite3_prepare_v2             sqlite3_api->prepare_v2
#define sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#define sqlite3_profile                sqlite3_api->profile
#define sqlite3_progress_handler       sqlite3_api->progress_handler
#define sqlite3_realloc                sqlite3_api->realloc
#define sqlite3_reset                  sqlite3_api->reset
#define sqlite3_result_blob            sqlite3_api->result_blob
#define sqlite3_result_double          sqlite3_api->result_double
#define sqlite3_result_error           sqlite3_api->result_error
#define sqlite3_result_error16         sqlite3_api->result_error16
#define sqlite3_result_int             sqlite3_api->result_int
#define sqlite3_result_int64           sqlite3_api->result_int64
#define sqlite3_result_null            sqlite3_api->result_null
#define sqlite3_result_text            sqlite3_api->result_text
#define sqlite3_result_text16          sqlite3_api->result_text16
#define sqlite3_result_text16be        sqlite3_api->result_text16be
#define sqlite3_result_text16le        sqlite3_api->result_text16le
#define sqlite3_result_মান           sqlite3_api->result_মান
#define sqlite3_rollback_hook          sqlite3_api->rollback_hook
#define sqlite3_set_authorizer         sqlite3_api->set_authorizer
#define sqlite3_set_auxdata            sqlite3_api->set_auxdata
#define sqlite3_snprintf               sqlite3_api->xsnprintf
#define sqlite3_step                   sqlite3_api->step
#define sqlite3_table_column_metadata  sqlite3_api->table_column_metadata
#define sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#define sqlite3_total_changes          sqlite3_api->total_changes
#define sqlite3_trace                  sqlite3_api->trace
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#endif
#define sqlite3_update_hook            sqlite3_api->update_hook
#define sqlite3_user_data              sqlite3_api->user_data
#define sqlite3_মান_blob             sqlite3_api->মান_blob
#define sqlite3_মান_bytes            sqlite3_api->মান_bytes
#define sqlite3_মান_bytes16          sqlite3_api->মান_bytes16
#define sqlite3_মান_double           sqlite3_api->মান_double
#define sqlite3_মান_int              sqlite3_api->মান_int
#define sqlite3_মান_int64            sqlite3_api->মান_int64
#define sqlite3_মান_numeric_type     sqlite3_api->মান_numeric_type
#define sqlite3_মান_text             sqlite3_api->মান_text
#define sqlite3_মান_text16           sqlite3_api->মান_text16
#define sqlite3_মান_text16be         sqlite3_api->মান_text16be
#define sqlite3_মান_text16le         sqlite3_api->মান_text16le
#define sqlite3_মান_type             sqlite3_api->মান_type
#define sqlite3_vmprintf               sqlite3_api->vmprintf
#define sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#define sqlite3_overload_function      sqlite3_api->overload_function
#define sqlite3_prepare_v2             sqlite3_api->prepare_v2
#define sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#define sqlite3_clear_bindings         sqlite3_api->clear_bindings
#define sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#define sqlite3_blob_bytes             sqlite3_api->blob_bytes
#define sqlite3_blob_close             sqlite3_api->blob_close
#define sqlite3_blob_open              sqlite3_api->blob_open
#define sqlite3_blob_read              sqlite3_api->blob_read
#define sqlite3_blob_write             sqlite3_api->blob_write
#define sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#define sqlite3_file_control           sqlite3_api->file_control
#define sqlite3_memory_highwater       sqlite3_api->memory_highwater
#define sqlite3_memory_used            sqlite3_api->memory_used
#define sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#define sqlite3_mutex_enter            sqlite3_api->mutex_enter
#define sqlite3_mutex_free             sqlite3_api->mutex_free
#define sqlite3_mutex_leave            sqlite3_api->mutex_leave
#define sqlite3_mutex_try              sqlite3_api->mutex_try
#define sqlite3_open_v2                sqlite3_api->open_v2
#define sqlite3_release_memory         sqlite3_api->release_memory
#define sqlite3_result_error_nomem     sqlite3_api->result_error_nomem
#define sqlite3_result_error_toobig    sqlite3_api->result_error_toobig
#define sqlite3_sleep                  sqlite3_api->sleep
#define sqlite3_soft_heap_limit        sqlite3_api->soft_heap_limit
#define sqlite3_vfs_find               sqlite3_api->vfs_find
#define sqlite3_vfs_register           sqlite3_api->vfs_register
#define sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#define sqlite3_threadsafe             sqlite3_api->xthreadsafe
#define sqlite3_result_zeroblob        sqlite3_api->result_zeroblob
#define sqlite3_result_error_code      sqlite3_api->result_error_code
#define sqlite3_test_control           sqlite3_api->test_control
#define sqlite3_randomness             sqlite3_api->randomness
#define sqlite3_context_db_handle      sqlite3_api->context_db_handle
#define sqlite3_extended_result_codes  sqlite3_api->extended_result_codes
#define sqlite3_limit                  sqlite3_api->limit
#define sqlite3_next_stmt              sqlite3_api->next_stmt
#define sqlite3_sql                    sqlite3_api->sql
#define sqlite3_status                 sqlite3_api->status
#define sqlite3_backup_finish          sqlite3_api->backup_finish
#define sqlite3_backup_init            sqlite3_api->backup_init
#define sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#define sqlite3_backup_remaining       sqlite3_api->backup_remaining
#define sqlite3_backup_step            sqlite3_api->backup_step
#define sqlite3_compileoption_get      sqlite3_api->compileoption_get
#define sqlite3_compileoption_used     sqlite3_api->compileoption_used
#define sqlite3_create_function_v2     sqlite3_api->create_function_v2
#define sqlite3_db_config              sqlite3_api->db_config
#define sqlite3_db_mutex               sqlite3_api->db_mutex
#define sqlite3_db_status              sqlite3_api->db_status
#define sqlite3_extended_errcode       sqlite3_api->extended_errcode
#define sqlite3_log                    sqlite3_api->log
#define sqlite3_soft_heap_limit64      sqlite3_api->soft_heap_limit64
#define sqlite3_sourceid               sqlite3_api->sourceid
#define sqlite3_stmt_status            sqlite3_api->stmt_status
#define sqlite3_strnicmp               sqlite3_api->strnicmp
#define sqlite3_unlock_notify          sqlite3_api->unlock_notify
#define sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#define sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#define sqlite3_wal_hook               sqlite3_api->wal_hook
#define sqlite3_blob_reopen            sqlite3_api->blob_reopen
#define sqlite3_vtab_config            sqlite3_api->vtab_config
#define sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#define sqlite3_close_v2               sqlite3_api->close_v2
#define sqlite3_db_filename            sqlite3_api->db_filename
#define sqlite3_db_readonly            sqlite3_api->db_readonly
#define sqlite3_db_release_memory      sqlite3_api->db_release_memory
#define sqlite3_errstr                 sqlite3_api->errstr
#define sqlite3_stmt_busy              sqlite3_api->stmt_busy
#define sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#define sqlite3_stricmp                sqlite3_api->stricmp
#define sqlite3_uri_boolean            sqlite3_api->uri_boolean
#define sqlite3_uri_int64              sqlite3_api->uri_int64
#define sqlite3_uri_parameter          sqlite3_api->uri_parameter
#define sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#define sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#define sqlite3_auto_extension         sqlite3_api->auto_extension
#define sqlite3_bind_blob64            sqlite3_api->bind_blob64
#define sqlite3_bind_text64            sqlite3_api->bind_text64
#define sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#define sqlite3_load_extension         sqlite3_api->load_extension
#define sqlite3_malloc64               sqlite3_api->malloc64
#define sqlite3_msize                  sqlite3_api->msize
#define sqlite3_realloc64              sqlite3_api->realloc64
#define sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#define sqlite3_result_blob64          sqlite3_api->result_blob64
#define sqlite3_result_text64          sqlite3_api->result_text64
#define sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#define sqlite3_মান_dup              sqlite3_api->মান_dup
#define sqlite3_মান_free             sqlite3_api->মান_free
#define sqlite3_result_zeroblob64      sqlite3_api->result_zeroblob64
#define sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define sqlite3_মান_subtype          sqlite3_api->মান_subtype
#define sqlite3_result_subtype         sqlite3_api->result_subtype
/* Version 3.10.0 and later */
#define sqlite3_status64               sqlite3_api->status64
#define sqlite3_strlike                sqlite3_api->strlike
#define sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#define sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#define sqlite3_trace_v2               sqlite3_api->trace_v2
#define sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#define sqlite3_set_last_ঢোকাও_rowid  sqlite3_api->set_last_ঢোকাও_rowid
/* Version 3.20.0 and later */
#define sqlite3_prepare_v3             sqlite3_api->prepare_v3
#define sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#define sqlite3_bind_pointer           sqlite3_api->bind_pointer
#define sqlite3_result_pointer         sqlite3_api->result_pointer
#define sqlite3_মান_pointer          sqlite3_api->মান_pointer
/* Version 3.22.0 and later */
#define sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#define sqlite3_মান_nochange         sqlite3_api->মান_nochange
#define sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#define sqlite3_keyword_গুনতি          sqlite3_api->keyword_গুনতি
#define sqlite3_keyword_name           sqlite3_api->keyword_name
#define sqlite3_keyword_check          sqlite3_api->keyword_check
#define sqlite3_str_new                sqlite3_api->str_new
#define sqlite3_str_finish             sqlite3_api->str_finish
#define sqlite3_str_appendf            sqlite3_api->str_appendf
#define sqlite3_str_vappendf           sqlite3_api->str_vappendf
#define sqlite3_str_append             sqlite3_api->str_append
#define sqlite3_str_appendall          sqlite3_api->str_appendall
#define sqlite3_str_appendchar         sqlite3_api->str_appendchar
#define sqlite3_str_reset              sqlite3_api->str_reset
#define sqlite3_str_errcode            sqlite3_api->str_errcode
#define sqlite3_str_length             sqlite3_api->str_length
#define sqlite3_str_মান              sqlite3_api->str_মান
/* Version 3.25.0 and later */
#define sqlite3_create_window_function sqlite3_api->create_window_function
/* Version 3.26.0 and later */
#define sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#define sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#define sqlite3_মান_frombind         sqlite3_api->মান_frombind
/* Version 3.30.0 and later */
#define sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#define sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#define sqlite3_uri_চাবি                sqlite3_api->uri_চাবি
#define sqlite3_filename_database      sqlite3_api->filename_database
#define sqlite3_filename_journal       sqlite3_api->filename_journal
#define sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#define sqlite3_create_filename        sqlite3_api->create_filename
#define sqlite3_free_filename          sqlite3_api->free_filename
#define sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#define sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#define sqlite3_changes64              sqlite3_api->changes64
#define sqlite3_total_changes64        sqlite3_api->total_changes64
/* Version 3.37.0 and later */
#define sqlite3_autovacuum_pages       sqlite3_api->autovacuum_pages
/* Version 3.38.0 and later */
#define sqlite3_error_offset           sqlite3_api->error_offset
#define sqlite3_vtab_rhs_মান         sqlite3_api->vtab_rhs_মান
#define sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#define sqlite3_vtab_in                sqlite3_api->vtab_in
#define sqlite3_vtab_in_first          sqlite3_api->vtab_in_first
#define sqlite3_vtab_in_next           sqlite3_api->vtab_in_next
/* Version 3.39.0 and later */
#ifndef SQLITE_OMIT_DESERIALIZE
#define sqlite3_deserialize            sqlite3_api->deserialize
#define sqlite3_serialize              sqlite3_api->serialize
#endif
#define sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#define sqlite3_মান_encoding         sqlite3_api->মান_encoding
/* Version 3.41.0 and later */
#define sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#define sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#define sqlite3_get_clientdata         sqlite3_api->get_clientdata
#define sqlite3_set_clientdata         sqlite3_api->set_clientdata
/* Version 3.50.0 and later */
#define sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#define sqlite3_set_errmsg             sqlite3_api->set_errmsg
#define sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#define sqlite3_str_truncate           sqlite3_api->str_truncate
#define sqlite3_str_free               sqlite3_api->str_free
#define sqlite3_carray_bind            sqlite3_api->carray_bind
#define sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#define sqlite3_incomplete             sqlite3_api->incomplete
#define sqlite3_result_str             sqlite3_api->result_str
#endif /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#যদি !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ধ্রুৱ sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    বাহ্য ধ্রুৱ sqlite3_api_routines *sqlite3_api;
#অন্যথা
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (শূন্য)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */
