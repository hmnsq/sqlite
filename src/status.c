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
#include "sqliteInt.h"
#include "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#якщо SQLITE_PTRSIZE>4
typedef sqlite3_int64 sqlite3StatValueType;
#інакше
typedef u32 sqlite3StatValueType;
#endif
typedef struct sqlite3StatType sqlite3StatType;
static SQLITE_WSD struct sqlite3StatType {
  sqlite3StatValueType nowValue[10];  /* Current value */
  sqlite3StatValueType mxValue[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
static const char statMutex[] = {
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
#інакше
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3StatusValue(ціле op){
  wsdStatInit;
  assert( op>=0 && op<ArraySize(wsdStat.nowValue) );
  assert( op>=0 && op<ArraySize(statMutex) );
  assert( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  повернути wsdStat.nowValue[op];
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
void sqlite3StatusUp(ціле op, ціле N){
  wsdStatInit;
  assert( op>=0 && op<ArraySize(wsdStat.nowValue) );
  assert( op>=0 && op<ArraySize(statMutex) );
  assert( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.nowValue[op] += N;
  якщо( wsdStat.nowValue[op]>wsdStat.mxValue[op] ){
    wsdStat.mxValue[op] = wsdStat.nowValue[op];
  }
}
void sqlite3StatusDown(ціле op, ціле N){
  wsdStatInit;
  assert( N>=0 );
  assert( op>=0 && op<ArraySize(statMutex) );
  assert( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  assert( op>=0 && op<ArraySize(wsdStat.nowValue) );
  wsdStat.nowValue[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
void sqlite3StatusHighwater(ціле op, ціле X){
  sqlite3StatValueType newValue;
  wsdStatInit;
  assert( X>=0 );
  newValue = (sqlite3StatValueType)X;
  assert( op>=0 && op<ArraySize(wsdStat.nowValue) );
  assert( op>=0 && op<ArraySize(statMutex) );
  assert( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  assert( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_PARSER_STACK );
  якщо( newValue>wsdStat.mxValue[op] ){
    wsdStat.mxValue[op] = newValue;
  }
}

/*
** Query status information.
*/
ціле sqlite3_status64(
  ціле op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  ціле resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  якщо( op<0 || op>=ArraySize(wsdStat.nowValue) ){
    повернути SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( pCurrent==0 || pHighwater==0 ) повернути SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.nowValue[op];
  *pHighwater = wsdStat.mxValue[op];
  якщо( resetFlag ){
    wsdStat.mxValue[op] = wsdStat.nowValue[op];
  }
  sqlite3_mutex_leave(pMutex);
  (void)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  повернути SQLITE_OK;
}
ціле sqlite3_status(ціле op, ціле *pCurrent, ціле *pHighwater, ціле resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  ціле rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( pCurrent==0 || pHighwater==0 ) повернути SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  якщо( rc==0 ){
    *pCurrent = (ціле)iCur;
    *pHighwater = (ціле)iHwtr;
  }
  повернути rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
static u32 countLookasideSlots(LookasideSlot *p){
  u32 cnt = 0;
  поки( p ){
    p = p->pNext;
    cnt++;
  }
  повернути cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
ціле sqlite3LookasideUsed(sqlite3 *db, ціле *pHighwater){
  u32 nInit = countLookasideSlots(db->lookaside.pInit);
  u32 nFree = countLookasideSlots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += countLookasideSlots(db->lookaside.pSmallInit);
  nFree += countLookasideSlots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  assert( db->lookaside.nSlot >= nInit+nFree );
  якщо( pHighwater ) *pHighwater = (ціле)(db->lookaside.nSlot - nInit);
  повернути (ціле)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
ціле sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  ціле op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  ціле resetFlag            /* Reset high-water mark if true */
){
  ціле rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    повернути SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  switch( op ){
    case SQLITE_DBSTATUS_LOOKASIDE_USED: {
      ціле H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      якщо( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        якщо( p ){
          поки( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        якщо( p ){
          поки( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      break;
    }

    case SQLITE_DBSTATUS_LOOKASIDE_HIT:
    case SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    case SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      assert( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      assert( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      якщо( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      break;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    case SQLITE_DBSTATUS_CACHE_USED_SHARED:
    case SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      ціле i;
      sqlite3BtreeEnterAll(db);
      для(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        якщо( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          ціле nByte = sqlite3PagerMemUsed(pPager);
          якщо( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3BtreeConnectionCount(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      break;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    case SQLITE_DBSTATUS_SCHEMA_USED: {
      ціле i;          /* Used to iterate through schemas */
      ціле nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      assert( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      для(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        якщо( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(sizeof(HashElem)) * (
              pSchema->tblHash.count 
            + pSchema->trigHash.count
            + pSchema->idxHash.count
            + pSchema->fkeyHash.count
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          для(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          для(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      break;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    case SQLITE_DBSTATUS_STMT_USED: {
      struct Vdbe *pVdbe;         /* Used to iterate through VMs */
      ціле nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      assert( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      для(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      break;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    case SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    case SQLITE_DBSTATUS_CACHE_HIT:
    case SQLITE_DBSTATUS_CACHE_MISS:
    case SQLITE_DBSTATUS_CACHE_WRITE:{
      ціле i;
      u64 nRet = 0;
      assert( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      assert( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      для(i=0; i<db->nDb; i++){
        якщо( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      break;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    case SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      якщо( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      якщо( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      break;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    case SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      break;
    }

    default: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  повернути rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
ціле sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  ціле op,                  /* Status verb */
  ціле *pCurrent,           /* Write current value here */
  ціле *pHighwtr,           /* Write high-water mark here */
  ціле resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  ціле rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    повернути SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  якщо( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  повернути rc;
}
