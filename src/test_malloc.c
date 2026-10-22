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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>

/*
** This structure is used to encapsulate the global state variables used 
** by malloc() fault simulation.
*/
अटल काष्ठा MemFault {
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
अटल व्योम sqlite3Fault(व्योम){
  अटल u64 cnt = 0;
  cnt++;
  अगर( cnt>((u64)1<<63) ) पात();
}

/*
** This routine exists as a place to set a breakpoint that will
** fire the first time any malloc() fails on a single test case.
** The sqlite3Fault() routine above runs on every malloc() failure.
** This routine only runs on the first such failure.
*/
अटल व्योम sqlite3FirstFault(व्योम){
  अटल u64 cnt2 = 0;
  cnt2++;
  अगर( cnt2>((u64)1<<63) ) पात();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
अटल पूर्णांक faultsimStep(व्योम){
  अगर( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    वापस 0;
  }
  अगर( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    वापस 0;
  }
  अगर( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  अगर( memfault.isBenignMode>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  अगर( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  वापस 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
अटल व्योम *faultsimMalloc(पूर्णांक n){
  व्योम *p = 0;
  अगर( !faultsimStep() ){
    p = memfault.m.xMalloc(n);
  }
  वापस p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
अटल व्योम *faultsimRealloc(व्योम *pOld, पूर्णांक n){
  व्योम *p = 0;
  अगर( !faultsimStep() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  वापस p;
}

/*
** This routine configures the malloc failure simulation.  After
** calling this routine, the next nDelay mallocs will succeed, followed
** by a block of nRepeat failures, after which malloc() calls will begin
** to succeed again.
*/
अटल व्योम faultsimConfig(पूर्णांक nDelay, पूर्णांक nRepeat){
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
अटल पूर्णांक faultsimFailures(व्योम){
  वापस memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
अटल पूर्णांक faultsimBenignFailures(व्योम){
  वापस memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
अटल पूर्णांक faultsimPending(व्योम){
  अगर( memfault.enable ){
    वापस memfault.iCountdown;
  }अन्यथा{
    वापस -1;
  }
}


अटल व्योम faultsimBeginBenign(व्योम){
  memfault.isBenignMode++;
}
अटल व्योम faultsimEndBenign(व्योम){
  memfault.isBenignMode--;
}

/*
** Add or remove the fault-simulation layer using sqlite3_config(). If
** the argument is non-zero, the 
*/
अटल पूर्णांक faultsimInstall(पूर्णांक install){
  पूर्णांक rc;

  install = (install ? 1 : 0);
  निश्चित(memfault.isInstalled==1 || memfault.isInstalled==0);

  अगर( install==memfault.isInstalled ){
    वापस SQLITE_ERROR;
  }

  अगर( install ){
    rc = sqlite3_config(SQLITE_CONFIG_GETMALLOC, &memfault.m);
    निश्चित(memfault.m.xMalloc);
    अगर( rc==SQLITE_OK ){
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
    निश्चित(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    स_रखो(&m2, 0, माप(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    निश्चित( स_भेद(&m2, &memfault.m, माप(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (व्योम*)0, (व्योम*)0);
  }

  अगर( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  वापस rc;
}

#अगर_घोषित SQLITE_TEST

/*
** This function is implemented in main.c. Returns a pointer to a static
** buffer containing the symbolic SQLite error code that corresponds to
** the least-significant 8-bits of the integer passed as an argument.
** For example:
**
**   sqlite3ErrName(1) -> "SQLITE_ERROR"
*/
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/*
** Transform pointers to text and back again
*/
अटल व्योम pointerToText(व्योम *p, अक्षर *z){
  स्थिर अक्षर *zRet = sqlite3TestPtrToText(p);
  प्र_लिखो(z, "%s", zRet);
}
अटल पूर्णांक textToPointer(स्थिर अक्षर *z, व्योम **pp){
  *pp = sqlite3TestTextToPtr(z);
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
अटल पूर्णांक SQLITE_TCLAPI test_malloc(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nByte;
  व्योम *p;
  अक्षर zOut[100];
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NBYTES");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) वापस TCL_ERROR;
  p = sqlite3_malloc((अचिन्हित)nByte);
  pointerToText(p, zOut);
  Tcl_Append_परिणाम(interp, zOut, शून्य);
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
अटल पूर्णांक SQLITE_TCLAPI test_realloc(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nByte;
  व्योम *pPrior, *p;
  अक्षर zOut[100];
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "PRIOR NBYTES");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) वापस TCL_ERROR;
  अगर( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_Append_परिणाम(interp, "bad pointer: ", Tcl_GetString(objv[1]), (अक्षर*)0);
    वापस TCL_ERROR;
  }
  p = sqlite3_realloc(pPrior, (अचिन्हित)nByte);
  pointerToText(p, zOut);
  Tcl_Append_परिणाम(interp, zOut, शून्य);
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
अटल पूर्णांक SQLITE_TCLAPI test_free(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  व्योम *pPrior;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "PRIOR");
    वापस TCL_ERROR;
  }
  अगर( textToPointer(Tcl_GetString(objv[1]), &pPrior) ){
    Tcl_Append_परिणाम(interp, "bad pointer: ", Tcl_GetString(objv[1]), (अक्षर*)0);
    वापस TCL_ERROR;
  }
  sqlite3_free(pPrior);
  वापस TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
पूर्णांक sqlite3TestHexToBin(स्थिर अक्षर *, पूर्णांक, अक्षर *);
पूर्णांक sqlite3TestBinToHex(अक्षर*,पूर्णांक);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
अटल पूर्णांक SQLITE_TCLAPI test_memset(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  व्योम *p;
  पूर्णांक आकार, i;
  Tcl_आकार n;
  अक्षर *zHex;
  अक्षर *zOut;
  अक्षर zBin[100];

  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "ADDRESS SIZE HEX");
    वापस TCL_ERROR;
  }
  अगर( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_परिणाम(interp, "bad pointer: ", Tcl_GetString(objv[1]), (अक्षर*)0);
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[2], &आकार) ){
    वापस TCL_ERROR;
  }
  अगर( आकार<=0 ){
    Tcl_Append_परिणाम(interp, "size must be positive", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  अगर( n>माप(zBin)*2 ) n = माप(zBin)*2;
  n = sqlite3TestHexToBin(zHex, (पूर्णांक)n, zBin);
  अगर( n==0 ){
    Tcl_Append_परिणाम(interp, "no data", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  zOut = p;
  क्रम(i=0; i<आकार; i++){
    zOut[i] = zBin[i%n];
  }
  वापस TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
अटल पूर्णांक SQLITE_TCLAPI test_memget(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  व्योम *p;
  पूर्णांक आकार, n;
  अक्षर *zBin;
  अक्षर zHex[100];

  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "ADDRESS SIZE");
    वापस TCL_ERROR;
  }
  अगर( textToPointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_परिणाम(interp, "bad pointer: ", Tcl_GetString(objv[1]), (अक्षर*)0);
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[2], &आकार) ){
    वापस TCL_ERROR;
  }
  अगर( आकार<=0 ){
    Tcl_Append_परिणाम(interp, "size must be positive", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  zBin = p;
  जबतक( आकार>0 ){
    अगर( आकार>(माप(zHex)-1)/2 ){
      n = (माप(zHex)-1)/2;
    }अन्यथा{
      n = आकार;
    }
    स_नकल(zHex, zBin, n);
    zBin += n;
    आकार -= n;
    sqlite3TestBinToHex(zHex, n);
    Tcl_Append_परिणाम(interp, zHex, (अक्षर*)0);
  }
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
अटल पूर्णांक SQLITE_TCLAPI test_memory_used(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewWideIntObj(sqlite3_memory_used()));
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
अटल पूर्णांक SQLITE_TCLAPI test_memory_highwater(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक resetFlag = 0;
  अगर( objc!=1 && objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?RESET?");
    वापस TCL_ERROR;
  }
  अगर( objc==2 ){
    अगर( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) वापस TCL_ERROR;
  } 
  Tcl_Set_Obj_परिणाम(interp, 
     Tcl_NewWideIntObj(sqlite3_memory_highwater(resetFlag)));
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
अटल पूर्णांक SQLITE_TCLAPI test_memdebug_backtrace(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक depth;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DEPT");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &depth) ) वापस TCL_ERROR;
#अगर_घोषित SQLITE_MEMDEBUG
  {
    बाह्य व्योम sqlite3MemdebugBacktrace(पूर्णांक);
    sqlite3MemdebugBacktrace(depth);
  }
#पूर्ण_अगर
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
अटल पूर्णांक SQLITE_TCLAPI test_memdebug_dump(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME");
    वापस TCL_ERROR;
  }
#अगर defined(SQLITE_MEMDEBUG) || defined(SQLITE_MEMORY_आकार) \
     || defined(SQLITE_POW2_MEMORY_आकार)
  {
    बाह्य व्योम sqlite3MemdebugDump(स्थिर अक्षर*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#पूर्ण_अगर
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
अटल पूर्णांक SQLITE_TCLAPI test_memdebug_malloc_गिनती(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nMalloc = -1;
  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }
#अगर defined(SQLITE_MEMDEBUG)
  {
    बाह्य पूर्णांक sqlite3_Memdebug_Malloc_गिनती();
    nMalloc = sqlite3_Memdebug_Malloc_गिनती();
  }
#पूर्ण_अगर
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(nMalloc));
  वापस TCL_OK;
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
अटल पूर्णांक SQLITE_TCLAPI test_memdebug_fail(
  व्योम * client_तथ्य,
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

  अगर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "COUNTER ?OPTIONS?");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) वापस TCL_ERROR;

  क्रम(ii=2; ii<objc; ii+=2){
    Tcl_आकार nOption;
    अक्षर *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    अक्षर *zErr = 0;

    अगर( nOption>1 && म_भेदन(zOption, "-repeat", nOption)==0 ){
      अगर( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अन्यथा{
        अगर( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          वापस TCL_ERROR;
        }
      }
    }अन्यथा अगर( nOption>1 && म_भेदन(zOption, "-benigncnt", nOption)==0 ){
      अगर( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अन्यथा{
        pBenignCnt = objv[ii+1];
      }
    }अन्यथा{
      zErr = "unknown option: ";
    }

    अगर( zErr ){
      Tcl_Append_परिणाम(interp, zErr, zOption, शून्य);
      वापस TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  अगर( pBenignCnt ){
    Tcl_ObjSetVar2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(nFail));
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
अटल पूर्णांक SQLITE_TCLAPI test_memdebug_pending(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nPending;
  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(nPending));
  वापस TCL_OK;
}

/*
** The following global variable keeps track of the number of tests
** that have run.  This variable is only useful when running in the
** debugger.
*/
अटल पूर्णांक sqlite3_memdebug_title_गिनती = 0;

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
अटल पूर्णांक SQLITE_TCLAPI test_memdebug_settitle(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_गिनती++;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "TITLE");
    वापस TCL_ERROR;
  }
#अगर_घोषित SQLITE_MEMDEBUG
  {
    स्थिर अक्षर *zTitle;
    बाह्य पूर्णांक sqlite3MemdebugSettitle(स्थिर अक्षर*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#पूर्ण_अगर
  वापस TCL_OK;
}

#घोषणा MALLOC_LOG_FRAMES  10 
#घोषणा MALLOC_LOG_KEYINTS (                                              \
    10 * ((माप(पूर्णांक)>=माप(व्योम*)) ? 1 : माप(व्योम*)/माप(पूर्णांक))   \
)
अटल Tcl_HashTable aMallocLog;
अटल पूर्णांक mallocLogEnabled = 0;

प्रकार काष्ठा MallocLog MallocLog;
काष्ठा MallocLog {
  पूर्णांक nCall;
  पूर्णांक nByte;
};

#अगर_घोषित SQLITE_MEMDEBUG
अटल व्योम test_memdebug_callback(पूर्णांक nByte, पूर्णांक nFrame, व्योम **aFrame){
  अगर( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_HashEntry *pEntry;
    पूर्णांक isNew;

    पूर्णांक a_कुंजी[MALLOC_LOG_KEYINTS];
    अचिन्हित पूर्णांक n_कुंजी = माप(पूर्णांक)*MALLOC_LOG_KEYINTS;

    स_रखो(a_कुंजी, 0, n_कुंजी);
    अगर( (माप(व्योम*)*nFrame)<n_कुंजी ){
      n_कुंजी = nFrame*माप(व्योम*);
    }
    स_नकल(a_कुंजी, aFrame, n_कुंजी);

    pEntry = Tcl_CreateHashEntry(&aMallocLog, (स्थिर अक्षर *)a_कुंजी, &isNew);
    अगर( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(माप(MallocLog));
      स_रखो(pLog, 0, माप(MallocLog));
      Tcl_Set_Hash_मूल्य(pEntry, (Client_तथ्य)pLog);
    }अन्यथा{
      pLog = (MallocLog *)Tcl_Get_Hash_मूल्य(pEntry);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#पूर्ण_अगर /* SQLITE_MEMDEBUG */

अटल व्योम test_memdebug_log_clear(व्योम){
  Tcl_Hash_खोज खोज;
  Tcl_HashEntry *pEntry;
  क्रम(
    pEntry=Tcl_FirstHashEntry(&aMallocLog, &खोज);
    pEntry;
    pEntry=Tcl_अग्र_Hash_Entry(&खोज)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मूल्य(pEntry);
    Tcl_Free((अक्षर *)pLog);
  }
  Tcl_DeleteHashTable(&aMallocLog);
  Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
}

अटल पूर्णांक SQLITE_TCLAPI test_memdebug_log(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  अटल पूर्णांक isInit = 0;
  पूर्णांक iSub;

  अटल स्थिर अक्षर *MB_strs[] = { "start", "stop", "dump", "clear", "sync" };
  क्रमागत MB_enum { 
      MB_LOG_START, MB_LOG_STOP, MB_LOG_DUMP, MB_LOG_CLEAR, MB_LOG_SYNC 
  };

  अगर( !isInit ){
#अगर_घोषित SQLITE_MEMDEBUG
    बाह्य व्योम sqlite3MemdebugBacktraceCallback(
        व्योम (*xBacktrace)(पूर्णांक, पूर्णांक, व्योम **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#पूर्ण_अगर
    Tcl_InitHashTable(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  अगर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND ...");
  }
  अगर( Tcl_Get_सूचकांक_From_Obj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    वापस TCL_ERROR;
  }

  चयन( (क्रमागत MB_enum)iSub ){
    हाल MB_LOG_START:
      mallocLogEnabled = 1;
      अवरोध;
    हाल MB_LOG_STOP:
      mallocLogEnabled = 0;
      अवरोध;
    हाल MB_LOG_DUMP: {
      Tcl_Hash_खोज खोज;
      Tcl_HashEntry *pEntry;
      Tcl_Obj *pRet = Tcl_NewObj();

      निश्चित(माप(Tcl_WideInt)>=माप(व्योम*));

      क्रम(
        pEntry=Tcl_FirstHashEntry(&aMallocLog, &खोज);
        pEntry;
        pEntry=Tcl_अग्र_Hash_Entry(&खोज)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_FRAMES+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मूल्य(pEntry);
        Tcl_WideInt *a_कुंजी = (Tcl_WideInt *)Tcl_Get_Hash_कुंजी(&aMallocLog, pEntry);
        पूर्णांक ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        क्रम(ii=0; ii<MALLOC_LOG_FRAMES; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_कुंजी[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_FRAMES+2, apElem)
        );
      }

      Tcl_Set_Obj_परिणाम(interp, pRet);
      अवरोध;
    }
    हाल MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      अवरोध;
    }

    हाल MB_LOG_SYNC: {
#अगर_घोषित SQLITE_MEMDEBUG
      बाह्य व्योम sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#पूर्ण_अगर
      अवरोध;
    }
  }

  वापस TCL_OK;
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
अटल पूर्णांक SQLITE_TCLAPI test_config_pagecache(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक sz, N;
  Tcl_Obj *pRes;
  अटल अक्षर *buf = 0;
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SIZE N");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &sz) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[2], &N) ) वापस TCL_ERROR;
  मुक्त(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_Set_Obj_परिणाम(interp, pRes);

  अगर( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (व्योम*)0, 0, 0);
  }अन्यथा{
    buf = दो_स्मृति( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  वापस TCL_OK;
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
अटल पूर्णांक SQLITE_TCLAPI test_alt_pcache(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक installFlag;
  पूर्णांक discardChance = 0;
  पूर्णांक prngSeed = 0;
  पूर्णांक ऊपरी_Stress = 0;
  बाह्य व्योम installTestPCache(पूर्णांक,अचिन्हित,अचिन्हित,अचिन्हित);
  अगर( objc<2 || objc>5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) वापस TCL_ERROR;
  अगर( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     वापस TCL_ERROR;
  }
  अगर( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prngSeed) ){
     वापस TCL_ERROR;
  }
  अगर( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &ऊपरी_Stress) ){
    वापस TCL_ERROR;
  }
  अगर( discardChance<0 || discardChance>100 ){
    Tcl_Append_परिणाम(interp, "discard-chance should be between 0 and 100",
                     (अक्षर*)0);
    वापस TCL_ERROR;
  }
  installTestPCache(installFlag, (अचिन्हित)discardChance, (अचिन्हित)prngSeed,
                    (अचिन्हित)ऊपरी_Stress);
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
अटल पूर्णांक SQLITE_TCLAPI test_config_memstatus(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक enable, rc;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOLEAN");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) वापस TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
अटल पूर्णांक SQLITE_TCLAPI test_config_lookaside(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक sz, cnt;
  Tcl_Obj *pRet;
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SIZE COUNT");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &sz) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) वापस TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_Set_Obj_परिणाम(interp, pRet);
  वापस TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
अटल पूर्णांक SQLITE_TCLAPI test_db_config_lookaside(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक sz, cnt;
  sqlite3 *db;
  पूर्णांक bufid;
  अटल अक्षर azBuf[2][10000];
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिर अक्षर*, sqlite3**);
  अगर( objc!=5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BUFID SIZE COUNT");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[3], &sz) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) वापस TCL_ERROR;
  अगर( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (व्योम*)0, sz, cnt);
  }अन्यथा अगर( bufid>=1 && bufid<=2 && sz*cnt<=माप(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }अन्यथा{
    Tcl_Append_परिणाम(interp, "illegal arguments - see documentation", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
अटल पूर्णांक SQLITE_TCLAPI test_config_heap(
  व्योम * client_तथ्य, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  अटल अक्षर *zBuf; /* Use this memory */
  पूर्णांक nByte;         /* Size of buffer to pass to sqlite3_config() */
  पूर्णांक nMinAlloc;     /* Size of minimum allocation */
  पूर्णांक rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  पूर्णांक nArg = objc-1;

  अगर( nArg!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NBYTE NMINALLOC");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) वापस TCL_ERROR;
  अगर( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) वापस TCL_ERROR;

  अगर( nByte==0 ){
    मुक्त( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (व्योम*)0, 0, 0);
  }अन्यथा{
    zBuf = पुनः_स्मृति(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
अटल पूर्णांक SQLITE_TCLAPI test_config_heap_आकार(
  व्योम * client_तथ्य, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक nByte;         /* Size to pass to sqlite3_config() */
  पूर्णांक rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  पूर्णांक nArg = objc-1;

  अगर( nArg!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "NBYTE");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) वापस TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
अटल पूर्णांक SQLITE_TCLAPI test_config_error(
  व्योम * client_तथ्य, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिर अक्षर*, sqlite3**);

  अगर( objc!=2 && objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "[DB]");
    वापस TCL_ERROR;
  }
  अगर( objc==2 ){
    अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
    अगर( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_Append_परिणाम(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (अक्षर*)0);
      वापस TCL_ERROR;
    }
  }अन्यथा{
    अगर( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_Append_परिणाम(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (अक्षर*)0);
      वापस TCL_ERROR;
    }
  }
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
अटल पूर्णांक SQLITE_TCLAPI test_config_uri(
  व्योम * client_तथ्य, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक bOpenUri;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOL");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    वापस TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);

  वापस TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
अटल पूर्णांक SQLITE_TCLAPI test_config_cis(
  व्योम * client_तथ्य, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक bUseCis;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOL");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    वापस TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_सूचकांक_SCAN, bUseCis);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);

  वापस TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
अटल पूर्णांक SQLITE_TCLAPI test_config_pmasz(
  व्योम * client_तथ्य, 
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक iPmaSz;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOL");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    वापस TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);

  वापस TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
अटल पूर्णांक SQLITE_TCLAPI test_dump_memsys3(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "FILENAME");
    वापस TCL_ERROR;
  }

  चयन( SQLITE_PTR_TO_INT(client_तथ्य) ){
    हाल 3: {
#अगर_घोषित SQLITE_ENABLE_MEMSYS3
      बाह्य व्योम sqlite3Memsys3Dump(स्थिर अक्षर*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      अवरोध;
#पूर्ण_अगर
    }
    हाल 5: {
#अगर_घोषित SQLITE_ENABLE_MEMSYS5
      बाह्य व्योम sqlite3Memsys5Dump(स्थिर अक्षर*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      अवरोध;
#पूर्ण_अगर
    }
  }
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
अटल पूर्णांक SQLITE_TCLAPI test_status(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc, i_मूल्य, mx_मूल्य;
  पूर्णांक i, op = 0, resetFlag;
  स्थिर अक्षर *zOpName;
  अटल स्थिर काष्ठा {
    स्थिर अक्षर *zName;
    पूर्णांक op;
  } aOp[] = {
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_STATUS_MALLOC_आकार         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_STATUS_PAGECACHE_आकार      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_STATUS_SCRATCH_आकार        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_STATUS_PARSER_ढेर        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_STATUS_MALLOC_गिनती        },
  };
  Tcl_Obj *p_परिणाम;
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "PARAMETER RESETFLAG");
    वापस TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  क्रम(i=0; i<Array_आकार(aOp); i++){
    अगर( म_भेद(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      अवरोध;
    }
  }
  अगर( i>=Array_आकार(aOp) ){
    अगर( Tcl_GetIntFromObj(interp, objv[1], &op) ) वापस TCL_ERROR;
  }
  अगर( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) वापस TCL_ERROR;
  i_मूल्य = 0;
  mx_मूल्य = 0;
  rc = sqlite3_status(op, &i_मूल्य, &mx_मूल्य, resetFlag);
  p_परिणाम = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(i_मूल्य));
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(mx_मूल्य));
  Tcl_Set_Obj_परिणाम(interp, p_परिणाम);
  वापस TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
अटल पूर्णांक SQLITE_TCLAPI test_db_status(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc, i_मूल्य, mx_मूल्य;
  पूर्णांक i, op = 0, resetFlag;
  स्थिर अक्षर *zOpName;
  sqlite3 *db;
  बाह्य पूर्णांक getDbPointer(Tcl_Interp*, स्थिर अक्षर*, sqlite3**);
  अटल स्थिर काष्ठा {
    स्थिर अक्षर *zName;
    पूर्णांक op;
  } aOp[] = {
    { "LOOKASIDE_USED",      SQLITE_DBSTATUS_LOOKASIDE_USED      },
    { "CACHE_USED",          SQLITE_DBSTATUS_CACHE_USED          },
    { "SCHEMA_USED",         SQLITE_DBSTATUS_SCHEMA_USED         },
    { "STMT_USED",           SQLITE_DBSTATUS_STMT_USED           },
    { "LOOKASIDE_HIT",       SQLITE_DBSTATUS_LOOKASIDE_HIT       },
    { "LOOKASIDE_MISS_SIZE", SQLITE_DBSTATUS_LOOKASIDE_MISS_आकार },
    { "LOOKASIDE_MISS_FULL", SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL },
    { "CACHE_HIT",           SQLITE_DBSTATUS_CACHE_HIT           },
    { "CACHE_MISS",          SQLITE_DBSTATUS_CACHE_MISS          },
    { "CACHE_WRITE",         SQLITE_DBSTATUS_CACHE_WRITE         },
    { "DEFERRED_FKS",        SQLITE_DBSTATUS_DEFERRED_FKS        },
    { "CACHE_USED_SHARED",   SQLITE_DBSTATUS_CACHE_USED_SHARED   },
    { "CACHE_SPILL",         SQLITE_DBSTATUS_CACHE_SPILL         },
    { "TEMPBUF_SPILL",       SQLITE_DBSTATUS_TEMPBUF_SPILL       },
  };
  Tcl_Obj *p_परिणाम;
  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB PARAMETER RESETFLAG");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  अगर( स_भेद(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  अगर( स_भेद(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  क्रम(i=0; i<Array_आकार(aOp); i++){
    अगर( म_भेद(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      अवरोध;
    }
  }
  अगर( i>=Array_आकार(aOp) ){
    अगर( Tcl_GetIntFromObj(interp, objv[2], &op) ) वापस TCL_ERROR;
  }
  अगर( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) वापस TCL_ERROR;
  i_मूल्य = 0;
  mx_मूल्य = 0;
  rc = sqlite3_db_status(db, op, &i_मूल्य, &mx_मूल्य, resetFlag);
  p_परिणाम = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(i_मूल्य));
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(mx_मूल्य));
  Tcl_Set_Obj_परिणाम(interp, p_परिणाम);
  वापस TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
अटल पूर्णांक SQLITE_TCLAPI test_install_malloc_faultsim(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc;
  पूर्णांक isInstall;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "BOOLEAN");
    वापस TCL_ERROR;
  }
  अगर( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    वापस TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  वापस TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
अटल पूर्णांक SQLITE_TCLAPI test_install_memsys3(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक rc = SQLITE_MISUSE;
#अगर_घोषित SQLITE_ENABLE_MEMSYS3
  स्थिर sqlite3_mem_methods *sqlite3MemGetMemsys3(व्योम);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#पूर्ण_अगर
  Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_VOLATILE);
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_vfs_oom_test(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णांक sqlite3_memdebug_vfs_oom_test;
  अगर( objc>2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "?INTEGER?");
    वापस TCL_ERROR;
  }अन्यथा अगर( objc==2 ){
    पूर्णांक iNew;
    अगर( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) वापस TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  वापस TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_malloc_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     पूर्णांक client_तथ्य;
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
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_गिनती ,0 },
     { "sqlite3_memdebug_log",       test_memdebug_log             ,0 },
     { "sqlite3_config_pagecache",   test_config_pagecache         ,0 },
     { "sqlite3_config_alt_pcache",  test_alt_pcache               ,0 },
     { "sqlite3_status",             test_status                   ,0 },
     { "sqlite3_db_status",          test_db_status                ,0 },
     { "install_malloc_faultsim",    test_install_malloc_faultsim  ,0 },
     { "sqlite3_config_heap",        test_config_heap              ,0 },
     { "sqlite3_config_heap_size",   test_config_heap_आकार         ,0 },
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
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Client_तथ्य c = (Client_तथ्य)SQLITE_INT_TO_PTR(aObjCmd[i].client_तथ्य);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  वापस TCL_OK;
}
#पूर्ण_अगर
