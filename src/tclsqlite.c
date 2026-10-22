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
#اگر defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#اگر defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#وگرنه
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#اگر TCL_MAJOR_VERSION==9
# define CONST ثابت
#وگرنهاگر !defined(Tcl_Size)
  تعریفنوع صحیح Tcl_Size;
# ifndef Tcl_Bounce_Ref_شمار
#  define Tcl_Bounce_Ref_شمار(X) Tcl_Incr_Ref_شمار(X); Tcl_Decr_Ref_شمار(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#endif
/**** End copy of tclsqlite.h ****/

#شامل <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ifndef SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  تعریفنوع unsigned نویسه u8;
# ifndef SQLITE_PTRSIZE
#   اگر defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   وگرنه
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# اگر defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    تعریفنوع uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    تعریفنوع unsigned صحیح uptr;
# وگرنه
    تعریفنوع sqlite3_uint64 uptr;
# endif
#endif
#شامل <ctype.h>

/* Used to get the current process ID */
#اگر !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#وگرنهاگر !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (صحیح)GetCurrentProcessId
#endif

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ifdef BUILD_sqlite
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_sqlite */

#define رقم_PREPARED_STMTS 10
#define MAX_PREPARED_STMTS 100

/* Forward declaration */
تعریفنوع ساختار SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#اگر defined(SQLITE_ENABLE_QRF_IN_TCL)
#شامل "qrf.h"
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
تعریفنوع ساختار SqlFunc SqlFunc;
ساختار SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  صحیح useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  صحیح eType;            /* Type of value to return */
  نویسه *zName;          /* Name of this function */
  SqlFunc *pNext;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
تعریفنوع ساختار SqlCollate SqlCollate;
ساختار SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  نویسه *zScript;        /* The script to be run */
  SqlCollate *pNext;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
تعریفنوع ساختار SqlPreparedStmt SqlPreparedStmt;
ساختار SqlPreparedStmt {
  SqlPreparedStmt *pNext;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  صحیح nSql;                /* chars in zSql[] */
  ثابت نویسه *zSql;        /* Text of the SQL statement */
  صحیح nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

تعریفنوع ساختار IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
ساختار SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  نویسه *zBusy;               /* The busy callback routine */
  نویسه *zCommit;             /* The commit hook callback routine */
  نویسه *zTrace;              /* The trace callback routine */
  نویسه *zTraceV2;            /* The trace_v2 callback routine */
  نویسه *zProfile;            /* The profile callback routine */
  نویسه *zProgress;           /* The progress callback routine */
  نویسه *zBindFallback;       /* Callback to invoke on a binding miss */
  نویسه *zAuth;               /* The authorization callback routine */
  صحیح disableAuth;           /* Disable the authorizer if it exists */
  نویسه *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  صحیح rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  صحیح maxStmt;               /* The next maximum number of stmtList */
  صحیح nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  صحیح nStep, nSort, nIndex;  /* Statistics for most recent operation */
  صحیح nVMStep;               /* Another statistic for most recent operation */
  صحیح nTransaction;          /* Number of nested [transaction] methods */
  صحیح openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  صحیح nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  صحیح bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

ساختار IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned صحیح isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *pNext;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
ایستا صحیح strlen30(ثابت نویسه *z){
  ثابت نویسه *z2 = z;
  وقتی( *z2 ){ z2++; }
  بازگشت 0x3fffffff & (صحیح)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
ایستا تهی closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *pNext;

  برای(p=pDb->pIncrblob; p; p=pNext){
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
ایستا صحیح SQLITE_TCLAPI incrblobClose2(
  ClientData instanceData,
  Tcl_Interp *interp,
  صحیح flags
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  صحیح  rc;
  sqlite3 *db = p->pDb->db;

  اگر( flags ){
    p->isClosed |= flags;
    بازگشت TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  اگر( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  اگر( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }
  اگر( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->pNext;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((نویسه *)p);

  اگر( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (نویسه *)sqlite3_errmsg(db), TCL_VOLATILE);
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}
ایستا صحیح SQLITE_TCLAPI incrblobClose(
  ClientData instanceData,
  Tcl_Interp *interp
){
  بازگشت incrblobClose2(instanceData, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
ایستا صحیح SQLITE_TCLAPI incrblob_درون_داد(
  ClientData instanceData,
  نویسه *buf,
  صحیح bufSize,
  صحیح *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nRead = bufSize;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  صحیح rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  اگر( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  اگر( nRead<=0 ){
    بازگشت 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (تهی *)buf, (صحیح)nRead, (صحیح)p->iSeek);
  اگر( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    بازگشت -1;
  }

  p->iSeek += nRead;
  بازگشت nRead;
}

/*
** Write data to an incremental blob channel.
*/
ایستا صحیح SQLITE_TCLAPI incrblobOutput(
  ClientData instanceData,
  ثابت نویسه *buf,
  صحیح toWrite,
  صحیح *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  صحیح rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  اگر( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    بازگشت -1;
  }
  اگر( nWrite<=0 ){
    بازگشت 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (تهی*)buf,(صحیح)nWrite, (صحیح)p->iSeek);
  اگر( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    بازگشت -1;
  }

  p->iSeek += nWrite;
  بازگشت nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#اگر TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#وگرنه
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
ایستا WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  ClientData instanceData,
  WideSeekProcType offset,
  صحیح seekMode,
  صحیح *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;

  انتخاب( seekMode ){
    حالت SEEK_SET:
      p->iSeek = offset;
      بشکن;
    حالت SEEK_CUR:
      p->iSeek += offset;
      بشکن;
    حالت SEEK_END:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      بشکن;

    پیشفرض: أكد(!"Bad seekMode");
  }

  بازگشت p->iSeek;
}
ایستا صحیح SQLITE_TCLAPI incrblobSeek(
  ClientData instanceData,
  long offset,
  صحیح seekMode,
  صحیح *errorCodePtr
){
  بازگشت incrblobWideSeek(instanceData,offset,seekMode,errorCodePtr);
}


ایستا تهی SQLITE_TCLAPI incrblobWatch(
  ClientData instanceData,
  صحیح mode
){
  /* NO-OP */
}
ایستا صحیح SQLITE_TCLAPI incrblobHandle(
  ClientData instanceData,
  صحیح dir,
  ClientData *hPtr
){
  بازگشت TCL_ERROR;
}

ایستا Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_VERSION_5,             /* version                              */
  incrblobClose,                     /* closeProc                            */
  incrblob_درون_داد,                     /* inputProc                            */
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
ایستا صحیح createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  ثابت نویسه *zDb,
  ثابت نویسه *zTable,
  ثابت نویسه *zColumn,
  sqlite_int64 iRow,
  صحیح isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  صحیح rc;
  صحیح flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  ایستا صحیح شمار = 0;
  نویسه zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  اگر( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (نویسه *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    بازگشت TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(اندازه(IncrblobChannel));
  memset(p, 0, اندازه(*p));
  p->pBlob = pBlob;
  اگر( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(اندازه(zChannel), zChannel, "incrblob_%d", ++شمار);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->pNext = pDb->pIncrblob;
  p->pPrev = 0;
  اگر( p->pNext ){
    p->pNext->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_SetResult(interp, (نویسه *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  بازگشت TCL_OK;
}
#وگرنه  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
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
ایستا صحیح safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  ثابت نویسه *z;
  Tcl_Size n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  وقتی( n-- > 0 ){
    صحیح c = *(z++);
    اگر( c=='$' || c=='[' || c==';' ) بازگشت 0;
  }
  بازگشت 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
ایستا SqlFunc *findSqlFunc(SqliteDb *pDb, ثابت نویسه *zName){
  SqlFunc *p, *pNew;
  صحیح nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( اندازه(*pNew) + nName + 1 );
  pNew->zName = (نویسه*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  برای(p=pDb->pFunc; p; p=p->pNext){
    اگر( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((نویسه*)pNew);
      بازگشت p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->pNext = pDb->pFunc;
  pDb->pFunc = pNew;
  بازگشت pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
ایستا تهی dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  اگر( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((نویسه *)pStmt->zSql);
  }
#endif
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((نویسه *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
ایستا تهی flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *pNext;

  برای(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pNext){
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
ایستا تهی addDatabaseRef(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
ایستا تهی delDatabaseRef(SqliteDb *pDb){
  أكد( pDb->nRef>0 );
  pDb->nRef--;
  اگر( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    وقتی( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->pNext;
      أكد( pFunc->pDb==pDb );
      Tcl_Decr_Ref_شمار(pFunc->pScript);
      Tcl_Free((نویسه*)pFunc);
    }
    وقتی( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->pNext;
      Tcl_Free((نویسه*)pCollate);
    }
    اگر( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    اگر( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    اگر( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    اگر( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    اگر( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    اگر( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    اگر( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    اگر( pDb->pUpdateHook ){
      Tcl_Decr_Ref_شمار(pDb->pUpdateHook);
    }
    اگر( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_شمار(pDb->pPreUpdateHook);
    }
    اگر( pDb->pRollbackHook ){
      Tcl_Decr_Ref_شمار(pDb->pRollbackHook);
    }
    اگر( pDb->pWalHook ){
      Tcl_Decr_Ref_شمار(pDb->pWalHook);
    }
    اگر( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_شمار(pDb->pCollateNeeded);
    }
    Tcl_Free((نویسه*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
ایستا تهی SQLITE_TCLAPI DbDeleteCmd(تهی *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
ایستا صحیح DbBusyHandler(تهی *cd, صحیح nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحیح rc;
  نویسه zVal[30];

  sqlite3_snprintf(اندازه(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (نویسه*)0);
  اگر( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    بازگشت 0;
  }
  بازگشت 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
ایستا صحیح DbProgressHandler(تهی *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحیح rc;

  أكد( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  اگر( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    بازگشت 1;
  }
  بازگشت 0;
}
#endif

#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
ایستا تهی DbTraceHandler(تهی *cd, ثابت نویسه *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_مقدار(&str));
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
ایستا صحیح DbTraceV2Handler(
  unsigned type, /* One of the SQLITE_TRACE_* event types. */
  تهی *cd,      /* The original context data pointer. */
  تهی *pd,      /* Primary event data, depends on event type. */
  تهی *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  انتخاب( type ){
    حالت SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      نویسه *zSql = (نویسه *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_شمار(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_شمار(pCmd);
      Tcl_ResetResult(pDb->interp);
      بشکن;
    }
    حالت SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_شمار(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_شمار(pCmd);
      Tcl_ResetResult(pDb->interp);
      بشکن;
    }
    حالت SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_شمار(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_شمار(pCmd);
      Tcl_ResetResult(pDb->interp);
      بشکن;
    }
    حالت SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_شمار(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_شمار(pCmd);
      Tcl_ResetResult(pDb->interp);
      بشکن;
    }
  }
  بازگشت SQLITE_OK;
}
#endif

#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
ایستا تهی DbProfileHandler(تهی *cd, ثابت نویسه *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  نویسه zTm[100];

  sqlite3_snprintf(اندازه(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_مقدار(&str));
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
ایستا صحیح DbCommitHandler(تهی *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحیح rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  اگر( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    بازگشت 1;
  }
  بازگشت 0;
}

ایستا تهی DbRollbackHandler(تهی *clientData){
  SqliteDb *pDb = (SqliteDb*)clientData;
  أكد(pDb->pRollbackHook);
  اگر( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
ایستا صحیح DbWalHandler(
  تهی *clientData,
  sqlite3 *db,
  ثابت نویسه *zDb,
  صحیح nEntry
){
  صحیح ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)clientData;
  Tcl_Interp *interp = pDb->interp;
  أكد(pDb->pWalHook);

  أكد( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_شمار(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  اگر( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_شمار(p);

  بازگشت ret;
}

#اگر defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ایستا تهی setTestUnlockNotifyVars(Tcl_Interp *interp, صحیح iArg, صحیح nArg){
  نویسه zBuf[64];
  sqlite3_snprintf(اندازه(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(اندازه(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#وگرنه
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
ایستا تهی DbUnlockNotify(تهی **apArg, صحیح nArg){
  صحیح i;
  برای(i=0; i<nArg; i++){
    ثابت صحیح flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    setTestUnlockNotifyVars(pDb->interp, i, nArg);
    أكد( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_شمار(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
ایستا تهی DbPreUpdateHandler(
  تهی *p,
  sqlite3 *db,
  صحیح op,
  ثابت نویسه *zDb,
  ثابت نویسه *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ایستا ثابت نویسه *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  أكد( (SQLITE_DELETE-1)/9 == 0 );
  أكد( (SQLITE_درج-1)/9 == 1 );
  أكد( (SQLITE_UPDATE-1)/9 == 2 );
  أكد( pDb->pPreUpdateHook );
  أكد( db==pDb->db );
  أكد( op==SQLITE_درج || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_شمار(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_شمار(pCmd);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

ایستا تهی DbUpdateHandler(
  تهی *p,
  صحیح op,
  ثابت نویسه *zDb,
  ثابت نویسه *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ایستا ثابت نویسه *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  أكد( (SQLITE_DELETE-1)/9 == 0 );
  أكد( (SQLITE_درج-1)/9 == 1 );
  أكد( (SQLITE_UPDATE-1)/9 == 2 );

  أكد( pDb->pUpdateHook );
  أكد( op==SQLITE_درج || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_شمار(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_شمار(pCmd);
}

ایستا تهی tclCollateNeeded(
  تهی *pCtx,
  sqlite3 *db,
  صحیح enc,
  ثابت نویسه *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_شمار(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_شمار(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
ایستا صحیح tclSqlCollate(
  تهی *pCtx,
  صحیح nA,
  ثابت تهی *zA,
  صحیح nB,
  ثابت تهی *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_شمار(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_شمار(pCmd);
  بازگشت (atoi(Tcl_GetStringResult(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
ایستا تهی tclSqlFunc(sqlite3_context *context, صحیح argc, sqlite3_مقدار**argv){
  SqlFunc *p = sqlite3_user_data(context);
  Tcl_Obj *pCmd;
  صحیح i;
  صحیح rc;

  اگر( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_شمار(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_شمار(pCmd);
  }وگرنه{
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
    اگر( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
      بازگشت;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_شمار(pCmd);
    برای(i=0; i<argc; i++){
      sqlite3_مقدار *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      انتخاب( sqlite3_مقدار_type(pIn) ){
        حالت SQLITE_BLOB: {
          صحیح bytes = sqlite3_مقدار_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_مقدار_blob(pIn), bytes);
          بشکن;
        }
        حالت SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_مقدار_int64(pIn);
          اگر( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((صحیح)v);
          }وگرنه{
            pVal = Tcl_NewWideIntObj(v);
          }
          بشکن;
        }
        حالت SQLITE_FLOAT: {
          دوگانه r = sqlite3_مقدار_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          بشکن;
        }
        حالت SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          بشکن;
        }
        پیشفرض: {
          صحیح bytes = sqlite3_مقدار_bytes(pIn);
          pVal = Tcl_NewStringObj((نویسه *)sqlite3_مقدار_text(pIn), bytes);
          بشکن;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      اگر( rc ){
        Tcl_Decr_Ref_شمار(pCmd);
        sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
        بازگشت;
      }
    }
    اگر( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_Decr_Ref_شمار(pCmd);
  }

  اگر( TCL_BREAK==rc ){
    sqlite3_result_null(context);
  }وگرنه اگر( rc && rc!=TCL_RETURN ){
    sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
  }وگرنه{
    Tcl_Obj *pVar = Tcl_GetObjResult(p->interp);
    Tcl_Size n;
    u8 *data;
    ثابت نویسه *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    نویسه c = zType[0];
    صحیح eType = p->eType;

    اگر( eType==SQLITE_NULL ){
      اگر( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }وگرنه اگر( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }وگرنه اگر( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }وگرنه{
        eType = SQLITE_TEXT;
      }
    }

    انتخاب( eType ){
      حالت SQLITE_BLOB: {
        data = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_result_blob64(context, data, n, SQLITE_TRANSIENT);
        بشکن;
      }
      حالت SQLITE_INTEGER: {
        Tcl_WideInt v;
        اگر( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_result_int64(context, v);
          بشکن;
        }
        /* fall-through */
      }
      حالت SQLITE_FLOAT: {
        دوگانه r;
        اگر( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_result_double(context, r);
          بشکن;
        }
        /* fall-through */
      }
      پیشفرض: {
        data = (unsigned نویسه *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_result_text64(context, (نویسه *)data, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        بشکن;
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
ایستا صحیح auth_callback(
  تهی *pArg,
  صحیح code,
  ثابت نویسه *zArg1,
  ثابت نویسه *zArg2,
  ثابت نویسه *zArg3,
  ثابت نویسه *zArg4
){
  ثابت نویسه *zCode;
  Tcl_DString str;
  صحیح rc;
  ثابت نویسه *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  اگر( pDb->disableAuth ) بازگشت SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  انتخاب( code ){
    حالت SQLITE_COPY              : zCode="SQLITE_COPY"; بشکن;
    حالت SQLITE_CREATE_INDEX      : zCode="SQLITE_CREATE_INDEX"; بشکن;
    حالت SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; بشکن;
    حالت SQLITE_CREATE_TEMP_INDEX : zCode="SQLITE_CREATE_TEMP_INDEX"; بشکن;
    حالت SQLITE_CREATE_TEMP_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; بشکن;
    حالت SQLITE_CREATE_TEMP_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; بشکن;
    حالت SQLITE_CREATE_TEMP_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; بشکن;
    حالت SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; بشکن;
    حالت SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; بشکن;
    حالت SQLITE_DELETE            : zCode="SQLITE_DELETE"; بشکن;
    حالت SQLITE_DROP_INDEX        : zCode="SQLITE_DROP_INDEX"; بشکن;
    حالت SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; بشکن;
    حالت SQLITE_DROP_TEMP_INDEX   : zCode="SQLITE_DROP_TEMP_INDEX"; بشکن;
    حالت SQLITE_DROP_TEMP_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; بشکن;
    حالت SQLITE_DROP_TEMP_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; بشکن;
    حالت SQLITE_DROP_TEMP_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; بشکن;
    حالت SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; بشکن;
    حالت SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; بشکن;
    حالت SQLITE_درج            : zCode="SQLITE_INSERT"; بشکن;
    حالت SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; بشکن;
    حالت SQLITE_READ              : zCode="SQLITE_READ"; بشکن;
    حالت SQLITE_SELECT            : zCode="SQLITE_SELECT"; بشکن;
    حالت SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; بشکن;
    حالت SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; بشکن;
    حالت SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; بشکن;
    حالت SQLITE_DETACH            : zCode="SQLITE_DETACH"; بشکن;
    حالت SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; بشکن;
    حالت SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; بشکن;
    حالت SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; بشکن;
    حالت SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; بشکن;
    حالت SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; بشکن;
    حالت SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; بشکن;
    حالت SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; بشکن;
    حالت SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; بشکن;
    پیشفرض                       : zCode="????"; بشکن;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, zCode);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_مقدار(&str));
  Tcl_DStringFree(&str);
  zReply = rc==TCL_OK ? Tcl_GetStringResult(pDb->interp) : "SQLITE_DENY";
  اگر( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }وگرنه اگر( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }وگرنه اگر( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }وگرنه{
    rc = 999;
  }
  بازگشت rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#اگر 0
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
ایستا نویسه *local_getline(نویسه *zPrompt, FILE *in){
  نویسه *z_سطر;
  صحیح n_سطر;
  صحیح n;

  n_سطر = 100;
  z_سطر = malloc( n_سطر );
  اگر( z_سطر==0 ) بازگشت 0;
  n = 0;
  وقتی( 1 ){
    اگر( n+100>n_سطر ){
      n_سطر = n_سطر*2 + 100;
      z_سطر = realloc(z_سطر, n_سطر);
      اگر( z_سطر==0 ) بازگشت 0;
    }
    اگر( fgets(&z_سطر[n], n_سطر - n, in)==0 ){
      اگر( n==0 ){
        آزاد(z_سطر);
        بازگشت 0;
      }
      z_سطر[n] = 0;
      بشکن;
    }
    وقتی( z_سطر[n] ){ n++; }
    اگر( n>0 && z_سطر[n-1]=='\n' ){
      n--;
      z_سطر[n] = 0;
      بشکن;
    }
  }
  z_سطر = realloc( z_سطر, n+1 );
  بازگشت z_سطر;
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
ایستا صحیح SQLITE_TCLAPI DbTransPostCmd(
  ClientData data[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  صحیح result                           /* Result of evaluating SCRIPT */
){
  ایستا ثابت نویسه *ثابت azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)data[0];
  صحیح rc = result;
  ثابت نویسه *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  اگر( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    اگر( rc!=TCL_ERROR ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (نویسه*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  بازگشت rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
ایستا صحیح dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  ثابت نویسه *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  ثابت نویسه **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned صحیح prepFlags = 0;
#ifdef SQLITE_TEST
  اگر( pDb->bLegacyPrepare ){
    بازگشت sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  اگر( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  بازگشت sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
ایستا صحیح dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  نویسه ثابت *zIn,                /* SQL to compile */
  نویسه ثابت **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  ثابت نویسه *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  صحیح nSql;                       /* Length of zSql in bytes */
  صحیح nVar = 0;                   /* Number of variables in statement */
  صحیح iParm = 0;                  /* Next free entry in apParm */
  نویسه c;
  صحیح i;
  صحیح needResultReset = 0;        /* Need to invoke Tcl_ResetResult() */
  صحیح rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  وقتی( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  برای(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->pNext){
    صحیح n = pPreStmt->nSql;
    اگر( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      اگر( pPreStmt->pPrev ){
        pPreStmt->pPrev->pNext = pPreStmt->pNext;
      }وگرنه{
        pDb->stmtList = pPreStmt->pNext;
      }
      اگر( pPreStmt->pNext ){
        pPreStmt->pNext->pPrev = pPreStmt->pPrev;
      }وگرنه{
        pDb->stmtLast = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_شمار(pStmt);
      بشکن;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  اگر( pPreStmt==0 ){
    صحیح nByte;

    اگر( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      بازگشت TCL_ERROR;
    }
    اگر( pStmt==0 ){
      اگر( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        بازگشت TCL_ERROR;
      }وگرنه{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        بازگشت TCL_OK;
      }
    }

    أكد( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_شمار(pStmt);
    nByte = اندازه(SqlPreparedStmt) + nVar*اندازه(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (صحیح)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    اگر( pPreStmt->zSql==0 ){
      نویسه *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      memcpy(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#endif
  }
  أكد( pPreStmt );
  أكد( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  أكد( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  برای(i=1; i<=nVar; i++){
    ثابت نویسه *zVar = sqlite3_bind_parameter_name(pStmt, i);
    اگر( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      اگر( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        صحیح rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_شمار(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        اگر( needResultReset ) Tcl_ResetResult(interp);
        needResultReset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_Decr_Ref_شمار(pCmd);
        اگر( rx==TCL_OK ){
          pVar = Tcl_GetObjResult(interp);
        }وگرنه اگر( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          بشکن;
        }وگرنه{
          pVar = 0;
        }
      }
      اگر( pVar ){
        Tcl_Size n;
        u8 *data;
        ثابت نویسه *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        اگر( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          data = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, data, n, SQLITE_STATIC);
          Tcl_Incr_Ref_شمار(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }وگرنه اگر( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          صحیح nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }وگرنه اگر( c=='d' && strcmp(zType,"double")==0 ){
          دوگانه r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }وگرنه اگر( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }وگرنه{
          data = (unsigned نویسه *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (نویسه *)data, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_شمار(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }وگرنه{
        sqlite3_bind_null(pStmt, i);
      }
      اگر( needResultReset ) Tcl_ResetResult(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  اگر( needResultReset && rc==TCL_OK ) Tcl_ResetResult(pDb->interp);

  بازگشت rc;
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
ایستا تهی dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  صحیح discard                     /* True to delete (not cache) the pPreStmt */
){
  صحیح i;

  /* Free the bound string and blob parameters */
  برای(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_شمار(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  اگر( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }وگرنه{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->pNext = pDb->stmtList;
    pPreStmt->pPrev = 0;
    اگر( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    اگر( pDb->stmtLast==0 ){
      أكد( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }وگرنه{
      أكد( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    وقتی( pDb->nStmt>pDb->maxStmt ){
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
تعریفنوع ساختار DbEvalContext DbEvalContext;
ساختار DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  ثابت نویسه *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  صحیح nCol;                       /* Number of columns returned by pStmt */
  صحیح evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#define SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#define SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
ایستا تهی dbReleaseColumnNames(DbEvalContext *p){
  اگر( p->apColName ){
    صحیح i;
    برای(i=0; i<p->nCol; i++){
      Tcl_Decr_Ref_شمار(p->apColName[i]);
    }
    Tcl_Free((نویسه *)p->apColName);
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
ایستا تهی dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  صحیح evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, اندازه(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_شمار(pSql);
  اگر( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_شمار(pVarName);
  }
  p->evalFlags = evalFlags;
  addDatabaseRef(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
ایستا تهی dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  صحیح *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  اگر( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    صحیح i;                        /* Iterator variable */
    صحیح nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_شمار(pStmt);
    اگر( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( اندازه(Tcl_Obj*)*nCol );
      برای(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_Incr_Ref_شمار(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    اگر( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_شمار(pColList);
      Tcl_Incr_Ref_شمار(pStar);
      برای(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      اگر( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }وگرنه{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, پوچ, 0);
        اگر( !pDict ){
          pDict = Tcl_NewDictObj();
        }وگرنه اگر( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        اگر( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, پوچ, pDict, 0);
        }
        Tcl_Bounce_Ref_شمار(pDict);
      }
      Tcl_Decr_Ref_شمار(pStar);
      Tcl_Decr_Ref_شمار(pColList);
    }
  }

  اگر( papColName ){
    *papColName = p->apColName;
  }
  اگر( pnCol ){
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
ایستا صحیح dbEvalStep(DbEvalContext *p){
  ثابت نویسه *zPrevSql = 0;       /* Previous value of p->zSql */

  وقتی( p->zSql[0] || p->pPreStmt ){
    صحیح rc;
    اگر( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      اگر( rc!=TCL_OK ) بازگشت rc;
    }وگرنه{
      صحیح rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      اگر( rcs==SQLITE_ROW ){
        بازگشت TCL_OK;
      }
      اگر( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->nIndex = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      اگر( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#اگر SQLITE_TEST
        اگر( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          ادامه;
        }
#endif
        Tcl_SetObjResult(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        بازگشت TCL_ERROR;
      }وگرنه{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  بازگشت TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
ایستا تهی dbEvalFinalize(DbEvalContext *p){
  اگر( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  اگر( p->pVarName ){
    Tcl_Decr_Ref_شمار(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_شمار(p->pSql);
  dbReleaseColumnNames(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
ایستا Tcl_Obj *db_Eval_Column_مقدار(DbEvalContext *p, صحیح iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  انتخاب( sqlite3_column_type(pStmt, iCol) ){
    حالت SQLITE_BLOB: {
      صحیح bytes = sqlite3_column_bytes(pStmt, iCol);
      ثابت نویسه *zBlob = sqlite3_column_blob(pStmt, iCol);
      اگر( !zBlob ) bytes = 0;
      بازگشت Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    حالت SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      اگر( v>=-2147483647 && v<=2147483647 ){
        بازگشت Tcl_NewIntObj((صحیح)v);
      }وگرنه{
        بازگشت Tcl_NewWideIntObj(v);
      }
    }
    حالت SQLITE_FLOAT: {
      بازگشت Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    حالت SQLITE_NULL: {
      بازگشت Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  بازگشت Tcl_NewStringObj((نویسه*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#اگر TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
ایستا صحیح DbUseNre(تهی){
  صحیح major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  بازگشت( (major==8 && minor>=6) || major>8 );
}
#وگرنه
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (تهی)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (تهی)0
#endif

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
ایستا صحیح SQLITE_TCLAPI DbEvalNextCmd(
  ClientData data[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  صحیح result                           /* Result so far */
){
  صحیح rc = result;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)data[0];
  Tcl_Obj * ثابت pScript = (Tcl_Obj *)data[1];
  Tcl_Obj * ثابت pVarName = p->pVarName;

  وقتی( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    صحیح i;
    صحیح nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    برای(i=0; i<nCol; i++){
      اگر( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, db_Eval_Column_مقدار(p,i), 0);
      }وگرنه اگر( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        اگر( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }وگرنه{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, پوچ, 0);
          اگر( pDict ){
            اگر( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            اگر( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, پوچ, pDict, 0);
            }
            Tcl_Bounce_Ref_شمار(pDict);
          }
        }
      }وگرنه اگر( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       db_Eval_Column_مقدار(p,i), 0);
      }وگرنه{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, پوچ, 0);
        اگر( !pDict ){
          pDict = Tcl_NewDictObj();
        }وگرنه اگر( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        اگر( Tcl_DictObjPut(interp, pDict, apColName[i],
                           db_Eval_Column_مقدار(p,i))==TCL_OK ){
          Tcl_ObjSetVar2(interp, pVarName, پوچ, pDict, 0);
        }
        Tcl_Bounce_Ref_شمار(pDict);
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
    اگر( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbEvalNextCmd, (تهی*)p, (تهی*)pScript, 0, 0);
      بازگشت Tcl_NREvalObj(interp, pScript, 0);
    }وگرنه{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_شمار(pScript);
  dbEvalFinalize(p);
  Tcl_Free((نویسه *)p);

  اگر( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_ResetResult(interp);
    rc = TCL_OK;
  }
  بازگشت rc;
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
ایستا تهی DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  اگر( *ppHook ){
    Tcl_SetObjResult(interp, *ppHook);
    اگر( pArg ){
      Tcl_Decr_Ref_شمار(*ppHook);
      *ppHook = 0;
    }
  }
  اگر( pArg ){
    أكد( !(*ppHook) );
    اگر( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_شمار(*ppHook);
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
ایستا صحیح dbQrf(SqliteDb *pDb, صحیح objc, Tcl_Obj *ثابت*objv){
#ifndef SQLITE_QRF_H
  Tcl_SetResult(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  بازگشت TCL_ERROR;
#وگرنه
  نویسه *zResult = 0;             /* Result to be returned */
  ثابت نویسه *zSql = 0;          /* SQL to run */
  صحیح i;                         /* Loop counter */
  صحیح rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  ایستا ثابت نویسه *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  ایستا ثابت unsigned نویسه aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_چپ,
    QRF_ALIGN_میانه, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_راست,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_بالاترین,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, اندازه(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &zResult;
  برای(i=2; i<objc; i++){
    ثابت نویسه *zArg = Tcl_GetString(objv[i]);
    ثابت نویسه *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    ثابت unsigned نویسه aBoolMap[] = { 0, 2, 1, 2, 1 };
    اگر( zArg[0]!='-' ){
      اگر( zSql ){
        Tcl_AppendResult(pDb->interp, "unknown argument: ", zArg, (نویسه*)0);
        rc = TCL_ERROR;
        برو format_failed;
      }
      zSql  = zArg;
    }وگرنه اگر( i==objc-1 ){
      Tcl_AppendResult(pDb->interp, "option has no argument: ", zArg,(نویسه*)0);
      rc = TCL_ERROR;
      برو format_failed;
    }وگرنه اگر( strcmp(zArg,"-style")==0 ){
      ایستا ثابت نویسه *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      ایستا unsigned نویسه aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_شمار,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_درج,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_سطر,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      صحیح style;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      اگر( rc ) برو format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }وگرنه اگر( strcmp(zArg,"-esc")==0 ){
      ایستا ثابت نویسه *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      ایستا unsigned نویسه aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_نماد
      };
      صحیح esc;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      اگر( rc ) برو format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }وگرنه اگر( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      ایستا ثابت نویسه *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      ایستا unsigned نویسه aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      صحیح txt;
      صحیح k = zArg[2]=='e';
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      اگر( rc ) برو format_failed;
      اگر( k ){
        qrf.eText = aTextMap[txt];
      }وگرنه اگر( txt<=2 ){
        أكد( QRF_No==1 );
        أكد( QRF_Yes==2 );
        أكد( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }وگرنه{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }وگرنه اگر( strcmp(zArg,"-blob")==0 ){
      ایستا ثابت نویسه *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      ایستا unsigned نویسه aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_Size
      };
      صحیح blob;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      اگر( rc ) برو format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }وگرنه اگر( strcmp(zArg,"-wordwrap")==0 ){
      صحیح v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      اگر( rc ) برو format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }وگرنه اگر( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      صحیح v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      اگر( rc ) برو format_failed;
      اگر( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }وگرنه اگر( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }وگرنه اگر( zArg[1]=='r' ){
        qrf.b_Row_شمار = aBoolMap[v];
      }وگرنه{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }وگرنه اگر( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      صحیح ax = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      اگر( rc ) برو format_failed;
      اگر( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }وگرنه{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }وگرنه اگر( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      صحیح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      اگر( rc ) برو format_failed;
      اگر( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }وگرنه اگر( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      اگر( zArg[1]=='w' ){
        qrf.nWrap = v;
      }وگرنه اگر( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }وگرنه اگر( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }وگرنه{
        qrf.n_سطر_Limit = v;
      }
      i++;
    }وگرنه اگر( strcmp(zArg,"-charlimit")==0 ){
      صحیح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      اگر( rc ) برو format_failed;
      اگر( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }وگرنه اگر( strcmp(zArg,"-multiinsert")==0 ){
      صحیح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      اگر( rc ) برو format_failed;
      اگر( v<0 ) v = 0;
      qrf.n_Multi_درج = v;
      i++;
    }وگرنه اگر( strcmp(zArg,"-align")==0 ){
      Tcl_Size n = 0;
      صحیح jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      اگر( rc ) برو format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*اندازه(qrf.aAlign[0]) );
      اگر( qrf.aAlign==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (نویسه*)0);
        rc = TCL_ERROR;
        برو format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*اندازه(qrf.aAlign[0]));
      qrf.nAlign = n;
      برای(jj=0; jj<n; jj++){
        صحیح x;
        Tcl_Obj *pTerm;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        اگر( rc ) برو format_failed;
        rc = Tcl_GetIndexFromObj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        اگر( rc ) برو format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }وگرنه اگر( strcmp(zArg,"-widths")==0 ){
      Tcl_Size n = 0;
      صحیح jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      اگر( rc ) برو format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*اندازه(qrf.aWidth[0]) );
      اگر( qrf.aWidth==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (نویسه*)0);
        rc = TCL_ERROR;
        برو format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*اندازه(qrf.aWidth[0]));
      qrf.nWidth = n;
      برای(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        صحیح v;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        اگر( rc ) برو format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        اگر( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }وگرنه اگر( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short صحیح)v;
      }
      i++;
    }وگرنه اگر( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }وگرنه اگر( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }وگرنه اگر( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }وگرنه اگر( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }وگرنه اگر( strcmp(zArg,"-ifmt")==0 ){
      نویسه *zFmt = Tcl_GetString(objv[i+1]);
      اگر( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }وگرنه اگر( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_AppendResult(pDb->interp, "bad format for -ifmt", (نویسه*)0);
        rc = TCL_ERROR;
        برو format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }وگرنه اگر( strcmp(zArg,"-fpfmt")==0 ){
      نویسه *zFmt = Tcl_GetString(objv[i+1]);
      اگر( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }وگرنه اگر( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_AppendResult(pDb->interp, "bad format for -fpfmt", (نویسه*)0);
        rc = TCL_ERROR;
        برو format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }وگرنه اگر( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }وگرنه{
      Tcl_AppendResult(pDb->interp, "unknown option: ", zArg, (نویسه*)0);
      rc = TCL_ERROR;
      برو format_failed;
    }
  }
  وقتی( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    نویسه *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    اگر( rc ) برو format_failed;
    اگر( pStmt==0 ) ادامه;
    rc = sqlite3_format_query_result(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    اگر( rc ){
      Tcl_SetResult(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      برو format_failed;
    }
  }
  Tcl_SetResult(pDb->interp, zResult, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.aWidth);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(zResult);
  بازگشت rc;

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
ایستا صحیح SQLITE_TCLAPI DbObjCmd(
  تهی *cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحیح choice;
  صحیح rc = TCL_OK;
  ایستا ثابت نویسه *DB_strs[] = {
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
  شمارش DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_LOAD_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_INTERRUPT,            DB_LAST_درج_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_PROGRESS,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_STATUS,                DB_TIMEOUT,              DB_TOTAL_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_VERSION,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  اگر( objc<2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SUBCOMMAND ...");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    بازگشت TCL_ERROR;
  }

  انتخاب( (شمارش DB_enum)choice ){

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
  حالت DB_AUTHORIZER: {
#ifdef SQLITE_OMIT_AUTHORIZATION
    Tcl_AppendResult(interp, "authorization not available in this build",
                     (نویسه*)0);
    بازگشت TCL_ERROR;
#وگرنه
    اگر( objc>3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?CALLBACK?");
      بازگشت TCL_ERROR;
    }وگرنه اگر( objc==2 ){
      اگر( pDb->zAuth ){
        Tcl_AppendResult(interp, pDb->zAuth, (نویسه*)0);
      }
    }وگرنه{
      نویسه *zAuth;
      Tcl_Size len;
      اگر( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }وگرنه{
        pDb->zAuth = 0;
      }
      اگر( pDb->zAuth ){
        تعریفنوع صحیح (*sqlite3_auth_cb)(
           تهی*,صحیح,ثابت نویسه*,ثابت نویسه*,
           ثابت نویسه*,ثابت نویسه*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }وگرنه{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#endif
    بشکن;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  حالت DB_BACKUP: {
    ثابت نویسه *zDestFile;
    ثابت نویسه *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    اگر( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }وگرنه اگر( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }وگرنه{
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?DATABASE? FILENAME");
      بازگشت TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    اگر( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (نویسه*)0);
      sqlite3_close(pDest);
      بازگشت TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    اگر( pBackup==0 ){
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (نویسه*)0);
      sqlite3_close(pDest);
      بازگشت TCL_ERROR;
    }
    وقتی(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    اگر( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }وگرنه{
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (نویسه*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    بشکن;
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
  حالت DB_BIND_FALLBACK: {
    اگر( objc>3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?CALLBACK?");
      بازگشت TCL_ERROR;
    }وگرنه اگر( objc==2 ){
      اگر( pDb->zBindFallback ){
        Tcl_AppendResult(interp, pDb->zBindFallback, (نویسه*)0);
      }
    }وگرنه{
      نویسه *zCallback;
      Tcl_Size len;
      اگر( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }وگرنه{
        pDb->zBindFallback = 0;
      }
    }
    بشکن;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  حالت DB_BUSY: {
    اگر( objc>3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "CALLBACK");
      بازگشت TCL_ERROR;
    }وگرنه اگر( objc==2 ){
      اگر( pDb->zBusy ){
        Tcl_AppendResult(interp, pDb->zBusy, (نویسه*)0);
      }
    }وگرنه{
      نویسه *zBusy;
      Tcl_Size len;
      اگر( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }وگرنه{
        pDb->zBusy = 0;
      }
      اگر( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }وگرنه{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    بشکن;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  حالت DB_CACHE: {
    نویسه *subCmd;
    صحیح n;

    اگر( objc<=2 ){
      Tcl_Wrong_رقم_Args(interp, 1, objv, "cache option ?arg?");
      بازگشت TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    اگر( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      اگر( objc!=3 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "flush");
        بازگشت TCL_ERROR;
      }وگرنه{
        flushStmtCache( pDb );
      }
    }وگرنه اگر( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      اگر( objc!=4 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "size n");
        بازگشت TCL_ERROR;
      }وگرنه{
        اگر( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_AppendResult( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (نویسه*)0);
          بازگشت TCL_ERROR;
        }وگرنه{
          اگر( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }وگرنه اگر( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }وگرنه{
      Tcl_AppendResult( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (نویسه*)0);
      بازگشت TCL_ERROR;
    }
    بشکن;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  حالت DB_CHANGES: {
    Tcl_Obj *pResult;
    اگر( objc!=2 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "");
      بازگشت TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_changes64(pDb->db));
    بشکن;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  حالت DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    بشکن;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  حالت DB_COLLATE: {
    SqlCollate *pCollate;
    نویسه *zName;
    نویسه *zScript;
    Tcl_Size nScript;
    اگر( objc!=4 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "NAME SCRIPT");
      بازگشت TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( اندازه(*pCollate) + nScript + 1 );
    اگر( pCollate==0 ) بازگشت TCL_ERROR;
    pCollate->interp = interp;
    pCollate->pNext = pDb->pCollate;
    pCollate->zScript = (نویسه*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    اگر( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_SetResult(interp, (نویسه *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      بازگشت TCL_ERROR;
    }
    بشکن;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  حالت DB_COLLATION_NEEDED: {
    اگر( objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "SCRIPT");
      بازگشت TCL_ERROR;
    }
    اگر( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_شمار(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_شمار(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    بشکن;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  حالت DB_COMMIT_HOOK: {
    اگر( objc>3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?CALLBACK?");
      بازگشت TCL_ERROR;
    }وگرنه اگر( objc==2 ){
      اگر( pDb->zCommit ){
        Tcl_AppendResult(interp, pDb->zCommit, (نویسه*)0);
      }
    }وگرنه{
      ثابت نویسه *zCommit;
      Tcl_Size len;
      اگر( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }وگرنه{
        pDb->zCommit = 0;
      }
      اگر( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }وگرنه{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    بشکن;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  حالت DB_COMPLETE: {
#ifndef SQLITE_OMIT_COMPLETE
    Tcl_Obj *pResult;
    صحیح isComplete;
    اگر( objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "SQL");
      بازگشت TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetBooleanObj(pResult, isComplete);
#endif
    بشکن;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  حالت DB_CONFIG: {
    ایستا ثابت ساختار DbConfigChoices {
      ثابت نویسه *zName;
      صحیح op;
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
    صحیح ii;
    اگر( objc>4 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      بازگشت TCL_ERROR;
    }
    اگر( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      pResult = Tcl_NewListObj(0,0);
      برای(ii=0; ii<اندازه(aDbConfig)/اندازه(aDbConfig[0]); ii++){
        صحیح v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewIntObj(v));
      }
    }وگرنه{
      ثابت نویسه *zOpt = Tcl_GetString(objv[2]);
      صحیح onoff = -1;
      صحیح v = 0;
      اگر( zOpt[0]=='-' ) zOpt++;
      برای(ii=0; ii<اندازه(aDbConfig)/اندازه(aDbConfig[0]); ii++){
        اگر( strcmp(aDbConfig[ii].zName, zOpt)==0 ) بشکن;
      }
      اگر( ii>=اندازه(aDbConfig)/اندازه(aDbConfig[0]) ){
        Tcl_AppendResult(interp, "unknown config option: \"", zOpt,
                                "\"", (تهی*)0);
        بازگشت TCL_ERROR;
      }
      اگر( objc==4 ){
        اگر( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          بازگشت TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      pResult = Tcl_NewIntObj(v);
    }
    Tcl_SetObjResult(interp, pResult);
    بشکن;
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
  حالت DB_COPY: {
    نویسه *zTable;               /* Insert data into this table */
    نویسه *zFile;                /* The file from which to extract data */
    نویسه *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    صحیح nCol;                   /* Number of columns in the table */
    صحیح nByte;                  /* Number of bytes in an SQL string */
    صحیح i, j;                   /* Loop counters */
    صحیح nSep;                   /* Number of bytes in zSep[] */
    صحیح nNull;                  /* Number of bytes in zNull[] */
    نویسه *zSql;                 /* An SQL statement */
    نویسه *z_سطر;                /* A single line of input from the file */
    نویسه **azCol;               /* zLine[] broken up into columns */
    ثابت نویسه *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    صحیح lineno = 0;             /* Line number of input file */
    نویسه z_سطر_رقم[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *pResult;           /* interp result */

    ثابت نویسه *zSep;
    ثابت نویسه *zNull;
    اگر( objc<5 || objc>7 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      بازگشت TCL_ERROR;
    }
    اگر( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }وگرنه{
      zSep = "\t";
    }
    اگر( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }وگرنه{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    اگر( nSep==0 ){
      Tcl_AppendResult(interp,"Error: non-null separator required for copy",
                       (نویسه*)0);
      بازگشت TCL_ERROR;
    }
    اگر(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_AppendResult(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (نویسه*)0);
      بازگشت TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    اگر( zSql==0 ){
      Tcl_AppendResult(interp, "Error: no such table: ", zTable, (نویسه*)0);
      بازگشت TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    اگر( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (نویسه*)0);
      nCol = 0;
    }وگرنه{
      nCol = sqlite3_column_شمار(pStmt);
    }
    sqlite3_finalize(pStmt);
    اگر( nCol==0 ) {
      بازگشت TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    اگر( zSql==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (نویسه*)0);
      بازگشت TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    برای(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    آزاد(zSql);
    اگر( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (نویسه*)0);
      sqlite3_finalize(pStmt);
      بازگشت TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    اگر( in==0 ){
      sqlite3_finalize(pStmt);
      بازگشت TCL_ERROR;
    }
    Tcl_SetChannelOption(پوچ, in, "-translation", "auto");
    azCol = malloc( اندازه(azCol[0])*(nCol+1) );
    اگر( azCol==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (نویسه*)0);
      Tcl_Close(interp, in);
      بازگشت TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_شمار(str);
    (تهی)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    وقتی( Tcl_GetsObj(in, str)>=0 ) {
      نویسه *z;
      Tcl_Size byteLen;
      lineno++;
      z_سطر = (نویسه *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = z_سطر;
      برای(i=0, z=z_سطر; *z; z++){
        اگر( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          اگر( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      اگر( i+1!=nCol ){
        نویسه *zErr;
        صحیح nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        اگر( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_AppendResult(interp, zErr, (نویسه*)0);
          آزاد(zErr);
        }
        zCommit = "ROLLBACK";
        بشکن;
      }
      برای(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        اگر( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }وگرنه{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_SetObjLength(str, 0);
      اگر( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp,"Error: ", sqlite3_errmsg(pDb->db), (نویسه*)0);
        zCommit = "ROLLBACK";
        بشکن;
      }
    }
    Tcl_Decr_Ref_شمار(str);
    آزاد(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (تهی)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    اگر( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      pResult = Tcl_GetObjResult(interp);
      اگر( Tcl_IsShared(pResult) ){
        pResult = Tcl_DuplicateObj(pResult);
      }
      Tcl_SetIntObj(pResult, lineno);
      rc = TCL_OK;
    }وگرنه{
      /* failure, append lineno where failed */
      sqlite3_snprintf(اندازه(z_سطر_رقم), z_سطر_رقم,"%d",lineno);
      Tcl_AppendResult(interp,", failed while processing line: ",z_سطر_رقم,
                       (نویسه*)0);
      rc = TCL_ERROR;
    }
    بشکن;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  حالت DB_DESERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (نویسه*)0);
    rc = TCL_ERROR;
#وگرنه
    ثابت نویسه *zSchema = 0;
    Tcl_Obj *p_مقدار = 0;
    unsigned نویسه *pBA;
    unsigned نویسه *pData;
    Tcl_Size len;
    صحیح xrc;
    sqlite3_int64 mxSize = 0;
    صحیح i;
    صحیح isReadonly = 0;


    اگر( objc<3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      بشکن;
    }
    برای(i=2; i<objc-1; i++){
      ثابت نویسه *z = Tcl_GetString(objv[i]);
      اگر( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        اگر( rc ) برو deserialize_error;
        mxSize = x;
        ادامه;
      }
      اگر( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        اگر( rc ) برو deserialize_error;
        ادامه;
      }
      اگر( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        ادامه;
      }
      Tcl_AppendResult(interp, "unknown option: ", z, (نویسه*)0);
      rc = TCL_ERROR;
      برو deserialize_error;
    }
    p_مقدار = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_مقدار, &len);
    pData = sqlite3_malloc64( len );
    اگر( pData==0 && len>0 ){
      Tcl_AppendResult(interp, "out of memory", (نویسه*)0);
      rc = TCL_ERROR;
    }وگرنه{
      صحیح flags;
      اگر( len>0 ) memcpy(pData, pBA, len);
      اگر( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }وگرنه{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, pData, len, len, flags);
      اگر( xrc ){
        Tcl_AppendResult(interp, "unable to set MEMDB content", (نویسه*)0);
        rc = TCL_ERROR;
      }
      اگر( mxSize>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_SIZE_LIMIT,&mxSize);
      }
    }
deserialize_error:
#endif
    بشکن; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  حالت DB_ENABLE_LOAD_EXTENSION: {
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    صحیح onoff;
    اگر( objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "BOOLEAN");
      بازگشت TCL_ERROR;
    }
    اگر( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      بازگشت TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    بشکن;
#وگرنه
    Tcl_AppendResult(interp, "extension loading is turned off at compile-time",
                     (نویسه*)0);
    بازگشت TCL_ERROR;
#endif
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  حالت DB_ERRORCODE: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    بشکن;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  حالت DB_ERROROFFSET: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    بشکن;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  حالت DB_EXISTS:
  حالت DB_ONECOLUMN: {
    Tcl_Obj *pResult = 0;
    DbEvalContext sEval;
    اگر( objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "SQL");
      بازگشت TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    اگر( choice==DB_ONECOLUMN ){
      اگر( rc==TCL_OK ){
        pResult = db_Eval_Column_مقدار(&sEval, 0);
      }وگرنه اگر( rc==TCL_BREAK ){
        Tcl_ResetResult(interp);
      }
    }وگرنه اگر( rc==TCL_BREAK || rc==TCL_OK ){
      pResult = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    اگر( pResult ) Tcl_SetObjResult(interp, pResult);

    اگر( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    بشکن;
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
  حالت DB_EVAL: {
    صحیح evalFlags = 0;
    ثابت نویسه *zOpt;
    وقتی( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      اگر( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }وگرنه اگر( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }وگرنه{
        Tcl_AppendResult(interp, "unknown option: \"", zOpt, "\"", (تهی*)0);
        بازگشت TCL_ERROR;
      }
      objc--;
      objv++;
    }
    اگر( objc<3 || objc>5 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      بازگشت TCL_ERROR;
    }

    اگر( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_شمار(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      وقتی( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        صحیح i;
        صحیح nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        برای(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_Column_مقدار(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      اگر( rc==TCL_BREAK ){
        Tcl_SetObjResult(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_شمار(pRet);
    }وگرنه{
      ClientData cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      اگر( objc>=5 && *(نویسه *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_شمار(pScript);

      p = (DbEvalContext *)Tcl_Alloc(اندازه(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (تهی *)p;
      cd2[1] = (تهی *)pScript;
      rc = DbEvalNextCmd(cd2, interp, TCL_OK);
    }
    بشکن;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  حالت DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    بشکن;
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
  حالت DB_FUNCTION: {
    صحیح flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    نویسه *zName;
    صحیح nArg = -1;
    صحیح i;
    صحیح eType = SQLITE_NULL;
    اگر( objc<4 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      بازگشت TCL_ERROR;
    }
    برای(i=3; i<(objc-1); i++){
      ثابت نویسه *z = Tcl_GetString(objv[i]);
      صحیح n = strlen30(z);
      اگر( n>1 && strncmp(z, "-argcount",n)==0 ){
        اگر( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(نویسه*)0);
          بازگشت TCL_ERROR;
        }
        اگر( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) بازگشت TCL_ERROR;
        اگر( nArg<0 ){
          Tcl_AppendResult(interp, "number of arguments must be non-negative",
                           (نویسه*)0);
          بازگشت TCL_ERROR;
        }
        i++;
      }وگرنه
      اگر( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }وگرنه
      اگر( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }وگرنه
      اگر( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }وگرنه
      اگر( n>1 && strncmp(z, "-returntype", n)==0 ){
        ثابت نویسه *azType[] = {"integer", "real", "text", "blob", "any", 0};
        أكد( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        أكد( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        اگر( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(نویسه*)0);
          بازگشت TCL_ERROR;
        }
        i++;
        اگر( Tcl_GetIndexFromObj(interp, objv[i], azType, "type", 0, &eType) ){
          بازگشت TCL_ERROR;
        }
        eType++;
      }وگرنه{
        Tcl_AppendResult(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (نویسه*)0
        );
        بازگشت TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    اگر( pFunc==0 ) بازگشت TCL_ERROR;
    اگر( pFunc->pScript ){
      Tcl_Decr_Ref_شمار(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_شمار(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    اگر( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_SetResult(interp, (نویسه *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    بشکن;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  حالت DB_INCRBLOB: {
#ifdef SQLITE_OMIT_INCRBLOB
    Tcl_AppendResult(interp, "incrblob not available in this build", (نویسه*)0);
    بازگشت TCL_ERROR;
#وگرنه
    صحیح isReadonly = 0;
    ثابت نویسه *zDb = "main";
    ثابت نویسه *zTable;
    ثابت نویسه *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    اگر( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    اگر( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      بازگشت TCL_ERROR;
    }

    اگر( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    اگر( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#endif
    بشکن;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  حالت DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    بشکن;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  حالت DB_NULLVALUE: {
    اگر( objc!=2 && objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "NULLVALUE");
      بازگشت TCL_ERROR;
    }
    اگر( objc==3 ){
      Tcl_Size len;
      نویسه *zNull = Tcl_GetStringFromObj(objv[2], &len);
      اگر( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      اگر( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }وگرنه{
        pDb->zNull = 0;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(pDb->zNull, -1));
    بشکن;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  حالت DB_LAST_درج_ROWID: {
    Tcl_Obj *pResult;
    Tcl_WideInt rowid;
    اگر( objc!=2 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "");
      بازگشت TCL_ERROR;
    }
    rowid = sqlite3_last_درج_rowid(pDb->db);
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, rowid);
    بشکن;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  حالت DB_PROGRESS: {
    اگر( objc==2 ){
      اگر( pDb->zProgress ){
        Tcl_AppendResult(interp, pDb->zProgress, (نویسه*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }وگرنه اگر( objc==4 ){
      نویسه *zProgress;
      Tcl_Size len;
      صحیح N;
      اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        بازگشت TCL_ERROR;
      };
      اگر( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      اگر( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }وگرنه{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      اگر( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }وگرنه{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }وگرنه{
      Tcl_Wrong_رقم_Args(interp, 2, objv, "N CALLBACK");
      بازگشت TCL_ERROR;
    }
    بشکن;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  حالت DB_PROFILE: {
    اگر( objc>3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?CALLBACK?");
      بازگشت TCL_ERROR;
    }وگرنه اگر( objc==2 ){
      اگر( pDb->zProfile ){
        Tcl_AppendResult(interp, pDb->zProfile, (نویسه*)0);
      }
    }وگرنه{
      نویسه *zProfile;
      Tcl_Size len;
      اگر( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }وگرنه{
        pDb->zProfile = 0;
      }
#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      اگر( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }وگرنه{
        sqlite3_profile(pDb->db, 0, 0);
      }
#endif
    }
    بشکن;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  حالت DB_REKEY: {
    اگر( objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "KEY");
      بازگشت TCL_ERROR;
    }
    بشکن;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  حالت DB_RESTORE: {
    ثابت نویسه *zSrcFile;
    ثابت نویسه *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    صحیح nTimeout = 0;

    اگر( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }وگرنه اگر( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }وگرنه{
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?DATABASE? FILENAME");
      بازگشت TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    اگر( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (نویسه*)0);
      sqlite3_close(pSrc);
      بازگشت TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    اگر( pBackup==0 ){
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (نویسه*)0);
      sqlite3_close(pSrc);
      بازگشت TCL_ERROR;
    }
    وقتی( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      اگر( rc==SQLITE_BUSY ){
        اگر( nTimeout++ >= 3 ) بشکن;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    اگر( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }وگرنه اگر( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_AppendResult(interp, "restore failed: source database busy",
                       (نویسه*)0);
      rc = TCL_ERROR;
    }وگرنه{
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (نویسه*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    بشکن;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  حالت DB_SERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (نویسه*)0);
    rc = TCL_ERROR;
#وگرنه
    ثابت نویسه *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned نویسه *pData;
    اگر( objc!=2 && objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }وگرنه{
      صحیح needFree;
      pData = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      اگر( pData ){
        needFree = 0;
      }وگرنه{
        pData = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(pData,sz));
      اگر( needFree ) sqlite3_free(pData);
    }
#endif
    بشکن;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  حالت DB_STATUS: {
    صحیح v;
    ثابت نویسه *zOp;
    اگر( objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "(step|sort|autoindex)");
      بازگشت TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    اگر( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }وگرنه اگر( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }وگرنه اگر( strcmp(zOp, "autoindex")==0 ){
      v = pDb->nIndex;
    }وگرنه اگر( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }وگرنه{
      Tcl_AppendResult(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (نویسه*)0);
      بازگشت TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(v));
    بشکن;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  حالت DB_TIMEOUT: {
    صحیح ms;
    اگر( objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "MILLISECONDS");
      بازگشت TCL_ERROR;
    }
    اگر( Tcl_GetIntFromObj(interp, objv[2], &ms) ) بازگشت TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    بشکن;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  حالت DB_TOTAL_CHANGES: {
    Tcl_Obj *pResult;
    اگر( objc!=2 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "");
      بازگشت TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_total_changes64(pDb->db));
    بشکن;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  حالت DB_TRACE: {
    اگر( objc>3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?CALLBACK?");
      بازگشت TCL_ERROR;
    }وگرنه اگر( objc==2 ){
      اگر( pDb->zTrace ){
        Tcl_AppendResult(interp, pDb->zTrace, (نویسه*)0);
      }
    }وگرنه{
      نویسه *zTrace;
      Tcl_Size len;
      اگر( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }وگرنه{
        pDb->zTrace = 0;
      }
#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      اگر( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }وگرنه{
        sqlite3_trace(pDb->db, 0, 0);
      }
#endif
    }
    بشکن;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  حالت DB_TRACE_V2: {
    اگر( objc>4 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?CALLBACK? ?MASK?");
      بازگشت TCL_ERROR;
    }وگرنه اگر( objc==2 ){
      اگر( pDb->zTraceV2 ){
        Tcl_AppendResult(interp, pDb->zTraceV2, (نویسه*)0);
      }
    }وگرنه{
      نویسه *zTraceV2;
      Tcl_Size len;
      Tcl_WideInt wMask = 0;
      اگر( objc==4 ){
        ایستا ثابت نویسه *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        شمارش TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_Size i;
        اگر( TCL_OK!=Tcl_ListObjLength(interp, objv[3], &len) ){
          بازگشت TCL_ERROR;
        }
        برای(i=0; i<len; i++){
          Tcl_Obj *pObj;
          صحیح ttype;
          اگر( TCL_OK!=Tcl_ListObjIndex(interp, objv[3], i, &pObj) ){
            بازگشت TCL_ERROR;
          }
          اگر( Tcl_GetIndexFromObj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
            Tcl_Incr_Ref_شمار(pError);
            اگر( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_شمار(pError);
              wMask |= wType;
            }وگرنه{
              Tcl_SetObjResult(interp, pError);
              Tcl_Decr_Ref_شمار(pError);
              بازگشت TCL_ERROR;
            }
          }وگرنه{
            انتخاب( (شمارش TTYPE_enum)ttype ){
              حالت TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    بشکن;
              حالت TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; بشکن;
              حالت TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     بشکن;
              حالت TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   بشکن;
            }
          }
        }
      }وگرنه{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      اگر( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }وگرنه{
        pDb->zTraceV2 = 0;
      }
#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      اگر( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }وگرنه{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#endif
    }
    بشکن;
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
  حالت DB_TRANSACTION: {
    Tcl_Obj *pScript;
    ثابت نویسه *zBegin = "SAVEPOINT _tcl_transaction";
    اگر( objc!=3 && objc!=4 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "[TYPE] SCRIPT");
      بازگشت TCL_ERROR;
    }

    اگر( pDb->nTransaction==0 && objc==4 ){
      ایستا ثابت نویسه *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      شمارش TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      صحیح ttype;
      اگر( Tcl_GetIndexFromObj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        بازگشت TCL_ERROR;
      }
      انتخاب( (شمارش TTYPE_enum)ttype ){
        حالت TTYPE_DEFERRED:    /* no-op */;                 بشکن;
        حالت TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  بشکن;
        حالت TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  بشکن;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    اگر( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (نویسه*)0);
      بازگشت TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    اگر( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (تهی)Tcl_NREvalObj(interp, pScript, 0);
    }وگرنه{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    بشکن;
  }

  /*
  **    $db unlock_notify ?script?
  */
  حالت DB_UNLOCK_NOTIFY: {
#ifndef SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_AppendResult(interp, "unlock_notify not available in this build",
                     (نویسه*)0);
    rc = TCL_ERROR;
#وگرنه
    اگر( objc!=2 && objc!=3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }وگرنه{
      تهی (*xNotify)(تهی **, صحیح) = 0;
      تهی *pNotifyArg = 0;

      اگر( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_شمار(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      اگر( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (تهی *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_شمار(pDb->pUnlockNotify);
      }

      اگر( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (نویسه*)0);
        rc = TCL_ERROR;
      }
    }
#endif
    بشکن;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  حالت DB_PREUPDATE: {
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_AppendResult(interp, "preupdate_hook was omitted at compile-time", 
                     (نویسه*)0);
    rc = TCL_ERROR;
#وگرنه
    ایستا ثابت نویسه *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    شمارش DbPreupdateSubCmd {
      PRE_شمار, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    صحیح iSub;

    اگر( objc<3 ){
      Tcl_Wrong_رقم_Args(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    اگر( Tcl_GetIndexFromObj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      بازگشت TCL_ERROR;
    }

    انتخاب( (شمارش DbPreupdateSubCmd)iSub ){
      حالت PRE_شمار: {
        صحیح nCol = sqlite3_preupdate_شمار(pDb->db);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nCol));
        بشکن;
      }

      حالت PRE_HOOK: {
        اگر( objc>4 ){
          Tcl_Wrong_رقم_Args(interp, 2, objv, "hook ?SCRIPT?");
          بازگشت TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        بشکن;
      }

      حالت PRE_DEPTH: {
        Tcl_Obj *pRet;
        اگر( objc!=3 ){
          Tcl_Wrong_رقم_Args(interp, 3, objv, "");
          بازگشت TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_SetObjResult(interp, pRet);
        بشکن;
      }

      حالت PRE_NEW:
      حالت PRE_OLD: {
        صحیح iIdx;
        sqlite3_مقدار *p_مقدار;
        اگر( objc!=4 ){
          Tcl_Wrong_رقم_Args(interp, 3, objv, "INDEX");
          بازگشت TCL_ERROR;
        }
        اگر( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          بازگشت TCL_ERROR;
        }

        اگر( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_مقدار);
        }وگرنه{
          أكد( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_مقدار);
        }

        اگر( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((نویسه*)sqlite3_مقدار_text(p_مقدار), -1);
          Tcl_SetObjResult(interp, pObj);
        }وگرنه{
          Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (نویسه*)0);
          بازگشت TCL_ERROR;
        }
      }
    }
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
    بشکن;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  حالت DB_WAL_HOOK:
  حالت DB_UPDATE_HOOK:
  حالت DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    اگر( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    اگر( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    اگر( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    اگر( objc>3 ){
       Tcl_Wrong_رقم_Args(interp, 2, objv, "?SCRIPT?");
       بازگشت TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    بشکن;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  حالت DB_VERSION: {
    صحیح i;
    برای(i=2; i<objc; i++){
      ثابت نویسه *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      اگر( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        اگر( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          بازگشت TCL_ERROR;
        }
      }وگرنه

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      اگر( strcmp(zArg, "-last-stmt-ptr")==0 ){
        بیرونی ثابت نویسه *sqlite3TestPtrToText(تهی*);
        تهی *ptr = (تهی*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_SetResult(interp, (نویسه*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }وگرنه
#endif /* SQLITE_TEST */
      {
        Tcl_AppendResult(interp, "unknown argument: ", zArg, (نویسه*)0);
        بازگشت TCL_ERROR;
      }
    }
    اگر( i==2 ){   
      Tcl_SetResult(interp, (نویسه *)sqlite3_libversion(), TCL_STATIC);
    }
    بشکن;
  }


  } /* End of the SWITCH statement */
  بازگشت rc;
}

#اگر SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
ایستا صحیح SQLITE_TCLAPI DbObjCmdAdaptor(
  تهی *cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  بازگشت Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
ایستا صحیح sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *ثابت*objv
){
  Tcl_Wrong_رقم_Args(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  بازگشت TCL_ERROR;
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
ایستا صحیح SQLITE_TCLAPI DbMain(
  تهی *cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  SqliteDb *p;
  ثابت نویسه *zArg;
  نویسه *z_Err_پیام;
  صحیح i;
  ثابت نویسه *zFile = 0;
  ثابت نویسه *zVfs = 0;
  صحیح flags;
  صحیح bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  صحیح rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#وگرنه
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  اگر( objc==1 ) بازگشت sqliteCmdUsage(interp, objv);
  اگر( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    اگر( strcmp(zArg,"-version")==0 ){
      Tcl_AppendResult(interp,sqlite3_libversion(), (نویسه*)0);
      بازگشت TCL_OK;
    }
    اگر( strcmp(zArg,"-sourceid")==0 ){
      Tcl_AppendResult(interp,sqlite3_sourceid(), (نویسه*)0);
      بازگشت TCL_OK;
    }
    اگر( strcmp(zArg,"-has-codec")==0 ){
      Tcl_AppendResult(interp,"0",(نویسه*)0);
      بازگشت TCL_OK;
    }
    اگر( zArg[0]=='-' ) بازگشت sqliteCmdUsage(interp, objv);
  }
  برای(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    اگر( zArg[0]!='-' ){
      اگر( zFile!=0 ) بازگشت sqliteCmdUsage(interp, objv);
      zFile = zArg;
      ادامه;
    }
    اگر( i==objc-1 ) بازگشت sqliteCmdUsage(interp, objv);
    i++;
    اگر( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }وگرنه اگر( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }وگرنه اگر( strcmp(zArg, "-readonly")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) بازگشت TCL_ERROR;
      اگر( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }وگرنه{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }وگرنه اگر( strcmp(zArg, "-create")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) بازگشت TCL_ERROR;
      اگر( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }وگرنه{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }وگرنه اگر( strcmp(zArg, "-nofollow")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) بازگشت TCL_ERROR;
      اگر( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }وگرنه{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }وگرنه اگر( strcmp(zArg, "-nomutex")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) بازگشت TCL_ERROR;
      اگر( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }وگرنه{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }وگرنه اگر( strcmp(zArg, "-fullmutex")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) بازگشت TCL_ERROR;
      اگر( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }وگرنه{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }وگرنه اگر( strcmp(zArg, "-uri")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) بازگشت TCL_ERROR;
      اگر( b ){
        flags |= SQLITE_OPEN_URI;
      }وگرنه{
        flags &= ~SQLITE_OPEN_URI;
      }
    }وگرنه اگر( strcmp(zArg, "-translatefilename")==0 ){
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        بازگشت TCL_ERROR;
      }
    }وگرنه{
      Tcl_AppendResult(interp, "unknown option: ", zArg, (نویسه*)0);
      بازگشت TCL_ERROR;
    }
  }
  z_Err_پیام = 0;
  p = (SqliteDb*)Tcl_Alloc( اندازه(*p) );
  memset(p, 0, اندازه(*p));
  اگر( zFile==0 ) zFile = "";
  اگر( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  اگر( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  اگر( p->db ){
    اگر( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_پیام = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }وگرنه{
    z_Err_پیام = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  اگر( p->db==0 ){
    Tcl_SetResult(interp, z_Err_پیام, TCL_VOLATILE);
    Tcl_Free((نویسه*)p);
    sqlite3_free(z_Err_پیام);
    بازگشت TCL_ERROR;
  }
  p->maxStmt = رقم_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  اگر( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (نویسه*)p, DbDeleteCmd);
  }وگرنه{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (نویسه*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  بازگشت TCL_OK;
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
EXTERN صحیح Sqlite3_Init(Tcl_Interp *interp){
  صحیح rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  اگر( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  بازگشت rc;
}
EXTERN صحیح Tclsqlite3_Init(Tcl_Interp *interp){ بازگشت Sqlite3_Init(interp); }
EXTERN صحیح Sqlite3_Unload(Tcl_Interp *interp, صحیح flags){ بازگشت TCL_OK; }
EXTERN صحیح Tclsqlite3_Unload(Tcl_Interp *interp, صحیح flags){ بازگشت TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN صحیح Sqlite3_SafeInit(Tcl_Interp *interp){ بازگشت TCL_ERROR; }
EXTERN صحیح Sqlite3_SafeUnload(Tcl_Interp *interp, صحیح flags){بازگشت TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN صحیح Sqlite_Init(Tcl_Interp *interp){ بازگشت Sqlite3_Init(interp);}
EXTERN صحیح Tclsqlite_Init(Tcl_Interp *interp){ بازگشت Sqlite3_Init(interp); }
EXTERN صحیح Sqlite_Unload(Tcl_Interp *interp, صحیح flags){ بازگشت TCL_OK; }
EXTERN صحیح Tclsqlite_Unload(Tcl_Interp *interp, صحیح flags){ بازگشت TCL_OK; }
EXTERN صحیح Sqlite_SafeInit(Tcl_Interp *interp){ بازگشت TCL_ERROR; }
EXTERN صحیح Sqlite_SafeUnload(Tcl_Interp *interp, صحیح flags){بازگشت TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN صحیح sqlite3_Init(Tcl_Interp *interp){ بازگشت Sqlite3_Init(interp);}
EXTERN صحیح sqlite_Init(Tcl_Interp *interp){ بازگشت Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#اگر defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
ایستا ثابت نویسه *tclsh_main_loop(تهی){
  ایستا ثابت نویسه zMainloop[] =
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
  بازگشت zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN اصلی
#endif
صحیح SQLITE_CDECL TCLSH_MAIN(صحیح argc, نویسه **argv){
  Tcl_Interp *interp;
  صحیح i;
  ثابت نویسه *zScript = 0;
  نویسه zArgc[32];
#اگر defined(TCLSH_INIT_PROC)
  بیرونی ثابت نویسه *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#اگر !defined(_WIN32_WCE)
  اگر( getenv("SQLITE_DEBUG_BREAK") ){
    اگر( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }وگرنه{
#اگر defined(_WIN32) || defined(WIN32)
      DebugBreak();
#وگرنهاگر defined(SIGTRAP)
      بالاببر(SIGTRAP);
#endif
    }
  }
#endif

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_SetSystemEncoding(پوچ, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(اندازه(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  برای(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_مقدار);
  }
#اگر defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  اگر( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  اگر( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    ثابت نویسه *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    اگر( zInfo==0 ) zInfo = Tcl_GetStringResult(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    بازگشت 1;
  }
  بازگشت 0;
}
#endif /* TCLSH */
