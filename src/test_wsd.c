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

#যদি defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#অন্তর্ভাব "sqliteInt.h"

#define PLS_HASHSIZE 43

প্রকারলেখ কাঠামো ProcessLocalStorage ProcessLocalStorage;
প্রকারলেখ কাঠামো ProcessLocalVar ProcessLocalVar;

কাঠামো ProcessLocalStorage {
  ProcessLocalVar *a_তথ্য[PLS_HASHSIZE];
  পূর্ণসংখ্যা nFree;
  u8 *pFree;
};

কাঠামো ProcessLocalVar {
  শূন্য *p_চাবি;
  ProcessLocalVar *p_পরবর্তী;
};

স্থির ProcessLocalStorage *pGlobal = 0;

পূর্ণসংখ্যা sqlite3_wsd_init(পূর্ণসংখ্যা N, পূর্ণসংখ্যা J){
  যদি( !pGlobal ){
    পূর্ণসংখ্যা nMalloc = N + আকার(ProcessLocalStorage) + J*আকার(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)malloc(nMalloc);
    যদি( pGlobal ){
      memset(pGlobal, 0, আকার(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - আকার(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  ফেরত pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

শূন্য *sqlite3_wsd_find(শূন্য *K, পূর্ণসংখ্যা L){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  জন্য(i=0; i<আকার(শূন্য*); i++){
    iHash = (iHash<<3) + ((unsigned অক্ষর *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  জন্য(pVar=pGlobal->a_তথ্য[iHash]; pVar && pVar->p_চাবি!=K; pVar=pVar->p_পরবর্তী);

  /* If no entry for K was found, create and populate a new one. */
  যদি( !pVar ){
    পূর্ণসংখ্যা nByte = ROUND8(আকার(ProcessLocalVar) + L);
    প্রতিজ্ঞা( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_চাবি = K;
    pVar->p_পরবর্তী = pGlobal->a_তথ্য[iHash];
    pGlobal->a_তথ্য[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    memcpy(&pVar[1], K, L);
  }

  ফেরত (শূন্য *)&pVar[1];
}

#endif
