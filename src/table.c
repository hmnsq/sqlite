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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GET_ಕೋಷ್ಟಕ

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Tab_ಫಲಿತಾಂಶ {
  ಅಕ್ಷರ **az_ಫಲಿತಾಂಶ;   /* Accumulated output */
  ಅಕ್ಷರ *z_Err_ಸಂದೇಶ;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 n_ಅಡ್ಡಸಾಲು;          /* Number of rows in the result */
  u32 n_ಸ್ತಂಭ;       /* Number of columns in the result */
  u32 n_ದತ್ತಾಂಶ;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  ಪೂರ್ಣಾಂಕ rc;            /* Return code from sqlite3_exec() */
} Tab_ಫಲಿತಾಂಶ;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_get_ಕೋಷ್ಟಕ_cb(ಶೂನ್ಯ *pArg, ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು, ಅಕ್ಷರ **argv, ಅಕ್ಷರ **colv){
  Tab_ಫಲಿತಾಂಶ *p = (Tab_ಫಲಿತಾಂಶ*)pArg;  /* Result accumulator */
  ಪೂರ್ಣಾಂಕ need;                         /* Slots needed in p->azResult[] */
  ಪೂರ್ಣಾಂಕ i;                            /* Loop counter */
  ಅಕ್ಷರ *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  ಆದರೆ( p->n_ಅಡ್ಡಸಾಲು==0 && argv!=0 ){
    need = n_ಲಂಬಸಾಲು*2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    need = n_ಲಂಬಸಾಲು;
  }
  ಆದರೆ( p->n_ದತ್ತಾಂಶ + need > p->nAlloc ){
    ಅಕ್ಷರ **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_ಫಲಿತಾಂಶ, ಗಾತ್ರ(ಅಕ್ಷರ*)*p->nAlloc );
    ಆದರೆ( azNew==0 ) ಹೋಗು malloc_failed;
    p->az_ಫಲಿತಾಂಶ = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  ಆದರೆ( p->n_ಅಡ್ಡಸಾಲು==0 ){
    p->n_ಸ್ತಂಭ = n_ಲಂಬಸಾಲು;
    ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      ಆದರೆ( z==0 ) ಹೋಗು malloc_failed;
      p->az_ಫಲಿತಾಂಶ[p->n_ದತ್ತಾಂಶ++] = z;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (ಪೂರ್ಣಾಂಕ)p->n_ಸ್ತಂಭ!=n_ಲಂಬಸಾಲು ){
    sqlite3_free(p->z_Err_ಸಂದೇಶ);
    p->z_Err_ಸಂದೇಶ = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    ಹಿಂತಿರುಗಿ 1;
  }

  /* Copy over the row data
  */
  ಆದರೆ( argv!=0 ){
    ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
      ಆದರೆ( argv[i]==0 ){
        z = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        ಆದರೆ( z==0 ) ಹೋಗು malloc_failed;
        ಸ್ಮರಣೆ_ನಕಲು(z, argv[i], n);
      }
      p->az_ಫಲಿತಾಂಶ[p->n_ದತ್ತಾಂಶ++] = z;
    }
    p->n_ಅಡ್ಡಸಾಲು++;
  }
  ಹಿಂತಿರುಗಿ 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  ಹಿಂತಿರುಗಿ 1;
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
ಪೂರ್ಣಾಂಕ sqlite3_get_ಕೋಷ್ಟಕ(
  sqlite3 *db,                /* The database on which the SQL executes */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,           /* The SQL to be executed */
  ಅಕ್ಷರ ***paz_ಫಲಿತಾಂಶ,          /* Write the result table here */
  ಪೂರ್ಣಾಂಕ *pn_ಅಡ್ಡಸಾಲು,                 /* Write the number of rows in the result here */
  ಪೂರ್ಣಾಂಕ *pn_ಸ್ತಂಭ,              /* Write the number of columns of result here */
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ             /* Write error messages here */
){
  ಪೂರ್ಣಾಂಕ rc;
  Tab_ಫಲಿತಾಂಶ res;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || paz_ಫಲಿತಾಂಶ==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  *paz_ಫಲಿತಾಂಶ = 0;
  ಆದರೆ( pn_ಸ್ತಂಭ ) *pn_ಸ್ತಂಭ = 0;
  ಆದರೆ( pn_ಅಡ್ಡಸಾಲು ) *pn_ಅಡ್ಡಸಾಲು = 0;
  ಆದರೆ( pz_Err_ಸಂದೇಶ ) *pz_Err_ಸಂದೇಶ = 0;
  res.z_Err_ಸಂದೇಶ = 0;
  res.n_ಅಡ್ಡಸಾಲು = 0;
  res.n_ಸ್ತಂಭ = 0;
  res.n_ದತ್ತಾಂಶ = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_ಫಲಿತಾಂಶ = sqlite3_malloc64(ಗಾತ್ರ(ಅಕ್ಷರ*)*res.nAlloc );
  ಆದರೆ( res.az_ಫಲಿತಾಂಶ==0 ){
     db->err_ಸಂಕೇತ = SQLITE_NOMEM;
     ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  res.az_ಫಲಿತಾಂಶ[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_ಕೋಷ್ಟಕ_cb, &res, pz_Err_ಸಂದೇಶ);
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(res.az_ಫಲಿತಾಂಶ[0])>= ಗಾತ್ರ(res.n_ದತ್ತಾಂಶ) );
  res.az_ಫಲಿತಾಂಶ[0] = SQLITE_INT_ಗಮ್ಯ_PTR(res.n_ದತ್ತಾಂಶ);
  ಆದರೆ( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_ಕೋಷ್ಟಕ(&res.az_ಫಲಿತಾಂಶ[1]);
    ಆದರೆ( res.z_Err_ಸಂದೇಶ ){
      ಆದರೆ( pz_Err_ಸಂದೇಶ ){
        sqlite3_free(*pz_Err_ಸಂದೇಶ);
        *pz_Err_ಸಂದೇಶ = sqlite3_mprintf("%s",res.z_Err_ಸಂದೇಶ);
      }
      sqlite3_free(res.z_Err_ಸಂದೇಶ);
    }
    db->err_ಸಂಕೇತ = res.rc;  /* Assume 32-bit assignment is atomic */
    ಹಿಂತಿರುಗಿ res.rc;
  }
  sqlite3_free(res.z_Err_ಸಂದೇಶ);
  ಆದರೆ( rc!=SQLITE_OK ){
    sqlite3_free_ಕೋಷ್ಟಕ(&res.az_ಫಲಿತಾಂಶ[1]);
    ಹಿಂತಿರುಗಿ rc;
  }
  ಆದರೆ( res.nAlloc>res.n_ದತ್ತಾಂಶ ){
    ಅಕ್ಷರ **azNew;
    azNew = sqlite3Realloc( res.az_ಫಲಿತಾಂಶ, ಗಾತ್ರ(ಅಕ್ಷರ*)*res.n_ದತ್ತಾಂಶ );
    ಆದರೆ( azNew==0 ){
      sqlite3_free_ಕೋಷ್ಟಕ(&res.az_ಫಲಿತಾಂಶ[1]);
      db->err_ಸಂಕೇತ = SQLITE_NOMEM;
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    res.az_ಫಲಿತಾಂಶ = azNew;
  }
  *paz_ಫಲಿತಾಂಶ = &res.az_ಫಲಿತಾಂಶ[1];
  ಆದರೆ( pn_ಸ್ತಂಭ ) *pn_ಸ್ತಂಭ = res.n_ಸ್ತಂಭ;
  ಆದರೆ( pn_ಅಡ್ಡಸಾಲು ) *pn_ಅಡ್ಡಸಾಲು = res.n_ಅಡ್ಡಸಾಲು;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
ಶೂನ್ಯ sqlite3_free_ಕೋಷ್ಟಕ(
  ಅಕ್ಷರ **az_ಫಲಿತಾಂಶ            /* Result returned from sqlite3_get_table() */
){
  ಆದರೆ( az_ಫಲಿತಾಂಶ ){
    ಪೂರ್ಣಾಂಕ i, n;
    az_ಫಲಿತಾಂಶ--;
    ಪ್ರತಿಜ್ಞಾ( az_ಫಲಿತಾಂಶ!=0 );
    n = SQLITE_PTR_ಗಮ್ಯ_INT(az_ಫಲಿತಾಂಶ[0]);
    ಗಾಗಿ(i=1; i<n; i++){ ಆದರೆ( az_ಫಲಿತಾಂಶ[i] ) sqlite3_free(az_ಫಲಿತಾಂಶ[i]); }
    sqlite3_free(az_ಫಲಿತಾಂಶ);
  }
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_GET_TABLE */
