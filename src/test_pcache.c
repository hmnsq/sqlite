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
#ضمن "sqlite3.h"
#ضمن <string.h>
#ضمن <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
تعريفنوع بنية testpcacheGlobalType testpcacheGlobalType;
بنية testpcacheGlobalType {
  فراغ *pDummy;             /* Dummy allocation to simulate failures */
  صحيح nInstance;            /* Number of current instances */
  بیعلامت discardChance;   /* Chance of discarding on an unpin (0-100) */
  بیعلامت prng_دانه_2;        /* Seed for the PRNG */
  بیعلامت بالا_Stress;      /* Call xStress aggressively */
};
ساكن testpcacheGlobalType testpcacheGlobal;

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
ساكن صحيح testpcacheInit(فراغ *pArg){
  أكد( pArg==(فراغ*)&testpcacheGlobal );
  أكد( testpcacheGlobal.pDummy==0 );
  أكد( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  أرجع testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
ساكن فراغ testpcacheShutdown(فراغ *pArg){
  أكد( pArg==(فراغ*)&testpcacheGlobal );
  أكد( testpcacheGlobal.pDummy!=0 );
  أكد( testpcacheGlobal.nInstance==0 );
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
#إذا defined(SQLITE_موقت_STORE) && SQLITE_موقت_STORE>=2
# define TESTPCACHE_NPAGE    499
#وإلا
# define TESTPCACHE_NPAGE    217
#پایان_اگر
#تعریف_کن TESTPCACHE_RESERVE   17

/*
** Magic numbers used to determine validity of the page cache.
*/
#تعریف_کن TESTPCACHE_VALID  0x364585fd
#تعریف_کن TESTPCACHE_CLEAR  0xd42670d4

/*
** Private implementation of a page cache.
*/
تعريفنوع بنية testpcache testpcache;
بنية testpcache {
  sqlite3_int64 sz_صفحه;     /* Size of each page.  Multiple of 8. */
  صحيح szExtra;              /* Size of extra data that accompanies each page */
  صحيح bPurgeable;           /* True if the page cache is purgeable */
  صحيح nFree;                /* Number of unused slots in a[] */
  صحيح nPinned;              /* Number of pinned slots in a[] */
  بیعلامت iRand;           /* State of the PRNG */
  بیعلامت iMagic;          /* Magic number for sanity checking */
  بنية testpcache_صفحه {
    sqlite3_pcache_صفحه صفحه;  /* Base class */
    بیعلامت کلید;              /* The key for this page. 0 means unallocated */
    صحيح isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
ساكن بیعلامت testpcacheRandom(testpcache *p){
  بیعلامت x = 0;
  صحيح i;
  لكل(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  أرجع x;
}


/*
** Allocate a new page cache instance.
*/
ساكن sqlite3_pcache *testpcacheCreate(
  صحيح sz_صفحه,
  صحيح szExtra,
  صحيح bPurgeable
){
  صحيح nMem;
  حرف *x;
  testpcache *p;
  صحيح i;
  أكد( testpcacheGlobal.pDummy!=0 );
  sz_صفحه = (sz_صفحه+7)&~7;
  szExtra = (sz_صفحه+7)&~7;
  nMem = حجم(testpcache) + TESTPCACHE_NPAGE*(sz_صفحه+szExtra);
  p = sqlite3_malloc( nMem );
  إذا( p==0 ) أرجع 0;
  x = (حرف*)&p[1];
  p->sz_صفحه = sz_صفحه;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prng_دانه_2;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  لكل(i=0; i<TESTPCACHE_NPAGE; i++, x += (sz_صفحه+szExtra)){
    p->a[i].کلید = 0;
    p->a[i].isPinned = 0;
    p->a[i].صفحه.pBuf = (فراغ*)x;
    p->a[i].صفحه.pExtra = (فراغ*)&x[sz_صفحه];
  }
  testpcacheGlobal.nInstance++;
  أرجع (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
ساكن فراغ testpcacheCachesize(sqlite3_pcache *pCache, صحيح new_اندازه_2){
  testpcache *p = (testpcache*)pCache;
  أكد( p->iMagic==TESTPCACHE_VALID );
  أكد( testpcacheGlobal.pDummy!=0 );
  أكد( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
ساكن صحيح testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  أكد( p->iMagic==TESTPCACHE_VALID );
  أكد( testpcacheGlobal.pDummy!=0 );
  أكد( testpcacheGlobal.nInstance>0 );
  أرجع TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
ساكن sqlite3_pcache_صفحه *testpcacheFetch(
  sqlite3_pcache *pCache,
  بیعلامت کلید,
  صحيح createFlag
){
  testpcache *p = (testpcache*)pCache;
  صحيح i, j;
  أكد( p->iMagic==TESTPCACHE_VALID );
  أكد( testpcacheGlobal.pDummy!=0 );
  أكد( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  لكل(i=0; i<TESTPCACHE_NPAGE; i++){
    إذا( p->a[i].کلید==کلید ){
      إذا( !p->a[i].isPinned ){
        p->nPinned++;
        أكد( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      أرجع &p->a[i].صفحه;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  إذا( createFlag==0 ){
    أرجع 0;
  }

  /* If no pages are available, always fail */
  إذا( p->nPinned==TESTPCACHE_NPAGE ){
    أرجع 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  إذا( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    أرجع 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  إذا( testpcacheGlobal.بالا_Stress && createFlag<2 ){
    أرجع 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  إذا( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    لكل(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      إذا( p->a[j].کلید==0 ){
        p->a[j].کلید = کلید;
        p->a[j].isPinned = 1;
        memset(p->a[j].صفحه.pBuf, 0, p->sz_صفحه);
        memset(p->a[j].صفحه.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        أكد( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        أرجع &p->a[j].صفحه;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    أكد( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  إذا( p->bPurgeable==0 ){
    أرجع 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  لكل(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    إذا( p->a[j].کلید>0 && p->a[j].isPinned==0 ){
      p->a[j].کلید = کلید;
      p->a[j].isPinned = 1;
      memset(p->a[j].صفحه.pBuf, 0, p->sz_صفحه);
      memset(p->a[j].صفحه.pExtra, 0, p->szExtra);
      p->nPinned++;
      أكد( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      أرجع &p->a[j].صفحه;
    }
  }

  /* The previous loop always finds a page to recycle. */
  أكد(0);
  أرجع 0;
}

/*
** Unpin a page.
*/
ساكن فراغ testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_صفحه *p_Old_صفحه,
  صحيح discard
){
  testpcache *p = (testpcache*)pCache;
  صحيح i;
  أكد( p->iMagic==TESTPCACHE_VALID );
  أكد( testpcacheGlobal.pDummy!=0 );
  أكد( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  إذا( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  لكل(i=0; i<TESTPCACHE_NPAGE; i++){
    إذا( &p->a[i].صفحه==p_Old_صفحه ){
      /* The pOldPage pointer always points to a pinned page */
      أكد( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      أكد( p->nPinned>=0 );
      إذا( discard ){
        p->a[i].کلید = 0;
        p->nFree++;
        أكد( p->nFree<=TESTPCACHE_NPAGE );
      }
      أرجع;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  أكد( 0 );
}


/*
** Rekey a single page.
*/
ساكن فراغ testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_صفحه *p_Old_صفحه,
  بیعلامت old_کلید,
  بیعلامت new_کلید
){
  testpcache *p = (testpcache*)pCache;
  صحيح i;
  أكد( p->iMagic==TESTPCACHE_VALID );
  أكد( testpcacheGlobal.pDummy!=0 );
  أكد( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  لكل(i=0; i<TESTPCACHE_NPAGE; i++){
    إذا( p->a[i].کلید==new_کلید ){
      /* The new key is never a page that is already pinned */
      أكد( p->a[i].isPinned==0 );
      p->a[i].کلید = 0;
      p->nFree++;
      أكد( p->nFree<=TESTPCACHE_NPAGE );
      اكسر;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  لكل(i=0; i<TESTPCACHE_NPAGE; i++){
    إذا( p->a[i].کلید==old_کلید ){
      /* The oldKey and pOldPage parameters match */
      أكد( &p->a[i].صفحه==p_Old_صفحه );
      /* Page to be rekeyed must be pinned */
      أكد( p->a[i].isPinned );
      p->a[i].کلید = new_کلید;
      أرجع;
    }
  }

  /* Rekey is always given a valid page to work with */
  أكد( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
ساكن فراغ testpcacheTruncate(sqlite3_pcache *pCache, بیعلامت i_حد){
  testpcache *p = (testpcache*)pCache;
  بیعلامت صحيح i;
  أكد( p->iMagic==TESTPCACHE_VALID );
  أكد( testpcacheGlobal.pDummy!=0 );
  أكد( testpcacheGlobal.nInstance>0 );
  لكل(i=0; i<TESTPCACHE_NPAGE; i++){
    إذا( p->a[i].کلید>=i_حد ){
      p->a[i].کلید = 0;
      إذا( p->a[i].isPinned ){
        p->nPinned--;
        أكد( p->nPinned>=0 );
      }
      p->nFree++;
      أكد( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
ساكن فراغ testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  أكد( p->iMagic==TESTPCACHE_VALID );
  أكد( testpcacheGlobal.pDummy!=0 );
  أكد( testpcacheGlobal.nInstance>0 );
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
فراغ installTestPCache(
  صحيح installFlag,            /* True to install.  False to uninstall. */
  بیعلامت discardChance,     /* 0-100.  Chance to discard on unpin */
  بیعلامت prng_دانه_2,          /* Seed for the PRNG */
  بیعلامت بالا_Stress         /* Call xStress aggressively */
){
  ساكن ثابت sqlite3_pcache_methods2 testPcache = {
    1,
    (فراغ*)&testpcacheGlobal,
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
  ساكن sqlite3_pcache_methods2 defaultPcache;
  ساكن صحيح isInstalled = 0;

  أكد( testpcacheGlobal.nInstance==0 );
  أكد( testpcacheGlobal.pDummy==0 );
  أكد( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prng_دانه_2 = prng_دانه_2 ^ (prng_دانه_2<<16);
  testpcacheGlobal.بالا_Stress = بالا_Stress;
  إذا( installFlag!=isInstalled ){
    إذا( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      أكد( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }وإلا{
      أكد( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
