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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#ಯದಿ SQLITE_PTRSIZE>4
ಪ್ರಕಾರನಿರ್ದೇಶ sqlite3_int64 sqlite3_Stat_ಮೌಲ್ಯ_Type;
#ಅನ್ಯಥಾ
ಪ್ರಕಾರನಿರ್ದೇಶ u32 sqlite3_Stat_ಮೌಲ್ಯ_Type;
#endif
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ sqlite3StatType sqlite3StatType;
ಸ್ಥಿರ SQLITE_WSD ಸಂರಚನಾ sqlite3StatType {
  sqlite3_Stat_ಮೌಲ್ಯ_Type now_ಮೌಲ್ಯ[10];  /* Current value */
  sqlite3_Stat_ಮೌಲ್ಯ_Type mx_ಮೌಲ್ಯ[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ statMutex[] = {
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
#ಅನ್ಯಥಾ
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_Status_ಮೌಲ್ಯ(ಪೂರ್ಣಾಙ್ಕ op){
  wsdStatInit;
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<ArraySize(wsdStat.now_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<ArraySize(statMutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ಪ್ರತ್ಯಾಗಮ wsdStat.now_ಮೌಲ್ಯ[op];
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
ಶೂನ್ಯ sqlite3StatusUp(ಪೂರ್ಣಾಙ್ಕ op, ಪೂರ್ಣಾಙ್ಕ N){
  wsdStatInit;
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<ArraySize(wsdStat.now_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<ArraySize(statMutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_ಮೌಲ್ಯ[op] += N;
  ಯದಿ( wsdStat.now_ಮೌಲ್ಯ[op]>wsdStat.mx_ಮೌಲ್ಯ[op] ){
    wsdStat.mx_ಮೌಲ್ಯ[op] = wsdStat.now_ಮೌಲ್ಯ[op];
  }
}
ಶೂನ್ಯ sqlite3StatusDown(ಪೂರ್ಣಾಙ್ಕ op, ಪೂರ್ಣಾಙ್ಕ N){
  wsdStatInit;
  ಪ್ರತಿಜ್ಞಾ( N>=0 );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<ArraySize(statMutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<ArraySize(wsdStat.now_ಮೌಲ್ಯ) );
  wsdStat.now_ಮೌಲ್ಯ[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
ಶೂನ್ಯ sqlite3StatusHighwater(ಪೂರ್ಣಾಙ್ಕ op, ಪೂರ್ಣಾಙ್ಕ X){
  sqlite3_Stat_ಮೌಲ್ಯ_Type new_ಮೌಲ್ಯ;
  wsdStatInit;
  ಪ್ರತಿಜ್ಞಾ( X>=0 );
  new_ಮೌಲ್ಯ = (sqlite3_Stat_ಮೌಲ್ಯ_Type)X;
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<ArraySize(wsdStat.now_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<ArraySize(statMutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ಪ್ರತಿಜ್ಞಾ( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_PARSER_ರಾಶಿ );
  ಯದಿ( new_ಮೌಲ್ಯ>wsdStat.mx_ಮೌಲ್ಯ[op] ){
    wsdStat.mx_ಮೌಲ್ಯ[op] = new_ಮೌಲ್ಯ;
  }
}

/*
** Query status information.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_status64(
  ಪೂರ್ಣಾಙ್ಕ op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  ಪೂರ್ಣಾಙ್ಕ resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  ಯದಿ( op<0 || op>=ArraySize(wsdStat.now_ಮೌಲ್ಯ) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCurrent==0 || pHighwater==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_ಮೌಲ್ಯ[op];
  *pHighwater = wsdStat.mx_ಮೌಲ್ಯ[op];
  ಯದಿ( resetFlag ){
    wsdStat.mx_ಮೌಲ್ಯ[op] = wsdStat.now_ಮೌಲ್ಯ[op];
  }
  sqlite3_mutex_leave(pMutex);
  (ಶೂನ್ಯ)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_status(ಪೂರ್ಣಾಙ್ಕ op, ಪೂರ್ಣಾಙ್ಕ *pCurrent, ಪೂರ್ಣಾಙ್ಕ *pHighwater, ಪೂರ್ಣಾಙ್ಕ resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( pCurrent==0 || pHighwater==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  ಯದಿ( rc==0 ){
    *pCurrent = (ಪೂರ್ಣಾಙ್ಕ)iCur;
    *pHighwater = (ಪೂರ್ಣಾಙ್ಕ)iHwtr;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
ಸ್ಥಿರ u32 ಎಣಿಕೆ_Lookaside_Slots(LookasideSlot *p){
  u32 cnt = 0;
  ಯಾವತ್( p ){
    p = p->pNext;
    cnt++;
  }
  ಪ್ರತ್ಯಾಗಮ cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3LookasideUsed(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ *pHighwater){
  u32 nInit = ಎಣಿಕೆ_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = ಎಣಿಕೆ_Lookaside_Slots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += ಎಣಿಕೆ_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += ಎಣಿಕೆ_Lookaside_Slots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ಪ್ರತಿಜ್ಞಾ( db->lookaside.nSlot >= nInit+nFree );
  ಯದಿ( pHighwater ) *pHighwater = (ಪೂರ್ಣಾಙ್ಕ)(db->lookaside.nSlot - nInit);
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  ಪೂರ್ಣಾಙ್ಕ op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  ಪೂರ್ಣಾಙ್ಕ resetFlag            /* Reset high-water mark if true */
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  ವಿಕಲ್ಪ( op ){
    ಪಕ್ಷ SQLITE_DBSTATUS_LOOKASIDE_USED: {
      ಪೂರ್ಣಾಙ್ಕ H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      ಯದಿ( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        ಯದಿ( p ){
          ಯಾವತ್( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        ಯದಿ( p ){
          ಯಾವತ್( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ SQLITE_DBSTATUS_LOOKASIDE_HIT:
    ಪಕ್ಷ SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    ಪಕ್ಷ SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      ಪ್ರತಿಜ್ಞಾ( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      ಪ್ರತಿಜ್ಞಾ( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      ಯದಿ( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      ಭಙ್ಗ;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    ಪಕ್ಷ SQLITE_DBSTATUS_CACHE_USED_SHARED:
    ಪಕ್ಷ SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      ಪೂರ್ಣಾಙ್ಕ i;
      sqlite3BtreeEnterAll(db);
      ಪ್ರತಿ(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        ಯದಿ( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          ಪೂರ್ಣಾಙ್ಕ nByte = sqlite3PagerMemUsed(pPager);
          ಯದಿ( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_ಎಣಿಕೆ(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      ಭಙ್ಗ;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    ಪಕ್ಷ SQLITE_DBSTATUS_SCHEMA_USED: {
      ಪೂರ್ಣಾಙ್ಕ i;          /* Used to iterate through schemas */
      ಪೂರ್ಣಾಙ್ಕ nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      ಪ್ರತಿಜ್ಞಾ( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      ಪ್ರತಿ(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        ಯದಿ( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(ಪರಿಮಾಣ(HashElem)) * (
              pSchema->tblHash.ಎಣಿಕೆ 
            + pSchema->trigHash.ಎಣಿಕೆ
            + pSchema->idxHash.ಎಣಿಕೆ
            + pSchema->fkeyHash.ಎಣಿಕೆ
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          ಪ್ರತಿ(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          ಪ್ರತಿ(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      ಭಙ್ಗ;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    ಪಕ್ಷ SQLITE_DBSTATUS_STMT_USED: {
      ಸಂರಚನಾ Vdbe *pVdbe;         /* Used to iterate through VMs */
      ಪೂರ್ಣಾಙ್ಕ nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      ಪ್ರತಿಜ್ಞಾ( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      ಪ್ರತಿ(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      ಭಙ್ಗ;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    ಪಕ್ಷ SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    ಪಕ್ಷ SQLITE_DBSTATUS_CACHE_HIT:
    ಪಕ್ಷ SQLITE_DBSTATUS_CACHE_MISS:
    ಪಕ್ಷ SQLITE_DBSTATUS_CACHE_WRITE:{
      ಪೂರ್ಣಾಙ್ಕ i;
      u64 nRet = 0;
      ಪ್ರತಿಜ್ಞಾ( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      ಪ್ರತಿಜ್ಞಾ( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      ಪ್ರತಿ(i=0; i<db->nDb; i++){
        ಯದಿ( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      ಭಙ್ಗ;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    ಪಕ್ಷ SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      ಯದಿ( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      ಯದಿ( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      ಭಙ್ಗ;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    ಪಕ್ಷ SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      ಭಙ್ಗ;
    }

    ಸ್ವಭಾವ: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  ಪೂರ್ಣಾಙ್ಕ op,                  /* Status verb */
  ಪೂರ್ಣಾಙ್ಕ *pCurrent,           /* Write current value here */
  ಪೂರ್ಣಾಙ್ಕ *pHighwtr,           /* Write high-water mark here */
  ಪೂರ್ಣಾಙ್ಕ resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  ಯದಿ( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
