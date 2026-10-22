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
#ਜੇ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਜੇ defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  ਬਣਤਰ iovec {
    ਖਾਲੀ *iov_ਅਧਾਰ;
    size_t iov_len;
  };
#ਨਹੀਂਤਾਂ
# include <sys/uio.h>
#ਜੇ_ਅੰਤ

/*
** Names of allowed datatypes
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ carray_bind carray_bind;
ਬਣਤਰ carray_bind {
  ਖਾਲੀ *a_ਅੰਕੜੇ;                /* The data */
  ਪੂਰਨ n_ਅੰਕੜੇ;                  /* Number of elements */
  ਪੂਰਨ mFlags;                 /* Control flags */
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*);        /* Destructor for aData */
  ਖਾਲੀ *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ carray_cursor carray_cursor;
ਬਣਤਰ carray_cursor {
  sqlite3_vtab_cursor ਅਧਾਰ;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  ਖਾਲੀ *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ eType;       /* One of the CARRAY_type values */
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
ਸਥਿਰ ਪੂਰਨ carrayConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  sqlite3_vtab *pNew;
  ਪੂਰਨ rc;

/* Column numbers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CARRAY_ਕਾਲਮ_ਮੁੱਲ   0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CARRAY_ਕਾਲਮ_ਸੂਚਕ 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CARRAY_ਕਾਲਮ_ਗਿਣਤੀ   2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CARRAY_ਕਾਲਮ_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  ਜੇ( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( ਆਕਾਰ(*pNew) );
    ਜੇ( pNew==0 ) ਵਾਪਸ SQLITE_NOMEM;
    memset(pNew, 0, ਆਕਾਰ(*pNew));
  }
  ਵਾਪਸ rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
ਸਥਿਰ ਪੂਰਨ carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
ਸਥਿਰ ਪੂਰਨ carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( ਆਕਾਰ(*pCur) );
  ਜੇ( pCur==0 ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pCur, 0, ਆਕਾਰ(*pCur));
  *ppCursor = &pCur->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
ਸਥਿਰ ਪੂਰਨ carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  ਵਾਪਸ SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
ਸਥਿਰ ਪੂਰਨ carray_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
ਸਥਿਰ ਪੂਰਨ carray_ਕਾਲਮ(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ਪੂਰਨ i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  ਚੋਣ( i ){
    ਹਾਲਤ CARRAY_ਕਾਲਮ_ਸੂਚਕ:   ਵਾਪਸ SQLITE_OK;
    ਹਾਲਤ CARRAY_ਕਾਲਮ_ਗਿਣਤੀ:     x = pCur->iCnt;   ਰੋਕੋ;
    ਹਾਲਤ CARRAY_ਕਾਲਮ_CTYPE: {
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      ਵਾਪਸ SQLITE_OK;
    }
    ਮੂਲ: {
      ਚੋਣ( pCur->eType ){
        ਹਾਲਤ CARRAY_INT32: {
          ਪੂਰਨ *p = (ਪੂਰਨ*)pCur->pPtr;
          sqlite3_ਨਤੀਜਾ_int(ctx, p[pCur->iRowid-1]);
          ਵਾਪਸ SQLITE_OK;
        }
        ਹਾਲਤ CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_ਨਤੀਜਾ_int64(ctx, p[pCur->iRowid-1]);
          ਵਾਪਸ SQLITE_OK;
        }
        ਹਾਲਤ CARRAY_DOUBLE: {
          ਦੁੱਗਣਾ *p = (ਦੁੱਗਣਾ*)pCur->pPtr;
          sqlite3_ਨਤੀਜਾ_double(ctx, p[pCur->iRowid-1]);
          ਵਾਪਸ SQLITE_OK;
        }
        ਹਾਲਤ CARRAY_ਪਾਠ: {
          ਅਚਲ ਅੱਖਰ **p = (ਅਚਲ ਅੱਖਰ**)pCur->pPtr;
          sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          ਵਾਪਸ SQLITE_OK;
        }
        ਮੂਲ: {
          ਅਚਲ ਬਣਤਰ iovec *p = (ਬਣਤਰ iovec*)pCur->pPtr;
          ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          ਜੇ( p->iov_len>0x7fffffff ){
            sqlite3_ਨਤੀਜਾ_error_toobig(ctx);
            ਵਾਪਸ SQLITE_TOOBIG;
          }ਨਹੀਂਤਾਂ{
            sqlite3_ਨਤੀਜਾ_blob(ctx, p->iov_ਅਧਾਰ, (ਪੂਰਨ)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          ਵਾਪਸ SQLITE_OK;
        }
      }
    }
  }
  sqlite3_ਨਤੀਜਾ_int64(ctx, x);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ਸਥਿਰ ਪੂਰਨ carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ਸਥਿਰ ਪੂਰਨ carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  ਵਾਪਸ pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
ਸਥਿਰ ਪੂਰਨ carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  ਚੋਣ( idx_ਅੰਕ ){
    ਹਾਲਤ 1: {
      carray_bind *pBind = sqlite3_ਮੁੱਲ_ਸੂਚਕ(argv[0], "carray-bind");
      ਜੇ( pBind==0 ) ਰੋਕੋ;
      pCur->pPtr = pBind->a_ਅੰਕੜੇ;
      pCur->iCnt = pBind->n_ਅੰਕੜੇ;
      pCur->eType = pBind->mFlags & 0x07;
      ਰੋਕੋ;
    }
    ਹਾਲਤ 2:
    ਹਾਲਤ 3: {
      pCur->pPtr = sqlite3_ਮੁੱਲ_ਸੂਚਕ(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_ਮੁੱਲ_int64(argv[1]) : 0;
      ਜੇ( idx_ਅੰਕ<3 ){
        pCur->eType = CARRAY_INT32;
      }ਨਹੀਂਤਾਂ{
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ i;
        ਅਚਲ ਅੱਖਰ *zType = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[2]);
        ਲਈ(i=0; i<ਆਕਾਰ(azCarrayType)/ਆਕਾਰ(azCarrayType[0]); i++){
          ਜੇ( sqlite3_stricmp(zType, azCarrayType[i])==0 ) ਰੋਕੋ;
        }
        ਜੇ( i>=ਆਕਾਰ(azCarrayType)/ਆਕਾਰ(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          ਵਾਪਸ SQLITE_ERROR;
        }ਨਹੀਂਤਾਂ{
          pCur->eType = i;
        }
      }
      ਰੋਕੋ;
    }
  }
  pCur->iRowid = 1;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ carray_ਉੱਤਮ_ਸੂਚਕਾਂਕ(
  sqlite3_vtab *tab,
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo
){
  ਪੂਰਨ i;                 /* Loop over constraints */
  ਪੂਰਨ ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  ਪੂਰਨ cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  ਪੂਰਨ ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਵੇਖਿਆ = 0;     /* Bitmask of == constrainted columns */

  ਅਚਲ ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  ਲਈ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ਜੇ( pConstraint->op!=SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ ) ਜਾਰੀ;
    ਜੇ( pConstraint->i_ਕਾਲਮ>=0 ) ਵੇਖਿਆ |= 1 << pConstraint->i_ਕਾਲਮ;
    ਜੇ( pConstraint->usable==0 ) ਜਾਰੀ;
    ਚੋਣ( pConstraint->i_ਕਾਲਮ ){
      ਹਾਲਤ CARRAY_ਕਾਲਮ_ਸੂਚਕ:
        ptrIdx = i;
        ਰੋਕੋ;
      ਹਾਲਤ CARRAY_ਕਾਲਮ_ਗਿਣਤੀ:
        cntIdx = i;
        ਰੋਕੋ;
      ਹਾਲਤ CARRAY_ਕਾਲਮ_CTYPE:
        ctypeIdx = i;
        ਰੋਕੋ;
    }
  }
  ਜੇ( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argv_ਸੂਚਕਾਂਕ = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimated_ਲਾਗਤ = (ਦੁੱਗਣਾ)1;
    pIdxInfo->estimated_ਪੰਕਤੀਆਂ = 100;
    pIdxInfo->idx_ਅੰਕ = 1;
    ਜੇ( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argv_ਸੂਚਕਾਂਕ = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_ਅੰਕ = 2;
      ਜੇ( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argv_ਸੂਚਕਾਂਕ = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_ਅੰਕ = 3;
      }ਨਹੀਂਤਾਂ ਜੇ( ਵੇਖਿਆ & (1<<CARRAY_ਕਾਲਮ_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        ਵਾਪਸ SQLITE_CONSTRAINT;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( ਵੇਖਿਆ & (1<<CARRAY_ਕਾਲਮ_ਗਿਣਤੀ) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      ਵਾਪਸ SQLITE_CONSTRAINT;
    }
  }ਨਹੀਂਤਾਂ{
    pIdxInfo->estimated_ਲਾਗਤ = (ਦੁੱਗਣਾ)2147483647;
    pIdxInfo->estimated_ਪੰਕਤੀਆਂ = 2147483647;
    pIdxInfo->idx_ਅੰਕ = 0;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
ਸਥਿਰ sqlite3_module carrayModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  carrayConnect,             /* xConnect */
  carray_ਉੱਤਮ_ਸੂਚਕਾਂਕ,           /* xBestIndex */
  carrayDisconnect,          /* xDisconnect */
  0,                         /* xDestroy */
  carrayOpen,                /* xOpen - open a cursor */
  carrayClose,               /* xClose - close a cursor */
  carrayFilter,              /* xFilter - configure scan constraints */
  carray_ਅਗਲਾ,                /* xNext - advance a cursor */
  carrayEof,                 /* xEof - check for end of scan */
  carray_ਕਾਲਮ,              /* xColumn - read data */
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
ਸਥਿਰ ਖਾਲੀ carrayBindDel(ਖਾਲੀ *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  ਜੇ( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API ਪੂਰਨ sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  ਪੂਰਨ idx,
  ਖਾਲੀ *a_ਅੰਕੜੇ,
  ਪੂਰਨ n_ਅੰਕੜੇ,
  ਪੂਰਨ mFlags,
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ*),
  ਖਾਲੀ *pDestroy
){
  carray_bind *pNew = 0;
  ਪੂਰਨ i;
  ਪੂਰਨ rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  ਪ੍ਰਤਿਜ੍ਞਾ( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( CARRAY_ਪਾਠ==3 && CARRAY_BLOB==4 );
  ਜੇ( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    ਜਾਓ carray_bind_error;
  }

  pNew = sqlite3_malloc64(ਆਕਾਰ(*pNew));
  ਜੇ( pNew==0 ){
    rc = SQLITE_NOMEM;
    ਜਾਓ carray_bind_error;
  }

  pNew->n_ਅੰਕੜੇ = n_ਅੰਕੜੇ;
  pNew->mFlags = mFlags;
  ਜੇ( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = n_ਅੰਕੜੇ;
    ਚੋਣ( mFlags ){
      ਹਾਲਤ CARRAY_INT32:   sz *= 4;                     ਰੋਕੋ;
      ਹਾਲਤ CARRAY_INT64:   sz *= 8;                     ਰੋਕੋ;
      ਹਾਲਤ CARRAY_DOUBLE:  sz *= 8;                     ਰੋਕੋ;
      ਹਾਲਤ CARRAY_ਪਾਠ:    sz *= ਆਕਾਰ(ਅੱਖਰ*);         ਰੋਕੋ;
      ਮੂਲ:             sz *= ਆਕਾਰ(ਬਣਤਰ iovec);  ਰੋਕੋ;
    }
    ਜੇ( mFlags==CARRAY_ਪਾਠ ){
      ਲਈ(i=0; i<n_ਅੰਕੜੇ; i++){
        ਅਚਲ ਅੱਖਰ *z = ((ਅੱਖਰ**)a_ਅੰਕੜੇ)[i];
        ਜੇ( z ) sz += ਲੜੀ_ਲੰਬਾਈ(z) + 1;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( mFlags==CARRAY_BLOB ){
      ਲਈ(i=0; i<n_ਅੰਕੜੇ; i++){
        sz += ((ਬਣਤਰ iovec*)a_ਅੰਕੜੇ)[i].iov_len;
      }
    } 

    pNew->a_ਅੰਕੜੇ = sqlite3_malloc64( sz>0 ? sz : 1 );
    ਜੇ( pNew->a_ਅੰਕੜੇ==0 ){
      rc = SQLITE_NOMEM;
      ਜਾਓ carray_bind_error;
    }

    ਜੇ( mFlags==CARRAY_ਪਾਠ ){
      ਅੱਖਰ **az = (ਅੱਖਰ**)pNew->a_ਅੰਕੜੇ;
      ਅੱਖਰ *z = (ਅੱਖਰ*)&az[n_ਅੰਕੜੇ];
      ਲਈ(i=0; i<n_ਅੰਕੜੇ; i++){
        ਅਚਲ ਅੱਖਰ *z_ਅੰਕੜੇ = ((ਅੱਖਰ**)a_ਅੰਕੜੇ)[i];
        sqlite3_int64 n;
        ਜੇ( z_ਅੰਕੜੇ==0 ){
          az[i] = 0;
          ਜਾਰੀ;
        }
        az[i] = z;
        n = ਲੜੀ_ਲੰਬਾਈ(z_ਅੰਕੜੇ);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(z, z_ਅੰਕੜੇ, n+1);
        z += n+1;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( mFlags==CARRAY_BLOB ){
      ਬਣਤਰ iovec *p = (ਬਣਤਰ iovec*)pNew->a_ਅੰਕੜੇ;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&p[n_ਅੰਕੜੇ];
      ਲਈ(i=0; i<n_ਅੰਕੜੇ; i++){
        size_t n = ((ਬਣਤਰ iovec*)a_ਅੰਕੜੇ)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_ਅਧਾਰ = z;
        z += n;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(p[i].iov_ਅਧਾਰ, ((ਬਣਤਰ iovec*)a_ਅੰਕੜੇ)[i].iov_ਅਧਾਰ, n);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( sz>0 ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew->a_ਅੰਕੜੇ, a_ਅੰਕੜੇ, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->a_ਅੰਕੜੇ;
  }ਨਹੀਂਤਾਂ{
    pNew->a_ਅੰਕੜੇ = a_ਅੰਕੜੇ;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  ਵਾਪਸ sqlite3_bind_ਸੂਚਕ(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  ਜੇ( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  ਵਾਪਸ rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API ਪੂਰਨ sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  ਪੂਰਨ idx,
  ਖਾਲੀ *a_ਅੰਕੜੇ,
  ਪੂਰਨ n_ਅੰਕੜੇ,
  ਪੂਰਨ mFlags,
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ*)
){
  ਵਾਪਸ sqlite3_carray_bind_v2(pStmt,idx,a_ਅੰਕੜੇ,n_ਅੰਕੜੇ,mFlags,xDestroy,a_ਅੰਕੜੇ);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3_Carray_ਰਜਿਸਟਰ(sqlite3 *db){
  ਵਾਪਸ sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
