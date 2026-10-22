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
#ಆದರೆ SQLITE_PTRSIZE>4
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ sqlite3_int64 sqlite3_Stat_ಮೌಲ್ಯ_Type;
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u32 sqlite3_Stat_ಮೌಲ್ಯ_Type;
#ಆದರೆ_ಅಂತ್ಯ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ sqlite3StatType sqlite3StatType;
ಸ್ಥಿರ SQLITE_WSD ರಚನೆ sqlite3StatType {
  sqlite3_Stat_ಮೌಲ್ಯ_Type now_ಮೌಲ್ಯ[10];  /* Current value */
  sqlite3_Stat_ಮೌಲ್ಯ_Type mx_ಮೌಲ್ಯ[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ statMutex[] = {
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WSD
# define wsdStatInit  sqlite3StatType *x = &GLOBAL(sqlite3StatType,sqlite3Stat)
# define wsdStat x[0]
#ಇಲ್ಲದಿದ್ದರೆ
# define wsdStatInit
# define wsdStat sqlite3Stat
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_ಸ್ಥಿತಿ_ಮೌಲ್ಯ(ಪೂರ್ಣಾಂಕ op){
  wsdStatInit;
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<Array_ಗಾತ್ರ(wsdStat.now_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<Array_ಗಾತ್ರ(statMutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ಹಿಂತಿರುಗಿ wsdStat.now_ಮೌಲ್ಯ[op];
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
ಶೂನ್ಯ sqlite3_ಸ್ಥಿತಿ_ಮೇಲೆ(ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ N){
  wsdStatInit;
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<Array_ಗಾತ್ರ(wsdStat.now_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<Array_ಗಾತ್ರ(statMutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_ಮೌಲ್ಯ[op] += N;
  ಆದರೆ( wsdStat.now_ಮೌಲ್ಯ[op]>wsdStat.mx_ಮೌಲ್ಯ[op] ){
    wsdStat.mx_ಮೌಲ್ಯ[op] = wsdStat.now_ಮೌಲ್ಯ[op];
  }
}
ಶೂನ್ಯ sqlite3_ಸ್ಥಿತಿ_ಕೆಳಕ್ಕೆ(ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ N){
  wsdStatInit;
  ಪ್ರತಿಜ್ಞಾ( N>=0 );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<Array_ಗಾತ್ರ(statMutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<Array_ಗಾತ್ರ(wsdStat.now_ಮೌಲ್ಯ) );
  wsdStat.now_ಮೌಲ್ಯ[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
ಶೂನ್ಯ sqlite3_ಸ್ಥಿತಿ_Highwater(ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ X){
  sqlite3_Stat_ಮೌಲ್ಯ_Type new_ಮೌಲ್ಯ;
  wsdStatInit;
  ಪ್ರತಿಜ್ಞಾ( X>=0 );
  new_ಮೌಲ್ಯ = (sqlite3_Stat_ಮೌಲ್ಯ_Type)X;
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<Array_ಗಾತ್ರ(wsdStat.now_ಮೌಲ್ಯ) );
  ಪ್ರತಿಜ್ಞಾ( op>=0 && op<Array_ಗಾತ್ರ(statMutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  ಪ್ರತಿಜ್ಞಾ( op==SQLITE_ಸ್ಥಿತಿ_MALLOC_ಗಾತ್ರ
          || op==SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಗಾತ್ರ
          || op==SQLITE_ಸ್ಥಿತಿ_PARSER_ರಾಶಿ );
  ಆದರೆ( new_ಮೌಲ್ಯ>wsdStat.mx_ಮೌಲ್ಯ[op] ){
    wsdStat.mx_ಮೌಲ್ಯ[op] = new_ಮೌಲ್ಯ;
  }
}

/*
** Query status information.
*/
ಪೂರ್ಣಾಂಕ sqlite3_status64(
  ಪೂರ್ಣಾಂಕ op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  ಪೂರ್ಣಾಂಕ resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  ಆದರೆ( op<0 || op>=Array_ಗಾತ್ರ(wsdStat.now_ಮೌಲ್ಯ) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCurrent==0 || pHighwater==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_ಮೌಲ್ಯ[op];
  *pHighwater = wsdStat.mx_ಮೌಲ್ಯ[op];
  ಆದರೆ( resetFlag ){
    wsdStat.mx_ಮೌಲ್ಯ[op] = wsdStat.now_ಮೌಲ್ಯ[op];
  }
  sqlite3_mutex_leave(pMutex);
  (ಶೂನ್ಯ)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ಥಿತಿ(ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ *pCurrent, ಪೂರ್ಣಾಂಕ *pHighwater, ಪೂರ್ಣಾಂಕ resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  ಪೂರ್ಣಾಂಕ rc;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( pCurrent==0 || pHighwater==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  ಆದರೆ( rc==0 ){
    *pCurrent = (ಪೂರ್ಣಾಂಕ)iCur;
    *pHighwater = (ಪೂರ್ಣಾಂಕ)iHwtr;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
ಸ್ಥಿರ u32 ಎಣಿಕೆ_Lookaside_Slots(Lookaside_ಸ್ಥಾನ *p){
  u32 cnt = 0;
  ಆಗ( p ){
    p = p->p_ಮುಂದಿನ;
    cnt++;
  }
  ಹಿಂತಿರುಗಿ cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
ಪೂರ್ಣಾಂಕ sqlite3_Lookaside_ಬಳಸಿದ(sqlite3 *db, ಪೂರ್ಣಾಂಕ *pHighwater){
  u32 nInit = ಎಣಿಕೆ_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = ಎಣಿಕೆ_Lookaside_Slots(db->lookaside.pFree);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += ಎಣಿಕೆ_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += ಎಣಿಕೆ_Lookaside_Slots(db->lookaside.pSmallFree);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ಪ್ರತಿಜ್ಞಾ( db->lookaside.n_ಸ್ಥಾನ >= nInit+nFree );
  ಆದರೆ( pHighwater ) *pHighwater = (ಪೂರ್ಣಾಂಕ)(db->lookaside.n_ಸ್ಥಾನ - nInit);
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)(db->lookaside.n_ಸ್ಥಾನ - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
ಪೂರ್ಣಾಂಕ sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  ಪೂರ್ಣಾಂಕ op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  ಪೂರ್ಣಾಂಕ resetFlag            /* Reset high-water mark if true */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;   /* Return code */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  ಆಯ್ಕೆ( op ){
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_LOOKASIDE_ಬಳಸಿದ: {
      ಪೂರ್ಣಾಂಕ H = 0;
      *pCurrent = sqlite3_Lookaside_ಬಳಸಿದ(db, &H);
      *pHighwtr = H;
      ಆದರೆ( resetFlag ){
        Lookaside_ಸ್ಥಾನ *p = db->lookaside.pFree;
        ಆದರೆ( p ){
          ಆಗ( p->p_ಮುಂದಿನ ) p = p->p_ಮುಂದಿನ;
          p->p_ಮುಂದಿನ = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        ಆದರೆ( p ){
          ಆಗ( p->p_ಮುಂದಿನ ) p = p->p_ಮುಂದಿನ;
          p->p_ಮುಂದಿನ = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#ಆದರೆ_ಅಂತ್ಯ
      }
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ SQLITE_DBSTATUS_LOOKASIDE_ದೊರಕಿತು:
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_LOOKASIDE_ತಪ್ಪಿದ_ಗಾತ್ರ:
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_LOOKASIDE_ತಪ್ಪಿದ_ತುಂಬಿದ: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_ದೊರಕಿತು );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_ತಪ್ಪಿದ_ಗಾತ್ರ );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_ತಪ್ಪಿದ_ತುಂಬಿದ );
      ಪ್ರತಿಜ್ಞಾ( (op-SQLITE_DBSTATUS_LOOKASIDE_ದೊರಕಿತು)>=0 );
      ಪ್ರತಿಜ್ಞಾ( (op-SQLITE_DBSTATUS_LOOKASIDE_ದೊರಕಿತು)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_ದೊರಕಿತು];
      ಆದರೆ( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_ದೊರಕಿತು] = 0;
      }
      ನಿಲ್ಲಿಸು;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_CACHE_ಬಳಸಿದ_SHARED:
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_CACHE_ಬಳಸಿದ: {
      sqlite3_int64 ಒಟ್ಟು_ಬಳಸಿದ = 0;
      ಪೂರ್ಣಾಂಕ i;
      sqlite3BtreeEnterAll(db);
      ಗಾಗಿ(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        ಆದರೆ( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          ಪೂರ್ಣಾಂಕ nByte = sqlite3_Pager_Mem_ಬಳಸಿದ(pPager);
          ಆದರೆ( op==SQLITE_DBSTATUS_CACHE_ಬಳಸಿದ_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_ಎಣಿಕೆ(pBt);
          }
          ಒಟ್ಟು_ಬಳಸಿದ += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = ಒಟ್ಟು_ಬಳಸಿದ;
      *pHighwtr = 0;
      ನಿಲ್ಲಿಸು;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_SCHEMA_ಬಳಸಿದ: {
      ಪೂರ್ಣಾಂಕ i;          /* Used to iterate through schemas */
      ಪೂರ್ಣಾಂಕ nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pn_ಬೈಟುಗಳು_Freed = &nByte;
      ಪ್ರತಿಜ್ಞಾ( db->lookaside.p_ಕೊನೆ==db->lookaside.p_True_ಕೊನೆ );
      db->lookaside.p_ಕೊನೆ = db->lookaside.p_ಆರಂಭ;
      ಗಾಗಿ(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        ಆದರೆ( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(ಗಾತ್ರ(HashElem)) * (
              pSchema->tblHash.ಎಣಿಕೆ_2 
            + pSchema->trigHash.ಎಣಿಕೆ_2
            + pSchema->idxHash.ಎಣಿಕೆ_2
            + pSchema->fkeyHash.ಎಣಿಕೆ_2
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          ಗಾಗಿ(p=sqlite_Hash_ಮೊದಲ(&pSchema->trigHash); p; p=sqlite_Hash_ಮುಂದಿನ(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqlite_Hash_ದತ್ತಾಂಶ(p));
          }
          ಗಾಗಿ(p=sqlite_Hash_ಮೊದಲ(&pSchema->tblHash); p; p=sqlite_Hash_ಮುಂದಿನ(p)){
            sqlite3_Delete_ಕೋಷ್ಟಕ(db, (ಕೋಷ್ಟಕ *)sqlite_Hash_ದತ್ತಾಂಶ(p));
          }
        }
      }
      db->pn_ಬೈಟುಗಳು_Freed = 0;
      db->lookaside.p_ಕೊನೆ = db->lookaside.p_True_ಕೊನೆ;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      ನಿಲ್ಲಿಸು;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_STMT_ಬಳಸಿದ: {
      ರಚನೆ Vdbe *pVdbe;         /* Used to iterate through VMs */
      ಪೂರ್ಣಾಂಕ nByte = 0;              /* Used to accumulate return value */

      db->pn_ಬೈಟುಗಳು_Freed = &nByte;
      ಪ್ರತಿಜ್ಞಾ( db->lookaside.p_ಕೊನೆ==db->lookaside.p_True_ಕೊನೆ );
      db->lookaside.p_ಕೊನೆ = db->lookaside.p_ಆರಂಭ;
      ಗಾಗಿ(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.p_ಕೊನೆ = db->lookaside.p_True_ಕೊನೆ;
      db->pn_ಬೈಟುಗಳು_Freed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      ನಿಲ್ಲಿಸು;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_CACHE_ದೊರಕಿತು:
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_CACHE_ತಪ್ಪಿದ:
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_CACHE_WRITE:{
      ಪೂರ್ಣಾಂಕ i;
      u64 nRet = 0;
      ಪ್ರತಿಜ್ಞಾ( SQLITE_DBSTATUS_CACHE_ತಪ್ಪಿದ==SQLITE_DBSTATUS_CACHE_ದೊರಕಿತು+1 );
      ಪ್ರತಿಜ್ಞಾ( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_ದೊರಕಿತು+2 );

      ಗಾಗಿ(i=0; i<db->nDb; i++){
        ಆದರೆ( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      ನಿಲ್ಲಿಸು;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      ಆದರೆ( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      ಆದರೆ( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      ನಿಲ್ಲಿಸು;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    ಸ್ಥಿತಿ SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      ನಿಲ್ಲಿಸು;
    }

    ಪೂರ್ವನಿಯೋಜಿತ: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
ಪೂರ್ಣಾಂಕ sqlite3_db_ಸ್ಥಿತಿ(
  sqlite3 *db,             /* The database connection whose status is desired */
  ಪೂರ್ಣಾಂಕ op,                  /* Status verb */
  ಪೂರ್ಣಾಂಕ *pCurrent,           /* Write current value here */
  ಪೂರ್ಣಾಂಕ *pHighwtr,           /* Write high-water mark here */
  ಪೂರ್ಣಾಂಕ resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  ಪೂರ್ಣಾಂಕ rc;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  ಆದರೆ( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  ಹಿಂತಿರುಗಿ rc;
}
