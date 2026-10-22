/*
** 2008 November 18
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
** This file contains code used for testing the SQLite system.
** None of the code in this file goes into a deliverable build.
**
** This file contains an application-defined pager cache
** implementation that can be plugged in in place of the
** default pcache.  This alternative pager cache will throw
** some errors that the default cache does not.
**
** This pagecache implementation is designed for simplicity
** not speed.
*/
#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
തരനിർവചനം ഘടന testpcacheGlobalType testpcacheGlobalType;
ഘടന testpcacheGlobalType {
  ശൂന്യം *pDummy;             /* Dummy allocation to simulate failures */
  പൂർണ്ണസംഖ്യ nInstance;            /* Number of current instances */
  unsigned discardChance;   /* Chance of discarding on an unpin (0-100) */
  unsigned prngSeed;        /* Seed for the PRNG */
  unsigned മുകളിലെ_Stress;      /* Call xStress aggressively */
};
സ്ഥിതം testpcacheGlobalType testpcacheGlobal;

/*
** Initializer.
**
** Verify that the initializer is only called when the system is
** uninitialized.  Allocate some memory and report SQLITE_NOMEM if
** the allocation fails.  This provides a means to test the recovery
** from a failed initialization attempt.  It also verifies that the
** the destructor always gets call - otherwise there would be a
** memory leak.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ testpcacheInit(ശൂന്യം *pArg){
  പ്രതിജ്ഞാ( pArg==(ശൂന്യം*)&testpcacheGlobal );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy==0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  തിരികെ testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
സ്ഥിതം ശൂന്യം testpcacheShutdown(ശൂന്യം *pArg){
  പ്രതിജ്ഞാ( pArg==(ശൂന്യം*)&testpcacheGlobal );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance==0 );
  sqlite3_free( testpcacheGlobal.pDummy );
  testpcacheGlobal.pDummy = 0;
}

/*
** Number of pages in a cache.
**
** The number of pages is a hard upper bound in this test module.
** If more pages are requested, sqlite3PcacheFetch() returns NULL.
**
** If testing with in-memory temp tables, provide a larger pcache.
** Some of the test cases need this.
*/
#എങ്കിൽ defined(SQLITE_TEMP_STORE) && SQLITE_TEMP_STORE>=2
# define TESTPCACHE_NPAGE    499
#അല്ലെങ്കിൽ
# define TESTPCACHE_NPAGE    217
#endif
#define TESTPCACHE_RESERVE   17

/*
** Magic numbers used to determine validity of the page cache.
*/
#define TESTPCACHE_VALID  0x364585fd
#define TESTPCACHE_CLEAR  0xd42670d4

/*
** Private implementation of a page cache.
*/
തരനിർവചനം ഘടന testpcache testpcache;
ഘടന testpcache {
  sqlite3_int64 szPage;     /* Size of each page.  Multiple of 8. */
  പൂർണ്ണസംഖ്യ szExtra;              /* Size of extra data that accompanies each page */
  പൂർണ്ണസംഖ്യ bPurgeable;           /* True if the page cache is purgeable */
  പൂർണ്ണസംഖ്യ nFree;                /* Number of unused slots in a[] */
  പൂർണ്ണസംഖ്യ nPinned;              /* Number of pinned slots in a[] */
  unsigned iRand;           /* State of the PRNG */
  unsigned iMagic;          /* Magic number for sanity checking */
  ഘടന testpcachePage {
    sqlite3_pcache_page page;  /* Base class */
    unsigned താക്കോൽ;              /* The key for this page. 0 means unallocated */
    പൂർണ്ണസംഖ്യ isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
സ്ഥിതം unsigned testpcacheRandom(testpcache *p){
  unsigned x = 0;
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  തിരികെ x;
}


/*
** Allocate a new page cache instance.
*/
സ്ഥിതം sqlite3_pcache *testpcacheCreate(
  പൂർണ്ണസംഖ്യ szPage,
  പൂർണ്ണസംഖ്യ szExtra,
  പൂർണ്ണസംഖ്യ bPurgeable
){
  പൂർണ്ണസംഖ്യ nMem;
  അക്ഷരം *x;
  testpcache *p;
  പൂർണ്ണസംഖ്യ i;
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  szPage = (szPage+7)&~7;
  szExtra = (szPage+7)&~7;
  nMem = വലുപ്പം(testpcache) + TESTPCACHE_NPAGE*(szPage+szExtra);
  p = sqlite3_malloc( nMem );
  എങ്കിൽ( p==0 ) തിരികെ 0;
  x = (അക്ഷരം*)&p[1];
  p->szPage = szPage;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prngSeed;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  വേണ്ടി(i=0; i<TESTPCACHE_NPAGE; i++, x += (szPage+szExtra)){
    p->a[i].താക്കോൽ = 0;
    p->a[i].isPinned = 0;
    p->a[i].page.pBuf = (ശൂന്യം*)x;
    p->a[i].page.pExtra = (ശൂന്യം*)&x[szPage];
  }
  testpcacheGlobal.nInstance++;
  തിരികെ (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
സ്ഥിതം ശൂന്യം testpcacheCachesize(sqlite3_pcache *pCache, പൂർണ്ണസംഖ്യ newSize){
  testpcache *p = (testpcache*)pCache;
  പ്രതിജ്ഞാ( p->iMagic==TESTPCACHE_VALID );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  പ്രതിജ്ഞാ( p->iMagic==TESTPCACHE_VALID );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance>0 );
  തിരികെ TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
സ്ഥിതം sqlite3_pcache_page *testpcacheFetch(
  sqlite3_pcache *pCache,
  unsigned താക്കോൽ,
  പൂർണ്ണസംഖ്യ createFlag
){
  testpcache *p = (testpcache*)pCache;
  പൂർണ്ണസംഖ്യ i, j;
  പ്രതിജ്ഞാ( p->iMagic==TESTPCACHE_VALID );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  വേണ്ടി(i=0; i<TESTPCACHE_NPAGE; i++){
    എങ്കിൽ( p->a[i].താക്കോൽ==താക്കോൽ ){
      എങ്കിൽ( !p->a[i].isPinned ){
        p->nPinned++;
        പ്രതിജ്ഞാ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      തിരികെ &p->a[i].page;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  എങ്കിൽ( createFlag==0 ){
    തിരികെ 0;
  }

  /* If no pages are available, always fail */
  എങ്കിൽ( p->nPinned==TESTPCACHE_NPAGE ){
    തിരികെ 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  എങ്കിൽ( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    തിരികെ 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  എങ്കിൽ( testpcacheGlobal.മുകളിലെ_Stress && createFlag<2 ){
    തിരികെ 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  എങ്കിൽ( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    വേണ്ടി(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      എങ്കിൽ( p->a[j].താക്കോൽ==0 ){
        p->a[j].താക്കോൽ = താക്കോൽ;
        p->a[j].isPinned = 1;
        memset(p->a[j].page.pBuf, 0, p->szPage);
        memset(p->a[j].page.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        പ്രതിജ്ഞാ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        തിരികെ &p->a[j].page;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    പ്രതിജ്ഞാ( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  എങ്കിൽ( p->bPurgeable==0 ){
    തിരികെ 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  വേണ്ടി(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    എങ്കിൽ( p->a[j].താക്കോൽ>0 && p->a[j].isPinned==0 ){
      p->a[j].താക്കോൽ = താക്കോൽ;
      p->a[j].isPinned = 1;
      memset(p->a[j].page.pBuf, 0, p->szPage);
      memset(p->a[j].page.pExtra, 0, p->szExtra);
      p->nPinned++;
      പ്രതിജ്ഞാ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      തിരികെ &p->a[j].page;
    }
  }

  /* The previous loop always finds a page to recycle. */
  പ്രതിജ്ഞാ(0);
  തിരികെ 0;
}

/*
** Unpin a page.
*/
സ്ഥിതം ശൂന്യം testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  പൂർണ്ണസംഖ്യ discard
){
  testpcache *p = (testpcache*)pCache;
  പൂർണ്ണസംഖ്യ i;
  പ്രതിജ്ഞാ( p->iMagic==TESTPCACHE_VALID );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  എങ്കിൽ( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  വേണ്ടി(i=0; i<TESTPCACHE_NPAGE; i++){
    എങ്കിൽ( &p->a[i].page==pOldPage ){
      /* The pOldPage pointer always points to a pinned page */
      പ്രതിജ്ഞാ( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      പ്രതിജ്ഞാ( p->nPinned>=0 );
      എങ്കിൽ( discard ){
        p->a[i].താക്കോൽ = 0;
        p->nFree++;
        പ്രതിജ്ഞാ( p->nFree<=TESTPCACHE_NPAGE );
      }
      തിരികെ;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  പ്രതിജ്ഞാ( 0 );
}


/*
** Rekey a single page.
*/
സ്ഥിതം ശൂന്യം testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  unsigned old_താക്കോൽ,
  unsigned new_താക്കോൽ
){
  testpcache *p = (testpcache*)pCache;
  പൂർണ്ണസംഖ്യ i;
  പ്രതിജ്ഞാ( p->iMagic==TESTPCACHE_VALID );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  വേണ്ടി(i=0; i<TESTPCACHE_NPAGE; i++){
    എങ്കിൽ( p->a[i].താക്കോൽ==new_താക്കോൽ ){
      /* The new key is never a page that is already pinned */
      പ്രതിജ്ഞാ( p->a[i].isPinned==0 );
      p->a[i].താക്കോൽ = 0;
      p->nFree++;
      പ്രതിജ്ഞാ( p->nFree<=TESTPCACHE_NPAGE );
      നിർത്തുക;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  വേണ്ടി(i=0; i<TESTPCACHE_NPAGE; i++){
    എങ്കിൽ( p->a[i].താക്കോൽ==old_താക്കോൽ ){
      /* The oldKey and pOldPage parameters match */
      പ്രതിജ്ഞാ( &p->a[i].page==pOldPage );
      /* Page to be rekeyed must be pinned */
      പ്രതിജ്ഞാ( p->a[i].isPinned );
      p->a[i].താക്കോൽ = new_താക്കോൽ;
      തിരികെ;
    }
  }

  /* Rekey is always given a valid page to work with */
  പ്രതിജ്ഞാ( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
സ്ഥിതം ശൂന്യം testpcacheTruncate(sqlite3_pcache *pCache, unsigned iLimit){
  testpcache *p = (testpcache*)pCache;
  unsigned പൂർണ്ണസംഖ്യ i;
  പ്രതിജ്ഞാ( p->iMagic==TESTPCACHE_VALID );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance>0 );
  വേണ്ടി(i=0; i<TESTPCACHE_NPAGE; i++){
    എങ്കിൽ( p->a[i].താക്കോൽ>=iLimit ){
      p->a[i].താക്കോൽ = 0;
      എങ്കിൽ( p->a[i].isPinned ){
        p->nPinned--;
        പ്രതിജ്ഞാ( p->nPinned>=0 );
      }
      p->nFree++;
      പ്രതിജ്ഞാ( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
സ്ഥിതം ശൂന്യം testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  പ്രതിജ്ഞാ( p->iMagic==TESTPCACHE_VALID );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy!=0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance>0 );
  p->iMagic = TESTPCACHE_CLEAR;
  sqlite3_free(p);
  testpcacheGlobal.nInstance--;
}


/*
** Invoke this routine to register or unregister the testing pager cache
** implemented by this file.
**
** Install the test pager cache if installFlag is 1 and uninstall it if
** installFlag is 0.
**
** When installing, discardChance is a number between 0 and 100 that
** indicates the probability of discarding a page when unpinning the
** page.  0 means never discard (unless the discard flag is set).
** 100 means always discard.
*/
ശൂന്യം installTestPCache(
  പൂർണ്ണസംഖ്യ installFlag,            /* True to install.  False to uninstall. */
  unsigned discardChance,     /* 0-100.  Chance to discard on unpin */
  unsigned prngSeed,          /* Seed for the PRNG */
  unsigned മുകളിലെ_Stress         /* Call xStress aggressively */
){
  സ്ഥിതം സ്ഥിരം sqlite3_pcache_methods2 testPcache = {
    1,
    (ശൂന്യം*)&testpcacheGlobal,
    testpcacheInit,
    testpcacheShutdown,
    testpcacheCreate,
    testpcacheCachesize,
    testpcachePagecount,
    testpcacheFetch,
    testpcacheUnpin,
    testpcacheRekey,
    testpcacheTruncate,
    testpcacheDestroy,
  };
  സ്ഥിതം sqlite3_pcache_methods2 defaultPcache;
  സ്ഥിതം പൂർണ്ണസംഖ്യ isInstalled = 0;

  പ്രതിജ്ഞാ( testpcacheGlobal.nInstance==0 );
  പ്രതിജ്ഞാ( testpcacheGlobal.pDummy==0 );
  പ്രതിജ്ഞാ( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prngSeed = prngSeed ^ (prngSeed<<16);
  testpcacheGlobal.മുകളിലെ_Stress = മുകളിലെ_Stress;
  എങ്കിൽ( installFlag!=isInstalled ){
    എങ്കിൽ( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      പ്രതിജ്ഞാ( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }അല്ലെങ്കിൽ{
      പ്രതിജ്ഞാ( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
