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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SHARED_CACHE
  خارجي BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  لكل(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_ايندڙ){
    ثابت حرف *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_سيٽ_Obj_نتيجو(interp, pRet);
#جيڪڏهن_پورو
  أرجع TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
فراغ sqlite3BtreeCursorList(Btree *p){
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  لكل(pCur=pBt->pCursor; pCur; pCur=pCur->p_ايندڙ){
    Mem_صفحو *p_صفحو = pCur->ap_صفحو[pCur->i_صفحو];
    حرف *z_گهڻائي = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_پاڙ, z_گهڻائي,
       p_صفحو ? p_صفحو->pgno : 0, pCur->aiIdx[pCur->i_صفحو],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#جيڪڏهن_پورو
}
