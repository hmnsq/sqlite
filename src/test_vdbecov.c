/*
** 2019 April 02
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
*/
#ако SQLITE_TEST          /* This file is used for testing only */

#include "sqlite3.h"
#include "sqliteInt.h"
#include "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

static u8 aBranchArray[200000];

static void test_vdbe_branch(
  void *pCtx, 
  unsigned цео iSrc, 
  unsigned char iBranch, 
  unsigned char iType
){
  ако( iSrc<sizeof(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

static void appendToList(
  Tcl_Obj *pList, 
  цео iLine, 
  цео iPath, 
  const char *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_IncrRefCount(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iLine));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_DecrRefCount(pNew);
}


static цео SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  цео objc,
  Tcl_Obj *CONST objv[]
){
  const char *aSub[] = { "start", "report", "stop", 0 };
  цео iSub = -1;
  ако( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "sub-command");
    врати TCL_ERROR;
  }

  ако( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    врати TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  assert( iSub==0 || iSub==1 || iSub==2 );
  switch( iSub ){
    case 0:       /* start */
      memset(aBranchArray, 0, sizeof(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      break;
    case 1: {     /* report */
      цео i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_IncrRefCount(pRes);
      за(i=0; i<sizeof(aBranchArray); i++){
        u8 b = aBranchArray[i];
        цео bFlag = ((b >> 4)==4);
        ако( b ){
          ако( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          ако( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          ако( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_DecrRefCount(pRes);
      break;
    };
      
    default:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      break;
  }

  врати TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

цео Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  врати TCL_OK;
}

#endif
