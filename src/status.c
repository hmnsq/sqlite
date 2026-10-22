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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#എങ്കിൽ SQLITE_PTRSIZE>4
തരനിർവചനം sqlite3_int64 sqlite3_Stat_മൂല്യം_Type;
#അല്ലെങ്കിൽ
തരനിർവചനം u32 sqlite3_Stat_മൂല്യം_Type;
#endif
തരനിർവചനം ഘടന sqlite3StatType sqlite3StatType;
സ്ഥിതം SQLITE_WSD ഘടന sqlite3StatType {
  sqlite3_Stat_മൂല്യം_Type now_മൂല്യം[10];  /* Current value */
  sqlite3_Stat_മൂല്യം_Type mx_മൂല്യം[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം statMutex[] = {
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
#അല്ലെങ്കിൽ
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_Status_മൂല്യം(പൂർണ്ണസംഖ്യ op){
  wsdStatInit;
  പ്രതിജ്ഞാ( op>=0 && op<ArraySize(wsdStat.now_മൂല്യം) );
  പ്രതിജ്ഞാ( op>=0 && op<ArraySize(statMutex) );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  തിരികെ wsdStat.now_മൂല്യം[op];
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
ശൂന്യം sqlite3StatusUp(പൂർണ്ണസംഖ്യ op, പൂർണ്ണസംഖ്യ N){
  wsdStatInit;
  പ്രതിജ്ഞാ( op>=0 && op<ArraySize(wsdStat.now_മൂല്യം) );
  പ്രതിജ്ഞാ( op>=0 && op<ArraySize(statMutex) );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_മൂല്യം[op] += N;
  എങ്കിൽ( wsdStat.now_മൂല്യം[op]>wsdStat.mx_മൂല്യം[op] ){
    wsdStat.mx_മൂല്യം[op] = wsdStat.now_മൂല്യം[op];
  }
}
ശൂന്യം sqlite3StatusDown(പൂർണ്ണസംഖ്യ op, പൂർണ്ണസംഖ്യ N){
  wsdStatInit;
  പ്രതിജ്ഞാ( N>=0 );
  പ്രതിജ്ഞാ( op>=0 && op<ArraySize(statMutex) );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  പ്രതിജ്ഞാ( op>=0 && op<ArraySize(wsdStat.now_മൂല്യം) );
  wsdStat.now_മൂല്യം[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
ശൂന്യം sqlite3StatusHighwater(പൂർണ്ണസംഖ്യ op, പൂർണ്ണസംഖ്യ X){
  sqlite3_Stat_മൂല്യം_Type new_മൂല്യം;
  wsdStatInit;
  പ്രതിജ്ഞാ( X>=0 );
  new_മൂല്യം = (sqlite3_Stat_മൂല്യം_Type)X;
  പ്രതിജ്ഞാ( op>=0 && op<ArraySize(wsdStat.now_മൂല്യം) );
  പ്രതിജ്ഞാ( op>=0 && op<ArraySize(statMutex) );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  പ്രതിജ്ഞാ( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_PARSER_അട്ടി );
  എങ്കിൽ( new_മൂല്യം>wsdStat.mx_മൂല്യം[op] ){
    wsdStat.mx_മൂല്യം[op] = new_മൂല്യം;
  }
}

/*
** Query status information.
*/
പൂർണ്ണസംഖ്യ sqlite3_status64(
  പൂർണ്ണസംഖ്യ op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  പൂർണ്ണസംഖ്യ resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  എങ്കിൽ( op<0 || op>=ArraySize(wsdStat.now_മൂല്യം) ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCurrent==0 || pHighwater==0 ) തിരികെ SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_മൂല്യം[op];
  *pHighwater = wsdStat.mx_മൂല്യം[op];
  എങ്കിൽ( resetFlag ){
    wsdStat.mx_മൂല്യം[op] = wsdStat.now_മൂല്യം[op];
  }
  sqlite3_mutex_leave(pMutex);
  (ശൂന്യം)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  തിരികെ SQLITE_OK;
}
പൂർണ്ണസംഖ്യ sqlite3_status(പൂർണ്ണസംഖ്യ op, പൂർണ്ണസംഖ്യ *pCurrent, പൂർണ്ണസംഖ്യ *pHighwater, പൂർണ്ണസംഖ്യ resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  പൂർണ്ണസംഖ്യ rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( pCurrent==0 || pHighwater==0 ) തിരികെ SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  എങ്കിൽ( rc==0 ){
    *pCurrent = (പൂർണ്ണസംഖ്യ)iCur;
    *pHighwater = (പൂർണ്ണസംഖ്യ)iHwtr;
  }
  തിരികെ rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
സ്ഥിതം u32 എണ്ണം_Lookaside_Slots(LookasideSlot *p){
  u32 cnt = 0;
  ആകുമ്പോൾ( p ){
    p = p->pNext;
    cnt++;
  }
  തിരികെ cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
പൂർണ്ണസംഖ്യ sqlite3LookasideUsed(sqlite3 *db, പൂർണ്ണസംഖ്യ *pHighwater){
  u32 nInit = എണ്ണം_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = എണ്ണം_Lookaside_Slots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += എണ്ണം_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += എണ്ണം_Lookaside_Slots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  പ്രതിജ്ഞാ( db->lookaside.nSlot >= nInit+nFree );
  എങ്കിൽ( pHighwater ) *pHighwater = (പൂർണ്ണസംഖ്യ)(db->lookaside.nSlot - nInit);
  തിരികെ (പൂർണ്ണസംഖ്യ)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
പൂർണ്ണസംഖ്യ sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  പൂർണ്ണസംഖ്യ op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  പൂർണ്ണസംഖ്യ resetFlag            /* Reset high-water mark if true */
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  തിരഞ്ഞെടുപ്പ്( op ){
    അവസ്ഥ SQLITE_DBSTATUS_LOOKASIDE_USED: {
      പൂർണ്ണസംഖ്യ H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      എങ്കിൽ( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        എങ്കിൽ( p ){
          ആകുമ്പോൾ( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        എങ്കിൽ( p ){
          ആകുമ്പോൾ( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      നിർത്തുക;
    }

    അവസ്ഥ SQLITE_DBSTATUS_LOOKASIDE_HIT:
    അവസ്ഥ SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    അവസ്ഥ SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      പ്രതിജ്ഞാ( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      പ്രതിജ്ഞാ( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      എങ്കിൽ( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      നിർത്തുക;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    അവസ്ഥ SQLITE_DBSTATUS_CACHE_USED_SHARED:
    അവസ്ഥ SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      പൂർണ്ണസംഖ്യ i;
      sqlite3BtreeEnterAll(db);
      വേണ്ടി(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        എങ്കിൽ( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          പൂർണ്ണസംഖ്യ nByte = sqlite3PagerMemUsed(pPager);
          എങ്കിൽ( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_എണ്ണം(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      നിർത്തുക;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    അവസ്ഥ SQLITE_DBSTATUS_SCHEMA_USED: {
      പൂർണ്ണസംഖ്യ i;          /* Used to iterate through schemas */
      പൂർണ്ണസംഖ്യ nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      പ്രതിജ്ഞാ( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      വേണ്ടി(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        എങ്കിൽ( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(വലുപ്പം(HashElem)) * (
              pSchema->tblHash.എണ്ണം 
            + pSchema->trigHash.എണ്ണം
            + pSchema->idxHash.എണ്ണം
            + pSchema->fkeyHash.എണ്ണം
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          വേണ്ടി(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          വേണ്ടി(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      നിർത്തുക;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    അവസ്ഥ SQLITE_DBSTATUS_STMT_USED: {
      ഘടന Vdbe *pVdbe;         /* Used to iterate through VMs */
      പൂർണ്ണസംഖ്യ nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      പ്രതിജ്ഞാ( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      വേണ്ടി(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      നിർത്തുക;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    അവസ്ഥ SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    അവസ്ഥ SQLITE_DBSTATUS_CACHE_HIT:
    അവസ്ഥ SQLITE_DBSTATUS_CACHE_MISS:
    അവസ്ഥ SQLITE_DBSTATUS_CACHE_WRITE:{
      പൂർണ്ണസംഖ്യ i;
      u64 nRet = 0;
      പ്രതിജ്ഞാ( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      പ്രതിജ്ഞാ( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      വേണ്ടി(i=0; i<db->nDb; i++){
        എങ്കിൽ( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      നിർത്തുക;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    അവസ്ഥ SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      എങ്കിൽ( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      എങ്കിൽ( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      നിർത്തുക;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    അവസ്ഥ SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      നിർത്തുക;
    }

    സ്ഥിരസ്ഥിതി: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  തിരികെ rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
പൂർണ്ണസംഖ്യ sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  പൂർണ്ണസംഖ്യ op,                  /* Status verb */
  പൂർണ്ണസംഖ്യ *pCurrent,           /* Write current value here */
  പൂർണ്ണസംഖ്യ *pHighwtr,           /* Write high-water mark here */
  പൂർണ്ണസംഖ്യ resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  പൂർണ്ണസംഖ്യ rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  എങ്കിൽ( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  തിരികെ rc;
}
