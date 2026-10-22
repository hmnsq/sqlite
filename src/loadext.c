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
** This file contains code used to dynamically load extensions into
** the SQLite library.
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_CORE
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3ext.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
/*
** Some API routines are omitted when various features are
** excluded from a build of SQLite.  Substitute a NULL pointer
** for any missing APIs.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_METADATA
# define sqlite3_ਕਾਲਮ_database_name   0
# define sqlite3_ਕਾਲਮ_database_name16 0
# define sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name      0
# define sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name16    0
# define sqlite3_ਕਾਲਮ_origin_name     0
# define sqlite3_ਕਾਲਮ_origin_name16   0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
# define sqlite3_ਸੈੱਟ_authorizer         0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
# define sqlite3_bind_text16            0
# define sqlite3_collation_needed16     0
# define sqlite3_ਕਾਲਮ_decltype16      0
# define sqlite3_ਕਾਲਮ_name16          0
# define sqlite3_ਕਾਲਮ_text16          0
# define sqlite3_complete16             0
# define sqlite3_create_collation16     0
# define sqlite3_create_function16      0
# define sqlite3_errmsg16               0
# define sqlite3_open16                 0
# define sqlite3_prepare16              0
# define sqlite3_prepare16_v2           0
# define sqlite3_prepare16_v3           0
# define sqlite3_ਨਤੀਜਾ_error16         0
# define sqlite3_ਨਤੀਜਾ_text16          0
# define sqlite3_ਨਤੀਜਾ_text16be        0
# define sqlite3_ਨਤੀਜਾ_text16le        0
# define sqlite3_ਮੁੱਲ_text16           0
# define sqlite3_ਮੁੱਲ_text16be         0
# define sqlite3_ਮੁੱਲ_text16le         0
# define sqlite3_ਕਾਲਮ_database_name16 0
# define sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name16    0
# define sqlite3_ਕਾਲਮ_origin_name16   0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPLETE
# define sqlite3_complete 0
# define sqlite3_complete16 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DECLTYPE
# define sqlite3_ਕਾਲਮ_decltype16      0
# define sqlite3_ਕਾਲਮ_decltype        0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
# define sqlite3_ਤਰੱਕੀ_handler 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
# define sqlite3_create_module 0
# define sqlite3_create_module_v2 0
# define sqlite3_declare_vtab 0
# define sqlite3_vtab_config 0
# define sqlite3_vtab_on_conflict 0
# define sqlite3_vtab_collation 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
# define sqlite3_enable_shared_cache 0
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_OMIT_TRACE) || defined(SQLITE_OMIT_DEPRECATED)
# define sqlite3_profile       0
# define sqlite3_trace         0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GET_ਸਾਰਣੀ
# define sqlite3_free_ਸਾਰਣੀ    0
# define sqlite3_get_ਸਾਰਣੀ     0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_zeroblob  0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_ਬਾਈਟਾਂ     0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_close     0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_open      0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_read      0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_write     0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_reopen    0
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_OMIT_TRACE)
# define sqlite3_trace_v2      0
#ਜੇ_ਅੰਤ

/*
** The following structure contains pointers to all SQLite API routines.
** A pointer to this structure is passed into extensions when they are
** loaded so that the extension can make calls back into the SQLite
** library.
**
** When adding new APIs, add them to the bottom of this structure
** in order to preserve backwards compatibility.
**
** Extensions that use newer APIs should first call the
** sqlite3_libversion_number() to make sure that the API they
** intend to use is supported by the library.  Extensions should
** also check to make sure that the pointer to the function is
** not NULL before calling it.
*/
ਸਥਿਰ ਅਚਲ sqlite3_api_routines sqlite3Apis = {
  sqlite3_aggregate_context,
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  sqlite3_aggregate_ਗਿਣਤੀ,
#ਨਹੀਂਤਾਂ
  0,
#ਜੇ_ਅੰਤ
  sqlite3_bind_blob,
  sqlite3_bind_double,
  sqlite3_bind_int,
  sqlite3_bind_int64,
  sqlite3_bind_null,
  sqlite3_bind_parameter_ਗਿਣਤੀ,
  sqlite3_bind_parameter_ਸੂਚਕਾਂਕ,
  sqlite3_bind_parameter_name,
  sqlite3_bind_ਪਾਠ,
  sqlite3_bind_text16,
  sqlite3_bind_ਮੁੱਲ,
  sqlite3_busy_handler,
  sqlite3_busy_timeout,
  sqlite3_changes,
  sqlite3_close,
  sqlite3_collation_needed,
  sqlite3_collation_needed16,
  sqlite3_ਕਾਲਮ_blob,
  sqlite3_ਕਾਲਮ_ਬਾਈਟਾਂ,
  sqlite3_ਕਾਲਮ_bytes16,
  sqlite3_ਕਾਲਮ_ਗਿਣਤੀ,
  sqlite3_ਕਾਲਮ_database_name,
  sqlite3_ਕਾਲਮ_database_name16,
  sqlite3_ਕਾਲਮ_decltype,
  sqlite3_ਕਾਲਮ_decltype16,
  sqlite3_ਕਾਲਮ_double,
  sqlite3_ਕਾਲਮ_int,
  sqlite3_ਕਾਲਮ_int64,
  sqlite3_ਕਾਲਮ_name,
  sqlite3_ਕਾਲਮ_name16,
  sqlite3_ਕਾਲਮ_origin_name,
  sqlite3_ਕਾਲਮ_origin_name16,
  sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name,
  sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name16,
  sqlite3_ਕਾਲਮ_ਪਾਠ,
  sqlite3_ਕਾਲਮ_text16,
  sqlite3_ਕਾਲਮ_type,
  sqlite3_ਕਾਲਮ_ਮੁੱਲ,
  sqlite3_commit_hook,
  sqlite3_complete,
  sqlite3_complete16,
  sqlite3_create_collation,
  sqlite3_create_collation16,
  sqlite3_create_function,
  sqlite3_create_function16,
  sqlite3_create_module,
  sqlite3_ਅੰਕੜੇ_ਗਿਣਤੀ,
  sqlite3_db_handle,
  sqlite3_declare_vtab,
  sqlite3_enable_shared_cache,
  sqlite3_errcode,
  sqlite3_errmsg,
  sqlite3_errmsg16,
  sqlite3_exec,
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  sqlite3_expired,
#ਨਹੀਂਤਾਂ
  0,
#ਜੇ_ਅੰਤ
  sqlite3_finalize,
  sqlite3_free,
  sqlite3_free_ਸਾਰਣੀ,
  sqlite3_get_autocommit,
  sqlite3_get_auxdata,
  sqlite3_get_ਸਾਰਣੀ,
  0,     /* Was sqlite3_global_recover(), but that function is deprecated */
  sqlite3_ਰੁਕਾਵਟ,
  sqlite3_ਆਖਰੀ_ਪਾਓ_rowid,
  sqlite3_libversion,
  sqlite3_libversion_ਸੰਖਿਆ,
  sqlite3_malloc,
  sqlite3_mprintf,
  sqlite3_open,
  sqlite3_open16,
  sqlite3_prepare,
  sqlite3_prepare16,
  sqlite3_profile,
  sqlite3_ਤਰੱਕੀ_handler,
  sqlite3_realloc,
  sqlite3_reset,
  sqlite3_ਨਤੀਜਾ_blob,
  sqlite3_ਨਤੀਜਾ_double,
  sqlite3_ਨਤੀਜਾ_error,
  sqlite3_ਨਤੀਜਾ_error16,
  sqlite3_ਨਤੀਜਾ_int,
  sqlite3_ਨਤੀਜਾ_int64,
  sqlite3_ਨਤੀਜਾ_null,
  sqlite3_ਨਤੀਜਾ_ਪਾਠ,
  sqlite3_ਨਤੀਜਾ_text16,
  sqlite3_ਨਤੀਜਾ_text16be,
  sqlite3_ਨਤੀਜਾ_text16le,
  sqlite3_ਨਤੀਜਾ_ਮੁੱਲ,
  sqlite3_rollback_hook,
  sqlite3_ਸੈੱਟ_authorizer,
  sqlite3_ਸੈੱਟ_auxdata,
  sqlite3_snprintf,
  sqlite3_ਕਦਮ,
  sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_metadata,
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup,
#ਨਹੀਂਤਾਂ
  0,
#ਜੇ_ਅੰਤ
  sqlite3_ਕੁੱਲ_changes,
  sqlite3_trace,
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  sqlite3_transfer_bindings,
#ਨਹੀਂਤਾਂ
  0,
#ਜੇ_ਅੰਤ
  sqlite3_update_hook,
  sqlite3_user_ਅੰਕੜੇ,
  sqlite3_ਮੁੱਲ_blob,
  sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ,
  sqlite3_ਮੁੱਲ_bytes16,
  sqlite3_ਮੁੱਲ_double,
  sqlite3_ਮੁੱਲ_int,
  sqlite3_ਮੁੱਲ_int64,
  sqlite3_ਮੁੱਲ_numeric_type,
  sqlite3_ਮੁੱਲ_ਪਾਠ,
  sqlite3_ਮੁੱਲ_text16,
  sqlite3_ਮੁੱਲ_text16be,
  sqlite3_ਮੁੱਲ_text16le,
  sqlite3_ਮੁੱਲ_type,
  sqlite3_vmprintf,
  /*
  ** The original API set ends here.  All extensions can call any
  ** of the APIs above provided that the pointer is not NULL.  But
  ** before calling APIs that follow, extension should check the
  ** sqlite3_libversion_number() to make sure they are dealing with
  ** a library that is new enough to support that API.
  *************************************************************************
  */
  sqlite3_overload_function,

  /*
  ** Added after 3.3.13
  */
  sqlite3_prepare_v2,
  sqlite3_prepare16_v2,
  sqlite3_clear_bindings,

  /*
  ** Added for 3.4.1
  */
  sqlite3_create_module_v2,

  /*
  ** Added for 3.5.0
  */
  sqlite3_bind_zeroblob,
  sqlite3_blob_ਬਾਈਟਾਂ,
  sqlite3_blob_close,
  sqlite3_blob_open,
  sqlite3_blob_read,
  sqlite3_blob_write,
  sqlite3_create_collation_v2,
  sqlite3_file_control,
  sqlite3_ਸਮ੍ਰਿਤੀ_highwater,
  sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MUTEX_OMIT
  0, 
  0, 
  0,
  0,
  0,
#ਨਹੀਂਤਾਂ
  sqlite3_mutex_alloc,
  sqlite3_mutex_enter,
  sqlite3_mutex_free,
  sqlite3_mutex_leave,
  sqlite3_mutex_try,
#ਜੇ_ਅੰਤ
  sqlite3_open_v2,
  sqlite3_release_ਸਮ੍ਰਿਤੀ,
  sqlite3_ਨਤੀਜਾ_error_nomem,
  sqlite3_ਨਤੀਜਾ_error_toobig,
  sqlite3_sleep,
  sqlite3_soft_ਢੇਰ_ਹੱਦਬੰਦੀ,
  sqlite3_vfs_ਲੱਭੋ,
  sqlite3_vfs_ਰਜਿਸਟਰ,
  sqlite3_vfs_unregister,

  /*
  ** Added for 3.5.8
  */
  sqlite3_threadsafe,
  sqlite3_ਨਤੀਜਾ_zeroblob,
  sqlite3_ਨਤੀਜਾ_error_ਕੋਡ,
  sqlite3_test_control,
  sqlite3_randomness,
  sqlite3_context_db_handle,

  /*
  ** Added for 3.6.0
  */
  sqlite3_extended_ਨਤੀਜਾ_codes,
  sqlite3_ਹੱਦਬੰਦੀ,
  sqlite3_ਅਗਲਾ_stmt,
  sqlite3_sql,
  sqlite3_ਸਥਿਤੀ,

  /*
  ** Added for 3.7.4
  */
  sqlite3_backup_ਅੰਤ,
  sqlite3_backup_init,
  sqlite3_backup_pagecount,
  sqlite3_backup_ਬਚਿਆ,
  sqlite3_backup_ਕਦਮ,
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPILEOPTION_DIAGS
  sqlite3_compileoption_get,
  sqlite3_compileoption_ਵਰਤਿਆ,
#ਨਹੀਂਤਾਂ
  0,
  0,
#ਜੇ_ਅੰਤ
  sqlite3_create_function_v2,
  sqlite3_db_config,
  sqlite3_db_mutex,
  sqlite3_db_ਸਥਿਤੀ,
  sqlite3_extended_errcode,
  sqlite3_log,
  sqlite3_soft_ਢੇਰ_limit64,
  sqlite3_sourceid,
  sqlite3_stmt_ਸਥਿਤੀ,
  sqlite3_strnicmp,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNLOCK_NOTIFY
  sqlite3_unlock_notify,
#ਨਹੀਂਤਾਂ
  0,
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  sqlite3_wal_autocheckpoint,
  sqlite3_wal_checkpoint,
  sqlite3_wal_hook,
#ਨਹੀਂਤਾਂ
  0,
  0,
  0,
#ਜੇ_ਅੰਤ
  sqlite3_blob_reopen,
  sqlite3_vtab_config,
  sqlite3_vtab_on_conflict,
  sqlite3_close_v2,
  sqlite3_db_filename,
  sqlite3_db_readonly,
  sqlite3_db_release_ਸਮ੍ਰਿਤੀ,
  sqlite3_errstr,
  sqlite3_stmt_busy,
  sqlite3_stmt_readonly,
  sqlite3_stricmp,
  sqlite3_uri_boolean,
  sqlite3_uri_int64,
  sqlite3_uri_parameter,
  sqlite3_vsnprintf,
  sqlite3_wal_checkpoint_v2,
  /* Version 3.8.7 and later */
  sqlite3_auto_extension,
  sqlite3_bind_blob64,
  sqlite3_bind_text64,
  sqlite3_cancel_auto_extension,
  sqlite3_ਲੱਦੋ_extension,
  sqlite3_malloc64,
  sqlite3_msize,
  sqlite3_realloc64,
  sqlite3_reset_auto_extension,
  sqlite3_ਨਤੀਜਾ_blob64,
  sqlite3_ਨਤੀਜਾ_text64,
  sqlite3_strglob,
  /* Version 3.8.11 and later */
  (sqlite3_ਮੁੱਲ*(*)(ਅਚਲ sqlite3_ਮੁੱਲ*))sqlite3_ਮੁੱਲ_dup,
  sqlite3_ਮੁੱਲ_free,
  sqlite3_ਨਤੀਜਾ_zeroblob64,
  sqlite3_bind_zeroblob64,
  /* Version 3.9.0 and later */
  sqlite3_ਮੁੱਲ_subtype,
  sqlite3_ਨਤੀਜਾ_subtype,
  /* Version 3.10.0 and later */
  sqlite3_status64,
  sqlite3_strlike,
  sqlite3_db_cacheflush,
  /* Version 3.12.0 and later */
  sqlite3_system_errno,
  /* Version 3.14.0 and later */
  sqlite3_trace_v2,
  sqlite3_expanded_sql,
  /* Version 3.18.0 and later */
  sqlite3_ਸੈੱਟ_ਆਖਰੀ_ਪਾਓ_rowid,
  /* Version 3.20.0 and later */
  sqlite3_prepare_v3,
  sqlite3_prepare16_v3,
  sqlite3_bind_ਸੂਚਕ,
  sqlite3_ਨਤੀਜਾ_ਸੂਚਕ,
  sqlite3_ਮੁੱਲ_ਸੂਚਕ,
  /* Version 3.22.0 and later */
  sqlite3_vtab_nochange,
  sqlite3_ਮੁੱਲ_nochange,
  sqlite3_vtab_collation,
  /* Version 3.24.0 and later */
  sqlite3_keyword_ਗਿਣਤੀ,
  sqlite3_keyword_name,
  sqlite3_keyword_check,
  sqlite3_str_new,
  sqlite3_str_ਅੰਤ,
  sqlite3_str_appendf,
  sqlite3_str_vappendf,
  sqlite3_str_append,
  sqlite3_str_appendall,
  sqlite3_str_appendchar,
  sqlite3_str_reset,
  sqlite3_str_errcode,
  sqlite3_str_ਲੰਮਾਈ,
  sqlite3_str_ਮੁੱਲ,
  /* Version 3.25.0 and later */
  sqlite3_create_ਖਿੜਕੀ_function,
  /* Version 3.26.0 and later */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
  sqlite3_normalized_sql,
#ਨਹੀਂਤਾਂ
  0,
#ਜੇ_ਅੰਤ
  /* Version 3.28.0 and later */
  sqlite3_stmt_isexplain,
  sqlite3_ਮੁੱਲ_frombind,
  /* Version 3.30.0 and later */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_drop_modules,
#ਨਹੀਂਤਾਂ
  0,
#ਜੇ_ਅੰਤ
  /* Version 3.31.0 and later */
  sqlite3_hard_ਢੇਰ_limit64,
  sqlite3_uri_ਕੁੰਜੀ,
  sqlite3_filename_database,
  sqlite3_filename_journal,
  sqlite3_filename_wal,
  /* Version 3.32.0 and later */
  sqlite3_create_filename,
  sqlite3_free_filename,
  sqlite3_database_file_object,
  /* Version 3.34.0 and later */
  sqlite3_txn_state,
  /* Version 3.36.1 and later */
  sqlite3_changes64,
  sqlite3_ਕੁੱਲ_changes64,
  /* Version 3.37.0 and later */
  sqlite3_autovacuum_ਪੰਨੇ,
  /* Version 3.38.0 and later */
  sqlite3_error_ਵਿੱਥ,
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_vtab_rhs_ਮੁੱਲ,
  sqlite3_vtab_distinct,
  sqlite3_vtab_ਵਿੱਚ,
  sqlite3_vtab_ਵਿੱਚ_ਪਹਿਲਾ,
  sqlite3_vtab_ਵਿੱਚ_ਅਗਲਾ,
#ਨਹੀਂਤਾਂ
  0,
  0,
  0,
  0,
  0,
#ਜੇ_ਅੰਤ
  /* Version 3.39.0 and later */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
  sqlite3_deserialize,
  sqlite3_serialize,
#ਨਹੀਂਤਾਂ
  0,
  0,
#ਜੇ_ਅੰਤ
  sqlite3_db_name,
  /* Version 3.40.0 and later */
  sqlite3_ਮੁੱਲ_encoding,
  /* Version 3.41.0 and later */
  sqlite3_is_interrupted,
  /* Version 3.43.0 and later */
  sqlite3_stmt_explain,
  /* Version 3.44.0 and later */
  sqlite3_get_clientdata,
  sqlite3_ਸੈੱਟ_clientdata,
  /* Version 3.50.0 and later */
  sqlite3_setlk_timeout,
  /* Version 3.51.0 and later */
  sqlite3_ਸੈੱਟ_errmsg,
  sqlite3_db_status64,
  /* Version 3.52.0 and later */
  sqlite3_str_truncate,
  sqlite3_str_free,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_CARRAY
  sqlite3_carray_bind,
  sqlite3_carray_bind_v2,
#ਨਹੀਂਤਾਂ
  0,
  0,
#ਜੇ_ਅੰਤ
  /* Version 3.54.0 and later */
  sqlite3_incomplete,
  sqlite3_ਨਤੀਜਾ_str
};

/* True if x is the directory separator character
*/
#ਜੇ SQLITE_OS_WIN
# define DirSep(X)  ((X)=='/'||(X)=='\\')
#ਨਹੀਂਤਾਂ
# define DirSep(X)  ((X)=='/')
#ਜੇ_ਅੰਤ

/*
** Attempt to load an SQLite extension library contained in the file
** zFile.  The entry point is zProc.  zProc may be 0 in which case a
** default entry point name (sqlite3_extension_init) is used.  Use
** of the default name is recommended.
**
** Return SQLITE_OK on success and SQLITE_ERROR if something goes wrong.
**
** If an error occurs and pzErrMsg is not 0, then fill *pzErrMsg with 
** error message text.  The calling function should free this memory
** by calling sqlite3DbFree(db, ).
*/
ਸਥਿਰ ਪੂਰਨ sqlite3_ਲੱਦੋ_Extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ਅਚਲ ਅੱਖਰ *zFile,    /* Name of the shared library containing extension */
  ਅਚਲ ਅੱਖਰ *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ       /* Put error message here if not 0 */
){
  sqlite3_vfs *pVfs = db->pVfs;
  ਖਾਲੀ *handle;
  sqlite3_loadext_ਇੰਦਰਾਜ xInit;
  ਅੱਖਰ *zErrmsg = 0;
  ਅਚਲ ਅੱਖਰ *z_ਇੰਦਰਾਜ;
  ਅੱਖਰ *z_Alt_ਇੰਦਰਾਜ = 0;
  ਖਾਲੀ **aHandle;
  u64 n_ਸੁਨੇਹਾ = ਲੜੀ_ਲੰਬਾਈ(zFile);
  ਪੂਰਨ ii;
  ਪੂਰਨ rc;

  /* Shared library endings to try if zFile cannot be loaded as written */
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azEndings[] = {
#ਜੇ SQLITE_OS_WIN
     "dll"   
#ਜਾਂਜੇ defined(__APPLE__)
     "dylib"
#ਨਹੀਂਤਾਂ
     "so"
#ਜੇ_ਅੰਤ
  };


  ਜੇ( pz_Err_ਸੁਨੇਹਾ ) *pz_Err_ਸੁਨੇਹਾ = 0;

  /* Ticket #1863.  To avoid a creating security problems for older
  ** applications that relink against newer versions of SQLite, the
  ** ability to run load_extension is turned off by default.  One
  ** must call either sqlite3_enable_load_extension(db) or
  ** sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, 0)
  ** to turn on extension loading.
  */
  ਜੇ( (db->flags & SQLITE_ਲੱਦੋ_Extension)==0 ){
    ਜੇ( pz_Err_ਸੁਨੇਹਾ ){
      *pz_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("not authorized");
    }
    ਵਾਪਸ SQLITE_ERROR;
  }

  z_ਇੰਦਰਾਜ = zProc ? zProc : "sqlite3_extension_init";

  /* tag-20210611-1.  Some dlopen() implementations will segfault if given
  ** an oversize filename.  Most filesystems have a pathname limit of 4K,
  ** so limit the extension filename length to about twice that.
  ** https://sqlite.org/forum/forumpost/08a0d6d9bf
  **
  ** Later (2023-03-25): Save an extra 6 bytes for the filename suffix.
  ** See https://sqlite.org/forum/forumpost/24083b579d.
  */
  ਜੇ( n_ਸੁਨੇਹਾ>SQLITE_MAX_PATHLEN ) ਜਾਓ extension_not_ਮਿਲਿਆ;

  /* Do not allow sqlite3_load_extension() to link to a copy of the
  ** running application, by passing in an empty filename. */
  ਜੇ( n_ਸੁਨੇਹਾ==0 ) ਜਾਓ extension_not_ਮਿਲਿਆ;
    
  handle = sqlite3OsDlOpen(pVfs, zFile);
#ਜੇ SQLITE_OS_UNIX || SQLITE_OS_WIN
  ਲਈ(ii=0; ii<Array_ਮਾਪ(azEndings) && handle==0; ii++){
    ਅੱਖਰ *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    ਜੇ( zAltFile==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    ਜੇ( n_ਸੁਨੇਹਾ+ਲੜੀ_ਲੰਬਾਈ(azEndings[ii])+1<=SQLITE_MAX_PATHLEN ){
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }
#ਜੇ_ਅੰਤ
  ਜੇ( handle==0 ) ਜਾਓ extension_not_ਮਿਲਿਆ;
  xInit = (sqlite3_loadext_ਇੰਦਰਾਜ)sqlite3OsDlSym(pVfs, handle, z_ਇੰਦਰਾਜ);

  /* If no entry point was specified and the default legacy
  ** entry point name "sqlite3_extension_init" was not found, then
  ** construct an entry point name "sqlite3_X_init" where the X is
  ** replaced by the lowercase value of every ASCII alphabetic 
  ** character in the filename after the last "/" up to the first ".",
  ** and skipping the first three characters if they are "lib".  
  ** Examples:
  **
  **    /usr/local/lib/libExample5.4.3.so ==>  sqlite3_example_init
  **    C:/lib/mathfuncs.dll              ==>  sqlite3_mathfuncs_init
  **
  ** If that still finds no entry point, repeat a second time but this
  ** time include both alphabetic and numeric characters up to the first
  ** ".".  Example:
  **
  **    /usr/local/lib/libExample5.4.3.so ==>  sqlite3_example5_init
  */
  ਜੇ( xInit==0 && zProc==0 ){
    ਪੂਰਨ iFile, i_ਇੰਦਰਾਜ, c;
    ਪੂਰਨ ncFile = sqlite3Strlen30(zFile);
    ਪੂਰਨ cnt = 0;
    z_Alt_ਇੰਦਰਾਜ = sqlite3_malloc64(ncFile+30);
    ਜੇ( z_Alt_ਇੰਦਰਾਜ==0 ){
      sqlite3OsDlClose(pVfs, handle);
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    ਕਰੋ{
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_Alt_ਇੰਦਰਾਜ, "sqlite3_", 8);
      ਲਈ(iFile=ncFile-1; iFile>=0 && !DirSep(zFile[iFile]); iFile--){}
      iFile++;
      ਜੇ( sqlite3_strnicmp(zFile+iFile, "lib", 3)==0 ) iFile += 3;
      ਲਈ(i_ਇੰਦਰਾਜ=8; (c = zFile[iFile])!=0 && c!='.'; iFile++){
        ਜੇ( sqlite3Isalpha(c) || (cnt && sqlite3Isdigit(c)) ){
          z_Alt_ਇੰਦਰਾਜ[i_ਇੰਦਰਾਜ++] = (ਅੱਖਰ)sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[(ਬਿਨਾਨਿਸ਼ਾਨੀ)c];
        }
      }
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_Alt_ਇੰਦਰਾਜ+i_ਇੰਦਰਾਜ, "_init", 6);
      z_ਇੰਦਰਾਜ = z_Alt_ਇੰਦਰਾਜ;
      xInit = (sqlite3_loadext_ਇੰਦਰਾਜ)sqlite3OsDlSym(pVfs, handle, z_ਇੰਦਰਾਜ);
    }ਜਦੋਂ( xInit==0 && (++cnt)<2 );
  }
  ਜੇ( xInit==0 ){
    ਜੇ( pz_Err_ਸੁਨੇਹਾ ){
      n_ਸੁਨੇਹਾ += ਲੜੀ_ਲੰਬਾਈ(z_ਇੰਦਰਾਜ) + 300;
      *pz_Err_ਸੁਨੇਹਾ = zErrmsg = sqlite3_malloc64(n_ਸੁਨੇਹਾ);
      ਜੇ( zErrmsg ){
        ਪ੍ਰਤਿਜ੍ਞਾ( n_ਸੁਨੇਹਾ<0x7fffffff );  /* zErrmsg would be NULL if not so */
        sqlite3_snprintf((ਪੂਰਨ)n_ਸੁਨੇਹਾ, zErrmsg,
            "no entry point [%s] in shared library [%s]", z_ਇੰਦਰਾਜ, zFile);
        sqlite3OsDlError(pVfs, n_ਸੁਨੇਹਾ-1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(z_Alt_ਇੰਦਰਾਜ);
    ਵਾਪਸ SQLITE_ERROR;
  }
  sqlite3_free(z_Alt_ਇੰਦਰਾਜ);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  ਜੇ( rc ){
    ਜੇ( rc==SQLITE_OK_ਲੱਦੋ_PERMANENTLY ) ਵਾਪਸ SQLITE_OK;
    ਜੇ( pz_Err_ਸੁਨੇਹਾ ){
      *pz_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("error during initialization: %s", zErrmsg);
    }
    sqlite3_free(zErrmsg);
    sqlite3OsDlClose(pVfs, handle);
    ਵਾਪਸ SQLITE_ERROR;
  }

  /* Append the new shared library handle to the db->aExtension array. */
  aHandle = sqlite3DbMallocZero(db, ਆਕਾਰ(handle)*(db->nExtension+1));
  ਜੇ( aHandle==0 ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਜੇ( db->nExtension>0 ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(aHandle, db->aExtension, ਆਕਾਰ(handle)*db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  ਵਾਪਸ SQLITE_OK;

extension_not_ਮਿਲਿਆ:
  ਜੇ( pz_Err_ਸੁਨੇਹਾ ){
    n_ਸੁਨੇਹਾ += 300;
    *pz_Err_ਸੁਨੇਹਾ = zErrmsg = sqlite3_malloc64(n_ਸੁਨੇਹਾ);
    ਜੇ( zErrmsg ){
      ਪ੍ਰਤਿਜ੍ਞਾ( n_ਸੁਨੇਹਾ<0x7fffffff );  /* zErrmsg would be NULL if not so */
      sqlite3_snprintf((ਪੂਰਨ)n_ਸੁਨੇਹਾ, zErrmsg,
          "unable to open shared library [%.*s]", SQLITE_MAX_PATHLEN, zFile);
      sqlite3OsDlError(pVfs, n_ਸੁਨੇਹਾ-1, zErrmsg);
    }
  }
  ਵਾਪਸ SQLITE_ERROR;
}
ਪੂਰਨ sqlite3_ਲੱਦੋ_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ਅਚਲ ਅੱਖਰ *zFile,    /* Name of the shared library containing extension */
  ਅਚਲ ਅੱਖਰ *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ       /* Put error message here if not 0 */
){
  ਪੂਰਨ rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_ਲੱਦੋ_Extension(db, zFile, zProc, pz_Err_ਸੁਨੇਹਾ);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Call this routine when the database connection is closing in order
** to clean up loaded extensions
*/
ਖਾਲੀ sqlite3CloseExtensions(sqlite3 *db){
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਲਈ(i=0; i<db->nExtension; i++){
    sqlite3OsDlClose(db->pVfs, db->aExtension[i]);
  }
  sqlite3DbFree(db, db->aExtension);
}

/*
** Enable or disable extension loading.  Extension loading is disabled by
** default so as not to open security holes in older applications.
*/
ਪੂਰਨ sqlite3_enable_ਲੱਦੋ_extension(sqlite3 *db, ਪੂਰਨ onoff){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਜੇ( onoff ){
    db->flags |= SQLITE_ਲੱਦੋ_Extension|SQLITE_ਲੱਦੋ_Ext_Func;
  }ਨਹੀਂਤਾਂ{
    db->flags &= ~(u64)(SQLITE_ਲੱਦੋ_Extension|SQLITE_ਲੱਦੋ_Ext_Func);
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_LOAD_EXTENSION) */

/*
** The following object holds the list of automatically loaded
** extensions.
**
** This list is shared across threads.  The SQLITE_MUTEX_STATIC_MAIN
** mutex must be held while accessing this list.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ sqlite3AutoExtList sqlite3AutoExtList;
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ sqlite3AutoExtList {
  u32 nExt;              /* Number of entries in aExt[] */
  ਖਾਲੀ (**aExt)(ਖਾਲੀ);   /* Pointers to the extension init functions */
} sqlite3Autoext = { 0, 0 };

/* The "wsdAutoext" macro will resolve to the autoextension
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Autoext" state vector declared above.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
# define wsdAutoextInit \
  sqlite3AutoExtList *x = &GLOBAL(sqlite3AutoExtList,sqlite3Autoext)
# define wsdAutoext x[0]
#ਨਹੀਂਤਾਂ
# define wsdAutoextInit
# define wsdAutoext sqlite3Autoext
#ਜੇ_ਅੰਤ


/*
** Register a statically linked extension that is automatically
** loaded by every new database connection.
*/
ਪੂਰਨ sqlite3_auto_extension(
  ਖਾਲੀ (*xInit)(ਖਾਲੀ)
){
  ਪੂਰਨ rc = SQLITE_OK;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( xInit==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ਜੇ( rc ){
    ਵਾਪਸ rc;
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  {
    u32 i;
#ਜੇ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ਜੇ_ਅੰਤ
    wsdAutoextInit;
    sqlite3_mutex_enter(mutex);
    ਲਈ(i=0; i<wsdAutoext.nExt; i++){
      ਜੇ( wsdAutoext.aExt[i]==xInit ) ਰੋਕੋ;
    }
    ਜੇ( i==wsdAutoext.nExt ){
      u64 nByte = (wsdAutoext.nExt+1)*ਆਕਾਰ(wsdAutoext.aExt[0]);
      ਖਾਲੀ (**aNew)(ਖਾਲੀ);
      aNew = sqlite3_realloc64(wsdAutoext.aExt, nByte);
      ਜੇ( aNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }ਨਹੀਂਤਾਂ{
        wsdAutoext.aExt = aNew;
        wsdAutoext.aExt[wsdAutoext.nExt] = xInit;
        wsdAutoext.nExt++;
      }
    }
    sqlite3_mutex_leave(mutex);
    ਪ੍ਰਤਿਜ੍ਞਾ( (rc&0xff)==rc );
    ਵਾਪਸ rc;
  }
}

/*
** Cancel a prior call to sqlite3_auto_extension.  Remove xInit from the
** set of routines that is invoked for each new database connection, if it
** is currently on the list.  If xInit is not on the list, then this
** routine is a no-op.
**
** Return 1 if xInit was found on the list and removed.  Return 0 if xInit
** was not on the list.
*/
ਪੂਰਨ sqlite3_cancel_auto_extension(
  ਖਾਲੀ (*xInit)(ਖਾਲੀ)
){
  ਪੂਰਨ n = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ){
    ਵਾਪਸ 0;
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  {
#ਜੇ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ਜੇ_ਅੰਤ
    ਪੂਰਨ i;
    wsdAutoextInit;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
    ਜੇ( xInit==0 ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
    sqlite3_mutex_enter(mutex);
    ਲਈ(i=(ਪੂਰਨ)wsdAutoext.nExt-1; i>=0; i--){
      ਜੇ( wsdAutoext.aExt[i]==xInit ){
        wsdAutoext.nExt--;
        wsdAutoext.aExt[i] = wsdAutoext.aExt[wsdAutoext.nExt];
        n++;
        ਰੋਕੋ;
      }
    }
    sqlite3_mutex_leave(mutex);
  }
  ਵਾਪਸ n;
}

/*
** Reset the automatic extension loading mechanism.
*/
ਖਾਲੀ sqlite3_reset_auto_extension(ਖਾਲੀ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize()==SQLITE_OK )
#ਜੇ_ਅੰਤ
  {
#ਜੇ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ਜੇ_ਅੰਤ
    wsdAutoextInit;
    sqlite3_mutex_enter(mutex);
    sqlite3_free(wsdAutoext.aExt);
    wsdAutoext.aExt = 0;
    wsdAutoext.nExt = 0;
    sqlite3_mutex_leave(mutex);
  }
}

/*
** Load all automatic extensions.
**
** If anything goes wrong, set an error in the database connection.
*/
ਖਾਲੀ sqlite3_Auto_ਲੱਦੋ_Extensions(sqlite3 *db){
  u32 i;
  ਪੂਰਨ go = 1;
  ਪੂਰਨ rc;
  sqlite3_loadext_ਇੰਦਰਾਜ xInit;

  wsdAutoextInit;
  ਜੇ( wsdAutoext.nExt==0 ){
    /* Common case: early out without every having to acquire a mutex */
    ਵਾਪਸ;
  }
  ਲਈ(i=0; go; i++){
    ਅੱਖਰ *zErrmsg;
#ਜੇ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
    ਅਚਲ sqlite3_api_routines *pThunk = 0;
#ਨਹੀਂਤਾਂ
    ਅਚਲ sqlite3_api_routines *pThunk = &sqlite3Apis;
#ਜੇ_ਅੰਤ
    sqlite3_mutex_enter(mutex);
    ਜੇ( i>=wsdAutoext.nExt ){
      xInit = 0;
      go = 0;
    }ਨਹੀਂਤਾਂ{
      xInit = (sqlite3_loadext_ਇੰਦਰਾਜ)wsdAutoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    ਜੇ( xInit && (rc = xInit(db, &zErrmsg, pThunk))!=0 ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(db, rc,
            "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}
