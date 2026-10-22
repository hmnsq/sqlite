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

#सचे defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#अन्तर्भाव "sqliteInt.h"

#define PLS_HASHSIZE 43

प्रकारनिर्देश संरचना ProcessLocalStorage ProcessLocalStorage;
प्रकारनिर्देश संरचना ProcessLocalVar ProcessLocalVar;

संरचना ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  पूर्णाङ्क nFree;
  u8 *pFree;
};

संरचना ProcessLocalVar {
  शून्य *p_कुञ्चिका;
  ProcessLocalVar *pNext;
};

स्थिर ProcessLocalStorage *pGlobal = 0;

पूर्णाङ्क sqlite3_wsd_init(पूर्णाङ्क N, पूर्णाङ्क J){
  सचे( !pGlobal ){
    पूर्णाङ्क nMalloc = N + परिमाण(ProcessLocalStorage) + J*परिमाण(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    सचे( pGlobal ){
      memset(pGlobal, 0, परिमाण(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - परिमाण(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  पटिनिवत्त pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

शून्य *sqlite3_wsd_find(शून्य *K, पूर्णाङ्क L){
  पूर्णाङ्क i;
  पूर्णाङ्क iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  अत्थाय(i=0; i<परिमाण(शून्य*); i++){
    iHash = (iHash<<3) + ((unsigned वर्ण *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  अत्थाय(pVar=pGlobal->aData[iHash]; pVar && pVar->p_कुञ्चिका!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  सचे( !pVar ){
    पूर्णाङ्क nByte = ROUND8(परिमाण(ProcessLocalVar) + L);
    प्रतिज्ञा( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_कुञ्चिका = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  पटिनिवत्त (शून्य *)&pVar[1];
}

#endif
