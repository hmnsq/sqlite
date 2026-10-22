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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#ఒకవేళ SQLITE_PTRSIZE>4
రకనిర్వచనం sqlite3_int64 sqlite3_Stat_విలువ_Type;
#లేకపోతే
రకనిర్వచనం u32 sqlite3_Stat_విలువ_Type;
#అయితే_ముగింపు
రకనిర్వచనం నిర్మాణం sqlite3StatType sqlite3StatType;
స్థిర SQLITE_WSD నిర్మాణం sqlite3StatType {
  sqlite3_Stat_విలువ_Type now_విలువ[10];  /* Current value */
  sqlite3_Stat_విలువ_Type mx_విలువ[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
స్థిర స్థిరరాశి అక్షరం statMutex[] = {
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
#నిర్వచితమైతే SQLITE_OMIT_WSD
# define wsdStatInit  sqlite3StatType *x = &GLOBAL(sqlite3StatType,sqlite3Stat)
# define wsdStat x[0]
#లేకపోతే
# define wsdStatInit
# define wsdStat sqlite3Stat
#అయితే_ముగింపు

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_స్థితి_విలువ(పూర్ణాంకం op){
  wsdStatInit;
  ప్రతిజ్ఞా( op>=0 && op<Array_కొలత(wsdStat.now_విలువ) );
  ప్రతిజ్ఞా( op>=0 && op<Array_కొలత(statMutex) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  తిరిగి wsdStat.now_విలువ[op];
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
శూన్య sqlite3_స్థితి_పైకి(పూర్ణాంకం op, పూర్ణాంకం N){
  wsdStatInit;
  ప్రతిజ్ఞా( op>=0 && op<Array_కొలత(wsdStat.now_విలువ) );
  ప్రతిజ్ఞా( op>=0 && op<Array_కొలత(statMutex) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_విలువ[op] += N;
  ఒకవేళ( wsdStat.now_విలువ[op]>wsdStat.mx_విలువ[op] ){
    wsdStat.mx_విలువ[op] = wsdStat.now_విలువ[op];
  }
}
శూన్య sqlite3_స్థితి_కిందికి(పూర్ణాంకం op, పూర్ణాంకం N){
  wsdStatInit;
  ప్రతిజ్ఞా( N>=0 );
  ప్రతిజ్ఞా( op>=0 && op<Array_కొలత(statMutex) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ప్రతిజ్ఞా( op>=0 && op<Array_కొలత(wsdStat.now_విలువ) );
  wsdStat.now_విలువ[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
శూన్య sqlite3_స్థితి_Highwater(పూర్ణాంకం op, పూర్ణాంకం X){
  sqlite3_Stat_విలువ_Type new_విలువ;
  wsdStatInit;
  ప్రతిజ్ఞా( X>=0 );
  new_విలువ = (sqlite3_Stat_విలువ_Type)X;
  ప్రతిజ్ఞా( op>=0 && op<Array_కొలత(wsdStat.now_విలువ) );
  ప్రతిజ్ఞా( op>=0 && op<Array_కొలత(statMutex) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ప్రతిజ్ఞా( op==SQLITE_స్థితి_MALLOC_కొలత
          || op==SQLITE_స్థితి_PAGECACHE_కొలత
          || op==SQLITE_స్థితి_PARSER_దొంతర );
  ఒకవేళ( new_విలువ>wsdStat.mx_విలువ[op] ){
    wsdStat.mx_విలువ[op] = new_విలువ;
  }
}

/*
** Query status information.
*/
పూర్ణాంకం sqlite3_status64(
  పూర్ణాంకం op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  పూర్ణాంకం resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  ఒకవేళ( op<0 || op>=Array_కొలత(wsdStat.now_విలువ) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCurrent==0 || pHighwater==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_విలువ[op];
  *pHighwater = wsdStat.mx_విలువ[op];
  ఒకవేళ( resetFlag ){
    wsdStat.mx_విలువ[op] = wsdStat.now_విలువ[op];
  }
  sqlite3_mutex_leave(pMutex);
  (శూన్య)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  తిరిగి SQLITE_OK;
}
పూర్ణాంకం sqlite3_స్థితి(పూర్ణాంకం op, పూర్ణాంకం *pCurrent, పూర్ణాంకం *pHighwater, పూర్ణాంకం resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  పూర్ణాంకం rc;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( pCurrent==0 || pHighwater==0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  ఒకవేళ( rc==0 ){
    *pCurrent = (పూర్ణాంకం)iCur;
    *pHighwater = (పూర్ణాంకం)iHwtr;
  }
  తిరిగి rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
స్థిర u32 లెక్క_Lookaside_Slots(Lookaside_స్థానం *p){
  u32 cnt = 0;
  అయితే( p ){
    p = p->p_తదుపరి;
    cnt++;
  }
  తిరిగి cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
పూర్ణాంకం sqlite3_Lookaside_వాడిన(sqlite3 *db, పూర్ణాంకం *pHighwater){
  u32 nInit = లెక్క_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = లెక్క_Lookaside_Slots(db->lookaside.pFree);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += లెక్క_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += లెక్క_Lookaside_Slots(db->lookaside.pSmallFree);
#అయితే_ముగింపు /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ప్రతిజ్ఞా( db->lookaside.n_స్థానం >= nInit+nFree );
  ఒకవేళ( pHighwater ) *pHighwater = (పూర్ణాంకం)(db->lookaside.n_స్థానం - nInit);
  తిరిగి (పూర్ణాంకం)(db->lookaside.n_స్థానం - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
పూర్ణాంకం sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  పూర్ణాంకం op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  పూర్ణాంకం resetFlag            /* Reset high-water mark if true */
){
  పూర్ణాంకం rc = SQLITE_OK;   /* Return code */
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);
  ఎంపిక( op ){
    స్థితి SQLITE_DBSTATUS_LOOKASIDE_వాడిన: {
      పూర్ణాంకం H = 0;
      *pCurrent = sqlite3_Lookaside_వాడిన(db, &H);
      *pHighwtr = H;
      ఒకవేళ( resetFlag ){
        Lookaside_స్థానం *p = db->lookaside.pFree;
        ఒకవేళ( p ){
          అయితే( p->p_తదుపరి ) p = p->p_తదుపరి;
          p->p_తదుపరి = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        ఒకవేళ( p ){
          అయితే( p->p_తదుపరి ) p = p->p_తదుపరి;
          p->p_తదుపరి = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#అయితే_ముగింపు
      }
      ఆపు;
    }

    స్థితి SQLITE_DBSTATUS_LOOKASIDE_తగిలింది:
    స్థితి SQLITE_DBSTATUS_LOOKASIDE_తప్పిపోవు_కొలత:
    స్థితి SQLITE_DBSTATUS_LOOKASIDE_తప్పిపోవు_నిండిన: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_తగిలింది );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_తప్పిపోవు_కొలత );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_తప్పిపోవు_నిండిన );
      ప్రతిజ్ఞా( (op-SQLITE_DBSTATUS_LOOKASIDE_తగిలింది)>=0 );
      ప్రతిజ్ఞా( (op-SQLITE_DBSTATUS_LOOKASIDE_తగిలింది)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_తగిలింది];
      ఒకవేళ( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_తగిలింది] = 0;
      }
      ఆపు;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    స్థితి SQLITE_DBSTATUS_CACHE_వాడిన_SHARED:
    స్థితి SQLITE_DBSTATUS_CACHE_వాడిన: {
      sqlite3_int64 మొత్తం_వాడిన = 0;
      పూర్ణాంకం i;
      sqlite3BtreeEnterAll(db);
      కొరకు(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        ఒకవేళ( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          పూర్ణాంకం nByte = sqlite3_Pager_Mem_వాడిన(pPager);
          ఒకవేళ( op==SQLITE_DBSTATUS_CACHE_వాడిన_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_లెక్క(pBt);
          }
          మొత్తం_వాడిన += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = మొత్తం_వాడిన;
      *pHighwtr = 0;
      ఆపు;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    స్థితి SQLITE_DBSTATUS_SCHEMA_వాడిన: {
      పూర్ణాంకం i;          /* Used to iterate through schemas */
      పూర్ణాంకం nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pn_బైట్లు_Freed = &nByte;
      ప్రతిజ్ఞా( db->lookaside.p_చివర==db->lookaside.p_True_చివర );
      db->lookaside.p_చివర = db->lookaside.p_ప్రారంభం;
      కొరకు(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        ఒకవేళ( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(పరిమాణం(HashElem)) * (
              pSchema->tblHash.లెక్క 
            + pSchema->trigHash.లెక్క
            + pSchema->idxHash.లెక్క
            + pSchema->fkeyHash.లెక్క
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          కొరకు(p=sqlite_Hash_మొదటి(&pSchema->trigHash); p; p=sqlite_Hash_తదుపరి(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqlite_Hash_దత్తాంశం(p));
          }
          కొరకు(p=sqlite_Hash_మొదటి(&pSchema->tblHash); p; p=sqlite_Hash_తదుపరి(p)){
            sqlite3_Delete_పట్టిక(db, (పట్టిక *)sqlite_Hash_దత్తాంశం(p));
          }
        }
      }
      db->pn_బైట్లు_Freed = 0;
      db->lookaside.p_చివర = db->lookaside.p_True_చివర;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      ఆపు;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    స్థితి SQLITE_DBSTATUS_STMT_వాడిన: {
      నిర్మాణం Vdbe *pVdbe;         /* Used to iterate through VMs */
      పూర్ణాంకం nByte = 0;              /* Used to accumulate return value */

      db->pn_బైట్లు_Freed = &nByte;
      ప్రతిజ్ఞా( db->lookaside.p_చివర==db->lookaside.p_True_చివర );
      db->lookaside.p_చివర = db->lookaside.p_ప్రారంభం;
      కొరకు(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.p_చివర = db->lookaside.p_True_చివర;
      db->pn_బైట్లు_Freed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      ఆపు;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    స్థితి SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    స్థితి SQLITE_DBSTATUS_CACHE_తగిలింది:
    స్థితి SQLITE_DBSTATUS_CACHE_తప్పిపోవు:
    స్థితి SQLITE_DBSTATUS_CACHE_WRITE:{
      పూర్ణాంకం i;
      u64 nRet = 0;
      ప్రతిజ్ఞా( SQLITE_DBSTATUS_CACHE_తప్పిపోవు==SQLITE_DBSTATUS_CACHE_తగిలింది+1 );
      ప్రతిజ్ఞా( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_తగిలింది+2 );

      కొరకు(i=0; i<db->nDb; i++){
        ఒకవేళ( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      ఆపు;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    స్థితి SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      ఒకవేళ( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3_Btree_Get_పుట_కొలత(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      ఒకవేళ( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      ఆపు;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    స్థితి SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      ఆపు;
    }

    అప్రమేయ: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
పూర్ణాంకం sqlite3_db_స్థితి(
  sqlite3 *db,             /* The database connection whose status is desired */
  పూర్ణాంకం op,                  /* Status verb */
  పూర్ణాంకం *pCurrent,           /* Write current value here */
  పూర్ణాంకం *pHighwtr,           /* Write high-water mark here */
  పూర్ణాంకం resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  పూర్ణాంకం rc;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  ఒకవేళ( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  తిరిగి rc;
}
