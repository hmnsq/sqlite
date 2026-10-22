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
#ضمن "btreeInt.h"
#ضمن "tclsqlite.h"

/*
** Usage: sqlite3_shared_cache_report
**
** Return a list of file that are shared and the number of
** references to each file.
*/
صحيح SQLITE_TCLAPI sqlite3BtreeSharedCacheReport(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
#که_نه_تعریف_شوی SQLITE_OMIT_SHARED_CACHE
  خارجي BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  لكل(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_بل){
    ثابت حرف *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_سېټ_Obj_پایله(interp, pRet);
#د_که_پای
  أرجع TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
فراغ sqlite3BtreeCursorList(Btree *p){
#که_تعریف_شوی SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  لكل(pCur=pBt->pCursor; pCur; pCur=pCur->p_بل){
    Mem_پاڼه *p_پاڼه = pCur->ap_پاڼه[pCur->i_پاڼه];
    حرف *z_ډېرپېښ = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_ریښه, z_ډېرپېښ,
       p_پاڼه ? p_پاڼه->pgno : 0, pCur->aiIdx[pCur->i_پاڼه],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#د_که_پای
}
