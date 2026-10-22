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
  यदि( cnt>((u64)1<<63) ) पात();
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
  यदि( cnt2>((u64)1<<63) ) पात();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
स्थिर पूर्णाङ्क faultsimStep(शून्य){
  यदि( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    फर्काऊ 0;
  }
  यदि( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    फर्काऊ 0;
  }
  यदि( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  यदि( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  यदि( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  फर्काऊ 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
स्थिर शून्य *faultsimMalloc(पूर्णाङ्क n){
  शून्य *p = 0;
  यदि( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  फर्काऊ p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
स्थिर शून्य *faultsimRealloc(शून्य *pOld, पूर्णाङ्क n){
  शून्य *p = 0;
  यदि( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  फर्काऊ p;
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
  फर्काऊ memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
स्थिर पूर्णाङ्क faultsimBenignFailures(शून्य){
  फर्काऊ memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
स्थिर पूर्णाङ्क faultsimPending(शून्य){
  यदि( memfault.enable ){
    फर्काऊ memfault.iCountdown;
  }अन्यथा{
    फर्काऊ -1;
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

  यदि( install==memfault.isInstalled ){
    फर्काऊ SQLITE_ERROR;
  }

  यदि( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    प्रतिज्ञा(memfault.m.xMalloc);
    यदि( rc==SQLITE_OK ){
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
    स_रखो(&m2, 0, परिमाण(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    प्रतिज्ञा( स_भेद(&m2, &memfault.m, परिमाण(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (शून्य*)0, (शून्य*)0);
  }

  यदि( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  फर्काऊ rc;
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
बाह्य अचर वर्ण *sqlite3ErrName(पूर्णाङ्क);

/*
** Transform pointers to text and back again
*/
स्थिर शून्य pointerToText(शून्य *p, वर्ण *z){
  अचर वर्ण *zRet = sqlite3TestPtrToText(p);
  प्र_लिखो(z, "%s", zRet);
}
स्थिर पूर्णाङ्क textToPointer(अचर वर्ण *z, शून्य **pp){
  *pp = sqlite3TestTextToPtr(z);
  फर्काऊ TCL_OK;
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
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "NBYTES");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) फर्काऊ TCL_ERROR;
  p = sqlite3_malloc((अचिन्हित)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, रिक्त);
  फर्काऊ TCL_OK;
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
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "PRIOR NBYTES");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) फर्काऊ TCL_ERROR;
  यदि( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (अचिन्हित)nByte);
  pointerToText(p, zOut);
  Tcl_AppendResult(interp, zOut, रिक्त);
  फर्काऊ TCL_OK;
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
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "PRIOR");
    फर्काऊ TCL_ERROR;
  }
  यदि( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  sqlite3_free(pPrior);
  फर्काऊ TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
पूर्णाङ्क sqlite3TestHexToBin(अचर वर्ण *, पूर्णाङ्क, वर्ण *);
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

  यदि( objc!=4 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    फर्काऊ TCL_ERROR;
  }
  यदि( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    फर्काऊ TCL_ERROR;
  }
  यदि( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  यदि( n>परिमाण(zBin)*2 ) n = परिमाण(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (पूर्णाङ्क)n, zBin);
  यदि( n==0 ){
    Tcl_AppendResult(interp, "no data", (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  zOut = p;
  लागि(i=0; i<size; i++){
    zOut[i] = zBin[i%n];
  }
  फर्काऊ TCL_OK;
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

  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "ADDRESS SIZE");
    फर्काऊ TCL_ERROR;
  }
  यदि( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_AppendResult(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &size) ){
    फर्काऊ TCL_ERROR;
  }
  यदि( size<=0 ){
    Tcl_AppendResult(interp, "size must be positive", (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  zBin = p;
  जबसम्म( size>0 ){
    यदि( size>(परिमाण(zHex)-1)/2 ){
      n = (परिमाण(zHex)-1)/2;
    }अन्यथा{
      n = size;
    }
    स_नकल(zHex, zBin, n);
    zBin += n;
    size -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_AppendResult(interp, zHex, (वर्ण*)0);
  }
  फर्काऊ TCL_OK;
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
  फर्काऊ TCL_OK;
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
  यदि( objc!=1 && objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "?RESET?");
    फर्काऊ TCL_ERROR;
  }
  यदि( objc==2 ){
    यदि( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) फर्काऊ TCL_ERROR;
  } 
  Tcl_SetObjResult(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  फर्काऊ TCL_OK;
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
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "DEPT");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &depth) ) फर्काऊ TCL_ERROR;
#ifdef SQLITE_MEMDEBUG
  {
    बाह्य शून्य sqlite3MemdebugBacktrace(पूर्णाङ्क);
    sqlite3MemdebugBacktrace(depth);
  }
#endif
  फर्काऊ TCL_OK;
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
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "FILENAME");
    फर्काऊ TCL_ERROR;
  }
#यदि defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_SIZE) \
     || defined(SQLITE_POW2_MEMORY_SIZE)
  {
    बाह्य शून्य sqlite3MemdebugDump(अचर वर्ण*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#endif
  फर्काऊ TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_malloc_गन्ती(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nMalloc = -1;
  यदि( objc!=1 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "");
    फर्काऊ TCL_ERROR;
  }
#यदि defined(SQLITE_MEMDEBUG)
  {
    बाह्य पूर्णाङ्क sqlite3_Memdebug_Malloc_गन्ती();
    nMalloc = sqlite3_Memdebug_Malloc_गन्ती();
  }
#endif
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nMalloc));
  फर्काऊ TCL_OK;
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

  यदि( objc<2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) फर्काऊ TCL_ERROR;

  लागि(ii=2; ii<objc; ii+=2){
    Tcl_Size nOption;
    वर्ण *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    वर्ण *zErr = 0;

    यदि( nOption>1 && म_भेदन(zOption, "-repeat", nOption)==0 ){
      यदि( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अन्यथा{
        यदि( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          फर्काऊ TCL_ERROR;
        }
      }
    }अन्यथा यदि( nOption>1 && म_भेदन(zOption, "-benigncnt", nOption)==0 ){
      यदि( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अन्यथा{
        pBenignCnt = objv[ii+1];
      }
    }अन्यथा{
      zErr = "unknown option: ";
    }

    यदि( zErr ){
      Tcl_AppendResult(interp, zErr, zOption, रिक्त);
      फर्काऊ TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  यदि( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nFail));
  फर्काऊ TCL_OK;
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
  यदि( objc!=1 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "");
    फर्काऊ TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_SetObjResult(interp, Tcl_NewIntObj(nPending));
  फर्काऊ TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
स्थिर पूर्णाङ्क sqlite3_memdebug_title_गन्ती = 0;

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
  sqlite3_memdebug_title_गन्ती++;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "TITLE");
    फर्काऊ TCL_ERROR;
  }
#ifdef SQLITE_MEMDEBUG
  {
    अचर वर्ण *zTitle;
    बाह्य पूर्णाङ्क sqlite3MemdebugSettitle(अचर वर्ण*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#endif
  फर्काऊ TCL_OK;
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
  यदि( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    पूर्णाङ्क isNew;

    पूर्णाङ्क a_साँचो[MALLOC_LOG_KEYINTS];
    अचिन्हित पूर्णाङ्क n_साँचो = परिमाण(पूर्णाङ्क)*MALLOC_LOG_KEYINTS;

    स_रखो(a_साँचो, 0, n_साँचो);
    यदि( (परिमाण(शून्य*)*nFrame)<n_साँचो ){
      n_साँचो = nFrame*परिमाण(शून्य*);
    }
    स_नकल(a_साँचो, aFrame, n_साँचो);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (अचर वर्ण *)a_साँचो, &isNew);
    यदि( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(परिमाण(MallocLog));
      स_रखो(pLog, 0, परिमाण(MallocLog));
      Tcl_Set_Hash_मान(pEntry, (ClientData)pLog);
    }अन्यथा{
      pLog = (MallocLog *)Tcl_Get_Hash_मान(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#endif /* SQLITE_MEMDEBUG */

स्थिर शून्य test_memdebug_log_clear(शून्य){
  Tcl_Hash_खोज खोज;
  Tcl_HashEntry *pEntry;
  लागि(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &खोज);
    pEntry;
    pEntry=Tcl_NextHashEntry(&खोज)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मान(pEntry);
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

  स्थिर अचर वर्ण *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  गणना MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  यदि( !isInit ){
#ifdef SQLITE_MEMDEBUG
    बाह्य शून्य sqlite3MemdebugBacktraceCallback(
        शून्य (*xBacktrace)(पूर्णाङ्क, पूर्णाङ्क, शून्य **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#endif
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  यदि( objc<2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  यदि( Tcl_GetIndexFromObj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    फर्काऊ TCL_ERROR;
  }

  विकल्प( (गणना MB_enum)iSub ){
    पक्ष MB_LOG_START:
      mallocLogEnabled = 1;
      रोक;
    पक्ष MB_LOG_STOP:
      mallocLogEnabled = 0;
      रोक;
    पक्ष MB_LOG_DUMP: {
      Tcl_Hash_खोज खोज;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      प्रतिज्ञा(परिमाण(Tcl_WideInt)>=परिमाण(शून्य*));

      लागि(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &खोज);
        pEntry;
        pEntry=Tcl_NextHashEntry(&खोज)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मान(pEntry);
        Tcl_WideInt *a_साँचो = (Tcl_WideInt *)Tcl_Get_Hash_साँचो(&aMallocLog, pEntry);
        पूर्णाङ्क ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        लागि(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_साँचो[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_SetObjResult(interp, pRet);
      रोक;
    }
    पक्ष MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      रोक;
    }

    पक्ष MB_LOG_SYNC: {
#ifdef SQLITE_MEMDEBUG
      बाह्य शून्य sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#endif
      रोक;
    }
  }

  फर्काऊ TCL_OK;
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
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "SIZE N");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &sz) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[2], &N) ) फर्काऊ TCL_ERROR;
  मुक्त(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_SetObjResult(interp, pRes);

  यदि( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (शून्य*)0, 0, 0);
  }अन्यथा{
    buf = दो_स्मृति( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  फर्काऊ TCL_OK;
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
  पूर्णाङ्क माथिल्लो_Stress = 0;
  बाह्य शून्य installTestPCache(पूर्णाङ्क,अचिन्हित,अचिन्हित,अचिन्हित);
  यदि( objc<2 || objc>5 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) फर्काऊ TCL_ERROR;
  यदि( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     फर्काऊ TCL_ERROR;
  }
  यदि( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     फर्काऊ TCL_ERROR;
  }
  यदि( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &माथिल्लो_Stress) ){
    फर्काऊ TCL_ERROR;
  }
  यदि( discardChance<0 || discardChance>100 ){
    Tcl_AppendResult(interp, "discard-chance should be between 0 and 100",
                     (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  installTestPCache(installFlag, (अचिन्हित)discardChance, (अचिन्हित)prngSeed,
                    (अचिन्हित)माथिल्लो_Stress);
  फर्काऊ TCL_OK;
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
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "BOOLEAN");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) फर्काऊ TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
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
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "SIZE COUNT");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &sz) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) फर्काऊ TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_SetObjResult(interp, pRet);
  फर्काऊ TCL_OK;
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
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, अचर वर्ण*, sqlite3**);
  यदि( objc!=5 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "BUFID SIZE COUNT");
    फर्काऊ TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[3], &sz) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) फर्काऊ TCL_ERROR;
  यदि( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (शून्य*)0, sz, cnt);
  }अन्यथा यदि( bufid>=1 && bufid<=2 && sz*cnt<=परिमाण(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }अन्यथा{
    Tcl_AppendResult(interp, "illegal arguments - see documentation", (वर्ण*)0);
    फर्काऊ TCL_ERROR;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(rc));
  फर्काऊ TCL_OK;
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

  यदि( nArg!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "NBYTE NMINALLOC");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) फर्काऊ TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) फर्काऊ TCL_ERROR;

  यदि( nByte==0 ){
    मुक्त( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (शून्य*)0, 0, 0);
  }अन्यथा{
    zBuf = पुनः_स्मृति(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  फर्काऊ TCL_OK;
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

  यदि( nArg!=1 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "NBYTE");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) फर्काऊ TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  फर्काऊ TCL_OK;
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
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, अचर वर्ण*, sqlite3**);

  यदि( objc!=2 && objc!=1 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "[DB]");
    फर्काऊ TCL_ERROR;
  }
  यदि( objc==2 ){
    यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;
    यदि( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (वर्ण*)0);
      फर्काऊ TCL_ERROR;
    }
  }अन्यथा{
    यदि( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_AppendResult(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (वर्ण*)0);
      फर्काऊ TCL_ERROR;
    }
  }
  फर्काऊ TCL_OK;
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

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "BOOL");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    फर्काऊ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  फर्काऊ TCL_OK;
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

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "BOOL");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    फर्काऊ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_INDEX_SCAN, bUseCis);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  फर्काऊ TCL_OK;
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

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "BOOL");
    फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    फर्काऊ TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  फर्काऊ TCL_OK;
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
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "FILENAME");
    फर्काऊ TCL_ERROR;
  }

  विकल्प( SQLITE_PTR_TO_INT(clientData) ){
    पक्ष 3: {
#ifdef SQLITE_ENABLE_MEMSYS3
      बाह्य शून्य sqlite3Memsys3Dump(अचर वर्ण*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      रोक;
#endif
    }
    पक्ष 5: {
#ifdef SQLITE_ENABLE_MEMSYS5
      बाह्य शून्य sqlite3Memsys5Dump(अचर वर्ण*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      रोक;
#endif
    }
  }
  फर्काऊ TCL_OK;
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
  पूर्णाङ्क rc, i_मान, mx_मान;
  पूर्णाङ्क i, op = 0, resetFlag;
  अचर वर्ण *zOpName;
  स्थिर अचर संरचना {
    अचर वर्ण *zName;
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
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_थाक        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_गन्ती        },
  };
  Tcl_Obj *pResult;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    फर्काऊ TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  लागि(i=0; i<ArraySize(aOp); i++){
    यदि( म_भेद(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      रोक;
    }
  }
  यदि( i>=ArraySize(aOp) ){
    यदि( Tcl_GetIntFromObj(interp, objv[1], &op) ) फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) फर्काऊ TCL_ERROR;
  i_मान = 0;
  mx_मान = 0;
  rc = sqlite3_status(op, &i_मान, &mx_मान, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_मान));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_मान));
  Tcl_SetObjResult(interp, pResult);
  फर्काऊ TCL_OK;
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
  पूर्णाङ्क rc, i_मान, mx_मान;
  पूर्णाङ्क i, op = 0, resetFlag;
  अचर वर्ण *zOpName;
  sqlite3 *db;
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, अचर वर्ण*, sqlite3**);
  स्थिर अचर संरचना {
    अचर वर्ण *zName;
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
  यदि( objc!=4 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    फर्काऊ TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) फर्काऊ TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  यदि( स_भेद(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  यदि( स_भेद(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  लागि(i=0; i<ArraySize(aOp); i++){
    यदि( म_भेद(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      रोक;
    }
  }
  यदि( i>=ArraySize(aOp) ){
    यदि( Tcl_GetIntFromObj(interp, objv[2], &op) ) फर्काऊ TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) फर्काऊ TCL_ERROR;
  i_मान = 0;
  mx_मान = 0;
  rc = sqlite3_db_status(db, op, &i_मान, &mx_मान, resetFlag);
  pResult = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(i_मान));
  Tcl_ListObjAppendElement(0, pResult, Tcl_NewIntObj(mx_मान));
  Tcl_SetObjResult(interp, pResult);
  फर्काऊ TCL_OK;
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

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "BOOLEAN");
    फर्काऊ TCL_ERROR;
  }
  यदि( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    फर्काऊ TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  फर्काऊ TCL_OK;
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
  अचर sqlite3_mem_methods *sqlite3MemGetMemsys3(शून्य);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#endif
  Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  फर्काऊ TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfs_oom_test(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णाङ्क sqlite3_memdebug_vfs_oom_test;
  यदि( objc>2 ){
    Tcl_Wrong_अङ्क_Args(interp, 1, objv, "?INTEGER?");
    फर्काऊ TCL_ERROR;
  }अन्यथा यदि( objc==2 ){
    पूर्णाङ्क iNew;
    यदि( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) फर्काऊ TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  फर्काऊ TCL_OK;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_गन्ती ,0 },
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
  लागि(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  फर्काऊ TCL_OK;
}
#endif
