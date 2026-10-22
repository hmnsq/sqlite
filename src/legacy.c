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

#include "sqliteInt.h"

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
số_nguyên sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  const char *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  void *pArg,                 /* First argument to xCallback() */
  char **pzErrMsg             /* Write error messages here */
){
  số_nguyên rc = SQLITE_OK;         /* Return code */
  const char *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  char **azCols = 0;          /* Names of result columns */
  số_nguyên callbackIsInit;         /* True if callback data is initialized */

  nếu( !sqlite3SafetyCheckOk(db) ) trả_về SQLITE_MISUSE_BKPT;
  nếu( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  trong_khi( rc==SQLITE_OK && zSql[0] ){
    số_nguyên nCol = 0;
    char **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    assert( rc==SQLITE_OK || pStmt==0 );
    nếu( rc!=SQLITE_OK ){
      continue;
    }
    nếu( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      continue;
    }
    callbackIsInit = 0;

    trong_khi( 1 ){
      số_nguyên i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      nếu( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        nếu( !callbackIsInit ){
          nCol = sqlite3_column_count(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*sizeof(const char*));
          nếu( azCols==0 ){
            goto exec_out;
          }
          cho(i=0; i<nCol; i++){
            azCols[i] = (char *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            assert( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        nếu( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          cho(i=0; i<nCol; i++){
            azVals[i] = (char *)sqlite3_column_text(pStmt, i);
            nếu( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              goto exec_out;
            }
          }
          azVals[i] = 0;
        }
        nếu( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          goto exec_out;
        }
      }

      nếu( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        trong_khi( sqlite3Isspace(zSql[0]) ) zSql++;
        break;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  nếu( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  nếu( rc!=SQLITE_OK && pzErrMsg ){
    *pzErrMsg = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    nếu( *pzErrMsg==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }không_thì nếu( pzErrMsg ){
    *pzErrMsg = 0;
  }

  assert( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  trả_về rc;
}
