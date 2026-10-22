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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ testpcacheGlobalType testpcacheGlobalType;
ਬਣਤਰ testpcacheGlobalType {
  ਖਾਲੀ *pDummy;             /* Dummy allocation to simulate failures */
  ਪੂਰਨ nInstance;            /* Number of current instances */
  ਬਿਨਾਨਿਸ਼ਾਨੀ discardChance;   /* Chance of discarding on an unpin (0-100) */
  ਬਿਨਾਨਿਸ਼ਾਨੀ prng_ਬੀਜ_2;        /* Seed for the PRNG */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਉੱਚਾ_Stress;      /* Call xStress aggressively */
};
ਸਥਿਰ testpcacheGlobalType testpcacheGlobal;

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
ਸਥਿਰ ਪੂਰਨ testpcacheInit(ਖਾਲੀ *pArg){
  ਪ੍ਰਤਿਜ੍ਞਾ( pArg==(ਖਾਲੀ*)&testpcacheGlobal );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  ਵਾਪਸ testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
ਸਥਿਰ ਖਾਲੀ testpcacheShutdown(ਖਾਲੀ *pArg){
  ਪ੍ਰਤਿਜ੍ਞਾ( pArg==(ਖਾਲੀ*)&testpcacheGlobal );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance==0 );
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
#ਜੇ defined(SQLITE_ਆਰਜ਼ੀ_STORE) && SQLITE_ਆਰਜ਼ੀ_STORE>=2
# define TESTPCACHE_NPAGE    499
#ਨਹੀਂਤਾਂ
# define TESTPCACHE_NPAGE    217
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTPCACHE_RESERVE   17

/*
** Magic numbers used to determine validity of the page cache.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTPCACHE_VALID  0x364585fd
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTPCACHE_CLEAR  0xd42670d4

/*
** Private implementation of a page cache.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ testpcache testpcache;
ਬਣਤਰ testpcache {
  sqlite3_int64 sz_ਪੰਨਾ;     /* Size of each page.  Multiple of 8. */
  ਪੂਰਨ szExtra;              /* Size of extra data that accompanies each page */
  ਪੂਰਨ bPurgeable;           /* True if the page cache is purgeable */
  ਪੂਰਨ nFree;                /* Number of unused slots in a[] */
  ਪੂਰਨ nPinned;              /* Number of pinned slots in a[] */
  ਬਿਨਾਨਿਸ਼ਾਨੀ iRand;           /* State of the PRNG */
  ਬਿਨਾਨਿਸ਼ਾਨੀ iMagic;          /* Magic number for sanity checking */
  ਬਣਤਰ testpcache_ਪੰਨਾ {
    sqlite3_pcache_ਪੰਨਾ ਪੰਨਾ;  /* Base class */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੁੰਜੀ;              /* The key for this page. 0 means unallocated */
    ਪੂਰਨ isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ testpcacheRandom(testpcache *p){
  ਬਿਨਾਨਿਸ਼ਾਨੀ x = 0;
  ਪੂਰਨ i;
  ਲਈ(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  ਵਾਪਸ x;
}


/*
** Allocate a new page cache instance.
*/
ਸਥਿਰ sqlite3_pcache *testpcacheCreate(
  ਪੂਰਨ sz_ਪੰਨਾ,
  ਪੂਰਨ szExtra,
  ਪੂਰਨ bPurgeable
){
  ਪੂਰਨ nMem;
  ਅੱਖਰ *x;
  testpcache *p;
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  sz_ਪੰਨਾ = (sz_ਪੰਨਾ+7)&~7;
  szExtra = (sz_ਪੰਨਾ+7)&~7;
  nMem = ਆਕਾਰ(testpcache) + TESTPCACHE_NPAGE*(sz_ਪੰਨਾ+szExtra);
  p = sqlite3_malloc( nMem );
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  x = (ਅੱਖਰ*)&p[1];
  p->sz_ਪੰਨਾ = sz_ਪੰਨਾ;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prng_ਬੀਜ_2;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  ਲਈ(i=0; i<TESTPCACHE_NPAGE; i++, x += (sz_ਪੰਨਾ+szExtra)){
    p->a[i].ਕੁੰਜੀ = 0;
    p->a[i].isPinned = 0;
    p->a[i].ਪੰਨਾ.pBuf = (ਖਾਲੀ*)x;
    p->a[i].ਪੰਨਾ.pExtra = (ਖਾਲੀ*)&x[sz_ਪੰਨਾ];
  }
  testpcacheGlobal.nInstance++;
  ਵਾਪਸ (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
ਸਥਿਰ ਖਾਲੀ testpcacheCachesize(sqlite3_pcache *pCache, ਪੂਰਨ new_ਮਾਪ_2){
  testpcache *p = (testpcache*)pCache;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iMagic==TESTPCACHE_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
ਸਥਿਰ ਪੂਰਨ testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iMagic==TESTPCACHE_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance>0 );
  ਵਾਪਸ TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
ਸਥਿਰ sqlite3_pcache_ਪੰਨਾ *testpcacheFetch(
  sqlite3_pcache *pCache,
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਕੁੰਜੀ,
  ਪੂਰਨ createFlag
){
  testpcache *p = (testpcache*)pCache;
  ਪੂਰਨ i, j;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iMagic==TESTPCACHE_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  ਲਈ(i=0; i<TESTPCACHE_NPAGE; i++){
    ਜੇ( p->a[i].ਕੁੰਜੀ==ਕੁੰਜੀ ){
      ਜੇ( !p->a[i].isPinned ){
        p->nPinned++;
        ਪ੍ਰਤਿਜ੍ਞਾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      ਵਾਪਸ &p->a[i].ਪੰਨਾ;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  ਜੇ( createFlag==0 ){
    ਵਾਪਸ 0;
  }

  /* If no pages are available, always fail */
  ਜੇ( p->nPinned==TESTPCACHE_NPAGE ){
    ਵਾਪਸ 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  ਜੇ( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    ਵਾਪਸ 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  ਜੇ( testpcacheGlobal.ਉੱਚਾ_Stress && createFlag<2 ){
    ਵਾਪਸ 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  ਜੇ( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    ਲਈ(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      ਜੇ( p->a[j].ਕੁੰਜੀ==0 ){
        p->a[j].ਕੁੰਜੀ = ਕੁੰਜੀ;
        p->a[j].isPinned = 1;
        memset(p->a[j].ਪੰਨਾ.pBuf, 0, p->sz_ਪੰਨਾ);
        memset(p->a[j].ਪੰਨਾ.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        ਪ੍ਰਤਿਜ੍ਞਾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        ਵਾਪਸ &p->a[j].ਪੰਨਾ;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  ਜੇ( p->bPurgeable==0 ){
    ਵਾਪਸ 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  ਲਈ(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    ਜੇ( p->a[j].ਕੁੰਜੀ>0 && p->a[j].isPinned==0 ){
      p->a[j].ਕੁੰਜੀ = ਕੁੰਜੀ;
      p->a[j].isPinned = 1;
      memset(p->a[j].ਪੰਨਾ.pBuf, 0, p->sz_ਪੰਨਾ);
      memset(p->a[j].ਪੰਨਾ.pExtra, 0, p->szExtra);
      p->nPinned++;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      ਵਾਪਸ &p->a[j].ਪੰਨਾ;
    }
  }

  /* The previous loop always finds a page to recycle. */
  ਪ੍ਰਤਿਜ੍ਞਾ(0);
  ਵਾਪਸ 0;
}

/*
** Unpin a page.
*/
ਸਥਿਰ ਖਾਲੀ testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_ਪੰਨਾ *p_Old_ਪੰਨਾ,
  ਪੂਰਨ discard
){
  testpcache *p = (testpcache*)pCache;
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iMagic==TESTPCACHE_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  ਜੇ( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  ਲਈ(i=0; i<TESTPCACHE_NPAGE; i++){
    ਜੇ( &p->a[i].ਪੰਨਾ==p_Old_ਪੰਨਾ ){
      /* The pOldPage pointer always points to a pinned page */
      ਪ੍ਰਤਿਜ੍ਞਾ( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->nPinned>=0 );
      ਜੇ( discard ){
        p->a[i].ਕੁੰਜੀ = 0;
        p->nFree++;
        ਪ੍ਰਤਿਜ੍ਞਾ( p->nFree<=TESTPCACHE_NPAGE );
      }
      ਵਾਪਸ;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
}


/*
** Rekey a single page.
*/
ਸਥਿਰ ਖਾਲੀ testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_ਪੰਨਾ *p_Old_ਪੰਨਾ,
  ਬਿਨਾਨਿਸ਼ਾਨੀ old_ਕੁੰਜੀ,
  ਬਿਨਾਨਿਸ਼ਾਨੀ new_ਕੁੰਜੀ
){
  testpcache *p = (testpcache*)pCache;
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iMagic==TESTPCACHE_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  ਲਈ(i=0; i<TESTPCACHE_NPAGE; i++){
    ਜੇ( p->a[i].ਕੁੰਜੀ==new_ਕੁੰਜੀ ){
      /* The new key is never a page that is already pinned */
      ਪ੍ਰਤਿਜ੍ਞਾ( p->a[i].isPinned==0 );
      p->a[i].ਕੁੰਜੀ = 0;
      p->nFree++;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->nFree<=TESTPCACHE_NPAGE );
      ਰੋਕੋ;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  ਲਈ(i=0; i<TESTPCACHE_NPAGE; i++){
    ਜੇ( p->a[i].ਕੁੰਜੀ==old_ਕੁੰਜੀ ){
      /* The oldKey and pOldPage parameters match */
      ਪ੍ਰਤਿਜ੍ਞਾ( &p->a[i].ਪੰਨਾ==p_Old_ਪੰਨਾ );
      /* Page to be rekeyed must be pinned */
      ਪ੍ਰਤਿਜ੍ਞਾ( p->a[i].isPinned );
      p->a[i].ਕੁੰਜੀ = new_ਕੁੰਜੀ;
      ਵਾਪਸ;
    }
  }

  /* Rekey is always given a valid page to work with */
  ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
ਸਥਿਰ ਖਾਲੀ testpcacheTruncate(sqlite3_pcache *pCache, ਬਿਨਾਨਿਸ਼ਾਨੀ i_ਹੱਦਬੰਦੀ){
  testpcache *p = (testpcache*)pCache;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iMagic==TESTPCACHE_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance>0 );
  ਲਈ(i=0; i<TESTPCACHE_NPAGE; i++){
    ਜੇ( p->a[i].ਕੁੰਜੀ>=i_ਹੱਦਬੰਦੀ ){
      p->a[i].ਕੁੰਜੀ = 0;
      ਜੇ( p->a[i].isPinned ){
        p->nPinned--;
        ਪ੍ਰਤਿਜ੍ਞਾ( p->nPinned>=0 );
      }
      p->nFree++;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
ਸਥਿਰ ਖਾਲੀ testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->iMagic==TESTPCACHE_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance>0 );
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
ਖਾਲੀ installTestPCache(
  ਪੂਰਨ installFlag,            /* True to install.  False to uninstall. */
  ਬਿਨਾਨਿਸ਼ਾਨੀ discardChance,     /* 0-100.  Chance to discard on unpin */
  ਬਿਨਾਨਿਸ਼ਾਨੀ prng_ਬੀਜ_2,          /* Seed for the PRNG */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਉੱਚਾ_Stress         /* Call xStress aggressively */
){
  ਸਥਿਰ ਅਚਲ sqlite3_pcache_methods2 testPcache = {
    1,
    (ਖਾਲੀ*)&testpcacheGlobal,
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
  ਸਥਿਰ sqlite3_pcache_methods2 defaultPcache;
  ਸਥਿਰ ਪੂਰਨ isInstalled = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.nInstance==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( testpcacheGlobal.pDummy==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prng_ਬੀਜ_2 = prng_ਬੀਜ_2 ^ (prng_ਬੀਜ_2<<16);
  testpcacheGlobal.ਉੱਚਾ_Stress = ਉੱਚਾ_Stress;
  ਜੇ( installFlag!=isInstalled ){
    ਜੇ( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      ਪ੍ਰਤਿਜ੍ਞਾ( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
