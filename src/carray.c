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
#ಆದರೆ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಆದರೆ defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  ರಚನೆ iovec {
    ಶೂನ್ಯ *iov_ತಳ;
    size_t iov_len;
  };
#ಇಲ್ಲದಿದ್ದರೆ
# include <sys/uio.h>
#ಆದರೆ_ಅಂತ್ಯ

/*
** Names of allowed datatypes
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ carray_bind carray_bind;
ರಚನೆ carray_bind {
  ಶೂನ್ಯ *a_ದತ್ತಾಂಶ;                /* The data */
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ;                  /* Number of elements */
  ಪೂರ್ಣಾಂಕ mFlags;                 /* Control flags */
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*);        /* Destructor for aData */
  ಶೂನ್ಯ *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ carray_cursor carray_cursor;
ರಚನೆ carray_cursor {
  sqlite3_vtab_cursor ತಳ;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  ಶೂನ್ಯ *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ eType;       /* One of the CARRAY_type values */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carrayConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  sqlite3_vtab *pNew;
  ಪೂರ್ಣಾಂಕ rc;

/* Column numbers */
#ವ್ಯಾಖ್ಯಾನಿಸು CARRAY_ಸ್ತಂಭ_ಮೌಲ್ಯ   0
#ವ್ಯಾಖ್ಯಾನಿಸು CARRAY_ಸ್ತಂಭ_ಸೂಚಕ 1
#ವ್ಯಾಖ್ಯಾನಿಸು CARRAY_ಸ್ತಂಭ_ಎಣಿಕೆ   2
#ವ್ಯಾಖ್ಯಾನಿಸು CARRAY_ಸ್ತಂಭ_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  ಆದರೆ( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( ಗಾತ್ರ(*pNew) );
    ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    memset(pNew, 0, ಗಾತ್ರ(*pNew));
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( ಗಾತ್ರ(*pCur) );
  ಆದರೆ( pCur==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pCur, 0, ಗಾತ್ರ(*pCur));
  *ppCursor = &pCur->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carray_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carray_ಸ್ತಂಭ(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  ಪೂರ್ಣಾಂಕ i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  ಆಯ್ಕೆ( i ){
    ಸ್ಥಿತಿ CARRAY_ಸ್ತಂಭ_ಸೂಚಕ:   ಹಿಂತಿರುಗಿ SQLITE_OK;
    ಸ್ಥಿತಿ CARRAY_ಸ್ತಂಭ_ಎಣಿಕೆ:     x = pCur->iCnt;   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ CARRAY_ಸ್ತಂಭ_CTYPE: {
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಆಯ್ಕೆ( pCur->eType ){
        ಸ್ಥಿತಿ CARRAY_INT32: {
          ಪೂರ್ಣಾಂಕ *p = (ಪೂರ್ಣಾಂಕ*)pCur->pPtr;
          sqlite3_ಫಲಿತಾಂಶ_int(ctx, p[pCur->iRowid-1]);
          ಹಿಂತಿರುಗಿ SQLITE_OK;
        }
        ಸ್ಥಿತಿ CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_ಫಲಿತಾಂಶ_int64(ctx, p[pCur->iRowid-1]);
          ಹಿಂತಿರುಗಿ SQLITE_OK;
        }
        ಸ್ಥಿತಿ CARRAY_DOUBLE: {
          ದ್ವಿಗುಣ *p = (ದ್ವಿಗುಣ*)pCur->pPtr;
          sqlite3_ಫಲಿತಾಂಶ_double(ctx, p[pCur->iRowid-1]);
          ಹಿಂತಿರುಗಿ SQLITE_OK;
        }
        ಸ್ಥಿತಿ CARRAY_ಪಠ್ಯ: {
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **p = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ**)pCur->pPtr;
          sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          ಹಿಂತಿರುಗಿ SQLITE_OK;
        }
        ಪೂರ್ವನಿಯೋಜಿತ: {
          ಸ್ಥಿರಾಂಕ ರಚನೆ iovec *p = (ರಚನೆ iovec*)pCur->pPtr;
          ಪ್ರತಿಜ್ಞಾ( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          ಆದರೆ( p->iov_len>0x7fffffff ){
            sqlite3_ಫಲಿತಾಂಶ_error_toobig(ctx);
            ಹಿಂತಿರುಗಿ SQLITE_TOOBIG;
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_ಫಲಿತಾಂಶ_blob(ctx, p->iov_ತಳ, (ಪೂರ್ಣಾಂಕ)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          ಹಿಂತಿರುಗಿ SQLITE_OK;
        }
      }
    }
  }
  sqlite3_ಫಲಿತಾಂಶ_int64(ctx, x);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  ಹಿಂತಿರುಗಿ pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  ಆಯ್ಕೆ( idx_ಅಂಕೆ ){
    ಸ್ಥಿತಿ 1: {
      carray_bind *pBind = sqlite3_ಮೌಲ್ಯ_ಸೂಚಕ(argv[0], "carray-bind");
      ಆದರೆ( pBind==0 ) ನಿಲ್ಲಿಸು;
      pCur->pPtr = pBind->a_ದತ್ತಾಂಶ;
      pCur->iCnt = pBind->n_ದತ್ತಾಂಶ;
      pCur->eType = pBind->mFlags & 0x07;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 2:
    ಸ್ಥಿತಿ 3: {
      pCur->pPtr = sqlite3_ಮೌಲ್ಯ_ಸೂಚಕ(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_ಮೌಲ್ಯ_int64(argv[1]) : 0;
      ಆದರೆ( idx_ಅಂಕೆ<3 ){
        pCur->eType = CARRAY_INT32;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ i;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[2]);
        ಗಾಗಿ(i=0; i<ಗಾತ್ರ(azCarrayType)/ಗಾತ್ರ(azCarrayType[0]); i++){
          ಆದರೆ( sqlite3_stricmp(zType, azCarrayType[i])==0 ) ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ( i>=ಗಾತ್ರ(azCarrayType)/ಗಾತ್ರ(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_ಸಂದೇಶ = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          ಹಿಂತಿರುಗಿ SQLITE_ERROR;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pCur->eType = i;
        }
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  pCur->iRowid = 1;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ carray_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(
  sqlite3_vtab *tab,
  sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo
){
  ಪೂರ್ಣಾಂಕ i;                 /* Loop over constraints */
  ಪೂರ್ಣಾಂಕ ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  ಪೂರ್ಣಾಂಕ cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  ಪೂರ್ಣಾಂಕ ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  ಚಿಹ್ನೆರಹಿತ ನೋಡಿದ = 0;     /* Bitmask of == constrainted columns */

  ಸ್ಥಿರಾಂಕ ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  ಗಾಗಿ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ಆದರೆ( pConstraint->op!=SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ ) ಮುಂದುವರಿ;
    ಆದರೆ( pConstraint->i_ಸ್ತಂಭ>=0 ) ನೋಡಿದ |= 1 << pConstraint->i_ಸ್ತಂಭ;
    ಆದರೆ( pConstraint->usable==0 ) ಮುಂದುವರಿ;
    ಆಯ್ಕೆ( pConstraint->i_ಸ್ತಂಭ ){
      ಸ್ಥಿತಿ CARRAY_ಸ್ತಂಭ_ಸೂಚಕ:
        ptrIdx = i;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ CARRAY_ಸ್ತಂಭ_ಎಣಿಕೆ:
        cntIdx = i;
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ CARRAY_ಸ್ತಂಭ_CTYPE:
        ctypeIdx = i;
        ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argv_ಸೂಚ್ಯಂಕ = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimated_ವೆಚ್ಚ = (ದ್ವಿಗುಣ)1;
    pIdxInfo->estimated_ಅಡ್ಡಸಾಲುಗಳು = 100;
    pIdxInfo->idx_ಅಂಕೆ = 1;
    ಆದರೆ( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argv_ಸೂಚ್ಯಂಕ = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_ಅಂಕೆ = 2;
      ಆದರೆ( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argv_ಸೂಚ್ಯಂಕ = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_ಅಂಕೆ = 3;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ನೋಡಿದ & (1<<CARRAY_ಸ್ತಂಭ_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ನೋಡಿದ & (1<<CARRAY_ಸ್ತಂಭ_ಎಣಿಕೆ) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    pIdxInfo->estimated_ವೆಚ್ಚ = (ದ್ವಿಗುಣ)2147483647;
    pIdxInfo->estimated_ಅಡ್ಡಸಾಲುಗಳು = 2147483647;
    pIdxInfo->idx_ಅಂಕೆ = 0;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
ಸ್ಥಿರ sqlite3_module carrayModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  carrayConnect,             /* xConnect */
  carray_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,           /* xBestIndex */
  carrayDisconnect,          /* xDisconnect */
  0,                         /* xDestroy */
  carrayOpen,                /* xOpen - open a cursor */
  carrayClose,               /* xClose - close a cursor */
  carrayFilter,              /* xFilter - configure scan constraints */
  carray_ಮುಂದಿನ,                /* xNext - advance a cursor */
  carrayEof,                 /* xEof - check for end of scan */
  carray_ಸ್ತಂಭ,              /* xColumn - read data */
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
  ಆದರೆ( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API ಪೂರ್ಣಾಂಕ sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಂಕ idx,
  ಶೂನ್ಯ *a_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ mFlags,
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*),
  ಶೂನ್ಯ *pDestroy
){
  carray_bind *pNew = 0;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  ಪ್ರತಿಜ್ಞಾ( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  ಪ್ರತಿಜ್ಞಾ( CARRAY_ಪಠ್ಯ==3 && CARRAY_BLOB==4 );
  ಆದರೆ( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    ಹೋಗು carray_bind_error;
  }

  pNew = sqlite3_malloc64(ಗಾತ್ರ(*pNew));
  ಆದರೆ( pNew==0 ){
    rc = SQLITE_NOMEM;
    ಹೋಗು carray_bind_error;
  }

  pNew->n_ದತ್ತಾಂಶ = n_ದತ್ತಾಂಶ;
  pNew->mFlags = mFlags;
  ಆದರೆ( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = n_ದತ್ತಾಂಶ;
    ಆಯ್ಕೆ( mFlags ){
      ಸ್ಥಿತಿ CARRAY_INT32:   sz *= 4;                     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ CARRAY_INT64:   sz *= 8;                     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ CARRAY_DOUBLE:  sz *= 8;                     ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ CARRAY_ಪಠ್ಯ:    sz *= ಗಾತ್ರ(ಅಕ್ಷರ*);         ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:             sz *= ಗಾತ್ರ(ರಚನೆ iovec);  ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( mFlags==CARRAY_ಪಠ್ಯ ){
      ಗಾಗಿ(i=0; i<n_ದತ್ತಾಂಶ; i++){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = ((ಅಕ್ಷರ**)a_ದತ್ತಾಂಶ)[i];
        ಆದರೆ( z ) sz += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z) + 1;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( mFlags==CARRAY_BLOB ){
      ಗಾಗಿ(i=0; i<n_ದತ್ತಾಂಶ; i++){
        sz += ((ರಚನೆ iovec*)a_ದತ್ತಾಂಶ)[i].iov_len;
      }
    } 

    pNew->a_ದತ್ತಾಂಶ = sqlite3_malloc64( sz>0 ? sz : 1 );
    ಆದರೆ( pNew->a_ದತ್ತಾಂಶ==0 ){
      rc = SQLITE_NOMEM;
      ಹೋಗು carray_bind_error;
    }

    ಆದರೆ( mFlags==CARRAY_ಪಠ್ಯ ){
      ಅಕ್ಷರ **az = (ಅಕ್ಷರ**)pNew->a_ದತ್ತಾಂಶ;
      ಅಕ್ಷರ *z = (ಅಕ್ಷರ*)&az[n_ದತ್ತಾಂಶ];
      ಗಾಗಿ(i=0; i<n_ದತ್ತಾಂಶ; i++){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ದತ್ತಾಂಶ = ((ಅಕ್ಷರ**)a_ದತ್ತಾಂಶ)[i];
        sqlite3_int64 n;
        ಆದರೆ( z_ದತ್ತಾಂಶ==0 ){
          az[i] = 0;
          ಮುಂದುವರಿ;
        }
        az[i] = z;
        n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ದತ್ತಾಂಶ);
        ಸ್ಮರಣೆ_ನಕಲು(z, z_ದತ್ತಾಂಶ, n+1);
        z += n+1;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( mFlags==CARRAY_BLOB ){
      ರಚನೆ iovec *p = (ರಚನೆ iovec*)pNew->a_ದತ್ತಾಂಶ;
      ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)&p[n_ದತ್ತಾಂಶ];
      ಗಾಗಿ(i=0; i<n_ದತ್ತಾಂಶ; i++){
        size_t n = ((ರಚನೆ iovec*)a_ದತ್ತಾಂಶ)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_ತಳ = z;
        z += n;
        ಸ್ಮರಣೆ_ನಕಲು(p[i].iov_ತಳ, ((ರಚನೆ iovec*)a_ದತ್ತಾಂಶ)[i].iov_ತಳ, n);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sz>0 ){
      ಸ್ಮರಣೆ_ನಕಲು(pNew->a_ದತ್ತಾಂಶ, a_ದತ್ತಾಂಶ, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->a_ದತ್ತಾಂಶ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pNew->a_ದತ್ತಾಂಶ = a_ದತ್ತಾಂಶ;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  ಹಿಂತಿರುಗಿ sqlite3_bind_ಸೂಚಕ(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  ಆದರೆ( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API ಪೂರ್ಣಾಂಕ sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  ಪೂರ್ಣಾಂಕ idx,
  ಶೂನ್ಯ *a_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ mFlags,
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ*)
){
  ಹಿಂತಿರುಗಿ sqlite3_carray_bind_v2(pStmt,idx,a_ದತ್ತಾಂಶ,n_ದತ್ತಾಂಶ,mFlags,xDestroy,a_ದತ್ತಾಂಶ);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3_Carray_ರಿಜಿಸ್ಟರ್(sqlite3 *db){
  ಹಿಂತಿರುಗಿ sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
