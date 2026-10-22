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
#समावेश "sqliteInt.h"

#अगर_अघोषित SQLITE_OMIT_GET_TABLE

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
प्रकार काष्ठा Tab_परिणाम {
  अक्षर **az_परिणाम;   /* Accumulated output */
  अक्षर *z_Err_संदेश;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 nRow;          /* Number of rows in the result */
  u32 nColumn;       /* Number of columns in the result */
  u32 n_तथ्य;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  पूर्णांक rc;            /* Return code from sqlite3_exec() */
} Tab_परिणाम;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
अटल पूर्णांक sqlite3_get_table_cb(व्योम *pArg, पूर्णांक nCol, अक्षर **argv, अक्षर **colv){
  Tab_परिणाम *p = (Tab_परिणाम*)pArg;  /* Result accumulator */
  पूर्णांक need;                         /* Slots needed in p->azResult[] */
  पूर्णांक i;                            /* Loop counter */
  अक्षर *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  अगर( p->nRow==0 && argv!=0 ){
    need = nCol*2;
  }अन्यथा{
    need = nCol;
  }
  अगर( p->n_तथ्य + need > p->nAlloc ){
    अक्षर **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_परिणाम, माप(अक्षर*)*p->nAlloc );
    अगर( azNew==0 ) जाओ malloc_failed;
    p->az_परिणाम = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  अगर( p->nRow==0 ){
    p->nColumn = nCol;
    क्रम(i=0; i<nCol; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      अगर( z==0 ) जाओ malloc_failed;
      p->az_परिणाम[p->n_तथ्य++] = z;
    }
  }अन्यथा अगर( (पूर्णांक)p->nColumn!=nCol ){
    sqlite3_free(p->z_Err_संदेश);
    p->z_Err_संदेश = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    वापस 1;
  }

  /* Copy over the row data
  */
  अगर( argv!=0 ){
    क्रम(i=0; i<nCol; i++){
      अगर( argv[i]==0 ){
        z = 0;
      }अन्यथा{
        पूर्णांक n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        अगर( z==0 ) जाओ malloc_failed;
        स_नकल(z, argv[i], n);
      }
      p->az_परिणाम[p->n_तथ्य++] = z;
    }
    p->nRow++;
  }
  वापस 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  वापस 1;
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
पूर्णांक sqlite3_get_table(
  sqlite3 *db,                /* The database on which the SQL executes */
  स्थिर अक्षर *zSql,           /* The SQL to be executed */
  अक्षर ***paz_परिणाम,          /* Write the result table here */
  पूर्णांक *pnRow,                 /* Write the number of rows in the result here */
  पूर्णांक *pnColumn,              /* Write the number of columns of result here */
  अक्षर **pz_Err_संदेश             /* Write error messages here */
){
  पूर्णांक rc;
  Tab_परिणाम res;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || paz_परिणाम==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  *paz_परिणाम = 0;
  अगर( pnColumn ) *pnColumn = 0;
  अगर( pnRow ) *pnRow = 0;
  अगर( pz_Err_संदेश ) *pz_Err_संदेश = 0;
  res.z_Err_संदेश = 0;
  res.nRow = 0;
  res.nColumn = 0;
  res.n_तथ्य = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_परिणाम = sqlite3_malloc64(माप(अक्षर*)*res.nAlloc );
  अगर( res.az_परिणाम==0 ){
     db->errCode = SQLITE_NOMEM;
     वापस SQLITE_NOMEM_BKPT;
  }
  res.az_परिणाम[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_table_cb, &res, pz_Err_संदेश);
  निश्चित( माप(res.az_परिणाम[0])>= माप(res.n_तथ्य) );
  res.az_परिणाम[0] = SQLITE_INT_TO_PTR(res.n_तथ्य);
  अगर( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_table(&res.az_परिणाम[1]);
    अगर( res.z_Err_संदेश ){
      अगर( pz_Err_संदेश ){
        sqlite3_free(*pz_Err_संदेश);
        *pz_Err_संदेश = sqlite3_mprintf("%s",res.z_Err_संदेश);
      }
      sqlite3_free(res.z_Err_संदेश);
    }
    db->errCode = res.rc;  /* Assume 32-bit assignment is atomic */
    वापस res.rc;
  }
  sqlite3_free(res.z_Err_संदेश);
  अगर( rc!=SQLITE_OK ){
    sqlite3_free_table(&res.az_परिणाम[1]);
    वापस rc;
  }
  अगर( res.nAlloc>res.n_तथ्य ){
    अक्षर **azNew;
    azNew = sqlite3Realloc( res.az_परिणाम, माप(अक्षर*)*res.n_तथ्य );
    अगर( azNew==0 ){
      sqlite3_free_table(&res.az_परिणाम[1]);
      db->errCode = SQLITE_NOMEM;
      वापस SQLITE_NOMEM_BKPT;
    }
    res.az_परिणाम = azNew;
  }
  *paz_परिणाम = &res.az_परिणाम[1];
  अगर( pnColumn ) *pnColumn = res.nColumn;
  अगर( pnRow ) *pnRow = res.nRow;
  वापस rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
व्योम sqlite3_free_table(
  अक्षर **az_परिणाम            /* Result returned from sqlite3_get_table() */
){
  अगर( az_परिणाम ){
    पूर्णांक i, n;
    az_परिणाम--;
    निश्चित( az_परिणाम!=0 );
    n = SQLITE_PTR_TO_INT(az_परिणाम[0]);
    क्रम(i=1; i<n; i++){ अगर( az_परिणाम[i] ) sqlite3_free(az_परिणाम[i]); }
    sqlite3_free(az_परिणाम);
  }
}

#पूर्ण_अगर /* SQLITE_OMIT_GET_TABLE */
