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
#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
வகைவரை கட்டமைப்பு testpcacheGlobalType testpcacheGlobalType;
கட்டமைப்பு testpcacheGlobalType {
  வெறுமை *pDummy;             /* Dummy allocation to simulate failures */
  முழு nInstance;            /* Number of current instances */
  unsigned discardChance;   /* Chance of discarding on an unpin (0-100) */
  unsigned prngSeed;        /* Seed for the PRNG */
  unsigned உயர்வு_Stress;      /* Call xStress aggressively */
};
நிலையான testpcacheGlobalType testpcacheGlobal;

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
நிலையான முழு testpcacheInit(வெறுமை *pArg){
  ப்ரதிஜ்ஞா( pArg==(வெறுமை*)&testpcacheGlobal );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy==0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  திரும்பு testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
நிலையான வெறுமை testpcacheShutdown(வெறுமை *pArg){
  ப்ரதிஜ்ஞா( pArg==(வெறுமை*)&testpcacheGlobal );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance==0 );
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
#எனில் defined(SQLITE_TEMP_STORE) && SQLITE_TEMP_STORE>=2
# define TESTPCACHE_NPAGE    499
#இல்லையெனில்
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
வகைவரை கட்டமைப்பு testpcache testpcache;
கட்டமைப்பு testpcache {
  sqlite3_int64 szPage;     /* Size of each page.  Multiple of 8. */
  முழு szExtra;              /* Size of extra data that accompanies each page */
  முழு bPurgeable;           /* True if the page cache is purgeable */
  முழு nFree;                /* Number of unused slots in a[] */
  முழு nPinned;              /* Number of pinned slots in a[] */
  unsigned iRand;           /* State of the PRNG */
  unsigned iMagic;          /* Magic number for sanity checking */
  கட்டமைப்பு testpcachePage {
    sqlite3_pcache_page page;  /* Base class */
    unsigned சாவி;              /* The key for this page. 0 means unallocated */
    முழு isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
நிலையான unsigned testpcacheRandom(testpcache *p){
  unsigned x = 0;
  முழு i;
  க்கு(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  திரும்பு x;
}


/*
** Allocate a new page cache instance.
*/
நிலையான sqlite3_pcache *testpcacheCreate(
  முழு szPage,
  முழு szExtra,
  முழு bPurgeable
){
  முழு nMem;
  எழுத்து *x;
  testpcache *p;
  முழு i;
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  szPage = (szPage+7)&~7;
  szExtra = (szPage+7)&~7;
  nMem = அளவு(testpcache) + TESTPCACHE_NPAGE*(szPage+szExtra);
  p = sqlite3_malloc( nMem );
  எனில்( p==0 ) திரும்பு 0;
  x = (எழுத்து*)&p[1];
  p->szPage = szPage;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prngSeed;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  க்கு(i=0; i<TESTPCACHE_NPAGE; i++, x += (szPage+szExtra)){
    p->a[i].சாவி = 0;
    p->a[i].isPinned = 0;
    p->a[i].page.pBuf = (வெறுமை*)x;
    p->a[i].page.pExtra = (வெறுமை*)&x[szPage];
  }
  testpcacheGlobal.nInstance++;
  திரும்பு (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
நிலையான வெறுமை testpcacheCachesize(sqlite3_pcache *pCache, முழு newSize){
  testpcache *p = (testpcache*)pCache;
  ப்ரதிஜ்ஞா( p->iMagic==TESTPCACHE_VALID );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
நிலையான முழு testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ப்ரதிஜ்ஞா( p->iMagic==TESTPCACHE_VALID );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance>0 );
  திரும்பு TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
நிலையான sqlite3_pcache_page *testpcacheFetch(
  sqlite3_pcache *pCache,
  unsigned சாவி,
  முழு createFlag
){
  testpcache *p = (testpcache*)pCache;
  முழு i, j;
  ப்ரதிஜ்ஞா( p->iMagic==TESTPCACHE_VALID );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  க்கு(i=0; i<TESTPCACHE_NPAGE; i++){
    எனில்( p->a[i].சாவி==சாவி ){
      எனில்( !p->a[i].isPinned ){
        p->nPinned++;
        ப்ரதிஜ்ஞா( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      திரும்பு &p->a[i].page;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  எனில்( createFlag==0 ){
    திரும்பு 0;
  }

  /* If no pages are available, always fail */
  எனில்( p->nPinned==TESTPCACHE_NPAGE ){
    திரும்பு 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  எனில்( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    திரும்பு 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  எனில்( testpcacheGlobal.உயர்வு_Stress && createFlag<2 ){
    திரும்பு 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  எனில்( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    க்கு(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      எனில்( p->a[j].சாவி==0 ){
        p->a[j].சாவி = சாவி;
        p->a[j].isPinned = 1;
        memset(p->a[j].page.pBuf, 0, p->szPage);
        memset(p->a[j].page.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        ப்ரதிஜ்ஞா( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        திரும்பு &p->a[j].page;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    ப்ரதிஜ்ஞா( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  எனில்( p->bPurgeable==0 ){
    திரும்பு 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  க்கு(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    எனில்( p->a[j].சாவி>0 && p->a[j].isPinned==0 ){
      p->a[j].சாவி = சாவி;
      p->a[j].isPinned = 1;
      memset(p->a[j].page.pBuf, 0, p->szPage);
      memset(p->a[j].page.pExtra, 0, p->szExtra);
      p->nPinned++;
      ப்ரதிஜ்ஞா( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      திரும்பு &p->a[j].page;
    }
  }

  /* The previous loop always finds a page to recycle. */
  ப்ரதிஜ்ஞா(0);
  திரும்பு 0;
}

/*
** Unpin a page.
*/
நிலையான வெறுமை testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  முழு discard
){
  testpcache *p = (testpcache*)pCache;
  முழு i;
  ப்ரதிஜ்ஞா( p->iMagic==TESTPCACHE_VALID );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  எனில்( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  க்கு(i=0; i<TESTPCACHE_NPAGE; i++){
    எனில்( &p->a[i].page==pOldPage ){
      /* The pOldPage pointer always points to a pinned page */
      ப்ரதிஜ்ஞா( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      ப்ரதிஜ்ஞா( p->nPinned>=0 );
      எனில்( discard ){
        p->a[i].சாவி = 0;
        p->nFree++;
        ப்ரதிஜ்ஞா( p->nFree<=TESTPCACHE_NPAGE );
      }
      திரும்பு;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  ப்ரதிஜ்ஞா( 0 );
}


/*
** Rekey a single page.
*/
நிலையான வெறுமை testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  unsigned old_சாவி,
  unsigned new_சாவி
){
  testpcache *p = (testpcache*)pCache;
  முழு i;
  ப்ரதிஜ்ஞா( p->iMagic==TESTPCACHE_VALID );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  க்கு(i=0; i<TESTPCACHE_NPAGE; i++){
    எனில்( p->a[i].சாவி==new_சாவி ){
      /* The new key is never a page that is already pinned */
      ப்ரதிஜ்ஞா( p->a[i].isPinned==0 );
      p->a[i].சாவி = 0;
      p->nFree++;
      ப்ரதிஜ்ஞா( p->nFree<=TESTPCACHE_NPAGE );
      நிறுத்து;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  க்கு(i=0; i<TESTPCACHE_NPAGE; i++){
    எனில்( p->a[i].சாவி==old_சாவி ){
      /* The oldKey and pOldPage parameters match */
      ப்ரதிஜ்ஞா( &p->a[i].page==pOldPage );
      /* Page to be rekeyed must be pinned */
      ப்ரதிஜ்ஞா( p->a[i].isPinned );
      p->a[i].சாவி = new_சாவி;
      திரும்பு;
    }
  }

  /* Rekey is always given a valid page to work with */
  ப்ரதிஜ்ஞா( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
நிலையான வெறுமை testpcacheTruncate(sqlite3_pcache *pCache, unsigned iLimit){
  testpcache *p = (testpcache*)pCache;
  unsigned முழு i;
  ப்ரதிஜ்ஞா( p->iMagic==TESTPCACHE_VALID );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance>0 );
  க்கு(i=0; i<TESTPCACHE_NPAGE; i++){
    எனில்( p->a[i].சாவி>=iLimit ){
      p->a[i].சாவி = 0;
      எனில்( p->a[i].isPinned ){
        p->nPinned--;
        ப்ரதிஜ்ஞா( p->nPinned>=0 );
      }
      p->nFree++;
      ப்ரதிஜ்ஞா( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
நிலையான வெறுமை testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ப்ரதிஜ்ஞா( p->iMagic==TESTPCACHE_VALID );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy!=0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance>0 );
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
வெறுமை installTestPCache(
  முழு installFlag,            /* True to install.  False to uninstall. */
  unsigned discardChance,     /* 0-100.  Chance to discard on unpin */
  unsigned prngSeed,          /* Seed for the PRNG */
  unsigned உயர்வு_Stress         /* Call xStress aggressively */
){
  நிலையான மாறிலி sqlite3_pcache_methods2 testPcache = {
    1,
    (வெறுமை*)&testpcacheGlobal,
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
  நிலையான sqlite3_pcache_methods2 defaultPcache;
  நிலையான முழு isInstalled = 0;

  ப்ரதிஜ்ஞா( testpcacheGlobal.nInstance==0 );
  ப்ரதிஜ்ஞா( testpcacheGlobal.pDummy==0 );
  ப்ரதிஜ்ஞா( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prngSeed = prngSeed ^ (prngSeed<<16);
  testpcacheGlobal.உயர்வு_Stress = உயர்வு_Stress;
  எனில்( installFlag!=isInstalled ){
    எனில்( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      ப்ரதிஜ்ஞா( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
