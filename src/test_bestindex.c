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


#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE


రకనిర్వచనం నిర్మాణం tcl_vtab tcl_vtab;
రకనిర్వచనం నిర్మాణం tcl_cursor tcl_cursor;
రకనిర్వచనం నిర్మాణం Test_కనుగొను_Function Test_కనుగొను_Function;
రకనిర్వచనం నిర్మాణం TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
నిర్మాణం tcl_vtab {
  sqlite3_vtab పునాది;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  Test_కనుగొను_Function *p_కనుగొను_Function_List;
  sqlite3 *db;
};

/* A tcl cursor object */
నిర్మాణం tcl_cursor {
  sqlite3_vtab_cursor పునాది;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

నిర్మాణం Test_కనుగొను_Function {
  tcl_vtab *pTab;
  స్థిరరాశి అక్షరం *zName;
  Test_కనుగొను_Function *p_తదుపరి;
};

నిర్మాణం TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
స్థిర శూన్య tclDequote(అక్షరం *z){
  అక్షరం q = z[0];

  /* Set stack variable q to the close-quote character */
  ఒకవేళ( q=='[' || q=='\'' || q=='"' || q=='`' ){
    పూర్ణాంకం i_లోపల = 1;
    పూర్ణాంకం i_బహిర్గతం = 0;
    ఒకవేళ( q=='[' ) q = ']';  

    అయితే( ALWAYS(z[i_లోపల]) ){
      ఒకవేళ( z[i_లోపల]==q ){
        ఒకవేళ( z[i_లోపల+1]!=q ){
          /* Character iIn was the close quote. */
          i_లోపల++;
          ఆపు;
        }లేకపోతే{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          i_లోపల += 2;
          z[i_బహిర్గతం++] = q;
        }
      }లేకపోతే{
        z[i_బహిర్గతం++] = z[i_లోపల++];
      }
    }

    z[i_బహిర్గతం] = '\0';
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
స్థిర పూర్ణాంకం tclConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  అక్షరం *zCmd = 0;
  Tcl_Obj *pScript = 0;
  పూర్ణాంకం rc = SQLITE_OK;

  ఒకవేళ( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    తిరిగి SQLITE_ERROR;
  }

  ఒకవేళ( argc==4 ){
    zCmd = sqlite3_malloc64(అక్షరశ్రేణి_పొడవు(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(పరిమాణం(tcl_vtab));
  ఒకవేళ( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, పరిమాణం(tcl_vtab));

    ఒకవేళ( zCmd ){
      జ్ఞాపకం_నకలు(zCmd, argv[3], అక్షరశ్రేణి_పొడవు(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }లేకపోతే{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_లెక్క(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_లెక్క(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    ఒకవేళ( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_Get_String_ఫలితం(interp));
      ఒకవేళ( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }లేకపోతే{
        rc = SQLITE_ERROR;
      }
    }లేకపోతే{
      rc = sqlite3_declare_vtab(db, Tcl_Get_String_ఫలితం(interp));
      ఒకవేళ( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    ఒకవేళ( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }లేకపోతే{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->పునాది : 0;
  తిరిగి rc;
}

/* The xDisconnect and xDestroy methods are also the same */
స్థిర పూర్ణాంకం tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  అయితే( pTab->p_కనుగొను_Function_List ){
    Test_కనుగొను_Function *p = pTab->p_కనుగొను_Function_List;
    pTab->p_కనుగొను_Function_List = p->p_తదుపరి;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_లెక్క(pTab->pCmd);
  sqlite3_free(pTab);
  తిరిగి SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
స్థిర పూర్ణాంకం tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(పరిమాణం(tcl_cursor));
  ఒకవేళ( pCur==0 ) తిరిగి SQLITE_NOMEM;
  memset(pCur, 0, పరిమాణం(tcl_cursor));
  *ppCursor = &pCur->పునాది;
  తిరిగి SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
స్థిర పూర్ణాంకం tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  ఒకవేళ( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం tcl_తదుపరి(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ఒకవేళ( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    పూర్ణాంకం rc = sqlite3_అడుగు(pCsr->pStmt);
    ఒకవేళ( rc!=SQLITE_అడ్డువరుస ){
      స్థిరరాశి అక్షరం *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      ఒకవేళ( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  పూర్ణాంకం ii;
  పూర్ణాంకం rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_లెక్క(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_అంకె));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_లెక్క(pArg);
  కొరకు(ii=0; ii<argc; ii++){
    స్థిరరాశి అక్షరం *zVal = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[ii]);
    Tcl_Obj *pVal;
    ఒకవేళ( zVal==0 ){
      sqlite3_విలువ *pMem;
      pVal = Tcl_NewObj();
      కొరకు(rc=sqlite3_vtab_లోపల_మొదటి(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_లోపల_తదుపరి(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(pMem);
        ఒకవేళ( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }లేకపోతే{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }లేకపోతే{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_లెక్క(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  ఒకవేళ( rc!=TCL_OK ){
    స్థిరరాశి అక్షరం *zErr = Tcl_Get_String_ఫలితం(interp);
    rc = SQLITE_ERROR;
    pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("%s", zErr);
  }లేకపోతే{
    /* Analyze the scripts return value. The return value should be a tcl 
    ** list object with an even number of elements. The first element of each
    ** pair must be one of:
    ** 
    **   "sql"          (SQL statement to return data)
    */
    Tcl_Obj *pRes = Tcl_Get_Obj_ఫలితం(interp);
    Tcl_Obj **apElem = 0;
    Tcl_కొలత nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    ఒకవేళ( rc!=TCL_OK ){
      స్థిరరాశి అక్షరం *zErr = Tcl_Get_String_ఫలితం(interp);
      rc = SQLITE_ERROR;
      pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("%s", zErr);
    }లేకపోతే{
      కొరకు(ii=0; rc==SQLITE_OK && ii<(పూర్ణాంకం)nElem; ii+=2){
        స్థిరరాశి అక్షరం *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ఒకవేళ( sqlite3_stricmp("sql", zCmd)==0 ){
          స్థిరరాశి అక్షరం *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          ఒకవేళ( rc!=SQLITE_OK ){
            స్థిరరాశి అక్షరం *zErr = sqlite3_errmsg(pTab->db);
            pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }లేకపోతే{
          rc = SQLITE_ERROR;
          pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  ఒకవేళ( rc==SQLITE_OK ){
    rc = tcl_తదుపరి(pVtabCursor);
  }
  తిరిగి rc;
}

స్థిర పూర్ణాంకం tcl_స్తంభం(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  పూర్ణాంకం i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_ఫలితం_విలువ(ctx, sqlite3_స్తంభం_విలువ(pCsr->pStmt, i+1));
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_స్తంభం_int64(pCsr->pStmt, 0);
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  తిరిగి (pCsr->pStmt==0);
}

స్థిర శూన్య test_ఉత్తమ_సూచీ_Obj_Constraints(
  Tcl_Interp *interp, 
  sqlite3_సూచీ_info *pIdxInfo
){
  పూర్ణాంకం ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_లెక్క(pRes);
  కొరకు(ii=0; ii<pIdxInfo->nConstraint; ii++){
    నిర్మాణం sqlite3_సూచీ_constraint స్థిరరాశి *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    స్థిరరాశి అక్షరం *zOp = 0;

    Tcl_Incr_Ref_లెక్క(pElem);

    ఎంపిక( pCons->op ){
      స్థితి SQLITE_సూచీ_CONSTRAINT_EQ:
        zOp = "eq"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_GT:
        zOp = "gt"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_LE:
        zOp = "le"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_LT:
        zOp = "lt"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_GE:
        zOp = "ge"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_సరిపోలిక:
        zOp = "match"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_LIKE:
        zOp = "like"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_GLOB:
        zOp = "glob"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_REGEXP:
        zOp = "regexp"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_NE:
        zOp = "ne"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_ISNOT:
        zOp = "isnot"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_ISNULL:
        zOp = "isnull"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_IS:
        zOp = "is"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_హద్దు:
        zOp = "limit"; ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_స్థానభ్రంశం:
        zOp = "offset"; ఆపు;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    ఒకవేళ( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }లేకపోతే{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->i_స్తంభం));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_లెక్క(pElem);
  }

  Tcl_సమితి_Obj_ఫలితం(interp, pRes);
  Tcl_Decr_Ref_లెక్క(pRes);
}

స్థిర శూన్య test_ఉత్తమ_సూచీ_Obj_Orderby(
  Tcl_Interp *interp, 
  sqlite3_సూచీ_info *pIdxInfo
){
  పూర్ణాంకం ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_లెక్క(pRes);
  కొరకు(ii=0; ii<pIdxInfo->n_క్రమం_By; ii++){
    నిర్మాణం sqlite3_సూచీ_orderby స్థిరరాశి *p_క్రమం = &pIdxInfo->a_క్రమం_By[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_లెక్క(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_క్రమం->i_స్తంభం));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_క్రమం->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_లెక్క(pElem);
  }

  Tcl_సమితి_Obj_ఫలితం(interp, pRes);
  Tcl_Decr_Ref_లెక్క(pRes);
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_ఉత్తమ_సూచీ_Obj(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  స్థిరరాశి అక్షరం *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  పూర్ణాంకం ii;
  sqlite3_సూచీ_info *pIdxInfo = (sqlite3_సూచీ_info*)client_దత్తాంశం;

  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SUB-COMMAND");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( ii<4 && objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( ii==4 && objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "INDEX BOOLEAN");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "INDEX ?DEFAULT?");
    తిరిగి TCL_ERROR;
  }

  ఎంపిక( ii ){
    స్థితి 0: ప్రతిజ్ఞా( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      test_ఉత్తమ_సూచీ_Obj_Constraints(interp, pIdxInfo);
      ఆపు;

    స్థితి 1: ప్రతిజ్ఞా( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      test_ఉత్తమ_సూచీ_Obj_Orderby(interp, pIdxInfo);
      ఆపు;

    స్థితి 2: ప్రతిజ్ఞా( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(pIdxInfo->నిలువు_వాడిన));
      ఆపు;

    స్థితి 3: ప్రతిజ్ఞా( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      పూర్ణాంకం bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(bDistinct));
      ఆపు;
    }

    స్థితి 4: ప్రతిజ్ఞా( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      పూర్ణాంకం iCons;
      పూర్ణాంకం bHandle;
      ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        తిరిగి TCL_ERROR;
      }
      Tcl_సమితి_Obj_ఫలితం(interp, 
          Tcl_NewIntObj(sqlite3_vtab_లోపల(pIdxInfo, iCons, bHandle))
      );
      ఆపు;
    }

    స్థితి 5: ప్రతిజ్ఞా( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      పూర్ణాంకం iCons = 0;
      పూర్ణాంకం rc;
      sqlite3_విలువ *pVal = 0;
      స్థిరరాశి అక్షరం *zVal = "";
      ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        తిరిగి TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_విలువ(pIdxInfo, iCons, &pVal);
      ఒకవేళ( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( pVal ){
        zVal = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(pVal);
      }లేకపోతే ఒకవేళ( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(zVal, -1));
      ఆపు;
    }

    స్థితి 6: ప్రతిజ్ఞా( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      పూర్ణాంకం iCons = 0;
      స్థిరరాశి అక్షరం *zColl = "";
      ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        తిరిగి TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(zColl, -1));
      ఆపు;
    }
  }

  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం tcl_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  పూర్ణాంకం rc = SQLITE_OK;

  స్థిర పూర్ణాంకం i_తదుపరి = 43;
  అక్షరం zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_లెక్క(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(పరిమాణం(zHdl), zHdl, "bestindex%d", i_తదుపరి++);
  Tcl_CreateObjCommand(interp, zHdl, test_ఉత్తమ_సూచీ_Obj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_లెక్క(pScript);

  ఒకవేళ( rc!=TCL_OK ){
    స్థిరరాశి అక్షరం *zErr = Tcl_Get_String_ఫలితం(interp);
    rc = SQLITE_ERROR;
    pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("%s", zErr);
  }లేకపోతే{
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
    Tcl_Obj *pRes = Tcl_Get_Obj_ఫలితం(interp);
    Tcl_Obj **apElem = 0;
    Tcl_కొలత nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    ఒకవేళ( rc!=TCL_OK ){
      స్థిరరాశి అక్షరం *zErr = Tcl_Get_String_ఫలితం(interp);
      rc = SQLITE_ERROR;
      pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("%s", zErr);
    }లేకపోతే{
      పూర్ణాంకం ii;
      పూర్ణాంకం iArgv = 1;
      కొరకు(ii=0; rc==SQLITE_OK && ii<(పూర్ణాంకం)nElem; ii+=2){
        స్థిరరాశి అక్షరం *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        ఒకవేళ( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimated_ఖర్చు);
        }లేకపోతే
        ఒకవేళ( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->క్రమం_By_Consumed);
        }లేకపోతే
        ఒకవేళ( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_అంకె);
        }లేకపోతే
        ఒకవేళ( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->need_గమ్యం_Free_Idx_Str = 1;
        }లేకపోతే
        ఒకవేళ( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimated_అడ్డువరుసలు = (tRowcnt)x;
        }లేకపోతే
        ఒకవేళ( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          పూర్ణాంకం iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          ఒకవేళ( rc==SQLITE_OK ){
            ఒకవేళ( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("unexpected: %d", iCons);
            }లేకపోతే{
              పూర్ణాంకం bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argv_సూచీ = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }లేకపోతే
        ఒకవేళ( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("%s", Tcl_GetString(p));
        }లేకపోతే{
          rc = SQLITE_ERROR;
          pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        ఒకవేళ( rc!=SQLITE_OK && pTab->పునాది.z_Err_సందేశం==0 ){
          స్థిరరాశి అక్షరం *zErr = Tcl_Get_String_ఫలితం(interp);
          pTab->పునాది.z_Err_సందేశం = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  తిరిగి rc;
}

స్థిర శూన్య tclFunction(sqlite3_context *pCtx, పూర్ణాంకం nArg, sqlite3_విలువ **apArg){
  Test_కనుగొను_Function *p = (Test_కనుగొను_Function*)sqlite3_user_దత్తాంశం(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  పూర్ణాంకం ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_లెక్క(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  కొరకు(ii=0; ii<nArg; ii++){
    స్థిరరాశి అక్షరం *zArg = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_లెక్క(pScript);

  pRet = Tcl_Get_Obj_ఫలితం(interp);
  sqlite3_ఫలితం_పాఠ్యం(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

స్థిర పూర్ణాంకం tcl_కనుగొను_Function(
  sqlite3_vtab *tab, 
  పూర్ణాంకం nArg, 
  స్థిరరాశి అక్షరం *zName,
  శూన్య (**pxFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**),   /*బహిర్గతం*/
  శూన్య **ppArg                                             /*బహిర్గతం*/
){
  పూర్ణాంకం iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  పూర్ణాంకం rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_లెక్క(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_లెక్క(pScript);

  ఒకవేళ( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_Get_Obj_ఫలితం(interp);

    ఒకవేళ( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }లేకపోతే ఒకవేళ( iRet>0 ){
      sqlite3_int64 nName = అక్షరశ్రేణి_పొడవు(zName);
      sqlite3_int64 nByte = nName + 1 + పరిమాణం(Test_కనుగొను_Function);
      Test_కనుగొను_Function *pNew = 0;

      pNew = (Test_కనుగొను_Function*)sqlite3_malloc64(nByte);
      ఒకవేళ( pNew==0 ){
        iRet = 0;
      }లేకపోతే{
        memset(pNew, 0, nByte);
        pNew->zName = (స్థిరరాశి అక్షరం*)&pNew[1];
        జ్ఞాపకం_నకలు((అక్షరం*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->p_తదుపరి = pTab->p_కనుగొను_Function_List;
        pTab->p_కనుగొను_Function_List = pNew;
        *ppArg = (శూన్య*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  తిరిగి iRet;
}

స్థిర పూర్ణాంకం tclUpdate(
  sqlite3_vtab *tab, 
  పూర్ణాంకం nArg, 
  sqlite3_విలువ **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  పూర్ణాంకం rc = TCL_OK;

  Tcl_Incr_Ref_లెక్క(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_లెక్క(pEval);

  ఒకవేళ( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_Get_Obj_ఫలితం(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  ఒకవేళ( rc!=TCL_OK ){
    tab->z_Err_సందేశం = sqlite3_mprintf("%s", Tcl_Get_String_ఫలితం(pTab->interp));
    తిరిగి rc;
  }

  తిరిగి SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
స్థిర sqlite3_module tclModule = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_ఉత్తమ_సూచీ,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_తదుపరి,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_స్తంభం,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_కనుగొను_Function,             /* xFindFunction */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};
స్థిర sqlite3_module tclModuleUpdate = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_ఉత్తమ_సూచీ,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_తదుపరి,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_స్తంభం,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  tclUpdate,                   /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_కనుగొను_Function,             /* xFindFunction */
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
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);

స్థిర శూన్య delTestVtabCtx(శూన్య *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  ఒకవేళ( pCtx->pDefault ){
    Tcl_Decr_Ref_లెక్క(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI రిజిస్టరు_tcl_module(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ఒకవేళ( objc!=2 && objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB ?DEFAULT-CMD?");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(పరిమాణం(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    ఒకవేళ( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_లెక్క(pCtx->pDefault);
    }

    ఒకవేళ( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (శూన్య*)pCtx, delTestVtabCtx);
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు


/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetesttcl_Init(Tcl_Interp *interp){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     శూన్య *client_దత్తాంశం;
  } aObjCmd[] = {
     { "register_tcl_module",   రిజిస్టరు_tcl_module, 0 },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_దత్తాంశం, 0);
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}
