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
প্রকারলেখ কাঠামো Tab_ফলাফল {
  অক্ষর **az_ফলাফল;   /* Accumulated output */
  অক্ষর *z_Err_বার্তা;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 n_তথ্য;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  পূর্ণসংখ্যা rc;            /* Return code from sqlite3_exec() */
} Tab_ফলাফল;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
স্থির পূর্ণসংখ্যা sqlite3_get_table_cb(শূন্য *pArg, পূর্ণসংখ্যা nCol, অক্ষর **argv, অক্ষর **colv){
  Tab_ফলাফল *p = (Tab_ফলাফল*)pArg;  /* Result accumulator */
  পূর্ণসংখ্যা need;                         /* Slots needed in p->azResult[] */
  পূর্ণসংখ্যা i;                            /* Loop counter */
  অক্ষর *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  যদি( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }অন্যথা{
    need = nCol;
  }
  যদি( p->n_তথ্য + need > p->nAlloc ){
    অক্ষর **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_ফলাফল, আকার(অক্ষর*)*p->nAlloc );
    যদি( azNew==0 ) যাও malloc_failed;
    p->az_ফলাফল = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  যদি( p->nRow==0 ){
    p->nColumn = nCol;
    জন্য(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      যদি( z==0 ) যাও malloc_failed;
      p->az_ফলাফল[p->n_তথ্য++] = z;
    }
  }অন্যথা যদি( (পূর্ণসংখ্যা)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_বার্তা);
    p->z_Err_বার্তা = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    ফেরত 1;
  }

  /* Copy over the row data
  */
  যদি( argv!=0 ){
    জন্য(i=0; i<nCol; i++){
      যদি( argv[i]==0 ){
        z = 0;
      }অন্যথা{
        পূর্ণসংখ্যা n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        যদি( z==0 ) যাও malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->az_ফলাফল[p->n_তথ্য++] = z;
    }
    p->nRow++;
  }
  ফেরত 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  ফেরত 1;
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
পূর্ণসংখ্যা sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  ধ্রুবক অক্ষর *zSql,           /* The SQL to be executed */
  অক্ষর ***paz_ফলাফল,          /* Write the result table here */
  পূর্ণসংখ্যা *pnRow,                 /* Write the number of rows in the result here */
  পূর্ণসংখ্যা *pnColumn,              /* Write the number of columns of result here */
  অক্ষর **pz_Err_বার্তা             /* Write error messages here */
){
  পূর্ণসংখ্যা rc;
  Tab_ফলাফল res;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) || paz_ফলাফল==0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  *paz_ফলাফল = 0;
  যদি( pnColumn ) *pnColumn = 0;
  যদি( pnRow ) *pnRow = 0;
  যদি( pz_Err_বার্তা ) *pz_Err_বার্তা = 0;
  res.z_Err_বার্তা = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.n_তথ্য = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_ফলাফল = sqlite3_malloc64(আকার(অক্ষর*)*res.nAlloc );
  যদি( res.az_ফলাফল==0 ){
     db->errCode = SQLITE_NOMEM;
     ফেরত SQLITE_NOMEM_BKPT;
  }
  res.az_ফলাফল[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_বার্তা);
  প্রতিজ্ঞা( আকার(res.az_ফলাফল[0])>= আকার(res.n_তথ্য) );
  res.az_ফলাফল[0] = SQLITE_INT_TO_PTR(res.n_তথ্য);
  যদি( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.az_ফলাফল[1]);
    যদি( res.z_Err_বার্তা ){
      যদি( pz_Err_বার্তা ){
        sqlite3_free(*pz_Err_বার্তা);
        *pz_Err_বার্তা = sqlite3_mprintf("%s",res.z_Err_বার্তা);
      }
      sqlite3_free(res.z_Err_বার্তা);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    ফেরত res.rc;
  }
  sqlite3_free(res.z_Err_বার্তা);
  যদি( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.az_ফলাফল[1]);
    ফেরত rc;
  }
  যদি( res.nAlloc>res.n_তথ্য ){
    অক্ষর **azNew;
    azNew = sqlite3Realloc( res.az_ফলাফল, আকার(অক্ষর*)*res.n_তথ্য );
    যদি( azNew==0 ){
      sqlite3_free_table(&res.az_ফলাফল[1]);
      db->errCode = SQLITE_NOMEM;
      ফেরত SQLITE_NOMEM_BKPT;
    }
    res.az_ফলাফল = azNew;
  }
  *paz_ফলাফল = &res.az_ফলাফল[1];
  যদি( pnColumn ) *pnColumn = res.nColumn;
  যদি( pnRow ) *pnRow = res.nRow;
  ফেরত rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
শূন্য sqlite3_free_table(
  অক্ষর **az_ফলাফল            /* Result returned from sqlite3_get_table() */
){
  যদি( az_ফলাফল ){
    পূর্ণসংখ্যা i, n;
    az_ফলাফল--;
    প্রতিজ্ঞা( az_ফলাফল!=0 );
    n = SQLITE_PTR_TO_INT(az_ফলাফল[0]);
    জন্য(i=1; i<n; i++){ যদি( az_ফলাফল[i] ) sqlite3_free(az_ফলাফল[i]); }
    sqlite3_free(az_ফলাফল);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
