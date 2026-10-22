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

#ਜੇ defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PLS_HASHSIZE 43

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ProcessLocalStorage ProcessLocalStorage;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ProcessLocalVar ProcessLocalVar;

ਬਣਤਰ ProcessLocalStorage {
  ProcessLocalVar *a_ਅੰਕੜੇ[PLS_HASHSIZE];
  ਪੂਰਨ nFree;
  u8 *pFree;
};

ਬਣਤਰ ProcessLocalVar {
  ਖਾਲੀ *p_ਕੁੰਜੀ;
  ProcessLocalVar *p_ਅਗਲਾ;
};

ਸਥਿਰ ProcessLocalStorage *pGlobal = 0;

ਪੂਰਨ sqlite3_wsd_init(ਪੂਰਨ N, ਪੂਰਨ J){
  ਜੇ( !pGlobal ){
    ਪੂਰਨ nMalloc = N + ਆਕਾਰ(ProcessLocalStorage) + J*ਆਕਾਰ(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)ਸਮ੍ਰਿਤੀ_ਦਿਓ(nMalloc);
    ਜੇ( pGlobal ){
      memset(pGlobal, 0, ਆਕਾਰ(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - ਆਕਾਰ(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  ਵਾਪਸ pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

ਖਾਲੀ *sqlite3_wsd_ਲੱਭੋ(ਖਾਲੀ *K, ਪੂਰਨ L){
  ਪੂਰਨ i;
  ਪੂਰਨ iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  ਲਈ(i=0; i<ਆਕਾਰ(ਖਾਲੀ*); i++){
    iHash = (iHash<<3) + ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  ਲਈ(pVar=pGlobal->a_ਅੰਕੜੇ[iHash]; pVar && pVar->p_ਕੁੰਜੀ!=K; pVar=pVar->p_ਅਗਲਾ);

  /* If no entry for K was found, create and populate a new one. */
  ਜੇ( !pVar ){
    ਪੂਰਨ nByte = ROUND8(ਆਕਾਰ(ProcessLocalVar) + L);
    ਪ੍ਰਤਿਜ੍ਞਾ( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_ਕੁੰਜੀ = K;
    pVar->p_ਅਗਲਾ = pGlobal->a_ਅੰਕੜੇ[iHash];
    pGlobal->a_ਅੰਕੜੇ[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pVar[1], K, L);
  }

  ਵਾਪਸ (ਖਾਲੀ *)&pVar[1];
}

#ਜੇ_ਅੰਤ
