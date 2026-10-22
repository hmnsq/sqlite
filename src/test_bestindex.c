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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ tcl_vtab tcl_vtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ tcl_cursor tcl_cursor;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Test_ಕಂಡುಹಿಡಿ_Function Test_ಕಂಡುಹಿಡಿ_Function;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
ರಚನೆ tcl_vtab {
  sqlite3_vtab ತಳ;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  Test_ಕಂಡುಹಿಡಿ_Function *p_ಕಂಡುಹಿಡಿ_Function_List;
  sqlite3 *db;
};

/* A tcl cursor object */
ರಚನೆ tcl_cursor {
  sqlite3_vtab_cursor ತಳ;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

ರಚನೆ Test_ಕಂಡುಹಿಡಿ_Function {
  tcl_vtab *pTab;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
  Test_ಕಂಡುಹಿಡಿ_Function *p_ಮುಂದಿನ;
};

ರಚನೆ TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tclDequote(ಅಕ್ಷರ *z){
  ಅಕ್ಷರ q = z[0];

  /* Set stack variable q to the close-quote character */
  ಆದರೆ( q=='[' || q=='\'' || q=='"' || q=='`' ){
    ಪೂರ್ಣಾಂಕ i_ಒಳ = 1;
    ಪೂರ್ಣಾಂಕ i_ಹೊರಹೋಗು = 0;
    ಆದರೆ( q=='[' ) q = ']';  

    ಆಗ( ALWAYS(z[i_ಒಳ]) ){
      ಆದರೆ( z[i_ಒಳ]==q ){
        ಆದರೆ( z[i_ಒಳ+1]!=q ){
          /* Character iIn was the close quote. */
          i_ಒಳ++;
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          i_ಒಳ += 2;
          z[i_ಹೊರಹೋಗು++] = q;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        z[i_ಹೊರಹೋಗು++] = z[i_ಒಳ++];
      }
    }

    z[i_ಹೊರಹೋಗು] = '\0';
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  ಅಕ್ಷರ *zCmd = 0;
  Tcl_Obj *pScript = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಆದರೆ( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  ಆದರೆ( argc==4 ){
    zCmd = sqlite3_malloc64(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(ಗಾತ್ರ(tcl_vtab));
  ಆದರೆ( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, ಗಾತ್ರ(tcl_vtab));

    ಆದರೆ( zCmd ){
      ಸ್ಮರಣೆ_ನಕಲು(zCmd, argv[3], ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_ಎಣಿಕೆ(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    ಆದರೆ( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_Get_String_ಫಲಿತಾಂಶ(interp));
      ಆದರೆ( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = SQLITE_ERROR;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = sqlite3_declare_vtab(db, Tcl_Get_String_ಫಲಿತಾಂಶ(interp));
      ಆದರೆ( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    ಆದರೆ( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->ತಳ : 0;
  ಹಿಂತಿರುಗಿ rc;
}

/* The xDisconnect and xDestroy methods are also the same */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  ಆಗ( pTab->p_ಕಂಡುಹಿಡಿ_Function_List ){
    Test_ಕಂಡುಹಿಡಿ_Function *p = pTab->p_ಕಂಡುಹಿಡಿ_Function_List;
    pTab->p_ಕಂಡುಹಿಡಿ_Function_List = p->p_ಮುಂದಿನ;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_ಎಣಿಕೆ(pTab->pCmd);
  sqlite3_free(pTab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(ಗಾತ್ರ(tcl_cursor));
  ಆದರೆ( pCur==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pCur, 0, ಗಾತ್ರ(tcl_cursor));
  *ppCursor = &pCur->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  ಆದರೆ( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tcl_ಮುಂದಿನ(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ಆದರೆ( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    ಪೂರ್ಣಾಂಕ rc = sqlite3_ಹೆಜ್ಜೆ(pCsr->pStmt);
    ಆದರೆ( rc!=SQLITE_ಅಡ್ಡಸಾಲು ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      ಆದರೆ( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  ಪೂರ್ಣಾಂಕ ii;
  ಪೂರ್ಣಾಂಕ rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_ಅಂಕೆ));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pArg);
  ಗಾಗಿ(ii=0; ii<argc; ii++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVal = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[ii]);
    Tcl_Obj *pVal;
    ಆದರೆ( zVal==0 ){
      sqlite3_ಮೌಲ್ಯ *pMem;
      pVal = Tcl_NewObj();
      ಗಾಗಿ(rc=sqlite3_vtab_ಒಳ_ಮೊದಲ(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_ಒಳ_ಮುಂದಿನ(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pMem);
        ಆದರೆ( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }ಇಲ್ಲದಿದ್ದರೆ{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  ಆದರೆ( rc!=TCL_OK ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr = Tcl_Get_String_ಫಲಿತಾಂಶ(interp);
    rc = SQLITE_ERROR;
    pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Analyze the scripts return value. The return value should be a tcl 
    ** list object with an even number of elements. The first element of each
    ** pair must be one of:
    ** 
    **   "sql"          (SQL statement to return data)
    */
    Tcl_Obj *pRes = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
    Tcl_Obj **apElem = 0;
    Tcl_ಗಾತ್ರ nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    ಆದರೆ( rc!=TCL_OK ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr = Tcl_Get_String_ಫಲಿತಾಂಶ(interp);
      rc = SQLITE_ERROR;
      pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಗಾಗಿ(ii=0; rc==SQLITE_OK && ii<(ಪೂರ್ಣಾಂಕ)nElem; ii+=2){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ಆದರೆ( sqlite3_stricmp("sql", zCmd)==0 ){
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          ಆದರೆ( rc!=SQLITE_OK ){
            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr = sqlite3_errmsg(pTab->db);
            pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = SQLITE_ERROR;
          pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  ಆದರೆ( rc==SQLITE_OK ){
    rc = tcl_ಮುಂದಿನ(pVtabCursor);
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tcl_ಸ್ತಂಭ(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಂಕ i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(ctx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pCsr->pStmt, i+1));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_ಸ್ತಂಭ_int64(pCsr->pStmt, 0);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ಹಿಂತಿರುಗಿ (pCsr->pStmt==0);
}

ಸ್ಥಿರ ಶೂನ್ಯ test_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ_Obj_Constraints(
  Tcl_Interp *interp, 
  sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo
){
  ಪೂರ್ಣಾಂಕ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRes);
  ಗಾಗಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint ಸ್ಥಿರಾಂಕ *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOp = 0;

    Tcl_Incr_Ref_ಎಣಿಕೆ(pElem);

    ಆಯ್ಕೆ( pCons->op ){
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ:
        zOp = "eq"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GT:
        zOp = "gt"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LE:
        zOp = "le"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LT:
        zOp = "lt"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GE:
        zOp = "ge"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಹೊಂದಿಕೆ:
        zOp = "match"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LIKE:
        zOp = "like"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GLOB:
        zOp = "glob"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_REGEXP:
        zOp = "regexp"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_NE:
        zOp = "ne"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ISNOT:
        zOp = "isnot"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ISNULL:
        zOp = "isnull"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_IS:
        zOp = "is"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಎಲ್ಲೆ:
        zOp = "limit"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಸ್ಥಾನಾಂತರ:
        zOp = "offset"; ನಿಲ್ಲಿಸು;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    ಆದರೆ( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->i_ಸ್ತಂಭ));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pElem);
  }

  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRes);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pRes);
}

ಸ್ಥಿರ ಶೂನ್ಯ test_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ_Obj_Orderby(
  Tcl_Interp *interp, 
  sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo
){
  ಪೂರ್ಣಾಂಕ ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pRes);
  ಗಾಗಿ(ii=0; ii<pIdxInfo->n_ಕ್ರಮ_By; ii++){
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_orderby ಸ್ಥಿರಾಂಕ *p_ಕ್ರಮ = &pIdxInfo->a_ಕ್ರಮ_By[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_ಎಣಿಕೆ(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_ಕ್ರಮ->i_ಸ್ತಂಭ));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_ಕ್ರಮ->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pElem);
  }

  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRes);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ_Obj(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  ಪೂರ್ಣಾಂಕ ii;
  sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo = (sqlite3_ಸೂಚ್ಯಂಕ_info*)client_ದತ್ತಾಂಶ;

  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SUB-COMMAND");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( ii<4 && objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( ii==4 && objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "INDEX BOOLEAN");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "INDEX ?DEFAULT?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆಯ್ಕೆ( ii ){
    ಸ್ಥಿತಿ 0: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      test_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ_Obj_Constraints(interp, pIdxInfo);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ 1: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      test_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ_Obj_Orderby(interp, pIdxInfo);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ 2: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(pIdxInfo->ಲಂಬಸಾಲು_ಬಳಸಿದ));
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ 3: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      ಪೂರ್ಣಾಂಕ bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(bDistinct));
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ 4: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      ಪೂರ್ಣಾಂಕ iCons;
      ಪೂರ್ಣಾಂಕ bHandle;
      ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, 
          Tcl_NewIntObj(sqlite3_vtab_ಒಳ(pIdxInfo, iCons, bHandle))
      );
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ 5: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      ಪೂರ್ಣಾಂಕ iCons = 0;
      ಪೂರ್ಣಾಂಕ rc;
      sqlite3_ಮೌಲ್ಯ *pVal = 0;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVal = "";
      ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_ಮೌಲ್ಯ(pIdxInfo, iCons, &pVal);
      ಆದರೆ( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( pVal ){
        zVal = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(pVal);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(zVal, -1));
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ 6: ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      ಪೂರ್ಣಾಂಕ iCons = 0;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zColl = "";
      ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(zColl, -1));
      ನಿಲ್ಲಿಸು;
    }
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tcl_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ i_ಮುಂದಿನ = 43;
  ಅಕ್ಷರ zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(ಗಾತ್ರ(zHdl), zHdl, "bestindex%d", i_ಮುಂದಿನ++);
  Tcl_CreateObjCommand(interp, zHdl, test_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ_Obj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pScript);

  ಆದರೆ( rc!=TCL_OK ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr = Tcl_Get_String_ಫಲಿತಾಂಶ(interp);
    rc = SQLITE_ERROR;
    pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
  }ಇಲ್ಲದಿದ್ದರೆ{
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
    Tcl_Obj *pRes = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
    Tcl_Obj **apElem = 0;
    Tcl_ಗಾತ್ರ nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    ಆದರೆ( rc!=TCL_OK ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr = Tcl_Get_String_ಫಲಿತಾಂಶ(interp);
      rc = SQLITE_ERROR;
      pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ ii;
      ಪೂರ್ಣಾಂಕ iArgv = 1;
      ಗಾಗಿ(ii=0; rc==SQLITE_OK && ii<(ಪೂರ್ಣಾಂಕ)nElem; ii+=2){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ಆದರೆ( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimated_ವೆಚ್ಚ);
        }ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->ಕ್ರಮ_By_Consumed);
        }ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_ಅಂಕೆ);
        }ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->need_ಗಮ್ಯ_Free_Idx_Str = 1;
        }ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimated_ಅಡ್ಡಸಾಲುಗಳು = (tRowcnt)x;
        }ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          ಪೂರ್ಣಾಂಕ iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          ಆದರೆ( rc==SQLITE_OK ){
            ಆದರೆ( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected: %d", iCons);
            }ಇಲ್ಲದಿದ್ದರೆ{
              ಪೂರ್ಣಾಂಕ bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argv_ಸೂಚ್ಯಂಕ = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", Tcl_GetString(p));
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = SQLITE_ERROR;
          pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        ಆದರೆ( rc!=SQLITE_OK && pTab->ತಳ.z_Err_ಸಂದೇಶ==0 ){
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr = Tcl_Get_String_ಫಲಿತಾಂಶ(interp);
          pTab->ತಳ.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಶೂನ್ಯ tclFunction(sqlite3_context *pCtx, ಪೂರ್ಣಾಂಕ nArg, sqlite3_ಮೌಲ್ಯ **apArg){
  Test_ಕಂಡುಹಿಡಿ_Function *p = (Test_ಕಂಡುಹಿಡಿ_Function*)sqlite3_user_ದತ್ತಾಂಶ(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  ಪೂರ್ಣಾಂಕ ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  ಗಾಗಿ(ii=0; ii<nArg; ii++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pScript);

  pRet = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tcl_ಕಂಡುಹಿಡಿ_Function(
  sqlite3_vtab *tab, 
  ಪೂರ್ಣಾಂಕ nArg, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  ಶೂನ್ಯ (**pxFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**),   /*ಹೊರಹೋಗು*/
  ಶೂನ್ಯ **ppArg                                             /*ಹೊರಹೋಗು*/
){
  ಪೂರ್ಣಾಂಕ iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pScript);

  ಆದರೆ( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);

    ಆದರೆ( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iRet>0 ){
      sqlite3_int64 nName = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName);
      sqlite3_int64 nByte = nName + 1 + ಗಾತ್ರ(Test_ಕಂಡುಹಿಡಿ_Function);
      Test_ಕಂಡುಹಿಡಿ_Function *pNew = 0;

      pNew = (Test_ಕಂಡುಹಿಡಿ_Function*)sqlite3_malloc64(nByte);
      ಆದರೆ( pNew==0 ){
        iRet = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        memset(pNew, 0, nByte);
        pNew->zName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&pNew[1];
        ಸ್ಮರಣೆ_ನಕಲು((ಅಕ್ಷರ*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->p_ಮುಂದಿನ = pTab->p_ಕಂಡುಹಿಡಿ_Function_List;
        pTab->p_ಕಂಡುಹಿಡಿ_Function_List = pNew;
        *ppArg = (ಶೂನ್ಯ*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  ಹಿಂತಿರುಗಿ iRet;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclUpdate(
  sqlite3_vtab *tab, 
  ಪೂರ್ಣಾಂಕ nArg, 
  sqlite3_ಮೌಲ್ಯ **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  ಪೂರ್ಣಾಂಕ rc = TCL_OK;

  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);

  ಆದರೆ( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  ಆದರೆ( rc!=TCL_OK ){
    tab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", Tcl_Get_String_ಫಲಿತಾಂಶ(pTab->interp));
    ಹಿಂತಿರುಗಿ rc;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ಸ್ಥಿರ sqlite3_module tclModule = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_ಮುಂದಿನ,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_ಸ್ತಂಭ,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_ಕಂಡುಹಿಡಿ_Function,             /* xFindFunction */
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
  tcl_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_ಮುಂದಿನ,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_ಸ್ತಂಭ,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  tclUpdate,                   /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_ಕಂಡುಹಿಡಿ_Function,             /* xFindFunction */
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
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);

ಸ್ಥಿರ ಶೂನ್ಯ delTestVtabCtx(ಶೂನ್ಯ *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  ಆದರೆ( pCtx->pDefault ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ರಿಜಿಸ್ಟರ್_tcl_module(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಆದರೆ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB ?DEFAULT-CMD?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(ಗಾತ್ರ(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    ಆದರೆ( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCtx->pDefault);
    }

    ಆದರೆ( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (ಶೂನ್ಯ*)pCtx, delTestVtabCtx);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetesttcl_Init(Tcl_Interp *interp){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "register_tcl_module",   ರಿಜಿಸ್ಟರ್_tcl_module, 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ದತ್ತಾಂಶ, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}
