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


#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"

#ifndef SQLITE_OMIT_VIRTUALTABLE


પ્રકારવ્યાખ્યા સંરચના tcl_vtab tcl_vtab;
પ્રકારવ્યાખ્યા સંરચના tcl_cursor tcl_cursor;
પ્રકારવ્યાખ્યા સંરચના TestFindFunction TestFindFunction;
પ્રકારવ્યાખ્યા સંરચના TestVtabContext TestVtabContext;

/* 
** A fs virtual-table object 
*/
સંરચના tcl_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
  Tcl_Obj *pCmd;
  TestFindFunction *pFindFunctionList;
  sqlite3 *db;
};

/* A tcl cursor object */
સંરચના tcl_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Read data from here */
};

સંરચના TestFindFunction {
  tcl_vtab *pTab;
  અચલ અક્ષર *zName;
  TestFindFunction *pNext;
};

સંરચના TestVtabContext {
  Tcl_Interp *interp;
  Tcl_Obj *pDefault;
};

/*
** Dequote string z in place.
*/
સ્થિર શૂન્ય tclDequote(અક્ષર *z){
  અક્ષર q = z[0];

  /* Set stack variable q to the close-quote character */
  જો( q=='[' || q=='\'' || q=='"' || q=='`' ){
    પૂર્ણાંક iIn = 1;
    પૂર્ણાંક iOut = 0;
    જો( q=='[' ) q = ']';  

    જ્યારે( ALWAYS(z[iIn]) ){
      જો( z[iIn]==q ){
        જો( z[iIn+1]!=q ){
          /* Character iIn was the close quote. */
          iIn++;
          તોડો;
        }નહીંતર{
          /* Character iIn and iIn+1 form an escaped quote character. Skip
          ** the input cursor past both and copy a single quote character 
          ** to the output buffer. */
          iIn += 2;
          z[iOut++] = q;
        }
      }નહીંતર{
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
સ્થિર પૂર્ણાંક tclConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  TestVtabContext *pCtx = (TestVtabContext*)pAux;
  Tcl_Interp *interp = pCtx->interp;
  tcl_vtab *pTab = 0;
  અક્ષર *zCmd = 0;
  Tcl_Obj *pScript = 0;
  પૂર્ણાંક rc = SQLITE_OK;

  જો( argc!=4 && (argc!=3 || pCtx->pDefault==0) ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    પાછા SQLITE_ERROR;
  }

  જો( argc==4 ){
    zCmd = sqlite3_malloc64(strlen(argv[3])+1);
  }
  pTab = (tcl_vtab*)sqlite3_malloc64(કદ(tcl_vtab));
  જો( (zCmd || argc==3) && pTab ){
    memset(pTab, 0, કદ(tcl_vtab));

    જો( zCmd ){
      memcpy(zCmd, argv[3], strlen(argv[3])+1);
      tclDequote(zCmd);
      pTab->pCmd = Tcl_NewStringObj(zCmd, -1);
    }નહીંતર{
      pTab->pCmd = Tcl_DuplicateObj(pCtx->pDefault);
    }

    pTab->interp = interp;
    pTab->db = db;
    Tcl_Incr_Ref_ગણતરી(pTab->pCmd);

    pScript = Tcl_DuplicateObj(pTab->pCmd);
    Tcl_Incr_Ref_ગણતરી(pScript);
    Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xConnect", -1));

    rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
    જો( rc!=TCL_OK ){
      *pzErr = sqlite3_mprintf("%s", Tcl_GetStringResult(interp));
      જો( sqlite3_stricmp(*pzErr, "database schema has changed")==0 ){
        rc = SQLITE_SCHEMA;
      }નહીંતર{
        rc = SQLITE_ERROR;
      }
    }નહીંતર{
      rc = sqlite3_declare_vtab(db, Tcl_GetStringResult(interp));
      જો( rc!=SQLITE_OK ){
        *pzErr = sqlite3_mprintf("declare_vtab: %s", sqlite3_errmsg(db));
      }
    }

    જો( rc!=SQLITE_OK ){
      sqlite3_free(pTab);
      pTab = 0;
    }
  }નહીંતર{
    rc = SQLITE_NOMEM;
  }

  sqlite3_free(zCmd);
  *ppVtab = pTab ? &pTab->base : 0;
  પાછા rc;
}

/* The xDisconnect and xDestroy methods are also the same */
સ્થિર પૂર્ણાંક tclDisconnect(sqlite3_vtab *pVtab){
  tcl_vtab *pTab = (tcl_vtab*)pVtab;
  જ્યારે( pTab->pFindFunctionList ){
    TestFindFunction *p = pTab->pFindFunctionList;
    pTab->pFindFunctionList = p->pNext;
    sqlite3_free(p);
  }
  Tcl_Decr_Ref_ગણતરી(pTab->pCmd);
  sqlite3_free(pTab);
  પાછા SQLITE_OK;
}

/*
** Open a new tcl cursor.
*/
સ્થિર પૂર્ણાંક tclOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tcl_cursor *pCur;
  pCur = sqlite3_malloc(કદ(tcl_cursor));
  જો( pCur==0 ) પાછા SQLITE_NOMEM;
  memset(pCur, 0, કદ(tcl_cursor));
  *ppCursor = &pCur->base;
  પાછા SQLITE_OK;
}

/*
** Close a tcl cursor.
*/
સ્થિર પૂર્ણાંક tclClose(sqlite3_vtab_cursor *cur){
  tcl_cursor *pCur = (tcl_cursor *)cur;
  જો( pCur ){
    sqlite3_finalize(pCur->pStmt);
    sqlite3_free(pCur);
  }
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક tclNext(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  જો( pCsr->pStmt ){
    tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
    પૂર્ણાંક rc = sqlite3_step(pCsr->pStmt);
    જો( rc!=SQLITE_ROW ){
      અચલ અક્ષર *zErr;
      rc = sqlite3_finalize(pCsr->pStmt);
      pCsr->pStmt = 0;
      જો( rc!=SQLITE_OK ){
        zErr = sqlite3_errmsg(pTab->db);
        pTab->base.z_Err_સંદેશ = sqlite3_mprintf("%s", zErr);
      }
    }
  }
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક tclFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  tcl_vtab *pTab = (tcl_vtab*)(pVtabCursor->pVtab);
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript;
  Tcl_Obj *pArg;
  પૂર્ણાંક ii;
  પૂર્ણાંક rc;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ગણતરી(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xFilter", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(idx_અંક));
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj(idxStr ? idxStr : "", -1)
  );

  pArg = Tcl_NewObj();
  Tcl_Incr_Ref_ગણતરી(pArg);
  માટે(ii=0; ii<argc; ii++){
    અચલ અક્ષર *zVal = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[ii]);
    Tcl_Obj *pVal;
    જો( zVal==0 ){
      sqlite3_કિંમત *pMem;
      pVal = Tcl_NewObj();
      માટે(rc=sqlite3_vtab_in_first(argv[ii], &pMem); 
          rc==SQLITE_OK && pMem;
          rc=sqlite3_vtab_in_next(argv[ii], &pMem)
      ){
        Tcl_Obj *pVal2 = 0;
        zVal = (અચલ અક્ષર*)sqlite3_કિંમત_text(pMem);
        જો( zVal ){
          pVal2 = Tcl_NewStringObj(zVal, -1);
        }નહીંતર{
          pVal2 = Tcl_NewObj();
        }
        Tcl_ListObjAppendElement(interp, pVal, pVal2);
      }
    }નહીંતર{
      pVal = Tcl_NewStringObj(zVal, -1);
    }
    Tcl_ListObjAppendElement(interp, pArg, pVal);
  }
  Tcl_ListObjAppendElement(interp, pScript, pArg);
  Tcl_Decr_Ref_ગણતરી(pArg);

  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  જો( rc!=TCL_OK ){
    અચલ અક્ષર *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_સંદેશ = sqlite3_mprintf("%s", zErr);
  }નહીંતર{
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
    જો( rc!=TCL_OK ){
      અચલ અક્ષર *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_સંદેશ = sqlite3_mprintf("%s", zErr);
    }નહીંતર{
      માટે(ii=0; rc==SQLITE_OK && ii<(પૂર્ણાંક)nElem; ii+=2){
        અચલ અક્ષર *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        જો( sqlite3_stricmp("sql", zCmd)==0 ){
          અચલ અક્ષર *zSql = Tcl_GetString(p);
          rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
          જો( rc!=SQLITE_OK ){
            અચલ અક્ષર *zErr = sqlite3_errmsg(pTab->db);
            pTab->base.z_Err_સંદેશ = sqlite3_mprintf("unexpected: %s", zErr);
          }
        }નહીંતર{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_સંદેશ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
      }
    }
  }

  જો( rc==SQLITE_OK ){
    rc = tclNext(pVtabCursor);
  }
  પાછા rc;
}

સ્થિર પૂર્ણાંક tclColumn(
  sqlite3_vtab_cursor *pVtabCursor, 
  sqlite3_context *ctx, 
  પૂર્ણાંક i
){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  sqlite3_result_કિંમત(ctx, sqlite3_column_કિંમત(pCsr->pStmt, i+1));
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક tclRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  *pRowid = sqlite3_column_int64(pCsr->pStmt, 0);
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક tclEof(sqlite3_vtab_cursor *pVtabCursor){
  tcl_cursor *pCsr = (tcl_cursor*)pVtabCursor;
  પાછા (pCsr->pStmt==0);
}

સ્થિર શૂન્ય testBestIndexObjConstraints(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  પૂર્ણાંક ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ગણતરી(pRes);
  માટે(ii=0; ii<pIdxInfo->nConstraint; ii++){
    સંરચના sqlite3_index_constraint અચલ *pCons = &pIdxInfo->aConstraint[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    અચલ અક્ષર *zOp = 0;

    Tcl_Incr_Ref_ગણતરી(pElem);

    પસંદગી( pCons->op ){
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_EQ:
        zOp = "eq"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_GT:
        zOp = "gt"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_LE:
        zOp = "le"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_LT:
        zOp = "lt"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_GE:
        zOp = "ge"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_MATCH:
        zOp = "match"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_LIKE:
        zOp = "like"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_GLOB:
        zOp = "glob"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_REGEXP:
        zOp = "regexp"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_NE:
        zOp = "ne"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_ISNOT:
        zOp = "isnot"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
        zOp = "isnotnull"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_ISNULL:
        zOp = "isnull"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_IS:
        zOp = "is"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_LIMIT:
        zOp = "limit"; તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_OFFSET:
        zOp = "offset"; તોડો;
    }

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("op", -1));
    જો( zOp ){
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj(zOp, -1));
    }નહીંતર{
      Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->op));
    }
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("usable", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pCons->usable));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ગણતરી(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_ગણતરી(pRes);
}

સ્થિર શૂન્ય testBestIndexObjOrderby(
  Tcl_Interp *interp, 
  sqlite3_index_info *pIdxInfo
){
  પૂર્ણાંક ii;
  Tcl_Obj *pRes = Tcl_NewObj();
  Tcl_Incr_Ref_ગણતરી(pRes);
  માટે(ii=0; ii<pIdxInfo->nOrderBy; ii++){
    સંરચના sqlite3_index_orderby અચલ *pOrder = &pIdxInfo->aOrderBy[ii];
    Tcl_Obj *pElem = Tcl_NewObj();
    Tcl_Incr_Ref_ગણતરી(pElem);

    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("column", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->iColumn));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewStringObj("desc", -1));
    Tcl_ListObjAppendElement(0, pElem, Tcl_NewIntObj(pOrder->desc));

    Tcl_ListObjAppendElement(0, pRes, pElem);
    Tcl_Decr_Ref_ગણતરી(pElem);
  }

  Tcl_SetObjResult(interp, pRes);
  Tcl_Decr_Ref_ગણતરી(pRes);
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI testBestIndexObj(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  અચલ અક્ષર *azSub[] = {
    "constraints",                /* 0 */
    "orderby",                    /* 1 */
    "mask",                       /* 2 */
    "distinct",                   /* 3 */
    "in",                         /* 4 */
    "rhs_value",                  /* 5 */
    "collation",                  /* 6 */
    0
  };
  પૂર્ણાંક ii;
  sqlite3_index_info *pIdxInfo = (sqlite3_index_info*)clientData;

  જો( objc<2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "SUB-COMMAND");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIndexFromObj(interp, objv[1], azSub, "sub-command", 0, &ii) ){
    પાછા TCL_ERROR;
  }

  જો( ii<4 && objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "");
    પાછા TCL_ERROR;
  }
  જો( ii==4 && objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "INDEX BOOLEAN");
    પાછા TCL_ERROR;
  }
  જો( ii==5 && objc!=3 && objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "INDEX ?DEFAULT?");
    પાછા TCL_ERROR;
  }

  પસંદગી( ii ){
    સ્થિતિ 0: પ્રતિજ્ઞા( sqlite3_stricmp(azSub[ii], "constraints")==0 );
      testBestIndexObjConstraints(interp, pIdxInfo);
      તોડો;

    સ્થિતિ 1: પ્રતિજ્ઞા( sqlite3_stricmp(azSub[ii], "orderby")==0 );
      testBestIndexObjOrderby(interp, pIdxInfo);
      તોડો;

    સ્થિતિ 2: પ્રતિજ્ઞા( sqlite3_stricmp(azSub[ii], "mask")==0 );
      Tcl_SetObjResult(interp, Tcl_NewWideIntObj(pIdxInfo->colUsed));
      તોડો;

    સ્થિતિ 3: પ્રતિજ્ઞા( sqlite3_stricmp(azSub[ii], "distinct")==0 ); {
      પૂર્ણાંક bDistinct = sqlite3_vtab_distinct(pIdxInfo);
      Tcl_SetObjResult(interp, Tcl_NewIntObj(bDistinct));
      તોડો;
    }

    સ્થિતિ 4: પ્રતિજ્ઞા( sqlite3_stricmp(azSub[ii], "in")==0 ); {
      પૂર્ણાંક iCons;
      પૂર્ણાંક bHandle;
      જો( Tcl_GetIntFromObj(interp, objv[2], &iCons) 
       || Tcl_GetBooleanFromObj(interp, objv[3], &bHandle) 
      ){
        પાછા TCL_ERROR;
      }
      Tcl_SetObjResult(interp, 
          Tcl_NewIntObj(sqlite3_vtab_in(pIdxInfo, iCons, bHandle))
      );
      તોડો;
    }

    સ્થિતિ 5: પ્રતિજ્ઞા( sqlite3_stricmp(azSub[ii], "rhs_value")==0 ); {
      પૂર્ણાંક iCons = 0;
      પૂર્ણાંક rc;
      sqlite3_કિંમત *pVal = 0;
      અચલ અક્ષર *zVal = "";
      જો( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        પાછા TCL_ERROR;
      }
      rc = sqlite3_vtab_rhs_કિંમત(pIdxInfo, iCons, &pVal);
      જો( rc!=SQLITE_OK && rc!=SQLITE_NOTFOUND ){
        Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
        પાછા TCL_ERROR;
      }
      જો( pVal ){
        zVal = (અચલ અક્ષર*)sqlite3_કિંમત_text(pVal);
      }નહીંતર જો( objc==4 ){
        zVal = Tcl_GetString(objv[3]);
      }
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zVal, -1));
      તોડો;
    }

    સ્થિતિ 6: પ્રતિજ્ઞા( sqlite3_stricmp(azSub[ii], "collation")==0 ); {
      પૂર્ણાંક iCons = 0;
      અચલ અક્ષર *zColl = "";
      જો( Tcl_GetIntFromObj(interp, objv[2], &iCons) ){
        પાછા TCL_ERROR;
      }
      zColl = sqlite3_vtab_collation(pIdxInfo, iCons);
      Tcl_SetObjResult(interp, Tcl_NewStringObj(zColl, -1));
      તોડો;
    }
  }

  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક tclBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  પૂર્ણાંક rc = SQLITE_OK;

  સ્થિર પૂર્ણાંક iNext = 43;
  અક્ષર zHdl[24];
  Tcl_Obj *pScript;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ગણતરી(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("xBestIndex", -1));

  sqlite3_snprintf(કદ(zHdl), zHdl, "bestindex%d", iNext++);
  Tcl_CreateObjCommand(interp, zHdl, testBestIndexObj, pIdxInfo, 0);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zHdl, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_DeleteCommand(interp, zHdl);
  Tcl_Decr_Ref_ગણતરી(pScript);

  જો( rc!=TCL_OK ){
    અચલ અક્ષર *zErr = Tcl_GetStringResult(interp);
    rc = SQLITE_ERROR;
    pTab->base.z_Err_સંદેશ = sqlite3_mprintf("%s", zErr);
  }નહીંતર{
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
    જો( rc!=TCL_OK ){
      અચલ અક્ષર *zErr = Tcl_GetStringResult(interp);
      rc = SQLITE_ERROR;
      pTab->base.z_Err_સંદેશ = sqlite3_mprintf("%s", zErr);
    }નહીંતર{
      પૂર્ણાંક ii;
      પૂર્ણાંક iArgv = 1;
      માટે(ii=0; rc==SQLITE_OK && ii<(પૂર્ણાંક)nElem; ii+=2){
        અચલ અક્ષર *zCmd = Tcl_GetString(apElem[ii]);
        Tcl_Obj *p = apElem[ii+1];
        જો( sqlite3_stricmp("cost", zCmd)==0 ){
          rc = Tcl_GetDoubleFromObj(interp, p, &pIdxInfo->estimatedCost);
        }નહીંતર
        જો( sqlite3_stricmp("orderby", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->orderByConsumed);
        }નહીંતર
        જો( sqlite3_stricmp("idxnum", zCmd)==0 ){
          rc = Tcl_GetIntFromObj(interp, p, &pIdxInfo->idx_અંક);
        }નહીંતર
        જો( sqlite3_stricmp("idxstr", zCmd)==0 ){
          sqlite3_free(pIdxInfo->idxStr);
          pIdxInfo->idxStr = sqlite3_mprintf("%s", Tcl_GetString(p));
          pIdxInfo->needToFreeIdxStr = 1;
        }નહીંતર
        જો( sqlite3_stricmp("rows", zCmd)==0 ){
          Tcl_WideInt x = 0;
          rc = Tcl_GetWideIntFromObj(interp, p, &x);
          pIdxInfo->estimatedRows = (tRowcnt)x;
        }નહીંતર
        જો( sqlite3_stricmp("use", zCmd)==0 
         || sqlite3_stricmp("omit", zCmd)==0 
        ){
          પૂર્ણાંક iCons;
          rc = Tcl_GetIntFromObj(interp, p, &iCons);
          જો( rc==SQLITE_OK ){
            જો( iCons<0 || iCons>=pIdxInfo->nConstraint ){
              rc = SQLITE_ERROR;
              pTab->base.z_Err_સંદેશ = sqlite3_mprintf("unexpected: %d", iCons);
            }નહીંતર{
              પૂર્ણાંક bOmit = (zCmd[0]=='o' || zCmd[0]=='O');
              pIdxInfo->aConstraintUsage[iCons].argvIndex = iArgv++;
              pIdxInfo->aConstraintUsage[iCons].omit = bOmit;
            }
          }
        }નહીંતર
        જો( sqlite3_stricmp("constraint", zCmd)==0 ){
          rc = SQLITE_CONSTRAINT;
          pTab->base.z_Err_સંદેશ = sqlite3_mprintf("%s", Tcl_GetString(p));
        }નહીંતર{
          rc = SQLITE_ERROR;
          pTab->base.z_Err_સંદેશ = sqlite3_mprintf("unexpected: %s", zCmd);
        }
        જો( rc!=SQLITE_OK && pTab->base.z_Err_સંદેશ==0 ){
          અચલ અક્ષર *zErr = Tcl_GetStringResult(interp);
          pTab->base.z_Err_સંદેશ = sqlite3_mprintf("%s", zErr);
        }
      }
    }
  }

  પાછા rc;
}

સ્થિર શૂન્ય tclFunction(sqlite3_context *pCtx, પૂર્ણાંક nArg, sqlite3_કિંમત **apArg){
  TestFindFunction *p = (TestFindFunction*)sqlite3_user_data(pCtx);
  Tcl_Interp *interp = p->pTab->interp;
  Tcl_Obj *pScript = 0;
  Tcl_Obj *pRet = 0;
  પૂર્ણાંક ii;

  pScript = Tcl_DuplicateObj(p->pTab->pCmd);
  Tcl_Incr_Ref_ગણતરી(pScript);
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj("function", -1));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(p->zName, -1));

  માટે(ii=0; ii<nArg; ii++){
    અચલ અક્ષર *zArg = (અચલ અક્ષર*)sqlite3_કિંમત_text(apArg[ii]);
    Tcl_ListObjAppendElement(interp, pScript,
        (zArg ? Tcl_NewStringObj(zArg, -1) : Tcl_NewObj())
    );
  }
  Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ગણતરી(pScript);

  pRet = Tcl_GetObjResult(interp);
  sqlite3_result_text(pCtx, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
}

સ્થિર પૂર્ણાંક tclFindFunction(
  sqlite3_vtab *tab, 
  પૂર્ણાંક nArg, 
  અચલ અક્ષર *zName,
  શૂન્ય (**pxFunc)(sqlite3_context*,પૂર્ણાંક,sqlite3_કિંમત**),   /* OUT */
  શૂન્ય **ppArg                                             /* OUT */
){
  પૂર્ણાંક iRet = 0;
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp;
  Tcl_Obj *pScript = 0;
  પૂર્ણાંક rc = SQLITE_OK;

  pScript = Tcl_DuplicateObj(pTab->pCmd);
  Tcl_Incr_Ref_ગણતરી(pScript);
  Tcl_ListObjAppendElement(
      interp, pScript, Tcl_NewStringObj("xFindFunction", -1)
  );
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewIntObj(nArg));
  Tcl_ListObjAppendElement(interp, pScript, Tcl_NewStringObj(zName, -1));
  rc = Tcl_EvalObjEx(interp, pScript, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ગણતરી(pScript);

  જો( rc==SQLITE_OK ){
    Tcl_Obj *pObj = Tcl_GetObjResult(interp);

    જો( Tcl_GetIntFromObj(interp, pObj, &iRet) ){
      rc = SQLITE_ERROR;
    }નહીંતર જો( iRet>0 ){
      sqlite3_int64 nName = strlen(zName);
      sqlite3_int64 nByte = nName + 1 + કદ(TestFindFunction);
      TestFindFunction *pNew = 0;

      pNew = (TestFindFunction*)sqlite3_malloc64(nByte);
      જો( pNew==0 ){
        iRet = 0;
      }નહીંતર{
        memset(pNew, 0, nByte);
        pNew->zName = (અચલ અક્ષર*)&pNew[1];
        memcpy((અક્ષર*)pNew->zName, zName, nName);
        pNew->pTab = pTab;
        pNew->pNext = pTab->pFindFunctionList;
        pTab->pFindFunctionList = pNew;
        *ppArg = (શૂન્ય*)pNew;
        *pxFunc = tclFunction;
      }
    }
  }

  પાછા iRet;
}

સ્થિર પૂર્ણાંક tclUpdate(
  sqlite3_vtab *tab, 
  પૂર્ણાંક nArg, 
  sqlite3_કિંમત **apVal, 
  sqlite3_int64 *piRowid
){
  tcl_vtab *pTab = (tcl_vtab*)tab;
  Tcl_Interp *interp = pTab->interp; 
  Tcl_Obj *pEval = Tcl_DuplicateObj(pTab->pCmd);
  પૂર્ણાંક rc = TCL_OK;

  Tcl_Incr_Ref_ગણતરી(pEval);
  Tcl_ListObjAppendElement(interp, pEval, Tcl_NewStringObj("xUpdate",-1));

  rc = Tcl_EvalObjEx(interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_Decr_Ref_ગણતરી(pEval);

  જો( rc==TCL_OK ){
    Tcl_Obj *pRes = Tcl_GetObjResult(interp);
    Tcl_WideInt v;
    rc = Tcl_GetWideIntFromObj(interp, pRes, &v);
    *piRowid = (sqlite3_int64)v;
  }

  જો( rc!=TCL_OK ){
    tab->z_Err_સંદેશ = sqlite3_mprintf("%s", Tcl_GetStringResult(pTab->interp));
    પાછા rc;
  }

  પાછા SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
સ્થિર sqlite3_module tclModule = {
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
સ્થિર sqlite3_module tclModuleUpdate = {
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
બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);

સ્થિર શૂન્ય delTestVtabCtx(શૂન્ય *p){
  TestVtabContext *pCtx = (TestVtabContext*)p;
  જો( pCtx->pDefault ){
    Tcl_Decr_Ref_ગણતરી(pCtx->pDefault);
  }
  ckfree(pCtx);
}

/*
** Register the echo virtual table module.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI register_tcl_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  જો( objc!=2 && objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB ?DEFAULT-CMD?");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  {
    sqlite3_module *pMod = &tclModule;
    TestVtabContext *pCtx = (TestVtabContext*)ckalloc(કદ(TestVtabContext));
    pCtx->interp = interp;
    pCtx->pDefault = 0;
    જો( objc==3 ){
      pCtx->pDefault = objv[2];
      Tcl_Incr_Ref_ગણતરી(pCtx->pDefault);
    }

    જો( objc==3 ){ pMod = &tclModuleUpdate; }
    sqlite3_create_module_v2(db, "tcl", pMod, (શૂન્ય*)pCtx, delTestVtabCtx);
  }
#endif
  પાછા TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetesttcl_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     શૂન્ય *clientData;
  } aObjCmd[] = {
     { "register_tcl_module",   register_tcl_module, 0 },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  પાછા TCL_OK;
}
