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


#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE


تعريفنوع بنية tcl_vtab tcl_vtab;
تعريفنوع بنية tcl_cursor tcl_cursor;
تعريفنوع بنية Test_ڳوليو_Function Test_ڳوليو_Function;
تعريفنوع بنية TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
بنية tcl_vtab {
  sqlite3_vtab بنياد;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  Test_ڳوليو_Function *p_ڳوليو_Function_List;
  sqlite3 *db;
};

/* A tcl cursor object */
بنية tcl_cursor {
  sqlite3_vtab_cursor بنياد;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

بنية Test_ڳوليو_Function {
  tcl_vtab *pTab;
  ثابت حرف *zName;
  Test_ڳوليو_Function *p_ايندڙ;
};

بنية TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
ساكن فراغ tclDequote(حرف *z){
  حرف q = z[0];

  /* Set stack variable q to the close-quote character */
  إذا( q=='[' || q=='\'' || q=='"' || q=='`' ){
    صحيح i_اندريون = 1;
    صحيح i_ٻاهريون = 0;
    إذا( q=='[' ) q = ']';  

    طالما( ALWAYS(z[i_اندريون]) ){
      إذا( z[i_اندريون]==q ){
        إذا( z[i_اندريون+1]!=q ){
          /* Character iIn was the close quote. */
          i_اندريون++;
          اكسر;
        }وإلا{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          i_اندريون += 2;
          z[i_ٻاهريون++] = q;
        }
      }وإلا{
        z[i_ٻاهريون++] = z[i_اندريون++];
      }
    }

    z[i_ٻاهريون] = '\0';
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
ساكن صحيح tclConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  حرف *zCmd = 0;
  Tcl_Obj *pScript = 0;
  صحيح rc = SQLITE_OK;

  إذا( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    أرجع SQLITE_ERROR;
  }

  إذا( argc==4 ){
    zCmd = sqlite3_malloc64(لڙي_ڊيگهه(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(حجم(tcl_vtab));
  إذا( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, حجم(tcl_vtab));

    إذا( zCmd ){
      يادگيري_نقل(zCmd, argv[3], لڙي_ڊيگهه(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }وإلا{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_ڳڻپ(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_ڳڻپ(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    إذا( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_Get_String_نتيجو(interp));
      إذا( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }وإلا{
        rc = SQLITE_ERROR;
      }
    }وإلا{
      rc = sqlite3_declare_vtab(db, Tcl_Get_String_نتيجو(interp));
      إذا( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    إذا( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }وإلا{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->بنياد : 0;
  أرجع rc;
}

/* The xDisconnect and xDestroy methods are also the same */
ساكن صحيح tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  طالما( pTab->p_ڳوليو_Function_List ){
    Test_ڳوليو_Function *p = pTab->p_ڳوليو_Function_List;
    pTab->p_ڳوليو_Function_List = p->p_ايندڙ;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_ڳڻپ(pTab->pCmd);
  sqlite3_free(pTab);
  أرجع SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
ساكن صحيح tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(حجم(tcl_cursor));
  إذا( pCur==0 ) أرجع SQLITE_NOMEM;
  memset(pCur, 0, حجم(tcl_cursor));
  *ppCursor = &pCur->بنياد;
  أرجع SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
ساكن صحيح tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  إذا( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  أرجع SQLITE_OK;
}

ساكن صحيح tcl_ايندڙ(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  إذا( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    صحيح rc = sqlite3_قدم(pCsr->pStmt);
    إذا( rc!=SQLITE_صف ){
      ثابت حرف *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      إذا( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  أرجع SQLITE_OK;
}

ساكن صحيح tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_انگ, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  صحيح ii;
  صحيح rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ڳڻپ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_انگ));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_ڳڻپ(pArg);
  لكل(ii=0; ii<argc; ii++){
    ثابت حرف *zVal = (ثابت حرف*)sqlite3_قدر_متن(argv[ii]);
    Tcl_Obj *pVal;
    إذا( zVal==0 ){
      sqlite3_قدر *pMem;
      pVal = Tcl_NewObj();
      لكل(rc=sqlite3_vtab_اندريون_پهريون(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_اندريون_ايندڙ(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (ثابت حرف*)sqlite3_قدر_متن(pMem);
        إذا( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }وإلا{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }وإلا{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_ڳڻپ(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  إذا( rc!=TCL_OK ){
    ثابت حرف *zErr = Tcl_Get_String_نتيجو(interp);
    rc = SQLITE_ERROR;
    pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("%s", zErr);
  }وإلا{
    /* Analyze the scripts return value. The return value should be a tcl 
    ** list object with an even number of elements. The first element of each
    ** pair must be one of:
    ** 
    **   "sql"          (SQL statement to return data)
    */
    Tcl_Obj *pRes = Tcl_Get_Obj_نتيجو(interp);
    Tcl_Obj **apElem = 0;
    Tcl_سائيز nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    إذا( rc!=TCL_OK ){
      ثابت حرف *zErr = Tcl_Get_String_نتيجو(interp);
      rc = SQLITE_ERROR;
      pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("%s", zErr);
    }وإلا{
      لكل(ii=0; rc==SQLITE_OK && ii<(صحيح)nElem; ii+=2){
        ثابت حرف *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        إذا( sqlite3_stricmp("sql", zCmd)==0 ){
          ثابت حرف *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          إذا( rc!=SQLITE_OK ){
            ثابت حرف *zErr = sqlite3_errmsg(pTab->db);
            pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }وإلا{
          rc = SQLITE_ERROR;
          pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  إذا( rc==SQLITE_OK ){
    rc = tcl_ايندڙ(pVtabCursor);
  }
  أرجع rc;
}

ساكن صحيح tcl_ڪالم(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  صحيح i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_نتيجو_قدر(ctx, sqlite3_ڪالم_قدر(pCsr->pStmt, i+1));
  أرجع SQLITE_OK;
}

ساكن صحيح tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_ڪالم_int64(pCsr->pStmt, 0);
  أرجع SQLITE_OK;
}

ساكن صحيح tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  أرجع (pCsr->pStmt==0);
}

ساكن فراغ test_بهترين_اشاريو_Obj_Constraints(
  Tcl_Interp *interp, 
  sqlite3_اشاريو_info *pIdxInfo
){
  صحيح ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ڳڻپ(pRes);
  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    بنية sqlite3_اشاريو_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    ثابت حرف *zOp = 0;

    Tcl_Incr_Ref_ڳڻپ(pElem);

    بدل( pCons->op ){
      حالة SQLITE_اشاريو_CONSTRAINT_EQ:
        zOp = "eq"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_GT:
        zOp = "gt"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_LE:
        zOp = "le"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_LT:
        zOp = "lt"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_GE:
        zOp = "ge"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_ميلاپ:
        zOp = "match"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_LIKE:
        zOp = "like"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_GLOB:
        zOp = "glob"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_REGEXP:
        zOp = "regexp"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_NE:
        zOp = "ne"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_ISNOT:
        zOp = "isnot"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_ISNULL:
        zOp = "isnull"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_IS:
        zOp = "is"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_حد_بندي:
        zOp = "limit"; اكسر;
      حالة SQLITE_اشاريو_CONSTRAINT_وٿي:
        zOp = "offset"; اكسر;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    إذا( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }وإلا{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->i_ڪالم));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ڳڻپ(pElem);
  }

  Tcl_سيٽ_Obj_نتيجو(interp, pRes);
  Tcl_Decr_Ref_ڳڻپ(pRes);
}

ساكن فراغ test_بهترين_اشاريو_Obj_Orderby(
  Tcl_Interp *interp, 
  sqlite3_اشاريو_info *pIdxInfo
){
  صحيح ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ڳڻپ(pRes);
  لكل(ii=0; ii<pIdxInfo->n_ترتيب_By; ii++){
    بنية sqlite3_اشاريو_orderby ثابت *p_ترتيب = &pIdxInfo->a_ترتيب_By[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_ڳڻپ(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_ترتيب->i_ڪالم));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(p_ترتيب->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ڳڻپ(pElem);
  }

  Tcl_سيٽ_Obj_نتيجو(interp, pRes);
  Tcl_Decr_Ref_ڳڻپ(pRes);
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
ساكن صحيح SQLITE_TCLAPI test_بهترين_اشاريو_Obj(
  Client_معلومات client_معلومات, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
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
  صحيح ii;
  sqlite3_اشاريو_info *pIdxInfo = (sqlite3_اشاريو_info*)client_معلومات;

  إذا( objc<2 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "SUB-COMMAND");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_Get_اشاريو_From_Obj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    أرجع TCL_ERROR;
  }

  إذا( ii<4 && objc!=2 ){
    Tcl_Wrong_انگ_دليلون(interp, 2, objv, "");
    أرجع TCL_ERROR;
  }
  إذا( ii==4 && objc!=4 ){
    Tcl_Wrong_انگ_دليلون(interp, 2, objv, "INDEX BOOLEAN");
    أرجع TCL_ERROR;
  }
  إذا( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_انگ_دليلون(interp, 2, objv, "INDEX ?DEFAULT?");
    أرجع TCL_ERROR;
  }

  بدل( ii ){
    حالة 0: أكد( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      test_بهترين_اشاريو_Obj_Constraints(interp, pIdxInfo);
      اكسر;

    حالة 1: أكد( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      test_بهترين_اشاريو_Obj_Orderby(interp, pIdxInfo);
      اكسر;

    حالة 2: أكد( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewWideIntObj(pIdxInfo->کانو_استعمال_ٿيل));
      اكسر;

    حالة 3: أكد( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      صحيح bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(bDistinct));
      اكسر;
    }

    حالة 4: أكد( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      صحيح iCons;
      صحيح bHandle;
      إذا( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        أرجع TCL_ERROR;
      }
      Tcl_سيٽ_Obj_نتيجو(interp, 
          Tcl_NewIntObj(sqlite3_vtab_اندريون(pIdxInfo, iCons, bHandle))
      );
      اكسر;
    }

    حالة 5: أكد( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      صحيح iCons = 0;
      صحيح rc;
      sqlite3_قدر *pVal = 0;
      ثابت حرف *zVal = "";
      إذا( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        أرجع TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_قدر(pIdxInfo, iCons, &pVal);
      إذا( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_سيٽ_نتيجو(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
        أرجع TCL_ERROR;
      }
      إذا( pVal ){
        zVal = (ثابت حرف*)sqlite3_قدر_متن(pVal);
      }وإلا إذا( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewStringObj(zVal, -1));
      اكسر;
    }

    حالة 6: أكد( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      صحيح iCons = 0;
      ثابت حرف *zColl = "";
      إذا( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        أرجع TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewStringObj(zColl, -1));
      اكسر;
    }
  }

  أرجع TCL_OK;
}

ساكن صحيح tcl_بهترين_اشاريو(sqlite3_vtab *tab, sqlite3_اشاريو_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  صحيح rc = SQLITE_OK;

  ساكن صحيح i_ايندڙ = 43;
  حرف zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ڳڻپ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(حجم(zHdl), zHdl, "bestindex%d", i_ايندڙ++);
  Tcl_CreateObjCommand(interp, zHdl, test_بهترين_اشاريو_Obj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_ڳڻپ(pScript);

  إذا( rc!=TCL_OK ){
    ثابت حرف *zErr = Tcl_Get_String_نتيجو(interp);
    rc = SQLITE_ERROR;
    pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("%s", zErr);
  }وإلا{
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
    Tcl_Obj *pRes = Tcl_Get_Obj_نتيجو(interp);
    Tcl_Obj **apElem = 0;
    Tcl_سائيز nElem;
    rc = Tcl_ListObjGetElements(interp, pRes, &nElem, &apElem);
    إذا( rc!=TCL_OK ){
      ثابت حرف *zErr = Tcl_Get_String_نتيجو(interp);
      rc = SQLITE_ERROR;
      pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("%s", zErr);
    }وإلا{
      صحيح ii;
      صحيح iArgv = 1;
      لكل(ii=0; rc==SQLITE_OK && ii<(صحيح)nElem; ii+=2){
        ثابت حرف *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        إذا( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimated_لاڳت);
        }وإلا
        إذا( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->ترتيب_By_Consumed);
        }وإلا
        إذا( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_انگ);
        }وإلا
        إذا( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->need_منزل_Free_Idx_Str = 1;
        }وإلا
        إذا( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimated_صفون = (tRowcnt)x;
        }وإلا
        إذا( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          صحيح iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          إذا( rc==SQLITE_OK ){
            إذا( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("unexpected: %d", iCons);
            }وإلا{
              صحيح bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argv_اشاريو = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }وإلا
        إذا( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("%s", Tcl_GetString(p));
        }وإلا{
          rc = SQLITE_ERROR;
          pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        إذا( rc!=SQLITE_OK && pTab->بنياد.z_Err_نياپو==0 ){
          ثابت حرف *zErr = Tcl_Get_String_نتيجو(interp);
          pTab->بنياد.z_Err_نياپو = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  أرجع rc;
}

ساكن فراغ tclFunction(sqlite3_context *pCtx, صحيح nArg, sqlite3_قدر **apArg){
  Test_ڳوليو_Function *p = (Test_ڳوليو_Function*)sqlite3_user_معلومات(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  صحيح ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_ڳڻپ(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  لكل(ii=0; ii<nArg; ii++){
    ثابت حرف *zArg = (ثابت حرف*)sqlite3_قدر_متن(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ڳڻپ(pScript);

  pRet = Tcl_Get_Obj_نتيجو(interp);
  sqlite3_نتيجو_متن(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

ساكن صحيح tcl_ڳوليو_Function(
  sqlite3_vtab *tab, 
  صحيح nArg, 
  ثابت حرف *zName,
  فراغ (**pxFunc)(sqlite3_context*,صحيح,sqlite3_قدر**),   /*ٻاهريون*/
  فراغ **ppArg                                             /*ٻاهريون*/
){
  صحيح iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  صحيح rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ڳڻپ(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ڳڻپ(pScript);

  إذا( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_Get_Obj_نتيجو(interp);

    إذا( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }وإلا إذا( iRet>0 ){
      sqlite3_int64 nName = لڙي_ڊيگهه(zName);
      sqlite3_int64 nByte = nName + 1 + حجم(Test_ڳوليو_Function);
      Test_ڳوليو_Function *pNew = 0;

      pNew = (Test_ڳوليو_Function*)sqlite3_malloc64(nByte);
      إذا( pNew==0 ){
        iRet = 0;
      }وإلا{
        memset(pNew, 0, nByte);
        pNew->zName = (ثابت حرف*)&pNew[1];
        يادگيري_نقل((حرف*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->p_ايندڙ = pTab->p_ڳوليو_Function_List;
        pTab->p_ڳوليو_Function_List = pNew;
        *ppArg = (فراغ*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  أرجع iRet;
}

ساكن صحيح tclUpdate(
  sqlite3_vtab *tab, 
  صحيح nArg, 
  sqlite3_قدر **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  صحيح rc = TCL_OK;

  Tcl_Incr_Ref_ڳڻپ(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ڳڻپ(pEval);

  إذا( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_Get_Obj_نتيجو(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  إذا( rc!=TCL_OK ){
    tab->z_Err_نياپو = sqlite3_mprintf("%s", Tcl_Get_String_نتيجو(pTab->interp));
    أرجع rc;
  }

  أرجع SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ساكن sqlite3_module tclModule = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_بهترين_اشاريو,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_ايندڙ,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_ڪالم,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_ڳوليو_Function,             /* xFindFunction */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};
ساكن sqlite3_module tclModuleUpdate = {
  0,                         /* iVersion */
  tclConnect,
  tclConnect,
  tcl_بهترين_اشاريو,
  tclDisconnect, 
  tclDisconnect,
  tclOpen,                      /* xOpen - open a cursor */
  tclClose,                     /* xClose - close a cursor */
  tclFilter,                    /* xFilter - configure scan constraints */
  tcl_ايندڙ,                      /* xNext - advance a cursor */
  tclEof,                       /* xEof - check for end of scan */
  tcl_ڪالم,                    /* xColumn - read data */
  tclRowid,                     /* xRowid - read data */
  tclUpdate,                   /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  tcl_ڳوليو_Function,             /* xFindFunction */
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
خارجي صحيح get_Db_اشارو(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

ساكن فراغ delTestVtabCtx(فراغ *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  إذا( pCtx->pDefault ){
    Tcl_Decr_Ref_ڳڻپ(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
ساكن صحيح SQLITE_TCLAPI رجسٽر_tcl_module(
  Client_معلومات client_معلومات, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  إذا( objc!=2 && objc!=3 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "DB ?DEFAULT-CMD?");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشارو(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(حجم(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    إذا( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_ڳڻپ(pCtx->pDefault);
    }

    إذا( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (فراغ*)pCtx, delTestVtabCtx);
  }
#جيڪڏهن_پورو
  أرجع TCL_OK;
}

#جيڪڏهن_پورو


/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetesttcl_Init(Tcl_Interp *interp){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     فراغ *client_معلومات;
  } aObjCmd[] = {
     { "register_tcl_module",   رجسٽر_tcl_module, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_معلومات, 0);
  }
#جيڪڏهن_پورو
  أرجع TCL_OK;
}
