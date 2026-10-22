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
#ಯದಿ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಯದಿ defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  ಸಂರಚನಾ iovec {
    ಶೂನ್ಯ *iov_base;
    size_t iov_len;
  };
#ಅನ್ಯಥಾ
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ carray_bind carray_bind;
ಸಂರಚನಾ carray_bind {
  ಶೂನ್ಯ *aData;                /* The data */
  ಪೂರ್ಣಾಙ್ಕ nData;                  /* Number of elements */
  ಪೂರ್ಣಾಙ್ಕ mFlags;                 /* Control flags */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*);        /* Destructor for aData */
  ಶೂನ್ಯ *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ carray_cursor carray_cursor;
ಸಂರಚನಾ carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  ಶೂನ್ಯ *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned ವರ್ಣ eType;       /* One of the CARRAY_type values */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  sqlite3_vtab *pNew;
  ಪೂರ್ಣಾಙ್ಕ rc;

/* Column numbers */
#define CARRAY_COLUMN_ಮೌಲ್ಯ   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_ಎಣಿಕೆ   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  ಯದಿ( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( ಪರಿಮಾಣ(*pNew) );
    ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    memset(pNew, 0, ಪರಿಮಾಣ(*pNew));
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( ಪರಿಮಾಣ(*pCur) );
  ಯದಿ( pCur==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pCur, 0, ಪರಿಮಾಣ(*pCur));
  *ppCursor = &pCur->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ಪೂರ್ಣಾಙ್ಕ i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  ವಿಕಲ್ಪ( i ){
    ಪಕ್ಷ CARRAY_COLUMN_POINTER:   ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    ಪಕ್ಷ CARRAY_COLUMN_ಎಣಿಕೆ:     x = pCur->iCnt;   ಭಙ್ಗ;
    ಪಕ್ಷ CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    }
    ಸ್ವಭಾವ: {
      ವಿಕಲ್ಪ( pCur->eType ){
        ಪಕ್ಷ CARRAY_INT32: {
          ಪೂರ್ಣಾಙ್ಕ *p = (ಪೂರ್ಣಾಙ್ಕ*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
        }
        ಪಕ್ಷ CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
        }
        ಪಕ್ಷ CARRAY_DOUBLE: {
          ದ್ವಿಗುಣ *p = (ದ್ವಿಗುಣ*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
        }
        ಪಕ್ಷ CARRAY_TEXT: {
          ಧ್ರುವ ವರ್ಣ **p = (ಧ್ರುವ ವರ್ಣ**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
        }
        ಸ್ವಭಾವ: {
          ಧ್ರುವ ಸಂರಚನಾ iovec *p = (ಸಂರಚನಾ iovec*)pCur->pPtr;
          ಪ್ರತಿಜ್ಞಾ( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          ಯದಿ( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            ಪ್ರತ್ಯಾಗಮ SQLITE_TOOBIG;
          }ಅನ್ಯಥಾ{
            sqlite3_result_blob(ctx, p->iov_base, (ಪೂರ್ಣಾಙ್ಕ)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  ಪ್ರತ್ಯಾಗಮ pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  ವಿಕಲ್ಪ( idx_ಅಂಕೆ ){
    ಪಕ್ಷ 1: {
      carray_bind *pBind = sqlite3_ಮೌಲ್ಯ_pointer(argv[0], "carray-bind");
      ಯದಿ( pBind==0 ) ಭಙ್ಗ;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 2:
    ಪಕ್ಷ 3: {
      pCur->pPtr = sqlite3_ಮೌಲ್ಯ_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_ಮೌಲ್ಯ_int64(argv[1]) : 0;
      ಯದಿ( idx_ಅಂಕೆ<3 ){
        pCur->eType = CARRAY_INT32;
      }ಅನ್ಯಥಾ{
        unsigned ವರ್ಣ i;
        ಧ್ರುವ ವರ್ಣ *zType = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[2]);
        ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(azCarrayType)/ಪರಿಮಾಣ(azCarrayType[0]); i++){
          ಯದಿ( sqlite3_stricmp(zType, azCarrayType[i])==0 ) ಭಙ್ಗ;
        }
        ಯದಿ( i>=ಪರಿಮಾಣ(azCarrayType)/ಪರಿಮಾಣ(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_ಸಂದೇಶ = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
        }ಅನ್ಯಥಾ{
          pCur->eType = i;
        }
      }
      ಭಙ್ಗ;
    }
  }
  pCur->iRowid = 1;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  ಪೂರ್ಣಾಙ್ಕ i;                 /* Loop over constraints */
  ಪೂರ್ಣಾಙ್ಕ ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  ಪೂರ್ಣಾಙ್ಕ cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  ಪೂರ್ಣಾಙ್ಕ ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  ಧ್ರುವ ಸಂರಚನಾ sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  ಪ್ರತಿ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ಯದಿ( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ಅನುವರ್ತ;
    ಯದಿ( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    ಯದಿ( pConstraint->usable==0 ) ಅನುವರ್ತ;
    ವಿಕಲ್ಪ( pConstraint->iColumn ){
      ಪಕ್ಷ CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        ಭಙ್ಗ;
      ಪಕ್ಷ CARRAY_COLUMN_ಎಣಿಕೆ:
        cntIdx = i;
        ಭಙ್ಗ;
      ಪಕ್ಷ CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        ಭಙ್ಗ;
    }
  }
  ಯದಿ( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (ದ್ವಿಗುಣ)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_ಅಂಕೆ = 1;
    ಯದಿ( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_ಅಂಕೆ = 2;
      ಯದಿ( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_ಅಂಕೆ = 3;
      }ಅನ್ಯಥಾ ಯದಿ( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        ಪ್ರತ್ಯಾಗಮ SQLITE_CONSTRAINT;
      }
    }ಅನ್ಯಥಾ ಯದಿ( seen & (1<<CARRAY_COLUMN_ಎಣಿಕೆ) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      ಪ್ರತ್ಯಾಗಮ SQLITE_CONSTRAINT;
    }
  }ಅನ್ಯಥಾ{
    pIdxInfo->estimatedCost = (ದ್ವಿಗುಣ)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_ಅಂಕೆ = 0;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
ಸ್ಥಿರ sqlite3_module carrayModule = {
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
ಸ್ಥಿರ ಶೂನ್ಯ carrayBindDel(ಶೂನ್ಯ *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  ಯದಿ( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API ಪೂರ್ಣಾಙ್ಕ sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಙ್ಕ idx,
  ಶೂನ್ಯ *aData,
  ಪೂರ್ಣಾಙ್ಕ nData,
  ಪೂರ್ಣಾಙ್ಕ mFlags,
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*),
  ಶೂನ್ಯ *pDestroy
){
  carray_bind *pNew = 0;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  ಪ್ರತಿಜ್ಞಾ( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  ಪ್ರತಿಜ್ಞಾ( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  ಯದಿ( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    ಗಚ್ಛ carray_bind_error;
  }

  pNew = sqlite3_malloc64(ಪರಿಮಾಣ(*pNew));
  ಯದಿ( pNew==0 ){
    rc = SQLITE_NOMEM;
    ಗಚ್ಛ carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  ಯದಿ( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    ವಿಕಲ್ಪ( mFlags ){
      ಪಕ್ಷ CARRAY_INT32:   sz *= 4;                     ಭಙ್ಗ;
      ಪಕ್ಷ CARRAY_INT64:   sz *= 8;                     ಭಙ್ಗ;
      ಪಕ್ಷ CARRAY_DOUBLE:  sz *= 8;                     ಭಙ್ಗ;
      ಪಕ್ಷ CARRAY_TEXT:    sz *= ಪರಿಮಾಣ(ವರ್ಣ*);         ಭಙ್ಗ;
      ಸ್ವಭಾವ:             sz *= ಪರಿಮಾಣ(ಸಂರಚನಾ iovec);  ಭಙ್ಗ;
    }
    ಯದಿ( mFlags==CARRAY_TEXT ){
      ಪ್ರತಿ(i=0; i<nData; i++){
        ಧ್ರುವ ವರ್ಣ *z = ((ವರ್ಣ**)aData)[i];
        ಯದಿ( z ) sz += strlen(z) + 1;
      }
    }ಅನ್ಯಥಾ ಯದಿ( mFlags==CARRAY_BLOB ){
      ಪ್ರತಿ(i=0; i<nData; i++){
        sz += ((ಸಂರಚನಾ iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    ಯದಿ( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      ಗಚ್ಛ carray_bind_error;
    }

    ಯದಿ( mFlags==CARRAY_TEXT ){
      ವರ್ಣ **az = (ವರ್ಣ**)pNew->aData;
      ವರ್ಣ *z = (ವರ್ಣ*)&az[nData];
      ಪ್ರತಿ(i=0; i<nData; i++){
        ಧ್ರುವ ವರ್ಣ *zData = ((ವರ್ಣ**)aData)[i];
        sqlite3_int64 n;
        ಯದಿ( zData==0 ){
          az[i] = 0;
          ಅನುವರ್ತ;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }ಅನ್ಯಥಾ ಯದಿ( mFlags==CARRAY_BLOB ){
      ಸಂರಚನಾ iovec *p = (ಸಂರಚನಾ iovec*)pNew->aData;
      unsigned ವರ್ಣ *z = (unsigned ವರ್ಣ*)&p[nData];
      ಪ್ರತಿ(i=0; i<nData; i++){
        size_t n = ((ಸಂರಚನಾ iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((ಸಂರಚನಾ iovec*)aData)[i].iov_base, n);
      }
    }ಅನ್ಯಥಾ ಯದಿ( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }ಅನ್ಯಥಾ{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  ಯದಿ( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API ಪೂರ್ಣಾಙ್ಕ sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಙ್ಕ idx,
  ಶೂನ್ಯ *aData,
  ಪೂರ್ಣಾಙ್ಕ nData,
  ಪೂರ್ಣಾಙ್ಕ mFlags,
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*)
){
  ಪ್ರತ್ಯಾಗಮ sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  ಪ್ರತ್ಯಾಗಮ sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
