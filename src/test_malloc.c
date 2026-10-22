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
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
ساكن بنية MemFault {
  صحيح iCountdown;         /* Number of pending successes before a failure */
  صحيح nRepeat;            /* Number of times to repeat the failure */
  صحيح nBenign;            /* Number of benign failures seen since last config */
  صحيح nFail;              /* Number of failures seen since last config */
  صحيح nOkBefore;          /* Successful allocations prior to the first fault */
  صحيح nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  صحيح isInstalled;        /* True if the fault simulation layer is installed */
  صحيح is_Benign_ډېرپېښ;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
ساكن فراغ sqlite3Fault(فراغ){
  ساكن u64 cnt = 0;
  cnt++;
  إذا( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
ساكن فراغ sqlite3_لومړی_Fault(فراغ){
  ساكن u64 cnt2 = 0;
  cnt2++;
  إذا( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
ساكن صحيح faultsim_ګام(فراغ){
  إذا( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    أرجع 0;
  }
  إذا( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    أرجع 0;
  }
  إذا( memfault.nFail==0 ) sqlite3_لومړی_Fault();
  sqlite3Fault();
  memfault.nFail++;
  إذا( memfault.is_Benign_ډېرپېښ>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  إذا( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  أرجع 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
ساكن فراغ *faultsimMalloc(صحيح n){
  فراغ *p = 0;
  إذا( !faultsim_ګام() ){
    p = memfault.m.xMalloc(n);
  }
  أرجع p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
ساكن فراغ *faultsimRealloc(فراغ *pOld, صحيح n){
  فراغ *p = 0;
  إذا( !faultsim_ګام() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  أرجع p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
ساكن فراغ faultsimConfig(صحيح nDelay, صحيح nRepeat){
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
  memfault.is_Benign_ډېرپېښ = 0;
}

/*
** Return the number of faults (both hard and benign faults) that have
** occurred since the injector was last configured.
*/
ساكن صحيح faultsimFailures(فراغ){
  أرجع memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
ساكن صحيح faultsimBenignFailures(فراغ){
  أرجع memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
ساكن صحيح faultsimPending(فراغ){
  إذا( memfault.enable ){
    أرجع memfault.iCountdown;
  }وإلا{
    أرجع -1;
  }
}


ساكن فراغ faultsimBeginBenign(فراغ){
  memfault.is_Benign_ډېرپېښ++;
}
ساكن فراغ faultsim_پای_ټکی_Benign(فراغ){
  memfault.is_Benign_ډېرپېښ--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
ساكن صحيح faultsimInstall(صحيح install){
  صحيح rc;

  install = (install ? 1 : 0);
  أكد(memfault.isInstalled==1 || memfault.isInstalled==0);

  إذا( install==memfault.isInstalled ){
    أرجع SQLITE_ERROR;
  }

  إذا( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    أكد(memfault.m.xMalloc);
    إذا( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsim_پای_ټکی_Benign
    );
  }وإلا{
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
        (فراغ*)0, (فراغ*)0);
  }

  إذا( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  أرجع rc;
}

#که_تعریف_شوی SQLITE_TEST

/*
** This function is implemented in main.c. Returns a pointer to a static
** buffer containing the symbolic SQLite error code that corresponds to
** the least-significant 8-bits of the integer passed as an argument.
** For example:
**
**   sqlite3ErrName(1) -> "SQLITE_ERROR"
*/
خارجي ثابت حرف *sqlite3ErrName(صحيح);

/*
** Transform pointers to text and back again
*/
ساكن فراغ اشاره_ګر_منزل_متن(فراغ *p, حرف *z){
  ثابت حرف *zRet = sqlite3_Test_Ptr_منزل_متن(p);
  sprintf(z, "%s", zRet);
}
ساكن صحيح متن_منزل_اشاره_ګر(ثابت حرف *z, فراغ **pp){
  *pp = sqlite3_Test_متن_منزل_Ptr(z);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
ساكن صحيح SQLITE_TCLAPI test_malloc(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nByte;
  فراغ *p;
  حرف z_بهر[100];
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "NBYTES");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) أرجع TCL_ERROR;
  p = sqlite3_malloc((بینښه)nByte);
  اشاره_ګر_منزل_متن(p, z_بهر);
  Tcl_Append_پایله(interp, z_بهر, عدم);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
ساكن صحيح SQLITE_TCLAPI test_realloc(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nByte;
  فراغ *p_مخکینی, *p;
  حرف z_بهر[100];
  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "PRIOR NBYTES");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) أرجع TCL_ERROR;
  إذا( متن_منزل_اشاره_ګر(Tcl_GetString(objv[1]), &p_مخکینی) ){
    Tcl_Append_پایله(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    أرجع TCL_ERROR;
  }
  p = sqlite3_realloc(p_مخکینی, (بینښه)nByte);
  اشاره_ګر_منزل_متن(p, z_بهر);
  Tcl_Append_پایله(interp, z_بهر, عدم);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
ساكن صحيح SQLITE_TCLAPI test_free(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ *p_مخکینی;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "PRIOR");
    أرجع TCL_ERROR;
  }
  إذا( متن_منزل_اشاره_ګر(Tcl_GetString(objv[1]), &p_مخکینی) ){
    Tcl_Append_پایله(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    أرجع TCL_ERROR;
  }
  sqlite3_free(p_مخکینی);
  أرجع TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
صحيح sqlite3_Test_Hex_منزل_Bin(ثابت حرف *, صحيح, حرف *);
صحيح sqlite3_Test_Bin_منزل_Hex(حرف*,صحيح);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
ساكن صحيح SQLITE_TCLAPI test_memset(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ *p;
  صحيح اندازه, i;
  Tcl_اندازه n;
  حرف *zHex;
  حرف *z_بهر;
  حرف zBin[100];

  إذا( objc!=4 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "ADDRESS SIZE HEX");
    أرجع TCL_ERROR;
  }
  إذا( متن_منزل_اشاره_ګر(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_پایله(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &اندازه) ){
    أرجع TCL_ERROR;
  }
  إذا( اندازه<=0 ){
    Tcl_Append_پایله(interp, "size must be positive", (حرف*)0);
    أرجع TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  إذا( n>حجم(zBin)*2 ) n = حجم(zBin)*2;
  n = sqlite3_Test_Hex_منزل_Bin(zHex, (صحيح)n, zBin);
  إذا( n==0 ){
    Tcl_Append_پایله(interp, "no data", (حرف*)0);
    أرجع TCL_ERROR;
  }
  z_بهر = p;
  لكل(i=0; i<اندازه; i++){
    z_بهر[i] = zBin[i%n];
  }
  أرجع TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
ساكن صحيح SQLITE_TCLAPI test_memget(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ *p;
  صحيح اندازه, n;
  حرف *zBin;
  حرف zHex[100];

  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "ADDRESS SIZE");
    أرجع TCL_ERROR;
  }
  إذا( متن_منزل_اشاره_ګر(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_پایله(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &اندازه) ){
    أرجع TCL_ERROR;
  }
  إذا( اندازه<=0 ){
    Tcl_Append_پایله(interp, "size must be positive", (حرف*)0);
    أرجع TCL_ERROR;
  }
  zBin = p;
  طالما( اندازه>0 ){
    إذا( اندازه>(حجم(zHex)-1)/2 ){
      n = (حجم(zHex)-1)/2;
    }وإلا{
      n = اندازه;
    }
    حافظه_نقل(zHex, zBin, n);
    zBin += n;
    اندازه -= n;
    sqlite3_Test_Bin_منزل_Hex(zHex, n);
    Tcl_Append_پایله(interp, zHex, (حرف*)0);
  }
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
ساكن صحيح SQLITE_TCLAPI test_حافظه_کارول_شوی(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewWideIntObj(sqlite3_حافظه_کارول_شوی()));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
ساكن صحيح SQLITE_TCLAPI test_حافظه_highwater(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح resetFlag = 0;
  إذا( objc!=1 && objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "?RESET?");
    أرجع TCL_ERROR;
  }
  إذا( objc==2 ){
    إذا( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) أرجع TCL_ERROR;
  } 
  Tcl_سېټ_Obj_پایله(interp, 
     Tcl_NewWideIntObj(sqlite3_حافظه_highwater(resetFlag)));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
ساكن صحيح SQLITE_TCLAPI test_memdebug_backtrace(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح ژوروالی;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "DEPT");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &ژوروالی) ) أرجع TCL_ERROR;
#که_تعریف_شوی SQLITE_MEMDEBUG
  {
    خارجي فراغ sqlite3MemdebugBacktrace(صحيح);
    sqlite3MemdebugBacktrace(ژوروالی);
  }
#د_که_پای
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
ساكن صحيح SQLITE_TCLAPI test_memdebug_dump(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "FILENAME");
    أرجع TCL_ERROR;
  }
#إذا defined(SQLITE_MEMDEBUG) || defined(SQLITE_حافظه_اندازه) \
     || defined(SQLITE_POW2_حافظه_اندازه)
  {
    خارجي فراغ sqlite3MemdebugDump(ثابت حرف*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#د_که_پای
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
ساكن صحيح SQLITE_TCLAPI test_memdebug_malloc_شمېر(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nMalloc = -1;
  إذا( objc!=1 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }
#إذا defined(SQLITE_MEMDEBUG)
  {
    خارجي صحيح sqlite3_Memdebug_Malloc_شمېر();
    nMalloc = sqlite3_Memdebug_Malloc_شمېر();
  }
#د_که_پای
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(nMalloc));
  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI test_memdebug_fail(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح ii;
  صحيح iFail;
  صحيح nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  صحيح nBenign;
  صحيح nFail = 0;

  إذا( objc<2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "COUNTER ?OPTIONS?");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) أرجع TCL_ERROR;

  لكل(ii=2; ii<objc; ii+=2){
    Tcl_اندازه nOption;
    حرف *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    حرف *zErr = 0;

    إذا( nOption>1 && مزي_پرتله(zOption, "-repeat", nOption)==0 ){
      إذا( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }وإلا{
        إذا( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          أرجع TCL_ERROR;
        }
      }
    }وإلا إذا( nOption>1 && مزي_پرتله(zOption, "-benigncnt", nOption)==0 ){
      إذا( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }وإلا{
        pBenignCnt = objv[ii+1];
      }
    }وإلا{
      zErr = "unknown option: ";
    }

    إذا( zErr ){
      Tcl_Append_پایله(interp, zErr, zOption, عدم);
      أرجع TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  إذا( pBenignCnt ){
    Tcl_Obj_سېټ_Var2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(nFail));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
ساكن صحيح SQLITE_TCLAPI test_memdebug_pending(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nPending;
  إذا( objc!=1 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(nPending));
  أرجع TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
ساكن صحيح sqlite3_memdebug_title_شمېر = 0;

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
ساكن صحيح SQLITE_TCLAPI test_memdebug_settitle(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_شمېر++;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "TITLE");
    أرجع TCL_ERROR;
  }
#که_تعریف_شوی SQLITE_MEMDEBUG
  {
    ثابت حرف *zTitle;
    خارجي صحيح sqlite3MemdebugSettitle(ثابت حرف*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#د_که_پای
  أرجع TCL_OK;
}

#تعریف_کړه MALLOC_LOG_چوکاټونه  10 
#تعریف_کړه MALLOC_LOG_KEYINTS (                                              \
    10 * ((حجم(صحيح)>=حجم(فراغ*)) ? 1 : حجم(فراغ*)/حجم(صحيح))   \
)
ساكن Tcl_Hash_جدول aMallocLog;
ساكن صحيح mallocLogEnabled = 0;

تعريفنوع بنية MallocLog MallocLog;
بنية MallocLog {
  صحيح nCall;
  صحيح nByte;
};

#که_تعریف_شوی SQLITE_MEMDEBUG
ساكن فراغ test_memdebug_callback(صحيح nByte, صحيح n_چوکاټ, فراغ **a_چوکاټ){
  إذا( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_Hash_ثبت *p_ثبت;
    صحيح isNew;

    صحيح a_کلی[MALLOC_LOG_KEYINTS];
    بینښه صحيح n_کلی = حجم(صحيح)*MALLOC_LOG_KEYINTS;

    memset(a_کلی, 0, n_کلی);
    إذا( (حجم(فراغ*)*n_چوکاټ)<n_کلی ){
      n_کلی = n_چوکاټ*حجم(فراغ*);
    }
    حافظه_نقل(a_کلی, a_چوکاټ, n_کلی);

    p_ثبت = Tcl_Create_Hash_ثبت(&aMallocLog, (ثابت حرف *)a_کلی, &isNew);
    إذا( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(حجم(MallocLog));
      memset(pLog, 0, حجم(MallocLog));
      Tcl_سېټ_Hash_ارزښت(p_ثبت, (Client_معلومات)pLog);
    }وإلا{
      pLog = (MallocLog *)Tcl_Get_Hash_ارزښت(p_ثبت);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#د_که_پای /* SQLITE_MEMDEBUG */

ساكن فراغ test_memdebug_log_clear(فراغ){
  Tcl_Hash_پلټنه پلټنه;
  Tcl_Hash_ثبت *p_ثبت;
  لكل(
    p_ثبت=Tcl_لومړی_Hash_ثبت(&aMallocLog, &پلټنه);
    p_ثبت;
    p_ثبت=Tcl_بل_Hash_ثبت(&پلټنه)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ارزښت(p_ثبت);
    Tcl_Free((حرف *)pLog);
  }
  Tcl_Delete_Hash_جدول(&aMallocLog);
  Tcl_Init_Hash_جدول(&aMallocLog, MALLOC_LOG_KEYINTS);
}

ساكن صحيح SQLITE_TCLAPI test_memdebug_log(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ساكن صحيح isInit = 0;
  صحيح iSub;

  ساكن ثابت حرف *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  تعداد MB_enum { 
      MB_LOG_پیل, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  إذا( !isInit ){
#که_تعریف_شوی SQLITE_MEMDEBUG
    خارجي فراغ sqlite3MemdebugBacktraceCallback(
        فراغ (*xBacktrace)(صحيح, صحيح, فراغ **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#د_که_پای
    Tcl_Init_Hash_جدول(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  إذا( objc<2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "SUB-COMMAND ...");
  }
  إذا( Tcl_Get_شاخص_From_Obj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    أرجع TCL_ERROR;
  }

  بدل( (تعداد MB_enum)iSub ){
    حالة MB_LOG_پیل:
      mallocLogEnabled = 1;
      اكسر;
    حالة MB_LOG_STOP:
      mallocLogEnabled = 0;
      اكسر;
    حالة MB_LOG_DUMP: {
      Tcl_Hash_پلټنه پلټنه;
      Tcl_Hash_ثبت *p_ثبت;
      Tcl_Obj *pRet = Tcl_NewObj();

      أكد(حجم(Tcl_WideInt)>=حجم(فراغ*));

      لكل(
        p_ثبت=Tcl_لومړی_Hash_ثبت(&aMallocLog, &پلټنه);
        p_ثبت;
        p_ثبت=Tcl_بل_Hash_ثبت(&پلټنه)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_چوکاټونه+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_ارزښت(p_ثبت);
        Tcl_WideInt *a_کلی = (Tcl_WideInt *)Tcl_Get_Hash_کلی(&aMallocLog, p_ثبت);
        صحيح ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        لكل(ii=0; ii<MALLOC_LOG_چوکاټونه; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_کلی[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_چوکاټونه+2, apElem)
        );
      }

      Tcl_سېټ_Obj_پایله(interp, pRet);
      اكسر;
    }
    حالة MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      اكسر;
    }

    حالة MB_LOG_SYNC: {
#که_تعریف_شوی SQLITE_MEMDEBUG
      خارجي فراغ sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#د_که_پای
      اكسر;
    }
  }

  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI test_config_pagecache(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح sz, N;
  Tcl_Obj *pRes;
  ساكن حرف *buf = 0;
  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "SIZE N");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &sz) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[2], &N) ) أرجع TCL_ERROR;
  حرر(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.sz_پاڼه));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.n_پاڼه));
  Tcl_سېټ_Obj_پایله(interp, pRes);

  إذا( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (فراغ*)0, 0, 0);
  }وإلا{
    buf = حافظه_ورکړه( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  أرجع TCL_OK;
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
ساكن صحيح SQLITE_TCLAPI test_alt_pcache(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح installFlag;
  صحيح discardChance = 0;
  صحيح prng_تخم_2 = 0;
  صحيح لوړ_Stress = 0;
  خارجي فراغ installTestPCache(صحيح,بینښه,بینښه,بینښه);
  إذا( objc<2 || objc>5 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) أرجع TCL_ERROR;
  إذا( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     أرجع TCL_ERROR;
  }
  إذا( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prng_تخم_2) ){
     أرجع TCL_ERROR;
  }
  إذا( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &لوړ_Stress) ){
    أرجع TCL_ERROR;
  }
  إذا( discardChance<0 || discardChance>100 ){
    Tcl_Append_پایله(interp, "discard-chance should be between 0 and 100",
                     (حرف*)0);
    أرجع TCL_ERROR;
  }
  installTestPCache(installFlag, (بینښه)discardChance, (بینښه)prng_تخم_2,
                    (بینښه)لوړ_Stress);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
ساكن صحيح SQLITE_TCLAPI test_config_memstatus(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح enable, rc;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "BOOLEAN");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) أرجع TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
ساكن صحيح SQLITE_TCLAPI test_config_lookaside(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح sz, cnt;
  Tcl_Obj *pRet;
  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "SIZE COUNT");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &sz) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) أرجع TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_سېټ_Obj_پایله(interp, pRet);
  أرجع TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
ساكن صحيح SQLITE_TCLAPI test_db_config_lookaside(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح sz, cnt;
  sqlite3 *db;
  صحيح bufid;
  ساكن حرف azBuf[2][10000];
  خارجي صحيح get_Db_اشاره_ګر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  إذا( objc!=5 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "BUFID SIZE COUNT");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[3], &sz) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) أرجع TCL_ERROR;
  إذا( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (فراغ*)0, sz, cnt);
  }وإلا إذا( bufid>=1 && bufid<=2 && sz*cnt<=حجم(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }وإلا{
    Tcl_Append_پایله(interp, "illegal arguments - see documentation", (حرف*)0);
    أرجع TCL_ERROR;
  }
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
ساكن صحيح SQLITE_TCLAPI test_config_ډېرۍ(
  فراغ * client_معلومات, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ساكن حرف *zBuf; /* Use this memory */
  صحيح nByte;         /* Size of buffer to pass to sqlite3_config() */
  صحيح nMinAlloc;     /* Size of minimum allocation */
  صحيح rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  صحيح nArg = objc-1;

  إذا( nArg!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "NBYTE NMINALLOC");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) أرجع TCL_ERROR;

  إذا( nByte==0 ){
    حرر( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_ډېرۍ, (فراغ*)0, 0, 0);
  }وإلا{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_ډېرۍ, zBuf, nByte, nMinAlloc);
  }

  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
ساكن صحيح SQLITE_TCLAPI test_config_ډېرۍ_اندازه(
  فراغ * client_معلومات, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nByte;         /* Size to pass to sqlite3_config() */
  صحيح rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  صحيح nArg = objc-1;

  إذا( nArg!=1 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "NBYTE");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) أرجع TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
ساكن صحيح SQLITE_TCLAPI test_config_error(
  فراغ * client_معلومات, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  خارجي صحيح get_Db_اشاره_ګر(Tcl_Interp*, ثابت حرف*, sqlite3**);

  إذا( objc!=2 && objc!=1 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "[DB]");
    أرجع TCL_ERROR;
  }
  إذا( objc==2 ){
    إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
    إذا( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_Append_پایله(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (حرف*)0);
      أرجع TCL_ERROR;
    }
  }وإلا{
    إذا( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_Append_پایله(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (حرف*)0);
      أرجع TCL_ERROR;
    }
  }
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
ساكن صحيح SQLITE_TCLAPI test_config_uri(
  فراغ * client_معلومات, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح bOpenUri;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "BOOL");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    أرجع TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
ساكن صحيح SQLITE_TCLAPI test_config_cis(
  فراغ * client_معلومات, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح bUseCis;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "BOOL");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    أرجع TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_شاخص_لټون, bUseCis);
  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
ساكن صحيح SQLITE_TCLAPI test_config_pmasz(
  فراغ * client_معلومات, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح iPmaSz;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "BOOL");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    أرجع TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  أرجع TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
ساكن صحيح SQLITE_TCLAPI test_dump_memsys3(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "FILENAME");
    أرجع TCL_ERROR;
  }

  بدل( SQLITE_PTR_منزل_INT(client_معلومات) ){
    حالة 3: {
#که_تعریف_شوی SQLITE_ENABLE_MEMSYS3
      خارجي فراغ sqlite3Memsys3Dump(ثابت حرف*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      اكسر;
#د_که_پای
    }
    حالة 5: {
#که_تعریف_شوی SQLITE_ENABLE_MEMSYS5
      خارجي فراغ sqlite3Memsys5Dump(ثابت حرف*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      اكسر;
#د_که_پای
    }
  }
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
ساكن صحيح SQLITE_TCLAPI test_حالت(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc, i_ارزښت, mx_ارزښت;
  صحيح i, op = 0, resetFlag;
  ثابت حرف *zOpName;
  ساكن ثابت بنية {
    ثابت حرف *zName;
    صحيح op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_حالت_حافظه_کارول_شوی         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_حالت_MALLOC_اندازه         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_حالت_PAGECACHE_کارول_شوی      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_حالت_PAGECACHE_ډکېدنه  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_حالت_PAGECACHE_اندازه      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_حالت_SCRATCH_کارول_شوی        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_حالت_SCRATCH_ډکېدنه    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_حالت_SCRATCH_اندازه        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_حالت_PARSER_ډېری        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_حالت_MALLOC_شمېر        },
  };
  Tcl_Obj *p_پایله;
  إذا( objc!=3 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "PARAMETER RESETFLAG");
    أرجع TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  لكل(i=0; i<Array_اندازه(aOp); i++){
    إذا( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      اكسر;
    }
  }
  إذا( i>=Array_اندازه(aOp) ){
    إذا( Tcl_GetIntFromObj(interp, objv[1], &op) ) أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) أرجع TCL_ERROR;
  i_ارزښت = 0;
  mx_ارزښت = 0;
  rc = sqlite3_حالت(op, &i_ارزښت, &mx_ارزښت, resetFlag);
  p_پایله = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_پایله, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_پایله, Tcl_NewIntObj(i_ارزښت));
  Tcl_ListObjAppendElement(0, p_پایله, Tcl_NewIntObj(mx_ارزښت));
  Tcl_سېټ_Obj_پایله(interp, p_پایله);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
ساكن صحيح SQLITE_TCLAPI test_db_حالت(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc, i_ارزښت, mx_ارزښت;
  صحيح i, op = 0, resetFlag;
  ثابت حرف *zOpName;
  sqlite3 *db;
  خارجي صحيح get_Db_اشاره_ګر(Tcl_Interp*, ثابت حرف*, sqlite3**);
  ساكن ثابت بنية {
    ثابت حرف *zName;
    صحيح op;
  } aOp[] = {
    { "LOOKASIDE_USED",      SQLITE_DBSTATUS_LOOKASIDE_کارول_شوی      },
    { "CACHE_USED",          SQLITE_DBSTATUS_CACHE_کارول_شوی          },
    { "SCHEMA_USED",         SQLITE_DBSTATUS_SCHEMA_کارول_شوی         },
    { "STMT_USED",           SQLITE_DBSTATUS_STMT_کارول_شوی           },
    { "LOOKASIDE_HIT",       SQLITE_DBSTATUS_LOOKASIDE_موندنه       },
    { "LOOKASIDE_MISS_SIZE", SQLITE_DBSTATUS_LOOKASIDE_نه_موندنه_اندازه },
    { "LOOKASIDE_MISS_FULL", SQLITE_DBSTATUS_LOOKASIDE_نه_موندنه_ډک },
    { "CACHE_HIT",           SQLITE_DBSTATUS_CACHE_موندنه           },
    { "CACHE_MISS",          SQLITE_DBSTATUS_CACHE_نه_موندنه          },
    { "CACHE_WRITE",         SQLITE_DBSTATUS_CACHE_WRITE         },
    { "DEFERRED_FKS",        SQLITE_DBSTATUS_DEFERRED_FKS        },
    { "CACHE_USED_SHARED",   SQLITE_DBSTATUS_CACHE_کارول_شوی_SHARED   },
    { "CACHE_SPILL",         SQLITE_DBSTATUS_CACHE_SPILL         },
    { "TEMPBUF_SPILL",       SQLITE_DBSTATUS_TEMPBUF_SPILL       },
  };
  Tcl_Obj *p_پایله;
  إذا( objc!=4 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "DB PARAMETER RESETFLAG");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  إذا( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  إذا( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  لكل(i=0; i<Array_اندازه(aOp); i++){
    إذا( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      اكسر;
    }
  }
  إذا( i>=Array_اندازه(aOp) ){
    إذا( Tcl_GetIntFromObj(interp, objv[2], &op) ) أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) أرجع TCL_ERROR;
  i_ارزښت = 0;
  mx_ارزښت = 0;
  rc = sqlite3_db_حالت(db, op, &i_ارزښت, &mx_ارزښت, resetFlag);
  p_پایله = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_پایله, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_پایله, Tcl_NewIntObj(i_ارزښت));
  Tcl_ListObjAppendElement(0, p_پایله, Tcl_NewIntObj(mx_ارزښت));
  Tcl_سېټ_Obj_پایله(interp, p_پایله);
  أرجع TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
ساكن صحيح SQLITE_TCLAPI test_install_malloc_faultsim(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح isInstall;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "BOOLEAN");
    أرجع TCL_ERROR;
  }
  إذا( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    أرجع TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
ساكن صحيح SQLITE_TCLAPI test_install_memsys3(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = SQLITE_MISUSE;
#که_تعریف_شوی SQLITE_ENABLE_MEMSYS3
  ثابت sqlite3_mem_methods *sqlite3MemGetMemsys3(فراغ);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#د_که_پای
  Tcl_سېټ_پایله(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_vfs_oom_test(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  خارجي صحيح sqlite3_memdebug_vfs_oom_test;
  إذا( objc>2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "?INTEGER?");
    أرجع TCL_ERROR;
  }وإلا إذا( objc==2 ){
    صحيح iNew;
    إذا( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) أرجع TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  أرجع TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest_malloc_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     صحيح client_معلومات;
  } aObjCmd[] = {
     { "sqlite3_malloc",             test_malloc                   ,0 },
     { "sqlite3_realloc",            test_realloc                  ,0 },
     { "sqlite3_free",               test_free                     ,0 },
     { "memset",                     test_memset                   ,0 },
     { "memget",                     test_memget                   ,0 },
     { "sqlite3_memory_used",        test_حافظه_کارول_شوی              ,0 },
     { "sqlite3_memory_highwater",   test_حافظه_highwater         ,0 },
     { "sqlite3_memdebug_backtrace", test_memdebug_backtrace       ,0 },
     { "sqlite3_memdebug_dump",      test_memdebug_dump            ,0 },
     { "sqlite3_memdebug_fail",      test_memdebug_fail            ,0 },
     { "sqlite3_memdebug_pending",   test_memdebug_pending         ,0 },
     { "sqlite3_memdebug_settitle",  test_memdebug_settitle        ,0 },
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_شمېر ,0 },
     { "sqlite3_memdebug_log",       test_memdebug_log             ,0 },
     { "sqlite3_config_pagecache",   test_config_pagecache         ,0 },
     { "sqlite3_config_alt_pcache",  test_alt_pcache               ,0 },
     { "sqlite3_status",             test_حالت                   ,0 },
     { "sqlite3_db_status",          test_db_حالت                ,0 },
     { "install_malloc_faultsim",    test_install_malloc_faultsim  ,0 },
     { "sqlite3_config_heap",        test_config_ډېرۍ              ,0 },
     { "sqlite3_config_heap_size",   test_config_ډېرۍ_اندازه         ,0 },
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
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Client_معلومات c = (Client_معلومات)SQLITE_INT_منزل_PTR(aObjCmd[i].client_معلومات);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  أرجع TCL_OK;
}
#د_که_پای
