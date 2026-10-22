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
  целое rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
static целое sqlite3_get_table_cb(void *pArg, целое nCol, char **argv, char **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  целое need;                         /* Slots needed in p->azResult[] */
  целое i;                            /* Loop counter */
  char *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  если( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }иначе{
    need = nCol;
  }
  если( p->nData + need > p->nAlloc ){
    char **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, sizeof(char*)*p->nAlloc );
    если( azNew==0 ) goto malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  если( p->nRow==0 ){
    p->nColumn = nCol;
    для(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      если( z==0 ) goto malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }иначе если( (целое)p->nColumn!=nCol ){
    sqlite3_free(p->zErrMsg);
    p->zErrMsg = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    вернуть 1;
  }

  /* Copy over the row data
  */
  если( argv!=0 ){
    для(i=0; i<nCol; i++){
      если( argv[i]==0 ){
        z = 0;
      }иначе{
        целое n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        если( z==0 ) goto malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  вернуть 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  вернуть 1;
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
целое sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  const char *zSql,           /* The SQL to be executed */
  char ***pazResult,          /* Write the result table here */
  целое *pnRow,                 /* Write the number of rows in the result here */
  целое *pnColumn,              /* Write the number of columns of result here */
  char **pzErrMsg             /* Write error messages here */
){
  целое rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  если( !sqlite3SafetyCheckOk(db) || pazResult==0 ) вернуть SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  если( pnColumn ) *pnColumn = 0;
  если( pnRow ) *pnRow = 0;
  если( pzErrMsg ) *pzErrMsg = 0;
  res.zErrMsg = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(sizeof(char*)*res.nAlloc );
  если( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     вернуть SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pzErrMsg);
  assert( sizeof(res.azResult[0])>= sizeof(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  если( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    если( res.zErrMsg ){
      если( pzErrMsg ){
        sqlite3_free(*pzErrMsg);
        *pzErrMsg = sqlite3_mprintf("%s",res.zErrMsg);
      }
      sqlite3_free(res.zErrMsg);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    вернуть res.rc;
  }
  sqlite3_free(res.zErrMsg);
  если( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    вернуть rc;
  }
  если( res.nAlloc>res.nData ){
    char **azNew;
    azNew = sqlite3Realloc( res.azResult, sizeof(char*)*res.nData );
    если( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      вернуть SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  если( pnColumn ) *pnColumn = res.nColumn;
  если( pnRow ) *pnRow = res.nRow;
  вернуть rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
void sqlite3_free_table(
  char **azResult            /* Result returned from sqlite3_get_table() */
){
  если( azResult ){
    целое i, n;
    azResult--;
    assert( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    для(i=1; i<n; i++){ если( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
