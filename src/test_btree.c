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
#شامل "btreeInt.h"
#شامل "tclsqlite.h"

/*
** Usage: sqlite3_shared_cache_report
**
** Return a list of file that are shared and the number of
** references to each file.
*/
صحیح SQLITE_TCLAPI sqlite3BtreeSharedCacheReport(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  خارجی BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  برائے(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_اگلا){
    ثابت حرف *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, pRet);
#اگر_ختم
  واپس TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
خالی sqlite3BtreeCursorList(Btree *p){
#اگر_تعریف_شدہ SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  برائے(pCur=pBt->pCursor; pCur; pCur=pCur->p_اگلا){
    Mem_صفحہ *p_صفحہ = pCur->ap_صفحہ[pCur->i_صفحہ];
    حرف *z_منوال = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_جڑ, z_منوال,
       p_صفحہ ? p_صفحہ->pgno : 0, pCur->aiIdx[pCur->i_صفحہ],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#اگر_ختم
}
