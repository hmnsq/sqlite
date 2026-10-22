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

#నిర్వచితం_కాకపోతే SQLITE_CORE
  #నిర్వచించు SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#అయితే_ముగింపు
#అన్తర్భావ "sqlite3ext.h"
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
/*
** Some API routines are omitted when various features are
** excluded from a build of SQLite.  Substitute a NULL pointer
** for any missing APIs.
*/
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_స్తంభం_METADATA
# define sqlite3_స్తంభం_database_name   0
# define sqlite3_స్తంభం_database_name16 0
# define sqlite3_స్తంభం_పట్టిక_name      0
# define sqlite3_స్తంభం_పట్టిక_name16    0
# define sqlite3_స్తంభం_origin_name     0
# define sqlite3_స్తంభం_origin_name16   0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_AUTHORIZATION
# define sqlite3_సమితి_authorizer         0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_UTF16
# define sqlite3_bind_text16            0
# define sqlite3_collation_needed16     0
# define sqlite3_స్తంభం_decltype16      0
# define sqlite3_స్తంభం_name16          0
# define sqlite3_స్తంభం_text16          0
# define sqlite3_complete16             0
# define sqlite3_create_collation16     0
# define sqlite3_create_function16      0
# define sqlite3_errmsg16               0
# define sqlite3_open16                 0
# define sqlite3_prepare16              0
# define sqlite3_prepare16_v2           0
# define sqlite3_prepare16_v3           0
# define sqlite3_ఫలితం_error16         0
# define sqlite3_ఫలితం_text16          0
# define sqlite3_ఫలితం_text16be        0
# define sqlite3_ఫలితం_text16le        0
# define sqlite3_విలువ_text16           0
# define sqlite3_విలువ_text16be         0
# define sqlite3_విలువ_text16le         0
# define sqlite3_స్తంభం_database_name16 0
# define sqlite3_స్తంభం_పట్టిక_name16    0
# define sqlite3_స్తంభం_origin_name16   0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_COMPLETE
# define sqlite3_complete 0
# define sqlite3_complete16 0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_DECLTYPE
# define sqlite3_స్తంభం_decltype16      0
# define sqlite3_స్తంభం_decltype        0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_పురోగతి_CALLBACK
# define sqlite3_పురోగతి_handler 0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_VIRTUALTABLE
# define sqlite3_create_module 0
# define sqlite3_create_module_v2 0
# define sqlite3_declare_vtab 0
# define sqlite3_vtab_config 0
# define sqlite3_vtab_on_conflict 0
# define sqlite3_vtab_collation 0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_SHARED_CACHE
# define sqlite3_enable_shared_cache 0
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_OMIT_TRACE) || defined(SQLITE_OMIT_DEPRECATED)
# define sqlite3_profile       0
# define sqlite3_trace         0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_GET_పట్టిక
# define sqlite3_free_పట్టిక    0
# define sqlite3_get_పట్టిక     0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_INCRBLOB
#నిర్వచించు sqlite3_bind_zeroblob  0
#నిర్వచించు sqlite3_blob_బైట్లు     0
#నిర్వచించు sqlite3_blob_close     0
#నిర్వచించు sqlite3_blob_open      0
#నిర్వచించు sqlite3_blob_read      0
#నిర్వచించు sqlite3_blob_write     0
#నిర్వచించు sqlite3_blob_reopen    0
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_OMIT_TRACE)
# define sqlite3_trace_v2      0
#అయితే_ముగింపు

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
స్థిర స్థిరరాశి sqlite3_api_routines sqlite3Apis = {
  sqlite3_aggregate_context,
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  sqlite3_aggregate_లెక్క,
#లేకపోతే
  0,
#అయితే_ముగింపు
  sqlite3_bind_blob,
  sqlite3_bind_double,
  sqlite3_bind_int,
  sqlite3_bind_int64,
  sqlite3_bind_null,
  sqlite3_bind_parameter_లెక్క,
  sqlite3_bind_parameter_సూచీ,
  sqlite3_bind_parameter_name,
  sqlite3_bind_పాఠ్యం,
  sqlite3_bind_text16,
  sqlite3_bind_విలువ,
  sqlite3_busy_handler,
  sqlite3_busy_timeout,
  sqlite3_changes,
  sqlite3_close,
  sqlite3_collation_needed,
  sqlite3_collation_needed16,
  sqlite3_స్తంభం_blob,
  sqlite3_స్తంభం_బైట్లు,
  sqlite3_స్తంభం_bytes16,
  sqlite3_స్తంభం_లెక్క,
  sqlite3_స్తంభం_database_name,
  sqlite3_స్తంభం_database_name16,
  sqlite3_స్తంభం_decltype,
  sqlite3_స్తంభం_decltype16,
  sqlite3_స్తంభం_double,
  sqlite3_స్తంభం_int,
  sqlite3_స్తంభం_int64,
  sqlite3_స్తంభం_name,
  sqlite3_స్తంభం_name16,
  sqlite3_స్తంభం_origin_name,
  sqlite3_స్తంభం_origin_name16,
  sqlite3_స్తంభం_పట్టిక_name,
  sqlite3_స్తంభం_పట్టిక_name16,
  sqlite3_స్తంభం_పాఠ్యం,
  sqlite3_స్తంభం_text16,
  sqlite3_స్తంభం_type,
  sqlite3_స్తంభం_విలువ,
  sqlite3_commit_hook,
  sqlite3_complete,
  sqlite3_complete16,
  sqlite3_create_collation,
  sqlite3_create_collation16,
  sqlite3_create_function,
  sqlite3_create_function16,
  sqlite3_create_module,
  sqlite3_దత్తాంశం_లెక్క,
  sqlite3_db_handle,
  sqlite3_declare_vtab,
  sqlite3_enable_shared_cache,
  sqlite3_errcode,
  sqlite3_errmsg,
  sqlite3_errmsg16,
  sqlite3_exec,
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  sqlite3_expired,
#లేకపోతే
  0,
#అయితే_ముగింపు
  sqlite3_finalize,
  sqlite3_free,
  sqlite3_free_పట్టిక,
  sqlite3_get_autocommit,
  sqlite3_get_auxdata,
  sqlite3_get_పట్టిక,
  0,     /* Was sqlite3_global_recover(), but that function is deprecated */
  sqlite3_అంతరాయం,
  sqlite3_చివరి_చొప్పించు_rowid,
  sqlite3_libversion,
  sqlite3_libversion_సంఖ్య,
  sqlite3_malloc,
  sqlite3_mprintf,
  sqlite3_open,
  sqlite3_open16,
  sqlite3_prepare,
  sqlite3_prepare16,
  sqlite3_profile,
  sqlite3_పురోగతి_handler,
  sqlite3_realloc,
  sqlite3_reset,
  sqlite3_ఫలితం_blob,
  sqlite3_ఫలితం_double,
  sqlite3_ఫలితం_error,
  sqlite3_ఫలితం_error16,
  sqlite3_ఫలితం_int,
  sqlite3_ఫలితం_int64,
  sqlite3_ఫలితం_null,
  sqlite3_ఫలితం_పాఠ్యం,
  sqlite3_ఫలితం_text16,
  sqlite3_ఫలితం_text16be,
  sqlite3_ఫలితం_text16le,
  sqlite3_ఫలితం_విలువ,
  sqlite3_rollback_hook,
  sqlite3_సమితి_authorizer,
  sqlite3_సమితి_auxdata,
  sqlite3_snprintf,
  sqlite3_అడుగు,
  sqlite3_పట్టిక_స్తంభం_metadata,
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup,
#లేకపోతే
  0,
#అయితే_ముగింపు
  sqlite3_మొత్తం_changes,
  sqlite3_trace,
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  sqlite3_transfer_bindings,
#లేకపోతే
  0,
#అయితే_ముగింపు
  sqlite3_update_hook,
  sqlite3_user_దత్తాంశం,
  sqlite3_విలువ_blob,
  sqlite3_విలువ_బైట్లు,
  sqlite3_విలువ_bytes16,
  sqlite3_విలువ_double,
  sqlite3_విలువ_int,
  sqlite3_విలువ_int64,
  sqlite3_విలువ_numeric_type,
  sqlite3_విలువ_పాఠ్యం,
  sqlite3_విలువ_text16,
  sqlite3_విలువ_text16be,
  sqlite3_విలువ_text16le,
  sqlite3_విలువ_type,
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
  sqlite3_blob_బైట్లు,
  sqlite3_blob_close,
  sqlite3_blob_open,
  sqlite3_blob_read,
  sqlite3_blob_write,
  sqlite3_create_collation_v2,
  sqlite3_file_control,
  sqlite3_జ్ఞాపకం_highwater,
  sqlite3_జ్ఞాపకం_వాడిన,
#నిర్వచితమైతే SQLITE_MUTEX_OMIT
  0, 
  0, 
  0,
  0,
  0,
#లేకపోతే
  sqlite3_mutex_alloc,
  sqlite3_mutex_enter,
  sqlite3_mutex_free,
  sqlite3_mutex_leave,
  sqlite3_mutex_try,
#అయితే_ముగింపు
  sqlite3_open_v2,
  sqlite3_release_జ్ఞాపకం,
  sqlite3_ఫలితం_error_nomem,
  sqlite3_ఫలితం_error_toobig,
  sqlite3_sleep,
  sqlite3_soft_కుప్ప_హద్దు,
  sqlite3_vfs_కనుగొను,
  sqlite3_vfs_రిజిస్టరు,
  sqlite3_vfs_unregister,

  /*
  ** Added for 3.5.8
  */
  sqlite3_threadsafe,
  sqlite3_ఫలితం_zeroblob,
  sqlite3_ఫలితం_error_సంకేతం,
  sqlite3_test_control,
  sqlite3_randomness,
  sqlite3_context_db_handle,

  /*
  ** Added for 3.6.0
  */
  sqlite3_extended_ఫలితం_codes,
  sqlite3_హద్దు,
  sqlite3_తదుపరి_stmt,
  sqlite3_sql,
  sqlite3_స్థితి,

  /*
  ** Added for 3.7.4
  */
  sqlite3_backup_ముగింపు,
  sqlite3_backup_init,
  sqlite3_backup_pagecount,
  sqlite3_backup_మిగిలిన,
  sqlite3_backup_అడుగు,
#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPILEOPTION_DIAGS
  sqlite3_compileoption_get,
  sqlite3_compileoption_వాడిన,
#లేకపోతే
  0,
  0,
#అయితే_ముగింపు
  sqlite3_create_function_v2,
  sqlite3_db_config,
  sqlite3_db_mutex,
  sqlite3_db_స్థితి,
  sqlite3_extended_errcode,
  sqlite3_log,
  sqlite3_soft_కుప్ప_limit64,
  sqlite3_sourceid,
  sqlite3_stmt_స్థితి,
  sqlite3_strnicmp,
#నిర్వచితమైతే SQLITE_ENABLE_UNLOCK_NOTIFY
  sqlite3_unlock_notify,
#లేకపోతే
  0,
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  sqlite3_wal_autocheckpoint,
  sqlite3_wal_checkpoint,
  sqlite3_wal_hook,
#లేకపోతే
  0,
  0,
  0,
#అయితే_ముగింపు
  sqlite3_blob_reopen,
  sqlite3_vtab_config,
  sqlite3_vtab_on_conflict,
  sqlite3_close_v2,
  sqlite3_db_filename,
  sqlite3_db_readonly,
  sqlite3_db_release_జ్ఞాపకం,
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
  sqlite3_ఎక్కించు_extension,
  sqlite3_malloc64,
  sqlite3_msize,
  sqlite3_realloc64,
  sqlite3_reset_auto_extension,
  sqlite3_ఫలితం_blob64,
  sqlite3_ఫలితం_text64,
  sqlite3_strglob,
  /* Version 3.8.11 and later */
  (sqlite3_విలువ*(*)(స్థిరరాశి sqlite3_విలువ*))sqlite3_విలువ_dup,
  sqlite3_విలువ_free,
  sqlite3_ఫలితం_zeroblob64,
  sqlite3_bind_zeroblob64,
  /* Version 3.9.0 and later */
  sqlite3_విలువ_subtype,
  sqlite3_ఫలితం_subtype,
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
  sqlite3_సమితి_చివరి_చొప్పించు_rowid,
  /* Version 3.20.0 and later */
  sqlite3_prepare_v3,
  sqlite3_prepare16_v3,
  sqlite3_bind_సూచిక,
  sqlite3_ఫలితం_సూచిక,
  sqlite3_విలువ_సూచిక,
  /* Version 3.22.0 and later */
  sqlite3_vtab_nochange,
  sqlite3_విలువ_nochange,
  sqlite3_vtab_collation,
  /* Version 3.24.0 and later */
  sqlite3_keyword_లెక్క,
  sqlite3_keyword_name,
  sqlite3_keyword_check,
  sqlite3_str_new,
  sqlite3_str_ముగింపు,
  sqlite3_str_appendf,
  sqlite3_str_vappendf,
  sqlite3_str_append,
  sqlite3_str_appendall,
  sqlite3_str_appendchar,
  sqlite3_str_reset,
  sqlite3_str_errcode,
  sqlite3_str_నిడివి,
  sqlite3_str_విలువ,
  /* Version 3.25.0 and later */
  sqlite3_create_కిటికీ_function,
  /* Version 3.26.0 and later */
#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
  sqlite3_normalized_sql,
#లేకపోతే
  0,
#అయితే_ముగింపు
  /* Version 3.28.0 and later */
  sqlite3_stmt_isexplain,
  sqlite3_విలువ_frombind,
  /* Version 3.30.0 and later */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3_drop_modules,
#లేకపోతే
  0,
#అయితే_ముగింపు
  /* Version 3.31.0 and later */
  sqlite3_hard_కుప్ప_limit64,
  sqlite3_uri_కీలకం,
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
  sqlite3_మొత్తం_changes64,
  /* Version 3.37.0 and later */
  sqlite3_autovacuum_పుటలు,
  /* Version 3.38.0 and later */
  sqlite3_error_స్థానభ్రంశం,
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3_vtab_rhs_విలువ,
  sqlite3_vtab_distinct,
  sqlite3_vtab_లోపల,
  sqlite3_vtab_లోపల_మొదటి,
  sqlite3_vtab_లోపల_తదుపరి,
#లేకపోతే
  0,
  0,
  0,
  0,
  0,
#అయితే_ముగింపు
  /* Version 3.39.0 and later */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DESERIALIZE
  sqlite3_deserialize,
  sqlite3_serialize,
#లేకపోతే
  0,
  0,
#అయితే_ముగింపు
  sqlite3_db_name,
  /* Version 3.40.0 and later */
  sqlite3_విలువ_encoding,
  /* Version 3.41.0 and later */
  sqlite3_is_interrupted,
  /* Version 3.43.0 and later */
  sqlite3_stmt_explain,
  /* Version 3.44.0 and later */
  sqlite3_get_clientdata,
  sqlite3_సమితి_clientdata,
  /* Version 3.50.0 and later */
  sqlite3_setlk_timeout,
  /* Version 3.51.0 and later */
  sqlite3_సమితి_errmsg,
  sqlite3_db_status64,
  /* Version 3.52.0 and later */
  sqlite3_str_truncate,
  sqlite3_str_free,
#నిర్వచితమైతే SQLITE_ENABLE_CARRAY
  sqlite3_carray_bind,
  sqlite3_carray_bind_v2,
#లేకపోతే
  0,
  0,
#అయితే_ముగింపు
  /* Version 3.54.0 and later */
  sqlite3_incomplete,
  sqlite3_ఫలితం_str
};

/* True if x is the directory separator character
*/
#ఒకవేళ SQLITE_OS_WIN
# define DirSep(X)  ((X)=='/'||(X)=='\\')
#లేకపోతే
# define DirSep(X)  ((X)=='/')
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం sqlite3_ఎక్కించు_Extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  స్థిరరాశి అక్షరం *zFile,    /* Name of the shared library containing extension */
  స్థిరరాశి అక్షరం *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  అక్షరం **pz_Err_సందేశం       /* Put error message here if not 0 */
){
  sqlite3_vfs *pVfs = db->pVfs;
  శూన్య *handle;
  sqlite3_loadext_నమోదు xInit;
  అక్షరం *zErrmsg = 0;
  స్థిరరాశి అక్షరం *z_నమోదు;
  అక్షరం *z_Alt_నమోదు = 0;
  శూన్య **aHandle;
  u64 n_సందేశం = అక్షరశ్రేణి_పొడవు(zFile);
  పూర్ణాంకం ii;
  పూర్ణాంకం rc;

  /* Shared library endings to try if zFile cannot be loaded as written */
  స్థిర స్థిరరాశి అక్షరం *azEndings[] = {
#ఒకవేళ SQLITE_OS_WIN
     "dll"   
#లేదావేళ defined(__APPLE__)
     "dylib"
#లేకపోతే
     "so"
#అయితే_ముగింపు
  };


  ఒకవేళ( pz_Err_సందేశం ) *pz_Err_సందేశం = 0;

  /* Ticket #1863.  To avoid a creating security problems for older
  ** applications that relink against newer versions of SQLite, the
  ** ability to run load_extension is turned off by default.  One
  ** must call either sqlite3_enable_load_extension(db) or
  ** sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, 0)
  ** to turn on extension loading.
  */
  ఒకవేళ( (db->flags & SQLITE_ఎక్కించు_Extension)==0 ){
    ఒకవేళ( pz_Err_సందేశం ){
      *pz_Err_సందేశం = sqlite3_mprintf("not authorized");
    }
    తిరిగి SQLITE_ERROR;
  }

  z_నమోదు = zProc ? zProc : "sqlite3_extension_init";

  /* tag-20210611-1.  Some dlopen() implementations will segfault if given
  ** an oversize filename.  Most filesystems have a pathname limit of 4K,
  ** so limit the extension filename length to about twice that.
  ** https://sqlite.org/forum/forumpost/08a0d6d9bf
  **
  ** Later (2023-03-25): Save an extra 6 bytes for the filename suffix.
  ** See https://sqlite.org/forum/forumpost/24083b579d.
  */
  ఒకవేళ( n_సందేశం>SQLITE_MAX_PATHLEN ) వెళ్ళు extension_not_దొరికింది;

  /* Do not allow sqlite3_load_extension() to link to a copy of the
  ** running application, by passing in an empty filename. */
  ఒకవేళ( n_సందేశం==0 ) వెళ్ళు extension_not_దొరికింది;
    
  handle = sqlite3OsDlOpen(pVfs, zFile);
#ఒకవేళ SQLITE_OS_UNIX || SQLITE_OS_WIN
  కొరకు(ii=0; ii<Array_కొలత(azEndings) && handle==0; ii++){
    అక్షరం *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    ఒకవేళ( zAltFile==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    ఒకవేళ( n_సందేశం+అక్షరశ్రేణి_పొడవు(azEndings[ii])+1<=SQLITE_MAX_PATHLEN ){
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }
#అయితే_ముగింపు
  ఒకవేళ( handle==0 ) వెళ్ళు extension_not_దొరికింది;
  xInit = (sqlite3_loadext_నమోదు)sqlite3OsDlSym(pVfs, handle, z_నమోదు);

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
  ఒకవేళ( xInit==0 && zProc==0 ){
    పూర్ణాంకం iFile, i_నమోదు, c;
    పూర్ణాంకం ncFile = sqlite3Strlen30(zFile);
    పూర్ణాంకం cnt = 0;
    z_Alt_నమోదు = sqlite3_malloc64(ncFile+30);
    ఒకవేళ( z_Alt_నమోదు==0 ){
      sqlite3OsDlClose(pVfs, handle);
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    చేయి{
      జ్ఞాపకం_నకలు(z_Alt_నమోదు, "sqlite3_", 8);
      కొరకు(iFile=ncFile-1; iFile>=0 && !DirSep(zFile[iFile]); iFile--){}
      iFile++;
      ఒకవేళ( sqlite3_strnicmp(zFile+iFile, "lib", 3)==0 ) iFile += 3;
      కొరకు(i_నమోదు=8; (c = zFile[iFile])!=0 && c!='.'; iFile++){
        ఒకవేళ( sqlite3Isalpha(c) || (cnt && sqlite3Isdigit(c)) ){
          z_Alt_నమోదు[i_నమోదు++] = (అక్షరం)sqlite3_పై_గమ్యం_క్రింది[(గుర్తులేని)c];
        }
      }
      జ్ఞాపకం_నకలు(z_Alt_నమోదు+i_నమోదు, "_init", 6);
      z_నమోదు = z_Alt_నమోదు;
      xInit = (sqlite3_loadext_నమోదు)sqlite3OsDlSym(pVfs, handle, z_నమోదు);
    }అయితే( xInit==0 && (++cnt)<2 );
  }
  ఒకవేళ( xInit==0 ){
    ఒకవేళ( pz_Err_సందేశం ){
      n_సందేశం += అక్షరశ్రేణి_పొడవు(z_నమోదు) + 300;
      *pz_Err_సందేశం = zErrmsg = sqlite3_malloc64(n_సందేశం);
      ఒకవేళ( zErrmsg ){
        ప్రతిజ్ఞా( n_సందేశం<0x7fffffff );  /* zErrmsg would be NULL if not so */
        sqlite3_snprintf((పూర్ణాంకం)n_సందేశం, zErrmsg,
            "no entry point [%s] in shared library [%s]", z_నమోదు, zFile);
        sqlite3OsDlError(pVfs, n_సందేశం-1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(z_Alt_నమోదు);
    తిరిగి SQLITE_ERROR;
  }
  sqlite3_free(z_Alt_నమోదు);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  ఒకవేళ( rc ){
    ఒకవేళ( rc==SQLITE_OK_ఎక్కించు_PERMANENTLY ) తిరిగి SQLITE_OK;
    ఒకవేళ( pz_Err_సందేశం ){
      *pz_Err_సందేశం = sqlite3_mprintf("error during initialization: %s", zErrmsg);
    }
    sqlite3_free(zErrmsg);
    sqlite3OsDlClose(pVfs, handle);
    తిరిగి SQLITE_ERROR;
  }

  /* Append the new shared library handle to the db->aExtension array. */
  aHandle = sqlite3DbMallocZero(db, పరిమాణం(handle)*(db->nExtension+1));
  ఒకవేళ( aHandle==0 ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  ఒకవేళ( db->nExtension>0 ){
    జ్ఞాపకం_నకలు(aHandle, db->aExtension, పరిమాణం(handle)*db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  తిరిగి SQLITE_OK;

extension_not_దొరికింది:
  ఒకవేళ( pz_Err_సందేశం ){
    n_సందేశం += 300;
    *pz_Err_సందేశం = zErrmsg = sqlite3_malloc64(n_సందేశం);
    ఒకవేళ( zErrmsg ){
      ప్రతిజ్ఞా( n_సందేశం<0x7fffffff );  /* zErrmsg would be NULL if not so */
      sqlite3_snprintf((పూర్ణాంకం)n_సందేశం, zErrmsg,
          "unable to open shared library [%.*s]", SQLITE_MAX_PATHLEN, zFile);
      sqlite3OsDlError(pVfs, n_సందేశం-1, zErrmsg);
    }
  }
  తిరిగి SQLITE_ERROR;
}
పూర్ణాంకం sqlite3_ఎక్కించు_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  స్థిరరాశి అక్షరం *zFile,    /* Name of the shared library containing extension */
  స్థిరరాశి అక్షరం *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  అక్షరం **pz_Err_సందేశం       /* Put error message here if not 0 */
){
  పూర్ణాంకం rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_ఎక్కించు_Extension(db, zFile, zProc, pz_Err_సందేశం);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Call this routine when the database connection is closing in order
** to clean up loaded extensions
*/
శూన్య sqlite3CloseExtensions(sqlite3 *db){
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  కొరకు(i=0; i<db->nExtension; i++){
    sqlite3OsDlClose(db->pVfs, db->aExtension[i]);
  }
  sqlite3DbFree(db, db->aExtension);
}

/*
** Enable or disable extension loading.  Extension loading is disabled by
** default so as not to open security holes in older applications.
*/
పూర్ణాంకం sqlite3_enable_ఎక్కించు_extension(sqlite3 *db, పూర్ణాంకం onoff){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( onoff ){
    db->flags |= SQLITE_ఎక్కించు_Extension|SQLITE_ఎక్కించు_Ext_Func;
  }లేకపోతే{
    db->flags &= ~(u64)(SQLITE_ఎక్కించు_Extension|SQLITE_ఎక్కించు_Ext_Func);
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి SQLITE_OK;
}

#అయితే_ముగింపు /* !defined(SQLITE_OMIT_LOAD_EXTENSION) */

/*
** The following object holds the list of automatically loaded
** extensions.
**
** This list is shared across threads.  The SQLITE_MUTEX_STATIC_MAIN
** mutex must be held while accessing this list.
*/
రకనిర్వచనం నిర్మాణం sqlite3AutoExtList sqlite3AutoExtList;
స్థిర SQLITE_WSD నిర్మాణం sqlite3AutoExtList {
  u32 nExt;              /* Number of entries in aExt[] */
  శూన్య (**aExt)(శూన్య);   /* Pointers to the extension init functions */
} sqlite3Autoext = { 0, 0 };

/* The "wsdAutoext" macro will resolve to the autoextension
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Autoext" state vector declared above.
*/
#నిర్వచితమైతే SQLITE_OMIT_WSD
# define wsdAutoextInit \
  sqlite3AutoExtList *x = &GLOBAL(sqlite3AutoExtList,sqlite3Autoext)
# define wsdAutoext x[0]
#లేకపోతే
# define wsdAutoextInit
# define wsdAutoext sqlite3Autoext
#అయితే_ముగింపు


/*
** Register a statically linked extension that is automatically
** loaded by every new database connection.
*/
పూర్ణాంకం sqlite3_auto_extension(
  శూన్య (*xInit)(శూన్య)
){
  పూర్ణాంకం rc = SQLITE_OK;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( xInit==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ఒకవేళ( rc ){
    తిరిగి rc;
  }లేకపోతే
#అయితే_ముగింపు
  {
    u32 i;
#ఒకవేళ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#అయితే_ముగింపు
    wsdAutoextInit;
    sqlite3_mutex_enter(mutex);
    కొరకు(i=0; i<wsdAutoext.nExt; i++){
      ఒకవేళ( wsdAutoext.aExt[i]==xInit ) ఆపు;
    }
    ఒకవేళ( i==wsdAutoext.nExt ){
      u64 nByte = (wsdAutoext.nExt+1)*పరిమాణం(wsdAutoext.aExt[0]);
      శూన్య (**aNew)(శూన్య);
      aNew = sqlite3_realloc64(wsdAutoext.aExt, nByte);
      ఒకవేళ( aNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }లేకపోతే{
        wsdAutoext.aExt = aNew;
        wsdAutoext.aExt[wsdAutoext.nExt] = xInit;
        wsdAutoext.nExt++;
      }
    }
    sqlite3_mutex_leave(mutex);
    ప్రతిజ్ఞా( (rc&0xff)==rc );
    తిరిగి rc;
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
పూర్ణాంకం sqlite3_cancel_auto_extension(
  శూన్య (*xInit)(శూన్య)
){
  పూర్ణాంకం n = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ){
    తిరిగి 0;
  }లేకపోతే
#అయితే_ముగింపు
  {
#ఒకవేళ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#అయితే_ముగింపు
    పూర్ణాంకం i;
    wsdAutoextInit;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
    ఒకవేళ( xInit==0 ) తిరిగి 0;
#అయితే_ముగింపు
    sqlite3_mutex_enter(mutex);
    కొరకు(i=(పూర్ణాంకం)wsdAutoext.nExt-1; i>=0; i--){
      ఒకవేళ( wsdAutoext.aExt[i]==xInit ){
        wsdAutoext.nExt--;
        wsdAutoext.aExt[i] = wsdAutoext.aExt[wsdAutoext.nExt];
        n++;
        ఆపు;
      }
    }
    sqlite3_mutex_leave(mutex);
  }
  తిరిగి n;
}

/*
** Reset the automatic extension loading mechanism.
*/
శూన్య sqlite3_reset_auto_extension(శూన్య){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize()==SQLITE_OK )
#అయితే_ముగింపు
  {
#ఒకవేళ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#అయితే_ముగింపు
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
శూన్య sqlite3_Auto_ఎక్కించు_Extensions(sqlite3 *db){
  u32 i;
  పూర్ణాంకం go = 1;
  పూర్ణాంకం rc;
  sqlite3_loadext_నమోదు xInit;

  wsdAutoextInit;
  ఒకవేళ( wsdAutoext.nExt==0 ){
    /* Common case: early out without every having to acquire a mutex */
    తిరిగి;
  }
  కొరకు(i=0; go; i++){
    అక్షరం *zErrmsg;
#ఒకవేళ SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_OMIT_ఎక్కించు_EXTENSION
    స్థిరరాశి sqlite3_api_routines *pThunk = 0;
#లేకపోతే
    స్థిరరాశి sqlite3_api_routines *pThunk = &sqlite3Apis;
#అయితే_ముగింపు
    sqlite3_mutex_enter(mutex);
    ఒకవేళ( i>=wsdAutoext.nExt ){
      xInit = 0;
      go = 0;
    }లేకపోతే{
      xInit = (sqlite3_loadext_నమోదు)wsdAutoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    ఒకవేళ( xInit && (rc = xInit(db, &zErrmsg, pThunk))!=0 ){
      sqlite3_Error_With_సందేశం(db, rc,
            "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}
