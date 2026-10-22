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

#إذا defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#ضمن "sqliteInt.h"

#تعریف_کرو PLS_HASHSIZE 43

تعريفنوع بنية ProcessLocalStorage ProcessLocalStorage;
تعريفنوع بنية ProcessLocalVar ProcessLocalVar;

بنية ProcessLocalStorage {
  ProcessLocalVar *a_انکڑے[PLS_HASHSIZE];
  صحيح nFree;
  u8 *pFree;
};

بنية ProcessLocalVar {
  فراغ *p_کلید;
  ProcessLocalVar *p_اگلا;
};

ساكن ProcessLocalStorage *pGlobal = 0;

صحيح sqlite3_wsd_init(صحيح N, صحيح J){
  إذا( !pGlobal ){
    صحيح nMalloc = N + حجم(ProcessLocalStorage) + J*حجم(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)حافظہ_دیو(nMalloc);
    إذا( pGlobal ){
      memset(pGlobal, 0, حجم(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - حجم(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  أرجع pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

فراغ *sqlite3_wsd_لبھو(فراغ *K, صحيح L){
  صحيح i;
  صحيح iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  لكل(i=0; i<حجم(فراغ*); i++){
    iHash = (iHash<<3) + ((بنانشانی حرف *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  لكل(pVar=pGlobal->a_انکڑے[iHash]; pVar && pVar->p_کلید!=K; pVar=pVar->p_اگلا);

  /* If no entry for K was found, create and populate a new one. */
  إذا( !pVar ){
    صحيح nByte = ROUND8(حجم(ProcessLocalVar) + L);
    أكد( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_کلید = K;
    pVar->p_اگلا = pGlobal->a_انکڑے[iHash];
    pGlobal->a_انکڑے[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    حافظہ_نقل(&pVar[1], K, L);
  }

  أرجع (فراغ *)&pVar[1];
}

#جے_انت
