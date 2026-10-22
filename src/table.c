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
#અન્તર્ભાવ "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
પ્રકારવ્યાખ્યા સંરચના TabResult {
  અક્ષર **azResult;   /* Accumulated output */
  અક્ષર *z_Err_સંદેશ;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  પૂર્ણાંક rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
સ્થિર પૂર્ણાંક sqlite3_get_table_cb(શૂન્ય *pArg, પૂર્ણાંક nCol, અક્ષર **argv, અક્ષર **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  પૂર્ણાંક need;                         /* Slots needed in p->azResult[] */
  પૂર્ણાંક i;                            /* Loop counter */
  અક્ષર *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  જો( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }નહીંતર{
    need = nCol;
  }
  જો( p->nData + need > p->nAlloc ){
    અક્ષર **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, કદ(અક્ષર*)*p->nAlloc );
    જો( azNew==0 ) જાઓ malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  જો( p->nRow==0 ){
    p->nColumn = nCol;
    માટે(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      જો( z==0 ) જાઓ malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }નહીંતર જો( (પૂર્ણાંક)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_સંદેશ);
    p->z_Err_સંદેશ = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    પાછા 1;
  }

  /* Copy over the row data
  */
  જો( argv!=0 ){
    માટે(i=0; i<nCol; i++){
      જો( argv[i]==0 ){
        z = 0;
      }નહીંતર{
        પૂર્ણાંક n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        જો( z==0 ) જાઓ malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  પાછા 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  પાછા 1;
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
પૂર્ણાંક sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  અચલ અક્ષર *zSql,           /* The SQL to be executed */
  અક્ષર ***pazResult,          /* Write the result table here */
  પૂર્ણાંક *pnRow,                 /* Write the number of rows in the result here */
  પૂર્ણાંક *pnColumn,              /* Write the number of columns of result here */
  અક્ષર **pz_Err_સંદેશ             /* Write error messages here */
){
  પૂર્ણાંક rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) || pazResult==0 ) પાછા SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  જો( pnColumn ) *pnColumn = 0;
  જો( pnRow ) *pnRow = 0;
  જો( pz_Err_સંદેશ ) *pz_Err_સંદેશ = 0;
  res.z_Err_સંદેશ = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(કદ(અક્ષર*)*res.nAlloc );
  જો( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     પાછા SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_સંદેશ);
  પ્રતિજ્ઞા( કદ(res.azResult[0])>= કદ(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  જો( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    જો( res.z_Err_સંદેશ ){
      જો( pz_Err_સંદેશ ){
        sqlite3_free(*pz_Err_સંદેશ);
        *pz_Err_સંદેશ = sqlite3_mprintf("%s",res.z_Err_સંદેશ);
      }
      sqlite3_free(res.z_Err_સંદેશ);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    પાછા res.rc;
  }
  sqlite3_free(res.z_Err_સંદેશ);
  જો( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    પાછા rc;
  }
  જો( res.nAlloc>res.nData ){
    અક્ષર **azNew;
    azNew = sqlite3Realloc( res.azResult, કદ(અક્ષર*)*res.nData );
    જો( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      પાછા SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  જો( pnColumn ) *pnColumn = res.nColumn;
  જો( pnRow ) *pnRow = res.nRow;
  પાછા rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
શૂન્ય sqlite3_free_table(
  અક્ષર **azResult            /* Result returned from sqlite3_get_table() */
){
  જો( azResult ){
    પૂર્ણાંક i, n;
    azResult--;
    પ્રતિજ્ઞા( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    માટે(i=1; i<n; i++){ જો( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
