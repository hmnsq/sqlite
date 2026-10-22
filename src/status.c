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
قسمتعریف sqlite3_int64 sqlite3_Stat_قدر_Type;
#ورنہ
قسمتعریف u32 sqlite3_Stat_قدر_Type;
#اگر_ختم
قسمتعریف ساخت sqlite3StatType sqlite3StatType;
ساکن SQLITE_WSD ساخت sqlite3StatType {
  sqlite3_Stat_قدر_Type now_قدر[10];  /* Current value */
  sqlite3_Stat_قدر_Type mx_قدر[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
ساکن ثابت حرف statMutex[] = {
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
#اگر_تعریف_شدہ SQLITE_OMIT_WSD
# define wsdStatInit  sqlite3StatType *x = &GLOBAL(sqlite3StatType,sqlite3Stat)
# define wsdStat x[0]
#ورنہ
# define wsdStatInit
# define wsdStat sqlite3Stat
#اگر_ختم

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_حالت_قدر(صحیح op){
  wsdStatInit;
  أكد( op>=0 && op<Array_سائز(wsdStat.now_قدر) );
  أكد( op>=0 && op<Array_سائز(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  واپس wsdStat.now_قدر[op];
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
خالی sqlite3_حالت_اوپر(صحیح op, صحیح N){
  wsdStatInit;
  أكد( op>=0 && op<Array_سائز(wsdStat.now_قدر) );
  أكد( op>=0 && op<Array_سائز(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_قدر[op] += N;
  اگر( wsdStat.now_قدر[op]>wsdStat.mx_قدر[op] ){
    wsdStat.mx_قدر[op] = wsdStat.now_قدر[op];
  }
}
خالی sqlite3_حالت_نیچے_کو(صحیح op, صحیح N){
  wsdStatInit;
  أكد( N>=0 );
  أكد( op>=0 && op<Array_سائز(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أكد( op>=0 && op<Array_سائز(wsdStat.now_قدر) );
  wsdStat.now_قدر[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
خالی sqlite3_حالت_Highwater(صحیح op, صحیح X){
  sqlite3_Stat_قدر_Type new_قدر;
  wsdStatInit;
  أكد( X>=0 );
  new_قدر = (sqlite3_Stat_قدر_Type)X;
  أكد( op>=0 && op<Array_سائز(wsdStat.now_قدر) );
  أكد( op>=0 && op<Array_سائز(statMutex) );
  أكد( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  أكد( op==SQLITE_حالت_MALLOC_سائز
          || op==SQLITE_حالت_PAGECACHE_سائز
          || op==SQLITE_حالت_PARSER_ڈھیر );
  اگر( new_قدر>wsdStat.mx_قدر[op] ){
    wsdStat.mx_قدر[op] = new_قدر;
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
  اگر( op<0 || op>=Array_سائز(wsdStat.now_قدر) ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCurrent==0 || pHighwater==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_قدر[op];
  *pHighwater = wsdStat.mx_قدر[op];
  اگر( resetFlag ){
    wsdStat.mx_قدر[op] = wsdStat.now_قدر[op];
  }
  sqlite3_mutex_leave(pMutex);
  (خالی)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  واپس SQLITE_OK;
}
صحیح sqlite3_حالت(صحیح op, صحیح *pCurrent, صحیح *pHighwater, صحیح resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  صحیح rc;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( pCurrent==0 || pHighwater==0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  اگر( rc==0 ){
    *pCurrent = (صحیح)iCur;
    *pHighwater = (صحیح)iHwtr;
  }
  واپس rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
ساکن u32 گنتی_Lookaside_Slots(Lookaside_خانہ *p){
  u32 cnt = 0;
  جب( p ){
    p = p->p_اگلا;
    cnt++;
  }
  واپس cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
صحیح sqlite3_Lookaside_مستعمل(sqlite3 *db, صحیح *pHighwater){
  u32 nInit = گنتی_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = گنتی_Lookaside_Slots(db->lookaside.pFree);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += گنتی_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += گنتی_Lookaside_Slots(db->lookaside.pSmallFree);
#اگر_ختم /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  أكد( db->lookaside.n_خانہ >= nInit+nFree );
  اگر( pHighwater ) *pHighwater = (صحیح)(db->lookaside.n_خانہ - nInit);
  واپس (صحیح)(db->lookaside.n_خانہ - (nInit+nFree));
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
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  انتخاب( op ){
    حالت SQLITE_DBSTATUS_LOOKASIDE_مستعمل: {
      صحیح H = 0;
      *pCurrent = sqlite3_Lookaside_مستعمل(db, &H);
      *pHighwtr = H;
      اگر( resetFlag ){
        Lookaside_خانہ *p = db->lookaside.pFree;
        اگر( p ){
          جب( p->p_اگلا ) p = p->p_اگلا;
          p->p_اگلا = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        اگر( p ){
          جب( p->p_اگلا ) p = p->p_اگلا;
          p->p_اگلا = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#اگر_ختم
      }
      توڑو;
    }

    حالت SQLITE_DBSTATUS_LOOKASIDE_کامیابی:
    حالت SQLITE_DBSTATUS_LOOKASIDE_ناکامی_سائز:
    حالت SQLITE_DBSTATUS_LOOKASIDE_ناکامی_بھرا: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_کامیابی );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_ناکامی_سائز );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_ناکامی_بھرا );
      أكد( (op-SQLITE_DBSTATUS_LOOKASIDE_کامیابی)>=0 );
      أكد( (op-SQLITE_DBSTATUS_LOOKASIDE_کامیابی)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_کامیابی];
      اگر( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_کامیابی] = 0;
      }
      توڑو;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    حالت SQLITE_DBSTATUS_CACHE_مستعمل_SHARED:
    حالت SQLITE_DBSTATUS_CACHE_مستعمل: {
      sqlite3_int64 کل_مستعمل = 0;
      صحیح i;
      sqlite3BtreeEnterAll(db);
      برائے(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        اگر( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          صحیح nByte = sqlite3_Pager_Mem_مستعمل(pPager);
          اگر( op==SQLITE_DBSTATUS_CACHE_مستعمل_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_گنتی(pBt);
          }
          کل_مستعمل += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = کل_مستعمل;
      *pHighwtr = 0;
      توڑو;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    حالت SQLITE_DBSTATUS_SCHEMA_مستعمل: {
      صحیح i;          /* Used to iterate through schemas */
      صحیح nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pn_بائٹیں_Freed = &nByte;
      أكد( db->lookaside.p_انتہا==db->lookaside.p_True_انتہا );
      db->lookaside.p_انتہا = db->lookaside.p_آغاز;
      برائے(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        اگر( ALWAYS(pSchema!=0) ){
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

          برائے(p=sqlite_Hash_پہلا(&pSchema->trigHash); p; p=sqlite_Hash_اگلا(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqlite_Hash_ڈیٹا(p));
          }
          برائے(p=sqlite_Hash_پہلا(&pSchema->tblHash); p; p=sqlite_Hash_اگلا(p)){
            sqlite3_Delete_جدول(db, (جدول *)sqlite_Hash_ڈیٹا(p));
          }
        }
      }
      db->pn_بائٹیں_Freed = 0;
      db->lookaside.p_انتہا = db->lookaside.p_True_انتہا;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      توڑو;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    حالت SQLITE_DBSTATUS_STMT_مستعمل: {
      ساخت Vdbe *pVdbe;         /* Used to iterate through VMs */
      صحیح nByte = 0;              /* Used to accumulate return value */

      db->pn_بائٹیں_Freed = &nByte;
      أكد( db->lookaside.p_انتہا==db->lookaside.p_True_انتہا );
      db->lookaside.p_انتہا = db->lookaside.p_آغاز;
      برائے(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.p_انتہا = db->lookaside.p_True_انتہا;
      db->pn_بائٹیں_Freed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      توڑو;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    حالت SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    حالت SQLITE_DBSTATUS_CACHE_کامیابی:
    حالت SQLITE_DBSTATUS_CACHE_ناکامی:
    حالت SQLITE_DBSTATUS_CACHE_WRITE:{
      صحیح i;
      u64 nRet = 0;
      أكد( SQLITE_DBSTATUS_CACHE_ناکامی==SQLITE_DBSTATUS_CACHE_کامیابی+1 );
      أكد( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_کامیابی+2 );

      برائے(i=0; i<db->nDb; i++){
        اگر( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      توڑو;
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
        nRet *= sqlite3_Btree_Get_صفحہ_سائز(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      اگر( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      توڑو;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    حالت SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      توڑو;
    }

    طےشدہ: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
صحیح sqlite3_db_حالت(
  sqlite3 *db,             /* The database connection whose status is desired */
  صحیح op,                  /* Status verb */
  صحیح *pCurrent,           /* Write current value here */
  صحیح *pHighwtr,           /* Write high-water mark here */
  صحیح resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  صحیح rc;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  اگر( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  واپس rc;
}
