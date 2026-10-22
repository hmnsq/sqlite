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
#অন্তর্ভাব "sqliteInt.h"
#যদি defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  কাঠামো iovec {
    শূন্য *iov_base;
    size_t iov_len;
  };
#অন্যথা
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
স্থির ধ্রুবক অক্ষর *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
প্রকারলেখ কাঠামো carray_bind carray_bind;
কাঠামো carray_bind {
  শূন্য *a_তথ্য;                /* The data */
  পূর্ণসংখ্যা n_তথ্য;                  /* Number of elements */
  পূর্ণসংখ্যা mFlags;                 /* Control flags */
  শূন্য (*xDel)(শূন্য*);        /* Destructor for aData */
  শূন্য *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
প্রকারলেখ কাঠামো carray_cursor carray_cursor;
কাঠামো carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  শূন্য *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned অক্ষর eType;       /* One of the CARRAY_type values */
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
স্থির পূর্ণসংখ্যা carrayConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  sqlite3_vtab *pNew;
  পূর্ণসংখ্যা rc;

/* Column numbers */
#define CARRAY_COLUMN_মান   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_গুনতি   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  যদি( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( আকার(*pNew) );
    যদি( pNew==0 ) ফেরত SQLITE_NOMEM;
    memset(pNew, 0, আকার(*pNew));
  }
  ফেরত rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
স্থির পূর্ণসংখ্যা carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ফেরত SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
স্থির পূর্ণসংখ্যা carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( আকার(*pCur) );
  যদি( pCur==0 ) ফেরত SQLITE_NOMEM;
  memset(pCur, 0, আকার(*pCur));
  *ppCursor = &pCur->base;
  ফেরত SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
স্থির পূর্ণসংখ্যা carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  ফেরত SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
স্থির পূর্ণসংখ্যা carray_পরবর্তী(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  ফেরত SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
স্থির পূর্ণসংখ্যা carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  পূর্ণসংখ্যা i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  নির্বাচন( i ){
    ক্ষেত্র CARRAY_COLUMN_POINTER:   ফেরত SQLITE_OK;
    ক্ষেত্র CARRAY_COLUMN_গুনতি:     x = pCur->iCnt;   বিরতি;
    ক্ষেত্র CARRAY_COLUMN_CTYPE: {
      sqlite3_ফলাফল_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      ফেরত SQLITE_OK;
    }
    পূর্বনির্ধারিত: {
      নির্বাচন( pCur->eType ){
        ক্ষেত্র CARRAY_INT32: {
          পূর্ণসংখ্যা *p = (পূর্ণসংখ্যা*)pCur->pPtr;
          sqlite3_ফলাফল_int(ctx, p[pCur->iRowid-1]);
          ফেরত SQLITE_OK;
        }
        ক্ষেত্র CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_ফলাফল_int64(ctx, p[pCur->iRowid-1]);
          ফেরত SQLITE_OK;
        }
        ক্ষেত্র CARRAY_DOUBLE: {
          দ্বিগুণ *p = (দ্বিগুণ*)pCur->pPtr;
          sqlite3_ফলাফল_double(ctx, p[pCur->iRowid-1]);
          ফেরত SQLITE_OK;
        }
        ক্ষেত্র CARRAY_TEXT: {
          ধ্রুবক অক্ষর **p = (ধ্রুবক অক্ষর**)pCur->pPtr;
          sqlite3_ফলাফল_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          ফেরত SQLITE_OK;
        }
        পূর্বনির্ধারিত: {
          ধ্রুবক কাঠামো iovec *p = (কাঠামো iovec*)pCur->pPtr;
          প্রতিজ্ঞা( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          যদি( p->iov_len>0x7fffffff ){
            sqlite3_ফলাফল_error_toobig(ctx);
            ফেরত SQLITE_TOOBIG;
          }অন্যথা{
            sqlite3_ফলাফল_blob(ctx, p->iov_base, (পূর্ণসংখ্যা)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          ফেরত SQLITE_OK;
        }
      }
    }
  }
  sqlite3_ফলাফল_int64(ctx, x);
  ফেরত SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
স্থির পূর্ণসংখ্যা carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  ফেরত SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
স্থির পূর্ণসংখ্যা carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  ফেরত pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
স্থির পূর্ণসংখ্যা carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  নির্বাচন( idx_অঙ্ক ){
    ক্ষেত্র 1: {
      carray_bind *pBind = sqlite3_মান_pointer(argv[0], "carray-bind");
      যদি( pBind==0 ) বিরতি;
      pCur->pPtr = pBind->a_তথ্য;
      pCur->iCnt = pBind->n_তথ্য;
      pCur->eType = pBind->mFlags & 0x07;
      বিরতি;
    }
    ক্ষেত্র 2:
    ক্ষেত্র 3: {
      pCur->pPtr = sqlite3_মান_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_মান_int64(argv[1]) : 0;
      যদি( idx_অঙ্ক<3 ){
        pCur->eType = CARRAY_INT32;
      }অন্যথা{
        unsigned অক্ষর i;
        ধ্রুবক অক্ষর *zType = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[2]);
        জন্য(i=0; i<আকার(azCarrayType)/আকার(azCarrayType[0]); i++){
          যদি( sqlite3_stricmp(zType, azCarrayType[i])==0 ) বিরতি;
        }
        যদি( i>=আকার(azCarrayType)/আকার(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_বার্তা = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          ফেরত SQLITE_ERROR;
        }অন্যথা{
          pCur->eType = i;
        }
      }
      বিরতি;
    }
  }
  pCur->iRowid = 1;
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা carray_Best_সূচক(
  sqlite3_vtab *tab,
  sqlite3_সূচক_info *pIdxInfo
){
  পূর্ণসংখ্যা i;                 /* Loop over constraints */
  পূর্ণসংখ্যা ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  পূর্ণসংখ্যা cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  পূর্ণসংখ্যা ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  ধ্রুবক কাঠামো sqlite3_সূচক_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  জন্য(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    যদি( pConstraint->op!=SQLITE_সূচক_CONSTRAINT_EQ ) চালু;
    যদি( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    যদি( pConstraint->usable==0 ) চালু;
    নির্বাচন( pConstraint->iColumn ){
      ক্ষেত্র CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        বিরতি;
      ক্ষেত্র CARRAY_COLUMN_গুনতি:
        cntIdx = i;
        বিরতি;
      ক্ষেত্র CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        বিরতি;
    }
  }
  যদি( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argv_সূচক = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (দ্বিগুণ)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_অঙ্ক = 1;
    যদি( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argv_সূচক = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_অঙ্ক = 2;
      যদি( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argv_সূচক = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_অঙ্ক = 3;
      }অন্যথা যদি( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        ফেরত SQLITE_CONSTRAINT;
      }
    }অন্যথা যদি( seen & (1<<CARRAY_COLUMN_গুনতি) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      ফেরত SQLITE_CONSTRAINT;
    }
  }অন্যথা{
    pIdxInfo->estimatedCost = (দ্বিগুণ)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_অঙ্ক = 0;
  }
  ফেরত SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
স্থির sqlite3_module carrayModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  carrayConnect,             /* xConnect */
  carray_Best_সূচক,           /* xBestIndex */
  carrayDisconnect,          /* xDisconnect */
  0,                         /* xDestroy */
  carrayOpen,                /* xOpen - open a cursor */
  carrayClose,               /* xClose - close a cursor */
  carrayFilter,              /* xFilter - configure scan constraints */
  carray_পরবর্তী,                /* xNext - advance a cursor */
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
SQLITE_API পূর্ণসংখ্যা sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  পূর্ণসংখ্যা idx,
  শূন্য *a_তথ্য,
  পূর্ণসংখ্যা n_তথ্য,
  পূর্ণসংখ্যা mFlags,
  শূন্য (*xDestroy)(শূন্য*),
  শূন্য *pDestroy
){
  carray_bind *pNew = 0;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  প্রতিজ্ঞা( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  প্রতিজ্ঞা( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  যদি( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    যাও carray_bind_error;
  }

  pNew = sqlite3_malloc64(আকার(*pNew));
  যদি( pNew==0 ){
    rc = SQLITE_NOMEM;
    যাও carray_bind_error;
  }

  pNew->n_তথ্য = n_তথ্য;
  pNew->mFlags = mFlags;
  যদি( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = n_তথ্য;
    নির্বাচন( mFlags ){
      ক্ষেত্র CARRAY_INT32:   sz *= 4;                     বিরতি;
      ক্ষেত্র CARRAY_INT64:   sz *= 8;                     বিরতি;
      ক্ষেত্র CARRAY_DOUBLE:  sz *= 8;                     বিরতি;
      ক্ষেত্র CARRAY_TEXT:    sz *= আকার(অক্ষর*);         বিরতি;
      পূর্বনির্ধারিত:             sz *= আকার(কাঠামো iovec);  বিরতি;
    }
    যদি( mFlags==CARRAY_TEXT ){
      জন্য(i=0; i<n_তথ্য; i++){
        ধ্রুবক অক্ষর *z = ((অক্ষর**)a_তথ্য)[i];
        যদি( z ) sz += strlen(z) + 1;
      }
    }অন্যথা যদি( mFlags==CARRAY_BLOB ){
      জন্য(i=0; i<n_তথ্য; i++){
        sz += ((কাঠামো iovec*)a_তথ্য)[i].iov_len;
      }
    } 

    pNew->a_তথ্য = sqlite3_malloc64( sz>0 ? sz : 1 );
    যদি( pNew->a_তথ্য==0 ){
      rc = SQLITE_NOMEM;
      যাও carray_bind_error;
    }

    যদি( mFlags==CARRAY_TEXT ){
      অক্ষর **az = (অক্ষর**)pNew->a_তথ্য;
      অক্ষর *z = (অক্ষর*)&az[n_তথ্য];
      জন্য(i=0; i<n_তথ্য; i++){
        ধ্রুবক অক্ষর *z_তথ্য = ((অক্ষর**)a_তথ্য)[i];
        sqlite3_int64 n;
        যদি( z_তথ্য==0 ){
          az[i] = 0;
          চালু;
        }
        az[i] = z;
        n = strlen(z_তথ্য);
        memcpy(z, z_তথ্য, n+1);
        z += n+1;
      }
    }অন্যথা যদি( mFlags==CARRAY_BLOB ){
      কাঠামো iovec *p = (কাঠামো iovec*)pNew->a_তথ্য;
      unsigned অক্ষর *z = (unsigned অক্ষর*)&p[n_তথ্য];
      জন্য(i=0; i<n_তথ্য; i++){
        size_t n = ((কাঠামো iovec*)a_তথ্য)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((কাঠামো iovec*)a_তথ্য)[i].iov_base, n);
      }
    }অন্যথা যদি( sz>0 ){
      memcpy(pNew->a_তথ্য, a_তথ্য, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->a_তথ্য;
  }অন্যথা{
    pNew->a_তথ্য = a_তথ্য;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  ফেরত sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  যদি( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  ফেরত rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API পূর্ণসংখ্যা sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  পূর্ণসংখ্যা idx,
  শূন্য *a_তথ্য,
  পূর্ণসংখ্যা n_তথ্য,
  পূর্ণসংখ্যা mFlags,
  শূন্য (*xDestroy)(শূন্য*)
){
  ফেরত sqlite3_carray_bind_v2(pStmt,idx,a_তথ্য,n_তথ্য,mFlags,xDestroy,a_তথ্য);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  ফেরত sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
