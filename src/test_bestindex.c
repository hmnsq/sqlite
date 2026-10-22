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

#ifndef SQLITE_OMIT_VIRTUALTABLE


تعریفنوع ساختار tcl_vtab tcl_vtab;
تعریفنوع ساختار tcl_cursor tcl_cursor;
تعریفنوع ساختار TestFindFunction TestFindFunction;
تعریفنوع ساختار TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
ساختار tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
ساختار tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

ساختار TestFindFunction {
  tcl_vtab *pTab;
  ثابت نویسه *zName;
  TestFindFunction *pNext;
};

ساختار TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
ایستا تهی tclDequote(نویسه *z){
  نویسه q = z[0];

  /* Set stack variable q to the close-quote character */
  اگر( q=='[' || q=='\'' || q=='"' || q=='`' ){
    صحیح iIn = 1;
    صحیح iOut = 0;
    اگر( q=='[' ) q = ']';  

    وقتی( ALWAYS(z[iIn]) ){
      اگر( z[iIn]==q ){
        اگر( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          بشکن;
        }وگرنه{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }وگرنه{
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
ایستا صحیح tclConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  نویسه *zCmd = 0;
  Tcl_Obj *pScript = 0;
  صحیح rc = SQLITE_OK;

  اگر( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    بازگشت SQLITE_ERROR;
  }

  اگر( argc==4 ){
    zCmd = sqlite3_malloc64(strlen(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(اندازه(tcl_vtab));
  اگر( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, اندازه(tcl_vtab));

    اگر( zCmd ){
      memcpy(zCmd, argv[3], strlen(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }وگرنه{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_شمار(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_شمار(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    اگر( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_GetStringResult(interp));
      اگر( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }وگرنه{
        rc = SQLITE_ERROR;
      }
    }وگرنه{
      rc = sqlite3_declare_vtab(db, Tcl_GetStringResult(interp));
      اگر( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    اگر( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }وگرنه{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  بازگشت rc;
}

/* The xDisconnect and xDestroy methods are also the same */
ایستا صحیح tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  وقتی( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->pNext;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_شمار(pTab->pCmd);
  sqlite3_free(pTab);
  بازگشت SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
ایستا صحیح tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(اندازه(tcl_cursor));
  اگر( pCur==0 ) بازگشت SQLITE_NOMEM;
  memset(pCur, 0, اندازه(tcl_cursor));
  *ppCursor = &pCur->base;
  بازگشت SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
ایستا صحیح tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  اگر( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  بازگشت SQLITE_OK;
}

ایستا صحیح tclNext(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  اگر( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    صحیح rc = sqlite3_step(pCsr->pStmt);
    اگر( rc!=SQLITE_ROW ){
      ثابت نویسه *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      اگر( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.z_Err_پیام = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  بازگشت SQLITE_OK;
}

ایستا صحیح tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  صحیح ii;
  صحیح rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_شمار(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_رقم));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_شمار(pArg);
  برای(ii=0; ii<argc; ii++){
    ثابت نویسه *zVal = (ثابت نویسه*)sqlite3_مقدار_text(argv[ii]);
    Tcl_Obj *pVal;
    اگر( zVal==0 ){
      sqlite3_مقدار *pMem;
      pVal = Tcl_NewObj();
      برای(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_next(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (ثابت نویسه*)sqlite3_مقدار_text(pMem);
        اگر( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }وگرنه{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }وگرنه{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_شمار(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  اگر( rc!=TCL_OK ){
    ثابت نویسه *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_پیام = sqlite3_mprintf("%s", zErr);
  }وگرنه{
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
    اگر( rc!=TCL_OK ){
      ثابت نویسه *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_پیام = sqlite3_mprintf("%s", zErr);
    }وگرنه{
      برای(ii=0; rc==SQLITE_OK && ii<(صحیح)nElem; ii+=2){
        ثابت نویسه *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        اگر( sqlite3_stricmp("sql", zCmd)==0 ){
          ثابت نویسه *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          اگر( rc!=SQLITE_OK ){
            ثابت نویسه *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.z_Err_پیام = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }وگرنه{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_پیام = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  اگر( rc==SQLITE_OK ){
    rc = tclNext(pVtabCursor);
  }
  بازگشت rc;
}

ایستا صحیح tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  صحیح i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_result_مقدار(ctx, sqlite3_column_مقدار(pCsr->pStmt, i+1));
  بازگشت SQLITE_OK;
}

ایستا صحیح tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  بازگشت SQLITE_OK;
}

ایستا صحیح tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  بازگشت (pCsr->pStmt==0);
}

ایستا تهی testBestIndexObjConstraints(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  صحیح ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_شمار(pRes);
  برای(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساختار sqlite3_index_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    ثابت نویسه *zOp = 0;

    Tcl_Incr_Ref_شمار(pElem);

    انتخاب( pCons->op ){
      حالت SQLITE_INDEX_CONSTRAINT_EQ:
        zOp = "eq"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_GT:
        zOp = "gt"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_LE:
        zOp = "le"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_LT:
        zOp = "lt"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_GE:
        zOp = "ge"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_MATCH:
        zOp = "match"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_LIKE:
        zOp = "like"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_GLOB:
        zOp = "glob"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_REGEXP:
        zOp = "regexp"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_NE:
        zOp = "ne"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_ISNOT:
        zOp = "isnot"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_ISNULL:
        zOp = "isnull"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_IS:
        zOp = "is"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_LIMIT:
        zOp = "limit"; بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_OFFSET:
        zOp = "offset"; بشکن;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    اگر( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }وگرنه{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_شمار(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_شمار(pRes);
}

ایستا تهی testBestIndexObjOrderby(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  صحیح ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_شمار(pRes);
  برای(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    ساختار sqlite3_index_orderby ثابت *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_شمار(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_شمار(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_شمار(pRes);
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
ایستا صحیح SQLITE_TCLAPI testBestIndexObj(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ثابت نویسه *azSub[] = {
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
  sqlite3_index_info *pIdxInfo = (sqlite3_index_info*)clientData;

  اگر( objc<2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SUB-COMMAND");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIndexFromObj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    بازگشت TCL_ERROR;
  }

  اگر( ii<4 && objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "");
    بازگشت TCL_ERROR;
  }
  اگر( ii==4 && objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "INDEX BOOLEAN");
    بازگشت TCL_ERROR;
  }
  اگر( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "INDEX ?DEFAULT?");
    بازگشت TCL_ERROR;
  }

  انتخاب( ii ){
    حالت 0: أكد( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      testBestIndexObjConstraints(interp, pIdxInfo);
      بشکن;

    حالت 1: أكد( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      testBestIndexObjOrderby(interp, pIdxInfo);
      بشکن;

    حالت 2: أكد( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_SetObjResult(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      بشکن;

    حالت 3: أكد( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      صحیح bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_SetObjResult(interp, Tcl_NewIntObj(bDistinct));
      بشکن;
    }

    حالت 4: أكد( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      صحیح iCons;
      صحیح bHandle;
      اگر( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        بازگشت TCL_ERROR;
      }
      Tcl_SetObjResult(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      بشکن;
    }

    حالت 5: أكد( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      صحیح iCons = 0;
      صحیح rc;
      sqlite3_مقدار *pVal = 0;
      ثابت نویسه *zVal = "";
      اگر( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        بازگشت TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_مقدار(pIdxInfo, iCons, &pVal);
      اگر( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
        بازگشت TCL_ERROR;
      }
      اگر( pVal ){
        zVal = (ثابت نویسه*)sqlite3_مقدار_text(pVal);
      }وگرنه اگر( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zVal, -1));
      بشکن;
    }

    حالت 6: أكد( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      صحیح iCons = 0;
      ثابت نویسه *zColl = "";
      اگر( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        بازگشت TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zColl, -1));
      بشکن;
    }
  }

  بازگشت TCL_OK;
}

ایستا صحیح tclBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  صحیح rc = SQLITE_OK;

  ایستا صحیح iNext = 43;
  نویسه zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_شمار(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(اندازه(zHdl), zHdl, "bestindex%d", iNext++);
  Tcl_CreateObjCommand(interp, zHdl, testBestIndexObj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_شمار(pScript);

  اگر( rc!=TCL_OK ){
    ثابت نویسه *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_پیام = sqlite3_mprintf("%s", zErr);
  }وگرنه{
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
    اگر( rc!=TCL_OK ){
      ثابت نویسه *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_پیام = sqlite3_mprintf("%s", zErr);
    }وگرنه{
      صحیح ii;
      صحیح iArgv = 1;
      برای(ii=0; rc==SQLITE_OK && ii<(صحیح)nElem; ii+=2){
        ثابت نویسه *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        اگر( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }وگرنه
        اگر( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }وگرنه
        اگر( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_رقم);
        }وگرنه
        اگر( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }وگرنه
        اگر( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }وگرنه
        اگر( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          صحیح iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          اگر( rc==SQLITE_OK ){
            اگر( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.z_Err_پیام = sqlite3_mprintf("unexpected: %d", iCons);
            }وگرنه{
              صحیح bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argvIndex = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }وگرنه
        اگر( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.z_Err_پیام = sqlite3_mprintf("%s", Tcl_GetString(p));
        }وگرنه{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_پیام = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        اگر( rc!=SQLITE_OK && pTab->base.z_Err_پیام==0 ){
          ثابت نویسه *zErr = Tcl_GetStringResult(interp);
          pTab->base.z_Err_پیام = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  بازگشت rc;
}

ایستا تهی tclFunction(sqlite3_context *pCtx, صحیح nArg, sqlite3_مقدار **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_data(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  صحیح ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_شمار(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  برای(ii=0; ii<nArg; ii++){
    ثابت نویسه *zArg = (ثابت نویسه*)sqlite3_مقدار_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_شمار(pScript);

  pRet = Tcl_GetObjResult(interp);
  sqlite3_result_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

ایستا صحیح tclFindFunction(
  sqlite3_vtab *tab, 
  صحیح nArg, 
  ثابت نویسه *zName,
  تهی (**pxFunc)(sqlite3_context*,صحیح,sqlite3_مقدار**),   /* OUT */
  تهی **ppArg                                             /* OUT */
){
  صحیح iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  صحیح rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_شمار(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_شمار(pScript);

  اگر( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_GetObjResult(interp);

    اگر( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }وگرنه اگر( iRet>0 ){
      sqlite3_int64 nName = strlen(zName);
      sqlite3_int64 nByte = nName + 1 + اندازه(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      اگر( pNew==0 ){
        iRet = 0;
      }وگرنه{
        memset(pNew, 0, nByte);
        pNew->zName = (ثابت نویسه*)&pNew[1];
        memcpy((نویسه*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->pNext = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (تهی*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  بازگشت iRet;
}

ایستا صحیح tclUpdate(
  sqlite3_vtab *tab, 
  صحیح nArg, 
  sqlite3_مقدار **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  صحیح rc = TCL_OK;

  Tcl_Incr_Ref_شمار(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_شمار(pEval);

  اگر( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  اگر( rc!=TCL_OK ){
    tab->z_Err_پیام = sqlite3_mprintf("%s", Tcl_GetStringResult(pTab->interp));
    بازگشت rc;
  }

  بازگشت SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ایستا sqlite3_module tclModule = {
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
ایستا sqlite3_module tclModuleUpdate = {
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
بیرونی صحیح getDbPointer(Tcl_Interp *interp, ثابت نویسه *zA, sqlite3 **ppDb);

ایستا تهی delTestVtabCtx(تهی *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  اگر( pCtx->pDefault ){
    Tcl_Decr_Ref_شمار(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
ایستا صحیح SQLITE_TCLAPI register_tcl_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB ?DEFAULT-CMD?");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(اندازه(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    اگر( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_شمار(pCtx->pDefault);
    }

    اگر( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (تهی*)pCtx, delTestVtabCtx);
  }
#endif
  بازگشت TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
     تهی *clientData;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  صحیح i;
  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  بازگشت TCL_OK;
}
