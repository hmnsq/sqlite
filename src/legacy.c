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

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  ଅଚଳ ଅକ୍ଷର *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  ଶୂନ୍ୟ *pArg,                 /* First argument to xCallback() */
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ             /* Write error messages here */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;         /* Return code */
  ଅଚଳ ଅକ୍ଷର *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  ଅକ୍ଷର **azCols = 0;          /* Names of result columns */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା callbackIsInit;         /* True if callback data is initialized */

  ଯଦି( !sqlite3SafetyCheckOk(db) ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  ଯଦି( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  ଯେତେବେଳେ( rc==SQLITE_OK && zSql[0] ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol = 0;
    ଅକ୍ଷର **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    ପ୍ରତିଜ୍ଞା( rc==SQLITE_OK || pStmt==0 );
    ଯଦି( rc!=SQLITE_OK ){
      ଜାରି;
    }
    ଯଦି( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      ଜାରି;
    }
    callbackIsInit = 0;

    ଯେତେବେଳେ( 1 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      ଯଦି( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        ଯଦି( !callbackIsInit ){
          nCol = sqlite3_column_ଗଣନା(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*ଆକାର(ଅଚଳ ଅକ୍ଷର*));
          ଯଦି( azCols==0 ){
            ଯାଅ exec_out;
          }
          ପାଇଁ(i=0; i<nCol; i++){
            azCols[i] = (ଅକ୍ଷର *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            ପ୍ରତିଜ୍ଞା( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        ଯଦି( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          ପାଇଁ(i=0; i<nCol; i++){
            azVals[i] = (ଅକ୍ଷର *)sqlite3_column_text(pStmt, i);
            ଯଦି( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              ଯାଅ exec_out;
            }
          }
          azVals[i] = 0;
        }
        ଯଦି( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          ଯାଅ exec_out;
        }
      }

      ଯଦି( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        ଯେତେବେଳେ( sqlite3Isspace(zSql[0]) ) zSql++;
        ଭାଙ୍ଗ;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  ଯଦି( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  ଯଦି( rc!=SQLITE_OK && pz_Err_ସନ୍ଦେଶ ){
    *pz_Err_ସନ୍ଦେଶ = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    ଯଦି( *pz_Err_ସନ୍ଦେଶ==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }ଅନ୍ୟଥା ଯଦି( pz_Err_ସନ୍ଦେଶ ){
    *pz_Err_ସନ୍ଦେଶ = 0;
  }

  ପ୍ରତିଜ୍ଞା( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  ଫେରନ୍ତୁ rc;
}
