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
#अन्तर्भाव "btreeInt.h"
#अन्तर्भाव "tclsqlite.h"

/*
** Usage: sqlite3_shared_cache_report
**
** Return a list of file that are shared and the number of
** references to each file.
*/
पूर्णाङ्क SQLITE_TCLAPI sqlite3BtreeSharedCacheReport(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
#यदि_अपरिभाषितम् SQLITE_OMIT_SHARED_CACHE
  बाह्य BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  प्रति(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_अग्र){
    ध्रुव वर्ण *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_समुच्चयः_Obj_परिणाम(interp, pRet);
#यदि_अन्तः
  प्रत्यागम TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
शून्य sqlite3BtreeCursorList(Btree *p){
#यदि_परिभाषितम् SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  प्रति(pCur=pBt->pCursor; pCur; pCur=pCur->p_अग्र){
    MemPage *pPage = pCur->apPage[pCur->iPage];
    वर्ण *z_बहुलकम् = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_मूलम्, z_बहुलकम्,
       pPage ? pPage->pgno : 0, pCur->aiIdx[pCur->iPage],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#यदि_अन्तः
}
