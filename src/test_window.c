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

#include "sqlite3.h"

#ifdef SQLITE_TEST

#include "sqliteInt.h"
#include "tclsqlite.h"

extern số_nguyên getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);
extern const char *sqlite3ErrName(số_nguyên);

typedef struct TestWindow TestWindow;
struct TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *xValue;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

typedef struct TestWindowCtx TestWindowCtx;
struct TestWindowCtx {
  Tcl_Obj *pVal;
};

static void doTestWindowStep(
  số_nguyên bInverse,
  sqlite3_context *ctx, 
  số_nguyên nArg, 
  sqlite3_value **apArg
){
  số_nguyên i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, sizeof(TestWindowCtx));

  Tcl_IncrRefCount(pEval);
  nếu( pCtx ){
    const char *zResult;
    số_nguyên rc;
    nếu( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }không_thì{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    cho(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((const char*)sqlite3_value_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    nếu( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }không_thì{
      nếu( pCtx->pVal ) Tcl_DecrRefCount(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_IncrRefCount(pCtx->pVal);
    }
  }
  Tcl_DecrRefCount(pEval);
}

static void doTestWindowFinalize(số_nguyên bValue, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bValue ? p->xValue : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, sizeof(TestWindowCtx));

  Tcl_IncrRefCount(pEval);
  nếu( pCtx ){
    const char *zResult;
    số_nguyên rc;
    nếu( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }không_thì{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    nếu( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }không_thì{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    nếu( bValue==0 ){
      nếu( pCtx->pVal ) Tcl_DecrRefCount(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_DecrRefCount(pEval);
}

static void testWindowStep(
  sqlite3_context *ctx, 
  số_nguyên nArg, 
  sqlite3_value **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
static void testWindowInverse(
  sqlite3_context *ctx, 
  số_nguyên nArg, 
  sqlite3_value **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

static void testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
static void testWindowValue(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

static void testWindowDestroy(void *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
static số_nguyên SQLITE_TCLAPI test_create_window(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  const char *zName;
  số_nguyên rc;

  nếu( objc!=7 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    trả_về TCL_ERROR;
  }

  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(sizeof(TestWindow));
  memset(pNew, 0, sizeof(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->xValue = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_IncrRefCount(pNew->xStep);
  Tcl_IncrRefCount(pNew->xFinal);
  Tcl_IncrRefCount(pNew->xValue);
  Tcl_IncrRefCount(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (void*)pNew,
      testWindowStep, testWindowFinal, testWindowValue, testWindowInverse,
      testWindowDestroy
  );
  nếu( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    trả_về TCL_ERROR;
  }

  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI test_create_window_misuse(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  số_nguyên rc;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, testWindowValue, testWindowInverse,
      0
  );
  nếu( rc!=SQLITE_MISUSE ) goto error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, testWindowValue, testWindowInverse,
      0
  );
  nếu( rc!=SQLITE_MISUSE ) goto error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  nếu( rc!=SQLITE_MISUSE ) goto error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, testWindowValue, 0,
      0
  );
  nếu( rc!=SQLITE_MISUSE ) goto error;

  trả_về TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  trả_về TCL_ERROR;
}

/*
** xStep for sumint().
*/
static void sumintStep(
  sqlite3_context *ctx, 
  số_nguyên nArg, 
  sqlite3_value *apArg[]
){
  sqlite3_int64 *pInt;

  assert( nArg==1 );
  nếu( sqlite3_value_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    trả_về;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, sizeof(sqlite3_int64));
  nếu( pInt ){
    *pInt += sqlite3_value_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
static void sumintInverse(
  sqlite3_context *ctx, 
  số_nguyên nArg, 
  sqlite3_value *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, sizeof(sqlite3_int64));
  *pInt -= sqlite3_value_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
static void sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  nếu( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
static void sumintValue(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  nếu( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

static số_nguyên SQLITE_TCLAPI test_create_sumint(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  số_nguyên rc;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumintValue, sumintInverse,
      0
  );

  nếu( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    trả_về TCL_ERROR;
  }
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI test_override_sum(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  số_nguyên rc;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  nếu( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    trả_về TCL_ERROR;
  }
  trả_về TCL_OK;
}

số_nguyên Sqlitetest_window_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     số_nguyên clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  số_nguyên i;
  cho(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  trả_về TCL_OK;
}
#endif
