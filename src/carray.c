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
#എങ്കിൽ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#അന്തര്ഭാവ "sqliteInt.h"
#എങ്കിൽ defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  ഘടന iovec {
    ശൂന്യം *iov_base;
    size_t iov_len;
  };
#അല്ലെങ്കിൽ
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
തരനിർവചനം ഘടന carray_bind carray_bind;
ഘടന carray_bind {
  ശൂന്യം *aData;                /* The data */
  പൂർണ്ണസംഖ്യ nData;                  /* Number of elements */
  പൂർണ്ണസംഖ്യ mFlags;                 /* Control flags */
  ശൂന്യം (*xDel)(ശൂന്യം*);        /* Destructor for aData */
  ശൂന്യം *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
തരനിർവചനം ഘടന carray_cursor carray_cursor;
ഘടന carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  ശൂന്യം *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned അക്ഷരം eType;       /* One of the CARRAY_type values */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  sqlite3_vtab *pNew;
  പൂർണ്ണസംഖ്യ rc;

/* Column numbers */
#define CARRAY_COLUMN_മൂല്യം   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_എണ്ണം   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  എങ്കിൽ( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( വലുപ്പം(*pNew) );
    എങ്കിൽ( pNew==0 ) തിരികെ SQLITE_NOMEM;
    memset(pNew, 0, വലുപ്പം(*pNew));
  }
  തിരികെ rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  തിരികെ SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( വലുപ്പം(*pCur) );
  എങ്കിൽ( pCur==0 ) തിരികെ SQLITE_NOMEM;
  memset(pCur, 0, വലുപ്പം(*pCur));
  *ppCursor = &pCur->base;
  തിരികെ SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  തിരികെ SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  തിരികെ SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  പൂർണ്ണസംഖ്യ i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  തിരഞ്ഞെടുപ്പ്( i ){
    അവസ്ഥ CARRAY_COLUMN_POINTER:   തിരികെ SQLITE_OK;
    അവസ്ഥ CARRAY_COLUMN_എണ്ണം:     x = pCur->iCnt;   നിർത്തുക;
    അവസ്ഥ CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      തിരികെ SQLITE_OK;
    }
    സ്ഥിരസ്ഥിതി: {
      തിരഞ്ഞെടുപ്പ്( pCur->eType ){
        അവസ്ഥ CARRAY_INT32: {
          പൂർണ്ണസംഖ്യ *p = (പൂർണ്ണസംഖ്യ*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          തിരികെ SQLITE_OK;
        }
        അവസ്ഥ CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          തിരികെ SQLITE_OK;
        }
        അവസ്ഥ CARRAY_DOUBLE: {
          ഇരട്ട *p = (ഇരട്ട*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          തിരികെ SQLITE_OK;
        }
        അവസ്ഥ CARRAY_TEXT: {
          സ്ഥിരം അക്ഷരം **p = (സ്ഥിരം അക്ഷരം**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          തിരികെ SQLITE_OK;
        }
        സ്ഥിരസ്ഥിതി: {
          സ്ഥിരം ഘടന iovec *p = (ഘടന iovec*)pCur->pPtr;
          പ്രതിജ്ഞാ( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          എങ്കിൽ( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            തിരികെ SQLITE_TOOBIG;
          }അല്ലെങ്കിൽ{
            sqlite3_result_blob(ctx, p->iov_base, (പൂർണ്ണസംഖ്യ)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          തിരികെ SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  തിരികെ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  തിരികെ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  തിരികെ pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  തിരഞ്ഞെടുപ്പ്( idx_അക്കം ){
    അവസ്ഥ 1: {
      carray_bind *pBind = sqlite3_മൂല്യം_pointer(argv[0], "carray-bind");
      എങ്കിൽ( pBind==0 ) നിർത്തുക;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      നിർത്തുക;
    }
    അവസ്ഥ 2:
    അവസ്ഥ 3: {
      pCur->pPtr = sqlite3_മൂല്യം_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_മൂല്യം_int64(argv[1]) : 0;
      എങ്കിൽ( idx_അക്കം<3 ){
        pCur->eType = CARRAY_INT32;
      }അല്ലെങ്കിൽ{
        unsigned അക്ഷരം i;
        സ്ഥിരം അക്ഷരം *zType = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[2]);
        വേണ്ടി(i=0; i<വലുപ്പം(azCarrayType)/വലുപ്പം(azCarrayType[0]); i++){
          എങ്കിൽ( sqlite3_stricmp(zType, azCarrayType[i])==0 ) നിർത്തുക;
        }
        എങ്കിൽ( i>=വലുപ്പം(azCarrayType)/വലുപ്പം(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_സന്ദേശം = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          തിരികെ SQLITE_ERROR;
        }അല്ലെങ്കിൽ{
          pCur->eType = i;
        }
      }
      നിർത്തുക;
    }
  }
  pCur->iRowid = 1;
  തിരികെ SQLITE_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  പൂർണ്ണസംഖ്യ i;                 /* Loop over constraints */
  പൂർണ്ണസംഖ്യ ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  പൂർണ്ണസംഖ്യ cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  പൂർണ്ണസംഖ്യ ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  സ്ഥിരം ഘടന sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  വേണ്ടി(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    എങ്കിൽ( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) തുടരുക;
    എങ്കിൽ( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    എങ്കിൽ( pConstraint->usable==0 ) തുടരുക;
    തിരഞ്ഞെടുപ്പ്( pConstraint->iColumn ){
      അവസ്ഥ CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        നിർത്തുക;
      അവസ്ഥ CARRAY_COLUMN_എണ്ണം:
        cntIdx = i;
        നിർത്തുക;
      അവസ്ഥ CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        നിർത്തുക;
    }
  }
  എങ്കിൽ( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (ഇരട്ട)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_അക്കം = 1;
    എങ്കിൽ( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_അക്കം = 2;
      എങ്കിൽ( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_അക്കം = 3;
      }അല്ലെങ്കിൽ എങ്കിൽ( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        തിരികെ SQLITE_CONSTRAINT;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( seen & (1<<CARRAY_COLUMN_എണ്ണം) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      തിരികെ SQLITE_CONSTRAINT;
    }
  }അല്ലെങ്കിൽ{
    pIdxInfo->estimatedCost = (ഇരട്ട)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_അക്കം = 0;
  }
  തിരികെ SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
സ്ഥിതം sqlite3_module carrayModule = {
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
സ്ഥിതം ശൂന്യം carrayBindDel(ശൂന്യം *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  എങ്കിൽ( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API പൂർണ്ണസംഖ്യ sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  പൂർണ്ണസംഖ്യ idx,
  ശൂന്യം *aData,
  പൂർണ്ണസംഖ്യ nData,
  പൂർണ്ണസംഖ്യ mFlags,
  ശൂന്യം (*xDestroy)(ശൂന്യം*),
  ശൂന്യം *pDestroy
){
  carray_bind *pNew = 0;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  പ്രതിജ്ഞാ( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  പ്രതിജ്ഞാ( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  എങ്കിൽ( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    പോകുക carray_bind_error;
  }

  pNew = sqlite3_malloc64(വലുപ്പം(*pNew));
  എങ്കിൽ( pNew==0 ){
    rc = SQLITE_NOMEM;
    പോകുക carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  എങ്കിൽ( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    തിരഞ്ഞെടുപ്പ്( mFlags ){
      അവസ്ഥ CARRAY_INT32:   sz *= 4;                     നിർത്തുക;
      അവസ്ഥ CARRAY_INT64:   sz *= 8;                     നിർത്തുക;
      അവസ്ഥ CARRAY_DOUBLE:  sz *= 8;                     നിർത്തുക;
      അവസ്ഥ CARRAY_TEXT:    sz *= വലുപ്പം(അക്ഷരം*);         നിർത്തുക;
      സ്ഥിരസ്ഥിതി:             sz *= വലുപ്പം(ഘടന iovec);  നിർത്തുക;
    }
    എങ്കിൽ( mFlags==CARRAY_TEXT ){
      വേണ്ടി(i=0; i<nData; i++){
        സ്ഥിരം അക്ഷരം *z = ((അക്ഷരം**)aData)[i];
        എങ്കിൽ( z ) sz += strlen(z) + 1;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( mFlags==CARRAY_BLOB ){
      വേണ്ടി(i=0; i<nData; i++){
        sz += ((ഘടന iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    എങ്കിൽ( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      പോകുക carray_bind_error;
    }

    എങ്കിൽ( mFlags==CARRAY_TEXT ){
      അക്ഷരം **az = (അക്ഷരം**)pNew->aData;
      അക്ഷരം *z = (അക്ഷരം*)&az[nData];
      വേണ്ടി(i=0; i<nData; i++){
        സ്ഥിരം അക്ഷരം *zData = ((അക്ഷരം**)aData)[i];
        sqlite3_int64 n;
        എങ്കിൽ( zData==0 ){
          az[i] = 0;
          തുടരുക;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( mFlags==CARRAY_BLOB ){
      ഘടന iovec *p = (ഘടന iovec*)pNew->aData;
      unsigned അക്ഷരം *z = (unsigned അക്ഷരം*)&p[nData];
      വേണ്ടി(i=0; i<nData; i++){
        size_t n = ((ഘടന iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((ഘടന iovec*)aData)[i].iov_base, n);
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }അല്ലെങ്കിൽ{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  തിരികെ sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  എങ്കിൽ( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  തിരികെ rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API പൂർണ്ണസംഖ്യ sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  പൂർണ്ണസംഖ്യ idx,
  ശൂന്യം *aData,
  പൂർണ്ണസംഖ്യ nData,
  പൂർണ്ണസംഖ്യ mFlags,
  ശൂന്യം (*xDestroy)(ശൂന്യം*)
){
  തിരികെ sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  തിരികെ sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
