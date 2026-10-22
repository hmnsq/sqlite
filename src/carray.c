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
#যদি !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#অন্তর্ভাৱ "sqliteInt.h"
#যদি defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  সংরচনা iovec {
    শূন্য *iov_base;
    size_t iov_len;
  };
#অন্যথা
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
স্থির ধ্রুৱ ৱর্ণ *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
প্রকারনির্দেশ সংরচনা carray_bind carray_bind;
সংরচনা carray_bind {
  শূন্য *aData;                /* The data */
  পূর্ণাঙ্ক nData;                  /* Number of elements */
  পূর্ণাঙ্ক mFlags;                 /* Control flags */
  শূন্য (*xDel)(শূন্য*);        /* Destructor for aData */
  শূন্য *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
প্রকারনির্দেশ সংরচনা carray_cursor carray_cursor;
সংরচনা carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  শূন্য *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned ৱর্ণ eType;       /* One of the CARRAY_type values */
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
স্থির পূর্ণাঙ্ক carrayConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ*argv,
  sqlite3_vtab **ppVtab,
  ৱর্ণ **pzErr
){
  sqlite3_vtab *pNew;
  পূর্ণাঙ্ক rc;

/* Column numbers */
#define CARRAY_COLUMN_মান   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_গুনতি   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  যদি( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( পরিমাণ(*pNew) );
    যদি( pNew==0 ) ঘূৰাই SQLITE_NOMEM;
    memset(pNew, 0, পরিমাণ(*pNew));
  }
  ঘূৰাই rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
স্থির পূর্ণাঙ্ক carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ঘূৰাই SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
স্থির পূর্ণাঙ্ক carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( পরিমাণ(*pCur) );
  যদি( pCur==0 ) ঘূৰাই SQLITE_NOMEM;
  memset(pCur, 0, পরিমাণ(*pCur));
  *ppCursor = &pCur->base;
  ঘূৰাই SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
স্থির পূর্ণাঙ্ক carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  ঘূৰাই SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
স্থির পূর্ণাঙ্ক carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  ঘূৰাই SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
স্থির পূর্ণাঙ্ক carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  পূর্ণাঙ্ক i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  ৱিকল্প( i ){
    পক্ষ CARRAY_COLUMN_POINTER:   ঘূৰাই SQLITE_OK;
    পক্ষ CARRAY_COLUMN_গুনতি:     x = pCur->iCnt;   ভঙ্গ;
    পক্ষ CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      ঘূৰাই SQLITE_OK;
    }
    স্ৱভাৱ: {
      ৱিকল্প( pCur->eType ){
        পক্ষ CARRAY_INT32: {
          পূর্ণাঙ্ক *p = (পূর্ণাঙ্ক*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          ঘূৰাই SQLITE_OK;
        }
        পক্ষ CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          ঘূৰাই SQLITE_OK;
        }
        পক্ষ CARRAY_DOUBLE: {
          দ্ৱিগুণ *p = (দ্ৱিগুণ*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          ঘূৰাই SQLITE_OK;
        }
        পক্ষ CARRAY_TEXT: {
          ধ্রুৱ ৱর্ণ **p = (ধ্রুৱ ৱর্ণ**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          ঘূৰাই SQLITE_OK;
        }
        স্ৱভাৱ: {
          ধ্রুৱ সংরচনা iovec *p = (সংরচনা iovec*)pCur->pPtr;
          প্রতিজ্ঞা( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          যদি( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            ঘূৰাই SQLITE_TOOBIG;
          }অন্যথা{
            sqlite3_result_blob(ctx, p->iov_base, (পূর্ণাঙ্ক)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          ঘূৰাই SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  ঘূৰাই SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
স্থির পূর্ণাঙ্ক carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  ঘূৰাই SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
স্থির পূর্ণাঙ্ক carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  ঘূৰাই pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
স্থির পূর্ণাঙ্ক carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণাঙ্ক idx_অঙ্ক, ধ্রুৱ ৱর্ণ *idxStr,
  পূর্ণাঙ্ক argc, sqlite3_মান **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  ৱিকল্প( idx_অঙ্ক ){
    পক্ষ 1: {
      carray_bind *pBind = sqlite3_মান_pointer(argv[0], "carray-bind");
      যদি( pBind==0 ) ভঙ্গ;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      ভঙ্গ;
    }
    পক্ষ 2:
    পক্ষ 3: {
      pCur->pPtr = sqlite3_মান_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_মান_int64(argv[1]) : 0;
      যদি( idx_অঙ্ক<3 ){
        pCur->eType = CARRAY_INT32;
      }অন্যথা{
        unsigned ৱর্ণ i;
        ধ্রুৱ ৱর্ণ *zType = (ধ্রুৱ ৱর্ণ*)sqlite3_মান_text(argv[2]);
        বাবে(i=0; i<পরিমাণ(azCarrayType)/পরিমাণ(azCarrayType[0]); i++){
          যদি( sqlite3_stricmp(zType, azCarrayType[i])==0 ) ভঙ্গ;
        }
        যদি( i>=পরিমাণ(azCarrayType)/পরিমাণ(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_বাৰ্তা = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          ঘূৰাই SQLITE_ERROR;
        }অন্যথা{
          pCur->eType = i;
        }
      }
      ভঙ্গ;
    }
  }
  pCur->iRowid = 1;
  ঘূৰাই SQLITE_OK;
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
স্থির পূর্ণাঙ্ক carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  পূর্ণাঙ্ক i;                 /* Loop over constraints */
  পূর্ণাঙ্ক ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  পূর্ণাঙ্ক cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  পূর্ণাঙ্ক ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  ধ্রুৱ সংরচনা sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  বাবে(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    যদি( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) অনুৱর্ত;
    যদি( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    যদি( pConstraint->usable==0 ) অনুৱর্ত;
    ৱিকল্প( pConstraint->iColumn ){
      পক্ষ CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        ভঙ্গ;
      পক্ষ CARRAY_COLUMN_গুনতি:
        cntIdx = i;
        ভঙ্গ;
      পক্ষ CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        ভঙ্গ;
    }
  }
  যদি( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (দ্ৱিগুণ)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_অঙ্ক = 1;
    যদি( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_অঙ্ক = 2;
      যদি( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_অঙ্ক = 3;
      }অন্যথা যদি( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        ঘূৰাই SQLITE_CONSTRAINT;
      }
    }অন্যথা যদি( seen & (1<<CARRAY_COLUMN_গুনতি) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      ঘূৰাই SQLITE_CONSTRAINT;
    }
  }অন্যথা{
    pIdxInfo->estimatedCost = (দ্ৱিগুণ)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_অঙ্ক = 0;
  }
  ঘূৰাই SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
স্থির sqlite3_module carrayModule = {
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
স্থির শূন্য carrayBindDel(শূন্য *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  যদি( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API পূর্ণাঙ্ক sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  পূর্ণাঙ্ক idx,
  শূন্য *aData,
  পূর্ণাঙ্ক nData,
  পূর্ণাঙ্ক mFlags,
  শূন্য (*xDestroy)(শূন্য*),
  শূন্য *pDestroy
){
  carray_bind *pNew = 0;
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  প্রতিজ্ঞা( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  প্রতিজ্ঞা( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  যদি( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    গচ্ছ carray_bind_error;
  }

  pNew = sqlite3_malloc64(পরিমাণ(*pNew));
  যদি( pNew==0 ){
    rc = SQLITE_NOMEM;
    গচ্ছ carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  যদি( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    ৱিকল্প( mFlags ){
      পক্ষ CARRAY_INT32:   sz *= 4;                     ভঙ্গ;
      পক্ষ CARRAY_INT64:   sz *= 8;                     ভঙ্গ;
      পক্ষ CARRAY_DOUBLE:  sz *= 8;                     ভঙ্গ;
      পক্ষ CARRAY_TEXT:    sz *= পরিমাণ(ৱর্ণ*);         ভঙ্গ;
      স্ৱভাৱ:             sz *= পরিমাণ(সংরচনা iovec);  ভঙ্গ;
    }
    যদি( mFlags==CARRAY_TEXT ){
      বাবে(i=0; i<nData; i++){
        ধ্রুৱ ৱর্ণ *z = ((ৱর্ণ**)aData)[i];
        যদি( z ) sz += strlen(z) + 1;
      }
    }অন্যথা যদি( mFlags==CARRAY_BLOB ){
      বাবে(i=0; i<nData; i++){
        sz += ((সংরচনা iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    যদি( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      গচ্ছ carray_bind_error;
    }

    যদি( mFlags==CARRAY_TEXT ){
      ৱর্ণ **az = (ৱর্ণ**)pNew->aData;
      ৱর্ণ *z = (ৱর্ণ*)&az[nData];
      বাবে(i=0; i<nData; i++){
        ধ্রুৱ ৱর্ণ *zData = ((ৱর্ণ**)aData)[i];
        sqlite3_int64 n;
        যদি( zData==0 ){
          az[i] = 0;
          অনুৱর্ত;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }অন্যথা যদি( mFlags==CARRAY_BLOB ){
      সংরচনা iovec *p = (সংরচনা iovec*)pNew->aData;
      unsigned ৱর্ণ *z = (unsigned ৱর্ণ*)&p[nData];
      বাবে(i=0; i<nData; i++){
        size_t n = ((সংরচনা iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((সংরচনা iovec*)aData)[i].iov_base, n);
      }
    }অন্যথা যদি( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }অন্যথা{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  ঘূৰাই sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  যদি( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  ঘূৰাই rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API পূর্ণাঙ্ক sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  পূর্ণাঙ্ক idx,
  শূন্য *aData,
  পূর্ণাঙ্ক nData,
  পূর্ণাঙ্ক mFlags,
  শূন্য (*xDestroy)(শূন্য*)
){
  ঘূৰাই sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  ঘূৰাই sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
