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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE3EXT_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE3EXT_H
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
ರಚನೆ sqlite3_api_routines {
  ಶೂನ್ಯ * (*aggregate_context)(sqlite3_context*,ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು);
  ಪೂರ್ಣಾಂಕ  (*aggregate_ಎಣಿಕೆ)(sqlite3_context*);
  ಪೂರ್ಣಾಂಕ  (*bind_blob)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ n,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ  (*bind_double)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ದ್ವಿಗುಣ);
  ಪೂರ್ಣಾಂಕ  (*bind_int)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ  (*bind_int64)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,sqlite_int64);
  ಪೂರ್ಣಾಂಕ  (*bind_null)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ  (*bind_parameter_ಎಣಿಕೆ)(sqlite3_stmt*);
  ಪೂರ್ಣಾಂಕ  (*bind_parameter_ಸೂಚ್ಯಂಕ)(sqlite3_stmt*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*zName);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*bind_parameter_name)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ  (*bind_ಪಠ್ಯ)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ n,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ  (*bind_text16)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ  (*bind_ಮೌಲ್ಯ)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*busy_handler)(sqlite3*,ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*busy_timeout)(sqlite3*,ಪೂರ್ಣಾಂಕ ms);
  ಪೂರ್ಣಾಂಕ  (*changes)(sqlite3*);
  ಪೂರ್ಣಾಂಕ  (*close)(sqlite3*);
  ಪೂರ್ಣಾಂಕ  (*collation_needed)(sqlite3*,ಶೂನ್ಯ*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*,sqlite3*,
                           ಪೂರ್ಣಾಂಕ e_ಪಠ್ಯ_Rep,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*));
  ಪೂರ್ಣಾಂಕ  (*collation_needed16)(sqlite3*,ಶೂನ್ಯ*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*,sqlite3*,
                             ಪೂರ್ಣಾಂಕ e_ಪಠ್ಯ_Rep,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*));
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಸ್ತಂಭ_blob)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  ಪೂರ್ಣಾಂಕ  (*ಸ್ತಂಭ_ಬೈಟುಗಳು)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  ಪೂರ್ಣಾಂಕ  (*ಸ್ತಂಭ_bytes16)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  ಪೂರ್ಣಾಂಕ  (*ಸ್ತಂಭ_ಎಣಿಕೆ)(sqlite3_stmt*pStmt);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*ಸ್ತಂಭ_database_name)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಸ್ತಂಭ_database_name16)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*ಸ್ತಂಭ_decltype)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಸ್ತಂಭ_decltype16)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ದ್ವಿಗುಣ  (*ಸ್ತಂಭ_double)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  ಪೂರ್ಣಾಂಕ  (*ಸ್ತಂಭ_int)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  sqlite_int64  (*ಸ್ತಂಭ_int64)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*ಸ್ತಂಭ_name)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಸ್ತಂಭ_name16)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*ಸ್ತಂಭ_origin_name)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಸ್ತಂಭ_origin_name16)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name16)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ * (*ಸ್ತಂಭ_ಪಠ್ಯ)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಸ್ತಂಭ_text16)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  ಪೂರ್ಣಾಂಕ  (*ಸ್ತಂಭ_type)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  sqlite3_ಮೌಲ್ಯ* (*ಸ್ತಂಭ_ಮೌಲ್ಯ)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು);
  ಶೂನ್ಯ * (*commit_hook)(sqlite3*,ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*complete)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*sql);
  ಪೂರ್ಣಾಂಕ  (*complete16)(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*sql);
  ಪೂರ್ಣಾಂಕ  (*create_collation)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,
                           ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ  (*create_collation16)(sqlite3*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,
                             ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ  (*create_function)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,
                          ಶೂನ್ಯ (*xFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
                          ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
                          ಶೂನ್ಯ (*xFinal)(sqlite3_context*));
  ಪೂರ್ಣಾಂಕ  (*create_function16)(sqlite3*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,
                            ಶೂನ್ಯ (*xFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*xFinal)(sqlite3_context*));
  ಪೂರ್ಣಾಂಕ (*create_module)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ sqlite3_module*,ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*ದತ್ತಾಂಶ_ಎಣಿಕೆ)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  ಪೂರ್ಣಾಂಕ (*declare_vtab)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ  (*enable_shared_cache)(ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ  (*errcode)(sqlite3*db);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*errmsg)(sqlite3*);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*errmsg16)(sqlite3*);
  ಪೂರ್ಣಾಂಕ  (*exec)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3_callback,ಶೂನ್ಯ*,ಅಕ್ಷರ**);
  ಪೂರ್ಣಾಂಕ  (*expired)(sqlite3_stmt*);
  ಪೂರ್ಣಾಂಕ  (*finalize)(sqlite3_stmt*pStmt);
  ಶೂನ್ಯ  (*ಬಿಡುಗಡೆ)(ಶೂನ್ಯ*);
  ಶೂನ್ಯ  (*free_ಕೋಷ್ಟಕ)(ಅಕ್ಷರ**ಫಲಿತಾಂಶ);
  ಪೂರ್ಣಾಂಕ  (*get_autocommit)(sqlite3*);
  ಶೂನ್ಯ * (*get_auxdata)(sqlite3_context*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ  (*get_ಕೋಷ್ಟಕ)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಅಕ್ಷರ***,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*,ಅಕ್ಷರ**);
  ಪೂರ್ಣಾಂಕ  (*global_recover)(ಶೂನ್ಯ);
  ಶೂನ್ಯ  (*interruptx)(sqlite3*);
  sqlite_int64  (*ಕೊನೆಯ_ಸೇರಿಸು_rowid)(sqlite3*);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*libversion)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಂಕ  (*libversion_ಸಂಖ್ಯೆ)(ಶೂನ್ಯ);
  ಶೂನ್ಯ *(*ಸ್ಮರಣೆ_ಕೊಡು)(ಪೂರ್ಣಾಂಕ);
  ಅಕ್ಷರ *(SQLITE_CDECL*mprintf)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,...);
  ಪೂರ್ಣಾಂಕ  (*open)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3**);
  ಪೂರ್ಣಾಂಕ  (*open16)(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,sqlite3**);
  ಪೂರ್ಣಾಂಕ  (*prepare)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,sqlite3_stmt**,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**);
  ಪೂರ್ಣಾಂಕ  (*prepare16)(sqlite3*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,sqlite3_stmt**,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ**);
  ಶೂನ್ಯ * (*profile)(sqlite3*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite_uint64),ಶೂನ್ಯ*);
  ಶೂನ್ಯ  (*ಪ್ರಗತಿ_handler)(sqlite3*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  ಶೂನ್ಯ *(*realloc)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ  (*reset)(sqlite3_stmt*pStmt);
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_blob)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_double)(sqlite3_context*,ದ್ವಿಗುಣ);
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_error)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_error16)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_int)(sqlite3_context*,ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_int64)(sqlite3_context*,sqlite_int64);
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_null)(sqlite3_context*);
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_ಪಠ್ಯ)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_text16)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_text16be)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_text16le)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ  (*ಫಲಿತಾಂಶ_ಮೌಲ್ಯ)(sqlite3_context*,sqlite3_ಮೌಲ್ಯ*);
  ಶೂನ್ಯ *(*rollback_hook)(sqlite3*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*ಗಣ_authorizer)(sqlite3*,ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
                         ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*),ಶೂನ್ಯ*);
  ಶೂನ್ಯ  (*ಗಣ_auxdata)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,ಶೂನ್ಯ (*)(ಶೂನ್ಯ*));
  ಅಕ್ಷರ *(SQLITE_CDECL*xsnprintf)(ಪೂರ್ಣಾಂಕ,ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,...);
  ಪೂರ್ಣಾಂಕ  (*ಹೆಜ್ಜೆ)(sqlite3_stmt*);
  ಪೂರ್ಣಾಂಕ  (*ಕೋಷ್ಟಕ_ಸ್ತಂಭ_metadata)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
                                ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ**,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ**,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*);
  ಶೂನ್ಯ  (*thread_cleanup)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಂಕ  (*ಒಟ್ಟು_changes)(sqlite3*);
  ಶೂನ್ಯ * (*trace)(sqlite3*,ಶೂನ್ಯ(*xTrace)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  ಶೂನ್ಯ * (*update_hook)(sqlite3*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ ,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ*,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ*,
                                         sqlite_int64),ಶೂನ್ಯ*);
  ಶೂನ್ಯ * (*user_ದತ್ತಾಂಶ)(sqlite3_context*);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಮೌಲ್ಯ_blob)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*ಮೌಲ್ಯ_ಬೈಟುಗಳು)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*ಮೌಲ್ಯ_bytes16)(sqlite3_ಮೌಲ್ಯ*);
  ದ್ವಿಗುಣ  (*ಮೌಲ್ಯ_double)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*ಮೌಲ್ಯ_int)(sqlite3_ಮೌಲ್ಯ*);
  sqlite_int64  (*ಮೌಲ್ಯ_int64)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*ಮೌಲ್ಯ_numeric_type)(sqlite3_ಮೌಲ್ಯ*);
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ * (*ಮೌಲ್ಯ_ಪಠ್ಯ)(sqlite3_ಮೌಲ್ಯ*);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಮೌಲ್ಯ_text16)(sqlite3_ಮೌಲ್ಯ*);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಮೌಲ್ಯ_text16be)(sqlite3_ಮೌಲ್ಯ*);
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ * (*ಮೌಲ್ಯ_text16le)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಂಕ  (*ಮೌಲ್ಯ_type)(sqlite3_ಮೌಲ್ಯ*);
  ಅಕ್ಷರ *(*vmprintf)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,va_list);
  /* Added ??? */
  ಪೂರ್ಣಾಂಕ (*overload_function)(sqlite3*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFuncName, ಪೂರ್ಣಾಂಕ nArg);
  /* Added by 3.3.13 */
  ಪೂರ್ಣಾಂಕ (*prepare_v2)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,sqlite3_stmt**,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**);
  ಪೂರ್ಣಾಂಕ (*prepare16_v2)(sqlite3*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,sqlite3_stmt**,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ**);
  ಪೂರ್ಣಾಂಕ (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  ಪೂರ್ಣಾಂಕ (*create_module_v2)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ sqlite3_module*,ಶೂನ್ಯ*,
                          ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ *));
  /* Added by 3.5.0 */
  ಪೂರ್ಣಾಂಕ (*bind_zeroblob)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*blob_ಬೈಟುಗಳು)(sqlite3_blob*);
  ಪೂರ್ಣಾಂಕ (*blob_close)(sqlite3_blob*);
  ಪೂರ್ಣಾಂಕ (*blob_open)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3_int64,
                   ಪೂರ್ಣಾಂಕ,sqlite3_blob**);
  ಪೂರ್ಣಾಂಕ (*blob_read)(sqlite3_blob*,ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*blob_write)(sqlite3_blob*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*create_collation_v2)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,
                             ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*),
                             ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ (*file_control)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*);
  sqlite3_int64 (*ಸ್ಮರಣೆ_highwater)(ಪೂರ್ಣಾಂಕ);
  sqlite3_int64 (*ಸ್ಮರಣೆ_ಬಳಸಿದ)(ಶೂನ್ಯ);
  sqlite3_mutex *(*mutex_alloc)(ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ (*mutex_enter)(sqlite3_mutex*);
  ಶೂನ್ಯ (*mutex_free)(sqlite3_mutex*);
  ಶೂನ್ಯ (*mutex_leave)(sqlite3_mutex*);
  ಪೂರ್ಣಾಂಕ (*mutex_try)(sqlite3_mutex*);
  ಪೂರ್ಣಾಂಕ (*open_v2)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3**,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*release_ಸ್ಮರಣೆ)(ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_error_nomem)(sqlite3_context*);
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_error_toobig)(sqlite3_context*);
  ಪೂರ್ಣಾಂಕ (*sleep)(ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ (*soft_ರಾಶಿ_ಎಲ್ಲೆ)(ಪೂರ್ಣಾಂಕ);
  sqlite3_vfs *(*vfs_ಕಂಡುಹಿಡಿ)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*vfs_ರಿಜಿಸ್ಟರ್)(sqlite3_vfs*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*vfs_unregister)(sqlite3_vfs*);
  ಪೂರ್ಣಾಂಕ (*xthreadsafe)(ಶೂನ್ಯ);
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_zeroblob)(sqlite3_context*,ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_error_ಸಂಕೇತ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (SQLITE_CDECL*test_control)(ಪೂರ್ಣಾಂಕ, ...);
  ಶೂನ್ಯ (*randomness)(ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  ಪೂರ್ಣಾಂಕ (*extended_ಫಲಿತಾಂಶ_codes)(sqlite3*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*ಎಲ್ಲೆ)(sqlite3*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
  sqlite3_stmt *(*ಮುಂದಿನ_stmt)(sqlite3*,sqlite3_stmt*);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*sql)(sqlite3_stmt*);
  ಪೂರ್ಣಾಂಕ (*ಸ್ಥಿತಿ_2)(ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*backup_ಮುಕ್ತಾಯ)(sqlite3_backup*);
  sqlite3_backup *(*backup_init)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*backup_pagecount)(sqlite3_backup*);
  ಪೂರ್ಣಾಂಕ (*backup_ಉಳಿದ)(sqlite3_backup*);
  ಪೂರ್ಣಾಂಕ (*backup_ಹೆಜ್ಜೆ)(sqlite3_backup*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*compileoption_get)(ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*compileoption_ಬಳಸಿದ)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*create_function_v2)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,
                            ಶೂನ್ಯ (*xFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*xFinal)(sqlite3_context*),
                            ಶೂನ್ಯ(*xDestroy)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ (SQLITE_CDECL*db_config)(sqlite3*,ಪೂರ್ಣಾಂಕ,...);
  sqlite3_mutex *(*db_mutex)(sqlite3*);
  ಪೂರ್ಣಾಂಕ (*db_ಸ್ಥಿತಿ)(sqlite3*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*extended_errcode)(sqlite3*);
  ಶೂನ್ಯ (SQLITE_CDECL*ಲಘುಗಣಕ)(ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,...);
  sqlite3_int64 (*soft_ರಾಶಿ_limit64)(sqlite3_int64);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*sourceid)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಂಕ (*stmt_ಸ್ಥಿತಿ)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*strnicmp)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*unlock_notify)(sqlite3*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ**,ಪೂರ್ಣಾಂಕ),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಂಕ (*wal_autocheckpoint)(sqlite3*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*wal_checkpoint)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ *(*wal_hook)(sqlite3*,ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*,sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ),ಶೂನ್ಯ*);
  ಪೂರ್ಣಾಂಕ (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
  ಪೂರ್ಣಾಂಕ (SQLITE_CDECL*vtab_config)(sqlite3*,ಪೂರ್ಣಾಂಕ op,...);
  ಪೂರ್ಣಾಂಕ (*vtab_on_conflict)(sqlite3*);
  /* Version 3.7.16 and later */
  ಪೂರ್ಣಾಂಕ (*close_v2)(sqlite3*);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*db_filename)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*db_readonly)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*db_release_ಸ್ಮರಣೆ)(sqlite3*);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*errstr)(ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*stmt_busy)(sqlite3_stmt*);
  ಪೂರ್ಣಾಂಕ (*stmt_readonly)(sqlite3_stmt*);
  ಪೂರ್ಣಾಂಕ (*stricmp)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*uri_boolean)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);
  sqlite3_int64 (*uri_int64)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3_int64);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*uri_parameter)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಅಕ್ಷರ *(*xvsnprintf)(ಪೂರ್ಣಾಂಕ,ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,va_list);
  ಪೂರ್ಣಾಂಕ (*wal_checkpoint_v2)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*);
  /* Version 3.8.7 and later */
  ಪೂರ್ಣಾಂಕ (*auto_extension)(ಶೂನ್ಯ(*)(ಶೂನ್ಯ));
  ಪೂರ್ಣಾಂಕ (*bind_blob64)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,sqlite3_uint64,
                     ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ (*bind_text64)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3_uint64,
                      ಶೂನ್ಯ(*)(ಶೂನ್ಯ*),ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ);
  ಪೂರ್ಣಾಂಕ (*cancel_auto_extension)(ಶೂನ್ಯ(*)(ಶೂನ್ಯ));
  ಪೂರ್ಣಾಂಕ (*ತುಂಬು_extension)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಅಕ್ಷರ**);
  ಶೂನ್ಯ *(*malloc64)(sqlite3_uint64);
  sqlite3_uint64 (*msize)(ಶೂನ್ಯ*);
  ಶೂನ್ಯ *(*realloc64)(ಶೂನ್ಯ*,sqlite3_uint64);
  ಶೂನ್ಯ (*reset_auto_extension)(ಶೂನ್ಯ);
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_blob64)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,sqlite3_uint64,
                        ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_text64)(sqlite3_context*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3_uint64,
                         ಶೂನ್ಯ(*)(ಶೂನ್ಯ*), ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ);
  ಪೂರ್ಣಾಂಕ (*strglob)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  /* Version 3.8.11 and later */
  sqlite3_ಮೌಲ್ಯ *(*ಮೌಲ್ಯ_dup)(ಸ್ಥಿರಾಂಕ sqlite3_ಮೌಲ್ಯ*);
  ಶೂನ್ಯ (*ಮೌಲ್ಯ_free)(sqlite3_ಮೌಲ್ಯ*);
  ಪೂರ್ಣಾಂಕ (*ಫಲಿತಾಂಶ_zeroblob64)(sqlite3_context*,sqlite3_uint64);
  ಪೂರ್ಣಾಂಕ (*bind_zeroblob64)(sqlite3_stmt*, ಪೂರ್ಣಾಂಕ, sqlite3_uint64);
  /* Version 3.9.0 and later */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ (*ಮೌಲ್ಯ_subtype)(sqlite3_ಮೌಲ್ಯ*);
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_subtype)(sqlite3_context*,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
  /* Version 3.10.0 and later */
  ಪೂರ್ಣಾಂಕ (*status64)(ಪೂರ್ಣಾಂಕ,sqlite3_int64*,sqlite3_int64*,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*strlike)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*db_cacheflush)(sqlite3*);
  /* Version 3.12.0 and later */
  ಪೂರ್ಣಾಂಕ (*system_errno)(sqlite3*);
  /* Version 3.14.0 and later */
  ಪೂರ್ಣಾಂಕ (*trace_v2)(sqlite3*,ಚಿಹ್ನೆರಹಿತ,ಪೂರ್ಣಾಂಕ(*)(ಚಿಹ್ನೆರಹಿತ,ಶೂನ್ಯ*,ಶೂನ್ಯ*,ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  ಅಕ್ಷರ *(*expanded_sql)(sqlite3_stmt*);
  /* Version 3.18.0 and later */
  ಶೂನ್ಯ (*ಗಣ_ಕೊನೆಯ_ಸೇರಿಸು_rowid)(sqlite3*,sqlite3_int64);
  /* Version 3.20.0 and later */
  ಪೂರ್ಣಾಂಕ (*prepare_v3)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ,
                    sqlite3_stmt**,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**);
  ಪೂರ್ಣಾಂಕ (*prepare16_v3)(sqlite3*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ,
                      sqlite3_stmt**,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ**);
  ಪೂರ್ಣಾಂಕ (*bind_ಸೂಚಕ)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_ಸೂಚಕ)(sqlite3_context*,ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಶೂನ್ಯ *(*ಮೌಲ್ಯ_ಸೂಚಕ)(sqlite3_ಮೌಲ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*vtab_nochange)(sqlite3_context*);
  ಪೂರ್ಣಾಂಕ (*ಮೌಲ್ಯ_nochange)(sqlite3_ಮೌಲ್ಯ*);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*vtab_collation)(sqlite3_ಸೂಚ್ಯಂಕ_info*,ಪೂರ್ಣಾಂಕ);
  /* Version 3.24.0 and later */
  ಪೂರ್ಣಾಂಕ (*keyword_ಎಣಿಕೆ)(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಂಕ (*keyword_name)(ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**,ಪೂರ್ಣಾಂಕ*);
  ಪೂರ್ಣಾಂಕ (*keyword_check)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);
  sqlite3_str *(*str_new)(sqlite3*);
  ಅಕ್ಷರ *(*str_ಮುಕ್ತಾಯ)(sqlite3_str*);
  ಶೂನ್ಯ (SQLITE_CDECL*str_appendf)(sqlite3_str*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...);
  ಶೂನ್ಯ (*str_vappendf)(sqlite3_str*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, va_list);
  ಶೂನ್ಯ (*str_append)(sqlite3_str*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ, ಪೂರ್ಣಾಂಕ N);
  ಶೂನ್ಯ (*str_appendall)(sqlite3_str*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ);
  ಶೂನ್ಯ (*str_appendchar)(sqlite3_str*, ಪೂರ್ಣಾಂಕ N, ಅಕ್ಷರ C);
  ಶೂನ್ಯ (*str_reset)(sqlite3_str*);
  ಪೂರ್ಣಾಂಕ (*str_errcode)(sqlite3_str*);
  ಪೂರ್ಣಾಂಕ (*str_ಉದ್ದಳತೆ)(sqlite3_str*);
  ಅಕ್ಷರ *(*str_ಮೌಲ್ಯ)(sqlite3_str*);
  /* Version 3.25.0 and later */
  ಪೂರ್ಣಾಂಕ (*create_ಕಿಟಕಿ_function)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,
                            ಶೂನ್ಯ (*x_ಹೆಜ್ಜೆ)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ (*xFinal)(sqlite3_context*),
                            ಶೂನ್ಯ (*x_ಮೌಲ್ಯ)(sqlite3_context*),
                            ಶೂನ್ಯ (*xInv)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),
                            ಶೂನ್ಯ(*xDestroy)(ಶೂನ್ಯ*));
  /* Version 3.26.0 and later */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*normalized_sql)(sqlite3_stmt*);
  /* Version 3.28.0 and later */
  ಪೂರ್ಣಾಂಕ (*stmt_isexplain)(sqlite3_stmt*);
  ಪೂರ್ಣಾಂಕ (*ಮೌಲ್ಯ_frombind)(sqlite3_ಮೌಲ್ಯ*);
  /* Version 3.30.0 and later */
  ಪೂರ್ಣಾಂಕ (*drop_modules)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**);
  /* Version 3.31.0 and later */
  sqlite3_int64 (*hard_ರಾಶಿ_limit64)(sqlite3_int64);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*uri_ಕೀಲಿ)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*filename_database)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*filename_journal)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*filename_wal)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  /* Version 3.32.0 and later */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*create_filename)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
                           ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**);
  ಶೂನ್ಯ (*free_filename)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  sqlite3_file *(*database_file_object)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  /* Version 3.34.0 and later */
  ಪೂರ್ಣಾಂಕ (*txn_state)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  /* Version 3.36.1 and later */
  sqlite3_int64 (*changes64)(sqlite3*);
  sqlite3_int64 (*ಒಟ್ಟು_changes64)(sqlite3*);
  /* Version 3.37.0 and later */
  ಪೂರ್ಣಾಂಕ (*autovacuum_ಪುಟಗಳು)(sqlite3*,
     ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ),
     ಶೂನ್ಯ*, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  /* Version 3.38.0 and later */
  ಪೂರ್ಣಾಂಕ (*error_ಸ್ಥಾನಾಂತರ)(sqlite3*);
  ಪೂರ್ಣಾಂಕ (*vtab_rhs_ಮೌಲ್ಯ)(sqlite3_ಸೂಚ್ಯಂಕ_info*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**);
  ಪೂರ್ಣಾಂಕ (*vtab_distinct)(sqlite3_ಸೂಚ್ಯಂಕ_info*);
  ಪೂರ್ಣಾಂಕ (*vtab_ಒಳ)(sqlite3_ಸೂಚ್ಯಂಕ_info*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ (*vtab_ಒಳ_ಮೊದಲ)(sqlite3_ಮೌಲ್ಯ*,sqlite3_ಮೌಲ್ಯ**);
  ಪೂರ್ಣಾಂಕ (*vtab_ಒಳ_ಮುಂದಿನ)(sqlite3_ಮೌಲ್ಯ*,sqlite3_ಮೌಲ್ಯ**);
  /* Version 3.39.0 and later */
  ಪೂರ್ಣಾಂಕ (*deserialize)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*,
                     sqlite3_int64,sqlite3_int64,ಚಿಹ್ನೆರಹಿತ);
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *(*serialize)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *,sqlite3_int64*,
                              ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*db_name)(sqlite3*,ಪೂರ್ಣಾಂಕ);
  /* Version 3.40.0 and later */
  ಪೂರ್ಣಾಂಕ (*ಮೌಲ್ಯ_encoding)(sqlite3_ಮೌಲ್ಯ*);
  /* Version 3.41.0 and later */
  ಪೂರ್ಣಾಂಕ (*is_interrupted)(sqlite3*);
  /* Version 3.43.0 and later */
  ಪೂರ್ಣಾಂಕ (*stmt_explain)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ);
  /* Version 3.44.0 and later */
  ಶೂನ್ಯ *(*get_clientdata)(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*ಗಣ_clientdata)(sqlite3*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಶೂನ್ಯ*, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  /* Version 3.50.0 and later */
  ಪೂರ್ಣಾಂಕ (*setlk_timeout)(sqlite3*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
  /* Version 3.51.0 and later */
  ಪೂರ್ಣಾಂಕ (*ಗಣ_errmsg)(sqlite3*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಪೂರ್ಣಾಂಕ (*db_status64)(sqlite3*,ಪೂರ್ಣಾಂಕ,sqlite3_int64*,sqlite3_int64*,ಪೂರ್ಣಾಂಕ);
  /* Version 3.52.0 and later */
  ಶೂನ್ಯ (*str_truncate)(sqlite3_str*,ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ (*str_free)(sqlite3_str*);
  ಪೂರ್ಣಾಂಕ (*carray_bind)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ (*carray_bind_v2)(sqlite3_stmt*,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
  /* Version 3.54.0 and later */
  sqlite3_int64 (*incomplete)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ (*ಫಲಿತಾಂಶ_str)(sqlite3_context*,sqlite3_str*,ಪೂರ್ಣಾಂಕ);
};

/*
** This is the function signature used for all extension entry points.  It
** is also defined in the file "loadext.c".
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*sqlite3_loadext_ನಮೂದು)(
  sqlite3 *db,                       /* Handle to the database. */
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ,                   /* Used to set error string on failure. */
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pThunk /* Extension API function pointers. */
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
#ಆದರೆ !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_ತುಂಬು_EXTENSION)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_aggregate_ಎಣಿಕೆ        sqlite3_api->aggregate_ಎಣಿಕೆ
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_blob              sqlite3_api->bind_blob
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_double            sqlite3_api->bind_double
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_int               sqlite3_api->bind_int
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_int64             sqlite3_api->bind_int64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_null              sqlite3_api->bind_null
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_parameter_ಎಣಿಕೆ   sqlite3_api->bind_parameter_ಎಣಿಕೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_parameter_ಸೂಚ್ಯಂಕ   sqlite3_api->bind_parameter_ಸೂಚ್ಯಂಕ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_ಪಠ್ಯ              sqlite3_api->bind_ಪಠ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_text16            sqlite3_api->bind_text16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_ಮೌಲ್ಯ             sqlite3_api->bind_ಮೌಲ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_busy_handler           sqlite3_api->busy_handler
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_busy_timeout           sqlite3_api->busy_timeout
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_changes                sqlite3_api->changes
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_close                  sqlite3_api->close
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_collation_needed       sqlite3_api->collation_needed
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_collation_needed16     sqlite3_api->collation_needed16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_blob            sqlite3_api->ಸ್ತಂಭ_blob
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_ಬೈಟುಗಳು           sqlite3_api->ಸ್ತಂಭ_ಬೈಟುಗಳು
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_bytes16         sqlite3_api->ಸ್ತಂಭ_bytes16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_ಎಣಿಕೆ           sqlite3_api->ಸ್ತಂಭ_ಎಣಿಕೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_database_name   sqlite3_api->ಸ್ತಂಭ_database_name
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_database_name16 sqlite3_api->ಸ್ತಂಭ_database_name16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_decltype        sqlite3_api->ಸ್ತಂಭ_decltype
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_decltype16      sqlite3_api->ಸ್ತಂಭ_decltype16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_double          sqlite3_api->ಸ್ತಂಭ_double
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_int             sqlite3_api->ಸ್ತಂಭ_int
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_int64           sqlite3_api->ಸ್ತಂಭ_int64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_name            sqlite3_api->ಸ್ತಂಭ_name
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_name16          sqlite3_api->ಸ್ತಂಭ_name16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_origin_name     sqlite3_api->ಸ್ತಂಭ_origin_name
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_origin_name16   sqlite3_api->ಸ್ತಂಭ_origin_name16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name      sqlite3_api->ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name16    sqlite3_api->ಸ್ತಂಭ_ಕೋಷ್ಟಕ_name16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ            sqlite3_api->ಸ್ತಂಭ_ಪಠ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_text16          sqlite3_api->ಸ್ತಂಭ_text16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_type            sqlite3_api->ಸ್ತಂಭ_type
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ           sqlite3_api->ಸ್ತಂಭ_ಮೌಲ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_commit_hook            sqlite3_api->commit_hook
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_complete               sqlite3_api->complete
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_complete16             sqlite3_api->complete16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_collation       sqlite3_api->create_collation
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_collation16     sqlite3_api->create_collation16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_function        sqlite3_api->create_function
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_function16      sqlite3_api->create_function16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_module          sqlite3_api->create_module
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_module_v2       sqlite3_api->create_module_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ದತ್ತಾಂಶ_ಎಣಿಕೆ             sqlite3_api->ದತ್ತಾಂಶ_ಎಣಿಕೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_handle              sqlite3_api->db_handle
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_declare_vtab           sqlite3_api->declare_vtab
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_errcode                sqlite3_api->errcode
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_errmsg                 sqlite3_api->errmsg
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_errmsg16               sqlite3_api->errmsg16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_exec                   sqlite3_api->exec
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_expired                sqlite3_api->expired
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_finalize               sqlite3_api->finalize
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_free                   sqlite3_api->ಬಿಡುಗಡೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_free_ಕೋಷ್ಟಕ             sqlite3_api->free_ಕೋಷ್ಟಕ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_get_autocommit         sqlite3_api->get_autocommit
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_get_auxdata            sqlite3_api->get_auxdata
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_get_ಕೋಷ್ಟಕ              sqlite3_api->get_ಕೋಷ್ಟಕ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_global_recover         sqlite3_api->global_recover
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಅಡಚಣೆ              sqlite3_api->interruptx
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಕೊನೆಯ_ಸೇರಿಸು_rowid      sqlite3_api->ಕೊನೆಯ_ಸೇರಿಸು_rowid
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_libversion             sqlite3_api->libversion
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_libversion_ಸಂಖ್ಯೆ      sqlite3_api->libversion_ಸಂಖ್ಯೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_malloc                 sqlite3_api->ಸ್ಮರಣೆ_ಕೊಡು
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mprintf                sqlite3_api->mprintf
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_open                   sqlite3_api->open
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_open16                 sqlite3_api->open16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_prepare                sqlite3_api->prepare
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_prepare16              sqlite3_api->prepare16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_prepare_v2             sqlite3_api->prepare_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_profile                sqlite3_api->profile
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಪ್ರಗತಿ_handler       sqlite3_api->ಪ್ರಗತಿ_handler
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_realloc                sqlite3_api->realloc
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_reset                  sqlite3_api->reset
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_blob            sqlite3_api->ಫಲಿತಾಂಶ_blob
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_double          sqlite3_api->ಫಲಿತಾಂಶ_double
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_error           sqlite3_api->ಫಲಿತಾಂಶ_error
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_error16         sqlite3_api->ಫಲಿತಾಂಶ_error16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_int             sqlite3_api->ಫಲಿತಾಂಶ_int
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_int64           sqlite3_api->ಫಲಿತಾಂಶ_int64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_null            sqlite3_api->ಫಲಿತಾಂಶ_null
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ            sqlite3_api->ಫಲಿತಾಂಶ_ಪಠ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_text16          sqlite3_api->ಫಲಿತಾಂಶ_text16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_text16be        sqlite3_api->ಫಲಿತಾಂಶ_text16be
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_text16le        sqlite3_api->ಫಲಿತಾಂಶ_text16le
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ           sqlite3_api->ಫಲಿತಾಂಶ_ಮೌಲ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_rollback_hook          sqlite3_api->rollback_hook
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಗಣ_authorizer         sqlite3_api->ಗಣ_authorizer
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಗಣ_auxdata            sqlite3_api->ಗಣ_auxdata
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_snprintf               sqlite3_api->xsnprintf
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಹೆಜ್ಜೆ                   sqlite3_api->ಹೆಜ್ಜೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_metadata  sqlite3_api->ಕೋಷ್ಟಕ_ಸ್ತಂಭ_metadata
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಒಟ್ಟು_changes          sqlite3_api->ಒಟ್ಟು_changes
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_trace                  sqlite3_api->trace
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_update_hook            sqlite3_api->update_hook
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_user_ದತ್ತಾಂಶ              sqlite3_api->user_ದತ್ತಾಂಶ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_blob             sqlite3_api->ಮೌಲ್ಯ_blob
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು            sqlite3_api->ಮೌಲ್ಯ_ಬೈಟುಗಳು
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_bytes16          sqlite3_api->ಮೌಲ್ಯ_bytes16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_double           sqlite3_api->ಮೌಲ್ಯ_double
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_int              sqlite3_api->ಮೌಲ್ಯ_int
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_int64            sqlite3_api->ಮೌಲ್ಯ_int64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_numeric_type     sqlite3_api->ಮೌಲ್ಯ_numeric_type
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ             sqlite3_api->ಮೌಲ್ಯ_ಪಠ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_text16           sqlite3_api->ಮೌಲ್ಯ_text16
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_text16be         sqlite3_api->ಮೌಲ್ಯ_text16be
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_text16le         sqlite3_api->ಮೌಲ್ಯ_text16le
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_type             sqlite3_api->ಮೌಲ್ಯ_type
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vmprintf               sqlite3_api->vmprintf
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vsnprintf              sqlite3_api->xvsnprintf
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_overload_function      sqlite3_api->overload_function
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_prepare_v2             sqlite3_api->prepare_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_clear_bindings         sqlite3_api->clear_bindings
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_ಬೈಟುಗಳು             sqlite3_api->blob_ಬೈಟುಗಳು
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_close             sqlite3_api->blob_close
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_open              sqlite3_api->blob_open
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_read              sqlite3_api->blob_read
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_write             sqlite3_api->blob_write
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_file_control           sqlite3_api->file_control
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ಮರಣೆ_highwater       sqlite3_api->ಸ್ಮರಣೆ_highwater
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ            sqlite3_api->ಸ್ಮರಣೆ_ಬಳಸಿದ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_enter            sqlite3_api->mutex_enter
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_free             sqlite3_api->mutex_free
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_leave            sqlite3_api->mutex_leave
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_try              sqlite3_api->mutex_try
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_open_v2                sqlite3_api->open_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_release_ಸ್ಮರಣೆ         sqlite3_api->release_ಸ್ಮರಣೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_error_nomem     sqlite3_api->ಫಲಿತಾಂಶ_error_nomem
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_error_toobig    sqlite3_api->ಫಲಿತಾಂಶ_error_toobig
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_sleep                  sqlite3_api->sleep
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_soft_ರಾಶಿ_ಎಲ್ಲೆ        sqlite3_api->soft_ರಾಶಿ_ಎಲ್ಲೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vfs_ಕಂಡುಹಿಡಿ               sqlite3_api->vfs_ಕಂಡುಹಿಡಿ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vfs_ರಿಜಿಸ್ಟರ್           sqlite3_api->vfs_ರಿಜಿಸ್ಟರ್
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_threadsafe             sqlite3_api->xthreadsafe
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_zeroblob        sqlite3_api->ಫಲಿತಾಂಶ_zeroblob
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_error_ಸಂಕೇತ      sqlite3_api->ಫಲಿತಾಂಶ_error_ಸಂಕೇತ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_test_control           sqlite3_api->test_control
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_randomness             sqlite3_api->randomness
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_context_db_handle      sqlite3_api->context_db_handle
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_extended_ಫಲಿತಾಂಶ_codes  sqlite3_api->extended_ಫಲಿತಾಂಶ_codes
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಎಲ್ಲೆ                  sqlite3_api->ಎಲ್ಲೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮುಂದಿನ_stmt              sqlite3_api->ಮುಂದಿನ_stmt
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_sql                    sqlite3_api->sql
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಸ್ಥಿತಿ                 sqlite3_api->ಸ್ಥಿತಿ_2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_backup_ಮುಕ್ತಾಯ          sqlite3_api->backup_ಮುಕ್ತಾಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_backup_init            sqlite3_api->backup_init
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_backup_pagecount       sqlite3_api->backup_pagecount
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_backup_ಉಳಿದ       sqlite3_api->backup_ಉಳಿದ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_backup_ಹೆಜ್ಜೆ            sqlite3_api->backup_ಹೆಜ್ಜೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_compileoption_get      sqlite3_api->compileoption_get
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_compileoption_ಬಳಸಿದ     sqlite3_api->compileoption_ಬಳಸಿದ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_function_v2     sqlite3_api->create_function_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_config              sqlite3_api->db_config
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_mutex               sqlite3_api->db_mutex
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_ಸ್ಥಿತಿ              sqlite3_api->db_ಸ್ಥಿತಿ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_extended_errcode       sqlite3_api->extended_errcode
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_log                    sqlite3_api->ಲಘುಗಣಕ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_soft_ರಾಶಿ_limit64      sqlite3_api->soft_ರಾಶಿ_limit64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_sourceid               sqlite3_api->sourceid
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_stmt_ಸ್ಥಿತಿ            sqlite3_api->stmt_ಸ್ಥಿತಿ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_strnicmp               sqlite3_api->strnicmp
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_unlock_notify          sqlite3_api->unlock_notify
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_wal_autocheckpoint     sqlite3_api->wal_autocheckpoint
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_wal_checkpoint         sqlite3_api->wal_checkpoint
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_wal_hook               sqlite3_api->wal_hook
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_blob_reopen            sqlite3_api->blob_reopen
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_config            sqlite3_api->vtab_config
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_on_conflict       sqlite3_api->vtab_on_conflict
/* Version 3.7.16 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_close_v2               sqlite3_api->close_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_filename            sqlite3_api->db_filename
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_readonly            sqlite3_api->db_readonly
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_release_ಸ್ಮರಣೆ      sqlite3_api->db_release_ಸ್ಮರಣೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_errstr                 sqlite3_api->errstr
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_stmt_busy              sqlite3_api->stmt_busy
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_stmt_readonly          sqlite3_api->stmt_readonly
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_stricmp                sqlite3_api->stricmp
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_uri_boolean            sqlite3_api->uri_boolean
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_uri_int64              sqlite3_api->uri_int64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_uri_parameter          sqlite3_api->uri_parameter
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_uri_vsnprintf          sqlite3_api->xvsnprintf
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_wal_checkpoint_v2      sqlite3_api->wal_checkpoint_v2
/* Version 3.8.7 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_auto_extension         sqlite3_api->auto_extension
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_blob64            sqlite3_api->bind_blob64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_text64            sqlite3_api->bind_text64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_cancel_auto_extension  sqlite3_api->cancel_auto_extension
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ತುಂಬು_extension         sqlite3_api->ತುಂಬು_extension
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_malloc64               sqlite3_api->malloc64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_msize                  sqlite3_api->msize
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_realloc64              sqlite3_api->realloc64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_reset_auto_extension   sqlite3_api->reset_auto_extension
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_blob64          sqlite3_api->ಫಲಿತಾಂಶ_blob64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_text64          sqlite3_api->ಫಲಿತಾಂಶ_text64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_strglob                sqlite3_api->strglob
/* Version 3.8.11 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_dup              sqlite3_api->ಮೌಲ್ಯ_dup
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_free             sqlite3_api->ಮೌಲ್ಯ_free
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_zeroblob64      sqlite3_api->ಫಲಿತಾಂಶ_zeroblob64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_zeroblob64        sqlite3_api->bind_zeroblob64
/* Version 3.9.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_subtype          sqlite3_api->ಮೌಲ್ಯ_subtype
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_subtype         sqlite3_api->ಫಲಿತಾಂಶ_subtype
/* Version 3.10.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_status64               sqlite3_api->status64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_strlike                sqlite3_api->strlike
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_cacheflush          sqlite3_api->db_cacheflush
/* Version 3.12.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_system_errno           sqlite3_api->system_errno
/* Version 3.14.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_trace_v2               sqlite3_api->trace_v2
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_expanded_sql           sqlite3_api->expanded_sql
/* Version 3.18.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಗಣ_ಕೊನೆಯ_ಸೇರಿಸು_rowid  sqlite3_api->ಗಣ_ಕೊನೆಯ_ಸೇರಿಸು_rowid
/* Version 3.20.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_prepare_v3             sqlite3_api->prepare_v3
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_prepare16_v3           sqlite3_api->prepare16_v3
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_bind_ಸೂಚಕ           sqlite3_api->bind_ಸೂಚಕ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_ಸೂಚಕ         sqlite3_api->ಫಲಿತಾಂಶ_ಸೂಚಕ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_ಸೂಚಕ          sqlite3_api->ಮೌಲ್ಯ_ಸೂಚಕ
/* Version 3.22.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_nochange          sqlite3_api->vtab_nochange
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_nochange         sqlite3_api->ಮೌಲ್ಯ_nochange
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_collation         sqlite3_api->vtab_collation
/* Version 3.24.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_keyword_ಎಣಿಕೆ          sqlite3_api->keyword_ಎಣಿಕೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_keyword_name           sqlite3_api->keyword_name
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_keyword_check          sqlite3_api->keyword_check
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_new                sqlite3_api->str_new
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_ಮುಕ್ತಾಯ             sqlite3_api->str_ಮುಕ್ತಾಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_appendf            sqlite3_api->str_appendf
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_vappendf           sqlite3_api->str_vappendf
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_append             sqlite3_api->str_append
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_appendall          sqlite3_api->str_appendall
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_appendchar         sqlite3_api->str_appendchar
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_reset              sqlite3_api->str_reset
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_errcode            sqlite3_api->str_errcode
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_ಉದ್ದಳತೆ             sqlite3_api->str_ಉದ್ದಳತೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_ಮೌಲ್ಯ              sqlite3_api->str_ಮೌಲ್ಯ
/* Version 3.25.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_ಕಿಟಕಿ_function sqlite3_api->create_ಕಿಟಕಿ_function
/* Version 3.26.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_normalized_sql         sqlite3_api->normalized_sql
/* Version 3.28.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_stmt_isexplain         sqlite3_api->stmt_isexplain
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_frombind         sqlite3_api->ಮೌಲ್ಯ_frombind
/* Version 3.30.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_drop_modules           sqlite3_api->drop_modules
/* Version 3.31.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_hard_ರಾಶಿ_limit64      sqlite3_api->hard_ರಾಶಿ_limit64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_uri_ಕೀಲಿ                sqlite3_api->uri_ಕೀಲಿ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_filename_database      sqlite3_api->filename_database
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_filename_journal       sqlite3_api->filename_journal
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_filename_wal           sqlite3_api->filename_wal
/* Version 3.32.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_create_filename        sqlite3_api->create_filename
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_free_filename          sqlite3_api->free_filename
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_database_file_object   sqlite3_api->database_file_object
/* Version 3.34.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_txn_state              sqlite3_api->txn_state
/* Version 3.36.1 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_changes64              sqlite3_api->changes64
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಒಟ್ಟು_changes64        sqlite3_api->ಒಟ್ಟು_changes64
/* Version 3.37.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_autovacuum_ಪುಟಗಳು       sqlite3_api->autovacuum_ಪುಟಗಳು
/* Version 3.38.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_error_ಸ್ಥಾನಾಂತರ           sqlite3_api->error_ಸ್ಥಾನಾಂತರ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_rhs_ಮೌಲ್ಯ         sqlite3_api->vtab_rhs_ಮೌಲ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_distinct          sqlite3_api->vtab_distinct
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_ಒಳ                sqlite3_api->vtab_ಒಳ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_ಒಳ_ಮೊದಲ          sqlite3_api->vtab_ಒಳ_ಮೊದಲ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_vtab_ಒಳ_ಮುಂದಿನ           sqlite3_api->vtab_ಒಳ_ಮುಂದಿನ
/* Version 3.39.0 and later */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DESERIALIZE
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_deserialize            sqlite3_api->deserialize
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_serialize              sqlite3_api->serialize
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_name                sqlite3_api->db_name
/* Version 3.40.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಮೌಲ್ಯ_encoding         sqlite3_api->ಮೌಲ್ಯ_encoding
/* Version 3.41.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_is_interrupted         sqlite3_api->is_interrupted
/* Version 3.43.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_stmt_explain           sqlite3_api->stmt_explain
/* Version 3.44.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_get_clientdata         sqlite3_api->get_clientdata
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಗಣ_clientdata         sqlite3_api->ಗಣ_clientdata
/* Version 3.50.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_setlk_timeout          sqlite3_api->setlk_timeout
/* Version 3.51.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಗಣ_errmsg             sqlite3_api->ಗಣ_errmsg
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_db_status64            sqlite3_api->db_status64
/* Version 3.52.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_truncate           sqlite3_api->str_truncate
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_str_free               sqlite3_api->str_free
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_carray_bind            sqlite3_api->carray_bind
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_carray_bind_v2         sqlite3_api->carray_bind_v2
/* Version 3.54.0 and later */
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_incomplete             sqlite3_api->incomplete
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಫಲಿತಾಂಶ_str             sqlite3_api->ಫಲಿತಾಂಶ_str
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_LOAD_EXTENSION) */

#ಆದರೆ !defined(SQLITE_CORE) && !defined(SQLITE_OMIT_ತುಂಬು_EXTENSION)
  /* This case when the file really is being compiled as a loadable 
  ** extension */
# define SQLITE_EXTENSION_INIT1     ಸ್ಥಿರಾಂಕ sqlite3_api_routines *sqlite3_api=0;
# define SQLITE_EXTENSION_INIT2(v)  sqlite3_api=v;
# define SQLITE_EXTENSION_INIT3     \
    ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ sqlite3_api_routines *sqlite3_api;
#ಇಲ್ಲದಿದ್ದರೆ
  /* This case when the file is being statically linked into the 
  ** application */
# define SQLITE_EXTENSION_INIT1     /*no-op*/
# define SQLITE_EXTENSION_INIT2(v)  (ಶೂನ್ಯ)v; /* unused parameter */
# define SQLITE_EXTENSION_INIT3     /*no-op*/
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE3EXT_H */
