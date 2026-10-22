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
#ಆದರೆ SQLITE_TEST          /* This file is used for testing only */

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE

ಸ್ಥಿರ u8 aBranchArray[200000];

ಸ್ಥಿರ ಶೂನ್ಯ test_vdbe_branch(
  ಶೂನ್ಯ *pCtx, 
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ iSrc, 
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ iBranch, 
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ iType
){
  ಆದರೆ( iSrc<ಗಾತ್ರ(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ append_ಗಮ್ಯ_List(
  Tcl_Obj *pList, 
  ಪೂರ್ಣಾಂಕ i_ಸಾಲು, 
  ಪೂರ್ಣಾಂಕ i_ಮಾರ್ಗ, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_ಸಾಲು));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_ಮಾರ್ಗ));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pNew);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_vdbe_coverage(
  Client_ದತ್ತಾಂಶ cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *aSub[] = { "start", "report", "stop", 0 };
  ಪೂರ್ಣಾಂಕ iSub = -1;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "sub-command");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  Tcl_Reset_ಫಲಿತಾಂಶ(interp);
  ಪ್ರತಿಜ್ಞಾ( iSub==0 || iSub==1 || iSub==2 );
  ಆಯ್ಕೆ( iSub ){
    ಸ್ಥಿತಿ 0:       /*ಆರಂಭ*/
      memset(aBranchArray, 0, ಗಾತ್ರ(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1: {     /* report */
      ಪೂರ್ಣಾಂಕ i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_ಎಣಿಕೆ(pRes);
      ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aBranchArray); i++){
        u8 b = aBranchArray[i];
        ಪೂರ್ಣಾಂಕ bFlag = ((b >> 4)==4);
        ಆದರೆ( b ){
          ಆದರೆ( (b & 0x01)==0 ){
            append_ಗಮ್ಯ_List(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          ಆದರೆ( (b & 0x02)==0 ){
            append_ಗಮ್ಯ_List(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          ಆದರೆ( (b & 0x04)==0 ){
            append_ಗಮ್ಯ_List(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRes);
      Tcl_Decr_Ref_ಎಣಿಕೆ(pRes);
      ನಿಲ್ಲಿಸು;
    };
      
    ಪೂರ್ವನಿಯೋಜಿತ:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      ನಿಲ್ಲಿಸು;
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ  /* SQLITE_VDBE_COVERAGE */

ಪೂರ್ಣಾಂಕ Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ
