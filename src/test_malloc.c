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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
സ്ഥിതം ഘടന MemFault {
  പൂർണ്ണസംഖ്യ iCountdown;         /* Number of pending successes before a failure */
  പൂർണ്ണസംഖ്യ nRepeat;            /* Number of times to repeat the failure */
  പൂർണ്ണസംഖ്യ nBenign;            /* Number of benign failures seen since last config */
  പൂർണ്ണസംഖ്യ nFail;              /* Number of failures seen since last config */
  പൂർണ്ണസംഖ്യ nOkBefore;          /* Successful allocations prior to the first fault */
  പൂർണ്ണസംഖ്യ nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  പൂർണ്ണസംഖ്യ isInstalled;        /* True if the fault simulation layer is installed */
  പൂർണ്ണസംഖ്യ isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
സ്ഥിതം ശൂന്യം sqlite3Fault(ശൂന്യം){
  സ്ഥിതം u64 cnt = 0;
  cnt++;
  എങ്കിൽ( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
സ്ഥിതം ശൂന്യം sqlite3FirstFault(ശൂന്യം){
  സ്ഥിതം u64 cnt2 = 0;
  cnt2++;
  എങ്കിൽ( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ faultsimStep(ശൂന്യം){
  എങ്കിൽ( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    തിരികെ 0;
  }
  എങ്കിൽ( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    തിരികെ 0;
  }
  എങ്കിൽ( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  എങ്കിൽ( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  എങ്കിൽ( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  തിരികെ 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
സ്ഥിതം ശൂന്യം *faultsimMalloc(പൂർണ്ണസംഖ്യ n){
  ശൂന്യം *p = 0;
  എങ്കിൽ( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  തിരികെ p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
സ്ഥിതം ശൂന്യം *faultsimRealloc(ശൂന്യം *pOld, പൂർണ്ണസംഖ്യ n){
  ശൂന്യം *p = 0;
  എങ്കിൽ( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  തിരികെ p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
സ്ഥിതം ശൂന്യം faultsimConfig(പൂർണ്ണസംഖ്യ nDelay, പൂർണ്ണസംഖ്യ nRepeat){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ faultsimFailures(ശൂന്യം){
  തിരികെ memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ faultsimBenignFailures(ശൂന്യം){
  തിരികെ memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ faultsimPending(ശൂന്യം){
  എങ്കിൽ( memfault.enable ){
    തിരികെ memfault.iCountdown;
  }അല്ലെങ്കിൽ{
    തിരികെ -1;
  }
}


സ്ഥിതം ശൂന്യം faultsimBeginBenign(ശൂന്യം){
  memfault.isBenignMode++;
}
സ്ഥിതം ശൂന്യം faultsimEndBenign(ശൂന്യം){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ faultsimInstall(പൂർണ്ണസംഖ്യ install){
  പൂർണ്ണസംഖ്യ rc;

  install = (install ? 1 : 0);
  പ്രതിജ്ഞാ(memfault.isInstalled==1 || memfault.isInstalled==0);

  എങ്കിൽ( install==memfault.isInstalled ){
    തിരികെ SQLITE_ERROR;
  }

  എങ്കിൽ( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    പ്രതിജ്ഞാ(memfault.m.xMalloc);
    എങ്കിൽ( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }അല്ലെങ്കിൽ{
    sqlite3_mem_methods m2;
    പ്രതിജ്ഞാ(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, വലുപ്പം(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    പ്രതിജ്ഞാ( memcmp(&m2, &memfault.m, വലുപ്പം(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (ശൂന്യം*)0, (ശൂന്യം*)0);
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  തിരികെ rc;
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
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/*
** Transform pointers to text and back again
*/
സ്ഥിതം ശൂന്യം pointerToText(ശൂന്യം *p, അക്ഷരം *z){
  സ്ഥിരം അക്ഷരം *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ textToPointer(സ്ഥിരം അക്ഷരം *z, ശൂന്യം **pp){
  *pp = sqlite3TestTextToPtr(z);
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_malloc(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ nByte;
  ശൂന്യം *p;
  അക്ഷരം zOut[100];
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "NBYTES");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) തിരികെ TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_realloc(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ nByte;
  ശൂന്യം *pPrior, *p;
  അക്ഷരം zOut[100];
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "PRIOR NBYTES");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, ശൂന്യ);
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_free(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  ശൂന്യം *pPrior;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "PRIOR");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  sqlite3_free(pPrior);
  തിരികെ TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
പൂർണ്ണസംഖ്യ sqlite3TestHexToBin(സ്ഥിരം അക്ഷരം *, പൂർണ്ണസംഖ്യ, അക്ഷരം *);
പൂർണ്ണസംഖ്യ sqlite3TestBinToHex(അക്ഷരം*,പൂർണ്ണസംഖ്യ);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memset(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  ശൂന്യം *p;
  പൂർണ്ണസംഖ്യ size, i;
  Tcl_Size n;
  അക്ഷരം *zHex;
  അക്ഷരം *zOut;
  അക്ഷരം zBin[100];

  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  എങ്കിൽ( n>വലുപ്പം(zBin)*2 ) n = വലുപ്പം(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (പൂർണ്ണസംഖ്യ)n, zBin);
  എങ്കിൽ( n==0 ){
    Tcl_AppendResult(interp, "no data", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  zOut = p;
  വേണ്ടി(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  തിരികെ TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memget(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  ശൂന്യം *p;
  പൂർണ്ണസംഖ്യ size, n;
  അക്ഷരം *zBin;
  അക്ഷരം zHex[100];

  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "ADDRESS SIZE");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  zBin = p;
  ആകുമ്പോൾ( size>0 ){
    എങ്കിൽ( size>(വലുപ്പം(zHex)-1)/2 ){
      n = (വലുപ്പം(zHex)-1)/2;
    }അല്ലെങ്കിൽ{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (അക്ഷരം*)0);
  }
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memory_used(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memory_highwater(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ resetFlag = 0;
  എങ്കിൽ( objc!=1 && objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "?RESET?");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( objc==2 ){
    എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) തിരികെ TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memdebug_backtrace(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ depth;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DEPT");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &depth) ) തിരികെ TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    ബാഹ്യ ശൂന്യം sqlite3MemdebugBacktrace(പൂർണ്ണസംഖ്യ);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memdebug_dump(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "FILENAME");
    തിരികെ TCL_ERROR;
  }
#എങ്കിൽ defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    ബാഹ്യ ശൂന്യം sqlite3MemdebugDump(സ്ഥിരം അക്ഷരം*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memdebug_malloc_എണ്ണം(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ nMalloc = -1;
  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }
#എങ്കിൽ defined(SQLITE_MEMDEBUG)
  {
    ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_Memdebug_Malloc_എണ്ണം();
    nMalloc = sqlite3_Memdebug_Malloc_എണ്ണം();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memdebug_fail(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ ii;
  പൂർണ്ണസംഖ്യ iFail;
  പൂർണ്ണസംഖ്യ nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  പൂർണ്ണസംഖ്യ nBenign;
  പൂർണ്ണസംഖ്യ nFail = 0;

  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) തിരികെ TCL_ERROR;

  വേണ്ടി(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    അക്ഷരം *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    അക്ഷരം *zErr = 0;

    എങ്കിൽ( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      എങ്കിൽ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          തിരികെ TCL_ERROR;
        }
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      എങ്കിൽ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }അല്ലെങ്കിൽ{
        pBenignCnt = objv[ii+1];
      }
    }അല്ലെങ്കിൽ{
      zErr = "unknown option: ";
    }

    എങ്കിൽ( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, ശൂന്യ);
      തിരികെ TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  എങ്കിൽ( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memdebug_pending(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ nPending;
  എങ്കിൽ( objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "");
    തിരികെ TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  തിരികെ TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3_memdebug_title_എണ്ണം = 0;

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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memdebug_settitle(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_എണ്ണം++;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "TITLE");
    തിരികെ TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    സ്ഥിരം അക്ഷരം *zTitle;
    ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3MemdebugSettitle(സ്ഥിരം അക്ഷരം*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  തിരികെ TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((വലുപ്പം(പൂർണ്ണസംഖ്യ)>=വലുപ്പം(ശൂന്യം*)) ? 1 : വലുപ്പം(ശൂന്യം*)/വലുപ്പം(പൂർണ്ണസംഖ്യ))   \
)
സ്ഥിതം Tcl_HashTable aMallocLog;
സ്ഥിതം പൂർണ്ണസംഖ്യ mallocLogEnabled = 0;

തരനിർവചനം ഘടന MallocLog MallocLog;
ഘടന MallocLog {
  പൂർണ്ണസംഖ്യ nCall;
  പൂർണ്ണസംഖ്യ nByte;
};

#ifdef SQLITE_MEMDEBUG
സ്ഥിതം ശൂന്യം test_memdebug_callback(പൂർണ്ണസംഖ്യ nByte, പൂർണ്ണസംഖ്യ nFrame, ശൂന്യം **aFrame){
  എങ്കിൽ( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    പൂർണ്ണസംഖ്യ isNew;

    പൂർണ്ണസംഖ്യ a_താക്കോൽ[MALLOC_LOG_KEYINTS];
    unsigned പൂർണ്ണസംഖ്യ n_താക്കോൽ = വലുപ്പം(പൂർണ്ണസംഖ്യ)*MALLOC_LOG_KEYINTS;

    memset(a_താക്കോൽ, 0, n_താക്കോൽ);
    എങ്കിൽ( (വലുപ്പം(ശൂന്യം*)*nFrame)<n_താക്കോൽ ){
      n_താക്കോൽ = nFrame*വലുപ്പം(ശൂന്യം*);
    }
    memcpy(a_താക്കോൽ, aFrame, n_താക്കോൽ);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (സ്ഥിരം അക്ഷരം *)a_താക്കോൽ, &isNew);
    എങ്കിൽ( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(വലുപ്പം(MallocLog));
      memset(pLog, 0, വലുപ്പം(MallocLog));
      Tcl_Set_Hash_മൂല്യം(pEntry, (ClientData)pLog);
    }അല്ലെങ്കിൽ{
      pLog = (MallocLog *)Tcl_Get_Hash_മൂല്യം(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

സ്ഥിതം ശൂന്യം test_memdebug_log_clear(ശൂന്യം){
  Tcl_Hash_തിരയൽ തിരയൽ;
  Tcl_HashEntry *pEntry;
  വേണ്ടി(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &തിരയൽ);
    pEntry;
    pEntry=Tcl_NextHashEntry(&തിരയൽ)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_മൂല്യം(pEntry);
    Tcl_Free((അക്ഷരം *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_memdebug_log(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിതം പൂർണ്ണസംഖ്യ isInit = 0;
  പൂർണ്ണസംഖ്യ iSub;

  സ്ഥിതം സ്ഥിരം അക്ഷരം *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  എണ്ണൽ MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  എങ്കിൽ( !isInit ){
#ifdef SQLITE_MEMDEBUG
    ബാഹ്യ ശൂന്യം sqlite3MemdebugBacktraceCallback(
        ശൂന്യം (*xBacktrace)(പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, ശൂന്യം **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  എങ്കിൽ( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    തിരികെ TCL_ERROR;
  }

  തിരഞ്ഞെടുപ്പ്( (എണ്ണൽ MB_enum)iSub ){
    അവസ്ഥ MB_LOG_START:
      mallocLogEnabled = 1;
      നിർത്തുക;
    അവസ്ഥ MB_LOG_STOP:
      mallocLogEnabled = 0;
      നിർത്തുക;
    അവസ്ഥ MB_LOG_DUMP: {
      Tcl_Hash_തിരയൽ തിരയൽ;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      പ്രതിജ്ഞാ(വലുപ്പം(Tcl_WideInt)>=വലുപ്പം(ശൂന്യം*));

      വേണ്ടി(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &തിരയൽ);
        pEntry;
        pEntry=Tcl_NextHashEntry(&തിരയൽ)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_മൂല്യം(pEntry);
        Tcl_WideInt *a_താക്കോൽ = (Tcl_WideInt *)Tcl_Get_Hash_താക്കോൽ(&aMallocLog, pEntry);
        പൂർണ്ണസംഖ്യ ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        വേണ്ടി(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_താക്കോൽ[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      നിർത്തുക;
    }
    അവസ്ഥ MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      നിർത്തുക;
    }

    അവസ്ഥ MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      ബാഹ്യ ശൂന്യം sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      നിർത്തുക;
    }
  }

  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_pagecache(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ sz, N;
  Tcl_Obj *pRes;
  സ്ഥിതം അക്ഷരം *buf = 0;
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SIZE N");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &N) ) തിരികെ TCL_ERROR;
  സ്വതന്ത്രം(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  എങ്കിൽ( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (ശൂന്യം*)0, 0, 0);
  }അല്ലെങ്കിൽ{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  തിരികെ TCL_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_alt_pcache(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ installFlag;
  പൂർണ്ണസംഖ്യ discardChance = 0;
  പൂർണ്ണസംഖ്യ prngSeed = 0;
  പൂർണ്ണസംഖ്യ മുകളിലെ_Stress = 0;
  ബാഹ്യ ശൂന്യം installTestPCache(പൂർണ്ണസംഖ്യ,unsigned,unsigned,unsigned);
  എങ്കിൽ( objc<2 || objc>5 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &മുകളിലെ_Stress) ){
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)മുകളിലെ_Stress);
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_memstatus(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ enable, rc;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "BOOLEAN");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) തിരികെ TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_lookaside(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ sz, cnt;
  Tcl_Obj *pRet;
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SIZE COUNT");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) തിരികെ TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  തിരികെ TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_db_config_lookaside(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ sz, cnt;
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ bufid;
  സ്ഥിതം അക്ഷരം azBuf[2][10000];
  ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp*, സ്ഥിരം അക്ഷരം*, sqlite3**);
  എങ്കിൽ( objc!=5 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "BUFID SIZE COUNT");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[3], &sz) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (ശൂന്യം*)0, sz, cnt);
  }അല്ലെങ്കിൽ എങ്കിൽ( bufid>=1 && bufid<=2 && sz*cnt<=വലുപ്പം(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }അല്ലെങ്കിൽ{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_heap(
  ശൂന്യം * clientData, 
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിതം അക്ഷരം *zBuf; /* Use this memory */
  പൂർണ്ണസംഖ്യ nByte;         /* Size of buffer to pass to sqlite3_config() */
  പൂർണ്ണസംഖ്യ nMinAlloc;     /* Size of minimum allocation */
  പൂർണ്ണസംഖ്യ rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  പൂർണ്ണസംഖ്യ nArg = objc-1;

  എങ്കിൽ( nArg!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "NBYTE NMINALLOC");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) തിരികെ TCL_ERROR;
  എങ്കിൽ( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) തിരികെ TCL_ERROR;

  എങ്കിൽ( nByte==0 ){
    സ്വതന്ത്രം( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (ശൂന്യം*)0, 0, 0);
  }അല്ലെങ്കിൽ{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_heap_size(
  ശൂന്യം * clientData, 
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ nByte;         /* Size to pass to sqlite3_config() */
  പൂർണ്ണസംഖ്യ rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  പൂർണ്ണസംഖ്യ nArg = objc-1;

  എങ്കിൽ( nArg!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "NBYTE");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) തിരികെ TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_error(
  ശൂന്യം * clientData, 
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp*, സ്ഥിരം അക്ഷരം*, sqlite3**);

  എങ്കിൽ( objc!=2 && objc!=1 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "[DB]");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( objc==2 ){
    എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
    എങ്കിൽ( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
  }
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_uri(
  ശൂന്യം * clientData, 
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ bOpenUri;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "BOOL");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    തിരികെ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);

  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_cis(
  ശൂന്യം * clientData, 
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ bUseCis;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "BOOL");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    തിരികെ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);

  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_config_pmasz(
  ശൂന്യം * clientData, 
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ iPmaSz;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "BOOL");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    തിരികെ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);

  തിരികെ TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_dump_memsys3(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "FILENAME");
    തിരികെ TCL_ERROR;
  }

  തിരഞ്ഞെടുപ്പ്( SQLITE_PTR_TO_INT(clientData) ){
    അവസ്ഥ 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      ബാഹ്യ ശൂന്യം sqlite3Memsys3Dump(സ്ഥിരം അക്ഷരം*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      നിർത്തുക;
#endif
    }
    അവസ്ഥ 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      ബാഹ്യ ശൂന്യം sqlite3Memsys5Dump(സ്ഥിരം അക്ഷരം*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      നിർത്തുക;
#endif
    }
  }
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_status(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc, i_മൂല്യം, mx_മൂല്യം;
  പൂർണ്ണസംഖ്യ i, op = 0, resetFlag;
  സ്ഥിരം അക്ഷരം *zOpName;
  സ്ഥിതം സ്ഥിരം ഘടന {
    സ്ഥിരം അക്ഷരം *zName;
    പൂർണ്ണസംഖ്യ op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_അട്ടി        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_എണ്ണം        },
  };
  Tcl_Obj *pResult;
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    തിരികെ TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  വേണ്ടി(i=0; i<ArraySize(aOp); i++){
    എങ്കിൽ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      നിർത്തുക;
    }
  }
  എങ്കിൽ( i>=ArraySize(aOp) ){
    എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &op) ) തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) തിരികെ TCL_ERROR;
  i_മൂല്യം = 0;
  mx_മൂല്യം = 0;
  rc = sqlite3_status(op, &i_മൂല്യം, &mx_മൂല്യം, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_മൂല്യം));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_മൂല്യം));
  Tcl_SetObjResult(interp, pResult);
  തിരികെ TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_db_status(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc, i_മൂല്യം, mx_മൂല്യം;
  പൂർണ്ണസംഖ്യ i, op = 0, resetFlag;
  സ്ഥിരം അക്ഷരം *zOpName;
  sqlite3 *db;
  ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp*, സ്ഥിരം അക്ഷരം*, sqlite3**);
  സ്ഥിതം സ്ഥിരം ഘടന {
    സ്ഥിരം അക്ഷരം *zName;
    പൂർണ്ണസംഖ്യ op;
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
  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  എങ്കിൽ( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  എങ്കിൽ( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  വേണ്ടി(i=0; i<ArraySize(aOp); i++){
    എങ്കിൽ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      നിർത്തുക;
    }
  }
  എങ്കിൽ( i>=ArraySize(aOp) ){
    എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &op) ) തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) തിരികെ TCL_ERROR;
  i_മൂല്യം = 0;
  mx_മൂല്യം = 0;
  rc = sqlite3_db_status(db, op, &i_മൂല്യം, &mx_മൂല്യം, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_മൂല്യം));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_മൂല്യം));
  Tcl_SetObjResult(interp, pResult);
  തിരികെ TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_install_malloc_faultsim(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ isInstall;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "BOOLEAN");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    തിരികെ TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_install_memsys3(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  സ്ഥിരം sqlite3_mem_methods *sqlite3MemGetMemsys3(ശൂന്യം);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_VOLATILE);
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_vfs_oom_test(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  ബാഹ്യ പൂർണ്ണസംഖ്യ sqlite3_memdebug_vfs_oom_test;
  എങ്കിൽ( objc>2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "?INTEGER?");
    തിരികെ TCL_ERROR;
  }അല്ലെങ്കിൽ എങ്കിൽ( objc==2 ){
    പൂർണ്ണസംഖ്യ iNew;
    എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) തിരികെ TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  തിരികെ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest_malloc_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     പൂർണ്ണസംഖ്യ clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_എണ്ണം ,0 },
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
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  തിരികെ TCL_OK;
}
#endif
