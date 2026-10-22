/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** A TCL Interface to SQLite.  Append this file to sqlite3.c and
** compile the whole thing to build a TCL-enabled version of SQLite.
**
** Compile-time options:
**
**  -DTCLSH         Add a "main()" routine that works as a tclsh.
**
**  -DTCLSH_INIT_PROC=name
**
**                  Invoke name(interp) to initialize the Tcl interpreter.
**                  If name(interp) returns a non-NULL string, then run
**                  that string as a Tcl script to launch the application.
**                  If name(interp) returns NULL, then run the regular
**                  tclsh-emulator code.
*/
#నిర్వచితమైతే TCLSH_INIT_PROC
# define TCLSH 1
#అయితే_ముగింపు

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#ఒకవేళ defined(INCLUDE_MSVC_H)
# include "msvc.h"
#అయితే_ముగింపు

/****** Copy of tclsqlite.h ******/
#ఒకవేళ defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#లేకపోతే
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#అయితే_ముగింపు
/* Compatability between Tcl8.6 and Tcl9.0 */
#ఒకవేళ TCL_MAJOR_సంస్కరణ==9
# define CONST స్థిరరాశి
#లేదావేళ !defined(Tcl_కొలత)
  రకనిర్వచనం పూర్ణాంకం Tcl_కొలత;
# ifndef Tcl_Bounce_Ref_లెక్క
#  define Tcl_Bounce_Ref_లెక్క(X) Tcl_Incr_Ref_లెక్క(X); Tcl_Decr_Ref_లెక్క(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#అయితే_ముగింపు
/**** End copy of tclsqlite.h ****/

#అన్తర్భావ <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#నిర్వచితం_కాకపోతే SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  రకనిర్వచనం గుర్తులేని అక్షరం u8;
# ifndef SQLITE_PTRSIZE
#   ఒకవేళ defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   లేకపోతే
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# ఒకవేళ defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    రకనిర్వచనం uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    రకనిర్వచనం గుర్తులేని పూర్ణాంకం uptr;
# లేకపోతే
    రకనిర్వచనం sqlite3_uint64 uptr;
# endif
#అయితే_ముగింపు
#అన్తర్భావ <ctype.h>

/* Used to get the current process ID */
#ఒకవేళ !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#లేదావేళ !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_సగటు
#   define WIN32_LEAN_AND_సగటు
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (పూర్ణాంకం)GetCurrentProcessId
#అయితే_ముగింపు

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#నిర్వచితమైతే BUILD_sqlite
#నిర్వచనం_రద్దు TCL_STORAGE_CLASS
#నిర్వచించు TCL_STORAGE_CLASS DLLEXPORT
#అయితే_ముగింపు /* BUILD_sqlite */

#నిర్వచించు అంకె_PREPARED_STMTS 10
#నిర్వచించు MAX_PREPARED_STMTS 100

/* Forward declaration */
రకనిర్వచనం నిర్మాణం SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#ఒకవేళ defined(SQLITE_ENABLE_QRF_లోపల_TCL)
#అన్తర్భావ "qrf.h"
#అయితే_ముగింపు

/*
** New SQL functions can be created as TCL scripts.  Each such function
** is described by an instance of the following structure.
**
** Variable eType may be set to SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT,
** SQLITE_BLOB or SQLITE_NULL. If it is SQLITE_NULL, then the implementation
** attempts to determine the type of the result based on the Tcl object.
** If it is SQLITE_TEXT or SQLITE_BLOB, then a text (sqlite3_result_text())
** or blob (sqlite3_result_blob()) is returned. If it is SQLITE_INTEGER
** or SQLITE_FLOAT, then an attempt is made to return an integer or float
** value, falling back to float and then text if this is not possible.
*/
రకనిర్వచనం నిర్మాణం SqlFunc SqlFunc;
నిర్మాణం SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  పూర్ణాంకం useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  పూర్ణాంకం eType;            /* Type of value to return */
  అక్షరం *zName;          /* Name of this function */
  SqlFunc *p_తదుపరి;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
రకనిర్వచనం నిర్మాణం SqlCollate SqlCollate;
నిర్మాణం SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  అక్షరం *zScript;        /* The script to be run */
  SqlCollate *p_తదుపరి;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
రకనిర్వచనం నిర్మాణం SqlPreparedStmt SqlPreparedStmt;
నిర్మాణం SqlPreparedStmt {
  SqlPreparedStmt *p_తదుపరి;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  పూర్ణాంకం nSql;                /* chars in zSql[] */
  స్థిరరాశి అక్షరం *zSql;        /* Text of the SQL statement */
  పూర్ణాంకం nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

రకనిర్వచనం నిర్మాణం IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
నిర్మాణం SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  అక్షరం *zBusy;               /* The busy callback routine */
  అక్షరం *zCommit;             /* The commit hook callback routine */
  అక్షరం *zTrace;              /* The trace callback routine */
  అక్షరం *zTraceV2;            /* The trace_v2 callback routine */
  అక్షరం *zProfile;            /* The profile callback routine */
  అక్షరం *z_పురోగతి;           /* The progress callback routine */
  అక్షరం *zBindFallback;       /* Callback to invoke on a binding miss */
  అక్షరం *zAuth;               /* The authorization callback routine */
  పూర్ణాంకం disableAuth;           /* Disable the authorizer if it exists */
  అక్షరం *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  పూర్ణాంకం rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmt_చివరి; /* Last statement in the list */
  పూర్ణాంకం maxStmt;               /* The next maximum number of stmtList */
  పూర్ణాంకం nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  పూర్ణాంకం n_అడుగు, n_క్రమీకరించు, n_సూచీ;  /* Statistics for most recent operation */
  పూర్ణాంకం nVMStep;               /* Another statistic for most recent operation */
  పూర్ణాంకం nTransaction;          /* Number of nested [transaction] methods */
  పూర్ణాంకం openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  పూర్ణాంకం nRef;                  /* Delete object when this reaches 0 */
#నిర్వచితమైతే SQLITE_TEST
  పూర్ణాంకం bLegacyPrepare;        /* True to use sqlite3_prepare() */
#అయితే_ముగింపు
};

నిర్మాణం IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  గుర్తులేని పూర్ణాంకం isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *p_తదుపరి;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
స్థిర పూర్ణాంకం strlen30(స్థిరరాశి అక్షరం *z){
  స్థిరరాశి అక్షరం *z2 = z;
  అయితే( *z2 ){ z2++; }
  తిరిగి 0x3fffffff & (పూర్ణాంకం)(z2 - z);
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
స్థిర శూన్య closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *p_తదుపరి;

  కొరకు(p=pDb->pIncrblob; p; p=p_తదుపరి){
    p_తదుపరి = p->p_తదుపరి;

    /* Note: Calling unregister here call Tcl_Close on the incrblob channel,
    ** which deletes the IncrblobChannel structure at *p. So do not
    ** call Tcl_Free() here.
    */
    Tcl_UnregisterChannel(pDb->interp, p->channel);
  }
}

/*
** Close an incremental blob channel.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI incrblobClose2(
  Client_దత్తాంశం instance_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం flags
){
  IncrblobChannel *p = (IncrblobChannel *)instance_దత్తాంశం;
  పూర్ణాంకం  rc;
  sqlite3 *db = p->pDb->db;

  ఒకవేళ( flags ){
    p->isClosed |= flags;
    తిరిగి TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  ఒకవేళ( p->p_తదుపరి ){
    p->p_తదుపరి->pPrev = p->pPrev;
  }
  ఒకవేళ( p->pPrev ){
    p->pPrev->p_తదుపరి = p->p_తదుపరి;
  }
  ఒకవేళ( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->p_తదుపరి;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((అక్షరం *)p);

  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3_errmsg(db), TCL_VOLATILE);
    తిరిగి TCL_ERROR;
  }
  తిరిగి TCL_OK;
}
స్థిర పూర్ణాంకం SQLITE_TCLAPI incrblobClose(
  Client_దత్తాంశం instance_దత్తాంశం,
  Tcl_Interp *interp
){
  తిరిగి incrblobClose2(instance_దత్తాంశం, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI incrblob_ఆదానం(
  Client_దత్తాంశం instance_దత్తాంశం,
  అక్షరం *buf,
  పూర్ణాంకం buf_కొలత,
  పూర్ణాంకం *error_సంకేతం_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_దత్తాంశం;
  sqlite3_int64 nRead = buf_కొలత;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  పూర్ణాంకం rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_బైట్లు(p->pBlob);
  ఒకవేళ( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  ఒకవేళ( nRead<=0 ){
    తిరిగి 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (శూన్య *)buf, (పూర్ణాంకం)nRead, (పూర్ణాంకం)p->iSeek);
  ఒకవేళ( rc!=SQLITE_OK ){
    *error_సంకేతం_Ptr = rc;
    తిరిగి -1;
  }

  p->iSeek += nRead;
  తిరిగి nRead;
}

/*
** Write data to an incremental blob channel.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI incrblobOutput(
  Client_దత్తాంశం instance_దత్తాంశం,
  స్థిరరాశి అక్షరం *buf,
  పూర్ణాంకం గమ్యం_Write,
  పూర్ణాంకం *error_సంకేతం_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_దత్తాంశం;
  sqlite3_int64 nWrite = గమ్యం_Write;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  పూర్ణాంకం rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_బైట్లు(p->pBlob);
  ఒకవేళ( (p->iSeek+nWrite)>nBlob ){
    *error_సంకేతం_Ptr = EINVAL;
    తిరిగి -1;
  }
  ఒకవేళ( nWrite<=0 ){
    తిరిగి 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (శూన్య*)buf,(పూర్ణాంకం)nWrite, (పూర్ణాంకం)p->iSeek);
  ఒకవేళ( rc!=SQLITE_OK ){
    *error_సంకేతం_Ptr = EIO;
    తిరిగి -1;
  }

  p->iSeek += nWrite;
  తిరిగి nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#ఒకవేళ TCL_MAJOR_సంస్కరణ==9
# define WideSeekProcType దీర్ఘ దీర్ఘ
#లేకపోతే
# define WideSeekProcType Tcl_WideInt
#అయితే_ముగింపు

/*
** Seek an incremental blob channel.
*/
స్థిర WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  Client_దత్తాంశం instance_దత్తాంశం,
  WideSeekProcType స్థానభ్రంశం,
  పూర్ణాంకం seek_బహుళకం,
  పూర్ణాంకం *error_సంకేతం_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_దత్తాంశం;

  ఎంపిక( seek_బహుళకం ){
    స్థితి SEEK_SET:
      p->iSeek = స్థానభ్రంశం;
      ఆపు;
    స్థితి SEEK_CUR:
      p->iSeek += స్థానభ్రంశం;
      ఆపు;
    స్థితి SEEK_END:
      p->iSeek = sqlite3_blob_బైట్లు(p->pBlob) + స్థానభ్రంశం;
      ఆపు;

    అప్రమేయ: ప్రతిజ్ఞా(!"Bad seekMode");
  }

  తిరిగి p->iSeek;
}
స్థిర పూర్ణాంకం SQLITE_TCLAPI incrblobSeek(
  Client_దత్తాంశం instance_దత్తాంశం,
  దీర్ఘ స్థానభ్రంశం,
  పూర్ణాంకం seek_బహుళకం,
  పూర్ణాంకం *error_సంకేతం_Ptr
){
  తిరిగి incrblobWideSeek(instance_దత్తాంశం,స్థానభ్రంశం,seek_బహుళకం,error_సంకేతం_Ptr);
}


స్థిర శూన్య SQLITE_TCLAPI incrblobWatch(
  Client_దత్తాంశం instance_దత్తాంశం,
  పూర్ణాంకం బహుళకం
){
  /* NO-OP */
}
స్థిర పూర్ణాంకం SQLITE_TCLAPI incrblobHandle(
  Client_దత్తాంశం instance_దత్తాంశం,
  పూర్ణాంకం dir,
  Client_దత్తాంశం *hPtr
){
  తిరిగి TCL_ERROR;
}

స్థిర Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_సంస్కరణ_5,             /*సంస్కరణ*/
  incrblobClose,                     /* closeProc                            */
  incrblob_ఆదానం,                     /* inputProc                            */
  incrblobOutput,                    /* outputProc                           */
  incrblobSeek,                      /* seekProc                             */
  0,                                 /* setOptionProc                        */
  0,                                 /* getOptionProc                        */
  incrblobWatch,                     /* watchProc (this is a no-op)          */
  incrblobHandle,                    /* getHandleProc (always returns error) */
  incrblobClose2,                    /* close2Proc                           */
  0,                                 /* blockModeProc                        */
  0,                                 /* flushProc                            */
  0,                                 /* handlerProc                          */
  incrblobWideSeek,                  /* wideSeekProc                         */
};

/*
** Create a new incrblob channel.
*/
స్థిర పూర్ణాంకం createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  స్థిరరాశి అక్షరం *zDb,
  స్థిరరాశి అక్షరం *z_పట్టిక,
  స్థిరరాశి అక్షరం *z_స్తంభం,
  sqlite_int64 i_అడ్డువరుస,
  పూర్ణాంకం isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  పూర్ణాంకం rc;
  పూర్ణాంకం flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  స్థిర పూర్ణాంకం లెక్క = 0;
  అక్షరం zChannel[64];

  rc = sqlite3_blob_open(db, zDb, z_పట్టిక, z_స్తంభం, i_అడ్డువరుస, !isReadonly, &pBlob);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    తిరిగి TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(పరిమాణం(IncrblobChannel));
  memset(p, 0, పరిమాణం(*p));
  p->pBlob = pBlob;
  ఒకవేళ( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(పరిమాణం(zChannel), zChannel, "incrblob_%d", ++లెక్క);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_రిజిస్టరు_Channel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->p_తదుపరి = pDb->pIncrblob;
  p->pPrev = 0;
  ఒకవేళ( p->p_తదుపరి ){
    p->p_తదుపరి->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_సమితి_ఫలితం(interp, (అక్షరం *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  తిరిగి TCL_OK;
}
#లేకపోతే  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
  #నిర్వచించు closeIncrblobChannels(pDb)
#అయితే_ముగింపు

/*
** Look at the script prefix in pCmd.  We will be executing this script
** after first appending one or more arguments.  This routine analyzes
** the script to see if it is safe to use Tcl_EvalObjv() on the script
** rather than the more general Tcl_EvalEx().  Tcl_EvalObjv() is much
** faster.
**
** Scripts that are safe to use with Tcl_EvalObjv() consists of a
** command name followed by zero or more arguments with no [...] or $
** or {...} or ; to be seen anywhere.  Most callback scripts consist
** of just a single procedure name and they meet this requirement.
*/
స్థిర పూర్ణాంకం సురక్షిత_గమ్యం_Use_Eval_Objv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  స్థిరరాశి అక్షరం *z;
  Tcl_కొలత n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  అయితే( n-- > 0 ){
    పూర్ణాంకం c = *(z++);
    ఒకవేళ( c=='$' || c=='[' || c==';' ) తిరిగి 0;
  }
  తిరిగి 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
స్థిర SqlFunc *కనుగొను_Sql_Func(SqliteDb *pDb, స్థిరరాశి అక్షరం *zName){
  SqlFunc *p, *pNew;
  పూర్ణాంకం nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( పరిమాణం(*pNew) + nName + 1 );
  pNew->zName = (అక్షరం*)&pNew[1];
  జ్ఞాపకం_నకలు(pNew->zName, zName, nName+1);
  కొరకు(p=pDb->pFunc; p; p=p->p_తదుపరి){
    ఒకవేళ( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((అక్షరం*)pNew);
      తిరిగి p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->p_తదుపరి = pDb->pFunc;
  pDb->pFunc = pNew;
  తిరిగి pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
స్థిర శూన్య dbFreeStmt(SqlPreparedStmt *pStmt){
#నిర్వచితమైతే SQLITE_TEST
  ఒకవేళ( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((అక్షరం *)pStmt->zSql);
  }
#అయితే_ముగింపు
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((అక్షరం *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
స్థిర శూన్య flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *p_తదుపరి;

  కొరకు(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=p_తదుపరి){
    p_తదుపరి = pPreStmt->p_తదుపరి;
    dbFreeStmt(pPreStmt);
  }
  pDb->nStmt = 0;
  pDb->stmt_చివరి = 0;
  pDb->stmtList = 0;
}

/*
** Increment the reference counter on the SqliteDb object. The reference
** should be released by calling delDatabaseRef().
*/
స్థిర శూన్య కూడు_Database_Ref(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
స్థిర శూన్య delDatabaseRef(SqliteDb *pDb){
  ప్రతిజ్ఞా( pDb->nRef>0 );
  pDb->nRef--;
  ఒకవేళ( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    అయితే( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->p_తదుపరి;
      ప్రతిజ్ఞా( pFunc->pDb==pDb );
      Tcl_Decr_Ref_లెక్క(pFunc->pScript);
      Tcl_Free((అక్షరం*)pFunc);
    }
    అయితే( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->p_తదుపరి;
      Tcl_Free((అక్షరం*)pCollate);
    }
    ఒకవేళ( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    ఒకవేళ( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    ఒకవేళ( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    ఒకవేళ( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    ఒకవేళ( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    ఒకవేళ( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    ఒకవేళ( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    ఒకవేళ( pDb->pUpdateHook ){
      Tcl_Decr_Ref_లెక్క(pDb->pUpdateHook);
    }
    ఒకవేళ( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_లెక్క(pDb->pPreUpdateHook);
    }
    ఒకవేళ( pDb->pRollbackHook ){
      Tcl_Decr_Ref_లెక్క(pDb->pRollbackHook);
    }
    ఒకవేళ( pDb->pWalHook ){
      Tcl_Decr_Ref_లెక్క(pDb->pWalHook);
    }
    ఒకవేళ( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_లెక్క(pDb->pCollateNeeded);
    }
    Tcl_Free((అక్షరం*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
స్థిర శూన్య SQLITE_TCLAPI DbDeleteCmd(శూన్య *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
స్థిర పూర్ణాంకం DbBusyHandler(శూన్య *cd, పూర్ణాంకం nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  పూర్ణాంకం rc;
  అక్షరం zVal[30];

  sqlite3_snprintf(పరిమాణం(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (అక్షరం*)0);
  ఒకవేళ( rc!=TCL_OK || atoi(Tcl_Get_String_ఫలితం(pDb->interp)) ){
    తిరిగి 0;
  }
  తిరిగి 1;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
స్థిర పూర్ణాంకం Db_పురోగతి_Handler(శూన్య *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( pDb->z_పురోగతి );
  rc = Tcl_Eval(pDb->interp, pDb->z_పురోగతి);
  ఒకవేళ( rc!=TCL_OK || atoi(Tcl_Get_String_ఫలితం(pDb->interp)) ){
    తిరిగి 1;
  }
  తిరిగి 0;
}
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_బిందువు) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
స్థిర శూన్య DbTraceHandler(శూన్య *cd, స్థిరరాశి అక్షరం *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_విలువ(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_ఫలితం(pDb->interp);
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
స్థిర పూర్ణాంకం DbTraceV2Handler(
  గుర్తులేని type, /* One of the SQLITE_TRACE_* event types. */
  శూన్య *cd,      /* The original context data pointer. */
  శూన్య *pd,      /* Primary event data, depends on event type. */
  శూన్య *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  ఎంపిక( type ){
    స్థితి SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      అక్షరం *zSql = (అక్షరం *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_లెక్క(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
      Tcl_Decr_Ref_లెక్క(pCmd);
      Tcl_Reset_ఫలితం(pDb->interp);
      ఆపు;
    }
    స్థితి SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_లెక్క(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
      Tcl_Decr_Ref_లెక్క(pCmd);
      Tcl_Reset_ఫలితం(pDb->interp);
      ఆపు;
    }
    స్థితి SQLITE_TRACE_అడ్డువరుస: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_లెక్క(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
      Tcl_Decr_Ref_లెక్క(pCmd);
      Tcl_Reset_ఫలితం(pDb->interp);
      ఆపు;
    }
    స్థితి SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_లెక్క(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
      Tcl_Decr_Ref_లెక్క(pCmd);
      Tcl_Reset_ఫలితం(pDb->interp);
      ఆపు;
    }
  }
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_బిందువు) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
స్థిర శూన్య DbProfileHandler(శూన్య *cd, స్థిరరాశి అక్షరం *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  అక్షరం zTm[100];

  sqlite3_snprintf(పరిమాణం(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_విలువ(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_ఫలితం(pDb->interp);
}
#అయితే_ముగింపు

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
స్థిర పూర్ణాంకం DbCommitHandler(శూన్య *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  పూర్ణాంకం rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  ఒకవేళ( rc!=TCL_OK || atoi(Tcl_Get_String_ఫలితం(pDb->interp)) ){
    తిరిగి 1;
  }
  తిరిగి 0;
}

స్థిర శూన్య DbRollbackHandler(శూన్య *client_దత్తాంశం){
  SqliteDb *pDb = (SqliteDb*)client_దత్తాంశం;
  ప్రతిజ్ఞా(pDb->pRollbackHook);
  ఒకవేళ( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
స్థిర పూర్ణాంకం DbWalHandler(
  శూన్య *client_దత్తాంశం,
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zDb,
  పూర్ణాంకం n_నమోదు
){
  పూర్ణాంకం ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)client_దత్తాంశం;
  Tcl_Interp *interp = pDb->interp;
  ప్రతిజ్ఞా(pDb->pWalHook);

  ప్రతిజ్ఞా( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_లెక్క(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(n_నమోదు));
  ఒకవేళ( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_Get_Obj_ఫలితం(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_లెక్క(p);

  తిరిగి ret;
}

#ఒకవేళ defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
స్థిర శూన్య సమితి_Test_Unlock_Notify_Vars(Tcl_Interp *interp, పూర్ణాంకం iArg, పూర్ణాంకం nArg){
  అక్షరం zBuf[64];
  sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "%d", iArg);
  Tcl_సమితి_Var(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(పరిమాణం(zBuf), zBuf, "%d", nArg);
  Tcl_సమితి_Var(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#లేకపోతే
# define సమితి_Test_Unlock_Notify_Vars(x,y,z)
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_UNLOCK_NOTIFY
స్థిర శూన్య DbUnlockNotify(శూన్య **apArg, పూర్ణాంకం nArg){
  పూర్ణాంకం i;
  కొరకు(i=0; i<nArg; i++){
    స్థిరరాశి పూర్ణాంకం flags = (TCL_EVAL_GLOBAL|TCL_EVAL_ప్రత్యక్ష);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    సమితి_Test_Unlock_Notify_Vars(pDb->interp, i, nArg);
    ప్రతిజ్ఞా( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_లెక్క(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
స్థిర శూన్య DbPreUpdateHandler(
  శూన్య *p,
  sqlite3 *db,
  పూర్ణాంకం op,
  స్థిరరాశి అక్షరం *zDb,
  స్థిరరాశి అక్షరం *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  స్థిర స్థిరరాశి అక్షరం *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ప్రతిజ్ఞా( (SQLITE_DELETE-1)/9 == 0 );
  ప్రతిజ్ఞా( (SQLITE_చొప్పించు-1)/9 == 1 );
  ప్రతిజ్ఞా( (SQLITE_UPDATE-1)/9 == 2 );
  ప్రతిజ్ఞా( pDb->pPreUpdateHook );
  ప్రతిజ్ఞా( db==pDb->db );
  ప్రతిజ్ఞా( op==SQLITE_చొప్పించు || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_లెక్క(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
  Tcl_Decr_Ref_లెక్క(pCmd);
}
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */

స్థిర శూన్య DbUpdateHandler(
  శూన్య *p,
  పూర్ణాంకం op,
  స్థిరరాశి అక్షరం *zDb,
  స్థిరరాశి అక్షరం *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  స్థిర స్థిరరాశి అక్షరం *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ప్రతిజ్ఞా( (SQLITE_DELETE-1)/9 == 0 );
  ప్రతిజ్ఞా( (SQLITE_చొప్పించు-1)/9 == 1 );
  ప్రతిజ్ఞా( (SQLITE_UPDATE-1)/9 == 2 );

  ప్రతిజ్ఞా( pDb->pUpdateHook );
  ప్రతిజ్ఞా( op==SQLITE_చొప్పించు || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_లెక్క(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
  Tcl_Decr_Ref_లెక్క(pCmd);
}

స్థిర శూన్య tclCollateNeeded(
  శూన్య *pCtx,
  sqlite3 *db,
  పూర్ణాంకం enc,
  స్థిరరాశి అక్షరం *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_లెక్క(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_లెక్క(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
స్థిర పూర్ణాంకం tclSqlCollate(
  శూన్య *pCtx,
  పూర్ణాంకం nA,
  స్థిరరాశి శూన్య *zA,
  పూర్ణాంకం nB,
  స్థిరరాశి శూన్య *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_లెక్క(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
  Tcl_Decr_Ref_లెక్క(pCmd);
  తిరిగి (atoi(Tcl_Get_String_ఫలితం(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
స్థిర శూన్య tclSqlFunc(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ**argv){
  SqlFunc *p = sqlite3_user_దత్తాంశం(context);
  Tcl_Obj *pCmd;
  పూర్ణాంకం i;
  పూర్ణాంకం rc;

  ఒకవేళ( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_లెక్క(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_లెక్క(pCmd);
  }లేకపోతే{
    /* If there are arguments to the function, make a shallow copy of the
    ** script object, lappend the arguments, then evaluate the copy.
    **
    ** By "shallow" copy, we mean only the outer list Tcl_Obj is duplicated.
    ** The new Tcl_Obj contains pointers to the original list elements.
    ** That way, when Tcl_EvalObjv() is run and shimmers the first element
    ** of the list to tclCmdNameType, that alternate representation will
    ** be preserved and reused on the next invocation.
    */
    Tcl_Obj **aArg;
    Tcl_కొలత nArg;
    ఒకవేళ( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_ఫలితం_error(context, Tcl_Get_String_ఫలితం(p->interp), -1);
      తిరిగి;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_లెక్క(pCmd);
    కొరకు(i=0; i<argc; i++){
      sqlite3_విలువ *p_లోపల = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      ఎంపిక( sqlite3_విలువ_type(p_లోపల) ){
        స్థితి SQLITE_BLOB: {
          పూర్ణాంకం బైట్లు = sqlite3_విలువ_బైట్లు(p_లోపల);
          pVal = Tcl_NewByteArrayObj(sqlite3_విలువ_blob(p_లోపల), బైట్లు);
          ఆపు;
        }
        స్థితి SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_విలువ_int64(p_లోపల);
          ఒకవేళ( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((పూర్ణాంకం)v);
          }లేకపోతే{
            pVal = Tcl_NewWideIntObj(v);
          }
          ఆపు;
        }
        స్థితి SQLITE_FLOAT: {
          ద్విగుణ r = sqlite3_విలువ_double(p_లోపల);
          pVal = Tcl_NewDoubleObj(r);
          ఆపు;
        }
        స్థితి SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          ఆపు;
        }
        అప్రమేయ: {
          పూర్ణాంకం బైట్లు = sqlite3_విలువ_బైట్లు(p_లోపల);
          pVal = Tcl_NewStringObj((అక్షరం *)sqlite3_విలువ_పాఠ్యం(p_లోపల), బైట్లు);
          ఆపు;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      ఒకవేళ( rc ){
        Tcl_Decr_Ref_లెక్క(pCmd);
        sqlite3_ఫలితం_error(context, Tcl_Get_String_ఫలితం(p->interp), -1);
        తిరిగి;
      }
    }
    ఒకవేళ( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
    Tcl_Decr_Ref_లెక్క(pCmd);
  }

  ఒకవేళ( TCL_BREAK==rc ){
    sqlite3_ఫలితం_null(context);
  }లేకపోతే ఒకవేళ( rc && rc!=TCL_RETURN ){
    sqlite3_ఫలితం_error(context, Tcl_Get_String_ఫలితం(p->interp), -1);
  }లేకపోతే{
    Tcl_Obj *pVar = Tcl_Get_Obj_ఫలితం(p->interp);
    Tcl_కొలత n;
    u8 *దత్తాంశం;
    స్థిరరాశి అక్షరం *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    అక్షరం c = zType[0];
    పూర్ణాంకం eType = p->eType;

    ఒకవేళ( eType==SQLITE_NULL ){
      ఒకవేళ( c=='b' && strcmp(zType,"bytearray")==0 && pVar->బైట్లు==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }లేకపోతే ఒకవేళ( (c=='b' && pVar->బైట్లు==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->బైట్లు==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }లేకపోతే ఒకవేళ( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }లేకపోతే{
        eType = SQLITE_పాఠ్యం;
      }
    }

    ఎంపిక( eType ){
      స్థితి SQLITE_BLOB: {
        దత్తాంశం = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_ఫలితం_blob64(context, దత్తాంశం, n, SQLITE_TRANSIENT);
        ఆపు;
      }
      స్థితి SQLITE_INTEGER: {
        Tcl_WideInt v;
        ఒకవేళ( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_ఫలితం_int64(context, v);
          ఆపు;
        }
        /* fall-through */
      }
      స్థితి SQLITE_FLOAT: {
        ద్విగుణ r;
        ఒకవేళ( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_ఫలితం_double(context, r);
          ఆపు;
        }
        /* fall-through */
      }
      అప్రమేయ: {
        దత్తాంశం = (గుర్తులేని అక్షరం *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_ఫలితం_text64(context, (అక్షరం *)దత్తాంశం, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        ఆపు;
      }
    }

  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
స్థిర పూర్ణాంకం auth_callback(
  శూన్య *pArg,
  పూర్ణాంకం సంకేతం_2,
  స్థిరరాశి అక్షరం *zArg1,
  స్థిరరాశి అక్షరం *zArg2,
  స్థిరరాశి అక్షరం *zArg3,
  స్థిరరాశి అక్షరం *zArg4
){
  స్థిరరాశి అక్షరం *z_సంకేతం;
  Tcl_DString str;
  పూర్ణాంకం rc;
  స్థిరరాశి అక్షరం *z_జవాబు;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  ఒకవేళ( pDb->disableAuth ) తిరిగి SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  ఎంపిక( సంకేతం_2 ){
    స్థితి SQLITE_COPY              : z_సంకేతం="SQLITE_COPY"; ఆపు;
    స్థితి SQLITE_CREATE_సూచీ      : z_సంకేతం="SQLITE_CREATE_INDEX"; ఆపు;
    స్థితి SQLITE_CREATE_పట్టిక      : z_సంకేతం="SQLITE_CREATE_TABLE"; ఆపు;
    స్థితి SQLITE_CREATE_తాత్కాలిక_సూచీ : z_సంకేతం="SQLITE_CREATE_TEMP_INDEX"; ఆపు;
    స్థితి SQLITE_CREATE_తాత్కాలిక_పట్టిక : z_సంకేతం="SQLITE_CREATE_TEMP_TABLE"; ఆపు;
    స్థితి SQLITE_CREATE_తాత్కాలిక_TRIGGER: z_సంకేతం="SQLITE_CREATE_TEMP_TRIGGER"; ఆపు;
    స్థితి SQLITE_CREATE_తాత్కాలిక_VIEW  : z_సంకేతం="SQLITE_CREATE_TEMP_VIEW"; ఆపు;
    స్థితి SQLITE_CREATE_TRIGGER    : z_సంకేతం="SQLITE_CREATE_TRIGGER"; ఆపు;
    స్థితి SQLITE_CREATE_VIEW       : z_సంకేతం="SQLITE_CREATE_VIEW"; ఆపు;
    స్థితి SQLITE_DELETE            : z_సంకేతం="SQLITE_DELETE"; ఆపు;
    స్థితి SQLITE_DROP_సూచీ        : z_సంకేతం="SQLITE_DROP_INDEX"; ఆపు;
    స్థితి SQLITE_DROP_పట్టిక        : z_సంకేతం="SQLITE_DROP_TABLE"; ఆపు;
    స్థితి SQLITE_DROP_తాత్కాలిక_సూచీ   : z_సంకేతం="SQLITE_DROP_TEMP_INDEX"; ఆపు;
    స్థితి SQLITE_DROP_తాత్కాలిక_పట్టిక   : z_సంకేతం="SQLITE_DROP_TEMP_TABLE"; ఆపు;
    స్థితి SQLITE_DROP_తాత్కాలిక_TRIGGER : z_సంకేతం="SQLITE_DROP_TEMP_TRIGGER"; ఆపు;
    స్థితి SQLITE_DROP_తాత్కాలిక_VIEW    : z_సంకేతం="SQLITE_DROP_TEMP_VIEW"; ఆపు;
    స్థితి SQLITE_DROP_TRIGGER      : z_సంకేతం="SQLITE_DROP_TRIGGER"; ఆపు;
    స్థితి SQLITE_DROP_VIEW         : z_సంకేతం="SQLITE_DROP_VIEW"; ఆపు;
    స్థితి SQLITE_చొప్పించు            : z_సంకేతం="SQLITE_INSERT"; ఆపు;
    స్థితి SQLITE_PRAGMA            : z_సంకేతం="SQLITE_PRAGMA"; ఆపు;
    స్థితి SQLITE_READ              : z_సంకేతం="SQLITE_READ"; ఆపు;
    స్థితి SQLITE_SELECT            : z_సంకేతం="SQLITE_SELECT"; ఆపు;
    స్థితి SQLITE_TRANSACTION       : z_సంకేతం="SQLITE_TRANSACTION"; ఆపు;
    స్థితి SQLITE_UPDATE            : z_సంకేతం="SQLITE_UPDATE"; ఆపు;
    స్థితి SQLITE_ATTACH            : z_సంకేతం="SQLITE_ATTACH"; ఆపు;
    స్థితి SQLITE_DETACH            : z_సంకేతం="SQLITE_DETACH"; ఆపు;
    స్థితి SQLITE_ALTER_పట్టిక       : z_సంకేతం="SQLITE_ALTER_TABLE"; ఆపు;
    స్థితి SQLITE_REINDEX           : z_సంకేతం="SQLITE_REINDEX"; ఆపు;
    స్థితి SQLITE_ANALYZE           : z_సంకేతం="SQLITE_ANALYZE"; ఆపు;
    స్థితి SQLITE_CREATE_VTABLE     : z_సంకేతం="SQLITE_CREATE_VTABLE"; ఆపు;
    స్థితి SQLITE_DROP_VTABLE       : z_సంకేతం="SQLITE_DROP_VTABLE"; ఆపు;
    స్థితి SQLITE_FUNCTION          : z_సంకేతం="SQLITE_FUNCTION"; ఆపు;
    స్థితి SQLITE_SAVEPOINT         : z_సంకేతం="SQLITE_SAVEPOINT"; ఆపు;
    స్థితి SQLITE_RECURSIVE         : z_సంకేతం="SQLITE_RECURSIVE"; ఆపు;
    అప్రమేయ                       : z_సంకేతం="????"; ఆపు;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, z_సంకేతం);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_విలువ(&str));
  Tcl_DStringFree(&str);
  z_జవాబు = rc==TCL_OK ? Tcl_Get_String_ఫలితం(pDb->interp) : "SQLITE_DENY";
  ఒకవేళ( strcmp(z_జవాబు,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }లేకపోతే ఒకవేళ( strcmp(z_జవాబు,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }లేకపోతే ఒకవేళ( strcmp(z_జవాబు,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }లేకపోతే{
    rc = 999;
  }
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_AUTHORIZATION */

#ఒకవేళ 0
/*
** This routine reads a line of text from FILE in, stores
** the text in memory obtained from malloc() and returns a pointer
** to the text.  NULL is returned at end of file, or if malloc()
** fails.
**
** The interface is like "readline" but no command-line editing
** is done.
**
** copied from shell.c from '.import' command
*/
స్థిర అక్షరం *local_getline(అక్షరం *zPrompt, FILE *లోపల){
  అక్షరం *z_పంక్తి;
  పూర్ణాంకం n_పంక్తి;
  పూర్ణాంకం n;

  n_పంక్తి = 100;
  z_పంక్తి = జ్ఞాపకం_ఇవ్వు( n_పంక్తి );
  ఒకవేళ( z_పంక్తి==0 ) తిరిగి 0;
  n = 0;
  అయితే( 1 ){
    ఒకవేళ( n+100>n_పంక్తి ){
      n_పంక్తి = n_పంక్తి*2 + 100;
      z_పంక్తి = realloc(z_పంక్తి, n_పంక్తి);
      ఒకవేళ( z_పంక్తి==0 ) తిరిగి 0;
    }
    ఒకవేళ( fgets(&z_పంక్తి[n], n_పంక్తి - n, లోపల)==0 ){
      ఒకవేళ( n==0 ){
        విడుదల(z_పంక్తి);
        తిరిగి 0;
      }
      z_పంక్తి[n] = 0;
      ఆపు;
    }
    అయితే( z_పంక్తి[n] ){ n++; }
    ఒకవేళ( n>0 && z_పంక్తి[n-1]=='\n' ){
      n--;
      z_పంక్తి[n] = 0;
      ఆపు;
    }
  }
  z_పంక్తి = realloc( z_పంక్తి, n+1 );
  తిరిగి z_పంక్తి;
}
#అయితే_ముగింపు


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI DbTransPostCmd(
  Client_దత్తాంశం దత్తాంశం[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  పూర్ణాంకం ఫలితం                           /* Result of evaluating SCRIPT */
){
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి az_చివర[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)దత్తాంశం[0];
  పూర్ణాంకం rc = ఫలితం;
  స్థిరరాశి అక్షరం *z_చివర;

  pDb->nTransaction--;
  z_చివర = az_చివర[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  ఒకవేళ( sqlite3_exec(pDb->db, z_చివర, 0, 0, 0) ){
      /* This is a tricky scenario to handle. The most likely cause of an
      ** error is that the exec() above was an attempt to commit the
      ** top-level transaction that returned SQLITE_BUSY. Or, less likely,
      ** that an IO-error has occurred. In either case, throw a Tcl exception
      ** and try to rollback the transaction.
      **
      ** But it could also be that the user executed one or more BEGIN,
      ** COMMIT, SAVEPOINT, RELEASE or ROLLBACK commands that are confusing
      ** this method's logic. Not clear how this would be best handled.
      */
    ఒకవేళ( rc!=TCL_ERROR ){
      Tcl_Append_ఫలితం(interp, sqlite3_errmsg(pDb->db), (అక్షరం*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  తిరిగి rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
స్థిర పూర్ణాంకం dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  స్థిరరాశి అక్షరం *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  స్థిరరాశి అక్షరం **pz_బహిర్గతం              /* OUT: Pointer to next SQL statement */
){
  గుర్తులేని పూర్ణాంకం prepFlags = 0;
#నిర్వచితమైతే SQLITE_TEST
  ఒకవేళ( pDb->bLegacyPrepare ){
    తిరిగి sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pz_బహిర్గతం);
  }
#అయితే_ముగింపు
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  ఒకవేళ( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  తిరిగి sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pz_బహిర్గతం);
}

/*
** Search the cache for a prepared-statement object that implements the
** first SQL statement in the buffer pointed to by parameter zIn. If
** no such prepared-statement can be found, allocate and prepare a new
** one. In either case, bind the current values of the relevant Tcl
** variables to any $var, :var or @var variables in the statement. Before
** returning, set *ppPreStmt to point to the prepared-statement object.
**
** Output parameter *pzOut is set to point to the next SQL statement in
** buffer zIn, or to the '\0' byte at the end of zIn if there is no
** next statement.
**
** If successful, TCL_OK is returned. Otherwise, TCL_ERROR is returned
** and an error message loaded into interpreter pDb->interp.
*/
స్థిర పూర్ణాంకం dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  అక్షరం స్థిరరాశి *z_లోపల,                /* SQL to compile */
  అక్షరం స్థిరరాశి **pz_బహిర్గతం,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  స్థిరరాశి అక్షరం *zSql = z_లోపల;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  పూర్ణాంకం nSql;                       /* Length of zSql in bytes */
  పూర్ణాంకం nVar = 0;                   /* Number of variables in statement */
  పూర్ణాంకం iParm = 0;                  /* Next free entry in apParm */
  అక్షరం c;
  పూర్ణాంకం i;
  పూర్ణాంకం need_ఫలితం_Reset = 0;        /* Need to invoke Tcl_ResetResult() */
  పూర్ణాంకం rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  అయితే( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  కొరకు(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->p_తదుపరి){
    పూర్ణాంకం n = pPreStmt->nSql;
    ఒకవేళ( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pz_బహిర్గతం = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      ఒకవేళ( pPreStmt->pPrev ){
        pPreStmt->pPrev->p_తదుపరి = pPreStmt->p_తదుపరి;
      }లేకపోతే{
        pDb->stmtList = pPreStmt->p_తదుపరి;
      }
      ఒకవేళ( pPreStmt->p_తదుపరి ){
        pPreStmt->p_తదుపరి->pPrev = pPreStmt->pPrev;
      }లేకపోతే{
        pDb->stmt_చివరి = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_లెక్క(pStmt);
      ఆపు;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  ఒకవేళ( pPreStmt==0 ){
    పూర్ణాంకం nByte;

    ఒకవేళ( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pz_బహిర్గతం) ){
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( pStmt==0 ){
      ఒకవేళ( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        తిరిగి TCL_ERROR;
      }లేకపోతే{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        తిరిగి TCL_OK;
      }
    }

    ప్రతిజ్ఞా( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_లెక్క(pStmt);
    nByte = పరిమాణం(SqlPreparedStmt) + nVar*పరిమాణం(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (పూర్ణాంకం)(*pz_బహిర్గతం - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#నిర్వచితమైతే SQLITE_TEST
    ఒకవేళ( pPreStmt->zSql==0 ){
      అక్షరం *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      జ్ఞాపకం_నకలు(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#అయితే_ముగింపు
  }
  ప్రతిజ్ఞా( pPreStmt );
  ప్రతిజ్ఞా( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  ప్రతిజ్ఞా( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  కొరకు(i=1; i<=nVar; i++){
    స్థిరరాశి అక్షరం *zVar = sqlite3_bind_parameter_name(pStmt, i);
    ఒకవేళ( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      ఒకవేళ( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        పూర్ణాంకం rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_లెక్క(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        ఒకవేళ( need_ఫలితం_Reset ) Tcl_Reset_ఫలితం(interp);
        need_ఫలితం_Reset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_ప్రత్యక్ష);
        Tcl_Decr_Ref_లెక్క(pCmd);
        ఒకవేళ( rx==TCL_OK ){
          pVar = Tcl_Get_Obj_ఫలితం(interp);
        }లేకపోతే ఒకవేళ( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          ఆపు;
        }లేకపోతే{
          pVar = 0;
        }
      }
      ఒకవేళ( pVar ){
        Tcl_కొలత n;
        u8 *దత్తాంశం;
        స్థిరరాశి అక్షరం *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        ఒకవేళ( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->బైట్లు==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          దత్తాంశం = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, దత్తాంశం, n, SQLITE_STATIC);
          Tcl_Incr_Ref_లెక్క(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }లేకపోతే ఒకవేళ( c=='b' && pVar->బైట్లు==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          పూర్ణాంకం nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }లేకపోతే ఒకవేళ( c=='d' && strcmp(zType,"double")==0 ){
          ద్విగుణ r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }లేకపోతే ఒకవేళ( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }లేకపోతే{
          దత్తాంశం = (గుర్తులేని అక్షరం *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (అక్షరం *)దత్తాంశం, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_లెక్క(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }లేకపోతే{
        sqlite3_bind_null(pStmt, i);
      }
      ఒకవేళ( need_ఫలితం_Reset ) Tcl_Reset_ఫలితం(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  ఒకవేళ( need_ఫలితం_Reset && rc==TCL_OK ) Tcl_Reset_ఫలితం(pDb->interp);

  తిరిగి rc;
}

/*
** Release a statement reference obtained by calling dbPrepareAndBind().
** There should be exactly one call to this function for each call to
** dbPrepareAndBind().
**
** If the discard parameter is non-zero, then the statement is deleted
** immediately. Otherwise it is added to the LRU list and may be returned
** by a subsequent call to dbPrepareAndBind().
*/
స్థిర శూన్య dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  పూర్ణాంకం discard                     /* True to delete (not cache) the pPreStmt */
){
  పూర్ణాంకం i;

  /* Free the bound string and blob parameters */
  కొరకు(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_లెక్క(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  ఒకవేళ( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }లేకపోతే{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->p_తదుపరి = pDb->stmtList;
    pPreStmt->pPrev = 0;
    ఒకవేళ( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    ఒకవేళ( pDb->stmt_చివరి==0 ){
      ప్రతిజ్ఞా( pDb->nStmt==0 );
      pDb->stmt_చివరి = pPreStmt;
    }లేకపోతే{
      ప్రతిజ్ఞా( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    అయితే( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *p_చివరి = pDb->stmt_చివరి;
      pDb->stmt_చివరి = p_చివరి->pPrev;
      pDb->stmt_చివరి->p_తదుపరి = 0;
      pDb->nStmt--;
      dbFreeStmt(p_చివరి);
    }
  }
}

/*
** Structure used with dbEvalXXX() functions:
**
**   dbEvalInit()
**   dbEvalStep()
**   dbEvalFinalize()
**   dbEvalRowInfo()
**   dbEvalColumnValue()
*/
రకనిర్వచనం నిర్మాణం DbEvalContext DbEvalContext;
నిర్మాణం DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  స్థిరరాశి అక్షరం *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  పూర్ణాంకం n_నిలువు;                       /* Number of columns returned by pStmt */
  పూర్ణాంకం evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **ap_నిలువు_Name;            /* Array of column names */
};

#నిర్వచించు SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#నిర్వచించు SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
స్థిర శూన్య db_Release_స్తంభం_పేర్లు(DbEvalContext *p){
  ఒకవేళ( p->ap_నిలువు_Name ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<p->n_నిలువు; i++){
      Tcl_Decr_Ref_లెక్క(p->ap_నిలువు_Name[i]);
    }
    Tcl_Free((అక్షరం *)p->ap_నిలువు_Name);
    p->ap_నిలువు_Name = 0;
  }
  p->n_నిలువు = 0;
}

/*
** Initialize a DbEvalContext structure.
**
** If pVarName is not NULL, then it contains the name of a Tcl array
** variable. The "*" member of this array is set to a list containing
** the names of the columns returned by the statement as part of each
** call to dbEvalStep(), in order from left to right. e.g. if the names
** of the returned columns are a, b and c, it does the equivalent of the
** tcl command:
**
**     set ${pVarName}(*) {a b c}
*/
స్థిర శూన్య dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  పూర్ణాంకం evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, పరిమాణం(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_లెక్క(pSql);
  ఒకవేళ( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_లెక్క(pVarName);
  }
  p->evalFlags = evalFlags;
  కూడు_Database_Ref(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
స్థిర శూన్య db_Eval_అడ్డువరుస_Info(
  DbEvalContext *p,               /* Evaluation context */
  పూర్ణాంకం *pn_నిలువు,                     /* OUT: Number of column names */
  Tcl_Obj ***pap_నిలువు_Name           /* OUT: Array of column names */
){
  /* Compute column names */
  ఒకవేళ( 0==p->ap_నిలువు_Name ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    పూర్ణాంకం i;                        /* Iterator variable */
    పూర్ణాంకం n_నిలువు;                     /* Number of columns returned by pStmt */
    Tcl_Obj **ap_నిలువు_Name = 0;      /* Array of column names */

    p->n_నిలువు = n_నిలువు = sqlite3_స్తంభం_లెక్క(pStmt);
    ఒకవేళ( n_నిలువు>0 && (pap_నిలువు_Name || p->pVarName) ){
      ap_నిలువు_Name = (Tcl_Obj**)Tcl_Alloc( పరిమాణం(Tcl_Obj*)*n_నిలువు );
      కొరకు(i=0; i<n_నిలువు; i++){
        ap_నిలువు_Name[i] = Tcl_NewStringObj(sqlite3_స్తంభం_name(pStmt,i), -1);
        Tcl_Incr_Ref_లెక్క(ap_నిలువు_Name[i]);
      }
      p->ap_నిలువు_Name = ap_నిలువు_Name;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    ఒకవేళ( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *p_నిలువు_List = Tcl_NewObj();
      Tcl_Obj *p_నక్షత్రం = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_లెక్క(p_నిలువు_List);
      Tcl_Incr_Ref_లెక్క(p_నక్షత్రం);
      కొరకు(i=0; i<n_నిలువు; i++){
        Tcl_ListObjAppendElement(interp, p_నిలువు_List, ap_నిలువు_Name[i]);
      }
      ఒకవేళ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_Obj_సమితి_Var2(interp, p->pVarName, p_నక్షత్రం, p_నిలువు_List, 0);
      }లేకపోతే{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, ఖాళీ, 0);
        ఒకవేళ( !pDict ){
          pDict = Tcl_NewDictObj();
        }లేకపోతే ఒకవేళ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ఒకవేళ( Tcl_DictObjPut(interp, pDict, p_నక్షత్రం, p_నిలువు_List)==TCL_OK ){
          Tcl_Obj_సమితి_Var2(interp, p->pVarName, ఖాళీ, pDict, 0);
        }
        Tcl_Bounce_Ref_లెక్క(pDict);
      }
      Tcl_Decr_Ref_లెక్క(p_నక్షత్రం);
      Tcl_Decr_Ref_లెక్క(p_నిలువు_List);
    }
  }

  ఒకవేళ( pap_నిలువు_Name ){
    *pap_నిలువు_Name = p->ap_నిలువు_Name;
  }
  ఒకవేళ( pn_నిలువు ){
    *pn_నిలువు = p->n_నిలువు;
  }
}

/*
** Return one of TCL_OK, TCL_BREAK or TCL_ERROR. If TCL_ERROR is
** returned, then an error message is stored in the interpreter before
** returning.
**
** A return value of TCL_OK means there is a row of data available. The
** data may be accessed using dbEvalRowInfo() and dbEvalColumnValue(). This
** is analogous to a return of SQLITE_ROW from sqlite3_step(). If TCL_BREAK
** is returned, then the SQL script has finished executing and there are
** no further rows available. This is similar to SQLITE_DONE.
*/
స్థిర పూర్ణాంకం db_Eval_అడుగు(DbEvalContext *p){
  స్థిరరాశి అక్షరం *zPrevSql = 0;       /* Previous value of p->zSql */

  అయితే( p->zSql[0] || p->pPreStmt ){
    పూర్ణాంకం rc;
    ఒకవేళ( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      ఒకవేళ( rc!=TCL_OK ) తిరిగి rc;
    }లేకపోతే{
      పూర్ణాంకం rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_అడుగు(pStmt);
      ఒకవేళ( rcs==SQLITE_అడ్డువరుస ){
        తిరిగి TCL_OK;
      }
      ఒకవేళ( p->pVarName ){
        db_Eval_అడ్డువరుస_Info(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->n_అడుగు = sqlite3_stmt_స్థితి(pStmt,SQLITE_STMTSTATUS_FULLSCAN_అడుగు,1);
      pDb->n_క్రమీకరించు = sqlite3_stmt_స్థితి(pStmt,SQLITE_STMTSTATUS_క్రమీకరించు,1);
      pDb->n_సూచీ = sqlite3_stmt_స్థితి(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_స్థితి(pStmt,SQLITE_STMTSTATUS_VM_అడుగు,1);
      db_Release_స్తంభం_పేర్లు(p);
      p->pPreStmt = 0;

      ఒకవేళ( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#ఒకవేళ SQLITE_TEST
        ఒకవేళ( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          కొనసాగు;
        }
#అయితే_ముగింపు
        Tcl_సమితి_Obj_ఫలితం(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        తిరిగి TCL_ERROR;
      }లేకపోతే{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /*ముగిసిన*/
  తిరిగి TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
స్థిర శూన్య dbEvalFinalize(DbEvalContext *p){
  ఒకవేళ( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  ఒకవేళ( p->pVarName ){
    Tcl_Decr_Ref_లెక్క(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_లెక్క(p->pSql);
  db_Release_స్తంభం_పేర్లు(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
స్థిర Tcl_Obj *db_Eval_స్తంభం_విలువ(DbEvalContext *p, పూర్ణాంకం i_నిలువు){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  ఎంపిక( sqlite3_స్తంభం_type(pStmt, i_నిలువు) ){
    స్థితి SQLITE_BLOB: {
      పూర్ణాంకం బైట్లు = sqlite3_స్తంభం_బైట్లు(pStmt, i_నిలువు);
      స్థిరరాశి అక్షరం *zBlob = sqlite3_స్తంభం_blob(pStmt, i_నిలువు);
      ఒకవేళ( !zBlob ) బైట్లు = 0;
      తిరిగి Tcl_NewByteArrayObj((u8*)zBlob, బైట్లు);
    }
    స్థితి SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_స్తంభం_int64(pStmt, i_నిలువు);
      ఒకవేళ( v>=-2147483647 && v<=2147483647 ){
        తిరిగి Tcl_NewIntObj((పూర్ణాంకం)v);
      }లేకపోతే{
        తిరిగి Tcl_NewWideIntObj(v);
      }
    }
    స్థితి SQLITE_FLOAT: {
      తిరిగి Tcl_NewDoubleObj(sqlite3_స్తంభం_double(pStmt, i_నిలువు));
    }
    స్థితి SQLITE_NULL: {
      తిరిగి Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  తిరిగి Tcl_NewStringObj((అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(pStmt, i_నిలువు), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#ఒకవేళ TCL_MAJOR_సంస్కరణ>8 || !defined(TCL_MINOR_సంస్కరణ) \
                        || TCL_MINOR_సంస్కరణ>=6
# define SQLITE_TCL_NRE 1
స్థిర పూర్ణాంకం DbUseNre(శూన్య){
  పూర్ణాంకం major, minor;
  Tcl_Get_సంస్కరణ(&major, &minor, 0, 0);
  తిరిగి( (major==8 && minor>=6) || major>8 );
}
#లేకపోతే
/*
** Compiling using headers earlier than 8.6. In this case NR cannot be
** used, so DbUseNre() to always return zero. Add #defines for the other
** Tcl_NRxxx() functions to prevent them from causing compilation errors,
** even though the only invocations of them are within conditional blocks
** of the form:
**
**   if( DbUseNre() ) { ... }
*/
# define SQLITE_TCL_NRE 0
# define DbUseNre() 0
# define Tcl_NRAddCallback(a,b,c,d,e,f) (శూన్య)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (శూన్య)0
#అయితే_ముగింపు

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI Db_Eval_తదుపరి_Cmd(
  Client_దత్తాంశం దత్తాంశం[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  పూర్ణాంకం ఫలితం                           /* Result so far */
){
  పూర్ణాంకం rc = ఫలితం;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)దత్తాంశం[0];
  Tcl_Obj * స్థిరరాశి pScript = (Tcl_Obj *)దత్తాంశం[1];
  Tcl_Obj * స్థిరరాశి pVarName = p->pVarName;

  అయితే( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = db_Eval_అడుగు(p)) ){
    పూర్ణాంకం i;
    పూర్ణాంకం n_నిలువు;
    Tcl_Obj **ap_నిలువు_Name;
    db_Eval_అడ్డువరుస_Info(p, &n_నిలువు, &ap_నిలువు_Name);
    కొరకు(i=0; i<n_నిలువు; i++){
      ఒకవేళ( pVarName==0 ){
        Tcl_Obj_సమితి_Var2(interp, ap_నిలువు_Name[i], 0, db_Eval_స్తంభం_విలువ(p,i), 0);
      }లేకపోతే ఒకవేళ( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_స్తంభం_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        ఒకవేళ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(ap_నిలువు_Name[i]), 0);
        }లేకపోతే{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ఖాళీ, 0);
          ఒకవేళ( pDict ){
            ఒకవేళ( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            ఒకవేళ( Tcl_DictObjRemove(interp, pDict, ap_నిలువు_Name[i])==TCL_OK ){
              Tcl_Obj_సమితి_Var2(interp, pVarName, ఖాళీ, pDict, 0);
            }
            Tcl_Bounce_Ref_లెక్క(pDict);
          }
        }
      }లేకపోతే ఒకవేళ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_Obj_సమితి_Var2(interp, pVarName, ap_నిలువు_Name[i],
                       db_Eval_స్తంభం_విలువ(p,i), 0);
      }లేకపోతే{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ఖాళీ, 0);
        ఒకవేళ( !pDict ){
          pDict = Tcl_NewDictObj();
        }లేకపోతే ఒకవేళ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ఒకవేళ( Tcl_DictObjPut(interp, pDict, ap_నిలువు_Name[i],
                           db_Eval_స్తంభం_విలువ(p,i))==TCL_OK ){
          Tcl_Obj_సమితి_Var2(interp, pVarName, ఖాళీ, pDict, 0);
        }
        Tcl_Bounce_Ref_లెక్క(pDict);
      }
    }

    /* The required interpreter variables are now populated with the data
    ** from the current row. If using NRE, schedule callbacks to evaluate
    ** script pScript, then to invoke this function again to fetch the next
    ** row (or clean up if there is no next row or the script throws an
    ** exception). After scheduling the callbacks, return control to the
    ** caller.
    **
    ** If not using NRE, evaluate pScript directly and continue with the
    ** next iteration of this while(...) loop.  */
    ఒకవేళ( DbUseNre() ){
      Tcl_NRAddCallback(interp, Db_Eval_తదుపరి_Cmd, (శూన్య*)p, (శూన్య*)pScript, 0, 0);
      తిరిగి Tcl_NREvalObj(interp, pScript, 0);
    }లేకపోతే{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_లెక్క(pScript);
  dbEvalFinalize(p);
  Tcl_Free((అక్షరం *)p);

  ఒకవేళ( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_Reset_ఫలితం(interp);
    rc = TCL_OK;
  }
  తిరిగి rc;
}

/*
** This function is used by the implementations of the following database
** handle sub-commands:
**
**   $db update_hook ?SCRIPT?
**   $db wal_hook ?SCRIPT?
**   $db commit_hook ?SCRIPT?
**   $db preupdate hook ?SCRIPT?
*/
స్థిర శూన్య DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  ఒకవేళ( *ppHook ){
    Tcl_సమితి_Obj_ఫలితం(interp, *ppHook);
    ఒకవేళ( pArg ){
      Tcl_Decr_Ref_లెక్క(*ppHook);
      *ppHook = 0;
    }
  }
  ఒకవేళ( pArg ){
    ప్రతిజ్ఞా( !(*ppHook) );
    ఒకవేళ( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_లెక్క(*ppHook);
    }
  }

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
  sqlite3_preupdate_hook(db, (pDb->pPreUpdateHook?DbPreUpdateHandler:0), pDb);
#అయితే_ముగింపు
  sqlite3_update_hook(db, (pDb->pUpdateHook?DbUpdateHandler:0), pDb);
  sqlite3_rollback_hook(db, (pDb->pRollbackHook?DbRollbackHandler:0), pDb);
  sqlite3_wal_hook(db, (pDb->pWalHook?DbWalHandler:0), pDb);
}

/*
** Implementation of the "db format" command.
**
** Based on provided options, format the results of the SQL statement(s)
** provided into human-readable form using the Query Result Formatter (QRF)
** and return the resuling text.
**
** Syntax:    db format OPTIONS SQL
**
** OPTIONS may be:
**
**     -style ("auto"|"box"|"column"|...)      Output style
**     -esc ("auto"|"off"|"ascii"|"symbol")    How to deal with ctrl chars
**     -text ("auto"|"off"|"sql"|"csv"|...)    How to escape TEXT values
**     -title ("auto"|"off"|"sql"|...|"off")   How to escape column names
**     -blob ("auto"|"text"|"sql"|...)         How to escape BLOB values
**     -wordwrap ("auto"|"off"|"on")           Try to wrap at word boundry?
**     -textjsonb ("auto"|"off"|"on")          Auto-convert JSONB to text?
**     -splitcolumn ("auto"|"off"|"on")        Enable split-column mode
**     -defaultalign ("auto"|"left"|...)       Default alignment
**     -titalalign ("auto"|"left"|"right"|...) Default column name alignment
**     -border ("auto"|"off"|"on")             Border for box and table styles
**     -rowcount ("auto"|"off"|"on")           Show row counts
**     -wrap NUMBER                            Max width of any single column
**     -screenwidth NUMBER                     Width of the display TTY
**     -linelimit NUMBER                       Max lines for any cell
**     -charlimit NUMBER                       Content truncated to this size
**     -titlelimit NUMBER                      Max width of column titles
**     -multiinsert NUMBER                     Multi-row INSERT byte size
**     -align LIST-OF-ALIGNMENT                Alignment of columns
**     -widths LIST-OF-NUMBERS                 Widths for individual columns
**     -columnsep TEXT                         Column separator text
**     -rowsep TEXT                            Row separator text
**     -tablename TEXT                         Table name for style "insert"
**     -null TEXT                              Text for NULL values
**     -ifmt STRING                            Format for integer values
**     -fpfmt STRING                           Format for floating-point values
**
** A mapping from TCL "format" command options to sqlite3_qrf_spec fields
** is below.  Use this to reference the QRF documentation:
**
**     TCL Option        spec field
**     ----------        ----------
**     -style            eStyle
**     -esc              eEsc
**     -text             eText
**     -title            eTitle, bTitles
**     -blob             eBlob
**     -wordwrap         bWordWrap
**     -textjsonb        bTextJsonb
**     -splitcolumn      bSplitColumn
**     -defaultalign     eDfltAlign
**     -titlealign       eTitleAlign
**     -border           bBorder
**     -rowcount         bRowCount
**     -wrap             nWrap
**     -screenwidth      nScreenWidth
**     -linelimit        nLineLimit
**     -charlimit        nCharLimit
**     -titlelimit       nTitleLimit
**     -multiinsert      nMultiInsert
**     -align            nAlign, aAlign
**     -widths           nWidth, aWidth
**     -columnsep        zColumnSep
**     -rowsep           zRowSep
**     -tablename        zTableName
**     -null             zNull
**     -ifmt             zIFmt
**     -fpfmt            zFpFmt
**
** The TCL -title option maps to eTitle and bTitles as follows:
**
**     -title       eTitle              bTitles 
**     -------      -----------------   ------------
**     off          QRF_TEXT_Auto       QRF_Off
**     on           QRF_TEXT_Auto       QRF_On
**     always       QRF_TEXT_Auto       QRF_Always
**     auto         QRF_TEXT_Auto       QRF_On
**     csv          QRF_TEXT_Csv        QRF_On
**     html         QRF_TEXT_Html       QRF_On
**     json         QRF_TEXT_Json       QRF_On
**     plain        QRF_TEXT_Plain      QRF_On
**     relaxed      QRF_TEXT_Relaxed    QRF_On
**     sql          QRF_TEXT_Sql        QRF_On
**     tcl          QRF_TEXT_Tcl        QRF_On
**
** Note in particular that for the last seven entries ("csv" through "tcl"),
** the CLI .mode --title option sets bTitles to QRF_Always, whereas this TCL
** interface set it to QRF_On.
*/
స్థిర పూర్ణాంకం dbQrf(SqliteDb *pDb, పూర్ణాంకం objc, Tcl_Obj *స్థిరరాశి*objv){
#నిర్వచితం_కాకపోతే SQLITE_QRF_H
  Tcl_సమితి_ఫలితం(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  తిరిగి TCL_ERROR;
#లేకపోతే
  అక్షరం *z_ఫలితం = 0;             /* Result to be returned */
  స్థిరరాశి అక్షరం *zSql = 0;          /* SQL to run */
  పూర్ణాంకం i;                         /* Loop counter */
  పూర్ణాంకం rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  స్థిర స్థిరరాశి అక్షరం *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  స్థిర స్థిరరాశి గుర్తులేని అక్షరం aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_ఎడమ,
    QRF_ALIGN_మధ్య, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_కుడి,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_శిఖరం,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, పరిమాణం(qrf));
  qrf.i_సంస్కరణ = 2;
  qrf.pzOutput = &z_ఫలితం;
  కొరకు(i=2; i<objc; i++){
    స్థిరరాశి అక్షరం *zArg = Tcl_GetString(objv[i]);
    స్థిరరాశి అక్షరం *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    స్థిరరాశి గుర్తులేని అక్షరం aBoolMap[] = { 0, 2, 1, 2, 1 };
    ఒకవేళ( zArg[0]!='-' ){
      ఒకవేళ( zSql ){
        Tcl_Append_ఫలితం(pDb->interp, "unknown argument: ", zArg, (అక్షరం*)0);
        rc = TCL_ERROR;
        వెళ్ళు format_failed;
      }
      zSql  = zArg;
    }లేకపోతే ఒకవేళ( i==objc-1 ){
      Tcl_Append_ఫలితం(pDb->interp, "option has no argument: ", zArg,(అక్షరం*)0);
      rc = TCL_ERROR;
      వెళ్ళు format_failed;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-style")==0 ){
      స్థిర స్థిరరాశి అక్షరం *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      స్థిర గుర్తులేని అక్షరం aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_స్తంభం,
        QRF_STYLE_లెక్క,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_చొప్పించు,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_పంక్తి,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_పట్టిక
      };
      పూర్ణాంకం style;
      rc = Tcl_Get_సూచీ_From_Obj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-esc")==0 ){
      స్థిర స్థిరరాశి అక్షరం *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      స్థిర గుర్తులేని అక్షరం aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_చిహ్నం
      };
      పూర్ణాంకం esc;
      rc = Tcl_Get_సూచీ_From_Obj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      స్థిర స్థిరరాశి అక్షరం *az_పాఠ్యం[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      స్థిర గుర్తులేని అక్షరం a_పాఠ్యం_Map[] = {
        QRF_పాఠ్యం_Auto,      QRF_పాఠ్యం_Csv,       QRF_పాఠ్యం_Html,
        QRF_పాఠ్యం_Json,      QRF_పాఠ్యం_Plain,     QRF_పాఠ్యం_Relaxed,
        QRF_పాఠ్యం_Sql,       QRF_పాఠ్యం_Tcl
      };
      పూర్ణాంకం txt;
      పూర్ణాంకం k = zArg[2]=='e';
      rc = Tcl_Get_సూచీ_From_Obj(pDb->interp, objv[i+1], &az_పాఠ్యం[k*3], zArg,
                               0, &txt);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      ఒకవేళ( k ){
        qrf.e_పాఠ్యం = a_పాఠ్యం_Map[txt];
      }లేకపోతే ఒకవేళ( txt<=2 ){
        ప్రతిజ్ఞా( QRF_No==1 );
        ప్రతిజ్ఞా( QRF_Yes==2 );
        ప్రతిజ్ఞా( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_పాఠ్యం_Auto;
      }లేకపోతే{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = a_పాఠ్యం_Map[txt-3];
      }
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-blob")==0 ){
      స్థిర స్థిరరాశి అక్షరం *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      స్థిర గుర్తులేని అక్షరం aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_పాఠ్యం,      QRF_BLOB_Sql,
        QRF_BLOB_కొలత
      };
      పూర్ణాంకం blob;
      rc = Tcl_Get_సూచీ_From_Obj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-wordwrap")==0 ){
      పూర్ణాంకం v = 0;
      rc = Tcl_Get_సూచీ_From_Obj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      qrf.b_మాట_Wrap = aBoolMap[v];
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      పూర్ణాంకం v = 0;
      rc = Tcl_Get_సూచీ_From_Obj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      ఒకవేళ( zArg[1]=='t' ){
        qrf.b_పాఠ్యం_Jsonb = aBoolMap[v];
      }లేకపోతే ఒకవేళ( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }లేకపోతే ఒకవేళ( zArg[1]=='r' ){
        qrf.b_అడ్డువరుస_లెక్క = aBoolMap[v];
      }లేకపోతే{
        qrf.b_విభజించు_స్తంభం = aBoolMap[v];
      }
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      పూర్ణాంకం ax = 0;
      rc = Tcl_Get_సూచీ_From_Obj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      ఒకవేళ( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }లేకపోతే{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      పూర్ణాంకం v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      ఒకవేళ( v<QRF_MIN_వెడల్పు ){
        v = QRF_MIN_వెడల్పు;
      }లేకపోతే ఒకవేళ( v>QRF_MAX_వెడల్పు ){
        v = QRF_MAX_వెడల్పు;
      }
      ఒకవేళ( zArg[1]=='w' ){
        qrf.nWrap = v;
      }లేకపోతే ఒకవేళ( zArg[1]=='s' ){
        qrf.n_Screen_వెడల్పు = v;
      }లేకపోతే ఒకవేళ( zArg[1]=='t' ){
        qrf.n_Title_హద్దు = v;
      }లేకపోతే{
        qrf.n_పంక్తి_హద్దు = v;
      }
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-charlimit")==0 ){
      పూర్ణాంకం v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      ఒకవేళ( v<0 ) v = 0;
      qrf.n_Char_హద్దు = v;
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-multiinsert")==0 ){
      పూర్ణాంకం v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      ఒకవేళ( v<0 ) v = 0;
      qrf.n_Multi_చొప్పించు = v;
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-align")==0 ){
      Tcl_కొలత n = 0;
      పూర్ణాంకం jj;
      rc = Tcl_List_Obj_నిడివి(pDb->interp, objv[i+1], &n);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*పరిమాణం(qrf.aAlign[0]) );
      ఒకవేళ( qrf.aAlign==0 ){
        Tcl_Append_ఫలితం(pDb->interp, "out of memory", (అక్షరం*)0);
        rc = TCL_ERROR;
        వెళ్ళు format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*పరిమాణం(qrf.aAlign[0]));
      qrf.nAlign = n;
      కొరకు(jj=0; jj<n; jj++){
        పూర్ణాంకం x;
        Tcl_Obj *p_పదం;
        rc = Tcl_List_Obj_సూచీ(pDb->interp, objv[i+1], jj, &p_పదం);
        ఒకవేళ( rc ) వెళ్ళు format_failed;
        rc = Tcl_Get_సూచీ_From_Obj(pDb->interp, p_పదం, azAlign,
                          "column alignment (-align)", 0, &x);
        ఒకవేళ( rc ) వెళ్ళు format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-widths")==0 ){
      Tcl_కొలత n = 0;
      పూర్ణాంకం jj;
      rc = Tcl_List_Obj_నిడివి(pDb->interp, objv[i+1], &n);
      ఒకవేళ( rc ) వెళ్ళు format_failed;
      sqlite3_free(qrf.a_వెడల్పు);
      qrf.a_వెడల్పు = sqlite3_malloc64( (n+1)*పరిమాణం(qrf.a_వెడల్పు[0]) );
      ఒకవేళ( qrf.a_వెడల్పు==0 ){
        Tcl_Append_ఫలితం(pDb->interp, "out of memory", (అక్షరం*)0);
        rc = TCL_ERROR;
        వెళ్ళు format_failed;
      }
      memset(qrf.a_వెడల్పు, 0, (n+1)*పరిమాణం(qrf.a_వెడల్పు[0]));
      qrf.n_వెడల్పు = n;
      కొరకు(jj=0; jj<n; jj++){
        Tcl_Obj *p_పదం;
        పూర్ణాంకం v;
        rc = Tcl_List_Obj_సూచీ(pDb->interp, objv[i+1], jj, &p_పదం);
        ఒకవేళ( rc ) వెళ్ళు format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, p_పదం, &v);
        ఒకవేళ( v<(-QRF_MAX_వెడల్పు) ){
          v = -QRF_MAX_వెడల్పు;
        }లేకపోతే ఒకవేళ( v>QRF_MAX_వెడల్పు ){
          v = QRF_MAX_వెడల్పు;
        }
        qrf.a_వెడల్పు[jj] = (పొట్టి పూర్ణాంకం)v;
      }
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-columnsep")==0 ){
      qrf.z_స్తంభం_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-rowsep")==0 ){
      qrf.z_అడ్డువరుస_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-tablename")==0 ){
      qrf.z_పట్టిక_Name = Tcl_GetString(objv[i+1]);
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-ifmt")==0 ){
      అక్షరం *zFmt = Tcl_GetString(objv[i+1]);
      ఒకవేళ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }లేకపోతే ఒకవేళ( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_Append_ఫలితం(pDb->interp, "bad format for -ifmt", (అక్షరం*)0);
        rc = TCL_ERROR;
        వెళ్ళు format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-fpfmt")==0 ){
      అక్షరం *zFmt = Tcl_GetString(objv[i+1]);
      ఒకవేళ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }లేకపోతే ఒకవేళ( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_Append_ఫలితం(pDb->interp, "bad format for -fpfmt", (అక్షరం*)0);
        rc = TCL_ERROR;
        వెళ్ళు format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }లేకపోతే ఒకవేళ( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.i_సంస్కరణ = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }లేకపోతే{
      Tcl_Append_ఫలితం(pDb->interp, "unknown option: ", zArg, (అక్షరం*)0);
      rc = TCL_ERROR;
      వెళ్ళు format_failed;
    }
  }
  అయితే( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    అక్షరం *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    ఒకవేళ( rc ) వెళ్ళు format_failed;
    ఒకవేళ( pStmt==0 ) కొనసాగు;
    rc = sqlite3_format_query_ఫలితం(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    ఒకవేళ( rc ){
      Tcl_సమితి_ఫలితం(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      వెళ్ళు format_failed;
    }
  }
  Tcl_సమితి_ఫలితం(pDb->interp, z_ఫలితం, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.a_వెడల్పు);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(z_ఫలితం);
  తిరిగి rc;

#అయితే_ముగింపు
}

/*
** The "sqlite" command below creates a new Tcl command for each
** connection it opens to an SQLite database.  This routine is invoked
** whenever one of those connection-specific commands is executed
** in Tcl.  For example, if you run Tcl code like this:
**
**       sqlite3 db1  "my_database"
**       db1 close
**
** The first command opens a connection to the "my_database" database
** and calls that connection "db1".  The second command causes this
** subroutine to be invoked.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI DbObjCmd(
  శూన్య *cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *స్థిరరాశి*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  పూర్ణాంకం choice;
  పూర్ణాంకం rc = TCL_OK;
  స్థిర స్థిరరాశి అక్షరం *DB_strs[] = {
    "authorizer",             "backup",                "bind_fallback",
    "busy",                   "cache",                 "changes",
    "close",                  "collate",               "collation_needed",
    "commit_hook",            "complete",              "config",
    "copy",                   "deserialize",           "enable_load_extension",
    "errorcode",              "erroroffset",           "eval",
    "exists",                 "format",                "function",
    "incrblob",               "interrupt",             "last_insert_rowid",
    "nullvalue",              "onecolumn",             "preupdate",
    "profile",                "progress",              "rekey",
    "restore",                "rollback_hook",         "serialize",
    "status",                 "timeout",               "total_changes",
    "trace",                  "trace_v2",              "transaction",
    "unlock_notify",          "update_hook",           "version",
    "wal_hook",               0                        
  };
  గణన DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_ఎక్కించు_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_అంతరాయం,            DB_చివరి_చొప్పించు_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_పురోగతి,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_స్థితి,                DB_TIMEOUT,              DB_మొత్తం_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_సంస్కరణ,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SUBCOMMAND ...");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    తిరిగి TCL_ERROR;
  }

  ఎంపిక( (గణన DB_enum)choice ){

  /*    $db authorizer ?CALLBACK?
  **
  ** Invoke the given callback to authorize each SQL operation as it is
  ** compiled.  5 arguments are appended to the callback before it is
  ** invoked:
  **
  **   (1) The authorization type (ex: SQLITE_CREATE_TABLE, SQLITE_INSERT, ...)
  **   (2) First descriptive name (depends on authorization type)
  **   (3) Second descriptive name
  **   (4) Name of the database (ex: "main", "temp")
  **   (5) Name of trigger that is doing the access
  **
  ** The callback should return one of the following strings: SQLITE_OK,
  ** SQLITE_IGNORE, or SQLITE_DENY.  Any other return value is an error.
  **
  ** If this method is invoked with no arguments, the current authorization
  ** callback string is returned.
  */
  స్థితి DB_AUTHORIZER: {
#నిర్వచితమైతే SQLITE_OMIT_AUTHORIZATION
    Tcl_Append_ఫలితం(interp, "authorization not available in this build",
                     (అక్షరం*)0);
    తిరిగి TCL_ERROR;
#లేకపోతే
    ఒకవేళ( objc>3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?CALLBACK?");
      తిరిగి TCL_ERROR;
    }లేకపోతే ఒకవేళ( objc==2 ){
      ఒకవేళ( pDb->zAuth ){
        Tcl_Append_ఫలితం(interp, pDb->zAuth, (అక్షరం*)0);
      }
    }లేకపోతే{
      అక్షరం *zAuth;
      Tcl_కొలత len;
      ఒకవేళ( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      ఒకవేళ( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->zAuth, zAuth, len+1);
      }లేకపోతే{
        pDb->zAuth = 0;
      }
      ఒకవేళ( pDb->zAuth ){
        రకనిర్వచనం పూర్ణాంకం (*sqlite3_auth_cb)(
           శూన్య*,పూర్ణాంకం,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,
           స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
        pDb->interp = interp;
        sqlite3_సమితి_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }లేకపోతే{
        sqlite3_సమితి_authorizer(pDb->db, 0, 0);
      }
    }
#అయితే_ముగింపు
    ఆపు;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  స్థితి DB_BACKUP: {
    స్థిరరాశి అక్షరం *zDestFile;
    స్థిరరాశి అక్షరం *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    ఒకవేళ( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }లేకపోతే ఒకవేళ( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }లేకపోతే{
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?DATABASE? FILENAME");
      తిరిగి TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    ఒకవేళ( rc!=SQLITE_OK ){
      Tcl_Append_ఫలితం(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (అక్షరం*)0);
      sqlite3_close(pDest);
      తిరిగి TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    ఒకవేళ( pBackup==0 ){
      Tcl_Append_ఫలితం(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (అక్షరం*)0);
      sqlite3_close(pDest);
      తిరిగి TCL_ERROR;
    }
    అయితే(  (rc = sqlite3_backup_అడుగు(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_ముగింపు(pBackup);
    ఒకవేళ( rc==SQLITE_పూర్తి ){
      rc = TCL_OK;
    }లేకపోతే{
      Tcl_Append_ఫలితం(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (అక్షరం*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    ఆపు;
  }

  /*    $db bind_fallback ?CALLBACK?
  **
  ** When resolving bind parameters in an SQL statement, if the parameter
  ** cannot be associated with a TCL variable then invoke CALLBACK with a
  ** single argument that is the name of the parameter and use the return
  ** value of the CALLBACK as the binding.  If CALLBACK returns something
  ** other than TCL_OK or TCL_ERROR then bind a NULL.
  **
  ** If CALLBACK is an empty string, then revert to the default behavior 
  ** which is to set the binding to NULL.
  **
  ** If CALLBACK returns an error, that causes the statement execution to
  ** abort.  Hence, to configure a connection so that it throws an error
  ** on an attempt to bind an unknown variable, do something like this:
  **
  **     proc bind_error {name} {error "no such variable: $name"}
  **     db bind_fallback bind_error
  */
  స్థితి DB_BIND_FALLBACK: {
    ఒకవేళ( objc>3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?CALLBACK?");
      తిరిగి TCL_ERROR;
    }లేకపోతే ఒకవేళ( objc==2 ){
      ఒకవేళ( pDb->zBindFallback ){
        Tcl_Append_ఫలితం(interp, pDb->zBindFallback, (అక్షరం*)0);
      }
    }లేకపోతే{
      అక్షరం *zCallback;
      Tcl_కొలత len;
      ఒకవేళ( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      ఒకవేళ( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->zBindFallback, zCallback, len+1);
      }లేకపోతే{
        pDb->zBindFallback = 0;
      }
    }
    ఆపు;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  స్థితి DB_BUSY: {
    ఒకవేళ( objc>3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "CALLBACK");
      తిరిగి TCL_ERROR;
    }లేకపోతే ఒకవేళ( objc==2 ){
      ఒకవేళ( pDb->zBusy ){
        Tcl_Append_ఫలితం(interp, pDb->zBusy, (అక్షరం*)0);
      }
    }లేకపోతే{
      అక్షరం *zBusy;
      Tcl_కొలత len;
      ఒకవేళ( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      ఒకవేళ( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->zBusy, zBusy, len+1);
      }లేకపోతే{
        pDb->zBusy = 0;
      }
      ఒకవేళ( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }లేకపోతే{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    ఆపు;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  స్థితి DB_CACHE: {
    అక్షరం *subCmd;
    పూర్ణాంకం n;

    ఒకవేళ( objc<=2 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "cache option ?arg?");
      తిరిగి TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    ఒకవేళ( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      ఒకవేళ( objc!=3 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "flush");
        తిరిగి TCL_ERROR;
      }లేకపోతే{
        flushStmtCache( pDb );
      }
    }లేకపోతే ఒకవేళ( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      ఒకవేళ( objc!=4 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "size n");
        తిరిగి TCL_ERROR;
      }లేకపోతే{
        ఒకవేళ( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_Append_ఫలితం( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (అక్షరం*)0);
          తిరిగి TCL_ERROR;
        }లేకపోతే{
          ఒకవేళ( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }లేకపోతే ఒకవేళ( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }లేకపోతే{
      Tcl_Append_ఫలితం( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
    ఆపు;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  స్థితి DB_CHANGES: {
    Tcl_Obj *p_ఫలితం;
    ఒకవేళ( objc!=2 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
      తిరిగి TCL_ERROR;
    }
    p_ఫలితం = Tcl_Get_Obj_ఫలితం(interp);
    Tcl_సమితి_Wide_Int_Obj(p_ఫలితం, sqlite3_changes64(pDb->db));
    ఆపు;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  స్థితి DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    ఆపు;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  స్థితి DB_COLLATE: {
    SqlCollate *pCollate;
    అక్షరం *zName;
    అక్షరం *zScript;
    Tcl_కొలత nScript;
    ఒకవేళ( objc!=4 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "NAME SCRIPT");
      తిరిగి TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( పరిమాణం(*pCollate) + nScript + 1 );
    ఒకవేళ( pCollate==0 ) తిరిగి TCL_ERROR;
    pCollate->interp = interp;
    pCollate->p_తదుపరి = pDb->pCollate;
    pCollate->zScript = (అక్షరం*)&pCollate[1];
    pDb->pCollate = pCollate;
    జ్ఞాపకం_నకలు(pCollate->zScript, zScript, nScript+1);
    ఒకవేళ( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      తిరిగి TCL_ERROR;
    }
    ఆపు;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  స్థితి DB_COLLATION_NEEDED: {
    ఒకవేళ( objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "SCRIPT");
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_లెక్క(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_లెక్క(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    ఆపు;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  స్థితి DB_COMMIT_HOOK: {
    ఒకవేళ( objc>3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?CALLBACK?");
      తిరిగి TCL_ERROR;
    }లేకపోతే ఒకవేళ( objc==2 ){
      ఒకవేళ( pDb->zCommit ){
        Tcl_Append_ఫలితం(interp, pDb->zCommit, (అక్షరం*)0);
      }
    }లేకపోతే{
      స్థిరరాశి అక్షరం *zCommit;
      Tcl_కొలత len;
      ఒకవేళ( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      ఒకవేళ( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->zCommit, zCommit, len+1);
      }లేకపోతే{
        pDb->zCommit = 0;
      }
      ఒకవేళ( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }లేకపోతే{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    ఆపు;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  స్థితి DB_COMPLETE: {
#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPLETE
    Tcl_Obj *p_ఫలితం;
    పూర్ణాంకం isComplete;
    ఒకవేళ( objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "SQL");
      తిరిగి TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    p_ఫలితం = Tcl_Get_Obj_ఫలితం(interp);
    Tcl_సమితి_Boolean_Obj(p_ఫలితం, isComplete);
#అయితే_ముగింపు
    ఆపు;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  స్థితి DB_CONFIG: {
    స్థిర స్థిరరాశి నిర్మాణం DbConfigChoices {
      స్థిరరాశి అక్షరం *zName;
      పూర్ణాంకం op;
    } aDbConfig[] = {
        { "defensive",          SQLITE_DBCONFIG_DEFENSIVE             },
        { "dqs_ddl",            SQLITE_DBCONFIG_DQS_DDL               },
        { "dqs_dml",            SQLITE_DBCONFIG_DQS_DML               },
        { "enable_fkey",        SQLITE_DBCONFIG_ENABLE_FKEY           },
        { "enable_qpsg",        SQLITE_DBCONFIG_ENABLE_QPSG           },
        { "enable_trigger",     SQLITE_DBCONFIG_ENABLE_TRIGGER        },
        { "enable_view",        SQLITE_DBCONFIG_ENABLE_VIEW           },
        { "fts3_tokenizer",     SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER },
        { "legacy_alter_table", SQLITE_DBCONFIG_LEGACY_ALTER_పట్టిక    },
        { "legacy_file_format", SQLITE_DBCONFIG_LEGACY_FILE_FORMAT    },
        { "load_extension",     SQLITE_DBCONFIG_ENABLE_ఎక్కించు_EXTENSION },
        { "no_ckpt_on_close",   SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE      },
        { "reset_database",     SQLITE_DBCONFIG_RESET_DATABASE        },
        { "trigger_eqp",        SQLITE_DBCONFIG_TRIGGER_EQP           },
        { "trusted_schema",     SQLITE_DBCONFIG_TRUSTED_SCHEMA        },
        { "writable_schema",    SQLITE_DBCONFIG_WRITABLE_SCHEMA       },
    };
    Tcl_Obj *p_ఫలితం;
    పూర్ణాంకం ii;
    ఒకవేళ( objc>4 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      p_ఫలితం = Tcl_NewListObj(0,0);
      కొరకు(ii=0; ii<పరిమాణం(aDbConfig)/పరిమాణం(aDbConfig[0]); ii++){
        పూర్ణాంకం v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, p_ఫలితం,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, p_ఫలితం,
           Tcl_NewIntObj(v));
      }
    }లేకపోతే{
      స్థిరరాశి అక్షరం *zOpt = Tcl_GetString(objv[2]);
      పూర్ణాంకం onoff = -1;
      పూర్ణాంకం v = 0;
      ఒకవేళ( zOpt[0]=='-' ) zOpt++;
      కొరకు(ii=0; ii<పరిమాణం(aDbConfig)/పరిమాణం(aDbConfig[0]); ii++){
        ఒకవేళ( strcmp(aDbConfig[ii].zName, zOpt)==0 ) ఆపు;
      }
      ఒకవేళ( ii>=పరిమాణం(aDbConfig)/పరిమాణం(aDbConfig[0]) ){
        Tcl_Append_ఫలితం(interp, "unknown config option: \"", zOpt,
                                "\"", (శూన్య*)0);
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( objc==4 ){
        ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          తిరిగి TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      p_ఫలితం = Tcl_NewIntObj(v);
    }
    Tcl_సమితి_Obj_ఫలితం(interp, p_ఫలితం);
    ఆపు;
  }

  /*    $db copy conflict-algorithm table filename ?SEPARATOR? ?NULLINDICATOR?
  **
  ** Copy data into table from filename, optionally using SEPARATOR
  ** as column separators.  If a column contains a null string, or the
  ** value of NULLINDICATOR, a NULL is inserted for the column.
  ** conflict-algorithm is one of the sqlite conflict algorithms:
  **    rollback, abort, fail, ignore, replace
  ** On success, return the number of lines processed, not necessarily same
  ** as 'db changes' due to conflict-algorithm selected.
  **
  ** This code is basically an implementation/enhancement of
  ** the sqlite3 shell.c ".import" command.
  **
  ** This command usage is equivalent to the sqlite2.x COPY statement,
  ** which imports file data into a table using the PostgreSQL COPY file format:
  **   $db copy $conflict_algorithm $table_name $filename \t \\N
  */
  స్థితి DB_COPY: {
    అక్షరం *z_పట్టిక;               /* Insert data into this table */
    అక్షరం *zFile;                /* The file from which to extract data */
    అక్షరం *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    పూర్ణాంకం n_నిలువు;                   /* Number of columns in the table */
    పూర్ణాంకం nByte;                  /* Number of bytes in an SQL string */
    పూర్ణాంకం i, j;                   /* Loop counters */
    పూర్ణాంకం nSep;                   /* Number of bytes in zSep[] */
    పూర్ణాంకం nNull;                  /* Number of bytes in zNull[] */
    అక్షరం *zSql;                 /* An SQL statement */
    అక్షరం *z_పంక్తి;                /* A single line of input from the file */
    అక్షరం **az_నిలువు;               /* zLine[] broken up into columns */
    స్థిరరాశి అక్షరం *zCommit;        /* How to commit changes */
    Tcl_Channel లోపల;             /* The input file */
    పూర్ణాంకం lineno = 0;             /* Line number of input file */
    అక్షరం z_పంక్తి_అంకె[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *p_ఫలితం;           /* interp result */

    స్థిరరాశి అక్షరం *zSep;
    స్థిరరాశి అక్షరం *zNull;
    ఒకవేళ( objc<5 || objc>7 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }లేకపోతే{
      zSep = "\t";
    }
    ఒకవేళ( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }లేకపోతే{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    z_పట్టిక = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    ఒకవేళ( nSep==0 ){
      Tcl_Append_ఫలితం(interp,"Error: non-null separator required for copy",
                       (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_Append_ఫలితం(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", z_పట్టిక);
    ఒకవేళ( zSql==0 ){
      Tcl_Append_ఫలితం(interp, "Error: no such table: ", z_పట్టిక, (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    ఒకవేళ( rc ){
      Tcl_Append_ఫలితం(interp, "Error: ", sqlite3_errmsg(pDb->db), (అక్షరం*)0);
      n_నిలువు = 0;
    }లేకపోతే{
      n_నిలువు = sqlite3_స్తంభం_లెక్క(pStmt);
    }
    sqlite3_finalize(pStmt);
    ఒకవేళ( n_నిలువు==0 ) {
      తిరిగి TCL_ERROR;
    }
    zSql = జ్ఞాపకం_ఇవ్వు( nByte + 50 + n_నిలువు*2 );
    ఒకవేళ( zSql==0 ) {
      Tcl_Append_ఫలితం(interp, "Error: can't malloc()", (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, z_పట్టిక);
    j = strlen30(zSql);
    కొరకు(i=1; i<n_నిలువు; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    విడుదల(zSql);
    ఒకవేళ( rc ){
      Tcl_Append_ఫలితం(interp, "Error: ", sqlite3_errmsg(pDb->db), (అక్షరం*)0);
      sqlite3_finalize(pStmt);
      తిరిగి TCL_ERROR;
    }
    లోపల = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    ఒకవేళ( లోపల==0 ){
      sqlite3_finalize(pStmt);
      తిరిగి TCL_ERROR;
    }
    Tcl_సమితి_Channel_Option(ఖాళీ, లోపల, "-translation", "auto");
    az_నిలువు = జ్ఞాపకం_ఇవ్వు( పరిమాణం(az_నిలువు[0])*(n_నిలువు+1) );
    ఒకవేళ( az_నిలువు==0 ) {
      Tcl_Append_ఫలితం(interp, "Error: can't malloc()", (అక్షరం*)0);
      Tcl_Close(interp, లోపల);
      తిరిగి TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_లెక్క(str);
    (శూన్య)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    అయితే( Tcl_GetsObj(లోపల, str)>=0 ) {
      అక్షరం *z;
      Tcl_కొలత byteLen;
      lineno++;
      z_పంక్తి = (అక్షరం *)Tcl_GetByteArrayFromObj(str, &byteLen);
      az_నిలువు[0] = z_పంక్తి;
      కొరకు(i=0, z=z_పంక్తి; *z; z++){
        ఒకవేళ( *z==zSep[0] && అక్షరశ్రేణి_పోలిక(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          ఒకవేళ( i<n_నిలువు ){
            az_నిలువు[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      ఒకవేళ( i+1!=n_నిలువు ){
        అక్షరం *zErr;
        పూర్ణాంకం nErr = strlen30(zFile) + 200;
        zErr = జ్ఞాపకం_ఇవ్వు(nErr);
        ఒకవేళ( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, n_నిలువు, i+1);
          Tcl_Append_ఫలితం(interp, zErr, (అక్షరం*)0);
          విడుదల(zErr);
        }
        zCommit = "ROLLBACK";
        ఆపు;
      }
      కొరకు(i=0; i<n_నిలువు; i++){
        /* check for null data, if so, bind as null */
        ఒకవేళ( (nNull>0 && strcmp(az_నిలువు[i], zNull)==0)
          || strlen30(az_నిలువు[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }లేకపోతే{
          sqlite3_bind_పాఠ్యం(pStmt, i+1, az_నిలువు[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_అడుగు(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_సమితి_Obj_నిడివి(str, 0);
      ఒకవేళ( rc!=SQLITE_OK ){
        Tcl_Append_ఫలితం(interp,"Error: ", sqlite3_errmsg(pDb->db), (అక్షరం*)0);
        zCommit = "ROLLBACK";
        ఆపు;
      }
    }
    Tcl_Decr_Ref_లెక్క(str);
    విడుదల(az_నిలువు);
    Tcl_Close(interp, లోపల);
    sqlite3_finalize(pStmt);
    (శూన్య)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    ఒకవేళ( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      p_ఫలితం = Tcl_Get_Obj_ఫలితం(interp);
      ఒకవేళ( Tcl_IsShared(p_ఫలితం) ){
        p_ఫలితం = Tcl_DuplicateObj(p_ఫలితం);
      }
      Tcl_సమితి_Int_Obj(p_ఫలితం, lineno);
      rc = TCL_OK;
    }లేకపోతే{
      /* failure, append lineno where failed */
      sqlite3_snprintf(పరిమాణం(z_పంక్తి_అంకె), z_పంక్తి_అంకె,"%d",lineno);
      Tcl_Append_ఫలితం(interp,", failed while processing line: ",z_పంక్తి_అంకె,
                       (అక్షరం*)0);
      rc = TCL_ERROR;
    }
    ఆపు;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  స్థితి DB_DESERIALIZE: {
#నిర్వచితమైతే SQLITE_OMIT_DESERIALIZE
    Tcl_Append_ఫలితం(interp, "MEMDB not available in this build",
                     (అక్షరం*)0);
    rc = TCL_ERROR;
#లేకపోతే
    స్థిరరాశి అక్షరం *zSchema = 0;
    Tcl_Obj *p_విలువ = 0;
    గుర్తులేని అక్షరం *pBA;
    గుర్తులేని అక్షరం *p_దత్తాంశం;
    Tcl_కొలత len;
    పూర్ణాంకం xrc;
    sqlite3_int64 mx_కొలత = 0;
    పూర్ణాంకం i;
    పూర్ణాంకం isReadonly = 0;


    ఒకవేళ( objc<3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      ఆపు;
    }
    కొరకు(i=2; i<objc-1; i++){
      స్థిరరాశి అక్షరం *z = Tcl_GetString(objv[i]);
      ఒకవేళ( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        ఒకవేళ( rc ) వెళ్ళు deserialize_error;
        mx_కొలత = x;
        కొనసాగు;
      }
      ఒకవేళ( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        ఒకవేళ( rc ) వెళ్ళు deserialize_error;
        కొనసాగు;
      }
      ఒకవేళ( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        కొనసాగు;
      }
      Tcl_Append_ఫలితం(interp, "unknown option: ", z, (అక్షరం*)0);
      rc = TCL_ERROR;
      వెళ్ళు deserialize_error;
    }
    p_విలువ = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_విలువ, &len);
    p_దత్తాంశం = sqlite3_malloc64( len );
    ఒకవేళ( p_దత్తాంశం==0 && len>0 ){
      Tcl_Append_ఫలితం(interp, "out of memory", (అక్షరం*)0);
      rc = TCL_ERROR;
    }లేకపోతే{
      పూర్ణాంకం flags;
      ఒకవేళ( len>0 ) జ్ఞాపకం_నకలు(p_దత్తాంశం, pBA, len);
      ఒకవేళ( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }లేకపోతే{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, p_దత్తాంశం, len, len, flags);
      ఒకవేళ( xrc ){
        Tcl_Append_ఫలితం(interp, "unable to set MEMDB content", (అక్షరం*)0);
        rc = TCL_ERROR;
      }
      ఒకవేళ( mx_కొలత>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_కొలత_హద్దు,&mx_కొలత);
      }
    }
deserialize_error:
#అయితే_ముగింపు
    ఆపు; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  స్థితి DB_ENABLE_ఎక్కించు_EXTENSION: {
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
    పూర్ణాంకం onoff;
    ఒకవేళ( objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "BOOLEAN");
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      తిరిగి TCL_ERROR;
    }
    sqlite3_enable_ఎక్కించు_extension(pDb->db, onoff);
    ఆపు;
#లేకపోతే
    Tcl_Append_ఫలితం(interp, "extension loading is turned off at compile-time",
                     (అక్షరం*)0);
    తిరిగి TCL_ERROR;
#అయితే_ముగింపు
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  స్థితి DB_ERRORCODE: {
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    ఆపు;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  స్థితి DB_ERROROFFSET: {
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(sqlite3_error_స్థానభ్రంశం(pDb->db)));
    ఆపు;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  స్థితి DB_EXISTS:
  స్థితి DB_ONECOLUMN: {
    Tcl_Obj *p_ఫలితం = 0;
    DbEvalContext sEval;
    ఒకవేళ( objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "SQL");
      తిరిగి TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = db_Eval_అడుగు(&sEval);
    ఒకవేళ( choice==DB_ONECOLUMN ){
      ఒకవేళ( rc==TCL_OK ){
        p_ఫలితం = db_Eval_స్తంభం_విలువ(&sEval, 0);
      }లేకపోతే ఒకవేళ( rc==TCL_BREAK ){
        Tcl_Reset_ఫలితం(interp);
      }
    }లేకపోతే ఒకవేళ( rc==TCL_BREAK || rc==TCL_OK ){
      p_ఫలితం = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    ఒకవేళ( p_ఫలితం ) Tcl_సమితి_Obj_ఫలితం(interp, p_ఫలితం);

    ఒకవేళ( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    ఆపు;
  }

  /*
  **    $db eval ?options? $sql ?varName? ?{  ...code... }?
  **
  ** The SQL statement in $sql is evaluated.  For each row, the values
  ** are placed in elements of the array or dict named $varName and
  ** ...code... is executed.  If $varName and $code are omitted, then
  ** no callback is ever invoked.  If $varName is an empty string,
  ** then the values are placed in variables that have the same name
  ** as the fields extracted by the query, and those variables are
  ** accessible during the eval of $code.
  */
  స్థితి DB_EVAL: {
    పూర్ణాంకం evalFlags = 0;
    స్థిరరాశి అక్షరం *zOpt;
    అయితే( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      ఒకవేళ( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }లేకపోతే ఒకవేళ( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }లేకపోతే{
        Tcl_Append_ఫలితం(interp, "unknown option: \"", zOpt, "\"", (శూన్య*)0);
        తిరిగి TCL_ERROR;
      }
      objc--;
      objv++;
    }
    ఒకవేళ( objc<3 || objc>5 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      తిరిగి TCL_ERROR;
    }

    ఒకవేళ( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_లెక్క(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      అయితే( TCL_OK==(rc = db_Eval_అడుగు(&sEval)) ){
        పూర్ణాంకం i;
        పూర్ణాంకం n_నిలువు;
        db_Eval_అడ్డువరుస_Info(&sEval, &n_నిలువు, 0);
        కొరకు(i=0; i<n_నిలువు; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_స్తంభం_విలువ(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      ఒకవేళ( rc==TCL_BREAK ){
        Tcl_సమితి_Obj_ఫలితం(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_లెక్క(pRet);
    }లేకపోతే{
      Client_దత్తాంశం cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      ఒకవేళ( objc>=5 && *(అక్షరం *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_లెక్క(pScript);

      p = (DbEvalContext *)Tcl_Alloc(పరిమాణం(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (శూన్య *)p;
      cd2[1] = (శూన్య *)pScript;
      rc = Db_Eval_తదుపరి_Cmd(cd2, interp, TCL_OK);
    }
    ఆపు;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  స్థితి DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    ఆపు;
  }

  /*
  **     $db function NAME [OPTIONS] SCRIPT
  **
  ** Create a new SQL function called NAME.  Whenever that function is
  ** called, invoke SCRIPT to evaluate the function.
  **
  ** Options:
  **         --argcount N           Function has exactly N arguments
  **         --deterministic        The function is pure
  **         --directonly           Prohibit use inside triggers and views
  **         --innocuous            Has no side effects or information leaks
  **         --returntype TYPE      Specify the return type of the function
  */
  స్థితి DB_FUNCTION: {
    పూర్ణాంకం flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    అక్షరం *zName;
    పూర్ణాంకం nArg = -1;
    పూర్ణాంకం i;
    పూర్ణాంకం eType = SQLITE_NULL;
    ఒకవేళ( objc<4 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      తిరిగి TCL_ERROR;
    }
    కొరకు(i=3; i<(objc-1); i++){
      స్థిరరాశి అక్షరం *z = Tcl_GetString(objv[i]);
      పూర్ణాంకం n = strlen30(z);
      ఒకవేళ( n>1 && అక్షరశ్రేణి_పోలిక(z, "-argcount",n)==0 ){
        ఒకవేళ( i==(objc-2) ){
          Tcl_Append_ఫలితం(interp, "option requires an argument: ", z,(అక్షరం*)0);
          తిరిగి TCL_ERROR;
        }
        ఒకవేళ( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) తిరిగి TCL_ERROR;
        ఒకవేళ( nArg<0 ){
          Tcl_Append_ఫలితం(interp, "number of arguments must be non-negative",
                           (అక్షరం*)0);
          తిరిగి TCL_ERROR;
        }
        i++;
      }లేకపోతే
      ఒకవేళ( n>1 && అక్షరశ్రేణి_పోలిక(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }లేకపోతే
      ఒకవేళ( n>1 && అక్షరశ్రేణి_పోలిక(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }లేకపోతే
      ఒకవేళ( n>1 && అక్షరశ్రేణి_పోలిక(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }లేకపోతే
      ఒకవేళ( n>1 && అక్షరశ్రేణి_పోలిక(z, "-returntype", n)==0 ){
        స్థిరరాశి అక్షరం *azType[] = {"integer", "real", "text", "blob", "any", 0};
        ప్రతిజ్ఞా( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_పాఠ్యం==3 );
        ప్రతిజ్ఞా( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        ఒకవేళ( i==(objc-2) ){
          Tcl_Append_ఫలితం(interp, "option requires an argument: ", z,(అక్షరం*)0);
          తిరిగి TCL_ERROR;
        }
        i++;
        ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, objv[i], azType, "type", 0, &eType) ){
          తిరిగి TCL_ERROR;
        }
        eType++;
      }లేకపోతే{
        Tcl_Append_ఫలితం(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (అక్షరం*)0
        );
        తిరిగి TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = కనుగొను_Sql_Func(pDb, zName);
    ఒకవేళ( pFunc==0 ) తిరిగి TCL_ERROR;
    ఒకవేళ( pFunc->pScript ){
      Tcl_Decr_Ref_లెక్క(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_లెక్క(pScript);
    pFunc->useEvalObjv = సురక్షిత_గమ్యం_Use_Eval_Objv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    ఒకవేళ( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    ఆపు;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  స్థితి DB_INCRBLOB: {
#నిర్వచితమైతే SQLITE_OMIT_INCRBLOB
    Tcl_Append_ఫలితం(interp, "incrblob not available in this build", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
#లేకపోతే
    పూర్ణాంకం isReadonly = 0;
    స్థిరరాశి అక్షరం *zDb = "main";
    స్థిరరాశి అక్షరం *z_పట్టిక;
    స్థిరరాశి అక్షరం *z_స్తంభం;
    Tcl_WideInt i_అడ్డువరుస;

    /* Check for the -readonly option */
    ఒకవేళ( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    ఒకవేళ( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      తిరిగి TCL_ERROR;
    }

    ఒకవేళ( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    z_పట్టిక = Tcl_GetString(objv[objc-3]);
    z_స్తంభం = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &i_అడ్డువరుస);

    ఒకవేళ( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, z_పట్టిక, z_స్తంభం, (sqlite3_int64)i_అడ్డువరుస, isReadonly
      );
    }
#అయితే_ముగింపు
    ఆపు;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  స్థితి DB_అంతరాయం: {
    sqlite3_అంతరాయం(pDb->db);
    ఆపు;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  స్థితి DB_NULLVALUE: {
    ఒకవేళ( objc!=2 && objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "NULLVALUE");
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( objc==3 ){
      Tcl_కొలత len;
      అక్షరం *zNull = Tcl_GetStringFromObj(objv[2], &len);
      ఒకవేళ( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      ఒకవేళ( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }లేకపోతే{
        pDb->zNull = 0;
      }
    }
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(pDb->zNull, -1));
    ఆపు;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  స్థితి DB_చివరి_చొప్పించు_ROWID: {
    Tcl_Obj *p_ఫలితం;
    Tcl_WideInt rowid;
    ఒకవేళ( objc!=2 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
      తిరిగి TCL_ERROR;
    }
    rowid = sqlite3_చివరి_చొప్పించు_rowid(pDb->db);
    p_ఫలితం = Tcl_Get_Obj_ఫలితం(interp);
    Tcl_సమితి_Wide_Int_Obj(p_ఫలితం, rowid);
    ఆపు;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  స్థితి DB_పురోగతి: {
    ఒకవేళ( objc==2 ){
      ఒకవేళ( pDb->z_పురోగతి ){
        Tcl_Append_ఫలితం(interp, pDb->z_పురోగతి, (అక్షరం*)0);
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
      sqlite3_పురోగతి_handler(pDb->db, 0, 0, 0);
#అయితే_ముగింపు
    }లేకపోతే ఒకవేళ( objc==4 ){
      అక్షరం *z_పురోగతి;
      Tcl_కొలత len;
      పూర్ణాంకం N;
      ఒకవేళ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        తిరిగి TCL_ERROR;
      };
      ఒకవేళ( pDb->z_పురోగతి ){
        Tcl_Free(pDb->z_పురోగతి);
      }
      z_పురోగతి = Tcl_GetStringFromObj(objv[3], &len);
      ఒకవేళ( z_పురోగతి && len>0 ){
        pDb->z_పురోగతి = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->z_పురోగతి, z_పురోగతి, len+1);
      }లేకపోతే{
        pDb->z_పురోగతి = 0;
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
      ఒకవేళ( pDb->z_పురోగతి ){
        pDb->interp = interp;
        sqlite3_పురోగతి_handler(pDb->db, N, Db_పురోగతి_Handler, pDb);
      }లేకపోతే{
        sqlite3_పురోగతి_handler(pDb->db, 0, 0, 0);
      }
#అయితే_ముగింపు
    }లేకపోతే{
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "N CALLBACK");
      తిరిగి TCL_ERROR;
    }
    ఆపు;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  స్థితి DB_PROFILE: {
    ఒకవేళ( objc>3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?CALLBACK?");
      తిరిగి TCL_ERROR;
    }లేకపోతే ఒకవేళ( objc==2 ){
      ఒకవేళ( pDb->zProfile ){
        Tcl_Append_ఫలితం(interp, pDb->zProfile, (అక్షరం*)0);
      }
    }లేకపోతే{
      అక్షరం *zProfile;
      Tcl_కొలత len;
      ఒకవేళ( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      ఒకవేళ( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->zProfile, zProfile, len+1);
      }లేకపోతే{
        pDb->zProfile = 0;
      }
#ఒకవేళ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_బిందువు) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ఒకవేళ( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }లేకపోతే{
        sqlite3_profile(pDb->db, 0, 0);
      }
#అయితే_ముగింపు
    }
    ఆపు;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  స్థితి DB_REKEY: {
    ఒకవేళ( objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "KEY");
      తిరిగి TCL_ERROR;
    }
    ఆపు;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  స్థితి DB_RESTORE: {
    స్థిరరాశి అక్షరం *zSrcFile;
    స్థిరరాశి అక్షరం *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    పూర్ణాంకం nTimeout = 0;

    ఒకవేళ( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }లేకపోతే ఒకవేళ( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }లేకపోతే{
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?DATABASE? FILENAME");
      తిరిగి TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    ఒకవేళ( rc!=SQLITE_OK ){
      Tcl_Append_ఫలితం(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (అక్షరం*)0);
      sqlite3_close(pSrc);
      తిరిగి TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    ఒకవేళ( pBackup==0 ){
      Tcl_Append_ఫలితం(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (అక్షరం*)0);
      sqlite3_close(pSrc);
      తిరిగి TCL_ERROR;
    }
    అయితే( (rc = sqlite3_backup_అడుగు(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      ఒకవేళ( rc==SQLITE_BUSY ){
        ఒకవేళ( nTimeout++ >= 3 ) ఆపు;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_ముగింపు(pBackup);
    ఒకవేళ( rc==SQLITE_పూర్తి ){
      rc = TCL_OK;
    }లేకపోతే ఒకవేళ( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_Append_ఫలితం(interp, "restore failed: source database busy",
                       (అక్షరం*)0);
      rc = TCL_ERROR;
    }లేకపోతే{
      Tcl_Append_ఫలితం(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (అక్షరం*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    ఆపు;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  స్థితి DB_SERIALIZE: {
#నిర్వచితమైతే SQLITE_OMIT_DESERIALIZE
    Tcl_Append_ఫలితం(interp, "MEMDB not available in this build",
                     (అక్షరం*)0);
    rc = TCL_ERROR;
#లేకపోతే
    స్థిరరాశి అక్షరం *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    గుర్తులేని అక్షరం *p_దత్తాంశం;
    ఒకవేళ( objc!=2 && objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }లేకపోతే{
      పూర్ణాంకం needFree;
      p_దత్తాంశం = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      ఒకవేళ( p_దత్తాంశం ){
        needFree = 0;
      }లేకపోతే{
        p_దత్తాంశం = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewByteArrayObj(p_దత్తాంశం,sz));
      ఒకవేళ( needFree ) sqlite3_free(p_దత్తాంశం);
    }
#అయితే_ముగింపు
    ఆపు;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  స్థితి DB_స్థితి: {
    పూర్ణాంకం v;
    స్థిరరాశి అక్షరం *zOp;
    ఒకవేళ( objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "(step|sort|autoindex)");
      తిరిగి TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    ఒకవేళ( strcmp(zOp, "step")==0 ){
      v = pDb->n_అడుగు;
    }లేకపోతే ఒకవేళ( strcmp(zOp, "sort")==0 ){
      v = pDb->n_క్రమీకరించు;
    }లేకపోతే ఒకవేళ( strcmp(zOp, "autoindex")==0 ){
      v = pDb->n_సూచీ;
    }లేకపోతే ఒకవేళ( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }లేకపోతే{
      Tcl_Append_ఫలితం(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(v));
    ఆపు;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  స్థితి DB_TIMEOUT: {
    పూర్ణాంకం ms;
    ఒకవేళ( objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "MILLISECONDS");
      తిరిగి TCL_ERROR;
    }
    ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &ms) ) తిరిగి TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    ఆపు;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  స్థితి DB_మొత్తం_CHANGES: {
    Tcl_Obj *p_ఫలితం;
    ఒకవేళ( objc!=2 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
      తిరిగి TCL_ERROR;
    }
    p_ఫలితం = Tcl_Get_Obj_ఫలితం(interp);
    Tcl_సమితి_Wide_Int_Obj(p_ఫలితం, sqlite3_మొత్తం_changes64(pDb->db));
    ఆపు;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  స్థితి DB_TRACE: {
    ఒకవేళ( objc>3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?CALLBACK?");
      తిరిగి TCL_ERROR;
    }లేకపోతే ఒకవేళ( objc==2 ){
      ఒకవేళ( pDb->zTrace ){
        Tcl_Append_ఫలితం(interp, pDb->zTrace, (అక్షరం*)0);
      }
    }లేకపోతే{
      అక్షరం *zTrace;
      Tcl_కొలత len;
      ఒకవేళ( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      ఒకవేళ( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->zTrace, zTrace, len+1);
      }లేకపోతే{
        pDb->zTrace = 0;
      }
#ఒకవేళ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_బిందువు) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ఒకవేళ( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }లేకపోతే{
        sqlite3_trace(pDb->db, 0, 0);
      }
#అయితే_ముగింపు
    }
    ఆపు;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  స్థితి DB_TRACE_V2: {
    ఒకవేళ( objc>4 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?CALLBACK? ?MASK?");
      తిరిగి TCL_ERROR;
    }లేకపోతే ఒకవేళ( objc==2 ){
      ఒకవేళ( pDb->zTraceV2 ){
        Tcl_Append_ఫలితం(interp, pDb->zTraceV2, (అక్షరం*)0);
      }
    }లేకపోతే{
      అక్షరం *zTraceV2;
      Tcl_కొలత len;
      Tcl_WideInt w_ముసుగు = 0;
      ఒకవేళ( objc==4 ){
        స్థిర స్థిరరాశి అక్షరం *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        గణన TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_అడ్డువరుస, TTYPE_CLOSE
        };
        Tcl_కొలత i;
        ఒకవేళ( TCL_OK!=Tcl_List_Obj_నిడివి(interp, objv[3], &len) ){
          తిరిగి TCL_ERROR;
        }
        కొరకు(i=0; i<len; i++){
          Tcl_Obj *pObj;
          పూర్ణాంకం ttype;
          ఒకవేళ( TCL_OK!=Tcl_List_Obj_సూచీ(interp, objv[3], i, &pObj) ){
            తిరిగి TCL_ERROR;
          }
          ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_Get_Obj_ఫలితం(interp));
            Tcl_Incr_Ref_లెక్క(pError);
            ఒకవేళ( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_లెక్క(pError);
              w_ముసుగు |= wType;
            }లేకపోతే{
              Tcl_సమితి_Obj_ఫలితం(interp, pError);
              Tcl_Decr_Ref_లెక్క(pError);
              తిరిగి TCL_ERROR;
            }
          }లేకపోతే{
            ఎంపిక( (గణన TTYPE_enum)ttype ){
              స్థితి TTYPE_STMT:    w_ముసుగు |= SQLITE_TRACE_STMT;    ఆపు;
              స్థితి TTYPE_PROFILE: w_ముసుగు |= SQLITE_TRACE_PROFILE; ఆపు;
              స్థితి TTYPE_అడ్డువరుస:     w_ముసుగు |= SQLITE_TRACE_అడ్డువరుస;     ఆపు;
              స్థితి TTYPE_CLOSE:   w_ముసుగు |= SQLITE_TRACE_CLOSE;   ఆపు;
            }
          }
        }
      }లేకపోతే{
        w_ముసుగు = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      ఒకవేళ( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      ఒకవేళ( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        జ్ఞాపకం_నకలు(pDb->zTraceV2, zTraceV2, len+1);
      }లేకపోతే{
        pDb->zTraceV2 = 0;
      }
#ఒకవేళ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_బిందువు)
      ఒకవేళ( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (గుర్తులేని)w_ముసుగు, DbTraceV2Handler, pDb);
      }లేకపోతే{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#అయితే_ముగింపు
    }
    ఆపు;
  }

  /*    $db transaction [-deferred|-immediate|-exclusive] SCRIPT
  **
  ** Start a new transaction (if we are not already in the midst of a
  ** transaction) and execute the TCL script SCRIPT.  After SCRIPT
  ** completes, either commit the transaction or roll it back if SCRIPT
  ** throws an exception.  Or if no new transaction was started, do nothing.
  ** pass the exception on up the stack.
  **
  ** This command was inspired by Dave Thomas's talk on Ruby at the
  ** 2005 O'Reilly Open Source Convention (OSCON).
  */
  స్థితి DB_TRANSACTION: {
    Tcl_Obj *pScript;
    స్థిరరాశి అక్షరం *zBegin = "SAVEPOINT _tcl_transaction";
    ఒకవేళ( objc!=3 && objc!=4 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "[TYPE] SCRIPT");
      తిరిగి TCL_ERROR;
    }

    ఒకవేళ( pDb->nTransaction==0 && objc==4 ){
      స్థిర స్థిరరాశి అక్షరం *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      గణన TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_తక్షణ
      };
      పూర్ణాంకం ttype;
      ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        తిరిగి TCL_ERROR;
      }
      ఎంపిక( (గణన TTYPE_enum)ttype ){
        స్థితి TTYPE_DEFERRED:    /* no-op */;                 ఆపు;
        స్థితి TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  ఆపు;
        స్థితి TTYPE_తక్షణ:   zBegin = "BEGIN IMMEDIATE";  ఆపు;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    ఒకవేళ( rc!=SQLITE_OK ){
      Tcl_Append_ఫలితం(interp, sqlite3_errmsg(pDb->db), (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    కూడు_Database_Ref(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    ఒకవేళ( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (శూన్య)Tcl_NREvalObj(interp, pScript, 0);
    }లేకపోతే{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    ఆపు;
  }

  /*
  **    $db unlock_notify ?script?
  */
  స్థితి DB_UNLOCK_NOTIFY: {
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_Append_ఫలితం(interp, "unlock_notify not available in this build",
                     (అక్షరం*)0);
    rc = TCL_ERROR;
#లేకపోతే
    ఒకవేళ( objc!=2 && objc!=3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }లేకపోతే{
      శూన్య (*xNotify)(శూన్య **, పూర్ణాంకం) = 0;
      శూన్య *pNotifyArg = 0;

      ఒకవేళ( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_లెక్క(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      ఒకవేళ( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (శూన్య *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_లెక్క(pDb->pUnlockNotify);
      }

      ఒకవేళ( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_Append_ఫలితం(interp, sqlite3_errmsg(pDb->db), (అక్షరం*)0);
        rc = TCL_ERROR;
      }
    }
#అయితే_ముగింపు
    ఆపు;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  స్థితి DB_PREUPDATE: {
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_Append_ఫలితం(interp, "preupdate_hook was omitted at compile-time", 
                     (అక్షరం*)0);
    rc = TCL_ERROR;
#లేకపోతే
    స్థిర స్థిరరాశి అక్షరం *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    గణన DbPreupdateSubCmd {
      PRE_లెక్క, PRE_లోతు, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    పూర్ణాంకం iSub;

    ఒకవేళ( objc<3 ){
      Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    ఒకవేళ( Tcl_Get_సూచీ_From_Obj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      తిరిగి TCL_ERROR;
    }

    ఎంపిక( (గణన DbPreupdateSubCmd)iSub ){
      స్థితి PRE_లెక్క: {
        పూర్ణాంకం n_నిలువు = sqlite3_preupdate_లెక్క(pDb->db);
        Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(n_నిలువు));
        ఆపు;
      }

      స్థితి PRE_HOOK: {
        ఒకవేళ( objc>4 ){
          Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "hook ?SCRIPT?");
          తిరిగి TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        ఆపు;
      }

      స్థితి PRE_లోతు: {
        Tcl_Obj *pRet;
        ఒకవేళ( objc!=3 ){
          Tcl_Wrong_అంకె_వాదనలు(interp, 3, objv, "");
          తిరిగి TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_లోతు(pDb->db));
        Tcl_సమితి_Obj_ఫలితం(interp, pRet);
        ఆపు;
      }

      స్థితి PRE_NEW:
      స్థితి PRE_OLD: {
        పూర్ణాంకం iIdx;
        sqlite3_విలువ *p_విలువ;
        ఒకవేళ( objc!=4 ){
          Tcl_Wrong_అంకె_వాదనలు(interp, 3, objv, "INDEX");
          తిరిగి TCL_ERROR;
        }
        ఒకవేళ( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          తిరిగి TCL_ERROR;
        }

        ఒకవేళ( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_విలువ);
        }లేకపోతే{
          ప్రతిజ్ఞా( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_విలువ);
        }

        ఒకవేళ( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((అక్షరం*)sqlite3_విలువ_పాఠ్యం(p_విలువ), -1);
          Tcl_సమితి_Obj_ఫలితం(interp, pObj);
        }లేకపోతే{
          Tcl_Append_ఫలితం(interp, sqlite3_errmsg(pDb->db), (అక్షరం*)0);
          తిరిగి TCL_ERROR;
        }
      }
    }
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */
    ఆపు;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  స్థితి DB_WAL_HOOK:
  స్థితి DB_UPDATE_HOOK:
  స్థితి DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    ఒకవేళ( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    ఒకవేళ( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    ఒకవేళ( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    ఒకవేళ( objc>3 ){
       Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?SCRIPT?");
       తిరిగి TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    ఆపు;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  స్థితి DB_సంస్కరణ: {
    పూర్ణాంకం i;
    కొరకు(i=2; i<objc; i++){
      స్థిరరాశి అక్షరం *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#నిర్వచితమైతే SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      ఒకవేళ( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          తిరిగి TCL_ERROR;
        }
      }లేకపోతే

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      ఒకవేళ( strcmp(zArg, "-last-stmt-ptr")==0 ){
        బాహ్య స్థిరరాశి అక్షరం *sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(శూన్య*);
        శూన్య *ptr = (శూన్య*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_సమితి_ఫలితం(interp, (అక్షరం*)sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(ptr), TCL_VOLATILE);
      }లేకపోతే
#అయితే_ముగింపు /* SQLITE_TEST */
      {
        Tcl_Append_ఫలితం(interp, "unknown argument: ", zArg, (అక్షరం*)0);
        తిరిగి TCL_ERROR;
      }
    }
    ఒకవేళ( i==2 ){   
      Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3_libversion(), TCL_STATIC);
    }
    ఆపు;
  }


  } /* End of the SWITCH statement */
  తిరిగి rc;
}

#ఒకవేళ SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI DbObjCmdAdaptor(
  శూన్య *cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *స్థిరరాశి*objv
){
  తిరిగి Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#అయితే_ముగింపు /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
స్థిర పూర్ణాంకం sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *స్థిరరాశి*objv
){
  Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  తిరిగి TCL_ERROR;
}

/*
**   sqlite3 DBNAME FILENAME ?-vfs VFSNAME? ?-key KEY? ?-readonly BOOLEAN?
**                           ?-create BOOLEAN? ?-nomutex BOOLEAN?
**                           ?-nofollow BOOLEAN?
**
** This is the main Tcl command.  When the "sqlite" Tcl command is
** invoked, this routine runs to process that command.
**
** The first argument, DBNAME, is an arbitrary name for a new
** database connection.  This command creates a new command named
** DBNAME that is used to control that connection.  The database
** connection is deleted when the DBNAME command is deleted.
**
** The second argument is the name of the database file.
**
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI DbMain(
  శూన్య *cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *స్థిరరాశి*objv
){
  SqliteDb *p;
  స్థిరరాశి అక్షరం *zArg;
  అక్షరం *z_Err_సందేశం;
  పూర్ణాంకం i;
  స్థిరరాశి అక్షరం *zFile = 0;
  స్థిరరాశి అక్షరం *zVfs = 0;
  పూర్ణాంకం flags;
  పూర్ణాంకం bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  పూర్ణాంకం rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#నిర్వచితమైతే SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#లేకపోతే
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#అయితే_ముగింపు

  ఒకవేళ( objc==1 ) తిరిగి sqliteCmdUsage(interp, objv);
  ఒకవేళ( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    ఒకవేళ( strcmp(zArg,"-version")==0 ){
      Tcl_Append_ఫలితం(interp,sqlite3_libversion(), (అక్షరం*)0);
      తిరిగి TCL_OK;
    }
    ఒకవేళ( strcmp(zArg,"-sourceid")==0 ){
      Tcl_Append_ఫలితం(interp,sqlite3_sourceid(), (అక్షరం*)0);
      తిరిగి TCL_OK;
    }
    ఒకవేళ( strcmp(zArg,"-has-codec")==0 ){
      Tcl_Append_ఫలితం(interp,"0",(అక్షరం*)0);
      తిరిగి TCL_OK;
    }
    ఒకవేళ( zArg[0]=='-' ) తిరిగి sqliteCmdUsage(interp, objv);
  }
  కొరకు(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    ఒకవేళ( zArg[0]!='-' ){
      ఒకవేళ( zFile!=0 ) తిరిగి sqliteCmdUsage(interp, objv);
      zFile = zArg;
      కొనసాగు;
    }
    ఒకవేళ( i==objc-1 ) తిరిగి sqliteCmdUsage(interp, objv);
    i++;
    ఒకవేళ( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }లేకపోతే ఒకవేళ( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }లేకపోతే ఒకవేళ( strcmp(zArg, "-readonly")==0 ){
      పూర్ణాంకం b;
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) తిరిగి TCL_ERROR;
      ఒకవేళ( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }లేకపోతే{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }లేకపోతే ఒకవేళ( strcmp(zArg, "-create")==0 ){
      పూర్ణాంకం b;
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) తిరిగి TCL_ERROR;
      ఒకవేళ( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }లేకపోతే{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }లేకపోతే ఒకవేళ( strcmp(zArg, "-nofollow")==0 ){
      పూర్ణాంకం b;
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) తిరిగి TCL_ERROR;
      ఒకవేళ( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }లేకపోతే{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }లేకపోతే ఒకవేళ( strcmp(zArg, "-nomutex")==0 ){
      పూర్ణాంకం b;
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) తిరిగి TCL_ERROR;
      ఒకవేళ( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }లేకపోతే{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }లేకపోతే ఒకవేళ( strcmp(zArg, "-fullmutex")==0 ){
      పూర్ణాంకం b;
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) తిరిగి TCL_ERROR;
      ఒకవేళ( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }లేకపోతే{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }లేకపోతే ఒకవేళ( strcmp(zArg, "-uri")==0 ){
      పూర్ణాంకం b;
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) తిరిగి TCL_ERROR;
      ఒకవేళ( b ){
        flags |= SQLITE_OPEN_URI;
      }లేకపోతే{
        flags &= ~SQLITE_OPEN_URI;
      }
    }లేకపోతే ఒకవేళ( strcmp(zArg, "-translatefilename")==0 ){
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        తిరిగి TCL_ERROR;
      }
    }లేకపోతే{
      Tcl_Append_ఫలితం(interp, "unknown option: ", zArg, (అక్షరం*)0);
      తిరిగి TCL_ERROR;
    }
  }
  z_Err_సందేశం = 0;
  p = (SqliteDb*)Tcl_Alloc( పరిమాణం(*p) );
  memset(p, 0, పరిమాణం(*p));
  ఒకవేళ( zFile==0 ) zFile = "";
  ఒకవేళ( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  ఒకవేళ( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  ఒకవేళ( p->db ){
    ఒకవేళ( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_సందేశం = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }లేకపోతే{
    z_Err_సందేశం = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  ఒకవేళ( p->db==0 ){
    Tcl_సమితి_ఫలితం(interp, z_Err_సందేశం, TCL_VOLATILE);
    Tcl_Free((అక్షరం*)p);
    sqlite3_free(z_Err_సందేశం);
    తిరిగి TCL_ERROR;
  }
  p->maxStmt = అంకె_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  ఒకవేళ( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (అక్షరం*)p, DbDeleteCmd);
  }లేకపోతే{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (అక్షరం*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  తిరిగి TCL_OK;
}

/*
** Provide a dummy Tcl_InitStubs if we are using this as a static
** library.
*/
#నిర్వచితం_కాకపోతే USE_TCL_STUBS
# undef  Tcl_InitStubs
# define Tcl_InitStubs(a,b,c) TCL_సంస్కరణ
#అయితే_ముగింపు

/*
** Make sure we have a PACKAGE_VERSION macro defined.  This will be
** defined automatically by the TEA makefile.  But other makefiles
** do not define it.
*/
#నిర్వచితం_కాకపోతే PACKAGE_సంస్కరణ
# define PACKAGE_సంస్కరణ SQLITE_సంస్కరణ
#అయితే_ముగింపు

/*
** Initialize this module.
**
** This Tcl module contains only a single new Tcl command named "sqlite".
** (Hence there is no namespace.  There is no point in using a namespace
** if the extension only supplies one new name!)  The "sqlite" command is
** used to open a new SQLite database.  See the DbMain() routine above
** for additional information.
**
** The EXTERN macros are required by TCL in order to work on windows.
*/
EXTERN పూర్ణాంకం Sqlite3_Init(Tcl_Interp *interp){
  పూర్ణాంకం rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  ఒకవేళ( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#నిర్వచితం_కాకపోతే SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#అయితే_ముగింపు
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_సంస్కరణ);
  }
  తిరిగి rc;
}
EXTERN పూర్ణాంకం Tclsqlite3_Init(Tcl_Interp *interp){ తిరిగి Sqlite3_Init(interp); }
EXTERN పూర్ణాంకం Sqlite3_Unload(Tcl_Interp *interp, పూర్ణాంకం flags){ తిరిగి TCL_OK; }
EXTERN పూర్ణాంకం Tclsqlite3_Unload(Tcl_Interp *interp, పూర్ణాంకం flags){ తిరిగి TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN పూర్ణాంకం Sqlite3_సురక్షిత_Init(Tcl_Interp *interp){ తిరిగి TCL_ERROR; }
EXTERN పూర్ణాంకం Sqlite3_సురక్షిత_Unload(Tcl_Interp *interp, పూర్ణాంకం flags){తిరిగి TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN పూర్ణాంకం Sqlite_Init(Tcl_Interp *interp){ తిరిగి Sqlite3_Init(interp);}
EXTERN పూర్ణాంకం Tclsqlite_Init(Tcl_Interp *interp){ తిరిగి Sqlite3_Init(interp); }
EXTERN పూర్ణాంకం Sqlite_Unload(Tcl_Interp *interp, పూర్ణాంకం flags){ తిరిగి TCL_OK; }
EXTERN పూర్ణాంకం Tclsqlite_Unload(Tcl_Interp *interp, పూర్ణాంకం flags){ తిరిగి TCL_OK; }
EXTERN పూర్ణాంకం Sqlite_సురక్షిత_Init(Tcl_Interp *interp){ తిరిగి TCL_ERROR; }
EXTERN పూర్ణాంకం Sqlite_సురక్షిత_Unload(Tcl_Interp *interp, పూర్ణాంకం flags){తిరిగి TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN పూర్ణాంకం sqlite3_Init(Tcl_Interp *interp){ తిరిగి Sqlite3_Init(interp);}
EXTERN పూర్ణాంకం sqlite_Init(Tcl_Interp *interp){ తిరిగి Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#ఒకవేళ defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
స్థిర స్థిరరాశి అక్షరం *tclsh_main_loop(శూన్య){
  స్థిర స్థిరరాశి అక్షరం zMainloop[] =
    "if {[llength $argv]>=1} {\n"
#నిర్వచితమైతే WIN32
      "set new [list]\n"
      "foreach arg $argv {\n"
        "if {[string match -* $arg] || [file exists $arg]} {\n"
          "lappend new $arg\n"
        "} else {\n"
          "set once 0\n"
          "foreach match [lsort [glob -nocomplain $arg]] {\n"
            "lappend new $match\n"
            "set once 1\n"
          "}\n"
          "if {!$once} {lappend new $arg}\n"
        "}\n"
      "}\n"
      "set argv $new\n"
      "unset new\n"
#అయితే_ముగింపు
      "set argv0 [lindex $argv 0]\n"
      "set argv [lrange $argv 1 end]\n"
      "source $argv0\n"
    "} else {\n"
      "set line {}\n"
      "while {![eof stdin]} {\n"
        "if {$line!=\"\"} {\n"
          "puts -nonewline \"> \"\n"
        "} else {\n"
          "puts -nonewline \"% \"\n"
        "}\n"
        "flush stdout\n"
        "append line [gets stdin]\n"
        "if {[info complete $line]} {\n"
          "if {[catch {uplevel #0 $line} result]} {\n"
            "puts stderr \"Error: $result\"\n"
          "} elseif {$result!=\"\"} {\n"
            "puts $result\n"
          "}\n"
          "set line {}\n"
        "} else {\n"
          "append line \\n\n"
        "}\n"
      "}\n"
    "}\n"
  ;
  తిరిగి zMainloop;
}

#నిర్వచితం_కాకపోతే TCLSH_MAIN
# define TCLSH_MAIN ప్రధాన
#అయితే_ముగింపు
పూర్ణాంకం SQLITE_CDECL TCLSH_MAIN(పూర్ణాంకం argc, అక్షరం **argv){
  Tcl_Interp *interp;
  పూర్ణాంకం i;
  స్థిరరాశి అక్షరం *zScript = 0;
  అక్షరం zArgc[32];
#ఒకవేళ defined(TCLSH_INIT_PROC)
  బాహ్య స్థిరరాశి అక్షరం *TCLSH_INIT_PROC(Tcl_Interp*);
#అయితే_ముగింపు

#ఒకవేళ !defined(_WIN32_WCE)
  ఒకవేళ( getenv("SQLITE_DEBUG_BREAK") ){
    ఒకవేళ( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }లేకపోతే{
#ఒకవేళ defined(_WIN32) || defined(WIN32)
      DebugBreak();
#లేదావేళ defined(SIGTRAP)
      లేపు(SIGTRAP);
#అయితే_ముగింపు
    }
  }
#అయితే_ముగింపు

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_కనుగొను_Executable(argv[0]);
  Tcl_సమితి_System_Encoding(ఖాళీ, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(పరిమాణం(zArgc), zArgc, "%d", argc-1);
  Tcl_సమితి_Var(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_సమితి_Var(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_సమితి_Var(interp,"argv", "", TCL_GLOBAL_ONLY);
  కొరకు(i=1; i<argc; i++){
    Tcl_సమితి_Var(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_విలువ);
  }
#ఒకవేళ defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#అయితే_ముగింపు
  ఒకవేళ( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  ఒకవేళ( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    స్థిరరాశి అక్షరం *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    ఒకవేళ( zInfo==0 ) zInfo = Tcl_Get_String_ఫలితం(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    తిరిగి 1;
  }
  తిరిగి 0;
}
#అయితే_ముగింపు /* TCLSH */
