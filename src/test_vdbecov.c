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
#अगर SQLITE_TEST          /* This file is used for testing only */

#समावेश "sqlite3.h"
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"

#अगर_घोषित SQLITE_VDBE_COVERAGE

अटल u8 aBranchArray[200000];

अटल व्योम test_vdbe_branch(
  व्योम *pCtx, 
  अचिन्हित पूर्णांक iSrc, 
  अचिन्हित अक्षर iBranch, 
  अचिन्हित अक्षर iType
){
  अगर( iSrc<माप(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

अटल व्योम appendToList(
  Tcl_Obj *pList, 
  पूर्णांक i_पंक्ति, 
  पूर्णांक iPath, 
  स्थिर अक्षर *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_गिनती(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_पंक्ति));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_गिनती(pNew);
}


अटल पूर्णांक SQLITE_TCLAPI test_vdbe_coverage(
  Client_तथ्य cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *aSub[] = { "start", "report", "stop", 0 };
  पूर्णांक iSub = -1;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "sub-command");
    वापस TCL_ERROR;
  }

  अगर( Tcl_Get_सूचकांक_From_Obj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    वापस TCL_ERROR;
  }

  Tcl_Reset_परिणाम(interp);
  निश्चित( iSub==0 || iSub==1 || iSub==2 );
  चयन( iSub ){
    हाल 0:       /* start */
      स_रखो(aBranchArray, 0, माप(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      अवरोध;
    हाल 1: {     /* report */
      पूर्णांक i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_गिनती(pRes);
      क्रम(i=0; i<माप(aBranchArray); i++){
        u8 b = aBranchArray[i];
        पूर्णांक bFlag = ((b >> 4)==4);
        अगर( b ){
          अगर( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          अगर( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          अगर( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_Set_Obj_परिणाम(interp, pRes);
      Tcl_Decr_Ref_गिनती(pRes);
      अवरोध;
    };
      
    शेष:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      अवरोध;
  }

  वापस TCL_OK;
}

#पूर्ण_अगर  /* SQLITE_VDBE_COVERAGE */

पूर्णांक Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#अगर_घोषित SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#पूर्ण_अगर
  वापस TCL_OK;
}

#पूर्ण_अगर
