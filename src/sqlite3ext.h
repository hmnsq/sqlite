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
#अगर_अघोषित SQLITE3EXT_H
#घोषणा SQLITE3EXT_H
#समावेश "sqlite3.h"

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
काष्ठा sqlite3_api_routines {
  व्योम * (*aggregate_context)(sqlite3_context*,पूर्णांक nBytes);
  पूर्णांक  (*aggregate_गिनती)(sqlite3_context*);
  पूर्णांक  (*bind_blob)(sqlite3_stmt*,पूर्णांक,स्थिर व्योम*,पूर्णांक n,व्योम(*)(व्योम*));
  पूर्णांक  (*bind_double)(sqlite3_stmt*,पूर्णांक,द्विगुन);
  पूर्णांक  (*bind_int)(sqlite3_stmt*,पूर्णांक,पूर्णांक);
  पूर्णांक  (*bind_int64)(sqlite3_stmt*,पूर्णांक,sqlite_int64);
  पूर्णांक  (*bind_null)(sqlite3_stmt*,पूर्णांक);
  पूर्णांक  (*bind_parameter_गिनती)(sqlite3_stmt*);
  पूर्णांक  (*bind_parameter_सूचकांक)(sqlite3_stmt*,स्थिर अक्षर*zName);
  स्थिर अक्षर * (*bind_parameter_name)(sqlite3_stmt*,पूर्णांक);
  पूर्णांक  (*bind_text)(sqlite3_stmt*,पूर्णांक,स्थिर अक्षर*,पूर्णांक n,व्योम(*)(व्योम*));
  पूर्णांक  (*bind_text16)(sqlite3_stmt*,पूर्णांक,स्थिर व्योम*,पूर्णांक,व्योम(*)(व्योम*));
  पूर्णांक  (*bind_मूल्य)(sqlite3_stmt*,पूर्णांक,स्थिर sqlite3_मूल्य*);
  पूर्णांक  (*busy_handler)(sqlite3*,पूर्णांक(*)(व्योम*,पूर्णांक),व्योम*);
  पूर्णांक  (*busy_timeout)(sqlite3*,पूर्णांक ms);
  पूर्णांक  (*changes)(sqlite3*);
  पूर्णांक  (*बंद)(sqlite3*);
  पूर्णांक  (*collation_needed)(sqlite3*,व्योम*,व्योम(*)(व्योम*,sqlite3*,
                           पूर्णांक eTextRep,स्थिर अक्षर*));
  पूर्णांक  (*collation_needed16)(sqlite3*,व्योम*,व्योम(*)(व्योम*,sqlite3*,
                             पूर्णांक eTextRep,स्थिर व्योम*));
  स्थिर व्योम * (*column_blob)(sqlite3_stmt*,पूर्णांक iCol);
  पूर्णांक  (*column_bytes)(sqlite3_stmt*,पूर्णांक iCol);
  पूर्णांक  (*column_bytes16)(sqlite3_stmt*,पूर्णांक iCol);
  पूर्णांक  (*column_गिनती)(sqlite3_stmt*pStmt);
  स्थिर अक्षर * (*column_database_name)(sqlite3_stmt*,पूर्णांक);
  स्थिर व्योम * (*column_database_name16)(sqlite3_stmt*,पूर्णांक);
  स्थिर अक्षर * (*column_decltype)(sqlite3_stmt*,पूर्णांक i);
  स्थिर व्योम * (*column_decltype16)(sqlite3_stmt*,पूर्णांक);
  द्विगुन  (*column_double)(sqlite3_stmt*,पूर्णांक iCol);
  पूर्णांक  (*column_int)(sqlite3_stmt*,पूर्णांक iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,पूर्णांक iCol);
  स्थिर अक्षर * (*column_name)(sqlite3_stmt*,पूर्णांक);
  स्थिर व्योम * (*column_name16)(sqlite3_stmt*,पूर्णांक);
  स्थिर अक्षर * (*column_origin_name)(sqlite3_stmt*,पूर्णांक);
  स्थिर व्योम * (*column_origin_name16)(sqlite3_stmt*,पूर्णांक);
  स्थिर अक्षर * (*column_table_name)(sqlite3_stmt*,पूर्णांक);
  स्थिर व्योम * (*column_table_name16)(sqlite3_stmt*,पूर्णांक);
  स्थिर अचिन्हित अक्षर * (*column_text)(sqlite3_stmt*,पूर्णांक iCol);
  स्थिर व्योम * (*column_text16)(sqlite3_stmt*,पूर्णांक iCol);
  पूर्णांक  (*column_type)(sqlite3_stmt*,पूर्णांक iCol);
  sqlite3_मूल्य* (*column_मूल्य)(sqlite3_stmt*,पूर्णांक iCol);
  व्योम * (*commit_hook)(sqlite3*,पूर्णांक(*)(व्योम*),व्योम*);
  पूर्णांक  (*complete)(स्थिर अक्षर*sql);
  पूर्णांक  (*complete16)(स्थिर व्योम*sql);
  पूर्णांक  (*create_collation)(sqlite3*,स्थिर अक्षर*,पूर्णांक,व्योम*,
                           पूर्णांक(*)(व्योम*,पूर्णांक,स्थिर व्योम*,पूर्णांक,स्थिर व्योम*));
  पूर्णांक  (*create_collation16)(sqlite3*,स्थिर व्योम*,पूर्णांक,व्योम*,
                             पूर्णांक(*)(व्योम*,पूर्णांक,स्थिर व्योम*,पूर्णांक,स्थिर व्योम*));
  पूर्णांक  (*create_function)(sqlite3*,स्थिर अक्षर*,पूर्णांक,पूर्णांक,व्योम*,
                          व्योम (*xFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
                          व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
                          व्योम (*xFinal)(sqlite3_context*));
  पूर्णांक  (*create_function16)(sqlite3*,स्थिर व्योम*,पूर्णांक,पूर्णांक,व्योम*,
                            व्योम (*xFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
                            व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
                            व्योम (*xFinal)(sqlite3_context*));
  पूर्णांक (*create_module)(sqlite3*,स्थिर अक्षर*,स्थिर sqlite3_module*,व्योम*);
  पूर्णांक  (*तथ्य_गिनती)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  पूर्णांक (*declare_vtab)(sqlite3*,स्थिर अक्षर*);
  पूर्णांक  (*enable_shared_cache)(पूर्णांक);
  पूर्णांक  (*errcode)(sqlite3*db);
  स्थिर अक्षर * (*errmsg)(sqlite3*);
  स्थिर व्योम * (*errmsg16)(sqlite3*);
  पूर्णांक  (*exec)(sqlite3*,स्थिर अक्षर*,sqlite3_callback,व्योम*,अक्षर**);
  पूर्णांक  (*expired)(sqlite3_stmt*);
  पूर्णांक  (*finalize)(sqlite3_stmt*pStmt);
  व्योम  (*मुक्त)(व्योम*);
  व्योम  (*free_table)(अक्षर**परिणाम);
  पूर्णांक  (*get_autocommit)(sqlite3*);
  व्योम * (*get_auxdata)(sqlite3_context*,पूर्णांक);
  पूर्णांक  (*get_table)(sqlite3*,स्थिर अक्षर*,अक्षर***,पूर्णांक*,पूर्णांक*,अक्षर**);
  पूर्णांक  (*global_recover)(व्योम);
  व्योम  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_जोड़ो_rowid)(sqlite3*);
  स्थिर अक्षर * (*libversion)(व्योम);
  पूर्णांक  (*libversion_संख्या)(व्योम);
  व्योम *(*दो_स्मृति)(पूर्णांक);
  अक्षर *(SQLITE_CDECL*mprintf)(स्थिर अक्षर*,...);
  पूर्णांक  (*खोलो)(स्थिर अक्षर*,sqlite3**);
  पूर्णांक  (*open16)(स्थिर व्योम*,sqlite3**);
  पूर्णांक  (*prepare)(sqlite3*,स्थिर अक्षर*,पूर्णांक,sqlite3_stmt**,स्थिर अक्षर**);
  पूर्णांक  (*prepare16)(sqlite3*,स्थिर व्योम*,पूर्णांक,sqlite3_stmt**,स्थिर व्योम**);
  व्योम * (*profile)(sqlite3*,व्योम(*)(व्योम*,स्थिर अक्षर*,sqlite_uint64),व्योम*);
  व्योम  (*progress_handler)(sqlite3*,पूर्णांक,पूर्णांक(*)(व्योम*),व्योम*);
  व्योम *(*पुनः_स्मृति)(व्योम*,पूर्णांक);
  पूर्णांक  (*reset)(sqlite3_stmt*pStmt);
  व्योम  (*परिणाम_blob)(sqlite3_context*,स्थिर व्योम*,पूर्णांक,व्योम(*)(व्योम*));
  व्योम  (*परिणाम_double)(sqlite3_context*,द्विगुन);
  व्योम  (*परिणाम_error)(sqlite3_context*,स्थिर अक्षर*,पूर्णांक);
  व्योम  (*परिणाम_error16)(sqlite3_context*,स्थिर व्योम*,पूर्णांक);
  व्योम  (*परिणाम_int)(sqlite3_context*,पूर्णांक);
  व्योम  (*परिणाम_int64)(sqlite3_context*,sqlite_int64);
  व्योम  (*परिणाम_null)(sqlite3_context*);
  व्योम  (*परिणाम_text)(sqlite3_context*,स्थिर अक्षर*,पूर्णांक,व्योम(*)(व्योम*));
  व्योम  (*परिणाम_text16)(sqlite3_context*,स्थिर व्योम*,पूर्णांक,व्योम(*)(व्योम*));
  व्योम  (*परिणाम_text16be)(sqlite3_context*,स्थिर व्योम*,पूर्णांक,व्योम(*)(व्योम*));
  व्योम  (*परिणाम_text16le)(sqlite3_context*,स्थिर व्योम*,पूर्णांक,व्योम(*)(व्योम*));
  व्योम  (*परिणाम_मूल्य)(sqlite3_context*,sqlite3_मूल्य*);
  व्योम *(*rollback_hook)(sqlite3*,व्योम(*)(व्योम*),व्योम*);
  पूर्णांक  (*set_authorizer)(sqlite3*,पूर्णांक(*)(व्योम*,पूर्णांक,स्थिर अक्षर*,स्थिर अक्षर*,
                         स्थिर अक्षर*,स्थिर अक्षर*),व्योम*);
  व्योम  (*set_auxdata)(sqlite3_context*,पूर्णांक,व्योम*,व्योम (*)(व्योम*));
  अक्षर *(SQLITE_CDECL*xsnprintf)(पूर्णांक,अक्षर*,स्थिर अक्षर*,...);
  पूर्णांक  (*step)(sqlite3_stmt*);
  पूर्णांक  (*table_column_metadata)(sqlite3*,स्थिर अक्षर*,स्थिर अक्षर*,स्थिर अक्षर*,
                                अक्षर स्थिर**,अक्षर स्थिर**,पूर्णांक*,पूर्णांक*,पूर्णांक*);
  व्योम  (*thread_cleanup)(व्योम);
  पूर्णांक  (*योग_changes)(sqlite3*);
  व्योम * (*trace)(sqlite3*,व्योम(*xTrace)(व्योम*,स्थिर अक्षर*),व्योम*);
  पूर्णांक  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  व्योम * (*update_hook)(sqlite3*,व्योम(*)(व्योम*,पूर्णांक ,अक्षर स्थिर*,अक्षर स्थिर*,
                                         sqlite_int64),व्योम*);
  व्योम * (*user_तथ्य)(sqlite3_context*);
  स्थिर व्योम * (*मूल्य_blob)(sqlite3_मूल्य*);
  पूर्णांक  (*मूल्य_bytes)(sqlite3_मूल्य*);
  पूर्णांक  (*मूल्य_bytes16)(sqlite3_मूल्य*);
  द्विगुन  (*मूल्य_double)(sqlite3_मूल्य*);
  पूर्णांक  (*मूल्य_int)(sqlite3_मूल्य*);
  sqlite_int64  (*मूल्य_int64)(sqlite3_मूल्य*);
  पूर्णांक  (*मूल्य_numeric_type)(sqlite3_मूल्य*);
  स्थिर अचिन्हित अक्षर * (*मूल्य_text)(sqlite3_मूल्य*);
  स्थिर व्योम * (*मूल्य_text16)(sqlite3_मूल्य*);
  स्थिर व्योम * (*मूल्य_text16be)(sqlite3_मूल्य*);
  स्थिर व्योम * (*मूल्य_text16le)(sqlite3_मूल्य*);
  पूर्णांक  (*मूल्य_type)(sqlite3_मूल्य*);
  अक्षर *(*vmprintf)(स्थिर अक्षर*,बहु_सूची);
  /* Added ??? */
  पूर्णांक (*overload_function)(sqlite3*, स्थिर अक्षर *zFuncName, पूर्णांक nArg);
  /* Added by 3.3.13 */
  पूर्णांक (*prepare_v2)(sqlite3*,स्थिर अक्षर*,पूर्णांक,sqlite3_stmt**,स्थिर अक्षर**);
  पूर्णांक (*prepare16_v2)(sqlite3*,स्थिर व्योम*,पूर्णांक,sqlite3_stmt**,स्थिर व्योम**);
  पूर्णांक (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  पूर्णांक (*create_module_v2)(sqlite3*,स्थिर अक्षर*,स्थिर sqlite3_module*,व्योम*,
                          व्योम (*xDestroy)(व्योम *));
  /* Added by 3.5.0 */
  पूर्णांक (*bind_zeroblob)(sqlite3_stmt*,पूर्णांक,पूर्णांक);
  पूर्णांक (*blob_bytes)(sqlite3_blob*);
  पूर्णांक (*blob_close)(sqlite3_blob*);
  पूर्णांक (*blob_open)(sqlite3*,स्थिर अक्षर*,स्थिर अक्षर*,स्थिर अक्षर*,sqlite3_int64,
                   पूर्णांक,sqlite3_blob**);
  पूर्णांक (*blob_read)(sqlite3_blob*,व्योम*,पूर्णांक,पूर्णांक);
  पूर्णांक (*blob_write)(sqlite3_blob*,स्थिर व्योम*,पूर्णांक,पूर्णांक);
  पूर्णांक (*create_collation_v2)(sqlite3*,स्थिर अक्षर*,पूर्णांक,व्योम*,
                             पूर्णांक(*)(व्योम*,पूर्णांक,स्थिर व्योम*,पूर्णांक,स्थिर व्योम*),
                             व्योम(*)(व्योम*));
  पूर्णांक (*file_control)(sqlite3*,स्थिर अक्षर*,पूर्णांक,व्योम*);
  sqlite3_int64 (*memory_highwater)(पूर्णांक);
  sqlite3_int64 (*memory_used)(व्योम);
  sqlite3_mutex *(*mutex_alloc)(पूर्णांक);
  व्योम (*mutex_enter)(sqlite3_mutex*);
  व्योम (*mutex_free)(sqlite3_mutex*);
  व्योम (*mutex_leave)(sqlite3_mutex*);
  पूर्णांक (*mutex_try)(sqlite3_mutex*);
  पूर्णांक (*open_v2)(स्थिर अक्षर*,sqlite3**,पूर्णांक,स्थिर अक्षर*);
  पूर्णांक (*release_memory)(पूर्णांक);
  व्योम (*परिणाम_error_nomem)(sqlite3_context*);
  व्योम (*परिणाम_error_toobig)(sqlite3_context*);
  पूर्णांक (*sleep)(पूर्णांक);
  व्योम (*soft_heap_limit)(पूर्णांक);
  sqlite3_vfs *(*vfs_find)(स्थिर अक्षर*);
  पूर्णांक (*vfs_register)(sqlite3_vfs*,पूर्णांक);
  पूर्णांक (*vfs_unregister)(sqlite3_vfs*);
  पूर्णांक (*xthreadsafe)(व्योम);
  व्योम (*परिणाम_zeroblob)(sqlite3_context*,पूर्णांक);
  व्योम (*परिणाम_error_code)(sqlite3_context*,पूर्णांक);
  पूर्णांक (SQLITE_CDECL*test_control)(पूर्णांक, ...);
  व्योम (*randomness)(पूर्णांक,व्योम*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  पूर्णांक (*extended_परिणाम_codes)(sqlite3*,पूर्णांक);
  पूर्णांक (*limit)(sqlite3*,पूर्णांक,पूर्णांक);
  sqlite3_stmt *(*अग्र_stmt)(sqlite3*,sqlite3_stmt*);
  स्थिर अक्षर *(*sql)(sqlite3_stmt*);
  पूर्णांक (*status)(पूर्णांक,पूर्णांक*,पूर्णांक*,पूर्णांक);
  पूर्णांक (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,स्थिर अक्षर*,sqlite3*,स्थिर अक्षर*);
  पूर्णांक (*backup_pagecount)(sqlite3_backup*);
  पूर्णांक (*backup_remaining)(sqlite3_backup*);
  पूर्णांक (*backup_step)(sqlite3_backup*,पूर्णांक);
  स्थिर अक्षर *(*compileoption_get)(पूर्णांक);
  पूर्णांक (*compileoption_used)(स्थिर अक्षर*);
  पूर्णांक (*create_function_v2)(sqlite3*,स्थिर अक्षर*,पूर्णांक,पूर्णांक,व्योम*,
                            व्योम (*xFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
                            व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
                            व्योम (*xFinal)(sqlite3_context*),
                            व्योम(*xDestroy)(व्योम*));
  पूर्णांक (SQLITE_CDECL*db_config)(sqlite3*,पूर्णांक,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  पूर्णांक (*db_status)(sqlite3*,पूर्णांक,पूर्णांक*,पूर्णांक*,पूर्णांक);
  पूर्णांक (*extended_errcode)(sqlite3*);
  व्योम (SQLITE_CDECL*log)(पूर्णांक,स्थिर अक्षर*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  स्थिर अक्षर *(*sourceid)(व्योम);
  पूर्णांक (*stmt_status)(sqlite3_stmt*,पूर्णांक,पूर्णांक);
  पूर्णांक (*strnicmp)(स्थिर अक्षर*,स्थिर अक्षर*,पूर्णांक);
  पूर्णांक (*unlock_notify)(sqlite3*,व्योम(*)(व्योम**,पूर्णांक),व्योम*);
  पूर्णांक (*wal_autocheckpoint)(sqlite3*,पूर्णांक);
  पूर्णांक (*wal_checkpoint)(sqlite3*,स्थिर अक्षर*);
  व्योम *(*wal_hook)(sqlite3*,पूर्णांक(*)(व्योम*,sqlite3*,स्थिर अक्षर*,पूर्णांक),व्योम*);
  पूर्णांक (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  पूर्णांक (SQLITE_CDECL*vtab_config)(sqlite3*,पूर्णांक op,...);
  पूर्णांक (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  पूर्णांक (*close_v2)(sqlite3*);
  स्थिर अक्षर *(*db_filename)(sqlite3*,स्थिर अक्षर*);
  पूर्णांक (*db_readonly)(sqlite3*,स्थिर अक्षर*);
  पूर्णांक (*db_release_memory)(sqlite3*);
  स्थिर अक्षर *(*errstr)(पूर्णांक);
  पूर्णांक (*stmt_busy)(sqlite3_stmt*);
  पूर्णांक (*stmt_readonly)(sqlite3_stmt*);
  पूर्णांक (*stricmp)(स्थिर अक्षर*,स्थिर अक्षर*);
  पूर्णांक (*uri_boolean)(स्थिर अक्षर*,स्थिर अक्षर*,पूर्णांक);
  sqlite3_int64 (*uri_int64)(स्थिर अक्षर*,स्थिर अक्षर*,sqlite3_int64);
  स्थिर अक्षर *(*uri_parameter)(स्थिर अक्षर*,स्थिर अक्षर*);
  अक्षर *(*xvsnprintf)(पूर्णांक,अक्षर*,स्थिर अक्षर*,बहु_सूची);
  पूर्णांक (*wal_checkpoint_v2)(sqlite3*,स्थिर अक्षर*,पूर्णांक,पूर्णांक*,पूर्णांक*);
  /* Version 3.8.7 and later */
  पूर्णांक (*auto_extension)(व्योम(*)(व्योम));
  पूर्णांक (*bind_blob64)(sqlite3_stmt*,पूर्णांक,स्थिर व्योम*,sqlite3_uint64,
                     व्योम(*)(व्योम*));
  पूर्णांक (*bind_text64)(sqlite3_stmt*,पूर्णांक,स्थिर अक्षर*,sqlite3_uint64,
                      व्योम(*)(व्योम*),अचिन्हित अक्षर);
  पूर्णांक (*cancel_auto_extension)(व्योम(*)(व्योम));
  पूर्णांक (*load_extension)(sqlite3*,स्थिर अक्षर*,स्थिर अक्षर*,अक्षर**);
  व्योम *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(व्योम*);
  व्योम *(*realloc64)(व्योम*,sqlite3_uint64);
  व्योम (*reset_auto_extension)(व्योम);
  व्योम (*परिणाम_blob64)(sqlite3_context*,स्थिर व्योम*,sqlite3_uint64,
                        व्योम(*)(व्योम*));
  व्योम (*परिणाम_text64)(sqlite3_context*,स्थिर अक्षर*,sqlite3_uint64,
                         व्योम(*)(व्योम*), अचिन्हित अक्षर);
  पूर्णांक (*strglob)(स्थिर अक्षर*,स्थिर अक्षर*);
  /* Version 3.8.11 and later */
  sqlite3_मूल्य *(*मूल्य_dup)(स्थिर sqlite3_मूल्य*);
  व्योम (*मूल्य_free)(sqlite3_मूल्य*);
  पूर्णांक (*परिणाम_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  पूर्णांक (*bind_zeroblob64)(sqlite3_stmt*, पूर्णांक, sqlite3_uint64);
  /* Version 3.9.0 and later */
  अचिन्हित पूर्णांक (*मूल्य_subtype)(sqlite3_मूल्य*);
  व्योम (*परिणाम_subtype)(sqlite3_context*,अचिन्हित पूर्णांक);
  /* Version 3.10.0 and later */
  पूर्णांक (*status64)(पूर्णांक,sqlite3_int64*,sqlite3_int64*,पूर्णांक);
  पूर्णांक (*strlike)(स्थिर अक्षर*,स्थिर अक्षर*,अचिन्हित पूर्णांक);
  पूर्णांक (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  पूर्णांक (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  पूर्णांक (*trace_v2)(sqlite3*,अचिन्हित,पूर्णांक(*)(अचिन्हित,व्योम*,व्योम*,व्योम*),व्योम*);
  अक्षर *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  व्योम (*set_last_जोड़ो_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  पूर्णांक (*prepare_v3)(sqlite3*,स्थिर अक्षर*,पूर्णांक,अचिन्हित पूर्णांक,
                    sqlite3_stmt**,स्थिर अक्षर**);
  पूर्णांक (*prepare16_v3)(sqlite3*,स्थिर व्योम*,पूर्णांक,अचिन्हित पूर्णांक,
                      sqlite3_stmt**,स्थिर व्योम**);
  पूर्णांक (*bind_pointer)(sqlite3_stmt*,पूर्णांक,व्योम*,स्थिर अक्षर*,व्योम(*)(व्योम*));
  व्योम (*परिणाम_pointer)(sqlite3_context*,व्योम*,स्थिर अक्षर*,व्योम(*)(व्योम*));
  व्योम *(*मूल्य_pointer)(sqlite3_मूल्य*,स्थिर अक्षर*);
  पूर्णांक (*vtab_nochange)(sqlite3_context*);
  पूर्णांक (*मूल्य_nochange)(sqlite3_मूल्य*);
  स्थिर अक्षर *(*vtab_collation)(sqlite3_सूचकांक_info*,पूर्णांक);
  /* Version 3.24.0 and later */
  पूर्णांक (*keyword_गिनती)(व्योम);
  पूर्णांक (*keyword_name)(पूर्णांक,स्थिर अक्षर**,पूर्णांक*);
  पूर्णांक (*keyword_check)(स्थिर अक्षर*,पूर्णांक);
  sqlite3_str *(*str_new)(sqlite3*);
  अक्षर *(*str_finish)(sqlite3_str*);
  व्योम (SQLITE_CDECL*str_appendf)(sqlite3_str*, स्थिर अक्षर *zFormat, ...);
  व्योम (*str_vappendf)(sqlite3_str*, स्थिर अक्षर *zFormat, बहु_सूची);
  व्योम (*str_append)(sqlite3_str*, स्थिर अक्षर *zIn, पूर्णांक N);
  व्योम (*str_appendall)(sqlite3_str*, स्थिर अक्षर *zIn);
  व्योम (*str_appendchar)(sqlite3_str*, पूर्णांक N, अक्षर C);
  व्योम (*str_reset)(sqlite3_str*);
  पूर्णांक (*str_errcode)(sqlite3_str*);
  पूर्णांक (*str_लंबाई)(sqlite3_str*);
  अक्षर *(*str_मूल्य)(sqlite3_str*);
  /* Version 3.25.0 and later */
  पूर्णांक (*create_window_function)(sqlite3*,स्थिर अक्षर*,पूर्णांक,पूर्णांक,व्योम*,
                            व्योम (*xStep)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
                            व्योम (*xFinal)(sqlite3_context*),
                            व्योम (*x_मूल्य)(sqlite3_context*),
                            व्योम (*xInv)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),
                            व्योम(*xDestroy)(व्योम*));
  /* Version 3.26.0 and later */
  स्थिर अक्षर *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  पूर्णांक (*stmt_isexplain)(sqlite3_stmt*);
  पूर्णांक (*मूल्य_frombind)(sqlite3_मूल्य*);
  /* Version 3.30.0 and later */
  पूर्णांक (*drop_modules)(sqlite3*,स्थिर अक्षर**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  स्थिर अक्षर *(*uri_कुंजी)(स्थिर अक्षर*,पूर्णांक);
  स्थिर अक्षर *(*filename_database)(स्थिर अक्षर*);
  स्थिर अक्षर *(*filename_journal)(स्थिर अक्षर*);
  स्थिर अक्षर *(*filename_wal)(स्थिर अक्षर*);
  /* Version 3.32.0 and later */
  स्थिर अक्षर *(*create_filename)(स्थिर अक्षर*,स्थिर अक्षर*,स्थिर अक्षर*,
                           पूर्णांक,स्थिर अक्षर**);
  व्योम (*free_filename)(स्थिर अक्षर*);
  sqlite3_file *(*database_file_object)(स्थिर अक्षर*);
  /* Version 3.34.0 and later */
  पूर्णांक (*txn_state)(sqlite3*,स्थिर अक्षर*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*योग_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  पूर्णांक (*autovacuum_pages)(sqlite3*,
     अचिन्हित पूर्णांक(*)(व्योम*,स्थिर अक्षर*,अचिन्हित पूर्णांक,अचिन्हित पूर्णांक,अचिन्हित पूर्णांक),
     व्योम*, व्योम(*)(व्योम*));
  /* Version 3.38.0 and later */
  पूर्णांक (*error_offset)(sqlite3*);
  पूर्णांक (*vtab_rhs_मूल्य)(sqlite3_सूचकांक_info*,पूर्णांक,sqlite3_मूल्य**);
  पूर्णांक (*vtab_distinct)(sqlite3_सूचकांक_info*);
  पूर्णांक (*vtab_in)(sqlite3_सूचकांक_info*,पूर्णांक,पूर्णांक);
  पूर्णांक (*vtab_in_first)(sqlite3_मूल्य*,sqlite3_मूल्य**);
  पूर्णांक (*vtab_in_अग्र)(sqlite3_मूल्य*,sqlite3_मूल्य**);
  /* Version 3.39.0 and later */
  पूर्णांक (*deserialize)(sqlite3*,स्थिर अक्षर*,अचिन्हित अक्षर*,
                     sqlite3_int64,sqlite3_int64,अचिन्हित);
  अचिन्हित अक्षर *(*serialize)(sqlite3*,स्थिर अक्षर *,sqlite3_int64*,
                              अचिन्हित पूर्णांक);
  स्थिर अक्षर *(*db_name)(sqlite3*,पूर्णांक);
  /* Version 3.40.0 and later */
  पूर्णांक (*मूल्य_encoding)(sqlite3_मूल्य*);
  /* Version 3.41.0 and later */
  पूर्णांक (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  पूर्णांक (*stmt_explain)(sqlite3_stmt*,पूर्णांक);
  /* Version 3.44.0 and later */
  व्योम *(*get_clientdata)(sqlite3*,स्थिर अक्षर*);
  पूर्णांक (*set_clientdata)(sqlite3*, स्थिर अक्षर*, व्योम*, व्योम(*)(व्योम*));
  /* Version 3.50.0 and later */
  पूर्णांक (*setlk_timeout)(sqlite3*,पूर्णांक,पूर्णांक);
  /* Version 3.51.0 and later */
  पूर्णांक (*set_errmsg)(sqlite3*,पूर्णांक,स्थिर अक्षर*);
  पूर्णांक (*db_status64)(sqlite3*,पूर्णांक,sqlite3_int64*,sqlite3_int64*,पूर्णांक);
  /* Version 3.52.0 and later */
  व्योम (*str_truncate)(sqlite3_str*,पूर्णांक);
  व्योम (*str_free)(sqlite3_str*);
  पूर्णांक (*carray_bind)(sqlite3_stmt*,पूर्णांक,व्योम*,पूर्णांक,पूर्णांक,व्योम(*)(व्योम*));
  पूर्णांक (*carray_bind_v2)(sqlite3_stmt*,पूर्णांक,व्योम*,पूर्णांक,पूर्णांक,व्योम(*)(व्योम*),व्योम*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(स्थिर अक्षर*);
  व्योम (*परिणाम_str)(sqlite3_context*,sqlite3_str*,पूर्णांक);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
प्रकार पूर्णांक (*sqlite3_loadext_entry)(
  sqlite3 *db,                       /* Handle to the database. */
  अक्षर **pz_Err_संदेश,                   /* Used to set error string on failure. */
  स्थिर sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#अगर !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#घोषणा sqlite3_aggregate_context      sqlite3_api->aggregate_context
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
#घोषणा sqlite3_aggregate_गिनती        sqlite3_api->aggregate_गिनती
#पूर्ण_अगर
#घोषणा sqlite3_bind_blob              sqlite3_api->bind_blob
#घोषणा sqlite3_bind_double            sqlite3_api->bind_double
#घोषणा sqlite3_bind_int               sqlite3_api->bind_int
#घोषणा sqlite3_bind_int64             sqlite3_api->bind_int64
#घोषणा sqlite3_bind_null              sqlite3_api->bind_null
#घोषणा sqlite3_bind_parameter_गिनती   sqlite3_api->bind_parameter_गिनती
#घोषणा sqlite3_bind_parameter_सूचकांक   sqlite3_api->bind_parameter_सूचकांक
#घोषणा sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#घोषणा sqlite3_bind_text              sqlite3_api->bind_text
#घोषणा sqlite3_bind_text16            sqlite3_api->bind_text16
#घोषणा sqlite3_bind_मूल्य             sqlite3_api->bind_मूल्य
#घोषणा sqlite3_busy_handler           sqlite3_api->busy_handler
#घोषणा sqlite3_busy_timeout           sqlite3_api->busy_timeout
#घोषणा sqlite3_changes                sqlite3_api->changes
#घोषणा sqlite3_close                  sqlite3_api->बंद
#घोषणा sqlite3_collation_needed       sqlite3_api->collation_needed
#घोषणा sqlite3_collation_needed16     sqlite3_api->collation_needed16
#घोषणा sqlite3_column_blob            sqlite3_api->column_blob
#घोषणा sqlite3_column_bytes           sqlite3_api->column_bytes
#घोषणा sqlite3_column_bytes16         sqlite3_api->column_bytes16
#घोषणा sqlite3_column_गिनती           sqlite3_api->column_गिनती
#घोषणा sqlite3_column_database_name   sqlite3_api->column_database_name
#घोषणा sqlite3_column_database_name16 sqlite3_api->column_database_name16
#घोषणा sqlite3_column_decltype        sqlite3_api->column_decltype
#घोषणा sqlite3_column_decltype16      sqlite3_api->column_decltype16
#घोषणा sqlite3_column_double          sqlite3_api->column_double
#घोषणा sqlite3_column_int             sqlite3_api->column_int
#घोषणा sqlite3_column_int64           sqlite3_api->column_int64
#घोषणा sqlite3_column_name            sqlite3_api->column_name
#घोषणा sqlite3_column_name16          sqlite3_api->column_name16
#घोषणा sqlite3_column_origin_name     sqlite3_api->column_origin_name
#घोषणा sqlite3_column_origin_name16   sqlite3_api->column_origin_name16
#घोषणा sqlite3_column_table_name      sqlite3_api->column_table_name
#घोषणा sqlite3_column_table_name16    sqlite3_api->column_table_name16
#घोषणा sqlite3_column_text            sqlite3_api->column_text
#घोषणा sqlite3_column_text16          sqlite3_api->column_text16
#घोषणा sqlite3_column_type            sqlite3_api->column_type
#घोषणा sqlite3_column_मूल्य           sqlite3_api->column_मूल्य
#घोषणा sqlite3_commit_hook            sqlite3_api->commit_hook
#घोषणा sqlite3_complete               sqlite3_api->complete
#घोषणा sqlite3_complete16             sqlite3_api->complete16
#घोषणा sqlite3_create_collation       sqlite3_api->create_collation
#घोषणा sqlite3_create_collation16     sqlite3_api->create_collation16
#घोषणा sqlite3_create_function        sqlite3_api->create_function
#घोषणा sqlite3_create_function16      sqlite3_api->create_function16
#घोषणा sqlite3_create_module          sqlite3_api->create_module
#घोषणा sqlite3_create_module_v2       sqlite3_api->create_module_v2
#घोषणा sqlite3_तथ्य_गिनती             sqlite3_api->तथ्य_गिनती
#घोषणा sqlite3_db_handle              sqlite3_api->db_handle
#घोषणा sqlite3_declare_vtab           sqlite3_api->declare_vtab
#घोषणा sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#घोषणा sqlite3_errcode                sqlite3_api->errcode
#घोषणा sqlite3_errmsg                 sqlite3_api->errmsg
#घोषणा sqlite3_errmsg16               sqlite3_api->errmsg16
#घोषणा sqlite3_exec                   sqlite3_api->exec
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
#घोषणा sqlite3_expired                sqlite3_api->expired
#पूर्ण_अगर
#घोषणा sqlite3_finalize               sqlite3_api->finalize
#घोषणा sqlite3_free                   sqlite3_api->मुक्त
#घोषणा sqlite3_free_table             sqlite3_api->free_table
#घोषणा sqlite3_get_autocommit         sqlite3_api->get_autocommit
#घोषणा sqlite3_get_auxdata            sqlite3_api->get_auxdata
#घोषणा sqlite3_get_table              sqlite3_api->get_table
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
#घोषणा sqlite3_global_recover         sqlite3_api->global_recover
#पूर्ण_अगर
#घोषणा sqlite3_interrupt              sqlite3_api->interruptx
#घोषणा sqlite3_last_जोड़ो_rowid      sqlite3_api->last_जोड़ो_rowid
#घोषणा sqlite3_libversion             sqlite3_api->libversion
#घोषणा sqlite3_libversion_संख्या      sqlite3_api->libversion_संख्या
#घोषणा sqlite3_malloc                 sqlite3_api->दो_स्मृति
#घोषणा sqlite3_mprintf                sqlite3_api->mprintf
#घोषणा sqlite3_open                   sqlite3_api->खोलो
#घोषणा sqlite3_open16                 sqlite3_api->open16
#घोषणा sqlite3_prepare                sqlite3_api->prepare
#घोषणा sqlite3_prepare16              sqlite3_api->prepare16
#घोषणा sqlite3_prepare_v2             sqlite3_api->prepare_v2
#घोषणा sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#घोषणा sqlite3_profile                sqlite3_api->profile
#घोषणा sqlite3_progress_handler       sqlite3_api->progress_handler
#घोषणा sqlite3_realloc                sqlite3_api->पुनः_स्मृति
#घोषणा sqlite3_reset                  sqlite3_api->reset
#घोषणा sqlite3_परिणाम_blob            sqlite3_api->परिणाम_blob
#घोषणा sqlite3_परिणाम_double          sqlite3_api->परिणाम_double
#घोषणा sqlite3_परिणाम_error           sqlite3_api->परिणाम_error
#घोषणा sqlite3_परिणाम_error16         sqlite3_api->परिणाम_error16
#घोषणा sqlite3_परिणाम_int             sqlite3_api->परिणाम_int
#घोषणा sqlite3_परिणाम_int64           sqlite3_api->परिणाम_int64
#घोषणा sqlite3_परिणाम_null            sqlite3_api->परिणाम_null
#घोषणा sqlite3_परिणाम_text            sqlite3_api->परिणाम_text
#घोषणा sqlite3_परिणाम_text16          sqlite3_api->परिणाम_text16
#घोषणा sqlite3_परिणाम_text16be        sqlite3_api->परिणाम_text16be
#घोषणा sqlite3_परिणाम_text16le        sqlite3_api->परिणाम_text16le
#घोषणा sqlite3_परिणाम_मूल्य           sqlite3_api->परिणाम_मूल्य
#घोषणा sqlite3_rollback_hook          sqlite3_api->rollback_hook
#घोषणा sqlite3_set_authorizer         sqlite3_api->set_authorizer
#घोषणा sqlite3_set_auxdata            sqlite3_api->set_auxdata
#घोषणा sqlite3_snprintf               sqlite3_api->xsnprintf
#घोषणा sqlite3_step                   sqlite3_api->step
#घोषणा sqlite3_table_column_metadata  sqlite3_api->table_column_metadata
#घोषणा sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#घोषणा sqlite3_योग_changes          sqlite3_api->योग_changes
#घोषणा sqlite3_trace                  sqlite3_api->trace
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
#घोषणा sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#पूर्ण_अगर
#घोषणा sqlite3_update_hook            sqlite3_api->update_hook
#घोषणा sqlite3_user_तथ्य              sqlite3_api->user_तथ्य
#घोषणा sqlite3_मूल्य_blob             sqlite3_api->मूल्य_blob
#घोषणा sqlite3_मूल्य_bytes            sqlite3_api->मूल्य_bytes
#घोषणा sqlite3_मूल्य_bytes16          sqlite3_api->मूल्य_bytes16
#घोषणा sqlite3_मूल्य_double           sqlite3_api->मूल्य_double
#घोषणा sqlite3_मूल्य_int              sqlite3_api->मूल्य_int
#घोषणा sqlite3_मूल्य_int64            sqlite3_api->मूल्य_int64
#घोषणा sqlite3_मूल्य_numeric_type     sqlite3_api->मूल्य_numeric_type
#घोषणा sqlite3_मूल्य_text             sqlite3_api->मूल्य_text
#घोषणा sqlite3_मूल्य_text16           sqlite3_api->मूल्य_text16
#घोषणा sqlite3_मूल्य_text16be         sqlite3_api->मूल्य_text16be
#घोषणा sqlite3_मूल्य_text16le         sqlite3_api->मूल्य_text16le
#घोषणा sqlite3_मूल्य_type             sqlite3_api->मूल्य_type
#घोषणा sqlite3_vmprintf               sqlite3_api->vmprintf
#घोषणा sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#घोषणा sqlite3_overload_function      sqlite3_api->overload_function
#घोषणा sqlite3_prepare_v2             sqlite3_api->prepare_v2
#घोषणा sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#घोषणा sqlite3_clear_bindings         sqlite3_api->clear_bindings
#घोषणा sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#घोषणा sqlite3_blob_bytes             sqlite3_api->blob_bytes
#घोषणा sqlite3_blob_close             sqlite3_api->blob_close
#घोषणा sqlite3_blob_open              sqlite3_api->blob_open
#घोषणा sqlite3_blob_read              sqlite3_api->blob_read
#घोषणा sqlite3_blob_write             sqlite3_api->blob_write
#घोषणा sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#घोषणा sqlite3_file_control           sqlite3_api->file_control
#घोषणा sqlite3_memory_highwater       sqlite3_api->memory_highwater
#घोषणा sqlite3_memory_used            sqlite3_api->memory_used
#घोषणा sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#घोषणा sqlite3_mutex_enter            sqlite3_api->mutex_enter
#घोषणा sqlite3_mutex_free             sqlite3_api->mutex_free
#घोषणा sqlite3_mutex_leave            sqlite3_api->mutex_leave
#घोषणा sqlite3_mutex_try              sqlite3_api->mutex_try
#घोषणा sqlite3_open_v2                sqlite3_api->open_v2
#घोषणा sqlite3_release_memory         sqlite3_api->release_memory
#घोषणा sqlite3_परिणाम_error_nomem     sqlite3_api->परिणाम_error_nomem
#घोषणा sqlite3_परिणाम_error_toobig    sqlite3_api->परिणाम_error_toobig
#घोषणा sqlite3_sleep                  sqlite3_api->sleep
#घोषणा sqlite3_soft_heap_limit        sqlite3_api->soft_heap_limit
#घोषणा sqlite3_vfs_find               sqlite3_api->vfs_find
#घोषणा sqlite3_vfs_register           sqlite3_api->vfs_register
#घोषणा sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#घोषणा sqlite3_threadsafe             sqlite3_api->xthreadsafe
#घोषणा sqlite3_परिणाम_zeroblob        sqlite3_api->परिणाम_zeroblob
#घोषणा sqlite3_परिणाम_error_code      sqlite3_api->परिणाम_error_code
#घोषणा sqlite3_test_control           sqlite3_api->test_control
#घोषणा sqlite3_randomness             sqlite3_api->randomness
#घोषणा sqlite3_context_db_handle      sqlite3_api->context_db_handle
#घोषणा sqlite3_extended_परिणाम_codes  sqlite3_api->extended_परिणाम_codes
#घोषणा sqlite3_limit                  sqlite3_api->limit
#घोषणा sqlite3_अग्र_stmt              sqlite3_api->अग्र_stmt
#घोषणा sqlite3_sql                    sqlite3_api->sql
#घोषणा sqlite3_status                 sqlite3_api->status
#घोषणा sqlite3_backup_finish          sqlite3_api->backup_finish
#घोषणा sqlite3_backup_init            sqlite3_api->backup_init
#घोषणा sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#घोषणा sqlite3_backup_remaining       sqlite3_api->backup_remaining
#घोषणा sqlite3_backup_step            sqlite3_api->backup_step
#घोषणा sqlite3_compileoption_get      sqlite3_api->compileoption_get
#घोषणा sqlite3_compileoption_used     sqlite3_api->compileoption_used
#घोषणा sqlite3_create_function_v2     sqlite3_api->create_function_v2
#घोषणा sqlite3_db_config              sqlite3_api->db_config
#घोषणा sqlite3_db_mutex               sqlite3_api->db_mutex
#घोषणा sqlite3_db_status              sqlite3_api->db_status
#घोषणा sqlite3_extended_errcode       sqlite3_api->extended_errcode
#घोषणा sqlite3_log                    sqlite3_api->log
#घोषणा sqlite3_soft_heap_limit64      sqlite3_api->soft_heap_limit64
#घोषणा sqlite3_sourceid               sqlite3_api->sourceid
#घोषणा sqlite3_stmt_status            sqlite3_api->stmt_status
#घोषणा sqlite3_strnicmp               sqlite3_api->strnicmp
#घोषणा sqlite3_unlock_notify          sqlite3_api->unlock_notify
#घोषणा sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#घोषणा sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#घोषणा sqlite3_wal_hook               sqlite3_api->wal_hook
#घोषणा sqlite3_blob_reopen            sqlite3_api->blob_reopen
#घोषणा sqlite3_vtab_config            sqlite3_api->vtab_config
#घोषणा sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#घोषणा sqlite3_close_v2               sqlite3_api->close_v2
#घोषणा sqlite3_db_filename            sqlite3_api->db_filename
#घोषणा sqlite3_db_readonly            sqlite3_api->db_readonly
#घोषणा sqlite3_db_release_memory      sqlite3_api->db_release_memory
#घोषणा sqlite3_errstr                 sqlite3_api->errstr
#घोषणा sqlite3_stmt_busy              sqlite3_api->stmt_busy
#घोषणा sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#घोषणा sqlite3_stricmp                sqlite3_api->stricmp
#घोषणा sqlite3_uri_boolean            sqlite3_api->uri_boolean
#घोषणा sqlite3_uri_int64              sqlite3_api->uri_int64
#घोषणा sqlite3_uri_parameter          sqlite3_api->uri_parameter
#घोषणा sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#घोषणा sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#घोषणा sqlite3_auto_extension         sqlite3_api->auto_extension
#घोषणा sqlite3_bind_blob64            sqlite3_api->bind_blob64
#घोषणा sqlite3_bind_text64            sqlite3_api->bind_text64
#घोषणा sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#घोषणा sqlite3_load_extension         sqlite3_api->load_extension
#घोषणा sqlite3_malloc64               sqlite3_api->malloc64
#घोषणा sqlite3_msize                  sqlite3_api->msize
#घोषणा sqlite3_realloc64              sqlite3_api->realloc64
#घोषणा sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#घोषणा sqlite3_परिणाम_blob64          sqlite3_api->परिणाम_blob64
#घोषणा sqlite3_परिणाम_text64          sqlite3_api->परिणाम_text64
#घोषणा sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#घोषणा sqlite3_मूल्य_dup              sqlite3_api->मूल्य_dup
#घोषणा sqlite3_मूल्य_free             sqlite3_api->मूल्य_free
#घोषणा sqlite3_परिणाम_zeroblob64      sqlite3_api->परिणाम_zeroblob64
#घोषणा sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#घोषणा sqlite3_मूल्य_subtype          sqlite3_api->मूल्य_subtype
#घोषणा sqlite3_परिणाम_subtype         sqlite3_api->परिणाम_subtype
/* Version 3.10.0 and later */
#घोषणा sqlite3_status64               sqlite3_api->status64
#घोषणा sqlite3_strlike                sqlite3_api->strlike
#घोषणा sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#घोषणा sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#घोषणा sqlite3_trace_v2               sqlite3_api->trace_v2
#घोषणा sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#घोषणा sqlite3_set_last_जोड़ो_rowid  sqlite3_api->set_last_जोड़ो_rowid
/* Version 3.20.0 and later */
#घोषणा sqlite3_prepare_v3             sqlite3_api->prepare_v3
#घोषणा sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#घोषणा sqlite3_bind_pointer           sqlite3_api->bind_pointer
#घोषणा sqlite3_परिणाम_pointer         sqlite3_api->परिणाम_pointer
#घोषणा sqlite3_मूल्य_pointer          sqlite3_api->मूल्य_pointer
/* Version 3.22.0 and later */
#घोषणा sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#घोषणा sqlite3_मूल्य_nochange         sqlite3_api->मूल्य_nochange
#घोषणा sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#घोषणा sqlite3_keyword_गिनती          sqlite3_api->keyword_गिनती
#घोषणा sqlite3_keyword_name           sqlite3_api->keyword_name
#घोषणा sqlite3_keyword_check          sqlite3_api->keyword_check
#घोषणा sqlite3_str_new                sqlite3_api->str_new
#घोषणा sqlite3_str_finish             sqlite3_api->str_finish
#घोषणा sqlite3_str_appendf            sqlite3_api->str_appendf
#घोषणा sqlite3_str_vappendf           sqlite3_api->str_vappendf
#घोषणा sqlite3_str_append             sqlite3_api->str_append
#घोषणा sqlite3_str_appendall          sqlite3_api->str_appendall
#घोषणा sqlite3_str_appendchar         sqlite3_api->str_appendchar
#घोषणा sqlite3_str_reset              sqlite3_api->str_reset
#घोषणा sqlite3_str_errcode            sqlite3_api->str_errcode
#घोषणा sqlite3_str_लंबाई             sqlite3_api->str_लंबाई
#घोषणा sqlite3_str_मूल्य              sqlite3_api->str_मूल्य
/* Version 3.25.0 and later */
#घोषणा sqlite3_create_window_function sqlite3_api->create_window_function
/* Version 3.26.0 and later */
#घोषणा sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#घोषणा sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#घोषणा sqlite3_मूल्य_frombind         sqlite3_api->मूल्य_frombind
/* Version 3.30.0 and later */
#घोषणा sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#घोषणा sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#घोषणा sqlite3_uri_कुंजी                sqlite3_api->uri_कुंजी
#घोषणा sqlite3_filename_database      sqlite3_api->filename_database
#घोषणा sqlite3_filename_journal       sqlite3_api->filename_journal
#घोषणा sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#घोषणा sqlite3_create_filename        sqlite3_api->create_filename
#घोषणा sqlite3_free_filename          sqlite3_api->free_filename
#घोषणा sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#घोषणा sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#घोषणा sqlite3_changes64              sqlite3_api->changes64
#घोषणा sqlite3_योग_changes64        sqlite3_api->योग_changes64
/* Version 3.37.0 and later */
#घोषणा sqlite3_autovacuum_pages       sqlite3_api->autovacuum_pages
/* Version 3.38.0 and later */
#घोषणा sqlite3_error_offset           sqlite3_api->error_offset
#घोषणा sqlite3_vtab_rhs_मूल्य         sqlite3_api->vtab_rhs_मूल्य
#घोषणा sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#घोषणा sqlite3_vtab_in                sqlite3_api->vtab_in
#घोषणा sqlite3_vtab_in_first          sqlite3_api->vtab_in_first
#घोषणा sqlite3_vtab_in_अग्र           sqlite3_api->vtab_in_अग्र
/* Version 3.39.0 and later */
#अगर_अघोषित SQLITE_OMIT_DESERIALIZE
#घोषणा sqlite3_deserialize            sqlite3_api->deserialize
#घोषणा sqlite3_serialize              sqlite3_api->serialize
#पूर्ण_अगर
#घोषणा sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#घोषणा sqlite3_मूल्य_encoding         sqlite3_api->मूल्य_encoding
/* Version 3.41.0 and later */
#घोषणा sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#घोषणा sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#घोषणा sqlite3_get_clientdata         sqlite3_api->get_clientdata
#घोषणा sqlite3_set_clientdata         sqlite3_api->set_clientdata
/* Version 3.50.0 and later */
#घोषणा sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#घोषणा sqlite3_set_errmsg             sqlite3_api->set_errmsg
#घोषणा sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#घोषणा sqlite3_str_truncate           sqlite3_api->str_truncate
#घोषणा sqlite3_str_free               sqlite3_api->str_free
#घोषणा sqlite3_carray_bind            sqlite3_api->carray_bind
#घोषणा sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#घोषणा sqlite3_incomplete             sqlite3_api->incomplete
#घोषणा sqlite3_परिणाम_str             sqlite3_api->परिणाम_str
#पूर्ण_अगर /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#अगर !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     स्थिर sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    बाह्य स्थिर sqlite3_api_routines *sqlite3_api;
#अन्यथा
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (व्योम)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#पूर्ण_अगर

#पूर्ण_अगर /* SQLITE3EXT_H */
