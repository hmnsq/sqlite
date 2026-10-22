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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TabResult {
  ଅକ୍ଷର **azResult;   /* Accumulated output */
  ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_get_table_cb(ଶୂନ୍ୟ *pArg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol, ଅକ୍ଷର **argv, ଅକ୍ଷର **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା need;                         /* Slots needed in p->azResult[] */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                            /* Loop counter */
  ଅକ୍ଷର *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  ଯଦି( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }ଅନ୍ୟଥା{
    need = nCol;
  }
  ଯଦି( p->nData + need > p->nAlloc ){
    ଅକ୍ଷର **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, ଆକାର(ଅକ୍ଷର*)*p->nAlloc );
    ଯଦି( azNew==0 ) ଯାଅ malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  ଯଦି( p->nRow==0 ){
    p->nColumn = nCol;
    ପାଇଁ(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      ଯଦି( z==0 ) ଯାଅ malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }ଅନ୍ୟଥା ଯଦି( (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_ସନ୍ଦେଶ);
    p->z_Err_ସନ୍ଦେଶ = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    ଫେରନ୍ତୁ 1;
  }

  /* Copy over the row data
  */
  ଯଦି( argv!=0 ){
    ପାଇଁ(i=0; i<nCol; i++){
      ଯଦି( argv[i]==0 ){
        z = 0;
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        ଯଦି( z==0 ) ଯାଅ malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  ଫେରନ୍ତୁ 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  ଫେରନ୍ତୁ 1;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  ଅଚଳ ଅକ୍ଷର *zSql,           /* The SQL to be executed */
  ଅକ୍ଷର ***pazResult,          /* Write the result table here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnRow,                 /* Write the number of rows in the result here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnColumn,              /* Write the number of columns of result here */
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ             /* Write error messages here */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) || pazResult==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  ଯଦି( pnColumn ) *pnColumn = 0;
  ଯଦି( pnRow ) *pnRow = 0;
  ଯଦି( pz_Err_ସନ୍ଦେଶ ) *pz_Err_ସନ୍ଦେଶ = 0;
  res.z_Err_ସନ୍ଦେଶ = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(ଆକାର(ଅକ୍ଷର*)*res.nAlloc );
  ଯଦି( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_ସନ୍ଦେଶ);
  ପ୍ରତିଜ୍ଞା( ଆକାର(res.azResult[0])>= ଆକାର(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  ଯଦି( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    ଯଦି( res.z_Err_ସନ୍ଦେଶ ){
      ଯଦି( pz_Err_ସନ୍ଦେଶ ){
        sqlite3_free(*pz_Err_ସନ୍ଦେଶ);
        *pz_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s",res.z_Err_ସନ୍ଦେଶ);
      }
      sqlite3_free(res.z_Err_ସନ୍ଦେଶ);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    ଫେରନ୍ତୁ res.rc;
  }
  sqlite3_free(res.z_Err_ସନ୍ଦେଶ);
  ଯଦି( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    ଫେରନ୍ତୁ rc;
  }
  ଯଦି( res.nAlloc>res.nData ){
    ଅକ୍ଷର **azNew;
    azNew = sqlite3Realloc( res.azResult, ଆକାର(ଅକ୍ଷର*)*res.nData );
    ଯଦି( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  ଯଦି( pnColumn ) *pnColumn = res.nColumn;
  ଯଦି( pnRow ) *pnRow = res.nRow;
  ଫେରନ୍ତୁ rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
ଶୂନ୍ୟ sqlite3_free_table(
  ଅକ୍ଷର **azResult            /* Result returned from sqlite3_get_table() */
){
  ଯଦି( azResult ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
    azResult--;
    ପ୍ରତିଜ୍ଞା( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    ପାଇଁ(i=1; i<n; i++){ ଯଦି( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
