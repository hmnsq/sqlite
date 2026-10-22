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
#ଯଦି defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#ଯଦି defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#ଅନ୍ୟଥା
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#ଯଦି TCL_MAJOR_VERSION==9
# define CONST ଅଚଳ
#ନଚେତଯଦି !defined(Tcl_Size)
  ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା Tcl_Size;
# ifndef Tcl_Bounce_Ref_ଗଣନା
#  define Tcl_Bounce_Ref_ଗଣନା(X) Tcl_Incr_Ref_ଗଣନା(X); Tcl_Decr_Ref_ଗଣନା(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#endif
/**** End copy of tclsqlite.h ****/

#ଅନ୍ତର୍ଭାଵ <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ifndef SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ଅକ୍ଷର u8;
# ifndef SQLITE_PTRSIZE
#   ଯଦି defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   ଅନ୍ୟଥା
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# ଯଦି defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    ପ୍ରକାରବ୍ୟାଖ୍ୟା uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା uptr;
# ଅନ୍ୟଥା
    ପ୍ରକାରବ୍ୟାଖ୍ୟା sqlite3_uint64 uptr;
# endif
#endif
#ଅନ୍ତର୍ଭାଵ <ctype.h>

/* Used to get the current process ID */
#ଯଦି !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#ନଚେତଯଦି !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)GetCurrentProcessId
#endif

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ifdef BUILD_sqlite
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_sqlite */

#define ଅଙ୍କ_PREPARED_STMTS 10
#define MAX_PREPARED_STMTS 100

/* Forward declaration */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#ଯଦି defined(SQLITE_ENABLE_QRF_IN_TCL)
#ଅନ୍ତର୍ଭାଵ "qrf.h"
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା SqlFunc SqlFunc;
ସଂରଚନା SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType;            /* Type of value to return */
  ଅକ୍ଷର *zName;          /* Name of this function */
  SqlFunc *pNext;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା SqlCollate SqlCollate;
ସଂରଚନା SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  ଅକ୍ଷର *zScript;        /* The script to be run */
  SqlCollate *pNext;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା SqlPreparedStmt SqlPreparedStmt;
ସଂରଚନା SqlPreparedStmt {
  SqlPreparedStmt *pNext;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSql;                /* chars in zSql[] */
  ଅଚଳ ଅକ୍ଷର *zSql;        /* Text of the SQL statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
ସଂରଚନା SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  ଅକ୍ଷର *zBusy;               /* The busy callback routine */
  ଅକ୍ଷର *zCommit;             /* The commit hook callback routine */
  ଅକ୍ଷର *zTrace;              /* The trace callback routine */
  ଅକ୍ଷର *zTraceV2;            /* The trace_v2 callback routine */
  ଅକ୍ଷର *zProfile;            /* The profile callback routine */
  ଅକ୍ଷର *zProgress;           /* The progress callback routine */
  ଅକ୍ଷର *zBindFallback;       /* Callback to invoke on a binding miss */
  ଅକ୍ଷର *zAuth;               /* The authorization callback routine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା disableAuth;           /* Disable the authorizer if it exists */
  ଅକ୍ଷର *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxStmt;               /* The next maximum number of stmtList */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nStep, nSort, nIndex;  /* Statistics for most recent operation */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVMStep;               /* Another statistic for most recent operation */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTransaction;          /* Number of nested [transaction] methods */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

ସଂରଚନା IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *pNext;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା strlen30(ଅଚଳ ଅକ୍ଷର *z){
  ଅଚଳ ଅକ୍ଷର *z2 = z;
  ଯେତେବେଳେ( *z2 ){ z2++; }
  ଫେରନ୍ତୁ 0x3fffffff & (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
ସ୍ଥିର ଶୂନ୍ୟ closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *pNext;

  ପାଇଁ(p=pDb->pIncrblob; p; p=pNext){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI incrblobClose2(
  ClientData instanceData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା  rc;
  sqlite3 *db = p->pDb->db;

  ଯଦି( flags ){
    p->isClosed |= flags;
    ଫେରନ୍ତୁ TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  ଯଦି( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  ଯଦି( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }
  ଯଦି( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->pNext;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((ଅକ୍ଷର *)p);

  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3_errmsg(db), TCL_VOLATILE);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI incrblobClose(
  ClientData instanceData,
  Tcl_Interp *interp
){
  ଫେରନ୍ତୁ incrblobClose2(instanceData, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI incrblob_ନିବେଶ(
  ClientData instanceData,
  ଅକ୍ଷର *buf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bufSize,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nRead = bufSize;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  ଯଦି( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  ଯଦି( nRead<=0 ){
    ଫେରନ୍ତୁ 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (ଶୂନ୍ୟ *)buf, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nRead, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p->iSeek);
  ଯଦି( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    ଫେରନ୍ତୁ -1;
  }

  p->iSeek += nRead;
  ଫେରନ୍ତୁ nRead;
}

/*
** Write data to an incremental blob channel.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI incrblobOutput(
  ClientData instanceData,
  ଅଚଳ ଅକ୍ଷର *buf,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା toWrite,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  ଯଦି( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    ଫେରନ୍ତୁ -1;
  }
  ଯଦି( nWrite<=0 ){
    ଫେରନ୍ତୁ 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (ଶୂନ୍ୟ*)buf,(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nWrite, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p->iSeek);
  ଯଦି( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    ଫେରନ୍ତୁ -1;
  }

  p->iSeek += nWrite;
  ଫେରନ୍ତୁ nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#ଯଦି TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#ଅନ୍ୟଥା
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
ସ୍ଥିର WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  ClientData instanceData,
  WideSeekProcType offset,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା seekMode,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;

  ବାଛ( seekMode ){
    ସ୍ଥିତି SEEK_SET:
      p->iSeek = offset;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି SEEK_CUR:
      p->iSeek += offset;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି SEEK_END:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ପ୍ରତିଜ୍ଞା(!"Bad seekMode");
  }

  ଫେରନ୍ତୁ p->iSeek;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI incrblobSeek(
  ClientData instanceData,
  long offset,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା seekMode,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *errorCodePtr
){
  ଫେରନ୍ତୁ incrblobWideSeek(instanceData,offset,seekMode,errorCodePtr);
}


ସ୍ଥିର ଶୂନ୍ୟ SQLITE_TCLAPI incrblobWatch(
  ClientData instanceData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode
){
  /* NO-OP */
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI incrblobHandle(
  ClientData instanceData,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir,
  ClientData *hPtr
){
  ଫେରନ୍ତୁ TCL_ERROR;
}

ସ୍ଥିର Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_VERSION_5,             /* version                              */
  incrblobClose,                     /* closeProc                            */
  incrblob_ନିବେଶ,                     /* inputProc                            */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  ଅଚଳ ଅକ୍ଷର *zDb,
  ଅଚଳ ଅକ୍ଷର *zTable,
  ଅଚଳ ଅକ୍ଷର *zColumn,
  sqlite_int64 iRow,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
  ଅକ୍ଷର zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(ଆକାର(IncrblobChannel));
  memset(p, 0, ଆକାର(*p));
  p->pBlob = pBlob;
  ଯଦି( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(ଆକାର(zChannel), zChannel, "incrblob_%d", ++ଗଣନା_2);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->pNext = pDb->pIncrblob;
  p->pPrev = 0;
  ଯଦି( p->pNext ){
    p->pNext->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_SetResult(interp, (ଅକ୍ଷର *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_OK;
}
#ଅନ୍ୟଥା  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  ଅଚଳ ଅକ୍ଷର *z;
  Tcl_Size n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  ଯେତେବେଳେ( n-- > 0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = *(z++);
    ଯଦି( c=='$' || c=='[' || c==';' ) ଫେରନ୍ତୁ 0;
  }
  ଫେରନ୍ତୁ 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
ସ୍ଥିର SqlFunc *findSqlFunc(SqliteDb *pDb, ଅଚଳ ଅକ୍ଷର *zName){
  SqlFunc *p, *pNew;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( ଆକାର(*pNew) + nName + 1 );
  pNew->zName = (ଅକ୍ଷର*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  ପାଇଁ(p=pDb->pFunc; p; p=p->pNext){
    ଯଦି( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((ଅକ୍ଷର*)pNew);
      ଫେରନ୍ତୁ p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->pNext = pDb->pFunc;
  pDb->pFunc = pNew;
  ଫେରନ୍ତୁ pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
ସ୍ଥିର ଶୂନ୍ୟ dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  ଯଦି( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((ଅକ୍ଷର *)pStmt->zSql);
  }
#endif
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((ଅକ୍ଷର *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
ସ୍ଥିର ଶୂନ୍ୟ flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *pNext;

  ପାଇଁ(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pNext){
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
ସ୍ଥିର ଶୂନ୍ୟ addDatabaseRef(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
ସ୍ଥିର ଶୂନ୍ୟ delDatabaseRef(SqliteDb *pDb){
  ପ୍ରତିଜ୍ଞା( pDb->nRef>0 );
  pDb->nRef--;
  ଯଦି( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    ଯେତେବେଳେ( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->pNext;
      ପ୍ରତିଜ୍ଞା( pFunc->pDb==pDb );
      Tcl_Decr_Ref_ଗଣନା(pFunc->pScript);
      Tcl_Free((ଅକ୍ଷର*)pFunc);
    }
    ଯେତେବେଳେ( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->pNext;
      Tcl_Free((ଅକ୍ଷର*)pCollate);
    }
    ଯଦି( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    ଯଦି( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    ଯଦି( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    ଯଦି( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    ଯଦି( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    ଯଦି( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    ଯଦି( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    ଯଦି( pDb->pUpdateHook ){
      Tcl_Decr_Ref_ଗଣନା(pDb->pUpdateHook);
    }
    ଯଦି( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_ଗଣନା(pDb->pPreUpdateHook);
    }
    ଯଦି( pDb->pRollbackHook ){
      Tcl_Decr_Ref_ଗଣନା(pDb->pRollbackHook);
    }
    ଯଦି( pDb->pWalHook ){
      Tcl_Decr_Ref_ଗଣନା(pDb->pWalHook);
    }
    ଯଦି( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_ଗଣନା(pDb->pCollateNeeded);
    }
    Tcl_Free((ଅକ୍ଷର*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
ସ୍ଥିର ଶୂନ୍ୟ SQLITE_TCLAPI DbDeleteCmd(ଶୂନ୍ୟ *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DbBusyHandler(ଶୂନ୍ୟ *cd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଅକ୍ଷର zVal[30];

  sqlite3_snprintf(ଆକାର(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (ଅକ୍ଷର*)0);
  ଯଦି( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ଫେରନ୍ତୁ 0;
  }
  ଫେରନ୍ତୁ 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DbProgressHandler(ଶୂନ୍ୟ *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ପ୍ରତିଜ୍ଞା( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  ଯଦି( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}
#endif

#ଯଦି !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
ସ୍ଥିର ଶୂନ୍ୟ DbTraceHandler(ଶୂନ୍ୟ *cd, ଅଚଳ ଅକ୍ଷର *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_ମୂଲ୍ୟ(&str));
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DbTraceV2Handler(
  unsigned type, /* One of the SQLITE_TRACE_* event types. */
  ଶୂନ୍ୟ *cd,      /* The original context data pointer. */
  ଶୂନ୍ୟ *pd,      /* Primary event data, depends on event type. */
  ଶୂନ୍ୟ *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  ବାଛ( type ){
    ସ୍ଥିତି SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      ଅକ୍ଷର *zSql = (ଅକ୍ଷର *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ଗଣନା(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_ଗଣନା(pCmd);
      Tcl_ResetResult(pDb->interp);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ଗଣନା(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_ଗଣନା(pCmd);
      Tcl_ResetResult(pDb->interp);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ଗଣନା(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_ଗଣନା(pCmd);
      Tcl_ResetResult(pDb->interp);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ଗଣନା(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_ଗଣନା(pCmd);
      Tcl_ResetResult(pDb->interp);
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}
#endif

#ଯଦି !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
ସ୍ଥିର ଶୂନ୍ୟ DbProfileHandler(ଶୂନ୍ୟ *cd, ଅଚଳ ଅକ୍ଷର *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  ଅକ୍ଷର zTm[100];

  sqlite3_snprintf(ଆକାର(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_ମୂଲ୍ୟ(&str));
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DbCommitHandler(ଶୂନ୍ୟ *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  ଯଦି( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ DbRollbackHandler(ଶୂନ୍ୟ *clientData){
  SqliteDb *pDb = (SqliteDb*)clientData;
  ପ୍ରତିଜ୍ଞା(pDb->pRollbackHook);
  ଯଦି( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DbWalHandler(
  ଶୂନ୍ୟ *clientData,
  sqlite3 *db,
  ଅଚଳ ଅକ୍ଷର *zDb,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nEntry
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)clientData;
  Tcl_Interp *interp = pDb->interp;
  ପ୍ରତିଜ୍ଞା(pDb->pWalHook);

  ପ୍ରତିଜ୍ଞା( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_ଗଣନା(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  ଯଦି( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_ଗଣନା(p);

  ଫେରନ୍ତୁ ret;
}

#ଯଦି defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ସ୍ଥିର ଶୂନ୍ୟ setTestUnlockNotifyVars(Tcl_Interp *interp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iArg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg){
  ଅକ୍ଷର zBuf[64];
  sqlite3_snprintf(ଆକାର(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(ଆକାର(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#ଅନ୍ୟଥା
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
ସ୍ଥିର ଶୂନ୍ୟ DbUnlockNotify(ଶୂନ୍ୟ **apArg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<nArg; i++){
    ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    setTestUnlockNotifyVars(pDb->interp, i, nArg);
    ପ୍ରତିଜ୍ଞା( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_ଗଣନା(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
ସ୍ଥିର ଶୂନ୍ୟ DbPreUpdateHandler(
  ଶୂନ୍ୟ *p,
  sqlite3 *db,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,
  ଅଚଳ ଅକ୍ଷର *zDb,
  ଅଚଳ ଅକ୍ଷର *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ପ୍ରତିଜ୍ଞା( (SQLITE_DELETE-1)/9 == 0 );
  ପ୍ରତିଜ୍ଞା( (SQLITE_ଭର୍ତ୍ତି-1)/9 == 1 );
  ପ୍ରତିଜ୍ଞା( (SQLITE_UPDATE-1)/9 == 2 );
  ପ୍ରତିଜ୍ଞା( pDb->pPreUpdateHook );
  ପ୍ରତିଜ୍ଞା( db==pDb->db );
  ପ୍ରତିଜ୍ଞା( op==SQLITE_ଭର୍ତ୍ତି || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_ଗଣନା(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_ଗଣନା(pCmd);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

ସ୍ଥିର ଶୂନ୍ୟ DbUpdateHandler(
  ଶୂନ୍ୟ *p,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op,
  ଅଚଳ ଅକ୍ଷର *zDb,
  ଅଚଳ ଅକ୍ଷର *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ପ୍ରତିଜ୍ଞା( (SQLITE_DELETE-1)/9 == 0 );
  ପ୍ରତିଜ୍ଞା( (SQLITE_ଭର୍ତ୍ତି-1)/9 == 1 );
  ପ୍ରତିଜ୍ଞା( (SQLITE_UPDATE-1)/9 == 2 );

  ପ୍ରତିଜ୍ଞା( pDb->pUpdateHook );
  ପ୍ରତିଜ୍ଞା( op==SQLITE_ଭର୍ତ୍ତି || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_ଗଣନା(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_ଗଣନା(pCmd);
}

ସ୍ଥିର ଶୂନ୍ୟ tclCollateNeeded(
  ଶୂନ୍ୟ *pCtx,
  sqlite3 *db,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା enc,
  ଅଚଳ ଅକ୍ଷର *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_ଗଣନା(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_ଗଣନା(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclSqlCollate(
  ଶୂନ୍ୟ *pCtx,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nA,
  ଅଚଳ ଶୂନ୍ୟ *zA,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nB,
  ଅଚଳ ଶୂନ୍ୟ *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_ଗଣନା(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_ଗଣନା(pCmd);
  ଫେରନ୍ତୁ (atoi(Tcl_GetStringResult(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
ସ୍ଥିର ଶୂନ୍ୟ tclSqlFunc(sqlite3_context *context, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ**argv){
  SqlFunc *p = sqlite3_user_data(context);
  Tcl_Obj *pCmd;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_ଗଣନା(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_ଗଣନା(pCmd);
  }ଅନ୍ୟଥା{
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
    ଯଦି( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
      ଫେରନ୍ତୁ;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_ଗଣନା(pCmd);
    ପାଇଁ(i=0; i<argc; i++){
      sqlite3_ମୂଲ୍ୟ *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      ବାଛ( sqlite3_ମୂଲ୍ୟ_type(pIn) ){
        ସ୍ଥିତି SQLITE_BLOB: {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes = sqlite3_ମୂଲ୍ୟ_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_ମୂଲ୍ୟ_blob(pIn), bytes);
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_ମୂଲ୍ୟ_int64(pIn);
          ଯଦି( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)v);
          }ଅନ୍ୟଥା{
            pVal = Tcl_NewWideIntObj(v);
          }
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି SQLITE_FLOAT: {
          ଦ୍ୱିଗୁଣ r = sqlite3_ମୂଲ୍ୟ_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          ଭାଙ୍ଗ;
        }
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes = sqlite3_ମୂଲ୍ୟ_bytes(pIn);
          pVal = Tcl_NewStringObj((ଅକ୍ଷର *)sqlite3_ମୂଲ୍ୟ_text(pIn), bytes);
          ଭାଙ୍ଗ;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      ଯଦି( rc ){
        Tcl_Decr_Ref_ଗଣନା(pCmd);
        sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
        ଫେରନ୍ତୁ;
      }
    }
    ଯଦି( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_Decr_Ref_ଗଣନା(pCmd);
  }

  ଯଦି( TCL_BREAK==rc ){
    sqlite3_result_null(context);
  }ଅନ୍ୟଥା ଯଦି( rc && rc!=TCL_RETURN ){
    sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
  }ଅନ୍ୟଥା{
    Tcl_Obj *pVar = Tcl_GetObjResult(p->interp);
    Tcl_Size n;
    u8 *data;
    ଅଚଳ ଅକ୍ଷର *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    ଅକ୍ଷର c = zType[0];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType = p->eType;

    ଯଦି( eType==SQLITE_NULL ){
      ଯଦି( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }ଅନ୍ୟଥା ଯଦି( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }ଅନ୍ୟଥା ଯଦି( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }ଅନ୍ୟଥା{
        eType = SQLITE_TEXT;
      }
    }

    ବାଛ( eType ){
      ସ୍ଥିତି SQLITE_BLOB: {
        data = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_result_blob64(context, data, n, SQLITE_TRANSIENT);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି SQLITE_INTEGER: {
        Tcl_WideInt v;
        ଯଦି( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_result_int64(context, v);
          ଭାଙ୍ଗ;
        }
        /* fall-through */
      }
      ସ୍ଥିତି SQLITE_FLOAT: {
        ଦ୍ୱିଗୁଣ r;
        ଯଦି( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_result_double(context, r);
          ଭାଙ୍ଗ;
        }
        /* fall-through */
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        data = (unsigned ଅକ୍ଷର *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_result_text64(context, (ଅକ୍ଷର *)data, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        ଭାଙ୍ଗ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା auth_callback(
  ଶୂନ୍ୟ *pArg,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା code,
  ଅଚଳ ଅକ୍ଷର *zArg1,
  ଅଚଳ ଅକ୍ଷର *zArg2,
  ଅଚଳ ଅକ୍ଷର *zArg3,
  ଅଚଳ ଅକ୍ଷର *zArg4
){
  ଅଚଳ ଅକ୍ଷର *zCode;
  Tcl_DString str;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଅଚଳ ଅକ୍ଷର *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  ଯଦି( pDb->disableAuth ) ଫେରନ୍ତୁ SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  ବାଛ( code ){
    ସ୍ଥିତି SQLITE_COPY              : zCode="SQLITE_COPY"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_INDEX      : zCode="SQLITE_CREATE_INDEX"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_TEMP_INDEX : zCode="SQLITE_CREATE_TEMP_INDEX"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_TEMP_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_TEMP_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_TEMP_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DELETE            : zCode="SQLITE_DELETE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_INDEX        : zCode="SQLITE_DROP_INDEX"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_TEMP_INDEX   : zCode="SQLITE_DROP_TEMP_INDEX"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_TEMP_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_TEMP_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_TEMP_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_ଭର୍ତ୍ତି            : zCode="SQLITE_INSERT"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_READ              : zCode="SQLITE_READ"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_SELECT            : zCode="SQLITE_SELECT"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DETACH            : zCode="SQLITE_DETACH"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ                       : zCode="????"; ଭାଙ୍ଗ;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, zCode);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_ମୂଲ୍ୟ(&str));
  Tcl_DStringFree(&str);
  zReply = rc==TCL_OK ? Tcl_GetStringResult(pDb->interp) : "SQLITE_DENY";
  ଯଦି( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }ଅନ୍ୟଥା ଯଦି( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }ଅନ୍ୟଥା ଯଦି( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }ଅନ୍ୟଥା{
    rc = 999;
  }
  ଫେରନ୍ତୁ rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#ଯଦି 0
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
ସ୍ଥିର ଅକ୍ଷର *local_getline(ଅକ୍ଷର *zPrompt, FILE *in){
  ଅକ୍ଷର *z_ଧାଡ଼ି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଧାଡ଼ି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

  n_ଧାଡ଼ି = 100;
  z_ଧାଡ଼ି = malloc( n_ଧାଡ଼ି );
  ଯଦି( z_ଧାଡ଼ି==0 ) ଫେରନ୍ତୁ 0;
  n = 0;
  ଯେତେବେଳେ( 1 ){
    ଯଦି( n+100>n_ଧାଡ଼ି ){
      n_ଧାଡ଼ି = n_ଧାଡ଼ି*2 + 100;
      z_ଧାଡ଼ି = realloc(z_ଧାଡ଼ି, n_ଧାଡ଼ି);
      ଯଦି( z_ଧାଡ଼ି==0 ) ଫେରନ୍ତୁ 0;
    }
    ଯଦି( fgets(&z_ଧାଡ଼ି[n], n_ଧାଡ଼ି - n, in)==0 ){
      ଯଦି( n==0 ){
        ମୁକ୍ତ(z_ଧାଡ଼ି);
        ଫେରନ୍ତୁ 0;
      }
      z_ଧାଡ଼ି[n] = 0;
      ଭାଙ୍ଗ;
    }
    ଯେତେବେଳେ( z_ଧାଡ଼ି[n] ){ n++; }
    ଯଦି( n>0 && z_ଧାଡ଼ି[n-1]=='\n' ){
      n--;
      z_ଧାଡ଼ି[n] = 0;
      ଭାଙ୍ଗ;
    }
  }
  z_ଧାଡ଼ି = realloc( z_ଧାଡ଼ି, n+1 );
  ଫେରନ୍ତୁ z_ଧାଡ଼ି;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI DbTransPostCmd(
  ClientData data[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା result                           /* Result of evaluating SCRIPT */
){
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)data[0];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = result;
  ଅଚଳ ଅକ୍ଷର *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  ଯଦି( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    ଯଦି( rc!=TCL_ERROR ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  ଫେରନ୍ତୁ rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  ଅଚଳ ଅକ୍ଷର *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  ଅଚଳ ଅକ୍ଷର **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା prepFlags = 0;
#ifdef SQLITE_TEST
  ଯଦି( pDb->bLegacyPrepare ){
    ଫେରନ୍ତୁ sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  ଯଦି( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  ଫେରନ୍ତୁ sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  ଅକ୍ଷର ଅଚଳ *zIn,                /* SQL to compile */
  ଅକ୍ଷର ଅଚଳ **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  ଅଚଳ ଅକ୍ଷର *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSql;                       /* Length of zSql in bytes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVar = 0;                   /* Number of variables in statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iParm = 0;                  /* Next free entry in apParm */
  ଅକ୍ଷର c;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା needResultReset = 0;        /* Need to invoke Tcl_ResetResult() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  ଯେତେବେଳେ( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  ପାଇଁ(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->pNext){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = pPreStmt->nSql;
    ଯଦି( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      ଯଦି( pPreStmt->pPrev ){
        pPreStmt->pPrev->pNext = pPreStmt->pNext;
      }ଅନ୍ୟଥା{
        pDb->stmtList = pPreStmt->pNext;
      }
      ଯଦି( pPreStmt->pNext ){
        pPreStmt->pNext->pPrev = pPreStmt->pPrev;
      }ଅନ୍ୟଥା{
        pDb->stmtLast = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_ଗଣନା(pStmt);
      ଭାଙ୍ଗ;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  ଯଦି( pPreStmt==0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;

    ଯଦି( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( pStmt==0 ){
      ଯଦି( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ଫେରନ୍ତୁ TCL_ERROR;
      }ଅନ୍ୟଥା{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        ଫେରନ୍ତୁ TCL_OK;
      }
    }

    ପ୍ରତିଜ୍ଞା( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_ଗଣନା(pStmt);
    nByte = ଆକାର(SqlPreparedStmt) + nVar*ଆକାର(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    ଯଦି( pPreStmt->zSql==0 ){
      ଅକ୍ଷର *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      memcpy(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#endif
  }
  ପ୍ରତିଜ୍ଞା( pPreStmt );
  ପ୍ରତିଜ୍ଞା( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  ପ୍ରତିଜ୍ଞା( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  ପାଇଁ(i=1; i<=nVar; i++){
    ଅଚଳ ଅକ୍ଷର *zVar = sqlite3_bind_parameter_name(pStmt, i);
    ଯଦି( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      ଯଦି( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_ଗଣନା(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        ଯଦି( needResultReset ) Tcl_ResetResult(interp);
        needResultReset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_Decr_Ref_ଗଣନା(pCmd);
        ଯଦି( rx==TCL_OK ){
          pVar = Tcl_GetObjResult(interp);
        }ଅନ୍ୟଥା ଯଦି( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା{
          pVar = 0;
        }
      }
      ଯଦି( pVar ){
        Tcl_Size n;
        u8 *data;
        ଅଚଳ ଅକ୍ଷର *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        ଯଦି( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          data = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, data, n, SQLITE_STATIC);
          Tcl_Incr_Ref_ଗଣନା(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }ଅନ୍ୟଥା ଯଦି( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }ଅନ୍ୟଥା ଯଦି( c=='d' && strcmp(zType,"double")==0 ){
          ଦ୍ୱିଗୁଣ r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }ଅନ୍ୟଥା ଯଦି( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }ଅନ୍ୟଥା{
          data = (unsigned ଅକ୍ଷର *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (ଅକ୍ଷର *)data, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_ଗଣନା(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }ଅନ୍ୟଥା{
        sqlite3_bind_null(pStmt, i);
      }
      ଯଦି( needResultReset ) Tcl_ResetResult(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  ଯଦି( needResultReset && rc==TCL_OK ) Tcl_ResetResult(pDb->interp);

  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ଶୂନ୍ୟ dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା discard                     /* True to delete (not cache) the pPreStmt */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* Free the bound string and blob parameters */
  ପାଇଁ(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_ଗଣନା(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  ଯଦି( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }ଅନ୍ୟଥା{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->pNext = pDb->stmtList;
    pPreStmt->pPrev = 0;
    ଯଦି( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    ଯଦି( pDb->stmtLast==0 ){
      ପ୍ରତିଜ୍ଞା( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    ଯେତେବେଳେ( pDb->nStmt>pDb->maxStmt ){
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା DbEvalContext DbEvalContext;
ସଂରଚନା DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  ଅଚଳ ଅକ୍ଷର *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;                       /* Number of columns returned by pStmt */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#define SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#define SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
ସ୍ଥିର ଶୂନ୍ୟ dbReleaseColumnNames(DbEvalContext *p){
  ଯଦି( p->apColName ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<p->nCol; i++){
      Tcl_Decr_Ref_ଗଣନା(p->apColName[i]);
    }
    Tcl_Free((ଅକ୍ଷର *)p->apColName);
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
ସ୍ଥିର ଶୂନ୍ୟ dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, ଆକାର(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_ଗଣନା(pSql);
  ଯଦି( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_ଗଣନା(pVarName);
  }
  p->evalFlags = evalFlags;
  addDatabaseRef(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
ସ୍ଥିର ଶୂନ୍ୟ dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  ଯଦି( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                        /* Iterator variable */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_ଗଣନା(pStmt);
    ଯଦି( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( ଆକାର(Tcl_Obj*)*nCol );
      ପାଇଁ(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_Incr_Ref_ଗଣନା(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    ଯଦି( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_ଗଣନା(pColList);
      Tcl_Incr_Ref_ଗଣନା(pStar);
      ପାଇଁ(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      ଯଦି( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }ଅନ୍ୟଥା{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, ଖାଲି, 0);
        ଯଦି( !pDict ){
          pDict = Tcl_NewDictObj();
        }ଅନ୍ୟଥା ଯଦି( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ଯଦି( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, ଖାଲି, pDict, 0);
        }
        Tcl_Bounce_Ref_ଗଣନା(pDict);
      }
      Tcl_Decr_Ref_ଗଣନା(pStar);
      Tcl_Decr_Ref_ଗଣନା(pColList);
    }
  }

  ଯଦି( papColName ){
    *papColName = p->apColName;
  }
  ଯଦି( pnCol ){
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbEvalStep(DbEvalContext *p){
  ଅଚଳ ଅକ୍ଷର *zPrevSql = 0;       /* Previous value of p->zSql */

  ଯେତେବେଳେ( p->zSql[0] || p->pPreStmt ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ଯଦି( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      ଯଦି( rc!=TCL_OK ) ଫେରନ୍ତୁ rc;
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      ଯଦି( rcs==SQLITE_ROW ){
        ଫେରନ୍ତୁ TCL_OK;
      }
      ଯଦି( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->nIndex = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      ଯଦି( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#ଯଦି SQLITE_TEST
        ଯଦି( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          ଜାରି;
        }
#endif
        Tcl_SetObjResult(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ଫେରନ୍ତୁ TCL_ERROR;
      }ଅନ୍ୟଥା{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  ଫେରନ୍ତୁ TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
ସ୍ଥିର ଶୂନ୍ୟ dbEvalFinalize(DbEvalContext *p){
  ଯଦି( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  ଯଦି( p->pVarName ){
    Tcl_Decr_Ref_ଗଣନା(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_ଗଣନା(p->pSql);
  dbReleaseColumnNames(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
ସ୍ଥିର Tcl_Obj *db_Eval_Column_ମୂଲ୍ୟ(DbEvalContext *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  ବାଛ( sqlite3_column_type(pStmt, iCol) ){
    ସ୍ଥିତି SQLITE_BLOB: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes = sqlite3_column_bytes(pStmt, iCol);
      ଅଚଳ ଅକ୍ଷର *zBlob = sqlite3_column_blob(pStmt, iCol);
      ଯଦି( !zBlob ) bytes = 0;
      ଫେରନ୍ତୁ Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    ସ୍ଥିତି SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      ଯଦି( v>=-2147483647 && v<=2147483647 ){
        ଫେରନ୍ତୁ Tcl_NewIntObj((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)v);
      }ଅନ୍ୟଥା{
        ଫେରନ୍ତୁ Tcl_NewWideIntObj(v);
      }
    }
    ସ୍ଥିତି SQLITE_FLOAT: {
      ଫେରନ୍ତୁ Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    ସ୍ଥିତି SQLITE_NULL: {
      ଫେରନ୍ତୁ Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  ଫେରନ୍ତୁ Tcl_NewStringObj((ଅକ୍ଷର*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#ଯଦି TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା DbUseNre(ଶୂନ୍ୟ){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  ଫେରନ୍ତୁ( (major==8 && minor>=6) || major>8 );
}
#ଅନ୍ୟଥା
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (ଶୂନ୍ୟ)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (ଶୂନ୍ୟ)0
#endif

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI DbEvalNextCmd(
  ClientData data[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା result                           /* Result so far */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = result;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)data[0];
  Tcl_Obj * ଅଚଳ pScript = (Tcl_Obj *)data[1];
  Tcl_Obj * ଅଚଳ pVarName = p->pVarName;

  ଯେତେବେଳେ( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    ପାଇଁ(i=0; i<nCol; i++){
      ଯଦି( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, db_Eval_Column_ମୂଲ୍ୟ(p,i), 0);
      }ଅନ୍ୟଥା ଯଦି( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        ଯଦି( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }ଅନ୍ୟଥା{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ଖାଲି, 0);
          ଯଦି( pDict ){
            ଯଦି( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            ଯଦି( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, ଖାଲି, pDict, 0);
            }
            Tcl_Bounce_Ref_ଗଣନା(pDict);
          }
        }
      }ଅନ୍ୟଥା ଯଦି( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       db_Eval_Column_ମୂଲ୍ୟ(p,i), 0);
      }ଅନ୍ୟଥା{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ଖାଲି, 0);
        ଯଦି( !pDict ){
          pDict = Tcl_NewDictObj();
        }ଅନ୍ୟଥା ଯଦି( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ଯଦି( Tcl_DictObjPut(interp, pDict, apColName[i],
                           db_Eval_Column_ମୂଲ୍ୟ(p,i))==TCL_OK ){
          Tcl_ObjSetVar2(interp, pVarName, ଖାଲି, pDict, 0);
        }
        Tcl_Bounce_Ref_ଗଣନା(pDict);
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
    ଯଦି( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbEvalNextCmd, (ଶୂନ୍ୟ*)p, (ଶୂନ୍ୟ*)pScript, 0, 0);
      ଫେରନ୍ତୁ Tcl_NREvalObj(interp, pScript, 0);
    }ଅନ୍ୟଥା{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_ଗଣନା(pScript);
  dbEvalFinalize(p);
  Tcl_Free((ଅକ୍ଷର *)p);

  ଯଦି( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_ResetResult(interp);
    rc = TCL_OK;
  }
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ଶୂନ୍ୟ DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  ଯଦି( *ppHook ){
    Tcl_SetObjResult(interp, *ppHook);
    ଯଦି( pArg ){
      Tcl_Decr_Ref_ଗଣନା(*ppHook);
      *ppHook = 0;
    }
  }
  ଯଦି( pArg ){
    ପ୍ରତିଜ୍ଞା( !(*ppHook) );
    ଯଦି( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_ଗଣନା(*ppHook);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dbQrf(SqliteDb *pDb, ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc, Tcl_Obj *ଅଚଳ*objv){
#ifndef SQLITE_QRF_H
  Tcl_SetResult(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  ଫେରନ୍ତୁ TCL_ERROR;
#ଅନ୍ୟଥା
  ଅକ୍ଷର *zResult = 0;             /* Result to be returned */
  ଅଚଳ ଅକ୍ଷର *zSql = 0;          /* SQL to run */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                         /* Loop counter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_ବାମ,
    QRF_ALIGN_ମଧ୍ୟ, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_ଡାହାଣ,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_ଶୀର୍ଷ,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, ଆକାର(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &zResult;
  ପାଇଁ(i=2; i<objc; i++){
    ଅଚଳ ଅକ୍ଷର *zArg = Tcl_GetString(objv[i]);
    ଅଚଳ ଅକ୍ଷର *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    ଅଚଳ unsigned ଅକ୍ଷର aBoolMap[] = { 0, 2, 1, 2, 1 };
    ଯଦି( zArg[0]!='-' ){
      ଯଦି( zSql ){
        Tcl_AppendResult(pDb->interp, "unknown argument: ", zArg, (ଅକ୍ଷର*)0);
        rc = TCL_ERROR;
        ଯାଅ format_failed;
      }
      zSql  = zArg;
    }ଅନ୍ୟଥା ଯଦି( i==objc-1 ){
      Tcl_AppendResult(pDb->interp, "option has no argument: ", zArg,(ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
      ଯାଅ format_failed;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-style")==0 ){
      ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      ସ୍ଥିର unsigned ଅକ୍ଷର aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_ଗଣନା,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_ଭର୍ତ୍ତି,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_ଧାଡ଼ି,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା style;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      ଯଦି( rc ) ଯାଅ format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-esc")==0 ){
      ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      ସ୍ଥିର unsigned ଅକ୍ଷର aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_ପ୍ରତୀକ
      };
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା esc;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      ଯଦି( rc ) ଯାଅ format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      ସ୍ଥିର unsigned ଅକ୍ଷର aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା txt;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = zArg[2]=='e';
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      ଯଦି( rc ) ଯାଅ format_failed;
      ଯଦି( k ){
        qrf.eText = aTextMap[txt];
      }ଅନ୍ୟଥା ଯଦି( txt<=2 ){
        ପ୍ରତିଜ୍ଞା( QRF_No==1 );
        ପ୍ରତିଜ୍ଞା( QRF_Yes==2 );
        ପ୍ରତିଜ୍ଞା( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }ଅନ୍ୟଥା{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-blob")==0 ){
      ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      ସ୍ଥିର unsigned ଅକ୍ଷର aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_Size
      };
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା blob;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      ଯଦି( rc ) ଯାଅ format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-wordwrap")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      ଯଦି( rc ) ଯାଅ format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      ଯଦି( rc ) ଯାଅ format_failed;
      ଯଦି( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }ଅନ୍ୟଥା ଯଦି( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }ଅନ୍ୟଥା ଯଦି( zArg[1]=='r' ){
        qrf.b_Row_ଗଣନା = aBoolMap[v];
      }ଅନ୍ୟଥା{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ax = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      ଯଦି( rc ) ଯାଅ format_failed;
      ଯଦି( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }ଅନ୍ୟଥା{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ଯଦି( rc ) ଯାଅ format_failed;
      ଯଦି( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }ଅନ୍ୟଥା ଯଦି( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      ଯଦି( zArg[1]=='w' ){
        qrf.nWrap = v;
      }ଅନ୍ୟଥା ଯଦି( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }ଅନ୍ୟଥା ଯଦି( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }ଅନ୍ୟଥା{
        qrf.n_ଧାଡ଼ି_Limit = v;
      }
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-charlimit")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ଯଦି( rc ) ଯାଅ format_failed;
      ଯଦି( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-multiinsert")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ଯଦି( rc ) ଯାଅ format_failed;
      ଯଦି( v<0 ) v = 0;
      qrf.n_Multi_ଭର୍ତ୍ତି = v;
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-align")==0 ){
      Tcl_Size n = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      ଯଦି( rc ) ଯାଅ format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*ଆକାର(qrf.aAlign[0]) );
      ଯଦି( qrf.aAlign==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (ଅକ୍ଷର*)0);
        rc = TCL_ERROR;
        ଯାଅ format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*ଆକାର(qrf.aAlign[0]));
      qrf.nAlign = n;
      ପାଇଁ(jj=0; jj<n; jj++){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
        Tcl_Obj *pTerm;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        ଯଦି( rc ) ଯାଅ format_failed;
        rc = Tcl_GetIndexFromObj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        ଯଦି( rc ) ଯାଅ format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-widths")==0 ){
      Tcl_Size n = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      ଯଦି( rc ) ଯାଅ format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*ଆକାର(qrf.aWidth[0]) );
      ଯଦି( qrf.aWidth==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (ଅକ୍ଷର*)0);
        rc = TCL_ERROR;
        ଯାଅ format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*ଆକାର(qrf.aWidth[0]));
      qrf.nWidth = n;
      ପାଇଁ(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା v;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        ଯଦି( rc ) ଯାଅ format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        ଯଦି( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }ଅନ୍ୟଥା ଯଦି( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short ପୂର୍ଣ୍ଣସଂଖ୍ୟା)v;
      }
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-ifmt")==0 ){
      ଅକ୍ଷର *zFmt = Tcl_GetString(objv[i+1]);
      ଯଦି( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }ଅନ୍ୟଥା ଯଦି( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_AppendResult(pDb->interp, "bad format for -ifmt", (ଅକ୍ଷର*)0);
        rc = TCL_ERROR;
        ଯାଅ format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-fpfmt")==0 ){
      ଅକ୍ଷର *zFmt = Tcl_GetString(objv[i+1]);
      ଯଦି( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }ଅନ୍ୟଥା ଯଦି( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_AppendResult(pDb->interp, "bad format for -fpfmt", (ଅକ୍ଷର*)0);
        rc = TCL_ERROR;
        ଯାଅ format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }ଅନ୍ୟଥା{
      Tcl_AppendResult(pDb->interp, "unknown option: ", zArg, (ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
      ଯାଅ format_failed;
    }
  }
  ଯେତେବେଳେ( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    ଅକ୍ଷର *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    ଯଦି( rc ) ଯାଅ format_failed;
    ଯଦି( pStmt==0 ) ଜାରି;
    rc = sqlite3_format_query_result(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    ଯଦି( rc ){
      Tcl_SetResult(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      ଯାଅ format_failed;
    }
  }
  Tcl_SetResult(pDb->interp, zResult, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.aWidth);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(zResult);
  ଫେରନ୍ତୁ rc;

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI DbObjCmd(
  ଶୂନ୍ୟ *cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *ଅଚଳ*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା choice;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = TCL_OK;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *DB_strs[] = {
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
  ଗଣନା DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_LOAD_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_INTERRUPT,            DB_LAST_ଭର୍ତ୍ତି_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_PROGRESS,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_STATUS,                DB_TIMEOUT,              DB_TOTAL_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_VERSION,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  ଯଦି( objc<2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SUBCOMMAND ...");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ବାଛ( (ଗଣନା DB_enum)choice ){

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
  ସ୍ଥିତି DB_AUTHORIZER: {
#ifdef SQLITE_OMIT_AUTHORIZATION
    Tcl_AppendResult(interp, "authorization not available in this build",
                     (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
#ଅନ୍ୟଥା
    ଯଦି( objc>3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?CALLBACK?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }ଅନ୍ୟଥା ଯଦି( objc==2 ){
      ଯଦି( pDb->zAuth ){
        Tcl_AppendResult(interp, pDb->zAuth, (ଅକ୍ଷର*)0);
      }
    }ଅନ୍ୟଥା{
      ଅକ୍ଷର *zAuth;
      Tcl_Size len;
      ଯଦି( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      ଯଦି( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }ଅନ୍ୟଥା{
        pDb->zAuth = 0;
      }
      ଯଦି( pDb->zAuth ){
        ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*sqlite3_auth_cb)(
           ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*,
           ଅଚଳ ଅକ୍ଷର*,ଅଚଳ ଅକ୍ଷର*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }ଅନ୍ୟଥା{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#endif
    ଭାଙ୍ଗ;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  ସ୍ଥିତି DB_BACKUP: {
    ଅଚଳ ଅକ୍ଷର *zDestFile;
    ଅଚଳ ଅକ୍ଷର *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    ଯଦି( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }ଅନ୍ୟଥା ଯଦି( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }ଅନ୍ୟଥା{
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?DATABASE? FILENAME");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    ଯଦି( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (ଅକ୍ଷର*)0);
      sqlite3_close(pDest);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    ଯଦି( pBackup==0 ){
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (ଅକ୍ଷର*)0);
      sqlite3_close(pDest);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯେତେବେଳେ(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    ଯଦି( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }ଅନ୍ୟଥା{
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    ଭାଙ୍ଗ;
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
  ସ୍ଥିତି DB_BIND_FALLBACK: {
    ଯଦି( objc>3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?CALLBACK?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }ଅନ୍ୟଥା ଯଦି( objc==2 ){
      ଯଦି( pDb->zBindFallback ){
        Tcl_AppendResult(interp, pDb->zBindFallback, (ଅକ୍ଷର*)0);
      }
    }ଅନ୍ୟଥା{
      ଅକ୍ଷର *zCallback;
      Tcl_Size len;
      ଯଦି( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      ଯଦି( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }ଅନ୍ୟଥା{
        pDb->zBindFallback = 0;
      }
    }
    ଭାଙ୍ଗ;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  ସ୍ଥିତି DB_BUSY: {
    ଯଦି( objc>3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "CALLBACK");
      ଫେରନ୍ତୁ TCL_ERROR;
    }ଅନ୍ୟଥା ଯଦି( objc==2 ){
      ଯଦି( pDb->zBusy ){
        Tcl_AppendResult(interp, pDb->zBusy, (ଅକ୍ଷର*)0);
      }
    }ଅନ୍ୟଥା{
      ଅକ୍ଷର *zBusy;
      Tcl_Size len;
      ଯଦି( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      ଯଦି( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }ଅନ୍ୟଥା{
        pDb->zBusy = 0;
      }
      ଯଦି( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }ଅନ୍ୟଥା{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    ଭାଙ୍ଗ;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  ସ୍ଥିତି DB_CACHE: {
    ଅକ୍ଷର *subCmd;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

    ଯଦି( objc<=2 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "cache option ?arg?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    ଯଦି( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      ଯଦି( objc!=3 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "flush");
        ଫେରନ୍ତୁ TCL_ERROR;
      }ଅନ୍ୟଥା{
        flushStmtCache( pDb );
      }
    }ଅନ୍ୟଥା ଯଦି( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      ଯଦି( objc!=4 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "size n");
        ଫେରନ୍ତୁ TCL_ERROR;
      }ଅନ୍ୟଥା{
        ଯଦି( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_AppendResult( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (ଅକ୍ଷର*)0);
          ଫେରନ୍ତୁ TCL_ERROR;
        }ଅନ୍ୟଥା{
          ଯଦି( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }ଅନ୍ୟଥା ଯଦି( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }ଅନ୍ୟଥା{
      Tcl_AppendResult( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଭାଙ୍ଗ;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  ସ୍ଥିତି DB_CHANGES: {
    Tcl_Obj *pResult;
    ଯଦି( objc!=2 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_changes64(pDb->db));
    ଭାଙ୍ଗ;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  ସ୍ଥିତି DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    ଭାଙ୍ଗ;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  ସ୍ଥିତି DB_COLLATE: {
    SqlCollate *pCollate;
    ଅକ୍ଷର *zName;
    ଅକ୍ଷର *zScript;
    Tcl_Size nScript;
    ଯଦି( objc!=4 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "NAME SCRIPT");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( ଆକାର(*pCollate) + nScript + 1 );
    ଯଦି( pCollate==0 ) ଫେରନ୍ତୁ TCL_ERROR;
    pCollate->interp = interp;
    pCollate->pNext = pDb->pCollate;
    pCollate->zScript = (ଅକ୍ଷର*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    ଯଦି( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଭାଙ୍ଗ;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  ସ୍ଥିତି DB_COLLATION_NEEDED: {
    ଯଦି( objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "SCRIPT");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_ଗଣନା(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_ଗଣନା(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    ଭାଙ୍ଗ;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  ସ୍ଥିତି DB_COMMIT_HOOK: {
    ଯଦି( objc>3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?CALLBACK?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }ଅନ୍ୟଥା ଯଦି( objc==2 ){
      ଯଦି( pDb->zCommit ){
        Tcl_AppendResult(interp, pDb->zCommit, (ଅକ୍ଷର*)0);
      }
    }ଅନ୍ୟଥା{
      ଅଚଳ ଅକ୍ଷର *zCommit;
      Tcl_Size len;
      ଯଦି( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      ଯଦି( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }ଅନ୍ୟଥା{
        pDb->zCommit = 0;
      }
      ଯଦି( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }ଅନ୍ୟଥା{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    ଭାଙ୍ଗ;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  ସ୍ଥିତି DB_COMPLETE: {
#ifndef SQLITE_OMIT_COMPLETE
    Tcl_Obj *pResult;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isComplete;
    ଯଦି( objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "SQL");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetBooleanObj(pResult, isComplete);
#endif
    ଭାଙ୍ଗ;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  ସ୍ଥିତି DB_CONFIG: {
    ସ୍ଥିର ଅଚଳ ସଂରଚନା DbConfigChoices {
      ଅଚଳ ଅକ୍ଷର *zName;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
    ଯଦି( objc>4 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      pResult = Tcl_NewListObj(0,0);
      ପାଇଁ(ii=0; ii<ଆକାର(aDbConfig)/ଆକାର(aDbConfig[0]); ii++){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewIntObj(v));
      }
    }ଅନ୍ୟଥା{
      ଅଚଳ ଅକ୍ଷର *zOpt = Tcl_GetString(objv[2]);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା onoff = -1;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା v = 0;
      ଯଦି( zOpt[0]=='-' ) zOpt++;
      ପାଇଁ(ii=0; ii<ଆକାର(aDbConfig)/ଆକାର(aDbConfig[0]); ii++){
        ଯଦି( strcmp(aDbConfig[ii].zName, zOpt)==0 ) ଭାଙ୍ଗ;
      }
      ଯଦି( ii>=ଆକାର(aDbConfig)/ଆକାର(aDbConfig[0]) ){
        Tcl_AppendResult(interp, "unknown config option: \"", zOpt,
                                "\"", (ଶୂନ୍ୟ*)0);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( objc==4 ){
        ଯଦି( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      pResult = Tcl_NewIntObj(v);
    }
    Tcl_SetObjResult(interp, pResult);
    ଭାଙ୍ଗ;
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
  ସ୍ଥିତି DB_COPY: {
    ଅକ୍ଷର *zTable;               /* Insert data into this table */
    ଅକ୍ଷର *zFile;                /* The file from which to extract data */
    ଅକ୍ଷର *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;                   /* Number of columns in the table */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;                  /* Number of bytes in an SQL string */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;                   /* Loop counters */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSep;                   /* Number of bytes in zSep[] */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nNull;                  /* Number of bytes in zNull[] */
    ଅକ୍ଷର *zSql;                 /* An SQL statement */
    ଅକ୍ଷର *z_ଧାଡ଼ି;                /* A single line of input from the file */
    ଅକ୍ଷର **azCol;               /* zLine[] broken up into columns */
    ଅଚଳ ଅକ୍ଷର *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno = 0;             /* Line number of input file */
    ଅକ୍ଷର z_ଧାଡ଼ି_ଅଙ୍କ[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *pResult;           /* interp result */

    ଅଚଳ ଅକ୍ଷର *zSep;
    ଅଚଳ ଅକ୍ଷର *zNull;
    ଯଦି( objc<5 || objc>7 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }ଅନ୍ୟଥା{
      zSep = "\t";
    }
    ଯଦି( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }ଅନ୍ୟଥା{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    ଯଦି( nSep==0 ){
      Tcl_AppendResult(interp,"Error: non-null separator required for copy",
                       (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_AppendResult(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    ଯଦି( zSql==0 ){
      Tcl_AppendResult(interp, "Error: no such table: ", zTable, (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    ଯଦି( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
      nCol = 0;
    }ଅନ୍ୟଥା{
      nCol = sqlite3_column_ଗଣନା(pStmt);
    }
    sqlite3_finalize(pStmt);
    ଯଦି( nCol==0 ) {
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    ଯଦି( zSql==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    ପାଇଁ(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    ମୁକ୍ତ(zSql);
    ଯଦି( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
      sqlite3_finalize(pStmt);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    ଯଦି( in==0 ){
      sqlite3_finalize(pStmt);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    Tcl_SetChannelOption(ଖାଲି, in, "-translation", "auto");
    azCol = malloc( ଆକାର(azCol[0])*(nCol+1) );
    ଯଦି( azCol==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (ଅକ୍ଷର*)0);
      Tcl_Close(interp, in);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_ଗଣନା(str);
    (ଶୂନ୍ୟ)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    ଯେତେବେଳେ( Tcl_GetsObj(in, str)>=0 ) {
      ଅକ୍ଷର *z;
      Tcl_Size byteLen;
      lineno++;
      z_ଧାଡ଼ି = (ଅକ୍ଷର *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = z_ଧାଡ଼ି;
      ପାଇଁ(i=0, z=z_ଧାଡ଼ି; *z; z++){
        ଯଦି( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          ଯଦି( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      ଯଦି( i+1!=nCol ){
        ଅକ୍ଷର *zErr;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        ଯଦି( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_AppendResult(interp, zErr, (ଅକ୍ଷର*)0);
          ମୁକ୍ତ(zErr);
        }
        zCommit = "ROLLBACK";
        ଭାଙ୍ଗ;
      }
      ପାଇଁ(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        ଯଦି( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }ଅନ୍ୟଥା{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_SetObjLength(str, 0);
      ଯଦି( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp,"Error: ", sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
        zCommit = "ROLLBACK";
        ଭାଙ୍ଗ;
      }
    }
    Tcl_Decr_Ref_ଗଣନା(str);
    ମୁକ୍ତ(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (ଶୂନ୍ୟ)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    ଯଦି( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      pResult = Tcl_GetObjResult(interp);
      ଯଦି( Tcl_IsShared(pResult) ){
        pResult = Tcl_DuplicateObj(pResult);
      }
      Tcl_SetIntObj(pResult, lineno);
      rc = TCL_OK;
    }ଅନ୍ୟଥା{
      /* failure, append lineno where failed */
      sqlite3_snprintf(ଆକାର(z_ଧାଡ଼ି_ଅଙ୍କ), z_ଧାଡ଼ି_ଅଙ୍କ,"%d",lineno);
      Tcl_AppendResult(interp,", failed while processing line: ",z_ଧାଡ଼ି_ଅଙ୍କ,
                       (ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
    }
    ଭାଙ୍ଗ;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  ସ୍ଥିତି DB_DESERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (ଅକ୍ଷର*)0);
    rc = TCL_ERROR;
#ଅନ୍ୟଥା
    ଅଚଳ ଅକ୍ଷର *zSchema = 0;
    Tcl_Obj *p_ମୂଲ୍ୟ = 0;
    unsigned ଅକ୍ଷର *pBA;
    unsigned ଅକ୍ଷର *pData;
    Tcl_Size len;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା xrc;
    sqlite3_int64 mxSize = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isReadonly = 0;


    ଯଦି( objc<3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      ଭାଙ୍ଗ;
    }
    ପାଇଁ(i=2; i<objc-1; i++){
      ଅଚଳ ଅକ୍ଷର *z = Tcl_GetString(objv[i]);
      ଯଦି( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        ଯଦି( rc ) ଯାଅ deserialize_error;
        mxSize = x;
        ଜାରି;
      }
      ଯଦି( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        ଯଦି( rc ) ଯାଅ deserialize_error;
        ଜାରି;
      }
      ଯଦି( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        ଜାରି;
      }
      Tcl_AppendResult(interp, "unknown option: ", z, (ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
      ଯାଅ deserialize_error;
    }
    p_ମୂଲ୍ୟ = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_ମୂଲ୍ୟ, &len);
    pData = sqlite3_malloc64( len );
    ଯଦି( pData==0 && len>0 ){
      Tcl_AppendResult(interp, "out of memory", (ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
      ଯଦି( len>0 ) memcpy(pData, pBA, len);
      ଯଦି( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }ଅନ୍ୟଥା{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, pData, len, len, flags);
      ଯଦି( xrc ){
        Tcl_AppendResult(interp, "unable to set MEMDB content", (ଅକ୍ଷର*)0);
        rc = TCL_ERROR;
      }
      ଯଦି( mxSize>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_SIZE_LIMIT,&mxSize);
      }
    }
deserialize_error:
#endif
    ଭାଙ୍ଗ; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  ସ୍ଥିତି DB_ENABLE_LOAD_EXTENSION: {
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା onoff;
    ଯଦି( objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "BOOLEAN");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    ଭାଙ୍ଗ;
#ଅନ୍ୟଥା
    Tcl_AppendResult(interp, "extension loading is turned off at compile-time",
                     (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
#endif
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  ସ୍ଥିତି DB_ERRORCODE: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    ଭାଙ୍ଗ;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  ସ୍ଥିତି DB_ERROROFFSET: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    ଭାଙ୍ଗ;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  ସ୍ଥିତି DB_EXISTS:
  ସ୍ଥିତି DB_ONECOLUMN: {
    Tcl_Obj *pResult = 0;
    DbEvalContext sEval;
    ଯଦି( objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "SQL");
      ଫେରନ୍ତୁ TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    ଯଦି( choice==DB_ONECOLUMN ){
      ଯଦି( rc==TCL_OK ){
        pResult = db_Eval_Column_ମୂଲ୍ୟ(&sEval, 0);
      }ଅନ୍ୟଥା ଯଦି( rc==TCL_BREAK ){
        Tcl_ResetResult(interp);
      }
    }ଅନ୍ୟଥା ଯଦି( rc==TCL_BREAK || rc==TCL_OK ){
      pResult = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    ଯଦି( pResult ) Tcl_SetObjResult(interp, pResult);

    ଯଦି( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    ଭାଙ୍ଗ;
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
  ସ୍ଥିତି DB_EVAL: {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା evalFlags = 0;
    ଅଚଳ ଅକ୍ଷର *zOpt;
    ଯେତେବେଳେ( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      ଯଦି( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }ଅନ୍ୟଥା ଯଦି( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }ଅନ୍ୟଥା{
        Tcl_AppendResult(interp, "unknown option: \"", zOpt, "\"", (ଶୂନ୍ୟ*)0);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      objc--;
      objv++;
    }
    ଯଦି( objc<3 || objc>5 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }

    ଯଦି( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_ଗଣନା(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      ଯେତେବେଳେ( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        ପାଇଁ(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_Column_ମୂଲ୍ୟ(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      ଯଦି( rc==TCL_BREAK ){
        Tcl_SetObjResult(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_ଗଣନା(pRet);
    }ଅନ୍ୟଥା{
      ClientData cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      ଯଦି( objc>=5 && *(ଅକ୍ଷର *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_ଗଣନା(pScript);

      p = (DbEvalContext *)Tcl_Alloc(ଆକାର(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (ଶୂନ୍ୟ *)p;
      cd2[1] = (ଶୂନ୍ୟ *)pScript;
      rc = DbEvalNextCmd(cd2, interp, TCL_OK);
    }
    ଭାଙ୍ଗ;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  ସ୍ଥିତି DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    ଭାଙ୍ଗ;
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
  ସ୍ଥିତି DB_FUNCTION: {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    ଅକ୍ଷର *zName;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg = -1;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType = SQLITE_NULL;
    ଯଦି( objc<4 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ପାଇଁ(i=3; i<(objc-1); i++){
      ଅଚଳ ଅକ୍ଷର *z = Tcl_GetString(objv[i]);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = strlen30(z);
      ଯଦି( n>1 && strncmp(z, "-argcount",n)==0 ){
        ଯଦି( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(ଅକ୍ଷର*)0);
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        ଯଦି( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) ଫେରନ୍ତୁ TCL_ERROR;
        ଯଦି( nArg<0 ){
          Tcl_AppendResult(interp, "number of arguments must be non-negative",
                           (ଅକ୍ଷର*)0);
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        i++;
      }ଅନ୍ୟଥା
      ଯଦି( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }ଅନ୍ୟଥା
      ଯଦି( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }ଅନ୍ୟଥା
      ଯଦି( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }ଅନ୍ୟଥା
      ଯଦି( n>1 && strncmp(z, "-returntype", n)==0 ){
        ଅଚଳ ଅକ୍ଷର *azType[] = {"integer", "real", "text", "blob", "any", 0};
        ପ୍ରତିଜ୍ଞା( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        ପ୍ରତିଜ୍ଞା( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        ଯଦି( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(ଅକ୍ଷର*)0);
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        i++;
        ଯଦି( Tcl_GetIndexFromObj(interp, objv[i], azType, "type", 0, &eType) ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        eType++;
      }ଅନ୍ୟଥା{
        Tcl_AppendResult(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (ଅକ୍ଷର*)0
        );
        ଫେରନ୍ତୁ TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    ଯଦି( pFunc==0 ) ଫେରନ୍ତୁ TCL_ERROR;
    ଯଦି( pFunc->pScript ){
      Tcl_Decr_Ref_ଗଣନା(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_ଗଣନା(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    ଯଦି( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    ଭାଙ୍ଗ;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  ସ୍ଥିତି DB_INCRBLOB: {
#ifdef SQLITE_OMIT_INCRBLOB
    Tcl_AppendResult(interp, "incrblob not available in this build", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
#ଅନ୍ୟଥା
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isReadonly = 0;
    ଅଚଳ ଅକ୍ଷର *zDb = "main";
    ଅଚଳ ଅକ୍ଷର *zTable;
    ଅଚଳ ଅକ୍ଷର *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    ଯଦି( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    ଯଦି( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      ଫେରନ୍ତୁ TCL_ERROR;
    }

    ଯଦି( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    ଯଦି( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#endif
    ଭାଙ୍ଗ;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  ସ୍ଥିତି DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    ଭାଙ୍ଗ;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  ସ୍ଥିତି DB_NULLVALUE: {
    ଯଦି( objc!=2 && objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "NULLVALUE");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( objc==3 ){
      Tcl_Size len;
      ଅକ୍ଷର *zNull = Tcl_GetStringFromObj(objv[2], &len);
      ଯଦି( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      ଯଦି( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }ଅନ୍ୟଥା{
        pDb->zNull = 0;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(pDb->zNull, -1));
    ଭାଙ୍ଗ;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  ସ୍ଥିତି DB_LAST_ଭର୍ତ୍ତି_ROWID: {
    Tcl_Obj *pResult;
    Tcl_WideInt rowid;
    ଯଦି( objc!=2 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    rowid = sqlite3_last_ଭର୍ତ୍ତି_rowid(pDb->db);
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, rowid);
    ଭାଙ୍ଗ;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  ସ୍ଥିତି DB_PROGRESS: {
    ଯଦି( objc==2 ){
      ଯଦି( pDb->zProgress ){
        Tcl_AppendResult(interp, pDb->zProgress, (ଅକ୍ଷର*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }ଅନ୍ୟଥା ଯଦି( objc==4 ){
      ଅକ୍ଷର *zProgress;
      Tcl_Size len;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା N;
      ଯଦି( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      };
      ଯଦି( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      ଯଦି( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }ଅନ୍ୟଥା{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      ଯଦି( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }ଅନ୍ୟଥା{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }ଅନ୍ୟଥା{
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "N CALLBACK");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଭାଙ୍ଗ;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  ସ୍ଥିତି DB_PROFILE: {
    ଯଦି( objc>3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?CALLBACK?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }ଅନ୍ୟଥା ଯଦି( objc==2 ){
      ଯଦି( pDb->zProfile ){
        Tcl_AppendResult(interp, pDb->zProfile, (ଅକ୍ଷର*)0);
      }
    }ଅନ୍ୟଥା{
      ଅକ୍ଷର *zProfile;
      Tcl_Size len;
      ଯଦି( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      ଯଦି( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }ଅନ୍ୟଥା{
        pDb->zProfile = 0;
      }
#ଯଦି !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ଯଦି( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }ଅନ୍ୟଥା{
        sqlite3_profile(pDb->db, 0, 0);
      }
#endif
    }
    ଭାଙ୍ଗ;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  ସ୍ଥିତି DB_REKEY: {
    ଯଦି( objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "KEY");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଭାଙ୍ଗ;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  ସ୍ଥିତି DB_RESTORE: {
    ଅଚଳ ଅକ୍ଷର *zSrcFile;
    ଅଚଳ ଅକ୍ଷର *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTimeout = 0;

    ଯଦି( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }ଅନ୍ୟଥା ଯଦି( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }ଅନ୍ୟଥା{
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?DATABASE? FILENAME");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    ଯଦି( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (ଅକ୍ଷର*)0);
      sqlite3_close(pSrc);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    ଯଦି( pBackup==0 ){
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
      sqlite3_close(pSrc);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯେତେବେଳେ( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      ଯଦି( rc==SQLITE_BUSY ){
        ଯଦି( nTimeout++ >= 3 ) ଭାଙ୍ଗ;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    ଯଦି( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }ଅନ୍ୟଥା ଯଦି( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_AppendResult(interp, "restore failed: source database busy",
                       (ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
    }ଅନ୍ୟଥା{
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    ଭାଙ୍ଗ;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  ସ୍ଥିତି DB_SERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (ଅକ୍ଷର*)0);
    rc = TCL_ERROR;
#ଅନ୍ୟଥା
    ଅଚଳ ଅକ୍ଷର *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned ଅକ୍ଷର *pData;
    ଯଦି( objc!=2 && objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା needFree;
      pData = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      ଯଦି( pData ){
        needFree = 0;
      }ଅନ୍ୟଥା{
        pData = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(pData,sz));
      ଯଦି( needFree ) sqlite3_free(pData);
    }
#endif
    ଭାଙ୍ଗ;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  ସ୍ଥିତି DB_STATUS: {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା v;
    ଅଚଳ ଅକ୍ଷର *zOp;
    ଯଦି( objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "(step|sort|autoindex)");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    ଯଦି( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zOp, "autoindex")==0 ){
      v = pDb->nIndex;
    }ଅନ୍ୟଥା ଯଦି( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }ଅନ୍ୟଥା{
      Tcl_AppendResult(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(v));
    ଭାଙ୍ଗ;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  ସ୍ଥିତି DB_TIMEOUT: {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ms;
    ଯଦି( objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "MILLISECONDS");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &ms) ) ଫେରନ୍ତୁ TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    ଭାଙ୍ଗ;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  ସ୍ଥିତି DB_TOTAL_CHANGES: {
    Tcl_Obj *pResult;
    ଯଦି( objc!=2 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_total_changes64(pDb->db));
    ଭାଙ୍ଗ;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  ସ୍ଥିତି DB_TRACE: {
    ଯଦି( objc>3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?CALLBACK?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }ଅନ୍ୟଥା ଯଦି( objc==2 ){
      ଯଦି( pDb->zTrace ){
        Tcl_AppendResult(interp, pDb->zTrace, (ଅକ୍ଷର*)0);
      }
    }ଅନ୍ୟଥା{
      ଅକ୍ଷର *zTrace;
      Tcl_Size len;
      ଯଦି( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      ଯଦି( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }ଅନ୍ୟଥା{
        pDb->zTrace = 0;
      }
#ଯଦି !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ଯଦି( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }ଅନ୍ୟଥା{
        sqlite3_trace(pDb->db, 0, 0);
      }
#endif
    }
    ଭାଙ୍ଗ;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  ସ୍ଥିତି DB_TRACE_V2: {
    ଯଦି( objc>4 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?CALLBACK? ?MASK?");
      ଫେରନ୍ତୁ TCL_ERROR;
    }ଅନ୍ୟଥା ଯଦି( objc==2 ){
      ଯଦି( pDb->zTraceV2 ){
        Tcl_AppendResult(interp, pDb->zTraceV2, (ଅକ୍ଷର*)0);
      }
    }ଅନ୍ୟଥା{
      ଅକ୍ଷର *zTraceV2;
      Tcl_Size len;
      Tcl_WideInt wMask = 0;
      ଯଦି( objc==4 ){
        ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        ଗଣନା TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_Size i;
        ଯଦି( TCL_OK!=Tcl_ListObjLength(interp, objv[3], &len) ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        ପାଇଁ(i=0; i<len; i++){
          Tcl_Obj *pObj;
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା ttype;
          ଯଦି( TCL_OK!=Tcl_ListObjIndex(interp, objv[3], i, &pObj) ){
            ଫେରନ୍ତୁ TCL_ERROR;
          }
          ଯଦି( Tcl_GetIndexFromObj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
            Tcl_Incr_Ref_ଗଣନା(pError);
            ଯଦି( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_ଗଣନା(pError);
              wMask |= wType;
            }ଅନ୍ୟଥା{
              Tcl_SetObjResult(interp, pError);
              Tcl_Decr_Ref_ଗଣନା(pError);
              ଫେରନ୍ତୁ TCL_ERROR;
            }
          }ଅନ୍ୟଥା{
            ବାଛ( (ଗଣନା TTYPE_enum)ttype ){
              ସ୍ଥିତି TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    ଭାଙ୍ଗ;
              ସ୍ଥିତି TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; ଭାଙ୍ଗ;
              ସ୍ଥିତି TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     ଭାଙ୍ଗ;
              ସ୍ଥିତି TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   ଭାଙ୍ଗ;
            }
          }
        }
      }ଅନ୍ୟଥା{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      ଯଦି( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      ଯଦି( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }ଅନ୍ୟଥା{
        pDb->zTraceV2 = 0;
      }
#ଯଦି !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      ଯଦି( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }ଅନ୍ୟଥା{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#endif
    }
    ଭାଙ୍ଗ;
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
  ସ୍ଥିତି DB_TRANSACTION: {
    Tcl_Obj *pScript;
    ଅଚଳ ଅକ୍ଷର *zBegin = "SAVEPOINT _tcl_transaction";
    ଯଦି( objc!=3 && objc!=4 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "[TYPE] SCRIPT");
      ଫେରନ୍ତୁ TCL_ERROR;
    }

    ଯଦି( pDb->nTransaction==0 && objc==4 ){
      ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      ଗଣନା TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ttype;
      ଯଦି( Tcl_GetIndexFromObj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ବାଛ( (ଗଣନା TTYPE_enum)ttype ){
        ସ୍ଥିତି TTYPE_DEFERRED:    /* no-op */;                 ଭାଙ୍ଗ;
        ସ୍ଥିତି TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  ଭାଙ୍ଗ;
        ସ୍ଥିତି TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  ଭାଙ୍ଗ;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    ଯଦି( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    ଯଦି( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (ଶୂନ୍ୟ)Tcl_NREvalObj(interp, pScript, 0);
    }ଅନ୍ୟଥା{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    ଭାଙ୍ଗ;
  }

  /*
  **    $db unlock_notify ?script?
  */
  ସ୍ଥିତି DB_UNLOCK_NOTIFY: {
#ifndef SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_AppendResult(interp, "unlock_notify not available in this build",
                     (ଅକ୍ଷର*)0);
    rc = TCL_ERROR;
#ଅନ୍ୟଥା
    ଯଦି( objc!=2 && objc!=3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }ଅନ୍ୟଥା{
      ଶୂନ୍ୟ (*xNotify)(ଶୂନ୍ୟ **, ପୂର୍ଣ୍ଣସଂଖ୍ୟା) = 0;
      ଶୂନ୍ୟ *pNotifyArg = 0;

      ଯଦି( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_ଗଣନା(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      ଯଦି( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (ଶୂନ୍ୟ *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_ଗଣନା(pDb->pUnlockNotify);
      }

      ଯଦି( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
        rc = TCL_ERROR;
      }
    }
#endif
    ଭାଙ୍ଗ;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  ସ୍ଥିତି DB_PREUPDATE: {
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_AppendResult(interp, "preupdate_hook was omitted at compile-time", 
                     (ଅକ୍ଷର*)0);
    rc = TCL_ERROR;
#ଅନ୍ୟଥା
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    ଗଣନା DbPreupdateSubCmd {
      PRE_ଗଣନା, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSub;

    ଯଦି( objc<3 ){
      Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    ଯଦି( Tcl_GetIndexFromObj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      ଫେରନ୍ତୁ TCL_ERROR;
    }

    ବାଛ( (ଗଣନା DbPreupdateSubCmd)iSub ){
      ସ୍ଥିତି PRE_ଗଣନା: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol = sqlite3_preupdate_ଗଣନା(pDb->db);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nCol));
        ଭାଙ୍ଗ;
      }

      ସ୍ଥିତି PRE_HOOK: {
        ଯଦି( objc>4 ){
          Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "hook ?SCRIPT?");
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        ଭାଙ୍ଗ;
      }

      ସ୍ଥିତି PRE_DEPTH: {
        Tcl_Obj *pRet;
        ଯଦି( objc!=3 ){
          Tcl_Wrong_ଅଙ୍କ_Args(interp, 3, objv, "");
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_SetObjResult(interp, pRet);
        ଭାଙ୍ଗ;
      }

      ସ୍ଥିତି PRE_NEW:
      ସ୍ଥିତି PRE_OLD: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIdx;
        sqlite3_ମୂଲ୍ୟ *p_ମୂଲ୍ୟ;
        ଯଦି( objc!=4 ){
          Tcl_Wrong_ଅଙ୍କ_Args(interp, 3, objv, "INDEX");
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        ଯଦି( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }

        ଯଦି( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_ମୂଲ୍ୟ);
        }ଅନ୍ୟଥା{
          ପ୍ରତିଜ୍ଞା( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_ମୂଲ୍ୟ);
        }

        ଯଦି( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(p_ମୂଲ୍ୟ), -1);
          Tcl_SetObjResult(interp, pObj);
        }ଅନ୍ୟଥା{
          Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (ଅକ୍ଷର*)0);
          ଫେରନ୍ତୁ TCL_ERROR;
        }
      }
    }
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
    ଭାଙ୍ଗ;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  ସ୍ଥିତି DB_WAL_HOOK:
  ସ୍ଥିତି DB_UPDATE_HOOK:
  ସ୍ଥିତି DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    ଯଦି( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    ଯଦି( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    ଯଦି( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    ଯଦି( objc>3 ){
       Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?SCRIPT?");
       ଫେରନ୍ତୁ TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    ଭାଙ୍ଗ;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  ସ୍ଥିତି DB_VERSION: {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=2; i<objc; i++){
      ଅଚଳ ଅକ୍ଷର *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      ଯଦି( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }
      }ଅନ୍ୟଥା

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      ଯଦି( strcmp(zArg, "-last-stmt-ptr")==0 ){
        ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3TestPtrToText(ଶୂନ୍ୟ*);
        ଶୂନ୍ୟ *ptr = (ଶୂନ୍ୟ*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_SetResult(interp, (ଅକ୍ଷର*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }ଅନ୍ୟଥା
#endif /* SQLITE_TEST */
      {
        Tcl_AppendResult(interp, "unknown argument: ", zArg, (ଅକ୍ଷର*)0);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
    }
    ଯଦି( i==2 ){   
      Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3_libversion(), TCL_STATIC);
    }
    ଭାଙ୍ଗ;
  }


  } /* End of the SWITCH statement */
  ଫେରନ୍ତୁ rc;
}

#ଯଦି SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI DbObjCmdAdaptor(
  ଶୂନ୍ୟ *cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *ଅଚଳ*objv
){
  ଫେରନ୍ତୁ Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *ଅଚଳ*objv
){
  Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  ଫେରନ୍ତୁ TCL_ERROR;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI DbMain(
  ଶୂନ୍ୟ *cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *ଅଚଳ*objv
){
  SqliteDb *p;
  ଅଚଳ ଅକ୍ଷର *zArg;
  ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅଚଳ ଅକ୍ଷର *zFile = 0;
  ଅଚଳ ଅକ୍ଷର *zVfs = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#ଅନ୍ୟଥା
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  ଯଦି( objc==1 ) ଫେରନ୍ତୁ sqliteCmdUsage(interp, objv);
  ଯଦି( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    ଯଦି( strcmp(zArg,"-version")==0 ){
      Tcl_AppendResult(interp,sqlite3_libversion(), (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_OK;
    }
    ଯଦି( strcmp(zArg,"-sourceid")==0 ){
      Tcl_AppendResult(interp,sqlite3_sourceid(), (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_OK;
    }
    ଯଦି( strcmp(zArg,"-has-codec")==0 ){
      Tcl_AppendResult(interp,"0",(ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_OK;
    }
    ଯଦି( zArg[0]=='-' ) ଫେରନ୍ତୁ sqliteCmdUsage(interp, objv);
  }
  ପାଇଁ(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    ଯଦି( zArg[0]!='-' ){
      ଯଦି( zFile!=0 ) ଫେରନ୍ତୁ sqliteCmdUsage(interp, objv);
      zFile = zArg;
      ଜାରି;
    }
    ଯଦି( i==objc-1 ) ଫେରନ୍ତୁ sqliteCmdUsage(interp, objv);
    i++;
    ଯଦି( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg, "-readonly")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ଫେରନ୍ତୁ TCL_ERROR;
      ଯଦି( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }ଅନ୍ୟଥା{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg, "-create")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ଫେରନ୍ତୁ TCL_ERROR;
      ଯଦି( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }ଅନ୍ୟଥା{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg, "-nofollow")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ଫେରନ୍ତୁ TCL_ERROR;
      ଯଦି( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }ଅନ୍ୟଥା{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg, "-nomutex")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ଫେରନ୍ତୁ TCL_ERROR;
      ଯଦି( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }ଅନ୍ୟଥା{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg, "-fullmutex")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ଫେରନ୍ତୁ TCL_ERROR;
      ଯଦି( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }ଅନ୍ୟଥା{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg, "-uri")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ଫେରନ୍ତୁ TCL_ERROR;
      ଯଦି( b ){
        flags |= SQLITE_OPEN_URI;
      }ଅନ୍ୟଥା{
        flags &= ~SQLITE_OPEN_URI;
      }
    }ଅନ୍ୟଥା ଯଦି( strcmp(zArg, "-translatefilename")==0 ){
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
    }ଅନ୍ୟଥା{
      Tcl_AppendResult(interp, "unknown option: ", zArg, (ଅକ୍ଷର*)0);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
  }
  z_Err_ସନ୍ଦେଶ = 0;
  p = (SqliteDb*)Tcl_Alloc( ଆକାର(*p) );
  memset(p, 0, ଆକାର(*p));
  ଯଦି( zFile==0 ) zFile = "";
  ଯଦି( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  ଯଦି( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  ଯଦି( p->db ){
    ଯଦି( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }ଅନ୍ୟଥା{
    z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  ଯଦି( p->db==0 ){
    Tcl_SetResult(interp, z_Err_ସନ୍ଦେଶ, TCL_VOLATILE);
    Tcl_Free((ଅକ୍ଷର*)p);
    sqlite3_free(z_Err_ସନ୍ଦେଶ);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  p->maxStmt = ଅଙ୍କ_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  ଯଦି( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (ଅକ୍ଷର*)p, DbDeleteCmd);
  }ଅନ୍ୟଥା{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (ଅକ୍ଷର*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  ଫେରନ୍ତୁ TCL_OK;
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
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite3_Init(Tcl_Interp *interp){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  ଯଦି( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  ଫେରନ୍ତୁ rc;
}
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Tclsqlite3_Init(Tcl_Interp *interp){ ଫେରନ୍ତୁ Sqlite3_Init(interp); }
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite3_Unload(Tcl_Interp *interp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){ ଫେରନ୍ତୁ TCL_OK; }
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Tclsqlite3_Unload(Tcl_Interp *interp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){ ଫେରନ୍ତୁ TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite3_SafeInit(Tcl_Interp *interp){ ଫେରନ୍ତୁ TCL_ERROR; }
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite3_SafeUnload(Tcl_Interp *interp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){ଫେରନ୍ତୁ TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite_Init(Tcl_Interp *interp){ ଫେରନ୍ତୁ Sqlite3_Init(interp);}
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Tclsqlite_Init(Tcl_Interp *interp){ ଫେରନ୍ତୁ Sqlite3_Init(interp); }
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite_Unload(Tcl_Interp *interp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){ ଫେରନ୍ତୁ TCL_OK; }
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Tclsqlite_Unload(Tcl_Interp *interp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){ ଫେରନ୍ତୁ TCL_OK; }
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite_SafeInit(Tcl_Interp *interp){ ଫେରନ୍ତୁ TCL_ERROR; }
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlite_SafeUnload(Tcl_Interp *interp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){ଫେରନ୍ତୁ TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Init(Tcl_Interp *interp){ ଫେରନ୍ତୁ Sqlite3_Init(interp);}
EXTERN ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite_Init(Tcl_Interp *interp){ ଫେରନ୍ତୁ Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#ଯଦି defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *tclsh_main_loop(ଶୂନ୍ୟ){
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର zMainloop[] =
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
  ଫେରନ୍ତୁ zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN ମୁଖ୍ୟ
#endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_CDECL TCLSH_MAIN(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv){
  Tcl_Interp *interp;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଅଚଳ ଅକ୍ଷର *zScript = 0;
  ଅକ୍ଷର zArgc[32];
#ଯଦି defined(TCLSH_INIT_PROC)
  ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#ଯଦି !defined(_WIN32_WCE)
  ଯଦି( getenv("SQLITE_DEBUG_BREAK") ){
    ଯଦି( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }ଅନ୍ୟଥା{
#ଯଦି defined(_WIN32) || defined(WIN32)
      DebugBreak();
#ନଚେତଯଦି defined(SIGTRAP)
      ଉଠାଅ(SIGTRAP);
#endif
    }
  }
#endif

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_SetSystemEncoding(ଖାଲି, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(ଆକାର(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  ପାଇଁ(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_ମୂଲ୍ୟ);
  }
#ଯଦି defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  ଯଦି( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  ଯଦି( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    ଅଚଳ ଅକ୍ଷର *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    ଯଦି( zInfo==0 ) zInfo = Tcl_GetStringResult(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}
#endif /* TCLSH */
