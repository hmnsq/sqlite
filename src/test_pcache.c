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
#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
પ્રકારવ્યાખ્યા સંરચના testpcacheGlobalType testpcacheGlobalType;
સંરચના testpcacheGlobalType {
  શૂન્ય *pDummy;             /* Dummy allocation to simulate failures */
  પૂર્ણાંક nInstance;            /* Number of current instances */
  unsigned discardChance;   /* Chance of discarding on an unpin (0-100) */
  unsigned prngSeed;        /* Seed for the PRNG */
  unsigned ઉપલું_Stress;      /* Call xStress aggressively */
};
સ્થિર testpcacheGlobalType testpcacheGlobal;

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
સ્થિર પૂર્ણાંક testpcacheInit(શૂન્ય *pArg){
  પ્રતિજ્ઞા( pArg==(શૂન્ય*)&testpcacheGlobal );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy==0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  પાછા testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
સ્થિર શૂન્ય testpcacheShutdown(શૂન્ય *pArg){
  પ્રતિજ્ઞા( pArg==(શૂન્ય*)&testpcacheGlobal );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance==0 );
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
#જો defined(SQLITE_TEMP_STORE) && SQLITE_TEMP_STORE>=2
# define TESTPCACHE_NPAGE    499
#નહીંતર
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
પ્રકારવ્યાખ્યા સંરચના testpcache testpcache;
સંરચના testpcache {
  sqlite3_int64 szPage;     /* Size of each page.  Multiple of 8. */
  પૂર્ણાંક szExtra;              /* Size of extra data that accompanies each page */
  પૂર્ણાંક bPurgeable;           /* True if the page cache is purgeable */
  પૂર્ણાંક nFree;                /* Number of unused slots in a[] */
  પૂર્ણાંક nPinned;              /* Number of pinned slots in a[] */
  unsigned iRand;           /* State of the PRNG */
  unsigned iMagic;          /* Magic number for sanity checking */
  સંરચના testpcachePage {
    sqlite3_pcache_page page;  /* Base class */
    unsigned ચાવી;              /* The key for this page. 0 means unallocated */
    પૂર્ણાંક isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
સ્થિર unsigned testpcacheRandom(testpcache *p){
  unsigned x = 0;
  પૂર્ણાંક i;
  માટે(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  પાછા x;
}


/*
** Allocate a new page cache instance.
*/
સ્થિર sqlite3_pcache *testpcacheCreate(
  પૂર્ણાંક szPage,
  પૂર્ણાંક szExtra,
  પૂર્ણાંક bPurgeable
){
  પૂર્ણાંક nMem;
  અક્ષર *x;
  testpcache *p;
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  szPage = (szPage+7)&~7;
  szExtra = (szPage+7)&~7;
  nMem = કદ(testpcache) + TESTPCACHE_NPAGE*(szPage+szExtra);
  p = sqlite3_malloc( nMem );
  જો( p==0 ) પાછા 0;
  x = (અક્ષર*)&p[1];
  p->szPage = szPage;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prngSeed;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  માટે(i=0; i<TESTPCACHE_NPAGE; i++, x += (szPage+szExtra)){
    p->a[i].ચાવી = 0;
    p->a[i].isPinned = 0;
    p->a[i].page.pBuf = (શૂન્ય*)x;
    p->a[i].page.pExtra = (શૂન્ય*)&x[szPage];
  }
  testpcacheGlobal.nInstance++;
  પાછા (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
સ્થિર શૂન્ય testpcacheCachesize(sqlite3_pcache *pCache, પૂર્ણાંક newSize){
  testpcache *p = (testpcache*)pCache;
  પ્રતિજ્ઞા( p->iMagic==TESTPCACHE_VALID );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
સ્થિર પૂર્ણાંક testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  પ્રતિજ્ઞા( p->iMagic==TESTPCACHE_VALID );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance>0 );
  પાછા TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
સ્થિર sqlite3_pcache_page *testpcacheFetch(
  sqlite3_pcache *pCache,
  unsigned ચાવી,
  પૂર્ણાંક createFlag
){
  testpcache *p = (testpcache*)pCache;
  પૂર્ણાંક i, j;
  પ્રતિજ્ઞા( p->iMagic==TESTPCACHE_VALID );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  માટે(i=0; i<TESTPCACHE_NPAGE; i++){
    જો( p->a[i].ચાવી==ચાવી ){
      જો( !p->a[i].isPinned ){
        p->nPinned++;
        પ્રતિજ્ઞા( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      પાછા &p->a[i].page;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  જો( createFlag==0 ){
    પાછા 0;
  }

  /* If no pages are available, always fail */
  જો( p->nPinned==TESTPCACHE_NPAGE ){
    પાછા 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  જો( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    પાછા 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  જો( testpcacheGlobal.ઉપલું_Stress && createFlag<2 ){
    પાછા 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  જો( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    માટે(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      જો( p->a[j].ચાવી==0 ){
        p->a[j].ચાવી = ચાવી;
        p->a[j].isPinned = 1;
        memset(p->a[j].page.pBuf, 0, p->szPage);
        memset(p->a[j].page.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        પ્રતિજ્ઞા( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        પાછા &p->a[j].page;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    પ્રતિજ્ઞા( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  જો( p->bPurgeable==0 ){
    પાછા 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  માટે(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    જો( p->a[j].ચાવી>0 && p->a[j].isPinned==0 ){
      p->a[j].ચાવી = ચાવી;
      p->a[j].isPinned = 1;
      memset(p->a[j].page.pBuf, 0, p->szPage);
      memset(p->a[j].page.pExtra, 0, p->szExtra);
      p->nPinned++;
      પ્રતિજ્ઞા( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      પાછા &p->a[j].page;
    }
  }

  /* The previous loop always finds a page to recycle. */
  પ્રતિજ્ઞા(0);
  પાછા 0;
}

/*
** Unpin a page.
*/
સ્થિર શૂન્ય testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  પૂર્ણાંક discard
){
  testpcache *p = (testpcache*)pCache;
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( p->iMagic==TESTPCACHE_VALID );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  જો( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  માટે(i=0; i<TESTPCACHE_NPAGE; i++){
    જો( &p->a[i].page==pOldPage ){
      /* The pOldPage pointer always points to a pinned page */
      પ્રતિજ્ઞા( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      પ્રતિજ્ઞા( p->nPinned>=0 );
      જો( discard ){
        p->a[i].ચાવી = 0;
        p->nFree++;
        પ્રતિજ્ઞા( p->nFree<=TESTPCACHE_NPAGE );
      }
      પાછા;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  પ્રતિજ્ઞા( 0 );
}


/*
** Rekey a single page.
*/
સ્થિર શૂન્ય testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  unsigned old_ચાવી,
  unsigned new_ચાવી
){
  testpcache *p = (testpcache*)pCache;
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( p->iMagic==TESTPCACHE_VALID );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  માટે(i=0; i<TESTPCACHE_NPAGE; i++){
    જો( p->a[i].ચાવી==new_ચાવી ){
      /* The new key is never a page that is already pinned */
      પ્રતિજ્ઞા( p->a[i].isPinned==0 );
      p->a[i].ચાવી = 0;
      p->nFree++;
      પ્રતિજ્ઞા( p->nFree<=TESTPCACHE_NPAGE );
      તોડો;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  માટે(i=0; i<TESTPCACHE_NPAGE; i++){
    જો( p->a[i].ચાવી==old_ચાવી ){
      /* The oldKey and pOldPage parameters match */
      પ્રતિજ્ઞા( &p->a[i].page==pOldPage );
      /* Page to be rekeyed must be pinned */
      પ્રતિજ્ઞા( p->a[i].isPinned );
      p->a[i].ચાવી = new_ચાવી;
      પાછા;
    }
  }

  /* Rekey is always given a valid page to work with */
  પ્રતિજ્ઞા( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
સ્થિર શૂન્ય testpcacheTruncate(sqlite3_pcache *pCache, unsigned iLimit){
  testpcache *p = (testpcache*)pCache;
  unsigned પૂર્ણાંક i;
  પ્રતિજ્ઞા( p->iMagic==TESTPCACHE_VALID );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance>0 );
  માટે(i=0; i<TESTPCACHE_NPAGE; i++){
    જો( p->a[i].ચાવી>=iLimit ){
      p->a[i].ચાવી = 0;
      જો( p->a[i].isPinned ){
        p->nPinned--;
        પ્રતિજ્ઞા( p->nPinned>=0 );
      }
      p->nFree++;
      પ્રતિજ્ઞા( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
સ્થિર શૂન્ય testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  પ્રતિજ્ઞા( p->iMagic==TESTPCACHE_VALID );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy!=0 );
  પ્રતિજ્ઞા( testpcacheGlobal.nInstance>0 );
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
શૂન્ય installTestPCache(
  પૂર્ણાંક installFlag,            /* True to install.  False to uninstall. */
  unsigned discardChance,     /* 0-100.  Chance to discard on unpin */
  unsigned prngSeed,          /* Seed for the PRNG */
  unsigned ઉપલું_Stress         /* Call xStress aggressively */
){
  સ્થિર અચલ sqlite3_pcache_methods2 testPcache = {
    1,
    (શૂન્ય*)&testpcacheGlobal,
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
  સ્થિર sqlite3_pcache_methods2 defaultPcache;
  સ્થિર પૂર્ણાંક isInstalled = 0;

  પ્રતિજ્ઞા( testpcacheGlobal.nInstance==0 );
  પ્રતિજ્ઞા( testpcacheGlobal.pDummy==0 );
  પ્રતિજ્ઞા( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prngSeed = prngSeed ^ (prngSeed<<16);
  testpcacheGlobal.ઉપલું_Stress = ઉપલું_Stress;
  જો( installFlag!=isInstalled ){
    જો( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      પ્રતિજ્ઞા( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }નહીંતર{
      પ્રતિજ્ઞા( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
