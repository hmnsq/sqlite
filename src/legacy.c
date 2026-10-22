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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

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
ਪੂਰਨ sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  ਅਚਲ ਅੱਖਰ *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  ਖਾਲੀ *pArg,                 /* First argument to xCallback() */
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ             /* Write error messages here */
){
  ਪੂਰਨ rc = SQLITE_OK;         /* Return code */
  ਅਚਲ ਅੱਖਰ *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  ਅੱਖਰ **az_ਖਾਨੇ_ਗਿਣਤੀ = 0;          /* Names of result columns */
  ਪੂਰਨ callbackIsInit;         /* True if callback data is initialized */

  ਜੇ( !sqlite3SafetyCheckOk(db) ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
  ਜੇ( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  ਜਦੋਂ( rc==SQLITE_OK && zSql[0] ){
    ਪੂਰਨ n_ਖਾਨਾ = 0;
    ਅੱਖਰ **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || pStmt==0 );
    ਜੇ( rc!=SQLITE_OK ){
      ਜਾਰੀ;
    }
    ਜੇ( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      ਜਾਰੀ;
    }
    callbackIsInit = 0;

    ਜਦੋਂ( 1 ){
      ਪੂਰਨ i;
      rc = sqlite3_ਕਦਮ(pStmt);

      /* Invoke the callback function if required */
      ਜੇ( xCallback && (SQLITE_ਪੰਕਤੀ==rc || 
          (SQLITE_ਪੂਰਾ==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        ਜੇ( !callbackIsInit ){
          n_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਗਿਣਤੀ(pStmt);
          az_ਖਾਨੇ_ਗਿਣਤੀ = sqlite3DbMallocRaw(db, (2*n_ਖਾਨਾ+1)*ਆਕਾਰ(ਅਚਲ ਅੱਖਰ*));
          ਜੇ( az_ਖਾਨੇ_ਗਿਣਤੀ==0 ){
            ਜਾਓ exec_ਬਾਹਰਲਾ;
          }
          ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
            az_ਖਾਨੇ_ਗਿਣਤੀ[i] = (ਅੱਖਰ *)sqlite3_ਕਾਲਮ_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            ਪ੍ਰਤਿਜ੍ਞਾ( az_ਖਾਨੇ_ਗਿਣਤੀ[i]!=0 );
          }
          callbackIsInit = 1;
        }
        ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
          azVals = &az_ਖਾਨੇ_ਗਿਣਤੀ[n_ਖਾਨਾ];
          ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
            azVals[i] = (ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, i);
            ਜੇ( !azVals[i] && sqlite3_ਕਾਲਮ_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              ਜਾਓ exec_ਬਾਹਰਲਾ;
            }
          }
          azVals[i] = 0;
        }
        ਜੇ( xCallback(pArg, n_ਖਾਨਾ, azVals, az_ਖਾਨੇ_ਗਿਣਤੀ) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          ਜਾਓ exec_ਬਾਹਰਲਾ;
        }
      }

      ਜੇ( rc!=SQLITE_ਪੰਕਤੀ ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        ਜਦੋਂ( sqlite3Isspace(zSql[0]) ) zSql++;
        ਰੋਕੋ;
      }
    }

    sqlite3DbFree(db, az_ਖਾਨੇ_ਗਿਣਤੀ);
    az_ਖਾਨੇ_ਗਿਣਤੀ = 0;
  }

exec_ਬਾਹਰਲਾ:
  ਜੇ( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, az_ਖਾਨੇ_ਗਿਣਤੀ);

  rc = sqlite3ApiExit(db, rc);
  ਜੇ( rc!=SQLITE_OK && pz_Err_ਸੁਨੇਹਾ ){
    *pz_Err_ਸੁਨੇਹਾ = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    ਜੇ( *pz_Err_ਸੁਨੇਹਾ==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pz_Err_ਸੁਨੇਹਾ ){
    *pz_Err_ਸੁਨੇਹਾ = 0;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( (rc&db->err_ਨਕਾਬ)==rc );
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}
