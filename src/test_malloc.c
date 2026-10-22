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
  पूर्णांक iCountdown;         /* Number of pending successes before a failure */
  पूर्णांक nRepeat;            /* Number of times to repeat the failure */
  पूर्णांक nBenign;            /* Number of benign failures seen since last config */
  पूर्णांक nFail;              /* Number of failures seen since last config */
  पूर्णांक nOkBefore;          /* Successful allocations prior to the first fault */
  पूर्णांक nOkAfter;           /* Successful allocations after a fault */
  u8 enable;              /* True if enabled */
  पूर्णांक isInstalled;        /* True if the fault simulation layer is installed */
  पूर्णांक isBenignMode;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
स्थिर शून्य sqlite3Fault(शून्य){
  स्थिर u64 cnt = 0;
  cnt++;
  जर( cnt>((u64)1<<63) ) पात();
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
  जर( cnt2>((u64)1<<63) ) पात();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
स्थिर पूर्णांक faultsimStep(शून्य){
  जर( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    परत 0;
  }
  जर( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    परत 0;
  }
  जर( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  जर( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  जर( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  परत 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
स्थिर शून्य *faultsimMalloc(पूर्णांक n){
  शून्य *p = 0;
  जर( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  परत p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
स्थिर शून्य *faultsimRealloc(शून्य *pOld, पूर्णांक n){
  शून्य *p = 0;
  जर( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  परत p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
स्थिर शून्य faultsimConfig(पूर्णांक nDelay, पूर्णांक nRepeat){
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
स्थिर पूर्णांक faultsimFailures(शून्य){
  परत memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
स्थिर पूर्णांक faultsimBenignFailures(शून्य){
  परत memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
स्थिर पूर्णांक faultsimPending(शून्य){
  जर( memfault.enable ){
    परत memfault.iCountdown;
  }अन्यथा{
    परत -1;
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
स्थिर पूर्णांक faultsimInstall(पूर्णांक install){
  पूर्णांक rc;

  install = (install ? 1 : 0);
  प्रतिज्ञा(memfault.isInstalled==1 || memfault.isInstalled==0);

  जर( install==memfault.isInstalled ){
    परत SQLITE_ERROR;
  }

  जर( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    प्रतिज्ञा(memfault.m.xMalloc);
    जर( rc==SQLITE_OK ){
      sqlite3_mem_methods m = memfault.m;
      m.xMalloc = faultsimMalloc;
      m.xRealloc = faultsimRealloc;
      rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &m);
    }
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS, 
        faultsimBeginBenign, faultsimEndBenign
    );
  }अन्यथा{
    sqlite3_mem_methods m2;
    प्रतिज्ञा(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    स_रखो(&m2, 0, आकार(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    प्रतिज्ञा( स_भेद(&m2, &memfault.m, आकार(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (शून्य*)0, (शून्य*)0);
  }

  जर( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  परत rc;
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
बाह्य स्थिरांक अक्षर *sqlite3ErrName(पूर्णांक);

/*
** Transform pointers to text and back again
*/
स्थिर शून्य pointerToText(शून्य *p, अक्षर *z){
  स्थिरांक अक्षर *zRet = sqlite3TestPtrToText(p);
  प्र_लिखो(z, "%s", zRet);
}
स्थिर पूर्णांक textToPointer(स्थिरांक अक्षर *z, शून्य **pp){
  *pp = sqlite3TestTextToPtr(z);
  परत TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_malloc(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nByte;
  शून्य *p;
  अक्षर zOut[100];
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NBYTES");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) परत TCL_ERROR;
  p = sqlite3_malloc((अचिन्हित)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, रिक्त);
  परत TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_realloc(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nByte;
  शून्य *pPrior, *p;
  अक्षर zOut[100];
  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "PRIOR NBYTES");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) परत TCL_ERROR;
  जर( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (अक्षर*)0);
    परत TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (अचिन्हित)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, रिक्त);
  परत TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_free(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *pPrior;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "PRIOR");
    परत TCL_ERROR;
  }
  जर( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (अक्षर*)0);
    परत TCL_ERROR;
  }
  sqlite3_free(pPrior);
  परत TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
पूर्णांक sqlite3TestHexToBin(स्थिरांक अक्षर *, पूर्णांक, अक्षर *);
पूर्णांक sqlite3TestBinToHex(अक्षर*,पूर्णांक);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_memset(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *p;
  पूर्णांक size, i;
  Tcl_Size n;
  अक्षर *zHex;
  अक्षर *zOut;
  अक्षर zBin[100];

  जर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    परत TCL_ERROR;
  }
  जर( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (अक्षर*)0);
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    परत TCL_ERROR;
  }
  जर( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (अक्षर*)0);
    परत TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  जर( n>आकार(zBin)*2 ) n = आकार(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (पूर्णांक)n, zBin);
  जर( n==0 ){
    Tcl_AppendResult(interp, "no data", (अक्षर*)0);
    परत TCL_ERROR;
  }
  zOut = p;
  साठी(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  परत TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_memget(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *p;
  पूर्णांक size, n;
  अक्षर *zBin;
  अक्षर zHex[100];

  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "ADDRESS SIZE");
    परत TCL_ERROR;
  }
  जर( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (अक्षर*)0);
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    परत TCL_ERROR;
  }
  जर( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (अक्षर*)0);
    परत TCL_ERROR;
  }
  zBin = p;
  पर्यंत( size>0 ){
    जर( size>(आकार(zHex)-1)/2 ){
      n = (आकार(zHex)-1)/2;
    }अन्यथा{
      n = size;
    }
    स_नकल(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (अक्षर*)0);
  }
  परत TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_memory_used(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_SetObjResult(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  परत TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_memory_highwater(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक resetFlag = 0;
  जर( objc!=1 && objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?RESET?");
    परत TCL_ERROR;
  }
  जर( objc==2 ){
    जर( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) परत TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  परत TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_memdebug_backtrace(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक depth;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DEPT");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &depth) ) परत TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    बाह्य शून्य sqlite3MemdebugBacktrace(पूर्णांक);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  परत TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_memdebug_dump(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME");
    परत TCL_ERROR;
  }
#जर defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    बाह्य शून्य sqlite3MemdebugDump(स्थिरांक अक्षर*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  परत TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_memdebug_malloc_मोजणी(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nMalloc = -1;
  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }
#जर defined(SQLITE_MEMDEBUG)
  {
    बाह्य पूर्णांक sqlite3_Memdebug_Malloc_मोजणी();
    nMalloc = sqlite3_Memdebug_Malloc_मोजणी();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  परत TCL_OK;
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
स्थिर पूर्णांक SQLITE_TCLAPI test_memdebug_fail(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक ii;
  पूर्णांक iFail;
  पूर्णांक nRepeat = 1;
  Tcl_Obj *pBenignCnt = 0;
  पूर्णांक nBenign;
  पूर्णांक nFail = 0;

  जर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) परत TCL_ERROR;

  साठी(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    अक्षर *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    अक्षर *zErr = 0;

    जर( nOption>1 && म_भेदन(zOption, "-repeat", nOption)==0 ){
      जर( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अन्यथा{
        जर( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          परत TCL_ERROR;
        }
      }
    }अन्यथा जर( nOption>1 && म_भेदन(zOption, "-benigncnt", nOption)==0 ){
      जर( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अन्यथा{
        pBenignCnt = objv[ii+1];
      }
    }अन्यथा{
      zErr = "unknown option: ";
    }

    जर( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, रिक्त);
      परत TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  जर( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  परत TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_memdebug_pending(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nPending;
  जर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    परत TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  परत TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
स्थिर पूर्णांक sqlite3_memdebug_title_मोजणी = 0;

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
स्थिर पूर्णांक SQLITE_TCLAPI test_memdebug_settitle(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_मोजणी++;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "TITLE");
    परत TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    स्थिरांक अक्षर *zTitle;
    बाह्य पूर्णांक sqlite3MemdebugSettitle(स्थिरांक अक्षर*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  परत TCL_OK;
}

#define MALLOC_LOG_FRAMES  10 
#define MALLOC_LOG_KEYINTS (                                              \
    10 * ((आकार(पूर्णांक)>=आकार(शून्य*)) ? 1 : आकार(शून्य*)/आकार(पूर्णांक))   \
)
स्थिर Tcl_HashTable aMallocLog;
स्थिर पूर्णांक mallocLogEnabled = 0;

प्रकारव्याख्या संरचना MallocLog MallocLog;
संरचना MallocLog {
  पूर्णांक nCall;
  पूर्णांक nByte;
};

#ifdef SQLITE_MEMDEBUG
स्थिर शून्य test_memdebug_callback(पूर्णांक nByte, पूर्णांक nFrame, शून्य **aFrame){
  जर( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    पूर्णांक isNew;

    पूर्णांक a_किल्ली[MALLOC_LOG_KEYINTS];
    अचिन्हित पूर्णांक n_किल्ली = आकार(पूर्णांक)*MALLOC_LOG_KEYINTS;

    स_रखो(a_किल्ली, 0, n_किल्ली);
    जर( (आकार(शून्य*)*nFrame)<n_किल्ली ){
      n_किल्ली = nFrame*आकार(शून्य*);
    }
    स_नकल(a_किल्ली, aFrame, n_किल्ली);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (स्थिरांक अक्षर *)a_किल्ली, &isNew);
    जर( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(आकार(MallocLog));
      स_रखो(pLog, 0, आकार(MallocLog));
      Tcl_Set_Hash_मूल्य(pEntry, (ClientData)pLog);
    }अन्यथा{
      pLog = (MallocLog *)Tcl_Get_Hash_मूल्य(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

स्थिर शून्य test_memdebug_log_clear(शून्य){
  Tcl_Hash_शोध शोध;
  Tcl_HashEntry *pEntry;
  साठी(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &शोध);
    pEntry;
    pEntry=Tcl_NextHashEntry(&शोध)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मूल्य(pEntry);
    Tcl_Free((अक्षर *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

स्थिर पूर्णांक SQLITE_TCLAPI test_memdebug_log(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर पूर्णांक isInit = 0;
  पूर्णांक iSub;

  स्थिर स्थिरांक अक्षर *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  गणना MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  जर( !isInit ){
#ifdef SQLITE_MEMDEBUG
    बाह्य शून्य sqlite3MemdebugBacktraceCallback(
        शून्य (*xBacktrace)(पूर्णांक, पूर्णांक, शून्य **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  जर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  जर( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    परत TCL_ERROR;
  }

  निवड( (गणना MB_enum)iSub ){
    स्थिती MB_LOG_START:
      mallocLogEnabled = 1;
      थांबा;
    स्थिती MB_LOG_STOP:
      mallocLogEnabled = 0;
      थांबा;
    स्थिती MB_LOG_DUMP: {
      Tcl_Hash_शोध शोध;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      प्रतिज्ञा(आकार(Tcl_WideInt)>=आकार(शून्य*));

      साठी(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &शोध);
        pEntry;
        pEntry=Tcl_NextHashEntry(&शोध)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मूल्य(pEntry);
        Tcl_WideInt *a_किल्ली = (Tcl_WideInt *)Tcl_Get_Hash_किल्ली(&aMallocLog, pEntry);
        पूर्णांक ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        साठी(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_किल्ली[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      थांबा;
    }
    स्थिती MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      थांबा;
    }

    स्थिती MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      बाह्य शून्य sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      थांबा;
    }
  }

  परत TCL_OK;
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
स्थिर पूर्णांक SQLITE_TCLAPI test_config_pagecache(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक sz, N;
  Tcl_Obj *pRes;
  स्थिर अक्षर *buf = 0;
  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SIZE N");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &sz) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, objv[2], &N) ) परत TCL_ERROR;
  मुक्त(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  जर( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (शून्य*)0, 0, 0);
  }अन्यथा{
    buf = दो_स्मृति( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  परत TCL_OK;
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
स्थिर पूर्णांक SQLITE_TCLAPI test_alt_pcache(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक installFlag;
  पूर्णांक discardChance = 0;
  पूर्णांक prngSeed = 0;
  पूर्णांक वरचा_Stress = 0;
  बाह्य शून्य installTestPCache(पूर्णांक,अचिन्हित,अचिन्हित,अचिन्हित);
  जर( objc<2 || objc>5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) परत TCL_ERROR;
  जर( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     परत TCL_ERROR;
  }
  जर( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     परत TCL_ERROR;
  }
  जर( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &वरचा_Stress) ){
    परत TCL_ERROR;
  }
  जर( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (अक्षर*)0);
    परत TCL_ERROR;
  }
  installTestPCache(installFlag, (अचिन्हित)discardChance, (अचिन्हित)prngSeed,
                    (अचिन्हित)वरचा_Stress);
  परत TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config_memstatus(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक enable, rc;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOLEAN");
    परत TCL_ERROR;
  }
  जर( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) परत TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  परत TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config_lookaside(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक sz, cnt;
  Tcl_Obj *pRet;
  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SIZE COUNT");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &sz) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) परत TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  परत TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_db_config_lookaside(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक sz, cnt;
  sqlite3 *db;
  पूर्णांक bufid;
  स्थिर अक्षर azBuf[2][10000];
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिरांक अक्षर*, sqlite3**);
  जर( objc!=5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BUFID SIZE COUNT");
    परत TCL_ERROR;
  }
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, objv[3], &sz) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) परत TCL_ERROR;
  जर( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (शून्य*)0, sz, cnt);
  }अन्यथा जर( bufid>=1 && bufid<=2 && sz*cnt<=आकार(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }अन्यथा{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (अक्षर*)0);
    परत TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  परत TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config_heap(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zBuf; /* Use this memory */
  पूर्णांक nByte;         /* Size of buffer to pass to sqlite3_config() */
  पूर्णांक nMinAlloc;     /* Size of minimum allocation */
  पूर्णांक rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  पूर्णांक nArg = objc-1;

  जर( nArg!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NBYTE NMINALLOC");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) परत TCL_ERROR;
  जर( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) परत TCL_ERROR;

  जर( nByte==0 ){
    मुक्त( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (शून्य*)0, 0, 0);
  }अन्यथा{
    zBuf = पुनः_स्मृति(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  परत TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config_heap_size(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nByte;         /* Size to pass to sqlite3_config() */
  पूर्णांक rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  पूर्णांक nArg = objc-1;

  जर( nArg!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NBYTE");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) परत TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  परत TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config_error(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिरांक अक्षर*, sqlite3**);

  जर( objc!=2 && objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "[DB]");
    परत TCL_ERROR;
  }
  जर( objc==2 ){
    जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;
    जर( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (अक्षर*)0);
      परत TCL_ERROR;
    }
  }अन्यथा{
    जर( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (अक्षर*)0);
      परत TCL_ERROR;
    }
  }
  परत TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config_uri(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक bOpenUri;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOL");
    परत TCL_ERROR;
  }
  जर( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    परत TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);

  परत TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config_cis(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक bUseCis;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOL");
    परत TCL_ERROR;
  }
  जर( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    परत TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);

  परत TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_config_pmasz(
  शून्य * clientData, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक iPmaSz;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOL");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    परत TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);

  परत TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_dump_memsys3(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME");
    परत TCL_ERROR;
  }

  निवड( SQLITE_PTR_TO_INT(clientData) ){
    स्थिती 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      बाह्य शून्य sqlite3Memsys3Dump(स्थिरांक अक्षर*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      थांबा;
#endif
    }
    स्थिती 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      बाह्य शून्य sqlite3Memsys5Dump(स्थिरांक अक्षर*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      थांबा;
#endif
    }
  }
  परत TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_status(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc, i_मूल्य, mx_मूल्य;
  पूर्णांक i, op = 0, resetFlag;
  स्थिरांक अक्षर *zOpName;
  स्थिर स्थिरांक संरचना {
    स्थिरांक अक्षर *zName;
    पूर्णांक op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_SIZE      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_SIZE        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_चळत        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_मोजणी        },
  };
  Tcl_Obj *pResult;
  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    परत TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  साठी(i=0; i<ArraySize(aOp); i++){
    जर( म_भेद(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      थांबा;
    }
  }
  जर( i>=ArraySize(aOp) ){
    जर( Tcl_GetIntFromObj(interp, objv[1], &op) ) परत TCL_ERROR;
  }
  जर( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) परत TCL_ERROR;
  i_मूल्य = 0;
  mx_मूल्य = 0;
  rc = sqlite3_status(op, &i_मूल्य, &mx_मूल्य, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_मूल्य));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_मूल्य));
  Tcl_SetObjResult(interp, pResult);
  परत TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_db_status(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc, i_मूल्य, mx_मूल्य;
  पूर्णांक i, op = 0, resetFlag;
  स्थिरांक अक्षर *zOpName;
  sqlite3 *db;
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिरांक अक्षर*, sqlite3**);
  स्थिर स्थिरांक संरचना {
    स्थिरांक अक्षर *zName;
    पूर्णांक op;
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
  जर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    परत TCL_ERROR;
  }
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  जर( स_भेद(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  जर( स_भेद(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  साठी(i=0; i<ArraySize(aOp); i++){
    जर( म_भेद(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      थांबा;
    }
  }
  जर( i>=ArraySize(aOp) ){
    जर( Tcl_GetIntFromObj(interp, objv[2], &op) ) परत TCL_ERROR;
  }
  जर( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) परत TCL_ERROR;
  i_मूल्य = 0;
  mx_मूल्य = 0;
  rc = sqlite3_db_status(db, op, &i_मूल्य, &mx_मूल्य, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_मूल्य));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_मूल्य));
  Tcl_SetObjResult(interp, pResult);
  परत TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_install_malloc_faultsim(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक isInstall;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOLEAN");
    परत TCL_ERROR;
  }
  जर( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    परत TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  परत TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_install_memsys3(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = SQLITE_MISUSE;
#ifdef SQLITE_ENABLE_MEMSYS3
  स्थिरांक sqlite3_mem_methods *sqlite3MemGetMemsys3(शून्य);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_vfs_oom_test(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णांक sqlite3_memdebug_vfs_oom_test;
  जर( objc>2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?INTEGER?");
    परत TCL_ERROR;
  }अन्यथा जर( objc==2 ){
    पूर्णांक iNew;
    जर( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) परत TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  परत TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_malloc_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     पूर्णांक clientData;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_मोजणी ,0 },
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
  पूर्णांक i;
  साठी(i=0; i<आकार(aObjCmd)/आकार(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  परत TCL_OK;
}
#endif
