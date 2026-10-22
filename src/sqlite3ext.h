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
#ಅನ್ತರ್ಭಾವ "sqlite3.h"

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
ಸಂರಚನಾ sqlite3_api_routines {
  ಶೂನ್ಯ * (*aggregate_context)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ nBytes);
  ಪೂರ್ಣಾಙ್ಕ  (*aggregate_ಎಣಿಕೆ)(sqlite3_context*);
  ಪೂರ್ಣಾಙ್ಕ  (*bind_blob)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ n,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ  (*bind_double)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ದ್ವಿಗುಣ);
  ಪೂರ್ಣಾಙ್ಕ  (*bind_int)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ  (*bind_int64)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,sqlite_int64);
  ಪೂರ್ಣಾಙ್ಕ  (*bind_null)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ  (*bind_parameter_ಎಣಿಕೆ)(sqlite3_stmt*);
  ಪೂರ್ಣಾಙ್ಕ  (*bind_parameter_index)(sqlite3_stmt*,ಧ್ರುವ ವರ್ಣ*zName);
  ಧ್ರುವ ವರ್ಣ * (*bind_parameter_name)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ  (*bind_text)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ n,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ  (*bind_text16)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ  (*bind_ಮೌಲ್ಯ)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*busy_handler)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*busy_timeout)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ ms);
  ಪೂರ್ಣಾಙ್ಕ  (*changes)(sqlite3*);
  ಪೂರ್ಣಾಙ್ಕ  (*close)(sqlite3*);
  ಪೂರ್ಣಾಙ್ಕ  (*collation_needed)(sqlite3*,ಶೂನ್ಯ*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*,sqlite3*,
                           ಪೂರ್ಣಾಙ್ಕ eTextRep,ಧ್ರುವ ವರ್ಣ*));
  ಪೂರ್ಣಾಙ್ಕ  (*collation_needed16)(sqlite3*,ಶೂನ್ಯ*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*,sqlite3*,
                             ಪೂರ್ಣಾಙ್ಕ eTextRep,ಧ್ರುವ ಶೂನ್ಯ*));
  ಧ್ರುವ ಶೂನ್ಯ * (*column_blob)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  ಪೂರ್ಣಾಙ್ಕ  (*column_bytes)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  ಪೂರ್ಣಾಙ್ಕ  (*column_bytes16)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  ಪೂರ್ಣಾಙ್ಕ  (*column_ಎಣಿಕೆ)(sqlite3_stmt*pStmt);
  ಧ್ರುವ ವರ್ಣ * (*column_database_name)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ಶೂನ್ಯ * (*column_database_name16)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ವರ್ಣ * (*column_decltype)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ i);
  ಧ್ರುವ ಶೂನ್ಯ * (*column_decltype16)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ದ್ವಿಗುಣ  (*column_double)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  ಪೂರ್ಣಾಙ್ಕ  (*column_int)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  ಧ್ರುವ ವರ್ಣ * (*column_name)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ಶೂನ್ಯ * (*column_name16)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ವರ್ಣ * (*column_origin_name)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ಶೂನ್ಯ * (*column_origin_name16)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ವರ್ಣ * (*column_table_name)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ಶೂನ್ಯ * (*column_table_name16)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ unsigned ವರ್ಣ * (*column_text)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  ಧ್ರುವ ಶೂನ್ಯ * (*column_text16)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  ಪೂರ್ಣಾಙ್ಕ  (*column_type)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  sqlite3_ಮೌಲ್ಯ* (*column_ಮೌಲ್ಯ)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ iCol);
  ಶೂನ್ಯ * (*commit_hook)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*complete)(ಧ್ರುವ ವರ್ಣ*sql);
  ಪೂರ್ಣಾಙ್ಕ  (*complete16)(ಧ್ರುವ ಶೂನ್ಯ*sql);
  ಪೂರ್ಣಾಙ್ಕ  (*create_collation)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,
                           ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ  (*create_collation16)(sqlite3*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,
                             ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ  (*create_function)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,
                          ಶೂನ್ಯ (*xFunc)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
                          ಶೂನ್ಯ (*xStep)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
                          ಶೂನ್ಯ (*xFinal)(sqlite3_context*));
  ಪೂರ್ಣಾಙ್ಕ  (*create_function16)(sqlite3*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,
                            ಶೂನ್ಯ (*xFunc)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*xStep)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*xFinal)(sqlite3_context*));
  ಪೂರ್ಣಾಙ್ಕ (*create_module)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ sqlite3_module*,ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*data_ಎಣಿಕೆ)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  ಪೂರ್ಣಾಙ್ಕ (*declare_vtab)(sqlite3*,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ  (*enable_shared_cache)(ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ  (*errcode)(sqlite3*db);
  ಧ್ರುವ ವರ್ಣ * (*errmsg)(sqlite3*);
  ಧ್ರುವ ಶೂನ್ಯ * (*errmsg16)(sqlite3*);
  ಪೂರ್ಣಾಙ್ಕ  (*exec)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,sqlite3_callback,ಶೂನ್ಯ*,ವರ್ಣ**);
  ಪೂರ್ಣಾಙ್ಕ  (*expired)(sqlite3_stmt*);
  ಪೂರ್ಣಾಙ್ಕ  (*finalize)(sqlite3_stmt*pStmt);
  ಶೂನ್ಯ  (*ಮುಕ್ತ)(ಶೂನ್ಯ*);
  ಶೂನ್ಯ  (*free_table)(ವರ್ಣ**result);
  ಪೂರ್ಣಾಙ್ಕ  (*get_autocommit)(sqlite3*);
  ಶೂನ್ಯ * (*get_auxdata)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ  (*get_table)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ವರ್ಣ***,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*,ವರ್ಣ**);
  ಪೂರ್ಣಾಙ್ಕ  (*global_recover)(ಶೂನ್ಯ);
  ಶೂನ್ಯ  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_ಸೇರಿಸು_rowid)(sqlite3*);
  ಧ್ರುವ ವರ್ಣ * (*libversion)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಙ್ಕ  (*libversion_ಸಂಖ್ಯೆ)(ಶೂನ್ಯ);
  ಶೂನ್ಯ *(*malloc)(ಪೂರ್ಣಾಙ್ಕ);
  ವರ್ಣ *(SQLITE_CDECL*mprintf)(ಧ್ರುವ ವರ್ಣ*,...);
  ಪೂರ್ಣಾಙ್ಕ  (*open)(ಧ್ರುವ ವರ್ಣ*,sqlite3**);
  ಪೂರ್ಣಾಙ್ಕ  (*open16)(ಧ್ರುವ ಶೂನ್ಯ*,sqlite3**);
  ಪೂರ್ಣಾಙ್ಕ  (*prepare)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_stmt**,ಧ್ರುವ ವರ್ಣ**);
  ಪೂರ್ಣಾಙ್ಕ  (*prepare16)(sqlite3*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_stmt**,ಧ್ರುವ ಶೂನ್ಯ**);
  ಶೂನ್ಯ * (*profile)(sqlite3*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*,ಧ್ರುವ ವರ್ಣ*,sqlite_uint64),ಶೂನ್ಯ*);
  ಶೂನ್ಯ  (*progress_handler)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  ಶೂನ್ಯ *(*realloc)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ  (*reset)(sqlite3_stmt*pStmt);
  ಶೂನ್ಯ  (*result_blob)(sqlite3_context*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*result_double)(sqlite3_context*,ದ್ವಿಗುಣ);
  ಶೂನ್ಯ  (*result_error)(sqlite3_context*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ);
  ಶೂನ್ಯ  (*result_error16)(sqlite3_context*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ);
  ಶೂನ್ಯ  (*result_int)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ);
  ಶೂನ್ಯ  (*result_int64)(sqlite3_context*,sqlite_int64);
  ಶೂನ್ಯ  (*result_null)(sqlite3_context*);
  ಶೂನ್ಯ  (*result_text)(sqlite3_context*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*result_text16)(sqlite3_context*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*result_text16be)(sqlite3_context*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*result_text16le)(sqlite3_context*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*result_ಮೌಲ್ಯ)(sqlite3_context*,sqlite3_ಮೌಲ್ಯ*);
  ಶೂನ್ಯ *(*rollback_hook)(sqlite3*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*set_authorizer)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,
                         ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*),ಶೂನ್ಯ*);
  ಶೂನ್ಯ  (*set_auxdata)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,ಶೂನ್ಯ (*)(ಶೂನ್ಯ*));
  ವರ್ಣ *(SQLITE_CDECL*xsnprintf)(ಪೂರ್ಣಾಙ್ಕ,ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,...);
  ಪೂರ್ಣಾಙ್ಕ  (*step)(sqlite3_stmt*);
  ಪೂರ್ಣಾಙ್ಕ  (*table_column_metadata)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,
                                ವರ್ಣ ಧ್ರುವ**,ವರ್ಣ ಧ್ರುವ**,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*);
  ಶೂನ್ಯ  (*thread_cleanup)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಙ್ಕ  (*total_changes)(sqlite3*);
  ಶೂನ್ಯ * (*trace)(sqlite3*,ಶೂನ್ಯ(*xTrace)(ಶೂನ್ಯ*,ಧ್ರುವ ವರ್ಣ*),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  ಶೂನ್ಯ * (*update_hook)(sqlite3*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ ,ವರ್ಣ ಧ್ರುವ*,ವರ್ಣ ಧ್ರುವ*,
                                         sqlite_int64),ಶೂನ್ಯ*);
  ಶೂನ್ಯ * (*user_data)(sqlite3_context*);
  ಧ್ರುವ ಶೂನ್ಯ * (*ಮೌಲ್ಯ_blob)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*ಮೌಲ್ಯ_bytes)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*ಮೌಲ್ಯ_bytes16)(sqlite3_ಮೌಲ್ಯ*);
  ದ್ವಿಗುಣ  (*ಮೌಲ್ಯ_double)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*ಮೌಲ್ಯ_int)(sqlite3_ಮೌಲ್ಯ*);
  sqlite_int64  (*ಮೌಲ್ಯ_int64)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*ಮೌಲ್ಯ_numeric_type)(sqlite3_ಮೌಲ್ಯ*);
  ಧ್ರುವ unsigned ವರ್ಣ * (*ಮೌಲ್ಯ_text)(sqlite3_ಮೌಲ್ಯ*);
  ಧ್ರುವ ಶೂನ್ಯ * (*ಮೌಲ್ಯ_text16)(sqlite3_ಮೌಲ್ಯ*);
  ಧ್ರುವ ಶೂನ್ಯ * (*ಮೌಲ್ಯ_text16be)(sqlite3_ಮೌಲ್ಯ*);
  ಧ್ರುವ ಶೂನ್ಯ * (*ಮೌಲ್ಯ_text16le)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ  (*ಮೌಲ್ಯ_type)(sqlite3_ಮೌಲ್ಯ*);
  ವರ್ಣ *(*vmprintf)(ಧ್ರುವ ವರ್ಣ*,va_list);
  /* Added ??? */
  ಪೂರ್ಣಾಙ್ಕ (*overload_function)(sqlite3*, ಧ್ರುವ ವರ್ಣ *zFuncName, ಪೂರ್ಣಾಙ್ಕ nArg);
  /* Added by 3.3.13 */
  ಪೂರ್ಣಾಙ್ಕ (*prepare_v2)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_stmt**,ಧ್ರುವ ವರ್ಣ**);
  ಪೂರ್ಣಾಙ್ಕ (*prepare16_v2)(sqlite3*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_stmt**,ಧ್ರುವ ಶೂನ್ಯ**);
  ಪೂರ್ಣಾಙ್ಕ (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  ಪೂರ್ಣಾಙ್ಕ (*create_module_v2)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ sqlite3_module*,ಶೂನ್ಯ*,
                          ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ *));
  /* Added by 3.5.0 */
  ಪೂರ್ಣಾಙ್ಕ (*bind_zeroblob)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*blob_bytes)(sqlite3_blob*);
  ಪೂರ್ಣಾಙ್ಕ (*blob_close)(sqlite3_blob*);
  ಪೂರ್ಣಾಙ್ಕ (*blob_open)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,sqlite3_int64,
                   ಪೂರ್ಣಾಙ್ಕ,sqlite3_blob**);
  ಪೂರ್ಣಾಙ್ಕ (*blob_read)(sqlite3_blob*,ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*blob_write)(sqlite3_blob*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*create_collation_v2)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,
                             ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*),
                             ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ (*file_control)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*);
  sqlite3_int64 (*memory_highwater)(ಪೂರ್ಣಾಙ್ಕ);
  sqlite3_int64 (*memory_used)(ಶೂನ್ಯ);
  sqlite3_mutex *(*mutex_alloc)(ಪೂರ್ಣಾಙ್ಕ);
  ಶೂನ್ಯ (*mutex_enter)(sqlite3_mutex*);
  ಶೂನ್ಯ (*mutex_free)(sqlite3_mutex*);
  ಶೂನ್ಯ (*mutex_leave)(sqlite3_mutex*);
  ಪೂರ್ಣಾಙ್ಕ (*mutex_try)(sqlite3_mutex*);
  ಪೂರ್ಣಾಙ್ಕ (*open_v2)(ಧ್ರುವ ವರ್ಣ*,sqlite3**,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*release_memory)(ಪೂರ್ಣಾಙ್ಕ);
  ಶೂನ್ಯ (*result_error_nomem)(sqlite3_context*);
  ಶೂನ್ಯ (*result_error_toobig)(sqlite3_context*);
  ಪೂರ್ಣಾಙ್ಕ (*sleep)(ಪೂರ್ಣಾಙ್ಕ);
  ಶೂನ್ಯ (*soft_heap_limit)(ಪೂರ್ಣಾಙ್ಕ);
  sqlite3_vfs *(*vfs_find)(ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*vfs_register)(sqlite3_vfs*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*vfs_unregister)(sqlite3_vfs*);
  ಪೂರ್ಣಾಙ್ಕ (*xthreadsafe)(ಶೂನ್ಯ);
  ಶೂನ್ಯ (*result_zeroblob)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ);
  ಶೂನ್ಯ (*result_error_code)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (SQLITE_CDECL*test_control)(ಪೂರ್ಣಾಙ್ಕ, ...);
  ಶೂನ್ಯ (*randomness)(ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  ಪೂರ್ಣಾಙ್ಕ (*extended_result_codes)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*limit)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
  sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
  ಧ್ರುವ ವರ್ಣ *(*sql)(sqlite3_stmt*);
  ಪೂರ್ಣಾಙ್ಕ (*status)(ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*backup_finish)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,sqlite3*,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*backup_pagecount)(sqlite3_backup*);
  ಪೂರ್ಣಾಙ್ಕ (*backup_remaining)(sqlite3_backup*);
  ಪೂರ್ಣಾಙ್ಕ (*backup_step)(sqlite3_backup*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ವರ್ಣ *(*compileoption_get)(ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*compileoption_used)(ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*create_function_v2)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,
                            ಶೂನ್ಯ (*xFunc)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*xStep)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*xFinal)(sqlite3_context*),
                            ಶೂನ್ಯ(*xDestroy)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ (SQLITE_CDECL*db_config)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  ಪೂರ್ಣಾಙ್ಕ (*db_status)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*extended_errcode)(sqlite3*);
  ಶೂನ್ಯ (SQLITE_CDECL*log)(ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ವರ್ಣ*,...);
  sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
  ಧ್ರುವ ವರ್ಣ *(*sourceid)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಙ್ಕ (*stmt_status)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*strnicmp)(ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*unlock_notify)(sqlite3*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ**,ಪೂರ್ಣಾಙ್ಕ),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ (*wal_autocheckpoint)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*wal_checkpoint)(sqlite3*,ಧ್ರುವ ವರ್ಣ*);
  ಶೂನ್ಯ *(*wal_hook)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*,sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  ಪೂರ್ಣಾಙ್ಕ (SQLITE_CDECL*vtab_config)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ op,...);
  ಪೂರ್ಣಾಙ್ಕ (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  ಪೂರ್ಣಾಙ್ಕ (*close_v2)(sqlite3*);
  ಧ್ರುವ ವರ್ಣ *(*db_filename)(sqlite3*,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*db_readonly)(sqlite3*,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*db_release_memory)(sqlite3*);
  ಧ್ರುವ ವರ್ಣ *(*errstr)(ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*stmt_busy)(sqlite3_stmt*);
  ಪೂರ್ಣಾಙ್ಕ (*stmt_readonly)(sqlite3_stmt*);
  ಪೂರ್ಣಾಙ್ಕ (*stricmp)(ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*uri_boolean)(ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ);
  sqlite3_int64 (*uri_int64)(ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,sqlite3_int64);
  ಧ್ರುವ ವರ್ಣ *(*uri_parameter)(ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*);
  ವರ್ಣ *(*xvsnprintf)(ಪೂರ್ಣಾಙ್ಕ,ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,va_list);
  ಪೂರ್ಣಾಙ್ಕ (*wal_checkpoint_v2)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*);
  /* Version 3.8.7 and later */
  ಪೂರ್ಣಾಙ್ಕ (*auto_extension)(ಶೂನ್ಯ(*)(ಶೂನ್ಯ));
  ಪೂರ್ಣಾಙ್ಕ (*bind_blob64)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,sqlite3_uint64,
                     ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ (*bind_text64)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ವರ್ಣ*,sqlite3_uint64,
                      ಶೂನ್ಯ(*)(ಶೂನ್ಯ*),unsigned ವರ್ಣ);
  ಪೂರ್ಣಾಙ್ಕ (*cancel_auto_extension)(ಶೂನ್ಯ(*)(ಶೂನ್ಯ));
  ಪೂರ್ಣಾಙ್ಕ (*load_extension)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,ವರ್ಣ**);
  ಶೂನ್ಯ *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(ಶೂನ್ಯ*);
  ಶೂನ್ಯ *(*realloc64)(ಶೂನ್ಯ*,sqlite3_uint64);
  ಶೂನ್ಯ (*reset_auto_extension)(ಶೂನ್ಯ);
  ಶೂನ್ಯ (*result_blob64)(sqlite3_context*,ಧ್ರುವ ಶೂನ್ಯ*,sqlite3_uint64,
                        ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ (*result_text64)(sqlite3_context*,ಧ್ರುವ ವರ್ಣ*,sqlite3_uint64,
                         ಶೂನ್ಯ(*)(ಶೂನ್ಯ*), unsigned ವರ್ಣ);
  ಪೂರ್ಣಾಙ್ಕ (*strglob)(ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*);
  /* Version 3.8.11 and later */
  sqlite3_ಮೌಲ್ಯ *(*ಮೌಲ್ಯ_dup)(ಧ್ರುವ sqlite3_ಮೌಲ್ಯ*);
  ಶೂನ್ಯ (*ಮೌಲ್ಯ_free)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಙ್ಕ (*result_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  ಪೂರ್ಣಾಙ್ಕ (*bind_zeroblob64)(sqlite3_stmt*, ಪೂರ್ಣಾಙ್ಕ, sqlite3_uint64);
  /* Version 3.9.0 and later */
  unsigned ಪೂರ್ಣಾಙ್ಕ (*ಮೌಲ್ಯ_subtype)(sqlite3_ಮೌಲ್ಯ*);
  ಶೂನ್ಯ (*result_subtype)(sqlite3_context*,unsigned ಪೂರ್ಣಾಙ್ಕ);
  /* Version 3.10.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*status64)(ಪೂರ್ಣಾಙ್ಕ,sqlite3_int64*,sqlite3_int64*,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*strlike)(ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,unsigned ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*trace_v2)(sqlite3*,unsigned,ಪೂರ್ಣಾಙ್ಕ(*)(unsigned,ಶೂನ್ಯ*,ಶೂನ್ಯ*,ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  ವರ್ಣ *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  ಶೂನ್ಯ (*set_last_ಸೇರಿಸು_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*prepare_v3)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,unsigned ಪೂರ್ಣಾಙ್ಕ,
                    sqlite3_stmt**,ಧ್ರುವ ವರ್ಣ**);
  ಪೂರ್ಣಾಙ್ಕ (*prepare16_v3)(sqlite3*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,unsigned ಪೂರ್ಣಾಙ್ಕ,
                      sqlite3_stmt**,ಧ್ರುವ ಶೂನ್ಯ**);
  ಪೂರ್ಣಾಙ್ಕ (*bind_pointer)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,ಧ್ರುವ ವರ್ಣ*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ (*result_pointer)(sqlite3_context*,ಶೂನ್ಯ*,ಧ್ರುವ ವರ್ಣ*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ *(*ಮೌಲ್ಯ_pointer)(sqlite3_ಮೌಲ್ಯ*,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*vtab_nochange)(sqlite3_context*);
  ಪೂರ್ಣಾಙ್ಕ (*ಮೌಲ್ಯ_nochange)(sqlite3_ಮೌಲ್ಯ*);
  ಧ್ರುವ ವರ್ಣ *(*vtab_collation)(sqlite3_index_info*,ಪೂರ್ಣಾಙ್ಕ);
  /* Version 3.24.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*keyword_ಎಣಿಕೆ)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಙ್ಕ (*keyword_name)(ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ವರ್ಣ**,ಪೂರ್ಣಾಙ್ಕ*);
  ಪೂರ್ಣಾಙ್ಕ (*keyword_check)(ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ);
  sqlite3_str *(*str_new)(sqlite3*);
  ವರ್ಣ *(*str_finish)(sqlite3_str*);
  ಶೂನ್ಯ (SQLITE_CDECL*str_appendf)(sqlite3_str*, ಧ್ರುವ ವರ್ಣ *zFormat, ...);
  ಶೂನ್ಯ (*str_vappendf)(sqlite3_str*, ಧ್ರುವ ವರ್ಣ *zFormat, va_list);
  ಶೂನ್ಯ (*str_append)(sqlite3_str*, ಧ್ರುವ ವರ್ಣ *zIn, ಪೂರ್ಣಾಙ್ಕ N);
  ಶೂನ್ಯ (*str_appendall)(sqlite3_str*, ಧ್ರುವ ವರ್ಣ *zIn);
  ಶೂನ್ಯ (*str_appendchar)(sqlite3_str*, ಪೂರ್ಣಾಙ್ಕ N, ವರ್ಣ C);
  ಶೂನ್ಯ (*str_reset)(sqlite3_str*);
  ಪೂರ್ಣಾಙ್ಕ (*str_errcode)(sqlite3_str*);
  ಪೂರ್ಣಾಙ್ಕ (*str_length)(sqlite3_str*);
  ವರ್ಣ *(*str_ಮೌಲ್ಯ)(sqlite3_str*);
  /* Version 3.25.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*create_window_function)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,
                            ಶೂನ್ಯ (*xStep)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*xFinal)(sqlite3_context*),
                            ಶೂನ್ಯ (*x_ಮೌಲ್ಯ)(sqlite3_context*),
                            ಶೂನ್ಯ (*xInv)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ(*xDestroy)(ಶೂನ್ಯ*));
  /* Version 3.26.0 and later */
  ಧ್ರುವ ವರ್ಣ *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*stmt_isexplain)(sqlite3_stmt*);
  ಪೂರ್ಣಾಙ್ಕ (*ಮೌಲ್ಯ_frombind)(sqlite3_ಮೌಲ್ಯ*);
  /* Version 3.30.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*drop_modules)(sqlite3*,ಧ್ರುವ ವರ್ಣ**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_heap_limit64)(sqlite3_int64);
  ಧ್ರುವ ವರ್ಣ *(*uri_ಕೀಲಿ)(ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ವರ್ಣ *(*filename_database)(ಧ್ರುವ ವರ್ಣ*);
  ಧ್ರುವ ವರ್ಣ *(*filename_journal)(ಧ್ರುವ ವರ್ಣ*);
  ಧ್ರುವ ವರ್ಣ *(*filename_wal)(ಧ್ರುವ ವರ್ಣ*);
  /* Version 3.32.0 and later */
  ಧ್ರುವ ವರ್ಣ *(*create_filename)(ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,ಧ್ರುವ ವರ್ಣ*,
                           ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ವರ್ಣ**);
  ಶೂನ್ಯ (*free_filename)(ಧ್ರುವ ವರ್ಣ*);
  sqlite3_file *(*database_file_object)(ಧ್ರುವ ವರ್ಣ*);
  /* Version 3.34.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*txn_state)(sqlite3*,ಧ್ರುವ ವರ್ಣ*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*total_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*autovacuum_pages)(sqlite3*,
     unsigned ಪೂರ್ಣಾಙ್ಕ(*)(ಶೂನ್ಯ*,ಧ್ರುವ ವರ್ಣ*,unsigned ಪೂರ್ಣಾಙ್ಕ,unsigned ಪೂರ್ಣಾಙ್ಕ,unsigned ಪೂರ್ಣಾಙ್ಕ),
     ಶೂನ್ಯ*, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  /* Version 3.38.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*error_offset)(sqlite3*);
  ಪೂರ್ಣಾಙ್ಕ (*vtab_rhs_ಮೌಲ್ಯ)(sqlite3_index_info*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**);
  ಪೂರ್ಣಾಙ್ಕ (*vtab_distinct)(sqlite3_index_info*);
  ಪೂರ್ಣಾಙ್ಕ (*vtab_in)(sqlite3_index_info*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
  ಪೂರ್ಣಾಙ್ಕ (*vtab_in_first)(sqlite3_ಮೌಲ್ಯ*,sqlite3_ಮೌಲ್ಯ**);
  ಪೂರ್ಣಾಙ್ಕ (*vtab_in_next)(sqlite3_ಮೌಲ್ಯ*,sqlite3_ಮೌಲ್ಯ**);
  /* Version 3.39.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*deserialize)(sqlite3*,ಧ್ರುವ ವರ್ಣ*,unsigned ವರ್ಣ*,
                     sqlite3_int64,sqlite3_int64,unsigned);
  unsigned ವರ್ಣ *(*serialize)(sqlite3*,ಧ್ರುವ ವರ್ಣ *,sqlite3_int64*,
                              unsigned ಪೂರ್ಣಾಙ್ಕ);
  ಧ್ರುವ ವರ್ಣ *(*db_name)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ);
  /* Version 3.40.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*ಮೌಲ್ಯ_encoding)(sqlite3_ಮೌಲ್ಯ*);
  /* Version 3.41.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*stmt_explain)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ);
  /* Version 3.44.0 and later */
  ಶೂನ್ಯ *(*get_clientdata)(sqlite3*,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*set_clientdata)(sqlite3*, ಧ್ರುವ ವರ್ಣ*, ಶೂನ್ಯ*, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  /* Version 3.50.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*setlk_timeout)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ);
  /* Version 3.51.0 and later */
  ಪೂರ್ಣಾಙ್ಕ (*set_errmsg)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ವರ್ಣ*);
  ಪೂರ್ಣಾಙ್ಕ (*db_status64)(sqlite3*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_int64*,sqlite3_int64*,ಪೂರ್ಣಾಙ್ಕ);
  /* Version 3.52.0 and later */
  ಶೂನ್ಯ (*str_truncate)(sqlite3_str*,ಪೂರ್ಣಾಙ್ಕ);
  ಶೂನ್ಯ (*str_free)(sqlite3_str*);
  ಪೂರ್ಣಾಙ್ಕ (*carray_bind)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ (*carray_bind_v2)(sqlite3_stmt*,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ಧ್ರುವ ವರ್ಣ*);
  ಶೂನ್ಯ (*result_str)(sqlite3_context*,sqlite3_str*,ಪೂರ್ಣಾಙ್ಕ);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ (*sqlite3_loadext_entry)(
  sqlite3 *db,                       /* Handle to the database. */
  ವರ್ಣ **pz_Err_ಸಂದೇಶ,                   /* Used to set error string on failure. */
  ಧ್ರುವ sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#ಯದಿ !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
#define sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_aggregate_ಎಣಿಕೆ        sqlite3_api->aggregate_ಎಣಿಕೆ
#endif
#define sqlite3_bind_blob              sqlite3_api->bind_blob
#define sqlite3_bind_double            sqlite3_api->bind_double
#define sqlite3_bind_int               sqlite3_api->bind_int
#define sqlite3_bind_int64             sqlite3_api->bind_int64
#define sqlite3_bind_null              sqlite3_api->bind_null
#define sqlite3_bind_parameter_ಎಣಿಕೆ   sqlite3_api->bind_parameter_ಎಣಿಕೆ
#define sqlite3_bind_parameter_index   sqlite3_api->bind_parameter_index
#define sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#define sqlite3_bind_text              sqlite3_api->bind_text
#define sqlite3_bind_text16            sqlite3_api->bind_text16
#define sqlite3_bind_ಮೌಲ್ಯ             sqlite3_api->bind_ಮೌಲ್ಯ
#define sqlite3_busy_handler           sqlite3_api->busy_handler
#define sqlite3_busy_timeout           sqlite3_api->busy_timeout
#define sqlite3_changes                sqlite3_api->changes
#define sqlite3_close                  sqlite3_api->close
#define sqlite3_collation_needed       sqlite3_api->collation_needed
#define sqlite3_collation_needed16     sqlite3_api->collation_needed16
#define sqlite3_column_blob            sqlite3_api->column_blob
#define sqlite3_column_bytes           sqlite3_api->column_bytes
#define sqlite3_column_bytes16         sqlite3_api->column_bytes16
#define sqlite3_column_ಎಣಿಕೆ           sqlite3_api->column_ಎಣಿಕೆ
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
#define sqlite3_column_ಮೌಲ್ಯ           sqlite3_api->column_ಮೌಲ್ಯ
#define sqlite3_commit_hook            sqlite3_api->commit_hook
#define sqlite3_complete               sqlite3_api->complete
#define sqlite3_complete16             sqlite3_api->complete16
#define sqlite3_create_collation       sqlite3_api->create_collation
#define sqlite3_create_collation16     sqlite3_api->create_collation16
#define sqlite3_create_function        sqlite3_api->create_function
#define sqlite3_create_function16      sqlite3_api->create_function16
#define sqlite3_create_module          sqlite3_api->create_module
#define sqlite3_create_module_v2       sqlite3_api->create_module_v2
#define sqlite3_data_ಎಣಿಕೆ             sqlite3_api->data_ಎಣಿಕೆ
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
#define sqlite3_free                   sqlite3_api->ಮುಕ್ತ
#define sqlite3_free_table             sqlite3_api->free_table
#define sqlite3_get_autocommit         sqlite3_api->get_autocommit
#define sqlite3_get_auxdata            sqlite3_api->get_auxdata
#define sqlite3_get_table              sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_global_recover         sqlite3_api->global_recover
#endif
#define sqlite3_interrupt              sqlite3_api->interruptx
#define sqlite3_last_ಸೇರಿಸು_rowid      sqlite3_api->last_ಸೇರಿಸು_rowid
#define sqlite3_libversion             sqlite3_api->libversion
#define sqlite3_libversion_ಸಂಖ್ಯೆ      sqlite3_api->libversion_ಸಂಖ್ಯೆ
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
#define sqlite3_result_ಮೌಲ್ಯ           sqlite3_api->result_ಮೌಲ್ಯ
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
#define sqlite3_ಮೌಲ್ಯ_blob             sqlite3_api->ಮೌಲ್ಯ_blob
#define sqlite3_ಮೌಲ್ಯ_bytes            sqlite3_api->ಮೌಲ್ಯ_bytes
#define sqlite3_ಮೌಲ್ಯ_bytes16          sqlite3_api->ಮೌಲ್ಯ_bytes16
#define sqlite3_ಮೌಲ್ಯ_double           sqlite3_api->ಮೌಲ್ಯ_double
#define sqlite3_ಮೌಲ್ಯ_int              sqlite3_api->ಮೌಲ್ಯ_int
#define sqlite3_ಮೌಲ್ಯ_int64            sqlite3_api->ಮೌಲ್ಯ_int64
#define sqlite3_ಮೌಲ್ಯ_numeric_type     sqlite3_api->ಮೌಲ್ಯ_numeric_type
#define sqlite3_ಮೌಲ್ಯ_text             sqlite3_api->ಮೌಲ್ಯ_text
#define sqlite3_ಮೌಲ್ಯ_text16           sqlite3_api->ಮೌಲ್ಯ_text16
#define sqlite3_ಮೌಲ್ಯ_text16be         sqlite3_api->ಮೌಲ್ಯ_text16be
#define sqlite3_ಮೌಲ್ಯ_text16le         sqlite3_api->ಮೌಲ್ಯ_text16le
#define sqlite3_ಮೌಲ್ಯ_type             sqlite3_api->ಮೌಲ್ಯ_type
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
#define sqlite3_ಮೌಲ್ಯ_dup              sqlite3_api->ಮೌಲ್ಯ_dup
#define sqlite3_ಮೌಲ್ಯ_free             sqlite3_api->ಮೌಲ್ಯ_free
#define sqlite3_result_zeroblob64      sqlite3_api->result_zeroblob64
#define sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#define sqlite3_ಮೌಲ್ಯ_subtype          sqlite3_api->ಮೌಲ್ಯ_subtype
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
#define sqlite3_set_last_ಸೇರಿಸು_rowid  sqlite3_api->set_last_ಸೇರಿಸು_rowid
/* Version 3.20.0 and later */
#define sqlite3_prepare_v3             sqlite3_api->prepare_v3
#define sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#define sqlite3_bind_pointer           sqlite3_api->bind_pointer
#define sqlite3_result_pointer         sqlite3_api->result_pointer
#define sqlite3_ಮೌಲ್ಯ_pointer          sqlite3_api->ಮೌಲ್ಯ_pointer
/* Version 3.22.0 and later */
#define sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#define sqlite3_ಮೌಲ್ಯ_nochange         sqlite3_api->ಮೌಲ್ಯ_nochange
#define sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#define sqlite3_keyword_ಎಣಿಕೆ          sqlite3_api->keyword_ಎಣಿಕೆ
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
#define sqlite3_str_ಮೌಲ್ಯ              sqlite3_api->str_ಮೌಲ್ಯ
/* Version 3.25.0 and later */
#define sqlite3_create_window_function sqlite3_api->create_window_function
/* Version 3.26.0 and later */
#define sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#define sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#define sqlite3_ಮೌಲ್ಯ_frombind         sqlite3_api->ಮೌಲ್ಯ_frombind
/* Version 3.30.0 and later */
#define sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#define sqlite3_hard_heap_limit64      sqlite3_api->hard_heap_limit64
#define sqlite3_uri_ಕೀಲಿ                sqlite3_api->uri_ಕೀಲಿ
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
#define sqlite3_vtab_rhs_ಮೌಲ್ಯ         sqlite3_api->vtab_rhs_ಮೌಲ್ಯ
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
#define sqlite3_ಮೌಲ್ಯ_encoding         sqlite3_api->ಮೌಲ್ಯ_encoding
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

#ಯದಿ !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ಧ್ರುವ sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    ಬಾಹ್ಯ ಧ್ರುವ sqlite3_api_routines *sqlite3_api;
#ಅನ್ಯಥಾ
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (ಶೂನ್ಯ)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#endif

#endif /* SQLITE3EXT_H */
