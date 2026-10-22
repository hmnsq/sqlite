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

#जर defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#अन्तर्भाव "sqliteInt.h"

#define PLS_HASHSIZE 43

प्रकारव्याख्या संरचना ProcessLocalStorage ProcessLocalStorage;
प्रकारव्याख्या संरचना ProcessLocalVar ProcessLocalVar;

संरचना ProcessLocalStorage {
  ProcessLocalVar *aData[PLS_HASHSIZE];
  पूर्णांक nFree;
  u8 *pFree;
};

संरचना ProcessLocalVar {
  शून्य *p_किल्ली;
  ProcessLocalVar *pNext;
};

स्थिर ProcessLocalStorage *pGlobal = 0;

पूर्णांक sqlite3_wsd_init(पूर्णांक N, पूर्णांक J){
  जर( !pGlobal ){
    पूर्णांक nMalloc = N + आकार(ProcessLocalStorage) + J*आकार(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)दो_स्मृति(nMalloc);
    जर( pGlobal ){
      स_रखो(pGlobal, 0, आकार(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - आकार(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  परत pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

शून्य *sqlite3_wsd_find(शून्य *K, पूर्णांक L){
  पूर्णांक i;
  पूर्णांक iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  साठी(i=0; i<आकार(शून्य*); i++){
    iHash = (iHash<<3) + ((अचिन्हित अक्षर *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  साठी(pVar=pGlobal->aData[iHash]; pVar && pVar->p_किल्ली!=K; pVar=pVar->pNext);

  /* If no entry for K was found, create and populate a new one. */
  जर( !pVar ){
    पूर्णांक nByte = ROUND8(आकार(ProcessLocalVar) + L);
    प्रतिज्ञा( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_किल्ली = K;
    pVar->pNext = pGlobal->aData[iHash];
    pGlobal->aData[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    स_नकल(&pVar[1], K, L);
  }

  परत (शून्य *)&pVar[1];
}

#endif
