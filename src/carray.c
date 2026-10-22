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
#ако !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#include "sqliteInt.h"
#ако defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  struct iovec {
    void *iov_base;
    size_t iov_len;
  };
#инако
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
static const char *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
typedef struct carray_bind carray_bind;
struct carray_bind {
  void *aData;                /* The data */
  цео nData;                  /* Number of elements */
  цео mFlags;                 /* Control flags */
  void (*xDel)(void*);        /* Destructor for aData */
  void *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
typedef struct carray_cursor carray_cursor;
struct carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  void *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  unsigned char eType;       /* One of the CARRAY_type values */
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
static цео carrayConnect(
  sqlite3 *db,
  void *pAux,
  цео argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  sqlite3_vtab *pNew;
  цео rc;

/* Column numbers */
#define CARRAY_COLUMN_VALUE   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_COUNT   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  ако( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( sizeof(*pNew) );
    ако( pNew==0 ) врати SQLITE_NOMEM;
    memset(pNew, 0, sizeof(*pNew));
  }
  врати rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
static цео carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  врати SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
static цео carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( sizeof(*pCur) );
  ако( pCur==0 ) врати SQLITE_NOMEM;
  memset(pCur, 0, sizeof(*pCur));
  *ppCursor = &pCur->base;
  врати SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
static цео carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  врати SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
static цео carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  врати SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
static цео carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  цео i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  switch( i ){
    case CARRAY_COLUMN_POINTER:   врати SQLITE_OK;
    case CARRAY_COLUMN_COUNT:     x = pCur->iCnt;   break;
    case CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      врати SQLITE_OK;
    }
    default: {
      switch( pCur->eType ){
        case CARRAY_INT32: {
          цео *p = (цео*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          врати SQLITE_OK;
        }
        case CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          врати SQLITE_OK;
        }
        case CARRAY_DOUBLE: {
          double *p = (double*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          врати SQLITE_OK;
        }
        case CARRAY_TEXT: {
          const char **p = (const char**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          врати SQLITE_OK;
        }
        default: {
          const struct iovec *p = (struct iovec*)pCur->pPtr;
          assert( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          ако( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            врати SQLITE_TOOBIG;
          }инако{
            sqlite3_result_blob(ctx, p->iov_base, (цео)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          врати SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  врати SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
static цео carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  врати SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static цео carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  врати pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
static цео carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  цео idxNum, const char *idxStr,
  цео argc, sqlite3_value **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  switch( idxNum ){
    case 1: {
      carray_bind *pBind = sqlite3_value_pointer(argv[0], "carray-bind");
      ако( pBind==0 ) break;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      break;
    }
    case 2:
    case 3: {
      pCur->pPtr = sqlite3_value_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_value_int64(argv[1]) : 0;
      ако( idxNum<3 ){
        pCur->eType = CARRAY_INT32;
      }инако{
        unsigned char i;
        const char *zType = (const char*)sqlite3_value_text(argv[2]);
        за(i=0; i<sizeof(azCarrayType)/sizeof(azCarrayType[0]); i++){
          ако( sqlite3_stricmp(zType, azCarrayType[i])==0 ) break;
        }
        ако( i>=sizeof(azCarrayType)/sizeof(azCarrayType[0]) ){
          pVtabCursor->pVtab->zErrMsg = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          врати SQLITE_ERROR;
        }инако{
          pCur->eType = i;
        }
      }
      break;
    }
  }
  pCur->iRowid = 1;
  врати SQLITE_OK;
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
static цео carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  цео i;                 /* Loop over constraints */
  цео ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  цео cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  цео ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  unsigned seen = 0;     /* Bitmask of == constrainted columns */

  const struct sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  за(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ако( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) continue;
    ако( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    ако( pConstraint->usable==0 ) continue;
    switch( pConstraint->iColumn ){
      case CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        break;
      case CARRAY_COLUMN_COUNT:
        cntIdx = i;
        break;
      case CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        break;
    }
  }
  ако( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (double)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idxNum = 1;
    ако( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idxNum = 2;
      ако( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idxNum = 3;
      }инако ако( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        врати SQLITE_CONSTRAINT;
      }
    }инако ако( seen & (1<<CARRAY_COLUMN_COUNT) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      врати SQLITE_CONSTRAINT;
    }
  }инако{
    pIdxInfo->estimatedCost = (double)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idxNum = 0;
  }
  врати SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
static sqlite3_module carrayModule = {
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
static void carrayBindDel(void *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  ако( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API цео sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  цео idx,
  void *aData,
  цео nData,
  цео mFlags,
  void (*xDestroy)(void*),
  void *pDestroy
){
  carray_bind *pNew = 0;
  цео i;
  цео rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  assert( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  assert( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  ако( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    goto carray_bind_error;
  }

  pNew = sqlite3_malloc64(sizeof(*pNew));
  ако( pNew==0 ){
    rc = SQLITE_NOMEM;
    goto carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  ако( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    switch( mFlags ){
      case CARRAY_INT32:   sz *= 4;                     break;
      case CARRAY_INT64:   sz *= 8;                     break;
      case CARRAY_DOUBLE:  sz *= 8;                     break;
      case CARRAY_TEXT:    sz *= sizeof(char*);         break;
      default:             sz *= sizeof(struct iovec);  break;
    }
    ако( mFlags==CARRAY_TEXT ){
      за(i=0; i<nData; i++){
        const char *z = ((char**)aData)[i];
        ако( z ) sz += strlen(z) + 1;
      }
    }инако ако( mFlags==CARRAY_BLOB ){
      за(i=0; i<nData; i++){
        sz += ((struct iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    ако( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      goto carray_bind_error;
    }

    ако( mFlags==CARRAY_TEXT ){
      char **az = (char**)pNew->aData;
      char *z = (char*)&az[nData];
      за(i=0; i<nData; i++){
        const char *zData = ((char**)aData)[i];
        sqlite3_int64 n;
        ако( zData==0 ){
          az[i] = 0;
          continue;
        }
        az[i] = z;
        n = strlen(zData);
        memcpy(z, zData, n+1);
        z += n+1;
      }
    }инако ако( mFlags==CARRAY_BLOB ){
      struct iovec *p = (struct iovec*)pNew->aData;
      unsigned char *z = (unsigned char*)&p[nData];
      за(i=0; i<nData; i++){
        size_t n = ((struct iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        memcpy(p[i].iov_base, ((struct iovec*)aData)[i].iov_base, n);
      }
    }инако ако( sz>0 ){
      memcpy(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }инако{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  врати sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  ако( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  врати rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API цео sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  цео idx,
  void *aData,
  цео nData,
  цео mFlags,
  void (*xDestroy)(void*)
){
  врати sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  врати sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
