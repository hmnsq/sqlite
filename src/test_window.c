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

#ಅನ್ತರ್ಭಾವ "sqlite3.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Test_ಕಿಟಕಿ Test_ಕಿಟಕಿ;
ರಚನೆ Test_ಕಿಟಕಿ {
  Tcl_Obj *x_ಹೆಜ್ಜೆ;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_ಮೌಲ್ಯ;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Test_ಕಿಟಕಿ_Ctx Test_ಕಿಟಕಿ_Ctx;
ರಚನೆ Test_ಕಿಟಕಿ_Ctx {
  Tcl_Obj *pVal;
};

ಸ್ಥಿರ ಶೂನ್ಯ do_Test_ಕಿಟಕಿ_ಹೆಜ್ಜೆ(
  ಪೂರ್ಣಾಂಕ bInverse,
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಂಕ nArg, 
  sqlite3_ಮೌಲ್ಯ **apArg
){
  ಪೂರ್ಣಾಂಕ i;
  Test_ಕಿಟಕಿ *p = (Test_ಕಿಟಕಿ*)sqlite3_user_ದತ್ತಾಂಶ(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->x_ಹೆಜ್ಜೆ);
  Test_ಕಿಟಕಿ_Ctx *pCtx = sqlite3_aggregate_context(ctx, ಗಾತ್ರ(Test_ಕಿಟಕಿ_Ctx));

  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  ಆದರೆ( pCtx ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಫಲಿತಾಂಶ;
    ಪೂರ್ಣಾಂಕ rc;
    ಆದರೆ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    ಗಾಗಿ(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    ಆದರೆ( rc!=TCL_OK ){
      z_ಫಲಿತಾಂಶ = Tcl_Get_String_ಫಲಿತಾಂಶ(p->interp);
      sqlite3_ಫಲಿತಾಂಶ_error(ctx, z_ಫಲಿತಾಂಶ, -1);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( pCtx->pVal ) Tcl_Decr_Ref_ಎಣಿಕೆ(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_ಫಲಿತಾಂಶ(p->interp));
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);
}

ಸ್ಥಿರ ಶೂನ್ಯ do_Test_ಕಿಟಕಿ_Finalize(ಪೂರ್ಣಾಂಕ b_ಮೌಲ್ಯ, sqlite3_context *ctx){
  Test_ಕಿಟಕಿ *p = (Test_ಕಿಟಕಿ*)sqlite3_user_ದತ್ತಾಂಶ(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_ಮೌಲ್ಯ ? p->x_ಮೌಲ್ಯ : p->xFinal);
  Test_ಕಿಟಕಿ_Ctx *pCtx = sqlite3_aggregate_context(ctx, ಗಾತ್ರ(Test_ಕಿಟಕಿ_Ctx));

  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  ಆದರೆ( pCtx ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಫಲಿತಾಂಶ;
    ಪೂರ್ಣಾಂಕ rc;
    ಆದರೆ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_ಫಲಿತಾಂಶ = Tcl_Get_String_ಫಲಿತಾಂಶ(p->interp);
    ಆದರೆ( rc!=TCL_OK ){
      sqlite3_ಫಲಿತಾಂಶ_error(ctx, z_ಫಲಿತಾಂಶ, -1);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, z_ಫಲಿತಾಂಶ, -1, SQLITE_TRANSIENT);
    }

    ಆದರೆ( b_ಮೌಲ್ಯ==0 ){
      ಆದರೆ( pCtx->pVal ) Tcl_Decr_Ref_ಎಣಿಕೆ(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);
}

ಸ್ಥಿರ ಶೂನ್ಯ test_ಕಿಟಕಿ_ಹೆಜ್ಜೆ(
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಂಕ nArg, 
  sqlite3_ಮೌಲ್ಯ **apArg
){
  do_Test_ಕಿಟಕಿ_ಹೆಜ್ಜೆ(0, ctx, nArg, apArg);
}
ಸ್ಥಿರ ಶೂನ್ಯ test_ಕಿಟಕಿ_Inverse(
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಂಕ nArg, 
  sqlite3_ಮೌಲ್ಯ **apArg
){
  do_Test_ಕಿಟಕಿ_ಹೆಜ್ಜೆ(1, ctx, nArg, apArg);
}

ಸ್ಥಿರ ಶೂನ್ಯ test_ಕಿಟಕಿ_Final(sqlite3_context *ctx){
  do_Test_ಕಿಟಕಿ_Finalize(0, ctx);
}
ಸ್ಥಿರ ಶೂನ್ಯ test_ಕಿಟಕಿ_ಮೌಲ್ಯ(sqlite3_context *ctx){
  do_Test_ಕಿಟಕಿ_Finalize(1, ctx);
}

ಸ್ಥಿರ ಶೂನ್ಯ test_ಕಿಟಕಿ_Destroy(ಶೂನ್ಯ *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_create_ಕಿಟಕಿ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Test_ಕಿಟಕಿ *pNew;
  sqlite3 *db;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=7 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (Test_ಕಿಟಕಿ*)ckalloc(ಗಾತ್ರ(Test_ಕಿಟಕಿ));
  memset(pNew, 0, ಗಾತ್ರ(Test_ಕಿಟಕಿ));
  pNew->x_ಹೆಜ್ಜೆ = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_ಮೌಲ್ಯ = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew->x_ಹೆಜ್ಜೆ);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew->xFinal);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew->x_ಮೌಲ್ಯ);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew->xInverse);

  rc = sqlite3_create_ಕಿಟಕಿ_function(db, zName, -1, SQLITE_UTF8, (ಶೂನ್ಯ*)pNew,
      test_ಕಿಟಕಿ_ಹೆಜ್ಜೆ, test_ಕಿಟಕಿ_Final, test_ಕಿಟಕಿ_ಮೌಲ್ಯ, test_ಕಿಟಕಿ_Inverse,
      test_ಕಿಟಕಿ_Destroy
  );
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_create_ಕಿಟಕಿ_misuse(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  rc = sqlite3_create_ಕಿಟಕಿ_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, test_ಕಿಟಕಿ_Final, test_ಕಿಟಕಿ_ಮೌಲ್ಯ, test_ಕಿಟಕಿ_Inverse,
      0
  );
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು error;
  rc = sqlite3_create_ಕಿಟಕಿ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_ಕಿಟಕಿ_ಹೆಜ್ಜೆ, 0, test_ಕಿಟಕಿ_ಮೌಲ್ಯ, test_ಕಿಟಕಿ_Inverse,
      0
  );
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು error;
  rc = sqlite3_create_ಕಿಟಕಿ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_ಕಿಟಕಿ_ಹೆಜ್ಜೆ, test_ಕಿಟಕಿ_Final, 0, test_ಕಿಟಕಿ_Inverse,
      0
  );
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು error;
  rc = sqlite3_create_ಕಿಟಕಿ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_ಕಿಟಕಿ_ಹೆಜ್ಜೆ, test_ಕಿಟಕಿ_Final, test_ಕಿಟಕಿ_ಮೌಲ್ಯ, 0,
      0
  );
  ಆದರೆ( rc!=SQLITE_MISUSE ) ಹೋಗು error;

  ಹಿಂತಿರುಗಿ TCL_OK;

 error:
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj("misuse test error", -1));
  ಹಿಂತಿರುಗಿ TCL_ERROR;
}

/*
** xStep for sumint().
*/
ಸ್ಥಿರ ಶೂನ್ಯ sumint_ಹೆಜ್ಜೆ(
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಂಕ nArg, 
  sqlite3_ಮೌಲ್ಯ *apArg[]
){
  sqlite3_int64 *pInt;

  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_ಫಲಿತಾಂಶ_error(ctx, "invalid argument", -1);
    ಹಿಂತಿರುಗಿ;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, ಗಾತ್ರ(sqlite3_int64));
  ಆದರೆ( pInt ){
    *pInt += sqlite3_ಮೌಲ್ಯ_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ಸ್ಥಿರ ಶೂನ್ಯ sumintInverse(
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಂಕ nArg, 
  sqlite3_ಮೌಲ್ಯ *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, ಗಾತ್ರ(sqlite3_int64));
  *pInt -= sqlite3_ಮೌಲ್ಯ_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ಸ್ಥಿರ ಶೂನ್ಯ sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ಆದರೆ( pInt ) res = *pInt;
  sqlite3_ಫಲಿತಾಂಶ_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ಸ್ಥಿರ ಶೂನ್ಯ sumint_ಮೌಲ್ಯ(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ಆದರೆ( pInt ) res = *pInt;
  sqlite3_ಫಲಿತಾಂಶ_int64(ctx, res);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_create_sumint(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  rc = sqlite3_create_ಕಿಟಕಿ_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumint_ಹೆಜ್ಜೆ, sumintFinal, sumint_ಮೌಲ್ಯ, sumintInverse,
      0
  );

  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_override_ಮೊತ್ತ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumint_ಹೆಜ್ಜೆ, sumintFinal
  );

  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಪೂರ್ಣಾಂಕ Sqlitetest_ಕಿಟಕಿ_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಪೂರ್ಣಾಂಕ client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_ಕಿಟಕಿ, 0 },
     { "test_create_window_function_misuse", test_create_ಕಿಟಕಿ_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_ಮೊತ್ತ, 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Client_ದತ್ತಾಂಶ c = (Client_ದತ್ತಾಂಶ)SQLITE_INT_ಗಮ್ಯ_PTR(aObjCmd[i].client_ದತ್ತಾಂಶ);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
