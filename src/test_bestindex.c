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


#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE


തരനിർവചനം ഘടന tcl_vtab tcl_vtab;
തരനിർവചനം ഘടന tcl_cursor tcl_cursor;
തരനിർവചനം ഘടന TestFindFunction TestFindFunction;
തരനിർവചനം ഘടന TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
ഘടന tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
ഘടന tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

ഘടന TestFindFunction {
  tcl_vtab *pTab;
  സ്ഥിരം അക്ഷരം *zName;
  TestFindFunction *pNext;
};

ഘടന TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
സ്ഥിതം ശൂന്യം tclDequote(അക്ഷരം *z){
  അക്ഷരം q = z[0];

  /* Set stack variable q to the close-quote character */
  എങ്കിൽ( q=='[' || q=='\'' || q=='"' || q=='`' ){
    പൂർണ്ണസംഖ്യ iIn = 1;
    പൂർണ്ണസംഖ്യ iOut = 0;
    എങ്കിൽ( q=='[' ) q = ']';  

    ആകുമ്പോൾ( ALWAYS(z[iIn]) ){
      എങ്കിൽ( z[iIn]==q ){
        എങ്കിൽ( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          നിർത്തുക;
        }അല്ലെങ്കിൽ{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }അല്ലെങ്കിൽ{
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
സ്ഥിതം പൂർണ്ണസംഖ്യ tclConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  അക്ഷരം *zCmd = 0;
  Tcl_Obj *pScript = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  എങ്കിൽ( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    തിരികെ SQLITE_ERROR;
  }

  എങ്കിൽ( argc==4 ){
    zCmd = sqlite3_malloc64(strlen(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(വലുപ്പം(tcl_vtab));
  എങ്കിൽ( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, വലുപ്പം(tcl_vtab));

    എങ്കിൽ( zCmd ){
      memcpy(zCmd, argv[3], strlen(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }അല്ലെങ്കിൽ{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_എണ്ണം(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_എണ്ണം(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    എങ്കിൽ( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_GetStringResult(interp));
      എങ്കിൽ( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }അല്ലെങ്കിൽ{
        rc = SQLITE_ERROR;
      }
    }അല്ലെങ്കിൽ{
      rc = sqlite3_declare_vtab(db, Tcl_GetStringResult(interp));
      എങ്കിൽ( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    എങ്കിൽ( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }അല്ലെങ്കിൽ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  തിരികെ rc;
}

/* The xDisconnect and xDestroy methods are also the same */
സ്ഥിതം പൂർണ്ണസംഖ്യ tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  ആകുമ്പോൾ( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->pNext;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_എണ്ണം(pTab->pCmd);
  sqlite3_free(pTab);
  തിരികെ SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(വലുപ്പം(tcl_cursor));
  എങ്കിൽ( pCur==0 ) തിരികെ SQLITE_NOMEM;
  memset(pCur, 0, വലുപ്പം(tcl_cursor));
  *ppCursor = &pCur->base;
  തിരികെ SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  എങ്കിൽ( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclNext(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  എങ്കിൽ( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    പൂർണ്ണസംഖ്യ rc = sqlite3_step(pCsr->pStmt);
    എങ്കിൽ( rc!=SQLITE_ROW ){
      സ്ഥിരം അക്ഷരം *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      എങ്കിൽ( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  പൂർണ്ണസംഖ്യ ii;
  പൂർണ്ണസംഖ്യ rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_എണ്ണം(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_അക്കം));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_എണ്ണം(pArg);
  വേണ്ടി(ii=0; ii<argc; ii++){
    സ്ഥിരം അക്ഷരം *zVal = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[ii]);
    Tcl_Obj *pVal;
    എങ്കിൽ( zVal==0 ){
      sqlite3_മൂല്യം *pMem;
      pVal = Tcl_NewObj();
      വേണ്ടി(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_next(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(pMem);
        എങ്കിൽ( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }അല്ലെങ്കിൽ{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }അല്ലെങ്കിൽ{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_എണ്ണം(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  എങ്കിൽ( rc!=TCL_OK ){
    സ്ഥിരം അക്ഷരം *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("%s", zErr);
  }അല്ലെങ്കിൽ{
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
    എങ്കിൽ( rc!=TCL_OK ){
      സ്ഥിരം അക്ഷരം *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("%s", zErr);
    }അല്ലെങ്കിൽ{
      വേണ്ടി(ii=0; rc==SQLITE_OK && ii<(പൂർണ്ണസംഖ്യ)nElem; ii+=2){
        സ്ഥിരം അക്ഷരം *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        എങ്കിൽ( sqlite3_stricmp("sql", zCmd)==0 ){
          സ്ഥിരം അക്ഷരം *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          എങ്കിൽ( rc!=SQLITE_OK ){
            സ്ഥിരം അക്ഷരം *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }അല്ലെങ്കിൽ{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    rc = tclNext(pVtabCursor);
  }
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  പൂർണ്ണസംഖ്യ i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_result_മൂല്യം(ctx, sqlite3_column_മൂല്യം(pCsr->pStmt, i+1));
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  തിരികെ (pCsr->pStmt==0);
}

സ്ഥിതം ശൂന്യം testBestIndexObjConstraints(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  പൂർണ്ണസംഖ്യ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_എണ്ണം(pRes);
  വേണ്ടി(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ഘടന sqlite3_index_constraint സ്ഥിരം *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    സ്ഥിരം അക്ഷരം *zOp = 0;

    Tcl_Incr_Ref_എണ്ണം(pElem);

    തിരഞ്ഞെടുപ്പ്( pCons->op ){
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_EQ:
        zOp = "eq"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_GT:
        zOp = "gt"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_LE:
        zOp = "le"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_LT:
        zOp = "lt"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_GE:
        zOp = "ge"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_MATCH:
        zOp = "match"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_LIKE:
        zOp = "like"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_GLOB:
        zOp = "glob"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_REGEXP:
        zOp = "regexp"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_NE:
        zOp = "ne"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_ISNOT:
        zOp = "isnot"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_ISNULL:
        zOp = "isnull"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_IS:
        zOp = "is"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_LIMIT:
        zOp = "limit"; നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_OFFSET:
        zOp = "offset"; നിർത്തുക;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    എങ്കിൽ( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }അല്ലെങ്കിൽ{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_എണ്ണം(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_എണ്ണം(pRes);
}

സ്ഥിതം ശൂന്യം testBestIndexObjOrderby(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  പൂർണ്ണസംഖ്യ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_എണ്ണം(pRes);
  വേണ്ടി(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    ഘടന sqlite3_index_orderby സ്ഥിരം *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_എണ്ണം(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_എണ്ണം(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_എണ്ണം(pRes);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI testBestIndexObj(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  സ്ഥിരം അക്ഷരം *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  പൂർണ്ണസംഖ്യ ii;
  sqlite3_index_info *pIdxInfo = (sqlite3_index_info*)clientData;

  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SUB-COMMAND");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIndexFromObj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( ii<4 && objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( ii==4 && objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "INDEX BOOLEAN");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "INDEX ?DEFAULT?");
    തിരികെ TCL_ERROR;
  }

  തിരഞ്ഞെടുപ്പ്( ii ){
    അവസ്ഥ 0: പ്രതിജ്ഞാ( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      testBestIndexObjConstraints(interp, pIdxInfo);
      നിർത്തുക;

    അവസ്ഥ 1: പ്രതിജ്ഞാ( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      testBestIndexObjOrderby(interp, pIdxInfo);
      നിർത്തുക;

    അവസ്ഥ 2: പ്രതിജ്ഞാ( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_SetObjResult(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      നിർത്തുക;

    അവസ്ഥ 3: പ്രതിജ്ഞാ( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      പൂർണ്ണസംഖ്യ bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_SetObjResult(interp, Tcl_NewIntObj(bDistinct));
      നിർത്തുക;
    }

    അവസ്ഥ 4: പ്രതിജ്ഞാ( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      പൂർണ്ണസംഖ്യ iCons;
      പൂർണ്ണസംഖ്യ bHandle;
      എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        തിരികെ TCL_ERROR;
      }
      Tcl_SetObjResult(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      നിർത്തുക;
    }

    അവസ്ഥ 5: പ്രതിജ്ഞാ( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      പൂർണ്ണസംഖ്യ iCons = 0;
      പൂർണ്ണസംഖ്യ rc;
      sqlite3_മൂല്യം *pVal = 0;
      സ്ഥിരം അക്ഷരം *zVal = "";
      എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        തിരികെ TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_മൂല്യം(pIdxInfo, iCons, &pVal);
      എങ്കിൽ( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( pVal ){
        zVal = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(pVal);
      }അല്ലെങ്കിൽ എങ്കിൽ( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zVal, -1));
      നിർത്തുക;
    }

    അവസ്ഥ 6: പ്രതിജ്ഞാ( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      പൂർണ്ണസംഖ്യ iCons = 0;
      സ്ഥിരം അക്ഷരം *zColl = "";
      എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        തിരികെ TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zColl, -1));
      നിർത്തുക;
    }
  }

  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  സ്ഥിതം പൂർണ്ണസംഖ്യ iNext = 43;
  അക്ഷരം zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_എണ്ണം(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(വലുപ്പം(zHdl), zHdl, "bestindex%d", iNext++);
  Tcl_CreateObjCommand(interp, zHdl, testBestIndexObj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_എണ്ണം(pScript);

  എങ്കിൽ( rc!=TCL_OK ){
    സ്ഥിരം അക്ഷരം *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("%s", zErr);
  }അല്ലെങ്കിൽ{
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
    എങ്കിൽ( rc!=TCL_OK ){
      സ്ഥിരം അക്ഷരം *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("%s", zErr);
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ ii;
      പൂർണ്ണസംഖ്യ iArgv = 1;
      വേണ്ടി(ii=0; rc==SQLITE_OK && ii<(പൂർണ്ണസംഖ്യ)nElem; ii+=2){
        സ്ഥിരം അക്ഷരം *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        എങ്കിൽ( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }അല്ലെങ്കിൽ
        എങ്കിൽ( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }അല്ലെങ്കിൽ
        എങ്കിൽ( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_അക്കം);
        }അല്ലെങ്കിൽ
        എങ്കിൽ( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }അല്ലെങ്കിൽ
        എങ്കിൽ( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }അല്ലെങ്കിൽ
        എങ്കിൽ( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          പൂർണ്ണസംഖ്യ iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          എങ്കിൽ( rc==SQLITE_OK ){
            എങ്കിൽ( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("unexpected: %d", iCons);
            }അല്ലെങ്കിൽ{
              പൂർണ്ണസംഖ്യ bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argvIndex = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }അല്ലെങ്കിൽ
        എങ്കിൽ( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("%s", Tcl_GetString(p));
        }അല്ലെങ്കിൽ{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        എങ്കിൽ( rc!=SQLITE_OK && pTab->base.z_Err_സന്ദേശം==0 ){
          സ്ഥിരം അക്ഷരം *zErr = Tcl_GetStringResult(interp);
          pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  തിരികെ rc;
}

സ്ഥിതം ശൂന്യം tclFunction(sqlite3_context *pCtx, പൂർണ്ണസംഖ്യ nArg, sqlite3_മൂല്യം **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_data(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  പൂർണ്ണസംഖ്യ ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_എണ്ണം(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  വേണ്ടി(ii=0; ii<nArg; ii++){
    സ്ഥിരം അക്ഷരം *zArg = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_എണ്ണം(pScript);

  pRet = Tcl_GetObjResult(interp);
  sqlite3_result_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclFindFunction(
  sqlite3_vtab *tab, 
  പൂർണ്ണസംഖ്യ nArg, 
  സ്ഥിരം അക്ഷരം *zName,
  ശൂന്യം (**pxFunc)(sqlite3_context*,പൂർണ്ണസംഖ്യ,sqlite3_മൂല്യം**),   /* OUT */
  ശൂന്യം **ppArg                                             /* OUT */
){
  പൂർണ്ണസംഖ്യ iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_എണ്ണം(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_എണ്ണം(pScript);

  എങ്കിൽ( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_GetObjResult(interp);

    എങ്കിൽ( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }അല്ലെങ്കിൽ എങ്കിൽ( iRet>0 ){
      sqlite3_int64 nName = strlen(zName);
      sqlite3_int64 nByte = nName + 1 + വലുപ്പം(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      എങ്കിൽ( pNew==0 ){
        iRet = 0;
      }അല്ലെങ്കിൽ{
        memset(pNew, 0, nByte);
        pNew->zName = (സ്ഥിരം അക്ഷരം*)&pNew[1];
        memcpy((അക്ഷരം*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->pNext = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (ശൂന്യം*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  തിരികെ iRet;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclUpdate(
  sqlite3_vtab *tab, 
  പൂർണ്ണസംഖ്യ nArg, 
  sqlite3_മൂല്യം **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  പൂർണ്ണസംഖ്യ rc = TCL_OK;

  Tcl_Incr_Ref_എണ്ണം(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_എണ്ണം(pEval);

  എങ്കിൽ( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  എങ്കിൽ( rc!=TCL_OK ){
    tab->z_Err_സന്ദേശം = sqlite3_mprintf("%s", Tcl_GetStringResult(pTab->interp));
    തിരികെ rc;
  }

  തിരികെ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
സ്ഥിതം sqlite3_module tclModule = {
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
സ്ഥിതം sqlite3_module tclModuleUpdate = {
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
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);

സ്ഥിതം ശൂന്യം delTestVtabCtx(ശൂന്യം *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  എങ്കിൽ( pCtx->pDefault ){
    Tcl_Decr_Ref_എണ്ണം(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI register_tcl_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  എങ്കിൽ( objc!=2 && objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB ?DEFAULT-CMD?");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(വലുപ്പം(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    എങ്കിൽ( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_എണ്ണം(pCtx->pDefault);
    }

    എങ്കിൽ( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (ശൂന്യം*)pCtx, delTestVtabCtx);
  }
#endif
  തിരികെ TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     ശൂന്യം *clientData;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  തിരികെ TCL_OK;
}
