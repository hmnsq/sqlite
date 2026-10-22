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


#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା tcl_vtab tcl_vtab;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା tcl_cursor tcl_cursor;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestFindFunction TestFindFunction;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
ସଂରଚନା tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
ସଂରଚନା tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

ସଂରଚନା TestFindFunction {
  tcl_vtab *pTab;
  ଅଚଳ ଅକ୍ଷର *zName;
  TestFindFunction *pNext;
};

ସଂରଚନା TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
ସ୍ଥିର ଶୂନ୍ୟ tclDequote(ଅକ୍ଷର *z){
  ଅକ୍ଷର q = z[0];

  /* Set stack variable q to the close-quote character */
  ଯଦି( q=='[' || q=='\'' || q=='"' || q=='`' ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIn = 1;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOut = 0;
    ଯଦି( q=='[' ) q = ']';  

    ଯେତେବେଳେ( ALWAYS(z[iIn]) ){
      ଯଦି( z[iIn]==q ){
        ଯଦି( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }ଅନ୍ୟଥା{
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  ଅକ୍ଷର *zCmd = 0;
  Tcl_Obj *pScript = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ଯଦି( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  ଯଦି( argc==4 ){
    zCmd = sqlite3_malloc64(strlen(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(ଆକାର(tcl_vtab));
  ଯଦି( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, ଆକାର(tcl_vtab));

    ଯଦି( zCmd ){
      memcpy(zCmd, argv[3], strlen(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }ଅନ୍ୟଥା{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_ଗଣନା(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_ଗଣନା(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    ଯଦି( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_GetStringResult(interp));
      ଯଦି( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }ଅନ୍ୟଥା{
        rc = SQLITE_ERROR;
      }
    }ଅନ୍ୟଥା{
      rc = sqlite3_declare_vtab(db, Tcl_GetStringResult(interp));
      ଯଦି( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    ଯଦି( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }ଅନ୍ୟଥା{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  ଫେରନ୍ତୁ rc;
}

/* The xDisconnect and xDestroy methods are also the same */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  ଯେତେବେଳେ( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->pNext;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_ଗଣନା(pTab->pCmd);
  sqlite3_free(pTab);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(ଆକାର(tcl_cursor));
  ଯଦି( pCur==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(pCur, 0, ଆକାର(tcl_cursor));
  *ppCursor = &pCur->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  ଯଦି( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclNext(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ଯଦି( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_step(pCsr->pStmt);
    ଯଦି( rc!=SQLITE_ROW ){
      ଅଚଳ ଅକ୍ଷର *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      ଯଦି( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ଗଣନା(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_ଅଙ୍କ));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_ଗଣନା(pArg);
  ପାଇଁ(ii=0; ii<argc; ii++){
    ଅଚଳ ଅକ୍ଷର *zVal = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[ii]);
    Tcl_Obj *pVal;
    ଯଦି( zVal==0 ){
      sqlite3_ମୂଲ୍ୟ *pMem;
      pVal = Tcl_NewObj();
      ପାଇଁ(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_next(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pMem);
        ଯଦି( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }ଅନ୍ୟଥା{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }ଅନ୍ୟଥା{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_ଗଣନା(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  ଯଦି( rc!=TCL_OK ){
    ଅଚଳ ଅକ୍ଷର *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", zErr);
  }ଅନ୍ୟଥା{
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
    ଯଦି( rc!=TCL_OK ){
      ଅଚଳ ଅକ୍ଷର *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", zErr);
    }ଅନ୍ୟଥା{
      ପାଇଁ(ii=0; rc==SQLITE_OK && ii<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nElem; ii+=2){
        ଅଚଳ ଅକ୍ଷର *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ଯଦି( sqlite3_stricmp("sql", zCmd)==0 ){
          ଅଚଳ ଅକ୍ଷର *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          ଯଦି( rc!=SQLITE_OK ){
            ଅଚଳ ଅକ୍ଷର *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }ଅନ୍ୟଥା{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  ଯଦି( rc==SQLITE_OK ){
    rc = tclNext(pVtabCursor);
  }
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_result_ମୂଲ୍ୟ(ctx, sqlite3_column_ମୂଲ୍ୟ(pCsr->pStmt, i+1));
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ଫେରନ୍ତୁ (pCsr->pStmt==0);
}

ସ୍ଥିର ଶୂନ୍ୟ testBestIndexObjConstraints(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ଗଣନା(pRes);
  ପାଇଁ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ସଂରଚନା sqlite3_index_constraint ଅଚଳ *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    ଅଚଳ ଅକ୍ଷର *zOp = 0;

    Tcl_Incr_Ref_ଗଣନା(pElem);

    ବାଛ( pCons->op ){
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_EQ:
        zOp = "eq"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_GT:
        zOp = "gt"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_LE:
        zOp = "le"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_LT:
        zOp = "lt"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_GE:
        zOp = "ge"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_MATCH:
        zOp = "match"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_LIKE:
        zOp = "like"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_GLOB:
        zOp = "glob"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_REGEXP:
        zOp = "regexp"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_NE:
        zOp = "ne"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_ISNOT:
        zOp = "isnot"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_ISNULL:
        zOp = "isnull"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_IS:
        zOp = "is"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_LIMIT:
        zOp = "limit"; ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_OFFSET:
        zOp = "offset"; ଭାଙ୍ଗ;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    ଯଦି( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }ଅନ୍ୟଥା{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ଗଣନା(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_ଗଣନା(pRes);
}

ସ୍ଥିର ଶୂନ୍ୟ testBestIndexObjOrderby(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ଗଣନା(pRes);
  ପାଇଁ(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    ସଂରଚନା sqlite3_index_orderby ଅଚଳ *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_ଗଣନା(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ଗଣନା(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_ଗଣନା(pRes);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI testBestIndexObj(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ଅଚଳ ଅକ୍ଷର *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  sqlite3_index_info *pIdxInfo = (sqlite3_index_info*)clientData;

  ଯଦି( objc<2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SUB-COMMAND");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIndexFromObj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( ii<4 && objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( ii==4 && objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "INDEX BOOLEAN");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "INDEX ?DEFAULT?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ବାଛ( ii ){
    ସ୍ଥିତି 0: ପ୍ରତିଜ୍ଞା( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      testBestIndexObjConstraints(interp, pIdxInfo);
      ଭାଙ୍ଗ;

    ସ୍ଥିତି 1: ପ୍ରତିଜ୍ଞା( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      testBestIndexObjOrderby(interp, pIdxInfo);
      ଭାଙ୍ଗ;

    ସ୍ଥିତି 2: ପ୍ରତିଜ୍ଞା( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_SetObjResult(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      ଭାଙ୍ଗ;

    ସ୍ଥିତି 3: ପ୍ରତିଜ୍ଞା( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_SetObjResult(interp, Tcl_NewIntObj(bDistinct));
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି 4: ପ୍ରତିଜ୍ଞା( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCons;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bHandle;
      ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      Tcl_SetObjResult(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି 5: ପ୍ରତିଜ୍ଞା( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCons = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
      sqlite3_ମୂଲ୍ୟ *pVal = 0;
      ଅଚଳ ଅକ୍ଷର *zVal = "";
      ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_ମୂଲ୍ୟ(pIdxInfo, iCons, &pVal);
      ଯଦି( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( pVal ){
        zVal = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(pVal);
      }ଅନ୍ୟଥା ଯଦି( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zVal, -1));
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି 6: ପ୍ରତିଜ୍ଞା( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCons = 0;
      ଅଚଳ ଅକ୍ଷର *zColl = "";
      ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zColl, -1));
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNext = 43;
  ଅକ୍ଷର zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ଗଣନା(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(ଆକାର(zHdl), zHdl, "bestindex%d", iNext++);
  Tcl_CreateObjCommand(interp, zHdl, testBestIndexObj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_ଗଣନା(pScript);

  ଯଦି( rc!=TCL_OK ){
    ଅଚଳ ଅକ୍ଷର *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", zErr);
  }ଅନ୍ୟଥା{
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
    ଯଦି( rc!=TCL_OK ){
      ଅଚଳ ଅକ୍ଷର *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", zErr);
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iArgv = 1;
      ପାଇଁ(ii=0; rc==SQLITE_OK && ii<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nElem; ii+=2){
        ଅଚଳ ଅକ୍ଷର *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ଯଦି( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }ଅନ୍ୟଥା
        ଯଦି( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }ଅନ୍ୟଥା
        ଯଦି( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_ଅଙ୍କ);
        }ଅନ୍ୟଥା
        ଯଦି( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }ଅନ୍ୟଥା
        ଯଦି( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }ଅନ୍ୟଥା
        ଯଦି( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          ଯଦି( rc==SQLITE_OK ){
            ଯଦି( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("unexpected: %d", iCons);
            }ଅନ୍ୟଥା{
              ପୂର୍ଣ୍ଣସଂଖ୍ୟା bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argvIndex = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }ଅନ୍ୟଥା
        ଯଦି( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", Tcl_GetString(p));
        }ଅନ୍ୟଥା{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        ଯଦି( rc!=SQLITE_OK && pTab->base.z_Err_ସନ୍ଦେଶ==0 ){
          ଅଚଳ ଅକ୍ଷର *zErr = Tcl_GetStringResult(interp);
          pTab->base.z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ଶୂନ୍ୟ tclFunction(sqlite3_context *pCtx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg, sqlite3_ମୂଲ୍ୟ **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_data(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_ଗଣନା(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  ପାଇଁ(ii=0; ii<nArg; ii++){
    ଅଚଳ ଅକ୍ଷର *zArg = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ଗଣନା(pScript);

  pRet = Tcl_GetObjResult(interp);
  sqlite3_result_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclFindFunction(
  sqlite3_vtab *tab, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg, 
  ଅଚଳ ଅକ୍ଷର *zName,
  ଶୂନ୍ୟ (**pxFunc)(sqlite3_context*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,sqlite3_ମୂଲ୍ୟ**),   /* OUT */
  ଶୂନ୍ୟ **ppArg                                             /* OUT */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ଗଣନା(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ଗଣନା(pScript);

  ଯଦି( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_GetObjResult(interp);

    ଯଦି( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }ଅନ୍ୟଥା ଯଦି( iRet>0 ){
      sqlite3_int64 nName = strlen(zName);
      sqlite3_int64 nByte = nName + 1 + ଆକାର(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      ଯଦି( pNew==0 ){
        iRet = 0;
      }ଅନ୍ୟଥା{
        memset(pNew, 0, nByte);
        pNew->zName = (ଅଚଳ ଅକ୍ଷର*)&pNew[1];
        memcpy((ଅକ୍ଷର*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->pNext = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (ଶୂନ୍ୟ*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  ଫେରନ୍ତୁ iRet;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclUpdate(
  sqlite3_vtab *tab, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg, 
  sqlite3_ମୂଲ୍ୟ **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = TCL_OK;

  Tcl_Incr_Ref_ଗଣନା(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ଗଣନା(pEval);

  ଯଦି( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  ଯଦି( rc!=TCL_OK ){
    tab->z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", Tcl_GetStringResult(pTab->interp));
    ଫେରନ୍ତୁ rc;
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ସ୍ଥିର sqlite3_module tclModule = {
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
ସ୍ଥିର sqlite3_module tclModuleUpdate = {
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
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);

ସ୍ଥିର ଶୂନ୍ୟ delTestVtabCtx(ଶୂନ୍ୟ *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  ଯଦି( pCtx->pDefault ){
    Tcl_Decr_Ref_ଗଣନା(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI register_tcl_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ଯଦି( objc!=2 && objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB ?DEFAULT-CMD?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(ଆକାର(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    ଯଦି( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_ଗଣନା(pCtx->pDefault);
    }

    ଯଦି( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (ଶୂନ୍ୟ*)pCtx, delTestVtabCtx);
  }
#endif
  ଫେରନ୍ତୁ TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
     ଶୂନ୍ୟ *clientData;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ଫେରନ୍ତୁ TCL_OK;
}
