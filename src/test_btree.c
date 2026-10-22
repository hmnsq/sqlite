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
#ਅਨ੍ਤਰ੍ਭਾਵ "btreeInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

/*
** Usage: sqlite3_shared_cache_report
**
** Return a list of file that are shared and the number of
** references to each file.
*/
ਪੂਰਨ SQLITE_TCLAPI sqlite3BtreeSharedCacheReport(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  ਬਾਹਰੀ BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  ਲਈ(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_ਅਗਲਾ){
    ਅਚਲ ਅੱਖਰ *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
ਖਾਲੀ sqlite3BtreeCursorList(Btree *p){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  ਲਈ(pCur=pBt->pCursor; pCur; pCur=pCur->p_ਅਗਲਾ){
    Mem_ਪੰਨਾ *p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ];
    ਅੱਖਰ *z_ਬਹੁਲਕ = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_ਜੜ੍ਹ, z_ਬਹੁਲਕ,
       p_ਪੰਨਾ ? p_ਪੰਨਾ->pgno : 0, pCur->aiIdx[pCur->i_ਪੰਨਾ],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#ਜੇ_ਅੰਤ
}
