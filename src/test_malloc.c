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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
स्थिर संरचना MemFault {
  पूर्णाङ्क iCountdown;         /* Number of pending successes before a failure */
  पूर्णाङ्क nRepeat;            /* Number of times to repeat the failure */
  पूर्णाङ्क nBenign;            /* Number of benign failures seen since last config */
  पूर्णाङ्क nFail;              /* Number of failures seen since last config */
  पूर्णाङ्क nOkBefore;          /* Successful allocations prior to the first fault */
  पूर्णाङ्क nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  पूर्णाङ्क isInstalled;        /* True if the fault simulation layer is installed */
  पूर्णाङ्क isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
स्थिर शून्य sqlite3Fault(शून्य){
  स्थिर u64 cnt = 0;
  cnt++;
  सचे( cnt>((u64)1<<63) ) abort();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
स्थिर शून्य sqlite3FirstFault(शून्य){
  स्थिर u64 cnt2 = 0;
  cnt2++;
  सचे( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
स्थिर पूर्णाङ्क faultsimStep(शून्य){
  सचे( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    पटिनिवत्त 0;
  }
  सचे( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    पटिनिवत्त 0;
  }
  सचे( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  सचे( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  सचे( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  पटिनिवत्त 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
स्थिर शून्य *faultsimMalloc(पूर्णाङ्क n){
  शून्य *p = 0;
  सचे( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  पटिनिवत्त p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
स्थिर शून्य *faultsimRealloc(शून्य *pOld, पूर्णाङ्क n){
  शून्य *p = 0;
  सचे( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  पटिनिवत्त p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
स्थिर शून्य faultsimConfig(पूर्णाङ्क nDelay, पूर्णाङ्क nRepeat){
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
स्थिर पूर्णाङ्क faultsimFailures(शून्य){
  पटिनिवत्त memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
स्थिर पूर्णाङ्क faultsimBenignFailures(शून्य){
  पटिनिवत्त memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
स्थिर पूर्णाङ्क faultsimPending(शून्य){
  सचे( memfault.enable ){
    पटिनिवत्त memfault.iCountdown;
  }अञ्ञथा{
    पटिनिवत्त -1;
  }
}


स्थिर शून्य faultsimBeginBenign(शून्य){
  memfault.isBenignMode++;
}
स्थिर शून्य faultsimEndBenign(शून्य){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
स्थिर पूर्णाङ्क faultsimInstall(पूर्णाङ्क install){
  पूर्णाङ्क rc;

  install = (install ? 1 : 0);
  प्रतिज्ञा(memfault.isInstalled==1 || memfault.isInstalled==0);

  सचे( install==memfault.isInstalled ){
    पटिनिवत्त SQLITE_ERROR;
  }

  सचे( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    प्रतिज्ञा(memfault.m.xMalloc);
    सचे( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }अञ्ञथा{
    sqlite3_mem_methods m2;
    प्रतिज्ञा(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    memset(&m2, 0, परिमाण(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    प्रतिज्ञा( memcmp(&m2, &memfault.m, परिमाण(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (शून्य*)0, (शून्य*)0);
  }

  सचे( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  पटिनिवत्त rc;
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
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

/*
** Transform pointers to text and back again
*/
स्थिर शून्य pointerToText(शून्य *p, वर्ण *z){
  ध्रुव वर्ण *zRet = sqlite3TestPtrToText(p);
  sprintf(z, "%s", zRet);
}
स्थिर पूर्णाङ्क textToPointer(ध्रुव वर्ण *z, शून्य **pp){
  *pp = sqlite3TestTextToPtr(z);
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_malloc(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nByte;
  शून्य *p;
  वर्ण zOut[100];
  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "NBYTES");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) पटिनिवत्त TCL_ERROR;
  p = sqlite3_malloc((unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, रिक्त);
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_realloc(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nByte;
  शून्य *pPrior, *p;
  वर्ण zOut[100];
  सचे( objc!=3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "PRIOR NBYTES");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) पटिनिवत्त TCL_ERROR;
  सचे( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (unsigned)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, रिक्त);
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_free(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *pPrior;
  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "PRIOR");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  sqlite3_free(pPrior);
  पटिनिवत्त TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
पूर्णाङ्क sqlite3TestHexToBin(ध्रुव वर्ण *, पूर्णाङ्क, वर्ण *);
पूर्णाङ्क sqlite3TestBinToHex(वर्ण*,पूर्णाङ्क);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memset(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *p;
  पूर्णाङ्क size, i;
  Tcl_Size n;
  वर्ण *zHex;
  वर्ण *zOut;
  वर्ण zBin[100];

  सचे( objc!=4 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    पटिनिवत्त TCL_ERROR;
  }
  सचे( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  सचे( n>परिमाण(zBin)*2 ) n = परिमाण(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (पूर्णाङ्क)n, zBin);
  सचे( n==0 ){
    Tcl_AppendResult(interp, "no data", (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  zOut = p;
  अत्थाय(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memget(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *p;
  पूर्णाङ्क size, n;
  वर्ण *zBin;
  वर्ण zHex[100];

  सचे( objc!=3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "ADDRESS SIZE");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    पटिनिवत्त TCL_ERROR;
  }
  सचे( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  zBin = p;
  याव( size>0 ){
    सचे( size>(परिमाण(zHex)-1)/2 ){
      n = (परिमाण(zHex)-1)/2;
    }अञ्ञथा{
      n = size;
    }
    memcpy(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (वर्ण*)0);
  }
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memory_used(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memory_highwater(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क resetFlag = 0;
  सचे( objc!=1 && objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "?RESET?");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( objc==2 ){
    सचे( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) पटिनिवत्त TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_backtrace(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क depth;
  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "DEPT");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[1], &depth) ) पटिनिवत्त TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    बाह्य शून्य sqlite3MemdebugBacktrace(पूर्णाङ्क);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_dump(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "FILENAME");
    पटिनिवत्त TCL_ERROR;
  }
#सचे defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    बाह्य शून्य sqlite3MemdebugDump(ध्रुव वर्ण*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_malloc_गणनम्(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nMalloc = -1;
  सचे( objc!=1 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "");
    पटिनिवत्त TCL_ERROR;
  }
#सचे defined(SQLITE_MEMDEBUG)
  {
    बाह्य पूर्णाङ्क sqlite3_Memdebug_Malloc_गणनम्();
    nMalloc = sqlite3_Memdebug_Malloc_गणनम्();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  पटिनिवत्त TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_fail(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क ii;
  पूर्णाङ्क iFail;
  पूर्णाङ्क nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  पूर्णाङ्क nBenign;
  पूर्णाङ्क nFail = 0;

  सचे( objc<2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) पटिनिवत्त TCL_ERROR;

  अत्थाय(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    वर्ण *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    वर्ण *zErr = 0;

    सचे( nOption>1 && strncmp(zOption, "-repeat", nOption)==0 ){
      सचे( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अञ्ञथा{
        सचे( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          पटिनिवत्त TCL_ERROR;
        }
      }
    }अञ्ञथा सचे( nOption>1 && strncmp(zOption, "-benigncnt", nOption)==0 ){
      सचे( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अञ्ञथा{
        pBenignCnt = objv[ii+1];
      }
    }अञ्ञथा{
      zErr = "unknown option: ";
    }

    सचे( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, रिक्त);
      पटिनिवत्त TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  सचे( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_pending(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nPending;
  सचे( objc!=1 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "");
    पटिनिवत्त TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  पटिनिवत्त TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
स्थिर पूर्णाङ्क sqlite3_memdebug_title_गणनम् = 0;

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
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_settitle(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_गणनम्++;
  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "TITLE");
    पटिनिवत्त TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    ध्रुव वर्ण *zTitle;
    बाह्य पूर्णाङ्क sqlite3MemdebugSettitle(ध्रुव वर्ण*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  पटिनिवत्त TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((परिमाण(पूर्णाङ्क)>=परिमाण(शून्य*)) ? 1 : परिमाण(शून्य*)/परिमाण(पूर्णाङ्क))   \
)
स्थिर Tcl_HashTable aMallocLog;
स्थिर पूर्णाङ्क mallocLogEnabled = 0;

प्रकारनिर्देश संरचना MallocLog MallocLog;
संरचना MallocLog {
  पूर्णाङ्क nCall;
  पूर्णाङ्क nByte;
};

#ifdef SQLITE_MEMDEBUG
स्थिर शून्य test_memdebug_callback(पूर्णाङ्क nByte, पूर्णाङ्क nFrame, शून्य **aFrame){
  सचे( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    पूर्णाङ्क isNew;

    पूर्णाङ्क a_कुञ्चिका[MALLOC_LOG_KEYINTS];
    unsigned पूर्णाङ्क n_कुञ्चिका = परिमाण(पूर्णाङ्क)*MALLOC_LOG_KEYINTS;

    memset(a_कुञ्चिका, 0, n_कुञ्चिका);
    सचे( (परिमाण(शून्य*)*nFrame)<n_कुञ्चिका ){
      n_कुञ्चिका = nFrame*परिमाण(शून्य*);
    }
    memcpy(a_कुञ्चिका, aFrame, n_कुञ्चिका);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (ध्रुव वर्ण *)a_कुञ्चिका, &isNew);
    सचे( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(परिमाण(MallocLog));
      memset(pLog, 0, परिमाण(MallocLog));
      Tcl_Set_Hash_मानम्(pEntry, (ClientData)pLog);
    }अञ्ञथा{
      pLog = (MallocLog *)Tcl_Get_Hash_मानम्(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

स्थिर शून्य test_memdebug_log_clear(शून्य){
  Tcl_Hash_अन्वेषणम् अन्वेषणम्;
  Tcl_HashEntry *pEntry;
  अत्थाय(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &अन्वेषणम्);
    pEntry;
    pEntry=Tcl_NextHashEntry(&अन्वेषणम्)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मानम्(pEntry);
    Tcl_Free((वर्ण *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_log(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर पूर्णाङ्क isInit = 0;
  पूर्णाङ्क iSub;

  स्थिर ध्रुव वर्ण *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  गणना MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  सचे( !isInit ){
#ifdef SQLITE_MEMDEBUG
    बाह्य शून्य sqlite3MemdebugBacktraceCallback(
        शून्य (*xBacktrace)(पूर्णाङ्क, पूर्णाङ्क, शून्य **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  सचे( objc<2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  सचे( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    पटिनिवत्त TCL_ERROR;
  }

  विकल्प( (गणना MB_enum)iSub ){
    पक्ष MB_LOG_START:
      mallocLogEnabled = 1;
      भङ्ग;
    पक्ष MB_LOG_STOP:
      mallocLogEnabled = 0;
      भङ्ग;
    पक्ष MB_LOG_DUMP: {
      Tcl_Hash_अन्वेषणम् अन्वेषणम्;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      प्रतिज्ञा(परिमाण(Tcl_WideInt)>=परिमाण(शून्य*));

      अत्थाय(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &अन्वेषणम्);
        pEntry;
        pEntry=Tcl_NextHashEntry(&अन्वेषणम्)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मानम्(pEntry);
        Tcl_WideInt *a_कुञ्चिका = (Tcl_WideInt *)Tcl_Get_Hash_कुञ्चिका(&aMallocLog, pEntry);
        पूर्णाङ्क ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        अत्थाय(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_कुञ्चिका[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      भङ्ग;
    }
    पक्ष MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      भङ्ग;
    }

    पक्ष MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      बाह्य शून्य sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      भङ्ग;
    }
  }

  पटिनिवत्त TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_pagecache(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क sz, N;
  Tcl_Obj *pRes;
  स्थिर वर्ण *buf = 0;
  सचे( objc!=3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "SIZE N");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[1], &sz) ) पटिनिवत्त TCL_ERROR;
  सचे( Tcl_GetIntFromObj(interp, objv[2], &N) ) पटिनिवत्त TCL_ERROR;
  मुक्त(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  सचे( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (शून्य*)0, 0, 0);
  }अञ्ञथा{
    buf = malloc( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  पटिनिवत्त TCL_OK;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_alt_pcache(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क installFlag;
  पूर्णाङ्क discardChance = 0;
  पूर्णाङ्क prngSeed = 0;
  पूर्णाङ्क उच्चः_Stress = 0;
  बाह्य शून्य installTestPCache(पूर्णाङ्क,unsigned,unsigned,unsigned);
  सचे( objc<2 || objc>5 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) पटिनिवत्त TCL_ERROR;
  सचे( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     पटिनिवत्त TCL_ERROR;
  }
  सचे( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     पटिनिवत्त TCL_ERROR;
  }
  सचे( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &उच्चः_Stress) ){
    पटिनिवत्त TCL_ERROR;
  }
  सचे( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  installTestPCache(installFlag, (unsigned)discardChance, (unsigned)prngSeed,
                    (unsigned)उच्चः_Stress);
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_memstatus(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क enable, rc;
  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "BOOLEAN");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) पटिनिवत्त TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_lookaside(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क sz, cnt;
  Tcl_Obj *pRet;
  सचे( objc!=3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "SIZE COUNT");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[1], &sz) ) पटिनिवत्त TCL_ERROR;
  सचे( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) पटिनिवत्त TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  पटिनिवत्त TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_db_config_lookaside(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क sz, cnt;
  sqlite3 *db;
  पूर्णाङ्क bufid;
  स्थिर वर्ण azBuf[2][10000];
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, ध्रुव वर्ण*, sqlite3**);
  सचे( objc!=5 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "BUFID SIZE COUNT");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) पटिनिवत्त TCL_ERROR;
  सचे( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) पटिनिवत्त TCL_ERROR;
  सचे( Tcl_GetIntFromObj(interp, objv[3], &sz) ) पटिनिवत्त TCL_ERROR;
  सचे( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) पटिनिवत्त TCL_ERROR;
  सचे( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (शून्य*)0, sz, cnt);
  }अञ्ञथा सचे( bufid>=1 && bufid<=2 && sz*cnt<=परिमाण(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }अञ्ञथा{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (वर्ण*)0);
    पटिनिवत्त TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_heap(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर वर्ण *zBuf; /* Use this memory */
  पूर्णाङ्क nByte;         /* Size of buffer to pass to sqlite3_config() */
  पूर्णाङ्क nMinAlloc;     /* Size of minimum allocation */
  पूर्णाङ्क rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  पूर्णाङ्क nArg = objc-1;

  सचे( nArg!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "NBYTE NMINALLOC");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) पटिनिवत्त TCL_ERROR;
  सचे( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) पटिनिवत्त TCL_ERROR;

  सचे( nByte==0 ){
    मुक्त( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (शून्य*)0, 0, 0);
  }अञ्ञथा{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_heap_size(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nByte;         /* Size to pass to sqlite3_config() */
  पूर्णाङ्क rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  पूर्णाङ्क nArg = objc-1;

  सचे( nArg!=1 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "NBYTE");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) पटिनिवत्त TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_error(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, ध्रुव वर्ण*, sqlite3**);

  सचे( objc!=2 && objc!=1 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "[DB]");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( objc==2 ){
    सचे( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) पटिनिवत्त TCL_ERROR;
    सचे( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (वर्ण*)0);
      पटिनिवत्त TCL_ERROR;
    }
  }अञ्ञथा{
    सचे( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (वर्ण*)0);
      पटिनिवत्त TCL_ERROR;
    }
  }
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_uri(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क bOpenUri;

  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "BOOL");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    पटिनिवत्त TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_cis(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क bUseCis;

  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "BOOL");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    पटिनिवत्त TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_pmasz(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क iPmaSz;

  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "BOOL");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    पटिनिवत्त TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  पटिनिवत्त TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_dump_memsys3(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "FILENAME");
    पटिनिवत्त TCL_ERROR;
  }

  विकल्प( SQLITE_PTR_TO_INT(clientData) ){
    पक्ष 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      बाह्य शून्य sqlite3Memsys3Dump(ध्रुव वर्ण*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      भङ्ग;
#endif
    }
    पक्ष 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      बाह्य शून्य sqlite3Memsys5Dump(ध्रुव वर्ण*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      भङ्ग;
#endif
    }
  }
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_status(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc, i_मानम्, mx_मानम्;
  पूर्णाङ्क i, op = 0, resetFlag;
  ध्रुव वर्ण *zOpName;
  स्थिर ध्रुव संरचना {
    ध्रुव वर्ण *zName;
    पूर्णाङ्क op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_राशिः        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_गणनम्        },
  };
  Tcl_Obj *pResult;
  सचे( objc!=3 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    पटिनिवत्त TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  अत्थाय(i=0; i<ArraySize(aOp); i++){
    सचे( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      भङ्ग;
    }
  }
  सचे( i>=ArraySize(aOp) ){
    सचे( Tcl_GetIntFromObj(interp, objv[1], &op) ) पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) पटिनिवत्त TCL_ERROR;
  i_मानम् = 0;
  mx_मानम् = 0;
  rc = sqlite3_status(op, &i_मानम्, &mx_मानम्, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_मानम्));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_मानम्));
  Tcl_SetObjResult(interp, pResult);
  पटिनिवत्त TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_db_status(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc, i_मानम्, mx_मानम्;
  पूर्णाङ्क i, op = 0, resetFlag;
  ध्रुव वर्ण *zOpName;
  sqlite3 *db;
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, ध्रुव वर्ण*, sqlite3**);
  स्थिर ध्रुव संरचना {
    ध्रुव वर्ण *zName;
    पूर्णाङ्क op;
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
  सचे( objc!=4 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) पटिनिवत्त TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  सचे( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  सचे( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  अत्थाय(i=0; i<ArraySize(aOp); i++){
    सचे( strcmp(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      भङ्ग;
    }
  }
  सचे( i>=ArraySize(aOp) ){
    सचे( Tcl_GetIntFromObj(interp, objv[2], &op) ) पटिनिवत्त TCL_ERROR;
  }
  सचे( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) पटिनिवत्त TCL_ERROR;
  i_मानम् = 0;
  mx_मानम् = 0;
  rc = sqlite3_db_status(db, op, &i_मानम्, &mx_मानम्, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_मानम्));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_मानम्));
  Tcl_SetObjResult(interp, pResult);
  पटिनिवत्त TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_install_malloc_faultsim(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क isInstall;

  सचे( objc!=2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "BOOLEAN");
    पटिनिवत्त TCL_ERROR;
  }
  सचे( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    पटिनिवत्त TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  पटिनिवत्त TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_install_memsys3(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  ध्रुव sqlite3_mem_methods *sqlite3MemGetMemsys3(शून्य);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  पटिनिवत्त TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfs_oom_test(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णाङ्क sqlite3_memdebug_vfs_oom_test;
  सचे( objc>2 ){
    Tcl_Wrong_अङ्कः_Args(interp, 1, objv, "?INTEGER?");
    पटिनिवत्त TCL_ERROR;
  }अञ्ञथा सचे( objc==2 ){
    पूर्णाङ्क iNew;
    सचे( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) पटिनिवत्त TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  पटिनिवत्त TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest_malloc_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     पूर्णाङ्क clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_गणनम् ,0 },
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
  पूर्णाङ्क i;
  अत्थाय(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  पटिनिवत्त TCL_OK;
}
#endif
