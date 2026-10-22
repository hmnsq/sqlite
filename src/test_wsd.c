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

#якщо defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#include "sqliteInt.h"

#define PLS_HASHSIZE 43

typedef struct ProcessLocalStorage ProcessLocalStorage;
typedef struct ProcessLocalVar ProcessLocalVar;

struct ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  ціле nFree;
  u8 *pFree;
};

struct ProcessLocalVar {
  void *pKey;
  ProcessLocalVar *pNext;
};

static ProcessLocalStorage *pGlobal = 0;

ціле sqlite3_wsd_init(ціле N, ціле J){
  якщо( !pGlobal ){
    ціле nMalloc = N + sizeof(ProcessLocalStorage) + J*sizeof(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    якщо( pGlobal ){
      memset(pGlobal, 0, sizeof(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - sizeof(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  повернути pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

void *sqlite3_wsd_find(void *K, ціле L){
  ціле i;
  ціле iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  для(i=0; i<sizeof(void*); i++){
    iHash = (iHash<<3) + ((unsigned char *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  для(pVar=pGlobal->aData[iHash]; pVar && pVar->pKey!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  якщо( !pVar ){
    ціле nByte = ROUND8(sizeof(ProcessLocalVar) + L);
    assert( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->pKey = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  повернути (void *)&pVar[1];
}

#endif
