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
ایستا ساختار MemFault {
  صحیح iCountdown;         /* Number of pending successes before a failure */
  صحیح nRepeat;            /* Number of times to repeat the failure */
  صحیح nBenign;            /* Number of benign failures seen since last config */
  صحیح nFail;              /* Number of failures seen since last config */
  صحیح nOkBefore;          /* Successful allocations prior to the first fault */
  صحیح nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  صحیح isInstalled;        /* True if the fault simulation layer is installed */
  صحیح isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
ایستا تهی sqlite3Fault(تهی){
  ایستا u64 cnt = 0;
  cnt++;
  اگر( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
ایستا تهی sqlite3FirstFault(تهی){
  ایستا u64 cnt2 = 0;
  cnt2++;
  اگر( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
ایستا صحیح faultsimStep(تهی){
  اگر( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    بازگشت 0;
  }
  اگر( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    بازگشت 0;
  }
  اگر( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  اگر( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  اگر( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  بازگشت 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
ایستا تهی *faultsimMalloc(صحیح n){
  تهی *p = 0;
  اگر( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  بازگشت p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
ایستا تهی *faultsimRealloc(تهی *pOld, صحیح n){
  تهی *p = 0;
  اگر( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  بازگشت p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
ایستا تهی faultsimConfig(صحیح nDelay, صحیح nRepeat){
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
ایستا صحیح faultsimFailures(تهی){
  بازگشت memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
ایستا صحیح faultsimBenignFailures(تهی){
  بازگشت memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
ایستا صحیح faultsimPending(تهی){
  اگر( memfault.enable ){
    بازگشت memfault.iCountdown;
  }وگرنه{
    بازگشت -1;
  }
}


ایستا تهی faultsimBeginBenign(تهی){
  memfault.isBenignMode++;
}
ایستا تهی faultsimEndBenign(تهی){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
ایستا صحیح faultsimInstall(صحیح install){
  صحیح rc;

  install = (install ? 1 : 0);
  أكد(memfault.isInstalled==1 || memfault.isInstalled==0);

  اگر( install==memfault.isInstalled ){
    بازگشت SQLITE_ERROR;
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
        faultsimBeginBenign, faultsimEndBenign
    );
  }وگرنه{
    sqlite3_mem_methods m2;
    أكد(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, اندازه(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    أكد( memcmp(&m2, &memfault.m, اندازه(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (تهی*)0, (تهی*)0);
  }

  اگر( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  بازگشت rc;
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
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

/*
** Transform pointers to text and back again
*/
ایستا تهی pointerToText(تهی *p, نویسه *z){
  ثابت نویسه *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
ایستا صحیح textToPointer(ثابت نویسه *z, تهی **pp){
  *pp = sqlite3TestTextToPtr(z);
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
ایستا صحیح SQLITE_TCLAPI test_malloc(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nByte;
  تهی *p;
  نویسه zOut[100];
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "NBYTES");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) بازگشت TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, پوچ);
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
ایستا صحیح SQLITE_TCLAPI test_realloc(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nByte;
  تهی *pPrior, *p;
  نویسه zOut[100];
  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "PRIOR NBYTES");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) بازگشت TCL_ERROR;
  اگر( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, پوچ);
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
ایستا صحیح SQLITE_TCLAPI test_free(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  تهی *pPrior;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "PRIOR");
    بازگشت TCL_ERROR;
  }
  اگر( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  sqlite3_free(pPrior);
  بازگشت TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
صحیح sqlite3TestHexToBin(ثابت نویسه *, صحیح, نویسه *);
صحیح sqlite3TestBinToHex(نویسه*,صحیح);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
ایستا صحیح SQLITE_TCLAPI test_memset(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  تهی *p;
  صحیح size, i;
  Tcl_Size n;
  نویسه *zHex;
  نویسه *zOut;
  نویسه zBin[100];

  اگر( objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    بازگشت TCL_ERROR;
  }
  اگر( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    بازگشت TCL_ERROR;
  }
  اگر( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  اگر( n>اندازه(zBin)*2 ) n = اندازه(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (صحیح)n, zBin);
  اگر( n==0 ){
    Tcl_AppendResult(interp, "no data", (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  zOut = p;
  برای(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  بازگشت TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
ایستا صحیح SQLITE_TCLAPI test_memget(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  تهی *p;
  صحیح size, n;
  نویسه *zBin;
  نویسه zHex[100];

  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "ADDRESS SIZE");
    بازگشت TCL_ERROR;
  }
  اگر( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    بازگشت TCL_ERROR;
  }
  اگر( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  zBin = p;
  وقتی( size>0 ){
    اگر( size>(اندازه(zHex)-1)/2 ){
      n = (اندازه(zHex)-1)/2;
    }وگرنه{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (نویسه*)0);
  }
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
ایستا صحیح SQLITE_TCLAPI test_memory_used(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
ایستا صحیح SQLITE_TCLAPI test_memory_highwater(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح resetFlag = 0;
  اگر( objc!=1 && objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "?RESET?");
    بازگشت TCL_ERROR;
  }
  اگر( objc==2 ){
    اگر( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) بازگشت TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
ایستا صحیح SQLITE_TCLAPI test_memdebug_backtrace(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح depth;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DEPT");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &depth) ) بازگشت TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    بیرونی تهی sqlite3MemdebugBacktrace(صحیح);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
ایستا صحیح SQLITE_TCLAPI test_memdebug_dump(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "FILENAME");
    بازگشت TCL_ERROR;
  }
#اگر defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    بیرونی تهی sqlite3MemdebugDump(ثابت نویسه*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
ایستا صحیح SQLITE_TCLAPI test_memdebug_malloc_شمار(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nMalloc = -1;
  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }
#اگر defined(SQLITE_MEMDEBUG)
  {
    بیرونی صحیح sqlite3_Memdebug_Malloc_شمار();
    nMalloc = sqlite3_Memdebug_Malloc_شمار();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI test_memdebug_fail(
  تهی * clientData,
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
    Tcl_Wrong_رقم_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) بازگشت TCL_ERROR;

  برای(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    نویسه *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    نویسه *zErr = 0;

    اگر( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      اگر( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }وگرنه{
        اگر( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          بازگشت TCL_ERROR;
        }
      }
    }وگرنه اگر( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      اگر( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }وگرنه{
        pBenignCnt = objv[ii+1];
      }
    }وگرنه{
      zErr = "unknown option: ";
    }

    اگر( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, پوچ);
      بازگشت TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  اگر( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
ایستا صحیح SQLITE_TCLAPI test_memdebug_pending(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nPending;
  اگر( objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "");
    بازگشت TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  بازگشت TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
ایستا صحیح sqlite3_memdebug_title_شمار = 0;

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
ایستا صحیح SQLITE_TCLAPI test_memdebug_settitle(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_شمار++;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "TITLE");
    بازگشت TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    ثابت نویسه *zTitle;
    بیرونی صحیح sqlite3MemdebugSettitle(ثابت نویسه*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  بازگشت TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((اندازه(صحیح)>=اندازه(تهی*)) ? 1 : اندازه(تهی*)/اندازه(صحیح))   \
)
ایستا Tcl_HashTable aMallocLog;
ایستا صحیح mallocLogEnabled = 0;

تعریفنوع ساختار MallocLog MallocLog;
ساختار MallocLog {
  صحیح nCall;
  صحیح nByte;
};

#ifdef SQLITE_MEMDEBUG
ایستا تهی test_memdebug_callback(صحیح nByte, صحیح nFrame, تهی **aFrame){
  اگر( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    صحیح isNew;

    صحیح a_کلید[MALLOC_LOG_KEYINTS];
    unsigned صحیح n_کلید = اندازه(صحیح)*MALLOC_LOG_KEYINTS;

    memset(a_کلید, 0, n_کلید);
    اگر( (اندازه(تهی*)*nFrame)<n_کلید ){
      n_کلید = nFrame*اندازه(تهی*);
    }
    memcpy(a_کلید, aFrame, n_کلید);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (ثابت نویسه *)a_کلید, &isNew);
    اگر( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(اندازه(MallocLog));
      memset(pLog, 0, اندازه(MallocLog));
      Tcl_Set_Hash_مقدار(pEntry, (ClientData)pLog);
    }وگرنه{
      pLog = (MallocLog *)Tcl_Get_Hash_مقدار(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

ایستا تهی test_memdebug_log_clear(تهی){
  Tcl_Hash_جستجو جستجو;
  Tcl_HashEntry *pEntry;
  برای(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &جستجو);
    pEntry;
    pEntry=Tcl_NextHashEntry(&جستجو)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_مقدار(pEntry);
    Tcl_Free((نویسه *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

ایستا صحیح SQLITE_TCLAPI test_memdebug_log(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ایستا صحیح isInit = 0;
  صحیح iSub;

  ایستا ثابت نویسه *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  شمارش MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  اگر( !isInit ){
#ifdef SQLITE_MEMDEBUG
    بیرونی تهی sqlite3MemdebugBacktraceCallback(
        تهی (*xBacktrace)(صحیح, صحیح, تهی **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  اگر( objc<2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  اگر( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    بازگشت TCL_ERROR;
  }

  انتخاب( (شمارش MB_enum)iSub ){
    حالت MB_LOG_START:
      mallocLogEnabled = 1;
      بشکن;
    حالت MB_LOG_STOP:
      mallocLogEnabled = 0;
      بشکن;
    حالت MB_LOG_DUMP: {
      Tcl_Hash_جستجو جستجو;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      أكد(اندازه(Tcl_WideInt)>=اندازه(تهی*));

      برای(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &جستجو);
        pEntry;
        pEntry=Tcl_NextHashEntry(&جستجو)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_مقدار(pEntry);
        Tcl_WideInt *a_کلید = (Tcl_WideInt *)Tcl_Get_Hash_کلید(&aMallocLog, pEntry);
        صحیح ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        برای(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_کلید[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      بشکن;
    }
    حالت MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      بشکن;
    }

    حالت MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      بیرونی تهی sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      بشکن;
    }
  }

  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI test_config_pagecache(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح sz, N;
  Tcl_Obj *pRes;
  ایستا نویسه *buf = 0;
  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SIZE N");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &sz) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[2], &N) ) بازگشت TCL_ERROR;
  آزاد(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  اگر( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (تهی*)0, 0, 0);
  }وگرنه{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  بازگشت TCL_OK;
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
ایستا صحیح SQLITE_TCLAPI test_alt_pcache(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح installFlag;
  صحیح discardChance = 0;
  صحیح prngSeed = 0;
  صحیح بالا_Stress = 0;
  بیرونی تهی installTestPCache(صحیح,unsigned,unsigned,unsigned);
  اگر( objc<2 || objc>5 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) بازگشت TCL_ERROR;
  اگر( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     بازگشت TCL_ERROR;
  }
  اگر( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     بازگشت TCL_ERROR;
  }
  اگر( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &بالا_Stress) ){
    بازگشت TCL_ERROR;
  }
  اگر( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)بالا_Stress);
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
ایستا صحیح SQLITE_TCLAPI test_config_memstatus(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح enable, rc;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "BOOLEAN");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) بازگشت TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
ایستا صحیح SQLITE_TCLAPI test_config_lookaside(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح sz, cnt;
  Tcl_Obj *pRet;
  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SIZE COUNT");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &sz) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) بازگشت TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  بازگشت TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
ایستا صحیح SQLITE_TCLAPI test_db_config_lookaside(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح sz, cnt;
  sqlite3 *db;
  صحیح bufid;
  ایستا نویسه azBuf[2][10000];
  بیرونی صحیح getDbPointer(Tcl_Interp*, ثابت نویسه*, sqlite3**);
  اگر( objc!=5 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "BUFID SIZE COUNT");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[3], &sz) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) بازگشت TCL_ERROR;
  اگر( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (تهی*)0, sz, cnt);
  }وگرنه اگر( bufid>=1 && bufid<=2 && sz*cnt<=اندازه(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }وگرنه{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (نویسه*)0);
    بازگشت TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
ایستا صحیح SQLITE_TCLAPI test_config_heap(
  تهی * clientData, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ایستا نویسه *zBuf; /* Use this memory */
  صحیح nByte;         /* Size of buffer to pass to sqlite3_config() */
  صحیح nMinAlloc;     /* Size of minimum allocation */
  صحیح rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  صحیح nArg = objc-1;

  اگر( nArg!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "NBYTE NMINALLOC");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) بازگشت TCL_ERROR;
  اگر( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) بازگشت TCL_ERROR;

  اگر( nByte==0 ){
    آزاد( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (تهی*)0, 0, 0);
  }وگرنه{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
ایستا صحیح SQLITE_TCLAPI test_config_heap_size(
  تهی * clientData, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح nByte;         /* Size to pass to sqlite3_config() */
  صحیح rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  صحیح nArg = objc-1;

  اگر( nArg!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "NBYTE");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) بازگشت TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
ایستا صحیح SQLITE_TCLAPI test_config_error(
  تهی * clientData, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  بیرونی صحیح getDbPointer(Tcl_Interp*, ثابت نویسه*, sqlite3**);

  اگر( objc!=2 && objc!=1 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "[DB]");
    بازگشت TCL_ERROR;
  }
  اگر( objc==2 ){
    اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
    اگر( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (نویسه*)0);
      بازگشت TCL_ERROR;
    }
  }وگرنه{
    اگر( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (نویسه*)0);
      بازگشت TCL_ERROR;
    }
  }
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
ایستا صحیح SQLITE_TCLAPI test_config_uri(
  تهی * clientData, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح bOpenUri;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "BOOL");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    بازگشت TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);

  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
ایستا صحیح SQLITE_TCLAPI test_config_cis(
  تهی * clientData, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح bUseCis;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "BOOL");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    بازگشت TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);

  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
ایستا صحیح SQLITE_TCLAPI test_config_pmasz(
  تهی * clientData, 
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح iPmaSz;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "BOOL");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    بازگشت TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);

  بازگشت TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
ایستا صحیح SQLITE_TCLAPI test_dump_memsys3(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "FILENAME");
    بازگشت TCL_ERROR;
  }

  انتخاب( SQLITE_PTR_TO_INT(clientData) ){
    حالت 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      بیرونی تهی sqlite3Memsys3Dump(ثابت نویسه*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      بشکن;
#endif
    }
    حالت 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      بیرونی تهی sqlite3Memsys5Dump(ثابت نویسه*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      بشکن;
#endif
    }
  }
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
ایستا صحیح SQLITE_TCLAPI test_status(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc, i_مقدار, mx_مقدار;
  صحیح i, op = 0, resetFlag;
  ثابت نویسه *zOpName;
  ایستا ثابت ساختار {
    ثابت نویسه *zName;
    صحیح op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_پشته        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_شمار        },
  };
  Tcl_Obj *pResult;
  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    بازگشت TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  برای(i=0; i<ArraySize(aOp); i++){
    اگر( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      بشکن;
    }
  }
  اگر( i>=ArraySize(aOp) ){
    اگر( Tcl_GetIntFromObj(interp, objv[1], &op) ) بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) بازگشت TCL_ERROR;
  i_مقدار = 0;
  mx_مقدار = 0;
  rc = sqlite3_status(op, &i_مقدار, &mx_مقدار, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_مقدار));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_مقدار));
  Tcl_SetObjResult(interp, pResult);
  بازگشت TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
ایستا صحیح SQLITE_TCLAPI test_db_status(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc, i_مقدار, mx_مقدار;
  صحیح i, op = 0, resetFlag;
  ثابت نویسه *zOpName;
  sqlite3 *db;
  بیرونی صحیح getDbPointer(Tcl_Interp*, ثابت نویسه*, sqlite3**);
  ایستا ثابت ساختار {
    ثابت نویسه *zName;
    صحیح op;
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
  اگر( objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  اگر( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  اگر( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  برای(i=0; i<ArraySize(aOp); i++){
    اگر( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      بشکن;
    }
  }
  اگر( i>=ArraySize(aOp) ){
    اگر( Tcl_GetIntFromObj(interp, objv[2], &op) ) بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) بازگشت TCL_ERROR;
  i_مقدار = 0;
  mx_مقدار = 0;
  rc = sqlite3_db_status(db, op, &i_مقدار, &mx_مقدار, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_مقدار));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_مقدار));
  Tcl_SetObjResult(interp, pResult);
  بازگشت TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
ایستا صحیح SQLITE_TCLAPI test_install_malloc_faultsim(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc;
  صحیح isInstall;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "BOOLEAN");
    بازگشت TCL_ERROR;
  }
  اگر( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    بازگشت TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  بازگشت TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
ایستا صحیح SQLITE_TCLAPI test_install_memsys3(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  ثابت sqlite3_mem_methods *sqlite3MemGetMemsys3(تهی);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_VOLATILE);
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_vfs_oom_test(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  بیرونی صحیح sqlite3_memdebug_vfs_oom_test;
  اگر( objc>2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "?INTEGER?");
    بازگشت TCL_ERROR;
  }وگرنه اگر( objc==2 ){
    صحیح iNew;
    اگر( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) بازگشت TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  بازگشت TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_malloc_Init(Tcl_Interp *interp){
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
     صحیح clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_شمار ,0 },
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
  صحیح i;
  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  بازگشت TCL_OK;
}
#endif
