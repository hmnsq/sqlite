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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
ସ୍ଥିର ସଂରଚନା MemFault {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCountdown;         /* Number of pending successes before a failure */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRepeat;            /* Number of times to repeat the failure */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBenign;            /* Number of benign failures seen since last config */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFail;              /* Number of failures seen since last config */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOkBefore;          /* Successful allocations prior to the first fault */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInstalled;        /* True if the fault simulation layer is installed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3Fault(ଶୂନ୍ୟ){
  ସ୍ଥିର u64 cnt = 0;
  cnt++;
  ଯଦି( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3FirstFault(ଶୂନ୍ୟ){
  ସ୍ଥିର u64 cnt2 = 0;
  cnt2++;
  ଯଦି( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା faultsimStep(ଶୂନ୍ୟ){
  ଯଦି( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  ଯଦି( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  ଯଦି( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  ଫେରନ୍ତୁ 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
ସ୍ଥିର ଶୂନ୍ୟ *faultsimMalloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  ଶୂନ୍ୟ *p = 0;
  ଯଦି( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  ଫେରନ୍ତୁ p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
ସ୍ଥିର ଶୂନ୍ୟ *faultsimRealloc(ଶୂନ୍ୟ *pOld, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  ଶୂନ୍ୟ *p = 0;
  ଯଦି( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  ଫେରନ୍ତୁ p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
ସ୍ଥିର ଶୂନ୍ୟ faultsimConfig(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nDelay, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRepeat){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା faultsimFailures(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା faultsimBenignFailures(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା faultsimPending(ଶୂନ୍ୟ){
  ଯଦି( memfault.enable ){
    ଫେରନ୍ତୁ memfault.iCountdown;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ -1;
  }
}


ସ୍ଥିର ଶୂନ୍ୟ faultsimBeginBenign(ଶୂନ୍ୟ){
  memfault.isBenignMode++;
}
ସ୍ଥିର ଶୂନ୍ୟ faultsimEndBenign(ଶୂନ୍ୟ){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା faultsimInstall(ପୂର୍ଣ୍ଣସଂଖ୍ୟା install){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  install = (install ? 1 : 0);
  ପ୍ରତିଜ୍ଞା(memfault.isInstalled==1 || memfault.isInstalled==0);

  ଯଦି( install==memfault.isInstalled ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  ଯଦି( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    ପ୍ରତିଜ୍ଞା(memfault.m.xMalloc);
    ଯଦି( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }ଅନ୍ୟଥା{
    sqlite3_mem_methods m2;
    ପ୍ରତିଜ୍ଞା(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, ଆକାର(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    ପ୍ରତିଜ୍ଞା( memcmp(&m2, &memfault.m, ଆକାର(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (ଶୂନ୍ୟ*)0, (ଶୂନ୍ୟ*)0);
  }

  ଯଦି( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  ଫେରନ୍ତୁ rc;
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
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** Transform pointers to text and back again
*/
ସ୍ଥିର ଶୂନ୍ୟ pointerToText(ଶୂନ୍ୟ *p, ଅକ୍ଷର *z){
  ଅଚଳ ଅକ୍ଷର *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା textToPointer(ଅଚଳ ଅକ୍ଷର *z, ଶୂନ୍ୟ **pp){
  *pp = sqlite3TestTextToPtr(z);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_malloc(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ଶୂନ୍ୟ *p;
  ଅକ୍ଷର zOut[100];
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "NBYTES");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) ଫେରନ୍ତୁ TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_realloc(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ଶୂନ୍ୟ *pPrior, *p;
  ଅକ୍ଷର zOut[100];
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "PRIOR NBYTES");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, ଖାଲି);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_free(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଶୂନ୍ୟ *pPrior;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "PRIOR");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  sqlite3_free(pPrior);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestHexToBin(ଅଚଳ ଅକ୍ଷର *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅକ୍ଷର *);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3TestBinToHex(ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memset(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଶୂନ୍ୟ *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, i;
  Tcl_Size n;
  ଅକ୍ଷର *zHex;
  ଅକ୍ଷର *zOut;
  ଅକ୍ଷର zBin[100];

  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  ଯଦି( n>ଆକାର(zBin)*2 ) n = ଆକାର(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n, zBin);
  ଯଦି( n==0 ){
    Tcl_AppendResult(interp, "no data", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zOut = p;
  ପାଇଁ(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memget(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଶୂନ୍ୟ *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, n;
  ଅକ୍ଷର *zBin;
  ଅକ୍ଷର zHex[100];

  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "ADDRESS SIZE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zBin = p;
  ଯେତେବେଳେ( size>0 ){
    ଯଦି( size>(ଆକାର(zHex)-1)/2 ){
      n = (ଆକାର(zHex)-1)/2;
    }ଅନ୍ୟଥା{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (ଅକ୍ଷର*)0);
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memory_used(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memory_highwater(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା resetFlag = 0;
  ଯଦି( objc!=1 && objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "?RESET?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( objc==2 ){
    ଯଦି( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) ଫେରନ୍ତୁ TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memdebug_backtrace(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DEPT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &depth) ) ଫେରନ୍ତୁ TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    ବାହ୍ୟ ଶୂନ୍ୟ sqlite3MemdebugBacktrace(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memdebug_dump(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "FILENAME");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
#ଯଦି defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    ବାହ୍ୟ ଶୂନ୍ୟ sqlite3MemdebugDump(ଅଚଳ ଅକ୍ଷର*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memdebug_malloc_ଗଣନା(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMalloc = -1;
  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
#ଯଦି defined(SQLITE_MEMDEBUG)
  {
    ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Memdebug_Malloc_ଗଣନା();
    nMalloc = sqlite3_Memdebug_Malloc_ଗଣନା();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memdebug_fail(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFail;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBenign;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFail = 0;

  ଯଦି( objc<2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) ଫେରନ୍ତୁ TCL_ERROR;

  ପାଇଁ(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    ଅକ୍ଷର *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    ଅକ୍ଷର *zErr = 0;

    ଯଦି( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      ଯଦି( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ଅନ୍ୟଥା{
        ଯଦି( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }
      }
    }ଅନ୍ୟଥା ଯଦି( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      ଯଦି( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ଅନ୍ୟଥା{
        pBenignCnt = objv[ii+1];
      }
    }ଅନ୍ୟଥା{
      zErr = "unknown option: ";
    }

    ଯଦି( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  ଯଦି( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memdebug_pending(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPending;
  ଯଦି( objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_memdebug_title_ଗଣନା = 0;

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memdebug_settitle(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_ଗଣନା++;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "TITLE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    ଅଚଳ ଅକ୍ଷର *zTitle;
    ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3MemdebugSettitle(ଅଚଳ ଅକ୍ଷର*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  ଫେରନ୍ତୁ TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)>=ଆକାର(ଶୂନ୍ୟ*)) ? 1 : ଆକାର(ଶୂନ୍ୟ*)/ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))   \
)
ସ୍ଥିର Tcl_HashTable aMallocLog;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mallocLogEnabled = 0;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା MallocLog MallocLog;
ସଂରଚନା MallocLog {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCall;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
};

#ifdef SQLITE_MEMDEBUG
ସ୍ଥିର ଶୂନ୍ୟ test_memdebug_callback(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFrame, ଶୂନ୍ୟ **aFrame){
  ଯଦି( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isNew;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା a_ଚାବି[MALLOC_LOG_KEYINTS];
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*MALLOC_LOG_KEYINTS;

    memset(a_ଚାବି, 0, n_ଚାବି);
    ଯଦି( (ଆକାର(ଶୂନ୍ୟ*)*nFrame)<n_ଚାବି ){
      n_ଚାବି = nFrame*ଆକାର(ଶୂନ୍ୟ*);
    }
    memcpy(a_ଚାବି, aFrame, n_ଚାବି);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (ଅଚଳ ଅକ୍ଷର *)a_ଚାବି, &isNew);
    ଯଦି( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(ଆକାର(MallocLog));
      memset(pLog, 0, ଆକାର(MallocLog));
      Tcl_Set_Hash_ମୂଲ୍ୟ(pEntry, (ClientData)pLog);
    }ଅନ୍ୟଥା{
      pLog = (MallocLog *)Tcl_Get_Hash_ମୂଲ୍ୟ(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

ସ୍ଥିର ଶୂନ୍ୟ test_memdebug_log_clear(ଶୂନ୍ୟ){
  Tcl_Hash_ସନ୍ଧାନ ସନ୍ଧାନ;
  Tcl_HashEntry *pEntry;
  ପାଇଁ(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &ସନ୍ଧାନ);
    pEntry;
    pEntry=Tcl_NextHashEntry(&ସନ୍ଧାନ)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ମୂଲ୍ୟ(pEntry);
    Tcl_Free((ଅକ୍ଷର *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_memdebug_log(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInit = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSub;

  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  ଗଣନା MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  ଯଦି( !isInit ){
#ifdef SQLITE_MEMDEBUG
    ବାହ୍ୟ ଶୂନ୍ୟ sqlite3MemdebugBacktraceCallback(
        ଶୂନ୍ୟ (*xBacktrace)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଶୂନ୍ୟ **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  ଯଦି( objc<2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  ଯଦି( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ବାଛ( (ଗଣନା MB_enum)iSub ){
    ସ୍ଥିତି MB_LOG_START:
      mallocLogEnabled = 1;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି MB_LOG_STOP:
      mallocLogEnabled = 0;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି MB_LOG_DUMP: {
      Tcl_Hash_ସନ୍ଧାନ ସନ୍ଧାନ;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      ପ୍ରତିଜ୍ଞା(ଆକାର(Tcl_WideInt)>=ଆକାର(ଶୂନ୍ୟ*));

      ପାଇଁ(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &ସନ୍ଧାନ);
        pEntry;
        pEntry=Tcl_NextHashEntry(&ସନ୍ଧାନ)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ମୂଲ୍ୟ(pEntry);
        Tcl_WideInt *a_ଚାବି = (Tcl_WideInt *)Tcl_Get_Hash_ଚାବି(&aMallocLog, pEntry);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        ପାଇଁ(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_ଚାବି[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      ବାହ୍ୟ ଶୂନ୍ୟ sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_pagecache(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz, N;
  Tcl_Obj *pRes;
  ସ୍ଥିର ଅକ୍ଷର *buf = 0;
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SIZE N");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &N) ) ଫେରନ୍ତୁ TCL_ERROR;
  ମୁକ୍ତ(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  ଯଦି( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (ଶୂନ୍ୟ*)0, 0, 0);
  }ଅନ୍ୟଥା{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  ଫେରନ୍ତୁ TCL_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_alt_pcache(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା installFlag;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା discardChance = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା prngSeed = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଉପର_Stress = 0;
  ବାହ୍ୟ ଶୂନ୍ୟ installTestPCache(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,unsigned,unsigned,unsigned);
  ଯଦି( objc<2 || objc>5 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &ଉପର_Stress) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)ଉପର_Stress);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_memstatus(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା enable, rc;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "BOOLEAN");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) ଫେରନ୍ତୁ TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_lookaside(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz, cnt;
  Tcl_Obj *pRet;
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SIZE COUNT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) ଫେରନ୍ତୁ TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_db_config_lookaside(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz, cnt;
  sqlite3 *db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bufid;
  ସ୍ଥିର ଅକ୍ଷର azBuf[2][10000];
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp*, ଅଚଳ ଅକ୍ଷର*, sqlite3**);
  ଯଦି( objc!=5 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "BUFID SIZE COUNT");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[3], &sz) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (ଶୂନ୍ୟ*)0, sz, cnt);
  }ଅନ୍ୟଥା ଯଦି( bufid>=1 && bufid<=2 && sz*cnt<=ଆକାର(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }ଅନ୍ୟଥା{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_heap(
  ଶୂନ୍ୟ * clientData, 
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ସ୍ଥିର ଅକ୍ଷର *zBuf; /* Use this memory */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;         /* Size of buffer to pass to sqlite3_config() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMinAlloc;     /* Size of minimum allocation */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg = objc-1;

  ଯଦି( nArg!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "NBYTE NMINALLOC");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ଫେରନ୍ତୁ TCL_ERROR;
  ଯଦି( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) ଫେରନ୍ତୁ TCL_ERROR;

  ଯଦି( nByte==0 ){
    ମୁକ୍ତ( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (ଶୂନ୍ୟ*)0, 0, 0);
  }ଅନ୍ୟଥା{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_heap_size(
  ଶୂନ୍ୟ * clientData, 
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;         /* Size to pass to sqlite3_config() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg = objc-1;

  ଯଦି( nArg!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "NBYTE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ଫେରନ୍ତୁ TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_error(
  ଶୂନ୍ୟ * clientData, 
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp*, ଅଚଳ ଅକ୍ଷର*, sqlite3**);

  ଯଦି( objc!=2 && objc!=1 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "[DB]");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( objc==2 ){
    ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
    ଯଦି( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
  }ଅନ୍ୟଥା{
    ଯଦି( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_uri(
  ଶୂନ୍ୟ * clientData, 
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bOpenUri;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "BOOL");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_cis(
  ଶୂନ୍ୟ * clientData, 
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bUseCis;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "BOOL");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);

  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_config_pmasz(
  ଶୂନ୍ୟ * clientData, 
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPmaSz;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "BOOL");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);

  ଫେରନ୍ତୁ TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_dump_memsys3(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "FILENAME");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ବାଛ( SQLITE_PTR_TO_INT(clientData) ){
    ସ୍ଥିତି 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      ବାହ୍ୟ ଶୂନ୍ୟ sqlite3Memsys3Dump(ଅଚଳ ଅକ୍ଷର*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      ଭାଙ୍ଗ;
#endif
    }
    ସ୍ଥିତି 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      ବାହ୍ୟ ଶୂନ୍ୟ sqlite3Memsys5Dump(ଅଚଳ ଅକ୍ଷର*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      ଭାଙ୍ଗ;
#endif
    }
  }
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_status(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, i_ମୂଲ୍ୟ, mx_ମୂଲ୍ୟ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, op = 0, resetFlag;
  ଅଚଳ ଅକ୍ଷର *zOpName;
  ସ୍ଥିର ଅଚଳ ସଂରଚନା {
    ଅଚଳ ଅକ୍ଷର *zName;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_ଗଦା        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_ଗଣନା        },
  };
  Tcl_Obj *pResult;
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  ପାଇଁ(i=0; i<ArraySize(aOp); i++){
    ଯଦି( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( i>=ArraySize(aOp) ){
    ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &op) ) ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) ଫେରନ୍ତୁ TCL_ERROR;
  i_ମୂଲ୍ୟ = 0;
  mx_ମୂଲ୍ୟ = 0;
  rc = sqlite3_status(op, &i_ମୂଲ୍ୟ, &mx_ମୂଲ୍ୟ, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_ମୂଲ୍ୟ));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_ମୂଲ୍ୟ));
  Tcl_SetObjResult(interp, pResult);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_db_status(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, i_ମୂଲ୍ୟ, mx_ମୂଲ୍ୟ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, op = 0, resetFlag;
  ଅଚଳ ଅକ୍ଷର *zOpName;
  sqlite3 *db;
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp*, ଅଚଳ ଅକ୍ଷର*, sqlite3**);
  ସ୍ଥିର ଅଚଳ ସଂରଚନା {
    ଅଚଳ ଅକ୍ଷର *zName;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
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
  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  ଯଦି( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  ଯଦି( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  ପାଇଁ(i=0; i<ArraySize(aOp); i++){
    ଯଦି( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( i>=ArraySize(aOp) ){
    ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &op) ) ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) ଫେରନ୍ତୁ TCL_ERROR;
  i_ମୂଲ୍ୟ = 0;
  mx_ମୂଲ୍ୟ = 0;
  rc = sqlite3_db_status(db, op, &i_ମୂଲ୍ୟ, &mx_ମୂଲ୍ୟ, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_ମୂଲ୍ୟ));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_ମୂଲ୍ୟ));
  Tcl_SetObjResult(interp, pResult);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_install_malloc_faultsim(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInstall;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "BOOLEAN");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_install_memsys3(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  ଅଚଳ sqlite3_mem_methods *sqlite3MemGetMemsys3(ଶୂନ୍ୟ);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_vfs_oom_test(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_memdebug_vfs_oom_test;
  ଯଦି( objc>2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "?INTEGER?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }ଅନ୍ୟଥା ଯଦି( objc==2 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNew;
    ଯଦି( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) ଫେରନ୍ତୁ TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_malloc_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
     ପୂର୍ଣ୍ଣସଂଖ୍ୟା clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_ଗଣନା ,0 },
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ଫେରନ୍ତୁ TCL_OK;
}
#endif
