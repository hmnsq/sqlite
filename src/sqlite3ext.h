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
#جيڪڏهن_وصف_نه_ٿيل SQLITE3EXT_H
#وصف_ڪريو SQLITE3EXT_H
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
  فراغ * (*aggregate_context)(sqlite3_context*,صحيح n_بائيٽون);
  صحيح  (*aggregate_ڳڻپ)(sqlite3_context*);
  صحيح  (*bind_blob)(sqlite3_stmt*,صحيح,ثابت فراغ*,صحيح n,فراغ(*)(فراغ*));
  صحيح  (*bind_double)(sqlite3_stmt*,صحيح,مزدوج);
  صحيح  (*bind_int)(sqlite3_stmt*,صحيح,صحيح);
  صحيح  (*bind_int64)(sqlite3_stmt*,صحيح,sqlite_int64);
  صحيح  (*bind_null)(sqlite3_stmt*,صحيح);
  صحيح  (*bind_parameter_ڳڻپ)(sqlite3_stmt*);
  صحيح  (*bind_parameter_اشاريو)(sqlite3_stmt*,ثابت حرف*zName);
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
  ثابت فراغ * (*ڪالم_blob)(sqlite3_stmt*,صحيح i_کانو);
  صحيح  (*ڪالم_بائيٽون)(sqlite3_stmt*,صحيح i_کانو);
  صحيح  (*ڪالم_bytes16)(sqlite3_stmt*,صحيح i_کانو);
  صحيح  (*ڪالم_ڳڻپ)(sqlite3_stmt*pStmt);
  ثابت حرف * (*ڪالم_database_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*ڪالم_database_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*ڪالم_decltype)(sqlite3_stmt*,صحيح i);
  ثابت فراغ * (*ڪالم_decltype16)(sqlite3_stmt*,صحيح);
  مزدوج  (*ڪالم_double)(sqlite3_stmt*,صحيح i_کانو);
  صحيح  (*ڪالم_int)(sqlite3_stmt*,صحيح i_کانو);
  sqlite_int64  (*ڪالم_int64)(sqlite3_stmt*,صحيح i_کانو);
  ثابت حرف * (*ڪالم_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*ڪالم_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*ڪالم_origin_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*ڪالم_origin_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*ڪالم_جدول_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*ڪالم_جدول_name16)(sqlite3_stmt*,صحيح);
  ثابت بنانشان حرف * (*ڪالم_متن)(sqlite3_stmt*,صحيح i_کانو);
  ثابت فراغ * (*ڪالم_text16)(sqlite3_stmt*,صحيح i_کانو);
  صحيح  (*ڪالم_type)(sqlite3_stmt*,صحيح i_کانو);
  sqlite3_قدر* (*ڪالم_قدر)(sqlite3_stmt*,صحيح i_کانو);
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
  صحيح  (*معلومات_ڳڻپ)(sqlite3_stmt*pStmt);
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
  فراغ  (*free_جدول)(حرف**نتيجو);
  صحيح  (*get_autocommit)(sqlite3*);
  فراغ * (*get_auxdata)(sqlite3_context*,صحيح);
  صحيح  (*get_جدول)(sqlite3*,ثابت حرف*,حرف***,صحيح*,صحيح*,حرف**);
  صحيح  (*global_recover)(فراغ);
  فراغ  (*interruptx)(sqlite3*);
  sqlite_int64  (*آخري_وجهو_rowid)(sqlite3*);
  ثابت حرف * (*libversion)(فراغ);
  صحيح  (*libversion_عدد)(فراغ);
  فراغ *(*يادگيري_ڏيو)(صحيح);
  حرف *(SQLITE_CDECL*mprintf)(ثابت حرف*,...);
  صحيح  (*open)(ثابت حرف*,sqlite3**);
  صحيح  (*open16)(ثابت فراغ*,sqlite3**);
  صحيح  (*prepare)(sqlite3*,ثابت حرف*,صحيح,sqlite3_stmt**,ثابت حرف**);
  صحيح  (*prepare16)(sqlite3*,ثابت فراغ*,صحيح,sqlite3_stmt**,ثابت فراغ**);
  فراغ * (*profile)(sqlite3*,فراغ(*)(فراغ*,ثابت حرف*,sqlite_uint64),فراغ*);
  فراغ  (*ترقي_handler)(sqlite3*,صحيح,صحيح(*)(فراغ*),فراغ*);
  فراغ *(*realloc)(فراغ*,صحيح);
  صحيح  (*reset)(sqlite3_stmt*pStmt);
  فراغ  (*نتيجو_blob)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتيجو_double)(sqlite3_context*,مزدوج);
  فراغ  (*نتيجو_error)(sqlite3_context*,ثابت حرف*,صحيح);
  فراغ  (*نتيجو_error16)(sqlite3_context*,ثابت فراغ*,صحيح);
  فراغ  (*نتيجو_int)(sqlite3_context*,صحيح);
  فراغ  (*نتيجو_int64)(sqlite3_context*,sqlite_int64);
  فراغ  (*نتيجو_null)(sqlite3_context*);
  فراغ  (*نتيجو_متن)(sqlite3_context*,ثابت حرف*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتيجو_text16)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتيجو_text16be)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتيجو_text16le)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتيجو_قدر)(sqlite3_context*,sqlite3_قدر*);
  فراغ *(*rollback_hook)(sqlite3*,فراغ(*)(فراغ*),فراغ*);
  صحيح  (*سيٽ_authorizer)(sqlite3*,صحيح(*)(فراغ*,صحيح,ثابت حرف*,ثابت حرف*,
                         ثابت حرف*,ثابت حرف*),فراغ*);
  فراغ  (*سيٽ_auxdata)(sqlite3_context*,صحيح,فراغ*,فراغ (*)(فراغ*));
  حرف *(SQLITE_CDECL*xsnprintf)(صحيح,حرف*,ثابت حرف*,...);
  صحيح  (*قدم)(sqlite3_stmt*);
  صحيح  (*جدول_ڪالم_metadata)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,
                                حرف ثابت**,حرف ثابت**,صحيح*,صحيح*,صحيح*);
  فراغ  (*thread_cleanup)(فراغ);
  صحيح  (*ڪل_changes)(sqlite3*);
  فراغ * (*trace)(sqlite3*,فراغ(*xTrace)(فراغ*,ثابت حرف*),فراغ*);
  صحيح  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  فراغ * (*update_hook)(sqlite3*,فراغ(*)(فراغ*,صحيح ,حرف ثابت*,حرف ثابت*,
                                         sqlite_int64),فراغ*);
  فراغ * (*user_معلومات)(sqlite3_context*);
  ثابت فراغ * (*قدر_blob)(sqlite3_قدر*);
  صحيح  (*قدر_بائيٽون)(sqlite3_قدر*);
  صحيح  (*قدر_bytes16)(sqlite3_قدر*);
  مزدوج  (*قدر_double)(sqlite3_قدر*);
  صحيح  (*قدر_int)(sqlite3_قدر*);
  sqlite_int64  (*قدر_int64)(sqlite3_قدر*);
  صحيح  (*قدر_numeric_type)(sqlite3_قدر*);
  ثابت بنانشان حرف * (*قدر_متن)(sqlite3_قدر*);
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
  صحيح (*blob_بائيٽون)(sqlite3_blob*);
  صحيح (*blob_close)(sqlite3_blob*);
  صحيح (*blob_open)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,sqlite3_int64,
                   صحيح,sqlite3_blob**);
  صحيح (*blob_read)(sqlite3_blob*,فراغ*,صحيح,صحيح);
  صحيح (*blob_write)(sqlite3_blob*,ثابت فراغ*,صحيح,صحيح);
  صحيح (*create_collation_v2)(sqlite3*,ثابت حرف*,صحيح,فراغ*,
                             صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*),
                             فراغ(*)(فراغ*));
  صحيح (*file_control)(sqlite3*,ثابت حرف*,صحيح,فراغ*);
  sqlite3_int64 (*يادگيري_highwater)(صحيح);
  sqlite3_int64 (*يادگيري_استعمال_ٿيل)(فراغ);
  sqlite3_mutex *(*mutex_alloc)(صحيح);
  فراغ (*mutex_enter)(sqlite3_mutex*);
  فراغ (*mutex_free)(sqlite3_mutex*);
  فراغ (*mutex_leave)(sqlite3_mutex*);
  صحيح (*mutex_try)(sqlite3_mutex*);
  صحيح (*open_v2)(ثابت حرف*,sqlite3**,صحيح,ثابت حرف*);
  صحيح (*release_يادگيري)(صحيح);
  فراغ (*نتيجو_error_nomem)(sqlite3_context*);
  فراغ (*نتيجو_error_toobig)(sqlite3_context*);
  صحيح (*sleep)(صحيح);
  فراغ (*soft_ڍير_حد_بندي)(صحيح);
  sqlite3_vfs *(*vfs_ڳوليو)(ثابت حرف*);
  صحيح (*vfs_رجسٽر)(sqlite3_vfs*,صحيح);
  صحيح (*vfs_unregister)(sqlite3_vfs*);
  صحيح (*xthreadsafe)(فراغ);
  فراغ (*نتيجو_zeroblob)(sqlite3_context*,صحيح);
  فراغ (*نتيجو_error_ڪوڊ)(sqlite3_context*,صحيح);
  صحيح (SQLITE_CDECL*test_control)(صحيح, ...);
  فراغ (*randomness)(صحيح,فراغ*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  صحيح (*extended_نتيجو_codes)(sqlite3*,صحيح);
  صحيح (*حد_بندي)(sqlite3*,صحيح,صحيح);
  sqlite3_stmt *(*ايندڙ_stmt)(sqlite3*,sqlite3_stmt*);
  ثابت حرف *(*sql)(sqlite3_stmt*);
  صحيح (*حالت)(صحيح,صحيح*,صحيح*,صحيح);
  صحيح (*backup_پڄاڻي)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ثابت حرف*,sqlite3*,ثابت حرف*);
  صحيح (*backup_pagecount)(sqlite3_backup*);
  صحيح (*backup_بچيل)(sqlite3_backup*);
  صحيح (*backup_قدم)(sqlite3_backup*,صحيح);
  ثابت حرف *(*compileoption_get)(صحيح);
  صحيح (*compileoption_استعمال_ٿيل)(ثابت حرف*);
  صحيح (*create_function_v2)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                            فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_قدر**),
                            فراغ (*x_قدم)(sqlite3_context*,صحيح,sqlite3_قدر**),
                            فراغ (*xFinal)(sqlite3_context*),
                            فراغ(*xDestroy)(فراغ*));
  صحيح (SQLITE_CDECL*db_config)(sqlite3*,صحيح,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  صحيح (*db_حالت)(sqlite3*,صحيح,صحيح*,صحيح*,صحيح);
  صحيح (*extended_errcode)(sqlite3*);
  فراغ (SQLITE_CDECL*لاگرٿم)(صحيح,ثابت حرف*,...);
  sqlite3_int64 (*soft_ڍير_limit64)(sqlite3_int64);
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
  صحيح (*db_release_يادگيري)(sqlite3*);
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
                      فراغ(*)(فراغ*),بنانشان حرف);
  صحيح (*cancel_auto_extension)(فراغ(*)(فراغ));
  صحيح (*ڀريو_extension)(sqlite3*,ثابت حرف*,ثابت حرف*,حرف**);
  فراغ *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(فراغ*);
  فراغ *(*realloc64)(فراغ*,sqlite3_uint64);
  فراغ (*reset_auto_extension)(فراغ);
  فراغ (*نتيجو_blob64)(sqlite3_context*,ثابت فراغ*,sqlite3_uint64,
                        فراغ(*)(فراغ*));
  فراغ (*نتيجو_text64)(sqlite3_context*,ثابت حرف*,sqlite3_uint64,
                         فراغ(*)(فراغ*), بنانشان حرف);
  صحيح (*strglob)(ثابت حرف*,ثابت حرف*);
  /* Version 3.8.11 and later */
  sqlite3_قدر *(*قدر_dup)(ثابت sqlite3_قدر*);
  فراغ (*قدر_free)(sqlite3_قدر*);
  صحيح (*نتيجو_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  صحيح (*bind_zeroblob64)(sqlite3_stmt*, صحيح, sqlite3_uint64);
  /* Version 3.9.0 and later */
  بنانشان صحيح (*قدر_subtype)(sqlite3_قدر*);
  فراغ (*نتيجو_subtype)(sqlite3_context*,بنانشان صحيح);
  /* Version 3.10.0 and later */
  صحيح (*status64)(صحيح,sqlite3_int64*,sqlite3_int64*,صحيح);
  صحيح (*strlike)(ثابت حرف*,ثابت حرف*,بنانشان صحيح);
  صحيح (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  صحيح (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  صحيح (*trace_v2)(sqlite3*,بنانشان,صحيح(*)(بنانشان,فراغ*,فراغ*,فراغ*),فراغ*);
  حرف *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  فراغ (*سيٽ_آخري_وجهو_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  صحيح (*prepare_v3)(sqlite3*,ثابت حرف*,صحيح,بنانشان صحيح,
                    sqlite3_stmt**,ثابت حرف**);
  صحيح (*prepare16_v3)(sqlite3*,ثابت فراغ*,صحيح,بنانشان صحيح,
                      sqlite3_stmt**,ثابت فراغ**);
  صحيح (*bind_اشارو)(sqlite3_stmt*,صحيح,فراغ*,ثابت حرف*,فراغ(*)(فراغ*));
  فراغ (*نتيجو_اشارو)(sqlite3_context*,فراغ*,ثابت حرف*,فراغ(*)(فراغ*));
  فراغ *(*قدر_اشارو)(sqlite3_قدر*,ثابت حرف*);
  صحيح (*vtab_nochange)(sqlite3_context*);
  صحيح (*قدر_nochange)(sqlite3_قدر*);
  ثابت حرف *(*vtab_collation)(sqlite3_اشاريو_info*,صحيح);
  /* Version 3.24.0 and later */
  صحيح (*keyword_ڳڻپ)(فراغ);
  صحيح (*keyword_name)(صحيح,ثابت حرف**,صحيح*);
  صحيح (*keyword_check)(ثابت حرف*,صحيح);
  sqlite3_str *(*str_new)(sqlite3*);
  حرف *(*str_پڄاڻي)(sqlite3_str*);
  فراغ (SQLITE_CDECL*str_appendf)(sqlite3_str*, ثابت حرف *zFormat, ...);
  فراغ (*str_vappendf)(sqlite3_str*, ثابت حرف *zFormat, va_list);
  فراغ (*str_append)(sqlite3_str*, ثابت حرف *z_اندريون, صحيح N);
  فراغ (*str_appendall)(sqlite3_str*, ثابت حرف *z_اندريون);
  فراغ (*str_appendchar)(sqlite3_str*, صحيح N, حرف C);
  فراغ (*str_reset)(sqlite3_str*);
  صحيح (*str_errcode)(sqlite3_str*);
  صحيح (*str_ڊيگهائي)(sqlite3_str*);
  حرف *(*str_قدر)(sqlite3_str*);
  /* Version 3.25.0 and later */
  صحيح (*create_دري_function)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
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
  sqlite3_int64 (*hard_ڍير_limit64)(sqlite3_int64);
  ثابت حرف *(*uri_ڪنجي)(ثابت حرف*,صحيح);
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
  sqlite3_int64 (*ڪل_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  صحيح (*autovacuum_صفحا)(sqlite3*,
     بنانشان صحيح(*)(فراغ*,ثابت حرف*,بنانشان صحيح,بنانشان صحيح,بنانشان صحيح),
     فراغ*, فراغ(*)(فراغ*));
  /* Version 3.38.0 and later */
  صحيح (*error_وٿي)(sqlite3*);
  صحيح (*vtab_rhs_قدر)(sqlite3_اشاريو_info*,صحيح,sqlite3_قدر**);
  صحيح (*vtab_distinct)(sqlite3_اشاريو_info*);
  صحيح (*vtab_اندريون)(sqlite3_اشاريو_info*,صحيح,صحيح);
  صحيح (*vtab_اندريون_پهريون)(sqlite3_قدر*,sqlite3_قدر**);
  صحيح (*vtab_اندريون_ايندڙ)(sqlite3_قدر*,sqlite3_قدر**);
  /* Version 3.39.0 and later */
  صحيح (*deserialize)(sqlite3*,ثابت حرف*,بنانشان حرف*,
                     sqlite3_int64,sqlite3_int64,بنانشان);
  بنانشان حرف *(*serialize)(sqlite3*,ثابت حرف *,sqlite3_int64*,
                              بنانشان صحيح);
  ثابت حرف *(*db_name)(sqlite3*,صحيح);
  /* Version 3.40.0 and later */
  صحيح (*قدر_encoding)(sqlite3_قدر*);
  /* Version 3.41.0 and later */
  صحيح (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  صحيح (*stmt_explain)(sqlite3_stmt*,صحيح);
  /* Version 3.44.0 and later */
  فراغ *(*get_clientdata)(sqlite3*,ثابت حرف*);
  صحيح (*سيٽ_clientdata)(sqlite3*, ثابت حرف*, فراغ*, فراغ(*)(فراغ*));
  /* Version 3.50.0 and later */
  صحيح (*setlk_timeout)(sqlite3*,صحيح,صحيح);
  /* Version 3.51.0 and later */
  صحيح (*سيٽ_errmsg)(sqlite3*,صحيح,ثابت حرف*);
  صحيح (*db_status64)(sqlite3*,صحيح,sqlite3_int64*,sqlite3_int64*,صحيح);
  /* Version 3.52.0 and later */
  فراغ (*str_truncate)(sqlite3_str*,صحيح);
  فراغ (*str_free)(sqlite3_str*);
  صحيح (*carray_bind)(sqlite3_stmt*,صحيح,فراغ*,صحيح,صحيح,فراغ(*)(فراغ*));
  صحيح (*carray_bind_v2)(sqlite3_stmt*,صحيح,فراغ*,صحيح,صحيح,فراغ(*)(فراغ*),فراغ*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ثابت حرف*);
  فراغ (*نتيجو_str)(sqlite3_context*,sqlite3_str*,صحيح);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
تعريفنوع صحيح (*sqlite3_loadext_داخلا)(
  sqlite3 *db,                       /* Handle to the database. */
  حرف **pz_Err_نياپو,                   /* Used to set error string on failure. */
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
#إذا !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_ڀريو_EXTENSION)
#وصف_ڪريو sqlite3_aggregate_context      sqlite3_api->aggregate_context
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DEPRECATED
#وصف_ڪريو sqlite3_aggregate_ڳڻپ        sqlite3_api->aggregate_ڳڻپ
#جيڪڏهن_پورو
#وصف_ڪريو sqlite3_bind_blob              sqlite3_api->bind_blob
#وصف_ڪريو sqlite3_bind_double            sqlite3_api->bind_double
#وصف_ڪريو sqlite3_bind_int               sqlite3_api->bind_int
#وصف_ڪريو sqlite3_bind_int64             sqlite3_api->bind_int64
#وصف_ڪريو sqlite3_bind_null              sqlite3_api->bind_null
#وصف_ڪريو sqlite3_bind_parameter_ڳڻپ   sqlite3_api->bind_parameter_ڳڻپ
#وصف_ڪريو sqlite3_bind_parameter_اشاريو   sqlite3_api->bind_parameter_اشاريو
#وصف_ڪريو sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#وصف_ڪريو sqlite3_bind_متن              sqlite3_api->bind_متن
#وصف_ڪريو sqlite3_bind_text16            sqlite3_api->bind_text16
#وصف_ڪريو sqlite3_bind_قدر             sqlite3_api->bind_قدر
#وصف_ڪريو sqlite3_busy_handler           sqlite3_api->busy_handler
#وصف_ڪريو sqlite3_busy_timeout           sqlite3_api->busy_timeout
#وصف_ڪريو sqlite3_changes                sqlite3_api->changes
#وصف_ڪريو sqlite3_close                  sqlite3_api->close
#وصف_ڪريو sqlite3_collation_needed       sqlite3_api->collation_needed
#وصف_ڪريو sqlite3_collation_needed16     sqlite3_api->collation_needed16
#وصف_ڪريو sqlite3_ڪالم_blob            sqlite3_api->ڪالم_blob
#وصف_ڪريو sqlite3_ڪالم_بائيٽون           sqlite3_api->ڪالم_بائيٽون
#وصف_ڪريو sqlite3_ڪالم_bytes16         sqlite3_api->ڪالم_bytes16
#وصف_ڪريو sqlite3_ڪالم_ڳڻپ           sqlite3_api->ڪالم_ڳڻپ
#وصف_ڪريو sqlite3_ڪالم_database_name   sqlite3_api->ڪالم_database_name
#وصف_ڪريو sqlite3_ڪالم_database_name16 sqlite3_api->ڪالم_database_name16
#وصف_ڪريو sqlite3_ڪالم_decltype        sqlite3_api->ڪالم_decltype
#وصف_ڪريو sqlite3_ڪالم_decltype16      sqlite3_api->ڪالم_decltype16
#وصف_ڪريو sqlite3_ڪالم_double          sqlite3_api->ڪالم_double
#وصف_ڪريو sqlite3_ڪالم_int             sqlite3_api->ڪالم_int
#وصف_ڪريو sqlite3_ڪالم_int64           sqlite3_api->ڪالم_int64
#وصف_ڪريو sqlite3_ڪالم_name            sqlite3_api->ڪالم_name
#وصف_ڪريو sqlite3_ڪالم_name16          sqlite3_api->ڪالم_name16
#وصف_ڪريو sqlite3_ڪالم_origin_name     sqlite3_api->ڪالم_origin_name
#وصف_ڪريو sqlite3_ڪالم_origin_name16   sqlite3_api->ڪالم_origin_name16
#وصف_ڪريو sqlite3_ڪالم_جدول_name      sqlite3_api->ڪالم_جدول_name
#وصف_ڪريو sqlite3_ڪالم_جدول_name16    sqlite3_api->ڪالم_جدول_name16
#وصف_ڪريو sqlite3_ڪالم_متن            sqlite3_api->ڪالم_متن
#وصف_ڪريو sqlite3_ڪالم_text16          sqlite3_api->ڪالم_text16
#وصف_ڪريو sqlite3_ڪالم_type            sqlite3_api->ڪالم_type
#وصف_ڪريو sqlite3_ڪالم_قدر           sqlite3_api->ڪالم_قدر
#وصف_ڪريو sqlite3_commit_hook            sqlite3_api->commit_hook
#وصف_ڪريو sqlite3_complete               sqlite3_api->complete
#وصف_ڪريو sqlite3_complete16             sqlite3_api->complete16
#وصف_ڪريو sqlite3_create_collation       sqlite3_api->create_collation
#وصف_ڪريو sqlite3_create_collation16     sqlite3_api->create_collation16
#وصف_ڪريو sqlite3_create_function        sqlite3_api->create_function
#وصف_ڪريو sqlite3_create_function16      sqlite3_api->create_function16
#وصف_ڪريو sqlite3_create_module          sqlite3_api->create_module
#وصف_ڪريو sqlite3_create_module_v2       sqlite3_api->create_module_v2
#وصف_ڪريو sqlite3_معلومات_ڳڻپ             sqlite3_api->معلومات_ڳڻپ
#وصف_ڪريو sqlite3_db_handle              sqlite3_api->db_handle
#وصف_ڪريو sqlite3_declare_vtab           sqlite3_api->declare_vtab
#وصف_ڪريو sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#وصف_ڪريو sqlite3_errcode                sqlite3_api->errcode
#وصف_ڪريو sqlite3_errmsg                 sqlite3_api->errmsg
#وصف_ڪريو sqlite3_errmsg16               sqlite3_api->errmsg16
#وصف_ڪريو sqlite3_exec                   sqlite3_api->exec
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DEPRECATED
#وصف_ڪريو sqlite3_expired                sqlite3_api->expired
#جيڪڏهن_پورو
#وصف_ڪريو sqlite3_finalize               sqlite3_api->finalize
#وصف_ڪريو sqlite3_free                   sqlite3_api->حرر
#وصف_ڪريو sqlite3_free_جدول             sqlite3_api->free_جدول
#وصف_ڪريو sqlite3_get_autocommit         sqlite3_api->get_autocommit
#وصف_ڪريو sqlite3_get_auxdata            sqlite3_api->get_auxdata
#وصف_ڪريو sqlite3_get_جدول              sqlite3_api->get_جدول
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DEPRECATED
#وصف_ڪريو sqlite3_global_recover         sqlite3_api->global_recover
#جيڪڏهن_پورو
#وصف_ڪريو sqlite3_مداخلت              sqlite3_api->interruptx
#وصف_ڪريو sqlite3_آخري_وجهو_rowid      sqlite3_api->آخري_وجهو_rowid
#وصف_ڪريو sqlite3_libversion             sqlite3_api->libversion
#وصف_ڪريو sqlite3_libversion_عدد      sqlite3_api->libversion_عدد
#وصف_ڪريو sqlite3_malloc                 sqlite3_api->يادگيري_ڏيو
#وصف_ڪريو sqlite3_mprintf                sqlite3_api->mprintf
#وصف_ڪريو sqlite3_open                   sqlite3_api->open
#وصف_ڪريو sqlite3_open16                 sqlite3_api->open16
#وصف_ڪريو sqlite3_prepare                sqlite3_api->prepare
#وصف_ڪريو sqlite3_prepare16              sqlite3_api->prepare16
#وصف_ڪريو sqlite3_prepare_v2             sqlite3_api->prepare_v2
#وصف_ڪريو sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#وصف_ڪريو sqlite3_profile                sqlite3_api->profile
#وصف_ڪريو sqlite3_ترقي_handler       sqlite3_api->ترقي_handler
#وصف_ڪريو sqlite3_realloc                sqlite3_api->realloc
#وصف_ڪريو sqlite3_reset                  sqlite3_api->reset
#وصف_ڪريو sqlite3_نتيجو_blob            sqlite3_api->نتيجو_blob
#وصف_ڪريو sqlite3_نتيجو_double          sqlite3_api->نتيجو_double
#وصف_ڪريو sqlite3_نتيجو_error           sqlite3_api->نتيجو_error
#وصف_ڪريو sqlite3_نتيجو_error16         sqlite3_api->نتيجو_error16
#وصف_ڪريو sqlite3_نتيجو_int             sqlite3_api->نتيجو_int
#وصف_ڪريو sqlite3_نتيجو_int64           sqlite3_api->نتيجو_int64
#وصف_ڪريو sqlite3_نتيجو_null            sqlite3_api->نتيجو_null
#وصف_ڪريو sqlite3_نتيجو_متن            sqlite3_api->نتيجو_متن
#وصف_ڪريو sqlite3_نتيجو_text16          sqlite3_api->نتيجو_text16
#وصف_ڪريو sqlite3_نتيجو_text16be        sqlite3_api->نتيجو_text16be
#وصف_ڪريو sqlite3_نتيجو_text16le        sqlite3_api->نتيجو_text16le
#وصف_ڪريو sqlite3_نتيجو_قدر           sqlite3_api->نتيجو_قدر
#وصف_ڪريو sqlite3_rollback_hook          sqlite3_api->rollback_hook
#وصف_ڪريو sqlite3_سيٽ_authorizer         sqlite3_api->سيٽ_authorizer
#وصف_ڪريو sqlite3_سيٽ_auxdata            sqlite3_api->سيٽ_auxdata
#وصف_ڪريو sqlite3_snprintf               sqlite3_api->xsnprintf
#وصف_ڪريو sqlite3_قدم                   sqlite3_api->قدم
#وصف_ڪريو sqlite3_جدول_ڪالم_metadata  sqlite3_api->جدول_ڪالم_metadata
#وصف_ڪريو sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#وصف_ڪريو sqlite3_ڪل_changes          sqlite3_api->ڪل_changes
#وصف_ڪريو sqlite3_trace                  sqlite3_api->trace
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DEPRECATED
#وصف_ڪريو sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#جيڪڏهن_پورو
#وصف_ڪريو sqlite3_update_hook            sqlite3_api->update_hook
#وصف_ڪريو sqlite3_user_معلومات              sqlite3_api->user_معلومات
#وصف_ڪريو sqlite3_قدر_blob             sqlite3_api->قدر_blob
#وصف_ڪريو sqlite3_قدر_بائيٽون            sqlite3_api->قدر_بائيٽون
#وصف_ڪريو sqlite3_قدر_bytes16          sqlite3_api->قدر_bytes16
#وصف_ڪريو sqlite3_قدر_double           sqlite3_api->قدر_double
#وصف_ڪريو sqlite3_قدر_int              sqlite3_api->قدر_int
#وصف_ڪريو sqlite3_قدر_int64            sqlite3_api->قدر_int64
#وصف_ڪريو sqlite3_قدر_numeric_type     sqlite3_api->قدر_numeric_type
#وصف_ڪريو sqlite3_قدر_متن             sqlite3_api->قدر_متن
#وصف_ڪريو sqlite3_قدر_text16           sqlite3_api->قدر_text16
#وصف_ڪريو sqlite3_قدر_text16be         sqlite3_api->قدر_text16be
#وصف_ڪريو sqlite3_قدر_text16le         sqlite3_api->قدر_text16le
#وصف_ڪريو sqlite3_قدر_type             sqlite3_api->قدر_type
#وصف_ڪريو sqlite3_vmprintf               sqlite3_api->vmprintf
#وصف_ڪريو sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#وصف_ڪريو sqlite3_overload_function      sqlite3_api->overload_function
#وصف_ڪريو sqlite3_prepare_v2             sqlite3_api->prepare_v2
#وصف_ڪريو sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#وصف_ڪريو sqlite3_clear_bindings         sqlite3_api->clear_bindings
#وصف_ڪريو sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#وصف_ڪريو sqlite3_blob_بائيٽون             sqlite3_api->blob_بائيٽون
#وصف_ڪريو sqlite3_blob_close             sqlite3_api->blob_close
#وصف_ڪريو sqlite3_blob_open              sqlite3_api->blob_open
#وصف_ڪريو sqlite3_blob_read              sqlite3_api->blob_read
#وصف_ڪريو sqlite3_blob_write             sqlite3_api->blob_write
#وصف_ڪريو sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#وصف_ڪريو sqlite3_file_control           sqlite3_api->file_control
#وصف_ڪريو sqlite3_يادگيري_highwater       sqlite3_api->يادگيري_highwater
#وصف_ڪريو sqlite3_يادگيري_استعمال_ٿيل            sqlite3_api->يادگيري_استعمال_ٿيل
#وصف_ڪريو sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#وصف_ڪريو sqlite3_mutex_enter            sqlite3_api->mutex_enter
#وصف_ڪريو sqlite3_mutex_free             sqlite3_api->mutex_free
#وصف_ڪريو sqlite3_mutex_leave            sqlite3_api->mutex_leave
#وصف_ڪريو sqlite3_mutex_try              sqlite3_api->mutex_try
#وصف_ڪريو sqlite3_open_v2                sqlite3_api->open_v2
#وصف_ڪريو sqlite3_release_يادگيري         sqlite3_api->release_يادگيري
#وصف_ڪريو sqlite3_نتيجو_error_nomem     sqlite3_api->نتيجو_error_nomem
#وصف_ڪريو sqlite3_نتيجو_error_toobig    sqlite3_api->نتيجو_error_toobig
#وصف_ڪريو sqlite3_sleep                  sqlite3_api->sleep
#وصف_ڪريو sqlite3_soft_ڍير_حد_بندي        sqlite3_api->soft_ڍير_حد_بندي
#وصف_ڪريو sqlite3_vfs_ڳوليو               sqlite3_api->vfs_ڳوليو
#وصف_ڪريو sqlite3_vfs_رجسٽر           sqlite3_api->vfs_رجسٽر
#وصف_ڪريو sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#وصف_ڪريو sqlite3_threadsafe             sqlite3_api->xthreadsafe
#وصف_ڪريو sqlite3_نتيجو_zeroblob        sqlite3_api->نتيجو_zeroblob
#وصف_ڪريو sqlite3_نتيجو_error_ڪوڊ      sqlite3_api->نتيجو_error_ڪوڊ
#وصف_ڪريو sqlite3_test_control           sqlite3_api->test_control
#وصف_ڪريو sqlite3_randomness             sqlite3_api->randomness
#وصف_ڪريو sqlite3_context_db_handle      sqlite3_api->context_db_handle
#وصف_ڪريو sqlite3_extended_نتيجو_codes  sqlite3_api->extended_نتيجو_codes
#وصف_ڪريو sqlite3_حد_بندي                  sqlite3_api->حد_بندي
#وصف_ڪريو sqlite3_ايندڙ_stmt              sqlite3_api->ايندڙ_stmt
#وصف_ڪريو sqlite3_sql                    sqlite3_api->sql
#وصف_ڪريو sqlite3_حالت                 sqlite3_api->حالت
#وصف_ڪريو sqlite3_backup_پڄاڻي          sqlite3_api->backup_پڄاڻي
#وصف_ڪريو sqlite3_backup_init            sqlite3_api->backup_init
#وصف_ڪريو sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#وصف_ڪريو sqlite3_backup_بچيل       sqlite3_api->backup_بچيل
#وصف_ڪريو sqlite3_backup_قدم            sqlite3_api->backup_قدم
#وصف_ڪريو sqlite3_compileoption_get      sqlite3_api->compileoption_get
#وصف_ڪريو sqlite3_compileoption_استعمال_ٿيل     sqlite3_api->compileoption_استعمال_ٿيل
#وصف_ڪريو sqlite3_create_function_v2     sqlite3_api->create_function_v2
#وصف_ڪريو sqlite3_db_config              sqlite3_api->db_config
#وصف_ڪريو sqlite3_db_mutex               sqlite3_api->db_mutex
#وصف_ڪريو sqlite3_db_حالت              sqlite3_api->db_حالت
#وصف_ڪريو sqlite3_extended_errcode       sqlite3_api->extended_errcode
#وصف_ڪريو sqlite3_log                    sqlite3_api->لاگرٿم
#وصف_ڪريو sqlite3_soft_ڍير_limit64      sqlite3_api->soft_ڍير_limit64
#وصف_ڪريو sqlite3_sourceid               sqlite3_api->sourceid
#وصف_ڪريو sqlite3_stmt_حالت            sqlite3_api->stmt_حالت
#وصف_ڪريو sqlite3_strnicmp               sqlite3_api->strnicmp
#وصف_ڪريو sqlite3_unlock_notify          sqlite3_api->unlock_notify
#وصف_ڪريو sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#وصف_ڪريو sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#وصف_ڪريو sqlite3_wal_hook               sqlite3_api->wal_hook
#وصف_ڪريو sqlite3_blob_reopen            sqlite3_api->blob_reopen
#وصف_ڪريو sqlite3_vtab_config            sqlite3_api->vtab_config
#وصف_ڪريو sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#وصف_ڪريو sqlite3_close_v2               sqlite3_api->close_v2
#وصف_ڪريو sqlite3_db_filename            sqlite3_api->db_filename
#وصف_ڪريو sqlite3_db_readonly            sqlite3_api->db_readonly
#وصف_ڪريو sqlite3_db_release_يادگيري      sqlite3_api->db_release_يادگيري
#وصف_ڪريو sqlite3_errstr                 sqlite3_api->errstr
#وصف_ڪريو sqlite3_stmt_busy              sqlite3_api->stmt_busy
#وصف_ڪريو sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#وصف_ڪريو sqlite3_stricmp                sqlite3_api->stricmp
#وصف_ڪريو sqlite3_uri_boolean            sqlite3_api->uri_boolean
#وصف_ڪريو sqlite3_uri_int64              sqlite3_api->uri_int64
#وصف_ڪريو sqlite3_uri_parameter          sqlite3_api->uri_parameter
#وصف_ڪريو sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#وصف_ڪريو sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#وصف_ڪريو sqlite3_auto_extension         sqlite3_api->auto_extension
#وصف_ڪريو sqlite3_bind_blob64            sqlite3_api->bind_blob64
#وصف_ڪريو sqlite3_bind_text64            sqlite3_api->bind_text64
#وصف_ڪريو sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#وصف_ڪريو sqlite3_ڀريو_extension         sqlite3_api->ڀريو_extension
#وصف_ڪريو sqlite3_malloc64               sqlite3_api->malloc64
#وصف_ڪريو sqlite3_msize                  sqlite3_api->msize
#وصف_ڪريو sqlite3_realloc64              sqlite3_api->realloc64
#وصف_ڪريو sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#وصف_ڪريو sqlite3_نتيجو_blob64          sqlite3_api->نتيجو_blob64
#وصف_ڪريو sqlite3_نتيجو_text64          sqlite3_api->نتيجو_text64
#وصف_ڪريو sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#وصف_ڪريو sqlite3_قدر_dup              sqlite3_api->قدر_dup
#وصف_ڪريو sqlite3_قدر_free             sqlite3_api->قدر_free
#وصف_ڪريو sqlite3_نتيجو_zeroblob64      sqlite3_api->نتيجو_zeroblob64
#وصف_ڪريو sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#وصف_ڪريو sqlite3_قدر_subtype          sqlite3_api->قدر_subtype
#وصف_ڪريو sqlite3_نتيجو_subtype         sqlite3_api->نتيجو_subtype
/* Version 3.10.0 and later */
#وصف_ڪريو sqlite3_status64               sqlite3_api->status64
#وصف_ڪريو sqlite3_strlike                sqlite3_api->strlike
#وصف_ڪريو sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#وصف_ڪريو sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#وصف_ڪريو sqlite3_trace_v2               sqlite3_api->trace_v2
#وصف_ڪريو sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#وصف_ڪريو sqlite3_سيٽ_آخري_وجهو_rowid  sqlite3_api->سيٽ_آخري_وجهو_rowid
/* Version 3.20.0 and later */
#وصف_ڪريو sqlite3_prepare_v3             sqlite3_api->prepare_v3
#وصف_ڪريو sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#وصف_ڪريو sqlite3_bind_اشارو           sqlite3_api->bind_اشارو
#وصف_ڪريو sqlite3_نتيجو_اشارو         sqlite3_api->نتيجو_اشارو
#وصف_ڪريو sqlite3_قدر_اشارو          sqlite3_api->قدر_اشارو
/* Version 3.22.0 and later */
#وصف_ڪريو sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#وصف_ڪريو sqlite3_قدر_nochange         sqlite3_api->قدر_nochange
#وصف_ڪريو sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#وصف_ڪريو sqlite3_keyword_ڳڻپ          sqlite3_api->keyword_ڳڻپ
#وصف_ڪريو sqlite3_keyword_name           sqlite3_api->keyword_name
#وصف_ڪريو sqlite3_keyword_check          sqlite3_api->keyword_check
#وصف_ڪريو sqlite3_str_new                sqlite3_api->str_new
#وصف_ڪريو sqlite3_str_پڄاڻي             sqlite3_api->str_پڄاڻي
#وصف_ڪريو sqlite3_str_appendf            sqlite3_api->str_appendf
#وصف_ڪريو sqlite3_str_vappendf           sqlite3_api->str_vappendf
#وصف_ڪريو sqlite3_str_append             sqlite3_api->str_append
#وصف_ڪريو sqlite3_str_appendall          sqlite3_api->str_appendall
#وصف_ڪريو sqlite3_str_appendchar         sqlite3_api->str_appendchar
#وصف_ڪريو sqlite3_str_reset              sqlite3_api->str_reset
#وصف_ڪريو sqlite3_str_errcode            sqlite3_api->str_errcode
#وصف_ڪريو sqlite3_str_ڊيگهائي             sqlite3_api->str_ڊيگهائي
#وصف_ڪريو sqlite3_str_قدر              sqlite3_api->str_قدر
/* Version 3.25.0 and later */
#وصف_ڪريو sqlite3_create_دري_function sqlite3_api->create_دري_function
/* Version 3.26.0 and later */
#وصف_ڪريو sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#وصف_ڪريو sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#وصف_ڪريو sqlite3_قدر_frombind         sqlite3_api->قدر_frombind
/* Version 3.30.0 and later */
#وصف_ڪريو sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#وصف_ڪريو sqlite3_hard_ڍير_limit64      sqlite3_api->hard_ڍير_limit64
#وصف_ڪريو sqlite3_uri_ڪنجي                sqlite3_api->uri_ڪنجي
#وصف_ڪريو sqlite3_filename_database      sqlite3_api->filename_database
#وصف_ڪريو sqlite3_filename_journal       sqlite3_api->filename_journal
#وصف_ڪريو sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#وصف_ڪريو sqlite3_create_filename        sqlite3_api->create_filename
#وصف_ڪريو sqlite3_free_filename          sqlite3_api->free_filename
#وصف_ڪريو sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#وصف_ڪريو sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#وصف_ڪريو sqlite3_changes64              sqlite3_api->changes64
#وصف_ڪريو sqlite3_ڪل_changes64        sqlite3_api->ڪل_changes64
/* Version 3.37.0 and later */
#وصف_ڪريو sqlite3_autovacuum_صفحا       sqlite3_api->autovacuum_صفحا
/* Version 3.38.0 and later */
#وصف_ڪريو sqlite3_error_وٿي           sqlite3_api->error_وٿي
#وصف_ڪريو sqlite3_vtab_rhs_قدر         sqlite3_api->vtab_rhs_قدر
#وصف_ڪريو sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#وصف_ڪريو sqlite3_vtab_اندريون                sqlite3_api->vtab_اندريون
#وصف_ڪريو sqlite3_vtab_اندريون_پهريون          sqlite3_api->vtab_اندريون_پهريون
#وصف_ڪريو sqlite3_vtab_اندريون_ايندڙ           sqlite3_api->vtab_اندريون_ايندڙ
/* Version 3.39.0 and later */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DESERIALIZE
#وصف_ڪريو sqlite3_deserialize            sqlite3_api->deserialize
#وصف_ڪريو sqlite3_serialize              sqlite3_api->serialize
#جيڪڏهن_پورو
#وصف_ڪريو sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#وصف_ڪريو sqlite3_قدر_encoding         sqlite3_api->قدر_encoding
/* Version 3.41.0 and later */
#وصف_ڪريو sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#وصف_ڪريو sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#وصف_ڪريو sqlite3_get_clientdata         sqlite3_api->get_clientdata
#وصف_ڪريو sqlite3_سيٽ_clientdata         sqlite3_api->سيٽ_clientdata
/* Version 3.50.0 and later */
#وصف_ڪريو sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#وصف_ڪريو sqlite3_سيٽ_errmsg             sqlite3_api->سيٽ_errmsg
#وصف_ڪريو sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#وصف_ڪريو sqlite3_str_truncate           sqlite3_api->str_truncate
#وصف_ڪريو sqlite3_str_free               sqlite3_api->str_free
#وصف_ڪريو sqlite3_carray_bind            sqlite3_api->carray_bind
#وصف_ڪريو sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#وصف_ڪريو sqlite3_incomplete             sqlite3_api->incomplete
#وصف_ڪريو sqlite3_نتيجو_str             sqlite3_api->نتيجو_str
#جيڪڏهن_پورو /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#إذا !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_ڀريو_EXTENSION)
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
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* SQLITE3EXT_H */
