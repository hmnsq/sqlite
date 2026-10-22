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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TabResult {
  ವರ್ಣ **azResult;   /* Accumulated output */
  ವರ್ಣ *z_Err_ಸಂದೇಶ;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  ಪೂರ್ಣಾಙ್ಕ rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3_get_table_cb(ಶೂನ್ಯ *pArg, ಪೂರ್ಣಾಙ್ಕ nCol, ವರ್ಣ **argv, ವರ್ಣ **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  ಪೂರ್ಣಾಙ್ಕ need;                         /* Slots needed in p->azResult[] */
  ಪೂರ್ಣಾಙ್ಕ i;                            /* Loop counter */
  ವರ್ಣ *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  ಯದಿ( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }ಅನ್ಯಥಾ{
    need = nCol;
  }
  ಯದಿ( p->nData + need > p->nAlloc ){
    ವರ್ಣ **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, ಪರಿಮಾಣ(ವರ್ಣ*)*p->nAlloc );
    ಯದಿ( azNew==0 ) ಗಚ್ಛ malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  ಯದಿ( p->nRow==0 ){
    p->nColumn = nCol;
    ಪ್ರತಿ(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      ಯದಿ( z==0 ) ಗಚ್ಛ malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }ಅನ್ಯಥಾ ಯದಿ( (ಪೂರ್ಣಾಙ್ಕ)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_ಸಂದೇಶ);
    p->z_Err_ಸಂದೇಶ = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    ಪ್ರತ್ಯಾಗಮ 1;
  }

  /* Copy over the row data
  */
  ಯದಿ( argv!=0 ){
    ಪ್ರತಿ(i=0; i<nCol; i++){
      ಯದಿ( argv[i]==0 ){
        z = 0;
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        ಯದಿ( z==0 ) ಗಚ್ಛ malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  ಪ್ರತ್ಯಾಗಮ 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  ಪ್ರತ್ಯಾಗಮ 1;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  ಧ್ರುವ ವರ್ಣ *zSql,           /* The SQL to be executed */
  ವರ್ಣ ***pazResult,          /* Write the result table here */
  ಪೂರ್ಣಾಙ್ಕ *pnRow,                 /* Write the number of rows in the result here */
  ಪೂರ್ಣಾಙ್ಕ *pnColumn,              /* Write the number of columns of result here */
  ವರ್ಣ **pz_Err_ಸಂದೇಶ             /* Write error messages here */
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( !sqlite3SafetyCheckOk(db) || pazResult==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  ಯದಿ( pnColumn ) *pnColumn = 0;
  ಯದಿ( pnRow ) *pnRow = 0;
  ಯದಿ( pz_Err_ಸಂದೇಶ ) *pz_Err_ಸಂದೇಶ = 0;
  res.z_Err_ಸಂದೇಶ = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(ಪರಿಮಾಣ(ವರ್ಣ*)*res.nAlloc );
  ಯದಿ( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_ಸಂದೇಶ);
  ಪ್ರತಿಜ್ಞಾ( ಪರಿಮಾಣ(res.azResult[0])>= ಪರಿಮಾಣ(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  ಯದಿ( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    ಯದಿ( res.z_Err_ಸಂದೇಶ ){
      ಯದಿ( pz_Err_ಸಂದೇಶ ){
        sqlite3_free(*pz_Err_ಸಂದೇಶ);
        *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("%s",res.z_Err_ಸಂದೇಶ);
      }
      sqlite3_free(res.z_Err_ಸಂದೇಶ);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    ಪ್ರತ್ಯಾಗಮ res.rc;
  }
  sqlite3_free(res.z_Err_ಸಂದೇಶ);
  ಯದಿ( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    ಪ್ರತ್ಯಾಗಮ rc;
  }
  ಯದಿ( res.nAlloc>res.nData ){
    ವರ್ಣ **azNew;
    azNew = sqlite3Realloc( res.azResult, ಪರಿಮಾಣ(ವರ್ಣ*)*res.nData );
    ಯದಿ( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  ಯದಿ( pnColumn ) *pnColumn = res.nColumn;
  ಯದಿ( pnRow ) *pnRow = res.nRow;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
ಶೂನ್ಯ sqlite3_free_table(
  ವರ್ಣ **azResult            /* Result returned from sqlite3_get_table() */
){
  ಯದಿ( azResult ){
    ಪೂರ್ಣಾಙ್ಕ i, n;
    azResult--;
    ಪ್ರತಿಜ್ಞಾ( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    ಪ್ರತಿ(i=1; i<n; i++){ ಯದಿ( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
