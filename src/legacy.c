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

#شامل "sqliteInt.h"

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
صحیح sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  ثابت نویسه *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  تهی *pArg,                 /* First argument to xCallback() */
  نویسه **pz_Err_پیام             /* Write error messages here */
){
  صحیح rc = SQLITE_OK;         /* Return code */
  ثابت نویسه *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  نویسه **azCols = 0;          /* Names of result columns */
  صحیح callbackIsInit;         /* True if callback data is initialized */

  اگر( !sqlite3SafetyCheckOk(db) ) بازگشت SQLITE_MISUSE_BKPT;
  اگر( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  وقتی( rc==SQLITE_OK && zSql[0] ){
    صحیح nCol = 0;
    نویسه **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    أكد( rc==SQLITE_OK || pStmt==0 );
    اگر( rc!=SQLITE_OK ){
      ادامه;
    }
    اگر( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      ادامه;
    }
    callbackIsInit = 0;

    وقتی( 1 ){
      صحیح i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      اگر( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        اگر( !callbackIsInit ){
          nCol = sqlite3_column_شمار(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*اندازه(ثابت نویسه*));
          اگر( azCols==0 ){
            برو exec_out;
          }
          برای(i=0; i<nCol; i++){
            azCols[i] = (نویسه *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            أكد( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        اگر( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          برای(i=0; i<nCol; i++){
            azVals[i] = (نویسه *)sqlite3_column_text(pStmt, i);
            اگر( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              برو exec_out;
            }
          }
          azVals[i] = 0;
        }
        اگر( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          برو exec_out;
        }
      }

      اگر( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        وقتی( sqlite3Isspace(zSql[0]) ) zSql++;
        بشکن;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  اگر( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  اگر( rc!=SQLITE_OK && pz_Err_پیام ){
    *pz_Err_پیام = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    اگر( *pz_Err_پیام==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }وگرنه اگر( pz_Err_پیام ){
    *pz_Err_پیام = 0;
  }

  أكد( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  بازگشت rc;
}
