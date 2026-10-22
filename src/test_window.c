/*
** 2018 June 17
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
*/

#అన్తర్భావ "sqlite3.h"

#నిర్వచితమైతే SQLITE_TEST

#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"

బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

రకనిర్వచనం నిర్మాణం Test_కిటికీ Test_కిటికీ;
నిర్మాణం Test_కిటికీ {
  Tcl_Obj *x_అడుగు;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_విలువ;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

రకనిర్వచనం నిర్మాణం Test_కిటికీ_Ctx Test_కిటికీ_Ctx;
నిర్మాణం Test_కిటికీ_Ctx {
  Tcl_Obj *pVal;
};

స్థిర శూన్య do_Test_కిటికీ_అడుగు(
  పూర్ణాంకం bInverse,
  sqlite3_context *ctx, 
  పూర్ణాంకం nArg, 
  sqlite3_విలువ **apArg
){
  పూర్ణాంకం i;
  Test_కిటికీ *p = (Test_కిటికీ*)sqlite3_user_దత్తాంశం(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->x_అడుగు);
  Test_కిటికీ_Ctx *pCtx = sqlite3_aggregate_context(ctx, పరిమాణం(Test_కిటికీ_Ctx));

  Tcl_Incr_Ref_లెక్క(pEval);
  ఒకవేళ( pCtx ){
    స్థిరరాశి అక్షరం *z_ఫలితం;
    పూర్ణాంకం rc;
    ఒకవేళ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }లేకపోతే{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    కొరకు(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    ఒకవేళ( rc!=TCL_OK ){
      z_ఫలితం = Tcl_Get_String_ఫలితం(p->interp);
      sqlite3_ఫలితం_error(ctx, z_ఫలితం, -1);
    }లేకపోతే{
      ఒకవేళ( pCtx->pVal ) Tcl_Decr_Ref_లెక్క(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_ఫలితం(p->interp));
      Tcl_Incr_Ref_లెక్క(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_లెక్క(pEval);
}

స్థిర శూన్య do_Test_కిటికీ_Finalize(పూర్ణాంకం b_విలువ, sqlite3_context *ctx){
  Test_కిటికీ *p = (Test_కిటికీ*)sqlite3_user_దత్తాంశం(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_విలువ ? p->x_విలువ : p->xFinal);
  Test_కిటికీ_Ctx *pCtx = sqlite3_aggregate_context(ctx, పరిమాణం(Test_కిటికీ_Ctx));

  Tcl_Incr_Ref_లెక్క(pEval);
  ఒకవేళ( pCtx ){
    స్థిరరాశి అక్షరం *z_ఫలితం;
    పూర్ణాంకం rc;
    ఒకవేళ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }లేకపోతే{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_ఫలితం = Tcl_Get_String_ఫలితం(p->interp);
    ఒకవేళ( rc!=TCL_OK ){
      sqlite3_ఫలితం_error(ctx, z_ఫలితం, -1);
    }లేకపోతే{
      sqlite3_ఫలితం_పాఠ్యం(ctx, z_ఫలితం, -1, SQLITE_TRANSIENT);
    }

    ఒకవేళ( b_విలువ==0 ){
      ఒకవేళ( pCtx->pVal ) Tcl_Decr_Ref_లెక్క(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_లెక్క(pEval);
}

స్థిర శూన్య test_కిటికీ_అడుగు(
  sqlite3_context *ctx, 
  పూర్ణాంకం nArg, 
  sqlite3_విలువ **apArg
){
  do_Test_కిటికీ_అడుగు(0, ctx, nArg, apArg);
}
స్థిర శూన్య test_కిటికీ_Inverse(
  sqlite3_context *ctx, 
  పూర్ణాంకం nArg, 
  sqlite3_విలువ **apArg
){
  do_Test_కిటికీ_అడుగు(1, ctx, nArg, apArg);
}

స్థిర శూన్య test_కిటికీ_Final(sqlite3_context *ctx){
  do_Test_కిటికీ_Finalize(0, ctx);
}
స్థిర శూన్య test_కిటికీ_విలువ(sqlite3_context *ctx){
  do_Test_కిటికీ_Finalize(1, ctx);
}

స్థిర శూన్య test_కిటికీ_Destroy(శూన్య *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_create_కిటికీ(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Test_కిటికీ *pNew;
  sqlite3 *db;
  స్థిరరాశి అక్షరం *zName;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=7 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (Test_కిటికీ*)ckalloc(పరిమాణం(Test_కిటికీ));
  memset(pNew, 0, పరిమాణం(Test_కిటికీ));
  pNew->x_అడుగు = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_విలువ = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_లెక్క(pNew->x_అడుగు);
  Tcl_Incr_Ref_లెక్క(pNew->xFinal);
  Tcl_Incr_Ref_లెక్క(pNew->x_విలువ);
  Tcl_Incr_Ref_లెక్క(pNew->xInverse);

  rc = sqlite3_create_కిటికీ_function(db, zName, -1, SQLITE_UTF8, (శూన్య*)pNew,
      test_కిటికీ_అడుగు, test_కిటికీ_Final, test_కిటికీ_విలువ, test_కిటికీ_Inverse,
      test_కిటికీ_Destroy
  );
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    తిరిగి TCL_ERROR;
  }

  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_create_కిటికీ_misuse(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;

  rc = sqlite3_create_కిటికీ_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, test_కిటికీ_Final, test_కిటికీ_విలువ, test_కిటికీ_Inverse,
      0
  );
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు error;
  rc = sqlite3_create_కిటికీ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_కిటికీ_అడుగు, 0, test_కిటికీ_విలువ, test_కిటికీ_Inverse,
      0
  );
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు error;
  rc = sqlite3_create_కిటికీ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_కిటికీ_అడుగు, test_కిటికీ_Final, 0, test_కిటికీ_Inverse,
      0
  );
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు error;
  rc = sqlite3_create_కిటికీ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_కిటికీ_అడుగు, test_కిటికీ_Final, test_కిటికీ_విలువ, 0,
      0
  );
  ఒకవేళ( rc!=SQLITE_MISUSE ) వెళ్ళు error;

  తిరిగి TCL_OK;

 error:
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj("misuse test error", -1));
  తిరిగి TCL_ERROR;
}

/*
** xStep for sumint().
*/
స్థిర శూన్య sumint_అడుగు(
  sqlite3_context *ctx, 
  పూర్ణాంకం nArg, 
  sqlite3_విలువ *apArg[]
){
  sqlite3_int64 *pInt;

  ప్రతిజ్ఞా( nArg==1 );
  ఒకవేళ( sqlite3_విలువ_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_ఫలితం_error(ctx, "invalid argument", -1);
    తిరిగి;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, పరిమాణం(sqlite3_int64));
  ఒకవేళ( pInt ){
    *pInt += sqlite3_విలువ_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
స్థిర శూన్య sumintInverse(
  sqlite3_context *ctx, 
  పూర్ణాంకం nArg, 
  sqlite3_విలువ *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, పరిమాణం(sqlite3_int64));
  *pInt -= sqlite3_విలువ_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
స్థిర శూన్య sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ఒకవేళ( pInt ) res = *pInt;
  sqlite3_ఫలితం_int64(ctx, res);
}

/*
** xValue for sumint().
*/
స్థిర శూన్య sumint_విలువ(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ఒకవేళ( pInt ) res = *pInt;
  sqlite3_ఫలితం_int64(ctx, res);
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_create_sumint(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;

  rc = sqlite3_create_కిటికీ_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumint_అడుగు, sumintFinal, sumint_విలువ, sumintInverse,
      0
  );

  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_override_కూడిక(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumint_అడుగు, sumintFinal
  );

  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}

పూర్ణాంకం Sqlitetest_కిటికీ_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     పూర్ణాంకం client_దత్తాంశం;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_కిటికీ, 0 },
     { "test_create_window_function_misuse", test_create_కిటికీ_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_కూడిక, 0 },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Client_దత్తాంశం c = (Client_దత్తాంశం)SQLITE_INT_గమ్యం_PTR(aObjCmd[i].client_దత్తాంశం);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  తిరిగి TCL_OK;
}
#అయితే_ముగింపు
