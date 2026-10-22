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
#ଯଦି SQLITE_TEST          /* This file is used for testing only */

#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

ସ୍ଥିର u8 aBranchArray[200000];

ସ୍ଥିର ଶୂନ୍ୟ test_vdbe_branch(
  ଶୂନ୍ୟ *pCtx, 
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSrc, 
  unsigned ଅକ୍ଷର iBranch, 
  unsigned ଅକ୍ଷର iType
){
  ଯଦି( iSrc<ଆକାର(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

ସ୍ଥିର ଶୂନ୍ୟ appendToList(
  Tcl_Obj *pList, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ଧାଡ଼ି, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPath, 
  ଅଚଳ ଅକ୍ଷର *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_ଗଣନା(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_ଧାଡ଼ି));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_ଗଣନା(pNew);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *aSub[] = { "start", "report", "stop", 0 };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSub = -1;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "sub-command");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  ପ୍ରତିଜ୍ଞା( iSub==0 || iSub==1 || iSub==2 );
  ବାଛ( iSub ){
    ସ୍ଥିତି 0:       /* start */
      memset(aBranchArray, 0, ଆକାର(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 1: {     /* report */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_ଗଣନା(pRes);
      ପାଇଁ(i=0; i<ଆକାର(aBranchArray); i++){
        u8 b = aBranchArray[i];
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା bFlag = ((b >> 4)==4);
        ଯଦି( b ){
          ଯଦି( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          ଯଦି( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          ଯଦି( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_ଗଣନା(pRes);
      ଭାଙ୍ଗ;
    };
      
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      ଭାଙ୍ଗ;
  }

  ଫେରନ୍ତୁ TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  ଫେରନ୍ତୁ TCL_OK;
}

#endif
