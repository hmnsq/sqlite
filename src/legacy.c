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
  حرف **pz_Err_نياپو             /* Write error messages here */
){
  صحيح rc = SQLITE_OK;         /* Return code */
  ثابت حرف *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  حرف **az_کانن_ڳڻپ = 0;          /* Names of result columns */
  صحيح callbackIsInit;         /* True if callback data is initialized */

  إذا( !sqlite3SafetyCheckOk(db) ) أرجع SQLITE_MISUSE_BKPT;
  إذا( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  طالما( rc==SQLITE_OK && zSql[0] ){
    صحيح n_کانو = 0;
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

    طالما( 1 ){
      صحيح i;
      rc = sqlite3_قدم(pStmt);

      /* Invoke the callback function if required */
      إذا( xCallback && (SQLITE_صف==rc || 
          (SQLITE_پورو==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        إذا( !callbackIsInit ){
          n_کانو = sqlite3_ڪالم_ڳڻپ(pStmt);
          az_کانن_ڳڻپ = sqlite3DbMallocRaw(db, (2*n_کانو+1)*حجم(ثابت حرف*));
          إذا( az_کانن_ڳڻپ==0 ){
            اذهب exec_ٻاهريون;
          }
          لكل(i=0; i<n_کانو; i++){
            az_کانن_ڳڻپ[i] = (حرف *)sqlite3_ڪالم_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            أكد( az_کانن_ڳڻپ[i]!=0 );
          }
          callbackIsInit = 1;
        }
        إذا( rc==SQLITE_صف ){
          azVals = &az_کانن_ڳڻپ[n_کانو];
          لكل(i=0; i<n_کانو; i++){
            azVals[i] = (حرف *)sqlite3_ڪالم_متن(pStmt, i);
            إذا( !azVals[i] && sqlite3_ڪالم_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              اذهب exec_ٻاهريون;
            }
          }
          azVals[i] = 0;
        }
        إذا( xCallback(pArg, n_کانو, azVals, az_کانن_ڳڻپ) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          اذهب exec_ٻاهريون;
        }
      }

      إذا( rc!=SQLITE_صف ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        طالما( sqlite3Isspace(zSql[0]) ) zSql++;
        اكسر;
      }
    }

    sqlite3DbFree(db, az_کانن_ڳڻپ);
    az_کانن_ڳڻپ = 0;
  }

exec_ٻاهريون:
  إذا( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, az_کانن_ڳڻپ);

  rc = sqlite3ApiExit(db, rc);
  إذا( rc!=SQLITE_OK && pz_Err_نياپو ){
    *pz_Err_نياپو = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    إذا( *pz_Err_نياپو==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }وإلا إذا( pz_Err_نياپو ){
    *pz_Err_نياپو = 0;
  }

  أكد( (rc&db->err_نقاب)==rc );
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}
