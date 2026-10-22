/*
** 2008 September 1
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** The code in this file contains sample implementations of the 
** sqlite3_wsd_init() and sqlite3_wsd_find() functions required if the
** SQLITE_OMIT_WSD symbol is defined at build time.
*/

#যদি defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#অন্তর্ভাব "sqliteInt.h"

#define PLS_HASHSIZE 43

প্রকারনির্দেশ সংরচনা ProcessLocalStorage ProcessLocalStorage;
প্রকারনির্দেশ সংরচনা ProcessLocalVar ProcessLocalVar;

সংরচনা ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  পূর্ণাঙ্ক nFree;
  u8 *pFree;
};

সংরচনা ProcessLocalVar {
  শূন্য *p_চাবি;
  ProcessLocalVar *pNext;
};

স্থির ProcessLocalStorage *pGlobal = 0;

পূর্ণাঙ্ক sqlite3_wsd_init(পূর্ণাঙ্ক N, পূর্ণাঙ্ক J){
  যদি( !pGlobal ){
    পূর্ণাঙ্ক nMalloc = N + পরিমাণ(ProcessLocalStorage) + J*পরিমাণ(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    যদি( pGlobal ){
      memset(pGlobal, 0, পরিমাণ(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - পরিমাণ(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  প্রত্যাগম pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

শূন্য *sqlite3_wsd_find(শূন্য *K, পূর্ণাঙ্ক L){
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  প্রতি(i=0; i<পরিমাণ(শূন্য*); i++){
    iHash = (iHash<<3) + ((unsigned বর্ণ *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  প্রতি(pVar=pGlobal->aData[iHash]; pVar && pVar->p_চাবি!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  যদি( !pVar ){
    পূর্ণাঙ্ক nByte = ROUND8(পরিমাণ(ProcessLocalVar) + L);
    প্রতিজ্ঞা( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_চাবি = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  প্রত্যাগম (শূন্য *)&pVar[1];
}

#endif
