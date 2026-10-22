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
#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ "tclsqlite.h"
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
স্থির সংরচনা MemFault {
  পূর্ণাঙ্ক iCountdown;         /* Number of pending successes before a failure */
  পূর্ণাঙ্ক nRepeat;            /* Number of times to repeat the failure */
  পূর্ণাঙ্ক nBenign;            /* Number of benign failures seen since last config */
  পূর্ণাঙ্ক nFail;              /* Number of failures seen since last config */
  পূর্ণাঙ্ক nOkBefore;          /* Successful allocations prior to the first fault */
  পূর্ণাঙ্ক nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  পূর্ণাঙ্ক isInstalled;        /* True if the fault simulation layer is installed */
  পূর্ণাঙ্ক isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
স্থির শূন্য sqlite3Fault(শূন্য){
  স্থির u64 cnt = 0;
  cnt++;
  যদি( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
স্থির শূন্য sqlite3FirstFault(শূন্য){
  স্থির u64 cnt2 = 0;
  cnt2++;
  যদি( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
স্থির পূর্ণাঙ্ক faultsimStep(শূন্য){
  যদি( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    ঘূৰাই 0;
  }
  যদি( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    ঘূৰাই 0;
  }
  যদি( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  যদি( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  যদি( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  ঘূৰাই 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
স্থির শূন্য *faultsimMalloc(পূর্ণাঙ্ক n){
  শূন্য *p = 0;
  যদি( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  ঘূৰাই p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
স্থির শূন্য *faultsimRealloc(শূন্য *pOld, পূর্ণাঙ্ক n){
  শূন্য *p = 0;
  যদি( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  ঘূৰাই p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
স্থির শূন্য faultsimConfig(পূর্ণাঙ্ক nDelay, পূর্ণাঙ্ক nRepeat){
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
স্থির পূর্ণাঙ্ক faultsimFailures(শূন্য){
  ঘূৰাই memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
স্থির পূর্ণাঙ্ক faultsimBenignFailures(শূন্য){
  ঘূৰাই memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
স্থির পূর্ণাঙ্ক faultsimPending(শূন্য){
  যদি( memfault.enable ){
    ঘূৰাই memfault.iCountdown;
  }অন্যথা{
    ঘূৰাই -1;
  }
}


স্থির শূন্য faultsimBeginBenign(শূন্য){
  memfault.isBenignMode++;
}
স্থির শূন্য faultsimEndBenign(শূন্য){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
স্থির পূর্ণাঙ্ক faultsimInstall(পূর্ণাঙ্ক install){
  পূর্ণাঙ্ক rc;

  install = (install ? 1 : 0);
  প্রতিজ্ঞা(memfault.isInstalled==1 || memfault.isInstalled==0);

  যদি( install==memfault.isInstalled ){
    ঘূৰাই SQLITE_ERROR;
  }

  যদি( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    প্রতিজ্ঞা(memfault.m.xMalloc);
    যদি( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }অন্যথা{
    sqlite3_mem_methods m2;
    প্রতিজ্ঞা(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, পরিমাণ(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    প্রতিজ্ঞা( memcmp(&m2, &memfault.m, পরিমাণ(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (শূন্য*)0, (শূন্য*)0);
  }

  যদি( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  ঘূৰাই rc;
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
বাহ্য ধ্রুৱ ৱর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);

/*
** Transform pointers to text and back again
*/
স্থির শূন্য pointerToText(শূন্য *p, ৱর্ণ *z){
  ধ্রুৱ ৱর্ণ *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
স্থির পূর্ণাঙ্ক textToPointer(ধ্রুৱ ৱর্ণ *z, শূন্য **pp){
  *pp = sqlite3TestTextToPtr(z);
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_malloc(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক nByte;
  শূন্য *p;
  ৱর্ণ zOut[100];
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NBYTES");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) ঘূৰাই TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, রিক্ত);
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_realloc(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক nByte;
  শূন্য *pPrior, *p;
  ৱর্ণ zOut[100];
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "PRIOR NBYTES");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) ঘূৰাই TCL_ERROR;
  যদি( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, রিক্ত);
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_free(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য *pPrior;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "PRIOR");
    ঘূৰাই TCL_ERROR;
  }
  যদি( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  sqlite3_free(pPrior);
  ঘূৰাই TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
পূর্ণাঙ্ক sqlite3TestHexToBin(ধ্রুৱ ৱর্ণ *, পূর্ণাঙ্ক, ৱর্ণ *);
পূর্ণাঙ্ক sqlite3TestBinToHex(ৱর্ণ*,পূর্ণাঙ্ক);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memset(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য *p;
  পূর্ণাঙ্ক size, i;
  Tcl_Size n;
  ৱর্ণ *zHex;
  ৱর্ণ *zOut;
  ৱর্ণ zBin[100];

  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    ঘূৰাই TCL_ERROR;
  }
  যদি( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    ঘূৰাই TCL_ERROR;
  }
  যদি( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  যদি( n>পরিমাণ(zBin)*2 ) n = পরিমাণ(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (পূর্ণাঙ্ক)n, zBin);
  যদি( n==0 ){
    Tcl_AppendResult(interp, "no data", (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  zOut = p;
  বাবে(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  ঘূৰাই TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memget(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য *p;
  পূর্ণাঙ্ক size, n;
  ৱর্ণ *zBin;
  ৱর্ণ zHex[100];

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "ADDRESS SIZE");
    ঘূৰাই TCL_ERROR;
  }
  যদি( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    ঘূৰাই TCL_ERROR;
  }
  যদি( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  zBin = p;
  যেতিয়ালৈ( size>0 ){
    যদি( size>(পরিমাণ(zHex)-1)/2 ){
      n = (পরিমাণ(zHex)-1)/2;
    }অন্যথা{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (ৱর্ণ*)0);
  }
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memory_used(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memory_highwater(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক resetFlag = 0;
  যদি( objc!=1 && objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "?RESET?");
    ঘূৰাই TCL_ERROR;
  }
  যদি( objc==2 ){
    যদি( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) ঘূৰাই TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memdebug_backtrace(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক depth;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DEPT");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &depth) ) ঘূৰাই TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    বাহ্য শূন্য sqlite3MemdebugBacktrace(পূর্ণাঙ্ক);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memdebug_dump(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME");
    ঘূৰাই TCL_ERROR;
  }
#যদি defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    বাহ্য শূন্য sqlite3MemdebugDump(ধ্রুৱ ৱর্ণ*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memdebug_malloc_গুনতি(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক nMalloc = -1;
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ঘূৰাই TCL_ERROR;
  }
#যদি defined(SQLITE_MEMDEBUG)
  {
    বাহ্য পূর্ণাঙ্ক sqlite3_Memdebug_Malloc_গুনতি();
    nMalloc = sqlite3_Memdebug_Malloc_গুনতি();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  ঘূৰাই TCL_OK;
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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memdebug_fail(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক ii;
  পূর্ণাঙ্ক iFail;
  পূর্ণাঙ্ক nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  পূর্ণাঙ্ক nBenign;
  পূর্ণাঙ্ক nFail = 0;

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) ঘূৰাই TCL_ERROR;

  বাবে(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    ৱর্ণ *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    ৱর্ণ *zErr = 0;

    যদি( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      যদি( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }অন্যথা{
        যদি( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          ঘূৰাই TCL_ERROR;
        }
      }
    }অন্যথা যদি( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      যদি( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }অন্যথা{
        pBenignCnt = objv[ii+1];
      }
    }অন্যথা{
      zErr = "unknown option: ";
    }

    যদি( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, রিক্ত);
      ঘূৰাই TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  যদি( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memdebug_pending(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক nPending;
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ঘূৰাই TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  ঘূৰাই TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
স্থির পূর্ণাঙ্ক sqlite3_memdebug_title_গুনতি = 0;

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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memdebug_settitle(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_গুনতি++;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "TITLE");
    ঘূৰাই TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    ধ্রুৱ ৱর্ণ *zTitle;
    বাহ্য পূর্ণাঙ্ক sqlite3MemdebugSettitle(ধ্রুৱ ৱর্ণ*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  ঘূৰাই TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((পরিমাণ(পূর্ণাঙ্ক)>=পরিমাণ(শূন্য*)) ? 1 : পরিমাণ(শূন্য*)/পরিমাণ(পূর্ণাঙ্ক))   \
)
স্থির Tcl_HashTable aMallocLog;
স্থির পূর্ণাঙ্ক mallocLogEnabled = 0;

প্রকারনির্দেশ সংরচনা MallocLog MallocLog;
সংরচনা MallocLog {
  পূর্ণাঙ্ক nCall;
  পূর্ণাঙ্ক nByte;
};

#ifdef SQLITE_MEMDEBUG
স্থির শূন্য test_memdebug_callback(পূর্ণাঙ্ক nByte, পূর্ণাঙ্ক nFrame, শূন্য **aFrame){
  যদি( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    পূর্ণাঙ্ক isNew;

    পূর্ণাঙ্ক a_চাবি[MALLOC_LOG_KEYINTS];
    unsigned পূর্ণাঙ্ক n_চাবি = পরিমাণ(পূর্ণাঙ্ক)*MALLOC_LOG_KEYINTS;

    memset(a_চাবি, 0, n_চাবি);
    যদি( (পরিমাণ(শূন্য*)*nFrame)<n_চাবি ){
      n_চাবি = nFrame*পরিমাণ(শূন্য*);
    }
    memcpy(a_চাবি, aFrame, n_চাবি);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (ধ্রুৱ ৱর্ণ *)a_চাবি, &isNew);
    যদি( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(পরিমাণ(MallocLog));
      memset(pLog, 0, পরিমাণ(MallocLog));
      Tcl_Set_Hash_মান(pEntry, (ClientData)pLog);
    }অন্যথা{
      pLog = (MallocLog *)Tcl_Get_Hash_মান(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

স্থির শূন্য test_memdebug_log_clear(শূন্য){
  Tcl_Hash_অনুসন্ধান অনুসন্ধান;
  Tcl_HashEntry *pEntry;
  বাবে(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &অনুসন্ধান);
    pEntry;
    pEntry=Tcl_NextHashEntry(&অনুসন্ধান)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_মান(pEntry);
    Tcl_Free((ৱর্ণ *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_memdebug_log(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  স্থির পূর্ণাঙ্ক isInit = 0;
  পূর্ণাঙ্ক iSub;

  স্থির ধ্রুৱ ৱর্ণ *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  গণনা MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  যদি( !isInit ){
#ifdef SQLITE_MEMDEBUG
    বাহ্য শূন্য sqlite3MemdebugBacktraceCallback(
        শূন্য (*xBacktrace)(পূর্ণাঙ্ক, পূর্ণাঙ্ক, শূন্য **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  যদি( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    ঘূৰাই TCL_ERROR;
  }

  ৱিকল্প( (গণনা MB_enum)iSub ){
    পক্ষ MB_LOG_START:
      mallocLogEnabled = 1;
      ভঙ্গ;
    পক্ষ MB_LOG_STOP:
      mallocLogEnabled = 0;
      ভঙ্গ;
    পক্ষ MB_LOG_DUMP: {
      Tcl_Hash_অনুসন্ধান অনুসন্ধান;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      প্রতিজ্ঞা(পরিমাণ(Tcl_WideInt)>=পরিমাণ(শূন্য*));

      বাবে(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &অনুসন্ধান);
        pEntry;
        pEntry=Tcl_NextHashEntry(&অনুসন্ধান)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_মান(pEntry);
        Tcl_WideInt *a_চাবি = (Tcl_WideInt *)Tcl_Get_Hash_চাবি(&aMallocLog, pEntry);
        পূর্ণাঙ্ক ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        বাবে(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_চাবি[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      ভঙ্গ;
    }
    পক্ষ MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      ভঙ্গ;
    }

    পক্ষ MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      বাহ্য শূন্য sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      ভঙ্গ;
    }
  }

  ঘূৰাই TCL_OK;
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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_pagecache(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক sz, N;
  Tcl_Obj *pRes;
  স্থির ৱর্ণ *buf = 0;
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SIZE N");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ঘূৰাই TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[2], &N) ) ঘূৰাই TCL_ERROR;
  মুক্ত(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  যদি( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (শূন্য*)0, 0, 0);
  }অন্যথা{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  ঘূৰাই TCL_OK;
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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_alt_pcache(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক installFlag;
  পূর্ণাঙ্ক discardChance = 0;
  পূর্ণাঙ্ক prngSeed = 0;
  পূর্ণাঙ্ক উপৰেৰ_Stress = 0;
  বাহ্য শূন্য installTestPCache(পূর্ণাঙ্ক,unsigned,unsigned,unsigned);
  যদি( objc<2 || objc>5 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) ঘূৰাই TCL_ERROR;
  যদি( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     ঘূৰাই TCL_ERROR;
  }
  যদি( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     ঘূৰাই TCL_ERROR;
  }
  যদি( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &উপৰেৰ_Stress) ){
    ঘূৰাই TCL_ERROR;
  }
  যদি( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)উপৰেৰ_Stress);
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_memstatus(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক enable, rc;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BOOLEAN");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) ঘূৰাই TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_lookaside(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক sz, cnt;
  Tcl_Obj *pRet;
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SIZE COUNT");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &sz) ) ঘূৰাই TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) ঘূৰাই TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  ঘূৰাই TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_db_config_lookaside(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক sz, cnt;
  sqlite3 *db;
  পূর্ণাঙ্ক bufid;
  স্থির ৱর্ণ azBuf[2][10000];
  বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp*, ধ্রুৱ ৱর্ণ*, sqlite3**);
  যদি( objc!=5 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BUFID SIZE COUNT");
    ঘূৰাই TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ঘূৰাই TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) ঘূৰাই TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[3], &sz) ) ঘূৰাই TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) ঘূৰাই TCL_ERROR;
  যদি( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (শূন্য*)0, sz, cnt);
  }অন্যথা যদি( bufid>=1 && bufid<=2 && sz*cnt<=পরিমাণ(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }অন্যথা{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (ৱর্ণ*)0);
    ঘূৰাই TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_heap(
  শূন্য * clientData, 
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  স্থির ৱর্ণ *zBuf; /* Use this memory */
  পূর্ণাঙ্ক nByte;         /* Size of buffer to pass to sqlite3_config() */
  পূর্ণাঙ্ক nMinAlloc;     /* Size of minimum allocation */
  পূর্ণাঙ্ক rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  পূর্ণাঙ্ক nArg = objc-1;

  যদি( nArg!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NBYTE NMINALLOC");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ঘূৰাই TCL_ERROR;
  যদি( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) ঘূৰাই TCL_ERROR;

  যদি( nByte==0 ){
    মুক্ত( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (শূন্য*)0, 0, 0);
  }অন্যথা{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_heap_size(
  শূন্য * clientData, 
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক nByte;         /* Size to pass to sqlite3_config() */
  পূর্ণাঙ্ক rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  পূর্ণাঙ্ক nArg = objc-1;

  যদি( nArg!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "NBYTE");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) ঘূৰাই TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_error(
  শূন্য * clientData, 
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp*, ধ্রুৱ ৱর্ণ*, sqlite3**);

  যদি( objc!=2 && objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "[DB]");
    ঘূৰাই TCL_ERROR;
  }
  যদি( objc==2 ){
    যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ঘূৰাই TCL_ERROR;
    যদি( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (ৱর্ণ*)0);
      ঘূৰাই TCL_ERROR;
    }
  }অন্যথা{
    যদি( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (ৱর্ণ*)0);
      ঘূৰাই TCL_ERROR;
    }
  }
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_uri(
  শূন্য * clientData, 
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক bOpenUri;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BOOL");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    ঘূৰাই TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_cis(
  শূন্য * clientData, 
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক bUseCis;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BOOL");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    ঘূৰাই TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_config_pmasz(
  শূন্য * clientData, 
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক iPmaSz;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BOOL");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    ঘূৰাই TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);

  ঘূৰাই TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_dump_memsys3(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "FILENAME");
    ঘূৰাই TCL_ERROR;
  }

  ৱিকল্প( SQLITE_PTR_TO_INT(clientData) ){
    পক্ষ 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      বাহ্য শূন্য sqlite3Memsys3Dump(ধ্রুৱ ৱর্ণ*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      ভঙ্গ;
#endif
    }
    পক্ষ 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      বাহ্য শূন্য sqlite3Memsys5Dump(ধ্রুৱ ৱর্ণ*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      ভঙ্গ;
#endif
    }
  }
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_status(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc, i_মান, mx_মান;
  পূর্ণাঙ্ক i, op = 0, resetFlag;
  ধ্রুৱ ৱর্ণ *zOpName;
  স্থির ধ্রুৱ সংরচনা {
    ধ্রুৱ ৱর্ণ *zName;
    পূর্ণাঙ্ক op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_স্তূপ        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_গুনতি        },
  };
  Tcl_Obj *pResult;
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    ঘূৰাই TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  বাবে(i=0; i<ArraySize(aOp); i++){
    যদি( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ভঙ্গ;
    }
  }
  যদি( i>=ArraySize(aOp) ){
    যদি( Tcl_GetIntFromObj(interp, objv[1], &op) ) ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) ঘূৰাই TCL_ERROR;
  i_মান = 0;
  mx_মান = 0;
  rc = sqlite3_status(op, &i_মান, &mx_মান, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_মান));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_মান));
  Tcl_SetObjResult(interp, pResult);
  ঘূৰাই TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_db_status(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc, i_মান, mx_মান;
  পূর্ণাঙ্ক i, op = 0, resetFlag;
  ধ্রুৱ ৱর্ণ *zOpName;
  sqlite3 *db;
  বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp*, ধ্রুৱ ৱর্ণ*, sqlite3**);
  স্থির ধ্রুৱ সংরচনা {
    ধ্রুৱ ৱর্ণ *zName;
    পূর্ণাঙ্ক op;
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
  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    ঘূৰাই TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ঘূৰাই TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  যদি( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  যদি( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  বাবে(i=0; i<ArraySize(aOp); i++){
    যদি( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      ভঙ্গ;
    }
  }
  যদি( i>=ArraySize(aOp) ){
    যদি( Tcl_GetIntFromObj(interp, objv[2], &op) ) ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) ঘূৰাই TCL_ERROR;
  i_মান = 0;
  mx_মান = 0;
  rc = sqlite3_db_status(db, op, &i_মান, &mx_মান, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_মান));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_মান));
  Tcl_SetObjResult(interp, pResult);
  ঘূৰাই TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_install_malloc_faultsim(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক isInstall;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "BOOLEAN");
    ঘূৰাই TCL_ERROR;
  }
  যদি( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    ঘূৰাই TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ঘূৰাই TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_install_memsys3(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  ধ্রুৱ sqlite3_mem_methods *sqlite3MemGetMemsys3(শূন্য);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_VOLATILE);
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_vfs_oom_test(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  বাহ্য পূর্ণাঙ্ক sqlite3_memdebug_vfs_oom_test;
  যদি( objc>2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "?INTEGER?");
    ঘূৰাই TCL_ERROR;
  }অন্যথা যদি( objc==2 ){
    পূর্ণাঙ্ক iNew;
    যদি( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) ঘূৰাই TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  ঘূৰাই TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণাঙ্ক Sqlitetest_malloc_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
     ৱর্ণ *zName;
     Tcl_ObjCmdProc *xProc;
     পূর্ণাঙ্ক clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_গুনতি ,0 },
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
  পূর্ণাঙ্ক i;
  বাবে(i=0; i<পরিমাণ(aObjCmd)/পরিমাণ(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ঘূৰাই TCL_OK;
}
#endif
