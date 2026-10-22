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
#જો !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#અન્તર્ભાવ "sqliteInt.h"
#જો defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  સંરચના iovec {
    શૂન્ય *iov_base;
    size_t iov_len;
  };
#નહીંતર
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
સ્થિર અચલ અક્ષર *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
પ્રકારવ્યાખ્યા સંરચના carray_bind carray_bind;
સંરચના carray_bind {
  શૂન્ય *aData;                /* The data */
  પૂર્ણાંક nData;                  /* Number of elements */
  પૂર્ણાંક mFlags;                 /* Control flags */
  શૂન્ય (*xDel)(શૂન્ય*);        /* Destructor for aData */
  શૂન્ય *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
પ્રકારવ્યાખ્યા સંરચના carray_cursor carray_cursor;
સંરચના carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  શૂન્ય *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned અક્ષર eType;       /* One of the CARRAY_type values */
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
સ્થિર પૂર્ણાંક carrayConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  sqlite3_vtab *pNew;
  પૂર્ણાંક rc;

/* Column numbers */
#define CARRAY_COLUMN_કિંમત   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_ગણતરી   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  જો( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( કદ(*pNew) );
    જો( pNew==0 ) પાછા SQLITE_NOMEM;
    memset(pNew, 0, કદ(*pNew));
  }
  પાછા rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
સ્થિર પૂર્ણાંક carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  પાછા SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
સ્થિર પૂર્ણાંક carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( કદ(*pCur) );
  જો( pCur==0 ) પાછા SQLITE_NOMEM;
  memset(pCur, 0, કદ(*pCur));
  *ppCursor = &pCur->base;
  પાછા SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
સ્થિર પૂર્ણાંક carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  પાછા SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
સ્થિર પૂર્ણાંક carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  પાછા SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
સ્થિર પૂર્ણાંક carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  પૂર્ણાંક i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  પસંદગી( i ){
    સ્થિતિ CARRAY_COLUMN_POINTER:   પાછા SQLITE_OK;
    સ્થિતિ CARRAY_COLUMN_ગણતરી:     x = pCur->iCnt;   તોડો;
    સ્થિતિ CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      પાછા SQLITE_OK;
    }
    મૂળભૂત: {
      પસંદગી( pCur->eType ){
        સ્થિતિ CARRAY_INT32: {
          પૂર્ણાંક *p = (પૂર્ણાંક*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          પાછા SQLITE_OK;
        }
        સ્થિતિ CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          પાછા SQLITE_OK;
        }
        સ્થિતિ CARRAY_DOUBLE: {
          દ્વિગુણ *p = (દ્વિગુણ*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          પાછા SQLITE_OK;
        }
        સ્થિતિ CARRAY_TEXT: {
          અચલ અક્ષર **p = (અચલ અક્ષર**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          પાછા SQLITE_OK;
        }
        મૂળભૂત: {
          અચલ સંરચના iovec *p = (સંરચના iovec*)pCur->pPtr;
          પ્રતિજ્ઞા( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          જો( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            પાછા SQLITE_TOOBIG;
          }નહીંતર{
            sqlite3_result_blob(ctx, p->iov_base, (પૂર્ણાંક)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          પાછા SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  પાછા SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
સ્થિર પૂર્ણાંક carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  પાછા SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
સ્થિર પૂર્ણાંક carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  પાછા pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
સ્થિર પૂર્ણાંક carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  પસંદગી( idx_અંક ){
    સ્થિતિ 1: {
      carray_bind *pBind = sqlite3_કિંમત_pointer(argv[0], "carray-bind");
      જો( pBind==0 ) તોડો;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      તોડો;
    }
    સ્થિતિ 2:
    સ્થિતિ 3: {
      pCur->pPtr = sqlite3_કિંમત_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_કિંમત_int64(argv[1]) : 0;
      જો( idx_અંક<3 ){
        pCur->eType = CARRAY_INT32;
      }નહીંતર{
        unsigned અક્ષર i;
        અચલ અક્ષર *zType = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[2]);
        માટે(i=0; i<કદ(azCarrayType)/કદ(azCarrayType[0]); i++){
          જો( sqlite3_stricmp(zType, azCarrayType[i])==0 ) તોડો;
        }
        જો( i>=કદ(azCarrayType)/કદ(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_સંદેશ = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          પાછા SQLITE_ERROR;
        }નહીંતર{
          pCur->eType = i;
        }
      }
      તોડો;
    }
  }
  pCur->iRowid = 1;
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  પૂર્ણાંક i;                 /* Loop over constraints */
  પૂર્ણાંક ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  પૂર્ણાંક cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  પૂર્ણાંક ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  અચલ સંરચના sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  માટે(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    જો( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ચાલુ;
    જો( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    જો( pConstraint->usable==0 ) ચાલુ;
    પસંદગી( pConstraint->iColumn ){
      સ્થિતિ CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        તોડો;
      સ્થિતિ CARRAY_COLUMN_ગણતરી:
        cntIdx = i;
        તોડો;
      સ્થિતિ CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        તોડો;
    }
  }
  જો( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (દ્વિગુણ)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_અંક = 1;
    જો( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_અંક = 2;
      જો( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_અંક = 3;
      }નહીંતર જો( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        પાછા SQLITE_CONSTRAINT;
      }
    }નહીંતર જો( seen & (1<<CARRAY_COLUMN_ગણતરી) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      પાછા SQLITE_CONSTRAINT;
    }
  }નહીંતર{
    pIdxInfo->estimatedCost = (દ્વિગુણ)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_અંક = 0;
  }
  પાછા SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
સ્થિર sqlite3_module carrayModule = {
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
સ્થિર શૂન્ય carrayBindDel(શૂન્ય *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  જો( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API પૂર્ણાંક sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  પૂર્ણાંક idx,
  શૂન્ય *aData,
  પૂર્ણાંક nData,
  પૂર્ણાંક mFlags,
  શૂન્ય (*xDestroy)(શૂન્ય*),
  શૂન્ય *pDestroy
){
  carray_bind *pNew = 0;
  પૂર્ણાંક i;
  પૂર્ણાંક rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  પ્રતિજ્ઞા( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  પ્રતિજ્ઞા( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  જો( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    જાઓ carray_bind_error;
  }

  pNew = sqlite3_malloc64(કદ(*pNew));
  જો( pNew==0 ){
    rc = SQLITE_NOMEM;
    જાઓ carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  જો( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    પસંદગી( mFlags ){
      સ્થિતિ CARRAY_INT32:   sz *= 4;                     તોડો;
      સ્થિતિ CARRAY_INT64:   sz *= 8;                     તોડો;
      સ્થિતિ CARRAY_DOUBLE:  sz *= 8;                     તોડો;
      સ્થિતિ CARRAY_TEXT:    sz *= કદ(અક્ષર*);         તોડો;
      મૂળભૂત:             sz *= કદ(સંરચના iovec);  તોડો;
    }
    જો( mFlags==CARRAY_TEXT ){
      માટે(i=0; i<nData; i++){
        અચલ અક્ષર *z = ((અક્ષર**)aData)[i];
        જો( z ) sz += strlen(z) + 1;
      }
    }નહીંતર જો( mFlags==CARRAY_BLOB ){
      માટે(i=0; i<nData; i++){
        sz += ((સંરચના iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    જો( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      જાઓ carray_bind_error;
    }

    જો( mFlags==CARRAY_TEXT ){
      અક્ષર **az = (અક્ષર**)pNew->aData;
      અક્ષર *z = (અક્ષર*)&az[nData];
      માટે(i=0; i<nData; i++){
        અચલ અક્ષર *zData = ((અક્ષર**)aData)[i];
        sqlite3_int64 n;
        જો( zData==0 ){
          az[i] = 0;
          ચાલુ;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }નહીંતર જો( mFlags==CARRAY_BLOB ){
      સંરચના iovec *p = (સંરચના iovec*)pNew->aData;
      unsigned અક્ષર *z = (unsigned અક્ષર*)&p[nData];
      માટે(i=0; i<nData; i++){
        size_t n = ((સંરચના iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((સંરચના iovec*)aData)[i].iov_base, n);
      }
    }નહીંતર જો( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }નહીંતર{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  પાછા sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  જો( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  પાછા rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API પૂર્ણાંક sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  પૂર્ણાંક idx,
  શૂન્ય *aData,
  પૂર્ણાંક nData,
  પૂર્ણાંક mFlags,
  શૂન્ય (*xDestroy)(શૂન્ય*)
){
  પાછા sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  પાછા sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
