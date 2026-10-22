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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TCLSH_INIT_PROC
# define TCLSH 1
#ਜੇ_ਅੰਤ

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#ਜੇ defined(INCLUDE_MSVC_H)
# include "msvc.h"
#ਜੇ_ਅੰਤ

/****** Copy of tclsqlite.h ******/
#ਜੇ defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#ਨਹੀਂਤਾਂ
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#ਜੇ_ਅੰਤ
/* Compatability between Tcl8.6 and Tcl9.0 */
#ਜੇ TCL_MAJOR_ਸੰਸਕਰਣ==9
# define CONST ਅਚਲ
#ਜਾਂਜੇ !defined(Tcl_ਮਾਪ)
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ Tcl_ਮਾਪ;
# ifndef Tcl_Bounce_Ref_ਗਿਣਤੀ
#  define Tcl_Bounce_Ref_ਗਿਣਤੀ(X) Tcl_Incr_Ref_ਗਿਣਤੀ(X); Tcl_Decr_Ref_ਗਿਣਤੀ(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#ਜੇ_ਅੰਤ
/**** End copy of tclsqlite.h ****/

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ u8;
# ifndef SQLITE_PTRSIZE
#   ਜੇ defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   ਨਹੀਂਤਾਂ
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# ਜੇ defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ uptr;
# ਨਹੀਂਤਾਂ
    ਕਿਸਮਪਰਿਭਾਸ਼ਾ sqlite3_uint64 uptr;
# endif
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>

/* Used to get the current process ID */
#ਜੇ !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#ਜਾਂਜੇ !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_ਔਸਤ
#   define WIN32_LEAN_AND_ਔਸਤ
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (ਪੂਰਨ)GetCurrentProcessId
#ਜੇ_ਅੰਤ

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BUILD_sqlite
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ TCL_STORAGE_CLASS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCL_STORAGE_CLASS DLLEXPORT
#ਜੇ_ਅੰਤ /* BUILD_sqlite */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_PREPARED_STMTS 10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_PREPARED_STMTS 100

/* Forward declaration */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#ਜੇ defined(SQLITE_ENABLE_QRF_ਵਿੱਚ_TCL)
#ਅਨ੍ਤਰ੍ਭਾਵ "qrf.h"
#ਜੇ_ਅੰਤ

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SqlFunc SqlFunc;
ਬਣਤਰ SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  ਪੂਰਨ useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  ਪੂਰਨ eType;            /* Type of value to return */
  ਅੱਖਰ *zName;          /* Name of this function */
  SqlFunc *p_ਅਗਲਾ;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SqlCollate SqlCollate;
ਬਣਤਰ SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  ਅੱਖਰ *zScript;        /* The script to be run */
  SqlCollate *p_ਅਗਲਾ;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SqlPreparedStmt SqlPreparedStmt;
ਬਣਤਰ SqlPreparedStmt {
  SqlPreparedStmt *p_ਅਗਲਾ;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  ਪੂਰਨ nSql;                /* chars in zSql[] */
  ਅਚਲ ਅੱਖਰ *zSql;        /* Text of the SQL statement */
  ਪੂਰਨ nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
ਬਣਤਰ SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  ਅੱਖਰ *zBusy;               /* The busy callback routine */
  ਅੱਖਰ *zCommit;             /* The commit hook callback routine */
  ਅੱਖਰ *zTrace;              /* The trace callback routine */
  ਅੱਖਰ *zTraceV2;            /* The trace_v2 callback routine */
  ਅੱਖਰ *zProfile;            /* The profile callback routine */
  ਅੱਖਰ *z_ਤਰੱਕੀ;           /* The progress callback routine */
  ਅੱਖਰ *zBindFallback;       /* Callback to invoke on a binding miss */
  ਅੱਖਰ *zAuth;               /* The authorization callback routine */
  ਪੂਰਨ disableAuth;           /* Disable the authorizer if it exists */
  ਅੱਖਰ *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  ਪੂਰਨ rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmt_ਆਖਰੀ; /* Last statement in the list */
  ਪੂਰਨ maxStmt;               /* The next maximum number of stmtList */
  ਪੂਰਨ nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  ਪੂਰਨ n_ਕਦਮ, n_ਛਾਂਟੋ, n_ਸੂਚਕਾਂਕ;  /* Statistics for most recent operation */
  ਪੂਰਨ nVMStep;               /* Another statistic for most recent operation */
  ਪੂਰਨ nTransaction;          /* Number of nested [transaction] methods */
  ਪੂਰਨ openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  ਪੂਰਨ nRef;                  /* Delete object when this reaches 0 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਪੂਰਨ bLegacyPrepare;        /* True to use sqlite3_prepare() */
#ਜੇ_ਅੰਤ
};

ਬਣਤਰ IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *p_ਅਗਲਾ;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
ਸਥਿਰ ਪੂਰਨ strlen30(ਅਚਲ ਅੱਖਰ *z){
  ਅਚਲ ਅੱਖਰ *z2 = z;
  ਜਦੋਂ( *z2 ){ z2++; }
  ਵਾਪਸ 0x3fffffff & (ਪੂਰਨ)(z2 - z);
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
ਸਥਿਰ ਖਾਲੀ closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *p_ਅਗਲਾ;

  ਲਈ(p=pDb->pIncrblob; p; p=p_ਅਗਲਾ){
    p_ਅਗਲਾ = p->p_ਅਗਲਾ;

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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI incrblobClose2(
  Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ flags
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ਅੰਕੜੇ;
  ਪੂਰਨ  rc;
  sqlite3 *db = p->pDb->db;

  ਜੇ( flags ){
    p->isClosed |= flags;
    ਵਾਪਸ TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  ਜੇ( p->p_ਅਗਲਾ ){
    p->p_ਅਗਲਾ->pPrev = p->pPrev;
  }
  ਜੇ( p->pPrev ){
    p->pPrev->p_ਅਗਲਾ = p->p_ਅਗਲਾ;
  }
  ਜੇ( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->p_ਅਗਲਾ;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((ਅੱਖਰ *)p);

  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3_errmsg(db), TCL_VOLATILE);
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI incrblobClose(
  Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ,
  Tcl_Interp *interp
){
  ਵਾਪਸ incrblobClose2(instance_ਅੰਕੜੇ, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI incrblob_ਨਿਵੇਸ਼(
  Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ,
  ਅੱਖਰ *buf,
  ਪੂਰਨ buf_ਮਾਪ,
  ਪੂਰਨ *error_ਕੋਡ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ਅੰਕੜੇ;
  sqlite3_int64 nRead = buf_ਮਾਪ;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  ਪੂਰਨ rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_ਬਾਈਟਾਂ(p->pBlob);
  ਜੇ( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  ਜੇ( nRead<=0 ){
    ਵਾਪਸ 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (ਖਾਲੀ *)buf, (ਪੂਰਨ)nRead, (ਪੂਰਨ)p->iSeek);
  ਜੇ( rc!=SQLITE_OK ){
    *error_ਕੋਡ_Ptr = rc;
    ਵਾਪਸ -1;
  }

  p->iSeek += nRead;
  ਵਾਪਸ nRead;
}

/*
** Write data to an incremental blob channel.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI incrblobOutput(
  Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ,
  ਅਚਲ ਅੱਖਰ *buf,
  ਪੂਰਨ ਮੰਜ਼ਿਲ_Write,
  ਪੂਰਨ *error_ਕੋਡ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ਅੰਕੜੇ;
  sqlite3_int64 nWrite = ਮੰਜ਼ਿਲ_Write;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  ਪੂਰਨ rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_ਬਾਈਟਾਂ(p->pBlob);
  ਜੇ( (p->iSeek+nWrite)>nBlob ){
    *error_ਕੋਡ_Ptr = EINVAL;
    ਵਾਪਸ -1;
  }
  ਜੇ( nWrite<=0 ){
    ਵਾਪਸ 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (ਖਾਲੀ*)buf,(ਪੂਰਨ)nWrite, (ਪੂਰਨ)p->iSeek);
  ਜੇ( rc!=SQLITE_OK ){
    *error_ਕੋਡ_Ptr = EIO;
    ਵਾਪਸ -1;
  }

  p->iSeek += nWrite;
  ਵਾਪਸ nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#ਜੇ TCL_MAJOR_ਸੰਸਕਰਣ==9
# define WideSeekProcType ਲੰਮਾ ਲੰਮਾ
#ਨਹੀਂਤਾਂ
# define WideSeekProcType Tcl_WideInt
#ਜੇ_ਅੰਤ

/*
** Seek an incremental blob channel.
*/
ਸਥਿਰ WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ,
  WideSeekProcType ਵਿੱਥ,
  ਪੂਰਨ seek_ਬਹੁਲਕ,
  ਪੂਰਨ *error_ਕੋਡ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ਅੰਕੜੇ;

  ਚੋਣ( seek_ਬਹੁਲਕ ){
    ਹਾਲਤ SEEK_SET:
      p->iSeek = ਵਿੱਥ;
      ਰੋਕੋ;
    ਹਾਲਤ SEEK_CUR:
      p->iSeek += ਵਿੱਥ;
      ਰੋਕੋ;
    ਹਾਲਤ SEEK_END:
      p->iSeek = sqlite3_blob_ਬਾਈਟਾਂ(p->pBlob) + ਵਿੱਥ;
      ਰੋਕੋ;

    ਮੂਲ: ਪ੍ਰਤਿਜ੍ਞਾ(!"Bad seekMode");
  }

  ਵਾਪਸ p->iSeek;
}
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI incrblobSeek(
  Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ,
  ਲੰਮਾ ਵਿੱਥ,
  ਪੂਰਨ seek_ਬਹੁਲਕ,
  ਪੂਰਨ *error_ਕੋਡ_Ptr
){
  ਵਾਪਸ incrblobWideSeek(instance_ਅੰਕੜੇ,ਵਿੱਥ,seek_ਬਹੁਲਕ,error_ਕੋਡ_Ptr);
}


ਸਥਿਰ ਖਾਲੀ SQLITE_TCLAPI incrblobWatch(
  Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ,
  ਪੂਰਨ ਬਹੁਲਕ
){
  /* NO-OP */
}
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI incrblobHandle(
  Client_ਅੰਕੜੇ instance_ਅੰਕੜੇ,
  ਪੂਰਨ dir,
  Client_ਅੰਕੜੇ *hPtr
){
  ਵਾਪਸ TCL_ERROR;
}

ਸਥਿਰ Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_ਸੰਸਕਰਣ_5,             /*ਸੰਸਕਰਣ*/
  incrblobClose,                     /* closeProc                            */
  incrblob_ਨਿਵੇਸ਼,                     /* inputProc                            */
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
ਸਥਿਰ ਪੂਰਨ createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  ਅਚਲ ਅੱਖਰ *zDb,
  ਅਚਲ ਅੱਖਰ *z_ਸਾਰਣੀ,
  ਅਚਲ ਅੱਖਰ *z_ਕਾਲਮ,
  sqlite_int64 i_ਪੰਕਤੀ,
  ਪੂਰਨ isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  ਪੂਰਨ rc;
  ਪੂਰਨ flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  ਸਥਿਰ ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
  ਅੱਖਰ zChannel[64];

  rc = sqlite3_blob_open(db, zDb, z_ਸਾਰਣੀ, z_ਕਾਲਮ, i_ਪੰਕਤੀ, !isReadonly, &pBlob);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    ਵਾਪਸ TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(ਆਕਾਰ(IncrblobChannel));
  memset(p, 0, ਆਕਾਰ(*p));
  p->pBlob = pBlob;
  ਜੇ( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(ਆਕਾਰ(zChannel), zChannel, "incrblob_%d", ++ਗਿਣਤੀ_2);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_ਰਜਿਸਟਰ_Channel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->p_ਅਗਲਾ = pDb->pIncrblob;
  p->pPrev = 0;
  ਜੇ( p->p_ਅਗਲਾ ){
    p->p_ਅਗਲਾ->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  ਵਾਪਸ TCL_OK;
}
#ਨਹੀਂਤਾਂ  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ closeIncrblobChannels(pDb)
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_Use_Eval_Objv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  ਅਚਲ ਅੱਖਰ *z;
  Tcl_ਮਾਪ n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  ਜਦੋਂ( n-- > 0 ){
    ਪੂਰਨ c = *(z++);
    ਜੇ( c=='$' || c=='[' || c==';' ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
ਸਥਿਰ SqlFunc *ਲੱਭੋ_Sql_Func(SqliteDb *pDb, ਅਚਲ ਅੱਖਰ *zName){
  SqlFunc *p, *pNew;
  ਪੂਰਨ nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( ਆਕਾਰ(*pNew) + nName + 1 );
  pNew->zName = (ਅੱਖਰ*)&pNew[1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew->zName, zName, nName+1);
  ਲਈ(p=pDb->pFunc; p; p=p->p_ਅਗਲਾ){
    ਜੇ( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((ਅੱਖਰ*)pNew);
      ਵਾਪਸ p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->p_ਅਗਲਾ = pDb->pFunc;
  pDb->pFunc = pNew;
  ਵਾਪਸ pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
ਸਥਿਰ ਖਾਲੀ dbFreeStmt(SqlPreparedStmt *pStmt){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਜੇ( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((ਅੱਖਰ *)pStmt->zSql);
  }
#ਜੇ_ਅੰਤ
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((ਅੱਖਰ *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
ਸਥਿਰ ਖਾਲੀ flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *p_ਅਗਲਾ;

  ਲਈ(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=p_ਅਗਲਾ){
    p_ਅਗਲਾ = pPreStmt->p_ਅਗਲਾ;
    dbFreeStmt(pPreStmt);
  }
  pDb->nStmt = 0;
  pDb->stmt_ਆਖਰੀ = 0;
  pDb->stmtList = 0;
}

/*
** Increment the reference counter on the SqliteDb object. The reference
** should be released by calling delDatabaseRef().
*/
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_Database_Ref(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
ਸਥਿਰ ਖਾਲੀ delDatabaseRef(SqliteDb *pDb){
  ਪ੍ਰਤਿਜ੍ਞਾ( pDb->nRef>0 );
  pDb->nRef--;
  ਜੇ( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    ਜਦੋਂ( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->p_ਅਗਲਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pFunc->pDb==pDb );
      Tcl_Decr_Ref_ਗਿਣਤੀ(pFunc->pScript);
      Tcl_Free((ਅੱਖਰ*)pFunc);
    }
    ਜਦੋਂ( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->p_ਅਗਲਾ;
      Tcl_Free((ਅੱਖਰ*)pCollate);
    }
    ਜੇ( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    ਜੇ( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    ਜੇ( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    ਜੇ( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    ਜੇ( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    ਜੇ( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    ਜੇ( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    ਜੇ( pDb->pUpdateHook ){
      Tcl_Decr_Ref_ਗਿਣਤੀ(pDb->pUpdateHook);
    }
    ਜੇ( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_ਗਿਣਤੀ(pDb->pPreUpdateHook);
    }
    ਜੇ( pDb->pRollbackHook ){
      Tcl_Decr_Ref_ਗਿਣਤੀ(pDb->pRollbackHook);
    }
    ਜੇ( pDb->pWalHook ){
      Tcl_Decr_Ref_ਗਿਣਤੀ(pDb->pWalHook);
    }
    ਜੇ( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_ਗਿਣਤੀ(pDb->pCollateNeeded);
    }
    Tcl_Free((ਅੱਖਰ*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
ਸਥਿਰ ਖਾਲੀ SQLITE_TCLAPI DbDeleteCmd(ਖਾਲੀ *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
ਸਥਿਰ ਪੂਰਨ DbBusyHandler(ਖਾਲੀ *cd, ਪੂਰਨ nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  ਪੂਰਨ rc;
  ਅੱਖਰ zVal[30];

  sqlite3_snprintf(ਆਕਾਰ(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (ਅੱਖਰ*)0);
  ਜੇ( rc!=TCL_OK || atoi(Tcl_Get_String_ਨਤੀਜਾ(pDb->interp)) ){
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
ਸਥਿਰ ਪੂਰਨ Db_ਤਰੱਕੀ_Handler(ਖਾਲੀ *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( pDb->z_ਤਰੱਕੀ );
  rc = Tcl_Eval(pDb->interp, pDb->z_ਤਰੱਕੀ);
  ਜੇ( rc!=TCL_OK || atoi(Tcl_Get_String_ਨਤੀਜਾ(pDb->interp)) ){
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ਬਿੰਦੂ) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
ਸਥਿਰ ਖਾਲੀ DbTraceHandler(ਖਾਲੀ *cd, ਅਚਲ ਅੱਖਰ *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_ਮੁੱਲ(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_ਨਤੀਜਾ(pDb->interp);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
ਸਥਿਰ ਪੂਰਨ DbTraceV2Handler(
  ਬਿਨਾਨਿਸ਼ਾਨੀ type, /* One of the SQLITE_TRACE_* event types. */
  ਖਾਲੀ *cd,      /* The original context data pointer. */
  ਖਾਲੀ *pd,      /* Primary event data, depends on event type. */
  ਖਾਲੀ *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  ਚੋਣ( type ){
    ਹਾਲਤ SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      ਅੱਖਰ *zSql = (ਅੱਖਰ *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
      Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
      Tcl_Reset_ਨਤੀਜਾ(pDb->interp);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
      Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
      Tcl_Reset_ਨਤੀਜਾ(pDb->interp);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_TRACE_ਪੰਕਤੀ: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
      Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
      Tcl_Reset_ਨਤੀਜਾ(pDb->interp);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
      Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
      Tcl_Reset_ਨਤੀਜਾ(pDb->interp);
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ਬਿੰਦੂ) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
ਸਥਿਰ ਖਾਲੀ DbProfileHandler(ਖਾਲੀ *cd, ਅਚਲ ਅੱਖਰ *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  ਅੱਖਰ zTm[100];

  sqlite3_snprintf(ਆਕਾਰ(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_ਮੁੱਲ(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_ਨਤੀਜਾ(pDb->interp);
}
#ਜੇ_ਅੰਤ

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
ਸਥਿਰ ਪੂਰਨ DbCommitHandler(ਖਾਲੀ *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  ਪੂਰਨ rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  ਜੇ( rc!=TCL_OK || atoi(Tcl_Get_String_ਨਤੀਜਾ(pDb->interp)) ){
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ DbRollbackHandler(ਖਾਲੀ *client_ਅੰਕੜੇ){
  SqliteDb *pDb = (SqliteDb*)client_ਅੰਕੜੇ;
  ਪ੍ਰਤਿਜ੍ਞਾ(pDb->pRollbackHook);
  ਜੇ( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
ਸਥਿਰ ਪੂਰਨ DbWalHandler(
  ਖਾਲੀ *client_ਅੰਕੜੇ,
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zDb,
  ਪੂਰਨ n_ਇੰਦਰਾਜ
){
  ਪੂਰਨ ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)client_ਅੰਕੜੇ;
  Tcl_Interp *interp = pDb->interp;
  ਪ੍ਰਤਿਜ੍ਞਾ(pDb->pWalHook);

  ਪ੍ਰਤਿਜ੍ਞਾ( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_ਗਿਣਤੀ(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(n_ਇੰਦਰਾਜ));
  ਜੇ( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_Get_Obj_ਨਤੀਜਾ(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_ਗਿਣਤੀ(p);

  ਵਾਪਸ ret;
}

#ਜੇ defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Test_Unlock_Notify_Vars(Tcl_Interp *interp, ਪੂਰਨ iArg, ਪੂਰਨ nArg){
  ਅੱਖਰ zBuf[64];
  sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "%d", iArg);
  Tcl_ਸੈੱਟ_Var(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(ਆਕਾਰ(zBuf), zBuf, "%d", nArg);
  Tcl_ਸੈੱਟ_Var(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#ਨਹੀਂਤਾਂ
# define ਸੈੱਟ_Test_Unlock_Notify_Vars(x,y,z)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNLOCK_NOTIFY
ਸਥਿਰ ਖਾਲੀ DbUnlockNotify(ਖਾਲੀ **apArg, ਪੂਰਨ nArg){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<nArg; i++){
    ਅਚਲ ਪੂਰਨ flags = (TCL_EVAL_GLOBAL|TCL_EVAL_ਸਿੱਧਾ);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    ਸੈੱਟ_Test_Unlock_Notify_Vars(pDb->interp, i, nArg);
    ਪ੍ਰਤਿਜ੍ਞਾ( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_ਗਿਣਤੀ(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
ਸਥਿਰ ਖਾਲੀ DbPreUpdateHandler(
  ਖਾਲੀ *p,
  sqlite3 *db,
  ਪੂਰਨ op,
  ਅਚਲ ਅੱਖਰ *zDb,
  ਅਚਲ ਅੱਖਰ *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ਪ੍ਰਤਿਜ੍ਞਾ( (SQLITE_DELETE-1)/9 == 0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (SQLITE_ਪਾਓ-1)/9 == 1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (SQLITE_UPDATE-1)/9 == 2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pDb->pPreUpdateHook );
  ਪ੍ਰਤਿਜ੍ਞਾ( db==pDb->db );
  ਪ੍ਰਤਿਜ੍ਞਾ( op==SQLITE_ਪਾਓ || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */

ਸਥਿਰ ਖਾਲੀ DbUpdateHandler(
  ਖਾਲੀ *p,
  ਪੂਰਨ op,
  ਅਚਲ ਅੱਖਰ *zDb,
  ਅਚਲ ਅੱਖਰ *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ਪ੍ਰਤਿਜ੍ਞਾ( (SQLITE_DELETE-1)/9 == 0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (SQLITE_ਪਾਓ-1)/9 == 1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (SQLITE_UPDATE-1)/9 == 2 );

  ਪ੍ਰਤਿਜ੍ਞਾ( pDb->pUpdateHook );
  ਪ੍ਰਤਿਜ੍ਞਾ( op==SQLITE_ਪਾਓ || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
}

ਸਥਿਰ ਖਾਲੀ tclCollateNeeded(
  ਖਾਲੀ *pCtx,
  sqlite3 *db,
  ਪੂਰਨ enc,
  ਅਚਲ ਅੱਖਰ *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
ਸਥਿਰ ਪੂਰਨ tclSqlCollate(
  ਖਾਲੀ *pCtx,
  ਪੂਰਨ nA,
  ਅਚਲ ਖਾਲੀ *zA,
  ਪੂਰਨ nB,
  ਅਚਲ ਖਾਲੀ *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
  ਵਾਪਸ (atoi(Tcl_Get_String_ਨਤੀਜਾ(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
ਸਥਿਰ ਖਾਲੀ tclSqlFunc(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ**argv){
  SqlFunc *p = sqlite3_user_ਅੰਕੜੇ(context);
  Tcl_Obj *pCmd;
  ਪੂਰਨ i;
  ਪੂਰਨ rc;

  ਜੇ( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
  }ਨਹੀਂਤਾਂ{
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
    Tcl_ਮਾਪ nArg;
    ਜੇ( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_ਨਤੀਜਾ_error(context, Tcl_Get_String_ਨਤੀਜਾ(p->interp), -1);
      ਵਾਪਸ;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
    ਲਈ(i=0; i<argc; i++){
      sqlite3_ਮੁੱਲ *p_ਵਿੱਚ = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      ਚੋਣ( sqlite3_ਮੁੱਲ_type(p_ਵਿੱਚ) ){
        ਹਾਲਤ SQLITE_BLOB: {
          ਪੂਰਨ ਬਾਈਟਾਂ = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(p_ਵਿੱਚ);
          pVal = Tcl_NewByteArrayObj(sqlite3_ਮੁੱਲ_blob(p_ਵਿੱਚ), ਬਾਈਟਾਂ);
          ਰੋਕੋ;
        }
        ਹਾਲਤ SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_ਮੁੱਲ_int64(p_ਵਿੱਚ);
          ਜੇ( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((ਪੂਰਨ)v);
          }ਨਹੀਂਤਾਂ{
            pVal = Tcl_NewWideIntObj(v);
          }
          ਰੋਕੋ;
        }
        ਹਾਲਤ SQLITE_FLOAT: {
          ਦੁੱਗਣਾ r = sqlite3_ਮੁੱਲ_double(p_ਵਿੱਚ);
          pVal = Tcl_NewDoubleObj(r);
          ਰੋਕੋ;
        }
        ਹਾਲਤ SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          ਰੋਕੋ;
        }
        ਮੂਲ: {
          ਪੂਰਨ ਬਾਈਟਾਂ = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(p_ਵਿੱਚ);
          pVal = Tcl_NewStringObj((ਅੱਖਰ *)sqlite3_ਮੁੱਲ_ਪਾਠ(p_ਵਿੱਚ), ਬਾਈਟਾਂ);
          ਰੋਕੋ;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      ਜੇ( rc ){
        Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
        sqlite3_ਨਤੀਜਾ_error(context, Tcl_Get_String_ਨਤੀਜਾ(p->interp), -1);
        ਵਾਪਸ;
      }
    }
    ਜੇ( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
    Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
  }

  ਜੇ( TCL_BREAK==rc ){
    sqlite3_ਨਤੀਜਾ_null(context);
  }ਨਹੀਂਤਾਂ ਜੇ( rc && rc!=TCL_RETURN ){
    sqlite3_ਨਤੀਜਾ_error(context, Tcl_Get_String_ਨਤੀਜਾ(p->interp), -1);
  }ਨਹੀਂਤਾਂ{
    Tcl_Obj *pVar = Tcl_Get_Obj_ਨਤੀਜਾ(p->interp);
    Tcl_ਮਾਪ n;
    u8 *ਅੰਕੜੇ;
    ਅਚਲ ਅੱਖਰ *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    ਅੱਖਰ c = zType[0];
    ਪੂਰਨ eType = p->eType;

    ਜੇ( eType==SQLITE_NULL ){
      ਜੇ( c=='b' && strcmp(zType,"bytearray")==0 && pVar->ਬਾਈਟਾਂ==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }ਨਹੀਂਤਾਂ ਜੇ( (c=='b' && pVar->ਬਾਈਟਾਂ==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->ਬਾਈਟਾਂ==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }ਨਹੀਂਤਾਂ ਜੇ( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }ਨਹੀਂਤਾਂ{
        eType = SQLITE_ਪਾਠ;
      }
    }

    ਚੋਣ( eType ){
      ਹਾਲਤ SQLITE_BLOB: {
        ਅੰਕੜੇ = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_ਨਤੀਜਾ_blob64(context, ਅੰਕੜੇ, n, SQLITE_TRANSIENT);
        ਰੋਕੋ;
      }
      ਹਾਲਤ SQLITE_INTEGER: {
        Tcl_WideInt v;
        ਜੇ( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_ਨਤੀਜਾ_int64(context, v);
          ਰੋਕੋ;
        }
        /* fall-through */
      }
      ਹਾਲਤ SQLITE_FLOAT: {
        ਦੁੱਗਣਾ r;
        ਜੇ( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_ਨਤੀਜਾ_double(context, r);
          ਰੋਕੋ;
        }
        /* fall-through */
      }
      ਮੂਲ: {
        ਅੰਕੜੇ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_ਨਤੀਜਾ_text64(context, (ਅੱਖਰ *)ਅੰਕੜੇ, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        ਰੋਕੋ;
      }
    }

  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
ਸਥਿਰ ਪੂਰਨ auth_callback(
  ਖਾਲੀ *pArg,
  ਪੂਰਨ ਕੋਡ,
  ਅਚਲ ਅੱਖਰ *zArg1,
  ਅਚਲ ਅੱਖਰ *zArg2,
  ਅਚਲ ਅੱਖਰ *zArg3,
  ਅਚਲ ਅੱਖਰ *zArg4
){
  ਅਚਲ ਅੱਖਰ *z_ਕੋਡ;
  Tcl_DString str;
  ਪੂਰਨ rc;
  ਅਚਲ ਅੱਖਰ *z_ਜਵਾਬ;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  ਜੇ( pDb->disableAuth ) ਵਾਪਸ SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  ਚੋਣ( ਕੋਡ ){
    ਹਾਲਤ SQLITE_COPY              : z_ਕੋਡ="SQLITE_COPY"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_ਸੂਚਕਾਂਕ      : z_ਕੋਡ="SQLITE_CREATE_INDEX"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_ਸਾਰਣੀ      : z_ਕੋਡ="SQLITE_CREATE_TABLE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_ਆਰਜ਼ੀ_ਸੂਚਕਾਂਕ : z_ਕੋਡ="SQLITE_CREATE_TEMP_INDEX"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_ਆਰਜ਼ੀ_ਸਾਰਣੀ : z_ਕੋਡ="SQLITE_CREATE_TEMP_TABLE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_ਆਰਜ਼ੀ_TRIGGER: z_ਕੋਡ="SQLITE_CREATE_TEMP_TRIGGER"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_ਆਰਜ਼ੀ_VIEW  : z_ਕੋਡ="SQLITE_CREATE_TEMP_VIEW"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_TRIGGER    : z_ਕੋਡ="SQLITE_CREATE_TRIGGER"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_VIEW       : z_ਕੋਡ="SQLITE_CREATE_VIEW"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DELETE            : z_ਕੋਡ="SQLITE_DELETE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_ਸੂਚਕਾਂਕ        : z_ਕੋਡ="SQLITE_DROP_INDEX"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_ਸਾਰਣੀ        : z_ਕੋਡ="SQLITE_DROP_TABLE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_ਆਰਜ਼ੀ_ਸੂਚਕਾਂਕ   : z_ਕੋਡ="SQLITE_DROP_TEMP_INDEX"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_ਆਰਜ਼ੀ_ਸਾਰਣੀ   : z_ਕੋਡ="SQLITE_DROP_TEMP_TABLE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_ਆਰਜ਼ੀ_TRIGGER : z_ਕੋਡ="SQLITE_DROP_TEMP_TRIGGER"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_ਆਰਜ਼ੀ_VIEW    : z_ਕੋਡ="SQLITE_DROP_TEMP_VIEW"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_TRIGGER      : z_ਕੋਡ="SQLITE_DROP_TRIGGER"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_VIEW         : z_ਕੋਡ="SQLITE_DROP_VIEW"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_ਪਾਓ            : z_ਕੋਡ="SQLITE_INSERT"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_PRAGMA            : z_ਕੋਡ="SQLITE_PRAGMA"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_READ              : z_ਕੋਡ="SQLITE_READ"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_SELECT            : z_ਕੋਡ="SQLITE_SELECT"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_TRANSACTION       : z_ਕੋਡ="SQLITE_TRANSACTION"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_UPDATE            : z_ਕੋਡ="SQLITE_UPDATE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_ATTACH            : z_ਕੋਡ="SQLITE_ATTACH"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DETACH            : z_ਕੋਡ="SQLITE_DETACH"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_ALTER_ਸਾਰਣੀ       : z_ਕੋਡ="SQLITE_ALTER_TABLE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_REINDEX           : z_ਕੋਡ="SQLITE_REINDEX"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_ANALYZE           : z_ਕੋਡ="SQLITE_ANALYZE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_CREATE_VTABLE     : z_ਕੋਡ="SQLITE_CREATE_VTABLE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_DROP_VTABLE       : z_ਕੋਡ="SQLITE_DROP_VTABLE"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_FUNCTION          : z_ਕੋਡ="SQLITE_FUNCTION"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_SAVEPOINT         : z_ਕੋਡ="SQLITE_SAVEPOINT"; ਰੋਕੋ;
    ਹਾਲਤ SQLITE_RECURSIVE         : z_ਕੋਡ="SQLITE_RECURSIVE"; ਰੋਕੋ;
    ਮੂਲ                       : z_ਕੋਡ="????"; ਰੋਕੋ;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, z_ਕੋਡ);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_ਮੁੱਲ(&str));
  Tcl_DStringFree(&str);
  z_ਜਵਾਬ = rc==TCL_OK ? Tcl_Get_String_ਨਤੀਜਾ(pDb->interp) : "SQLITE_DENY";
  ਜੇ( strcmp(z_ਜਵਾਬ,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ ਜੇ( strcmp(z_ਜਵਾਬ,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }ਨਹੀਂਤਾਂ ਜੇ( strcmp(z_ਜਵਾਬ,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }ਨਹੀਂਤਾਂ{
    rc = 999;
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_AUTHORIZATION */

#ਜੇ 0
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
ਸਥਿਰ ਅੱਖਰ *local_getline(ਅੱਖਰ *zPrompt, FILE *ਵਿੱਚ){
  ਅੱਖਰ *z_ਸਤਰ;
  ਪੂਰਨ n_ਸਤਰ;
  ਪੂਰਨ n;

  n_ਸਤਰ = 100;
  z_ਸਤਰ = ਸਮ੍ਰਿਤੀ_ਦਿਓ( n_ਸਤਰ );
  ਜੇ( z_ਸਤਰ==0 ) ਵਾਪਸ 0;
  n = 0;
  ਜਦੋਂ( 1 ){
    ਜੇ( n+100>n_ਸਤਰ ){
      n_ਸਤਰ = n_ਸਤਰ*2 + 100;
      z_ਸਤਰ = realloc(z_ਸਤਰ, n_ਸਤਰ);
      ਜੇ( z_ਸਤਰ==0 ) ਵਾਪਸ 0;
    }
    ਜੇ( fgets(&z_ਸਤਰ[n], n_ਸਤਰ - n, ਵਿੱਚ)==0 ){
      ਜੇ( n==0 ){
        ਮੁਕਤ(z_ਸਤਰ);
        ਵਾਪਸ 0;
      }
      z_ਸਤਰ[n] = 0;
      ਰੋਕੋ;
    }
    ਜਦੋਂ( z_ਸਤਰ[n] ){ n++; }
    ਜੇ( n>0 && z_ਸਤਰ[n-1]=='\n' ){
      n--;
      z_ਸਤਰ[n] = 0;
      ਰੋਕੋ;
    }
  }
  z_ਸਤਰ = realloc( z_ਸਤਰ, n+1 );
  ਵਾਪਸ z_ਸਤਰ;
}
#ਜੇ_ਅੰਤ


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI DbTransPostCmd(
  Client_ਅੰਕੜੇ ਅੰਕੜੇ[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  ਪੂਰਨ ਨਤੀਜਾ                           /* Result of evaluating SCRIPT */
){
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ az_ਸਿਰਾ[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)ਅੰਕੜੇ[0];
  ਪੂਰਨ rc = ਨਤੀਜਾ;
  ਅਚਲ ਅੱਖਰ *z_ਸਿਰਾ;

  pDb->nTransaction--;
  z_ਸਿਰਾ = az_ਸਿਰਾ[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  ਜੇ( sqlite3_exec(pDb->db, z_ਸਿਰਾ, 0, 0, 0) ){
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
    ਜੇ( rc!=TCL_ERROR ){
      Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  ਵਾਪਸ rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
ਸਥਿਰ ਪੂਰਨ dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  ਅਚਲ ਅੱਖਰ *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  ਅਚਲ ਅੱਖਰ **pz_ਬਾਹਰਲਾ              /* OUT: Pointer to next SQL statement */
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ prepFlags = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਜੇ( pDb->bLegacyPrepare ){
    ਵਾਪਸ sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pz_ਬਾਹਰਲਾ);
  }
#ਜੇ_ਅੰਤ
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  ਜੇ( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  ਵਾਪਸ sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pz_ਬਾਹਰਲਾ);
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
ਸਥਿਰ ਪੂਰਨ dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  ਅੱਖਰ ਅਚਲ *z_ਵਿੱਚ,                /* SQL to compile */
  ਅੱਖਰ ਅਚਲ **pz_ਬਾਹਰਲਾ,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  ਅਚਲ ਅੱਖਰ *zSql = z_ਵਿੱਚ;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  ਪੂਰਨ nSql;                       /* Length of zSql in bytes */
  ਪੂਰਨ nVar = 0;                   /* Number of variables in statement */
  ਪੂਰਨ iParm = 0;                  /* Next free entry in apParm */
  ਅੱਖਰ c;
  ਪੂਰਨ i;
  ਪੂਰਨ need_ਨਤੀਜਾ_Reset = 0;        /* Need to invoke Tcl_ResetResult() */
  ਪੂਰਨ rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  ਜਦੋਂ( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  ਲਈ(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->p_ਅਗਲਾ){
    ਪੂਰਨ n = pPreStmt->nSql;
    ਜੇ( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pz_ਬਾਹਰਲਾ = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      ਜੇ( pPreStmt->pPrev ){
        pPreStmt->pPrev->p_ਅਗਲਾ = pPreStmt->p_ਅਗਲਾ;
      }ਨਹੀਂਤਾਂ{
        pDb->stmtList = pPreStmt->p_ਅਗਲਾ;
      }
      ਜੇ( pPreStmt->p_ਅਗਲਾ ){
        pPreStmt->p_ਅਗਲਾ->pPrev = pPreStmt->pPrev;
      }ਨਹੀਂਤਾਂ{
        pDb->stmt_ਆਖਰੀ = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_ਗਿਣਤੀ(pStmt);
      ਰੋਕੋ;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  ਜੇ( pPreStmt==0 ){
    ਪੂਰਨ nByte;

    ਜੇ( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pz_ਬਾਹਰਲਾ) ){
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( pStmt==0 ){
      ਜੇ( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ਵਾਪਸ TCL_ERROR;
      }ਨਹੀਂਤਾਂ{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        ਵਾਪਸ TCL_OK;
      }
    }

    ਪ੍ਰਤਿਜ੍ਞਾ( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_ਗਿਣਤੀ(pStmt);
    nByte = ਆਕਾਰ(SqlPreparedStmt) + nVar*ਆਕਾਰ(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (ਪੂਰਨ)(*pz_ਬਾਹਰਲਾ - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
    ਜੇ( pPreStmt->zSql==0 ){
      ਅੱਖਰ *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#ਜੇ_ਅੰਤ
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pPreStmt );
  ਪ੍ਰਤਿਜ੍ਞਾ( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  ਲਈ(i=1; i<=nVar; i++){
    ਅਚਲ ਅੱਖਰ *zVar = sqlite3_bind_parameter_name(pStmt, i);
    ਜੇ( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      ਜੇ( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        ਪੂਰਨ rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_ਗਿਣਤੀ(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        ਜੇ( need_ਨਤੀਜਾ_Reset ) Tcl_Reset_ਨਤੀਜਾ(interp);
        need_ਨਤੀਜਾ_Reset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_ਸਿੱਧਾ);
        Tcl_Decr_Ref_ਗਿਣਤੀ(pCmd);
        ਜੇ( rx==TCL_OK ){
          pVar = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
        }ਨਹੀਂਤਾਂ ਜੇ( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ{
          pVar = 0;
        }
      }
      ਜੇ( pVar ){
        Tcl_ਮਾਪ n;
        u8 *ਅੰਕੜੇ;
        ਅਚਲ ਅੱਖਰ *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        ਜੇ( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->ਬਾਈਟਾਂ==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          ਅੰਕੜੇ = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, ਅੰਕੜੇ, n, SQLITE_STATIC);
          Tcl_Incr_Ref_ਗਿਣਤੀ(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }ਨਹੀਂਤਾਂ ਜੇ( c=='b' && pVar->ਬਾਈਟਾਂ==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          ਪੂਰਨ nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }ਨਹੀਂਤਾਂ ਜੇ( c=='d' && strcmp(zType,"double")==0 ){
          ਦੁੱਗਣਾ r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }ਨਹੀਂਤਾਂ ਜੇ( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }ਨਹੀਂਤਾਂ{
          ਅੰਕੜੇ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (ਅੱਖਰ *)ਅੰਕੜੇ, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_ਗਿਣਤੀ(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }ਨਹੀਂਤਾਂ{
        sqlite3_bind_null(pStmt, i);
      }
      ਜੇ( need_ਨਤੀਜਾ_Reset ) Tcl_Reset_ਨਤੀਜਾ(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  ਜੇ( need_ਨਤੀਜਾ_Reset && rc==TCL_OK ) Tcl_Reset_ਨਤੀਜਾ(pDb->interp);

  ਵਾਪਸ rc;
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
ਸਥਿਰ ਖਾਲੀ dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  ਪੂਰਨ discard                     /* True to delete (not cache) the pPreStmt */
){
  ਪੂਰਨ i;

  /* Free the bound string and blob parameters */
  ਲਈ(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_ਗਿਣਤੀ(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  ਜੇ( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }ਨਹੀਂਤਾਂ{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->p_ਅਗਲਾ = pDb->stmtList;
    pPreStmt->pPrev = 0;
    ਜੇ( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    ਜੇ( pDb->stmt_ਆਖਰੀ==0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pDb->nStmt==0 );
      pDb->stmt_ਆਖਰੀ = pPreStmt;
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    ਜਦੋਂ( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *p_ਆਖਰੀ = pDb->stmt_ਆਖਰੀ;
      pDb->stmt_ਆਖਰੀ = p_ਆਖਰੀ->pPrev;
      pDb->stmt_ਆਖਰੀ->p_ਅਗਲਾ = 0;
      pDb->nStmt--;
      dbFreeStmt(p_ਆਖਰੀ);
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ DbEvalContext DbEvalContext;
ਬਣਤਰ DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  ਅਚਲ ਅੱਖਰ *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  ਪੂਰਨ n_ਖਾਨਾ;                       /* Number of columns returned by pStmt */
  ਪੂਰਨ evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **ap_ਖਾਨਾ_Name;            /* Array of column names */
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
ਸਥਿਰ ਖਾਲੀ db_Release_ਕਾਲਮ_ਨਾਂ(DbEvalContext *p){
  ਜੇ( p->ap_ਖਾਨਾ_Name ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<p->n_ਖਾਨਾ; i++){
      Tcl_Decr_Ref_ਗਿਣਤੀ(p->ap_ਖਾਨਾ_Name[i]);
    }
    Tcl_Free((ਅੱਖਰ *)p->ap_ਖਾਨਾ_Name);
    p->ap_ਖਾਨਾ_Name = 0;
  }
  p->n_ਖਾਨਾ = 0;
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
ਸਥਿਰ ਖਾਲੀ dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  ਪੂਰਨ evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, ਆਕਾਰ(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_ਗਿਣਤੀ(pSql);
  ਜੇ( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_ਗਿਣਤੀ(pVarName);
  }
  p->evalFlags = evalFlags;
  ਜੋੜੋ_Database_Ref(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
ਸਥਿਰ ਖਾਲੀ db_Eval_ਪੰਕਤੀ_Info(
  DbEvalContext *p,               /* Evaluation context */
  ਪੂਰਨ *pn_ਖਾਨਾ,                     /* OUT: Number of column names */
  Tcl_Obj ***pap_ਖਾਨਾ_Name           /* OUT: Array of column names */
){
  /* Compute column names */
  ਜੇ( 0==p->ap_ਖਾਨਾ_Name ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    ਪੂਰਨ i;                        /* Iterator variable */
    ਪੂਰਨ n_ਖਾਨਾ;                     /* Number of columns returned by pStmt */
    Tcl_Obj **ap_ਖਾਨਾ_Name = 0;      /* Array of column names */

    p->n_ਖਾਨਾ = n_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਗਿਣਤੀ(pStmt);
    ਜੇ( n_ਖਾਨਾ>0 && (pap_ਖਾਨਾ_Name || p->pVarName) ){
      ap_ਖਾਨਾ_Name = (Tcl_Obj**)Tcl_Alloc( ਆਕਾਰ(Tcl_Obj*)*n_ਖਾਨਾ );
      ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
        ap_ਖਾਨਾ_Name[i] = Tcl_NewStringObj(sqlite3_ਕਾਲਮ_name(pStmt,i), -1);
        Tcl_Incr_Ref_ਗਿਣਤੀ(ap_ਖਾਨਾ_Name[i]);
      }
      p->ap_ਖਾਨਾ_Name = ap_ਖਾਨਾ_Name;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    ਜੇ( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *p_ਖਾਨਾ_List = Tcl_NewObj();
      Tcl_Obj *p_ਤਾਰਾ = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_ਗਿਣਤੀ(p_ਖਾਨਾ_List);
      Tcl_Incr_Ref_ਗਿਣਤੀ(p_ਤਾਰਾ);
      ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
        Tcl_ListObjAppendElement(interp, p_ਖਾਨਾ_List, ap_ਖਾਨਾ_Name[i]);
      }
      ਜੇ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_Obj_ਸੈੱਟ_Var2(interp, p->pVarName, p_ਤਾਰਾ, p_ਖਾਨਾ_List, 0);
      }ਨਹੀਂਤਾਂ{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, ਨਿਰਾ, 0);
        ਜੇ( !pDict ){
          pDict = Tcl_NewDictObj();
        }ਨਹੀਂਤਾਂ ਜੇ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ਜੇ( Tcl_DictObjPut(interp, pDict, p_ਤਾਰਾ, p_ਖਾਨਾ_List)==TCL_OK ){
          Tcl_Obj_ਸੈੱਟ_Var2(interp, p->pVarName, ਨਿਰਾ, pDict, 0);
        }
        Tcl_Bounce_Ref_ਗਿਣਤੀ(pDict);
      }
      Tcl_Decr_Ref_ਗਿਣਤੀ(p_ਤਾਰਾ);
      Tcl_Decr_Ref_ਗਿਣਤੀ(p_ਖਾਨਾ_List);
    }
  }

  ਜੇ( pap_ਖਾਨਾ_Name ){
    *pap_ਖਾਨਾ_Name = p->ap_ਖਾਨਾ_Name;
  }
  ਜੇ( pn_ਖਾਨਾ ){
    *pn_ਖਾਨਾ = p->n_ਖਾਨਾ;
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
ਸਥਿਰ ਪੂਰਨ db_Eval_ਕਦਮ(DbEvalContext *p){
  ਅਚਲ ਅੱਖਰ *zPrevSql = 0;       /* Previous value of p->zSql */

  ਜਦੋਂ( p->zSql[0] || p->pPreStmt ){
    ਪੂਰਨ rc;
    ਜੇ( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      ਜੇ( rc!=TCL_OK ) ਵਾਪਸ rc;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_ਕਦਮ(pStmt);
      ਜੇ( rcs==SQLITE_ਪੰਕਤੀ ){
        ਵਾਪਸ TCL_OK;
      }
      ਜੇ( p->pVarName ){
        db_Eval_ਪੰਕਤੀ_Info(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->n_ਕਦਮ = sqlite3_stmt_ਸਥਿਤੀ(pStmt,SQLITE_STMTSTATUS_FULLSCAN_ਕਦਮ,1);
      pDb->n_ਛਾਂਟੋ = sqlite3_stmt_ਸਥਿਤੀ(pStmt,SQLITE_STMTSTATUS_ਛਾਂਟੋ,1);
      pDb->n_ਸੂਚਕਾਂਕ = sqlite3_stmt_ਸਥਿਤੀ(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_ਸਥਿਤੀ(pStmt,SQLITE_STMTSTATUS_VM_ਕਦਮ,1);
      db_Release_ਕਾਲਮ_ਨਾਂ(p);
      p->pPreStmt = 0;

      ਜੇ( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#ਜੇ SQLITE_TEST
        ਜੇ( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          ਜਾਰੀ;
        }
#ਜੇ_ਅੰਤ
        Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ਵਾਪਸ TCL_ERROR;
      }ਨਹੀਂਤਾਂ{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /*ਮੁਕੰਮਲ*/
  ਵਾਪਸ TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
ਸਥਿਰ ਖਾਲੀ dbEvalFinalize(DbEvalContext *p){
  ਜੇ( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  ਜੇ( p->pVarName ){
    Tcl_Decr_Ref_ਗਿਣਤੀ(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_ਗਿਣਤੀ(p->pSql);
  db_Release_ਕਾਲਮ_ਨਾਂ(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
ਸਥਿਰ Tcl_Obj *db_Eval_ਕਾਲਮ_ਮੁੱਲ(DbEvalContext *p, ਪੂਰਨ i_ਖਾਨਾ){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  ਚੋਣ( sqlite3_ਕਾਲਮ_type(pStmt, i_ਖਾਨਾ) ){
    ਹਾਲਤ SQLITE_BLOB: {
      ਪੂਰਨ ਬਾਈਟਾਂ = sqlite3_ਕਾਲਮ_ਬਾਈਟਾਂ(pStmt, i_ਖਾਨਾ);
      ਅਚਲ ਅੱਖਰ *zBlob = sqlite3_ਕਾਲਮ_blob(pStmt, i_ਖਾਨਾ);
      ਜੇ( !zBlob ) ਬਾਈਟਾਂ = 0;
      ਵਾਪਸ Tcl_NewByteArrayObj((u8*)zBlob, ਬਾਈਟਾਂ);
    }
    ਹਾਲਤ SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_ਕਾਲਮ_int64(pStmt, i_ਖਾਨਾ);
      ਜੇ( v>=-2147483647 && v<=2147483647 ){
        ਵਾਪਸ Tcl_NewIntObj((ਪੂਰਨ)v);
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ Tcl_NewWideIntObj(v);
      }
    }
    ਹਾਲਤ SQLITE_FLOAT: {
      ਵਾਪਸ Tcl_NewDoubleObj(sqlite3_ਕਾਲਮ_double(pStmt, i_ਖਾਨਾ));
    }
    ਹਾਲਤ SQLITE_NULL: {
      ਵਾਪਸ Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  ਵਾਪਸ Tcl_NewStringObj((ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, i_ਖਾਨਾ), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#ਜੇ TCL_MAJOR_ਸੰਸਕਰਣ>8 || !defined(TCL_MINOR_ਸੰਸਕਰਣ) \
                        || TCL_MINOR_ਸੰਸਕਰਣ>=6
# define SQLITE_TCL_NRE 1
ਸਥਿਰ ਪੂਰਨ DbUseNre(ਖਾਲੀ){
  ਪੂਰਨ major, minor;
  Tcl_Get_ਸੰਸਕਰਣ(&major, &minor, 0, 0);
  ਵਾਪਸ( (major==8 && minor>=6) || major>8 );
}
#ਨਹੀਂਤਾਂ
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (ਖਾਲੀ)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (ਖਾਲੀ)0
#ਜੇ_ਅੰਤ

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI Db_Eval_ਅਗਲਾ_Cmd(
  Client_ਅੰਕੜੇ ਅੰਕੜੇ[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  ਪੂਰਨ ਨਤੀਜਾ                           /* Result so far */
){
  ਪੂਰਨ rc = ਨਤੀਜਾ;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)ਅੰਕੜੇ[0];
  Tcl_Obj * ਅਚਲ pScript = (Tcl_Obj *)ਅੰਕੜੇ[1];
  Tcl_Obj * ਅਚਲ pVarName = p->pVarName;

  ਜਦੋਂ( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = db_Eval_ਕਦਮ(p)) ){
    ਪੂਰਨ i;
    ਪੂਰਨ n_ਖਾਨਾ;
    Tcl_Obj **ap_ਖਾਨਾ_Name;
    db_Eval_ਪੰਕਤੀ_Info(p, &n_ਖਾਨਾ, &ap_ਖਾਨਾ_Name);
    ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
      ਜੇ( pVarName==0 ){
        Tcl_Obj_ਸੈੱਟ_Var2(interp, ap_ਖਾਨਾ_Name[i], 0, db_Eval_ਕਾਲਮ_ਮੁੱਲ(p,i), 0);
      }ਨਹੀਂਤਾਂ ਜੇ( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_ਕਾਲਮ_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        ਜੇ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(ap_ਖਾਨਾ_Name[i]), 0);
        }ਨਹੀਂਤਾਂ{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ਨਿਰਾ, 0);
          ਜੇ( pDict ){
            ਜੇ( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            ਜੇ( Tcl_DictObjRemove(interp, pDict, ap_ਖਾਨਾ_Name[i])==TCL_OK ){
              Tcl_Obj_ਸੈੱਟ_Var2(interp, pVarName, ਨਿਰਾ, pDict, 0);
            }
            Tcl_Bounce_Ref_ਗਿਣਤੀ(pDict);
          }
        }
      }ਨਹੀਂਤਾਂ ਜੇ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_Obj_ਸੈੱਟ_Var2(interp, pVarName, ap_ਖਾਨਾ_Name[i],
                       db_Eval_ਕਾਲਮ_ਮੁੱਲ(p,i), 0);
      }ਨਹੀਂਤਾਂ{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ਨਿਰਾ, 0);
        ਜੇ( !pDict ){
          pDict = Tcl_NewDictObj();
        }ਨਹੀਂਤਾਂ ਜੇ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ਜੇ( Tcl_DictObjPut(interp, pDict, ap_ਖਾਨਾ_Name[i],
                           db_Eval_ਕਾਲਮ_ਮੁੱਲ(p,i))==TCL_OK ){
          Tcl_Obj_ਸੈੱਟ_Var2(interp, pVarName, ਨਿਰਾ, pDict, 0);
        }
        Tcl_Bounce_Ref_ਗਿਣਤੀ(pDict);
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
    ਜੇ( DbUseNre() ){
      Tcl_NRAddCallback(interp, Db_Eval_ਅਗਲਾ_Cmd, (ਖਾਲੀ*)p, (ਖਾਲੀ*)pScript, 0, 0);
      ਵਾਪਸ Tcl_NREvalObj(interp, pScript, 0);
    }ਨਹੀਂਤਾਂ{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_ਗਿਣਤੀ(pScript);
  dbEvalFinalize(p);
  Tcl_Free((ਅੱਖਰ *)p);

  ਜੇ( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_Reset_ਨਤੀਜਾ(interp);
    rc = TCL_OK;
  }
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਖਾਲੀ DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  ਜੇ( *ppHook ){
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, *ppHook);
    ਜੇ( pArg ){
      Tcl_Decr_Ref_ਗਿਣਤੀ(*ppHook);
      *ppHook = 0;
    }
  }
  ਜੇ( pArg ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !(*ppHook) );
    ਜੇ( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_ਗਿਣਤੀ(*ppHook);
    }
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
  sqlite3_preupdate_hook(db, (pDb->pPreUpdateHook?DbPreUpdateHandler:0), pDb);
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਪੂਰਨ dbQrf(SqliteDb *pDb, ਪੂਰਨ objc, Tcl_Obj *ਅਚਲ*objv){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_QRF_H
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  ਵਾਪਸ TCL_ERROR;
#ਨਹੀਂਤਾਂ
  ਅੱਖਰ *z_ਨਤੀਜਾ = 0;             /* Result to be returned */
  ਅਚਲ ਅੱਖਰ *zSql = 0;          /* SQL to run */
  ਪੂਰਨ i;                         /* Loop counter */
  ਪੂਰਨ rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_ਖੱਬਾ,
    QRF_ALIGN_ਵਿਚਕਾਰ, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_ਸੱਜਾ,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_ਸਿਖਰ,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, ਆਕਾਰ(qrf));
  qrf.i_ਸੰਸਕਰਣ = 2;
  qrf.pzOutput = &z_ਨਤੀਜਾ;
  ਲਈ(i=2; i<objc; i++){
    ਅਚਲ ਅੱਖਰ *zArg = Tcl_GetString(objv[i]);
    ਅਚਲ ਅੱਖਰ *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aBoolMap[] = { 0, 2, 1, 2, 1 };
    ਜੇ( zArg[0]!='-' ){
      ਜੇ( zSql ){
        Tcl_Append_ਨਤੀਜਾ(pDb->interp, "unknown argument: ", zArg, (ਅੱਖਰ*)0);
        rc = TCL_ERROR;
        ਜਾਓ format_failed;
      }
      zSql  = zArg;
    }ਨਹੀਂਤਾਂ ਜੇ( i==objc-1 ){
      Tcl_Append_ਨਤੀਜਾ(pDb->interp, "option has no argument: ", zArg,(ਅੱਖਰ*)0);
      rc = TCL_ERROR;
      ਜਾਓ format_failed;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-style")==0 ){
      ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_ਕਾਲਮ,
        QRF_STYLE_ਗਿਣਤੀ,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_ਪਾਓ,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_ਸਤਰ,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_ਸਾਰਣੀ
      };
      ਪੂਰਨ style;
      rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      ਜੇ( rc ) ਜਾਓ format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-esc")==0 ){
      ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_ਚਿੰਨ੍ਹ
      };
      ਪੂਰਨ esc;
      rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      ਜੇ( rc ) ਜਾਓ format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      ਸਥਿਰ ਅਚਲ ਅੱਖਰ *az_ਪਾਠ[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ a_ਪਾਠ_Map[] = {
        QRF_ਪਾਠ_Auto,      QRF_ਪਾਠ_Csv,       QRF_ਪਾਠ_Html,
        QRF_ਪਾਠ_Json,      QRF_ਪਾਠ_Plain,     QRF_ਪਾਠ_Relaxed,
        QRF_ਪਾਠ_Sql,       QRF_ਪਾਠ_Tcl
      };
      ਪੂਰਨ txt;
      ਪੂਰਨ k = zArg[2]=='e';
      rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pDb->interp, objv[i+1], &az_ਪਾਠ[k*3], zArg,
                               0, &txt);
      ਜੇ( rc ) ਜਾਓ format_failed;
      ਜੇ( k ){
        qrf.e_ਪਾਠ = a_ਪਾਠ_Map[txt];
      }ਨਹੀਂਤਾਂ ਜੇ( txt<=2 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( QRF_No==1 );
        ਪ੍ਰਤਿਜ੍ਞਾ( QRF_Yes==2 );
        ਪ੍ਰਤਿਜ੍ਞਾ( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_ਪਾਠ_Auto;
      }ਨਹੀਂਤਾਂ{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = a_ਪਾਠ_Map[txt-3];
      }
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-blob")==0 ){
      ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_ਪਾਠ,      QRF_BLOB_Sql,
        QRF_BLOB_ਮਾਪ
      };
      ਪੂਰਨ blob;
      rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      ਜੇ( rc ) ਜਾਓ format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-wordwrap")==0 ){
      ਪੂਰਨ v = 0;
      rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      ਜੇ( rc ) ਜਾਓ format_failed;
      qrf.b_ਸ਼ਬਦ_Wrap = aBoolMap[v];
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      ਪੂਰਨ v = 0;
      rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      ਜੇ( rc ) ਜਾਓ format_failed;
      ਜੇ( zArg[1]=='t' ){
        qrf.b_ਪਾਠ_Jsonb = aBoolMap[v];
      }ਨਹੀਂਤਾਂ ਜੇ( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }ਨਹੀਂਤਾਂ ਜੇ( zArg[1]=='r' ){
        qrf.b_ਪੰਕਤੀ_ਗਿਣਤੀ = aBoolMap[v];
      }ਨਹੀਂਤਾਂ{
        qrf.b_ਵੰਡੋ_ਕਾਲਮ = aBoolMap[v];
      }
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      ਪੂਰਨ ax = 0;
      rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      ਜੇ( rc ) ਜਾਓ format_failed;
      ਜੇ( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }ਨਹੀਂਤਾਂ{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      ਪੂਰਨ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ਜੇ( rc ) ਜਾਓ format_failed;
      ਜੇ( v<QRF_MIN_ਚੌੜਾਈ ){
        v = QRF_MIN_ਚੌੜਾਈ;
      }ਨਹੀਂਤਾਂ ਜੇ( v>QRF_MAX_ਚੌੜਾਈ ){
        v = QRF_MAX_ਚੌੜਾਈ;
      }
      ਜੇ( zArg[1]=='w' ){
        qrf.nWrap = v;
      }ਨਹੀਂਤਾਂ ਜੇ( zArg[1]=='s' ){
        qrf.n_Screen_ਚੌੜਾਈ = v;
      }ਨਹੀਂਤਾਂ ਜੇ( zArg[1]=='t' ){
        qrf.n_Title_ਹੱਦਬੰਦੀ = v;
      }ਨਹੀਂਤਾਂ{
        qrf.n_ਸਤਰ_ਹੱਦਬੰਦੀ = v;
      }
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-charlimit")==0 ){
      ਪੂਰਨ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ਜੇ( rc ) ਜਾਓ format_failed;
      ਜੇ( v<0 ) v = 0;
      qrf.n_Char_ਹੱਦਬੰਦੀ = v;
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-multiinsert")==0 ){
      ਪੂਰਨ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ਜੇ( rc ) ਜਾਓ format_failed;
      ਜੇ( v<0 ) v = 0;
      qrf.n_Multi_ਪਾਓ = v;
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-align")==0 ){
      Tcl_ਮਾਪ n = 0;
      ਪੂਰਨ jj;
      rc = Tcl_List_Obj_ਲੰਮਾਈ(pDb->interp, objv[i+1], &n);
      ਜੇ( rc ) ਜਾਓ format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*ਆਕਾਰ(qrf.aAlign[0]) );
      ਜੇ( qrf.aAlign==0 ){
        Tcl_Append_ਨਤੀਜਾ(pDb->interp, "out of memory", (ਅੱਖਰ*)0);
        rc = TCL_ERROR;
        ਜਾਓ format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*ਆਕਾਰ(qrf.aAlign[0]));
      qrf.nAlign = n;
      ਲਈ(jj=0; jj<n; jj++){
        ਪੂਰਨ x;
        Tcl_Obj *p_ਪਦ;
        rc = Tcl_List_Obj_ਸੂਚਕਾਂਕ(pDb->interp, objv[i+1], jj, &p_ਪਦ);
        ਜੇ( rc ) ਜਾਓ format_failed;
        rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(pDb->interp, p_ਪਦ, azAlign,
                          "column alignment (-align)", 0, &x);
        ਜੇ( rc ) ਜਾਓ format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-widths")==0 ){
      Tcl_ਮਾਪ n = 0;
      ਪੂਰਨ jj;
      rc = Tcl_List_Obj_ਲੰਮਾਈ(pDb->interp, objv[i+1], &n);
      ਜੇ( rc ) ਜਾਓ format_failed;
      sqlite3_free(qrf.a_ਚੌੜਾਈ);
      qrf.a_ਚੌੜਾਈ = sqlite3_malloc64( (n+1)*ਆਕਾਰ(qrf.a_ਚੌੜਾਈ[0]) );
      ਜੇ( qrf.a_ਚੌੜਾਈ==0 ){
        Tcl_Append_ਨਤੀਜਾ(pDb->interp, "out of memory", (ਅੱਖਰ*)0);
        rc = TCL_ERROR;
        ਜਾਓ format_failed;
      }
      memset(qrf.a_ਚੌੜਾਈ, 0, (n+1)*ਆਕਾਰ(qrf.a_ਚੌੜਾਈ[0]));
      qrf.n_ਚੌੜਾਈ = n;
      ਲਈ(jj=0; jj<n; jj++){
        Tcl_Obj *p_ਪਦ;
        ਪੂਰਨ v;
        rc = Tcl_List_Obj_ਸੂਚਕਾਂਕ(pDb->interp, objv[i+1], jj, &p_ਪਦ);
        ਜੇ( rc ) ਜਾਓ format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, p_ਪਦ, &v);
        ਜੇ( v<(-QRF_MAX_ਚੌੜਾਈ) ){
          v = -QRF_MAX_ਚੌੜਾਈ;
        }ਨਹੀਂਤਾਂ ਜੇ( v>QRF_MAX_ਚੌੜਾਈ ){
          v = QRF_MAX_ਚੌੜਾਈ;
        }
        qrf.a_ਚੌੜਾਈ[jj] = (ਛੋਟਾ ਪੂਰਨ)v;
      }
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-columnsep")==0 ){
      qrf.z_ਕਾਲਮ_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-rowsep")==0 ){
      qrf.z_ਪੰਕਤੀ_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-tablename")==0 ){
      qrf.z_ਸਾਰਣੀ_Name = Tcl_GetString(objv[i+1]);
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-ifmt")==0 ){
      ਅੱਖਰ *zFmt = Tcl_GetString(objv[i+1]);
      ਜੇ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_Append_ਨਤੀਜਾ(pDb->interp, "bad format for -ifmt", (ਅੱਖਰ*)0);
        rc = TCL_ERROR;
        ਜਾਓ format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-fpfmt")==0 ){
      ਅੱਖਰ *zFmt = Tcl_GetString(objv[i+1]);
      ਜੇ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_Append_ਨਤੀਜਾ(pDb->interp, "bad format for -fpfmt", (ਅੱਖਰ*)0);
        rc = TCL_ERROR;
        ਜਾਓ format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.i_ਸੰਸਕਰਣ = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }ਨਹੀਂਤਾਂ{
      Tcl_Append_ਨਤੀਜਾ(pDb->interp, "unknown option: ", zArg, (ਅੱਖਰ*)0);
      rc = TCL_ERROR;
      ਜਾਓ format_failed;
    }
  }
  ਜਦੋਂ( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    ਅੱਖਰ *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    ਜੇ( rc ) ਜਾਓ format_failed;
    ਜੇ( pStmt==0 ) ਜਾਰੀ;
    rc = sqlite3_format_query_ਨਤੀਜਾ(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    ਜੇ( rc ){
      Tcl_ਸੈੱਟ_ਨਤੀਜਾ(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      ਜਾਓ format_failed;
    }
  }
  Tcl_ਸੈੱਟ_ਨਤੀਜਾ(pDb->interp, z_ਨਤੀਜਾ, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.a_ਚੌੜਾਈ);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(z_ਨਤੀਜਾ);
  ਵਾਪਸ rc;

#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI DbObjCmd(
  ਖਾਲੀ *cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *ਅਚਲ*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  ਪੂਰਨ choice;
  ਪੂਰਨ rc = TCL_OK;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *DB_strs[] = {
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
  ਗਿਣਤੀ DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_ਲੱਦੋ_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_ਰੁਕਾਵਟ,            DB_ਆਖਰੀ_ਪਾਓ_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_ਤਰੱਕੀ,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_ਸਥਿਤੀ,                DB_TIMEOUT,              DB_ਕੁੱਲ_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_ਸੰਸਕਰਣ,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SUBCOMMAND ...");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਚੋਣ( (ਗਿਣਤੀ DB_enum)choice ){

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
  ਹਾਲਤ DB_AUTHORIZER: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
    Tcl_Append_ਨਤੀਜਾ(interp, "authorization not available in this build",
                     (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
#ਨਹੀਂਤਾਂ
    ਜੇ( objc>3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?CALLBACK?");
      ਵਾਪਸ TCL_ERROR;
    }ਨਹੀਂਤਾਂ ਜੇ( objc==2 ){
      ਜੇ( pDb->zAuth ){
        Tcl_Append_ਨਤੀਜਾ(interp, pDb->zAuth, (ਅੱਖਰ*)0);
      }
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ *zAuth;
      Tcl_ਮਾਪ len;
      ਜੇ( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      ਜੇ( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->zAuth, zAuth, len+1);
      }ਨਹੀਂਤਾਂ{
        pDb->zAuth = 0;
      }
      ਜੇ( pDb->zAuth ){
        ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*sqlite3_auth_cb)(
           ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,
           ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
        pDb->interp = interp;
        sqlite3_ਸੈੱਟ_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਸੈੱਟ_authorizer(pDb->db, 0, 0);
      }
    }
#ਜੇ_ਅੰਤ
    ਰੋਕੋ;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  ਹਾਲਤ DB_BACKUP: {
    ਅਚਲ ਅੱਖਰ *zDestFile;
    ਅਚਲ ਅੱਖਰ *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    ਜੇ( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }ਨਹੀਂਤਾਂ ਜੇ( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }ਨਹੀਂਤਾਂ{
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?DATABASE? FILENAME");
      ਵਾਪਸ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    ਜੇ( rc!=SQLITE_OK ){
      Tcl_Append_ਨਤੀਜਾ(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (ਅੱਖਰ*)0);
      sqlite3_close(pDest);
      ਵਾਪਸ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    ਜੇ( pBackup==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (ਅੱਖਰ*)0);
      sqlite3_close(pDest);
      ਵਾਪਸ TCL_ERROR;
    }
    ਜਦੋਂ(  (rc = sqlite3_backup_ਕਦਮ(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_ਅੰਤ(pBackup);
    ਜੇ( rc==SQLITE_ਪੂਰਾ ){
      rc = TCL_OK;
    }ਨਹੀਂਤਾਂ{
      Tcl_Append_ਨਤੀਜਾ(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (ਅੱਖਰ*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    ਰੋਕੋ;
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
  ਹਾਲਤ DB_BIND_FALLBACK: {
    ਜੇ( objc>3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?CALLBACK?");
      ਵਾਪਸ TCL_ERROR;
    }ਨਹੀਂਤਾਂ ਜੇ( objc==2 ){
      ਜੇ( pDb->zBindFallback ){
        Tcl_Append_ਨਤੀਜਾ(interp, pDb->zBindFallback, (ਅੱਖਰ*)0);
      }
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ *zCallback;
      Tcl_ਮਾਪ len;
      ਜੇ( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      ਜੇ( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->zBindFallback, zCallback, len+1);
      }ਨਹੀਂਤਾਂ{
        pDb->zBindFallback = 0;
      }
    }
    ਰੋਕੋ;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  ਹਾਲਤ DB_BUSY: {
    ਜੇ( objc>3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "CALLBACK");
      ਵਾਪਸ TCL_ERROR;
    }ਨਹੀਂਤਾਂ ਜੇ( objc==2 ){
      ਜੇ( pDb->zBusy ){
        Tcl_Append_ਨਤੀਜਾ(interp, pDb->zBusy, (ਅੱਖਰ*)0);
      }
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ *zBusy;
      Tcl_ਮਾਪ len;
      ਜੇ( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      ਜੇ( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->zBusy, zBusy, len+1);
      }ਨਹੀਂਤਾਂ{
        pDb->zBusy = 0;
      }
      ਜੇ( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }ਨਹੀਂਤਾਂ{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    ਰੋਕੋ;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  ਹਾਲਤ DB_CACHE: {
    ਅੱਖਰ *subCmd;
    ਪੂਰਨ n;

    ਜੇ( objc<=2 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "cache option ?arg?");
      ਵਾਪਸ TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    ਜੇ( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      ਜੇ( objc!=3 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "flush");
        ਵਾਪਸ TCL_ERROR;
      }ਨਹੀਂਤਾਂ{
        flushStmtCache( pDb );
      }
    }ਨਹੀਂਤਾਂ ਜੇ( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      ਜੇ( objc!=4 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "size n");
        ਵਾਪਸ TCL_ERROR;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_Append_ਨਤੀਜਾ( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (ਅੱਖਰ*)0);
          ਵਾਪਸ TCL_ERROR;
        }ਨਹੀਂਤਾਂ{
          ਜੇ( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }ਨਹੀਂਤਾਂ ਜੇ( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }ਨਹੀਂਤਾਂ{
      Tcl_Append_ਨਤੀਜਾ( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
    ਰੋਕੋ;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  ਹਾਲਤ DB_CHANGES: {
    Tcl_Obj *p_ਨਤੀਜਾ;
    ਜੇ( objc!=2 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
      ਵਾਪਸ TCL_ERROR;
    }
    p_ਨਤੀਜਾ = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
    Tcl_ਸੈੱਟ_Wide_Int_Obj(p_ਨਤੀਜਾ, sqlite3_changes64(pDb->db));
    ਰੋਕੋ;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  ਹਾਲਤ DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    ਰੋਕੋ;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  ਹਾਲਤ DB_COLLATE: {
    SqlCollate *pCollate;
    ਅੱਖਰ *zName;
    ਅੱਖਰ *zScript;
    Tcl_ਮਾਪ nScript;
    ਜੇ( objc!=4 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "NAME SCRIPT");
      ਵਾਪਸ TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( ਆਕਾਰ(*pCollate) + nScript + 1 );
    ਜੇ( pCollate==0 ) ਵਾਪਸ TCL_ERROR;
    pCollate->interp = interp;
    pCollate->p_ਅਗਲਾ = pDb->pCollate;
    pCollate->zScript = (ਅੱਖਰ*)&pCollate[1];
    pDb->pCollate = pCollate;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pCollate->zScript, zScript, nScript+1);
    ਜੇ( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      ਵਾਪਸ TCL_ERROR;
    }
    ਰੋਕੋ;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  ਹਾਲਤ DB_COLLATION_NEEDED: {
    ਜੇ( objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "SCRIPT");
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_ਗਿਣਤੀ(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_ਗਿਣਤੀ(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    ਰੋਕੋ;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  ਹਾਲਤ DB_COMMIT_HOOK: {
    ਜੇ( objc>3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?CALLBACK?");
      ਵਾਪਸ TCL_ERROR;
    }ਨਹੀਂਤਾਂ ਜੇ( objc==2 ){
      ਜੇ( pDb->zCommit ){
        Tcl_Append_ਨਤੀਜਾ(interp, pDb->zCommit, (ਅੱਖਰ*)0);
      }
    }ਨਹੀਂਤਾਂ{
      ਅਚਲ ਅੱਖਰ *zCommit;
      Tcl_ਮਾਪ len;
      ਜੇ( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      ਜੇ( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->zCommit, zCommit, len+1);
      }ਨਹੀਂਤਾਂ{
        pDb->zCommit = 0;
      }
      ਜੇ( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }ਨਹੀਂਤਾਂ{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    ਰੋਕੋ;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  ਹਾਲਤ DB_COMPLETE: {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPLETE
    Tcl_Obj *p_ਨਤੀਜਾ;
    ਪੂਰਨ isComplete;
    ਜੇ( objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "SQL");
      ਵਾਪਸ TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    p_ਨਤੀਜਾ = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
    Tcl_ਸੈੱਟ_Boolean_Obj(p_ਨਤੀਜਾ, isComplete);
#ਜੇ_ਅੰਤ
    ਰੋਕੋ;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  ਹਾਲਤ DB_CONFIG: {
    ਸਥਿਰ ਅਚਲ ਬਣਤਰ DbConfigChoices {
      ਅਚਲ ਅੱਖਰ *zName;
      ਪੂਰਨ op;
    } aDbConfig[] = {
        { "defensive",          SQLITE_DBCONFIG_DEFENSIVE             },
        { "dqs_ddl",            SQLITE_DBCONFIG_DQS_DDL               },
        { "dqs_dml",            SQLITE_DBCONFIG_DQS_DML               },
        { "enable_fkey",        SQLITE_DBCONFIG_ENABLE_FKEY           },
        { "enable_qpsg",        SQLITE_DBCONFIG_ENABLE_QPSG           },
        { "enable_trigger",     SQLITE_DBCONFIG_ENABLE_TRIGGER        },
        { "enable_view",        SQLITE_DBCONFIG_ENABLE_VIEW           },
        { "fts3_tokenizer",     SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER },
        { "legacy_alter_table", SQLITE_DBCONFIG_LEGACY_ALTER_ਸਾਰਣੀ    },
        { "legacy_file_format", SQLITE_DBCONFIG_LEGACY_FILE_FORMAT    },
        { "load_extension",     SQLITE_DBCONFIG_ENABLE_ਲੱਦੋ_EXTENSION },
        { "no_ckpt_on_close",   SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE      },
        { "reset_database",     SQLITE_DBCONFIG_RESET_DATABASE        },
        { "trigger_eqp",        SQLITE_DBCONFIG_TRIGGER_EQP           },
        { "trusted_schema",     SQLITE_DBCONFIG_TRUSTED_SCHEMA        },
        { "writable_schema",    SQLITE_DBCONFIG_WRITABLE_SCHEMA       },
    };
    Tcl_Obj *p_ਨਤੀਜਾ;
    ਪੂਰਨ ii;
    ਜੇ( objc>4 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      p_ਨਤੀਜਾ = Tcl_NewListObj(0,0);
      ਲਈ(ii=0; ii<ਆਕਾਰ(aDbConfig)/ਆਕਾਰ(aDbConfig[0]); ii++){
        ਪੂਰਨ v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, p_ਨਤੀਜਾ,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, p_ਨਤੀਜਾ,
           Tcl_NewIntObj(v));
      }
    }ਨਹੀਂਤਾਂ{
      ਅਚਲ ਅੱਖਰ *zOpt = Tcl_GetString(objv[2]);
      ਪੂਰਨ onoff = -1;
      ਪੂਰਨ v = 0;
      ਜੇ( zOpt[0]=='-' ) zOpt++;
      ਲਈ(ii=0; ii<ਆਕਾਰ(aDbConfig)/ਆਕਾਰ(aDbConfig[0]); ii++){
        ਜੇ( strcmp(aDbConfig[ii].zName, zOpt)==0 ) ਰੋਕੋ;
      }
      ਜੇ( ii>=ਆਕਾਰ(aDbConfig)/ਆਕਾਰ(aDbConfig[0]) ){
        Tcl_Append_ਨਤੀਜਾ(interp, "unknown config option: \"", zOpt,
                                "\"", (ਖਾਲੀ*)0);
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( objc==4 ){
        ਜੇ( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          ਵਾਪਸ TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      p_ਨਤੀਜਾ = Tcl_NewIntObj(v);
    }
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, p_ਨਤੀਜਾ);
    ਰੋਕੋ;
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
  ਹਾਲਤ DB_COPY: {
    ਅੱਖਰ *z_ਸਾਰਣੀ;               /* Insert data into this table */
    ਅੱਖਰ *zFile;                /* The file from which to extract data */
    ਅੱਖਰ *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    ਪੂਰਨ n_ਖਾਨਾ;                   /* Number of columns in the table */
    ਪੂਰਨ nByte;                  /* Number of bytes in an SQL string */
    ਪੂਰਨ i, j;                   /* Loop counters */
    ਪੂਰਨ nSep;                   /* Number of bytes in zSep[] */
    ਪੂਰਨ nNull;                  /* Number of bytes in zNull[] */
    ਅੱਖਰ *zSql;                 /* An SQL statement */
    ਅੱਖਰ *z_ਸਤਰ;                /* A single line of input from the file */
    ਅੱਖਰ **az_ਖਾਨਾ;               /* zLine[] broken up into columns */
    ਅਚਲ ਅੱਖਰ *zCommit;        /* How to commit changes */
    Tcl_Channel ਵਿੱਚ;             /* The input file */
    ਪੂਰਨ lineno = 0;             /* Line number of input file */
    ਅੱਖਰ z_ਸਤਰ_ਅੰਕ[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *p_ਨਤੀਜਾ;           /* interp result */

    ਅਚਲ ਅੱਖਰ *zSep;
    ਅਚਲ ਅੱਖਰ *zNull;
    ਜੇ( objc<5 || objc>7 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }ਨਹੀਂਤਾਂ{
      zSep = "\t";
    }
    ਜੇ( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }ਨਹੀਂਤਾਂ{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    z_ਸਾਰਣੀ = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    ਜੇ( nSep==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp,"Error: non-null separator required for copy",
                       (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_Append_ਨਤੀਜਾ(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", z_ਸਾਰਣੀ);
    ਜੇ( zSql==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp, "Error: no such table: ", z_ਸਾਰਣੀ, (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    ਜੇ( rc ){
      Tcl_Append_ਨਤੀਜਾ(interp, "Error: ", sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
      n_ਖਾਨਾ = 0;
    }ਨਹੀਂਤਾਂ{
      n_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਗਿਣਤੀ(pStmt);
    }
    sqlite3_finalize(pStmt);
    ਜੇ( n_ਖਾਨਾ==0 ) {
      ਵਾਪਸ TCL_ERROR;
    }
    zSql = ਸਮ੍ਰਿਤੀ_ਦਿਓ( nByte + 50 + n_ਖਾਨਾ*2 );
    ਜੇ( zSql==0 ) {
      Tcl_Append_ਨਤੀਜਾ(interp, "Error: can't malloc()", (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, z_ਸਾਰਣੀ);
    j = strlen30(zSql);
    ਲਈ(i=1; i<n_ਖਾਨਾ; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    ਮੁਕਤ(zSql);
    ਜੇ( rc ){
      Tcl_Append_ਨਤੀਜਾ(interp, "Error: ", sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
      sqlite3_finalize(pStmt);
      ਵਾਪਸ TCL_ERROR;
    }
    ਵਿੱਚ = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    ਜੇ( ਵਿੱਚ==0 ){
      sqlite3_finalize(pStmt);
      ਵਾਪਸ TCL_ERROR;
    }
    Tcl_ਸੈੱਟ_Channel_Option(ਨਿਰਾ, ਵਿੱਚ, "-translation", "auto");
    az_ਖਾਨਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ( ਆਕਾਰ(az_ਖਾਨਾ[0])*(n_ਖਾਨਾ+1) );
    ਜੇ( az_ਖਾਨਾ==0 ) {
      Tcl_Append_ਨਤੀਜਾ(interp, "Error: can't malloc()", (ਅੱਖਰ*)0);
      Tcl_Close(interp, ਵਿੱਚ);
      ਵਾਪਸ TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_ਗਿਣਤੀ(str);
    (ਖਾਲੀ)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    ਜਦੋਂ( Tcl_GetsObj(ਵਿੱਚ, str)>=0 ) {
      ਅੱਖਰ *z;
      Tcl_ਮਾਪ byteLen;
      lineno++;
      z_ਸਤਰ = (ਅੱਖਰ *)Tcl_GetByteArrayFromObj(str, &byteLen);
      az_ਖਾਨਾ[0] = z_ਸਤਰ;
      ਲਈ(i=0, z=z_ਸਤਰ; *z; z++){
        ਜੇ( *z==zSep[0] && ਲੜੀ_ਤੁਲਨਾ(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          ਜੇ( i<n_ਖਾਨਾ ){
            az_ਖਾਨਾ[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      ਜੇ( i+1!=n_ਖਾਨਾ ){
        ਅੱਖਰ *zErr;
        ਪੂਰਨ nErr = strlen30(zFile) + 200;
        zErr = ਸਮ੍ਰਿਤੀ_ਦਿਓ(nErr);
        ਜੇ( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, n_ਖਾਨਾ, i+1);
          Tcl_Append_ਨਤੀਜਾ(interp, zErr, (ਅੱਖਰ*)0);
          ਮੁਕਤ(zErr);
        }
        zCommit = "ROLLBACK";
        ਰੋਕੋ;
      }
      ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
        /* check for null data, if so, bind as null */
        ਜੇ( (nNull>0 && strcmp(az_ਖਾਨਾ[i], zNull)==0)
          || strlen30(az_ਖਾਨਾ[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }ਨਹੀਂਤਾਂ{
          sqlite3_bind_ਪਾਠ(pStmt, i+1, az_ਖਾਨਾ[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_ਕਦਮ(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_ਸੈੱਟ_Obj_ਲੰਮਾਈ(str, 0);
      ਜੇ( rc!=SQLITE_OK ){
        Tcl_Append_ਨਤੀਜਾ(interp,"Error: ", sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
        zCommit = "ROLLBACK";
        ਰੋਕੋ;
      }
    }
    Tcl_Decr_Ref_ਗਿਣਤੀ(str);
    ਮੁਕਤ(az_ਖਾਨਾ);
    Tcl_Close(interp, ਵਿੱਚ);
    sqlite3_finalize(pStmt);
    (ਖਾਲੀ)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    ਜੇ( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      p_ਨਤੀਜਾ = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
      ਜੇ( Tcl_IsShared(p_ਨਤੀਜਾ) ){
        p_ਨਤੀਜਾ = Tcl_DuplicateObj(p_ਨਤੀਜਾ);
      }
      Tcl_ਸੈੱਟ_Int_Obj(p_ਨਤੀਜਾ, lineno);
      rc = TCL_OK;
    }ਨਹੀਂਤਾਂ{
      /* failure, append lineno where failed */
      sqlite3_snprintf(ਆਕਾਰ(z_ਸਤਰ_ਅੰਕ), z_ਸਤਰ_ਅੰਕ,"%d",lineno);
      Tcl_Append_ਨਤੀਜਾ(interp,", failed while processing line: ",z_ਸਤਰ_ਅੰਕ,
                       (ਅੱਖਰ*)0);
      rc = TCL_ERROR;
    }
    ਰੋਕੋ;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  ਹਾਲਤ DB_DESERIALIZE: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
    Tcl_Append_ਨਤੀਜਾ(interp, "MEMDB not available in this build",
                     (ਅੱਖਰ*)0);
    rc = TCL_ERROR;
#ਨਹੀਂਤਾਂ
    ਅਚਲ ਅੱਖਰ *zSchema = 0;
    Tcl_Obj *p_ਮੁੱਲ = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pBA;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p_ਅੰਕੜੇ;
    Tcl_ਮਾਪ len;
    ਪੂਰਨ xrc;
    sqlite3_int64 mx_ਮਾਪ = 0;
    ਪੂਰਨ i;
    ਪੂਰਨ isReadonly = 0;


    ਜੇ( objc<3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      ਰੋਕੋ;
    }
    ਲਈ(i=2; i<objc-1; i++){
      ਅਚਲ ਅੱਖਰ *z = Tcl_GetString(objv[i]);
      ਜੇ( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        ਜੇ( rc ) ਜਾਓ deserialize_error;
        mx_ਮਾਪ = x;
        ਜਾਰੀ;
      }
      ਜੇ( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        ਜੇ( rc ) ਜਾਓ deserialize_error;
        ਜਾਰੀ;
      }
      ਜੇ( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        ਜਾਰੀ;
      }
      Tcl_Append_ਨਤੀਜਾ(interp, "unknown option: ", z, (ਅੱਖਰ*)0);
      rc = TCL_ERROR;
      ਜਾਓ deserialize_error;
    }
    p_ਮੁੱਲ = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_ਮੁੱਲ, &len);
    p_ਅੰਕੜੇ = sqlite3_malloc64( len );
    ਜੇ( p_ਅੰਕੜੇ==0 && len>0 ){
      Tcl_Append_ਨਤੀਜਾ(interp, "out of memory", (ਅੱਖਰ*)0);
      rc = TCL_ERROR;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ flags;
      ਜੇ( len>0 ) ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਅੰਕੜੇ, pBA, len);
      ਜੇ( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }ਨਹੀਂਤਾਂ{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, p_ਅੰਕੜੇ, len, len, flags);
      ਜੇ( xrc ){
        Tcl_Append_ਨਤੀਜਾ(interp, "unable to set MEMDB content", (ਅੱਖਰ*)0);
        rc = TCL_ERROR;
      }
      ਜੇ( mx_ਮਾਪ>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_ਮਾਪ_ਹੱਦਬੰਦੀ,&mx_ਮਾਪ);
      }
    }
deserialize_error:
#ਜੇ_ਅੰਤ
    ਰੋਕੋ; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  ਹਾਲਤ DB_ENABLE_ਲੱਦੋ_EXTENSION: {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
    ਪੂਰਨ onoff;
    ਜੇ( objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "BOOLEAN");
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      ਵਾਪਸ TCL_ERROR;
    }
    sqlite3_enable_ਲੱਦੋ_extension(pDb->db, onoff);
    ਰੋਕੋ;
#ਨਹੀਂਤਾਂ
    Tcl_Append_ਨਤੀਜਾ(interp, "extension loading is turned off at compile-time",
                     (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
#ਜੇ_ਅੰਤ
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  ਹਾਲਤ DB_ERRORCODE: {
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    ਰੋਕੋ;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  ਹਾਲਤ DB_ERROROFFSET: {
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(sqlite3_error_ਵਿੱਥ(pDb->db)));
    ਰੋਕੋ;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  ਹਾਲਤ DB_EXISTS:
  ਹਾਲਤ DB_ONECOLUMN: {
    Tcl_Obj *p_ਨਤੀਜਾ = 0;
    DbEvalContext sEval;
    ਜੇ( objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "SQL");
      ਵਾਪਸ TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = db_Eval_ਕਦਮ(&sEval);
    ਜੇ( choice==DB_ONECOLUMN ){
      ਜੇ( rc==TCL_OK ){
        p_ਨਤੀਜਾ = db_Eval_ਕਾਲਮ_ਮੁੱਲ(&sEval, 0);
      }ਨਹੀਂਤਾਂ ਜੇ( rc==TCL_BREAK ){
        Tcl_Reset_ਨਤੀਜਾ(interp);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( rc==TCL_BREAK || rc==TCL_OK ){
      p_ਨਤੀਜਾ = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    ਜੇ( p_ਨਤੀਜਾ ) Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, p_ਨਤੀਜਾ);

    ਜੇ( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    ਰੋਕੋ;
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
  ਹਾਲਤ DB_EVAL: {
    ਪੂਰਨ evalFlags = 0;
    ਅਚਲ ਅੱਖਰ *zOpt;
    ਜਦੋਂ( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      ਜੇ( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }ਨਹੀਂਤਾਂ{
        Tcl_Append_ਨਤੀਜਾ(interp, "unknown option: \"", zOpt, "\"", (ਖਾਲੀ*)0);
        ਵਾਪਸ TCL_ERROR;
      }
      objc--;
      objv++;
    }
    ਜੇ( objc<3 || objc>5 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      ਵਾਪਸ TCL_ERROR;
    }

    ਜੇ( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_ਗਿਣਤੀ(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      ਜਦੋਂ( TCL_OK==(rc = db_Eval_ਕਦਮ(&sEval)) ){
        ਪੂਰਨ i;
        ਪੂਰਨ n_ਖਾਨਾ;
        db_Eval_ਪੰਕਤੀ_Info(&sEval, &n_ਖਾਨਾ, 0);
        ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_ਕਾਲਮ_ਮੁੱਲ(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      ਜੇ( rc==TCL_BREAK ){
        Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_ਗਿਣਤੀ(pRet);
    }ਨਹੀਂਤਾਂ{
      Client_ਅੰਕੜੇ cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      ਜੇ( objc>=5 && *(ਅੱਖਰ *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);

      p = (DbEvalContext *)Tcl_Alloc(ਆਕਾਰ(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (ਖਾਲੀ *)p;
      cd2[1] = (ਖਾਲੀ *)pScript;
      rc = Db_Eval_ਅਗਲਾ_Cmd(cd2, interp, TCL_OK);
    }
    ਰੋਕੋ;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  ਹਾਲਤ DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    ਰੋਕੋ;
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
  ਹਾਲਤ DB_FUNCTION: {
    ਪੂਰਨ flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    ਅੱਖਰ *zName;
    ਪੂਰਨ nArg = -1;
    ਪੂਰਨ i;
    ਪੂਰਨ eType = SQLITE_NULL;
    ਜੇ( objc<4 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      ਵਾਪਸ TCL_ERROR;
    }
    ਲਈ(i=3; i<(objc-1); i++){
      ਅਚਲ ਅੱਖਰ *z = Tcl_GetString(objv[i]);
      ਪੂਰਨ n = strlen30(z);
      ਜੇ( n>1 && ਲੜੀ_ਤੁਲਨਾ(z, "-argcount",n)==0 ){
        ਜੇ( i==(objc-2) ){
          Tcl_Append_ਨਤੀਜਾ(interp, "option requires an argument: ", z,(ਅੱਖਰ*)0);
          ਵਾਪਸ TCL_ERROR;
        }
        ਜੇ( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) ਵਾਪਸ TCL_ERROR;
        ਜੇ( nArg<0 ){
          Tcl_Append_ਨਤੀਜਾ(interp, "number of arguments must be non-negative",
                           (ਅੱਖਰ*)0);
          ਵਾਪਸ TCL_ERROR;
        }
        i++;
      }ਨਹੀਂਤਾਂ
      ਜੇ( n>1 && ਲੜੀ_ਤੁਲਨਾ(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }ਨਹੀਂਤਾਂ
      ਜੇ( n>1 && ਲੜੀ_ਤੁਲਨਾ(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }ਨਹੀਂਤਾਂ
      ਜੇ( n>1 && ਲੜੀ_ਤੁਲਨਾ(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }ਨਹੀਂਤਾਂ
      ਜੇ( n>1 && ਲੜੀ_ਤੁਲਨਾ(z, "-returntype", n)==0 ){
        ਅਚਲ ਅੱਖਰ *azType[] = {"integer", "real", "text", "blob", "any", 0};
        ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_ਪਾਠ==3 );
        ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        ਜੇ( i==(objc-2) ){
          Tcl_Append_ਨਤੀਜਾ(interp, "option requires an argument: ", z,(ਅੱਖਰ*)0);
          ਵਾਪਸ TCL_ERROR;
        }
        i++;
        ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[i], azType, "type", 0, &eType) ){
          ਵਾਪਸ TCL_ERROR;
        }
        eType++;
      }ਨਹੀਂਤਾਂ{
        Tcl_Append_ਨਤੀਜਾ(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (ਅੱਖਰ*)0
        );
        ਵਾਪਸ TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = ਲੱਭੋ_Sql_Func(pDb, zName);
    ਜੇ( pFunc==0 ) ਵਾਪਸ TCL_ERROR;
    ਜੇ( pFunc->pScript ){
      Tcl_Decr_Ref_ਗਿਣਤੀ(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_ਗਿਣਤੀ(pScript);
    pFunc->useEvalObjv = ਸੁਰੱਖਿਅਤ_ਮੰਜ਼ਿਲ_Use_Eval_Objv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    ਜੇ( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    ਰੋਕੋ;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  ਹਾਲਤ DB_INCRBLOB: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
    Tcl_Append_ਨਤੀਜਾ(interp, "incrblob not available in this build", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
#ਨਹੀਂਤਾਂ
    ਪੂਰਨ isReadonly = 0;
    ਅਚਲ ਅੱਖਰ *zDb = "main";
    ਅਚਲ ਅੱਖਰ *z_ਸਾਰਣੀ;
    ਅਚਲ ਅੱਖਰ *z_ਕਾਲਮ;
    Tcl_WideInt i_ਪੰਕਤੀ;

    /* Check for the -readonly option */
    ਜੇ( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    ਜੇ( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      ਵਾਪਸ TCL_ERROR;
    }

    ਜੇ( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    z_ਸਾਰਣੀ = Tcl_GetString(objv[objc-3]);
    z_ਕਾਲਮ = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &i_ਪੰਕਤੀ);

    ਜੇ( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, z_ਸਾਰਣੀ, z_ਕਾਲਮ, (sqlite3_int64)i_ਪੰਕਤੀ, isReadonly
      );
    }
#ਜੇ_ਅੰਤ
    ਰੋਕੋ;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  ਹਾਲਤ DB_ਰੁਕਾਵਟ: {
    sqlite3_ਰੁਕਾਵਟ(pDb->db);
    ਰੋਕੋ;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  ਹਾਲਤ DB_NULLVALUE: {
    ਜੇ( objc!=2 && objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "NULLVALUE");
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( objc==3 ){
      Tcl_ਮਾਪ len;
      ਅੱਖਰ *zNull = Tcl_GetStringFromObj(objv[2], &len);
      ਜੇ( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      ਜੇ( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }ਨਹੀਂਤਾਂ{
        pDb->zNull = 0;
      }
    }
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(pDb->zNull, -1));
    ਰੋਕੋ;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  ਹਾਲਤ DB_ਆਖਰੀ_ਪਾਓ_ROWID: {
    Tcl_Obj *p_ਨਤੀਜਾ;
    Tcl_WideInt rowid;
    ਜੇ( objc!=2 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
      ਵਾਪਸ TCL_ERROR;
    }
    rowid = sqlite3_ਆਖਰੀ_ਪਾਓ_rowid(pDb->db);
    p_ਨਤੀਜਾ = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
    Tcl_ਸੈੱਟ_Wide_Int_Obj(p_ਨਤੀਜਾ, rowid);
    ਰੋਕੋ;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  ਹਾਲਤ DB_ਤਰੱਕੀ: {
    ਜੇ( objc==2 ){
      ਜੇ( pDb->z_ਤਰੱਕੀ ){
        Tcl_Append_ਨਤੀਜਾ(interp, pDb->z_ਤਰੱਕੀ, (ਅੱਖਰ*)0);
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
      sqlite3_ਤਰੱਕੀ_handler(pDb->db, 0, 0, 0);
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ ਜੇ( objc==4 ){
      ਅੱਖਰ *z_ਤਰੱਕੀ;
      Tcl_ਮਾਪ len;
      ਪੂਰਨ N;
      ਜੇ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        ਵਾਪਸ TCL_ERROR;
      };
      ਜੇ( pDb->z_ਤਰੱਕੀ ){
        Tcl_Free(pDb->z_ਤਰੱਕੀ);
      }
      z_ਤਰੱਕੀ = Tcl_GetStringFromObj(objv[3], &len);
      ਜੇ( z_ਤਰੱਕੀ && len>0 ){
        pDb->z_ਤਰੱਕੀ = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->z_ਤਰੱਕੀ, z_ਤਰੱਕੀ, len+1);
      }ਨਹੀਂਤਾਂ{
        pDb->z_ਤਰੱਕੀ = 0;
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
      ਜੇ( pDb->z_ਤਰੱਕੀ ){
        pDb->interp = interp;
        sqlite3_ਤਰੱਕੀ_handler(pDb->db, N, Db_ਤਰੱਕੀ_Handler, pDb);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਤਰੱਕੀ_handler(pDb->db, 0, 0, 0);
      }
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ{
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "N CALLBACK");
      ਵਾਪਸ TCL_ERROR;
    }
    ਰੋਕੋ;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  ਹਾਲਤ DB_PROFILE: {
    ਜੇ( objc>3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?CALLBACK?");
      ਵਾਪਸ TCL_ERROR;
    }ਨਹੀਂਤਾਂ ਜੇ( objc==2 ){
      ਜੇ( pDb->zProfile ){
        Tcl_Append_ਨਤੀਜਾ(interp, pDb->zProfile, (ਅੱਖਰ*)0);
      }
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ *zProfile;
      Tcl_ਮਾਪ len;
      ਜੇ( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      ਜੇ( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->zProfile, zProfile, len+1);
      }ਨਹੀਂਤਾਂ{
        pDb->zProfile = 0;
      }
#ਜੇ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ਬਿੰਦੂ) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ਜੇ( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }ਨਹੀਂਤਾਂ{
        sqlite3_profile(pDb->db, 0, 0);
      }
#ਜੇ_ਅੰਤ
    }
    ਰੋਕੋ;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  ਹਾਲਤ DB_REKEY: {
    ਜੇ( objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "KEY");
      ਵਾਪਸ TCL_ERROR;
    }
    ਰੋਕੋ;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  ਹਾਲਤ DB_RESTORE: {
    ਅਚਲ ਅੱਖਰ *zSrcFile;
    ਅਚਲ ਅੱਖਰ *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    ਪੂਰਨ nTimeout = 0;

    ਜੇ( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }ਨਹੀਂਤਾਂ ਜੇ( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }ਨਹੀਂਤਾਂ{
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?DATABASE? FILENAME");
      ਵਾਪਸ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    ਜੇ( rc!=SQLITE_OK ){
      Tcl_Append_ਨਤੀਜਾ(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (ਅੱਖਰ*)0);
      sqlite3_close(pSrc);
      ਵਾਪਸ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    ਜੇ( pBackup==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
      sqlite3_close(pSrc);
      ਵਾਪਸ TCL_ERROR;
    }
    ਜਦੋਂ( (rc = sqlite3_backup_ਕਦਮ(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      ਜੇ( rc==SQLITE_BUSY ){
        ਜੇ( nTimeout++ >= 3 ) ਰੋਕੋ;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_ਅੰਤ(pBackup);
    ਜੇ( rc==SQLITE_ਪੂਰਾ ){
      rc = TCL_OK;
    }ਨਹੀਂਤਾਂ ਜੇ( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_Append_ਨਤੀਜਾ(interp, "restore failed: source database busy",
                       (ਅੱਖਰ*)0);
      rc = TCL_ERROR;
    }ਨਹੀਂਤਾਂ{
      Tcl_Append_ਨਤੀਜਾ(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    ਰੋਕੋ;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  ਹਾਲਤ DB_SERIALIZE: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
    Tcl_Append_ਨਤੀਜਾ(interp, "MEMDB not available in this build",
                     (ਅੱਖਰ*)0);
    rc = TCL_ERROR;
#ਨਹੀਂਤਾਂ
    ਅਚਲ ਅੱਖਰ *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p_ਅੰਕੜੇ;
    ਜੇ( objc!=2 && objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ needFree;
      p_ਅੰਕੜੇ = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      ਜੇ( p_ਅੰਕੜੇ ){
        needFree = 0;
      }ਨਹੀਂਤਾਂ{
        p_ਅੰਕੜੇ = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewByteArrayObj(p_ਅੰਕੜੇ,sz));
      ਜੇ( needFree ) sqlite3_free(p_ਅੰਕੜੇ);
    }
#ਜੇ_ਅੰਤ
    ਰੋਕੋ;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  ਹਾਲਤ DB_ਸਥਿਤੀ: {
    ਪੂਰਨ v;
    ਅਚਲ ਅੱਖਰ *zOp;
    ਜੇ( objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "(step|sort|autoindex)");
      ਵਾਪਸ TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    ਜੇ( strcmp(zOp, "step")==0 ){
      v = pDb->n_ਕਦਮ;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zOp, "sort")==0 ){
      v = pDb->n_ਛਾਂਟੋ;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zOp, "autoindex")==0 ){
      v = pDb->n_ਸੂਚਕਾਂਕ;
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }ਨਹੀਂਤਾਂ{
      Tcl_Append_ਨਤੀਜਾ(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(v));
    ਰੋਕੋ;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  ਹਾਲਤ DB_TIMEOUT: {
    ਪੂਰਨ ms;
    ਜੇ( objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "MILLISECONDS");
      ਵਾਪਸ TCL_ERROR;
    }
    ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &ms) ) ਵਾਪਸ TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    ਰੋਕੋ;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  ਹਾਲਤ DB_ਕੁੱਲ_CHANGES: {
    Tcl_Obj *p_ਨਤੀਜਾ;
    ਜੇ( objc!=2 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
      ਵਾਪਸ TCL_ERROR;
    }
    p_ਨਤੀਜਾ = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
    Tcl_ਸੈੱਟ_Wide_Int_Obj(p_ਨਤੀਜਾ, sqlite3_ਕੁੱਲ_changes64(pDb->db));
    ਰੋਕੋ;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  ਹਾਲਤ DB_TRACE: {
    ਜੇ( objc>3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?CALLBACK?");
      ਵਾਪਸ TCL_ERROR;
    }ਨਹੀਂਤਾਂ ਜੇ( objc==2 ){
      ਜੇ( pDb->zTrace ){
        Tcl_Append_ਨਤੀਜਾ(interp, pDb->zTrace, (ਅੱਖਰ*)0);
      }
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ *zTrace;
      Tcl_ਮਾਪ len;
      ਜੇ( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      ਜੇ( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->zTrace, zTrace, len+1);
      }ਨਹੀਂਤਾਂ{
        pDb->zTrace = 0;
      }
#ਜੇ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ਬਿੰਦੂ) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ਜੇ( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }ਨਹੀਂਤਾਂ{
        sqlite3_trace(pDb->db, 0, 0);
      }
#ਜੇ_ਅੰਤ
    }
    ਰੋਕੋ;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  ਹਾਲਤ DB_TRACE_V2: {
    ਜੇ( objc>4 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?CALLBACK? ?MASK?");
      ਵਾਪਸ TCL_ERROR;
    }ਨਹੀਂਤਾਂ ਜੇ( objc==2 ){
      ਜੇ( pDb->zTraceV2 ){
        Tcl_Append_ਨਤੀਜਾ(interp, pDb->zTraceV2, (ਅੱਖਰ*)0);
      }
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ *zTraceV2;
      Tcl_ਮਾਪ len;
      Tcl_WideInt w_ਨਕਾਬ = 0;
      ਜੇ( objc==4 ){
        ਸਥਿਰ ਅਚਲ ਅੱਖਰ *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        ਗਿਣਤੀ TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ਪੰਕਤੀ, TTYPE_CLOSE
        };
        Tcl_ਮਾਪ i;
        ਜੇ( TCL_OK!=Tcl_List_Obj_ਲੰਮਾਈ(interp, objv[3], &len) ){
          ਵਾਪਸ TCL_ERROR;
        }
        ਲਈ(i=0; i<len; i++){
          Tcl_Obj *pObj;
          ਪੂਰਨ ttype;
          ਜੇ( TCL_OK!=Tcl_List_Obj_ਸੂਚਕਾਂਕ(interp, objv[3], i, &pObj) ){
            ਵਾਪਸ TCL_ERROR;
          }
          ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_Get_Obj_ਨਤੀਜਾ(interp));
            Tcl_Incr_Ref_ਗਿਣਤੀ(pError);
            ਜੇ( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_ਗਿਣਤੀ(pError);
              w_ਨਕਾਬ |= wType;
            }ਨਹੀਂਤਾਂ{
              Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pError);
              Tcl_Decr_Ref_ਗਿਣਤੀ(pError);
              ਵਾਪਸ TCL_ERROR;
            }
          }ਨਹੀਂਤਾਂ{
            ਚੋਣ( (ਗਿਣਤੀ TTYPE_enum)ttype ){
              ਹਾਲਤ TTYPE_STMT:    w_ਨਕਾਬ |= SQLITE_TRACE_STMT;    ਰੋਕੋ;
              ਹਾਲਤ TTYPE_PROFILE: w_ਨਕਾਬ |= SQLITE_TRACE_PROFILE; ਰੋਕੋ;
              ਹਾਲਤ TTYPE_ਪੰਕਤੀ:     w_ਨਕਾਬ |= SQLITE_TRACE_ਪੰਕਤੀ;     ਰੋਕੋ;
              ਹਾਲਤ TTYPE_CLOSE:   w_ਨਕਾਬ |= SQLITE_TRACE_CLOSE;   ਰੋਕੋ;
            }
          }
        }
      }ਨਹੀਂਤਾਂ{
        w_ਨਕਾਬ = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      ਜੇ( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      ਜੇ( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDb->zTraceV2, zTraceV2, len+1);
      }ਨਹੀਂਤਾਂ{
        pDb->zTraceV2 = 0;
      }
#ਜੇ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ਬਿੰਦੂ)
      ਜੇ( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (ਬਿਨਾਨਿਸ਼ਾਨੀ)w_ਨਕਾਬ, DbTraceV2Handler, pDb);
      }ਨਹੀਂਤਾਂ{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#ਜੇ_ਅੰਤ
    }
    ਰੋਕੋ;
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
  ਹਾਲਤ DB_TRANSACTION: {
    Tcl_Obj *pScript;
    ਅਚਲ ਅੱਖਰ *zBegin = "SAVEPOINT _tcl_transaction";
    ਜੇ( objc!=3 && objc!=4 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "[TYPE] SCRIPT");
      ਵਾਪਸ TCL_ERROR;
    }

    ਜੇ( pDb->nTransaction==0 && objc==4 ){
      ਸਥਿਰ ਅਚਲ ਅੱਖਰ *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      ਗਿਣਤੀ TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_ਤੁਰੰਤ
      };
      ਪੂਰਨ ttype;
      ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        ਵਾਪਸ TCL_ERROR;
      }
      ਚੋਣ( (ਗਿਣਤੀ TTYPE_enum)ttype ){
        ਹਾਲਤ TTYPE_DEFERRED:    /* no-op */;                 ਰੋਕੋ;
        ਹਾਲਤ TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  ਰੋਕੋ;
        ਹਾਲਤ TTYPE_ਤੁਰੰਤ:   zBegin = "BEGIN IMMEDIATE";  ਰੋਕੋ;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    ਜੇ( rc!=SQLITE_OK ){
      Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    ਜੋੜੋ_Database_Ref(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    ਜੇ( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (ਖਾਲੀ)Tcl_NREvalObj(interp, pScript, 0);
    }ਨਹੀਂਤਾਂ{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    ਰੋਕੋ;
  }

  /*
  **    $db unlock_notify ?script?
  */
  ਹਾਲਤ DB_UNLOCK_NOTIFY: {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_Append_ਨਤੀਜਾ(interp, "unlock_notify not available in this build",
                     (ਅੱਖਰ*)0);
    rc = TCL_ERROR;
#ਨਹੀਂਤਾਂ
    ਜੇ( objc!=2 && objc!=3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }ਨਹੀਂਤਾਂ{
      ਖਾਲੀ (*xNotify)(ਖਾਲੀ **, ਪੂਰਨ) = 0;
      ਖਾਲੀ *pNotifyArg = 0;

      ਜੇ( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_ਗਿਣਤੀ(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      ਜੇ( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (ਖਾਲੀ *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_ਗਿਣਤੀ(pDb->pUnlockNotify);
      }

      ਜੇ( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
        rc = TCL_ERROR;
      }
    }
#ਜੇ_ਅੰਤ
    ਰੋਕੋ;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  ਹਾਲਤ DB_PREUPDATE: {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_Append_ਨਤੀਜਾ(interp, "preupdate_hook was omitted at compile-time", 
                     (ਅੱਖਰ*)0);
    rc = TCL_ERROR;
#ਨਹੀਂਤਾਂ
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    ਗਿਣਤੀ DbPreupdateSubCmd {
      PRE_ਗਿਣਤੀ, PRE_ਡੂੰਘਾਈ, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    ਪੂਰਨ iSub;

    ਜੇ( objc<3 ){
      Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      ਵਾਪਸ TCL_ERROR;
    }

    ਚੋਣ( (ਗਿਣਤੀ DbPreupdateSubCmd)iSub ){
      ਹਾਲਤ PRE_ਗਿਣਤੀ: {
        ਪੂਰਨ n_ਖਾਨਾ = sqlite3_preupdate_ਗਿਣਤੀ(pDb->db);
        Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(n_ਖਾਨਾ));
        ਰੋਕੋ;
      }

      ਹਾਲਤ PRE_HOOK: {
        ਜੇ( objc>4 ){
          Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "hook ?SCRIPT?");
          ਵਾਪਸ TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        ਰੋਕੋ;
      }

      ਹਾਲਤ PRE_ਡੂੰਘਾਈ: {
        Tcl_Obj *pRet;
        ਜੇ( objc!=3 ){
          Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 3, objv, "");
          ਵਾਪਸ TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_ਡੂੰਘਾਈ(pDb->db));
        Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);
        ਰੋਕੋ;
      }

      ਹਾਲਤ PRE_NEW:
      ਹਾਲਤ PRE_OLD: {
        ਪੂਰਨ iIdx;
        sqlite3_ਮੁੱਲ *p_ਮੁੱਲ;
        ਜੇ( objc!=4 ){
          Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 3, objv, "INDEX");
          ਵਾਪਸ TCL_ERROR;
        }
        ਜੇ( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          ਵਾਪਸ TCL_ERROR;
        }

        ਜੇ( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_ਮੁੱਲ);
        }ਨਹੀਂਤਾਂ{
          ਪ੍ਰਤਿਜ੍ਞਾ( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_ਮੁੱਲ);
        }

        ਜੇ( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(p_ਮੁੱਲ), -1);
          Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pObj);
        }ਨਹੀਂਤਾਂ{
          Tcl_Append_ਨਤੀਜਾ(interp, sqlite3_errmsg(pDb->db), (ਅੱਖਰ*)0);
          ਵਾਪਸ TCL_ERROR;
        }
      }
    }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */
    ਰੋਕੋ;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  ਹਾਲਤ DB_WAL_HOOK:
  ਹਾਲਤ DB_UPDATE_HOOK:
  ਹਾਲਤ DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    ਜੇ( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    ਜੇ( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    ਜੇ( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    ਜੇ( objc>3 ){
       Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?SCRIPT?");
       ਵਾਪਸ TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    ਰੋਕੋ;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  ਹਾਲਤ DB_ਸੰਸਕਰਣ: {
    ਪੂਰਨ i;
    ਲਈ(i=2; i<objc; i++){
      ਅਚਲ ਅੱਖਰ *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      ਜੇ( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          ਵਾਪਸ TCL_ERROR;
        }
      }ਨਹੀਂਤਾਂ

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      ਜੇ( strcmp(zArg, "-last-stmt-ptr")==0 ){
        ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(ਖਾਲੀ*);
        ਖਾਲੀ *ptr = (ਖਾਲੀ*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ*)sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(ptr), TCL_VOLATILE);
      }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_TEST */
      {
        Tcl_Append_ਨਤੀਜਾ(interp, "unknown argument: ", zArg, (ਅੱਖਰ*)0);
        ਵਾਪਸ TCL_ERROR;
      }
    }
    ਜੇ( i==2 ){   
      Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3_libversion(), TCL_STATIC);
    }
    ਰੋਕੋ;
  }


  } /* End of the SWITCH statement */
  ਵਾਪਸ rc;
}

#ਜੇ SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI DbObjCmdAdaptor(
  ਖਾਲੀ *cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *ਅਚਲ*objv
){
  ਵਾਪਸ Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#ਜੇ_ਅੰਤ /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
ਸਥਿਰ ਪੂਰਨ sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *ਅਚਲ*objv
){
  Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  ਵਾਪਸ TCL_ERROR;
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI DbMain(
  ਖਾਲੀ *cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *ਅਚਲ*objv
){
  SqliteDb *p;
  ਅਚਲ ਅੱਖਰ *zArg;
  ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ;
  ਪੂਰਨ i;
  ਅਚਲ ਅੱਖਰ *zFile = 0;
  ਅਚਲ ਅੱਖਰ *zVfs = 0;
  ਪੂਰਨ flags;
  ਪੂਰਨ bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  ਪੂਰਨ rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#ਨਹੀਂਤਾਂ
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#ਜੇ_ਅੰਤ

  ਜੇ( objc==1 ) ਵਾਪਸ sqliteCmdUsage(interp, objv);
  ਜੇ( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    ਜੇ( strcmp(zArg,"-version")==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp,sqlite3_libversion(), (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_OK;
    }
    ਜੇ( strcmp(zArg,"-sourceid")==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp,sqlite3_sourceid(), (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_OK;
    }
    ਜੇ( strcmp(zArg,"-has-codec")==0 ){
      Tcl_Append_ਨਤੀਜਾ(interp,"0",(ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_OK;
    }
    ਜੇ( zArg[0]=='-' ) ਵਾਪਸ sqliteCmdUsage(interp, objv);
  }
  ਲਈ(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    ਜੇ( zArg[0]!='-' ){
      ਜੇ( zFile!=0 ) ਵਾਪਸ sqliteCmdUsage(interp, objv);
      zFile = zArg;
      ਜਾਰੀ;
    }
    ਜੇ( i==objc-1 ) ਵਾਪਸ sqliteCmdUsage(interp, objv);
    i++;
    ਜੇ( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg, "-readonly")==0 ){
      ਪੂਰਨ b;
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ਵਾਪਸ TCL_ERROR;
      ਜੇ( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }ਨਹੀਂਤਾਂ{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg, "-create")==0 ){
      ਪੂਰਨ b;
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ਵਾਪਸ TCL_ERROR;
      ਜੇ( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }ਨਹੀਂਤਾਂ{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg, "-nofollow")==0 ){
      ਪੂਰਨ b;
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ਵਾਪਸ TCL_ERROR;
      ਜੇ( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }ਨਹੀਂਤਾਂ{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg, "-nomutex")==0 ){
      ਪੂਰਨ b;
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ਵਾਪਸ TCL_ERROR;
      ਜੇ( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }ਨਹੀਂਤਾਂ{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg, "-fullmutex")==0 ){
      ਪੂਰਨ b;
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ਵਾਪਸ TCL_ERROR;
      ਜੇ( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }ਨਹੀਂਤਾਂ{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg, "-uri")==0 ){
      ਪੂਰਨ b;
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ਵਾਪਸ TCL_ERROR;
      ਜੇ( b ){
        flags |= SQLITE_OPEN_URI;
      }ਨਹੀਂਤਾਂ{
        flags &= ~SQLITE_OPEN_URI;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( strcmp(zArg, "-translatefilename")==0 ){
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        ਵਾਪਸ TCL_ERROR;
      }
    }ਨਹੀਂਤਾਂ{
      Tcl_Append_ਨਤੀਜਾ(interp, "unknown option: ", zArg, (ਅੱਖਰ*)0);
      ਵਾਪਸ TCL_ERROR;
    }
  }
  z_Err_ਸੁਨੇਹਾ = 0;
  p = (SqliteDb*)Tcl_Alloc( ਆਕਾਰ(*p) );
  memset(p, 0, ਆਕਾਰ(*p));
  ਜੇ( zFile==0 ) zFile = "";
  ਜੇ( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  ਜੇ( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  ਜੇ( p->db ){
    ਜੇ( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }ਨਹੀਂਤਾਂ{
    z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  ਜੇ( p->db==0 ){
    Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, z_Err_ਸੁਨੇਹਾ, TCL_VOLATILE);
    Tcl_Free((ਅੱਖਰ*)p);
    sqlite3_free(z_Err_ਸੁਨੇਹਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  p->maxStmt = ਅੰਕ_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  ਜੇ( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (ਅੱਖਰ*)p, DbDeleteCmd);
  }ਨਹੀਂਤਾਂ{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (ਅੱਖਰ*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  ਵਾਪਸ TCL_OK;
}

/*
** Provide a dummy Tcl_InitStubs if we are using this as a static
** library.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE_TCL_STUBS
# undef  Tcl_InitStubs
# define Tcl_InitStubs(a,b,c) TCL_ਸੰਸਕਰਣ
#ਜੇ_ਅੰਤ

/*
** Make sure we have a PACKAGE_VERSION macro defined.  This will be
** defined automatically by the TEA makefile.  But other makefiles
** do not define it.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ PACKAGE_ਸੰਸਕਰਣ
# define PACKAGE_ਸੰਸਕਰਣ SQLITE_ਸੰਸਕਰਣ
#ਜੇ_ਅੰਤ

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
EXTERN ਪੂਰਨ Sqlite3_Init(Tcl_Interp *interp){
  ਪੂਰਨ rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  ਜੇ( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ਜੇ_ਅੰਤ
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_ਸੰਸਕਰਣ);
  }
  ਵਾਪਸ rc;
}
EXTERN ਪੂਰਨ Tclsqlite3_Init(Tcl_Interp *interp){ ਵਾਪਸ Sqlite3_Init(interp); }
EXTERN ਪੂਰਨ Sqlite3_Unload(Tcl_Interp *interp, ਪੂਰਨ flags){ ਵਾਪਸ TCL_OK; }
EXTERN ਪੂਰਨ Tclsqlite3_Unload(Tcl_Interp *interp, ਪੂਰਨ flags){ ਵਾਪਸ TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN ਪੂਰਨ Sqlite3_ਸੁਰੱਖਿਅਤ_Init(Tcl_Interp *interp){ ਵਾਪਸ TCL_ERROR; }
EXTERN ਪੂਰਨ Sqlite3_ਸੁਰੱਖਿਅਤ_Unload(Tcl_Interp *interp, ਪੂਰਨ flags){ਵਾਪਸ TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN ਪੂਰਨ Sqlite_Init(Tcl_Interp *interp){ ਵਾਪਸ Sqlite3_Init(interp);}
EXTERN ਪੂਰਨ Tclsqlite_Init(Tcl_Interp *interp){ ਵਾਪਸ Sqlite3_Init(interp); }
EXTERN ਪੂਰਨ Sqlite_Unload(Tcl_Interp *interp, ਪੂਰਨ flags){ ਵਾਪਸ TCL_OK; }
EXTERN ਪੂਰਨ Tclsqlite_Unload(Tcl_Interp *interp, ਪੂਰਨ flags){ ਵਾਪਸ TCL_OK; }
EXTERN ਪੂਰਨ Sqlite_ਸੁਰੱਖਿਅਤ_Init(Tcl_Interp *interp){ ਵਾਪਸ TCL_ERROR; }
EXTERN ਪੂਰਨ Sqlite_ਸੁਰੱਖਿਅਤ_Unload(Tcl_Interp *interp, ਪੂਰਨ flags){ਵਾਪਸ TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN ਪੂਰਨ sqlite3_Init(Tcl_Interp *interp){ ਵਾਪਸ Sqlite3_Init(interp);}
EXTERN ਪੂਰਨ sqlite_Init(Tcl_Interp *interp){ ਵਾਪਸ Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#ਜੇ defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *tclsh_main_loop(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ zMainloop[] =
    "if {[llength $argv]>=1} {\n"
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIN32
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
#ਜੇ_ਅੰਤ
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
  ਵਾਪਸ zMainloop;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TCLSH_MAIN
# define TCLSH_MAIN ਮੁੱਖ
#ਜੇ_ਅੰਤ
ਪੂਰਨ SQLITE_CDECL TCLSH_MAIN(ਪੂਰਨ argc, ਅੱਖਰ **argv){
  Tcl_Interp *interp;
  ਪੂਰਨ i;
  ਅਚਲ ਅੱਖਰ *zScript = 0;
  ਅੱਖਰ zArgc[32];
#ਜੇ defined(TCLSH_INIT_PROC)
  ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *TCLSH_INIT_PROC(Tcl_Interp*);
#ਜੇ_ਅੰਤ

#ਜੇ !defined(_WIN32_WCE)
  ਜੇ( getenv("SQLITE_DEBUG_BREAK") ){
    ਜੇ( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }ਨਹੀਂਤਾਂ{
#ਜੇ defined(_WIN32) || defined(WIN32)
      DebugBreak();
#ਜਾਂਜੇ defined(SIGTRAP)
      ਉਠਾਓ(SIGTRAP);
#ਜੇ_ਅੰਤ
    }
  }
#ਜੇ_ਅੰਤ

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_ਲੱਭੋ_Executable(argv[0]);
  Tcl_ਸੈੱਟ_System_Encoding(ਨਿਰਾ, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(ਆਕਾਰ(zArgc), zArgc, "%d", argc-1);
  Tcl_ਸੈੱਟ_Var(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_ਸੈੱਟ_Var(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_ਸੈੱਟ_Var(interp,"argv", "", TCL_GLOBAL_ONLY);
  ਲਈ(i=1; i<argc; i++){
    Tcl_ਸੈੱਟ_Var(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_ਮੁੱਲ);
  }
#ਜੇ defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#ਜੇ_ਅੰਤ
  ਜੇ( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  ਜੇ( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    ਅਚਲ ਅੱਖਰ *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    ਜੇ( zInfo==0 ) zInfo = Tcl_Get_String_ਨਤੀਜਾ(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* TCLSH */
