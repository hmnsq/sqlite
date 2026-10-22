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
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
రకనిర్వచనం నిర్మాణం testpcacheGlobalType testpcacheGlobalType;
నిర్మాణం testpcacheGlobalType {
  శూన్య *pDummy;             /* Dummy allocation to simulate failures */
  పూర్ణాంకం nInstance;            /* Number of current instances */
  గుర్తులేని discardChance;   /* Chance of discarding on an unpin (0-100) */
  గుర్తులేని prng_విత్తనం_2;        /* Seed for the PRNG */
  గుర్తులేని ఎగువ_Stress;      /* Call xStress aggressively */
};
స్థిర testpcacheGlobalType testpcacheGlobal;

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
స్థిర పూర్ణాంకం testpcacheInit(శూన్య *pArg){
  ప్రతిజ్ఞా( pArg==(శూన్య*)&testpcacheGlobal );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy==0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  తిరిగి testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
స్థిర శూన్య testpcacheShutdown(శూన్య *pArg){
  ప్రతిజ్ఞా( pArg==(శూన్య*)&testpcacheGlobal );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance==0 );
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
#ఒకవేళ defined(SQLITE_తాత్కాలిక_STORE) && SQLITE_తాత్కాలిక_STORE>=2
# define TESTPCACHE_NPAGE    499
#లేకపోతే
# define TESTPCACHE_NPAGE    217
#అయితే_ముగింపు
#నిర్వచించు TESTPCACHE_RESERVE   17

/*
** Magic numbers used to determine validity of the page cache.
*/
#నిర్వచించు TESTPCACHE_VALID  0x364585fd
#నిర్వచించు TESTPCACHE_CLEAR  0xd42670d4

/*
** Private implementation of a page cache.
*/
రకనిర్వచనం నిర్మాణం testpcache testpcache;
నిర్మాణం testpcache {
  sqlite3_int64 sz_పుట;     /* Size of each page.  Multiple of 8. */
  పూర్ణాంకం szExtra;              /* Size of extra data that accompanies each page */
  పూర్ణాంకం bPurgeable;           /* True if the page cache is purgeable */
  పూర్ణాంకం nFree;                /* Number of unused slots in a[] */
  పూర్ణాంకం nPinned;              /* Number of pinned slots in a[] */
  గుర్తులేని iRand;           /* State of the PRNG */
  గుర్తులేని iMagic;          /* Magic number for sanity checking */
  నిర్మాణం testpcache_పుట {
    sqlite3_pcache_పుట పుట;  /* Base class */
    గుర్తులేని కీలకం;              /* The key for this page. 0 means unallocated */
    పూర్ణాంకం isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
స్థిర గుర్తులేని testpcacheRandom(testpcache *p){
  గుర్తులేని x = 0;
  పూర్ణాంకం i;
  కొరకు(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  తిరిగి x;
}


/*
** Allocate a new page cache instance.
*/
స్థిర sqlite3_pcache *testpcacheCreate(
  పూర్ణాంకం sz_పుట,
  పూర్ణాంకం szExtra,
  పూర్ణాంకం bPurgeable
){
  పూర్ణాంకం nMem;
  అక్షరం *x;
  testpcache *p;
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  sz_పుట = (sz_పుట+7)&~7;
  szExtra = (sz_పుట+7)&~7;
  nMem = పరిమాణం(testpcache) + TESTPCACHE_NPAGE*(sz_పుట+szExtra);
  p = sqlite3_malloc( nMem );
  ఒకవేళ( p==0 ) తిరిగి 0;
  x = (అక్షరం*)&p[1];
  p->sz_పుట = sz_పుట;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prng_విత్తనం_2;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  కొరకు(i=0; i<TESTPCACHE_NPAGE; i++, x += (sz_పుట+szExtra)){
    p->a[i].కీలకం = 0;
    p->a[i].isPinned = 0;
    p->a[i].పుట.pBuf = (శూన్య*)x;
    p->a[i].పుట.pExtra = (శూన్య*)&x[sz_పుట];
  }
  testpcacheGlobal.nInstance++;
  తిరిగి (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
స్థిర శూన్య testpcacheCachesize(sqlite3_pcache *pCache, పూర్ణాంకం new_కొలత_2){
  testpcache *p = (testpcache*)pCache;
  ప్రతిజ్ఞా( p->iMagic==TESTPCACHE_VALID );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
స్థిర పూర్ణాంకం testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ప్రతిజ్ఞా( p->iMagic==TESTPCACHE_VALID );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance>0 );
  తిరిగి TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
స్థిర sqlite3_pcache_పుట *testpcacheFetch(
  sqlite3_pcache *pCache,
  గుర్తులేని కీలకం,
  పూర్ణాంకం createFlag
){
  testpcache *p = (testpcache*)pCache;
  పూర్ణాంకం i, j;
  ప్రతిజ్ఞా( p->iMagic==TESTPCACHE_VALID );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  కొరకు(i=0; i<TESTPCACHE_NPAGE; i++){
    ఒకవేళ( p->a[i].కీలకం==కీలకం ){
      ఒకవేళ( !p->a[i].isPinned ){
        p->nPinned++;
        ప్రతిజ్ఞా( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      తిరిగి &p->a[i].పుట;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  ఒకవేళ( createFlag==0 ){
    తిరిగి 0;
  }

  /* If no pages are available, always fail */
  ఒకవేళ( p->nPinned==TESTPCACHE_NPAGE ){
    తిరిగి 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  ఒకవేళ( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    తిరిగి 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  ఒకవేళ( testpcacheGlobal.ఎగువ_Stress && createFlag<2 ){
    తిరిగి 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  ఒకవేళ( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    కొరకు(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      ఒకవేళ( p->a[j].కీలకం==0 ){
        p->a[j].కీలకం = కీలకం;
        p->a[j].isPinned = 1;
        memset(p->a[j].పుట.pBuf, 0, p->sz_పుట);
        memset(p->a[j].పుట.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        ప్రతిజ్ఞా( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        తిరిగి &p->a[j].పుట;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    ప్రతిజ్ఞా( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  ఒకవేళ( p->bPurgeable==0 ){
    తిరిగి 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  కొరకు(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    ఒకవేళ( p->a[j].కీలకం>0 && p->a[j].isPinned==0 ){
      p->a[j].కీలకం = కీలకం;
      p->a[j].isPinned = 1;
      memset(p->a[j].పుట.pBuf, 0, p->sz_పుట);
      memset(p->a[j].పుట.pExtra, 0, p->szExtra);
      p->nPinned++;
      ప్రతిజ్ఞా( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      తిరిగి &p->a[j].పుట;
    }
  }

  /* The previous loop always finds a page to recycle. */
  ప్రతిజ్ఞా(0);
  తిరిగి 0;
}

/*
** Unpin a page.
*/
స్థిర శూన్య testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_పుట *p_Old_పుట,
  పూర్ణాంకం discard
){
  testpcache *p = (testpcache*)pCache;
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( p->iMagic==TESTPCACHE_VALID );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  ఒకవేళ( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  కొరకు(i=0; i<TESTPCACHE_NPAGE; i++){
    ఒకవేళ( &p->a[i].పుట==p_Old_పుట ){
      /* The pOldPage pointer always points to a pinned page */
      ప్రతిజ్ఞా( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      ప్రతిజ్ఞా( p->nPinned>=0 );
      ఒకవేళ( discard ){
        p->a[i].కీలకం = 0;
        p->nFree++;
        ప్రతిజ్ఞా( p->nFree<=TESTPCACHE_NPAGE );
      }
      తిరిగి;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  ప్రతిజ్ఞా( 0 );
}


/*
** Rekey a single page.
*/
స్థిర శూన్య testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_పుట *p_Old_పుట,
  గుర్తులేని old_కీలకం,
  గుర్తులేని new_కీలకం
){
  testpcache *p = (testpcache*)pCache;
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( p->iMagic==TESTPCACHE_VALID );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  కొరకు(i=0; i<TESTPCACHE_NPAGE; i++){
    ఒకవేళ( p->a[i].కీలకం==new_కీలకం ){
      /* The new key is never a page that is already pinned */
      ప్రతిజ్ఞా( p->a[i].isPinned==0 );
      p->a[i].కీలకం = 0;
      p->nFree++;
      ప్రతిజ్ఞా( p->nFree<=TESTPCACHE_NPAGE );
      ఆపు;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  కొరకు(i=0; i<TESTPCACHE_NPAGE; i++){
    ఒకవేళ( p->a[i].కీలకం==old_కీలకం ){
      /* The oldKey and pOldPage parameters match */
      ప్రతిజ్ఞా( &p->a[i].పుట==p_Old_పుట );
      /* Page to be rekeyed must be pinned */
      ప్రతిజ్ఞా( p->a[i].isPinned );
      p->a[i].కీలకం = new_కీలకం;
      తిరిగి;
    }
  }

  /* Rekey is always given a valid page to work with */
  ప్రతిజ్ఞా( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
స్థిర శూన్య testpcacheTruncate(sqlite3_pcache *pCache, గుర్తులేని i_హద్దు){
  testpcache *p = (testpcache*)pCache;
  గుర్తులేని పూర్ణాంకం i;
  ప్రతిజ్ఞా( p->iMagic==TESTPCACHE_VALID );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance>0 );
  కొరకు(i=0; i<TESTPCACHE_NPAGE; i++){
    ఒకవేళ( p->a[i].కీలకం>=i_హద్దు ){
      p->a[i].కీలకం = 0;
      ఒకవేళ( p->a[i].isPinned ){
        p->nPinned--;
        ప్రతిజ్ఞా( p->nPinned>=0 );
      }
      p->nFree++;
      ప్రతిజ్ఞా( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
స్థిర శూన్య testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  ప్రతిజ్ఞా( p->iMagic==TESTPCACHE_VALID );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy!=0 );
  ప్రతిజ్ఞా( testpcacheGlobal.nInstance>0 );
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
శూన్య installTestPCache(
  పూర్ణాంకం installFlag,            /* True to install.  False to uninstall. */
  గుర్తులేని discardChance,     /* 0-100.  Chance to discard on unpin */
  గుర్తులేని prng_విత్తనం_2,          /* Seed for the PRNG */
  గుర్తులేని ఎగువ_Stress         /* Call xStress aggressively */
){
  స్థిర స్థిరరాశి sqlite3_pcache_methods2 testPcache = {
    1,
    (శూన్య*)&testpcacheGlobal,
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
  స్థిర sqlite3_pcache_methods2 defaultPcache;
  స్థిర పూర్ణాంకం isInstalled = 0;

  ప్రతిజ్ఞా( testpcacheGlobal.nInstance==0 );
  ప్రతిజ్ఞా( testpcacheGlobal.pDummy==0 );
  ప్రతిజ్ఞా( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prng_విత్తనం_2 = prng_విత్తనం_2 ^ (prng_విత్తనం_2<<16);
  testpcacheGlobal.ఎగువ_Stress = ఎగువ_Stress;
  ఒకవేళ( installFlag!=isInstalled ){
    ఒకవేళ( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      ప్రతిజ్ఞా( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }లేకపోతే{
      ప్రతిజ్ఞా( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
