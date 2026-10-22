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
  ساخت iovec {
    خالی *iov_بنیاد;
    size_t iov_len;
  };
#ورنہ
# include <sys/uio.h>
#اگر_ختم

/*
** Names of allowed datatypes
*/
ساکن ثابت حرف *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
قسمتعریف ساخت carray_bind carray_bind;
ساخت carray_bind {
  خالی *a_ڈیٹا;                /* The data */
  صحیح n_ڈیٹا;                  /* Number of elements */
  صحیح mFlags;                 /* Control flags */
  خالی (*xDel)(خالی*);        /* Destructor for aData */
  خالی *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
قسمتعریف ساخت carray_cursor carray_cursor;
ساخت carray_cursor {
  sqlite3_vtab_cursor بنیاد;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  خالی *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  غیرعلامتی حرف eType;       /* One of the CARRAY_type values */
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
ساکن صحیح carrayConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  sqlite3_vtab *pNew;
  صحیح rc;

/* Column numbers */
#تعریف_کرو CARRAY_عمود_قدر   0
#تعریف_کرو CARRAY_عمود_اشارہ_گر 1
#تعریف_کرو CARRAY_عمود_گنتی   2
#تعریف_کرو CARRAY_عمود_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  اگر( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( حجم(*pNew) );
    اگر( pNew==0 ) واپس SQLITE_NOMEM;
    memset(pNew, 0, حجم(*pNew));
  }
  واپس rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
ساکن صحیح carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  واپس SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
ساکن صحیح carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( حجم(*pCur) );
  اگر( pCur==0 ) واپس SQLITE_NOMEM;
  memset(pCur, 0, حجم(*pCur));
  *ppCursor = &pCur->بنیاد;
  واپس SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
ساکن صحیح carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  واپس SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
ساکن صحیح carray_اگلا(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  واپس SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
ساکن صحیح carray_عمود(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  صحیح i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  انتخاب( i ){
    حالت CARRAY_عمود_اشارہ_گر:   واپس SQLITE_OK;
    حالت CARRAY_عمود_گنتی:     x = pCur->iCnt;   توڑو;
    حالت CARRAY_عمود_CTYPE: {
      sqlite3_نتیجہ_متن(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      واپس SQLITE_OK;
    }
    طےشدہ: {
      انتخاب( pCur->eType ){
        حالت CARRAY_INT32: {
          صحیح *p = (صحیح*)pCur->pPtr;
          sqlite3_نتیجہ_int(ctx, p[pCur->iRowid-1]);
          واپس SQLITE_OK;
        }
        حالت CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_نتیجہ_int64(ctx, p[pCur->iRowid-1]);
          واپس SQLITE_OK;
        }
        حالت CARRAY_DOUBLE: {
          دوہرا *p = (دوہرا*)pCur->pPtr;
          sqlite3_نتیجہ_double(ctx, p[pCur->iRowid-1]);
          واپس SQLITE_OK;
        }
        حالت CARRAY_متن: {
          ثابت حرف **p = (ثابت حرف**)pCur->pPtr;
          sqlite3_نتیجہ_متن(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          واپس SQLITE_OK;
        }
        طےشدہ: {
          ثابت ساخت iovec *p = (ساخت iovec*)pCur->pPtr;
          أكد( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          اگر( p->iov_len>0x7fffffff ){
            sqlite3_نتیجہ_error_toobig(ctx);
            واپس SQLITE_TOOBIG;
          }ورنہ{
            sqlite3_نتیجہ_blob(ctx, p->iov_بنیاد, (صحیح)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          واپس SQLITE_OK;
        }
      }
    }
  }
  sqlite3_نتیجہ_int64(ctx, x);
  واپس SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
ساکن صحیح carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  واپس SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
ساکن صحیح carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  واپس pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
ساکن صحیح carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  انتخاب( idx_عدد ){
    حالت 1: {
      carray_bind *pBind = sqlite3_قدر_اشارہ_گر(argv[0], "carray-bind");
      اگر( pBind==0 ) توڑو;
      pCur->pPtr = pBind->a_ڈیٹا;
      pCur->iCnt = pBind->n_ڈیٹا;
      pCur->eType = pBind->mFlags & 0x07;
      توڑو;
    }
    حالت 2:
    حالت 3: {
      pCur->pPtr = sqlite3_قدر_اشارہ_گر(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_قدر_int64(argv[1]) : 0;
      اگر( idx_عدد<3 ){
        pCur->eType = CARRAY_INT32;
      }ورنہ{
        غیرعلامتی حرف i;
        ثابت حرف *zType = (ثابت حرف*)sqlite3_قدر_متن(argv[2]);
        برائے(i=0; i<حجم(azCarrayType)/حجم(azCarrayType[0]); i++){
          اگر( sqlite3_stricmp(zType, azCarrayType[i])==0 ) توڑو;
        }
        اگر( i>=حجم(azCarrayType)/حجم(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_پیغام = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          واپس SQLITE_ERROR;
        }ورنہ{
          pCur->eType = i;
        }
      }
      توڑو;
    }
  }
  pCur->iRowid = 1;
  واپس SQLITE_OK;
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
ساکن صحیح carray_بہترین_اشاریہ(
  sqlite3_vtab *tab,
  sqlite3_اشاریہ_info *pIdxInfo
){
  صحیح i;                 /* Loop over constraints */
  صحیح ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  صحیح cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  صحیح ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  غیرعلامتی نظر_آیا = 0;     /* Bitmask of == constrainted columns */

  ثابت ساخت sqlite3_اشاریہ_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  برائے(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    اگر( pConstraint->op!=SQLITE_اشاریہ_CONSTRAINT_EQ ) جاری;
    اگر( pConstraint->i_عمود>=0 ) نظر_آیا |= 1 << pConstraint->i_عمود;
    اگر( pConstraint->usable==0 ) جاری;
    انتخاب( pConstraint->i_عمود ){
      حالت CARRAY_عمود_اشارہ_گر:
        ptrIdx = i;
        توڑو;
      حالت CARRAY_عمود_گنتی:
        cntIdx = i;
        توڑو;
      حالت CARRAY_عمود_CTYPE:
        ctypeIdx = i;
        توڑو;
    }
  }
  اگر( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argv_اشاریہ = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimated_لاگت = (دوہرا)1;
    pIdxInfo->estimated_صفیں = 100;
    pIdxInfo->idx_عدد = 1;
    اگر( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argv_اشاریہ = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_عدد = 2;
      اگر( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argv_اشاریہ = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_عدد = 3;
      }ورنہ اگر( نظر_آیا & (1<<CARRAY_عمود_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        واپس SQLITE_CONSTRAINT;
      }
    }ورنہ اگر( نظر_آیا & (1<<CARRAY_عمود_گنتی) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      واپس SQLITE_CONSTRAINT;
    }
  }ورنہ{
    pIdxInfo->estimated_لاگت = (دوہرا)2147483647;
    pIdxInfo->estimated_صفیں = 2147483647;
    pIdxInfo->idx_عدد = 0;
  }
  واپس SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
ساکن sqlite3_module carrayModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  carrayConnect,             /* xConnect */
  carray_بہترین_اشاریہ,           /* xBestIndex */
  carrayDisconnect,          /* xDisconnect */
  0,                         /* xDestroy */
  carrayOpen,                /* xOpen - open a cursor */
  carrayClose,               /* xClose - close a cursor */
  carrayFilter,              /* xFilter - configure scan constraints */
  carray_اگلا,                /* xNext - advance a cursor */
  carrayEof,                 /* xEof - check for end of scan */
  carray_عمود,              /* xColumn - read data */
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
ساکن خالی carrayBindDel(خالی *pPtr){
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
  خالی *a_ڈیٹا,
  صحیح n_ڈیٹا,
  صحیح mFlags,
  خالی (*xDestroy)(خالی*),
  خالی *pDestroy
){
  carray_bind *pNew = 0;
  صحیح i;
  صحیح rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  أكد( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  أكد( CARRAY_متن==3 && CARRAY_BLOB==4 );
  اگر( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    جاؤ carray_bind_error;
  }

  pNew = sqlite3_malloc64(حجم(*pNew));
  اگر( pNew==0 ){
    rc = SQLITE_NOMEM;
    جاؤ carray_bind_error;
  }

  pNew->n_ڈیٹا = n_ڈیٹا;
  pNew->mFlags = mFlags;
  اگر( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = n_ڈیٹا;
    انتخاب( mFlags ){
      حالت CARRAY_INT32:   sz *= 4;                     توڑو;
      حالت CARRAY_INT64:   sz *= 8;                     توڑو;
      حالت CARRAY_DOUBLE:  sz *= 8;                     توڑو;
      حالت CARRAY_متن:    sz *= حجم(حرف*);         توڑو;
      طےشدہ:             sz *= حجم(ساخت iovec);  توڑو;
    }
    اگر( mFlags==CARRAY_متن ){
      برائے(i=0; i<n_ڈیٹا; i++){
        ثابت حرف *z = ((حرف**)a_ڈیٹا)[i];
        اگر( z ) sz += لڑی_لمبائی(z) + 1;
      }
    }ورنہ اگر( mFlags==CARRAY_BLOB ){
      برائے(i=0; i<n_ڈیٹا; i++){
        sz += ((ساخت iovec*)a_ڈیٹا)[i].iov_len;
      }
    } 

    pNew->a_ڈیٹا = sqlite3_malloc64( sz>0 ? sz : 1 );
    اگر( pNew->a_ڈیٹا==0 ){
      rc = SQLITE_NOMEM;
      جاؤ carray_bind_error;
    }

    اگر( mFlags==CARRAY_متن ){
      حرف **az = (حرف**)pNew->a_ڈیٹا;
      حرف *z = (حرف*)&az[n_ڈیٹا];
      برائے(i=0; i<n_ڈیٹا; i++){
        ثابت حرف *z_ڈیٹا = ((حرف**)a_ڈیٹا)[i];
        sqlite3_int64 n;
        اگر( z_ڈیٹا==0 ){
          az[i] = 0;
          جاری;
        }
        az[i] = z;
        n = لڑی_لمبائی(z_ڈیٹا);
        حافظہ_نقل(z, z_ڈیٹا, n+1);
        z += n+1;
      }
    }ورنہ اگر( mFlags==CARRAY_BLOB ){
      ساخت iovec *p = (ساخت iovec*)pNew->a_ڈیٹا;
      غیرعلامتی حرف *z = (غیرعلامتی حرف*)&p[n_ڈیٹا];
      برائے(i=0; i<n_ڈیٹا; i++){
        size_t n = ((ساخت iovec*)a_ڈیٹا)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_بنیاد = z;
        z += n;
        حافظہ_نقل(p[i].iov_بنیاد, ((ساخت iovec*)a_ڈیٹا)[i].iov_بنیاد, n);
      }
    }ورنہ اگر( sz>0 ){
      حافظہ_نقل(pNew->a_ڈیٹا, a_ڈیٹا, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->a_ڈیٹا;
  }ورنہ{
    pNew->a_ڈیٹا = a_ڈیٹا;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  واپس sqlite3_bind_اشارہ_گر(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  اگر( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  واپس rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API صحیح sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  صحیح idx,
  خالی *a_ڈیٹا,
  صحیح n_ڈیٹا,
  صحیح mFlags,
  خالی (*xDestroy)(خالی*)
){
  واپس sqlite3_carray_bind_v2(pStmt,idx,a_ڈیٹا,n_ڈیٹا,mFlags,xDestroy,a_ڈیٹا);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3_Carray_رجسٹر(sqlite3 *db){
  واپس sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#اگر_ختم /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
