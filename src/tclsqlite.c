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
#ifdef TCLSH_INIT_PROC
# define TCLSH 1
#endif

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#যদি defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#যদি defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#অন্যথা
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#যদি TCL_MAJOR_VERSION==9
# define CONST ধ্রুবক
#নতুবা_যদি !defined(Tcl_আকার)
  প্রকারলেখ পূর্ণসংখ্যা Tcl_আকার;
# ifndef Tcl_Bounce_Ref_গুনতি
#  define Tcl_Bounce_Ref_গুনতি(X) Tcl_Incr_Ref_গুনতি(X); Tcl_Decr_Ref_গুনতি(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#endif
/**** End copy of tclsqlite.h ****/

#অন্তর্ভাব <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ifndef SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  প্রকারলেখ unsigned অক্ষর u8;
# ifndef SQLITE_PTRSIZE
#   যদি defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   অন্যথা
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# যদি defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    প্রকারলেখ uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    প্রকারলেখ unsigned পূর্ণসংখ্যা uptr;
# অন্যথা
    প্রকারলেখ sqlite3_uint64 uptr;
# endif
#endif
#অন্তর্ভাব <ctype.h>

/* Used to get the current process ID */
#যদি !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#নতুবা_যদি !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (পূর্ণসংখ্যা)GetCurrentProcessId
#endif

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ifdef BUILD_sqlite
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_sqlite */

#define অঙ্ক_PREPARED_STMTS 10
#define MAX_PREPARED_STMTS 100

/* Forward declaration */
প্রকারলেখ কাঠামো SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#যদি defined(SQLITE_ENABLE_QRF_IN_TCL)
#অন্তর্ভাব "qrf.h"
#endif

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
প্রকারলেখ কাঠামো SqlFunc SqlFunc;
কাঠামো SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  পূর্ণসংখ্যা useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  পূর্ণসংখ্যা eType;            /* Type of value to return */
  অক্ষর *zName;          /* Name of this function */
  SqlFunc *p_পরবর্তী;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
প্রকারলেখ কাঠামো SqlCollate SqlCollate;
কাঠামো SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  অক্ষর *zScript;        /* The script to be run */
  SqlCollate *p_পরবর্তী;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
প্রকারলেখ কাঠামো SqlPreparedStmt SqlPreparedStmt;
কাঠামো SqlPreparedStmt {
  SqlPreparedStmt *p_পরবর্তী;  /* Next in linked list */
  SqlPreparedStmt *p_পূর্ব;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  পূর্ণসংখ্যা nSql;                /* chars in zSql[] */
  ধ্রুবক অক্ষর *zSql;        /* Text of the SQL statement */
  পূর্ণসংখ্যা nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

প্রকারলেখ কাঠামো IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
কাঠামো SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  অক্ষর *zBusy;               /* The busy callback routine */
  অক্ষর *zCommit;             /* The commit hook callback routine */
  অক্ষর *zTrace;              /* The trace callback routine */
  অক্ষর *zTraceV2;            /* The trace_v2 callback routine */
  অক্ষর *zProfile;            /* The profile callback routine */
  অক্ষর *zProgress;           /* The progress callback routine */
  অক্ষর *zBindFallback;       /* Callback to invoke on a binding miss */
  অক্ষর *zAuth;               /* The authorization callback routine */
  পূর্ণসংখ্যা disableAuth;           /* Disable the authorizer if it exists */
  অক্ষর *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  পূর্ণসংখ্যা rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  পূর্ণসংখ্যা maxStmt;               /* The next maximum number of stmtList */
  পূর্ণসংখ্যা nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  পূর্ণসংখ্যা nStep, nSort, n_সূচক;  /* Statistics for most recent operation */
  পূর্ণসংখ্যা nVMStep;               /* Another statistic for most recent operation */
  পূর্ণসংখ্যা nTransaction;          /* Number of nested [transaction] methods */
  পূর্ণসংখ্যা openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  পূর্ণসংখ্যা nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  পূর্ণসংখ্যা bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

কাঠামো IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned পূর্ণসংখ্যা isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *p_পরবর্তী;   /* Linked list of all open incrblob channels */
  IncrblobChannel *p_পূর্ব;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
স্থির পূর্ণসংখ্যা strlen30(ধ্রুবক অক্ষর *z){
  ধ্রুবক অক্ষর *z2 = z;
  যখন( *z2 ){ z2++; }
  ফেরত 0x3fffffff & (পূর্ণসংখ্যা)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
স্থির শূন্য closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *p_পরবর্তী;

  জন্য(p=pDb->pIncrblob; p; p=p_পরবর্তী){
    p_পরবর্তী = p->p_পরবর্তী;

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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI incrblobClose2(
  Client_তথ্য instance_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা flags
){
  IncrblobChannel *p = (IncrblobChannel *)instance_তথ্য;
  পূর্ণসংখ্যা  rc;
  sqlite3 *db = p->pDb->db;

  যদি( flags ){
    p->isClosed |= flags;
    ফেরত TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  যদি( p->p_পরবর্তী ){
    p->p_পরবর্তী->p_পূর্ব = p->p_পূর্ব;
  }
  যদি( p->p_পূর্ব ){
    p->p_পূর্ব->p_পরবর্তী = p->p_পরবর্তী;
  }
  যদি( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->p_পরবর্তী;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((অক্ষর *)p);

  যদি( rc!=SQLITE_OK ){
    Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3_errmsg(db), TCL_VOLATILE);
    ফেরত TCL_ERROR;
  }
  ফেরত TCL_OK;
}
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI incrblobClose(
  Client_তথ্য instance_তথ্য,
  Tcl_Interp *interp
){
  ফেরত incrblobClose2(instance_তথ্য, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI incrblob_প্রবেশ(
  Client_তথ্য instance_তথ্য,
  অক্ষর *buf,
  পূর্ণসংখ্যা buf_আকার,
  পূর্ণসংখ্যা *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_তথ্য;
  sqlite3_int64 nRead = buf_আকার;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  পূর্ণসংখ্যা rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  যদি( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  যদি( nRead<=0 ){
    ফেরত 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (শূন্য *)buf, (পূর্ণসংখ্যা)nRead, (পূর্ণসংখ্যা)p->iSeek);
  যদি( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    ফেরত -1;
  }

  p->iSeek += nRead;
  ফেরত nRead;
}

/*
** Write data to an incremental blob channel.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI incrblobOutput(
  Client_তথ্য instance_তথ্য,
  ধ্রুবক অক্ষর *buf,
  পূর্ণসংখ্যা toWrite,
  পূর্ণসংখ্যা *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_তথ্য;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  পূর্ণসংখ্যা rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  যদি( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    ফেরত -1;
  }
  যদি( nWrite<=0 ){
    ফেরত 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (শূন্য*)buf,(পূর্ণসংখ্যা)nWrite, (পূর্ণসংখ্যা)p->iSeek);
  যদি( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    ফেরত -1;
  }

  p->iSeek += nWrite;
  ফেরত nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#যদি TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#অন্যথা
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
স্থির WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  Client_তথ্য instance_তথ্য,
  WideSeekProcType offset,
  পূর্ণসংখ্যা seekMode,
  পূর্ণসংখ্যা *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_তথ্য;

  নির্বাচন( seekMode ){
    ক্ষেত্র SEEK_SET:
      p->iSeek = offset;
      বিরতি;
    ক্ষেত্র SEEK_CUR:
      p->iSeek += offset;
      বিরতি;
    ক্ষেত্র SEEK_END:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      বিরতি;

    পূর্বনির্ধারিত: প্রতিজ্ঞা(!"Bad seekMode");
  }

  ফেরত p->iSeek;
}
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI incrblobSeek(
  Client_তথ্য instance_তথ্য,
  long offset,
  পূর্ণসংখ্যা seekMode,
  পূর্ণসংখ্যা *errorCodePtr
){
  ফেরত incrblobWideSeek(instance_তথ্য,offset,seekMode,errorCodePtr);
}


স্থির শূন্য SQLITE_TCLAPI incrblobWatch(
  Client_তথ্য instance_তথ্য,
  পূর্ণসংখ্যা mode
){
  /* NO-OP */
}
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI incrblobHandle(
  Client_তথ্য instance_তথ্য,
  পূর্ণসংখ্যা dir,
  Client_তথ্য *hPtr
){
  ফেরত TCL_ERROR;
}

স্থির Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_VERSION_5,             /* version                              */
  incrblobClose,                     /* closeProc                            */
  incrblob_প্রবেশ,                     /* inputProc                            */
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
স্থির পূর্ণসংখ্যা createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  ধ্রুবক অক্ষর *zDb,
  ধ্রুবক অক্ষর *zTable,
  ধ্রুবক অক্ষর *zColumn,
  sqlite_int64 iRow,
  পূর্ণসংখ্যা isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  স্থির পূর্ণসংখ্যা গুনতি = 0;
  অক্ষর zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  যদি( rc!=SQLITE_OK ){
    Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    ফেরত TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(আকার(IncrblobChannel));
  memset(p, 0, আকার(*p));
  p->pBlob = pBlob;
  যদি( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(আকার(zChannel), zChannel, "incrblob_%d", ++গুনতি);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->p_পরবর্তী = pDb->pIncrblob;
  p->p_পূর্ব = 0;
  যদি( p->p_পরবর্তী ){
    p->p_পরবর্তী->p_পূর্ব = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_Set_ফলাফল(interp, (অক্ষর *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  ফেরত TCL_OK;
}
#অন্যথা  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
  #define closeIncrblobChannels(pDb)
#endif

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
স্থির পূর্ণসংখ্যা safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  ধ্রুবক অক্ষর *z;
  Tcl_আকার n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  যখন( n-- > 0 ){
    পূর্ণসংখ্যা c = *(z++);
    যদি( c=='$' || c=='[' || c==';' ) ফেরত 0;
  }
  ফেরত 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
স্থির SqlFunc *findSqlFunc(SqliteDb *pDb, ধ্রুবক অক্ষর *zName){
  SqlFunc *p, *pNew;
  পূর্ণসংখ্যা nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( আকার(*pNew) + nName + 1 );
  pNew->zName = (অক্ষর*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  জন্য(p=pDb->pFunc; p; p=p->p_পরবর্তী){
    যদি( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((অক্ষর*)pNew);
      ফেরত p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->p_পরবর্তী = pDb->pFunc;
  pDb->pFunc = pNew;
  ফেরত pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
স্থির শূন্য dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  যদি( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((অক্ষর *)pStmt->zSql);
  }
#endif
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((অক্ষর *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
স্থির শূন্য flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *p_পরবর্তী;

  জন্য(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=p_পরবর্তী){
    p_পরবর্তী = pPreStmt->p_পরবর্তী;
    dbFreeStmt(pPreStmt);
  }
  pDb->nStmt = 0;
  pDb->stmtLast = 0;
  pDb->stmtList = 0;
}

/*
** Increment the reference counter on the SqliteDb object. The reference
** should be released by calling delDatabaseRef().
*/
স্থির শূন্য addDatabaseRef(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
স্থির শূন্য delDatabaseRef(SqliteDb *pDb){
  প্রতিজ্ঞা( pDb->nRef>0 );
  pDb->nRef--;
  যদি( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    যখন( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->p_পরবর্তী;
      প্রতিজ্ঞা( pFunc->pDb==pDb );
      Tcl_Decr_Ref_গুনতি(pFunc->pScript);
      Tcl_Free((অক্ষর*)pFunc);
    }
    যখন( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->p_পরবর্তী;
      Tcl_Free((অক্ষর*)pCollate);
    }
    যদি( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    যদি( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    যদি( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    যদি( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    যদি( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    যদি( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    যদি( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    যদি( pDb->pUpdateHook ){
      Tcl_Decr_Ref_গুনতি(pDb->pUpdateHook);
    }
    যদি( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_গুনতি(pDb->pPreUpdateHook);
    }
    যদি( pDb->pRollbackHook ){
      Tcl_Decr_Ref_গুনতি(pDb->pRollbackHook);
    }
    যদি( pDb->pWalHook ){
      Tcl_Decr_Ref_গুনতি(pDb->pWalHook);
    }
    যদি( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_গুনতি(pDb->pCollateNeeded);
    }
    Tcl_Free((অক্ষর*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
স্থির শূন্য SQLITE_TCLAPI DbDeleteCmd(শূন্য *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
স্থির পূর্ণসংখ্যা DbBusyHandler(শূন্য *cd, পূর্ণসংখ্যা nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  পূর্ণসংখ্যা rc;
  অক্ষর zVal[30];

  sqlite3_snprintf(আকার(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (অক্ষর*)0);
  যদি( rc!=TCL_OK || atoi(Tcl_Get_String_ফলাফল(pDb->interp)) ){
    ফেরত 0;
  }
  ফেরত 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
স্থির পূর্ণসংখ্যা DbProgressHandler(শূন্য *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  পূর্ণসংখ্যা rc;

  প্রতিজ্ঞা( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  যদি( rc!=TCL_OK || atoi(Tcl_Get_String_ফলাফল(pDb->interp)) ){
    ফেরত 1;
  }
  ফেরত 0;
}
#endif

#যদি !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
স্থির শূন্য DbTraceHandler(শূন্য *cd, ধ্রুবক অক্ষর *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_মান(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_ফলাফল(pDb->interp);
}
#endif

#ifndef SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
স্থির পূর্ণসংখ্যা DbTraceV2Handler(
  unsigned type, /* One of the SQLITE_TRACE_* event types. */
  শূন্য *cd,      /* The original context data pointer. */
  শূন্য *pd,      /* Primary event data, depends on event type. */
  শূন্য *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  নির্বাচন( type ){
    ক্ষেত্র SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      অক্ষর *zSql = (অক্ষর *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_গুনতি(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_গুনতি(pCmd);
      Tcl_Reset_ফলাফল(pDb->interp);
      বিরতি;
    }
    ক্ষেত্র SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_গুনতি(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_গুনতি(pCmd);
      Tcl_Reset_ফলাফল(pDb->interp);
      বিরতি;
    }
    ক্ষেত্র SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_গুনতি(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_গুনতি(pCmd);
      Tcl_Reset_ফলাফল(pDb->interp);
      বিরতি;
    }
    ক্ষেত্র SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_গুনতি(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_গুনতি(pCmd);
      Tcl_Reset_ফলাফল(pDb->interp);
      বিরতি;
    }
  }
  ফেরত SQLITE_OK;
}
#endif

#যদি !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
স্থির শূন্য DbProfileHandler(শূন্য *cd, ধ্রুবক অক্ষর *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  অক্ষর zTm[100];

  sqlite3_snprintf(আকার(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_মান(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_ফলাফল(pDb->interp);
}
#endif

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
স্থির পূর্ণসংখ্যা DbCommitHandler(শূন্য *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  পূর্ণসংখ্যা rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  যদি( rc!=TCL_OK || atoi(Tcl_Get_String_ফলাফল(pDb->interp)) ){
    ফেরত 1;
  }
  ফেরত 0;
}

স্থির শূন্য DbRollbackHandler(শূন্য *client_তথ্য){
  SqliteDb *pDb = (SqliteDb*)client_তথ্য;
  প্রতিজ্ঞা(pDb->pRollbackHook);
  যদি( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
স্থির পূর্ণসংখ্যা DbWalHandler(
  শূন্য *client_তথ্য,
  sqlite3 *db,
  ধ্রুবক অক্ষর *zDb,
  পূর্ণসংখ্যা nEntry
){
  পূর্ণসংখ্যা ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)client_তথ্য;
  Tcl_Interp *interp = pDb->interp;
  প্রতিজ্ঞা(pDb->pWalHook);

  প্রতিজ্ঞা( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_গুনতি(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  যদি( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_Get_Obj_ফলাফল(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_গুনতি(p);

  ফেরত ret;
}

#যদি defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
স্থির শূন্য setTestUnlockNotifyVars(Tcl_Interp *interp, পূর্ণসংখ্যা iArg, পূর্ণসংখ্যা nArg){
  অক্ষর zBuf[64];
  sqlite3_snprintf(আকার(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(আকার(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#অন্যথা
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
স্থির শূন্য DbUnlockNotify(শূন্য **apArg, পূর্ণসংখ্যা nArg){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<nArg; i++){
    ধ্রুবক পূর্ণসংখ্যা flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    setTestUnlockNotifyVars(pDb->interp, i, nArg);
    প্রতিজ্ঞা( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_গুনতি(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
স্থির শূন্য DbPreUpdateHandler(
  শূন্য *p,
  sqlite3 *db,
  পূর্ণসংখ্যা op,
  ধ্রুবক অক্ষর *zDb,
  ধ্রুবক অক্ষর *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  স্থির ধ্রুবক অক্ষর *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  প্রতিজ্ঞা( (SQLITE_DELETE-1)/9 == 0 );
  প্রতিজ্ঞা( (SQLITE_ঢোকাও-1)/9 == 1 );
  প্রতিজ্ঞা( (SQLITE_UPDATE-1)/9 == 2 );
  প্রতিজ্ঞা( pDb->pPreUpdateHook );
  প্রতিজ্ঞা( db==pDb->db );
  প্রতিজ্ঞা( op==SQLITE_ঢোকাও || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_গুনতি(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_গুনতি(pCmd);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

স্থির শূন্য DbUpdateHandler(
  শূন্য *p,
  পূর্ণসংখ্যা op,
  ধ্রুবক অক্ষর *zDb,
  ধ্রুবক অক্ষর *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  স্থির ধ্রুবক অক্ষর *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  প্রতিজ্ঞা( (SQLITE_DELETE-1)/9 == 0 );
  প্রতিজ্ঞা( (SQLITE_ঢোকাও-1)/9 == 1 );
  প্রতিজ্ঞা( (SQLITE_UPDATE-1)/9 == 2 );

  প্রতিজ্ঞা( pDb->pUpdateHook );
  প্রতিজ্ঞা( op==SQLITE_ঢোকাও || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_গুনতি(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_গুনতি(pCmd);
}

স্থির শূন্য tclCollateNeeded(
  শূন্য *pCtx,
  sqlite3 *db,
  পূর্ণসংখ্যা enc,
  ধ্রুবক অক্ষর *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_গুনতি(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_গুনতি(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
স্থির পূর্ণসংখ্যা tclSqlCollate(
  শূন্য *pCtx,
  পূর্ণসংখ্যা nA,
  ধ্রুবক শূন্য *zA,
  পূর্ণসংখ্যা nB,
  ধ্রুবক শূন্য *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_গুনতি(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_গুনতি(pCmd);
  ফেরত (atoi(Tcl_Get_String_ফলাফল(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
স্থির শূন্য tclSqlFunc(sqlite3_context *context, পূর্ণসংখ্যা argc, sqlite3_মান**argv){
  SqlFunc *p = sqlite3_user_তথ্য(context);
  Tcl_Obj *pCmd;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc;

  যদি( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_গুনতি(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_গুনতি(pCmd);
  }অন্যথা{
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
    Tcl_আকার nArg;
    যদি( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_ফলাফল_error(context, Tcl_Get_String_ফলাফল(p->interp), -1);
      ফেরত;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_গুনতি(pCmd);
    জন্য(i=0; i<argc; i++){
      sqlite3_মান *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      নির্বাচন( sqlite3_মান_type(pIn) ){
        ক্ষেত্র SQLITE_BLOB: {
          পূর্ণসংখ্যা bytes = sqlite3_মান_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_মান_blob(pIn), bytes);
          বিরতি;
        }
        ক্ষেত্র SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_মান_int64(pIn);
          যদি( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((পূর্ণসংখ্যা)v);
          }অন্যথা{
            pVal = Tcl_NewWideIntObj(v);
          }
          বিরতি;
        }
        ক্ষেত্র SQLITE_FLOAT: {
          দ্বিগুণ r = sqlite3_মান_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          বিরতি;
        }
        ক্ষেত্র SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          বিরতি;
        }
        পূর্বনির্ধারিত: {
          পূর্ণসংখ্যা bytes = sqlite3_মান_bytes(pIn);
          pVal = Tcl_NewStringObj((অক্ষর *)sqlite3_মান_text(pIn), bytes);
          বিরতি;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      যদি( rc ){
        Tcl_Decr_Ref_গুনতি(pCmd);
        sqlite3_ফলাফল_error(context, Tcl_Get_String_ফলাফল(p->interp), -1);
        ফেরত;
      }
    }
    যদি( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_Decr_Ref_গুনতি(pCmd);
  }

  যদি( TCL_BREAK==rc ){
    sqlite3_ফলাফল_null(context);
  }অন্যথা যদি( rc && rc!=TCL_RETURN ){
    sqlite3_ফলাফল_error(context, Tcl_Get_String_ফলাফল(p->interp), -1);
  }অন্যথা{
    Tcl_Obj *pVar = Tcl_Get_Obj_ফলাফল(p->interp);
    Tcl_আকার n;
    u8 *তথ্য;
    ধ্রুবক অক্ষর *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    অক্ষর c = zType[0];
    পূর্ণসংখ্যা eType = p->eType;

    যদি( eType==SQLITE_NULL ){
      যদি( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }অন্যথা যদি( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }অন্যথা যদি( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }অন্যথা{
        eType = SQLITE_TEXT;
      }
    }

    নির্বাচন( eType ){
      ক্ষেত্র SQLITE_BLOB: {
        তথ্য = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_ফলাফল_blob64(context, তথ্য, n, SQLITE_TRANSIENT);
        বিরতি;
      }
      ক্ষেত্র SQLITE_INTEGER: {
        Tcl_WideInt v;
        যদি( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_ফলাফল_int64(context, v);
          বিরতি;
        }
        /* fall-through */
      }
      ক্ষেত্র SQLITE_FLOAT: {
        দ্বিগুণ r;
        যদি( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_ফলাফল_double(context, r);
          বিরতি;
        }
        /* fall-through */
      }
      পূর্বনির্ধারিত: {
        তথ্য = (unsigned অক্ষর *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_ফলাফল_text64(context, (অক্ষর *)তথ্য, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        বিরতি;
      }
    }

  }
}

#ifndef SQLITE_OMIT_AUTHORIZATION
/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
স্থির পূর্ণসংখ্যা auth_callback(
  শূন্য *pArg,
  পূর্ণসংখ্যা code,
  ধ্রুবক অক্ষর *zArg1,
  ধ্রুবক অক্ষর *zArg2,
  ধ্রুবক অক্ষর *zArg3,
  ধ্রুবক অক্ষর *zArg4
){
  ধ্রুবক অক্ষর *zCode;
  Tcl_DString str;
  পূর্ণসংখ্যা rc;
  ধ্রুবক অক্ষর *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  যদি( pDb->disableAuth ) ফেরত SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  নির্বাচন( code ){
    ক্ষেত্র SQLITE_COPY              : zCode="SQLITE_COPY"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_সূচক      : zCode="SQLITE_CREATE_INDEX"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_অস্থায়ী_সূচক : zCode="SQLITE_CREATE_TEMP_INDEX"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_অস্থায়ী_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_অস্থায়ী_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_অস্থায়ী_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; বিরতি;
    ক্ষেত্র SQLITE_DELETE            : zCode="SQLITE_DELETE"; বিরতি;
    ক্ষেত্র SQLITE_DROP_সূচক        : zCode="SQLITE_DROP_INDEX"; বিরতি;
    ক্ষেত্র SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; বিরতি;
    ক্ষেত্র SQLITE_DROP_অস্থায়ী_সূচক   : zCode="SQLITE_DROP_TEMP_INDEX"; বিরতি;
    ক্ষেত্র SQLITE_DROP_অস্থায়ী_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; বিরতি;
    ক্ষেত্র SQLITE_DROP_অস্থায়ী_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; বিরতি;
    ক্ষেত্র SQLITE_DROP_অস্থায়ী_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; বিরতি;
    ক্ষেত্র SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; বিরতি;
    ক্ষেত্র SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; বিরতি;
    ক্ষেত্র SQLITE_ঢোকাও            : zCode="SQLITE_INSERT"; বিরতি;
    ক্ষেত্র SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; বিরতি;
    ক্ষেত্র SQLITE_READ              : zCode="SQLITE_READ"; বিরতি;
    ক্ষেত্র SQLITE_SELECT            : zCode="SQLITE_SELECT"; বিরতি;
    ক্ষেত্র SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; বিরতি;
    ক্ষেত্র SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; বিরতি;
    ক্ষেত্র SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; বিরতি;
    ক্ষেত্র SQLITE_DETACH            : zCode="SQLITE_DETACH"; বিরতি;
    ক্ষেত্র SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; বিরতি;
    ক্ষেত্র SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; বিরতি;
    ক্ষেত্র SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; বিরতি;
    ক্ষেত্র SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; বিরতি;
    ক্ষেত্র SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; বিরতি;
    ক্ষেত্র SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; বিরতি;
    ক্ষেত্র SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; বিরতি;
    ক্ষেত্র SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; বিরতি;
    পূর্বনির্ধারিত                       : zCode="????"; বিরতি;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, zCode);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_মান(&str));
  Tcl_DStringFree(&str);
  zReply = rc==TCL_OK ? Tcl_Get_String_ফলাফল(pDb->interp) : "SQLITE_DENY";
  যদি( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }অন্যথা যদি( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }অন্যথা যদি( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }অন্যথা{
    rc = 999;
  }
  ফেরত rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#যদি 0
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
স্থির অক্ষর *local_getline(অক্ষর *zPrompt, FILE *in){
  অক্ষর *z_সারি;
  পূর্ণসংখ্যা n_সারি;
  পূর্ণসংখ্যা n;

  n_সারি = 100;
  z_সারি = malloc( n_সারি );
  যদি( z_সারি==0 ) ফেরত 0;
  n = 0;
  যখন( 1 ){
    যদি( n+100>n_সারি ){
      n_সারি = n_সারি*2 + 100;
      z_সারি = realloc(z_সারি, n_সারি);
      যদি( z_সারি==0 ) ফেরত 0;
    }
    যদি( fgets(&z_সারি[n], n_সারি - n, in)==0 ){
      যদি( n==0 ){
        মুক্ত(z_সারি);
        ফেরত 0;
      }
      z_সারি[n] = 0;
      বিরতি;
    }
    যখন( z_সারি[n] ){ n++; }
    যদি( n>0 && z_সারি[n-1]=='\n' ){
      n--;
      z_সারি[n] = 0;
      বিরতি;
    }
  }
  z_সারি = realloc( z_সারি, n+1 );
  ফেরত z_সারি;
}
#endif


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI DbTransPostCmd(
  Client_তথ্য তথ্য[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  পূর্ণসংখ্যা ফলাফল                           /* Result of evaluating SCRIPT */
){
  স্থির ধ্রুবক অক্ষর *ধ্রুবক azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)তথ্য[0];
  পূর্ণসংখ্যা rc = ফলাফল;
  ধ্রুবক অক্ষর *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  যদি( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    যদি( rc!=TCL_ERROR ){
      Tcl_Append_ফলাফল(interp, sqlite3_errmsg(pDb->db), (অক্ষর*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  ফেরত rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
স্থির পূর্ণসংখ্যা dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  ধ্রুবক অক্ষর *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  ধ্রুবক অক্ষর **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned পূর্ণসংখ্যা prepFlags = 0;
#ifdef SQLITE_TEST
  যদি( pDb->bLegacyPrepare ){
    ফেরত sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  যদি( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  ফেরত sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
স্থির পূর্ণসংখ্যা dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  অক্ষর ধ্রুবক *zIn,                /* SQL to compile */
  অক্ষর ধ্রুবক **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  ধ্রুবক অক্ষর *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  পূর্ণসংখ্যা nSql;                       /* Length of zSql in bytes */
  পূর্ণসংখ্যা nVar = 0;                   /* Number of variables in statement */
  পূর্ণসংখ্যা iParm = 0;                  /* Next free entry in apParm */
  অক্ষর c;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা need_ফলাফল_Reset = 0;        /* Need to invoke Tcl_ResetResult() */
  পূর্ণসংখ্যা rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  যখন( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  জন্য(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->p_পরবর্তী){
    পূর্ণসংখ্যা n = pPreStmt->nSql;
    যদি( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      যদি( pPreStmt->p_পূর্ব ){
        pPreStmt->p_পূর্ব->p_পরবর্তী = pPreStmt->p_পরবর্তী;
      }অন্যথা{
        pDb->stmtList = pPreStmt->p_পরবর্তী;
      }
      যদি( pPreStmt->p_পরবর্তী ){
        pPreStmt->p_পরবর্তী->p_পূর্ব = pPreStmt->p_পূর্ব;
      }অন্যথা{
        pDb->stmtLast = pPreStmt->p_পূর্ব;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_গুনতি(pStmt);
      বিরতি;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  যদি( pPreStmt==0 ){
    পূর্ণসংখ্যা nByte;

    যদি( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      ফেরত TCL_ERROR;
    }
    যদি( pStmt==0 ){
      যদি( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ফেরত TCL_ERROR;
      }অন্যথা{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        ফেরত TCL_OK;
      }
    }

    প্রতিজ্ঞা( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_গুনতি(pStmt);
    nByte = আকার(SqlPreparedStmt) + nVar*আকার(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (পূর্ণসংখ্যা)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    যদি( pPreStmt->zSql==0 ){
      অক্ষর *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      memcpy(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#endif
  }
  প্রতিজ্ঞা( pPreStmt );
  প্রতিজ্ঞা( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  প্রতিজ্ঞা( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  জন্য(i=1; i<=nVar; i++){
    ধ্রুবক অক্ষর *zVar = sqlite3_bind_parameter_name(pStmt, i);
    যদি( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      যদি( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        পূর্ণসংখ্যা rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_গুনতি(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        যদি( need_ফলাফল_Reset ) Tcl_Reset_ফলাফল(interp);
        need_ফলাফল_Reset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_Decr_Ref_গুনতি(pCmd);
        যদি( rx==TCL_OK ){
          pVar = Tcl_Get_Obj_ফলাফল(interp);
        }অন্যথা যদি( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          বিরতি;
        }অন্যথা{
          pVar = 0;
        }
      }
      যদি( pVar ){
        Tcl_আকার n;
        u8 *তথ্য;
        ধ্রুবক অক্ষর *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        যদি( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          তথ্য = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, তথ্য, n, SQLITE_STATIC);
          Tcl_Incr_Ref_গুনতি(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }অন্যথা যদি( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          পূর্ণসংখ্যা nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }অন্যথা যদি( c=='d' && strcmp(zType,"double")==0 ){
          দ্বিগুণ r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }অন্যথা যদি( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }অন্যথা{
          তথ্য = (unsigned অক্ষর *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (অক্ষর *)তথ্য, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_গুনতি(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }অন্যথা{
        sqlite3_bind_null(pStmt, i);
      }
      যদি( need_ফলাফল_Reset ) Tcl_Reset_ফলাফল(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  যদি( need_ফলাফল_Reset && rc==TCL_OK ) Tcl_Reset_ফলাফল(pDb->interp);

  ফেরত rc;
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
স্থির শূন্য dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  পূর্ণসংখ্যা discard                     /* True to delete (not cache) the pPreStmt */
){
  পূর্ণসংখ্যা i;

  /* Free the bound string and blob parameters */
  জন্য(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_গুনতি(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  যদি( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }অন্যথা{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->p_পরবর্তী = pDb->stmtList;
    pPreStmt->p_পূর্ব = 0;
    যদি( pDb->stmtList ){
     pDb->stmtList->p_পূর্ব = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    যদি( pDb->stmtLast==0 ){
      প্রতিজ্ঞা( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }অন্যথা{
      প্রতিজ্ঞা( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    যখন( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *pLast = pDb->stmtLast;
      pDb->stmtLast = pLast->p_পূর্ব;
      pDb->stmtLast->p_পরবর্তী = 0;
      pDb->nStmt--;
      dbFreeStmt(pLast);
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
প্রকারলেখ কাঠামো DbEvalContext DbEvalContext;
কাঠামো DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  ধ্রুবক অক্ষর *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  পূর্ণসংখ্যা nCol;                       /* Number of columns returned by pStmt */
  পূর্ণসংখ্যা evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#define SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#define SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
স্থির শূন্য dbReleaseColumnNames(DbEvalContext *p){
  যদি( p->apColName ){
    পূর্ণসংখ্যা i;
    জন্য(i=0; i<p->nCol; i++){
      Tcl_Decr_Ref_গুনতি(p->apColName[i]);
    }
    Tcl_Free((অক্ষর *)p->apColName);
    p->apColName = 0;
  }
  p->nCol = 0;
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
স্থির শূন্য dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  পূর্ণসংখ্যা evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, আকার(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_গুনতি(pSql);
  যদি( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_গুনতি(pVarName);
  }
  p->evalFlags = evalFlags;
  addDatabaseRef(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
স্থির শূন্য dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  পূর্ণসংখ্যা *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  যদি( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    পূর্ণসংখ্যা i;                        /* Iterator variable */
    পূর্ণসংখ্যা nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_গুনতি(pStmt);
    যদি( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( আকার(Tcl_Obj*)*nCol );
      জন্য(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_Incr_Ref_গুনতি(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    যদি( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_গুনতি(pColList);
      Tcl_Incr_Ref_গুনতি(pStar);
      জন্য(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      যদি( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }অন্যথা{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, ফাঁকা, 0);
        যদি( !pDict ){
          pDict = Tcl_NewDictObj();
        }অন্যথা যদি( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        যদি( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, ফাঁকা, pDict, 0);
        }
        Tcl_Bounce_Ref_গুনতি(pDict);
      }
      Tcl_Decr_Ref_গুনতি(pStar);
      Tcl_Decr_Ref_গুনতি(pColList);
    }
  }

  যদি( papColName ){
    *papColName = p->apColName;
  }
  যদি( pnCol ){
    *pnCol = p->nCol;
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
স্থির পূর্ণসংখ্যা dbEvalStep(DbEvalContext *p){
  ধ্রুবক অক্ষর *z_পূর্ব_Sql = 0;       /* Previous value of p->zSql */

  যখন( p->zSql[0] || p->pPreStmt ){
    পূর্ণসংখ্যা rc;
    যদি( p->pPreStmt==0 ){
      z_পূর্ব_Sql = (p->zSql==z_পূর্ব_Sql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      যদি( rc!=TCL_OK ) ফেরত rc;
    }অন্যথা{
      পূর্ণসংখ্যা rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      যদি( rcs==SQLITE_ROW ){
        ফেরত TCL_OK;
      }
      যদি( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->n_সূচক = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      যদি( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#যদি SQLITE_TEST
        যদি( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && z_পূর্ব_Sql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = z_পূর্ব_Sql;
          চালু;
        }
#endif
        Tcl_Set_Obj_ফলাফল(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ফেরত TCL_ERROR;
      }অন্যথা{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  ফেরত TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
স্থির শূন্য dbEvalFinalize(DbEvalContext *p){
  যদি( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  যদি( p->pVarName ){
    Tcl_Decr_Ref_গুনতি(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_গুনতি(p->pSql);
  dbReleaseColumnNames(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
স্থির Tcl_Obj *db_Eval_Column_মান(DbEvalContext *p, পূর্ণসংখ্যা iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  নির্বাচন( sqlite3_column_type(pStmt, iCol) ){
    ক্ষেত্র SQLITE_BLOB: {
      পূর্ণসংখ্যা bytes = sqlite3_column_bytes(pStmt, iCol);
      ধ্রুবক অক্ষর *zBlob = sqlite3_column_blob(pStmt, iCol);
      যদি( !zBlob ) bytes = 0;
      ফেরত Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    ক্ষেত্র SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      যদি( v>=-2147483647 && v<=2147483647 ){
        ফেরত Tcl_NewIntObj((পূর্ণসংখ্যা)v);
      }অন্যথা{
        ফেরত Tcl_NewWideIntObj(v);
      }
    }
    ক্ষেত্র SQLITE_FLOAT: {
      ফেরত Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    ক্ষেত্র SQLITE_NULL: {
      ফেরত Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  ফেরত Tcl_NewStringObj((অক্ষর*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#যদি TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
স্থির পূর্ণসংখ্যা DbUseNre(শূন্য){
  পূর্ণসংখ্যা major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  ফেরত( (major==8 && minor>=6) || major>8 );
}
#অন্যথা
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (শূন্য)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (শূন্য)0
#endif

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI Db_Eval_পরবর্তী_Cmd(
  Client_তথ্য তথ্য[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  পূর্ণসংখ্যা ফলাফল                           /* Result so far */
){
  পূর্ণসংখ্যা rc = ফলাফল;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)তথ্য[0];
  Tcl_Obj * ধ্রুবক pScript = (Tcl_Obj *)তথ্য[1];
  Tcl_Obj * ধ্রুবক pVarName = p->pVarName;

  যখন( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    পূর্ণসংখ্যা i;
    পূর্ণসংখ্যা nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    জন্য(i=0; i<nCol; i++){
      যদি( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, db_Eval_Column_মান(p,i), 0);
      }অন্যথা যদি( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        যদি( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }অন্যথা{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ফাঁকা, 0);
          যদি( pDict ){
            যদি( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            যদি( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, ফাঁকা, pDict, 0);
            }
            Tcl_Bounce_Ref_গুনতি(pDict);
          }
        }
      }অন্যথা যদি( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       db_Eval_Column_মান(p,i), 0);
      }অন্যথা{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ফাঁকা, 0);
        যদি( !pDict ){
          pDict = Tcl_NewDictObj();
        }অন্যথা যদি( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        যদি( Tcl_DictObjPut(interp, pDict, apColName[i],
                           db_Eval_Column_মান(p,i))==TCL_OK ){
          Tcl_ObjSetVar2(interp, pVarName, ফাঁকা, pDict, 0);
        }
        Tcl_Bounce_Ref_গুনতি(pDict);
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
    যদি( DbUseNre() ){
      Tcl_NRAddCallback(interp, Db_Eval_পরবর্তী_Cmd, (শূন্য*)p, (শূন্য*)pScript, 0, 0);
      ফেরত Tcl_NREvalObj(interp, pScript, 0);
    }অন্যথা{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_গুনতি(pScript);
  dbEvalFinalize(p);
  Tcl_Free((অক্ষর *)p);

  যদি( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_Reset_ফলাফল(interp);
    rc = TCL_OK;
  }
  ফেরত rc;
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
স্থির শূন্য DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  যদি( *ppHook ){
    Tcl_Set_Obj_ফলাফল(interp, *ppHook);
    যদি( pArg ){
      Tcl_Decr_Ref_গুনতি(*ppHook);
      *ppHook = 0;
    }
  }
  যদি( pArg ){
    প্রতিজ্ঞা( !(*ppHook) );
    যদি( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_গুনতি(*ppHook);
    }
  }

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
  sqlite3_preupdate_hook(db, (pDb->pPreUpdateHook?DbPreUpdateHandler:0), pDb);
#endif
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
স্থির পূর্ণসংখ্যা dbQrf(SqliteDb *pDb, পূর্ণসংখ্যা objc, Tcl_Obj *ধ্রুবক*objv){
#ifndef SQLITE_QRF_H
  Tcl_Set_ফলাফল(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  ফেরত TCL_ERROR;
#অন্যথা
  অক্ষর *z_ফলাফল = 0;             /* Result to be returned */
  ধ্রুবক অক্ষর *zSql = 0;          /* SQL to run */
  পূর্ণসংখ্যা i;                         /* Loop counter */
  পূর্ণসংখ্যা rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  স্থির ধ্রুবক অক্ষর *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  স্থির ধ্রুবক unsigned অক্ষর aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_বাম,
    QRF_ALIGN_মধ্য, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_ডান,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_শীর্ষ,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, আকার(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &z_ফলাফল;
  জন্য(i=2; i<objc; i++){
    ধ্রুবক অক্ষর *zArg = Tcl_GetString(objv[i]);
    ধ্রুবক অক্ষর *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    ধ্রুবক unsigned অক্ষর aBoolMap[] = { 0, 2, 1, 2, 1 };
    যদি( zArg[0]!='-' ){
      যদি( zSql ){
        Tcl_Append_ফলাফল(pDb->interp, "unknown argument: ", zArg, (অক্ষর*)0);
        rc = TCL_ERROR;
        যাও format_failed;
      }
      zSql  = zArg;
    }অন্যথা যদি( i==objc-1 ){
      Tcl_Append_ফলাফল(pDb->interp, "option has no argument: ", zArg,(অক্ষর*)0);
      rc = TCL_ERROR;
      যাও format_failed;
    }অন্যথা যদি( strcmp(zArg,"-style")==0 ){
      স্থির ধ্রুবক অক্ষর *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      স্থির unsigned অক্ষর aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_গুনতি,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_ঢোকাও,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_সারি,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      পূর্ণসংখ্যা style;
      rc = Tcl_Get_সূচক_From_Obj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      যদি( rc ) যাও format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }অন্যথা যদি( strcmp(zArg,"-esc")==0 ){
      স্থির ধ্রুবক অক্ষর *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      স্থির unsigned অক্ষর aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_প্রতীক
      };
      পূর্ণসংখ্যা esc;
      rc = Tcl_Get_সূচক_From_Obj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      যদি( rc ) যাও format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }অন্যথা যদি( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      স্থির ধ্রুবক অক্ষর *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      স্থির unsigned অক্ষর aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      পূর্ণসংখ্যা txt;
      পূর্ণসংখ্যা k = zArg[2]=='e';
      rc = Tcl_Get_সূচক_From_Obj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      যদি( rc ) যাও format_failed;
      যদি( k ){
        qrf.eText = aTextMap[txt];
      }অন্যথা যদি( txt<=2 ){
        প্রতিজ্ঞা( QRF_No==1 );
        প্রতিজ্ঞা( QRF_Yes==2 );
        প্রতিজ্ঞা( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }অন্যথা{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }অন্যথা যদি( strcmp(zArg,"-blob")==0 ){
      স্থির ধ্রুবক অক্ষর *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      স্থির unsigned অক্ষর aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_আকার
      };
      পূর্ণসংখ্যা blob;
      rc = Tcl_Get_সূচক_From_Obj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      যদি( rc ) যাও format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }অন্যথা যদি( strcmp(zArg,"-wordwrap")==0 ){
      পূর্ণসংখ্যা v = 0;
      rc = Tcl_Get_সূচক_From_Obj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      যদি( rc ) যাও format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }অন্যথা যদি( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      পূর্ণসংখ্যা v = 0;
      rc = Tcl_Get_সূচক_From_Obj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      যদি( rc ) যাও format_failed;
      যদি( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }অন্যথা যদি( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }অন্যথা যদি( zArg[1]=='r' ){
        qrf.b_Row_গুনতি = aBoolMap[v];
      }অন্যথা{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }অন্যথা যদি( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      পূর্ণসংখ্যা ax = 0;
      rc = Tcl_Get_সূচক_From_Obj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      যদি( rc ) যাও format_failed;
      যদি( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }অন্যথা{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }অন্যথা যদি( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      পূর্ণসংখ্যা v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      যদি( rc ) যাও format_failed;
      যদি( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }অন্যথা যদি( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      যদি( zArg[1]=='w' ){
        qrf.nWrap = v;
      }অন্যথা যদি( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }অন্যথা যদি( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }অন্যথা{
        qrf.n_সারি_Limit = v;
      }
      i++;
    }অন্যথা যদি( strcmp(zArg,"-charlimit")==0 ){
      পূর্ণসংখ্যা v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      যদি( rc ) যাও format_failed;
      যদি( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }অন্যথা যদি( strcmp(zArg,"-multiinsert")==0 ){
      পূর্ণসংখ্যা v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      যদি( rc ) যাও format_failed;
      যদি( v<0 ) v = 0;
      qrf.n_Multi_ঢোকাও = v;
      i++;
    }অন্যথা যদি( strcmp(zArg,"-align")==0 ){
      Tcl_আকার n = 0;
      পূর্ণসংখ্যা jj;
      rc = Tcl_List_Obj_দৈর্ঘ্য(pDb->interp, objv[i+1], &n);
      যদি( rc ) যাও format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*আকার(qrf.aAlign[0]) );
      যদি( qrf.aAlign==0 ){
        Tcl_Append_ফলাফল(pDb->interp, "out of memory", (অক্ষর*)0);
        rc = TCL_ERROR;
        যাও format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*আকার(qrf.aAlign[0]));
      qrf.nAlign = n;
      জন্য(jj=0; jj<n; jj++){
        পূর্ণসংখ্যা x;
        Tcl_Obj *pTerm;
        rc = Tcl_List_Obj_সূচক(pDb->interp, objv[i+1], jj, &pTerm);
        যদি( rc ) যাও format_failed;
        rc = Tcl_Get_সূচক_From_Obj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        যদি( rc ) যাও format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }অন্যথা যদি( strcmp(zArg,"-widths")==0 ){
      Tcl_আকার n = 0;
      পূর্ণসংখ্যা jj;
      rc = Tcl_List_Obj_দৈর্ঘ্য(pDb->interp, objv[i+1], &n);
      যদি( rc ) যাও format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*আকার(qrf.aWidth[0]) );
      যদি( qrf.aWidth==0 ){
        Tcl_Append_ফলাফল(pDb->interp, "out of memory", (অক্ষর*)0);
        rc = TCL_ERROR;
        যাও format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*আকার(qrf.aWidth[0]));
      qrf.nWidth = n;
      জন্য(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        পূর্ণসংখ্যা v;
        rc = Tcl_List_Obj_সূচক(pDb->interp, objv[i+1], jj, &pTerm);
        যদি( rc ) যাও format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        যদি( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }অন্যথা যদি( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short পূর্ণসংখ্যা)v;
      }
      i++;
    }অন্যথা যদি( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }অন্যথা যদি( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }অন্যথা যদি( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }অন্যথা যদি( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }অন্যথা যদি( strcmp(zArg,"-ifmt")==0 ){
      অক্ষর *zFmt = Tcl_GetString(objv[i+1]);
      যদি( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }অন্যথা যদি( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_Append_ফলাফল(pDb->interp, "bad format for -ifmt", (অক্ষর*)0);
        rc = TCL_ERROR;
        যাও format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }অন্যথা যদি( strcmp(zArg,"-fpfmt")==0 ){
      অক্ষর *zFmt = Tcl_GetString(objv[i+1]);
      যদি( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }অন্যথা যদি( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_Append_ফলাফল(pDb->interp, "bad format for -fpfmt", (অক্ষর*)0);
        rc = TCL_ERROR;
        যাও format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }অন্যথা যদি( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }অন্যথা{
      Tcl_Append_ফলাফল(pDb->interp, "unknown option: ", zArg, (অক্ষর*)0);
      rc = TCL_ERROR;
      যাও format_failed;
    }
  }
  যখন( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    অক্ষর *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    যদি( rc ) যাও format_failed;
    যদি( pStmt==0 ) চালু;
    rc = sqlite3_format_query_ফলাফল(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    যদি( rc ){
      Tcl_Set_ফলাফল(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      যাও format_failed;
    }
  }
  Tcl_Set_ফলাফল(pDb->interp, z_ফলাফল, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.aWidth);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(z_ফলাফল);
  ফেরত rc;

#endif
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI DbObjCmd(
  শূন্য *cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *ধ্রুবক*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  পূর্ণসংখ্যা choice;
  পূর্ণসংখ্যা rc = TCL_OK;
  স্থির ধ্রুবক অক্ষর *DB_strs[] = {
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
  গণনা DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_LOAD_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_INTERRUPT,            DB_LAST_ঢোকাও_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_PROGRESS,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_STATUS,                DB_TIMEOUT,              DB_যোগ_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_VERSION,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUBCOMMAND ...");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_Get_সূচক_From_Obj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    ফেরত TCL_ERROR;
  }

  নির্বাচন( (গণনা DB_enum)choice ){

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
  ক্ষেত্র DB_AUTHORIZER: {
#ifdef SQLITE_OMIT_AUTHORIZATION
    Tcl_Append_ফলাফল(interp, "authorization not available in this build",
                     (অক্ষর*)0);
    ফেরত TCL_ERROR;
#অন্যথা
    যদি( objc>3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?CALLBACK?");
      ফেরত TCL_ERROR;
    }অন্যথা যদি( objc==2 ){
      যদি( pDb->zAuth ){
        Tcl_Append_ফলাফল(interp, pDb->zAuth, (অক্ষর*)0);
      }
    }অন্যথা{
      অক্ষর *zAuth;
      Tcl_আকার len;
      যদি( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      যদি( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }অন্যথা{
        pDb->zAuth = 0;
      }
      যদি( pDb->zAuth ){
        প্রকারলেখ পূর্ণসংখ্যা (*sqlite3_auth_cb)(
           শূন্য*,পূর্ণসংখ্যা,ধ্রুবক অক্ষর*,ধ্রুবক অক্ষর*,
           ধ্রুবক অক্ষর*,ধ্রুবক অক্ষর*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }অন্যথা{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#endif
    বিরতি;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  ক্ষেত্র DB_BACKUP: {
    ধ্রুবক অক্ষর *zDestFile;
    ধ্রুবক অক্ষর *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    যদি( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }অন্যথা যদি( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }অন্যথা{
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?DATABASE? FILENAME");
      ফেরত TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    যদি( rc!=SQLITE_OK ){
      Tcl_Append_ফলাফল(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (অক্ষর*)0);
      sqlite3_close(pDest);
      ফেরত TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    যদি( pBackup==0 ){
      Tcl_Append_ফলাফল(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (অক্ষর*)0);
      sqlite3_close(pDest);
      ফেরত TCL_ERROR;
    }
    যখন(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    যদি( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }অন্যথা{
      Tcl_Append_ফলাফল(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (অক্ষর*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    বিরতি;
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
  ক্ষেত্র DB_BIND_FALLBACK: {
    যদি( objc>3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?CALLBACK?");
      ফেরত TCL_ERROR;
    }অন্যথা যদি( objc==2 ){
      যদি( pDb->zBindFallback ){
        Tcl_Append_ফলাফল(interp, pDb->zBindFallback, (অক্ষর*)0);
      }
    }অন্যথা{
      অক্ষর *zCallback;
      Tcl_আকার len;
      যদি( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      যদি( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }অন্যথা{
        pDb->zBindFallback = 0;
      }
    }
    বিরতি;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  ক্ষেত্র DB_BUSY: {
    যদি( objc>3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "CALLBACK");
      ফেরত TCL_ERROR;
    }অন্যথা যদি( objc==2 ){
      যদি( pDb->zBusy ){
        Tcl_Append_ফলাফল(interp, pDb->zBusy, (অক্ষর*)0);
      }
    }অন্যথা{
      অক্ষর *zBusy;
      Tcl_আকার len;
      যদি( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      যদি( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }অন্যথা{
        pDb->zBusy = 0;
      }
      যদি( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }অন্যথা{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    বিরতি;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  ক্ষেত্র DB_CACHE: {
    অক্ষর *subCmd;
    পূর্ণসংখ্যা n;

    যদি( objc<=2 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "cache option ?arg?");
      ফেরত TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    যদি( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      যদি( objc!=3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "flush");
        ফেরত TCL_ERROR;
      }অন্যথা{
        flushStmtCache( pDb );
      }
    }অন্যথা যদি( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      যদি( objc!=4 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "size n");
        ফেরত TCL_ERROR;
      }অন্যথা{
        যদি( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_Append_ফলাফল( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (অক্ষর*)0);
          ফেরত TCL_ERROR;
        }অন্যথা{
          যদি( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }অন্যথা যদি( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }অন্যথা{
      Tcl_Append_ফলাফল( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (অক্ষর*)0);
      ফেরত TCL_ERROR;
    }
    বিরতি;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  ক্ষেত্র DB_CHANGES: {
    Tcl_Obj *p_ফলাফল;
    যদি( objc!=2 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
      ফেরত TCL_ERROR;
    }
    p_ফলাফল = Tcl_Get_Obj_ফলাফল(interp);
    Tcl_SetWideIntObj(p_ফলাফল, sqlite3_changes64(pDb->db));
    বিরতি;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  ক্ষেত্র DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    বিরতি;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  ক্ষেত্র DB_COLLATE: {
    SqlCollate *pCollate;
    অক্ষর *zName;
    অক্ষর *zScript;
    Tcl_আকার nScript;
    যদি( objc!=4 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "NAME SCRIPT");
      ফেরত TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( আকার(*pCollate) + nScript + 1 );
    যদি( pCollate==0 ) ফেরত TCL_ERROR;
    pCollate->interp = interp;
    pCollate->p_পরবর্তী = pDb->pCollate;
    pCollate->zScript = (অক্ষর*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    যদি( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      ফেরত TCL_ERROR;
    }
    বিরতি;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  ক্ষেত্র DB_COLLATION_NEEDED: {
    যদি( objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "SCRIPT");
      ফেরত TCL_ERROR;
    }
    যদি( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_গুনতি(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_গুনতি(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    বিরতি;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  ক্ষেত্র DB_COMMIT_HOOK: {
    যদি( objc>3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?CALLBACK?");
      ফেরত TCL_ERROR;
    }অন্যথা যদি( objc==2 ){
      যদি( pDb->zCommit ){
        Tcl_Append_ফলাফল(interp, pDb->zCommit, (অক্ষর*)0);
      }
    }অন্যথা{
      ধ্রুবক অক্ষর *zCommit;
      Tcl_আকার len;
      যদি( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      যদি( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }অন্যথা{
        pDb->zCommit = 0;
      }
      যদি( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }অন্যথা{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    বিরতি;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  ক্ষেত্র DB_COMPLETE: {
#ifndef SQLITE_OMIT_COMPLETE
    Tcl_Obj *p_ফলাফল;
    পূর্ণসংখ্যা isComplete;
    যদি( objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "SQL");
      ফেরত TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    p_ফলাফল = Tcl_Get_Obj_ফলাফল(interp);
    Tcl_SetBooleanObj(p_ফলাফল, isComplete);
#endif
    বিরতি;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  ক্ষেত্র DB_CONFIG: {
    স্থির ধ্রুবক কাঠামো DbConfigChoices {
      ধ্রুবক অক্ষর *zName;
      পূর্ণসংখ্যা op;
    } aDbConfig[] = {
        { "defensive",          SQLITE_DBCONFIG_DEFENSIVE             },
        { "dqs_ddl",            SQLITE_DBCONFIG_DQS_DDL               },
        { "dqs_dml",            SQLITE_DBCONFIG_DQS_DML               },
        { "enable_fkey",        SQLITE_DBCONFIG_ENABLE_FKEY           },
        { "enable_qpsg",        SQLITE_DBCONFIG_ENABLE_QPSG           },
        { "enable_trigger",     SQLITE_DBCONFIG_ENABLE_TRIGGER        },
        { "enable_view",        SQLITE_DBCONFIG_ENABLE_VIEW           },
        { "fts3_tokenizer",     SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER },
        { "legacy_alter_table", SQLITE_DBCONFIG_LEGACY_ALTER_TABLE    },
        { "legacy_file_format", SQLITE_DBCONFIG_LEGACY_FILE_FORMAT    },
        { "load_extension",     SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION },
        { "no_ckpt_on_close",   SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE      },
        { "reset_database",     SQLITE_DBCONFIG_RESET_DATABASE        },
        { "trigger_eqp",        SQLITE_DBCONFIG_TRIGGER_EQP           },
        { "trusted_schema",     SQLITE_DBCONFIG_TRUSTED_SCHEMA        },
        { "writable_schema",    SQLITE_DBCONFIG_WRITABLE_SCHEMA       },
    };
    Tcl_Obj *p_ফলাফল;
    পূর্ণসংখ্যা ii;
    যদি( objc>4 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      ফেরত TCL_ERROR;
    }
    যদি( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      p_ফলাফল = Tcl_NewListObj(0,0);
      জন্য(ii=0; ii<আকার(aDbConfig)/আকার(aDbConfig[0]); ii++){
        পূর্ণসংখ্যা v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, p_ফলাফল,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, p_ফলাফল,
           Tcl_NewIntObj(v));
      }
    }অন্যথা{
      ধ্রুবক অক্ষর *zOpt = Tcl_GetString(objv[2]);
      পূর্ণসংখ্যা onoff = -1;
      পূর্ণসংখ্যা v = 0;
      যদি( zOpt[0]=='-' ) zOpt++;
      জন্য(ii=0; ii<আকার(aDbConfig)/আকার(aDbConfig[0]); ii++){
        যদি( strcmp(aDbConfig[ii].zName, zOpt)==0 ) বিরতি;
      }
      যদি( ii>=আকার(aDbConfig)/আকার(aDbConfig[0]) ){
        Tcl_Append_ফলাফল(interp, "unknown config option: \"", zOpt,
                                "\"", (শূন্য*)0);
        ফেরত TCL_ERROR;
      }
      যদি( objc==4 ){
        যদি( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          ফেরত TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      p_ফলাফল = Tcl_NewIntObj(v);
    }
    Tcl_Set_Obj_ফলাফল(interp, p_ফলাফল);
    বিরতি;
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
  ক্ষেত্র DB_COPY: {
    অক্ষর *zTable;               /* Insert data into this table */
    অক্ষর *zFile;                /* The file from which to extract data */
    অক্ষর *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    পূর্ণসংখ্যা nCol;                   /* Number of columns in the table */
    পূর্ণসংখ্যা nByte;                  /* Number of bytes in an SQL string */
    পূর্ণসংখ্যা i, j;                   /* Loop counters */
    পূর্ণসংখ্যা nSep;                   /* Number of bytes in zSep[] */
    পূর্ণসংখ্যা nNull;                  /* Number of bytes in zNull[] */
    অক্ষর *zSql;                 /* An SQL statement */
    অক্ষর *z_সারি;                /* A single line of input from the file */
    অক্ষর **azCol;               /* zLine[] broken up into columns */
    ধ্রুবক অক্ষর *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    পূর্ণসংখ্যা lineno = 0;             /* Line number of input file */
    অক্ষর z_সারি_অঙ্ক[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *p_ফলাফল;           /* interp result */

    ধ্রুবক অক্ষর *zSep;
    ধ্রুবক অক্ষর *zNull;
    যদি( objc<5 || objc>7 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      ফেরত TCL_ERROR;
    }
    যদি( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }অন্যথা{
      zSep = "\t";
    }
    যদি( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }অন্যথা{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    যদি( nSep==0 ){
      Tcl_Append_ফলাফল(interp,"Error: non-null separator required for copy",
                       (অক্ষর*)0);
      ফেরত TCL_ERROR;
    }
    যদি(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_Append_ফলাফল(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (অক্ষর*)0);
      ফেরত TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    যদি( zSql==0 ){
      Tcl_Append_ফলাফল(interp, "Error: no such table: ", zTable, (অক্ষর*)0);
      ফেরত TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    যদি( rc ){
      Tcl_Append_ফলাফল(interp, "Error: ", sqlite3_errmsg(pDb->db), (অক্ষর*)0);
      nCol = 0;
    }অন্যথা{
      nCol = sqlite3_column_গুনতি(pStmt);
    }
    sqlite3_finalize(pStmt);
    যদি( nCol==0 ) {
      ফেরত TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    যদি( zSql==0 ) {
      Tcl_Append_ফলাফল(interp, "Error: can't malloc()", (অক্ষর*)0);
      ফেরত TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    জন্য(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    মুক্ত(zSql);
    যদি( rc ){
      Tcl_Append_ফলাফল(interp, "Error: ", sqlite3_errmsg(pDb->db), (অক্ষর*)0);
      sqlite3_finalize(pStmt);
      ফেরত TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    যদি( in==0 ){
      sqlite3_finalize(pStmt);
      ফেরত TCL_ERROR;
    }
    Tcl_SetChannelOption(ফাঁকা, in, "-translation", "auto");
    azCol = malloc( আকার(azCol[0])*(nCol+1) );
    যদি( azCol==0 ) {
      Tcl_Append_ফলাফল(interp, "Error: can't malloc()", (অক্ষর*)0);
      Tcl_Close(interp, in);
      ফেরত TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_গুনতি(str);
    (শূন্য)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    যখন( Tcl_GetsObj(in, str)>=0 ) {
      অক্ষর *z;
      Tcl_আকার byteLen;
      lineno++;
      z_সারি = (অক্ষর *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = z_সারি;
      জন্য(i=0, z=z_সারি; *z; z++){
        যদি( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          যদি( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      যদি( i+1!=nCol ){
        অক্ষর *zErr;
        পূর্ণসংখ্যা nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        যদি( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_Append_ফলাফল(interp, zErr, (অক্ষর*)0);
          মুক্ত(zErr);
        }
        zCommit = "ROLLBACK";
        বিরতি;
      }
      জন্য(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        যদি( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }অন্যথা{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_Set_Obj_দৈর্ঘ্য(str, 0);
      যদি( rc!=SQLITE_OK ){
        Tcl_Append_ফলাফল(interp,"Error: ", sqlite3_errmsg(pDb->db), (অক্ষর*)0);
        zCommit = "ROLLBACK";
        বিরতি;
      }
    }
    Tcl_Decr_Ref_গুনতি(str);
    মুক্ত(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (শূন্য)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    যদি( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      p_ফলাফল = Tcl_Get_Obj_ফলাফল(interp);
      যদি( Tcl_IsShared(p_ফলাফল) ){
        p_ফলাফল = Tcl_DuplicateObj(p_ফলাফল);
      }
      Tcl_SetIntObj(p_ফলাফল, lineno);
      rc = TCL_OK;
    }অন্যথা{
      /* failure, append lineno where failed */
      sqlite3_snprintf(আকার(z_সারি_অঙ্ক), z_সারি_অঙ্ক,"%d",lineno);
      Tcl_Append_ফলাফল(interp,", failed while processing line: ",z_সারি_অঙ্ক,
                       (অক্ষর*)0);
      rc = TCL_ERROR;
    }
    বিরতি;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  ক্ষেত্র DB_DESERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_Append_ফলাফল(interp, "MEMDB not available in this build",
                     (অক্ষর*)0);
    rc = TCL_ERROR;
#অন্যথা
    ধ্রুবক অক্ষর *zSchema = 0;
    Tcl_Obj *p_মান = 0;
    unsigned অক্ষর *pBA;
    unsigned অক্ষর *p_তথ্য;
    Tcl_আকার len;
    পূর্ণসংখ্যা xrc;
    sqlite3_int64 mx_আকার = 0;
    পূর্ণসংখ্যা i;
    পূর্ণসংখ্যা isReadonly = 0;


    যদি( objc<3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      বিরতি;
    }
    জন্য(i=2; i<objc-1; i++){
      ধ্রুবক অক্ষর *z = Tcl_GetString(objv[i]);
      যদি( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        যদি( rc ) যাও deserialize_error;
        mx_আকার = x;
        চালু;
      }
      যদি( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        যদি( rc ) যাও deserialize_error;
        চালু;
      }
      যদি( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        চালু;
      }
      Tcl_Append_ফলাফল(interp, "unknown option: ", z, (অক্ষর*)0);
      rc = TCL_ERROR;
      যাও deserialize_error;
    }
    p_মান = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_মান, &len);
    p_তথ্য = sqlite3_malloc64( len );
    যদি( p_তথ্য==0 && len>0 ){
      Tcl_Append_ফলাফল(interp, "out of memory", (অক্ষর*)0);
      rc = TCL_ERROR;
    }অন্যথা{
      পূর্ণসংখ্যা flags;
      যদি( len>0 ) memcpy(p_তথ্য, pBA, len);
      যদি( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }অন্যথা{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, p_তথ্য, len, len, flags);
      যদি( xrc ){
        Tcl_Append_ফলাফল(interp, "unable to set MEMDB content", (অক্ষর*)0);
        rc = TCL_ERROR;
      }
      যদি( mx_আকার>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_আকার_LIMIT,&mx_আকার);
      }
    }
deserialize_error:
#endif
    বিরতি; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  ক্ষেত্র DB_ENABLE_LOAD_EXTENSION: {
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    পূর্ণসংখ্যা onoff;
    যদি( objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "BOOLEAN");
      ফেরত TCL_ERROR;
    }
    যদি( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      ফেরত TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    বিরতি;
#অন্যথা
    Tcl_Append_ফলাফল(interp, "extension loading is turned off at compile-time",
                     (অক্ষর*)0);
    ফেরত TCL_ERROR;
#endif
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  ক্ষেত্র DB_ERRORCODE: {
    Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    বিরতি;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  ক্ষেত্র DB_ERROROFFSET: {
    Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    বিরতি;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  ক্ষেত্র DB_EXISTS:
  ক্ষেত্র DB_ONECOLUMN: {
    Tcl_Obj *p_ফলাফল = 0;
    DbEvalContext sEval;
    যদি( objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "SQL");
      ফেরত TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    যদি( choice==DB_ONECOLUMN ){
      যদি( rc==TCL_OK ){
        p_ফলাফল = db_Eval_Column_মান(&sEval, 0);
      }অন্যথা যদি( rc==TCL_BREAK ){
        Tcl_Reset_ফলাফল(interp);
      }
    }অন্যথা যদি( rc==TCL_BREAK || rc==TCL_OK ){
      p_ফলাফল = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    যদি( p_ফলাফল ) Tcl_Set_Obj_ফলাফল(interp, p_ফলাফল);

    যদি( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    বিরতি;
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
  ক্ষেত্র DB_EVAL: {
    পূর্ণসংখ্যা evalFlags = 0;
    ধ্রুবক অক্ষর *zOpt;
    যখন( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      যদি( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }অন্যথা যদি( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }অন্যথা{
        Tcl_Append_ফলাফল(interp, "unknown option: \"", zOpt, "\"", (শূন্য*)0);
        ফেরত TCL_ERROR;
      }
      objc--;
      objv++;
    }
    যদি( objc<3 || objc>5 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      ফেরত TCL_ERROR;
    }

    যদি( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_গুনতি(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      যখন( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        পূর্ণসংখ্যা i;
        পূর্ণসংখ্যা nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        জন্য(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_Column_মান(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      যদি( rc==TCL_BREAK ){
        Tcl_Set_Obj_ফলাফল(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_গুনতি(pRet);
    }অন্যথা{
      Client_তথ্য cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      যদি( objc>=5 && *(অক্ষর *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_গুনতি(pScript);

      p = (DbEvalContext *)Tcl_Alloc(আকার(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (শূন্য *)p;
      cd2[1] = (শূন্য *)pScript;
      rc = Db_Eval_পরবর্তী_Cmd(cd2, interp, TCL_OK);
    }
    বিরতি;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  ক্ষেত্র DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    বিরতি;
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
  ক্ষেত্র DB_FUNCTION: {
    পূর্ণসংখ্যা flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    অক্ষর *zName;
    পূর্ণসংখ্যা nArg = -1;
    পূর্ণসংখ্যা i;
    পূর্ণসংখ্যা eType = SQLITE_NULL;
    যদি( objc<4 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      ফেরত TCL_ERROR;
    }
    জন্য(i=3; i<(objc-1); i++){
      ধ্রুবক অক্ষর *z = Tcl_GetString(objv[i]);
      পূর্ণসংখ্যা n = strlen30(z);
      যদি( n>1 && strncmp(z, "-argcount",n)==0 ){
        যদি( i==(objc-2) ){
          Tcl_Append_ফলাফল(interp, "option requires an argument: ", z,(অক্ষর*)0);
          ফেরত TCL_ERROR;
        }
        যদি( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) ফেরত TCL_ERROR;
        যদি( nArg<0 ){
          Tcl_Append_ফলাফল(interp, "number of arguments must be non-negative",
                           (অক্ষর*)0);
          ফেরত TCL_ERROR;
        }
        i++;
      }অন্যথা
      যদি( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }অন্যথা
      যদি( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }অন্যথা
      যদি( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }অন্যথা
      যদি( n>1 && strncmp(z, "-returntype", n)==0 ){
        ধ্রুবক অক্ষর *azType[] = {"integer", "real", "text", "blob", "any", 0};
        প্রতিজ্ঞা( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        প্রতিজ্ঞা( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        যদি( i==(objc-2) ){
          Tcl_Append_ফলাফল(interp, "option requires an argument: ", z,(অক্ষর*)0);
          ফেরত TCL_ERROR;
        }
        i++;
        যদি( Tcl_Get_সূচক_From_Obj(interp, objv[i], azType, "type", 0, &eType) ){
          ফেরত TCL_ERROR;
        }
        eType++;
      }অন্যথা{
        Tcl_Append_ফলাফল(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (অক্ষর*)0
        );
        ফেরত TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    যদি( pFunc==0 ) ফেরত TCL_ERROR;
    যদি( pFunc->pScript ){
      Tcl_Decr_Ref_গুনতি(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_গুনতি(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    যদি( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    বিরতি;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  ক্ষেত্র DB_INCRBLOB: {
#ifdef SQLITE_OMIT_INCRBLOB
    Tcl_Append_ফলাফল(interp, "incrblob not available in this build", (অক্ষর*)0);
    ফেরত TCL_ERROR;
#অন্যথা
    পূর্ণসংখ্যা isReadonly = 0;
    ধ্রুবক অক্ষর *zDb = "main";
    ধ্রুবক অক্ষর *zTable;
    ধ্রুবক অক্ষর *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    যদি( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    যদি( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      ফেরত TCL_ERROR;
    }

    যদি( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    যদি( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#endif
    বিরতি;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  ক্ষেত্র DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    বিরতি;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  ক্ষেত্র DB_NULLVALUE: {
    যদি( objc!=2 && objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "NULLVALUE");
      ফেরত TCL_ERROR;
    }
    যদি( objc==3 ){
      Tcl_আকার len;
      অক্ষর *zNull = Tcl_GetStringFromObj(objv[2], &len);
      যদি( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      যদি( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }অন্যথা{
        pDb->zNull = 0;
      }
    }
    Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(pDb->zNull, -1));
    বিরতি;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  ক্ষেত্র DB_LAST_ঢোকাও_ROWID: {
    Tcl_Obj *p_ফলাফল;
    Tcl_WideInt rowid;
    যদি( objc!=2 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
      ফেরত TCL_ERROR;
    }
    rowid = sqlite3_last_ঢোকাও_rowid(pDb->db);
    p_ফলাফল = Tcl_Get_Obj_ফলাফল(interp);
    Tcl_SetWideIntObj(p_ফলাফল, rowid);
    বিরতি;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  ক্ষেত্র DB_PROGRESS: {
    যদি( objc==2 ){
      যদি( pDb->zProgress ){
        Tcl_Append_ফলাফল(interp, pDb->zProgress, (অক্ষর*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }অন্যথা যদি( objc==4 ){
      অক্ষর *zProgress;
      Tcl_আকার len;
      পূর্ণসংখ্যা N;
      যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        ফেরত TCL_ERROR;
      };
      যদি( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      যদি( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }অন্যথা{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      যদি( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }অন্যথা{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }অন্যথা{
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "N CALLBACK");
      ফেরত TCL_ERROR;
    }
    বিরতি;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  ক্ষেত্র DB_PROFILE: {
    যদি( objc>3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?CALLBACK?");
      ফেরত TCL_ERROR;
    }অন্যথা যদি( objc==2 ){
      যদি( pDb->zProfile ){
        Tcl_Append_ফলাফল(interp, pDb->zProfile, (অক্ষর*)0);
      }
    }অন্যথা{
      অক্ষর *zProfile;
      Tcl_আকার len;
      যদি( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      যদি( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }অন্যথা{
        pDb->zProfile = 0;
      }
#যদি !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      যদি( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }অন্যথা{
        sqlite3_profile(pDb->db, 0, 0);
      }
#endif
    }
    বিরতি;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  ক্ষেত্র DB_REKEY: {
    যদি( objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "KEY");
      ফেরত TCL_ERROR;
    }
    বিরতি;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  ক্ষেত্র DB_RESTORE: {
    ধ্রুবক অক্ষর *zSrcFile;
    ধ্রুবক অক্ষর *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    পূর্ণসংখ্যা nTimeout = 0;

    যদি( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }অন্যথা যদি( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }অন্যথা{
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?DATABASE? FILENAME");
      ফেরত TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    যদি( rc!=SQLITE_OK ){
      Tcl_Append_ফলাফল(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (অক্ষর*)0);
      sqlite3_close(pSrc);
      ফেরত TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    যদি( pBackup==0 ){
      Tcl_Append_ফলাফল(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (অক্ষর*)0);
      sqlite3_close(pSrc);
      ফেরত TCL_ERROR;
    }
    যখন( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      যদি( rc==SQLITE_BUSY ){
        যদি( nTimeout++ >= 3 ) বিরতি;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    যদি( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }অন্যথা যদি( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_Append_ফলাফল(interp, "restore failed: source database busy",
                       (অক্ষর*)0);
      rc = TCL_ERROR;
    }অন্যথা{
      Tcl_Append_ফলাফল(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (অক্ষর*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    বিরতি;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  ক্ষেত্র DB_SERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_Append_ফলাফল(interp, "MEMDB not available in this build",
                     (অক্ষর*)0);
    rc = TCL_ERROR;
#অন্যথা
    ধ্রুবক অক্ষর *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned অক্ষর *p_তথ্য;
    যদি( objc!=2 && objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }অন্যথা{
      পূর্ণসংখ্যা needFree;
      p_তথ্য = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      যদি( p_তথ্য ){
        needFree = 0;
      }অন্যথা{
        p_তথ্য = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewByteArrayObj(p_তথ্য,sz));
      যদি( needFree ) sqlite3_free(p_তথ্য);
    }
#endif
    বিরতি;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  ক্ষেত্র DB_STATUS: {
    পূর্ণসংখ্যা v;
    ধ্রুবক অক্ষর *zOp;
    যদি( objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "(step|sort|autoindex)");
      ফেরত TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    যদি( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }অন্যথা যদি( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }অন্যথা যদি( strcmp(zOp, "autoindex")==0 ){
      v = pDb->n_সূচক;
    }অন্যথা যদি( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }অন্যথা{
      Tcl_Append_ফলাফল(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (অক্ষর*)0);
      ফেরত TCL_ERROR;
    }
    Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(v));
    বিরতি;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  ক্ষেত্র DB_TIMEOUT: {
    পূর্ণসংখ্যা ms;
    যদি( objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "MILLISECONDS");
      ফেরত TCL_ERROR;
    }
    যদি( Tcl_GetIntFromObj(interp, objv[2], &ms) ) ফেরত TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    বিরতি;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  ক্ষেত্র DB_যোগ_CHANGES: {
    Tcl_Obj *p_ফলাফল;
    যদি( objc!=2 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
      ফেরত TCL_ERROR;
    }
    p_ফলাফল = Tcl_Get_Obj_ফলাফল(interp);
    Tcl_SetWideIntObj(p_ফলাফল, sqlite3_যোগ_changes64(pDb->db));
    বিরতি;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  ক্ষেত্র DB_TRACE: {
    যদি( objc>3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?CALLBACK?");
      ফেরত TCL_ERROR;
    }অন্যথা যদি( objc==2 ){
      যদি( pDb->zTrace ){
        Tcl_Append_ফলাফল(interp, pDb->zTrace, (অক্ষর*)0);
      }
    }অন্যথা{
      অক্ষর *zTrace;
      Tcl_আকার len;
      যদি( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      যদি( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }অন্যথা{
        pDb->zTrace = 0;
      }
#যদি !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      যদি( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }অন্যথা{
        sqlite3_trace(pDb->db, 0, 0);
      }
#endif
    }
    বিরতি;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  ক্ষেত্র DB_TRACE_V2: {
    যদি( objc>4 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?CALLBACK? ?MASK?");
      ফেরত TCL_ERROR;
    }অন্যথা যদি( objc==2 ){
      যদি( pDb->zTraceV2 ){
        Tcl_Append_ফলাফল(interp, pDb->zTraceV2, (অক্ষর*)0);
      }
    }অন্যথা{
      অক্ষর *zTraceV2;
      Tcl_আকার len;
      Tcl_WideInt wMask = 0;
      যদি( objc==4 ){
        স্থির ধ্রুবক অক্ষর *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        গণনা TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_আকার i;
        যদি( TCL_OK!=Tcl_List_Obj_দৈর্ঘ্য(interp, objv[3], &len) ){
          ফেরত TCL_ERROR;
        }
        জন্য(i=0; i<len; i++){
          Tcl_Obj *pObj;
          পূর্ণসংখ্যা ttype;
          যদি( TCL_OK!=Tcl_List_Obj_সূচক(interp, objv[3], i, &pObj) ){
            ফেরত TCL_ERROR;
          }
          যদি( Tcl_Get_সূচক_From_Obj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_Get_Obj_ফলাফল(interp));
            Tcl_Incr_Ref_গুনতি(pError);
            যদি( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_গুনতি(pError);
              wMask |= wType;
            }অন্যথা{
              Tcl_Set_Obj_ফলাফল(interp, pError);
              Tcl_Decr_Ref_গুনতি(pError);
              ফেরত TCL_ERROR;
            }
          }অন্যথা{
            নির্বাচন( (গণনা TTYPE_enum)ttype ){
              ক্ষেত্র TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    বিরতি;
              ক্ষেত্র TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; বিরতি;
              ক্ষেত্র TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     বিরতি;
              ক্ষেত্র TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   বিরতি;
            }
          }
        }
      }অন্যথা{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      যদি( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      যদি( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }অন্যথা{
        pDb->zTraceV2 = 0;
      }
#যদি !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      যদি( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }অন্যথা{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#endif
    }
    বিরতি;
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
  ক্ষেত্র DB_TRANSACTION: {
    Tcl_Obj *pScript;
    ধ্রুবক অক্ষর *zBegin = "SAVEPOINT _tcl_transaction";
    যদি( objc!=3 && objc!=4 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "[TYPE] SCRIPT");
      ফেরত TCL_ERROR;
    }

    যদি( pDb->nTransaction==0 && objc==4 ){
      স্থির ধ্রুবক অক্ষর *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      গণনা TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      পূর্ণসংখ্যা ttype;
      যদি( Tcl_Get_সূচক_From_Obj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        ফেরত TCL_ERROR;
      }
      নির্বাচন( (গণনা TTYPE_enum)ttype ){
        ক্ষেত্র TTYPE_DEFERRED:    /* no-op */;                 বিরতি;
        ক্ষেত্র TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  বিরতি;
        ক্ষেত্র TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  বিরতি;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    যদি( rc!=SQLITE_OK ){
      Tcl_Append_ফলাফল(interp, sqlite3_errmsg(pDb->db), (অক্ষর*)0);
      ফেরত TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    যদি( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (শূন্য)Tcl_NREvalObj(interp, pScript, 0);
    }অন্যথা{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    বিরতি;
  }

  /*
  **    $db unlock_notify ?script?
  */
  ক্ষেত্র DB_UNLOCK_NOTIFY: {
#ifndef SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_Append_ফলাফল(interp, "unlock_notify not available in this build",
                     (অক্ষর*)0);
    rc = TCL_ERROR;
#অন্যথা
    যদি( objc!=2 && objc!=3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }অন্যথা{
      শূন্য (*xNotify)(শূন্য **, পূর্ণসংখ্যা) = 0;
      শূন্য *pNotifyArg = 0;

      যদি( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_গুনতি(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      যদি( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (শূন্য *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_গুনতি(pDb->pUnlockNotify);
      }

      যদি( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_Append_ফলাফল(interp, sqlite3_errmsg(pDb->db), (অক্ষর*)0);
        rc = TCL_ERROR;
      }
    }
#endif
    বিরতি;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  ক্ষেত্র DB_PREUPDATE: {
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_Append_ফলাফল(interp, "preupdate_hook was omitted at compile-time", 
                     (অক্ষর*)0);
    rc = TCL_ERROR;
#অন্যথা
    স্থির ধ্রুবক অক্ষর *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    গণনা DbPreupdateSubCmd {
      PRE_গুনতি, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    পূর্ণসংখ্যা iSub;

    যদি( objc<3 ){
      Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    যদি( Tcl_Get_সূচক_From_Obj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      ফেরত TCL_ERROR;
    }

    নির্বাচন( (গণনা DbPreupdateSubCmd)iSub ){
      ক্ষেত্র PRE_গুনতি: {
        পূর্ণসংখ্যা nCol = sqlite3_preupdate_গুনতি(pDb->db);
        Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(nCol));
        বিরতি;
      }

      ক্ষেত্র PRE_HOOK: {
        যদি( objc>4 ){
          Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "hook ?SCRIPT?");
          ফেরত TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        বিরতি;
      }

      ক্ষেত্র PRE_DEPTH: {
        Tcl_Obj *pRet;
        যদি( objc!=3 ){
          Tcl_Wrong_অঙ্ক_Args(interp, 3, objv, "");
          ফেরত TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_Set_Obj_ফলাফল(interp, pRet);
        বিরতি;
      }

      ক্ষেত্র PRE_NEW:
      ক্ষেত্র PRE_OLD: {
        পূর্ণসংখ্যা iIdx;
        sqlite3_মান *p_মান;
        যদি( objc!=4 ){
          Tcl_Wrong_অঙ্ক_Args(interp, 3, objv, "INDEX");
          ফেরত TCL_ERROR;
        }
        যদি( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          ফেরত TCL_ERROR;
        }

        যদি( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_মান);
        }অন্যথা{
          প্রতিজ্ঞা( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_মান);
        }

        যদি( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((অক্ষর*)sqlite3_মান_text(p_মান), -1);
          Tcl_Set_Obj_ফলাফল(interp, pObj);
        }অন্যথা{
          Tcl_Append_ফলাফল(interp, sqlite3_errmsg(pDb->db), (অক্ষর*)0);
          ফেরত TCL_ERROR;
        }
      }
    }
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
    বিরতি;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  ক্ষেত্র DB_WAL_HOOK:
  ক্ষেত্র DB_UPDATE_HOOK:
  ক্ষেত্র DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    যদি( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    যদি( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    যদি( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    যদি( objc>3 ){
       Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?SCRIPT?");
       ফেরত TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    বিরতি;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  ক্ষেত্র DB_VERSION: {
    পূর্ণসংখ্যা i;
    জন্য(i=2; i<objc; i++){
      ধ্রুবক অক্ষর *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      যদি( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        যদি( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          ফেরত TCL_ERROR;
        }
      }অন্যথা

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      যদি( strcmp(zArg, "-last-stmt-ptr")==0 ){
        বাহ্যিক ধ্রুবক অক্ষর *sqlite3TestPtrToText(শূন্য*);
        শূন্য *ptr = (শূন্য*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_Set_ফলাফল(interp, (অক্ষর*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }অন্যথা
#endif /* SQLITE_TEST */
      {
        Tcl_Append_ফলাফল(interp, "unknown argument: ", zArg, (অক্ষর*)0);
        ফেরত TCL_ERROR;
      }
    }
    যদি( i==2 ){   
      Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3_libversion(), TCL_STATIC);
    }
    বিরতি;
  }


  } /* End of the SWITCH statement */
  ফেরত rc;
}

#যদি SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI DbObjCmdAdaptor(
  শূন্য *cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *ধ্রুবক*objv
){
  ফেরত Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
স্থির পূর্ণসংখ্যা sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *ধ্রুবক*objv
){
  Tcl_Wrong_অঙ্ক_Args(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  ফেরত TCL_ERROR;
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI DbMain(
  শূন্য *cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *ধ্রুবক*objv
){
  SqliteDb *p;
  ধ্রুবক অক্ষর *zArg;
  অক্ষর *z_Err_বার্তা;
  পূর্ণসংখ্যা i;
  ধ্রুবক অক্ষর *zFile = 0;
  ধ্রুবক অক্ষর *zVfs = 0;
  পূর্ণসংখ্যা flags;
  পূর্ণসংখ্যা bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  পূর্ণসংখ্যা rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#অন্যথা
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  যদি( objc==1 ) ফেরত sqliteCmdUsage(interp, objv);
  যদি( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    যদি( strcmp(zArg,"-version")==0 ){
      Tcl_Append_ফলাফল(interp,sqlite3_libversion(), (অক্ষর*)0);
      ফেরত TCL_OK;
    }
    যদি( strcmp(zArg,"-sourceid")==0 ){
      Tcl_Append_ফলাফল(interp,sqlite3_sourceid(), (অক্ষর*)0);
      ফেরত TCL_OK;
    }
    যদি( strcmp(zArg,"-has-codec")==0 ){
      Tcl_Append_ফলাফল(interp,"0",(অক্ষর*)0);
      ফেরত TCL_OK;
    }
    যদি( zArg[0]=='-' ) ফেরত sqliteCmdUsage(interp, objv);
  }
  জন্য(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    যদি( zArg[0]!='-' ){
      যদি( zFile!=0 ) ফেরত sqliteCmdUsage(interp, objv);
      zFile = zArg;
      চালু;
    }
    যদি( i==objc-1 ) ফেরত sqliteCmdUsage(interp, objv);
    i++;
    যদি( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }অন্যথা যদি( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }অন্যথা যদি( strcmp(zArg, "-readonly")==0 ){
      পূর্ণসংখ্যা b;
      যদি( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ফেরত TCL_ERROR;
      যদি( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }অন্যথা{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }অন্যথা যদি( strcmp(zArg, "-create")==0 ){
      পূর্ণসংখ্যা b;
      যদি( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ফেরত TCL_ERROR;
      যদি( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }অন্যথা{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }অন্যথা যদি( strcmp(zArg, "-nofollow")==0 ){
      পূর্ণসংখ্যা b;
      যদি( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ফেরত TCL_ERROR;
      যদি( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }অন্যথা{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }অন্যথা যদি( strcmp(zArg, "-nomutex")==0 ){
      পূর্ণসংখ্যা b;
      যদি( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ফেরত TCL_ERROR;
      যদি( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }অন্যথা{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }অন্যথা যদি( strcmp(zArg, "-fullmutex")==0 ){
      পূর্ণসংখ্যা b;
      যদি( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ফেরত TCL_ERROR;
      যদি( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }অন্যথা{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }অন্যথা যদি( strcmp(zArg, "-uri")==0 ){
      পূর্ণসংখ্যা b;
      যদি( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ফেরত TCL_ERROR;
      যদি( b ){
        flags |= SQLITE_OPEN_URI;
      }অন্যথা{
        flags &= ~SQLITE_OPEN_URI;
      }
    }অন্যথা যদি( strcmp(zArg, "-translatefilename")==0 ){
      যদি( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        ফেরত TCL_ERROR;
      }
    }অন্যথা{
      Tcl_Append_ফলাফল(interp, "unknown option: ", zArg, (অক্ষর*)0);
      ফেরত TCL_ERROR;
    }
  }
  z_Err_বার্তা = 0;
  p = (SqliteDb*)Tcl_Alloc( আকার(*p) );
  memset(p, 0, আকার(*p));
  যদি( zFile==0 ) zFile = "";
  যদি( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  যদি( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  যদি( p->db ){
    যদি( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_বার্তা = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }অন্যথা{
    z_Err_বার্তা = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  যদি( p->db==0 ){
    Tcl_Set_ফলাফল(interp, z_Err_বার্তা, TCL_VOLATILE);
    Tcl_Free((অক্ষর*)p);
    sqlite3_free(z_Err_বার্তা);
    ফেরত TCL_ERROR;
  }
  p->maxStmt = অঙ্ক_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  যদি( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (অক্ষর*)p, DbDeleteCmd);
  }অন্যথা{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (অক্ষর*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  ফেরত TCL_OK;
}

/*
** Provide a dummy Tcl_InitStubs if we are using this as a static
** library.
*/
#ifndef USE_TCL_STUBS
# undef  Tcl_InitStubs
# define Tcl_InitStubs(a,b,c) TCL_VERSION
#endif

/*
** Make sure we have a PACKAGE_VERSION macro defined.  This will be
** defined automatically by the TEA makefile.  But other makefiles
** do not define it.
*/
#ifndef PACKAGE_VERSION
# define PACKAGE_VERSION SQLITE_VERSION
#endif

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
EXTERN পূর্ণসংখ্যা Sqlite3_Init(Tcl_Interp *interp){
  পূর্ণসংখ্যা rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  যদি( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  ফেরত rc;
}
EXTERN পূর্ণসংখ্যা Tclsqlite3_Init(Tcl_Interp *interp){ ফেরত Sqlite3_Init(interp); }
EXTERN পূর্ণসংখ্যা Sqlite3_Unload(Tcl_Interp *interp, পূর্ণসংখ্যা flags){ ফেরত TCL_OK; }
EXTERN পূর্ণসংখ্যা Tclsqlite3_Unload(Tcl_Interp *interp, পূর্ণসংখ্যা flags){ ফেরত TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN পূর্ণসংখ্যা Sqlite3_SafeInit(Tcl_Interp *interp){ ফেরত TCL_ERROR; }
EXTERN পূর্ণসংখ্যা Sqlite3_SafeUnload(Tcl_Interp *interp, পূর্ণসংখ্যা flags){ফেরত TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN পূর্ণসংখ্যা Sqlite_Init(Tcl_Interp *interp){ ফেরত Sqlite3_Init(interp);}
EXTERN পূর্ণসংখ্যা Tclsqlite_Init(Tcl_Interp *interp){ ফেরত Sqlite3_Init(interp); }
EXTERN পূর্ণসংখ্যা Sqlite_Unload(Tcl_Interp *interp, পূর্ণসংখ্যা flags){ ফেরত TCL_OK; }
EXTERN পূর্ণসংখ্যা Tclsqlite_Unload(Tcl_Interp *interp, পূর্ণসংখ্যা flags){ ফেরত TCL_OK; }
EXTERN পূর্ণসংখ্যা Sqlite_SafeInit(Tcl_Interp *interp){ ফেরত TCL_ERROR; }
EXTERN পূর্ণসংখ্যা Sqlite_SafeUnload(Tcl_Interp *interp, পূর্ণসংখ্যা flags){ফেরত TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN পূর্ণসংখ্যা sqlite3_Init(Tcl_Interp *interp){ ফেরত Sqlite3_Init(interp);}
EXTERN পূর্ণসংখ্যা sqlite_Init(Tcl_Interp *interp){ ফেরত Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#যদি defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
স্থির ধ্রুবক অক্ষর *tclsh_main_loop(শূন্য){
  স্থির ধ্রুবক অক্ষর zMainloop[] =
    "if {[llength $argv]>=1} {\n"
#ifdef WIN32
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
#endif
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
  ফেরত zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN প্রধান
#endif
পূর্ণসংখ্যা SQLITE_CDECL TCLSH_MAIN(পূর্ণসংখ্যা argc, অক্ষর **argv){
  Tcl_Interp *interp;
  পূর্ণসংখ্যা i;
  ধ্রুবক অক্ষর *zScript = 0;
  অক্ষর zArgc[32];
#যদি defined(TCLSH_INIT_PROC)
  বাহ্যিক ধ্রুবক অক্ষর *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#যদি !defined(_WIN32_WCE)
  যদি( getenv("SQLITE_DEBUG_BREAK") ){
    যদি( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }অন্যথা{
#যদি defined(_WIN32) || defined(WIN32)
      DebugBreak();
#নতুবা_যদি defined(SIGTRAP)
      তোলো(SIGTRAP);
#endif
    }
  }
#endif

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_SetSystemEncoding(ফাঁকা, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(আকার(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  জন্য(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_মান);
  }
#যদি defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  যদি( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  যদি( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    ধ্রুবক অক্ষর *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    যদি( zInfo==0 ) zInfo = Tcl_Get_String_ফলাফল(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    ফেরত 1;
  }
  ফেরত 0;
}
#endif /* TCLSH */
