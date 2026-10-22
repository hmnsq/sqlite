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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#যদি SQLITE_PTRSIZE>4
প্রকারলেখ sqlite3_int64 sqlite3_Stat_মান_Type;
#অন্যথা
প্রকারলেখ u32 sqlite3_Stat_মান_Type;
#endif
প্রকারলেখ কাঠামো sqlite3StatType sqlite3StatType;
স্থির SQLITE_WSD কাঠামো sqlite3StatType {
  sqlite3_Stat_মান_Type now_মান[10];  /* Current value */
  sqlite3_Stat_মান_Type mx_মান[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
স্থির ধ্রুবক অক্ষর statMutex[] = {
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
#অন্যথা
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_Status_মান(পূর্ণসংখ্যা op){
  wsdStatInit;
  প্রতিজ্ঞা( op>=0 && op<Array_আকার(wsdStat.now_মান) );
  প্রতিজ্ঞা( op>=0 && op<Array_আকার(statMutex) );
  প্রতিজ্ঞা( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ফেরত wsdStat.now_মান[op];
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
শূন্য sqlite3StatusUp(পূর্ণসংখ্যা op, পূর্ণসংখ্যা N){
  wsdStatInit;
  প্রতিজ্ঞা( op>=0 && op<Array_আকার(wsdStat.now_মান) );
  প্রতিজ্ঞা( op>=0 && op<Array_আকার(statMutex) );
  প্রতিজ্ঞা( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_মান[op] += N;
  যদি( wsdStat.now_মান[op]>wsdStat.mx_মান[op] ){
    wsdStat.mx_মান[op] = wsdStat.now_মান[op];
  }
}
শূন্য sqlite3StatusDown(পূর্ণসংখ্যা op, পূর্ণসংখ্যা N){
  wsdStatInit;
  প্রতিজ্ঞা( N>=0 );
  প্রতিজ্ঞা( op>=0 && op<Array_আকার(statMutex) );
  প্রতিজ্ঞা( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  প্রতিজ্ঞা( op>=0 && op<Array_আকার(wsdStat.now_মান) );
  wsdStat.now_মান[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
শূন্য sqlite3StatusHighwater(পূর্ণসংখ্যা op, পূর্ণসংখ্যা X){
  sqlite3_Stat_মান_Type new_মান;
  wsdStatInit;
  প্রতিজ্ঞা( X>=0 );
  new_মান = (sqlite3_Stat_মান_Type)X;
  প্রতিজ্ঞা( op>=0 && op<Array_আকার(wsdStat.now_মান) );
  প্রতিজ্ঞা( op>=0 && op<Array_আকার(statMutex) );
  প্রতিজ্ঞা( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  প্রতিজ্ঞা( op==SQLITE_STATUS_MALLOC_আকার
          || op==SQLITE_STATUS_PAGECACHE_আকার
          || op==SQLITE_STATUS_PARSER_স্তূপ );
  যদি( new_মান>wsdStat.mx_মান[op] ){
    wsdStat.mx_মান[op] = new_মান;
  }
}

/*
** Query status information.
*/
পূর্ণসংখ্যা sqlite3_status64(
  পূর্ণসংখ্যা op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  পূর্ণসংখ্যা resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  যদি( op<0 || op>=Array_আকার(wsdStat.now_মান) ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCurrent==0 || pHighwater==0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_মান[op];
  *pHighwater = wsdStat.mx_মান[op];
  যদি( resetFlag ){
    wsdStat.mx_মান[op] = wsdStat.now_মান[op];
  }
  sqlite3_mutex_leave(pMutex);
  (শূন্য)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  ফেরত SQLITE_OK;
}
পূর্ণসংখ্যা sqlite3_status(পূর্ণসংখ্যা op, পূর্ণসংখ্যা *pCurrent, পূর্ণসংখ্যা *pHighwater, পূর্ণসংখ্যা resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  পূর্ণসংখ্যা rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( pCurrent==0 || pHighwater==0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  যদি( rc==0 ){
    *pCurrent = (পূর্ণসংখ্যা)iCur;
    *pHighwater = (পূর্ণসংখ্যা)iHwtr;
  }
  ফেরত rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
স্থির u32 গুনতি_Lookaside_Slots(LookasideSlot *p){
  u32 cnt = 0;
  যখন( p ){
    p = p->p_পরবর্তী;
    cnt++;
  }
  ফেরত cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
পূর্ণসংখ্যা sqlite3LookasideUsed(sqlite3 *db, পূর্ণসংখ্যা *pHighwater){
  u32 nInit = গুনতি_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = গুনতি_Lookaside_Slots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += গুনতি_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += গুনতি_Lookaside_Slots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  প্রতিজ্ঞা( db->lookaside.nSlot >= nInit+nFree );
  যদি( pHighwater ) *pHighwater = (পূর্ণসংখ্যা)(db->lookaside.nSlot - nInit);
  ফেরত (পূর্ণসংখ্যা)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
পূর্ণসংখ্যা sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  পূর্ণসংখ্যা op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  পূর্ণসংখ্যা resetFlag            /* Reset high-water mark if true */
){
  পূর্ণসংখ্যা rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  নির্বাচন( op ){
    ক্ষেত্র SQLITE_DBSTATUS_LOOKASIDE_USED: {
      পূর্ণসংখ্যা H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      যদি( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        যদি( p ){
          যখন( p->p_পরবর্তী ) p = p->p_পরবর্তী;
          p->p_পরবর্তী = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        যদি( p ){
          যখন( p->p_পরবর্তী ) p = p->p_পরবর্তী;
          p->p_পরবর্তী = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      বিরতি;
    }

    ক্ষেত্র SQLITE_DBSTATUS_LOOKASIDE_HIT:
    ক্ষেত্র SQLITE_DBSTATUS_LOOKASIDE_MISS_আকার:
    ক্ষেত্র SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_আকার );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      প্রতিজ্ঞা( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      প্রতিজ্ঞা( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      যদি( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      বিরতি;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    ক্ষেত্র SQLITE_DBSTATUS_CACHE_USED_SHARED:
    ক্ষেত্র SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 যোগ_Used = 0;
      পূর্ণসংখ্যা i;
      sqlite3BtreeEnterAll(db);
      জন্য(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        যদি( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          পূর্ণসংখ্যা nByte = sqlite3PagerMemUsed(pPager);
          যদি( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_গুনতি(pBt);
          }
          যোগ_Used += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = যোগ_Used;
      *pHighwtr = 0;
      বিরতি;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    ক্ষেত্র SQLITE_DBSTATUS_SCHEMA_USED: {
      পূর্ণসংখ্যা i;          /* Used to iterate through schemas */
      পূর্ণসংখ্যা nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      প্রতিজ্ঞা( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      জন্য(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        যদি( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(আকার(HashElem)) * (
              pSchema->tblHash.গুনতি 
            + pSchema->trigHash.গুনতি
            + pSchema->idxHash.গুনতি
            + pSchema->fkeyHash.গুনতি
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          জন্য(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqlite_Hash_পরবর্তী(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqlite_Hash_তথ্য(p));
          }
          জন্য(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqlite_Hash_পরবর্তী(p)){
            sqlite3DeleteTable(db, (Table *)sqlite_Hash_তথ্য(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      বিরতি;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    ক্ষেত্র SQLITE_DBSTATUS_STMT_USED: {
      কাঠামো Vdbe *pVdbe;         /* Used to iterate through VMs */
      পূর্ণসংখ্যা nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      প্রতিজ্ঞা( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      জন্য(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      বিরতি;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    ক্ষেত্র SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    ক্ষেত্র SQLITE_DBSTATUS_CACHE_HIT:
    ক্ষেত্র SQLITE_DBSTATUS_CACHE_MISS:
    ক্ষেত্র SQLITE_DBSTATUS_CACHE_WRITE:{
      পূর্ণসংখ্যা i;
      u64 nRet = 0;
      প্রতিজ্ঞা( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      প্রতিজ্ঞা( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      জন্য(i=0; i<db->nDb; i++){
        যদি( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      বিরতি;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    ক্ষেত্র SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      যদি( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3_Btree_Get_Page_আকার(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      যদি( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      বিরতি;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    ক্ষেত্র SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      বিরতি;
    }

    পূর্বনির্ধারিত: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ফেরত rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
পূর্ণসংখ্যা sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  পূর্ণসংখ্যা op,                  /* Status verb */
  পূর্ণসংখ্যা *pCurrent,           /* Write current value here */
  পূর্ণসংখ্যা *pHighwtr,           /* Write high-water mark here */
  পূর্ণসংখ্যা resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  পূর্ণসংখ্যা rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  যদি( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  ফেরত rc;
}
