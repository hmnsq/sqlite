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

#ضمن "sqliteInt.h"

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
صحيح sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  ثابت حرف *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  فراغ *pArg,                 /* First argument to xCallback() */
  حرف **pz_Err_رسالة             /* Write error messages here */
){
  صحيح rc = SQLITE_OK;         /* Return code */
  ثابت حرف *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  حرف **azCols = 0;          /* Names of result columns */
  صحيح callbackIsInit;         /* True if callback data is initialized */

  إذا( !sqlite3SafetyCheckOk(db) ) إرجاع SQLITE_MISUSE_BKPT;
  إذا( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  بينما( rc==SQLITE_OK && zSql[0] ){
    صحيح nCol = 0;
    حرف **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    أكد( rc==SQLITE_OK || pStmt==0 );
    إذا( rc!=SQLITE_OK ){
      استمر;
    }
    إذا( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      استمر;
    }
    callbackIsInit = 0;

    بينما( 1 ){
      صحيح i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      إذا( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        إذا( !callbackIsInit ){
          nCol = sqlite3_column_تعداد(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*حجم(ثابت حرف*));
          إذا( azCols==0 ){
            اذهب exec_out;
          }
          لكل(i=0; i<nCol; i++){
            azCols[i] = (حرف *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            أكد( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        إذا( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          لكل(i=0; i<nCol; i++){
            azVals[i] = (حرف *)sqlite3_column_text(pStmt, i);
            إذا( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              اذهب exec_out;
            }
          }
          azVals[i] = 0;
        }
        إذا( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          اذهب exec_out;
        }
      }

      إذا( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        بينما( sqlite3Isspace(zSql[0]) ) zSql++;
        اكسر;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  إذا( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  إذا( rc!=SQLITE_OK && pz_Err_رسالة ){
    *pz_Err_رسالة = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    إذا( *pz_Err_رسالة==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }وإلا إذا( pz_Err_رسالة ){
    *pz_Err_رسالة = 0;
  }

  أكد( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  إرجاع rc;
}
