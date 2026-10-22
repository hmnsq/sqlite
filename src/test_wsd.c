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

#اگر defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#شامل "sqliteInt.h"

#define PLS_HASHSIZE 43

تعریفنوع ساختار ProcessLocalStorage ProcessLocalStorage;
تعریفنوع ساختار ProcessLocalVar ProcessLocalVar;

ساختار ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  صحیح nFree;
  u8 *pFree;
};

ساختار ProcessLocalVar {
  تهی *p_کلید;
  ProcessLocalVar *pNext;
};

ایستا ProcessLocalStorage *pGlobal = 0;

صحیح sqlite3_wsd_init(صحیح N, صحیح J){
  اگر( !pGlobal ){
    صحیح nMalloc = N + اندازه(ProcessLocalStorage) + J*اندازه(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    اگر( pGlobal ){
      memset(pGlobal, 0, اندازه(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - اندازه(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  بازگشت pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

تهی *sqlite3_wsd_find(تهی *K, صحیح L){
  صحیح i;
  صحیح iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  برای(i=0; i<اندازه(تهی*); i++){
    iHash = (iHash<<3) + ((unsigned نویسه *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  برای(pVar=pGlobal->aData[iHash]; pVar && pVar->p_کلید!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  اگر( !pVar ){
    صحیح nByte = ROUND8(اندازه(ProcessLocalVar) + L);
    أكد( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_کلید = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  بازگشت (تهی *)&pVar[1];
}

#endif
