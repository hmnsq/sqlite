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
#nếu defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#nếu defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#không_thì
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#nếu TCL_MAJOR_VERSION==9
# define CONST const
#elif !defined(Tcl_Size)
  typedef số_nguyên Tcl_Size;
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
  typedef unsigned char u8;
# ifndef SQLITE_PTRSIZE
#   nếu defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   không_thì
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# nếu defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    typedef uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    typedef unsigned số_nguyên uptr;
# không_thì
    typedef sqlite3_uint64 uptr;
# endif
#endif
#include <ctype.h>

/* Used to get the current process ID */
#nếu !defined(_WIN32)
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
# define GETPID (số_nguyên)GetCurrentProcessId
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
typedef struct SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#nếu defined(SQLITE_ENABLE_QRF_IN_TCL)
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
typedef struct SqlFunc SqlFunc;
struct SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  số_nguyên useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  số_nguyên eType;            /* Type of value to return */
  char *zName;          /* Name of this function */
  SqlFunc *pNext;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
typedef struct SqlCollate SqlCollate;
struct SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  char *zScript;        /* The script to be run */
  SqlCollate *pNext;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
typedef struct SqlPreparedStmt SqlPreparedStmt;
struct SqlPreparedStmt {
  SqlPreparedStmt *pNext;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  số_nguyên nSql;                /* chars in zSql[] */
  const char *zSql;        /* Text of the SQL statement */
  số_nguyên nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

typedef struct IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
struct SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  char *zBusy;               /* The busy callback routine */
  char *zCommit;             /* The commit hook callback routine */
  char *zTrace;              /* The trace callback routine */
  char *zTraceV2;            /* The trace_v2 callback routine */
  char *zProfile;            /* The profile callback routine */
  char *zProgress;           /* The progress callback routine */
  char *zBindFallback;       /* Callback to invoke on a binding miss */
  char *zAuth;               /* The authorization callback routine */
  số_nguyên disableAuth;           /* Disable the authorizer if it exists */
  char *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  số_nguyên rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  số_nguyên maxStmt;               /* The next maximum number of stmtList */
  số_nguyên nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  số_nguyên nStep, nSort, nIndex;  /* Statistics for most recent operation */
  số_nguyên nVMStep;               /* Another statistic for most recent operation */
  số_nguyên nTransaction;          /* Number of nested [transaction] methods */
  số_nguyên openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  số_nguyên nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  số_nguyên bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

struct IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned số_nguyên isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *pNext;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
static số_nguyên strlen30(const char *z){
  const char *z2 = z;
  trong_khi( *z2 ){ z2++; }
  trả_về 0x3fffffff & (số_nguyên)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
static void closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *pNext;

  cho(p=pDb->pIncrblob; p; p=pNext){
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
static số_nguyên SQLITE_TCLAPI incrblobClose2(
  ClientData instanceData,
  Tcl_Interp *interp,
  số_nguyên flags
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  số_nguyên  rc;
  sqlite3 *db = p->pDb->db;

  nếu( flags ){
    p->isClosed |= flags;
    trả_về TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  nếu( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  nếu( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }
  nếu( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->pNext;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((char *)p);

  nếu( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (char *)sqlite3_errmsg(db), TCL_VOLATILE);
    trả_về TCL_ERROR;
  }
  trả_về TCL_OK;
}
static số_nguyên SQLITE_TCLAPI incrblobClose(
  ClientData instanceData,
  Tcl_Interp *interp
){
  trả_về incrblobClose2(instanceData, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
static số_nguyên SQLITE_TCLAPI incrblobInput(
  ClientData instanceData,
  char *buf,
  số_nguyên bufSize,
  số_nguyên *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nRead = bufSize;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  số_nguyên rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  nếu( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  nếu( nRead<=0 ){
    trả_về 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (void *)buf, (số_nguyên)nRead, (số_nguyên)p->iSeek);
  nếu( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    trả_về -1;
  }

  p->iSeek += nRead;
  trả_về nRead;
}

/*
** Write data to an incremental blob channel.
*/
static số_nguyên SQLITE_TCLAPI incrblobOutput(
  ClientData instanceData,
  const char *buf,
  số_nguyên toWrite,
  số_nguyên *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  số_nguyên rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  nếu( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    trả_về -1;
  }
  nếu( nWrite<=0 ){
    trả_về 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (void*)buf,(số_nguyên)nWrite, (số_nguyên)p->iSeek);
  nếu( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    trả_về -1;
  }

  p->iSeek += nWrite;
  trả_về nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#nếu TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#không_thì
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
static WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  ClientData instanceData,
  WideSeekProcType offset,
  số_nguyên seekMode,
  số_nguyên *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;

  switch( seekMode ){
    case SEEK_SET:
      p->iSeek = offset;
      break;
    case SEEK_CUR:
      p->iSeek += offset;
      break;
    case SEEK_END:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      break;

    default: assert(!"Bad seekMode");
  }

  trả_về p->iSeek;
}
static số_nguyên SQLITE_TCLAPI incrblobSeek(
  ClientData instanceData,
  long offset,
  số_nguyên seekMode,
  số_nguyên *errorCodePtr
){
  trả_về incrblobWideSeek(instanceData,offset,seekMode,errorCodePtr);
}


static void SQLITE_TCLAPI incrblobWatch(
  ClientData instanceData,
  số_nguyên mode
){
  /* NO-OP */
}
static số_nguyên SQLITE_TCLAPI incrblobHandle(
  ClientData instanceData,
  số_nguyên dir,
  ClientData *hPtr
){
  trả_về TCL_ERROR;
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
static số_nguyên createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  const char *zDb,
  const char *zTable,
  const char *zColumn,
  sqlite_int64 iRow,
  số_nguyên isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  số_nguyên rc;
  số_nguyên flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  static số_nguyên count = 0;
  char zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  nếu( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (char *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    trả_về TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(sizeof(IncrblobChannel));
  memset(p, 0, sizeof(*p));
  p->pBlob = pBlob;
  nếu( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(sizeof(zChannel), zChannel, "incrblob_%d", ++count);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->pNext = pDb->pIncrblob;
  p->pPrev = 0;
  nếu( p->pNext ){
    p->pNext->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_SetResult(interp, (char *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  trả_về TCL_OK;
}
#không_thì  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
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
static số_nguyên safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  const char *z;
  Tcl_Size n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  trong_khi( n-- > 0 ){
    số_nguyên c = *(z++);
    nếu( c=='$' || c=='[' || c==';' ) trả_về 0;
  }
  trả_về 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
static SqlFunc *findSqlFunc(SqliteDb *pDb, const char *zName){
  SqlFunc *p, *pNew;
  số_nguyên nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( sizeof(*pNew) + nName + 1 );
  pNew->zName = (char*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  cho(p=pDb->pFunc; p; p=p->pNext){
    nếu( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((char*)pNew);
      trả_về p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->pNext = pDb->pFunc;
  pDb->pFunc = pNew;
  trả_về pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
static void dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  nếu( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((char *)pStmt->zSql);
  }
#endif
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((char *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
static void flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *pNext;

  cho(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pNext){
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
  nếu( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    trong_khi( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->pNext;
      assert( pFunc->pDb==pDb );
      Tcl_DecrRefCount(pFunc->pScript);
      Tcl_Free((char*)pFunc);
    }
    trong_khi( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->pNext;
      Tcl_Free((char*)pCollate);
    }
    nếu( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    nếu( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    nếu( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    nếu( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    nếu( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    nếu( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    nếu( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    nếu( pDb->pUpdateHook ){
      Tcl_DecrRefCount(pDb->pUpdateHook);
    }
    nếu( pDb->pPreUpdateHook ){
      Tcl_DecrRefCount(pDb->pPreUpdateHook);
    }
    nếu( pDb->pRollbackHook ){
      Tcl_DecrRefCount(pDb->pRollbackHook);
    }
    nếu( pDb->pWalHook ){
      Tcl_DecrRefCount(pDb->pWalHook);
    }
    nếu( pDb->pCollateNeeded ){
      Tcl_DecrRefCount(pDb->pCollateNeeded);
    }
    Tcl_Free((char*)pDb);
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
static số_nguyên DbBusyHandler(void *cd, số_nguyên nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  số_nguyên rc;
  char zVal[30];

  sqlite3_snprintf(sizeof(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (char*)0);
  nếu( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    trả_về 0;
  }
  trả_về 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
static số_nguyên DbProgressHandler(void *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  số_nguyên rc;

  assert( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  nếu( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    trả_về 1;
  }
  trả_về 0;
}
#endif

#nếu !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
static void DbTraceHandler(void *cd, const char *zSql){
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
static số_nguyên DbTraceV2Handler(
  unsigned type, /* One of the SQLITE_TRACE_* event types. */
  void *cd,      /* The original context data pointer. */
  void *pd,      /* Primary event data, depends on event type. */
  void *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  switch( type ){
    case SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      char *zSql = (char *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_IncrRefCount(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(pCmd);
      Tcl_ResetResult(pDb->interp);
      break;
    }
    case SQLITE_TRACE_PROFILE: {
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
      break;
    }
    case SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_IncrRefCount(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(pCmd);
      Tcl_ResetResult(pDb->interp);
      break;
    }
    case SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_IncrRefCount(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(pCmd);
      Tcl_ResetResult(pDb->interp);
      break;
    }
  }
  trả_về SQLITE_OK;
}
#endif

#nếu !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
static void DbProfileHandler(void *cd, const char *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  char zTm[100];

  sqlite3_snprintf(sizeof(zTm)-1, zTm, "%lld", tm);
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
static số_nguyên DbCommitHandler(void *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  số_nguyên rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  nếu( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    trả_về 1;
  }
  trả_về 0;
}

static void DbRollbackHandler(void *clientData){
  SqliteDb *pDb = (SqliteDb*)clientData;
  assert(pDb->pRollbackHook);
  nếu( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
static số_nguyên DbWalHandler(
  void *clientData,
  sqlite3 *db,
  const char *zDb,
  số_nguyên nEntry
){
  số_nguyên ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)clientData;
  Tcl_Interp *interp = pDb->interp;
  assert(pDb->pWalHook);

  assert( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_IncrRefCount(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  nếu( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_DecrRefCount(p);

  trả_về ret;
}

#nếu defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
static void setTestUnlockNotifyVars(Tcl_Interp *interp, số_nguyên iArg, số_nguyên nArg){
  char zBuf[64];
  sqlite3_snprintf(sizeof(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(sizeof(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#không_thì
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
static void DbUnlockNotify(void **apArg, số_nguyên nArg){
  số_nguyên i;
  cho(i=0; i<nArg; i++){
    const số_nguyên flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
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
  số_nguyên op,
  const char *zDb,
  const char *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  static const char *azStr[] = {"DELETE", "INSERT", "UPDATE"};

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
  số_nguyên op,
  const char *zDb,
  const char *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  static const char *azStr[] = {"DELETE", "INSERT", "UPDATE"};

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
  số_nguyên enc,
  const char *zName
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
static số_nguyên tclSqlCollate(
  void *pCtx,
  số_nguyên nA,
  const void *zA,
  số_nguyên nB,
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
  trả_về (atoi(Tcl_GetStringResult(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
static void tclSqlFunc(sqlite3_context *context, số_nguyên argc, sqlite3_value**argv){
  SqlFunc *p = sqlite3_user_data(context);
  Tcl_Obj *pCmd;
  số_nguyên i;
  số_nguyên rc;

  nếu( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_IncrRefCount(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_DecrRefCount(pCmd);
  }không_thì{
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
    nếu( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
      trả_về;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_IncrRefCount(pCmd);
    cho(i=0; i<argc; i++){
      sqlite3_value *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      switch( sqlite3_value_type(pIn) ){
        case SQLITE_BLOB: {
          số_nguyên bytes = sqlite3_value_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_value_blob(pIn), bytes);
          break;
        }
        case SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_value_int64(pIn);
          nếu( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((số_nguyên)v);
          }không_thì{
            pVal = Tcl_NewWideIntObj(v);
          }
          break;
        }
        case SQLITE_FLOAT: {
          double r = sqlite3_value_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          break;
        }
        case SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          break;
        }
        default: {
          số_nguyên bytes = sqlite3_value_bytes(pIn);
          pVal = Tcl_NewStringObj((char *)sqlite3_value_text(pIn), bytes);
          break;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      nếu( rc ){
        Tcl_DecrRefCount(pCmd);
        sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
        trả_về;
      }
    }
    nếu( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_DecrRefCount(pCmd);
  }

  nếu( TCL_BREAK==rc ){
    sqlite3_result_null(context);
  }không_thì nếu( rc && rc!=TCL_RETURN ){
    sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
  }không_thì{
    Tcl_Obj *pVar = Tcl_GetObjResult(p->interp);
    Tcl_Size n;
    u8 *data;
    const char *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    char c = zType[0];
    số_nguyên eType = p->eType;

    nếu( eType==SQLITE_NULL ){
      nếu( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }không_thì nếu( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }không_thì nếu( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }không_thì{
        eType = SQLITE_TEXT;
      }
    }

    switch( eType ){
      case SQLITE_BLOB: {
        data = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_result_blob64(context, data, n, SQLITE_TRANSIENT);
        break;
      }
      case SQLITE_INTEGER: {
        Tcl_WideInt v;
        nếu( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_result_int64(context, v);
          break;
        }
        /* fall-through */
      }
      case SQLITE_FLOAT: {
        double r;
        nếu( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_result_double(context, r);
          break;
        }
        /* fall-through */
      }
      default: {
        data = (unsigned char *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_result_text64(context, (char *)data, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        break;
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
static số_nguyên auth_callback(
  void *pArg,
  số_nguyên code,
  const char *zArg1,
  const char *zArg2,
  const char *zArg3,
  const char *zArg4
){
  const char *zCode;
  Tcl_DString str;
  số_nguyên rc;
  const char *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  nếu( pDb->disableAuth ) trả_về SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  switch( code ){
    case SQLITE_COPY              : zCode="SQLITE_COPY"; break;
    case SQLITE_CREATE_INDEX      : zCode="SQLITE_CREATE_INDEX"; break;
    case SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; break;
    case SQLITE_CREATE_TEMP_INDEX : zCode="SQLITE_CREATE_TEMP_INDEX"; break;
    case SQLITE_CREATE_TEMP_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; break;
    case SQLITE_CREATE_TEMP_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; break;
    case SQLITE_CREATE_TEMP_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; break;
    case SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; break;
    case SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; break;
    case SQLITE_DELETE            : zCode="SQLITE_DELETE"; break;
    case SQLITE_DROP_INDEX        : zCode="SQLITE_DROP_INDEX"; break;
    case SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; break;
    case SQLITE_DROP_TEMP_INDEX   : zCode="SQLITE_DROP_TEMP_INDEX"; break;
    case SQLITE_DROP_TEMP_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; break;
    case SQLITE_DROP_TEMP_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; break;
    case SQLITE_DROP_TEMP_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; break;
    case SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; break;
    case SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; break;
    case SQLITE_INSERT            : zCode="SQLITE_INSERT"; break;
    case SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; break;
    case SQLITE_READ              : zCode="SQLITE_READ"; break;
    case SQLITE_SELECT            : zCode="SQLITE_SELECT"; break;
    case SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; break;
    case SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; break;
    case SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; break;
    case SQLITE_DETACH            : zCode="SQLITE_DETACH"; break;
    case SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; break;
    case SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; break;
    case SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; break;
    case SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; break;
    case SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; break;
    case SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; break;
    case SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; break;
    case SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; break;
    default                       : zCode="????"; break;
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
  nếu( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }không_thì nếu( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }không_thì nếu( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }không_thì{
    rc = 999;
  }
  trả_về rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#nếu 0
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
static char *local_getline(char *zPrompt, FILE *in){
  char *zLine;
  số_nguyên nLine;
  số_nguyên n;

  nLine = 100;
  zLine = malloc( nLine );
  nếu( zLine==0 ) trả_về 0;
  n = 0;
  trong_khi( 1 ){
    nếu( n+100>nLine ){
      nLine = nLine*2 + 100;
      zLine = realloc(zLine, nLine);
      nếu( zLine==0 ) trả_về 0;
    }
    nếu( fgets(&zLine[n], nLine - n, in)==0 ){
      nếu( n==0 ){
        free(zLine);
        trả_về 0;
      }
      zLine[n] = 0;
      break;
    }
    trong_khi( zLine[n] ){ n++; }
    nếu( n>0 && zLine[n-1]=='\n' ){
      n--;
      zLine[n] = 0;
      break;
    }
  }
  zLine = realloc( zLine, n+1 );
  trả_về zLine;
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
static số_nguyên SQLITE_TCLAPI DbTransPostCmd(
  ClientData data[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  số_nguyên result                           /* Result of evaluating SCRIPT */
){
  static const char *const azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)data[0];
  số_nguyên rc = result;
  const char *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  nếu( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    nếu( rc!=TCL_ERROR ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (char*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  trả_về rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
static số_nguyên dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  const char *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  const char **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned số_nguyên prepFlags = 0;
#ifdef SQLITE_TEST
  nếu( pDb->bLegacyPrepare ){
    trả_về sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  nếu( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  trả_về sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
static số_nguyên dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  char const *zIn,                /* SQL to compile */
  char const **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  const char *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  số_nguyên nSql;                       /* Length of zSql in bytes */
  số_nguyên nVar = 0;                   /* Number of variables in statement */
  số_nguyên iParm = 0;                  /* Next free entry in apParm */
  char c;
  số_nguyên i;
  số_nguyên needResultReset = 0;        /* Need to invoke Tcl_ResetResult() */
  số_nguyên rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  trong_khi( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  cho(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->pNext){
    số_nguyên n = pPreStmt->nSql;
    nếu( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      nếu( pPreStmt->pPrev ){
        pPreStmt->pPrev->pNext = pPreStmt->pNext;
      }không_thì{
        pDb->stmtList = pPreStmt->pNext;
      }
      nếu( pPreStmt->pNext ){
        pPreStmt->pNext->pPrev = pPreStmt->pPrev;
      }không_thì{
        pDb->stmtLast = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_count(pStmt);
      break;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  nếu( pPreStmt==0 ){
    số_nguyên nByte;

    nếu( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      trả_về TCL_ERROR;
    }
    nếu( pStmt==0 ){
      nếu( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        trả_về TCL_ERROR;
      }không_thì{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        trả_về TCL_OK;
      }
    }

    assert( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_count(pStmt);
    nByte = sizeof(SqlPreparedStmt) + nVar*sizeof(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (số_nguyên)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    nếu( pPreStmt->zSql==0 ){
      char *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
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
  cho(i=1; i<=nVar; i++){
    const char *zVar = sqlite3_bind_parameter_name(pStmt, i);
    nếu( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      nếu( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        số_nguyên rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_IncrRefCount(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        nếu( needResultReset ) Tcl_ResetResult(interp);
        needResultReset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_DecrRefCount(pCmd);
        nếu( rx==TCL_OK ){
          pVar = Tcl_GetObjResult(interp);
        }không_thì nếu( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          break;
        }không_thì{
          pVar = 0;
        }
      }
      nếu( pVar ){
        Tcl_Size n;
        u8 *data;
        const char *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        nếu( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          data = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, data, n, SQLITE_STATIC);
          Tcl_IncrRefCount(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }không_thì nếu( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          số_nguyên nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }không_thì nếu( c=='d' && strcmp(zType,"double")==0 ){
          double r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }không_thì nếu( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }không_thì{
          data = (unsigned char *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (char *)data, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_IncrRefCount(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }không_thì{
        sqlite3_bind_null(pStmt, i);
      }
      nếu( needResultReset ) Tcl_ResetResult(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  nếu( needResultReset && rc==TCL_OK ) Tcl_ResetResult(pDb->interp);

  trả_về rc;
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
  số_nguyên discard                     /* True to delete (not cache) the pPreStmt */
){
  số_nguyên i;

  /* Free the bound string and blob parameters */
  cho(i=0; i<pPreStmt->nParm; i++){
    Tcl_DecrRefCount(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  nếu( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }không_thì{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->pNext = pDb->stmtList;
    pPreStmt->pPrev = 0;
    nếu( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    nếu( pDb->stmtLast==0 ){
      assert( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }không_thì{
      assert( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    trong_khi( pDb->nStmt>pDb->maxStmt ){
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
typedef struct DbEvalContext DbEvalContext;
struct DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  const char *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  số_nguyên nCol;                       /* Number of columns returned by pStmt */
  số_nguyên evalFlags;                  /* Flags used */
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
  nếu( p->apColName ){
    số_nguyên i;
    cho(i=0; i<p->nCol; i++){
      Tcl_DecrRefCount(p->apColName[i]);
    }
    Tcl_Free((char *)p->apColName);
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
  số_nguyên evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, sizeof(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_IncrRefCount(pSql);
  nếu( pVarName ){
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
  số_nguyên *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  nếu( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    số_nguyên i;                        /* Iterator variable */
    số_nguyên nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_count(pStmt);
    nếu( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( sizeof(Tcl_Obj*)*nCol );
      cho(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_IncrRefCount(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    nếu( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_IncrRefCount(pColList);
      Tcl_IncrRefCount(pStar);
      cho(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      nếu( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }không_thì{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, NULL, 0);
        nếu( !pDict ){
          pDict = Tcl_NewDictObj();
        }không_thì nếu( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        nếu( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, NULL, pDict, 0);
        }
        Tcl_BounceRefCount(pDict);
      }
      Tcl_DecrRefCount(pStar);
      Tcl_DecrRefCount(pColList);
    }
  }

  nếu( papColName ){
    *papColName = p->apColName;
  }
  nếu( pnCol ){
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
static số_nguyên dbEvalStep(DbEvalContext *p){
  const char *zPrevSql = 0;       /* Previous value of p->zSql */

  trong_khi( p->zSql[0] || p->pPreStmt ){
    số_nguyên rc;
    nếu( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      nếu( rc!=TCL_OK ) trả_về rc;
    }không_thì{
      số_nguyên rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      nếu( rcs==SQLITE_ROW ){
        trả_về TCL_OK;
      }
      nếu( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->nIndex = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      nếu( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#nếu SQLITE_TEST
        nếu( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
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
        trả_về TCL_ERROR;
      }không_thì{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  trả_về TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
static void dbEvalFinalize(DbEvalContext *p){
  nếu( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  nếu( p->pVarName ){
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
static Tcl_Obj *dbEvalColumnValue(DbEvalContext *p, số_nguyên iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  switch( sqlite3_column_type(pStmt, iCol) ){
    case SQLITE_BLOB: {
      số_nguyên bytes = sqlite3_column_bytes(pStmt, iCol);
      const char *zBlob = sqlite3_column_blob(pStmt, iCol);
      nếu( !zBlob ) bytes = 0;
      trả_về Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    case SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      nếu( v>=-2147483647 && v<=2147483647 ){
        trả_về Tcl_NewIntObj((số_nguyên)v);
      }không_thì{
        trả_về Tcl_NewWideIntObj(v);
      }
    }
    case SQLITE_FLOAT: {
      trả_về Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    case SQLITE_NULL: {
      trả_về Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  trả_về Tcl_NewStringObj((char*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#nếu TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
static số_nguyên DbUseNre(void){
  số_nguyên major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  trả_về( (major==8 && minor>=6) || major>8 );
}
#không_thì
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
static số_nguyên SQLITE_TCLAPI DbEvalNextCmd(
  ClientData data[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  số_nguyên result                           /* Result so far */
){
  số_nguyên rc = result;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)data[0];
  Tcl_Obj * const pScript = (Tcl_Obj *)data[1];
  Tcl_Obj * const pVarName = p->pVarName;

  trong_khi( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    số_nguyên i;
    số_nguyên nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    cho(i=0; i<nCol; i++){
      nếu( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, dbEvalColumnValue(p,i), 0);
      }không_thì nếu( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        nếu( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }không_thì{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, NULL, 0);
          nếu( pDict ){
            nếu( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            nếu( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, NULL, pDict, 0);
            }
            Tcl_BounceRefCount(pDict);
          }
        }
      }không_thì nếu( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       dbEvalColumnValue(p,i), 0);
      }không_thì{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, NULL, 0);
        nếu( !pDict ){
          pDict = Tcl_NewDictObj();
        }không_thì nếu( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        nếu( Tcl_DictObjPut(interp, pDict, apColName[i],
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
    nếu( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbEvalNextCmd, (void*)p, (void*)pScript, 0, 0);
      trả_về Tcl_NREvalObj(interp, pScript, 0);
    }không_thì{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_DecrRefCount(pScript);
  dbEvalFinalize(p);
  Tcl_Free((char *)p);

  nếu( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_ResetResult(interp);
    rc = TCL_OK;
  }
  trả_về rc;
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

  nếu( *ppHook ){
    Tcl_SetObjResult(interp, *ppHook);
    nếu( pArg ){
      Tcl_DecrRefCount(*ppHook);
      *ppHook = 0;
    }
  }
  nếu( pArg ){
    assert( !(*ppHook) );
    nếu( Tcl_GetString(pArg)[0] ){
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
static số_nguyên dbQrf(SqliteDb *pDb, số_nguyên objc, Tcl_Obj *const*objv){
#ifndef SQLITE_QRF_H
  Tcl_SetResult(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  trả_về TCL_ERROR;
#không_thì
  char *zResult = 0;             /* Result to be returned */
  const char *zSql = 0;          /* SQL to run */
  số_nguyên i;                         /* Loop counter */
  số_nguyên rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  static const char *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  static const unsigned char aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_Left,
    QRF_ALIGN_Middle, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_Right,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_Top,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, sizeof(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &zResult;
  cho(i=2; i<objc; i++){
    const char *zArg = Tcl_GetString(objv[i]);
    const char *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    const unsigned char aBoolMap[] = { 0, 2, 1, 2, 1 };
    nếu( zArg[0]!='-' ){
      nếu( zSql ){
        Tcl_AppendResult(pDb->interp, "unknown argument: ", zArg, (char*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      zSql  = zArg;
    }không_thì nếu( i==objc-1 ){
      Tcl_AppendResult(pDb->interp, "option has no argument: ", zArg,(char*)0);
      rc = TCL_ERROR;
      goto format_failed;
    }không_thì nếu( strcmp(zArg,"-style")==0 ){
      static const char *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      static unsigned char aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_Count,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_Insert,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_Line,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      số_nguyên style;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      nếu( rc ) goto format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }không_thì nếu( strcmp(zArg,"-esc")==0 ){
      static const char *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      static unsigned char aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_Symbol
      };
      số_nguyên esc;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      nếu( rc ) goto format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }không_thì nếu( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      static const char *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      static unsigned char aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      số_nguyên txt;
      số_nguyên k = zArg[2]=='e';
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      nếu( rc ) goto format_failed;
      nếu( k ){
        qrf.eText = aTextMap[txt];
      }không_thì nếu( txt<=2 ){
        assert( QRF_No==1 );
        assert( QRF_Yes==2 );
        assert( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }không_thì{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }không_thì nếu( strcmp(zArg,"-blob")==0 ){
      static const char *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      static unsigned char aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_Size
      };
      số_nguyên blob;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      nếu( rc ) goto format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }không_thì nếu( strcmp(zArg,"-wordwrap")==0 ){
      số_nguyên v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      nếu( rc ) goto format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }không_thì nếu( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      số_nguyên v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      nếu( rc ) goto format_failed;
      nếu( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }không_thì nếu( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }không_thì nếu( zArg[1]=='r' ){
        qrf.bRowCount = aBoolMap[v];
      }không_thì{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }không_thì nếu( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      số_nguyên ax = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      nếu( rc ) goto format_failed;
      nếu( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }không_thì{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }không_thì nếu( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      số_nguyên v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      nếu( rc ) goto format_failed;
      nếu( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }không_thì nếu( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      nếu( zArg[1]=='w' ){
        qrf.nWrap = v;
      }không_thì nếu( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }không_thì nếu( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }không_thì{
        qrf.nLineLimit = v;
      }
      i++;
    }không_thì nếu( strcmp(zArg,"-charlimit")==0 ){
      số_nguyên v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      nếu( rc ) goto format_failed;
      nếu( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }không_thì nếu( strcmp(zArg,"-multiinsert")==0 ){
      số_nguyên v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      nếu( rc ) goto format_failed;
      nếu( v<0 ) v = 0;
      qrf.nMultiInsert = v;
      i++;
    }không_thì nếu( strcmp(zArg,"-align")==0 ){
      Tcl_Size n = 0;
      số_nguyên jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      nếu( rc ) goto format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*sizeof(qrf.aAlign[0]) );
      nếu( qrf.aAlign==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (char*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*sizeof(qrf.aAlign[0]));
      qrf.nAlign = n;
      cho(jj=0; jj<n; jj++){
        số_nguyên x;
        Tcl_Obj *pTerm;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        nếu( rc ) goto format_failed;
        rc = Tcl_GetIndexFromObj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        nếu( rc ) goto format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }không_thì nếu( strcmp(zArg,"-widths")==0 ){
      Tcl_Size n = 0;
      số_nguyên jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      nếu( rc ) goto format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*sizeof(qrf.aWidth[0]) );
      nếu( qrf.aWidth==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (char*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*sizeof(qrf.aWidth[0]));
      qrf.nWidth = n;
      cho(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        số_nguyên v;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        nếu( rc ) goto format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        nếu( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }không_thì nếu( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short số_nguyên)v;
      }
      i++;
    }không_thì nếu( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }không_thì nếu( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }không_thì nếu( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }không_thì nếu( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }không_thì nếu( strcmp(zArg,"-ifmt")==0 ){
      char *zFmt = Tcl_GetString(objv[i+1]);
      nếu( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }không_thì nếu( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_AppendResult(pDb->interp, "bad format for -ifmt", (char*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }không_thì nếu( strcmp(zArg,"-fpfmt")==0 ){
      char *zFmt = Tcl_GetString(objv[i+1]);
      nếu( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }không_thì nếu( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_AppendResult(pDb->interp, "bad format for -fpfmt", (char*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }không_thì nếu( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }không_thì{
      Tcl_AppendResult(pDb->interp, "unknown option: ", zArg, (char*)0);
      rc = TCL_ERROR;
      goto format_failed;
    }
  }
  trong_khi( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    char *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    nếu( rc ) goto format_failed;
    nếu( pStmt==0 ) continue;
    rc = sqlite3_format_query_result(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    nếu( rc ){
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
  trả_về rc;

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
static số_nguyên SQLITE_TCLAPI DbObjCmd(
  void *cd,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *const*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  số_nguyên choice;
  số_nguyên rc = TCL_OK;
  static const char *DB_strs[] = {
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
  enum DB_enum {
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

  nếu( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    trả_về TCL_ERROR;
  }

  switch( (enum DB_enum)choice ){

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
  case DB_AUTHORIZER: {
#ifdef SQLITE_OMIT_AUTHORIZATION
    Tcl_AppendResult(interp, "authorization not available in this build",
                     (char*)0);
    trả_về TCL_ERROR;
#không_thì
    nếu( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      trả_về TCL_ERROR;
    }không_thì nếu( objc==2 ){
      nếu( pDb->zAuth ){
        Tcl_AppendResult(interp, pDb->zAuth, (char*)0);
      }
    }không_thì{
      char *zAuth;
      Tcl_Size len;
      nếu( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      nếu( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }không_thì{
        pDb->zAuth = 0;
      }
      nếu( pDb->zAuth ){
        typedef số_nguyên (*sqlite3_auth_cb)(
           void*,số_nguyên,const char*,const char*,
           const char*,const char*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }không_thì{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#endif
    break;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  case DB_BACKUP: {
    const char *zDestFile;
    const char *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    nếu( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }không_thì nếu( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }không_thì{
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? FILENAME");
      trả_về TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    nếu( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (char*)0);
      sqlite3_close(pDest);
      trả_về TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    nếu( pBackup==0 ){
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (char*)0);
      sqlite3_close(pDest);
      trả_về TCL_ERROR;
    }
    trong_khi(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    nếu( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }không_thì{
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (char*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    break;
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
  case DB_BIND_FALLBACK: {
    nếu( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      trả_về TCL_ERROR;
    }không_thì nếu( objc==2 ){
      nếu( pDb->zBindFallback ){
        Tcl_AppendResult(interp, pDb->zBindFallback, (char*)0);
      }
    }không_thì{
      char *zCallback;
      Tcl_Size len;
      nếu( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      nếu( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }không_thì{
        pDb->zBindFallback = 0;
      }
    }
    break;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  case DB_BUSY: {
    nếu( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "CALLBACK");
      trả_về TCL_ERROR;
    }không_thì nếu( objc==2 ){
      nếu( pDb->zBusy ){
        Tcl_AppendResult(interp, pDb->zBusy, (char*)0);
      }
    }không_thì{
      char *zBusy;
      Tcl_Size len;
      nếu( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      nếu( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }không_thì{
        pDb->zBusy = 0;
      }
      nếu( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }không_thì{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    break;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  case DB_CACHE: {
    char *subCmd;
    số_nguyên n;

    nếu( objc<=2 ){
      Tcl_WrongNumArgs(interp, 1, objv, "cache option ?arg?");
      trả_về TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    nếu( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      nếu( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "flush");
        trả_về TCL_ERROR;
      }không_thì{
        flushStmtCache( pDb );
      }
    }không_thì nếu( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      nếu( objc!=4 ){
        Tcl_WrongNumArgs(interp, 2, objv, "size n");
        trả_về TCL_ERROR;
      }không_thì{
        nếu( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_AppendResult( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (char*)0);
          trả_về TCL_ERROR;
        }không_thì{
          nếu( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }không_thì nếu( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }không_thì{
      Tcl_AppendResult( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (char*)0);
      trả_về TCL_ERROR;
    }
    break;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  case DB_CHANGES: {
    Tcl_Obj *pResult;
    nếu( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      trả_về TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_changes64(pDb->db));
    break;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  case DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    break;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  case DB_COLLATE: {
    SqlCollate *pCollate;
    char *zName;
    char *zScript;
    Tcl_Size nScript;
    nếu( objc!=4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NAME SCRIPT");
      trả_về TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( sizeof(*pCollate) + nScript + 1 );
    nếu( pCollate==0 ) trả_về TCL_ERROR;
    pCollate->interp = interp;
    pCollate->pNext = pDb->pCollate;
    pCollate->zScript = (char*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    nếu( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_SetResult(interp, (char *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      trả_về TCL_ERROR;
    }
    break;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  case DB_COLLATION_NEEDED: {
    nếu( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SCRIPT");
      trả_về TCL_ERROR;
    }
    nếu( pDb->pCollateNeeded ){
      Tcl_DecrRefCount(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_IncrRefCount(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    break;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  case DB_COMMIT_HOOK: {
    nếu( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      trả_về TCL_ERROR;
    }không_thì nếu( objc==2 ){
      nếu( pDb->zCommit ){
        Tcl_AppendResult(interp, pDb->zCommit, (char*)0);
      }
    }không_thì{
      const char *zCommit;
      Tcl_Size len;
      nếu( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      nếu( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }không_thì{
        pDb->zCommit = 0;
      }
      nếu( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }không_thì{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    break;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  case DB_COMPLETE: {
#ifndef SQLITE_OMIT_COMPLETE
    Tcl_Obj *pResult;
    số_nguyên isComplete;
    nếu( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SQL");
      trả_về TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetBooleanObj(pResult, isComplete);
#endif
    break;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  case DB_CONFIG: {
    static const struct DbConfigChoices {
      const char *zName;
      số_nguyên op;
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
    số_nguyên ii;
    nếu( objc>4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      trả_về TCL_ERROR;
    }
    nếu( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      pResult = Tcl_NewListObj(0,0);
      cho(ii=0; ii<sizeof(aDbConfig)/sizeof(aDbConfig[0]); ii++){
        số_nguyên v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewIntObj(v));
      }
    }không_thì{
      const char *zOpt = Tcl_GetString(objv[2]);
      số_nguyên onoff = -1;
      số_nguyên v = 0;
      nếu( zOpt[0]=='-' ) zOpt++;
      cho(ii=0; ii<sizeof(aDbConfig)/sizeof(aDbConfig[0]); ii++){
        nếu( strcmp(aDbConfig[ii].zName, zOpt)==0 ) break;
      }
      nếu( ii>=sizeof(aDbConfig)/sizeof(aDbConfig[0]) ){
        Tcl_AppendResult(interp, "unknown config option: \"", zOpt,
                                "\"", (void*)0);
        trả_về TCL_ERROR;
      }
      nếu( objc==4 ){
        nếu( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          trả_về TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      pResult = Tcl_NewIntObj(v);
    }
    Tcl_SetObjResult(interp, pResult);
    break;
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
  case DB_COPY: {
    char *zTable;               /* Insert data into this table */
    char *zFile;                /* The file from which to extract data */
    char *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    số_nguyên nCol;                   /* Number of columns in the table */
    số_nguyên nByte;                  /* Number of bytes in an SQL string */
    số_nguyên i, j;                   /* Loop counters */
    số_nguyên nSep;                   /* Number of bytes in zSep[] */
    số_nguyên nNull;                  /* Number of bytes in zNull[] */
    char *zSql;                 /* An SQL statement */
    char *zLine;                /* A single line of input from the file */
    char **azCol;               /* zLine[] broken up into columns */
    const char *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    số_nguyên lineno = 0;             /* Line number of input file */
    char zLineNum[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *pResult;           /* interp result */

    const char *zSep;
    const char *zNull;
    nếu( objc<5 || objc>7 ){
      Tcl_WrongNumArgs(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      trả_về TCL_ERROR;
    }
    nếu( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }không_thì{
      zSep = "\t";
    }
    nếu( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }không_thì{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    nếu( nSep==0 ){
      Tcl_AppendResult(interp,"Error: non-null separator required for copy",
                       (char*)0);
      trả_về TCL_ERROR;
    }
    nếu(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_AppendResult(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (char*)0);
      trả_về TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    nếu( zSql==0 ){
      Tcl_AppendResult(interp, "Error: no such table: ", zTable, (char*)0);
      trả_về TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    nếu( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (char*)0);
      nCol = 0;
    }không_thì{
      nCol = sqlite3_column_count(pStmt);
    }
    sqlite3_finalize(pStmt);
    nếu( nCol==0 ) {
      trả_về TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    nếu( zSql==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (char*)0);
      trả_về TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    cho(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    free(zSql);
    nếu( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (char*)0);
      sqlite3_finalize(pStmt);
      trả_về TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    nếu( in==0 ){
      sqlite3_finalize(pStmt);
      trả_về TCL_ERROR;
    }
    Tcl_SetChannelOption(NULL, in, "-translation", "auto");
    azCol = malloc( sizeof(azCol[0])*(nCol+1) );
    nếu( azCol==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (char*)0);
      Tcl_Close(interp, in);
      trả_về TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_IncrRefCount(str);
    (void)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    trong_khi( Tcl_GetsObj(in, str)>=0 ) {
      char *z;
      Tcl_Size byteLen;
      lineno++;
      zLine = (char *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = zLine;
      cho(i=0, z=zLine; *z; z++){
        nếu( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          nếu( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      nếu( i+1!=nCol ){
        char *zErr;
        số_nguyên nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        nếu( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_AppendResult(interp, zErr, (char*)0);
          free(zErr);
        }
        zCommit = "ROLLBACK";
        break;
      }
      cho(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        nếu( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }không_thì{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_SetObjLength(str, 0);
      nếu( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp,"Error: ", sqlite3_errmsg(pDb->db), (char*)0);
        zCommit = "ROLLBACK";
        break;
      }
    }
    Tcl_DecrRefCount(str);
    free(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (void)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    nếu( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      pResult = Tcl_GetObjResult(interp);
      nếu( Tcl_IsShared(pResult) ){
        pResult = Tcl_DuplicateObj(pResult);
      }
      Tcl_SetIntObj(pResult, lineno);
      rc = TCL_OK;
    }không_thì{
      /* failure, append lineno where failed */
      sqlite3_snprintf(sizeof(zLineNum), zLineNum,"%d",lineno);
      Tcl_AppendResult(interp,", failed while processing line: ",zLineNum,
                       (char*)0);
      rc = TCL_ERROR;
    }
    break;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  case DB_DESERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (char*)0);
    rc = TCL_ERROR;
#không_thì
    const char *zSchema = 0;
    Tcl_Obj *pValue = 0;
    unsigned char *pBA;
    unsigned char *pData;
    Tcl_Size len;
    số_nguyên xrc;
    sqlite3_int64 mxSize = 0;
    số_nguyên i;
    số_nguyên isReadonly = 0;


    nếu( objc<3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      break;
    }
    cho(i=2; i<objc-1; i++){
      const char *z = Tcl_GetString(objv[i]);
      nếu( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        nếu( rc ) goto deserialize_error;
        mxSize = x;
        continue;
      }
      nếu( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        nếu( rc ) goto deserialize_error;
        continue;
      }
      nếu( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        continue;
      }
      Tcl_AppendResult(interp, "unknown option: ", z, (char*)0);
      rc = TCL_ERROR;
      goto deserialize_error;
    }
    pValue = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(pValue, &len);
    pData = sqlite3_malloc64( len );
    nếu( pData==0 && len>0 ){
      Tcl_AppendResult(interp, "out of memory", (char*)0);
      rc = TCL_ERROR;
    }không_thì{
      số_nguyên flags;
      nếu( len>0 ) memcpy(pData, pBA, len);
      nếu( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }không_thì{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, pData, len, len, flags);
      nếu( xrc ){
        Tcl_AppendResult(interp, "unable to set MEMDB content", (char*)0);
        rc = TCL_ERROR;
      }
      nếu( mxSize>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_SIZE_LIMIT,&mxSize);
      }
    }
deserialize_error:
#endif
    break; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  case DB_ENABLE_LOAD_EXTENSION: {
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    số_nguyên onoff;
    nếu( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "BOOLEAN");
      trả_về TCL_ERROR;
    }
    nếu( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      trả_về TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    break;
#không_thì
    Tcl_AppendResult(interp, "extension loading is turned off at compile-time",
                     (char*)0);
    trả_về TCL_ERROR;
#endif
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  case DB_ERRORCODE: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    break;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  case DB_ERROROFFSET: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    break;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  case DB_EXISTS:
  case DB_ONECOLUMN: {
    Tcl_Obj *pResult = 0;
    DbEvalContext sEval;
    nếu( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SQL");
      trả_về TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    nếu( choice==DB_ONECOLUMN ){
      nếu( rc==TCL_OK ){
        pResult = dbEvalColumnValue(&sEval, 0);
      }không_thì nếu( rc==TCL_BREAK ){
        Tcl_ResetResult(interp);
      }
    }không_thì nếu( rc==TCL_BREAK || rc==TCL_OK ){
      pResult = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    nếu( pResult ) Tcl_SetObjResult(interp, pResult);

    nếu( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    break;
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
  case DB_EVAL: {
    số_nguyên evalFlags = 0;
    const char *zOpt;
    trong_khi( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      nếu( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }không_thì nếu( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }không_thì{
        Tcl_AppendResult(interp, "unknown option: \"", zOpt, "\"", (void*)0);
        trả_về TCL_ERROR;
      }
      objc--;
      objv++;
    }
    nếu( objc<3 || objc>5 ){
      Tcl_WrongNumArgs(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      trả_về TCL_ERROR;
    }

    nếu( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_IncrRefCount(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      trong_khi( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        số_nguyên i;
        số_nguyên nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        cho(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, dbEvalColumnValue(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      nếu( rc==TCL_BREAK ){
        Tcl_SetObjResult(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_DecrRefCount(pRet);
    }không_thì{
      ClientData cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      nếu( objc>=5 && *(char *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_IncrRefCount(pScript);

      p = (DbEvalContext *)Tcl_Alloc(sizeof(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (void *)p;
      cd2[1] = (void *)pScript;
      rc = DbEvalNextCmd(cd2, interp, TCL_OK);
    }
    break;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  case DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    break;
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
  case DB_FUNCTION: {
    số_nguyên flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    char *zName;
    số_nguyên nArg = -1;
    số_nguyên i;
    số_nguyên eType = SQLITE_NULL;
    nếu( objc<4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      trả_về TCL_ERROR;
    }
    cho(i=3; i<(objc-1); i++){
      const char *z = Tcl_GetString(objv[i]);
      số_nguyên n = strlen30(z);
      nếu( n>1 && strncmp(z, "-argcount",n)==0 ){
        nếu( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(char*)0);
          trả_về TCL_ERROR;
        }
        nếu( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) trả_về TCL_ERROR;
        nếu( nArg<0 ){
          Tcl_AppendResult(interp, "number of arguments must be non-negative",
                           (char*)0);
          trả_về TCL_ERROR;
        }
        i++;
      }không_thì
      nếu( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }không_thì
      nếu( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }không_thì
      nếu( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }không_thì
      nếu( n>1 && strncmp(z, "-returntype", n)==0 ){
        const char *azType[] = {"integer", "real", "text", "blob", "any", 0};
        assert( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        assert( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        nếu( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(char*)0);
          trả_về TCL_ERROR;
        }
        i++;
        nếu( Tcl_GetIndexFromObj(interp, objv[i], azType, "type", 0, &eType) ){
          trả_về TCL_ERROR;
        }
        eType++;
      }không_thì{
        Tcl_AppendResult(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (char*)0
        );
        trả_về TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    nếu( pFunc==0 ) trả_về TCL_ERROR;
    nếu( pFunc->pScript ){
      Tcl_DecrRefCount(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_IncrRefCount(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    nếu( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_SetResult(interp, (char *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    break;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  case DB_INCRBLOB: {
#ifdef SQLITE_OMIT_INCRBLOB
    Tcl_AppendResult(interp, "incrblob not available in this build", (char*)0);
    trả_về TCL_ERROR;
#không_thì
    số_nguyên isReadonly = 0;
    const char *zDb = "main";
    const char *zTable;
    const char *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    nếu( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    nếu( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_WrongNumArgs(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      trả_về TCL_ERROR;
    }

    nếu( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    nếu( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#endif
    break;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  case DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    break;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  case DB_NULLVALUE: {
    nếu( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NULLVALUE");
      trả_về TCL_ERROR;
    }
    nếu( objc==3 ){
      Tcl_Size len;
      char *zNull = Tcl_GetStringFromObj(objv[2], &len);
      nếu( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      nếu( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }không_thì{
        pDb->zNull = 0;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(pDb->zNull, -1));
    break;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  case DB_LAST_INSERT_ROWID: {
    Tcl_Obj *pResult;
    Tcl_WideInt rowid;
    nếu( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      trả_về TCL_ERROR;
    }
    rowid = sqlite3_last_insert_rowid(pDb->db);
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, rowid);
    break;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  case DB_PROGRESS: {
    nếu( objc==2 ){
      nếu( pDb->zProgress ){
        Tcl_AppendResult(interp, pDb->zProgress, (char*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }không_thì nếu( objc==4 ){
      char *zProgress;
      Tcl_Size len;
      số_nguyên N;
      nếu( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        trả_về TCL_ERROR;
      };
      nếu( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      nếu( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }không_thì{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      nếu( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }không_thì{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }không_thì{
      Tcl_WrongNumArgs(interp, 2, objv, "N CALLBACK");
      trả_về TCL_ERROR;
    }
    break;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  case DB_PROFILE: {
    nếu( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      trả_về TCL_ERROR;
    }không_thì nếu( objc==2 ){
      nếu( pDb->zProfile ){
        Tcl_AppendResult(interp, pDb->zProfile, (char*)0);
      }
    }không_thì{
      char *zProfile;
      Tcl_Size len;
      nếu( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      nếu( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }không_thì{
        pDb->zProfile = 0;
      }
#nếu !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      nếu( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }không_thì{
        sqlite3_profile(pDb->db, 0, 0);
      }
#endif
    }
    break;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  case DB_REKEY: {
    nếu( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "KEY");
      trả_về TCL_ERROR;
    }
    break;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  case DB_RESTORE: {
    const char *zSrcFile;
    const char *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    số_nguyên nTimeout = 0;

    nếu( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }không_thì nếu( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }không_thì{
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? FILENAME");
      trả_về TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    nếu( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (char*)0);
      sqlite3_close(pSrc);
      trả_về TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    nếu( pBackup==0 ){
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (char*)0);
      sqlite3_close(pSrc);
      trả_về TCL_ERROR;
    }
    trong_khi( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      nếu( rc==SQLITE_BUSY ){
        nếu( nTimeout++ >= 3 ) break;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    nếu( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }không_thì nếu( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_AppendResult(interp, "restore failed: source database busy",
                       (char*)0);
      rc = TCL_ERROR;
    }không_thì{
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (char*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    break;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  case DB_SERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (char*)0);
    rc = TCL_ERROR;
#không_thì
    const char *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned char *pData;
    nếu( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }không_thì{
      số_nguyên needFree;
      pData = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      nếu( pData ){
        needFree = 0;
      }không_thì{
        pData = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(pData,sz));
      nếu( needFree ) sqlite3_free(pData);
    }
#endif
    break;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  case DB_STATUS: {
    số_nguyên v;
    const char *zOp;
    nếu( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "(step|sort|autoindex)");
      trả_về TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    nếu( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }không_thì nếu( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }không_thì nếu( strcmp(zOp, "autoindex")==0 ){
      v = pDb->nIndex;
    }không_thì nếu( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }không_thì{
      Tcl_AppendResult(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (char*)0);
      trả_về TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(v));
    break;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  case DB_TIMEOUT: {
    số_nguyên ms;
    nếu( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "MILLISECONDS");
      trả_về TCL_ERROR;
    }
    nếu( Tcl_GetIntFromObj(interp, objv[2], &ms) ) trả_về TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    break;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  case DB_TOTAL_CHANGES: {
    Tcl_Obj *pResult;
    nếu( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      trả_về TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_total_changes64(pDb->db));
    break;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  case DB_TRACE: {
    nếu( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      trả_về TCL_ERROR;
    }không_thì nếu( objc==2 ){
      nếu( pDb->zTrace ){
        Tcl_AppendResult(interp, pDb->zTrace, (char*)0);
      }
    }không_thì{
      char *zTrace;
      Tcl_Size len;
      nếu( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      nếu( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }không_thì{
        pDb->zTrace = 0;
      }
#nếu !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      nếu( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }không_thì{
        sqlite3_trace(pDb->db, 0, 0);
      }
#endif
    }
    break;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  case DB_TRACE_V2: {
    nếu( objc>4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK? ?MASK?");
      trả_về TCL_ERROR;
    }không_thì nếu( objc==2 ){
      nếu( pDb->zTraceV2 ){
        Tcl_AppendResult(interp, pDb->zTraceV2, (char*)0);
      }
    }không_thì{
      char *zTraceV2;
      Tcl_Size len;
      Tcl_WideInt wMask = 0;
      nếu( objc==4 ){
        static const char *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        enum TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_Size i;
        nếu( TCL_OK!=Tcl_ListObjLength(interp, objv[3], &len) ){
          trả_về TCL_ERROR;
        }
        cho(i=0; i<len; i++){
          Tcl_Obj *pObj;
          số_nguyên ttype;
          nếu( TCL_OK!=Tcl_ListObjIndex(interp, objv[3], i, &pObj) ){
            trả_về TCL_ERROR;
          }
          nếu( Tcl_GetIndexFromObj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
            Tcl_IncrRefCount(pError);
            nếu( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_DecrRefCount(pError);
              wMask |= wType;
            }không_thì{
              Tcl_SetObjResult(interp, pError);
              Tcl_DecrRefCount(pError);
              trả_về TCL_ERROR;
            }
          }không_thì{
            switch( (enum TTYPE_enum)ttype ){
              case TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    break;
              case TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; break;
              case TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     break;
              case TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   break;
            }
          }
        }
      }không_thì{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      nếu( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      nếu( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }không_thì{
        pDb->zTraceV2 = 0;
      }
#nếu !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      nếu( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }không_thì{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#endif
    }
    break;
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
  case DB_TRANSACTION: {
    Tcl_Obj *pScript;
    const char *zBegin = "SAVEPOINT _tcl_transaction";
    nếu( objc!=3 && objc!=4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "[TYPE] SCRIPT");
      trả_về TCL_ERROR;
    }

    nếu( pDb->nTransaction==0 && objc==4 ){
      static const char *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      enum TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      số_nguyên ttype;
      nếu( Tcl_GetIndexFromObj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        trả_về TCL_ERROR;
      }
      switch( (enum TTYPE_enum)ttype ){
        case TTYPE_DEFERRED:    /* no-op */;                 break;
        case TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  break;
        case TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  break;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    nếu( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (char*)0);
      trả_về TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    nếu( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (void)Tcl_NREvalObj(interp, pScript, 0);
    }không_thì{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    break;
  }

  /*
  **    $db unlock_notify ?script?
  */
  case DB_UNLOCK_NOTIFY: {
#ifndef SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_AppendResult(interp, "unlock_notify not available in this build",
                     (char*)0);
    rc = TCL_ERROR;
#không_thì
    nếu( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }không_thì{
      void (*xNotify)(void **, số_nguyên) = 0;
      void *pNotifyArg = 0;

      nếu( pDb->pUnlockNotify ){
        Tcl_DecrRefCount(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      nếu( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (void *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_IncrRefCount(pDb->pUnlockNotify);
      }

      nếu( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (char*)0);
        rc = TCL_ERROR;
      }
    }
#endif
    break;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  case DB_PREUPDATE: {
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_AppendResult(interp, "preupdate_hook was omitted at compile-time", 
                     (char*)0);
    rc = TCL_ERROR;
#không_thì
    static const char *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    enum DbPreupdateSubCmd {
      PRE_COUNT, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    số_nguyên iSub;

    nếu( objc<3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    nếu( Tcl_GetIndexFromObj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      trả_về TCL_ERROR;
    }

    switch( (enum DbPreupdateSubCmd)iSub ){
      case PRE_COUNT: {
        số_nguyên nCol = sqlite3_preupdate_count(pDb->db);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nCol));
        break;
      }

      case PRE_HOOK: {
        nếu( objc>4 ){
          Tcl_WrongNumArgs(interp, 2, objv, "hook ?SCRIPT?");
          trả_về TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        break;
      }

      case PRE_DEPTH: {
        Tcl_Obj *pRet;
        nếu( objc!=3 ){
          Tcl_WrongNumArgs(interp, 3, objv, "");
          trả_về TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_SetObjResult(interp, pRet);
        break;
      }

      case PRE_NEW:
      case PRE_OLD: {
        số_nguyên iIdx;
        sqlite3_value *pValue;
        nếu( objc!=4 ){
          Tcl_WrongNumArgs(interp, 3, objv, "INDEX");
          trả_về TCL_ERROR;
        }
        nếu( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          trả_về TCL_ERROR;
        }

        nếu( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &pValue);
        }không_thì{
          assert( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &pValue);
        }

        nếu( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((char*)sqlite3_value_text(pValue), -1);
          Tcl_SetObjResult(interp, pObj);
        }không_thì{
          Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (char*)0);
          trả_về TCL_ERROR;
        }
      }
    }
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
    break;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  case DB_WAL_HOOK:
  case DB_UPDATE_HOOK:
  case DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    nếu( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    nếu( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    nếu( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    nếu( objc>3 ){
       Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
       trả_về TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    break;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  case DB_VERSION: {
    số_nguyên i;
    cho(i=2; i<objc; i++){
      const char *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      nếu( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        nếu( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          trả_về TCL_ERROR;
        }
      }không_thì

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      nếu( strcmp(zArg, "-last-stmt-ptr")==0 ){
        extern const char *sqlite3TestPtrToText(void*);
        void *ptr = (void*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_SetResult(interp, (char*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }không_thì
#endif /* SQLITE_TEST */
      {
        Tcl_AppendResult(interp, "unknown argument: ", zArg, (char*)0);
        trả_về TCL_ERROR;
      }
    }
    nếu( i==2 ){   
      Tcl_SetResult(interp, (char *)sqlite3_libversion(), TCL_STATIC);
    }
    break;
  }


  } /* End of the SWITCH statement */
  trả_về rc;
}

#nếu SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
static số_nguyên SQLITE_TCLAPI DbObjCmdAdaptor(
  void *cd,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *const*objv
){
  trả_về Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
static số_nguyên sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *const*objv
){
  Tcl_WrongNumArgs(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  trả_về TCL_ERROR;
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
static số_nguyên SQLITE_TCLAPI DbMain(
  void *cd,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *const*objv
){
  SqliteDb *p;
  const char *zArg;
  char *zErrMsg;
  số_nguyên i;
  const char *zFile = 0;
  const char *zVfs = 0;
  số_nguyên flags;
  số_nguyên bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  số_nguyên rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#không_thì
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  nếu( objc==1 ) trả_về sqliteCmdUsage(interp, objv);
  nếu( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    nếu( strcmp(zArg,"-version")==0 ){
      Tcl_AppendResult(interp,sqlite3_libversion(), (char*)0);
      trả_về TCL_OK;
    }
    nếu( strcmp(zArg,"-sourceid")==0 ){
      Tcl_AppendResult(interp,sqlite3_sourceid(), (char*)0);
      trả_về TCL_OK;
    }
    nếu( strcmp(zArg,"-has-codec")==0 ){
      Tcl_AppendResult(interp,"0",(char*)0);
      trả_về TCL_OK;
    }
    nếu( zArg[0]=='-' ) trả_về sqliteCmdUsage(interp, objv);
  }
  cho(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    nếu( zArg[0]!='-' ){
      nếu( zFile!=0 ) trả_về sqliteCmdUsage(interp, objv);
      zFile = zArg;
      continue;
    }
    nếu( i==objc-1 ) trả_về sqliteCmdUsage(interp, objv);
    i++;
    nếu( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }không_thì nếu( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }không_thì nếu( strcmp(zArg, "-readonly")==0 ){
      số_nguyên b;
      nếu( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) trả_về TCL_ERROR;
      nếu( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }không_thì{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }không_thì nếu( strcmp(zArg, "-create")==0 ){
      số_nguyên b;
      nếu( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) trả_về TCL_ERROR;
      nếu( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }không_thì{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }không_thì nếu( strcmp(zArg, "-nofollow")==0 ){
      số_nguyên b;
      nếu( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) trả_về TCL_ERROR;
      nếu( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }không_thì{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }không_thì nếu( strcmp(zArg, "-nomutex")==0 ){
      số_nguyên b;
      nếu( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) trả_về TCL_ERROR;
      nếu( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }không_thì{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }không_thì nếu( strcmp(zArg, "-fullmutex")==0 ){
      số_nguyên b;
      nếu( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) trả_về TCL_ERROR;
      nếu( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }không_thì{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }không_thì nếu( strcmp(zArg, "-uri")==0 ){
      số_nguyên b;
      nếu( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) trả_về TCL_ERROR;
      nếu( b ){
        flags |= SQLITE_OPEN_URI;
      }không_thì{
        flags &= ~SQLITE_OPEN_URI;
      }
    }không_thì nếu( strcmp(zArg, "-translatefilename")==0 ){
      nếu( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        trả_về TCL_ERROR;
      }
    }không_thì{
      Tcl_AppendResult(interp, "unknown option: ", zArg, (char*)0);
      trả_về TCL_ERROR;
    }
  }
  zErrMsg = 0;
  p = (SqliteDb*)Tcl_Alloc( sizeof(*p) );
  memset(p, 0, sizeof(*p));
  nếu( zFile==0 ) zFile = "";
  nếu( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  nếu( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  nếu( p->db ){
    nếu( SQLITE_OK!=sqlite3_errcode(p->db) ){
      zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }không_thì{
    zErrMsg = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  nếu( p->db==0 ){
    Tcl_SetResult(interp, zErrMsg, TCL_VOLATILE);
    Tcl_Free((char*)p);
    sqlite3_free(zErrMsg);
    trả_về TCL_ERROR;
  }
  p->maxStmt = NUM_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  nếu( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (char*)p, DbDeleteCmd);
  }không_thì{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (char*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  trả_về TCL_OK;
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
EXTERN số_nguyên Sqlite3_Init(Tcl_Interp *interp){
  số_nguyên rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  nếu( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  trả_về rc;
}
EXTERN số_nguyên Tclsqlite3_Init(Tcl_Interp *interp){ trả_về Sqlite3_Init(interp); }
EXTERN số_nguyên Sqlite3_Unload(Tcl_Interp *interp, số_nguyên flags){ trả_về TCL_OK; }
EXTERN số_nguyên Tclsqlite3_Unload(Tcl_Interp *interp, số_nguyên flags){ trả_về TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN số_nguyên Sqlite3_SafeInit(Tcl_Interp *interp){ trả_về TCL_ERROR; }
EXTERN số_nguyên Sqlite3_SafeUnload(Tcl_Interp *interp, số_nguyên flags){trả_về TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN số_nguyên Sqlite_Init(Tcl_Interp *interp){ trả_về Sqlite3_Init(interp);}
EXTERN số_nguyên Tclsqlite_Init(Tcl_Interp *interp){ trả_về Sqlite3_Init(interp); }
EXTERN số_nguyên Sqlite_Unload(Tcl_Interp *interp, số_nguyên flags){ trả_về TCL_OK; }
EXTERN số_nguyên Tclsqlite_Unload(Tcl_Interp *interp, số_nguyên flags){ trả_về TCL_OK; }
EXTERN số_nguyên Sqlite_SafeInit(Tcl_Interp *interp){ trả_về TCL_ERROR; }
EXTERN số_nguyên Sqlite_SafeUnload(Tcl_Interp *interp, số_nguyên flags){trả_về TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN số_nguyên sqlite3_Init(Tcl_Interp *interp){ trả_về Sqlite3_Init(interp);}
EXTERN số_nguyên sqlite_Init(Tcl_Interp *interp){ trả_về Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#nếu defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
static const char *tclsh_main_loop(void){
  static const char zMainloop[] =
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
  trả_về zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN main
#endif
số_nguyên SQLITE_CDECL TCLSH_MAIN(số_nguyên argc, char **argv){
  Tcl_Interp *interp;
  số_nguyên i;
  const char *zScript = 0;
  char zArgc[32];
#nếu defined(TCLSH_INIT_PROC)
  extern const char *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#nếu !defined(_WIN32_WCE)
  nếu( getenv("SQLITE_DEBUG_BREAK") ){
    nếu( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }không_thì{
#nếu defined(_WIN32) || defined(WIN32)
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

  sqlite3_snprintf(sizeof(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  cho(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_VALUE);
  }
#nếu defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  nếu( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  nếu( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    const char *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    nếu( zInfo==0 ) zInfo = Tcl_GetStringResult(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    trả_về 1;
  }
  trả_về 0;
}
#endif /* TCLSH */
