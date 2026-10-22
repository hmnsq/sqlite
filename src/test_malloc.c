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
ಸ್ಥಿರ ರಚನೆ MemFault {
  ಪೂರ್ಣಾಂಕ iCountdown;         /* Number of pending successes before a failure */
  ಪೂರ್ಣಾಂಕ nRepeat;            /* Number of times to repeat the failure */
  ಪೂರ್ಣಾಂಕ nBenign;            /* Number of benign failures seen since last config */
  ಪೂರ್ಣಾಂಕ nFail;              /* Number of failures seen since last config */
  ಪೂರ್ಣಾಂಕ nOkBefore;          /* Successful allocations prior to the first fault */
  ಪೂರ್ಣಾಂಕ nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  ಪೂರ್ಣಾಂಕ isInstalled;        /* True if the fault simulation layer is installed */
  ಪೂರ್ಣಾಂಕ is_Benign_ಬಹುಲಕ;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3Fault(ಶೂನ್ಯ){
  ಸ್ಥಿರ u64 cnt = 0;
  cnt++;
  ಆದರೆ( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ಮೊದಲ_Fault(ಶೂನ್ಯ){
  ಸ್ಥಿರ u64 cnt2 = 0;
  cnt2++;
  ಆದರೆ( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ faultsim_ಹೆಜ್ಜೆ(ಶೂನ್ಯ){
  ಆದರೆ( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( memfault.nFail==0 ) sqlite3_ಮೊದಲ_Fault();
  sqlite3Fault();
  memfault.nFail++;
  ಆದರೆ( memfault.is_Benign_ಬಹುಲಕ>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  ಆದರೆ( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  ಹಿಂತಿರುಗಿ 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *faultsimMalloc(ಪೂರ್ಣಾಂಕ n){
  ಶೂನ್ಯ *p = 0;
  ಆದರೆ( !faultsim_ಹೆಜ್ಜೆ() ){
    p = memfault.m.xMalloc(n);
  }
  ಹಿಂತಿರುಗಿ p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *faultsimRealloc(ಶೂನ್ಯ *pOld, ಪೂರ್ಣಾಂಕ n){
  ಶೂನ್ಯ *p = 0;
  ಆದರೆ( !faultsim_ಹೆಜ್ಜೆ() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
ಸ್ಥಿರ ಶೂನ್ಯ faultsimConfig(ಪೂರ್ಣಾಂಕ nDelay, ಪೂರ್ಣಾಂಕ nRepeat){
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
  memfault.is_Benign_ಬಹುಲಕ = 0;
}

/*
** Return the number of faults (both hard and benign faults) that have
** occurred since the injector was last configured.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ faultsimFailures(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ faultsimBenignFailures(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ faultsimPending(ಶೂನ್ಯ){
  ಆದರೆ( memfault.enable ){
    ಹಿಂತಿರುಗಿ memfault.iCountdown;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ -1;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ faultsimBeginBenign(ಶೂನ್ಯ){
  memfault.is_Benign_ಬಹುಲಕ++;
}
ಸ್ಥಿರ ಶೂನ್ಯ faultsim_ಕೊನೆ_Benign(ಶೂನ್ಯ){
  memfault.is_Benign_ಬಹುಲಕ--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ faultsimInstall(ಪೂರ್ಣಾಂಕ install){
  ಪೂರ್ಣಾಂಕ rc;

  install = (install ? 1 : 0);
  ಪ್ರತಿಜ್ಞಾ(memfault.isInstalled==1 || memfault.isInstalled==0);

  ಆದರೆ( install==memfault.isInstalled ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  ಆದರೆ( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    ಪ್ರತಿಜ್ಞಾ(memfault.m.xMalloc);
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsim_ಕೊನೆ_Benign
    );
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_mem_methods m2;
    ಪ್ರತಿಜ್ಞಾ(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, ಗಾತ್ರ(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    ಪ್ರತಿಜ್ಞಾ( memcmp(&m2, &memfault.m, ಗಾತ್ರ(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (ಶೂನ್ಯ*)0, (ಶೂನ್ಯ*)0);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST

/*
** This function is implemented in main.c. Returns a pointer to a static
** buffer containing the symbolic SQLite error code that corresponds to
** the least-significant 8-bits of the integer passed as an argument.
** For example:
**
**   sqlite3ErrName(1) -> "SQLITE_ERROR"
*/
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/*
** Transform pointers to text and back again
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸೂಚಕ_ಗಮ್ಯ_ಪಠ್ಯ(ಶೂನ್ಯ *p, ಅಕ್ಷರ *z){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRet = sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(p);
  sprintf(z, "%s", zRet);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಪಠ್ಯ_ಗಮ್ಯ_ಸೂಚಕ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಶೂನ್ಯ **pp){
  *pp = sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(z);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_malloc(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ nByte;
  ಶೂನ್ಯ *p;
  ಅಕ್ಷರ z_ಹೊರಹೋಗು[100];
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "NBYTES");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  p = sqlite3_malloc((ಚಿಹ್ನೆರಹಿತ)nByte);
  ಸೂಚಕ_ಗಮ್ಯ_ಪಠ್ಯ(p, z_ಹೊರಹೋಗು);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, z_ಹೊರಹೋಗು, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_realloc(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ nByte;
  ಶೂನ್ಯ *p_ಪೂರ್ವ, *p;
  ಅಕ್ಷರ z_ಹೊರಹೋಗು[100];
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "PRIOR NBYTES");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( ಪಠ್ಯ_ಗಮ್ಯ_ಸೂಚಕ(Tcl_GetString(objv[1]), &p_ಪೂರ್ವ) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p = sqlite3_realloc(p_ಪೂರ್ವ, (ಚಿಹ್ನೆರಹಿತ)nByte);
  ಸೂಚಕ_ಗಮ್ಯ_ಪಠ್ಯ(p, z_ಹೊರಹೋಗು);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, z_ಹೊರಹೋಗು, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_free(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ *p_ಪೂರ್ವ;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "PRIOR");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( ಪಠ್ಯ_ಗಮ್ಯ_ಸೂಚಕ(Tcl_GetString(objv[1]), &p_ಪೂರ್ವ) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  sqlite3_free(p_ಪೂರ್ವ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
ಪೂರ್ಣಾಂಕ sqlite3_Test_Hex_ಗಮ್ಯ_Bin(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *);
ಪೂರ್ಣಾಂಕ sqlite3_Test_Bin_ಗಮ್ಯ_Hex(ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memset(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ *p;
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, i;
  Tcl_ಗಾತ್ರ n;
  ಅಕ್ಷರ *zHex;
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;
  ಅಕ್ಷರ zBin[100];

  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "ADDRESS SIZE HEX");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( ಪಠ್ಯ_ಗಮ್ಯ_ಸೂಚಕ(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &ಗಾತ್ರ_2) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( ಗಾತ್ರ_2<=0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "size must be positive", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  ಆದರೆ( n>ಗಾತ್ರ(zBin)*2 ) n = ಗಾತ್ರ(zBin)*2;
  n = sqlite3_Test_Hex_ಗಮ್ಯ_Bin(zHex, (ಪೂರ್ಣಾಂಕ)n, zBin);
  ಆದರೆ( n==0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "no data", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  z_ಹೊರಹೋಗು = p;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ_2; i++){
    z_ಹೊರಹೋಗು[i] = zBin[i%n];
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memget(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ *p;
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, n;
  ಅಕ್ಷರ *zBin;
  ಅಕ್ಷರ zHex[100];

  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "ADDRESS SIZE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( ಪಠ್ಯ_ಗಮ್ಯ_ಸೂಚಕ(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &ಗಾತ್ರ_2) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( ಗಾತ್ರ_2<=0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "size must be positive", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zBin = p;
  ಆಗ( ಗಾತ್ರ_2>0 ){
    ಆದರೆ( ಗಾತ್ರ_2>(ಗಾತ್ರ(zHex)-1)/2 ){
      n = (ಗಾತ್ರ(zHex)-1)/2;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n = ಗಾತ್ರ_2;
    }
    ಸ್ಮರಣೆ_ನಕಲು(zHex, zBin, n);
    zBin += n;
    ಗಾತ್ರ_2 -= n;
    sqlite3_Test_Bin_ಗಮ್ಯ_Hex(zHex, n);
    Tcl_Append_ಫಲಿತಾಂಶ(interp, zHex, (ಅಕ್ಷರ*)0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_ಸ್ಮರಣೆ_ಬಳಸಿದ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewWideIntObj(sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ()));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_ಸ್ಮರಣೆ_highwater(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ resetFlag = 0;
  ಆದರೆ( objc!=1 && objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "?RESET?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( objc==2 ){
    ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  } 
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, 
     Tcl_NewWideIntObj(sqlite3_ಸ್ಮರಣೆ_highwater(resetFlag)));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memdebug_backtrace(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ ಆಳ;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DEPT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &ಆಳ) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MEMDEBUG
  {
    ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3MemdebugBacktrace(ಪೂರ್ಣಾಂಕ);
    sqlite3MemdebugBacktrace(ಆಳ);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memdebug_dump(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "FILENAME");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
#ಆದರೆ defined(SQLITE_MEMDEBUG) || defined(SQLITE_ಸ್ಮರಣೆ_ಗಾತ್ರ) \
     || defined(SQLITE_POW2_ಸ್ಮರಣೆ_ಗಾತ್ರ)
  {
    ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3MemdebugDump(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memdebug_malloc_ಎಣಿಕೆ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ nMalloc = -1;
  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
#ಆದರೆ defined(SQLITE_MEMDEBUG)
  {
    ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_Memdebug_Malloc_ಎಣಿಕೆ();
    nMalloc = sqlite3_Memdebug_Malloc_ಎಣಿಕೆ();
  }
#ಆದರೆ_ಅಂತ್ಯ
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(nMalloc));
  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memdebug_fail(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ ii;
  ಪೂರ್ಣಾಂಕ iFail;
  ಪೂರ್ಣಾಂಕ nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  ಪೂರ್ಣಾಂಕ nBenign;
  ಪೂರ್ಣಾಂಕ nFail = 0;

  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "COUNTER ?OPTIONS?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  ಗಾಗಿ(ii=2; ii<objc; ii+=2){
    Tcl_ಗಾತ್ರ nOption;
    ಅಕ್ಷರ *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    ಅಕ್ಷರ *zErr = 0;

    ಆದರೆ( nOption>1 && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zOption, "-repeat", nOption)==0 ){
      ಆದರೆ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nOption>1 && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zOption, "-benigncnt", nOption)==0 ){
      ಆದರೆ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ಇಲ್ಲದಿದ್ದರೆ{
        pBenignCnt = objv[ii+1];
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      zErr = "unknown option: ";
    }

    ಆದರೆ( zErr ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, zErr, zOption, ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  ಆದರೆ( pBenignCnt ){
    Tcl_Obj_ಗಣ_Var2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(nFail));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memdebug_pending(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ nPending;
  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(nPending));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_memdebug_title_ಎಣಿಕೆ = 0;

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memdebug_settitle(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_ಎಣಿಕೆ++;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "TITLE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MEMDEBUG
  {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTitle;
    ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3MemdebugSettitle(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ವ್ಯಾಖ್ಯಾನಿಸು MALLOC_LOG_ಚೌಕಟ್ಟುಗಳು  10 
#ವ್ಯಾಖ್ಯಾನಿಸು MALLOC_LOG_KEYINTS (                                              \
    10 * ((ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)>=ಗಾತ್ರ(ಶೂನ್ಯ*)) ? 1 : ಗಾತ್ರ(ಶೂನ್ಯ*)/ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))   \
)
ಸ್ಥಿರ Tcl_Hash_ಕೋಷ್ಟಕ aMallocLog;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mallocLogEnabled = 0;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ MallocLog MallocLog;
ರಚನೆ MallocLog {
  ಪೂರ್ಣಾಂಕ nCall;
  ಪೂರ್ಣಾಂಕ nByte;
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MEMDEBUG
ಸ್ಥಿರ ಶೂನ್ಯ test_memdebug_callback(ಪೂರ್ಣಾಂಕ nByte, ಪೂರ್ಣಾಂಕ n_ಚೌಕಟ್ಟು, ಶೂನ್ಯ **a_ಚೌಕಟ್ಟು){
  ಆದರೆ( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_Hash_ನಮೂದು *p_ನಮೂದು;
    ಪೂರ್ಣಾಂಕ isNew;

    ಪೂರ್ಣಾಂಕ a_ಕೀಲಿ[MALLOC_LOG_KEYINTS];
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ = ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*MALLOC_LOG_KEYINTS;

    memset(a_ಕೀಲಿ, 0, n_ಕೀಲಿ);
    ಆದರೆ( (ಗಾತ್ರ(ಶೂನ್ಯ*)*n_ಚೌಕಟ್ಟು)<n_ಕೀಲಿ ){
      n_ಕೀಲಿ = n_ಚೌಕಟ್ಟು*ಗಾತ್ರ(ಶೂನ್ಯ*);
    }
    ಸ್ಮರಣೆ_ನಕಲು(a_ಕೀಲಿ, a_ಚೌಕಟ್ಟು, n_ಕೀಲಿ);

    p_ನಮೂದು = Tcl_Create_Hash_ನಮೂದು(&aMallocLog, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)a_ಕೀಲಿ, &isNew);
    ಆದರೆ( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(ಗಾತ್ರ(MallocLog));
      memset(pLog, 0, ಗಾತ್ರ(MallocLog));
      Tcl_ಗಣ_Hash_ಮೌಲ್ಯ(p_ನಮೂದು, (Client_ದತ್ತಾಂಶ)pLog);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pLog = (MallocLog *)Tcl_Get_Hash_ಮೌಲ್ಯ(p_ನಮೂದು);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MEMDEBUG */

ಸ್ಥಿರ ಶೂನ್ಯ test_memdebug_log_clear(ಶೂನ್ಯ){
  Tcl_Hash_ಹುಡುಕು ಹುಡುಕು;
  Tcl_Hash_ನಮೂದು *p_ನಮೂದು;
  ಗಾಗಿ(
    p_ನಮೂದು=Tcl_ಮೊದಲ_Hash_ನಮೂದು(&aMallocLog, &ಹುಡುಕು);
    p_ನಮೂದು;
    p_ನಮೂದು=Tcl_ಮುಂದಿನ_Hash_ನಮೂದು(&ಹುಡುಕು)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ಮೌಲ್ಯ(p_ನಮೂದು);
    Tcl_Free((ಅಕ್ಷರ *)pLog);
  }
  Tcl_Delete_Hash_ಕೋಷ್ಟಕ(&aMallocLog);
  Tcl_Init_Hash_ಕೋಷ್ಟಕ(&aMallocLog, MALLOC_LOG_KEYINTS);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_memdebug_log(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isInit = 0;
  ಪೂರ್ಣಾಂಕ iSub;

  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  ಎಣಿಕೆ MB_enum { 
      MB_LOG_ಆರಂಭ, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  ಆದರೆ( !isInit ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MEMDEBUG
    ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3MemdebugBacktraceCallback(
        ಶೂನ್ಯ (*xBacktrace)(ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#ಆದರೆ_ಅಂತ್ಯ
    Tcl_Init_Hash_ಕೋಷ್ಟಕ(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SUB-COMMAND ...");
  }
  ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆಯ್ಕೆ( (ಎಣಿಕೆ MB_enum)iSub ){
    ಸ್ಥಿತಿ MB_LOG_ಆರಂಭ:
      mallocLogEnabled = 1;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ MB_LOG_STOP:
      mallocLogEnabled = 0;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ MB_LOG_DUMP: {
      Tcl_Hash_ಹುಡುಕು ಹುಡುಕು;
      Tcl_Hash_ನಮೂದು *p_ನಮೂದು;
      Tcl_Obj *pRet = Tcl_NewObj();

      ಪ್ರತಿಜ್ಞಾ(ಗಾತ್ರ(Tcl_WideInt)>=ಗಾತ್ರ(ಶೂನ್ಯ*));

      ಗಾಗಿ(
        p_ನಮೂದು=Tcl_ಮೊದಲ_Hash_ನಮೂದು(&aMallocLog, &ಹುಡುಕು);
        p_ನಮೂದು;
        p_ನಮೂದು=Tcl_ಮುಂದಿನ_Hash_ನಮೂದು(&ಹುಡುಕು)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_ಚೌಕಟ್ಟುಗಳು+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ಮೌಲ್ಯ(p_ನಮೂದು);
        Tcl_WideInt *a_ಕೀಲಿ = (Tcl_WideInt *)Tcl_Get_Hash_ಕೀಲಿ(&aMallocLog, p_ನಮೂದು);
        ಪೂರ್ಣಾಂಕ ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        ಗಾಗಿ(ii=0; ii<MALLOC_LOG_ಚೌಕಟ್ಟುಗಳು; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_ಕೀಲಿ[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_ಚೌಕಟ್ಟುಗಳು+2, apElem)
        );
      }

      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ MB_LOG_SYNC: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MEMDEBUG
      ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    }
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_pagecache(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ sz, N;
  Tcl_Obj *pRes;
  ಸ್ಥಿರ ಅಕ್ಷರ *buf = 0;
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SIZE N");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &N) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಬಿಡುಗಡೆ(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.sz_ಪುಟ));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.n_ಪುಟ));
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRes);

  ಆದರೆ( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (ಶೂನ್ಯ*)0, 0, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    buf = ಸ್ಮರಣೆ_ಕೊಡು( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_alt_pcache(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ installFlag;
  ಪೂರ್ಣಾಂಕ discardChance = 0;
  ಪೂರ್ಣಾಂಕ prng_ಬೀಜ_2 = 0;
  ಪೂರ್ಣಾಂಕ ಮೇಲಿನ_Stress = 0;
  ಬಾಹ್ಯ ಶೂನ್ಯ installTestPCache(ಪೂರ್ಣಾಂಕ,ಚಿಹ್ನೆರಹಿತ,ಚಿಹ್ನೆರಹಿತ,ಚಿಹ್ನೆರಹಿತ);
  ಆದರೆ( objc<2 || objc>5 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prng_ಬೀಜ_2) ){
     ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &ಮೇಲಿನ_Stress) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( discardChance<0 || discardChance>100 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "discard-chance should be between 0 and 100",
                     (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  installTestPCache(installFlag, (ಚಿಹ್ನೆರಹಿತ)discardChance, (ಚಿಹ್ನೆರಹಿತ)prng_ಬೀಜ_2,
                    (ಚಿಹ್ನೆರಹಿತ)ಮೇಲಿನ_Stress);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_memstatus(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ enable, rc;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "BOOLEAN");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_lookaside(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ sz, cnt;
  Tcl_Obj *pRet;
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SIZE COUNT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);
  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_db_config_lookaside(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ sz, cnt;
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ bufid;
  ಸ್ಥಿರ ಅಕ್ಷರ azBuf[2][10000];
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, sqlite3**);
  ಆದರೆ( objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "BUFID SIZE COUNT");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[3], &sz) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (ಶೂನ್ಯ*)0, sz, cnt);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( bufid>=1 && bufid<=2 && sz*cnt<=ಗಾತ್ರ(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }ಇಲ್ಲದಿದ್ದರೆ{
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "illegal arguments - see documentation", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(rc));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_ರಾಶಿ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರ ಅಕ್ಷರ *zBuf; /* Use this memory */
  ಪೂರ್ಣಾಂಕ nByte;         /* Size of buffer to pass to sqlite3_config() */
  ಪೂರ್ಣಾಂಕ nMinAlloc;     /* Size of minimum allocation */
  ಪೂರ್ಣಾಂಕ rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ಪೂರ್ಣಾಂಕ nArg = objc-1;

  ಆದರೆ( nArg!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "NBYTE NMINALLOC");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  ಆದರೆ( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  ಆದರೆ( nByte==0 ){
    ಬಿಡುಗಡೆ( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_ರಾಶಿ, (ಶೂನ್ಯ*)0, 0, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_ರಾಶಿ, zBuf, nByte, nMinAlloc);
  }

  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_ರಾಶಿ_ಗಾತ್ರ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ nByte;         /* Size to pass to sqlite3_config() */
  ಪೂರ್ಣಾಂಕ rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  ಪೂರ್ಣಾಂಕ nArg = objc-1;

  ಆದರೆ( nArg!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "NBYTE");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ಹಿಂತಿರುಗಿ TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_error(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, sqlite3**);

  ಆದರೆ( objc!=2 && objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "[DB]");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( objc==2 ){
    ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
    ಆದರೆ( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_uri(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ bOpenUri;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "BOOL");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_cis(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ bUseCis;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "BOOL");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_ಸೂಚ್ಯಂಕ_ಪರಿಶೀಲನೆ, bUseCis);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_config_pmasz(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ, 
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ iPmaSz;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "BOOL");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ಹಿಂತಿರುಗಿ TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_dump_memsys3(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "FILENAME");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆಯ್ಕೆ( SQLITE_PTR_ಗಮ್ಯ_INT(client_ದತ್ತಾಂಶ) ){
    ಸ್ಥಿತಿ 3: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS3
      ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3Memsys3Dump(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಸ್ಥಿತಿ 5: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS5
      ಬಾಹ್ಯ ಶೂನ್ಯ sqlite3Memsys5Dump(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
    }
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_ಸ್ಥಿತಿ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc, i_ಮೌಲ್ಯ, mx_ಮೌಲ್ಯ;
  ಪೂರ್ಣಾಂಕ i, op = 0, resetFlag;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOpName;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    ಪೂರ್ಣಾಂಕ op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_ಸ್ಥಿತಿ_MALLOC_ಗಾತ್ರ         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಬಳಸಿದ      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಅತಿಪ್ರವಾಹ  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಗಾತ್ರ      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_ಸ್ಥಿತಿ_SCRATCH_ಬಳಸಿದ        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_ಸ್ಥಿತಿ_SCRATCH_ಅತಿಪ್ರವಾಹ    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_ಸ್ಥಿತಿ_SCRATCH_ಗಾತ್ರ        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_ಸ್ಥಿತಿ_PARSER_ರಾಶಿ        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_ಸ್ಥಿತಿ_MALLOC_ಎಣಿಕೆ        },
  };
  Tcl_Obj *p_ಫಲಿತಾಂಶ;
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "PARAMETER RESETFLAG");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(aOp); i++){
    ಆದರೆ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( i>=Array_ಗಾತ್ರ(aOp) ){
    ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &op) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  i_ಮೌಲ್ಯ = 0;
  mx_ಮೌಲ್ಯ = 0;
  rc = sqlite3_ಸ್ಥಿತಿ(op, &i_ಮೌಲ್ಯ, &mx_ಮೌಲ್ಯ, resetFlag);
  p_ಫಲಿತಾಂಶ = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_ಫಲಿತಾಂಶ, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_ಫಲಿತಾಂಶ, Tcl_NewIntObj(i_ಮೌಲ್ಯ));
  Tcl_ListObjAppendElement(0, p_ಫಲಿತಾಂಶ, Tcl_NewIntObj(mx_ಮೌಲ್ಯ));
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, p_ಫಲಿತಾಂಶ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_db_ಸ್ಥಿತಿ(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc, i_ಮೌಲ್ಯ, mx_ಮೌಲ್ಯ;
  ಪೂರ್ಣಾಂಕ i, op = 0, resetFlag;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOpName;
  sqlite3 *db;
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, sqlite3**);
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    ಪೂರ್ಣಾಂಕ op;
  } aOp[] = {
    { "LOOKASIDE_USED",      SQLITE_DBSTATUS_LOOKASIDE_ಬಳಸಿದ      },
    { "CACHE_USED",          SQLITE_DBSTATUS_CACHE_ಬಳಸಿದ          },
    { "SCHEMA_USED",         SQLITE_DBSTATUS_SCHEMA_ಬಳಸಿದ         },
    { "STMT_USED",           SQLITE_DBSTATUS_STMT_ಬಳಸಿದ           },
    { "LOOKASIDE_HIT",       SQLITE_DBSTATUS_LOOKASIDE_ದೊರಕಿತು       },
    { "LOOKASIDE_MISS_SIZE", SQLITE_DBSTATUS_LOOKASIDE_ತಪ್ಪಿದ_ಗಾತ್ರ },
    { "LOOKASIDE_MISS_FULL", SQLITE_DBSTATUS_LOOKASIDE_ತಪ್ಪಿದ_ತುಂಬಿದ },
    { "CACHE_HIT",           SQLITE_DBSTATUS_CACHE_ದೊರಕಿತು           },
    { "CACHE_MISS",          SQLITE_DBSTATUS_CACHE_ತಪ್ಪಿದ          },
    { "CACHE_WRITE",         SQLITE_DBSTATUS_CACHE_WRITE         },
    { "DEFERRED_FKS",        SQLITE_DBSTATUS_DEFERRED_FKS        },
    { "CACHE_USED_SHARED",   SQLITE_DBSTATUS_CACHE_ಬಳಸಿದ_SHARED   },
    { "CACHE_SPILL",         SQLITE_DBSTATUS_CACHE_SPILL         },
    { "TEMPBUF_SPILL",       SQLITE_DBSTATUS_TEMPBUF_SPILL       },
  };
  Tcl_Obj *p_ಫಲಿತಾಂಶ;
  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB PARAMETER RESETFLAG");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  ಆದರೆ( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  ಆದರೆ( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(aOp); i++){
    ಆದರೆ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( i>=Array_ಗಾತ್ರ(aOp) ){
    ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &op) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
  i_ಮೌಲ್ಯ = 0;
  mx_ಮೌಲ್ಯ = 0;
  rc = sqlite3_db_ಸ್ಥಿತಿ(db, op, &i_ಮೌಲ್ಯ, &mx_ಮೌಲ್ಯ, resetFlag);
  p_ಫಲಿತಾಂಶ = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_ಫಲಿತಾಂಶ, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_ಫಲಿತಾಂಶ, Tcl_NewIntObj(i_ಮೌಲ್ಯ));
  Tcl_ListObjAppendElement(0, p_ಫಲಿತಾಂಶ, Tcl_NewIntObj(mx_ಮೌಲ್ಯ));
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, p_ಫಲಿತಾಂಶ);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_install_malloc_faultsim(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ isInstall;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "BOOLEAN");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_install_memsys3(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_MISUSE;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS3
  ಸ್ಥಿರಾಂಕ sqlite3_mem_methods *sqlite3MemGetMemsys3(ಶೂನ್ಯ);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#ಆದರೆ_ಅಂತ್ಯ
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_vfs_oom_test(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_memdebug_vfs_oom_test;
  ಆದರೆ( objc>2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "?INTEGER?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==2 ){
    ಪೂರ್ಣಾಂಕ iNew;
    ಆದರೆ( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest_malloc_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಪೂರ್ಣಾಂಕ client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "sqlite3_malloc",             test_malloc                   ,0 },
     { "sqlite3_realloc",            test_realloc                  ,0 },
     { "sqlite3_free",               test_free                     ,0 },
     { "memset",                     test_memset                   ,0 },
     { "memget",                     test_memget                   ,0 },
     { "sqlite3_memory_used",        test_ಸ್ಮರಣೆ_ಬಳಸಿದ              ,0 },
     { "sqlite3_memory_highwater",   test_ಸ್ಮರಣೆ_highwater         ,0 },
     { "sqlite3_memdebug_backtrace", test_memdebug_backtrace       ,0 },
     { "sqlite3_memdebug_dump",      test_memdebug_dump            ,0 },
     { "sqlite3_memdebug_fail",      test_memdebug_fail            ,0 },
     { "sqlite3_memdebug_pending",   test_memdebug_pending         ,0 },
     { "sqlite3_memdebug_settitle",  test_memdebug_settitle        ,0 },
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_ಎಣಿಕೆ ,0 },
     { "sqlite3_memdebug_log",       test_memdebug_log             ,0 },
     { "sqlite3_config_pagecache",   test_config_pagecache         ,0 },
     { "sqlite3_config_alt_pcache",  test_alt_pcache               ,0 },
     { "sqlite3_status",             test_ಸ್ಥಿತಿ                   ,0 },
     { "sqlite3_db_status",          test_db_ಸ್ಥಿತಿ                ,0 },
     { "install_malloc_faultsim",    test_install_malloc_faultsim  ,0 },
     { "sqlite3_config_heap",        test_config_ರಾಶಿ              ,0 },
     { "sqlite3_config_heap_size",   test_config_ರಾಶಿ_ಗಾತ್ರ         ,0 },
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
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Client_ದತ್ತಾಂಶ c = (Client_ದತ್ತಾಂಶ)SQLITE_INT_ಗಮ್ಯ_PTR(aObjCmd[i].client_ದತ್ತಾಂಶ);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
