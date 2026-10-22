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
  số_nguyên iCountdown;         /* Number of pending successes before a failure */
  số_nguyên nRepeat;            /* Number of times to repeat the failure */
  số_nguyên nBenign;            /* Number of benign failures seen since last config */
  số_nguyên nFail;              /* Number of failures seen since last config */
  số_nguyên nOkBefore;          /* Successful allocations prior to the first fault */
  số_nguyên nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  số_nguyên isInstalled;        /* True if the fault simulation layer is installed */
  số_nguyên isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
static void sqlite3Fault(void){
  static u64 cnt = 0;
  cnt++;
  nếu( cnt>((u64)1<<63) ) abort();
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
  nếu( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
static số_nguyên faultsimStep(void){
  nếu( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    trả_về 0;
  }
  nếu( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    trả_về 0;
  }
  nếu( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  nếu( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  nếu( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  trả_về 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
static void *faultsimMalloc(số_nguyên n){
  void *p = 0;
  nếu( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  trả_về p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
static void *faultsimRealloc(void *pOld, số_nguyên n){
  void *p = 0;
  nếu( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  trả_về p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
static void faultsimConfig(số_nguyên nDelay, số_nguyên nRepeat){
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
static số_nguyên faultsimFailures(void){
  trả_về memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
static số_nguyên faultsimBenignFailures(void){
  trả_về memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
static số_nguyên faultsimPending(void){
  nếu( memfault.enable ){
    trả_về memfault.iCountdown;
  }không_thì{
    trả_về -1;
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
static số_nguyên faultsimInstall(số_nguyên install){
  số_nguyên rc;

  install = (install ? 1 : 0);
  assert(memfault.isInstalled==1 || memfault.isInstalled==0);

  nếu( install==memfault.isInstalled ){
    trả_về SQLITE_ERROR;
  }

  nếu( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    assert(memfault.m.xMalloc);
    nếu( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }không_thì{
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

  nếu( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  trả_về rc;
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
extern const char *sqlite3ErrName(số_nguyên);

/*
** Transform pointers to text and back again
*/
static void pointerToText(void *p, char *z){
  const char *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
static số_nguyên textToPointer(const char *z, void **pp){
  *pp = sqlite3TestTextToPtr(z);
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
static số_nguyên SQLITE_TCLAPI test_malloc(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên nByte;
  void *p;
  char zOut[100];
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTES");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) trả_về TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, NULL);
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
static số_nguyên SQLITE_TCLAPI test_realloc(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên nByte;
  void *pPrior, *p;
  char zOut[100];
  nếu( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PRIOR NBYTES");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) trả_về TCL_ERROR;
  nếu( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (char*)0);
    trả_về TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, NULL);
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
static số_nguyên SQLITE_TCLAPI test_free(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  void *pPrior;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PRIOR");
    trả_về TCL_ERROR;
  }
  nếu( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (char*)0);
    trả_về TCL_ERROR;
  }
  sqlite3_free(pPrior);
  trả_về TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
số_nguyên sqlite3TestHexToBin(const char *, số_nguyên, char *);
số_nguyên sqlite3TestBinToHex(char*,số_nguyên);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
static số_nguyên SQLITE_TCLAPI test_memset(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  void *p;
  số_nguyên size, i;
  Tcl_Size n;
  char *zHex;
  char *zOut;
  char zBin[100];

  nếu( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "ADDRESS SIZE HEX");
    trả_về TCL_ERROR;
  }
  nếu( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (char*)0);
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    trả_về TCL_ERROR;
  }
  nếu( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (char*)0);
    trả_về TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  nếu( n>sizeof(zBin)*2 ) n = sizeof(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (số_nguyên)n, zBin);
  nếu( n==0 ){
    Tcl_AppendResult(interp, "no data", (char*)0);
    trả_về TCL_ERROR;
  }
  zOut = p;
  cho(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  trả_về TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
static số_nguyên SQLITE_TCLAPI test_memget(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  void *p;
  số_nguyên size, n;
  char *zBin;
  char zHex[100];

  nếu( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "ADDRESS SIZE");
    trả_về TCL_ERROR;
  }
  nếu( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (char*)0);
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    trả_về TCL_ERROR;
  }
  nếu( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (char*)0);
    trả_về TCL_ERROR;
  }
  zBin = p;
  trong_khi( size>0 ){
    nếu( size>(sizeof(zHex)-1)/2 ){
      n = (sizeof(zHex)-1)/2;
    }không_thì{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (char*)0);
  }
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
static số_nguyên SQLITE_TCLAPI test_memory_used(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
static số_nguyên SQLITE_TCLAPI test_memory_highwater(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên resetFlag = 0;
  nếu( objc!=1 && objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "?RESET?");
    trả_về TCL_ERROR;
  }
  nếu( objc==2 ){
    nếu( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) trả_về TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
static số_nguyên SQLITE_TCLAPI test_memdebug_backtrace(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên depth;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DEPT");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &depth) ) trả_về TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    extern void sqlite3MemdebugBacktrace(số_nguyên);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
static số_nguyên SQLITE_TCLAPI test_memdebug_dump(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    trả_về TCL_ERROR;
  }
#nếu defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    extern void sqlite3MemdebugDump(const char*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
static số_nguyên SQLITE_TCLAPI test_memdebug_malloc_count(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên nMalloc = -1;
  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }
#nếu defined(SQLITE_MEMDEBUG)
  {
    extern số_nguyên sqlite3MemdebugMallocCount();
    nMalloc = sqlite3MemdebugMallocCount();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  trả_về TCL_OK;
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
static số_nguyên SQLITE_TCLAPI test_memdebug_fail(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên ii;
  số_nguyên iFail;
  số_nguyên nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  số_nguyên nBenign;
  số_nguyên nFail = 0;

  nếu( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "COUNTER ?OPTIONS?");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) trả_về TCL_ERROR;

  cho(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    char *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    char *zErr = 0;

    nếu( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      nếu( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }không_thì{
        nếu( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          trả_về TCL_ERROR;
        }
      }
    }không_thì nếu( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      nếu( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }không_thì{
        pBenignCnt = objv[ii+1];
      }
    }không_thì{
      zErr = "unknown option: ";
    }

    nếu( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, NULL);
      trả_về TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  nếu( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
static số_nguyên SQLITE_TCLAPI test_memdebug_pending(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên nPending;
  nếu( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    trả_về TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  trả_về TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
static số_nguyên sqlite3_memdebug_title_count = 0;

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
static số_nguyên SQLITE_TCLAPI test_memdebug_settitle(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_count++;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "TITLE");
    trả_về TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    const char *zTitle;
    extern số_nguyên sqlite3MemdebugSettitle(const char*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  trả_về TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((sizeof(số_nguyên)>=sizeof(void*)) ? 1 : sizeof(void*)/sizeof(số_nguyên))   \
)
static Tcl_HashTable aMallocLog;
static số_nguyên mallocLogEnabled = 0;

typedef struct MallocLog MallocLog;
struct MallocLog {
  số_nguyên nCall;
  số_nguyên nByte;
};

#ifdef SQLITE_MEMDEBUG
static void test_memdebug_callback(số_nguyên nByte, số_nguyên nFrame, void **aFrame){
  nếu( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    số_nguyên isNew;

    số_nguyên aKey[MALLOC_LOG_KEYINTS];
    unsigned số_nguyên nKey = sizeof(số_nguyên)*MALLOC_LOG_KEYINTS;

    memset(aKey, 0, nKey);
    nếu( (sizeof(void*)*nFrame)<nKey ){
      nKey = nFrame*sizeof(void*);
    }
    memcpy(aKey, aFrame, nKey);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (const char *)aKey, &isNew);
    nếu( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(sizeof(MallocLog));
      memset(pLog, 0, sizeof(MallocLog));
      Tcl_SetHashValue(pEntry, (ClientData)pLog);
    }không_thì{
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
  cho(
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

static số_nguyên SQLITE_TCLAPI test_memdebug_log(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  static số_nguyên isInit = 0;
  số_nguyên iSub;

  static const char *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  enum MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  nếu( !isInit ){
#ifdef SQLITE_MEMDEBUG
    extern void sqlite3MemdebugBacktraceCallback(
        void (*xBacktrace)(số_nguyên, số_nguyên, void **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  nếu( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
  }
  nếu( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    trả_về TCL_ERROR;
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

      cho(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &search);
        pEntry;
        pEntry=Tcl_NextHashEntry(&search)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_GetHashValue(pEntry);
        Tcl_WideInt *aKey = (Tcl_WideInt *)Tcl_GetHashKey(&aMallocLog, pEntry);
        số_nguyên ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        cho(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
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

  trả_về TCL_OK;
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
static số_nguyên SQLITE_TCLAPI test_config_pagecache(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên sz, N;
  Tcl_Obj *pRes;
  static char *buf = 0;
  nếu( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SIZE N");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &sz) ) trả_về TCL_ERROR;
  nếu( Tcl_GetIntFromObj(interp, objv[2], &N) ) trả_về TCL_ERROR;
  free(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  nếu( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (void*)0, 0, 0);
  }không_thì{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  trả_về TCL_OK;
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
static số_nguyên SQLITE_TCLAPI test_alt_pcache(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên installFlag;
  số_nguyên discardChance = 0;
  số_nguyên prngSeed = 0;
  số_nguyên highStress = 0;
  extern void installTestPCache(số_nguyên,unsigned,unsigned,unsigned);
  nếu( objc<2 || objc>5 ){
    Tcl_WrongNumArgs(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) trả_về TCL_ERROR;
  nếu( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     trả_về TCL_ERROR;
  }
  nếu( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     trả_về TCL_ERROR;
  }
  nếu( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &highStress) ){
    trả_về TCL_ERROR;
  }
  nếu( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (char*)0);
    trả_về TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)highStress);
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
static số_nguyên SQLITE_TCLAPI test_config_memstatus(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên enable, rc;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) trả_về TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
static số_nguyên SQLITE_TCLAPI test_config_lookaside(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên sz, cnt;
  Tcl_Obj *pRet;
  nếu( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SIZE COUNT");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &sz) ) trả_về TCL_ERROR;
  nếu( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) trả_về TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  trả_về TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
static số_nguyên SQLITE_TCLAPI test_db_config_lookaside(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc;
  số_nguyên sz, cnt;
  sqlite3 *db;
  số_nguyên bufid;
  static char azBuf[2][10000];
  extern số_nguyên getDbPointer(Tcl_Interp*, const char*, sqlite3**);
  nếu( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BUFID SIZE COUNT");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;
  nếu( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) trả_về TCL_ERROR;
  nếu( Tcl_GetIntFromObj(interp, objv[3], &sz) ) trả_về TCL_ERROR;
  nếu( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) trả_về TCL_ERROR;
  nếu( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (void*)0, sz, cnt);
  }không_thì nếu( bufid>=1 && bufid<=2 && sz*cnt<=sizeof(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }không_thì{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (char*)0);
    trả_về TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
static số_nguyên SQLITE_TCLAPI test_config_heap(
  void * clientData, 
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  static char *zBuf; /* Use this memory */
  số_nguyên nByte;         /* Size of buffer to pass to sqlite3_config() */
  số_nguyên nMinAlloc;     /* Size of minimum allocation */
  số_nguyên rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  số_nguyên nArg = objc-1;

  nếu( nArg!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTE NMINALLOC");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) trả_về TCL_ERROR;
  nếu( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) trả_về TCL_ERROR;

  nếu( nByte==0 ){
    free( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (void*)0, 0, 0);
  }không_thì{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
static số_nguyên SQLITE_TCLAPI test_config_heap_size(
  void * clientData, 
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên nByte;         /* Size to pass to sqlite3_config() */
  số_nguyên rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  số_nguyên nArg = objc-1;

  nếu( nArg!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTE");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) trả_về TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
static số_nguyên SQLITE_TCLAPI test_config_error(
  void * clientData, 
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  extern số_nguyên getDbPointer(Tcl_Interp*, const char*, sqlite3**);

  nếu( objc!=2 && objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "[DB]");
    trả_về TCL_ERROR;
  }
  nếu( objc==2 ){
    nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;
    nếu( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (char*)0);
      trả_về TCL_ERROR;
    }
  }không_thì{
    nếu( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (char*)0);
      trả_về TCL_ERROR;
    }
  }
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
static số_nguyên SQLITE_TCLAPI test_config_uri(
  void * clientData, 
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc;
  số_nguyên bOpenUri;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    trả_về TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);

  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
static số_nguyên SQLITE_TCLAPI test_config_cis(
  void * clientData, 
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc;
  số_nguyên bUseCis;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    trả_về TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);

  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
static số_nguyên SQLITE_TCLAPI test_config_pmasz(
  void * clientData, 
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc;
  số_nguyên iPmaSz;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    trả_về TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);

  trả_về TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
static số_nguyên SQLITE_TCLAPI test_dump_memsys3(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    trả_về TCL_ERROR;
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
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
static số_nguyên SQLITE_TCLAPI test_status(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc, iValue, mxValue;
  số_nguyên i, op = 0, resetFlag;
  const char *zOpName;
  static const struct {
    const char *zName;
    số_nguyên op;
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
  nếu( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PARAMETER RESETFLAG");
    trả_về TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  cho(i=0; i<ArraySize(aOp); i++){
    nếu( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      break;
    }
  }
  nếu( i>=ArraySize(aOp) ){
    nếu( Tcl_GetIntFromObj(interp, objv[1], &op) ) trả_về TCL_ERROR;
  }
  nếu( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) trả_về TCL_ERROR;
  iValue = 0;
  mxValue = 0;
  rc = sqlite3_status(op, &iValue, &mxValue, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(iValue));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mxValue));
  Tcl_SetObjResult(interp, pResult);
  trả_về TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
static số_nguyên SQLITE_TCLAPI test_db_status(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc, iValue, mxValue;
  số_nguyên i, op = 0, resetFlag;
  const char *zOpName;
  sqlite3 *db;
  extern số_nguyên getDbPointer(Tcl_Interp*, const char*, sqlite3**);
  static const struct {
    const char *zName;
    số_nguyên op;
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
  nếu( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB PARAMETER RESETFLAG");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  nếu( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  nếu( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  cho(i=0; i<ArraySize(aOp); i++){
    nếu( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      break;
    }
  }
  nếu( i>=ArraySize(aOp) ){
    nếu( Tcl_GetIntFromObj(interp, objv[2], &op) ) trả_về TCL_ERROR;
  }
  nếu( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) trả_về TCL_ERROR;
  iValue = 0;
  mxValue = 0;
  rc = sqlite3_db_status(db, op, &iValue, &mxValue, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(iValue));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mxValue));
  Tcl_SetObjResult(interp, pResult);
  trả_về TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
static số_nguyên SQLITE_TCLAPI test_install_malloc_faultsim(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc;
  số_nguyên isInstall;

  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    trả_về TCL_ERROR;
  }
  nếu( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    trả_về TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  trả_về TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
static số_nguyên SQLITE_TCLAPI test_install_memsys3(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  số_nguyên rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  const sqlite3_mem_methods *sqlite3MemGetMemsys3(void);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_VOLATILE);
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI test_vfs_oom_test(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  extern số_nguyên sqlite3_memdebug_vfs_oom_test;
  nếu( objc>2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "?INTEGER?");
    trả_về TCL_ERROR;
  }không_thì nếu( objc==2 ){
    số_nguyên iNew;
    nếu( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) trả_về TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  trả_về TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
số_nguyên Sqlitetest_malloc_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     số_nguyên clientData;
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
  số_nguyên i;
  cho(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  trả_về TCL_OK;
}
#endif
