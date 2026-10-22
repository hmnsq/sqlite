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


#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE


প্রকারলেখ কাঠামো tcl_vtab tcl_vtab;
প্রকারলেখ কাঠামো tcl_cursor tcl_cursor;
প্রকারলেখ কাঠামো TestFindFunction TestFindFunction;
প্রকারলেখ কাঠামো TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
কাঠামো tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
কাঠামো tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

কাঠামো TestFindFunction {
  tcl_vtab *pTab;
  ধ্রুবক অক্ষর *zName;
  TestFindFunction *p_পরবর্তী;
};

কাঠামো TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
স্থির শূন্য tclDequote(অক্ষর *z){
  অক্ষর q = z[0];

  /* Set stack variable q to the close-quote character */
  যদি( q=='[' || q=='\'' || q=='"' || q=='`' ){
    পূর্ণসংখ্যা iIn = 1;
    পূর্ণসংখ্যা iOut = 0;
    যদি( q=='[' ) q = ']';  

    যখন( ALWAYS(z[iIn]) ){
      যদি( z[iIn]==q ){
        যদি( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          বিরতি;
        }অন্যথা{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }অন্যথা{
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
স্থির পূর্ণসংখ্যা tclConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  অক্ষর *zCmd = 0;
  Tcl_Obj *pScript = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  যদি( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ফেরত SQLITE_ERROR;
  }

  যদি( argc==4 ){
    zCmd = sqlite3_malloc64(strlen(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(আকার(tcl_vtab));
  যদি( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, আকার(tcl_vtab));

    যদি( zCmd ){
      memcpy(zCmd, argv[3], strlen(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }অন্যথা{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_গুনতি(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_গুনতি(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    যদি( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_Get_String_ফলাফল(interp));
      যদি( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }অন্যথা{
        rc = SQLITE_ERROR;
      }
    }অন্যথা{
      rc = sqlite3_declare_vtab(db, Tcl_Get_String_ফলাফল(interp));
      যদি( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    যদি( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }অন্যথা{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  ফেরত rc;
}

/* The xDisconnect and xDestroy methods are also the same */
স্থির পূর্ণসংখ্যা tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  যখন( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->p_পরবর্তী;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_গুনতি(pTab->pCmd);
  sqlite3_free(pTab);
  ফেরত SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
স্থির পূর্ণসংখ্যা tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(আকার(tcl_cursor));
  যদি( pCur==0 ) ফেরত SQLITE_NOMEM;
  memset(pCur, 0, আকার(tcl_cursor));
  *ppCursor = &pCur->base;
  ফেরত SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
স্থির পূর্ণসংখ্যা tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  যদি( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা tcl_পরবর্তী(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  যদি( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    পূর্ণসংখ্যা rc = sqlite3_step(pCsr->pStmt);
    যদি( rc!=SQLITE_ROW ){
      ধ্রুবক অক্ষর *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      যদি( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.z_Err_বার্তা = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  পূর্ণসংখ্যা ii;
  পূর্ণসংখ্যা rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_গুনতি(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_অঙ্ক));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pArg);
  জন্য(ii=0; ii<argc; ii++){
    ধ্রুবক অক্ষর *zVal = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[ii]);
    Tcl_Obj *pVal;
    যদি( zVal==0 ){
      sqlite3_মান *pMem;
      pVal = Tcl_NewObj();
      জন্য(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_পরবর্তী(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (ধ্রুবক অক্ষর*)sqlite3_মান_text(pMem);
        যদি( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }অন্যথা{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }অন্যথা{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_গুনতি(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  যদি( rc!=TCL_OK ){
    ধ্রুবক অক্ষর *zErr = Tcl_Get_String_ফলাফল(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_বার্তা = sqlite3_mprintf("%s", zErr);
  }অন্যথা{
    /* Analyze the scripts return value. The return value should be a tcl 
    ** list object with an even number of elements. The first element of each
    ** pair must be one of:
    ** 
    **   "sql"          (SQL statement to return data)
    */
    Tcl_Obj *pRes = Tcl_Get_Obj_ফলাফল(interp);
    Tcl_Obj **apElem = 0;
    Tcl_আকার nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    যদি( rc!=TCL_OK ){
      ধ্রুবক অক্ষর *zErr = Tcl_Get_String_ফলাফল(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_বার্তা = sqlite3_mprintf("%s", zErr);
    }অন্যথা{
      জন্য(ii=0; rc==SQLITE_OK && ii<(পূর্ণসংখ্যা)nElem; ii+=2){
        ধ্রুবক অক্ষর *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        যদি( sqlite3_stricmp("sql", zCmd)==0 ){
          ধ্রুবক অক্ষর *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          যদি( rc!=SQLITE_OK ){
            ধ্রুবক অক্ষর *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.z_Err_বার্তা = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }অন্যথা{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_বার্তা = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  যদি( rc==SQLITE_OK ){
    rc = tcl_পরবর্তী(pVtabCursor);
  }
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  পূর্ণসংখ্যা i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_ফলাফল_মান(ctx, sqlite3_column_মান(pCsr->pStmt, i+1));
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  ফেরত (pCsr->pStmt==0);
}

স্থির শূন্য test_Best_সূচক_Obj_Constraints(
  Tcl_Interp *interp, 
  sqlite3_সূচক_info *pIdxInfo
){
  পূর্ণসংখ্যা ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pRes);
  জন্য(ii=0; ii<pIdxInfo->nConstraint; ii++){
    কাঠামো sqlite3_সূচক_constraint ধ্রুবক *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    ধ্রুবক অক্ষর *zOp = 0;

    Tcl_Incr_Ref_গুনতি(pElem);

    নির্বাচন( pCons->op ){
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_EQ:
        zOp = "eq"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_GT:
        zOp = "gt"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_LE:
        zOp = "le"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_LT:
        zOp = "lt"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_GE:
        zOp = "ge"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_MATCH:
        zOp = "match"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_LIKE:
        zOp = "like"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_GLOB:
        zOp = "glob"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_REGEXP:
        zOp = "regexp"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_NE:
        zOp = "ne"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_ISNOT:
        zOp = "isnot"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_ISNULL:
        zOp = "isnull"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_IS:
        zOp = "is"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_LIMIT:
        zOp = "limit"; বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_OFFSET:
        zOp = "offset"; বিরতি;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    যদি( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }অন্যথা{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_গুনতি(pElem);
  }

  Tcl_Set_Obj_ফলাফল(interp, pRes);
  Tcl_Decr_Ref_গুনতি(pRes);
}

স্থির শূন্য test_Best_সূচক_Obj_Orderby(
  Tcl_Interp *interp, 
  sqlite3_সূচক_info *pIdxInfo
){
  পূর্ণসংখ্যা ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pRes);
  জন্য(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    কাঠামো sqlite3_সূচক_orderby ধ্রুবক *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_গুনতি(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_গুনতি(pElem);
  }

  Tcl_Set_Obj_ফলাফল(interp, pRes);
  Tcl_Decr_Ref_গুনতি(pRes);
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_Best_সূচক_Obj(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ধ্রুবক অক্ষর *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  পূর্ণসংখ্যা ii;
  sqlite3_সূচক_info *pIdxInfo = (sqlite3_সূচক_info*)client_তথ্য;

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUB-COMMAND");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_Get_সূচক_From_Obj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    ফেরত TCL_ERROR;
  }

  যদি( ii<4 && objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ফেরত TCL_ERROR;
  }
  যদি( ii==4 && objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "INDEX BOOLEAN");
    ফেরত TCL_ERROR;
  }
  যদি( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "INDEX ?DEFAULT?");
    ফেরত TCL_ERROR;
  }

  নির্বাচন( ii ){
    ক্ষেত্র 0: প্রতিজ্ঞা( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      test_Best_সূচক_Obj_Constraints(interp, pIdxInfo);
      বিরতি;

    ক্ষেত্র 1: প্রতিজ্ঞা( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      test_Best_সূচক_Obj_Orderby(interp, pIdxInfo);
      বিরতি;

    ক্ষেত্র 2: প্রতিজ্ঞা( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      বিরতি;

    ক্ষেত্র 3: প্রতিজ্ঞা( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      পূর্ণসংখ্যা bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(bDistinct));
      বিরতি;
    }

    ক্ষেত্র 4: প্রতিজ্ঞা( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      পূর্ণসংখ্যা iCons;
      পূর্ণসংখ্যা bHandle;
      যদি( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        ফেরত TCL_ERROR;
      }
      Tcl_Set_Obj_ফলাফল(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      বিরতি;
    }

    ক্ষেত্র 5: প্রতিজ্ঞা( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      পূর্ণসংখ্যা iCons = 0;
      পূর্ণসংখ্যা rc;
      sqlite3_মান *pVal = 0;
      ধ্রুবক অক্ষর *zVal = "";
      যদি( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ফেরত TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_মান(pIdxInfo, iCons, &pVal);
      যদি( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_VOLATILE);
        ফেরত TCL_ERROR;
      }
      যদি( pVal ){
        zVal = (ধ্রুবক অক্ষর*)sqlite3_মান_text(pVal);
      }অন্যথা যদি( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(zVal, -1));
      বিরতি;
    }

    ক্ষেত্র 6: প্রতিজ্ঞা( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      পূর্ণসংখ্যা iCons = 0;
      ধ্রুবক অক্ষর *zColl = "";
      যদি( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        ফেরত TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(zColl, -1));
      বিরতি;
    }
  }

  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা tcl_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  স্থির পূর্ণসংখ্যা i_পরবর্তী = 43;
  অক্ষর zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_গুনতি(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(আকার(zHdl), zHdl, "bestindex%d", i_পরবর্তী++);
  Tcl_CreateObjCommand(interp, zHdl, test_Best_সূচক_Obj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_গুনতি(pScript);

  যদি( rc!=TCL_OK ){
    ধ্রুবক অক্ষর *zErr = Tcl_Get_String_ফলাফল(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_বার্তা = sqlite3_mprintf("%s", zErr);
  }অন্যথা{
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
    Tcl_Obj *pRes = Tcl_Get_Obj_ফলাফল(interp);
    Tcl_Obj **apElem = 0;
    Tcl_আকার nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    যদি( rc!=TCL_OK ){
      ধ্রুবক অক্ষর *zErr = Tcl_Get_String_ফলাফল(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_বার্তা = sqlite3_mprintf("%s", zErr);
    }অন্যথা{
      পূর্ণসংখ্যা ii;
      পূর্ণসংখ্যা iArgv = 1;
      জন্য(ii=0; rc==SQLITE_OK && ii<(পূর্ণসংখ্যা)nElem; ii+=2){
        ধ্রুবক অক্ষর *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        যদি( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }অন্যথা
        যদি( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }অন্যথা
        যদি( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_অঙ্ক);
        }অন্যথা
        যদি( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }অন্যথা
        যদি( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }অন্যথা
        যদি( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          পূর্ণসংখ্যা iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          যদি( rc==SQLITE_OK ){
            যদি( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.z_Err_বার্তা = sqlite3_mprintf("unexpected: %d", iCons);
            }অন্যথা{
              পূর্ণসংখ্যা bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argv_সূচক = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }অন্যথা
        যদি( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.z_Err_বার্তা = sqlite3_mprintf("%s", Tcl_GetString(p));
        }অন্যথা{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_বার্তা = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        যদি( rc!=SQLITE_OK && pTab->base.z_Err_বার্তা==0 ){
          ধ্রুবক অক্ষর *zErr = Tcl_Get_String_ফলাফল(interp);
          pTab->base.z_Err_বার্তা = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  ফেরত rc;
}

স্থির শূন্য tclFunction(sqlite3_context *pCtx, পূর্ণসংখ্যা nArg, sqlite3_মান **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_তথ্য(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  পূর্ণসংখ্যা ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_গুনতি(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  জন্য(ii=0; ii<nArg; ii++){
    ধ্রুবক অক্ষর *zArg = (ধ্রুবক অক্ষর*)sqlite3_মান_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_গুনতি(pScript);

  pRet = Tcl_Get_Obj_ফলাফল(interp);
  sqlite3_ফলাফল_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

স্থির পূর্ণসংখ্যা tclFindFunction(
  sqlite3_vtab *tab, 
  পূর্ণসংখ্যা nArg, 
  ধ্রুবক অক্ষর *zName,
  শূন্য (**pxFunc)(sqlite3_context*,পূর্ণসংখ্যা,sqlite3_মান**),   /* OUT */
  শূন্য **ppArg                                             /* OUT */
){
  পূর্ণসংখ্যা iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_গুনতি(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_গুনতি(pScript);

  যদি( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_Get_Obj_ফলাফল(interp);

    যদি( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }অন্যথা যদি( iRet>0 ){
      sqlite3_int64 nName = strlen(zName);
      sqlite3_int64 nByte = nName + 1 + আকার(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      যদি( pNew==0 ){
        iRet = 0;
      }অন্যথা{
        memset(pNew, 0, nByte);
        pNew->zName = (ধ্রুবক অক্ষর*)&pNew[1];
        memcpy((অক্ষর*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->p_পরবর্তী = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (শূন্য*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  ফেরত iRet;
}

স্থির পূর্ণসংখ্যা tclUpdate(
  sqlite3_vtab *tab, 
  পূর্ণসংখ্যা nArg, 
  sqlite3_মান **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  পূর্ণসংখ্যা rc = TCL_OK;

  Tcl_Incr_Ref_গুনতি(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_গুনতি(pEval);

  যদি( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_Get_Obj_ফলাফল(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  যদি( rc!=TCL_OK ){
    tab->z_Err_বার্তা = sqlite3_mprintf("%s", Tcl_Get_String_ফলাফল(pTab->interp));
    ফেরত rc;
  }

  ফেরত SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
স্থির sqlite3_module tclModule = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_Best_সূচক,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_পরবর্তী,                      /* xNext - advance a cursor */
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
স্থির sqlite3_module tclModuleUpdate = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_Best_সূচক,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_পরবর্তী,                      /* xNext - advance a cursor */
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
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);

স্থির শূন্য delTestVtabCtx(শূন্য *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  যদি( pCtx->pDefault ){
    Tcl_Decr_Ref_গুনতি(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI register_tcl_module(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  যদি( objc!=2 && objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB ?DEFAULT-CMD?");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(আকার(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    যদি( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_গুনতি(pCtx->pDefault);
    }

    যদি( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (শূন্য*)pCtx, delTestVtabCtx);
  }
#endif
  ফেরত TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *client_তথ্য;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_তথ্য, 0);
  }
#endif
  ফেরত TCL_OK;
}
