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

#ถ้า defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#include "sqliteInt.h"

#define PLS_HASHSIZE 43

typedef sa_nracanaa ProcessLocalStorage ProcessLocalStorage;
typedef sa_nracanaa ProcessLocalVar ProcessLocalVar;

sa_nracanaa ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  จำนวนเต็ม nFree;
  u8 *pFree;
};

sa_nracanaa ProcessLocalVar {
  void *pKey;
  ProcessLocalVar *pNext;
};

static ProcessLocalStorage *pGlobal = 0;

จำนวนเต็ม sqlite3_wsd_init(จำนวนเต็ม N, จำนวนเต็ม J){
  ถ้า( !pGlobal ){
    จำนวนเต็ม nMalloc = N + parimaa_na(ProcessLocalStorage) + J*parimaa_na(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    ถ้า( pGlobal ){
      memset(pGlobal, 0, parimaa_na(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - parimaa_na(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  คืน pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

void *sqlite3_wsd_find(void *K, จำนวนเต็ม L){
  จำนวนเต็ม i;
  จำนวนเต็ม iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  สำหรับ(i=0; i<parimaa_na(void*); i++){
    iHash = (iHash<<3) + ((unsigned var_na *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  สำหรับ(pVar=pGlobal->aData[iHash]; pVar && pVar->pKey!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  ถ้า( !pVar ){
    จำนวนเต็ม nByte = ROUND8(parimaa_na(ProcessLocalVar) + L);
    assert( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->pKey = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  คืน (void *)&pVar[1];
}

#endif
