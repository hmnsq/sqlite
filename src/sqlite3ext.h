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
#ifndef SQLITE3EXT_H
#define SQLITE3EXT_H
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"

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
ସଂରଚନା sqlite3_api_routines {
  ଶୂନ୍ୟ * (*aggregate_context)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBytes);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*aggregate_ଗଣନା)(sqlite3_context*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_blob)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_double)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଦ୍ୱିଗୁଣ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_int)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_int64)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite_int64);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_null)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_parameter_ଗଣନା)(sqlite3_stmt*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_parameter_index)(sqlite3_stmt*,ଅଚଳ ଅକ୍ଷର*zName);
  ଅଚଳ ଅକ୍ଷର * (*bind_parameter_name)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_text)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_text16)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*bind_ମୂଲ୍ୟ)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ sqlite3_ମୂଲ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*busy_handler)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା),ଶୂନ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*busy_timeout)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା ms);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*changes)(sqlite3*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*close)(sqlite3*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*collation_needed)(sqlite3*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*,sqlite3*,
                           ପୂର୍ଣ୍ଣସଂଖ୍ୟା eTextRep,ଅଚଳ ଅକ୍ଷର*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*collation_needed16)(sqlite3*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*,sqlite3*,
                             ପୂର୍ଣ୍ଣସଂଖ୍ୟା eTextRep,ଅଚଳ ଶୂନ୍ୟ*));
  ଅଚଳ ଶୂନ୍ୟ * (*column_blob)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*column_bytes)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*column_bytes16)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*column_ଗଣନା)(sqlite3_stmt*pStmt);
  ଅଚଳ ଅକ୍ଷର * (*column_database_name)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଶୂନ୍ୟ * (*column_database_name16)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଅକ୍ଷର * (*column_decltype)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା i);
  ଅଚଳ ଶୂନ୍ୟ * (*column_decltype16)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଦ୍ୱିଗୁଣ  (*column_double)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*column_int)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  ଅଚଳ ଅକ୍ଷର * (*column_name)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଶୂନ୍ୟ * (*column_name16)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଅକ୍ଷର * (*column_origin_name)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଶୂନ୍ୟ * (*column_origin_name16)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଅକ୍ଷର * (*column_table_name)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଶୂନ୍ୟ * (*column_table_name16)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ unsigned ଅକ୍ଷର * (*column_text)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  ଅଚଳ ଶୂନ୍ୟ * (*column_text16)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*column_type)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  sqlite3_ମୂଲ୍ୟ* (*column_ମୂଲ୍ୟ)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol);
  ଶୂନ୍ୟ * (*commit_hook)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*),ଶୂନ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*complete)(ଅଚଳ ଅକ୍ଷର*sql);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*complete16)(ଅଚଳ ଶୂନ୍ୟ*sql);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*create_collation)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,
                           ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*create_collation16)(sqlite3*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,
                             ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*create_function)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,
                          ଶୂନ୍ୟ (*xFunc)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
                          ଶୂନ୍ୟ (*xStep)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
                          ଶୂନ୍ୟ (*xFinal)(sqlite3_context*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*create_function16)(sqlite3*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,
                            ଶୂନ୍ୟ (*xFunc)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
                            ଶୂନ୍ୟ (*xStep)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
                            ଶୂନ୍ୟ (*xFinal)(sqlite3_context*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*create_module)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ sqlite3_module*,ଶୂନ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*data_ଗଣନା)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*declare_vtab)(sqlite3*,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*enable_shared_cache)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*errcode)(sqlite3*db);
  ଅଚଳ ଅକ୍ଷର * (*errmsg)(sqlite3*);
  ଅଚଳ ଶୂନ୍ୟ * (*errmsg16)(sqlite3*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*exec)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,sqlite3_callback,ଶୂନ୍ୟ*,ଅକ୍ଷର**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*expired)(sqlite3_stmt*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*finalize)(sqlite3_stmt*pStmt);
  ଶୂନ୍ୟ  (*ମୁକ୍ତ)(ଶୂନ୍ୟ*);
  ଶୂନ୍ୟ  (*free_table)(ଅକ୍ଷର**result);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*get_autocommit)(sqlite3*);
  ଶୂନ୍ୟ * (*get_auxdata)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*get_table)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ଅକ୍ଷର***,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ଅକ୍ଷର**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*global_recover)(ଶୂନ୍ୟ);
  ଶୂନ୍ୟ  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_ଭର୍ତ୍ତି_rowid)(sqlite3*);
  ଅଚଳ ଅକ୍ଷର * (*libversion)(ଶୂନ୍ୟ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*libversion_ସଂଖ୍ୟା)(ଶୂନ୍ୟ);
  ଶୂନ୍ୟ *(*malloc)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅକ୍ଷର *(SQLITE_CDECL*mprintf)(ଅଚଳ ଅକ୍ଷର*,...);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*open)(ଅଚଳ ଅକ୍ଷର*,sqlite3**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*open16)(ଅଚଳ ଶୂନ୍ୟ*,sqlite3**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*prepare)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_stmt**,ଅଚଳ ଅକ୍ଷର**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*prepare16)(sqlite3*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_stmt**,ଅଚଳ ଶୂନ୍ୟ**);
  ଶୂନ୍ୟ * (*profile)(sqlite3*,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*,ଅଚଳ ଅକ୍ଷର*,sqlite_uint64),ଶୂନ୍ୟ*);
  ଶୂନ୍ୟ  (*progress_handler)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*),ଶୂନ୍ୟ*);
  ଶୂନ୍ୟ *(*realloc)(ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*reset)(sqlite3_stmt*pStmt);
  ଶୂନ୍ୟ  (*result_blob)(sqlite3_context*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ଶୂନ୍ୟ  (*result_double)(sqlite3_context*,ଦ୍ୱିଗୁଣ);
  ଶୂନ୍ୟ  (*result_error)(sqlite3_context*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଶୂନ୍ୟ  (*result_error16)(sqlite3_context*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଶୂନ୍ୟ  (*result_int)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଶୂନ୍ୟ  (*result_int64)(sqlite3_context*,sqlite_int64);
  ଶୂନ୍ୟ  (*result_null)(sqlite3_context*);
  ଶୂନ୍ୟ  (*result_text)(sqlite3_context*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ଶୂନ୍ୟ  (*result_text16)(sqlite3_context*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ଶୂନ୍ୟ  (*result_text16be)(sqlite3_context*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ଶୂନ୍ୟ  (*result_text16le)(sqlite3_context*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ଶୂନ୍ୟ  (*result_ମୂଲ୍ୟ)(sqlite3_context*,sqlite3_ମୂଲ୍ୟ*);
  ଶୂନ୍ୟ *(*rollback_hook)(sqlite3*,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*),ଶୂନ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*set_authorizer)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,
                         ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*),ଶୂନ୍ୟ*);
  ଶୂନ୍ୟ  (*set_auxdata)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ*));
  ଅକ୍ଷର *(SQLITE_CDECL*xsnprintf)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,...);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*step)(sqlite3_stmt*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*table_column_metadata)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,
                                ଅକ୍ଷର ଅଚଳ**,ଅକ୍ଷର ଅଚଳ**,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*);
  ଶୂନ୍ୟ  (*thread_cleanup)(ଶୂନ୍ୟ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*total_changes)(sqlite3*);
  ଶୂନ୍ୟ * (*trace)(sqlite3*,ଶୂନ୍ୟ(*xTrace)(ଶୂନ୍ୟ*,ଅଚଳ ଅକ୍ଷର*),ଶୂନ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  ଶୂନ୍ୟ * (*update_hook)(sqlite3*,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା ,ଅକ୍ଷର ଅଚଳ*,ଅକ୍ଷର ଅଚଳ*,
                                         sqlite_int64),ଶୂନ୍ୟ*);
  ଶୂନ୍ୟ * (*user_data)(sqlite3_context*);
  ଅଚଳ ଶୂନ୍ୟ * (*ମୂଲ୍ୟ_blob)(sqlite3_ମୂଲ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*ମୂଲ୍ୟ_bytes)(sqlite3_ମୂଲ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*ମୂଲ୍ୟ_bytes16)(sqlite3_ମୂଲ୍ୟ*);
  ଦ୍ୱିଗୁଣ  (*ମୂଲ୍ୟ_double)(sqlite3_ମୂଲ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*ମୂଲ୍ୟ_int)(sqlite3_ମୂଲ୍ୟ*);
  sqlite_int64  (*ମୂଲ୍ୟ_int64)(sqlite3_ମୂଲ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*ମୂଲ୍ୟ_numeric_type)(sqlite3_ମୂଲ୍ୟ*);
  ଅଚଳ unsigned ଅକ୍ଷର * (*ମୂଲ୍ୟ_text)(sqlite3_ମୂଲ୍ୟ*);
  ଅଚଳ ଶୂନ୍ୟ * (*ମୂଲ୍ୟ_text16)(sqlite3_ମୂଲ୍ୟ*);
  ଅଚଳ ଶୂନ୍ୟ * (*ମୂଲ୍ୟ_text16be)(sqlite3_ମୂଲ୍ୟ*);
  ଅଚଳ ଶୂନ୍ୟ * (*ମୂଲ୍ୟ_text16le)(sqlite3_ମୂଲ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  (*ମୂଲ୍ୟ_type)(sqlite3_ମୂଲ୍ୟ*);
  ଅକ୍ଷର *(*vmprintf)(ଅଚଳ ଅକ୍ଷର*,va_list);
  /* Added ??? */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*overload_function)(sqlite3*, ଅଚଳ ଅକ୍ଷର *zFuncName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg);
  /* Added by 3.3.13 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*prepare_v2)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_stmt**,ଅଚଳ ଅକ୍ଷର**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*prepare16_v2)(sqlite3*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_stmt**,ଅଚଳ ଶୂନ୍ୟ**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*create_module_v2)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ sqlite3_module*,ଶୂନ୍ୟ*,
                          ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ *));
  /* Added by 3.5.0 */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*bind_zeroblob)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*blob_bytes)(sqlite3_blob*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*blob_close)(sqlite3_blob*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*blob_open)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,sqlite3_int64,
                   ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_blob**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*blob_read)(sqlite3_blob*,ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*blob_write)(sqlite3_blob*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*create_collation_v2)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,
                             ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*),
                             ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*file_control)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*);
  sqlite3_int64 (*memory_highwater)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  sqlite3_int64 (*memory_used)(ଶୂନ୍ୟ);
  sqlite3_mutex *(*mutex_alloc)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଶୂନ୍ୟ (*mutex_enter)(sqlite3_mutex*);
  ଶୂନ୍ୟ (*mutex_free)(sqlite3_mutex*);
  ଶୂନ୍ୟ (*mutex_leave)(sqlite3_mutex*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*mutex_try)(sqlite3_mutex*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*open_v2)(ଅଚଳ ଅକ୍ଷର*,sqlite3**,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*release_memory)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଶୂନ୍ୟ (*result_error_nomem)(sqlite3_context*);
  ଶୂନ୍ୟ (*result_error_toobig)(sqlite3_context*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*sleep)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଶୂନ୍ୟ (*soft_heap_limit)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  sqlite3_vfs *(*vfs_find)(ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vfs_register)(sqlite3_vfs*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vfs_unregister)(sqlite3_vfs*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xthreadsafe)(ଶୂନ୍ୟ);
  ଶୂନ୍ୟ (*result_zeroblob)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଶୂନ୍ୟ (*result_error_code)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (SQLITE_CDECL*test_control)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ...);
  ଶୂନ୍ୟ (*randomness)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*extended_result_codes)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*limit)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
  ଅଚଳ ଅକ୍ଷର *(*sql)(sqlite3_stmt*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*status)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,sqlite3*,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*backup_pagecount)(sqlite3_backup*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*backup_remaining)(sqlite3_backup*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*backup_step)(sqlite3_backup*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଅକ୍ଷର *(*compileoption_get)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*compileoption_used)(ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*create_function_v2)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,
                            ଶୂନ୍ୟ (*xFunc)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
                            ଶୂନ୍ୟ (*xStep)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
                            ଶୂନ୍ୟ (*xFinal)(sqlite3_context*),
                            ଶୂନ୍ୟ(*xDestroy)(ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (SQLITE_CDECL*db_config)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*db_status)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*extended_errcode)(sqlite3*);
  ଶୂନ୍ୟ (SQLITE_CDECL*log)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  ଅଚଳ ଅକ୍ଷର *(*sourceid)(ଶୂନ୍ୟ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*stmt_status)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*strnicmp)(ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*unlock_notify)(sqlite3*,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ**,ପୂର୍ଣ୍ଣସଂଖ୍ୟା),ଶୂନ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*wal_autocheckpoint)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*wal_checkpoint)(sqlite3*,ଅଚଳ ଅକ୍ଷର*);
  ଶୂନ୍ୟ *(*wal_hook)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*,sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା),ଶୂନ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (SQLITE_CDECL*vtab_config)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,...);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*close_v2)(sqlite3*);
  ଅଚଳ ଅକ୍ଷର *(*db_filename)(sqlite3*,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*db_readonly)(sqlite3*,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*db_release_memory)(sqlite3*);
  ଅଚଳ ଅକ୍ଷର *(*errstr)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*stmt_busy)(sqlite3_stmt*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*stmt_readonly)(sqlite3_stmt*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*stricmp)(ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*uri_boolean)(ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  sqlite3_int64 (*uri_int64)(ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,sqlite3_int64);
  ଅଚଳ ଅକ୍ଷର *(*uri_parameter)(ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*);
  ଅକ୍ଷର *(*xvsnprintf)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,va_list);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*wal_checkpoint_v2)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*);
  /* Version 3.8.7 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*auto_extension)(ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*bind_blob64)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,sqlite3_uint64,
                     ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*bind_text64)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର*,sqlite3_uint64,
                      ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*),unsigned ଅକ୍ଷର);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*cancel_auto_extension)(ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*load_extension)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,ଅକ୍ଷର**);
  ଶୂନ୍ୟ *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(ଶୂନ୍ୟ*);
  ଶୂନ୍ୟ *(*realloc64)(ଶୂନ୍ୟ*,sqlite3_uint64);
  ଶୂନ୍ୟ (*reset_auto_extension)(ଶୂନ୍ୟ);
  ଶୂନ୍ୟ (*result_blob64)(sqlite3_context*,ଅଚଳ ଶୂନ୍ୟ*,sqlite3_uint64,
                        ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ଶୂନ୍ୟ (*result_text64)(sqlite3_context*,ଅଚଳ ଅକ୍ଷର*,sqlite3_uint64,
                         ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*), unsigned ଅକ୍ଷର);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*strglob)(ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*);
  /* Version 3.8.11 and later */
  sqlite3_ମୂଲ୍ୟ *(*ମୂଲ୍ୟ_dup)(ଅଚଳ sqlite3_ମୂଲ୍ୟ*);
  ଶୂନ୍ୟ (*ମୂଲ୍ୟ_free)(sqlite3_ମୂଲ୍ୟ*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*result_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*bind_zeroblob64)(sqlite3_stmt*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*ମୂଲ୍ୟ_subtype)(sqlite3_ମୂଲ୍ୟ*);
  ଶୂନ୍ୟ (*result_subtype)(sqlite3_context*,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  /* Version 3.10.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*status64)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_int64*,sqlite3_int64*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*strlike)(ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*trace_v2)(sqlite3*,unsigned,ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(unsigned,ଶୂନ୍ୟ*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ*),ଶୂନ୍ୟ*);
  ଅକ୍ଷର *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  ଶୂନ୍ୟ (*set_last_ଭର୍ତ୍ତି_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*prepare_v3)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା,
                    sqlite3_stmt**,ଅଚଳ ଅକ୍ଷର**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*prepare16_v3)(sqlite3*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା,
                      sqlite3_stmt**,ଅଚଳ ଶୂନ୍ୟ**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*bind_pointer)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,ଅଚଳ ଅକ୍ଷର*,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ଶୂନ୍ୟ (*result_pointer)(sqlite3_context*,ଶୂନ୍ୟ*,ଅଚଳ ଅକ୍ଷର*,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ଶୂନ୍ୟ *(*ମୂଲ୍ୟ_pointer)(sqlite3_ମୂଲ୍ୟ*,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vtab_nochange)(sqlite3_context*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*ମୂଲ୍ୟ_nochange)(sqlite3_ମୂଲ୍ୟ*);
  ଅଚଳ ଅକ୍ଷର *(*vtab_collation)(sqlite3_index_info*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  /* Version 3.24.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*keyword_ଗଣନା)(ଶୂନ୍ୟ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*keyword_name)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର**,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*keyword_check)(ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  sqlite3_str *(*str_new)(sqlite3*);
  ଅକ୍ଷର *(*str_finish)(sqlite3_str*);
  ଶୂନ୍ୟ (SQLITE_CDECL*str_appendf)(sqlite3_str*, ଅଚଳ ଅକ୍ଷର *zFormat, ...);
  ଶୂନ୍ୟ (*str_vappendf)(sqlite3_str*, ଅଚଳ ଅକ୍ଷର *zFormat, va_list);
  ଶୂନ୍ୟ (*str_append)(sqlite3_str*, ଅଚଳ ଅକ୍ଷର *zIn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N);
  ଶୂନ୍ୟ (*str_appendall)(sqlite3_str*, ଅଚଳ ଅକ୍ଷର *zIn);
  ଶୂନ୍ୟ (*str_appendchar)(sqlite3_str*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N, ଅକ୍ଷର C);
  ଶୂନ୍ୟ (*str_reset)(sqlite3_str*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*str_errcode)(sqlite3_str*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*str_length)(sqlite3_str*);
  ଅକ୍ଷର *(*str_ମୂଲ୍ୟ)(sqlite3_str*);
  /* Version 3.25.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*create_window_function)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,
                            ଶୂନ୍ୟ (*xStep)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
                            ଶୂନ୍ୟ (*xFinal)(sqlite3_context*),
                            ଶୂନ୍ୟ (*x_ମୂଲ୍ୟ)(sqlite3_context*),
                            ଶୂନ୍ୟ (*xInv)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),
                            ଶୂନ୍ୟ(*xDestroy)(ଶୂନ୍ୟ*));
  /* Version 3.26.0 and later */
  ଅଚଳ ଅକ୍ଷର *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*stmt_isexplain)(sqlite3_stmt*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*ମୂଲ୍ୟ_frombind)(sqlite3_ମୂଲ୍ୟ*);
  /* Version 3.30.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*drop_modules)(sqlite3*,ଅଚଳ ଅକ୍ଷର**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  ଅଚଳ ଅକ୍ଷର *(*uri_ଚାବି)(ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଅକ୍ଷର *(*filename_database)(ଅଚଳ ଅକ୍ଷର*);
  ଅଚଳ ଅକ୍ଷର *(*filename_journal)(ଅଚଳ ଅକ୍ଷର*);
  ଅଚଳ ଅକ୍ଷର *(*filename_wal)(ଅଚଳ ଅକ୍ଷର*);
  /* Version 3.32.0 and later */
  ଅଚଳ ଅକ୍ଷର *(*create_filename)(ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,
                           ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର**);
  ଶୂନ୍ୟ (*free_filename)(ଅଚଳ ଅକ୍ଷର*);
  sqlite3_file *(*database_file_object)(ଅଚଳ ଅକ୍ଷର*);
  /* Version 3.34.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*txn_state)(sqlite3*,ଅଚଳ ଅକ୍ଷର*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*total_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*autovacuum_pages)(sqlite3*,
     unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଶୂନ୍ୟ*,ଅଚଳ ଅକ୍ଷର*,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା,unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା),
     ଶୂନ୍ୟ*, ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  /* Version 3.38.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*error_offset)(sqlite3*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vtab_rhs_ମୂଲ୍ୟ)(sqlite3_index_info*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vtab_distinct)(sqlite3_index_info*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vtab_in)(sqlite3_index_info*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vtab_in_first)(sqlite3_ମୂଲ୍ୟ*,sqlite3_ମୂଲ୍ୟ**);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*vtab_in_next)(sqlite3_ମୂଲ୍ୟ*,sqlite3_ମୂଲ୍ୟ**);
  /* Version 3.39.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*deserialize)(sqlite3*,ଅଚଳ ଅକ୍ଷର*,unsigned ଅକ୍ଷର*,
                     sqlite3_int64,sqlite3_int64,unsigned);
  unsigned ଅକ୍ଷର *(*serialize)(sqlite3*,ଅଚଳ ଅକ୍ଷର *,sqlite3_int64*,
                              unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଅଚଳ ଅକ୍ଷର *(*db_name)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  /* Version 3.40.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*ମୂଲ୍ୟ_encoding)(sqlite3_ମୂଲ୍ୟ*);
  /* Version 3.41.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*stmt_explain)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  /* Version 3.44.0 and later */
  ଶୂନ୍ୟ *(*get_clientdata)(sqlite3*,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*set_clientdata)(sqlite3*, ଅଚଳ ଅକ୍ଷର*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  /* Version 3.50.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*setlk_timeout)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  /* Version 3.51.0 and later */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*set_errmsg)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*db_status64)(sqlite3*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_int64*,sqlite3_int64*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  /* Version 3.52.0 and later */
  ଶୂନ୍ୟ (*str_truncate)(sqlite3_str*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
  ଶୂନ୍ୟ (*str_free)(sqlite3_str*);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*carray_bind)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*carray_bind_v2)(sqlite3_stmt*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ*),ଶୂନ୍ୟ*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ଅଚଳ ଅକ୍ଷର*);
  ଶୂନ୍ୟ (*result_str)(sqlite3_context*,sqlite3_str*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*sqlite3_loadext_entry)(
  sqlite3 *db,                       /* Handle to the database. */
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ,                   /* Used to set error string on failure. */
  ଅଚଳ sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#ଯଦି !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_aggregate_ଗଣନା        sqlite3_api->aggregate_ଗଣନା
#endif
#define sqlite3_bind_blob              sqlite3_api->bind_blob
#define sqlite3_bind_double            sqlite3_api->bind_double
#define sqlite3_bind_int               sqlite3_api->bind_int
#define sqlite3_bind_int64             sqlite3_api->bind_int64
#define sqlite3_bind_null              sqlite3_api->bind_null
#define sqlite3_bind_parameter_ଗଣନା   sqlite3_api->bind_parameter_ଗଣନା
#define sqlite3_bind_parameter_index   sqlite3_api->bind_parameter_index
#define sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#define sqlite3_bind_text              sqlite3_api->bind_text
#define sqlite3_bind_text16            sqlite3_api->bind_text16
#define sqlite3_bind_ମୂଲ୍ୟ             sqlite3_api->bind_ମୂଲ୍ୟ
#define sqlite3_busy_handler           sqlite3_api->busy_handler
#define sqlite3_busy_timeout           sqlite3_api->busy_timeout
#define sqlite3_changes                sqlite3_api->changes
#define sqlite3_close                  sqlite3_api->close
#define sqlite3_collation_needed       sqlite3_api->collation_needed
#define sqlite3_collation_needed16     sqlite3_api->collation_needed16
#define sqlite3_column_blob            sqlite3_api->column_blob
#define sqlite3_column_bytes           sqlite3_api->column_bytes
#define sqlite3_column_bytes16         sqlite3_api->column_bytes16
#define sqlite3_column_ଗଣନା           sqlite3_api->column_ଗଣନା
#define sqlite3_column_database_name   sqlite3_api->column_database_name
#define sqlite3_column_database_name16 sqlite3_api->column_database_name16
#define sqlite3_column_decltype        sqlite3_api->column_decltype
#define sqlite3_column_decltype16      sqlite3_api->column_decltype16
#define sqlite3_column_double          sqlite3_api->column_double
#define sqlite3_column_int             sqlite3_api->column_int
#define sqlite3_column_int64           sqlite3_api->column_int64
#define sqlite3_column_name            sqlite3_api->column_name
#define sqlite3_column_name16          sqlite3_api->column_name16
#define sqlite3_column_origin_name     sqlite3_api->column_origin_name
#define sqlite3_column_origin_name16   sqlite3_api->column_origin_name16
#define sqlite3_column_table_name      sqlite3_api->column_table_name
#define sqlite3_column_table_name16    sqlite3_api->column_table_name16
#define sqlite3_column_text            sqlite3_api->column_text
#define sqlite3_column_text16          sqlite3_api->column_text16
#define sqlite3_column_type            sqlite3_api->column_type
#define sqlite3_column_ମୂଲ୍ୟ           sqlite3_api->column_ମୂଲ୍ୟ
#define sqlite3_commit_hook            sqlite3_api->commit_hook
#define sqlite3_complete               sqlite3_api->complete
#define sqlite3_complete16             sqlite3_api->complete16
#define sqlite3_create_collation       sqlite3_api->create_collation
#define sqlite3_create_collation16     sqlite3_api->create_collation16
#define sqlite3_create_function        sqlite3_api->create_function
#define sqlite3_create_function16      sqlite3_api->create_function16
#define sqlite3_create_module          sqlite3_api->create_module
#define sqlite3_create_module_v2       sqlite3_api->create_module_v2
#define sqlite3_data_ଗଣନା             sqlite3_api->data_ଗଣନା
#define sqlite3_db_handle              sqlite3_api->db_handle
#define sqlite3_declare_vtab           sqlite3_api->declare_vtab
#define sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#define sqlite3_errcode                sqlite3_api->errcode
#define sqlite3_errmsg                 sqlite3_api->errmsg
#define sqlite3_errmsg16               sqlite3_api->errmsg16
#define sqlite3_exec                   sqlite3_api->exec
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_expired                sqlite3_api->expired
#endif
#define sqlite3_finalize               sqlite3_api->finalize
#define sqlite3_free                   sqlite3_api->ମୁକ୍ତ
#define sqlite3_free_table             sqlite3_api->free_table
#define sqlite3_get_autocommit         sqlite3_api->get_autocommit
#define sqlite3_get_auxdata            sqlite3_api->get_auxdata
#define sqlite3_get_table              sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_global_recover         sqlite3_api->global_recover
#endif
#define sqlite3_interrupt              sqlite3_api->interruptx
#define sqlite3_last_ଭର୍ତ୍ତି_rowid      sqlite3_api->last_ଭର୍ତ୍ତି_rowid
#define sqlite3_libversion             sqlite3_api->libversion
#define sqlite3_libversion_ସଂଖ୍ୟା      sqlite3_api->libversion_ସଂଖ୍ୟା
#define sqlite3_malloc                 sqlite3_api->malloc
#define sqlite3_mprintf                sqlite3_api->mprintf
#define sqlite3_open                   sqlite3_api->open
#define sqlite3_open16                 sqlite3_api->open16
#define sqlite3_prepare                sqlite3_api->prepare
#define sqlite3_prepare16              sqlite3_api->prepare16
#define sqlite3_prepare_v2             sqlite3_api->prepare_v2
#define sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#define sqlite3_profile                sqlite3_api->profile
#define sqlite3_progress_handler       sqlite3_api->progress_handler
#define sqlite3_realloc                sqlite3_api->realloc
#define sqlite3_reset                  sqlite3_api->reset
#define sqlite3_result_blob            sqlite3_api->result_blob
#define sqlite3_result_double          sqlite3_api->result_double
#define sqlite3_result_error           sqlite3_api->result_error
#define sqlite3_result_error16         sqlite3_api->result_error16
#define sqlite3_result_int             sqlite3_api->result_int
#define sqlite3_result_int64           sqlite3_api->result_int64
#define sqlite3_result_null            sqlite3_api->result_null
#define sqlite3_result_text            sqlite3_api->result_text
#define sqlite3_result_text16          sqlite3_api->result_text16
#define sqlite3_result_text16be        sqlite3_api->result_text16be
#define sqlite3_result_text16le        sqlite3_api->result_text16le
#define sqlite3_result_ମୂଲ୍ୟ           sqlite3_api->result_ମୂଲ୍ୟ
#define sqlite3_rollback_hook          sqlite3_api->rollback_hook
#define sqlite3_set_authorizer         sqlite3_api->set_authorizer
#define sqlite3_set_auxdata            sqlite3_api->set_auxdata
#define sqlite3_snprintf               sqlite3_api->xsnprintf
#define sqlite3_step                   sqlite3_api->step
#define sqlite3_table_column_metadata  sqlite3_api->table_column_metadata
#define sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#define sqlite3_total_changes          sqlite3_api->total_changes
#define sqlite3_trace                  sqlite3_api->trace
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#endif
#define sqlite3_update_hook            sqlite3_api->update_hook
#define sqlite3_user_data              sqlite3_api->user_data
#define sqlite3_ମୂଲ୍ୟ_blob             sqlite3_api->ମୂଲ୍ୟ_blob
#define sqlite3_ମୂଲ୍ୟ_bytes            sqlite3_api->ମୂଲ୍ୟ_bytes
#define sqlite3_ମୂଲ୍ୟ_bytes16          sqlite3_api->ମୂଲ୍ୟ_bytes16
#define sqlite3_ମୂଲ୍ୟ_double           sqlite3_api->ମୂଲ୍ୟ_double
#define sqlite3_ମୂଲ୍ୟ_int              sqlite3_api->ମୂଲ୍ୟ_int
#define sqlite3_ମୂଲ୍ୟ_int64            sqlite3_api->ମୂଲ୍ୟ_int64
#define sqlite3_ମୂଲ୍ୟ_numeric_type     sqlite3_api->ମୂଲ୍ୟ_numeric_type
#define sqlite3_ମୂଲ୍ୟ_text             sqlite3_api->ମୂଲ୍ୟ_text
#define sqlite3_ମୂଲ୍ୟ_text16           sqlite3_api->ମୂଲ୍ୟ_text16
#define sqlite3_ମୂଲ୍ୟ_text16be         sqlite3_api->ମୂଲ୍ୟ_text16be
#define sqlite3_ମୂଲ୍ୟ_text16le         sqlite3_api->ମୂଲ୍ୟ_text16le
#define sqlite3_ମୂଲ୍ୟ_type             sqlite3_api->ମୂଲ୍ୟ_type
#define sqlite3_vmprintf               sqlite3_api->vmprintf
#define sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#define sqlite3_overload_function      sqlite3_api->overload_function
#define sqlite3_prepare_v2             sqlite3_api->prepare_v2
#define sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#define sqlite3_clear_bindings         sqlite3_api->clear_bindings
#define sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#define sqlite3_blob_bytes             sqlite3_api->blob_bytes
#define sqlite3_blob_close             sqlite3_api->blob_close
#define sqlite3_blob_open              sqlite3_api->blob_open
#define sqlite3_blob_read              sqlite3_api->blob_read
#define sqlite3_blob_write             sqlite3_api->blob_write
#define sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#define sqlite3_file_control           sqlite3_api->file_control
#define sqlite3_memory_highwater       sqlite3_api->memory_highwater
#define sqlite3_memory_used            sqlite3_api->memory_used
#define sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#define sqlite3_mutex_enter            sqlite3_api->mutex_enter
#define sqlite3_mutex_free             sqlite3_api->mutex_free
#define sqlite3_mutex_leave            sqlite3_api->mutex_leave
#define sqlite3_mutex_try              sqlite3_api->mutex_try
#define sqlite3_open_v2                sqlite3_api->open_v2
#define sqlite3_release_memory         sqlite3_api->release_memory
#define sqlite3_result_error_nomem     sqlite3_api->result_error_nomem
#define sqlite3_result_error_toobig    sqlite3_api->result_error_toobig
#define sqlite3_sleep                  sqlite3_api->sleep
#define sqlite3_soft_heap_limit        sqlite3_api->soft_heap_limit
#define sqlite3_vfs_find               sqlite3_api->vfs_find
#define sqlite3_vfs_register           sqlite3_api->vfs_register
#define sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#define sqlite3_threadsafe             sqlite3_api->xthreadsafe
#define sqlite3_result_zeroblob        sqlite3_api->result_zeroblob
#define sqlite3_result_error_code      sqlite3_api->result_error_code
#define sqlite3_test_control           sqlite3_api->test_control
#define sqlite3_randomness             sqlite3_api->randomness
#define sqlite3_context_db_handle      sqlite3_api->context_db_handle
#define sqlite3_extended_result_codes  sqlite3_api->extended_result_codes
#define sqlite3_limit                  sqlite3_api->limit
#define sqlite3_next_stmt              sqlite3_api->next_stmt
#define sqlite3_sql                    sqlite3_api->sql
#define sqlite3_status                 sqlite3_api->status
#define sqlite3_backup_finish          sqlite3_api->backup_finish
#define sqlite3_backup_init            sqlite3_api->backup_init
#define sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#define sqlite3_backup_remaining       sqlite3_api->backup_remaining
#define sqlite3_backup_step            sqlite3_api->backup_step
#define sqlite3_compileoption_get      sqlite3_api->compileoption_get
#define sqlite3_compileoption_used     sqlite3_api->compileoption_used
#define sqlite3_create_function_v2     sqlite3_api->create_function_v2
#define sqlite3_db_config              sqlite3_api->db_config
#define sqlite3_db_mutex               sqlite3_api->db_mutex
#define sqlite3_db_status              sqlite3_api->db_status
#define sqlite3_extended_errcode       sqlite3_api->extended_errcode
#define sqlite3_log                    sqlite3_api->log
#define sqlite3_soft_heap_limit64      sqlite3_api->soft_heap_limit64
#define sqlite3_sourceid               sqlite3_api->sourceid
#define sqlite3_stmt_status            sqlite3_api->stmt_status
#define sqlite3_strnicmp               sqlite3_api->strnicmp
#define sqlite3_unlock_notify          sqlite3_api->unlock_notify
#define sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#define sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#define sqlite3_wal_hook               sqlite3_api->wal_hook
#define sqlite3_blob_reopen            sqlite3_api->blob_reopen
#define sqlite3_vtab_config            sqlite3_api->vtab_config
#define sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#define sqlite3_close_v2               sqlite3_api->close_v2
#define sqlite3_db_filename            sqlite3_api->db_filename
#define sqlite3_db_readonly            sqlite3_api->db_readonly
#define sqlite3_db_release_memory      sqlite3_api->db_release_memory
#define sqlite3_errstr                 sqlite3_api->errstr
#define sqlite3_stmt_busy              sqlite3_api->stmt_busy
#define sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#define sqlite3_stricmp                sqlite3_api->stricmp
#define sqlite3_uri_boolean            sqlite3_api->uri_boolean
#define sqlite3_uri_int64              sqlite3_api->uri_int64
#define sqlite3_uri_parameter          sqlite3_api->uri_parameter
#define sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#define sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#define sqlite3_auto_extension         sqlite3_api->auto_extension
#define sqlite3_bind_blob64            sqlite3_api->bind_blob64
#define sqlite3_bind_text64            sqlite3_api->bind_text64
#define sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#define sqlite3_load_extension         sqlite3_api->load_extension
#define sqlite3_malloc64               sqlite3_api->malloc64
#define sqlite3_msize                  sqlite3_api->msize
#define sqlite3_realloc64              sqlite3_api->realloc64
#define sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#define sqlite3_result_blob64          sqlite3_api->result_blob64
#define sqlite3_result_text64          sqlite3_api->result_text64
#define sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#define sqlite3_ମୂଲ୍ୟ_dup              sqlite3_api->ମୂଲ୍ୟ_dup
#define sqlite3_ମୂଲ୍ୟ_free             sqlite3_api->ମୂଲ୍ୟ_free
#define sqlite3_result_zeroblob64      sqlite3_api->result_zeroblob64
#define sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define sqlite3_ମୂଲ୍ୟ_subtype          sqlite3_api->ମୂଲ୍ୟ_subtype
#define sqlite3_result_subtype         sqlite3_api->result_subtype
/* Version 3.10.0 and later */
#define sqlite3_status64               sqlite3_api->status64
#define sqlite3_strlike                sqlite3_api->strlike
#define sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#define sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#define sqlite3_trace_v2               sqlite3_api->trace_v2
#define sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#define sqlite3_set_last_ଭର୍ତ୍ତି_rowid  sqlite3_api->set_last_ଭର୍ତ୍ତି_rowid
/* Version 3.20.0 and later */
#define sqlite3_prepare_v3             sqlite3_api->prepare_v3
#define sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#define sqlite3_bind_pointer           sqlite3_api->bind_pointer
#define sqlite3_result_pointer         sqlite3_api->result_pointer
#define sqlite3_ମୂଲ୍ୟ_pointer          sqlite3_api->ମୂଲ୍ୟ_pointer
/* Version 3.22.0 and later */
#define sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#define sqlite3_ମୂଲ୍ୟ_nochange         sqlite3_api->ମୂଲ୍ୟ_nochange
#define sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#define sqlite3_keyword_ଗଣନା          sqlite3_api->keyword_ଗଣନା
#define sqlite3_keyword_name           sqlite3_api->keyword_name
#define sqlite3_keyword_check          sqlite3_api->keyword_check
#define sqlite3_str_new                sqlite3_api->str_new
#define sqlite3_str_finish             sqlite3_api->str_finish
#define sqlite3_str_appendf            sqlite3_api->str_appendf
#define sqlite3_str_vappendf           sqlite3_api->str_vappendf
#define sqlite3_str_append             sqlite3_api->str_append
#define sqlite3_str_appendall          sqlite3_api->str_appendall
#define sqlite3_str_appendchar         sqlite3_api->str_appendchar
#define sqlite3_str_reset              sqlite3_api->str_reset
#define sqlite3_str_errcode            sqlite3_api->str_errcode
#define sqlite3_str_length             sqlite3_api->str_length
#define sqlite3_str_ମୂଲ୍ୟ              sqlite3_api->str_ମୂଲ୍ୟ
/* Version 3.25.0 and later */
#define sqlite3_create_window_function sqlite3_api->create_window_function
/* Version 3.26.0 and later */
#define sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#define sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#define sqlite3_ମୂଲ୍ୟ_frombind         sqlite3_api->ମୂଲ୍ୟ_frombind
/* Version 3.30.0 and later */
#define sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#define sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#define sqlite3_uri_ଚାବି                sqlite3_api->uri_ଚାବି
#define sqlite3_filename_database      sqlite3_api->filename_database
#define sqlite3_filename_journal       sqlite3_api->filename_journal
#define sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#define sqlite3_create_filename        sqlite3_api->create_filename
#define sqlite3_free_filename          sqlite3_api->free_filename
#define sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#define sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#define sqlite3_changes64              sqlite3_api->changes64
#define sqlite3_total_changes64        sqlite3_api->total_changes64
/* Version 3.37.0 and later */
#define sqlite3_autovacuum_pages       sqlite3_api->autovacuum_pages
/* Version 3.38.0 and later */
#define sqlite3_error_offset           sqlite3_api->error_offset
#define sqlite3_vtab_rhs_ମୂଲ୍ୟ         sqlite3_api->vtab_rhs_ମୂଲ୍ୟ
#define sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#define sqlite3_vtab_in                sqlite3_api->vtab_in
#define sqlite3_vtab_in_first          sqlite3_api->vtab_in_first
#define sqlite3_vtab_in_next           sqlite3_api->vtab_in_next
/* Version 3.39.0 and later */
#ifndef SQLITE_OMIT_DESERIALIZE
#define sqlite3_deserialize            sqlite3_api->deserialize
#define sqlite3_serialize              sqlite3_api->serialize
#endif
#define sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#define sqlite3_ମୂଲ୍ୟ_encoding         sqlite3_api->ମୂଲ୍ୟ_encoding
/* Version 3.41.0 and later */
#define sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#define sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#define sqlite3_get_clientdata         sqlite3_api->get_clientdata
#define sqlite3_set_clientdata         sqlite3_api->set_clientdata
/* Version 3.50.0 and later */
#define sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#define sqlite3_set_errmsg             sqlite3_api->set_errmsg
#define sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#define sqlite3_str_truncate           sqlite3_api->str_truncate
#define sqlite3_str_free               sqlite3_api->str_free
#define sqlite3_carray_bind            sqlite3_api->carray_bind
#define sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#define sqlite3_incomplete             sqlite3_api->incomplete
#define sqlite3_result_str             sqlite3_api->result_str
#endif /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#ଯଦି !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ଅଚଳ sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    ବାହ୍ୟ ଅଚଳ sqlite3_api_routines *sqlite3_api;
#ଅନ୍ୟଥା
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (ଶୂନ୍ୟ)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */
