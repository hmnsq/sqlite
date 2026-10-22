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

#ఒకవేళ defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#అన్తర్భావ "sqliteInt.h"

#నిర్వచించు PLS_HASHSIZE 43

రకనిర్వచనం నిర్మాణం ProcessLocalStorage ProcessLocalStorage;
రకనిర్వచనం నిర్మాణం ProcessLocalVar ProcessLocalVar;

నిర్మాణం ProcessLocalStorage {
  ProcessLocalVar *a_దత్తాంశం[PLS_HASHSIZE];
  పూర్ణాంకం nFree;
  u8 *pFree;
};

నిర్మాణం ProcessLocalVar {
  శూన్య *p_కీలకం;
  ProcessLocalVar *p_తదుపరి;
};

స్థిర ProcessLocalStorage *pGlobal = 0;

పూర్ణాంకం sqlite3_wsd_init(పూర్ణాంకం N, పూర్ణాంకం J){
  ఒకవేళ( !pGlobal ){
    పూర్ణాంకం nMalloc = N + పరిమాణం(ProcessLocalStorage) + J*పరిమాణం(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)జ్ఞాపకం_ఇవ్వు(nMalloc);
    ఒకవేళ( pGlobal ){
      memset(pGlobal, 0, పరిమాణం(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - పరిమాణం(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  తిరిగి pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

శూన్య *sqlite3_wsd_కనుగొను(శూన్య *K, పూర్ణాంకం L){
  పూర్ణాంకం i;
  పూర్ణాంకం iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  కొరకు(i=0; i<పరిమాణం(శూన్య*); i++){
    iHash = (iHash<<3) + ((గుర్తులేని అక్షరం *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  కొరకు(pVar=pGlobal->a_దత్తాంశం[iHash]; pVar && pVar->p_కీలకం!=K; pVar=pVar->p_తదుపరి);

  /* If no entry for K was found, create and populate a new one. */
  ఒకవేళ( !pVar ){
    పూర్ణాంకం nByte = ROUND8(పరిమాణం(ProcessLocalVar) + L);
    ప్రతిజ్ఞా( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_కీలకం = K;
    pVar->p_తదుపరి = pGlobal->a_దత్తాంశం[iHash];
    pGlobal->a_దత్తాంశం[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    జ్ఞాపకం_నకలు(&pVar[1], K, L);
  }

  తిరిగి (శూన్య *)&pVar[1];
}

#అయితే_ముగింపు
