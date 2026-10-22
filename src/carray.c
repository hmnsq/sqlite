/*
** 2016-06-29
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements a table-valued-function that
** returns the values in a C-language array.
** Examples:
**
**      SELECT * FROM carray($ptr,5)
**
** The query above returns 5 integers contained in a C-language array
** at the address $ptr.  $ptr is a pointer to the array of integers.
** The pointer value must be assigned to $ptr using the
** sqlite3_bind_pointer() interface with a pointer type of "carray".
** For example:
**
**    static int aX[] = { 53, 9, 17, 2231, 4, 99 };
**    int i = sqlite3_bind_parameter_index(pStmt, "$ptr");
**    sqlite3_bind_pointer(pStmt, i, aX, "carray", 0);
**
** There is an optional third parameter to determine the datatype of
** the C-language array.  Allowed values of the third parameter are
** 'int32', 'int64', 'double', 'char*', 'struct iovec'.  Example:
**
**      SELECT * FROM carray($ptr,10,'char*');
**
** The default value of the third parameter is 'int32'.
**
** HOW IT WORKS
**
** The carray "function" is really a virtual table with the
** following schema:
**
**     CREATE TABLE carray(
**       value,
**       pointer HIDDEN,
**       count HIDDEN,
**       ctype TEXT HIDDEN
**     );
**
** If the hidden columns "pointer" and "count" are unconstrained, then 
** the virtual table has no rows.  Otherwise, the virtual table interprets
** the integer value of "pointer" as a pointer to the array and "count"
** as the number of elements in the array.  The virtual table steps through
** the array, element by element.
*/
#ఒకవేళ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#అన్తర్భావ "sqliteInt.h"
#ఒకవేళ defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  నిర్మాణం iovec {
    శూన్య *iov_పునాది;
    size_t iov_len;
  };
#లేకపోతే
# include <sys/uio.h>
#అయితే_ముగింపు

/*
** Names of allowed datatypes
*/
స్థిర స్థిరరాశి అక్షరం *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
రకనిర్వచనం నిర్మాణం carray_bind carray_bind;
నిర్మాణం carray_bind {
  శూన్య *a_దత్తాంశం;                /* The data */
  పూర్ణాంకం n_దత్తాంశం;                  /* Number of elements */
  పూర్ణాంకం mFlags;                 /* Control flags */
  శూన్య (*xDel)(శూన్య*);        /* Destructor for aData */
  శూన్య *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
రకనిర్వచనం నిర్మాణం carray_cursor carray_cursor;
నిర్మాణం carray_cursor {
  sqlite3_vtab_cursor పునాది;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  శూన్య *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  గుర్తులేని అక్షరం eType;       /* One of the CARRAY_type values */
};

/*
** The carrayConnect() method is invoked to create a new
** carray_vtab that describes the carray virtual table.
**
** Think of this routine as the constructor for carray_vtab objects.
**
** All this routine needs to do is:
**
**    (1) Allocate the carray_vtab object and initialize all fields.
**
**    (2) Tell SQLite (via the sqlite3_declare_vtab() interface) what the
**        result set of queries against carray will look like.
*/
స్థిర పూర్ణాంకం carrayConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  sqlite3_vtab *pNew;
  పూర్ణాంకం rc;

/* Column numbers */
#నిర్వచించు CARRAY_స్తంభం_విలువ   0
#నిర్వచించు CARRAY_స్తంభం_సూచిక 1
#నిర్వచించు CARRAY_స్తంభం_లెక్క   2
#నిర్వచించు CARRAY_స్తంభం_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  ఒకవేళ( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( పరిమాణం(*pNew) );
    ఒకవేళ( pNew==0 ) తిరిగి SQLITE_NOMEM;
    memset(pNew, 0, పరిమాణం(*pNew));
  }
  తిరిగి rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
స్థిర పూర్ణాంకం carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  తిరిగి SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
స్థిర పూర్ణాంకం carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( పరిమాణం(*pCur) );
  ఒకవేళ( pCur==0 ) తిరిగి SQLITE_NOMEM;
  memset(pCur, 0, పరిమాణం(*pCur));
  *ppCursor = &pCur->పునాది;
  తిరిగి SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
స్థిర పూర్ణాంకం carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  తిరిగి SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
స్థిర పూర్ణాంకం carray_తదుపరి(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  తిరిగి SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
స్థిర పూర్ణాంకం carray_స్తంభం(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  పూర్ణాంకం i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  ఎంపిక( i ){
    స్థితి CARRAY_స్తంభం_సూచిక:   తిరిగి SQLITE_OK;
    స్థితి CARRAY_స్తంభం_లెక్క:     x = pCur->iCnt;   ఆపు;
    స్థితి CARRAY_స్తంభం_CTYPE: {
      sqlite3_ఫలితం_పాఠ్యం(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      తిరిగి SQLITE_OK;
    }
    అప్రమేయ: {
      ఎంపిక( pCur->eType ){
        స్థితి CARRAY_INT32: {
          పూర్ణాంకం *p = (పూర్ణాంకం*)pCur->pPtr;
          sqlite3_ఫలితం_int(ctx, p[pCur->iRowid-1]);
          తిరిగి SQLITE_OK;
        }
        స్థితి CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_ఫలితం_int64(ctx, p[pCur->iRowid-1]);
          తిరిగి SQLITE_OK;
        }
        స్థితి CARRAY_DOUBLE: {
          ద్విగుణ *p = (ద్విగుణ*)pCur->pPtr;
          sqlite3_ఫలితం_double(ctx, p[pCur->iRowid-1]);
          తిరిగి SQLITE_OK;
        }
        స్థితి CARRAY_పాఠ్యం: {
          స్థిరరాశి అక్షరం **p = (స్థిరరాశి అక్షరం**)pCur->pPtr;
          sqlite3_ఫలితం_పాఠ్యం(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          తిరిగి SQLITE_OK;
        }
        అప్రమేయ: {
          స్థిరరాశి నిర్మాణం iovec *p = (నిర్మాణం iovec*)pCur->pPtr;
          ప్రతిజ్ఞా( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          ఒకవేళ( p->iov_len>0x7fffffff ){
            sqlite3_ఫలితం_error_toobig(ctx);
            తిరిగి SQLITE_TOOBIG;
          }లేకపోతే{
            sqlite3_ఫలితం_blob(ctx, p->iov_పునాది, (పూర్ణాంకం)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          తిరిగి SQLITE_OK;
        }
      }
    }
  }
  sqlite3_ఫలితం_int64(ctx, x);
  తిరిగి SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
స్థిర పూర్ణాంకం carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  తిరిగి SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
స్థిర పూర్ణాంకం carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  తిరిగి pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
స్థిర పూర్ణాంకం carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  ఎంపిక( idx_అంకె ){
    స్థితి 1: {
      carray_bind *pBind = sqlite3_విలువ_సూచిక(argv[0], "carray-bind");
      ఒకవేళ( pBind==0 ) ఆపు;
      pCur->pPtr = pBind->a_దత్తాంశం;
      pCur->iCnt = pBind->n_దత్తాంశం;
      pCur->eType = pBind->mFlags & 0x07;
      ఆపు;
    }
    స్థితి 2:
    స్థితి 3: {
      pCur->pPtr = sqlite3_విలువ_సూచిక(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_విలువ_int64(argv[1]) : 0;
      ఒకవేళ( idx_అంకె<3 ){
        pCur->eType = CARRAY_INT32;
      }లేకపోతే{
        గుర్తులేని అక్షరం i;
        స్థిరరాశి అక్షరం *zType = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[2]);
        కొరకు(i=0; i<పరిమాణం(azCarrayType)/పరిమాణం(azCarrayType[0]); i++){
          ఒకవేళ( sqlite3_stricmp(zType, azCarrayType[i])==0 ) ఆపు;
        }
        ఒకవేళ( i>=పరిమాణం(azCarrayType)/పరిమాణం(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_సందేశం = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          తిరిగి SQLITE_ERROR;
        }లేకపోతే{
          pCur->eType = i;
        }
      }
      ఆపు;
    }
  }
  pCur->iRowid = 1;
  తిరిగి SQLITE_OK;
}

/*
** SQLite will invoke this method one or more times while planning a query
** that uses the carray virtual table.  This routine needs to create
** a query plan for each invocation and compute an estimated cost for that
** plan.
**
** In this implementation idxNum is used to represent the
** query plan.  idxStr is unused.
**
** idxNum is:
**
**    1    If only the pointer= constraint exists.  In this case, the
**         parameter must be bound using sqlite3_carray_bind().
**
**    2    if the pointer= and count= constraints exist.
**
**    3    if the ctype= constraint also exists.
**
** idxNum is 0 otherwise and carray becomes an empty table.
*/
స్థిర పూర్ణాంకం carray_ఉత్తమ_సూచీ(
  sqlite3_vtab *tab,
  sqlite3_సూచీ_info *pIdxInfo
){
  పూర్ణాంకం i;                 /* Loop over constraints */
  పూర్ణాంకం ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  పూర్ణాంకం cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  పూర్ణాంకం ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  గుర్తులేని చూసిన = 0;     /* Bitmask of == constrainted columns */

  స్థిరరాశి నిర్మాణం sqlite3_సూచీ_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  కొరకు(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ఒకవేళ( pConstraint->op!=SQLITE_సూచీ_CONSTRAINT_EQ ) కొనసాగు;
    ఒకవేళ( pConstraint->i_స్తంభం>=0 ) చూసిన |= 1 << pConstraint->i_స్తంభం;
    ఒకవేళ( pConstraint->usable==0 ) కొనసాగు;
    ఎంపిక( pConstraint->i_స్తంభం ){
      స్థితి CARRAY_స్తంభం_సూచిక:
        ptrIdx = i;
        ఆపు;
      స్థితి CARRAY_స్తంభం_లెక్క:
        cntIdx = i;
        ఆపు;
      స్థితి CARRAY_స్తంభం_CTYPE:
        ctypeIdx = i;
        ఆపు;
    }
  }
  ఒకవేళ( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argv_సూచీ = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimated_ఖర్చు = (ద్విగుణ)1;
    pIdxInfo->estimated_అడ్డువరుసలు = 100;
    pIdxInfo->idx_అంకె = 1;
    ఒకవేళ( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argv_సూచీ = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_అంకె = 2;
      ఒకవేళ( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argv_సూచీ = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_అంకె = 3;
      }లేకపోతే ఒకవేళ( చూసిన & (1<<CARRAY_స్తంభం_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        తిరిగి SQLITE_CONSTRAINT;
      }
    }లేకపోతే ఒకవేళ( చూసిన & (1<<CARRAY_స్తంభం_లెక్క) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      తిరిగి SQLITE_CONSTRAINT;
    }
  }లేకపోతే{
    pIdxInfo->estimated_ఖర్చు = (ద్విగుణ)2147483647;
    pIdxInfo->estimated_అడ్డువరుసలు = 2147483647;
    pIdxInfo->idx_అంకె = 0;
  }
  తిరిగి SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
స్థిర sqlite3_module carrayModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  carrayConnect,             /* xConnect */
  carray_ఉత్తమ_సూచీ,           /* xBestIndex */
  carrayDisconnect,          /* xDisconnect */
  0,                         /* xDestroy */
  carrayOpen,                /* xOpen - open a cursor */
  carrayClose,               /* xClose - close a cursor */
  carrayFilter,              /* xFilter - configure scan constraints */
  carray_తదుపరి,                /* xNext - advance a cursor */
  carrayEof,                 /* xEof - check for end of scan */
  carray_స్తంభం,              /* xColumn - read data */
  carrayRowid,               /* xRowid - read data */
  0,                         /* xUpdate */
  0,                         /* xBegin */
  0,                         /* xSync */
  0,                         /* xCommit */
  0,                         /* xRollback */
  0,                         /* xFindMethod */
  0,                         /* xRename */
  0,                         /* xSavepoint */
  0,                         /* xRelease */
  0,                         /* xRollbackTo */
  0,                         /* xShadow */
  0                          /* xIntegrity */
};

/*
** Destructor for the carray_bind object
*/
స్థిర శూన్య carrayBindDel(శూన్య *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  ఒకవేళ( p->xDel!=SQLITE_STATIC ){
    p->xDel(p->pDel);
  }
  sqlite3_free(p);
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().
**
**    pStmt        The prepared statement to which to bind
**    idx          The index of the parameter of pStmt to which to bind
**    aData        The data to be bound
**    nData        The number of elements in aData
**    mFlags       One of SQLITE_CARRAY_xxxx indicating datatype of aData
**    xDestroy     Destructor for pDestroy or aData if pDestroy==NULL.
**    pDestroy     Invoke xDestroy on this pointer if not NULL
**
** The destructor is called pDestroy if pDestroy!=NULL, or against
** aData if pDestroy==NULL.
*/
SQLITE_API పూర్ణాంకం sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  పూర్ణాంకం idx,
  శూన్య *a_దత్తాంశం,
  పూర్ణాంకం n_దత్తాంశం,
  పూర్ణాంకం mFlags,
  శూన్య (*xDestroy)(శూన్య*),
  శూన్య *pDestroy
){
  carray_bind *pNew = 0;
  పూర్ణాంకం i;
  పూర్ణాంకం rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  ప్రతిజ్ఞా( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  ప్రతిజ్ఞా( CARRAY_పాఠ్యం==3 && CARRAY_BLOB==4 );
  ఒకవేళ( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    వెళ్ళు carray_bind_error;
  }

  pNew = sqlite3_malloc64(పరిమాణం(*pNew));
  ఒకవేళ( pNew==0 ){
    rc = SQLITE_NOMEM;
    వెళ్ళు carray_bind_error;
  }

  pNew->n_దత్తాంశం = n_దత్తాంశం;
  pNew->mFlags = mFlags;
  ఒకవేళ( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = n_దత్తాంశం;
    ఎంపిక( mFlags ){
      స్థితి CARRAY_INT32:   sz *= 4;                     ఆపు;
      స్థితి CARRAY_INT64:   sz *= 8;                     ఆపు;
      స్థితి CARRAY_DOUBLE:  sz *= 8;                     ఆపు;
      స్థితి CARRAY_పాఠ్యం:    sz *= పరిమాణం(అక్షరం*);         ఆపు;
      అప్రమేయ:             sz *= పరిమాణం(నిర్మాణం iovec);  ఆపు;
    }
    ఒకవేళ( mFlags==CARRAY_పాఠ్యం ){
      కొరకు(i=0; i<n_దత్తాంశం; i++){
        స్థిరరాశి అక్షరం *z = ((అక్షరం**)a_దత్తాంశం)[i];
        ఒకవేళ( z ) sz += అక్షరశ్రేణి_పొడవు(z) + 1;
      }
    }లేకపోతే ఒకవేళ( mFlags==CARRAY_BLOB ){
      కొరకు(i=0; i<n_దత్తాంశం; i++){
        sz += ((నిర్మాణం iovec*)a_దత్తాంశం)[i].iov_len;
      }
    } 

    pNew->a_దత్తాంశం = sqlite3_malloc64( sz>0 ? sz : 1 );
    ఒకవేళ( pNew->a_దత్తాంశం==0 ){
      rc = SQLITE_NOMEM;
      వెళ్ళు carray_bind_error;
    }

    ఒకవేళ( mFlags==CARRAY_పాఠ్యం ){
      అక్షరం **az = (అక్షరం**)pNew->a_దత్తాంశం;
      అక్షరం *z = (అక్షరం*)&az[n_దత్తాంశం];
      కొరకు(i=0; i<n_దత్తాంశం; i++){
        స్థిరరాశి అక్షరం *z_దత్తాంశం = ((అక్షరం**)a_దత్తాంశం)[i];
        sqlite3_int64 n;
        ఒకవేళ( z_దత్తాంశం==0 ){
          az[i] = 0;
          కొనసాగు;
        }
        az[i] = z;
        n = అక్షరశ్రేణి_పొడవు(z_దత్తాంశం);
        జ్ఞాపకం_నకలు(z, z_దత్తాంశం, n+1);
        z += n+1;
      }
    }లేకపోతే ఒకవేళ( mFlags==CARRAY_BLOB ){
      నిర్మాణం iovec *p = (నిర్మాణం iovec*)pNew->a_దత్తాంశం;
      గుర్తులేని అక్షరం *z = (గుర్తులేని అక్షరం*)&p[n_దత్తాంశం];
      కొరకు(i=0; i<n_దత్తాంశం; i++){
        size_t n = ((నిర్మాణం iovec*)a_దత్తాంశం)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_పునాది = z;
        z += n;
        జ్ఞాపకం_నకలు(p[i].iov_పునాది, ((నిర్మాణం iovec*)a_దత్తాంశం)[i].iov_పునాది, n);
      }
    }లేకపోతే ఒకవేళ( sz>0 ){
      జ్ఞాపకం_నకలు(pNew->a_దత్తాంశం, a_దత్తాంశం, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->a_దత్తాంశం;
  }లేకపోతే{
    pNew->a_దత్తాంశం = a_దత్తాంశం;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  తిరిగి sqlite3_bind_సూచిక(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  ఒకవేళ( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  తిరిగి rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API పూర్ణాంకం sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  పూర్ణాంకం idx,
  శూన్య *a_దత్తాంశం,
  పూర్ణాంకం n_దత్తాంశం,
  పూర్ణాంకం mFlags,
  శూన్య (*xDestroy)(శూన్య*)
){
  తిరిగి sqlite3_carray_bind_v2(pStmt,idx,a_దత్తాంశం,n_దత్తాంశం,mFlags,xDestroy,a_దత్తాంశం);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3_Carray_రిజిస్టరు(sqlite3 *db){
  తిరిగి sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#అయితే_ముగింపు /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
