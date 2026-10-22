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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
స్థిర నిర్మాణం MemFault {
  పూర్ణాంకం iCountdown;         /* Number of pending successes before a failure */
  పూర్ణాంకం nRepeat;            /* Number of times to repeat the failure */
  పూర్ణాంకం nBenign;            /* Number of benign failures seen since last config */
  పూర్ణాంకం nFail;              /* Number of failures seen since last config */
  పూర్ణాంకం nOkBefore;          /* Successful allocations prior to the first fault */
  పూర్ణాంకం nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  పూర్ణాంకం isInstalled;        /* True if the fault simulation layer is installed */
  పూర్ణాంకం is_Benign_బహుళకం;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
స్థిర శూన్య sqlite3Fault(శూన్య){
  స్థిర u64 cnt = 0;
  cnt++;
  ఒకవేళ( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
స్థిర శూన్య sqlite3_మొదటి_Fault(శూన్య){
  స్థిర u64 cnt2 = 0;
  cnt2++;
  ఒకవేళ( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
స్థిర పూర్ణాంకం faultsim_అడుగు(శూన్య){
  ఒకవేళ( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    తిరిగి 0;
  }
  ఒకవేళ( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    తిరిగి 0;
  }
  ఒకవేళ( memfault.nFail==0 ) sqlite3_మొదటి_Fault();
  sqlite3Fault();
  memfault.nFail++;
  ఒకవేళ( memfault.is_Benign_బహుళకం>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  ఒకవేళ( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  తిరిగి 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
స్థిర శూన్య *faultsimMalloc(పూర్ణాంకం n){
  శూన్య *p = 0;
  ఒకవేళ( !faultsim_అడుగు() ){
    p = memfault.m.xMalloc(n);
  }
  తిరిగి p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
స్థిర శూన్య *faultsimRealloc(శూన్య *pOld, పూర్ణాంకం n){
  శూన్య *p = 0;
  ఒకవేళ( !faultsim_అడుగు() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  తిరిగి p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
స్థిర శూన్య faultsimConfig(పూర్ణాంకం nDelay, పూర్ణాంకం nRepeat){
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
  memfault.is_Benign_బహుళకం = 0;
}

/*
** Return the number of faults (both hard and benign faults) that have
** occurred since the injector was last configured.
*/
స్థిర పూర్ణాంకం faultsimFailures(శూన్య){
  తిరిగి memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
స్థిర పూర్ణాంకం faultsimBenignFailures(శూన్య){
  తిరిగి memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
స్థిర పూర్ణాంకం faultsimPending(శూన్య){
  ఒకవేళ( memfault.enable ){
    తిరిగి memfault.iCountdown;
  }లేకపోతే{
    తిరిగి -1;
  }
}


స్థిర శూన్య faultsimBeginBenign(శూన్య){
  memfault.is_Benign_బహుళకం++;
}
స్థిర శూన్య faultsim_చివర_Benign(శూన్య){
  memfault.is_Benign_బహుళకం--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
స్థిర పూర్ణాంకం faultsimInstall(పూర్ణాంకం install){
  పూర్ణాంకం rc;

  install = (install ? 1 : 0);
  ప్రతిజ్ఞా(memfault.isInstalled==1 || memfault.isInstalled==0);

  ఒకవేళ( install==memfault.isInstalled ){
    తిరిగి SQLITE_ERROR;
  }

  ఒకవేళ( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    ప్రతిజ్ఞా(memfault.m.xMalloc);
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsim_చివర_Benign
    );
  }లేకపోతే{
    sqlite3_mem_methods m2;
    ప్రతిజ్ఞా(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, పరిమాణం(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    ప్రతిజ్ఞా( memcmp(&m2, &memfault.m, పరిమాణం(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (శూన్య*)0, (శూన్య*)0);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_TEST

/*
** This function is implemented in main.c. Returns a pointer to a static
** buffer containing the symbolic SQLite error code that corresponds to
** the least-significant 8-bits of the integer passed as an argument.
** For example:
**
**   sqlite3ErrName(1) -> "SQLITE_ERROR"
*/
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/*
** Transform pointers to text and back again
*/
స్థిర శూన్య సూచిక_గమ్యం_పాఠ్యం(శూన్య *p, అక్షరం *z){
  స్థిరరాశి అక్షరం *zRet = sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(p);
  sprintf(z, "%s", zRet);
}
స్థిర పూర్ణాంకం పాఠ్యం_గమ్యం_సూచిక(స్థిరరాశి అక్షరం *z, శూన్య **pp){
  *pp = sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(z);
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_malloc(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం nByte;
  శూన్య *p;
  అక్షరం z_బహిర్గతం[100];
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "NBYTES");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) తిరిగి TCL_ERROR;
  p = sqlite3_malloc((గుర్తులేని)nByte);
  సూచిక_గమ్యం_పాఠ్యం(p, z_బహిర్గతం);
  Tcl_Append_ఫలితం(interp, z_బహిర్గతం, ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_realloc(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం nByte;
  శూన్య *p_పూర్వ, *p;
  అక్షరం z_బహిర్గతం[100];
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "PRIOR NBYTES");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) తిరిగి TCL_ERROR;
  ఒకవేళ( పాఠ్యం_గమ్యం_సూచిక(Tcl_GetString(objv[1]), &p_పూర్వ) ){
    Tcl_Append_ఫలితం(interp, "bad pointer: ", Tcl_GetString(objv[1]), (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  p = sqlite3_realloc(p_పూర్వ, (గుర్తులేని)nByte);
  సూచిక_గమ్యం_పాఠ్యం(p, z_బహిర్గతం);
  Tcl_Append_ఫలితం(interp, z_బహిర్గతం, ఖాళీ);
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_free(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  శూన్య *p_పూర్వ;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "PRIOR");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( పాఠ్యం_గమ్యం_సూచిక(Tcl_GetString(objv[1]), &p_పూర్వ) ){
    Tcl_Append_ఫలితం(interp, "bad pointer: ", Tcl_GetString(objv[1]), (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  sqlite3_free(p_పూర్వ);
  తిరిగి TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
పూర్ణాంకం sqlite3_Test_Hex_గమ్యం_Bin(స్థిరరాశి అక్షరం *, పూర్ణాంకం, అక్షరం *);
పూర్ణాంకం sqlite3_Test_Bin_గమ్యం_Hex(అక్షరం*,పూర్ణాంకం);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memset(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  శూన్య *p;
  పూర్ణాంకం కొలత, i;
  Tcl_కొలత n;
  అక్షరం *zHex;
  అక్షరం *z_బహిర్గతం;
  అక్షరం zBin[100];

  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "ADDRESS SIZE HEX");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( పాఠ్యం_గమ్యం_సూచిక(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_ఫలితం(interp, "bad pointer: ", Tcl_GetString(objv[1]), (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &కొలత) ){
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( కొలత<=0 ){
    Tcl_Append_ఫలితం(interp, "size must be positive", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  ఒకవేళ( n>పరిమాణం(zBin)*2 ) n = పరిమాణం(zBin)*2;
  n = sqlite3_Test_Hex_గమ్యం_Bin(zHex, (పూర్ణాంకం)n, zBin);
  ఒకవేళ( n==0 ){
    Tcl_Append_ఫలితం(interp, "no data", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  z_బహిర్గతం = p;
  కొరకు(i=0; i<కొలత; i++){
    z_బహిర్గతం[i] = zBin[i%n];
  }
  తిరిగి TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memget(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  శూన్య *p;
  పూర్ణాంకం కొలత, n;
  అక్షరం *zBin;
  అక్షరం zHex[100];

  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "ADDRESS SIZE");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( పాఠ్యం_గమ్యం_సూచిక(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_ఫలితం(interp, "bad pointer: ", Tcl_GetString(objv[1]), (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &కొలత) ){
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( కొలత<=0 ){
    Tcl_Append_ఫలితం(interp, "size must be positive", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  zBin = p;
  అయితే( కొలత>0 ){
    ఒకవేళ( కొలత>(పరిమాణం(zHex)-1)/2 ){
      n = (పరిమాణం(zHex)-1)/2;
    }లేకపోతే{
      n = కొలత;
    }
    జ్ఞాపకం_నకలు(zHex, zBin, n);
    zBin += n;
    కొలత -= n;
    sqlite3_Test_Bin_గమ్యం_Hex(zHex, n);
    Tcl_Append_ఫలితం(interp, zHex, (అక్షరం*)0);
  }
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_జ్ఞాపకం_వాడిన(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewWideIntObj(sqlite3_జ్ఞాపకం_వాడిన()));
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_జ్ఞాపకం_highwater(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం resetFlag = 0;
  ఒకవేళ( objc!=1 && objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "?RESET?");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( objc==2 ){
    ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) తిరిగి TCL_ERROR;
  } 
  Tcl_సమితి_Obj_ఫలితం(interp, 
     Tcl_NewWideIntObj(sqlite3_జ్ఞాపకం_highwater(resetFlag)));
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memdebug_backtrace(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం లోతు;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DEPT");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &లోతు) ) తిరిగి TCL_ERROR;
#నిర్వచితమైతే SQLITE_MEMDEBUG
  {
    బాహ్య శూన్య sqlite3MemdebugBacktrace(పూర్ణాంకం);
    sqlite3MemdebugBacktrace(లోతు);
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memdebug_dump(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "FILENAME");
    తిరిగి TCL_ERROR;
  }
#ఒకవేళ defined(SQLITE_MEMDEBUG) || defined(SQLITE_జ్ఞాపకం_కొలత) \
     || defined(SQLITE_POW2_జ్ఞాపకం_కొలత)
  {
    బాహ్య శూన్య sqlite3MemdebugDump(స్థిరరాశి అక్షరం*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memdebug_malloc_లెక్క(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం nMalloc = -1;
  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }
#ఒకవేళ defined(SQLITE_MEMDEBUG)
  {
    బాహ్య పూర్ణాంకం sqlite3_Memdebug_Malloc_లెక్క();
    nMalloc = sqlite3_Memdebug_Malloc_లెక్క();
  }
#అయితే_ముగింపు
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(nMalloc));
  తిరిగి TCL_OK;
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memdebug_fail(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం ii;
  పూర్ణాంకం iFail;
  పూర్ణాంకం nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  పూర్ణాంకం nBenign;
  పూర్ణాంకం nFail = 0;

  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "COUNTER ?OPTIONS?");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) తిరిగి TCL_ERROR;

  కొరకు(ii=2; ii<objc; ii+=2){
    Tcl_కొలత nOption;
    అక్షరం *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    అక్షరం *zErr = 0;

    ఒకవేళ( nOption>1 && అక్షరశ్రేణి_పోలిక(zOption, "-repeat", nOption)==0 ){
      ఒకవేళ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }లేకపోతే{
        ఒకవేళ( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          తిరిగి TCL_ERROR;
        }
      }
    }లేకపోతే ఒకవేళ( nOption>1 && అక్షరశ్రేణి_పోలిక(zOption, "-benigncnt", nOption)==0 ){
      ఒకవేళ( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }లేకపోతే{
        pBenignCnt = objv[ii+1];
      }
    }లేకపోతే{
      zErr = "unknown option: ";
    }

    ఒకవేళ( zErr ){
      Tcl_Append_ఫలితం(interp, zErr, zOption, ఖాళీ);
      తిరిగి TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  ఒకవేళ( pBenignCnt ){
    Tcl_Obj_సమితి_Var2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(nFail));
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memdebug_pending(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం nPending;
  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(nPending));
  తిరిగి TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
స్థిర పూర్ణాంకం sqlite3_memdebug_title_లెక్క = 0;

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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memdebug_settitle(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_లెక్క++;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "TITLE");
    తిరిగి TCL_ERROR;
  }
#నిర్వచితమైతే SQLITE_MEMDEBUG
  {
    స్థిరరాశి అక్షరం *zTitle;
    బాహ్య పూర్ణాంకం sqlite3MemdebugSettitle(స్థిరరాశి అక్షరం*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

#నిర్వచించు MALLOC_LOG_చట్రాలు  10 
#నిర్వచించు MALLOC_LOG_KEYINTS (                                              \
    10 * ((పరిమాణం(పూర్ణాంకం)>=పరిమాణం(శూన్య*)) ? 1 : పరిమాణం(శూన్య*)/పరిమాణం(పూర్ణాంకం))   \
)
స్థిర Tcl_Hash_పట్టిక aMallocLog;
స్థిర పూర్ణాంకం mallocLogEnabled = 0;

రకనిర్వచనం నిర్మాణం MallocLog MallocLog;
నిర్మాణం MallocLog {
  పూర్ణాంకం nCall;
  పూర్ణాంకం nByte;
};

#నిర్వచితమైతే SQLITE_MEMDEBUG
స్థిర శూన్య test_memdebug_callback(పూర్ణాంకం nByte, పూర్ణాంకం n_చట్రం, శూన్య **a_చట్రం){
  ఒకవేళ( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_Hash_నమోదు *p_నమోదు;
    పూర్ణాంకం isNew;

    పూర్ణాంకం a_కీలకం[MALLOC_LOG_KEYINTS];
    గుర్తులేని పూర్ణాంకం n_కీలకం = పరిమాణం(పూర్ణాంకం)*MALLOC_LOG_KEYINTS;

    memset(a_కీలకం, 0, n_కీలకం);
    ఒకవేళ( (పరిమాణం(శూన్య*)*n_చట్రం)<n_కీలకం ){
      n_కీలకం = n_చట్రం*పరిమాణం(శూన్య*);
    }
    జ్ఞాపకం_నకలు(a_కీలకం, a_చట్రం, n_కీలకం);

    p_నమోదు = Tcl_Create_Hash_నమోదు(&aMallocLog, (స్థిరరాశి అక్షరం *)a_కీలకం, &isNew);
    ఒకవేళ( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(పరిమాణం(MallocLog));
      memset(pLog, 0, పరిమాణం(MallocLog));
      Tcl_సమితి_Hash_విలువ(p_నమోదు, (Client_దత్తాంశం)pLog);
    }లేకపోతే{
      pLog = (MallocLog *)Tcl_Get_Hash_విలువ(p_నమోదు);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#అయితే_ముగింపు /* SQLITE_MEMDEBUG */

స్థిర శూన్య test_memdebug_log_clear(శూన్య){
  Tcl_Hash_శోధన శోధన;
  Tcl_Hash_నమోదు *p_నమోదు;
  కొరకు(
    p_నమోదు=Tcl_మొదటి_Hash_నమోదు(&aMallocLog, &శోధన);
    p_నమోదు;
    p_నమోదు=Tcl_తదుపరి_Hash_నమోదు(&శోధన)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_విలువ(p_నమోదు);
    Tcl_Free((అక్షరం *)pLog);
  }
  Tcl_Delete_Hash_పట్టిక(&aMallocLog);
  Tcl_Init_Hash_పట్టిక(&aMallocLog, MALLOC_LOG_KEYINTS);
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_memdebug_log(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిర పూర్ణాంకం isInit = 0;
  పూర్ణాంకం iSub;

  స్థిర స్థిరరాశి అక్షరం *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  గణన MB_enum { 
      MB_LOG_ప్రారంభం, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  ఒకవేళ( !isInit ){
#నిర్వచితమైతే SQLITE_MEMDEBUG
    బాహ్య శూన్య sqlite3MemdebugBacktraceCallback(
        శూన్య (*xBacktrace)(పూర్ణాంకం, పూర్ణాంకం, శూన్య **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#అయితే_ముగింపు
    Tcl_Init_Hash_పట్టిక(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SUB-COMMAND ...");
  }
  ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    తిరిగి TCL_ERROR;
  }

  ఎంపిక( (గణన MB_enum)iSub ){
    స్థితి MB_LOG_ప్రారంభం:
      mallocLogEnabled = 1;
      ఆపు;
    స్థితి MB_LOG_STOP:
      mallocLogEnabled = 0;
      ఆపు;
    స్థితి MB_LOG_DUMP: {
      Tcl_Hash_శోధన శోధన;
      Tcl_Hash_నమోదు *p_నమోదు;
      Tcl_Obj *pRet = Tcl_NewObj();

      ప్రతిజ్ఞా(పరిమాణం(Tcl_WideInt)>=పరిమాణం(శూన్య*));

      కొరకు(
        p_నమోదు=Tcl_మొదటి_Hash_నమోదు(&aMallocLog, &శోధన);
        p_నమోదు;
        p_నమోదు=Tcl_తదుపరి_Hash_నమోదు(&శోధన)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_చట్రాలు+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_విలువ(p_నమోదు);
        Tcl_WideInt *a_కీలకం = (Tcl_WideInt *)Tcl_Get_Hash_కీలకం(&aMallocLog, p_నమోదు);
        పూర్ణాంకం ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        కొరకు(ii=0; ii<MALLOC_LOG_చట్రాలు; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_కీలకం[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_చట్రాలు+2, apElem)
        );
      }

      Tcl_సమితి_Obj_ఫలితం(interp, pRet);
      ఆపు;
    }
    స్థితి MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      ఆపు;
    }

    స్థితి MB_LOG_SYNC: {
#నిర్వచితమైతే SQLITE_MEMDEBUG
      బాహ్య శూన్య sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#అయితే_ముగింపు
      ఆపు;
    }
  }

  తిరిగి TCL_OK;
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_pagecache(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం sz, N;
  Tcl_Obj *pRes;
  స్థిర అక్షరం *buf = 0;
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SIZE N");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &N) ) తిరిగి TCL_ERROR;
  విడుదల(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.sz_పుట));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.n_పుట));
  Tcl_సమితి_Obj_ఫలితం(interp, pRes);

  ఒకవేళ( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (శూన్య*)0, 0, 0);
  }లేకపోతే{
    buf = జ్ఞాపకం_ఇవ్వు( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  తిరిగి TCL_OK;
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_alt_pcache(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం installFlag;
  పూర్ణాంకం discardChance = 0;
  పూర్ణాంకం prng_విత్తనం_2 = 0;
  పూర్ణాంకం ఎగువ_Stress = 0;
  బాహ్య శూన్య installTestPCache(పూర్ణాంకం,గుర్తులేని,గుర్తులేని,గుర్తులేని);
  ఒకవేళ( objc<2 || objc>5 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) తిరిగి TCL_ERROR;
  ఒకవేళ( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     తిరిగి TCL_ERROR;
  }
  ఒకవేళ( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prng_విత్తనం_2) ){
     తిరిగి TCL_ERROR;
  }
  ఒకవేళ( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &ఎగువ_Stress) ){
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( discardChance<0 || discardChance>100 ){
    Tcl_Append_ఫలితం(interp, "discard-chance should be between 0 and 100",
                     (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  installTestPCache(installFlag, (గుర్తులేని)discardChance, (గుర్తులేని)prng_విత్తనం_2,
                    (గుర్తులేని)ఎగువ_Stress);
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_memstatus(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం enable, rc;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "BOOLEAN");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) తిరిగి TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_lookaside(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం sz, cnt;
  Tcl_Obj *pRet;
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SIZE COUNT");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &sz) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) తిరిగి TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_సమితి_Obj_ఫలితం(interp, pRet);
  తిరిగి TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_db_config_lookaside(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;
  పూర్ణాంకం sz, cnt;
  sqlite3 *db;
  పూర్ణాంకం bufid;
  స్థిర అక్షరం azBuf[2][10000];
  బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp*, స్థిరరాశి అక్షరం*, sqlite3**);
  ఒకవేళ( objc!=5 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "BUFID SIZE COUNT");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[3], &sz) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) తిరిగి TCL_ERROR;
  ఒకవేళ( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (శూన్య*)0, sz, cnt);
  }లేకపోతే ఒకవేళ( bufid>=1 && bufid<=2 && sz*cnt<=పరిమాణం(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }లేకపోతే{
    Tcl_Append_ఫలితం(interp, "illegal arguments - see documentation", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(rc));
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_కుప్ప(
  శూన్య * client_దత్తాంశం, 
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిర అక్షరం *zBuf; /* Use this memory */
  పూర్ణాంకం nByte;         /* Size of buffer to pass to sqlite3_config() */
  పూర్ణాంకం nMinAlloc;     /* Size of minimum allocation */
  పూర్ణాంకం rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  పూర్ణాంకం nArg = objc-1;

  ఒకవేళ( nArg!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "NBYTE NMINALLOC");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) తిరిగి TCL_ERROR;
  ఒకవేళ( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) తిరిగి TCL_ERROR;

  ఒకవేళ( nByte==0 ){
    విడుదల( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_కుప్ప, (శూన్య*)0, 0, 0);
  }లేకపోతే{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_కుప్ప, zBuf, nByte, nMinAlloc);
  }

  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_కుప్ప_కొలత(
  శూన్య * client_దత్తాంశం, 
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం nByte;         /* Size to pass to sqlite3_config() */
  పూర్ణాంకం rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  పూర్ణాంకం nArg = objc-1;

  ఒకవేళ( nArg!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "NBYTE");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) తిరిగి TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_error(
  శూన్య * client_దత్తాంశం, 
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp*, స్థిరరాశి అక్షరం*, sqlite3**);

  ఒకవేళ( objc!=2 && objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "[DB]");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( objc==2 ){
    ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
    ఒకవేళ( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_Append_ఫలితం(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
  }లేకపోతే{
    ఒకవేళ( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_Append_ఫలితం(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
  }
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_uri(
  శూన్య * client_దత్తాంశం, 
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;
  పూర్ణాంకం bOpenUri;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "BOOL");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    తిరిగి TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);

  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_cis(
  శూన్య * client_దత్తాంశం, 
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;
  పూర్ణాంకం bUseCis;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "BOOL");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    తిరిగి TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_సూచీ_పరిశీలన, bUseCis);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);

  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_config_pmasz(
  శూన్య * client_దత్తాంశం, 
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;
  పూర్ణాంకం iPmaSz;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "BOOL");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    తిరిగి TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);

  తిరిగి TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_dump_memsys3(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "FILENAME");
    తిరిగి TCL_ERROR;
  }

  ఎంపిక( SQLITE_PTR_గమ్యం_INT(client_దత్తాంశం) ){
    స్థితి 3: {
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS3
      బాహ్య శూన్య sqlite3Memsys3Dump(స్థిరరాశి అక్షరం*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      ఆపు;
#అయితే_ముగింపు
    }
    స్థితి 5: {
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS5
      బాహ్య శూన్య sqlite3Memsys5Dump(స్థిరరాశి అక్షరం*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      ఆపు;
#అయితే_ముగింపు
    }
  }
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_స్థితి(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc, i_విలువ, mx_విలువ;
  పూర్ణాంకం i, op = 0, resetFlag;
  స్థిరరాశి అక్షరం *zOpName;
  స్థిర స్థిరరాశి నిర్మాణం {
    స్థిరరాశి అక్షరం *zName;
    పూర్ణాంకం op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_స్థితి_జ్ఞాపకం_వాడిన         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_స్థితి_MALLOC_కొలత         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_స్థితి_PAGECACHE_వాడిన      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_స్థితి_PAGECACHE_అధికప్రవాహం  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_స్థితి_PAGECACHE_కొలత      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_స్థితి_SCRATCH_వాడిన        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_స్థితి_SCRATCH_అధికప్రవాహం    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_స్థితి_SCRATCH_కొలత        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_స్థితి_PARSER_దొంతర        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_స్థితి_MALLOC_లెక్క        },
  };
  Tcl_Obj *p_ఫలితం;
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "PARAMETER RESETFLAG");
    తిరిగి TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  కొరకు(i=0; i<Array_కొలత(aOp); i++){
    ఒకవేళ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ఆపు;
    }
  }
  ఒకవేళ( i>=Array_కొలత(aOp) ){
    ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &op) ) తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) తిరిగి TCL_ERROR;
  i_విలువ = 0;
  mx_విలువ = 0;
  rc = sqlite3_స్థితి(op, &i_విలువ, &mx_విలువ, resetFlag);
  p_ఫలితం = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_ఫలితం, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_ఫలితం, Tcl_NewIntObj(i_విలువ));
  Tcl_ListObjAppendElement(0, p_ఫలితం, Tcl_NewIntObj(mx_విలువ));
  Tcl_సమితి_Obj_ఫలితం(interp, p_ఫలితం);
  తిరిగి TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_db_స్థితి(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc, i_విలువ, mx_విలువ;
  పూర్ణాంకం i, op = 0, resetFlag;
  స్థిరరాశి అక్షరం *zOpName;
  sqlite3 *db;
  బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp*, స్థిరరాశి అక్షరం*, sqlite3**);
  స్థిర స్థిరరాశి నిర్మాణం {
    స్థిరరాశి అక్షరం *zName;
    పూర్ణాంకం op;
  } aOp[] = {
    { "LOOKASIDE_USED",      SQLITE_DBSTATUS_LOOKASIDE_వాడిన      },
    { "CACHE_USED",          SQLITE_DBSTATUS_CACHE_వాడిన          },
    { "SCHEMA_USED",         SQLITE_DBSTATUS_SCHEMA_వాడిన         },
    { "STMT_USED",           SQLITE_DBSTATUS_STMT_వాడిన           },
    { "LOOKASIDE_HIT",       SQLITE_DBSTATUS_LOOKASIDE_తగిలింది       },
    { "LOOKASIDE_MISS_SIZE", SQLITE_DBSTATUS_LOOKASIDE_తప్పిపోవు_కొలత },
    { "LOOKASIDE_MISS_FULL", SQLITE_DBSTATUS_LOOKASIDE_తప్పిపోవు_నిండిన },
    { "CACHE_HIT",           SQLITE_DBSTATUS_CACHE_తగిలింది           },
    { "CACHE_MISS",          SQLITE_DBSTATUS_CACHE_తప్పిపోవు          },
    { "CACHE_WRITE",         SQLITE_DBSTATUS_CACHE_WRITE         },
    { "DEFERRED_FKS",        SQLITE_DBSTATUS_DEFERRED_FKS        },
    { "CACHE_USED_SHARED",   SQLITE_DBSTATUS_CACHE_వాడిన_SHARED   },
    { "CACHE_SPILL",         SQLITE_DBSTATUS_CACHE_SPILL         },
    { "TEMPBUF_SPILL",       SQLITE_DBSTATUS_TEMPBUF_SPILL       },
  };
  Tcl_Obj *p_ఫలితం;
  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB PARAMETER RESETFLAG");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  ఒకవేళ( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  ఒకవేళ( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  కొరకు(i=0; i<Array_కొలత(aOp); i++){
    ఒకవేళ( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ఆపు;
    }
  }
  ఒకవేళ( i>=Array_కొలత(aOp) ){
    ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &op) ) తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) తిరిగి TCL_ERROR;
  i_విలువ = 0;
  mx_విలువ = 0;
  rc = sqlite3_db_స్థితి(db, op, &i_విలువ, &mx_విలువ, resetFlag);
  p_ఫలితం = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_ఫలితం, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_ఫలితం, Tcl_NewIntObj(i_విలువ));
  Tcl_ListObjAppendElement(0, p_ఫలితం, Tcl_NewIntObj(mx_విలువ));
  Tcl_సమితి_Obj_ఫలితం(interp, p_ఫలితం);
  తిరిగి TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_install_malloc_faultsim(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc;
  పూర్ణాంకం isInstall;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "BOOLEAN");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    తిరిగి TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_install_memsys3(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం rc = SQLITE_MISUSE;
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS3
  స్థిరరాశి sqlite3_mem_methods *sqlite3MemGetMemsys3(శూన్య);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#అయితే_ముగింపు
  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_VOLATILE);
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_vfs_oom_test(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  బాహ్య పూర్ణాంకం sqlite3_memdebug_vfs_oom_test;
  ఒకవేళ( objc>2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "?INTEGER?");
    తిరిగి TCL_ERROR;
  }లేకపోతే ఒకవేళ( objc==2 ){
    పూర్ణాంకం iNew;
    ఒకవేళ( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) తిరిగి TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  తిరిగి TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest_malloc_Init(Tcl_Interp *interp){
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     పూర్ణాంకం client_దత్తాంశం;
  } aObjCmd[] = {
     { "sqlite3_malloc",             test_malloc                   ,0 },
     { "sqlite3_realloc",            test_realloc                  ,0 },
     { "sqlite3_free",               test_free                     ,0 },
     { "memset",                     test_memset                   ,0 },
     { "memget",                     test_memget                   ,0 },
     { "sqlite3_memory_used",        test_జ్ఞాపకం_వాడిన              ,0 },
     { "sqlite3_memory_highwater",   test_జ్ఞాపకం_highwater         ,0 },
     { "sqlite3_memdebug_backtrace", test_memdebug_backtrace       ,0 },
     { "sqlite3_memdebug_dump",      test_memdebug_dump            ,0 },
     { "sqlite3_memdebug_fail",      test_memdebug_fail            ,0 },
     { "sqlite3_memdebug_pending",   test_memdebug_pending         ,0 },
     { "sqlite3_memdebug_settitle",  test_memdebug_settitle        ,0 },
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_లెక్క ,0 },
     { "sqlite3_memdebug_log",       test_memdebug_log             ,0 },
     { "sqlite3_config_pagecache",   test_config_pagecache         ,0 },
     { "sqlite3_config_alt_pcache",  test_alt_pcache               ,0 },
     { "sqlite3_status",             test_స్థితి                   ,0 },
     { "sqlite3_db_status",          test_db_స్థితి                ,0 },
     { "install_malloc_faultsim",    test_install_malloc_faultsim  ,0 },
     { "sqlite3_config_heap",        test_config_కుప్ప              ,0 },
     { "sqlite3_config_heap_size",   test_config_కుప్ప_కొలత         ,0 },
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
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Client_దత్తాంశం c = (Client_దత్తాంశం)SQLITE_INT_గమ్యం_PTR(aObjCmd[i].client_దత్తాంశం);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  తిరిగి TCL_OK;
}
#అయితే_ముగింపు
