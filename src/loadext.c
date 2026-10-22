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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_CORE
  #ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ "sqlite3ext.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
/*
** Some API routines are omitted when various features are
** excluded from a build of SQLite.  Substitute a NULL pointer
** for any missing APIs.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_METADATA
# define sqlite3_ಸ್ತಂಭ_database_name   0
# define sqlite3_ಸ್ತಂಭ_database_name16 0
# define sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name      0
# define sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name16    0
# define sqlite3_ಸ್ತಂಭ_origin_name     0
# define sqlite3_ಸ್ತಂಭ_origin_name16   0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTHORIZATION
# define sqlite3_ಗಣ_authorizer         0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_UTF16
# define sqlite3_bind_text16            0
# define sqlite3_collation_needed16     0
# define sqlite3_ಸ್ತಂಭ_decltype16      0
# define sqlite3_ಸ್ತಂಭ_name16          0
# define sqlite3_ಸ್ತಂಭ_text16          0
# define sqlite3_complete16             0
# define sqlite3_create_collation16     0
# define sqlite3_create_function16      0
# define sqlite3_errmsg16               0
# define sqlite3_open16                 0
# define sqlite3_prepare16              0
# define sqlite3_prepare16_v2           0
# define sqlite3_prepare16_v3           0
# define sqlite3_ಫಲಿತಾಂಶ_error16         0
# define sqlite3_ಫಲಿತಾಂಶ_text16          0
# define sqlite3_ಫಲಿತಾಂಶ_text16be        0
# define sqlite3_ಫಲಿತಾಂಶ_text16le        0
# define sqlite3_ಮೌಲ್ಯ_text16           0
# define sqlite3_ಮೌಲ್ಯ_text16be         0
# define sqlite3_ಮೌಲ್ಯ_text16le         0
# define sqlite3_ಸ್ತಂಭ_database_name16 0
# define sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name16    0
# define sqlite3_ಸ್ತಂಭ_origin_name16   0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_COMPLETE
# define sqlite3_complete 0
# define sqlite3_complete16 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_DECLTYPE
# define sqlite3_ಸ್ತಂಭ_decltype16      0
# define sqlite3_ಸ್ತಂಭ_decltype        0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
# define sqlite3_ಪ್ರಗತಿ_handler 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_VIRTUALTABLE
# define sqlite3_create_module 0
# define sqlite3_create_module_v2 0
# define sqlite3_declare_vtab 0
# define sqlite3_vtab_config 0
# define sqlite3_vtab_on_conflict 0
# define sqlite3_vtab_collation 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_SHARED_CACHE
# define sqlite3_enable_shared_cache 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_OMIT_TRACE) || defined(SQLITE_OMIT_DEPRECATED)
# define sqlite3_profile       0
# define sqlite3_trace         0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_GET_ಕೋಷ್ಟಕ
# define sqlite3_free_ಕೋಷ್ಟಕ    0
# define sqlite3_get_ಕೋಷ್ಟಕ     0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_INCRBLOB
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_zeroblob  0
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_ಬೈಟುಗಳು     0
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_close     0
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_open      0
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_read      0
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_write     0
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_reopen    0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_OMIT_TRACE)
# define sqlite3_trace_v2      0
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_api_routines sqlite3Apis = {
  sqlite3_aggregate_context,
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  sqlite3_aggregate_ಎಣಿಕೆ,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_bind_blob,
  sqlite3_bind_double,
  sqlite3_bind_int,
  sqlite3_bind_int64,
  sqlite3_bind_null,
  sqlite3_bind_parameter_ಎಣಿಕೆ,
  sqlite3_bind_parameter_ಸೂಚ್ಯಂಕ,
  sqlite3_bind_parameter_name,
  sqlite3_bind_ಪಠ್ಯ,
  sqlite3_bind_text16,
  sqlite3_bind_ಮೌಲ್ಯ,
  sqlite3_busy_handler,
  sqlite3_busy_timeout,
  sqlite3_changes,
  sqlite3_close,
  sqlite3_collation_needed,
  sqlite3_collation_needed16,
  sqlite3_ಸ್ತಂಭ_blob,
  sqlite3_ಸ್ತಂಭ_ಬೈಟುಗಳು,
  sqlite3_ಸ್ತಂಭ_bytes16,
  sqlite3_ಸ್ತಂಭ_ಎಣಿಕೆ,
  sqlite3_ಸ್ತಂಭ_database_name,
  sqlite3_ಸ್ತಂಭ_database_name16,
  sqlite3_ಸ್ತಂಭ_decltype,
  sqlite3_ಸ್ತಂಭ_decltype16,
  sqlite3_ಸ್ತಂಭ_double,
  sqlite3_ಸ್ತಂಭ_int,
  sqlite3_ಸ್ತಂಭ_int64,
  sqlite3_ಸ್ತಂಭ_name,
  sqlite3_ಸ್ತಂಭ_name16,
  sqlite3_ಸ್ತಂಭ_origin_name,
  sqlite3_ಸ್ತಂಭ_origin_name16,
  sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name,
  sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name16,
  sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ,
  sqlite3_ಸ್ತಂಭ_text16,
  sqlite3_ಸ್ತಂಭ_type,
  sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ,
  sqlite3_commit_hook,
  sqlite3_complete,
  sqlite3_complete16,
  sqlite3_create_collation,
  sqlite3_create_collation16,
  sqlite3_create_function,
  sqlite3_create_function16,
  sqlite3_create_module,
  sqlite3_ದತ್ತಾಂಶ_ಎಣಿಕೆ,
  sqlite3_db_handle,
  sqlite3_declare_vtab,
  sqlite3_enable_shared_cache,
  sqlite3_errcode,
  sqlite3_errmsg,
  sqlite3_errmsg16,
  sqlite3_exec,
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  sqlite3_expired,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_finalize,
  sqlite3_free,
  sqlite3_free_ಕೋಷ್ಟಕ,
  sqlite3_get_autocommit,
  sqlite3_get_auxdata,
  sqlite3_get_ಕೋಷ್ಟಕ,
  0,     /* Was sqlite3_global_recover(), but that function is deprecated */
  sqlite3_ಅಡಚಣೆ,
  sqlite3_ಕೊನೆಯ_ಸೇರಿಸು_rowid,
  sqlite3_libversion,
  sqlite3_libversion_ಸಂಖ್ಯೆ,
  sqlite3_malloc,
  sqlite3_mprintf,
  sqlite3_open,
  sqlite3_open16,
  sqlite3_prepare,
  sqlite3_prepare16,
  sqlite3_profile,
  sqlite3_ಪ್ರಗತಿ_handler,
  sqlite3_realloc,
  sqlite3_reset,
  sqlite3_ಫಲಿತಾಂಶ_blob,
  sqlite3_ಫಲಿತಾಂಶ_double,
  sqlite3_ಫಲಿತಾಂಶ_error,
  sqlite3_ಫಲಿತಾಂಶ_error16,
  sqlite3_ಫಲಿತಾಂಶ_int,
  sqlite3_ಫಲಿತಾಂಶ_int64,
  sqlite3_ಫಲಿತಾಂಶ_null,
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ,
  sqlite3_ಫಲಿತಾಂಶ_text16,
  sqlite3_ಫಲಿತಾಂಶ_text16be,
  sqlite3_ಫಲಿತಾಂಶ_text16le,
  sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ,
  sqlite3_rollback_hook,
  sqlite3_ಗಣ_authorizer,
  sqlite3_ಗಣ_auxdata,
  sqlite3_snprintf,
  sqlite3_ಹೆಜ್ಜೆ,
  sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_metadata,
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_ಒಟ್ಟು_changes,
  sqlite3_trace,
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  sqlite3_transfer_bindings,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_update_hook,
  sqlite3_user_ದತ್ತಾಂಶ,
  sqlite3_ಮೌಲ್ಯ_blob,
  sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು,
  sqlite3_ಮೌಲ್ಯ_bytes16,
  sqlite3_ಮೌಲ್ಯ_double,
  sqlite3_ಮೌಲ್ಯ_int,
  sqlite3_ಮೌಲ್ಯ_int64,
  sqlite3_ಮೌಲ್ಯ_numeric_type,
  sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ,
  sqlite3_ಮೌಲ್ಯ_text16,
  sqlite3_ಮೌಲ್ಯ_text16be,
  sqlite3_ಮೌಲ್ಯ_text16le,
  sqlite3_ಮೌಲ್ಯ_type,
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
  sqlite3_blob_ಬೈಟುಗಳು,
  sqlite3_blob_close,
  sqlite3_blob_open,
  sqlite3_blob_read,
  sqlite3_blob_write,
  sqlite3_create_collation_v2,
  sqlite3_file_control,
  sqlite3_ಸ್ಮರಣೆ_highwater,
  sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MUTEX_OMIT
  0, 
  0, 
  0,
  0,
  0,
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3_mutex_alloc,
  sqlite3_mutex_enter,
  sqlite3_mutex_free,
  sqlite3_mutex_leave,
  sqlite3_mutex_try,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_open_v2,
  sqlite3_release_ಸ್ಮರಣೆ,
  sqlite3_ಫಲಿತಾಂಶ_error_nomem,
  sqlite3_ಫಲಿತಾಂಶ_error_toobig,
  sqlite3_sleep,
  sqlite3_soft_ರಾಶಿ_ಎಲ್ಲೆ,
  sqlite3_vfs_ಕಂಡುಹಿಡಿ,
  sqlite3_vfs_ರಿಜಿಸ್ಟರ್,
  sqlite3_vfs_unregister,

  /*
  ** Added for 3.5.8
  */
  sqlite3_threadsafe,
  sqlite3_ಫಲಿತಾಂಶ_zeroblob,
  sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ,
  sqlite3_test_control,
  sqlite3_randomness,
  sqlite3_context_db_handle,

  /*
  ** Added for 3.6.0
  */
  sqlite3_extended_ಫಲಿತಾಂಶ_codes,
  sqlite3_ಎಲ್ಲೆ,
  sqlite3_ಮುಂದಿನ_stmt,
  sqlite3_sql,
  sqlite3_ಸ್ಥಿತಿ,

  /*
  ** Added for 3.7.4
  */
  sqlite3_backup_ಮುಕ್ತಾಯ,
  sqlite3_backup_init,
  sqlite3_backup_pagecount,
  sqlite3_backup_ಉಳಿದ,
  sqlite3_backup_ಹೆಜ್ಜೆ,
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPILEOPTION_DIAGS
  sqlite3_compileoption_get,
  sqlite3_compileoption_ಬಳಸಿದ,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
  0,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_create_function_v2,
  sqlite3_db_config,
  sqlite3_db_mutex,
  sqlite3_db_ಸ್ಥಿತಿ,
  sqlite3_extended_errcode,
  sqlite3_log,
  sqlite3_soft_ರಾಶಿ_limit64,
  sqlite3_sourceid,
  sqlite3_stmt_ಸ್ಥಿತಿ,
  sqlite3_strnicmp,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNLOCK_NOTIFY
  sqlite3_unlock_notify,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  sqlite3_wal_autocheckpoint,
  sqlite3_wal_checkpoint,
  sqlite3_wal_hook,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
  0,
  0,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_blob_reopen,
  sqlite3_vtab_config,
  sqlite3_vtab_on_conflict,
  sqlite3_close_v2,
  sqlite3_db_filename,
  sqlite3_db_readonly,
  sqlite3_db_release_ಸ್ಮರಣೆ,
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
  sqlite3_ತುಂಬು_extension,
  sqlite3_malloc64,
  sqlite3_msize,
  sqlite3_realloc64,
  sqlite3_reset_auto_extension,
  sqlite3_ಫಲಿತಾಂಶ_blob64,
  sqlite3_ಫಲಿತಾಂಶ_text64,
  sqlite3_strglob,
  /* Version 3.8.11 and later */
  (sqlite3_ಮೌಲ್ಯ*(*)(ಸ್ಥಿರಾಂಕ sqlite3_ಮೌಲ್ಯ*))sqlite3_ಮೌಲ್ಯ_dup,
  sqlite3_ಮೌಲ್ಯ_free,
  sqlite3_ಫಲಿತಾಂಶ_zeroblob64,
  sqlite3_bind_zeroblob64,
  /* Version 3.9.0 and later */
  sqlite3_ಮೌಲ್ಯ_subtype,
  sqlite3_ಫಲಿತಾಂಶ_subtype,
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
  sqlite3_ಗಣ_ಕೊನೆಯ_ಸೇರಿಸು_rowid,
  /* Version 3.20.0 and later */
  sqlite3_prepare_v3,
  sqlite3_prepare16_v3,
  sqlite3_bind_ಸೂಚಕ,
  sqlite3_ಫಲಿತಾಂಶ_ಸೂಚಕ,
  sqlite3_ಮೌಲ್ಯ_ಸೂಚಕ,
  /* Version 3.22.0 and later */
  sqlite3_vtab_nochange,
  sqlite3_ಮೌಲ್ಯ_nochange,
  sqlite3_vtab_collation,
  /* Version 3.24.0 and later */
  sqlite3_keyword_ಎಣಿಕೆ,
  sqlite3_keyword_name,
  sqlite3_keyword_check,
  sqlite3_str_new,
  sqlite3_str_ಮುಕ್ತಾಯ,
  sqlite3_str_appendf,
  sqlite3_str_vappendf,
  sqlite3_str_append,
  sqlite3_str_appendall,
  sqlite3_str_appendchar,
  sqlite3_str_reset,
  sqlite3_str_errcode,
  sqlite3_str_ಉದ್ದಳತೆ,
  sqlite3_str_ಮೌಲ್ಯ,
  /* Version 3.25.0 and later */
  sqlite3_create_ಕಿಟಕಿ_function,
  /* Version 3.26.0 and later */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
  sqlite3_normalized_sql,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
#ಆದರೆ_ಅಂತ್ಯ
  /* Version 3.28.0 and later */
  sqlite3_stmt_isexplain,
  sqlite3_ಮೌಲ್ಯ_frombind,
  /* Version 3.30.0 and later */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_drop_modules,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
#ಆದರೆ_ಅಂತ್ಯ
  /* Version 3.31.0 and later */
  sqlite3_hard_ರಾಶಿ_limit64,
  sqlite3_uri_ಕೀಲಿ,
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
  sqlite3_ಒಟ್ಟು_changes64,
  /* Version 3.37.0 and later */
  sqlite3_autovacuum_ಪುಟಗಳು,
  /* Version 3.38.0 and later */
  sqlite3_error_ಸ್ಥಾನಾಂತರ,
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_vtab_rhs_ಮೌಲ್ಯ,
  sqlite3_vtab_distinct,
  sqlite3_vtab_ಒಳ,
  sqlite3_vtab_ಒಳ_ಮೊದಲ,
  sqlite3_vtab_ಒಳ_ಮುಂದಿನ,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
  0,
  0,
  0,
  0,
#ಆದರೆ_ಅಂತ್ಯ
  /* Version 3.39.0 and later */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DESERIALIZE
  sqlite3_deserialize,
  sqlite3_serialize,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
  0,
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_db_name,
  /* Version 3.40.0 and later */
  sqlite3_ಮೌಲ್ಯ_encoding,
  /* Version 3.41.0 and later */
  sqlite3_is_interrupted,
  /* Version 3.43.0 and later */
  sqlite3_stmt_explain,
  /* Version 3.44.0 and later */
  sqlite3_get_clientdata,
  sqlite3_ಗಣ_clientdata,
  /* Version 3.50.0 and later */
  sqlite3_setlk_timeout,
  /* Version 3.51.0 and later */
  sqlite3_ಗಣ_errmsg,
  sqlite3_db_status64,
  /* Version 3.52.0 and later */
  sqlite3_str_truncate,
  sqlite3_str_free,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_CARRAY
  sqlite3_carray_bind,
  sqlite3_carray_bind_v2,
#ಇಲ್ಲದಿದ್ದರೆ
  0,
  0,
#ಆದರೆ_ಅಂತ್ಯ
  /* Version 3.54.0 and later */
  sqlite3_incomplete,
  sqlite3_ಫಲಿತಾಂಶ_str
};

/* True if x is the directory separator character
*/
#ಆದರೆ SQLITE_OS_WIN
# define DirSep(X)  ((X)=='/'||(X)=='\\')
#ಇಲ್ಲದಿದ್ದರೆ
# define DirSep(X)  ((X)=='/')
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_ತುಂಬು_Extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile,    /* Name of the shared library containing extension */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ       /* Put error message here if not 0 */
){
  sqlite3_vfs *pVfs = db->pVfs;
  ಶೂನ್ಯ *handle;
  sqlite3_loadext_ನಮೂದು xInit;
  ಅಕ್ಷರ *zErrmsg = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ನಮೂದು;
  ಅಕ್ಷರ *z_Alt_ನಮೂದು = 0;
  ಶೂನ್ಯ **aHandle;
  u64 n_ಸಂದೇಶ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zFile);
  ಪೂರ್ಣಾಂಕ ii;
  ಪೂರ್ಣಾಂಕ rc;

  /* Shared library endings to try if zFile cannot be loaded as written */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azEndings[] = {
#ಆದರೆ SQLITE_OS_WIN
     "dll"   
#ಅಥವಾಆದರೆ defined(__APPLE__)
     "dylib"
#ಇಲ್ಲದಿದ್ದರೆ
     "so"
#ಆದರೆ_ಅಂತ್ಯ
  };


  ಆದರೆ( pz_Err_ಸಂದೇಶ ) *pz_Err_ಸಂದೇಶ = 0;

  /* Ticket #1863.  To avoid a creating security problems for older
  ** applications that relink against newer versions of SQLite, the
  ** ability to run load_extension is turned off by default.  One
  ** must call either sqlite3_enable_load_extension(db) or
  ** sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, 0)
  ** to turn on extension loading.
  */
  ಆದರೆ( (db->flags & SQLITE_ತುಂಬು_Extension)==0 ){
    ಆದರೆ( pz_Err_ಸಂದೇಶ ){
      *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("not authorized");
    }
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  z_ನಮೂದು = zProc ? zProc : "sqlite3_extension_init";

  /* tag-20210611-1.  Some dlopen() implementations will segfault if given
  ** an oversize filename.  Most filesystems have a pathname limit of 4K,
  ** so limit the extension filename length to about twice that.
  ** https://sqlite.org/forum/forumpost/08a0d6d9bf
  **
  ** Later (2023-03-25): Save an extra 6 bytes for the filename suffix.
  ** See https://sqlite.org/forum/forumpost/24083b579d.
  */
  ಆದರೆ( n_ಸಂದೇಶ>SQLITE_MAX_PATHLEN ) ಹೋಗು extension_not_ಸಿಕ್ಕಿತು;

  /* Do not allow sqlite3_load_extension() to link to a copy of the
  ** running application, by passing in an empty filename. */
  ಆದರೆ( n_ಸಂದೇಶ==0 ) ಹೋಗು extension_not_ಸಿಕ್ಕಿತು;
    
  handle = sqlite3OsDlOpen(pVfs, zFile);
#ಆದರೆ SQLITE_OS_UNIX || SQLITE_OS_WIN
  ಗಾಗಿ(ii=0; ii<Array_ಗಾತ್ರ(azEndings) && handle==0; ii++){
    ಅಕ್ಷರ *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    ಆದರೆ( zAltFile==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    ಆದರೆ( n_ಸಂದೇಶ+ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(azEndings[ii])+1<=SQLITE_MAX_PATHLEN ){
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( handle==0 ) ಹೋಗು extension_not_ಸಿಕ್ಕಿತು;
  xInit = (sqlite3_loadext_ನಮೂದು)sqlite3OsDlSym(pVfs, handle, z_ನಮೂದು);

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
  ಆದರೆ( xInit==0 && zProc==0 ){
    ಪೂರ್ಣಾಂಕ iFile, i_ನಮೂದು, c;
    ಪೂರ್ಣಾಂಕ ncFile = sqlite3Strlen30(zFile);
    ಪೂರ್ಣಾಂಕ cnt = 0;
    z_Alt_ನಮೂದು = sqlite3_malloc64(ncFile+30);
    ಆದರೆ( z_Alt_ನಮೂದು==0 ){
      sqlite3OsDlClose(pVfs, handle);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    ಮಾಡು{
      ಸ್ಮರಣೆ_ನಕಲು(z_Alt_ನಮೂದು, "sqlite3_", 8);
      ಗಾಗಿ(iFile=ncFile-1; iFile>=0 && !DirSep(zFile[iFile]); iFile--){}
      iFile++;
      ಆದರೆ( sqlite3_strnicmp(zFile+iFile, "lib", 3)==0 ) iFile += 3;
      ಗಾಗಿ(i_ನಮೂದು=8; (c = zFile[iFile])!=0 && c!='.'; iFile++){
        ಆದರೆ( sqlite3Isalpha(c) || (cnt && sqlite3Isdigit(c)) ){
          z_Alt_ನಮೂದು[i_ನಮೂದು++] = (ಅಕ್ಷರ)sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[(ಚಿಹ್ನೆರಹಿತ)c];
        }
      }
      ಸ್ಮರಣೆ_ನಕಲು(z_Alt_ನಮೂದು+i_ನಮೂದು, "_init", 6);
      z_ನಮೂದು = z_Alt_ನಮೂದು;
      xInit = (sqlite3_loadext_ನಮೂದು)sqlite3OsDlSym(pVfs, handle, z_ನಮೂದು);
    }ಆಗ( xInit==0 && (++cnt)<2 );
  }
  ಆದರೆ( xInit==0 ){
    ಆದರೆ( pz_Err_ಸಂದೇಶ ){
      n_ಸಂದೇಶ += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ನಮೂದು) + 300;
      *pz_Err_ಸಂದೇಶ = zErrmsg = sqlite3_malloc64(n_ಸಂದೇಶ);
      ಆದರೆ( zErrmsg ){
        ಪ್ರತಿಜ್ಞಾ( n_ಸಂದೇಶ<0x7fffffff );  /* zErrmsg would be NULL if not so */
        sqlite3_snprintf((ಪೂರ್ಣಾಂಕ)n_ಸಂದೇಶ, zErrmsg,
            "no entry point [%s] in shared library [%s]", z_ನಮೂದು, zFile);
        sqlite3OsDlError(pVfs, n_ಸಂದೇಶ-1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(z_Alt_ನಮೂದು);
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  sqlite3_free(z_Alt_ನಮೂದು);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  ಆದರೆ( rc ){
    ಆದರೆ( rc==SQLITE_OK_ತುಂಬು_PERMANENTLY ) ಹಿಂತಿರುಗಿ SQLITE_OK;
    ಆದರೆ( pz_Err_ಸಂದೇಶ ){
      *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("error during initialization: %s", zErrmsg);
    }
    sqlite3_free(zErrmsg);
    sqlite3OsDlClose(pVfs, handle);
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  /* Append the new shared library handle to the db->aExtension array. */
  aHandle = sqlite3DbMallocZero(db, ಗಾತ್ರ(handle)*(db->nExtension+1));
  ಆದರೆ( aHandle==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  ಆದರೆ( db->nExtension>0 ){
    ಸ್ಮರಣೆ_ನಕಲು(aHandle, db->aExtension, ಗಾತ್ರ(handle)*db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  ಹಿಂತಿರುಗಿ SQLITE_OK;

extension_not_ಸಿಕ್ಕಿತು:
  ಆದರೆ( pz_Err_ಸಂದೇಶ ){
    n_ಸಂದೇಶ += 300;
    *pz_Err_ಸಂದೇಶ = zErrmsg = sqlite3_malloc64(n_ಸಂದೇಶ);
    ಆದರೆ( zErrmsg ){
      ಪ್ರತಿಜ್ಞಾ( n_ಸಂದೇಶ<0x7fffffff );  /* zErrmsg would be NULL if not so */
      sqlite3_snprintf((ಪೂರ್ಣಾಂಕ)n_ಸಂದೇಶ, zErrmsg,
          "unable to open shared library [%.*s]", SQLITE_MAX_PATHLEN, zFile);
      sqlite3OsDlError(pVfs, n_ಸಂದೇಶ-1, zErrmsg);
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_ERROR;
}
ಪೂರ್ಣಾಂಕ sqlite3_ತುಂಬು_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile,    /* Name of the shared library containing extension */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ       /* Put error message here if not 0 */
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_ತುಂಬು_Extension(db, zFile, zProc, pz_Err_ಸಂದೇಶ);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Call this routine when the database connection is closing in order
** to clean up loaded extensions
*/
ಶೂನ್ಯ sqlite3CloseExtensions(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಗಾಗಿ(i=0; i<db->nExtension; i++){
    sqlite3OsDlClose(db->pVfs, db->aExtension[i]);
  }
  sqlite3DbFree(db, db->aExtension);
}

/*
** Enable or disable extension loading.  Extension loading is disabled by
** default so as not to open security holes in older applications.
*/
ಪೂರ್ಣಾಂಕ sqlite3_enable_ತುಂಬು_extension(sqlite3 *db, ಪೂರ್ಣಾಂಕ onoff){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( onoff ){
    db->flags |= SQLITE_ತುಂಬು_Extension|SQLITE_ತುಂಬು_Ext_Func;
  }ಇಲ್ಲದಿದ್ದರೆ{
    db->flags &= ~(u64)(SQLITE_ತುಂಬು_Extension|SQLITE_ತುಂಬು_Ext_Func);
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_LOAD_EXTENSION) */

/*
** The following object holds the list of automatically loaded
** extensions.
**
** This list is shared across threads.  The SQLITE_MUTEX_STATIC_MAIN
** mutex must be held while accessing this list.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ sqlite3AutoExtList sqlite3AutoExtList;
ಸ್ಥಿರ SQLITE_WSD ರಚನೆ sqlite3AutoExtList {
  u32 nExt;              /* Number of entries in aExt[] */
  ಶೂನ್ಯ (**aExt)(ಶೂನ್ಯ);   /* Pointers to the extension init functions */
} sqlite3Autoext = { 0, 0 };

/* The "wsdAutoext" macro will resolve to the autoextension
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Autoext" state vector declared above.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WSD
# define wsdAutoextInit \
  sqlite3AutoExtList *x = &GLOBAL(sqlite3AutoExtList,sqlite3Autoext)
# define wsdAutoext x[0]
#ಇಲ್ಲದಿದ್ದರೆ
# define wsdAutoextInit
# define wsdAutoext sqlite3Autoext
#ಆದರೆ_ಅಂತ್ಯ


/*
** Register a statically linked extension that is automatically
** loaded by every new database connection.
*/
ಪೂರ್ಣಾಂಕ sqlite3_auto_extension(
  ಶೂನ್ಯ (*xInit)(ಶೂನ್ಯ)
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( xInit==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ಆದರೆ( rc ){
    ಹಿಂತಿರುಗಿ rc;
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  {
    u32 i;
#ಆದರೆ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ಆದರೆ_ಅಂತ್ಯ
    wsdAutoextInit;
    sqlite3_mutex_enter(mutex);
    ಗಾಗಿ(i=0; i<wsdAutoext.nExt; i++){
      ಆದರೆ( wsdAutoext.aExt[i]==xInit ) ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( i==wsdAutoext.nExt ){
      u64 nByte = (wsdAutoext.nExt+1)*ಗಾತ್ರ(wsdAutoext.aExt[0]);
      ಶೂನ್ಯ (**aNew)(ಶೂನ್ಯ);
      aNew = sqlite3_realloc64(wsdAutoext.aExt, nByte);
      ಆದರೆ( aNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }ಇಲ್ಲದಿದ್ದರೆ{
        wsdAutoext.aExt = aNew;
        wsdAutoext.aExt[wsdAutoext.nExt] = xInit;
        wsdAutoext.nExt++;
      }
    }
    sqlite3_mutex_leave(mutex);
    ಪ್ರತಿಜ್ಞಾ( (rc&0xff)==rc );
    ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3_cancel_auto_extension(
  ಶೂನ್ಯ (*xInit)(ಶೂನ್ಯ)
){
  ಪೂರ್ಣಾಂಕ n = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ){
    ಹಿಂತಿರುಗಿ 0;
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  {
#ಆದರೆ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ಆದರೆ_ಅಂತ್ಯ
    ಪೂರ್ಣಾಂಕ i;
    wsdAutoextInit;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
    ಆದರೆ( xInit==0 ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_mutex_enter(mutex);
    ಗಾಗಿ(i=(ಪೂರ್ಣಾಂಕ)wsdAutoext.nExt-1; i>=0; i--){
      ಆದರೆ( wsdAutoext.aExt[i]==xInit ){
        wsdAutoext.nExt--;
        wsdAutoext.aExt[i] = wsdAutoext.aExt[wsdAutoext.nExt];
        n++;
        ನಿಲ್ಲಿಸು;
      }
    }
    sqlite3_mutex_leave(mutex);
  }
  ಹಿಂತಿರುಗಿ n;
}

/*
** Reset the automatic extension loading mechanism.
*/
ಶೂನ್ಯ sqlite3_reset_auto_extension(ಶೂನ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize()==SQLITE_OK )
#ಆದರೆ_ಅಂತ್ಯ
  {
#ಆದರೆ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ಆದರೆ_ಅಂತ್ಯ
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
ಶೂನ್ಯ sqlite3_Auto_ತುಂಬು_Extensions(sqlite3 *db){
  u32 i;
  ಪೂರ್ಣಾಂಕ go = 1;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_loadext_ನಮೂದು xInit;

  wsdAutoextInit;
  ಆದರೆ( wsdAutoext.nExt==0 ){
    /* Common case: early out without every having to acquire a mutex */
    ಹಿಂತಿರುಗಿ;
  }
  ಗಾಗಿ(i=0; go; i++){
    ಅಕ್ಷರ *zErrmsg;
#ಆದರೆ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
    ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pThunk = 0;
#ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pThunk = &sqlite3Apis;
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_mutex_enter(mutex);
    ಆದರೆ( i>=wsdAutoext.nExt ){
      xInit = 0;
      go = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      xInit = (sqlite3_loadext_ನಮೂದು)wsdAutoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    ಆದರೆ( xInit && (rc = xInit(db, &zErrmsg, pThunk))!=0 ){
      sqlite3_Error_With_ಸಂದೇಶ(db, rc,
            "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}
