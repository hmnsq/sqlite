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

#ប្រសិនបើ defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#include "sqliteInt.h"

#define PLS_HASHSIZE 43

typedef sa_nracanaa ProcessLocalStorage ProcessLocalStorage;
typedef sa_nracanaa ProcessLocalVar ProcessLocalVar;

sa_nracanaa ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  ចំនួនគត់ nFree;
  u8 *pFree;
};

sa_nracanaa ProcessLocalVar {
  void *pKey;
  ProcessLocalVar *pNext;
};

static ProcessLocalStorage *pGlobal = 0;

ចំនួនគត់ sqlite3_wsd_init(ចំនួនគត់ N, ចំនួនគត់ J){
  ប្រសិនបើ( !pGlobal ){
    ចំនួនគត់ nMalloc = N + parimaa_na(ProcessLocalStorage) + J*parimaa_na(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    ប្រសិនបើ( pGlobal ){
      memset(pGlobal, 0, parimaa_na(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - parimaa_na(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  ត្រឡប់ pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

void *sqlite3_wsd_find(void *K, ចំនួនគត់ L){
  ចំនួនគត់ i;
  ចំនួនគត់ iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  សម្រាប់(i=0; i<parimaa_na(void*); i++){
    iHash = (iHash<<3) + ((unsigned var_na *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  សម្រាប់(pVar=pGlobal->aData[iHash]; pVar && pVar->pKey!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  ប្រសិនបើ( !pVar ){
    ចំនួនគត់ nByte = ROUND8(parimaa_na(ProcessLocalVar) + L);
    assert( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->pKey = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  ត្រឡប់ (void *)&pVar[1];
}

#endif
