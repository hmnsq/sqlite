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
#නම් defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#නම් defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#නැතහොත්
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#නම් TCL_MAJOR_VERSION==9
# define CONST const
#elif !defined(Tcl_Size)
  typedef පූර්ණ_සංඛ්‍යා Tcl_Size;
# ifndef Tcl_BounceRefCount
#  define Tcl_BounceRefCount(X) Tcl_IncrRefCount(X); Tcl_DecrRefCount(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#endif
/**** End copy of tclsqlite.h ****/

#include <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ifndef SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  typedef unsigned var_na u8;
# ifndef SQLITE_PTRSIZE
#   නම් defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   නැතහොත්
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# නම් defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    typedef uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    typedef unsigned පූර්ණ_සංඛ්‍යා uptr;
# නැතහොත්
    typedef sqlite3_uint64 uptr;
# endif
#endif
#include <ctype.h>

/* Used to get the current process ID */
#නම් !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#elif !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (පූර්ණ_සංඛ්‍යා)GetCurrentProcessId
#endif

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ifdef BUILD_sqlite
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_sqlite */

#define NUM_PREPARED_STMTS 10
#define MAX_PREPARED_STMTS 100

/* Forward declaration */
typedef sa_nracanaa SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#නම් defined(SQLITE_ENABLE_QRF_IN_TCL)
#include "qrf.h"
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
typedef sa_nracanaa SqlFunc SqlFunc;
sa_nracanaa SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  පූර්ණ_සංඛ්‍යා useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  පූර්ණ_සංඛ්‍යා eType;            /* Type of value to return */
  var_na *zName;          /* Name of this function */
  SqlFunc *pNext;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
typedef sa_nracanaa SqlCollate SqlCollate;
sa_nracanaa SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  var_na *zScript;        /* The script to be run */
  SqlCollate *pNext;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
typedef sa_nracanaa SqlPreparedStmt SqlPreparedStmt;
sa_nracanaa SqlPreparedStmt {
  SqlPreparedStmt *pNext;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  පූර්ණ_සංඛ්‍යා nSql;                /* chars in zSql[] */
  const var_na *zSql;        /* Text of the SQL statement */
  පූර්ණ_සංඛ්‍යා nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

typedef sa_nracanaa IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
sa_nracanaa SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  var_na *zBusy;               /* The busy callback routine */
  var_na *zCommit;             /* The commit hook callback routine */
  var_na *zTrace;              /* The trace callback routine */
  var_na *zTraceV2;            /* The trace_v2 callback routine */
  var_na *zProfile;            /* The profile callback routine */
  var_na *zProgress;           /* The progress callback routine */
  var_na *zBindFallback;       /* Callback to invoke on a binding miss */
  var_na *zAuth;               /* The authorization callback routine */
  පූර්ණ_සංඛ්‍යා disableAuth;           /* Disable the authorizer if it exists */
  var_na *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  පූර්ණ_සංඛ්‍යා rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  පූර්ණ_සංඛ්‍යා maxStmt;               /* The next maximum number of stmtList */
  පූර්ණ_සංඛ්‍යා nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  පූර්ණ_සංඛ්‍යා nStep, nSort, nIndex;  /* Statistics for most recent operation */
  පූර්ණ_සංඛ්‍යා nVMStep;               /* Another statistic for most recent operation */
  පූර්ණ_සංඛ්‍යා nTransaction;          /* Number of nested [transaction] methods */
  පූර්ණ_සංඛ්‍යා openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  පූර්ණ_සංඛ්‍යා nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  පූර්ණ_සංඛ්‍යා bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

sa_nracanaa IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned පූර්ණ_සංඛ්‍යා isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *pNext;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
static පූර්ණ_සංඛ්‍යා strlen30(const var_na *z){
  const var_na *z2 = z;
  අතරතුර( *z2 ){ z2++; }
  ආපසු 0x3fffffff & (පූර්ණ_සංඛ්‍යා)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
static void closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *pNext;

  සඳහා(p=pDb->pIncrblob; p; p=pNext){
    pNext = p->pNext;

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
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI incrblobClose2(
  ClientData instanceData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා flags
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  පූර්ණ_සංඛ්‍යා  rc;
  sqlite3 *db = p->pDb->db;

  නම්( flags ){
    p->isClosed |= flags;
    ආපසු TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  නම්( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  නම්( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }
  නම්( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->pNext;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((var_na *)p);

  නම්( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (var_na *)sqlite3_errmsg(db), TCL_VOLATILE);
    ආපසු TCL_ERROR;
  }
  ආපසු TCL_OK;
}
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI incrblobClose(
  ClientData instanceData,
  Tcl_Interp *interp
){
  ආපසු incrblobClose2(instanceData, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI incrblobInput(
  ClientData instanceData,
  var_na *buf,
  පූර්ණ_සංඛ්‍යා bufSize,
  පූර්ණ_සංඛ්‍යා *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nRead = bufSize;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  පූර්ණ_සංඛ්‍යා rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  නම්( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  නම්( nRead<=0 ){
    ආපසු 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (void *)buf, (පූර්ණ_සංඛ්‍යා)nRead, (පූර්ණ_සංඛ්‍යා)p->iSeek);
  නම්( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    ආපසු -1;
  }

  p->iSeek += nRead;
  ආපසු nRead;
}

/*
** Write data to an incremental blob channel.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI incrblobOutput(
  ClientData instanceData,
  const var_na *buf,
  පූර්ණ_සංඛ්‍යා toWrite,
  පූර්ණ_සංඛ්‍යා *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  පූර්ණ_සංඛ්‍යා rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  නම්( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    ආපසු -1;
  }
  නම්( nWrite<=0 ){
    ආපසු 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (void*)buf,(පූර්ණ_සංඛ්‍යා)nWrite, (පූර්ණ_සංඛ්‍යා)p->iSeek);
  නම්( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    ආපසු -1;
  }

  p->iSeek += nWrite;
  ආපසු nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#නම් TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#නැතහොත්
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
static WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  ClientData instanceData,
  WideSeekProcType offset,
  පූර්ණ_සංඛ්‍යා seekMode,
  පූර්ණ_සංඛ්‍යා *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;

  switch( seekMode ){
    pak_sa SEEK_SET:
      p->iSeek = offset;
      bha_nga;
    pak_sa SEEK_CUR:
      p->iSeek += offset;
      bha_nga;
    pak_sa SEEK_END:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      bha_nga;

    default: assert(!"Bad seekMode");
  }

  ආපසු p->iSeek;
}
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI incrblobSeek(
  ClientData instanceData,
  long offset,
  පූර්ණ_සංඛ්‍යා seekMode,
  පූර්ණ_සංඛ්‍යා *errorCodePtr
){
  ආපසු incrblobWideSeek(instanceData,offset,seekMode,errorCodePtr);
}


static void SQLITE_TCLAPI incrblobWatch(
  ClientData instanceData,
  පූර්ණ_සංඛ්‍යා mode
){
  /* NO-OP */
}
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI incrblobHandle(
  ClientData instanceData,
  පූර්ණ_සංඛ්‍යා dir,
  ClientData *hPtr
){
  ආපසු TCL_ERROR;
}

static Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_VERSION_5,             /* version                              */
  incrblobClose,                     /* closeProc                            */
  incrblobInput,                     /* inputProc                            */
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
static පූර්ණ_සංඛ්‍යා createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  const var_na *zDb,
  const var_na *zTable,
  const var_na *zColumn,
  sqlite_int64 iRow,
  පූර්ණ_සංඛ්‍යා isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  පූර්ණ_සංඛ්‍යා rc;
  පූර්ණ_සංඛ්‍යා flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  static පූර්ණ_සංඛ්‍යා count = 0;
  var_na zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  නම්( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (var_na *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    ආපසු TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(parimaa_na(IncrblobChannel));
  memset(p, 0, parimaa_na(*p));
  p->pBlob = pBlob;
  නම්( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(parimaa_na(zChannel), zChannel, "incrblob_%d", ++count);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->pNext = pDb->pIncrblob;
  p->pPrev = 0;
  නම්( p->pNext ){
    p->pNext->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_SetResult(interp, (var_na *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  ආපසු TCL_OK;
}
#නැතහොත්  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
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
static පූර්ණ_සංඛ්‍යා safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  const var_na *z;
  Tcl_Size n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  අතරතුර( n-- > 0 ){
    පූර්ණ_සංඛ්‍යා c = *(z++);
    නම්( c=='$' || c=='[' || c==';' ) ආපසු 0;
  }
  ආපසු 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
static SqlFunc *findSqlFunc(SqliteDb *pDb, const var_na *zName){
  SqlFunc *p, *pNew;
  පූර්ණ_සංඛ්‍යා nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( parimaa_na(*pNew) + nName + 1 );
  pNew->zName = (var_na*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  සඳහා(p=pDb->pFunc; p; p=p->pNext){
    නම්( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((var_na*)pNew);
      ආපසු p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->pNext = pDb->pFunc;
  pDb->pFunc = pNew;
  ආපසු pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
static void dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  නම්( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((var_na *)pStmt->zSql);
  }
#endif
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((var_na *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
static void flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *pNext;

  සඳහා(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pNext){
    pNext = pPreStmt->pNext;
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
static void addDatabaseRef(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
static void delDatabaseRef(SqliteDb *pDb){
  assert( pDb->nRef>0 );
  pDb->nRef--;
  නම්( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    අතරතුර( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->pNext;
      assert( pFunc->pDb==pDb );
      Tcl_DecrRefCount(pFunc->pScript);
      Tcl_Free((var_na*)pFunc);
    }
    අතරතුර( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->pNext;
      Tcl_Free((var_na*)pCollate);
    }
    නම්( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    නම්( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    නම්( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    නම්( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    නම්( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    නම්( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    නම්( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    නම්( pDb->pUpdateHook ){
      Tcl_DecrRefCount(pDb->pUpdateHook);
    }
    නම්( pDb->pPreUpdateHook ){
      Tcl_DecrRefCount(pDb->pPreUpdateHook);
    }
    නම්( pDb->pRollbackHook ){
      Tcl_DecrRefCount(pDb->pRollbackHook);
    }
    නම්( pDb->pWalHook ){
      Tcl_DecrRefCount(pDb->pWalHook);
    }
    නම්( pDb->pCollateNeeded ){
      Tcl_DecrRefCount(pDb->pCollateNeeded);
    }
    Tcl_Free((var_na*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
static void SQLITE_TCLAPI DbDeleteCmd(void *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
static පූර්ණ_සංඛ්‍යා DbBusyHandler(void *cd, පූර්ණ_සංඛ්‍යා nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  පූර්ණ_සංඛ්‍යා rc;
  var_na zVal[30];

  sqlite3_snprintf(parimaa_na(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (var_na*)0);
  නම්( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ආපසු 0;
  }
  ආපසු 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
static පූර්ණ_සංඛ්‍යා DbProgressHandler(void *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  පූර්ණ_සංඛ්‍යා rc;

  assert( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  නම්( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ආපසු 1;
  }
  ආපසු 0;
}
#endif

#නම් !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
static void DbTraceHandler(void *cd, const var_na *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DStringValue(&str));
  Tcl_DStringFree(&str);
  Tcl_ResetResult(pDb->interp);
}
#endif

#ifndef SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
static පූර්ණ_සංඛ්‍යා DbTraceV2Handler(
  unsigned type, /* One of the SQLITE_TRACE_* event types. */
  void *cd,      /* The original context data pointer. */
  void *pd,      /* Primary event data, depends on event type. */
  void *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  switch( type ){
    pak_sa SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      var_na *zSql = (var_na *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_IncrRefCount(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(pCmd);
      Tcl_ResetResult(pDb->interp);
      bha_nga;
    }
    pak_sa SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_IncrRefCount(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(pCmd);
      Tcl_ResetResult(pDb->interp);
      bha_nga;
    }
    pak_sa SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_IncrRefCount(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(pCmd);
      Tcl_ResetResult(pDb->interp);
      bha_nga;
    }
    pak_sa SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_IncrRefCount(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(pCmd);
      Tcl_ResetResult(pDb->interp);
      bha_nga;
    }
  }
  ආපසු SQLITE_OK;
}
#endif

#නම් !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
static void DbProfileHandler(void *cd, const var_na *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  var_na zTm[100];

  sqlite3_snprintf(parimaa_na(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DStringValue(&str));
  Tcl_DStringFree(&str);
  Tcl_ResetResult(pDb->interp);
}
#endif

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
static පූර්ණ_සංඛ්‍යා DbCommitHandler(void *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  පූර්ණ_සංඛ්‍යා rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  නම්( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ආපසු 1;
  }
  ආපසු 0;
}

static void DbRollbackHandler(void *clientData){
  SqliteDb *pDb = (SqliteDb*)clientData;
  assert(pDb->pRollbackHook);
  නම්( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
static පූර්ණ_සංඛ්‍යා DbWalHandler(
  void *clientData,
  sqlite3 *db,
  const var_na *zDb,
  පූර්ණ_සංඛ්‍යා nEntry
){
  පූර්ණ_සංඛ්‍යා ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)clientData;
  Tcl_Interp *interp = pDb->interp;
  assert(pDb->pWalHook);

  assert( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_IncrRefCount(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  නම්( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_DecrRefCount(p);

  ආපසු ret;
}

#නම් defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
static void setTestUnlockNotifyVars(Tcl_Interp *interp, පූර්ණ_සංඛ්‍යා iArg, පූර්ණ_සංඛ්‍යා nArg){
  var_na zBuf[64];
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#නැතහොත්
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
static void DbUnlockNotify(void **apArg, පූර්ණ_සංඛ්‍යා nArg){
  පූර්ණ_සංඛ්‍යා i;
  සඳහා(i=0; i<nArg; i++){
    const පූර්ණ_සංඛ්‍යා flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    setTestUnlockNotifyVars(pDb->interp, i, nArg);
    assert( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_DecrRefCount(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
static void DbPreUpdateHandler(
  void *p,
  sqlite3 *db,
  පූර්ණ_සංඛ්‍යා op,
  const var_na *zDb,
  const var_na *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  static const var_na *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  assert( (SQLITE_DELETE-1)/9 == 0 );
  assert( (SQLITE_INSERT-1)/9 == 1 );
  assert( (SQLITE_UPDATE-1)/9 == 2 );
  assert( pDb->pPreUpdateHook );
  assert( db==pDb->db );
  assert( op==SQLITE_INSERT || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_IncrRefCount(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_DecrRefCount(pCmd);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

static void DbUpdateHandler(
  void *p,
  පූර්ණ_සංඛ්‍යා op,
  const var_na *zDb,
  const var_na *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  static const var_na *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  assert( (SQLITE_DELETE-1)/9 == 0 );
  assert( (SQLITE_INSERT-1)/9 == 1 );
  assert( (SQLITE_UPDATE-1)/9 == 2 );

  assert( pDb->pUpdateHook );
  assert( op==SQLITE_INSERT || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_IncrRefCount(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_DecrRefCount(pCmd);
}

static void tclCollateNeeded(
  void *pCtx,
  sqlite3 *db,
  පූර්ණ_සංඛ්‍යා enc,
  const var_na *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_IncrRefCount(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_DecrRefCount(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
static පූර්ණ_සංඛ්‍යා tclSqlCollate(
  void *pCtx,
  පූර්ණ_සංඛ්‍යා nA,
  const void *zA,
  පූර්ණ_සංඛ්‍යා nB,
  const void *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_IncrRefCount(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_DecrRefCount(pCmd);
  ආපසු (atoi(Tcl_GetStringResult(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
static void tclSqlFunc(sqlite3_context *context, පූර්ණ_සංඛ්‍යා argc, sqlite3_value**argv){
  SqlFunc *p = sqlite3_user_data(context);
  Tcl_Obj *pCmd;
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා rc;

  නම්( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_IncrRefCount(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_DecrRefCount(pCmd);
  }නැතහොත්{
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
    Tcl_Size nArg;
    නම්( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
      ආපසු;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_IncrRefCount(pCmd);
    සඳහා(i=0; i<argc; i++){
      sqlite3_value *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      switch( sqlite3_value_type(pIn) ){
        pak_sa SQLITE_BLOB: {
          පූර්ණ_සංඛ්‍යා bytes = sqlite3_value_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_value_blob(pIn), bytes);
          bha_nga;
        }
        pak_sa SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_value_int64(pIn);
          නම්( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((පූර්ණ_සංඛ්‍යා)v);
          }නැතහොත්{
            pVal = Tcl_NewWideIntObj(v);
          }
          bha_nga;
        }
        pak_sa SQLITE_FLOAT: {
          dviguu_na r = sqlite3_value_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          bha_nga;
        }
        pak_sa SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          bha_nga;
        }
        default: {
          පූර්ණ_සංඛ්‍යා bytes = sqlite3_value_bytes(pIn);
          pVal = Tcl_NewStringObj((var_na *)sqlite3_value_text(pIn), bytes);
          bha_nga;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      නම්( rc ){
        Tcl_DecrRefCount(pCmd);
        sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
        ආපසු;
      }
    }
    නම්( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_DecrRefCount(pCmd);
  }

  නම්( TCL_BREAK==rc ){
    sqlite3_result_null(context);
  }නැතහොත් නම්( rc && rc!=TCL_RETURN ){
    sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
  }නැතහොත්{
    Tcl_Obj *pVar = Tcl_GetObjResult(p->interp);
    Tcl_Size n;
    u8 *data;
    const var_na *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    var_na c = zType[0];
    පූර්ණ_සංඛ්‍යා eType = p->eType;

    නම්( eType==SQLITE_NULL ){
      නම්( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }නැතහොත් නම්( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }නැතහොත් නම්( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }නැතහොත්{
        eType = SQLITE_TEXT;
      }
    }

    switch( eType ){
      pak_sa SQLITE_BLOB: {
        data = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_result_blob64(context, data, n, SQLITE_TRANSIENT);
        bha_nga;
      }
      pak_sa SQLITE_INTEGER: {
        Tcl_WideInt v;
        නම්( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_result_int64(context, v);
          bha_nga;
        }
        /* fall-through */
      }
      pak_sa SQLITE_FLOAT: {
        dviguu_na r;
        නම්( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_result_double(context, r);
          bha_nga;
        }
        /* fall-through */
      }
      default: {
        data = (unsigned var_na *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_result_text64(context, (var_na *)data, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        bha_nga;
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
static පූර්ණ_සංඛ්‍යා auth_callback(
  void *pArg,
  පූර්ණ_සංඛ්‍යා code,
  const var_na *zArg1,
  const var_na *zArg2,
  const var_na *zArg3,
  const var_na *zArg4
){
  const var_na *zCode;
  Tcl_DString str;
  පූර්ණ_සංඛ්‍යා rc;
  const var_na *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  නම්( pDb->disableAuth ) ආපසු SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  switch( code ){
    pak_sa SQLITE_COPY              : zCode="SQLITE_COPY"; bha_nga;
    pak_sa SQLITE_CREATE_INDEX      : zCode="SQLITE_CREATE_INDEX"; bha_nga;
    pak_sa SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; bha_nga;
    pak_sa SQLITE_CREATE_TEMP_INDEX : zCode="SQLITE_CREATE_TEMP_INDEX"; bha_nga;
    pak_sa SQLITE_CREATE_TEMP_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; bha_nga;
    pak_sa SQLITE_CREATE_TEMP_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; bha_nga;
    pak_sa SQLITE_CREATE_TEMP_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; bha_nga;
    pak_sa SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; bha_nga;
    pak_sa SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; bha_nga;
    pak_sa SQLITE_DELETE            : zCode="SQLITE_DELETE"; bha_nga;
    pak_sa SQLITE_DROP_INDEX        : zCode="SQLITE_DROP_INDEX"; bha_nga;
    pak_sa SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; bha_nga;
    pak_sa SQLITE_DROP_TEMP_INDEX   : zCode="SQLITE_DROP_TEMP_INDEX"; bha_nga;
    pak_sa SQLITE_DROP_TEMP_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; bha_nga;
    pak_sa SQLITE_DROP_TEMP_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; bha_nga;
    pak_sa SQLITE_DROP_TEMP_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; bha_nga;
    pak_sa SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; bha_nga;
    pak_sa SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; bha_nga;
    pak_sa SQLITE_INSERT            : zCode="SQLITE_INSERT"; bha_nga;
    pak_sa SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; bha_nga;
    pak_sa SQLITE_READ              : zCode="SQLITE_READ"; bha_nga;
    pak_sa SQLITE_SELECT            : zCode="SQLITE_SELECT"; bha_nga;
    pak_sa SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; bha_nga;
    pak_sa SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; bha_nga;
    pak_sa SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; bha_nga;
    pak_sa SQLITE_DETACH            : zCode="SQLITE_DETACH"; bha_nga;
    pak_sa SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; bha_nga;
    pak_sa SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; bha_nga;
    pak_sa SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; bha_nga;
    pak_sa SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; bha_nga;
    pak_sa SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; bha_nga;
    pak_sa SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; bha_nga;
    pak_sa SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; bha_nga;
    pak_sa SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; bha_nga;
    default                       : zCode="????"; bha_nga;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, zCode);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DStringValue(&str));
  Tcl_DStringFree(&str);
  zReply = rc==TCL_OK ? Tcl_GetStringResult(pDb->interp) : "SQLITE_DENY";
  නම්( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }නැතහොත් නම්( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }නැතහොත් නම්( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }නැතහොත්{
    rc = 999;
  }
  ආපසු rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#නම් 0
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
static var_na *local_getline(var_na *zPrompt, FILE *in){
  var_na *zLine;
  පූර්ණ_සංඛ්‍යා nLine;
  පූර්ණ_සංඛ්‍යා n;

  nLine = 100;
  zLine = malloc( nLine );
  නම්( zLine==0 ) ආපසු 0;
  n = 0;
  අතරතුර( 1 ){
    නම්( n+100>nLine ){
      nLine = nLine*2 + 100;
      zLine = realloc(zLine, nLine);
      නම්( zLine==0 ) ආපසු 0;
    }
    නම්( fgets(&zLine[n], nLine - n, in)==0 ){
      නම්( n==0 ){
        free(zLine);
        ආපසු 0;
      }
      zLine[n] = 0;
      bha_nga;
    }
    අතරතුර( zLine[n] ){ n++; }
    නම්( n>0 && zLine[n-1]=='\n' ){
      n--;
      zLine[n] = 0;
      bha_nga;
    }
  }
  zLine = realloc( zLine, n+1 );
  ආපසු zLine;
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
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI DbTransPostCmd(
  ClientData data[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  පූර්ණ_සංඛ්‍යා result                           /* Result of evaluating SCRIPT */
){
  static const var_na *const azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)data[0];
  පූර්ණ_සංඛ්‍යා rc = result;
  const var_na *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  නම්( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    නම්( rc!=TCL_ERROR ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (var_na*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  ආපසු rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
static පූර්ණ_සංඛ්‍යා dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  const var_na *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  const var_na **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned පූර්ණ_සංඛ්‍යා prepFlags = 0;
#ifdef SQLITE_TEST
  නම්( pDb->bLegacyPrepare ){
    ආපසු sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  නම්( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  ආපසු sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
static පූර්ණ_සංඛ්‍යා dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  var_na const *zIn,                /* SQL to compile */
  var_na const **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  const var_na *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  පූර්ණ_සංඛ්‍යා nSql;                       /* Length of zSql in bytes */
  පූර්ණ_සංඛ්‍යා nVar = 0;                   /* Number of variables in statement */
  පූර්ණ_සංඛ්‍යා iParm = 0;                  /* Next free entry in apParm */
  var_na c;
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා needResultReset = 0;        /* Need to invoke Tcl_ResetResult() */
  පූර්ණ_සංඛ්‍යා rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  අතරතුර( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  සඳහා(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->pNext){
    පූර්ණ_සංඛ්‍යා n = pPreStmt->nSql;
    නම්( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      නම්( pPreStmt->pPrev ){
        pPreStmt->pPrev->pNext = pPreStmt->pNext;
      }නැතහොත්{
        pDb->stmtList = pPreStmt->pNext;
      }
      නම්( pPreStmt->pNext ){
        pPreStmt->pNext->pPrev = pPreStmt->pPrev;
      }නැතහොත්{
        pDb->stmtLast = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_count(pStmt);
      bha_nga;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  නම්( pPreStmt==0 ){
    පූර්ණ_සංඛ්‍යා nByte;

    නම්( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      ආපසු TCL_ERROR;
    }
    නම්( pStmt==0 ){
      නම්( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ආපසු TCL_ERROR;
      }නැතහොත්{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        ආපසු TCL_OK;
      }
    }

    assert( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_count(pStmt);
    nByte = parimaa_na(SqlPreparedStmt) + nVar*parimaa_na(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (පූර්ණ_සංඛ්‍යා)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    නම්( pPreStmt->zSql==0 ){
      var_na *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      memcpy(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#endif
  }
  assert( pPreStmt );
  assert( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  assert( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  සඳහා(i=1; i<=nVar; i++){
    const var_na *zVar = sqlite3_bind_parameter_name(pStmt, i);
    නම්( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      නම්( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        පූර්ණ_සංඛ්‍යා rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_IncrRefCount(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        නම්( needResultReset ) Tcl_ResetResult(interp);
        needResultReset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_DecrRefCount(pCmd);
        නම්( rx==TCL_OK ){
          pVar = Tcl_GetObjResult(interp);
        }නැතහොත් නම්( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          bha_nga;
        }නැතහොත්{
          pVar = 0;
        }
      }
      නම්( pVar ){
        Tcl_Size n;
        u8 *data;
        const var_na *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        නම්( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          data = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, data, n, SQLITE_STATIC);
          Tcl_IncrRefCount(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }නැතහොත් නම්( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          පූර්ණ_සංඛ්‍යා nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }නැතහොත් නම්( c=='d' && strcmp(zType,"double")==0 ){
          dviguu_na r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }නැතහොත් නම්( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }නැතහොත්{
          data = (unsigned var_na *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (var_na *)data, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_IncrRefCount(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }නැතහොත්{
        sqlite3_bind_null(pStmt, i);
      }
      නම්( needResultReset ) Tcl_ResetResult(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  නම්( needResultReset && rc==TCL_OK ) Tcl_ResetResult(pDb->interp);

  ආපසු rc;
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
static void dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  පූර්ණ_සංඛ්‍යා discard                     /* True to delete (not cache) the pPreStmt */
){
  පූර්ණ_සංඛ්‍යා i;

  /* Free the bound string and blob parameters */
  සඳහා(i=0; i<pPreStmt->nParm; i++){
    Tcl_DecrRefCount(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  නම්( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }නැතහොත්{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->pNext = pDb->stmtList;
    pPreStmt->pPrev = 0;
    නම්( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    නම්( pDb->stmtLast==0 ){
      assert( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }නැතහොත්{
      assert( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    අතරතුර( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *pLast = pDb->stmtLast;
      pDb->stmtLast = pLast->pPrev;
      pDb->stmtLast->pNext = 0;
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
typedef sa_nracanaa DbEvalContext DbEvalContext;
sa_nracanaa DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  const var_na *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  පූර්ණ_සංඛ්‍යා nCol;                       /* Number of columns returned by pStmt */
  පූර්ණ_සංඛ්‍යා evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#define SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#define SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
static void dbReleaseColumnNames(DbEvalContext *p){
  නම්( p->apColName ){
    පූර්ණ_සංඛ්‍යා i;
    සඳහා(i=0; i<p->nCol; i++){
      Tcl_DecrRefCount(p->apColName[i]);
    }
    Tcl_Free((var_na *)p->apColName);
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
static void dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  පූර්ණ_සංඛ්‍යා evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, parimaa_na(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_IncrRefCount(pSql);
  නම්( pVarName ){
    p->pVarName = pVarName;
    Tcl_IncrRefCount(pVarName);
  }
  p->evalFlags = evalFlags;
  addDatabaseRef(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
static void dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  පූර්ණ_සංඛ්‍යා *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  නම්( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    පූර්ණ_සංඛ්‍යා i;                        /* Iterator variable */
    පූර්ණ_සංඛ්‍යා nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_count(pStmt);
    නම්( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( parimaa_na(Tcl_Obj*)*nCol );
      සඳහා(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_IncrRefCount(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    නම්( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_IncrRefCount(pColList);
      Tcl_IncrRefCount(pStar);
      සඳහා(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      නම්( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }නැතහොත්{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, NULL, 0);
        නම්( !pDict ){
          pDict = Tcl_NewDictObj();
        }නැතහොත් නම්( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        නම්( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, NULL, pDict, 0);
        }
        Tcl_BounceRefCount(pDict);
      }
      Tcl_DecrRefCount(pStar);
      Tcl_DecrRefCount(pColList);
    }
  }

  නම්( papColName ){
    *papColName = p->apColName;
  }
  නම්( pnCol ){
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
static පූර්ණ_සංඛ්‍යා dbEvalStep(DbEvalContext *p){
  const var_na *zPrevSql = 0;       /* Previous value of p->zSql */

  අතරතුර( p->zSql[0] || p->pPreStmt ){
    පූර්ණ_සංඛ්‍යා rc;
    නම්( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      නම්( rc!=TCL_OK ) ආපසු rc;
    }නැතහොත්{
      පූර්ණ_සංඛ්‍යා rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      නම්( rcs==SQLITE_ROW ){
        ආපසු TCL_OK;
      }
      නම්( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->nIndex = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      නම්( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#නම් SQLITE_TEST
        නම්( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          continue;
        }
#endif
        Tcl_SetObjResult(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ආපසු TCL_ERROR;
      }නැතහොත්{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  ආපසු TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
static void dbEvalFinalize(DbEvalContext *p){
  නම්( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  නම්( p->pVarName ){
    Tcl_DecrRefCount(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_DecrRefCount(p->pSql);
  dbReleaseColumnNames(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
static Tcl_Obj *dbEvalColumnValue(DbEvalContext *p, පූර්ණ_සංඛ්‍යා iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  switch( sqlite3_column_type(pStmt, iCol) ){
    pak_sa SQLITE_BLOB: {
      පූර්ණ_සංඛ්‍යා bytes = sqlite3_column_bytes(pStmt, iCol);
      const var_na *zBlob = sqlite3_column_blob(pStmt, iCol);
      නම්( !zBlob ) bytes = 0;
      ආපසු Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    pak_sa SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      නම්( v>=-2147483647 && v<=2147483647 ){
        ආපසු Tcl_NewIntObj((පූර්ණ_සංඛ්‍යා)v);
      }නැතහොත්{
        ආපසු Tcl_NewWideIntObj(v);
      }
    }
    pak_sa SQLITE_FLOAT: {
      ආපසු Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    pak_sa SQLITE_NULL: {
      ආපසු Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  ආපසු Tcl_NewStringObj((var_na*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#නම් TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
static පූර්ණ_සංඛ්‍යා DbUseNre(void){
  පූර්ණ_සංඛ්‍යා major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  ආපසු( (major==8 && minor>=6) || major>8 );
}
#නැතහොත්
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (void)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (void)0
#endif

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI DbEvalNextCmd(
  ClientData data[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  පූර්ණ_සංඛ්‍යා result                           /* Result so far */
){
  පූර්ණ_සංඛ්‍යා rc = result;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)data[0];
  Tcl_Obj * const pScript = (Tcl_Obj *)data[1];
  Tcl_Obj * const pVarName = p->pVarName;

  අතරතුර( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    පූර්ණ_සංඛ්‍යා i;
    පූර්ණ_සංඛ්‍යා nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    සඳහා(i=0; i<nCol; i++){
      නම්( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, dbEvalColumnValue(p,i), 0);
      }නැතහොත් නම්( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        නම්( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }නැතහොත්{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, NULL, 0);
          නම්( pDict ){
            නම්( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            නම්( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, NULL, pDict, 0);
            }
            Tcl_BounceRefCount(pDict);
          }
        }
      }නැතහොත් නම්( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       dbEvalColumnValue(p,i), 0);
      }නැතහොත්{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, NULL, 0);
        නම්( !pDict ){
          pDict = Tcl_NewDictObj();
        }නැතහොත් නම්( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        නම්( Tcl_DictObjPut(interp, pDict, apColName[i],
                           dbEvalColumnValue(p,i))==TCL_OK ){
          Tcl_ObjSetVar2(interp, pVarName, NULL, pDict, 0);
        }
        Tcl_BounceRefCount(pDict);
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
    නම්( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbEvalNextCmd, (void*)p, (void*)pScript, 0, 0);
      ආපසු Tcl_NREvalObj(interp, pScript, 0);
    }නැතහොත්{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_DecrRefCount(pScript);
  dbEvalFinalize(p);
  Tcl_Free((var_na *)p);

  නම්( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_ResetResult(interp);
    rc = TCL_OK;
  }
  ආපසු rc;
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
static void DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  නම්( *ppHook ){
    Tcl_SetObjResult(interp, *ppHook);
    නම්( pArg ){
      Tcl_DecrRefCount(*ppHook);
      *ppHook = 0;
    }
  }
  නම්( pArg ){
    assert( !(*ppHook) );
    නම්( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_IncrRefCount(*ppHook);
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
static පූර්ණ_සංඛ්‍යා dbQrf(SqliteDb *pDb, පූර්ණ_සංඛ්‍යා objc, Tcl_Obj *const*objv){
#ifndef SQLITE_QRF_H
  Tcl_SetResult(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  ආපසු TCL_ERROR;
#නැතහොත්
  var_na *zResult = 0;             /* Result to be returned */
  const var_na *zSql = 0;          /* SQL to run */
  පූර්ණ_සංඛ්‍යා i;                         /* Loop counter */
  පූර්ණ_සංඛ්‍යා rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  static const var_na *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  static const unsigned var_na aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_Left,
    QRF_ALIGN_Middle, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_Right,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_Top,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, parimaa_na(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &zResult;
  සඳහා(i=2; i<objc; i++){
    const var_na *zArg = Tcl_GetString(objv[i]);
    const var_na *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    const unsigned var_na aBoolMap[] = { 0, 2, 1, 2, 1 };
    නම්( zArg[0]!='-' ){
      නම්( zSql ){
        Tcl_AppendResult(pDb->interp, "unknown argument: ", zArg, (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      zSql  = zArg;
    }නැතහොත් නම්( i==objc-1 ){
      Tcl_AppendResult(pDb->interp, "option has no argument: ", zArg,(var_na*)0);
      rc = TCL_ERROR;
      goto format_failed;
    }නැතහොත් නම්( strcmp(zArg,"-style")==0 ){
      static const var_na *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      static unsigned var_na aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_Count,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_Insert,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_Line,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      පූර්ණ_සංඛ්‍යා style;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      නම්( rc ) goto format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-esc")==0 ){
      static const var_na *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      static unsigned var_na aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_Symbol
      };
      පූර්ණ_සංඛ්‍යා esc;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      නම්( rc ) goto format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      static const var_na *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      static unsigned var_na aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      පූර්ණ_සංඛ්‍යා txt;
      පූර්ණ_සංඛ්‍යා k = zArg[2]=='e';
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      නම්( rc ) goto format_failed;
      නම්( k ){
        qrf.eText = aTextMap[txt];
      }නැතහොත් නම්( txt<=2 ){
        assert( QRF_No==1 );
        assert( QRF_Yes==2 );
        assert( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }නැතහොත්{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-blob")==0 ){
      static const var_na *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      static unsigned var_na aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_Size
      };
      පූර්ණ_සංඛ්‍යා blob;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      නම්( rc ) goto format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-wordwrap")==0 ){
      පූර්ණ_සංඛ්‍යා v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      නම්( rc ) goto format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      පූර්ණ_සංඛ්‍යා v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      නම්( rc ) goto format_failed;
      නම්( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }නැතහොත් නම්( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }නැතහොත් නම්( zArg[1]=='r' ){
        qrf.bRowCount = aBoolMap[v];
      }නැතහොත්{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      පූර්ණ_සංඛ්‍යා ax = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      නම්( rc ) goto format_failed;
      නම්( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }නැතහොත්{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      පූර්ණ_සංඛ්‍යා v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      නම්( rc ) goto format_failed;
      නම්( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }නැතහොත් නම්( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      නම්( zArg[1]=='w' ){
        qrf.nWrap = v;
      }නැතහොත් නම්( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }නැතහොත් නම්( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }නැතහොත්{
        qrf.nLineLimit = v;
      }
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-charlimit")==0 ){
      පූර්ණ_සංඛ්‍යා v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      නම්( rc ) goto format_failed;
      නම්( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-multiinsert")==0 ){
      පූර්ණ_සංඛ්‍යා v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      නම්( rc ) goto format_failed;
      නම්( v<0 ) v = 0;
      qrf.nMultiInsert = v;
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-align")==0 ){
      Tcl_Size n = 0;
      පූර්ණ_සංඛ්‍යා jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      නම්( rc ) goto format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*parimaa_na(qrf.aAlign[0]) );
      නම්( qrf.aAlign==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*parimaa_na(qrf.aAlign[0]));
      qrf.nAlign = n;
      සඳහා(jj=0; jj<n; jj++){
        පූර්ණ_සංඛ්‍යා x;
        Tcl_Obj *pTerm;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        නම්( rc ) goto format_failed;
        rc = Tcl_GetIndexFromObj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        නම්( rc ) goto format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-widths")==0 ){
      Tcl_Size n = 0;
      පූර්ණ_සංඛ්‍යා jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      නම්( rc ) goto format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*parimaa_na(qrf.aWidth[0]) );
      නම්( qrf.aWidth==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*parimaa_na(qrf.aWidth[0]));
      qrf.nWidth = n;
      සඳහා(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        පූර්ණ_සංඛ්‍යා v;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        නම්( rc ) goto format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        නම්( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }නැතහොත් නම්( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short පූර්ණ_සංඛ්‍යා)v;
      }
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-ifmt")==0 ){
      var_na *zFmt = Tcl_GetString(objv[i+1]);
      නම්( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }නැතහොත් නම්( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_AppendResult(pDb->interp, "bad format for -ifmt", (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-fpfmt")==0 ){
      var_na *zFmt = Tcl_GetString(objv[i+1]);
      නම්( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }නැතහොත් නම්( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_AppendResult(pDb->interp, "bad format for -fpfmt", (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }නැතහොත් නම්( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }නැතහොත්{
      Tcl_AppendResult(pDb->interp, "unknown option: ", zArg, (var_na*)0);
      rc = TCL_ERROR;
      goto format_failed;
    }
  }
  අතරතුර( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    var_na *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    නම්( rc ) goto format_failed;
    නම්( pStmt==0 ) continue;
    rc = sqlite3_format_query_result(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    නම්( rc ){
      Tcl_SetResult(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      goto format_failed;
    }
  }
  Tcl_SetResult(pDb->interp, zResult, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.aWidth);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(zResult);
  ආපසු rc;

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
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI DbObjCmd(
  void *cd,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *const*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  පූර්ණ_සංඛ්‍යා choice;
  පූර්ණ_සංඛ්‍යා rc = TCL_OK;
  static const var_na *DB_strs[] = {
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
  ga_nanaa DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_LOAD_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_INTERRUPT,            DB_LAST_INSERT_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_PROGRESS,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_STATUS,                DB_TIMEOUT,              DB_TOTAL_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_VERSION,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  නම්( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    ආපසු TCL_ERROR;
  }
  නම්( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    ආපසු TCL_ERROR;
  }

  switch( (ga_nanaa DB_enum)choice ){

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
  pak_sa DB_AUTHORIZER: {
#ifdef SQLITE_OMIT_AUTHORIZATION
    Tcl_AppendResult(interp, "authorization not available in this build",
                     (var_na*)0);
    ආපසු TCL_ERROR;
#නැතහොත්
    නම්( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ආපසු TCL_ERROR;
    }නැතහොත් නම්( objc==2 ){
      නම්( pDb->zAuth ){
        Tcl_AppendResult(interp, pDb->zAuth, (var_na*)0);
      }
    }නැතහොත්{
      var_na *zAuth;
      Tcl_Size len;
      නම්( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      නම්( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }නැතහොත්{
        pDb->zAuth = 0;
      }
      නම්( pDb->zAuth ){
        typedef පූර්ණ_සංඛ්‍යා (*sqlite3_auth_cb)(
           void*,පූර්ණ_සංඛ්‍යා,const var_na*,const var_na*,
           const var_na*,const var_na*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }නැතහොත්{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#endif
    bha_nga;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  pak_sa DB_BACKUP: {
    const var_na *zDestFile;
    const var_na *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    නම්( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }නැතහොත් නම්( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }නැතහොත්{
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? FILENAME");
      ආපසු TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    නම්( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (var_na*)0);
      sqlite3_close(pDest);
      ආපසු TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    නම්( pBackup==0 ){
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (var_na*)0);
      sqlite3_close(pDest);
      ආපසු TCL_ERROR;
    }
    අතරතුර(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    නම්( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }නැතහොත්{
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (var_na*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    bha_nga;
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
  pak_sa DB_BIND_FALLBACK: {
    නම්( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ආපසු TCL_ERROR;
    }නැතහොත් නම්( objc==2 ){
      නම්( pDb->zBindFallback ){
        Tcl_AppendResult(interp, pDb->zBindFallback, (var_na*)0);
      }
    }නැතහොත්{
      var_na *zCallback;
      Tcl_Size len;
      නම්( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      නම්( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }නැතහොත්{
        pDb->zBindFallback = 0;
      }
    }
    bha_nga;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  pak_sa DB_BUSY: {
    නම්( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "CALLBACK");
      ආපසු TCL_ERROR;
    }නැතහොත් නම්( objc==2 ){
      නම්( pDb->zBusy ){
        Tcl_AppendResult(interp, pDb->zBusy, (var_na*)0);
      }
    }නැතහොත්{
      var_na *zBusy;
      Tcl_Size len;
      නම්( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      නම්( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }නැතහොත්{
        pDb->zBusy = 0;
      }
      නම්( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }නැතහොත්{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    bha_nga;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  pak_sa DB_CACHE: {
    var_na *subCmd;
    පූර්ණ_සංඛ්‍යා n;

    නම්( objc<=2 ){
      Tcl_WrongNumArgs(interp, 1, objv, "cache option ?arg?");
      ආපසු TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    නම්( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      නම්( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "flush");
        ආපසු TCL_ERROR;
      }නැතහොත්{
        flushStmtCache( pDb );
      }
    }නැතහොත් නම්( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      නම්( objc!=4 ){
        Tcl_WrongNumArgs(interp, 2, objv, "size n");
        ආපසු TCL_ERROR;
      }නැතහොත්{
        නම්( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_AppendResult( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (var_na*)0);
          ආපසු TCL_ERROR;
        }නැතහොත්{
          නම්( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }නැතහොත් නම්( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }නැතහොත්{
      Tcl_AppendResult( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (var_na*)0);
      ආපසු TCL_ERROR;
    }
    bha_nga;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  pak_sa DB_CHANGES: {
    Tcl_Obj *pResult;
    නම්( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      ආපසු TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_changes64(pDb->db));
    bha_nga;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  pak_sa DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    bha_nga;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  pak_sa DB_COLLATE: {
    SqlCollate *pCollate;
    var_na *zName;
    var_na *zScript;
    Tcl_Size nScript;
    නම්( objc!=4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NAME SCRIPT");
      ආපසු TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( parimaa_na(*pCollate) + nScript + 1 );
    නම්( pCollate==0 ) ආපසු TCL_ERROR;
    pCollate->interp = interp;
    pCollate->pNext = pDb->pCollate;
    pCollate->zScript = (var_na*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    නම්( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_SetResult(interp, (var_na *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      ආපසු TCL_ERROR;
    }
    bha_nga;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  pak_sa DB_COLLATION_NEEDED: {
    නම්( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SCRIPT");
      ආපසු TCL_ERROR;
    }
    නම්( pDb->pCollateNeeded ){
      Tcl_DecrRefCount(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_IncrRefCount(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    bha_nga;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  pak_sa DB_COMMIT_HOOK: {
    නම්( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ආපසු TCL_ERROR;
    }නැතහොත් නම්( objc==2 ){
      නම්( pDb->zCommit ){
        Tcl_AppendResult(interp, pDb->zCommit, (var_na*)0);
      }
    }නැතහොත්{
      const var_na *zCommit;
      Tcl_Size len;
      නම්( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      නම්( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }නැතහොත්{
        pDb->zCommit = 0;
      }
      නම්( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }නැතහොත්{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    bha_nga;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  pak_sa DB_COMPLETE: {
#ifndef SQLITE_OMIT_COMPLETE
    Tcl_Obj *pResult;
    පූර්ණ_සංඛ්‍යා isComplete;
    නම්( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SQL");
      ආපසු TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetBooleanObj(pResult, isComplete);
#endif
    bha_nga;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  pak_sa DB_CONFIG: {
    static const sa_nracanaa DbConfigChoices {
      const var_na *zName;
      පූර්ණ_සංඛ්‍යා op;
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
    Tcl_Obj *pResult;
    පූර්ණ_සංඛ්‍යා ii;
    නම්( objc>4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      ආපසු TCL_ERROR;
    }
    නම්( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      pResult = Tcl_NewListObj(0,0);
      සඳහා(ii=0; ii<parimaa_na(aDbConfig)/parimaa_na(aDbConfig[0]); ii++){
        පූර්ණ_සංඛ්‍යා v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewIntObj(v));
      }
    }නැතහොත්{
      const var_na *zOpt = Tcl_GetString(objv[2]);
      පූර්ණ_සංඛ්‍යා onoff = -1;
      පූර්ණ_සංඛ්‍යා v = 0;
      නම්( zOpt[0]=='-' ) zOpt++;
      සඳහා(ii=0; ii<parimaa_na(aDbConfig)/parimaa_na(aDbConfig[0]); ii++){
        නම්( strcmp(aDbConfig[ii].zName, zOpt)==0 ) bha_nga;
      }
      නම්( ii>=parimaa_na(aDbConfig)/parimaa_na(aDbConfig[0]) ){
        Tcl_AppendResult(interp, "unknown config option: \"", zOpt,
                                "\"", (void*)0);
        ආපසු TCL_ERROR;
      }
      නම්( objc==4 ){
        නම්( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          ආපසු TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      pResult = Tcl_NewIntObj(v);
    }
    Tcl_SetObjResult(interp, pResult);
    bha_nga;
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
  pak_sa DB_COPY: {
    var_na *zTable;               /* Insert data into this table */
    var_na *zFile;                /* The file from which to extract data */
    var_na *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    පූර්ණ_සංඛ්‍යා nCol;                   /* Number of columns in the table */
    පූර්ණ_සංඛ්‍යා nByte;                  /* Number of bytes in an SQL string */
    පූර්ණ_සංඛ්‍යා i, j;                   /* Loop counters */
    පූර්ණ_සංඛ්‍යා nSep;                   /* Number of bytes in zSep[] */
    පූර්ණ_සංඛ්‍යා nNull;                  /* Number of bytes in zNull[] */
    var_na *zSql;                 /* An SQL statement */
    var_na *zLine;                /* A single line of input from the file */
    var_na **azCol;               /* zLine[] broken up into columns */
    const var_na *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    පූර්ණ_සංඛ්‍යා lineno = 0;             /* Line number of input file */
    var_na zLineNum[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *pResult;           /* interp result */

    const var_na *zSep;
    const var_na *zNull;
    නම්( objc<5 || objc>7 ){
      Tcl_WrongNumArgs(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      ආපසු TCL_ERROR;
    }
    නම්( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }නැතහොත්{
      zSep = "\t";
    }
    නම්( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }නැතහොත්{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    නම්( nSep==0 ){
      Tcl_AppendResult(interp,"Error: non-null separator required for copy",
                       (var_na*)0);
      ආපසු TCL_ERROR;
    }
    නම්(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_AppendResult(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (var_na*)0);
      ආපසු TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    නම්( zSql==0 ){
      Tcl_AppendResult(interp, "Error: no such table: ", zTable, (var_na*)0);
      ආපසු TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    නම්( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (var_na*)0);
      nCol = 0;
    }නැතහොත්{
      nCol = sqlite3_column_count(pStmt);
    }
    sqlite3_finalize(pStmt);
    නම්( nCol==0 ) {
      ආපසු TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    නම්( zSql==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (var_na*)0);
      ආපසු TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    සඳහා(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    free(zSql);
    නම්( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (var_na*)0);
      sqlite3_finalize(pStmt);
      ආපසු TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    නම්( in==0 ){
      sqlite3_finalize(pStmt);
      ආපසු TCL_ERROR;
    }
    Tcl_SetChannelOption(NULL, in, "-translation", "auto");
    azCol = malloc( parimaa_na(azCol[0])*(nCol+1) );
    නම්( azCol==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (var_na*)0);
      Tcl_Close(interp, in);
      ආපසු TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_IncrRefCount(str);
    (void)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    අතරතුර( Tcl_GetsObj(in, str)>=0 ) {
      var_na *z;
      Tcl_Size byteLen;
      lineno++;
      zLine = (var_na *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = zLine;
      සඳහා(i=0, z=zLine; *z; z++){
        නම්( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          නම්( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      නම්( i+1!=nCol ){
        var_na *zErr;
        පූර්ණ_සංඛ්‍යා nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        නම්( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_AppendResult(interp, zErr, (var_na*)0);
          free(zErr);
        }
        zCommit = "ROLLBACK";
        bha_nga;
      }
      සඳහා(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        නම්( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }නැතහොත්{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_SetObjLength(str, 0);
      නම්( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp,"Error: ", sqlite3_errmsg(pDb->db), (var_na*)0);
        zCommit = "ROLLBACK";
        bha_nga;
      }
    }
    Tcl_DecrRefCount(str);
    free(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (void)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    නම්( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      pResult = Tcl_GetObjResult(interp);
      නම්( Tcl_IsShared(pResult) ){
        pResult = Tcl_DuplicateObj(pResult);
      }
      Tcl_SetIntObj(pResult, lineno);
      rc = TCL_OK;
    }නැතහොත්{
      /* failure, append lineno where failed */
      sqlite3_snprintf(parimaa_na(zLineNum), zLineNum,"%d",lineno);
      Tcl_AppendResult(interp,", failed while processing line: ",zLineNum,
                       (var_na*)0);
      rc = TCL_ERROR;
    }
    bha_nga;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  pak_sa DB_DESERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (var_na*)0);
    rc = TCL_ERROR;
#නැතහොත්
    const var_na *zSchema = 0;
    Tcl_Obj *pValue = 0;
    unsigned var_na *pBA;
    unsigned var_na *pData;
    Tcl_Size len;
    පූර්ණ_සංඛ්‍යා xrc;
    sqlite3_int64 mxSize = 0;
    පූර්ණ_සංඛ්‍යා i;
    පූර්ණ_සංඛ්‍යා isReadonly = 0;


    නම්( objc<3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      bha_nga;
    }
    සඳහා(i=2; i<objc-1; i++){
      const var_na *z = Tcl_GetString(objv[i]);
      නම්( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        නම්( rc ) goto deserialize_error;
        mxSize = x;
        continue;
      }
      නම්( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        නම්( rc ) goto deserialize_error;
        continue;
      }
      නම්( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        continue;
      }
      Tcl_AppendResult(interp, "unknown option: ", z, (var_na*)0);
      rc = TCL_ERROR;
      goto deserialize_error;
    }
    pValue = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(pValue, &len);
    pData = sqlite3_malloc64( len );
    නම්( pData==0 && len>0 ){
      Tcl_AppendResult(interp, "out of memory", (var_na*)0);
      rc = TCL_ERROR;
    }නැතහොත්{
      පූර්ණ_සංඛ්‍යා flags;
      නම්( len>0 ) memcpy(pData, pBA, len);
      නම්( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }නැතහොත්{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, pData, len, len, flags);
      නම්( xrc ){
        Tcl_AppendResult(interp, "unable to set MEMDB content", (var_na*)0);
        rc = TCL_ERROR;
      }
      නම්( mxSize>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_SIZE_LIMIT,&mxSize);
      }
    }
deserialize_error:
#endif
    bha_nga; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  pak_sa DB_ENABLE_LOAD_EXTENSION: {
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    පූර්ණ_සංඛ්‍යා onoff;
    නම්( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "BOOLEAN");
      ආපසු TCL_ERROR;
    }
    නම්( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      ආපසු TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    bha_nga;
#නැතහොත්
    Tcl_AppendResult(interp, "extension loading is turned off at compile-time",
                     (var_na*)0);
    ආපසු TCL_ERROR;
#endif
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  pak_sa DB_ERRORCODE: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    bha_nga;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  pak_sa DB_ERROROFFSET: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    bha_nga;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  pak_sa DB_EXISTS:
  pak_sa DB_ONECOLUMN: {
    Tcl_Obj *pResult = 0;
    DbEvalContext sEval;
    නම්( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SQL");
      ආපසු TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    නම්( choice==DB_ONECOLUMN ){
      නම්( rc==TCL_OK ){
        pResult = dbEvalColumnValue(&sEval, 0);
      }නැතහොත් නම්( rc==TCL_BREAK ){
        Tcl_ResetResult(interp);
      }
    }නැතහොත් නම්( rc==TCL_BREAK || rc==TCL_OK ){
      pResult = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    නම්( pResult ) Tcl_SetObjResult(interp, pResult);

    නම්( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    bha_nga;
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
  pak_sa DB_EVAL: {
    පූර්ණ_සංඛ්‍යා evalFlags = 0;
    const var_na *zOpt;
    අතරතුර( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      නම්( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }නැතහොත් නම්( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }නැතහොත්{
        Tcl_AppendResult(interp, "unknown option: \"", zOpt, "\"", (void*)0);
        ආපසු TCL_ERROR;
      }
      objc--;
      objv++;
    }
    නම්( objc<3 || objc>5 ){
      Tcl_WrongNumArgs(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      ආපසු TCL_ERROR;
    }

    නම්( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_IncrRefCount(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      අතරතුර( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        පූර්ණ_සංඛ්‍යා i;
        පූර්ණ_සංඛ්‍යා nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        සඳහා(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, dbEvalColumnValue(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      නම්( rc==TCL_BREAK ){
        Tcl_SetObjResult(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_DecrRefCount(pRet);
    }නැතහොත්{
      ClientData cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      නම්( objc>=5 && *(var_na *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_IncrRefCount(pScript);

      p = (DbEvalContext *)Tcl_Alloc(parimaa_na(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (void *)p;
      cd2[1] = (void *)pScript;
      rc = DbEvalNextCmd(cd2, interp, TCL_OK);
    }
    bha_nga;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  pak_sa DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    bha_nga;
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
  pak_sa DB_FUNCTION: {
    පූර්ණ_සංඛ්‍යා flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    var_na *zName;
    පූර්ණ_සංඛ්‍යා nArg = -1;
    පූර්ණ_සංඛ්‍යා i;
    පූර්ණ_සංඛ්‍යා eType = SQLITE_NULL;
    නම්( objc<4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      ආපසු TCL_ERROR;
    }
    සඳහා(i=3; i<(objc-1); i++){
      const var_na *z = Tcl_GetString(objv[i]);
      පූර්ණ_සංඛ්‍යා n = strlen30(z);
      නම්( n>1 && strncmp(z, "-argcount",n)==0 ){
        නම්( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(var_na*)0);
          ආපසු TCL_ERROR;
        }
        නම්( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) ආපසු TCL_ERROR;
        නම්( nArg<0 ){
          Tcl_AppendResult(interp, "number of arguments must be non-negative",
                           (var_na*)0);
          ආපසු TCL_ERROR;
        }
        i++;
      }නැතහොත්
      නම්( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }නැතහොත්
      නම්( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }නැතහොත්
      නම්( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }නැතහොත්
      නම්( n>1 && strncmp(z, "-returntype", n)==0 ){
        const var_na *azType[] = {"integer", "real", "text", "blob", "any", 0};
        assert( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        assert( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        නම්( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(var_na*)0);
          ආපසු TCL_ERROR;
        }
        i++;
        නම්( Tcl_GetIndexFromObj(interp, objv[i], azType, "type", 0, &eType) ){
          ආපසු TCL_ERROR;
        }
        eType++;
      }නැතහොත්{
        Tcl_AppendResult(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (var_na*)0
        );
        ආපසු TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    නම්( pFunc==0 ) ආපසු TCL_ERROR;
    නම්( pFunc->pScript ){
      Tcl_DecrRefCount(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_IncrRefCount(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    නම්( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_SetResult(interp, (var_na *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    bha_nga;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  pak_sa DB_INCRBLOB: {
#ifdef SQLITE_OMIT_INCRBLOB
    Tcl_AppendResult(interp, "incrblob not available in this build", (var_na*)0);
    ආපසු TCL_ERROR;
#නැතහොත්
    පූර්ණ_සංඛ්‍යා isReadonly = 0;
    const var_na *zDb = "main";
    const var_na *zTable;
    const var_na *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    නම්( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    නම්( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_WrongNumArgs(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      ආපසු TCL_ERROR;
    }

    නම්( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    නම්( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#endif
    bha_nga;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  pak_sa DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    bha_nga;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  pak_sa DB_NULLVALUE: {
    නම්( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NULLVALUE");
      ආපසු TCL_ERROR;
    }
    නම්( objc==3 ){
      Tcl_Size len;
      var_na *zNull = Tcl_GetStringFromObj(objv[2], &len);
      නම්( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      නම්( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }නැතහොත්{
        pDb->zNull = 0;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(pDb->zNull, -1));
    bha_nga;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  pak_sa DB_LAST_INSERT_ROWID: {
    Tcl_Obj *pResult;
    Tcl_WideInt rowid;
    නම්( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      ආපසු TCL_ERROR;
    }
    rowid = sqlite3_last_insert_rowid(pDb->db);
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, rowid);
    bha_nga;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  pak_sa DB_PROGRESS: {
    නම්( objc==2 ){
      නම්( pDb->zProgress ){
        Tcl_AppendResult(interp, pDb->zProgress, (var_na*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }නැතහොත් නම්( objc==4 ){
      var_na *zProgress;
      Tcl_Size len;
      පූර්ණ_සංඛ්‍යා N;
      නම්( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        ආපසු TCL_ERROR;
      };
      නම්( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      නම්( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }නැතහොත්{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      නම්( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }නැතහොත්{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }නැතහොත්{
      Tcl_WrongNumArgs(interp, 2, objv, "N CALLBACK");
      ආපසු TCL_ERROR;
    }
    bha_nga;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  pak_sa DB_PROFILE: {
    නම්( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ආපසු TCL_ERROR;
    }නැතහොත් නම්( objc==2 ){
      නම්( pDb->zProfile ){
        Tcl_AppendResult(interp, pDb->zProfile, (var_na*)0);
      }
    }නැතහොත්{
      var_na *zProfile;
      Tcl_Size len;
      නම්( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      නම්( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }නැතහොත්{
        pDb->zProfile = 0;
      }
#නම් !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      නම්( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }නැතහොත්{
        sqlite3_profile(pDb->db, 0, 0);
      }
#endif
    }
    bha_nga;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  pak_sa DB_REKEY: {
    නම්( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "KEY");
      ආපසු TCL_ERROR;
    }
    bha_nga;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  pak_sa DB_RESTORE: {
    const var_na *zSrcFile;
    const var_na *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    පූර්ණ_සංඛ්‍යා nTimeout = 0;

    නම්( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }නැතහොත් නම්( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }නැතහොත්{
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? FILENAME");
      ආපසු TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    නම්( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (var_na*)0);
      sqlite3_close(pSrc);
      ආපසු TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    නම්( pBackup==0 ){
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (var_na*)0);
      sqlite3_close(pSrc);
      ආපසු TCL_ERROR;
    }
    අතරතුර( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      නම්( rc==SQLITE_BUSY ){
        නම්( nTimeout++ >= 3 ) bha_nga;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    නම්( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }නැතහොත් නම්( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_AppendResult(interp, "restore failed: source database busy",
                       (var_na*)0);
      rc = TCL_ERROR;
    }නැතහොත්{
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (var_na*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    bha_nga;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  pak_sa DB_SERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (var_na*)0);
    rc = TCL_ERROR;
#නැතහොත්
    const var_na *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned var_na *pData;
    නම්( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }නැතහොත්{
      පූර්ණ_සංඛ්‍යා needFree;
      pData = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      නම්( pData ){
        needFree = 0;
      }නැතහොත්{
        pData = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(pData,sz));
      නම්( needFree ) sqlite3_free(pData);
    }
#endif
    bha_nga;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  pak_sa DB_STATUS: {
    පූර්ණ_සංඛ්‍යා v;
    const var_na *zOp;
    නම්( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "(step|sort|autoindex)");
      ආපසු TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    නම්( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }නැතහොත් නම්( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }නැතහොත් නම්( strcmp(zOp, "autoindex")==0 ){
      v = pDb->nIndex;
    }නැතහොත් නම්( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }නැතහොත්{
      Tcl_AppendResult(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (var_na*)0);
      ආපසු TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(v));
    bha_nga;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  pak_sa DB_TIMEOUT: {
    පූර්ණ_සංඛ්‍යා ms;
    නම්( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "MILLISECONDS");
      ආපසු TCL_ERROR;
    }
    නම්( Tcl_GetIntFromObj(interp, objv[2], &ms) ) ආපසු TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    bha_nga;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  pak_sa DB_TOTAL_CHANGES: {
    Tcl_Obj *pResult;
    නම්( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      ආපසු TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_total_changes64(pDb->db));
    bha_nga;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  pak_sa DB_TRACE: {
    නම්( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ආපසු TCL_ERROR;
    }නැතහොත් නම්( objc==2 ){
      නම්( pDb->zTrace ){
        Tcl_AppendResult(interp, pDb->zTrace, (var_na*)0);
      }
    }නැතහොත්{
      var_na *zTrace;
      Tcl_Size len;
      නම්( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      නම්( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }නැතහොත්{
        pDb->zTrace = 0;
      }
#නම් !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      නම්( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }නැතහොත්{
        sqlite3_trace(pDb->db, 0, 0);
      }
#endif
    }
    bha_nga;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  pak_sa DB_TRACE_V2: {
    නම්( objc>4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK? ?MASK?");
      ආපසු TCL_ERROR;
    }නැතහොත් නම්( objc==2 ){
      නම්( pDb->zTraceV2 ){
        Tcl_AppendResult(interp, pDb->zTraceV2, (var_na*)0);
      }
    }නැතහොත්{
      var_na *zTraceV2;
      Tcl_Size len;
      Tcl_WideInt wMask = 0;
      නම්( objc==4 ){
        static const var_na *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        ga_nanaa TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_Size i;
        නම්( TCL_OK!=Tcl_ListObjLength(interp, objv[3], &len) ){
          ආපසු TCL_ERROR;
        }
        සඳහා(i=0; i<len; i++){
          Tcl_Obj *pObj;
          පූර්ණ_සංඛ්‍යා ttype;
          නම්( TCL_OK!=Tcl_ListObjIndex(interp, objv[3], i, &pObj) ){
            ආපසු TCL_ERROR;
          }
          නම්( Tcl_GetIndexFromObj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
            Tcl_IncrRefCount(pError);
            නම්( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_DecrRefCount(pError);
              wMask |= wType;
            }නැතහොත්{
              Tcl_SetObjResult(interp, pError);
              Tcl_DecrRefCount(pError);
              ආපසු TCL_ERROR;
            }
          }නැතහොත්{
            switch( (ga_nanaa TTYPE_enum)ttype ){
              pak_sa TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    bha_nga;
              pak_sa TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; bha_nga;
              pak_sa TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     bha_nga;
              pak_sa TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   bha_nga;
            }
          }
        }
      }නැතහොත්{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      නම්( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      නම්( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }නැතහොත්{
        pDb->zTraceV2 = 0;
      }
#නම් !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      නම්( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }නැතහොත්{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#endif
    }
    bha_nga;
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
  pak_sa DB_TRANSACTION: {
    Tcl_Obj *pScript;
    const var_na *zBegin = "SAVEPOINT _tcl_transaction";
    නම්( objc!=3 && objc!=4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "[TYPE] SCRIPT");
      ආපසු TCL_ERROR;
    }

    නම්( pDb->nTransaction==0 && objc==4 ){
      static const var_na *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      ga_nanaa TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      පූර්ණ_සංඛ්‍යා ttype;
      නම්( Tcl_GetIndexFromObj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        ආපසු TCL_ERROR;
      }
      switch( (ga_nanaa TTYPE_enum)ttype ){
        pak_sa TTYPE_DEFERRED:    /* no-op */;                 bha_nga;
        pak_sa TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  bha_nga;
        pak_sa TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  bha_nga;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    නම්( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (var_na*)0);
      ආපසු TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    නම්( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (void)Tcl_NREvalObj(interp, pScript, 0);
    }නැතහොත්{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    bha_nga;
  }

  /*
  **    $db unlock_notify ?script?
  */
  pak_sa DB_UNLOCK_NOTIFY: {
#ifndef SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_AppendResult(interp, "unlock_notify not available in this build",
                     (var_na*)0);
    rc = TCL_ERROR;
#නැතහොත්
    නම්( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }නැතහොත්{
      void (*xNotify)(void **, පූර්ණ_සංඛ්‍යා) = 0;
      void *pNotifyArg = 0;

      නම්( pDb->pUnlockNotify ){
        Tcl_DecrRefCount(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      නම්( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (void *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_IncrRefCount(pDb->pUnlockNotify);
      }

      නම්( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (var_na*)0);
        rc = TCL_ERROR;
      }
    }
#endif
    bha_nga;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  pak_sa DB_PREUPDATE: {
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_AppendResult(interp, "preupdate_hook was omitted at compile-time", 
                     (var_na*)0);
    rc = TCL_ERROR;
#නැතහොත්
    static const var_na *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    ga_nanaa DbPreupdateSubCmd {
      PRE_COUNT, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    පූර්ණ_සංඛ්‍යා iSub;

    නම්( objc<3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    නම්( Tcl_GetIndexFromObj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      ආපසු TCL_ERROR;
    }

    switch( (ga_nanaa DbPreupdateSubCmd)iSub ){
      pak_sa PRE_COUNT: {
        පූර්ණ_සංඛ්‍යා nCol = sqlite3_preupdate_count(pDb->db);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nCol));
        bha_nga;
      }

      pak_sa PRE_HOOK: {
        නම්( objc>4 ){
          Tcl_WrongNumArgs(interp, 2, objv, "hook ?SCRIPT?");
          ආපසු TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        bha_nga;
      }

      pak_sa PRE_DEPTH: {
        Tcl_Obj *pRet;
        නම්( objc!=3 ){
          Tcl_WrongNumArgs(interp, 3, objv, "");
          ආපසු TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_SetObjResult(interp, pRet);
        bha_nga;
      }

      pak_sa PRE_NEW:
      pak_sa PRE_OLD: {
        පූර්ණ_සංඛ්‍යා iIdx;
        sqlite3_value *pValue;
        නම්( objc!=4 ){
          Tcl_WrongNumArgs(interp, 3, objv, "INDEX");
          ආපසු TCL_ERROR;
        }
        නම්( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          ආපසු TCL_ERROR;
        }

        නම්( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &pValue);
        }නැතහොත්{
          assert( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &pValue);
        }

        නම්( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((var_na*)sqlite3_value_text(pValue), -1);
          Tcl_SetObjResult(interp, pObj);
        }නැතහොත්{
          Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (var_na*)0);
          ආපසු TCL_ERROR;
        }
      }
    }
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
    bha_nga;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  pak_sa DB_WAL_HOOK:
  pak_sa DB_UPDATE_HOOK:
  pak_sa DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    නම්( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    නම්( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    නම්( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    නම්( objc>3 ){
       Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
       ආපසු TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    bha_nga;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  pak_sa DB_VERSION: {
    පූර්ණ_සංඛ්‍යා i;
    සඳහා(i=2; i<objc; i++){
      const var_na *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      නම්( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        නම්( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          ආපසු TCL_ERROR;
        }
      }නැතහොත්

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      නම්( strcmp(zArg, "-last-stmt-ptr")==0 ){
        extern const var_na *sqlite3TestPtrToText(void*);
        void *ptr = (void*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_SetResult(interp, (var_na*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }නැතහොත්
#endif /* SQLITE_TEST */
      {
        Tcl_AppendResult(interp, "unknown argument: ", zArg, (var_na*)0);
        ආපසු TCL_ERROR;
      }
    }
    නම්( i==2 ){   
      Tcl_SetResult(interp, (var_na *)sqlite3_libversion(), TCL_STATIC);
    }
    bha_nga;
  }


  } /* End of the SWITCH statement */
  ආපසු rc;
}

#නම් SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI DbObjCmdAdaptor(
  void *cd,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *const*objv
){
  ආපසු Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
static පූර්ණ_සංඛ්‍යා sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *const*objv
){
  Tcl_WrongNumArgs(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  ආපසු TCL_ERROR;
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
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI DbMain(
  void *cd,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *const*objv
){
  SqliteDb *p;
  const var_na *zArg;
  var_na *zErrMsg;
  පූර්ණ_සංඛ්‍යා i;
  const var_na *zFile = 0;
  const var_na *zVfs = 0;
  පූර්ණ_සංඛ්‍යා flags;
  පූර්ණ_සංඛ්‍යා bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  පූර්ණ_සංඛ්‍යා rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#නැතහොත්
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  නම්( objc==1 ) ආපසු sqliteCmdUsage(interp, objv);
  නම්( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    නම්( strcmp(zArg,"-version")==0 ){
      Tcl_AppendResult(interp,sqlite3_libversion(), (var_na*)0);
      ආපසු TCL_OK;
    }
    නම්( strcmp(zArg,"-sourceid")==0 ){
      Tcl_AppendResult(interp,sqlite3_sourceid(), (var_na*)0);
      ආපසු TCL_OK;
    }
    නම්( strcmp(zArg,"-has-codec")==0 ){
      Tcl_AppendResult(interp,"0",(var_na*)0);
      ආපසු TCL_OK;
    }
    නම්( zArg[0]=='-' ) ආපසු sqliteCmdUsage(interp, objv);
  }
  සඳහා(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    නම්( zArg[0]!='-' ){
      නම්( zFile!=0 ) ආපසු sqliteCmdUsage(interp, objv);
      zFile = zArg;
      continue;
    }
    නම්( i==objc-1 ) ආපසු sqliteCmdUsage(interp, objv);
    i++;
    නම්( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }නැතහොත් නම්( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }නැතහොත් නම්( strcmp(zArg, "-readonly")==0 ){
      පූර්ණ_සංඛ්‍යා b;
      නම්( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ආපසු TCL_ERROR;
      නම්( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }නැතහොත්{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }නැතහොත් නම්( strcmp(zArg, "-create")==0 ){
      පූර්ණ_සංඛ්‍යා b;
      නම්( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ආපසු TCL_ERROR;
      නම්( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }නැතහොත්{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }නැතහොත් නම්( strcmp(zArg, "-nofollow")==0 ){
      පූර්ණ_සංඛ්‍යා b;
      නම්( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ආපසු TCL_ERROR;
      නම්( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }නැතහොත්{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }නැතහොත් නම්( strcmp(zArg, "-nomutex")==0 ){
      පූර්ණ_සංඛ්‍යා b;
      නම්( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ආපසු TCL_ERROR;
      නම්( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }නැතහොත්{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }නැතහොත් නම්( strcmp(zArg, "-fullmutex")==0 ){
      පූර්ණ_සංඛ්‍යා b;
      නම්( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ආපසු TCL_ERROR;
      නම්( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }නැතහොත්{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }නැතහොත් නම්( strcmp(zArg, "-uri")==0 ){
      පූර්ණ_සංඛ්‍යා b;
      නම්( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ආපසු TCL_ERROR;
      නම්( b ){
        flags |= SQLITE_OPEN_URI;
      }නැතහොත්{
        flags &= ~SQLITE_OPEN_URI;
      }
    }නැතහොත් නම්( strcmp(zArg, "-translatefilename")==0 ){
      නම්( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        ආපසු TCL_ERROR;
      }
    }නැතහොත්{
      Tcl_AppendResult(interp, "unknown option: ", zArg, (var_na*)0);
      ආපසු TCL_ERROR;
    }
  }
  zErrMsg = 0;
  p = (SqliteDb*)Tcl_Alloc( parimaa_na(*p) );
  memset(p, 0, parimaa_na(*p));
  නම්( zFile==0 ) zFile = "";
  නම්( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  නම්( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  නම්( p->db ){
    නම්( SQLITE_OK!=sqlite3_errcode(p->db) ){
      zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }නැතහොත්{
    zErrMsg = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  නම්( p->db==0 ){
    Tcl_SetResult(interp, zErrMsg, TCL_VOLATILE);
    Tcl_Free((var_na*)p);
    sqlite3_free(zErrMsg);
    ආපසු TCL_ERROR;
  }
  p->maxStmt = NUM_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  නම්( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (var_na*)p, DbDeleteCmd);
  }නැතහොත්{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (var_na*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  ආපසු TCL_OK;
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
EXTERN පූර්ණ_සංඛ්‍යා Sqlite3_Init(Tcl_Interp *interp){
  පූර්ණ_සංඛ්‍යා rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  නම්( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  ආපසු rc;
}
EXTERN පූර්ණ_සංඛ්‍යා Tclsqlite3_Init(Tcl_Interp *interp){ ආපසු Sqlite3_Init(interp); }
EXTERN පූර්ණ_සංඛ්‍යා Sqlite3_Unload(Tcl_Interp *interp, පූර්ණ_සංඛ්‍යා flags){ ආපසු TCL_OK; }
EXTERN පූර්ණ_සංඛ්‍යා Tclsqlite3_Unload(Tcl_Interp *interp, පූර්ණ_සංඛ්‍යා flags){ ආපසු TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN පූර්ණ_සංඛ්‍යා Sqlite3_SafeInit(Tcl_Interp *interp){ ආපසු TCL_ERROR; }
EXTERN පූර්ණ_සංඛ්‍යා Sqlite3_SafeUnload(Tcl_Interp *interp, පූර්ණ_සංඛ්‍යා flags){ආපසු TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN පූර්ණ_සංඛ්‍යා Sqlite_Init(Tcl_Interp *interp){ ආපසු Sqlite3_Init(interp);}
EXTERN පූර්ණ_සංඛ්‍යා Tclsqlite_Init(Tcl_Interp *interp){ ආපසු Sqlite3_Init(interp); }
EXTERN පූර්ණ_සංඛ්‍යා Sqlite_Unload(Tcl_Interp *interp, පූර්ණ_සංඛ්‍යා flags){ ආපසු TCL_OK; }
EXTERN පූර්ණ_සංඛ්‍යා Tclsqlite_Unload(Tcl_Interp *interp, පූර්ණ_සංඛ්‍යා flags){ ආපසු TCL_OK; }
EXTERN පූර්ණ_සංඛ්‍යා Sqlite_SafeInit(Tcl_Interp *interp){ ආපසු TCL_ERROR; }
EXTERN පූර්ණ_සංඛ්‍යා Sqlite_SafeUnload(Tcl_Interp *interp, පූර්ණ_සංඛ්‍යා flags){ආපසු TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN පූර්ණ_සංඛ්‍යා sqlite3_Init(Tcl_Interp *interp){ ආපසු Sqlite3_Init(interp);}
EXTERN පූර්ණ_සංඛ්‍යා sqlite_Init(Tcl_Interp *interp){ ආපසු Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#නම් defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
static const var_na *tclsh_main_loop(void){
  static const var_na zMainloop[] =
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
  ආපසු zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN main
#endif
පූර්ණ_සංඛ්‍යා SQLITE_CDECL TCLSH_MAIN(පූර්ණ_සංඛ්‍යා argc, var_na **argv){
  Tcl_Interp *interp;
  පූර්ණ_සංඛ්‍යා i;
  const var_na *zScript = 0;
  var_na zArgc[32];
#නම් defined(TCLSH_INIT_PROC)
  extern const var_na *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#නම් !defined(_WIN32_WCE)
  නම්( getenv("SQLITE_DEBUG_BREAK") ){
    නම්( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }නැතහොත්{
#නම් defined(_WIN32) || defined(WIN32)
      DebugBreak();
#elif defined(SIGTRAP)
      raise(SIGTRAP);
#endif
    }
  }
#endif

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_SetSystemEncoding(NULL, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(parimaa_na(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  සඳහා(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_VALUE);
  }
#නම් defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  නම්( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  නම්( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    const var_na *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    නම්( zInfo==0 ) zInfo = Tcl_GetStringResult(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    ආපසු 1;
  }
  ආපසු 0;
}
#endif /* TCLSH */
