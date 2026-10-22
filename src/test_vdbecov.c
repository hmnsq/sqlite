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
#إذا SQLITE_TEST          /* This file is used for testing only */

#ضمن "sqlite3.h"
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

ساكن u8 aBranchArray[200000];

ساكن فراغ test_vdbe_branch(
  فراغ *pCtx, 
  unsigned صحيح iSrc, 
  unsigned حرف iBranch, 
  unsigned حرف iType
){
  إذا( iSrc<حجم(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

ساكن فراغ appendToList(
  Tcl_Obj *pList, 
  صحيح i_سطر, 
  صحيح iPath, 
  ثابت حرف *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_سطر));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_گنتی(pNew);
}


ساكن صحيح SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *aSub[] = { "start", "report", "stop", 0 };
  صحيح iSub = -1;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_Args(interp, 1, objv, "sub-command");
    أرجع TCL_ERROR;
  }

  إذا( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    أرجع TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  أكد( iSub==0 || iSub==1 || iSub==2 );
  بدل( iSub ){
    حالة 0:       /* start */
      memset(aBranchArray, 0, حجم(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      اكسر;
    حالة 1: {     /* report */
      صحيح i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_گنتی(pRes);
      لكل(i=0; i<حجم(aBranchArray); i++){
        u8 b = aBranchArray[i];
        صحيح bFlag = ((b >> 4)==4);
        إذا( b ){
          إذا( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          إذا( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          إذا( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_گنتی(pRes);
      اكسر;
    };
      
    افتراضي:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      اكسر;
  }

  أرجع TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

صحيح Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  أرجع TCL_OK;
}

#endif
