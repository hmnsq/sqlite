/*
** 2016-03-01
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the virtual table xBestIndex method and the query
** planner.
*/


/*
** INSTRUCTIONS
**
** This module exports a single tcl command - [register_tcl_module]. When
** invoked, it registers a special virtual table module with a database
** connection.
**
** The virtual table is currently read-only. And always returns zero rows.
** It is created with a single argument - the name of a Tcl command - as
** follows:
**
**   CREATE VIRTUAL TABLE x1 USING tcl(tcl_command);
**
** The command [tcl_command] is invoked when the table is first created (or
** connected), when the xBestIndex() method is invoked and when the xFilter()
** method is called. When it is created (or connected), it is invoked as
** follows:
**
**   tcl_command xConnect
**
** In this case the return value of the script is passed to the
** sqlite3_declare_vtab() function to create the virtual table schema.
**
** When the xBestIndex() method is called by SQLite, the Tcl command is
** invoked as:
**
**   tcl_command xBestIndex CONSTRAINTS ORDERBY MASK
**
** where CONSTRAINTS is a tcl representation of the aConstraints[] array,
** ORDERBY is a representation of the contents of the aOrderBy[] array and
** MASK is a copy of sqlite3_index_info.colUsed. For example if the virtual
** table is declared as:
**
**   CREATE TABLE x1(a, b, c)
**
** and the query is:
**
**   SELECT * FROM x1 WHERE a=? AND c<? ORDER BY b, c;
**
** then the Tcl command is:
**
**   tcl_command xBestIndex                                  \
**     {{op eq column 0 usable 1} {op lt column 2 usable 1}} \
**     {{column 1 desc 0} {column 2 desc 0}}                 \
**     7
**
** The return value of the script is a list of key-value pairs used to
** populate the output fields of the sqlite3_index_info structure. Possible
** keys and the usage of the accompanying values are:
** 
**   "orderby"          (value of orderByConsumed flag)
**   "cost"             (value of estimatedCost field)
**   "rows"             (value of estimatedRows field)
**   "use"              (index of used constraint in aConstraint[])
**   "omit"             (like "use", but also sets omit flag)
**   "idxnum"           (value of idxNum field)
**   "idxstr"           (value of idxStr field)
**
** Refer to code below for further details.
**
** When SQLite calls the xFilter() method, this module invokes the following
** Tcl script:
**
**   tcl_command xFilter IDXNUM IDXSTR ARGLIST
**
** IDXNUM and IDXSTR are the values of the idxNum and idxStr parameters
** passed to xFilter. ARGLIST is a Tcl list containing each of the arguments
** passed to xFilter in text form.
**
** As with xBestIndex(), the return value of the script is interpreted as a
** list of key-value pairs. There is currently only one key defined - "sql".
** The value must be the full text of an SQL statement that returns the data
** for the current scan. The leftmost column returned by the SELECT is assumed
** to contain the rowid. Other columns must follow, in order from left to
** right.
*/


#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ tcl_vtab tcl_vtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ tcl_cursor tcl_cursor;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestFindFunction TestFindFunction;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
ಸಂರಚನಾ tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
ಸಂರಚನಾ tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

ಸಂರಚನಾ TestFindFunction {
  tcl_vtab *pTab;
  ಧ್ರುವ ವರ್ಣ *zName;
  TestFindFunction *pNext;
};

ಸಂರಚನಾ TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tclDequote(ವರ್ಣ *z){
  ವರ್ಣ q = z[0];

  /* Set stack variable q to the close-quote character */
  ಯದಿ( q=='[' || q=='\'' || q=='"' || q=='`' ){
    ಪೂರ್ಣಾಙ್ಕ iIn = 1;
    ಪೂರ್ಣಾಙ್ಕ iOut = 0;
    ಯದಿ( q=='[' ) q = ']';  

    ಯಾವತ್( ALWAYS(z[iIn]) ){
      ಯದಿ( z[iIn]==q ){
        ಯದಿ( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }ಅನ್ಯಥಾ{
        z[iOut++] = z[iIn++];
      }
    }

    z[iOut] = '\0';
  }
}

/*
** This function is the implementation of both the xConnect and xCreate
** methods of the fs virtual table.
**
** The argv[] array contains the following:
**
**   argv[0]   -> module name  ("fs")
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> other module argument fields.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  ವರ್ಣ *zCmd = 0;
  Tcl_Obj *pScript = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  ಯದಿ( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  ಯದಿ( argc==4 ){
    zCmd = sqlite3_malloc64(strlen(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(ಪರಿಮಾಣ(tcl_vtab));
  ಯದಿ( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, ಪರಿಮಾಣ(tcl_vtab));

    ಯದಿ( zCmd ){
      memcpy(zCmd, argv[3], strlen(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }ಅನ್ಯಥಾ{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_ಎಣಿಕೆ(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    ಯದಿ( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_GetStringResult(interp));
      ಯದಿ( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }ಅನ್ಯಥಾ{
        rc = SQLITE_ERROR;
      }
    }ಅನ್ಯಥಾ{
      rc = sqlite3_declare_vtab(db, Tcl_GetStringResult(interp));
      ಯದಿ( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    ಯದಿ( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }ಅನ್ಯಥಾ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* The xDisconnect and xDestroy methods are also the same */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  ಯಾವತ್( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->pNext;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_ಎಣಿಕೆ(pTab->pCmd);
  sqlite3_free(pTab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(ಪರಿಮಾಣ(tcl_cursor));
  ಯದಿ( pCur==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pCur, 0, ಪರಿಮಾಣ(tcl_cursor));
  *ppCursor = &pCur->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  ಯದಿ( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclNext(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ಯದಿ( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_step(pCsr->pStmt);
    ಯದಿ( rc!=SQLITE_ROW ){
      ಧ್ರುವ ವರ್ಣ *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      ಯದಿ( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪೂರ್ಣಾಙ್ಕ rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_ಅಂಕೆ));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pArg);
  ಪ್ರತಿ(ii=0; ii<argc; ii++){
    ಧ್ರುವ ವರ್ಣ *zVal = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[ii]);
    Tcl_Obj *pVal;
    ಯದಿ( zVal==0 ){
      sqlite3_ಮೌಲ್ಯ *pMem;
      pVal = Tcl_NewObj();
      ಪ್ರತಿ(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_next(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pMem);
        ಯದಿ( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }ಅನ್ಯಥಾ{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }ಅನ್ಯಥಾ{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  ಯದಿ( rc!=TCL_OK ){
    ಧ್ರುವ ವರ್ಣ *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
  }ಅನ್ಯಥಾ{
    /* Analyze the scripts return value. The return value should be a tcl 
    ** list object with an even number of elements. The first element of each
    ** pair must be one of:
    ** 
    **   "sql"          (SQL statement to return data)
    */
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_Obj **apElem = 0;
    Tcl_Size nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    ಯದಿ( rc!=TCL_OK ){
      ಧ್ರುವ ವರ್ಣ *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿ(ii=0; rc==SQLITE_OK && ii<(ಪೂರ್ಣಾಙ್ಕ)nElem; ii+=2){
        ಧ್ರುವ ವರ್ಣ *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ಯದಿ( sqlite3_stricmp("sql", zCmd)==0 ){
          ಧ್ರುವ ವರ್ಣ *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          ಯದಿ( rc!=SQLITE_OK ){
            ಧ್ರುವ ವರ್ಣ *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }ಅನ್ಯಥಾ{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  ಯದಿ( rc==SQLITE_OK ){
    rc = tclNext(pVtabCursor);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಙ್ಕ i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_result_ಮೌಲ್ಯ(ctx, sqlite3_column_ಮೌಲ್ಯ(pCsr->pStmt, i+1));
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ಪ್ರತ್ಯಾಗಮ (pCsr->pStmt==0);
}

ಸ್ಥಿರ ಶೂನ್ಯ testBestIndexObjConstraints(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  ಪೂರ್ಣಾಙ್ಕ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRes);
  ಪ್ರತಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ಸಂರಚನಾ sqlite3_index_constraint ಧ್ರುವ *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    ಧ್ರುವ ವರ್ಣ *zOp = 0;

    Tcl_Incr_Ref_ಎಣಿಕೆ(pElem);

    ವಿಕಲ್ಪ( pCons->op ){
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_EQ:
        zOp = "eq"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_GT:
        zOp = "gt"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_LE:
        zOp = "le"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_LT:
        zOp = "lt"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_GE:
        zOp = "ge"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_MATCH:
        zOp = "match"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_LIKE:
        zOp = "like"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_GLOB:
        zOp = "glob"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_REGEXP:
        zOp = "regexp"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_NE:
        zOp = "ne"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_ISNOT:
        zOp = "isnot"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_ISNULL:
        zOp = "isnull"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_IS:
        zOp = "is"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_LIMIT:
        zOp = "limit"; ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_OFFSET:
        zOp = "offset"; ಭಙ್ಗ;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    ಯದಿ( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }ಅನ್ಯಥಾ{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pRes);
}

ಸ್ಥಿರ ಶೂನ್ಯ testBestIndexObjOrderby(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  ಪೂರ್ಣಾಙ್ಕ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRes);
  ಪ್ರತಿ(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    ಸಂರಚನಾ sqlite3_index_orderby ಧ್ರುವ *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_ಎಣಿಕೆ(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pRes);
}

/*
** Implementation of the handle passed to each xBestIndex callback. This
** object features the following sub-commands:
**
**    $hdl constraints
**    $hdl orderby
**    $hdl mask
**
**    $hdl distinct
**      Return the result (an integer) of calling sqlite3_vtab_distinct()
**      on the index-info structure.
**
**    $hdl in IDX BOOLEAN
**      Wrapper around sqlite3_vtab_in(). Returns an integer.
**
**    $hdl rhs_value IDX ?DEFAULT?
**      Wrapper around sqlite3_vtab_rhs_value().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI testBestIndexObj(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಧ್ರುವ ವರ್ಣ *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  ಪೂರ್ಣಾಙ್ಕ ii;
  sqlite3_index_info *pIdxInfo = (sqlite3_index_info*)clientData;

  ಯದಿ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "SUB-COMMAND");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIndexFromObj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( ii<4 && objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( ii==4 && objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "INDEX BOOLEAN");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "INDEX ?DEFAULT?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ವಿಕಲ್ಪ( ii ){
    ಪಕ್ಷ 0: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      testBestIndexObjConstraints(interp, pIdxInfo);
      ಭಙ್ಗ;

    ಪಕ್ಷ 1: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      testBestIndexObjOrderby(interp, pIdxInfo);
      ಭಙ್ಗ;

    ಪಕ್ಷ 2: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_SetObjResult(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      ಭಙ್ಗ;

    ಪಕ್ಷ 3: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      ಪೂರ್ಣಾಙ್ಕ bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_SetObjResult(interp, Tcl_NewIntObj(bDistinct));
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ 4: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      ಪೂರ್ಣಾಙ್ಕ iCons;
      ಪೂರ್ಣಾಙ್ಕ bHandle;
      ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      Tcl_SetObjResult(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ 5: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      ಪೂರ್ಣಾಙ್ಕ iCons = 0;
      ಪೂರ್ಣಾಙ್ಕ rc;
      sqlite3_ಮೌಲ್ಯ *pVal = 0;
      ಧ್ರುವ ವರ್ಣ *zVal = "";
      ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_ಮೌಲ್ಯ(pIdxInfo, iCons, &pVal);
      ಯದಿ( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಯದಿ( pVal ){
        zVal = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(pVal);
      }ಅನ್ಯಥಾ ಯದಿ( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zVal, -1));
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ 6: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      ಪೂರ್ಣಾಙ್ಕ iCons = 0;
      ಧ್ರುವ ವರ್ಣ *zColl = "";
      ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zColl, -1));
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ iNext = 43;
  ವರ್ಣ zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(ಪರಿಮಾಣ(zHdl), zHdl, "bestindex%d", iNext++);
  Tcl_CreateObjCommand(interp, zHdl, testBestIndexObj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pScript);

  ಯದಿ( rc!=TCL_OK ){
    ಧ್ರುವ ವರ್ಣ *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
  }ಅನ್ಯಥಾ{
    /* Analyze the scripts return value. The return value should be a tcl 
    ** list object with an even number of elements. The first element of each
    ** pair must be one of:
    ** 
    **   "orderby"          (value of orderByConsumed flag)
    **   "cost"             (value of estimatedCost field)
    **   "rows"             (value of estimatedRows field)
    **   "use"              (index of used constraint in aConstraint[])
    **   "idxnum"           (value of idxNum field)
    **   "idxstr"           (value of idxStr field)
    **   "omit"             (index of omitted constraint in aConstraint[])
    */
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_Obj **apElem = 0;
    Tcl_Size nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    ಯದಿ( rc!=TCL_OK ){
      ಧ್ರುವ ವರ್ಣ *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ ii;
      ಪೂರ್ಣಾಙ್ಕ iArgv = 1;
      ಪ್ರತಿ(ii=0; rc==SQLITE_OK && ii<(ಪೂರ್ಣಾಙ್ಕ)nElem; ii+=2){
        ಧ್ರುವ ವರ್ಣ *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ಯದಿ( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }ಅನ್ಯಥಾ
        ಯದಿ( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }ಅನ್ಯಥಾ
        ಯದಿ( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_ಅಂಕೆ);
        }ಅನ್ಯಥಾ
        ಯದಿ( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }ಅನ್ಯಥಾ
        ಯದಿ( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }ಅನ್ಯಥಾ
        ಯದಿ( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          ಪೂರ್ಣಾಙ್ಕ iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          ಯದಿ( rc==SQLITE_OK ){
            ಯದಿ( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected: %d", iCons);
            }ಅನ್ಯಥಾ{
              ಪೂರ್ಣಾಙ್ಕ bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argvIndex = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }ಅನ್ಯಥಾ
        ಯದಿ( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", Tcl_GetString(p));
        }ಅನ್ಯಥಾ{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        ಯದಿ( rc!=SQLITE_OK && pTab->base.z_Err_ಸಂದೇಶ==0 ){
          ಧ್ರುವ ವರ್ಣ *zErr = Tcl_GetStringResult(interp);
          pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಶೂನ್ಯ tclFunction(sqlite3_context *pCtx, ಪೂರ್ಣಾಙ್ಕ nArg, sqlite3_ಮೌಲ್ಯ **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_data(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  ಪೂರ್ಣಾಙ್ಕ ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  ಪ್ರತಿ(ii=0; ii<nArg; ii++){
    ಧ್ರುವ ವರ್ಣ *zArg = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pScript);

  pRet = Tcl_GetObjResult(interp);
  sqlite3_result_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclFindFunction(
  sqlite3_vtab *tab, 
  ಪೂರ್ಣಾಙ್ಕ nArg, 
  ಧ್ರುವ ವರ್ಣ *zName,
  ಶೂನ್ಯ (**pxFunc)(sqlite3_context*,ಪೂರ್ಣಾಙ್ಕ,sqlite3_ಮೌಲ್ಯ**),   /* OUT */
  ಶೂನ್ಯ **ppArg                                             /* OUT */
){
  ಪೂರ್ಣಾಙ್ಕ iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pScript);

  ಯದಿ( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_GetObjResult(interp);

    ಯದಿ( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }ಅನ್ಯಥಾ ಯದಿ( iRet>0 ){
      sqlite3_int64 nName = strlen(zName);
      sqlite3_int64 nByte = nName + 1 + ಪರಿಮಾಣ(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      ಯದಿ( pNew==0 ){
        iRet = 0;
      }ಅನ್ಯಥಾ{
        memset(pNew, 0, nByte);
        pNew->zName = (ಧ್ರುವ ವರ್ಣ*)&pNew[1];
        memcpy((ವರ್ಣ*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->pNext = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (ಶೂನ್ಯ*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ iRet;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclUpdate(
  sqlite3_vtab *tab, 
  ಪೂರ್ಣಾಙ್ಕ nArg, 
  sqlite3_ಮೌಲ್ಯ **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  ಪೂರ್ಣಾಙ್ಕ rc = TCL_OK;

  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);

  ಯದಿ( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  ಯದಿ( rc!=TCL_OK ){
    tab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", Tcl_GetStringResult(pTab->interp));
    ಪ್ರತ್ಯಾಗಮ rc;
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ಸ್ಥಿರ sqlite3_module tclModule = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tclBestIndex,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tclNext,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tclColumn,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tclFindFunction,             /* xFindFunction */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};
ಸ್ಥಿರ sqlite3_module tclModuleUpdate = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tclBestIndex,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tclNext,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tclColumn,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  tclUpdate,                   /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tclFindFunction,             /* xFindFunction */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};

/*
** Decode a pointer to an sqlite3 object.
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);

ಸ್ಥಿರ ಶೂನ್ಯ delTestVtabCtx(ಶೂನ್ಯ *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  ಯದಿ( pCtx->pDefault ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI register_tcl_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಯದಿ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB ?DEFAULT-CMD?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(ಪರಿಮಾಣ(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    ಯದಿ( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCtx->pDefault);
    }

    ಯದಿ( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (ಶೂನ್ಯ*)pCtx, delTestVtabCtx);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *clientData;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
