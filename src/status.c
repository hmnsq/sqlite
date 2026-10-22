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
#شامل "sqliteInt.h"
#شامل "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#اگر SQLITE_PTRSIZE>4
تعریفنوع sqlite3_int64 sqlite3_Stat_مقدار_Type;
#وگرنه
تعریفنوع u32 sqlite3_Stat_مقدار_Type;
#endif
تعریفنوع ساختار sqlite3StatType sqlite3StatType;
ایستا SQLITE_WSD ساختار sqlite3StatType {
  sqlite3_Stat_مقدار_Type now_مقدار[10];  /* Current value */
  sqlite3_Stat_مقدار_Type mx_مقدار[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
ایستا ثابت نویسه statMutex[] = {
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
#وگرنه
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_Status_مقدار(صحیح op){
  wsdStatInit;
  أكد( op>=0 && op<ArraySize(wsdStat.now_مقدار) );
  أكد( op>=0 && op<ArraySize(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  بازگشت wsdStat.now_مقدار[op];
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
تهی sqlite3StatusUp(صحیح op, صحیح N){
  wsdStatInit;
  أكد( op>=0 && op<ArraySize(wsdStat.now_مقدار) );
  أكد( op>=0 && op<ArraySize(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_مقدار[op] += N;
  اگر( wsdStat.now_مقدار[op]>wsdStat.mx_مقدار[op] ){
    wsdStat.mx_مقدار[op] = wsdStat.now_مقدار[op];
  }
}
تهی sqlite3StatusDown(صحیح op, صحیح N){
  wsdStatInit;
  أكد( N>=0 );
  أكد( op>=0 && op<ArraySize(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أكد( op>=0 && op<ArraySize(wsdStat.now_مقدار) );
  wsdStat.now_مقدار[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
تهی sqlite3StatusHighwater(صحیح op, صحیح X){
  sqlite3_Stat_مقدار_Type new_مقدار;
  wsdStatInit;
  أكد( X>=0 );
  new_مقدار = (sqlite3_Stat_مقدار_Type)X;
  أكد( op>=0 && op<ArraySize(wsdStat.now_مقدار) );
  أكد( op>=0 && op<ArraySize(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أكد( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_PARSER_پشته );
  اگر( new_مقدار>wsdStat.mx_مقدار[op] ){
    wsdStat.mx_مقدار[op] = new_مقدار;
  }
}

/*
** Query status information.
*/
صحیح sqlite3_status64(
  صحیح op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  صحیح resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  اگر( op<0 || op>=ArraySize(wsdStat.now_مقدار) ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCurrent==0 || pHighwater==0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_مقدار[op];
  *pHighwater = wsdStat.mx_مقدار[op];
  اگر( resetFlag ){
    wsdStat.mx_مقدار[op] = wsdStat.now_مقدار[op];
  }
  sqlite3_mutex_leave(pMutex);
  (تهی)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  بازگشت SQLITE_OK;
}
صحیح sqlite3_status(صحیح op, صحیح *pCurrent, صحیح *pHighwater, صحیح resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  صحیح rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( pCurrent==0 || pHighwater==0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  اگر( rc==0 ){
    *pCurrent = (صحیح)iCur;
    *pHighwater = (صحیح)iHwtr;
  }
  بازگشت rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
ایستا u32 شمار_Lookaside_Slots(LookasideSlot *p){
  u32 cnt = 0;
  وقتی( p ){
    p = p->pNext;
    cnt++;
  }
  بازگشت cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
صحیح sqlite3LookasideUsed(sqlite3 *db, صحیح *pHighwater){
  u32 nInit = شمار_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = شمار_Lookaside_Slots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += شمار_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += شمار_Lookaside_Slots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  أكد( db->lookaside.nSlot >= nInit+nFree );
  اگر( pHighwater ) *pHighwater = (صحیح)(db->lookaside.nSlot - nInit);
  بازگشت (صحیح)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
صحیح sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  صحیح op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  صحیح resetFlag            /* Reset high-water mark if true */
){
  صحیح rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  انتخاب( op ){
    حالت SQLITE_DBSTATUS_LOOKASIDE_USED: {
      صحیح H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      اگر( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        اگر( p ){
          وقتی( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        اگر( p ){
          وقتی( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      بشکن;
    }

    حالت SQLITE_DBSTATUS_LOOKASIDE_HIT:
    حالت SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    حالت SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      أكد( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      أكد( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      اگر( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      بشکن;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    حالت SQLITE_DBSTATUS_CACHE_USED_SHARED:
    حالت SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      صحیح i;
      sqlite3BtreeEnterAll(db);
      برای(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        اگر( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          صحیح nByte = sqlite3PagerMemUsed(pPager);
          اگر( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_شمار(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      بشکن;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    حالت SQLITE_DBSTATUS_SCHEMA_USED: {
      صحیح i;          /* Used to iterate through schemas */
      صحیح nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      أكد( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      برای(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        اگر( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(اندازه(HashElem)) * (
              pSchema->tblHash.شمار 
            + pSchema->trigHash.شمار
            + pSchema->idxHash.شمار
            + pSchema->fkeyHash.شمار
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          برای(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          برای(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      بشکن;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    حالت SQLITE_DBSTATUS_STMT_USED: {
      ساختار Vdbe *pVdbe;         /* Used to iterate through VMs */
      صحیح nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      أكد( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      برای(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      بشکن;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    حالت SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    حالت SQLITE_DBSTATUS_CACHE_HIT:
    حالت SQLITE_DBSTATUS_CACHE_MISS:
    حالت SQLITE_DBSTATUS_CACHE_WRITE:{
      صحیح i;
      u64 nRet = 0;
      أكد( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      أكد( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      برای(i=0; i<db->nDb; i++){
        اگر( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      بشکن;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    حالت SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      اگر( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      اگر( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      بشکن;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    حالت SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      بشکن;
    }

    پیشفرض: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  بازگشت rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
صحیح sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  صحیح op,                  /* Status verb */
  صحیح *pCurrent,           /* Write current value here */
  صحیح *pHighwtr,           /* Write high-water mark here */
  صحیح resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  صحیح rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  اگر( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  بازگشت rc;
}
