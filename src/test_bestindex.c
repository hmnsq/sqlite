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


#include "sqliteInt.h"
#include "tclsqlite.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE


typedef sa_nracanaa tcl_vtab tcl_vtab;
typedef sa_nracanaa tcl_cursor tcl_cursor;
typedef sa_nracanaa TestFindFunction TestFindFunction;
typedef sa_nracanaa TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
sa_nracanaa tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
sa_nracanaa tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

sa_nracanaa TestFindFunction {
  tcl_vtab *pTab;
  const var_na *zName;
  TestFindFunction *pNext;
};

sa_nracanaa TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
static void tclDequote(var_na *z){
  var_na q = z[0];

  /* Set stack variable q to the close-quote character */
  ប្រសិនបើ( q=='[' || q=='\'' || q=='"' || q=='`' ){
    ចំនួនគត់ iIn = 1;
    ចំនួនគត់ iOut = 0;
    ប្រសិនបើ( q=='[' ) q = ']';  

    ខណៈពេល( ALWAYS(z[iIn]) ){
      ប្រសិនបើ( z[iIn]==q ){
        ប្រសិនបើ( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          bha_nga;
        }បើមិនដូច្នេះ{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }បើមិនដូច្នេះ{
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
static ចំនួនគត់ tclConnect(
  sqlite3 *db,
  void *pAux,
  ចំនួនគត់ argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  var_na *zCmd = 0;
  Tcl_Obj *pScript = 0;
  ចំនួនគត់ rc = SQLITE_OK;

  ប្រសិនបើ( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ត្រឡប់ SQLITE_ERROR;
  }

  ប្រសិនបើ( argc==4 ){
    zCmd = sqlite3_malloc64(strlen(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(parimaa_na(tcl_vtab));
  ប្រសិនបើ( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, parimaa_na(tcl_vtab));

    ប្រសិនបើ( zCmd ){
      memcpy(zCmd, argv[3], strlen(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }បើមិនដូច្នេះ{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_IncrRefCount(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_IncrRefCount(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    ប្រសិនបើ( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_GetStringResult(interp));
      ប្រសិនបើ( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }បើមិនដូច្នេះ{
        rc = SQLITE_ERROR;
      }
    }បើមិនដូច្នេះ{
      rc = sqlite3_declare_vtab(db, Tcl_GetStringResult(interp));
      ប្រសិនបើ( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    ប្រសិនបើ( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }បើមិនដូច្នេះ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  ត្រឡប់ rc;
}

/* The xDisconnect and xDestroy methods are also the same */
static ចំនួនគត់ tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  ខណៈពេល( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->pNext;
    sqlite3_free(p);
  }
  Tcl_DecrRefCount(pTab->pCmd);
  sqlite3_free(pTab);
  ត្រឡប់ SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
static ចំនួនគត់ tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(parimaa_na(tcl_cursor));
  ប្រសិនបើ( pCur==0 ) ត្រឡប់ SQLITE_NOMEM;
  memset(pCur, 0, parimaa_na(tcl_cursor));
  *ppCursor = &pCur->base;
  ត្រឡប់ SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
static ចំនួនគត់ tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  ប្រសិនបើ( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  ត្រឡប់ SQLITE_OK;
}

static ចំនួនគត់ tclNext(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ប្រសិនបើ( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    ចំនួនគត់ rc = sqlite3_step(pCsr->pStmt);
    ប្រសិនបើ( rc!=SQLITE_ROW ){
      const var_na *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      ប្រសិនបើ( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.zErrMsg = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  ត្រឡប់ SQLITE_OK;
}

static ចំនួនគត់ tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ចំនួនគត់ idxNum, const var_na *idxStr,
  ចំនួនគត់ argc, sqlite3_value **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  ចំនួនគត់ ii;
  ចំនួនគត់ rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_IncrRefCount(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idxNum));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_IncrRefCount(pArg);
  សម្រាប់(ii=0; ii<argc; ii++){
    const var_na *zVal = (const var_na*)sqlite3_value_text(argv[ii]);
    Tcl_Obj *pVal;
    ប្រសិនបើ( zVal==0 ){
      sqlite3_value *pMem;
      pVal = Tcl_NewObj();
      សម្រាប់(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_next(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (const var_na*)sqlite3_value_text(pMem);
        ប្រសិនបើ( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }បើមិនដូច្នេះ{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }បើមិនដូច្នេះ{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_DecrRefCount(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  ប្រសិនបើ( rc!=TCL_OK ){
    const var_na *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.zErrMsg = sqlite3_mprintf("%s", zErr);
  }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( rc!=TCL_OK ){
      const var_na *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.zErrMsg = sqlite3_mprintf("%s", zErr);
    }បើមិនដូច្នេះ{
      សម្រាប់(ii=0; rc==SQLITE_OK && ii<(ចំនួនគត់)nElem; ii+=2){
        const var_na *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ប្រសិនបើ( sqlite3_stricmp("sql", zCmd)==0 ){
          const var_na *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          ប្រសិនបើ( rc!=SQLITE_OK ){
            const var_na *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.zErrMsg = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }បើមិនដូច្នេះ{
          rc = SQLITE_ERROR;
          pTab->base.zErrMsg = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  ប្រសិនបើ( rc==SQLITE_OK ){
    rc = tclNext(pVtabCursor);
  }
  ត្រឡប់ rc;
}

static ចំនួនគត់ tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  ចំនួនគត់ i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_result_value(ctx, sqlite3_column_value(pCsr->pStmt, i+1));
  ត្រឡប់ SQLITE_OK;
}

static ចំនួនគត់ tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  ត្រឡប់ SQLITE_OK;
}

static ចំនួនគត់ tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ត្រឡប់ (pCsr->pStmt==0);
}

static void testBestIndexObjConstraints(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  ចំនួនគត់ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_IncrRefCount(pRes);
  សម្រាប់(ii=0; ii<pIdxInfo->nConstraint; ii++){
    sa_nracanaa sqlite3_index_constraint const *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    const var_na *zOp = 0;

    Tcl_IncrRefCount(pElem);

    switch( pCons->op ){
      pak_sa SQLITE_INDEX_CONSTRAINT_EQ:
        zOp = "eq"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_GT:
        zOp = "gt"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_LE:
        zOp = "le"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_LT:
        zOp = "lt"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_GE:
        zOp = "ge"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_MATCH:
        zOp = "match"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_LIKE:
        zOp = "like"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_GLOB:
        zOp = "glob"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_REGEXP:
        zOp = "regexp"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_NE:
        zOp = "ne"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_ISNOT:
        zOp = "isnot"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_ISNULL:
        zOp = "isnull"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_IS:
        zOp = "is"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_LIMIT:
        zOp = "limit"; bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_OFFSET:
        zOp = "offset"; bha_nga;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    ប្រសិនបើ( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }បើមិនដូច្នេះ{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_DecrRefCount(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_DecrRefCount(pRes);
}

static void testBestIndexObjOrderby(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  ចំនួនគត់ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_IncrRefCount(pRes);
  សម្រាប់(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    sa_nracanaa sqlite3_index_orderby const *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_IncrRefCount(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_DecrRefCount(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_DecrRefCount(pRes);
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
static ចំនួនគត់ SQLITE_TCLAPI testBestIndexObj(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ចំនួនគត់ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  const var_na *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  ចំនួនគត់ ii;
  sqlite3_index_info *pIdxInfo = (sqlite3_index_info*)clientData;

  ប្រសិនបើ( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_GetIndexFromObj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( ii<4 && objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( ii==4 && objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "INDEX BOOLEAN");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( ii==5 && objc!=3 && objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "INDEX ?DEFAULT?");
    ត្រឡប់ TCL_ERROR;
  }

  switch( ii ){
    pak_sa 0: assert( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      testBestIndexObjConstraints(interp, pIdxInfo);
      bha_nga;

    pak_sa 1: assert( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      testBestIndexObjOrderby(interp, pIdxInfo);
      bha_nga;

    pak_sa 2: assert( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_SetObjResult(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      bha_nga;

    pak_sa 3: assert( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      ចំនួនគត់ bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_SetObjResult(interp, Tcl_NewIntObj(bDistinct));
      bha_nga;
    }

    pak_sa 4: assert( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      ចំនួនគត់ iCons;
      ចំនួនគត់ bHandle;
      ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        ត្រឡប់ TCL_ERROR;
      }
      Tcl_SetObjResult(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      bha_nga;
    }

    pak_sa 5: assert( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      ចំនួនគត់ iCons = 0;
      ចំនួនគត់ rc;
      sqlite3_value *pVal = 0;
      const var_na *zVal = "";
      ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ត្រឡប់ TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_value(pIdxInfo, iCons, &pVal);
      ប្រសិនបើ( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
        ត្រឡប់ TCL_ERROR;
      }
      ប្រសិនបើ( pVal ){
        zVal = (const var_na*)sqlite3_value_text(pVal);
      }បើមិនដូច្នេះ ប្រសិនបើ( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zVal, -1));
      bha_nga;
    }

    pak_sa 6: assert( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      ចំនួនគត់ iCons = 0;
      const var_na *zColl = "";
      ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ត្រឡប់ TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zColl, -1));
      bha_nga;
    }
  }

  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ tclBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  ចំនួនគត់ rc = SQLITE_OK;

  static ចំនួនគត់ iNext = 43;
  var_na zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_IncrRefCount(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(parimaa_na(zHdl), zHdl, "bestindex%d", iNext++);
  Tcl_CreateObjCommand(interp, zHdl, testBestIndexObj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_DecrRefCount(pScript);

  ប្រសិនបើ( rc!=TCL_OK ){
    const var_na *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.zErrMsg = sqlite3_mprintf("%s", zErr);
  }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( rc!=TCL_OK ){
      const var_na *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.zErrMsg = sqlite3_mprintf("%s", zErr);
    }បើមិនដូច្នេះ{
      ចំនួនគត់ ii;
      ចំនួនគត់ iArgv = 1;
      សម្រាប់(ii=0; rc==SQLITE_OK && ii<(ចំនួនគត់)nElem; ii+=2){
        const var_na *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ប្រសិនបើ( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }បើមិនដូច្នេះ
        ប្រសិនបើ( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }បើមិនដូច្នេះ
        ប្រសិនបើ( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idxNum);
        }បើមិនដូច្នេះ
        ប្រសិនបើ( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }បើមិនដូច្នេះ
        ប្រសិនបើ( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }បើមិនដូច្នេះ
        ប្រសិនបើ( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          ចំនួនគត់ iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          ប្រសិនបើ( rc==SQLITE_OK ){
            ប្រសិនបើ( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.zErrMsg = sqlite3_mprintf("unexpected: %d", iCons);
            }បើមិនដូច្នេះ{
              ចំនួនគត់ bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argvIndex = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }បើមិនដូច្នេះ
        ប្រសិនបើ( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.zErrMsg = sqlite3_mprintf("%s", Tcl_GetString(p));
        }បើមិនដូច្នេះ{
          rc = SQLITE_ERROR;
          pTab->base.zErrMsg = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        ប្រសិនបើ( rc!=SQLITE_OK && pTab->base.zErrMsg==0 ){
          const var_na *zErr = Tcl_GetStringResult(interp);
          pTab->base.zErrMsg = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  ត្រឡប់ rc;
}

static void tclFunction(sqlite3_context *pCtx, ចំនួនគត់ nArg, sqlite3_value **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_data(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  ចំនួនគត់ ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_IncrRefCount(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  សម្រាប់(ii=0; ii<nArg; ii++){
    const var_na *zArg = (const var_na*)sqlite3_value_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DecrRefCount(pScript);

  pRet = Tcl_GetObjResult(interp);
  sqlite3_result_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

static ចំនួនគត់ tclFindFunction(
  sqlite3_vtab *tab, 
  ចំនួនគត់ nArg, 
  const var_na *zName,
  void (**pxFunc)(sqlite3_context*,ចំនួនគត់,sqlite3_value**),   /* OUT */
  void **ppArg                                             /* OUT */
){
  ចំនួនគត់ iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  ចំនួនគត់ rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_IncrRefCount(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DecrRefCount(pScript);

  ប្រសិនបើ( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_GetObjResult(interp);

    ប្រសិនបើ( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }បើមិនដូច្នេះ ប្រសិនបើ( iRet>0 ){
      sqlite3_int64 nName = strlen(zName);
      sqlite3_int64 nByte = nName + 1 + parimaa_na(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      ប្រសិនបើ( pNew==0 ){
        iRet = 0;
      }បើមិនដូច្នេះ{
        memset(pNew, 0, nByte);
        pNew->zName = (const var_na*)&pNew[1];
        memcpy((var_na*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->pNext = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (void*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  ត្រឡប់ iRet;
}

static ចំនួនគត់ tclUpdate(
  sqlite3_vtab *tab, 
  ចំនួនគត់ nArg, 
  sqlite3_value **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  ចំនួនគត់ rc = TCL_OK;

  Tcl_IncrRefCount(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_DecrRefCount(pEval);

  ប្រសិនបើ( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  ប្រសិនបើ( rc!=TCL_OK ){
    tab->zErrMsg = sqlite3_mprintf("%s", Tcl_GetStringResult(pTab->interp));
    ត្រឡប់ rc;
  }

  ត្រឡប់ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
static sqlite3_module tclModule = {
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
static sqlite3_module tclModuleUpdate = {
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
extern ចំនួនគត់ getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);

static void delTestVtabCtx(void *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  ប្រសិនបើ( pCtx->pDefault ){
    Tcl_DecrRefCount(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
static ចំនួនគត់ SQLITE_TCLAPI register_tcl_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ចំនួនគត់ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ប្រសិនបើ( objc!=2 && objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB ?DEFAULT-CMD?");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ត្រឡប់ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(parimaa_na(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    ប្រសិនបើ( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_IncrRefCount(pCtx->pDefault);
    }

    ប្រសិនបើ( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (void*)pCtx, delTestVtabCtx);
  }
#endif
  ត្រឡប់ TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
ចំនួនគត់ Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ត្រឡប់ TCL_OK;
}
