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
#समावेश "sqliteInt.h"
#समावेश "vdbeInt.h"

/*
** Variables in which to record status information.
*/
#अगर SQLITE_PTRSIZE>4
प्रकार sqlite3_int64 sqlite3_Stat_मूल्य_Type;
#अन्यथा
प्रकार u32 sqlite3_Stat_मूल्य_Type;
#पूर्ण_अगर
प्रकार काष्ठा sqlite3StatType sqlite3StatType;
अटल SQLITE_WSD काष्ठा sqlite3StatType {
  sqlite3_Stat_मूल्य_Type now_मूल्य[10];  /* Current value */
  sqlite3_Stat_मूल्य_Type mx_मूल्य[10];   /* Maximum value */
} sqlite3Stat = { {0,}, {0,} };

/*
** Elements of sqlite3Stat[] are protected by either the memory allocator
** mutex, or by the pcache1 mutex.  The following array determines which.
*/
अटल स्थिर अक्षर statMutex[] = {
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
#अगर_घोषित SQLITE_OMIT_WSD
# define wsdStatInit  sqlite3StatType *x = &GLOBAL(sqlite3StatType,sqlite3Stat)
# define wsdStat x[0]
#अन्यथा
# define wsdStatInit
# define wsdStat sqlite3Stat
#पूर्ण_अगर

/*
** Return the current value of a status parameter.  The caller must
** be holding the appropriate mutex.
*/
sqlite3_int64 sqlite3_Status_मूल्य(पूर्णांक op){
  wsdStatInit;
  निश्चित( op>=0 && op<Array_आकार(wsdStat.now_मूल्य) );
  निश्चित( op>=0 && op<Array_आकार(statMutex) );
  निश्चित( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  वापस wsdStat.now_मूल्य[op];
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
व्योम sqlite3StatusUp(पूर्णांक op, पूर्णांक N){
  wsdStatInit;
  निश्चित( op>=0 && op<Array_आकार(wsdStat.now_मूल्य) );
  निश्चित( op>=0 && op<Array_आकार(statMutex) );
  निश्चित( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  wsdStat.now_मूल्य[op] += N;
  अगर( wsdStat.now_मूल्य[op]>wsdStat.mx_मूल्य[op] ){
    wsdStat.mx_मूल्य[op] = wsdStat.now_मूल्य[op];
  }
}
व्योम sqlite3StatusDown(पूर्णांक op, पूर्णांक N){
  wsdStatInit;
  निश्चित( N>=0 );
  निश्चित( op>=0 && op<Array_आकार(statMutex) );
  निश्चित( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  निश्चित( op>=0 && op<Array_आकार(wsdStat.now_मूल्य) );
  wsdStat.now_मूल्य[op] -= N;
}

/*
** Adjust the highwater mark if necessary.
** The caller must hold the appropriate mutex.
*/
व्योम sqlite3StatusHighwater(पूर्णांक op, पूर्णांक X){
  sqlite3_Stat_मूल्य_Type new_मूल्य;
  wsdStatInit;
  निश्चित( X>=0 );
  new_मूल्य = (sqlite3_Stat_मूल्य_Type)X;
  निश्चित( op>=0 && op<Array_आकार(wsdStat.now_मूल्य) );
  निश्चित( op>=0 && op<Array_आकार(statMutex) );
  निश्चित( sqlite3_mutex_held(statMutex[op] ? sqlite3Pcache1Mutex()
                                           : sqlite3MallocMutex()) );
  निश्चित( op==SQLITE_STATUS_MALLOC_आकार
          || op==SQLITE_STATUS_PAGECACHE_आकार
          || op==SQLITE_STATUS_PARSER_ढेर );
  अगर( new_मूल्य>wsdStat.mx_मूल्य[op] ){
    wsdStat.mx_मूल्य[op] = new_मूल्य;
  }
}

/*
** Query status information.
*/
पूर्णांक sqlite3_status64(
  पूर्णांक op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  पूर्णांक resetFlag
){
  sqlite3_mutex *pMutex;
  wsdStatInit;
  अगर( op<0 || op>=Array_आकार(wsdStat.now_मूल्य) ){
    वापस SQLITE_MISUSE_BKPT;
  }
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCurrent==0 || pHighwater==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  pMutex = statMutex[op] ? sqlite3Pcache1Mutex() : sqlite3MallocMutex();
  sqlite3_mutex_enter(pMutex);
  *pCurrent = wsdStat.now_मूल्य[op];
  *pHighwater = wsdStat.mx_मूल्य[op];
  अगर( resetFlag ){
    wsdStat.mx_मूल्य[op] = wsdStat.now_मूल्य[op];
  }
  sqlite3_mutex_leave(pMutex);
  (व्योम)pMutex;  /* Prevent warning when SQLITE_THREADSAFE=0 */
  वापस SQLITE_OK;
}
पूर्णांक sqlite3_status(पूर्णांक op, पूर्णांक *pCurrent, पूर्णांक *pHighwater, पूर्णांक resetFlag){
  sqlite3_int64 iCur = 0, iHwtr = 0;
  पूर्णांक rc;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( pCurrent==0 || pHighwater==0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर
  rc = sqlite3_status64(op, &iCur, &iHwtr, resetFlag);
  अगर( rc==0 ){
    *pCurrent = (पूर्णांक)iCur;
    *pHighwater = (पूर्णांक)iHwtr;
  }
  वापस rc;
}

/*
** Return the number of LookasideSlot elements on the linked list
*/
अटल u32 गिनती_Lookaside_Slots(LookasideSlot *p){
  u32 cnt = 0;
  जबतक( p ){
    p = p->p_अग्र;
    cnt++;
  }
  वापस cnt;
}

/*
** Count the number of slots of lookaside memory that are outstanding
*/
पूर्णांक sqlite3LookasideUsed(sqlite3 *db, पूर्णांक *pHighwater){
  u32 nInit = गिनती_Lookaside_Slots(db->lookaside.pInit);
  u32 nFree = गिनती_Lookaside_Slots(db->lookaside.pFree);
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
  nInit += गिनती_Lookaside_Slots(db->lookaside.pSmallInit);
  nFree += गिनती_Lookaside_Slots(db->lookaside.pSmallFree);
#पूर्ण_अगर /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  निश्चित( db->lookaside.nSlot >= nInit+nFree );
  अगर( pHighwater ) *pHighwater = (पूर्णांक)(db->lookaside.nSlot - nInit);
  वापस (पूर्णांक)(db->lookaside.nSlot - (nInit+nFree));
}

/*
** Query status information for a single database connection
*/
पूर्णांक sqlite3_db_status64(
  sqlite3 *db,             /* The database connection whose status is desired */
  पूर्णांक op,                  /* Status verb */
  sqlite3_int64 *pCurrent, /* Write current value here */
  sqlite3_int64 *pHighwtr, /* Write high-water mark here */
  पूर्णांक resetFlag            /* Reset high-water mark if true */
){
  पूर्णांक rc = SQLITE_OK;   /* Return code */
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);
  चयन( op ){
    हाल SQLITE_DBSTATUS_LOOKASIDE_USED: {
      पूर्णांक H = 0;
      *pCurrent = sqlite3LookasideUsed(db, &H);
      *pHighwtr = H;
      अगर( resetFlag ){
        LookasideSlot *p = db->lookaside.pFree;
        अगर( p ){
          जबतक( p->p_अग्र ) p = p->p_अग्र;
          p->p_अग्र = db->lookaside.pInit;
          db->lookaside.pInit = db->lookaside.pFree;
          db->lookaside.pFree = 0;
        }
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
        p = db->lookaside.pSmallFree;
        अगर( p ){
          जबतक( p->p_अग्र ) p = p->p_अग्र;
          p->p_अग्र = db->lookaside.pSmallInit;
          db->lookaside.pSmallInit = db->lookaside.pSmallFree;
          db->lookaside.pSmallFree = 0;
        }
#पूर्ण_अगर
      }
      अवरोध;
    }

    हाल SQLITE_DBSTATUS_LOOKASIDE_HIT:
    हाल SQLITE_DBSTATUS_LOOKASIDE_MISS_आकार:
    हाल SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL: {
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_HIT );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_आकार );
      testcase( op==SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL );
      निश्चित( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)>=0 );
      निश्चित( (op-SQLITE_DBSTATUS_LOOKASIDE_HIT)<3 );
      *pCurrent = 0;
      *pHighwtr = db->lookaside.anStat[op-SQLITE_DBSTATUS_LOOKASIDE_HIT];
      अगर( resetFlag ){
        db->lookaside.anStat[op - SQLITE_DBSTATUS_LOOKASIDE_HIT] = 0;
      }
      अवरोध;
    }

    /* 
    ** Return an approximation for the amount of memory currently used
    ** by all pagers associated with the given database connection.  The
    ** highwater mark is meaningless and is returned as zero.
    */
    हाल SQLITE_DBSTATUS_CACHE_USED_SHARED:
    हाल SQLITE_DBSTATUS_CACHE_USED: {
      sqlite3_int64 योग_Used = 0;
      पूर्णांक i;
      sqlite3BtreeEnterAll(db);
      क्रम(i=0; i<db->nDb; i++){
        Btree *pBt = db->aDb[i].pBt;
        अगर( pBt ){
          Pager *pPager = sqlite3BtreePager(pBt);
          पूर्णांक nByte = sqlite3PagerMemUsed(pPager);
          अगर( op==SQLITE_DBSTATUS_CACHE_USED_SHARED ){
            nByte = nByte / sqlite3_Btree_Connection_गिनती(pBt);
          }
          योग_Used += nByte;
        }
      }
      sqlite3BtreeLeaveAll(db);
      *pCurrent = योग_Used;
      *pHighwtr = 0;
      अवरोध;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store the schema for all databases (main, temp, and any ATTACHed
    ** databases.  *pHighwtr is set to zero.
    */
    हाल SQLITE_DBSTATUS_SCHEMA_USED: {
      पूर्णांक i;          /* Used to iterate through schemas */
      पूर्णांक nByte = 0;  /* Used to accumulate return value */

      sqlite3BtreeEnterAll(db);
      db->pnBytesFreed = &nByte;
      निश्चित( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      क्रम(i=0; i<db->nDb; i++){
        Schema *pSchema = db->aDb[i].pSchema;
        अगर( ALWAYS(pSchema!=0) ){
          HashElem *p;

          nByte += sqlite3GlobalConfig.m.xRoundup(माप(HashElem)) * (
              pSchema->tblHash.गिनती 
            + pSchema->trigHash.गिनती
            + pSchema->idxHash.गिनती
            + pSchema->fkeyHash.गिनती
          );
          nByte += sqlite3_msize(pSchema->tblHash.ht);
          nByte += sqlite3_msize(pSchema->trigHash.ht);
          nByte += sqlite3_msize(pSchema->idxHash.ht);
          nByte += sqlite3_msize(pSchema->fkeyHash.ht);

          क्रम(p=sqliteHashFirst(&pSchema->trigHash); p; p=sqlite_Hash_अग्र(p)){
            sqlite3DeleteTrigger(db, (Trigger*)sqlite_Hash_तथ्य(p));
          }
          क्रम(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqlite_Hash_अग्र(p)){
            sqlite3DeleteTable(db, (Table *)sqlite_Hash_तथ्य(p));
          }
        }
      }
      db->pnBytesFreed = 0;
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      sqlite3BtreeLeaveAll(db);

      *pHighwtr = 0;
      *pCurrent = nByte;
      अवरोध;
    }

    /*
    ** *pCurrent gets an accurate estimate of the amount of memory used
    ** to store all prepared statements.
    ** *pHighwtr is set to zero.
    */
    हाल SQLITE_DBSTATUS_STMT_USED: {
      काष्ठा Vdbe *pVdbe;         /* Used to iterate through VMs */
      पूर्णांक nByte = 0;              /* Used to accumulate return value */

      db->pnBytesFreed = &nByte;
      निश्चित( db->lookaside.pEnd==db->lookaside.pTrueEnd );
      db->lookaside.pEnd = db->lookaside.pStart;
      क्रम(pVdbe=db->pVdbe; pVdbe; pVdbe=pVdbe->pVNext){
        sqlite3VdbeDelete(pVdbe);
      }
      db->lookaside.pEnd = db->lookaside.pTrueEnd;
      db->pnBytesFreed = 0;

      *pHighwtr = 0;  /* IMP: R-64479-57858 */
      *pCurrent = nByte;

      अवरोध;
    }

    /*
    ** Set *pCurrent to the total cache hits or misses encountered by all
    ** pagers the database handle is connected to. *pHighwtr is always set 
    ** to zero.
    */
    हाल SQLITE_DBSTATUS_CACHE_SPILL:
      op = SQLITE_DBSTATUS_CACHE_WRITE+1;
      /* no break */ deliberate_fall_through
    हाल SQLITE_DBSTATUS_CACHE_HIT:
    हाल SQLITE_DBSTATUS_CACHE_MISS:
    हाल SQLITE_DBSTATUS_CACHE_WRITE:{
      पूर्णांक i;
      u64 nRet = 0;
      निश्चित( SQLITE_DBSTATUS_CACHE_MISS==SQLITE_DBSTATUS_CACHE_HIT+1 );
      निश्चित( SQLITE_DBSTATUS_CACHE_WRITE==SQLITE_DBSTATUS_CACHE_HIT+2 );

      क्रम(i=0; i<db->nDb; i++){
        अगर( db->aDb[i].pBt ){
          Pager *pPager = sqlite3BtreePager(db->aDb[i].pBt);
          sqlite3PagerCacheStat(pPager, op, resetFlag, &nRet);
        }
      }
      *pHighwtr = 0; /* IMP: R-42420-56072 */
                       /* IMP: R-54100-20147 */
                       /* IMP: R-29431-39229 */
      *pCurrent = nRet;
      अवरोध;
    }

    /* Set *pCurrent to the number of bytes that the db database connection
    ** has spilled to the filesystem in temporary files that could have been
    ** stored in memory, had sufficient memory been available.
    ** The *pHighwater is always set to zero.
    */
    हाल SQLITE_DBSTATUS_TEMPBUF_SPILL: {
      u64 nRet = 0;
      अगर( db->aDb[1].pBt ){
        Pager *pPager = sqlite3BtreePager(db->aDb[1].pBt);
        sqlite3PagerCacheStat(pPager, SQLITE_DBSTATUS_CACHE_WRITE,
                              resetFlag, &nRet);
        nRet *= sqlite3_Btree_Get_Page_आकार(db->aDb[1].pBt);
      }
      nRet += db->nSpill;
      अगर( resetFlag ) db->nSpill = 0;
      *pHighwtr = 0;
      *pCurrent = nRet;
      अवरोध;
    }

    /* Set *pCurrent to non-zero if there are unresolved deferred foreign
    ** key constraints.  Set *pCurrent to zero if all foreign key constraints
    ** have been satisfied.  The *pHighwtr is always set to zero.
    */
    हाल SQLITE_DBSTATUS_DEFERRED_FKS: {
      *pHighwtr = 0;  /* IMP: R-11967-56545 */
      *pCurrent = db->nDeferredImmCons>0 || db->nDeferredCons>0;
      अवरोध;
    }

    शेष: {
      rc = SQLITE_ERROR;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** 32-bit variant of sqlite3_db_status64()
*/
पूर्णांक sqlite3_db_status(
  sqlite3 *db,             /* The database connection whose status is desired */
  पूर्णांक op,                  /* Status verb */
  पूर्णांक *pCurrent,           /* Write current value here */
  पूर्णांक *pHighwtr,           /* Write high-water mark here */
  पूर्णांक resetFlag            /* Reset high-water mark if true */
){
  sqlite3_int64 C = 0, H = 0;
  पूर्णांक rc;
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) || pCurrent==0|| pHighwtr==0 ){
    वापस SQLITE_MISUSE_BKPT;
  }
#पूर्ण_अगर
  rc = sqlite3_db_status64(db, op, &C, &H, resetFlag);
  अगर( rc==0 ){
    *pCurrent = C & 0x7fffffff;
    *pHighwtr = H & 0x7fffffff;
  }
  वापस rc;
}
