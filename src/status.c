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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#यदि SQLITE_PTRSIZE>4
प्रकारनिर्देश sqlite3_int64 sqlite3_Stat_मान_Type;
#अन्यथा
प्रकारनिर्देश u32 sqlite3_Stat_मान_Type;
#endif
प्रकारनिर्देश संरचना sqlite3StatType sqlite3StatType;
स्थिर SQLITE_WSD संरचना sqlite3StatType {
  sqlite3_Stat_मान_Type now_मान[10];  /* Current value */
  sqlite3_Stat_मान_Type mx_मान[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
स्थिर अचर वर्ण statMutex[] = {
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
#अन्यथा
# define wsdStatInit
# define wsdStat sqlite3Stat
#endif

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_Status_मान(पूर्णाङ्क op){
  wsdStatInit;
  प्रतिज्ञा( op>=0 && op<ArraySize(wsdStat.now_मान) );
  प्रतिज्ञा( op>=0 && op<ArraySize(statMutex) );
  प्रतिज्ञा( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  फर्काऊ wsdStat.now_मान[op];
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
शून्य sqlite3StatusUp(पूर्णाङ्क op, पूर्णाङ्क N){
  wsdStatInit;
  प्रतिज्ञा( op>=0 && op<ArraySize(wsdStat.now_मान) );
  प्रतिज्ञा( op>=0 && op<ArraySize(statMutex) );
  प्रतिज्ञा( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_मान[op] += N;
  यदि( wsdStat.now_मान[op]>wsdStat.mx_मान[op] ){
    wsdStat.mx_मान[op] = wsdStat.now_मान[op];
  }
}
शून्य sqlite3StatusDown(पूर्णाङ्क op, पूर्णाङ्क N){
  wsdStatInit;
  प्रतिज्ञा( N>=0 );
  प्रतिज्ञा( op>=0 && op<ArraySize(statMutex) );
  प्रतिज्ञा( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  प्रतिज्ञा( op>=0 && op<ArraySize(wsdStat.now_मान) );
  wsdStat.now_मान[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
शून्य sqlite3StatusHighwater(पूर्णाङ्क op, पूर्णाङ्क X){
  sqlite3_Stat_मान_Type new_मान;
  wsdStatInit;
  प्रतिज्ञा( X>=0 );
  new_मान = (sqlite3_Stat_मान_Type)X;
  प्रतिज्ञा( op>=0 && op<ArraySize(wsdStat.now_मान) );
  प्रतिज्ञा( op>=0 && op<ArraySize(statMutex) );
  प्रतिज्ञा( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  प्रतिज्ञा( op==SQLITE_STATUS_MALLOC_SIZE
          || op==SQLITE_STATUS_PAGECACHE_SIZE
          || op==SQLITE_STATUS_PARSER_थाक );
  यदि( new_मान>wsdStat.mx_मान[op] ){
    wsdStat.mx_मान[op] = new_मान;
  }
}

/*
** Query status information.
*/
पूर्णाङ्क sqlite3_status64(
  पूर्णाङ्क op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  पूर्णाङ्क resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  यदि( op<0 || op>=ArraySize(wsdStat.now_मान) ){
    फर्काऊ SQLITE_MISUSE_BKPT;
  }
#ifdef SQLITE_ENABLE_API_ARMOR
  यदि( pCurrent==0 || pHighwater==0 ) फर्काऊ SQLITE_MISUSE_BKPT;
#endif
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_मान[op];
  *pHighwater = wsdStat.mx_मान[op];
  यदि( resetFlag ){
    wsdStat.mx_मान[op] = wsdStat.now_मान[op];
  }
  sqlite3_mutex_leave(pMutex);
  (शून्य)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  फर्काऊ SQLITE_OK;
}
पूर्णाङ्क sqlite3_status(पूर्णाङ्क op, पूर्णाङ्क *pCurrent, पूर्णाङ्क *pHighwater, पूर्णाङ्क resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  पूर्णाङ्क rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  यदि( pCurrent==0 || pHighwater==0 ) फर्काऊ SQLITE_MISUSE_BKPT;
#endif
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  यदि( rc==0 ){
    *pCurrent = (पूर्णाङ्क)iCur;
    *pHighwater = (पूर्णाङ्क)iHwtr;
  }
  फर्काऊ rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
स्थिर u32 गन्ती_Lookaside_Slots(LookasideSlot *p){
  u32 cnt = 0;
  जबसम्म( p ){
    p = p->pNext;
    cnt++;
  }
  फर्काऊ cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
पूर्णाङ्क sqlite3LookasideUsed(sqlite3 *db, पूर्णाङ्क *pHighwater){
  u32 nInit = गन्ती_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = गन्ती_Lookaside_Slots(db->lookaside.pFree);
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += गन्ती_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += गन्ती_Lookaside_Slots(db->lookaside.pSmallFree);
#endif /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  प्रतिज्ञा( db->lookaside.nSlot >= nInit+nFree );
  यदि( pHighwater ) *pHighwater = (पूर्णाङ्क)(db->lookaside.nSlot - nInit);
  फर्काऊ (पूर्णाङ्क)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
पूर्णाङ्क sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  पूर्णाङ्क op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  पूर्णाङ्क resetFlag            /* Reset high-water mark if true */
){
  पूर्णाङ्क rc = SQLITE_OK;   /* Return code */
#ifdef SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    फर्काऊ SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  विकल्प( op ){
    पक्ष SQLITE_DBSTATUS_LOOKASIDE_USED: {
      पूर्णाङ्क H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      यदि( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        यदि( p ){
          जबसम्म( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ifndef SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        यदि( p ){
          जबसम्म( p->pNext ) p = p->pNext;
          p->pNext = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#endif
      }
      रोक;
    }

    पक्ष SQLITE_DBSTATUS_LOOKASIDE_HIT:
    पक्ष SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE:
    पक्ष SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      प्रतिज्ञा( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      प्रतिज्ञा( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      यदि( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      रोक;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    पक्ष SQLITE_DBSTATUS_CACHE_USED_SHARED:
    पक्ष SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 totalUsed = 0;
      पूर्णाङ्क i;
      sqlite3BtreeEnterAll(db);
      लागि(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        यदि( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          पूर्णाङ्क nByte = sqlite3PagerMemUsed(pPager);
          यदि( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_गन्ती(pBt);
          }
          totalUsed += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = totalUsed;
      *pHighwtr = 0;
      रोक;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    पक्ष SQLITE_DBSTATUS_SCHEMA_USED: {
      पूर्णाङ्क i;          /* Used to iterate through schemas */
      पूर्णाङ्क nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      प्रतिज्ञा( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      लागि(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        यदि( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(परिमाण(HashElem)) * (
              pSchema->tblHash.गन्ती 
            + pSchema->trigHash.गन्ती
            + pSchema->idxHash.गन्ती
            + pSchema->fkeyHash.गन्ती
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          लागि(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqliteHashData(p));
          }
          लागि(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
            sqlite3DeleteTable(db, (Table *)sqliteHashData(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      रोक;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    पक्ष SQLITE_DBSTATUS_STMT_USED: {
      संरचना Vdbe *pVdbe;         /* Used to iterate through VMs */
      पूर्णाङ्क nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      प्रतिज्ञा( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      लागि(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      रोक;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    पक्ष SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    पक्ष SQLITE_DBSTATUS_CACHE_HIT:
    पक्ष SQLITE_DBSTATUS_CACHE_MISS:
    पक्ष SQLITE_DBSTATUS_CACHE_WRITE:{
      पूर्णाङ्क i;
      u64 nRet = 0;
      प्रतिज्ञा( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      प्रतिज्ञा( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      लागि(i=0; i<db->nDb; i++){
        यदि( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      रोक;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    पक्ष SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      यदि( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3BtreeGetPageSize(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      यदि( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      रोक;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    पक्ष SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      रोक;
    }

    स्वभाव: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  फर्काऊ rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
पूर्णाङ्क sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  पूर्णाङ्क op,                  /* Status verb */
  पूर्णाङ्क *pCurrent,           /* Write current value here */
  पूर्णाङ्क *pHighwtr,           /* Write high-water mark here */
  पूर्णाङ्क resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  पूर्णाङ्क rc;
#ifdef SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    फर्काऊ SQLITE_MISUSE_BKPT;
  }
#endif
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  यदि( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  फर्काऊ rc;
}
