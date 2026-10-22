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
#اگر !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#شامل "sqliteInt.h"
#اگر defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  ساختار iovec {
    تهی *iov_base;
    size_t iov_len;
  };
#وگرنه
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
ایستا ثابت نویسه *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
تعریفنوع ساختار carray_bind carray_bind;
ساختار carray_bind {
  تهی *aData;                /* The data */
  صحیح nData;                  /* Number of elements */
  صحیح mFlags;                 /* Control flags */
  تهی (*xDel)(تهی*);        /* Destructor for aData */
  تهی *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
تعریفنوع ساختار carray_cursor carray_cursor;
ساختار carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  تهی *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned نویسه eType;       /* One of the CARRAY_type values */
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
ایستا صحیح carrayConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  sqlite3_vtab *pNew;
  صحیح rc;

/* Column numbers */
#define CARRAY_COLUMN_مقدار   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_شمار   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  اگر( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( اندازه(*pNew) );
    اگر( pNew==0 ) بازگشت SQLITE_NOMEM;
    memset(pNew, 0, اندازه(*pNew));
  }
  بازگشت rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
ایستا صحیح carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  بازگشت SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
ایستا صحیح carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( اندازه(*pCur) );
  اگر( pCur==0 ) بازگشت SQLITE_NOMEM;
  memset(pCur, 0, اندازه(*pCur));
  *ppCursor = &pCur->base;
  بازگشت SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
ایستا صحیح carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  بازگشت SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
ایستا صحیح carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  بازگشت SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
ایستا صحیح carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحیح i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  انتخاب( i ){
    حالت CARRAY_COLUMN_POINTER:   بازگشت SQLITE_OK;
    حالت CARRAY_COLUMN_شمار:     x = pCur->iCnt;   بشکن;
    حالت CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      بازگشت SQLITE_OK;
    }
    پیشفرض: {
      انتخاب( pCur->eType ){
        حالت CARRAY_INT32: {
          صحیح *p = (صحیح*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          بازگشت SQLITE_OK;
        }
        حالت CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          بازگشت SQLITE_OK;
        }
        حالت CARRAY_DOUBLE: {
          دوگانه *p = (دوگانه*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          بازگشت SQLITE_OK;
        }
        حالت CARRAY_TEXT: {
          ثابت نویسه **p = (ثابت نویسه**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          بازگشت SQLITE_OK;
        }
        پیشفرض: {
          ثابت ساختار iovec *p = (ساختار iovec*)pCur->pPtr;
          أكد( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          اگر( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            بازگشت SQLITE_TOOBIG;
          }وگرنه{
            sqlite3_result_blob(ctx, p->iov_base, (صحیح)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          بازگشت SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  بازگشت SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ایستا صحیح carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  بازگشت SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ایستا صحیح carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  بازگشت pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
ایستا صحیح carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  انتخاب( idx_رقم ){
    حالت 1: {
      carray_bind *pBind = sqlite3_مقدار_pointer(argv[0], "carray-bind");
      اگر( pBind==0 ) بشکن;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      بشکن;
    }
    حالت 2:
    حالت 3: {
      pCur->pPtr = sqlite3_مقدار_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_مقدار_int64(argv[1]) : 0;
      اگر( idx_رقم<3 ){
        pCur->eType = CARRAY_INT32;
      }وگرنه{
        unsigned نویسه i;
        ثابت نویسه *zType = (ثابت نویسه*)sqlite3_مقدار_text(argv[2]);
        برای(i=0; i<اندازه(azCarrayType)/اندازه(azCarrayType[0]); i++){
          اگر( sqlite3_stricmp(zType, azCarrayType[i])==0 ) بشکن;
        }
        اگر( i>=اندازه(azCarrayType)/اندازه(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_پیام = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          بازگشت SQLITE_ERROR;
        }وگرنه{
          pCur->eType = i;
        }
      }
      بشکن;
    }
  }
  pCur->iRowid = 1;
  بازگشت SQLITE_OK;
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
ایستا صحیح carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  صحیح i;                 /* Loop over constraints */
  صحیح ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  صحیح cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  صحیح ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  ثابت ساختار sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  برای(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    اگر( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ادامه;
    اگر( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    اگر( pConstraint->usable==0 ) ادامه;
    انتخاب( pConstraint->iColumn ){
      حالت CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        بشکن;
      حالت CARRAY_COLUMN_شمار:
        cntIdx = i;
        بشکن;
      حالت CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        بشکن;
    }
  }
  اگر( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (دوگانه)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_رقم = 1;
    اگر( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_رقم = 2;
      اگر( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_رقم = 3;
      }وگرنه اگر( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        بازگشت SQLITE_CONSTRAINT;
      }
    }وگرنه اگر( seen & (1<<CARRAY_COLUMN_شمار) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      بازگشت SQLITE_CONSTRAINT;
    }
  }وگرنه{
    pIdxInfo->estimatedCost = (دوگانه)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_رقم = 0;
  }
  بازگشت SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
ایستا sqlite3_module carrayModule = {
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
ایستا تهی carrayBindDel(تهی *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  اگر( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API صحیح sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  صحیح idx,
  تهی *aData,
  صحیح nData,
  صحیح mFlags,
  تهی (*xDestroy)(تهی*),
  تهی *pDestroy
){
  carray_bind *pNew = 0;
  صحیح i;
  صحیح rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  أكد( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  أكد( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  اگر( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    برو carray_bind_error;
  }

  pNew = sqlite3_malloc64(اندازه(*pNew));
  اگر( pNew==0 ){
    rc = SQLITE_NOMEM;
    برو carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  اگر( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    انتخاب( mFlags ){
      حالت CARRAY_INT32:   sz *= 4;                     بشکن;
      حالت CARRAY_INT64:   sz *= 8;                     بشکن;
      حالت CARRAY_DOUBLE:  sz *= 8;                     بشکن;
      حالت CARRAY_TEXT:    sz *= اندازه(نویسه*);         بشکن;
      پیشفرض:             sz *= اندازه(ساختار iovec);  بشکن;
    }
    اگر( mFlags==CARRAY_TEXT ){
      برای(i=0; i<nData; i++){
        ثابت نویسه *z = ((نویسه**)aData)[i];
        اگر( z ) sz += strlen(z) + 1;
      }
    }وگرنه اگر( mFlags==CARRAY_BLOB ){
      برای(i=0; i<nData; i++){
        sz += ((ساختار iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    اگر( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      برو carray_bind_error;
    }

    اگر( mFlags==CARRAY_TEXT ){
      نویسه **az = (نویسه**)pNew->aData;
      نویسه *z = (نویسه*)&az[nData];
      برای(i=0; i<nData; i++){
        ثابت نویسه *zData = ((نویسه**)aData)[i];
        sqlite3_int64 n;
        اگر( zData==0 ){
          az[i] = 0;
          ادامه;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }وگرنه اگر( mFlags==CARRAY_BLOB ){
      ساختار iovec *p = (ساختار iovec*)pNew->aData;
      unsigned نویسه *z = (unsigned نویسه*)&p[nData];
      برای(i=0; i<nData; i++){
        size_t n = ((ساختار iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((ساختار iovec*)aData)[i].iov_base, n);
      }
    }وگرنه اگر( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }وگرنه{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  بازگشت sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  اگر( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  بازگشت rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API صحیح sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  صحیح idx,
  تهی *aData,
  صحیح nData,
  صحیح mFlags,
  تهی (*xDestroy)(تهی*)
){
  بازگشت sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  بازگشت sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
