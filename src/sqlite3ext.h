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
#جے_بنا_تعریف SQLITE3EXT_H
#تعریف_کرو SQLITE3EXT_H
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
  فراغ * (*aggregate_context)(sqlite3_context*,صحيح n_بائٹاں);
  صحيح  (*aggregate_گنتی)(sqlite3_context*);
  صحيح  (*bind_blob)(sqlite3_stmt*,صحيح,ثابت فراغ*,صحيح n,فراغ(*)(فراغ*));
  صحيح  (*bind_double)(sqlite3_stmt*,صحيح,مزدوج);
  صحيح  (*bind_int)(sqlite3_stmt*,صحيح,صحيح);
  صحيح  (*bind_int64)(sqlite3_stmt*,صحيح,sqlite_int64);
  صحيح  (*bind_null)(sqlite3_stmt*,صحيح);
  صحيح  (*bind_parameter_گنتی)(sqlite3_stmt*);
  صحيح  (*bind_parameter_اشاریہ)(sqlite3_stmt*,ثابت حرف*zName);
  ثابت حرف * (*bind_parameter_name)(sqlite3_stmt*,صحيح);
  صحيح  (*bind_متن)(sqlite3_stmt*,صحيح,ثابت حرف*,صحيح n,فراغ(*)(فراغ*));
  صحيح  (*bind_text16)(sqlite3_stmt*,صحيح,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  صحيح  (*bind_قدر)(sqlite3_stmt*,صحيح,ثابت sqlite3_قدر*);
  صحيح  (*busy_handler)(sqlite3*,صحيح(*)(فراغ*,صحيح),فراغ*);
  صحيح  (*busy_timeout)(sqlite3*,صحيح ms);
  صحيح  (*changes)(sqlite3*);
  صحيح  (*close)(sqlite3*);
  صحيح  (*collation_needed)(sqlite3*,فراغ*,فراغ(*)(فراغ*,sqlite3*,
                           صحيح e_متن_Rep,ثابت حرف*));
  صحيح  (*collation_needed16)(sqlite3*,فراغ*,فراغ(*)(فراغ*,sqlite3*,
                             صحيح e_متن_Rep,ثابت فراغ*));
  ثابت فراغ * (*کالم_blob)(sqlite3_stmt*,صحيح i_خانہ);
  صحيح  (*کالم_بائٹاں)(sqlite3_stmt*,صحيح i_خانہ);
  صحيح  (*کالم_bytes16)(sqlite3_stmt*,صحيح i_خانہ);
  صحيح  (*کالم_گنتی)(sqlite3_stmt*pStmt);
  ثابت حرف * (*کالم_database_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*کالم_database_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*کالم_decltype)(sqlite3_stmt*,صحيح i);
  ثابت فراغ * (*کالم_decltype16)(sqlite3_stmt*,صحيح);
  مزدوج  (*کالم_double)(sqlite3_stmt*,صحيح i_خانہ);
  صحيح  (*کالم_int)(sqlite3_stmt*,صحيح i_خانہ);
  sqlite_int64  (*کالم_int64)(sqlite3_stmt*,صحيح i_خانہ);
  ثابت حرف * (*کالم_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*کالم_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*کالم_origin_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*کالم_origin_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*کالم_جدول_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*کالم_جدول_name16)(sqlite3_stmt*,صحيح);
  ثابت بنانشانی حرف * (*کالم_متن)(sqlite3_stmt*,صحيح i_خانہ);
  ثابت فراغ * (*کالم_text16)(sqlite3_stmt*,صحيح i_خانہ);
  صحيح  (*کالم_type)(sqlite3_stmt*,صحيح i_خانہ);
  sqlite3_قدر* (*کالم_قدر)(sqlite3_stmt*,صحيح i_خانہ);
  فراغ * (*commit_hook)(sqlite3*,صحيح(*)(فراغ*),فراغ*);
  صحيح  (*complete)(ثابت حرف*sql);
  صحيح  (*complete16)(ثابت فراغ*sql);
  صحيح  (*create_collation)(sqlite3*,ثابت حرف*,صحيح,فراغ*,
                           صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*));
  صحيح  (*create_collation16)(sqlite3*,ثابت فراغ*,صحيح,فراغ*,
                             صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*));
  صحيح  (*create_function)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                          فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_قدر**),
                          فراغ (*x_قدم)(sqlite3_context*,صحيح,sqlite3_قدر**),
                          فراغ (*xFinal)(sqlite3_context*));
  صحيح  (*create_function16)(sqlite3*,ثابت فراغ*,صحيح,صحيح,فراغ*,
                            فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_قدر**),
                            فراغ (*x_قدم)(sqlite3_context*,صحيح,sqlite3_قدر**),
                            فراغ (*xFinal)(sqlite3_context*));
  صحيح (*create_module)(sqlite3*,ثابت حرف*,ثابت sqlite3_module*,فراغ*);
  صحيح  (*انکڑے_گنتی)(sqlite3_stmt*pStmt);
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
  فراغ  (*free_جدول)(حرف**نتیجہ);
  صحيح  (*get_autocommit)(sqlite3*);
  فراغ * (*get_auxdata)(sqlite3_context*,صحيح);
  صحيح  (*get_جدول)(sqlite3*,ثابت حرف*,حرف***,صحيح*,صحيح*,حرف**);
  صحيح  (*global_recover)(فراغ);
  فراغ  (*interruptx)(sqlite3*);
  sqlite_int64  (*آخری_داخل_کرو_rowid)(sqlite3*);
  ثابت حرف * (*libversion)(فراغ);
  صحيح  (*libversion_نمبر)(فراغ);
  فراغ *(*حافظہ_دیو)(صحيح);
  حرف *(SQLITE_CDECL*mprintf)(ثابت حرف*,...);
  صحيح  (*open)(ثابت حرف*,sqlite3**);
  صحيح  (*open16)(ثابت فراغ*,sqlite3**);
  صحيح  (*prepare)(sqlite3*,ثابت حرف*,صحيح,sqlite3_stmt**,ثابت حرف**);
  صحيح  (*prepare16)(sqlite3*,ثابت فراغ*,صحيح,sqlite3_stmt**,ثابت فراغ**);
  فراغ * (*profile)(sqlite3*,فراغ(*)(فراغ*,ثابت حرف*,sqlite_uint64),فراغ*);
  فراغ  (*ترقی_handler)(sqlite3*,صحيح,صحيح(*)(فراغ*),فراغ*);
  فراغ *(*realloc)(فراغ*,صحيح);
  صحيح  (*reset)(sqlite3_stmt*pStmt);
  فراغ  (*نتیجہ_blob)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجہ_double)(sqlite3_context*,مزدوج);
  فراغ  (*نتیجہ_error)(sqlite3_context*,ثابت حرف*,صحيح);
  فراغ  (*نتیجہ_error16)(sqlite3_context*,ثابت فراغ*,صحيح);
  فراغ  (*نتیجہ_int)(sqlite3_context*,صحيح);
  فراغ  (*نتیجہ_int64)(sqlite3_context*,sqlite_int64);
  فراغ  (*نتیجہ_null)(sqlite3_context*);
  فراغ  (*نتیجہ_متن)(sqlite3_context*,ثابت حرف*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجہ_text16)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجہ_text16be)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجہ_text16le)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجہ_قدر)(sqlite3_context*,sqlite3_قدر*);
  فراغ *(*rollback_hook)(sqlite3*,فراغ(*)(فراغ*),فراغ*);
  صحيح  (*سیٹ_authorizer)(sqlite3*,صحيح(*)(فراغ*,صحيح,ثابت حرف*,ثابت حرف*,
                         ثابت حرف*,ثابت حرف*),فراغ*);
  فراغ  (*سیٹ_auxdata)(sqlite3_context*,صحيح,فراغ*,فراغ (*)(فراغ*));
  حرف *(SQLITE_CDECL*xsnprintf)(صحيح,حرف*,ثابت حرف*,...);
  صحيح  (*قدم)(sqlite3_stmt*);
  صحيح  (*جدول_کالم_metadata)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,
                                حرف ثابت**,حرف ثابت**,صحيح*,صحيح*,صحيح*);
  فراغ  (*thread_cleanup)(فراغ);
  صحيح  (*کل_changes)(sqlite3*);
  فراغ * (*trace)(sqlite3*,فراغ(*xTrace)(فراغ*,ثابت حرف*),فراغ*);
  صحيح  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  فراغ * (*update_hook)(sqlite3*,فراغ(*)(فراغ*,صحيح ,حرف ثابت*,حرف ثابت*,
                                         sqlite_int64),فراغ*);
  فراغ * (*user_انکڑے)(sqlite3_context*);
  ثابت فراغ * (*قدر_blob)(sqlite3_قدر*);
  صحيح  (*قدر_بائٹاں)(sqlite3_قدر*);
  صحيح  (*قدر_bytes16)(sqlite3_قدر*);
  مزدوج  (*قدر_double)(sqlite3_قدر*);
  صحيح  (*قدر_int)(sqlite3_قدر*);
  sqlite_int64  (*قدر_int64)(sqlite3_قدر*);
  صحيح  (*قدر_numeric_type)(sqlite3_قدر*);
  ثابت بنانشانی حرف * (*قدر_متن)(sqlite3_قدر*);
  ثابت فراغ * (*قدر_text16)(sqlite3_قدر*);
  ثابت فراغ * (*قدر_text16be)(sqlite3_قدر*);
  ثابت فراغ * (*قدر_text16le)(sqlite3_قدر*);
  صحيح  (*قدر_type)(sqlite3_قدر*);
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
  صحيح (*blob_بائٹاں)(sqlite3_blob*);
  صحيح (*blob_close)(sqlite3_blob*);
  صحيح (*blob_open)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,sqlite3_int64,
                   صحيح,sqlite3_blob**);
  صحيح (*blob_read)(sqlite3_blob*,فراغ*,صحيح,صحيح);
  صحيح (*blob_write)(sqlite3_blob*,ثابت فراغ*,صحيح,صحيح);
  صحيح (*create_collation_v2)(sqlite3*,ثابت حرف*,صحيح,فراغ*,
                             صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*),
                             فراغ(*)(فراغ*));
  صحيح (*file_control)(sqlite3*,ثابت حرف*,صحيح,فراغ*);
  sqlite3_int64 (*حافظہ_highwater)(صحيح);
  sqlite3_int64 (*حافظہ_ورتیا)(فراغ);
  sqlite3_mutex *(*mutex_alloc)(صحيح);
  فراغ (*mutex_enter)(sqlite3_mutex*);
  فراغ (*mutex_free)(sqlite3_mutex*);
  فراغ (*mutex_leave)(sqlite3_mutex*);
  صحيح (*mutex_try)(sqlite3_mutex*);
  صحيح (*open_v2)(ثابت حرف*,sqlite3**,صحيح,ثابت حرف*);
  صحيح (*release_حافظہ)(صحيح);
  فراغ (*نتیجہ_error_nomem)(sqlite3_context*);
  فراغ (*نتیجہ_error_toobig)(sqlite3_context*);
  صحيح (*sleep)(صحيح);
  فراغ (*soft_ڈھیر_حد_بندی)(صحيح);
  sqlite3_vfs *(*vfs_لبھو)(ثابت حرف*);
  صحيح (*vfs_رجسٹر)(sqlite3_vfs*,صحيح);
  صحيح (*vfs_unregister)(sqlite3_vfs*);
  صحيح (*xthreadsafe)(فراغ);
  فراغ (*نتیجہ_zeroblob)(sqlite3_context*,صحيح);
  فراغ (*نتیجہ_error_کوڈ)(sqlite3_context*,صحيح);
  صحيح (SQLITE_CDECL*test_control)(صحيح, ...);
  فراغ (*randomness)(صحيح,فراغ*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  صحيح (*extended_نتیجہ_codes)(sqlite3*,صحيح);
  صحيح (*حد_بندی)(sqlite3*,صحيح,صحيح);
  sqlite3_stmt *(*اگلا_stmt)(sqlite3*,sqlite3_stmt*);
  ثابت حرف *(*sql)(sqlite3_stmt*);
  صحيح (*حالت)(صحيح,صحيح*,صحيح*,صحيح);
  صحيح (*backup_انت)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ثابت حرف*,sqlite3*,ثابت حرف*);
  صحيح (*backup_pagecount)(sqlite3_backup*);
  صحيح (*backup_بچیا)(sqlite3_backup*);
  صحيح (*backup_قدم)(sqlite3_backup*,صحيح);
  ثابت حرف *(*compileoption_get)(صحيح);
  صحيح (*compileoption_ورتیا)(ثابت حرف*);
  صحيح (*create_function_v2)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                            فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_قدر**),
                            فراغ (*x_قدم)(sqlite3_context*,صحيح,sqlite3_قدر**),
                            فراغ (*xFinal)(sqlite3_context*),
                            فراغ(*xDestroy)(فراغ*));
  صحيح (SQLITE_CDECL*db_config)(sqlite3*,صحيح,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  صحيح (*db_حالت)(sqlite3*,صحيح,صحيح*,صحيح*,صحيح);
  صحيح (*extended_errcode)(sqlite3*);
  فراغ (SQLITE_CDECL*لوگارتھم)(صحيح,ثابت حرف*,...);
  sqlite3_int64 (*soft_ڈھیر_limit64)(sqlite3_int64);
  ثابت حرف *(*sourceid)(فراغ);
  صحيح (*stmt_حالت)(sqlite3_stmt*,صحيح,صحيح);
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
  صحيح (*db_release_حافظہ)(sqlite3*);
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
                      فراغ(*)(فراغ*),بنانشانی حرف);
  صحيح (*cancel_auto_extension)(فراغ(*)(فراغ));
  صحيح (*لدو_extension)(sqlite3*,ثابت حرف*,ثابت حرف*,حرف**);
  فراغ *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(فراغ*);
  فراغ *(*realloc64)(فراغ*,sqlite3_uint64);
  فراغ (*reset_auto_extension)(فراغ);
  فراغ (*نتیجہ_blob64)(sqlite3_context*,ثابت فراغ*,sqlite3_uint64,
                        فراغ(*)(فراغ*));
  فراغ (*نتیجہ_text64)(sqlite3_context*,ثابت حرف*,sqlite3_uint64,
                         فراغ(*)(فراغ*), بنانشانی حرف);
  صحيح (*strglob)(ثابت حرف*,ثابت حرف*);
  /* Version 3.8.11 and later */
  sqlite3_قدر *(*قدر_dup)(ثابت sqlite3_قدر*);
  فراغ (*قدر_free)(sqlite3_قدر*);
  صحيح (*نتیجہ_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  صحيح (*bind_zeroblob64)(sqlite3_stmt*, صحيح, sqlite3_uint64);
  /* Version 3.9.0 and later */
  بنانشانی صحيح (*قدر_subtype)(sqlite3_قدر*);
  فراغ (*نتیجہ_subtype)(sqlite3_context*,بنانشانی صحيح);
  /* Version 3.10.0 and later */
  صحيح (*status64)(صحيح,sqlite3_int64*,sqlite3_int64*,صحيح);
  صحيح (*strlike)(ثابت حرف*,ثابت حرف*,بنانشانی صحيح);
  صحيح (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  صحيح (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  صحيح (*trace_v2)(sqlite3*,بنانشانی,صحيح(*)(بنانشانی,فراغ*,فراغ*,فراغ*),فراغ*);
  حرف *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  فراغ (*سیٹ_آخری_داخل_کرو_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  صحيح (*prepare_v3)(sqlite3*,ثابت حرف*,صحيح,بنانشانی صحيح,
                    sqlite3_stmt**,ثابت حرف**);
  صحيح (*prepare16_v3)(sqlite3*,ثابت فراغ*,صحيح,بنانشانی صحيح,
                      sqlite3_stmt**,ثابت فراغ**);
  صحيح (*bind_اشارہ_گر)(sqlite3_stmt*,صحيح,فراغ*,ثابت حرف*,فراغ(*)(فراغ*));
  فراغ (*نتیجہ_اشارہ_گر)(sqlite3_context*,فراغ*,ثابت حرف*,فراغ(*)(فراغ*));
  فراغ *(*قدر_اشارہ_گر)(sqlite3_قدر*,ثابت حرف*);
  صحيح (*vtab_nochange)(sqlite3_context*);
  صحيح (*قدر_nochange)(sqlite3_قدر*);
  ثابت حرف *(*vtab_collation)(sqlite3_اشاریہ_info*,صحيح);
  /* Version 3.24.0 and later */
  صحيح (*keyword_گنتی)(فراغ);
  صحيح (*keyword_name)(صحيح,ثابت حرف**,صحيح*);
  صحيح (*keyword_check)(ثابت حرف*,صحيح);
  sqlite3_str *(*str_new)(sqlite3*);
  حرف *(*str_انت)(sqlite3_str*);
  فراغ (SQLITE_CDECL*str_appendf)(sqlite3_str*, ثابت حرف *zFormat, ...);
  فراغ (*str_vappendf)(sqlite3_str*, ثابت حرف *zFormat, va_list);
  فراغ (*str_append)(sqlite3_str*, ثابت حرف *z_وچ, صحيح N);
  فراغ (*str_appendall)(sqlite3_str*, ثابت حرف *z_وچ);
  فراغ (*str_appendchar)(sqlite3_str*, صحيح N, حرف C);
  فراغ (*str_reset)(sqlite3_str*);
  صحيح (*str_errcode)(sqlite3_str*);
  صحيح (*str_لمائی)(sqlite3_str*);
  حرف *(*str_قدر)(sqlite3_str*);
  /* Version 3.25.0 and later */
  صحيح (*create_کھڑکی_function)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                            فراغ (*x_قدم)(sqlite3_context*,صحيح,sqlite3_قدر**),
                            فراغ (*xFinal)(sqlite3_context*),
                            فراغ (*x_قدر)(sqlite3_context*),
                            فراغ (*xInv)(sqlite3_context*,صحيح,sqlite3_قدر**),
                            فراغ(*xDestroy)(فراغ*));
  /* Version 3.26.0 and later */
  ثابت حرف *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  صحيح (*stmt_isexplain)(sqlite3_stmt*);
  صحيح (*قدر_frombind)(sqlite3_قدر*);
  /* Version 3.30.0 and later */
  صحيح (*drop_modules)(sqlite3*,ثابت حرف**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_ڈھیر_limit64)(sqlite3_int64);
  ثابت حرف *(*uri_کلید)(ثابت حرف*,صحيح);
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
  sqlite3_int64 (*کل_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  صحيح (*autovacuum_ورقے)(sqlite3*,
     بنانشانی صحيح(*)(فراغ*,ثابت حرف*,بنانشانی صحيح,بنانشانی صحيح,بنانشانی صحيح),
     فراغ*, فراغ(*)(فراغ*));
  /* Version 3.38.0 and later */
  صحيح (*error_وتھ)(sqlite3*);
  صحيح (*vtab_rhs_قدر)(sqlite3_اشاریہ_info*,صحيح,sqlite3_قدر**);
  صحيح (*vtab_distinct)(sqlite3_اشاریہ_info*);
  صحيح (*vtab_وچ)(sqlite3_اشاریہ_info*,صحيح,صحيح);
  صحيح (*vtab_وچ_پہلا)(sqlite3_قدر*,sqlite3_قدر**);
  صحيح (*vtab_وچ_اگلا)(sqlite3_قدر*,sqlite3_قدر**);
  /* Version 3.39.0 and later */
  صحيح (*deserialize)(sqlite3*,ثابت حرف*,بنانشانی حرف*,
                     sqlite3_int64,sqlite3_int64,بنانشانی);
  بنانشانی حرف *(*serialize)(sqlite3*,ثابت حرف *,sqlite3_int64*,
                              بنانشانی صحيح);
  ثابت حرف *(*db_name)(sqlite3*,صحيح);
  /* Version 3.40.0 and later */
  صحيح (*قدر_encoding)(sqlite3_قدر*);
  /* Version 3.41.0 and later */
  صحيح (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  صحيح (*stmt_explain)(sqlite3_stmt*,صحيح);
  /* Version 3.44.0 and later */
  فراغ *(*get_clientdata)(sqlite3*,ثابت حرف*);
  صحيح (*سیٹ_clientdata)(sqlite3*, ثابت حرف*, فراغ*, فراغ(*)(فراغ*));
  /* Version 3.50.0 and later */
  صحيح (*setlk_timeout)(sqlite3*,صحيح,صحيح);
  /* Version 3.51.0 and later */
  صحيح (*سیٹ_errmsg)(sqlite3*,صحيح,ثابت حرف*);
  صحيح (*db_status64)(sqlite3*,صحيح,sqlite3_int64*,sqlite3_int64*,صحيح);
  /* Version 3.52.0 and later */
  فراغ (*str_truncate)(sqlite3_str*,صحيح);
  فراغ (*str_free)(sqlite3_str*);
  صحيح (*carray_bind)(sqlite3_stmt*,صحيح,فراغ*,صحيح,صحيح,فراغ(*)(فراغ*));
  صحيح (*carray_bind_v2)(sqlite3_stmt*,صحيح,فراغ*,صحيح,صحيح,فراغ(*)(فراغ*),فراغ*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ثابت حرف*);
  فراغ (*نتیجہ_str)(sqlite3_context*,sqlite3_str*,صحيح);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
تعريفنوع صحيح (*sqlite3_loadext_اندراج)(
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
#إذا !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_لدو_EXTENSION)
#تعریف_کرو sqlite3_aggregate_context      sqlite3_api->aggregate_context
#جے_بنا_تعریف SQLITE_OMIT_DEPRECATED
#تعریف_کرو sqlite3_aggregate_گنتی        sqlite3_api->aggregate_گنتی
#جے_انت
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
#تعریف_کرو sqlite3_کالم_blob            sqlite3_api->کالم_blob
#تعریف_کرو sqlite3_کالم_بائٹاں           sqlite3_api->کالم_بائٹاں
#تعریف_کرو sqlite3_کالم_bytes16         sqlite3_api->کالم_bytes16
#تعریف_کرو sqlite3_کالم_گنتی           sqlite3_api->کالم_گنتی
#تعریف_کرو sqlite3_کالم_database_name   sqlite3_api->کالم_database_name
#تعریف_کرو sqlite3_کالم_database_name16 sqlite3_api->کالم_database_name16
#تعریف_کرو sqlite3_کالم_decltype        sqlite3_api->کالم_decltype
#تعریف_کرو sqlite3_کالم_decltype16      sqlite3_api->کالم_decltype16
#تعریف_کرو sqlite3_کالم_double          sqlite3_api->کالم_double
#تعریف_کرو sqlite3_کالم_int             sqlite3_api->کالم_int
#تعریف_کرو sqlite3_کالم_int64           sqlite3_api->کالم_int64
#تعریف_کرو sqlite3_کالم_name            sqlite3_api->کالم_name
#تعریف_کرو sqlite3_کالم_name16          sqlite3_api->کالم_name16
#تعریف_کرو sqlite3_کالم_origin_name     sqlite3_api->کالم_origin_name
#تعریف_کرو sqlite3_کالم_origin_name16   sqlite3_api->کالم_origin_name16
#تعریف_کرو sqlite3_کالم_جدول_name      sqlite3_api->کالم_جدول_name
#تعریف_کرو sqlite3_کالم_جدول_name16    sqlite3_api->کالم_جدول_name16
#تعریف_کرو sqlite3_کالم_متن            sqlite3_api->کالم_متن
#تعریف_کرو sqlite3_کالم_text16          sqlite3_api->کالم_text16
#تعریف_کرو sqlite3_کالم_type            sqlite3_api->کالم_type
#تعریف_کرو sqlite3_کالم_قدر           sqlite3_api->کالم_قدر
#تعریف_کرو sqlite3_commit_hook            sqlite3_api->commit_hook
#تعریف_کرو sqlite3_complete               sqlite3_api->complete
#تعریف_کرو sqlite3_complete16             sqlite3_api->complete16
#تعریف_کرو sqlite3_create_collation       sqlite3_api->create_collation
#تعریف_کرو sqlite3_create_collation16     sqlite3_api->create_collation16
#تعریف_کرو sqlite3_create_function        sqlite3_api->create_function
#تعریف_کرو sqlite3_create_function16      sqlite3_api->create_function16
#تعریف_کرو sqlite3_create_module          sqlite3_api->create_module
#تعریف_کرو sqlite3_create_module_v2       sqlite3_api->create_module_v2
#تعریف_کرو sqlite3_انکڑے_گنتی             sqlite3_api->انکڑے_گنتی
#تعریف_کرو sqlite3_db_handle              sqlite3_api->db_handle
#تعریف_کرو sqlite3_declare_vtab           sqlite3_api->declare_vtab
#تعریف_کرو sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#تعریف_کرو sqlite3_errcode                sqlite3_api->errcode
#تعریف_کرو sqlite3_errmsg                 sqlite3_api->errmsg
#تعریف_کرو sqlite3_errmsg16               sqlite3_api->errmsg16
#تعریف_کرو sqlite3_exec                   sqlite3_api->exec
#جے_بنا_تعریف SQLITE_OMIT_DEPRECATED
#تعریف_کرو sqlite3_expired                sqlite3_api->expired
#جے_انت
#تعریف_کرو sqlite3_finalize               sqlite3_api->finalize
#تعریف_کرو sqlite3_free                   sqlite3_api->حرر
#تعریف_کرو sqlite3_free_جدول             sqlite3_api->free_جدول
#تعریف_کرو sqlite3_get_autocommit         sqlite3_api->get_autocommit
#تعریف_کرو sqlite3_get_auxdata            sqlite3_api->get_auxdata
#تعریف_کرو sqlite3_get_جدول              sqlite3_api->get_جدول
#جے_بنا_تعریف SQLITE_OMIT_DEPRECATED
#تعریف_کرو sqlite3_global_recover         sqlite3_api->global_recover
#جے_انت
#تعریف_کرو sqlite3_رکاوٹ              sqlite3_api->interruptx
#تعریف_کرو sqlite3_آخری_داخل_کرو_rowid      sqlite3_api->آخری_داخل_کرو_rowid
#تعریف_کرو sqlite3_libversion             sqlite3_api->libversion
#تعریف_کرو sqlite3_libversion_نمبر      sqlite3_api->libversion_نمبر
#تعریف_کرو sqlite3_malloc                 sqlite3_api->حافظہ_دیو
#تعریف_کرو sqlite3_mprintf                sqlite3_api->mprintf
#تعریف_کرو sqlite3_open                   sqlite3_api->open
#تعریف_کرو sqlite3_open16                 sqlite3_api->open16
#تعریف_کرو sqlite3_prepare                sqlite3_api->prepare
#تعریف_کرو sqlite3_prepare16              sqlite3_api->prepare16
#تعریف_کرو sqlite3_prepare_v2             sqlite3_api->prepare_v2
#تعریف_کرو sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#تعریف_کرو sqlite3_profile                sqlite3_api->profile
#تعریف_کرو sqlite3_ترقی_handler       sqlite3_api->ترقی_handler
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
#تعریف_کرو sqlite3_جدول_کالم_metadata  sqlite3_api->جدول_کالم_metadata
#تعریف_کرو sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#تعریف_کرو sqlite3_کل_changes          sqlite3_api->کل_changes
#تعریف_کرو sqlite3_trace                  sqlite3_api->trace
#جے_بنا_تعریف SQLITE_OMIT_DEPRECATED
#تعریف_کرو sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#جے_انت
#تعریف_کرو sqlite3_update_hook            sqlite3_api->update_hook
#تعریف_کرو sqlite3_user_انکڑے              sqlite3_api->user_انکڑے
#تعریف_کرو sqlite3_قدر_blob             sqlite3_api->قدر_blob
#تعریف_کرو sqlite3_قدر_بائٹاں            sqlite3_api->قدر_بائٹاں
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
#تعریف_کرو sqlite3_blob_بائٹاں             sqlite3_api->blob_بائٹاں
#تعریف_کرو sqlite3_blob_close             sqlite3_api->blob_close
#تعریف_کرو sqlite3_blob_open              sqlite3_api->blob_open
#تعریف_کرو sqlite3_blob_read              sqlite3_api->blob_read
#تعریف_کرو sqlite3_blob_write             sqlite3_api->blob_write
#تعریف_کرو sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#تعریف_کرو sqlite3_file_control           sqlite3_api->file_control
#تعریف_کرو sqlite3_حافظہ_highwater       sqlite3_api->حافظہ_highwater
#تعریف_کرو sqlite3_حافظہ_ورتیا            sqlite3_api->حافظہ_ورتیا
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
#تعریف_کرو sqlite3_vfs_لبھو               sqlite3_api->vfs_لبھو
#تعریف_کرو sqlite3_vfs_رجسٹر           sqlite3_api->vfs_رجسٹر
#تعریف_کرو sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#تعریف_کرو sqlite3_threadsafe             sqlite3_api->xthreadsafe
#تعریف_کرو sqlite3_نتیجہ_zeroblob        sqlite3_api->نتیجہ_zeroblob
#تعریف_کرو sqlite3_نتیجہ_error_کوڈ      sqlite3_api->نتیجہ_error_کوڈ
#تعریف_کرو sqlite3_test_control           sqlite3_api->test_control
#تعریف_کرو sqlite3_randomness             sqlite3_api->randomness
#تعریف_کرو sqlite3_context_db_handle      sqlite3_api->context_db_handle
#تعریف_کرو sqlite3_extended_نتیجہ_codes  sqlite3_api->extended_نتیجہ_codes
#تعریف_کرو sqlite3_حد_بندی                  sqlite3_api->حد_بندی
#تعریف_کرو sqlite3_اگلا_stmt              sqlite3_api->اگلا_stmt
#تعریف_کرو sqlite3_sql                    sqlite3_api->sql
#تعریف_کرو sqlite3_حالت                 sqlite3_api->حالت
#تعریف_کرو sqlite3_backup_انت          sqlite3_api->backup_انت
#تعریف_کرو sqlite3_backup_init            sqlite3_api->backup_init
#تعریف_کرو sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#تعریف_کرو sqlite3_backup_بچیا       sqlite3_api->backup_بچیا
#تعریف_کرو sqlite3_backup_قدم            sqlite3_api->backup_قدم
#تعریف_کرو sqlite3_compileoption_get      sqlite3_api->compileoption_get
#تعریف_کرو sqlite3_compileoption_ورتیا     sqlite3_api->compileoption_ورتیا
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
#تعریف_کرو sqlite3_لدو_extension         sqlite3_api->لدو_extension
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
#تعریف_کرو sqlite3_str_انت             sqlite3_api->str_انت
#تعریف_کرو sqlite3_str_appendf            sqlite3_api->str_appendf
#تعریف_کرو sqlite3_str_vappendf           sqlite3_api->str_vappendf
#تعریف_کرو sqlite3_str_append             sqlite3_api->str_append
#تعریف_کرو sqlite3_str_appendall          sqlite3_api->str_appendall
#تعریف_کرو sqlite3_str_appendchar         sqlite3_api->str_appendchar
#تعریف_کرو sqlite3_str_reset              sqlite3_api->str_reset
#تعریف_کرو sqlite3_str_errcode            sqlite3_api->str_errcode
#تعریف_کرو sqlite3_str_لمائی             sqlite3_api->str_لمائی
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
#تعریف_کرو sqlite3_autovacuum_ورقے       sqlite3_api->autovacuum_ورقے
/* Version 3.38.0 and later */
#تعریف_کرو sqlite3_error_وتھ           sqlite3_api->error_وتھ
#تعریف_کرو sqlite3_vtab_rhs_قدر         sqlite3_api->vtab_rhs_قدر
#تعریف_کرو sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#تعریف_کرو sqlite3_vtab_وچ                sqlite3_api->vtab_وچ
#تعریف_کرو sqlite3_vtab_وچ_پہلا          sqlite3_api->vtab_وچ_پہلا
#تعریف_کرو sqlite3_vtab_وچ_اگلا           sqlite3_api->vtab_وچ_اگلا
/* Version 3.39.0 and later */
#جے_بنا_تعریف SQLITE_OMIT_DESERIALIZE
#تعریف_کرو sqlite3_deserialize            sqlite3_api->deserialize
#تعریف_کرو sqlite3_serialize              sqlite3_api->serialize
#جے_انت
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
#جے_انت /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#إذا !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_لدو_EXTENSION)
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
#جے_انت

#جے_انت /* SQLITE3EXT_H */
