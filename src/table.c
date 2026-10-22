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

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_GET_جدول

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
تعريفنوع بنية Tab_نتيجو {
  حرف **az_نتيجو;   /* Accumulated output */
  حرف *z_Err_نياپو;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 n_صف;          /* Number of rows in the result */
  u32 n_ڪالم;       /* Number of columns in the result */
  u32 n_معلومات;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  صحيح rc;            /* Return code from sqlite3_exec() */
} Tab_نتيجو;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
ساكن صحيح sqlite3_get_جدول_cb(فراغ *pArg, صحيح n_کانو, حرف **argv, حرف **colv){
  Tab_نتيجو *p = (Tab_نتيجو*)pArg;  /* Result accumulator */
  صحيح need;                         /* Slots needed in p->azResult[] */
  صحيح i;                            /* Loop counter */
  حرف *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  إذا( p->n_صف==0 && argv!=0 ){
    need = n_کانو*2;
  }وإلا{
    need = n_کانو;
  }
  إذا( p->n_معلومات + need > p->nAlloc ){
    حرف **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_نتيجو, حجم(حرف*)*p->nAlloc );
    إذا( azNew==0 ) اذهب malloc_failed;
    p->az_نتيجو = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  إذا( p->n_صف==0 ){
    p->n_ڪالم = n_کانو;
    لكل(i=0; i<n_کانو; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      إذا( z==0 ) اذهب malloc_failed;
      p->az_نتيجو[p->n_معلومات++] = z;
    }
  }وإلا إذا( (صحيح)p->n_ڪالم!=n_کانو ){
    sqlite3_free(p->z_Err_نياپو);
    p->z_Err_نياپو = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    أرجع 1;
  }

  /* Copy over the row data
  */
  إذا( argv!=0 ){
    لكل(i=0; i<n_کانو; i++){
      إذا( argv[i]==0 ){
        z = 0;
      }وإلا{
        صحيح n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        إذا( z==0 ) اذهب malloc_failed;
        يادگيري_نقل(z, argv[i], n);
      }
      p->az_نتيجو[p->n_معلومات++] = z;
    }
    p->n_صف++;
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
  حرف ***paz_نتيجو,          /* Write the result table here */
  صحيح *pn_صف,                 /* Write the number of rows in the result here */
  صحيح *pn_ڪالم,              /* Write the number of columns of result here */
  حرف **pz_Err_نياپو             /* Write error messages here */
){
  صحيح rc;
  Tab_نتيجو res;

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || paz_نتيجو==0 ) أرجع SQLITE_MISUSE_BKPT;
#جيڪڏهن_پورو
  *paz_نتيجو = 0;
  إذا( pn_ڪالم ) *pn_ڪالم = 0;
  إذا( pn_صف ) *pn_صف = 0;
  إذا( pz_Err_نياپو ) *pz_Err_نياپو = 0;
  res.z_Err_نياپو = 0;
  res.n_صف = 0;
  res.n_ڪالم = 0;
  res.n_معلومات = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_نتيجو = sqlite3_malloc64(حجم(حرف*)*res.nAlloc );
  إذا( res.az_نتيجو==0 ){
     db->err_ڪوڊ = SQLITE_NOMEM;
     أرجع SQLITE_NOMEM_BKPT;
  }
  res.az_نتيجو[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_جدول_cb, &res, pz_Err_نياپو);
  أكد( حجم(res.az_نتيجو[0])>= حجم(res.n_معلومات) );
  res.az_نتيجو[0] = SQLITE_INT_منزل_PTR(res.n_معلومات);
  إذا( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_جدول(&res.az_نتيجو[1]);
    إذا( res.z_Err_نياپو ){
      إذا( pz_Err_نياپو ){
        sqlite3_free(*pz_Err_نياپو);
        *pz_Err_نياپو = sqlite3_mprintf("%s",res.z_Err_نياپو);
      }
      sqlite3_free(res.z_Err_نياپو);
    }
    db->err_ڪوڊ = res.rc;  /* Assume 32-bit assignment is atomic */
    أرجع res.rc;
  }
  sqlite3_free(res.z_Err_نياپو);
  إذا( rc!=SQLITE_OK ){
    sqlite3_free_جدول(&res.az_نتيجو[1]);
    أرجع rc;
  }
  إذا( res.nAlloc>res.n_معلومات ){
    حرف **azNew;
    azNew = sqlite3Realloc( res.az_نتيجو, حجم(حرف*)*res.n_معلومات );
    إذا( azNew==0 ){
      sqlite3_free_جدول(&res.az_نتيجو[1]);
      db->err_ڪوڊ = SQLITE_NOMEM;
      أرجع SQLITE_NOMEM_BKPT;
    }
    res.az_نتيجو = azNew;
  }
  *paz_نتيجو = &res.az_نتيجو[1];
  إذا( pn_ڪالم ) *pn_ڪالم = res.n_ڪالم;
  إذا( pn_صف ) *pn_صف = res.n_صف;
  أرجع rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
فراغ sqlite3_free_جدول(
  حرف **az_نتيجو            /* Result returned from sqlite3_get_table() */
){
  إذا( az_نتيجو ){
    صحيح i, n;
    az_نتيجو--;
    أكد( az_نتيجو!=0 );
    n = SQLITE_PTR_منزل_INT(az_نتيجو[0]);
    لكل(i=1; i<n; i++){ إذا( az_نتيجو[i] ) sqlite3_free(az_نتيجو[i]); }
    sqlite3_free(az_نتيجو);
  }
}

#جيڪڏهن_پورو /* SQLITE_OMIT_GET_TABLE */
