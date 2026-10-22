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

#यदि defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#अन्तर्भाव "sqliteInt.h"

#परिभाषय PLS_HASHSIZE 43

प्रकारनिर्देश संरचना ProcessLocalStorage ProcessLocalStorage;
प्रकारनिर्देश संरचना ProcessLocalVar ProcessLocalVar;

संरचना ProcessLocalStorage {
  ProcessLocalVar *a_दत्त[PLS_HASHSIZE];
  पूर्णाङ्क nFree;
  u8 *pFree;
};

संरचना ProcessLocalVar {
  शून्य *p_कुञ्चिका;
  ProcessLocalVar *p_अग्र;
};

स्थिर ProcessLocalStorage *pGlobal = 0;

पूर्णाङ्क sqlite3_wsd_init(पूर्णाङ्क N, पूर्णाङ्क J){
  यदि( !pGlobal ){
    पूर्णाङ्क nMalloc = N + परिमाण(ProcessLocalStorage) + J*परिमाण(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)स्मृतिं_देहि(nMalloc);
    यदि( pGlobal ){
      स्मृतिं_पूरय(pGlobal, 0, परिमाण(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - परिमाण(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  प्रत्यागम pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

शून्य *sqlite3_wsd_find(शून्य *K, पूर्णाङ्क L){
  पूर्णाङ्क i;
  पूर्णाङ्क iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  प्रति(i=0; i<परिमाण(शून्य*); i++){
    iHash = (iHash<<3) + ((अचिह्नित वर्ण *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  प्रति(pVar=pGlobal->a_दत्त[iHash]; pVar && pVar->p_कुञ्चिका!=K; pVar=pVar->p_अग्र);

  /* If no entry for K was found, create and populate a new one. */
  यदि( !pVar ){
    पूर्णाङ्क nByte = ROUND8(परिमाण(ProcessLocalVar) + L);
    प्रतिज्ञा( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_कुञ्चिका = K;
    pVar->p_अग्र = pGlobal->a_दत्त[iHash];
    pGlobal->a_दत्त[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    स्मृतिं_प्रतिलिख(&pVar[1], K, L);
  }

  प्रत्यागम (शून्य *)&pVar[1];
}

#यदि_अन्तः
