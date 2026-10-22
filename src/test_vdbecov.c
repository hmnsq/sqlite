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
#എങ്കിൽ SQLITE_TEST          /* This file is used for testing only */

#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

സ്ഥിതം u8 aBranchArray[200000];

സ്ഥിതം ശൂന്യം test_vdbe_branch(
  ശൂന്യം *pCtx, 
  unsigned പൂർണ്ണസംഖ്യ iSrc, 
  unsigned അക്ഷരം iBranch, 
  unsigned അക്ഷരം iType
){
  എങ്കിൽ( iSrc<വലുപ്പം(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

സ്ഥിതം ശൂന്യം appendToList(
  Tcl_Obj *pList, 
  പൂർണ്ണസംഖ്യ i_വരി, 
  പൂർണ്ണസംഖ്യ iPath, 
  സ്ഥിരം അക്ഷരം *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_എണ്ണം(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_വരി));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_എണ്ണം(pNew);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *aSub[] = { "start", "report", "stop", 0 };
  പൂർണ്ണസംഖ്യ iSub = -1;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "sub-command");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    തിരികെ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  പ്രതിജ്ഞാ( iSub==0 || iSub==1 || iSub==2 );
  തിരഞ്ഞെടുപ്പ്( iSub ){
    അവസ്ഥ 0:       /* start */
      memset(aBranchArray, 0, വലുപ്പം(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      നിർത്തുക;
    അവസ്ഥ 1: {     /* report */
      പൂർണ്ണസംഖ്യ i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_എണ്ണം(pRes);
      വേണ്ടി(i=0; i<വലുപ്പം(aBranchArray); i++){
        u8 b = aBranchArray[i];
        പൂർണ്ണസംഖ്യ bFlag = ((b >> 4)==4);
        എങ്കിൽ( b ){
          എങ്കിൽ( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          എങ്കിൽ( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          എങ്കിൽ( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_എണ്ണം(pRes);
      നിർത്തുക;
    };
      
    സ്ഥിരസ്ഥിതി:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      നിർത്തുക;
  }

  തിരികെ TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

പൂർണ്ണസംഖ്യ Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  തിരികെ TCL_OK;
}

#endif
