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

#ଯଦି defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

#define PLS_HASHSIZE 43

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା ProcessLocalStorage ProcessLocalStorage;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା ProcessLocalVar ProcessLocalVar;

ସଂରଚନା ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFree;
  u8 *pFree;
};

ସଂରଚନା ProcessLocalVar {
  ଶୂନ୍ୟ *p_ଚାବି;
  ProcessLocalVar *pNext;
};

ସ୍ଥିର ProcessLocalStorage *pGlobal = 0;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_wsd_init(ପୂର୍ଣ୍ଣସଂଖ୍ୟା N, ପୂର୍ଣ୍ଣସଂଖ୍ୟା J){
  ଯଦି( !pGlobal ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMalloc = N + ଆକାର(ProcessLocalStorage) + J*ଆକାର(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    ଯଦି( pGlobal ){
      memset(pGlobal, 0, ଆକାର(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - ଆକାର(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  ଫେରନ୍ତୁ pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

ଶୂନ୍ୟ *sqlite3_wsd_find(ଶୂନ୍ୟ *K, ପୂର୍ଣ୍ଣସଂଖ୍ୟା L){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  ପାଇଁ(i=0; i<ଆକାର(ଶୂନ୍ୟ*); i++){
    iHash = (iHash<<3) + ((unsigned ଅକ୍ଷର *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  ପାଇଁ(pVar=pGlobal->aData[iHash]; pVar && pVar->p_ଚାବି!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  ଯଦି( !pVar ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte = ROUND8(ଆକାର(ProcessLocalVar) + L);
    ପ୍ରତିଜ୍ଞା( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_ଚାବି = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)&pVar[1];
}

#endif
