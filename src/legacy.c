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

#अन्तर्भाव "sqliteInt.h"

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
पूर्णांक sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  स्थिरांक अक्षर *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  शून्य *pArg,                 /* First argument to xCallback() */
  अक्षर **pz_Err_संदेश             /* Write error messages here */
){
  पूर्णांक rc = SQLITE_OK;         /* Return code */
  स्थिरांक अक्षर *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  अक्षर **azCols = 0;          /* Names of result columns */
  पूर्णांक callbackIsInit;         /* True if callback data is initialized */

  जर( !sqlite3SafetyCheckOk(db) ) परत SQLITE_MISUSE_BKPT;
  जर( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  पर्यंत( rc==SQLITE_OK && zSql[0] ){
    पूर्णांक nCol = 0;
    अक्षर **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    प्रतिज्ञा( rc==SQLITE_OK || pStmt==0 );
    जर( rc!=SQLITE_OK ){
      सुरूठेवा;
    }
    जर( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      सुरूठेवा;
    }
    callbackIsInit = 0;

    पर्यंत( 1 ){
      पूर्णांक i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      जर( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        जर( !callbackIsInit ){
          nCol = sqlite3_column_मोजणी(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*आकार(स्थिरांक अक्षर*));
          जर( azCols==0 ){
            जा exec_out;
          }
          साठी(i=0; i<nCol; i++){
            azCols[i] = (अक्षर *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            प्रतिज्ञा( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        जर( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          साठी(i=0; i<nCol; i++){
            azVals[i] = (अक्षर *)sqlite3_column_text(pStmt, i);
            जर( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              जा exec_out;
            }
          }
          azVals[i] = 0;
        }
        जर( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          जा exec_out;
        }
      }

      जर( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        पर्यंत( sqlite3Isspace(zSql[0]) ) zSql++;
        थांबा;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  जर( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  जर( rc!=SQLITE_OK && pz_Err_संदेश ){
    *pz_Err_संदेश = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    जर( *pz_Err_संदेश==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }अन्यथा जर( pz_Err_संदेश ){
    *pz_Err_संदेश = 0;
  }

  प्रतिज्ञा( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  परत rc;
}
