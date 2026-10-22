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


#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ tcl_vtab tcl_vtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ tcl_cursor tcl_cursor;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Test_ਲੱਭੋ_Function Test_ਲੱਭੋ_Function;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
ਬਣਤਰ tcl_vtab {
  sqlite3_vtab ਅਧਾਰ;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  Test_ਲੱਭੋ_Function *p_ਲੱਭੋ_Function_List;
  sqlite3 *db;
};

/* A tcl cursor object */
ਬਣਤਰ tcl_cursor {
  sqlite3_vtab_cursor ਅਧਾਰ;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

ਬਣਤਰ Test_ਲੱਭੋ_Function {
  tcl_vtab *pTab;
  ਅਚਲ ਅੱਖਰ *zName;
  Test_ਲੱਭੋ_Function *p_ਅਗਲਾ;
};

ਬਣਤਰ TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
ਸਥਿਰ ਖਾਲੀ tclDequote(ਅੱਖਰ *z){
  ਅੱਖਰ q = z[0];

  /* Set stack variable q to the close-quote character */
  ਜੇ( q=='[' || q=='\'' || q=='"' || q=='`' ){
    ਪੂਰਨ i_ਵਿੱਚ = 1;
    ਪੂਰਨ i_ਬਾਹਰਲਾ = 0;
    ਜੇ( q=='[' ) q = ']';  

    ਜਦੋਂ( ALWAYS(z[i_ਵਿੱਚ]) ){
      ਜੇ( z[i_ਵਿੱਚ]==q ){
        ਜੇ( z[i_ਵਿੱਚ+1]!=q ){
          /* Character iIn was the close quote. */
          i_ਵਿੱਚ++;
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          i_ਵਿੱਚ += 2;
          z[i_ਬਾਹਰਲਾ++] = q;
        }
      }ਨਹੀਂਤਾਂ{
        z[i_ਬਾਹਰਲਾ++] = z[i_ਵਿੱਚ++];
      }
    }

    z[i_ਬਾਹਰਲਾ] = '\0';
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
ਸਥਿਰ ਪੂਰਨ tclConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  ਅੱਖਰ *zCmd = 0;
  Tcl_Obj *pScript = 0;
  ਪੂਰਨ rc = SQLITE_OK;

  ਜੇ( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ਵਾਪਸ SQLITE_ERROR;
  }

  ਜੇ( argc==4 ){
    zCmd = sqlite3_malloc64(ਲੜੀ_ਲੰਬਾਈ(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(ਆਕਾਰ(tcl_vtab));
  ਜੇ( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, ਆਕਾਰ(tcl_vtab));

    ਜੇ( zCmd ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zCmd, argv[3], ਲੜੀ_ਲੰਬਾਈ(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }ਨਹੀਂਤਾਂ{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_ਗਿਣਤੀ(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    ਜੇ( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_Get_String_ਨਤੀਜਾ(interp));
      ਜੇ( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }ਨਹੀਂਤਾਂ{
        rc = SQLITE_ERROR;
      }
    }ਨਹੀਂਤਾਂ{
      rc = sqlite3_declare_vtab(db, Tcl_Get_String_ਨਤੀਜਾ(interp));
      ਜੇ( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    ਜੇ( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->ਅਧਾਰ : 0;
  ਵਾਪਸ rc;
}

/* The xDisconnect and xDestroy methods are also the same */
ਸਥਿਰ ਪੂਰਨ tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  ਜਦੋਂ( pTab->p_ਲੱਭੋ_Function_List ){
    Test_ਲੱਭੋ_Function *p = pTab->p_ਲੱਭੋ_Function_List;
    pTab->p_ਲੱਭੋ_Function_List = p->p_ਅਗਲਾ;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_ਗਿਣਤੀ(pTab->pCmd);
  sqlite3_free(pTab);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
ਸਥਿਰ ਪੂਰਨ tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(ਆਕਾਰ(tcl_cursor));
  ਜੇ( pCur==0 ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pCur, 0, ਆਕਾਰ(tcl_cursor));
  *ppCursor = &pCur->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
ਸਥਿਰ ਪੂਰਨ tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  ਜੇ( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ tcl_ਅਗਲਾ(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ਜੇ( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    ਪੂਰਨ rc = sqlite3_ਕਦਮ(pCsr->pStmt);
    ਜੇ( rc!=SQLITE_ਪੰਕਤੀ ){
      ਅਚਲ ਅੱਖਰ *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      ਜੇ( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  ਪੂਰਨ ii;
  ਪੂਰਨ rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_ਅੰਕ));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_ਗਿਣਤੀ(pArg);
  ਲਈ(ii=0; ii<argc; ii++){
    ਅਚਲ ਅੱਖਰ *zVal = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[ii]);
    Tcl_Obj *pVal;
    ਜੇ( zVal==0 ){
      sqlite3_ਮੁੱਲ *pMem;
      pVal = Tcl_NewObj();
      ਲਈ(rc=sqlite3_vtab_ਵਿੱਚ_ਪਹਿਲਾ(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_ਵਿੱਚ_ਅਗਲਾ(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pMem);
        ਜੇ( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }ਨਹੀਂਤਾਂ{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }ਨਹੀਂਤਾਂ{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  ਜੇ( rc!=TCL_OK ){
    ਅਚਲ ਅੱਖਰ *zErr = Tcl_Get_String_ਨਤੀਜਾ(interp);
    rc = SQLITE_ERROR;
    pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", zErr);
  }ਨਹੀਂਤਾਂ{
    /* Analyze the scripts return value. The return value should be a tcl 
    ** list object with an even number of elements. The first element of each
    ** pair must be one of:
    ** 
    **   "sql"          (SQL statement to return data)
    */
    Tcl_Obj *pRes = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
    Tcl_Obj **apElem = 0;
    Tcl_ਮਾਪ nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    ਜੇ( rc!=TCL_OK ){
      ਅਚਲ ਅੱਖਰ *zErr = Tcl_Get_String_ਨਤੀਜਾ(interp);
      rc = SQLITE_ERROR;
      pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", zErr);
    }ਨਹੀਂਤਾਂ{
      ਲਈ(ii=0; rc==SQLITE_OK && ii<(ਪੂਰਨ)nElem; ii+=2){
        ਅਚਲ ਅੱਖਰ *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ਜੇ( sqlite3_stricmp("sql", zCmd)==0 ){
          ਅਚਲ ਅੱਖਰ *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          ਜੇ( rc!=SQLITE_OK ){
            ਅਚਲ ਅੱਖਰ *zErr = sqlite3_errmsg(pTab->db);
            pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }ਨਹੀਂਤਾਂ{
          rc = SQLITE_ERROR;
          pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  ਜੇ( rc==SQLITE_OK ){
    rc = tcl_ਅਗਲਾ(pVtabCursor);
  }
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ tcl_ਕਾਲਮ(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  ਪੂਰਨ i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(ctx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pCsr->pStmt, i+1));
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_ਕਾਲਮ_int64(pCsr->pStmt, 0);
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ਵਾਪਸ (pCsr->pStmt==0);
}

ਸਥਿਰ ਖਾਲੀ test_ਉੱਤਮ_ਸੂਚਕਾਂਕ_Obj_Constraints(
  Tcl_Interp *interp, 
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo
){
  ਪੂਰਨ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ਗਿਣਤੀ(pRes);
  ਲਈ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint ਅਚਲ *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    ਅਚਲ ਅੱਖਰ *zOp = 0;

    Tcl_Incr_Ref_ਗਿਣਤੀ(pElem);

    ਚੋਣ( pCons->op ){
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ:
        zOp = "eq"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GT:
        zOp = "gt"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LE:
        zOp = "le"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LT:
        zOp = "lt"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GE:
        zOp = "ge"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਮੇਲ:
        zOp = "match"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LIKE:
        zOp = "like"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GLOB:
        zOp = "glob"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_REGEXP:
        zOp = "regexp"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_NE:
        zOp = "ne"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ISNOT:
        zOp = "isnot"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ISNULL:
        zOp = "isnull"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_IS:
        zOp = "is"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਹੱਦਬੰਦੀ:
        zOp = "limit"; ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਵਿੱਥ:
        zOp = "offset"; ਰੋਕੋ;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    ਜੇ( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }ਨਹੀਂਤਾਂ{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->i_ਕਾਲਮ));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ਗਿਣਤੀ(pElem);
  }

  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRes);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pRes);
}

ਸਥਿਰ ਖਾਲੀ test_ਉੱਤਮ_ਸੂਚਕਾਂਕ_Obj_Orderby(
  Tcl_Interp *interp, 
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo
){
  ਪੂਰਨ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ਗਿਣਤੀ(pRes);
  ਲਈ(ii=0; ii<pIdxInfo->n_ਕ੍ਰਮ_By; ii++){
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_orderby ਅਚਲ *p_ਕ੍ਰਮ = &pIdxInfo->a_ਕ੍ਰਮ_By[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_ਗਿਣਤੀ(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_ਕ੍ਰਮ->i_ਕਾਲਮ));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_ਕ੍ਰਮ->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ਗਿਣਤੀ(pElem);
  }

  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRes);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pRes);
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_ਉੱਤਮ_ਸੂਚਕਾਂਕ_Obj(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਅਚਲ ਅੱਖਰ *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  ਪੂਰਨ ii;
  sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo = (sqlite3_ਸੂਚਕਾਂਕ_info*)client_ਅੰਕੜੇ;

  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SUB-COMMAND");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( ii<4 && objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( ii==4 && objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "INDEX BOOLEAN");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "INDEX ?DEFAULT?");
    ਵਾਪਸ TCL_ERROR;
  }

  ਚੋਣ( ii ){
    ਹਾਲਤ 0: ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      test_ਉੱਤਮ_ਸੂਚਕਾਂਕ_Obj_Constraints(interp, pIdxInfo);
      ਰੋਕੋ;

    ਹਾਲਤ 1: ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      test_ਉੱਤਮ_ਸੂਚਕਾਂਕ_Obj_Orderby(interp, pIdxInfo);
      ਰੋਕੋ;

    ਹਾਲਤ 2: ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(pIdxInfo->ਖਾਨਾ_ਵਰਤਿਆ));
      ਰੋਕੋ;

    ਹਾਲਤ 3: ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      ਪੂਰਨ bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(bDistinct));
      ਰੋਕੋ;
    }

    ਹਾਲਤ 4: ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      ਪੂਰਨ iCons;
      ਪੂਰਨ bHandle;
      ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        ਵਾਪਸ TCL_ERROR;
      }
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, 
          Tcl_NewIntObj(sqlite3_vtab_ਵਿੱਚ(pIdxInfo, iCons, bHandle))
      );
      ਰੋਕੋ;
    }

    ਹਾਲਤ 5: ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      ਪੂਰਨ iCons = 0;
      ਪੂਰਨ rc;
      sqlite3_ਮੁੱਲ *pVal = 0;
      ਅਚਲ ਅੱਖਰ *zVal = "";
      ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ਵਾਪਸ TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_ਮੁੱਲ(pIdxInfo, iCons, &pVal);
      ਜੇ( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( pVal ){
        zVal = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(pVal);
      }ਨਹੀਂਤਾਂ ਜੇ( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(zVal, -1));
      ਰੋਕੋ;
    }

    ਹਾਲਤ 6: ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      ਪੂਰਨ iCons = 0;
      ਅਚਲ ਅੱਖਰ *zColl = "";
      ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ਵਾਪਸ TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(zColl, -1));
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ tcl_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  ਪੂਰਨ rc = SQLITE_OK;

  ਸਥਿਰ ਪੂਰਨ i_ਅਗਲਾ = 43;
  ਅੱਖਰ zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(ਆਕਾਰ(zHdl), zHdl, "bestindex%d", i_ਅਗਲਾ++);
  Tcl_CreateObjCommand(interp, zHdl, test_ਉੱਤਮ_ਸੂਚਕਾਂਕ_Obj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pScript);

  ਜੇ( rc!=TCL_OK ){
    ਅਚਲ ਅੱਖਰ *zErr = Tcl_Get_String_ਨਤੀਜਾ(interp);
    rc = SQLITE_ERROR;
    pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", zErr);
  }ਨਹੀਂਤਾਂ{
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
    Tcl_Obj *pRes = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
    Tcl_Obj **apElem = 0;
    Tcl_ਮਾਪ nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    ਜੇ( rc!=TCL_OK ){
      ਅਚਲ ਅੱਖਰ *zErr = Tcl_Get_String_ਨਤੀਜਾ(interp);
      rc = SQLITE_ERROR;
      pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", zErr);
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ ii;
      ਪੂਰਨ iArgv = 1;
      ਲਈ(ii=0; rc==SQLITE_OK && ii<(ਪੂਰਨ)nElem; ii+=2){
        ਅਚਲ ਅੱਖਰ *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ਜੇ( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimated_ਲਾਗਤ);
        }ਨਹੀਂਤਾਂ
        ਜੇ( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->ਕ੍ਰਮ_By_Consumed);
        }ਨਹੀਂਤਾਂ
        ਜੇ( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_ਅੰਕ);
        }ਨਹੀਂਤਾਂ
        ਜੇ( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->need_ਮੰਜ਼ਿਲ_Free_Idx_Str = 1;
        }ਨਹੀਂਤਾਂ
        ਜੇ( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimated_ਪੰਕਤੀਆਂ = (tRowcnt)x;
        }ਨਹੀਂਤਾਂ
        ਜੇ( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          ਪੂਰਨ iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          ਜੇ( rc==SQLITE_OK ){
            ਜੇ( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("unexpected: %d", iCons);
            }ਨਹੀਂਤਾਂ{
              ਪੂਰਨ bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argv_ਸੂਚਕਾਂਕ = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }ਨਹੀਂਤਾਂ
        ਜੇ( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", Tcl_GetString(p));
        }ਨਹੀਂਤਾਂ{
          rc = SQLITE_ERROR;
          pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        ਜੇ( rc!=SQLITE_OK && pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ==0 ){
          ਅਚਲ ਅੱਖਰ *zErr = Tcl_Get_String_ਨਤੀਜਾ(interp);
          pTab->ਅਧਾਰ.z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  ਵਾਪਸ rc;
}

ਸਥਿਰ ਖਾਲੀ tclFunction(sqlite3_context *pCtx, ਪੂਰਨ nArg, sqlite3_ਮੁੱਲ **apArg){
  Test_ਲੱਭੋ_Function *p = (Test_ਲੱਭੋ_Function*)sqlite3_user_ਅੰਕੜੇ(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  ਪੂਰਨ ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  ਲਈ(ii=0; ii<nArg; ii++){
    ਅਚਲ ਅੱਖਰ *zArg = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pScript);

  pRet = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

ਸਥਿਰ ਪੂਰਨ tcl_ਲੱਭੋ_Function(
  sqlite3_vtab *tab, 
  ਪੂਰਨ nArg, 
  ਅਚਲ ਅੱਖਰ *zName,
  ਖਾਲੀ (**pxFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**),   /*ਬਾਹਰਲਾ*/
  ਖਾਲੀ **ppArg                                             /*ਬਾਹਰਲਾ*/
){
  ਪੂਰਨ iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  ਪੂਰਨ rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pScript);

  ਜੇ( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_Get_Obj_ਨਤੀਜਾ(interp);

    ਜੇ( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }ਨਹੀਂਤਾਂ ਜੇ( iRet>0 ){
      sqlite3_int64 nName = ਲੜੀ_ਲੰਬਾਈ(zName);
      sqlite3_int64 nByte = nName + 1 + ਆਕਾਰ(Test_ਲੱਭੋ_Function);
      Test_ਲੱਭੋ_Function *pNew = 0;

      pNew = (Test_ਲੱਭੋ_Function*)sqlite3_malloc64(nByte);
      ਜੇ( pNew==0 ){
        iRet = 0;
      }ਨਹੀਂਤਾਂ{
        memset(pNew, 0, nByte);
        pNew->zName = (ਅਚਲ ਅੱਖਰ*)&pNew[1];
        ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਅੱਖਰ*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->p_ਅਗਲਾ = pTab->p_ਲੱਭੋ_Function_List;
        pTab->p_ਲੱਭੋ_Function_List = pNew;
        *ppArg = (ਖਾਲੀ*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  ਵਾਪਸ iRet;
}

ਸਥਿਰ ਪੂਰਨ tclUpdate(
  sqlite3_vtab *tab, 
  ਪੂਰਨ nArg, 
  sqlite3_ਮੁੱਲ **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  ਪੂਰਨ rc = TCL_OK;

  Tcl_Incr_Ref_ਗਿਣਤੀ(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pEval);

  ਜੇ( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  ਜੇ( rc!=TCL_OK ){
    tab->z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", Tcl_Get_String_ਨਤੀਜਾ(pTab->interp));
    ਵਾਪਸ rc;
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ਸਥਿਰ sqlite3_module tclModule = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_ਉੱਤਮ_ਸੂਚਕਾਂਕ,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_ਅਗਲਾ,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_ਕਾਲਮ,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_ਲੱਭੋ_Function,             /* xFindFunction */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};
ਸਥਿਰ sqlite3_module tclModuleUpdate = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_ਉੱਤਮ_ਸੂਚਕਾਂਕ,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_ਅਗਲਾ,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_ਕਾਲਮ,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  tclUpdate,                   /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_ਲੱਭੋ_Function,             /* xFindFunction */
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
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);

ਸਥਿਰ ਖਾਲੀ delTestVtabCtx(ਖਾਲੀ *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  ਜੇ( pCtx->pDefault ){
    Tcl_Decr_Ref_ਗਿਣਤੀ(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਰਜਿਸਟਰ_tcl_module(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ਜੇ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB ?DEFAULT-CMD?");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(ਆਕਾਰ(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    ਜੇ( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_ਗਿਣਤੀ(pCtx->pDefault);
    }

    ਜੇ( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (ਖਾਲੀ*)pCtx, delTestVtabCtx);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ


/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetesttcl_Init(Tcl_Interp *interp){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਖਾਲੀ *client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "register_tcl_module",   ਰਜਿਸਟਰ_tcl_module, 0 },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ਅੰਕੜੇ, 0);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}
