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
#ضمن "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
تعريفنوع بنية TabResult {
  حرف **azResult;   /* Accumulated output */
  حرف *zErrMsg;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  صحيح rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ساكن صحيح sqlite3_get_table_cb(فراغ *pArg, صحيح nCol, حرف **argv, حرف **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  صحيح need;                         /* Slots needed in p->azResult[] */
  صحيح i;                            /* Loop counter */
  حرف *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  𐡄𐡍( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }وإلا{
    need = nCol;
  }
  𐡄𐡍( p->nData + need > p->nAlloc ){
    حرف **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, حجم(حرف*)*p->nAlloc );
    𐡄𐡍( azNew==0 ) اذهب malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  𐡄𐡍( p->nRow==0 ){
    p->nColumn = nCol;
    لكل(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      𐡄𐡍( z==0 ) اذهب malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }وإلا 𐡄𐡍( (صحيح)p->nColumn!=nCol ){
    sqlite3_free(p->zErrMsg);
    p->zErrMsg = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    أرجع 1;
  }

  /* Copy over the row data
  */
  𐡄𐡍( argv!=0 ){
    لكل(i=0; i<nCol; i++){
      𐡄𐡍( argv[i]==0 ){
        z = 0;
      }وإلا{
        صحيح n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        𐡄𐡍( z==0 ) اذهب malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  أرجع 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  أرجع 1;
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
صحيح sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  ثابت حرف *zSql,           /* The SQL to be executed */
  حرف ***pazResult,          /* Write the result table here */
  صحيح *pnRow,                 /* Write the number of rows in the result here */
  صحيح *pnColumn,              /* Write the number of columns of result here */
  حرف **pzErrMsg             /* Write error messages here */
){
  صحيح rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  𐡄𐡍( !sqlite3SafetyCheckOk(db) || pazResult==0 ) أرجع SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  𐡄𐡍( pnColumn ) *pnColumn = 0;
  𐡄𐡍( pnRow ) *pnRow = 0;
  𐡄𐡍( pzErrMsg ) *pzErrMsg = 0;
  res.zErrMsg = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(حجم(حرف*)*res.nAlloc );
  𐡄𐡍( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     أرجع SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pzErrMsg);
  أكد( حجم(res.azResult[0])>= حجم(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  𐡄𐡍( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    𐡄𐡍( res.zErrMsg ){
      𐡄𐡍( pzErrMsg ){
        sqlite3_free(*pzErrMsg);
        *pzErrMsg = sqlite3_mprintf("%s",res.zErrMsg);
      }
      sqlite3_free(res.zErrMsg);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    أرجع res.rc;
  }
  sqlite3_free(res.zErrMsg);
  𐡄𐡍( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    أرجع rc;
  }
  𐡄𐡍( res.nAlloc>res.nData ){
    حرف **azNew;
    azNew = sqlite3Realloc( res.azResult, حجم(حرف*)*res.nData );
    𐡄𐡍( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      أرجع SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  𐡄𐡍( pnColumn ) *pnColumn = res.nColumn;
  𐡄𐡍( pnRow ) *pnRow = res.nRow;
  أرجع rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
فراغ sqlite3_free_table(
  حرف **azResult            /* Result returned from sqlite3_get_table() */
){
  𐡄𐡍( azResult ){
    صحيح i, n;
    azResult--;
    أكد( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    لكل(i=1; i<n; i++){ 𐡄𐡍( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
