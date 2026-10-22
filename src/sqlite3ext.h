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
#شامل "sqlite3.h"

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
ساختار sqlite3_api_routines {
  تهی * (*aggregate_context)(sqlite3_context*,صحیح nBytes);
  صحیح  (*aggregate_شمار)(sqlite3_context*);
  صحیح  (*bind_blob)(sqlite3_stmt*,صحیح,ثابت تهی*,صحیح n,تهی(*)(تهی*));
  صحیح  (*bind_double)(sqlite3_stmt*,صحیح,دوگانه);
  صحیح  (*bind_int)(sqlite3_stmt*,صحیح,صحیح);
  صحیح  (*bind_int64)(sqlite3_stmt*,صحیح,sqlite_int64);
  صحیح  (*bind_null)(sqlite3_stmt*,صحیح);
  صحیح  (*bind_parameter_شمار)(sqlite3_stmt*);
  صحیح  (*bind_parameter_index)(sqlite3_stmt*,ثابت نویسه*zName);
  ثابت نویسه * (*bind_parameter_name)(sqlite3_stmt*,صحیح);
  صحیح  (*bind_text)(sqlite3_stmt*,صحیح,ثابت نویسه*,صحیح n,تهی(*)(تهی*));
  صحیح  (*bind_text16)(sqlite3_stmt*,صحیح,ثابت تهی*,صحیح,تهی(*)(تهی*));
  صحیح  (*bind_مقدار)(sqlite3_stmt*,صحیح,ثابت sqlite3_مقدار*);
  صحیح  (*busy_handler)(sqlite3*,صحیح(*)(تهی*,صحیح),تهی*);
  صحیح  (*busy_timeout)(sqlite3*,صحیح ms);
  صحیح  (*changes)(sqlite3*);
  صحیح  (*close)(sqlite3*);
  صحیح  (*collation_needed)(sqlite3*,تهی*,تهی(*)(تهی*,sqlite3*,
                           صحیح eTextRep,ثابت نویسه*));
  صحیح  (*collation_needed16)(sqlite3*,تهی*,تهی(*)(تهی*,sqlite3*,
                             صحیح eTextRep,ثابت تهی*));
  ثابت تهی * (*column_blob)(sqlite3_stmt*,صحیح iCol);
  صحیح  (*column_bytes)(sqlite3_stmt*,صحیح iCol);
  صحیح  (*column_bytes16)(sqlite3_stmt*,صحیح iCol);
  صحیح  (*column_شمار)(sqlite3_stmt*pStmt);
  ثابت نویسه * (*column_database_name)(sqlite3_stmt*,صحیح);
  ثابت تهی * (*column_database_name16)(sqlite3_stmt*,صحیح);
  ثابت نویسه * (*column_decltype)(sqlite3_stmt*,صحیح i);
  ثابت تهی * (*column_decltype16)(sqlite3_stmt*,صحیح);
  دوگانه  (*column_double)(sqlite3_stmt*,صحیح iCol);
  صحیح  (*column_int)(sqlite3_stmt*,صحیح iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,صحیح iCol);
  ثابت نویسه * (*column_name)(sqlite3_stmt*,صحیح);
  ثابت تهی * (*column_name16)(sqlite3_stmt*,صحیح);
  ثابت نویسه * (*column_origin_name)(sqlite3_stmt*,صحیح);
  ثابت تهی * (*column_origin_name16)(sqlite3_stmt*,صحیح);
  ثابت نویسه * (*column_table_name)(sqlite3_stmt*,صحیح);
  ثابت تهی * (*column_table_name16)(sqlite3_stmt*,صحیح);
  ثابت unsigned نویسه * (*column_text)(sqlite3_stmt*,صحیح iCol);
  ثابت تهی * (*column_text16)(sqlite3_stmt*,صحیح iCol);
  صحیح  (*column_type)(sqlite3_stmt*,صحیح iCol);
  sqlite3_مقدار* (*column_مقدار)(sqlite3_stmt*,صحیح iCol);
  تهی * (*commit_hook)(sqlite3*,صحیح(*)(تهی*),تهی*);
  صحیح  (*complete)(ثابت نویسه*sql);
  صحیح  (*complete16)(ثابت تهی*sql);
  صحیح  (*create_collation)(sqlite3*,ثابت نویسه*,صحیح,تهی*,
                           صحیح(*)(تهی*,صحیح,ثابت تهی*,صحیح,ثابت تهی*));
  صحیح  (*create_collation16)(sqlite3*,ثابت تهی*,صحیح,تهی*,
                             صحیح(*)(تهی*,صحیح,ثابت تهی*,صحیح,ثابت تهی*));
  صحیح  (*create_function)(sqlite3*,ثابت نویسه*,صحیح,صحیح,تهی*,
                          تهی (*xFunc)(sqlite3_context*,صحیح,sqlite3_مقدار**),
                          تهی (*xStep)(sqlite3_context*,صحیح,sqlite3_مقدار**),
                          تهی (*xFinal)(sqlite3_context*));
  صحیح  (*create_function16)(sqlite3*,ثابت تهی*,صحیح,صحیح,تهی*,
                            تهی (*xFunc)(sqlite3_context*,صحیح,sqlite3_مقدار**),
                            تهی (*xStep)(sqlite3_context*,صحیح,sqlite3_مقدار**),
                            تهی (*xFinal)(sqlite3_context*));
  صحیح (*create_module)(sqlite3*,ثابت نویسه*,ثابت sqlite3_module*,تهی*);
  صحیح  (*data_شمار)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  صحیح (*declare_vtab)(sqlite3*,ثابت نویسه*);
  صحیح  (*enable_shared_cache)(صحیح);
  صحیح  (*errcode)(sqlite3*db);
  ثابت نویسه * (*errmsg)(sqlite3*);
  ثابت تهی * (*errmsg16)(sqlite3*);
  صحیح  (*exec)(sqlite3*,ثابت نویسه*,sqlite3_callback,تهی*,نویسه**);
  صحیح  (*expired)(sqlite3_stmt*);
  صحیح  (*finalize)(sqlite3_stmt*pStmt);
  تهی  (*آزاد)(تهی*);
  تهی  (*free_table)(نویسه**result);
  صحیح  (*get_autocommit)(sqlite3*);
  تهی * (*get_auxdata)(sqlite3_context*,صحیح);
  صحیح  (*get_table)(sqlite3*,ثابت نویسه*,نویسه***,صحیح*,صحیح*,نویسه**);
  صحیح  (*global_recover)(تهی);
  تهی  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_درج_rowid)(sqlite3*);
  ثابت نویسه * (*libversion)(تهی);
  صحیح  (*libversion_عدد)(تهی);
  تهی *(*malloc)(صحیح);
  نویسه *(SQLITE_CDECL*mprintf)(ثابت نویسه*,...);
  صحیح  (*open)(ثابت نویسه*,sqlite3**);
  صحیح  (*open16)(ثابت تهی*,sqlite3**);
  صحیح  (*prepare)(sqlite3*,ثابت نویسه*,صحیح,sqlite3_stmt**,ثابت نویسه**);
  صحیح  (*prepare16)(sqlite3*,ثابت تهی*,صحیح,sqlite3_stmt**,ثابت تهی**);
  تهی * (*profile)(sqlite3*,تهی(*)(تهی*,ثابت نویسه*,sqlite_uint64),تهی*);
  تهی  (*progress_handler)(sqlite3*,صحیح,صحیح(*)(تهی*),تهی*);
  تهی *(*realloc)(تهی*,صحیح);
  صحیح  (*reset)(sqlite3_stmt*pStmt);
  تهی  (*result_blob)(sqlite3_context*,ثابت تهی*,صحیح,تهی(*)(تهی*));
  تهی  (*result_double)(sqlite3_context*,دوگانه);
  تهی  (*result_error)(sqlite3_context*,ثابت نویسه*,صحیح);
  تهی  (*result_error16)(sqlite3_context*,ثابت تهی*,صحیح);
  تهی  (*result_int)(sqlite3_context*,صحیح);
  تهی  (*result_int64)(sqlite3_context*,sqlite_int64);
  تهی  (*result_null)(sqlite3_context*);
  تهی  (*result_text)(sqlite3_context*,ثابت نویسه*,صحیح,تهی(*)(تهی*));
  تهی  (*result_text16)(sqlite3_context*,ثابت تهی*,صحیح,تهی(*)(تهی*));
  تهی  (*result_text16be)(sqlite3_context*,ثابت تهی*,صحیح,تهی(*)(تهی*));
  تهی  (*result_text16le)(sqlite3_context*,ثابت تهی*,صحیح,تهی(*)(تهی*));
  تهی  (*result_مقدار)(sqlite3_context*,sqlite3_مقدار*);
  تهی *(*rollback_hook)(sqlite3*,تهی(*)(تهی*),تهی*);
  صحیح  (*set_authorizer)(sqlite3*,صحیح(*)(تهی*,صحیح,ثابت نویسه*,ثابت نویسه*,
                         ثابت نویسه*,ثابت نویسه*),تهی*);
  تهی  (*set_auxdata)(sqlite3_context*,صحیح,تهی*,تهی (*)(تهی*));
  نویسه *(SQLITE_CDECL*xsnprintf)(صحیح,نویسه*,ثابت نویسه*,...);
  صحیح  (*step)(sqlite3_stmt*);
  صحیح  (*table_column_metadata)(sqlite3*,ثابت نویسه*,ثابت نویسه*,ثابت نویسه*,
                                نویسه ثابت**,نویسه ثابت**,صحیح*,صحیح*,صحیح*);
  تهی  (*thread_cleanup)(تهی);
  صحیح  (*total_changes)(sqlite3*);
  تهی * (*trace)(sqlite3*,تهی(*xTrace)(تهی*,ثابت نویسه*),تهی*);
  صحیح  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  تهی * (*update_hook)(sqlite3*,تهی(*)(تهی*,صحیح ,نویسه ثابت*,نویسه ثابت*,
                                         sqlite_int64),تهی*);
  تهی * (*user_data)(sqlite3_context*);
  ثابت تهی * (*مقدار_blob)(sqlite3_مقدار*);
  صحیح  (*مقدار_bytes)(sqlite3_مقدار*);
  صحیح  (*مقدار_bytes16)(sqlite3_مقدار*);
  دوگانه  (*مقدار_double)(sqlite3_مقدار*);
  صحیح  (*مقدار_int)(sqlite3_مقدار*);
  sqlite_int64  (*مقدار_int64)(sqlite3_مقدار*);
  صحیح  (*مقدار_numeric_type)(sqlite3_مقدار*);
  ثابت unsigned نویسه * (*مقدار_text)(sqlite3_مقدار*);
  ثابت تهی * (*مقدار_text16)(sqlite3_مقدار*);
  ثابت تهی * (*مقدار_text16be)(sqlite3_مقدار*);
  ثابت تهی * (*مقدار_text16le)(sqlite3_مقدار*);
  صحیح  (*مقدار_type)(sqlite3_مقدار*);
  نویسه *(*vmprintf)(ثابت نویسه*,va_list);
  /* Added ??? */
  صحیح (*overload_function)(sqlite3*, ثابت نویسه *zFuncName, صحیح nArg);
  /* Added by 3.3.13 */
  صحیح (*prepare_v2)(sqlite3*,ثابت نویسه*,صحیح,sqlite3_stmt**,ثابت نویسه**);
  صحیح (*prepare16_v2)(sqlite3*,ثابت تهی*,صحیح,sqlite3_stmt**,ثابت تهی**);
  صحیح (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  صحیح (*create_module_v2)(sqlite3*,ثابت نویسه*,ثابت sqlite3_module*,تهی*,
                          تهی (*xDestroy)(تهی *));
  /* Added by 3.5.0 */
  صحیح (*bind_zeroblob)(sqlite3_stmt*,صحیح,صحیح);
  صحیح (*blob_bytes)(sqlite3_blob*);
  صحیح (*blob_close)(sqlite3_blob*);
  صحیح (*blob_open)(sqlite3*,ثابت نویسه*,ثابت نویسه*,ثابت نویسه*,sqlite3_int64,
                   صحیح,sqlite3_blob**);
  صحیح (*blob_read)(sqlite3_blob*,تهی*,صحیح,صحیح);
  صحیح (*blob_write)(sqlite3_blob*,ثابت تهی*,صحیح,صحیح);
  صحیح (*create_collation_v2)(sqlite3*,ثابت نویسه*,صحیح,تهی*,
                             صحیح(*)(تهی*,صحیح,ثابت تهی*,صحیح,ثابت تهی*),
                             تهی(*)(تهی*));
  صحیح (*file_control)(sqlite3*,ثابت نویسه*,صحیح,تهی*);
  sqlite3_int64 (*memory_highwater)(صحیح);
  sqlite3_int64 (*memory_used)(تهی);
  sqlite3_mutex *(*mutex_alloc)(صحیح);
  تهی (*mutex_enter)(sqlite3_mutex*);
  تهی (*mutex_free)(sqlite3_mutex*);
  تهی (*mutex_leave)(sqlite3_mutex*);
  صحیح (*mutex_try)(sqlite3_mutex*);
  صحیح (*open_v2)(ثابت نویسه*,sqlite3**,صحیح,ثابت نویسه*);
  صحیح (*release_memory)(صحیح);
  تهی (*result_error_nomem)(sqlite3_context*);
  تهی (*result_error_toobig)(sqlite3_context*);
  صحیح (*sleep)(صحیح);
  تهی (*soft_heap_limit)(صحیح);
  sqlite3_vfs *(*vfs_find)(ثابت نویسه*);
  صحیح (*vfs_register)(sqlite3_vfs*,صحیح);
  صحیح (*vfs_unregister)(sqlite3_vfs*);
  صحیح (*xthreadsafe)(تهی);
  تهی (*result_zeroblob)(sqlite3_context*,صحیح);
  تهی (*result_error_code)(sqlite3_context*,صحیح);
  صحیح (SQLITE_CDECL*test_control)(صحیح, ...);
  تهی (*randomness)(صحیح,تهی*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  صحیح (*extended_result_codes)(sqlite3*,صحیح);
  صحیح (*limit)(sqlite3*,صحیح,صحیح);
  sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
  ثابت نویسه *(*sql)(sqlite3_stmt*);
  صحیح (*status)(صحیح,صحیح*,صحیح*,صحیح);
  صحیح (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ثابت نویسه*,sqlite3*,ثابت نویسه*);
  صحیح (*backup_pagecount)(sqlite3_backup*);
  صحیح (*backup_remaining)(sqlite3_backup*);
  صحیح (*backup_step)(sqlite3_backup*,صحیح);
  ثابت نویسه *(*compileoption_get)(صحیح);
  صحیح (*compileoption_used)(ثابت نویسه*);
  صحیح (*create_function_v2)(sqlite3*,ثابت نویسه*,صحیح,صحیح,تهی*,
                            تهی (*xFunc)(sqlite3_context*,صحیح,sqlite3_مقدار**),
                            تهی (*xStep)(sqlite3_context*,صحیح,sqlite3_مقدار**),
                            تهی (*xFinal)(sqlite3_context*),
                            تهی(*xDestroy)(تهی*));
  صحیح (SQLITE_CDECL*db_config)(sqlite3*,صحیح,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  صحیح (*db_status)(sqlite3*,صحیح,صحیح*,صحیح*,صحیح);
  صحیح (*extended_errcode)(sqlite3*);
  تهی (SQLITE_CDECL*log)(صحیح,ثابت نویسه*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  ثابت نویسه *(*sourceid)(تهی);
  صحیح (*stmt_status)(sqlite3_stmt*,صحیح,صحیح);
  صحیح (*strnicmp)(ثابت نویسه*,ثابت نویسه*,صحیح);
  صحیح (*unlock_notify)(sqlite3*,تهی(*)(تهی**,صحیح),تهی*);
  صحیح (*wal_autocheckpoint)(sqlite3*,صحیح);
  صحیح (*wal_checkpoint)(sqlite3*,ثابت نویسه*);
  تهی *(*wal_hook)(sqlite3*,صحیح(*)(تهی*,sqlite3*,ثابت نویسه*,صحیح),تهی*);
  صحیح (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  صحیح (SQLITE_CDECL*vtab_config)(sqlite3*,صحیح op,...);
  صحیح (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  صحیح (*close_v2)(sqlite3*);
  ثابت نویسه *(*db_filename)(sqlite3*,ثابت نویسه*);
  صحیح (*db_readonly)(sqlite3*,ثابت نویسه*);
  صحیح (*db_release_memory)(sqlite3*);
  ثابت نویسه *(*errstr)(صحیح);
  صحیح (*stmt_busy)(sqlite3_stmt*);
  صحیح (*stmt_readonly)(sqlite3_stmt*);
  صحیح (*stricmp)(ثابت نویسه*,ثابت نویسه*);
  صحیح (*uri_boolean)(ثابت نویسه*,ثابت نویسه*,صحیح);
  sqlite3_int64 (*uri_int64)(ثابت نویسه*,ثابت نویسه*,sqlite3_int64);
  ثابت نویسه *(*uri_parameter)(ثابت نویسه*,ثابت نویسه*);
  نویسه *(*xvsnprintf)(صحیح,نویسه*,ثابت نویسه*,va_list);
  صحیح (*wal_checkpoint_v2)(sqlite3*,ثابت نویسه*,صحیح,صحیح*,صحیح*);
  /* Version 3.8.7 and later */
  صحیح (*auto_extension)(تهی(*)(تهی));
  صحیح (*bind_blob64)(sqlite3_stmt*,صحیح,ثابت تهی*,sqlite3_uint64,
                     تهی(*)(تهی*));
  صحیح (*bind_text64)(sqlite3_stmt*,صحیح,ثابت نویسه*,sqlite3_uint64,
                      تهی(*)(تهی*),unsigned نویسه);
  صحیح (*cancel_auto_extension)(تهی(*)(تهی));
  صحیح (*load_extension)(sqlite3*,ثابت نویسه*,ثابت نویسه*,نویسه**);
  تهی *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(تهی*);
  تهی *(*realloc64)(تهی*,sqlite3_uint64);
  تهی (*reset_auto_extension)(تهی);
  تهی (*result_blob64)(sqlite3_context*,ثابت تهی*,sqlite3_uint64,
                        تهی(*)(تهی*));
  تهی (*result_text64)(sqlite3_context*,ثابت نویسه*,sqlite3_uint64,
                         تهی(*)(تهی*), unsigned نویسه);
  صحیح (*strglob)(ثابت نویسه*,ثابت نویسه*);
  /* Version 3.8.11 and later */
  sqlite3_مقدار *(*مقدار_dup)(ثابت sqlite3_مقدار*);
  تهی (*مقدار_free)(sqlite3_مقدار*);
  صحیح (*result_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  صحیح (*bind_zeroblob64)(sqlite3_stmt*, صحیح, sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned صحیح (*مقدار_subtype)(sqlite3_مقدار*);
  تهی (*result_subtype)(sqlite3_context*,unsigned صحیح);
  /* Version 3.10.0 and later */
  صحیح (*status64)(صحیح,sqlite3_int64*,sqlite3_int64*,صحیح);
  صحیح (*strlike)(ثابت نویسه*,ثابت نویسه*,unsigned صحیح);
  صحیح (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  صحیح (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  صحیح (*trace_v2)(sqlite3*,unsigned,صحیح(*)(unsigned,تهی*,تهی*,تهی*),تهی*);
  نویسه *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  تهی (*set_last_درج_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  صحیح (*prepare_v3)(sqlite3*,ثابت نویسه*,صحیح,unsigned صحیح,
                    sqlite3_stmt**,ثابت نویسه**);
  صحیح (*prepare16_v3)(sqlite3*,ثابت تهی*,صحیح,unsigned صحیح,
                      sqlite3_stmt**,ثابت تهی**);
  صحیح (*bind_pointer)(sqlite3_stmt*,صحیح,تهی*,ثابت نویسه*,تهی(*)(تهی*));
  تهی (*result_pointer)(sqlite3_context*,تهی*,ثابت نویسه*,تهی(*)(تهی*));
  تهی *(*مقدار_pointer)(sqlite3_مقدار*,ثابت نویسه*);
  صحیح (*vtab_nochange)(sqlite3_context*);
  صحیح (*مقدار_nochange)(sqlite3_مقدار*);
  ثابت نویسه *(*vtab_collation)(sqlite3_index_info*,صحیح);
  /* Version 3.24.0 and later */
  صحیح (*keyword_شمار)(تهی);
  صحیح (*keyword_name)(صحیح,ثابت نویسه**,صحیح*);
  صحیح (*keyword_check)(ثابت نویسه*,صحیح);
  sqlite3_str *(*str_new)(sqlite3*);
  نویسه *(*str_finish)(sqlite3_str*);
  تهی (SQLITE_CDECL*str_appendf)(sqlite3_str*, ثابت نویسه *zFormat, ...);
  تهی (*str_vappendf)(sqlite3_str*, ثابت نویسه *zFormat, va_list);
  تهی (*str_append)(sqlite3_str*, ثابت نویسه *zIn, صحیح N);
  تهی (*str_appendall)(sqlite3_str*, ثابت نویسه *zIn);
  تهی (*str_appendchar)(sqlite3_str*, صحیح N, نویسه C);
  تهی (*str_reset)(sqlite3_str*);
  صحیح (*str_errcode)(sqlite3_str*);
  صحیح (*str_length)(sqlite3_str*);
  نویسه *(*str_مقدار)(sqlite3_str*);
  /* Version 3.25.0 and later */
  صحیح (*create_window_function)(sqlite3*,ثابت نویسه*,صحیح,صحیح,تهی*,
                            تهی (*xStep)(sqlite3_context*,صحیح,sqlite3_مقدار**),
                            تهی (*xFinal)(sqlite3_context*),
                            تهی (*x_مقدار)(sqlite3_context*),
                            تهی (*xInv)(sqlite3_context*,صحیح,sqlite3_مقدار**),
                            تهی(*xDestroy)(تهی*));
  /* Version 3.26.0 and later */
  ثابت نویسه *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  صحیح (*stmt_isexplain)(sqlite3_stmt*);
  صحیح (*مقدار_frombind)(sqlite3_مقدار*);
  /* Version 3.30.0 and later */
  صحیح (*drop_modules)(sqlite3*,ثابت نویسه**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  ثابت نویسه *(*uri_کلید)(ثابت نویسه*,صحیح);
  ثابت نویسه *(*filename_database)(ثابت نویسه*);
  ثابت نویسه *(*filename_journal)(ثابت نویسه*);
  ثابت نویسه *(*filename_wal)(ثابت نویسه*);
  /* Version 3.32.0 and later */
  ثابت نویسه *(*create_filename)(ثابت نویسه*,ثابت نویسه*,ثابت نویسه*,
                           صحیح,ثابت نویسه**);
  تهی (*free_filename)(ثابت نویسه*);
  sqlite3_file *(*database_file_object)(ثابت نویسه*);
  /* Version 3.34.0 and later */
  صحیح (*txn_state)(sqlite3*,ثابت نویسه*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*total_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  صحیح (*autovacuum_pages)(sqlite3*,
     unsigned صحیح(*)(تهی*,ثابت نویسه*,unsigned صحیح,unsigned صحیح,unsigned صحیح),
     تهی*, تهی(*)(تهی*));
  /* Version 3.38.0 and later */
  صحیح (*error_offset)(sqlite3*);
  صحیح (*vtab_rhs_مقدار)(sqlite3_index_info*,صحیح,sqlite3_مقدار**);
  صحیح (*vtab_distinct)(sqlite3_index_info*);
  صحیح (*vtab_in)(sqlite3_index_info*,صحیح,صحیح);
  صحیح (*vtab_in_first)(sqlite3_مقدار*,sqlite3_مقدار**);
  صحیح (*vtab_in_next)(sqlite3_مقدار*,sqlite3_مقدار**);
  /* Version 3.39.0 and later */
  صحیح (*deserialize)(sqlite3*,ثابت نویسه*,unsigned نویسه*,
                     sqlite3_int64,sqlite3_int64,unsigned);
  unsigned نویسه *(*serialize)(sqlite3*,ثابت نویسه *,sqlite3_int64*,
                              unsigned صحیح);
  ثابت نویسه *(*db_name)(sqlite3*,صحیح);
  /* Version 3.40.0 and later */
  صحیح (*مقدار_encoding)(sqlite3_مقدار*);
  /* Version 3.41.0 and later */
  صحیح (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  صحیح (*stmt_explain)(sqlite3_stmt*,صحیح);
  /* Version 3.44.0 and later */
  تهی *(*get_clientdata)(sqlite3*,ثابت نویسه*);
  صحیح (*set_clientdata)(sqlite3*, ثابت نویسه*, تهی*, تهی(*)(تهی*));
  /* Version 3.50.0 and later */
  صحیح (*setlk_timeout)(sqlite3*,صحیح,صحیح);
  /* Version 3.51.0 and later */
  صحیح (*set_errmsg)(sqlite3*,صحیح,ثابت نویسه*);
  صحیح (*db_status64)(sqlite3*,صحیح,sqlite3_int64*,sqlite3_int64*,صحیح);
  /* Version 3.52.0 and later */
  تهی (*str_truncate)(sqlite3_str*,صحیح);
  تهی (*str_free)(sqlite3_str*);
  صحیح (*carray_bind)(sqlite3_stmt*,صحیح,تهی*,صحیح,صحیح,تهی(*)(تهی*));
  صحیح (*carray_bind_v2)(sqlite3_stmt*,صحیح,تهی*,صحیح,صحیح,تهی(*)(تهی*),تهی*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ثابت نویسه*);
  تهی (*result_str)(sqlite3_context*,sqlite3_str*,صحیح);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
تعریفنوع صحیح (*sqlite3_loadext_entry)(
  sqlite3 *db,                       /* Handle to the database. */
  نویسه **pz_Err_پیام,                   /* Used to set error string on failure. */
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
#اگر !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_aggregate_شمار        sqlite3_api->aggregate_شمار
#endif
#define sqlite3_bind_blob              sqlite3_api->bind_blob
#define sqlite3_bind_double            sqlite3_api->bind_double
#define sqlite3_bind_int               sqlite3_api->bind_int
#define sqlite3_bind_int64             sqlite3_api->bind_int64
#define sqlite3_bind_null              sqlite3_api->bind_null
#define sqlite3_bind_parameter_شمار   sqlite3_api->bind_parameter_شمار
#define sqlite3_bind_parameter_index   sqlite3_api->bind_parameter_index
#define sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#define sqlite3_bind_text              sqlite3_api->bind_text
#define sqlite3_bind_text16            sqlite3_api->bind_text16
#define sqlite3_bind_مقدار             sqlite3_api->bind_مقدار
#define sqlite3_busy_handler           sqlite3_api->busy_handler
#define sqlite3_busy_timeout           sqlite3_api->busy_timeout
#define sqlite3_changes                sqlite3_api->changes
#define sqlite3_close                  sqlite3_api->close
#define sqlite3_collation_needed       sqlite3_api->collation_needed
#define sqlite3_collation_needed16     sqlite3_api->collation_needed16
#define sqlite3_column_blob            sqlite3_api->column_blob
#define sqlite3_column_bytes           sqlite3_api->column_bytes
#define sqlite3_column_bytes16         sqlite3_api->column_bytes16
#define sqlite3_column_شمار           sqlite3_api->column_شمار
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
#define sqlite3_column_مقدار           sqlite3_api->column_مقدار
#define sqlite3_commit_hook            sqlite3_api->commit_hook
#define sqlite3_complete               sqlite3_api->complete
#define sqlite3_complete16             sqlite3_api->complete16
#define sqlite3_create_collation       sqlite3_api->create_collation
#define sqlite3_create_collation16     sqlite3_api->create_collation16
#define sqlite3_create_function        sqlite3_api->create_function
#define sqlite3_create_function16      sqlite3_api->create_function16
#define sqlite3_create_module          sqlite3_api->create_module
#define sqlite3_create_module_v2       sqlite3_api->create_module_v2
#define sqlite3_data_شمار             sqlite3_api->data_شمار
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
#define sqlite3_free                   sqlite3_api->آزاد
#define sqlite3_free_table             sqlite3_api->free_table
#define sqlite3_get_autocommit         sqlite3_api->get_autocommit
#define sqlite3_get_auxdata            sqlite3_api->get_auxdata
#define sqlite3_get_table              sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_global_recover         sqlite3_api->global_recover
#endif
#define sqlite3_interrupt              sqlite3_api->interruptx
#define sqlite3_last_درج_rowid      sqlite3_api->last_درج_rowid
#define sqlite3_libversion             sqlite3_api->libversion
#define sqlite3_libversion_عدد      sqlite3_api->libversion_عدد
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
#define sqlite3_result_مقدار           sqlite3_api->result_مقدار
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
#define sqlite3_مقدار_blob             sqlite3_api->مقدار_blob
#define sqlite3_مقدار_bytes            sqlite3_api->مقدار_bytes
#define sqlite3_مقدار_bytes16          sqlite3_api->مقدار_bytes16
#define sqlite3_مقدار_double           sqlite3_api->مقدار_double
#define sqlite3_مقدار_int              sqlite3_api->مقدار_int
#define sqlite3_مقدار_int64            sqlite3_api->مقدار_int64
#define sqlite3_مقدار_numeric_type     sqlite3_api->مقدار_numeric_type
#define sqlite3_مقدار_text             sqlite3_api->مقدار_text
#define sqlite3_مقدار_text16           sqlite3_api->مقدار_text16
#define sqlite3_مقدار_text16be         sqlite3_api->مقدار_text16be
#define sqlite3_مقدار_text16le         sqlite3_api->مقدار_text16le
#define sqlite3_مقدار_type             sqlite3_api->مقدار_type
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
#define sqlite3_مقدار_dup              sqlite3_api->مقدار_dup
#define sqlite3_مقدار_free             sqlite3_api->مقدار_free
#define sqlite3_result_zeroblob64      sqlite3_api->result_zeroblob64
#define sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define sqlite3_مقدار_subtype          sqlite3_api->مقدار_subtype
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
#define sqlite3_set_last_درج_rowid  sqlite3_api->set_last_درج_rowid
/* Version 3.20.0 and later */
#define sqlite3_prepare_v3             sqlite3_api->prepare_v3
#define sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#define sqlite3_bind_pointer           sqlite3_api->bind_pointer
#define sqlite3_result_pointer         sqlite3_api->result_pointer
#define sqlite3_مقدار_pointer          sqlite3_api->مقدار_pointer
/* Version 3.22.0 and later */
#define sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#define sqlite3_مقدار_nochange         sqlite3_api->مقدار_nochange
#define sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#define sqlite3_keyword_شمار          sqlite3_api->keyword_شمار
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
#define sqlite3_str_مقدار              sqlite3_api->str_مقدار
/* Version 3.25.0 and later */
#define sqlite3_create_window_function sqlite3_api->create_window_function
/* Version 3.26.0 and later */
#define sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#define sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#define sqlite3_مقدار_frombind         sqlite3_api->مقدار_frombind
/* Version 3.30.0 and later */
#define sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#define sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#define sqlite3_uri_کلید                sqlite3_api->uri_کلید
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
#define sqlite3_vtab_rhs_مقدار         sqlite3_api->vtab_rhs_مقدار
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
#define sqlite3_مقدار_encoding         sqlite3_api->مقدار_encoding
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

#اگر !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ثابت sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    بیرونی ثابت sqlite3_api_routines *sqlite3_api;
#وگرنه
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (تهی)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */
