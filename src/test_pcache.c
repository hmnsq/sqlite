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
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା testpcacheGlobalType testpcacheGlobalType;
ସଂରଚନା testpcacheGlobalType {
  ଶୂନ୍ୟ *pDummy;             /* Dummy allocation to simulate failures */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nInstance;            /* Number of current instances */
  unsigned discardChance;   /* Chance of discarding on an unpin (0-100) */
  unsigned prngSeed;        /* Seed for the PRNG */
  unsigned ଉପର_Stress;      /* Call xStress aggressively */
};
ସ୍ଥିର testpcacheGlobalType testpcacheGlobal;

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା testpcacheInit(ଶୂନ୍ୟ *pArg){
  ପ୍ରତିଜ୍ଞା( pArg==(ଶୂନ୍ୟ*)&testpcacheGlobal );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy==0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  ଫେରନ୍ତୁ testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
ସ୍ଥିର ଶୂନ୍ୟ testpcacheShutdown(ଶୂନ୍ୟ *pArg){
  ପ୍ରତିଜ୍ଞା( pArg==(ଶୂନ୍ୟ*)&testpcacheGlobal );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance==0 );
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
#ଯଦି defined(SQLITE_TEMP_STORE) && SQLITE_TEMP_STORE>=2
# define TESTPCACHE_NPAGE    499
#ଅନ୍ୟଥା
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା testpcache testpcache;
ସଂରଚନା testpcache {
  sqlite3_int64 szPage;     /* Size of each page.  Multiple of 8. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szExtra;              /* Size of extra data that accompanies each page */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bPurgeable;           /* True if the page cache is purgeable */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFree;                /* Number of unused slots in a[] */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPinned;              /* Number of pinned slots in a[] */
  unsigned iRand;           /* State of the PRNG */
  unsigned iMagic;          /* Magic number for sanity checking */
  ସଂରଚନା testpcachePage {
    sqlite3_pcache_page page;  /* Base class */
    unsigned ଚାବି;              /* The key for this page. 0 means unallocated */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
ସ୍ଥିର unsigned testpcacheRandom(testpcache *p){
  unsigned x = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  ଫେରନ୍ତୁ x;
}


/*
** Allocate a new page cache instance.
*/
ସ୍ଥିର sqlite3_pcache *testpcacheCreate(
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szPage,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szExtra,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bPurgeable
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMem;
  ଅକ୍ଷର *x;
  testpcache *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  szPage = (szPage+7)&~7;
  szExtra = (szPage+7)&~7;
  nMem = ଆକାର(testpcache) + TESTPCACHE_NPAGE*(szPage+szExtra);
  p = sqlite3_malloc( nMem );
  ଯଦି( p==0 ) ଫେରନ୍ତୁ 0;
  x = (ଅକ୍ଷର*)&p[1];
  p->szPage = szPage;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prngSeed;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  ପାଇଁ(i=0; i<TESTPCACHE_NPAGE; i++, x += (szPage+szExtra)){
    p->a[i].ଚାବି = 0;
    p->a[i].isPinned = 0;
    p->a[i].page.pBuf = (ଶୂନ୍ୟ*)x;
    p->a[i].page.pExtra = (ଶୂନ୍ୟ*)&x[szPage];
  }
  testpcacheGlobal.nInstance++;
  ଫେରନ୍ତୁ (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
ସ୍ଥିର ଶୂନ୍ୟ testpcacheCachesize(sqlite3_pcache *pCache, ପୂର୍ଣ୍ଣସଂଖ୍ୟା newSize){
  testpcache *p = (testpcache*)pCache;
  ପ୍ରତିଜ୍ଞା( p->iMagic==TESTPCACHE_VALID );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ପ୍ରତିଜ୍ଞା( p->iMagic==TESTPCACHE_VALID );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance>0 );
  ଫେରନ୍ତୁ TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
ସ୍ଥିର sqlite3_pcache_page *testpcacheFetch(
  sqlite3_pcache *pCache,
  unsigned ଚାବି,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା createFlag
){
  testpcache *p = (testpcache*)pCache;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ପ୍ରତିଜ୍ଞା( p->iMagic==TESTPCACHE_VALID );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  ପାଇଁ(i=0; i<TESTPCACHE_NPAGE; i++){
    ଯଦି( p->a[i].ଚାବି==ଚାବି ){
      ଯଦି( !p->a[i].isPinned ){
        p->nPinned++;
        ପ୍ରତିଜ୍ଞା( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      ଫେରନ୍ତୁ &p->a[i].page;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  ଯଦି( createFlag==0 ){
    ଫେରନ୍ତୁ 0;
  }

  /* If no pages are available, always fail */
  ଯଦି( p->nPinned==TESTPCACHE_NPAGE ){
    ଫେରନ୍ତୁ 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  ଯଦି( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    ଫେରନ୍ତୁ 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  ଯଦି( testpcacheGlobal.ଉପର_Stress && createFlag<2 ){
    ଫେରନ୍ତୁ 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  ଯଦି( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    ପାଇଁ(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      ଯଦି( p->a[j].ଚାବି==0 ){
        p->a[j].ଚାବି = ଚାବି;
        p->a[j].isPinned = 1;
        memset(p->a[j].page.pBuf, 0, p->szPage);
        memset(p->a[j].page.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        ପ୍ରତିଜ୍ଞା( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        ଫେରନ୍ତୁ &p->a[j].page;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    ପ୍ରତିଜ୍ଞା( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  ଯଦି( p->bPurgeable==0 ){
    ଫେରନ୍ତୁ 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  ପାଇଁ(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    ଯଦି( p->a[j].ଚାବି>0 && p->a[j].isPinned==0 ){
      p->a[j].ଚାବି = ଚାବି;
      p->a[j].isPinned = 1;
      memset(p->a[j].page.pBuf, 0, p->szPage);
      memset(p->a[j].page.pExtra, 0, p->szExtra);
      p->nPinned++;
      ପ୍ରତିଜ୍ଞା( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      ଫେରନ୍ତୁ &p->a[j].page;
    }
  }

  /* The previous loop always finds a page to recycle. */
  ପ୍ରତିଜ୍ଞା(0);
  ଫେରନ୍ତୁ 0;
}

/*
** Unpin a page.
*/
ସ୍ଥିର ଶୂନ୍ୟ testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା discard
){
  testpcache *p = (testpcache*)pCache;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( p->iMagic==TESTPCACHE_VALID );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  ଯଦି( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  ପାଇଁ(i=0; i<TESTPCACHE_NPAGE; i++){
    ଯଦି( &p->a[i].page==pOldPage ){
      /* The pOldPage pointer always points to a pinned page */
      ପ୍ରତିଜ୍ଞା( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      ପ୍ରତିଜ୍ଞା( p->nPinned>=0 );
      ଯଦି( discard ){
        p->a[i].ଚାବି = 0;
        p->nFree++;
        ପ୍ରତିଜ୍ଞା( p->nFree<=TESTPCACHE_NPAGE );
      }
      ଫେରନ୍ତୁ;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  ପ୍ରତିଜ୍ଞା( 0 );
}


/*
** Rekey a single page.
*/
ସ୍ଥିର ଶୂନ୍ୟ testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  unsigned old_ଚାବି,
  unsigned new_ଚାବି
){
  testpcache *p = (testpcache*)pCache;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( p->iMagic==TESTPCACHE_VALID );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  ପାଇଁ(i=0; i<TESTPCACHE_NPAGE; i++){
    ଯଦି( p->a[i].ଚାବି==new_ଚାବି ){
      /* The new key is never a page that is already pinned */
      ପ୍ରତିଜ୍ଞା( p->a[i].isPinned==0 );
      p->a[i].ଚାବି = 0;
      p->nFree++;
      ପ୍ରତିଜ୍ଞା( p->nFree<=TESTPCACHE_NPAGE );
      ଭାଙ୍ଗ;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  ପାଇଁ(i=0; i<TESTPCACHE_NPAGE; i++){
    ଯଦି( p->a[i].ଚାବି==old_ଚାବି ){
      /* The oldKey and pOldPage parameters match */
      ପ୍ରତିଜ୍ଞା( &p->a[i].page==pOldPage );
      /* Page to be rekeyed must be pinned */
      ପ୍ରତିଜ୍ଞା( p->a[i].isPinned );
      p->a[i].ଚାବି = new_ଚାବି;
      ଫେରନ୍ତୁ;
    }
  }

  /* Rekey is always given a valid page to work with */
  ପ୍ରତିଜ୍ଞା( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
ସ୍ଥିର ଶୂନ୍ୟ testpcacheTruncate(sqlite3_pcache *pCache, unsigned iLimit){
  testpcache *p = (testpcache*)pCache;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( p->iMagic==TESTPCACHE_VALID );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance>0 );
  ପାଇଁ(i=0; i<TESTPCACHE_NPAGE; i++){
    ଯଦି( p->a[i].ଚାବି>=iLimit ){
      p->a[i].ଚାବି = 0;
      ଯଦି( p->a[i].isPinned ){
        p->nPinned--;
        ପ୍ରତିଜ୍ଞା( p->nPinned>=0 );
      }
      p->nFree++;
      ପ୍ରତିଜ୍ଞା( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
ସ୍ଥିର ଶୂନ୍ୟ testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ପ୍ରତିଜ୍ଞା( p->iMagic==TESTPCACHE_VALID );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy!=0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance>0 );
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
ଶୂନ୍ୟ installTestPCache(
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା installFlag,            /* True to install.  False to uninstall. */
  unsigned discardChance,     /* 0-100.  Chance to discard on unpin */
  unsigned prngSeed,          /* Seed for the PRNG */
  unsigned ଉପର_Stress         /* Call xStress aggressively */
){
  ସ୍ଥିର ଅଚଳ sqlite3_pcache_methods2 testPcache = {
    1,
    (ଶୂନ୍ୟ*)&testpcacheGlobal,
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
  ସ୍ଥିର sqlite3_pcache_methods2 defaultPcache;
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInstalled = 0;

  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.nInstance==0 );
  ପ୍ରତିଜ୍ଞା( testpcacheGlobal.pDummy==0 );
  ପ୍ରତିଜ୍ଞା( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prngSeed = prngSeed ^ (prngSeed<<16);
  testpcacheGlobal.ଉପର_Stress = ଉପର_Stress;
  ଯଦି( installFlag!=isInstalled ){
    ଯଦି( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      ପ୍ରତିଜ୍ଞା( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
