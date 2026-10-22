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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
நிலையான கட்டமைப்பு MemFault {
  முழு iCountdown;         /* Number of pending successes before a failure */
  முழு nRepeat;            /* Number of times to repeat the failure */
  முழு nBenign;            /* Number of benign failures seen since last config */
  முழு nFail;              /* Number of failures seen since last config */
  முழு nOkBefore;          /* Successful allocations prior to the first fault */
  முழு nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  முழு isInstalled;        /* True if the fault simulation layer is installed */
  முழு isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
நிலையான வெறுமை sqlite3Fault(வெறுமை){
  நிலையான u64 cnt = 0;
  cnt++;
  எனில்( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
நிலையான வெறுமை sqlite3FirstFault(வெறுமை){
  நிலையான u64 cnt2 = 0;
  cnt2++;
  எனில்( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
நிலையான முழு faultsimStep(வெறுமை){
  எனில்( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    திரும்பு 0;
  }
  எனில்( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    திரும்பு 0;
  }
  எனில்( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  எனில்( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  எனில்( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  திரும்பு 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
நிலையான வெறுமை *faultsimMalloc(முழு n){
  வெறுமை *p = 0;
  எனில்( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  திரும்பு p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
நிலையான வெறுமை *faultsimRealloc(வெறுமை *pOld, முழு n){
  வெறுமை *p = 0;
  எனில்( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  திரும்பு p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
நிலையான வெறுமை faultsimConfig(முழு nDelay, முழு nRepeat){
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
நிலையான முழு faultsimFailures(வெறுமை){
  திரும்பு memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
நிலையான முழு faultsimBenignFailures(வெறுமை){
  திரும்பு memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
நிலையான முழு faultsimPending(வெறுமை){
  எனில்( memfault.enable ){
    திரும்பு memfault.iCountdown;
  }இல்லையெனில்{
    திரும்பு -1;
  }
}


நிலையான வெறுமை faultsimBeginBenign(வெறுமை){
  memfault.isBenignMode++;
}
நிலையான வெறுமை faultsimEndBenign(வெறுமை){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
நிலையான முழு faultsimInstall(முழு install){
  முழு rc;

  install = (install ? 1 : 0);
  ப்ரதிஜ்ஞா(memfault.isInstalled==1 || memfault.isInstalled==0);

  எனில்( install==memfault.isInstalled ){
    திரும்பு SQLITE_ERROR;
  }

  எனில்( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    ப்ரதிஜ்ஞா(memfault.m.xMalloc);
    எனில்( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }இல்லையெனில்{
    sqlite3_mem_methods m2;
    ப்ரதிஜ்ஞா(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, அளவு(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    ப்ரதிஜ்ஞா( memcmp(&m2, &memfault.m, அளவு(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (வெறுமை*)0, (வெறுமை*)0);
  }

  எனில்( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  திரும்பு rc;
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
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

/*
** Transform pointers to text and back again
*/
நிலையான வெறுமை pointerToText(வெறுமை *p, எழுத்து *z){
  மாறிலி எழுத்து *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
நிலையான முழு textToPointer(மாறிலி எழுத்து *z, வெறுமை **pp){
  *pp = sqlite3TestTextToPtr(z);
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
நிலையான முழு SQLITE_TCLAPI test_malloc(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு nByte;
  வெறுமை *p;
  எழுத்து zOut[100];
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "NBYTES");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) திரும்பு TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
நிலையான முழு SQLITE_TCLAPI test_realloc(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு nByte;
  வெறுமை *pPrior, *p;
  எழுத்து zOut[100];
  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "PRIOR NBYTES");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) திரும்பு TCL_ERROR;
  எனில்( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, வெற்று);
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
நிலையான முழு SQLITE_TCLAPI test_free(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  வெறுமை *pPrior;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "PRIOR");
    திரும்பு TCL_ERROR;
  }
  எனில்( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  sqlite3_free(pPrior);
  திரும்பு TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
முழு sqlite3TestHexToBin(மாறிலி எழுத்து *, முழு, எழுத்து *);
முழு sqlite3TestBinToHex(எழுத்து*,முழு);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
நிலையான முழு SQLITE_TCLAPI test_memset(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  வெறுமை *p;
  முழு size, i;
  Tcl_Size n;
  எழுத்து *zHex;
  எழுத்து *zOut;
  எழுத்து zBin[100];

  எனில்( objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    திரும்பு TCL_ERROR;
  }
  எனில்( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    திரும்பு TCL_ERROR;
  }
  எனில்( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  எனில்( n>அளவு(zBin)*2 ) n = அளவு(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (முழு)n, zBin);
  எனில்( n==0 ){
    Tcl_AppendResult(interp, "no data", (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  zOut = p;
  க்கு(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  திரும்பு TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
நிலையான முழு SQLITE_TCLAPI test_memget(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  வெறுமை *p;
  முழு size, n;
  எழுத்து *zBin;
  எழுத்து zHex[100];

  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "ADDRESS SIZE");
    திரும்பு TCL_ERROR;
  }
  எனில்( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    திரும்பு TCL_ERROR;
  }
  எனில்( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  zBin = p;
  போது( size>0 ){
    எனில்( size>(அளவு(zHex)-1)/2 ){
      n = (அளவு(zHex)-1)/2;
    }இல்லையெனில்{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (எழுத்து*)0);
  }
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
நிலையான முழு SQLITE_TCLAPI test_memory_used(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
நிலையான முழு SQLITE_TCLAPI test_memory_highwater(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு resetFlag = 0;
  எனில்( objc!=1 && objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "?RESET?");
    திரும்பு TCL_ERROR;
  }
  எனில்( objc==2 ){
    எனில்( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) திரும்பு TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
நிலையான முழு SQLITE_TCLAPI test_memdebug_backtrace(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு depth;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DEPT");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &depth) ) திரும்பு TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    வெளி வெறுமை sqlite3MemdebugBacktrace(முழு);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
நிலையான முழு SQLITE_TCLAPI test_memdebug_dump(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "FILENAME");
    திரும்பு TCL_ERROR;
  }
#எனில் defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    வெளி வெறுமை sqlite3MemdebugDump(மாறிலி எழுத்து*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
நிலையான முழு SQLITE_TCLAPI test_memdebug_malloc_மொத்தம்(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு nMalloc = -1;
  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }
#எனில் defined(SQLITE_MEMDEBUG)
  {
    வெளி முழு sqlite3_Memdebug_Malloc_மொத்தம்();
    nMalloc = sqlite3_Memdebug_Malloc_மொத்தம்();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI test_memdebug_fail(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு ii;
  முழு iFail;
  முழு nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  முழு nBenign;
  முழு nFail = 0;

  எனில்( objc<2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) திரும்பு TCL_ERROR;

  க்கு(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    எழுத்து *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    எழுத்து *zErr = 0;

    எனில்( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      எனில்( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }இல்லையெனில்{
        எனில்( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          திரும்பு TCL_ERROR;
        }
      }
    }இல்லையெனில் எனில்( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      எனில்( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }இல்லையெனில்{
        pBenignCnt = objv[ii+1];
      }
    }இல்லையெனில்{
      zErr = "unknown option: ";
    }

    எனில்( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, வெற்று);
      திரும்பு TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  எனில்( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
நிலையான முழு SQLITE_TCLAPI test_memdebug_pending(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு nPending;
  எனில்( objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "");
    திரும்பு TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  திரும்பு TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
நிலையான முழு sqlite3_memdebug_title_மொத்தம் = 0;

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
நிலையான முழு SQLITE_TCLAPI test_memdebug_settitle(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_மொத்தம்++;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "TITLE");
    திரும்பு TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    மாறிலி எழுத்து *zTitle;
    வெளி முழு sqlite3MemdebugSettitle(மாறிலி எழுத்து*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  திரும்பு TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((அளவு(முழு)>=அளவு(வெறுமை*)) ? 1 : அளவு(வெறுமை*)/அளவு(முழு))   \
)
நிலையான Tcl_HashTable aMallocLog;
நிலையான முழு mallocLogEnabled = 0;

வகைவரை கட்டமைப்பு MallocLog MallocLog;
கட்டமைப்பு MallocLog {
  முழு nCall;
  முழு nByte;
};

#ifdef SQLITE_MEMDEBUG
நிலையான வெறுமை test_memdebug_callback(முழு nByte, முழு nFrame, வெறுமை **aFrame){
  எனில்( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    முழு isNew;

    முழு a_சாவி[MALLOC_LOG_KEYINTS];
    unsigned முழு n_சாவி = அளவு(முழு)*MALLOC_LOG_KEYINTS;

    memset(a_சாவி, 0, n_சாவி);
    எனில்( (அளவு(வெறுமை*)*nFrame)<n_சாவி ){
      n_சாவி = nFrame*அளவு(வெறுமை*);
    }
    memcpy(a_சாவி, aFrame, n_சாவி);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (மாறிலி எழுத்து *)a_சாவி, &isNew);
    எனில்( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(அளவு(MallocLog));
      memset(pLog, 0, அளவு(MallocLog));
      Tcl_Set_Hash_மதிப்பு(pEntry, (ClientData)pLog);
    }இல்லையெனில்{
      pLog = (MallocLog *)Tcl_Get_Hash_மதிப்பு(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

நிலையான வெறுமை test_memdebug_log_clear(வெறுமை){
  Tcl_Hash_தேடல் தேடல்;
  Tcl_HashEntry *pEntry;
  க்கு(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &தேடல்);
    pEntry;
    pEntry=Tcl_NextHashEntry(&தேடல்)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_மதிப்பு(pEntry);
    Tcl_Free((எழுத்து *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

நிலையான முழு SQLITE_TCLAPI test_memdebug_log(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  நிலையான முழு isInit = 0;
  முழு iSub;

  நிலையான மாறிலி எழுத்து *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  எண்ணிக்கை MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  எனில்( !isInit ){
#ifdef SQLITE_MEMDEBUG
    வெளி வெறுமை sqlite3MemdebugBacktraceCallback(
        வெறுமை (*xBacktrace)(முழு, முழு, வெறுமை **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  எனில்( objc<2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  எனில்( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    திரும்பு TCL_ERROR;
  }

  தேர்வு( (எண்ணிக்கை MB_enum)iSub ){
    நிலை MB_LOG_START:
      mallocLogEnabled = 1;
      நிறுத்து;
    நிலை MB_LOG_STOP:
      mallocLogEnabled = 0;
      நிறுத்து;
    நிலை MB_LOG_DUMP: {
      Tcl_Hash_தேடல் தேடல்;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      ப்ரதிஜ்ஞா(அளவு(Tcl_WideInt)>=அளவு(வெறுமை*));

      க்கு(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &தேடல்);
        pEntry;
        pEntry=Tcl_NextHashEntry(&தேடல்)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_மதிப்பு(pEntry);
        Tcl_WideInt *a_சாவி = (Tcl_WideInt *)Tcl_Get_Hash_சாவி(&aMallocLog, pEntry);
        முழு ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        க்கு(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_சாவி[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      நிறுத்து;
    }
    நிலை MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      நிறுத்து;
    }

    நிலை MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      வெளி வெறுமை sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      நிறுத்து;
    }
  }

  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI test_config_pagecache(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு sz, N;
  Tcl_Obj *pRes;
  நிலையான எழுத்து *buf = 0;
  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SIZE N");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &sz) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &N) ) திரும்பு TCL_ERROR;
  விடுவி(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  எனில்( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (வெறுமை*)0, 0, 0);
  }இல்லையெனில்{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  திரும்பு TCL_OK;
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
நிலையான முழு SQLITE_TCLAPI test_alt_pcache(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு installFlag;
  முழு discardChance = 0;
  முழு prngSeed = 0;
  முழு உயர்வு_Stress = 0;
  வெளி வெறுமை installTestPCache(முழு,unsigned,unsigned,unsigned);
  எனில்( objc<2 || objc>5 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) திரும்பு TCL_ERROR;
  எனில்( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     திரும்பு TCL_ERROR;
  }
  எனில்( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     திரும்பு TCL_ERROR;
  }
  எனில்( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &உயர்வு_Stress) ){
    திரும்பு TCL_ERROR;
  }
  எனில்( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)உயர்வு_Stress);
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
நிலையான முழு SQLITE_TCLAPI test_config_memstatus(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு enable, rc;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "BOOLEAN");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) திரும்பு TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
நிலையான முழு SQLITE_TCLAPI test_config_lookaside(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு sz, cnt;
  Tcl_Obj *pRet;
  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SIZE COUNT");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &sz) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) திரும்பு TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  திரும்பு TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
நிலையான முழு SQLITE_TCLAPI test_db_config_lookaside(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;
  முழு sz, cnt;
  sqlite3 *db;
  முழு bufid;
  நிலையான எழுத்து azBuf[2][10000];
  வெளி முழு getDbPointer(Tcl_Interp*, மாறிலி எழுத்து*, sqlite3**);
  எனில்( objc!=5 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "BUFID SIZE COUNT");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, objv[3], &sz) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) திரும்பு TCL_ERROR;
  எனில்( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (வெறுமை*)0, sz, cnt);
  }இல்லையெனில் எனில்( bufid>=1 && bufid<=2 && sz*cnt<=அளவு(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }இல்லையெனில்{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (எழுத்து*)0);
    திரும்பு TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
நிலையான முழு SQLITE_TCLAPI test_config_heap(
  வெறுமை * clientData, 
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  நிலையான எழுத்து *zBuf; /* Use this memory */
  முழு nByte;         /* Size of buffer to pass to sqlite3_config() */
  முழு nMinAlloc;     /* Size of minimum allocation */
  முழு rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  முழு nArg = objc-1;

  எனில்( nArg!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "NBYTE NMINALLOC");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) திரும்பு TCL_ERROR;
  எனில்( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) திரும்பு TCL_ERROR;

  எனில்( nByte==0 ){
    விடுவி( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (வெறுமை*)0, 0, 0);
  }இல்லையெனில்{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
நிலையான முழு SQLITE_TCLAPI test_config_heap_size(
  வெறுமை * clientData, 
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு nByte;         /* Size to pass to sqlite3_config() */
  முழு rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  முழு nArg = objc-1;

  எனில்( nArg!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "NBYTE");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) திரும்பு TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
நிலையான முழு SQLITE_TCLAPI test_config_error(
  வெறுமை * clientData, 
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  வெளி முழு getDbPointer(Tcl_Interp*, மாறிலி எழுத்து*, sqlite3**);

  எனில்( objc!=2 && objc!=1 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "[DB]");
    திரும்பு TCL_ERROR;
  }
  எனில்( objc==2 ){
    எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
    எனில்( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
  }இல்லையெனில்{
    எனில்( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
  }
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
நிலையான முழு SQLITE_TCLAPI test_config_uri(
  வெறுமை * clientData, 
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;
  முழு bOpenUri;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "BOOL");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    திரும்பு TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);

  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
நிலையான முழு SQLITE_TCLAPI test_config_cis(
  வெறுமை * clientData, 
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;
  முழு bUseCis;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "BOOL");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    திரும்பு TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);

  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
நிலையான முழு SQLITE_TCLAPI test_config_pmasz(
  வெறுமை * clientData, 
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;
  முழு iPmaSz;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "BOOL");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    திரும்பு TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);

  திரும்பு TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
நிலையான முழு SQLITE_TCLAPI test_dump_memsys3(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "FILENAME");
    திரும்பு TCL_ERROR;
  }

  தேர்வு( SQLITE_PTR_TO_INT(clientData) ){
    நிலை 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      வெளி வெறுமை sqlite3Memsys3Dump(மாறிலி எழுத்து*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      நிறுத்து;
#endif
    }
    நிலை 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      வெளி வெறுமை sqlite3Memsys5Dump(மாறிலி எழுத்து*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      நிறுத்து;
#endif
    }
  }
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
நிலையான முழு SQLITE_TCLAPI test_status(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc, i_மதிப்பு, mx_மதிப்பு;
  முழு i, op = 0, resetFlag;
  மாறிலி எழுத்து *zOpName;
  நிலையான மாறிலி கட்டமைப்பு {
    மாறிலி எழுத்து *zName;
    முழு op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_அடுக்கு        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_மொத்தம்        },
  };
  Tcl_Obj *pResult;
  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    திரும்பு TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  க்கு(i=0; i<ArraySize(aOp); i++){
    எனில்( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      நிறுத்து;
    }
  }
  எனில்( i>=ArraySize(aOp) ){
    எனில்( Tcl_GetIntFromObj(interp, objv[1], &op) ) திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) திரும்பு TCL_ERROR;
  i_மதிப்பு = 0;
  mx_மதிப்பு = 0;
  rc = sqlite3_status(op, &i_மதிப்பு, &mx_மதிப்பு, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_மதிப்பு));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_மதிப்பு));
  Tcl_SetObjResult(interp, pResult);
  திரும்பு TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
நிலையான முழு SQLITE_TCLAPI test_db_status(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc, i_மதிப்பு, mx_மதிப்பு;
  முழு i, op = 0, resetFlag;
  மாறிலி எழுத்து *zOpName;
  sqlite3 *db;
  வெளி முழு getDbPointer(Tcl_Interp*, மாறிலி எழுத்து*, sqlite3**);
  நிலையான மாறிலி கட்டமைப்பு {
    மாறிலி எழுத்து *zName;
    முழு op;
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
  எனில்( objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  எனில்( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  எனில்( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  க்கு(i=0; i<ArraySize(aOp); i++){
    எனில்( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      நிறுத்து;
    }
  }
  எனில்( i>=ArraySize(aOp) ){
    எனில்( Tcl_GetIntFromObj(interp, objv[2], &op) ) திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) திரும்பு TCL_ERROR;
  i_மதிப்பு = 0;
  mx_மதிப்பு = 0;
  rc = sqlite3_db_status(db, op, &i_மதிப்பு, &mx_மதிப்பு, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_மதிப்பு));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_மதிப்பு));
  Tcl_SetObjResult(interp, pResult);
  திரும்பு TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
நிலையான முழு SQLITE_TCLAPI test_install_malloc_faultsim(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc;
  முழு isInstall;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "BOOLEAN");
    திரும்பு TCL_ERROR;
  }
  எனில்( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    திரும்பு TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
நிலையான முழு SQLITE_TCLAPI test_install_memsys3(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  மாறிலி sqlite3_mem_methods *sqlite3MemGetMemsys3(வெறுமை);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_VOLATILE);
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_vfs_oom_test(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  வெளி முழு sqlite3_memdebug_vfs_oom_test;
  எனில்( objc>2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "?INTEGER?");
    திரும்பு TCL_ERROR;
  }இல்லையெனில் எனில்( objc==2 ){
    முழு iNew;
    எனில்( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) திரும்பு TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  திரும்பு TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetest_malloc_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
     முழு clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_மொத்தம் ,0 },
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
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  திரும்பு TCL_OK;
}
#endif
