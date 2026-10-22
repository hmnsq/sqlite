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
#شامل "sqliteInt.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GET_جدول

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
قسمتعریف ساخت Tab_نتیجہ {
  حرف **az_نتیجہ;   /* Accumulated output */
  حرف *z_Err_پیغام;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 n_صف;          /* Number of rows in the result */
  u32 n_عمود;       /* Number of columns in the result */
  u32 n_ڈیٹا;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  صحیح rc;            /* Return code from sqlite3_exec() */
} Tab_نتیجہ;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ساکن صحیح sqlite3_get_جدول_cb(خالی *pArg, صحیح n_ستون, حرف **argv, حرف **colv){
  Tab_نتیجہ *p = (Tab_نتیجہ*)pArg;  /* Result accumulator */
  صحیح need;                         /* Slots needed in p->azResult[] */
  صحیح i;                            /* Loop counter */
  حرف *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  اگر( p->n_صف==0 && argv!=0 ){
    need = n_ستون*2;
  }ورنہ{
    need = n_ستون;
  }
  اگر( p->n_ڈیٹا + need > p->nAlloc ){
    حرف **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_نتیجہ, حجم(حرف*)*p->nAlloc );
    اگر( azNew==0 ) جاؤ malloc_failed;
    p->az_نتیجہ = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  اگر( p->n_صف==0 ){
    p->n_عمود = n_ستون;
    برائے(i=0; i<n_ستون; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      اگر( z==0 ) جاؤ malloc_failed;
      p->az_نتیجہ[p->n_ڈیٹا++] = z;
    }
  }ورنہ اگر( (صحیح)p->n_عمود!=n_ستون ){
    sqlite3_free(p->z_Err_پیغام);
    p->z_Err_پیغام = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    واپس 1;
  }

  /* Copy over the row data
  */
  اگر( argv!=0 ){
    برائے(i=0; i<n_ستون; i++){
      اگر( argv[i]==0 ){
        z = 0;
      }ورنہ{
        صحیح n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        اگر( z==0 ) جاؤ malloc_failed;
        حافظہ_نقل(z, argv[i], n);
      }
      p->az_نتیجہ[p->n_ڈیٹا++] = z;
    }
    p->n_صف++;
  }
  واپس 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  واپس 1;
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
صحیح sqlite3_get_جدول(
  sqlite3 *db,                /* The database on which the SQL executes */
  ثابت حرف *zSql,           /* The SQL to be executed */
  حرف ***paz_نتیجہ,          /* Write the result table here */
  صحیح *pn_صف,                 /* Write the number of rows in the result here */
  صحیح *pn_عمود,              /* Write the number of columns of result here */
  حرف **pz_Err_پیغام             /* Write error messages here */
){
  صحیح rc;
  Tab_نتیجہ res;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || paz_نتیجہ==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  *paz_نتیجہ = 0;
  اگر( pn_عمود ) *pn_عمود = 0;
  اگر( pn_صف ) *pn_صف = 0;
  اگر( pz_Err_پیغام ) *pz_Err_پیغام = 0;
  res.z_Err_پیغام = 0;
  res.n_صف = 0;
  res.n_عمود = 0;
  res.n_ڈیٹا = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_نتیجہ = sqlite3_malloc64(حجم(حرف*)*res.nAlloc );
  اگر( res.az_نتیجہ==0 ){
     db->err_رمز = SQLITE_NOMEM;
     واپس SQLITE_NOMEM_BKPT;
  }
  res.az_نتیجہ[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_جدول_cb, &res, pz_Err_پیغام);
  أكد( حجم(res.az_نتیجہ[0])>= حجم(res.n_ڈیٹا) );
  res.az_نتیجہ[0] = SQLITE_INT_مقصد_PTR(res.n_ڈیٹا);
  اگر( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_جدول(&res.az_نتیجہ[1]);
    اگر( res.z_Err_پیغام ){
      اگر( pz_Err_پیغام ){
        sqlite3_free(*pz_Err_پیغام);
        *pz_Err_پیغام = sqlite3_mprintf("%s",res.z_Err_پیغام);
      }
      sqlite3_free(res.z_Err_پیغام);
    }
    db->err_رمز = res.rc;  /* Assume 32-bit assignment is atomic */
    واپس res.rc;
  }
  sqlite3_free(res.z_Err_پیغام);
  اگر( rc!=SQLITE_OK ){
    sqlite3_free_جدول(&res.az_نتیجہ[1]);
    واپس rc;
  }
  اگر( res.nAlloc>res.n_ڈیٹا ){
    حرف **azNew;
    azNew = sqlite3Realloc( res.az_نتیجہ, حجم(حرف*)*res.n_ڈیٹا );
    اگر( azNew==0 ){
      sqlite3_free_جدول(&res.az_نتیجہ[1]);
      db->err_رمز = SQLITE_NOMEM;
      واپس SQLITE_NOMEM_BKPT;
    }
    res.az_نتیجہ = azNew;
  }
  *paz_نتیجہ = &res.az_نتیجہ[1];
  اگر( pn_عمود ) *pn_عمود = res.n_عمود;
  اگر( pn_صف ) *pn_صف = res.n_صف;
  واپس rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
خالی sqlite3_free_جدول(
  حرف **az_نتیجہ            /* Result returned from sqlite3_get_table() */
){
  اگر( az_نتیجہ ){
    صحیح i, n;
    az_نتیجہ--;
    أكد( az_نتیجہ!=0 );
    n = SQLITE_PTR_مقصد_INT(az_نتیجہ[0]);
    برائے(i=1; i<n; i++){ اگر( az_نتیجہ[i] ) sqlite3_free(az_نتیجہ[i]); }
    sqlite3_free(az_نتیجہ);
  }
}

#اگر_ختم /* SQLITE_OMIT_GET_TABLE */
