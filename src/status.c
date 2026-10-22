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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#ਜੇ SQLITE_PTRSIZE>4
ਕਿਸਮਪਰਿਭਾਸ਼ਾ sqlite3_int64 sqlite3_Stat_ਮੁੱਲ_Type;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ u32 sqlite3_Stat_ਮੁੱਲ_Type;
#ਜੇ_ਅੰਤ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ sqlite3StatType sqlite3StatType;
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ sqlite3StatType {
  sqlite3_Stat_ਮੁੱਲ_Type now_ਮੁੱਲ[10];  /* Current value */
  sqlite3_Stat_ਮੁੱਲ_Type mx_ਮੁੱਲ[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ statMutex[] = {
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
# define wsdStatInit  sqlite3StatType *x = &GLOBAL(sqlite3StatType,sqlite3Stat)
# define wsdStat x[0]
#ਨਹੀਂਤਾਂ
# define wsdStatInit
# define wsdStat sqlite3Stat
#ਜੇ_ਅੰਤ

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_ਸਥਿਤੀ_ਮੁੱਲ(ਪੂਰਨ op){
  wsdStatInit;
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<Array_ਮਾਪ(wsdStat.now_ਮੁੱਲ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<Array_ਮਾਪ(statMutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ਵਾਪਸ wsdStat.now_ਮੁੱਲ[op];
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
ਖਾਲੀ sqlite3_ਸਥਿਤੀ_ਉੱਪਰ(ਪੂਰਨ op, ਪੂਰਨ N){
  wsdStatInit;
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<Array_ਮਾਪ(wsdStat.now_ਮੁੱਲ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<Array_ਮਾਪ(statMutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_ਮੁੱਲ[op] += N;
  ਜੇ( wsdStat.now_ਮੁੱਲ[op]>wsdStat.mx_ਮੁੱਲ[op] ){
    wsdStat.mx_ਮੁੱਲ[op] = wsdStat.now_ਮੁੱਲ[op];
  }
}
ਖਾਲੀ sqlite3_ਸਥਿਤੀ_ਥੱਲੇ(ਪੂਰਨ op, ਪੂਰਨ N){
  wsdStatInit;
  ਪ੍ਰਤਿਜ੍ਞਾ( N>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<Array_ਮਾਪ(statMutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<Array_ਮਾਪ(wsdStat.now_ਮੁੱਲ) );
  wsdStat.now_ਮੁੱਲ[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
ਖਾਲੀ sqlite3_ਸਥਿਤੀ_Highwater(ਪੂਰਨ op, ਪੂਰਨ X){
  sqlite3_Stat_ਮੁੱਲ_Type new_ਮੁੱਲ;
  wsdStatInit;
  ਪ੍ਰਤਿਜ੍ਞਾ( X>=0 );
  new_ਮੁੱਲ = (sqlite3_Stat_ਮੁੱਲ_Type)X;
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<Array_ਮਾਪ(wsdStat.now_ਮੁੱਲ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( op>=0 && op<Array_ਮਾਪ(statMutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ਪ੍ਰਤਿਜ੍ਞਾ( op==SQLITE_ਸਥਿਤੀ_MALLOC_ਮਾਪ
          || op==SQLITE_ਸਥਿਤੀ_PAGECACHE_ਮਾਪ
          || op==SQLITE_ਸਥਿਤੀ_PARSER_ਢੇਰ );
  ਜੇ( new_ਮੁੱਲ>wsdStat.mx_ਮੁੱਲ[op] ){
    wsdStat.mx_ਮੁੱਲ[op] = new_ਮੁੱਲ;
  }
}

/*
** Query status information.
*/
ਪੂਰਨ sqlite3_status64(
  ਪੂਰਨ op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  ਪੂਰਨ resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  ਜੇ( op<0 || op>=Array_ਮਾਪ(wsdStat.now_ਮੁੱਲ) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCurrent==0 || pHighwater==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_ਮੁੱਲ[op];
  *pHighwater = wsdStat.mx_ਮੁੱਲ[op];
  ਜੇ( resetFlag ){
    wsdStat.mx_ਮੁੱਲ[op] = wsdStat.now_ਮੁੱਲ[op];
  }
  sqlite3_mutex_leave(pMutex);
  (ਖਾਲੀ)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  ਵਾਪਸ SQLITE_OK;
}
ਪੂਰਨ sqlite3_ਸਥਿਤੀ(ਪੂਰਨ op, ਪੂਰਨ *pCurrent, ਪੂਰਨ *pHighwater, ਪੂਰਨ resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  ਪੂਰਨ rc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( pCurrent==0 || pHighwater==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  ਜੇ( rc==0 ){
    *pCurrent = (ਪੂਰਨ)iCur;
    *pHighwater = (ਪੂਰਨ)iHwtr;
  }
  ਵਾਪਸ rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
ਸਥਿਰ u32 ਗਿਣਤੀ_Lookaside_Slots(Lookaside_ਥਾਂ *p){
  u32 cnt = 0;
  ਜਦੋਂ( p ){
    p = p->p_ਅਗਲਾ;
    cnt++;
  }
  ਵਾਪਸ cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
ਪੂਰਨ sqlite3_Lookaside_ਵਰਤਿਆ(sqlite3 *db, ਪੂਰਨ *pHighwater){
  u32 nInit = ਗਿਣਤੀ_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = ਗਿਣਤੀ_Lookaside_Slots(db->lookaside.pFree);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += ਗਿਣਤੀ_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += ਗਿਣਤੀ_Lookaside_Slots(db->lookaside.pSmallFree);
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ਪ੍ਰਤਿਜ੍ਞਾ( db->lookaside.n_ਥਾਂ >= nInit+nFree );
  ਜੇ( pHighwater ) *pHighwater = (ਪੂਰਨ)(db->lookaside.n_ਥਾਂ - nInit);
  ਵਾਪਸ (ਪੂਰਨ)(db->lookaside.n_ਥਾਂ - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
ਪੂਰਨ sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  ਪੂਰਨ op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  ਪੂਰਨ resetFlag            /* Reset high-water mark if true */
){
  ਪੂਰਨ rc = SQLITE_OK;   /* Return code */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);
  ਚੋਣ( op ){
    ਹਾਲਤ SQLITE_DBSTATUS_LOOKASIDE_ਵਰਤਿਆ: {
      ਪੂਰਨ H = 0;
      *pCurrent = sqlite3_Lookaside_ਵਰਤਿਆ(db, &H);
      *pHighwtr = H;
      ਜੇ( resetFlag ){
        Lookaside_ਥਾਂ *p = db->lookaside.pFree;
        ਜੇ( p ){
          ਜਦੋਂ( p->p_ਅਗਲਾ ) p = p->p_ਅਗਲਾ;
          p->p_ਅਗਲਾ = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        ਜੇ( p ){
          ਜਦੋਂ( p->p_ਅਗਲਾ ) p = p->p_ਅਗਲਾ;
          p->p_ਅਗਲਾ = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#ਜੇ_ਅੰਤ
      }
      ਰੋਕੋ;
    }

    ਹਾਲਤ SQLITE_DBSTATUS_LOOKASIDE_ਪ੍ਰਾਪਤੀ:
    ਹਾਲਤ SQLITE_DBSTATUS_LOOKASIDE_ਖੁੰਝ_ਮਾਪ:
    ਹਾਲਤ SQLITE_DBSTATUS_LOOKASIDE_ਖੁੰਝ_ਭਰਿਆ: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_ਪ੍ਰਾਪਤੀ );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_ਖੁੰਝ_ਮਾਪ );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_ਖੁੰਝ_ਭਰਿਆ );
      ਪ੍ਰਤਿਜ੍ਞਾ( (op-SQLITE_DBSTATUS_LOOKASIDE_ਪ੍ਰਾਪਤੀ)>=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( (op-SQLITE_DBSTATUS_LOOKASIDE_ਪ੍ਰਾਪਤੀ)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_ਪ੍ਰਾਪਤੀ];
      ਜੇ( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_ਪ੍ਰਾਪਤੀ] = 0;
      }
      ਰੋਕੋ;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    ਹਾਲਤ SQLITE_DBSTATUS_CACHE_ਵਰਤਿਆ_SHARED:
    ਹਾਲਤ SQLITE_DBSTATUS_CACHE_ਵਰਤਿਆ: {
      sqlite3_int64 ਕੁੱਲ_ਵਰਤਿਆ = 0;
      ਪੂਰਨ i;
      sqlite3BtreeEnterAll(db);
      ਲਈ(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        ਜੇ( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          ਪੂਰਨ nByte = sqlite3_Pager_Mem_ਵਰਤਿਆ(pPager);
          ਜੇ( op==SQLITE_DBSTATUS_CACHE_ਵਰਤਿਆ_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_ਗਿਣਤੀ(pBt);
          }
          ਕੁੱਲ_ਵਰਤਿਆ += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = ਕੁੱਲ_ਵਰਤਿਆ;
      *pHighwtr = 0;
      ਰੋਕੋ;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    ਹਾਲਤ SQLITE_DBSTATUS_SCHEMA_ਵਰਤਿਆ: {
      ਪੂਰਨ i;          /* Used to iterate through schemas */
      ਪੂਰਨ nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pn_ਬਾਈਟਾਂ_Freed = &nByte;
      ਪ੍ਰਤਿਜ੍ਞਾ( db->lookaside.p_ਸਿਰਾ==db->lookaside.p_True_ਸਿਰਾ );
      db->lookaside.p_ਸਿਰਾ = db->lookaside.p_ਸ਼ੁਰੂ;
      ਲਈ(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        ਜੇ( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(ਆਕਾਰ(HashElem)) * (
              pSchema->tblHash.ਗਿਣਤੀ_2 
            + pSchema->trigHash.ਗਿਣਤੀ_2
            + pSchema->idxHash.ਗਿਣਤੀ_2
            + pSchema->fkeyHash.ਗਿਣਤੀ_2
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          ਲਈ(p=sqlite_Hash_ਪਹਿਲਾ(&pSchema->trigHash); p; p=sqlite_Hash_ਅਗਲਾ(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqlite_Hash_ਅੰਕੜੇ(p));
          }
          ਲਈ(p=sqlite_Hash_ਪਹਿਲਾ(&pSchema->tblHash); p; p=sqlite_Hash_ਅਗਲਾ(p)){
            sqlite3_Delete_ਸਾਰਣੀ(db, (ਸਾਰਣੀ *)sqlite_Hash_ਅੰਕੜੇ(p));
          }
        }
      }
      db->pn_ਬਾਈਟਾਂ_Freed = 0;
      db->lookaside.p_ਸਿਰਾ = db->lookaside.p_True_ਸਿਰਾ;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      ਰੋਕੋ;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    ਹਾਲਤ SQLITE_DBSTATUS_STMT_ਵਰਤਿਆ: {
      ਬਣਤਰ Vdbe *pVdbe;         /* Used to iterate through VMs */
      ਪੂਰਨ nByte = 0;              /* Used to accumulate return value */

      db->pn_ਬਾਈਟਾਂ_Freed = &nByte;
      ਪ੍ਰਤਿਜ੍ਞਾ( db->lookaside.p_ਸਿਰਾ==db->lookaside.p_True_ਸਿਰਾ );
      db->lookaside.p_ਸਿਰਾ = db->lookaside.p_ਸ਼ੁਰੂ;
      ਲਈ(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.p_ਸਿਰਾ = db->lookaside.p_True_ਸਿਰਾ;
      db->pn_ਬਾਈਟਾਂ_Freed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      ਰੋਕੋ;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    ਹਾਲਤ SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    ਹਾਲਤ SQLITE_DBSTATUS_CACHE_ਪ੍ਰਾਪਤੀ:
    ਹਾਲਤ SQLITE_DBSTATUS_CACHE_ਖੁੰਝ:
    ਹਾਲਤ SQLITE_DBSTATUS_CACHE_WRITE:{
      ਪੂਰਨ i;
      u64 nRet = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_DBSTATUS_CACHE_ਖੁੰਝ==SQLITE_DBSTATUS_CACHE_ਪ੍ਰਾਪਤੀ+1 );
      ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_ਪ੍ਰਾਪਤੀ+2 );

      ਲਈ(i=0; i<db->nDb; i++){
        ਜੇ( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      ਰੋਕੋ;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    ਹਾਲਤ SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      ਜੇ( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      ਜੇ( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      ਰੋਕੋ;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    ਹਾਲਤ SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      ਰੋਕੋ;
    }

    ਮੂਲ: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
ਪੂਰਨ sqlite3_db_ਸਥਿਤੀ(
  sqlite3 *db,             /* The database connection whose status is desired */
  ਪੂਰਨ op,                  /* Status verb */
  ਪੂਰਨ *pCurrent,           /* Write current value here */
  ਪੂਰਨ *pHighwtr,           /* Write high-water mark here */
  ਪੂਰਨ resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  ਪੂਰਨ rc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  ਜੇ( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  ਵਾਪਸ rc;
}
