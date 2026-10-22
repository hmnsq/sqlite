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
#అన్తర్భావ "sqliteInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_GET_పట్టిక

/*
** This structure is used to pass data from sqlite3_get_table() through
** to the callback function is uses to build the result.
*/
రకనిర్వచనం నిర్మాణం Tab_ఫలితం {
  అక్షరం **az_ఫలితం;   /* Accumulated output */
  అక్షరం *z_Err_సందేశం;     /* Error message text, if an error occurs */
  u32 nAlloc;        /* Slots allocated for azResult[] */
  u32 n_అడ్డువరుస;          /* Number of rows in the result */
  u32 n_స్తంభం;       /* Number of columns in the result */
  u32 n_దత్తాంశం;         /* Slots used in azResult[].  (nRow+1)*nColumn */
  పూర్ణాంకం rc;            /* Return code from sqlite3_exec() */
} Tab_ఫలితం;

/*
** This routine is called once for each row in the result table.  Its job
** is to fill in the TabResult structure appropriately, allocating new
** memory as necessary.
*/
స్థిర పూర్ణాంకం sqlite3_get_పట్టిక_cb(శూన్య *pArg, పూర్ణాంకం n_నిలువు, అక్షరం **argv, అక్షరం **colv){
  Tab_ఫలితం *p = (Tab_ఫలితం*)pArg;  /* Result accumulator */
  పూర్ణాంకం need;                         /* Slots needed in p->azResult[] */
  పూర్ణాంకం i;                            /* Loop counter */
  అక్షరం *z;                          /* A single column of result */

  /* Make sure there is enough space in p->azResult to hold everything
  ** we need to remember from this invocation of the callback.
  */
  ఒకవేళ( p->n_అడ్డువరుస==0 && argv!=0 ){
    need = n_నిలువు*2;
  }లేకపోతే{
    need = n_నిలువు;
  }
  ఒకవేళ( p->n_దత్తాంశం + need > p->nAlloc ){
    అక్షరం **azNew;
    p->nAlloc = p->nAlloc*2 + need;
    azNew = sqlite3Realloc( p->az_ఫలితం, పరిమాణం(అక్షరం*)*p->nAlloc );
    ఒకవేళ( azNew==0 ) వెళ్ళు malloc_failed;
    p->az_ఫలితం = azNew;
  }

  /* If this is the first row, then generate an extra row containing
  ** the names of all columns.
  */
  ఒకవేళ( p->n_అడ్డువరుస==0 ){
    p->n_స్తంభం = n_నిలువు;
    కొరకు(i=0; i<n_నిలువు; i++){
      z = sqlite3_mprintf("%s", colv[i]);
      ఒకవేళ( z==0 ) వెళ్ళు malloc_failed;
      p->az_ఫలితం[p->n_దత్తాంశం++] = z;
    }
  }లేకపోతే ఒకవేళ( (పూర్ణాంకం)p->n_స్తంభం!=n_నిలువు ){
    sqlite3_free(p->z_Err_సందేశం);
    p->z_Err_సందేశం = sqlite3_mprintf(
       "sqlite3_get_table() called with two or more incompatible queries"
    );
    p->rc = SQLITE_ERROR;
    తిరిగి 1;
  }

  /* Copy over the row data
  */
  ఒకవేళ( argv!=0 ){
    కొరకు(i=0; i<n_నిలువు; i++){
      ఒకవేళ( argv[i]==0 ){
        z = 0;
      }లేకపోతే{
        పూర్ణాంకం n = sqlite3Strlen30(argv[i])+1;
        z = sqlite3_malloc64( n );
        ఒకవేళ( z==0 ) వెళ్ళు malloc_failed;
        జ్ఞాపకం_నకలు(z, argv[i], n);
      }
      p->az_ఫలితం[p->n_దత్తాంశం++] = z;
    }
    p->n_అడ్డువరుస++;
  }
  తిరిగి 0;

malloc_failed:
  p->rc = SQLITE_NOMEM_BKPT;
  తిరిగి 1;
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
పూర్ణాంకం sqlite3_get_పట్టిక(
  sqlite3 *db,                /* The database on which the SQL executes */
  స్థిరరాశి అక్షరం *zSql,           /* The SQL to be executed */
  అక్షరం ***paz_ఫలితం,          /* Write the result table here */
  పూర్ణాంకం *pn_అడ్డువరుస,                 /* Write the number of rows in the result here */
  పూర్ణాంకం *pn_స్తంభం,              /* Write the number of columns of result here */
  అక్షరం **pz_Err_సందేశం             /* Write error messages here */
){
  పూర్ణాంకం rc;
  Tab_ఫలితం res;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || paz_ఫలితం==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  *paz_ఫలితం = 0;
  ఒకవేళ( pn_స్తంభం ) *pn_స్తంభం = 0;
  ఒకవేళ( pn_అడ్డువరుస ) *pn_అడ్డువరుస = 0;
  ఒకవేళ( pz_Err_సందేశం ) *pz_Err_సందేశం = 0;
  res.z_Err_సందేశం = 0;
  res.n_అడ్డువరుస = 0;
  res.n_స్తంభం = 0;
  res.n_దత్తాంశం = 1;
  res.nAlloc = 20;
  res.rc = SQLITE_OK;
  res.az_ఫలితం = sqlite3_malloc64(పరిమాణం(అక్షరం*)*res.nAlloc );
  ఒకవేళ( res.az_ఫలితం==0 ){
     db->err_సంకేతం = SQLITE_NOMEM;
     తిరిగి SQLITE_NOMEM_BKPT;
  }
  res.az_ఫలితం[0] = 0;
  rc = sqlite3_exec(db, zSql, sqlite3_get_పట్టిక_cb, &res, pz_Err_సందేశం);
  ప్రతిజ్ఞా( పరిమాణం(res.az_ఫలితం[0])>= పరిమాణం(res.n_దత్తాంశం) );
  res.az_ఫలితం[0] = SQLITE_INT_గమ్యం_PTR(res.n_దత్తాంశం);
  ఒకవేళ( (rc&0xff)==SQLITE_ABORT ){
    sqlite3_free_పట్టిక(&res.az_ఫలితం[1]);
    ఒకవేళ( res.z_Err_సందేశం ){
      ఒకవేళ( pz_Err_సందేశం ){
        sqlite3_free(*pz_Err_సందేశం);
        *pz_Err_సందేశం = sqlite3_mprintf("%s",res.z_Err_సందేశం);
      }
      sqlite3_free(res.z_Err_సందేశం);
    }
    db->err_సంకేతం = res.rc;  /* Assume 32-bit assignment is atomic */
    తిరిగి res.rc;
  }
  sqlite3_free(res.z_Err_సందేశం);
  ఒకవేళ( rc!=SQLITE_OK ){
    sqlite3_free_పట్టిక(&res.az_ఫలితం[1]);
    తిరిగి rc;
  }
  ఒకవేళ( res.nAlloc>res.n_దత్తాంశం ){
    అక్షరం **azNew;
    azNew = sqlite3Realloc( res.az_ఫలితం, పరిమాణం(అక్షరం*)*res.n_దత్తాంశం );
    ఒకవేళ( azNew==0 ){
      sqlite3_free_పట్టిక(&res.az_ఫలితం[1]);
      db->err_సంకేతం = SQLITE_NOMEM;
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    res.az_ఫలితం = azNew;
  }
  *paz_ఫలితం = &res.az_ఫలితం[1];
  ఒకవేళ( pn_స్తంభం ) *pn_స్తంభం = res.n_స్తంభం;
  ఒకవేళ( pn_అడ్డువరుస ) *pn_అడ్డువరుస = res.n_అడ్డువరుస;
  తిరిగి rc;
}

/*
** This routine frees the space the sqlite3_get_table() malloced.
*/
శూన్య sqlite3_free_పట్టిక(
  అక్షరం **az_ఫలితం            /* Result returned from sqlite3_get_table() */
){
  ఒకవేళ( az_ఫలితం ){
    పూర్ణాంకం i, n;
    az_ఫలితం--;
    ప్రతిజ్ఞా( az_ఫలితం!=0 );
    n = SQLITE_PTR_గమ్యం_INT(az_ఫలితం[0]);
    కొరకు(i=1; i<n; i++){ ఒకవేళ( az_ఫలితం[i] ) sqlite3_free(az_ఫలితం[i]); }
    sqlite3_free(az_ఫలితం);
  }
}

#అయితే_ముగింపు /* SQLITE_OMIT_GET_TABLE */
