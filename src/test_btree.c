/*
** 2007 May 05
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the btree.c module in SQLite.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
*/
#అన్తర్భావ "btreeInt.h"
#అన్తర్భావ "tclsqlite.h"

/*
** Usage: sqlite3_shared_cache_report
**
** Return a list of file that are shared and the number of
** references to each file.
*/
పూర్ణాంకం SQLITE_TCLAPI sqlite3BtreeSharedCacheReport(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  బాహ్య BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  కొరకు(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_తదుపరి){
    స్థిరరాశి అక్షరం *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_సమితి_Obj_ఫలితం(interp, pRet);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
శూన్య sqlite3BtreeCursorList(Btree *p){
#నిర్వచితమైతే SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  కొరకు(pCur=pBt->pCursor; pCur; pCur=pCur->p_తదుపరి){
    Mem_పుట *p_పుట = pCur->ap_పుట[pCur->i_పుట];
    అక్షరం *z_బహుళకం = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_మూలం, z_బహుళకం,
       p_పుట ? p_పుట->pgno : 0, pCur->aiIdx[pCur->i_పుట],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#అయితే_ముగింపు
}
