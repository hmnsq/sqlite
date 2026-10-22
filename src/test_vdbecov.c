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
#ఒకవేళ SQLITE_TEST          /* This file is used for testing only */

#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"

#నిర్వచితమైతే SQLITE_VDBE_COVERAGE

స్థిర u8 aBranchArray[200000];

స్థిర శూన్య test_vdbe_branch(
  శూన్య *pCtx, 
  గుర్తులేని పూర్ణాంకం iSrc, 
  గుర్తులేని అక్షరం iBranch, 
  గుర్తులేని అక్షరం iType
){
  ఒకవేళ( iSrc<పరిమాణం(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

స్థిర శూన్య append_గమ్యం_List(
  Tcl_Obj *pList, 
  పూర్ణాంకం i_పంక్తి, 
  పూర్ణాంకం i_మార్గం, 
  స్థిరరాశి అక్షరం *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_లెక్క(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_పంక్తి));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_మార్గం));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_లెక్క(pNew);
}


స్థిర పూర్ణాంకం SQLITE_TCLAPI test_vdbe_coverage(
  Client_దత్తాంశం cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *aSub[] = { "start", "report", "stop", 0 };
  పూర్ణాంకం iSub = -1;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "sub-command");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    తిరిగి TCL_ERROR;
  }

  Tcl_Reset_ఫలితం(interp);
  ప్రతిజ్ఞా( iSub==0 || iSub==1 || iSub==2 );
  ఎంపిక( iSub ){
    స్థితి 0:       /*ప్రారంభం*/
      memset(aBranchArray, 0, పరిమాణం(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      ఆపు;
    స్థితి 1: {     /* report */
      పూర్ణాంకం i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_లెక్క(pRes);
      కొరకు(i=0; i<పరిమాణం(aBranchArray); i++){
        u8 b = aBranchArray[i];
        పూర్ణాంకం bFlag = ((b >> 4)==4);
        ఒకవేళ( b ){
          ఒకవేళ( (b & 0x01)==0 ){
            append_గమ్యం_List(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          ఒకవేళ( (b & 0x02)==0 ){
            append_గమ్యం_List(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          ఒకవేళ( (b & 0x04)==0 ){
            append_గమ్యం_List(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_సమితి_Obj_ఫలితం(interp, pRes);
      Tcl_Decr_Ref_లెక్క(pRes);
      ఆపు;
    };
      
    అప్రమేయ:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      ఆపు;
  }

  తిరిగి TCL_OK;
}

#అయితే_ముగింపు  /* SQLITE_VDBE_COVERAGE */

పూర్ణాంకం Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు
