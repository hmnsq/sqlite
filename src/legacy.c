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
  ثابت حرف *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  خالی *pArg,                 /* First argument to xCallback() */
  حرف **pz_Err_پیغام             /* Write error messages here */
){
  صحیح rc = SQLITE_OK;         /* Return code */
  ثابت حرف *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  حرف **az_ستون_تعداد = 0;          /* Names of result columns */
  صحیح callbackIsInit;         /* True if callback data is initialized */

  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
  اگر( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  جب( rc==SQLITE_OK && zSql[0] ){
    صحیح n_ستون = 0;
    حرف **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    أكد( rc==SQLITE_OK || pStmt==0 );
    اگر( rc!=SQLITE_OK ){
      جاری;
    }
    اگر( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      جاری;
    }
    callbackIsInit = 0;

    جب( 1 ){
      صحیح i;
      rc = sqlite3_قدم(pStmt);

      /* Invoke the callback function if required */
      اگر( xCallback && (SQLITE_صف==rc || 
          (SQLITE_مکمل==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        اگر( !callbackIsInit ){
          n_ستون = sqlite3_عمود_گنتی(pStmt);
          az_ستون_تعداد = sqlite3DbMallocRaw(db, (2*n_ستون+1)*حجم(ثابت حرف*));
          اگر( az_ستون_تعداد==0 ){
            جاؤ exec_برآمد;
          }
          برائے(i=0; i<n_ستون; i++){
            az_ستون_تعداد[i] = (حرف *)sqlite3_عمود_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            أكد( az_ستون_تعداد[i]!=0 );
          }
          callbackIsInit = 1;
        }
        اگر( rc==SQLITE_صف ){
          azVals = &az_ستون_تعداد[n_ستون];
          برائے(i=0; i<n_ستون; i++){
            azVals[i] = (حرف *)sqlite3_عمود_متن(pStmt, i);
            اگر( !azVals[i] && sqlite3_عمود_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              جاؤ exec_برآمد;
            }
          }
          azVals[i] = 0;
        }
        اگر( xCallback(pArg, n_ستون, azVals, az_ستون_تعداد) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          جاؤ exec_برآمد;
        }
      }

      اگر( rc!=SQLITE_صف ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        جب( sqlite3Isspace(zSql[0]) ) zSql++;
        توڑو;
      }
    }

    sqlite3DbFree(db, az_ستون_تعداد);
    az_ستون_تعداد = 0;
  }

exec_برآمد:
  اگر( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, az_ستون_تعداد);

  rc = sqlite3ApiExit(db, rc);
  اگر( rc!=SQLITE_OK && pz_Err_پیغام ){
    *pz_Err_پیغام = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    اگر( *pz_Err_پیغام==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }ورنہ اگر( pz_Err_پیغام ){
    *pz_Err_پیغام = 0;
  }

  أكد( (rc&db->err_نقاب)==rc );
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}
