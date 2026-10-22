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

#اگر_تعریف_نشده SQLITE_CORE
  #تعریف_کن SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#پایان_اگر
#ضمن "sqlite3ext.h"
#ضمن "sqliteInt.h"

#اگر_تعریف_نشده SQLITE_OMIT_بارگیری_EXTENSION
/*
** Some API routines are omitted when various features are
** excluded from a build of SQLite.  Substitute a NULL pointer
** for any missing APIs.
*/
#اگر_تعریف_نشده SQLITE_ENABLE_ستونی_METADATA
# define sqlite3_ستونی_database_name   0
# define sqlite3_ستونی_database_name16 0
# define sqlite3_ستونی_جدول_name      0
# define sqlite3_ستونی_جدول_name16    0
# define sqlite3_ستونی_origin_name     0
# define sqlite3_ستونی_origin_name16   0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_AUTHORIZATION
# define sqlite3_مجموعه_authorizer         0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_UTF16
# define sqlite3_bind_text16            0
# define sqlite3_collation_needed16     0
# define sqlite3_ستونی_decltype16      0
# define sqlite3_ستونی_name16          0
# define sqlite3_ستونی_text16          0
# define sqlite3_complete16             0
# define sqlite3_create_collation16     0
# define sqlite3_create_function16      0
# define sqlite3_errmsg16               0
# define sqlite3_open16                 0
# define sqlite3_prepare16              0
# define sqlite3_prepare16_v2           0
# define sqlite3_prepare16_v3           0
# define sqlite3_نتیجه_error16         0
# define sqlite3_نتیجه_text16          0
# define sqlite3_نتیجه_text16be        0
# define sqlite3_نتیجه_text16le        0
# define sqlite3_مقدار_text16           0
# define sqlite3_مقدار_text16be         0
# define sqlite3_مقدار_text16le         0
# define sqlite3_ستونی_database_name16 0
# define sqlite3_ستونی_جدول_name16    0
# define sqlite3_ستونی_origin_name16   0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_COMPLETE
# define sqlite3_complete 0
# define sqlite3_complete16 0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_DECLTYPE
# define sqlite3_ستونی_decltype16      0
# define sqlite3_ستونی_decltype        0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_پیشرفت_CALLBACK
# define sqlite3_پیشرفت_handler 0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_VIRTUALTABLE
# define sqlite3_create_module 0
# define sqlite3_create_module_v2 0
# define sqlite3_declare_vtab 0
# define sqlite3_vtab_config 0
# define sqlite3_vtab_on_conflict 0
# define sqlite3_vtab_collation 0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_SHARED_CACHE
# define sqlite3_enable_shared_cache 0
#پایان_اگر

#إذا defined(SQLITE_OMIT_TRACE) || defined(SQLITE_OMIT_DEPRECATED)
# define sqlite3_profile       0
# define sqlite3_trace         0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_GET_جدول
# define sqlite3_free_جدول    0
# define sqlite3_get_جدول     0
#پایان_اگر

#اگر_تعریف_شده SQLITE_OMIT_INCRBLOB
#تعریف_کن sqlite3_bind_zeroblob  0
#تعریف_کن sqlite3_blob_بایتها     0
#تعریف_کن sqlite3_blob_close     0
#تعریف_کن sqlite3_blob_open      0
#تعریف_کن sqlite3_blob_read      0
#تعریف_کن sqlite3_blob_write     0
#تعریف_کن sqlite3_blob_reopen    0
#پایان_اگر

#إذا defined(SQLITE_OMIT_TRACE)
# define sqlite3_trace_v2      0
#پایان_اگر

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
#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
  sqlite3_aggregate_شمار,
#وإلا
  0,
#پایان_اگر
  sqlite3_bind_blob,
  sqlite3_bind_double,
  sqlite3_bind_int,
  sqlite3_bind_int64,
  sqlite3_bind_null,
  sqlite3_bind_parameter_شمار,
  sqlite3_bind_parameter_شاخص,
  sqlite3_bind_parameter_name,
  sqlite3_bind_متن,
  sqlite3_bind_text16,
  sqlite3_bind_مقدار,
  sqlite3_busy_handler,
  sqlite3_busy_timeout,
  sqlite3_changes,
  sqlite3_close,
  sqlite3_collation_needed,
  sqlite3_collation_needed16,
  sqlite3_ستونی_blob,
  sqlite3_ستونی_بایتها,
  sqlite3_ستونی_bytes16,
  sqlite3_ستونی_شمار,
  sqlite3_ستونی_database_name,
  sqlite3_ستونی_database_name16,
  sqlite3_ستونی_decltype,
  sqlite3_ستونی_decltype16,
  sqlite3_ستونی_double,
  sqlite3_ستونی_int,
  sqlite3_ستونی_int64,
  sqlite3_ستونی_name,
  sqlite3_ستونی_name16,
  sqlite3_ستونی_origin_name,
  sqlite3_ستونی_origin_name16,
  sqlite3_ستونی_جدول_name,
  sqlite3_ستونی_جدول_name16,
  sqlite3_ستونی_متن,
  sqlite3_ستونی_text16,
  sqlite3_ستونی_type,
  sqlite3_ستونی_مقدار,
  sqlite3_commit_hook,
  sqlite3_complete,
  sqlite3_complete16,
  sqlite3_create_collation,
  sqlite3_create_collation16,
  sqlite3_create_function,
  sqlite3_create_function16,
  sqlite3_create_module,
  sqlite3_داده_شمار,
  sqlite3_db_handle,
  sqlite3_declare_vtab,
  sqlite3_enable_shared_cache,
  sqlite3_errcode,
  sqlite3_errmsg,
  sqlite3_errmsg16,
  sqlite3_exec,
#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
  sqlite3_expired,
#وإلا
  0,
#پایان_اگر
  sqlite3_finalize,
  sqlite3_free,
  sqlite3_free_جدول,
  sqlite3_get_autocommit,
  sqlite3_get_auxdata,
  sqlite3_get_جدول,
  0,     /* Was sqlite3_global_recover(), but that function is deprecated */
  sqlite3_وقفه,
  sqlite3_آخرین_درج_rowid,
  sqlite3_libversion,
  sqlite3_libversion_عدد,
  sqlite3_malloc,
  sqlite3_mprintf,
  sqlite3_open,
  sqlite3_open16,
  sqlite3_prepare,
  sqlite3_prepare16,
  sqlite3_profile,
  sqlite3_پیشرفت_handler,
  sqlite3_realloc,
  sqlite3_reset,
  sqlite3_نتیجه_blob,
  sqlite3_نتیجه_double,
  sqlite3_نتیجه_error,
  sqlite3_نتیجه_error16,
  sqlite3_نتیجه_int,
  sqlite3_نتیجه_int64,
  sqlite3_نتیجه_null,
  sqlite3_نتیجه_متن,
  sqlite3_نتیجه_text16,
  sqlite3_نتیجه_text16be,
  sqlite3_نتیجه_text16le,
  sqlite3_نتیجه_مقدار,
  sqlite3_rollback_hook,
  sqlite3_مجموعه_authorizer,
  sqlite3_مجموعه_auxdata,
  sqlite3_snprintf,
  sqlite3_گام,
  sqlite3_جدول_ستونی_metadata,
#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
  sqlite3_thread_cleanup,
#وإلا
  0,
#پایان_اگر
  sqlite3_مجموع_changes,
  sqlite3_trace,
#اگر_تعریف_نشده SQLITE_OMIT_DEPRECATED
  sqlite3_transfer_bindings,
#وإلا
  0,
#پایان_اگر
  sqlite3_update_hook,
  sqlite3_user_داده,
  sqlite3_مقدار_blob,
  sqlite3_مقدار_بایتها,
  sqlite3_مقدار_bytes16,
  sqlite3_مقدار_double,
  sqlite3_مقدار_int,
  sqlite3_مقدار_int64,
  sqlite3_مقدار_numeric_type,
  sqlite3_مقدار_متن,
  sqlite3_مقدار_text16,
  sqlite3_مقدار_text16be,
  sqlite3_مقدار_text16le,
  sqlite3_مقدار_type,
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
  sqlite3_blob_بایتها,
  sqlite3_blob_close,
  sqlite3_blob_open,
  sqlite3_blob_read,
  sqlite3_blob_write,
  sqlite3_create_collation_v2,
  sqlite3_file_control,
  sqlite3_حافظه_highwater,
  sqlite3_حافظه_استفاده_شده,
#اگر_تعریف_شده SQLITE_MUTEX_OMIT
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
#پایان_اگر
  sqlite3_open_v2,
  sqlite3_release_حافظه,
  sqlite3_نتیجه_error_nomem,
  sqlite3_نتیجه_error_toobig,
  sqlite3_sleep,
  sqlite3_soft_انباشته_حد,
  sqlite3_vfs_پیدا_کن,
  sqlite3_vfs_ثبات,
  sqlite3_vfs_unregister,

  /*
  ** Added for 3.5.8
  */
  sqlite3_threadsafe,
  sqlite3_نتیجه_zeroblob,
  sqlite3_نتیجه_error_کد,
  sqlite3_test_control,
  sqlite3_randomness,
  sqlite3_context_db_handle,

  /*
  ** Added for 3.6.0
  */
  sqlite3_extended_نتیجه_codes,
  sqlite3_حد,
  sqlite3_بعدی_stmt,
  sqlite3_sql,
  sqlite3_وضعیت,

  /*
  ** Added for 3.7.4
  */
  sqlite3_backup_پایان,
  sqlite3_backup_init,
  sqlite3_backup_pagecount,
  sqlite3_backup_مانده,
  sqlite3_backup_گام,
#اگر_تعریف_نشده SQLITE_OMIT_COMPILEOPTION_DIAGS
  sqlite3_compileoption_get,
  sqlite3_compileoption_استفاده_شده,
#وإلا
  0,
  0,
#پایان_اگر
  sqlite3_create_function_v2,
  sqlite3_db_config,
  sqlite3_db_mutex,
  sqlite3_db_وضعیت,
  sqlite3_extended_errcode,
  sqlite3_log,
  sqlite3_soft_انباشته_limit64,
  sqlite3_sourceid,
  sqlite3_stmt_وضعیت,
  sqlite3_strnicmp,
#اگر_تعریف_شده SQLITE_ENABLE_UNLOCK_NOTIFY
  sqlite3_unlock_notify,
#وإلا
  0,
#پایان_اگر
#اگر_تعریف_نشده SQLITE_OMIT_WAL
  sqlite3_wal_autocheckpoint,
  sqlite3_wal_checkpoint,
  sqlite3_wal_hook,
#وإلا
  0,
  0,
  0,
#پایان_اگر
  sqlite3_blob_reopen,
  sqlite3_vtab_config,
  sqlite3_vtab_on_conflict,
  sqlite3_close_v2,
  sqlite3_db_filename,
  sqlite3_db_readonly,
  sqlite3_db_release_حافظه,
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
  sqlite3_بارگیری_extension,
  sqlite3_malloc64,
  sqlite3_msize,
  sqlite3_realloc64,
  sqlite3_reset_auto_extension,
  sqlite3_نتیجه_blob64,
  sqlite3_نتیجه_text64,
  sqlite3_strglob,
  /* Version 3.8.11 and later */
  (sqlite3_مقدار*(*)(ثابت sqlite3_مقدار*))sqlite3_مقدار_dup,
  sqlite3_مقدار_free,
  sqlite3_نتیجه_zeroblob64,
  sqlite3_bind_zeroblob64,
  /* Version 3.9.0 and later */
  sqlite3_مقدار_subtype,
  sqlite3_نتیجه_subtype,
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
  sqlite3_مجموعه_آخرین_درج_rowid,
  /* Version 3.20.0 and later */
  sqlite3_prepare_v3,
  sqlite3_prepare16_v3,
  sqlite3_bind_اشاره_گر,
  sqlite3_نتیجه_اشاره_گر,
  sqlite3_مقدار_اشاره_گر,
  /* Version 3.22.0 and later */
  sqlite3_vtab_nochange,
  sqlite3_مقدار_nochange,
  sqlite3_vtab_collation,
  /* Version 3.24.0 and later */
  sqlite3_keyword_شمار,
  sqlite3_keyword_name,
  sqlite3_keyword_check,
  sqlite3_str_new,
  sqlite3_str_پایان,
  sqlite3_str_appendf,
  sqlite3_str_vappendf,
  sqlite3_str_append,
  sqlite3_str_appendall,
  sqlite3_str_appendchar,
  sqlite3_str_reset,
  sqlite3_str_errcode,
  sqlite3_str_درازی,
  sqlite3_str_مقدار,
  /* Version 3.25.0 and later */
  sqlite3_create_پنجره_function,
  /* Version 3.26.0 and later */
#اگر_تعریف_شده SQLITE_ENABLE_NORMALIZE
  sqlite3_normalized_sql,
#وإلا
  0,
#پایان_اگر
  /* Version 3.28.0 and later */
  sqlite3_stmt_isexplain,
  sqlite3_مقدار_frombind,
  /* Version 3.30.0 and later */
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
  sqlite3_drop_modules,
#وإلا
  0,
#پایان_اگر
  /* Version 3.31.0 and later */
  sqlite3_hard_انباشته_limit64,
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
  sqlite3_مجموع_changes64,
  /* Version 3.37.0 and later */
  sqlite3_autovacuum_صفحات,
  /* Version 3.38.0 and later */
  sqlite3_error_جابه_جایی,
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
  sqlite3_vtab_rhs_مقدار,
  sqlite3_vtab_distinct,
  sqlite3_vtab_درون,
  sqlite3_vtab_درون_اولی,
  sqlite3_vtab_درون_بعدی,
#وإلا
  0,
  0,
  0,
  0,
  0,
#پایان_اگر
  /* Version 3.39.0 and later */
#اگر_تعریف_نشده SQLITE_OMIT_DESERIALIZE
  sqlite3_deserialize,
  sqlite3_serialize,
#وإلا
  0,
  0,
#پایان_اگر
  sqlite3_db_name,
  /* Version 3.40.0 and later */
  sqlite3_مقدار_encoding,
  /* Version 3.41.0 and later */
  sqlite3_is_interrupted,
  /* Version 3.43.0 and later */
  sqlite3_stmt_explain,
  /* Version 3.44.0 and later */
  sqlite3_get_clientdata,
  sqlite3_مجموعه_clientdata,
  /* Version 3.50.0 and later */
  sqlite3_setlk_timeout,
  /* Version 3.51.0 and later */
  sqlite3_مجموعه_errmsg,
  sqlite3_db_status64,
  /* Version 3.52.0 and later */
  sqlite3_str_truncate,
  sqlite3_str_free,
#اگر_تعریف_شده SQLITE_ENABLE_CARRAY
  sqlite3_carray_bind,
  sqlite3_carray_bind_v2,
#وإلا
  0,
  0,
#پایان_اگر
  /* Version 3.54.0 and later */
  sqlite3_incomplete,
  sqlite3_نتیجه_str
};

/* True if x is the directory separator character
*/
#إذا SQLITE_OS_WIN
# define DirSep(X)  ((X)=='/'||(X)=='\\')
#وإلا
# define DirSep(X)  ((X)=='/')
#پایان_اگر

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
ساكن صحيح sqlite3_بارگیری_Extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ثابت حرف *zFile,    /* Name of the shared library containing extension */
  ثابت حرف *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  حرف **pz_Err_پیام       /* Put error message here if not 0 */
){
  sqlite3_vfs *pVfs = db->pVfs;
  فراغ *handle;
  sqlite3_loadext_مدخل xInit;
  حرف *zErrmsg = 0;
  ثابت حرف *z_مدخل;
  حرف *z_Alt_مدخل = 0;
  فراغ **aHandle;
  u64 n_پیام = طول_رشته(zFile);
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
#پایان_اگر
  };


  إذا( pz_Err_پیام ) *pz_Err_پیام = 0;

  /* Ticket #1863.  To avoid a creating security problems for older
  ** applications that relink against newer versions of SQLite, the
  ** ability to run load_extension is turned off by default.  One
  ** must call either sqlite3_enable_load_extension(db) or
  ** sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, 0)
  ** to turn on extension loading.
  */
  إذا( (db->flags & SQLITE_بارگیری_Extension)==0 ){
    إذا( pz_Err_پیام ){
      *pz_Err_پیام = sqlite3_mprintf("not authorized");
    }
    أرجع SQLITE_ERROR;
  }

  z_مدخل = zProc ? zProc : "sqlite3_extension_init";

  /* tag-20210611-1.  Some dlopen() implementations will segfault if given
  ** an oversize filename.  Most filesystems have a pathname limit of 4K,
  ** so limit the extension filename length to about twice that.
  ** https://sqlite.org/forum/forumpost/08a0d6d9bf
  **
  ** Later (2023-03-25): Save an extra 6 bytes for the filename suffix.
  ** See https://sqlite.org/forum/forumpost/24083b579d.
  */
  إذا( n_پیام>SQLITE_MAX_PATHLEN ) اذهب extension_not_یافت;

  /* Do not allow sqlite3_load_extension() to link to a copy of the
  ** running application, by passing in an empty filename. */
  إذا( n_پیام==0 ) اذهب extension_not_یافت;
    
  handle = sqlite3OsDlOpen(pVfs, zFile);
#إذا SQLITE_OS_UNIX || SQLITE_OS_WIN
  لكل(ii=0; ii<Array_اندازه(azEndings) && handle==0; ii++){
    حرف *zAltFile = sqlite3_mprintf("%s.%s", zFile, azEndings[ii]);
    إذا( zAltFile==0 ) أرجع SQLITE_NOMEM_BKPT;
    إذا( n_پیام+طول_رشته(azEndings[ii])+1<=SQLITE_MAX_PATHLEN ){
      handle = sqlite3OsDlOpen(pVfs, zAltFile);
    }
    sqlite3_free(zAltFile);
  }
#پایان_اگر
  إذا( handle==0 ) اذهب extension_not_یافت;
  xInit = (sqlite3_loadext_مدخل)sqlite3OsDlSym(pVfs, handle, z_مدخل);

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
    صحيح iFile, i_مدخل, c;
    صحيح ncFile = sqlite3Strlen30(zFile);
    صحيح cnt = 0;
    z_Alt_مدخل = sqlite3_malloc64(ncFile+30);
    إذا( z_Alt_مدخل==0 ){
      sqlite3OsDlClose(pVfs, handle);
      أرجع SQLITE_NOMEM_BKPT;
    }
    افعل{
      کپی_حافظه(z_Alt_مدخل, "sqlite3_", 8);
      لكل(iFile=ncFile-1; iFile>=0 && !DirSep(zFile[iFile]); iFile--){}
      iFile++;
      إذا( sqlite3_strnicmp(zFile+iFile, "lib", 3)==0 ) iFile += 3;
      لكل(i_مدخل=8; (c = zFile[iFile])!=0 && c!='.'; iFile++){
        إذا( sqlite3Isalpha(c) || (cnt && sqlite3Isdigit(c)) ){
          z_Alt_مدخل[i_مدخل++] = (حرف)sqlite3_فوقانی_مقصد_پایینی[(بیعلامت)c];
        }
      }
      کپی_حافظه(z_Alt_مدخل+i_مدخل, "_init", 6);
      z_مدخل = z_Alt_مدخل;
      xInit = (sqlite3_loadext_مدخل)sqlite3OsDlSym(pVfs, handle, z_مدخل);
    }طالما( xInit==0 && (++cnt)<2 );
  }
  إذا( xInit==0 ){
    إذا( pz_Err_پیام ){
      n_پیام += طول_رشته(z_مدخل) + 300;
      *pz_Err_پیام = zErrmsg = sqlite3_malloc64(n_پیام);
      إذا( zErrmsg ){
        أكد( n_پیام<0x7fffffff );  /* zErrmsg would be NULL if not so */
        sqlite3_snprintf((صحيح)n_پیام, zErrmsg,
            "no entry point [%s] in shared library [%s]", z_مدخل, zFile);
        sqlite3OsDlError(pVfs, n_پیام-1, zErrmsg);
      }
    }
    sqlite3OsDlClose(pVfs, handle);
    sqlite3_free(z_Alt_مدخل);
    أرجع SQLITE_ERROR;
  }
  sqlite3_free(z_Alt_مدخل);
  rc = xInit(db, &zErrmsg, &sqlite3Apis);
  إذا( rc ){
    إذا( rc==SQLITE_OK_بارگیری_PERMANENTLY ) أرجع SQLITE_OK;
    إذا( pz_Err_پیام ){
      *pz_Err_پیام = sqlite3_mprintf("error during initialization: %s", zErrmsg);
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
    کپی_حافظه(aHandle, db->aExtension, حجم(handle)*db->nExtension);
  }
  sqlite3DbFree(db, db->aExtension);
  db->aExtension = aHandle;

  db->aExtension[db->nExtension++] = handle;
  أرجع SQLITE_OK;

extension_not_یافت:
  إذا( pz_Err_پیام ){
    n_پیام += 300;
    *pz_Err_پیام = zErrmsg = sqlite3_malloc64(n_پیام);
    إذا( zErrmsg ){
      أكد( n_پیام<0x7fffffff );  /* zErrmsg would be NULL if not so */
      sqlite3_snprintf((صحيح)n_پیام, zErrmsg,
          "unable to open shared library [%.*s]", SQLITE_MAX_PATHLEN, zFile);
      sqlite3OsDlError(pVfs, n_پیام-1, zErrmsg);
    }
  }
  أرجع SQLITE_ERROR;
}
صحيح sqlite3_بارگیری_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  ثابت حرف *zFile,    /* Name of the shared library containing extension */
  ثابت حرف *zProc,    /* Entry point.  Use "sqlite3_extension_init" if 0 */
  حرف **pz_Err_پیام       /* Put error message here if not 0 */
){
  صحيح rc;
  sqlite3_mutex_enter(db->mutex);
  rc = sqlite3_بارگیری_Extension(db, zFile, zProc, pz_Err_پیام);
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
صحيح sqlite3_enable_بارگیری_extension(sqlite3 *db, صحيح onoff){
#اگر_تعریف_شده SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ) أرجع SQLITE_MISUSE_BKPT;
#پایان_اگر
  sqlite3_mutex_enter(db->mutex);
  إذا( onoff ){
    db->flags |= SQLITE_بارگیری_Extension|SQLITE_بارگیری_Ext_Func;
  }وإلا{
    db->flags &= ~(u64)(SQLITE_بارگیری_Extension|SQLITE_بارگیری_Ext_Func);
  }
  sqlite3_mutex_leave(db->mutex);
  أرجع SQLITE_OK;
}

#پایان_اگر /* !defined(SQLITE_OMIT_LOAD_EXTENSION) */

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
#اگر_تعریف_شده SQLITE_OMIT_WSD
# define wsdAutoextInit \
  sqlite3AutoExtList *x = &GLOBAL(sqlite3AutoExtList,sqlite3Autoext)
# define wsdAutoext x[0]
#وإلا
# define wsdAutoextInit
# define wsdAutoext sqlite3Autoext
#پایان_اگر


/*
** Register a statically linked extension that is automatically
** loaded by every new database connection.
*/
صحيح sqlite3_auto_extension(
  فراغ (*xInit)(فراغ)
){
  صحيح rc = SQLITE_OK;
#اگر_تعریف_شده SQLITE_ENABLE_API_ARMOR
  إذا( xInit==0 ) أرجع SQLITE_MISUSE_BKPT;
#پایان_اگر
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  إذا( rc ){
    أرجع rc;
  }وإلا
#پایان_اگر
  {
    u32 i;
#إذا SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#پایان_اگر
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
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ){
    أرجع 0;
  }وإلا
#پایان_اگر
  {
#إذا SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#پایان_اگر
    صحيح i;
    wsdAutoextInit;
#اگر_تعریف_شده SQLITE_ENABLE_API_ARMOR
    إذا( xInit==0 ) أرجع 0;
#پایان_اگر
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
#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize()==SQLITE_OK )
#پایان_اگر
  {
#إذا SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#پایان_اگر
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
فراغ sqlite3_Auto_بارگیری_Extensions(sqlite3 *db){
  u32 i;
  صحيح go = 1;
  صحيح rc;
  sqlite3_loadext_مدخل xInit;

  wsdAutoextInit;
  إذا( wsdAutoext.nExt==0 ){
    /* Common case: early out without every having to acquire a mutex */
    أرجع;
  }
  لكل(i=0; go; i++){
    حرف *zErrmsg;
#إذا SQLITE_THREADSAFE
    sqlite3_mutex *mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
#پایان_اگر
#اگر_تعریف_شده SQLITE_OMIT_بارگیری_EXTENSION
    ثابت sqlite3_api_routines *pThunk = 0;
#وإلا
    ثابت sqlite3_api_routines *pThunk = &sqlite3Apis;
#پایان_اگر
    sqlite3_mutex_enter(mutex);
    إذا( i>=wsdAutoext.nExt ){
      xInit = 0;
      go = 0;
    }وإلا{
      xInit = (sqlite3_loadext_مدخل)wsdAutoext.aExt[i];
    }
    sqlite3_mutex_leave(mutex);
    zErrmsg = 0;
    إذا( xInit && (rc = xInit(db, &zErrmsg, pThunk))!=0 ){
      sqlite3_Error_With_پیام(db, rc,
            "automatic extension loading failed: %s", zErrmsg);
      go = 0;
    }
    sqlite3_free(zErrmsg);
  }
}
