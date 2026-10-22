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

#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#અન્તર્ભાવ "sqlite3ext.h"
#અન્તર્ભાવ "sqliteInt.h"

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Some API routines are omitted when various features are
** excluded from a build of SQLite.  Substitute a NULL pointer
** for any missing APIs.
*/
#ifndef SQLITE_ENABLE_COLUMN_METADATA
# define sqlite3_column_database_name   0
# define sqlite3_column_database_name16 0
# define sqlite3_column_table_name      0
# define sqlite3_column_table_name16    0
# define sqlite3_column_origin_name     0
# define sqlite3_column_origin_name16   0
#endif

#ifdef SQLITE_OMIT_AUTHORIZATION
# define sqlite3_set_authorizer         0
#endif

#ifdef SQLITE_OMIT_UTF16
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
# define sqlite3_result_error16         0
# define sqlite3_result_text16          0
# define sqlite3_result_text16be        0
# define sqlite3_result_text16le        0
# define sqlite3_કિંમત_text16           0
# define sqlite3_કિંમત_text16be         0
# define sqlite3_કિંમત_text16le         0
# define sqlite3_column_database_name16 0
# define sqlite3_column_table_name16    0
# define sqlite3_column_origin_name16   0
#endif

#ifdef SQLITE_OMIT_COMPLETE
# define sqlite3_complete 0
# define sqlite3_complete16 0
#endif

#ifdef SQLITE_OMIT_DECLTYPE
# define sqlite3_column_decltype16      0
# define sqlite3_column_decltype        0
#endif

#ifdef SQLITE_OMIT_PROGRESS_CALLBACK
# define sqlite3_progress_handler 0
#endif

#ifdef SQLITE_OMIT_VIRTUALTABLE
# define sqlite3_create_module 0
# define sqlite3_create_module_v2 0
# define sqlite3_declare_vtab 0
# define sqlite3_vtab_config 0
# define sqlite3_vtab_on_conflict 0
# define sqlite3_vtab_collation 0
#endif

#ifdef SQLITE_OMIT_SHARED_CACHE
# define sqlite3_enable_shared_cache 0
#endif

#જો defined(SQLITE_OMIT_TRACE) || defined(SQLITE_OMIT_DEPRECATED)
# define sqlite3_profile       0
# define sqlite3_trace         0
#endif

#ifdef SQLITE_OMIT_GET_TABLE
# define sqlite3_free_table    0
# define sqlite3_get_table     0
#endif

#ifdef SQLITE_OMIT_INCRBLOB
#define sqlite3_bind_zeroblob  0
#define sqlite3_blob_bytes     0
#define sqlite3_blob_close     0
#define sqlite3_blob_open      0
#define sqlite3_blob_read      0
#define sqlite3_blob_write     0
#define sqlite3_blob_reopen    0
#endif

#જો defined(SQLITE_OMIT_TRACE)
# define sqlite3_trace_v2      0
#endif

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
સ્થિર અચલ sqlite3_api_routines sqlite3Apis = {
  sqlite3_aggregate_context,
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_aggregate_ગણતરી,
#નહીંતર
  0,
#endif
  sqlite3_bind_blob,
  sqlite3_bind_double,
  sqlite3_bind_int,
  sqlite3_bind_int64,
  sqlite3_bind_null,
  sqlite3_bind_parameter_ગણતરી,
  sqlite3_bind_parameter_index,
  sqlite3_bind_parameter_name,
  sqlite3_bind_text,
  sqlite3_bind_text16,
  sqlite3_bind_કિંમત,
  sqlite3_busy_handler,
  sqlite3_busy_timeout,
  sqlite3_changes,
  sqlite3_close,
  sqlite3_collation_needed,
  sqlite3_collation_needed16,
  sqlite3_column_blob,
  sqlite3_column_bytes,
  sqlite3_column_bytes16,
  sqlite3_column_ગણતરી,
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
  sqlite3_column_table_name,
  sqlite3_column_table_name16,
  sqlite3_column_text,
  sqlite3_column_text16,
  sqlite3_column_type,
  sqlite3_column_કિંમત,
  sqlite3_commit_hook,
  sqlite3_complete,
  sqlite3_complete16,
  sqlite3_create_collation,
  sqlite3_create_collation16,
  sqlite3_create_function,
  sqlite3_create_function16,
  sqlite3_create_module,
  sqlite3_data_ગણતરી,
  sqlite3_db_handle,
  sqlite3_declare_vtab,
  sqlite3_enable_shared_cache,
  sqlite3_errcode,
  sqlite3_errmsg,
  sqlite3_errmsg16,
  sqlite3_exec,
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_expired,
#નહીંતર
  0,
#endif
  sqlite3_finalize,
  sqlite3_free,
  sqlite3_free_table,
  sqlite3_get_autocommit,
  sqlite3_get_auxdata,
  sqlite3_get_table,
  0,     /* Was sqlite3_global_recover(), but that function is deprecated */
  sqlite3_interrupt,
  sqlite3_last_ઉમેરો_rowid,
  sqlite3_libversion,
  sqlite3_libversion_સંખ્યા,
  sqlite3_malloc,
  sqlite3_mprintf,
  sqlite3_open,
  sqlite3_open16,
  sqlite3_prepare,
  sqlite3_prepare16,
  sqlite3_profile,
  sqlite3_progress_handler,
  sqlite3_realloc,
  sqlite3_reset,
  sqlite3_result_blob,
  sqlite3_result_double,
  sqlite3_result_error,
  sqlite3_result_error16,
  sqlite3_result_int,
  sqlite3_result_int64,
  sqlite3_result_null,
  sqlite3_result_text,
  sqlite3_result_text16,
  sqlite3_result_text16be,
  sqlite3_result_text16le,
  sqlite3_result_કિંમત,
  sqlite3_rollback_hook,
  sqlite3_set_authorizer,
  sqlite3_set_auxdata,
  sqlite3_snprintf,
  sqlite3_step,
  sqlite3_table_column_metadata,
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup,
#નહીંતર
  0,
#endif
  sqlite3_total_changes,
  sqlite3_trace,
#ifndef SQLITE_OMIT_DEPRECATED
  sqlite3_transfer_bindings,
#નહીંતર
  0,
#endif
  sqlite3_update_hook,
  sqlite3_user_data,
  sqlite3_કિંમત_blob,
  sqlite3_કિંમત_bytes,
  sqlite3_કિંમત_bytes16,
  sqlite3_કિંમત_double,
  sqlite3_કિંમત_int,
  sqlite3_કિંમત_int64,
  sqlite3_કિંમત_numeric_type,
  sqlite3_કિંમત_text,
  sqlite3_કિંમત_text16,
  sqlite3_કિંમત_text16be,
  sqlite3_કિંમત_text16le,
  sqlite3_કિંમત_type,
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
  sqlite3_blob_bytes,
  sqlite3_blob_close,
  sqlite3_blob_open,
  sqlite3_blob_read,
  sqlite3_blob_write,
  sqlite3_create_collation_v2,
  sqlite3_file_control,
  sqlite3_memory_highwater,
  sqlite3_memory_used,
#ifdef SQLITE_MUTEX_OMIT
  0, 
  0, 
  0,
  0,
  0,
#નહીંતર
  sqlite3_mutex_alloc,
  sqlite3_mutex_enter,
  sqlite3_mutex_free,
  sqlite3_mutex_leave,
  sqlite3_mutex_try,
#endif
  sqlite3_open_v2,
  sqlite3_release_memory,
  sqlite3_result_error_nomem,
  sqlite3_result_error_toobig,
  sqlite3_sleep,
  sqlite3_soft_heap_limit,
  sqlite3_vfs_find,
  sqlite3_vfs_register,
  sqlite3_vfs_unregister,

  /*
  ** Added for 3.5.8
  */
  sqlite3_threadsafe,
  sqlite3_result_zeroblob,
  sqlite3_result_error_code,
  sqlite3_test_control,
  sqlite3_randomness,
  sqlite3_context_db_handle,

  /*
  ** Added for 3.6.0
  */
  sqlite3_extended_result_codes,
  sqlite3_limit,
  sqlite3_next_stmt,
  sqlite3_sql,
  sqlite3_status,

  /*
  ** Added for 3.7.4
  */
  sqlite3_backup_finish,
  sqlite3_backup_init,
  sqlite3_backup_pagecount,
  sqlite3_backup_remaining,
  sqlite3_backup_step,
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  sqlite3_compileoption_get,
  sqlite3_compileoption_used,
#નહીંતર
  0,
  0,
#endif
  sqlite3_create_function_v2,
  sqlite3_db_config,
  sqlite3_db_mutex,
  sqlite3_db_status,
  sqlite3_extended_errcode,
  sqlite3_log,
  sqlite3_soft_heap_limit64,
  sqlite3_sourceid,
  sqlite3_stmt_status,
  sqlite3_strnicmp,
#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
  sqlite3_unlock_notify,
#નહીંતર
  0,
#endif
#ifndef SQLITE_OMIT_WAL
  sqlite3_wal_autocheckpoint,
  sqlite3_wal_checkpoint,
  sqlite3_wal_hook,
#નહીંતર
  0,
  0,
  0,
#endif
  sqlite3_blob_reopen,
  sqlite3_vtab_config,
  sqlite3_vtab_on_conflict,
  sqlite3_close_v2,
  sqlite3_db_filename,
  sqlite3_db_readonly,
  sqlite3_db_release_memory,
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
  sqlite3_load_extension,
  sqlite3_malloc64,
  sqlite3_msize,
  sqlite3_realloc64,
  sqlite3_reset_auto_extension,
  sqlite3_result_blob64,
  sqlite3_result_text64,
  sqlite3_strglob,
  /* Version 3.8.11 and later */
  (sqlite3_કિંમત*(*)(અચલ sqlite3_કિંમત*))sqlite3_કિંમત_dup,
  sqlite3_કિંમત_free,
  sqlite3_result_zeroblob64,
  sqlite3_bind_zeroblob64,
  /* Version 3.9.0 and later */
  sqlite3_કિંમત_subtype,
  sqlite3_result_subtype,
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
  sqlite3_set_last_ઉમેરો_rowid,
  /* Version 3.20.0 and later */
  sqlite3_prepare_v3,
  sqlite3_prepare16_v3,
  sqlite3_bind_pointer,
  sqlite3_result_pointer,
  sqlite3_કિંમત_pointer,
  /* Version 3.22.0 and later */
  sqlite3_vtab_nochange,
  sqlite3_કિંમત_nochange,
  sqlite3_vtab_collation,
  /* Version 3.24.0 and later */
  sqlite3_keyword_ગણતરી,
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
  sqlite3_str_length,
  sqlite3_str_કિંમત,
  /* Version 3.25.0 and later */
  sqlite3_create_window_function,
  /* Version 3.26.0 and later */
#ifdef SQLITE_ENABLE_NORMALIZE
  sqlite3_normalized_sql,
#નહીંતર
  0,
#endif
  /* Version 3.28.0 and later */
  sqlite3_stmt_isexplain,
  sqlite3_કિંમત_frombind,
  /* Version 3.30.0 and later */
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_drop_modules,
#નહીંતર
  0,
#endif
  /* Version 3.31.0 and later */
  sqlite3_hard_heap_limit64,
  sqlite3_uri_ચાવી,
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
  sqlite3_total_changes64,
  /* Version 3.37.0 and later */
  sqlite3_autovacuum_pages,
  /* Version 3.38.0 and later */
  sqlite3_error_offset,
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_vtab_rhs_કિંમત,
  sqlite3_vtab_distinct,
  sqlite3_vtab_in,
  sqlite3_vtab_in_first,
  sqlite3_vtab_in_next,
#નહીંતર
  0,
  0,
  0,
  0,
  0,
#endif
  /* Version 3.39.0 and later */
#ifndef SQLITE_OMIT_DESERIALIZE
  sqlite3_deserialize,
  sqlite3_serialize,
#નહીંતર
  0,
  0,
#endif
  sqlite3_db_name,
  /* Version 3.40.0 and later */
  sqlite3_કિંમત_encoding,
  /* Version 3.41.0 and later */
  sqlite3_is_interrupted,
  /* Version 3.43.0 and later */
  sqlite3_stmt_explain,
  /* Version 3.44.0 and later */
  sqlite3_get_clientdata,
  sqlite3_set_clientdata,
  /* Version 3.50.0 and later */
  sqlite3_setlk_timeout,
  /* Version 3.51.0 and later */
  sqlite3_set_errmsg,
  sqlite3_db_status64,
  /* Version 3.52.0 and later */
  sqlite3_str_truncate,
  sqlite3_str_free,
#ifdef SQLITE_ENABLE_CARRAY
  sqlite3_carray_bind,
  sqlite3_carray_bind_v2,
#નહીંતર
  0,
  0,
#endif
  /* Version 3.54.0 and later */
  sqlite3_incomplete,
  sqlite3_result_str
};

/* True if x is the directory separator character
*/
#જો SQLITE_OS_WIN
# define DirSep(X)  ((X)=='/'||(X)=='\\')
#નહીંતર
# define DirSep(X)  ((X)=='/')
#endif

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
સ્થિર પૂર્ણાંક sqlite3LoadExtension(
  sqlite3 *db,          /* Load the extension into this database connection */
  અચલ અક્ષર *zFile,    /* Name of the shared library containing extension */
  અચલ અક્ષર *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  અક્ષર **pz_Err_સંદેશ       /* Put error message here if not 0 */
){
  sqlite3_vfs *pVfs = db->pVfs;
  શૂન્ય *handle;
  sqlite3_loadext_entry xInit;
  અક્ષર *zErrmsg = 0;
  અચલ અક્ષર *zEntry;
  અક્ષર *zAltEntry = 0;
  શૂન્ય **aHandle;
  u64 n_સંદેશ = strlen(zFile);
  પૂર્ણાંક ii;
  પૂર્ણાંક rc;

  /* Shared library endings to try if zFile cannot be loaded as written */
  સ્થિર અચલ અક્ષર *azEndings[] = {
#જો SQLITE_OS_WIN
     "dll"   
#અથવાજો defined(__APPLE__)
     "dylib"
#નહીંતર
     "so"
#endif
  };


  જો( pz_Err_સંદેશ ) *pz_Err_સંદેશ = 0;

  /* Ticket #1863.  To avoid a creating security problems for older
  ** applications that relink against newer versions of SQLite, the
  ** ability to run load_extension is turned off by default.  One
  ** must call either sqlite3_enable_load_extension(db) or
  ** sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, 0)
  ** to turn on extension loading.
  */
  જો( (db->flags & SQLITE_LoadExtension)==0 ){
    જો( pz_Err_સંદેશ ){
      *pz_Err_સંદેશ = sqlite3_mprintf("not authorized");
    }
    પાછા SQLITE_ERROR;
  }

  zEntry = zProc ? zProc : "sqlite3_extension_init";

  /* tag-20210611-1.  Some dlopen() implementations will segfault if given
  ** an oversize filename.  Most filesystems have a pathname limit of 4K,
  ** so limit the extension filename length to about twice that.
  ** https://sqlite.org/forum/forumpost/08a0d6d9bf
  **
  ** Later (2023-03-25): Save an extra 6 bytes for the filename suffix.
  ** See https://sqlite.org/forum/forumpost/24083b579d.
  */
  જો( n_સંદેશ>SQLITE_MAX_PATHLEN ) જાઓ extension_not_found;

  /* Do not allow sqlite3_load_extension() to link to a copy of the
  ** running application, by passing in an empty filename. */
  જો( n_સંદેશ==0 ) જાઓ extension_not_found;
    
  handle = sqlite3OsDlOpen(pVfs, zFile);
#જો SQLITE_OS_UNIX || SQLITE_OS_WIN
  માટે(ii=0; ii<ArraySize(azEndings) && handle==0; ii++){
    અક્ષર *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    જો( zAltFile==0 ) પાછા SQLITE_NOMEM_BKPT;
    જો( n_સંદેશ+strlen(azEndings[ii])+1<=SQLITE_MAX_PATHLEN ){
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }
#endif
  જો( handle==0 ) જાઓ extension_not_found;
  xInit = (sqlite3_loadext_entry)sqlite3OsDlSym(pVfs, handle, zEntry);

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
  જો( xInit==0 && zProc==0 ){
    પૂર્ણાંક iFile, iEntry, c;
    પૂર્ણાંક ncFile = sqlite3Strlen30(zFile);
    પૂર્ણાંક cnt = 0;
    zAltEntry = sqlite3_malloc64(ncFile+30);
    જો( zAltEntry==0 ){
      sqlite3OsDlClose(pVfs, handle);
      પાછા SQLITE_NOMEM_BKPT;
    }
    કરો{
      memcpy(zAltEntry, "sqlite3_", 8);
      માટે(iFile=ncFile-1; iFile>=0 && !DirSep(zFile[iFile]); iFile--){}
      iFile++;
      જો( sqlite3_strnicmp(zFile+iFile, "lib", 3)==0 ) iFile += 3;
      માટે(iEntry=8; (c = zFile[iFile])!=0 && c!='.'; iFile++){
        જો( sqlite3Isalpha(c) || (cnt && sqlite3Isdigit(c)) ){
          zAltEntry[iEntry++] = (અક્ષર)sqlite3UpperToLower[(unsigned)c];
        }
      }
      memcpy(zAltEntry+iEntry, "_init", 6);
      zEntry = zAltEntry;
      xInit = (sqlite3_loadext_entry)sqlite3OsDlSym(pVfs, handle, zEntry);
    }જ્યારે( xInit==0 && (++cnt)<2 );
  }
  જો( xInit==0 ){
    જો( pz_Err_સંદેશ ){
      n_સંદેશ += strlen(zEntry) + 300;
      *pz_Err_સંદેશ = zErrmsg = sqlite3_malloc64(n_સંદેશ);
      જો( zErrmsg ){
        પ્રતિજ્ઞા( n_સંદેશ<0x7fffffff );  /* zErrmsg would be NULL if not so */
        sqlite3_snprintf((પૂર્ણાંક)n_સંદેશ, zErrmsg,
            "no entry point [%s] in shared library [%s]", zEntry, zFile);
        sqlite3OsDlError(pVfs, n_સંદેશ-1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(zAltEntry);
    પાછા SQLITE_ERROR;
  }
  sqlite3_free(zAltEntry);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  જો( rc ){
    જો( rc==SQLITE_OK_LOAD_PERMANENTLY ) પાછા SQLITE_OK;
    જો( pz_Err_સંદેશ ){
      *pz_Err_સંદેશ = sqlite3_mprintf("error during initialization: %s", zErrmsg);
    }
    sqlite3_free(zErrmsg);
    sqlite3OsDlClose(pVfs, handle);
    પાછા SQLITE_ERROR;
  }

  /* Append the new shared library handle to the db->aExtension array. */
  aHandle = sqlite3DbMallocZero(db, કદ(handle)*(db->nExtension+1));
  જો( aHandle==0 ){
    પાછા SQLITE_NOMEM_BKPT;
  }
  જો( db->nExtension>0 ){
    memcpy(aHandle, db->aExtension, કદ(handle)*db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  પાછા SQLITE_OK;

extension_not_found:
  જો( pz_Err_સંદેશ ){
    n_સંદેશ += 300;
    *pz_Err_સંદેશ = zErrmsg = sqlite3_malloc64(n_સંદેશ);
    જો( zErrmsg ){
      પ્રતિજ્ઞા( n_સંદેશ<0x7fffffff );  /* zErrmsg would be NULL if not so */
      sqlite3_snprintf((પૂર્ણાંક)n_સંદેશ, zErrmsg,
          "unable to open shared library [%.*s]", SQLITE_MAX_PATHLEN, zFile);
      sqlite3OsDlError(pVfs, n_સંદેશ-1, zErrmsg);
    }
  }
  પાછા SQLITE_ERROR;
}
પૂર્ણાંક sqlite3_load_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  અચલ અક્ષર *zFile,    /* Name of the shared library containing extension */
  અચલ અક્ષર *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  અક્ષર **pz_Err_સંદેશ       /* Put error message here if not 0 */
){
  પૂર્ણાંક rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3LoadExtension(db, zFile, zProc, pz_Err_સંદેશ);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  પાછા rc;
}

/*
** Call this routine when the database connection is closing in order
** to clean up loaded extensions
*/
શૂન્ય sqlite3CloseExtensions(sqlite3 *db){
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( sqlite3_mutex_held(db->mutex) );
  માટે(i=0; i<db->nExtension; i++){
    sqlite3OsDlClose(db->pVfs, db->aExtension[i]);
  }
  sqlite3DbFree(db, db->aExtension);
}

/*
** Enable or disable extension loading.  Extension loading is disabled by
** default so as not to open security holes in older applications.
*/
પૂર્ણાંક sqlite3_enable_load_extension(sqlite3 *db, પૂર્ણાંક onoff){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) ) પાછા SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  જો( onoff ){
    db->flags |= SQLITE_LoadExtension|SQLITE_LoadExtFunc;
  }નહીંતર{
    db->flags &= ~(u64)(SQLITE_LoadExtension|SQLITE_LoadExtFunc);
  }
  sqlite3_mutex_leave(db->mutex);
  પાછા SQLITE_OK;
}

#endif /* !defined(SQLITE_OMIT_LOAD_EXTENSION) */

/*
** The following object holds the list of automatically loaded
** extensions.
**
** This list is shared across threads.  The SQLITE_MUTEX_STATIC_MAIN
** mutex must be held while accessing this list.
*/
પ્રકારવ્યાખ્યા સંરચના sqlite3AutoExtList sqlite3AutoExtList;
સ્થિર SQLITE_WSD સંરચના sqlite3AutoExtList {
  u32 nExt;              /* Number of entries in aExt[] */
  શૂન્ય (**aExt)(શૂન્ય);   /* Pointers to the extension init functions */
} sqlite3Autoext = { 0, 0 };

/* The "wsdAutoext" macro will resolve to the autoextension
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Autoext" state vector declared above.
*/
#ifdef SQLITE_OMIT_WSD
# define wsdAutoextInit \
  sqlite3AutoExtList *x = &GLOBAL(sqlite3AutoExtList,sqlite3Autoext)
# define wsdAutoext x[0]
#નહીંતર
# define wsdAutoextInit
# define wsdAutoext sqlite3Autoext
#endif


/*
** Register a statically linked extension that is automatically
** loaded by every new database connection.
*/
પૂર્ણાંક sqlite3_auto_extension(
  શૂન્ય (*xInit)(શૂન્ય)
){
  પૂર્ણાંક rc = SQLITE_OK;
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( xInit==0 ) પાછા SQLITE_MISUSE_BKPT;
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  જો( rc ){
    પાછા rc;
  }નહીંતર
#endif
  {
    u32 i;
#જો SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
    wsdAutoextInit;
    sqlite3_mutex_enter(mutex);
    માટે(i=0; i<wsdAutoext.nExt; i++){
      જો( wsdAutoext.aExt[i]==xInit ) તોડો;
    }
    જો( i==wsdAutoext.nExt ){
      u64 nByte = (wsdAutoext.nExt+1)*કદ(wsdAutoext.aExt[0]);
      શૂન્ય (**aNew)(શૂન્ય);
      aNew = sqlite3_realloc64(wsdAutoext.aExt, nByte);
      જો( aNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }નહીંતર{
        wsdAutoext.aExt = aNew;
        wsdAutoext.aExt[wsdAutoext.nExt] = xInit;
        wsdAutoext.nExt++;
      }
    }
    sqlite3_mutex_leave(mutex);
    પ્રતિજ્ઞા( (rc&0xff)==rc );
    પાછા rc;
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
પૂર્ણાંક sqlite3_cancel_auto_extension(
  શૂન્ય (*xInit)(શૂન્ય)
){
  પૂર્ણાંક n = 0;
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ){
    પાછા 0;
  }નહીંતર
#endif
  {
#જો SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
    પૂર્ણાંક i;
    wsdAutoextInit;
#ifdef SQLITE_ENABLE_API_ARMOR
    જો( xInit==0 ) પાછા 0;
#endif
    sqlite3_mutex_enter(mutex);
    માટે(i=(પૂર્ણાંક)wsdAutoext.nExt-1; i>=0; i--){
      જો( wsdAutoext.aExt[i]==xInit ){
        wsdAutoext.nExt--;
        wsdAutoext.aExt[i] = wsdAutoext.aExt[wsdAutoext.nExt];
        n++;
        તોડો;
      }
    }
    sqlite3_mutex_leave(mutex);
  }
  પાછા n;
}

/*
** Reset the automatic extension loading mechanism.
*/
શૂન્ય sqlite3_reset_auto_extension(શૂન્ય){
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize()==SQLITE_OK )
#endif
  {
#જો SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
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
શૂન્ય sqlite3AutoLoadExtensions(sqlite3 *db){
  u32 i;
  પૂર્ણાંક go = 1;
  પૂર્ણાંક rc;
  sqlite3_loadext_entry xInit;

  wsdAutoextInit;
  જો( wsdAutoext.nExt==0 ){
    /* Common case: early out without every having to acquire a mutex */
    પાછા;
  }
  માટે(i=0; go; i++){
    અક્ષર *zErrmsg;
#જો SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#endif
#ifdef SQLITE_OMIT_LOAD_EXTENSION
    અચલ sqlite3_api_routines *pThunk = 0;
#નહીંતર
    અચલ sqlite3_api_routines *pThunk = &sqlite3Apis;
#endif
    sqlite3_mutex_enter(mutex);
    જો( i>=wsdAutoext.nExt ){
      xInit = 0;
      go = 0;
    }નહીંતર{
      xInit = (sqlite3_loadext_entry)wsdAutoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    જો( xInit && (rc = xInit(db, &zErrmsg, pThunk))!=0 ){
      sqlite3_Error_With_સંદેશ(db, rc,
            "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}
