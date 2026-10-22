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
  صحيح isBenignMode;       /* True if malloc failures are considered benign */
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
ساكن فراغ sqlite3FirstFault(فراغ){
  ساكن u64 cnt2 = 0;
  cnt2++;
  إذا( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
ساكن صحيح faultsimStep(فراغ){
  إذا( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    أرجع 0;
  }
  إذا( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    أرجع 0;
  }
  إذا( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  إذا( memfault.isBenignMode>0 ){
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
  إذا( !faultsimStep() ){
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
  إذا( !faultsimStep() ){
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
  memfault.isBenignMode = 0;
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
  memfault.isBenignMode++;
}
ساكن فراغ faultsimEndBenign(فراغ){
  memfault.isBenignMode--;
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
        faultsimBeginBenign, faultsimEndBenign
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

#ifdef SQLITE_TEST

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
ساكن فراغ pointerToText(فراغ *p, حرف *z){
  ثابت حرف *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
ساكن صحيح textToPointer(ثابت حرف *z, فراغ **pp){
  *pp = sqlite3TestTextToPtr(z);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
ساكن صحيح SQLITE_TCLAPI test_malloc(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nByte;
  فراغ *p;
  حرف zOut[100];
  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "NBYTES");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) أرجع TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, عدم);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
ساكن صحيح SQLITE_TCLAPI test_realloc(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nByte;
  فراغ *pPrior, *p;
  حرف zOut[100];
  إذا( objc!=3 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "PRIOR NBYTES");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) أرجع TCL_ERROR;
  إذا( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    أرجع TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, عدم);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
ساكن صحيح SQLITE_TCLAPI test_free(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ *pPrior;
  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "PRIOR");
    أرجع TCL_ERROR;
  }
  إذا( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    أرجع TCL_ERROR;
  }
  sqlite3_free(pPrior);
  أرجع TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
صحيح sqlite3TestHexToBin(ثابت حرف *, صحيح, حرف *);
صحيح sqlite3TestBinToHex(حرف*,صحيح);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
ساكن صحيح SQLITE_TCLAPI test_memset(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ *p;
  صحيح size, i;
  Tcl_Size n;
  حرف *zHex;
  حرف *zOut;
  حرف zBin[100];

  إذا( objc!=4 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "ADDRESS SIZE HEX");
    أرجع TCL_ERROR;
  }
  إذا( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    أرجع TCL_ERROR;
  }
  إذا( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (حرف*)0);
    أرجع TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  إذا( n>حجم(zBin)*2 ) n = حجم(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (صحيح)n, zBin);
  إذا( n==0 ){
    Tcl_AppendResult(interp, "no data", (حرف*)0);
    أرجع TCL_ERROR;
  }
  zOut = p;
  لكل(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  أرجع TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
ساكن صحيح SQLITE_TCLAPI test_memget(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  فراغ *p;
  صحيح size, n;
  حرف *zBin;
  حرف zHex[100];

  إذا( objc!=3 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "ADDRESS SIZE");
    أرجع TCL_ERROR;
  }
  إذا( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (حرف*)0);
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    أرجع TCL_ERROR;
  }
  إذا( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (حرف*)0);
    أرجع TCL_ERROR;
  }
  zBin = p;
  طالما( size>0 ){
    إذا( size>(حجم(zHex)-1)/2 ){
      n = (حجم(zHex)-1)/2;
    }وإلا{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (حرف*)0);
  }
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
ساكن صحيح SQLITE_TCLAPI test_memory_used(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
ساكن صحيح SQLITE_TCLAPI test_memory_highwater(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح resetFlag = 0;
  إذا( objc!=1 && objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "?RESET?");
    أرجع TCL_ERROR;
  }
  إذا( objc==2 ){
    إذا( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) أرجع TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
ساكن صحيح SQLITE_TCLAPI test_memdebug_backtrace(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح depth;
  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "DEPT");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &depth) ) أرجع TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    خارجي فراغ sqlite3MemdebugBacktrace(صحيح);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
ساكن صحيح SQLITE_TCLAPI test_memdebug_dump(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "FILENAME");
    أرجع TCL_ERROR;
  }
#إذا defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    خارجي فراغ sqlite3MemdebugDump(ثابت حرف*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
ساكن صحيح SQLITE_TCLAPI test_memdebug_malloc_كدي(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nMalloc = -1;
  إذا( objc!=1 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }
#إذا defined(SQLITE_MEMDEBUG)
  {
    خارجي صحيح sqlite3_Memdebug_Malloc_كدي();
    nMalloc = sqlite3_Memdebug_Malloc_كدي();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
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
  فراغ * clientData,
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
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "COUNTER ?OPTIONS?");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) أرجع TCL_ERROR;

  لكل(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    حرف *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    حرف *zErr = 0;

    إذا( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      إذا( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }وإلا{
        إذا( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          أرجع TCL_ERROR;
        }
      }
    }وإلا إذا( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      إذا( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }وإلا{
        pBenignCnt = objv[ii+1];
      }
    }وإلا{
      zErr = "unknown option: ";
    }

    إذا( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, عدم);
      أرجع TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  إذا( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
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
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nPending;
  إذا( objc!=1 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "");
    أرجع TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  أرجع TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
ساكن صحيح sqlite3_memdebug_title_كدي = 0;

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
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_كدي++;
  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "TITLE");
    أرجع TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    ثابت حرف *zTitle;
    خارجي صحيح sqlite3MemdebugSettitle(ثابت حرف*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  أرجع TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((حجم(صحيح)>=حجم(فراغ*)) ? 1 : حجم(فراغ*)/حجم(صحيح))   \
)
ساكن Tcl_HashTable aMallocLog;
ساكن صحيح mallocLogEnabled = 0;

تعريفنوع بنية MallocLog MallocLog;
بنية MallocLog {
  صحيح nCall;
  صحيح nByte;
};

#ifdef SQLITE_MEMDEBUG
ساكن فراغ test_memdebug_callback(صحيح nByte, صحيح nFrame, فراغ **aFrame){
  إذا( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    صحيح isNew;

    صحيح a_مبودي[MALLOC_LOG_KEYINTS];
    unsigned صحيح n_مبودي = حجم(صحيح)*MALLOC_LOG_KEYINTS;

    memset(a_مبودي, 0, n_مبودي);
    إذا( (حجم(فراغ*)*nFrame)<n_مبودي ){
      n_مبودي = nFrame*حجم(فراغ*);
    }
    memcpy(a_مبودي, aFrame, n_مبودي);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (ثابت حرف *)a_مبودي, &isNew);
    إذا( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(حجم(MallocLog));
      memset(pLog, 0, حجم(MallocLog));
      Tcl_Set_Hash_درج(pEntry, (ClientData)pLog);
    }وإلا{
      pLog = (MallocLog *)Tcl_Get_Hash_درج(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

ساكن فراغ test_memdebug_log_clear(فراغ){
  Tcl_Hash_نيما نيما;
  Tcl_HashEntry *pEntry;
  لكل(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &نيما);
    pEntry;
    pEntry=Tcl_NextHashEntry(&نيما)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_درج(pEntry);
    Tcl_Free((حرف *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

ساكن صحيح SQLITE_TCLAPI test_memdebug_log(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ساكن صحيح isInit = 0;
  صحيح iSub;

  ساكن ثابت حرف *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  تعداد MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  إذا( !isInit ){
#ifdef SQLITE_MEMDEBUG
    خارجي فراغ sqlite3MemdebugBacktraceCallback(
        فراغ (*xBacktrace)(صحيح, صحيح, فراغ **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  إذا( objc<2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "SUB-COMMAND ...");
  }
  إذا( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    أرجع TCL_ERROR;
  }

  بدل( (تعداد MB_enum)iSub ){
    حالة MB_LOG_START:
      mallocLogEnabled = 1;
      اكسر;
    حالة MB_LOG_STOP:
      mallocLogEnabled = 0;
      اكسر;
    حالة MB_LOG_DUMP: {
      Tcl_Hash_نيما نيما;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      أكد(حجم(Tcl_WideInt)>=حجم(فراغ*));

      لكل(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &نيما);
        pEntry;
        pEntry=Tcl_NextHashEntry(&نيما)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_درج(pEntry);
        Tcl_WideInt *a_مبودي = (Tcl_WideInt *)Tcl_Get_Hash_مبودي(&aMallocLog, pEntry);
        صحيح ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        لكل(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_مبودي[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      اكسر;
    }
    حالة MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      اكسر;
    }

    حالة MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      خارجي فراغ sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
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
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح sz, N;
  Tcl_Obj *pRes;
  ساكن حرف *buf = 0;
  إذا( objc!=3 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "SIZE N");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &sz) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[2], &N) ) أرجع TCL_ERROR;
  حرر(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  إذا( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (فراغ*)0, 0, 0);
  }وإلا{
    buf = malloc( sz*N );
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
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح installFlag;
  صحيح discardChance = 0;
  صحيح prngSeed = 0;
  صحيح سم_Stress = 0;
  خارجي فراغ installTestPCache(صحيح,unsigned,unsigned,unsigned);
  إذا( objc<2 || objc>5 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) أرجع TCL_ERROR;
  إذا( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     أرجع TCL_ERROR;
  }
  إذا( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     أرجع TCL_ERROR;
  }
  إذا( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &سم_Stress) ){
    أرجع TCL_ERROR;
  }
  إذا( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (حرف*)0);
    أرجع TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)سم_Stress);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
ساكن صحيح SQLITE_TCLAPI test_config_memstatus(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح enable, rc;
  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "BOOLEAN");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) أرجع TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
ساكن صحيح SQLITE_TCLAPI test_config_lookaside(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح sz, cnt;
  Tcl_Obj *pRet;
  إذا( objc!=3 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "SIZE COUNT");
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
  Tcl_SetObjResult(interp, pRet);
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
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح sz, cnt;
  sqlite3 *db;
  صحيح bufid;
  ساكن حرف azBuf[2][10000];
  خارجي صحيح getDbPointer(Tcl_Interp*, ثابت حرف*, sqlite3**);
  إذا( objc!=5 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "BUFID SIZE COUNT");
    أرجع TCL_ERROR;
  }
  إذا( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[3], &sz) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) أرجع TCL_ERROR;
  إذا( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (فراغ*)0, sz, cnt);
  }وإلا إذا( bufid>=1 && bufid<=2 && sz*cnt<=حجم(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }وإلا{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (حرف*)0);
    أرجع TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
ساكن صحيح SQLITE_TCLAPI test_config_heap(
  فراغ * clientData, 
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
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "NBYTE NMINALLOC");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) أرجع TCL_ERROR;
  إذا( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) أرجع TCL_ERROR;

  إذا( nByte==0 ){
    حرر( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (فراغ*)0, 0, 0);
  }وإلا{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
ساكن صحيح SQLITE_TCLAPI test_config_heap_size(
  فراغ * clientData, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح nByte;         /* Size to pass to sqlite3_config() */
  صحيح rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  صحيح nArg = objc-1;

  إذا( nArg!=1 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "NBYTE");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) أرجع TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
ساكن صحيح SQLITE_TCLAPI test_config_error(
  فراغ * clientData, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  خارجي صحيح getDbPointer(Tcl_Interp*, ثابت حرف*, sqlite3**);

  إذا( objc!=2 && objc!=1 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "[DB]");
    أرجع TCL_ERROR;
  }
  إذا( objc==2 ){
    إذا( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
    إذا( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (حرف*)0);
      أرجع TCL_ERROR;
    }
  }وإلا{
    إذا( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
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
  فراغ * clientData, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح bOpenUri;

  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "BOOL");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    أرجع TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
ساكن صحيح SQLITE_TCLAPI test_config_cis(
  فراغ * clientData, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح bUseCis;

  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "BOOL");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    أرجع TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
ساكن صحيح SQLITE_TCLAPI test_config_pmasz(
  فراغ * clientData, 
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح iPmaSz;

  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "BOOL");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    أرجع TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);

  أرجع TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
ساكن صحيح SQLITE_TCLAPI test_dump_memsys3(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "FILENAME");
    أرجع TCL_ERROR;
  }

  بدل( SQLITE_PTR_TO_INT(clientData) ){
    حالة 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      خارجي فراغ sqlite3Memsys3Dump(ثابت حرف*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      اكسر;
#endif
    }
    حالة 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      خارجي فراغ sqlite3Memsys5Dump(ثابت حرف*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      اكسر;
#endif
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
ساكن صحيح SQLITE_TCLAPI test_status(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc, i_درج, mx_درج;
  صحيح i, op = 0, resetFlag;
  ثابت حرف *zOpName;
  ساكن ثابت بنية {
    ثابت حرف *zName;
    صحيح op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_تر        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_كدي        },
  };
  Tcl_Obj *pResult;
  إذا( objc!=3 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "PARAMETER RESETFLAG");
    أرجع TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  لكل(i=0; i<ArraySize(aOp); i++){
    إذا( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      اكسر;
    }
  }
  إذا( i>=ArraySize(aOp) ){
    إذا( Tcl_GetIntFromObj(interp, objv[1], &op) ) أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) أرجع TCL_ERROR;
  i_درج = 0;
  mx_درج = 0;
  rc = sqlite3_status(op, &i_درج, &mx_درج, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_درج));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_درج));
  Tcl_SetObjResult(interp, pResult);
  أرجع TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
ساكن صحيح SQLITE_TCLAPI test_db_status(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc, i_درج, mx_درج;
  صحيح i, op = 0, resetFlag;
  ثابت حرف *zOpName;
  sqlite3 *db;
  خارجي صحيح getDbPointer(Tcl_Interp*, ثابت حرف*, sqlite3**);
  ساكن ثابت بنية {
    ثابت حرف *zName;
    صحيح op;
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
  إذا( objc!=4 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "DB PARAMETER RESETFLAG");
    أرجع TCL_ERROR;
  }
  إذا( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  إذا( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  إذا( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  لكل(i=0; i<ArraySize(aOp); i++){
    إذا( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      اكسر;
    }
  }
  إذا( i>=ArraySize(aOp) ){
    إذا( Tcl_GetIntFromObj(interp, objv[2], &op) ) أرجع TCL_ERROR;
  }
  إذا( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) أرجع TCL_ERROR;
  i_درج = 0;
  mx_درج = 0;
  rc = sqlite3_db_status(db, op, &i_درج, &mx_درج, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_درج));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_درج));
  Tcl_SetObjResult(interp, pResult);
  أرجع TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
ساكن صحيح SQLITE_TCLAPI test_install_malloc_faultsim(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc;
  صحيح isInstall;

  إذا( objc!=2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "BOOLEAN");
    أرجع TCL_ERROR;
  }
  إذا( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    أرجع TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
ساكن صحيح SQLITE_TCLAPI test_install_memsys3(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  ثابت sqlite3_mem_methods *sqlite3MemGetMemsys3(فراغ);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_VOLATILE);
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_vfs_oom_test(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  خارجي صحيح sqlite3_memdebug_vfs_oom_test;
  إذا( objc>2 ){
    Tcl_Wrong_لمب_حجوجي(interp, 1, objv, "?INTEGER?");
    أرجع TCL_ERROR;
  }وإلا إذا( objc==2 ){
    صحيح iNew;
    إذا( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) أرجع TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  أرجع TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetest_malloc_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     صحيح clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_كدي ,0 },
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
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  أرجع TCL_OK;
}
#endif
