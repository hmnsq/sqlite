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
#அந்தர்பாவ "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
வகைவரை கட்டமைப்பு TabResult {
  எழுத்து **azResult;   /* Accumulated output */
  எழுத்து *z_Err_செய்தி;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  முழு rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
நிலையான முழு sqlite3_get_table_cb(வெறுமை *pArg, முழு nCol, எழுத்து **argv, எழுத்து **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  முழு need;                         /* Slots needed in p->azResult[] */
  முழு i;                            /* Loop counter */
  எழுத்து *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  எனில்( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }இல்லையெனில்{
    need = nCol;
  }
  எனில்( p->nData + need > p->nAlloc ){
    எழுத்து **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, அளவு(எழுத்து*)*p->nAlloc );
    எனில்( azNew==0 ) செல் malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  எனில்( p->nRow==0 ){
    p->nColumn = nCol;
    க்கு(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      எனில்( z==0 ) செல் malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }இல்லையெனில் எனில்( (முழு)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_செய்தி);
    p->z_Err_செய்தி = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    திரும்பு 1;
  }

  /* Copy over the row data
  */
  எனில்( argv!=0 ){
    க்கு(i=0; i<nCol; i++){
      எனில்( argv[i]==0 ){
        z = 0;
      }இல்லையெனில்{
        முழு n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        எனில்( z==0 ) செல் malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  திரும்பு 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  திரும்பு 1;
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
முழு sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  மாறிலி எழுத்து *zSql,           /* The SQL to be executed */
  எழுத்து ***pazResult,          /* Write the result table here */
  முழு *pnRow,                 /* Write the number of rows in the result here */
  முழு *pnColumn,              /* Write the number of columns of result here */
  எழுத்து **pz_Err_செய்தி             /* Write error messages here */
){
  முழு rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) || pazResult==0 ) திரும்பு SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  எனில்( pnColumn ) *pnColumn = 0;
  எனில்( pnRow ) *pnRow = 0;
  எனில்( pz_Err_செய்தி ) *pz_Err_செய்தி = 0;
  res.z_Err_செய்தி = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(அளவு(எழுத்து*)*res.nAlloc );
  எனில்( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     திரும்பு SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_செய்தி);
  ப்ரதிஜ்ஞா( அளவு(res.azResult[0])>= அளவு(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  எனில்( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    எனில்( res.z_Err_செய்தி ){
      எனில்( pz_Err_செய்தி ){
        sqlite3_free(*pz_Err_செய்தி);
        *pz_Err_செய்தி = sqlite3_mprintf("%s",res.z_Err_செய்தி);
      }
      sqlite3_free(res.z_Err_செய்தி);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    திரும்பு res.rc;
  }
  sqlite3_free(res.z_Err_செய்தி);
  எனில்( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    திரும்பு rc;
  }
  எனில்( res.nAlloc>res.nData ){
    எழுத்து **azNew;
    azNew = sqlite3Realloc( res.azResult, அளவு(எழுத்து*)*res.nData );
    எனில்( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      திரும்பு SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  எனில்( pnColumn ) *pnColumn = res.nColumn;
  எனில்( pnRow ) *pnRow = res.nRow;
  திரும்பு rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
வெறுமை sqlite3_free_table(
  எழுத்து **azResult            /* Result returned from sqlite3_get_table() */
){
  எனில்( azResult ){
    முழு i, n;
    azResult--;
    ப்ரதிஜ்ஞா( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    க்கு(i=1; i<n; i++){ எனில்( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
