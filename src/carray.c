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
#ଯଦି !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଯଦି defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  ସଂରଚନା iovec {
    ଶୂନ୍ୟ *iov_base;
    size_t iov_len;
  };
#ଅନ୍ୟଥା
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା carray_bind carray_bind;
ସଂରଚନା carray_bind {
  ଶୂନ୍ୟ *aData;                /* The data */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData;                  /* Number of elements */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mFlags;                 /* Control flags */
  ଶୂନ୍ୟ (*xDel)(ଶୂନ୍ୟ*);        /* Destructor for aData */
  ଶୂନ୍ୟ *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା carray_cursor carray_cursor;
ସଂରଚନା carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  ଶୂନ୍ୟ *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned ଅକ୍ଷର eType;       /* One of the CARRAY_type values */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  sqlite3_vtab *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

/* Column numbers */
#define CARRAY_COLUMN_ମୂଲ୍ୟ   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_ଗଣନା   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  ଯଦି( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( ଆକାର(*pNew) );
    ଯଦି( pNew==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
    memset(pNew, 0, ଆକାର(*pNew));
  }
  ଫେରନ୍ତୁ rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( ଆକାର(*pCur) );
  ଯଦି( pCur==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(pCur, 0, ଆକାର(*pCur));
  *ppCursor = &pCur->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  ଫେରନ୍ତୁ SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  ବାଛ( i ){
    ସ୍ଥିତି CARRAY_COLUMN_POINTER:   ଫେରନ୍ତୁ SQLITE_OK;
    ସ୍ଥିତି CARRAY_COLUMN_ଗଣନା:     x = pCur->iCnt;   ଭାଙ୍ଗ;
    ସ୍ଥିତି CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ବାଛ( pCur->eType ){
        ସ୍ଥିତି CARRAY_INT32: {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା *p = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          ଫେରନ୍ତୁ SQLITE_OK;
        }
        ସ୍ଥିତି CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          ଫେରନ୍ତୁ SQLITE_OK;
        }
        ସ୍ଥିତି CARRAY_DOUBLE: {
          ଦ୍ୱିଗୁଣ *p = (ଦ୍ୱିଗୁଣ*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          ଫେରନ୍ତୁ SQLITE_OK;
        }
        ସ୍ଥିତି CARRAY_TEXT: {
          ଅଚଳ ଅକ୍ଷର **p = (ଅଚଳ ଅକ୍ଷର**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          ଫେରନ୍ତୁ SQLITE_OK;
        }
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
          ଅଚଳ ସଂରଚନା iovec *p = (ସଂରଚନା iovec*)pCur->pPtr;
          ପ୍ରତିଜ୍ଞା( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          ଯଦି( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            ଫେରନ୍ତୁ SQLITE_TOOBIG;
          }ଅନ୍ୟଥା{
            sqlite3_result_blob(ctx, p->iov_base, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          ଫେରନ୍ତୁ SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  ଫେରନ୍ତୁ pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  ବାଛ( idx_ଅଙ୍କ ){
    ସ୍ଥିତି 1: {
      carray_bind *pBind = sqlite3_ମୂଲ୍ୟ_pointer(argv[0], "carray-bind");
      ଯଦି( pBind==0 ) ଭାଙ୍ଗ;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 2:
    ସ୍ଥିତି 3: {
      pCur->pPtr = sqlite3_ମୂଲ୍ୟ_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_ମୂଲ୍ୟ_int64(argv[1]) : 0;
      ଯଦି( idx_ଅଙ୍କ<3 ){
        pCur->eType = CARRAY_INT32;
      }ଅନ୍ୟଥା{
        unsigned ଅକ୍ଷର i;
        ଅଚଳ ଅକ୍ଷର *zType = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[2]);
        ପାଇଁ(i=0; i<ଆକାର(azCarrayType)/ଆକାର(azCarrayType[0]); i++){
          ଯଦି( sqlite3_stricmp(zType, azCarrayType[i])==0 ) ଭାଙ୍ଗ;
        }
        ଯଦି( i>=ଆକାର(azCarrayType)/ଆକାର(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_ସନ୍ଦେଶ = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          ଫେରନ୍ତୁ SQLITE_ERROR;
        }ଅନ୍ୟଥା{
          pCur->eType = i;
        }
      }
      ଭାଙ୍ଗ;
    }
  }
  pCur->iRowid = 1;
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                 /* Loop over constraints */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  ଅଚଳ ସଂରଚନା sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  ପାଇଁ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ଯଦି( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ଜାରି;
    ଯଦି( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    ଯଦି( pConstraint->usable==0 ) ଜାରି;
    ବାଛ( pConstraint->iColumn ){
      ସ୍ଥିତି CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି CARRAY_COLUMN_ଗଣନା:
        cntIdx = i;
        ଭାଙ୍ଗ;
      ସ୍ଥିତି CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        ଭାଙ୍ଗ;
    }
  }
  ଯଦି( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (ଦ୍ୱିଗୁଣ)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_ଅଙ୍କ = 1;
    ଯଦି( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_ଅଙ୍କ = 2;
      ଯଦି( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_ଅଙ୍କ = 3;
      }ଅନ୍ୟଥା ଯଦି( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        ଫେରନ୍ତୁ SQLITE_CONSTRAINT;
      }
    }ଅନ୍ୟଥା ଯଦି( seen & (1<<CARRAY_COLUMN_ଗଣନା) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      ଫେରନ୍ତୁ SQLITE_CONSTRAINT;
    }
  }ଅନ୍ୟଥା{
    pIdxInfo->estimatedCost = (ଦ୍ୱିଗୁଣ)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_ଅଙ୍କ = 0;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
ସ୍ଥିର sqlite3_module carrayModule = {
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
ସ୍ଥିର ଶୂନ୍ୟ carrayBindDel(ଶୂନ୍ୟ *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  ଯଦି( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx,
  ଶୂନ୍ୟ *aData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mFlags,
  ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ*),
  ଶୂନ୍ୟ *pDestroy
){
  carray_bind *pNew = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  ପ୍ରତିଜ୍ଞା( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  ପ୍ରତିଜ୍ଞା( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  ଯଦି( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    ଯାଅ carray_bind_error;
  }

  pNew = sqlite3_malloc64(ଆକାର(*pNew));
  ଯଦି( pNew==0 ){
    rc = SQLITE_NOMEM;
    ଯାଅ carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  ଯଦି( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    ବାଛ( mFlags ){
      ସ୍ଥିତି CARRAY_INT32:   sz *= 4;                     ଭାଙ୍ଗ;
      ସ୍ଥିତି CARRAY_INT64:   sz *= 8;                     ଭାଙ୍ଗ;
      ସ୍ଥିତି CARRAY_DOUBLE:  sz *= 8;                     ଭାଙ୍ଗ;
      ସ୍ଥିତି CARRAY_TEXT:    sz *= ଆକାର(ଅକ୍ଷର*);         ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:             sz *= ଆକାର(ସଂରଚନା iovec);  ଭାଙ୍ଗ;
    }
    ଯଦି( mFlags==CARRAY_TEXT ){
      ପାଇଁ(i=0; i<nData; i++){
        ଅଚଳ ଅକ୍ଷର *z = ((ଅକ୍ଷର**)aData)[i];
        ଯଦି( z ) sz += strlen(z) + 1;
      }
    }ଅନ୍ୟଥା ଯଦି( mFlags==CARRAY_BLOB ){
      ପାଇଁ(i=0; i<nData; i++){
        sz += ((ସଂରଚନା iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    ଯଦି( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      ଯାଅ carray_bind_error;
    }

    ଯଦି( mFlags==CARRAY_TEXT ){
      ଅକ୍ଷର **az = (ଅକ୍ଷର**)pNew->aData;
      ଅକ୍ଷର *z = (ଅକ୍ଷର*)&az[nData];
      ପାଇଁ(i=0; i<nData; i++){
        ଅଚଳ ଅକ୍ଷର *zData = ((ଅକ୍ଷର**)aData)[i];
        sqlite3_int64 n;
        ଯଦି( zData==0 ){
          az[i] = 0;
          ଜାରି;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }ଅନ୍ୟଥା ଯଦି( mFlags==CARRAY_BLOB ){
      ସଂରଚନା iovec *p = (ସଂରଚନା iovec*)pNew->aData;
      unsigned ଅକ୍ଷର *z = (unsigned ଅକ୍ଷର*)&p[nData];
      ପାଇଁ(i=0; i<nData; i++){
        size_t n = ((ସଂରଚନା iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((ସଂରଚନା iovec*)aData)[i].iov_base, n);
      }
    }ଅନ୍ୟଥା ଯଦି( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }ଅନ୍ୟଥା{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  ଫେରନ୍ତୁ sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  ଯଦି( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  ଫେରନ୍ତୁ rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx,
  ଶୂନ୍ୟ *aData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mFlags,
  ଶୂନ୍ୟ (*xDestroy)(ଶୂନ୍ୟ*)
){
  ଫେରନ୍ତୁ sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  ଫେରନ୍ତୁ sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
