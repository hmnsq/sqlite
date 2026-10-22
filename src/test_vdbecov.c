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
#যদি SQLITE_TEST          /* This file is used for testing only */

#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

স্থির u8 aBranchArray[200000];

স্থির শূন্য test_vdbe_branch(
  শূন্য *pCtx, 
  unsigned পূর্ণাঙ্ক iSrc, 
  unsigned বর্ণ iBranch, 
  unsigned বর্ণ iType
){
  যদি( iSrc<পরিমাণ(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

স্থির শূন্য appendToList(
  Tcl_Obj *pList, 
  পূর্ণাঙ্ক i_সারি, 
  পূর্ণাঙ্ক iPath, 
  ধ্রুব বর্ণ *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_সারি));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_গুনতি(pNew);
}


স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুব বর্ণ *aSub[] = { "start", "report", "stop", 0 };
  পূর্ণাঙ্ক iSub = -1;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "sub-command");
    প্রত্যাগম TCL_ERROR;
  }

  যদি( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    প্রত্যাগম TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  প্রতিজ্ঞা( iSub==0 || iSub==1 || iSub==2 );
  বিকল্প( iSub ){
    পক্ষ 0:       /* start */
      memset(aBranchArray, 0, পরিমাণ(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      ভঙ্গ;
    পক্ষ 1: {     /* report */
      পূর্ণাঙ্ক i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_গুনতি(pRes);
      প্রতি(i=0; i<পরিমাণ(aBranchArray); i++){
        u8 b = aBranchArray[i];
        পূর্ণাঙ্ক bFlag = ((b >> 4)==4);
        যদি( b ){
          যদি( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          যদি( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          যদি( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_গুনতি(pRes);
      ভঙ্গ;
    };
      
    স্বভাব:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      ভঙ্গ;
  }

  প্রত্যাগম TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

পূর্ণাঙ্ক Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  প্রত্যাগম TCL_OK;
}

#endif
