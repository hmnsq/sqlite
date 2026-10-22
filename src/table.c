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
** This file contains the sqlite3_get_table() and sqlite3_free_table()
** interface routines.  These are just wrappers around the main
** interface routine of sqlite3_exec().
**
** These routines are in a separate files so that they will not be linked
** if they are not used.
*/
#अन्तर्भाव "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
प्रकारव्याख्या संरचना TabResult {
  अक्षर **azResult;   /* Accumulated output */
  अक्षर *z_Err_संदेश;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  पूर्णांक rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
स्थिर पूर्णांक sqlite3_get_table_cb(शून्य *pArg, पूर्णांक nCol, अक्षर **argv, अक्षर **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  पूर्णांक need;                         /* Slots needed in p->azResult[] */
  पूर्णांक i;                            /* Loop counter */
  अक्षर *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  जर( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }अन्यथा{
    need = nCol;
  }
  जर( p->nData + need > p->nAlloc ){
    अक्षर **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, आकार(अक्षर*)*p->nAlloc );
    जर( azNew==0 ) जा malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  जर( p->nRow==0 ){
    p->nColumn = nCol;
    साठी(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      जर( z==0 ) जा malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }अन्यथा जर( (पूर्णांक)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_संदेश);
    p->z_Err_संदेश = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    परत 1;
  }

  /* Copy over the row data
  */
  जर( argv!=0 ){
    साठी(i=0; i<nCol; i++){
      जर( argv[i]==0 ){
        z = 0;
      }अन्यथा{
        पूर्णांक n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        जर( z==0 ) जा malloc_failed;
        स_नकल(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  परत 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  परत 1;
}

/*
** Query the database.  But instead of invoking a callback for each row,
** malloc() for space to hold the result and return the entire results
** at the conclusion of the call.
**
** The result that is written to ***pazResult is held in memory obtained
** from malloc().  But the caller cannot free this memory directly.  
** Instead, the entire table should be passed to sqlite3_free_table() when
** the calling procedure is finished using it.
*/
पूर्णांक sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  स्थिरांक अक्षर *zSql,           /* The SQL to be executed */
  अक्षर ***pazResult,          /* Write the result table here */
  पूर्णांक *pnRow,                 /* Write the number of rows in the result here */
  पूर्णांक *pnColumn,              /* Write the number of columns of result here */
  अक्षर **pz_Err_संदेश             /* Write error messages here */
){
  पूर्णांक rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( !sqlite3SafetyCheckOk(db) || pazResult==0 ) परत SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  जर( pnColumn ) *pnColumn = 0;
  जर( pnRow ) *pnRow = 0;
  जर( pz_Err_संदेश ) *pz_Err_संदेश = 0;
  res.z_Err_संदेश = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(आकार(अक्षर*)*res.nAlloc );
  जर( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     परत SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_संदेश);
  प्रतिज्ञा( आकार(res.azResult[0])>= आकार(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  जर( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    जर( res.z_Err_संदेश ){
      जर( pz_Err_संदेश ){
        sqlite3_free(*pz_Err_संदेश);
        *pz_Err_संदेश = sqlite3_mprintf("%s",res.z_Err_संदेश);
      }
      sqlite3_free(res.z_Err_संदेश);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    परत res.rc;
  }
  sqlite3_free(res.z_Err_संदेश);
  जर( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    परत rc;
  }
  जर( res.nAlloc>res.nData ){
    अक्षर **azNew;
    azNew = sqlite3Realloc( res.azResult, आकार(अक्षर*)*res.nData );
    जर( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      परत SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  जर( pnColumn ) *pnColumn = res.nColumn;
  जर( pnRow ) *pnRow = res.nRow;
  परत rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
शून्य sqlite3_free_table(
  अक्षर **azResult            /* Result returned from sqlite3_get_table() */
){
  जर( azResult ){
    पूर्णांक i, n;
    azResult--;
    प्रतिज्ञा( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    साठी(i=1; i<n; i++){ जर( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
