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

#அந்தர்பாவ "sqliteInt.h"

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
முழு sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  மாறிலி எழுத்து *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  வெறுமை *pArg,                 /* First argument to xCallback() */
  எழுத்து **pz_Err_செய்தி             /* Write error messages here */
){
  முழு rc = SQLITE_OK;         /* Return code */
  மாறிலி எழுத்து *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  எழுத்து **azCols = 0;          /* Names of result columns */
  முழு callbackIsInit;         /* True if callback data is initialized */

  எனில்( !sqlite3SafetyCheckOk(db) ) திரும்பு SQLITE_MISUSE_BKPT;
  எனில்( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  போது( rc==SQLITE_OK && zSql[0] ){
    முழு nCol = 0;
    எழுத்து **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    ப்ரதிஜ்ஞா( rc==SQLITE_OK || pStmt==0 );
    எனில்( rc!=SQLITE_OK ){
      தொடர்;
    }
    எனில்( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      தொடர்;
    }
    callbackIsInit = 0;

    போது( 1 ){
      முழு i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      எனில்( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        எனில்( !callbackIsInit ){
          nCol = sqlite3_column_மொத்தம்(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*அளவு(மாறிலி எழுத்து*));
          எனில்( azCols==0 ){
            செல் exec_out;
          }
          க்கு(i=0; i<nCol; i++){
            azCols[i] = (எழுத்து *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            ப்ரதிஜ்ஞா( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        எனில்( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          க்கு(i=0; i<nCol; i++){
            azVals[i] = (எழுத்து *)sqlite3_column_text(pStmt, i);
            எனில்( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              செல் exec_out;
            }
          }
          azVals[i] = 0;
        }
        எனில்( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          செல் exec_out;
        }
      }

      எனில்( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        போது( sqlite3Isspace(zSql[0]) ) zSql++;
        நிறுத்து;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  எனில்( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  எனில்( rc!=SQLITE_OK && pz_Err_செய்தி ){
    *pz_Err_செய்தி = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    எனில்( *pz_Err_செய்தி==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }இல்லையெனில் எனில்( pz_Err_செய்தி ){
    *pz_Err_செய்தி = 0;
  }

  ப்ரதிஜ்ஞா( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}
