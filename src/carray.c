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
#जर !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
#अन्तर्भाव "sqliteInt.h"
#जर defined(_WIN32) || defined(__RTP__) || defined(_WRS_KERNEL)
  संरचना iovec {
    शून्य *iov_base;
    माप_प्रकार iov_len;
  };
#अन्यथा
# include <sys/uio.h>
#endif

/*
** Names of allowed datatypes
*/
स्थिर स्थिरांक अक्षर *azCarrayType[] = {
  "int32", "int64", "double", "char*", "struct iovec"
};

/*
** Structure used to hold the sqlite3_carray_bind() information
*/
प्रकारव्याख्या संरचना carray_bind carray_bind;
संरचना carray_bind {
  शून्य *aData;                /* The data */
  पूर्णांक nData;                  /* Number of elements */
  पूर्णांक mFlags;                 /* Control flags */
  शून्य (*xDel)(शून्य*);        /* Destructor for aData */
  शून्य *pDel;                 /* Alternative argument to xDel() */
};


/* carray_cursor is a subclass of sqlite3_vtab_cursor which will
** serve as the underlying representation of a cursor that scans
** over rows of the result
*/
प्रकारव्याख्या संरचना carray_cursor carray_cursor;
संरचना carray_cursor {
  sqlite3_vtab_cursor base;  /* Base class - must be first */
  sqlite3_int64 iRowid;      /* The rowid */
  शून्य *pPtr;                /* Pointer to the array of values */
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
स्थिर पूर्णांक carrayConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  sqlite3_vtab *pNew;
  पूर्णांक rc;

/* Column numbers */
#define CARRAY_COLUMN_मूल्य   0
#define CARRAY_COLUMN_POINTER 1
#define CARRAY_COLUMN_मोजणी   2
#define CARRAY_COLUMN_CTYPE   3

  rc = sqlite3_declare_vtab(db,
     "CREATE TABLE x(value,pointer hidden,count hidden,ctype hidden)");
  जर( rc==SQLITE_OK ){
    pNew = *ppVtab = sqlite3_malloc( आकार(*pNew) );
    जर( pNew==0 ) परत SQLITE_NOMEM;
    स_रखो(pNew, 0, आकार(*pNew));
  }
  परत rc;
}

/*
** This method is the destructor for carray_cursor objects.
*/
स्थिर पूर्णांक carrayDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  परत SQLITE_OK;
}

/*
** Constructor for a new carray_cursor object.
*/
स्थिर पूर्णांक carrayOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor){
  carray_cursor *pCur;
  pCur = sqlite3_malloc( आकार(*pCur) );
  जर( pCur==0 ) परत SQLITE_NOMEM;
  स_रखो(pCur, 0, आकार(*pCur));
  *ppCursor = &pCur->base;
  परत SQLITE_OK;
}

/*
** Destructor for a carray_cursor.
*/
स्थिर पूर्णांक carrayClose(sqlite3_vtab_cursor *cur){
  sqlite3_free(cur);
  परत SQLITE_OK;
}


/*
** Advance a carray_cursor to its next row of output.
*/
स्थिर पूर्णांक carrayNext(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  pCur->iRowid++;
  परत SQLITE_OK;
}

/*
** Return values of columns for the row at which the carray_cursor
** is currently pointing.
*/
स्थिर पूर्णांक carrayColumn(
  sqlite3_vtab_cursor *cur,   /* The cursor */
  sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
  पूर्णांक i                       /* Which column to return */
){
  carray_cursor *pCur = (carray_cursor*)cur;
  sqlite3_int64 x = 0;
  निवड( i ){
    स्थिती CARRAY_COLUMN_POINTER:   परत SQLITE_OK;
    स्थिती CARRAY_COLUMN_मोजणी:     x = pCur->iCnt;   थांबा;
    स्थिती CARRAY_COLUMN_CTYPE: {
      sqlite3_result_text(ctx, azCarrayType[pCur->eType], -1, SQLITE_STATIC);
      परत SQLITE_OK;
    }
    पूर्वनिर्धारित: {
      निवड( pCur->eType ){
        स्थिती CARRAY_INT32: {
          पूर्णांक *p = (पूर्णांक*)pCur->pPtr;
          sqlite3_result_int(ctx, p[pCur->iRowid-1]);
          परत SQLITE_OK;
        }
        स्थिती CARRAY_INT64: {
          sqlite3_int64 *p = (sqlite3_int64*)pCur->pPtr;
          sqlite3_result_int64(ctx, p[pCur->iRowid-1]);
          परत SQLITE_OK;
        }
        स्थिती CARRAY_DOUBLE: {
          द्विगुण *p = (द्विगुण*)pCur->pPtr;
          sqlite3_result_double(ctx, p[pCur->iRowid-1]);
          परत SQLITE_OK;
        }
        स्थिती CARRAY_TEXT: {
          स्थिरांक अक्षर **p = (स्थिरांक अक्षर**)pCur->pPtr;
          sqlite3_result_text(ctx, p[pCur->iRowid-1], -1, SQLITE_TRANSIENT);
          परत SQLITE_OK;
        }
        पूर्वनिर्धारित: {
          स्थिरांक संरचना iovec *p = (संरचना iovec*)pCur->pPtr;
          प्रतिज्ञा( pCur->eType==CARRAY_BLOB );
          p += pCur->iRowid-1;
          जर( p->iov_len>0x7fffffff ){
            sqlite3_result_error_toobig(ctx);
            परत SQLITE_TOOBIG;
          }अन्यथा{
            sqlite3_result_blob(ctx, p->iov_base, (पूर्णांक)p->iov_len,
                                 SQLITE_TRANSIENT);
          }
          परत SQLITE_OK;
        }
      }
    }
  }
  sqlite3_result_int64(ctx, x);
  परत SQLITE_OK;
}

/*
** Return the rowid for the current row.  In this implementation, the
** rowid is the same as the output value.
*/
स्थिर पूर्णांक carrayRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  carray_cursor *pCur = (carray_cursor*)cur;
  *pRowid = pCur->iRowid;
  परत SQLITE_OK;
}

/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
स्थिर पूर्णांक carrayEof(sqlite3_vtab_cursor *cur){
  carray_cursor *pCur = (carray_cursor*)cur;
  परत pCur->iRowid>pCur->iCnt;
}

/*
** This method is called to "rewind" the carray_cursor object back
** to the first row of output.
*/
स्थिर पूर्णांक carrayFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिरांक अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  carray_cursor *pCur = (carray_cursor *)pVtabCursor;
  pCur->pPtr = 0;
  pCur->iCnt = 0;
  निवड( idx_अंक ){
    स्थिती 1: {
      carray_bind *pBind = sqlite3_मूल्य_pointer(argv[0], "carray-bind");
      जर( pBind==0 ) थांबा;
      pCur->pPtr = pBind->aData;
      pCur->iCnt = pBind->nData;
      pCur->eType = pBind->mFlags & 0x07;
      थांबा;
    }
    स्थिती 2:
    स्थिती 3: {
      pCur->pPtr = sqlite3_मूल्य_pointer(argv[0], "carray");
      pCur->iCnt = pCur->pPtr ? sqlite3_मूल्य_int64(argv[1]) : 0;
      जर( idx_अंक<3 ){
        pCur->eType = CARRAY_INT32;
      }अन्यथा{
        अचिन्हित अक्षर i;
        स्थिरांक अक्षर *zType = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[2]);
        साठी(i=0; i<आकार(azCarrayType)/आकार(azCarrayType[0]); i++){
          जर( sqlite3_stricmp(zType, azCarrayType[i])==0 ) थांबा;
        }
        जर( i>=आकार(azCarrayType)/आकार(azCarrayType[0]) ){
          pVtabCursor->pVtab->z_Err_संदेश = sqlite3_mprintf(
            "unknown datatype: %Q", zType);
          परत SQLITE_ERROR;
        }अन्यथा{
          pCur->eType = i;
        }
      }
      थांबा;
    }
  }
  pCur->iRowid = 1;
  परत SQLITE_OK;
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
स्थिर पूर्णांक carrayBestIndex(
  sqlite3_vtab *tab,
  sqlite3_index_info *pIdxInfo
){
  पूर्णांक i;                 /* Loop over constraints */
  पूर्णांक ptrIdx = -1;       /* Index of the pointer= constraint, or -1 if none */
  पूर्णांक cntIdx = -1;       /* Index of the count= constraint, or -1 if none */
  पूर्णांक ctypeIdx = -1;     /* Index of the ctype= constraint, or -1 if none */
  अचिन्हित seen = 0;     /* Bitmask of == constrainted columns */

  स्थिरांक संरचना sqlite3_index_constraint *pConstraint;
  pConstraint = pIdxInfo->aConstraint;
  साठी(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    जर( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) सुरूठेवा;
    जर( pConstraint->iColumn>=0 ) seen |= 1 << pConstraint->iColumn;
    जर( pConstraint->usable==0 ) सुरूठेवा;
    निवड( pConstraint->iColumn ){
      स्थिती CARRAY_COLUMN_POINTER:
        ptrIdx = i;
        थांबा;
      स्थिती CARRAY_COLUMN_मोजणी:
        cntIdx = i;
        थांबा;
      स्थिती CARRAY_COLUMN_CTYPE:
        ctypeIdx = i;
        थांबा;
    }
  }
  जर( ptrIdx>=0 ){
    pIdxInfo->aConstraintUsage[ptrIdx].argvIndex = 1;
    pIdxInfo->aConstraintUsage[ptrIdx].omit = 1;
    pIdxInfo->estimatedCost = (द्विगुण)1;
    pIdxInfo->estimatedRows = 100;
    pIdxInfo->idx_अंक = 1;
    जर( cntIdx>=0 ){
      pIdxInfo->aConstraintUsage[cntIdx].argvIndex = 2;
      pIdxInfo->aConstraintUsage[cntIdx].omit = 1;
      pIdxInfo->idx_अंक = 2;
      जर( ctypeIdx>=0 ){
        pIdxInfo->aConstraintUsage[ctypeIdx].argvIndex = 3;
        pIdxInfo->aConstraintUsage[ctypeIdx].omit = 1;
        pIdxInfo->idx_अंक = 3;
      }अन्यथा जर( seen & (1<<CARRAY_COLUMN_CTYPE) ){
        /* In a three-argument carray(), we need to know the value of all
        ** three arguments */
        परत SQLITE_CONSTRAINT;
      }
    }अन्यथा जर( seen & (1<<CARRAY_COLUMN_मोजणी) ){
      /* In a two-argument carray(), we need to know the value of both
      ** arguments */
      परत SQLITE_CONSTRAINT;
    }
  }अन्यथा{
    pIdxInfo->estimatedCost = (द्विगुण)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    pIdxInfo->idx_अंक = 0;
  }
  परत SQLITE_OK;
}

/*
** This following structure defines all the methods for the 
** carray virtual table.
*/
स्थिर sqlite3_module carrayModule = {
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
स्थिर शून्य carrayBindDel(शून्य *pPtr){
  carray_bind *p = (carray_bind*)pPtr;
  जर( p->xDel!=SQLITE_STATIC ){
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
  शून्य *aData,
  पूर्णांक nData,
  पूर्णांक mFlags,
  शून्य (*xDestroy)(शून्य*),
  शून्य *pDestroy
){
  carray_bind *pNew = 0;
  पूर्णांक i;
  पूर्णांक rc = SQLITE_OK;
  
  /* Ensure that the mFlags value is acceptable. */
  प्रतिज्ञा( CARRAY_INT32==0 && CARRAY_INT64==1 && CARRAY_DOUBLE==2 );
  प्रतिज्ञा( CARRAY_TEXT==3 && CARRAY_BLOB==4 );
  जर( mFlags<CARRAY_INT32 || mFlags>CARRAY_BLOB ){
    rc = SQLITE_ERROR;
    जा carray_bind_error;
  }

  pNew = sqlite3_malloc64(आकार(*pNew));
  जर( pNew==0 ){
    rc = SQLITE_NOMEM;
    जा carray_bind_error;
  }

  pNew->nData = nData;
  pNew->mFlags = mFlags;
  जर( xDestroy==SQLITE_TRANSIENT ){
    sqlite3_int64 sz = nData;
    निवड( mFlags ){
      स्थिती CARRAY_INT32:   sz *= 4;                     थांबा;
      स्थिती CARRAY_INT64:   sz *= 8;                     थांबा;
      स्थिती CARRAY_DOUBLE:  sz *= 8;                     थांबा;
      स्थिती CARRAY_TEXT:    sz *= आकार(अक्षर*);         थांबा;
      पूर्वनिर्धारित:             sz *= आकार(संरचना iovec);  थांबा;
    }
    जर( mFlags==CARRAY_TEXT ){
      साठी(i=0; i<nData; i++){
        स्थिरांक अक्षर *z = ((अक्षर**)aData)[i];
        जर( z ) sz += म_माप(z) + 1;
      }
    }अन्यथा जर( mFlags==CARRAY_BLOB ){
      साठी(i=0; i<nData; i++){
        sz += ((संरचना iovec*)aData)[i].iov_len;
      }
    } 

    pNew->aData = sqlite3_malloc64( sz>0 ? sz : 1 );
    जर( pNew->aData==0 ){
      rc = SQLITE_NOMEM;
      जा carray_bind_error;
    }

    जर( mFlags==CARRAY_TEXT ){
      अक्षर **az = (अक्षर**)pNew->aData;
      अक्षर *z = (अक्षर*)&az[nData];
      साठी(i=0; i<nData; i++){
        स्थिरांक अक्षर *zData = ((अक्षर**)aData)[i];
        sqlite3_int64 n;
        जर( zData==0 ){
          az[i] = 0;
          सुरूठेवा;
        }
        az[i] = z;
        n = म_माप(zData);
        स_नकल(z, zData, n+1);
        z += n+1;
      }
    }अन्यथा जर( mFlags==CARRAY_BLOB ){
      संरचना iovec *p = (संरचना iovec*)pNew->aData;
      अचिन्हित अक्षर *z = (अचिन्हित अक्षर*)&p[nData];
      साठी(i=0; i<nData; i++){
        माप_प्रकार n = ((संरचना iovec*)aData)[i].iov_len;
        p[i].iov_len = n;
        p[i].iov_base = z;
        z += n;
        स_नकल(p[i].iov_base, ((संरचना iovec*)aData)[i].iov_base, n);
      }
    }अन्यथा जर( sz>0 ){
      स_नकल(pNew->aData, aData, sz);
    }
    pNew->xDel = sqlite3_free;
    pNew->pDel = pNew->aData;
  }अन्यथा{
    pNew->aData = aData;
    pNew->xDel = xDestroy;
    pNew->pDel = pDestroy;
  }
  परत sqlite3_bind_pointer(pStmt, idx, pNew, "carray-bind", carrayBindDel);
 
 carray_bind_error:
  जर( xDestroy!=SQLITE_STATIC && xDestroy!=SQLITE_TRANSIENT ){
    xDestroy(pDestroy);
  }
  sqlite3_free(pNew);
  परत rc;
}

/*
** Invoke this interface in order to bind to the single-argument
** version of CARRAY().  Same as sqlite3_carray_bind_v2() with the
** pDestroy parameter set to NULL.
*/
SQLITE_API पूर्णांक sqlite3_carray_bind(
  sqlite3_stmt *pStmt,
  पूर्णांक idx,
  शून्य *aData,
  पूर्णांक nData,
  पूर्णांक mFlags,
  शून्य (*xDestroy)(शून्य*)
){
  परत sqlite3_carray_bind_v2(pStmt,idx,aData,nData,mFlags,xDestroy,aData);
}

/*
** Invoke this routine to register the carray() function.
*/
Module *sqlite3CarrayRegister(sqlite3 *db){
  परत sqlite3VtabCreateModule(db, "carray", &carrayModule, 0, 0);
}

#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY) */
