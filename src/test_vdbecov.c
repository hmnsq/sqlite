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
#اگر SQLITE_TEST          /* This file is used for testing only */

#شامل "sqlite3.h"
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE

ساکن u8 aBranchArray[200000];

ساکن خالی test_vdbe_branch(
  خالی *pCtx, 
  غیرعلامتی صحیح iSrc, 
  غیرعلامتی حرف iBranch, 
  غیرعلامتی حرف iType
){
  اگر( iSrc<حجم(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

ساکن خالی append_مقصد_List(
  Tcl_Obj *pList, 
  صحیح i_سطر, 
  صحیح i_راستہ, 
  ثابت حرف *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_سطر));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_راستہ));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_گنتی(pNew);
}


ساکن صحیح SQLITE_TCLAPI test_vdbe_coverage(
  Client_ڈیٹا cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *aSub[] = { "start", "report", "stop", 0 };
  صحیح iSub = -1;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "sub-command");
    واپس TCL_ERROR;
  }

  اگر( Tcl_Get_اشاریہ_From_Obj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    واپس TCL_ERROR;
  }

  Tcl_Reset_نتیجہ(interp);
  أكد( iSub==0 || iSub==1 || iSub==2 );
  انتخاب( iSub ){
    حالت 0:       /*آغاز*/
      memset(aBranchArray, 0, حجم(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      توڑو;
    حالت 1: {     /* report */
      صحیح i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_گنتی(pRes);
      برائے(i=0; i<حجم(aBranchArray); i++){
        u8 b = aBranchArray[i];
        صحیح bFlag = ((b >> 4)==4);
        اگر( b ){
          اگر( (b & 0x01)==0 ){
            append_مقصد_List(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          اگر( (b & 0x02)==0 ){
            append_مقصد_List(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          اگر( (b & 0x04)==0 ){
            append_مقصد_List(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_سیٹ_Obj_نتیجہ(interp, pRes);
      Tcl_Decr_Ref_گنتی(pRes);
      توڑو;
    };
      
    طےشدہ:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      توڑو;
  }

  واپس TCL_OK;
}

#اگر_ختم  /* SQLITE_VDBE_COVERAGE */

صحیح Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#اگر_تعریف_شدہ SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#اگر_ختم
  واپس TCL_OK;
}

#اگر_ختم
