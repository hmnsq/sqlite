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
#నిర్వచితం_కాకపోతే SQLITE3EXT_H
#నిర్వచించు SQLITE3EXT_H
#అన్తర్భావ "sqlite3.h"

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
నిర్మాణం sqlite3_api_routines {
  శూన్య * (*aggregate_context)(sqlite3_context*,పూర్ణాంకం n_బైట్లు);
  పూర్ణాంకం  (*aggregate_లెక్క)(sqlite3_context*);
  పూర్ణాంకం  (*bind_blob)(sqlite3_stmt*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం n,శూన్య(*)(శూన్య*));
  పూర్ణాంకం  (*bind_double)(sqlite3_stmt*,పూర్ణాంకం,ద్విగుణ);
  పూర్ణాంకం  (*bind_int)(sqlite3_stmt*,పూర్ణాంకం,పూర్ణాంకం);
  పూర్ణాంకం  (*bind_int64)(sqlite3_stmt*,పూర్ణాంకం,sqlite_int64);
  పూర్ణాంకం  (*bind_null)(sqlite3_stmt*,పూర్ణాంకం);
  పూర్ణాంకం  (*bind_parameter_లెక్క)(sqlite3_stmt*);
  పూర్ణాంకం  (*bind_parameter_సూచీ)(sqlite3_stmt*,స్థిరరాశి అక్షరం*zName);
  స్థిరరాశి అక్షరం * (*bind_parameter_name)(sqlite3_stmt*,పూర్ణాంకం);
  పూర్ణాంకం  (*bind_పాఠ్యం)(sqlite3_stmt*,పూర్ణాంకం,స్థిరరాశి అక్షరం*,పూర్ణాంకం n,శూన్య(*)(శూన్య*));
  పూర్ణాంకం  (*bind_text16)(sqlite3_stmt*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,శూన్య(*)(శూన్య*));
  పూర్ణాంకం  (*bind_విలువ)(sqlite3_stmt*,పూర్ణాంకం,స్థిరరాశి sqlite3_విలువ*);
  పూర్ణాంకం  (*busy_handler)(sqlite3*,పూర్ణాంకం(*)(శూన్య*,పూర్ణాంకం),శూన్య*);
  పూర్ణాంకం  (*busy_timeout)(sqlite3*,పూర్ణాంకం ms);
  పూర్ణాంకం  (*changes)(sqlite3*);
  పూర్ణాంకం  (*close)(sqlite3*);
  పూర్ణాంకం  (*collation_needed)(sqlite3*,శూన్య*,శూన్య(*)(శూన్య*,sqlite3*,
                           పూర్ణాంకం e_పాఠ్యం_Rep,స్థిరరాశి అక్షరం*));
  పూర్ణాంకం  (*collation_needed16)(sqlite3*,శూన్య*,శూన్య(*)(శూన్య*,sqlite3*,
                             పూర్ణాంకం e_పాఠ్యం_Rep,స్థిరరాశి శూన్య*));
  స్థిరరాశి శూన్య * (*స్తంభం_blob)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  పూర్ణాంకం  (*స్తంభం_బైట్లు)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  పూర్ణాంకం  (*స్తంభం_bytes16)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  పూర్ణాంకం  (*స్తంభం_లెక్క)(sqlite3_stmt*pStmt);
  స్థిరరాశి అక్షరం * (*స్తంభం_database_name)(sqlite3_stmt*,పూర్ణాంకం);
  స్థిరరాశి శూన్య * (*స్తంభం_database_name16)(sqlite3_stmt*,పూర్ణాంకం);
  స్థిరరాశి అక్షరం * (*స్తంభం_decltype)(sqlite3_stmt*,పూర్ణాంకం i);
  స్థిరరాశి శూన్య * (*స్తంభం_decltype16)(sqlite3_stmt*,పూర్ణాంకం);
  ద్విగుణ  (*స్తంభం_double)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  పూర్ణాంకం  (*స్తంభం_int)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  sqlite_int64  (*స్తంభం_int64)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  స్థిరరాశి అక్షరం * (*స్తంభం_name)(sqlite3_stmt*,పూర్ణాంకం);
  స్థిరరాశి శూన్య * (*స్తంభం_name16)(sqlite3_stmt*,పూర్ణాంకం);
  స్థిరరాశి అక్షరం * (*స్తంభం_origin_name)(sqlite3_stmt*,పూర్ణాంకం);
  స్థిరరాశి శూన్య * (*స్తంభం_origin_name16)(sqlite3_stmt*,పూర్ణాంకం);
  స్థిరరాశి అక్షరం * (*స్తంభం_పట్టిక_name)(sqlite3_stmt*,పూర్ణాంకం);
  స్థిరరాశి శూన్య * (*స్తంభం_పట్టిక_name16)(sqlite3_stmt*,పూర్ణాంకం);
  స్థిరరాశి గుర్తులేని అక్షరం * (*స్తంభం_పాఠ్యం)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  స్థిరరాశి శూన్య * (*స్తంభం_text16)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  పూర్ణాంకం  (*స్తంభం_type)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  sqlite3_విలువ* (*స్తంభం_విలువ)(sqlite3_stmt*,పూర్ణాంకం i_నిలువు);
  శూన్య * (*commit_hook)(sqlite3*,పూర్ణాంకం(*)(శూన్య*),శూన్య*);
  పూర్ణాంకం  (*complete)(స్థిరరాశి అక్షరం*sql);
  పూర్ణాంకం  (*complete16)(స్థిరరాశి శూన్య*sql);
  పూర్ణాంకం  (*create_collation)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,శూన్య*,
                           పూర్ణాంకం(*)(శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*));
  పూర్ణాంకం  (*create_collation16)(sqlite3*,స్థిరరాశి శూన్య*,పూర్ణాంకం,శూన్య*,
                             పూర్ణాంకం(*)(శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*));
  పూర్ణాంకం  (*create_function)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,పూర్ణాంకం,శూన్య*,
                          శూన్య (*xFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
                          శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
                          శూన్య (*xFinal)(sqlite3_context*));
  పూర్ణాంకం  (*create_function16)(sqlite3*,స్థిరరాశి శూన్య*,పూర్ణాంకం,పూర్ణాంకం,శూన్య*,
                            శూన్య (*xFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
                            శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
                            శూన్య (*xFinal)(sqlite3_context*));
  పూర్ణాంకం (*create_module)(sqlite3*,స్థిరరాశి అక్షరం*,స్థిరరాశి sqlite3_module*,శూన్య*);
  పూర్ణాంకం  (*దత్తాంశం_లెక్క)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  పూర్ణాంకం (*declare_vtab)(sqlite3*,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం  (*enable_shared_cache)(పూర్ణాంకం);
  పూర్ణాంకం  (*errcode)(sqlite3*db);
  స్థిరరాశి అక్షరం * (*errmsg)(sqlite3*);
  స్థిరరాశి శూన్య * (*errmsg16)(sqlite3*);
  పూర్ణాంకం  (*exec)(sqlite3*,స్థిరరాశి అక్షరం*,sqlite3_callback,శూన్య*,అక్షరం**);
  పూర్ణాంకం  (*expired)(sqlite3_stmt*);
  పూర్ణాంకం  (*finalize)(sqlite3_stmt*pStmt);
  శూన్య  (*విడుదల)(శూన్య*);
  శూన్య  (*free_పట్టిక)(అక్షరం**ఫలితం);
  పూర్ణాంకం  (*get_autocommit)(sqlite3*);
  శూన్య * (*get_auxdata)(sqlite3_context*,పూర్ణాంకం);
  పూర్ణాంకం  (*get_పట్టిక)(sqlite3*,స్థిరరాశి అక్షరం*,అక్షరం***,పూర్ణాంకం*,పూర్ణాంకం*,అక్షరం**);
  పూర్ణాంకం  (*global_recover)(శూన్య);
  శూన్య  (*interruptx)(sqlite3*);
  sqlite_int64  (*చివరి_చొప్పించు_rowid)(sqlite3*);
  స్థిరరాశి అక్షరం * (*libversion)(శూన్య);
  పూర్ణాంకం  (*libversion_సంఖ్య)(శూన్య);
  శూన్య *(*జ్ఞాపకం_ఇవ్వు)(పూర్ణాంకం);
  అక్షరం *(SQLITE_CDECL*mprintf)(స్థిరరాశి అక్షరం*,...);
  పూర్ణాంకం  (*open)(స్థిరరాశి అక్షరం*,sqlite3**);
  పూర్ణాంకం  (*open16)(స్థిరరాశి శూన్య*,sqlite3**);
  పూర్ణాంకం  (*prepare)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,sqlite3_stmt**,స్థిరరాశి అక్షరం**);
  పూర్ణాంకం  (*prepare16)(sqlite3*,స్థిరరాశి శూన్య*,పూర్ణాంకం,sqlite3_stmt**,స్థిరరాశి శూన్య**);
  శూన్య * (*profile)(sqlite3*,శూన్య(*)(శూన్య*,స్థిరరాశి అక్షరం*,sqlite_uint64),శూన్య*);
  శూన్య  (*పురోగతి_handler)(sqlite3*,పూర్ణాంకం,పూర్ణాంకం(*)(శూన్య*),శూన్య*);
  శూన్య *(*realloc)(శూన్య*,పూర్ణాంకం);
  పూర్ణాంకం  (*reset)(sqlite3_stmt*pStmt);
  శూన్య  (*ఫలితం_blob)(sqlite3_context*,స్థిరరాశి శూన్య*,పూర్ణాంకం,శూన్య(*)(శూన్య*));
  శూన్య  (*ఫలితం_double)(sqlite3_context*,ద్విగుణ);
  శూన్య  (*ఫలితం_error)(sqlite3_context*,స్థిరరాశి అక్షరం*,పూర్ణాంకం);
  శూన్య  (*ఫలితం_error16)(sqlite3_context*,స్థిరరాశి శూన్య*,పూర్ణాంకం);
  శూన్య  (*ఫలితం_int)(sqlite3_context*,పూర్ణాంకం);
  శూన్య  (*ఫలితం_int64)(sqlite3_context*,sqlite_int64);
  శూన్య  (*ఫలితం_null)(sqlite3_context*);
  శూన్య  (*ఫలితం_పాఠ్యం)(sqlite3_context*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,శూన్య(*)(శూన్య*));
  శూన్య  (*ఫలితం_text16)(sqlite3_context*,స్థిరరాశి శూన్య*,పూర్ణాంకం,శూన్య(*)(శూన్య*));
  శూన్య  (*ఫలితం_text16be)(sqlite3_context*,స్థిరరాశి శూన్య*,పూర్ణాంకం,శూన్య(*)(శూన్య*));
  శూన్య  (*ఫలితం_text16le)(sqlite3_context*,స్థిరరాశి శూన్య*,పూర్ణాంకం,శూన్య(*)(శూన్య*));
  శూన్య  (*ఫలితం_విలువ)(sqlite3_context*,sqlite3_విలువ*);
  శూన్య *(*rollback_hook)(sqlite3*,శూన్య(*)(శూన్య*),శూన్య*);
  పూర్ణాంకం  (*సమితి_authorizer)(sqlite3*,పూర్ణాంకం(*)(శూన్య*,పూర్ణాంకం,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,
                         స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*),శూన్య*);
  శూన్య  (*సమితి_auxdata)(sqlite3_context*,పూర్ణాంకం,శూన్య*,శూన్య (*)(శూన్య*));
  అక్షరం *(SQLITE_CDECL*xsnprintf)(పూర్ణాంకం,అక్షరం*,స్థిరరాశి అక్షరం*,...);
  పూర్ణాంకం  (*అడుగు)(sqlite3_stmt*);
  పూర్ణాంకం  (*పట్టిక_స్తంభం_metadata)(sqlite3*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,
                                అక్షరం స్థిరరాశి**,అక్షరం స్థిరరాశి**,పూర్ణాంకం*,పూర్ణాంకం*,పూర్ణాంకం*);
  శూన్య  (*thread_cleanup)(శూన్య);
  పూర్ణాంకం  (*మొత్తం_changes)(sqlite3*);
  శూన్య * (*trace)(sqlite3*,శూన్య(*xTrace)(శూన్య*,స్థిరరాశి అక్షరం*),శూన్య*);
  పూర్ణాంకం  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  శూన్య * (*update_hook)(sqlite3*,శూన్య(*)(శూన్య*,పూర్ణాంకం ,అక్షరం స్థిరరాశి*,అక్షరం స్థిరరాశి*,
                                         sqlite_int64),శూన్య*);
  శూన్య * (*user_దత్తాంశం)(sqlite3_context*);
  స్థిరరాశి శూన్య * (*విలువ_blob)(sqlite3_విలువ*);
  పూర్ణాంకం  (*విలువ_బైట్లు)(sqlite3_విలువ*);
  పూర్ణాంకం  (*విలువ_bytes16)(sqlite3_విలువ*);
  ద్విగుణ  (*విలువ_double)(sqlite3_విలువ*);
  పూర్ణాంకం  (*విలువ_int)(sqlite3_విలువ*);
  sqlite_int64  (*విలువ_int64)(sqlite3_విలువ*);
  పూర్ణాంకం  (*విలువ_numeric_type)(sqlite3_విలువ*);
  స్థిరరాశి గుర్తులేని అక్షరం * (*విలువ_పాఠ్యం)(sqlite3_విలువ*);
  స్థిరరాశి శూన్య * (*విలువ_text16)(sqlite3_విలువ*);
  స్థిరరాశి శూన్య * (*విలువ_text16be)(sqlite3_విలువ*);
  స్థిరరాశి శూన్య * (*విలువ_text16le)(sqlite3_విలువ*);
  పూర్ణాంకం  (*విలువ_type)(sqlite3_విలువ*);
  అక్షరం *(*vmprintf)(స్థిరరాశి అక్షరం*,va_list);
  /* Added ??? */
  పూర్ణాంకం (*overload_function)(sqlite3*, స్థిరరాశి అక్షరం *zFuncName, పూర్ణాంకం nArg);
  /* Added by 3.3.13 */
  పూర్ణాంకం (*prepare_v2)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,sqlite3_stmt**,స్థిరరాశి అక్షరం**);
  పూర్ణాంకం (*prepare16_v2)(sqlite3*,స్థిరరాశి శూన్య*,పూర్ణాంకం,sqlite3_stmt**,స్థిరరాశి శూన్య**);
  పూర్ణాంకం (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  పూర్ణాంకం (*create_module_v2)(sqlite3*,స్థిరరాశి అక్షరం*,స్థిరరాశి sqlite3_module*,శూన్య*,
                          శూన్య (*xDestroy)(శూన్య *));
  /* Added by 3.5.0 */
  పూర్ణాంకం (*bind_zeroblob)(sqlite3_stmt*,పూర్ణాంకం,పూర్ణాంకం);
  పూర్ణాంకం (*blob_బైట్లు)(sqlite3_blob*);
  పూర్ణాంకం (*blob_close)(sqlite3_blob*);
  పూర్ణాంకం (*blob_open)(sqlite3*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,sqlite3_int64,
                   పూర్ణాంకం,sqlite3_blob**);
  పూర్ణాంకం (*blob_read)(sqlite3_blob*,శూన్య*,పూర్ణాంకం,పూర్ణాంకం);
  పూర్ణాంకం (*blob_write)(sqlite3_blob*,స్థిరరాశి శూన్య*,పూర్ణాంకం,పూర్ణాంకం);
  పూర్ణాంకం (*create_collation_v2)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,శూన్య*,
                             పూర్ణాంకం(*)(శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*),
                             శూన్య(*)(శూన్య*));
  పూర్ణాంకం (*file_control)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,శూన్య*);
  sqlite3_int64 (*జ్ఞాపకం_highwater)(పూర్ణాంకం);
  sqlite3_int64 (*జ్ఞాపకం_వాడిన)(శూన్య);
  sqlite3_mutex *(*mutex_alloc)(పూర్ణాంకం);
  శూన్య (*mutex_enter)(sqlite3_mutex*);
  శూన్య (*mutex_free)(sqlite3_mutex*);
  శూన్య (*mutex_leave)(sqlite3_mutex*);
  పూర్ణాంకం (*mutex_try)(sqlite3_mutex*);
  పూర్ణాంకం (*open_v2)(స్థిరరాశి అక్షరం*,sqlite3**,పూర్ణాంకం,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*release_జ్ఞాపకం)(పూర్ణాంకం);
  శూన్య (*ఫలితం_error_nomem)(sqlite3_context*);
  శూన్య (*ఫలితం_error_toobig)(sqlite3_context*);
  పూర్ణాంకం (*sleep)(పూర్ణాంకం);
  శూన్య (*soft_కుప్ప_హద్దు)(పూర్ణాంకం);
  sqlite3_vfs *(*vfs_కనుగొను)(స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*vfs_రిజిస్టరు)(sqlite3_vfs*,పూర్ణాంకం);
  పూర్ణాంకం (*vfs_unregister)(sqlite3_vfs*);
  పూర్ణాంకం (*xthreadsafe)(శూన్య);
  శూన్య (*ఫలితం_zeroblob)(sqlite3_context*,పూర్ణాంకం);
  శూన్య (*ఫలితం_error_సంకేతం)(sqlite3_context*,పూర్ణాంకం);
  పూర్ణాంకం (SQLITE_CDECL*test_control)(పూర్ణాంకం, ...);
  శూన్య (*randomness)(పూర్ణాంకం,శూన్య*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  పూర్ణాంకం (*extended_ఫలితం_codes)(sqlite3*,పూర్ణాంకం);
  పూర్ణాంకం (*హద్దు)(sqlite3*,పూర్ణాంకం,పూర్ణాంకం);
  sqlite3_stmt *(*తదుపరి_stmt)(sqlite3*,sqlite3_stmt*);
  స్థిరరాశి అక్షరం *(*sql)(sqlite3_stmt*);
  పూర్ణాంకం (*స్థితి_2)(పూర్ణాంకం,పూర్ణాంకం*,పూర్ణాంకం*,పూర్ణాంకం);
  పూర్ణాంకం (*backup_ముగింపు)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,స్థిరరాశి అక్షరం*,sqlite3*,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*backup_pagecount)(sqlite3_backup*);
  పూర్ణాంకం (*backup_మిగిలిన)(sqlite3_backup*);
  పూర్ణాంకం (*backup_అడుగు)(sqlite3_backup*,పూర్ణాంకం);
  స్థిరరాశి అక్షరం *(*compileoption_get)(పూర్ణాంకం);
  పూర్ణాంకం (*compileoption_వాడిన)(స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*create_function_v2)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,పూర్ణాంకం,శూన్య*,
                            శూన్య (*xFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
                            శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
                            శూన్య (*xFinal)(sqlite3_context*),
                            శూన్య(*xDestroy)(శూన్య*));
  పూర్ణాంకం (SQLITE_CDECL*db_config)(sqlite3*,పూర్ణాంకం,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  పూర్ణాంకం (*db_స్థితి)(sqlite3*,పూర్ణాంకం,పూర్ణాంకం*,పూర్ణాంకం*,పూర్ణాంకం);
  పూర్ణాంకం (*extended_errcode)(sqlite3*);
  శూన్య (SQLITE_CDECL*సంవర్గమానం)(పూర్ణాంకం,స్థిరరాశి అక్షరం*,...);
  sqlite3_int64 (*soft_కుప్ప_limit64)(sqlite3_int64);
  స్థిరరాశి అక్షరం *(*sourceid)(శూన్య);
  పూర్ణాంకం (*stmt_స్థితి)(sqlite3_stmt*,పూర్ణాంకం,పూర్ణాంకం);
  పూర్ణాంకం (*strnicmp)(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,పూర్ణాంకం);
  పూర్ణాంకం (*unlock_notify)(sqlite3*,శూన్య(*)(శూన్య**,పూర్ణాంకం),శూన్య*);
  పూర్ణాంకం (*wal_autocheckpoint)(sqlite3*,పూర్ణాంకం);
  పూర్ణాంకం (*wal_checkpoint)(sqlite3*,స్థిరరాశి అక్షరం*);
  శూన్య *(*wal_hook)(sqlite3*,పూర్ణాంకం(*)(శూన్య*,sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం),శూన్య*);
  పూర్ణాంకం (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  పూర్ణాంకం (SQLITE_CDECL*vtab_config)(sqlite3*,పూర్ణాంకం op,...);
  పూర్ణాంకం (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  పూర్ణాంకం (*close_v2)(sqlite3*);
  స్థిరరాశి అక్షరం *(*db_filename)(sqlite3*,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*db_readonly)(sqlite3*,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*db_release_జ్ఞాపకం)(sqlite3*);
  స్థిరరాశి అక్షరం *(*errstr)(పూర్ణాంకం);
  పూర్ణాంకం (*stmt_busy)(sqlite3_stmt*);
  పూర్ణాంకం (*stmt_readonly)(sqlite3_stmt*);
  పూర్ణాంకం (*stricmp)(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*uri_boolean)(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,పూర్ణాంకం);
  sqlite3_int64 (*uri_int64)(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,sqlite3_int64);
  స్థిరరాశి అక్షరం *(*uri_parameter)(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
  అక్షరం *(*xvsnprintf)(పూర్ణాంకం,అక్షరం*,స్థిరరాశి అక్షరం*,va_list);
  పూర్ణాంకం (*wal_checkpoint_v2)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,పూర్ణాంకం*,పూర్ణాంకం*);
  /* Version 3.8.7 and later */
  పూర్ణాంకం (*auto_extension)(శూన్య(*)(శూన్య));
  పూర్ణాంకం (*bind_blob64)(sqlite3_stmt*,పూర్ణాంకం,స్థిరరాశి శూన్య*,sqlite3_uint64,
                     శూన్య(*)(శూన్య*));
  పూర్ణాంకం (*bind_text64)(sqlite3_stmt*,పూర్ణాంకం,స్థిరరాశి అక్షరం*,sqlite3_uint64,
                      శూన్య(*)(శూన్య*),గుర్తులేని అక్షరం);
  పూర్ణాంకం (*cancel_auto_extension)(శూన్య(*)(శూన్య));
  పూర్ణాంకం (*ఎక్కించు_extension)(sqlite3*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,అక్షరం**);
  శూన్య *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(శూన్య*);
  శూన్య *(*realloc64)(శూన్య*,sqlite3_uint64);
  శూన్య (*reset_auto_extension)(శూన్య);
  శూన్య (*ఫలితం_blob64)(sqlite3_context*,స్థిరరాశి శూన్య*,sqlite3_uint64,
                        శూన్య(*)(శూన్య*));
  శూన్య (*ఫలితం_text64)(sqlite3_context*,స్థిరరాశి అక్షరం*,sqlite3_uint64,
                         శూన్య(*)(శూన్య*), గుర్తులేని అక్షరం);
  పూర్ణాంకం (*strglob)(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
  /* Version 3.8.11 and later */
  sqlite3_విలువ *(*విలువ_dup)(స్థిరరాశి sqlite3_విలువ*);
  శూన్య (*విలువ_free)(sqlite3_విలువ*);
  పూర్ణాంకం (*ఫలితం_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  పూర్ణాంకం (*bind_zeroblob64)(sqlite3_stmt*, పూర్ణాంకం, sqlite3_uint64);
  /* Version 3.9.0 and later */
  గుర్తులేని పూర్ణాంకం (*విలువ_subtype)(sqlite3_విలువ*);
  శూన్య (*ఫలితం_subtype)(sqlite3_context*,గుర్తులేని పూర్ణాంకం);
  /* Version 3.10.0 and later */
  పూర్ణాంకం (*status64)(పూర్ణాంకం,sqlite3_int64*,sqlite3_int64*,పూర్ణాంకం);
  పూర్ణాంకం (*strlike)(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,గుర్తులేని పూర్ణాంకం);
  పూర్ణాంకం (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  పూర్ణాంకం (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  పూర్ణాంకం (*trace_v2)(sqlite3*,గుర్తులేని,పూర్ణాంకం(*)(గుర్తులేని,శూన్య*,శూన్య*,శూన్య*),శూన్య*);
  అక్షరం *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  శూన్య (*సమితి_చివరి_చొప్పించు_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  పూర్ణాంకం (*prepare_v3)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,గుర్తులేని పూర్ణాంకం,
                    sqlite3_stmt**,స్థిరరాశి అక్షరం**);
  పూర్ణాంకం (*prepare16_v3)(sqlite3*,స్థిరరాశి శూన్య*,పూర్ణాంకం,గుర్తులేని పూర్ణాంకం,
                      sqlite3_stmt**,స్థిరరాశి శూన్య**);
  పూర్ణాంకం (*bind_సూచిక)(sqlite3_stmt*,పూర్ణాంకం,శూన్య*,స్థిరరాశి అక్షరం*,శూన్య(*)(శూన్య*));
  శూన్య (*ఫలితం_సూచిక)(sqlite3_context*,శూన్య*,స్థిరరాశి అక్షరం*,శూన్య(*)(శూన్య*));
  శూన్య *(*విలువ_సూచిక)(sqlite3_విలువ*,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*vtab_nochange)(sqlite3_context*);
  పూర్ణాంకం (*విలువ_nochange)(sqlite3_విలువ*);
  స్థిరరాశి అక్షరం *(*vtab_collation)(sqlite3_సూచీ_info*,పూర్ణాంకం);
  /* Version 3.24.0 and later */
  పూర్ణాంకం (*keyword_లెక్క)(శూన్య);
  పూర్ణాంకం (*keyword_name)(పూర్ణాంకం,స్థిరరాశి అక్షరం**,పూర్ణాంకం*);
  పూర్ణాంకం (*keyword_check)(స్థిరరాశి అక్షరం*,పూర్ణాంకం);
  sqlite3_str *(*str_new)(sqlite3*);
  అక్షరం *(*str_ముగింపు)(sqlite3_str*);
  శూన్య (SQLITE_CDECL*str_appendf)(sqlite3_str*, స్థిరరాశి అక్షరం *zFormat, ...);
  శూన్య (*str_vappendf)(sqlite3_str*, స్థిరరాశి అక్షరం *zFormat, va_list);
  శూన్య (*str_append)(sqlite3_str*, స్థిరరాశి అక్షరం *z_లోపల, పూర్ణాంకం N);
  శూన్య (*str_appendall)(sqlite3_str*, స్థిరరాశి అక్షరం *z_లోపల);
  శూన్య (*str_appendchar)(sqlite3_str*, పూర్ణాంకం N, అక్షరం C);
  శూన్య (*str_reset)(sqlite3_str*);
  పూర్ణాంకం (*str_errcode)(sqlite3_str*);
  పూర్ణాంకం (*str_నిడివి)(sqlite3_str*);
  అక్షరం *(*str_విలువ)(sqlite3_str*);
  /* Version 3.25.0 and later */
  పూర్ణాంకం (*create_కిటికీ_function)(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,పూర్ణాంకం,శూన్య*,
                            శూన్య (*x_అడుగు)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
                            శూన్య (*xFinal)(sqlite3_context*),
                            శూన్య (*x_విలువ)(sqlite3_context*),
                            శూన్య (*xInv)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),
                            శూన్య(*xDestroy)(శూన్య*));
  /* Version 3.26.0 and later */
  స్థిరరాశి అక్షరం *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  పూర్ణాంకం (*stmt_isexplain)(sqlite3_stmt*);
  పూర్ణాంకం (*విలువ_frombind)(sqlite3_విలువ*);
  /* Version 3.30.0 and later */
  పూర్ణాంకం (*drop_modules)(sqlite3*,స్థిరరాశి అక్షరం**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_కుప్ప_limit64)(sqlite3_int64);
  స్థిరరాశి అక్షరం *(*uri_కీలకం)(స్థిరరాశి అక్షరం*,పూర్ణాంకం);
  స్థిరరాశి అక్షరం *(*filename_database)(స్థిరరాశి అక్షరం*);
  స్థిరరాశి అక్షరం *(*filename_journal)(స్థిరరాశి అక్షరం*);
  స్థిరరాశి అక్షరం *(*filename_wal)(స్థిరరాశి అక్షరం*);
  /* Version 3.32.0 and later */
  స్థిరరాశి అక్షరం *(*create_filename)(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,
                           పూర్ణాంకం,స్థిరరాశి అక్షరం**);
  శూన్య (*free_filename)(స్థిరరాశి అక్షరం*);
  sqlite3_file *(*database_file_object)(స్థిరరాశి అక్షరం*);
  /* Version 3.34.0 and later */
  పూర్ణాంకం (*txn_state)(sqlite3*,స్థిరరాశి అక్షరం*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*మొత్తం_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  పూర్ణాంకం (*autovacuum_పుటలు)(sqlite3*,
     గుర్తులేని పూర్ణాంకం(*)(శూన్య*,స్థిరరాశి అక్షరం*,గుర్తులేని పూర్ణాంకం,గుర్తులేని పూర్ణాంకం,గుర్తులేని పూర్ణాంకం),
     శూన్య*, శూన్య(*)(శూన్య*));
  /* Version 3.38.0 and later */
  పూర్ణాంకం (*error_స్థానభ్రంశం)(sqlite3*);
  పూర్ణాంకం (*vtab_rhs_విలువ)(sqlite3_సూచీ_info*,పూర్ణాంకం,sqlite3_విలువ**);
  పూర్ణాంకం (*vtab_distinct)(sqlite3_సూచీ_info*);
  పూర్ణాంకం (*vtab_లోపల)(sqlite3_సూచీ_info*,పూర్ణాంకం,పూర్ణాంకం);
  పూర్ణాంకం (*vtab_లోపల_మొదటి)(sqlite3_విలువ*,sqlite3_విలువ**);
  పూర్ణాంకం (*vtab_లోపల_తదుపరి)(sqlite3_విలువ*,sqlite3_విలువ**);
  /* Version 3.39.0 and later */
  పూర్ణాంకం (*deserialize)(sqlite3*,స్థిరరాశి అక్షరం*,గుర్తులేని అక్షరం*,
                     sqlite3_int64,sqlite3_int64,గుర్తులేని);
  గుర్తులేని అక్షరం *(*serialize)(sqlite3*,స్థిరరాశి అక్షరం *,sqlite3_int64*,
                              గుర్తులేని పూర్ణాంకం);
  స్థిరరాశి అక్షరం *(*db_name)(sqlite3*,పూర్ణాంకం);
  /* Version 3.40.0 and later */
  పూర్ణాంకం (*విలువ_encoding)(sqlite3_విలువ*);
  /* Version 3.41.0 and later */
  పూర్ణాంకం (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  పూర్ణాంకం (*stmt_explain)(sqlite3_stmt*,పూర్ణాంకం);
  /* Version 3.44.0 and later */
  శూన్య *(*get_clientdata)(sqlite3*,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*సమితి_clientdata)(sqlite3*, స్థిరరాశి అక్షరం*, శూన్య*, శూన్య(*)(శూన్య*));
  /* Version 3.50.0 and later */
  పూర్ణాంకం (*setlk_timeout)(sqlite3*,పూర్ణాంకం,పూర్ణాంకం);
  /* Version 3.51.0 and later */
  పూర్ణాంకం (*సమితి_errmsg)(sqlite3*,పూర్ణాంకం,స్థిరరాశి అక్షరం*);
  పూర్ణాంకం (*db_status64)(sqlite3*,పూర్ణాంకం,sqlite3_int64*,sqlite3_int64*,పూర్ణాంకం);
  /* Version 3.52.0 and later */
  శూన్య (*str_truncate)(sqlite3_str*,పూర్ణాంకం);
  శూన్య (*str_free)(sqlite3_str*);
  పూర్ణాంకం (*carray_bind)(sqlite3_stmt*,పూర్ణాంకం,శూన్య*,పూర్ణాంకం,పూర్ణాంకం,శూన్య(*)(శూన్య*));
  పూర్ణాంకం (*carray_bind_v2)(sqlite3_stmt*,పూర్ణాంకం,శూన్య*,పూర్ణాంకం,పూర్ణాంకం,శూన్య(*)(శూన్య*),శూన్య*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(స్థిరరాశి అక్షరం*);
  శూన్య (*ఫలితం_str)(sqlite3_context*,sqlite3_str*,పూర్ణాంకం);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
రకనిర్వచనం పూర్ణాంకం (*sqlite3_loadext_నమోదు)(
  sqlite3 *db,                       /* Handle to the database. */
  అక్షరం **pz_Err_సందేశం,                   /* Used to set error string on failure. */
  స్థిరరాశి sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#ఒకవేళ !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_ఎక్కించు_EXTENSION)
#నిర్వచించు sqlite3_aggregate_context      sqlite3_api->aggregate_context
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
#నిర్వచించు sqlite3_aggregate_లెక్క        sqlite3_api->aggregate_లెక్క
#అయితే_ముగింపు
#నిర్వచించు sqlite3_bind_blob              sqlite3_api->bind_blob
#నిర్వచించు sqlite3_bind_double            sqlite3_api->bind_double
#నిర్వచించు sqlite3_bind_int               sqlite3_api->bind_int
#నిర్వచించు sqlite3_bind_int64             sqlite3_api->bind_int64
#నిర్వచించు sqlite3_bind_null              sqlite3_api->bind_null
#నిర్వచించు sqlite3_bind_parameter_లెక్క   sqlite3_api->bind_parameter_లెక్క
#నిర్వచించు sqlite3_bind_parameter_సూచీ   sqlite3_api->bind_parameter_సూచీ
#నిర్వచించు sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#నిర్వచించు sqlite3_bind_పాఠ్యం              sqlite3_api->bind_పాఠ్యం
#నిర్వచించు sqlite3_bind_text16            sqlite3_api->bind_text16
#నిర్వచించు sqlite3_bind_విలువ             sqlite3_api->bind_విలువ
#నిర్వచించు sqlite3_busy_handler           sqlite3_api->busy_handler
#నిర్వచించు sqlite3_busy_timeout           sqlite3_api->busy_timeout
#నిర్వచించు sqlite3_changes                sqlite3_api->changes
#నిర్వచించు sqlite3_close                  sqlite3_api->close
#నిర్వచించు sqlite3_collation_needed       sqlite3_api->collation_needed
#నిర్వచించు sqlite3_collation_needed16     sqlite3_api->collation_needed16
#నిర్వచించు sqlite3_స్తంభం_blob            sqlite3_api->స్తంభం_blob
#నిర్వచించు sqlite3_స్తంభం_బైట్లు           sqlite3_api->స్తంభం_బైట్లు
#నిర్వచించు sqlite3_స్తంభం_bytes16         sqlite3_api->స్తంభం_bytes16
#నిర్వచించు sqlite3_స్తంభం_లెక్క           sqlite3_api->స్తంభం_లెక్క
#నిర్వచించు sqlite3_స్తంభం_database_name   sqlite3_api->స్తంభం_database_name
#నిర్వచించు sqlite3_స్తంభం_database_name16 sqlite3_api->స్తంభం_database_name16
#నిర్వచించు sqlite3_స్తంభం_decltype        sqlite3_api->స్తంభం_decltype
#నిర్వచించు sqlite3_స్తంభం_decltype16      sqlite3_api->స్తంభం_decltype16
#నిర్వచించు sqlite3_స్తంభం_double          sqlite3_api->స్తంభం_double
#నిర్వచించు sqlite3_స్తంభం_int             sqlite3_api->స్తంభం_int
#నిర్వచించు sqlite3_స్తంభం_int64           sqlite3_api->స్తంభం_int64
#నిర్వచించు sqlite3_స్తంభం_name            sqlite3_api->స్తంభం_name
#నిర్వచించు sqlite3_స్తంభం_name16          sqlite3_api->స్తంభం_name16
#నిర్వచించు sqlite3_స్తంభం_origin_name     sqlite3_api->స్తంభం_origin_name
#నిర్వచించు sqlite3_స్తంభం_origin_name16   sqlite3_api->స్తంభం_origin_name16
#నిర్వచించు sqlite3_స్తంభం_పట్టిక_name      sqlite3_api->స్తంభం_పట్టిక_name
#నిర్వచించు sqlite3_స్తంభం_పట్టిక_name16    sqlite3_api->స్తంభం_పట్టిక_name16
#నిర్వచించు sqlite3_స్తంభం_పాఠ్యం            sqlite3_api->స్తంభం_పాఠ్యం
#నిర్వచించు sqlite3_స్తంభం_text16          sqlite3_api->స్తంభం_text16
#నిర్వచించు sqlite3_స్తంభం_type            sqlite3_api->స్తంభం_type
#నిర్వచించు sqlite3_స్తంభం_విలువ           sqlite3_api->స్తంభం_విలువ
#నిర్వచించు sqlite3_commit_hook            sqlite3_api->commit_hook
#నిర్వచించు sqlite3_complete               sqlite3_api->complete
#నిర్వచించు sqlite3_complete16             sqlite3_api->complete16
#నిర్వచించు sqlite3_create_collation       sqlite3_api->create_collation
#నిర్వచించు sqlite3_create_collation16     sqlite3_api->create_collation16
#నిర్వచించు sqlite3_create_function        sqlite3_api->create_function
#నిర్వచించు sqlite3_create_function16      sqlite3_api->create_function16
#నిర్వచించు sqlite3_create_module          sqlite3_api->create_module
#నిర్వచించు sqlite3_create_module_v2       sqlite3_api->create_module_v2
#నిర్వచించు sqlite3_దత్తాంశం_లెక్క             sqlite3_api->దత్తాంశం_లెక్క
#నిర్వచించు sqlite3_db_handle              sqlite3_api->db_handle
#నిర్వచించు sqlite3_declare_vtab           sqlite3_api->declare_vtab
#నిర్వచించు sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#నిర్వచించు sqlite3_errcode                sqlite3_api->errcode
#నిర్వచించు sqlite3_errmsg                 sqlite3_api->errmsg
#నిర్వచించు sqlite3_errmsg16               sqlite3_api->errmsg16
#నిర్వచించు sqlite3_exec                   sqlite3_api->exec
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
#నిర్వచించు sqlite3_expired                sqlite3_api->expired
#అయితే_ముగింపు
#నిర్వచించు sqlite3_finalize               sqlite3_api->finalize
#నిర్వచించు sqlite3_free                   sqlite3_api->విడుదల
#నిర్వచించు sqlite3_free_పట్టిక             sqlite3_api->free_పట్టిక
#నిర్వచించు sqlite3_get_autocommit         sqlite3_api->get_autocommit
#నిర్వచించు sqlite3_get_auxdata            sqlite3_api->get_auxdata
#నిర్వచించు sqlite3_get_పట్టిక              sqlite3_api->get_పట్టిక
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
#నిర్వచించు sqlite3_global_recover         sqlite3_api->global_recover
#అయితే_ముగింపు
#నిర్వచించు sqlite3_అంతరాయం              sqlite3_api->interruptx
#నిర్వచించు sqlite3_చివరి_చొప్పించు_rowid      sqlite3_api->చివరి_చొప్పించు_rowid
#నిర్వచించు sqlite3_libversion             sqlite3_api->libversion
#నిర్వచించు sqlite3_libversion_సంఖ్య      sqlite3_api->libversion_సంఖ్య
#నిర్వచించు sqlite3_malloc                 sqlite3_api->జ్ఞాపకం_ఇవ్వు
#నిర్వచించు sqlite3_mprintf                sqlite3_api->mprintf
#నిర్వచించు sqlite3_open                   sqlite3_api->open
#నిర్వచించు sqlite3_open16                 sqlite3_api->open16
#నిర్వచించు sqlite3_prepare                sqlite3_api->prepare
#నిర్వచించు sqlite3_prepare16              sqlite3_api->prepare16
#నిర్వచించు sqlite3_prepare_v2             sqlite3_api->prepare_v2
#నిర్వచించు sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#నిర్వచించు sqlite3_profile                sqlite3_api->profile
#నిర్వచించు sqlite3_పురోగతి_handler       sqlite3_api->పురోగతి_handler
#నిర్వచించు sqlite3_realloc                sqlite3_api->realloc
#నిర్వచించు sqlite3_reset                  sqlite3_api->reset
#నిర్వచించు sqlite3_ఫలితం_blob            sqlite3_api->ఫలితం_blob
#నిర్వచించు sqlite3_ఫలితం_double          sqlite3_api->ఫలితం_double
#నిర్వచించు sqlite3_ఫలితం_error           sqlite3_api->ఫలితం_error
#నిర్వచించు sqlite3_ఫలితం_error16         sqlite3_api->ఫలితం_error16
#నిర్వచించు sqlite3_ఫలితం_int             sqlite3_api->ఫలితం_int
#నిర్వచించు sqlite3_ఫలితం_int64           sqlite3_api->ఫలితం_int64
#నిర్వచించు sqlite3_ఫలితం_null            sqlite3_api->ఫలితం_null
#నిర్వచించు sqlite3_ఫలితం_పాఠ్యం            sqlite3_api->ఫలితం_పాఠ్యం
#నిర్వచించు sqlite3_ఫలితం_text16          sqlite3_api->ఫలితం_text16
#నిర్వచించు sqlite3_ఫలితం_text16be        sqlite3_api->ఫలితం_text16be
#నిర్వచించు sqlite3_ఫలితం_text16le        sqlite3_api->ఫలితం_text16le
#నిర్వచించు sqlite3_ఫలితం_విలువ           sqlite3_api->ఫలితం_విలువ
#నిర్వచించు sqlite3_rollback_hook          sqlite3_api->rollback_hook
#నిర్వచించు sqlite3_సమితి_authorizer         sqlite3_api->సమితి_authorizer
#నిర్వచించు sqlite3_సమితి_auxdata            sqlite3_api->సమితి_auxdata
#నిర్వచించు sqlite3_snprintf               sqlite3_api->xsnprintf
#నిర్వచించు sqlite3_అడుగు                   sqlite3_api->అడుగు
#నిర్వచించు sqlite3_పట్టిక_స్తంభం_metadata  sqlite3_api->పట్టిక_స్తంభం_metadata
#నిర్వచించు sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#నిర్వచించు sqlite3_మొత్తం_changes          sqlite3_api->మొత్తం_changes
#నిర్వచించు sqlite3_trace                  sqlite3_api->trace
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
#నిర్వచించు sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#అయితే_ముగింపు
#నిర్వచించు sqlite3_update_hook            sqlite3_api->update_hook
#నిర్వచించు sqlite3_user_దత్తాంశం              sqlite3_api->user_దత్తాంశం
#నిర్వచించు sqlite3_విలువ_blob             sqlite3_api->విలువ_blob
#నిర్వచించు sqlite3_విలువ_బైట్లు            sqlite3_api->విలువ_బైట్లు
#నిర్వచించు sqlite3_విలువ_bytes16          sqlite3_api->విలువ_bytes16
#నిర్వచించు sqlite3_విలువ_double           sqlite3_api->విలువ_double
#నిర్వచించు sqlite3_విలువ_int              sqlite3_api->విలువ_int
#నిర్వచించు sqlite3_విలువ_int64            sqlite3_api->విలువ_int64
#నిర్వచించు sqlite3_విలువ_numeric_type     sqlite3_api->విలువ_numeric_type
#నిర్వచించు sqlite3_విలువ_పాఠ్యం             sqlite3_api->విలువ_పాఠ్యం
#నిర్వచించు sqlite3_విలువ_text16           sqlite3_api->విలువ_text16
#నిర్వచించు sqlite3_విలువ_text16be         sqlite3_api->విలువ_text16be
#నిర్వచించు sqlite3_విలువ_text16le         sqlite3_api->విలువ_text16le
#నిర్వచించు sqlite3_విలువ_type             sqlite3_api->విలువ_type
#నిర్వచించు sqlite3_vmprintf               sqlite3_api->vmprintf
#నిర్వచించు sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#నిర్వచించు sqlite3_overload_function      sqlite3_api->overload_function
#నిర్వచించు sqlite3_prepare_v2             sqlite3_api->prepare_v2
#నిర్వచించు sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#నిర్వచించు sqlite3_clear_bindings         sqlite3_api->clear_bindings
#నిర్వచించు sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#నిర్వచించు sqlite3_blob_బైట్లు             sqlite3_api->blob_బైట్లు
#నిర్వచించు sqlite3_blob_close             sqlite3_api->blob_close
#నిర్వచించు sqlite3_blob_open              sqlite3_api->blob_open
#నిర్వచించు sqlite3_blob_read              sqlite3_api->blob_read
#నిర్వచించు sqlite3_blob_write             sqlite3_api->blob_write
#నిర్వచించు sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#నిర్వచించు sqlite3_file_control           sqlite3_api->file_control
#నిర్వచించు sqlite3_జ్ఞాపకం_highwater       sqlite3_api->జ్ఞాపకం_highwater
#నిర్వచించు sqlite3_జ్ఞాపకం_వాడిన            sqlite3_api->జ్ఞాపకం_వాడిన
#నిర్వచించు sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#నిర్వచించు sqlite3_mutex_enter            sqlite3_api->mutex_enter
#నిర్వచించు sqlite3_mutex_free             sqlite3_api->mutex_free
#నిర్వచించు sqlite3_mutex_leave            sqlite3_api->mutex_leave
#నిర్వచించు sqlite3_mutex_try              sqlite3_api->mutex_try
#నిర్వచించు sqlite3_open_v2                sqlite3_api->open_v2
#నిర్వచించు sqlite3_release_జ్ఞాపకం         sqlite3_api->release_జ్ఞాపకం
#నిర్వచించు sqlite3_ఫలితం_error_nomem     sqlite3_api->ఫలితం_error_nomem
#నిర్వచించు sqlite3_ఫలితం_error_toobig    sqlite3_api->ఫలితం_error_toobig
#నిర్వచించు sqlite3_sleep                  sqlite3_api->sleep
#నిర్వచించు sqlite3_soft_కుప్ప_హద్దు        sqlite3_api->soft_కుప్ప_హద్దు
#నిర్వచించు sqlite3_vfs_కనుగొను               sqlite3_api->vfs_కనుగొను
#నిర్వచించు sqlite3_vfs_రిజిస్టరు           sqlite3_api->vfs_రిజిస్టరు
#నిర్వచించు sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#నిర్వచించు sqlite3_threadsafe             sqlite3_api->xthreadsafe
#నిర్వచించు sqlite3_ఫలితం_zeroblob        sqlite3_api->ఫలితం_zeroblob
#నిర్వచించు sqlite3_ఫలితం_error_సంకేతం      sqlite3_api->ఫలితం_error_సంకేతం
#నిర్వచించు sqlite3_test_control           sqlite3_api->test_control
#నిర్వచించు sqlite3_randomness             sqlite3_api->randomness
#నిర్వచించు sqlite3_context_db_handle      sqlite3_api->context_db_handle
#నిర్వచించు sqlite3_extended_ఫలితం_codes  sqlite3_api->extended_ఫలితం_codes
#నిర్వచించు sqlite3_హద్దు                  sqlite3_api->హద్దు
#నిర్వచించు sqlite3_తదుపరి_stmt              sqlite3_api->తదుపరి_stmt
#నిర్వచించు sqlite3_sql                    sqlite3_api->sql
#నిర్వచించు sqlite3_స్థితి                 sqlite3_api->స్థితి_2
#నిర్వచించు sqlite3_backup_ముగింపు          sqlite3_api->backup_ముగింపు
#నిర్వచించు sqlite3_backup_init            sqlite3_api->backup_init
#నిర్వచించు sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#నిర్వచించు sqlite3_backup_మిగిలిన       sqlite3_api->backup_మిగిలిన
#నిర్వచించు sqlite3_backup_అడుగు            sqlite3_api->backup_అడుగు
#నిర్వచించు sqlite3_compileoption_get      sqlite3_api->compileoption_get
#నిర్వచించు sqlite3_compileoption_వాడిన     sqlite3_api->compileoption_వాడిన
#నిర్వచించు sqlite3_create_function_v2     sqlite3_api->create_function_v2
#నిర్వచించు sqlite3_db_config              sqlite3_api->db_config
#నిర్వచించు sqlite3_db_mutex               sqlite3_api->db_mutex
#నిర్వచించు sqlite3_db_స్థితి              sqlite3_api->db_స్థితి
#నిర్వచించు sqlite3_extended_errcode       sqlite3_api->extended_errcode
#నిర్వచించు sqlite3_log                    sqlite3_api->సంవర్గమానం
#నిర్వచించు sqlite3_soft_కుప్ప_limit64      sqlite3_api->soft_కుప్ప_limit64
#నిర్వచించు sqlite3_sourceid               sqlite3_api->sourceid
#నిర్వచించు sqlite3_stmt_స్థితి            sqlite3_api->stmt_స్థితి
#నిర్వచించు sqlite3_strnicmp               sqlite3_api->strnicmp
#నిర్వచించు sqlite3_unlock_notify          sqlite3_api->unlock_notify
#నిర్వచించు sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#నిర్వచించు sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#నిర్వచించు sqlite3_wal_hook               sqlite3_api->wal_hook
#నిర్వచించు sqlite3_blob_reopen            sqlite3_api->blob_reopen
#నిర్వచించు sqlite3_vtab_config            sqlite3_api->vtab_config
#నిర్వచించు sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#నిర్వచించు sqlite3_close_v2               sqlite3_api->close_v2
#నిర్వచించు sqlite3_db_filename            sqlite3_api->db_filename
#నిర్వచించు sqlite3_db_readonly            sqlite3_api->db_readonly
#నిర్వచించు sqlite3_db_release_జ్ఞాపకం      sqlite3_api->db_release_జ్ఞాపకం
#నిర్వచించు sqlite3_errstr                 sqlite3_api->errstr
#నిర్వచించు sqlite3_stmt_busy              sqlite3_api->stmt_busy
#నిర్వచించు sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#నిర్వచించు sqlite3_stricmp                sqlite3_api->stricmp
#నిర్వచించు sqlite3_uri_boolean            sqlite3_api->uri_boolean
#నిర్వచించు sqlite3_uri_int64              sqlite3_api->uri_int64
#నిర్వచించు sqlite3_uri_parameter          sqlite3_api->uri_parameter
#నిర్వచించు sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#నిర్వచించు sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#నిర్వచించు sqlite3_auto_extension         sqlite3_api->auto_extension
#నిర్వచించు sqlite3_bind_blob64            sqlite3_api->bind_blob64
#నిర్వచించు sqlite3_bind_text64            sqlite3_api->bind_text64
#నిర్వచించు sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#నిర్వచించు sqlite3_ఎక్కించు_extension         sqlite3_api->ఎక్కించు_extension
#నిర్వచించు sqlite3_malloc64               sqlite3_api->malloc64
#నిర్వచించు sqlite3_msize                  sqlite3_api->msize
#నిర్వచించు sqlite3_realloc64              sqlite3_api->realloc64
#నిర్వచించు sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#నిర్వచించు sqlite3_ఫలితం_blob64          sqlite3_api->ఫలితం_blob64
#నిర్వచించు sqlite3_ఫలితం_text64          sqlite3_api->ఫలితం_text64
#నిర్వచించు sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#నిర్వచించు sqlite3_విలువ_dup              sqlite3_api->విలువ_dup
#నిర్వచించు sqlite3_విలువ_free             sqlite3_api->విలువ_free
#నిర్వచించు sqlite3_ఫలితం_zeroblob64      sqlite3_api->ఫలితం_zeroblob64
#నిర్వచించు sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#నిర్వచించు sqlite3_విలువ_subtype          sqlite3_api->విలువ_subtype
#నిర్వచించు sqlite3_ఫలితం_subtype         sqlite3_api->ఫలితం_subtype
/* Version 3.10.0 and later */
#నిర్వచించు sqlite3_status64               sqlite3_api->status64
#నిర్వచించు sqlite3_strlike                sqlite3_api->strlike
#నిర్వచించు sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#నిర్వచించు sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#నిర్వచించు sqlite3_trace_v2               sqlite3_api->trace_v2
#నిర్వచించు sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#నిర్వచించు sqlite3_సమితి_చివరి_చొప్పించు_rowid  sqlite3_api->సమితి_చివరి_చొప్పించు_rowid
/* Version 3.20.0 and later */
#నిర్వచించు sqlite3_prepare_v3             sqlite3_api->prepare_v3
#నిర్వచించు sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#నిర్వచించు sqlite3_bind_సూచిక           sqlite3_api->bind_సూచిక
#నిర్వచించు sqlite3_ఫలితం_సూచిక         sqlite3_api->ఫలితం_సూచిక
#నిర్వచించు sqlite3_విలువ_సూచిక          sqlite3_api->విలువ_సూచిక
/* Version 3.22.0 and later */
#నిర్వచించు sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#నిర్వచించు sqlite3_విలువ_nochange         sqlite3_api->విలువ_nochange
#నిర్వచించు sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#నిర్వచించు sqlite3_keyword_లెక్క          sqlite3_api->keyword_లెక్క
#నిర్వచించు sqlite3_keyword_name           sqlite3_api->keyword_name
#నిర్వచించు sqlite3_keyword_check          sqlite3_api->keyword_check
#నిర్వచించు sqlite3_str_new                sqlite3_api->str_new
#నిర్వచించు sqlite3_str_ముగింపు             sqlite3_api->str_ముగింపు
#నిర్వచించు sqlite3_str_appendf            sqlite3_api->str_appendf
#నిర్వచించు sqlite3_str_vappendf           sqlite3_api->str_vappendf
#నిర్వచించు sqlite3_str_append             sqlite3_api->str_append
#నిర్వచించు sqlite3_str_appendall          sqlite3_api->str_appendall
#నిర్వచించు sqlite3_str_appendchar         sqlite3_api->str_appendchar
#నిర్వచించు sqlite3_str_reset              sqlite3_api->str_reset
#నిర్వచించు sqlite3_str_errcode            sqlite3_api->str_errcode
#నిర్వచించు sqlite3_str_నిడివి             sqlite3_api->str_నిడివి
#నిర్వచించు sqlite3_str_విలువ              sqlite3_api->str_విలువ
/* Version 3.25.0 and later */
#నిర్వచించు sqlite3_create_కిటికీ_function sqlite3_api->create_కిటికీ_function
/* Version 3.26.0 and later */
#నిర్వచించు sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#నిర్వచించు sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#నిర్వచించు sqlite3_విలువ_frombind         sqlite3_api->విలువ_frombind
/* Version 3.30.0 and later */
#నిర్వచించు sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#నిర్వచించు sqlite3_hard_కుప్ప_limit64      sqlite3_api->hard_కుప్ప_limit64
#నిర్వచించు sqlite3_uri_కీలకం                sqlite3_api->uri_కీలకం
#నిర్వచించు sqlite3_filename_database      sqlite3_api->filename_database
#నిర్వచించు sqlite3_filename_journal       sqlite3_api->filename_journal
#నిర్వచించు sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#నిర్వచించు sqlite3_create_filename        sqlite3_api->create_filename
#నిర్వచించు sqlite3_free_filename          sqlite3_api->free_filename
#నిర్వచించు sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#నిర్వచించు sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#నిర్వచించు sqlite3_changes64              sqlite3_api->changes64
#నిర్వచించు sqlite3_మొత్తం_changes64        sqlite3_api->మొత్తం_changes64
/* Version 3.37.0 and later */
#నిర్వచించు sqlite3_autovacuum_పుటలు       sqlite3_api->autovacuum_పుటలు
/* Version 3.38.0 and later */
#నిర్వచించు sqlite3_error_స్థానభ్రంశం           sqlite3_api->error_స్థానభ్రంశం
#నిర్వచించు sqlite3_vtab_rhs_విలువ         sqlite3_api->vtab_rhs_విలువ
#నిర్వచించు sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#నిర్వచించు sqlite3_vtab_లోపల                sqlite3_api->vtab_లోపల
#నిర్వచించు sqlite3_vtab_లోపల_మొదటి          sqlite3_api->vtab_లోపల_మొదటి
#నిర్వచించు sqlite3_vtab_లోపల_తదుపరి           sqlite3_api->vtab_లోపల_తదుపరి
/* Version 3.39.0 and later */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DESERIALIZE
#నిర్వచించు sqlite3_deserialize            sqlite3_api->deserialize
#నిర్వచించు sqlite3_serialize              sqlite3_api->serialize
#అయితే_ముగింపు
#నిర్వచించు sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#నిర్వచించు sqlite3_విలువ_encoding         sqlite3_api->విలువ_encoding
/* Version 3.41.0 and later */
#నిర్వచించు sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#నిర్వచించు sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#నిర్వచించు sqlite3_get_clientdata         sqlite3_api->get_clientdata
#నిర్వచించు sqlite3_సమితి_clientdata         sqlite3_api->సమితి_clientdata
/* Version 3.50.0 and later */
#నిర్వచించు sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#నిర్వచించు sqlite3_సమితి_errmsg             sqlite3_api->సమితి_errmsg
#నిర్వచించు sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#నిర్వచించు sqlite3_str_truncate           sqlite3_api->str_truncate
#నిర్వచించు sqlite3_str_free               sqlite3_api->str_free
#నిర్వచించు sqlite3_carray_bind            sqlite3_api->carray_bind
#నిర్వచించు sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#నిర్వచించు sqlite3_incomplete             sqlite3_api->incomplete
#నిర్వచించు sqlite3_ఫలితం_str             sqlite3_api->ఫలితం_str
#అయితే_ముగింపు /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#ఒకవేళ !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_ఎక్కించు_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     స్థిరరాశి sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    బాహ్య స్థిరరాశి sqlite3_api_routines *sqlite3_api;
#లేకపోతే
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (శూన్య)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#అయితే_ముగింపు

#అయితే_ముగింపు /* SQLITE3EXT_H */
