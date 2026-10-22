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
#ಅನ್ತರ್ಭಾವ "btreeInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

/*
** Usage: sqlite3_shared_cache_report
**
** Return a list of file that are shared and the number of
** references to each file.
*/
ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI sqlite3BtreeSharedCacheReport(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  ಬಾಹ್ಯ BtShared *sqlite3SharedCacheList;
  BtShared *pBt;
  Tcl_Obj *pRet = Tcl_NewObj();
  ಗಾಗಿ(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_ಮುಂದಿನ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile = sqlite3PagerFilename(pBt->pPager, 1);
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewStringObj(zFile, -1));
    Tcl_ListObjAppendElement(interp, pRet, Tcl_NewIntObj(pBt->nRef));
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Print debugging information about all cursors to standard output.
*/
ಶೂನ್ಯ sqlite3BtreeCursorList(Btree *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  BtCursor *pCur;
  BtShared *pBt = p->pBt;
  ಗಾಗಿ(pCur=pBt->pCursor; pCur; pCur=pCur->p_ಮುಂದಿನ){
    Mem_ಪುಟ *p_ಪುಟ = pCur->ap_ಪುಟ[pCur->i_ಪುಟ];
    ಅಕ್ಷರ *z_ಬಹುಲಕ = (pCur->curFlags & BTCF_WriteFlag) ? "rw" : "ro";
    sqlite3DebugPrintf("CURSOR %p rooted at %4d(%s) currently at %d.%d%s\n",
       pCur, pCur->pgno_ಬೇರು, z_ಬಹುಲಕ,
       p_ಪುಟ ? p_ಪುಟ->pgno : 0, pCur->aiIdx[pCur->i_ಪುಟ],
       (pCur->eState==CURSOR_VALID) ? "" : " eof"
    );
  }
#ಆದರೆ_ಅಂತ್ಯ
}
