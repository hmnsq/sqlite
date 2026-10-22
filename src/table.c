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

#यदि_अपरिभाषितम् SQLITE_OMIT_GET_सारणी

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
प्रकारनिर्देश संरचना Tab_परिणाम {
  वर्ण **az_परिणाम;   /* Accumulated output */
  वर्ण *z_Err_सन्देशः;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 n_दत्त;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  पूर्णाङ्क rc;            /* Return code from sqlite3_exec() */
} Tab_परिणाम;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
स्थिर पूर्णाङ्क sqlite3_get_सारणी_cb(शून्य *pArg, पूर्णाङ्क nCol, वर्ण **argv, वर्ण **colv){
  Tab_परिणाम *p = (Tab_परिणाम*)pArg;  /* Result accumulator */
  पूर्णाङ्क need;                         /* Slots needed in p->azResult[] */
  पूर्णाङ्क i;                            /* Loop counter */
  वर्ण *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  यदि( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }अन्यथा{
    need = nCol;
  }
  यदि( p->n_दत्त + need > p->nAlloc ){
    वर्ण **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_परिणाम, परिमाण(वर्ण*)*p->nAlloc );
    यदि( azNew==0 ) गच्छ malloc_failed;
    p->az_परिणाम = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  यदि( p->nRow==0 ){
    p->nColumn = nCol;
    प्रति(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      यदि( z==0 ) गच्छ malloc_failed;
      p->az_परिणाम[p->n_दत्त++] = z;
    }
  }अन्यथा यदि( (पूर्णाङ्क)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_सन्देशः);
    p->z_Err_सन्देशः = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    प्रत्यागम 1;
  }

  /* Copy over the row data
  */
  यदि( argv!=0 ){
    प्रति(i=0; i<nCol; i++){
      यदि( argv[i]==0 ){
        z = 0;
      }अन्यथा{
        पूर्णाङ्क n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        यदि( z==0 ) गच्छ malloc_failed;
        स्मृतिं_प्रतिलिख(z, argv[i], n);
      }
      p->az_परिणाम[p->n_दत्त++] = z;
    }
    p->nRow++;
  }
  प्रत्यागम 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  प्रत्यागम 1;
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
पूर्णाङ्क sqlite3_get_सारणी(
  sqlite3 *db,                /* The database on which the SQL executes */
  ध्रुव वर्ण *zSql,           /* The SQL to be executed */
  वर्ण ***paz_परिणाम,          /* Write the result table here */
  पूर्णाङ्क *pnRow,                 /* Write the number of rows in the result here */
  पूर्णाङ्क *pnColumn,              /* Write the number of columns of result here */
  वर्ण **pz_Err_सन्देशः             /* Write error messages here */
){
  पूर्णाङ्क rc;
  Tab_परिणाम res;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(db) || paz_परिणाम==0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#यदि_अन्तः
  *paz_परिणाम = 0;
  यदि( pnColumn ) *pnColumn = 0;
  यदि( pnRow ) *pnRow = 0;
  यदि( pz_Err_सन्देशः ) *pz_Err_सन्देशः = 0;
  res.z_Err_सन्देशः = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.n_दत्त = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_परिणाम = sqlite3_malloc64(परिमाण(वर्ण*)*res.nAlloc );
  यदि( res.az_परिणाम==0 ){
     db->err_संहिता = SQLITE_NOMEM;
     प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  res.az_परिणाम[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_सारणी_cb, &res, pz_Err_सन्देशः);
  प्रतिज्ञा( परिमाण(res.az_परिणाम[0])>= परिमाण(res.n_दत्त) );
  res.az_परिणाम[0] = SQLITE_INT_गन्तव्यम्_PTR(res.n_दत्त);
  यदि( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_सारणी(&res.az_परिणाम[1]);
    यदि( res.z_Err_सन्देशः ){
      यदि( pz_Err_सन्देशः ){
        sqlite3_free(*pz_Err_सन्देशः);
        *pz_Err_सन्देशः = sqlite3_mprintf("%s",res.z_Err_सन्देशः);
      }
      sqlite3_free(res.z_Err_सन्देशः);
    }
    db->err_संहिता = res.rc;  /* Assume 32-bit assignment is atomic */
    प्रत्यागम res.rc;
  }
  sqlite3_free(res.z_Err_सन्देशः);
  यदि( rc!=SQLITE_OK ){
    sqlite3_free_सारणी(&res.az_परिणाम[1]);
    प्रत्यागम rc;
  }
  यदि( res.nAlloc>res.n_दत्त ){
    वर्ण **azNew;
    azNew = sqlite3Realloc( res.az_परिणाम, परिमाण(वर्ण*)*res.n_दत्त );
    यदि( azNew==0 ){
      sqlite3_free_सारणी(&res.az_परिणाम[1]);
      db->err_संहिता = SQLITE_NOMEM;
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    res.az_परिणाम = azNew;
  }
  *paz_परिणाम = &res.az_परिणाम[1];
  यदि( pnColumn ) *pnColumn = res.nColumn;
  यदि( pnRow ) *pnRow = res.nRow;
  प्रत्यागम rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
शून्य sqlite3_free_सारणी(
  वर्ण **az_परिणाम            /* Result returned from sqlite3_get_table() */
){
  यदि( az_परिणाम ){
    पूर्णाङ्क i, n;
    az_परिणाम--;
    प्रतिज्ञा( az_परिणाम!=0 );
    n = SQLITE_PTR_गन्तव्यम्_INT(az_परिणाम[0]);
    प्रति(i=1; i<n; i++){ यदि( az_परिणाम[i] ) sqlite3_free(az_परिणाम[i]); }
    sqlite3_free(az_परिणाम);
  }
}

#यदि_अन्तः /* SQLITE_OMIT_GET_TABLE */
