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

#अन्तर्भाव "sqlite3.h"

#यदि_परिभाषितम् SQLITE_TEST

#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

प्रकारनिर्देश संरचना Test_गवाक्षः Test_गवाक्षः;
संरचना Test_गवाक्षः {
  Tcl_Obj *x_चरणः;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_मानम्;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

प्रकारनिर्देश संरचना Test_गवाक्षः_Ctx Test_गवाक्षः_Ctx;
संरचना Test_गवाक्षः_Ctx {
  Tcl_Obj *pVal;
};

स्थिर शून्य do_Test_गवाक्षः_चरणः(
  पूर्णाङ्क bInverse,
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मानम् **apArg
){
  पूर्णाङ्क i;
  Test_गवाक्षः *p = (Test_गवाक्षः*)sqlite3_user_दत्त(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->x_चरणः);
  Test_गवाक्षः_Ctx *pCtx = sqlite3_aggregate_context(ctx, परिमाण(Test_गवाक्षः_Ctx));

  Tcl_Incr_Ref_गणनम्(pEval);
  यदि( pCtx ){
    ध्रुव वर्ण *z_परिणाम;
    पूर्णाङ्क rc;
    यदि( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }अन्यथा{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    प्रति(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ध्रुव वर्ण*)sqlite3_मानम्_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    यदि( rc!=TCL_OK ){
      z_परिणाम = Tcl_Get_String_परिणाम(p->interp);
      sqlite3_परिणाम_error(ctx, z_परिणाम, -1);
    }अन्यथा{
      यदि( pCtx->pVal ) Tcl_Decr_Ref_गणनम्(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_परिणाम(p->interp));
      Tcl_Incr_Ref_गणनम्(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_गणनम्(pEval);
}

स्थिर शून्य do_Test_गवाक्षः_Finalize(पूर्णाङ्क b_मानम्, sqlite3_context *ctx){
  Test_गवाक्षः *p = (Test_गवाक्षः*)sqlite3_user_दत्त(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_मानम् ? p->x_मानम् : p->xFinal);
  Test_गवाक्षः_Ctx *pCtx = sqlite3_aggregate_context(ctx, परिमाण(Test_गवाक्षः_Ctx));

  Tcl_Incr_Ref_गणनम्(pEval);
  यदि( pCtx ){
    ध्रुव वर्ण *z_परिणाम;
    पूर्णाङ्क rc;
    यदि( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }अन्यथा{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_परिणाम = Tcl_Get_String_परिणाम(p->interp);
    यदि( rc!=TCL_OK ){
      sqlite3_परिणाम_error(ctx, z_परिणाम, -1);
    }अन्यथा{
      sqlite3_परिणाम_text(ctx, z_परिणाम, -1, SQLITE_TRANSIENT);
    }

    यदि( b_मानम्==0 ){
      यदि( pCtx->pVal ) Tcl_Decr_Ref_गणनम्(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_गणनम्(pEval);
}

स्थिर शून्य test_गवाक्षः_चरणः(
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मानम् **apArg
){
  do_Test_गवाक्षः_चरणः(0, ctx, nArg, apArg);
}
स्थिर शून्य test_गवाक्षः_Inverse(
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मानम् **apArg
){
  do_Test_गवाक्षः_चरणः(1, ctx, nArg, apArg);
}

स्थिर शून्य test_गवाक्षः_Final(sqlite3_context *ctx){
  do_Test_गवाक्षः_Finalize(0, ctx);
}
स्थिर शून्य test_गवाक्षः_मानम्(sqlite3_context *ctx){
  do_Test_गवाक्षः_Finalize(1, ctx);
}

स्थिर शून्य test_गवाक्षः_Destroy(शून्य *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_create_गवाक्षः(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Test_गवाक्षः *pNew;
  sqlite3 *db;
  ध्रुव वर्ण *zName;
  पूर्णाङ्क rc;

  यदि( objc!=7 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    प्रत्यागम TCL_ERROR;
  }

  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (Test_गवाक्षः*)ckalloc(परिमाण(Test_गवाक्षः));
  स्मृतिं_पूरय(pNew, 0, परिमाण(Test_गवाक्षः));
  pNew->x_चरणः = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_मानम् = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_गणनम्(pNew->x_चरणः);
  Tcl_Incr_Ref_गणनम्(pNew->xFinal);
  Tcl_Incr_Ref_गणनम्(pNew->x_मानम्);
  Tcl_Incr_Ref_गणनम्(pNew->xInverse);

  rc = sqlite3_create_गवाक्षः_function(db, zName, -1, SQLITE_UTF8, (शून्य*)pNew,
      test_गवाक्षः_चरणः, test_गवाक्षः_Final, test_गवाक्षः_मानम्, test_गवाक्षः_Inverse,
      test_गवाक्षः_Destroy
  );
  यदि( rc!=SQLITE_OK ){
    Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    प्रत्यागम TCL_ERROR;
  }

  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_create_गवाक्षः_misuse(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;

  rc = sqlite3_create_गवाक्षः_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, test_गवाक्षः_Final, test_गवाक्षः_मानम्, test_गवाक्षः_Inverse,
      0
  );
  यदि( rc!=SQLITE_MISUSE ) गच्छ error;
  rc = sqlite3_create_गवाक्षः_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_गवाक्षः_चरणः, 0, test_गवाक्षः_मानम्, test_गवाक्षः_Inverse,
      0
  );
  यदि( rc!=SQLITE_MISUSE ) गच्छ error;
  rc = sqlite3_create_गवाक्षः_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_गवाक्षः_चरणः, test_गवाक्षः_Final, 0, test_गवाक्षः_Inverse,
      0
  );
  यदि( rc!=SQLITE_MISUSE ) गच्छ error;
  rc = sqlite3_create_गवाक्षः_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_गवाक्षः_चरणः, test_गवाक्षः_Final, test_गवाक्षः_मानम्, 0,
      0
  );
  यदि( rc!=SQLITE_MISUSE ) गच्छ error;

  प्रत्यागम TCL_OK;

 error:
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj("misuse test error", -1));
  प्रत्यागम TCL_ERROR;
}

/*
** xStep for sumint().
*/
स्थिर शून्य sumint_चरणः(
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मानम् *apArg[]
){
  sqlite3_int64 *pInt;

  प्रतिज्ञा( nArg==1 );
  यदि( sqlite3_मानम्_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_परिणाम_error(ctx, "invalid argument", -1);
    प्रत्यागम;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, परिमाण(sqlite3_int64));
  यदि( pInt ){
    *pInt += sqlite3_मानम्_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
स्थिर शून्य sumintInverse(
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मानम् *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, परिमाण(sqlite3_int64));
  *pInt -= sqlite3_मानम्_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
स्थिर शून्य sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  यदि( pInt ) res = *pInt;
  sqlite3_परिणाम_int64(ctx, res);
}

/*
** xValue for sumint().
*/
स्थिर शून्य sumint_मानम्(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  यदि( pInt ) res = *pInt;
  sqlite3_परिणाम_int64(ctx, res);
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_create_sumint(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;

  rc = sqlite3_create_गवाक्षः_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumint_चरणः, sumintFinal, sumint_मानम्, sumintInverse,
      0
  );

  यदि( rc!=SQLITE_OK ){
    Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    प्रत्यागम TCL_ERROR;
  }
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_override_योग(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumint_चरणः, sumintFinal
  );

  यदि( rc!=SQLITE_OK ){
    Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    प्रत्यागम TCL_ERROR;
  }
  प्रत्यागम TCL_OK;
}

पूर्णाङ्क Sqlitetest_गवाक्षः_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     पूर्णाङ्क client_दत्त;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_गवाक्षः, 0 },
     { "test_create_window_function_misuse", test_create_गवाक्षः_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_योग, 0 },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Client_दत्त c = (Client_दत्त)SQLITE_INT_गन्तव्यम्_PTR(aObjCmd[i].client_दत्त);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  प्रत्यागम TCL_OK;
}
#यदि_अन्तः
