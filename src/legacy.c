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
पूर्णाङ्क sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  ध्रुव वर्ण *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  शून्य *pArg,                 /* First argument to xCallback() */
  वर्ण **pz_Err_सन्देशः             /* Write error messages here */
){
  पूर्णाङ्क rc = SQLITE_OK;         /* Return code */
  ध्रुव वर्ण *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  वर्ण **az_स्तम्भसङ्ख्या = 0;          /* Names of result columns */
  पूर्णाङ्क callbackIsInit;         /* True if callback data is initialized */

  यदि( !sqlite3SafetyCheckOk(db) ) प्रत्यागम SQLITE_MISUSE_BKPT;
  यदि( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  यावत्( rc==SQLITE_OK && zSql[0] ){
    पूर्णाङ्क nCol = 0;
    वर्ण **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    प्रतिज्ञा( rc==SQLITE_OK || pStmt==0 );
    यदि( rc!=SQLITE_OK ){
      अनुवर्त;
    }
    यदि( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      अनुवर्त;
    }
    callbackIsInit = 0;

    यावत्( 1 ){
      पूर्णाङ्क i;
      rc = sqlite3_चरणः(pStmt);

      /* Invoke the callback function if required */
      यदि( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_समाप्तम्==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        यदि( !callbackIsInit ){
          nCol = sqlite3_column_गणनम्(pStmt);
          az_स्तम्भसङ्ख्या = sqlite3DbMallocRaw(db, (2*nCol+1)*परिमाण(ध्रुव वर्ण*));
          यदि( az_स्तम्भसङ्ख्या==0 ){
            गच्छ exec_बहिर्गमनम्;
          }
          प्रति(i=0; i<nCol; i++){
            az_स्तम्भसङ्ख्या[i] = (वर्ण *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            प्रतिज्ञा( az_स्तम्भसङ्ख्या[i]!=0 );
          }
          callbackIsInit = 1;
        }
        यदि( rc==SQLITE_ROW ){
          azVals = &az_स्तम्भसङ्ख्या[nCol];
          प्रति(i=0; i<nCol; i++){
            azVals[i] = (वर्ण *)sqlite3_column_text(pStmt, i);
            यदि( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              गच्छ exec_बहिर्गमनम्;
            }
          }
          azVals[i] = 0;
        }
        यदि( xCallback(pArg, nCol, azVals, az_स्तम्भसङ्ख्या) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          गच्छ exec_बहिर्गमनम्;
        }
      }

      यदि( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        यावत्( sqlite3Isspace(zSql[0]) ) zSql++;
        भङ्ग;
      }
    }

    sqlite3DbFree(db, az_स्तम्भसङ्ख्या);
    az_स्तम्भसङ्ख्या = 0;
  }

exec_बहिर्गमनम्:
  यदि( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, az_स्तम्भसङ्ख्या);

  rc = sqlite3ApiExit(db, rc);
  यदि( rc!=SQLITE_OK && pz_Err_सन्देशः ){
    *pz_Err_सन्देशः = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    यदि( *pz_Err_सन्देशः==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }अन्यथा यदि( pz_Err_सन्देशः ){
    *pz_Err_सन्देशः = 0;
  }

  प्रतिज्ञा( (rc&db->err_आवरणम्)==rc );
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम rc;
}
