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

#اگر defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#شامل "sqliteInt.h"

#تعریف_کرو PLS_HASHSIZE 43

قسمتعریف ساخت ProcessLocalStorage ProcessLocalStorage;
قسمتعریف ساخت ProcessLocalVar ProcessLocalVar;

ساخت ProcessLocalStorage {
  ProcessLocalVar *a_ڈیٹا[PLS_HASHSIZE];
  صحیح nFree;
  u8 *pFree;
};

ساخت ProcessLocalVar {
  خالی *p_کلید;
  ProcessLocalVar *p_اگلا;
};

ساکن ProcessLocalStorage *pGlobal = 0;

صحیح sqlite3_wsd_init(صحیح N, صحیح J){
  اگر( !pGlobal ){
    صحیح nMalloc = N + حجم(ProcessLocalStorage) + J*حجم(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)حافظہ_دو(nMalloc);
    اگر( pGlobal ){
      memset(pGlobal, 0, حجم(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - حجم(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  واپس pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

خالی *sqlite3_wsd_ڈھونڈو(خالی *K, صحیح L){
  صحیح i;
  صحیح iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  برائے(i=0; i<حجم(خالی*); i++){
    iHash = (iHash<<3) + ((غیرعلامتی حرف *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  برائے(pVar=pGlobal->a_ڈیٹا[iHash]; pVar && pVar->p_کلید!=K; pVar=pVar->p_اگلا);

  /* If no entry for K was found, create and populate a new one. */
  اگر( !pVar ){
    صحیح nByte = ROUND8(حجم(ProcessLocalVar) + L);
    أكد( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_کلید = K;
    pVar->p_اگلا = pGlobal->a_ڈیٹا[iHash];
    pGlobal->a_ڈیٹا[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    حافظہ_نقل(&pVar[1], K, L);
  }

  واپس (خالی *)&pVar[1];
}

#اگر_ختم
