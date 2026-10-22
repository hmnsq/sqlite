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
#जर SQLITE_TEST          /* This file is used for testing only */

#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

स्थिर u8 aBranchArray[200000];

स्थिर शून्य test_vdbe_branch(
  शून्य *pCtx, 
  अचिन्हित पूर्णांक iSrc, 
  अचिन्हित अक्षर iBranch, 
  अचिन्हित अक्षर iType
){
  जर( iSrc<आकार(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

स्थिर शून्य appendToList(
  Tcl_Obj *pList, 
  पूर्णांक i_ओळ, 
  पूर्णांक iPath, 
  स्थिरांक अक्षर *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_मोजणी(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_ओळ));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_मोजणी(pNew);
}


स्थिर पूर्णांक SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिरांक अक्षर *aSub[] = { "start", "report", "stop", 0 };
  पूर्णांक iSub = -1;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "sub-command");
    परत TCL_ERROR;
  }

  जर( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    परत TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  प्रतिज्ञा( iSub==0 || iSub==1 || iSub==2 );
  निवड( iSub ){
    स्थिती 0:       /* start */
      स_रखो(aBranchArray, 0, आकार(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      थांबा;
    स्थिती 1: {     /* report */
      पूर्णांक i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_मोजणी(pRes);
      साठी(i=0; i<आकार(aBranchArray); i++){
        u8 b = aBranchArray[i];
        पूर्णांक bFlag = ((b >> 4)==4);
        जर( b ){
          जर( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          जर( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          जर( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_मोजणी(pRes);
      थांबा;
    };
      
    पूर्वनिर्धारित:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      थांबा;
  }

  परत TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

पूर्णांक Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  परत TCL_OK;
}

#endif
