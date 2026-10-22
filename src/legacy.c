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

#അന്തര്ഭാവ "sqliteInt.h"

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
പൂർണ്ണസംഖ്യ sqlite3_exec(
  sqlite3 *db,                /* The database on which the SQL executes */
  സ്ഥിരം അക്ഷരം *zSql,           /* The SQL to be executed */
  sqlite3_callback xCallback, /* Invoke this callback routine */
  ശൂന്യം *pArg,                 /* First argument to xCallback() */
  അക്ഷരം **pz_Err_സന്ദേശം             /* Write error messages here */
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;         /* Return code */
  സ്ഥിരം അക്ഷരം *zLeftover;      /* Tail of unprocessed SQL */
  sqlite3_stmt *pStmt = 0;    /* The current SQL statement */
  അക്ഷരം **azCols = 0;          /* Names of result columns */
  പൂർണ്ണസംഖ്യ callbackIsInit;         /* True if callback data is initialized */

  എങ്കിൽ( !sqlite3SafetyCheckOk(db) ) തിരികെ SQLITE_MISUSE_BKPT;
  എങ്കിൽ( zSql==0 ) zSql = "";

  sqlite3_mutex_enter(db->mutex);
  sqlite3Error(db, SQLITE_OK);
  ആകുമ്പോൾ( rc==SQLITE_OK && zSql[0] ){
    പൂർണ്ണസംഖ്യ nCol = 0;
    അക്ഷരം **azVals = 0;

    pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, &zLeftover);
    പ്രതിജ്ഞാ( rc==SQLITE_OK || pStmt==0 );
    എങ്കിൽ( rc!=SQLITE_OK ){
      തുടരുക;
    }
    എങ്കിൽ( !pStmt ){
      /* this happens for a comment or white-space */
      zSql = zLeftover;
      തുടരുക;
    }
    callbackIsInit = 0;

    ആകുമ്പോൾ( 1 ){
      പൂർണ്ണസംഖ്യ i;
      rc = sqlite3_step(pStmt);

      /* Invoke the callback function if required */
      എങ്കിൽ( xCallback && (SQLITE_ROW==rc || 
          (SQLITE_DONE==rc && !callbackIsInit
                           && db->flags&SQLITE_NullCallback)) ){
        എങ്കിൽ( !callbackIsInit ){
          nCol = sqlite3_column_എണ്ണം(pStmt);
          azCols = sqlite3DbMallocRaw(db, (2*nCol+1)*വലുപ്പം(സ്ഥിരം അക്ഷരം*));
          എങ്കിൽ( azCols==0 ){
            പോകുക exec_out;
          }
          വേണ്ടി(i=0; i<nCol; i++){
            azCols[i] = (അക്ഷരം *)sqlite3_column_name(pStmt, i);
            /* sqlite3VdbeSetColName() installs column names as UTF8
            ** strings so there is no way for sqlite3_column_name() to fail. */
            പ്രതിജ്ഞാ( azCols[i]!=0 );
          }
          callbackIsInit = 1;
        }
        എങ്കിൽ( rc==SQLITE_ROW ){
          azVals = &azCols[nCol];
          വേണ്ടി(i=0; i<nCol; i++){
            azVals[i] = (അക്ഷരം *)sqlite3_column_text(pStmt, i);
            എങ്കിൽ( !azVals[i] && sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
              sqlite3OomFault(db);
              പോകുക exec_out;
            }
          }
          azVals[i] = 0;
        }
        എങ്കിൽ( xCallback(pArg, nCol, azVals, azCols) ){
          /* EVIDENCE-OF: R-38229-40159 If the callback function to
          ** sqlite3_exec() returns non-zero, then sqlite3_exec() will
          ** return SQLITE_ABORT. */
          rc = SQLITE_ABORT;
          sqlite3VdbeFinalize((Vdbe *)pStmt);
          pStmt = 0;
          sqlite3Error(db, SQLITE_ABORT);
          പോകുക exec_out;
        }
      }

      എങ്കിൽ( rc!=SQLITE_ROW ){
        rc = sqlite3VdbeFinalize((Vdbe *)pStmt);
        pStmt = 0;
        zSql = zLeftover;
        ആകുമ്പോൾ( sqlite3Isspace(zSql[0]) ) zSql++;
        നിർത്തുക;
      }
    }

    sqlite3DbFree(db, azCols);
    azCols = 0;
  }

exec_out:
  എങ്കിൽ( pStmt ) sqlite3VdbeFinalize((Vdbe *)pStmt);
  sqlite3DbFree(db, azCols);

  rc = sqlite3ApiExit(db, rc);
  എങ്കിൽ( rc!=SQLITE_OK && pz_Err_സന്ദേശം ){
    *pz_Err_സന്ദേശം = sqlite3DbStrDup(0, sqlite3_errmsg(db));
    എങ്കിൽ( *pz_Err_സന്ദേശം==0 ){
      rc = SQLITE_NOMEM_BKPT;
      sqlite3Error(db, SQLITE_NOMEM);
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( pz_Err_സന്ദേശം ){
    *pz_Err_സന്ദേശം = 0;
  }

  പ്രതിജ്ഞാ( (rc&db->errMask)==rc );
  sqlite3_mutex_leave(db->mutex);
  തിരികെ rc;
}
