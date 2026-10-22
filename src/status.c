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
#إذا SQLITE_PTRSIZE>4
تعريفنوع sqlite3_int64 sqlite3_Stat_قدر_Type;
#وإلا
تعريفنوع u32 sqlite3_Stat_قدر_Type;
#جے_انت
تعريفنوع بنية sqlite3StatType sqlite3StatType;
ساكن SQLITE_WSD بنية sqlite3StatType {
  sqlite3_Stat_قدر_Type now_قدر[10];  /* Current value */
  sqlite3_Stat_قدر_Type mx_قدر[10];   /* Maximum value */
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
#جے_تعریف_شدہ SQLITE_OMIT_WSD
# define wsdStatInit  sqlite3StatType *x = &GLOBAL(sqlite3StatType,sqlite3Stat)
# define wsdStat x[0]
#وإلا
# define wsdStatInit
# define wsdStat sqlite3Stat
#جے_انت

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_حالت_قدر(صحيح op){
  wsdStatInit;
  أكد( op>=0 && op<Array_ناپ(wsdStat.now_قدر) );
  أكد( op>=0 && op<Array_ناپ(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أرجع wsdStat.now_قدر[op];
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
فراغ sqlite3_حالت_اتے(صحيح op, صحيح N){
  wsdStatInit;
  أكد( op>=0 && op<Array_ناپ(wsdStat.now_قدر) );
  أكد( op>=0 && op<Array_ناپ(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_قدر[op] += N;
  إذا( wsdStat.now_قدر[op]>wsdStat.mx_قدر[op] ){
    wsdStat.mx_قدر[op] = wsdStat.now_قدر[op];
  }
}
فراغ sqlite3_حالت_تھلے(صحيح op, صحيح N){
  wsdStatInit;
  أكد( N>=0 );
  أكد( op>=0 && op<Array_ناپ(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أكد( op>=0 && op<Array_ناپ(wsdStat.now_قدر) );
  wsdStat.now_قدر[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
فراغ sqlite3_حالت_Highwater(صحيح op, صحيح X){
  sqlite3_Stat_قدر_Type new_قدر;
  wsdStatInit;
  أكد( X>=0 );
  new_قدر = (sqlite3_Stat_قدر_Type)X;
  أكد( op>=0 && op<Array_ناپ(wsdStat.now_قدر) );
  أكد( op>=0 && op<Array_ناپ(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أكد( op==SQLITE_حالت_MALLOC_ناپ
          || op==SQLITE_حالت_PAGECACHE_ناپ
          || op==SQLITE_حالت_PARSER_ڈھیر );
  إذا( new_قدر>wsdStat.mx_قدر[op] ){
    wsdStat.mx_قدر[op] = new_قدر;
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
  إذا( op<0 || op>=Array_ناپ(wsdStat.now_قدر) ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( pCurrent==0 || pHighwater==0 ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_قدر[op];
  *pHighwater = wsdStat.mx_قدر[op];
  إذا( resetFlag ){
    wsdStat.mx_قدر[op] = wsdStat.now_قدر[op];
  }
  sqlite3_mutex_leave(pMutex);
  (فراغ)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  أرجع SQLITE_OK;
}
صحيح sqlite3_حالت(صحيح op, صحيح *pCurrent, صحيح *pHighwater, صحيح resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  صحيح rc;
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( pCurrent==0 || pHighwater==0 ) أرجع SQLITE_MISUSE_BKPT;
#جے_انت
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  إذا( rc==0 ){
    *pCurrent = (صحيح)iCur;
    *pHighwater = (صحيح)iHwtr;
  }
  أرجع rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
ساكن u32 گنتی_Lookaside_Slots(Lookaside_تھاں *p){
  u32 cnt = 0;
  طالما( p ){
    p = p->p_اگلا;
    cnt++;
  }
  أرجع cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
صحيح sqlite3_Lookaside_ورتیا(sqlite3 *db, صحيح *pHighwater){
  u32 nInit = گنتی_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = گنتی_Lookaside_Slots(db->lookaside.pFree);
#جے_بنا_تعریف SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += گنتی_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += گنتی_Lookaside_Slots(db->lookaside.pSmallFree);
#جے_انت /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  أكد( db->lookaside.n_تھاں >= nInit+nFree );
  إذا( pHighwater ) *pHighwater = (صحيح)(db->lookaside.n_تھاں - nInit);
  أرجع (صحيح)(db->lookaside.n_تھاں - (nInit+nFree));
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
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#جے_انت
  sqlite3_mutex_enter(db->mutex);
  بدل( op ){
    حالة SQLITE_DBSTATUS_LOOKASIDE_ورتیا: {
      صحيح H = 0;
      *pCurrent = sqlite3_Lookaside_ورتیا(db, &H);
      *pHighwtr = H;
      إذا( resetFlag ){
        Lookaside_تھاں *p = db->lookaside.pFree;
        إذا( p ){
          طالما( p->p_اگلا ) p = p->p_اگلا;
          p->p_اگلا = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#جے_بنا_تعریف SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        إذا( p ){
          طالما( p->p_اگلا ) p = p->p_اگلا;
          p->p_اگلا = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#جے_انت
      }
      اكسر;
    }

    حالة SQLITE_DBSTATUS_LOOKASIDE_کامیابی:
    حالة SQLITE_DBSTATUS_LOOKASIDE_کھنج_ناپ:
    حالة SQLITE_DBSTATUS_LOOKASIDE_کھنج_بھریا: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_کامیابی );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_کھنج_ناپ );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_کھنج_بھریا );
      أكد( (op-SQLITE_DBSTATUS_LOOKASIDE_کامیابی)>=0 );
      أكد( (op-SQLITE_DBSTATUS_LOOKASIDE_کامیابی)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_کامیابی];
      إذا( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_کامیابی] = 0;
      }
      اكسر;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    حالة SQLITE_DBSTATUS_CACHE_ورتیا_SHARED:
    حالة SQLITE_DBSTATUS_CACHE_ورتیا: {
      sqlite3_int64 کل_ورتیا = 0;
      صحيح i;
      sqlite3BtreeEnterAll(db);
      لكل(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        إذا( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          صحيح nByte = sqlite3_Pager_Mem_ورتیا(pPager);
          إذا( op==SQLITE_DBSTATUS_CACHE_ورتیا_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_گنتی(pBt);
          }
          کل_ورتیا += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = کل_ورتیا;
      *pHighwtr = 0;
      اكسر;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    حالة SQLITE_DBSTATUS_SCHEMA_ورتیا: {
      صحيح i;          /* Used to iterate through schemas */
      صحيح nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pn_بائٹاں_Freed = &nByte;
      أكد( db->lookaside.p_سرا==db->lookaside.p_True_سرا );
      db->lookaside.p_سرا = db->lookaside.p_شروع;
      لكل(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        إذا( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(حجم(HashElem)) * (
              pSchema->tblHash.گنتی 
            + pSchema->trigHash.گنتی
            + pSchema->idxHash.گنتی
            + pSchema->fkeyHash.گنتی
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          لكل(p=sqlite_Hash_پہلا(&pSchema->trigHash); p; p=sqlite_Hash_اگلا(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqlite_Hash_انکڑے(p));
          }
          لكل(p=sqlite_Hash_پہلا(&pSchema->tblHash); p; p=sqlite_Hash_اگلا(p)){
            sqlite3_Delete_جدول(db, (جدول *)sqlite_Hash_انکڑے(p));
          }
        }
      }
      db->pn_بائٹاں_Freed = 0;
      db->lookaside.p_سرا = db->lookaside.p_True_سرا;
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
    حالة SQLITE_DBSTATUS_STMT_ورتیا: {
      بنية Vdbe *pVdbe;         /* Used to iterate through VMs */
      صحيح nByte = 0;              /* Used to accumulate return value */

      db->pn_بائٹاں_Freed = &nByte;
      أكد( db->lookaside.p_سرا==db->lookaside.p_True_سرا );
      db->lookaside.p_سرا = db->lookaside.p_شروع;
      لكل(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.p_سرا = db->lookaside.p_True_سرا;
      db->pn_بائٹاں_Freed = 0;

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
    حالة SQLITE_DBSTATUS_CACHE_کامیابی:
    حالة SQLITE_DBSTATUS_CACHE_کھنج:
    حالة SQLITE_DBSTATUS_CACHE_WRITE:{
      صحيح i;
      u64 nRet = 0;
      أكد( SQLITE_DBSTATUS_CACHE_کھنج==SQLITE_DBSTATUS_CACHE_کامیابی+1 );
      أكد( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_کامیابی+2 );

      لكل(i=0; i<db->nDb; i++){
        إذا( db->aDb[i].pBt ){
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
      إذا( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3_Btree_Get_ورقہ_ناپ(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      إذا( resetFlag ) db->nSpill = 0;
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
صحيح sqlite3_db_حالت(
  sqlite3 *db,             /* The database connection whose status is desired */
  صحيح op,                  /* Status verb */
  صحيح *pCurrent,           /* Write current value here */
  صحيح *pHighwtr,           /* Write high-water mark here */
  صحيح resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  صحيح rc;
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#جے_انت
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  إذا( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  أرجع rc;
}
