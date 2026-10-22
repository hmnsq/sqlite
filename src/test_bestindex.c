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


#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE


प्रकारव्याख्या संरचना tcl_vtab tcl_vtab;
प्रकारव्याख्या संरचना tcl_cursor tcl_cursor;
प्रकारव्याख्या संरचना TestFindFunction TestFindFunction;
प्रकारव्याख्या संरचना TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
संरचना tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
संरचना tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

संरचना TestFindFunction {
  tcl_vtab *pTab;
  स्थिरांक अक्षर *zName;
  TestFindFunction *pNext;
};

संरचना TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
स्थिर शून्य tclDequote(अक्षर *z){
  अक्षर q = z[0];

  /* Set stack variable q to the close-quote character */
  जर( q=='[' || q=='\'' || q=='"' || q=='`' ){
    पूर्णांक iIn = 1;
    पूर्णांक iOut = 0;
    जर( q=='[' ) q = ']';  

    पर्यंत( ALWAYS(z[iIn]) ){
      जर( z[iIn]==q ){
        जर( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          थांबा;
        }अन्यथा{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }अन्यथा{
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
स्थिर पूर्णांक tclConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  अक्षर *zCmd = 0;
  Tcl_Obj *pScript = 0;
  पूर्णांक rc = SQLITE_OK;

  जर( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    परत SQLITE_ERROR;
  }

  जर( argc==4 ){
    zCmd = sqlite3_malloc64(म_माप(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(आकार(tcl_vtab));
  जर( (zCmd || argc==3) && pTab ){
    स_रखो(pTab, 0, आकार(tcl_vtab));

    जर( zCmd ){
      स_नकल(zCmd, argv[3], म_माप(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }अन्यथा{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_मोजणी(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_मोजणी(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    जर( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_GetStringResult(interp));
      जर( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }अन्यथा{
        rc = SQLITE_ERROR;
      }
    }अन्यथा{
      rc = sqlite3_declare_vtab(db, Tcl_GetStringResult(interp));
      जर( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    जर( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }अन्यथा{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  परत rc;
}

/* The xDisconnect and xDestroy methods are also the same */
स्थिर पूर्णांक tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  पर्यंत( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->pNext;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_मोजणी(pTab->pCmd);
  sqlite3_free(pTab);
  परत SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
स्थिर पूर्णांक tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(आकार(tcl_cursor));
  जर( pCur==0 ) परत SQLITE_NOMEM;
  स_रखो(pCur, 0, आकार(tcl_cursor));
  *ppCursor = &pCur->base;
  परत SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
स्थिर पूर्णांक tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  जर( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  परत SQLITE_OK;
}

स्थिर पूर्णांक tclNext(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  जर( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    पूर्णांक rc = sqlite3_step(pCsr->pStmt);
    जर( rc!=SQLITE_ROW ){
      स्थिरांक अक्षर *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      जर( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.z_Err_संदेश = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  परत SQLITE_OK;
}

स्थिर पूर्णांक tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिरांक अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  पूर्णांक ii;
  पूर्णांक rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_मोजणी(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_अंक));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_मोजणी(pArg);
  साठी(ii=0; ii<argc; ii++){
    स्थिरांक अक्षर *zVal = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[ii]);
    Tcl_Obj *pVal;
    जर( zVal==0 ){
      sqlite3_मूल्य *pMem;
      pVal = Tcl_NewObj();
      साठी(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_next(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(pMem);
        जर( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }अन्यथा{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }अन्यथा{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_मोजणी(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  जर( rc!=TCL_OK ){
    स्थिरांक अक्षर *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_संदेश = sqlite3_mprintf("%s", zErr);
  }अन्यथा{
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
    जर( rc!=TCL_OK ){
      स्थिरांक अक्षर *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_संदेश = sqlite3_mprintf("%s", zErr);
    }अन्यथा{
      साठी(ii=0; rc==SQLITE_OK && ii<(पूर्णांक)nElem; ii+=2){
        स्थिरांक अक्षर *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        जर( sqlite3_stricmp("sql", zCmd)==0 ){
          स्थिरांक अक्षर *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          जर( rc!=SQLITE_OK ){
            स्थिरांक अक्षर *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.z_Err_संदेश = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }अन्यथा{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_संदेश = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  जर( rc==SQLITE_OK ){
    rc = tclNext(pVtabCursor);
  }
  परत rc;
}

स्थिर पूर्णांक tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  पूर्णांक i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_result_मूल्य(ctx, sqlite3_column_मूल्य(pCsr->pStmt, i+1));
  परत SQLITE_OK;
}

स्थिर पूर्णांक tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  परत SQLITE_OK;
}

स्थिर पूर्णांक tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  परत (pCsr->pStmt==0);
}

स्थिर शून्य testBestIndexObjConstraints(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  पूर्णांक ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_मोजणी(pRes);
  साठी(ii=0; ii<pIdxInfo->nConstraint; ii++){
    संरचना sqlite3_index_constraint स्थिरांक *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    स्थिरांक अक्षर *zOp = 0;

    Tcl_Incr_Ref_मोजणी(pElem);

    निवड( pCons->op ){
      स्थिती SQLITE_INDEX_CONSTRAINT_EQ:
        zOp = "eq"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_GT:
        zOp = "gt"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_LE:
        zOp = "le"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_LT:
        zOp = "lt"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_GE:
        zOp = "ge"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_MATCH:
        zOp = "match"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_LIKE:
        zOp = "like"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_GLOB:
        zOp = "glob"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_REGEXP:
        zOp = "regexp"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_NE:
        zOp = "ne"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_ISNOT:
        zOp = "isnot"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_ISNULL:
        zOp = "isnull"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_IS:
        zOp = "is"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_LIMIT:
        zOp = "limit"; थांबा;
      स्थिती SQLITE_INDEX_CONSTRAINT_OFFSET:
        zOp = "offset"; थांबा;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    जर( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }अन्यथा{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_मोजणी(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_मोजणी(pRes);
}

स्थिर शून्य testBestIndexObjOrderby(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  पूर्णांक ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_मोजणी(pRes);
  साठी(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    संरचना sqlite3_index_orderby स्थिरांक *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_मोजणी(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_मोजणी(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_मोजणी(pRes);
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
स्थिर पूर्णांक SQLITE_TCLAPI testBestIndexObj(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  स्थिरांक अक्षर *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  पूर्णांक ii;
  sqlite3_index_info *pIdxInfo = (sqlite3_index_info*)clientData;

  जर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIndexFromObj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    परत TCL_ERROR;
  }

  जर( ii<4 && objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    परत TCL_ERROR;
  }
  जर( ii==4 && objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "INDEX BOOLEAN");
    परत TCL_ERROR;
  }
  जर( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "INDEX ?DEFAULT?");
    परत TCL_ERROR;
  }

  निवड( ii ){
    स्थिती 0: प्रतिज्ञा( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      testBestIndexObjConstraints(interp, pIdxInfo);
      थांबा;

    स्थिती 1: प्रतिज्ञा( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      testBestIndexObjOrderby(interp, pIdxInfo);
      थांबा;

    स्थिती 2: प्रतिज्ञा( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_SetObjResult(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      थांबा;

    स्थिती 3: प्रतिज्ञा( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      पूर्णांक bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_SetObjResult(interp, Tcl_NewIntObj(bDistinct));
      थांबा;
    }

    स्थिती 4: प्रतिज्ञा( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      पूर्णांक iCons;
      पूर्णांक bHandle;
      जर( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        परत TCL_ERROR;
      }
      Tcl_SetObjResult(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      थांबा;
    }

    स्थिती 5: प्रतिज्ञा( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      पूर्णांक iCons = 0;
      पूर्णांक rc;
      sqlite3_मूल्य *pVal = 0;
      स्थिरांक अक्षर *zVal = "";
      जर( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        परत TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_मूल्य(pIdxInfo, iCons, &pVal);
      जर( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
        परत TCL_ERROR;
      }
      जर( pVal ){
        zVal = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(pVal);
      }अन्यथा जर( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zVal, -1));
      थांबा;
    }

    स्थिती 6: प्रतिज्ञा( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      पूर्णांक iCons = 0;
      स्थिरांक अक्षर *zColl = "";
      जर( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        परत TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zColl, -1));
      थांबा;
    }
  }

  परत TCL_OK;
}

स्थिर पूर्णांक tclBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  पूर्णांक rc = SQLITE_OK;

  स्थिर पूर्णांक iNext = 43;
  अक्षर zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_मोजणी(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(आकार(zHdl), zHdl, "bestindex%d", iNext++);
  Tcl_CreateObjCommand(interp, zHdl, testBestIndexObj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_मोजणी(pScript);

  जर( rc!=TCL_OK ){
    स्थिरांक अक्षर *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_संदेश = sqlite3_mprintf("%s", zErr);
  }अन्यथा{
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
    जर( rc!=TCL_OK ){
      स्थिरांक अक्षर *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_संदेश = sqlite3_mprintf("%s", zErr);
    }अन्यथा{
      पूर्णांक ii;
      पूर्णांक iArgv = 1;
      साठी(ii=0; rc==SQLITE_OK && ii<(पूर्णांक)nElem; ii+=2){
        स्थिरांक अक्षर *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        जर( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }अन्यथा
        जर( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }अन्यथा
        जर( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_अंक);
        }अन्यथा
        जर( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }अन्यथा
        जर( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }अन्यथा
        जर( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          पूर्णांक iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          जर( rc==SQLITE_OK ){
            जर( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.z_Err_संदेश = sqlite3_mprintf("unexpected: %d", iCons);
            }अन्यथा{
              पूर्णांक bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argvIndex = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }अन्यथा
        जर( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.z_Err_संदेश = sqlite3_mprintf("%s", Tcl_GetString(p));
        }अन्यथा{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_संदेश = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        जर( rc!=SQLITE_OK && pTab->base.z_Err_संदेश==0 ){
          स्थिरांक अक्षर *zErr = Tcl_GetStringResult(interp);
          pTab->base.z_Err_संदेश = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  परत rc;
}

स्थिर शून्य tclFunction(sqlite3_context *pCtx, पूर्णांक nArg, sqlite3_मूल्य **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_data(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  पूर्णांक ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_मोजणी(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  साठी(ii=0; ii<nArg; ii++){
    स्थिरांक अक्षर *zArg = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_मोजणी(pScript);

  pRet = Tcl_GetObjResult(interp);
  sqlite3_result_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

स्थिर पूर्णांक tclFindFunction(
  sqlite3_vtab *tab, 
  पूर्णांक nArg, 
  स्थिरांक अक्षर *zName,
  शून्य (**pxFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**),   /* OUT */
  शून्य **ppArg                                             /* OUT */
){
  पूर्णांक iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  पूर्णांक rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_मोजणी(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_मोजणी(pScript);

  जर( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_GetObjResult(interp);

    जर( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }अन्यथा जर( iRet>0 ){
      sqlite3_int64 nName = म_माप(zName);
      sqlite3_int64 nByte = nName + 1 + आकार(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      जर( pNew==0 ){
        iRet = 0;
      }अन्यथा{
        स_रखो(pNew, 0, nByte);
        pNew->zName = (स्थिरांक अक्षर*)&pNew[1];
        स_नकल((अक्षर*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->pNext = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (शून्य*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  परत iRet;
}

स्थिर पूर्णांक tclUpdate(
  sqlite3_vtab *tab, 
  पूर्णांक nArg, 
  sqlite3_मूल्य **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  पूर्णांक rc = TCL_OK;

  Tcl_Incr_Ref_मोजणी(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_मोजणी(pEval);

  जर( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  जर( rc!=TCL_OK ){
    tab->z_Err_संदेश = sqlite3_mprintf("%s", Tcl_GetStringResult(pTab->interp));
    परत rc;
  }

  परत SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
स्थिर sqlite3_module tclModule = {
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
स्थिर sqlite3_module tclModuleUpdate = {
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
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिरांक अक्षर *zA, sqlite3 **ppDb);

स्थिर शून्य delTestVtabCtx(शून्य *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  जर( pCtx->pDefault ){
    Tcl_Decr_Ref_मोजणी(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
स्थिर पूर्णांक SQLITE_TCLAPI register_tcl_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  जर( objc!=2 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB ?DEFAULT-CMD?");
    परत TCL_ERROR;
  }
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(आकार(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    जर( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_मोजणी(pCtx->pDefault);
    }

    जर( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (शून्य*)pCtx, delTestVtabCtx);
  }
#endif
  परत TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  स्थिर संरचना {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *clientData;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  पूर्णांक i;
  साठी(i=0; i<आकार(aObjCmd)/आकार(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  परत TCL_OK;
}
