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
#ꦪꦼꦤ꧀ SQLITE_TEST          /* This file is used for testing only */

#include "sqlite3.h"
#include "sqliteInt.h"
#include "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

static u8 aBranchArray[200000];

static void test_vdbe_branch(
  void *pCtx, 
  unsigned puur_naa_nka iSrc, 
  unsigned var_na iBranch, 
  unsigned var_na iType
){
  ꦪꦼꦤ꧀( iSrc<parimaa_na(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

static void appendToList(
  Tcl_Obj *pList, 
  puur_naa_nka iLine, 
  puur_naa_nka iPath, 
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


static puur_naa_nka SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *aSub[] = { "start", "report", "stop", 0 };
  puur_naa_nka iSub = -1;
  ꦪꦼꦤ꧀( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "sub-command");
    ꦮꦭꦶ TCL_ERROR;
  }

  ꦪꦼꦤ꧀( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    ꦮꦭꦶ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  assert( iSub==0 || iSub==1 || iSub==2 );
  switch( iSub ){
    pak_sa 0:       /* start */
      memset(aBranchArray, 0, parimaa_na(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      bha_nga;
    pak_sa 1: {     /* report */
      puur_naa_nka i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_IncrRefCount(pRes);
      ꦏꦁꦒꦺꦴ(i=0; i<parimaa_na(aBranchArray); i++){
        u8 b = aBranchArray[i];
        puur_naa_nka bFlag = ((b >> 4)==4);
        ꦪꦼꦤ꧀( b ){
          ꦪꦼꦤ꧀( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          ꦪꦼꦤ꧀( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          ꦪꦼꦤ꧀( (b & 0x04)==0 ){
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

  ꦮꦭꦶ TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

puur_naa_nka Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  ꦮꦭꦶ TCL_OK;
}

#endif
