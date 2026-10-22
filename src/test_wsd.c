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

#ಯದಿ defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#define PLS_HASHSIZE 43

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ ProcessLocalStorage ProcessLocalStorage;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ ProcessLocalVar ProcessLocalVar;

ಸಂರಚನಾ ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  ಪೂರ್ಣಾಙ್ಕ nFree;
  u8 *pFree;
};

ಸಂರಚನಾ ProcessLocalVar {
  ಶೂನ್ಯ *p_ಕೀಲಿ;
  ProcessLocalVar *pNext;
};

ಸ್ಥಿರ ProcessLocalStorage *pGlobal = 0;

ಪೂರ್ಣಾಙ್ಕ sqlite3_wsd_init(ಪೂರ್ಣಾಙ್ಕ N, ಪೂರ್ಣಾಙ್ಕ J){
  ಯದಿ( !pGlobal ){
    ಪೂರ್ಣಾಙ್ಕ nMalloc = N + ಪರಿಮಾಣ(ProcessLocalStorage) + J*ಪರಿಮಾಣ(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    ಯದಿ( pGlobal ){
      memset(pGlobal, 0, ಪರಿಮಾಣ(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - ಪರಿಮಾಣ(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  ಪ್ರತ್ಯಾಗಮ pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

ಶೂನ್ಯ *sqlite3_wsd_find(ಶೂನ್ಯ *K, ಪೂರ್ಣಾಙ್ಕ L){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(ಶೂನ್ಯ*); i++){
    iHash = (iHash<<3) + ((unsigned ವರ್ಣ *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  ಪ್ರತಿ(pVar=pGlobal->aData[iHash]; pVar && pVar->p_ಕೀಲಿ!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  ಯದಿ( !pVar ){
    ಪೂರ್ಣಾಙ್ಕ nByte = ROUND8(ಪರಿಮಾಣ(ProcessLocalVar) + L);
    ಪ್ರತಿಜ್ಞಾ( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_ಕೀಲಿ = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *)&pVar[1];
}

#endif
