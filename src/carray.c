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
#אם !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#ضمن "sqliteInt.h"
#אם defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  بنية iovec {
    فراغ *iov_base;
    size_t iov_len;
  };
#אחרת
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
ساكن ثابت حرف *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
تعريفنوع بنية carray_bind carray_bind;
بنية carray_bind {
  فراغ *aData;                /* The data */
  صحيح nData;                  /* Number of elements */
  صحيح mFlags;                 /* Control flags */
  فراغ (*xDel)(فراغ*);        /* Destructor for aData */
  فراغ *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
تعريفنوع بنية carray_cursor carray_cursor;
بنية carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  فراغ *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned حرف eType;       /* One of the CARRAY_type values */
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
ساكن صحيح carrayConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  sqlite3_vtab *pNew;
  صحيح rc;

/* Column numbers */
#define CARRAY_COLUMN_VALUE   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_COUNT   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  אם( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( حجم(*pNew) );
    אם( pNew==0 ) החזר SQLITE_NOMEM;
    memset(pNew, 0, حجم(*pNew));
  }
  החזר rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
ساكن صحيح carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  החזר SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
ساكن صحيح carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( حجم(*pCur) );
  אם( pCur==0 ) החזר SQLITE_NOMEM;
  memset(pCur, 0, حجم(*pCur));
  *ppCursor = &pCur->base;
  החזר SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
ساكن صحيح carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  החזר SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
ساكن صحيح carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  החזר SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
ساكن صحيح carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحيح i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  بدل( i ){
    حالة CARRAY_COLUMN_POINTER:   החזר SQLITE_OK;
    حالة CARRAY_COLUMN_COUNT:     x = pCur->iCnt;   اكسر;
    حالة CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      החזר SQLITE_OK;
    }
    افتراضي: {
      بدل( pCur->eType ){
        حالة CARRAY_INT32: {
          صحيح *p = (صحيح*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          החזר SQLITE_OK;
        }
        حالة CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          החזר SQLITE_OK;
        }
        حالة CARRAY_DOUBLE: {
          مزدوج *p = (مزدوج*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          החזר SQLITE_OK;
        }
        حالة CARRAY_TEXT: {
          ثابت حرف **p = (ثابت حرف**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          החזר SQLITE_OK;
        }
        افتراضي: {
          ثابت بنية iovec *p = (بنية iovec*)pCur->pPtr;
          أكد( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          אם( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            החזר SQLITE_TOOBIG;
          }אחרת{
            sqlite3_result_blob(ctx, p->iov_base, (صحيح)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          החזר SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  החזר SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ساكن صحيح carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  החזר SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ساكن صحيح carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  החזר pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
ساكن صحيح carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idxNum, ثابت حرف *idxStr,
  صحيح argc, sqlite3_value **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  بدل( idxNum ){
    حالة 1: {
      carray_bind *pBind = sqlite3_value_pointer(argv[0], "carray-bind");
      אם( pBind==0 ) اكسر;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      اكسر;
    }
    حالة 2:
    حالة 3: {
      pCur->pPtr = sqlite3_value_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_value_int64(argv[1]) : 0;
      אם( idxNum<3 ){
        pCur->eType = CARRAY_INT32;
      }אחרת{
        unsigned حرف i;
        ثابت حرف *zType = (ثابت حرف*)sqlite3_value_text(argv[2]);
        עבור(i=0; i<حجم(azCarrayType)/حجم(azCarrayType[0]); i++){
          אם( sqlite3_stricmp(zType, azCarrayType[i])==0 ) اكسر;
        }
        אם( i>=حجم(azCarrayType)/حجم(azCarrayType[0]) ){
          pVtabCursor->pVtab->zErrMsg = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          החזר SQLITE_ERROR;
        }אחרת{
          pCur->eType = i;
        }
      }
      اكسر;
    }
  }
  pCur->iRowid = 1;
  החזר SQLITE_OK;
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
ساكن صحيح carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  صحيح i;                 /* Loop over constraints */
  صحيح ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  صحيح cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  صحيح ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  ثابت بنية sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  עבור(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    אם( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) استمر;
    אם( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    אם( pConstraint->usable==0 ) استمر;
    بدل( pConstraint->iColumn ){
      حالة CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        اكسر;
      حالة CARRAY_COLUMN_COUNT:
        cntIdx = i;
        اكسر;
      حالة CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        اكسر;
    }
  }
  אם( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (مزدوج)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idxNum = 1;
    אם( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idxNum = 2;
      אם( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idxNum = 3;
      }אחרת אם( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        החזר SQLITE_CONSTRAINT;
      }
    }אחרת אם( seen & (1<<CARRAY_COLUMN_COUNT) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      החזר SQLITE_CONSTRAINT;
    }
  }אחרת{
    pIdxInfo->estimatedCost = (مزدوج)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idxNum = 0;
  }
  החזר SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
ساكن sqlite3_module carrayModule = {
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
ساكن فراغ carrayBindDel(فراغ *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  אם( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API صحيح sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  صحيح idx,
  فراغ *aData,
  صحيح nData,
  صحيح mFlags,
  فراغ (*xDestroy)(فراغ*),
  فراغ *pDestroy
){
  carray_bind *pNew = 0;
  صحيح i;
  صحيح rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  أكد( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  أكد( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  אם( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    اذهب carray_bind_error;
  }

  pNew = sqlite3_malloc64(حجم(*pNew));
  אם( pNew==0 ){
    rc = SQLITE_NOMEM;
    اذهب carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  אם( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    بدل( mFlags ){
      حالة CARRAY_INT32:   sz *= 4;                     اكسر;
      حالة CARRAY_INT64:   sz *= 8;                     اكسر;
      حالة CARRAY_DOUBLE:  sz *= 8;                     اكسر;
      حالة CARRAY_TEXT:    sz *= حجم(حرف*);         اكسر;
      افتراضي:             sz *= حجم(بنية iovec);  اكسر;
    }
    אם( mFlags==CARRAY_TEXT ){
      עבור(i=0; i<nData; i++){
        ثابت حرف *z = ((حرف**)aData)[i];
        אם( z ) sz += strlen(z) + 1;
      }
    }אחרת אם( mFlags==CARRAY_BLOB ){
      עבור(i=0; i<nData; i++){
        sz += ((بنية iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    אם( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      اذهب carray_bind_error;
    }

    אם( mFlags==CARRAY_TEXT ){
      حرف **az = (حرف**)pNew->aData;
      حرف *z = (حرف*)&az[nData];
      עבור(i=0; i<nData; i++){
        ثابت حرف *zData = ((حرف**)aData)[i];
        sqlite3_int64 n;
        אם( zData==0 ){
          az[i] = 0;
          استمر;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }אחרת אם( mFlags==CARRAY_BLOB ){
      بنية iovec *p = (بنية iovec*)pNew->aData;
      unsigned حرف *z = (unsigned حرف*)&p[nData];
      עבור(i=0; i<nData; i++){
        size_t n = ((بنية iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((بنية iovec*)aData)[i].iov_base, n);
      }
    }אחרת אם( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }אחרת{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  החזר sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  אם( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  החזר rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API صحيح sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  صحيح idx,
  فراغ *aData,
  صحيح nData,
  صحيح mFlags,
  فراغ (*xDestroy)(فراغ*)
){
  החזר sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  החזר sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
