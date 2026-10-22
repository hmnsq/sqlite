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

#यदि_परिभाषितम् SQLITE_VDBE_COVERAGE

स्थिर u8 aBranchArray[200000];

स्थिर शून्य test_vdbe_branch(
  शून्य *pCtx, 
  अचिह्नित पूर्णाङ्क iSrc, 
  अचिह्नित वर्ण iBranch, 
  अचिह्नित वर्ण iType
){
  यदि( iSrc<परिमाण(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

स्थिर शून्य append_गन्तव्यम्_List(
  Tcl_Obj *pList, 
  पूर्णाङ्क i_पङ्क्तिः, 
  पूर्णाङ्क i_मार्गः, 
  ध्रुव वर्ण *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_गणनम्(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_पङ्क्तिः));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_मार्गः));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_गणनम्(pNew);
}


स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vdbe_coverage(
  Client_दत्त cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *aSub[] = { "start", "report", "stop", 0 };
  पूर्णाङ्क iSub = -1;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "sub-command");
    प्रत्यागम TCL_ERROR;
  }

  यदि( Tcl_Get_सूचकांक_From_Obj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    प्रत्यागम TCL_ERROR;
  }

  Tcl_Reset_परिणाम(interp);
  प्रतिज्ञा( iSub==0 || iSub==1 || iSub==2 );
  विकल्प( iSub ){
    पक्ष 0:       /* start */
      स्मृतिं_पूरय(aBranchArray, 0, परिमाण(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      भङ्ग;
    पक्ष 1: {     /* report */
      पूर्णाङ्क i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_गणनम्(pRes);
      प्रति(i=0; i<परिमाण(aBranchArray); i++){
        u8 b = aBranchArray[i];
        पूर्णाङ्क bFlag = ((b >> 4)==4);
        यदि( b ){
          यदि( (b & 0x01)==0 ){
            append_गन्तव्यम्_List(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          यदि( (b & 0x02)==0 ){
            append_गन्तव्यम्_List(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          यदि( (b & 0x04)==0 ){
            append_गन्तव्यम्_List(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_समुच्चयः_Obj_परिणाम(interp, pRes);
      Tcl_Decr_Ref_गणनम्(pRes);
      भङ्ग;
    };
      
    स्वभाव:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      भङ्ग;
  }

  प्रत्यागम TCL_OK;
}

#यदि_अन्तः  /* SQLITE_VDBE_COVERAGE */

पूर्णाङ्क Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#यदि_परिभाषितम् SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#यदि_अन्तः
  प्रत्यागम TCL_OK;
}

#यदि_अन्तः
