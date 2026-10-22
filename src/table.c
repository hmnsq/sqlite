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
#ضمن "sqliteInt.h"

#که_نه_تعریف_شوی SQLITE_OMIT_GET_جدول

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
تعريفنوع بنية Tab_پایله {
  حرف **az_پایله;   /* Accumulated output */
  حرف *z_Err_پیغام;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 n_لیکه;          /* Number of rows in the result */
  u32 n_ستنه;       /* Number of columns in the result */
  u32 n_معلومات;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  صحيح rc;            /* Return code from sqlite3_exec() */
} Tab_پایله;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ساكن صحيح sqlite3_get_جدول_cb(فراغ *pArg, صحيح n_ستون, حرف **argv, حرف **colv){
  Tab_پایله *p = (Tab_پایله*)pArg;  /* Result accumulator */
  صحيح need;                         /* Slots needed in p->azResult[] */
  صحيح i;                            /* Loop counter */
  حرف *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  إذا( p->n_لیکه==0 && argv!=0 ){
    need = n_ستون*2;
  }وإلا{
    need = n_ستون;
  }
  إذا( p->n_معلومات + need > p->nAlloc ){
    حرف **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_پایله, حجم(حرف*)*p->nAlloc );
    إذا( azNew==0 ) اذهب malloc_failed;
    p->az_پایله = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  إذا( p->n_لیکه==0 ){
    p->n_ستنه = n_ستون;
    لكل(i=0; i<n_ستون; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      إذا( z==0 ) اذهب malloc_failed;
      p->az_پایله[p->n_معلومات++] = z;
    }
  }وإلا إذا( (صحيح)p->n_ستنه!=n_ستون ){
    sqlite3_free(p->z_Err_پیغام);
    p->z_Err_پیغام = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    أرجع 1;
  }

  /* Copy over the row data
  */
  إذا( argv!=0 ){
    لكل(i=0; i<n_ستون; i++){
      إذا( argv[i]==0 ){
        z = 0;
      }وإلا{
        صحيح n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        إذا( z==0 ) اذهب malloc_failed;
        حافظه_نقل(z, argv[i], n);
      }
      p->az_پایله[p->n_معلومات++] = z;
    }
    p->n_لیکه++;
  }
  أرجع 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  أرجع 1;
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
صحيح sqlite3_get_جدول(
  sqlite3 *db,                /* The database on which the SQL executes */
  ثابت حرف *zSql,           /* The SQL to be executed */
  حرف ***paz_پایله,          /* Write the result table here */
  صحيح *pn_لیکه,                 /* Write the number of rows in the result here */
  صحيح *pn_ستنه,              /* Write the number of columns of result here */
  حرف **pz_Err_پیغام             /* Write error messages here */
){
  صحيح rc;
  Tab_پایله res;

#که_تعریف_شوی SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || paz_پایله==0 ) أرجع SQLITE_MISUSE_BKPT;
#د_که_پای
  *paz_پایله = 0;
  إذا( pn_ستنه ) *pn_ستنه = 0;
  إذا( pn_لیکه ) *pn_لیکه = 0;
  إذا( pz_Err_پیغام ) *pz_Err_پیغام = 0;
  res.z_Err_پیغام = 0;
  res.n_لیکه = 0;
  res.n_ستنه = 0;
  res.n_معلومات = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_پایله = sqlite3_malloc64(حجم(حرف*)*res.nAlloc );
  إذا( res.az_پایله==0 ){
     db->err_کوډ = SQLITE_NOMEM;
     أرجع SQLITE_NOMEM_BKPT;
  }
  res.az_پایله[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_جدول_cb, &res, pz_Err_پیغام);
  أكد( حجم(res.az_پایله[0])>= حجم(res.n_معلومات) );
  res.az_پایله[0] = SQLITE_INT_منزل_PTR(res.n_معلومات);
  إذا( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_جدول(&res.az_پایله[1]);
    إذا( res.z_Err_پیغام ){
      إذا( pz_Err_پیغام ){
        sqlite3_free(*pz_Err_پیغام);
        *pz_Err_پیغام = sqlite3_mprintf("%s",res.z_Err_پیغام);
      }
      sqlite3_free(res.z_Err_پیغام);
    }
    db->err_کوډ = res.rc;  /* Assume 32-bit assignment is atomic */
    أرجع res.rc;
  }
  sqlite3_free(res.z_Err_پیغام);
  إذا( rc!=SQLITE_OK ){
    sqlite3_free_جدول(&res.az_پایله[1]);
    أرجع rc;
  }
  إذا( res.nAlloc>res.n_معلومات ){
    حرف **azNew;
    azNew = sqlite3Realloc( res.az_پایله, حجم(حرف*)*res.n_معلومات );
    إذا( azNew==0 ){
      sqlite3_free_جدول(&res.az_پایله[1]);
      db->err_کوډ = SQLITE_NOMEM;
      أرجع SQLITE_NOMEM_BKPT;
    }
    res.az_پایله = azNew;
  }
  *paz_پایله = &res.az_پایله[1];
  إذا( pn_ستنه ) *pn_ستنه = res.n_ستنه;
  إذا( pn_لیکه ) *pn_لیکه = res.n_لیکه;
  أرجع rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
فراغ sqlite3_free_جدول(
  حرف **az_پایله            /* Result returned from sqlite3_get_table() */
){
  إذا( az_پایله ){
    صحيح i, n;
    az_پایله--;
    أكد( az_پایله!=0 );
    n = SQLITE_PTR_منزل_INT(az_پایله[0]);
    لكل(i=1; i<n; i++){ إذا( az_پایله[i] ) sqlite3_free(az_پایله[i]); }
    sqlite3_free(az_پایله);
  }
}

#د_که_پای /* SQLITE_OMIT_GET_TABLE */
