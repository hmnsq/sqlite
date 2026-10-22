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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
ਸਥਿਰ ਬਣਤਰ MemFault {
  ਪੂਰਨ iCountdown;         /* Number of pending successes before a failure */
  ਪੂਰਨ nRepeat;            /* Number of times to repeat the failure */
  ਪੂਰਨ nBenign;            /* Number of benign failures seen since last config */
  ਪੂਰਨ nFail;              /* Number of failures seen since last config */
  ਪੂਰਨ nOkBefore;          /* Successful allocations prior to the first fault */
  ਪੂਰਨ nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  ਪੂਰਨ isInstalled;        /* True if the fault simulation layer is installed */
  ਪੂਰਨ is_Benign_ਬਹੁਲਕ;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3Fault(ਖਾਲੀ){
  ਸਥਿਰ u64 cnt = 0;
  cnt++;
  ਜੇ( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3_ਪਹਿਲਾ_Fault(ਖਾਲੀ){
  ਸਥਿਰ u64 cnt2 = 0;
  cnt2++;
  ਜੇ( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
ਸਥਿਰ ਪੂਰਨ faultsim_ਕਦਮ(ਖਾਲੀ){
  ਜੇ( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    ਵਾਪਸ 0;
  }
  ਜੇ( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    ਵਾਪਸ 0;
  }
  ਜੇ( memfault.nFail==0 ) sqlite3_ਪਹਿਲਾ_Fault();
  sqlite3Fault();
  memfault.nFail++;
  ਜੇ( memfault.is_Benign_ਬਹੁਲਕ>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  ਜੇ( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  ਵਾਪਸ 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
ਸਥਿਰ ਖਾਲੀ *faultsimMalloc(ਪੂਰਨ n){
  ਖਾਲੀ *p = 0;
  ਜੇ( !faultsim_ਕਦਮ() ){
    p = memfault.m.xMalloc(n);
  }
  ਵਾਪਸ p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
ਸਥਿਰ ਖਾਲੀ *faultsimRealloc(ਖਾਲੀ *pOld, ਪੂਰਨ n){
  ਖਾਲੀ *p = 0;
  ਜੇ( !faultsim_ਕਦਮ() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  ਵਾਪਸ p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
ਸਥਿਰ ਖਾਲੀ faultsimConfig(ਪੂਰਨ nDelay, ਪੂਰਨ nRepeat){
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
  memfault.is_Benign_ਬਹੁਲਕ = 0;
}

/*
** Return the number of faults (both hard and benign faults) that have
** occurred since the injector was last configured.
*/
ਸਥਿਰ ਪੂਰਨ faultsimFailures(ਖਾਲੀ){
  ਵਾਪਸ memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
ਸਥਿਰ ਪੂਰਨ faultsimBenignFailures(ਖਾਲੀ){
  ਵਾਪਸ memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
ਸਥਿਰ ਪੂਰਨ faultsimPending(ਖਾਲੀ){
  ਜੇ( memfault.enable ){
    ਵਾਪਸ memfault.iCountdown;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ -1;
  }
}


ਸਥਿਰ ਖਾਲੀ faultsimBeginBenign(ਖਾਲੀ){
  memfault.is_Benign_ਬਹੁਲਕ++;
}
ਸਥਿਰ ਖਾਲੀ faultsim_ਸਿਰਾ_Benign(ਖਾਲੀ){
  memfault.is_Benign_ਬਹੁਲਕ--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
ਸਥਿਰ ਪੂਰਨ faultsimInstall(ਪੂਰਨ install){
  ਪੂਰਨ rc;

  install = (install ? 1 : 0);
  ਪ੍ਰਤਿਜ੍ਞਾ(memfault.isInstalled==1 || memfault.isInstalled==0);

  ਜੇ( install==memfault.isInstalled ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  ਜੇ( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    ਪ੍ਰਤਿਜ੍ਞਾ(memfault.m.xMalloc);
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsim_ਸਿਰਾ_Benign
    );
  }ਨਹੀਂਤਾਂ{
    sqlite3_mem_methods m2;
    ਪ੍ਰਤਿਜ੍ਞਾ(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, ਆਕਾਰ(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    ਪ੍ਰਤਿਜ੍ਞਾ( memcmp(&m2, &memfault.m, ਆਕਾਰ(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (ਖਾਲੀ*)0, (ਖਾਲੀ*)0);
  }

  ਜੇ( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST

/*
** This function is implemented in main.c. Returns a pointer to a static
** buffer containing the symbolic SQLite error code that corresponds to
** the least-significant 8-bits of the integer passed as an argument.
** For example:
**
**   sqlite3ErrName(1) -> "SQLITE_ERROR"
*/
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/*
** Transform pointers to text and back again
*/
ਸਥਿਰ ਖਾਲੀ ਸੂਚਕ_ਮੰਜ਼ਿਲ_ਪਾਠ(ਖਾਲੀ *p, ਅੱਖਰ *z){
  ਅਚਲ ਅੱਖਰ *zRet = sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(p);
  sprintf(z, "%s", zRet);
}
ਸਥਿਰ ਪੂਰਨ ਪਾਠ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(ਅਚਲ ਅੱਖਰ *z, ਖਾਲੀ **pp){
  *pp = sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(z);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_malloc(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ nByte;
  ਖਾਲੀ *p;
  ਅੱਖਰ z_ਬਾਹਰਲਾ[100];
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "NBYTES");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) ਵਾਪਸ TCL_ERROR;
  p = sqlite3_malloc((ਬਿਨਾਨਿਸ਼ਾਨੀ)nByte);
  ਸੂਚਕ_ਮੰਜ਼ਿਲ_ਪਾਠ(p, z_ਬਾਹਰਲਾ);
  Tcl_Append_ਨਤੀਜਾ(interp, z_ਬਾਹਰਲਾ, ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_realloc(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ nByte;
  ਖਾਲੀ *p_ਪੂਰਵ, *p;
  ਅੱਖਰ z_ਬਾਹਰਲਾ[100];
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "PRIOR NBYTES");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( ਪਾਠ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(Tcl_GetString(objv[1]), &p_ਪੂਰਵ) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  p = sqlite3_realloc(p_ਪੂਰਵ, (ਬਿਨਾਨਿਸ਼ਾਨੀ)nByte);
  ਸੂਚਕ_ਮੰਜ਼ਿਲ_ਪਾਠ(p, z_ਬਾਹਰਲਾ);
  Tcl_Append_ਨਤੀਜਾ(interp, z_ਬਾਹਰਲਾ, ਨਿਰਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_free(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਖਾਲੀ *p_ਪੂਰਵ;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "PRIOR");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( ਪਾਠ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(Tcl_GetString(objv[1]), &p_ਪੂਰਵ) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  sqlite3_free(p_ਪੂਰਵ);
  ਵਾਪਸ TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
ਪੂਰਨ sqlite3_Test_Hex_ਮੰਜ਼ਿਲ_Bin(ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ, ਅੱਖਰ *);
ਪੂਰਨ sqlite3_Test_Bin_ਮੰਜ਼ਿਲ_Hex(ਅੱਖਰ*,ਪੂਰਨ);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memset(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਖਾਲੀ *p;
  ਪੂਰਨ ਮਾਪ, i;
  Tcl_ਮਾਪ n;
  ਅੱਖਰ *zHex;
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;
  ਅੱਖਰ zBin[100];

  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "ADDRESS SIZE HEX");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( ਪਾਠ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &ਮਾਪ) ){
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( ਮਾਪ<=0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "size must be positive", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  ਜੇ( n>ਆਕਾਰ(zBin)*2 ) n = ਆਕਾਰ(zBin)*2;
  n = sqlite3_Test_Hex_ਮੰਜ਼ਿਲ_Bin(zHex, (ਪੂਰਨ)n, zBin);
  ਜੇ( n==0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "no data", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  z_ਬਾਹਰਲਾ = p;
  ਲਈ(i=0; i<ਮਾਪ; i++){
    z_ਬਾਹਰਲਾ[i] = zBin[i%n];
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memget(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਖਾਲੀ *p;
  ਪੂਰਨ ਮਾਪ, n;
  ਅੱਖਰ *zBin;
  ਅੱਖਰ zHex[100];

  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "ADDRESS SIZE");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( ਪਾਠ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_ਨਤੀਜਾ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &ਮਾਪ) ){
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( ਮਾਪ<=0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "size must be positive", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  zBin = p;
  ਜਦੋਂ( ਮਾਪ>0 ){
    ਜੇ( ਮਾਪ>(ਆਕਾਰ(zHex)-1)/2 ){
      n = (ਆਕਾਰ(zHex)-1)/2;
    }ਨਹੀਂਤਾਂ{
      n = ਮਾਪ;
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zHex, zBin, n);
    zBin += n;
    ਮਾਪ -= n;
    sqlite3_Test_Bin_ਮੰਜ਼ਿਲ_Hex(zHex, n);
    Tcl_Append_ਨਤੀਜਾ(interp, zHex, (ਅੱਖਰ*)0);
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewWideIntObj(sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ()));
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_ਸਮ੍ਰਿਤੀ_highwater(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ resetFlag = 0;
  ਜੇ( objc!=1 && objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "?RESET?");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( objc==2 ){
    ਜੇ( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) ਵਾਪਸ TCL_ERROR;
  } 
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, 
     Tcl_NewWideIntObj(sqlite3_ਸਮ੍ਰਿਤੀ_highwater(resetFlag)));
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memdebug_backtrace(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ ਡੂੰਘਾਈ;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DEPT");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &ਡੂੰਘਾਈ) ) ਵਾਪਸ TCL_ERROR;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MEMDEBUG
  {
    ਬਾਹਰੀ ਖਾਲੀ sqlite3MemdebugBacktrace(ਪੂਰਨ);
    sqlite3MemdebugBacktrace(ਡੂੰਘਾਈ);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memdebug_dump(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "FILENAME");
    ਵਾਪਸ TCL_ERROR;
  }
#ਜੇ defined(SQLITE_MEMDEBUG) || defined(SQLITE_ਸਮ੍ਰਿਤੀ_ਮਾਪ) \
     || defined(SQLITE_POW2_ਸਮ੍ਰਿਤੀ_ਮਾਪ)
  {
    ਬਾਹਰੀ ਖਾਲੀ sqlite3MemdebugDump(ਅਚਲ ਅੱਖਰ*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memdebug_malloc_ਗਿਣਤੀ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ nMalloc = -1;
  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }
#ਜੇ defined(SQLITE_MEMDEBUG)
  {
    ਬਾਹਰੀ ਪੂਰਨ sqlite3_Memdebug_Malloc_ਗਿਣਤੀ();
    nMalloc = sqlite3_Memdebug_Malloc_ਗਿਣਤੀ();
  }
#ਜੇ_ਅੰਤ
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(nMalloc));
  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memdebug_fail(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ ii;
  ਪੂਰਨ iFail;
  ਪੂਰਨ nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  ਪੂਰਨ nBenign;
  ਪੂਰਨ nFail = 0;

  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "COUNTER ?OPTIONS?");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) ਵਾਪਸ TCL_ERROR;

  ਲਈ(ii=2; ii<objc; ii+=2){
    Tcl_ਮਾਪ nOption;
    ਅੱਖਰ *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    ਅੱਖਰ *zErr = 0;

    ਜੇ( nOption>1 && ਲੜੀ_ਤੁਲਨਾ(zOption, "-repeat", nOption)==0 ){
      ਜੇ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ਨਹੀਂਤਾਂ{
        ਜੇ( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          ਵਾਪਸ TCL_ERROR;
        }
      }
    }ਨਹੀਂਤਾਂ ਜੇ( nOption>1 && ਲੜੀ_ਤੁਲਨਾ(zOption, "-benigncnt", nOption)==0 ){
      ਜੇ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ਨਹੀਂਤਾਂ{
        pBenignCnt = objv[ii+1];
      }
    }ਨਹੀਂਤਾਂ{
      zErr = "unknown option: ";
    }

    ਜੇ( zErr ){
      Tcl_Append_ਨਤੀਜਾ(interp, zErr, zOption, ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  ਜੇ( pBenignCnt ){
    Tcl_Obj_ਸੈੱਟ_Var2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(nFail));
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memdebug_pending(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ nPending;
  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(nPending));
  ਵਾਪਸ TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3_memdebug_title_ਗਿਣਤੀ = 0;

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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memdebug_settitle(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_ਗਿਣਤੀ++;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "TITLE");
    ਵਾਪਸ TCL_ERROR;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MEMDEBUG
  {
    ਅਚਲ ਅੱਖਰ *zTitle;
    ਬਾਹਰੀ ਪੂਰਨ sqlite3MemdebugSettitle(ਅਚਲ ਅੱਖਰ*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_LOG_ਚੌਖਟੇ  10 
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_LOG_KEYINTS (                                              \
    10 * ((ਆਕਾਰ(ਪੂਰਨ)>=ਆਕਾਰ(ਖਾਲੀ*)) ? 1 : ਆਕਾਰ(ਖਾਲੀ*)/ਆਕਾਰ(ਪੂਰਨ))   \
)
ਸਥਿਰ Tcl_Hash_ਸਾਰਣੀ aMallocLog;
ਸਥਿਰ ਪੂਰਨ mallocLogEnabled = 0;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ MallocLog MallocLog;
ਬਣਤਰ MallocLog {
  ਪੂਰਨ nCall;
  ਪੂਰਨ nByte;
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MEMDEBUG
ਸਥਿਰ ਖਾਲੀ test_memdebug_callback(ਪੂਰਨ nByte, ਪੂਰਨ n_ਚੌਖਟਾ, ਖਾਲੀ **a_ਚੌਖਟਾ){
  ਜੇ( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_Hash_ਇੰਦਰਾਜ *p_ਇੰਦਰਾਜ;
    ਪੂਰਨ isNew;

    ਪੂਰਨ a_ਕੁੰਜੀ[MALLOC_LOG_KEYINTS];
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n_ਕੁੰਜੀ = ਆਕਾਰ(ਪੂਰਨ)*MALLOC_LOG_KEYINTS;

    memset(a_ਕੁੰਜੀ, 0, n_ਕੁੰਜੀ);
    ਜੇ( (ਆਕਾਰ(ਖਾਲੀ*)*n_ਚੌਖਟਾ)<n_ਕੁੰਜੀ ){
      n_ਕੁੰਜੀ = n_ਚੌਖਟਾ*ਆਕਾਰ(ਖਾਲੀ*);
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(a_ਕੁੰਜੀ, a_ਚੌਖਟਾ, n_ਕੁੰਜੀ);

    p_ਇੰਦਰਾਜ = Tcl_Create_Hash_ਇੰਦਰਾਜ(&aMallocLog, (ਅਚਲ ਅੱਖਰ *)a_ਕੁੰਜੀ, &isNew);
    ਜੇ( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(ਆਕਾਰ(MallocLog));
      memset(pLog, 0, ਆਕਾਰ(MallocLog));
      Tcl_ਸੈੱਟ_Hash_ਮੁੱਲ(p_ਇੰਦਰਾਜ, (Client_ਅੰਕੜੇ)pLog);
    }ਨਹੀਂਤਾਂ{
      pLog = (MallocLog *)Tcl_Get_Hash_ਮੁੱਲ(p_ਇੰਦਰਾਜ);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_MEMDEBUG */

ਸਥਿਰ ਖਾਲੀ test_memdebug_log_clear(ਖਾਲੀ){
  Tcl_Hash_ਖੋਜ ਖੋਜ;
  Tcl_Hash_ਇੰਦਰਾਜ *p_ਇੰਦਰਾਜ;
  ਲਈ(
    p_ਇੰਦਰਾਜ=Tcl_ਪਹਿਲਾ_Hash_ਇੰਦਰਾਜ(&aMallocLog, &ਖੋਜ);
    p_ਇੰਦਰਾਜ;
    p_ਇੰਦਰਾਜ=Tcl_ਅਗਲਾ_Hash_ਇੰਦਰਾਜ(&ਖੋਜ)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ਮੁੱਲ(p_ਇੰਦਰਾਜ);
    Tcl_Free((ਅੱਖਰ *)pLog);
  }
  Tcl_Delete_Hash_ਸਾਰਣੀ(&aMallocLog);
  Tcl_Init_Hash_ਸਾਰਣੀ(&aMallocLog, MALLOC_LOG_KEYINTS);
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_memdebug_log(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਸਥਿਰ ਪੂਰਨ isInit = 0;
  ਪੂਰਨ iSub;

  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  ਗਿਣਤੀ MB_enum { 
      MB_LOG_ਸ਼ੁਰੂ, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  ਜੇ( !isInit ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MEMDEBUG
    ਬਾਹਰੀ ਖਾਲੀ sqlite3MemdebugBacktraceCallback(
        ਖਾਲੀ (*xBacktrace)(ਪੂਰਨ, ਪੂਰਨ, ਖਾਲੀ **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#ਜੇ_ਅੰਤ
    Tcl_Init_Hash_ਸਾਰਣੀ(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SUB-COMMAND ...");
  }
  ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਚੋਣ( (ਗਿਣਤੀ MB_enum)iSub ){
    ਹਾਲਤ MB_LOG_ਸ਼ੁਰੂ:
      mallocLogEnabled = 1;
      ਰੋਕੋ;
    ਹਾਲਤ MB_LOG_STOP:
      mallocLogEnabled = 0;
      ਰੋਕੋ;
    ਹਾਲਤ MB_LOG_DUMP: {
      Tcl_Hash_ਖੋਜ ਖੋਜ;
      Tcl_Hash_ਇੰਦਰਾਜ *p_ਇੰਦਰਾਜ;
      Tcl_Obj *pRet = Tcl_NewObj();

      ਪ੍ਰਤਿਜ੍ਞਾ(ਆਕਾਰ(Tcl_WideInt)>=ਆਕਾਰ(ਖਾਲੀ*));

      ਲਈ(
        p_ਇੰਦਰਾਜ=Tcl_ਪਹਿਲਾ_Hash_ਇੰਦਰਾਜ(&aMallocLog, &ਖੋਜ);
        p_ਇੰਦਰਾਜ;
        p_ਇੰਦਰਾਜ=Tcl_ਅਗਲਾ_Hash_ਇੰਦਰਾਜ(&ਖੋਜ)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_ਚੌਖਟੇ+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ਮੁੱਲ(p_ਇੰਦਰਾਜ);
        Tcl_WideInt *a_ਕੁੰਜੀ = (Tcl_WideInt *)Tcl_Get_Hash_ਕੁੰਜੀ(&aMallocLog, p_ਇੰਦਰਾਜ);
        ਪੂਰਨ ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        ਲਈ(ii=0; ii<MALLOC_LOG_ਚੌਖਟੇ; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_ਕੁੰਜੀ[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_ਚੌਖਟੇ+2, apElem)
        );
      }

      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);
      ਰੋਕੋ;
    }
    ਹਾਲਤ MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      ਰੋਕੋ;
    }

    ਹਾਲਤ MB_LOG_SYNC: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MEMDEBUG
      ਬਾਹਰੀ ਖਾਲੀ sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_pagecache(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ sz, N;
  Tcl_Obj *pRes;
  ਸਥਿਰ ਅੱਖਰ *buf = 0;
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SIZE N");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &N) ) ਵਾਪਸ TCL_ERROR;
  ਮੁਕਤ(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.sz_ਪੰਨਾ));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.n_ਪੰਨਾ));
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRes);

  ਜੇ( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (ਖਾਲੀ*)0, 0, 0);
  }ਨਹੀਂਤਾਂ{
    buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  ਵਾਪਸ TCL_OK;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_alt_pcache(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ installFlag;
  ਪੂਰਨ discardChance = 0;
  ਪੂਰਨ prng_ਬੀਜ_2 = 0;
  ਪੂਰਨ ਉੱਚਾ_Stress = 0;
  ਬਾਹਰੀ ਖਾਲੀ installTestPCache(ਪੂਰਨ,ਬਿਨਾਨਿਸ਼ਾਨੀ,ਬਿਨਾਨਿਸ਼ਾਨੀ,ਬਿਨਾਨਿਸ਼ਾਨੀ);
  ਜੇ( objc<2 || objc>5 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prng_ਬੀਜ_2) ){
     ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &ਉੱਚਾ_Stress) ){
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( discardChance<0 || discardChance>100 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "discard-chance should be between 0 and 100",
                     (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  installTestPCache(installFlag, (ਬਿਨਾਨਿਸ਼ਾਨੀ)discardChance, (ਬਿਨਾਨਿਸ਼ਾਨੀ)prng_ਬੀਜ_2,
                    (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਉੱਚਾ_Stress);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_memstatus(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ enable, rc;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "BOOLEAN");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) ਵਾਪਸ TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_lookaside(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ sz, cnt;
  Tcl_Obj *pRet;
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SIZE COUNT");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) ਵਾਪਸ TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);
  ਵਾਪਸ TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_db_config_lookaside(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;
  ਪੂਰਨ sz, cnt;
  sqlite3 *db;
  ਪੂਰਨ bufid;
  ਸਥਿਰ ਅੱਖਰ azBuf[2][10000];
  ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp*, ਅਚਲ ਅੱਖਰ*, sqlite3**);
  ਜੇ( objc!=5 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "BUFID SIZE COUNT");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[3], &sz) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (ਖਾਲੀ*)0, sz, cnt);
  }ਨਹੀਂਤਾਂ ਜੇ( bufid>=1 && bufid<=2 && sz*cnt<=ਆਕਾਰ(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }ਨਹੀਂਤਾਂ{
    Tcl_Append_ਨਤੀਜਾ(interp, "illegal arguments - see documentation", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(rc));
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_ਢੇਰ(
  ਖਾਲੀ * client_ਅੰਕੜੇ, 
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਸਥਿਰ ਅੱਖਰ *zBuf; /* Use this memory */
  ਪੂਰਨ nByte;         /* Size of buffer to pass to sqlite3_config() */
  ਪੂਰਨ nMinAlloc;     /* Size of minimum allocation */
  ਪੂਰਨ rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ਪੂਰਨ nArg = objc-1;

  ਜੇ( nArg!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "NBYTE NMINALLOC");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ਵਾਪਸ TCL_ERROR;
  ਜੇ( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) ਵਾਪਸ TCL_ERROR;

  ਜੇ( nByte==0 ){
    ਮੁਕਤ( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_ਢੇਰ, (ਖਾਲੀ*)0, 0, 0);
  }ਨਹੀਂਤਾਂ{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_ਢੇਰ, zBuf, nByte, nMinAlloc);
  }

  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_ਢੇਰ_ਮਾਪ(
  ਖਾਲੀ * client_ਅੰਕੜੇ, 
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ nByte;         /* Size to pass to sqlite3_config() */
  ਪੂਰਨ rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ਪੂਰਨ nArg = objc-1;

  ਜੇ( nArg!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "NBYTE");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ਵਾਪਸ TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_error(
  ਖਾਲੀ * client_ਅੰਕੜੇ, 
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp*, ਅਚਲ ਅੱਖਰ*, sqlite3**);

  ਜੇ( objc!=2 && objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "[DB]");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( objc==2 ){
    ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
    ਜੇ( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_Append_ਨਤੀਜਾ(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_Append_ਨਤੀਜਾ(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_uri(
  ਖਾਲੀ * client_ਅੰਕੜੇ, 
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;
  ਪੂਰਨ bOpenUri;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "BOOL");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    ਵਾਪਸ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_cis(
  ਖਾਲੀ * client_ਅੰਕੜੇ, 
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;
  ਪੂਰਨ bUseCis;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "BOOL");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    ਵਾਪਸ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_ਸੂਚਕਾਂਕ_ਜਾਂਚ, bUseCis);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_config_pmasz(
  ਖਾਲੀ * client_ਅੰਕੜੇ, 
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;
  ਪੂਰਨ iPmaSz;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "BOOL");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    ਵਾਪਸ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ਵਾਪਸ TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_dump_memsys3(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "FILENAME");
    ਵਾਪਸ TCL_ERROR;
  }

  ਚੋਣ( SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(client_ਅੰਕੜੇ) ){
    ਹਾਲਤ 3: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS3
      ਬਾਹਰੀ ਖਾਲੀ sqlite3Memsys3Dump(ਅਚਲ ਅੱਖਰ*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ
    }
    ਹਾਲਤ 5: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS5
      ਬਾਹਰੀ ਖਾਲੀ sqlite3Memsys5Dump(ਅਚਲ ਅੱਖਰ*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ
    }
  }
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_ਸਥਿਤੀ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc, i_ਮੁੱਲ, mx_ਮੁੱਲ;
  ਪੂਰਨ i, op = 0, resetFlag;
  ਅਚਲ ਅੱਖਰ *zOpName;
  ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
    ਅਚਲ ਅੱਖਰ *zName;
    ਪੂਰਨ op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_ਸਥਿਤੀ_MALLOC_ਮਾਪ         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_ਸਥਿਤੀ_PAGECACHE_ਵਰਤਿਆ      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_ਸਥਿਤੀ_PAGECACHE_ਉਛਾਲ  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_ਸਥਿਤੀ_PAGECACHE_ਮਾਪ      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_ਸਥਿਤੀ_SCRATCH_ਵਰਤਿਆ        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_ਸਥਿਤੀ_SCRATCH_ਉਛਾਲ    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_ਸਥਿਤੀ_SCRATCH_ਮਾਪ        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_ਸਥਿਤੀ_PARSER_ਢੇਰ        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_ਸਥਿਤੀ_MALLOC_ਗਿਣਤੀ        },
  };
  Tcl_Obj *p_ਨਤੀਜਾ;
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "PARAMETER RESETFLAG");
    ਵਾਪਸ TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  ਲਈ(i=0; i<Array_ਮਾਪ(aOp); i++){
    ਜੇ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ਰੋਕੋ;
    }
  }
  ਜੇ( i>=Array_ਮਾਪ(aOp) ){
    ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &op) ) ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) ਵਾਪਸ TCL_ERROR;
  i_ਮੁੱਲ = 0;
  mx_ਮੁੱਲ = 0;
  rc = sqlite3_ਸਥਿਤੀ(op, &i_ਮੁੱਲ, &mx_ਮੁੱਲ, resetFlag);
  p_ਨਤੀਜਾ = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_ਨਤੀਜਾ, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_ਨਤੀਜਾ, Tcl_NewIntObj(i_ਮੁੱਲ));
  Tcl_ListObjAppendElement(0, p_ਨਤੀਜਾ, Tcl_NewIntObj(mx_ਮੁੱਲ));
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, p_ਨਤੀਜਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_db_ਸਥਿਤੀ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc, i_ਮੁੱਲ, mx_ਮੁੱਲ;
  ਪੂਰਨ i, op = 0, resetFlag;
  ਅਚਲ ਅੱਖਰ *zOpName;
  sqlite3 *db;
  ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp*, ਅਚਲ ਅੱਖਰ*, sqlite3**);
  ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
    ਅਚਲ ਅੱਖਰ *zName;
    ਪੂਰਨ op;
  } aOp[] = {
    { "LOOKASIDE_USED",      SQLITE_DBSTATUS_LOOKASIDE_ਵਰਤਿਆ      },
    { "CACHE_USED",          SQLITE_DBSTATUS_CACHE_ਵਰਤਿਆ          },
    { "SCHEMA_USED",         SQLITE_DBSTATUS_SCHEMA_ਵਰਤਿਆ         },
    { "STMT_USED",           SQLITE_DBSTATUS_STMT_ਵਰਤਿਆ           },
    { "LOOKASIDE_HIT",       SQLITE_DBSTATUS_LOOKASIDE_ਪ੍ਰਾਪਤੀ       },
    { "LOOKASIDE_MISS_SIZE", SQLITE_DBSTATUS_LOOKASIDE_ਖੁੰਝ_ਮਾਪ },
    { "LOOKASIDE_MISS_FULL", SQLITE_DBSTATUS_LOOKASIDE_ਖੁੰਝ_ਭਰਿਆ },
    { "CACHE_HIT",           SQLITE_DBSTATUS_CACHE_ਪ੍ਰਾਪਤੀ           },
    { "CACHE_MISS",          SQLITE_DBSTATUS_CACHE_ਖੁੰਝ          },
    { "CACHE_WRITE",         SQLITE_DBSTATUS_CACHE_WRITE         },
    { "DEFERRED_FKS",        SQLITE_DBSTATUS_DEFERRED_FKS        },
    { "CACHE_USED_SHARED",   SQLITE_DBSTATUS_CACHE_ਵਰਤਿਆ_SHARED   },
    { "CACHE_SPILL",         SQLITE_DBSTATUS_CACHE_SPILL         },
    { "TEMPBUF_SPILL",       SQLITE_DBSTATUS_TEMPBUF_SPILL       },
  };
  Tcl_Obj *p_ਨਤੀਜਾ;
  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB PARAMETER RESETFLAG");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  ਜੇ( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  ਜੇ( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  ਲਈ(i=0; i<Array_ਮਾਪ(aOp); i++){
    ਜੇ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ਰੋਕੋ;
    }
  }
  ਜੇ( i>=Array_ਮਾਪ(aOp) ){
    ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &op) ) ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) ਵਾਪਸ TCL_ERROR;
  i_ਮੁੱਲ = 0;
  mx_ਮੁੱਲ = 0;
  rc = sqlite3_db_ਸਥਿਤੀ(db, op, &i_ਮੁੱਲ, &mx_ਮੁੱਲ, resetFlag);
  p_ਨਤੀਜਾ = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_ਨਤੀਜਾ, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_ਨਤੀਜਾ, Tcl_NewIntObj(i_ਮੁੱਲ));
  Tcl_ListObjAppendElement(0, p_ਨਤੀਜਾ, Tcl_NewIntObj(mx_ਮੁੱਲ));
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, p_ਨਤੀਜਾ);
  ਵਾਪਸ TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_install_malloc_faultsim(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc;
  ਪੂਰਨ isInstall;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "BOOLEAN");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ਵਾਪਸ TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_install_memsys3(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ rc = SQLITE_MISUSE;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS3
  ਅਚਲ sqlite3_mem_methods *sqlite3MemGetMemsys3(ਖਾਲੀ);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#ਜੇ_ਅੰਤ
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_vfs_oom_test(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਬਾਹਰੀ ਪੂਰਨ sqlite3_memdebug_vfs_oom_test;
  ਜੇ( objc>2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "?INTEGER?");
    ਵਾਪਸ TCL_ERROR;
  }ਨਹੀਂਤਾਂ ਜੇ( objc==2 ){
    ਪੂਰਨ iNew;
    ਜੇ( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) ਵਾਪਸ TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  ਵਾਪਸ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest_malloc_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਪੂਰਨ client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "sqlite3_malloc",             test_malloc                   ,0 },
     { "sqlite3_realloc",            test_realloc                  ,0 },
     { "sqlite3_free",               test_free                     ,0 },
     { "memset",                     test_memset                   ,0 },
     { "memget",                     test_memget                   ,0 },
     { "sqlite3_memory_used",        test_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ              ,0 },
     { "sqlite3_memory_highwater",   test_ਸਮ੍ਰਿਤੀ_highwater         ,0 },
     { "sqlite3_memdebug_backtrace", test_memdebug_backtrace       ,0 },
     { "sqlite3_memdebug_dump",      test_memdebug_dump            ,0 },
     { "sqlite3_memdebug_fail",      test_memdebug_fail            ,0 },
     { "sqlite3_memdebug_pending",   test_memdebug_pending         ,0 },
     { "sqlite3_memdebug_settitle",  test_memdebug_settitle        ,0 },
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_ਗਿਣਤੀ ,0 },
     { "sqlite3_memdebug_log",       test_memdebug_log             ,0 },
     { "sqlite3_config_pagecache",   test_config_pagecache         ,0 },
     { "sqlite3_config_alt_pcache",  test_alt_pcache               ,0 },
     { "sqlite3_status",             test_ਸਥਿਤੀ                   ,0 },
     { "sqlite3_db_status",          test_db_ਸਥਿਤੀ                ,0 },
     { "install_malloc_faultsim",    test_install_malloc_faultsim  ,0 },
     { "sqlite3_config_heap",        test_config_ਢੇਰ              ,0 },
     { "sqlite3_config_heap_size",   test_config_ਢੇਰ_ਮਾਪ         ,0 },
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
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Client_ਅੰਕੜੇ c = (Client_ਅੰਕੜੇ)SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(aObjCmd[i].client_ਅੰਕੜੇ);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ਵਾਪਸ TCL_OK;
}
#ਜੇ_ਅੰਤ
