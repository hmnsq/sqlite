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


#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE


قسمتعریف ساخت tcl_vtab tcl_vtab;
قسمتعریف ساخت tcl_cursor tcl_cursor;
قسمتعریف ساخت Test_ڈھونڈو_Function Test_ڈھونڈو_Function;
قسمتعریف ساخت TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
ساخت tcl_vtab {
  sqlite3_vtab بنیاد;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  Test_ڈھونڈو_Function *p_ڈھونڈو_Function_List;
  sqlite3 *db;
};

/* A tcl cursor object */
ساخت tcl_cursor {
  sqlite3_vtab_cursor بنیاد;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

ساخت Test_ڈھونڈو_Function {
  tcl_vtab *pTab;
  ثابت حرف *zName;
  Test_ڈھونڈو_Function *p_اگلا;
};

ساخت TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
ساکن خالی tclDequote(حرف *z){
  حرف q = z[0];

  /* Set stack variable q to the close-quote character */
  اگر( q=='[' || q=='\'' || q=='"' || q=='`' ){
    صحیح i_اندر = 1;
    صحیح i_برآمد = 0;
    اگر( q=='[' ) q = ']';  

    جب( ALWAYS(z[i_اندر]) ){
      اگر( z[i_اندر]==q ){
        اگر( z[i_اندر+1]!=q ){
          /* Character iIn was the close quote. */
          i_اندر++;
          توڑو;
        }ورنہ{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          i_اندر += 2;
          z[i_برآمد++] = q;
        }
      }ورنہ{
        z[i_برآمد++] = z[i_اندر++];
      }
    }

    z[i_برآمد] = '\0';
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
ساکن صحیح tclConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  حرف *zCmd = 0;
  Tcl_Obj *pScript = 0;
  صحیح rc = SQLITE_OK;

  اگر( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    واپس SQLITE_ERROR;
  }

  اگر( argc==4 ){
    zCmd = sqlite3_malloc64(لڑی_لمبائی(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(حجم(tcl_vtab));
  اگر( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, حجم(tcl_vtab));

    اگر( zCmd ){
      حافظہ_نقل(zCmd, argv[3], لڑی_لمبائی(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }ورنہ{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_گنتی(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_گنتی(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    اگر( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_Get_String_نتیجہ(interp));
      اگر( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }ورنہ{
        rc = SQLITE_ERROR;
      }
    }ورنہ{
      rc = sqlite3_declare_vtab(db, Tcl_Get_String_نتیجہ(interp));
      اگر( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    اگر( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }ورنہ{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->بنیاد : 0;
  واپس rc;
}

/* The xDisconnect and xDestroy methods are also the same */
ساکن صحیح tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  جب( pTab->p_ڈھونڈو_Function_List ){
    Test_ڈھونڈو_Function *p = pTab->p_ڈھونڈو_Function_List;
    pTab->p_ڈھونڈو_Function_List = p->p_اگلا;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_گنتی(pTab->pCmd);
  sqlite3_free(pTab);
  واپس SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
ساکن صحیح tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(حجم(tcl_cursor));
  اگر( pCur==0 ) واپس SQLITE_NOMEM;
  memset(pCur, 0, حجم(tcl_cursor));
  *ppCursor = &pCur->بنیاد;
  واپس SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
ساکن صحیح tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  اگر( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  واپس SQLITE_OK;
}

ساکن صحیح tcl_اگلا(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  اگر( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    صحیح rc = sqlite3_قدم(pCsr->pStmt);
    اگر( rc!=SQLITE_صف ){
      ثابت حرف *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      اگر( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  واپس SQLITE_OK;
}

ساکن صحیح tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  صحیح ii;
  صحیح rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_گنتی(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_عدد));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pArg);
  برائے(ii=0; ii<argc; ii++){
    ثابت حرف *zVal = (ثابت حرف*)sqlite3_قدر_متن(argv[ii]);
    Tcl_Obj *pVal;
    اگر( zVal==0 ){
      sqlite3_قدر *pMem;
      pVal = Tcl_NewObj();
      برائے(rc=sqlite3_vtab_اندر_پہلا(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_اندر_اگلا(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (ثابت حرف*)sqlite3_قدر_متن(pMem);
        اگر( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }ورنہ{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }ورنہ{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_گنتی(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  اگر( rc!=TCL_OK ){
    ثابت حرف *zErr = Tcl_Get_String_نتیجہ(interp);
    rc = SQLITE_ERROR;
    pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("%s", zErr);
  }ورنہ{
    /* Analyze the scripts return value. The return value should be a tcl 
    ** list object with an even number of elements. The first element of each
    ** pair must be one of:
    ** 
    **   "sql"          (SQL statement to return data)
    */
    Tcl_Obj *pRes = Tcl_Get_Obj_نتیجہ(interp);
    Tcl_Obj **apElem = 0;
    Tcl_سائز nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    اگر( rc!=TCL_OK ){
      ثابت حرف *zErr = Tcl_Get_String_نتیجہ(interp);
      rc = SQLITE_ERROR;
      pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("%s", zErr);
    }ورنہ{
      برائے(ii=0; rc==SQLITE_OK && ii<(صحیح)nElem; ii+=2){
        ثابت حرف *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        اگر( sqlite3_stricmp("sql", zCmd)==0 ){
          ثابت حرف *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          اگر( rc!=SQLITE_OK ){
            ثابت حرف *zErr = sqlite3_errmsg(pTab->db);
            pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }ورنہ{
          rc = SQLITE_ERROR;
          pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  اگر( rc==SQLITE_OK ){
    rc = tcl_اگلا(pVtabCursor);
  }
  واپس rc;
}

ساکن صحیح tcl_عمود(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  صحیح i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_نتیجہ_قدر(ctx, sqlite3_عمود_قدر(pCsr->pStmt, i+1));
  واپس SQLITE_OK;
}

ساکن صحیح tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_عمود_int64(pCsr->pStmt, 0);
  واپس SQLITE_OK;
}

ساکن صحیح tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  واپس (pCsr->pStmt==0);
}

ساکن خالی test_بہترین_اشاریہ_Obj_Constraints(
  Tcl_Interp *interp, 
  sqlite3_اشاریہ_info *pIdxInfo
){
  صحیح ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pRes);
  برائے(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساخت sqlite3_اشاریہ_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    ثابت حرف *zOp = 0;

    Tcl_Incr_Ref_گنتی(pElem);

    انتخاب( pCons->op ){
      حالت SQLITE_اشاریہ_CONSTRAINT_EQ:
        zOp = "eq"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_GT:
        zOp = "gt"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_LE:
        zOp = "le"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_LT:
        zOp = "lt"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_GE:
        zOp = "ge"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_مطابقت:
        zOp = "match"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_LIKE:
        zOp = "like"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_GLOB:
        zOp = "glob"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_REGEXP:
        zOp = "regexp"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_NE:
        zOp = "ne"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_ISNOT:
        zOp = "isnot"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_ISNULL:
        zOp = "isnull"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_IS:
        zOp = "is"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_حد_بندی:
        zOp = "limit"; توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_انحراف:
        zOp = "offset"; توڑو;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    اگر( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }ورنہ{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->i_عمود));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_گنتی(pElem);
  }

  Tcl_سیٹ_Obj_نتیجہ(interp, pRes);
  Tcl_Decr_Ref_گنتی(pRes);
}

ساکن خالی test_بہترین_اشاریہ_Obj_Orderby(
  Tcl_Interp *interp, 
  sqlite3_اشاریہ_info *pIdxInfo
){
  صحیح ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pRes);
  برائے(ii=0; ii<pIdxInfo->n_ترتیب_By; ii++){
    ساخت sqlite3_اشاریہ_orderby ثابت *p_ترتیب = &pIdxInfo->a_ترتیب_By[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_گنتی(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_ترتیب->i_عمود));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_ترتیب->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_گنتی(pElem);
  }

  Tcl_سیٹ_Obj_نتیجہ(interp, pRes);
  Tcl_Decr_Ref_گنتی(pRes);
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
ساکن صحیح SQLITE_TCLAPI test_بہترین_اشاریہ_Obj(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ثابت حرف *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  صحیح ii;
  sqlite3_اشاریہ_info *pIdxInfo = (sqlite3_اشاریہ_info*)client_ڈیٹا;

  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SUB-COMMAND");
    واپس TCL_ERROR;
  }
  اگر( Tcl_Get_اشاریہ_From_Obj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    واپس TCL_ERROR;
  }

  اگر( ii<4 && objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
    واپس TCL_ERROR;
  }
  اگر( ii==4 && objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "INDEX BOOLEAN");
    واپس TCL_ERROR;
  }
  اگر( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "INDEX ?DEFAULT?");
    واپس TCL_ERROR;
  }

  انتخاب( ii ){
    حالت 0: أكد( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      test_بہترین_اشاریہ_Obj_Constraints(interp, pIdxInfo);
      توڑو;

    حالت 1: أكد( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      test_بہترین_اشاریہ_Obj_Orderby(interp, pIdxInfo);
      توڑو;

    حالت 2: أكد( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(pIdxInfo->ستون_مستعمل));
      توڑو;

    حالت 3: أكد( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      صحیح bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(bDistinct));
      توڑو;
    }

    حالت 4: أكد( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      صحیح iCons;
      صحیح bHandle;
      اگر( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        واپس TCL_ERROR;
      }
      Tcl_سیٹ_Obj_نتیجہ(interp, 
          Tcl_NewIntObj(sqlite3_vtab_اندر(pIdxInfo, iCons, bHandle))
      );
      توڑو;
    }

    حالت 5: أكد( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      صحیح iCons = 0;
      صحیح rc;
      sqlite3_قدر *pVal = 0;
      ثابت حرف *zVal = "";
      اگر( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        واپس TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_قدر(pIdxInfo, iCons, &pVal);
      اگر( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
        واپس TCL_ERROR;
      }
      اگر( pVal ){
        zVal = (ثابت حرف*)sqlite3_قدر_متن(pVal);
      }ورنہ اگر( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(zVal, -1));
      توڑو;
    }

    حالت 6: أكد( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      صحیح iCons = 0;
      ثابت حرف *zColl = "";
      اگر( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        واپس TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(zColl, -1));
      توڑو;
    }
  }

  واپس TCL_OK;
}

ساکن صحیح tcl_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  صحیح rc = SQLITE_OK;

  ساکن صحیح i_اگلا = 43;
  حرف zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_گنتی(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(حجم(zHdl), zHdl, "bestindex%d", i_اگلا++);
  Tcl_CreateObjCommand(interp, zHdl, test_بہترین_اشاریہ_Obj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_گنتی(pScript);

  اگر( rc!=TCL_OK ){
    ثابت حرف *zErr = Tcl_Get_String_نتیجہ(interp);
    rc = SQLITE_ERROR;
    pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("%s", zErr);
  }ورنہ{
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
    Tcl_Obj *pRes = Tcl_Get_Obj_نتیجہ(interp);
    Tcl_Obj **apElem = 0;
    Tcl_سائز nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    اگر( rc!=TCL_OK ){
      ثابت حرف *zErr = Tcl_Get_String_نتیجہ(interp);
      rc = SQLITE_ERROR;
      pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("%s", zErr);
    }ورنہ{
      صحیح ii;
      صحیح iArgv = 1;
      برائے(ii=0; rc==SQLITE_OK && ii<(صحیح)nElem; ii+=2){
        ثابت حرف *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        اگر( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimated_لاگت);
        }ورنہ
        اگر( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->ترتیب_By_Consumed);
        }ورنہ
        اگر( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_عدد);
        }ورنہ
        اگر( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->need_مقصد_Free_Idx_Str = 1;
        }ورنہ
        اگر( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimated_صفیں = (tRowcnt)x;
        }ورنہ
        اگر( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          صحیح iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          اگر( rc==SQLITE_OK ){
            اگر( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("unexpected: %d", iCons);
            }ورنہ{
              صحیح bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argv_اشاریہ = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }ورنہ
        اگر( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("%s", Tcl_GetString(p));
        }ورنہ{
          rc = SQLITE_ERROR;
          pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        اگر( rc!=SQLITE_OK && pTab->بنیاد.z_Err_پیغام==0 ){
          ثابت حرف *zErr = Tcl_Get_String_نتیجہ(interp);
          pTab->بنیاد.z_Err_پیغام = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  واپس rc;
}

ساکن خالی tclFunction(sqlite3_context *pCtx, صحیح nArg, sqlite3_قدر **apArg){
  Test_ڈھونڈو_Function *p = (Test_ڈھونڈو_Function*)sqlite3_user_ڈیٹا(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  صحیح ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_گنتی(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  برائے(ii=0; ii<nArg; ii++){
    ثابت حرف *zArg = (ثابت حرف*)sqlite3_قدر_متن(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_گنتی(pScript);

  pRet = Tcl_Get_Obj_نتیجہ(interp);
  sqlite3_نتیجہ_متن(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

ساکن صحیح tcl_ڈھونڈو_Function(
  sqlite3_vtab *tab, 
  صحیح nArg, 
  ثابت حرف *zName,
  خالی (**pxFunc)(sqlite3_context*,صحیح,sqlite3_قدر**),   /*برآمد*/
  خالی **ppArg                                             /*برآمد*/
){
  صحیح iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  صحیح rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_گنتی(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_گنتی(pScript);

  اگر( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_Get_Obj_نتیجہ(interp);

    اگر( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }ورنہ اگر( iRet>0 ){
      sqlite3_int64 nName = لڑی_لمبائی(zName);
      sqlite3_int64 nByte = nName + 1 + حجم(Test_ڈھونڈو_Function);
      Test_ڈھونڈو_Function *pNew = 0;

      pNew = (Test_ڈھونڈو_Function*)sqlite3_malloc64(nByte);
      اگر( pNew==0 ){
        iRet = 0;
      }ورنہ{
        memset(pNew, 0, nByte);
        pNew->zName = (ثابت حرف*)&pNew[1];
        حافظہ_نقل((حرف*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->p_اگلا = pTab->p_ڈھونڈو_Function_List;
        pTab->p_ڈھونڈو_Function_List = pNew;
        *ppArg = (خالی*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  واپس iRet;
}

ساکن صحیح tclUpdate(
  sqlite3_vtab *tab, 
  صحیح nArg, 
  sqlite3_قدر **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  صحیح rc = TCL_OK;

  Tcl_Incr_Ref_گنتی(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_گنتی(pEval);

  اگر( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_Get_Obj_نتیجہ(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  اگر( rc!=TCL_OK ){
    tab->z_Err_پیغام = sqlite3_mprintf("%s", Tcl_Get_String_نتیجہ(pTab->interp));
    واپس rc;
  }

  واپس SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ساکن sqlite3_module tclModule = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_بہترین_اشاریہ,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_اگلا,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_عمود,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_ڈھونڈو_Function,             /* xFindFunction */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};
ساکن sqlite3_module tclModuleUpdate = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_بہترین_اشاریہ,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_اگلا,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_عمود,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  tclUpdate,                   /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_ڈھونڈو_Function,             /* xFindFunction */
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
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

ساکن خالی delTestVtabCtx(خالی *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  اگر( pCtx->pDefault ){
    Tcl_Decr_Ref_گنتی(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
ساکن صحیح SQLITE_TCLAPI رجسٹر_tcl_module(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB ?DEFAULT-CMD?");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(حجم(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    اگر( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_گنتی(pCtx->pDefault);
    }

    اگر( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (خالی*)pCtx, delTestVtabCtx);
  }
#اگر_ختم
  واپس TCL_OK;
}

#اگر_ختم


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetesttcl_Init(Tcl_Interp *interp){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     خالی *client_ڈیٹا;
  } aObjCmd[] = {
     { "register_tcl_module",   رجسٹر_tcl_module, 0 },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ڈیٹا, 0);
  }
#اگر_ختم
  واپس TCL_OK;
}
