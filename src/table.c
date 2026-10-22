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
#অন্তর্ভাব "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
প্রকারনির্দেশ সংরচনা TabResult {
  বর্ণ **azResult;   /* Accumulated output */
  বর্ণ *z_Err_বার্তা;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  পূর্ণাঙ্ক rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
স্থির পূর্ণাঙ্ক sqlite3_get_table_cb(শূন্য *pArg, পূর্ণাঙ্ক nCol, বর্ণ **argv, বর্ণ **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  পূর্ণাঙ্ক need;                         /* Slots needed in p->azResult[] */
  পূর্ণাঙ্ক i;                            /* Loop counter */
  বর্ণ *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  যদি( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }অন্যথা{
    need = nCol;
  }
  যদি( p->nData + need > p->nAlloc ){
    বর্ণ **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, পরিমাণ(বর্ণ*)*p->nAlloc );
    যদি( azNew==0 ) গচ্ছ malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  যদি( p->nRow==0 ){
    p->nColumn = nCol;
    প্রতি(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      যদি( z==0 ) গচ্ছ malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }অন্যথা যদি( (পূর্ণাঙ্ক)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_বার্তা);
    p->z_Err_বার্তা = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    প্রত্যাগম 1;
  }

  /* Copy over the row data
  */
  যদি( argv!=0 ){
    প্রতি(i=0; i<nCol; i++){
      যদি( argv[i]==0 ){
        z = 0;
      }অন্যথা{
        পূর্ণাঙ্ক n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        যদি( z==0 ) গচ্ছ malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  প্রত্যাগম 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  প্রত্যাগম 1;
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
পূর্ণাঙ্ক sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  ধ্রুব বর্ণ *zSql,           /* The SQL to be executed */
  বর্ণ ***pazResult,          /* Write the result table here */
  পূর্ণাঙ্ক *pnRow,                 /* Write the number of rows in the result here */
  পূর্ণাঙ্ক *pnColumn,              /* Write the number of columns of result here */
  বর্ণ **pz_Err_বার্তা             /* Write error messages here */
){
  পূর্ণাঙ্ক rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) || pazResult==0 ) প্রত্যাগম SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  যদি( pnColumn ) *pnColumn = 0;
  যদি( pnRow ) *pnRow = 0;
  যদি( pz_Err_বার্তা ) *pz_Err_বার্তা = 0;
  res.z_Err_বার্তা = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(পরিমাণ(বর্ণ*)*res.nAlloc );
  যদি( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_বার্তা);
  প্রতিজ্ঞা( পরিমাণ(res.azResult[0])>= পরিমাণ(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  যদি( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    যদি( res.z_Err_বার্তা ){
      যদি( pz_Err_বার্তা ){
        sqlite3_free(*pz_Err_বার্তা);
        *pz_Err_বার্তা = sqlite3_mprintf("%s",res.z_Err_বার্তা);
      }
      sqlite3_free(res.z_Err_বার্তা);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    প্রত্যাগম res.rc;
  }
  sqlite3_free(res.z_Err_বার্তা);
  যদি( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    প্রত্যাগম rc;
  }
  যদি( res.nAlloc>res.nData ){
    বর্ণ **azNew;
    azNew = sqlite3Realloc( res.azResult, পরিমাণ(বর্ণ*)*res.nData );
    যদি( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      প্রত্যাগম SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  যদি( pnColumn ) *pnColumn = res.nColumn;
  যদি( pnRow ) *pnRow = res.nRow;
  প্রত্যাগম rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
শূন্য sqlite3_free_table(
  বর্ণ **azResult            /* Result returned from sqlite3_get_table() */
){
  যদি( azResult ){
    পূর্ণাঙ্ক i, n;
    azResult--;
    প্রতিজ্ঞা( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    প্রতি(i=1; i<n; i++){ যদি( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
