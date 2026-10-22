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

#अगर defined(SQLITE_OMIT_WSD) && defined(SQLITE_TEST)

#समावेश "sqliteInt.h"

#घोषणा PLS_HASHSIZE 43

प्रकार काष्ठा ProcessLocalStorage ProcessLocalStorage;
प्रकार काष्ठा ProcessLocalVar ProcessLocalVar;

काष्ठा ProcessLocalStorage {
  ProcessLocalVar *a_तथ्य[PLS_HASHSIZE];
  पूर्णांक nFree;
  u8 *pFree;
};

काष्ठा ProcessLocalVar {
  व्योम *p_कुंजी;
  ProcessLocalVar *p_अग्र;
};

अटल ProcessLocalStorage *pGlobal = 0;

पूर्णांक sqlite3_wsd_init(पूर्णांक N, पूर्णांक J){
  अगर( !pGlobal ){
    पूर्णांक nMalloc = N + माप(ProcessLocalStorage) + J*माप(ProcessLocalVar);
    pGlobal = (ProcessLocalStorage *)दो_स्मृति(nMalloc);
    अगर( pGlobal ){
      स_रखो(pGlobal, 0, माप(ProcessLocalStorage));
      pGlobal->nFree = nMalloc - माप(ProcessLocalStorage);
      pGlobal->pFree = (u8 *)&pGlobal[1];
    }
  }

  वापस pGlobal ? SQLITE_OK : SQLITE_NOMEM;
}

व्योम *sqlite3_wsd_find(व्योम *K, पूर्णांक L){
  पूर्णांक i;
  पूर्णांक iHash = 0;
  ProcessLocalVar *pVar;

  /* Calculate a hash of K */
  क्रम(i=0; i<माप(व्योम*); i++){
    iHash = (iHash<<3) + ((अचिन्हित अक्षर *)&K)[i];
  }
  iHash = iHash%PLS_HASHSIZE;

  /* Search the hash table for K. */
  क्रम(pVar=pGlobal->a_तथ्य[iHash]; pVar && pVar->p_कुंजी!=K; pVar=pVar->p_अग्र);

  /* If no entry for K was found, create and populate a new one. */
  अगर( !pVar ){
    पूर्णांक nByte = ROUND8(माप(ProcessLocalVar) + L);
    निश्चित( pGlobal->nFree>=nByte );
    pVar = (ProcessLocalVar *)pGlobal->pFree;
    pVar->p_कुंजी = K;
    pVar->p_अग्र = pGlobal->a_तथ्य[iHash];
    pGlobal->a_तथ्य[iHash] = pVar;
    pGlobal->nFree -= nByte;
    pGlobal->pFree += nByte;
    स_नकल(&pVar[1], K, L);
  }

  वापस (व्योम *)&pVar[1];
}

#पूर्ण_अगर
