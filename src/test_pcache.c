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
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

/*
** Global data used by this test implementation.  There is no
** mutexing, which means this page cache will not work in a
** multi-threaded test.
*/
प्रकारनिर्देश संरचना testpcacheGlobalType testpcacheGlobalType;
संरचना testpcacheGlobalType {
  शून्य *pDummy;             /* Dummy allocation to simulate failures */
  पूर्णाङ्क nInstance;            /* Number of current instances */
  अचिह्नित discardChance;   /* Chance of discarding on an unpin (0-100) */
  अचिह्नित prng_बीजम्_2;        /* Seed for the PRNG */
  अचिह्नित उच्चः_Stress;      /* Call xStress aggressively */
};
स्थिर testpcacheGlobalType testpcacheGlobal;

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
स्थिर पूर्णाङ्क testpcacheInit(शून्य *pArg){
  प्रतिज्ञा( pArg==(शून्य*)&testpcacheGlobal );
  प्रतिज्ञा( testpcacheGlobal.pDummy==0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance==0 );
  testpcacheGlobal.pDummy = sqlite3_malloc(10);
  प्रत्यागम testpcacheGlobal.pDummy==0 ? SQLITE_NOMEM : SQLITE_OK;
}

/*
** Destructor
**
** Verify that this is only called after initialization.
** Free the memory allocated by the initializer.
*/
स्थिर शून्य testpcacheShutdown(शून्य *pArg){
  प्रतिज्ञा( pArg==(शून्य*)&testpcacheGlobal );
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance==0 );
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
#यदि defined(SQLITE_अस्थायी_STORE) && SQLITE_अस्थायी_STORE>=2
# define TESTPCACHE_NPAGE    499
#अन्यथा
# define TESTPCACHE_NPAGE    217
#यदि_अन्तः
#परिभाषय TESTPCACHE_RESERVE   17

/*
** Magic numbers used to determine validity of the page cache.
*/
#परिभाषय TESTPCACHE_VALID  0x364585fd
#परिभाषय TESTPCACHE_CLEAR  0xd42670d4

/*
** Private implementation of a page cache.
*/
प्रकारनिर्देश संरचना testpcache testpcache;
संरचना testpcache {
  sqlite3_int64 szPage;     /* Size of each page.  Multiple of 8. */
  पूर्णाङ्क szExtra;              /* Size of extra data that accompanies each page */
  पूर्णाङ्क bPurgeable;           /* True if the page cache is purgeable */
  पूर्णाङ्क nFree;                /* Number of unused slots in a[] */
  पूर्णाङ्क nPinned;              /* Number of pinned slots in a[] */
  अचिह्नित iRand;           /* State of the PRNG */
  अचिह्नित iMagic;          /* Magic number for sanity checking */
  संरचना testpcachePage {
    sqlite3_pcache_page page;  /* Base class */
    अचिह्नित कुञ्चिका;              /* The key for this page. 0 means unallocated */
    पूर्णाङ्क isPinned;              /* True if the page is pinned */
  } a[TESTPCACHE_NPAGE];    /* All pages in the cache */
};

/*
** Get a random number using the PRNG in the given page cache.
*/
स्थिर अचिह्नित testpcacheRandom(testpcache *p){
  अचिह्नित x = 0;
  पूर्णाङ्क i;
  प्रति(i=0; i<4; i++){
    p->iRand = (p->iRand*69069 + 5);
    x = (x<<8) | ((p->iRand>>16)&0xff);
  }
  प्रत्यागम x;
}


/*
** Allocate a new page cache instance.
*/
स्थिर sqlite3_pcache *testpcacheCreate(
  पूर्णाङ्क szPage,
  पूर्णाङ्क szExtra,
  पूर्णाङ्क bPurgeable
){
  पूर्णाङ्क nMem;
  वर्ण *x;
  testpcache *p;
  पूर्णाङ्क i;
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  szPage = (szPage+7)&~7;
  szExtra = (szPage+7)&~7;
  nMem = परिमाण(testpcache) + TESTPCACHE_NPAGE*(szPage+szExtra);
  p = sqlite3_malloc( nMem );
  यदि( p==0 ) प्रत्यागम 0;
  x = (वर्ण*)&p[1];
  p->szPage = szPage;
  p->szExtra = szExtra;
  p->nFree = TESTPCACHE_NPAGE;
  p->nPinned = 0;
  p->iRand = testpcacheGlobal.prng_बीजम्_2;
  p->bPurgeable = bPurgeable;
  p->iMagic = TESTPCACHE_VALID;
  प्रति(i=0; i<TESTPCACHE_NPAGE; i++, x += (szPage+szExtra)){
    p->a[i].कुञ्चिका = 0;
    p->a[i].isPinned = 0;
    p->a[i].page.pBuf = (शून्य*)x;
    p->a[i].page.pExtra = (शून्य*)&x[szPage];
  }
  testpcacheGlobal.nInstance++;
  प्रत्यागम (sqlite3_pcache*)p;
}

/*
** Set the cache size
*/
स्थिर शून्य testpcacheCachesize(sqlite3_pcache *pCache, पूर्णाङ्क new_आकार_2){
  testpcache *p = (testpcache*)pCache;
  प्रतिज्ञा( p->iMagic==TESTPCACHE_VALID );
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance>0 );
}

/*
** Return the number of pages in the cache that are being used.
** This includes both pinned and unpinned pages.
*/
स्थिर पूर्णाङ्क testpcachePagecount(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  प्रतिज्ञा( p->iMagic==TESTPCACHE_VALID );
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance>0 );
  प्रत्यागम TESTPCACHE_NPAGE - p->nFree;
}

/*
** Fetch a page.
*/
स्थिर sqlite3_pcache_page *testpcacheFetch(
  sqlite3_pcache *pCache,
  अचिह्नित कुञ्चिका,
  पूर्णाङ्क createFlag
){
  testpcache *p = (testpcache*)pCache;
  पूर्णाङ्क i, j;
  प्रतिज्ञा( p->iMagic==TESTPCACHE_VALID );
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance>0 );

  /* See if the page is already in cache.  Return immediately if it is */
  प्रति(i=0; i<TESTPCACHE_NPAGE; i++){
    यदि( p->a[i].कुञ्चिका==कुञ्चिका ){
      यदि( !p->a[i].isPinned ){
        p->nPinned++;
        प्रतिज्ञा( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        p->a[i].isPinned = 1;
      }
      प्रत्यागम &p->a[i].page;
    }
  }

  /* If createFlag is 0, never allocate a new page */
  यदि( createFlag==0 ){
    प्रत्यागम 0;
  }

  /* If no pages are available, always fail */
  यदि( p->nPinned==TESTPCACHE_NPAGE ){
    प्रत्यागम 0;
  }

  /* Do not allocate the last TESTPCACHE_RESERVE pages unless createFlag is 2 */
  यदि( p->nPinned>=TESTPCACHE_NPAGE-TESTPCACHE_RESERVE && createFlag<2 ){
    प्रत्यागम 0;
  }

  /* Do not allocate if highStress is enabled and createFlag is not 2.
  **
  ** The highStress setting causes pagerStress() to be called much more
  ** often, which exercises the pager logic more intensely.
  */
  यदि( testpcacheGlobal.उच्चः_Stress && createFlag<2 ){
    प्रत्यागम 0;
  }

  /* Find a free page to allocate if there are any free pages.
  ** Withhold TESTPCACHE_RESERVE free pages until createFlag is 2.
  */
  यदि( p->nFree>TESTPCACHE_RESERVE || (createFlag==2 && p->nFree>0) ){
    j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
    प्रति(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
      यदि( p->a[j].कुञ्चिका==0 ){
        p->a[j].कुञ्चिका = कुञ्चिका;
        p->a[j].isPinned = 1;
        स्मृतिं_पूरय(p->a[j].page.pBuf, 0, p->szPage);
        स्मृतिं_पूरय(p->a[j].page.pExtra, 0, p->szExtra);
        p->nPinned++;
        p->nFree--;
        प्रतिज्ञा( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
        प्रत्यागम &p->a[j].page;
      }
    }

    /* The prior loop always finds a freepage to allocate */
    प्रतिज्ञा( 0 );
  }

  /* If this cache is not purgeable then we have to fail.
  */
  यदि( p->bPurgeable==0 ){
    प्रत्यागम 0;
  }

  /* If there are no free pages, recycle a page.  The page to
  ** recycle is selected at random from all unpinned pages.
  */
  j = testpcacheRandom(p) % TESTPCACHE_NPAGE;
  प्रति(i=0; i<TESTPCACHE_NPAGE; i++, j = (j+1)%TESTPCACHE_NPAGE){
    यदि( p->a[j].कुञ्चिका>0 && p->a[j].isPinned==0 ){
      p->a[j].कुञ्चिका = कुञ्चिका;
      p->a[j].isPinned = 1;
      स्मृतिं_पूरय(p->a[j].page.pBuf, 0, p->szPage);
      स्मृतिं_पूरय(p->a[j].page.pExtra, 0, p->szExtra);
      p->nPinned++;
      प्रतिज्ञा( p->nPinned <= TESTPCACHE_NPAGE - p->nFree );
      प्रत्यागम &p->a[j].page;
    }
  }

  /* The previous loop always finds a page to recycle. */
  प्रतिज्ञा(0);
  प्रत्यागम 0;
}

/*
** Unpin a page.
*/
स्थिर शून्य testpcacheUnpin(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  पूर्णाङ्क discard
){
  testpcache *p = (testpcache*)pCache;
  पूर्णाङ्क i;
  प्रतिज्ञा( p->iMagic==TESTPCACHE_VALID );
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance>0 );

  /* Randomly discard pages as they are unpinned according to the
  ** discardChance setting.  If discardChance is 0, the random discard
  ** never happens.  If discardChance is 100, it always happens.
  */
  यदि( p->bPurgeable
  && (100-testpcacheGlobal.discardChance) <= (testpcacheRandom(p)%100)
  ){
    discard = 1;
  }

  प्रति(i=0; i<TESTPCACHE_NPAGE; i++){
    यदि( &p->a[i].page==pOldPage ){
      /* The pOldPage pointer always points to a pinned page */
      प्रतिज्ञा( p->a[i].isPinned );
      p->a[i].isPinned = 0;
      p->nPinned--;
      प्रतिज्ञा( p->nPinned>=0 );
      यदि( discard ){
        p->a[i].कुञ्चिका = 0;
        p->nFree++;
        प्रतिज्ञा( p->nFree<=TESTPCACHE_NPAGE );
      }
      प्रत्यागम;
    }
  }

  /* The pOldPage pointer always points to a valid page */
  प्रतिज्ञा( 0 );
}


/*
** Rekey a single page.
*/
स्थिर शून्य testpcacheRekey(
  sqlite3_pcache *pCache,
  sqlite3_pcache_page *pOldPage,
  अचिह्नित old_कुञ्चिका,
  अचिह्नित new_कुञ्चिका
){
  testpcache *p = (testpcache*)pCache;
  पूर्णाङ्क i;
  प्रतिज्ञा( p->iMagic==TESTPCACHE_VALID );
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance>0 );

  /* If there already exists another page at newKey, verify that
  ** the other page is unpinned and discard it.
  */
  प्रति(i=0; i<TESTPCACHE_NPAGE; i++){
    यदि( p->a[i].कुञ्चिका==new_कुञ्चिका ){
      /* The new key is never a page that is already pinned */
      प्रतिज्ञा( p->a[i].isPinned==0 );
      p->a[i].कुञ्चिका = 0;
      p->nFree++;
      प्रतिज्ञा( p->nFree<=TESTPCACHE_NPAGE );
      भङ्ग;
    }
  }

  /* Find the page to be rekeyed and rekey it.
  */
  प्रति(i=0; i<TESTPCACHE_NPAGE; i++){
    यदि( p->a[i].कुञ्चिका==old_कुञ्चिका ){
      /* The oldKey and pOldPage parameters match */
      प्रतिज्ञा( &p->a[i].page==pOldPage );
      /* Page to be rekeyed must be pinned */
      प्रतिज्ञा( p->a[i].isPinned );
      p->a[i].कुञ्चिका = new_कुञ्चिका;
      प्रत्यागम;
    }
  }

  /* Rekey is always given a valid page to work with */
  प्रतिज्ञा( 0 );
}


/*
** Truncate the page cache.  Every page with a key of iLimit or larger
** is discarded.
*/
स्थिर शून्य testpcacheTruncate(sqlite3_pcache *pCache, अचिह्नित i_सीमा){
  testpcache *p = (testpcache*)pCache;
  अचिह्नित पूर्णाङ्क i;
  प्रतिज्ञा( p->iMagic==TESTPCACHE_VALID );
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance>0 );
  प्रति(i=0; i<TESTPCACHE_NPAGE; i++){
    यदि( p->a[i].कुञ्चिका>=i_सीमा ){
      p->a[i].कुञ्चिका = 0;
      यदि( p->a[i].isPinned ){
        p->nPinned--;
        प्रतिज्ञा( p->nPinned>=0 );
      }
      p->nFree++;
      प्रतिज्ञा( p->nFree<=TESTPCACHE_NPAGE );
    }
  }
}

/*
** Destroy a page cache.
*/
स्थिर शून्य testpcacheDestroy(sqlite3_pcache *pCache){
  testpcache *p = (testpcache*)pCache;
  प्रतिज्ञा( p->iMagic==TESTPCACHE_VALID );
  प्रतिज्ञा( testpcacheGlobal.pDummy!=0 );
  प्रतिज्ञा( testpcacheGlobal.nInstance>0 );
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
शून्य installTestPCache(
  पूर्णाङ्क installFlag,            /* True to install.  False to uninstall. */
  अचिह्नित discardChance,     /* 0-100.  Chance to discard on unpin */
  अचिह्नित prng_बीजम्_2,          /* Seed for the PRNG */
  अचिह्नित उच्चः_Stress         /* Call xStress aggressively */
){
  स्थिर ध्रुव sqlite3_pcache_methods2 testPcache = {
    1,
    (शून्य*)&testpcacheGlobal,
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
  स्थिर sqlite3_pcache_methods2 defaultPcache;
  स्थिर पूर्णाङ्क isInstalled = 0;

  प्रतिज्ञा( testpcacheGlobal.nInstance==0 );
  प्रतिज्ञा( testpcacheGlobal.pDummy==0 );
  प्रतिज्ञा( discardChance<=100 );
  testpcacheGlobal.discardChance = discardChance;
  testpcacheGlobal.prng_बीजम्_2 = prng_बीजम्_2 ^ (prng_बीजम्_2<<16);
  testpcacheGlobal.उच्चः_Stress = उच्चः_Stress;
  यदि( installFlag!=isInstalled ){
    यदि( installFlag ){
      sqlite3_config(SQLITE_CONFIG_GETPCACHE2, &defaultPcache);
      प्रतिज्ञा( defaultPcache.xCreate!=testpcacheCreate );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &testPcache);
    }अन्यथा{
      प्रतिज्ञा( defaultPcache.xCreate!=0 );
      sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultPcache);
    }
    isInstalled = installFlag;
  }
}
