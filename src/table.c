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

#اگر_تعریف_نشده SQLITE_OMIT_GET_جدول

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
تعريفنوع بنية Tab_نتیجه {
  حرف **az_نتیجه;   /* Accumulated output */
  حرف *z_Err_پیام;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 n_ردیف;          /* Number of rows in the result */
  u32 n_ستونی;       /* Number of columns in the result */
  u32 n_داده;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  صحيح rc;            /* Return code from sqlite3_exec() */
} Tab_نتیجه;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ساكن صحيح sqlite3_get_جدول_cb(فراغ *pArg, صحيح n_ستون, حرف **argv, حرف **colv){
  Tab_نتیجه *p = (Tab_نتیجه*)pArg;  /* Result accumulator */
  صحيح need;                         /* Slots needed in p->azResult[] */
  صحيح i;                            /* Loop counter */
  حرف *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  إذا( p->n_ردیف==0 && argv!=0 ){
    need = n_ستون*2;
  }وإلا{
    need = n_ستون;
  }
  إذا( p->n_داده + need > p->nAlloc ){
    حرف **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_نتیجه, حجم(حرف*)*p->nAlloc );
    إذا( azNew==0 ) اذهب malloc_failed;
    p->az_نتیجه = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  إذا( p->n_ردیف==0 ){
    p->n_ستونی = n_ستون;
    لكل(i=0; i<n_ستون; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      إذا( z==0 ) اذهب malloc_failed;
      p->az_نتیجه[p->n_داده++] = z;
    }
  }وإلا إذا( (صحيح)p->n_ستونی!=n_ستون ){
    sqlite3_free(p->z_Err_پیام);
    p->z_Err_پیام = sqlite3_mprintf(
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
        کپی_حافظه(z, argv[i], n);
      }
      p->az_نتیجه[p->n_داده++] = z;
    }
    p->n_ردیف++;
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
  حرف ***paz_نتیجه,          /* Write the result table here */
  صحيح *pn_ردیف,                 /* Write the number of rows in the result here */
  صحيح *pn_ستونی,              /* Write the number of columns of result here */
  حرف **pz_Err_پیام             /* Write error messages here */
){
  صحيح rc;
  Tab_نتیجه res;

#اگر_تعریف_شده SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || paz_نتیجه==0 ) أرجع SQLITE_MISUSE_BKPT;
#پایان_اگر
  *paz_نتیجه = 0;
  إذا( pn_ستونی ) *pn_ستونی = 0;
  إذا( pn_ردیف ) *pn_ردیف = 0;
  إذا( pz_Err_پیام ) *pz_Err_پیام = 0;
  res.z_Err_پیام = 0;
  res.n_ردیف = 0;
  res.n_ستونی = 0;
  res.n_داده = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_نتیجه = sqlite3_malloc64(حجم(حرف*)*res.nAlloc );
  إذا( res.az_نتیجه==0 ){
     db->err_کد = SQLITE_NOMEM;
     أرجع SQLITE_NOMEM_BKPT;
  }
  res.az_نتیجه[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_جدول_cb, &res, pz_Err_پیام);
  أكد( حجم(res.az_نتیجه[0])>= حجم(res.n_داده) );
  res.az_نتیجه[0] = SQLITE_INT_مقصد_PTR(res.n_داده);
  إذا( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_جدول(&res.az_نتیجه[1]);
    إذا( res.z_Err_پیام ){
      إذا( pz_Err_پیام ){
        sqlite3_free(*pz_Err_پیام);
        *pz_Err_پیام = sqlite3_mprintf("%s",res.z_Err_پیام);
      }
      sqlite3_free(res.z_Err_پیام);
    }
    db->err_کد = res.rc;  /* Assume 32-bit assignment is atomic */
    أرجع res.rc;
  }
  sqlite3_free(res.z_Err_پیام);
  إذا( rc!=SQLITE_OK ){
    sqlite3_free_جدول(&res.az_نتیجه[1]);
    أرجع rc;
  }
  إذا( res.nAlloc>res.n_داده ){
    حرف **azNew;
    azNew = sqlite3Realloc( res.az_نتیجه, حجم(حرف*)*res.n_داده );
    إذا( azNew==0 ){
      sqlite3_free_جدول(&res.az_نتیجه[1]);
      db->err_کد = SQLITE_NOMEM;
      أرجع SQLITE_NOMEM_BKPT;
    }
    res.az_نتیجه = azNew;
  }
  *paz_نتیجه = &res.az_نتیجه[1];
  إذا( pn_ستونی ) *pn_ستونی = res.n_ستونی;
  إذا( pn_ردیف ) *pn_ردیف = res.n_ردیف;
  أرجع rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
فراغ sqlite3_free_جدول(
  حرف **az_نتیجه            /* Result returned from sqlite3_get_table() */
){
  إذا( az_نتیجه ){
    صحيح i, n;
    az_نتیجه--;
    أكد( az_نتیجه!=0 );
    n = SQLITE_PTR_مقصد_INT(az_نتیجه[0]);
    لكل(i=1; i<n; i++){ إذا( az_نتیجه[i] ) sqlite3_free(az_نتیجه[i]); }
    sqlite3_free(az_نتیجه);
  }
}

#پایان_اگر /* SQLITE_OMIT_GET_TABLE */
