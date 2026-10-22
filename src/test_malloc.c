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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
ساکن ساخت MemFault {
  صحیح iCountdown;         /* Number of pending successes before a failure */
  صحیح nRepeat;            /* Number of times to repeat the failure */
  صحیح nBenign;            /* Number of benign failures seen since last config */
  صحیح nFail;              /* Number of failures seen since last config */
  صحیح nOkBefore;          /* Successful allocations prior to the first fault */
  صحیح nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  صحیح isInstalled;        /* True if the fault simulation layer is installed */
  صحیح is_Benign_منوال;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
ساکن خالی sqlite3Fault(خالی){
  ساکن u64 cnt = 0;
  cnt++;
  اگر( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
ساکن خالی sqlite3_پہلا_Fault(خالی){
  ساکن u64 cnt2 = 0;
  cnt2++;
  اگر( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
ساکن صحیح faultsim_قدم(خالی){
  اگر( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    واپس 0;
  }
  اگر( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    واپس 0;
  }
  اگر( memfault.nFail==0 ) sqlite3_پہلا_Fault();
  sqlite3Fault();
  memfault.nFail++;
  اگر( memfault.is_Benign_منوال>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  اگر( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  واپس 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
ساکن خالی *faultsimMalloc(صحیح n){
  خالی *p = 0;
  اگر( !faultsim_قدم() ){
    p = memfault.m.xMalloc(n);
  }
  واپس p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
ساکن خالی *faultsimRealloc(خالی *pOld, صحیح n){
  خالی *p = 0;
  اگر( !faultsim_قدم() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  واپس p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
ساکن خالی faultsimConfig(صحیح nDelay, صحیح nRepeat){
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
  memfault.is_Benign_منوال = 0;
}

/*
** Return the number of faults (both hard and benign faults) that have
** occurred since the injector was last configured.
*/
ساکن صحیح faultsimFailures(خالی){
  واپس memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
ساکن صحیح faultsimBenignFailures(خالی){
  واپس memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
ساکن صحیح faultsimPending(خالی){
  اگر( memfault.enable ){
    واپس memfault.iCountdown;
  }ورنہ{
    واپس -1;
  }
}


ساکن خالی faultsimBeginBenign(خالی){
  memfault.is_Benign_منوال++;
}
ساکن خالی faultsim_انتہا_Benign(خالی){
  memfault.is_Benign_منوال--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
ساکن صحیح faultsimInstall(صحیح install){
  صحیح rc;

  install = (install ? 1 : 0);
  أكد(memfault.isInstalled==1 || memfault.isInstalled==0);

  اگر( install==memfault.isInstalled ){
    واپس SQLITE_ERROR;
  }

  اگر( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    أكد(memfault.m.xMalloc);
    اگر( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsim_انتہا_Benign
    );
  }ورنہ{
    sqlite3_mem_methods m2;
    أكد(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, حجم(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    أكد( memcmp(&m2, &memfault.m, حجم(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (خالی*)0, (خالی*)0);
  }

  اگر( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  واپس rc;
}

#اگر_تعریف_شدہ SQLITE_TEST

/*
** This function is implemented in main.c. Returns a pointer to a static
** buffer containing the symbolic SQLite error code that corresponds to
** the least-significant 8-bits of the integer passed as an argument.
** For example:
**
**   sqlite3ErrName(1) -> "SQLITE_ERROR"
*/
خارجی ثابت حرف *sqlite3ErrName(صحیح);

/*
** Transform pointers to text and back again
*/
ساکن خالی اشارہ_گر_مقصد_متن(خالی *p, حرف *z){
  ثابت حرف *zRet = sqlite3_Test_Ptr_مقصد_متن(p);
  sprintf(z, "%s", zRet);
}
ساکن صحیح متن_مقصد_اشارہ_گر(ثابت حرف *z, خالی **pp){
  *pp = sqlite3_Test_متن_مقصد_Ptr(z);
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
ساکن صحیح SQLITE_TCLAPI test_malloc(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nByte;
  خالی *p;
  حرف z_برآمد[100];
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "NBYTES");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) واپس TCL_ERROR;
  p = sqlite3_malloc((غیرعلامتی)nByte);
  اشارہ_گر_مقصد_متن(p, z_برآمد);
  Tcl_Append_نتیجہ(interp, z_برآمد, تہی);
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
ساکن صحیح SQLITE_TCLAPI test_realloc(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nByte;
  خالی *p_پیشگی, *p;
  حرف z_برآمد[100];
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "PRIOR NBYTES");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) واپس TCL_ERROR;
  اگر( متن_مقصد_اشارہ_گر(Tcl_GetString(objv[1]), &p_پیشگی) ){
    Tcl_Append_نتیجہ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    واپس TCL_ERROR;
  }
  p = sqlite3_realloc(p_پیشگی, (غیرعلامتی)nByte);
  اشارہ_گر_مقصد_متن(p, z_برآمد);
  Tcl_Append_نتیجہ(interp, z_برآمد, تہی);
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
ساکن صحیح SQLITE_TCLAPI test_free(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خالی *p_پیشگی;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "PRIOR");
    واپس TCL_ERROR;
  }
  اگر( متن_مقصد_اشارہ_گر(Tcl_GetString(objv[1]), &p_پیشگی) ){
    Tcl_Append_نتیجہ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    واپس TCL_ERROR;
  }
  sqlite3_free(p_پیشگی);
  واپس TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
صحیح sqlite3_Test_Hex_مقصد_Bin(ثابت حرف *, صحیح, حرف *);
صحیح sqlite3_Test_Bin_مقصد_Hex(حرف*,صحیح);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
ساکن صحیح SQLITE_TCLAPI test_memset(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خالی *p;
  صحیح سائز, i;
  Tcl_سائز n;
  حرف *zHex;
  حرف *z_برآمد;
  حرف zBin[100];

  اگر( objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "ADDRESS SIZE HEX");
    واپس TCL_ERROR;
  }
  اگر( متن_مقصد_اشارہ_گر(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_نتیجہ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &سائز) ){
    واپس TCL_ERROR;
  }
  اگر( سائز<=0 ){
    Tcl_Append_نتیجہ(interp, "size must be positive", (حرف*)0);
    واپس TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  اگر( n>حجم(zBin)*2 ) n = حجم(zBin)*2;
  n = sqlite3_Test_Hex_مقصد_Bin(zHex, (صحیح)n, zBin);
  اگر( n==0 ){
    Tcl_Append_نتیجہ(interp, "no data", (حرف*)0);
    واپس TCL_ERROR;
  }
  z_برآمد = p;
  برائے(i=0; i<سائز; i++){
    z_برآمد[i] = zBin[i%n];
  }
  واپس TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
ساکن صحیح SQLITE_TCLAPI test_memget(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خالی *p;
  صحیح سائز, n;
  حرف *zBin;
  حرف zHex[100];

  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "ADDRESS SIZE");
    واپس TCL_ERROR;
  }
  اگر( متن_مقصد_اشارہ_گر(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_نتیجہ(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &سائز) ){
    واپس TCL_ERROR;
  }
  اگر( سائز<=0 ){
    Tcl_Append_نتیجہ(interp, "size must be positive", (حرف*)0);
    واپس TCL_ERROR;
  }
  zBin = p;
  جب( سائز>0 ){
    اگر( سائز>(حجم(zHex)-1)/2 ){
      n = (حجم(zHex)-1)/2;
    }ورنہ{
      n = سائز;
    }
    حافظہ_نقل(zHex, zBin, n);
    zBin += n;
    سائز -= n;
    sqlite3_Test_Bin_مقصد_Hex(zHex, n);
    Tcl_Append_نتیجہ(interp, zHex, (حرف*)0);
  }
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
ساکن صحیح SQLITE_TCLAPI test_حافظہ_مستعمل(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewWideIntObj(sqlite3_حافظہ_مستعمل()));
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
ساکن صحیح SQLITE_TCLAPI test_حافظہ_highwater(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح resetFlag = 0;
  اگر( objc!=1 && objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "?RESET?");
    واپس TCL_ERROR;
  }
  اگر( objc==2 ){
    اگر( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) واپس TCL_ERROR;
  } 
  Tcl_سیٹ_Obj_نتیجہ(interp, 
     Tcl_NewWideIntObj(sqlite3_حافظہ_highwater(resetFlag)));
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
ساکن صحیح SQLITE_TCLAPI test_memdebug_backtrace(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح گہرائی;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DEPT");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &گہرائی) ) واپس TCL_ERROR;
#اگر_تعریف_شدہ SQLITE_MEMDEBUG
  {
    خارجی خالی sqlite3MemdebugBacktrace(صحیح);
    sqlite3MemdebugBacktrace(گہرائی);
  }
#اگر_ختم
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
ساکن صحیح SQLITE_TCLAPI test_memdebug_dump(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "FILENAME");
    واپس TCL_ERROR;
  }
#اگر defined(SQLITE_MEMDEBUG) || defined(SQLITE_حافظہ_سائز) \
     || defined(SQLITE_POW2_حافظہ_سائز)
  {
    خارجی خالی sqlite3MemdebugDump(ثابت حرف*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#اگر_ختم
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
ساکن صحیح SQLITE_TCLAPI test_memdebug_malloc_گنتی(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nMalloc = -1;
  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }
#اگر defined(SQLITE_MEMDEBUG)
  {
    خارجی صحیح sqlite3_Memdebug_Malloc_گنتی();
    nMalloc = sqlite3_Memdebug_Malloc_گنتی();
  }
#اگر_ختم
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(nMalloc));
  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI test_memdebug_fail(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح ii;
  صحیح iFail;
  صحیح nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  صحیح nBenign;
  صحیح nFail = 0;

  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "COUNTER ?OPTIONS?");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) واپس TCL_ERROR;

  برائے(ii=2; ii<objc; ii+=2){
    Tcl_سائز nOption;
    حرف *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    حرف *zErr = 0;

    اگر( nOption>1 && لڑی_موازنہ(zOption, "-repeat", nOption)==0 ){
      اگر( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ورنہ{
        اگر( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          واپس TCL_ERROR;
        }
      }
    }ورنہ اگر( nOption>1 && لڑی_موازنہ(zOption, "-benigncnt", nOption)==0 ){
      اگر( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }ورنہ{
        pBenignCnt = objv[ii+1];
      }
    }ورنہ{
      zErr = "unknown option: ";
    }

    اگر( zErr ){
      Tcl_Append_نتیجہ(interp, zErr, zOption, تہی);
      واپس TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  اگر( pBenignCnt ){
    Tcl_Obj_سیٹ_Var2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(nFail));
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
ساکن صحیح SQLITE_TCLAPI test_memdebug_pending(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nPending;
  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(nPending));
  واپس TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
ساکن صحیح sqlite3_memdebug_title_گنتی = 0;

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
ساکن صحیح SQLITE_TCLAPI test_memdebug_settitle(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_گنتی++;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "TITLE");
    واپس TCL_ERROR;
  }
#اگر_تعریف_شدہ SQLITE_MEMDEBUG
  {
    ثابت حرف *zTitle;
    خارجی صحیح sqlite3MemdebugSettitle(ثابت حرف*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#اگر_ختم
  واپس TCL_OK;
}

#تعریف_کرو MALLOC_LOG_چوکھٹے  10 
#تعریف_کرو MALLOC_LOG_KEYINTS (                                              \
    10 * ((حجم(صحیح)>=حجم(خالی*)) ? 1 : حجم(خالی*)/حجم(صحیح))   \
)
ساکن Tcl_Hash_جدول aMallocLog;
ساکن صحیح mallocLogEnabled = 0;

قسمتعریف ساخت MallocLog MallocLog;
ساخت MallocLog {
  صحیح nCall;
  صحیح nByte;
};

#اگر_تعریف_شدہ SQLITE_MEMDEBUG
ساکن خالی test_memdebug_callback(صحیح nByte, صحیح n_چوکھٹا, خالی **a_چوکھٹا){
  اگر( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_Hash_اندراج *p_اندراج;
    صحیح isNew;

    صحیح a_کلید[MALLOC_LOG_KEYINTS];
    غیرعلامتی صحیح n_کلید = حجم(صحیح)*MALLOC_LOG_KEYINTS;

    memset(a_کلید, 0, n_کلید);
    اگر( (حجم(خالی*)*n_چوکھٹا)<n_کلید ){
      n_کلید = n_چوکھٹا*حجم(خالی*);
    }
    حافظہ_نقل(a_کلید, a_چوکھٹا, n_کلید);

    p_اندراج = Tcl_Create_Hash_اندراج(&aMallocLog, (ثابت حرف *)a_کلید, &isNew);
    اگر( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(حجم(MallocLog));
      memset(pLog, 0, حجم(MallocLog));
      Tcl_سیٹ_Hash_قدر(p_اندراج, (Client_ڈیٹا)pLog);
    }ورنہ{
      pLog = (MallocLog *)Tcl_Get_Hash_قدر(p_اندراج);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#اگر_ختم /* SQLITE_MEMDEBUG */

ساکن خالی test_memdebug_log_clear(خالی){
  Tcl_Hash_تلاش تلاش;
  Tcl_Hash_اندراج *p_اندراج;
  برائے(
    p_اندراج=Tcl_پہلا_Hash_اندراج(&aMallocLog, &تلاش);
    p_اندراج;
    p_اندراج=Tcl_اگلا_Hash_اندراج(&تلاش)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_قدر(p_اندراج);
    Tcl_Free((حرف *)pLog);
  }
  Tcl_Delete_Hash_جدول(&aMallocLog);
  Tcl_Init_Hash_جدول(&aMallocLog, MALLOC_LOG_KEYINTS);
}

ساکن صحیح SQLITE_TCLAPI test_memdebug_log(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساکن صحیح isInit = 0;
  صحیح iSub;

  ساکن ثابت حرف *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  شمار MB_enum { 
      MB_LOG_آغاز, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  اگر( !isInit ){
#اگر_تعریف_شدہ SQLITE_MEMDEBUG
    خارجی خالی sqlite3MemdebugBacktraceCallback(
        خالی (*xBacktrace)(صحیح, صحیح, خالی **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#اگر_ختم
    Tcl_Init_Hash_جدول(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SUB-COMMAND ...");
  }
  اگر( Tcl_Get_اشاریہ_From_Obj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    واپس TCL_ERROR;
  }

  انتخاب( (شمار MB_enum)iSub ){
    حالت MB_LOG_آغاز:
      mallocLogEnabled = 1;
      توڑو;
    حالت MB_LOG_STOP:
      mallocLogEnabled = 0;
      توڑو;
    حالت MB_LOG_DUMP: {
      Tcl_Hash_تلاش تلاش;
      Tcl_Hash_اندراج *p_اندراج;
      Tcl_Obj *pRet = Tcl_NewObj();

      أكد(حجم(Tcl_WideInt)>=حجم(خالی*));

      برائے(
        p_اندراج=Tcl_پہلا_Hash_اندراج(&aMallocLog, &تلاش);
        p_اندراج;
        p_اندراج=Tcl_اگلا_Hash_اندراج(&تلاش)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_چوکھٹے+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_قدر(p_اندراج);
        Tcl_WideInt *a_کلید = (Tcl_WideInt *)Tcl_Get_Hash_کلید(&aMallocLog, p_اندراج);
        صحیح ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        برائے(ii=0; ii<MALLOC_LOG_چوکھٹے; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_کلید[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_چوکھٹے+2, apElem)
        );
      }

      Tcl_سیٹ_Obj_نتیجہ(interp, pRet);
      توڑو;
    }
    حالت MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      توڑو;
    }

    حالت MB_LOG_SYNC: {
#اگر_تعریف_شدہ SQLITE_MEMDEBUG
      خارجی خالی sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#اگر_ختم
      توڑو;
    }
  }

  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI test_config_pagecache(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح sz, N;
  Tcl_Obj *pRes;
  ساکن حرف *buf = 0;
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SIZE N");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &sz) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[2], &N) ) واپس TCL_ERROR;
  آزاد(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.sz_صفحہ));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.n_صفحہ));
  Tcl_سیٹ_Obj_نتیجہ(interp, pRes);

  اگر( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (خالی*)0, 0, 0);
  }ورنہ{
    buf = حافظہ_دو( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  واپس TCL_OK;
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
ساکن صحیح SQLITE_TCLAPI test_alt_pcache(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح installFlag;
  صحیح discardChance = 0;
  صحیح prng_بیج_2 = 0;
  صحیح بلند_Stress = 0;
  خارجی خالی installTestPCache(صحیح,غیرعلامتی,غیرعلامتی,غیرعلامتی);
  اگر( objc<2 || objc>5 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) واپس TCL_ERROR;
  اگر( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     واپس TCL_ERROR;
  }
  اگر( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prng_بیج_2) ){
     واپس TCL_ERROR;
  }
  اگر( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &بلند_Stress) ){
    واپس TCL_ERROR;
  }
  اگر( discardChance<0 || discardChance>100 ){
    Tcl_Append_نتیجہ(interp, "discard-chance should be between 0 and 100",
                     (حرف*)0);
    واپس TCL_ERROR;
  }
  installTestPCache(installFlag, (غیرعلامتی)discardChance, (غیرعلامتی)prng_بیج_2,
                    (غیرعلامتی)بلند_Stress);
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
ساکن صحیح SQLITE_TCLAPI test_config_memstatus(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح enable, rc;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "BOOLEAN");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) واپس TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
ساکن صحیح SQLITE_TCLAPI test_config_lookaside(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح sz, cnt;
  Tcl_Obj *pRet;
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SIZE COUNT");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &sz) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) واپس TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_سیٹ_Obj_نتیجہ(interp, pRet);
  واپس TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
ساکن صحیح SQLITE_TCLAPI test_db_config_lookaside(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح sz, cnt;
  sqlite3 *db;
  صحیح bufid;
  ساکن حرف azBuf[2][10000];
  خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  اگر( objc!=5 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "BUFID SIZE COUNT");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[3], &sz) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) واپس TCL_ERROR;
  اگر( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (خالی*)0, sz, cnt);
  }ورنہ اگر( bufid>=1 && bufid<=2 && sz*cnt<=حجم(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }ورنہ{
    Tcl_Append_نتیجہ(interp, "illegal arguments - see documentation", (حرف*)0);
    واپس TCL_ERROR;
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(rc));
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
ساکن صحیح SQLITE_TCLAPI test_config_ڈھیر(
  خالی * client_ڈیٹا, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساکن حرف *zBuf; /* Use this memory */
  صحیح nByte;         /* Size of buffer to pass to sqlite3_config() */
  صحیح nMinAlloc;     /* Size of minimum allocation */
  صحیح rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  صحیح nArg = objc-1;

  اگر( nArg!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "NBYTE NMINALLOC");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) واپس TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) واپس TCL_ERROR;

  اگر( nByte==0 ){
    آزاد( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_ڈھیر, (خالی*)0, 0, 0);
  }ورنہ{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_ڈھیر, zBuf, nByte, nMinAlloc);
  }

  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
ساکن صحیح SQLITE_TCLAPI test_config_ڈھیر_سائز(
  خالی * client_ڈیٹا, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nByte;         /* Size to pass to sqlite3_config() */
  صحیح rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  صحیح nArg = objc-1;

  اگر( nArg!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "NBYTE");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) واپس TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
ساکن صحیح SQLITE_TCLAPI test_config_error(
  خالی * client_ڈیٹا, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);

  اگر( objc!=2 && objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "[DB]");
    واپس TCL_ERROR;
  }
  اگر( objc==2 ){
    اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
    اگر( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_Append_نتیجہ(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (حرف*)0);
      واپس TCL_ERROR;
    }
  }ورنہ{
    اگر( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_Append_نتیجہ(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (حرف*)0);
      واپس TCL_ERROR;
    }
  }
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
ساکن صحیح SQLITE_TCLAPI test_config_uri(
  خالی * client_ڈیٹا, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح bOpenUri;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "BOOL");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    واپس TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  واپس TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
ساکن صحیح SQLITE_TCLAPI test_config_cis(
  خالی * client_ڈیٹا, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح bUseCis;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "BOOL");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    واپس TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_اشاریہ_جائزہ, bUseCis);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  واپس TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
ساکن صحیح SQLITE_TCLAPI test_config_pmasz(
  خالی * client_ڈیٹا, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح iPmaSz;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "BOOL");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    واپس TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  واپس TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
ساکن صحیح SQLITE_TCLAPI test_dump_memsys3(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "FILENAME");
    واپس TCL_ERROR;
  }

  انتخاب( SQLITE_PTR_مقصد_INT(client_ڈیٹا) ){
    حالت 3: {
#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS3
      خارجی خالی sqlite3Memsys3Dump(ثابت حرف*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      توڑو;
#اگر_ختم
    }
    حالت 5: {
#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS5
      خارجی خالی sqlite3Memsys5Dump(ثابت حرف*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      توڑو;
#اگر_ختم
    }
  }
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
ساکن صحیح SQLITE_TCLAPI test_حالت(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc, i_قدر, mx_قدر;
  صحیح i, op = 0, resetFlag;
  ثابت حرف *zOpName;
  ساکن ثابت ساخت {
    ثابت حرف *zName;
    صحیح op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_حالت_حافظہ_مستعمل         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_حالت_MALLOC_سائز         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_حالت_PAGECACHE_مستعمل      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_حالت_PAGECACHE_حد_سے_تجاوز  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_حالت_PAGECACHE_سائز      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_حالت_SCRATCH_مستعمل        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_حالت_SCRATCH_حد_سے_تجاوز    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_حالت_SCRATCH_سائز        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_حالت_PARSER_ڈھیر        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_حالت_MALLOC_گنتی        },
  };
  Tcl_Obj *p_نتیجہ;
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "PARAMETER RESETFLAG");
    واپس TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  برائے(i=0; i<Array_سائز(aOp); i++){
    اگر( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      توڑو;
    }
  }
  اگر( i>=Array_سائز(aOp) ){
    اگر( Tcl_GetIntFromObj(interp, objv[1], &op) ) واپس TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) واپس TCL_ERROR;
  i_قدر = 0;
  mx_قدر = 0;
  rc = sqlite3_حالت(op, &i_قدر, &mx_قدر, resetFlag);
  p_نتیجہ = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_نتیجہ, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_نتیجہ, Tcl_NewIntObj(i_قدر));
  Tcl_ListObjAppendElement(0, p_نتیجہ, Tcl_NewIntObj(mx_قدر));
  Tcl_سیٹ_Obj_نتیجہ(interp, p_نتیجہ);
  واپس TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
ساکن صحیح SQLITE_TCLAPI test_db_حالت(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc, i_قدر, mx_قدر;
  صحیح i, op = 0, resetFlag;
  ثابت حرف *zOpName;
  sqlite3 *db;
  خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  ساکن ثابت ساخت {
    ثابت حرف *zName;
    صحیح op;
  } aOp[] = {
    { "LOOKASIDE_USED",      SQLITE_DBSTATUS_LOOKASIDE_مستعمل      },
    { "CACHE_USED",          SQLITE_DBSTATUS_CACHE_مستعمل          },
    { "SCHEMA_USED",         SQLITE_DBSTATUS_SCHEMA_مستعمل         },
    { "STMT_USED",           SQLITE_DBSTATUS_STMT_مستعمل           },
    { "LOOKASIDE_HIT",       SQLITE_DBSTATUS_LOOKASIDE_کامیابی       },
    { "LOOKASIDE_MISS_SIZE", SQLITE_DBSTATUS_LOOKASIDE_ناکامی_سائز },
    { "LOOKASIDE_MISS_FULL", SQLITE_DBSTATUS_LOOKASIDE_ناکامی_بھرا },
    { "CACHE_HIT",           SQLITE_DBSTATUS_CACHE_کامیابی           },
    { "CACHE_MISS",          SQLITE_DBSTATUS_CACHE_ناکامی          },
    { "CACHE_WRITE",         SQLITE_DBSTATUS_CACHE_WRITE         },
    { "DEFERRED_FKS",        SQLITE_DBSTATUS_DEFERRED_FKS        },
    { "CACHE_USED_SHARED",   SQLITE_DBSTATUS_CACHE_مستعمل_SHARED   },
    { "CACHE_SPILL",         SQLITE_DBSTATUS_CACHE_SPILL         },
    { "TEMPBUF_SPILL",       SQLITE_DBSTATUS_TEMPBUF_SPILL       },
  };
  Tcl_Obj *p_نتیجہ;
  اگر( objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB PARAMETER RESETFLAG");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  اگر( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  اگر( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  برائے(i=0; i<Array_سائز(aOp); i++){
    اگر( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      توڑو;
    }
  }
  اگر( i>=Array_سائز(aOp) ){
    اگر( Tcl_GetIntFromObj(interp, objv[2], &op) ) واپس TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) واپس TCL_ERROR;
  i_قدر = 0;
  mx_قدر = 0;
  rc = sqlite3_db_حالت(db, op, &i_قدر, &mx_قدر, resetFlag);
  p_نتیجہ = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_نتیجہ, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_نتیجہ, Tcl_NewIntObj(i_قدر));
  Tcl_ListObjAppendElement(0, p_نتیجہ, Tcl_NewIntObj(mx_قدر));
  Tcl_سیٹ_Obj_نتیجہ(interp, p_نتیجہ);
  واپس TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
ساکن صحیح SQLITE_TCLAPI test_install_malloc_faultsim(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح isInstall;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "BOOLEAN");
    واپس TCL_ERROR;
  }
  اگر( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    واپس TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  واپس TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
ساکن صحیح SQLITE_TCLAPI test_install_memsys3(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = SQLITE_MISUSE;
#اگر_تعریف_شدہ SQLITE_ENABLE_MEMSYS3
  ثابت sqlite3_mem_methods *sqlite3MemGetMemsys3(خالی);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#اگر_ختم
  Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_vfs_oom_test(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خارجی صحیح sqlite3_memdebug_vfs_oom_test;
  اگر( objc>2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "?INTEGER?");
    واپس TCL_ERROR;
  }ورنہ اگر( objc==2 ){
    صحیح iNew;
    اگر( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) واپس TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  واپس TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_malloc_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     صحیح client_ڈیٹا;
  } aObjCmd[] = {
     { "sqlite3_malloc",             test_malloc                   ,0 },
     { "sqlite3_realloc",            test_realloc                  ,0 },
     { "sqlite3_free",               test_free                     ,0 },
     { "memset",                     test_memset                   ,0 },
     { "memget",                     test_memget                   ,0 },
     { "sqlite3_memory_used",        test_حافظہ_مستعمل              ,0 },
     { "sqlite3_memory_highwater",   test_حافظہ_highwater         ,0 },
     { "sqlite3_memdebug_backtrace", test_memdebug_backtrace       ,0 },
     { "sqlite3_memdebug_dump",      test_memdebug_dump            ,0 },
     { "sqlite3_memdebug_fail",      test_memdebug_fail            ,0 },
     { "sqlite3_memdebug_pending",   test_memdebug_pending         ,0 },
     { "sqlite3_memdebug_settitle",  test_memdebug_settitle        ,0 },
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_گنتی ,0 },
     { "sqlite3_memdebug_log",       test_memdebug_log             ,0 },
     { "sqlite3_config_pagecache",   test_config_pagecache         ,0 },
     { "sqlite3_config_alt_pcache",  test_alt_pcache               ,0 },
     { "sqlite3_status",             test_حالت                   ,0 },
     { "sqlite3_db_status",          test_db_حالت                ,0 },
     { "install_malloc_faultsim",    test_install_malloc_faultsim  ,0 },
     { "sqlite3_config_heap",        test_config_ڈھیر              ,0 },
     { "sqlite3_config_heap_size",   test_config_ڈھیر_سائز         ,0 },
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
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Client_ڈیٹا c = (Client_ڈیٹا)SQLITE_INT_مقصد_PTR(aObjCmd[i].client_ڈیٹا);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  واپس TCL_OK;
}
#اگر_ختم
