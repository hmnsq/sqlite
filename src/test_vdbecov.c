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
#यदि SQLITE_TEST          /* This file is used for testing only */

#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

स्थिर u8 aBranchArray[200000];

स्थिर शून्य test_vdbe_branch(
  शून्य *pCtx, 
  अचिन्हित पूर्णाङ्क iSrc, 
  अचिन्हित वर्ण iBranch, 
  अचिन्हित वर्ण iType
){
  यदि( iSrc<परिमाण(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

स्थिर शून्य appendToList(
  Tcl_Obj *pList, 
  पूर्णाङ्क i_पङ्क्ति, 
  पूर्णाङ्क iPath, 
  अचर वर्ण *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_गन्ती(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_पङ्क्ति));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_गन्ती(pNew);
}


स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  अचर वर्ण *aSub[] = { "start", "report", "stop", 0 };
  पूर्णाङ्क iSub = -1;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "sub-command");
    फर्काऊ TCL_ERROR;
  }

  यदि( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    फर्काऊ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  प्रतिज्ञा( iSub==0 || iSub==1 || iSub==2 );
  विकल्प( iSub ){
    पक्ष 0:       /* start */
      स_रखो(aBranchArray, 0, परिमाण(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      रोक;
    पक्ष 1: {     /* report */
      पूर्णाङ्क i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_गन्ती(pRes);
      लागि(i=0; i<परिमाण(aBranchArray); i++){
        u8 b = aBranchArray[i];
        पूर्णाङ्क bFlag = ((b >> 4)==4);
        यदि( b ){
          यदि( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          यदि( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          यदि( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_गन्ती(pRes);
      रोक;
    };
      
    स्वभाव:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      रोक;
  }

  फर्काऊ TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

पूर्णाङ्क Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  फर्काऊ TCL_OK;
}

#endif
