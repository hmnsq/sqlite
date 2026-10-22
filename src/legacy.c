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

#అన్తర్భావ "sqliteInt.h"

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
పూర్ణాంకం sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  స్థిరరాశి అక్షరం *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  శూన్య *pArg,                 /* First argument to xCallback() */
  అక్షరం **pz_Err_సందేశం             /* Write error messages here */
){
  పూర్ణాంకం rc = SQLITE_OK;         /* Return code */
  స్థిరరాశి అక్షరం *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  అక్షరం **az_నిలువుల_సంఖ్య = 0;          /* Names of result columns */
  పూర్ణాంకం callbackIsInit;         /* True if callback data is initialized */

  ఒకవేళ( !sqlite3SafetyCheckOk(db) ) తిరిగి SQLITE_MISUSE_BKPT;
  ఒకవేళ( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  అయితే( rc==SQLITE_OK && zSql[0] ){
    పూర్ణాంకం n_నిలువు = 0;
    అక్షరం **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    ప్రతిజ్ఞా( rc==SQLITE_OK || pStmt==0 );
    ఒకవేళ( rc!=SQLITE_OK ){
      కొనసాగు;
    }
    ఒకవేళ( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      కొనసాగు;
    }
    callbackIsInit = 0;

    అయితే( 1 ){
      పూర్ణాంకం i;
      rc = sqlite3_అడుగు(pStmt);

      /* Invoke the callback function if required */
      ఒకవేళ( xCallback && (SQLITE_అడ్డువరుస==rc || 
          (SQLITE_పూర్తి==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        ఒకవేళ( !callbackIsInit ){
          n_నిలువు = sqlite3_స్తంభం_లెక్క(pStmt);
          az_నిలువుల_సంఖ్య = sqlite3DbMallocRaw(db, (2*n_నిలువు+1)*పరిమాణం(స్థిరరాశి అక్షరం*));
          ఒకవేళ( az_నిలువుల_సంఖ్య==0 ){
            వెళ్ళు exec_బహిర్గతం;
          }
          కొరకు(i=0; i<n_నిలువు; i++){
            az_నిలువుల_సంఖ్య[i] = (అక్షరం *)sqlite3_స్తంభం_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            ప్రతిజ్ఞా( az_నిలువుల_సంఖ్య[i]!=0 );
          }
          callbackIsInit = 1;
        }
        ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
          azVals = &az_నిలువుల_సంఖ్య[n_నిలువు];
          కొరకు(i=0; i<n_నిలువు; i++){
            azVals[i] = (అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pStmt, i);
            ఒకవేళ( !azVals[i] && sqlite3_స్తంభం_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              వెళ్ళు exec_బహిర్గతం;
            }
          }
          azVals[i] = 0;
        }
        ఒకవేళ( xCallback(pArg, n_నిలువు, azVals, az_నిలువుల_సంఖ్య) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          వెళ్ళు exec_బహిర్గతం;
        }
      }

      ఒకవేళ( rc!=SQLITE_అడ్డువరుస ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        అయితే( sqlite3Isspace(zSql[0]) ) zSql++;
        ఆపు;
      }
    }

    sqlite3DbFree(db, az_నిలువుల_సంఖ్య);
    az_నిలువుల_సంఖ్య = 0;
  }

exec_బహిర్గతం:
  ఒకవేళ( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, az_నిలువుల_సంఖ్య);

  rc = sqlite3ApiExit(db, rc);
  ఒకవేళ( rc!=SQLITE_OK && pz_Err_సందేశం ){
    *pz_Err_సందేశం = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    ఒకవేళ( *pz_Err_సందేశం==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }లేకపోతే ఒకవేళ( pz_Err_సందేశం ){
    *pz_Err_సందేశం = 0;
  }

  ప్రతిజ్ఞా( (rc&db->err_ముసుగు)==rc );
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}
