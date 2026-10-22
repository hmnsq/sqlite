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
#ប្រសិនបើ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#include "sqliteInt.h"
#ប្រសិនបើ defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  sa_nracanaa iovec {
    void *iov_base;
    size_t iov_len;
  };
#បើមិនដូច្នេះ
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
static const var_na *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
typedef sa_nracanaa carray_bind carray_bind;
sa_nracanaa carray_bind {
  void *aData;                /* The data */
  ចំនួនគត់ nData;                  /* Number of elements */
  ចំនួនគត់ mFlags;                 /* Control flags */
  void (*xDel)(void*);        /* Destructor for aData */
  void *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
typedef sa_nracanaa carray_cursor carray_cursor;
sa_nracanaa carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  void *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned var_na eType;       /* One of the CARRAY_type values */
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
static ចំនួនគត់ carrayConnect(
  sqlite3 *db,
  void *pAux,
  ចំនួនគត់ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  sqlite3_vtab *pNew;
  ចំនួនគត់ rc;

/* Column numbers */
#define CARRAY_COLUMN_VALUE   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_COUNT   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  ប្រសិនបើ( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( parimaa_na(*pNew) );
    ប្រសិនបើ( pNew==0 ) ត្រឡប់ SQLITE_NOMEM;
    memset(pNew, 0, parimaa_na(*pNew));
  }
  ត្រឡប់ rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
static ចំនួនគត់ carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ត្រឡប់ SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
static ចំនួនគត់ carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( parimaa_na(*pCur) );
  ប្រសិនបើ( pCur==0 ) ត្រឡប់ SQLITE_NOMEM;
  memset(pCur, 0, parimaa_na(*pCur));
  *ppCursor = &pCur->base;
  ត្រឡប់ SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
static ចំនួនគត់ carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  ត្រឡប់ SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
static ចំនួនគត់ carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  ត្រឡប់ SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
static ចំនួនគត់ carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ចំនួនគត់ i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  switch( i ){
    pak_sa CARRAY_COLUMN_POINTER:   ត្រឡប់ SQLITE_OK;
    pak_sa CARRAY_COLUMN_COUNT:     x = pCur->iCnt;   bha_nga;
    pak_sa CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      ត្រឡប់ SQLITE_OK;
    }
    default: {
      switch( pCur->eType ){
        pak_sa CARRAY_INT32: {
          ចំនួនគត់ *p = (ចំនួនគត់*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          ត្រឡប់ SQLITE_OK;
        }
        pak_sa CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          ត្រឡប់ SQLITE_OK;
        }
        pak_sa CARRAY_DOUBLE: {
          dviguu_na *p = (dviguu_na*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          ត្រឡប់ SQLITE_OK;
        }
        pak_sa CARRAY_TEXT: {
          const var_na **p = (const var_na**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          ត្រឡប់ SQLITE_OK;
        }
        default: {
          const sa_nracanaa iovec *p = (sa_nracanaa iovec*)pCur->pPtr;
          assert( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          ប្រសិនបើ( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            ត្រឡប់ SQLITE_TOOBIG;
          }បើមិនដូច្នេះ{
            sqlite3_result_blob(ctx, p->iov_base, (ចំនួនគត់)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          ត្រឡប់ SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  ត្រឡប់ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static ចំនួនគត់ carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  ត្រឡប់ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static ចំនួនគត់ carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  ត្រឡប់ pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
static ចំនួនគត់ carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ចំនួនគត់ idxNum, const var_na *idxStr,
  ចំនួនគត់ argc, sqlite3_value **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  switch( idxNum ){
    pak_sa 1: {
      carray_bind *pBind = sqlite3_value_pointer(argv[0], "carray-bind");
      ប្រសិនបើ( pBind==0 ) bha_nga;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      bha_nga;
    }
    pak_sa 2:
    pak_sa 3: {
      pCur->pPtr = sqlite3_value_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_value_int64(argv[1]) : 0;
      ប្រសិនបើ( idxNum<3 ){
        pCur->eType = CARRAY_INT32;
      }បើមិនដូច្នេះ{
        unsigned var_na i;
        const var_na *zType = (const var_na*)sqlite3_value_text(argv[2]);
        សម្រាប់(i=0; i<parimaa_na(azCarrayType)/parimaa_na(azCarrayType[0]); i++){
          ប្រសិនបើ( sqlite3_stricmp(zType, azCarrayType[i])==0 ) bha_nga;
        }
        ប្រសិនបើ( i>=parimaa_na(azCarrayType)/parimaa_na(azCarrayType[0]) ){
          pVtabCursor->pVtab->zErrMsg = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          ត្រឡប់ SQLITE_ERROR;
        }បើមិនដូច្នេះ{
          pCur->eType = i;
        }
      }
      bha_nga;
    }
  }
  pCur->iRowid = 1;
  ត្រឡប់ SQLITE_OK;
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
static ចំនួនគត់ carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  ចំនួនគត់ i;                 /* Loop over constraints */
  ចំនួនគត់ ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  ចំនួនគត់ cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  ចំនួនគត់ ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  const sa_nracanaa sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  សម្រាប់(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ប្រសិនបើ( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    ប្រសិនបើ( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    ប្រសិនបើ( pConstraint->usable==0 ) continue;
    switch( pConstraint->iColumn ){
      pak_sa CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        bha_nga;
      pak_sa CARRAY_COLUMN_COUNT:
        cntIdx = i;
        bha_nga;
      pak_sa CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        bha_nga;
    }
  }
  ប្រសិនបើ( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (dviguu_na)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idxNum = 1;
    ប្រសិនបើ( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idxNum = 2;
      ប្រសិនបើ( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idxNum = 3;
      }បើមិនដូច្នេះ ប្រសិនបើ( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        ត្រឡប់ SQLITE_CONSTRAINT;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( seen & (1<<CARRAY_COLUMN_COUNT) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      ត្រឡប់ SQLITE_CONSTRAINT;
    }
  }បើមិនដូច្នេះ{
    pIdxInfo->estimatedCost = (dviguu_na)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idxNum = 0;
  }
  ត្រឡប់ SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
static sqlite3_module carrayModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  carrayConnect,             /* xConnect */
  carrayBestIndex,           /* xBestIndex */
  carrayDisconnect,          /* xDisconnect */
  0,                         /* xDestroy */
  carrayOpen,                /* xOpen - open a cursor */
  carrayClose,               /* xClose - close a cursor */
  carrayFilter,              /* xFilter - configure scan constraints */
  carrayNext,                /* xNext - advance a cursor */
  carrayEof,                 /* xEof - check for end of scan */
  carrayColumn,              /* xColumn - read data */
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
static void carrayBindDel(void *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  ប្រសិនបើ( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API ចំនួនគត់ sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  ចំនួនគត់ idx,
  void *aData,
  ចំនួនគត់ nData,
  ចំនួនគត់ mFlags,
  void (*xDestroy)(void*),
  void *pDestroy
){
  carray_bind *pNew = 0;
  ចំនួនគត់ i;
  ចំនួនគត់ rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  assert( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  assert( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  ប្រសិនបើ( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    goto carray_bind_error;
  }

  pNew = sqlite3_malloc64(parimaa_na(*pNew));
  ប្រសិនបើ( pNew==0 ){
    rc = SQLITE_NOMEM;
    goto carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  ប្រសិនបើ( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    switch( mFlags ){
      pak_sa CARRAY_INT32:   sz *= 4;                     bha_nga;
      pak_sa CARRAY_INT64:   sz *= 8;                     bha_nga;
      pak_sa CARRAY_DOUBLE:  sz *= 8;                     bha_nga;
      pak_sa CARRAY_TEXT:    sz *= parimaa_na(var_na*);         bha_nga;
      default:             sz *= parimaa_na(sa_nracanaa iovec);  bha_nga;
    }
    ប្រសិនបើ( mFlags==CARRAY_TEXT ){
      សម្រាប់(i=0; i<nData; i++){
        const var_na *z = ((var_na**)aData)[i];
        ប្រសិនបើ( z ) sz += strlen(z) + 1;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( mFlags==CARRAY_BLOB ){
      សម្រាប់(i=0; i<nData; i++){
        sz += ((sa_nracanaa iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    ប្រសិនបើ( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      goto carray_bind_error;
    }

    ប្រសិនបើ( mFlags==CARRAY_TEXT ){
      var_na **az = (var_na**)pNew->aData;
      var_na *z = (var_na*)&az[nData];
      សម្រាប់(i=0; i<nData; i++){
        const var_na *zData = ((var_na**)aData)[i];
        sqlite3_int64 n;
        ប្រសិនបើ( zData==0 ){
          az[i] = 0;
          continue;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( mFlags==CARRAY_BLOB ){
      sa_nracanaa iovec *p = (sa_nracanaa iovec*)pNew->aData;
      unsigned var_na *z = (unsigned var_na*)&p[nData];
      សម្រាប់(i=0; i<nData; i++){
        size_t n = ((sa_nracanaa iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((sa_nracanaa iovec*)aData)[i].iov_base, n);
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }បើមិនដូច្នេះ{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  ត្រឡប់ sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  ប្រសិនបើ( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  ត្រឡប់ rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API ចំនួនគត់ sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  ចំនួនគត់ idx,
  void *aData,
  ចំនួនគត់ nData,
  ចំនួនគត់ mFlags,
  void (*xDestroy)(void*)
){
  ត្រឡប់ sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  ត្រឡប់ sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
