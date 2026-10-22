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
#જો SQLITE_TEST          /* This file is used for testing only */

#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"

#ifdef SQLITE_VDBE_COVERAGE

સ્થિર u8 aBranchArray[200000];

સ્થિર શૂન્ય test_vdbe_branch(
  શૂન્ય *pCtx, 
  unsigned પૂર્ણાંક iSrc, 
  unsigned અક્ષર iBranch, 
  unsigned અક્ષર iType
){
  જો( iSrc<કદ(aBranchArray) ){
    aBranchArray[iSrc] |= iBranch;
  }
}

સ્થિર શૂન્ય appendToList(
  Tcl_Obj *pList, 
  પૂર્ણાંક i_લીટી, 
  પૂર્ણાંક iPath, 
  અચલ અક્ષર *zNever
){
  Tcl_Obj *pNew = Tcl_NewObj();
  Tcl_Incr_Ref_ગણતરી(pNew);
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(i_લીટી));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewIntObj(iPath));
  Tcl_ListObjAppendElement(0, pNew, Tcl_NewStringObj(zNever, -1));
  Tcl_ListObjAppendElement(0, pList, pNew);
  Tcl_Decr_Ref_ગણતરી(pNew);
}


સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_vdbe_coverage(
  ClientData cd,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *aSub[] = { "start", "report", "stop", 0 };
  પૂર્ણાંક iSub = -1;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "sub-command");
    પાછા TCL_ERROR;
  }

  જો( Tcl_GetIndexFromObj(interp, objv[1], aSub, "sub-command", 0, &iSub) ){
    પાછા TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  પ્રતિજ્ઞા( iSub==0 || iSub==1 || iSub==2 );
  પસંદગી( iSub ){
    સ્થિતિ 0:       /* start */
      memset(aBranchArray, 0, કદ(aBranchArray));
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, test_vdbe_branch, 0);
      તોડો;
    સ્થિતિ 1: {     /* report */
      પૂર્ણાંક i;
      Tcl_Obj *pRes = Tcl_NewObj();
      Tcl_Incr_Ref_ગણતરી(pRes);
      માટે(i=0; i<કદ(aBranchArray); i++){
        u8 b = aBranchArray[i];
        પૂર્ણાંક bFlag = ((b >> 4)==4);
        જો( b ){
          જો( (b & 0x01)==0 ){
            appendToList(pRes, i, 0, bFlag ? "less than" : "falls through");
          }
          જો( (b & 0x02)==0 ){
            appendToList(pRes, i, 1, bFlag ? "equal" : "taken");
          }
          જો( (b & 0x04)==0 ){
            appendToList(pRes, i, 2, bFlag ? "greater-than" : "NULL");
          }
        }
      }
      Tcl_SetObjResult(interp, pRes);
      Tcl_Decr_Ref_ગણતરી(pRes);
      તોડો;
    };
      
    મૂળભૂત:      /* stop */         
      sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, 0, 0);
      તોડો;
  }

  પાછા TCL_OK;
}

#endif  /* SQLITE_VDBE_COVERAGE */

પૂર્ણાંક Sqlitetestvdbecov_Init(Tcl_Interp *interp){
#ifdef SQLITE_VDBE_COVERAGE
  Tcl_CreateObjCommand(interp, "vdbe_coverage", test_vdbe_coverage, 0, 0);
#endif
  પાછા TCL_OK;
}

#endif
