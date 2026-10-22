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
#اگر_تعریف_نشده SQLITE3EXT_H
#تعریف_کن SQLITE3EXT_H
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
  فراغ * (*aggregate_context)(sqlite3_context*,صحيح n_بایتها);
  صحيح  (*aggregate_شمار)(sqlite3_context*);
  صحيح  (*bind_blob)(sqlite3_stmt*,صحيح,ثابت فراغ*,صحيح n,فراغ(*)(فراغ*));
  صحيح  (*bind_double)(sqlite3_stmt*,صحيح,مزدوج);
  صحيح  (*bind_int)(sqlite3_stmt*,صحيح,صحيح);
  صحيح  (*bind_int64)(sqlite3_stmt*,صحيح,sqlite_int64);
  صحيح  (*bind_null)(sqlite3_stmt*,صحيح);
  صحيح  (*bind_parameter_شمار)(sqlite3_stmt*);
  صحيح  (*bind_parameter_شاخص)(sqlite3_stmt*,ثابت حرف*zName);
  ثابت حرف * (*bind_parameter_name)(sqlite3_stmt*,صحيح);
  صحيح  (*bind_متن)(sqlite3_stmt*,صحيح,ثابت حرف*,صحيح n,فراغ(*)(فراغ*));
  صحيح  (*bind_text16)(sqlite3_stmt*,صحيح,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  صحيح  (*bind_مقدار)(sqlite3_stmt*,صحيح,ثابت sqlite3_مقدار*);
  صحيح  (*busy_handler)(sqlite3*,صحيح(*)(فراغ*,صحيح),فراغ*);
  صحيح  (*busy_timeout)(sqlite3*,صحيح ms);
  صحيح  (*changes)(sqlite3*);
  صحيح  (*close)(sqlite3*);
  صحيح  (*collation_needed)(sqlite3*,فراغ*,فراغ(*)(فراغ*,sqlite3*,
                           صحيح e_متن_Rep,ثابت حرف*));
  صحيح  (*collation_needed16)(sqlite3*,فراغ*,فراغ(*)(فراغ*,sqlite3*,
                             صحيح e_متن_Rep,ثابت فراغ*));
  ثابت فراغ * (*ستونی_blob)(sqlite3_stmt*,صحيح i_ستون);
  صحيح  (*ستونی_بایتها)(sqlite3_stmt*,صحيح i_ستون);
  صحيح  (*ستونی_bytes16)(sqlite3_stmt*,صحيح i_ستون);
  صحيح  (*ستونی_شمار)(sqlite3_stmt*pStmt);
  ثابت حرف * (*ستونی_database_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*ستونی_database_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*ستونی_decltype)(sqlite3_stmt*,صحيح i);
  ثابت فراغ * (*ستونی_decltype16)(sqlite3_stmt*,صحيح);
  مزدوج  (*ستونی_double)(sqlite3_stmt*,صحيح i_ستون);
  صحيح  (*ستونی_int)(sqlite3_stmt*,صحيح i_ستون);
  sqlite_int64  (*ستونی_int64)(sqlite3_stmt*,صحيح i_ستون);
  ثابت حرف * (*ستونی_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*ستونی_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*ستونی_origin_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*ستونی_origin_name16)(sqlite3_stmt*,صحيح);
  ثابت حرف * (*ستونی_جدول_name)(sqlite3_stmt*,صحيح);
  ثابت فراغ * (*ستونی_جدول_name16)(sqlite3_stmt*,صحيح);
  ثابت بیعلامت حرف * (*ستونی_متن)(sqlite3_stmt*,صحيح i_ستون);
  ثابت فراغ * (*ستونی_text16)(sqlite3_stmt*,صحيح i_ستون);
  صحيح  (*ستونی_type)(sqlite3_stmt*,صحيح i_ستون);
  sqlite3_مقدار* (*ستونی_مقدار)(sqlite3_stmt*,صحيح i_ستون);
  فراغ * (*commit_hook)(sqlite3*,صحيح(*)(فراغ*),فراغ*);
  صحيح  (*complete)(ثابت حرف*sql);
  صحيح  (*complete16)(ثابت فراغ*sql);
  صحيح  (*create_collation)(sqlite3*,ثابت حرف*,صحيح,فراغ*,
                           صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*));
  صحيح  (*create_collation16)(sqlite3*,ثابت فراغ*,صحيح,فراغ*,
                             صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*));
  صحيح  (*create_function)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                          فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_مقدار**),
                          فراغ (*x_گام)(sqlite3_context*,صحيح,sqlite3_مقدار**),
                          فراغ (*xFinal)(sqlite3_context*));
  صحيح  (*create_function16)(sqlite3*,ثابت فراغ*,صحيح,صحيح,فراغ*,
                            فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_مقدار**),
                            فراغ (*x_گام)(sqlite3_context*,صحيح,sqlite3_مقدار**),
                            فراغ (*xFinal)(sqlite3_context*));
  صحيح (*create_module)(sqlite3*,ثابت حرف*,ثابت sqlite3_module*,فراغ*);
  صحيح  (*داده_شمار)(sqlite3_stmt*pStmt);
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
  فراغ  (*free_جدول)(حرف**نتیجه);
  صحيح  (*get_autocommit)(sqlite3*);
  فراغ * (*get_auxdata)(sqlite3_context*,صحيح);
  صحيح  (*get_جدول)(sqlite3*,ثابت حرف*,حرف***,صحيح*,صحيح*,حرف**);
  صحيح  (*global_recover)(فراغ);
  فراغ  (*interruptx)(sqlite3*);
  sqlite_int64  (*آخرین_درج_rowid)(sqlite3*);
  ثابت حرف * (*libversion)(فراغ);
  صحيح  (*libversion_عدد)(فراغ);
  فراغ *(*حافظه_بده)(صحيح);
  حرف *(SQLITE_CDECL*mprintf)(ثابت حرف*,...);
  صحيح  (*open)(ثابت حرف*,sqlite3**);
  صحيح  (*open16)(ثابت فراغ*,sqlite3**);
  صحيح  (*prepare)(sqlite3*,ثابت حرف*,صحيح,sqlite3_stmt**,ثابت حرف**);
  صحيح  (*prepare16)(sqlite3*,ثابت فراغ*,صحيح,sqlite3_stmt**,ثابت فراغ**);
  فراغ * (*profile)(sqlite3*,فراغ(*)(فراغ*,ثابت حرف*,sqlite_uint64),فراغ*);
  فراغ  (*پیشرفت_handler)(sqlite3*,صحيح,صحيح(*)(فراغ*),فراغ*);
  فراغ *(*realloc)(فراغ*,صحيح);
  صحيح  (*reset)(sqlite3_stmt*pStmt);
  فراغ  (*نتیجه_blob)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجه_double)(sqlite3_context*,مزدوج);
  فراغ  (*نتیجه_error)(sqlite3_context*,ثابت حرف*,صحيح);
  فراغ  (*نتیجه_error16)(sqlite3_context*,ثابت فراغ*,صحيح);
  فراغ  (*نتیجه_int)(sqlite3_context*,صحيح);
  فراغ  (*نتیجه_int64)(sqlite3_context*,sqlite_int64);
  فراغ  (*نتیجه_null)(sqlite3_context*);
  فراغ  (*نتیجه_متن)(sqlite3_context*,ثابت حرف*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجه_text16)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجه_text16be)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجه_text16le)(sqlite3_context*,ثابت فراغ*,صحيح,فراغ(*)(فراغ*));
  فراغ  (*نتیجه_مقدار)(sqlite3_context*,sqlite3_مقدار*);
  فراغ *(*rollback_hook)(sqlite3*,فراغ(*)(فراغ*),فراغ*);
  صحيح  (*مجموعه_authorizer)(sqlite3*,صحيح(*)(فراغ*,صحيح,ثابت حرف*,ثابت حرف*,
                         ثابت حرف*,ثابت حرف*),فراغ*);
  فراغ  (*مجموعه_auxdata)(sqlite3_context*,صحيح,فراغ*,فراغ (*)(فراغ*));
  حرف *(SQLITE_CDECL*xsnprintf)(صحيح,حرف*,ثابت حرف*,...);
  صحيح  (*گام)(sqlite3_stmt*);
  صحيح  (*جدول_ستونی_metadata)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,
                                حرف ثابت**,حرف ثابت**,صحيح*,صحيح*,صحيح*);
  فراغ  (*thread_cleanup)(فراغ);
  صحيح  (*مجموع_changes)(sqlite3*);
  فراغ * (*trace)(sqlite3*,فراغ(*xTrace)(فراغ*,ثابت حرف*),فراغ*);
  صحيح  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  فراغ * (*update_hook)(sqlite3*,فراغ(*)(فراغ*,صحيح ,حرف ثابت*,حرف ثابت*,
                                         sqlite_int64),فراغ*);
  فراغ * (*user_داده)(sqlite3_context*);
  ثابت فراغ * (*مقدار_blob)(sqlite3_مقدار*);
  صحيح  (*مقدار_بایتها)(sqlite3_مقدار*);
  صحيح  (*مقدار_bytes16)(sqlite3_مقدار*);
  مزدوج  (*مقدار_double)(sqlite3_مقدار*);
  صحيح  (*مقدار_int)(sqlite3_مقدار*);
  sqlite_int64  (*مقدار_int64)(sqlite3_مقدار*);
  صحيح  (*مقدار_numeric_type)(sqlite3_مقدار*);
  ثابت بیعلامت حرف * (*مقدار_متن)(sqlite3_مقدار*);
  ثابت فراغ * (*مقدار_text16)(sqlite3_مقدار*);
  ثابت فراغ * (*مقدار_text16be)(sqlite3_مقدار*);
  ثابت فراغ * (*مقدار_text16le)(sqlite3_مقدار*);
  صحيح  (*مقدار_type)(sqlite3_مقدار*);
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
  صحيح (*blob_بایتها)(sqlite3_blob*);
  صحيح (*blob_close)(sqlite3_blob*);
  صحيح (*blob_open)(sqlite3*,ثابت حرف*,ثابت حرف*,ثابت حرف*,sqlite3_int64,
                   صحيح,sqlite3_blob**);
  صحيح (*blob_read)(sqlite3_blob*,فراغ*,صحيح,صحيح);
  صحيح (*blob_write)(sqlite3_blob*,ثابت فراغ*,صحيح,صحيح);
  صحيح (*create_collation_v2)(sqlite3*,ثابت حرف*,صحيح,فراغ*,
                             صحيح(*)(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*),
                             فراغ(*)(فراغ*));
  صحيح (*file_control)(sqlite3*,ثابت حرف*,صحيح,فراغ*);
  sqlite3_int64 (*حافظه_highwater)(صحيح);
  sqlite3_int64 (*حافظه_استفاده_شده)(فراغ);
  sqlite3_mutex *(*mutex_alloc)(صحيح);
  فراغ (*mutex_enter)(sqlite3_mutex*);
  فراغ (*mutex_free)(sqlite3_mutex*);
  فراغ (*mutex_leave)(sqlite3_mutex*);
  صحيح (*mutex_try)(sqlite3_mutex*);
  صحيح (*open_v2)(ثابت حرف*,sqlite3**,صحيح,ثابت حرف*);
  صحيح (*release_حافظه)(صحيح);
  فراغ (*نتیجه_error_nomem)(sqlite3_context*);
  فراغ (*نتیجه_error_toobig)(sqlite3_context*);
  صحيح (*sleep)(صحيح);
  فراغ (*soft_انباشته_حد)(صحيح);
  sqlite3_vfs *(*vfs_پیدا_کن)(ثابت حرف*);
  صحيح (*vfs_ثبات)(sqlite3_vfs*,صحيح);
  صحيح (*vfs_unregister)(sqlite3_vfs*);
  صحيح (*xthreadsafe)(فراغ);
  فراغ (*نتیجه_zeroblob)(sqlite3_context*,صحيح);
  فراغ (*نتیجه_error_کد)(sqlite3_context*,صحيح);
  صحيح (SQLITE_CDECL*test_control)(صحيح, ...);
  فراغ (*randomness)(صحيح,فراغ*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  صحيح (*extended_نتیجه_codes)(sqlite3*,صحيح);
  صحيح (*حد)(sqlite3*,صحيح,صحيح);
  sqlite3_stmt *(*بعدی_stmt)(sqlite3*,sqlite3_stmt*);
  ثابت حرف *(*sql)(sqlite3_stmt*);
  صحيح (*وضعیت)(صحيح,صحيح*,صحيح*,صحيح);
  صحيح (*backup_پایان)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ثابت حرف*,sqlite3*,ثابت حرف*);
  صحيح (*backup_pagecount)(sqlite3_backup*);
  صحيح (*backup_مانده)(sqlite3_backup*);
  صحيح (*backup_گام)(sqlite3_backup*,صحيح);
  ثابت حرف *(*compileoption_get)(صحيح);
  صحيح (*compileoption_استفاده_شده)(ثابت حرف*);
  صحيح (*create_function_v2)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                            فراغ (*xFunc)(sqlite3_context*,صحيح,sqlite3_مقدار**),
                            فراغ (*x_گام)(sqlite3_context*,صحيح,sqlite3_مقدار**),
                            فراغ (*xFinal)(sqlite3_context*),
                            فراغ(*xDestroy)(فراغ*));
  صحيح (SQLITE_CDECL*db_config)(sqlite3*,صحيح,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  صحيح (*db_وضعیت)(sqlite3*,صحيح,صحيح*,صحيح*,صحيح);
  صحيح (*extended_errcode)(sqlite3*);
  فراغ (SQLITE_CDECL*لگاریتم)(صحيح,ثابت حرف*,...);
  sqlite3_int64 (*soft_انباشته_limit64)(sqlite3_int64);
  ثابت حرف *(*sourceid)(فراغ);
  صحيح (*stmt_وضعیت)(sqlite3_stmt*,صحيح,صحيح);
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
  صحيح (*db_release_حافظه)(sqlite3*);
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
                      فراغ(*)(فراغ*),بیعلامت حرف);
  صحيح (*cancel_auto_extension)(فراغ(*)(فراغ));
  صحيح (*بارگیری_extension)(sqlite3*,ثابت حرف*,ثابت حرف*,حرف**);
  فراغ *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(فراغ*);
  فراغ *(*realloc64)(فراغ*,sqlite3_uint64);
  فراغ (*reset_auto_extension)(فراغ);
  فراغ (*نتیجه_blob64)(sqlite3_context*,ثابت فراغ*,sqlite3_uint64,
                        فراغ(*)(فراغ*));
  فراغ (*نتیجه_text64)(sqlite3_context*,ثابت حرف*,sqlite3_uint64,
                         فراغ(*)(فراغ*), بیعلامت حرف);
  صحيح (*strglob)(ثابت حرف*,ثابت حرف*);
  /* Version 3.8.11 and later */
  sqlite3_مقدار *(*مقدار_dup)(ثابت sqlite3_مقدار*);
  فراغ (*مقدار_free)(sqlite3_مقدار*);
  صحيح (*نتیجه_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  صحيح (*bind_zeroblob64)(sqlite3_stmt*, صحيح, sqlite3_uint64);
  /* Version 3.9.0 and later */
  بیعلامت صحيح (*مقدار_subtype)(sqlite3_مقدار*);
  فراغ (*نتیجه_subtype)(sqlite3_context*,بیعلامت صحيح);
  /* Version 3.10.0 and later */
  صحيح (*status64)(صحيح,sqlite3_int64*,sqlite3_int64*,صحيح);
  صحيح (*strlike)(ثابت حرف*,ثابت حرف*,بیعلامت صحيح);
  صحيح (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  صحيح (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  صحيح (*trace_v2)(sqlite3*,بیعلامت,صحيح(*)(بیعلامت,فراغ*,فراغ*,فراغ*),فراغ*);
  حرف *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  فراغ (*مجموعه_آخرین_درج_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  صحيح (*prepare_v3)(sqlite3*,ثابت حرف*,صحيح,بیعلامت صحيح,
                    sqlite3_stmt**,ثابت حرف**);
  صحيح (*prepare16_v3)(sqlite3*,ثابت فراغ*,صحيح,بیعلامت صحيح,
                      sqlite3_stmt**,ثابت فراغ**);
  صحيح (*bind_اشاره_گر)(sqlite3_stmt*,صحيح,فراغ*,ثابت حرف*,فراغ(*)(فراغ*));
  فراغ (*نتیجه_اشاره_گر)(sqlite3_context*,فراغ*,ثابت حرف*,فراغ(*)(فراغ*));
  فراغ *(*مقدار_اشاره_گر)(sqlite3_مقدار*,ثابت حرف*);
  صحيح (*vtab_nochange)(sqlite3_context*);
  صحيح (*مقدار_nochange)(sqlite3_مقدار*);
  ثابت حرف *(*vtab_collation)(sqlite3_شاخص_info*,صحيح);
  /* Version 3.24.0 and later */
  صحيح (*keyword_شمار)(فراغ);
  صحيح (*keyword_name)(صحيح,ثابت حرف**,صحيح*);
  صحيح (*keyword_check)(ثابت حرف*,صحيح);
  sqlite3_str *(*str_new)(sqlite3*);
  حرف *(*str_پایان)(sqlite3_str*);
  فراغ (SQLITE_CDECL*str_appendf)(sqlite3_str*, ثابت حرف *zFormat, ...);
  فراغ (*str_vappendf)(sqlite3_str*, ثابت حرف *zFormat, va_list);
  فراغ (*str_append)(sqlite3_str*, ثابت حرف *z_درون, صحيح N);
  فراغ (*str_appendall)(sqlite3_str*, ثابت حرف *z_درون);
  فراغ (*str_appendchar)(sqlite3_str*, صحيح N, حرف C);
  فراغ (*str_reset)(sqlite3_str*);
  صحيح (*str_errcode)(sqlite3_str*);
  صحيح (*str_درازی)(sqlite3_str*);
  حرف *(*str_مقدار)(sqlite3_str*);
  /* Version 3.25.0 and later */
  صحيح (*create_پنجره_function)(sqlite3*,ثابت حرف*,صحيح,صحيح,فراغ*,
                            فراغ (*x_گام)(sqlite3_context*,صحيح,sqlite3_مقدار**),
                            فراغ (*xFinal)(sqlite3_context*),
                            فراغ (*x_مقدار)(sqlite3_context*),
                            فراغ (*xInv)(sqlite3_context*,صحيح,sqlite3_مقدار**),
                            فراغ(*xDestroy)(فراغ*));
  /* Version 3.26.0 and later */
  ثابت حرف *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  صحيح (*stmt_isexplain)(sqlite3_stmt*);
  صحيح (*مقدار_frombind)(sqlite3_مقدار*);
  /* Version 3.30.0 and later */
  صحيح (*drop_modules)(sqlite3*,ثابت حرف**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_انباشته_limit64)(sqlite3_int64);
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
  sqlite3_int64 (*مجموع_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  صحيح (*autovacuum_صفحات)(sqlite3*,
     بیعلامت صحيح(*)(فراغ*,ثابت حرف*,بیعلامت صحيح,بیعلامت صحيح,بیعلامت صحيح),
     فراغ*, فراغ(*)(فراغ*));
  /* Version 3.38.0 and later */
  صحيح (*error_جابه_جایی)(sqlite3*);
  صحيح (*vtab_rhs_مقدار)(sqlite3_شاخص_info*,صحيح,sqlite3_مقدار**);
  صحيح (*vtab_distinct)(sqlite3_شاخص_info*);
  صحيح (*vtab_درون)(sqlite3_شاخص_info*,صحيح,صحيح);
  صحيح (*vtab_درون_اولی)(sqlite3_مقدار*,sqlite3_مقدار**);
  صحيح (*vtab_درون_بعدی)(sqlite3_مقدار*,sqlite3_مقدار**);
  /* Version 3.39.0 and later */
  صحيح (*deserialize)(sqlite3*,ثابت حرف*,بیعلامت حرف*,
                     sqlite3_int64,sqlite3_int64,بیعلامت);
  بیعلامت حرف *(*serialize)(sqlite3*,ثابت حرف *,sqlite3_int64*,
                              بیعلامت صحيح);
  ثابت حرف *(*db_name)(sqlite3*,صحيح);
  /* Version 3.40.0 and later */
  صحيح (*مقدار_encoding)(sqlite3_مقدار*);
  /* Version 3.41.0 and later */
  صحيح (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  صحيح (*stmt_explain)(sqlite3_stmt*,صحيح);
  /* Version 3.44.0 and later */
  فراغ *(*get_clientdata)(sqlite3*,ثابت حرف*);
  صحيح (*مجموعه_clientdata)(sqlite3*, ثابت حرف*, فراغ*, فراغ(*)(فراغ*));
  /* Version 3.50.0 and later */
  صحيح (*setlk_timeout)(sqlite3*,صحيح,صحيح);
  /* Version 3.51.0 and later */
  صحيح (*مجموعه_errmsg)(sqlite3*,صحيح,ثابت حرف*);
  صحيح (*db_status64)(sqlite3*,صحيح,sqlite3_int64*,sqlite3_int64*,صحيح);
  /* Version 3.52.0 and later */
  فراغ (*str_truncate)(sqlite3_str*,صحيح);
  فراغ (*str_free)(sqlite3_str*);
  صحيح (*carray_bind)(sqlite3_stmt*,صحيح,فراغ*,صحيح,صحيح,فراغ(*)(فراغ*));
  صحيح (*carray_bind_v2)(sqlite3_stmt*,صحيح,فراغ*,صحيح,صحيح,فراغ(*)(فراغ*),فراغ*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ثابت حرف*);
  فراغ (*نتیجه_str)(sqlite3_context*,sqlite3_str*,صحيح);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
تعريفنوع صحيح (*sqlite3_loadext_مدخل)(
  sqlite3 *db,                       /* Handle to the database. */
  حرف **pz_Err_پیام,                   /* Used to set error string on failure. */
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
#إذا !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_بارگیری_EXTENSION)
#تعریف_کن sqlite3_aggregate_context      sqlite3_api->aggregate_context
#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
#تعریف_کن sqlite3_aggregate_شمار        sqlite3_api->aggregate_شمار
#پایان_اگر
#تعریف_کن sqlite3_bind_blob              sqlite3_api->bind_blob
#تعریف_کن sqlite3_bind_double            sqlite3_api->bind_double
#تعریف_کن sqlite3_bind_int               sqlite3_api->bind_int
#تعریف_کن sqlite3_bind_int64             sqlite3_api->bind_int64
#تعریف_کن sqlite3_bind_null              sqlite3_api->bind_null
#تعریف_کن sqlite3_bind_parameter_شمار   sqlite3_api->bind_parameter_شمار
#تعریف_کن sqlite3_bind_parameter_شاخص   sqlite3_api->bind_parameter_شاخص
#تعریف_کن sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#تعریف_کن sqlite3_bind_متن              sqlite3_api->bind_متن
#تعریف_کن sqlite3_bind_text16            sqlite3_api->bind_text16
#تعریف_کن sqlite3_bind_مقدار             sqlite3_api->bind_مقدار
#تعریف_کن sqlite3_busy_handler           sqlite3_api->busy_handler
#تعریف_کن sqlite3_busy_timeout           sqlite3_api->busy_timeout
#تعریف_کن sqlite3_changes                sqlite3_api->changes
#تعریف_کن sqlite3_close                  sqlite3_api->close
#تعریف_کن sqlite3_collation_needed       sqlite3_api->collation_needed
#تعریف_کن sqlite3_collation_needed16     sqlite3_api->collation_needed16
#تعریف_کن sqlite3_ستونی_blob            sqlite3_api->ستونی_blob
#تعریف_کن sqlite3_ستونی_بایتها           sqlite3_api->ستونی_بایتها
#تعریف_کن sqlite3_ستونی_bytes16         sqlite3_api->ستونی_bytes16
#تعریف_کن sqlite3_ستونی_شمار           sqlite3_api->ستونی_شمار
#تعریف_کن sqlite3_ستونی_database_name   sqlite3_api->ستونی_database_name
#تعریف_کن sqlite3_ستونی_database_name16 sqlite3_api->ستونی_database_name16
#تعریف_کن sqlite3_ستونی_decltype        sqlite3_api->ستونی_decltype
#تعریف_کن sqlite3_ستونی_decltype16      sqlite3_api->ستونی_decltype16
#تعریف_کن sqlite3_ستونی_double          sqlite3_api->ستونی_double
#تعریف_کن sqlite3_ستونی_int             sqlite3_api->ستونی_int
#تعریف_کن sqlite3_ستونی_int64           sqlite3_api->ستونی_int64
#تعریف_کن sqlite3_ستونی_name            sqlite3_api->ستونی_name
#تعریف_کن sqlite3_ستونی_name16          sqlite3_api->ستونی_name16
#تعریف_کن sqlite3_ستونی_origin_name     sqlite3_api->ستونی_origin_name
#تعریف_کن sqlite3_ستونی_origin_name16   sqlite3_api->ستونی_origin_name16
#تعریف_کن sqlite3_ستونی_جدول_name      sqlite3_api->ستونی_جدول_name
#تعریف_کن sqlite3_ستونی_جدول_name16    sqlite3_api->ستونی_جدول_name16
#تعریف_کن sqlite3_ستونی_متن            sqlite3_api->ستونی_متن
#تعریف_کن sqlite3_ستونی_text16          sqlite3_api->ستونی_text16
#تعریف_کن sqlite3_ستونی_type            sqlite3_api->ستونی_type
#تعریف_کن sqlite3_ستونی_مقدار           sqlite3_api->ستونی_مقدار
#تعریف_کن sqlite3_commit_hook            sqlite3_api->commit_hook
#تعریف_کن sqlite3_complete               sqlite3_api->complete
#تعریف_کن sqlite3_complete16             sqlite3_api->complete16
#تعریف_کن sqlite3_create_collation       sqlite3_api->create_collation
#تعریف_کن sqlite3_create_collation16     sqlite3_api->create_collation16
#تعریف_کن sqlite3_create_function        sqlite3_api->create_function
#تعریف_کن sqlite3_create_function16      sqlite3_api->create_function16
#تعریف_کن sqlite3_create_module          sqlite3_api->create_module
#تعریف_کن sqlite3_create_module_v2       sqlite3_api->create_module_v2
#تعریف_کن sqlite3_داده_شمار             sqlite3_api->داده_شمار
#تعریف_کن sqlite3_db_handle              sqlite3_api->db_handle
#تعریف_کن sqlite3_declare_vtab           sqlite3_api->declare_vtab
#تعریف_کن sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#تعریف_کن sqlite3_errcode                sqlite3_api->errcode
#تعریف_کن sqlite3_errmsg                 sqlite3_api->errmsg
#تعریف_کن sqlite3_errmsg16               sqlite3_api->errmsg16
#تعریف_کن sqlite3_exec                   sqlite3_api->exec
#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
#تعریف_کن sqlite3_expired                sqlite3_api->expired
#پایان_اگر
#تعریف_کن sqlite3_finalize               sqlite3_api->finalize
#تعریف_کن sqlite3_free                   sqlite3_api->حرر
#تعریف_کن sqlite3_free_جدول             sqlite3_api->free_جدول
#تعریف_کن sqlite3_get_autocommit         sqlite3_api->get_autocommit
#تعریف_کن sqlite3_get_auxdata            sqlite3_api->get_auxdata
#تعریف_کن sqlite3_get_جدول              sqlite3_api->get_جدول
#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
#تعریف_کن sqlite3_global_recover         sqlite3_api->global_recover
#پایان_اگر
#تعریف_کن sqlite3_وقفه              sqlite3_api->interruptx
#تعریف_کن sqlite3_آخرین_درج_rowid      sqlite3_api->آخرین_درج_rowid
#تعریف_کن sqlite3_libversion             sqlite3_api->libversion
#تعریف_کن sqlite3_libversion_عدد      sqlite3_api->libversion_عدد
#تعریف_کن sqlite3_malloc                 sqlite3_api->حافظه_بده
#تعریف_کن sqlite3_mprintf                sqlite3_api->mprintf
#تعریف_کن sqlite3_open                   sqlite3_api->open
#تعریف_کن sqlite3_open16                 sqlite3_api->open16
#تعریف_کن sqlite3_prepare                sqlite3_api->prepare
#تعریف_کن sqlite3_prepare16              sqlite3_api->prepare16
#تعریف_کن sqlite3_prepare_v2             sqlite3_api->prepare_v2
#تعریف_کن sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#تعریف_کن sqlite3_profile                sqlite3_api->profile
#تعریف_کن sqlite3_پیشرفت_handler       sqlite3_api->پیشرفت_handler
#تعریف_کن sqlite3_realloc                sqlite3_api->realloc
#تعریف_کن sqlite3_reset                  sqlite3_api->reset
#تعریف_کن sqlite3_نتیجه_blob            sqlite3_api->نتیجه_blob
#تعریف_کن sqlite3_نتیجه_double          sqlite3_api->نتیجه_double
#تعریف_کن sqlite3_نتیجه_error           sqlite3_api->نتیجه_error
#تعریف_کن sqlite3_نتیجه_error16         sqlite3_api->نتیجه_error16
#تعریف_کن sqlite3_نتیجه_int             sqlite3_api->نتیجه_int
#تعریف_کن sqlite3_نتیجه_int64           sqlite3_api->نتیجه_int64
#تعریف_کن sqlite3_نتیجه_null            sqlite3_api->نتیجه_null
#تعریف_کن sqlite3_نتیجه_متن            sqlite3_api->نتیجه_متن
#تعریف_کن sqlite3_نتیجه_text16          sqlite3_api->نتیجه_text16
#تعریف_کن sqlite3_نتیجه_text16be        sqlite3_api->نتیجه_text16be
#تعریف_کن sqlite3_نتیجه_text16le        sqlite3_api->نتیجه_text16le
#تعریف_کن sqlite3_نتیجه_مقدار           sqlite3_api->نتیجه_مقدار
#تعریف_کن sqlite3_rollback_hook          sqlite3_api->rollback_hook
#تعریف_کن sqlite3_مجموعه_authorizer         sqlite3_api->مجموعه_authorizer
#تعریف_کن sqlite3_مجموعه_auxdata            sqlite3_api->مجموعه_auxdata
#تعریف_کن sqlite3_snprintf               sqlite3_api->xsnprintf
#تعریف_کن sqlite3_گام                   sqlite3_api->گام
#تعریف_کن sqlite3_جدول_ستونی_metadata  sqlite3_api->جدول_ستونی_metadata
#تعریف_کن sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#تعریف_کن sqlite3_مجموع_changes          sqlite3_api->مجموع_changes
#تعریف_کن sqlite3_trace                  sqlite3_api->trace
#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
#تعریف_کن sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#پایان_اگر
#تعریف_کن sqlite3_update_hook            sqlite3_api->update_hook
#تعریف_کن sqlite3_user_داده              sqlite3_api->user_داده
#تعریف_کن sqlite3_مقدار_blob             sqlite3_api->مقدار_blob
#تعریف_کن sqlite3_مقدار_بایتها            sqlite3_api->مقدار_بایتها
#تعریف_کن sqlite3_مقدار_bytes16          sqlite3_api->مقدار_bytes16
#تعریف_کن sqlite3_مقدار_double           sqlite3_api->مقدار_double
#تعریف_کن sqlite3_مقدار_int              sqlite3_api->مقدار_int
#تعریف_کن sqlite3_مقدار_int64            sqlite3_api->مقدار_int64
#تعریف_کن sqlite3_مقدار_numeric_type     sqlite3_api->مقدار_numeric_type
#تعریف_کن sqlite3_مقدار_متن             sqlite3_api->مقدار_متن
#تعریف_کن sqlite3_مقدار_text16           sqlite3_api->مقدار_text16
#تعریف_کن sqlite3_مقدار_text16be         sqlite3_api->مقدار_text16be
#تعریف_کن sqlite3_مقدار_text16le         sqlite3_api->مقدار_text16le
#تعریف_کن sqlite3_مقدار_type             sqlite3_api->مقدار_type
#تعریف_کن sqlite3_vmprintf               sqlite3_api->vmprintf
#تعریف_کن sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#تعریف_کن sqlite3_overload_function      sqlite3_api->overload_function
#تعریف_کن sqlite3_prepare_v2             sqlite3_api->prepare_v2
#تعریف_کن sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#تعریف_کن sqlite3_clear_bindings         sqlite3_api->clear_bindings
#تعریف_کن sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#تعریف_کن sqlite3_blob_بایتها             sqlite3_api->blob_بایتها
#تعریف_کن sqlite3_blob_close             sqlite3_api->blob_close
#تعریف_کن sqlite3_blob_open              sqlite3_api->blob_open
#تعریف_کن sqlite3_blob_read              sqlite3_api->blob_read
#تعریف_کن sqlite3_blob_write             sqlite3_api->blob_write
#تعریف_کن sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#تعریف_کن sqlite3_file_control           sqlite3_api->file_control
#تعریف_کن sqlite3_حافظه_highwater       sqlite3_api->حافظه_highwater
#تعریف_کن sqlite3_حافظه_استفاده_شده            sqlite3_api->حافظه_استفاده_شده
#تعریف_کن sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#تعریف_کن sqlite3_mutex_enter            sqlite3_api->mutex_enter
#تعریف_کن sqlite3_mutex_free             sqlite3_api->mutex_free
#تعریف_کن sqlite3_mutex_leave            sqlite3_api->mutex_leave
#تعریف_کن sqlite3_mutex_try              sqlite3_api->mutex_try
#تعریف_کن sqlite3_open_v2                sqlite3_api->open_v2
#تعریف_کن sqlite3_release_حافظه         sqlite3_api->release_حافظه
#تعریف_کن sqlite3_نتیجه_error_nomem     sqlite3_api->نتیجه_error_nomem
#تعریف_کن sqlite3_نتیجه_error_toobig    sqlite3_api->نتیجه_error_toobig
#تعریف_کن sqlite3_sleep                  sqlite3_api->sleep
#تعریف_کن sqlite3_soft_انباشته_حد        sqlite3_api->soft_انباشته_حد
#تعریف_کن sqlite3_vfs_پیدا_کن               sqlite3_api->vfs_پیدا_کن
#تعریف_کن sqlite3_vfs_ثبات           sqlite3_api->vfs_ثبات
#تعریف_کن sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#تعریف_کن sqlite3_threadsafe             sqlite3_api->xthreadsafe
#تعریف_کن sqlite3_نتیجه_zeroblob        sqlite3_api->نتیجه_zeroblob
#تعریف_کن sqlite3_نتیجه_error_کد      sqlite3_api->نتیجه_error_کد
#تعریف_کن sqlite3_test_control           sqlite3_api->test_control
#تعریف_کن sqlite3_randomness             sqlite3_api->randomness
#تعریف_کن sqlite3_context_db_handle      sqlite3_api->context_db_handle
#تعریف_کن sqlite3_extended_نتیجه_codes  sqlite3_api->extended_نتیجه_codes
#تعریف_کن sqlite3_حد                  sqlite3_api->حد
#تعریف_کن sqlite3_بعدی_stmt              sqlite3_api->بعدی_stmt
#تعریف_کن sqlite3_sql                    sqlite3_api->sql
#تعریف_کن sqlite3_وضعیت                 sqlite3_api->وضعیت
#تعریف_کن sqlite3_backup_پایان          sqlite3_api->backup_پایان
#تعریف_کن sqlite3_backup_init            sqlite3_api->backup_init
#تعریف_کن sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#تعریف_کن sqlite3_backup_مانده       sqlite3_api->backup_مانده
#تعریف_کن sqlite3_backup_گام            sqlite3_api->backup_گام
#تعریف_کن sqlite3_compileoption_get      sqlite3_api->compileoption_get
#تعریف_کن sqlite3_compileoption_استفاده_شده     sqlite3_api->compileoption_استفاده_شده
#تعریف_کن sqlite3_create_function_v2     sqlite3_api->create_function_v2
#تعریف_کن sqlite3_db_config              sqlite3_api->db_config
#تعریف_کن sqlite3_db_mutex               sqlite3_api->db_mutex
#تعریف_کن sqlite3_db_وضعیت              sqlite3_api->db_وضعیت
#تعریف_کن sqlite3_extended_errcode       sqlite3_api->extended_errcode
#تعریف_کن sqlite3_log                    sqlite3_api->لگاریتم
#تعریف_کن sqlite3_soft_انباشته_limit64      sqlite3_api->soft_انباشته_limit64
#تعریف_کن sqlite3_sourceid               sqlite3_api->sourceid
#تعریف_کن sqlite3_stmt_وضعیت            sqlite3_api->stmt_وضعیت
#تعریف_کن sqlite3_strnicmp               sqlite3_api->strnicmp
#تعریف_کن sqlite3_unlock_notify          sqlite3_api->unlock_notify
#تعریف_کن sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#تعریف_کن sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#تعریف_کن sqlite3_wal_hook               sqlite3_api->wal_hook
#تعریف_کن sqlite3_blob_reopen            sqlite3_api->blob_reopen
#تعریف_کن sqlite3_vtab_config            sqlite3_api->vtab_config
#تعریف_کن sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#تعریف_کن sqlite3_close_v2               sqlite3_api->close_v2
#تعریف_کن sqlite3_db_filename            sqlite3_api->db_filename
#تعریف_کن sqlite3_db_readonly            sqlite3_api->db_readonly
#تعریف_کن sqlite3_db_release_حافظه      sqlite3_api->db_release_حافظه
#تعریف_کن sqlite3_errstr                 sqlite3_api->errstr
#تعریف_کن sqlite3_stmt_busy              sqlite3_api->stmt_busy
#تعریف_کن sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#تعریف_کن sqlite3_stricmp                sqlite3_api->stricmp
#تعریف_کن sqlite3_uri_boolean            sqlite3_api->uri_boolean
#تعریف_کن sqlite3_uri_int64              sqlite3_api->uri_int64
#تعریف_کن sqlite3_uri_parameter          sqlite3_api->uri_parameter
#تعریف_کن sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#تعریف_کن sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#تعریف_کن sqlite3_auto_extension         sqlite3_api->auto_extension
#تعریف_کن sqlite3_bind_blob64            sqlite3_api->bind_blob64
#تعریف_کن sqlite3_bind_text64            sqlite3_api->bind_text64
#تعریف_کن sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#تعریف_کن sqlite3_بارگیری_extension         sqlite3_api->بارگیری_extension
#تعریف_کن sqlite3_malloc64               sqlite3_api->malloc64
#تعریف_کن sqlite3_msize                  sqlite3_api->msize
#تعریف_کن sqlite3_realloc64              sqlite3_api->realloc64
#تعریف_کن sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#تعریف_کن sqlite3_نتیجه_blob64          sqlite3_api->نتیجه_blob64
#تعریف_کن sqlite3_نتیجه_text64          sqlite3_api->نتیجه_text64
#تعریف_کن sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#تعریف_کن sqlite3_مقدار_dup              sqlite3_api->مقدار_dup
#تعریف_کن sqlite3_مقدار_free             sqlite3_api->مقدار_free
#تعریف_کن sqlite3_نتیجه_zeroblob64      sqlite3_api->نتیجه_zeroblob64
#تعریف_کن sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#تعریف_کن sqlite3_مقدار_subtype          sqlite3_api->مقدار_subtype
#تعریف_کن sqlite3_نتیجه_subtype         sqlite3_api->نتیجه_subtype
/* Version 3.10.0 and later */
#تعریف_کن sqlite3_status64               sqlite3_api->status64
#تعریف_کن sqlite3_strlike                sqlite3_api->strlike
#تعریف_کن sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#تعریف_کن sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#تعریف_کن sqlite3_trace_v2               sqlite3_api->trace_v2
#تعریف_کن sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#تعریف_کن sqlite3_مجموعه_آخرین_درج_rowid  sqlite3_api->مجموعه_آخرین_درج_rowid
/* Version 3.20.0 and later */
#تعریف_کن sqlite3_prepare_v3             sqlite3_api->prepare_v3
#تعریف_کن sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#تعریف_کن sqlite3_bind_اشاره_گر           sqlite3_api->bind_اشاره_گر
#تعریف_کن sqlite3_نتیجه_اشاره_گر         sqlite3_api->نتیجه_اشاره_گر
#تعریف_کن sqlite3_مقدار_اشاره_گر          sqlite3_api->مقدار_اشاره_گر
/* Version 3.22.0 and later */
#تعریف_کن sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#تعریف_کن sqlite3_مقدار_nochange         sqlite3_api->مقدار_nochange
#تعریف_کن sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#تعریف_کن sqlite3_keyword_شمار          sqlite3_api->keyword_شمار
#تعریف_کن sqlite3_keyword_name           sqlite3_api->keyword_name
#تعریف_کن sqlite3_keyword_check          sqlite3_api->keyword_check
#تعریف_کن sqlite3_str_new                sqlite3_api->str_new
#تعریف_کن sqlite3_str_پایان             sqlite3_api->str_پایان
#تعریف_کن sqlite3_str_appendf            sqlite3_api->str_appendf
#تعریف_کن sqlite3_str_vappendf           sqlite3_api->str_vappendf
#تعریف_کن sqlite3_str_append             sqlite3_api->str_append
#تعریف_کن sqlite3_str_appendall          sqlite3_api->str_appendall
#تعریف_کن sqlite3_str_appendchar         sqlite3_api->str_appendchar
#تعریف_کن sqlite3_str_reset              sqlite3_api->str_reset
#تعریف_کن sqlite3_str_errcode            sqlite3_api->str_errcode
#تعریف_کن sqlite3_str_درازی             sqlite3_api->str_درازی
#تعریف_کن sqlite3_str_مقدار              sqlite3_api->str_مقدار
/* Version 3.25.0 and later */
#تعریف_کن sqlite3_create_پنجره_function sqlite3_api->create_پنجره_function
/* Version 3.26.0 and later */
#تعریف_کن sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#تعریف_کن sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#تعریف_کن sqlite3_مقدار_frombind         sqlite3_api->مقدار_frombind
/* Version 3.30.0 and later */
#تعریف_کن sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#تعریف_کن sqlite3_hard_انباشته_limit64      sqlite3_api->hard_انباشته_limit64
#تعریف_کن sqlite3_uri_کلید                sqlite3_api->uri_کلید
#تعریف_کن sqlite3_filename_database      sqlite3_api->filename_database
#تعریف_کن sqlite3_filename_journal       sqlite3_api->filename_journal
#تعریف_کن sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#تعریف_کن sqlite3_create_filename        sqlite3_api->create_filename
#تعریف_کن sqlite3_free_filename          sqlite3_api->free_filename
#تعریف_کن sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#تعریف_کن sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#تعریف_کن sqlite3_changes64              sqlite3_api->changes64
#تعریف_کن sqlite3_مجموع_changes64        sqlite3_api->مجموع_changes64
/* Version 3.37.0 and later */
#تعریف_کن sqlite3_autovacuum_صفحات       sqlite3_api->autovacuum_صفحات
/* Version 3.38.0 and later */
#تعریف_کن sqlite3_error_جابه_جایی           sqlite3_api->error_جابه_جایی
#تعریف_کن sqlite3_vtab_rhs_مقدار         sqlite3_api->vtab_rhs_مقدار
#تعریف_کن sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#تعریف_کن sqlite3_vtab_درون                sqlite3_api->vtab_درون
#تعریف_کن sqlite3_vtab_درون_اولی          sqlite3_api->vtab_درون_اولی
#تعریف_کن sqlite3_vtab_درون_بعدی           sqlite3_api->vtab_درون_بعدی
/* Version 3.39.0 and later */
#اگر_تعریف_نشده SQLITE_OMIT_DESERIALIZE
#تعریف_کن sqlite3_deserialize            sqlite3_api->deserialize
#تعریف_کن sqlite3_serialize              sqlite3_api->serialize
#پایان_اگر
#تعریف_کن sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#تعریف_کن sqlite3_مقدار_encoding         sqlite3_api->مقدار_encoding
/* Version 3.41.0 and later */
#تعریف_کن sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#تعریف_کن sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#تعریف_کن sqlite3_get_clientdata         sqlite3_api->get_clientdata
#تعریف_کن sqlite3_مجموعه_clientdata         sqlite3_api->مجموعه_clientdata
/* Version 3.50.0 and later */
#تعریف_کن sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#تعریف_کن sqlite3_مجموعه_errmsg             sqlite3_api->مجموعه_errmsg
#تعریف_کن sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#تعریف_کن sqlite3_str_truncate           sqlite3_api->str_truncate
#تعریف_کن sqlite3_str_free               sqlite3_api->str_free
#تعریف_کن sqlite3_carray_bind            sqlite3_api->carray_bind
#تعریف_کن sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#تعریف_کن sqlite3_incomplete             sqlite3_api->incomplete
#تعریف_کن sqlite3_نتیجه_str             sqlite3_api->نتیجه_str
#پایان_اگر /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#إذا !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_بارگیری_EXTENSION)
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
#پایان_اگر

#پایان_اگر /* SQLITE3EXT_H */
