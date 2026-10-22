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

#جے_بنا_تعریف SQLITE_CORE
  #تعریف_کرو SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#جے_انت
#ضمن "sqlite3ext.h"
#ضمن "sqliteInt.h"

#جے_بنا_تعریف SQLITE_OMIT_لدو_EXTENSION
/*
** Some API routines are omitted when various features are
** excluded from a build of SQLite.  Substitute a NULL pointer
** for any missing APIs.
*/
#جے_بنا_تعریف SQLITE_ENABLE_کالم_METADATA
# define sqlite3_کالم_database_name   0
# define sqlite3_کالم_database_name16 0
# define sqlite3_کالم_جدول_name      0
# define sqlite3_کالم_جدول_name16    0
# define sqlite3_کالم_origin_name     0
# define sqlite3_کالم_origin_name16   0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
# define sqlite3_سیٹ_authorizer         0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_UTF16
# define sqlite3_bind_text16            0
# define sqlite3_collation_needed16     0
# define sqlite3_کالم_decltype16      0
# define sqlite3_کالم_name16          0
# define sqlite3_کالم_text16          0
# define sqlite3_complete16             0
# define sqlite3_create_collation16     0
# define sqlite3_create_function16      0
# define sqlite3_errmsg16               0
# define sqlite3_open16                 0
# define sqlite3_prepare16              0
# define sqlite3_prepare16_v2           0
# define sqlite3_prepare16_v3           0
# define sqlite3_نتیجہ_error16         0
# define sqlite3_نتیجہ_text16          0
# define sqlite3_نتیجہ_text16be        0
# define sqlite3_نتیجہ_text16le        0
# define sqlite3_قدر_text16           0
# define sqlite3_قدر_text16be         0
# define sqlite3_قدر_text16le         0
# define sqlite3_کالم_database_name16 0
# define sqlite3_کالم_جدول_name16    0
# define sqlite3_کالم_origin_name16   0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_COMPLETE
# define sqlite3_complete 0
# define sqlite3_complete16 0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_DECLTYPE
# define sqlite3_کالم_decltype16      0
# define sqlite3_کالم_decltype        0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_ترقی_CALLBACK
# define sqlite3_ترقی_handler 0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
# define sqlite3_create_module 0
# define sqlite3_create_module_v2 0
# define sqlite3_declare_vtab 0
# define sqlite3_vtab_config 0
# define sqlite3_vtab_on_conflict 0
# define sqlite3_vtab_collation 0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
# define sqlite3_enable_shared_cache 0
#جے_انت

#إذا defined(SQLITE_OMIT_TRACE) || defined(SQLITE_OMIT_DEPRECATED)
# define sqlite3_profile       0
# define sqlite3_trace         0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_GET_جدول
# define sqlite3_free_جدول    0
# define sqlite3_get_جدول     0
#جے_انت

#جے_تعریف_شدہ SQLITE_OMIT_INCRBLOB
#تعریف_کرو sqlite3_bind_zeroblob  0
#تعریف_کرو sqlite3_blob_بائٹاں     0
#تعریف_کرو sqlite3_blob_close     0
#تعریف_کرو sqlite3_blob_open      0
#تعریف_کرو sqlite3_blob_read      0
#تعریف_کرو sqlite3_blob_write     0
#تعریف_کرو sqlite3_blob_reopen    0
#جے_انت

#إذا defined(SQLITE_OMIT_TRACE)
# define sqlite3_trace_v2      0
#جے_انت

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
ساكن ثابت sqlite3_api_routines sqlite3Apis = {
  sqlite3_aggregate_context,
#جے_بنا_تعریف SQLITE_OMIT_DEPRECATED
  sqlite3_aggregate_گنتی,
#وإلا
  0,
#جے_انت
  sqlite3_bind_blob,
  sqlite3_bind_double,
  sqlite3_bind_int,
  sqlite3_bind_int64,
  sqlite3_bind_null,
  sqlite3_bind_parameter_گنتی,
  sqlite3_bind_parameter_اشاریہ,
  sqlite3_bind_parameter_name,
  sqlite3_bind_متن,
  sqlite3_bind_text16,
  sqlite3_bind_قدر,
  sqlite3_busy_handler,
  sqlite3_busy_timeout,
  sqlite3_changes,
  sqlite3_close,
  sqlite3_collation_needed,
  sqlite3_collation_needed16,
  sqlite3_کالم_blob,
  sqlite3_کالم_بائٹاں,
  sqlite3_کالم_bytes16,
  sqlite3_کالم_گنتی,
  sqlite3_کالم_database_name,
  sqlite3_کالم_database_name16,
  sqlite3_کالم_decltype,
  sqlite3_کالم_decltype16,
  sqlite3_کالم_double,
  sqlite3_کالم_int,
  sqlite3_کالم_int64,
  sqlite3_کالم_name,
  sqlite3_کالم_name16,
  sqlite3_کالم_origin_name,
  sqlite3_کالم_origin_name16,
  sqlite3_کالم_جدول_name,
  sqlite3_کالم_جدول_name16,
  sqlite3_کالم_متن,
  sqlite3_کالم_text16,
  sqlite3_کالم_type,
  sqlite3_کالم_قدر,
  sqlite3_commit_hook,
  sqlite3_complete,
  sqlite3_complete16,
  sqlite3_create_collation,
  sqlite3_create_collation16,
  sqlite3_create_function,
  sqlite3_create_function16,
  sqlite3_create_module,
  sqlite3_انکڑے_گنتی,
  sqlite3_db_handle,
  sqlite3_declare_vtab,
  sqlite3_enable_shared_cache,
  sqlite3_errcode,
  sqlite3_errmsg,
  sqlite3_errmsg16,
  sqlite3_exec,
#جے_بنا_تعریف SQLITE_OMIT_DEPRECATED
  sqlite3_expired,
#وإلا
  0,
#جے_انت
  sqlite3_finalize,
  sqlite3_free,
  sqlite3_free_جدول,
  sqlite3_get_autocommit,
  sqlite3_get_auxdata,
  sqlite3_get_جدول,
  0,     /* Was sqlite3_global_recover(), but that function is deprecated */
  sqlite3_رکاوٹ,
  sqlite3_آخری_داخل_کرو_rowid,
  sqlite3_libversion,
  sqlite3_libversion_نمبر,
  sqlite3_malloc,
  sqlite3_mprintf,
  sqlite3_open,
  sqlite3_open16,
  sqlite3_prepare,
  sqlite3_prepare16,
  sqlite3_profile,
  sqlite3_ترقی_handler,
  sqlite3_realloc,
  sqlite3_reset,
  sqlite3_نتیجہ_blob,
  sqlite3_نتیجہ_double,
  sqlite3_نتیجہ_error,
  sqlite3_نتیجہ_error16,
  sqlite3_نتیجہ_int,
  sqlite3_نتیجہ_int64,
  sqlite3_نتیجہ_null,
  sqlite3_نتیجہ_متن,
  sqlite3_نتیجہ_text16,
  sqlite3_نتیجہ_text16be,
  sqlite3_نتیجہ_text16le,
  sqlite3_نتیجہ_قدر,
  sqlite3_rollback_hook,
  sqlite3_سیٹ_authorizer,
  sqlite3_سیٹ_auxdata,
  sqlite3_snprintf,
  sqlite3_قدم,
  sqlite3_جدول_کالم_metadata,
#جے_بنا_تعریف SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup,
#وإلا
  0,
#جے_انت
  sqlite3_کل_changes,
  sqlite3_trace,
#جے_بنا_تعریف SQLITE_OMIT_DEPRECATED
  sqlite3_transfer_bindings,
#وإلا
  0,
#جے_انت
  sqlite3_update_hook,
  sqlite3_user_انکڑے,
  sqlite3_قدر_blob,
  sqlite3_قدر_بائٹاں,
  sqlite3_قدر_bytes16,
  sqlite3_قدر_double,
  sqlite3_قدر_int,
  sqlite3_قدر_int64,
  sqlite3_قدر_numeric_type,
  sqlite3_قدر_متن,
  sqlite3_قدر_text16,
  sqlite3_قدر_text16be,
  sqlite3_قدر_text16le,
  sqlite3_قدر_type,
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
  sqlite3_blob_بائٹاں,
  sqlite3_blob_close,
  sqlite3_blob_open,
  sqlite3_blob_read,
  sqlite3_blob_write,
  sqlite3_create_collation_v2,
  sqlite3_file_control,
  sqlite3_حافظہ_highwater,
  sqlite3_حافظہ_ورتیا,
#جے_تعریف_شدہ SQLITE_MUTEX_OMIT
  0, 
  0, 
  0,
  0,
  0,
#وإلا
  sqlite3_mutex_alloc,
  sqlite3_mutex_enter,
  sqlite3_mutex_free,
  sqlite3_mutex_leave,
  sqlite3_mutex_try,
#جے_انت
  sqlite3_open_v2,
  sqlite3_release_حافظہ,
  sqlite3_نتیجہ_error_nomem,
  sqlite3_نتیجہ_error_toobig,
  sqlite3_sleep,
  sqlite3_soft_ڈھیر_حد_بندی,
  sqlite3_vfs_لبھو,
  sqlite3_vfs_رجسٹر,
  sqlite3_vfs_unregister,

  /*
  ** Added for 3.5.8
  */
  sqlite3_threadsafe,
  sqlite3_نتیجہ_zeroblob,
  sqlite3_نتیجہ_error_کوڈ,
  sqlite3_test_control,
  sqlite3_randomness,
  sqlite3_context_db_handle,

  /*
  ** Added for 3.6.0
  */
  sqlite3_extended_نتیجہ_codes,
  sqlite3_حد_بندی,
  sqlite3_اگلا_stmt,
  sqlite3_sql,
  sqlite3_حالت,

  /*
  ** Added for 3.7.4
  */
  sqlite3_backup_انت,
  sqlite3_backup_init,
  sqlite3_backup_pagecount,
  sqlite3_backup_بچیا,
  sqlite3_backup_قدم,
#جے_بنا_تعریف SQLITE_OMIT_COMPILEOPTION_DIAGS
  sqlite3_compileoption_get,
  sqlite3_compileoption_ورتیا,
#وإلا
  0,
  0,
#جے_انت
  sqlite3_create_function_v2,
  sqlite3_db_config,
  sqlite3_db_mutex,
  sqlite3_db_حالت,
  sqlite3_extended_errcode,
  sqlite3_log,
  sqlite3_soft_ڈھیر_limit64,
  sqlite3_sourceid,
  sqlite3_stmt_حالت,
  sqlite3_strnicmp,
#جے_تعریف_شدہ SQLITE_ENABLE_UNLOCK_NOTIFY
  sqlite3_unlock_notify,
#وإلا
  0,
#جے_انت
#جے_بنا_تعریف SQLITE_OMIT_WAL
  sqlite3_wal_autocheckpoint,
  sqlite3_wal_checkpoint,
  sqlite3_wal_hook,
#وإلا
  0,
  0,
  0,
#جے_انت
  sqlite3_blob_reopen,
  sqlite3_vtab_config,
  sqlite3_vtab_on_conflict,
  sqlite3_close_v2,
  sqlite3_db_filename,
  sqlite3_db_readonly,
  sqlite3_db_release_حافظہ,
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
  sqlite3_لدو_extension,
  sqlite3_malloc64,
  sqlite3_msize,
  sqlite3_realloc64,
  sqlite3_reset_auto_extension,
  sqlite3_نتیجہ_blob64,
  sqlite3_نتیجہ_text64,
  sqlite3_strglob,
  /* Version 3.8.11 and later */
  (sqlite3_قدر*(*)(ثابت sqlite3_قدر*))sqlite3_قدر_dup,
  sqlite3_قدر_free,
  sqlite3_نتیجہ_zeroblob64,
  sqlite3_bind_zeroblob64,
  /* Version 3.9.0 and later */
  sqlite3_قدر_subtype,
  sqlite3_نتیجہ_subtype,
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
  sqlite3_سیٹ_آخری_داخل_کرو_rowid,
  /* Version 3.20.0 and later */
  sqlite3_prepare_v3,
  sqlite3_prepare16_v3,
  sqlite3_bind_اشارہ_گر,
  sqlite3_نتیجہ_اشارہ_گر,
  sqlite3_قدر_اشارہ_گر,
  /* Version 3.22.0 and later */
  sqlite3_vtab_nochange,
  sqlite3_قدر_nochange,
  sqlite3_vtab_collation,
  /* Version 3.24.0 and later */
  sqlite3_keyword_گنتی,
  sqlite3_keyword_name,
  sqlite3_keyword_check,
  sqlite3_str_new,
  sqlite3_str_انت,
  sqlite3_str_appendf,
  sqlite3_str_vappendf,
  sqlite3_str_append,
  sqlite3_str_appendall,
  sqlite3_str_appendchar,
  sqlite3_str_reset,
  sqlite3_str_errcode,
  sqlite3_str_لمائی,
  sqlite3_str_قدر,
  /* Version 3.25.0 and later */
  sqlite3_create_کھڑکی_function,
  /* Version 3.26.0 and later */
#جے_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
  sqlite3_normalized_sql,
#وإلا
  0,
#جے_انت
  /* Version 3.28.0 and later */
  sqlite3_stmt_isexplain,
  sqlite3_قدر_frombind,
  /* Version 3.30.0 and later */
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  sqlite3_drop_modules,
#وإلا
  0,
#جے_انت
  /* Version 3.31.0 and later */
  sqlite3_hard_ڈھیر_limit64,
  sqlite3_uri_کلید,
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
  sqlite3_کل_changes64,
  /* Version 3.37.0 and later */
  sqlite3_autovacuum_ورقے,
  /* Version 3.38.0 and later */
  sqlite3_error_وتھ,
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  sqlite3_vtab_rhs_قدر,
  sqlite3_vtab_distinct,
  sqlite3_vtab_وچ,
  sqlite3_vtab_وچ_پہلا,
  sqlite3_vtab_وچ_اگلا,
#وإلا
  0,
  0,
  0,
  0,
  0,
#جے_انت
  /* Version 3.39.0 and later */
#جے_بنا_تعریف SQLITE_OMIT_DESERIALIZE
  sqlite3_deserialize,
  sqlite3_serialize,
#وإلا
  0,
  0,
#جے_انت
  sqlite3_db_name,
  /* Version 3.40.0 and later */
  sqlite3_قدر_encoding,
  /* Version 3.41.0 and later */
  sqlite3_is_interrupted,
  /* Version 3.43.0 and later */
  sqlite3_stmt_explain,
  /* Version 3.44.0 and later */
  sqlite3_get_clientdata,
  sqlite3_سیٹ_clientdata,
  /* Version 3.50.0 and later */
  sqlite3_setlk_timeout,
  /* Version 3.51.0 and later */
  sqlite3_سیٹ_errmsg,
  sqlite3_db_status64,
  /* Version 3.52.0 and later */
  sqlite3_str_truncate,
  sqlite3_str_free,
#جے_تعریف_شدہ SQLITE_ENABLE_CARRAY
  sqlite3_carray_bind,
  sqlite3_carray_bind_v2,
#وإلا
  0,
  0,
#جے_انت
  /* Version 3.54.0 and later */
  sqlite3_incomplete,
  sqlite3_نتیجہ_str
};

/* True if x is the directory separator character
*/
#إذا SQLITE_OS_WIN
# define DirSep(X)  ((X)=='/'||(X)=='\\')
#وإلا
# define DirSep(X)  ((X)=='/')
#جے_انت

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
ساكن صحيح sqlite3_لدو_Extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ثابت حرف *zFile,    /* Name of the shared library containing extension */
  ثابت حرف *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  حرف **pz_Err_پیغام       /* Put error message here if not 0 */
){
  sqlite3_vfs *pVfs = db->pVfs;
  فراغ *handle;
  sqlite3_loadext_اندراج xInit;
  حرف *zErrmsg = 0;
  ثابت حرف *z_اندراج;
  حرف *z_Alt_اندراج = 0;
  فراغ **aHandle;
  u64 n_پیغام = لڑی_لمبائی(zFile);
  صحيح ii;
  صحيح rc;

  /* Shared library endings to try if zFile cannot be loaded as written */
  ساكن ثابت حرف *azEndings[] = {
#إذا SQLITE_OS_WIN
     "dll"   
#وإلاإذا defined(__APPLE__)
     "dylib"
#وإلا
     "so"
#جے_انت
  };


  إذا( pz_Err_پیغام ) *pz_Err_پیغام = 0;

  /* Ticket #1863.  To avoid a creating security problems for older
  ** applications that relink against newer versions of SQLite, the
  ** ability to run load_extension is turned off by default.  One
  ** must call either sqlite3_enable_load_extension(db) or
  ** sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, 0)
  ** to turn on extension loading.
  */
  إذا( (db->flags & SQLITE_لدو_Extension)==0 ){
    إذا( pz_Err_پیغام ){
      *pz_Err_پیغام = sqlite3_mprintf("not authorized");
    }
    أرجع SQLITE_ERROR;
  }

  z_اندراج = zProc ? zProc : "sqlite3_extension_init";

  /* tag-20210611-1.  Some dlopen() implementations will segfault if given
  ** an oversize filename.  Most filesystems have a pathname limit of 4K,
  ** so limit the extension filename length to about twice that.
  ** https://sqlite.org/forum/forumpost/08a0d6d9bf
  **
  ** Later (2023-03-25): Save an extra 6 bytes for the filename suffix.
  ** See https://sqlite.org/forum/forumpost/24083b579d.
  */
  إذا( n_پیغام>SQLITE_MAX_PATHLEN ) اذهب extension_not_لبھیا;

  /* Do not allow sqlite3_load_extension() to link to a copy of the
  ** running application, by passing in an empty filename. */
  إذا( n_پیغام==0 ) اذهب extension_not_لبھیا;
    
  handle = sqlite3OsDlOpen(pVfs, zFile);
#إذا SQLITE_OS_UNIX || SQLITE_OS_WIN
  لكل(ii=0; ii<Array_ناپ(azEndings) && handle==0; ii++){
    حرف *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    إذا( zAltFile==0 ) أرجع SQLITE_NOMEM_BKPT;
    إذا( n_پیغام+لڑی_لمبائی(azEndings[ii])+1<=SQLITE_MAX_PATHLEN ){
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }
#جے_انت
  إذا( handle==0 ) اذهب extension_not_لبھیا;
  xInit = (sqlite3_loadext_اندراج)sqlite3OsDlSym(pVfs, handle, z_اندراج);

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
  إذا( xInit==0 && zProc==0 ){
    صحيح iFile, i_اندراج, c;
    صحيح ncFile = sqlite3Strlen30(zFile);
    صحيح cnt = 0;
    z_Alt_اندراج = sqlite3_malloc64(ncFile+30);
    إذا( z_Alt_اندراج==0 ){
      sqlite3OsDlClose(pVfs, handle);
      أرجع SQLITE_NOMEM_BKPT;
    }
    افعل{
      حافظہ_نقل(z_Alt_اندراج, "sqlite3_", 8);
      لكل(iFile=ncFile-1; iFile>=0 && !DirSep(zFile[iFile]); iFile--){}
      iFile++;
      إذا( sqlite3_strnicmp(zFile+iFile, "lib", 3)==0 ) iFile += 3;
      لكل(i_اندراج=8; (c = zFile[iFile])!=0 && c!='.'; iFile++){
        إذا( sqlite3Isalpha(c) || (cnt && sqlite3Isdigit(c)) ){
          z_Alt_اندراج[i_اندراج++] = (حرف)sqlite3_اتلا_منزل_ہیٹھلا[(بنانشانی)c];
        }
      }
      حافظہ_نقل(z_Alt_اندراج+i_اندراج, "_init", 6);
      z_اندراج = z_Alt_اندراج;
      xInit = (sqlite3_loadext_اندراج)sqlite3OsDlSym(pVfs, handle, z_اندراج);
    }طالما( xInit==0 && (++cnt)<2 );
  }
  إذا( xInit==0 ){
    إذا( pz_Err_پیغام ){
      n_پیغام += لڑی_لمبائی(z_اندراج) + 300;
      *pz_Err_پیغام = zErrmsg = sqlite3_malloc64(n_پیغام);
      إذا( zErrmsg ){
        أكد( n_پیغام<0x7fffffff );  /* zErrmsg would be NULL if not so */
        sqlite3_snprintf((صحيح)n_پیغام, zErrmsg,
            "no entry point [%s] in shared library [%s]", z_اندراج, zFile);
        sqlite3OsDlError(pVfs, n_پیغام-1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(z_Alt_اندراج);
    أرجع SQLITE_ERROR;
  }
  sqlite3_free(z_Alt_اندراج);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  إذا( rc ){
    إذا( rc==SQLITE_OK_لدو_PERMANENTLY ) أرجع SQLITE_OK;
    إذا( pz_Err_پیغام ){
      *pz_Err_پیغام = sqlite3_mprintf("error during initialization: %s", zErrmsg);
    }
    sqlite3_free(zErrmsg);
    sqlite3OsDlClose(pVfs, handle);
    أرجع SQLITE_ERROR;
  }

  /* Append the new shared library handle to the db->aExtension array. */
  aHandle = sqlite3DbMallocZero(db, حجم(handle)*(db->nExtension+1));
  إذا( aHandle==0 ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  إذا( db->nExtension>0 ){
    حافظہ_نقل(aHandle, db->aExtension, حجم(handle)*db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  أرجع SQLITE_OK;

extension_not_لبھیا:
  إذا( pz_Err_پیغام ){
    n_پیغام += 300;
    *pz_Err_پیغام = zErrmsg = sqlite3_malloc64(n_پیغام);
    إذا( zErrmsg ){
      أكد( n_پیغام<0x7fffffff );  /* zErrmsg would be NULL if not so */
      sqlite3_snprintf((صحيح)n_پیغام, zErrmsg,
          "unable to open shared library [%.*s]", SQLITE_MAX_PATHLEN, zFile);
      sqlite3OsDlError(pVfs, n_پیغام-1, zErrmsg);
    }
  }
  أرجع SQLITE_ERROR;
}
صحيح sqlite3_لدو_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ثابت حرف *zFile,    /* Name of the shared library containing extension */
  ثابت حرف *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  حرف **pz_Err_پیغام       /* Put error message here if not 0 */
){
  صحيح rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_لدو_Extension(db, zFile, zProc, pz_Err_پیغام);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}

/*
** Call this routine when the database connection is closing in order
** to clean up loaded extensions
*/
فراغ sqlite3CloseExtensions(sqlite3 *db){
  صحيح i;
  أكد( sqlite3_mutex_held(db->mutex) );
  لكل(i=0; i<db->nExtension; i++){
    sqlite3OsDlClose(db->pVfs, db->aExtension[i]);
  }
  sqlite3DbFree(db, db->aExtension);
}

/*
** Enable or disable extension loading.  Extension loading is disabled by
** default so as not to open security holes in older applications.
*/
صحيح sqlite3_enable_لدو_extension(sqlite3 *db, صحيح onoff){
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
  sqlite3_mutex_enter(db->mutex);
  إذا( onoff ){
    db->flags |= SQLITE_لدو_Extension|SQLITE_لدو_Ext_Func;
  }وإلا{
    db->flags &= ~(u64)(SQLITE_لدو_Extension|SQLITE_لدو_Ext_Func);
  }
  sqlite3_mutex_leave(db->mutex);
  أرجع SQLITE_OK;
}

#جے_انت /* !defined(SQLITE_OMIT_LOAD_EXTENSION) */

/*
** The following object holds the list of automatically loaded
** extensions.
**
** This list is shared across threads.  The SQLITE_MUTEX_STATIC_MAIN
** mutex must be held while accessing this list.
*/
تعريفنوع بنية sqlite3AutoExtList sqlite3AutoExtList;
ساكن SQLITE_WSD بنية sqlite3AutoExtList {
  u32 nExt;              /* Number of entries in aExt[] */
  فراغ (**aExt)(فراغ);   /* Pointers to the extension init functions */
} sqlite3Autoext = { 0, 0 };

/* The "wsdAutoext" macro will resolve to the autoextension
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Autoext" state vector declared above.
*/
#جے_تعریف_شدہ SQLITE_OMIT_WSD
# define wsdAutoextInit \
  sqlite3AutoExtList *x = &GLOBAL(sqlite3AutoExtList,sqlite3Autoext)
# define wsdAutoext x[0]
#وإلا
# define wsdAutoextInit
# define wsdAutoext sqlite3Autoext
#جے_انت


/*
** Register a statically linked extension that is automatically
** loaded by every new database connection.
*/
صحيح sqlite3_auto_extension(
  فراغ (*xInit)(فراغ)
){
  صحيح rc = SQLITE_OK;
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( xInit==0 ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
#جے_بنا_تعریف SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  إذا( rc ){
    أرجع rc;
  }وإلا
#جے_انت
  {
    u32 i;
#إذا SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#جے_انت
    wsdAutoextInit;
    sqlite3_mutex_enter(mutex);
    لكل(i=0; i<wsdAutoext.nExt; i++){
      إذا( wsdAutoext.aExt[i]==xInit ) اكسر;
    }
    إذا( i==wsdAutoext.nExt ){
      u64 nByte = (wsdAutoext.nExt+1)*حجم(wsdAutoext.aExt[0]);
      فراغ (**aNew)(فراغ);
      aNew = sqlite3_realloc64(wsdAutoext.aExt, nByte);
      إذا( aNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }وإلا{
        wsdAutoext.aExt = aNew;
        wsdAutoext.aExt[wsdAutoext.nExt] = xInit;
        wsdAutoext.nExt++;
      }
    }
    sqlite3_mutex_leave(mutex);
    أكد( (rc&0xff)==rc );
    أرجع rc;
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
صحيح sqlite3_cancel_auto_extension(
  فراغ (*xInit)(فراغ)
){
  صحيح n = 0;
#جے_بنا_تعریف SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ){
    أرجع 0;
  }وإلا
#جے_انت
  {
#إذا SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#جے_انت
    صحيح i;
    wsdAutoextInit;
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
    إذا( xInit==0 ) أرجع 0;
#جے_انت
    sqlite3_mutex_enter(mutex);
    لكل(i=(صحيح)wsdAutoext.nExt-1; i>=0; i--){
      إذا( wsdAutoext.aExt[i]==xInit ){
        wsdAutoext.nExt--;
        wsdAutoext.aExt[i] = wsdAutoext.aExt[wsdAutoext.nExt];
        n++;
        اكسر;
      }
    }
    sqlite3_mutex_leave(mutex);
  }
  أرجع n;
}

/*
** Reset the automatic extension loading mechanism.
*/
فراغ sqlite3_reset_auto_extension(فراغ){
#جے_بنا_تعریف SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize()==SQLITE_OK )
#جے_انت
  {
#إذا SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#جے_انت
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
فراغ sqlite3_Auto_لدو_Extensions(sqlite3 *db){
  u32 i;
  صحيح go = 1;
  صحيح rc;
  sqlite3_loadext_اندراج xInit;

  wsdAutoextInit;
  إذا( wsdAutoext.nExt==0 ){
    /* Common case: early out without every having to acquire a mutex */
    أرجع;
  }
  لكل(i=0; go; i++){
    حرف *zErrmsg;
#إذا SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#جے_انت
#جے_تعریف_شدہ SQLITE_OMIT_لدو_EXTENSION
    ثابت sqlite3_api_routines *pThunk = 0;
#وإلا
    ثابت sqlite3_api_routines *pThunk = &sqlite3Apis;
#جے_انت
    sqlite3_mutex_enter(mutex);
    إذا( i>=wsdAutoext.nExt ){
      xInit = 0;
      go = 0;
    }وإلا{
      xInit = (sqlite3_loadext_اندراج)wsdAutoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    إذا( xInit && (rc = xInit(db, &zErrmsg, pThunk))!=0 ){
      sqlite3_Error_With_پیغام(db, rc,
            "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}
