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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#எனில் SQLITE_PTRSIZE>4
வகைவரை sqlite3_int64 sqlite3_Stat_மதிப்பு_Type;
#இல்லையெனில்
வகைவரை u32 sqlite3_Stat_மதிப்பு_Type;
#endif
வகைவரை கட்டமைப்பு sqlite3StatType sqlite3StatType;
நிலையான SQLITE_WSD கட்டமைப்பு sqlite3StatType {
  sqlite3_Stat_மதிப்பு_Type now_மதிப்பு[10];  /* Current value */
  sqlite3_Stat_மதிப்பு_Type mx_மதிப்பு[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
நிலையான மாறிலி எழுத்து statMutex[] = {
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
#இல்லையெனில்
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_Status_மதிப்பு(முழு op){
  wsdStatInit;
  ப்ரதிஜ்ஞா( op>=0 && op<ArraySize(wsdStat.now_மதிப்பு) );
  ப்ரதிஜ்ஞா( op>=0 && op<ArraySize(statMutex) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  திரும்பு wsdStat.now_மதிப்பு[op];
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
வெறுமை sqlite3StatusUp(முழு op, முழு N){
  wsdStatInit;
  ப்ரதிஜ்ஞா( op>=0 && op<ArraySize(wsdStat.now_மதிப்பு) );
  ப்ரதிஜ்ஞா( op>=0 && op<ArraySize(statMutex) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_மதிப்பு[op] += N;
  எனில்( wsdStat.now_மதிப்பு[op]>wsdStat.mx_மதிப்பு[op] ){
    wsdStat.mx_மதிப்பு[op] = wsdStat.now_மதிப்பு[op];
  }
}
வெறுமை sqlite3StatusDown(முழு op, முழு N){
  wsdStatInit;
  ப்ரதிஜ்ஞா( N>=0 );
  ப்ரதிஜ்ஞா( op>=0 && op<ArraySize(statMutex) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ப்ரதிஜ்ஞா( op>=0 && op<ArraySize(wsdStat.now_மதிப்பு) );
  wsdStat.now_மதிப்பு[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
வெறுமை sqlite3StatusHighwater(முழு op, முழு X){
  sqlite3_Stat_மதிப்பு_Type new_மதிப்பு;
  wsdStatInit;
  ப்ரதிஜ்ஞா( X>=0 );
  new_மதிப்பு = (sqlite3_Stat_மதிப்பு_Type)X;
  ப்ரதிஜ்ஞா( op>=0 && op<ArraySize(wsdStat.now_மதிப்பு) );
  ப்ரதிஜ்ஞா( op>=0 && op<ArraySize(statMutex) );
  ப்ரதிஜ்ஞா( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ப்ரதிஜ்ஞா( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_PARSER_அடுக்கு );
  எனில்( new_மதிப்பு>wsdStat.mx_மதிப்பு[op] ){
    wsdStat.mx_மதிப்பு[op] = new_மதிப்பு;
  }
}

/*
** Query status information.
*/
முழு sqlite3_status64(
  முழு op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  முழு resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  எனில்( op<0 || op>=ArraySize(wsdStat.now_மதிப்பு) ){
    திரும்பு SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( pCurrent==0 || pHighwater==0 ) திரும்பு SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_மதிப்பு[op];
  *pHighwater = wsdStat.mx_மதிப்பு[op];
  எனில்( resetFlag ){
    wsdStat.mx_மதிப்பு[op] = wsdStat.now_மதிப்பு[op];
  }
  sqlite3_mutex_leave(pMutex);
  (வெறுமை)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  திரும்பு SQLITE_OK;
}
முழு sqlite3_status(முழு op, முழு *pCurrent, முழு *pHighwater, முழு resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  முழு rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( pCurrent==0 || pHighwater==0 ) திரும்பு SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  எனில்( rc==0 ){
    *pCurrent = (முழு)iCur;
    *pHighwater = (முழு)iHwtr;
  }
  திரும்பு rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
நிலையான u32 மொத்தம்_Lookaside_Slots(LookasideSlot *p){
  u32 cnt = 0;
  போது( p ){
    p = p->pNext;
    cnt++;
  }
  திரும்பு cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
முழு sqlite3LookasideUsed(sqlite3 *db, முழு *pHighwater){
  u32 nInit = மொத்தம்_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = மொத்தம்_Lookaside_Slots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += மொத்தம்_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += மொத்தம்_Lookaside_Slots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ப்ரதிஜ்ஞா( db->lookaside.nSlot >= nInit+nFree );
  எனில்( pHighwater ) *pHighwater = (முழு)(db->lookaside.nSlot - nInit);
  திரும்பு (முழு)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
முழு sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  முழு op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  முழு resetFlag            /* Reset high-water mark if true */
){
  முழு rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    திரும்பு SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  தேர்வு( op ){
    நிலை SQLITE_DBSTATUS_LOOKASIDE_USED: {
      முழு H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      எனில்( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        எனில்( p ){
          போது( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        எனில்( p ){
          போது( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      நிறுத்து;
    }

    நிலை SQLITE_DBSTATUS_LOOKASIDE_HIT:
    நிலை SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    நிலை SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      ப்ரதிஜ்ஞா( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      ப்ரதிஜ்ஞா( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      எனில்( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      நிறுத்து;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    நிலை SQLITE_DBSTATUS_CACHE_USED_SHARED:
    நிலை SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      முழு i;
      sqlite3BtreeEnterAll(db);
      க்கு(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        எனில்( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          முழு nByte = sqlite3PagerMemUsed(pPager);
          எனில்( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_மொத்தம்(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      நிறுத்து;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    நிலை SQLITE_DBSTATUS_SCHEMA_USED: {
      முழு i;          /* Used to iterate through schemas */
      முழு nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      ப்ரதிஜ்ஞா( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      க்கு(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        எனில்( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(அளவு(HashElem)) * (
              pSchema->tblHash.மொத்தம் 
            + pSchema->trigHash.மொத்தம்
            + pSchema->idxHash.மொத்தம்
            + pSchema->fkeyHash.மொத்தம்
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          க்கு(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          க்கு(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      நிறுத்து;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    நிலை SQLITE_DBSTATUS_STMT_USED: {
      கட்டமைப்பு Vdbe *pVdbe;         /* Used to iterate through VMs */
      முழு nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      ப்ரதிஜ்ஞா( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      க்கு(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      நிறுத்து;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    நிலை SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    நிலை SQLITE_DBSTATUS_CACHE_HIT:
    நிலை SQLITE_DBSTATUS_CACHE_MISS:
    நிலை SQLITE_DBSTATUS_CACHE_WRITE:{
      முழு i;
      u64 nRet = 0;
      ப்ரதிஜ்ஞா( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      ப்ரதிஜ்ஞா( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      க்கு(i=0; i<db->nDb; i++){
        எனில்( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      நிறுத்து;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    நிலை SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      எனில்( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      எனில்( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      நிறுத்து;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    நிலை SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      நிறுத்து;
    }

    இயல்பு: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
முழு sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  முழு op,                  /* Status verb */
  முழு *pCurrent,           /* Write current value here */
  முழு *pHighwtr,           /* Write high-water mark here */
  முழு resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  முழு rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    திரும்பு SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  எனில்( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  திரும்பு rc;
}
