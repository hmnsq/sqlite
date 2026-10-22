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
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#𐤀𐤌 SQLITE_PTRSIZE>4
تعريفنوع sqlite3_int64 sqlite3StatValueType;
#وإلا
تعريفنوع u32 sqlite3StatValueType;
#endif
تعريفنوع بنية sqlite3StatType sqlite3StatType;
ساكن SQLITE_WSD بنية sqlite3StatType {
  sqlite3StatValueType nowValue[10];  /* Current value */
  sqlite3StatValueType mxValue[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
ساكن ثابت حرف statMutex[] = {
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
#وإلا
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3StatusValue(صحيح op){
  wsdStatInit;
  أكد( op>=0 && op<ArraySize(wsdStat.nowValue) );
  أكد( op>=0 && op<ArraySize(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أرجع wsdStat.nowValue[op];
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
فراغ sqlite3StatusUp(صحيح op, صحيح N){
  wsdStatInit;
  أكد( op>=0 && op<ArraySize(wsdStat.nowValue) );
  أكد( op>=0 && op<ArraySize(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.nowValue[op] += N;
  𐤀𐤌( wsdStat.nowValue[op]>wsdStat.mxValue[op] ){
    wsdStat.mxValue[op] = wsdStat.nowValue[op];
  }
}
فراغ sqlite3StatusDown(صحيح op, صحيح N){
  wsdStatInit;
  أكد( N>=0 );
  أكد( op>=0 && op<ArraySize(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أكد( op>=0 && op<ArraySize(wsdStat.nowValue) );
  wsdStat.nowValue[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
فراغ sqlite3StatusHighwater(صحيح op, صحيح X){
  sqlite3StatValueType newValue;
  wsdStatInit;
  أكد( X>=0 );
  newValue = (sqlite3StatValueType)X;
  أكد( op>=0 && op<ArraySize(wsdStat.nowValue) );
  أكد( op>=0 && op<ArraySize(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أكد( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_PARSER_STACK );
  𐤀𐤌( newValue>wsdStat.mxValue[op] ){
    wsdStat.mxValue[op] = newValue;
  }
}

/*
** Query status information.
*/
صحيح sqlite3_status64(
  صحيح op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  صحيح resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  𐤀𐤌( op<0 || op>=ArraySize(wsdStat.nowValue) ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  𐤀𐤌( pCurrent==0 || pHighwater==0 ) أرجع SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.nowValue[op];
  *pHighwater = wsdStat.mxValue[op];
  𐤀𐤌( resetFlag ){
    wsdStat.mxValue[op] = wsdStat.nowValue[op];
  }
  sqlite3_mutex_leave(pMutex);
  (فراغ)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  أرجع SQLITE_OK;
}
صحيح sqlite3_status(صحيح op, صحيح *pCurrent, صحيح *pHighwater, صحيح resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  صحيح rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  𐤀𐤌( pCurrent==0 || pHighwater==0 ) أرجع SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  𐤀𐤌( rc==0 ){
    *pCurrent = (صحيح)iCur;
    *pHighwater = (صحيح)iHwtr;
  }
  أرجع rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
ساكن u32 countLookasideSlots(LookasideSlot *p){
  u32 cnt = 0;
  طالما( p ){
    p = p->pNext;
    cnt++;
  }
  أرجع cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
صحيح sqlite3LookasideUsed(sqlite3 *db, صحيح *pHighwater){
  u32 nInit = countLookasideSlots(db->lookaside.pInit);
  u32 nFree = countLookasideSlots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += countLookasideSlots(db->lookaside.pSmallInit);
  nFree += countLookasideSlots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  أكد( db->lookaside.nSlot >= nInit+nFree );
  𐤀𐤌( pHighwater ) *pHighwater = (صحيح)(db->lookaside.nSlot - nInit);
  أرجع (صحيح)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
صحيح sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  صحيح op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  صحيح resetFlag            /* Reset high-water mark if true */
){
  صحيح rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  𐤀𐤌( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  بدل( op ){
    حالة SQLITE_DBSTATUS_LOOKASIDE_USED: {
      صحيح H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      𐤀𐤌( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        𐤀𐤌( p ){
          طالما( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        𐤀𐤌( p ){
          طالما( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      اكسر;
    }

    حالة SQLITE_DBSTATUS_LOOKASIDE_HIT:
    حالة SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    حالة SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      أكد( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      أكد( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      𐤀𐤌( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      اكسر;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    حالة SQLITE_DBSTATUS_CACHE_USED_SHARED:
    حالة SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      صحيح i;
      sqlite3BtreeEnterAll(db);
      لكل(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        𐤀𐤌( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          صحيح nByte = sqlite3PagerMemUsed(pPager);
          𐤀𐤌( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3BtreeConnectionCount(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      اكسر;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    حالة SQLITE_DBSTATUS_SCHEMA_USED: {
      صحيح i;          /* Used to iterate through schemas */
      صحيح nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      أكد( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      لكل(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        𐤀𐤌( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(حجم(HashElem)) * (
              pSchema->tblHash.count 
            + pSchema->trigHash.count
            + pSchema->idxHash.count
            + pSchema->fkeyHash.count
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          لكل(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          لكل(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      اكسر;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    حالة SQLITE_DBSTATUS_STMT_USED: {
      بنية Vdbe *pVdbe;         /* Used to iterate through VMs */
      صحيح nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      أكد( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      لكل(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      اكسر;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    حالة SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    حالة SQLITE_DBSTATUS_CACHE_HIT:
    حالة SQLITE_DBSTATUS_CACHE_MISS:
    حالة SQLITE_DBSTATUS_CACHE_WRITE:{
      صحيح i;
      u64 nRet = 0;
      أكد( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      أكد( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      لكل(i=0; i<db->nDb; i++){
        𐤀𐤌( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      اكسر;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    حالة SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      𐤀𐤌( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      𐤀𐤌( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      اكسر;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    حالة SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      اكسر;
    }

    افتراضي: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
صحيح sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  صحيح op,                  /* Status verb */
  صحيح *pCurrent,           /* Write current value here */
  صحيح *pHighwtr,           /* Write high-water mark here */
  صحيح resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  صحيح rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  𐤀𐤌( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  𐤀𐤌( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  أرجع rc;
}
