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
#अगर !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#समावेश "sqliteInt.h"
#अगर defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  काष्ठा iovec {
    व्योम *iov_base;
    माप_प्रकार iov_len;
  };
#अन्यथा
# include <sys/uio.h>
#पूर्ण_अगर

/*
** Names of allowed datatypes
*/
अटल स्थिर अक्षर *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
प्रकार काष्ठा carray_bind carray_bind;
काष्ठा carray_bind {
  व्योम *a_तथ्य;                /* The data */
  पूर्णांक n_तथ्य;                  /* Number of elements */
  पूर्णांक mFlags;                 /* Control flags */
  व्योम (*xDel)(व्योम*);        /* Destructor for aData */
  व्योम *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
प्रकार काष्ठा carray_cursor carray_cursor;
काष्ठा carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  व्योम *pPtr;                /* Pointer to the array of values */
  sqlite3_int64 iCnt;        /* Number of integers in the array */
  अचिन्हित अक्षर eType;       /* One of the CARRAY_type values */
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
अटल पूर्णांक carrayConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  sqlite3_vtab *pNew;
  पूर्णांक rc;

/* Column numbers */
#घोषणा CARRAY_COLUMN_मूल्य   0
#घोषणा CARRAY_COLUMN_POINTER 1
#घोषणा CARRAY_COLUMN_गिनती   2
#घोषणा CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  अगर( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( माप(*pNew) );
    अगर( pNew==0 ) वापस SQLITE_NOMEM;
    स_रखो(pNew, 0, माप(*pNew));
  }
  वापस rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
अटल पूर्णांक carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  वापस SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
अटल पूर्णांक carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( माप(*pCur) );
  अगर( pCur==0 ) वापस SQLITE_NOMEM;
  स_रखो(pCur, 0, माप(*pCur));
  *ppCursor = &pCur->base;
  वापस SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
अटल पूर्णांक carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  वापस SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
अटल पूर्णांक carray_अग्र(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  वापस SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
अटल पूर्णांक carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  पूर्णांक i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  चयन( i ){
    हाल CARRAY_COLUMN_POINTER:   वापस SQLITE_OK;
    हाल CARRAY_COLUMN_गिनती:     x = pCur->iCnt;   अवरोध;
    हाल CARRAY_COLUMN_CTYPE: {
      sqlite3_परिणाम_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      वापस SQLITE_OK;
    }
    शेष: {
      चयन( pCur->eType ){
        हाल CARRAY_INT32: {
          पूर्णांक *p = (पूर्णांक*)pCur->pPtr;
          sqlite3_परिणाम_int(ctx, p[pCur->iRowid-1]);
          वापस SQLITE_OK;
        }
        हाल CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_परिणाम_int64(ctx, p[pCur->iRowid-1]);
          वापस SQLITE_OK;
        }
        हाल CARRAY_DOUBLE: {
          द्विगुन *p = (द्विगुन*)pCur->pPtr;
          sqlite3_परिणाम_double(ctx, p[pCur->iRowid-1]);
          वापस SQLITE_OK;
        }
        हाल CARRAY_TEXT: {
          स्थिर अक्षर **p = (स्थिर अक्षर**)pCur->pPtr;
          sqlite3_परिणाम_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          वापस SQLITE_OK;
        }
        शेष: {
          स्थिर काष्ठा iovec *p = (काष्ठा iovec*)pCur->pPtr;
          निश्चित( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          अगर( p->iov_len>0x7fffffff ){
            sqlite3_परिणाम_error_toobig(ctx);
            वापस SQLITE_TOOBIG;
          }अन्यथा{
            sqlite3_परिणाम_blob(ctx, p->iov_base, (पूर्णांक)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          वापस SQLITE_OK;
        }
      }
    }
  }
  sqlite3_परिणाम_int64(ctx, x);
  वापस SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
अटल पूर्णांक carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  वापस SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
अटल पूर्णांक carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  वापस pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
अटल पूर्णांक carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  चयन( idx_अंक ){
    हाल 1: {
      carray_bind *pBind = sqlite3_मूल्य_pointer(argv[0], "carray-bind");
      अगर( pBind==0 ) अवरोध;
      pCur->pPtr = pBind->a_तथ्य;
      pCur->iCnt = pBind->n_तथ्य;
      pCur->eType = pBind->mFlags & 0x07;
      अवरोध;
    }
    हाल 2:
    हाल 3: {
      pCur->pPtr = sqlite3_मूल्य_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_मूल्य_int64(argv[1]) : 0;
      अगर( idx_अंक<3 ){
        pCur->eType = CARRAY_INT32;
      }अन्यथा{
        अचिन्हित अक्षर i;
        स्थिर अक्षर *zType = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[2]);
        क्रम(i=0; i<माप(azCarrayType)/माप(azCarrayType[0]); i++){
          अगर( sqlite3_stricmp(zType, azCarrayType[i])==0 ) अवरोध;
        }
        अगर( i>=माप(azCarrayType)/माप(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_संदेश = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          वापस SQLITE_ERROR;
        }अन्यथा{
          pCur->eType = i;
        }
      }
      अवरोध;
    }
  }
  pCur->iRowid = 1;
  वापस SQLITE_OK;
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
अटल पूर्णांक carray_Best_सूचकांक(
  sqlite3_vtab *tab,
  sqlite3_सूचकांक_info *pIdxInfo
){
  पूर्णांक i;                 /* Loop over constraints */
  पूर्णांक ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  पूर्णांक cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  पूर्णांक ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  अचिन्हित seen = 0;     /* Bitmask of == constrainted columns */

  स्थिर काष्ठा sqlite3_सूचकांक_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  क्रम(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    अगर( pConstraint->op!=SQLITE_सूचकांक_CONSTRAINT_EQ ) जारी;
    अगर( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    अगर( pConstraint->usable==0 ) जारी;
    चयन( pConstraint->iColumn ){
      हाल CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        अवरोध;
      हाल CARRAY_COLUMN_गिनती:
        cntIdx = i;
        अवरोध;
      हाल CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        अवरोध;
    }
  }
  अगर( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argv_सूचकांक = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (द्विगुन)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_अंक = 1;
    अगर( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argv_सूचकांक = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_अंक = 2;
      अगर( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argv_सूचकांक = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_अंक = 3;
      }अन्यथा अगर( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        वापस SQLITE_CONSTRAINT;
      }
    }अन्यथा अगर( seen & (1<<CARRAY_COLUMN_गिनती) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      वापस SQLITE_CONSTRAINT;
    }
  }अन्यथा{
    pIdxInfo->estimatedCost = (द्विगुन)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_अंक = 0;
  }
  वापस SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
अटल sqlite3_module carrayModule = {
  0,                         /* iVersion */
  0,                         /* xCreate */
  carrayConnect,             /* xConnect */
  carray_Best_सूचकांक,           /* xBestIndex */
  carrayDisconnect,          /* xDisconnect */
  0,                         /* xDestroy */
  carrayOpen,                /* xOpen - open a cursor */
  carrayClose,               /* xClose - close a cursor */
  carrayFilter,              /* xFilter - configure scan constraints */
  carray_अग्र,                /* xNext - advance a cursor */
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
अटल व्योम carrayBindDel(व्योम *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  अगर( p->xDel!=SQLITE_STATIC ){
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
SQLITE_API पूर्णांक sqlite3_carray_bind_v2(
  sqlite3_stmt *pStmt,
  पूर्णांक idx,
  व्योम *a_तथ्य,
  पूर्णांक n_तथ्य,
  पूर्णांक mFlags,
  व्योम (*xDestroy)(व्योम*),
  व्योम *pDestroy
){
  carray_bind *pNew = 0;
  पूर्णांक i;
  पूर्णांक rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  निश्चित( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  निश्चित( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  अगर( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    जाओ carray_bind_error;
  }

  pNew = sqlite3_malloc64(माप(*pNew));
  अगर( pNew==0 ){
    rc = SQLITE_NOMEM;
    जाओ carray_bind_error;
  }

  pNew->n_तथ्य = n_तथ्य;
  pNew->mFlags = mFlags;
  अगर( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = n_तथ्य;
    चयन( mFlags ){
      हाल CARRAY_INT32:   sz *= 4;                     अवरोध;
      हाल CARRAY_INT64:   sz *= 8;                     अवरोध;
      हाल CARRAY_DOUBLE:  sz *= 8;                     अवरोध;
      हाल CARRAY_TEXT:    sz *= माप(अक्षर*);         अवरोध;
      शेष:             sz *= माप(काष्ठा iovec);  अवरोध;
    }
    अगर( mFlags==CARRAY_TEXT ){
      क्रम(i=0; i<n_तथ्य; i++){
        स्थिर अक्षर *z = ((अक्षर**)a_तथ्य)[i];
        अगर( z ) sz += म_माप(z) + 1;
      }
    }अन्यथा अगर( mFlags==CARRAY_BLOB ){
      क्रम(i=0; i<n_तथ्य; i++){
        sz += ((काष्ठा iovec*)a_तथ्य)[i].iov_len;
      }
    } 

    pNew->a_तथ्य = sqlite3_malloc64( sz>0 ? sz : 1 );
    अगर( pNew->a_तथ्य==0 ){
      rc = SQLITE_NOMEM;
      जाओ carray_bind_error;
    }

    अगर( mFlags==CARRAY_TEXT ){
      अक्षर **az = (अक्षर**)pNew->a_तथ्य;
      अक्षर *z = (अक्षर*)&az[n_तथ्य];
      क्रम(i=0; i<n_तथ्य; i++){
        स्थिर अक्षर *z_तथ्य = ((अक्षर**)a_तथ्य)[i];
        sqlite3_int64 n;
        अगर( z_तथ्य==0 ){
          az[i] = 0;
          जारी;
        }
        az[i] = z;
        n = म_माप(z_तथ्य);
        स_नकल(z, z_तथ्य, n+1);
        z += n+1;
      }
    }अन्यथा अगर( mFlags==CARRAY_BLOB ){
      काष्ठा iovec *p = (काष्ठा iovec*)pNew->a_तथ्य;
      अचिन्हित अक्षर *z = (अचिन्हित अक्षर*)&p[n_तथ्य];
      क्रम(i=0; i<n_तथ्य; i++){
        माप_प्रकार n = ((काष्ठा iovec*)a_तथ्य)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        स_नकल(p[i].iov_base, ((काष्ठा iovec*)a_तथ्य)[i].iov_base, n);
      }
    }अन्यथा अगर( sz>0 ){
      स_नकल(pNew->a_तथ्य, a_तथ्य, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->a_तथ्य;
  }अन्यथा{
    pNew->a_तथ्य = a_तथ्य;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  वापस sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  अगर( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  वापस rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API पूर्णांक sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  पूर्णांक idx,
  व्योम *a_तथ्य,
  पूर्णांक n_तथ्य,
  पूर्णांक mFlags,
  व्योम (*xDestroy)(व्योम*)
){
  वापस sqlite3_carray_bind_v2(pStmt,idx,a_तथ्य,n_तथ्य,mFlags,xDestroy,a_तथ्य);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  वापस sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#पूर्ण_अगर /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
