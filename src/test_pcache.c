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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ testpcacheGlobalType testpcacheGlobalType;
ರಚನೆ testpcacheGlobalType {
  ಶೂನ್ಯ *pDummy;             /* Dummy allocation to simulate failures */
  ಪೂರ್ಣಾಂಕ nInstance;            /* Number of current instances */
  ಚಿಹ್ನೆರಹಿತ discardChance;   /* Chance of discarding on an unpin (0-100) */
  ಚಿಹ್ನೆರಹಿತ prng_ಬೀಜ_2;        /* Seed for the PRNG */
  ಚಿಹ್ನೆರಹಿತ ಮೇಲಿನ_Stress;      /* Call xStress aggressively */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ testpcacheInit(ಶೂನ್ಯ *pArg){
  ಪ್ರತಿಜ್ಞಾ( pArg==(ಶೂನ್ಯ*)&testpcacheGlobal );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy==0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  ಹಿಂತಿರುಗಿ testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
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
#ಆದರೆ defined(SQLITE_ತಾತ್ಕಾಲಿಕ_STORE) && SQLITE_ತಾತ್ಕಾಲಿಕ_STORE>=2
# define TESTPCACHE_NPAGE    499
#ಇಲ್ಲದಿದ್ದರೆ
# define TESTPCACHE_NPAGE    217
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು TESTPCACHE_RESERVE   17

/*
** Magic numbers used to determine validity of the page cache.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು TESTPCACHE_VALID  0x364585fd
#ವ್ಯಾಖ್ಯಾನಿಸು TESTPCACHE_CLEAR  0xd42670d4

/*
** Private implementation of a page cache.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ testpcache testpcache;
ರಚನೆ testpcache {
  sqlite3_int64 sz_ಪುಟ;     /* Size of each page.  Multiple of 8. */
  ಪೂರ್ಣಾಂಕ szExtra;              /* Size of extra data that accompanies each page */
  ಪೂರ್ಣಾಂಕ bPurgeable;           /* True if the page cache is purgeable */
  ಪೂರ್ಣಾಂಕ nFree;                /* Number of unused slots in a[] */
  ಪೂರ್ಣಾಂಕ nPinned;              /* Number of pinned slots in a[] */
  ಚಿಹ್ನೆರಹಿತ iRand;           /* State of the PRNG */
  ಚಿಹ್ನೆರಹಿತ iMagic;          /* Magic number for sanity checking */
  ರಚನೆ testpcache_ಪುಟ {
    sqlite3_pcache_ಪುಟ ಪುಟ;  /* Base class */
    ಚಿಹ್ನೆರಹಿತ ಕೀಲಿ;              /* The key for this page. 0 means unallocated */
    ಪೂರ್ಣಾಂಕ isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ testpcacheRandom(testpcache *p){
  ಚಿಹ್ನೆರಹಿತ x = 0;
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  ಹಿಂತಿರುಗಿ x;
}


/*
** Allocate a new page cache instance.
*/
ಸ್ಥಿರ sqlite3_pcache *testpcacheCreate(
  ಪೂರ್ಣಾಂಕ sz_ಪುಟ,
  ಪೂರ್ಣಾಂಕ szExtra,
  ಪೂರ್ಣಾಂಕ bPurgeable
){
  ಪೂರ್ಣಾಂಕ nMem;
  ಅಕ್ಷರ *x;
  testpcache *p;
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  sz_ಪುಟ = (sz_ಪುಟ+7)&~7;
  szExtra = (sz_ಪುಟ+7)&~7;
  nMem = ಗಾತ್ರ(testpcache) + TESTPCACHE_NPAGE*(sz_ಪುಟ+szExtra);
  p = sqlite3_malloc( nMem );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  x = (ಅಕ್ಷರ*)&p[1];
  p->sz_ಪುಟ = sz_ಪುಟ;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prng_ಬೀಜ_2;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  ಗಾಗಿ(i=0; i<TESTPCACHE_NPAGE; i++, x += (sz_ಪುಟ+szExtra)){
    p->a[i].ಕೀಲಿ = 0;
    p->a[i].isPinned = 0;
    p->a[i].ಪುಟ.pBuf = (ಶೂನ್ಯ*)x;
    p->a[i].ಪುಟ.pExtra = (ಶೂನ್ಯ*)&x[sz_ಪುಟ];
  }
  testpcacheGlobal.nInstance++;
  ಹಿಂತಿರುಗಿ (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheCachesize(sqlite3_pcache *pCache, ಪೂರ್ಣಾಂಕ new_ಗಾತ್ರ_2){
  testpcache *p = (testpcache*)pCache;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );
  ಹಿಂತಿರುಗಿ TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
ಸ್ಥಿರ sqlite3_pcache_ಪುಟ *testpcacheFetch(
  sqlite3_pcache *pCache,
  ಚಿಹ್ನೆರಹಿತ ಕೀಲಿ,
  ಪೂರ್ಣಾಂಕ createFlag
){
  testpcache *p = (testpcache*)pCache;
  ಪೂರ್ಣಾಂಕ i, j;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  ಗಾಗಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಆದರೆ( p->a[i].ಕೀಲಿ==ಕೀಲಿ ){
      ಆದರೆ( !p->a[i].isPinned ){
        p->nPinned++;
        ಪ್ರತಿಜ್ಞಾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      ಹಿಂತಿರುಗಿ &p->a[i].ಪುಟ;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  ಆದರೆ( createFlag==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }

  /* If no pages are available, always fail */
  ಆದರೆ( p->nPinned==TESTPCACHE_NPAGE ){
    ಹಿಂತಿರುಗಿ 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  ಆದರೆ( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    ಹಿಂತಿರುಗಿ 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  ಆದರೆ( testpcacheGlobal.ಮೇಲಿನ_Stress && createFlag<2 ){
    ಹಿಂತಿರುಗಿ 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  ಆದರೆ( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    ಗಾಗಿ(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      ಆದರೆ( p->a[j].ಕೀಲಿ==0 ){
        p->a[j].ಕೀಲಿ = ಕೀಲಿ;
        p->a[j].isPinned = 1;
        memset(p->a[j].ಪುಟ.pBuf, 0, p->sz_ಪುಟ);
        memset(p->a[j].ಪುಟ.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        ಪ್ರತಿಜ್ಞಾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        ಹಿಂತಿರುಗಿ &p->a[j].ಪುಟ;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    ಪ್ರತಿಜ್ಞಾ( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  ಆದರೆ( p->bPurgeable==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  ಗಾಗಿ(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    ಆದರೆ( p->a[j].ಕೀಲಿ>0 && p->a[j].isPinned==0 ){
      p->a[j].ಕೀಲಿ = ಕೀಲಿ;
      p->a[j].isPinned = 1;
      memset(p->a[j].ಪುಟ.pBuf, 0, p->sz_ಪುಟ);
      memset(p->a[j].ಪುಟ.pExtra, 0, p->szExtra);
      p->nPinned++;
      ಪ್ರತಿಜ್ಞಾ( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      ಹಿಂತಿರುಗಿ &p->a[j].ಪುಟ;
    }
  }

  /* The previous loop always finds a page to recycle. */
  ಪ್ರತಿಜ್ಞಾ(0);
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Unpin a page.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_ಪುಟ *p_Old_ಪುಟ,
  ಪೂರ್ಣಾಂಕ discard
){
  testpcache *p = (testpcache*)pCache;
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  ಆದರೆ( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  ಗಾಗಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಆದರೆ( &p->a[i].ಪುಟ==p_Old_ಪುಟ ){
      /* The pOldPage pointer always points to a pinned page */
      ಪ್ರತಿಜ್ಞಾ( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      ಪ್ರತಿಜ್ಞಾ( p->nPinned>=0 );
      ಆದರೆ( discard ){
        p->a[i].ಕೀಲಿ = 0;
        p->nFree++;
        ಪ್ರತಿಜ್ಞಾ( p->nFree<=TESTPCACHE_NPAGE );
      }
      ಹಿಂತಿರುಗಿ;
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
  sqlite3_pcache_ಪುಟ *p_Old_ಪುಟ,
  ಚಿಹ್ನೆರಹಿತ old_ಕೀಲಿ,
  ಚಿಹ್ನೆರಹಿತ new_ಕೀಲಿ
){
  testpcache *p = (testpcache*)pCache;
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  ಗಾಗಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಆದರೆ( p->a[i].ಕೀಲಿ==new_ಕೀಲಿ ){
      /* The new key is never a page that is already pinned */
      ಪ್ರತಿಜ್ಞಾ( p->a[i].isPinned==0 );
      p->a[i].ಕೀಲಿ = 0;
      p->nFree++;
      ಪ್ರತಿಜ್ಞಾ( p->nFree<=TESTPCACHE_NPAGE );
      ನಿಲ್ಲಿಸು;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  ಗಾಗಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಆದರೆ( p->a[i].ಕೀಲಿ==old_ಕೀಲಿ ){
      /* The oldKey and pOldPage parameters match */
      ಪ್ರತಿಜ್ಞಾ( &p->a[i].ಪುಟ==p_Old_ಪುಟ );
      /* Page to be rekeyed must be pinned */
      ಪ್ರತಿಜ್ಞಾ( p->a[i].isPinned );
      p->a[i].ಕೀಲಿ = new_ಕೀಲಿ;
      ಹಿಂತಿರುಗಿ;
    }
  }

  /* Rekey is always given a valid page to work with */
  ಪ್ರತಿಜ್ಞಾ( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testpcacheTruncate(sqlite3_pcache *pCache, ಚಿಹ್ನೆರಹಿತ i_ಎಲ್ಲೆ){
  testpcache *p = (testpcache*)pCache;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( p->iMagic==TESTPCACHE_VALID );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy!=0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance>0 );
  ಗಾಗಿ(i=0; i<TESTPCACHE_NPAGE; i++){
    ಆದರೆ( p->a[i].ಕೀಲಿ>=i_ಎಲ್ಲೆ ){
      p->a[i].ಕೀಲಿ = 0;
      ಆದರೆ( p->a[i].isPinned ){
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
  ಪೂರ್ಣಾಂಕ installFlag,            /* True to install.  False to uninstall. */
  ಚಿಹ್ನೆರಹಿತ discardChance,     /* 0-100.  Chance to discard on unpin */
  ಚಿಹ್ನೆರಹಿತ prng_ಬೀಜ_2,          /* Seed for the PRNG */
  ಚಿಹ್ನೆರಹಿತ ಮೇಲಿನ_Stress         /* Call xStress aggressively */
){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_pcache_methods2 testPcache = {
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
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isInstalled = 0;

  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.nInstance==0 );
  ಪ್ರತಿಜ್ಞಾ( testpcacheGlobal.pDummy==0 );
  ಪ್ರತಿಜ್ಞಾ( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prng_ಬೀಜ_2 = prng_ಬೀಜ_2 ^ (prng_ಬೀಜ_2<<16);
  testpcacheGlobal.ಮೇಲಿನ_Stress = ಮೇಲಿನ_Stress;
  ಆದರೆ( installFlag!=isInstalled ){
    ಆದರೆ( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      ಪ್ರತಿಜ್ಞಾ( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
