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
#અન્તર્ભાવ "sqlite3.h"

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
સંરચના sqlite3_api_routines {
  શૂન્ય * (*aggregate_context)(sqlite3_context*,પૂર્ણાંક nBytes);
  પૂર્ણાંક  (*aggregate_ગણતરી)(sqlite3_context*);
  પૂર્ણાંક  (*bind_blob)(sqlite3_stmt*,પૂર્ણાંક,અચલ શૂન્ય*,પૂર્ણાંક n,શૂન્ય(*)(શૂન્ય*));
  પૂર્ણાંક  (*bind_double)(sqlite3_stmt*,પૂર્ણાંક,દ્વિગુણ);
  પૂર્ણાંક  (*bind_int)(sqlite3_stmt*,પૂર્ણાંક,પૂર્ણાંક);
  પૂર્ણાંક  (*bind_int64)(sqlite3_stmt*,પૂર્ણાંક,sqlite_int64);
  પૂર્ણાંક  (*bind_null)(sqlite3_stmt*,પૂર્ણાંક);
  પૂર્ણાંક  (*bind_parameter_ગણતરી)(sqlite3_stmt*);
  પૂર્ણાંક  (*bind_parameter_index)(sqlite3_stmt*,અચલ અક્ષર*zName);
  અચલ અક્ષર * (*bind_parameter_name)(sqlite3_stmt*,પૂર્ણાંક);
  પૂર્ણાંક  (*bind_text)(sqlite3_stmt*,પૂર્ણાંક,અચલ અક્ષર*,પૂર્ણાંક n,શૂન્ય(*)(શૂન્ય*));
  પૂર્ણાંક  (*bind_text16)(sqlite3_stmt*,પૂર્ણાંક,અચલ શૂન્ય*,પૂર્ણાંક,શૂન્ય(*)(શૂન્ય*));
  પૂર્ણાંક  (*bind_કિંમત)(sqlite3_stmt*,પૂર્ણાંક,અચલ sqlite3_કિંમત*);
  પૂર્ણાંક  (*busy_handler)(sqlite3*,પૂર્ણાંક(*)(શૂન્ય*,પૂર્ણાંક),શૂન્ય*);
  પૂર્ણાંક  (*busy_timeout)(sqlite3*,પૂર્ણાંક ms);
  પૂર્ણાંક  (*changes)(sqlite3*);
  પૂર્ણાંક  (*close)(sqlite3*);
  પૂર્ણાંક  (*collation_needed)(sqlite3*,શૂન્ય*,શૂન્ય(*)(શૂન્ય*,sqlite3*,
                           પૂર્ણાંક eTextRep,અચલ અક્ષર*));
  પૂર્ણાંક  (*collation_needed16)(sqlite3*,શૂન્ય*,શૂન્ય(*)(શૂન્ય*,sqlite3*,
                             પૂર્ણાંક eTextRep,અચલ શૂન્ય*));
  અચલ શૂન્ય * (*column_blob)(sqlite3_stmt*,પૂર્ણાંક iCol);
  પૂર્ણાંક  (*column_bytes)(sqlite3_stmt*,પૂર્ણાંક iCol);
  પૂર્ણાંક  (*column_bytes16)(sqlite3_stmt*,પૂર્ણાંક iCol);
  પૂર્ણાંક  (*column_ગણતરી)(sqlite3_stmt*pStmt);
  અચલ અક્ષર * (*column_database_name)(sqlite3_stmt*,પૂર્ણાંક);
  અચલ શૂન્ય * (*column_database_name16)(sqlite3_stmt*,પૂર્ણાંક);
  અચલ અક્ષર * (*column_decltype)(sqlite3_stmt*,પૂર્ણાંક i);
  અચલ શૂન્ય * (*column_decltype16)(sqlite3_stmt*,પૂર્ણાંક);
  દ્વિગુણ  (*column_double)(sqlite3_stmt*,પૂર્ણાંક iCol);
  પૂર્ણાંક  (*column_int)(sqlite3_stmt*,પૂર્ણાંક iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,પૂર્ણાંક iCol);
  અચલ અક્ષર * (*column_name)(sqlite3_stmt*,પૂર્ણાંક);
  અચલ શૂન્ય * (*column_name16)(sqlite3_stmt*,પૂર્ણાંક);
  અચલ અક્ષર * (*column_origin_name)(sqlite3_stmt*,પૂર્ણાંક);
  અચલ શૂન્ય * (*column_origin_name16)(sqlite3_stmt*,પૂર્ણાંક);
  અચલ અક્ષર * (*column_table_name)(sqlite3_stmt*,પૂર્ણાંક);
  અચલ શૂન્ય * (*column_table_name16)(sqlite3_stmt*,પૂર્ણાંક);
  અચલ unsigned અક્ષર * (*column_text)(sqlite3_stmt*,પૂર્ણાંક iCol);
  અચલ શૂન્ય * (*column_text16)(sqlite3_stmt*,પૂર્ણાંક iCol);
  પૂર્ણાંક  (*column_type)(sqlite3_stmt*,પૂર્ણાંક iCol);
  sqlite3_કિંમત* (*column_કિંમત)(sqlite3_stmt*,પૂર્ણાંક iCol);
  શૂન્ય * (*commit_hook)(sqlite3*,પૂર્ણાંક(*)(શૂન્ય*),શૂન્ય*);
  પૂર્ણાંક  (*complete)(અચલ અક્ષર*sql);
  પૂર્ણાંક  (*complete16)(અચલ શૂન્ય*sql);
  પૂર્ણાંક  (*create_collation)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,શૂન્ય*,
                           પૂર્ણાંક(*)(શૂન્ય*,પૂર્ણાંક,અચલ શૂન્ય*,પૂર્ણાંક,અચલ શૂન્ય*));
  પૂર્ણાંક  (*create_collation16)(sqlite3*,અચલ શૂન્ય*,પૂર્ણાંક,શૂન્ય*,
                             પૂર્ણાંક(*)(શૂન્ય*,પૂર્ણાંક,અચલ શૂન્ય*,પૂર્ણાંક,અચલ શૂન્ય*));
  પૂર્ણાંક  (*create_function)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,પૂર્ણાંક,શૂન્ય*,
                          શૂન્ય (*xFunc)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
                          શૂન્ય (*xStep)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
                          શૂન્ય (*xFinal)(sqlite3_context*));
  પૂર્ણાંક  (*create_function16)(sqlite3*,અચલ શૂન્ય*,પૂર્ણાંક,પૂર્ણાંક,શૂન્ય*,
                            શૂન્ય (*xFunc)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
                            શૂન્ય (*xStep)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
                            શૂન્ય (*xFinal)(sqlite3_context*));
  પૂર્ણાંક (*create_module)(sqlite3*,અચલ અક્ષર*,અચલ sqlite3_module*,શૂન્ય*);
  પૂર્ણાંક  (*data_ગણતરી)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  પૂર્ણાંક (*declare_vtab)(sqlite3*,અચલ અક્ષર*);
  પૂર્ણાંક  (*enable_shared_cache)(પૂર્ણાંક);
  પૂર્ણાંક  (*errcode)(sqlite3*db);
  અચલ અક્ષર * (*errmsg)(sqlite3*);
  અચલ શૂન્ય * (*errmsg16)(sqlite3*);
  પૂર્ણાંક  (*exec)(sqlite3*,અચલ અક્ષર*,sqlite3_callback,શૂન્ય*,અક્ષર**);
  પૂર્ણાંક  (*expired)(sqlite3_stmt*);
  પૂર્ણાંક  (*finalize)(sqlite3_stmt*pStmt);
  શૂન્ય  (*મુક્ત)(શૂન્ય*);
  શૂન્ય  (*free_table)(અક્ષર**result);
  પૂર્ણાંક  (*get_autocommit)(sqlite3*);
  શૂન્ય * (*get_auxdata)(sqlite3_context*,પૂર્ણાંક);
  પૂર્ણાંક  (*get_table)(sqlite3*,અચલ અક્ષર*,અક્ષર***,પૂર્ણાંક*,પૂર્ણાંક*,અક્ષર**);
  પૂર્ણાંક  (*global_recover)(શૂન્ય);
  શૂન્ય  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_ઉમેરો_rowid)(sqlite3*);
  અચલ અક્ષર * (*libversion)(શૂન્ય);
  પૂર્ણાંક  (*libversion_સંખ્યા)(શૂન્ય);
  શૂન્ય *(*malloc)(પૂર્ણાંક);
  અક્ષર *(SQLITE_CDECL*mprintf)(અચલ અક્ષર*,...);
  પૂર્ણાંક  (*open)(અચલ અક્ષર*,sqlite3**);
  પૂર્ણાંક  (*open16)(અચલ શૂન્ય*,sqlite3**);
  પૂર્ણાંક  (*prepare)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,sqlite3_stmt**,અચલ અક્ષર**);
  પૂર્ણાંક  (*prepare16)(sqlite3*,અચલ શૂન્ય*,પૂર્ણાંક,sqlite3_stmt**,અચલ શૂન્ય**);
  શૂન્ય * (*profile)(sqlite3*,શૂન્ય(*)(શૂન્ય*,અચલ અક્ષર*,sqlite_uint64),શૂન્ય*);
  શૂન્ય  (*progress_handler)(sqlite3*,પૂર્ણાંક,પૂર્ણાંક(*)(શૂન્ય*),શૂન્ય*);
  શૂન્ય *(*realloc)(શૂન્ય*,પૂર્ણાંક);
  પૂર્ણાંક  (*reset)(sqlite3_stmt*pStmt);
  શૂન્ય  (*result_blob)(sqlite3_context*,અચલ શૂન્ય*,પૂર્ણાંક,શૂન્ય(*)(શૂન્ય*));
  શૂન્ય  (*result_double)(sqlite3_context*,દ્વિગુણ);
  શૂન્ય  (*result_error)(sqlite3_context*,અચલ અક્ષર*,પૂર્ણાંક);
  શૂન્ય  (*result_error16)(sqlite3_context*,અચલ શૂન્ય*,પૂર્ણાંક);
  શૂન્ય  (*result_int)(sqlite3_context*,પૂર્ણાંક);
  શૂન્ય  (*result_int64)(sqlite3_context*,sqlite_int64);
  શૂન્ય  (*result_null)(sqlite3_context*);
  શૂન્ય  (*result_text)(sqlite3_context*,અચલ અક્ષર*,પૂર્ણાંક,શૂન્ય(*)(શૂન્ય*));
  શૂન્ય  (*result_text16)(sqlite3_context*,અચલ શૂન્ય*,પૂર્ણાંક,શૂન્ય(*)(શૂન્ય*));
  શૂન્ય  (*result_text16be)(sqlite3_context*,અચલ શૂન્ય*,પૂર્ણાંક,શૂન્ય(*)(શૂન્ય*));
  શૂન્ય  (*result_text16le)(sqlite3_context*,અચલ શૂન્ય*,પૂર્ણાંક,શૂન્ય(*)(શૂન્ય*));
  શૂન્ય  (*result_કિંમત)(sqlite3_context*,sqlite3_કિંમત*);
  શૂન્ય *(*rollback_hook)(sqlite3*,શૂન્ય(*)(શૂન્ય*),શૂન્ય*);
  પૂર્ણાંક  (*set_authorizer)(sqlite3*,પૂર્ણાંક(*)(શૂન્ય*,પૂર્ણાંક,અચલ અક્ષર*,અચલ અક્ષર*,
                         અચલ અક્ષર*,અચલ અક્ષર*),શૂન્ય*);
  શૂન્ય  (*set_auxdata)(sqlite3_context*,પૂર્ણાંક,શૂન્ય*,શૂન્ય (*)(શૂન્ય*));
  અક્ષર *(SQLITE_CDECL*xsnprintf)(પૂર્ણાંક,અક્ષર*,અચલ અક્ષર*,...);
  પૂર્ણાંક  (*step)(sqlite3_stmt*);
  પૂર્ણાંક  (*table_column_metadata)(sqlite3*,અચલ અક્ષર*,અચલ અક્ષર*,અચલ અક્ષર*,
                                અક્ષર અચલ**,અક્ષર અચલ**,પૂર્ણાંક*,પૂર્ણાંક*,પૂર્ણાંક*);
  શૂન્ય  (*thread_cleanup)(શૂન્ય);
  પૂર્ણાંક  (*total_changes)(sqlite3*);
  શૂન્ય * (*trace)(sqlite3*,શૂન્ય(*xTrace)(શૂન્ય*,અચલ અક્ષર*),શૂન્ય*);
  પૂર્ણાંક  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  શૂન્ય * (*update_hook)(sqlite3*,શૂન્ય(*)(શૂન્ય*,પૂર્ણાંક ,અક્ષર અચલ*,અક્ષર અચલ*,
                                         sqlite_int64),શૂન્ય*);
  શૂન્ય * (*user_data)(sqlite3_context*);
  અચલ શૂન્ય * (*કિંમત_blob)(sqlite3_કિંમત*);
  પૂર્ણાંક  (*કિંમત_bytes)(sqlite3_કિંમત*);
  પૂર્ણાંક  (*કિંમત_bytes16)(sqlite3_કિંમત*);
  દ્વિગુણ  (*કિંમત_double)(sqlite3_કિંમત*);
  પૂર્ણાંક  (*કિંમત_int)(sqlite3_કિંમત*);
  sqlite_int64  (*કિંમત_int64)(sqlite3_કિંમત*);
  પૂર્ણાંક  (*કિંમત_numeric_type)(sqlite3_કિંમત*);
  અચલ unsigned અક્ષર * (*કિંમત_text)(sqlite3_કિંમત*);
  અચલ શૂન્ય * (*કિંમત_text16)(sqlite3_કિંમત*);
  અચલ શૂન્ય * (*કિંમત_text16be)(sqlite3_કિંમત*);
  અચલ શૂન્ય * (*કિંમત_text16le)(sqlite3_કિંમત*);
  પૂર્ણાંક  (*કિંમત_type)(sqlite3_કિંમત*);
  અક્ષર *(*vmprintf)(અચલ અક્ષર*,va_list);
  /* Added ??? */
  પૂર્ણાંક (*overload_function)(sqlite3*, અચલ અક્ષર *zFuncName, પૂર્ણાંક nArg);
  /* Added by 3.3.13 */
  પૂર્ણાંક (*prepare_v2)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,sqlite3_stmt**,અચલ અક્ષર**);
  પૂર્ણાંક (*prepare16_v2)(sqlite3*,અચલ શૂન્ય*,પૂર્ણાંક,sqlite3_stmt**,અચલ શૂન્ય**);
  પૂર્ણાંક (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  પૂર્ણાંક (*create_module_v2)(sqlite3*,અચલ અક્ષર*,અચલ sqlite3_module*,શૂન્ય*,
                          શૂન્ય (*xDestroy)(શૂન્ય *));
  /* Added by 3.5.0 */
  પૂર્ણાંક (*bind_zeroblob)(sqlite3_stmt*,પૂર્ણાંક,પૂર્ણાંક);
  પૂર્ણાંક (*blob_bytes)(sqlite3_blob*);
  પૂર્ણાંક (*blob_close)(sqlite3_blob*);
  પૂર્ણાંક (*blob_open)(sqlite3*,અચલ અક્ષર*,અચલ અક્ષર*,અચલ અક્ષર*,sqlite3_int64,
                   પૂર્ણાંક,sqlite3_blob**);
  પૂર્ણાંક (*blob_read)(sqlite3_blob*,શૂન્ય*,પૂર્ણાંક,પૂર્ણાંક);
  પૂર્ણાંક (*blob_write)(sqlite3_blob*,અચલ શૂન્ય*,પૂર્ણાંક,પૂર્ણાંક);
  પૂર્ણાંક (*create_collation_v2)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,શૂન્ય*,
                             પૂર્ણાંક(*)(શૂન્ય*,પૂર્ણાંક,અચલ શૂન્ય*,પૂર્ણાંક,અચલ શૂન્ય*),
                             શૂન્ય(*)(શૂન્ય*));
  પૂર્ણાંક (*file_control)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,શૂન્ય*);
  sqlite3_int64 (*memory_highwater)(પૂર્ણાંક);
  sqlite3_int64 (*memory_used)(શૂન્ય);
  sqlite3_mutex *(*mutex_alloc)(પૂર્ણાંક);
  શૂન્ય (*mutex_enter)(sqlite3_mutex*);
  શૂન્ય (*mutex_free)(sqlite3_mutex*);
  શૂન્ય (*mutex_leave)(sqlite3_mutex*);
  પૂર્ણાંક (*mutex_try)(sqlite3_mutex*);
  પૂર્ણાંક (*open_v2)(અચલ અક્ષર*,sqlite3**,પૂર્ણાંક,અચલ અક્ષર*);
  પૂર્ણાંક (*release_memory)(પૂર્ણાંક);
  શૂન્ય (*result_error_nomem)(sqlite3_context*);
  શૂન્ય (*result_error_toobig)(sqlite3_context*);
  પૂર્ણાંક (*sleep)(પૂર્ણાંક);
  શૂન્ય (*soft_heap_limit)(પૂર્ણાંક);
  sqlite3_vfs *(*vfs_find)(અચલ અક્ષર*);
  પૂર્ણાંક (*vfs_register)(sqlite3_vfs*,પૂર્ણાંક);
  પૂર્ણાંક (*vfs_unregister)(sqlite3_vfs*);
  પૂર્ણાંક (*xthreadsafe)(શૂન્ય);
  શૂન્ય (*result_zeroblob)(sqlite3_context*,પૂર્ણાંક);
  શૂન્ય (*result_error_code)(sqlite3_context*,પૂર્ણાંક);
  પૂર્ણાંક (SQLITE_CDECL*test_control)(પૂર્ણાંક, ...);
  શૂન્ય (*randomness)(પૂર્ણાંક,શૂન્ય*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  પૂર્ણાંક (*extended_result_codes)(sqlite3*,પૂર્ણાંક);
  પૂર્ણાંક (*limit)(sqlite3*,પૂર્ણાંક,પૂર્ણાંક);
  sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
  અચલ અક્ષર *(*sql)(sqlite3_stmt*);
  પૂર્ણાંક (*status)(પૂર્ણાંક,પૂર્ણાંક*,પૂર્ણાંક*,પૂર્ણાંક);
  પૂર્ણાંક (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,અચલ અક્ષર*,sqlite3*,અચલ અક્ષર*);
  પૂર્ણાંક (*backup_pagecount)(sqlite3_backup*);
  પૂર્ણાંક (*backup_remaining)(sqlite3_backup*);
  પૂર્ણાંક (*backup_step)(sqlite3_backup*,પૂર્ણાંક);
  અચલ અક્ષર *(*compileoption_get)(પૂર્ણાંક);
  પૂર્ણાંક (*compileoption_used)(અચલ અક્ષર*);
  પૂર્ણાંક (*create_function_v2)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,પૂર્ણાંક,શૂન્ય*,
                            શૂન્ય (*xFunc)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
                            શૂન્ય (*xStep)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
                            શૂન્ય (*xFinal)(sqlite3_context*),
                            શૂન્ય(*xDestroy)(શૂન્ય*));
  પૂર્ણાંક (SQLITE_CDECL*db_config)(sqlite3*,પૂર્ણાંક,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  પૂર્ણાંક (*db_status)(sqlite3*,પૂર્ણાંક,પૂર્ણાંક*,પૂર્ણાંક*,પૂર્ણાંક);
  પૂર્ણાંક (*extended_errcode)(sqlite3*);
  શૂન્ય (SQLITE_CDECL*log)(પૂર્ણાંક,અચલ અક્ષર*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  અચલ અક્ષર *(*sourceid)(શૂન્ય);
  પૂર્ણાંક (*stmt_status)(sqlite3_stmt*,પૂર્ણાંક,પૂર્ણાંક);
  પૂર્ણાંક (*strnicmp)(અચલ અક્ષર*,અચલ અક્ષર*,પૂર્ણાંક);
  પૂર્ણાંક (*unlock_notify)(sqlite3*,શૂન્ય(*)(શૂન્ય**,પૂર્ણાંક),શૂન્ય*);
  પૂર્ણાંક (*wal_autocheckpoint)(sqlite3*,પૂર્ણાંક);
  પૂર્ણાંક (*wal_checkpoint)(sqlite3*,અચલ અક્ષર*);
  શૂન્ય *(*wal_hook)(sqlite3*,પૂર્ણાંક(*)(શૂન્ય*,sqlite3*,અચલ અક્ષર*,પૂર્ણાંક),શૂન્ય*);
  પૂર્ણાંક (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  પૂર્ણાંક (SQLITE_CDECL*vtab_config)(sqlite3*,પૂર્ણાંક op,...);
  પૂર્ણાંક (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  પૂર્ણાંક (*close_v2)(sqlite3*);
  અચલ અક્ષર *(*db_filename)(sqlite3*,અચલ અક્ષર*);
  પૂર્ણાંક (*db_readonly)(sqlite3*,અચલ અક્ષર*);
  પૂર્ણાંક (*db_release_memory)(sqlite3*);
  અચલ અક્ષર *(*errstr)(પૂર્ણાંક);
  પૂર્ણાંક (*stmt_busy)(sqlite3_stmt*);
  પૂર્ણાંક (*stmt_readonly)(sqlite3_stmt*);
  પૂર્ણાંક (*stricmp)(અચલ અક્ષર*,અચલ અક્ષર*);
  પૂર્ણાંક (*uri_boolean)(અચલ અક્ષર*,અચલ અક્ષર*,પૂર્ણાંક);
  sqlite3_int64 (*uri_int64)(અચલ અક્ષર*,અચલ અક્ષર*,sqlite3_int64);
  અચલ અક્ષર *(*uri_parameter)(અચલ અક્ષર*,અચલ અક્ષર*);
  અક્ષર *(*xvsnprintf)(પૂર્ણાંક,અક્ષર*,અચલ અક્ષર*,va_list);
  પૂર્ણાંક (*wal_checkpoint_v2)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,પૂર્ણાંક*,પૂર્ણાંક*);
  /* Version 3.8.7 and later */
  પૂર્ણાંક (*auto_extension)(શૂન્ય(*)(શૂન્ય));
  પૂર્ણાંક (*bind_blob64)(sqlite3_stmt*,પૂર્ણાંક,અચલ શૂન્ય*,sqlite3_uint64,
                     શૂન્ય(*)(શૂન્ય*));
  પૂર્ણાંક (*bind_text64)(sqlite3_stmt*,પૂર્ણાંક,અચલ અક્ષર*,sqlite3_uint64,
                      શૂન્ય(*)(શૂન્ય*),unsigned અક્ષર);
  પૂર્ણાંક (*cancel_auto_extension)(શૂન્ય(*)(શૂન્ય));
  પૂર્ણાંક (*load_extension)(sqlite3*,અચલ અક્ષર*,અચલ અક્ષર*,અક્ષર**);
  શૂન્ય *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(શૂન્ય*);
  શૂન્ય *(*realloc64)(શૂન્ય*,sqlite3_uint64);
  શૂન્ય (*reset_auto_extension)(શૂન્ય);
  શૂન્ય (*result_blob64)(sqlite3_context*,અચલ શૂન્ય*,sqlite3_uint64,
                        શૂન્ય(*)(શૂન્ય*));
  શૂન્ય (*result_text64)(sqlite3_context*,અચલ અક્ષર*,sqlite3_uint64,
                         શૂન્ય(*)(શૂન્ય*), unsigned અક્ષર);
  પૂર્ણાંક (*strglob)(અચલ અક્ષર*,અચલ અક્ષર*);
  /* Version 3.8.11 and later */
  sqlite3_કિંમત *(*કિંમત_dup)(અચલ sqlite3_કિંમત*);
  શૂન્ય (*કિંમત_free)(sqlite3_કિંમત*);
  પૂર્ણાંક (*result_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  પૂર્ણાંક (*bind_zeroblob64)(sqlite3_stmt*, પૂર્ણાંક, sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned પૂર્ણાંક (*કિંમત_subtype)(sqlite3_કિંમત*);
  શૂન્ય (*result_subtype)(sqlite3_context*,unsigned પૂર્ણાંક);
  /* Version 3.10.0 and later */
  પૂર્ણાંક (*status64)(પૂર્ણાંક,sqlite3_int64*,sqlite3_int64*,પૂર્ણાંક);
  પૂર્ણાંક (*strlike)(અચલ અક્ષર*,અચલ અક્ષર*,unsigned પૂર્ણાંક);
  પૂર્ણાંક (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  પૂર્ણાંક (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  પૂર્ણાંક (*trace_v2)(sqlite3*,unsigned,પૂર્ણાંક(*)(unsigned,શૂન્ય*,શૂન્ય*,શૂન્ય*),શૂન્ય*);
  અક્ષર *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  શૂન્ય (*set_last_ઉમેરો_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  પૂર્ણાંક (*prepare_v3)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,unsigned પૂર્ણાંક,
                    sqlite3_stmt**,અચલ અક્ષર**);
  પૂર્ણાંક (*prepare16_v3)(sqlite3*,અચલ શૂન્ય*,પૂર્ણાંક,unsigned પૂર્ણાંક,
                      sqlite3_stmt**,અચલ શૂન્ય**);
  પૂર્ણાંક (*bind_pointer)(sqlite3_stmt*,પૂર્ણાંક,શૂન્ય*,અચલ અક્ષર*,શૂન્ય(*)(શૂન્ય*));
  શૂન્ય (*result_pointer)(sqlite3_context*,શૂન્ય*,અચલ અક્ષર*,શૂન્ય(*)(શૂન્ય*));
  શૂન્ય *(*કિંમત_pointer)(sqlite3_કિંમત*,અચલ અક્ષર*);
  પૂર્ણાંક (*vtab_nochange)(sqlite3_context*);
  પૂર્ણાંક (*કિંમત_nochange)(sqlite3_કિંમત*);
  અચલ અક્ષર *(*vtab_collation)(sqlite3_index_info*,પૂર્ણાંક);
  /* Version 3.24.0 and later */
  પૂર્ણાંક (*keyword_ગણતરી)(શૂન્ય);
  પૂર્ણાંક (*keyword_name)(પૂર્ણાંક,અચલ અક્ષર**,પૂર્ણાંક*);
  પૂર્ણાંક (*keyword_check)(અચલ અક્ષર*,પૂર્ણાંક);
  sqlite3_str *(*str_new)(sqlite3*);
  અક્ષર *(*str_finish)(sqlite3_str*);
  શૂન્ય (SQLITE_CDECL*str_appendf)(sqlite3_str*, અચલ અક્ષર *zFormat, ...);
  શૂન્ય (*str_vappendf)(sqlite3_str*, અચલ અક્ષર *zFormat, va_list);
  શૂન્ય (*str_append)(sqlite3_str*, અચલ અક્ષર *zIn, પૂર્ણાંક N);
  શૂન્ય (*str_appendall)(sqlite3_str*, અચલ અક્ષર *zIn);
  શૂન્ય (*str_appendchar)(sqlite3_str*, પૂર્ણાંક N, અક્ષર C);
  શૂન્ય (*str_reset)(sqlite3_str*);
  પૂર્ણાંક (*str_errcode)(sqlite3_str*);
  પૂર્ણાંક (*str_length)(sqlite3_str*);
  અક્ષર *(*str_કિંમત)(sqlite3_str*);
  /* Version 3.25.0 and later */
  પૂર્ણાંક (*create_window_function)(sqlite3*,અચલ અક્ષર*,પૂર્ણાંક,પૂર્ણાંક,શૂન્ય*,
                            શૂન્ય (*xStep)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
                            શૂન્ય (*xFinal)(sqlite3_context*),
                            શૂન્ય (*x_કિંમત)(sqlite3_context*),
                            શૂન્ય (*xInv)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),
                            શૂન્ય(*xDestroy)(શૂન્ય*));
  /* Version 3.26.0 and later */
  અચલ અક્ષર *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  પૂર્ણાંક (*stmt_isexplain)(sqlite3_stmt*);
  પૂર્ણાંક (*કિંમત_frombind)(sqlite3_કિંમત*);
  /* Version 3.30.0 and later */
  પૂર્ણાંક (*drop_modules)(sqlite3*,અચલ અક્ષર**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  અચલ અક્ષર *(*uri_ચાવી)(અચલ અક્ષર*,પૂર્ણાંક);
  અચલ અક્ષર *(*filename_database)(અચલ અક્ષર*);
  અચલ અક્ષર *(*filename_journal)(અચલ અક્ષર*);
  અચલ અક્ષર *(*filename_wal)(અચલ અક્ષર*);
  /* Version 3.32.0 and later */
  અચલ અક્ષર *(*create_filename)(અચલ અક્ષર*,અચલ અક્ષર*,અચલ અક્ષર*,
                           પૂર્ણાંક,અચલ અક્ષર**);
  શૂન્ય (*free_filename)(અચલ અક્ષર*);
  sqlite3_file *(*database_file_object)(અચલ અક્ષર*);
  /* Version 3.34.0 and later */
  પૂર્ણાંક (*txn_state)(sqlite3*,અચલ અક્ષર*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*total_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  પૂર્ણાંક (*autovacuum_pages)(sqlite3*,
     unsigned પૂર્ણાંક(*)(શૂન્ય*,અચલ અક્ષર*,unsigned પૂર્ણાંક,unsigned પૂર્ણાંક,unsigned પૂર્ણાંક),
     શૂન્ય*, શૂન્ય(*)(શૂન્ય*));
  /* Version 3.38.0 and later */
  પૂર્ણાંક (*error_offset)(sqlite3*);
  પૂર્ણાંક (*vtab_rhs_કિંમત)(sqlite3_index_info*,પૂર્ણાંક,sqlite3_કિંમત**);
  પૂર્ણાંક (*vtab_distinct)(sqlite3_index_info*);
  પૂર્ણાંક (*vtab_in)(sqlite3_index_info*,પૂર્ણાંક,પૂર્ણાંક);
  પૂર્ણાંક (*vtab_in_first)(sqlite3_કિંમત*,sqlite3_કિંમત**);
  પૂર્ણાંક (*vtab_in_next)(sqlite3_કિંમત*,sqlite3_કિંમત**);
  /* Version 3.39.0 and later */
  પૂર્ણાંક (*deserialize)(sqlite3*,અચલ અક્ષર*,unsigned અક્ષર*,
                     sqlite3_int64,sqlite3_int64,unsigned);
  unsigned અક્ષર *(*serialize)(sqlite3*,અચલ અક્ષર *,sqlite3_int64*,
                              unsigned પૂર્ણાંક);
  અચલ અક્ષર *(*db_name)(sqlite3*,પૂર્ણાંક);
  /* Version 3.40.0 and later */
  પૂર્ણાંક (*કિંમત_encoding)(sqlite3_કિંમત*);
  /* Version 3.41.0 and later */
  પૂર્ણાંક (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  પૂર્ણાંક (*stmt_explain)(sqlite3_stmt*,પૂર્ણાંક);
  /* Version 3.44.0 and later */
  શૂન્ય *(*get_clientdata)(sqlite3*,અચલ અક્ષર*);
  પૂર્ણાંક (*set_clientdata)(sqlite3*, અચલ અક્ષર*, શૂન્ય*, શૂન્ય(*)(શૂન્ય*));
  /* Version 3.50.0 and later */
  પૂર્ણાંક (*setlk_timeout)(sqlite3*,પૂર્ણાંક,પૂર્ણાંક);
  /* Version 3.51.0 and later */
  પૂર્ણાંક (*set_errmsg)(sqlite3*,પૂર્ણાંક,અચલ અક્ષર*);
  પૂર્ણાંક (*db_status64)(sqlite3*,પૂર્ણાંક,sqlite3_int64*,sqlite3_int64*,પૂર્ણાંક);
  /* Version 3.52.0 and later */
  શૂન્ય (*str_truncate)(sqlite3_str*,પૂર્ણાંક);
  શૂન્ય (*str_free)(sqlite3_str*);
  પૂર્ણાંક (*carray_bind)(sqlite3_stmt*,પૂર્ણાંક,શૂન્ય*,પૂર્ણાંક,પૂર્ણાંક,શૂન્ય(*)(શૂન્ય*));
  પૂર્ણાંક (*carray_bind_v2)(sqlite3_stmt*,પૂર્ણાંક,શૂન્ય*,પૂર્ણાંક,પૂર્ણાંક,શૂન્ય(*)(શૂન્ય*),શૂન્ય*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(અચલ અક્ષર*);
  શૂન્ય (*result_str)(sqlite3_context*,sqlite3_str*,પૂર્ણાંક);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
પ્રકારવ્યાખ્યા પૂર્ણાંક (*sqlite3_loadext_entry)(
  sqlite3 *db,                       /* Handle to the database. */
  અક્ષર **pz_Err_સંદેશ,                   /* Used to set error string on failure. */
  અચલ sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#જો !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_aggregate_ગણતરી        sqlite3_api->aggregate_ગણતરી
#endif
#define sqlite3_bind_blob              sqlite3_api->bind_blob
#define sqlite3_bind_double            sqlite3_api->bind_double
#define sqlite3_bind_int               sqlite3_api->bind_int
#define sqlite3_bind_int64             sqlite3_api->bind_int64
#define sqlite3_bind_null              sqlite3_api->bind_null
#define sqlite3_bind_parameter_ગણતરી   sqlite3_api->bind_parameter_ગણતરી
#define sqlite3_bind_parameter_index   sqlite3_api->bind_parameter_index
#define sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#define sqlite3_bind_text              sqlite3_api->bind_text
#define sqlite3_bind_text16            sqlite3_api->bind_text16
#define sqlite3_bind_કિંમત             sqlite3_api->bind_કિંમત
#define sqlite3_busy_handler           sqlite3_api->busy_handler
#define sqlite3_busy_timeout           sqlite3_api->busy_timeout
#define sqlite3_changes                sqlite3_api->changes
#define sqlite3_close                  sqlite3_api->close
#define sqlite3_collation_needed       sqlite3_api->collation_needed
#define sqlite3_collation_needed16     sqlite3_api->collation_needed16
#define sqlite3_column_blob            sqlite3_api->column_blob
#define sqlite3_column_bytes           sqlite3_api->column_bytes
#define sqlite3_column_bytes16         sqlite3_api->column_bytes16
#define sqlite3_column_ગણતરી           sqlite3_api->column_ગણતરી
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
#define sqlite3_column_કિંમત           sqlite3_api->column_કિંમત
#define sqlite3_commit_hook            sqlite3_api->commit_hook
#define sqlite3_complete               sqlite3_api->complete
#define sqlite3_complete16             sqlite3_api->complete16
#define sqlite3_create_collation       sqlite3_api->create_collation
#define sqlite3_create_collation16     sqlite3_api->create_collation16
#define sqlite3_create_function        sqlite3_api->create_function
#define sqlite3_create_function16      sqlite3_api->create_function16
#define sqlite3_create_module          sqlite3_api->create_module
#define sqlite3_create_module_v2       sqlite3_api->create_module_v2
#define sqlite3_data_ગણતરી             sqlite3_api->data_ગણતરી
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
#define sqlite3_free                   sqlite3_api->મુક્ત
#define sqlite3_free_table             sqlite3_api->free_table
#define sqlite3_get_autocommit         sqlite3_api->get_autocommit
#define sqlite3_get_auxdata            sqlite3_api->get_auxdata
#define sqlite3_get_table              sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_global_recover         sqlite3_api->global_recover
#endif
#define sqlite3_interrupt              sqlite3_api->interruptx
#define sqlite3_last_ઉમેરો_rowid      sqlite3_api->last_ઉમેરો_rowid
#define sqlite3_libversion             sqlite3_api->libversion
#define sqlite3_libversion_સંખ્યા      sqlite3_api->libversion_સંખ્યા
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
#define sqlite3_result_કિંમત           sqlite3_api->result_કિંમત
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
#define sqlite3_કિંમત_blob             sqlite3_api->કિંમત_blob
#define sqlite3_કિંમત_bytes            sqlite3_api->કિંમત_bytes
#define sqlite3_કિંમત_bytes16          sqlite3_api->કિંમત_bytes16
#define sqlite3_કિંમત_double           sqlite3_api->કિંમત_double
#define sqlite3_કિંમત_int              sqlite3_api->કિંમત_int
#define sqlite3_કિંમત_int64            sqlite3_api->કિંમત_int64
#define sqlite3_કિંમત_numeric_type     sqlite3_api->કિંમત_numeric_type
#define sqlite3_કિંમત_text             sqlite3_api->કિંમત_text
#define sqlite3_કિંમત_text16           sqlite3_api->કિંમત_text16
#define sqlite3_કિંમત_text16be         sqlite3_api->કિંમત_text16be
#define sqlite3_કિંમત_text16le         sqlite3_api->કિંમત_text16le
#define sqlite3_કિંમત_type             sqlite3_api->કિંમત_type
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
#define sqlite3_કિંમત_dup              sqlite3_api->કિંમત_dup
#define sqlite3_કિંમત_free             sqlite3_api->કિંમત_free
#define sqlite3_result_zeroblob64      sqlite3_api->result_zeroblob64
#define sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define sqlite3_કિંમત_subtype          sqlite3_api->કિંમત_subtype
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
#define sqlite3_set_last_ઉમેરો_rowid  sqlite3_api->set_last_ઉમેરો_rowid
/* Version 3.20.0 and later */
#define sqlite3_prepare_v3             sqlite3_api->prepare_v3
#define sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#define sqlite3_bind_pointer           sqlite3_api->bind_pointer
#define sqlite3_result_pointer         sqlite3_api->result_pointer
#define sqlite3_કિંમત_pointer          sqlite3_api->કિંમત_pointer
/* Version 3.22.0 and later */
#define sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#define sqlite3_કિંમત_nochange         sqlite3_api->કિંમત_nochange
#define sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#define sqlite3_keyword_ગણતરી          sqlite3_api->keyword_ગણતરી
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
#define sqlite3_str_કિંમત              sqlite3_api->str_કિંમત
/* Version 3.25.0 and later */
#define sqlite3_create_window_function sqlite3_api->create_window_function
/* Version 3.26.0 and later */
#define sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#define sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#define sqlite3_કિંમત_frombind         sqlite3_api->કિંમત_frombind
/* Version 3.30.0 and later */
#define sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#define sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#define sqlite3_uri_ચાવી                sqlite3_api->uri_ચાવી
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
#define sqlite3_vtab_rhs_કિંમત         sqlite3_api->vtab_rhs_કિંમત
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
#define sqlite3_કિંમત_encoding         sqlite3_api->કિંમત_encoding
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

#જો !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     અચલ sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    બાહ્ય અચલ sqlite3_api_routines *sqlite3_api;
#નહીંતર
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (શૂન્ય)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */
