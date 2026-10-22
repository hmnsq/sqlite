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
#ضمن "sqlite3.h"

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
بنية sqlite3_api_routines {
  فراغ * (*aggregate_context)(sqlite3_context*,صحيح nBytes);
  صحيح  (*aggregate_كدي)(sqlite3_context*);
  صحيح  (*bind_blob)(sqlite3_stmt*,صحيح,ثابت فراغ*,صحيح n,فراغ(*)(فراغ*));
  صحيح  (*bind_double)(sqlite3_stmt*,صحيح,مزدوج);
  صحيح  (*bind_int)(sqlite3_stmt*,صحيح,صحيح);
  صحيح  (*bind_int64)(sqlite3_stmt*,صحيح,sqlite_int64);
  صحيح  (*bind_null)(sqlite3_stmt*,صحيح);
  صحيح  (*bind_parameter_كدي)(sqlite3_stmt*);
  صحيح  (*bind_parameter_index)(sqlite3_stmt*,ثابت حرف*zName);
  ثابت حرف * (*bind_parameter_name)(sqlite3_stmt*,صحيح);
  صحيح  (*bind_text)(sqlite3_stmt*,صحيح,ثابت حرف*,صحيح n,فراغ(*)(فراغ*));
  صحيح  (*bind_text16)(sqlite3_stmt*,صحيح,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  صحيح  (*bind_درج)(sqlite3_stmt*,صحيح,ثابت sqlite3_درج*);
  صحيح  (*busy_handler)(sqlite3*,صحيح(*)(فراغ*,صحيح),فراغ*);
  صحيح  (*busy_timeout)(sqlite3*,صحيح ms);
  صحيح  (*changes)(sqlite3*);
  صحيح  (*close)(sqlite3*);
  صحيح  (*collation_needed)(sqlite3*,فراغ*,فراغ(*)(فراغ*,sqlite3*,
                           صحيح eTextRep,ثابت حرف*));
  صحيح  (*collation_needed16)(sqlite3*,فراغ*,فراغ(*)(فراغ*,sqlite3*,
                             صحيح eTextRep,ثابت فراغ*));
  ثابت فراغ * (*column_blob)(sqlite3_stmt*,صحيح iCol);
  صحيح  (*column_bytes)(sqlite3_stmt*,صحيح iCol);
  صحيح  (*column_bytes16)(sqlite3_stmt*,صحيح iCol);
  صحيح  (*column_كدي)(sqlite3_stmt*pStmt);
  ثابت حرف * (*column_database_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*column_database_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*column_decltype)(sqlite3_stmt*,صحيح i);
  ثابت فراغ * (*column_decltype16)(sqlite3_stmt*,صحيح);
  مزدوج  (*column_double)(sqlite3_stmt*,صحيح iCol);
  صحيح  (*column_int)(sqlite3_stmt*,صحيح iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,صحيح iCol);
  ثابت حرف * (*column_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*column_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*column_origin_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*column_origin_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*column_table_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*column_table_name16)(sqlite3_stmt*,صحيح);
  ثابت unsigned حرف * (*column_text)(sqlite3_stmt*,صحيح iCol);
  ثابت فراغ * (*column_text16)(sqlite3_stmt*,صحيح iCol);
  صحيح  (*column_type)(sqlite3_stmt*,صحيح iCol);
  sqlite3_درج* (*column_درج)(sqlite3_stmt*,صحيح iCol);
  فراغ * (*commit_hook)(sqlite3*,صحيح(*)(فراغ*),فراغ*);
  صحيح  (*complete)(ثابت حرف*sql);
  صحيح  (*complete16)(ثابت فراغ*sql);
  صحيح  (*create_collation)(sqlite3*,ثابت حرف*,صحيح,فراغ*,
                           صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*));
  صحيح  (*create_collation16)(sqlite3*,ثابت فراغ*,صحيح,فراغ*,
                             صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*));
  صحيح  (*create_function)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                          فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_درج**),
                          فراغ (*xStep)(sqlite3_context*,صحيح,sqlite3_درج**),
                          فراغ (*xFinal)(sqlite3_context*));
  صحيح  (*create_function16)(sqlite3*,ثابت فراغ*,صحيح,صحيح,فراغ*,
                            فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_درج**),
                            فراغ (*xStep)(sqlite3_context*,صحيح,sqlite3_درج**),
                            فراغ (*xFinal)(sqlite3_context*));
  صحيح (*create_module)(sqlite3*,ثابت حرف*,ثابت sqlite3_module*,فراغ*);
  صحيح  (*data_كدي)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  صحيح (*declare_vtab)(sqlite3*,ثابت حرف*);
  صحيح  (*enable_shared_cache)(صحيح);
  صحيح  (*errcode)(sqlite3*db);
  ثابت حرف * (*errmsg)(sqlite3*);
  ثابت فراغ * (*errmsg16)(sqlite3*);
  صحيح  (*exec)(sqlite3*,ثابت حرف*,sqlite3_callback,فراغ*,حرف**);
  صحيح  (*expired)(sqlite3_stmt*);
  صحيح  (*finalize)(sqlite3_stmt*pStmt);
  فراغ  (*حرر)(فراغ*);
  فراغ  (*free_table)(حرف**result);
  صحيح  (*get_autocommit)(sqlite3*);
  فراغ * (*get_auxdata)(sqlite3_context*,صحيح);
  صحيح  (*get_table)(sqlite3*,ثابت حرف*,حرف***,صحيح*,صحيح*,حرف**);
  صحيح  (*global_recover)(فراغ);
  فراغ  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_سك_rowid)(sqlite3*);
  ثابت حرف * (*libversion)(فراغ);
  صحيح  (*libversion_لمب)(فراغ);
  فراغ *(*malloc)(صحيح);
  حرف *(SQLITE_CDECL*mprintf)(ثابت حرف*,...);
  صحيح  (*open)(ثابت حرف*,sqlite3**);
  صحيح  (*open16)(ثابت فراغ*,sqlite3**);
  صحيح  (*prepare)(sqlite3*,ثابت حرف*,صحيح,sqlite3_stmt**,ثابت حرف**);
  صحيح  (*prepare16)(sqlite3*,ثابت فراغ*,صحيح,sqlite3_stmt**,ثابت فراغ**);
  فراغ * (*profile)(sqlite3*,فراغ(*)(فراغ*,ثابت حرف*,sqlite_uint64),فراغ*);
  فراغ  (*progress_handler)(sqlite3*,صحيح,صحيح(*)(فراغ*),فراغ*);
  فراغ *(*realloc)(فراغ*,صحيح);
  صحيح  (*reset)(sqlite3_stmt*pStmt);
  فراغ  (*result_blob)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*result_double)(sqlite3_context*,مزدوج);
  فراغ  (*result_error)(sqlite3_context*,ثابت حرف*,صحيح);
  فراغ  (*result_error16)(sqlite3_context*,ثابت فراغ*,صحيح);
  فراغ  (*result_int)(sqlite3_context*,صحيح);
  فراغ  (*result_int64)(sqlite3_context*,sqlite_int64);
  فراغ  (*result_null)(sqlite3_context*);
  فراغ  (*result_text)(sqlite3_context*,ثابت حرف*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*result_text16)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*result_text16be)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*result_text16le)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*result_درج)(sqlite3_context*,sqlite3_درج*);
  فراغ *(*rollback_hook)(sqlite3*,فراغ(*)(فراغ*),فراغ*);
  صحيح  (*set_authorizer)(sqlite3*,صحيح(*)(فراغ*,صحيح,ثابت حرف*,ثابت حرف*,
                         ثابت حرف*,ثابت حرف*),فراغ*);
  فراغ  (*set_auxdata)(sqlite3_context*,صحيح,فراغ*,فراغ (*)(فراغ*));
  حرف *(SQLITE_CDECL*xsnprintf)(صحيح,حرف*,ثابت حرف*,...);
  صحيح  (*step)(sqlite3_stmt*);
  صحيح  (*table_column_metadata)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,
                                حرف ثابت**,حرف ثابت**,صحيح*,صحيح*,صحيح*);
  فراغ  (*thread_cleanup)(فراغ);
  صحيح  (*total_changes)(sqlite3*);
  فراغ * (*trace)(sqlite3*,فراغ(*xTrace)(فراغ*,ثابت حرف*),فراغ*);
  صحيح  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  فراغ * (*update_hook)(sqlite3*,فراغ(*)(فراغ*,صحيح ,حرف ثابت*,حرف ثابت*,
                                         sqlite_int64),فراغ*);
  فراغ * (*user_data)(sqlite3_context*);
  ثابت فراغ * (*درج_blob)(sqlite3_درج*);
  صحيح  (*درج_bytes)(sqlite3_درج*);
  صحيح  (*درج_bytes16)(sqlite3_درج*);
  مزدوج  (*درج_double)(sqlite3_درج*);
  صحيح  (*درج_int)(sqlite3_درج*);
  sqlite_int64  (*درج_int64)(sqlite3_درج*);
  صحيح  (*درج_numeric_type)(sqlite3_درج*);
  ثابت unsigned حرف * (*درج_text)(sqlite3_درج*);
  ثابت فراغ * (*درج_text16)(sqlite3_درج*);
  ثابت فراغ * (*درج_text16be)(sqlite3_درج*);
  ثابت فراغ * (*درج_text16le)(sqlite3_درج*);
  صحيح  (*درج_type)(sqlite3_درج*);
  حرف *(*vmprintf)(ثابت حرف*,va_list);
  /* Added ??? */
  صحيح (*overload_function)(sqlite3*, ثابت حرف *zFuncName, صحيح nArg);
  /* Added by 3.3.13 */
  صحيح (*prepare_v2)(sqlite3*,ثابت حرف*,صحيح,sqlite3_stmt**,ثابت حرف**);
  صحيح (*prepare16_v2)(sqlite3*,ثابت فراغ*,صحيح,sqlite3_stmt**,ثابت فراغ**);
  صحيح (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  صحيح (*create_module_v2)(sqlite3*,ثابت حرف*,ثابت sqlite3_module*,فراغ*,
                          فراغ (*xDestroy)(فراغ *));
  /* Added by 3.5.0 */
  صحيح (*bind_zeroblob)(sqlite3_stmt*,صحيح,صحيح);
  صحيح (*blob_bytes)(sqlite3_blob*);
  صحيح (*blob_close)(sqlite3_blob*);
  صحيح (*blob_open)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,sqlite3_int64,
                   صحيح,sqlite3_blob**);
  صحيح (*blob_read)(sqlite3_blob*,فراغ*,صحيح,صحيح);
  صحيح (*blob_write)(sqlite3_blob*,ثابت فراغ*,صحيح,صحيح);
  صحيح (*create_collation_v2)(sqlite3*,ثابت حرف*,صحيح,فراغ*,
                             صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*),
                             فراغ(*)(فراغ*));
  صحيح (*file_control)(sqlite3*,ثابت حرف*,صحيح,فراغ*);
  sqlite3_int64 (*memory_highwater)(صحيح);
  sqlite3_int64 (*memory_used)(فراغ);
  sqlite3_mutex *(*mutex_alloc)(صحيح);
  فراغ (*mutex_enter)(sqlite3_mutex*);
  فراغ (*mutex_free)(sqlite3_mutex*);
  فراغ (*mutex_leave)(sqlite3_mutex*);
  صحيح (*mutex_try)(sqlite3_mutex*);
  صحيح (*open_v2)(ثابت حرف*,sqlite3**,صحيح,ثابت حرف*);
  صحيح (*release_memory)(صحيح);
  فراغ (*result_error_nomem)(sqlite3_context*);
  فراغ (*result_error_toobig)(sqlite3_context*);
  صحيح (*sleep)(صحيح);
  فراغ (*soft_heap_limit)(صحيح);
  sqlite3_vfs *(*vfs_find)(ثابت حرف*);
  صحيح (*vfs_register)(sqlite3_vfs*,صحيح);
  صحيح (*vfs_unregister)(sqlite3_vfs*);
  صحيح (*xthreadsafe)(فراغ);
  فراغ (*result_zeroblob)(sqlite3_context*,صحيح);
  فراغ (*result_error_code)(sqlite3_context*,صحيح);
  صحيح (SQLITE_CDECL*test_control)(صحيح, ...);
  فراغ (*randomness)(صحيح,فراغ*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  صحيح (*extended_result_codes)(sqlite3*,صحيح);
  صحيح (*limit)(sqlite3*,صحيح,صحيح);
  sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
  ثابت حرف *(*sql)(sqlite3_stmt*);
  صحيح (*status)(صحيح,صحيح*,صحيح*,صحيح);
  صحيح (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ثابت حرف*,sqlite3*,ثابت حرف*);
  صحيح (*backup_pagecount)(sqlite3_backup*);
  صحيح (*backup_remaining)(sqlite3_backup*);
  صحيح (*backup_step)(sqlite3_backup*,صحيح);
  ثابت حرف *(*compileoption_get)(صحيح);
  صحيح (*compileoption_used)(ثابت حرف*);
  صحيح (*create_function_v2)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                            فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_درج**),
                            فراغ (*xStep)(sqlite3_context*,صحيح,sqlite3_درج**),
                            فراغ (*xFinal)(sqlite3_context*),
                            فراغ(*xDestroy)(فراغ*));
  صحيح (SQLITE_CDECL*db_config)(sqlite3*,صحيح,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  صحيح (*db_status)(sqlite3*,صحيح,صحيح*,صحيح*,صحيح);
  صحيح (*extended_errcode)(sqlite3*);
  فراغ (SQLITE_CDECL*log)(صحيح,ثابت حرف*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  ثابت حرف *(*sourceid)(فراغ);
  صحيح (*stmt_status)(sqlite3_stmt*,صحيح,صحيح);
  صحيح (*strnicmp)(ثابت حرف*,ثابت حرف*,صحيح);
  صحيح (*unlock_notify)(sqlite3*,فراغ(*)(فراغ**,صحيح),فراغ*);
  صحيح (*wal_autocheckpoint)(sqlite3*,صحيح);
  صحيح (*wal_checkpoint)(sqlite3*,ثابت حرف*);
  فراغ *(*wal_hook)(sqlite3*,صحيح(*)(فراغ*,sqlite3*,ثابت حرف*,صحيح),فراغ*);
  صحيح (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  صحيح (SQLITE_CDECL*vtab_config)(sqlite3*,صحيح op,...);
  صحيح (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  صحيح (*close_v2)(sqlite3*);
  ثابت حرف *(*db_filename)(sqlite3*,ثابت حرف*);
  صحيح (*db_readonly)(sqlite3*,ثابت حرف*);
  صحيح (*db_release_memory)(sqlite3*);
  ثابت حرف *(*errstr)(صحيح);
  صحيح (*stmt_busy)(sqlite3_stmt*);
  صحيح (*stmt_readonly)(sqlite3_stmt*);
  صحيح (*stricmp)(ثابت حرف*,ثابت حرف*);
  صحيح (*uri_boolean)(ثابت حرف*,ثابت حرف*,صحيح);
  sqlite3_int64 (*uri_int64)(ثابت حرف*,ثابت حرف*,sqlite3_int64);
  ثابت حرف *(*uri_parameter)(ثابت حرف*,ثابت حرف*);
  حرف *(*xvsnprintf)(صحيح,حرف*,ثابت حرف*,va_list);
  صحيح (*wal_checkpoint_v2)(sqlite3*,ثابت حرف*,صحيح,صحيح*,صحيح*);
  /* Version 3.8.7 and later */
  صحيح (*auto_extension)(فراغ(*)(فراغ));
  صحيح (*bind_blob64)(sqlite3_stmt*,صحيح,ثابت فراغ*,sqlite3_uint64,
                     فراغ(*)(فراغ*));
  صحيح (*bind_text64)(sqlite3_stmt*,صحيح,ثابت حرف*,sqlite3_uint64,
                      فراغ(*)(فراغ*),unsigned حرف);
  صحيح (*cancel_auto_extension)(فراغ(*)(فراغ));
  صحيح (*load_extension)(sqlite3*,ثابت حرف*,ثابت حرف*,حرف**);
  فراغ *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(فراغ*);
  فراغ *(*realloc64)(فراغ*,sqlite3_uint64);
  فراغ (*reset_auto_extension)(فراغ);
  فراغ (*result_blob64)(sqlite3_context*,ثابت فراغ*,sqlite3_uint64,
                        فراغ(*)(فراغ*));
  فراغ (*result_text64)(sqlite3_context*,ثابت حرف*,sqlite3_uint64,
                         فراغ(*)(فراغ*), unsigned حرف);
  صحيح (*strglob)(ثابت حرف*,ثابت حرف*);
  /* Version 3.8.11 and later */
  sqlite3_درج *(*درج_dup)(ثابت sqlite3_درج*);
  فراغ (*درج_free)(sqlite3_درج*);
  صحيح (*result_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  صحيح (*bind_zeroblob64)(sqlite3_stmt*, صحيح, sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned صحيح (*درج_subtype)(sqlite3_درج*);
  فراغ (*result_subtype)(sqlite3_context*,unsigned صحيح);
  /* Version 3.10.0 and later */
  صحيح (*status64)(صحيح,sqlite3_int64*,sqlite3_int64*,صحيح);
  صحيح (*strlike)(ثابت حرف*,ثابت حرف*,unsigned صحيح);
  صحيح (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  صحيح (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  صحيح (*trace_v2)(sqlite3*,unsigned,صحيح(*)(unsigned,فراغ*,فراغ*,فراغ*),فراغ*);
  حرف *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  فراغ (*set_last_سك_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  صحيح (*prepare_v3)(sqlite3*,ثابت حرف*,صحيح,unsigned صحيح,
                    sqlite3_stmt**,ثابت حرف**);
  صحيح (*prepare16_v3)(sqlite3*,ثابت فراغ*,صحيح,unsigned صحيح,
                      sqlite3_stmt**,ثابت فراغ**);
  صحيح (*bind_pointer)(sqlite3_stmt*,صحيح,فراغ*,ثابت حرف*,فراغ(*)(فراغ*));
  فراغ (*result_pointer)(sqlite3_context*,فراغ*,ثابت حرف*,فراغ(*)(فراغ*));
  فراغ *(*درج_pointer)(sqlite3_درج*,ثابت حرف*);
  صحيح (*vtab_nochange)(sqlite3_context*);
  صحيح (*درج_nochange)(sqlite3_درج*);
  ثابت حرف *(*vtab_collation)(sqlite3_index_info*,صحيح);
  /* Version 3.24.0 and later */
  صحيح (*keyword_كدي)(فراغ);
  صحيح (*keyword_name)(صحيح,ثابت حرف**,صحيح*);
  صحيح (*keyword_check)(ثابت حرف*,صحيح);
  sqlite3_str *(*str_new)(sqlite3*);
  حرف *(*str_finish)(sqlite3_str*);
  فراغ (SQLITE_CDECL*str_appendf)(sqlite3_str*, ثابت حرف *zFormat, ...);
  فراغ (*str_vappendf)(sqlite3_str*, ثابت حرف *zFormat, va_list);
  فراغ (*str_append)(sqlite3_str*, ثابت حرف *zIn, صحيح N);
  فراغ (*str_appendall)(sqlite3_str*, ثابت حرف *zIn);
  فراغ (*str_appendchar)(sqlite3_str*, صحيح N, حرف C);
  فراغ (*str_reset)(sqlite3_str*);
  صحيح (*str_errcode)(sqlite3_str*);
  صحيح (*str_length)(sqlite3_str*);
  حرف *(*str_درج)(sqlite3_str*);
  /* Version 3.25.0 and later */
  صحيح (*create_window_function)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                            فراغ (*xStep)(sqlite3_context*,صحيح,sqlite3_درج**),
                            فراغ (*xFinal)(sqlite3_context*),
                            فراغ (*x_درج)(sqlite3_context*),
                            فراغ (*xInv)(sqlite3_context*,صحيح,sqlite3_درج**),
                            فراغ(*xDestroy)(فراغ*));
  /* Version 3.26.0 and later */
  ثابت حرف *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  صحيح (*stmt_isexplain)(sqlite3_stmt*);
  صحيح (*درج_frombind)(sqlite3_درج*);
  /* Version 3.30.0 and later */
  صحيح (*drop_modules)(sqlite3*,ثابت حرف**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  ثابت حرف *(*uri_مبودي)(ثابت حرف*,صحيح);
  ثابت حرف *(*filename_database)(ثابت حرف*);
  ثابت حرف *(*filename_journal)(ثابت حرف*);
  ثابت حرف *(*filename_wal)(ثابت حرف*);
  /* Version 3.32.0 and later */
  ثابت حرف *(*create_filename)(ثابت حرف*,ثابت حرف*,ثابت حرف*,
                           صحيح,ثابت حرف**);
  فراغ (*free_filename)(ثابت حرف*);
  sqlite3_file *(*database_file_object)(ثابت حرف*);
  /* Version 3.34.0 and later */
  صحيح (*txn_state)(sqlite3*,ثابت حرف*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*total_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  صحيح (*autovacuum_pages)(sqlite3*,
     unsigned صحيح(*)(فراغ*,ثابت حرف*,unsigned صحيح,unsigned صحيح,unsigned صحيح),
     فراغ*, فراغ(*)(فراغ*));
  /* Version 3.38.0 and later */
  صحيح (*error_offset)(sqlite3*);
  صحيح (*vtab_rhs_درج)(sqlite3_index_info*,صحيح,sqlite3_درج**);
  صحيح (*vtab_distinct)(sqlite3_index_info*);
  صحيح (*vtab_in)(sqlite3_index_info*,صحيح,صحيح);
  صحيح (*vtab_in_first)(sqlite3_درج*,sqlite3_درج**);
  صحيح (*vtab_in_next)(sqlite3_درج*,sqlite3_درج**);
  /* Version 3.39.0 and later */
  صحيح (*deserialize)(sqlite3*,ثابت حرف*,unsigned حرف*,
                     sqlite3_int64,sqlite3_int64,unsigned);
  unsigned حرف *(*serialize)(sqlite3*,ثابت حرف *,sqlite3_int64*,
                              unsigned صحيح);
  ثابت حرف *(*db_name)(sqlite3*,صحيح);
  /* Version 3.40.0 and later */
  صحيح (*درج_encoding)(sqlite3_درج*);
  /* Version 3.41.0 and later */
  صحيح (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  صحيح (*stmt_explain)(sqlite3_stmt*,صحيح);
  /* Version 3.44.0 and later */
  فراغ *(*get_clientdata)(sqlite3*,ثابت حرف*);
  صحيح (*set_clientdata)(sqlite3*, ثابت حرف*, فراغ*, فراغ(*)(فراغ*));
  /* Version 3.50.0 and later */
  صحيح (*setlk_timeout)(sqlite3*,صحيح,صحيح);
  /* Version 3.51.0 and later */
  صحيح (*set_errmsg)(sqlite3*,صحيح,ثابت حرف*);
  صحيح (*db_status64)(sqlite3*,صحيح,sqlite3_int64*,sqlite3_int64*,صحيح);
  /* Version 3.52.0 and later */
  فراغ (*str_truncate)(sqlite3_str*,صحيح);
  فراغ (*str_free)(sqlite3_str*);
  صحيح (*carray_bind)(sqlite3_stmt*,صحيح,فراغ*,صحيح,صحيح,فراغ(*)(فراغ*));
  صحيح (*carray_bind_v2)(sqlite3_stmt*,صحيح,فراغ*,صحيح,صحيح,فراغ(*)(فراغ*),فراغ*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ثابت حرف*);
  فراغ (*result_str)(sqlite3_context*,sqlite3_str*,صحيح);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
تعريفنوع صحيح (*sqlite3_loadext_entry)(
  sqlite3 *db,                       /* Handle to the database. */
  حرف **pz_Err_سكو,                   /* Used to set error string on failure. */
  ثابت sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#إذا !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_aggregate_كدي        sqlite3_api->aggregate_كدي
#endif
#define sqlite3_bind_blob              sqlite3_api->bind_blob
#define sqlite3_bind_double            sqlite3_api->bind_double
#define sqlite3_bind_int               sqlite3_api->bind_int
#define sqlite3_bind_int64             sqlite3_api->bind_int64
#define sqlite3_bind_null              sqlite3_api->bind_null
#define sqlite3_bind_parameter_كدي   sqlite3_api->bind_parameter_كدي
#define sqlite3_bind_parameter_index   sqlite3_api->bind_parameter_index
#define sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#define sqlite3_bind_text              sqlite3_api->bind_text
#define sqlite3_bind_text16            sqlite3_api->bind_text16
#define sqlite3_bind_درج             sqlite3_api->bind_درج
#define sqlite3_busy_handler           sqlite3_api->busy_handler
#define sqlite3_busy_timeout           sqlite3_api->busy_timeout
#define sqlite3_changes                sqlite3_api->changes
#define sqlite3_close                  sqlite3_api->close
#define sqlite3_collation_needed       sqlite3_api->collation_needed
#define sqlite3_collation_needed16     sqlite3_api->collation_needed16
#define sqlite3_column_blob            sqlite3_api->column_blob
#define sqlite3_column_bytes           sqlite3_api->column_bytes
#define sqlite3_column_bytes16         sqlite3_api->column_bytes16
#define sqlite3_column_كدي           sqlite3_api->column_كدي
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
#define sqlite3_column_درج           sqlite3_api->column_درج
#define sqlite3_commit_hook            sqlite3_api->commit_hook
#define sqlite3_complete               sqlite3_api->complete
#define sqlite3_complete16             sqlite3_api->complete16
#define sqlite3_create_collation       sqlite3_api->create_collation
#define sqlite3_create_collation16     sqlite3_api->create_collation16
#define sqlite3_create_function        sqlite3_api->create_function
#define sqlite3_create_function16      sqlite3_api->create_function16
#define sqlite3_create_module          sqlite3_api->create_module
#define sqlite3_create_module_v2       sqlite3_api->create_module_v2
#define sqlite3_data_كدي             sqlite3_api->data_كدي
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
#define sqlite3_free                   sqlite3_api->حرر
#define sqlite3_free_table             sqlite3_api->free_table
#define sqlite3_get_autocommit         sqlite3_api->get_autocommit
#define sqlite3_get_auxdata            sqlite3_api->get_auxdata
#define sqlite3_get_table              sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_global_recover         sqlite3_api->global_recover
#endif
#define sqlite3_interrupt              sqlite3_api->interruptx
#define sqlite3_last_سك_rowid      sqlite3_api->last_سك_rowid
#define sqlite3_libversion             sqlite3_api->libversion
#define sqlite3_libversion_لمب      sqlite3_api->libversion_لمب
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
#define sqlite3_result_درج           sqlite3_api->result_درج
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
#define sqlite3_درج_blob             sqlite3_api->درج_blob
#define sqlite3_درج_bytes            sqlite3_api->درج_bytes
#define sqlite3_درج_bytes16          sqlite3_api->درج_bytes16
#define sqlite3_درج_double           sqlite3_api->درج_double
#define sqlite3_درج_int              sqlite3_api->درج_int
#define sqlite3_درج_int64            sqlite3_api->درج_int64
#define sqlite3_درج_numeric_type     sqlite3_api->درج_numeric_type
#define sqlite3_درج_text             sqlite3_api->درج_text
#define sqlite3_درج_text16           sqlite3_api->درج_text16
#define sqlite3_درج_text16be         sqlite3_api->درج_text16be
#define sqlite3_درج_text16le         sqlite3_api->درج_text16le
#define sqlite3_درج_type             sqlite3_api->درج_type
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
#define sqlite3_درج_dup              sqlite3_api->درج_dup
#define sqlite3_درج_free             sqlite3_api->درج_free
#define sqlite3_result_zeroblob64      sqlite3_api->result_zeroblob64
#define sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define sqlite3_درج_subtype          sqlite3_api->درج_subtype
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
#define sqlite3_set_last_سك_rowid  sqlite3_api->set_last_سك_rowid
/* Version 3.20.0 and later */
#define sqlite3_prepare_v3             sqlite3_api->prepare_v3
#define sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#define sqlite3_bind_pointer           sqlite3_api->bind_pointer
#define sqlite3_result_pointer         sqlite3_api->result_pointer
#define sqlite3_درج_pointer          sqlite3_api->درج_pointer
/* Version 3.22.0 and later */
#define sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#define sqlite3_درج_nochange         sqlite3_api->درج_nochange
#define sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#define sqlite3_keyword_كدي          sqlite3_api->keyword_كدي
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
#define sqlite3_str_درج              sqlite3_api->str_درج
/* Version 3.25.0 and later */
#define sqlite3_create_window_function sqlite3_api->create_window_function
/* Version 3.26.0 and later */
#define sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#define sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#define sqlite3_درج_frombind         sqlite3_api->درج_frombind
/* Version 3.30.0 and later */
#define sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#define sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#define sqlite3_uri_مبودي                sqlite3_api->uri_مبودي
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
#define sqlite3_vtab_rhs_درج         sqlite3_api->vtab_rhs_درج
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
#define sqlite3_درج_encoding         sqlite3_api->درج_encoding
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

#إذا !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ثابت sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    خارجي ثابت sqlite3_api_routines *sqlite3_api;
#وإلا
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (فراغ)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */
