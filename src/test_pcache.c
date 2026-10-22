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
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ testpcacheGlobalType testpcacheGlobalType;
ಸಂರಚನಾ testpcacheGlobalType {
  ಶೂನ್ಯ *pDummy;             /* Dummy allocation to simulate failures */
  ಪೂರ್ಣಾಙ್ಕ nInstance;            /* Number of current instances */
  unsigned discardChance;   /* Chance of discarding on an unpin (0-100) */
  unsigned prngSeed;        /* Seed for the PRNG */
  unsigned ಮೇಲಿನ_Stress;      /* Call xStress aggressively */
};
ಸ್ಥಿರ testpcacheGlobalType testpcacheGlobal;

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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ testpcacheInit(ಶೂನ್ಯ *pArg){
  ಪ್ರತಿಜ್ಞಾ( pArg==(ಶೂನ್ಯ*)&testpcacheGlobal );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy==0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  ಪ್ರತ್ಯಾಗಮ testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheShutdown(ಶೂನ್ಯ *pArg){
  ಪ್ರತಿಜ್ಞಾ( pArg==(ಶೂನ್ಯ*)&testpcacheGlobal );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance==0 );
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
#ಯದಿ defined(SQLITE_TEMP_STORE) && SQLITE_TEMP_STORE>=2
# define TESTPCACHE_NPAGE    499
#ಅನ್ಯಥಾ
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ testpcache testpcache;
ಸಂರಚನಾ testpcache {
  sqlite3_int64 szPage;     /* Size of each page.  Multiple of 8. */
  ಪೂರ್ಣಾಙ್ಕ szExtra;              /* Size of extra data that accompanies each page */
  ಪೂರ್ಣಾಙ್ಕ bPurgeable;           /* True if the page cache is purgeable */
  ಪೂರ್ಣಾಙ್ಕ nFree;                /* Number of unused slots in a[] */
  ಪೂರ್ಣಾಙ್ಕ nPinned;              /* Number of pinned slots in a[] */
  unsigned iRand;           /* State of the PRNG */
  unsigned iMagic;          /* Magic number for sanity checking */
  ಸಂರಚನಾ testpcachePage {
    sqlite3_pcache_page page;  /* Base class */
    unsigned ಕೀಲಿ;              /* The key for this page. 0 means unallocated */
    ಪೂರ್ಣಾಙ್ಕ isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
ಸ್ಥಿರ unsigned testpcacheRandom(testpcache *p){
  unsigned x = 0;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  ಪ್ರತ್ಯಾಗಮ x;
}


/*
** Allocate a new page cache instance.
*/
ಸ್ಥಿರ sqlite3_pcache *testpcacheCreate(
  ಪೂರ್ಣಾಙ್ಕ szPage,
  ಪೂರ್ಣಾಙ್ಕ szExtra,
  ಪೂರ್ಣಾಙ್ಕ bPurgeable
){
  ಪೂರ್ಣಾಙ್ಕ nMem;
  ವರ್ಣ *x;
  testpcache *p;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  szPage = (szPage+7)&~7;
  szExtra = (szPage+7)&~7;
  nMem = ಪರಿಮಾಣ(testpcache) + TESTPCACHE_NPAGE*(szPage+szExtra);
  p = sqlite3_malloc( nMem );
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  x = (ವರ್ಣ*)&p[1];
  p->szPage = szPage;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prngSeed;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  ಪ್ರತಿ(i=0; i<TESTPCACHE_NPAGE; i++, x += (szPage+szExtra)){
    p->a[i].ಕೀಲಿ = 0;
    p->a[i].isPinned = 0;
    p->a[i].page.pBuf = (ಶೂನ್ಯ*)x;
    p->a[i].page.pExtra = (ಶೂನ್ಯ*)&x[szPage];
  }
  testpcacheGlobal.nInstance++;
  ಪ್ರತ್ಯಾಗಮ (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheCachesize(sqlite3_pcache *pCache, ಪೂರ್ಣಾಙ್ಕ newSize){
  testpcache *p = (testpcache*)pCache;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );
  ಪ್ರತ್ಯಾಗಮ TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
ಸ್ಥಿರ sqlite3_pcache_page *testpcacheFetch(
  sqlite3_pcache *pCache,
  unsigned ಕೀಲಿ,
  ಪೂರ್ಣಾಙ್ಕ createFlag
){
  testpcache *p = (testpcache*)pCache;
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  ಪ್ರತಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಯದಿ( p->a[i].ಕೀಲಿ==ಕೀಲಿ ){
      ಯದಿ( !p->a[i].isPinned ){
        p->nPinned++;
        ಪ್ರತಿಜ್ಞಾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      ಪ್ರತ್ಯಾಗಮ &p->a[i].page;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  ಯದಿ( createFlag==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  /* If no pages are available, always fail */
  ಯದಿ( p->nPinned==TESTPCACHE_NPAGE ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  ಯದಿ( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  ಯದಿ( testpcacheGlobal.ಮೇಲಿನ_Stress && createFlag<2 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  ಯದಿ( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    ಪ್ರತಿ(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      ಯದಿ( p->a[j].ಕೀಲಿ==0 ){
        p->a[j].ಕೀಲಿ = ಕೀಲಿ;
        p->a[j].isPinned = 1;
        memset(p->a[j].page.pBuf, 0, p->szPage);
        memset(p->a[j].page.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        ಪ್ರತಿಜ್ಞಾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        ಪ್ರತ್ಯಾಗಮ &p->a[j].page;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    ಪ್ರತಿಜ್ಞಾ( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  ಯದಿ( p->bPurgeable==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  ಪ್ರತಿ(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    ಯದಿ( p->a[j].ಕೀಲಿ>0 && p->a[j].isPinned==0 ){
      p->a[j].ಕೀಲಿ = ಕೀಲಿ;
      p->a[j].isPinned = 1;
      memset(p->a[j].page.pBuf, 0, p->szPage);
      memset(p->a[j].page.pExtra, 0, p->szExtra);
      p->nPinned++;
      ಪ್ರತಿಜ್ಞಾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      ಪ್ರತ್ಯಾಗಮ &p->a[j].page;
    }
  }

  /* The previous loop always finds a page to recycle. */
  ಪ್ರತಿಜ್ಞಾ(0);
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Unpin a page.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  ಪೂರ್ಣಾಙ್ಕ discard
){
  testpcache *p = (testpcache*)pCache;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  ಯದಿ( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  ಪ್ರತಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಯದಿ( &p->a[i].page==pOldPage ){
      /* The pOldPage pointer always points to a pinned page */
      ಪ್ರತಿಜ್ಞಾ( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      ಪ್ರತಿಜ್ಞಾ( p->nPinned>=0 );
      ಯದಿ( discard ){
        p->a[i].ಕೀಲಿ = 0;
        p->nFree++;
        ಪ್ರತಿಜ್ಞಾ( p->nFree<=TESTPCACHE_NPAGE );
      }
      ಪ್ರತ್ಯಾಗಮ;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  ಪ್ರತಿಜ್ಞಾ( 0 );
}


/*
** Rekey a single page.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  unsigned old_ಕೀಲಿ,
  unsigned new_ಕೀಲಿ
){
  testpcache *p = (testpcache*)pCache;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  ಪ್ರತಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಯದಿ( p->a[i].ಕೀಲಿ==new_ಕೀಲಿ ){
      /* The new key is never a page that is already pinned */
      ಪ್ರತಿಜ್ಞಾ( p->a[i].isPinned==0 );
      p->a[i].ಕೀಲಿ = 0;
      p->nFree++;
      ಪ್ರತಿಜ್ಞಾ( p->nFree<=TESTPCACHE_NPAGE );
      ಭಙ್ಗ;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  ಪ್ರತಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಯದಿ( p->a[i].ಕೀಲಿ==old_ಕೀಲಿ ){
      /* The oldKey and pOldPage parameters match */
      ಪ್ರತಿಜ್ಞಾ( &p->a[i].page==pOldPage );
      /* Page to be rekeyed must be pinned */
      ಪ್ರತಿಜ್ಞಾ( p->a[i].isPinned );
      p->a[i].ಕೀಲಿ = new_ಕೀಲಿ;
      ಪ್ರತ್ಯಾಗಮ;
    }
  }

  /* Rekey is always given a valid page to work with */
  ಪ್ರತಿಜ್ಞಾ( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheTruncate(sqlite3_pcache *pCache, unsigned iLimit){
  testpcache *p = (testpcache*)pCache;
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );
  ಪ್ರತಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಯದಿ( p->a[i].ಕೀಲಿ>=iLimit ){
      p->a[i].ಕೀಲಿ = 0;
      ಯದಿ( p->a[i].isPinned ){
        p->nPinned--;
        ಪ್ರತಿಜ್ಞಾ( p->nPinned>=0 );
      }
      p->nFree++;
      ಪ್ರತಿಜ್ಞಾ( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );
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
ಶೂನ್ಯ installTestPCache(
  ಪೂರ್ಣಾಙ್ಕ installFlag,            /* True to install.  False to uninstall. */
  unsigned discardChance,     /* 0-100.  Chance to discard on unpin */
  unsigned prngSeed,          /* Seed for the PRNG */
  unsigned ಮೇಲಿನ_Stress         /* Call xStress aggressively */
){
  ಸ್ಥಿರ ಧ್ರುವ sqlite3_pcache_methods2 testPcache = {
    1,
    (ಶೂನ್ಯ*)&testpcacheGlobal,
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
  ಸ್ಥಿರ sqlite3_pcache_methods2 defaultPcache;
  ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isInstalled = 0;

  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance==0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy==0 );
  ಪ್ರತಿಜ್ಞಾ( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prngSeed = prngSeed ^ (prngSeed<<16);
  testpcacheGlobal.ಮೇಲಿನ_Stress = ಮೇಲಿನ_Stress;
  ಯದಿ( installFlag!=isInstalled ){
    ಯದಿ( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      ಪ್ರತಿಜ್ಞಾ( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
