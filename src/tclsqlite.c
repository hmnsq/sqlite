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
#જો defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#જો defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#નહીંતર
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#જો TCL_MAJOR_VERSION==9
# define CONST અચલ
#અથવાજો !defined(Tcl_Size)
  પ્રકારવ્યાખ્યા પૂર્ણાંક Tcl_Size;
# ifndef Tcl_Bounce_Ref_ગણતરી
#  define Tcl_Bounce_Ref_ગણતરી(X) Tcl_Incr_Ref_ગણતરી(X); Tcl_Decr_Ref_ગણતરી(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#endif
/**** End copy of tclsqlite.h ****/

#અન્તર્ભાવ <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ifndef SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  પ્રકારવ્યાખ્યા unsigned અક્ષર u8;
# ifndef SQLITE_PTRSIZE
#   જો defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   નહીંતર
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# જો defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    પ્રકારવ્યાખ્યા uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    પ્રકારવ્યાખ્યા unsigned પૂર્ણાંક uptr;
# નહીંતર
    પ્રકારવ્યાખ્યા sqlite3_uint64 uptr;
# endif
#endif
#અન્તર્ભાવ <ctype.h>

/* Used to get the current process ID */
#જો !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#અથવાજો !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (પૂર્ણાંક)GetCurrentProcessId
#endif

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ifdef BUILD_sqlite
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_sqlite */

#define અંક_PREPARED_STMTS 10
#define MAX_PREPARED_STMTS 100

/* Forward declaration */
પ્રકારવ્યાખ્યા સંરચના SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#જો defined(SQLITE_ENABLE_QRF_IN_TCL)
#અન્તર્ભાવ "qrf.h"
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
પ્રકારવ્યાખ્યા સંરચના SqlFunc SqlFunc;
સંરચના SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  પૂર્ણાંક useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  પૂર્ણાંક eType;            /* Type of value to return */
  અક્ષર *zName;          /* Name of this function */
  SqlFunc *pNext;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
પ્રકારવ્યાખ્યા સંરચના SqlCollate SqlCollate;
સંરચના SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  અક્ષર *zScript;        /* The script to be run */
  SqlCollate *pNext;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
પ્રકારવ્યાખ્યા સંરચના SqlPreparedStmt SqlPreparedStmt;
સંરચના SqlPreparedStmt {
  SqlPreparedStmt *pNext;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  પૂર્ણાંક nSql;                /* chars in zSql[] */
  અચલ અક્ષર *zSql;        /* Text of the SQL statement */
  પૂર્ણાંક nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

પ્રકારવ્યાખ્યા સંરચના IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
સંરચના SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  અક્ષર *zBusy;               /* The busy callback routine */
  અક્ષર *zCommit;             /* The commit hook callback routine */
  અક્ષર *zTrace;              /* The trace callback routine */
  અક્ષર *zTraceV2;            /* The trace_v2 callback routine */
  અક્ષર *zProfile;            /* The profile callback routine */
  અક્ષર *zProgress;           /* The progress callback routine */
  અક્ષર *zBindFallback;       /* Callback to invoke on a binding miss */
  અક્ષર *zAuth;               /* The authorization callback routine */
  પૂર્ણાંક disableAuth;           /* Disable the authorizer if it exists */
  અક્ષર *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  પૂર્ણાંક rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  પૂર્ણાંક maxStmt;               /* The next maximum number of stmtList */
  પૂર્ણાંક nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  પૂર્ણાંક nStep, nSort, nIndex;  /* Statistics for most recent operation */
  પૂર્ણાંક nVMStep;               /* Another statistic for most recent operation */
  પૂર્ણાંક nTransaction;          /* Number of nested [transaction] methods */
  પૂર્ણાંક openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  પૂર્ણાંક nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  પૂર્ણાંક bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

સંરચના IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned પૂર્ણાંક isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *pNext;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
સ્થિર પૂર્ણાંક strlen30(અચલ અક્ષર *z){
  અચલ અક્ષર *z2 = z;
  જ્યારે( *z2 ){ z2++; }
  પાછા 0x3fffffff & (પૂર્ણાંક)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
સ્થિર શૂન્ય closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *pNext;

  માટે(p=pDb->pIncrblob; p; p=pNext){
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI incrblobClose2(
  ClientData instanceData,
  Tcl_Interp *interp,
  પૂર્ણાંક flags
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  પૂર્ણાંક  rc;
  sqlite3 *db = p->pDb->db;

  જો( flags ){
    p->isClosed |= flags;
    પાછા TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  જો( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  જો( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }
  જો( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->pNext;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((અક્ષર *)p);

  જો( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (અક્ષર *)sqlite3_errmsg(db), TCL_VOLATILE);
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}
સ્થિર પૂર્ણાંક SQLITE_TCLAPI incrblobClose(
  ClientData instanceData,
  Tcl_Interp *interp
){
  પાછા incrblobClose2(instanceData, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI incrblob_આગત(
  ClientData instanceData,
  અક્ષર *buf,
  પૂર્ણાંક bufSize,
  પૂર્ણાંક *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nRead = bufSize;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  પૂર્ણાંક rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  જો( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  જો( nRead<=0 ){
    પાછા 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (શૂન્ય *)buf, (પૂર્ણાંક)nRead, (પૂર્ણાંક)p->iSeek);
  જો( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    પાછા -1;
  }

  p->iSeek += nRead;
  પાછા nRead;
}

/*
** Write data to an incremental blob channel.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI incrblobOutput(
  ClientData instanceData,
  અચલ અક્ષર *buf,
  પૂર્ણાંક toWrite,
  પૂર્ણાંક *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  પૂર્ણાંક rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  જો( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    પાછા -1;
  }
  જો( nWrite<=0 ){
    પાછા 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (શૂન્ય*)buf,(પૂર્ણાંક)nWrite, (પૂર્ણાંક)p->iSeek);
  જો( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    પાછા -1;
  }

  p->iSeek += nWrite;
  પાછા nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#જો TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#નહીંતર
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
સ્થિર WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  ClientData instanceData,
  WideSeekProcType offset,
  પૂર્ણાંક seekMode,
  પૂર્ણાંક *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;

  પસંદગી( seekMode ){
    સ્થિતિ SEEK_SET:
      p->iSeek = offset;
      તોડો;
    સ્થિતિ SEEK_CUR:
      p->iSeek += offset;
      તોડો;
    સ્થિતિ SEEK_END:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      તોડો;

    મૂળભૂત: પ્રતિજ્ઞા(!"Bad seekMode");
  }

  પાછા p->iSeek;
}
સ્થિર પૂર્ણાંક SQLITE_TCLAPI incrblobSeek(
  ClientData instanceData,
  long offset,
  પૂર્ણાંક seekMode,
  પૂર્ણાંક *errorCodePtr
){
  પાછા incrblobWideSeek(instanceData,offset,seekMode,errorCodePtr);
}


સ્થિર શૂન્ય SQLITE_TCLAPI incrblobWatch(
  ClientData instanceData,
  પૂર્ણાંક mode
){
  /* NO-OP */
}
સ્થિર પૂર્ણાંક SQLITE_TCLAPI incrblobHandle(
  ClientData instanceData,
  પૂર્ણાંક dir,
  ClientData *hPtr
){
  પાછા TCL_ERROR;
}

સ્થિર Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_VERSION_5,             /* version                              */
  incrblobClose,                     /* closeProc                            */
  incrblob_આગત,                     /* inputProc                            */
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
સ્થિર પૂર્ણાંક createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  અચલ અક્ષર *zDb,
  અચલ અક્ષર *zTable,
  અચલ અક્ષર *zColumn,
  sqlite_int64 iRow,
  પૂર્ણાંક isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  પૂર્ણાંક rc;
  પૂર્ણાંક flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  સ્થિર પૂર્ણાંક ગણતરી = 0;
  અક્ષર zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  જો( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (અક્ષર *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    પાછા TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(કદ(IncrblobChannel));
  memset(p, 0, કદ(*p));
  p->pBlob = pBlob;
  જો( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(કદ(zChannel), zChannel, "incrblob_%d", ++ગણતરી);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->pNext = pDb->pIncrblob;
  p->pPrev = 0;
  જો( p->pNext ){
    p->pNext->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_SetResult(interp, (અક્ષર *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  પાછા TCL_OK;
}
#નહીંતર  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
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
સ્થિર પૂર્ણાંક safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  અચલ અક્ષર *z;
  Tcl_Size n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  જ્યારે( n-- > 0 ){
    પૂર્ણાંક c = *(z++);
    જો( c=='$' || c=='[' || c==';' ) પાછા 0;
  }
  પાછા 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
સ્થિર SqlFunc *findSqlFunc(SqliteDb *pDb, અચલ અક્ષર *zName){
  SqlFunc *p, *pNew;
  પૂર્ણાંક nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( કદ(*pNew) + nName + 1 );
  pNew->zName = (અક્ષર*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  માટે(p=pDb->pFunc; p; p=p->pNext){
    જો( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((અક્ષર*)pNew);
      પાછા p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->pNext = pDb->pFunc;
  pDb->pFunc = pNew;
  પાછા pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
સ્થિર શૂન્ય dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  જો( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((અક્ષર *)pStmt->zSql);
  }
#endif
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((અક્ષર *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
સ્થિર શૂન્ય flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *pNext;

  માટે(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pNext){
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
સ્થિર શૂન્ય addDatabaseRef(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
સ્થિર શૂન્ય delDatabaseRef(SqliteDb *pDb){
  પ્રતિજ્ઞા( pDb->nRef>0 );
  pDb->nRef--;
  જો( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    જ્યારે( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->pNext;
      પ્રતિજ્ઞા( pFunc->pDb==pDb );
      Tcl_Decr_Ref_ગણતરી(pFunc->pScript);
      Tcl_Free((અક્ષર*)pFunc);
    }
    જ્યારે( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->pNext;
      Tcl_Free((અક્ષર*)pCollate);
    }
    જો( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    જો( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    જો( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    જો( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    જો( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    જો( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    જો( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    જો( pDb->pUpdateHook ){
      Tcl_Decr_Ref_ગણતરી(pDb->pUpdateHook);
    }
    જો( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_ગણતરી(pDb->pPreUpdateHook);
    }
    જો( pDb->pRollbackHook ){
      Tcl_Decr_Ref_ગણતરી(pDb->pRollbackHook);
    }
    જો( pDb->pWalHook ){
      Tcl_Decr_Ref_ગણતરી(pDb->pWalHook);
    }
    જો( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_ગણતરી(pDb->pCollateNeeded);
    }
    Tcl_Free((અક્ષર*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
સ્થિર શૂન્ય SQLITE_TCLAPI DbDeleteCmd(શૂન્ય *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
સ્થિર પૂર્ણાંક DbBusyHandler(શૂન્ય *cd, પૂર્ણાંક nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  પૂર્ણાંક rc;
  અક્ષર zVal[30];

  sqlite3_snprintf(કદ(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (અક્ષર*)0);
  જો( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    પાછા 0;
  }
  પાછા 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
સ્થિર પૂર્ણાંક DbProgressHandler(શૂન્ય *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  પૂર્ણાંક rc;

  પ્રતિજ્ઞા( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  જો( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    પાછા 1;
  }
  પાછા 0;
}
#endif

#જો !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
સ્થિર શૂન્ય DbTraceHandler(શૂન્ય *cd, અચલ અક્ષર *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_કિંમત(&str));
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
સ્થિર પૂર્ણાંક DbTraceV2Handler(
  unsigned type, /* One of the SQLITE_TRACE_* event types. */
  શૂન્ય *cd,      /* The original context data pointer. */
  શૂન્ય *pd,      /* Primary event data, depends on event type. */
  શૂન્ય *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  પસંદગી( type ){
    સ્થિતિ SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      અક્ષર *zSql = (અક્ષર *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ગણતરી(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_ગણતરી(pCmd);
      Tcl_ResetResult(pDb->interp);
      તોડો;
    }
    સ્થિતિ SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ગણતરી(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_ગણતરી(pCmd);
      Tcl_ResetResult(pDb->interp);
      તોડો;
    }
    સ્થિતિ SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ગણતરી(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_ગણતરી(pCmd);
      Tcl_ResetResult(pDb->interp);
      તોડો;
    }
    સ્થિતિ SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ગણતરી(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_ગણતરી(pCmd);
      Tcl_ResetResult(pDb->interp);
      તોડો;
    }
  }
  પાછા SQLITE_OK;
}
#endif

#જો !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
સ્થિર શૂન્ય DbProfileHandler(શૂન્ય *cd, અચલ અક્ષર *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  અક્ષર zTm[100];

  sqlite3_snprintf(કદ(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_કિંમત(&str));
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
સ્થિર પૂર્ણાંક DbCommitHandler(શૂન્ય *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  પૂર્ણાંક rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  જો( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    પાછા 1;
  }
  પાછા 0;
}

સ્થિર શૂન્ય DbRollbackHandler(શૂન્ય *clientData){
  SqliteDb *pDb = (SqliteDb*)clientData;
  પ્રતિજ્ઞા(pDb->pRollbackHook);
  જો( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
સ્થિર પૂર્ણાંક DbWalHandler(
  શૂન્ય *clientData,
  sqlite3 *db,
  અચલ અક્ષર *zDb,
  પૂર્ણાંક nEntry
){
  પૂર્ણાંક ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)clientData;
  Tcl_Interp *interp = pDb->interp;
  પ્રતિજ્ઞા(pDb->pWalHook);

  પ્રતિજ્ઞા( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_ગણતરી(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  જો( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_ગણતરી(p);

  પાછા ret;
}

#જો defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
સ્થિર શૂન્ય setTestUnlockNotifyVars(Tcl_Interp *interp, પૂર્ણાંક iArg, પૂર્ણાંક nArg){
  અક્ષર zBuf[64];
  sqlite3_snprintf(કદ(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(કદ(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#નહીંતર
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
સ્થિર શૂન્ય DbUnlockNotify(શૂન્ય **apArg, પૂર્ણાંક nArg){
  પૂર્ણાંક i;
  માટે(i=0; i<nArg; i++){
    અચલ પૂર્ણાંક flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    setTestUnlockNotifyVars(pDb->interp, i, nArg);
    પ્રતિજ્ઞા( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_ગણતરી(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
સ્થિર શૂન્ય DbPreUpdateHandler(
  શૂન્ય *p,
  sqlite3 *db,
  પૂર્ણાંક op,
  અચલ અક્ષર *zDb,
  અચલ અક્ષર *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  સ્થિર અચલ અક્ષર *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  પ્રતિજ્ઞા( (SQLITE_DELETE-1)/9 == 0 );
  પ્રતિજ્ઞા( (SQLITE_ઉમેરો-1)/9 == 1 );
  પ્રતિજ્ઞા( (SQLITE_UPDATE-1)/9 == 2 );
  પ્રતિજ્ઞા( pDb->pPreUpdateHook );
  પ્રતિજ્ઞા( db==pDb->db );
  પ્રતિજ્ઞા( op==SQLITE_ઉમેરો || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_ગણતરી(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_ગણતરી(pCmd);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

સ્થિર શૂન્ય DbUpdateHandler(
  શૂન્ય *p,
  પૂર્ણાંક op,
  અચલ અક્ષર *zDb,
  અચલ અક્ષર *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  સ્થિર અચલ અક્ષર *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  પ્રતિજ્ઞા( (SQLITE_DELETE-1)/9 == 0 );
  પ્રતિજ્ઞા( (SQLITE_ઉમેરો-1)/9 == 1 );
  પ્રતિજ્ઞા( (SQLITE_UPDATE-1)/9 == 2 );

  પ્રતિજ્ઞા( pDb->pUpdateHook );
  પ્રતિજ્ઞા( op==SQLITE_ઉમેરો || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_ગણતરી(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_ગણતરી(pCmd);
}

સ્થિર શૂન્ય tclCollateNeeded(
  શૂન્ય *pCtx,
  sqlite3 *db,
  પૂર્ણાંક enc,
  અચલ અક્ષર *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_ગણતરી(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_ગણતરી(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
સ્થિર પૂર્ણાંક tclSqlCollate(
  શૂન્ય *pCtx,
  પૂર્ણાંક nA,
  અચલ શૂન્ય *zA,
  પૂર્ણાંક nB,
  અચલ શૂન્ય *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_ગણતરી(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_ગણતરી(pCmd);
  પાછા (atoi(Tcl_GetStringResult(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
સ્થિર શૂન્ય tclSqlFunc(sqlite3_context *context, પૂર્ણાંક argc, sqlite3_કિંમત**argv){
  SqlFunc *p = sqlite3_user_data(context);
  Tcl_Obj *pCmd;
  પૂર્ણાંક i;
  પૂર્ણાંક rc;

  જો( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_ગણતરી(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_ગણતરી(pCmd);
  }નહીંતર{
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
    જો( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
      પાછા;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_ગણતરી(pCmd);
    માટે(i=0; i<argc; i++){
      sqlite3_કિંમત *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      પસંદગી( sqlite3_કિંમત_type(pIn) ){
        સ્થિતિ SQLITE_BLOB: {
          પૂર્ણાંક bytes = sqlite3_કિંમત_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_કિંમત_blob(pIn), bytes);
          તોડો;
        }
        સ્થિતિ SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_કિંમત_int64(pIn);
          જો( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((પૂર્ણાંક)v);
          }નહીંતર{
            pVal = Tcl_NewWideIntObj(v);
          }
          તોડો;
        }
        સ્થિતિ SQLITE_FLOAT: {
          દ્વિગુણ r = sqlite3_કિંમત_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          તોડો;
        }
        સ્થિતિ SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          તોડો;
        }
        મૂળભૂત: {
          પૂર્ણાંક bytes = sqlite3_કિંમત_bytes(pIn);
          pVal = Tcl_NewStringObj((અક્ષર *)sqlite3_કિંમત_text(pIn), bytes);
          તોડો;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      જો( rc ){
        Tcl_Decr_Ref_ગણતરી(pCmd);
        sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
        પાછા;
      }
    }
    જો( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_Decr_Ref_ગણતરી(pCmd);
  }

  જો( TCL_BREAK==rc ){
    sqlite3_result_null(context);
  }નહીંતર જો( rc && rc!=TCL_RETURN ){
    sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
  }નહીંતર{
    Tcl_Obj *pVar = Tcl_GetObjResult(p->interp);
    Tcl_Size n;
    u8 *data;
    અચલ અક્ષર *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    અક્ષર c = zType[0];
    પૂર્ણાંક eType = p->eType;

    જો( eType==SQLITE_NULL ){
      જો( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }નહીંતર જો( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }નહીંતર જો( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }નહીંતર{
        eType = SQLITE_TEXT;
      }
    }

    પસંદગી( eType ){
      સ્થિતિ SQLITE_BLOB: {
        data = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_result_blob64(context, data, n, SQLITE_TRANSIENT);
        તોડો;
      }
      સ્થિતિ SQLITE_INTEGER: {
        Tcl_WideInt v;
        જો( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_result_int64(context, v);
          તોડો;
        }
        /* fall-through */
      }
      સ્થિતિ SQLITE_FLOAT: {
        દ્વિગુણ r;
        જો( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_result_double(context, r);
          તોડો;
        }
        /* fall-through */
      }
      મૂળભૂત: {
        data = (unsigned અક્ષર *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_result_text64(context, (અક્ષર *)data, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        તોડો;
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
સ્થિર પૂર્ણાંક auth_callback(
  શૂન્ય *pArg,
  પૂર્ણાંક code,
  અચલ અક્ષર *zArg1,
  અચલ અક્ષર *zArg2,
  અચલ અક્ષર *zArg3,
  અચલ અક્ષર *zArg4
){
  અચલ અક્ષર *zCode;
  Tcl_DString str;
  પૂર્ણાંક rc;
  અચલ અક્ષર *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  જો( pDb->disableAuth ) પાછા SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  પસંદગી( code ){
    સ્થિતિ SQLITE_COPY              : zCode="SQLITE_COPY"; તોડો;
    સ્થિતિ SQLITE_CREATE_INDEX      : zCode="SQLITE_CREATE_INDEX"; તોડો;
    સ્થિતિ SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; તોડો;
    સ્થિતિ SQLITE_CREATE_TEMP_INDEX : zCode="SQLITE_CREATE_TEMP_INDEX"; તોડો;
    સ્થિતિ SQLITE_CREATE_TEMP_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; તોડો;
    સ્થિતિ SQLITE_CREATE_TEMP_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; તોડો;
    સ્થિતિ SQLITE_CREATE_TEMP_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; તોડો;
    સ્થિતિ SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; તોડો;
    સ્થિતિ SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; તોડો;
    સ્થિતિ SQLITE_DELETE            : zCode="SQLITE_DELETE"; તોડો;
    સ્થિતિ SQLITE_DROP_INDEX        : zCode="SQLITE_DROP_INDEX"; તોડો;
    સ્થિતિ SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; તોડો;
    સ્થિતિ SQLITE_DROP_TEMP_INDEX   : zCode="SQLITE_DROP_TEMP_INDEX"; તોડો;
    સ્થિતિ SQLITE_DROP_TEMP_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; તોડો;
    સ્થિતિ SQLITE_DROP_TEMP_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; તોડો;
    સ્થિતિ SQLITE_DROP_TEMP_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; તોડો;
    સ્થિતિ SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; તોડો;
    સ્થિતિ SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; તોડો;
    સ્થિતિ SQLITE_ઉમેરો            : zCode="SQLITE_INSERT"; તોડો;
    સ્થિતિ SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; તોડો;
    સ્થિતિ SQLITE_READ              : zCode="SQLITE_READ"; તોડો;
    સ્થિતિ SQLITE_SELECT            : zCode="SQLITE_SELECT"; તોડો;
    સ્થિતિ SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; તોડો;
    સ્થિતિ SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; તોડો;
    સ્થિતિ SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; તોડો;
    સ્થિતિ SQLITE_DETACH            : zCode="SQLITE_DETACH"; તોડો;
    સ્થિતિ SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; તોડો;
    સ્થિતિ SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; તોડો;
    સ્થિતિ SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; તોડો;
    સ્થિતિ SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; તોડો;
    સ્થિતિ SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; તોડો;
    સ્થિતિ SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; તોડો;
    સ્થિતિ SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; તોડો;
    સ્થિતિ SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; તોડો;
    મૂળભૂત                       : zCode="????"; તોડો;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, zCode);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_કિંમત(&str));
  Tcl_DStringFree(&str);
  zReply = rc==TCL_OK ? Tcl_GetStringResult(pDb->interp) : "SQLITE_DENY";
  જો( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }નહીંતર જો( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }નહીંતર જો( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }નહીંતર{
    rc = 999;
  }
  પાછા rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#જો 0
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
સ્થિર અક્ષર *local_getline(અક્ષર *zPrompt, FILE *in){
  અક્ષર *z_લીટી;
  પૂર્ણાંક n_લીટી;
  પૂર્ણાંક n;

  n_લીટી = 100;
  z_લીટી = malloc( n_લીટી );
  જો( z_લીટી==0 ) પાછા 0;
  n = 0;
  જ્યારે( 1 ){
    જો( n+100>n_લીટી ){
      n_લીટી = n_લીટી*2 + 100;
      z_લીટી = realloc(z_લીટી, n_લીટી);
      જો( z_લીટી==0 ) પાછા 0;
    }
    જો( fgets(&z_લીટી[n], n_લીટી - n, in)==0 ){
      જો( n==0 ){
        મુક્ત(z_લીટી);
        પાછા 0;
      }
      z_લીટી[n] = 0;
      તોડો;
    }
    જ્યારે( z_લીટી[n] ){ n++; }
    જો( n>0 && z_લીટી[n-1]=='\n' ){
      n--;
      z_લીટી[n] = 0;
      તોડો;
    }
  }
  z_લીટી = realloc( z_લીટી, n+1 );
  પાછા z_લીટી;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI DbTransPostCmd(
  ClientData data[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  પૂર્ણાંક result                           /* Result of evaluating SCRIPT */
){
  સ્થિર અચલ અક્ષર *અચલ azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)data[0];
  પૂર્ણાંક rc = result;
  અચલ અક્ષર *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  જો( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    જો( rc!=TCL_ERROR ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (અક્ષર*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  પાછા rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
સ્થિર પૂર્ણાંક dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  અચલ અક્ષર *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  અચલ અક્ષર **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned પૂર્ણાંક prepFlags = 0;
#ifdef SQLITE_TEST
  જો( pDb->bLegacyPrepare ){
    પાછા sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  જો( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  પાછા sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
સ્થિર પૂર્ણાંક dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  અક્ષર અચલ *zIn,                /* SQL to compile */
  અક્ષર અચલ **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  અચલ અક્ષર *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  પૂર્ણાંક nSql;                       /* Length of zSql in bytes */
  પૂર્ણાંક nVar = 0;                   /* Number of variables in statement */
  પૂર્ણાંક iParm = 0;                  /* Next free entry in apParm */
  અક્ષર c;
  પૂર્ણાંક i;
  પૂર્ણાંક needResultReset = 0;        /* Need to invoke Tcl_ResetResult() */
  પૂર્ણાંક rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  જ્યારે( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  માટે(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->pNext){
    પૂર્ણાંક n = pPreStmt->nSql;
    જો( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      જો( pPreStmt->pPrev ){
        pPreStmt->pPrev->pNext = pPreStmt->pNext;
      }નહીંતર{
        pDb->stmtList = pPreStmt->pNext;
      }
      જો( pPreStmt->pNext ){
        pPreStmt->pNext->pPrev = pPreStmt->pPrev;
      }નહીંતર{
        pDb->stmtLast = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_ગણતરી(pStmt);
      તોડો;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  જો( pPreStmt==0 ){
    પૂર્ણાંક nByte;

    જો( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      પાછા TCL_ERROR;
    }
    જો( pStmt==0 ){
      જો( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        પાછા TCL_ERROR;
      }નહીંતર{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        પાછા TCL_OK;
      }
    }

    પ્રતિજ્ઞા( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_ગણતરી(pStmt);
    nByte = કદ(SqlPreparedStmt) + nVar*કદ(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (પૂર્ણાંક)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    જો( pPreStmt->zSql==0 ){
      અક્ષર *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      memcpy(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#endif
  }
  પ્રતિજ્ઞા( pPreStmt );
  પ્રતિજ્ઞા( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  પ્રતિજ્ઞા( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  માટે(i=1; i<=nVar; i++){
    અચલ અક્ષર *zVar = sqlite3_bind_parameter_name(pStmt, i);
    જો( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      જો( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        પૂર્ણાંક rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_ગણતરી(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        જો( needResultReset ) Tcl_ResetResult(interp);
        needResultReset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_Decr_Ref_ગણતરી(pCmd);
        જો( rx==TCL_OK ){
          pVar = Tcl_GetObjResult(interp);
        }નહીંતર જો( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          તોડો;
        }નહીંતર{
          pVar = 0;
        }
      }
      જો( pVar ){
        Tcl_Size n;
        u8 *data;
        અચલ અક્ષર *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        જો( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          data = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, data, n, SQLITE_STATIC);
          Tcl_Incr_Ref_ગણતરી(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }નહીંતર જો( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          પૂર્ણાંક nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }નહીંતર જો( c=='d' && strcmp(zType,"double")==0 ){
          દ્વિગુણ r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }નહીંતર જો( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }નહીંતર{
          data = (unsigned અક્ષર *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (અક્ષર *)data, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_ગણતરી(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }નહીંતર{
        sqlite3_bind_null(pStmt, i);
      }
      જો( needResultReset ) Tcl_ResetResult(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  જો( needResultReset && rc==TCL_OK ) Tcl_ResetResult(pDb->interp);

  પાછા rc;
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
સ્થિર શૂન્ય dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  પૂર્ણાંક discard                     /* True to delete (not cache) the pPreStmt */
){
  પૂર્ણાંક i;

  /* Free the bound string and blob parameters */
  માટે(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_ગણતરી(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  જો( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }નહીંતર{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->pNext = pDb->stmtList;
    pPreStmt->pPrev = 0;
    જો( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    જો( pDb->stmtLast==0 ){
      પ્રતિજ્ઞા( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }નહીંતર{
      પ્રતિજ્ઞા( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    જ્યારે( pDb->nStmt>pDb->maxStmt ){
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
પ્રકારવ્યાખ્યા સંરચના DbEvalContext DbEvalContext;
સંરચના DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  અચલ અક્ષર *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  પૂર્ણાંક nCol;                       /* Number of columns returned by pStmt */
  પૂર્ણાંક evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#define SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#define SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
સ્થિર શૂન્ય dbReleaseColumnNames(DbEvalContext *p){
  જો( p->apColName ){
    પૂર્ણાંક i;
    માટે(i=0; i<p->nCol; i++){
      Tcl_Decr_Ref_ગણતરી(p->apColName[i]);
    }
    Tcl_Free((અક્ષર *)p->apColName);
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
સ્થિર શૂન્ય dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  પૂર્ણાંક evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, કદ(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_ગણતરી(pSql);
  જો( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_ગણતરી(pVarName);
  }
  p->evalFlags = evalFlags;
  addDatabaseRef(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
સ્થિર શૂન્ય dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  પૂર્ણાંક *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  જો( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    પૂર્ણાંક i;                        /* Iterator variable */
    પૂર્ણાંક nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_ગણતરી(pStmt);
    જો( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( કદ(Tcl_Obj*)*nCol );
      માટે(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_Incr_Ref_ગણતરી(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    જો( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_ગણતરી(pColList);
      Tcl_Incr_Ref_ગણતરી(pStar);
      માટે(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      જો( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }નહીંતર{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, ખાલી, 0);
        જો( !pDict ){
          pDict = Tcl_NewDictObj();
        }નહીંતર જો( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        જો( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, ખાલી, pDict, 0);
        }
        Tcl_Bounce_Ref_ગણતરી(pDict);
      }
      Tcl_Decr_Ref_ગણતરી(pStar);
      Tcl_Decr_Ref_ગણતરી(pColList);
    }
  }

  જો( papColName ){
    *papColName = p->apColName;
  }
  જો( pnCol ){
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
સ્થિર પૂર્ણાંક dbEvalStep(DbEvalContext *p){
  અચલ અક્ષર *zPrevSql = 0;       /* Previous value of p->zSql */

  જ્યારે( p->zSql[0] || p->pPreStmt ){
    પૂર્ણાંક rc;
    જો( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      જો( rc!=TCL_OK ) પાછા rc;
    }નહીંતર{
      પૂર્ણાંક rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      જો( rcs==SQLITE_ROW ){
        પાછા TCL_OK;
      }
      જો( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->nIndex = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      જો( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#જો SQLITE_TEST
        જો( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          ચાલુ;
        }
#endif
        Tcl_SetObjResult(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        પાછા TCL_ERROR;
      }નહીંતર{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  પાછા TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
સ્થિર શૂન્ય dbEvalFinalize(DbEvalContext *p){
  જો( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  જો( p->pVarName ){
    Tcl_Decr_Ref_ગણતરી(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_ગણતરી(p->pSql);
  dbReleaseColumnNames(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
સ્થિર Tcl_Obj *db_Eval_Column_કિંમત(DbEvalContext *p, પૂર્ણાંક iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  પસંદગી( sqlite3_column_type(pStmt, iCol) ){
    સ્થિતિ SQLITE_BLOB: {
      પૂર્ણાંક bytes = sqlite3_column_bytes(pStmt, iCol);
      અચલ અક્ષર *zBlob = sqlite3_column_blob(pStmt, iCol);
      જો( !zBlob ) bytes = 0;
      પાછા Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    સ્થિતિ SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      જો( v>=-2147483647 && v<=2147483647 ){
        પાછા Tcl_NewIntObj((પૂર્ણાંક)v);
      }નહીંતર{
        પાછા Tcl_NewWideIntObj(v);
      }
    }
    સ્થિતિ SQLITE_FLOAT: {
      પાછા Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    સ્થિતિ SQLITE_NULL: {
      પાછા Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  પાછા Tcl_NewStringObj((અક્ષર*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#જો TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
સ્થિર પૂર્ણાંક DbUseNre(શૂન્ય){
  પૂર્ણાંક major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  પાછા( (major==8 && minor>=6) || major>8 );
}
#નહીંતર
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (શૂન્ય)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (શૂન્ય)0
#endif

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI DbEvalNextCmd(
  ClientData data[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  પૂર્ણાંક result                           /* Result so far */
){
  પૂર્ણાંક rc = result;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)data[0];
  Tcl_Obj * અચલ pScript = (Tcl_Obj *)data[1];
  Tcl_Obj * અચલ pVarName = p->pVarName;

  જ્યારે( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    પૂર્ણાંક i;
    પૂર્ણાંક nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    માટે(i=0; i<nCol; i++){
      જો( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, db_Eval_Column_કિંમત(p,i), 0);
      }નહીંતર જો( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        જો( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }નહીંતર{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ખાલી, 0);
          જો( pDict ){
            જો( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            જો( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, ખાલી, pDict, 0);
            }
            Tcl_Bounce_Ref_ગણતરી(pDict);
          }
        }
      }નહીંતર જો( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       db_Eval_Column_કિંમત(p,i), 0);
      }નહીંતર{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ખાલી, 0);
        જો( !pDict ){
          pDict = Tcl_NewDictObj();
        }નહીંતર જો( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        જો( Tcl_DictObjPut(interp, pDict, apColName[i],
                           db_Eval_Column_કિંમત(p,i))==TCL_OK ){
          Tcl_ObjSetVar2(interp, pVarName, ખાલી, pDict, 0);
        }
        Tcl_Bounce_Ref_ગણતરી(pDict);
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
    જો( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbEvalNextCmd, (શૂન્ય*)p, (શૂન્ય*)pScript, 0, 0);
      પાછા Tcl_NREvalObj(interp, pScript, 0);
    }નહીંતર{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_ગણતરી(pScript);
  dbEvalFinalize(p);
  Tcl_Free((અક્ષર *)p);

  જો( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_ResetResult(interp);
    rc = TCL_OK;
  }
  પાછા rc;
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
સ્થિર શૂન્ય DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  જો( *ppHook ){
    Tcl_SetObjResult(interp, *ppHook);
    જો( pArg ){
      Tcl_Decr_Ref_ગણતરી(*ppHook);
      *ppHook = 0;
    }
  }
  જો( pArg ){
    પ્રતિજ્ઞા( !(*ppHook) );
    જો( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_ગણતરી(*ppHook);
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
સ્થિર પૂર્ણાંક dbQrf(SqliteDb *pDb, પૂર્ણાંક objc, Tcl_Obj *અચલ*objv){
#ifndef SQLITE_QRF_H
  Tcl_SetResult(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  પાછા TCL_ERROR;
#નહીંતર
  અક્ષર *zResult = 0;             /* Result to be returned */
  અચલ અક્ષર *zSql = 0;          /* SQL to run */
  પૂર્ણાંક i;                         /* Loop counter */
  પૂર્ણાંક rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  સ્થિર અચલ અક્ષર *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  સ્થિર અચલ unsigned અક્ષર aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_ડાબું,
    QRF_ALIGN_મધ્ય, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_જમણું,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_ટોચ,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, કદ(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &zResult;
  માટે(i=2; i<objc; i++){
    અચલ અક્ષર *zArg = Tcl_GetString(objv[i]);
    અચલ અક્ષર *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    અચલ unsigned અક્ષર aBoolMap[] = { 0, 2, 1, 2, 1 };
    જો( zArg[0]!='-' ){
      જો( zSql ){
        Tcl_AppendResult(pDb->interp, "unknown argument: ", zArg, (અક્ષર*)0);
        rc = TCL_ERROR;
        જાઓ format_failed;
      }
      zSql  = zArg;
    }નહીંતર જો( i==objc-1 ){
      Tcl_AppendResult(pDb->interp, "option has no argument: ", zArg,(અક્ષર*)0);
      rc = TCL_ERROR;
      જાઓ format_failed;
    }નહીંતર જો( strcmp(zArg,"-style")==0 ){
      સ્થિર અચલ અક્ષર *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      સ્થિર unsigned અક્ષર aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_ગણતરી,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_ઉમેરો,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_લીટી,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      પૂર્ણાંક style;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      જો( rc ) જાઓ format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }નહીંતર જો( strcmp(zArg,"-esc")==0 ){
      સ્થિર અચલ અક્ષર *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      સ્થિર unsigned અક્ષર aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_પ્રતીક
      };
      પૂર્ણાંક esc;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      જો( rc ) જાઓ format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }નહીંતર જો( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      સ્થિર અચલ અક્ષર *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      સ્થિર unsigned અક્ષર aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      પૂર્ણાંક txt;
      પૂર્ણાંક k = zArg[2]=='e';
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      જો( rc ) જાઓ format_failed;
      જો( k ){
        qrf.eText = aTextMap[txt];
      }નહીંતર જો( txt<=2 ){
        પ્રતિજ્ઞા( QRF_No==1 );
        પ્રતિજ્ઞા( QRF_Yes==2 );
        પ્રતિજ્ઞા( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }નહીંતર{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }નહીંતર જો( strcmp(zArg,"-blob")==0 ){
      સ્થિર અચલ અક્ષર *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      સ્થિર unsigned અક્ષર aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_Size
      };
      પૂર્ણાંક blob;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      જો( rc ) જાઓ format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }નહીંતર જો( strcmp(zArg,"-wordwrap")==0 ){
      પૂર્ણાંક v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      જો( rc ) જાઓ format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }નહીંતર જો( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      પૂર્ણાંક v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      જો( rc ) જાઓ format_failed;
      જો( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }નહીંતર જો( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }નહીંતર જો( zArg[1]=='r' ){
        qrf.b_Row_ગણતરી = aBoolMap[v];
      }નહીંતર{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }નહીંતર જો( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      પૂર્ણાંક ax = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      જો( rc ) જાઓ format_failed;
      જો( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }નહીંતર{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }નહીંતર જો( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      પૂર્ણાંક v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      જો( rc ) જાઓ format_failed;
      જો( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }નહીંતર જો( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      જો( zArg[1]=='w' ){
        qrf.nWrap = v;
      }નહીંતર જો( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }નહીંતર જો( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }નહીંતર{
        qrf.n_લીટી_Limit = v;
      }
      i++;
    }નહીંતર જો( strcmp(zArg,"-charlimit")==0 ){
      પૂર્ણાંક v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      જો( rc ) જાઓ format_failed;
      જો( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }નહીંતર જો( strcmp(zArg,"-multiinsert")==0 ){
      પૂર્ણાંક v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      જો( rc ) જાઓ format_failed;
      જો( v<0 ) v = 0;
      qrf.n_Multi_ઉમેરો = v;
      i++;
    }નહીંતર જો( strcmp(zArg,"-align")==0 ){
      Tcl_Size n = 0;
      પૂર્ણાંક jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      જો( rc ) જાઓ format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*કદ(qrf.aAlign[0]) );
      જો( qrf.aAlign==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (અક્ષર*)0);
        rc = TCL_ERROR;
        જાઓ format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*કદ(qrf.aAlign[0]));
      qrf.nAlign = n;
      માટે(jj=0; jj<n; jj++){
        પૂર્ણાંક x;
        Tcl_Obj *pTerm;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        જો( rc ) જાઓ format_failed;
        rc = Tcl_GetIndexFromObj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        જો( rc ) જાઓ format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }નહીંતર જો( strcmp(zArg,"-widths")==0 ){
      Tcl_Size n = 0;
      પૂર્ણાંક jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      જો( rc ) જાઓ format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*કદ(qrf.aWidth[0]) );
      જો( qrf.aWidth==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (અક્ષર*)0);
        rc = TCL_ERROR;
        જાઓ format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*કદ(qrf.aWidth[0]));
      qrf.nWidth = n;
      માટે(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        પૂર્ણાંક v;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        જો( rc ) જાઓ format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        જો( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }નહીંતર જો( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short પૂર્ણાંક)v;
      }
      i++;
    }નહીંતર જો( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }નહીંતર જો( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }નહીંતર જો( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }નહીંતર જો( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }નહીંતર જો( strcmp(zArg,"-ifmt")==0 ){
      અક્ષર *zFmt = Tcl_GetString(objv[i+1]);
      જો( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }નહીંતર જો( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_AppendResult(pDb->interp, "bad format for -ifmt", (અક્ષર*)0);
        rc = TCL_ERROR;
        જાઓ format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }નહીંતર જો( strcmp(zArg,"-fpfmt")==0 ){
      અક્ષર *zFmt = Tcl_GetString(objv[i+1]);
      જો( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }નહીંતર જો( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_AppendResult(pDb->interp, "bad format for -fpfmt", (અક્ષર*)0);
        rc = TCL_ERROR;
        જાઓ format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }નહીંતર જો( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }નહીંતર{
      Tcl_AppendResult(pDb->interp, "unknown option: ", zArg, (અક્ષર*)0);
      rc = TCL_ERROR;
      જાઓ format_failed;
    }
  }
  જ્યારે( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    અક્ષર *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    જો( rc ) જાઓ format_failed;
    જો( pStmt==0 ) ચાલુ;
    rc = sqlite3_format_query_result(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    જો( rc ){
      Tcl_SetResult(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      જાઓ format_failed;
    }
  }
  Tcl_SetResult(pDb->interp, zResult, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.aWidth);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(zResult);
  પાછા rc;

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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI DbObjCmd(
  શૂન્ય *cd,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *અચલ*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  પૂર્ણાંક choice;
  પૂર્ણાંક rc = TCL_OK;
  સ્થિર અચલ અક્ષર *DB_strs[] = {
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
  ગણના DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_LOAD_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_INTERRUPT,            DB_LAST_ઉમેરો_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_PROGRESS,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_STATUS,                DB_TIMEOUT,              DB_TOTAL_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_VERSION,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  જો( objc<2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "SUBCOMMAND ...");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    પાછા TCL_ERROR;
  }

  પસંદગી( (ગણના DB_enum)choice ){

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
  સ્થિતિ DB_AUTHORIZER: {
#ifdef SQLITE_OMIT_AUTHORIZATION
    Tcl_AppendResult(interp, "authorization not available in this build",
                     (અક્ષર*)0);
    પાછા TCL_ERROR;
#નહીંતર
    જો( objc>3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?CALLBACK?");
      પાછા TCL_ERROR;
    }નહીંતર જો( objc==2 ){
      જો( pDb->zAuth ){
        Tcl_AppendResult(interp, pDb->zAuth, (અક્ષર*)0);
      }
    }નહીંતર{
      અક્ષર *zAuth;
      Tcl_Size len;
      જો( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      જો( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }નહીંતર{
        pDb->zAuth = 0;
      }
      જો( pDb->zAuth ){
        પ્રકારવ્યાખ્યા પૂર્ણાંક (*sqlite3_auth_cb)(
           શૂન્ય*,પૂર્ણાંક,અચલ અક્ષર*,અચલ અક્ષર*,
           અચલ અક્ષર*,અચલ અક્ષર*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }નહીંતર{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#endif
    તોડો;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  સ્થિતિ DB_BACKUP: {
    અચલ અક્ષર *zDestFile;
    અચલ અક્ષર *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    જો( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }નહીંતર જો( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }નહીંતર{
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?DATABASE? FILENAME");
      પાછા TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    જો( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (અક્ષર*)0);
      sqlite3_close(pDest);
      પાછા TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    જો( pBackup==0 ){
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (અક્ષર*)0);
      sqlite3_close(pDest);
      પાછા TCL_ERROR;
    }
    જ્યારે(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    જો( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }નહીંતર{
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (અક્ષર*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    તોડો;
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
  સ્થિતિ DB_BIND_FALLBACK: {
    જો( objc>3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?CALLBACK?");
      પાછા TCL_ERROR;
    }નહીંતર જો( objc==2 ){
      જો( pDb->zBindFallback ){
        Tcl_AppendResult(interp, pDb->zBindFallback, (અક્ષર*)0);
      }
    }નહીંતર{
      અક્ષર *zCallback;
      Tcl_Size len;
      જો( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      જો( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }નહીંતર{
        pDb->zBindFallback = 0;
      }
    }
    તોડો;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  સ્થિતિ DB_BUSY: {
    જો( objc>3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "CALLBACK");
      પાછા TCL_ERROR;
    }નહીંતર જો( objc==2 ){
      જો( pDb->zBusy ){
        Tcl_AppendResult(interp, pDb->zBusy, (અક્ષર*)0);
      }
    }નહીંતર{
      અક્ષર *zBusy;
      Tcl_Size len;
      જો( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      જો( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }નહીંતર{
        pDb->zBusy = 0;
      }
      જો( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }નહીંતર{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    તોડો;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  સ્થિતિ DB_CACHE: {
    અક્ષર *subCmd;
    પૂર્ણાંક n;

    જો( objc<=2 ){
      Tcl_Wrong_અંક_Args(interp, 1, objv, "cache option ?arg?");
      પાછા TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    જો( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      જો( objc!=3 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "flush");
        પાછા TCL_ERROR;
      }નહીંતર{
        flushStmtCache( pDb );
      }
    }નહીંતર જો( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      જો( objc!=4 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "size n");
        પાછા TCL_ERROR;
      }નહીંતર{
        જો( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_AppendResult( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (અક્ષર*)0);
          પાછા TCL_ERROR;
        }નહીંતર{
          જો( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }નહીંતર જો( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }નહીંતર{
      Tcl_AppendResult( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
    તોડો;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  સ્થિતિ DB_CHANGES: {
    Tcl_Obj *pResult;
    જો( objc!=2 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "");
      પાછા TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_changes64(pDb->db));
    તોડો;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  સ્થિતિ DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    તોડો;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  સ્થિતિ DB_COLLATE: {
    SqlCollate *pCollate;
    અક્ષર *zName;
    અક્ષર *zScript;
    Tcl_Size nScript;
    જો( objc!=4 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "NAME SCRIPT");
      પાછા TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( કદ(*pCollate) + nScript + 1 );
    જો( pCollate==0 ) પાછા TCL_ERROR;
    pCollate->interp = interp;
    pCollate->pNext = pDb->pCollate;
    pCollate->zScript = (અક્ષર*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    જો( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_SetResult(interp, (અક્ષર *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      પાછા TCL_ERROR;
    }
    તોડો;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  સ્થિતિ DB_COLLATION_NEEDED: {
    જો( objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "SCRIPT");
      પાછા TCL_ERROR;
    }
    જો( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_ગણતરી(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_ગણતરી(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    તોડો;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  સ્થિતિ DB_COMMIT_HOOK: {
    જો( objc>3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?CALLBACK?");
      પાછા TCL_ERROR;
    }નહીંતર જો( objc==2 ){
      જો( pDb->zCommit ){
        Tcl_AppendResult(interp, pDb->zCommit, (અક્ષર*)0);
      }
    }નહીંતર{
      અચલ અક્ષર *zCommit;
      Tcl_Size len;
      જો( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      જો( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }નહીંતર{
        pDb->zCommit = 0;
      }
      જો( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }નહીંતર{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    તોડો;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  સ્થિતિ DB_COMPLETE: {
#ifndef SQLITE_OMIT_COMPLETE
    Tcl_Obj *pResult;
    પૂર્ણાંક isComplete;
    જો( objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "SQL");
      પાછા TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetBooleanObj(pResult, isComplete);
#endif
    તોડો;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  સ્થિતિ DB_CONFIG: {
    સ્થિર અચલ સંરચના DbConfigChoices {
      અચલ અક્ષર *zName;
      પૂર્ણાંક op;
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
    પૂર્ણાંક ii;
    જો( objc>4 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      પાછા TCL_ERROR;
    }
    જો( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      pResult = Tcl_NewListObj(0,0);
      માટે(ii=0; ii<કદ(aDbConfig)/કદ(aDbConfig[0]); ii++){
        પૂર્ણાંક v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewIntObj(v));
      }
    }નહીંતર{
      અચલ અક્ષર *zOpt = Tcl_GetString(objv[2]);
      પૂર્ણાંક onoff = -1;
      પૂર્ણાંક v = 0;
      જો( zOpt[0]=='-' ) zOpt++;
      માટે(ii=0; ii<કદ(aDbConfig)/કદ(aDbConfig[0]); ii++){
        જો( strcmp(aDbConfig[ii].zName, zOpt)==0 ) તોડો;
      }
      જો( ii>=કદ(aDbConfig)/કદ(aDbConfig[0]) ){
        Tcl_AppendResult(interp, "unknown config option: \"", zOpt,
                                "\"", (શૂન્ય*)0);
        પાછા TCL_ERROR;
      }
      જો( objc==4 ){
        જો( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          પાછા TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      pResult = Tcl_NewIntObj(v);
    }
    Tcl_SetObjResult(interp, pResult);
    તોડો;
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
  સ્થિતિ DB_COPY: {
    અક્ષર *zTable;               /* Insert data into this table */
    અક્ષર *zFile;                /* The file from which to extract data */
    અક્ષર *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    પૂર્ણાંક nCol;                   /* Number of columns in the table */
    પૂર્ણાંક nByte;                  /* Number of bytes in an SQL string */
    પૂર્ણાંક i, j;                   /* Loop counters */
    પૂર્ણાંક nSep;                   /* Number of bytes in zSep[] */
    પૂર્ણાંક nNull;                  /* Number of bytes in zNull[] */
    અક્ષર *zSql;                 /* An SQL statement */
    અક્ષર *z_લીટી;                /* A single line of input from the file */
    અક્ષર **azCol;               /* zLine[] broken up into columns */
    અચલ અક્ષર *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    પૂર્ણાંક lineno = 0;             /* Line number of input file */
    અક્ષર z_લીટી_અંક[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *pResult;           /* interp result */

    અચલ અક્ષર *zSep;
    અચલ અક્ષર *zNull;
    જો( objc<5 || objc>7 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      પાછા TCL_ERROR;
    }
    જો( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }નહીંતર{
      zSep = "\t";
    }
    જો( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }નહીંતર{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    જો( nSep==0 ){
      Tcl_AppendResult(interp,"Error: non-null separator required for copy",
                       (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
    જો(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_AppendResult(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    જો( zSql==0 ){
      Tcl_AppendResult(interp, "Error: no such table: ", zTable, (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    જો( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (અક્ષર*)0);
      nCol = 0;
    }નહીંતર{
      nCol = sqlite3_column_ગણતરી(pStmt);
    }
    sqlite3_finalize(pStmt);
    જો( nCol==0 ) {
      પાછા TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    જો( zSql==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    માટે(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    મુક્ત(zSql);
    જો( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (અક્ષર*)0);
      sqlite3_finalize(pStmt);
      પાછા TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    જો( in==0 ){
      sqlite3_finalize(pStmt);
      પાછા TCL_ERROR;
    }
    Tcl_SetChannelOption(ખાલી, in, "-translation", "auto");
    azCol = malloc( કદ(azCol[0])*(nCol+1) );
    જો( azCol==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (અક્ષર*)0);
      Tcl_Close(interp, in);
      પાછા TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_ગણતરી(str);
    (શૂન્ય)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    જ્યારે( Tcl_GetsObj(in, str)>=0 ) {
      અક્ષર *z;
      Tcl_Size byteLen;
      lineno++;
      z_લીટી = (અક્ષર *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = z_લીટી;
      માટે(i=0, z=z_લીટી; *z; z++){
        જો( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          જો( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      જો( i+1!=nCol ){
        અક્ષર *zErr;
        પૂર્ણાંક nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        જો( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_AppendResult(interp, zErr, (અક્ષર*)0);
          મુક્ત(zErr);
        }
        zCommit = "ROLLBACK";
        તોડો;
      }
      માટે(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        જો( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }નહીંતર{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_SetObjLength(str, 0);
      જો( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp,"Error: ", sqlite3_errmsg(pDb->db), (અક્ષર*)0);
        zCommit = "ROLLBACK";
        તોડો;
      }
    }
    Tcl_Decr_Ref_ગણતરી(str);
    મુક્ત(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (શૂન્ય)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    જો( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      pResult = Tcl_GetObjResult(interp);
      જો( Tcl_IsShared(pResult) ){
        pResult = Tcl_DuplicateObj(pResult);
      }
      Tcl_SetIntObj(pResult, lineno);
      rc = TCL_OK;
    }નહીંતર{
      /* failure, append lineno where failed */
      sqlite3_snprintf(કદ(z_લીટી_અંક), z_લીટી_અંક,"%d",lineno);
      Tcl_AppendResult(interp,", failed while processing line: ",z_લીટી_અંક,
                       (અક્ષર*)0);
      rc = TCL_ERROR;
    }
    તોડો;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  સ્થિતિ DB_DESERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (અક્ષર*)0);
    rc = TCL_ERROR;
#નહીંતર
    અચલ અક્ષર *zSchema = 0;
    Tcl_Obj *p_કિંમત = 0;
    unsigned અક્ષર *pBA;
    unsigned અક્ષર *pData;
    Tcl_Size len;
    પૂર્ણાંક xrc;
    sqlite3_int64 mxSize = 0;
    પૂર્ણાંક i;
    પૂર્ણાંક isReadonly = 0;


    જો( objc<3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      તોડો;
    }
    માટે(i=2; i<objc-1; i++){
      અચલ અક્ષર *z = Tcl_GetString(objv[i]);
      જો( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        જો( rc ) જાઓ deserialize_error;
        mxSize = x;
        ચાલુ;
      }
      જો( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        જો( rc ) જાઓ deserialize_error;
        ચાલુ;
      }
      જો( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        ચાલુ;
      }
      Tcl_AppendResult(interp, "unknown option: ", z, (અક્ષર*)0);
      rc = TCL_ERROR;
      જાઓ deserialize_error;
    }
    p_કિંમત = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_કિંમત, &len);
    pData = sqlite3_malloc64( len );
    જો( pData==0 && len>0 ){
      Tcl_AppendResult(interp, "out of memory", (અક્ષર*)0);
      rc = TCL_ERROR;
    }નહીંતર{
      પૂર્ણાંક flags;
      જો( len>0 ) memcpy(pData, pBA, len);
      જો( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }નહીંતર{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, pData, len, len, flags);
      જો( xrc ){
        Tcl_AppendResult(interp, "unable to set MEMDB content", (અક્ષર*)0);
        rc = TCL_ERROR;
      }
      જો( mxSize>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_SIZE_LIMIT,&mxSize);
      }
    }
deserialize_error:
#endif
    તોડો; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  સ્થિતિ DB_ENABLE_LOAD_EXTENSION: {
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    પૂર્ણાંક onoff;
    જો( objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "BOOLEAN");
      પાછા TCL_ERROR;
    }
    જો( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      પાછા TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    તોડો;
#નહીંતર
    Tcl_AppendResult(interp, "extension loading is turned off at compile-time",
                     (અક્ષર*)0);
    પાછા TCL_ERROR;
#endif
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  સ્થિતિ DB_ERRORCODE: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    તોડો;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  સ્થિતિ DB_ERROROFFSET: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    તોડો;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  સ્થિતિ DB_EXISTS:
  સ્થિતિ DB_ONECOLUMN: {
    Tcl_Obj *pResult = 0;
    DbEvalContext sEval;
    જો( objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "SQL");
      પાછા TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    જો( choice==DB_ONECOLUMN ){
      જો( rc==TCL_OK ){
        pResult = db_Eval_Column_કિંમત(&sEval, 0);
      }નહીંતર જો( rc==TCL_BREAK ){
        Tcl_ResetResult(interp);
      }
    }નહીંતર જો( rc==TCL_BREAK || rc==TCL_OK ){
      pResult = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    જો( pResult ) Tcl_SetObjResult(interp, pResult);

    જો( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    તોડો;
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
  સ્થિતિ DB_EVAL: {
    પૂર્ણાંક evalFlags = 0;
    અચલ અક્ષર *zOpt;
    જ્યારે( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      જો( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }નહીંતર જો( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }નહીંતર{
        Tcl_AppendResult(interp, "unknown option: \"", zOpt, "\"", (શૂન્ય*)0);
        પાછા TCL_ERROR;
      }
      objc--;
      objv++;
    }
    જો( objc<3 || objc>5 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      પાછા TCL_ERROR;
    }

    જો( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_ગણતરી(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      જ્યારે( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        પૂર્ણાંક i;
        પૂર્ણાંક nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        માટે(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_Column_કિંમત(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      જો( rc==TCL_BREAK ){
        Tcl_SetObjResult(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_ગણતરી(pRet);
    }નહીંતર{
      ClientData cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      જો( objc>=5 && *(અક્ષર *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_ગણતરી(pScript);

      p = (DbEvalContext *)Tcl_Alloc(કદ(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (શૂન્ય *)p;
      cd2[1] = (શૂન્ય *)pScript;
      rc = DbEvalNextCmd(cd2, interp, TCL_OK);
    }
    તોડો;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  સ્થિતિ DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    તોડો;
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
  સ્થિતિ DB_FUNCTION: {
    પૂર્ણાંક flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    અક્ષર *zName;
    પૂર્ણાંક nArg = -1;
    પૂર્ણાંક i;
    પૂર્ણાંક eType = SQLITE_NULL;
    જો( objc<4 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      પાછા TCL_ERROR;
    }
    માટે(i=3; i<(objc-1); i++){
      અચલ અક્ષર *z = Tcl_GetString(objv[i]);
      પૂર્ણાંક n = strlen30(z);
      જો( n>1 && strncmp(z, "-argcount",n)==0 ){
        જો( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(અક્ષર*)0);
          પાછા TCL_ERROR;
        }
        જો( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) પાછા TCL_ERROR;
        જો( nArg<0 ){
          Tcl_AppendResult(interp, "number of arguments must be non-negative",
                           (અક્ષર*)0);
          પાછા TCL_ERROR;
        }
        i++;
      }નહીંતર
      જો( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }નહીંતર
      જો( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }નહીંતર
      જો( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }નહીંતર
      જો( n>1 && strncmp(z, "-returntype", n)==0 ){
        અચલ અક્ષર *azType[] = {"integer", "real", "text", "blob", "any", 0};
        પ્રતિજ્ઞા( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        પ્રતિજ્ઞા( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        જો( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(અક્ષર*)0);
          પાછા TCL_ERROR;
        }
        i++;
        જો( Tcl_GetIndexFromObj(interp, objv[i], azType, "type", 0, &eType) ){
          પાછા TCL_ERROR;
        }
        eType++;
      }નહીંતર{
        Tcl_AppendResult(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (અક્ષર*)0
        );
        પાછા TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    જો( pFunc==0 ) પાછા TCL_ERROR;
    જો( pFunc->pScript ){
      Tcl_Decr_Ref_ગણતરી(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_ગણતરી(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    જો( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_SetResult(interp, (અક્ષર *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    તોડો;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  સ્થિતિ DB_INCRBLOB: {
#ifdef SQLITE_OMIT_INCRBLOB
    Tcl_AppendResult(interp, "incrblob not available in this build", (અક્ષર*)0);
    પાછા TCL_ERROR;
#નહીંતર
    પૂર્ણાંક isReadonly = 0;
    અચલ અક્ષર *zDb = "main";
    અચલ અક્ષર *zTable;
    અચલ અક્ષર *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    જો( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    જો( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      પાછા TCL_ERROR;
    }

    જો( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    જો( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#endif
    તોડો;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  સ્થિતિ DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    તોડો;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  સ્થિતિ DB_NULLVALUE: {
    જો( objc!=2 && objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "NULLVALUE");
      પાછા TCL_ERROR;
    }
    જો( objc==3 ){
      Tcl_Size len;
      અક્ષર *zNull = Tcl_GetStringFromObj(objv[2], &len);
      જો( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      જો( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }નહીંતર{
        pDb->zNull = 0;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(pDb->zNull, -1));
    તોડો;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  સ્થિતિ DB_LAST_ઉમેરો_ROWID: {
    Tcl_Obj *pResult;
    Tcl_WideInt rowid;
    જો( objc!=2 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "");
      પાછા TCL_ERROR;
    }
    rowid = sqlite3_last_ઉમેરો_rowid(pDb->db);
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, rowid);
    તોડો;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  સ્થિતિ DB_PROGRESS: {
    જો( objc==2 ){
      જો( pDb->zProgress ){
        Tcl_AppendResult(interp, pDb->zProgress, (અક્ષર*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }નહીંતર જો( objc==4 ){
      અક્ષર *zProgress;
      Tcl_Size len;
      પૂર્ણાંક N;
      જો( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        પાછા TCL_ERROR;
      };
      જો( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      જો( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }નહીંતર{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      જો( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }નહીંતર{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }નહીંતર{
      Tcl_Wrong_અંક_Args(interp, 2, objv, "N CALLBACK");
      પાછા TCL_ERROR;
    }
    તોડો;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  સ્થિતિ DB_PROFILE: {
    જો( objc>3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?CALLBACK?");
      પાછા TCL_ERROR;
    }નહીંતર જો( objc==2 ){
      જો( pDb->zProfile ){
        Tcl_AppendResult(interp, pDb->zProfile, (અક્ષર*)0);
      }
    }નહીંતર{
      અક્ષર *zProfile;
      Tcl_Size len;
      જો( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      જો( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }નહીંતર{
        pDb->zProfile = 0;
      }
#જો !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      જો( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }નહીંતર{
        sqlite3_profile(pDb->db, 0, 0);
      }
#endif
    }
    તોડો;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  સ્થિતિ DB_REKEY: {
    જો( objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "KEY");
      પાછા TCL_ERROR;
    }
    તોડો;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  સ્થિતિ DB_RESTORE: {
    અચલ અક્ષર *zSrcFile;
    અચલ અક્ષર *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    પૂર્ણાંક nTimeout = 0;

    જો( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }નહીંતર જો( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }નહીંતર{
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?DATABASE? FILENAME");
      પાછા TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    જો( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (અક્ષર*)0);
      sqlite3_close(pSrc);
      પાછા TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    જો( pBackup==0 ){
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (અક્ષર*)0);
      sqlite3_close(pSrc);
      પાછા TCL_ERROR;
    }
    જ્યારે( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      જો( rc==SQLITE_BUSY ){
        જો( nTimeout++ >= 3 ) તોડો;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    જો( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }નહીંતર જો( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_AppendResult(interp, "restore failed: source database busy",
                       (અક્ષર*)0);
      rc = TCL_ERROR;
    }નહીંતર{
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (અક્ષર*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    તોડો;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  સ્થિતિ DB_SERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (અક્ષર*)0);
    rc = TCL_ERROR;
#નહીંતર
    અચલ અક્ષર *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned અક્ષર *pData;
    જો( objc!=2 && objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }નહીંતર{
      પૂર્ણાંક needFree;
      pData = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      જો( pData ){
        needFree = 0;
      }નહીંતર{
        pData = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(pData,sz));
      જો( needFree ) sqlite3_free(pData);
    }
#endif
    તોડો;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  સ્થિતિ DB_STATUS: {
    પૂર્ણાંક v;
    અચલ અક્ષર *zOp;
    જો( objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "(step|sort|autoindex)");
      પાછા TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    જો( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }નહીંતર જો( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }નહીંતર જો( strcmp(zOp, "autoindex")==0 ){
      v = pDb->nIndex;
    }નહીંતર જો( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }નહીંતર{
      Tcl_AppendResult(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(v));
    તોડો;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  સ્થિતિ DB_TIMEOUT: {
    પૂર્ણાંક ms;
    જો( objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "MILLISECONDS");
      પાછા TCL_ERROR;
    }
    જો( Tcl_GetIntFromObj(interp, objv[2], &ms) ) પાછા TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    તોડો;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  સ્થિતિ DB_TOTAL_CHANGES: {
    Tcl_Obj *pResult;
    જો( objc!=2 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "");
      પાછા TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_total_changes64(pDb->db));
    તોડો;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  સ્થિતિ DB_TRACE: {
    જો( objc>3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?CALLBACK?");
      પાછા TCL_ERROR;
    }નહીંતર જો( objc==2 ){
      જો( pDb->zTrace ){
        Tcl_AppendResult(interp, pDb->zTrace, (અક્ષર*)0);
      }
    }નહીંતર{
      અક્ષર *zTrace;
      Tcl_Size len;
      જો( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      જો( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }નહીંતર{
        pDb->zTrace = 0;
      }
#જો !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      જો( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }નહીંતર{
        sqlite3_trace(pDb->db, 0, 0);
      }
#endif
    }
    તોડો;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  સ્થિતિ DB_TRACE_V2: {
    જો( objc>4 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?CALLBACK? ?MASK?");
      પાછા TCL_ERROR;
    }નહીંતર જો( objc==2 ){
      જો( pDb->zTraceV2 ){
        Tcl_AppendResult(interp, pDb->zTraceV2, (અક્ષર*)0);
      }
    }નહીંતર{
      અક્ષર *zTraceV2;
      Tcl_Size len;
      Tcl_WideInt wMask = 0;
      જો( objc==4 ){
        સ્થિર અચલ અક્ષર *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        ગણના TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_Size i;
        જો( TCL_OK!=Tcl_ListObjLength(interp, objv[3], &len) ){
          પાછા TCL_ERROR;
        }
        માટે(i=0; i<len; i++){
          Tcl_Obj *pObj;
          પૂર્ણાંક ttype;
          જો( TCL_OK!=Tcl_ListObjIndex(interp, objv[3], i, &pObj) ){
            પાછા TCL_ERROR;
          }
          જો( Tcl_GetIndexFromObj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
            Tcl_Incr_Ref_ગણતરી(pError);
            જો( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_ગણતરી(pError);
              wMask |= wType;
            }નહીંતર{
              Tcl_SetObjResult(interp, pError);
              Tcl_Decr_Ref_ગણતરી(pError);
              પાછા TCL_ERROR;
            }
          }નહીંતર{
            પસંદગી( (ગણના TTYPE_enum)ttype ){
              સ્થિતિ TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    તોડો;
              સ્થિતિ TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; તોડો;
              સ્થિતિ TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     તોડો;
              સ્થિતિ TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   તોડો;
            }
          }
        }
      }નહીંતર{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      જો( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      જો( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }નહીંતર{
        pDb->zTraceV2 = 0;
      }
#જો !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      જો( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }નહીંતર{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#endif
    }
    તોડો;
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
  સ્થિતિ DB_TRANSACTION: {
    Tcl_Obj *pScript;
    અચલ અક્ષર *zBegin = "SAVEPOINT _tcl_transaction";
    જો( objc!=3 && objc!=4 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "[TYPE] SCRIPT");
      પાછા TCL_ERROR;
    }

    જો( pDb->nTransaction==0 && objc==4 ){
      સ્થિર અચલ અક્ષર *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      ગણના TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      પૂર્ણાંક ttype;
      જો( Tcl_GetIndexFromObj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        પાછા TCL_ERROR;
      }
      પસંદગી( (ગણના TTYPE_enum)ttype ){
        સ્થિતિ TTYPE_DEFERRED:    /* no-op */;                 તોડો;
        સ્થિતિ TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  તોડો;
        સ્થિતિ TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  તોડો;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    જો( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    જો( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (શૂન્ય)Tcl_NREvalObj(interp, pScript, 0);
    }નહીંતર{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    તોડો;
  }

  /*
  **    $db unlock_notify ?script?
  */
  સ્થિતિ DB_UNLOCK_NOTIFY: {
#ifndef SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_AppendResult(interp, "unlock_notify not available in this build",
                     (અક્ષર*)0);
    rc = TCL_ERROR;
#નહીંતર
    જો( objc!=2 && objc!=3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }નહીંતર{
      શૂન્ય (*xNotify)(શૂન્ય **, પૂર્ણાંક) = 0;
      શૂન્ય *pNotifyArg = 0;

      જો( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_ગણતરી(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      જો( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (શૂન્ય *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_ગણતરી(pDb->pUnlockNotify);
      }

      જો( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (અક્ષર*)0);
        rc = TCL_ERROR;
      }
    }
#endif
    તોડો;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  સ્થિતિ DB_PREUPDATE: {
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_AppendResult(interp, "preupdate_hook was omitted at compile-time", 
                     (અક્ષર*)0);
    rc = TCL_ERROR;
#નહીંતર
    સ્થિર અચલ અક્ષર *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    ગણના DbPreupdateSubCmd {
      PRE_ગણતરી, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    પૂર્ણાંક iSub;

    જો( objc<3 ){
      Tcl_Wrong_અંક_Args(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    જો( Tcl_GetIndexFromObj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      પાછા TCL_ERROR;
    }

    પસંદગી( (ગણના DbPreupdateSubCmd)iSub ){
      સ્થિતિ PRE_ગણતરી: {
        પૂર્ણાંક nCol = sqlite3_preupdate_ગણતરી(pDb->db);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nCol));
        તોડો;
      }

      સ્થિતિ PRE_HOOK: {
        જો( objc>4 ){
          Tcl_Wrong_અંક_Args(interp, 2, objv, "hook ?SCRIPT?");
          પાછા TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        તોડો;
      }

      સ્થિતિ PRE_DEPTH: {
        Tcl_Obj *pRet;
        જો( objc!=3 ){
          Tcl_Wrong_અંક_Args(interp, 3, objv, "");
          પાછા TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_SetObjResult(interp, pRet);
        તોડો;
      }

      સ્થિતિ PRE_NEW:
      સ્થિતિ PRE_OLD: {
        પૂર્ણાંક iIdx;
        sqlite3_કિંમત *p_કિંમત;
        જો( objc!=4 ){
          Tcl_Wrong_અંક_Args(interp, 3, objv, "INDEX");
          પાછા TCL_ERROR;
        }
        જો( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          પાછા TCL_ERROR;
        }

        જો( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_કિંમત);
        }નહીંતર{
          પ્રતિજ્ઞા( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_કિંમત);
        }

        જો( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((અક્ષર*)sqlite3_કિંમત_text(p_કિંમત), -1);
          Tcl_SetObjResult(interp, pObj);
        }નહીંતર{
          Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (અક્ષર*)0);
          પાછા TCL_ERROR;
        }
      }
    }
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
    તોડો;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  સ્થિતિ DB_WAL_HOOK:
  સ્થિતિ DB_UPDATE_HOOK:
  સ્થિતિ DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    જો( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    જો( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    જો( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    જો( objc>3 ){
       Tcl_Wrong_અંક_Args(interp, 2, objv, "?SCRIPT?");
       પાછા TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    તોડો;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  સ્થિતિ DB_VERSION: {
    પૂર્ણાંક i;
    માટે(i=2; i<objc; i++){
      અચલ અક્ષર *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      જો( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        જો( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          પાછા TCL_ERROR;
        }
      }નહીંતર

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      જો( strcmp(zArg, "-last-stmt-ptr")==0 ){
        બાહ્ય અચલ અક્ષર *sqlite3TestPtrToText(શૂન્ય*);
        શૂન્ય *ptr = (શૂન્ય*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_SetResult(interp, (અક્ષર*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }નહીંતર
#endif /* SQLITE_TEST */
      {
        Tcl_AppendResult(interp, "unknown argument: ", zArg, (અક્ષર*)0);
        પાછા TCL_ERROR;
      }
    }
    જો( i==2 ){   
      Tcl_SetResult(interp, (અક્ષર *)sqlite3_libversion(), TCL_STATIC);
    }
    તોડો;
  }


  } /* End of the SWITCH statement */
  પાછા rc;
}

#જો SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI DbObjCmdAdaptor(
  શૂન્ય *cd,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *અચલ*objv
){
  પાછા Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
સ્થિર પૂર્ણાંક sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *અચલ*objv
){
  Tcl_Wrong_અંક_Args(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  પાછા TCL_ERROR;
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI DbMain(
  શૂન્ય *cd,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *અચલ*objv
){
  SqliteDb *p;
  અચલ અક્ષર *zArg;
  અક્ષર *z_Err_સંદેશ;
  પૂર્ણાંક i;
  અચલ અક્ષર *zFile = 0;
  અચલ અક્ષર *zVfs = 0;
  પૂર્ણાંક flags;
  પૂર્ણાંક bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  પૂર્ણાંક rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#નહીંતર
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  જો( objc==1 ) પાછા sqliteCmdUsage(interp, objv);
  જો( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    જો( strcmp(zArg,"-version")==0 ){
      Tcl_AppendResult(interp,sqlite3_libversion(), (અક્ષર*)0);
      પાછા TCL_OK;
    }
    જો( strcmp(zArg,"-sourceid")==0 ){
      Tcl_AppendResult(interp,sqlite3_sourceid(), (અક્ષર*)0);
      પાછા TCL_OK;
    }
    જો( strcmp(zArg,"-has-codec")==0 ){
      Tcl_AppendResult(interp,"0",(અક્ષર*)0);
      પાછા TCL_OK;
    }
    જો( zArg[0]=='-' ) પાછા sqliteCmdUsage(interp, objv);
  }
  માટે(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    જો( zArg[0]!='-' ){
      જો( zFile!=0 ) પાછા sqliteCmdUsage(interp, objv);
      zFile = zArg;
      ચાલુ;
    }
    જો( i==objc-1 ) પાછા sqliteCmdUsage(interp, objv);
    i++;
    જો( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }નહીંતર જો( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }નહીંતર જો( strcmp(zArg, "-readonly")==0 ){
      પૂર્ણાંક b;
      જો( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) પાછા TCL_ERROR;
      જો( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }નહીંતર{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }નહીંતર જો( strcmp(zArg, "-create")==0 ){
      પૂર્ણાંક b;
      જો( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) પાછા TCL_ERROR;
      જો( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }નહીંતર{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }નહીંતર જો( strcmp(zArg, "-nofollow")==0 ){
      પૂર્ણાંક b;
      જો( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) પાછા TCL_ERROR;
      જો( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }નહીંતર{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }નહીંતર જો( strcmp(zArg, "-nomutex")==0 ){
      પૂર્ણાંક b;
      જો( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) પાછા TCL_ERROR;
      જો( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }નહીંતર{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }નહીંતર જો( strcmp(zArg, "-fullmutex")==0 ){
      પૂર્ણાંક b;
      જો( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) પાછા TCL_ERROR;
      જો( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }નહીંતર{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }નહીંતર જો( strcmp(zArg, "-uri")==0 ){
      પૂર્ણાંક b;
      જો( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) પાછા TCL_ERROR;
      જો( b ){
        flags |= SQLITE_OPEN_URI;
      }નહીંતર{
        flags &= ~SQLITE_OPEN_URI;
      }
    }નહીંતર જો( strcmp(zArg, "-translatefilename")==0 ){
      જો( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        પાછા TCL_ERROR;
      }
    }નહીંતર{
      Tcl_AppendResult(interp, "unknown option: ", zArg, (અક્ષર*)0);
      પાછા TCL_ERROR;
    }
  }
  z_Err_સંદેશ = 0;
  p = (SqliteDb*)Tcl_Alloc( કદ(*p) );
  memset(p, 0, કદ(*p));
  જો( zFile==0 ) zFile = "";
  જો( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  જો( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  જો( p->db ){
    જો( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_સંદેશ = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }નહીંતર{
    z_Err_સંદેશ = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  જો( p->db==0 ){
    Tcl_SetResult(interp, z_Err_સંદેશ, TCL_VOLATILE);
    Tcl_Free((અક્ષર*)p);
    sqlite3_free(z_Err_સંદેશ);
    પાછા TCL_ERROR;
  }
  p->maxStmt = અંક_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  જો( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (અક્ષર*)p, DbDeleteCmd);
  }નહીંતર{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (અક્ષર*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  પાછા TCL_OK;
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
EXTERN પૂર્ણાંક Sqlite3_Init(Tcl_Interp *interp){
  પૂર્ણાંક rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  જો( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  પાછા rc;
}
EXTERN પૂર્ણાંક Tclsqlite3_Init(Tcl_Interp *interp){ પાછા Sqlite3_Init(interp); }
EXTERN પૂર્ણાંક Sqlite3_Unload(Tcl_Interp *interp, પૂર્ણાંક flags){ પાછા TCL_OK; }
EXTERN પૂર્ણાંક Tclsqlite3_Unload(Tcl_Interp *interp, પૂર્ણાંક flags){ પાછા TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN પૂર્ણાંક Sqlite3_SafeInit(Tcl_Interp *interp){ પાછા TCL_ERROR; }
EXTERN પૂર્ણાંક Sqlite3_SafeUnload(Tcl_Interp *interp, પૂર્ણાંક flags){પાછા TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN પૂર્ણાંક Sqlite_Init(Tcl_Interp *interp){ પાછા Sqlite3_Init(interp);}
EXTERN પૂર્ણાંક Tclsqlite_Init(Tcl_Interp *interp){ પાછા Sqlite3_Init(interp); }
EXTERN પૂર્ણાંક Sqlite_Unload(Tcl_Interp *interp, પૂર્ણાંક flags){ પાછા TCL_OK; }
EXTERN પૂર્ણાંક Tclsqlite_Unload(Tcl_Interp *interp, પૂર્ણાંક flags){ પાછા TCL_OK; }
EXTERN પૂર્ણાંક Sqlite_SafeInit(Tcl_Interp *interp){ પાછા TCL_ERROR; }
EXTERN પૂર્ણાંક Sqlite_SafeUnload(Tcl_Interp *interp, પૂર્ણાંક flags){પાછા TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN પૂર્ણાંક sqlite3_Init(Tcl_Interp *interp){ પાછા Sqlite3_Init(interp);}
EXTERN પૂર્ણાંક sqlite_Init(Tcl_Interp *interp){ પાછા Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#જો defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
સ્થિર અચલ અક્ષર *tclsh_main_loop(શૂન્ય){
  સ્થિર અચલ અક્ષર zMainloop[] =
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
  પાછા zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN મુખ્ય
#endif
પૂર્ણાંક SQLITE_CDECL TCLSH_MAIN(પૂર્ણાંક argc, અક્ષર **argv){
  Tcl_Interp *interp;
  પૂર્ણાંક i;
  અચલ અક્ષર *zScript = 0;
  અક્ષર zArgc[32];
#જો defined(TCLSH_INIT_PROC)
  બાહ્ય અચલ અક્ષર *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#જો !defined(_WIN32_WCE)
  જો( getenv("SQLITE_DEBUG_BREAK") ){
    જો( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }નહીંતર{
#જો defined(_WIN32) || defined(WIN32)
      DebugBreak();
#અથવાજો defined(SIGTRAP)
      ઉઠાવો(SIGTRAP);
#endif
    }
  }
#endif

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_SetSystemEncoding(ખાલી, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(કદ(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  માટે(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_કિંમત);
  }
#જો defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  જો( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  જો( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    અચલ અક્ષર *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    જો( zInfo==0 ) zInfo = Tcl_GetStringResult(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    પાછા 1;
  }
  પાછા 0;
}
#endif /* TCLSH */
