/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Main file for the SQLite library.  The routines in this file
** implement the programmer interface to the library.  Routines in
** other files are for internal use by SQLite and should not be
** accessed by users of the library.
*/

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** Execute SQL code.  Return one of the SQLITE_ success/failure
** codes.  Also write an error message into memory obtained from
** malloc() and make *pzErrMsg point to that message.
**
** If the SQL is a query, then for each row in the query result
** the xCallback() function is called.  pArg becomes the first
** argument to xCallback().  If xCallback=NULL then no callback
** is invoked, even for queries.
*/
ಪೂರ್ಣಾಂಕ sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  ಶೂನ್ಯ *pArg,                 /* First argument to xCallback() */
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ             /* Write error messages here */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;         /* Return code */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  ಅಕ್ಷರ **az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = 0;          /* Names of result columns */
  ಪೂರ್ಣಾಂಕ callbackIsInit;         /* True if callback data is initialized */

  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  ಆದರೆ( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  ಆಗ( rc==SQLITE_OK && zSql[0] ){
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = 0;
    ಅಕ್ಷರ **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || pStmt==0 );
    ಆದರೆ( rc!=SQLITE_OK ){
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      ಮುಂದುವರಿ;
    }
    callbackIsInit = 0;

    ಆಗ( 1 ){
      ಪೂರ್ಣಾಂಕ i;
      rc = sqlite3_ಹೆಜ್ಜೆ(pStmt);

      /* Invoke the callback function if required */
      ಆದರೆ( xCallback && (SQLITE_ಅಡ್ಡಸಾಲು==rc || 
          (SQLITE_ಮುಗಿದ==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        ಆದರೆ( !callbackIsInit ){
          n_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಎಣಿಕೆ(pStmt);
          az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = sqlite3DbMallocRaw(db, (2*n_ಲಂಬಸಾಲು+1)*ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*));
          ಆದರೆ( az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ==0 ){
            ಹೋಗು exec_ಹೊರಹೋಗು;
          }
          ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
            az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ[i] = (ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            ಪ್ರತಿಜ್ಞಾ( az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ[i]!=0 );
          }
          callbackIsInit = 1;
        }
        ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
          azVals = &az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ[n_ಲಂಬಸಾಲು];
          ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
            azVals[i] = (ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, i);
            ಆದರೆ( !azVals[i] && sqlite3_ಸ್ತಂಭ_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              ಹೋಗು exec_ಹೊರಹೋಗು;
            }
          }
          azVals[i] = 0;
        }
        ಆದರೆ( xCallback(pArg, n_ಲಂಬಸಾಲು, azVals, az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          ಹೋಗು exec_ಹೊರಹೋಗು;
        }
      }

      ಆದರೆ( rc!=SQLITE_ಅಡ್ಡಸಾಲು ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        ಆಗ( sqlite3Isspace(zSql[0]) ) zSql++;
        ನಿಲ್ಲಿಸು;
      }
    }

    sqlite3DbFree(db, az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
    az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = 0;
  }

exec_ಹೊರಹೋಗು:
  ಆದರೆ( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, az_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);

  rc = sqlite3ApiExit(db, rc);
  ಆದರೆ( rc!=SQLITE_OK && pz_Err_ಸಂದೇಶ ){
    *pz_Err_ಸಂದೇಶ = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    ಆದರೆ( *pz_Err_ಸಂದೇಶ==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pz_Err_ಸಂದೇಶ ){
    *pz_Err_ಸಂದೇಶ = 0;
  }

  ಪ್ರತಿಜ್ಞಾ( (rc&db->err_ಮುಖವಾಡ)==rc );
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}
