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
  पूर्णाङ्क is_Benign_बहुलकम्;       /* True if malloc failures are considered benign */
  sqlite3_mem_methods m;  /* 'Real' malloc implementation */
} memfault;

/*
** This routine exists as a place to set a breakpoint that will
** fire on any simulated malloc() failure.
*/
स्थिर शून्य sqlite3Fault(शून्य){
  स्थिर u64 cnt = 0;
  cnt++;
  यदि( cnt>((u64)1<<63) ) abort();
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
  यदि( cnt2>((u64)1<<63) ) abort();
}

/*
** Check to see if a fault should be simulated.  Return true to simulate
** the fault.  Return false if the fault should not be simulated.
*/
स्थिर पूर्णाङ्क faultsim_चरणः(शून्य){
  यदि( likely(!memfault.enable) ){
    memfault.nOkAfter++;
    प्रत्यागम 0;
  }
  यदि( memfault.iCountdown>0 ){
    memfault.iCountdown--;
    memfault.nOkBefore++;
    प्रत्यागम 0;
  }
  यदि( memfault.nFail==0 ) sqlite3FirstFault();
  sqlite3Fault();
  memfault.nFail++;
  यदि( memfault.is_Benign_बहुलकम्>0 ){
    memfault.nBenign++;
  }
  memfault.nRepeat--;
  यदि( memfault.nRepeat<=0 ){
    memfault.enable = 0;
  }
  प्रत्यागम 1;  
}

/*
** A version of sqlite3_mem_methods.xMalloc() that includes fault simulation
** logic.
*/
स्थिर शून्य *faultsimMalloc(पूर्णाङ्क n){
  शून्य *p = 0;
  यदि( !faultsim_चरणः() ){
    p = memfault.m.xMalloc(n);
  }
  प्रत्यागम p;
}


/*
** A version of sqlite3_mem_methods.xRealloc() that includes fault simulation
** logic.
*/
स्थिर शून्य *faultsimRealloc(शून्य *pOld, पूर्णाङ्क n){
  शून्य *p = 0;
  यदि( !faultsim_चरणः() ){
    p = memfault.m.xRealloc(pOld, n);
  }
  प्रत्यागम p;
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
  memfault.is_Benign_बहुलकम् = 0;
}

/*
** Return the number of faults (both hard and benign faults) that have
** occurred since the injector was last configured.
*/
स्थिर पूर्णाङ्क faultsimFailures(शून्य){
  प्रत्यागम memfault.nFail;
}

/*
** Return the number of benign faults that have occurred since the
** injector was last configured.
*/
स्थिर पूर्णाङ्क faultsimBenignFailures(शून्य){
  प्रत्यागम memfault.nBenign;
}

/*
** Return the number of successes that will occur before the next failure.
** If no failures are scheduled, return -1.
*/
स्थिर पूर्णाङ्क faultsimPending(शून्य){
  यदि( memfault.enable ){
    प्रत्यागम memfault.iCountdown;
  }अन्यथा{
    प्रत्यागम -1;
  }
}


स्थिर शून्य faultsimBeginBenign(शून्य){
  memfault.is_Benign_बहुलकम्++;
}
स्थिर शून्य faultsim_अन्तः_Benign(शून्य){
  memfault.is_Benign_बहुलकम्--;
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
    प्रत्यागम SQLITE_ERROR;
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
        faultsimBeginBenign, faultsim_अन्तः_Benign
    );
  }अन्यथा{
    sqlite3_mem_methods m2;
    प्रतिज्ञा(memfault.m.xMalloc);

    /* One should be able to reset the default memory allocator by storing
    ** a zeroed allocator then calling GETMALLOC. */
    स्मृतिं_पूरय(&m2, 0, परिमाण(m2));
    sqlite3_config(SQLITE_CONFIG_MALLOC, &m2);
    sqlite3_config(SQLITE_CONFIG_GETMALLOC, &m2);
    प्रतिज्ञा( memcmp(&m2, &memfault.m, परिमाण(m2))==0 );

    rc = sqlite3_config(SQLITE_CONFIG_MALLOC, &memfault.m);
    sqlite3_test_control(SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,
        (शून्य*)0, (शून्य*)0);
  }

  यदि( rc==SQLITE_OK ){
    memfault.isInstalled = 1;
  }
  प्रत्यागम rc;
}

#यदि_परिभाषितम् SQLITE_TEST

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
स्थिर शून्य pointer_गन्तव्यम्_Text(शून्य *p, वर्ण *z){
  ध्रुव वर्ण *zRet = sqlite3_Test_Ptr_गन्तव्यम्_Text(p);
  sprintf(z, "%s", zRet);
}
स्थिर पूर्णाङ्क text_गन्तव्यम्_Pointer(ध्रुव वर्ण *z, शून्य **pp){
  *pp = sqlite3_Test_Text_गन्तव्यम्_Ptr(z);
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_malloc  NBYTES
**
** Raw test interface for sqlite3_malloc().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_malloc(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nByte;
  शून्य *p;
  वर्ण z_बहिर्गमनम्[100];
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "NBYTES");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &nByte) ) प्रत्यागम TCL_ERROR;
  p = sqlite3_malloc((अचिह्नित)nByte);
  pointer_गन्तव्यम्_Text(p, z_बहिर्गमनम्);
  Tcl_Append_परिणाम(interp, z_बहिर्गमनम्, रिक्त);
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_realloc  PRIOR  NBYTES
**
** Raw test interface for sqlite3_realloc().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_realloc(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nByte;
  शून्य *p_प्राक्, *p;
  वर्ण z_बहिर्गमनम्[100];
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "PRIOR NBYTES");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &nByte) ) प्रत्यागम TCL_ERROR;
  यदि( text_गन्तव्यम्_Pointer(Tcl_GetString(objv[1]), &p_प्राक्) ){
    Tcl_Append_परिणाम(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  p = sqlite3_realloc(p_प्राक्, (अचिह्नित)nByte);
  pointer_गन्तव्यम्_Text(p, z_बहिर्गमनम्);
  Tcl_Append_परिणाम(interp, z_बहिर्गमनम्, रिक्त);
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_free  PRIOR
**
** Raw test interface for sqlite3_free().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_free(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *p_प्राक्;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "PRIOR");
    प्रत्यागम TCL_ERROR;
  }
  यदि( text_गन्तव्यम्_Pointer(Tcl_GetString(objv[1]), &p_प्राक्) ){
    Tcl_Append_परिणाम(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  sqlite3_free(p_प्राक्);
  प्रत्यागम TCL_OK;
}

/*
** These routines are in test_hexio.c
*/
पूर्णाङ्क sqlite3_Test_Hex_गन्तव्यम्_Bin(ध्रुव वर्ण *, पूर्णाङ्क, वर्ण *);
पूर्णाङ्क sqlite3_Test_Bin_गन्तव्यम्_Hex(वर्ण*,पूर्णाङ्क);

/*
** Usage:    memset  ADDRESS  SIZE  HEX
**
** Set a chunk of memory (obtained from malloc, probably) to a
** specified hex pattern.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memset(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *p;
  पूर्णाङ्क आकार, i;
  Tcl_आकार n;
  वर्ण *zHex;
  वर्ण *z_बहिर्गमनम्;
  वर्ण zBin[100];

  यदि( objc!=4 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "ADDRESS SIZE HEX");
    प्रत्यागम TCL_ERROR;
  }
  यदि( text_गन्तव्यम्_Pointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_परिणाम(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &आकार) ){
    प्रत्यागम TCL_ERROR;
  }
  यदि( आकार<=0 ){
    Tcl_Append_परिणाम(interp, "size must be positive", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  zHex = Tcl_GetStringFromObj(objv[3], &n);
  यदि( n>परिमाण(zBin)*2 ) n = परिमाण(zBin)*2;
  n = sqlite3_Test_Hex_गन्तव्यम्_Bin(zHex, (पूर्णाङ्क)n, zBin);
  यदि( n==0 ){
    Tcl_Append_परिणाम(interp, "no data", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  z_बहिर्गमनम् = p;
  प्रति(i=0; i<आकार; i++){
    z_बहिर्गमनम्[i] = zBin[i%n];
  }
  प्रत्यागम TCL_OK;
}

/*
** Usage:    memget  ADDRESS  SIZE
**
** Return memory as hexadecimal text.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memget(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  शून्य *p;
  पूर्णाङ्क आकार, n;
  वर्ण *zBin;
  वर्ण zHex[100];

  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "ADDRESS SIZE");
    प्रत्यागम TCL_ERROR;
  }
  यदि( text_गन्तव्यम्_Pointer(Tcl_GetString(objv[1]), &p) ){
    Tcl_Append_परिणाम(interp, "bad pointer: ", Tcl_GetString(objv[1]), (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &आकार) ){
    प्रत्यागम TCL_ERROR;
  }
  यदि( आकार<=0 ){
    Tcl_Append_परिणाम(interp, "size must be positive", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  zBin = p;
  यावत्( आकार>0 ){
    यदि( आकार>(परिमाण(zHex)-1)/2 ){
      n = (परिमाण(zHex)-1)/2;
    }अन्यथा{
      n = आकार;
    }
    स्मृतिं_प्रतिलिख(zHex, zBin, n);
    zBin += n;
    आकार -= n;
    sqlite3_Test_Bin_गन्तव्यम्_Hex(zHex, n);
    Tcl_Append_परिणाम(interp, zHex, (वर्ण*)0);
  }
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_memory_used
**
** Raw test interface for sqlite3_memory_used().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_स्मृतिः_प्रयुक्तम्(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewWideIntObj(sqlite3_स्मृतिः_प्रयुक्तम्()));
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_memory_highwater ?RESETFLAG?
**
** Raw test interface for sqlite3_memory_highwater().
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_स्मृतिः_highwater(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क resetFlag = 0;
  यदि( objc!=1 && objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "?RESET?");
    प्रत्यागम TCL_ERROR;
  }
  यदि( objc==2 ){
    यदि( Tcl_GetBooleanFromObj(interp, objv[1], &resetFlag) ) प्रत्यागम TCL_ERROR;
  } 
  Tcl_समुच्चयः_Obj_परिणाम(interp, 
     Tcl_NewWideIntObj(sqlite3_स्मृतिः_highwater(resetFlag)));
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_backtrace DEPTH
**
** Set the depth of backtracing.  If SQLITE_MEMDEBUG is not defined
** then this routine is a no-op.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_backtrace(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क गभीरता;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DEPT");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &गभीरता) ) प्रत्यागम TCL_ERROR;
#यदि_परिभाषितम् SQLITE_MEMDEBUG
  {
    बाह्य शून्य sqlite3MemdebugBacktrace(पूर्णाङ्क);
    sqlite3MemdebugBacktrace(गभीरता);
  }
#यदि_अन्तः
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_dump  FILENAME
**
** Write a summary of unfreed memory to FILENAME.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_dump(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "FILENAME");
    प्रत्यागम TCL_ERROR;
  }
#यदि defined(SQLITE_MEMDEBUG) || defined(SQLITE_स्मृतिः_आकार) \
     || defined(SQLITE_POW2_स्मृतिः_आकार)
  {
    बाह्य शून्य sqlite3MemdebugDump(ध्रुव वर्ण*);
    sqlite3MemdebugDump(Tcl_GetString(objv[1]));
  }
#यदि_अन्तः
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_malloc_count
**
** Return the total number of times malloc() has been called.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_malloc_गणनम्(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nMalloc = -1;
  यदि( objc!=1 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }
#यदि defined(SQLITE_MEMDEBUG)
  {
    बाह्य पूर्णाङ्क sqlite3_Memdebug_Malloc_गणनम्();
    nMalloc = sqlite3_Memdebug_Malloc_गणनम्();
  }
#यदि_अन्तः
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(nMalloc));
  प्रत्यागम TCL_OK;
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
  शून्य * client_दत्त,
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
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "COUNTER ?OPTIONS?");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &iFail) ) प्रत्यागम TCL_ERROR;

  प्रति(ii=2; ii<objc; ii+=2){
    Tcl_आकार nOption;
    वर्ण *zOption = Tcl_GetStringFromObj(objv[ii], &nOption);
    वर्ण *zErr = 0;

    यदि( nOption>1 && वाक्ये_तुलय(zOption, "-repeat", nOption)==0 ){
      यदि( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अन्यथा{
        यदि( Tcl_GetIntFromObj(interp, objv[ii+1], &nRepeat) ){
          प्रत्यागम TCL_ERROR;
        }
      }
    }अन्यथा यदि( nOption>1 && वाक्ये_तुलय(zOption, "-benigncnt", nOption)==0 ){
      यदि( ii==(objc-1) ){
        zErr = "option requires an argument: ";
      }अन्यथा{
        pBenignCnt = objv[ii+1];
      }
    }अन्यथा{
      zErr = "unknown option: ";
    }

    यदि( zErr ){
      Tcl_Append_परिणाम(interp, zErr, zOption, रिक्त);
      प्रत्यागम TCL_ERROR;
    }
  }
  
  nBenign = faultsimBenignFailures();
  nFail = faultsimFailures();
  faultsimConfig(iFail, nRepeat);

  यदि( pBenignCnt ){
    Tcl_Obj_समुच्चयः_Var2(interp, pBenignCnt, 0, Tcl_NewIntObj(nBenign), 0);
  }
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(nFail));
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_memdebug_pending
**
** Return the number of malloc() calls that will succeed before a 
** simulated failure occurs. A negative return value indicates that
** no malloc() failure is scheduled.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_pending(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nPending;
  यदि( objc!=1 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "");
    प्रत्यागम TCL_ERROR;
  }
  nPending = faultsimPending();
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(nPending));
  प्रत्यागम TCL_OK;
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
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_memdebug_title_गणनम्++;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "TITLE");
    प्रत्यागम TCL_ERROR;
  }
#यदि_परिभाषितम् SQLITE_MEMDEBUG
  {
    ध्रुव वर्ण *zTitle;
    बाह्य पूर्णाङ्क sqlite3MemdebugSettitle(ध्रुव वर्ण*);
    zTitle = Tcl_GetString(objv[1]);
    sqlite3MemdebugSettitle(zTitle);
  }
#यदि_अन्तः
  प्रत्यागम TCL_OK;
}

#परिभाषय MALLOC_LOG_चौकटाः  10 
#परिभाषय MALLOC_LOG_KEYINTS (                                              \
    10 * ((परिमाण(पूर्णाङ्क)>=परिमाण(शून्य*)) ? 1 : परिमाण(शून्य*)/परिमाण(पूर्णाङ्क))   \
)
स्थिर Tcl_Hash_सारणी aMallocLog;
स्थिर पूर्णाङ्क mallocLogEnabled = 0;

प्रकारनिर्देश संरचना MallocLog MallocLog;
संरचना MallocLog {
  पूर्णाङ्क nCall;
  पूर्णाङ्क nByte;
};

#यदि_परिभाषितम् SQLITE_MEMDEBUG
स्थिर शून्य test_memdebug_callback(पूर्णाङ्क nByte, पूर्णाङ्क nFrame, शून्य **aFrame){
  यदि( mallocLogEnabled ){
    MallocLog *pLog;
    Tcl_Hash_प्रविष्टिः *p_प्रविष्टिः;
    पूर्णाङ्क isNew;

    पूर्णाङ्क a_कुञ्चिका[MALLOC_LOG_KEYINTS];
    अचिह्नित पूर्णाङ्क n_कुञ्चिका = परिमाण(पूर्णाङ्क)*MALLOC_LOG_KEYINTS;

    स्मृतिं_पूरय(a_कुञ्चिका, 0, n_कुञ्चिका);
    यदि( (परिमाण(शून्य*)*nFrame)<n_कुञ्चिका ){
      n_कुञ्चिका = nFrame*परिमाण(शून्य*);
    }
    स्मृतिं_प्रतिलिख(a_कुञ्चिका, aFrame, n_कुञ्चिका);

    p_प्रविष्टिः = Tcl_Create_Hash_प्रविष्टिः(&aMallocLog, (ध्रुव वर्ण *)a_कुञ्चिका, &isNew);
    यदि( isNew ){
      pLog = (MallocLog *)Tcl_Alloc(परिमाण(MallocLog));
      स्मृतिं_पूरय(pLog, 0, परिमाण(MallocLog));
      Tcl_समुच्चयः_Hash_मानम्(p_प्रविष्टिः, (Client_दत्त)pLog);
    }अन्यथा{
      pLog = (MallocLog *)Tcl_Get_Hash_मानम्(p_प्रविष्टिः);
    }

    pLog->nCall++;
    pLog->nByte += nByte;
  }
}
#यदि_अन्तः /* SQLITE_MEMDEBUG */

स्थिर शून्य test_memdebug_log_clear(शून्य){
  Tcl_Hash_अन्वेषणम् अन्वेषणम्;
  Tcl_Hash_प्रविष्टिः *p_प्रविष्टिः;
  प्रति(
    p_प्रविष्टिः=Tcl_First_Hash_प्रविष्टिः(&aMallocLog, &अन्वेषणम्);
    p_प्रविष्टिः;
    p_प्रविष्टिः=Tcl_अग्र_Hash_प्रविष्टिः(&अन्वेषणम्)
  ){
    MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मानम्(p_प्रविष्टिः);
    Tcl_Free((वर्ण *)pLog);
  }
  Tcl_Delete_Hash_सारणी(&aMallocLog);
  Tcl_Init_Hash_सारणी(&aMallocLog, MALLOC_LOG_KEYINTS);
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_memdebug_log(
  शून्य * client_दत्त,
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

  यदि( !isInit ){
#यदि_परिभाषितम् SQLITE_MEMDEBUG
    बाह्य शून्य sqlite3MemdebugBacktraceCallback(
        शून्य (*xBacktrace)(पूर्णाङ्क, पूर्णाङ्क, शून्य **));
    sqlite3MemdebugBacktraceCallback(test_memdebug_callback);
#यदि_अन्तः
    Tcl_Init_Hash_सारणी(&aMallocLog, MALLOC_LOG_KEYINTS);
    isInit = 1;
  }

  यदि( objc<2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "SUB-COMMAND ...");
  }
  यदि( Tcl_Get_सूचकांक_From_Obj(interp, objv[1], MB_strs, "sub-command", 0, &iSub) ){
    प्रत्यागम TCL_ERROR;
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
      Tcl_Hash_प्रविष्टिः *p_प्रविष्टिः;
      Tcl_Obj *pRet = Tcl_NewObj();

      प्रतिज्ञा(परिमाण(Tcl_WideInt)>=परिमाण(शून्य*));

      प्रति(
        p_प्रविष्टिः=Tcl_First_Hash_प्रविष्टिः(&aMallocLog, &अन्वेषणम्);
        p_प्रविष्टिः;
        p_प्रविष्टिः=Tcl_अग्र_Hash_प्रविष्टिः(&अन्वेषणम्)
      ){
        Tcl_Obj *apElem[MALLOC_LOG_चौकटाः+2];
        MallocLog *pLog = (MallocLog *)Tcl_Get_Hash_मानम्(p_प्रविष्टिः);
        Tcl_WideInt *a_कुञ्चिका = (Tcl_WideInt *)Tcl_Get_Hash_कुञ्चिका(&aMallocLog, p_प्रविष्टिः);
        पूर्णाङ्क ii;
  
        apElem[0] = Tcl_NewIntObj(pLog->nCall);
        apElem[1] = Tcl_NewIntObj(pLog->nByte);
        प्रति(ii=0; ii<MALLOC_LOG_चौकटाः; ii++){
          apElem[ii+2] = Tcl_NewWideIntObj(a_कुञ्चिका[ii]);
        }

        Tcl_ListObjAppendElement(interp, pRet,
            Tcl_NewListObj(MALLOC_LOG_चौकटाः+2, apElem)
        );
      }

      Tcl_समुच्चयः_Obj_परिणाम(interp, pRet);
      भङ्ग;
    }
    पक्ष MB_LOG_CLEAR: {
      test_memdebug_log_clear();
      भङ्ग;
    }

    पक्ष MB_LOG_SYNC: {
#यदि_परिभाषितम् SQLITE_MEMDEBUG
      बाह्य शून्य sqlite3MemdebugSync();
      test_memdebug_log_clear();
      mallocLogEnabled = 1;
      sqlite3MemdebugSync();
#यदि_अन्तः
      भङ्ग;
    }
  }

  प्रत्यागम TCL_OK;
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
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क sz, N;
  Tcl_Obj *pRes;
  स्थिर वर्ण *buf = 0;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "SIZE N");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &sz) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[2], &N) ) प्रत्यागम TCL_ERROR;
  मुक्त(buf);
  buf = 0;

  /* Set the return value */
  pRes = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.szPage));
  Tcl_ListObjAppendElement(0, pRes, Tcl_NewIntObj(sqlite3GlobalConfig.nPage));
  Tcl_समुच्चयः_Obj_परिणाम(interp, pRes);

  यदि( sz<0 ){
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, (शून्य*)0, 0, 0);
  }अन्यथा{
    buf = स्मृतिं_देहि( sz*N );
    sqlite3_config(SQLITE_CONFIG_PAGECACHE, buf, sz, N);
  }
  प्रत्यागम TCL_OK;
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
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क installFlag;
  पूर्णाङ्क discardChance = 0;
  पूर्णाङ्क prng_बीजम्_2 = 0;
  पूर्णाङ्क उच्चः_Stress = 0;
  बाह्य शून्य installTestPCache(पूर्णाङ्क,अचिह्नित,अचिह्नित,अचिह्नित);
  यदि( objc<2 || objc>5 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, 
        "INSTALLFLAG DISCARDCHANCE PRNGSEEED HIGHSTRESS");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &installFlag) ) प्रत्यागम TCL_ERROR;
  यदि( objc>=3 && Tcl_GetIntFromObj(interp, objv[2], &discardChance) ){
     प्रत्यागम TCL_ERROR;
  }
  यदि( objc>=4 && Tcl_GetIntFromObj(interp, objv[3], &prng_बीजम्_2) ){
     प्रत्यागम TCL_ERROR;
  }
  यदि( objc>=5 && Tcl_GetIntFromObj(interp, objv[4], &उच्चः_Stress) ){
    प्रत्यागम TCL_ERROR;
  }
  यदि( discardChance<0 || discardChance>100 ){
    Tcl_Append_परिणाम(interp, "discard-chance should be between 0 and 100",
                     (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  installTestPCache(installFlag, (अचिह्नित)discardChance, (अचिह्नित)prng_बीजम्_2,
                    (अचिह्नित)उच्चः_Stress);
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_config_memstatus BOOLEAN
**
** Enable or disable memory status reporting using SQLITE_CONFIG_MEMSTATUS.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_memstatus(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क enable, rc;
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "BOOLEAN");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[1], &enable) ) प्रत्यागम TCL_ERROR;
  rc = sqlite3_config(SQLITE_CONFIG_MEMSTATUS, enable);
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_config_lookaside  SIZE  COUNT
**
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_lookaside(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क sz, cnt;
  Tcl_Obj *pRet;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "SIZE COUNT");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &sz) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[2], &cnt) ) प्रत्यागम TCL_ERROR;
  pRet = Tcl_NewObj();
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.szLookaside)
  );
  Tcl_ListObjAppendElement(
      interp, pRet, Tcl_NewIntObj(sqlite3GlobalConfig.nLookaside)
  );
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, sz, cnt);
  Tcl_समुच्चयः_Obj_परिणाम(interp, pRet);
  प्रत्यागम TCL_OK;
}


/*
** Usage:    sqlite3_db_config_lookaside  CONNECTION  BUFID  SIZE  COUNT
**
** There are two static buffers with BUFID 1 and 2.   Each static buffer
** is 10KB in size.  A BUFID of 0 indicates that the buffer should be NULL
** which will cause sqlite3_db_config() to allocate space on its own.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_db_config_lookaside(
  शून्य * client_दत्त,
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
  यदि( objc!=5 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "BUFID SIZE COUNT");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[2], &bufid) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[3], &sz) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, objv[4], &cnt) ) प्रत्यागम TCL_ERROR;
  यदि( bufid==0 ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, (शून्य*)0, sz, cnt);
  }अन्यथा यदि( bufid>=1 && bufid<=2 && sz*cnt<=परिमाण(azBuf[0]) ){
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_LOOKASIDE, azBuf[bufid], sz,cnt);
  }अन्यथा{
    Tcl_Append_परिणाम(interp, "illegal arguments - see documentation", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(rc));
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_config_heap NBYTE NMINALLOC
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_heap(
  शून्य * client_दत्त, 
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
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "NBYTE NMINALLOC");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) प्रत्यागम TCL_ERROR;
  यदि( Tcl_GetIntFromObj(interp, aArg[1], &nMinAlloc) ) प्रत्यागम TCL_ERROR;

  यदि( nByte==0 ){
    मुक्त( zBuf );
    zBuf = 0;
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, (शून्य*)0, 0, 0);
  }अन्यथा{
    zBuf = realloc(zBuf, nByte);
    rc = sqlite3_config(SQLITE_CONFIG_HEAP, zBuf, nByte, nMinAlloc);
  }

  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_config_heap_size NBYTE
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_heap_आकार(
  शून्य * client_दत्त, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क nByte;         /* Size to pass to sqlite3_config() */
  पूर्णाङ्क rc;            /* Return code of sqlite3_config() */

  Tcl_Obj * CONST *aArg = &objv[1];
  पूर्णाङ्क nArg = objc-1;

  यदि( nArg!=1 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "NBYTE");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, aArg[0], &nByte) ) प्रत्यागम TCL_ERROR;

  rc = sqlite3_config(SQLITE_CONFIG_WIN32_HEAPSIZE, nByte);

  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_config_error  [DB]
**
** Invoke sqlite3_config() or sqlite3_db_config() with invalid
** opcodes and verify that they return errors.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_error(
  शून्य * client_दत्त, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp*, ध्रुव वर्ण*, sqlite3**);

  यदि( objc!=2 && objc!=1 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "[DB]");
    प्रत्यागम TCL_ERROR;
  }
  यदि( objc==2 ){
    यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
    यदि( sqlite3_db_config(db, 99999)!=SQLITE_ERROR ){
      Tcl_Append_परिणाम(interp, 
            "sqlite3_db_config(db, 99999) does not return SQLITE_ERROR",
            (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
  }अन्यथा{
    यदि( sqlite3_config(99999)!=SQLITE_ERROR ){
      Tcl_Append_परिणाम(interp, 
          "sqlite3_config(99999) does not return SQLITE_ERROR",
          (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
  }
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_config_uri  BOOLEAN
**
** Enables or disables interpretation of URI parameters by default using
** SQLITE_CONFIG_URI.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_uri(
  शून्य * client_दत्त, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क bOpenUri;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "BOOL");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[1], &bOpenUri) ){
    प्रत्यागम TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_URI, bOpenUri);
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_config_cis  BOOLEAN
**
** Enables or disables the use of the covering-index scan optimization.
** SQLITE_CONFIG_COVERING_INDEX_SCAN.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_cis(
  शून्य * client_दत्त, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क bUseCis;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "BOOL");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[1], &bUseCis) ){
    प्रत्यागम TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_COVERING_सूचकांक_अवलोकनम्, bUseCis);
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_config_pmasz  INTEGER
**
** Set the minimum PMA size.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_config_pmasz(
  शून्य * client_दत्त, 
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क iPmaSz;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "BOOL");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[1], &iPmaSz) ){
    प्रत्यागम TCL_ERROR;
  }

  rc = sqlite3_config(SQLITE_CONFIG_PMASZ, iPmaSz);
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);

  प्रत्यागम TCL_OK;
}


/*
** Usage:    sqlite3_dump_memsys3  FILENAME
**           sqlite3_dump_memsys5  FILENAME
**
** Write a summary of unfreed memsys3 allocations to FILENAME.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_dump_memsys3(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "FILENAME");
    प्रत्यागम TCL_ERROR;
  }

  विकल्प( SQLITE_PTR_गन्तव्यम्_INT(client_दत्त) ){
    पक्ष 3: {
#यदि_परिभाषितम् SQLITE_ENABLE_MEMSYS3
      बाह्य शून्य sqlite3Memsys3Dump(ध्रुव वर्ण*);
      sqlite3Memsys3Dump(Tcl_GetString(objv[1]));
      भङ्ग;
#यदि_अन्तः
    }
    पक्ष 5: {
#यदि_परिभाषितम् SQLITE_ENABLE_MEMSYS5
      बाह्य शून्य sqlite3Memsys5Dump(ध्रुव वर्ण*);
      sqlite3Memsys5Dump(Tcl_GetString(objv[1]));
      भङ्ग;
#यदि_अन्तः
    }
  }
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_status  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_status() return
** code, the current value, and the high-water mark value.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_स्थितिः(
  शून्य * client_दत्त,
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
    { "SQLITE_STATUS_MEMORY_USED",         SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्         },
    { "SQLITE_STATUS_MALLOC_SIZE",         SQLITE_स्थितिः_MALLOC_आकार         },
    { "SQLITE_STATUS_PAGECACHE_USED",      SQLITE_स्थितिः_PAGECACHE_प्रयुक्तम्      },
    { "SQLITE_STATUS_PAGECACHE_OVERFLOW",  SQLITE_स्थितिः_PAGECACHE_अतिप्रवाहः  },
    { "SQLITE_STATUS_PAGECACHE_SIZE",      SQLITE_स्थितिः_PAGECACHE_आकार      },
    { "SQLITE_STATUS_SCRATCH_USED",        SQLITE_स्थितिः_SCRATCH_प्रयुक्तम्        },
    { "SQLITE_STATUS_SCRATCH_OVERFLOW",    SQLITE_स्थितिः_SCRATCH_अतिप्रवाहः    },
    { "SQLITE_STATUS_SCRATCH_SIZE",        SQLITE_स्थितिः_SCRATCH_आकार        },
    { "SQLITE_STATUS_PARSER_STACK",        SQLITE_स्थितिः_PARSER_राशिः        },
    { "SQLITE_STATUS_MALLOC_COUNT",        SQLITE_स्थितिः_MALLOC_गणनम्        },
  };
  Tcl_Obj *p_परिणाम;
  यदि( objc!=3 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "PARAMETER RESETFLAG");
    प्रत्यागम TCL_ERROR;
  }
  zOpName = Tcl_GetString(objv[1]);
  प्रति(i=0; i<Array_आकार(aOp); i++){
    यदि( वाक्यं_तुलय(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      भङ्ग;
    }
  }
  यदि( i>=Array_आकार(aOp) ){
    यदि( Tcl_GetIntFromObj(interp, objv[1], &op) ) प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[2], &resetFlag) ) प्रत्यागम TCL_ERROR;
  i_मानम् = 0;
  mx_मानम् = 0;
  rc = sqlite3_स्थितिः(op, &i_मानम्, &mx_मानम्, resetFlag);
  p_परिणाम = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(i_मानम्));
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(mx_मानम्));
  Tcl_समुच्चयः_Obj_परिणाम(interp, p_परिणाम);
  प्रत्यागम TCL_OK;
}

/*
** Usage:    sqlite3_db_status  DATABASE  OPCODE  RESETFLAG
**
** Return a list of three elements which are the sqlite3_db_status() return
** code, the current value, and the high-water mark value.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_db_स्थितिः(
  शून्य * client_दत्त,
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
    { "LOOKASIDE_USED",      SQLITE_DBSTATUS_LOOKASIDE_प्रयुक्तम्      },
    { "CACHE_USED",          SQLITE_DBSTATUS_CACHE_प्रयुक्तम्          },
    { "SCHEMA_USED",         SQLITE_DBSTATUS_SCHEMA_प्रयुक्तम्         },
    { "STMT_USED",           SQLITE_DBSTATUS_STMT_प्रयुक्तम्           },
    { "LOOKASIDE_HIT",       SQLITE_DBSTATUS_LOOKASIDE_लब्धम्       },
    { "LOOKASIDE_MISS_SIZE", SQLITE_DBSTATUS_LOOKASIDE_अलब्धिः_आकार },
    { "LOOKASIDE_MISS_FULL", SQLITE_DBSTATUS_LOOKASIDE_अलब्धिः_पूर्णम् },
    { "CACHE_HIT",           SQLITE_DBSTATUS_CACHE_लब्धम्           },
    { "CACHE_MISS",          SQLITE_DBSTATUS_CACHE_अलब्धिः          },
    { "CACHE_WRITE",         SQLITE_DBSTATUS_CACHE_WRITE         },
    { "DEFERRED_FKS",        SQLITE_DBSTATUS_DEFERRED_FKS        },
    { "CACHE_USED_SHARED",   SQLITE_DBSTATUS_CACHE_प्रयुक्तम्_SHARED   },
    { "CACHE_SPILL",         SQLITE_DBSTATUS_CACHE_SPILL         },
    { "TEMPBUF_SPILL",       SQLITE_DBSTATUS_TEMPBUF_SPILL       },
  };
  Tcl_Obj *p_परिणाम;
  यदि( objc!=4 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB PARAMETER RESETFLAG");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
  zOpName = Tcl_GetString(objv[2]);
  यदि( memcmp(zOpName, "SQLITE_", 7)==0 ) zOpName += 7;
  यदि( memcmp(zOpName, "DBSTATUS_", 9)==0 ) zOpName += 9;
  प्रति(i=0; i<Array_आकार(aOp); i++){
    यदि( वाक्यं_तुलय(aOp[i].zName, zOpName)==0 ){
      op = aOp[i].op;
      भङ्ग;
    }
  }
  यदि( i>=Array_आकार(aOp) ){
    यदि( Tcl_GetIntFromObj(interp, objv[2], &op) ) प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetBooleanFromObj(interp, objv[3], &resetFlag) ) प्रत्यागम TCL_ERROR;
  i_मानम् = 0;
  mx_मानम् = 0;
  rc = sqlite3_db_स्थितिः(db, op, &i_मानम्, &mx_मानम्, resetFlag);
  p_परिणाम = Tcl_NewObj();
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(rc));
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(i_मानम्));
  Tcl_ListObjAppendElement(0, p_परिणाम, Tcl_NewIntObj(mx_मानम्));
  Tcl_समुच्चयः_Obj_परिणाम(interp, p_परिणाम);
  प्रत्यागम TCL_OK;
}

/*
** install_malloc_faultsim BOOLEAN
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_install_malloc_faultsim(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc;
  पूर्णाङ्क isInstall;

  यदि( objc!=2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "BOOLEAN");
    प्रत्यागम TCL_ERROR;
  }
  यदि( TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[1], &isInstall) ){
    प्रत्यागम TCL_ERROR;
  }
  rc = faultsimInstall(isInstall);
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  प्रत्यागम TCL_OK;
}

/*
** sqlite3_install_memsys3
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_install_memsys3(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क rc = SQLITE_MISUSE;
#यदि_परिभाषितम् SQLITE_ENABLE_MEMSYS3
  ध्रुव sqlite3_mem_methods *sqlite3MemGetMemsys3(शून्य);
  rc = sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetMemsys3());
#यदि_अन्तः
  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3ErrName(rc), TCL_VOLATILE);
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfs_oom_test(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  बाह्य पूर्णाङ्क sqlite3_memdebug_vfs_oom_test;
  यदि( objc>2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "?INTEGER?");
    प्रत्यागम TCL_ERROR;
  }अन्यथा यदि( objc==2 ){
    पूर्णाङ्क iNew;
    यदि( Tcl_GetIntFromObj(interp, objv[1], &iNew) ) प्रत्यागम TCL_ERROR;
    sqlite3_memdebug_vfs_oom_test = iNew;
  }
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(sqlite3_memdebug_vfs_oom_test));
  प्रत्यागम TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest_malloc_Init(Tcl_Interp *interp){
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     पूर्णाङ्क client_दत्त;
  } aObjCmd[] = {
     { "sqlite3_malloc",             test_malloc                   ,0 },
     { "sqlite3_realloc",            test_realloc                  ,0 },
     { "sqlite3_free",               test_free                     ,0 },
     { "memset",                     test_memset                   ,0 },
     { "memget",                     test_memget                   ,0 },
     { "sqlite3_memory_used",        test_स्मृतिः_प्रयुक्तम्              ,0 },
     { "sqlite3_memory_highwater",   test_स्मृतिः_highwater         ,0 },
     { "sqlite3_memdebug_backtrace", test_memdebug_backtrace       ,0 },
     { "sqlite3_memdebug_dump",      test_memdebug_dump            ,0 },
     { "sqlite3_memdebug_fail",      test_memdebug_fail            ,0 },
     { "sqlite3_memdebug_pending",   test_memdebug_pending         ,0 },
     { "sqlite3_memdebug_settitle",  test_memdebug_settitle        ,0 },
     { "sqlite3_memdebug_malloc_count", test_memdebug_malloc_गणनम् ,0 },
     { "sqlite3_memdebug_log",       test_memdebug_log             ,0 },
     { "sqlite3_config_pagecache",   test_config_pagecache         ,0 },
     { "sqlite3_config_alt_pcache",  test_alt_pcache               ,0 },
     { "sqlite3_status",             test_स्थितिः                   ,0 },
     { "sqlite3_db_status",          test_db_स्थितिः                ,0 },
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
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Client_दत्त c = (Client_दत्त)SQLITE_INT_गन्तव्यम्_PTR(aObjCmd[i].client_दत्त);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  प्रत्यागम TCL_OK;
}
#यदि_अन्तः
