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
static sa_nracanaa MemFault {
  puur_naa_nka iCountdown;         /* Number of pending successes before a failure */
  puur_naa_nka nRepeat;            /* Number of times to repeat the failure */
  puur_naa_nka nBenign;            /* Number of benign failures seen since last config */
  puur_naa_nka nFail;              /* Number of failures seen since last config */
  puur_naa_nka nOkBefore;          /* Successful allocations prior to the first fault */
  puur_naa_nka nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  puur_naa_nka isInstalled;        /* True if the fault simulation layer is installed */
  puur_naa_nka isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
static void sqlite3Fault(void){
  static u64 cnt = 0;
  cnt++;
  ᰀᰧ( cnt>((u64)1<<63) ) abort();
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
  ᰀᰧ( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
static puur_naa_nka faultsimStep(void){
  ᰀᰧ( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    ᰀᰧᰌ 0;
  }
  ᰀᰧ( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    ᰀᰧᰌ 0;
  }
  ᰀᰧ( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  ᰀᰧ( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  ᰀᰧ( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  ᰀᰧᰌ 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
static void *faultsimMalloc(puur_naa_nka n){
  void *p = 0;
  ᰀᰧ( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  ᰀᰧᰌ p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
static void *faultsimRealloc(void *pOld, puur_naa_nka n){
  void *p = 0;
  ᰀᰧ( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  ᰀᰧᰌ p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
static void faultsimConfig(puur_naa_nka nDelay, puur_naa_nka nRepeat){
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
static puur_naa_nka faultsimFailures(void){
  ᰀᰧᰌ memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
static puur_naa_nka faultsimBenignFailures(void){
  ᰀᰧᰌ memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
static puur_naa_nka faultsimPending(void){
  ᰀᰧ( memfault.enable ){
    ᰀᰧᰌ memfault.iCountdown;
  }ᰀᰧᰀ{
    ᰀᰧᰌ -1;
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
static puur_naa_nka faultsimInstall(puur_naa_nka install){
  puur_naa_nka rc;

  install = (install ? 1 : 0);
  assert(memfault.isInstalled==1 || memfault.isInstalled==0);

  ᰀᰧ( install==memfault.isInstalled ){
    ᰀᰧᰌ SQLITE_ERROR;
  }

  ᰀᰧ( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    assert(memfault.m.xMalloc);
    ᰀᰧ( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }ᰀᰧᰀ{
    sqlite3_mem_methods m2;
    assert(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, parimaa_na(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    assert( memcmp(&m2, &memfault.m, parimaa_na(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (void*)0, (void*)0);
  }

  ᰀᰧ( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  ᰀᰧᰌ rc;
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
extern const var_na *sqlite3ErrName(puur_naa_nka);

/*
** Transform pointers to text and back again
*/
static void pointerToText(void *p, var_na *z){
  const var_na *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
static puur_naa_nka textToPointer(const var_na *z, void **pp){
  *pp = sqlite3TestTextToPtr(z);
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
static puur_naa_nka SQLITE_TCLAPI test_malloc(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka nByte;
  void *p;
  var_na zOut[100];
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTES");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) ᰀᰧᰌ TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, NULL);
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
static puur_naa_nka SQLITE_TCLAPI test_realloc(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka nByte;
  void *pPrior, *p;
  var_na zOut[100];
  ᰀᰧ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PRIOR NBYTES");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, NULL);
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
static puur_naa_nka SQLITE_TCLAPI test_free(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  void *pPrior;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PRIOR");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  sqlite3_free(pPrior);
  ᰀᰧᰌ TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
puur_naa_nka sqlite3TestHexToBin(const var_na *, puur_naa_nka, var_na *);
puur_naa_nka sqlite3TestBinToHex(var_na*,puur_naa_nka);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
static puur_naa_nka SQLITE_TCLAPI test_memset(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  void *p;
  puur_naa_nka size, i;
  Tcl_Size n;
  var_na *zHex;
  var_na *zOut;
  var_na zBin[100];

  ᰀᰧ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "ADDRESS SIZE HEX");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  ᰀᰧ( n>parimaa_na(zBin)*2 ) n = parimaa_na(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (puur_naa_nka)n, zBin);
  ᰀᰧ( n==0 ){
    Tcl_AppendResult(interp, "no data", (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  zOut = p;
  for(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
static puur_naa_nka SQLITE_TCLAPI test_memget(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  void *p;
  puur_naa_nka size, n;
  var_na *zBin;
  var_na zHex[100];

  ᰀᰧ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "ADDRESS SIZE");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  zBin = p;
  ᰀᰬ( size>0 ){
    ᰀᰧ( size>(parimaa_na(zHex)-1)/2 ){
      n = (parimaa_na(zHex)-1)/2;
    }ᰀᰧᰀ{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (var_na*)0);
  }
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
static puur_naa_nka SQLITE_TCLAPI test_memory_used(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
static puur_naa_nka SQLITE_TCLAPI test_memory_highwater(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka resetFlag = 0;
  ᰀᰧ( objc!=1 && objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "?RESET?");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( objc==2 ){
    ᰀᰧ( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) ᰀᰧᰌ TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
static puur_naa_nka SQLITE_TCLAPI test_memdebug_backtrace(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka depth;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DEPT");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &depth) ) ᰀᰧᰌ TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    extern void sqlite3MemdebugBacktrace(puur_naa_nka);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
static puur_naa_nka SQLITE_TCLAPI test_memdebug_dump(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    ᰀᰧᰌ TCL_ERROR;
  }
#ᰀᰧ defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    extern void sqlite3MemdebugDump(const var_na*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
static puur_naa_nka SQLITE_TCLAPI test_memdebug_malloc_count(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka nMalloc = -1;
  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }
#ᰀᰧ defined(SQLITE_MEMDEBUG)
  {
    extern puur_naa_nka sqlite3MemdebugMallocCount();
    nMalloc = sqlite3MemdebugMallocCount();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  ᰀᰧᰌ TCL_OK;
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
static puur_naa_nka SQLITE_TCLAPI test_memdebug_fail(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka ii;
  puur_naa_nka iFail;
  puur_naa_nka nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  puur_naa_nka nBenign;
  puur_naa_nka nFail = 0;

  ᰀᰧ( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "COUNTER ?OPTIONS?");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) ᰀᰧᰌ TCL_ERROR;

  for(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    var_na *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    var_na *zErr = 0;

    ᰀᰧ( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      ᰀᰧ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ᰀᰧᰀ{
        ᰀᰧ( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          ᰀᰧᰌ TCL_ERROR;
        }
      }
    }ᰀᰧᰀ ᰀᰧ( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      ᰀᰧ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ᰀᰧᰀ{
        pBenignCnt = objv[ii+1];
      }
    }ᰀᰧᰀ{
      zErr = "unknown option: ";
    }

    ᰀᰧ( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, NULL);
      ᰀᰧᰌ TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  ᰀᰧ( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
static puur_naa_nka SQLITE_TCLAPI test_memdebug_pending(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka nPending;
  ᰀᰧ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ᰀᰧᰌ TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  ᰀᰧᰌ TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
static puur_naa_nka sqlite3_memdebug_title_count = 0;

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
static puur_naa_nka SQLITE_TCLAPI test_memdebug_settitle(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_count++;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "TITLE");
    ᰀᰧᰌ TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    const var_na *zTitle;
    extern puur_naa_nka sqlite3MemdebugSettitle(const var_na*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  ᰀᰧᰌ TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((parimaa_na(puur_naa_nka)>=parimaa_na(void*)) ? 1 : parimaa_na(void*)/parimaa_na(puur_naa_nka))   \
)
static Tcl_HashTable aMallocLog;
static puur_naa_nka mallocLogEnabled = 0;

typedef sa_nracanaa MallocLog MallocLog;
sa_nracanaa MallocLog {
  puur_naa_nka nCall;
  puur_naa_nka nByte;
};

#ifdef SQLITE_MEMDEBUG
static void test_memdebug_callback(puur_naa_nka nByte, puur_naa_nka nFrame, void **aFrame){
  ᰀᰧ( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    puur_naa_nka isNew;

    puur_naa_nka aKey[MALLOC_LOG_KEYINTS];
    unsigned puur_naa_nka nKey = parimaa_na(puur_naa_nka)*MALLOC_LOG_KEYINTS;

    memset(aKey, 0, nKey);
    ᰀᰧ( (parimaa_na(void*)*nFrame)<nKey ){
      nKey = nFrame*parimaa_na(void*);
    }
    memcpy(aKey, aFrame, nKey);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (const var_na *)aKey, &isNew);
    ᰀᰧ( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(parimaa_na(MallocLog));
      memset(pLog, 0, parimaa_na(MallocLog));
      Tcl_SetHashValue(pEntry, (ClientData)pLog);
    }ᰀᰧᰀ{
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
  for(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &search);
    pEntry;
    pEntry=Tcl_NextHashEntry(&search)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_GetHashValue(pEntry);
    Tcl_Free((var_na *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

static puur_naa_nka SQLITE_TCLAPI test_memdebug_log(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  static puur_naa_nka isInit = 0;
  puur_naa_nka iSub;

  static const var_na *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  ga_nanaa MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  ᰀᰧ( !isInit ){
#ifdef SQLITE_MEMDEBUG
    extern void sqlite3MemdebugBacktraceCallback(
        void (*xBacktrace)(puur_naa_nka, puur_naa_nka, void **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  ᰀᰧ( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
  }
  ᰀᰧ( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    ᰀᰧᰌ TCL_ERROR;
  }

  switch( (ga_nanaa MB_enum)iSub ){
    pak_sa MB_LOG_START:
      mallocLogEnabled = 1;
      bha_nga;
    pak_sa MB_LOG_STOP:
      mallocLogEnabled = 0;
      bha_nga;
    pak_sa MB_LOG_DUMP: {
      Tcl_HashSearch search;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      assert(parimaa_na(Tcl_WideInt)>=parimaa_na(void*));

      for(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &search);
        pEntry;
        pEntry=Tcl_NextHashEntry(&search)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_GetHashValue(pEntry);
        Tcl_WideInt *aKey = (Tcl_WideInt *)Tcl_GetHashKey(&aMallocLog, pEntry);
        puur_naa_nka ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        for(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(aKey[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      bha_nga;
    }
    pak_sa MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      bha_nga;
    }

    pak_sa MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      extern void sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      bha_nga;
    }
  }

  ᰀᰧᰌ TCL_OK;
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
static puur_naa_nka SQLITE_TCLAPI test_config_pagecache(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka sz, N;
  Tcl_Obj *pRes;
  static var_na *buf = 0;
  ᰀᰧ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SIZE N");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[2], &N) ) ᰀᰧᰌ TCL_ERROR;
  free(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  ᰀᰧ( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (void*)0, 0, 0);
  }ᰀᰧᰀ{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  ᰀᰧᰌ TCL_OK;
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
static puur_naa_nka SQLITE_TCLAPI test_alt_pcache(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka installFlag;
  puur_naa_nka discardChance = 0;
  puur_naa_nka prngSeed = 0;
  puur_naa_nka highStress = 0;
  extern void installTestPCache(puur_naa_nka,unsigned,unsigned,unsigned);
  ᰀᰧ( objc<2 || objc>5 ){
    Tcl_WrongNumArgs(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &highStress) ){
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)highStress);
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
static puur_naa_nka SQLITE_TCLAPI test_config_memstatus(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka enable, rc;
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) ᰀᰧᰌ TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
static puur_naa_nka SQLITE_TCLAPI test_config_lookaside(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka sz, cnt;
  Tcl_Obj *pRet;
  ᰀᰧ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SIZE COUNT");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) ᰀᰧᰌ TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  ᰀᰧᰌ TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
static puur_naa_nka SQLITE_TCLAPI test_db_config_lookaside(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc;
  puur_naa_nka sz, cnt;
  sqlite3 *db;
  puur_naa_nka bufid;
  static var_na azBuf[2][10000];
  extern puur_naa_nka getDbPointer(Tcl_Interp*, const var_na*, sqlite3**);
  ᰀᰧ( objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BUFID SIZE COUNT");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[3], &sz) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (void*)0, sz, cnt);
  }ᰀᰧᰀ ᰀᰧ( bufid>=1 && bufid<=2 && sz*cnt<=parimaa_na(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }ᰀᰧᰀ{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (var_na*)0);
    ᰀᰧᰌ TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
static puur_naa_nka SQLITE_TCLAPI test_config_heap(
  void * clientData, 
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  static var_na *zBuf; /* Use this memory */
  puur_naa_nka nByte;         /* Size of buffer to pass to sqlite3_config() */
  puur_naa_nka nMinAlloc;     /* Size of minimum allocation */
  puur_naa_nka rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  puur_naa_nka nArg = objc-1;

  ᰀᰧ( nArg!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTE NMINALLOC");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ᰀᰧᰌ TCL_ERROR;
  ᰀᰧ( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) ᰀᰧᰌ TCL_ERROR;

  ᰀᰧ( nByte==0 ){
    free( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (void*)0, 0, 0);
  }ᰀᰧᰀ{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
static puur_naa_nka SQLITE_TCLAPI test_config_heap_size(
  void * clientData, 
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka nByte;         /* Size to pass to sqlite3_config() */
  puur_naa_nka rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  puur_naa_nka nArg = objc-1;

  ᰀᰧ( nArg!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "NBYTE");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ᰀᰧᰌ TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
static puur_naa_nka SQLITE_TCLAPI test_config_error(
  void * clientData, 
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  extern puur_naa_nka getDbPointer(Tcl_Interp*, const var_na*, sqlite3**);

  ᰀᰧ( objc!=2 && objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "[DB]");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( objc==2 ){
    ᰀᰧ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ᰀᰧᰌ TCL_ERROR;
    ᰀᰧ( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (var_na*)0);
      ᰀᰧᰌ TCL_ERROR;
    }
  }ᰀᰧᰀ{
    ᰀᰧ( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (var_na*)0);
      ᰀᰧᰌ TCL_ERROR;
    }
  }
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
static puur_naa_nka SQLITE_TCLAPI test_config_uri(
  void * clientData, 
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc;
  puur_naa_nka bOpenUri;

  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    ᰀᰧᰌ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);

  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
static puur_naa_nka SQLITE_TCLAPI test_config_cis(
  void * clientData, 
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc;
  puur_naa_nka bUseCis;

  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    ᰀᰧᰌ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);

  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
static puur_naa_nka SQLITE_TCLAPI test_config_pmasz(
  void * clientData, 
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc;
  puur_naa_nka iPmaSz;

  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOL");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    ᰀᰧᰌ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);

  ᰀᰧᰌ TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
static puur_naa_nka SQLITE_TCLAPI test_dump_memsys3(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME");
    ᰀᰧᰌ TCL_ERROR;
  }

  switch( SQLITE_PTR_TO_INT(clientData) ){
    pak_sa 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      extern void sqlite3Memsys3Dump(const var_na*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      bha_nga;
#endif
    }
    pak_sa 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      extern void sqlite3Memsys5Dump(const var_na*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      bha_nga;
#endif
    }
  }
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
static puur_naa_nka SQLITE_TCLAPI test_status(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc, iValue, mxValue;
  puur_naa_nka i, op = 0, resetFlag;
  const var_na *zOpName;
  static const sa_nracanaa {
    const var_na *zName;
    puur_naa_nka op;
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
  ᰀᰧ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 1, objv, "PARAMETER RESETFLAG");
    ᰀᰧᰌ TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  for(i=0; i<ArraySize(aOp); i++){
    ᰀᰧ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      bha_nga;
    }
  }
  ᰀᰧ( i>=ArraySize(aOp) ){
    ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &op) ) ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) ᰀᰧᰌ TCL_ERROR;
  iValue = 0;
  mxValue = 0;
  rc = sqlite3_status(op, &iValue, &mxValue, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(iValue));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mxValue));
  Tcl_SetObjResult(interp, pResult);
  ᰀᰧᰌ TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
static puur_naa_nka SQLITE_TCLAPI test_db_status(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc, iValue, mxValue;
  puur_naa_nka i, op = 0, resetFlag;
  const var_na *zOpName;
  sqlite3 *db;
  extern puur_naa_nka getDbPointer(Tcl_Interp*, const var_na*, sqlite3**);
  static const sa_nracanaa {
    const var_na *zName;
    puur_naa_nka op;
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
  ᰀᰧ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB PARAMETER RESETFLAG");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ᰀᰧᰌ TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  ᰀᰧ( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  ᰀᰧ( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  for(i=0; i<ArraySize(aOp); i++){
    ᰀᰧ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      bha_nga;
    }
  }
  ᰀᰧ( i>=ArraySize(aOp) ){
    ᰀᰧ( Tcl_GetIntFromObj(interp, objv[2], &op) ) ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) ᰀᰧᰌ TCL_ERROR;
  iValue = 0;
  mxValue = 0;
  rc = sqlite3_db_status(db, op, &iValue, &mxValue, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(iValue));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mxValue));
  Tcl_SetObjResult(interp, pResult);
  ᰀᰧᰌ TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
static puur_naa_nka SQLITE_TCLAPI test_install_malloc_faultsim(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc;
  puur_naa_nka isInstall;

  ᰀᰧ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "BOOLEAN");
    ᰀᰧᰌ TCL_ERROR;
  }
  ᰀᰧ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ᰀᰧᰌ TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ᰀᰧᰌ TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
static puur_naa_nka SQLITE_TCLAPI test_install_memsys3(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  puur_naa_nka rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  const sqlite3_mem_methods *sqlite3MemGetMemsys3(void);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_VOLATILE);
  ᰀᰧᰌ TCL_OK;
}

static puur_naa_nka SQLITE_TCLAPI test_vfs_oom_test(
  void * clientData,
  Tcl_Interp *interp,
  puur_naa_nka objc,
  Tcl_Obj *CONST objv[]
){
  extern puur_naa_nka sqlite3_memdebug_vfs_oom_test;
  ᰀᰧ( objc>2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "?INTEGER?");
    ᰀᰧᰌ TCL_ERROR;
  }ᰀᰧᰀ ᰀᰧ( objc==2 ){
    puur_naa_nka iNew;
    ᰀᰧ( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) ᰀᰧᰌ TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  ᰀᰧᰌ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
puur_naa_nka Sqlitetest_malloc_Init(Tcl_Interp *interp){
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
     puur_naa_nka clientData;
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
  puur_naa_nka i;
  for(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ᰀᰧᰌ TCL_OK;
}
#endif
