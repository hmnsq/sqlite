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
  unsigned পূর্ণসংখ্যা iSrc, 
  unsigned অক্ষর iBranch, 
  unsigned অক্ষর iType
){
  যদি( iSrc<আকার(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

স্থির শূন্য appendToList(
  Tcl_Obj *pList, 
  পূর্ণসংখ্যা i_সারি, 
  পূর্ণসংখ্যা iPath, 
  ধ্রুবক অক্ষর *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_সারি));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_গুনতি(pNew);
}


স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_vdbe_coverage(
  Client_তথ্য cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুবক অক্ষর *aSub[] = { "start", "report", "stop", 0 };
  পূর্ণসংখ্যা iSub = -1;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "sub-command");
    ফেরত TCL_ERROR;
  }

  যদি( Tcl_Get_সূচক_From_Obj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    ফেরত TCL_ERROR;
  }

  Tcl_Reset_ফলাফল(interp);
  প্রতিজ্ঞা( iSub==0 || iSub==1 || iSub==2 );
  নির্বাচন( iSub ){
    ক্ষেত্র 0:       /* start */
      memset(aBranchArray, 0, আকার(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      বিরতি;
    ক্ষেত্র 1: {     /* report */
      পূর্ণসংখ্যা i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_গুনতি(pRes);
      জন্য(i=0; i<আকার(aBranchArray); i++){
        u8 b = aBranchArray[i];
        পূর্ণসংখ্যা bFlag = ((b >> 4)==4);
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
      Tcl_Set_Obj_ফলাফল(interp, pRes);
      Tcl_Decr_Ref_গুনতি(pRes);
      বিরতি;
    };
      
    পূর্বনির্ধারিত:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      বিরতি;
  }

  ফেরত TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

পূর্ণসংখ্যা Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  ফেরত TCL_OK;
}

#endif
