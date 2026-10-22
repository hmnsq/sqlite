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


#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE


வகைவரை கட்டமைப்பு tcl_vtab tcl_vtab;
வகைவரை கட்டமைப்பு tcl_cursor tcl_cursor;
வகைவரை கட்டமைப்பு TestFindFunction TestFindFunction;
வகைவரை கட்டமைப்பு TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
கட்டமைப்பு tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
கட்டமைப்பு tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

கட்டமைப்பு TestFindFunction {
  tcl_vtab *pTab;
  மாறிலி எழுத்து *zName;
  TestFindFunction *pNext;
};

கட்டமைப்பு TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
நிலையான வெறுமை tclDequote(எழுத்து *z){
  எழுத்து q = z[0];

  /* Set stack variable q to the close-quote character */
  எனில்( q=='[' || q=='\'' || q=='"' || q=='`' ){
    முழு iIn = 1;
    முழு iOut = 0;
    எனில்( q=='[' ) q = ']';  

    போது( ALWAYS(z[iIn]) ){
      எனில்( z[iIn]==q ){
        எனில்( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          நிறுத்து;
        }இல்லையெனில்{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }இல்லையெனில்{
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
நிலையான முழு tclConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  எழுத்து *zCmd = 0;
  Tcl_Obj *pScript = 0;
  முழு rc = SQLITE_OK;

  எனில்( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    திரும்பு SQLITE_ERROR;
  }

  எனில்( argc==4 ){
    zCmd = sqlite3_malloc64(strlen(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(அளவு(tcl_vtab));
  எனில்( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, அளவு(tcl_vtab));

    எனில்( zCmd ){
      memcpy(zCmd, argv[3], strlen(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }இல்லையெனில்{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_மொத்தம்(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_மொத்தம்(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    எனில்( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_GetStringResult(interp));
      எனில்( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }இல்லையெனில்{
        rc = SQLITE_ERROR;
      }
    }இல்லையெனில்{
      rc = sqlite3_declare_vtab(db, Tcl_GetStringResult(interp));
      எனில்( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    எனில்( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }இல்லையெனில்{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  திரும்பு rc;
}

/* The xDisconnect and xDestroy methods are also the same */
நிலையான முழு tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  போது( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->pNext;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_மொத்தம்(pTab->pCmd);
  sqlite3_free(pTab);
  திரும்பு SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
நிலையான முழு tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(அளவு(tcl_cursor));
  எனில்( pCur==0 ) திரும்பு SQLITE_NOMEM;
  memset(pCur, 0, அளவு(tcl_cursor));
  *ppCursor = &pCur->base;
  திரும்பு SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
நிலையான முழு tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  எனில்( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  திரும்பு SQLITE_OK;
}

நிலையான முழு tclNext(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  எனில்( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    முழு rc = sqlite3_step(pCsr->pStmt);
    எனில்( rc!=SQLITE_ROW ){
      மாறிலி எழுத்து *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      எனில்( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.z_Err_செய்தி = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  திரும்பு SQLITE_OK;
}

நிலையான முழு tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  முழு ii;
  முழு rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_மொத்தம்(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_இலக்கம்));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_மொத்தம்(pArg);
  க்கு(ii=0; ii<argc; ii++){
    மாறிலி எழுத்து *zVal = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[ii]);
    Tcl_Obj *pVal;
    எனில்( zVal==0 ){
      sqlite3_மதிப்பு *pMem;
      pVal = Tcl_NewObj();
      க்கு(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_next(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(pMem);
        எனில்( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }இல்லையெனில்{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }இல்லையெனில்{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_மொத்தம்(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  எனில்( rc!=TCL_OK ){
    மாறிலி எழுத்து *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_செய்தி = sqlite3_mprintf("%s", zErr);
  }இல்லையெனில்{
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
    எனில்( rc!=TCL_OK ){
      மாறிலி எழுத்து *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_செய்தி = sqlite3_mprintf("%s", zErr);
    }இல்லையெனில்{
      க்கு(ii=0; rc==SQLITE_OK && ii<(முழு)nElem; ii+=2){
        மாறிலி எழுத்து *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        எனில்( sqlite3_stricmp("sql", zCmd)==0 ){
          மாறிலி எழுத்து *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          எனில்( rc!=SQLITE_OK ){
            மாறிலி எழுத்து *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.z_Err_செய்தி = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }இல்லையெனில்{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_செய்தி = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  எனில்( rc==SQLITE_OK ){
    rc = tclNext(pVtabCursor);
  }
  திரும்பு rc;
}

நிலையான முழு tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  முழு i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_result_மதிப்பு(ctx, sqlite3_column_மதிப்பு(pCsr->pStmt, i+1));
  திரும்பு SQLITE_OK;
}

நிலையான முழு tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  திரும்பு SQLITE_OK;
}

நிலையான முழு tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  திரும்பு (pCsr->pStmt==0);
}

நிலையான வெறுமை testBestIndexObjConstraints(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  முழு ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_மொத்தம்(pRes);
  க்கு(ii=0; ii<pIdxInfo->nConstraint; ii++){
    கட்டமைப்பு sqlite3_index_constraint மாறிலி *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    மாறிலி எழுத்து *zOp = 0;

    Tcl_Incr_Ref_மொத்தம்(pElem);

    தேர்வு( pCons->op ){
      நிலை SQLITE_INDEX_CONSTRAINT_EQ:
        zOp = "eq"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_GT:
        zOp = "gt"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_LE:
        zOp = "le"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_LT:
        zOp = "lt"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_GE:
        zOp = "ge"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_MATCH:
        zOp = "match"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_LIKE:
        zOp = "like"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_GLOB:
        zOp = "glob"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_REGEXP:
        zOp = "regexp"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_NE:
        zOp = "ne"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_ISNOT:
        zOp = "isnot"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_ISNULL:
        zOp = "isnull"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_IS:
        zOp = "is"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_LIMIT:
        zOp = "limit"; நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_OFFSET:
        zOp = "offset"; நிறுத்து;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    எனில்( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }இல்லையெனில்{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_மொத்தம்(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_மொத்தம்(pRes);
}

நிலையான வெறுமை testBestIndexObjOrderby(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  முழு ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_மொத்தம்(pRes);
  க்கு(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    கட்டமைப்பு sqlite3_index_orderby மாறிலி *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_மொத்தம்(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_மொத்தம்(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_மொத்தம்(pRes);
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
நிலையான முழு SQLITE_TCLAPI testBestIndexObj(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  மாறிலி எழுத்து *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  முழு ii;
  sqlite3_index_info *pIdxInfo = (sqlite3_index_info*)clientData;

  எனில்( objc<2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SUB-COMMAND");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIndexFromObj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    திரும்பு TCL_ERROR;
  }

  எனில்( ii<4 && objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
    திரும்பு TCL_ERROR;
  }
  எனில்( ii==4 && objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "INDEX BOOLEAN");
    திரும்பு TCL_ERROR;
  }
  எனில்( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "INDEX ?DEFAULT?");
    திரும்பு TCL_ERROR;
  }

  தேர்வு( ii ){
    நிலை 0: ப்ரதிஜ்ஞா( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      testBestIndexObjConstraints(interp, pIdxInfo);
      நிறுத்து;

    நிலை 1: ப்ரதிஜ்ஞா( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      testBestIndexObjOrderby(interp, pIdxInfo);
      நிறுத்து;

    நிலை 2: ப்ரதிஜ்ஞா( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_SetObjResult(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      நிறுத்து;

    நிலை 3: ப்ரதிஜ்ஞா( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      முழு bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_SetObjResult(interp, Tcl_NewIntObj(bDistinct));
      நிறுத்து;
    }

    நிலை 4: ப்ரதிஜ்ஞா( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      முழு iCons;
      முழு bHandle;
      எனில்( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        திரும்பு TCL_ERROR;
      }
      Tcl_SetObjResult(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      நிறுத்து;
    }

    நிலை 5: ப்ரதிஜ்ஞா( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      முழு iCons = 0;
      முழு rc;
      sqlite3_மதிப்பு *pVal = 0;
      மாறிலி எழுத்து *zVal = "";
      எனில்( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        திரும்பு TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_மதிப்பு(pIdxInfo, iCons, &pVal);
      எனில்( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
        திரும்பு TCL_ERROR;
      }
      எனில்( pVal ){
        zVal = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(pVal);
      }இல்லையெனில் எனில்( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zVal, -1));
      நிறுத்து;
    }

    நிலை 6: ப்ரதிஜ்ஞா( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      முழு iCons = 0;
      மாறிலி எழுத்து *zColl = "";
      எனில்( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        திரும்பு TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zColl, -1));
      நிறுத்து;
    }
  }

  திரும்பு TCL_OK;
}

நிலையான முழு tclBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  முழு rc = SQLITE_OK;

  நிலையான முழு iNext = 43;
  எழுத்து zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_மொத்தம்(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(அளவு(zHdl), zHdl, "bestindex%d", iNext++);
  Tcl_CreateObjCommand(interp, zHdl, testBestIndexObj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_மொத்தம்(pScript);

  எனில்( rc!=TCL_OK ){
    மாறிலி எழுத்து *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_செய்தி = sqlite3_mprintf("%s", zErr);
  }இல்லையெனில்{
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
    எனில்( rc!=TCL_OK ){
      மாறிலி எழுத்து *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_செய்தி = sqlite3_mprintf("%s", zErr);
    }இல்லையெனில்{
      முழு ii;
      முழு iArgv = 1;
      க்கு(ii=0; rc==SQLITE_OK && ii<(முழு)nElem; ii+=2){
        மாறிலி எழுத்து *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        எனில்( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }இல்லையெனில்
        எனில்( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }இல்லையெனில்
        எனில்( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_இலக்கம்);
        }இல்லையெனில்
        எனில்( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }இல்லையெனில்
        எனில்( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }இல்லையெனில்
        எனில்( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          முழு iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          எனில்( rc==SQLITE_OK ){
            எனில்( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.z_Err_செய்தி = sqlite3_mprintf("unexpected: %d", iCons);
            }இல்லையெனில்{
              முழு bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argvIndex = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }இல்லையெனில்
        எனில்( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.z_Err_செய்தி = sqlite3_mprintf("%s", Tcl_GetString(p));
        }இல்லையெனில்{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_செய்தி = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        எனில்( rc!=SQLITE_OK && pTab->base.z_Err_செய்தி==0 ){
          மாறிலி எழுத்து *zErr = Tcl_GetStringResult(interp);
          pTab->base.z_Err_செய்தி = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  திரும்பு rc;
}

நிலையான வெறுமை tclFunction(sqlite3_context *pCtx, முழு nArg, sqlite3_மதிப்பு **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_data(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  முழு ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_மொத்தம்(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  க்கு(ii=0; ii<nArg; ii++){
    மாறிலி எழுத்து *zArg = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_மொத்தம்(pScript);

  pRet = Tcl_GetObjResult(interp);
  sqlite3_result_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

நிலையான முழு tclFindFunction(
  sqlite3_vtab *tab, 
  முழு nArg, 
  மாறிலி எழுத்து *zName,
  வெறுமை (**pxFunc)(sqlite3_context*,முழு,sqlite3_மதிப்பு**),   /* OUT */
  வெறுமை **ppArg                                             /* OUT */
){
  முழு iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  முழு rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_மொத்தம்(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_மொத்தம்(pScript);

  எனில்( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_GetObjResult(interp);

    எனில்( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }இல்லையெனில் எனில்( iRet>0 ){
      sqlite3_int64 nName = strlen(zName);
      sqlite3_int64 nByte = nName + 1 + அளவு(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      எனில்( pNew==0 ){
        iRet = 0;
      }இல்லையெனில்{
        memset(pNew, 0, nByte);
        pNew->zName = (மாறிலி எழுத்து*)&pNew[1];
        memcpy((எழுத்து*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->pNext = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (வெறுமை*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  திரும்பு iRet;
}

நிலையான முழு tclUpdate(
  sqlite3_vtab *tab, 
  முழு nArg, 
  sqlite3_மதிப்பு **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  முழு rc = TCL_OK;

  Tcl_Incr_Ref_மொத்தம்(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_மொத்தம்(pEval);

  எனில்( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  எனில்( rc!=TCL_OK ){
    tab->z_Err_செய்தி = sqlite3_mprintf("%s", Tcl_GetStringResult(pTab->interp));
    திரும்பு rc;
  }

  திரும்பு SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
நிலையான sqlite3_module tclModule = {
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
நிலையான sqlite3_module tclModuleUpdate = {
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
வெளி முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);

நிலையான வெறுமை delTestVtabCtx(வெறுமை *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  எனில்( pCtx->pDefault ){
    Tcl_Decr_Ref_மொத்தம்(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
நிலையான முழு SQLITE_TCLAPI register_tcl_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  எனில்( objc!=2 && objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB ?DEFAULT-CMD?");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(அளவு(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    எனில்( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_மொத்தம்(pCtx->pDefault);
    }

    எனில்( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (வெறுமை*)pCtx, delTestVtabCtx);
  }
#endif
  திரும்பு TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
     வெறுமை *clientData;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  திரும்பு TCL_OK;
}
