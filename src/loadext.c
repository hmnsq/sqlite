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

#यदि_अपरिभाषितम् SQLITE_CORE
  #परिभाषय SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#यदि_अन्तः
#अन्तर्भाव "sqlite3ext.h"
#अन्तर्भाव "sqliteInt.h"

#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
/*
** Some API routines are omitted when various features are
** excluded from a build of SQLite.  Substitute a NULL pointer
** for any missing APIs.
*/
#यदि_अपरिभाषितम् SQLITE_ENABLE_COLUMN_METADATA
# define sqlite3_column_database_name   0
# define sqlite3_column_database_name16 0
# define sqlite3_column_सारणी_name      0
# define sqlite3_column_सारणी_name16    0
# define sqlite3_column_origin_name     0
# define sqlite3_column_origin_name16   0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_AUTHORIZATION
# define sqlite3_समुच्चयः_authorizer         0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_UTF16
# define sqlite3_bind_text16            0
# define sqlite3_collation_needed16     0
# define sqlite3_column_decltype16      0
# define sqlite3_column_name16          0
# define sqlite3_column_text16          0
# define sqlite3_complete16             0
# define sqlite3_create_collation16     0
# define sqlite3_create_function16      0
# define sqlite3_errmsg16               0
# define sqlite3_open16                 0
# define sqlite3_prepare16              0
# define sqlite3_prepare16_v2           0
# define sqlite3_prepare16_v3           0
# define sqlite3_परिणाम_error16         0
# define sqlite3_परिणाम_text16          0
# define sqlite3_परिणाम_text16be        0
# define sqlite3_परिणाम_text16le        0
# define sqlite3_मानम्_text16           0
# define sqlite3_मानम्_text16be         0
# define sqlite3_मानम्_text16le         0
# define sqlite3_column_database_name16 0
# define sqlite3_column_सारणी_name16    0
# define sqlite3_column_origin_name16   0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_COMPLETE
# define sqlite3_complete 0
# define sqlite3_complete16 0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_DECLTYPE
# define sqlite3_column_decltype16      0
# define sqlite3_column_decltype        0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_प्रगतिः_CALLBACK
# define sqlite3_प्रगतिः_handler 0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_VIRTUALTABLE
# define sqlite3_create_module 0
# define sqlite3_create_module_v2 0
# define sqlite3_declare_vtab 0
# define sqlite3_vtab_config 0
# define sqlite3_vtab_on_conflict 0
# define sqlite3_vtab_collation 0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_SHARED_CACHE
# define sqlite3_enable_shared_cache 0
#यदि_अन्तः

#यदि defined(SQLITE_OMIT_TRACE) || defined(SQLITE_OMIT_DEPRECATED)
# define sqlite3_profile       0
# define sqlite3_trace         0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_GET_सारणी
# define sqlite3_free_सारणी    0
# define sqlite3_get_सारणी     0
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_INCRBLOB
#परिभाषय sqlite3_bind_zeroblob  0
#परिभाषय sqlite3_blob_अष्टकानि     0
#परिभाषय sqlite3_blob_close     0
#परिभाषय sqlite3_blob_open      0
#परिभाषय sqlite3_blob_read      0
#परिभाषय sqlite3_blob_write     0
#परिभाषय sqlite3_blob_reopen    0
#यदि_अन्तः

#यदि defined(SQLITE_OMIT_TRACE)
# define sqlite3_trace_v2      0
#यदि_अन्तः

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
स्थिर ध्रुव sqlite3_api_routines sqlite3Apis = {
  sqlite3_aggregate_context,
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
  sqlite3_aggregate_गणनम्,
#अन्यथा
  0,
#यदि_अन्तः
  sqlite3_bind_blob,
  sqlite3_bind_double,
  sqlite3_bind_int,
  sqlite3_bind_int64,
  sqlite3_bind_null,
  sqlite3_bind_parameter_गणनम्,
  sqlite3_bind_parameter_सूचकांक,
  sqlite3_bind_parameter_name,
  sqlite3_bind_text,
  sqlite3_bind_text16,
  sqlite3_bind_मानम्,
  sqlite3_busy_handler,
  sqlite3_busy_timeout,
  sqlite3_changes,
  sqlite3_close,
  sqlite3_collation_needed,
  sqlite3_collation_needed16,
  sqlite3_column_blob,
  sqlite3_column_अष्टकानि,
  sqlite3_column_bytes16,
  sqlite3_column_गणनम्,
  sqlite3_column_database_name,
  sqlite3_column_database_name16,
  sqlite3_column_decltype,
  sqlite3_column_decltype16,
  sqlite3_column_double,
  sqlite3_column_int,
  sqlite3_column_int64,
  sqlite3_column_name,
  sqlite3_column_name16,
  sqlite3_column_origin_name,
  sqlite3_column_origin_name16,
  sqlite3_column_सारणी_name,
  sqlite3_column_सारणी_name16,
  sqlite3_column_text,
  sqlite3_column_text16,
  sqlite3_column_type,
  sqlite3_column_मानम्,
  sqlite3_commit_hook,
  sqlite3_complete,
  sqlite3_complete16,
  sqlite3_create_collation,
  sqlite3_create_collation16,
  sqlite3_create_function,
  sqlite3_create_function16,
  sqlite3_create_module,
  sqlite3_दत्त_गणनम्,
  sqlite3_db_handle,
  sqlite3_declare_vtab,
  sqlite3_enable_shared_cache,
  sqlite3_errcode,
  sqlite3_errmsg,
  sqlite3_errmsg16,
  sqlite3_exec,
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
  sqlite3_expired,
#अन्यथा
  0,
#यदि_अन्तः
  sqlite3_finalize,
  sqlite3_free,
  sqlite3_free_सारणी,
  sqlite3_get_autocommit,
  sqlite3_get_auxdata,
  sqlite3_get_सारणी,
  0,     /* Was sqlite3_global_recover(), but that function is deprecated */
  sqlite3_व्यवधानम्,
  sqlite3_चरमः_निवेशय_rowid,
  sqlite3_libversion,
  sqlite3_libversion_सङ्ख्या,
  sqlite3_malloc,
  sqlite3_mprintf,
  sqlite3_open,
  sqlite3_open16,
  sqlite3_prepare,
  sqlite3_prepare16,
  sqlite3_profile,
  sqlite3_प्रगतिः_handler,
  sqlite3_realloc,
  sqlite3_reset,
  sqlite3_परिणाम_blob,
  sqlite3_परिणाम_double,
  sqlite3_परिणाम_error,
  sqlite3_परिणाम_error16,
  sqlite3_परिणाम_int,
  sqlite3_परिणाम_int64,
  sqlite3_परिणाम_null,
  sqlite3_परिणाम_text,
  sqlite3_परिणाम_text16,
  sqlite3_परिणाम_text16be,
  sqlite3_परिणाम_text16le,
  sqlite3_परिणाम_मानम्,
  sqlite3_rollback_hook,
  sqlite3_समुच्चयः_authorizer,
  sqlite3_समुच्चयः_auxdata,
  sqlite3_snprintf,
  sqlite3_चरणः,
  sqlite3_सारणी_column_metadata,
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup,
#अन्यथा
  0,
#यदि_अन्तः
  sqlite3_योग_changes,
  sqlite3_trace,
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
  sqlite3_transfer_bindings,
#अन्यथा
  0,
#यदि_अन्तः
  sqlite3_update_hook,
  sqlite3_user_दत्त,
  sqlite3_मानम्_blob,
  sqlite3_मानम्_अष्टकानि,
  sqlite3_मानम्_bytes16,
  sqlite3_मानम्_double,
  sqlite3_मानम्_int,
  sqlite3_मानम्_int64,
  sqlite3_मानम्_numeric_type,
  sqlite3_मानम्_text,
  sqlite3_मानम्_text16,
  sqlite3_मानम्_text16be,
  sqlite3_मानम्_text16le,
  sqlite3_मानम्_type,
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
  sqlite3_blob_अष्टकानि,
  sqlite3_blob_close,
  sqlite3_blob_open,
  sqlite3_blob_read,
  sqlite3_blob_write,
  sqlite3_create_collation_v2,
  sqlite3_file_control,
  sqlite3_स्मृतिः_highwater,
  sqlite3_स्मृतिः_प्रयुक्तम्,
#यदि_परिभाषितम् SQLITE_MUTEX_OMIT
  0, 
  0, 
  0,
  0,
  0,
#अन्यथा
  sqlite3_mutex_alloc,
  sqlite3_mutex_enter,
  sqlite3_mutex_free,
  sqlite3_mutex_leave,
  sqlite3_mutex_try,
#यदि_अन्तः
  sqlite3_open_v2,
  sqlite3_release_स्मृतिः,
  sqlite3_परिणाम_error_nomem,
  sqlite3_परिणाम_error_toobig,
  sqlite3_sleep,
  sqlite3_soft_heap_सीमा,
  sqlite3_vfs_find,
  sqlite3_vfs_पञ्जिकाकोशः,
  sqlite3_vfs_unregister,

  /*
  ** Added for 3.5.8
  */
  sqlite3_threadsafe,
  sqlite3_परिणाम_zeroblob,
  sqlite3_परिणाम_error_संहिता,
  sqlite3_test_control,
  sqlite3_randomness,
  sqlite3_context_db_handle,

  /*
  ** Added for 3.6.0
  */
  sqlite3_extended_परिणाम_codes,
  sqlite3_सीमा,
  sqlite3_अग्र_stmt,
  sqlite3_sql,
  sqlite3_स्थितिः,

  /*
  ** Added for 3.7.4
  */
  sqlite3_backup_finish,
  sqlite3_backup_init,
  sqlite3_backup_pagecount,
  sqlite3_backup_remaining,
  sqlite3_backup_चरणः,
#यदि_अपरिभाषितम् SQLITE_OMIT_COMPILEOPTION_DIAGS
  sqlite3_compileoption_get,
  sqlite3_compileoption_प्रयुक्तम्,
#अन्यथा
  0,
  0,
#यदि_अन्तः
  sqlite3_create_function_v2,
  sqlite3_db_config,
  sqlite3_db_mutex,
  sqlite3_db_स्थितिः,
  sqlite3_extended_errcode,
  sqlite3_log,
  sqlite3_soft_heap_limit64,
  sqlite3_sourceid,
  sqlite3_stmt_स्थितिः,
  sqlite3_strnicmp,
#यदि_परिभाषितम् SQLITE_ENABLE_UNLOCK_NOTIFY
  sqlite3_unlock_notify,
#अन्यथा
  0,
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  sqlite3_wal_autocheckpoint,
  sqlite3_wal_checkpoint,
  sqlite3_wal_hook,
#अन्यथा
  0,
  0,
  0,
#यदि_अन्तः
  sqlite3_blob_reopen,
  sqlite3_vtab_config,
  sqlite3_vtab_on_conflict,
  sqlite3_close_v2,
  sqlite3_db_filename,
  sqlite3_db_readonly,
  sqlite3_db_release_स्मृतिः,
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
  sqlite3_आरोपय_extension,
  sqlite3_malloc64,
  sqlite3_msize,
  sqlite3_realloc64,
  sqlite3_reset_auto_extension,
  sqlite3_परिणाम_blob64,
  sqlite3_परिणाम_text64,
  sqlite3_strglob,
  /* Version 3.8.11 and later */
  (sqlite3_मानम्*(*)(ध्रुव sqlite3_मानम्*))sqlite3_मानम्_dup,
  sqlite3_मानम्_free,
  sqlite3_परिणाम_zeroblob64,
  sqlite3_bind_zeroblob64,
  /* Version 3.9.0 and later */
  sqlite3_मानम्_subtype,
  sqlite3_परिणाम_subtype,
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
  sqlite3_समुच्चयः_चरमः_निवेशय_rowid,
  /* Version 3.20.0 and later */
  sqlite3_prepare_v3,
  sqlite3_prepare16_v3,
  sqlite3_bind_pointer,
  sqlite3_परिणाम_pointer,
  sqlite3_मानम्_pointer,
  /* Version 3.22.0 and later */
  sqlite3_vtab_nochange,
  sqlite3_मानम्_nochange,
  sqlite3_vtab_collation,
  /* Version 3.24.0 and later */
  sqlite3_keyword_गणनम्,
  sqlite3_keyword_name,
  sqlite3_keyword_check,
  sqlite3_str_new,
  sqlite3_str_finish,
  sqlite3_str_appendf,
  sqlite3_str_vappendf,
  sqlite3_str_append,
  sqlite3_str_appendall,
  sqlite3_str_appendchar,
  sqlite3_str_reset,
  sqlite3_str_errcode,
  sqlite3_str_दीर्घता,
  sqlite3_str_मानम्,
  /* Version 3.25.0 and later */
  sqlite3_create_गवाक्षः_function,
  /* Version 3.26.0 and later */
#यदि_परिभाषितम् SQLITE_ENABLE_NORMALIZE
  sqlite3_normalized_sql,
#अन्यथा
  0,
#यदि_अन्तः
  /* Version 3.28.0 and later */
  sqlite3_stmt_isexplain,
  sqlite3_मानम्_frombind,
  /* Version 3.30.0 and later */
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  sqlite3_drop_modules,
#अन्यथा
  0,
#यदि_अन्तः
  /* Version 3.31.0 and later */
  sqlite3_hard_heap_limit64,
  sqlite3_uri_कुञ्चिका,
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
  sqlite3_योग_changes64,
  /* Version 3.37.0 and later */
  sqlite3_autovacuum_पृष्ठानि,
  /* Version 3.38.0 and later */
  sqlite3_error_विस्थापनम्,
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  sqlite3_vtab_rhs_मानम्,
  sqlite3_vtab_distinct,
  sqlite3_vtab_अन्तर्,
  sqlite3_vtab_अन्तर्_first,
  sqlite3_vtab_अन्तर्_अग्र,
#अन्यथा
  0,
  0,
  0,
  0,
  0,
#यदि_अन्तः
  /* Version 3.39.0 and later */
#यदि_अपरिभाषितम् SQLITE_OMIT_DESERIALIZE
  sqlite3_deserialize,
  sqlite3_serialize,
#अन्यथा
  0,
  0,
#यदि_अन्तः
  sqlite3_db_name,
  /* Version 3.40.0 and later */
  sqlite3_मानम्_encoding,
  /* Version 3.41.0 and later */
  sqlite3_is_interrupted,
  /* Version 3.43.0 and later */
  sqlite3_stmt_explain,
  /* Version 3.44.0 and later */
  sqlite3_get_clientdata,
  sqlite3_समुच्चयः_clientdata,
  /* Version 3.50.0 and later */
  sqlite3_setlk_timeout,
  /* Version 3.51.0 and later */
  sqlite3_समुच्चयः_errmsg,
  sqlite3_db_status64,
  /* Version 3.52.0 and later */
  sqlite3_str_truncate,
  sqlite3_str_free,
#यदि_परिभाषितम् SQLITE_ENABLE_CARRAY
  sqlite3_carray_bind,
  sqlite3_carray_bind_v2,
#अन्यथा
  0,
  0,
#यदि_अन्तः
  /* Version 3.54.0 and later */
  sqlite3_incomplete,
  sqlite3_परिणाम_str
};

/* True if x is the directory separator character
*/
#यदि SQLITE_OS_WIN
# define DirSep(X)  ((X)=='/'||(X)=='\\')
#अन्यथा
# define DirSep(X)  ((X)=='/')
#यदि_अन्तः

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
स्थिर पूर्णाङ्क sqlite3_आरोपय_Extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ध्रुव वर्ण *zFile,    /* Name of the shared library containing extension */
  ध्रुव वर्ण *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  वर्ण **pz_Err_सन्देशः       /* Put error message here if not 0 */
){
  sqlite3_vfs *pVfs = db->pVfs;
  शून्य *handle;
  sqlite3_loadext_प्रविष्टिः xInit;
  वर्ण *zErrmsg = 0;
  ध्रुव वर्ण *z_प्रविष्टिः;
  वर्ण *z_Alt_प्रविष्टिः = 0;
  शून्य **aHandle;
  u64 n_सन्देशः = वाक्यदैर्घ्यम्(zFile);
  पूर्णाङ्क ii;
  पूर्णाङ्क rc;

  /* Shared library endings to try if zFile cannot be loaded as written */
  स्थिर ध्रुव वर्ण *azEndings[] = {
#यदि SQLITE_OS_WIN
     "dll"   
#नोचेद्यदि defined(__APPLE__)
     "dylib"
#अन्यथा
     "so"
#यदि_अन्तः
  };


  यदि( pz_Err_सन्देशः ) *pz_Err_सन्देशः = 0;

  /* Ticket #1863.  To avoid a creating security problems for older
  ** applications that relink against newer versions of SQLite, the
  ** ability to run load_extension is turned off by default.  One
  ** must call either sqlite3_enable_load_extension(db) or
  ** sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, 0)
  ** to turn on extension loading.
  */
  यदि( (db->flags & SQLITE_आरोपय_Extension)==0 ){
    यदि( pz_Err_सन्देशः ){
      *pz_Err_सन्देशः = sqlite3_mprintf("not authorized");
    }
    प्रत्यागम SQLITE_ERROR;
  }

  z_प्रविष्टिः = zProc ? zProc : "sqlite3_extension_init";

  /* tag-20210611-1.  Some dlopen() implementations will segfault if given
  ** an oversize filename.  Most filesystems have a pathname limit of 4K,
  ** so limit the extension filename length to about twice that.
  ** https://sqlite.org/forum/forumpost/08a0d6d9bf
  **
  ** Later (2023-03-25): Save an extra 6 bytes for the filename suffix.
  ** See https://sqlite.org/forum/forumpost/24083b579d.
  */
  यदि( n_सन्देशः>SQLITE_MAX_PATHLEN ) गच्छ extension_not_प्राप्तम्;

  /* Do not allow sqlite3_load_extension() to link to a copy of the
  ** running application, by passing in an empty filename. */
  यदि( n_सन्देशः==0 ) गच्छ extension_not_प्राप्तम्;
    
  handle = sqlite3OsDlOpen(pVfs, zFile);
#यदि SQLITE_OS_UNIX || SQLITE_OS_WIN
  प्रति(ii=0; ii<Array_आकार(azEndings) && handle==0; ii++){
    वर्ण *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    यदि( zAltFile==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
    यदि( n_सन्देशः+वाक्यदैर्घ्यम्(azEndings[ii])+1<=SQLITE_MAX_PATHLEN ){
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }
#यदि_अन्तः
  यदि( handle==0 ) गच्छ extension_not_प्राप्तम्;
  xInit = (sqlite3_loadext_प्रविष्टिः)sqlite3OsDlSym(pVfs, handle, z_प्रविष्टिः);

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
  यदि( xInit==0 && zProc==0 ){
    पूर्णाङ्क iFile, i_प्रविष्टिः, c;
    पूर्णाङ्क ncFile = sqlite3Strlen30(zFile);
    पूर्णाङ्क cnt = 0;
    z_Alt_प्रविष्टिः = sqlite3_malloc64(ncFile+30);
    यदि( z_Alt_प्रविष्टिः==0 ){
      sqlite3OsDlClose(pVfs, handle);
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    कुरु{
      स्मृतिं_प्रतिलिख(z_Alt_प्रविष्टिः, "sqlite3_", 8);
      प्रति(iFile=ncFile-1; iFile>=0 && !DirSep(zFile[iFile]); iFile--){}
      iFile++;
      यदि( sqlite3_strnicmp(zFile+iFile, "lib", 3)==0 ) iFile += 3;
      प्रति(i_प्रविष्टिः=8; (c = zFile[iFile])!=0 && c!='.'; iFile++){
        यदि( sqlite3Isalpha(c) || (cnt && sqlite3Isdigit(c)) ){
          z_Alt_प्रविष्टिः[i_प्रविष्टिः++] = (वर्ण)sqlite3_ऊर्ध्वस्थम्_गन्तव्यम्_अधस्तनम्[(अचिह्नित)c];
        }
      }
      स्मृतिं_प्रतिलिख(z_Alt_प्रविष्टिः+i_प्रविष्टिः, "_init", 6);
      z_प्रविष्टिः = z_Alt_प्रविष्टिः;
      xInit = (sqlite3_loadext_प्रविष्टिः)sqlite3OsDlSym(pVfs, handle, z_प्रविष्टिः);
    }यावत्( xInit==0 && (++cnt)<2 );
  }
  यदि( xInit==0 ){
    यदि( pz_Err_सन्देशः ){
      n_सन्देशः += वाक्यदैर्घ्यम्(z_प्रविष्टिः) + 300;
      *pz_Err_सन्देशः = zErrmsg = sqlite3_malloc64(n_सन्देशः);
      यदि( zErrmsg ){
        प्रतिज्ञा( n_सन्देशः<0x7fffffff );  /* zErrmsg would be NULL if not so */
        sqlite3_snprintf((पूर्णाङ्क)n_सन्देशः, zErrmsg,
            "no entry point [%s] in shared library [%s]", z_प्रविष्टिः, zFile);
        sqlite3OsDlError(pVfs, n_सन्देशः-1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(z_Alt_प्रविष्टिः);
    प्रत्यागम SQLITE_ERROR;
  }
  sqlite3_free(z_Alt_प्रविष्टिः);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  यदि( rc ){
    यदि( rc==SQLITE_OK_आरोपय_PERMANENTLY ) प्रत्यागम SQLITE_OK;
    यदि( pz_Err_सन्देशः ){
      *pz_Err_सन्देशः = sqlite3_mprintf("error during initialization: %s", zErrmsg);
    }
    sqlite3_free(zErrmsg);
    sqlite3OsDlClose(pVfs, handle);
    प्रत्यागम SQLITE_ERROR;
  }

  /* Append the new shared library handle to the db->aExtension array. */
  aHandle = sqlite3DbMallocZero(db, परिमाण(handle)*(db->nExtension+1));
  यदि( aHandle==0 ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  यदि( db->nExtension>0 ){
    स्मृतिं_प्रतिलिख(aHandle, db->aExtension, परिमाण(handle)*db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  प्रत्यागम SQLITE_OK;

extension_not_प्राप्तम्:
  यदि( pz_Err_सन्देशः ){
    n_सन्देशः += 300;
    *pz_Err_सन्देशः = zErrmsg = sqlite3_malloc64(n_सन्देशः);
    यदि( zErrmsg ){
      प्रतिज्ञा( n_सन्देशः<0x7fffffff );  /* zErrmsg would be NULL if not so */
      sqlite3_snprintf((पूर्णाङ्क)n_सन्देशः, zErrmsg,
          "unable to open shared library [%.*s]", SQLITE_MAX_PATHLEN, zFile);
      sqlite3OsDlError(pVfs, n_सन्देशः-1, zErrmsg);
    }
  }
  प्रत्यागम SQLITE_ERROR;
}
पूर्णाङ्क sqlite3_आरोपय_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ध्रुव वर्ण *zFile,    /* Name of the shared library containing extension */
  ध्रुव वर्ण *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  वर्ण **pz_Err_सन्देशः       /* Put error message here if not 0 */
){
  पूर्णाङ्क rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_आरोपय_Extension(db, zFile, zProc, pz_Err_सन्देशः);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम rc;
}

/*
** Call this routine when the database connection is closing in order
** to clean up loaded extensions
*/
शून्य sqlite3CloseExtensions(sqlite3 *db){
  पूर्णाङ्क i;
  प्रतिज्ञा( sqlite3_mutex_held(db->mutex) );
  प्रति(i=0; i<db->nExtension; i++){
    sqlite3OsDlClose(db->pVfs, db->aExtension[i]);
  }
  sqlite3DbFree(db, db->aExtension);
}

/*
** Enable or disable extension loading.  Extension loading is disabled by
** default so as not to open security holes in older applications.
*/
पूर्णाङ्क sqlite3_enable_आरोपय_extension(sqlite3 *db, पूर्णाङ्क onoff){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(db) ) प्रत्यागम SQLITE_MISUSE_BKPT;
#यदि_अन्तः
  sqlite3_mutex_enter(db->mutex);
  यदि( onoff ){
    db->flags |= SQLITE_आरोपय_Extension|SQLITE_आरोपय_Ext_Func;
  }अन्यथा{
    db->flags &= ~(u64)(SQLITE_आरोपय_Extension|SQLITE_आरोपय_Ext_Func);
  }
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम SQLITE_OK;
}

#यदि_अन्तः /* !defined(SQLITE_OMIT_LOAD_EXTENSION) */

/*
** The following object holds the list of automatically loaded
** extensions.
**
** This list is shared across threads.  The SQLITE_MUTEX_STATIC_MAIN
** mutex must be held while accessing this list.
*/
प्रकारनिर्देश संरचना sqlite3AutoExtList sqlite3AutoExtList;
स्थिर SQLITE_WSD संरचना sqlite3AutoExtList {
  u32 nExt;              /* Number of entries in aExt[] */
  शून्य (**aExt)(शून्य);   /* Pointers to the extension init functions */
} sqlite3Autoext = { 0, 0 };

/* The "wsdAutoext" macro will resolve to the autoextension
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Autoext" state vector declared above.
*/
#यदि_परिभाषितम् SQLITE_OMIT_WSD
# define wsdAutoextInit \
  sqlite3AutoExtList *x = &GLOBAL(sqlite3AutoExtList,sqlite3Autoext)
# define wsdAutoext x[0]
#अन्यथा
# define wsdAutoextInit
# define wsdAutoext sqlite3Autoext
#यदि_अन्तः


/*
** Register a statically linked extension that is automatically
** loaded by every new database connection.
*/
पूर्णाङ्क sqlite3_auto_extension(
  शून्य (*xInit)(शून्य)
){
  पूर्णाङ्क rc = SQLITE_OK;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( xInit==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  यदि( rc ){
    प्रत्यागम rc;
  }अन्यथा
#यदि_अन्तः
  {
    u32 i;
#यदि SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#यदि_अन्तः
    wsdAutoextInit;
    sqlite3_mutex_enter(mutex);
    प्रति(i=0; i<wsdAutoext.nExt; i++){
      यदि( wsdAutoext.aExt[i]==xInit ) भङ्ग;
    }
    यदि( i==wsdAutoext.nExt ){
      u64 nByte = (wsdAutoext.nExt+1)*परिमाण(wsdAutoext.aExt[0]);
      शून्य (**aNew)(शून्य);
      aNew = sqlite3_realloc64(wsdAutoext.aExt, nByte);
      यदि( aNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }अन्यथा{
        wsdAutoext.aExt = aNew;
        wsdAutoext.aExt[wsdAutoext.nExt] = xInit;
        wsdAutoext.nExt++;
      }
    }
    sqlite3_mutex_leave(mutex);
    प्रतिज्ञा( (rc&0xff)==rc );
    प्रत्यागम rc;
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
पूर्णाङ्क sqlite3_cancel_auto_extension(
  शून्य (*xInit)(शून्य)
){
  पूर्णाङ्क n = 0;
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ){
    प्रत्यागम 0;
  }अन्यथा
#यदि_अन्तः
  {
#यदि SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#यदि_अन्तः
    पूर्णाङ्क i;
    wsdAutoextInit;
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
    यदि( xInit==0 ) प्रत्यागम 0;
#यदि_अन्तः
    sqlite3_mutex_enter(mutex);
    प्रति(i=(पूर्णाङ्क)wsdAutoext.nExt-1; i>=0; i--){
      यदि( wsdAutoext.aExt[i]==xInit ){
        wsdAutoext.nExt--;
        wsdAutoext.aExt[i] = wsdAutoext.aExt[wsdAutoext.nExt];
        n++;
        भङ्ग;
      }
    }
    sqlite3_mutex_leave(mutex);
  }
  प्रत्यागम n;
}

/*
** Reset the automatic extension loading mechanism.
*/
शून्य sqlite3_reset_auto_extension(शून्य){
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize()==SQLITE_OK )
#यदि_अन्तः
  {
#यदि SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#यदि_अन्तः
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
शून्य sqlite3_Auto_आरोपय_Extensions(sqlite3 *db){
  u32 i;
  पूर्णाङ्क go = 1;
  पूर्णाङ्क rc;
  sqlite3_loadext_प्रविष्टिः xInit;

  wsdAutoextInit;
  यदि( wsdAutoext.nExt==0 ){
    /* Common case: early out without every having to acquire a mutex */
    प्रत्यागम;
  }
  प्रति(i=0; go; i++){
    वर्ण *zErrmsg;
#यदि SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
    ध्रुव sqlite3_api_routines *pThunk = 0;
#अन्यथा
    ध्रुव sqlite3_api_routines *pThunk = &sqlite3Apis;
#यदि_अन्तः
    sqlite3_mutex_enter(mutex);
    यदि( i>=wsdAutoext.nExt ){
      xInit = 0;
      go = 0;
    }अन्यथा{
      xInit = (sqlite3_loadext_प्रविष्टिः)wsdAutoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    यदि( xInit && (rc = xInit(db, &zErrmsg, pThunk))!=0 ){
      sqlite3_Error_With_सन्देशः(db, rc,
            "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}
