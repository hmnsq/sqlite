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

#എങ്കിൽ defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#അന്തര്ഭാവ "sqliteInt.h"

#define PLS_HASHSIZE 43

തരനിർവചനം ഘടന ProcessLocalStorage ProcessLocalStorage;
തരനിർവചനം ഘടന ProcessLocalVar ProcessLocalVar;

ഘടന ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  പൂർണ്ണസംഖ്യ nFree;
  u8 *pFree;
};

ഘടന ProcessLocalVar {
  ശൂന്യം *p_താക്കോൽ;
  ProcessLocalVar *pNext;
};

സ്ഥിതം ProcessLocalStorage *pGlobal = 0;

പൂർണ്ണസംഖ്യ sqlite3_wsd_init(പൂർണ്ണസംഖ്യ N, പൂർണ്ണസംഖ്യ J){
  എങ്കിൽ( !pGlobal ){
    പൂർണ്ണസംഖ്യ nMalloc = N + വലുപ്പം(ProcessLocalStorage) + J*വലുപ്പം(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    എങ്കിൽ( pGlobal ){
      memset(pGlobal, 0, വലുപ്പം(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - വലുപ്പം(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  തിരികെ pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

ശൂന്യം *sqlite3_wsd_find(ശൂന്യം *K, പൂർണ്ണസംഖ്യ L){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  വേണ്ടി(i=0; i<വലുപ്പം(ശൂന്യം*); i++){
    iHash = (iHash<<3) + ((unsigned അക്ഷരം *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  വേണ്ടി(pVar=pGlobal->aData[iHash]; pVar && pVar->p_താക്കോൽ!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  എങ്കിൽ( !pVar ){
    പൂർണ്ണസംഖ്യ nByte = ROUND8(വലുപ്പം(ProcessLocalVar) + L);
    പ്രതിജ്ഞാ( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_താക്കോൽ = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  തിരികെ (ശൂന്യം *)&pVar[1];
}

#endif
