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
#ប្រសិនបើ defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#ប្រសិនបើ defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#បើមិនដូច្នេះ
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#ប្រសិនបើ TCL_MAJOR_VERSION==9
# define CONST const
#elif !defined(Tcl_Size)
  typedef ចំនួនគត់ Tcl_Size;
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
#   ប្រសិនបើ defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   បើមិនដូច្នេះ
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# ប្រសិនបើ defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    typedef uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    typedef unsigned ចំនួនគត់ uptr;
# បើមិនដូច្នេះ
    typedef sqlite3_uint64 uptr;
# endif
#endif
#include <ctype.h>

/* Used to get the current process ID */
#ប្រសិនបើ !defined(_WIN32)
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
# define GETPID (ចំនួនគត់)GetCurrentProcessId
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
#ប្រសិនបើ defined(SQLITE_ENABLE_QRF_IN_TCL)
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
  ចំនួនគត់ useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  ចំនួនគត់ eType;            /* Type of value to return */
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
  ចំនួនគត់ nSql;                /* chars in zSql[] */
  const var_na *zSql;        /* Text of the SQL statement */
  ចំនួនគត់ nParm;               /* Size of apParm array */
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
  ចំនួនគត់ disableAuth;           /* Disable the authorizer if it exists */
  var_na *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  ចំនួនគត់ rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  ចំនួនគត់ maxStmt;               /* The next maximum number of stmtList */
  ចំនួនគត់ nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  ចំនួនគត់ nStep, nSort, nIndex;  /* Statistics for most recent operation */
  ចំនួនគត់ nVMStep;               /* Another statistic for most recent operation */
  ចំនួនគត់ nTransaction;          /* Number of nested [transaction] methods */
  ចំនួនគត់ openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  ចំនួនគត់ nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  ចំនួនគត់ bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

sa_nracanaa IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned ចំនួនគត់ isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *pNext;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
static ចំនួនគត់ strlen30(const var_na *z){
  const var_na *z2 = z;
  ខណៈពេល( *z2 ){ z2++; }
  ត្រឡប់ 0x3fffffff & (ចំនួនគត់)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
static void closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *pNext;

  សម្រាប់(p=pDb->pIncrblob; p; p=pNext){
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
static ចំនួនគត់ SQLITE_TCLAPI incrblobClose2(
  ClientData instanceData,
  Tcl_Interp *interp,
  ចំនួនគត់ flags
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  ចំនួនគត់  rc;
  sqlite3 *db = p->pDb->db;

  ប្រសិនបើ( flags ){
    p->isClosed |= flags;
    ត្រឡប់ TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  ប្រសិនបើ( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  ប្រសិនបើ( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }
  ប្រសិនបើ( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->pNext;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((var_na *)p);

  ប្រសិនបើ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (var_na *)sqlite3_errmsg(db), TCL_VOLATILE);
    ត្រឡប់ TCL_ERROR;
  }
  ត្រឡប់ TCL_OK;
}
static ចំនួនគត់ SQLITE_TCLAPI incrblobClose(
  ClientData instanceData,
  Tcl_Interp *interp
){
  ត្រឡប់ incrblobClose2(instanceData, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
static ចំនួនគត់ SQLITE_TCLAPI incrblobInput(
  ClientData instanceData,
  var_na *buf,
  ចំនួនគត់ bufSize,
  ចំនួនគត់ *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nRead = bufSize;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  ចំនួនគត់ rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  ប្រសិនបើ( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  ប្រសិនបើ( nRead<=0 ){
    ត្រឡប់ 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (void *)buf, (ចំនួនគត់)nRead, (ចំនួនគត់)p->iSeek);
  ប្រសិនបើ( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    ត្រឡប់ -1;
  }

  p->iSeek += nRead;
  ត្រឡប់ nRead;
}

/*
** Write data to an incremental blob channel.
*/
static ចំនួនគត់ SQLITE_TCLAPI incrblobOutput(
  ClientData instanceData,
  const var_na *buf,
  ចំនួនគត់ toWrite,
  ចំនួនគត់ *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  ចំនួនគត់ rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  ប្រសិនបើ( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    ត្រឡប់ -1;
  }
  ប្រសិនបើ( nWrite<=0 ){
    ត្រឡប់ 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (void*)buf,(ចំនួនគត់)nWrite, (ចំនួនគត់)p->iSeek);
  ប្រសិនបើ( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    ត្រឡប់ -1;
  }

  p->iSeek += nWrite;
  ត្រឡប់ nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#ប្រសិនបើ TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#បើមិនដូច្នេះ
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
static WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  ClientData instanceData,
  WideSeekProcType offset,
  ចំនួនគត់ seekMode,
  ចំនួនគត់ *errorCodePtr
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

  ត្រឡប់ p->iSeek;
}
static ចំនួនគត់ SQLITE_TCLAPI incrblobSeek(
  ClientData instanceData,
  long offset,
  ចំនួនគត់ seekMode,
  ចំនួនគត់ *errorCodePtr
){
  ត្រឡប់ incrblobWideSeek(instanceData,offset,seekMode,errorCodePtr);
}


static void SQLITE_TCLAPI incrblobWatch(
  ClientData instanceData,
  ចំនួនគត់ mode
){
  /* NO-OP */
}
static ចំនួនគត់ SQLITE_TCLAPI incrblobHandle(
  ClientData instanceData,
  ចំនួនគត់ dir,
  ClientData *hPtr
){
  ត្រឡប់ TCL_ERROR;
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
static ចំនួនគត់ createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  const var_na *zDb,
  const var_na *zTable,
  const var_na *zColumn,
  sqlite_int64 iRow,
  ចំនួនគត់ isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  ចំនួនគត់ rc;
  ចំនួនគត់ flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  static ចំនួនគត់ count = 0;
  var_na zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  ប្រសិនបើ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (var_na *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    ត្រឡប់ TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(parimaa_na(IncrblobChannel));
  memset(p, 0, parimaa_na(*p));
  p->pBlob = pBlob;
  ប្រសិនបើ( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(parimaa_na(zChannel), zChannel, "incrblob_%d", ++count);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->pNext = pDb->pIncrblob;
  p->pPrev = 0;
  ប្រសិនបើ( p->pNext ){
    p->pNext->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_SetResult(interp, (var_na *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  ត្រឡប់ TCL_OK;
}
#បើមិនដូច្នេះ  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
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
static ចំនួនគត់ safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  const var_na *z;
  Tcl_Size n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  ខណៈពេល( n-- > 0 ){
    ចំនួនគត់ c = *(z++);
    ប្រសិនបើ( c=='$' || c=='[' || c==';' ) ត្រឡប់ 0;
  }
  ត្រឡប់ 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
static SqlFunc *findSqlFunc(SqliteDb *pDb, const var_na *zName){
  SqlFunc *p, *pNew;
  ចំនួនគត់ nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( parimaa_na(*pNew) + nName + 1 );
  pNew->zName = (var_na*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  សម្រាប់(p=pDb->pFunc; p; p=p->pNext){
    ប្រសិនបើ( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((var_na*)pNew);
      ត្រឡប់ p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->pNext = pDb->pFunc;
  pDb->pFunc = pNew;
  ត្រឡប់ pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
static void dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  ប្រសិនបើ( sqlite3_sql(pStmt->pStmt)==0 ){
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

  សម្រាប់(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pNext){
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
  ប្រសិនបើ( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    ខណៈពេល( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->pNext;
      assert( pFunc->pDb==pDb );
      Tcl_DecrRefCount(pFunc->pScript);
      Tcl_Free((var_na*)pFunc);
    }
    ខណៈពេល( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->pNext;
      Tcl_Free((var_na*)pCollate);
    }
    ប្រសិនបើ( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    ប្រសិនបើ( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    ប្រសិនបើ( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    ប្រសិនបើ( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    ប្រសិនបើ( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    ប្រសិនបើ( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    ប្រសិនបើ( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    ប្រសិនបើ( pDb->pUpdateHook ){
      Tcl_DecrRefCount(pDb->pUpdateHook);
    }
    ប្រសិនបើ( pDb->pPreUpdateHook ){
      Tcl_DecrRefCount(pDb->pPreUpdateHook);
    }
    ប្រសិនបើ( pDb->pRollbackHook ){
      Tcl_DecrRefCount(pDb->pRollbackHook);
    }
    ប្រសិនបើ( pDb->pWalHook ){
      Tcl_DecrRefCount(pDb->pWalHook);
    }
    ប្រសិនបើ( pDb->pCollateNeeded ){
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
static ចំនួនគត់ DbBusyHandler(void *cd, ចំនួនគត់ nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  ចំនួនគត់ rc;
  var_na zVal[30];

  sqlite3_snprintf(parimaa_na(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (var_na*)0);
  ប្រសិនបើ( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ត្រឡប់ 0;
  }
  ត្រឡប់ 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
static ចំនួនគត់ DbProgressHandler(void *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  ចំនួនគត់ rc;

  assert( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  ប្រសិនបើ( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ត្រឡប់ 1;
  }
  ត្រឡប់ 0;
}
#endif

#ប្រសិនបើ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
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
static ចំនួនគត់ DbTraceV2Handler(
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
  ត្រឡប់ SQLITE_OK;
}
#endif

#ប្រសិនបើ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
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
static ចំនួនគត់ DbCommitHandler(void *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  ចំនួនគត់ rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  ប្រសិនបើ( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ត្រឡប់ 1;
  }
  ត្រឡប់ 0;
}

static void DbRollbackHandler(void *clientData){
  SqliteDb *pDb = (SqliteDb*)clientData;
  assert(pDb->pRollbackHook);
  ប្រសិនបើ( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
static ចំនួនគត់ DbWalHandler(
  void *clientData,
  sqlite3 *db,
  const var_na *zDb,
  ចំនួនគត់ nEntry
){
  ចំនួនគត់ ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)clientData;
  Tcl_Interp *interp = pDb->interp;
  assert(pDb->pWalHook);

  assert( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_IncrRefCount(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  ប្រសិនបើ( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_DecrRefCount(p);

  ត្រឡប់ ret;
}

#ប្រសិនបើ defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
static void setTestUnlockNotifyVars(Tcl_Interp *interp, ចំនួនគត់ iArg, ចំនួនគត់ nArg){
  var_na zBuf[64];
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(parimaa_na(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#បើមិនដូច្នេះ
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
static void DbUnlockNotify(void **apArg, ចំនួនគត់ nArg){
  ចំនួនគត់ i;
  សម្រាប់(i=0; i<nArg; i++){
    const ចំនួនគត់ flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
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
  ចំនួនគត់ op,
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
  ចំនួនគត់ op,
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
  ចំនួនគត់ enc,
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
static ចំនួនគត់ tclSqlCollate(
  void *pCtx,
  ចំនួនគត់ nA,
  const void *zA,
  ចំនួនគត់ nB,
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
  ត្រឡប់ (atoi(Tcl_GetStringResult(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
static void tclSqlFunc(sqlite3_context *context, ចំនួនគត់ argc, sqlite3_value**argv){
  SqlFunc *p = sqlite3_user_data(context);
  Tcl_Obj *pCmd;
  ចំនួនគត់ i;
  ចំនួនគត់ rc;

  ប្រសិនបើ( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_IncrRefCount(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_DecrRefCount(pCmd);
  }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
      ត្រឡប់;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_IncrRefCount(pCmd);
    សម្រាប់(i=0; i<argc; i++){
      sqlite3_value *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      switch( sqlite3_value_type(pIn) ){
        pak_sa SQLITE_BLOB: {
          ចំនួនគត់ bytes = sqlite3_value_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_value_blob(pIn), bytes);
          bha_nga;
        }
        pak_sa SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_value_int64(pIn);
          ប្រសិនបើ( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((ចំនួនគត់)v);
          }បើមិនដូច្នេះ{
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
          ចំនួនគត់ bytes = sqlite3_value_bytes(pIn);
          pVal = Tcl_NewStringObj((var_na *)sqlite3_value_text(pIn), bytes);
          bha_nga;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      ប្រសិនបើ( rc ){
        Tcl_DecrRefCount(pCmd);
        sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
        ត្រឡប់;
      }
    }
    ប្រសិនបើ( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_DecrRefCount(pCmd);
  }

  ប្រសិនបើ( TCL_BREAK==rc ){
    sqlite3_result_null(context);
  }បើមិនដូច្នេះ ប្រសិនបើ( rc && rc!=TCL_RETURN ){
    sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
  }បើមិនដូច្នេះ{
    Tcl_Obj *pVar = Tcl_GetObjResult(p->interp);
    Tcl_Size n;
    u8 *data;
    const var_na *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    var_na c = zType[0];
    ចំនួនគត់ eType = p->eType;

    ប្រសិនបើ( eType==SQLITE_NULL ){
      ប្រសិនបើ( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }បើមិនដូច្នេះ ប្រសិនបើ( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }បើមិនដូច្នេះ ប្រសិនបើ( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }បើមិនដូច្នេះ{
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
        ប្រសិនបើ( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_result_int64(context, v);
          bha_nga;
        }
        /* fall-through */
      }
      pak_sa SQLITE_FLOAT: {
        dviguu_na r;
        ប្រសិនបើ( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
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
static ចំនួនគត់ auth_callback(
  void *pArg,
  ចំនួនគត់ code,
  const var_na *zArg1,
  const var_na *zArg2,
  const var_na *zArg3,
  const var_na *zArg4
){
  const var_na *zCode;
  Tcl_DString str;
  ចំនួនគត់ rc;
  const var_na *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  ប្រសិនបើ( pDb->disableAuth ) ត្រឡប់ SQLITE_OK;

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
  ប្រសិនបើ( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }បើមិនដូច្នេះ{
    rc = 999;
  }
  ត្រឡប់ rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#ប្រសិនបើ 0
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
  ចំនួនគត់ nLine;
  ចំនួនគត់ n;

  nLine = 100;
  zLine = malloc( nLine );
  ប្រសិនបើ( zLine==0 ) ត្រឡប់ 0;
  n = 0;
  ខណៈពេល( 1 ){
    ប្រសិនបើ( n+100>nLine ){
      nLine = nLine*2 + 100;
      zLine = realloc(zLine, nLine);
      ប្រសិនបើ( zLine==0 ) ត្រឡប់ 0;
    }
    ប្រសិនបើ( fgets(&zLine[n], nLine - n, in)==0 ){
      ប្រសិនបើ( n==0 ){
        free(zLine);
        ត្រឡប់ 0;
      }
      zLine[n] = 0;
      bha_nga;
    }
    ខណៈពេល( zLine[n] ){ n++; }
    ប្រសិនបើ( n>0 && zLine[n-1]=='\n' ){
      n--;
      zLine[n] = 0;
      bha_nga;
    }
  }
  zLine = realloc( zLine, n+1 );
  ត្រឡប់ zLine;
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
static ចំនួនគត់ SQLITE_TCLAPI DbTransPostCmd(
  ClientData data[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  ចំនួនគត់ result                           /* Result of evaluating SCRIPT */
){
  static const var_na *const azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)data[0];
  ចំនួនគត់ rc = result;
  const var_na *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  ប្រសិនបើ( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    ប្រសិនបើ( rc!=TCL_ERROR ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (var_na*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  ត្រឡប់ rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
static ចំនួនគត់ dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  const var_na *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  const var_na **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned ចំនួនគត់ prepFlags = 0;
#ifdef SQLITE_TEST
  ប្រសិនបើ( pDb->bLegacyPrepare ){
    ត្រឡប់ sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  ប្រសិនបើ( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  ត្រឡប់ sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
static ចំនួនគត់ dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  var_na const *zIn,                /* SQL to compile */
  var_na const **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  const var_na *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  ចំនួនគត់ nSql;                       /* Length of zSql in bytes */
  ចំនួនគត់ nVar = 0;                   /* Number of variables in statement */
  ចំនួនគត់ iParm = 0;                  /* Next free entry in apParm */
  var_na c;
  ចំនួនគត់ i;
  ចំនួនគត់ needResultReset = 0;        /* Need to invoke Tcl_ResetResult() */
  ចំនួនគត់ rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  ខណៈពេល( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  សម្រាប់(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->pNext){
    ចំនួនគត់ n = pPreStmt->nSql;
    ប្រសិនបើ( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      ប្រសិនបើ( pPreStmt->pPrev ){
        pPreStmt->pPrev->pNext = pPreStmt->pNext;
      }បើមិនដូច្នេះ{
        pDb->stmtList = pPreStmt->pNext;
      }
      ប្រសិនបើ( pPreStmt->pNext ){
        pPreStmt->pNext->pPrev = pPreStmt->pPrev;
      }បើមិនដូច្នេះ{
        pDb->stmtLast = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_count(pStmt);
      bha_nga;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  ប្រសិនបើ( pPreStmt==0 ){
    ចំនួនគត់ nByte;

    ប្រសិនបើ( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      ត្រឡប់ TCL_ERROR;
    }
    ប្រសិនបើ( pStmt==0 ){
      ប្រសិនបើ( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ត្រឡប់ TCL_ERROR;
      }បើមិនដូច្នេះ{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        ត្រឡប់ TCL_OK;
      }
    }

    assert( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_count(pStmt);
    nByte = parimaa_na(SqlPreparedStmt) + nVar*parimaa_na(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (ចំនួនគត់)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    ប្រសិនបើ( pPreStmt->zSql==0 ){
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
  សម្រាប់(i=1; i<=nVar; i++){
    const var_na *zVar = sqlite3_bind_parameter_name(pStmt, i);
    ប្រសិនបើ( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      ប្រសិនបើ( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        ចំនួនគត់ rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_IncrRefCount(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        ប្រសិនបើ( needResultReset ) Tcl_ResetResult(interp);
        needResultReset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_DecrRefCount(pCmd);
        ប្រសិនបើ( rx==TCL_OK ){
          pVar = Tcl_GetObjResult(interp);
        }បើមិនដូច្នេះ ប្រសិនបើ( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          bha_nga;
        }បើមិនដូច្នេះ{
          pVar = 0;
        }
      }
      ប្រសិនបើ( pVar ){
        Tcl_Size n;
        u8 *data;
        const var_na *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        ប្រសិនបើ( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          data = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, data, n, SQLITE_STATIC);
          Tcl_IncrRefCount(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }បើមិនដូច្នេះ ប្រសិនបើ( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          ចំនួនគត់ nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }បើមិនដូច្នេះ ប្រសិនបើ( c=='d' && strcmp(zType,"double")==0 ){
          dviguu_na r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }បើមិនដូច្នេះ ប្រសិនបើ( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }បើមិនដូច្នេះ{
          data = (unsigned var_na *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (var_na *)data, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_IncrRefCount(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }បើមិនដូច្នេះ{
        sqlite3_bind_null(pStmt, i);
      }
      ប្រសិនបើ( needResultReset ) Tcl_ResetResult(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  ប្រសិនបើ( needResultReset && rc==TCL_OK ) Tcl_ResetResult(pDb->interp);

  ត្រឡប់ rc;
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
  ចំនួនគត់ discard                     /* True to delete (not cache) the pPreStmt */
){
  ចំនួនគត់ i;

  /* Free the bound string and blob parameters */
  សម្រាប់(i=0; i<pPreStmt->nParm; i++){
    Tcl_DecrRefCount(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  ប្រសិនបើ( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }បើមិនដូច្នេះ{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->pNext = pDb->stmtList;
    pPreStmt->pPrev = 0;
    ប្រសិនបើ( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    ប្រសិនបើ( pDb->stmtLast==0 ){
      assert( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }បើមិនដូច្នេះ{
      assert( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    ខណៈពេល( pDb->nStmt>pDb->maxStmt ){
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
  ចំនួនគត់ nCol;                       /* Number of columns returned by pStmt */
  ចំនួនគត់ evalFlags;                  /* Flags used */
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
  ប្រសិនបើ( p->apColName ){
    ចំនួនគត់ i;
    សម្រាប់(i=0; i<p->nCol; i++){
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
  ចំនួនគត់ evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, parimaa_na(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_IncrRefCount(pSql);
  ប្រសិនបើ( pVarName ){
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
  ចំនួនគត់ *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  ប្រសិនបើ( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    ចំនួនគត់ i;                        /* Iterator variable */
    ចំនួនគត់ nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_count(pStmt);
    ប្រសិនបើ( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( parimaa_na(Tcl_Obj*)*nCol );
      សម្រាប់(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_IncrRefCount(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    ប្រសិនបើ( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_IncrRefCount(pColList);
      Tcl_IncrRefCount(pStar);
      សម្រាប់(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      ប្រសិនបើ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }បើមិនដូច្នេះ{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, NULL, 0);
        ប្រសិនបើ( !pDict ){
          pDict = Tcl_NewDictObj();
        }បើមិនដូច្នេះ ប្រសិនបើ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ប្រសិនបើ( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, NULL, pDict, 0);
        }
        Tcl_BounceRefCount(pDict);
      }
      Tcl_DecrRefCount(pStar);
      Tcl_DecrRefCount(pColList);
    }
  }

  ប្រសិនបើ( papColName ){
    *papColName = p->apColName;
  }
  ប្រសិនបើ( pnCol ){
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
static ចំនួនគត់ dbEvalStep(DbEvalContext *p){
  const var_na *zPrevSql = 0;       /* Previous value of p->zSql */

  ខណៈពេល( p->zSql[0] || p->pPreStmt ){
    ចំនួនគត់ rc;
    ប្រសិនបើ( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      ប្រសិនបើ( rc!=TCL_OK ) ត្រឡប់ rc;
    }បើមិនដូច្នេះ{
      ចំនួនគត់ rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      ប្រសិនបើ( rcs==SQLITE_ROW ){
        ត្រឡប់ TCL_OK;
      }
      ប្រសិនបើ( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->nIndex = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      ប្រសិនបើ( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#ប្រសិនបើ SQLITE_TEST
        ប្រសិនបើ( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
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
        ត្រឡប់ TCL_ERROR;
      }បើមិនដូច្នេះ{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  ត្រឡប់ TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
static void dbEvalFinalize(DbEvalContext *p){
  ប្រសិនបើ( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  ប្រសិនបើ( p->pVarName ){
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
static Tcl_Obj *dbEvalColumnValue(DbEvalContext *p, ចំនួនគត់ iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  switch( sqlite3_column_type(pStmt, iCol) ){
    pak_sa SQLITE_BLOB: {
      ចំនួនគត់ bytes = sqlite3_column_bytes(pStmt, iCol);
      const var_na *zBlob = sqlite3_column_blob(pStmt, iCol);
      ប្រសិនបើ( !zBlob ) bytes = 0;
      ត្រឡប់ Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    pak_sa SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      ប្រសិនបើ( v>=-2147483647 && v<=2147483647 ){
        ត្រឡប់ Tcl_NewIntObj((ចំនួនគត់)v);
      }បើមិនដូច្នេះ{
        ត្រឡប់ Tcl_NewWideIntObj(v);
      }
    }
    pak_sa SQLITE_FLOAT: {
      ត្រឡប់ Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    pak_sa SQLITE_NULL: {
      ត្រឡប់ Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  ត្រឡប់ Tcl_NewStringObj((var_na*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#ប្រសិនបើ TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
static ចំនួនគត់ DbUseNre(void){
  ចំនួនគត់ major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  ត្រឡប់( (major==8 && minor>=6) || major>8 );
}
#បើមិនដូច្នេះ
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
static ចំនួនគត់ SQLITE_TCLAPI DbEvalNextCmd(
  ClientData data[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  ចំនួនគត់ result                           /* Result so far */
){
  ចំនួនគត់ rc = result;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)data[0];
  Tcl_Obj * const pScript = (Tcl_Obj *)data[1];
  Tcl_Obj * const pVarName = p->pVarName;

  ខណៈពេល( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    ចំនួនគត់ i;
    ចំនួនគត់ nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    សម្រាប់(i=0; i<nCol; i++){
      ប្រសិនបើ( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, dbEvalColumnValue(p,i), 0);
      }បើមិនដូច្នេះ ប្រសិនបើ( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        ប្រសិនបើ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }បើមិនដូច្នេះ{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, NULL, 0);
          ប្រសិនបើ( pDict ){
            ប្រសិនបើ( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            ប្រសិនបើ( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, NULL, pDict, 0);
            }
            Tcl_BounceRefCount(pDict);
          }
        }
      }បើមិនដូច្នេះ ប្រសិនបើ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       dbEvalColumnValue(p,i), 0);
      }បើមិនដូច្នេះ{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, NULL, 0);
        ប្រសិនបើ( !pDict ){
          pDict = Tcl_NewDictObj();
        }បើមិនដូច្នេះ ប្រសិនបើ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ប្រសិនបើ( Tcl_DictObjPut(interp, pDict, apColName[i],
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
    ប្រសិនបើ( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbEvalNextCmd, (void*)p, (void*)pScript, 0, 0);
      ត្រឡប់ Tcl_NREvalObj(interp, pScript, 0);
    }បើមិនដូច្នេះ{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_DecrRefCount(pScript);
  dbEvalFinalize(p);
  Tcl_Free((var_na *)p);

  ប្រសិនបើ( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_ResetResult(interp);
    rc = TCL_OK;
  }
  ត្រឡប់ rc;
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

  ប្រសិនបើ( *ppHook ){
    Tcl_SetObjResult(interp, *ppHook);
    ប្រសិនបើ( pArg ){
      Tcl_DecrRefCount(*ppHook);
      *ppHook = 0;
    }
  }
  ប្រសិនបើ( pArg ){
    assert( !(*ppHook) );
    ប្រសិនបើ( Tcl_GetString(pArg)[0] ){
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
static ចំនួនគត់ dbQrf(SqliteDb *pDb, ចំនួនគត់ objc, Tcl_Obj *const*objv){
#ifndef SQLITE_QRF_H
  Tcl_SetResult(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  ត្រឡប់ TCL_ERROR;
#បើមិនដូច្នេះ
  var_na *zResult = 0;             /* Result to be returned */
  const var_na *zSql = 0;          /* SQL to run */
  ចំនួនគត់ i;                         /* Loop counter */
  ចំនួនគត់ rc;                        /* Result code */
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
  សម្រាប់(i=2; i<objc; i++){
    const var_na *zArg = Tcl_GetString(objv[i]);
    const var_na *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    const unsigned var_na aBoolMap[] = { 0, 2, 1, 2, 1 };
    ប្រសិនបើ( zArg[0]!='-' ){
      ប្រសិនបើ( zSql ){
        Tcl_AppendResult(pDb->interp, "unknown argument: ", zArg, (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      zSql  = zArg;
    }បើមិនដូច្នេះ ប្រសិនបើ( i==objc-1 ){
      Tcl_AppendResult(pDb->interp, "option has no argument: ", zArg,(var_na*)0);
      rc = TCL_ERROR;
      goto format_failed;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-style")==0 ){
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
      ចំនួនគត់ style;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      ប្រសិនបើ( rc ) goto format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-esc")==0 ){
      static const var_na *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      static unsigned var_na aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_Symbol
      };
      ចំនួនគត់ esc;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      ប្រសិនបើ( rc ) goto format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
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
      ចំនួនគត់ txt;
      ចំនួនគត់ k = zArg[2]=='e';
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      ប្រសិនបើ( rc ) goto format_failed;
      ប្រសិនបើ( k ){
        qrf.eText = aTextMap[txt];
      }បើមិនដូច្នេះ ប្រសិនបើ( txt<=2 ){
        assert( QRF_No==1 );
        assert( QRF_Yes==2 );
        assert( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }បើមិនដូច្នេះ{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-blob")==0 ){
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
      ចំនួនគត់ blob;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      ប្រសិនបើ( rc ) goto format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-wordwrap")==0 ){
      ចំនួនគត់ v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      ប្រសិនបើ( rc ) goto format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      ចំនួនគត់ v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      ប្រសិនបើ( rc ) goto format_failed;
      ប្រសិនបើ( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }បើមិនដូច្នេះ ប្រសិនបើ( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }បើមិនដូច្នេះ ប្រសិនបើ( zArg[1]=='r' ){
        qrf.bRowCount = aBoolMap[v];
      }បើមិនដូច្នេះ{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      ចំនួនគត់ ax = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      ប្រសិនបើ( rc ) goto format_failed;
      ប្រសិនបើ( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }បើមិនដូច្នេះ{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      ចំនួនគត់ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ប្រសិនបើ( rc ) goto format_failed;
      ប្រសិនបើ( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }បើមិនដូច្នេះ ប្រសិនបើ( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      ប្រសិនបើ( zArg[1]=='w' ){
        qrf.nWrap = v;
      }បើមិនដូច្នេះ ប្រសិនបើ( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }បើមិនដូច្នេះ ប្រសិនបើ( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }បើមិនដូច្នេះ{
        qrf.nLineLimit = v;
      }
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-charlimit")==0 ){
      ចំនួនគត់ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ប្រសិនបើ( rc ) goto format_failed;
      ប្រសិនបើ( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-multiinsert")==0 ){
      ចំនួនគត់ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ប្រសិនបើ( rc ) goto format_failed;
      ប្រសិនបើ( v<0 ) v = 0;
      qrf.nMultiInsert = v;
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-align")==0 ){
      Tcl_Size n = 0;
      ចំនួនគត់ jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      ប្រសិនបើ( rc ) goto format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*parimaa_na(qrf.aAlign[0]) );
      ប្រសិនបើ( qrf.aAlign==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*parimaa_na(qrf.aAlign[0]));
      qrf.nAlign = n;
      សម្រាប់(jj=0; jj<n; jj++){
        ចំនួនគត់ x;
        Tcl_Obj *pTerm;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        ប្រសិនបើ( rc ) goto format_failed;
        rc = Tcl_GetIndexFromObj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        ប្រសិនបើ( rc ) goto format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-widths")==0 ){
      Tcl_Size n = 0;
      ចំនួនគត់ jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      ប្រសិនបើ( rc ) goto format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*parimaa_na(qrf.aWidth[0]) );
      ប្រសិនបើ( qrf.aWidth==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*parimaa_na(qrf.aWidth[0]));
      qrf.nWidth = n;
      សម្រាប់(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        ចំនួនគត់ v;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        ប្រសិនបើ( rc ) goto format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        ប្រសិនបើ( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }បើមិនដូច្នេះ ប្រសិនបើ( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short ចំនួនគត់)v;
      }
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-ifmt")==0 ){
      var_na *zFmt = Tcl_GetString(objv[i+1]);
      ប្រសិនបើ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_AppendResult(pDb->interp, "bad format for -ifmt", (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-fpfmt")==0 ){
      var_na *zFmt = Tcl_GetString(objv[i+1]);
      ប្រសិនបើ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }បើមិនដូច្នេះ ប្រសិនបើ( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_AppendResult(pDb->interp, "bad format for -fpfmt", (var_na*)0);
        rc = TCL_ERROR;
        goto format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }បើមិនដូច្នេះ{
      Tcl_AppendResult(pDb->interp, "unknown option: ", zArg, (var_na*)0);
      rc = TCL_ERROR;
      goto format_failed;
    }
  }
  ខណៈពេល( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    var_na *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    ប្រសិនបើ( rc ) goto format_failed;
    ប្រសិនបើ( pStmt==0 ) continue;
    rc = sqlite3_format_query_result(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    ប្រសិនបើ( rc ){
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
  ត្រឡប់ rc;

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
static ចំនួនគត់ SQLITE_TCLAPI DbObjCmd(
  void *cd,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *const*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  ចំនួនគត់ choice;
  ចំនួនគត់ rc = TCL_OK;
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

  ប្រសិនបើ( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    ត្រឡប់ TCL_ERROR;
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
    ត្រឡប់ TCL_ERROR;
#បើមិនដូច្នេះ
    ប្រសិនបើ( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ត្រឡប់ TCL_ERROR;
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==2 ){
      ប្រសិនបើ( pDb->zAuth ){
        Tcl_AppendResult(interp, pDb->zAuth, (var_na*)0);
      }
    }បើមិនដូច្នេះ{
      var_na *zAuth;
      Tcl_Size len;
      ប្រសិនបើ( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      ប្រសិនបើ( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }បើមិនដូច្នេះ{
        pDb->zAuth = 0;
      }
      ប្រសិនបើ( pDb->zAuth ){
        typedef ចំនួនគត់ (*sqlite3_auth_cb)(
           void*,ចំនួនគត់,const var_na*,const var_na*,
           const var_na*,const var_na*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }បើមិនដូច្នេះ{
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

    ប្រសិនបើ( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }បើមិនដូច្នេះ{
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? FILENAME");
      ត្រឡប់ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    ប្រសិនបើ( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (var_na*)0);
      sqlite3_close(pDest);
      ត្រឡប់ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    ប្រសិនបើ( pBackup==0 ){
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (var_na*)0);
      sqlite3_close(pDest);
      ត្រឡប់ TCL_ERROR;
    }
    ខណៈពេល(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    ប្រសិនបើ( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ត្រឡប់ TCL_ERROR;
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==2 ){
      ប្រសិនបើ( pDb->zBindFallback ){
        Tcl_AppendResult(interp, pDb->zBindFallback, (var_na*)0);
      }
    }បើមិនដូច្នេះ{
      var_na *zCallback;
      Tcl_Size len;
      ប្រសិនបើ( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      ប្រសិនបើ( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "CALLBACK");
      ត្រឡប់ TCL_ERROR;
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==2 ){
      ប្រសិនបើ( pDb->zBusy ){
        Tcl_AppendResult(interp, pDb->zBusy, (var_na*)0);
      }
    }បើមិនដូច្នេះ{
      var_na *zBusy;
      Tcl_Size len;
      ប្រសិនបើ( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      ប្រសិនបើ( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }បើមិនដូច្នេះ{
        pDb->zBusy = 0;
      }
      ប្រសិនបើ( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }បើមិនដូច្នេះ{
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
    ចំនួនគត់ n;

    ប្រសិនបើ( objc<=2 ){
      Tcl_WrongNumArgs(interp, 1, objv, "cache option ?arg?");
      ត្រឡប់ TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    ប្រសិនបើ( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      ប្រសិនបើ( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "flush");
        ត្រឡប់ TCL_ERROR;
      }បើមិនដូច្នេះ{
        flushStmtCache( pDb );
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      ប្រសិនបើ( objc!=4 ){
        Tcl_WrongNumArgs(interp, 2, objv, "size n");
        ត្រឡប់ TCL_ERROR;
      }បើមិនដូច្នេះ{
        ប្រសិនបើ( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_AppendResult( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (var_na*)0);
          ត្រឡប់ TCL_ERROR;
        }បើមិនដូច្នេះ{
          ប្រសិនបើ( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }បើមិនដូច្នេះ ប្រសិនបើ( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }បើមិនដូច្នេះ{
      Tcl_AppendResult( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (var_na*)0);
      ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( objc!=4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NAME SCRIPT");
      ត្រឡប់ TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( parimaa_na(*pCollate) + nScript + 1 );
    ប្រសិនបើ( pCollate==0 ) ត្រឡប់ TCL_ERROR;
    pCollate->interp = interp;
    pCollate->pNext = pDb->pCollate;
    pCollate->zScript = (var_na*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    ប្រសិនបើ( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_SetResult(interp, (var_na *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SCRIPT");
      ត្រឡប់ TCL_ERROR;
    }
    ប្រសិនបើ( pDb->pCollateNeeded ){
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
    ប្រសិនបើ( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ត្រឡប់ TCL_ERROR;
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==2 ){
      ប្រសិនបើ( pDb->zCommit ){
        Tcl_AppendResult(interp, pDb->zCommit, (var_na*)0);
      }
    }បើមិនដូច្នេះ{
      const var_na *zCommit;
      Tcl_Size len;
      ប្រសិនបើ( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      ប្រសិនបើ( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }បើមិនដូច្នេះ{
        pDb->zCommit = 0;
      }
      ប្រសិនបើ( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }បើមិនដូច្នេះ{
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
    ចំនួនគត់ isComplete;
    ប្រសិនបើ( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SQL");
      ត្រឡប់ TCL_ERROR;
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
      ចំនួនគត់ op;
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
    ចំនួនគត់ ii;
    ប្រសិនបើ( objc>4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      ត្រឡប់ TCL_ERROR;
    }
    ប្រសិនបើ( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      pResult = Tcl_NewListObj(0,0);
      សម្រាប់(ii=0; ii<parimaa_na(aDbConfig)/parimaa_na(aDbConfig[0]); ii++){
        ចំនួនគត់ v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewIntObj(v));
      }
    }បើមិនដូច្នេះ{
      const var_na *zOpt = Tcl_GetString(objv[2]);
      ចំនួនគត់ onoff = -1;
      ចំនួនគត់ v = 0;
      ប្រសិនបើ( zOpt[0]=='-' ) zOpt++;
      សម្រាប់(ii=0; ii<parimaa_na(aDbConfig)/parimaa_na(aDbConfig[0]); ii++){
        ប្រសិនបើ( strcmp(aDbConfig[ii].zName, zOpt)==0 ) bha_nga;
      }
      ប្រសិនបើ( ii>=parimaa_na(aDbConfig)/parimaa_na(aDbConfig[0]) ){
        Tcl_AppendResult(interp, "unknown config option: \"", zOpt,
                                "\"", (void*)0);
        ត្រឡប់ TCL_ERROR;
      }
      ប្រសិនបើ( objc==4 ){
        ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          ត្រឡប់ TCL_ERROR;
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
    ចំនួនគត់ nCol;                   /* Number of columns in the table */
    ចំនួនគត់ nByte;                  /* Number of bytes in an SQL string */
    ចំនួនគត់ i, j;                   /* Loop counters */
    ចំនួនគត់ nSep;                   /* Number of bytes in zSep[] */
    ចំនួនគត់ nNull;                  /* Number of bytes in zNull[] */
    var_na *zSql;                 /* An SQL statement */
    var_na *zLine;                /* A single line of input from the file */
    var_na **azCol;               /* zLine[] broken up into columns */
    const var_na *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    ចំនួនគត់ lineno = 0;             /* Line number of input file */
    var_na zLineNum[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *pResult;           /* interp result */

    const var_na *zSep;
    const var_na *zNull;
    ប្រសិនបើ( objc<5 || objc>7 ){
      Tcl_WrongNumArgs(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      ត្រឡប់ TCL_ERROR;
    }
    ប្រសិនបើ( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }បើមិនដូច្នេះ{
      zSep = "\t";
    }
    ប្រសិនបើ( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }បើមិនដូច្នេះ{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    ប្រសិនបើ( nSep==0 ){
      Tcl_AppendResult(interp,"Error: non-null separator required for copy",
                       (var_na*)0);
      ត្រឡប់ TCL_ERROR;
    }
    ប្រសិនបើ(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_AppendResult(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (var_na*)0);
      ត្រឡប់ TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    ប្រសិនបើ( zSql==0 ){
      Tcl_AppendResult(interp, "Error: no such table: ", zTable, (var_na*)0);
      ត្រឡប់ TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    ប្រសិនបើ( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (var_na*)0);
      nCol = 0;
    }បើមិនដូច្នេះ{
      nCol = sqlite3_column_count(pStmt);
    }
    sqlite3_finalize(pStmt);
    ប្រសិនបើ( nCol==0 ) {
      ត្រឡប់ TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    ប្រសិនបើ( zSql==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (var_na*)0);
      ត្រឡប់ TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    សម្រាប់(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    free(zSql);
    ប្រសិនបើ( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (var_na*)0);
      sqlite3_finalize(pStmt);
      ត្រឡប់ TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    ប្រសិនបើ( in==0 ){
      sqlite3_finalize(pStmt);
      ត្រឡប់ TCL_ERROR;
    }
    Tcl_SetChannelOption(NULL, in, "-translation", "auto");
    azCol = malloc( parimaa_na(azCol[0])*(nCol+1) );
    ប្រសិនបើ( azCol==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (var_na*)0);
      Tcl_Close(interp, in);
      ត្រឡប់ TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_IncrRefCount(str);
    (void)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    ខណៈពេល( Tcl_GetsObj(in, str)>=0 ) {
      var_na *z;
      Tcl_Size byteLen;
      lineno++;
      zLine = (var_na *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = zLine;
      សម្រាប់(i=0, z=zLine; *z; z++){
        ប្រសិនបើ( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          ប្រសិនបើ( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      ប្រសិនបើ( i+1!=nCol ){
        var_na *zErr;
        ចំនួនគត់ nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        ប្រសិនបើ( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_AppendResult(interp, zErr, (var_na*)0);
          free(zErr);
        }
        zCommit = "ROLLBACK";
        bha_nga;
      }
      សម្រាប់(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        ប្រសិនបើ( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }បើមិនដូច្នេះ{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_SetObjLength(str, 0);
      ប្រសិនបើ( rc!=SQLITE_OK ){
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

    ប្រសិនបើ( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      pResult = Tcl_GetObjResult(interp);
      ប្រសិនបើ( Tcl_IsShared(pResult) ){
        pResult = Tcl_DuplicateObj(pResult);
      }
      Tcl_SetIntObj(pResult, lineno);
      rc = TCL_OK;
    }បើមិនដូច្នេះ{
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
#បើមិនដូច្នេះ
    const var_na *zSchema = 0;
    Tcl_Obj *pValue = 0;
    unsigned var_na *pBA;
    unsigned var_na *pData;
    Tcl_Size len;
    ចំនួនគត់ xrc;
    sqlite3_int64 mxSize = 0;
    ចំនួនគត់ i;
    ចំនួនគត់ isReadonly = 0;


    ប្រសិនបើ( objc<3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      bha_nga;
    }
    សម្រាប់(i=2; i<objc-1; i++){
      const var_na *z = Tcl_GetString(objv[i]);
      ប្រសិនបើ( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        ប្រសិនបើ( rc ) goto deserialize_error;
        mxSize = x;
        continue;
      }
      ប្រសិនបើ( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        ប្រសិនបើ( rc ) goto deserialize_error;
        continue;
      }
      ប្រសិនបើ( zSchema==0 && i==objc-2 && z[0]!='-' ){
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
    ប្រសិនបើ( pData==0 && len>0 ){
      Tcl_AppendResult(interp, "out of memory", (var_na*)0);
      rc = TCL_ERROR;
    }បើមិនដូច្នេះ{
      ចំនួនគត់ flags;
      ប្រសិនបើ( len>0 ) memcpy(pData, pBA, len);
      ប្រសិនបើ( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }បើមិនដូច្នេះ{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, pData, len, len, flags);
      ប្រសិនបើ( xrc ){
        Tcl_AppendResult(interp, "unable to set MEMDB content", (var_na*)0);
        rc = TCL_ERROR;
      }
      ប្រសិនបើ( mxSize>0 ){
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
    ចំនួនគត់ onoff;
    ប្រសិនបើ( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "BOOLEAN");
      ត្រឡប់ TCL_ERROR;
    }
    ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      ត្រឡប់ TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    bha_nga;
#បើមិនដូច្នេះ
    Tcl_AppendResult(interp, "extension loading is turned off at compile-time",
                     (var_na*)0);
    ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SQL");
      ត្រឡប់ TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    ប្រសិនបើ( choice==DB_ONECOLUMN ){
      ប្រសិនបើ( rc==TCL_OK ){
        pResult = dbEvalColumnValue(&sEval, 0);
      }បើមិនដូច្នេះ ប្រសិនបើ( rc==TCL_BREAK ){
        Tcl_ResetResult(interp);
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( rc==TCL_BREAK || rc==TCL_OK ){
      pResult = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    ប្រសិនបើ( pResult ) Tcl_SetObjResult(interp, pResult);

    ប្រសិនបើ( rc==TCL_BREAK ){
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
    ចំនួនគត់ evalFlags = 0;
    const var_na *zOpt;
    ខណៈពេល( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      ប្រសិនបើ( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }បើមិនដូច្នេះ{
        Tcl_AppendResult(interp, "unknown option: \"", zOpt, "\"", (void*)0);
        ត្រឡប់ TCL_ERROR;
      }
      objc--;
      objv++;
    }
    ប្រសិនបើ( objc<3 || objc>5 ){
      Tcl_WrongNumArgs(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      ត្រឡប់ TCL_ERROR;
    }

    ប្រសិនបើ( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_IncrRefCount(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      ខណៈពេល( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        ចំនួនគត់ i;
        ចំនួនគត់ nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        សម្រាប់(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, dbEvalColumnValue(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      ប្រសិនបើ( rc==TCL_BREAK ){
        Tcl_SetObjResult(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_DecrRefCount(pRet);
    }បើមិនដូច្នេះ{
      ClientData cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      ប្រសិនបើ( objc>=5 && *(var_na *)Tcl_GetString(objv[3]) ){
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
    ចំនួនគត់ flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    var_na *zName;
    ចំនួនគត់ nArg = -1;
    ចំនួនគត់ i;
    ចំនួនគត់ eType = SQLITE_NULL;
    ប្រសិនបើ( objc<4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      ត្រឡប់ TCL_ERROR;
    }
    សម្រាប់(i=3; i<(objc-1); i++){
      const var_na *z = Tcl_GetString(objv[i]);
      ចំនួនគត់ n = strlen30(z);
      ប្រសិនបើ( n>1 && strncmp(z, "-argcount",n)==0 ){
        ប្រសិនបើ( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(var_na*)0);
          ត្រឡប់ TCL_ERROR;
        }
        ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) ត្រឡប់ TCL_ERROR;
        ប្រសិនបើ( nArg<0 ){
          Tcl_AppendResult(interp, "number of arguments must be non-negative",
                           (var_na*)0);
          ត្រឡប់ TCL_ERROR;
        }
        i++;
      }បើមិនដូច្នេះ
      ប្រសិនបើ( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }បើមិនដូច្នេះ
      ប្រសិនបើ( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }បើមិនដូច្នេះ
      ប្រសិនបើ( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }បើមិនដូច្នេះ
      ប្រសិនបើ( n>1 && strncmp(z, "-returntype", n)==0 ){
        const var_na *azType[] = {"integer", "real", "text", "blob", "any", 0};
        assert( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        assert( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        ប្រសិនបើ( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(var_na*)0);
          ត្រឡប់ TCL_ERROR;
        }
        i++;
        ប្រសិនបើ( Tcl_GetIndexFromObj(interp, objv[i], azType, "type", 0, &eType) ){
          ត្រឡប់ TCL_ERROR;
        }
        eType++;
      }បើមិនដូច្នេះ{
        Tcl_AppendResult(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (var_na*)0
        );
        ត្រឡប់ TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    ប្រសិនបើ( pFunc==0 ) ត្រឡប់ TCL_ERROR;
    ប្រសិនបើ( pFunc->pScript ){
      Tcl_DecrRefCount(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_IncrRefCount(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    ប្រសិនបើ( rc!=SQLITE_OK ){
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
    ត្រឡប់ TCL_ERROR;
#បើមិនដូច្នេះ
    ចំនួនគត់ isReadonly = 0;
    const var_na *zDb = "main";
    const var_na *zTable;
    const var_na *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    ប្រសិនបើ( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    ប្រសិនបើ( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_WrongNumArgs(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      ត្រឡប់ TCL_ERROR;
    }

    ប្រសិនបើ( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    ប្រសិនបើ( rc==TCL_OK ){
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
    ប្រសិនបើ( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "NULLVALUE");
      ត្រឡប់ TCL_ERROR;
    }
    ប្រសិនបើ( objc==3 ){
      Tcl_Size len;
      var_na *zNull = Tcl_GetStringFromObj(objv[2], &len);
      ប្រសិនបើ( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      ប្រសិនបើ( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( objc==2 ){
      ប្រសិនបើ( pDb->zProgress ){
        Tcl_AppendResult(interp, pDb->zProgress, (var_na*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==4 ){
      var_na *zProgress;
      Tcl_Size len;
      ចំនួនគត់ N;
      ប្រសិនបើ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        ត្រឡប់ TCL_ERROR;
      };
      ប្រសិនបើ( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      ប្រសិនបើ( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }បើមិនដូច្នេះ{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      ប្រសិនបើ( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }បើមិនដូច្នេះ{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }បើមិនដូច្នេះ{
      Tcl_WrongNumArgs(interp, 2, objv, "N CALLBACK");
      ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ត្រឡប់ TCL_ERROR;
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==2 ){
      ប្រសិនបើ( pDb->zProfile ){
        Tcl_AppendResult(interp, pDb->zProfile, (var_na*)0);
      }
    }បើមិនដូច្នេះ{
      var_na *zProfile;
      Tcl_Size len;
      ប្រសិនបើ( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      ប្រសិនបើ( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }បើមិនដូច្នេះ{
        pDb->zProfile = 0;
      }
#ប្រសិនបើ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ប្រសិនបើ( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "KEY");
      ត្រឡប់ TCL_ERROR;
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
    ចំនួនគត់ nTimeout = 0;

    ប្រសិនបើ( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }បើមិនដូច្នេះ{
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE? FILENAME");
      ត្រឡប់ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    ប្រសិនបើ( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (var_na*)0);
      sqlite3_close(pSrc);
      ត្រឡប់ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    ប្រសិនបើ( pBackup==0 ){
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (var_na*)0);
      sqlite3_close(pSrc);
      ត្រឡប់ TCL_ERROR;
    }
    ខណៈពេល( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      ប្រសិនបើ( rc==SQLITE_BUSY ){
        ប្រសិនបើ( nTimeout++ >= 3 ) bha_nga;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    ប្រសិនបើ( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }បើមិនដូច្នេះ ប្រសិនបើ( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_AppendResult(interp, "restore failed: source database busy",
                       (var_na*)0);
      rc = TCL_ERROR;
    }បើមិនដូច្នេះ{
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
#បើមិនដូច្នេះ
    const var_na *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned var_na *pData;
    ប្រសិនបើ( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }បើមិនដូច្នេះ{
      ចំនួនគត់ needFree;
      pData = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      ប្រសិនបើ( pData ){
        needFree = 0;
      }បើមិនដូច្នេះ{
        pData = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(pData,sz));
      ប្រសិនបើ( needFree ) sqlite3_free(pData);
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
    ចំនួនគត់ v;
    const var_na *zOp;
    ប្រសិនបើ( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "(step|sort|autoindex)");
      ត្រឡប់ TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    ប្រសិនបើ( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zOp, "autoindex")==0 ){
      v = pDb->nIndex;
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }បើមិនដូច្នេះ{
      Tcl_AppendResult(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (var_na*)0);
      ត្រឡប់ TCL_ERROR;
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
    ចំនួនគត់ ms;
    ប្រសិនបើ( objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "MILLISECONDS");
      ត្រឡប់ TCL_ERROR;
    }
    ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[2], &ms) ) ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( objc!=2 ){
      Tcl_WrongNumArgs(interp, 2, objv, "");
      ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( objc>3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK?");
      ត្រឡប់ TCL_ERROR;
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==2 ){
      ប្រសិនបើ( pDb->zTrace ){
        Tcl_AppendResult(interp, pDb->zTrace, (var_na*)0);
      }
    }បើមិនដូច្នេះ{
      var_na *zTrace;
      Tcl_Size len;
      ប្រសិនបើ( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      ប្រសិនបើ( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }បើមិនដូច្នេះ{
        pDb->zTrace = 0;
      }
#ប្រសិនបើ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ប្រសិនបើ( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( objc>4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?CALLBACK? ?MASK?");
      ត្រឡប់ TCL_ERROR;
    }បើមិនដូច្នេះ ប្រសិនបើ( objc==2 ){
      ប្រសិនបើ( pDb->zTraceV2 ){
        Tcl_AppendResult(interp, pDb->zTraceV2, (var_na*)0);
      }
    }បើមិនដូច្នេះ{
      var_na *zTraceV2;
      Tcl_Size len;
      Tcl_WideInt wMask = 0;
      ប្រសិនបើ( objc==4 ){
        static const var_na *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        ga_nanaa TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_Size i;
        ប្រសិនបើ( TCL_OK!=Tcl_ListObjLength(interp, objv[3], &len) ){
          ត្រឡប់ TCL_ERROR;
        }
        សម្រាប់(i=0; i<len; i++){
          Tcl_Obj *pObj;
          ចំនួនគត់ ttype;
          ប្រសិនបើ( TCL_OK!=Tcl_ListObjIndex(interp, objv[3], i, &pObj) ){
            ត្រឡប់ TCL_ERROR;
          }
          ប្រសិនបើ( Tcl_GetIndexFromObj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
            Tcl_IncrRefCount(pError);
            ប្រសិនបើ( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_DecrRefCount(pError);
              wMask |= wType;
            }បើមិនដូច្នេះ{
              Tcl_SetObjResult(interp, pError);
              Tcl_DecrRefCount(pError);
              ត្រឡប់ TCL_ERROR;
            }
          }បើមិនដូច្នេះ{
            switch( (ga_nanaa TTYPE_enum)ttype ){
              pak_sa TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    bha_nga;
              pak_sa TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; bha_nga;
              pak_sa TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     bha_nga;
              pak_sa TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   bha_nga;
            }
          }
        }
      }បើមិនដូច្នេះ{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      ប្រសិនបើ( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      ប្រសិនបើ( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }បើមិនដូច្នេះ{
        pDb->zTraceV2 = 0;
      }
#ប្រសិនបើ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      ប្រសិនបើ( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }បើមិនដូច្នេះ{
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
    ប្រសិនបើ( objc!=3 && objc!=4 ){
      Tcl_WrongNumArgs(interp, 2, objv, "[TYPE] SCRIPT");
      ត្រឡប់ TCL_ERROR;
    }

    ប្រសិនបើ( pDb->nTransaction==0 && objc==4 ){
      static const var_na *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      ga_nanaa TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      ចំនួនគត់ ttype;
      ប្រសិនបើ( Tcl_GetIndexFromObj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (var_na*)0);
      ត្រឡប់ TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    ប្រសិនបើ( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (void)Tcl_NREvalObj(interp, pScript, 0);
    }បើមិនដូច្នេះ{
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
#បើមិនដូច្នេះ
    ប្រសិនបើ( objc!=2 && objc!=3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }បើមិនដូច្នេះ{
      void (*xNotify)(void **, ចំនួនគត់) = 0;
      void *pNotifyArg = 0;

      ប្រសិនបើ( pDb->pUnlockNotify ){
        Tcl_DecrRefCount(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      ប្រសិនបើ( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (void *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_IncrRefCount(pDb->pUnlockNotify);
      }

      ប្រសិនបើ( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
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
#បើមិនដូច្នេះ
    static const var_na *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    ga_nanaa DbPreupdateSubCmd {
      PRE_COUNT, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    ចំនួនគត់ iSub;

    ប្រសិនបើ( objc<3 ){
      Tcl_WrongNumArgs(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    ប្រសិនបើ( Tcl_GetIndexFromObj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      ត្រឡប់ TCL_ERROR;
    }

    switch( (ga_nanaa DbPreupdateSubCmd)iSub ){
      pak_sa PRE_COUNT: {
        ចំនួនគត់ nCol = sqlite3_preupdate_count(pDb->db);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nCol));
        bha_nga;
      }

      pak_sa PRE_HOOK: {
        ប្រសិនបើ( objc>4 ){
          Tcl_WrongNumArgs(interp, 2, objv, "hook ?SCRIPT?");
          ត្រឡប់ TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        bha_nga;
      }

      pak_sa PRE_DEPTH: {
        Tcl_Obj *pRet;
        ប្រសិនបើ( objc!=3 ){
          Tcl_WrongNumArgs(interp, 3, objv, "");
          ត្រឡប់ TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_SetObjResult(interp, pRet);
        bha_nga;
      }

      pak_sa PRE_NEW:
      pak_sa PRE_OLD: {
        ចំនួនគត់ iIdx;
        sqlite3_value *pValue;
        ប្រសិនបើ( objc!=4 ){
          Tcl_WrongNumArgs(interp, 3, objv, "INDEX");
          ត្រឡប់ TCL_ERROR;
        }
        ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          ត្រឡប់ TCL_ERROR;
        }

        ប្រសិនបើ( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &pValue);
        }បើមិនដូច្នេះ{
          assert( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &pValue);
        }

        ប្រសិនបើ( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((var_na*)sqlite3_value_text(pValue), -1);
          Tcl_SetObjResult(interp, pObj);
        }បើមិនដូច្នេះ{
          Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (var_na*)0);
          ត្រឡប់ TCL_ERROR;
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
    ប្រសិនបើ( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    ប្រសិនបើ( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    ប្រសិនបើ( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    ប្រសិនបើ( objc>3 ){
       Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
       ត្រឡប់ TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    bha_nga;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  pak_sa DB_VERSION: {
    ចំនួនគត់ i;
    សម្រាប់(i=2; i<objc; i++){
      const var_na *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      ប្រសិនបើ( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          ត្រឡប់ TCL_ERROR;
        }
      }បើមិនដូច្នេះ

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      ប្រសិនបើ( strcmp(zArg, "-last-stmt-ptr")==0 ){
        extern const var_na *sqlite3TestPtrToText(void*);
        void *ptr = (void*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_SetResult(interp, (var_na*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }បើមិនដូច្នេះ
#endif /* SQLITE_TEST */
      {
        Tcl_AppendResult(interp, "unknown argument: ", zArg, (var_na*)0);
        ត្រឡប់ TCL_ERROR;
      }
    }
    ប្រសិនបើ( i==2 ){   
      Tcl_SetResult(interp, (var_na *)sqlite3_libversion(), TCL_STATIC);
    }
    bha_nga;
  }


  } /* End of the SWITCH statement */
  ត្រឡប់ rc;
}

#ប្រសិនបើ SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
static ចំនួនគត់ SQLITE_TCLAPI DbObjCmdAdaptor(
  void *cd,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *const*objv
){
  ត្រឡប់ Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
static ចំនួនគត់ sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *const*objv
){
  Tcl_WrongNumArgs(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  ត្រឡប់ TCL_ERROR;
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
static ចំនួនគត់ SQLITE_TCLAPI DbMain(
  void *cd,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *const*objv
){
  SqliteDb *p;
  const var_na *zArg;
  var_na *zErrMsg;
  ចំនួនគត់ i;
  const var_na *zFile = 0;
  const var_na *zVfs = 0;
  ចំនួនគត់ flags;
  ចំនួនគត់ bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  ចំនួនគត់ rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#បើមិនដូច្នេះ
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  ប្រសិនបើ( objc==1 ) ត្រឡប់ sqliteCmdUsage(interp, objv);
  ប្រសិនបើ( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    ប្រសិនបើ( strcmp(zArg,"-version")==0 ){
      Tcl_AppendResult(interp,sqlite3_libversion(), (var_na*)0);
      ត្រឡប់ TCL_OK;
    }
    ប្រសិនបើ( strcmp(zArg,"-sourceid")==0 ){
      Tcl_AppendResult(interp,sqlite3_sourceid(), (var_na*)0);
      ត្រឡប់ TCL_OK;
    }
    ប្រសិនបើ( strcmp(zArg,"-has-codec")==0 ){
      Tcl_AppendResult(interp,"0",(var_na*)0);
      ត្រឡប់ TCL_OK;
    }
    ប្រសិនបើ( zArg[0]=='-' ) ត្រឡប់ sqliteCmdUsage(interp, objv);
  }
  សម្រាប់(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    ប្រសិនបើ( zArg[0]!='-' ){
      ប្រសិនបើ( zFile!=0 ) ត្រឡប់ sqliteCmdUsage(interp, objv);
      zFile = zArg;
      continue;
    }
    ប្រសិនបើ( i==objc-1 ) ត្រឡប់ sqliteCmdUsage(interp, objv);
    i++;
    ប្រសិនបើ( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg, "-readonly")==0 ){
      ចំនួនគត់ b;
      ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ត្រឡប់ TCL_ERROR;
      ប្រសិនបើ( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }បើមិនដូច្នេះ{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg, "-create")==0 ){
      ចំនួនគត់ b;
      ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ត្រឡប់ TCL_ERROR;
      ប្រសិនបើ( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }បើមិនដូច្នេះ{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg, "-nofollow")==0 ){
      ចំនួនគត់ b;
      ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ត្រឡប់ TCL_ERROR;
      ប្រសិនបើ( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }បើមិនដូច្នេះ{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg, "-nomutex")==0 ){
      ចំនួនគត់ b;
      ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ត្រឡប់ TCL_ERROR;
      ប្រសិនបើ( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }បើមិនដូច្នេះ{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg, "-fullmutex")==0 ){
      ចំនួនគត់ b;
      ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ត្រឡប់ TCL_ERROR;
      ប្រសិនបើ( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }បើមិនដូច្នេះ{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg, "-uri")==0 ){
      ចំនួនគត់ b;
      ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ត្រឡប់ TCL_ERROR;
      ប្រសិនបើ( b ){
        flags |= SQLITE_OPEN_URI;
      }បើមិនដូច្នេះ{
        flags &= ~SQLITE_OPEN_URI;
      }
    }បើមិនដូច្នេះ ប្រសិនបើ( strcmp(zArg, "-translatefilename")==0 ){
      ប្រសិនបើ( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        ត្រឡប់ TCL_ERROR;
      }
    }បើមិនដូច្នេះ{
      Tcl_AppendResult(interp, "unknown option: ", zArg, (var_na*)0);
      ត្រឡប់ TCL_ERROR;
    }
  }
  zErrMsg = 0;
  p = (SqliteDb*)Tcl_Alloc( parimaa_na(*p) );
  memset(p, 0, parimaa_na(*p));
  ប្រសិនបើ( zFile==0 ) zFile = "";
  ប្រសិនបើ( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  ប្រសិនបើ( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  ប្រសិនបើ( p->db ){
    ប្រសិនបើ( SQLITE_OK!=sqlite3_errcode(p->db) ){
      zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }បើមិនដូច្នេះ{
    zErrMsg = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  ប្រសិនបើ( p->db==0 ){
    Tcl_SetResult(interp, zErrMsg, TCL_VOLATILE);
    Tcl_Free((var_na*)p);
    sqlite3_free(zErrMsg);
    ត្រឡប់ TCL_ERROR;
  }
  p->maxStmt = NUM_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  ប្រសិនបើ( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (var_na*)p, DbDeleteCmd);
  }បើមិនដូច្នេះ{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (var_na*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  ត្រឡប់ TCL_OK;
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
EXTERN ចំនួនគត់ Sqlite3_Init(Tcl_Interp *interp){
  ចំនួនគត់ rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  ប្រសិនបើ( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  ត្រឡប់ rc;
}
EXTERN ចំនួនគត់ Tclsqlite3_Init(Tcl_Interp *interp){ ត្រឡប់ Sqlite3_Init(interp); }
EXTERN ចំនួនគត់ Sqlite3_Unload(Tcl_Interp *interp, ចំនួនគត់ flags){ ត្រឡប់ TCL_OK; }
EXTERN ចំនួនគត់ Tclsqlite3_Unload(Tcl_Interp *interp, ចំនួនគត់ flags){ ត្រឡប់ TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN ចំនួនគត់ Sqlite3_SafeInit(Tcl_Interp *interp){ ត្រឡប់ TCL_ERROR; }
EXTERN ចំនួនគត់ Sqlite3_SafeUnload(Tcl_Interp *interp, ចំនួនគត់ flags){ត្រឡប់ TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN ចំនួនគត់ Sqlite_Init(Tcl_Interp *interp){ ត្រឡប់ Sqlite3_Init(interp);}
EXTERN ចំនួនគត់ Tclsqlite_Init(Tcl_Interp *interp){ ត្រឡប់ Sqlite3_Init(interp); }
EXTERN ចំនួនគត់ Sqlite_Unload(Tcl_Interp *interp, ចំនួនគត់ flags){ ត្រឡប់ TCL_OK; }
EXTERN ចំនួនគត់ Tclsqlite_Unload(Tcl_Interp *interp, ចំនួនគត់ flags){ ត្រឡប់ TCL_OK; }
EXTERN ចំនួនគត់ Sqlite_SafeInit(Tcl_Interp *interp){ ត្រឡប់ TCL_ERROR; }
EXTERN ចំនួនគត់ Sqlite_SafeUnload(Tcl_Interp *interp, ចំនួនគត់ flags){ត្រឡប់ TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN ចំនួនគត់ sqlite3_Init(Tcl_Interp *interp){ ត្រឡប់ Sqlite3_Init(interp);}
EXTERN ចំនួនគត់ sqlite_Init(Tcl_Interp *interp){ ត្រឡប់ Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#ប្រសិនបើ defined(TCLSH)

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
  ត្រឡប់ zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN main
#endif
ចំនួនគត់ SQLITE_CDECL TCLSH_MAIN(ចំនួនគត់ argc, var_na **argv){
  Tcl_Interp *interp;
  ចំនួនគត់ i;
  const var_na *zScript = 0;
  var_na zArgc[32];
#ប្រសិនបើ defined(TCLSH_INIT_PROC)
  extern const var_na *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#ប្រសិនបើ !defined(_WIN32_WCE)
  ប្រសិនបើ( getenv("SQLITE_DEBUG_BREAK") ){
    ប្រសិនបើ( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }បើមិនដូច្នេះ{
#ប្រសិនបើ defined(_WIN32) || defined(WIN32)
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
  សម្រាប់(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_VALUE);
  }
#ប្រសិនបើ defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  ប្រសិនបើ( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  ប្រសិនបើ( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    const var_na *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    ប្រសិនបើ( zInfo==0 ) zInfo = Tcl_GetStringResult(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    ត្រឡប់ 1;
  }
  ត្រឡប់ 0;
}
#endif /* TCLSH */
