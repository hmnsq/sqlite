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
typedef struct TabResult {
  char **azResult;   /* Accumulated output */
  char *zErrMsg;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  整数 rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
static 整数 sqlite3_get_table_cb(void *pArg, 整数 nCol, char **argv, char **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  整数 need;                         /* Slots needed in p->azResult[] */
  整数 i;                            /* Loop counter */
  char *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  もし( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }否则{
    need = nCol;
  }
  もし( p->nData + need > p->nAlloc ){
    char **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, sizeof(char*)*p->nAlloc );
    もし( azNew==0 ) goto malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  もし( p->nRow==0 ){
    p->nColumn = nCol;
    ために(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      もし( z==0 ) goto malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }否则 もし( (整数)p->nColumn!=nCol ){
    sqlite3_free(p->zErrMsg);
    p->zErrMsg = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    戻る 1;
  }

  /* Copy over the row data
  */
  もし( argv!=0 ){
    ために(i=0; i<nCol; i++){
      もし( argv[i]==0 ){
        z = 0;
      }否则{
        整数 n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        もし( z==0 ) goto malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  戻る 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  戻る 1;
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
整数 sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  const char *zSql,           /* The SQL to be executed */
  char ***pazResult,          /* Write the result table here */
  整数 *pnRow,                 /* Write the number of rows in the result here */
  整数 *pnColumn,              /* Write the number of columns of result here */
  char **pzErrMsg             /* Write error messages here */
){
  整数 rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  もし( !sqlite3SafetyCheckOk(db) || pazResult==0 ) 戻る SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  もし( pnColumn ) *pnColumn = 0;
  もし( pnRow ) *pnRow = 0;
  もし( pzErrMsg ) *pzErrMsg = 0;
  res.zErrMsg = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(sizeof(char*)*res.nAlloc );
  もし( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     戻る SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pzErrMsg);
  assert( sizeof(res.azResult[0])>= sizeof(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  もし( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    もし( res.zErrMsg ){
      もし( pzErrMsg ){
        sqlite3_free(*pzErrMsg);
        *pzErrMsg = sqlite3_mprintf("%s",res.zErrMsg);
      }
      sqlite3_free(res.zErrMsg);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    戻る res.rc;
  }
  sqlite3_free(res.zErrMsg);
  もし( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    戻る rc;
  }
  もし( res.nAlloc>res.nData ){
    char **azNew;
    azNew = sqlite3Realloc( res.azResult, sizeof(char*)*res.nData );
    もし( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      戻る SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  もし( pnColumn ) *pnColumn = res.nColumn;
  もし( pnRow ) *pnRow = res.nRow;
  戻る rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
void sqlite3_free_table(
  char **azResult            /* Result returned from sqlite3_get_table() */
){
  もし( azResult ){
    整数 i, n;
    azResult--;
    assert( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    ために(i=1; i<n; i++){ もし( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
