/*
** 2007 August 15
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
** This file contains code used to implement test interfaces to the
** memory allocation subsystem.
*/
#include "sqliteInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
static struct MemFault {
  ціле iCountdown;         /* Number of pending successes before a failure */
  ціле nRepeat;            /* Number of times to repeat the failure */
  ціле nBenign;            /* Number of benign failures seen since last config */
  ціле nFail;              /* Number of failures seen since last config */
  ціле nOkBefore;          /* Successful allocations prior to the first fault */
  ціле nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  ціле isInstalled;        /* True if the fault simulation layer is installed */
  ціле isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
static void sqlite3Fault(void){
  static u64 cnt = 0;
  cnt++;
  якщо( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
static void sqlite3FirstFault(void){
  static u64 cnt2 = 0;
  cnt2++;
  якщо( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
static ціле faultsimStep(void){
  якщо( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    повернути 0;
  }
  якщо( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    повернути 0;
  }
  якщо( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  якщо( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  якщо( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  повернути 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
static void *faultsimMalloc(ціле n){
  void *p = 0;
  якщо( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  повернути p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
static void *faultsimRealloc(void *pOld, ціле n){
  void *p = 0;
  якщо( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  повернути p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
static void faultsimConfig(ціле nDelay, ціле nRepeat){
  memfault.iCountdown = nDelay;
  memfault.nRepeat = nRepeat;
  memfault.nBenign = 0;
  memfault.nFail = 0;
  memfault.nOkBefore = 0;
  memfault.nOkAfter = 0;
  memfault.enable = nDelay>=0;

  /* Sometimes, when running multi-threaded tests, the isBenignMode 
  ** variable is not properly incremented/decremented so that it is
  ** 0 when not inside a benign malloc block. This doesn't affect
  ** the multi-threaded tests, as they do not use this system. But
  ** it does affect OOM tests run later in the same process. So
  ** zero the variable here, just to be sure.
  */
  memfault.isBenignMode = 0;
}

/*
** Return the number of faults (both hard and benign faults) that have
** occurred since the injector was last configured.
*/
static ціле faultsimFailures(void){
  повернути memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
static ціле faultsimBenignFailures(void){
  повернути memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
static ціле faultsimPending(void){
  якщо( memfault.enable ){
    повернути memfault.iCountdown;
  }інакше{
    повернути -1;
  }
}


static void faultsimBeginBenign(void){
  memfault.isBenignMode++;
}
static void faultsimEndBenign(void){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
static ціле faultsimInstall(ціле install){
  ціле rc;

  install = (install ? 1 : 0);
  assert(memfault.isInstalled==1 || memfault.isInstalled==0);

  якщо( install==memfault.isInstalled ){
    повернути SQLITE_ERROR;
  }

  якщо( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    assert(memfault.m.xMalloc);
    якщо( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }інакше{
    sqlite3_mem_methods m2;
    assert(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, sizeof(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    assert( memcmp(&m2, &memfault.m, sizeof(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (void*)0, (void*)0);
  }

  якщо( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  повернути rc;
}

#ifdef SQLITE_TEST

/*
** This function is implemented in main.c. Returns a pointer to a static
** buffer containing the symbolic SQLite error code that corresponds to
** the least-significant 8-bits of the integer passed as an argument.
** For example:
**
**   sqlite3ErrName(1) -> "SQLITE_ERROR"
*/
extern const char *sqlite3ErrName(ціле);

/*
** Transform pointers to text and back again
*/
static void pointerToText(void *p, char *z){
  const char *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
static ціле textToPointer(const char *z, void **pp){
  *pp = sqlite3TestTextToPtr(z);
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
static ціле SQLITE_TCLAPI test_malloc(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле nByte;
  void *p;
  char zOut[100];
  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTES");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) повернути TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, NULL);
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
static ціле SQLITE_TCLAPI test_realloc(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле nByte;
  void *pPrior, *p;
  char zOut[100];
  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PRIOR NBYTES");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) повернути TCL_ERROR;
  якщо( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (char*)0);
    повернути TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, NULL);
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
static ціле SQLITE_TCLAPI test_free(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  void *pPrior;
  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PRIOR");
    повернути TCL_ERROR;
  }
  якщо( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (char*)0);
    повернути TCL_ERROR;
  }
  sqlite3_free(pPrior);
  повернути TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
ціле sqlite3TestHexToBin(const char *, ціле, char *);
ціле sqlite3TestBinToHex(char*,ціле);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
static ціле SQLITE_TCLAPI test_memset(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  void *p;
  ціле size, i;
  Tcl_Size n;
  char *zHex;
  char *zOut;
  char zBin[100];

  якщо( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "ADDRESS SIZE HEX");
    повернути TCL_ERROR;
  }
  якщо( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (char*)0);
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    повернути TCL_ERROR;
  }
  якщо( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (char*)0);
    повернути TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  якщо( n>sizeof(zBin)*2 ) n = sizeof(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (ціле)n, zBin);
  якщо( n==0 ){
    Tcl_AppendResult(interp, "no data", (char*)0);
    повернути TCL_ERROR;
  }
  zOut = p;
  для(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  повернути TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
static ціле SQLITE_TCLAPI test_memget(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  void *p;
  ціле size, n;
  char *zBin;
  char zHex[100];

  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "ADDRESS SIZE");
    повернути TCL_ERROR;
  }
  якщо( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (char*)0);
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    повернути TCL_ERROR;
  }
  якщо( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (char*)0);
    повернути TCL_ERROR;
  }
  zBin = p;
  поки( size>0 ){
    якщо( size>(sizeof(zHex)-1)/2 ){
      n = (sizeof(zHex)-1)/2;
    }інакше{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (char*)0);
  }
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
static ціле SQLITE_TCLAPI test_memory_used(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
static ціле SQLITE_TCLAPI test_memory_highwater(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле resetFlag = 0;
  якщо( objc!=1 && objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "?RESET?");
    повернути TCL_ERROR;
  }
  якщо( objc==2 ){
    якщо( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) повернути TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
static ціле SQLITE_TCLAPI test_memdebug_backtrace(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле depth;
  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DEPT");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[1], &depth) ) повернути TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    extern void sqlite3MemdebugBacktrace(ціле);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
static ціле SQLITE_TCLAPI test_memdebug_dump(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    повернути TCL_ERROR;
  }
#якщо defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    extern void sqlite3MemdebugDump(const char*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
static ціле SQLITE_TCLAPI test_memdebug_malloc_count(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле nMalloc = -1;
  якщо( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    повернути TCL_ERROR;
  }
#якщо defined(SQLITE_MEMDEBUG)
  {
    extern ціле sqlite3MemdebugMallocCount();
    nMalloc = sqlite3MemdebugMallocCount();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  повернути TCL_OK;
}


/*
** Usage:    sqlite3_memdebug_fail  COUNTER  ?OPTIONS?
**
** where options are:
**
**     -repeat    <count>
**     -benigncnt <varname>
**
** Arrange for a simulated malloc() failure after COUNTER successes.
** If a repeat count is specified, the fault is repeated that many
** times.
**
** Each call to this routine overrides the prior counter value.
** This routine returns the number of simulated failures that have
** happened since the previous call to this routine.
**
** To disable simulated failures, use a COUNTER of -1.
*/
static ціле SQLITE_TCLAPI test_memdebug_fail(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле ii;
  ціле iFail;
  ціле nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  ціле nBenign;
  ціле nFail = 0;

  якщо( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "COUNTER ?OPTIONS?");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) повернути TCL_ERROR;

  для(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    char *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    char *zErr = 0;

    якщо( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      якщо( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }інакше{
        якщо( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          повернути TCL_ERROR;
        }
      }
    }інакше якщо( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      якщо( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }інакше{
        pBenignCnt = objv[ii+1];
      }
    }інакше{
      zErr = "unknown option: ";
    }

    якщо( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, NULL);
      повернути TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  якщо( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
static ціле SQLITE_TCLAPI test_memdebug_pending(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле nPending;
  якщо( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    повернути TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  повернути TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
static ціле sqlite3_memdebug_title_count = 0;

/*
** Usage:    sqlite3_memdebug_settitle TITLE
**
** Set a title string stored with each allocation.  The TITLE is
** typically the name of the test that was running when the
** allocation occurred.  The TITLE is stored with the allocation
** and can be used to figure out which tests are leaking memory.
**
** Each title overwrite the previous.
*/
static ціле SQLITE_TCLAPI test_memdebug_settitle(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_count++;
  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "TITLE");
    повернути TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    const char *zTitle;
    extern ціле sqlite3MemdebugSettitle(const char*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  повернути TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((sizeof(ціле)>=sizeof(void*)) ? 1 : sizeof(void*)/sizeof(ціле))   \
)
static Tcl_HashTable aMallocLog;
static ціле mallocLogEnabled = 0;

typedef struct MallocLog MallocLog;
struct MallocLog {
  ціле nCall;
  ціле nByte;
};

#ifdef SQLITE_MEMDEBUG
static void test_memdebug_callback(ціле nByte, ціле nFrame, void **aFrame){
  якщо( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    ціле isNew;

    ціле aKey[MALLOC_LOG_KEYINTS];
    unsigned ціле nKey = sizeof(ціле)*MALLOC_LOG_KEYINTS;

    memset(aKey, 0, nKey);
    якщо( (sizeof(void*)*nFrame)<nKey ){
      nKey = nFrame*sizeof(void*);
    }
    memcpy(aKey, aFrame, nKey);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (const char *)aKey, &isNew);
    якщо( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(sizeof(MallocLog));
      memset(pLog, 0, sizeof(MallocLog));
      Tcl_SetHashValue(pEntry, (ClientData)pLog);
    }інакше{
      pLog = (MallocLog *)Tcl_GetHashValue(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

static void test_memdebug_log_clear(void){
  Tcl_HashSearch search;
  Tcl_HashEntry *pEntry;
  для(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &search);
    pEntry;
    pEntry=Tcl_NextHashEntry(&search)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_GetHashValue(pEntry);
    Tcl_Free((char *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

static ціле SQLITE_TCLAPI test_memdebug_log(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  static ціле isInit = 0;
  ціле iSub;

  static const char *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  enum MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  якщо( !isInit ){
#ifdef SQLITE_MEMDEBUG
    extern void sqlite3MemdebugBacktraceCallback(
        void (*xBacktrace)(ціле, ціле, void **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  якщо( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
  }
  якщо( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    повернути TCL_ERROR;
  }

  switch( (enum MB_enum)iSub ){
    case MB_LOG_START:
      mallocLogEnabled = 1;
      break;
    case MB_LOG_STOP:
      mallocLogEnabled = 0;
      break;
    case MB_LOG_DUMP: {
      Tcl_HashSearch search;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      assert(sizeof(Tcl_WideInt)>=sizeof(void*));

      для(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &search);
        pEntry;
        pEntry=Tcl_NextHashEntry(&search)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_GetHashValue(pEntry);
        Tcl_WideInt *aKey = (Tcl_WideInt *)Tcl_GetHashKey(&aMallocLog, pEntry);
        ціле ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        для(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(aKey[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      break;
    }
    case MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      break;
    }

    case MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      extern void sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      break;
    }
  }

  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_pagecache SIZE N
**
** Set the page-cache memory buffer using SQLITE_CONFIG_PAGECACHE.
** The buffer is static and is of limited size.  N might be
** adjusted downward as needed to accommodate the requested size.
** The revised value of N is returned.
**
** A negative SIZE causes the buffer pointer to be NULL.
*/
static ціле SQLITE_TCLAPI test_config_pagecache(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле sz, N;
  Tcl_Obj *pRes;
  static char *buf = 0;
  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SIZE N");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[1], &sz) ) повернути TCL_ERROR;
  якщо( Tcl_GetIntFromObj(interp, objv[2], &N) ) повернути TCL_ERROR;
  free(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  якщо( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (void*)0, 0, 0);
  }інакше{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_alt_pcache INSTALL_FLAG DISCARD_CHANCE PRNG_SEED
**
** Set up the alternative test page cache.  Install if INSTALL_FLAG is
** true and uninstall (reverting to the default page cache) if INSTALL_FLAG
** is false.  DISCARD_CHANGE is an integer between 0 and 100 inclusive
** which determines the chance of discarding a page when unpinned.  100
** is certainty.  0 is never.  PRNG_SEED is the pseudo-random number generator
** seed.
*/
static ціле SQLITE_TCLAPI test_alt_pcache(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле installFlag;
  ціле discardChance = 0;
  ціле prngSeed = 0;
  ціле highStress = 0;
  extern void installTestPCache(ціле,unsigned,unsigned,unsigned);
  якщо( objc<2 || objc>5 ){
    Tcl_WrongNumArgs(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) повернути TCL_ERROR;
  якщо( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     повернути TCL_ERROR;
  }
  якщо( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     повернути TCL_ERROR;
  }
  якщо( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &highStress) ){
    повернути TCL_ERROR;
  }
  якщо( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (char*)0);
    повернути TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)highStress);
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
static ціле SQLITE_TCLAPI test_config_memstatus(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле enable, rc;
  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) повернути TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
static ціле SQLITE_TCLAPI test_config_lookaside(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле sz, cnt;
  Tcl_Obj *pRet;
  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SIZE COUNT");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[1], &sz) ) повернути TCL_ERROR;
  якщо( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) повернути TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  повернути TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
static ціле SQLITE_TCLAPI test_db_config_lookaside(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc;
  ціле sz, cnt;
  sqlite3 *db;
  ціле bufid;
  static char azBuf[2][10000];
  extern ціле getDbPointer(Tcl_Interp*, const char*, sqlite3**);
  якщо( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BUFID SIZE COUNT");
    повернути TCL_ERROR;
  }
  якщо( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) повернути TCL_ERROR;
  якщо( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) повернути TCL_ERROR;
  якщо( Tcl_GetIntFromObj(interp, objv[3], &sz) ) повернути TCL_ERROR;
  якщо( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) повернути TCL_ERROR;
  якщо( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (void*)0, sz, cnt);
  }інакше якщо( bufid>=1 && bufid<=2 && sz*cnt<=sizeof(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }інакше{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (char*)0);
    повернути TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
static ціле SQLITE_TCLAPI test_config_heap(
  void * clientData, 
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  static char *zBuf; /* Use this memory */
  ціле nByte;         /* Size of buffer to pass to sqlite3_config() */
  ціле nMinAlloc;     /* Size of minimum allocation */
  ціле rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ціле nArg = objc-1;

  якщо( nArg!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTE NMINALLOC");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) повернути TCL_ERROR;
  якщо( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) повернути TCL_ERROR;

  якщо( nByte==0 ){
    free( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (void*)0, 0, 0);
  }інакше{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
static ціле SQLITE_TCLAPI test_config_heap_size(
  void * clientData, 
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле nByte;         /* Size to pass to sqlite3_config() */
  ціле rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ціле nArg = objc-1;

  якщо( nArg!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTE");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) повернути TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
static ціле SQLITE_TCLAPI test_config_error(
  void * clientData, 
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  extern ціле getDbPointer(Tcl_Interp*, const char*, sqlite3**);

  якщо( objc!=2 && objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "[DB]");
    повернути TCL_ERROR;
  }
  якщо( objc==2 ){
    якщо( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) повернути TCL_ERROR;
    якщо( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (char*)0);
      повернути TCL_ERROR;
    }
  }інакше{
    якщо( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (char*)0);
      повернути TCL_ERROR;
    }
  }
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
static ціле SQLITE_TCLAPI test_config_uri(
  void * clientData, 
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc;
  ціле bOpenUri;

  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    повернути TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);

  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
static ціле SQLITE_TCLAPI test_config_cis(
  void * clientData, 
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc;
  ціле bUseCis;

  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    повернути TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);

  повернути TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
static ціле SQLITE_TCLAPI test_config_pmasz(
  void * clientData, 
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc;
  ціле iPmaSz;

  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    повернути TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);

  повернути TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
static ціле SQLITE_TCLAPI test_dump_memsys3(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    повернути TCL_ERROR;
  }

  switch( SQLITE_PTR_TO_INT(clientData) ){
    case 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      extern void sqlite3Memsys3Dump(const char*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      break;
#endif
    }
    case 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      extern void sqlite3Memsys5Dump(const char*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      break;
#endif
    }
  }
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
static ціле SQLITE_TCLAPI test_status(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc, iValue, mxValue;
  ціле i, op = 0, resetFlag;
  const char *zOpName;
  static const struct {
    const char *zName;
    ціле op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_STACK        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_COUNT        },
  };
  Tcl_Obj *pResult;
  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PARAMETER RESETFLAG");
    повернути TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  для(i=0; i<ArraySize(aOp); i++){
    якщо( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      break;
    }
  }
  якщо( i>=ArraySize(aOp) ){
    якщо( Tcl_GetIntFromObj(interp, objv[1], &op) ) повернути TCL_ERROR;
  }
  якщо( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) повернути TCL_ERROR;
  iValue = 0;
  mxValue = 0;
  rc = sqlite3_status(op, &iValue, &mxValue, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(iValue));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mxValue));
  Tcl_SetObjResult(interp, pResult);
  повернути TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
static ціле SQLITE_TCLAPI test_db_status(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc, iValue, mxValue;
  ціле i, op = 0, resetFlag;
  const char *zOpName;
  sqlite3 *db;
  extern ціле getDbPointer(Tcl_Interp*, const char*, sqlite3**);
  static const struct {
    const char *zName;
    ціле op;
  } aOp[] = {
    { "LOOKASIDE_USED",      SQLITE_DBSTATUS_LOOKASIDE_USED      },
    { "CACHE_USED",          SQLITE_DBSTATUS_CACHE_USED          },
    { "SCHEMA_USED",         SQLITE_DBSTATUS_SCHEMA_USED         },
    { "STMT_USED",           SQLITE_DBSTATUS_STMT_USED           },
    { "LOOKASIDE_HIT",       SQLITE_DBSTATUS_LOOKASIDE_HIT       },
    { "LOOKASIDE_MISS_SIZE", SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE },
    { "LOOKASIDE_MISS_FULL", SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL },
    { "CACHE_HIT",           SQLITE_DBSTATUS_CACHE_HIT           },
    { "CACHE_MISS",          SQLITE_DBSTATUS_CACHE_MISS          },
    { "CACHE_WRITE",         SQLITE_DBSTATUS_CACHE_WRITE         },
    { "DEFERRED_FKS",        SQLITE_DBSTATUS_DEFERRED_FKS        },
    { "CACHE_USED_SHARED",   SQLITE_DBSTATUS_CACHE_USED_SHARED   },
    { "CACHE_SPILL",         SQLITE_DBSTATUS_CACHE_SPILL         },
    { "TEMPBUF_SPILL",       SQLITE_DBSTATUS_TEMPBUF_SPILL       },
  };
  Tcl_Obj *pResult;
  якщо( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB PARAMETER RESETFLAG");
    повернути TCL_ERROR;
  }
  якщо( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) повернути TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  якщо( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  якщо( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  для(i=0; i<ArraySize(aOp); i++){
    якщо( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      break;
    }
  }
  якщо( i>=ArraySize(aOp) ){
    якщо( Tcl_GetIntFromObj(interp, objv[2], &op) ) повернути TCL_ERROR;
  }
  якщо( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) повернути TCL_ERROR;
  iValue = 0;
  mxValue = 0;
  rc = sqlite3_db_status(db, op, &iValue, &mxValue, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(iValue));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mxValue));
  Tcl_SetObjResult(interp, pResult);
  повернути TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
static ціле SQLITE_TCLAPI test_install_malloc_faultsim(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc;
  ціле isInstall;

  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    повернути TCL_ERROR;
  }
  якщо( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    повернути TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  повернути TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
static ціле SQLITE_TCLAPI test_install_memsys3(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  const sqlite3_mem_methods *sqlite3MemGetMemsys3(void);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_vfs_oom_test(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  extern ціле sqlite3_memdebug_vfs_oom_test;
  якщо( objc>2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "?INTEGER?");
    повернути TCL_ERROR;
  }інакше якщо( objc==2 ){
    ціле iNew;
    якщо( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) повернути TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  повернути TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ціле Sqlitetest_malloc_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     ціле clientData;
  } aObjCmd[] = {
     { "sqlite3_malloc",             test_malloc                   ,0 },
     { "sqlite3_realloc",            test_realloc                  ,0 },
     { "sqlite3_free",               test_free                     ,0 },
     { "memset",                     test_memset                   ,0 },
     { "memget",                     test_memget                   ,0 },
     { "sqlite3_memory_used",        test_memory_used              ,0 },
     { "sqlite3_memory_highwater",   test_memory_highwater         ,0 },
     { "sqlite3_memdebug_backtrace", test_memdebug_backtrace       ,0 },
     { "sqlite3_memdebug_dump",      test_memdebug_dump            ,0 },
     { "sqlite3_memdebug_fail",      test_memdebug_fail            ,0 },
     { "sqlite3_memdebug_pending",   test_memdebug_pending         ,0 },
     { "sqlite3_memdebug_settitle",  test_memdebug_settitle        ,0 },
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_count ,0 },
     { "sqlite3_memdebug_log",       test_memdebug_log             ,0 },
     { "sqlite3_config_pagecache",   test_config_pagecache         ,0 },
     { "sqlite3_config_alt_pcache",  test_alt_pcache               ,0 },
     { "sqlite3_status",             test_status                   ,0 },
     { "sqlite3_db_status",          test_db_status                ,0 },
     { "install_malloc_faultsim",    test_install_malloc_faultsim  ,0 },
     { "sqlite3_config_heap",        test_config_heap              ,0 },
     { "sqlite3_config_heap_size",   test_config_heap_size         ,0 },
     { "sqlite3_config_memstatus",   test_config_memstatus         ,0 },
     { "sqlite3_config_lookaside",   test_config_lookaside         ,0 },
     { "sqlite3_config_error",       test_config_error             ,0 },
     { "sqlite3_config_uri",         test_config_uri               ,0 },
     { "sqlite3_config_cis",         test_config_cis               ,0 },
     { "sqlite3_config_pmasz",       test_config_pmasz             ,0 },
     { "sqlite3_db_config_lookaside",test_db_config_lookaside      ,0 },
     { "sqlite3_dump_memsys3",       test_dump_memsys3             ,3 },
     { "sqlite3_dump_memsys5",       test_dump_memsys3             ,5 },
     { "sqlite3_install_memsys3",    test_install_memsys3          ,0 },
     { "sqlite3_memdebug_vfs_oom_test", test_vfs_oom_test          ,0 },
  };
  ціле i;
  для(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  повернути TCL_OK;
}
#endif
