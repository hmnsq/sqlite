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
#எனில் SQLITE_TEST          /* This file is used for testing only */

#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

நிலையான u8 aBranchArray[200000];

நிலையான வெறுமை test_vdbe_branch(
  வெறுமை *pCtx, 
  unsigned முழு iSrc, 
  unsigned எழுத்து iBranch, 
  unsigned எழுத்து iType
){
  எனில்( iSrc<அளவு(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

நிலையான வெறுமை appendToList(
  Tcl_Obj *pList, 
  முழு i_வரி, 
  முழு iPath, 
  மாறிலி எழுத்து *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_மொத்தம்(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_வரி));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_மொத்தம்(pNew);
}


நிலையான முழு SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  மாறிலி எழுத்து *aSub[] = { "start", "report", "stop", 0 };
  முழு iSub = -1;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "sub-command");
    திரும்பு TCL_ERROR;
  }

  எனில்( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    திரும்பு TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  ப்ரதிஜ்ஞா( iSub==0 || iSub==1 || iSub==2 );
  தேர்வு( iSub ){
    நிலை 0:       /* start */
      memset(aBranchArray, 0, அளவு(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      நிறுத்து;
    நிலை 1: {     /* report */
      முழு i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_மொத்தம்(pRes);
      க்கு(i=0; i<அளவு(aBranchArray); i++){
        u8 b = aBranchArray[i];
        முழு bFlag = ((b >> 4)==4);
        எனில்( b ){
          எனில்( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          எனில்( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          எனில்( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_மொத்தம்(pRes);
      நிறுத்து;
    };
      
    இயல்பு:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      நிறுத்து;
  }

  திரும்பு TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

முழு Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  திரும்பு TCL_OK;
}

#endif
