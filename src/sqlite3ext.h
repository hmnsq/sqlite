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
#यदि_अपरिभाषितम् SQLITE3EXT_H
#परिभाषय SQLITE3EXT_H
#अन्तर्भाव "sqlite3.h"

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
संरचना sqlite3_api_routines {
  शून्य * (*aggregate_context)(sqlite3_context*,पूर्णाङ्क n_अष्टकानि);
  पूर्णाङ्क  (*aggregate_गणनम्)(sqlite3_context*);
  पूर्णाङ्क  (*bind_blob)(sqlite3_stmt*,पूर्णाङ्क,ध्रुव शून्य*,पूर्णाङ्क n,शून्य(*)(शून्य*));
  पूर्णाङ्क  (*bind_double)(sqlite3_stmt*,पूर्णाङ्क,द्विगुण);
  पूर्णाङ्क  (*bind_int)(sqlite3_stmt*,पूर्णाङ्क,पूर्णाङ्क);
  पूर्णाङ्क  (*bind_int64)(sqlite3_stmt*,पूर्णाङ्क,sqlite_int64);
  पूर्णाङ्क  (*bind_null)(sqlite3_stmt*,पूर्णाङ्क);
  पूर्णाङ्क  (*bind_parameter_गणनम्)(sqlite3_stmt*);
  पूर्णाङ्क  (*bind_parameter_सूचकांक)(sqlite3_stmt*,ध्रुव वर्ण*zName);
  ध्रुव वर्ण * (*bind_parameter_name)(sqlite3_stmt*,पूर्णाङ्क);
  पूर्णाङ्क  (*bind_text)(sqlite3_stmt*,पूर्णाङ्क,ध्रुव वर्ण*,पूर्णाङ्क n,शून्य(*)(शून्य*));
  पूर्णाङ्क  (*bind_text16)(sqlite3_stmt*,पूर्णाङ्क,ध्रुव शून्य*,पूर्णाङ्क,शून्य(*)(शून्य*));
  पूर्णाङ्क  (*bind_मानम्)(sqlite3_stmt*,पूर्णाङ्क,ध्रुव sqlite3_मानम्*);
  पूर्णाङ्क  (*busy_handler)(sqlite3*,पूर्णाङ्क(*)(शून्य*,पूर्णाङ्क),शून्य*);
  पूर्णाङ्क  (*busy_timeout)(sqlite3*,पूर्णाङ्क ms);
  पूर्णाङ्क  (*changes)(sqlite3*);
  पूर्णाङ्क  (*close)(sqlite3*);
  पूर्णाङ्क  (*collation_needed)(sqlite3*,शून्य*,शून्य(*)(शून्य*,sqlite3*,
                           पूर्णाङ्क eTextRep,ध्रुव वर्ण*));
  पूर्णाङ्क  (*collation_needed16)(sqlite3*,शून्य*,शून्य(*)(शून्य*,sqlite3*,
                             पूर्णाङ्क eTextRep,ध्रुव शून्य*));
  ध्रुव शून्य * (*column_blob)(sqlite3_stmt*,पूर्णाङ्क iCol);
  पूर्णाङ्क  (*column_अष्टकानि)(sqlite3_stmt*,पूर्णाङ्क iCol);
  पूर्णाङ्क  (*column_bytes16)(sqlite3_stmt*,पूर्णाङ्क iCol);
  पूर्णाङ्क  (*column_गणनम्)(sqlite3_stmt*pStmt);
  ध्रुव वर्ण * (*column_database_name)(sqlite3_stmt*,पूर्णाङ्क);
  ध्रुव शून्य * (*column_database_name16)(sqlite3_stmt*,पूर्णाङ्क);
  ध्रुव वर्ण * (*column_decltype)(sqlite3_stmt*,पूर्णाङ्क i);
  ध्रुव शून्य * (*column_decltype16)(sqlite3_stmt*,पूर्णाङ्क);
  द्विगुण  (*column_double)(sqlite3_stmt*,पूर्णाङ्क iCol);
  पूर्णाङ्क  (*column_int)(sqlite3_stmt*,पूर्णाङ्क iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,पूर्णाङ्क iCol);
  ध्रुव वर्ण * (*column_name)(sqlite3_stmt*,पूर्णाङ्क);
  ध्रुव शून्य * (*column_name16)(sqlite3_stmt*,पूर्णाङ्क);
  ध्रुव वर्ण * (*column_origin_name)(sqlite3_stmt*,पूर्णाङ्क);
  ध्रुव शून्य * (*column_origin_name16)(sqlite3_stmt*,पूर्णाङ्क);
  ध्रुव वर्ण * (*column_सारणी_name)(sqlite3_stmt*,पूर्णाङ्क);
  ध्रुव शून्य * (*column_सारणी_name16)(sqlite3_stmt*,पूर्णाङ्क);
  ध्रुव अचिह्नित वर्ण * (*column_text)(sqlite3_stmt*,पूर्णाङ्क iCol);
  ध्रुव शून्य * (*column_text16)(sqlite3_stmt*,पूर्णाङ्क iCol);
  पूर्णाङ्क  (*column_type)(sqlite3_stmt*,पूर्णाङ्क iCol);
  sqlite3_मानम्* (*column_मानम्)(sqlite3_stmt*,पूर्णाङ्क iCol);
  शून्य * (*commit_hook)(sqlite3*,पूर्णाङ्क(*)(शून्य*),शून्य*);
  पूर्णाङ्क  (*complete)(ध्रुव वर्ण*sql);
  पूर्णाङ्क  (*complete16)(ध्रुव शून्य*sql);
  पूर्णाङ्क  (*create_collation)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,शून्य*,
                           पूर्णाङ्क(*)(शून्य*,पूर्णाङ्क,ध्रुव शून्य*,पूर्णाङ्क,ध्रुव शून्य*));
  पूर्णाङ्क  (*create_collation16)(sqlite3*,ध्रुव शून्य*,पूर्णाङ्क,शून्य*,
                             पूर्णाङ्क(*)(शून्य*,पूर्णाङ्क,ध्रुव शून्य*,पूर्णाङ्क,ध्रुव शून्य*));
  पूर्णाङ्क  (*create_function)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,पूर्णाङ्क,शून्य*,
                          शून्य (*xFunc)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
                          शून्य (*x_चरणः)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
                          शून्य (*xFinal)(sqlite3_context*));
  पूर्णाङ्क  (*create_function16)(sqlite3*,ध्रुव शून्य*,पूर्णाङ्क,पूर्णाङ्क,शून्य*,
                            शून्य (*xFunc)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
                            शून्य (*x_चरणः)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
                            शून्य (*xFinal)(sqlite3_context*));
  पूर्णाङ्क (*create_module)(sqlite3*,ध्रुव वर्ण*,ध्रुव sqlite3_module*,शून्य*);
  पूर्णाङ्क  (*दत्त_गणनम्)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  पूर्णाङ्क (*declare_vtab)(sqlite3*,ध्रुव वर्ण*);
  पूर्णाङ्क  (*enable_shared_cache)(पूर्णाङ्क);
  पूर्णाङ्क  (*errcode)(sqlite3*db);
  ध्रुव वर्ण * (*errmsg)(sqlite3*);
  ध्रुव शून्य * (*errmsg16)(sqlite3*);
  पूर्णाङ्क  (*exec)(sqlite3*,ध्रुव वर्ण*,sqlite3_callback,शून्य*,वर्ण**);
  पूर्णाङ्क  (*expired)(sqlite3_stmt*);
  पूर्णाङ्क  (*finalize)(sqlite3_stmt*pStmt);
  शून्य  (*मुक्त)(शून्य*);
  शून्य  (*free_सारणी)(वर्ण**परिणाम);
  पूर्णाङ्क  (*get_autocommit)(sqlite3*);
  शून्य * (*get_auxdata)(sqlite3_context*,पूर्णाङ्क);
  पूर्णाङ्क  (*get_सारणी)(sqlite3*,ध्रुव वर्ण*,वर्ण***,पूर्णाङ्क*,पूर्णाङ्क*,वर्ण**);
  पूर्णाङ्क  (*global_recover)(शून्य);
  शून्य  (*interruptx)(sqlite3*);
  sqlite_int64  (*चरमः_निवेशय_rowid)(sqlite3*);
  ध्रुव वर्ण * (*libversion)(शून्य);
  पूर्णाङ्क  (*libversion_सङ्ख्या)(शून्य);
  शून्य *(*स्मृतिं_देहि)(पूर्णाङ्क);
  वर्ण *(SQLITE_CDECL*mprintf)(ध्रुव वर्ण*,...);
  पूर्णाङ्क  (*उद्घाटय)(ध्रुव वर्ण*,sqlite3**);
  पूर्णाङ्क  (*open16)(ध्रुव शून्य*,sqlite3**);
  पूर्णाङ्क  (*prepare)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,sqlite3_stmt**,ध्रुव वर्ण**);
  पूर्णाङ्क  (*prepare16)(sqlite3*,ध्रुव शून्य*,पूर्णाङ्क,sqlite3_stmt**,ध्रुव शून्य**);
  शून्य * (*profile)(sqlite3*,शून्य(*)(शून्य*,ध्रुव वर्ण*,sqlite_uint64),शून्य*);
  शून्य  (*प्रगतिः_handler)(sqlite3*,पूर्णाङ्क,पूर्णाङ्क(*)(शून्य*),शून्य*);
  शून्य *(*realloc)(शून्य*,पूर्णाङ्क);
  पूर्णाङ्क  (*reset)(sqlite3_stmt*pStmt);
  शून्य  (*परिणाम_blob)(sqlite3_context*,ध्रुव शून्य*,पूर्णाङ्क,शून्य(*)(शून्य*));
  शून्य  (*परिणाम_double)(sqlite3_context*,द्विगुण);
  शून्य  (*परिणाम_error)(sqlite3_context*,ध्रुव वर्ण*,पूर्णाङ्क);
  शून्य  (*परिणाम_error16)(sqlite3_context*,ध्रुव शून्य*,पूर्णाङ्क);
  शून्य  (*परिणाम_int)(sqlite3_context*,पूर्णाङ्क);
  शून्य  (*परिणाम_int64)(sqlite3_context*,sqlite_int64);
  शून्य  (*परिणाम_null)(sqlite3_context*);
  शून्य  (*परिणाम_text)(sqlite3_context*,ध्रुव वर्ण*,पूर्णाङ्क,शून्य(*)(शून्य*));
  शून्य  (*परिणाम_text16)(sqlite3_context*,ध्रुव शून्य*,पूर्णाङ्क,शून्य(*)(शून्य*));
  शून्य  (*परिणाम_text16be)(sqlite3_context*,ध्रुव शून्य*,पूर्णाङ्क,शून्य(*)(शून्य*));
  शून्य  (*परिणाम_text16le)(sqlite3_context*,ध्रुव शून्य*,पूर्णाङ्क,शून्य(*)(शून्य*));
  शून्य  (*परिणाम_मानम्)(sqlite3_context*,sqlite3_मानम्*);
  शून्य *(*rollback_hook)(sqlite3*,शून्य(*)(शून्य*),शून्य*);
  पूर्णाङ्क  (*समुच्चयः_authorizer)(sqlite3*,पूर्णाङ्क(*)(शून्य*,पूर्णाङ्क,ध्रुव वर्ण*,ध्रुव वर्ण*,
                         ध्रुव वर्ण*,ध्रुव वर्ण*),शून्य*);
  शून्य  (*समुच्चयः_auxdata)(sqlite3_context*,पूर्णाङ्क,शून्य*,शून्य (*)(शून्य*));
  वर्ण *(SQLITE_CDECL*xsnprintf)(पूर्णाङ्क,वर्ण*,ध्रुव वर्ण*,...);
  पूर्णाङ्क  (*चरणः)(sqlite3_stmt*);
  पूर्णाङ्क  (*सारणी_column_metadata)(sqlite3*,ध्रुव वर्ण*,ध्रुव वर्ण*,ध्रुव वर्ण*,
                                वर्ण ध्रुव**,वर्ण ध्रुव**,पूर्णाङ्क*,पूर्णाङ्क*,पूर्णाङ्क*);
  शून्य  (*thread_cleanup)(शून्य);
  पूर्णाङ्क  (*योग_changes)(sqlite3*);
  शून्य * (*trace)(sqlite3*,शून्य(*xTrace)(शून्य*,ध्रुव वर्ण*),शून्य*);
  पूर्णाङ्क  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  शून्य * (*update_hook)(sqlite3*,शून्य(*)(शून्य*,पूर्णाङ्क ,वर्ण ध्रुव*,वर्ण ध्रुव*,
                                         sqlite_int64),शून्य*);
  शून्य * (*user_दत्त)(sqlite3_context*);
  ध्रुव शून्य * (*मानम्_blob)(sqlite3_मानम्*);
  पूर्णाङ्क  (*मानम्_अष्टकानि)(sqlite3_मानम्*);
  पूर्णाङ्क  (*मानम्_bytes16)(sqlite3_मानम्*);
  द्विगुण  (*मानम्_double)(sqlite3_मानम्*);
  पूर्णाङ्क  (*मानम्_int)(sqlite3_मानम्*);
  sqlite_int64  (*मानम्_int64)(sqlite3_मानम्*);
  पूर्णाङ्क  (*मानम्_numeric_type)(sqlite3_मानम्*);
  ध्रुव अचिह्नित वर्ण * (*मानम्_text)(sqlite3_मानम्*);
  ध्रुव शून्य * (*मानम्_text16)(sqlite3_मानम्*);
  ध्रुव शून्य * (*मानम्_text16be)(sqlite3_मानम्*);
  ध्रुव शून्य * (*मानम्_text16le)(sqlite3_मानम्*);
  पूर्णाङ्क  (*मानम्_type)(sqlite3_मानम्*);
  वर्ण *(*vmprintf)(ध्रुव वर्ण*,va_list);
  /* Added ??? */
  पूर्णाङ्क (*overload_function)(sqlite3*, ध्रुव वर्ण *zFuncName, पूर्णाङ्क nArg);
  /* Added by 3.3.13 */
  पूर्णाङ्क (*prepare_v2)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,sqlite3_stmt**,ध्रुव वर्ण**);
  पूर्णाङ्क (*prepare16_v2)(sqlite3*,ध्रुव शून्य*,पूर्णाङ्क,sqlite3_stmt**,ध्रुव शून्य**);
  पूर्णाङ्क (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  पूर्णाङ्क (*create_module_v2)(sqlite3*,ध्रुव वर्ण*,ध्रुव sqlite3_module*,शून्य*,
                          शून्य (*xDestroy)(शून्य *));
  /* Added by 3.5.0 */
  पूर्णाङ्क (*bind_zeroblob)(sqlite3_stmt*,पूर्णाङ्क,पूर्णाङ्क);
  पूर्णाङ्क (*blob_अष्टकानि)(sqlite3_blob*);
  पूर्णाङ्क (*blob_close)(sqlite3_blob*);
  पूर्णाङ्क (*blob_open)(sqlite3*,ध्रुव वर्ण*,ध्रुव वर्ण*,ध्रुव वर्ण*,sqlite3_int64,
                   पूर्णाङ्क,sqlite3_blob**);
  पूर्णाङ्क (*blob_read)(sqlite3_blob*,शून्य*,पूर्णाङ्क,पूर्णाङ्क);
  पूर्णाङ्क (*blob_write)(sqlite3_blob*,ध्रुव शून्य*,पूर्णाङ्क,पूर्णाङ्क);
  पूर्णाङ्क (*create_collation_v2)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,शून्य*,
                             पूर्णाङ्क(*)(शून्य*,पूर्णाङ्क,ध्रुव शून्य*,पूर्णाङ्क,ध्रुव शून्य*),
                             शून्य(*)(शून्य*));
  पूर्णाङ्क (*file_control)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,शून्य*);
  sqlite3_int64 (*स्मृतिः_highwater)(पूर्णाङ्क);
  sqlite3_int64 (*स्मृतिः_प्रयुक्तम्)(शून्य);
  sqlite3_mutex *(*mutex_alloc)(पूर्णाङ्क);
  शून्य (*mutex_enter)(sqlite3_mutex*);
  शून्य (*mutex_free)(sqlite3_mutex*);
  शून्य (*mutex_leave)(sqlite3_mutex*);
  पूर्णाङ्क (*mutex_try)(sqlite3_mutex*);
  पूर्णाङ्क (*open_v2)(ध्रुव वर्ण*,sqlite3**,पूर्णाङ्क,ध्रुव वर्ण*);
  पूर्णाङ्क (*release_स्मृतिः)(पूर्णाङ्क);
  शून्य (*परिणाम_error_nomem)(sqlite3_context*);
  शून्य (*परिणाम_error_toobig)(sqlite3_context*);
  पूर्णाङ्क (*sleep)(पूर्णाङ्क);
  शून्य (*soft_heap_सीमा)(पूर्णाङ्क);
  sqlite3_vfs *(*vfs_find)(ध्रुव वर्ण*);
  पूर्णाङ्क (*vfs_पञ्जिकाकोशः)(sqlite3_vfs*,पूर्णाङ्क);
  पूर्णाङ्क (*vfs_unregister)(sqlite3_vfs*);
  पूर्णाङ्क (*xthreadsafe)(शून्य);
  शून्य (*परिणाम_zeroblob)(sqlite3_context*,पूर्णाङ्क);
  शून्य (*परिणाम_error_संहिता)(sqlite3_context*,पूर्णाङ्क);
  पूर्णाङ्क (SQLITE_CDECL*test_control)(पूर्णाङ्क, ...);
  शून्य (*randomness)(पूर्णाङ्क,शून्य*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  पूर्णाङ्क (*extended_परिणाम_codes)(sqlite3*,पूर्णाङ्क);
  पूर्णाङ्क (*सीमा)(sqlite3*,पूर्णाङ्क,पूर्णाङ्क);
  sqlite3_stmt *(*अग्र_stmt)(sqlite3*,sqlite3_stmt*);
  ध्रुव वर्ण *(*sql)(sqlite3_stmt*);
  पूर्णाङ्क (*स्थितिः)(पूर्णाङ्क,पूर्णाङ्क*,पूर्णाङ्क*,पूर्णाङ्क);
  पूर्णाङ्क (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ध्रुव वर्ण*,sqlite3*,ध्रुव वर्ण*);
  पूर्णाङ्क (*backup_pagecount)(sqlite3_backup*);
  पूर्णाङ्क (*backup_remaining)(sqlite3_backup*);
  पूर्णाङ्क (*backup_चरणः)(sqlite3_backup*,पूर्णाङ्क);
  ध्रुव वर्ण *(*compileoption_get)(पूर्णाङ्क);
  पूर्णाङ्क (*compileoption_प्रयुक्तम्)(ध्रुव वर्ण*);
  पूर्णाङ्क (*create_function_v2)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,पूर्णाङ्क,शून्य*,
                            शून्य (*xFunc)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
                            शून्य (*x_चरणः)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
                            शून्य (*xFinal)(sqlite3_context*),
                            शून्य(*xDestroy)(शून्य*));
  पूर्णाङ्क (SQLITE_CDECL*db_config)(sqlite3*,पूर्णाङ्क,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  पूर्णाङ्क (*db_स्थितिः)(sqlite3*,पूर्णाङ्क,पूर्णाङ्क*,पूर्णाङ्क*,पूर्णाङ्क);
  पूर्णाङ्क (*extended_errcode)(sqlite3*);
  शून्य (SQLITE_CDECL*लघुगणकः)(पूर्णाङ्क,ध्रुव वर्ण*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  ध्रुव वर्ण *(*sourceid)(शून्य);
  पूर्णाङ्क (*stmt_स्थितिः)(sqlite3_stmt*,पूर्णाङ्क,पूर्णाङ्क);
  पूर्णाङ्क (*strnicmp)(ध्रुव वर्ण*,ध्रुव वर्ण*,पूर्णाङ्क);
  पूर्णाङ्क (*unlock_notify)(sqlite3*,शून्य(*)(शून्य**,पूर्णाङ्क),शून्य*);
  पूर्णाङ्क (*wal_autocheckpoint)(sqlite3*,पूर्णाङ्क);
  पूर्णाङ्क (*wal_checkpoint)(sqlite3*,ध्रुव वर्ण*);
  शून्य *(*wal_hook)(sqlite3*,पूर्णाङ्क(*)(शून्य*,sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क),शून्य*);
  पूर्णाङ्क (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  पूर्णाङ्क (SQLITE_CDECL*vtab_config)(sqlite3*,पूर्णाङ्क op,...);
  पूर्णाङ्क (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  पूर्णाङ्क (*close_v2)(sqlite3*);
  ध्रुव वर्ण *(*db_filename)(sqlite3*,ध्रुव वर्ण*);
  पूर्णाङ्क (*db_readonly)(sqlite3*,ध्रुव वर्ण*);
  पूर्णाङ्क (*db_release_स्मृतिः)(sqlite3*);
  ध्रुव वर्ण *(*errstr)(पूर्णाङ्क);
  पूर्णाङ्क (*stmt_busy)(sqlite3_stmt*);
  पूर्णाङ्क (*stmt_readonly)(sqlite3_stmt*);
  पूर्णाङ्क (*stricmp)(ध्रुव वर्ण*,ध्रुव वर्ण*);
  पूर्णाङ्क (*uri_boolean)(ध्रुव वर्ण*,ध्रुव वर्ण*,पूर्णाङ्क);
  sqlite3_int64 (*uri_int64)(ध्रुव वर्ण*,ध्रुव वर्ण*,sqlite3_int64);
  ध्रुव वर्ण *(*uri_parameter)(ध्रुव वर्ण*,ध्रुव वर्ण*);
  वर्ण *(*xvsnprintf)(पूर्णाङ्क,वर्ण*,ध्रुव वर्ण*,va_list);
  पूर्णाङ्क (*wal_checkpoint_v2)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,पूर्णाङ्क*,पूर्णाङ्क*);
  /* Version 3.8.7 and later */
  पूर्णाङ्क (*auto_extension)(शून्य(*)(शून्य));
  पूर्णाङ्क (*bind_blob64)(sqlite3_stmt*,पूर्णाङ्क,ध्रुव शून्य*,sqlite3_uint64,
                     शून्य(*)(शून्य*));
  पूर्णाङ्क (*bind_text64)(sqlite3_stmt*,पूर्णाङ्क,ध्रुव वर्ण*,sqlite3_uint64,
                      शून्य(*)(शून्य*),अचिह्नित वर्ण);
  पूर्णाङ्क (*cancel_auto_extension)(शून्य(*)(शून्य));
  पूर्णाङ्क (*आरोपय_extension)(sqlite3*,ध्रुव वर्ण*,ध्रुव वर्ण*,वर्ण**);
  शून्य *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(शून्य*);
  शून्य *(*realloc64)(शून्य*,sqlite3_uint64);
  शून्य (*reset_auto_extension)(शून्य);
  शून्य (*परिणाम_blob64)(sqlite3_context*,ध्रुव शून्य*,sqlite3_uint64,
                        शून्य(*)(शून्य*));
  शून्य (*परिणाम_text64)(sqlite3_context*,ध्रुव वर्ण*,sqlite3_uint64,
                         शून्य(*)(शून्य*), अचिह्नित वर्ण);
  पूर्णाङ्क (*strglob)(ध्रुव वर्ण*,ध्रुव वर्ण*);
  /* Version 3.8.11 and later */
  sqlite3_मानम् *(*मानम्_dup)(ध्रुव sqlite3_मानम्*);
  शून्य (*मानम्_free)(sqlite3_मानम्*);
  पूर्णाङ्क (*परिणाम_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  पूर्णाङ्क (*bind_zeroblob64)(sqlite3_stmt*, पूर्णाङ्क, sqlite3_uint64);
  /* Version 3.9.0 and later */
  अचिह्नित पूर्णाङ्क (*मानम्_subtype)(sqlite3_मानम्*);
  शून्य (*परिणाम_subtype)(sqlite3_context*,अचिह्नित पूर्णाङ्क);
  /* Version 3.10.0 and later */
  पूर्णाङ्क (*status64)(पूर्णाङ्क,sqlite3_int64*,sqlite3_int64*,पूर्णाङ्क);
  पूर्णाङ्क (*strlike)(ध्रुव वर्ण*,ध्रुव वर्ण*,अचिह्नित पूर्णाङ्क);
  पूर्णाङ्क (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  पूर्णाङ्क (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  पूर्णाङ्क (*trace_v2)(sqlite3*,अचिह्नित,पूर्णाङ्क(*)(अचिह्नित,शून्य*,शून्य*,शून्य*),शून्य*);
  वर्ण *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  शून्य (*समुच्चयः_चरमः_निवेशय_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  पूर्णाङ्क (*prepare_v3)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,अचिह्नित पूर्णाङ्क,
                    sqlite3_stmt**,ध्रुव वर्ण**);
  पूर्णाङ्क (*prepare16_v3)(sqlite3*,ध्रुव शून्य*,पूर्णाङ्क,अचिह्नित पूर्णाङ्क,
                      sqlite3_stmt**,ध्रुव शून्य**);
  पूर्णाङ्क (*bind_pointer)(sqlite3_stmt*,पूर्णाङ्क,शून्य*,ध्रुव वर्ण*,शून्य(*)(शून्य*));
  शून्य (*परिणाम_pointer)(sqlite3_context*,शून्य*,ध्रुव वर्ण*,शून्य(*)(शून्य*));
  शून्य *(*मानम्_pointer)(sqlite3_मानम्*,ध्रुव वर्ण*);
  पूर्णाङ्क (*vtab_nochange)(sqlite3_context*);
  पूर्णाङ्क (*मानम्_nochange)(sqlite3_मानम्*);
  ध्रुव वर्ण *(*vtab_collation)(sqlite3_सूचकांक_info*,पूर्णाङ्क);
  /* Version 3.24.0 and later */
  पूर्णाङ्क (*keyword_गणनम्)(शून्य);
  पूर्णाङ्क (*keyword_name)(पूर्णाङ्क,ध्रुव वर्ण**,पूर्णाङ्क*);
  पूर्णाङ्क (*keyword_check)(ध्रुव वर्ण*,पूर्णाङ्क);
  sqlite3_str *(*str_new)(sqlite3*);
  वर्ण *(*str_finish)(sqlite3_str*);
  शून्य (SQLITE_CDECL*str_appendf)(sqlite3_str*, ध्रुव वर्ण *zFormat, ...);
  शून्य (*str_vappendf)(sqlite3_str*, ध्रुव वर्ण *zFormat, va_list);
  शून्य (*str_append)(sqlite3_str*, ध्रुव वर्ण *z_अन्तर्, पूर्णाङ्क N);
  शून्य (*str_appendall)(sqlite3_str*, ध्रुव वर्ण *z_अन्तर्);
  शून्य (*str_appendchar)(sqlite3_str*, पूर्णाङ्क N, वर्ण C);
  शून्य (*str_reset)(sqlite3_str*);
  पूर्णाङ्क (*str_errcode)(sqlite3_str*);
  पूर्णाङ्क (*str_दीर्घता)(sqlite3_str*);
  वर्ण *(*str_मानम्)(sqlite3_str*);
  /* Version 3.25.0 and later */
  पूर्णाङ्क (*create_गवाक्षः_function)(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,पूर्णाङ्क,शून्य*,
                            शून्य (*x_चरणः)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
                            शून्य (*xFinal)(sqlite3_context*),
                            शून्य (*x_मानम्)(sqlite3_context*),
                            शून्य (*xInv)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**),
                            शून्य(*xDestroy)(शून्य*));
  /* Version 3.26.0 and later */
  ध्रुव वर्ण *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  पूर्णाङ्क (*stmt_isexplain)(sqlite3_stmt*);
  पूर्णाङ्क (*मानम्_frombind)(sqlite3_मानम्*);
  /* Version 3.30.0 and later */
  पूर्णाङ्क (*drop_modules)(sqlite3*,ध्रुव वर्ण**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  ध्रुव वर्ण *(*uri_कुञ्चिका)(ध्रुव वर्ण*,पूर्णाङ्क);
  ध्रुव वर्ण *(*filename_database)(ध्रुव वर्ण*);
  ध्रुव वर्ण *(*filename_journal)(ध्रुव वर्ण*);
  ध्रुव वर्ण *(*filename_wal)(ध्रुव वर्ण*);
  /* Version 3.32.0 and later */
  ध्रुव वर्ण *(*create_filename)(ध्रुव वर्ण*,ध्रुव वर्ण*,ध्रुव वर्ण*,
                           पूर्णाङ्क,ध्रुव वर्ण**);
  शून्य (*free_filename)(ध्रुव वर्ण*);
  sqlite3_file *(*database_file_object)(ध्रुव वर्ण*);
  /* Version 3.34.0 and later */
  पूर्णाङ्क (*txn_state)(sqlite3*,ध्रुव वर्ण*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*योग_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  पूर्णाङ्क (*autovacuum_पृष्ठानि)(sqlite3*,
     अचिह्नित पूर्णाङ्क(*)(शून्य*,ध्रुव वर्ण*,अचिह्नित पूर्णाङ्क,अचिह्नित पूर्णाङ्क,अचिह्नित पूर्णाङ्क),
     शून्य*, शून्य(*)(शून्य*));
  /* Version 3.38.0 and later */
  पूर्णाङ्क (*error_विस्थापनम्)(sqlite3*);
  पूर्णाङ्क (*vtab_rhs_मानम्)(sqlite3_सूचकांक_info*,पूर्णाङ्क,sqlite3_मानम्**);
  पूर्णाङ्क (*vtab_distinct)(sqlite3_सूचकांक_info*);
  पूर्णाङ्क (*vtab_अन्तर्)(sqlite3_सूचकांक_info*,पूर्णाङ्क,पूर्णाङ्क);
  पूर्णाङ्क (*vtab_अन्तर्_first)(sqlite3_मानम्*,sqlite3_मानम्**);
  पूर्णाङ्क (*vtab_अन्तर्_अग्र)(sqlite3_मानम्*,sqlite3_मानम्**);
  /* Version 3.39.0 and later */
  पूर्णाङ्क (*deserialize)(sqlite3*,ध्रुव वर्ण*,अचिह्नित वर्ण*,
                     sqlite3_int64,sqlite3_int64,अचिह्नित);
  अचिह्नित वर्ण *(*serialize)(sqlite3*,ध्रुव वर्ण *,sqlite3_int64*,
                              अचिह्नित पूर्णाङ्क);
  ध्रुव वर्ण *(*db_name)(sqlite3*,पूर्णाङ्क);
  /* Version 3.40.0 and later */
  पूर्णाङ्क (*मानम्_encoding)(sqlite3_मानम्*);
  /* Version 3.41.0 and later */
  पूर्णाङ्क (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  पूर्णाङ्क (*stmt_explain)(sqlite3_stmt*,पूर्णाङ्क);
  /* Version 3.44.0 and later */
  शून्य *(*get_clientdata)(sqlite3*,ध्रुव वर्ण*);
  पूर्णाङ्क (*समुच्चयः_clientdata)(sqlite3*, ध्रुव वर्ण*, शून्य*, शून्य(*)(शून्य*));
  /* Version 3.50.0 and later */
  पूर्णाङ्क (*setlk_timeout)(sqlite3*,पूर्णाङ्क,पूर्णाङ्क);
  /* Version 3.51.0 and later */
  पूर्णाङ्क (*समुच्चयः_errmsg)(sqlite3*,पूर्णाङ्क,ध्रुव वर्ण*);
  पूर्णाङ्क (*db_status64)(sqlite3*,पूर्णाङ्क,sqlite3_int64*,sqlite3_int64*,पूर्णाङ्क);
  /* Version 3.52.0 and later */
  शून्य (*str_truncate)(sqlite3_str*,पूर्णाङ्क);
  शून्य (*str_free)(sqlite3_str*);
  पूर्णाङ्क (*carray_bind)(sqlite3_stmt*,पूर्णाङ्क,शून्य*,पूर्णाङ्क,पूर्णाङ्क,शून्य(*)(शून्य*));
  पूर्णाङ्क (*carray_bind_v2)(sqlite3_stmt*,पूर्णाङ्क,शून्य*,पूर्णाङ्क,पूर्णाङ्क,शून्य(*)(शून्य*),शून्य*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ध्रुव वर्ण*);
  शून्य (*परिणाम_str)(sqlite3_context*,sqlite3_str*,पूर्णाङ्क);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
प्रकारनिर्देश पूर्णाङ्क (*sqlite3_loadext_प्रविष्टिः)(
  sqlite3 *db,                       /* Handle to the database. */
  वर्ण **pz_Err_सन्देशः,                   /* Used to set error string on failure. */
  ध्रुव sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#यदि !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_आरोपय_EXTENSION)
#परिभाषय sqlite3_aggregate_context      sqlite3_api->aggregate_context
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
#परिभाषय sqlite3_aggregate_गणनम्        sqlite3_api->aggregate_गणनम्
#यदि_अन्तः
#परिभाषय sqlite3_bind_blob              sqlite3_api->bind_blob
#परिभाषय sqlite3_bind_double            sqlite3_api->bind_double
#परिभाषय sqlite3_bind_int               sqlite3_api->bind_int
#परिभाषय sqlite3_bind_int64             sqlite3_api->bind_int64
#परिभाषय sqlite3_bind_null              sqlite3_api->bind_null
#परिभाषय sqlite3_bind_parameter_गणनम्   sqlite3_api->bind_parameter_गणनम्
#परिभाषय sqlite3_bind_parameter_सूचकांक   sqlite3_api->bind_parameter_सूचकांक
#परिभाषय sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#परिभाषय sqlite3_bind_text              sqlite3_api->bind_text
#परिभाषय sqlite3_bind_text16            sqlite3_api->bind_text16
#परिभाषय sqlite3_bind_मानम्             sqlite3_api->bind_मानम्
#परिभाषय sqlite3_busy_handler           sqlite3_api->busy_handler
#परिभाषय sqlite3_busy_timeout           sqlite3_api->busy_timeout
#परिभाषय sqlite3_changes                sqlite3_api->changes
#परिभाषय sqlite3_close                  sqlite3_api->close
#परिभाषय sqlite3_collation_needed       sqlite3_api->collation_needed
#परिभाषय sqlite3_collation_needed16     sqlite3_api->collation_needed16
#परिभाषय sqlite3_column_blob            sqlite3_api->column_blob
#परिभाषय sqlite3_column_अष्टकानि           sqlite3_api->column_अष्टकानि
#परिभाषय sqlite3_column_bytes16         sqlite3_api->column_bytes16
#परिभाषय sqlite3_column_गणनम्           sqlite3_api->column_गणनम्
#परिभाषय sqlite3_column_database_name   sqlite3_api->column_database_name
#परिभाषय sqlite3_column_database_name16 sqlite3_api->column_database_name16
#परिभाषय sqlite3_column_decltype        sqlite3_api->column_decltype
#परिभाषय sqlite3_column_decltype16      sqlite3_api->column_decltype16
#परिभाषय sqlite3_column_double          sqlite3_api->column_double
#परिभाषय sqlite3_column_int             sqlite3_api->column_int
#परिभाषय sqlite3_column_int64           sqlite3_api->column_int64
#परिभाषय sqlite3_column_name            sqlite3_api->column_name
#परिभाषय sqlite3_column_name16          sqlite3_api->column_name16
#परिभाषय sqlite3_column_origin_name     sqlite3_api->column_origin_name
#परिभाषय sqlite3_column_origin_name16   sqlite3_api->column_origin_name16
#परिभाषय sqlite3_column_सारणी_name      sqlite3_api->column_सारणी_name
#परिभाषय sqlite3_column_सारणी_name16    sqlite3_api->column_सारणी_name16
#परिभाषय sqlite3_column_text            sqlite3_api->column_text
#परिभाषय sqlite3_column_text16          sqlite3_api->column_text16
#परिभाषय sqlite3_column_type            sqlite3_api->column_type
#परिभाषय sqlite3_column_मानम्           sqlite3_api->column_मानम्
#परिभाषय sqlite3_commit_hook            sqlite3_api->commit_hook
#परिभाषय sqlite3_complete               sqlite3_api->complete
#परिभाषय sqlite3_complete16             sqlite3_api->complete16
#परिभाषय sqlite3_create_collation       sqlite3_api->create_collation
#परिभाषय sqlite3_create_collation16     sqlite3_api->create_collation16
#परिभाषय sqlite3_create_function        sqlite3_api->create_function
#परिभाषय sqlite3_create_function16      sqlite3_api->create_function16
#परिभाषय sqlite3_create_module          sqlite3_api->create_module
#परिभाषय sqlite3_create_module_v2       sqlite3_api->create_module_v2
#परिभाषय sqlite3_दत्त_गणनम्             sqlite3_api->दत्त_गणनम्
#परिभाषय sqlite3_db_handle              sqlite3_api->db_handle
#परिभाषय sqlite3_declare_vtab           sqlite3_api->declare_vtab
#परिभाषय sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#परिभाषय sqlite3_errcode                sqlite3_api->errcode
#परिभाषय sqlite3_errmsg                 sqlite3_api->errmsg
#परिभाषय sqlite3_errmsg16               sqlite3_api->errmsg16
#परिभाषय sqlite3_exec                   sqlite3_api->exec
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
#परिभाषय sqlite3_expired                sqlite3_api->expired
#यदि_अन्तः
#परिभाषय sqlite3_finalize               sqlite3_api->finalize
#परिभाषय sqlite3_free                   sqlite3_api->मुक्त
#परिभाषय sqlite3_free_सारणी             sqlite3_api->free_सारणी
#परिभाषय sqlite3_get_autocommit         sqlite3_api->get_autocommit
#परिभाषय sqlite3_get_auxdata            sqlite3_api->get_auxdata
#परिभाषय sqlite3_get_सारणी              sqlite3_api->get_सारणी
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
#परिभाषय sqlite3_global_recover         sqlite3_api->global_recover
#यदि_अन्तः
#परिभाषय sqlite3_व्यवधानम्              sqlite3_api->interruptx
#परिभाषय sqlite3_चरमः_निवेशय_rowid      sqlite3_api->चरमः_निवेशय_rowid
#परिभाषय sqlite3_libversion             sqlite3_api->libversion
#परिभाषय sqlite3_libversion_सङ्ख्या      sqlite3_api->libversion_सङ्ख्या
#परिभाषय sqlite3_malloc                 sqlite3_api->स्मृतिं_देहि
#परिभाषय sqlite3_mprintf                sqlite3_api->mprintf
#परिभाषय sqlite3_open                   sqlite3_api->उद्घाटय
#परिभाषय sqlite3_open16                 sqlite3_api->open16
#परिभाषय sqlite3_prepare                sqlite3_api->prepare
#परिभाषय sqlite3_prepare16              sqlite3_api->prepare16
#परिभाषय sqlite3_prepare_v2             sqlite3_api->prepare_v2
#परिभाषय sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#परिभाषय sqlite3_profile                sqlite3_api->profile
#परिभाषय sqlite3_प्रगतिः_handler       sqlite3_api->प्रगतिः_handler
#परिभाषय sqlite3_realloc                sqlite3_api->realloc
#परिभाषय sqlite3_reset                  sqlite3_api->reset
#परिभाषय sqlite3_परिणाम_blob            sqlite3_api->परिणाम_blob
#परिभाषय sqlite3_परिणाम_double          sqlite3_api->परिणाम_double
#परिभाषय sqlite3_परिणाम_error           sqlite3_api->परिणाम_error
#परिभाषय sqlite3_परिणाम_error16         sqlite3_api->परिणाम_error16
#परिभाषय sqlite3_परिणाम_int             sqlite3_api->परिणाम_int
#परिभाषय sqlite3_परिणाम_int64           sqlite3_api->परिणाम_int64
#परिभाषय sqlite3_परिणाम_null            sqlite3_api->परिणाम_null
#परिभाषय sqlite3_परिणाम_text            sqlite3_api->परिणाम_text
#परिभाषय sqlite3_परिणाम_text16          sqlite3_api->परिणाम_text16
#परिभाषय sqlite3_परिणाम_text16be        sqlite3_api->परिणाम_text16be
#परिभाषय sqlite3_परिणाम_text16le        sqlite3_api->परिणाम_text16le
#परिभाषय sqlite3_परिणाम_मानम्           sqlite3_api->परिणाम_मानम्
#परिभाषय sqlite3_rollback_hook          sqlite3_api->rollback_hook
#परिभाषय sqlite3_समुच्चयः_authorizer         sqlite3_api->समुच्चयः_authorizer
#परिभाषय sqlite3_समुच्चयः_auxdata            sqlite3_api->समुच्चयः_auxdata
#परिभाषय sqlite3_snprintf               sqlite3_api->xsnprintf
#परिभाषय sqlite3_चरणः                   sqlite3_api->चरणः
#परिभाषय sqlite3_सारणी_column_metadata  sqlite3_api->सारणी_column_metadata
#परिभाषय sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#परिभाषय sqlite3_योग_changes          sqlite3_api->योग_changes
#परिभाषय sqlite3_trace                  sqlite3_api->trace
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
#परिभाषय sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#यदि_अन्तः
#परिभाषय sqlite3_update_hook            sqlite3_api->update_hook
#परिभाषय sqlite3_user_दत्त              sqlite3_api->user_दत्त
#परिभाषय sqlite3_मानम्_blob             sqlite3_api->मानम्_blob
#परिभाषय sqlite3_मानम्_अष्टकानि            sqlite3_api->मानम्_अष्टकानि
#परिभाषय sqlite3_मानम्_bytes16          sqlite3_api->मानम्_bytes16
#परिभाषय sqlite3_मानम्_double           sqlite3_api->मानम्_double
#परिभाषय sqlite3_मानम्_int              sqlite3_api->मानम्_int
#परिभाषय sqlite3_मानम्_int64            sqlite3_api->मानम्_int64
#परिभाषय sqlite3_मानम्_numeric_type     sqlite3_api->मानम्_numeric_type
#परिभाषय sqlite3_मानम्_text             sqlite3_api->मानम्_text
#परिभाषय sqlite3_मानम्_text16           sqlite3_api->मानम्_text16
#परिभाषय sqlite3_मानम्_text16be         sqlite3_api->मानम्_text16be
#परिभाषय sqlite3_मानम्_text16le         sqlite3_api->मानम्_text16le
#परिभाषय sqlite3_मानम्_type             sqlite3_api->मानम्_type
#परिभाषय sqlite3_vmprintf               sqlite3_api->vmprintf
#परिभाषय sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#परिभाषय sqlite3_overload_function      sqlite3_api->overload_function
#परिभाषय sqlite3_prepare_v2             sqlite3_api->prepare_v2
#परिभाषय sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#परिभाषय sqlite3_clear_bindings         sqlite3_api->clear_bindings
#परिभाषय sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#परिभाषय sqlite3_blob_अष्टकानि             sqlite3_api->blob_अष्टकानि
#परिभाषय sqlite3_blob_close             sqlite3_api->blob_close
#परिभाषय sqlite3_blob_open              sqlite3_api->blob_open
#परिभाषय sqlite3_blob_read              sqlite3_api->blob_read
#परिभाषय sqlite3_blob_write             sqlite3_api->blob_write
#परिभाषय sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#परिभाषय sqlite3_file_control           sqlite3_api->file_control
#परिभाषय sqlite3_स्मृतिः_highwater       sqlite3_api->स्मृतिः_highwater
#परिभाषय sqlite3_स्मृतिः_प्रयुक्तम्            sqlite3_api->स्मृतिः_प्रयुक्तम्
#परिभाषय sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#परिभाषय sqlite3_mutex_enter            sqlite3_api->mutex_enter
#परिभाषय sqlite3_mutex_free             sqlite3_api->mutex_free
#परिभाषय sqlite3_mutex_leave            sqlite3_api->mutex_leave
#परिभाषय sqlite3_mutex_try              sqlite3_api->mutex_try
#परिभाषय sqlite3_open_v2                sqlite3_api->open_v2
#परिभाषय sqlite3_release_स्मृतिः         sqlite3_api->release_स्मृतिः
#परिभाषय sqlite3_परिणाम_error_nomem     sqlite3_api->परिणाम_error_nomem
#परिभाषय sqlite3_परिणाम_error_toobig    sqlite3_api->परिणाम_error_toobig
#परिभाषय sqlite3_sleep                  sqlite3_api->sleep
#परिभाषय sqlite3_soft_heap_सीमा        sqlite3_api->soft_heap_सीमा
#परिभाषय sqlite3_vfs_find               sqlite3_api->vfs_find
#परिभाषय sqlite3_vfs_पञ्जिकाकोशः           sqlite3_api->vfs_पञ्जिकाकोशः
#परिभाषय sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#परिभाषय sqlite3_threadsafe             sqlite3_api->xthreadsafe
#परिभाषय sqlite3_परिणाम_zeroblob        sqlite3_api->परिणाम_zeroblob
#परिभाषय sqlite3_परिणाम_error_संहिता      sqlite3_api->परिणाम_error_संहिता
#परिभाषय sqlite3_test_control           sqlite3_api->test_control
#परिभाषय sqlite3_randomness             sqlite3_api->randomness
#परिभाषय sqlite3_context_db_handle      sqlite3_api->context_db_handle
#परिभाषय sqlite3_extended_परिणाम_codes  sqlite3_api->extended_परिणाम_codes
#परिभाषय sqlite3_सीमा                  sqlite3_api->सीमा
#परिभाषय sqlite3_अग्र_stmt              sqlite3_api->अग्र_stmt
#परिभाषय sqlite3_sql                    sqlite3_api->sql
#परिभाषय sqlite3_स्थितिः                 sqlite3_api->स्थितिः
#परिभाषय sqlite3_backup_finish          sqlite3_api->backup_finish
#परिभाषय sqlite3_backup_init            sqlite3_api->backup_init
#परिभाषय sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#परिभाषय sqlite3_backup_remaining       sqlite3_api->backup_remaining
#परिभाषय sqlite3_backup_चरणः            sqlite3_api->backup_चरणः
#परिभाषय sqlite3_compileoption_get      sqlite3_api->compileoption_get
#परिभाषय sqlite3_compileoption_प्रयुक्तम्     sqlite3_api->compileoption_प्रयुक्तम्
#परिभाषय sqlite3_create_function_v2     sqlite3_api->create_function_v2
#परिभाषय sqlite3_db_config              sqlite3_api->db_config
#परिभाषय sqlite3_db_mutex               sqlite3_api->db_mutex
#परिभाषय sqlite3_db_स्थितिः              sqlite3_api->db_स्थितिः
#परिभाषय sqlite3_extended_errcode       sqlite3_api->extended_errcode
#परिभाषय sqlite3_log                    sqlite3_api->लघुगणकः
#परिभाषय sqlite3_soft_heap_limit64      sqlite3_api->soft_heap_limit64
#परिभाषय sqlite3_sourceid               sqlite3_api->sourceid
#परिभाषय sqlite3_stmt_स्थितिः            sqlite3_api->stmt_स्थितिः
#परिभाषय sqlite3_strnicmp               sqlite3_api->strnicmp
#परिभाषय sqlite3_unlock_notify          sqlite3_api->unlock_notify
#परिभाषय sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#परिभाषय sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#परिभाषय sqlite3_wal_hook               sqlite3_api->wal_hook
#परिभाषय sqlite3_blob_reopen            sqlite3_api->blob_reopen
#परिभाषय sqlite3_vtab_config            sqlite3_api->vtab_config
#परिभाषय sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#परिभाषय sqlite3_close_v2               sqlite3_api->close_v2
#परिभाषय sqlite3_db_filename            sqlite3_api->db_filename
#परिभाषय sqlite3_db_readonly            sqlite3_api->db_readonly
#परिभाषय sqlite3_db_release_स्मृतिः      sqlite3_api->db_release_स्मृतिः
#परिभाषय sqlite3_errstr                 sqlite3_api->errstr
#परिभाषय sqlite3_stmt_busy              sqlite3_api->stmt_busy
#परिभाषय sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#परिभाषय sqlite3_stricmp                sqlite3_api->stricmp
#परिभाषय sqlite3_uri_boolean            sqlite3_api->uri_boolean
#परिभाषय sqlite3_uri_int64              sqlite3_api->uri_int64
#परिभाषय sqlite3_uri_parameter          sqlite3_api->uri_parameter
#परिभाषय sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#परिभाषय sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#परिभाषय sqlite3_auto_extension         sqlite3_api->auto_extension
#परिभाषय sqlite3_bind_blob64            sqlite3_api->bind_blob64
#परिभाषय sqlite3_bind_text64            sqlite3_api->bind_text64
#परिभाषय sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#परिभाषय sqlite3_आरोपय_extension         sqlite3_api->आरोपय_extension
#परिभाषय sqlite3_malloc64               sqlite3_api->malloc64
#परिभाषय sqlite3_msize                  sqlite3_api->msize
#परिभाषय sqlite3_realloc64              sqlite3_api->realloc64
#परिभाषय sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#परिभाषय sqlite3_परिणाम_blob64          sqlite3_api->परिणाम_blob64
#परिभाषय sqlite3_परिणाम_text64          sqlite3_api->परिणाम_text64
#परिभाषय sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#परिभाषय sqlite3_मानम्_dup              sqlite3_api->मानम्_dup
#परिभाषय sqlite3_मानम्_free             sqlite3_api->मानम्_free
#परिभाषय sqlite3_परिणाम_zeroblob64      sqlite3_api->परिणाम_zeroblob64
#परिभाषय sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#परिभाषय sqlite3_मानम्_subtype          sqlite3_api->मानम्_subtype
#परिभाषय sqlite3_परिणाम_subtype         sqlite3_api->परिणाम_subtype
/* Version 3.10.0 and later */
#परिभाषय sqlite3_status64               sqlite3_api->status64
#परिभाषय sqlite3_strlike                sqlite3_api->strlike
#परिभाषय sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#परिभाषय sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#परिभाषय sqlite3_trace_v2               sqlite3_api->trace_v2
#परिभाषय sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#परिभाषय sqlite3_समुच्चयः_चरमः_निवेशय_rowid  sqlite3_api->समुच्चयः_चरमः_निवेशय_rowid
/* Version 3.20.0 and later */
#परिभाषय sqlite3_prepare_v3             sqlite3_api->prepare_v3
#परिभाषय sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#परिभाषय sqlite3_bind_pointer           sqlite3_api->bind_pointer
#परिभाषय sqlite3_परिणाम_pointer         sqlite3_api->परिणाम_pointer
#परिभाषय sqlite3_मानम्_pointer          sqlite3_api->मानम्_pointer
/* Version 3.22.0 and later */
#परिभाषय sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#परिभाषय sqlite3_मानम्_nochange         sqlite3_api->मानम्_nochange
#परिभाषय sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#परिभाषय sqlite3_keyword_गणनम्          sqlite3_api->keyword_गणनम्
#परिभाषय sqlite3_keyword_name           sqlite3_api->keyword_name
#परिभाषय sqlite3_keyword_check          sqlite3_api->keyword_check
#परिभाषय sqlite3_str_new                sqlite3_api->str_new
#परिभाषय sqlite3_str_finish             sqlite3_api->str_finish
#परिभाषय sqlite3_str_appendf            sqlite3_api->str_appendf
#परिभाषय sqlite3_str_vappendf           sqlite3_api->str_vappendf
#परिभाषय sqlite3_str_append             sqlite3_api->str_append
#परिभाषय sqlite3_str_appendall          sqlite3_api->str_appendall
#परिभाषय sqlite3_str_appendchar         sqlite3_api->str_appendchar
#परिभाषय sqlite3_str_reset              sqlite3_api->str_reset
#परिभाषय sqlite3_str_errcode            sqlite3_api->str_errcode
#परिभाषय sqlite3_str_दीर्घता             sqlite3_api->str_दीर्घता
#परिभाषय sqlite3_str_मानम्              sqlite3_api->str_मानम्
/* Version 3.25.0 and later */
#परिभाषय sqlite3_create_गवाक्षः_function sqlite3_api->create_गवाक्षः_function
/* Version 3.26.0 and later */
#परिभाषय sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#परिभाषय sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#परिभाषय sqlite3_मानम्_frombind         sqlite3_api->मानम्_frombind
/* Version 3.30.0 and later */
#परिभाषय sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#परिभाषय sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#परिभाषय sqlite3_uri_कुञ्चिका                sqlite3_api->uri_कुञ्चिका
#परिभाषय sqlite3_filename_database      sqlite3_api->filename_database
#परिभाषय sqlite3_filename_journal       sqlite3_api->filename_journal
#परिभाषय sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#परिभाषय sqlite3_create_filename        sqlite3_api->create_filename
#परिभाषय sqlite3_free_filename          sqlite3_api->free_filename
#परिभाषय sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#परिभाषय sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#परिभाषय sqlite3_changes64              sqlite3_api->changes64
#परिभाषय sqlite3_योग_changes64        sqlite3_api->योग_changes64
/* Version 3.37.0 and later */
#परिभाषय sqlite3_autovacuum_पृष्ठानि       sqlite3_api->autovacuum_पृष्ठानि
/* Version 3.38.0 and later */
#परिभाषय sqlite3_error_विस्थापनम्           sqlite3_api->error_विस्थापनम्
#परिभाषय sqlite3_vtab_rhs_मानम्         sqlite3_api->vtab_rhs_मानम्
#परिभाषय sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#परिभाषय sqlite3_vtab_अन्तर्                sqlite3_api->vtab_अन्तर्
#परिभाषय sqlite3_vtab_अन्तर्_first          sqlite3_api->vtab_अन्तर्_first
#परिभाषय sqlite3_vtab_अन्तर्_अग्र           sqlite3_api->vtab_अन्तर्_अग्र
/* Version 3.39.0 and later */
#यदि_अपरिभाषितम् SQLITE_OMIT_DESERIALIZE
#परिभाषय sqlite3_deserialize            sqlite3_api->deserialize
#परिभाषय sqlite3_serialize              sqlite3_api->serialize
#यदि_अन्तः
#परिभाषय sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#परिभाषय sqlite3_मानम्_encoding         sqlite3_api->मानम्_encoding
/* Version 3.41.0 and later */
#परिभाषय sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#परिभाषय sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#परिभाषय sqlite3_get_clientdata         sqlite3_api->get_clientdata
#परिभाषय sqlite3_समुच्चयः_clientdata         sqlite3_api->समुच्चयः_clientdata
/* Version 3.50.0 and later */
#परिभाषय sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#परिभाषय sqlite3_समुच्चयः_errmsg             sqlite3_api->समुच्चयः_errmsg
#परिभाषय sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#परिभाषय sqlite3_str_truncate           sqlite3_api->str_truncate
#परिभाषय sqlite3_str_free               sqlite3_api->str_free
#परिभाषय sqlite3_carray_bind            sqlite3_api->carray_bind
#परिभाषय sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#परिभाषय sqlite3_incomplete             sqlite3_api->incomplete
#परिभाषय sqlite3_परिणाम_str             sqlite3_api->परिणाम_str
#यदि_अन्तः /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#यदि !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_आरोपय_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ध्रुव sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    बाह्य ध्रुव sqlite3_api_routines *sqlite3_api;
#अन्यथा
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (शून्य)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#यदि_अन्तः

#यदि_अन्तः /* SQLITE3EXT_H */
