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
#எனில் !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#அந்தர்பாவ "sqliteInt.h"
#எனில் defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  கட்டமைப்பு iovec {
    வெறுமை *iov_base;
    size_t iov_len;
  };
#இல்லையெனில்
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
நிலையான மாறிலி எழுத்து *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
வகைவரை கட்டமைப்பு carray_bind carray_bind;
கட்டமைப்பு carray_bind {
  வெறுமை *aData;                /* The data */
  முழு nData;                  /* Number of elements */
  முழு mFlags;                 /* Control flags */
  வெறுமை (*xDel)(வெறுமை*);        /* Destructor for aData */
  வெறுமை *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
வகைவரை கட்டமைப்பு carray_cursor carray_cursor;
கட்டமைப்பு carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  வெறுமை *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned எழுத்து eType;       /* One of the CARRAY_type values */
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
நிலையான முழு carrayConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  sqlite3_vtab *pNew;
  முழு rc;

/* Column numbers */
#define CARRAY_COLUMN_மதிப்பு   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_மொத்தம்   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  எனில்( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( அளவு(*pNew) );
    எனில்( pNew==0 ) திரும்பு SQLITE_NOMEM;
    memset(pNew, 0, அளவு(*pNew));
  }
  திரும்பு rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
நிலையான முழு carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  திரும்பு SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
நிலையான முழு carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( அளவு(*pCur) );
  எனில்( pCur==0 ) திரும்பு SQLITE_NOMEM;
  memset(pCur, 0, அளவு(*pCur));
  *ppCursor = &pCur->base;
  திரும்பு SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
நிலையான முழு carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  திரும்பு SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
நிலையான முழு carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  திரும்பு SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
நிலையான முழு carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  முழு i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  தேர்வு( i ){
    நிலை CARRAY_COLUMN_POINTER:   திரும்பு SQLITE_OK;
    நிலை CARRAY_COLUMN_மொத்தம்:     x = pCur->iCnt;   நிறுத்து;
    நிலை CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      திரும்பு SQLITE_OK;
    }
    இயல்பு: {
      தேர்வு( pCur->eType ){
        நிலை CARRAY_INT32: {
          முழு *p = (முழு*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          திரும்பு SQLITE_OK;
        }
        நிலை CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          திரும்பு SQLITE_OK;
        }
        நிலை CARRAY_DOUBLE: {
          இரட்டை *p = (இரட்டை*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          திரும்பு SQLITE_OK;
        }
        நிலை CARRAY_TEXT: {
          மாறிலி எழுத்து **p = (மாறிலி எழுத்து**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          திரும்பு SQLITE_OK;
        }
        இயல்பு: {
          மாறிலி கட்டமைப்பு iovec *p = (கட்டமைப்பு iovec*)pCur->pPtr;
          ப்ரதிஜ்ஞா( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          எனில்( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            திரும்பு SQLITE_TOOBIG;
          }இல்லையெனில்{
            sqlite3_result_blob(ctx, p->iov_base, (முழு)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          திரும்பு SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  திரும்பு SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
நிலையான முழு carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  திரும்பு SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
நிலையான முழு carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  திரும்பு pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
நிலையான முழு carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  தேர்வு( idx_இலக்கம் ){
    நிலை 1: {
      carray_bind *pBind = sqlite3_மதிப்பு_pointer(argv[0], "carray-bind");
      எனில்( pBind==0 ) நிறுத்து;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      நிறுத்து;
    }
    நிலை 2:
    நிலை 3: {
      pCur->pPtr = sqlite3_மதிப்பு_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_மதிப்பு_int64(argv[1]) : 0;
      எனில்( idx_இலக்கம்<3 ){
        pCur->eType = CARRAY_INT32;
      }இல்லையெனில்{
        unsigned எழுத்து i;
        மாறிலி எழுத்து *zType = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[2]);
        க்கு(i=0; i<அளவு(azCarrayType)/அளவு(azCarrayType[0]); i++){
          எனில்( sqlite3_stricmp(zType, azCarrayType[i])==0 ) நிறுத்து;
        }
        எனில்( i>=அளவு(azCarrayType)/அளவு(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_செய்தி = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          திரும்பு SQLITE_ERROR;
        }இல்லையெனில்{
          pCur->eType = i;
        }
      }
      நிறுத்து;
    }
  }
  pCur->iRowid = 1;
  திரும்பு SQLITE_OK;
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
நிலையான முழு carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  முழு i;                 /* Loop over constraints */
  முழு ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  முழு cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  முழு ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  மாறிலி கட்டமைப்பு sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  க்கு(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    எனில்( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) தொடர்;
    எனில்( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    எனில்( pConstraint->usable==0 ) தொடர்;
    தேர்வு( pConstraint->iColumn ){
      நிலை CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        நிறுத்து;
      நிலை CARRAY_COLUMN_மொத்தம்:
        cntIdx = i;
        நிறுத்து;
      நிலை CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        நிறுத்து;
    }
  }
  எனில்( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (இரட்டை)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_இலக்கம் = 1;
    எனில்( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_இலக்கம் = 2;
      எனில்( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_இலக்கம் = 3;
      }இல்லையெனில் எனில்( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        திரும்பு SQLITE_CONSTRAINT;
      }
    }இல்லையெனில் எனில்( seen & (1<<CARRAY_COLUMN_மொத்தம்) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      திரும்பு SQLITE_CONSTRAINT;
    }
  }இல்லையெனில்{
    pIdxInfo->estimatedCost = (இரட்டை)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_இலக்கம் = 0;
  }
  திரும்பு SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
நிலையான sqlite3_module carrayModule = {
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
நிலையான வெறுமை carrayBindDel(வெறுமை *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  எனில்( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API முழு sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  முழு idx,
  வெறுமை *aData,
  முழு nData,
  முழு mFlags,
  வெறுமை (*xDestroy)(வெறுமை*),
  வெறுமை *pDestroy
){
  carray_bind *pNew = 0;
  முழு i;
  முழு rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  ப்ரதிஜ்ஞா( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  ப்ரதிஜ்ஞா( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  எனில்( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    செல் carray_bind_error;
  }

  pNew = sqlite3_malloc64(அளவு(*pNew));
  எனில்( pNew==0 ){
    rc = SQLITE_NOMEM;
    செல் carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  எனில்( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    தேர்வு( mFlags ){
      நிலை CARRAY_INT32:   sz *= 4;                     நிறுத்து;
      நிலை CARRAY_INT64:   sz *= 8;                     நிறுத்து;
      நிலை CARRAY_DOUBLE:  sz *= 8;                     நிறுத்து;
      நிலை CARRAY_TEXT:    sz *= அளவு(எழுத்து*);         நிறுத்து;
      இயல்பு:             sz *= அளவு(கட்டமைப்பு iovec);  நிறுத்து;
    }
    எனில்( mFlags==CARRAY_TEXT ){
      க்கு(i=0; i<nData; i++){
        மாறிலி எழுத்து *z = ((எழுத்து**)aData)[i];
        எனில்( z ) sz += strlen(z) + 1;
      }
    }இல்லையெனில் எனில்( mFlags==CARRAY_BLOB ){
      க்கு(i=0; i<nData; i++){
        sz += ((கட்டமைப்பு iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    எனில்( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      செல் carray_bind_error;
    }

    எனில்( mFlags==CARRAY_TEXT ){
      எழுத்து **az = (எழுத்து**)pNew->aData;
      எழுத்து *z = (எழுத்து*)&az[nData];
      க்கு(i=0; i<nData; i++){
        மாறிலி எழுத்து *zData = ((எழுத்து**)aData)[i];
        sqlite3_int64 n;
        எனில்( zData==0 ){
          az[i] = 0;
          தொடர்;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }இல்லையெனில் எனில்( mFlags==CARRAY_BLOB ){
      கட்டமைப்பு iovec *p = (கட்டமைப்பு iovec*)pNew->aData;
      unsigned எழுத்து *z = (unsigned எழுத்து*)&p[nData];
      க்கு(i=0; i<nData; i++){
        size_t n = ((கட்டமைப்பு iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((கட்டமைப்பு iovec*)aData)[i].iov_base, n);
      }
    }இல்லையெனில் எனில்( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }இல்லையெனில்{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  திரும்பு sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  எனில்( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  திரும்பு rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API முழு sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  முழு idx,
  வெறுமை *aData,
  முழு nData,
  முழு mFlags,
  வெறுமை (*xDestroy)(வெறுமை*)
){
  திரும்பு sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  திரும்பு sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
