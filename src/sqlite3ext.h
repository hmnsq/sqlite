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
#include "sqlite3.h"

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
struct sqlite3_api_routines {
  void * (*aggregate_context)(sqlite3_context*,정수 nBytes);
  정수  (*aggregate_count)(sqlite3_context*);
  정수  (*bind_blob)(sqlite3_stmt*,정수,const void*,정수 n,void(*)(void*));
  정수  (*bind_double)(sqlite3_stmt*,정수,double);
  정수  (*bind_int)(sqlite3_stmt*,정수,정수);
  정수  (*bind_int64)(sqlite3_stmt*,정수,sqlite_int64);
  정수  (*bind_null)(sqlite3_stmt*,정수);
  정수  (*bind_parameter_count)(sqlite3_stmt*);
  정수  (*bind_parameter_index)(sqlite3_stmt*,const char*zName);
  const char * (*bind_parameter_name)(sqlite3_stmt*,정수);
  정수  (*bind_text)(sqlite3_stmt*,정수,const char*,정수 n,void(*)(void*));
  정수  (*bind_text16)(sqlite3_stmt*,정수,const void*,정수,void(*)(void*));
  정수  (*bind_value)(sqlite3_stmt*,정수,const sqlite3_value*);
  정수  (*busy_handler)(sqlite3*,정수(*)(void*,정수),void*);
  정수  (*busy_timeout)(sqlite3*,정수 ms);
  정수  (*changes)(sqlite3*);
  정수  (*close)(sqlite3*);
  정수  (*collation_needed)(sqlite3*,void*,void(*)(void*,sqlite3*,
                           정수 eTextRep,const char*));
  정수  (*collation_needed16)(sqlite3*,void*,void(*)(void*,sqlite3*,
                             정수 eTextRep,const void*));
  const void * (*column_blob)(sqlite3_stmt*,정수 iCol);
  정수  (*column_bytes)(sqlite3_stmt*,정수 iCol);
  정수  (*column_bytes16)(sqlite3_stmt*,정수 iCol);
  정수  (*column_count)(sqlite3_stmt*pStmt);
  const char * (*column_database_name)(sqlite3_stmt*,정수);
  const void * (*column_database_name16)(sqlite3_stmt*,정수);
  const char * (*column_decltype)(sqlite3_stmt*,정수 i);
  const void * (*column_decltype16)(sqlite3_stmt*,정수);
  double  (*column_double)(sqlite3_stmt*,정수 iCol);
  정수  (*column_int)(sqlite3_stmt*,정수 iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,정수 iCol);
  const char * (*column_name)(sqlite3_stmt*,정수);
  const void * (*column_name16)(sqlite3_stmt*,정수);
  const char * (*column_origin_name)(sqlite3_stmt*,정수);
  const void * (*column_origin_name16)(sqlite3_stmt*,정수);
  const char * (*column_table_name)(sqlite3_stmt*,정수);
  const void * (*column_table_name16)(sqlite3_stmt*,정수);
  const unsigned char * (*column_text)(sqlite3_stmt*,정수 iCol);
  const void * (*column_text16)(sqlite3_stmt*,정수 iCol);
  정수  (*column_type)(sqlite3_stmt*,정수 iCol);
  sqlite3_value* (*column_value)(sqlite3_stmt*,정수 iCol);
  void * (*commit_hook)(sqlite3*,정수(*)(void*),void*);
  정수  (*complete)(const char*sql);
  정수  (*complete16)(const void*sql);
  정수  (*create_collation)(sqlite3*,const char*,정수,void*,
                           정수(*)(void*,정수,const void*,정수,const void*));
  정수  (*create_collation16)(sqlite3*,const void*,정수,void*,
                             정수(*)(void*,정수,const void*,정수,const void*));
  정수  (*create_function)(sqlite3*,const char*,정수,정수,void*,
                          void (*xFunc)(sqlite3_context*,정수,sqlite3_value**),
                          void (*xStep)(sqlite3_context*,정수,sqlite3_value**),
                          void (*xFinal)(sqlite3_context*));
  정수  (*create_function16)(sqlite3*,const void*,정수,정수,void*,
                            void (*xFunc)(sqlite3_context*,정수,sqlite3_value**),
                            void (*xStep)(sqlite3_context*,정수,sqlite3_value**),
                            void (*xFinal)(sqlite3_context*));
  정수 (*create_module)(sqlite3*,const char*,const sqlite3_module*,void*);
  정수  (*data_count)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  정수 (*declare_vtab)(sqlite3*,const char*);
  정수  (*enable_shared_cache)(정수);
  정수  (*errcode)(sqlite3*db);
  const char * (*errmsg)(sqlite3*);
  const void * (*errmsg16)(sqlite3*);
  정수  (*exec)(sqlite3*,const char*,sqlite3_callback,void*,char**);
  정수  (*expired)(sqlite3_stmt*);
  정수  (*finalize)(sqlite3_stmt*pStmt);
  void  (*free)(void*);
  void  (*free_table)(char**result);
  정수  (*get_autocommit)(sqlite3*);
  void * (*get_auxdata)(sqlite3_context*,정수);
  정수  (*get_table)(sqlite3*,const char*,char***,정수*,정수*,char**);
  정수  (*global_recover)(void);
  void  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_insert_rowid)(sqlite3*);
  const char * (*libversion)(void);
  정수  (*libversion_number)(void);
  void *(*malloc)(정수);
  char *(SQLITE_CDECL*mprintf)(const char*,...);
  정수  (*open)(const char*,sqlite3**);
  정수  (*open16)(const void*,sqlite3**);
  정수  (*prepare)(sqlite3*,const char*,정수,sqlite3_stmt**,const char**);
  정수  (*prepare16)(sqlite3*,const void*,정수,sqlite3_stmt**,const void**);
  void * (*profile)(sqlite3*,void(*)(void*,const char*,sqlite_uint64),void*);
  void  (*progress_handler)(sqlite3*,정수,정수(*)(void*),void*);
  void *(*realloc)(void*,정수);
  정수  (*reset)(sqlite3_stmt*pStmt);
  void  (*result_blob)(sqlite3_context*,const void*,정수,void(*)(void*));
  void  (*result_double)(sqlite3_context*,double);
  void  (*result_error)(sqlite3_context*,const char*,정수);
  void  (*result_error16)(sqlite3_context*,const void*,정수);
  void  (*result_int)(sqlite3_context*,정수);
  void  (*result_int64)(sqlite3_context*,sqlite_int64);
  void  (*result_null)(sqlite3_context*);
  void  (*result_text)(sqlite3_context*,const char*,정수,void(*)(void*));
  void  (*result_text16)(sqlite3_context*,const void*,정수,void(*)(void*));
  void  (*result_text16be)(sqlite3_context*,const void*,정수,void(*)(void*));
  void  (*result_text16le)(sqlite3_context*,const void*,정수,void(*)(void*));
  void  (*result_value)(sqlite3_context*,sqlite3_value*);
  void *(*rollback_hook)(sqlite3*,void(*)(void*),void*);
  정수  (*set_authorizer)(sqlite3*,정수(*)(void*,정수,const char*,const char*,
                         const char*,const char*),void*);
  void  (*set_auxdata)(sqlite3_context*,정수,void*,void (*)(void*));
  char *(SQLITE_CDECL*xsnprintf)(정수,char*,const char*,...);
  정수  (*step)(sqlite3_stmt*);
  정수  (*table_column_metadata)(sqlite3*,const char*,const char*,const char*,
                                char const**,char const**,정수*,정수*,정수*);
  void  (*thread_cleanup)(void);
  정수  (*total_changes)(sqlite3*);
  void * (*trace)(sqlite3*,void(*xTrace)(void*,const char*),void*);
  정수  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  void * (*update_hook)(sqlite3*,void(*)(void*,정수 ,char const*,char const*,
                                         sqlite_int64),void*);
  void * (*user_data)(sqlite3_context*);
  const void * (*value_blob)(sqlite3_value*);
  정수  (*value_bytes)(sqlite3_value*);
  정수  (*value_bytes16)(sqlite3_value*);
  double  (*value_double)(sqlite3_value*);
  정수  (*value_int)(sqlite3_value*);
  sqlite_int64  (*value_int64)(sqlite3_value*);
  정수  (*value_numeric_type)(sqlite3_value*);
  const unsigned char * (*value_text)(sqlite3_value*);
  const void * (*value_text16)(sqlite3_value*);
  const void * (*value_text16be)(sqlite3_value*);
  const void * (*value_text16le)(sqlite3_value*);
  정수  (*value_type)(sqlite3_value*);
  char *(*vmprintf)(const char*,va_list);
  /* Added ??? */
  정수 (*overload_function)(sqlite3*, const char *zFuncName, 정수 nArg);
  /* Added by 3.3.13 */
  정수 (*prepare_v2)(sqlite3*,const char*,정수,sqlite3_stmt**,const char**);
  정수 (*prepare16_v2)(sqlite3*,const void*,정수,sqlite3_stmt**,const void**);
  정수 (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  정수 (*create_module_v2)(sqlite3*,const char*,const sqlite3_module*,void*,
                          void (*xDestroy)(void *));
  /* Added by 3.5.0 */
  정수 (*bind_zeroblob)(sqlite3_stmt*,정수,정수);
  정수 (*blob_bytes)(sqlite3_blob*);
  정수 (*blob_close)(sqlite3_blob*);
  정수 (*blob_open)(sqlite3*,const char*,const char*,const char*,sqlite3_int64,
                   정수,sqlite3_blob**);
  정수 (*blob_read)(sqlite3_blob*,void*,정수,정수);
  정수 (*blob_write)(sqlite3_blob*,const void*,정수,정수);
  정수 (*create_collation_v2)(sqlite3*,const char*,정수,void*,
                             정수(*)(void*,정수,const void*,정수,const void*),
                             void(*)(void*));
  정수 (*file_control)(sqlite3*,const char*,정수,void*);
  sqlite3_int64 (*memory_highwater)(정수);
  sqlite3_int64 (*memory_used)(void);
  sqlite3_mutex *(*mutex_alloc)(정수);
  void (*mutex_enter)(sqlite3_mutex*);
  void (*mutex_free)(sqlite3_mutex*);
  void (*mutex_leave)(sqlite3_mutex*);
  정수 (*mutex_try)(sqlite3_mutex*);
  정수 (*open_v2)(const char*,sqlite3**,정수,const char*);
  정수 (*release_memory)(정수);
  void (*result_error_nomem)(sqlite3_context*);
  void (*result_error_toobig)(sqlite3_context*);
  정수 (*sleep)(정수);
  void (*soft_heap_limit)(정수);
  sqlite3_vfs *(*vfs_find)(const char*);
  정수 (*vfs_register)(sqlite3_vfs*,정수);
  정수 (*vfs_unregister)(sqlite3_vfs*);
  정수 (*xthreadsafe)(void);
  void (*result_zeroblob)(sqlite3_context*,정수);
  void (*result_error_code)(sqlite3_context*,정수);
  정수 (SQLITE_CDECL*test_control)(정수, ...);
  void (*randomness)(정수,void*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  정수 (*extended_result_codes)(sqlite3*,정수);
  정수 (*limit)(sqlite3*,정수,정수);
  sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
  const char *(*sql)(sqlite3_stmt*);
  정수 (*status)(정수,정수*,정수*,정수);
  정수 (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,const char*,sqlite3*,const char*);
  정수 (*backup_pagecount)(sqlite3_backup*);
  정수 (*backup_remaining)(sqlite3_backup*);
  정수 (*backup_step)(sqlite3_backup*,정수);
  const char *(*compileoption_get)(정수);
  정수 (*compileoption_used)(const char*);
  정수 (*create_function_v2)(sqlite3*,const char*,정수,정수,void*,
                            void (*xFunc)(sqlite3_context*,정수,sqlite3_value**),
                            void (*xStep)(sqlite3_context*,정수,sqlite3_value**),
                            void (*xFinal)(sqlite3_context*),
                            void(*xDestroy)(void*));
  정수 (SQLITE_CDECL*db_config)(sqlite3*,정수,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  정수 (*db_status)(sqlite3*,정수,정수*,정수*,정수);
  정수 (*extended_errcode)(sqlite3*);
  void (SQLITE_CDECL*log)(정수,const char*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  const char *(*sourceid)(void);
  정수 (*stmt_status)(sqlite3_stmt*,정수,정수);
  정수 (*strnicmp)(const char*,const char*,정수);
  정수 (*unlock_notify)(sqlite3*,void(*)(void**,정수),void*);
  정수 (*wal_autocheckpoint)(sqlite3*,정수);
  정수 (*wal_checkpoint)(sqlite3*,const char*);
  void *(*wal_hook)(sqlite3*,정수(*)(void*,sqlite3*,const char*,정수),void*);
  정수 (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  정수 (SQLITE_CDECL*vtab_config)(sqlite3*,정수 op,...);
  정수 (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  정수 (*close_v2)(sqlite3*);
  const char *(*db_filename)(sqlite3*,const char*);
  정수 (*db_readonly)(sqlite3*,const char*);
  정수 (*db_release_memory)(sqlite3*);
  const char *(*errstr)(정수);
  정수 (*stmt_busy)(sqlite3_stmt*);
  정수 (*stmt_readonly)(sqlite3_stmt*);
  정수 (*stricmp)(const char*,const char*);
  정수 (*uri_boolean)(const char*,const char*,정수);
  sqlite3_int64 (*uri_int64)(const char*,const char*,sqlite3_int64);
  const char *(*uri_parameter)(const char*,const char*);
  char *(*xvsnprintf)(정수,char*,const char*,va_list);
  정수 (*wal_checkpoint_v2)(sqlite3*,const char*,정수,정수*,정수*);
  /* Version 3.8.7 and later */
  정수 (*auto_extension)(void(*)(void));
  정수 (*bind_blob64)(sqlite3_stmt*,정수,const void*,sqlite3_uint64,
                     void(*)(void*));
  정수 (*bind_text64)(sqlite3_stmt*,정수,const char*,sqlite3_uint64,
                      void(*)(void*),unsigned char);
  정수 (*cancel_auto_extension)(void(*)(void));
  정수 (*load_extension)(sqlite3*,const char*,const char*,char**);
  void *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(void*);
  void *(*realloc64)(void*,sqlite3_uint64);
  void (*reset_auto_extension)(void);
  void (*result_blob64)(sqlite3_context*,const void*,sqlite3_uint64,
                        void(*)(void*));
  void (*result_text64)(sqlite3_context*,const char*,sqlite3_uint64,
                         void(*)(void*), unsigned char);
  정수 (*strglob)(const char*,const char*);
  /* Version 3.8.11 and later */
  sqlite3_value *(*value_dup)(const sqlite3_value*);
  void (*value_free)(sqlite3_value*);
  정수 (*result_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  정수 (*bind_zeroblob64)(sqlite3_stmt*, 정수, sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned 정수 (*value_subtype)(sqlite3_value*);
  void (*result_subtype)(sqlite3_context*,unsigned 정수);
  /* Version 3.10.0 and later */
  정수 (*status64)(정수,sqlite3_int64*,sqlite3_int64*,정수);
  정수 (*strlike)(const char*,const char*,unsigned 정수);
  정수 (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  정수 (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  정수 (*trace_v2)(sqlite3*,unsigned,정수(*)(unsigned,void*,void*,void*),void*);
  char *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  void (*set_last_insert_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  정수 (*prepare_v3)(sqlite3*,const char*,정수,unsigned 정수,
                    sqlite3_stmt**,const char**);
  정수 (*prepare16_v3)(sqlite3*,const void*,정수,unsigned 정수,
                      sqlite3_stmt**,const void**);
  정수 (*bind_pointer)(sqlite3_stmt*,정수,void*,const char*,void(*)(void*));
  void (*result_pointer)(sqlite3_context*,void*,const char*,void(*)(void*));
  void *(*value_pointer)(sqlite3_value*,const char*);
  정수 (*vtab_nochange)(sqlite3_context*);
  정수 (*value_nochange)(sqlite3_value*);
  const char *(*vtab_collation)(sqlite3_index_info*,정수);
  /* Version 3.24.0 and later */
  정수 (*keyword_count)(void);
  정수 (*keyword_name)(정수,const char**,정수*);
  정수 (*keyword_check)(const char*,정수);
  sqlite3_str *(*str_new)(sqlite3*);
  char *(*str_finish)(sqlite3_str*);
  void (SQLITE_CDECL*str_appendf)(sqlite3_str*, const char *zFormat, ...);
  void (*str_vappendf)(sqlite3_str*, const char *zFormat, va_list);
  void (*str_append)(sqlite3_str*, const char *zIn, 정수 N);
  void (*str_appendall)(sqlite3_str*, const char *zIn);
  void (*str_appendchar)(sqlite3_str*, 정수 N, char C);
  void (*str_reset)(sqlite3_str*);
  정수 (*str_errcode)(sqlite3_str*);
  정수 (*str_length)(sqlite3_str*);
  char *(*str_value)(sqlite3_str*);
  /* Version 3.25.0 and later */
  정수 (*create_window_function)(sqlite3*,const char*,정수,정수,void*,
                            void (*xStep)(sqlite3_context*,정수,sqlite3_value**),
                            void (*xFinal)(sqlite3_context*),
                            void (*xValue)(sqlite3_context*),
                            void (*xInv)(sqlite3_context*,정수,sqlite3_value**),
                            void(*xDestroy)(void*));
  /* Version 3.26.0 and later */
  const char *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  정수 (*stmt_isexplain)(sqlite3_stmt*);
  정수 (*value_frombind)(sqlite3_value*);
  /* Version 3.30.0 and later */
  정수 (*drop_modules)(sqlite3*,const char**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  const char *(*uri_key)(const char*,정수);
  const char *(*filename_database)(const char*);
  const char *(*filename_journal)(const char*);
  const char *(*filename_wal)(const char*);
  /* Version 3.32.0 and later */
  const char *(*create_filename)(const char*,const char*,const char*,
                           정수,const char**);
  void (*free_filename)(const char*);
  sqlite3_file *(*database_file_object)(const char*);
  /* Version 3.34.0 and later */
  정수 (*txn_state)(sqlite3*,const char*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*total_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  정수 (*autovacuum_pages)(sqlite3*,
     unsigned 정수(*)(void*,const char*,unsigned 정수,unsigned 정수,unsigned 정수),
     void*, void(*)(void*));
  /* Version 3.38.0 and later */
  정수 (*error_offset)(sqlite3*);
  정수 (*vtab_rhs_value)(sqlite3_index_info*,정수,sqlite3_value**);
  정수 (*vtab_distinct)(sqlite3_index_info*);
  정수 (*vtab_in)(sqlite3_index_info*,정수,정수);
  정수 (*vtab_in_first)(sqlite3_value*,sqlite3_value**);
  정수 (*vtab_in_next)(sqlite3_value*,sqlite3_value**);
  /* Version 3.39.0 and later */
  정수 (*deserialize)(sqlite3*,const char*,unsigned char*,
                     sqlite3_int64,sqlite3_int64,unsigned);
  unsigned char *(*serialize)(sqlite3*,const char *,sqlite3_int64*,
                              unsigned 정수);
  const char *(*db_name)(sqlite3*,정수);
  /* Version 3.40.0 and later */
  정수 (*value_encoding)(sqlite3_value*);
  /* Version 3.41.0 and later */
  정수 (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  정수 (*stmt_explain)(sqlite3_stmt*,정수);
  /* Version 3.44.0 and later */
  void *(*get_clientdata)(sqlite3*,const char*);
  정수 (*set_clientdata)(sqlite3*, const char*, void*, void(*)(void*));
  /* Version 3.50.0 and later */
  정수 (*setlk_timeout)(sqlite3*,정수,정수);
  /* Version 3.51.0 and later */
  정수 (*set_errmsg)(sqlite3*,정수,const char*);
  정수 (*db_status64)(sqlite3*,정수,sqlite3_int64*,sqlite3_int64*,정수);
  /* Version 3.52.0 and later */
  void (*str_truncate)(sqlite3_str*,정수);
  void (*str_free)(sqlite3_str*);
  정수 (*carray_bind)(sqlite3_stmt*,정수,void*,정수,정수,void(*)(void*));
  정수 (*carray_bind_v2)(sqlite3_stmt*,정수,void*,정수,정수,void(*)(void*),void*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(const char*);
  void (*result_str)(sqlite3_context*,sqlite3_str*,정수);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
typedef 정수 (*sqlite3_loadext_entry)(
  sqlite3 *db,                       /* Handle to the database. */
  char **pzErrMsg,                   /* Used to set error string on failure. */
  const sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#만약 !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_aggregate_count        sqlite3_api->aggregate_count
#endif
#define sqlite3_bind_blob              sqlite3_api->bind_blob
#define sqlite3_bind_double            sqlite3_api->bind_double
#define sqlite3_bind_int               sqlite3_api->bind_int
#define sqlite3_bind_int64             sqlite3_api->bind_int64
#define sqlite3_bind_null              sqlite3_api->bind_null
#define sqlite3_bind_parameter_count   sqlite3_api->bind_parameter_count
#define sqlite3_bind_parameter_index   sqlite3_api->bind_parameter_index
#define sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#define sqlite3_bind_text              sqlite3_api->bind_text
#define sqlite3_bind_text16            sqlite3_api->bind_text16
#define sqlite3_bind_value             sqlite3_api->bind_value
#define sqlite3_busy_handler           sqlite3_api->busy_handler
#define sqlite3_busy_timeout           sqlite3_api->busy_timeout
#define sqlite3_changes                sqlite3_api->changes
#define sqlite3_close                  sqlite3_api->close
#define sqlite3_collation_needed       sqlite3_api->collation_needed
#define sqlite3_collation_needed16     sqlite3_api->collation_needed16
#define sqlite3_column_blob            sqlite3_api->column_blob
#define sqlite3_column_bytes           sqlite3_api->column_bytes
#define sqlite3_column_bytes16         sqlite3_api->column_bytes16
#define sqlite3_column_count           sqlite3_api->column_count
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
#define sqlite3_column_value           sqlite3_api->column_value
#define sqlite3_commit_hook            sqlite3_api->commit_hook
#define sqlite3_complete               sqlite3_api->complete
#define sqlite3_complete16             sqlite3_api->complete16
#define sqlite3_create_collation       sqlite3_api->create_collation
#define sqlite3_create_collation16     sqlite3_api->create_collation16
#define sqlite3_create_function        sqlite3_api->create_function
#define sqlite3_create_function16      sqlite3_api->create_function16
#define sqlite3_create_module          sqlite3_api->create_module
#define sqlite3_create_module_v2       sqlite3_api->create_module_v2
#define sqlite3_data_count             sqlite3_api->data_count
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
#define sqlite3_free                   sqlite3_api->free
#define sqlite3_free_table             sqlite3_api->free_table
#define sqlite3_get_autocommit         sqlite3_api->get_autocommit
#define sqlite3_get_auxdata            sqlite3_api->get_auxdata
#define sqlite3_get_table              sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_global_recover         sqlite3_api->global_recover
#endif
#define sqlite3_interrupt              sqlite3_api->interruptx
#define sqlite3_last_insert_rowid      sqlite3_api->last_insert_rowid
#define sqlite3_libversion             sqlite3_api->libversion
#define sqlite3_libversion_number      sqlite3_api->libversion_number
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
#define sqlite3_result_value           sqlite3_api->result_value
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
#define sqlite3_value_blob             sqlite3_api->value_blob
#define sqlite3_value_bytes            sqlite3_api->value_bytes
#define sqlite3_value_bytes16          sqlite3_api->value_bytes16
#define sqlite3_value_double           sqlite3_api->value_double
#define sqlite3_value_int              sqlite3_api->value_int
#define sqlite3_value_int64            sqlite3_api->value_int64
#define sqlite3_value_numeric_type     sqlite3_api->value_numeric_type
#define sqlite3_value_text             sqlite3_api->value_text
#define sqlite3_value_text16           sqlite3_api->value_text16
#define sqlite3_value_text16be         sqlite3_api->value_text16be
#define sqlite3_value_text16le         sqlite3_api->value_text16le
#define sqlite3_value_type             sqlite3_api->value_type
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
#define sqlite3_value_dup              sqlite3_api->value_dup
#define sqlite3_value_free             sqlite3_api->value_free
#define sqlite3_result_zeroblob64      sqlite3_api->result_zeroblob64
#define sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define sqlite3_value_subtype          sqlite3_api->value_subtype
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
#define sqlite3_set_last_insert_rowid  sqlite3_api->set_last_insert_rowid
/* Version 3.20.0 and later */
#define sqlite3_prepare_v3             sqlite3_api->prepare_v3
#define sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#define sqlite3_bind_pointer           sqlite3_api->bind_pointer
#define sqlite3_result_pointer         sqlite3_api->result_pointer
#define sqlite3_value_pointer          sqlite3_api->value_pointer
/* Version 3.22.0 and later */
#define sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#define sqlite3_value_nochange         sqlite3_api->value_nochange
#define sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#define sqlite3_keyword_count          sqlite3_api->keyword_count
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
#define sqlite3_str_value              sqlite3_api->str_value
/* Version 3.25.0 and later */
#define sqlite3_create_window_function sqlite3_api->create_window_function
/* Version 3.26.0 and later */
#define sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#define sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#define sqlite3_value_frombind         sqlite3_api->value_frombind
/* Version 3.30.0 and later */
#define sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#define sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#define sqlite3_uri_key                sqlite3_api->uri_key
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
#define sqlite3_vtab_rhs_value         sqlite3_api->vtab_rhs_value
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
#define sqlite3_value_encoding         sqlite3_api->value_encoding
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

#만약 !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     const sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    extern const sqlite3_api_routines *sqlite3_api;
#아니면
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (void)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */
