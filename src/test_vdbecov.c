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

#ifdef SQLITE_VDBE_COVERAGE

ایستا u8 aBranchArray[200000];

ایستا تهی test_vdbe_branch(
  تهی *pCtx, 
  unsigned صحیح iSrc, 
  unsigned نویسه iBranch, 
  unsigned نویسه iType
){
  اگر( iSrc<اندازه(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

ایستا تهی appendToList(
  Tcl_Obj *pList, 
  صحیح i_سطر, 
  صحیح iPath, 
  ثابت نویسه *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_شمار(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_سطر));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_شمار(pNew);
}


ایستا صحیح SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت نویسه *aSub[] = { "start", "report", "stop", 0 };
  صحیح iSub = -1;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "sub-command");
    بازگشت TCL_ERROR;
  }

  اگر( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    بازگشت TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  أكد( iSub==0 || iSub==1 || iSub==2 );
  انتخاب( iSub ){
    حالت 0:       /* start */
      memset(aBranchArray, 0, اندازه(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      بشکن;
    حالت 1: {     /* report */
      صحیح i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_شمار(pRes);
      برای(i=0; i<اندازه(aBranchArray); i++){
        u8 b = aBranchArray[i];
        صحیح bFlag = ((b >> 4)==4);
        اگر( b ){
          اگر( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          اگر( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          اگر( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_شمار(pRes);
      بشکن;
    };
      
    پیشفرض:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      بشکن;
  }

  بازگشت TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

صحیح Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  بازگشت TCL_OK;
}

#endif
