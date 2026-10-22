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
#include "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
typedef sa_nracanaa TabResult {
  var_na **azResult;   /* Accumulated output */
  var_na *zErrMsg;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  ཧྲིལ rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
static ཧྲིལ sqlite3_get_table_cb(void *pArg, ཧྲིལ nCol, var_na **argv, var_na **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  ཧྲིལ need;                         /* Slots needed in p->azResult[] */
  ཧྲིལ i;                            /* Loop counter */
  var_na *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  གལ་སྲིད( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }དེ་མིན{
    need = nCol;
  }
  གལ་སྲིད( p->nData + need > p->nAlloc ){
    var_na **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, parimaa_na(var_na*)*p->nAlloc );
    གལ་སྲིད( azNew==0 ) goto malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  གལ་སྲིད( p->nRow==0 ){
    p->nColumn = nCol;
    དོན་ལུ(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      གལ་སྲིད( z==0 ) goto malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }དེ་མིན གལ་སྲིད( (ཧྲིལ)p->nColumn!=nCol ){
    sqlite3_free(p->zErrMsg);
    p->zErrMsg = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    ལོག 1;
  }

  /* Copy over the row data
  */
  གལ་སྲིད( argv!=0 ){
    དོན་ལུ(i=0; i<nCol; i++){
      གལ་སྲིད( argv[i]==0 ){
        z = 0;
      }དེ་མིན{
        ཧྲིལ n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        གལ་སྲིད( z==0 ) goto malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  ལོག 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  ལོག 1;
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
ཧྲིལ sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  const var_na *zSql,           /* The SQL to be executed */
  var_na ***pazResult,          /* Write the result table here */
  ཧྲིལ *pnRow,                 /* Write the number of rows in the result here */
  ཧྲིལ *pnColumn,              /* Write the number of columns of result here */
  var_na **pzErrMsg             /* Write error messages here */
){
  ཧྲིལ rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) || pazResult==0 ) ལོག SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  གལ་སྲིད( pnColumn ) *pnColumn = 0;
  གལ་སྲིད( pnRow ) *pnRow = 0;
  གལ་སྲིད( pzErrMsg ) *pzErrMsg = 0;
  res.zErrMsg = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(parimaa_na(var_na*)*res.nAlloc );
  གལ་སྲིད( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     ལོག SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pzErrMsg);
  assert( parimaa_na(res.azResult[0])>= parimaa_na(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  གལ་སྲིད( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    གལ་སྲིད( res.zErrMsg ){
      གལ་སྲིད( pzErrMsg ){
        sqlite3_free(*pzErrMsg);
        *pzErrMsg = sqlite3_mprintf("%s",res.zErrMsg);
      }
      sqlite3_free(res.zErrMsg);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    ལོག res.rc;
  }
  sqlite3_free(res.zErrMsg);
  གལ་སྲིད( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    ལོག rc;
  }
  གལ་སྲིད( res.nAlloc>res.nData ){
    var_na **azNew;
    azNew = sqlite3Realloc( res.azResult, parimaa_na(var_na*)*res.nData );
    གལ་སྲིད( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      ལོག SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  གལ་སྲིད( pnColumn ) *pnColumn = res.nColumn;
  གལ་སྲིད( pnRow ) *pnRow = res.nRow;
  ལོག rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
void sqlite3_free_table(
  var_na **azResult            /* Result returned from sqlite3_get_table() */
){
  གལ་སྲིད( azResult ){
    ཧྲིལ i, n;
    azResult--;
    assert( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    དོན་ལུ(i=1; i<n; i++){ གལ་སྲིད( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
