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
#ਜੇ SQLITE_TEST          /* This file is used for testing only */

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE

ਸਥਿਰ u8 aBranchArray[200000];

ਸਥਿਰ ਖਾਲੀ test_vdbe_branch(
  ਖਾਲੀ *pCtx, 
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ iSrc, 
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ iBranch, 
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ iType
){
  ਜੇ( iSrc<ਆਕਾਰ(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

ਸਥਿਰ ਖਾਲੀ append_ਮੰਜ਼ਿਲ_List(
  Tcl_Obj *pList, 
  ਪੂਰਨ i_ਸਤਰ, 
  ਪੂਰਨ i_ਰਸਤਾ, 
  ਅਚਲ ਅੱਖਰ *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_ਗਿਣਤੀ(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_ਸਤਰ));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_ਰਸਤਾ));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pNew);
}


ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_vdbe_coverage(
  Client_ਅੰਕੜੇ cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *aSub[] = { "start", "report", "stop", 0 };
  ਪੂਰਨ iSub = -1;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "sub-command");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    ਵਾਪਸ TCL_ERROR;
  }

  Tcl_Reset_ਨਤੀਜਾ(interp);
  ਪ੍ਰਤਿਜ੍ਞਾ( iSub==0 || iSub==1 || iSub==2 );
  ਚੋਣ( iSub ){
    ਹਾਲਤ 0:       /*ਸ਼ੁਰੂ*/
      memset(aBranchArray, 0, ਆਕਾਰ(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      ਰੋਕੋ;
    ਹਾਲਤ 1: {     /* report */
      ਪੂਰਨ i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_ਗਿਣਤੀ(pRes);
      ਲਈ(i=0; i<ਆਕਾਰ(aBranchArray); i++){
        u8 b = aBranchArray[i];
        ਪੂਰਨ bFlag = ((b >> 4)==4);
        ਜੇ( b ){
          ਜੇ( (b & 0x01)==0 ){
            append_ਮੰਜ਼ਿਲ_List(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          ਜੇ( (b & 0x02)==0 ){
            append_ਮੰਜ਼ਿਲ_List(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          ਜੇ( (b & 0x04)==0 ){
            append_ਮੰਜ਼ਿਲ_List(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRes);
      Tcl_Decr_Ref_ਗਿਣਤੀ(pRes);
      ਰੋਕੋ;
    };
      
    ਮੂਲ:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      ਰੋਕੋ;
  }

  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ  /* SQLITE_VDBE_COVERAGE */

ਪੂਰਨ Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ
