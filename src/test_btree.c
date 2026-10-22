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
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
#اگر_تعریف_نشده SQLITE_OMIT_SHARED_CACHE
  خارجي BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  لكل(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_بعدی){
    ثابت حرف *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_مجموعه_Obj_نتیجه(interp, pRet);
#پایان_اگر
  أرجع TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
فراغ sqlite3BtreeCursorList(Btree *p){
#اگر_تعریف_شده SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  لكل(pCur=pBt->pCursor; pCur; pCur=pCur->p_بعدی){
    Mem_صفحه *p_صفحه = pCur->ap_صفحه[pCur->i_صفحه];
    حرف *z_نما = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_ریشه, z_نما,
       p_صفحه ? p_صفحه->pgno : 0, pCur->aiIdx[pCur->i_صفحه],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#پایان_اگر
}
