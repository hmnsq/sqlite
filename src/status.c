/*
** 2008 June 18
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
** This module implements the sqlite3_status() interface and related
** functionality.
*/
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#ଯଦି SQLITE_PTRSIZE>4
ପ୍ରକାରବ୍ୟାଖ୍ୟା sqlite3_int64 sqlite3_Stat_ମୂଲ୍ୟ_Type;
#ଅନ୍ୟଥା
ପ୍ରକାରବ୍ୟାଖ୍ୟା u32 sqlite3_Stat_ମୂଲ୍ୟ_Type;
#endif
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା sqlite3StatType sqlite3StatType;
ସ୍ଥିର SQLITE_WSD ସଂରଚନା sqlite3StatType {
  sqlite3_Stat_ମୂଲ୍ୟ_Type now_ମୂଲ୍ୟ[10];  /* Current value */
  sqlite3_Stat_ମୂଲ୍ୟ_Type mx_ମୂଲ୍ୟ[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର statMutex[] = {
  0,  /* SQLITE_STATUS_MEMORY_USED */
  1,  /* SQLITE_STATUS_PAGECACHE_USED */
  1,  /* SQLITE_STATUS_PAGECACHE_OVERFLOW */
  0,  /* SQLITE_STATUS_SCRATCH_USED */
  0,  /* SQLITE_STATUS_SCRATCH_OVERFLOW */
  0,  /* SQLITE_STATUS_MALLOC_SIZE */
  0,  /* SQLITE_STATUS_PARSER_STACK */
  1,  /* SQLITE_STATUS_PAGECACHE_SIZE */
  0,  /* SQLITE_STATUS_SCRATCH_SIZE */
  0,  /* SQLITE_STATUS_MALLOC_COUNT */
};


/* The "wsdStat" macro will resolve to the status information
** state vector.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdStat can refer directly
** to the "sqlite3Stat" state vector declared above.
*/
#ifdef SQLITE_OMIT_WSD
# define wsdStatInit  sqlite3StatType *x = &GLOBAL(sqlite3StatType,sqlite3Stat)
# define wsdStat x[0]
#ଅନ୍ୟଥା
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_Status_ମୂଲ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା op){
  wsdStatInit;
  ପ୍ରତିଜ୍ଞା( op>=0 && op<ArraySize(wsdStat.now_ମୂଲ୍ୟ) );
  ପ୍ରତିଜ୍ଞା( op>=0 && op<ArraySize(statMutex) );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ଫେରନ୍ତୁ wsdStat.now_ମୂଲ୍ୟ[op];
}

/*
** Add N to the value of a status record.  The caller must hold the
** appropriate mutex.  (Locking is checked by assert()).
**
** The StatusUp() routine can accept positive or negative values for N.
** The value of N is added to the current status value and the high-water
** mark is adjusted if necessary.
**
** The StatusDown() routine lowers the current value by N.  The highwater
** mark is unchanged.  N must be non-negative for StatusDown().
*/
ଶୂନ୍ୟ sqlite3StatusUp(ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  wsdStatInit;
  ପ୍ରତିଜ୍ଞା( op>=0 && op<ArraySize(wsdStat.now_ମୂଲ୍ୟ) );
  ପ୍ରତିଜ୍ଞା( op>=0 && op<ArraySize(statMutex) );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_ମୂଲ୍ୟ[op] += N;
  ଯଦି( wsdStat.now_ମୂଲ୍ୟ[op]>wsdStat.mx_ମୂଲ୍ୟ[op] ){
    wsdStat.mx_ମୂଲ୍ୟ[op] = wsdStat.now_ମୂଲ୍ୟ[op];
  }
}
ଶୂନ୍ୟ sqlite3StatusDown(ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  wsdStatInit;
  ପ୍ରତିଜ୍ଞା( N>=0 );
  ପ୍ରତିଜ୍ଞା( op>=0 && op<ArraySize(statMutex) );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ପ୍ରତିଜ୍ଞା( op>=0 && op<ArraySize(wsdStat.now_ମୂଲ୍ୟ) );
  wsdStat.now_ମୂଲ୍ୟ[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
ଶୂନ୍ୟ sqlite3StatusHighwater(ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା X){
  sqlite3_Stat_ମୂଲ୍ୟ_Type new_ମୂଲ୍ୟ;
  wsdStatInit;
  ପ୍ରତିଜ୍ଞା( X>=0 );
  new_ମୂଲ୍ୟ = (sqlite3_Stat_ମୂଲ୍ୟ_Type)X;
  ପ୍ରତିଜ୍ଞା( op>=0 && op<ArraySize(wsdStat.now_ମୂଲ୍ୟ) );
  ପ୍ରତିଜ୍ଞା( op>=0 && op<ArraySize(statMutex) );
  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ପ୍ରତିଜ୍ଞା( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_PARSER_ଗଦା );
  ଯଦି( new_ମୂଲ୍ୟ>wsdStat.mx_ମୂଲ୍ୟ[op] ){
    wsdStat.mx_ମୂଲ୍ୟ[op] = new_ମୂଲ୍ୟ;
  }
}

/*
** Query status information.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_status64(
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  ଯଦି( op<0 || op>=ArraySize(wsdStat.now_ମୂଲ୍ୟ) ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCurrent==0 || pHighwater==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_ମୂଲ୍ୟ[op];
  *pHighwater = wsdStat.mx_ମୂଲ୍ୟ[op];
  ଯଦି( resetFlag ){
    wsdStat.mx_ମୂଲ୍ୟ[op] = wsdStat.now_ମୂଲ୍ୟ[op];
  }
  sqlite3_mutex_leave(pMutex);
  (ଶୂନ୍ୟ)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  ଫେରନ୍ତୁ SQLITE_OK;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_status(ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pCurrent, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pHighwater, ପୂର୍ଣ୍ଣସଂଖ୍ୟା resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( pCurrent==0 || pHighwater==0 ) ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  ଯଦି( rc==0 ){
    *pCurrent = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)iCur;
    *pHighwater = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)iHwtr;
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
ସ୍ଥିର u32 ଗଣନା_Lookaside_Slots(LookasideSlot *p){
  u32 cnt = 0;
  ଯେତେବେଳେ( p ){
    p = p->pNext;
    cnt++;
  }
  ଫେରନ୍ତୁ cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3LookasideUsed(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pHighwater){
  u32 nInit = ଗଣନା_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = ଗଣନା_Lookaside_Slots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += ଗଣନା_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += ଗଣନା_Lookaside_Slots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ପ୍ରତିଜ୍ଞା( db->lookaside.nSlot >= nInit+nFree );
  ଯଦି( pHighwater ) *pHighwater = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(db->lookaside.nSlot - nInit);
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା resetFlag            /* Reset high-water mark if true */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  ବାଛ( op ){
    ସ୍ଥିତି SQLITE_DBSTATUS_LOOKASIDE_USED: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      ଯଦି( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        ଯଦି( p ){
          ଯେତେବେଳେ( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        ଯଦି( p ){
          ଯେତେବେଳେ( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି SQLITE_DBSTATUS_LOOKASIDE_HIT:
    ସ୍ଥିତି SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    ସ୍ଥିତି SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      ପ୍ରତିଜ୍ଞା( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      ପ୍ରତିଜ୍ଞା( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      ଯଦି( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      ଭାଙ୍ଗ;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    ସ୍ଥିତି SQLITE_DBSTATUS_CACHE_USED_SHARED:
    ସ୍ଥିତି SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      sqlite3BtreeEnterAll(db);
      ପାଇଁ(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        ଯଦି( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte = sqlite3PagerMemUsed(pPager);
          ଯଦି( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_ଗଣନା(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      ଭାଙ୍ଗ;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    ସ୍ଥିତି SQLITE_DBSTATUS_SCHEMA_USED: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;          /* Used to iterate through schemas */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      ପ୍ରତିଜ୍ଞା( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      ପାଇଁ(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        ଯଦି( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(ଆକାର(HashElem)) * (
              pSchema->tblHash.ଗଣନା_2 
            + pSchema->trigHash.ଗଣନା_2
            + pSchema->idxHash.ଗଣନା_2
            + pSchema->fkeyHash.ଗଣନା_2
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          ପାଇଁ(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          ପାଇଁ(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      ଭାଙ୍ଗ;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    ସ୍ଥିତି SQLITE_DBSTATUS_STMT_USED: {
      ସଂରଚନା Vdbe *pVdbe;         /* Used to iterate through VMs */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      ପ୍ରତିଜ୍ଞା( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      ପାଇଁ(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      ଭାଙ୍ଗ;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    ସ୍ଥିତି SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    ସ୍ଥିତି SQLITE_DBSTATUS_CACHE_HIT:
    ସ୍ଥିତି SQLITE_DBSTATUS_CACHE_MISS:
    ସ୍ଥିତି SQLITE_DBSTATUS_CACHE_WRITE:{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      u64 nRet = 0;
      ପ୍ରତିଜ୍ଞା( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      ପ୍ରତିଜ୍ଞା( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      ପାଇଁ(i=0; i<db->nDb; i++){
        ଯଦି( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      ଭାଙ୍ଗ;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    ସ୍ଥିତି SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      ଯଦି( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      ଯଦି( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      ଭାଙ୍ଗ;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    ସ୍ଥିତି SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      ଭାଙ୍ଗ;
    }

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ଫେରନ୍ତୁ rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,                  /* Status verb */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pCurrent,           /* Write current value here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pHighwtr,           /* Write high-water mark here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  ଯଦି( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  ଫେରନ୍ତୁ rc;
}
