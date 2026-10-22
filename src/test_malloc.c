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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
ಸ್ಥಿರ ಸಂರಚನಾ MemFault {
  ಪೂರ್ಣಾಙ್ಕ iCountdown;         /* Number of pending successes before a failure */
  ಪೂರ್ಣಾಙ್ಕ nRepeat;            /* Number of times to repeat the failure */
  ಪೂರ್ಣಾಙ್ಕ nBenign;            /* Number of benign failures seen since last config */
  ಪೂರ್ಣಾಙ್ಕ nFail;              /* Number of failures seen since last config */
  ಪೂರ್ಣಾಙ್ಕ nOkBefore;          /* Successful allocations prior to the first fault */
  ಪೂರ್ಣಾಙ್ಕ nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  ಪೂರ್ಣಾಙ್ಕ isInstalled;        /* True if the fault simulation layer is installed */
  ಪೂರ್ಣಾಙ್ಕ isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3Fault(ಶೂನ್ಯ){
  ಸ್ಥಿರ u64 cnt = 0;
  cnt++;
  ಯದಿ( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3FirstFault(ಶೂನ್ಯ){
  ಸ್ಥಿರ u64 cnt2 = 0;
  cnt2++;
  ಯದಿ( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ faultsimStep(ಶೂನ್ಯ){
  ಯದಿ( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  ಯದಿ( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  ಯದಿ( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  ಪ್ರತ್ಯಾಗಮ 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *faultsimMalloc(ಪೂರ್ಣಾಙ್ಕ n){
  ಶೂನ್ಯ *p = 0;
  ಯದಿ( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  ಪ್ರತ್ಯಾಗಮ p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *faultsimRealloc(ಶೂನ್ಯ *pOld, ಪೂರ್ಣಾಙ್ಕ n){
  ಶೂನ್ಯ *p = 0;
  ಯದಿ( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
ಸ್ಥಿರ ಶೂನ್ಯ faultsimConfig(ಪೂರ್ಣಾಙ್ಕ nDelay, ಪೂರ್ಣಾಙ್ಕ nRepeat){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ faultsimFailures(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ faultsimBenignFailures(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ faultsimPending(ಶೂನ್ಯ){
  ಯದಿ( memfault.enable ){
    ಪ್ರತ್ಯಾಗಮ memfault.iCountdown;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ -1;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ faultsimBeginBenign(ಶೂನ್ಯ){
  memfault.isBenignMode++;
}
ಸ್ಥಿರ ಶೂನ್ಯ faultsimEndBenign(ಶೂನ್ಯ){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ faultsimInstall(ಪೂರ್ಣಾಙ್ಕ install){
  ಪೂರ್ಣಾಙ್ಕ rc;

  install = (install ? 1 : 0);
  ಪ್ರತಿಜ್ಞಾ(memfault.isInstalled==1 || memfault.isInstalled==0);

  ಯದಿ( install==memfault.isInstalled ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  ಯದಿ( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    ಪ್ರತಿಜ್ಞಾ(memfault.m.xMalloc);
    ಯದಿ( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }ಅನ್ಯಥಾ{
    sqlite3_mem_methods m2;
    ಪ್ರತಿಜ್ಞಾ(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, ಪರಿಮಾಣ(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    ಪ್ರತಿಜ್ಞಾ( memcmp(&m2, &memfault.m, ಪರಿಮಾಣ(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (ಶೂನ್ಯ*)0, (ಶೂನ್ಯ*)0);
  }

  ಯದಿ( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

/*
** Transform pointers to text and back again
*/
ಸ್ಥಿರ ಶೂನ್ಯ pointerToText(ಶೂನ್ಯ *p, ವರ್ಣ *z){
  ಧ್ರುವ ವರ್ಣ *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ textToPointer(ಧ್ರುವ ವರ್ಣ *z, ಶೂನ್ಯ **pp){
  *pp = sqlite3TestTextToPtr(z);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_malloc(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ nByte;
  ಶೂನ್ಯ *p;
  ವರ್ಣ zOut[100];
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "NBYTES");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_realloc(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ nByte;
  ಶೂನ್ಯ *pPrior, *p;
  ವರ್ಣ zOut[100];
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "PRIOR NBYTES");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_free(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ *pPrior;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "PRIOR");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  sqlite3_free(pPrior);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3TestHexToBin(ಧ್ರುವ ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ *);
ಪೂರ್ಣಾಙ್ಕ sqlite3TestBinToHex(ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memset(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ *p;
  ಪೂರ್ಣಾಙ್ಕ size, i;
  Tcl_Size n;
  ವರ್ಣ *zHex;
  ವರ್ಣ *zOut;
  ವರ್ಣ zBin[100];

  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  ಯದಿ( n>ಪರಿಮಾಣ(zBin)*2 ) n = ಪರಿಮಾಣ(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (ಪೂರ್ಣಾಙ್ಕ)n, zBin);
  ಯದಿ( n==0 ){
    Tcl_AppendResult(interp, "no data", (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zOut = p;
  ಪ್ರತಿ(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memget(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ *p;
  ಪೂರ್ಣಾಙ್ಕ size, n;
  ವರ್ಣ *zBin;
  ವರ್ಣ zHex[100];

  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "ADDRESS SIZE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zBin = p;
  ಯಾವತ್( size>0 ){
    ಯದಿ( size>(ಪರಿಮಾಣ(zHex)-1)/2 ){
      n = (ಪರಿಮಾಣ(zHex)-1)/2;
    }ಅನ್ಯಥಾ{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (ವರ್ಣ*)0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memory_used(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memory_highwater(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ resetFlag = 0;
  ಯದಿ( objc!=1 && objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "?RESET?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( objc==2 ){
    ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memdebug_backtrace(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ depth;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DEPT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &depth) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3MemdebugBacktrace(ಪೂರ್ಣಾಙ್ಕ);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memdebug_dump(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "FILENAME");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
#ಯದಿ defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3MemdebugDump(ಧ್ರುವ ವರ್ಣ*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memdebug_malloc_ಎಣಿಕೆ(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ nMalloc = -1;
  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
#ಯದಿ defined(SQLITE_MEMDEBUG)
  {
    ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_Memdebug_Malloc_ಎಣಿಕೆ();
    nMalloc = sqlite3_Memdebug_Malloc_ಎಣಿಕೆ();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memdebug_fail(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪೂರ್ಣಾಙ್ಕ iFail;
  ಪೂರ್ಣಾಙ್ಕ nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  ಪೂರ್ಣಾಙ್ಕ nBenign;
  ಪೂರ್ಣಾಙ್ಕ nFail = 0;

  ಯದಿ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  ಪ್ರತಿ(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    ವರ್ಣ *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    ವರ್ಣ *zErr = 0;

    ಯದಿ( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      ಯದಿ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ಅನ್ಯಥಾ{
        ಯದಿ( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
        }
      }
    }ಅನ್ಯಥಾ ಯದಿ( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      ಯದಿ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ಅನ್ಯಥಾ{
        pBenignCnt = objv[ii+1];
      }
    }ಅನ್ಯಥಾ{
      zErr = "unknown option: ";
    }

    ಯದಿ( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  ಯದಿ( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memdebug_pending(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ nPending;
  ಯದಿ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3_memdebug_title_ಎಣಿಕೆ = 0;

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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memdebug_settitle(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_ಎಣಿಕೆ++;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "TITLE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    ಧ್ರುವ ವರ್ಣ *zTitle;
    ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3MemdebugSettitle(ಧ್ರುವ ವರ್ಣ*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)>=ಪರಿಮಾಣ(ಶೂನ್ಯ*)) ? 1 : ಪರಿಮಾಣ(ಶೂನ್ಯ*)/ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))   \
)
ಸ್ಥಿರ Tcl_HashTable aMallocLog;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mallocLogEnabled = 0;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ MallocLog MallocLog;
ಸಂರಚನಾ MallocLog {
  ಪೂರ್ಣಾಙ್ಕ nCall;
  ಪೂರ್ಣಾಙ್ಕ nByte;
};

#ifdef SQLITE_MEMDEBUG
ಸ್ಥಿರ ಶೂನ್ಯ test_memdebug_callback(ಪೂರ್ಣಾಙ್ಕ nByte, ಪೂರ್ಣಾಙ್ಕ nFrame, ಶೂನ್ಯ **aFrame){
  ಯದಿ( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    ಪೂರ್ಣಾಙ್ಕ isNew;

    ಪೂರ್ಣಾಙ್ಕ a_ಕೀಲಿ[MALLOC_LOG_KEYINTS];
    unsigned ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)*MALLOC_LOG_KEYINTS;

    memset(a_ಕೀಲಿ, 0, n_ಕೀಲಿ);
    ಯದಿ( (ಪರಿಮಾಣ(ಶೂನ್ಯ*)*nFrame)<n_ಕೀಲಿ ){
      n_ಕೀಲಿ = nFrame*ಪರಿಮಾಣ(ಶೂನ್ಯ*);
    }
    memcpy(a_ಕೀಲಿ, aFrame, n_ಕೀಲಿ);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (ಧ್ರುವ ವರ್ಣ *)a_ಕೀಲಿ, &isNew);
    ಯದಿ( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(ಪರಿಮಾಣ(MallocLog));
      memset(pLog, 0, ಪರಿಮಾಣ(MallocLog));
      Tcl_Set_Hash_ಮೌಲ್ಯ(pEntry, (ClientData)pLog);
    }ಅನ್ಯಥಾ{
      pLog = (MallocLog *)Tcl_Get_Hash_ಮೌಲ್ಯ(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

ಸ್ಥಿರ ಶೂನ್ಯ test_memdebug_log_clear(ಶೂನ್ಯ){
  Tcl_Hash_ಹುಡುಕು ಹುಡುಕು;
  Tcl_HashEntry *pEntry;
  ಪ್ರತಿ(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &ಹುಡುಕು);
    pEntry;
    pEntry=Tcl_NextHashEntry(&ಹುಡುಕು)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ಮೌಲ್ಯ(pEntry);
    Tcl_Free((ವರ್ಣ *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_memdebug_log(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isInit = 0;
  ಪೂರ್ಣಾಙ್ಕ iSub;

  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  ಗಣನಾ MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  ಯದಿ( !isInit ){
#ifdef SQLITE_MEMDEBUG
    ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3MemdebugBacktraceCallback(
        ಶೂನ್ಯ (*xBacktrace)(ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, ಶೂನ್ಯ **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  ಯದಿ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  ಯದಿ( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ವಿಕಲ್ಪ( (ಗಣನಾ MB_enum)iSub ){
    ಪಕ್ಷ MB_LOG_START:
      mallocLogEnabled = 1;
      ಭಙ್ಗ;
    ಪಕ್ಷ MB_LOG_STOP:
      mallocLogEnabled = 0;
      ಭಙ್ಗ;
    ಪಕ್ಷ MB_LOG_DUMP: {
      Tcl_Hash_ಹುಡುಕು ಹುಡುಕು;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      ಪ್ರತಿಜ್ಞಾ(ಪರಿಮಾಣ(Tcl_WideInt)>=ಪರಿಮಾಣ(ಶೂನ್ಯ*));

      ಪ್ರತಿ(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &ಹುಡುಕು);
        pEntry;
        pEntry=Tcl_NextHashEntry(&ಹುಡುಕು)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ಮೌಲ್ಯ(pEntry);
        Tcl_WideInt *a_ಕೀಲಿ = (Tcl_WideInt *)Tcl_Get_Hash_ಕೀಲಿ(&aMallocLog, pEntry);
        ಪೂರ್ಣಾಙ್ಕ ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        ಪ್ರತಿ(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_ಕೀಲಿ[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_pagecache(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ sz, N;
  Tcl_Obj *pRes;
  ಸ್ಥಿರ ವರ್ಣ *buf = 0;
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "SIZE N");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &N) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಮುಕ್ತ(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  ಯದಿ( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (ಶೂನ್ಯ*)0, 0, 0);
  }ಅನ್ಯಥಾ{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_alt_pcache(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ installFlag;
  ಪೂರ್ಣಾಙ್ಕ discardChance = 0;
  ಪೂರ್ಣಾಙ್ಕ prngSeed = 0;
  ಪೂರ್ಣಾಙ್ಕ ಮೇಲಿನ_Stress = 0;
  ಬಾಹ್ಯ ಶೂನ್ಯ installTestPCache(ಪೂರ್ಣಾಙ್ಕ,unsigned,unsigned,unsigned);
  ಯದಿ( objc<2 || objc>5 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &ಮೇಲಿನ_Stress) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)ಮೇಲಿನ_Stress);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_memstatus(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ enable, rc;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "BOOLEAN");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_lookaside(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ sz, cnt;
  Tcl_Obj *pRet;
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "SIZE COUNT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_db_config_lookaside(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ sz, cnt;
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ bufid;
  ಸ್ಥಿರ ವರ್ಣ azBuf[2][10000];
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp*, ಧ್ರುವ ವರ್ಣ*, sqlite3**);
  ಯದಿ( objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "BUFID SIZE COUNT");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[3], &sz) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (ಶೂನ್ಯ*)0, sz, cnt);
  }ಅನ್ಯಥಾ ಯದಿ( bufid>=1 && bufid<=2 && sz*cnt<=ಪರಿಮಾಣ(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }ಅನ್ಯಥಾ{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (ವರ್ಣ*)0);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_heap(
  ಶೂನ್ಯ * clientData, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರ ವರ್ಣ *zBuf; /* Use this memory */
  ಪೂರ್ಣಾಙ್ಕ nByte;         /* Size of buffer to pass to sqlite3_config() */
  ಪೂರ್ಣಾಙ್ಕ nMinAlloc;     /* Size of minimum allocation */
  ಪೂರ್ಣಾಙ್ಕ rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ಪೂರ್ಣಾಙ್ಕ nArg = objc-1;

  ಯದಿ( nArg!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "NBYTE NMINALLOC");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  ಯದಿ( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  ಯದಿ( nByte==0 ){
    ಮುಕ್ತ( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (ಶೂನ್ಯ*)0, 0, 0);
  }ಅನ್ಯಥಾ{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_heap_size(
  ಶೂನ್ಯ * clientData, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ nByte;         /* Size to pass to sqlite3_config() */
  ಪೂರ್ಣಾಙ್ಕ rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ಪೂರ್ಣಾಙ್ಕ nArg = objc-1;

  ಯದಿ( nArg!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "NBYTE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_error(
  ಶೂನ್ಯ * clientData, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp*, ಧ್ರುವ ವರ್ಣ*, sqlite3**);

  ಯದಿ( objc!=2 && objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "[DB]");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( objc==2 ){
    ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    ಯದಿ( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (ವರ್ಣ*)0);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
  }ಅನ್ಯಥಾ{
    ಯದಿ( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (ವರ್ಣ*)0);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_uri(
  ಶೂನ್ಯ * clientData, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ bOpenUri;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "BOOL");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_cis(
  ಶೂನ್ಯ * clientData, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ bUseCis;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "BOOL");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_config_pmasz(
  ಶೂನ್ಯ * clientData, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ iPmaSz;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "BOOL");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_dump_memsys3(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "FILENAME");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ವಿಕಲ್ಪ( SQLITE_PTR_TO_INT(clientData) ){
    ಪಕ್ಷ 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3Memsys3Dump(ಧ್ರುವ ವರ್ಣ*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      ಭಙ್ಗ;
#endif
    }
    ಪಕ್ಷ 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3Memsys5Dump(ಧ್ರುವ ವರ್ಣ*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      ಭಙ್ಗ;
#endif
    }
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_status(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc, i_ಮೌಲ್ಯ, mx_ಮೌಲ್ಯ;
  ಪೂರ್ಣಾಙ್ಕ i, op = 0, resetFlag;
  ಧ್ರುವ ವರ್ಣ *zOpName;
  ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
    ಧ್ರುವ ವರ್ಣ *zName;
    ಪೂರ್ಣಾಙ್ಕ op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_ರಾಶಿ        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_ಎಣಿಕೆ        },
  };
  Tcl_Obj *pResult;
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  ಪ್ರತಿ(i=0; i<ArraySize(aOp); i++){
    ಯದಿ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( i>=ArraySize(aOp) ){
    ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &op) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  i_ಮೌಲ್ಯ = 0;
  mx_ಮೌಲ್ಯ = 0;
  rc = sqlite3_status(op, &i_ಮೌಲ್ಯ, &mx_ಮೌಲ್ಯ, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_ಮೌಲ್ಯ));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_ಮೌಲ್ಯ));
  Tcl_SetObjResult(interp, pResult);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_db_status(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc, i_ಮೌಲ್ಯ, mx_ಮೌಲ್ಯ;
  ಪೂರ್ಣಾಙ್ಕ i, op = 0, resetFlag;
  ಧ್ರುವ ವರ್ಣ *zOpName;
  sqlite3 *db;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp*, ಧ್ರುವ ವರ್ಣ*, sqlite3**);
  ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
    ಧ್ರುವ ವರ್ಣ *zName;
    ಪೂರ್ಣಾಙ್ಕ op;
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
  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  ಯದಿ( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  ಯದಿ( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  ಪ್ರತಿ(i=0; i<ArraySize(aOp); i++){
    ಯದಿ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( i>=ArraySize(aOp) ){
    ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &op) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  i_ಮೌಲ್ಯ = 0;
  mx_ಮೌಲ್ಯ = 0;
  rc = sqlite3_db_status(db, op, &i_ಮೌಲ್ಯ, &mx_ಮೌಲ್ಯ, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_ಮೌಲ್ಯ));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_ಮೌಲ್ಯ));
  Tcl_SetObjResult(interp, pResult);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_install_malloc_faultsim(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ isInstall;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "BOOLEAN");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_install_memsys3(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  ಧ್ರುವ sqlite3_mem_methods *sqlite3MemGetMemsys3(ಶೂನ್ಯ);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_vfs_oom_test(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ sqlite3_memdebug_vfs_oom_test;
  ಯದಿ( objc>2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "?INTEGER?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }ಅನ್ಯಥಾ ಯದಿ( objc==2 ){
    ಪೂರ್ಣಾಙ್ಕ iNew;
    ಯದಿ( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetest_malloc_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಪೂರ್ಣಾಙ್ಕ clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_ಎಣಿಕೆ ,0 },
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
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
#endif
