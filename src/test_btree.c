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
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
#ifndef SQLITE_OMIT_SHARED_CACHE
  بیرونی BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  برای(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->pNext){
    ثابت نویسه *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_SetObjResult(interp, pRet);
#endif
  بازگشت TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
تهی sqlite3BtreeCursorList(Btree *p){
#ifdef SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  برای(pCur=pBt->pCursor; pCur; pCur=pCur->pNext){
    MemPage *pPage = pCur->apPage[pCur->iPage];
    نویسه *zMode = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_ریشه, zMode,
       pPage ? pPage->pgno : 0, pCur->aiIdx[pCur->iPage],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#endif
}
