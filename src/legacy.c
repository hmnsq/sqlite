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

#অন্তর্ভাব "sqliteInt.h"

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
পূর্ণসংখ্যা sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  ধ্রুবক অক্ষর *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  শূন্য *pArg,                 /* First argument to xCallback() */
  অক্ষর **pz_Err_বার্তা             /* Write error messages here */
){
  পূর্ণসংখ্যা rc = SQLITE_OK;         /* Return code */
  ধ্রুবক অক্ষর *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  অক্ষর **azCols = 0;          /* Names of result columns */
  পূর্ণসংখ্যা callbackIsInit;         /* True if callback data is initialized */

  যদি( !sqlite3SafetyCheckOk(db) ) ফেরত SQLITE_MISUSE_BKPT;
  যদি( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  যখন( rc==SQLITE_OK && zSql[0] ){
    পূর্ণসংখ্যা nCol = 0;
    অক্ষর **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    প্রতিজ্ঞা( rc==SQLITE_OK || pStmt==0 );
    যদি( rc!=SQLITE_OK ){
      চালু;
    }
    যদি( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      চালু;
    }
    callbackIsInit = 0;

    যখন( 1 ){
      পূর্ণসংখ্যা i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      যদি( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        যদি( !callbackIsInit ){
          nCol = sqlite3_column_গুনতি(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*আকার(ধ্রুবক অক্ষর*));
          যদি( azCols==0 ){
            যাও exec_out;
          }
          জন্য(i=0; i<nCol; i++){
            azCols[i] = (অক্ষর *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            প্রতিজ্ঞা( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        যদি( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          জন্য(i=0; i<nCol; i++){
            azVals[i] = (অক্ষর *)sqlite3_column_text(pStmt, i);
            যদি( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              যাও exec_out;
            }
          }
          azVals[i] = 0;
        }
        যদি( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          যাও exec_out;
        }
      }

      যদি( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        যখন( sqlite3Isspace(zSql[0]) ) zSql++;
        বিরতি;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  যদি( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  যদি( rc!=SQLITE_OK && pz_Err_বার্তা ){
    *pz_Err_বার্তা = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    যদি( *pz_Err_বার্তা==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }অন্যথা যদি( pz_Err_বার্তা ){
    *pz_Err_বার্তা = 0;
  }

  প্রতিজ্ঞা( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  ফেরত rc;
}
