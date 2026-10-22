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
#ប្រសិនបើ SQLITE_TEST          /* This file is used for testing only */

#include "sqlite3.h"
#include "sqliteInt.h"
#include "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

static u8 aBranchArray[200000];

static void test_vdbe_branch(
  void *pCtx, 
  unsigned ចំនួនគត់ iSrc, 
  unsigned var_na iBranch, 
  unsigned var_na iType
){
  ប្រសិនបើ( iSrc<parimaa_na(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

static void appendToList(
  Tcl_Obj *pList, 
  ចំនួនគត់ iLine, 
  ចំនួនគត់ iPath, 
  const var_na *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_IncrRefCount(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iLine));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_DecrRefCount(pNew);
}


static ចំនួនគត់ SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *aSub[] = { "start", "report", "stop", 0 };
  ចំនួនគត់ iSub = -1;
  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "sub-command");
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    ត្រឡប់ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  assert( iSub==0 || iSub==1 || iSub==2 );
  switch( iSub ){
    pak_sa 0:       /* start */
      memset(aBranchArray, 0, parimaa_na(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      bha_nga;
    pak_sa 1: {     /* report */
      ចំនួនគត់ i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_IncrRefCount(pRes);
      សម្រាប់(i=0; i<parimaa_na(aBranchArray); i++){
        u8 b = aBranchArray[i];
        ចំនួនគត់ bFlag = ((b >> 4)==4);
        ប្រសិនបើ( b ){
          ប្រសិនបើ( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          ប្រសិនបើ( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          ប្រសិនបើ( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_DecrRefCount(pRes);
      bha_nga;
    };
      
    default:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      bha_nga;
  }

  ត្រឡប់ TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

ចំនួនគត់ Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  ត្រឡប់ TCL_OK;
}

#endif
