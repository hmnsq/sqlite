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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GET_ਸਾਰਣੀ

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Tab_ਨਤੀਜਾ {
  ਅੱਖਰ **az_ਨਤੀਜਾ;   /* Accumulated output */
  ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 n_ਪੰਕਤੀ;          /* Number of rows in the result */
  u32 n_ਕਾਲਮ;       /* Number of columns in the result */
  u32 n_ਅੰਕੜੇ;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  ਪੂਰਨ rc;            /* Return code from sqlite3_exec() */
} Tab_ਨਤੀਜਾ;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3_get_ਸਾਰਣੀ_cb(ਖਾਲੀ *pArg, ਪੂਰਨ n_ਖਾਨਾ, ਅੱਖਰ **argv, ਅੱਖਰ **colv){
  Tab_ਨਤੀਜਾ *p = (Tab_ਨਤੀਜਾ*)pArg;  /* Result accumulator */
  ਪੂਰਨ need;                         /* Slots needed in p->azResult[] */
  ਪੂਰਨ i;                            /* Loop counter */
  ਅੱਖਰ *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  ਜੇ( p->n_ਪੰਕਤੀ==0 && argv!=0 ){
    need = n_ਖਾਨਾ*2;
  }ਨਹੀਂਤਾਂ{
    need = n_ਖਾਨਾ;
  }
  ਜੇ( p->n_ਅੰਕੜੇ + need > p->nAlloc ){
    ਅੱਖਰ **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_ਨਤੀਜਾ, ਆਕਾਰ(ਅੱਖਰ*)*p->nAlloc );
    ਜੇ( azNew==0 ) ਜਾਓ malloc_failed;
    p->az_ਨਤੀਜਾ = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  ਜੇ( p->n_ਪੰਕਤੀ==0 ){
    p->n_ਕਾਲਮ = n_ਖਾਨਾ;
    ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      ਜੇ( z==0 ) ਜਾਓ malloc_failed;
      p->az_ਨਤੀਜਾ[p->n_ਅੰਕੜੇ++] = z;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( (ਪੂਰਨ)p->n_ਕਾਲਮ!=n_ਖਾਨਾ ){
    sqlite3_free(p->z_Err_ਸੁਨੇਹਾ);
    p->z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    ਵਾਪਸ 1;
  }

  /* Copy over the row data
  */
  ਜੇ( argv!=0 ){
    ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
      ਜੇ( argv[i]==0 ){
        z = 0;
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        ਜੇ( z==0 ) ਜਾਓ malloc_failed;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(z, argv[i], n);
      }
      p->az_ਨਤੀਜਾ[p->n_ਅੰਕੜੇ++] = z;
    }
    p->n_ਪੰਕਤੀ++;
  }
  ਵਾਪਸ 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  ਵਾਪਸ 1;
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
ਪੂਰਨ sqlite3_get_ਸਾਰਣੀ(
  sqlite3 *db,                /* The database on which the SQL executes */
  ਅਚਲ ਅੱਖਰ *zSql,           /* The SQL to be executed */
  ਅੱਖਰ ***paz_ਨਤੀਜਾ,          /* Write the result table here */
  ਪੂਰਨ *pn_ਪੰਕਤੀ,                 /* Write the number of rows in the result here */
  ਪੂਰਨ *pn_ਕਾਲਮ,              /* Write the number of columns of result here */
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ             /* Write error messages here */
){
  ਪੂਰਨ rc;
  Tab_ਨਤੀਜਾ res;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || paz_ਨਤੀਜਾ==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  *paz_ਨਤੀਜਾ = 0;
  ਜੇ( pn_ਕਾਲਮ ) *pn_ਕਾਲਮ = 0;
  ਜੇ( pn_ਪੰਕਤੀ ) *pn_ਪੰਕਤੀ = 0;
  ਜੇ( pz_Err_ਸੁਨੇਹਾ ) *pz_Err_ਸੁਨੇਹਾ = 0;
  res.z_Err_ਸੁਨੇਹਾ = 0;
  res.n_ਪੰਕਤੀ = 0;
  res.n_ਕਾਲਮ = 0;
  res.n_ਅੰਕੜੇ = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_ਨਤੀਜਾ = sqlite3_malloc64(ਆਕਾਰ(ਅੱਖਰ*)*res.nAlloc );
  ਜੇ( res.az_ਨਤੀਜਾ==0 ){
     db->err_ਕੋਡ = SQLITE_NOMEM;
     ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  res.az_ਨਤੀਜਾ[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_ਸਾਰਣੀ_cb, &res, pz_Err_ਸੁਨੇਹਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(res.az_ਨਤੀਜਾ[0])>= ਆਕਾਰ(res.n_ਅੰਕੜੇ) );
  res.az_ਨਤੀਜਾ[0] = SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(res.n_ਅੰਕੜੇ);
  ਜੇ( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_ਸਾਰਣੀ(&res.az_ਨਤੀਜਾ[1]);
    ਜੇ( res.z_Err_ਸੁਨੇਹਾ ){
      ਜੇ( pz_Err_ਸੁਨੇਹਾ ){
        sqlite3_free(*pz_Err_ਸੁਨੇਹਾ);
        *pz_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s",res.z_Err_ਸੁਨੇਹਾ);
      }
      sqlite3_free(res.z_Err_ਸੁਨੇਹਾ);
    }
    db->err_ਕੋਡ = res.rc;  /* Assume 32-bit assignment is atomic */
    ਵਾਪਸ res.rc;
  }
  sqlite3_free(res.z_Err_ਸੁਨੇਹਾ);
  ਜੇ( rc!=SQLITE_OK ){
    sqlite3_free_ਸਾਰਣੀ(&res.az_ਨਤੀਜਾ[1]);
    ਵਾਪਸ rc;
  }
  ਜੇ( res.nAlloc>res.n_ਅੰਕੜੇ ){
    ਅੱਖਰ **azNew;
    azNew = sqlite3Realloc( res.az_ਨਤੀਜਾ, ਆਕਾਰ(ਅੱਖਰ*)*res.n_ਅੰਕੜੇ );
    ਜੇ( azNew==0 ){
      sqlite3_free_ਸਾਰਣੀ(&res.az_ਨਤੀਜਾ[1]);
      db->err_ਕੋਡ = SQLITE_NOMEM;
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    res.az_ਨਤੀਜਾ = azNew;
  }
  *paz_ਨਤੀਜਾ = &res.az_ਨਤੀਜਾ[1];
  ਜੇ( pn_ਕਾਲਮ ) *pn_ਕਾਲਮ = res.n_ਕਾਲਮ;
  ਜੇ( pn_ਪੰਕਤੀ ) *pn_ਪੰਕਤੀ = res.n_ਪੰਕਤੀ;
  ਵਾਪਸ rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
ਖਾਲੀ sqlite3_free_ਸਾਰਣੀ(
  ਅੱਖਰ **az_ਨਤੀਜਾ            /* Result returned from sqlite3_get_table() */
){
  ਜੇ( az_ਨਤੀਜਾ ){
    ਪੂਰਨ i, n;
    az_ਨਤੀਜਾ--;
    ਪ੍ਰਤਿਜ੍ਞਾ( az_ਨਤੀਜਾ!=0 );
    n = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(az_ਨਤੀਜਾ[0]);
    ਲਈ(i=1; i<n; i++){ ਜੇ( az_ਨਤੀਜਾ[i] ) sqlite3_free(az_ਨਤੀਜਾ[i]); }
    sqlite3_free(az_ਨਤੀਜਾ);
  }
}

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_GET_TABLE */
