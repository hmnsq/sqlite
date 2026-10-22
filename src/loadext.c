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

#اگر_غیر_تعریف_شدہ SQLITE_CORE
  #تعریف_کرو SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#اگر_ختم
#شامل "sqlite3ext.h"
#شامل "sqliteInt.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
/*
** Some API routines are omitted when various features are
** excluded from a build of SQLite.  Substitute a NULL pointer
** for any missing APIs.
*/
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA
# define sqlite3_عمود_database_name   0
# define sqlite3_عمود_database_name16 0
# define sqlite3_عمود_جدول_name      0
# define sqlite3_عمود_جدول_name16    0
# define sqlite3_عمود_origin_name     0
# define sqlite3_عمود_origin_name16   0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
# define sqlite3_سیٹ_authorizer         0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_UTF16
# define sqlite3_bind_text16            0
# define sqlite3_collation_needed16     0
# define sqlite3_عمود_decltype16      0
# define sqlite3_عمود_name16          0
# define sqlite3_عمود_text16          0
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
# define sqlite3_عمود_database_name16 0
# define sqlite3_عمود_جدول_name16    0
# define sqlite3_عمود_origin_name16   0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_COMPLETE
# define sqlite3_complete 0
# define sqlite3_complete16 0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_DECLTYPE
# define sqlite3_عمود_decltype16      0
# define sqlite3_عمود_decltype        0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_پیش_رفت_CALLBACK
# define sqlite3_پیش_رفت_handler 0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
# define sqlite3_create_module 0
# define sqlite3_create_module_v2 0
# define sqlite3_declare_vtab 0
# define sqlite3_vtab_config 0
# define sqlite3_vtab_on_conflict 0
# define sqlite3_vtab_collation 0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
# define sqlite3_enable_shared_cache 0
#اگر_ختم

#اگر defined(SQLITE_OMIT_TRACE) || defined(SQLITE_OMIT_DEPRECATED)
# define sqlite3_profile       0
# define sqlite3_trace         0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_GET_جدول
# define sqlite3_free_جدول    0
# define sqlite3_get_جدول     0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
#تعریف_کرو sqlite3_bind_zeroblob  0
#تعریف_کرو sqlite3_blob_بائٹیں     0
#تعریف_کرو sqlite3_blob_close     0
#تعریف_کرو sqlite3_blob_open      0
#تعریف_کرو sqlite3_blob_read      0
#تعریف_کرو sqlite3_blob_write     0
#تعریف_کرو sqlite3_blob_reopen    0
#اگر_ختم

#اگر defined(SQLITE_OMIT_TRACE)
# define sqlite3_trace_v2      0
#اگر_ختم

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
ساکن ثابت sqlite3_api_routines sqlite3Apis = {
  sqlite3_aggregate_context,
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
  sqlite3_aggregate_گنتی,
#ورنہ
  0,
#اگر_ختم
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
  sqlite3_عمود_blob,
  sqlite3_عمود_بائٹیں,
  sqlite3_عمود_bytes16,
  sqlite3_عمود_گنتی,
  sqlite3_عمود_database_name,
  sqlite3_عمود_database_name16,
  sqlite3_عمود_decltype,
  sqlite3_عمود_decltype16,
  sqlite3_عمود_double,
  sqlite3_عمود_int,
  sqlite3_عمود_int64,
  sqlite3_عمود_name,
  sqlite3_عمود_name16,
  sqlite3_عمود_origin_name,
  sqlite3_عمود_origin_name16,
  sqlite3_عمود_جدول_name,
  sqlite3_عمود_جدول_name16,
  sqlite3_عمود_متن,
  sqlite3_عمود_text16,
  sqlite3_عمود_type,
  sqlite3_عمود_قدر,
  sqlite3_commit_hook,
  sqlite3_complete,
  sqlite3_complete16,
  sqlite3_create_collation,
  sqlite3_create_collation16,
  sqlite3_create_function,
  sqlite3_create_function16,
  sqlite3_create_module,
  sqlite3_ڈیٹا_گنتی,
  sqlite3_db_handle,
  sqlite3_declare_vtab,
  sqlite3_enable_shared_cache,
  sqlite3_errcode,
  sqlite3_errmsg,
  sqlite3_errmsg16,
  sqlite3_exec,
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
  sqlite3_expired,
#ورنہ
  0,
#اگر_ختم
  sqlite3_finalize,
  sqlite3_free,
  sqlite3_free_جدول,
  sqlite3_get_autocommit,
  sqlite3_get_auxdata,
  sqlite3_get_جدول,
  0,     /* Was sqlite3_global_recover(), but that function is deprecated */
  sqlite3_مداخلت,
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
  sqlite3_پیش_رفت_handler,
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
  sqlite3_جدول_عمود_metadata,
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup,
#ورنہ
  0,
#اگر_ختم
  sqlite3_کل_changes,
  sqlite3_trace,
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
  sqlite3_transfer_bindings,
#ورنہ
  0,
#اگر_ختم
  sqlite3_update_hook,
  sqlite3_user_ڈیٹا,
  sqlite3_قدر_blob,
  sqlite3_قدر_بائٹیں,
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
  sqlite3_blob_بائٹیں,
  sqlite3_blob_close,
  sqlite3_blob_open,
  sqlite3_blob_read,
  sqlite3_blob_write,
  sqlite3_create_collation_v2,
  sqlite3_file_control,
  sqlite3_حافظہ_highwater,
  sqlite3_حافظہ_مستعمل,
#اگر_تعریف_شدہ SQLITE_MUTEX_OMIT
  0, 
  0, 
  0,
  0,
  0,
#ورنہ
  sqlite3_mutex_alloc,
  sqlite3_mutex_enter,
  sqlite3_mutex_free,
  sqlite3_mutex_leave,
  sqlite3_mutex_try,
#اگر_ختم
  sqlite3_open_v2,
  sqlite3_release_حافظہ,
  sqlite3_نتیجہ_error_nomem,
  sqlite3_نتیجہ_error_toobig,
  sqlite3_sleep,
  sqlite3_soft_ڈھیر_حد_بندی,
  sqlite3_vfs_ڈھونڈو,
  sqlite3_vfs_رجسٹر,
  sqlite3_vfs_unregister,

  /*
  ** Added for 3.5.8
  */
  sqlite3_threadsafe,
  sqlite3_نتیجہ_zeroblob,
  sqlite3_نتیجہ_error_رمز,
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
  sqlite3_backup_اختتام,
  sqlite3_backup_init,
  sqlite3_backup_pagecount,
  sqlite3_backup_بقایا,
  sqlite3_backup_قدم,
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPILEOPTION_DIAGS
  sqlite3_compileoption_get,
  sqlite3_compileoption_مستعمل,
#ورنہ
  0,
  0,
#اگر_ختم
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
#اگر_تعریف_شدہ SQLITE_ENABLE_UNLOCK_NOTIFY
  sqlite3_unlock_notify,
#ورنہ
  0,
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  sqlite3_wal_autocheckpoint,
  sqlite3_wal_checkpoint,
  sqlite3_wal_hook,
#ورنہ
  0,
  0,
  0,
#اگر_ختم
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
  sqlite3_بھرو_extension,
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
  sqlite3_str_اختتام,
  sqlite3_str_appendf,
  sqlite3_str_vappendf,
  sqlite3_str_append,
  sqlite3_str_appendall,
  sqlite3_str_appendchar,
  sqlite3_str_reset,
  sqlite3_str_errcode,
  sqlite3_str_لمبائی,
  sqlite3_str_قدر,
  /* Version 3.25.0 and later */
  sqlite3_create_کھڑکی_function,
  /* Version 3.26.0 and later */
#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
  sqlite3_normalized_sql,
#ورنہ
  0,
#اگر_ختم
  /* Version 3.28.0 and later */
  sqlite3_stmt_isexplain,
  sqlite3_قدر_frombind,
  /* Version 3.30.0 and later */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_drop_modules,
#ورنہ
  0,
#اگر_ختم
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
  sqlite3_autovacuum_صفحات,
  /* Version 3.38.0 and later */
  sqlite3_error_انحراف,
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_vtab_rhs_قدر,
  sqlite3_vtab_distinct,
  sqlite3_vtab_اندر,
  sqlite3_vtab_اندر_پہلا,
  sqlite3_vtab_اندر_اگلا,
#ورنہ
  0,
  0,
  0,
  0,
  0,
#اگر_ختم
  /* Version 3.39.0 and later */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DESERIALIZE
  sqlite3_deserialize,
  sqlite3_serialize,
#ورنہ
  0,
  0,
#اگر_ختم
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
#اگر_تعریف_شدہ SQLITE_ENABLE_CARRAY
  sqlite3_carray_bind,
  sqlite3_carray_bind_v2,
#ورنہ
  0,
  0,
#اگر_ختم
  /* Version 3.54.0 and later */
  sqlite3_incomplete,
  sqlite3_نتیجہ_str
};

/* True if x is the directory separator character
*/
#اگر SQLITE_OS_WIN
# define DirSep(X)  ((X)=='/'||(X)=='\\')
#ورنہ
# define DirSep(X)  ((X)=='/')
#اگر_ختم

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
ساکن صحیح sqlite3_بھرو_Extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ثابت حرف *zFile,    /* Name of the shared library containing extension */
  ثابت حرف *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  حرف **pz_Err_پیغام       /* Put error message here if not 0 */
){
  sqlite3_vfs *pVfs = db->pVfs;
  خالی *handle;
  sqlite3_loadext_اندراج xInit;
  حرف *zErrmsg = 0;
  ثابت حرف *z_اندراج;
  حرف *z_Alt_اندراج = 0;
  خالی **aHandle;
  u64 n_پیغام = لڑی_لمبائی(zFile);
  صحیح ii;
  صحیح rc;

  /* Shared library endings to try if zFile cannot be loaded as written */
  ساکن ثابت حرف *azEndings[] = {
#اگر SQLITE_OS_WIN
     "dll"   
#وگرنہ_اگر defined(__APPLE__)
     "dylib"
#ورنہ
     "so"
#اگر_ختم
  };


  اگر( pz_Err_پیغام ) *pz_Err_پیغام = 0;

  /* Ticket #1863.  To avoid a creating security problems for older
  ** applications that relink against newer versions of SQLite, the
  ** ability to run load_extension is turned off by default.  One
  ** must call either sqlite3_enable_load_extension(db) or
  ** sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, 0)
  ** to turn on extension loading.
  */
  اگر( (db->flags & SQLITE_بھرو_Extension)==0 ){
    اگر( pz_Err_پیغام ){
      *pz_Err_پیغام = sqlite3_mprintf("not authorized");
    }
    واپس SQLITE_ERROR;
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
  اگر( n_پیغام>SQLITE_MAX_PATHLEN ) جاؤ extension_not_ملا;

  /* Do not allow sqlite3_load_extension() to link to a copy of the
  ** running application, by passing in an empty filename. */
  اگر( n_پیغام==0 ) جاؤ extension_not_ملا;
    
  handle = sqlite3OsDlOpen(pVfs, zFile);
#اگر SQLITE_OS_UNIX || SQLITE_OS_WIN
  برائے(ii=0; ii<Array_سائز(azEndings) && handle==0; ii++){
    حرف *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    اگر( zAltFile==0 ) واپس SQLITE_NOMEM_BKPT;
    اگر( n_پیغام+لڑی_لمبائی(azEndings[ii])+1<=SQLITE_MAX_PATHLEN ){
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }
#اگر_ختم
  اگر( handle==0 ) جاؤ extension_not_ملا;
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
  اگر( xInit==0 && zProc==0 ){
    صحیح iFile, i_اندراج, c;
    صحیح ncFile = sqlite3Strlen30(zFile);
    صحیح cnt = 0;
    z_Alt_اندراج = sqlite3_malloc64(ncFile+30);
    اگر( z_Alt_اندراج==0 ){
      sqlite3OsDlClose(pVfs, handle);
      واپس SQLITE_NOMEM_BKPT;
    }
    کرو{
      حافظہ_نقل(z_Alt_اندراج, "sqlite3_", 8);
      برائے(iFile=ncFile-1; iFile>=0 && !DirSep(zFile[iFile]); iFile--){}
      iFile++;
      اگر( sqlite3_strnicmp(zFile+iFile, "lib", 3)==0 ) iFile += 3;
      برائے(i_اندراج=8; (c = zFile[iFile])!=0 && c!='.'; iFile++){
        اگر( sqlite3Isalpha(c) || (cnt && sqlite3Isdigit(c)) ){
          z_Alt_اندراج[i_اندراج++] = (حرف)sqlite3_بالائی_مقصد_زیریں[(غیرعلامتی)c];
        }
      }
      حافظہ_نقل(z_Alt_اندراج+i_اندراج, "_init", 6);
      z_اندراج = z_Alt_اندراج;
      xInit = (sqlite3_loadext_اندراج)sqlite3OsDlSym(pVfs, handle, z_اندراج);
    }جب( xInit==0 && (++cnt)<2 );
  }
  اگر( xInit==0 ){
    اگر( pz_Err_پیغام ){
      n_پیغام += لڑی_لمبائی(z_اندراج) + 300;
      *pz_Err_پیغام = zErrmsg = sqlite3_malloc64(n_پیغام);
      اگر( zErrmsg ){
        أكد( n_پیغام<0x7fffffff );  /* zErrmsg would be NULL if not so */
        sqlite3_snprintf((صحیح)n_پیغام, zErrmsg,
            "no entry point [%s] in shared library [%s]", z_اندراج, zFile);
        sqlite3OsDlError(pVfs, n_پیغام-1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(z_Alt_اندراج);
    واپس SQLITE_ERROR;
  }
  sqlite3_free(z_Alt_اندراج);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  اگر( rc ){
    اگر( rc==SQLITE_OK_بھرو_PERMANENTLY ) واپس SQLITE_OK;
    اگر( pz_Err_پیغام ){
      *pz_Err_پیغام = sqlite3_mprintf("error during initialization: %s", zErrmsg);
    }
    sqlite3_free(zErrmsg);
    sqlite3OsDlClose(pVfs, handle);
    واپس SQLITE_ERROR;
  }

  /* Append the new shared library handle to the db->aExtension array. */
  aHandle = sqlite3DbMallocZero(db, حجم(handle)*(db->nExtension+1));
  اگر( aHandle==0 ){
    واپس SQLITE_NOMEM_BKPT;
  }
  اگر( db->nExtension>0 ){
    حافظہ_نقل(aHandle, db->aExtension, حجم(handle)*db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  واپس SQLITE_OK;

extension_not_ملا:
  اگر( pz_Err_پیغام ){
    n_پیغام += 300;
    *pz_Err_پیغام = zErrmsg = sqlite3_malloc64(n_پیغام);
    اگر( zErrmsg ){
      أكد( n_پیغام<0x7fffffff );  /* zErrmsg would be NULL if not so */
      sqlite3_snprintf((صحیح)n_پیغام, zErrmsg,
          "unable to open shared library [%.*s]", SQLITE_MAX_PATHLEN, zFile);
      sqlite3OsDlError(pVfs, n_پیغام-1, zErrmsg);
    }
  }
  واپس SQLITE_ERROR;
}
صحیح sqlite3_بھرو_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ثابت حرف *zFile,    /* Name of the shared library containing extension */
  ثابت حرف *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  حرف **pz_Err_پیغام       /* Put error message here if not 0 */
){
  صحیح rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_بھرو_Extension(db, zFile, zProc, pz_Err_پیغام);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** Call this routine when the database connection is closing in order
** to clean up loaded extensions
*/
خالی sqlite3CloseExtensions(sqlite3 *db){
  صحیح i;
  أكد( sqlite3_mutex_held(db->mutex) );
  برائے(i=0; i<db->nExtension; i++){
    sqlite3OsDlClose(db->pVfs, db->aExtension[i]);
  }
  sqlite3DbFree(db, db->aExtension);
}

/*
** Enable or disable extension loading.  Extension loading is disabled by
** default so as not to open security holes in older applications.
*/
صحیح sqlite3_enable_بھرو_extension(sqlite3 *db, صحیح onoff){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  اگر( onoff ){
    db->flags |= SQLITE_بھرو_Extension|SQLITE_بھرو_Ext_Func;
  }ورنہ{
    db->flags &= ~(u64)(SQLITE_بھرو_Extension|SQLITE_بھرو_Ext_Func);
  }
  sqlite3_mutex_leave(db->mutex);
  واپس SQLITE_OK;
}

#اگر_ختم /* !defined(SQLITE_OMIT_LOAD_EXTENSION) */

/*
** The following object holds the list of automatically loaded
** extensions.
**
** This list is shared across threads.  The SQLITE_MUTEX_STATIC_MAIN
** mutex must be held while accessing this list.
*/
قسمتعریف ساخت sqlite3AutoExtList sqlite3AutoExtList;
ساکن SQLITE_WSD ساخت sqlite3AutoExtList {
  u32 nExt;              /* Number of entries in aExt[] */
  خالی (**aExt)(خالی);   /* Pointers to the extension init functions */
} sqlite3Autoext = { 0, 0 };

/* The "wsdAutoext" macro will resolve to the autoextension
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Autoext" state vector declared above.
*/
#اگر_تعریف_شدہ SQLITE_OMIT_WSD
# define wsdAutoextInit \
  sqlite3AutoExtList *x = &GLOBAL(sqlite3AutoExtList,sqlite3Autoext)
# define wsdAutoext x[0]
#ورنہ
# define wsdAutoextInit
# define wsdAutoext sqlite3Autoext
#اگر_ختم


/*
** Register a statically linked extension that is automatically
** loaded by every new database connection.
*/
صحیح sqlite3_auto_extension(
  خالی (*xInit)(خالی)
){
  صحیح rc = SQLITE_OK;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( xInit==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  اگر( rc ){
    واپس rc;
  }ورنہ
#اگر_ختم
  {
    u32 i;
#اگر SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#اگر_ختم
    wsdAutoextInit;
    sqlite3_mutex_enter(mutex);
    برائے(i=0; i<wsdAutoext.nExt; i++){
      اگر( wsdAutoext.aExt[i]==xInit ) توڑو;
    }
    اگر( i==wsdAutoext.nExt ){
      u64 nByte = (wsdAutoext.nExt+1)*حجم(wsdAutoext.aExt[0]);
      خالی (**aNew)(خالی);
      aNew = sqlite3_realloc64(wsdAutoext.aExt, nByte);
      اگر( aNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }ورنہ{
        wsdAutoext.aExt = aNew;
        wsdAutoext.aExt[wsdAutoext.nExt] = xInit;
        wsdAutoext.nExt++;
      }
    }
    sqlite3_mutex_leave(mutex);
    أكد( (rc&0xff)==rc );
    واپس rc;
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
صحیح sqlite3_cancel_auto_extension(
  خالی (*xInit)(خالی)
){
  صحیح n = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ){
    واپس 0;
  }ورنہ
#اگر_ختم
  {
#اگر SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#اگر_ختم
    صحیح i;
    wsdAutoextInit;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
    اگر( xInit==0 ) واپس 0;
#اگر_ختم
    sqlite3_mutex_enter(mutex);
    برائے(i=(صحیح)wsdAutoext.nExt-1; i>=0; i--){
      اگر( wsdAutoext.aExt[i]==xInit ){
        wsdAutoext.nExt--;
        wsdAutoext.aExt[i] = wsdAutoext.aExt[wsdAutoext.nExt];
        n++;
        توڑو;
      }
    }
    sqlite3_mutex_leave(mutex);
  }
  واپس n;
}

/*
** Reset the automatic extension loading mechanism.
*/
خالی sqlite3_reset_auto_extension(خالی){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize()==SQLITE_OK )
#اگر_ختم
  {
#اگر SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#اگر_ختم
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
خالی sqlite3_Auto_بھرو_Extensions(sqlite3 *db){
  u32 i;
  صحیح go = 1;
  صحیح rc;
  sqlite3_loadext_اندراج xInit;

  wsdAutoextInit;
  اگر( wsdAutoext.nExt==0 ){
    /* Common case: early out without every having to acquire a mutex */
    واپس;
  }
  برائے(i=0; go; i++){
    حرف *zErrmsg;
#اگر SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
    ثابت sqlite3_api_routines *pThunk = 0;
#ورنہ
    ثابت sqlite3_api_routines *pThunk = &sqlite3Apis;
#اگر_ختم
    sqlite3_mutex_enter(mutex);
    اگر( i>=wsdAutoext.nExt ){
      xInit = 0;
      go = 0;
    }ورنہ{
      xInit = (sqlite3_loadext_اندراج)wsdAutoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    اگر( xInit && (rc = xInit(db, &zErrmsg, pThunk))!=0 ){
      sqlite3_Error_With_پیغام(db, rc,
            "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}
