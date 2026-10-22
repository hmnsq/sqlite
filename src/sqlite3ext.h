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
#اگر_غیر_تعریف_شدہ SQLITE3EXT_H
#تعریف_کرو SQLITE3EXT_H
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
ساخت sqlite3_api_routines {
  خالی * (*aggregate_context)(sqlite3_context*,صحیح n_بائٹیں);
  صحیح  (*aggregate_گنتی)(sqlite3_context*);
  صحیح  (*bind_blob)(sqlite3_stmt*,صحیح,ثابت خالی*,صحیح n,خالی(*)(خالی*));
  صحیح  (*bind_double)(sqlite3_stmt*,صحیح,دوہرا);
  صحیح  (*bind_int)(sqlite3_stmt*,صحیح,صحیح);
  صحیح  (*bind_int64)(sqlite3_stmt*,صحیح,sqlite_int64);
  صحیح  (*bind_null)(sqlite3_stmt*,صحیح);
  صحیح  (*bind_parameter_گنتی)(sqlite3_stmt*);
  صحیح  (*bind_parameter_اشاریہ)(sqlite3_stmt*,ثابت حرف*zName);
  ثابت حرف * (*bind_parameter_name)(sqlite3_stmt*,صحیح);
  صحیح  (*bind_متن)(sqlite3_stmt*,صحیح,ثابت حرف*,صحیح n,خالی(*)(خالی*));
  صحیح  (*bind_text16)(sqlite3_stmt*,صحیح,ثابت خالی*,صحیح,خالی(*)(خالی*));
  صحیح  (*bind_قدر)(sqlite3_stmt*,صحیح,ثابت sqlite3_قدر*);
  صحیح  (*busy_handler)(sqlite3*,صحیح(*)(خالی*,صحیح),خالی*);
  صحیح  (*busy_timeout)(sqlite3*,صحیح ms);
  صحیح  (*changes)(sqlite3*);
  صحیح  (*close)(sqlite3*);
  صحیح  (*collation_needed)(sqlite3*,خالی*,خالی(*)(خالی*,sqlite3*,
                           صحیح e_متن_Rep,ثابت حرف*));
  صحیح  (*collation_needed16)(sqlite3*,خالی*,خالی(*)(خالی*,sqlite3*,
                             صحیح e_متن_Rep,ثابت خالی*));
  ثابت خالی * (*عمود_blob)(sqlite3_stmt*,صحیح i_ستون);
  صحیح  (*عمود_بائٹیں)(sqlite3_stmt*,صحیح i_ستون);
  صحیح  (*عمود_bytes16)(sqlite3_stmt*,صحیح i_ستون);
  صحیح  (*عمود_گنتی)(sqlite3_stmt*pStmt);
  ثابت حرف * (*عمود_database_name)(sqlite3_stmt*,صحیح);
  ثابت خالی * (*عمود_database_name16)(sqlite3_stmt*,صحیح);
  ثابت حرف * (*عمود_decltype)(sqlite3_stmt*,صحیح i);
  ثابت خالی * (*عمود_decltype16)(sqlite3_stmt*,صحیح);
  دوہرا  (*عمود_double)(sqlite3_stmt*,صحیح i_ستون);
  صحیح  (*عمود_int)(sqlite3_stmt*,صحیح i_ستون);
  sqlite_int64  (*عمود_int64)(sqlite3_stmt*,صحیح i_ستون);
  ثابت حرف * (*عمود_name)(sqlite3_stmt*,صحیح);
  ثابت خالی * (*عمود_name16)(sqlite3_stmt*,صحیح);
  ثابت حرف * (*عمود_origin_name)(sqlite3_stmt*,صحیح);
  ثابت خالی * (*عمود_origin_name16)(sqlite3_stmt*,صحیح);
  ثابت حرف * (*عمود_جدول_name)(sqlite3_stmt*,صحیح);
  ثابت خالی * (*عمود_جدول_name16)(sqlite3_stmt*,صحیح);
  ثابت غیرعلامتی حرف * (*عمود_متن)(sqlite3_stmt*,صحیح i_ستون);
  ثابت خالی * (*عمود_text16)(sqlite3_stmt*,صحیح i_ستون);
  صحیح  (*عمود_type)(sqlite3_stmt*,صحیح i_ستون);
  sqlite3_قدر* (*عمود_قدر)(sqlite3_stmt*,صحیح i_ستون);
  خالی * (*commit_hook)(sqlite3*,صحیح(*)(خالی*),خالی*);
  صحیح  (*complete)(ثابت حرف*sql);
  صحیح  (*complete16)(ثابت خالی*sql);
  صحیح  (*create_collation)(sqlite3*,ثابت حرف*,صحیح,خالی*,
                           صحیح(*)(خالی*,صحیح,ثابت خالی*,صحیح,ثابت خالی*));
  صحیح  (*create_collation16)(sqlite3*,ثابت خالی*,صحیح,خالی*,
                             صحیح(*)(خالی*,صحیح,ثابت خالی*,صحیح,ثابت خالی*));
  صحیح  (*create_function)(sqlite3*,ثابت حرف*,صحیح,صحیح,خالی*,
                          خالی (*xFunc)(sqlite3_context*,صحیح,sqlite3_قدر**),
                          خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر**),
                          خالی (*xFinal)(sqlite3_context*));
  صحیح  (*create_function16)(sqlite3*,ثابت خالی*,صحیح,صحیح,خالی*,
                            خالی (*xFunc)(sqlite3_context*,صحیح,sqlite3_قدر**),
                            خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر**),
                            خالی (*xFinal)(sqlite3_context*));
  صحیح (*create_module)(sqlite3*,ثابت حرف*,ثابت sqlite3_module*,خالی*);
  صحیح  (*ڈیٹا_گنتی)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  صحیح (*declare_vtab)(sqlite3*,ثابت حرف*);
  صحیح  (*enable_shared_cache)(صحیح);
  صحیح  (*errcode)(sqlite3*db);
  ثابت حرف * (*errmsg)(sqlite3*);
  ثابت خالی * (*errmsg16)(sqlite3*);
  صحیح  (*exec)(sqlite3*,ثابت حرف*,sqlite3_callback,خالی*,حرف**);
  صحیح  (*expired)(sqlite3_stmt*);
  صحیح  (*finalize)(sqlite3_stmt*pStmt);
  خالی  (*آزاد)(خالی*);
  خالی  (*free_جدول)(حرف**نتیجہ);
  صحیح  (*get_autocommit)(sqlite3*);
  خالی * (*get_auxdata)(sqlite3_context*,صحیح);
  صحیح  (*get_جدول)(sqlite3*,ثابت حرف*,حرف***,صحیح*,صحیح*,حرف**);
  صحیح  (*global_recover)(خالی);
  خالی  (*interruptx)(sqlite3*);
  sqlite_int64  (*آخری_داخل_کرو_rowid)(sqlite3*);
  ثابت حرف * (*libversion)(خالی);
  صحیح  (*libversion_نمبر)(خالی);
  خالی *(*حافظہ_دو)(صحیح);
  حرف *(SQLITE_CDECL*mprintf)(ثابت حرف*,...);
  صحیح  (*open)(ثابت حرف*,sqlite3**);
  صحیح  (*open16)(ثابت خالی*,sqlite3**);
  صحیح  (*prepare)(sqlite3*,ثابت حرف*,صحیح,sqlite3_stmt**,ثابت حرف**);
  صحیح  (*prepare16)(sqlite3*,ثابت خالی*,صحیح,sqlite3_stmt**,ثابت خالی**);
  خالی * (*profile)(sqlite3*,خالی(*)(خالی*,ثابت حرف*,sqlite_uint64),خالی*);
  خالی  (*پیش_رفت_handler)(sqlite3*,صحیح,صحیح(*)(خالی*),خالی*);
  خالی *(*realloc)(خالی*,صحیح);
  صحیح  (*reset)(sqlite3_stmt*pStmt);
  خالی  (*نتیجہ_blob)(sqlite3_context*,ثابت خالی*,صحیح,خالی(*)(خالی*));
  خالی  (*نتیجہ_double)(sqlite3_context*,دوہرا);
  خالی  (*نتیجہ_error)(sqlite3_context*,ثابت حرف*,صحیح);
  خالی  (*نتیجہ_error16)(sqlite3_context*,ثابت خالی*,صحیح);
  خالی  (*نتیجہ_int)(sqlite3_context*,صحیح);
  خالی  (*نتیجہ_int64)(sqlite3_context*,sqlite_int64);
  خالی  (*نتیجہ_null)(sqlite3_context*);
  خالی  (*نتیجہ_متن)(sqlite3_context*,ثابت حرف*,صحیح,خالی(*)(خالی*));
  خالی  (*نتیجہ_text16)(sqlite3_context*,ثابت خالی*,صحیح,خالی(*)(خالی*));
  خالی  (*نتیجہ_text16be)(sqlite3_context*,ثابت خالی*,صحیح,خالی(*)(خالی*));
  خالی  (*نتیجہ_text16le)(sqlite3_context*,ثابت خالی*,صحیح,خالی(*)(خالی*));
  خالی  (*نتیجہ_قدر)(sqlite3_context*,sqlite3_قدر*);
  خالی *(*rollback_hook)(sqlite3*,خالی(*)(خالی*),خالی*);
  صحیح  (*سیٹ_authorizer)(sqlite3*,صحیح(*)(خالی*,صحیح,ثابت حرف*,ثابت حرف*,
                         ثابت حرف*,ثابت حرف*),خالی*);
  خالی  (*سیٹ_auxdata)(sqlite3_context*,صحیح,خالی*,خالی (*)(خالی*));
  حرف *(SQLITE_CDECL*xsnprintf)(صحیح,حرف*,ثابت حرف*,...);
  صحیح  (*قدم)(sqlite3_stmt*);
  صحیح  (*جدول_عمود_metadata)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,
                                حرف ثابت**,حرف ثابت**,صحیح*,صحیح*,صحیح*);
  خالی  (*thread_cleanup)(خالی);
  صحیح  (*کل_changes)(sqlite3*);
  خالی * (*trace)(sqlite3*,خالی(*xTrace)(خالی*,ثابت حرف*),خالی*);
  صحیح  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  خالی * (*update_hook)(sqlite3*,خالی(*)(خالی*,صحیح ,حرف ثابت*,حرف ثابت*,
                                         sqlite_int64),خالی*);
  خالی * (*user_ڈیٹا)(sqlite3_context*);
  ثابت خالی * (*قدر_blob)(sqlite3_قدر*);
  صحیح  (*قدر_بائٹیں)(sqlite3_قدر*);
  صحیح  (*قدر_bytes16)(sqlite3_قدر*);
  دوہرا  (*قدر_double)(sqlite3_قدر*);
  صحیح  (*قدر_int)(sqlite3_قدر*);
  sqlite_int64  (*قدر_int64)(sqlite3_قدر*);
  صحیح  (*قدر_numeric_type)(sqlite3_قدر*);
  ثابت غیرعلامتی حرف * (*قدر_متن)(sqlite3_قدر*);
  ثابت خالی * (*قدر_text16)(sqlite3_قدر*);
  ثابت خالی * (*قدر_text16be)(sqlite3_قدر*);
  ثابت خالی * (*قدر_text16le)(sqlite3_قدر*);
  صحیح  (*قدر_type)(sqlite3_قدر*);
  حرف *(*vmprintf)(ثابت حرف*,va_list);
  /* Added ??? */
  صحیح (*overload_function)(sqlite3*, ثابت حرف *zFuncName, صحیح nArg);
  /* Added by 3.3.13 */
  صحیح (*prepare_v2)(sqlite3*,ثابت حرف*,صحیح,sqlite3_stmt**,ثابت حرف**);
  صحیح (*prepare16_v2)(sqlite3*,ثابت خالی*,صحیح,sqlite3_stmt**,ثابت خالی**);
  صحیح (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  صحیح (*create_module_v2)(sqlite3*,ثابت حرف*,ثابت sqlite3_module*,خالی*,
                          خالی (*xDestroy)(خالی *));
  /* Added by 3.5.0 */
  صحیح (*bind_zeroblob)(sqlite3_stmt*,صحیح,صحیح);
  صحیح (*blob_بائٹیں)(sqlite3_blob*);
  صحیح (*blob_close)(sqlite3_blob*);
  صحیح (*blob_open)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,sqlite3_int64,
                   صحیح,sqlite3_blob**);
  صحیح (*blob_read)(sqlite3_blob*,خالی*,صحیح,صحیح);
  صحیح (*blob_write)(sqlite3_blob*,ثابت خالی*,صحیح,صحیح);
  صحیح (*create_collation_v2)(sqlite3*,ثابت حرف*,صحیح,خالی*,
                             صحیح(*)(خالی*,صحیح,ثابت خالی*,صحیح,ثابت خالی*),
                             خالی(*)(خالی*));
  صحیح (*file_control)(sqlite3*,ثابت حرف*,صحیح,خالی*);
  sqlite3_int64 (*حافظہ_highwater)(صحیح);
  sqlite3_int64 (*حافظہ_مستعمل)(خالی);
  sqlite3_mutex *(*mutex_alloc)(صحیح);
  خالی (*mutex_enter)(sqlite3_mutex*);
  خالی (*mutex_free)(sqlite3_mutex*);
  خالی (*mutex_leave)(sqlite3_mutex*);
  صحیح (*mutex_try)(sqlite3_mutex*);
  صحیح (*open_v2)(ثابت حرف*,sqlite3**,صحیح,ثابت حرف*);
  صحیح (*release_حافظہ)(صحیح);
  خالی (*نتیجہ_error_nomem)(sqlite3_context*);
  خالی (*نتیجہ_error_toobig)(sqlite3_context*);
  صحیح (*sleep)(صحیح);
  خالی (*soft_ڈھیر_حد_بندی)(صحیح);
  sqlite3_vfs *(*vfs_ڈھونڈو)(ثابت حرف*);
  صحیح (*vfs_رجسٹر)(sqlite3_vfs*,صحیح);
  صحیح (*vfs_unregister)(sqlite3_vfs*);
  صحیح (*xthreadsafe)(خالی);
  خالی (*نتیجہ_zeroblob)(sqlite3_context*,صحیح);
  خالی (*نتیجہ_error_رمز)(sqlite3_context*,صحیح);
  صحیح (SQLITE_CDECL*test_control)(صحیح, ...);
  خالی (*randomness)(صحیح,خالی*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  صحیح (*extended_نتیجہ_codes)(sqlite3*,صحیح);
  صحیح (*حد_بندی)(sqlite3*,صحیح,صحیح);
  sqlite3_stmt *(*اگلا_stmt)(sqlite3*,sqlite3_stmt*);
  ثابت حرف *(*sql)(sqlite3_stmt*);
  صحیح (*حالت_2)(صحیح,صحیح*,صحیح*,صحیح);
  صحیح (*backup_اختتام)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ثابت حرف*,sqlite3*,ثابت حرف*);
  صحیح (*backup_pagecount)(sqlite3_backup*);
  صحیح (*backup_بقایا)(sqlite3_backup*);
  صحیح (*backup_قدم)(sqlite3_backup*,صحیح);
  ثابت حرف *(*compileoption_get)(صحیح);
  صحیح (*compileoption_مستعمل)(ثابت حرف*);
  صحیح (*create_function_v2)(sqlite3*,ثابت حرف*,صحیح,صحیح,خالی*,
                            خالی (*xFunc)(sqlite3_context*,صحیح,sqlite3_قدر**),
                            خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر**),
                            خالی (*xFinal)(sqlite3_context*),
                            خالی(*xDestroy)(خالی*));
  صحیح (SQLITE_CDECL*db_config)(sqlite3*,صحیح,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  صحیح (*db_حالت)(sqlite3*,صحیح,صحیح*,صحیح*,صحیح);
  صحیح (*extended_errcode)(sqlite3*);
  خالی (SQLITE_CDECL*لوگارتھم)(صحیح,ثابت حرف*,...);
  sqlite3_int64 (*soft_ڈھیر_limit64)(sqlite3_int64);
  ثابت حرف *(*sourceid)(خالی);
  صحیح (*stmt_حالت)(sqlite3_stmt*,صحیح,صحیح);
  صحیح (*strnicmp)(ثابت حرف*,ثابت حرف*,صحیح);
  صحیح (*unlock_notify)(sqlite3*,خالی(*)(خالی**,صحیح),خالی*);
  صحیح (*wal_autocheckpoint)(sqlite3*,صحیح);
  صحیح (*wal_checkpoint)(sqlite3*,ثابت حرف*);
  خالی *(*wal_hook)(sqlite3*,صحیح(*)(خالی*,sqlite3*,ثابت حرف*,صحیح),خالی*);
  صحیح (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  صحیح (SQLITE_CDECL*vtab_config)(sqlite3*,صحیح op,...);
  صحیح (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  صحیح (*close_v2)(sqlite3*);
  ثابت حرف *(*db_filename)(sqlite3*,ثابت حرف*);
  صحیح (*db_readonly)(sqlite3*,ثابت حرف*);
  صحیح (*db_release_حافظہ)(sqlite3*);
  ثابت حرف *(*errstr)(صحیح);
  صحیح (*stmt_busy)(sqlite3_stmt*);
  صحیح (*stmt_readonly)(sqlite3_stmt*);
  صحیح (*stricmp)(ثابت حرف*,ثابت حرف*);
  صحیح (*uri_boolean)(ثابت حرف*,ثابت حرف*,صحیح);
  sqlite3_int64 (*uri_int64)(ثابت حرف*,ثابت حرف*,sqlite3_int64);
  ثابت حرف *(*uri_parameter)(ثابت حرف*,ثابت حرف*);
  حرف *(*xvsnprintf)(صحیح,حرف*,ثابت حرف*,va_list);
  صحیح (*wal_checkpoint_v2)(sqlite3*,ثابت حرف*,صحیح,صحیح*,صحیح*);
  /* Version 3.8.7 and later */
  صحیح (*auto_extension)(خالی(*)(خالی));
  صحیح (*bind_blob64)(sqlite3_stmt*,صحیح,ثابت خالی*,sqlite3_uint64,
                     خالی(*)(خالی*));
  صحیح (*bind_text64)(sqlite3_stmt*,صحیح,ثابت حرف*,sqlite3_uint64,
                      خالی(*)(خالی*),غیرعلامتی حرف);
  صحیح (*cancel_auto_extension)(خالی(*)(خالی));
  صحیح (*بھرو_extension)(sqlite3*,ثابت حرف*,ثابت حرف*,حرف**);
  خالی *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(خالی*);
  خالی *(*realloc64)(خالی*,sqlite3_uint64);
  خالی (*reset_auto_extension)(خالی);
  خالی (*نتیجہ_blob64)(sqlite3_context*,ثابت خالی*,sqlite3_uint64,
                        خالی(*)(خالی*));
  خالی (*نتیجہ_text64)(sqlite3_context*,ثابت حرف*,sqlite3_uint64,
                         خالی(*)(خالی*), غیرعلامتی حرف);
  صحیح (*strglob)(ثابت حرف*,ثابت حرف*);
  /* Version 3.8.11 and later */
  sqlite3_قدر *(*قدر_dup)(ثابت sqlite3_قدر*);
  خالی (*قدر_free)(sqlite3_قدر*);
  صحیح (*نتیجہ_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  صحیح (*bind_zeroblob64)(sqlite3_stmt*, صحیح, sqlite3_uint64);
  /* Version 3.9.0 and later */
  غیرعلامتی صحیح (*قدر_subtype)(sqlite3_قدر*);
  خالی (*نتیجہ_subtype)(sqlite3_context*,غیرعلامتی صحیح);
  /* Version 3.10.0 and later */
  صحیح (*status64)(صحیح,sqlite3_int64*,sqlite3_int64*,صحیح);
  صحیح (*strlike)(ثابت حرف*,ثابت حرف*,غیرعلامتی صحیح);
  صحیح (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  صحیح (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  صحیح (*trace_v2)(sqlite3*,غیرعلامتی,صحیح(*)(غیرعلامتی,خالی*,خالی*,خالی*),خالی*);
  حرف *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  خالی (*سیٹ_آخری_داخل_کرو_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  صحیح (*prepare_v3)(sqlite3*,ثابت حرف*,صحیح,غیرعلامتی صحیح,
                    sqlite3_stmt**,ثابت حرف**);
  صحیح (*prepare16_v3)(sqlite3*,ثابت خالی*,صحیح,غیرعلامتی صحیح,
                      sqlite3_stmt**,ثابت خالی**);
  صحیح (*bind_اشارہ_گر)(sqlite3_stmt*,صحیح,خالی*,ثابت حرف*,خالی(*)(خالی*));
  خالی (*نتیجہ_اشارہ_گر)(sqlite3_context*,خالی*,ثابت حرف*,خالی(*)(خالی*));
  خالی *(*قدر_اشارہ_گر)(sqlite3_قدر*,ثابت حرف*);
  صحیح (*vtab_nochange)(sqlite3_context*);
  صحیح (*قدر_nochange)(sqlite3_قدر*);
  ثابت حرف *(*vtab_collation)(sqlite3_اشاریہ_info*,صحیح);
  /* Version 3.24.0 and later */
  صحیح (*keyword_گنتی)(خالی);
  صحیح (*keyword_name)(صحیح,ثابت حرف**,صحیح*);
  صحیح (*keyword_check)(ثابت حرف*,صحیح);
  sqlite3_str *(*str_new)(sqlite3*);
  حرف *(*str_اختتام)(sqlite3_str*);
  خالی (SQLITE_CDECL*str_appendf)(sqlite3_str*, ثابت حرف *zFormat, ...);
  خالی (*str_vappendf)(sqlite3_str*, ثابت حرف *zFormat, va_list);
  خالی (*str_append)(sqlite3_str*, ثابت حرف *z_اندر, صحیح N);
  خالی (*str_appendall)(sqlite3_str*, ثابت حرف *z_اندر);
  خالی (*str_appendchar)(sqlite3_str*, صحیح N, حرف C);
  خالی (*str_reset)(sqlite3_str*);
  صحیح (*str_errcode)(sqlite3_str*);
  صحیح (*str_لمبائی)(sqlite3_str*);
  حرف *(*str_قدر)(sqlite3_str*);
  /* Version 3.25.0 and later */
  صحیح (*create_کھڑکی_function)(sqlite3*,ثابت حرف*,صحیح,صحیح,خالی*,
                            خالی (*x_قدم)(sqlite3_context*,صحیح,sqlite3_قدر**),
                            خالی (*xFinal)(sqlite3_context*),
                            خالی (*x_قدر)(sqlite3_context*),
                            خالی (*xInv)(sqlite3_context*,صحیح,sqlite3_قدر**),
                            خالی(*xDestroy)(خالی*));
  /* Version 3.26.0 and later */
  ثابت حرف *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  صحیح (*stmt_isexplain)(sqlite3_stmt*);
  صحیح (*قدر_frombind)(sqlite3_قدر*);
  /* Version 3.30.0 and later */
  صحیح (*drop_modules)(sqlite3*,ثابت حرف**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_ڈھیر_limit64)(sqlite3_int64);
  ثابت حرف *(*uri_کلید)(ثابت حرف*,صحیح);
  ثابت حرف *(*filename_database)(ثابت حرف*);
  ثابت حرف *(*filename_journal)(ثابت حرف*);
  ثابت حرف *(*filename_wal)(ثابت حرف*);
  /* Version 3.32.0 and later */
  ثابت حرف *(*create_filename)(ثابت حرف*,ثابت حرف*,ثابت حرف*,
                           صحیح,ثابت حرف**);
  خالی (*free_filename)(ثابت حرف*);
  sqlite3_file *(*database_file_object)(ثابت حرف*);
  /* Version 3.34.0 and later */
  صحیح (*txn_state)(sqlite3*,ثابت حرف*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*کل_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  صحیح (*autovacuum_صفحات)(sqlite3*,
     غیرعلامتی صحیح(*)(خالی*,ثابت حرف*,غیرعلامتی صحیح,غیرعلامتی صحیح,غیرعلامتی صحیح),
     خالی*, خالی(*)(خالی*));
  /* Version 3.38.0 and later */
  صحیح (*error_انحراف)(sqlite3*);
  صحیح (*vtab_rhs_قدر)(sqlite3_اشاریہ_info*,صحیح,sqlite3_قدر**);
  صحیح (*vtab_distinct)(sqlite3_اشاریہ_info*);
  صحیح (*vtab_اندر)(sqlite3_اشاریہ_info*,صحیح,صحیح);
  صحیح (*vtab_اندر_پہلا)(sqlite3_قدر*,sqlite3_قدر**);
  صحیح (*vtab_اندر_اگلا)(sqlite3_قدر*,sqlite3_قدر**);
  /* Version 3.39.0 and later */
  صحیح (*deserialize)(sqlite3*,ثابت حرف*,غیرعلامتی حرف*,
                     sqlite3_int64,sqlite3_int64,غیرعلامتی);
  غیرعلامتی حرف *(*serialize)(sqlite3*,ثابت حرف *,sqlite3_int64*,
                              غیرعلامتی صحیح);
  ثابت حرف *(*db_name)(sqlite3*,صحیح);
  /* Version 3.40.0 and later */
  صحیح (*قدر_encoding)(sqlite3_قدر*);
  /* Version 3.41.0 and later */
  صحیح (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  صحیح (*stmt_explain)(sqlite3_stmt*,صحیح);
  /* Version 3.44.0 and later */
  خالی *(*get_clientdata)(sqlite3*,ثابت حرف*);
  صحیح (*سیٹ_clientdata)(sqlite3*, ثابت حرف*, خالی*, خالی(*)(خالی*));
  /* Version 3.50.0 and later */
  صحیح (*setlk_timeout)(sqlite3*,صحیح,صحیح);
  /* Version 3.51.0 and later */
  صحیح (*سیٹ_errmsg)(sqlite3*,صحیح,ثابت حرف*);
  صحیح (*db_status64)(sqlite3*,صحیح,sqlite3_int64*,sqlite3_int64*,صحیح);
  /* Version 3.52.0 and later */
  خالی (*str_truncate)(sqlite3_str*,صحیح);
  خالی (*str_free)(sqlite3_str*);
  صحیح (*carray_bind)(sqlite3_stmt*,صحیح,خالی*,صحیح,صحیح,خالی(*)(خالی*));
  صحیح (*carray_bind_v2)(sqlite3_stmt*,صحیح,خالی*,صحیح,صحیح,خالی(*)(خالی*),خالی*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ثابت حرف*);
  خالی (*نتیجہ_str)(sqlite3_context*,sqlite3_str*,صحیح);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
قسمتعریف صحیح (*sqlite3_loadext_اندراج)(
  sqlite3 *db,                       /* Handle to the database. */
  حرف **pz_Err_پیغام,                   /* Used to set error string on failure. */
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
#اگر !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_بھرو_EXTENSION)
#تعریف_کرو sqlite3_aggregate_context      sqlite3_api->aggregate_context
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
#تعریف_کرو sqlite3_aggregate_گنتی        sqlite3_api->aggregate_گنتی
#اگر_ختم
#تعریف_کرو sqlite3_bind_blob              sqlite3_api->bind_blob
#تعریف_کرو sqlite3_bind_double            sqlite3_api->bind_double
#تعریف_کرو sqlite3_bind_int               sqlite3_api->bind_int
#تعریف_کرو sqlite3_bind_int64             sqlite3_api->bind_int64
#تعریف_کرو sqlite3_bind_null              sqlite3_api->bind_null
#تعریف_کرو sqlite3_bind_parameter_گنتی   sqlite3_api->bind_parameter_گنتی
#تعریف_کرو sqlite3_bind_parameter_اشاریہ   sqlite3_api->bind_parameter_اشاریہ
#تعریف_کرو sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#تعریف_کرو sqlite3_bind_متن              sqlite3_api->bind_متن
#تعریف_کرو sqlite3_bind_text16            sqlite3_api->bind_text16
#تعریف_کرو sqlite3_bind_قدر             sqlite3_api->bind_قدر
#تعریف_کرو sqlite3_busy_handler           sqlite3_api->busy_handler
#تعریف_کرو sqlite3_busy_timeout           sqlite3_api->busy_timeout
#تعریف_کرو sqlite3_changes                sqlite3_api->changes
#تعریف_کرو sqlite3_close                  sqlite3_api->close
#تعریف_کرو sqlite3_collation_needed       sqlite3_api->collation_needed
#تعریف_کرو sqlite3_collation_needed16     sqlite3_api->collation_needed16
#تعریف_کرو sqlite3_عمود_blob            sqlite3_api->عمود_blob
#تعریف_کرو sqlite3_عمود_بائٹیں           sqlite3_api->عمود_بائٹیں
#تعریف_کرو sqlite3_عمود_bytes16         sqlite3_api->عمود_bytes16
#تعریف_کرو sqlite3_عمود_گنتی           sqlite3_api->عمود_گنتی
#تعریف_کرو sqlite3_عمود_database_name   sqlite3_api->عمود_database_name
#تعریف_کرو sqlite3_عمود_database_name16 sqlite3_api->عمود_database_name16
#تعریف_کرو sqlite3_عمود_decltype        sqlite3_api->عمود_decltype
#تعریف_کرو sqlite3_عمود_decltype16      sqlite3_api->عمود_decltype16
#تعریف_کرو sqlite3_عمود_double          sqlite3_api->عمود_double
#تعریف_کرو sqlite3_عمود_int             sqlite3_api->عمود_int
#تعریف_کرو sqlite3_عمود_int64           sqlite3_api->عمود_int64
#تعریف_کرو sqlite3_عمود_name            sqlite3_api->عمود_name
#تعریف_کرو sqlite3_عمود_name16          sqlite3_api->عمود_name16
#تعریف_کرو sqlite3_عمود_origin_name     sqlite3_api->عمود_origin_name
#تعریف_کرو sqlite3_عمود_origin_name16   sqlite3_api->عمود_origin_name16
#تعریف_کرو sqlite3_عمود_جدول_name      sqlite3_api->عمود_جدول_name
#تعریف_کرو sqlite3_عمود_جدول_name16    sqlite3_api->عمود_جدول_name16
#تعریف_کرو sqlite3_عمود_متن            sqlite3_api->عمود_متن
#تعریف_کرو sqlite3_عمود_text16          sqlite3_api->عمود_text16
#تعریف_کرو sqlite3_عمود_type            sqlite3_api->عمود_type
#تعریف_کرو sqlite3_عمود_قدر           sqlite3_api->عمود_قدر
#تعریف_کرو sqlite3_commit_hook            sqlite3_api->commit_hook
#تعریف_کرو sqlite3_complete               sqlite3_api->complete
#تعریف_کرو sqlite3_complete16             sqlite3_api->complete16
#تعریف_کرو sqlite3_create_collation       sqlite3_api->create_collation
#تعریف_کرو sqlite3_create_collation16     sqlite3_api->create_collation16
#تعریف_کرو sqlite3_create_function        sqlite3_api->create_function
#تعریف_کرو sqlite3_create_function16      sqlite3_api->create_function16
#تعریف_کرو sqlite3_create_module          sqlite3_api->create_module
#تعریف_کرو sqlite3_create_module_v2       sqlite3_api->create_module_v2
#تعریف_کرو sqlite3_ڈیٹا_گنتی             sqlite3_api->ڈیٹا_گنتی
#تعریف_کرو sqlite3_db_handle              sqlite3_api->db_handle
#تعریف_کرو sqlite3_declare_vtab           sqlite3_api->declare_vtab
#تعریف_کرو sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#تعریف_کرو sqlite3_errcode                sqlite3_api->errcode
#تعریف_کرو sqlite3_errmsg                 sqlite3_api->errmsg
#تعریف_کرو sqlite3_errmsg16               sqlite3_api->errmsg16
#تعریف_کرو sqlite3_exec                   sqlite3_api->exec
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
#تعریف_کرو sqlite3_expired                sqlite3_api->expired
#اگر_ختم
#تعریف_کرو sqlite3_finalize               sqlite3_api->finalize
#تعریف_کرو sqlite3_free                   sqlite3_api->آزاد
#تعریف_کرو sqlite3_free_جدول             sqlite3_api->free_جدول
#تعریف_کرو sqlite3_get_autocommit         sqlite3_api->get_autocommit
#تعریف_کرو sqlite3_get_auxdata            sqlite3_api->get_auxdata
#تعریف_کرو sqlite3_get_جدول              sqlite3_api->get_جدول
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
#تعریف_کرو sqlite3_global_recover         sqlite3_api->global_recover
#اگر_ختم
#تعریف_کرو sqlite3_مداخلت              sqlite3_api->interruptx
#تعریف_کرو sqlite3_آخری_داخل_کرو_rowid      sqlite3_api->آخری_داخل_کرو_rowid
#تعریف_کرو sqlite3_libversion             sqlite3_api->libversion
#تعریف_کرو sqlite3_libversion_نمبر      sqlite3_api->libversion_نمبر
#تعریف_کرو sqlite3_malloc                 sqlite3_api->حافظہ_دو
#تعریف_کرو sqlite3_mprintf                sqlite3_api->mprintf
#تعریف_کرو sqlite3_open                   sqlite3_api->open
#تعریف_کرو sqlite3_open16                 sqlite3_api->open16
#تعریف_کرو sqlite3_prepare                sqlite3_api->prepare
#تعریف_کرو sqlite3_prepare16              sqlite3_api->prepare16
#تعریف_کرو sqlite3_prepare_v2             sqlite3_api->prepare_v2
#تعریف_کرو sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#تعریف_کرو sqlite3_profile                sqlite3_api->profile
#تعریف_کرو sqlite3_پیش_رفت_handler       sqlite3_api->پیش_رفت_handler
#تعریف_کرو sqlite3_realloc                sqlite3_api->realloc
#تعریف_کرو sqlite3_reset                  sqlite3_api->reset
#تعریف_کرو sqlite3_نتیجہ_blob            sqlite3_api->نتیجہ_blob
#تعریف_کرو sqlite3_نتیجہ_double          sqlite3_api->نتیجہ_double
#تعریف_کرو sqlite3_نتیجہ_error           sqlite3_api->نتیجہ_error
#تعریف_کرو sqlite3_نتیجہ_error16         sqlite3_api->نتیجہ_error16
#تعریف_کرو sqlite3_نتیجہ_int             sqlite3_api->نتیجہ_int
#تعریف_کرو sqlite3_نتیجہ_int64           sqlite3_api->نتیجہ_int64
#تعریف_کرو sqlite3_نتیجہ_null            sqlite3_api->نتیجہ_null
#تعریف_کرو sqlite3_نتیجہ_متن            sqlite3_api->نتیجہ_متن
#تعریف_کرو sqlite3_نتیجہ_text16          sqlite3_api->نتیجہ_text16
#تعریف_کرو sqlite3_نتیجہ_text16be        sqlite3_api->نتیجہ_text16be
#تعریف_کرو sqlite3_نتیجہ_text16le        sqlite3_api->نتیجہ_text16le
#تعریف_کرو sqlite3_نتیجہ_قدر           sqlite3_api->نتیجہ_قدر
#تعریف_کرو sqlite3_rollback_hook          sqlite3_api->rollback_hook
#تعریف_کرو sqlite3_سیٹ_authorizer         sqlite3_api->سیٹ_authorizer
#تعریف_کرو sqlite3_سیٹ_auxdata            sqlite3_api->سیٹ_auxdata
#تعریف_کرو sqlite3_snprintf               sqlite3_api->xsnprintf
#تعریف_کرو sqlite3_قدم                   sqlite3_api->قدم
#تعریف_کرو sqlite3_جدول_عمود_metadata  sqlite3_api->جدول_عمود_metadata
#تعریف_کرو sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#تعریف_کرو sqlite3_کل_changes          sqlite3_api->کل_changes
#تعریف_کرو sqlite3_trace                  sqlite3_api->trace
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
#تعریف_کرو sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#اگر_ختم
#تعریف_کرو sqlite3_update_hook            sqlite3_api->update_hook
#تعریف_کرو sqlite3_user_ڈیٹا              sqlite3_api->user_ڈیٹا
#تعریف_کرو sqlite3_قدر_blob             sqlite3_api->قدر_blob
#تعریف_کرو sqlite3_قدر_بائٹیں            sqlite3_api->قدر_بائٹیں
#تعریف_کرو sqlite3_قدر_bytes16          sqlite3_api->قدر_bytes16
#تعریف_کرو sqlite3_قدر_double           sqlite3_api->قدر_double
#تعریف_کرو sqlite3_قدر_int              sqlite3_api->قدر_int
#تعریف_کرو sqlite3_قدر_int64            sqlite3_api->قدر_int64
#تعریف_کرو sqlite3_قدر_numeric_type     sqlite3_api->قدر_numeric_type
#تعریف_کرو sqlite3_قدر_متن             sqlite3_api->قدر_متن
#تعریف_کرو sqlite3_قدر_text16           sqlite3_api->قدر_text16
#تعریف_کرو sqlite3_قدر_text16be         sqlite3_api->قدر_text16be
#تعریف_کرو sqlite3_قدر_text16le         sqlite3_api->قدر_text16le
#تعریف_کرو sqlite3_قدر_type             sqlite3_api->قدر_type
#تعریف_کرو sqlite3_vmprintf               sqlite3_api->vmprintf
#تعریف_کرو sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#تعریف_کرو sqlite3_overload_function      sqlite3_api->overload_function
#تعریف_کرو sqlite3_prepare_v2             sqlite3_api->prepare_v2
#تعریف_کرو sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#تعریف_کرو sqlite3_clear_bindings         sqlite3_api->clear_bindings
#تعریف_کرو sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#تعریف_کرو sqlite3_blob_بائٹیں             sqlite3_api->blob_بائٹیں
#تعریف_کرو sqlite3_blob_close             sqlite3_api->blob_close
#تعریف_کرو sqlite3_blob_open              sqlite3_api->blob_open
#تعریف_کرو sqlite3_blob_read              sqlite3_api->blob_read
#تعریف_کرو sqlite3_blob_write             sqlite3_api->blob_write
#تعریف_کرو sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#تعریف_کرو sqlite3_file_control           sqlite3_api->file_control
#تعریف_کرو sqlite3_حافظہ_highwater       sqlite3_api->حافظہ_highwater
#تعریف_کرو sqlite3_حافظہ_مستعمل            sqlite3_api->حافظہ_مستعمل
#تعریف_کرو sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#تعریف_کرو sqlite3_mutex_enter            sqlite3_api->mutex_enter
#تعریف_کرو sqlite3_mutex_free             sqlite3_api->mutex_free
#تعریف_کرو sqlite3_mutex_leave            sqlite3_api->mutex_leave
#تعریف_کرو sqlite3_mutex_try              sqlite3_api->mutex_try
#تعریف_کرو sqlite3_open_v2                sqlite3_api->open_v2
#تعریف_کرو sqlite3_release_حافظہ         sqlite3_api->release_حافظہ
#تعریف_کرو sqlite3_نتیجہ_error_nomem     sqlite3_api->نتیجہ_error_nomem
#تعریف_کرو sqlite3_نتیجہ_error_toobig    sqlite3_api->نتیجہ_error_toobig
#تعریف_کرو sqlite3_sleep                  sqlite3_api->sleep
#تعریف_کرو sqlite3_soft_ڈھیر_حد_بندی        sqlite3_api->soft_ڈھیر_حد_بندی
#تعریف_کرو sqlite3_vfs_ڈھونڈو               sqlite3_api->vfs_ڈھونڈو
#تعریف_کرو sqlite3_vfs_رجسٹر           sqlite3_api->vfs_رجسٹر
#تعریف_کرو sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#تعریف_کرو sqlite3_threadsafe             sqlite3_api->xthreadsafe
#تعریف_کرو sqlite3_نتیجہ_zeroblob        sqlite3_api->نتیجہ_zeroblob
#تعریف_کرو sqlite3_نتیجہ_error_رمز      sqlite3_api->نتیجہ_error_رمز
#تعریف_کرو sqlite3_test_control           sqlite3_api->test_control
#تعریف_کرو sqlite3_randomness             sqlite3_api->randomness
#تعریف_کرو sqlite3_context_db_handle      sqlite3_api->context_db_handle
#تعریف_کرو sqlite3_extended_نتیجہ_codes  sqlite3_api->extended_نتیجہ_codes
#تعریف_کرو sqlite3_حد_بندی                  sqlite3_api->حد_بندی
#تعریف_کرو sqlite3_اگلا_stmt              sqlite3_api->اگلا_stmt
#تعریف_کرو sqlite3_sql                    sqlite3_api->sql
#تعریف_کرو sqlite3_حالت                 sqlite3_api->حالت_2
#تعریف_کرو sqlite3_backup_اختتام          sqlite3_api->backup_اختتام
#تعریف_کرو sqlite3_backup_init            sqlite3_api->backup_init
#تعریف_کرو sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#تعریف_کرو sqlite3_backup_بقایا       sqlite3_api->backup_بقایا
#تعریف_کرو sqlite3_backup_قدم            sqlite3_api->backup_قدم
#تعریف_کرو sqlite3_compileoption_get      sqlite3_api->compileoption_get
#تعریف_کرو sqlite3_compileoption_مستعمل     sqlite3_api->compileoption_مستعمل
#تعریف_کرو sqlite3_create_function_v2     sqlite3_api->create_function_v2
#تعریف_کرو sqlite3_db_config              sqlite3_api->db_config
#تعریف_کرو sqlite3_db_mutex               sqlite3_api->db_mutex
#تعریف_کرو sqlite3_db_حالت              sqlite3_api->db_حالت
#تعریف_کرو sqlite3_extended_errcode       sqlite3_api->extended_errcode
#تعریف_کرو sqlite3_log                    sqlite3_api->لوگارتھم
#تعریف_کرو sqlite3_soft_ڈھیر_limit64      sqlite3_api->soft_ڈھیر_limit64
#تعریف_کرو sqlite3_sourceid               sqlite3_api->sourceid
#تعریف_کرو sqlite3_stmt_حالت            sqlite3_api->stmt_حالت
#تعریف_کرو sqlite3_strnicmp               sqlite3_api->strnicmp
#تعریف_کرو sqlite3_unlock_notify          sqlite3_api->unlock_notify
#تعریف_کرو sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#تعریف_کرو sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#تعریف_کرو sqlite3_wal_hook               sqlite3_api->wal_hook
#تعریف_کرو sqlite3_blob_reopen            sqlite3_api->blob_reopen
#تعریف_کرو sqlite3_vtab_config            sqlite3_api->vtab_config
#تعریف_کرو sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#تعریف_کرو sqlite3_close_v2               sqlite3_api->close_v2
#تعریف_کرو sqlite3_db_filename            sqlite3_api->db_filename
#تعریف_کرو sqlite3_db_readonly            sqlite3_api->db_readonly
#تعریف_کرو sqlite3_db_release_حافظہ      sqlite3_api->db_release_حافظہ
#تعریف_کرو sqlite3_errstr                 sqlite3_api->errstr
#تعریف_کرو sqlite3_stmt_busy              sqlite3_api->stmt_busy
#تعریف_کرو sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#تعریف_کرو sqlite3_stricmp                sqlite3_api->stricmp
#تعریف_کرو sqlite3_uri_boolean            sqlite3_api->uri_boolean
#تعریف_کرو sqlite3_uri_int64              sqlite3_api->uri_int64
#تعریف_کرو sqlite3_uri_parameter          sqlite3_api->uri_parameter
#تعریف_کرو sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#تعریف_کرو sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#تعریف_کرو sqlite3_auto_extension         sqlite3_api->auto_extension
#تعریف_کرو sqlite3_bind_blob64            sqlite3_api->bind_blob64
#تعریف_کرو sqlite3_bind_text64            sqlite3_api->bind_text64
#تعریف_کرو sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#تعریف_کرو sqlite3_بھرو_extension         sqlite3_api->بھرو_extension
#تعریف_کرو sqlite3_malloc64               sqlite3_api->malloc64
#تعریف_کرو sqlite3_msize                  sqlite3_api->msize
#تعریف_کرو sqlite3_realloc64              sqlite3_api->realloc64
#تعریف_کرو sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#تعریف_کرو sqlite3_نتیجہ_blob64          sqlite3_api->نتیجہ_blob64
#تعریف_کرو sqlite3_نتیجہ_text64          sqlite3_api->نتیجہ_text64
#تعریف_کرو sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#تعریف_کرو sqlite3_قدر_dup              sqlite3_api->قدر_dup
#تعریف_کرو sqlite3_قدر_free             sqlite3_api->قدر_free
#تعریف_کرو sqlite3_نتیجہ_zeroblob64      sqlite3_api->نتیجہ_zeroblob64
#تعریف_کرو sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#تعریف_کرو sqlite3_قدر_subtype          sqlite3_api->قدر_subtype
#تعریف_کرو sqlite3_نتیجہ_subtype         sqlite3_api->نتیجہ_subtype
/* Version 3.10.0 and later */
#تعریف_کرو sqlite3_status64               sqlite3_api->status64
#تعریف_کرو sqlite3_strlike                sqlite3_api->strlike
#تعریف_کرو sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#تعریف_کرو sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#تعریف_کرو sqlite3_trace_v2               sqlite3_api->trace_v2
#تعریف_کرو sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#تعریف_کرو sqlite3_سیٹ_آخری_داخل_کرو_rowid  sqlite3_api->سیٹ_آخری_داخل_کرو_rowid
/* Version 3.20.0 and later */
#تعریف_کرو sqlite3_prepare_v3             sqlite3_api->prepare_v3
#تعریف_کرو sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#تعریف_کرو sqlite3_bind_اشارہ_گر           sqlite3_api->bind_اشارہ_گر
#تعریف_کرو sqlite3_نتیجہ_اشارہ_گر         sqlite3_api->نتیجہ_اشارہ_گر
#تعریف_کرو sqlite3_قدر_اشارہ_گر          sqlite3_api->قدر_اشارہ_گر
/* Version 3.22.0 and later */
#تعریف_کرو sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#تعریف_کرو sqlite3_قدر_nochange         sqlite3_api->قدر_nochange
#تعریف_کرو sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#تعریف_کرو sqlite3_keyword_گنتی          sqlite3_api->keyword_گنتی
#تعریف_کرو sqlite3_keyword_name           sqlite3_api->keyword_name
#تعریف_کرو sqlite3_keyword_check          sqlite3_api->keyword_check
#تعریف_کرو sqlite3_str_new                sqlite3_api->str_new
#تعریف_کرو sqlite3_str_اختتام             sqlite3_api->str_اختتام
#تعریف_کرو sqlite3_str_appendf            sqlite3_api->str_appendf
#تعریف_کرو sqlite3_str_vappendf           sqlite3_api->str_vappendf
#تعریف_کرو sqlite3_str_append             sqlite3_api->str_append
#تعریف_کرو sqlite3_str_appendall          sqlite3_api->str_appendall
#تعریف_کرو sqlite3_str_appendchar         sqlite3_api->str_appendchar
#تعریف_کرو sqlite3_str_reset              sqlite3_api->str_reset
#تعریف_کرو sqlite3_str_errcode            sqlite3_api->str_errcode
#تعریف_کرو sqlite3_str_لمبائی             sqlite3_api->str_لمبائی
#تعریف_کرو sqlite3_str_قدر              sqlite3_api->str_قدر
/* Version 3.25.0 and later */
#تعریف_کرو sqlite3_create_کھڑکی_function sqlite3_api->create_کھڑکی_function
/* Version 3.26.0 and later */
#تعریف_کرو sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#تعریف_کرو sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#تعریف_کرو sqlite3_قدر_frombind         sqlite3_api->قدر_frombind
/* Version 3.30.0 and later */
#تعریف_کرو sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#تعریف_کرو sqlite3_hard_ڈھیر_limit64      sqlite3_api->hard_ڈھیر_limit64
#تعریف_کرو sqlite3_uri_کلید                sqlite3_api->uri_کلید
#تعریف_کرو sqlite3_filename_database      sqlite3_api->filename_database
#تعریف_کرو sqlite3_filename_journal       sqlite3_api->filename_journal
#تعریف_کرو sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#تعریف_کرو sqlite3_create_filename        sqlite3_api->create_filename
#تعریف_کرو sqlite3_free_filename          sqlite3_api->free_filename
#تعریف_کرو sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#تعریف_کرو sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#تعریف_کرو sqlite3_changes64              sqlite3_api->changes64
#تعریف_کرو sqlite3_کل_changes64        sqlite3_api->کل_changes64
/* Version 3.37.0 and later */
#تعریف_کرو sqlite3_autovacuum_صفحات       sqlite3_api->autovacuum_صفحات
/* Version 3.38.0 and later */
#تعریف_کرو sqlite3_error_انحراف           sqlite3_api->error_انحراف
#تعریف_کرو sqlite3_vtab_rhs_قدر         sqlite3_api->vtab_rhs_قدر
#تعریف_کرو sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#تعریف_کرو sqlite3_vtab_اندر                sqlite3_api->vtab_اندر
#تعریف_کرو sqlite3_vtab_اندر_پہلا          sqlite3_api->vtab_اندر_پہلا
#تعریف_کرو sqlite3_vtab_اندر_اگلا           sqlite3_api->vtab_اندر_اگلا
/* Version 3.39.0 and later */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DESERIALIZE
#تعریف_کرو sqlite3_deserialize            sqlite3_api->deserialize
#تعریف_کرو sqlite3_serialize              sqlite3_api->serialize
#اگر_ختم
#تعریف_کرو sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#تعریف_کرو sqlite3_قدر_encoding         sqlite3_api->قدر_encoding
/* Version 3.41.0 and later */
#تعریف_کرو sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#تعریف_کرو sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#تعریف_کرو sqlite3_get_clientdata         sqlite3_api->get_clientdata
#تعریف_کرو sqlite3_سیٹ_clientdata         sqlite3_api->سیٹ_clientdata
/* Version 3.50.0 and later */
#تعریف_کرو sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#تعریف_کرو sqlite3_سیٹ_errmsg             sqlite3_api->سیٹ_errmsg
#تعریف_کرو sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#تعریف_کرو sqlite3_str_truncate           sqlite3_api->str_truncate
#تعریف_کرو sqlite3_str_free               sqlite3_api->str_free
#تعریف_کرو sqlite3_carray_bind            sqlite3_api->carray_bind
#تعریف_کرو sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#تعریف_کرو sqlite3_incomplete             sqlite3_api->incomplete
#تعریف_کرو sqlite3_نتیجہ_str             sqlite3_api->نتیجہ_str
#اگر_ختم /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#اگر !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_بھرو_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ثابت sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    خارجی ثابت sqlite3_api_routines *sqlite3_api;
#ورنہ
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (خالی)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#اگر_ختم

#اگر_ختم /* SQLITE3EXT_H */
