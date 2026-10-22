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
#എങ്കിൽ defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#എങ്കിൽ defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#അല്ലെങ്കിൽ
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#എങ്കിൽ TCL_MAJOR_VERSION==9
# define CONST സ്ഥിരം
#അല്ലെങ്കിലെങ്കിൽ !defined(Tcl_Size)
  തരനിർവചനം പൂർണ്ണസംഖ്യ Tcl_Size;
# ifndef Tcl_Bounce_Ref_എണ്ണം
#  define Tcl_Bounce_Ref_എണ്ണം(X) Tcl_Incr_Ref_എണ്ണം(X); Tcl_Decr_Ref_എണ്ണം(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#endif
/**** End copy of tclsqlite.h ****/

#അന്തര്ഭാവ <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ifndef SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  തരനിർവചനം unsigned അക്ഷരം u8;
# ifndef SQLITE_PTRSIZE
#   എങ്കിൽ defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   അല്ലെങ്കിൽ
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# എങ്കിൽ defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    തരനിർവചനം uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    തരനിർവചനം unsigned പൂർണ്ണസംഖ്യ uptr;
# അല്ലെങ്കിൽ
    തരനിർവചനം sqlite3_uint64 uptr;
# endif
#endif
#അന്തര്ഭാവ <ctype.h>

/* Used to get the current process ID */
#എങ്കിൽ !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#അല്ലെങ്കിലെങ്കിൽ !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (പൂർണ്ണസംഖ്യ)GetCurrentProcessId
#endif

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ifdef BUILD_sqlite
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_sqlite */

#define അക്കം_PREPARED_STMTS 10
#define MAX_PREPARED_STMTS 100

/* Forward declaration */
തരനിർവചനം ഘടന SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#എങ്കിൽ defined(SQLITE_ENABLE_QRF_IN_TCL)
#അന്തര്ഭാവ "qrf.h"
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
തരനിർവചനം ഘടന SqlFunc SqlFunc;
ഘടന SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  പൂർണ്ണസംഖ്യ useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  പൂർണ്ണസംഖ്യ eType;            /* Type of value to return */
  അക്ഷരം *zName;          /* Name of this function */
  SqlFunc *pNext;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
തരനിർവചനം ഘടന SqlCollate SqlCollate;
ഘടന SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  അക്ഷരം *zScript;        /* The script to be run */
  SqlCollate *pNext;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
തരനിർവചനം ഘടന SqlPreparedStmt SqlPreparedStmt;
ഘടന SqlPreparedStmt {
  SqlPreparedStmt *pNext;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  പൂർണ്ണസംഖ്യ nSql;                /* chars in zSql[] */
  സ്ഥിരം അക്ഷരം *zSql;        /* Text of the SQL statement */
  പൂർണ്ണസംഖ്യ nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

തരനിർവചനം ഘടന IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
ഘടന SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  അക്ഷരം *zBusy;               /* The busy callback routine */
  അക്ഷരം *zCommit;             /* The commit hook callback routine */
  അക്ഷരം *zTrace;              /* The trace callback routine */
  അക്ഷരം *zTraceV2;            /* The trace_v2 callback routine */
  അക്ഷരം *zProfile;            /* The profile callback routine */
  അക്ഷരം *zProgress;           /* The progress callback routine */
  അക്ഷരം *zBindFallback;       /* Callback to invoke on a binding miss */
  അക്ഷരം *zAuth;               /* The authorization callback routine */
  പൂർണ്ണസംഖ്യ disableAuth;           /* Disable the authorizer if it exists */
  അക്ഷരം *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  പൂർണ്ണസംഖ്യ rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  പൂർണ്ണസംഖ്യ maxStmt;               /* The next maximum number of stmtList */
  പൂർണ്ണസംഖ്യ nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  പൂർണ്ണസംഖ്യ nStep, nSort, nIndex;  /* Statistics for most recent operation */
  പൂർണ്ണസംഖ്യ nVMStep;               /* Another statistic for most recent operation */
  പൂർണ്ണസംഖ്യ nTransaction;          /* Number of nested [transaction] methods */
  പൂർണ്ണസംഖ്യ openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  പൂർണ്ണസംഖ്യ nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  പൂർണ്ണസംഖ്യ bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

ഘടന IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned പൂർണ്ണസംഖ്യ isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *pNext;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ strlen30(സ്ഥിരം അക്ഷരം *z){
  സ്ഥിരം അക്ഷരം *z2 = z;
  ആകുമ്പോൾ( *z2 ){ z2++; }
  തിരികെ 0x3fffffff & (പൂർണ്ണസംഖ്യ)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
സ്ഥിതം ശൂന്യം closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *pNext;

  വേണ്ടി(p=pDb->pIncrblob; p; p=pNext){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI incrblobClose2(
  ClientData instanceData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ flags
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  പൂർണ്ണസംഖ്യ  rc;
  sqlite3 *db = p->pDb->db;

  എങ്കിൽ( flags ){
    p->isClosed |= flags;
    തിരികെ TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  എങ്കിൽ( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  എങ്കിൽ( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }
  എങ്കിൽ( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->pNext;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((അക്ഷരം *)p);

  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (അക്ഷരം *)sqlite3_errmsg(db), TCL_VOLATILE);
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI incrblobClose(
  ClientData instanceData,
  Tcl_Interp *interp
){
  തിരികെ incrblobClose2(instanceData, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI incrblob_നിവേശം(
  ClientData instanceData,
  അക്ഷരം *buf,
  പൂർണ്ണസംഖ്യ bufSize,
  പൂർണ്ണസംഖ്യ *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nRead = bufSize;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  പൂർണ്ണസംഖ്യ rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  എങ്കിൽ( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  എങ്കിൽ( nRead<=0 ){
    തിരികെ 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (ശൂന്യം *)buf, (പൂർണ്ണസംഖ്യ)nRead, (പൂർണ്ണസംഖ്യ)p->iSeek);
  എങ്കിൽ( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    തിരികെ -1;
  }

  p->iSeek += nRead;
  തിരികെ nRead;
}

/*
** Write data to an incremental blob channel.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI incrblobOutput(
  ClientData instanceData,
  സ്ഥിരം അക്ഷരം *buf,
  പൂർണ്ണസംഖ്യ toWrite,
  പൂർണ്ണസംഖ്യ *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  പൂർണ്ണസംഖ്യ rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  എങ്കിൽ( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    തിരികെ -1;
  }
  എങ്കിൽ( nWrite<=0 ){
    തിരികെ 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (ശൂന്യം*)buf,(പൂർണ്ണസംഖ്യ)nWrite, (പൂർണ്ണസംഖ്യ)p->iSeek);
  എങ്കിൽ( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    തിരികെ -1;
  }

  p->iSeek += nWrite;
  തിരികെ nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#എങ്കിൽ TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#അല്ലെങ്കിൽ
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
സ്ഥിതം WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  ClientData instanceData,
  WideSeekProcType offset,
  പൂർണ്ണസംഖ്യ seekMode,
  പൂർണ്ണസംഖ്യ *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;

  തിരഞ്ഞെടുപ്പ്( seekMode ){
    അവസ്ഥ SEEK_SET:
      p->iSeek = offset;
      നിർത്തുക;
    അവസ്ഥ SEEK_CUR:
      p->iSeek += offset;
      നിർത്തുക;
    അവസ്ഥ SEEK_END:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി: പ്രതിജ്ഞാ(!"Bad seekMode");
  }

  തിരികെ p->iSeek;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI incrblobSeek(
  ClientData instanceData,
  long offset,
  പൂർണ്ണസംഖ്യ seekMode,
  പൂർണ്ണസംഖ്യ *errorCodePtr
){
  തിരികെ incrblobWideSeek(instanceData,offset,seekMode,errorCodePtr);
}


സ്ഥിതം ശൂന്യം SQLITE_TCLAPI incrblobWatch(
  ClientData instanceData,
  പൂർണ്ണസംഖ്യ mode
){
  /* NO-OP */
}
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI incrblobHandle(
  ClientData instanceData,
  പൂർണ്ണസംഖ്യ dir,
  ClientData *hPtr
){
  തിരികെ TCL_ERROR;
}

സ്ഥിതം Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_VERSION_5,             /* version                              */
  incrblobClose,                     /* closeProc                            */
  incrblob_നിവേശം,                     /* inputProc                            */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  സ്ഥിരം അക്ഷരം *zDb,
  സ്ഥിരം അക്ഷരം *zTable,
  സ്ഥിരം അക്ഷരം *zColumn,
  sqlite_int64 iRow,
  പൂർണ്ണസംഖ്യ isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  സ്ഥിതം പൂർണ്ണസംഖ്യ എണ്ണം = 0;
  അക്ഷരം zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (അക്ഷരം *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    തിരികെ TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(വലുപ്പം(IncrblobChannel));
  memset(p, 0, വലുപ്പം(*p));
  p->pBlob = pBlob;
  എങ്കിൽ( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(വലുപ്പം(zChannel), zChannel, "incrblob_%d", ++എണ്ണം);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->pNext = pDb->pIncrblob;
  p->pPrev = 0;
  എങ്കിൽ( p->pNext ){
    p->pNext->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_SetResult(interp, (അക്ഷരം *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  തിരികെ TCL_OK;
}
#അല്ലെങ്കിൽ  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  സ്ഥിരം അക്ഷരം *z;
  Tcl_Size n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  ആകുമ്പോൾ( n-- > 0 ){
    പൂർണ്ണസംഖ്യ c = *(z++);
    എങ്കിൽ( c=='$' || c=='[' || c==';' ) തിരികെ 0;
  }
  തിരികെ 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
സ്ഥിതം SqlFunc *findSqlFunc(SqliteDb *pDb, സ്ഥിരം അക്ഷരം *zName){
  SqlFunc *p, *pNew;
  പൂർണ്ണസംഖ്യ nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( വലുപ്പം(*pNew) + nName + 1 );
  pNew->zName = (അക്ഷരം*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  വേണ്ടി(p=pDb->pFunc; p; p=p->pNext){
    എങ്കിൽ( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((അക്ഷരം*)pNew);
      തിരികെ p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->pNext = pDb->pFunc;
  pDb->pFunc = pNew;
  തിരികെ pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
സ്ഥിതം ശൂന്യം dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  എങ്കിൽ( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((അക്ഷരം *)pStmt->zSql);
  }
#endif
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((അക്ഷരം *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
സ്ഥിതം ശൂന്യം flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *pNext;

  വേണ്ടി(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pNext){
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
സ്ഥിതം ശൂന്യം addDatabaseRef(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
സ്ഥിതം ശൂന്യം delDatabaseRef(SqliteDb *pDb){
  പ്രതിജ്ഞാ( pDb->nRef>0 );
  pDb->nRef--;
  എങ്കിൽ( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    ആകുമ്പോൾ( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->pNext;
      പ്രതിജ്ഞാ( pFunc->pDb==pDb );
      Tcl_Decr_Ref_എണ്ണം(pFunc->pScript);
      Tcl_Free((അക്ഷരം*)pFunc);
    }
    ആകുമ്പോൾ( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->pNext;
      Tcl_Free((അക്ഷരം*)pCollate);
    }
    എങ്കിൽ( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    എങ്കിൽ( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    എങ്കിൽ( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    എങ്കിൽ( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    എങ്കിൽ( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    എങ്കിൽ( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    എങ്കിൽ( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    എങ്കിൽ( pDb->pUpdateHook ){
      Tcl_Decr_Ref_എണ്ണം(pDb->pUpdateHook);
    }
    എങ്കിൽ( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_എണ്ണം(pDb->pPreUpdateHook);
    }
    എങ്കിൽ( pDb->pRollbackHook ){
      Tcl_Decr_Ref_എണ്ണം(pDb->pRollbackHook);
    }
    എങ്കിൽ( pDb->pWalHook ){
      Tcl_Decr_Ref_എണ്ണം(pDb->pWalHook);
    }
    എങ്കിൽ( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_എണ്ണം(pDb->pCollateNeeded);
    }
    Tcl_Free((അക്ഷരം*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
സ്ഥിതം ശൂന്യം SQLITE_TCLAPI DbDeleteCmd(ശൂന്യം *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ DbBusyHandler(ശൂന്യം *cd, പൂർണ്ണസംഖ്യ nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  പൂർണ്ണസംഖ്യ rc;
  അക്ഷരം zVal[30];

  sqlite3_snprintf(വലുപ്പം(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (അക്ഷരം*)0);
  എങ്കിൽ( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    തിരികെ 0;
  }
  തിരികെ 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ DbProgressHandler(ശൂന്യം *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  പൂർണ്ണസംഖ്യ rc;

  പ്രതിജ്ഞാ( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  എങ്കിൽ( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    തിരികെ 1;
  }
  തിരികെ 0;
}
#endif

#എങ്കിൽ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
സ്ഥിതം ശൂന്യം DbTraceHandler(ശൂന്യം *cd, സ്ഥിരം അക്ഷരം *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_മൂല്യം(&str));
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
സ്ഥിതം പൂർണ്ണസംഖ്യ DbTraceV2Handler(
  unsigned type, /* One of the SQLITE_TRACE_* event types. */
  ശൂന്യം *cd,      /* The original context data pointer. */
  ശൂന്യം *pd,      /* Primary event data, depends on event type. */
  ശൂന്യം *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  തിരഞ്ഞെടുപ്പ്( type ){
    അവസ്ഥ SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      അക്ഷരം *zSql = (അക്ഷരം *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_എണ്ണം(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_എണ്ണം(pCmd);
      Tcl_ResetResult(pDb->interp);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_എണ്ണം(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_എണ്ണം(pCmd);
      Tcl_ResetResult(pDb->interp);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_എണ്ണം(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_എണ്ണം(pCmd);
      Tcl_ResetResult(pDb->interp);
      നിർത്തുക;
    }
    അവസ്ഥ SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_എണ്ണം(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_എണ്ണം(pCmd);
      Tcl_ResetResult(pDb->interp);
      നിർത്തുക;
    }
  }
  തിരികെ SQLITE_OK;
}
#endif

#എങ്കിൽ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
സ്ഥിതം ശൂന്യം DbProfileHandler(ശൂന്യം *cd, സ്ഥിരം അക്ഷരം *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  അക്ഷരം zTm[100];

  sqlite3_snprintf(വലുപ്പം(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_മൂല്യം(&str));
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
സ്ഥിതം പൂർണ്ണസംഖ്യ DbCommitHandler(ശൂന്യം *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  പൂർണ്ണസംഖ്യ rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  എങ്കിൽ( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    തിരികെ 1;
  }
  തിരികെ 0;
}

സ്ഥിതം ശൂന്യം DbRollbackHandler(ശൂന്യം *clientData){
  SqliteDb *pDb = (SqliteDb*)clientData;
  പ്രതിജ്ഞാ(pDb->pRollbackHook);
  എങ്കിൽ( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ DbWalHandler(
  ശൂന്യം *clientData,
  sqlite3 *db,
  സ്ഥിരം അക്ഷരം *zDb,
  പൂർണ്ണസംഖ്യ nEntry
){
  പൂർണ്ണസംഖ്യ ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)clientData;
  Tcl_Interp *interp = pDb->interp;
  പ്രതിജ്ഞാ(pDb->pWalHook);

  പ്രതിജ്ഞാ( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_എണ്ണം(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  എങ്കിൽ( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_എണ്ണം(p);

  തിരികെ ret;
}

#എങ്കിൽ defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
സ്ഥിതം ശൂന്യം setTestUnlockNotifyVars(Tcl_Interp *interp, പൂർണ്ണസംഖ്യ iArg, പൂർണ്ണസംഖ്യ nArg){
  അക്ഷരം zBuf[64];
  sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(വലുപ്പം(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#അല്ലെങ്കിൽ
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
സ്ഥിതം ശൂന്യം DbUnlockNotify(ശൂന്യം **apArg, പൂർണ്ണസംഖ്യ nArg){
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<nArg; i++){
    സ്ഥിരം പൂർണ്ണസംഖ്യ flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    setTestUnlockNotifyVars(pDb->interp, i, nArg);
    പ്രതിജ്ഞാ( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_എണ്ണം(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
സ്ഥിതം ശൂന്യം DbPreUpdateHandler(
  ശൂന്യം *p,
  sqlite3 *db,
  പൂർണ്ണസംഖ്യ op,
  സ്ഥിരം അക്ഷരം *zDb,
  സ്ഥിരം അക്ഷരം *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  സ്ഥിതം സ്ഥിരം അക്ഷരം *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  പ്രതിജ്ഞാ( (SQLITE_DELETE-1)/9 == 0 );
  പ്രതിജ്ഞാ( (SQLITE_തിരുകുക-1)/9 == 1 );
  പ്രതിജ്ഞാ( (SQLITE_UPDATE-1)/9 == 2 );
  പ്രതിജ്ഞാ( pDb->pPreUpdateHook );
  പ്രതിജ്ഞാ( db==pDb->db );
  പ്രതിജ്ഞാ( op==SQLITE_തിരുകുക || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_എണ്ണം(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_എണ്ണം(pCmd);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

സ്ഥിതം ശൂന്യം DbUpdateHandler(
  ശൂന്യം *p,
  പൂർണ്ണസംഖ്യ op,
  സ്ഥിരം അക്ഷരം *zDb,
  സ്ഥിരം അക്ഷരം *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  സ്ഥിതം സ്ഥിരം അക്ഷരം *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  പ്രതിജ്ഞാ( (SQLITE_DELETE-1)/9 == 0 );
  പ്രതിജ്ഞാ( (SQLITE_തിരുകുക-1)/9 == 1 );
  പ്രതിജ്ഞാ( (SQLITE_UPDATE-1)/9 == 2 );

  പ്രതിജ്ഞാ( pDb->pUpdateHook );
  പ്രതിജ്ഞാ( op==SQLITE_തിരുകുക || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_എണ്ണം(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_എണ്ണം(pCmd);
}

സ്ഥിതം ശൂന്യം tclCollateNeeded(
  ശൂന്യം *pCtx,
  sqlite3 *db,
  പൂർണ്ണസംഖ്യ enc,
  സ്ഥിരം അക്ഷരം *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_എണ്ണം(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_എണ്ണം(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tclSqlCollate(
  ശൂന്യം *pCtx,
  പൂർണ്ണസംഖ്യ nA,
  സ്ഥിരം ശൂന്യം *zA,
  പൂർണ്ണസംഖ്യ nB,
  സ്ഥിരം ശൂന്യം *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_എണ്ണം(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_എണ്ണം(pCmd);
  തിരികെ (atoi(Tcl_GetStringResult(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
സ്ഥിതം ശൂന്യം tclSqlFunc(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം**argv){
  SqlFunc *p = sqlite3_user_data(context);
  Tcl_Obj *pCmd;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_എണ്ണം(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_എണ്ണം(pCmd);
  }അല്ലെങ്കിൽ{
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
    എങ്കിൽ( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
      തിരികെ;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_എണ്ണം(pCmd);
    വേണ്ടി(i=0; i<argc; i++){
      sqlite3_മൂല്യം *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      തിരഞ്ഞെടുപ്പ്( sqlite3_മൂല്യം_type(pIn) ){
        അവസ്ഥ SQLITE_BLOB: {
          പൂർണ്ണസംഖ്യ bytes = sqlite3_മൂല്യം_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_മൂല്യം_blob(pIn), bytes);
          നിർത്തുക;
        }
        അവസ്ഥ SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_മൂല്യം_int64(pIn);
          എങ്കിൽ( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((പൂർണ്ണസംഖ്യ)v);
          }അല്ലെങ്കിൽ{
            pVal = Tcl_NewWideIntObj(v);
          }
          നിർത്തുക;
        }
        അവസ്ഥ SQLITE_FLOAT: {
          ഇരട്ട r = sqlite3_മൂല്യം_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          നിർത്തുക;
        }
        അവസ്ഥ SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          നിർത്തുക;
        }
        സ്ഥിരസ്ഥിതി: {
          പൂർണ്ണസംഖ്യ bytes = sqlite3_മൂല്യം_bytes(pIn);
          pVal = Tcl_NewStringObj((അക്ഷരം *)sqlite3_മൂല്യം_text(pIn), bytes);
          നിർത്തുക;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      എങ്കിൽ( rc ){
        Tcl_Decr_Ref_എണ്ണം(pCmd);
        sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
        തിരികെ;
      }
    }
    എങ്കിൽ( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_Decr_Ref_എണ്ണം(pCmd);
  }

  എങ്കിൽ( TCL_BREAK==rc ){
    sqlite3_result_null(context);
  }അല്ലെങ്കിൽ എങ്കിൽ( rc && rc!=TCL_RETURN ){
    sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
  }അല്ലെങ്കിൽ{
    Tcl_Obj *pVar = Tcl_GetObjResult(p->interp);
    Tcl_Size n;
    u8 *data;
    സ്ഥിരം അക്ഷരം *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    അക്ഷരം c = zType[0];
    പൂർണ്ണസംഖ്യ eType = p->eType;

    എങ്കിൽ( eType==SQLITE_NULL ){
      എങ്കിൽ( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }അല്ലെങ്കിൽ എങ്കിൽ( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }അല്ലെങ്കിൽ എങ്കിൽ( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }അല്ലെങ്കിൽ{
        eType = SQLITE_TEXT;
      }
    }

    തിരഞ്ഞെടുപ്പ്( eType ){
      അവസ്ഥ SQLITE_BLOB: {
        data = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_result_blob64(context, data, n, SQLITE_TRANSIENT);
        നിർത്തുക;
      }
      അവസ്ഥ SQLITE_INTEGER: {
        Tcl_WideInt v;
        എങ്കിൽ( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_result_int64(context, v);
          നിർത്തുക;
        }
        /* fall-through */
      }
      അവസ്ഥ SQLITE_FLOAT: {
        ഇരട്ട r;
        എങ്കിൽ( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_result_double(context, r);
          നിർത്തുക;
        }
        /* fall-through */
      }
      സ്ഥിരസ്ഥിതി: {
        data = (unsigned അക്ഷരം *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_result_text64(context, (അക്ഷരം *)data, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        നിർത്തുക;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ auth_callback(
  ശൂന്യം *pArg,
  പൂർണ്ണസംഖ്യ code,
  സ്ഥിരം അക്ഷരം *zArg1,
  സ്ഥിരം അക്ഷരം *zArg2,
  സ്ഥിരം അക്ഷരം *zArg3,
  സ്ഥിരം അക്ഷരം *zArg4
){
  സ്ഥിരം അക്ഷരം *zCode;
  Tcl_DString str;
  പൂർണ്ണസംഖ്യ rc;
  സ്ഥിരം അക്ഷരം *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  എങ്കിൽ( pDb->disableAuth ) തിരികെ SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  തിരഞ്ഞെടുപ്പ്( code ){
    അവസ്ഥ SQLITE_COPY              : zCode="SQLITE_COPY"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_INDEX      : zCode="SQLITE_CREATE_INDEX"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_TEMP_INDEX : zCode="SQLITE_CREATE_TEMP_INDEX"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_TEMP_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_TEMP_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_TEMP_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; നിർത്തുക;
    അവസ്ഥ SQLITE_DELETE            : zCode="SQLITE_DELETE"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_INDEX        : zCode="SQLITE_DROP_INDEX"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_TEMP_INDEX   : zCode="SQLITE_DROP_TEMP_INDEX"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_TEMP_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_TEMP_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_TEMP_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; നിർത്തുക;
    അവസ്ഥ SQLITE_തിരുകുക            : zCode="SQLITE_INSERT"; നിർത്തുക;
    അവസ്ഥ SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; നിർത്തുക;
    അവസ്ഥ SQLITE_READ              : zCode="SQLITE_READ"; നിർത്തുക;
    അവസ്ഥ SQLITE_SELECT            : zCode="SQLITE_SELECT"; നിർത്തുക;
    അവസ്ഥ SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; നിർത്തുക;
    അവസ്ഥ SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; നിർത്തുക;
    അവസ്ഥ SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; നിർത്തുക;
    അവസ്ഥ SQLITE_DETACH            : zCode="SQLITE_DETACH"; നിർത്തുക;
    അവസ്ഥ SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; നിർത്തുക;
    അവസ്ഥ SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; നിർത്തുക;
    അവസ്ഥ SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; നിർത്തുക;
    അവസ്ഥ SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; നിർത്തുക;
    അവസ്ഥ SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; നിർത്തുക;
    അവസ്ഥ SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; നിർത്തുക;
    അവസ്ഥ SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; നിർത്തുക;
    അവസ്ഥ SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; നിർത്തുക;
    സ്ഥിരസ്ഥിതി                       : zCode="????"; നിർത്തുക;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, zCode);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_മൂല്യം(&str));
  Tcl_DStringFree(&str);
  zReply = rc==TCL_OK ? Tcl_GetStringResult(pDb->interp) : "SQLITE_DENY";
  എങ്കിൽ( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }അല്ലെങ്കിൽ{
    rc = 999;
  }
  തിരികെ rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#എങ്കിൽ 0
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
സ്ഥിതം അക്ഷരം *local_getline(അക്ഷരം *zPrompt, FILE *in){
  അക്ഷരം *z_വരി;
  പൂർണ്ണസംഖ്യ n_വരി;
  പൂർണ്ണസംഖ്യ n;

  n_വരി = 100;
  z_വരി = malloc( n_വരി );
  എങ്കിൽ( z_വരി==0 ) തിരികെ 0;
  n = 0;
  ആകുമ്പോൾ( 1 ){
    എങ്കിൽ( n+100>n_വരി ){
      n_വരി = n_വരി*2 + 100;
      z_വരി = realloc(z_വരി, n_വരി);
      എങ്കിൽ( z_വരി==0 ) തിരികെ 0;
    }
    എങ്കിൽ( fgets(&z_വരി[n], n_വരി - n, in)==0 ){
      എങ്കിൽ( n==0 ){
        സ്വതന്ത്രം(z_വരി);
        തിരികെ 0;
      }
      z_വരി[n] = 0;
      നിർത്തുക;
    }
    ആകുമ്പോൾ( z_വരി[n] ){ n++; }
    എങ്കിൽ( n>0 && z_വരി[n-1]=='\n' ){
      n--;
      z_വരി[n] = 0;
      നിർത്തുക;
    }
  }
  z_വരി = realloc( z_വരി, n+1 );
  തിരികെ z_വരി;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI DbTransPostCmd(
  ClientData data[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  പൂർണ്ണസംഖ്യ result                           /* Result of evaluating SCRIPT */
){
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)data[0];
  പൂർണ്ണസംഖ്യ rc = result;
  സ്ഥിരം അക്ഷരം *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  എങ്കിൽ( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    എങ്കിൽ( rc!=TCL_ERROR ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  തിരികെ rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  സ്ഥിരം അക്ഷരം *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  സ്ഥിരം അക്ഷരം **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned പൂർണ്ണസംഖ്യ prepFlags = 0;
#ifdef SQLITE_TEST
  എങ്കിൽ( pDb->bLegacyPrepare ){
    തിരികെ sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  എങ്കിൽ( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  തിരികെ sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  അക്ഷരം സ്ഥിരം *zIn,                /* SQL to compile */
  അക്ഷരം സ്ഥിരം **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  സ്ഥിരം അക്ഷരം *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  പൂർണ്ണസംഖ്യ nSql;                       /* Length of zSql in bytes */
  പൂർണ്ണസംഖ്യ nVar = 0;                   /* Number of variables in statement */
  പൂർണ്ണസംഖ്യ iParm = 0;                  /* Next free entry in apParm */
  അക്ഷരം c;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ needResultReset = 0;        /* Need to invoke Tcl_ResetResult() */
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  ആകുമ്പോൾ( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  വേണ്ടി(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->pNext){
    പൂർണ്ണസംഖ്യ n = pPreStmt->nSql;
    എങ്കിൽ( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      എങ്കിൽ( pPreStmt->pPrev ){
        pPreStmt->pPrev->pNext = pPreStmt->pNext;
      }അല്ലെങ്കിൽ{
        pDb->stmtList = pPreStmt->pNext;
      }
      എങ്കിൽ( pPreStmt->pNext ){
        pPreStmt->pNext->pPrev = pPreStmt->pPrev;
      }അല്ലെങ്കിൽ{
        pDb->stmtLast = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_എണ്ണം(pStmt);
      നിർത്തുക;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  എങ്കിൽ( pPreStmt==0 ){
    പൂർണ്ണസംഖ്യ nByte;

    എങ്കിൽ( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( pStmt==0 ){
      എങ്കിൽ( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        തിരികെ TCL_ERROR;
      }അല്ലെങ്കിൽ{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        തിരികെ TCL_OK;
      }
    }

    പ്രതിജ്ഞാ( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_എണ്ണം(pStmt);
    nByte = വലുപ്പം(SqlPreparedStmt) + nVar*വലുപ്പം(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (പൂർണ്ണസംഖ്യ)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    എങ്കിൽ( pPreStmt->zSql==0 ){
      അക്ഷരം *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      memcpy(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#endif
  }
  പ്രതിജ്ഞാ( pPreStmt );
  പ്രതിജ്ഞാ( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  പ്രതിജ്ഞാ( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  വേണ്ടി(i=1; i<=nVar; i++){
    സ്ഥിരം അക്ഷരം *zVar = sqlite3_bind_parameter_name(pStmt, i);
    എങ്കിൽ( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      എങ്കിൽ( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        പൂർണ്ണസംഖ്യ rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_എണ്ണം(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        എങ്കിൽ( needResultReset ) Tcl_ResetResult(interp);
        needResultReset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_Decr_Ref_എണ്ണം(pCmd);
        എങ്കിൽ( rx==TCL_OK ){
          pVar = Tcl_GetObjResult(interp);
        }അല്ലെങ്കിൽ എങ്കിൽ( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          നിർത്തുക;
        }അല്ലെങ്കിൽ{
          pVar = 0;
        }
      }
      എങ്കിൽ( pVar ){
        Tcl_Size n;
        u8 *data;
        സ്ഥിരം അക്ഷരം *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        എങ്കിൽ( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          data = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, data, n, SQLITE_STATIC);
          Tcl_Incr_Ref_എണ്ണം(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }അല്ലെങ്കിൽ എങ്കിൽ( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          പൂർണ്ണസംഖ്യ nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }അല്ലെങ്കിൽ എങ്കിൽ( c=='d' && strcmp(zType,"double")==0 ){
          ഇരട്ട r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }അല്ലെങ്കിൽ എങ്കിൽ( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }അല്ലെങ്കിൽ{
          data = (unsigned അക്ഷരം *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (അക്ഷരം *)data, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_എണ്ണം(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }അല്ലെങ്കിൽ{
        sqlite3_bind_null(pStmt, i);
      }
      എങ്കിൽ( needResultReset ) Tcl_ResetResult(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  എങ്കിൽ( needResultReset && rc==TCL_OK ) Tcl_ResetResult(pDb->interp);

  തിരികെ rc;
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
സ്ഥിതം ശൂന്യം dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  പൂർണ്ണസംഖ്യ discard                     /* True to delete (not cache) the pPreStmt */
){
  പൂർണ്ണസംഖ്യ i;

  /* Free the bound string and blob parameters */
  വേണ്ടി(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_എണ്ണം(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  എങ്കിൽ( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }അല്ലെങ്കിൽ{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->pNext = pDb->stmtList;
    pPreStmt->pPrev = 0;
    എങ്കിൽ( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    എങ്കിൽ( pDb->stmtLast==0 ){
      പ്രതിജ്ഞാ( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }അല്ലെങ്കിൽ{
      പ്രതിജ്ഞാ( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    ആകുമ്പോൾ( pDb->nStmt>pDb->maxStmt ){
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
തരനിർവചനം ഘടന DbEvalContext DbEvalContext;
ഘടന DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  സ്ഥിരം അക്ഷരം *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  പൂർണ്ണസംഖ്യ nCol;                       /* Number of columns returned by pStmt */
  പൂർണ്ണസംഖ്യ evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#define SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#define SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
സ്ഥിതം ശൂന്യം dbReleaseColumnNames(DbEvalContext *p){
  എങ്കിൽ( p->apColName ){
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<p->nCol; i++){
      Tcl_Decr_Ref_എണ്ണം(p->apColName[i]);
    }
    Tcl_Free((അക്ഷരം *)p->apColName);
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
സ്ഥിതം ശൂന്യം dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  പൂർണ്ണസംഖ്യ evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, വലുപ്പം(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_എണ്ണം(pSql);
  എങ്കിൽ( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_എണ്ണം(pVarName);
  }
  p->evalFlags = evalFlags;
  addDatabaseRef(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
സ്ഥിതം ശൂന്യം dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  പൂർണ്ണസംഖ്യ *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  എങ്കിൽ( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    പൂർണ്ണസംഖ്യ i;                        /* Iterator variable */
    പൂർണ്ണസംഖ്യ nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_എണ്ണം(pStmt);
    എങ്കിൽ( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( വലുപ്പം(Tcl_Obj*)*nCol );
      വേണ്ടി(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_Incr_Ref_എണ്ണം(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    എങ്കിൽ( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_എണ്ണം(pColList);
      Tcl_Incr_Ref_എണ്ണം(pStar);
      വേണ്ടി(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      എങ്കിൽ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }അല്ലെങ്കിൽ{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, ശൂന്യ, 0);
        എങ്കിൽ( !pDict ){
          pDict = Tcl_NewDictObj();
        }അല്ലെങ്കിൽ എങ്കിൽ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        എങ്കിൽ( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, ശൂന്യ, pDict, 0);
        }
        Tcl_Bounce_Ref_എണ്ണം(pDict);
      }
      Tcl_Decr_Ref_എണ്ണം(pStar);
      Tcl_Decr_Ref_എണ്ണം(pColList);
    }
  }

  എങ്കിൽ( papColName ){
    *papColName = p->apColName;
  }
  എങ്കിൽ( pnCol ){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ dbEvalStep(DbEvalContext *p){
  സ്ഥിരം അക്ഷരം *zPrevSql = 0;       /* Previous value of p->zSql */

  ആകുമ്പോൾ( p->zSql[0] || p->pPreStmt ){
    പൂർണ്ണസംഖ്യ rc;
    എങ്കിൽ( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      എങ്കിൽ( rc!=TCL_OK ) തിരികെ rc;
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      എങ്കിൽ( rcs==SQLITE_ROW ){
        തിരികെ TCL_OK;
      }
      എങ്കിൽ( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->nIndex = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      എങ്കിൽ( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#എങ്കിൽ SQLITE_TEST
        എങ്കിൽ( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          തുടരുക;
        }
#endif
        Tcl_SetObjResult(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        തിരികെ TCL_ERROR;
      }അല്ലെങ്കിൽ{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  തിരികെ TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
സ്ഥിതം ശൂന്യം dbEvalFinalize(DbEvalContext *p){
  എങ്കിൽ( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  എങ്കിൽ( p->pVarName ){
    Tcl_Decr_Ref_എണ്ണം(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_എണ്ണം(p->pSql);
  dbReleaseColumnNames(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
സ്ഥിതം Tcl_Obj *db_Eval_Column_മൂല്യം(DbEvalContext *p, പൂർണ്ണസംഖ്യ iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  തിരഞ്ഞെടുപ്പ്( sqlite3_column_type(pStmt, iCol) ){
    അവസ്ഥ SQLITE_BLOB: {
      പൂർണ്ണസംഖ്യ bytes = sqlite3_column_bytes(pStmt, iCol);
      സ്ഥിരം അക്ഷരം *zBlob = sqlite3_column_blob(pStmt, iCol);
      എങ്കിൽ( !zBlob ) bytes = 0;
      തിരികെ Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    അവസ്ഥ SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      എങ്കിൽ( v>=-2147483647 && v<=2147483647 ){
        തിരികെ Tcl_NewIntObj((പൂർണ്ണസംഖ്യ)v);
      }അല്ലെങ്കിൽ{
        തിരികെ Tcl_NewWideIntObj(v);
      }
    }
    അവസ്ഥ SQLITE_FLOAT: {
      തിരികെ Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    അവസ്ഥ SQLITE_NULL: {
      തിരികെ Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  തിരികെ Tcl_NewStringObj((അക്ഷരം*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#എങ്കിൽ TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
സ്ഥിതം പൂർണ്ണസംഖ്യ DbUseNre(ശൂന്യം){
  പൂർണ്ണസംഖ്യ major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  തിരികെ( (major==8 && minor>=6) || major>8 );
}
#അല്ലെങ്കിൽ
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (ശൂന്യം)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (ശൂന്യം)0
#endif

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI DbEvalNextCmd(
  ClientData data[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  പൂർണ്ണസംഖ്യ result                           /* Result so far */
){
  പൂർണ്ണസംഖ്യ rc = result;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)data[0];
  Tcl_Obj * സ്ഥിരം pScript = (Tcl_Obj *)data[1];
  Tcl_Obj * സ്ഥിരം pVarName = p->pVarName;

  ആകുമ്പോൾ( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    വേണ്ടി(i=0; i<nCol; i++){
      എങ്കിൽ( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, db_Eval_Column_മൂല്യം(p,i), 0);
      }അല്ലെങ്കിൽ എങ്കിൽ( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        എങ്കിൽ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }അല്ലെങ്കിൽ{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ശൂന്യ, 0);
          എങ്കിൽ( pDict ){
            എങ്കിൽ( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            എങ്കിൽ( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, ശൂന്യ, pDict, 0);
            }
            Tcl_Bounce_Ref_എണ്ണം(pDict);
          }
        }
      }അല്ലെങ്കിൽ എങ്കിൽ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       db_Eval_Column_മൂല്യം(p,i), 0);
      }അല്ലെങ്കിൽ{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ശൂന്യ, 0);
        എങ്കിൽ( !pDict ){
          pDict = Tcl_NewDictObj();
        }അല്ലെങ്കിൽ എങ്കിൽ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        എങ്കിൽ( Tcl_DictObjPut(interp, pDict, apColName[i],
                           db_Eval_Column_മൂല്യം(p,i))==TCL_OK ){
          Tcl_ObjSetVar2(interp, pVarName, ശൂന്യ, pDict, 0);
        }
        Tcl_Bounce_Ref_എണ്ണം(pDict);
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
    എങ്കിൽ( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbEvalNextCmd, (ശൂന്യം*)p, (ശൂന്യം*)pScript, 0, 0);
      തിരികെ Tcl_NREvalObj(interp, pScript, 0);
    }അല്ലെങ്കിൽ{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_എണ്ണം(pScript);
  dbEvalFinalize(p);
  Tcl_Free((അക്ഷരം *)p);

  എങ്കിൽ( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_ResetResult(interp);
    rc = TCL_OK;
  }
  തിരികെ rc;
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
സ്ഥിതം ശൂന്യം DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  എങ്കിൽ( *ppHook ){
    Tcl_SetObjResult(interp, *ppHook);
    എങ്കിൽ( pArg ){
      Tcl_Decr_Ref_എണ്ണം(*ppHook);
      *ppHook = 0;
    }
  }
  എങ്കിൽ( pArg ){
    പ്രതിജ്ഞാ( !(*ppHook) );
    എങ്കിൽ( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_എണ്ണം(*ppHook);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ dbQrf(SqliteDb *pDb, പൂർണ്ണസംഖ്യ objc, Tcl_Obj *സ്ഥിരം*objv){
#ifndef SQLITE_QRF_H
  Tcl_SetResult(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  തിരികെ TCL_ERROR;
#അല്ലെങ്കിൽ
  അക്ഷരം *zResult = 0;             /* Result to be returned */
  സ്ഥിരം അക്ഷരം *zSql = 0;          /* SQL to run */
  പൂർണ്ണസംഖ്യ i;                         /* Loop counter */
  പൂർണ്ണസംഖ്യ rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  സ്ഥിതം സ്ഥിരം അക്ഷരം *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_ഇടത്,
    QRF_ALIGN_നടുവ്, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_വലത്,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_മുകൾ,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, വലുപ്പം(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &zResult;
  വേണ്ടി(i=2; i<objc; i++){
    സ്ഥിരം അക്ഷരം *zArg = Tcl_GetString(objv[i]);
    സ്ഥിരം അക്ഷരം *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    സ്ഥിരം unsigned അക്ഷരം aBoolMap[] = { 0, 2, 1, 2, 1 };
    എങ്കിൽ( zArg[0]!='-' ){
      എങ്കിൽ( zSql ){
        Tcl_AppendResult(pDb->interp, "unknown argument: ", zArg, (അക്ഷരം*)0);
        rc = TCL_ERROR;
        പോകുക format_failed;
      }
      zSql  = zArg;
    }അല്ലെങ്കിൽ എങ്കിൽ( i==objc-1 ){
      Tcl_AppendResult(pDb->interp, "option has no argument: ", zArg,(അക്ഷരം*)0);
      rc = TCL_ERROR;
      പോകുക format_failed;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-style")==0 ){
      സ്ഥിതം സ്ഥിരം അക്ഷരം *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      സ്ഥിതം unsigned അക്ഷരം aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_എണ്ണം,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_തിരുകുക,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_വരി,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      പൂർണ്ണസംഖ്യ style;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      എങ്കിൽ( rc ) പോകുക format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-esc")==0 ){
      സ്ഥിതം സ്ഥിരം അക്ഷരം *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      സ്ഥിതം unsigned അക്ഷരം aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_ചിഹ്നം
      };
      പൂർണ്ണസംഖ്യ esc;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      എങ്കിൽ( rc ) പോകുക format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      സ്ഥിതം സ്ഥിരം അക്ഷരം *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      സ്ഥിതം unsigned അക്ഷരം aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      പൂർണ്ണസംഖ്യ txt;
      പൂർണ്ണസംഖ്യ k = zArg[2]=='e';
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      എങ്കിൽ( rc ) പോകുക format_failed;
      എങ്കിൽ( k ){
        qrf.eText = aTextMap[txt];
      }അല്ലെങ്കിൽ എങ്കിൽ( txt<=2 ){
        പ്രതിജ്ഞാ( QRF_No==1 );
        പ്രതിജ്ഞാ( QRF_Yes==2 );
        പ്രതിജ്ഞാ( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }അല്ലെങ്കിൽ{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-blob")==0 ){
      സ്ഥിതം സ്ഥിരം അക്ഷരം *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      സ്ഥിതം unsigned അക്ഷരം aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_Size
      };
      പൂർണ്ണസംഖ്യ blob;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      എങ്കിൽ( rc ) പോകുക format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-wordwrap")==0 ){
      പൂർണ്ണസംഖ്യ v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      എങ്കിൽ( rc ) പോകുക format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      പൂർണ്ണസംഖ്യ v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      എങ്കിൽ( rc ) പോകുക format_failed;
      എങ്കിൽ( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }അല്ലെങ്കിൽ എങ്കിൽ( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }അല്ലെങ്കിൽ എങ്കിൽ( zArg[1]=='r' ){
        qrf.b_Row_എണ്ണം = aBoolMap[v];
      }അല്ലെങ്കിൽ{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      പൂർണ്ണസംഖ്യ ax = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      എങ്കിൽ( rc ) പോകുക format_failed;
      എങ്കിൽ( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }അല്ലെങ്കിൽ{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      പൂർണ്ണസംഖ്യ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      എങ്കിൽ( rc ) പോകുക format_failed;
      എങ്കിൽ( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }അല്ലെങ്കിൽ എങ്കിൽ( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      എങ്കിൽ( zArg[1]=='w' ){
        qrf.nWrap = v;
      }അല്ലെങ്കിൽ എങ്കിൽ( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }അല്ലെങ്കിൽ എങ്കിൽ( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }അല്ലെങ്കിൽ{
        qrf.n_വരി_Limit = v;
      }
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-charlimit")==0 ){
      പൂർണ്ണസംഖ്യ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      എങ്കിൽ( rc ) പോകുക format_failed;
      എങ്കിൽ( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-multiinsert")==0 ){
      പൂർണ്ണസംഖ്യ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      എങ്കിൽ( rc ) പോകുക format_failed;
      എങ്കിൽ( v<0 ) v = 0;
      qrf.n_Multi_തിരുകുക = v;
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-align")==0 ){
      Tcl_Size n = 0;
      പൂർണ്ണസംഖ്യ jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      എങ്കിൽ( rc ) പോകുക format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*വലുപ്പം(qrf.aAlign[0]) );
      എങ്കിൽ( qrf.aAlign==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (അക്ഷരം*)0);
        rc = TCL_ERROR;
        പോകുക format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*വലുപ്പം(qrf.aAlign[0]));
      qrf.nAlign = n;
      വേണ്ടി(jj=0; jj<n; jj++){
        പൂർണ്ണസംഖ്യ x;
        Tcl_Obj *pTerm;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        എങ്കിൽ( rc ) പോകുക format_failed;
        rc = Tcl_GetIndexFromObj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        എങ്കിൽ( rc ) പോകുക format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-widths")==0 ){
      Tcl_Size n = 0;
      പൂർണ്ണസംഖ്യ jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      എങ്കിൽ( rc ) പോകുക format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*വലുപ്പം(qrf.aWidth[0]) );
      എങ്കിൽ( qrf.aWidth==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (അക്ഷരം*)0);
        rc = TCL_ERROR;
        പോകുക format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*വലുപ്പം(qrf.aWidth[0]));
      qrf.nWidth = n;
      വേണ്ടി(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        പൂർണ്ണസംഖ്യ v;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        എങ്കിൽ( rc ) പോകുക format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        എങ്കിൽ( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }അല്ലെങ്കിൽ എങ്കിൽ( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short പൂർണ്ണസംഖ്യ)v;
      }
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-ifmt")==0 ){
      അക്ഷരം *zFmt = Tcl_GetString(objv[i+1]);
      എങ്കിൽ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_AppendResult(pDb->interp, "bad format for -ifmt", (അക്ഷരം*)0);
        rc = TCL_ERROR;
        പോകുക format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-fpfmt")==0 ){
      അക്ഷരം *zFmt = Tcl_GetString(objv[i+1]);
      എങ്കിൽ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_AppendResult(pDb->interp, "bad format for -fpfmt", (അക്ഷരം*)0);
        rc = TCL_ERROR;
        പോകുക format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }അല്ലെങ്കിൽ{
      Tcl_AppendResult(pDb->interp, "unknown option: ", zArg, (അക്ഷരം*)0);
      rc = TCL_ERROR;
      പോകുക format_failed;
    }
  }
  ആകുമ്പോൾ( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    അക്ഷരം *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    എങ്കിൽ( rc ) പോകുക format_failed;
    എങ്കിൽ( pStmt==0 ) തുടരുക;
    rc = sqlite3_format_query_result(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    എങ്കിൽ( rc ){
      Tcl_SetResult(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      പോകുക format_failed;
    }
  }
  Tcl_SetResult(pDb->interp, zResult, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.aWidth);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(zResult);
  തിരികെ rc;

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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI DbObjCmd(
  ശൂന്യം *cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *സ്ഥിരം*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  പൂർണ്ണസംഖ്യ choice;
  പൂർണ്ണസംഖ്യ rc = TCL_OK;
  സ്ഥിതം സ്ഥിരം അക്ഷരം *DB_strs[] = {
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
  എണ്ണൽ DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_LOAD_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_INTERRUPT,            DB_LAST_തിരുകുക_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_PROGRESS,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_STATUS,                DB_TIMEOUT,              DB_TOTAL_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_VERSION,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SUBCOMMAND ...");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    തിരികെ TCL_ERROR;
  }

  തിരഞ്ഞെടുപ്പ്( (എണ്ണൽ DB_enum)choice ){

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
  അവസ്ഥ DB_AUTHORIZER: {
#ifdef SQLITE_OMIT_AUTHORIZATION
    Tcl_AppendResult(interp, "authorization not available in this build",
                     (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
#അല്ലെങ്കിൽ
    എങ്കിൽ( objc>3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?CALLBACK?");
      തിരികെ TCL_ERROR;
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==2 ){
      എങ്കിൽ( pDb->zAuth ){
        Tcl_AppendResult(interp, pDb->zAuth, (അക്ഷരം*)0);
      }
    }അല്ലെങ്കിൽ{
      അക്ഷരം *zAuth;
      Tcl_Size len;
      എങ്കിൽ( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      എങ്കിൽ( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }അല്ലെങ്കിൽ{
        pDb->zAuth = 0;
      }
      എങ്കിൽ( pDb->zAuth ){
        തരനിർവചനം പൂർണ്ണസംഖ്യ (*sqlite3_auth_cb)(
           ശൂന്യം*,പൂർണ്ണസംഖ്യ,സ്ഥിരം അക്ഷരം*,സ്ഥിരം അക്ഷരം*,
           സ്ഥിരം അക്ഷരം*,സ്ഥിരം അക്ഷരം*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }അല്ലെങ്കിൽ{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#endif
    നിർത്തുക;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  അവസ്ഥ DB_BACKUP: {
    സ്ഥിരം അക്ഷരം *zDestFile;
    സ്ഥിരം അക്ഷരം *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    എങ്കിൽ( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }അല്ലെങ്കിൽ{
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?DATABASE? FILENAME");
      തിരികെ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    എങ്കിൽ( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (അക്ഷരം*)0);
      sqlite3_close(pDest);
      തിരികെ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    എങ്കിൽ( pBackup==0 ){
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (അക്ഷരം*)0);
      sqlite3_close(pDest);
      തിരികെ TCL_ERROR;
    }
    ആകുമ്പോൾ(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    എങ്കിൽ( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }അല്ലെങ്കിൽ{
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (അക്ഷരം*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    നിർത്തുക;
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
  അവസ്ഥ DB_BIND_FALLBACK: {
    എങ്കിൽ( objc>3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?CALLBACK?");
      തിരികെ TCL_ERROR;
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==2 ){
      എങ്കിൽ( pDb->zBindFallback ){
        Tcl_AppendResult(interp, pDb->zBindFallback, (അക്ഷരം*)0);
      }
    }അല്ലെങ്കിൽ{
      അക്ഷരം *zCallback;
      Tcl_Size len;
      എങ്കിൽ( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      എങ്കിൽ( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }അല്ലെങ്കിൽ{
        pDb->zBindFallback = 0;
      }
    }
    നിർത്തുക;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  അവസ്ഥ DB_BUSY: {
    എങ്കിൽ( objc>3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "CALLBACK");
      തിരികെ TCL_ERROR;
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==2 ){
      എങ്കിൽ( pDb->zBusy ){
        Tcl_AppendResult(interp, pDb->zBusy, (അക്ഷരം*)0);
      }
    }അല്ലെങ്കിൽ{
      അക്ഷരം *zBusy;
      Tcl_Size len;
      എങ്കിൽ( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      എങ്കിൽ( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }അല്ലെങ്കിൽ{
        pDb->zBusy = 0;
      }
      എങ്കിൽ( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }അല്ലെങ്കിൽ{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    നിർത്തുക;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  അവസ്ഥ DB_CACHE: {
    അക്ഷരം *subCmd;
    പൂർണ്ണസംഖ്യ n;

    എങ്കിൽ( objc<=2 ){
      Tcl_Wrong_അക്കം_Args(interp, 1, objv, "cache option ?arg?");
      തിരികെ TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    എങ്കിൽ( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      എങ്കിൽ( objc!=3 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "flush");
        തിരികെ TCL_ERROR;
      }അല്ലെങ്കിൽ{
        flushStmtCache( pDb );
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      എങ്കിൽ( objc!=4 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "size n");
        തിരികെ TCL_ERROR;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_AppendResult( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (അക്ഷരം*)0);
          തിരികെ TCL_ERROR;
        }അല്ലെങ്കിൽ{
          എങ്കിൽ( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }അല്ലെങ്കിൽ എങ്കിൽ( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }അല്ലെങ്കിൽ{
      Tcl_AppendResult( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
    നിർത്തുക;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  അവസ്ഥ DB_CHANGES: {
    Tcl_Obj *pResult;
    എങ്കിൽ( objc!=2 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
      തിരികെ TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_changes64(pDb->db));
    നിർത്തുക;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  അവസ്ഥ DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    നിർത്തുക;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  അവസ്ഥ DB_COLLATE: {
    SqlCollate *pCollate;
    അക്ഷരം *zName;
    അക്ഷരം *zScript;
    Tcl_Size nScript;
    എങ്കിൽ( objc!=4 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "NAME SCRIPT");
      തിരികെ TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( വലുപ്പം(*pCollate) + nScript + 1 );
    എങ്കിൽ( pCollate==0 ) തിരികെ TCL_ERROR;
    pCollate->interp = interp;
    pCollate->pNext = pDb->pCollate;
    pCollate->zScript = (അക്ഷരം*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    എങ്കിൽ( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_SetResult(interp, (അക്ഷരം *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      തിരികെ TCL_ERROR;
    }
    നിർത്തുക;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  അവസ്ഥ DB_COLLATION_NEEDED: {
    എങ്കിൽ( objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "SCRIPT");
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_എണ്ണം(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_എണ്ണം(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    നിർത്തുക;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  അവസ്ഥ DB_COMMIT_HOOK: {
    എങ്കിൽ( objc>3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?CALLBACK?");
      തിരികെ TCL_ERROR;
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==2 ){
      എങ്കിൽ( pDb->zCommit ){
        Tcl_AppendResult(interp, pDb->zCommit, (അക്ഷരം*)0);
      }
    }അല്ലെങ്കിൽ{
      സ്ഥിരം അക്ഷരം *zCommit;
      Tcl_Size len;
      എങ്കിൽ( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      എങ്കിൽ( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }അല്ലെങ്കിൽ{
        pDb->zCommit = 0;
      }
      എങ്കിൽ( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }അല്ലെങ്കിൽ{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    നിർത്തുക;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  അവസ്ഥ DB_COMPLETE: {
#ifndef SQLITE_OMIT_COMPLETE
    Tcl_Obj *pResult;
    പൂർണ്ണസംഖ്യ isComplete;
    എങ്കിൽ( objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "SQL");
      തിരികെ TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetBooleanObj(pResult, isComplete);
#endif
    നിർത്തുക;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  അവസ്ഥ DB_CONFIG: {
    സ്ഥിതം സ്ഥിരം ഘടന DbConfigChoices {
      സ്ഥിരം അക്ഷരം *zName;
      പൂർണ്ണസംഖ്യ op;
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
    പൂർണ്ണസംഖ്യ ii;
    എങ്കിൽ( objc>4 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      pResult = Tcl_NewListObj(0,0);
      വേണ്ടി(ii=0; ii<വലുപ്പം(aDbConfig)/വലുപ്പം(aDbConfig[0]); ii++){
        പൂർണ്ണസംഖ്യ v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewIntObj(v));
      }
    }അല്ലെങ്കിൽ{
      സ്ഥിരം അക്ഷരം *zOpt = Tcl_GetString(objv[2]);
      പൂർണ്ണസംഖ്യ onoff = -1;
      പൂർണ്ണസംഖ്യ v = 0;
      എങ്കിൽ( zOpt[0]=='-' ) zOpt++;
      വേണ്ടി(ii=0; ii<വലുപ്പം(aDbConfig)/വലുപ്പം(aDbConfig[0]); ii++){
        എങ്കിൽ( strcmp(aDbConfig[ii].zName, zOpt)==0 ) നിർത്തുക;
      }
      എങ്കിൽ( ii>=വലുപ്പം(aDbConfig)/വലുപ്പം(aDbConfig[0]) ){
        Tcl_AppendResult(interp, "unknown config option: \"", zOpt,
                                "\"", (ശൂന്യം*)0);
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( objc==4 ){
        എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          തിരികെ TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      pResult = Tcl_NewIntObj(v);
    }
    Tcl_SetObjResult(interp, pResult);
    നിർത്തുക;
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
  അവസ്ഥ DB_COPY: {
    അക്ഷരം *zTable;               /* Insert data into this table */
    അക്ഷരം *zFile;                /* The file from which to extract data */
    അക്ഷരം *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    പൂർണ്ണസംഖ്യ nCol;                   /* Number of columns in the table */
    പൂർണ്ണസംഖ്യ nByte;                  /* Number of bytes in an SQL string */
    പൂർണ്ണസംഖ്യ i, j;                   /* Loop counters */
    പൂർണ്ണസംഖ്യ nSep;                   /* Number of bytes in zSep[] */
    പൂർണ്ണസംഖ്യ nNull;                  /* Number of bytes in zNull[] */
    അക്ഷരം *zSql;                 /* An SQL statement */
    അക്ഷരം *z_വരി;                /* A single line of input from the file */
    അക്ഷരം **azCol;               /* zLine[] broken up into columns */
    സ്ഥിരം അക്ഷരം *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    പൂർണ്ണസംഖ്യ lineno = 0;             /* Line number of input file */
    അക്ഷരം z_വരി_അക്കം[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *pResult;           /* interp result */

    സ്ഥിരം അക്ഷരം *zSep;
    സ്ഥിരം അക്ഷരം *zNull;
    എങ്കിൽ( objc<5 || objc>7 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }അല്ലെങ്കിൽ{
      zSep = "\t";
    }
    എങ്കിൽ( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }അല്ലെങ്കിൽ{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    എങ്കിൽ( nSep==0 ){
      Tcl_AppendResult(interp,"Error: non-null separator required for copy",
                       (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_AppendResult(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    എങ്കിൽ( zSql==0 ){
      Tcl_AppendResult(interp, "Error: no such table: ", zTable, (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    എങ്കിൽ( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
      nCol = 0;
    }അല്ലെങ്കിൽ{
      nCol = sqlite3_column_എണ്ണം(pStmt);
    }
    sqlite3_finalize(pStmt);
    എങ്കിൽ( nCol==0 ) {
      തിരികെ TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    എങ്കിൽ( zSql==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    വേണ്ടി(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    സ്വതന്ത്രം(zSql);
    എങ്കിൽ( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
      sqlite3_finalize(pStmt);
      തിരികെ TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    എങ്കിൽ( in==0 ){
      sqlite3_finalize(pStmt);
      തിരികെ TCL_ERROR;
    }
    Tcl_SetChannelOption(ശൂന്യ, in, "-translation", "auto");
    azCol = malloc( വലുപ്പം(azCol[0])*(nCol+1) );
    എങ്കിൽ( azCol==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (അക്ഷരം*)0);
      Tcl_Close(interp, in);
      തിരികെ TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_എണ്ണം(str);
    (ശൂന്യം)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    ആകുമ്പോൾ( Tcl_GetsObj(in, str)>=0 ) {
      അക്ഷരം *z;
      Tcl_Size byteLen;
      lineno++;
      z_വരി = (അക്ഷരം *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = z_വരി;
      വേണ്ടി(i=0, z=z_വരി; *z; z++){
        എങ്കിൽ( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          എങ്കിൽ( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      എങ്കിൽ( i+1!=nCol ){
        അക്ഷരം *zErr;
        പൂർണ്ണസംഖ്യ nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        എങ്കിൽ( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_AppendResult(interp, zErr, (അക്ഷരം*)0);
          സ്വതന്ത്രം(zErr);
        }
        zCommit = "ROLLBACK";
        നിർത്തുക;
      }
      വേണ്ടി(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        എങ്കിൽ( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }അല്ലെങ്കിൽ{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_SetObjLength(str, 0);
      എങ്കിൽ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp,"Error: ", sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
        zCommit = "ROLLBACK";
        നിർത്തുക;
      }
    }
    Tcl_Decr_Ref_എണ്ണം(str);
    സ്വതന്ത്രം(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (ശൂന്യം)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    എങ്കിൽ( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      pResult = Tcl_GetObjResult(interp);
      എങ്കിൽ( Tcl_IsShared(pResult) ){
        pResult = Tcl_DuplicateObj(pResult);
      }
      Tcl_SetIntObj(pResult, lineno);
      rc = TCL_OK;
    }അല്ലെങ്കിൽ{
      /* failure, append lineno where failed */
      sqlite3_snprintf(വലുപ്പം(z_വരി_അക്കം), z_വരി_അക്കം,"%d",lineno);
      Tcl_AppendResult(interp,", failed while processing line: ",z_വരി_അക്കം,
                       (അക്ഷരം*)0);
      rc = TCL_ERROR;
    }
    നിർത്തുക;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  അവസ്ഥ DB_DESERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (അക്ഷരം*)0);
    rc = TCL_ERROR;
#അല്ലെങ്കിൽ
    സ്ഥിരം അക്ഷരം *zSchema = 0;
    Tcl_Obj *p_മൂല്യം = 0;
    unsigned അക്ഷരം *pBA;
    unsigned അക്ഷരം *pData;
    Tcl_Size len;
    പൂർണ്ണസംഖ്യ xrc;
    sqlite3_int64 mxSize = 0;
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ isReadonly = 0;


    എങ്കിൽ( objc<3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      നിർത്തുക;
    }
    വേണ്ടി(i=2; i<objc-1; i++){
      സ്ഥിരം അക്ഷരം *z = Tcl_GetString(objv[i]);
      എങ്കിൽ( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        എങ്കിൽ( rc ) പോകുക deserialize_error;
        mxSize = x;
        തുടരുക;
      }
      എങ്കിൽ( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        എങ്കിൽ( rc ) പോകുക deserialize_error;
        തുടരുക;
      }
      എങ്കിൽ( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        തുടരുക;
      }
      Tcl_AppendResult(interp, "unknown option: ", z, (അക്ഷരം*)0);
      rc = TCL_ERROR;
      പോകുക deserialize_error;
    }
    p_മൂല്യം = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_മൂല്യം, &len);
    pData = sqlite3_malloc64( len );
    എങ്കിൽ( pData==0 && len>0 ){
      Tcl_AppendResult(interp, "out of memory", (അക്ഷരം*)0);
      rc = TCL_ERROR;
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ flags;
      എങ്കിൽ( len>0 ) memcpy(pData, pBA, len);
      എങ്കിൽ( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }അല്ലെങ്കിൽ{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, pData, len, len, flags);
      എങ്കിൽ( xrc ){
        Tcl_AppendResult(interp, "unable to set MEMDB content", (അക്ഷരം*)0);
        rc = TCL_ERROR;
      }
      എങ്കിൽ( mxSize>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_SIZE_LIMIT,&mxSize);
      }
    }
deserialize_error:
#endif
    നിർത്തുക; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  അവസ്ഥ DB_ENABLE_LOAD_EXTENSION: {
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    പൂർണ്ണസംഖ്യ onoff;
    എങ്കിൽ( objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "BOOLEAN");
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      തിരികെ TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    നിർത്തുക;
#അല്ലെങ്കിൽ
    Tcl_AppendResult(interp, "extension loading is turned off at compile-time",
                     (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
#endif
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  അവസ്ഥ DB_ERRORCODE: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    നിർത്തുക;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  അവസ്ഥ DB_ERROROFFSET: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    നിർത്തുക;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  അവസ്ഥ DB_EXISTS:
  അവസ്ഥ DB_ONECOLUMN: {
    Tcl_Obj *pResult = 0;
    DbEvalContext sEval;
    എങ്കിൽ( objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "SQL");
      തിരികെ TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    എങ്കിൽ( choice==DB_ONECOLUMN ){
      എങ്കിൽ( rc==TCL_OK ){
        pResult = db_Eval_Column_മൂല്യം(&sEval, 0);
      }അല്ലെങ്കിൽ എങ്കിൽ( rc==TCL_BREAK ){
        Tcl_ResetResult(interp);
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( rc==TCL_BREAK || rc==TCL_OK ){
      pResult = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    എങ്കിൽ( pResult ) Tcl_SetObjResult(interp, pResult);

    എങ്കിൽ( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    നിർത്തുക;
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
  അവസ്ഥ DB_EVAL: {
    പൂർണ്ണസംഖ്യ evalFlags = 0;
    സ്ഥിരം അക്ഷരം *zOpt;
    ആകുമ്പോൾ( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      എങ്കിൽ( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }അല്ലെങ്കിൽ{
        Tcl_AppendResult(interp, "unknown option: \"", zOpt, "\"", (ശൂന്യം*)0);
        തിരികെ TCL_ERROR;
      }
      objc--;
      objv++;
    }
    എങ്കിൽ( objc<3 || objc>5 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      തിരികെ TCL_ERROR;
    }

    എങ്കിൽ( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_എണ്ണം(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      ആകുമ്പോൾ( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        പൂർണ്ണസംഖ്യ i;
        പൂർണ്ണസംഖ്യ nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        വേണ്ടി(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_Column_മൂല്യം(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      എങ്കിൽ( rc==TCL_BREAK ){
        Tcl_SetObjResult(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_എണ്ണം(pRet);
    }അല്ലെങ്കിൽ{
      ClientData cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      എങ്കിൽ( objc>=5 && *(അക്ഷരം *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_എണ്ണം(pScript);

      p = (DbEvalContext *)Tcl_Alloc(വലുപ്പം(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (ശൂന്യം *)p;
      cd2[1] = (ശൂന്യം *)pScript;
      rc = DbEvalNextCmd(cd2, interp, TCL_OK);
    }
    നിർത്തുക;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  അവസ്ഥ DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    നിർത്തുക;
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
  അവസ്ഥ DB_FUNCTION: {
    പൂർണ്ണസംഖ്യ flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    അക്ഷരം *zName;
    പൂർണ്ണസംഖ്യ nArg = -1;
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ eType = SQLITE_NULL;
    എങ്കിൽ( objc<4 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      തിരികെ TCL_ERROR;
    }
    വേണ്ടി(i=3; i<(objc-1); i++){
      സ്ഥിരം അക്ഷരം *z = Tcl_GetString(objv[i]);
      പൂർണ്ണസംഖ്യ n = strlen30(z);
      എങ്കിൽ( n>1 && strncmp(z, "-argcount",n)==0 ){
        എങ്കിൽ( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(അക്ഷരം*)0);
          തിരികെ TCL_ERROR;
        }
        എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) തിരികെ TCL_ERROR;
        എങ്കിൽ( nArg<0 ){
          Tcl_AppendResult(interp, "number of arguments must be non-negative",
                           (അക്ഷരം*)0);
          തിരികെ TCL_ERROR;
        }
        i++;
      }അല്ലെങ്കിൽ
      എങ്കിൽ( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }അല്ലെങ്കിൽ
      എങ്കിൽ( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }അല്ലെങ്കിൽ
      എങ്കിൽ( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }അല്ലെങ്കിൽ
      എങ്കിൽ( n>1 && strncmp(z, "-returntype", n)==0 ){
        സ്ഥിരം അക്ഷരം *azType[] = {"integer", "real", "text", "blob", "any", 0};
        പ്രതിജ്ഞാ( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        പ്രതിജ്ഞാ( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        എങ്കിൽ( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(അക്ഷരം*)0);
          തിരികെ TCL_ERROR;
        }
        i++;
        എങ്കിൽ( Tcl_GetIndexFromObj(interp, objv[i], azType, "type", 0, &eType) ){
          തിരികെ TCL_ERROR;
        }
        eType++;
      }അല്ലെങ്കിൽ{
        Tcl_AppendResult(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (അക്ഷരം*)0
        );
        തിരികെ TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    എങ്കിൽ( pFunc==0 ) തിരികെ TCL_ERROR;
    എങ്കിൽ( pFunc->pScript ){
      Tcl_Decr_Ref_എണ്ണം(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_എണ്ണം(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    എങ്കിൽ( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_SetResult(interp, (അക്ഷരം *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    നിർത്തുക;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  അവസ്ഥ DB_INCRBLOB: {
#ifdef SQLITE_OMIT_INCRBLOB
    Tcl_AppendResult(interp, "incrblob not available in this build", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
#അല്ലെങ്കിൽ
    പൂർണ്ണസംഖ്യ isReadonly = 0;
    സ്ഥിരം അക്ഷരം *zDb = "main";
    സ്ഥിരം അക്ഷരം *zTable;
    സ്ഥിരം അക്ഷരം *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    എങ്കിൽ( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    എങ്കിൽ( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      തിരികെ TCL_ERROR;
    }

    എങ്കിൽ( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    എങ്കിൽ( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#endif
    നിർത്തുക;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  അവസ്ഥ DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    നിർത്തുക;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  അവസ്ഥ DB_NULLVALUE: {
    എങ്കിൽ( objc!=2 && objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "NULLVALUE");
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( objc==3 ){
      Tcl_Size len;
      അക്ഷരം *zNull = Tcl_GetStringFromObj(objv[2], &len);
      എങ്കിൽ( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      എങ്കിൽ( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }അല്ലെങ്കിൽ{
        pDb->zNull = 0;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(pDb->zNull, -1));
    നിർത്തുക;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  അവസ്ഥ DB_LAST_തിരുകുക_ROWID: {
    Tcl_Obj *pResult;
    Tcl_WideInt rowid;
    എങ്കിൽ( objc!=2 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
      തിരികെ TCL_ERROR;
    }
    rowid = sqlite3_last_തിരുകുക_rowid(pDb->db);
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, rowid);
    നിർത്തുക;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  അവസ്ഥ DB_PROGRESS: {
    എങ്കിൽ( objc==2 ){
      എങ്കിൽ( pDb->zProgress ){
        Tcl_AppendResult(interp, pDb->zProgress, (അക്ഷരം*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==4 ){
      അക്ഷരം *zProgress;
      Tcl_Size len;
      പൂർണ്ണസംഖ്യ N;
      എങ്കിൽ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        തിരികെ TCL_ERROR;
      };
      എങ്കിൽ( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      എങ്കിൽ( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }അല്ലെങ്കിൽ{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      എങ്കിൽ( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }അല്ലെങ്കിൽ{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }അല്ലെങ്കിൽ{
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "N CALLBACK");
      തിരികെ TCL_ERROR;
    }
    നിർത്തുക;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  അവസ്ഥ DB_PROFILE: {
    എങ്കിൽ( objc>3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?CALLBACK?");
      തിരികെ TCL_ERROR;
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==2 ){
      എങ്കിൽ( pDb->zProfile ){
        Tcl_AppendResult(interp, pDb->zProfile, (അക്ഷരം*)0);
      }
    }അല്ലെങ്കിൽ{
      അക്ഷരം *zProfile;
      Tcl_Size len;
      എങ്കിൽ( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      എങ്കിൽ( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }അല്ലെങ്കിൽ{
        pDb->zProfile = 0;
      }
#എങ്കിൽ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      എങ്കിൽ( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }അല്ലെങ്കിൽ{
        sqlite3_profile(pDb->db, 0, 0);
      }
#endif
    }
    നിർത്തുക;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  അവസ്ഥ DB_REKEY: {
    എങ്കിൽ( objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "KEY");
      തിരികെ TCL_ERROR;
    }
    നിർത്തുക;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  അവസ്ഥ DB_RESTORE: {
    സ്ഥിരം അക്ഷരം *zSrcFile;
    സ്ഥിരം അക്ഷരം *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    പൂർണ്ണസംഖ്യ nTimeout = 0;

    എങ്കിൽ( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }അല്ലെങ്കിൽ{
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?DATABASE? FILENAME");
      തിരികെ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    എങ്കിൽ( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (അക്ഷരം*)0);
      sqlite3_close(pSrc);
      തിരികെ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    എങ്കിൽ( pBackup==0 ){
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
      sqlite3_close(pSrc);
      തിരികെ TCL_ERROR;
    }
    ആകുമ്പോൾ( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      എങ്കിൽ( rc==SQLITE_BUSY ){
        എങ്കിൽ( nTimeout++ >= 3 ) നിർത്തുക;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    എങ്കിൽ( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }അല്ലെങ്കിൽ എങ്കിൽ( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_AppendResult(interp, "restore failed: source database busy",
                       (അക്ഷരം*)0);
      rc = TCL_ERROR;
    }അല്ലെങ്കിൽ{
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    നിർത്തുക;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  അവസ്ഥ DB_SERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (അക്ഷരം*)0);
    rc = TCL_ERROR;
#അല്ലെങ്കിൽ
    സ്ഥിരം അക്ഷരം *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned അക്ഷരം *pData;
    എങ്കിൽ( objc!=2 && objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ needFree;
      pData = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      എങ്കിൽ( pData ){
        needFree = 0;
      }അല്ലെങ്കിൽ{
        pData = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(pData,sz));
      എങ്കിൽ( needFree ) sqlite3_free(pData);
    }
#endif
    നിർത്തുക;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  അവസ്ഥ DB_STATUS: {
    പൂർണ്ണസംഖ്യ v;
    സ്ഥിരം അക്ഷരം *zOp;
    എങ്കിൽ( objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "(step|sort|autoindex)");
      തിരികെ TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    എങ്കിൽ( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zOp, "autoindex")==0 ){
      v = pDb->nIndex;
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }അല്ലെങ്കിൽ{
      Tcl_AppendResult(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(v));
    നിർത്തുക;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  അവസ്ഥ DB_TIMEOUT: {
    പൂർണ്ണസംഖ്യ ms;
    എങ്കിൽ( objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "MILLISECONDS");
      തിരികെ TCL_ERROR;
    }
    എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &ms) ) തിരികെ TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    നിർത്തുക;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  അവസ്ഥ DB_TOTAL_CHANGES: {
    Tcl_Obj *pResult;
    എങ്കിൽ( objc!=2 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
      തിരികെ TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_total_changes64(pDb->db));
    നിർത്തുക;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  അവസ്ഥ DB_TRACE: {
    എങ്കിൽ( objc>3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?CALLBACK?");
      തിരികെ TCL_ERROR;
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==2 ){
      എങ്കിൽ( pDb->zTrace ){
        Tcl_AppendResult(interp, pDb->zTrace, (അക്ഷരം*)0);
      }
    }അല്ലെങ്കിൽ{
      അക്ഷരം *zTrace;
      Tcl_Size len;
      എങ്കിൽ( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      എങ്കിൽ( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }അല്ലെങ്കിൽ{
        pDb->zTrace = 0;
      }
#എങ്കിൽ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      എങ്കിൽ( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }അല്ലെങ്കിൽ{
        sqlite3_trace(pDb->db, 0, 0);
      }
#endif
    }
    നിർത്തുക;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  അവസ്ഥ DB_TRACE_V2: {
    എങ്കിൽ( objc>4 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?CALLBACK? ?MASK?");
      തിരികെ TCL_ERROR;
    }അല്ലെങ്കിൽ എങ്കിൽ( objc==2 ){
      എങ്കിൽ( pDb->zTraceV2 ){
        Tcl_AppendResult(interp, pDb->zTraceV2, (അക്ഷരം*)0);
      }
    }അല്ലെങ്കിൽ{
      അക്ഷരം *zTraceV2;
      Tcl_Size len;
      Tcl_WideInt wMask = 0;
      എങ്കിൽ( objc==4 ){
        സ്ഥിതം സ്ഥിരം അക്ഷരം *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        എണ്ണൽ TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_Size i;
        എങ്കിൽ( TCL_OK!=Tcl_ListObjLength(interp, objv[3], &len) ){
          തിരികെ TCL_ERROR;
        }
        വേണ്ടി(i=0; i<len; i++){
          Tcl_Obj *pObj;
          പൂർണ്ണസംഖ്യ ttype;
          എങ്കിൽ( TCL_OK!=Tcl_ListObjIndex(interp, objv[3], i, &pObj) ){
            തിരികെ TCL_ERROR;
          }
          എങ്കിൽ( Tcl_GetIndexFromObj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
            Tcl_Incr_Ref_എണ്ണം(pError);
            എങ്കിൽ( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_എണ്ണം(pError);
              wMask |= wType;
            }അല്ലെങ്കിൽ{
              Tcl_SetObjResult(interp, pError);
              Tcl_Decr_Ref_എണ്ണം(pError);
              തിരികെ TCL_ERROR;
            }
          }അല്ലെങ്കിൽ{
            തിരഞ്ഞെടുപ്പ്( (എണ്ണൽ TTYPE_enum)ttype ){
              അവസ്ഥ TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    നിർത്തുക;
              അവസ്ഥ TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; നിർത്തുക;
              അവസ്ഥ TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     നിർത്തുക;
              അവസ്ഥ TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   നിർത്തുക;
            }
          }
        }
      }അല്ലെങ്കിൽ{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      എങ്കിൽ( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      എങ്കിൽ( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }അല്ലെങ്കിൽ{
        pDb->zTraceV2 = 0;
      }
#എങ്കിൽ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      എങ്കിൽ( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }അല്ലെങ്കിൽ{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#endif
    }
    നിർത്തുക;
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
  അവസ്ഥ DB_TRANSACTION: {
    Tcl_Obj *pScript;
    സ്ഥിരം അക്ഷരം *zBegin = "SAVEPOINT _tcl_transaction";
    എങ്കിൽ( objc!=3 && objc!=4 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "[TYPE] SCRIPT");
      തിരികെ TCL_ERROR;
    }

    എങ്കിൽ( pDb->nTransaction==0 && objc==4 ){
      സ്ഥിതം സ്ഥിരം അക്ഷരം *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      എണ്ണൽ TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      പൂർണ്ണസംഖ്യ ttype;
      എങ്കിൽ( Tcl_GetIndexFromObj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        തിരികെ TCL_ERROR;
      }
      തിരഞ്ഞെടുപ്പ്( (എണ്ണൽ TTYPE_enum)ttype ){
        അവസ്ഥ TTYPE_DEFERRED:    /* no-op */;                 നിർത്തുക;
        അവസ്ഥ TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  നിർത്തുക;
        അവസ്ഥ TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  നിർത്തുക;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    എങ്കിൽ( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    എങ്കിൽ( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (ശൂന്യം)Tcl_NREvalObj(interp, pScript, 0);
    }അല്ലെങ്കിൽ{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    നിർത്തുക;
  }

  /*
  **    $db unlock_notify ?script?
  */
  അവസ്ഥ DB_UNLOCK_NOTIFY: {
#ifndef SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_AppendResult(interp, "unlock_notify not available in this build",
                     (അക്ഷരം*)0);
    rc = TCL_ERROR;
#അല്ലെങ്കിൽ
    എങ്കിൽ( objc!=2 && objc!=3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }അല്ലെങ്കിൽ{
      ശൂന്യം (*xNotify)(ശൂന്യം **, പൂർണ്ണസംഖ്യ) = 0;
      ശൂന്യം *pNotifyArg = 0;

      എങ്കിൽ( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_എണ്ണം(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      എങ്കിൽ( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (ശൂന്യം *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_എണ്ണം(pDb->pUnlockNotify);
      }

      എങ്കിൽ( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
        rc = TCL_ERROR;
      }
    }
#endif
    നിർത്തുക;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  അവസ്ഥ DB_PREUPDATE: {
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_AppendResult(interp, "preupdate_hook was omitted at compile-time", 
                     (അക്ഷരം*)0);
    rc = TCL_ERROR;
#അല്ലെങ്കിൽ
    സ്ഥിതം സ്ഥിരം അക്ഷരം *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    എണ്ണൽ DbPreupdateSubCmd {
      PRE_എണ്ണം, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    പൂർണ്ണസംഖ്യ iSub;

    എങ്കിൽ( objc<3 ){
      Tcl_Wrong_അക്കം_Args(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    എങ്കിൽ( Tcl_GetIndexFromObj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      തിരികെ TCL_ERROR;
    }

    തിരഞ്ഞെടുപ്പ്( (എണ്ണൽ DbPreupdateSubCmd)iSub ){
      അവസ്ഥ PRE_എണ്ണം: {
        പൂർണ്ണസംഖ്യ nCol = sqlite3_preupdate_എണ്ണം(pDb->db);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nCol));
        നിർത്തുക;
      }

      അവസ്ഥ PRE_HOOK: {
        എങ്കിൽ( objc>4 ){
          Tcl_Wrong_അക്കം_Args(interp, 2, objv, "hook ?SCRIPT?");
          തിരികെ TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        നിർത്തുക;
      }

      അവസ്ഥ PRE_DEPTH: {
        Tcl_Obj *pRet;
        എങ്കിൽ( objc!=3 ){
          Tcl_Wrong_അക്കം_Args(interp, 3, objv, "");
          തിരികെ TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_SetObjResult(interp, pRet);
        നിർത്തുക;
      }

      അവസ്ഥ PRE_NEW:
      അവസ്ഥ PRE_OLD: {
        പൂർണ്ണസംഖ്യ iIdx;
        sqlite3_മൂല്യം *p_മൂല്യം;
        എങ്കിൽ( objc!=4 ){
          Tcl_Wrong_അക്കം_Args(interp, 3, objv, "INDEX");
          തിരികെ TCL_ERROR;
        }
        എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          തിരികെ TCL_ERROR;
        }

        എങ്കിൽ( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_മൂല്യം);
        }അല്ലെങ്കിൽ{
          പ്രതിജ്ഞാ( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_മൂല്യം);
        }

        എങ്കിൽ( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((അക്ഷരം*)sqlite3_മൂല്യം_text(p_മൂല്യം), -1);
          Tcl_SetObjResult(interp, pObj);
        }അല്ലെങ്കിൽ{
          Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (അക്ഷരം*)0);
          തിരികെ TCL_ERROR;
        }
      }
    }
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
    നിർത്തുക;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  അവസ്ഥ DB_WAL_HOOK:
  അവസ്ഥ DB_UPDATE_HOOK:
  അവസ്ഥ DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    എങ്കിൽ( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    എങ്കിൽ( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    എങ്കിൽ( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    എങ്കിൽ( objc>3 ){
       Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?SCRIPT?");
       തിരികെ TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    നിർത്തുക;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  അവസ്ഥ DB_VERSION: {
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=2; i<objc; i++){
      സ്ഥിരം അക്ഷരം *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      എങ്കിൽ( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          തിരികെ TCL_ERROR;
        }
      }അല്ലെങ്കിൽ

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      എങ്കിൽ( strcmp(zArg, "-last-stmt-ptr")==0 ){
        ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3TestPtrToText(ശൂന്യം*);
        ശൂന്യം *ptr = (ശൂന്യം*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_SetResult(interp, (അക്ഷരം*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }അല്ലെങ്കിൽ
#endif /* SQLITE_TEST */
      {
        Tcl_AppendResult(interp, "unknown argument: ", zArg, (അക്ഷരം*)0);
        തിരികെ TCL_ERROR;
      }
    }
    എങ്കിൽ( i==2 ){   
      Tcl_SetResult(interp, (അക്ഷരം *)sqlite3_libversion(), TCL_STATIC);
    }
    നിർത്തുക;
  }


  } /* End of the SWITCH statement */
  തിരികെ rc;
}

#എങ്കിൽ SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI DbObjCmdAdaptor(
  ശൂന്യം *cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *സ്ഥിരം*objv
){
  തിരികെ Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *സ്ഥിരം*objv
){
  Tcl_Wrong_അക്കം_Args(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  തിരികെ TCL_ERROR;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI DbMain(
  ശൂന്യം *cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *സ്ഥിരം*objv
){
  SqliteDb *p;
  സ്ഥിരം അക്ഷരം *zArg;
  അക്ഷരം *z_Err_സന്ദേശം;
  പൂർണ്ണസംഖ്യ i;
  സ്ഥിരം അക്ഷരം *zFile = 0;
  സ്ഥിരം അക്ഷരം *zVfs = 0;
  പൂർണ്ണസംഖ്യ flags;
  പൂർണ്ണസംഖ്യ bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  പൂർണ്ണസംഖ്യ rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#അല്ലെങ്കിൽ
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  എങ്കിൽ( objc==1 ) തിരികെ sqliteCmdUsage(interp, objv);
  എങ്കിൽ( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    എങ്കിൽ( strcmp(zArg,"-version")==0 ){
      Tcl_AppendResult(interp,sqlite3_libversion(), (അക്ഷരം*)0);
      തിരികെ TCL_OK;
    }
    എങ്കിൽ( strcmp(zArg,"-sourceid")==0 ){
      Tcl_AppendResult(interp,sqlite3_sourceid(), (അക്ഷരം*)0);
      തിരികെ TCL_OK;
    }
    എങ്കിൽ( strcmp(zArg,"-has-codec")==0 ){
      Tcl_AppendResult(interp,"0",(അക്ഷരം*)0);
      തിരികെ TCL_OK;
    }
    എങ്കിൽ( zArg[0]=='-' ) തിരികെ sqliteCmdUsage(interp, objv);
  }
  വേണ്ടി(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    എങ്കിൽ( zArg[0]!='-' ){
      എങ്കിൽ( zFile!=0 ) തിരികെ sqliteCmdUsage(interp, objv);
      zFile = zArg;
      തുടരുക;
    }
    എങ്കിൽ( i==objc-1 ) തിരികെ sqliteCmdUsage(interp, objv);
    i++;
    എങ്കിൽ( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg, "-readonly")==0 ){
      പൂർണ്ണസംഖ്യ b;
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) തിരികെ TCL_ERROR;
      എങ്കിൽ( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }അല്ലെങ്കിൽ{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg, "-create")==0 ){
      പൂർണ്ണസംഖ്യ b;
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) തിരികെ TCL_ERROR;
      എങ്കിൽ( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }അല്ലെങ്കിൽ{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg, "-nofollow")==0 ){
      പൂർണ്ണസംഖ്യ b;
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) തിരികെ TCL_ERROR;
      എങ്കിൽ( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }അല്ലെങ്കിൽ{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg, "-nomutex")==0 ){
      പൂർണ്ണസംഖ്യ b;
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) തിരികെ TCL_ERROR;
      എങ്കിൽ( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }അല്ലെങ്കിൽ{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg, "-fullmutex")==0 ){
      പൂർണ്ണസംഖ്യ b;
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) തിരികെ TCL_ERROR;
      എങ്കിൽ( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }അല്ലെങ്കിൽ{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg, "-uri")==0 ){
      പൂർണ്ണസംഖ്യ b;
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) തിരികെ TCL_ERROR;
      എങ്കിൽ( b ){
        flags |= SQLITE_OPEN_URI;
      }അല്ലെങ്കിൽ{
        flags &= ~SQLITE_OPEN_URI;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( strcmp(zArg, "-translatefilename")==0 ){
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        തിരികെ TCL_ERROR;
      }
    }അല്ലെങ്കിൽ{
      Tcl_AppendResult(interp, "unknown option: ", zArg, (അക്ഷരം*)0);
      തിരികെ TCL_ERROR;
    }
  }
  z_Err_സന്ദേശം = 0;
  p = (SqliteDb*)Tcl_Alloc( വലുപ്പം(*p) );
  memset(p, 0, വലുപ്പം(*p));
  എങ്കിൽ( zFile==0 ) zFile = "";
  എങ്കിൽ( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  എങ്കിൽ( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  എങ്കിൽ( p->db ){
    എങ്കിൽ( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_സന്ദേശം = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }അല്ലെങ്കിൽ{
    z_Err_സന്ദേശം = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  എങ്കിൽ( p->db==0 ){
    Tcl_SetResult(interp, z_Err_സന്ദേശം, TCL_VOLATILE);
    Tcl_Free((അക്ഷരം*)p);
    sqlite3_free(z_Err_സന്ദേശം);
    തിരികെ TCL_ERROR;
  }
  p->maxStmt = അക്കം_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  എങ്കിൽ( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (അക്ഷരം*)p, DbDeleteCmd);
  }അല്ലെങ്കിൽ{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (അക്ഷരം*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  തിരികെ TCL_OK;
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
EXTERN പൂർണ്ണസംഖ്യ Sqlite3_Init(Tcl_Interp *interp){
  പൂർണ്ണസംഖ്യ rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  എങ്കിൽ( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  തിരികെ rc;
}
EXTERN പൂർണ്ണസംഖ്യ Tclsqlite3_Init(Tcl_Interp *interp){ തിരികെ Sqlite3_Init(interp); }
EXTERN പൂർണ്ണസംഖ്യ Sqlite3_Unload(Tcl_Interp *interp, പൂർണ്ണസംഖ്യ flags){ തിരികെ TCL_OK; }
EXTERN പൂർണ്ണസംഖ്യ Tclsqlite3_Unload(Tcl_Interp *interp, പൂർണ്ണസംഖ്യ flags){ തിരികെ TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN പൂർണ്ണസംഖ്യ Sqlite3_SafeInit(Tcl_Interp *interp){ തിരികെ TCL_ERROR; }
EXTERN പൂർണ്ണസംഖ്യ Sqlite3_SafeUnload(Tcl_Interp *interp, പൂർണ്ണസംഖ്യ flags){തിരികെ TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN പൂർണ്ണസംഖ്യ Sqlite_Init(Tcl_Interp *interp){ തിരികെ Sqlite3_Init(interp);}
EXTERN പൂർണ്ണസംഖ്യ Tclsqlite_Init(Tcl_Interp *interp){ തിരികെ Sqlite3_Init(interp); }
EXTERN പൂർണ്ണസംഖ്യ Sqlite_Unload(Tcl_Interp *interp, പൂർണ്ണസംഖ്യ flags){ തിരികെ TCL_OK; }
EXTERN പൂർണ്ണസംഖ്യ Tclsqlite_Unload(Tcl_Interp *interp, പൂർണ്ണസംഖ്യ flags){ തിരികെ TCL_OK; }
EXTERN പൂർണ്ണസംഖ്യ Sqlite_SafeInit(Tcl_Interp *interp){ തിരികെ TCL_ERROR; }
EXTERN പൂർണ്ണസംഖ്യ Sqlite_SafeUnload(Tcl_Interp *interp, പൂർണ്ണസംഖ്യ flags){തിരികെ TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN പൂർണ്ണസംഖ്യ sqlite3_Init(Tcl_Interp *interp){ തിരികെ Sqlite3_Init(interp);}
EXTERN പൂർണ്ണസംഖ്യ sqlite_Init(Tcl_Interp *interp){ തിരികെ Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#എങ്കിൽ defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *tclsh_main_loop(ശൂന്യം){
  സ്ഥിതം സ്ഥിരം അക്ഷരം zMainloop[] =
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
  തിരികെ zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN പ്രധാന
#endif
പൂർണ്ണസംഖ്യ SQLITE_CDECL TCLSH_MAIN(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv){
  Tcl_Interp *interp;
  പൂർണ്ണസംഖ്യ i;
  സ്ഥിരം അക്ഷരം *zScript = 0;
  അക്ഷരം zArgc[32];
#എങ്കിൽ defined(TCLSH_INIT_PROC)
  ബാഹ്യ സ്ഥിരം അക്ഷരം *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#എങ്കിൽ !defined(_WIN32_WCE)
  എങ്കിൽ( getenv("SQLITE_DEBUG_BREAK") ){
    എങ്കിൽ( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }അല്ലെങ്കിൽ{
#എങ്കിൽ defined(_WIN32) || defined(WIN32)
      DebugBreak();
#അല്ലെങ്കിലെങ്കിൽ defined(SIGTRAP)
      ഉയർത്തുക(SIGTRAP);
#endif
    }
  }
#endif

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_SetSystemEncoding(ശൂന്യ, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(വലുപ്പം(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  വേണ്ടി(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_മൂല്യം);
  }
#എങ്കിൽ defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  എങ്കിൽ( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  എങ്കിൽ( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    സ്ഥിരം അക്ഷരം *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    എങ്കിൽ( zInfo==0 ) zInfo = Tcl_GetStringResult(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    തിരികെ 1;
  }
  തിരികെ 0;
}
#endif /* TCLSH */
