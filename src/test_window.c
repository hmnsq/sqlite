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

#ifdef SQLITE_TEST

#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, अचर वर्ण *zA, sqlite3 **ppDb);
बाह्य अचर वर्ण *sqlite3ErrName(पूर्णाङ्क);

प्रकारनिर्देश संरचना TestWindow TestWindow;
संरचना TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_मान;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

प्रकारनिर्देश संरचना TestWindowCtx TestWindowCtx;
संरचना TestWindowCtx {
  Tcl_Obj *pVal;
};

स्थिर शून्य doTestWindowStep(
  पूर्णाङ्क bInverse,
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मान **apArg
){
  पूर्णाङ्क i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, परिमाण(TestWindowCtx));

  Tcl_Incr_Ref_गन्ती(pEval);
  यदि( pCtx ){
    अचर वर्ण *zResult;
    पूर्णाङ्क rc;
    यदि( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }अन्यथा{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    लागि(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((अचर वर्ण*)sqlite3_मान_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    यदि( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }अन्यथा{
      यदि( pCtx->pVal ) Tcl_Decr_Ref_गन्ती(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_Incr_Ref_गन्ती(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_गन्ती(pEval);
}

स्थिर शून्य doTestWindowFinalize(पूर्णाङ्क b_मान, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_मान ? p->x_मान : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, परिमाण(TestWindowCtx));

  Tcl_Incr_Ref_गन्ती(pEval);
  यदि( pCtx ){
    अचर वर्ण *zResult;
    पूर्णाङ्क rc;
    यदि( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }अन्यथा{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    यदि( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }अन्यथा{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    यदि( b_मान==0 ){
      यदि( pCtx->pVal ) Tcl_Decr_Ref_गन्ती(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_गन्ती(pEval);
}

स्थिर शून्य testWindowStep(
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मान **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
स्थिर शून्य testWindowInverse(
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मान **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

स्थिर शून्य testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
स्थिर शून्य test_Window_मान(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

स्थिर शून्य testWindowDestroy(शून्य *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_create_window(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  अचर वर्ण *zName;
  पूर्णाङ्क rc;

  यदि( objc!=7 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    फर्काऊ TCL_ERROR;
  }

  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(परिमाण(TestWindow));
  स_रखो(pNew, 0, परिमाण(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_मान = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_गन्ती(pNew->xStep);
  Tcl_Incr_Ref_गन्ती(pNew->xFinal);
  Tcl_Incr_Ref_गन्ती(pNew->x_मान);
  Tcl_Incr_Ref_गन्ती(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (शून्य*)pNew,
      testWindowStep, testWindowFinal, test_Window_मान, testWindowInverse,
      testWindowDestroy
  );
  यदि( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    फर्काऊ TCL_ERROR;
  }

  फर्काऊ TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_create_window_misuse(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "DB");
    फर्काऊ TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_मान, testWindowInverse,
      0
  );
  यदि( rc!=SQLITE_MISUSE ) गच्छ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_मान, testWindowInverse,
      0
  );
  यदि( rc!=SQLITE_MISUSE ) गच्छ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  यदि( rc!=SQLITE_MISUSE ) गच्छ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_मान, 0,
      0
  );
  यदि( rc!=SQLITE_MISUSE ) गच्छ error;

  फर्काऊ TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  फर्काऊ TCL_ERROR;
}

/*
** xStep for sumint().
*/
स्थिर शून्य sumintStep(
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मान *apArg[]
){
  sqlite3_int64 *pInt;

  प्रतिज्ञा( nArg==1 );
  यदि( sqlite3_मान_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    फर्काऊ;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, परिमाण(sqlite3_int64));
  यदि( pInt ){
    *pInt += sqlite3_मान_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
स्थिर शून्य sumintInverse(
  sqlite3_context *ctx, 
  पूर्णाङ्क nArg, 
  sqlite3_मान *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, परिमाण(sqlite3_int64));
  *pInt -= sqlite3_मान_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
स्थिर शून्य sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  यदि( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
स्थिर शून्य sumint_मान(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  यदि( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_create_sumint(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "DB");
    फर्काऊ TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_मान, sumintInverse,
      0
  );

  यदि( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_override_sum(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णाङ्क rc;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "DB");
    फर्काऊ TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  यदि( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    फर्काऊ TCL_ERROR;
  }
  फर्काऊ TCL_OK;
}

पूर्णाङ्क Sqlitetest_window_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     पूर्णाङ्क clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  पूर्णाङ्क i;
  लागि(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  फर्काऊ TCL_OK;
}
#endif
