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

#જો defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#અન્તર્ભાવ "sqliteInt.h"

#define PLS_HASHSIZE 43

પ્રકારવ્યાખ્યા સંરચના ProcessLocalStorage ProcessLocalStorage;
પ્રકારવ્યાખ્યા સંરચના ProcessLocalVar ProcessLocalVar;

સંરચના ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  પૂર્ણાંક nFree;
  u8 *pFree;
};

સંરચના ProcessLocalVar {
  શૂન્ય *p_ચાવી;
  ProcessLocalVar *pNext;
};

સ્થિર ProcessLocalStorage *pGlobal = 0;

પૂર્ણાંક sqlite3_wsd_init(પૂર્ણાંક N, પૂર્ણાંક J){
  જો( !pGlobal ){
    પૂર્ણાંક nMalloc = N + કદ(ProcessLocalStorage) + J*કદ(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    જો( pGlobal ){
      memset(pGlobal, 0, કદ(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - કદ(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  પાછા pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

શૂન્ય *sqlite3_wsd_find(શૂન્ય *K, પૂર્ણાંક L){
  પૂર્ણાંક i;
  પૂર્ણાંક iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  માટે(i=0; i<કદ(શૂન્ય*); i++){
    iHash = (iHash<<3) + ((unsigned અક્ષર *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  માટે(pVar=pGlobal->aData[iHash]; pVar && pVar->p_ચાવી!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  જો( !pVar ){
    પૂર્ણાંક nByte = ROUND8(કદ(ProcessLocalVar) + L);
    પ્રતિજ્ઞા( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_ચાવી = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  પાછા (શૂન્ય *)&pVar[1];
}

#endif
