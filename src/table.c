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
#अन्तर्भाव "sqliteInt.h"

#ifndef SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
प्रकारनिर्देश संरचना TabResult {
  वर्ण **azResult;   /* Accumulated output */
  वर्ण *z_Err_सन्देशः;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 nData;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  पूर्णाङ्क rc;            /* Return code from sqlite3_exec() */
} TabResult;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
स्थिर पूर्णाङ्क sqlite3_get_table_cb(शून्य *pArg, पूर्णाङ्क nCol, वर्ण **argv, वर्ण **colv){
  TabResult *p = (TabResult*)pArg;  /* Result accumulator */
  पूर्णाङ्क need;                         /* Slots needed in p->azResult[] */
  पूर्णाङ्क i;                            /* Loop counter */
  वर्ण *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  सचे( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }अञ्ञथा{
    need = nCol;
  }
  सचे( p->nData + need > p->nAlloc ){
    वर्ण **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->azResult, परिमाण(वर्ण*)*p->nAlloc );
    सचे( azNew==0 ) गच्छ malloc_failed;
    p->azResult = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  सचे( p->nRow==0 ){
    p->nColumn = nCol;
    अत्थाय(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      सचे( z==0 ) गच्छ malloc_failed;
      p->azResult[p->nData++] = z;
    }
  }अञ्ञथा सचे( (पूर्णाङ्क)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_सन्देशः);
    p->z_Err_सन्देशः = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    पटिनिवत्त 1;
  }

  /* Copy over the row data
  */
  सचे( argv!=0 ){
    अत्थाय(i=0; i<nCol; i++){
      सचे( argv[i]==0 ){
        z = 0;
      }अञ्ञथा{
        पूर्णाङ्क n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        सचे( z==0 ) गच्छ malloc_failed;
        memcpy(z, argv[i], n);
      }
      p->azResult[p->nData++] = z;
    }
    p->nRow++;
  }
  पटिनिवत्त 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  पटिनिवत्त 1;
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
पूर्णाङ्क sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  ध्रुव वर्ण *zSql,           /* The SQL to be executed */
  वर्ण ***pazResult,          /* Write the result table here */
  पूर्णाङ्क *pnRow,                 /* Write the number of rows in the result here */
  पूर्णाङ्क *pnColumn,              /* Write the number of columns of result here */
  वर्ण **pz_Err_सन्देशः             /* Write error messages here */
){
  पूर्णाङ्क rc;
  TabResult res;

#ifdef SQLITE_ENABLE_API_ARMOR
  सचे( !sqlite3SafetyCheckOk(db) || pazResult==0 ) पटिनिवत्त SQLITE_MISUSE_BKPT;
#endif
  *pazResult = 0;
  सचे( pnColumn ) *pnColumn = 0;
  सचे( pnRow ) *pnRow = 0;
  सचे( pz_Err_सन्देशः ) *pz_Err_सन्देशः = 0;
  res.z_Err_सन्देशः = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.nData = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.azResult = sqlite3_malloc64(परिमाण(वर्ण*)*res.nAlloc );
  सचे( res.azResult==0 ){
     db->errCode = SQLITE_NOMEM;
     पटिनिवत्त SQLITE_NOMEM_BKPT;
  }
  res.azResult[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_सन्देशः);
  प्रतिज्ञा( परिमाण(res.azResult[0])>= परिमाण(res.nData) );
  res.azResult[0] = SQLITE_INT_TO_PTR(res.nData);
  सचे( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.azResult[1]);
    सचे( res.z_Err_सन्देशः ){
      सचे( pz_Err_सन्देशः ){
        sqlite3_free(*pz_Err_सन्देशः);
        *pz_Err_सन्देशः = sqlite3_mprintf("%s",res.z_Err_सन्देशः);
      }
      sqlite3_free(res.z_Err_सन्देशः);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    पटिनिवत्त res.rc;
  }
  sqlite3_free(res.z_Err_सन्देशः);
  सचे( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.azResult[1]);
    पटिनिवत्त rc;
  }
  सचे( res.nAlloc>res.nData ){
    वर्ण **azNew;
    azNew = sqlite3Realloc( res.azResult, परिमाण(वर्ण*)*res.nData );
    सचे( azNew==0 ){
      sqlite3_free_table(&res.azResult[1]);
      db->errCode = SQLITE_NOMEM;
      पटिनिवत्त SQLITE_NOMEM_BKPT;
    }
    res.azResult = azNew;
  }
  *pazResult = &res.azResult[1];
  सचे( pnColumn ) *pnColumn = res.nColumn;
  सचे( pnRow ) *pnRow = res.nRow;
  पटिनिवत्त rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
शून्य sqlite3_free_table(
  वर्ण **azResult            /* Result returned from sqlite3_get_table() */
){
  सचे( azResult ){
    पूर्णाङ्क i, n;
    azResult--;
    प्रतिज्ञा( azResult!=0 );
    n = SQLITE_PTR_TO_INT(azResult[0]);
    अत्थाय(i=1; i<n; i++){ सचे( azResult[i] ) sqlite3_free(azResult[i]); }
    sqlite3_free(azResult);
  }
}

#endif /* SQLITE_OMIT_GET_TABLE */
