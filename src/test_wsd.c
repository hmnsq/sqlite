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

#ಆದರೆ defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ವ್ಯಾಖ್ಯಾನಿಸು PLS_HASHSIZE 43

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ProcessLocalStorage ProcessLocalStorage;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ProcessLocalVar ProcessLocalVar;

ರಚನೆ ProcessLocalStorage {
  ProcessLocalVar *a_ದತ್ತಾಂಶ[PLS_HASHSIZE];
  ಪೂರ್ಣಾಂಕ nFree;
  u8 *pFree;
};

ರಚನೆ ProcessLocalVar {
  ಶೂನ್ಯ *p_ಕೀಲಿ;
  ProcessLocalVar *p_ಮುಂದಿನ;
};

ಸ್ಥಿರ ProcessLocalStorage *pGlobal = 0;

ಪೂರ್ಣಾಂಕ sqlite3_wsd_init(ಪೂರ್ಣಾಂಕ N, ಪೂರ್ಣಾಂಕ J){
  ಆದರೆ( !pGlobal ){
    ಪೂರ್ಣಾಂಕ nMalloc = N + ಗಾತ್ರ(ProcessLocalStorage) + J*ಗಾತ್ರ(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)ಸ್ಮರಣೆ_ಕೊಡು(nMalloc);
    ಆದರೆ( pGlobal ){
      memset(pGlobal, 0, ಗಾತ್ರ(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - ಗಾತ್ರ(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  ಹಿಂತಿರುಗಿ pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

ಶೂನ್ಯ *sqlite3_wsd_ಕಂಡುಹಿಡಿ(ಶೂನ್ಯ *K, ಪೂರ್ಣಾಂಕ L){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(ಶೂನ್ಯ*); i++){
    iHash = (iHash<<3) + ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  ಗಾಗಿ(pVar=pGlobal->a_ದತ್ತಾಂಶ[iHash]; pVar && pVar->p_ಕೀಲಿ!=K; pVar=pVar->p_ಮುಂದಿನ);

  /* If no entry for K was found, create and populate a new one. */
  ಆದರೆ( !pVar ){
    ಪೂರ್ಣಾಂಕ nByte = ROUND8(ಗಾತ್ರ(ProcessLocalVar) + L);
    ಪ್ರತಿಜ್ಞಾ( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_ಕೀಲಿ = K;
    pVar->p_ಮುಂದಿನ = pGlobal->a_ದತ್ತಾಂಶ[iHash];
    pGlobal->a_ದತ್ತಾಂಶ[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    ಸ್ಮರಣೆ_ನಕಲು(&pVar[1], K, L);
  }

  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)&pVar[1];
}

#ಆದರೆ_ಅಂತ್ಯ
