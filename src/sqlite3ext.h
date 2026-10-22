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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE3EXT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE3EXT_H
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"

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
ਬਣਤਰ sqlite3_api_routines {
  ਖਾਲੀ * (*aggregate_context)(sqlite3_context*,ਪੂਰਨ n_ਬਾਈਟਾਂ);
  ਪੂਰਨ  (*aggregate_ਗਿਣਤੀ)(sqlite3_context*);
  ਪੂਰਨ  (*bind_blob)(sqlite3_stmt*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ n,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਪੂਰਨ  (*bind_double)(sqlite3_stmt*,ਪੂਰਨ,ਦੁੱਗਣਾ);
  ਪੂਰਨ  (*bind_int)(sqlite3_stmt*,ਪੂਰਨ,ਪੂਰਨ);
  ਪੂਰਨ  (*bind_int64)(sqlite3_stmt*,ਪੂਰਨ,sqlite_int64);
  ਪੂਰਨ  (*bind_null)(sqlite3_stmt*,ਪੂਰਨ);
  ਪੂਰਨ  (*bind_parameter_ਗਿਣਤੀ)(sqlite3_stmt*);
  ਪੂਰਨ  (*bind_parameter_ਸੂਚਕਾਂਕ)(sqlite3_stmt*,ਅਚਲ ਅੱਖਰ*zName);
  ਅਚਲ ਅੱਖਰ * (*bind_parameter_name)(sqlite3_stmt*,ਪੂਰਨ);
  ਪੂਰਨ  (*bind_ਪਾਠ)(sqlite3_stmt*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ n,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਪੂਰਨ  (*bind_text16)(sqlite3_stmt*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਪੂਰਨ  (*bind_ਮੁੱਲ)(sqlite3_stmt*,ਪੂਰਨ,ਅਚਲ sqlite3_ਮੁੱਲ*);
  ਪੂਰਨ  (*busy_handler)(sqlite3*,ਪੂਰਨ(*)(ਖਾਲੀ*,ਪੂਰਨ),ਖਾਲੀ*);
  ਪੂਰਨ  (*busy_timeout)(sqlite3*,ਪੂਰਨ ms);
  ਪੂਰਨ  (*changes)(sqlite3*);
  ਪੂਰਨ  (*close)(sqlite3*);
  ਪੂਰਨ  (*collation_needed)(sqlite3*,ਖਾਲੀ*,ਖਾਲੀ(*)(ਖਾਲੀ*,sqlite3*,
                           ਪੂਰਨ e_ਪਾਠ_Rep,ਅਚਲ ਅੱਖਰ*));
  ਪੂਰਨ  (*collation_needed16)(sqlite3*,ਖਾਲੀ*,ਖਾਲੀ(*)(ਖਾਲੀ*,sqlite3*,
                             ਪੂਰਨ e_ਪਾਠ_Rep,ਅਚਲ ਖਾਲੀ*));
  ਅਚਲ ਖਾਲੀ * (*ਕਾਲਮ_blob)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  ਪੂਰਨ  (*ਕਾਲਮ_ਬਾਈਟਾਂ)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  ਪੂਰਨ  (*ਕਾਲਮ_bytes16)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  ਪੂਰਨ  (*ਕਾਲਮ_ਗਿਣਤੀ)(sqlite3_stmt*pStmt);
  ਅਚਲ ਅੱਖਰ * (*ਕਾਲਮ_database_name)(sqlite3_stmt*,ਪੂਰਨ);
  ਅਚਲ ਖਾਲੀ * (*ਕਾਲਮ_database_name16)(sqlite3_stmt*,ਪੂਰਨ);
  ਅਚਲ ਅੱਖਰ * (*ਕਾਲਮ_decltype)(sqlite3_stmt*,ਪੂਰਨ i);
  ਅਚਲ ਖਾਲੀ * (*ਕਾਲਮ_decltype16)(sqlite3_stmt*,ਪੂਰਨ);
  ਦੁੱਗਣਾ  (*ਕਾਲਮ_double)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  ਪੂਰਨ  (*ਕਾਲਮ_int)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  sqlite_int64  (*ਕਾਲਮ_int64)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  ਅਚਲ ਅੱਖਰ * (*ਕਾਲਮ_name)(sqlite3_stmt*,ਪੂਰਨ);
  ਅਚਲ ਖਾਲੀ * (*ਕਾਲਮ_name16)(sqlite3_stmt*,ਪੂਰਨ);
  ਅਚਲ ਅੱਖਰ * (*ਕਾਲਮ_origin_name)(sqlite3_stmt*,ਪੂਰਨ);
  ਅਚਲ ਖਾਲੀ * (*ਕਾਲਮ_origin_name16)(sqlite3_stmt*,ਪੂਰਨ);
  ਅਚਲ ਅੱਖਰ * (*ਕਾਲਮ_ਸਾਰਣੀ_name)(sqlite3_stmt*,ਪੂਰਨ);
  ਅਚਲ ਖਾਲੀ * (*ਕਾਲਮ_ਸਾਰਣੀ_name16)(sqlite3_stmt*,ਪੂਰਨ);
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ * (*ਕਾਲਮ_ਪਾਠ)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  ਅਚਲ ਖਾਲੀ * (*ਕਾਲਮ_text16)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  ਪੂਰਨ  (*ਕਾਲਮ_type)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  sqlite3_ਮੁੱਲ* (*ਕਾਲਮ_ਮੁੱਲ)(sqlite3_stmt*,ਪੂਰਨ i_ਖਾਨਾ);
  ਖਾਲੀ * (*commit_hook)(sqlite3*,ਪੂਰਨ(*)(ਖਾਲੀ*),ਖਾਲੀ*);
  ਪੂਰਨ  (*complete)(ਅਚਲ ਅੱਖਰ*sql);
  ਪੂਰਨ  (*complete16)(ਅਚਲ ਖਾਲੀ*sql);
  ਪੂਰਨ  (*create_collation)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਖਾਲੀ*,
                           ਪੂਰਨ(*)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*));
  ਪੂਰਨ  (*create_collation16)(sqlite3*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਖਾਲੀ*,
                             ਪੂਰਨ(*)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*));
  ਪੂਰਨ  (*create_function)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਪੂਰਨ,ਖਾਲੀ*,
                          ਖਾਲੀ (*xFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
                          ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
                          ਖਾਲੀ (*xFinal)(sqlite3_context*));
  ਪੂਰਨ  (*create_function16)(sqlite3*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਪੂਰਨ,ਖਾਲੀ*,
                            ਖਾਲੀ (*xFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
                            ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
                            ਖਾਲੀ (*xFinal)(sqlite3_context*));
  ਪੂਰਨ (*create_module)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ sqlite3_module*,ਖਾਲੀ*);
  ਪੂਰਨ  (*ਅੰਕੜੇ_ਗਿਣਤੀ)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  ਪੂਰਨ (*declare_vtab)(sqlite3*,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ  (*enable_shared_cache)(ਪੂਰਨ);
  ਪੂਰਨ  (*errcode)(sqlite3*db);
  ਅਚਲ ਅੱਖਰ * (*errmsg)(sqlite3*);
  ਅਚਲ ਖਾਲੀ * (*errmsg16)(sqlite3*);
  ਪੂਰਨ  (*exec)(sqlite3*,ਅਚਲ ਅੱਖਰ*,sqlite3_callback,ਖਾਲੀ*,ਅੱਖਰ**);
  ਪੂਰਨ  (*expired)(sqlite3_stmt*);
  ਪੂਰਨ  (*finalize)(sqlite3_stmt*pStmt);
  ਖਾਲੀ  (*ਮੁਕਤ)(ਖਾਲੀ*);
  ਖਾਲੀ  (*free_ਸਾਰਣੀ)(ਅੱਖਰ**ਨਤੀਜਾ);
  ਪੂਰਨ  (*get_autocommit)(sqlite3*);
  ਖਾਲੀ * (*get_auxdata)(sqlite3_context*,ਪੂਰਨ);
  ਪੂਰਨ  (*get_ਸਾਰਣੀ)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਅੱਖਰ***,ਪੂਰਨ*,ਪੂਰਨ*,ਅੱਖਰ**);
  ਪੂਰਨ  (*global_recover)(ਖਾਲੀ);
  ਖਾਲੀ  (*interruptx)(sqlite3*);
  sqlite_int64  (*ਆਖਰੀ_ਪਾਓ_rowid)(sqlite3*);
  ਅਚਲ ਅੱਖਰ * (*libversion)(ਖਾਲੀ);
  ਪੂਰਨ  (*libversion_ਸੰਖਿਆ)(ਖਾਲੀ);
  ਖਾਲੀ *(*ਸਮ੍ਰਿਤੀ_ਦਿਓ)(ਪੂਰਨ);
  ਅੱਖਰ *(SQLITE_CDECL*mprintf)(ਅਚਲ ਅੱਖਰ*,...);
  ਪੂਰਨ  (*open)(ਅਚਲ ਅੱਖਰ*,sqlite3**);
  ਪੂਰਨ  (*open16)(ਅਚਲ ਖਾਲੀ*,sqlite3**);
  ਪੂਰਨ  (*prepare)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,sqlite3_stmt**,ਅਚਲ ਅੱਖਰ**);
  ਪੂਰਨ  (*prepare16)(sqlite3*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,sqlite3_stmt**,ਅਚਲ ਖਾਲੀ**);
  ਖਾਲੀ * (*profile)(sqlite3*,ਖਾਲੀ(*)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*,sqlite_uint64),ਖਾਲੀ*);
  ਖਾਲੀ  (*ਤਰੱਕੀ_handler)(sqlite3*,ਪੂਰਨ,ਪੂਰਨ(*)(ਖਾਲੀ*),ਖਾਲੀ*);
  ਖਾਲੀ *(*realloc)(ਖਾਲੀ*,ਪੂਰਨ);
  ਪੂਰਨ  (*reset)(sqlite3_stmt*pStmt);
  ਖਾਲੀ  (*ਨਤੀਜਾ_blob)(sqlite3_context*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਖਾਲੀ  (*ਨਤੀਜਾ_double)(sqlite3_context*,ਦੁੱਗਣਾ);
  ਖਾਲੀ  (*ਨਤੀਜਾ_error)(sqlite3_context*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);
  ਖਾਲੀ  (*ਨਤੀਜਾ_error16)(sqlite3_context*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ);
  ਖਾਲੀ  (*ਨਤੀਜਾ_int)(sqlite3_context*,ਪੂਰਨ);
  ਖਾਲੀ  (*ਨਤੀਜਾ_int64)(sqlite3_context*,sqlite_int64);
  ਖਾਲੀ  (*ਨਤੀਜਾ_null)(sqlite3_context*);
  ਖਾਲੀ  (*ਨਤੀਜਾ_ਪਾਠ)(sqlite3_context*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਖਾਲੀ  (*ਨਤੀਜਾ_text16)(sqlite3_context*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਖਾਲੀ  (*ਨਤੀਜਾ_text16be)(sqlite3_context*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਖਾਲੀ  (*ਨਤੀਜਾ_text16le)(sqlite3_context*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਖਾਲੀ  (*ਨਤੀਜਾ_ਮੁੱਲ)(sqlite3_context*,sqlite3_ਮੁੱਲ*);
  ਖਾਲੀ *(*rollback_hook)(sqlite3*,ਖਾਲੀ(*)(ਖਾਲੀ*),ਖਾਲੀ*);
  ਪੂਰਨ  (*ਸੈੱਟ_authorizer)(sqlite3*,ਪੂਰਨ(*)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,
                         ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*),ਖਾਲੀ*);
  ਖਾਲੀ  (*ਸੈੱਟ_auxdata)(sqlite3_context*,ਪੂਰਨ,ਖਾਲੀ*,ਖਾਲੀ (*)(ਖਾਲੀ*));
  ਅੱਖਰ *(SQLITE_CDECL*xsnprintf)(ਪੂਰਨ,ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,...);
  ਪੂਰਨ  (*ਕਦਮ)(sqlite3_stmt*);
  ਪੂਰਨ  (*ਸਾਰਣੀ_ਕਾਲਮ_metadata)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,
                                ਅੱਖਰ ਅਚਲ**,ਅੱਖਰ ਅਚਲ**,ਪੂਰਨ*,ਪੂਰਨ*,ਪੂਰਨ*);
  ਖਾਲੀ  (*thread_cleanup)(ਖਾਲੀ);
  ਪੂਰਨ  (*ਕੁੱਲ_changes)(sqlite3*);
  ਖਾਲੀ * (*trace)(sqlite3*,ਖਾਲੀ(*xTrace)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*),ਖਾਲੀ*);
  ਪੂਰਨ  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  ਖਾਲੀ * (*update_hook)(sqlite3*,ਖਾਲੀ(*)(ਖਾਲੀ*,ਪੂਰਨ ,ਅੱਖਰ ਅਚਲ*,ਅੱਖਰ ਅਚਲ*,
                                         sqlite_int64),ਖਾਲੀ*);
  ਖਾਲੀ * (*user_ਅੰਕੜੇ)(sqlite3_context*);
  ਅਚਲ ਖਾਲੀ * (*ਮੁੱਲ_blob)(sqlite3_ਮੁੱਲ*);
  ਪੂਰਨ  (*ਮੁੱਲ_ਬਾਈਟਾਂ)(sqlite3_ਮੁੱਲ*);
  ਪੂਰਨ  (*ਮੁੱਲ_bytes16)(sqlite3_ਮੁੱਲ*);
  ਦੁੱਗਣਾ  (*ਮੁੱਲ_double)(sqlite3_ਮੁੱਲ*);
  ਪੂਰਨ  (*ਮੁੱਲ_int)(sqlite3_ਮੁੱਲ*);
  sqlite_int64  (*ਮੁੱਲ_int64)(sqlite3_ਮੁੱਲ*);
  ਪੂਰਨ  (*ਮੁੱਲ_numeric_type)(sqlite3_ਮੁੱਲ*);
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ * (*ਮੁੱਲ_ਪਾਠ)(sqlite3_ਮੁੱਲ*);
  ਅਚਲ ਖਾਲੀ * (*ਮੁੱਲ_text16)(sqlite3_ਮੁੱਲ*);
  ਅਚਲ ਖਾਲੀ * (*ਮੁੱਲ_text16be)(sqlite3_ਮੁੱਲ*);
  ਅਚਲ ਖਾਲੀ * (*ਮੁੱਲ_text16le)(sqlite3_ਮੁੱਲ*);
  ਪੂਰਨ  (*ਮੁੱਲ_type)(sqlite3_ਮੁੱਲ*);
  ਅੱਖਰ *(*vmprintf)(ਅਚਲ ਅੱਖਰ*,va_list);
  /* Added ??? */
  ਪੂਰਨ (*overload_function)(sqlite3*, ਅਚਲ ਅੱਖਰ *zFuncName, ਪੂਰਨ nArg);
  /* Added by 3.3.13 */
  ਪੂਰਨ (*prepare_v2)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,sqlite3_stmt**,ਅਚਲ ਅੱਖਰ**);
  ਪੂਰਨ (*prepare16_v2)(sqlite3*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,sqlite3_stmt**,ਅਚਲ ਖਾਲੀ**);
  ਪੂਰਨ (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  ਪੂਰਨ (*create_module_v2)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ sqlite3_module*,ਖਾਲੀ*,
                          ਖਾਲੀ (*xDestroy)(ਖਾਲੀ *));
  /* Added by 3.5.0 */
  ਪੂਰਨ (*bind_zeroblob)(sqlite3_stmt*,ਪੂਰਨ,ਪੂਰਨ);
  ਪੂਰਨ (*blob_ਬਾਈਟਾਂ)(sqlite3_blob*);
  ਪੂਰਨ (*blob_close)(sqlite3_blob*);
  ਪੂਰਨ (*blob_open)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,sqlite3_int64,
                   ਪੂਰਨ,sqlite3_blob**);
  ਪੂਰਨ (*blob_read)(sqlite3_blob*,ਖਾਲੀ*,ਪੂਰਨ,ਪੂਰਨ);
  ਪੂਰਨ (*blob_write)(sqlite3_blob*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਪੂਰਨ);
  ਪੂਰਨ (*create_collation_v2)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਖਾਲੀ*,
                             ਪੂਰਨ(*)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*),
                             ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਪੂਰਨ (*file_control)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਖਾਲੀ*);
  sqlite3_int64 (*ਸਮ੍ਰਿਤੀ_highwater)(ਪੂਰਨ);
  sqlite3_int64 (*ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ)(ਖਾਲੀ);
  sqlite3_mutex *(*mutex_alloc)(ਪੂਰਨ);
  ਖਾਲੀ (*mutex_enter)(sqlite3_mutex*);
  ਖਾਲੀ (*mutex_free)(sqlite3_mutex*);
  ਖਾਲੀ (*mutex_leave)(sqlite3_mutex*);
  ਪੂਰਨ (*mutex_try)(sqlite3_mutex*);
  ਪੂਰਨ (*open_v2)(ਅਚਲ ਅੱਖਰ*,sqlite3**,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*release_ਸਮ੍ਰਿਤੀ)(ਪੂਰਨ);
  ਖਾਲੀ (*ਨਤੀਜਾ_error_nomem)(sqlite3_context*);
  ਖਾਲੀ (*ਨਤੀਜਾ_error_toobig)(sqlite3_context*);
  ਪੂਰਨ (*sleep)(ਪੂਰਨ);
  ਖਾਲੀ (*soft_ਢੇਰ_ਹੱਦਬੰਦੀ)(ਪੂਰਨ);
  sqlite3_vfs *(*vfs_ਲੱਭੋ)(ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*vfs_ਰਜਿਸਟਰ)(sqlite3_vfs*,ਪੂਰਨ);
  ਪੂਰਨ (*vfs_unregister)(sqlite3_vfs*);
  ਪੂਰਨ (*xthreadsafe)(ਖਾਲੀ);
  ਖਾਲੀ (*ਨਤੀਜਾ_zeroblob)(sqlite3_context*,ਪੂਰਨ);
  ਖਾਲੀ (*ਨਤੀਜਾ_error_ਕੋਡ)(sqlite3_context*,ਪੂਰਨ);
  ਪੂਰਨ (SQLITE_CDECL*test_control)(ਪੂਰਨ, ...);
  ਖਾਲੀ (*randomness)(ਪੂਰਨ,ਖਾਲੀ*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  ਪੂਰਨ (*extended_ਨਤੀਜਾ_codes)(sqlite3*,ਪੂਰਨ);
  ਪੂਰਨ (*ਹੱਦਬੰਦੀ)(sqlite3*,ਪੂਰਨ,ਪੂਰਨ);
  sqlite3_stmt *(*ਅਗਲਾ_stmt)(sqlite3*,sqlite3_stmt*);
  ਅਚਲ ਅੱਖਰ *(*sql)(sqlite3_stmt*);
  ਪੂਰਨ (*ਸਥਿਤੀ)(ਪੂਰਨ,ਪੂਰਨ*,ਪੂਰਨ*,ਪੂਰਨ);
  ਪੂਰਨ (*backup_ਅੰਤ)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ਅਚਲ ਅੱਖਰ*,sqlite3*,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*backup_pagecount)(sqlite3_backup*);
  ਪੂਰਨ (*backup_ਬਚਿਆ)(sqlite3_backup*);
  ਪੂਰਨ (*backup_ਕਦਮ)(sqlite3_backup*,ਪੂਰਨ);
  ਅਚਲ ਅੱਖਰ *(*compileoption_get)(ਪੂਰਨ);
  ਪੂਰਨ (*compileoption_ਵਰਤਿਆ)(ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*create_function_v2)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਪੂਰਨ,ਖਾਲੀ*,
                            ਖਾਲੀ (*xFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
                            ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
                            ਖਾਲੀ (*xFinal)(sqlite3_context*),
                            ਖਾਲੀ(*xDestroy)(ਖਾਲੀ*));
  ਪੂਰਨ (SQLITE_CDECL*db_config)(sqlite3*,ਪੂਰਨ,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  ਪੂਰਨ (*db_ਸਥਿਤੀ)(sqlite3*,ਪੂਰਨ,ਪੂਰਨ*,ਪੂਰਨ*,ਪੂਰਨ);
  ਪੂਰਨ (*extended_errcode)(sqlite3*);
  ਖਾਲੀ (SQLITE_CDECL*ਲਘੂਗਣਕ)(ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*,...);
  sqlite3_int64 (*soft_ਢੇਰ_limit64)(sqlite3_int64);
  ਅਚਲ ਅੱਖਰ *(*sourceid)(ਖਾਲੀ);
  ਪੂਰਨ (*stmt_ਸਥਿਤੀ)(sqlite3_stmt*,ਪੂਰਨ,ਪੂਰਨ);
  ਪੂਰਨ (*strnicmp)(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);
  ਪੂਰਨ (*unlock_notify)(sqlite3*,ਖਾਲੀ(*)(ਖਾਲੀ**,ਪੂਰਨ),ਖਾਲੀ*);
  ਪੂਰਨ (*wal_autocheckpoint)(sqlite3*,ਪੂਰਨ);
  ਪੂਰਨ (*wal_checkpoint)(sqlite3*,ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ *(*wal_hook)(sqlite3*,ਪੂਰਨ(*)(ਖਾਲੀ*,sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ),ਖਾਲੀ*);
  ਪੂਰਨ (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  ਪੂਰਨ (SQLITE_CDECL*vtab_config)(sqlite3*,ਪੂਰਨ op,...);
  ਪੂਰਨ (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  ਪੂਰਨ (*close_v2)(sqlite3*);
  ਅਚਲ ਅੱਖਰ *(*db_filename)(sqlite3*,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*db_readonly)(sqlite3*,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*db_release_ਸਮ੍ਰਿਤੀ)(sqlite3*);
  ਅਚਲ ਅੱਖਰ *(*errstr)(ਪੂਰਨ);
  ਪੂਰਨ (*stmt_busy)(sqlite3_stmt*);
  ਪੂਰਨ (*stmt_readonly)(sqlite3_stmt*);
  ਪੂਰਨ (*stricmp)(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*uri_boolean)(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);
  sqlite3_int64 (*uri_int64)(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,sqlite3_int64);
  ਅਚਲ ਅੱਖਰ *(*uri_parameter)(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
  ਅੱਖਰ *(*xvsnprintf)(ਪੂਰਨ,ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,va_list);
  ਪੂਰਨ (*wal_checkpoint_v2)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਪੂਰਨ*,ਪੂਰਨ*);
  /* Version 3.8.7 and later */
  ਪੂਰਨ (*auto_extension)(ਖਾਲੀ(*)(ਖਾਲੀ));
  ਪੂਰਨ (*bind_blob64)(sqlite3_stmt*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,sqlite3_uint64,
                     ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਪੂਰਨ (*bind_text64)(sqlite3_stmt*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*,sqlite3_uint64,
                      ਖਾਲੀ(*)(ਖਾਲੀ*),ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ);
  ਪੂਰਨ (*cancel_auto_extension)(ਖਾਲੀ(*)(ਖਾਲੀ));
  ਪੂਰਨ (*ਲੱਦੋ_extension)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਅੱਖਰ**);
  ਖਾਲੀ *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(ਖਾਲੀ*);
  ਖਾਲੀ *(*realloc64)(ਖਾਲੀ*,sqlite3_uint64);
  ਖਾਲੀ (*reset_auto_extension)(ਖਾਲੀ);
  ਖਾਲੀ (*ਨਤੀਜਾ_blob64)(sqlite3_context*,ਅਚਲ ਖਾਲੀ*,sqlite3_uint64,
                        ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਖਾਲੀ (*ਨਤੀਜਾ_text64)(sqlite3_context*,ਅਚਲ ਅੱਖਰ*,sqlite3_uint64,
                         ਖਾਲੀ(*)(ਖਾਲੀ*), ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ);
  ਪੂਰਨ (*strglob)(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
  /* Version 3.8.11 and later */
  sqlite3_ਮੁੱਲ *(*ਮੁੱਲ_dup)(ਅਚਲ sqlite3_ਮੁੱਲ*);
  ਖਾਲੀ (*ਮੁੱਲ_free)(sqlite3_ਮੁੱਲ*);
  ਪੂਰਨ (*ਨਤੀਜਾ_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  ਪੂਰਨ (*bind_zeroblob64)(sqlite3_stmt*, ਪੂਰਨ, sqlite3_uint64);
  /* Version 3.9.0 and later */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ (*ਮੁੱਲ_subtype)(sqlite3_ਮੁੱਲ*);
  ਖਾਲੀ (*ਨਤੀਜਾ_subtype)(sqlite3_context*,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
  /* Version 3.10.0 and later */
  ਪੂਰਨ (*status64)(ਪੂਰਨ,sqlite3_int64*,sqlite3_int64*,ਪੂਰਨ);
  ਪੂਰਨ (*strlike)(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
  ਪੂਰਨ (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  ਪੂਰਨ (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  ਪੂਰਨ (*trace_v2)(sqlite3*,ਬਿਨਾਨਿਸ਼ਾਨੀ,ਪੂਰਨ(*)(ਬਿਨਾਨਿਸ਼ਾਨੀ,ਖਾਲੀ*,ਖਾਲੀ*,ਖਾਲੀ*),ਖਾਲੀ*);
  ਅੱਖਰ *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  ਖਾਲੀ (*ਸੈੱਟ_ਆਖਰੀ_ਪਾਓ_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  ਪੂਰਨ (*prepare_v3)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ,
                    sqlite3_stmt**,ਅਚਲ ਅੱਖਰ**);
  ਪੂਰਨ (*prepare16_v3)(sqlite3*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ,
                      sqlite3_stmt**,ਅਚਲ ਖਾਲੀ**);
  ਪੂਰਨ (*bind_ਸੂਚਕ)(sqlite3_stmt*,ਪੂਰਨ,ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਖਾਲੀ (*ਨਤੀਜਾ_ਸੂਚਕ)(sqlite3_context*,ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਖਾਲੀ *(*ਮੁੱਲ_ਸੂਚਕ)(sqlite3_ਮੁੱਲ*,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*vtab_nochange)(sqlite3_context*);
  ਪੂਰਨ (*ਮੁੱਲ_nochange)(sqlite3_ਮੁੱਲ*);
  ਅਚਲ ਅੱਖਰ *(*vtab_collation)(sqlite3_ਸੂਚਕਾਂਕ_info*,ਪੂਰਨ);
  /* Version 3.24.0 and later */
  ਪੂਰਨ (*keyword_ਗਿਣਤੀ)(ਖਾਲੀ);
  ਪੂਰਨ (*keyword_name)(ਪੂਰਨ,ਅਚਲ ਅੱਖਰ**,ਪੂਰਨ*);
  ਪੂਰਨ (*keyword_check)(ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);
  sqlite3_str *(*str_new)(sqlite3*);
  ਅੱਖਰ *(*str_ਅੰਤ)(sqlite3_str*);
  ਖਾਲੀ (SQLITE_CDECL*str_appendf)(sqlite3_str*, ਅਚਲ ਅੱਖਰ *zFormat, ...);
  ਖਾਲੀ (*str_vappendf)(sqlite3_str*, ਅਚਲ ਅੱਖਰ *zFormat, va_list);
  ਖਾਲੀ (*str_append)(sqlite3_str*, ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ, ਪੂਰਨ N);
  ਖਾਲੀ (*str_appendall)(sqlite3_str*, ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ);
  ਖਾਲੀ (*str_appendchar)(sqlite3_str*, ਪੂਰਨ N, ਅੱਖਰ C);
  ਖਾਲੀ (*str_reset)(sqlite3_str*);
  ਪੂਰਨ (*str_errcode)(sqlite3_str*);
  ਪੂਰਨ (*str_ਲੰਮਾਈ)(sqlite3_str*);
  ਅੱਖਰ *(*str_ਮੁੱਲ)(sqlite3_str*);
  /* Version 3.25.0 and later */
  ਪੂਰਨ (*create_ਖਿੜਕੀ_function)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਪੂਰਨ,ਖਾਲੀ*,
                            ਖਾਲੀ (*x_ਕਦਮ)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
                            ਖਾਲੀ (*xFinal)(sqlite3_context*),
                            ਖਾਲੀ (*x_ਮੁੱਲ)(sqlite3_context*),
                            ਖਾਲੀ (*xInv)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),
                            ਖਾਲੀ(*xDestroy)(ਖਾਲੀ*));
  /* Version 3.26.0 and later */
  ਅਚਲ ਅੱਖਰ *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  ਪੂਰਨ (*stmt_isexplain)(sqlite3_stmt*);
  ਪੂਰਨ (*ਮੁੱਲ_frombind)(sqlite3_ਮੁੱਲ*);
  /* Version 3.30.0 and later */
  ਪੂਰਨ (*drop_modules)(sqlite3*,ਅਚਲ ਅੱਖਰ**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_ਢੇਰ_limit64)(sqlite3_int64);
  ਅਚਲ ਅੱਖਰ *(*uri_ਕੁੰਜੀ)(ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);
  ਅਚਲ ਅੱਖਰ *(*filename_database)(ਅਚਲ ਅੱਖਰ*);
  ਅਚਲ ਅੱਖਰ *(*filename_journal)(ਅਚਲ ਅੱਖਰ*);
  ਅਚਲ ਅੱਖਰ *(*filename_wal)(ਅਚਲ ਅੱਖਰ*);
  /* Version 3.32.0 and later */
  ਅਚਲ ਅੱਖਰ *(*create_filename)(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,
                           ਪੂਰਨ,ਅਚਲ ਅੱਖਰ**);
  ਖਾਲੀ (*free_filename)(ਅਚਲ ਅੱਖਰ*);
  sqlite3_file *(*database_file_object)(ਅਚਲ ਅੱਖਰ*);
  /* Version 3.34.0 and later */
  ਪੂਰਨ (*txn_state)(sqlite3*,ਅਚਲ ਅੱਖਰ*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*ਕੁੱਲ_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  ਪੂਰਨ (*autovacuum_ਪੰਨੇ)(sqlite3*,
     ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ(*)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ),
     ਖਾਲੀ*, ਖਾਲੀ(*)(ਖਾਲੀ*));
  /* Version 3.38.0 and later */
  ਪੂਰਨ (*error_ਵਿੱਥ)(sqlite3*);
  ਪੂਰਨ (*vtab_rhs_ਮੁੱਲ)(sqlite3_ਸੂਚਕਾਂਕ_info*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**);
  ਪੂਰਨ (*vtab_distinct)(sqlite3_ਸੂਚਕਾਂਕ_info*);
  ਪੂਰਨ (*vtab_ਵਿੱਚ)(sqlite3_ਸੂਚਕਾਂਕ_info*,ਪੂਰਨ,ਪੂਰਨ);
  ਪੂਰਨ (*vtab_ਵਿੱਚ_ਪਹਿਲਾ)(sqlite3_ਮੁੱਲ*,sqlite3_ਮੁੱਲ**);
  ਪੂਰਨ (*vtab_ਵਿੱਚ_ਅਗਲਾ)(sqlite3_ਮੁੱਲ*,sqlite3_ਮੁੱਲ**);
  /* Version 3.39.0 and later */
  ਪੂਰਨ (*deserialize)(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*,
                     sqlite3_int64,sqlite3_int64,ਬਿਨਾਨਿਸ਼ਾਨੀ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *(*serialize)(sqlite3*,ਅਚਲ ਅੱਖਰ *,sqlite3_int64*,
                              ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
  ਅਚਲ ਅੱਖਰ *(*db_name)(sqlite3*,ਪੂਰਨ);
  /* Version 3.40.0 and later */
  ਪੂਰਨ (*ਮੁੱਲ_encoding)(sqlite3_ਮੁੱਲ*);
  /* Version 3.41.0 and later */
  ਪੂਰਨ (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  ਪੂਰਨ (*stmt_explain)(sqlite3_stmt*,ਪੂਰਨ);
  /* Version 3.44.0 and later */
  ਖਾਲੀ *(*get_clientdata)(sqlite3*,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*ਸੈੱਟ_clientdata)(sqlite3*, ਅਚਲ ਅੱਖਰ*, ਖਾਲੀ*, ਖਾਲੀ(*)(ਖਾਲੀ*));
  /* Version 3.50.0 and later */
  ਪੂਰਨ (*setlk_timeout)(sqlite3*,ਪੂਰਨ,ਪੂਰਨ);
  /* Version 3.51.0 and later */
  ਪੂਰਨ (*ਸੈੱਟ_errmsg)(sqlite3*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
  ਪੂਰਨ (*db_status64)(sqlite3*,ਪੂਰਨ,sqlite3_int64*,sqlite3_int64*,ਪੂਰਨ);
  /* Version 3.52.0 and later */
  ਖਾਲੀ (*str_truncate)(sqlite3_str*,ਪੂਰਨ);
  ਖਾਲੀ (*str_free)(sqlite3_str*);
  ਪੂਰਨ (*carray_bind)(sqlite3_stmt*,ਪੂਰਨ,ਖਾਲੀ*,ਪੂਰਨ,ਪੂਰਨ,ਖਾਲੀ(*)(ਖਾਲੀ*));
  ਪੂਰਨ (*carray_bind_v2)(sqlite3_stmt*,ਪੂਰਨ,ਖਾਲੀ*,ਪੂਰਨ,ਪੂਰਨ,ਖਾਲੀ(*)(ਖਾਲੀ*),ਖਾਲੀ*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ (*ਨਤੀਜਾ_str)(sqlite3_context*,sqlite3_str*,ਪੂਰਨ);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*sqlite3_loadext_ਇੰਦਰਾਜ)(
  sqlite3 *db,                       /* Handle to the database. */
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ,                   /* Used to set error string on failure. */
  ਅਚਲ sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#ਜੇ !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_ਲੱਦੋ_EXTENSION)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_aggregate_ਗਿਣਤੀ        sqlite3_api->aggregate_ਗਿਣਤੀ
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_blob              sqlite3_api->bind_blob
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_double            sqlite3_api->bind_double
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_int               sqlite3_api->bind_int
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_int64             sqlite3_api->bind_int64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_null              sqlite3_api->bind_null
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_parameter_ਗਿਣਤੀ   sqlite3_api->bind_parameter_ਗਿਣਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_parameter_ਸੂਚਕਾਂਕ   sqlite3_api->bind_parameter_ਸੂਚਕਾਂਕ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_ਪਾਠ              sqlite3_api->bind_ਪਾਠ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_text16            sqlite3_api->bind_text16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_ਮੁੱਲ             sqlite3_api->bind_ਮੁੱਲ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_busy_handler           sqlite3_api->busy_handler
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_busy_timeout           sqlite3_api->busy_timeout
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_changes                sqlite3_api->changes
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_close                  sqlite3_api->close
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_collation_needed       sqlite3_api->collation_needed
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_collation_needed16     sqlite3_api->collation_needed16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_blob            sqlite3_api->ਕਾਲਮ_blob
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_ਬਾਈਟਾਂ           sqlite3_api->ਕਾਲਮ_ਬਾਈਟਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_bytes16         sqlite3_api->ਕਾਲਮ_bytes16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_ਗਿਣਤੀ           sqlite3_api->ਕਾਲਮ_ਗਿਣਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_database_name   sqlite3_api->ਕਾਲਮ_database_name
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_database_name16 sqlite3_api->ਕਾਲਮ_database_name16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_decltype        sqlite3_api->ਕਾਲਮ_decltype
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_decltype16      sqlite3_api->ਕਾਲਮ_decltype16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_double          sqlite3_api->ਕਾਲਮ_double
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_int             sqlite3_api->ਕਾਲਮ_int
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_int64           sqlite3_api->ਕਾਲਮ_int64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_name            sqlite3_api->ਕਾਲਮ_name
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_name16          sqlite3_api->ਕਾਲਮ_name16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_origin_name     sqlite3_api->ਕਾਲਮ_origin_name
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_origin_name16   sqlite3_api->ਕਾਲਮ_origin_name16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name      sqlite3_api->ਕਾਲਮ_ਸਾਰਣੀ_name
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_ਸਾਰਣੀ_name16    sqlite3_api->ਕਾਲਮ_ਸਾਰਣੀ_name16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_ਪਾਠ            sqlite3_api->ਕਾਲਮ_ਪਾਠ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_text16          sqlite3_api->ਕਾਲਮ_text16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_type            sqlite3_api->ਕਾਲਮ_type
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਾਲਮ_ਮੁੱਲ           sqlite3_api->ਕਾਲਮ_ਮੁੱਲ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_commit_hook            sqlite3_api->commit_hook
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_complete               sqlite3_api->complete
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_complete16             sqlite3_api->complete16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_collation       sqlite3_api->create_collation
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_collation16     sqlite3_api->create_collation16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_function        sqlite3_api->create_function
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_function16      sqlite3_api->create_function16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_module          sqlite3_api->create_module
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_module_v2       sqlite3_api->create_module_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਅੰਕੜੇ_ਗਿਣਤੀ             sqlite3_api->ਅੰਕੜੇ_ਗਿਣਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_handle              sqlite3_api->db_handle
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_declare_vtab           sqlite3_api->declare_vtab
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_errcode                sqlite3_api->errcode
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_errmsg                 sqlite3_api->errmsg
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_errmsg16               sqlite3_api->errmsg16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_exec                   sqlite3_api->exec
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_expired                sqlite3_api->expired
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_finalize               sqlite3_api->finalize
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_free                   sqlite3_api->ਮੁਕਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_free_ਸਾਰਣੀ             sqlite3_api->free_ਸਾਰਣੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_get_autocommit         sqlite3_api->get_autocommit
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_get_auxdata            sqlite3_api->get_auxdata
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_get_ਸਾਰਣੀ              sqlite3_api->get_ਸਾਰਣੀ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_global_recover         sqlite3_api->global_recover
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਰੁਕਾਵਟ              sqlite3_api->interruptx
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਆਖਰੀ_ਪਾਓ_rowid      sqlite3_api->ਆਖਰੀ_ਪਾਓ_rowid
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_libversion             sqlite3_api->libversion
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_libversion_ਸੰਖਿਆ      sqlite3_api->libversion_ਸੰਖਿਆ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_malloc                 sqlite3_api->ਸਮ੍ਰਿਤੀ_ਦਿਓ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mprintf                sqlite3_api->mprintf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_open                   sqlite3_api->open
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_open16                 sqlite3_api->open16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_prepare                sqlite3_api->prepare
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_prepare16              sqlite3_api->prepare16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_prepare_v2             sqlite3_api->prepare_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_profile                sqlite3_api->profile
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਤਰੱਕੀ_handler       sqlite3_api->ਤਰੱਕੀ_handler
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_realloc                sqlite3_api->realloc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_reset                  sqlite3_api->reset
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_blob            sqlite3_api->ਨਤੀਜਾ_blob
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_double          sqlite3_api->ਨਤੀਜਾ_double
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_error           sqlite3_api->ਨਤੀਜਾ_error
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_error16         sqlite3_api->ਨਤੀਜਾ_error16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_int             sqlite3_api->ਨਤੀਜਾ_int
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_int64           sqlite3_api->ਨਤੀਜਾ_int64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_null            sqlite3_api->ਨਤੀਜਾ_null
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_ਪਾਠ            sqlite3_api->ਨਤੀਜਾ_ਪਾਠ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_text16          sqlite3_api->ਨਤੀਜਾ_text16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_text16be        sqlite3_api->ਨਤੀਜਾ_text16be
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_text16le        sqlite3_api->ਨਤੀਜਾ_text16le
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_ਮੁੱਲ           sqlite3_api->ਨਤੀਜਾ_ਮੁੱਲ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_rollback_hook          sqlite3_api->rollback_hook
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸੈੱਟ_authorizer         sqlite3_api->ਸੈੱਟ_authorizer
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸੈੱਟ_auxdata            sqlite3_api->ਸੈੱਟ_auxdata
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_snprintf               sqlite3_api->xsnprintf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕਦਮ                   sqlite3_api->ਕਦਮ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_metadata  sqlite3_api->ਸਾਰਣੀ_ਕਾਲਮ_metadata
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕੁੱਲ_changes          sqlite3_api->ਕੁੱਲ_changes
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_trace                  sqlite3_api->trace
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_update_hook            sqlite3_api->update_hook
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_user_ਅੰਕੜੇ              sqlite3_api->user_ਅੰਕੜੇ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_blob             sqlite3_api->ਮੁੱਲ_blob
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ            sqlite3_api->ਮੁੱਲ_ਬਾਈਟਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_bytes16          sqlite3_api->ਮੁੱਲ_bytes16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_double           sqlite3_api->ਮੁੱਲ_double
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_int              sqlite3_api->ਮੁੱਲ_int
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_int64            sqlite3_api->ਮੁੱਲ_int64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_numeric_type     sqlite3_api->ਮੁੱਲ_numeric_type
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_ਪਾਠ             sqlite3_api->ਮੁੱਲ_ਪਾਠ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_text16           sqlite3_api->ਮੁੱਲ_text16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_text16be         sqlite3_api->ਮੁੱਲ_text16be
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_text16le         sqlite3_api->ਮੁੱਲ_text16le
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_type             sqlite3_api->ਮੁੱਲ_type
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vmprintf               sqlite3_api->vmprintf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_overload_function      sqlite3_api->overload_function
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_prepare_v2             sqlite3_api->prepare_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_clear_bindings         sqlite3_api->clear_bindings
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_ਬਾਈਟਾਂ             sqlite3_api->blob_ਬਾਈਟਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_close             sqlite3_api->blob_close
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_open              sqlite3_api->blob_open
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_read              sqlite3_api->blob_read
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_write             sqlite3_api->blob_write
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_file_control           sqlite3_api->file_control
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸਮ੍ਰਿਤੀ_highwater       sqlite3_api->ਸਮ੍ਰਿਤੀ_highwater
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ            sqlite3_api->ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_enter            sqlite3_api->mutex_enter
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_free             sqlite3_api->mutex_free
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_leave            sqlite3_api->mutex_leave
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_mutex_try              sqlite3_api->mutex_try
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_open_v2                sqlite3_api->open_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_release_ਸਮ੍ਰਿਤੀ         sqlite3_api->release_ਸਮ੍ਰਿਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_error_nomem     sqlite3_api->ਨਤੀਜਾ_error_nomem
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_error_toobig    sqlite3_api->ਨਤੀਜਾ_error_toobig
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_sleep                  sqlite3_api->sleep
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_soft_ਢੇਰ_ਹੱਦਬੰਦੀ        sqlite3_api->soft_ਢੇਰ_ਹੱਦਬੰਦੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vfs_ਲੱਭੋ               sqlite3_api->vfs_ਲੱਭੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vfs_ਰਜਿਸਟਰ           sqlite3_api->vfs_ਰਜਿਸਟਰ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_threadsafe             sqlite3_api->xthreadsafe
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_zeroblob        sqlite3_api->ਨਤੀਜਾ_zeroblob
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_error_ਕੋਡ      sqlite3_api->ਨਤੀਜਾ_error_ਕੋਡ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_test_control           sqlite3_api->test_control
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_randomness             sqlite3_api->randomness
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_context_db_handle      sqlite3_api->context_db_handle
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_extended_ਨਤੀਜਾ_codes  sqlite3_api->extended_ਨਤੀਜਾ_codes
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਹੱਦਬੰਦੀ                  sqlite3_api->ਹੱਦਬੰਦੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਅਗਲਾ_stmt              sqlite3_api->ਅਗਲਾ_stmt
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_sql                    sqlite3_api->sql
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸਥਿਤੀ                 sqlite3_api->ਸਥਿਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_backup_ਅੰਤ          sqlite3_api->backup_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_backup_init            sqlite3_api->backup_init
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_backup_ਬਚਿਆ       sqlite3_api->backup_ਬਚਿਆ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_backup_ਕਦਮ            sqlite3_api->backup_ਕਦਮ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_compileoption_get      sqlite3_api->compileoption_get
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_compileoption_ਵਰਤਿਆ     sqlite3_api->compileoption_ਵਰਤਿਆ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_function_v2     sqlite3_api->create_function_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_config              sqlite3_api->db_config
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_mutex               sqlite3_api->db_mutex
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_ਸਥਿਤੀ              sqlite3_api->db_ਸਥਿਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_extended_errcode       sqlite3_api->extended_errcode
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_log                    sqlite3_api->ਲਘੂਗਣਕ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_soft_ਢੇਰ_limit64      sqlite3_api->soft_ਢੇਰ_limit64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_sourceid               sqlite3_api->sourceid
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_stmt_ਸਥਿਤੀ            sqlite3_api->stmt_ਸਥਿਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_strnicmp               sqlite3_api->strnicmp
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_unlock_notify          sqlite3_api->unlock_notify
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_wal_hook               sqlite3_api->wal_hook
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_blob_reopen            sqlite3_api->blob_reopen
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_config            sqlite3_api->vtab_config
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_close_v2               sqlite3_api->close_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_filename            sqlite3_api->db_filename
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_readonly            sqlite3_api->db_readonly
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_release_ਸਮ੍ਰਿਤੀ      sqlite3_api->db_release_ਸਮ੍ਰਿਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_errstr                 sqlite3_api->errstr
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_stmt_busy              sqlite3_api->stmt_busy
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_stricmp                sqlite3_api->stricmp
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_uri_boolean            sqlite3_api->uri_boolean
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_uri_int64              sqlite3_api->uri_int64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_uri_parameter          sqlite3_api->uri_parameter
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_auto_extension         sqlite3_api->auto_extension
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_blob64            sqlite3_api->bind_blob64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_text64            sqlite3_api->bind_text64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਲੱਦੋ_extension         sqlite3_api->ਲੱਦੋ_extension
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_malloc64               sqlite3_api->malloc64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_msize                  sqlite3_api->msize
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_realloc64              sqlite3_api->realloc64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_blob64          sqlite3_api->ਨਤੀਜਾ_blob64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_text64          sqlite3_api->ਨਤੀਜਾ_text64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_dup              sqlite3_api->ਮੁੱਲ_dup
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_free             sqlite3_api->ਮੁੱਲ_free
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_zeroblob64      sqlite3_api->ਨਤੀਜਾ_zeroblob64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_subtype          sqlite3_api->ਮੁੱਲ_subtype
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_subtype         sqlite3_api->ਨਤੀਜਾ_subtype
/* Version 3.10.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_status64               sqlite3_api->status64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_strlike                sqlite3_api->strlike
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_trace_v2               sqlite3_api->trace_v2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸੈੱਟ_ਆਖਰੀ_ਪਾਓ_rowid  sqlite3_api->ਸੈੱਟ_ਆਖਰੀ_ਪਾਓ_rowid
/* Version 3.20.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_prepare_v3             sqlite3_api->prepare_v3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_bind_ਸੂਚਕ           sqlite3_api->bind_ਸੂਚਕ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_ਸੂਚਕ         sqlite3_api->ਨਤੀਜਾ_ਸੂਚਕ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_ਸੂਚਕ          sqlite3_api->ਮੁੱਲ_ਸੂਚਕ
/* Version 3.22.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_nochange         sqlite3_api->ਮੁੱਲ_nochange
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_keyword_ਗਿਣਤੀ          sqlite3_api->keyword_ਗਿਣਤੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_keyword_name           sqlite3_api->keyword_name
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_keyword_check          sqlite3_api->keyword_check
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_new                sqlite3_api->str_new
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_ਅੰਤ             sqlite3_api->str_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_appendf            sqlite3_api->str_appendf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_vappendf           sqlite3_api->str_vappendf
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_append             sqlite3_api->str_append
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_appendall          sqlite3_api->str_appendall
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_appendchar         sqlite3_api->str_appendchar
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_reset              sqlite3_api->str_reset
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_errcode            sqlite3_api->str_errcode
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_ਲੰਮਾਈ             sqlite3_api->str_ਲੰਮਾਈ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_ਮੁੱਲ              sqlite3_api->str_ਮੁੱਲ
/* Version 3.25.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_ਖਿੜਕੀ_function sqlite3_api->create_ਖਿੜਕੀ_function
/* Version 3.26.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_frombind         sqlite3_api->ਮੁੱਲ_frombind
/* Version 3.30.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_hard_ਢੇਰ_limit64      sqlite3_api->hard_ਢੇਰ_limit64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_uri_ਕੁੰਜੀ                sqlite3_api->uri_ਕੁੰਜੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_filename_database      sqlite3_api->filename_database
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_filename_journal       sqlite3_api->filename_journal
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_create_filename        sqlite3_api->create_filename
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_free_filename          sqlite3_api->free_filename
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_changes64              sqlite3_api->changes64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਕੁੱਲ_changes64        sqlite3_api->ਕੁੱਲ_changes64
/* Version 3.37.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_autovacuum_ਪੰਨੇ       sqlite3_api->autovacuum_ਪੰਨੇ
/* Version 3.38.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_error_ਵਿੱਥ           sqlite3_api->error_ਵਿੱਥ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_rhs_ਮੁੱਲ         sqlite3_api->vtab_rhs_ਮੁੱਲ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_ਵਿੱਚ                sqlite3_api->vtab_ਵਿੱਚ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_ਵਿੱਚ_ਪਹਿਲਾ          sqlite3_api->vtab_ਵਿੱਚ_ਪਹਿਲਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_vtab_ਵਿੱਚ_ਅਗਲਾ           sqlite3_api->vtab_ਵਿੱਚ_ਅਗਲਾ
/* Version 3.39.0 and later */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_deserialize            sqlite3_api->deserialize
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_serialize              sqlite3_api->serialize
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਮੁੱਲ_encoding         sqlite3_api->ਮੁੱਲ_encoding
/* Version 3.41.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_get_clientdata         sqlite3_api->get_clientdata
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸੈੱਟ_clientdata         sqlite3_api->ਸੈੱਟ_clientdata
/* Version 3.50.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸੈੱਟ_errmsg             sqlite3_api->ਸੈੱਟ_errmsg
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_truncate           sqlite3_api->str_truncate
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_str_free               sqlite3_api->str_free
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_carray_bind            sqlite3_api->carray_bind
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_incomplete             sqlite3_api->incomplete
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਨਤੀਜਾ_str             sqlite3_api->ਨਤੀਜਾ_str
#ਜੇ_ਅੰਤ /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#ਜੇ !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_ਲੱਦੋ_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ਅਚਲ sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    ਬਾਹਰੀ ਅਚਲ sqlite3_api_routines *sqlite3_api;
#ਨਹੀਂਤਾਂ
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (ਖਾਲੀ)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* SQLITE3EXT_H */
