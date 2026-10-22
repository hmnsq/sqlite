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
#شامل "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
تعریفنوع ساختار TabResult {
  نویسه **azResult;   /* Accumulated output */
  نویسه *z_Err_پیام;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  صحیح rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ایستا صحیح sqlite3_get_table_cb(تهی *pArg, صحیح nCol, نویسه **argv, نویسه **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  صحیح need;                         /* Slots needed in p->azResult[] */
  صحیح i;                            /* Loop counter */
  نویسه *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  اگر( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }وگرنه{
    need = nCol;
  }
  اگر( p->nData + need > p->nAlloc ){
    نویسه **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, اندازه(نویسه*)*p->nAlloc );
    اگر( azNew==0 ) برو malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  اگر( p->nRow==0 ){
    p->nColumn = nCol;
    برای(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      اگر( z==0 ) برو malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }وگرنه اگر( (صحیح)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_پیام);
    p->z_Err_پیام = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    بازگشت 1;
  }

  /* Copy over the row data
  */
  اگر( argv!=0 ){
    برای(i=0; i<nCol; i++){
      اگر( argv[i]==0 ){
        z = 0;
      }وگرنه{
        صحیح n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        اگر( z==0 ) برو malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  بازگشت 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  بازگشت 1;
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
صحیح sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  ثابت نویسه *zSql,           /* The SQL to be executed */
  نویسه ***pazResult,          /* Write the result table here */
  صحیح *pnRow,                 /* Write the number of rows in the result here */
  صحیح *pnColumn,              /* Write the number of columns of result here */
  نویسه **pz_Err_پیام             /* Write error messages here */
){
  صحیح rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || pazResult==0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  اگر( pnColumn ) *pnColumn = 0;
  اگر( pnRow ) *pnRow = 0;
  اگر( pz_Err_پیام ) *pz_Err_پیام = 0;
  res.z_Err_پیام = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(اندازه(نویسه*)*res.nAlloc );
  اگر( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     بازگشت SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_پیام);
  أكد( اندازه(res.azResult[0])>= اندازه(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  اگر( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    اگر( res.z_Err_پیام ){
      اگر( pz_Err_پیام ){
        sqlite3_free(*pz_Err_پیام);
        *pz_Err_پیام = sqlite3_mprintf("%s",res.z_Err_پیام);
      }
      sqlite3_free(res.z_Err_پیام);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    بازگشت res.rc;
  }
  sqlite3_free(res.z_Err_پیام);
  اگر( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    بازگشت rc;
  }
  اگر( res.nAlloc>res.nData ){
    نویسه **azNew;
    azNew = sqlite3Realloc( res.azResult, اندازه(نویسه*)*res.nData );
    اگر( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      بازگشت SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  اگر( pnColumn ) *pnColumn = res.nColumn;
  اگر( pnRow ) *pnRow = res.nRow;
  بازگشت rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
تهی sqlite3_free_table(
  نویسه **azResult            /* Result returned from sqlite3_get_table() */
){
  اگر( azResult ){
    صحیح i, n;
    azResult--;
    أكد( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    برای(i=1; i<n; i++){ اگر( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
