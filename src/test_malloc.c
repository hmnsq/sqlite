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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
સ્થિર સંરચના MemFault {
  પૂર્ણાંક iCountdown;         /* Number of pending successes before a failure */
  પૂર્ણાંક nRepeat;            /* Number of times to repeat the failure */
  પૂર્ણાંક nBenign;            /* Number of benign failures seen since last config */
  પૂર્ણાંક nFail;              /* Number of failures seen since last config */
  પૂર્ણાંક nOkBefore;          /* Successful allocations prior to the first fault */
  પૂર્ણાંક nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  પૂર્ણાંક isInstalled;        /* True if the fault simulation layer is installed */
  પૂર્ણાંક isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
સ્થિર શૂન્ય sqlite3Fault(શૂન્ય){
  સ્થિર u64 cnt = 0;
  cnt++;
  જો( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
સ્થિર શૂન્ય sqlite3FirstFault(શૂન્ય){
  સ્થિર u64 cnt2 = 0;
  cnt2++;
  જો( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
સ્થિર પૂર્ણાંક faultsimStep(શૂન્ય){
  જો( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    પાછા 0;
  }
  જો( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    પાછા 0;
  }
  જો( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  જો( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  જો( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  પાછા 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
સ્થિર શૂન્ય *faultsimMalloc(પૂર્ણાંક n){
  શૂન્ય *p = 0;
  જો( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  પાછા p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
સ્થિર શૂન્ય *faultsimRealloc(શૂન્ય *pOld, પૂર્ણાંક n){
  શૂન્ય *p = 0;
  જો( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  પાછા p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
સ્થિર શૂન્ય faultsimConfig(પૂર્ણાંક nDelay, પૂર્ણાંક nRepeat){
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
સ્થિર પૂર્ણાંક faultsimFailures(શૂન્ય){
  પાછા memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
સ્થિર પૂર્ણાંક faultsimBenignFailures(શૂન્ય){
  પાછા memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
સ્થિર પૂર્ણાંક faultsimPending(શૂન્ય){
  જો( memfault.enable ){
    પાછા memfault.iCountdown;
  }નહીંતર{
    પાછા -1;
  }
}


સ્થિર શૂન્ય faultsimBeginBenign(શૂન્ય){
  memfault.isBenignMode++;
}
સ્થિર શૂન્ય faultsimEndBenign(શૂન્ય){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
સ્થિર પૂર્ણાંક faultsimInstall(પૂર્ણાંક install){
  પૂર્ણાંક rc;

  install = (install ? 1 : 0);
  પ્રતિજ્ઞા(memfault.isInstalled==1 || memfault.isInstalled==0);

  જો( install==memfault.isInstalled ){
    પાછા SQLITE_ERROR;
  }

  જો( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    પ્રતિજ્ઞા(memfault.m.xMalloc);
    જો( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }નહીંતર{
    sqlite3_mem_methods m2;
    પ્રતિજ્ઞા(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, કદ(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    પ્રતિજ્ઞા( memcmp(&m2, &memfault.m, કદ(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (શૂન્ય*)0, (શૂન્ય*)0);
  }

  જો( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  પાછા rc;
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
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

/*
** Transform pointers to text and back again
*/
સ્થિર શૂન્ય pointerToText(શૂન્ય *p, અક્ષર *z){
  અચલ અક્ષર *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
સ્થિર પૂર્ણાંક textToPointer(અચલ અક્ષર *z, શૂન્ય **pp){
  *pp = sqlite3TestTextToPtr(z);
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_malloc(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક nByte;
  શૂન્ય *p;
  અક્ષર zOut[100];
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "NBYTES");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) પાછા TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, ખાલી);
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_realloc(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક nByte;
  શૂન્ય *pPrior, *p;
  અક્ષર zOut[100];
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "PRIOR NBYTES");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) પાછા TCL_ERROR;
  જો( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, ખાલી);
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_free(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  શૂન્ય *pPrior;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "PRIOR");
    પાછા TCL_ERROR;
  }
  જો( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  sqlite3_free(pPrior);
  પાછા TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
પૂર્ણાંક sqlite3TestHexToBin(અચલ અક્ષર *, પૂર્ણાંક, અક્ષર *);
પૂર્ણાંક sqlite3TestBinToHex(અક્ષર*,પૂર્ણાંક);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memset(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  શૂન્ય *p;
  પૂર્ણાંક size, i;
  Tcl_Size n;
  અક્ષર *zHex;
  અક્ષર *zOut;
  અક્ષર zBin[100];

  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    પાછા TCL_ERROR;
  }
  જો( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    પાછા TCL_ERROR;
  }
  જો( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  જો( n>કદ(zBin)*2 ) n = કદ(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (પૂર્ણાંક)n, zBin);
  જો( n==0 ){
    Tcl_AppendResult(interp, "no data", (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  zOut = p;
  માટે(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  પાછા TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memget(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  શૂન્ય *p;
  પૂર્ણાંક size, n;
  અક્ષર *zBin;
  અક્ષર zHex[100];

  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "ADDRESS SIZE");
    પાછા TCL_ERROR;
  }
  જો( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    પાછા TCL_ERROR;
  }
  જો( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  zBin = p;
  જ્યારે( size>0 ){
    જો( size>(કદ(zHex)-1)/2 ){
      n = (કદ(zHex)-1)/2;
    }નહીંતર{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (અક્ષર*)0);
  }
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memory_used(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memory_highwater(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક resetFlag = 0;
  જો( objc!=1 && objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "?RESET?");
    પાછા TCL_ERROR;
  }
  જો( objc==2 ){
    જો( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) પાછા TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memdebug_backtrace(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક depth;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DEPT");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &depth) ) પાછા TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    બાહ્ય શૂન્ય sqlite3MemdebugBacktrace(પૂર્ણાંક);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memdebug_dump(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "FILENAME");
    પાછા TCL_ERROR;
  }
#જો defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    બાહ્ય શૂન્ય sqlite3MemdebugDump(અચલ અક્ષર*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memdebug_malloc_ગણતરી(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક nMalloc = -1;
  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }
#જો defined(SQLITE_MEMDEBUG)
  {
    બાહ્ય પૂર્ણાંક sqlite3_Memdebug_Malloc_ગણતરી();
    nMalloc = sqlite3_Memdebug_Malloc_ગણતરી();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memdebug_fail(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક ii;
  પૂર્ણાંક iFail;
  પૂર્ણાંક nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  પૂર્ણાંક nBenign;
  પૂર્ણાંક nFail = 0;

  જો( objc<2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) પાછા TCL_ERROR;

  માટે(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    અક્ષર *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    અક્ષર *zErr = 0;

    જો( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      જો( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }નહીંતર{
        જો( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          પાછા TCL_ERROR;
        }
      }
    }નહીંતર જો( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      જો( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }નહીંતર{
        pBenignCnt = objv[ii+1];
      }
    }નહીંતર{
      zErr = "unknown option: ";
    }

    જો( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, ખાલી);
      પાછા TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  જો( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memdebug_pending(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક nPending;
  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  પાછા TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
સ્થિર પૂર્ણાંક sqlite3_memdebug_title_ગણતરી = 0;

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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memdebug_settitle(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_ગણતરી++;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "TITLE");
    પાછા TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    અચલ અક્ષર *zTitle;
    બાહ્ય પૂર્ણાંક sqlite3MemdebugSettitle(અચલ અક્ષર*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  પાછા TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((કદ(પૂર્ણાંક)>=કદ(શૂન્ય*)) ? 1 : કદ(શૂન્ય*)/કદ(પૂર્ણાંક))   \
)
સ્થિર Tcl_HashTable aMallocLog;
સ્થિર પૂર્ણાંક mallocLogEnabled = 0;

પ્રકારવ્યાખ્યા સંરચના MallocLog MallocLog;
સંરચના MallocLog {
  પૂર્ણાંક nCall;
  પૂર્ણાંક nByte;
};

#ifdef SQLITE_MEMDEBUG
સ્થિર શૂન્ય test_memdebug_callback(પૂર્ણાંક nByte, પૂર્ણાંક nFrame, શૂન્ય **aFrame){
  જો( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    પૂર્ણાંક isNew;

    પૂર્ણાંક a_ચાવી[MALLOC_LOG_KEYINTS];
    unsigned પૂર્ણાંક n_ચાવી = કદ(પૂર્ણાંક)*MALLOC_LOG_KEYINTS;

    memset(a_ચાવી, 0, n_ચાવી);
    જો( (કદ(શૂન્ય*)*nFrame)<n_ચાવી ){
      n_ચાવી = nFrame*કદ(શૂન્ય*);
    }
    memcpy(a_ચાવી, aFrame, n_ચાવી);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (અચલ અક્ષર *)a_ચાવી, &isNew);
    જો( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(કદ(MallocLog));
      memset(pLog, 0, કદ(MallocLog));
      Tcl_Set_Hash_કિંમત(pEntry, (ClientData)pLog);
    }નહીંતર{
      pLog = (MallocLog *)Tcl_Get_Hash_કિંમત(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

સ્થિર શૂન્ય test_memdebug_log_clear(શૂન્ય){
  Tcl_Hash_શોધ શોધ;
  Tcl_HashEntry *pEntry;
  માટે(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &શોધ);
    pEntry;
    pEntry=Tcl_NextHashEntry(&શોધ)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_કિંમત(pEntry);
    Tcl_Free((અક્ષર *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_memdebug_log(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  સ્થિર પૂર્ણાંક isInit = 0;
  પૂર્ણાંક iSub;

  સ્થિર અચલ અક્ષર *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  ગણના MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  જો( !isInit ){
#ifdef SQLITE_MEMDEBUG
    બાહ્ય શૂન્ય sqlite3MemdebugBacktraceCallback(
        શૂન્ય (*xBacktrace)(પૂર્ણાંક, પૂર્ણાંક, શૂન્ય **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  જો( objc<2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  જો( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    પાછા TCL_ERROR;
  }

  પસંદગી( (ગણના MB_enum)iSub ){
    સ્થિતિ MB_LOG_START:
      mallocLogEnabled = 1;
      તોડો;
    સ્થિતિ MB_LOG_STOP:
      mallocLogEnabled = 0;
      તોડો;
    સ્થિતિ MB_LOG_DUMP: {
      Tcl_Hash_શોધ શોધ;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      પ્રતિજ્ઞા(કદ(Tcl_WideInt)>=કદ(શૂન્ય*));

      માટે(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &શોધ);
        pEntry;
        pEntry=Tcl_NextHashEntry(&શોધ)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_કિંમત(pEntry);
        Tcl_WideInt *a_ચાવી = (Tcl_WideInt *)Tcl_Get_Hash_ચાવી(&aMallocLog, pEntry);
        પૂર્ણાંક ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        માટે(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_ચાવી[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      તોડો;
    }
    સ્થિતિ MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      તોડો;
    }

    સ્થિતિ MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      બાહ્ય શૂન્ય sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      તોડો;
    }
  }

  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_pagecache(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક sz, N;
  Tcl_Obj *pRes;
  સ્થિર અક્ષર *buf = 0;
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "SIZE N");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &sz) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[2], &N) ) પાછા TCL_ERROR;
  મુક્ત(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  જો( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (શૂન્ય*)0, 0, 0);
  }નહીંતર{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  પાછા TCL_OK;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_alt_pcache(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક installFlag;
  પૂર્ણાંક discardChance = 0;
  પૂર્ણાંક prngSeed = 0;
  પૂર્ણાંક ઉપલું_Stress = 0;
  બાહ્ય શૂન્ય installTestPCache(પૂર્ણાંક,unsigned,unsigned,unsigned);
  જો( objc<2 || objc>5 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) પાછા TCL_ERROR;
  જો( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     પાછા TCL_ERROR;
  }
  જો( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     પાછા TCL_ERROR;
  }
  જો( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &ઉપલું_Stress) ){
    પાછા TCL_ERROR;
  }
  જો( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)ઉપલું_Stress);
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_memstatus(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક enable, rc;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "BOOLEAN");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) પાછા TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_lookaside(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક sz, cnt;
  Tcl_Obj *pRet;
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "SIZE COUNT");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &sz) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) પાછા TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  પાછા TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_db_config_lookaside(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc;
  પૂર્ણાંક sz, cnt;
  sqlite3 *db;
  પૂર્ણાંક bufid;
  સ્થિર અક્ષર azBuf[2][10000];
  બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp*, અચલ અક્ષર*, sqlite3**);
  જો( objc!=5 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "BUFID SIZE COUNT");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[3], &sz) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) પાછા TCL_ERROR;
  જો( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (શૂન્ય*)0, sz, cnt);
  }નહીંતર જો( bufid>=1 && bufid<=2 && sz*cnt<=કદ(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }નહીંતર{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (અક્ષર*)0);
    પાછા TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_heap(
  શૂન્ય * clientData, 
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  સ્થિર અક્ષર *zBuf; /* Use this memory */
  પૂર્ણાંક nByte;         /* Size of buffer to pass to sqlite3_config() */
  પૂર્ણાંક nMinAlloc;     /* Size of minimum allocation */
  પૂર્ણાંક rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  પૂર્ણાંક nArg = objc-1;

  જો( nArg!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "NBYTE NMINALLOC");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) પાછા TCL_ERROR;
  જો( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) પાછા TCL_ERROR;

  જો( nByte==0 ){
    મુક્ત( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (શૂન્ય*)0, 0, 0);
  }નહીંતર{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_heap_size(
  શૂન્ય * clientData, 
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક nByte;         /* Size to pass to sqlite3_config() */
  પૂર્ણાંક rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  પૂર્ણાંક nArg = objc-1;

  જો( nArg!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "NBYTE");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) પાછા TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_error(
  શૂન્ય * clientData, 
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp*, અચલ અક્ષર*, sqlite3**);

  જો( objc!=2 && objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "[DB]");
    પાછા TCL_ERROR;
  }
  જો( objc==2 ){
    જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
    જો( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
  }નહીંતર{
    જો( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
  }
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_uri(
  શૂન્ય * clientData, 
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc;
  પૂર્ણાંક bOpenUri;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "BOOL");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    પાછા TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);

  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_cis(
  શૂન્ય * clientData, 
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc;
  પૂર્ણાંક bUseCis;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "BOOL");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    પાછા TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);

  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_config_pmasz(
  શૂન્ય * clientData, 
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc;
  પૂર્ણાંક iPmaSz;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "BOOL");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    પાછા TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);

  પાછા TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_dump_memsys3(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "FILENAME");
    પાછા TCL_ERROR;
  }

  પસંદગી( SQLITE_PTR_TO_INT(clientData) ){
    સ્થિતિ 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      બાહ્ય શૂન્ય sqlite3Memsys3Dump(અચલ અક્ષર*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      તોડો;
#endif
    }
    સ્થિતિ 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      બાહ્ય શૂન્ય sqlite3Memsys5Dump(અચલ અક્ષર*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      તોડો;
#endif
    }
  }
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_status(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc, i_કિંમત, mx_કિંમત;
  પૂર્ણાંક i, op = 0, resetFlag;
  અચલ અક્ષર *zOpName;
  સ્થિર અચલ સંરચના {
    અચલ અક્ષર *zName;
    પૂર્ણાંક op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_ઢગલો        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_ગણતરી        },
  };
  Tcl_Obj *pResult;
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    પાછા TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  માટે(i=0; i<ArraySize(aOp); i++){
    જો( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      તોડો;
    }
  }
  જો( i>=ArraySize(aOp) ){
    જો( Tcl_GetIntFromObj(interp, objv[1], &op) ) પાછા TCL_ERROR;
  }
  જો( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) પાછા TCL_ERROR;
  i_કિંમત = 0;
  mx_કિંમત = 0;
  rc = sqlite3_status(op, &i_કિંમત, &mx_કિંમત, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_કિંમત));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_કિંમત));
  Tcl_SetObjResult(interp, pResult);
  પાછા TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_db_status(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc, i_કિંમત, mx_કિંમત;
  પૂર્ણાંક i, op = 0, resetFlag;
  અચલ અક્ષર *zOpName;
  sqlite3 *db;
  બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp*, અચલ અક્ષર*, sqlite3**);
  સ્થિર અચલ સંરચના {
    અચલ અક્ષર *zName;
    પૂર્ણાંક op;
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
  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  જો( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  જો( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  માટે(i=0; i<ArraySize(aOp); i++){
    જો( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      તોડો;
    }
  }
  જો( i>=ArraySize(aOp) ){
    જો( Tcl_GetIntFromObj(interp, objv[2], &op) ) પાછા TCL_ERROR;
  }
  જો( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) પાછા TCL_ERROR;
  i_કિંમત = 0;
  mx_કિંમત = 0;
  rc = sqlite3_db_status(db, op, &i_કિંમત, &mx_કિંમત, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_કિંમત));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_કિંમત));
  Tcl_SetObjResult(interp, pResult);
  પાછા TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_install_malloc_faultsim(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc;
  પૂર્ણાંક isInstall;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "BOOLEAN");
    પાછા TCL_ERROR;
  }
  જો( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    પાછા TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  પાછા TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_install_memsys3(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  અચલ sqlite3_mem_methods *sqlite3MemGetMemsys3(શૂન્ય);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_VOLATILE);
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_vfs_oom_test(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  બાહ્ય પૂર્ણાંક sqlite3_memdebug_vfs_oom_test;
  જો( objc>2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "?INTEGER?");
    પાછા TCL_ERROR;
  }નહીંતર જો( objc==2 ){
    પૂર્ણાંક iNew;
    જો( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) પાછા TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  પાછા TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest_malloc_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     પૂર્ણાંક clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_ગણતરી ,0 },
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
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  પાછા TCL_OK;
}
#endif
