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

#எனில் defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#அந்தர்பாவ "sqliteInt.h"

#define PLS_HASHSIZE 43

வகைவரை கட்டமைப்பு ProcessLocalStorage ProcessLocalStorage;
வகைவரை கட்டமைப்பு ProcessLocalVar ProcessLocalVar;

கட்டமைப்பு ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  முழு nFree;
  u8 *pFree;
};

கட்டமைப்பு ProcessLocalVar {
  வெறுமை *p_சாவி;
  ProcessLocalVar *pNext;
};

நிலையான ProcessLocalStorage *pGlobal = 0;

முழு sqlite3_wsd_init(முழு N, முழு J){
  எனில்( !pGlobal ){
    முழு nMalloc = N + அளவு(ProcessLocalStorage) + J*அளவு(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    எனில்( pGlobal ){
      memset(pGlobal, 0, அளவு(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - அளவு(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  திரும்பு pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

வெறுமை *sqlite3_wsd_find(வெறுமை *K, முழு L){
  முழு i;
  முழு iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  க்கு(i=0; i<அளவு(வெறுமை*); i++){
    iHash = (iHash<<3) + ((unsigned எழுத்து *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  க்கு(pVar=pGlobal->aData[iHash]; pVar && pVar->p_சாவி!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  எனில்( !pVar ){
    முழு nByte = ROUND8(அளவு(ProcessLocalVar) + L);
    ப்ரதிஜ்ஞா( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_சாவி = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  திரும்பு (வெறுமை *)&pVar[1];
}

#endif
