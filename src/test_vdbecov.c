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
#ಯದಿ SQLITE_TEST          /* This file is used for testing only */

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

ಸ್ಥಿರ u8 aBranchArray[200000];

ಸ್ಥಿರ ಶೂನ್ಯ test_vdbe_branch(
  ಶೂನ್ಯ *pCtx, 
  unsigned ಪೂರ್ಣಾಙ್ಕ iSrc, 
  unsigned ವರ್ಣ iBranch, 
  unsigned ವರ್ಣ iType
){
  ಯದಿ( iSrc<ಪರಿಮಾಣ(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

ಸ್ಥಿರ ಶೂನ್ಯ appendToList(
  Tcl_Obj *pList, 
  ಪೂರ್ಣಾಙ್ಕ i_ಸಾಲು, 
  ಪೂರ್ಣಾಙ್ಕ iPath, 
  ಧ್ರುವ ವರ್ಣ *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_ಸಾಲು));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pNew);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *aSub[] = { "start", "report", "stop", 0 };
  ಪೂರ್ಣಾಙ್ಕ iSub = -1;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "sub-command");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  ಪ್ರತಿಜ್ಞಾ( iSub==0 || iSub==1 || iSub==2 );
  ವಿಕಲ್ಪ( iSub ){
    ಪಕ್ಷ 0:       /* start */
      memset(aBranchArray, 0, ಪರಿಮಾಣ(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      ಭಙ್ಗ;
    ಪಕ್ಷ 1: {     /* report */
      ಪೂರ್ಣಾಙ್ಕ i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_ಎಣಿಕೆ(pRes);
      ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aBranchArray); i++){
        u8 b = aBranchArray[i];
        ಪೂರ್ಣಾಙ್ಕ bFlag = ((b >> 4)==4);
        ಯದಿ( b ){
          ಯದಿ( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          ಯದಿ( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          ಯದಿ( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_ಎಣಿಕೆ(pRes);
      ಭಙ್ಗ;
    };
      
    ಸ್ವಭಾವ:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      ಭಙ್ಗ;
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

ಪೂರ್ಣಾಙ್ಕ Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif
