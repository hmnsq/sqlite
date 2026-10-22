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
ಪೂರ್ಣಾಙ್ಕ sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  ಧ್ರುವ ವರ್ಣ *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  ಶೂನ್ಯ *pArg,                 /* First argument to xCallback() */
  ವರ್ಣ **pz_Err_ಸಂದೇಶ             /* Write error messages here */
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;         /* Return code */
  ಧ್ರುವ ವರ್ಣ *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  ವರ್ಣ **azCols = 0;          /* Names of result columns */
  ಪೂರ್ಣಾಙ್ಕ callbackIsInit;         /* True if callback data is initialized */

  ಯದಿ( !sqlite3SafetyCheckOk(db) ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  ಯದಿ( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  ಯಾವತ್( rc==SQLITE_OK && zSql[0] ){
    ಪೂರ್ಣಾಙ್ಕ nCol = 0;
    ವರ್ಣ **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || pStmt==0 );
    ಯದಿ( rc!=SQLITE_OK ){
      ಅನುವರ್ತ;
    }
    ಯದಿ( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      ಅನುವರ್ತ;
    }
    callbackIsInit = 0;

    ಯಾವತ್( 1 ){
      ಪೂರ್ಣಾಙ್ಕ i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      ಯದಿ( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        ಯದಿ( !callbackIsInit ){
          nCol = sqlite3_column_ಎಣಿಕೆ(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*ಪರಿಮಾಣ(ಧ್ರುವ ವರ್ಣ*));
          ಯದಿ( azCols==0 ){
            ಗಚ್ಛ exec_out;
          }
          ಪ್ರತಿ(i=0; i<nCol; i++){
            azCols[i] = (ವರ್ಣ *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            ಪ್ರತಿಜ್ಞಾ( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        ಯದಿ( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          ಪ್ರತಿ(i=0; i<nCol; i++){
            azVals[i] = (ವರ್ಣ *)sqlite3_column_text(pStmt, i);
            ಯದಿ( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              ಗಚ್ಛ exec_out;
            }
          }
          azVals[i] = 0;
        }
        ಯದಿ( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          ಗಚ್ಛ exec_out;
        }
      }

      ಯದಿ( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        ಯಾವತ್( sqlite3Isspace(zSql[0]) ) zSql++;
        ಭಙ್ಗ;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  ಯದಿ( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  ಯದಿ( rc!=SQLITE_OK && pz_Err_ಸಂದೇಶ ){
    *pz_Err_ಸಂದೇಶ = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    ಯದಿ( *pz_Err_ಸಂದೇಶ==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }ಅನ್ಯಥಾ ಯದಿ( pz_Err_ಸಂದೇಶ ){
    *pz_Err_ಸಂದೇಶ = 0;
  }

  ಪ್ರತಿಜ್ಞಾ( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತ್ಯಾಗಮ rc;
}
