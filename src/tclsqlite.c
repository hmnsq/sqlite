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
#எனில் defined(INCLUDE_MSVC_H)
# include "msvc.h"
#endif

/****** Copy of tclsqlite.h ******/
#எனில் defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#இல்லையெனில்
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#endif
/* Compatability between Tcl8.6 and Tcl9.0 */
#எனில் TCL_MAJOR_VERSION==9
# define CONST மாறிலி
#அல்லதெனில் !defined(Tcl_Size)
  வகைவரை முழு Tcl_Size;
# ifndef Tcl_Bounce_Ref_மொத்தம்
#  define Tcl_Bounce_Ref_மொத்தம்(X) Tcl_Incr_Ref_மொத்தம்(X); Tcl_Decr_Ref_மொத்தம்(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#endif
/**** End copy of tclsqlite.h ****/

#அந்தர்பாவ <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ifndef SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  வகைவரை unsigned எழுத்து u8;
# ifndef SQLITE_PTRSIZE
#   எனில் defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   இல்லையெனில்
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# எனில் defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    வகைவரை uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    வகைவரை unsigned முழு uptr;
# இல்லையெனில்
    வகைவரை sqlite3_uint64 uptr;
# endif
#endif
#அந்தர்பாவ <ctype.h>

/* Used to get the current process ID */
#எனில் !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#அல்லதெனில் !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (முழு)GetCurrentProcessId
#endif

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ifdef BUILD_sqlite
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_sqlite */

#define இலக்கம்_PREPARED_STMTS 10
#define MAX_PREPARED_STMTS 100

/* Forward declaration */
வகைவரை கட்டமைப்பு SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#எனில் defined(SQLITE_ENABLE_QRF_IN_TCL)
#அந்தர்பாவ "qrf.h"
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
வகைவரை கட்டமைப்பு SqlFunc SqlFunc;
கட்டமைப்பு SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  முழு useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  முழு eType;            /* Type of value to return */
  எழுத்து *zName;          /* Name of this function */
  SqlFunc *pNext;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
வகைவரை கட்டமைப்பு SqlCollate SqlCollate;
கட்டமைப்பு SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  எழுத்து *zScript;        /* The script to be run */
  SqlCollate *pNext;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
வகைவரை கட்டமைப்பு SqlPreparedStmt SqlPreparedStmt;
கட்டமைப்பு SqlPreparedStmt {
  SqlPreparedStmt *pNext;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  முழு nSql;                /* chars in zSql[] */
  மாறிலி எழுத்து *zSql;        /* Text of the SQL statement */
  முழு nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

வகைவரை கட்டமைப்பு IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
கட்டமைப்பு SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  எழுத்து *zBusy;               /* The busy callback routine */
  எழுத்து *zCommit;             /* The commit hook callback routine */
  எழுத்து *zTrace;              /* The trace callback routine */
  எழுத்து *zTraceV2;            /* The trace_v2 callback routine */
  எழுத்து *zProfile;            /* The profile callback routine */
  எழுத்து *zProgress;           /* The progress callback routine */
  எழுத்து *zBindFallback;       /* Callback to invoke on a binding miss */
  எழுத்து *zAuth;               /* The authorization callback routine */
  முழு disableAuth;           /* Disable the authorizer if it exists */
  எழுத்து *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  முழு rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  முழு maxStmt;               /* The next maximum number of stmtList */
  முழு nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  முழு nStep, nSort, nIndex;  /* Statistics for most recent operation */
  முழு nVMStep;               /* Another statistic for most recent operation */
  முழு nTransaction;          /* Number of nested [transaction] methods */
  முழு openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  முழு nRef;                  /* Delete object when this reaches 0 */
#ifdef SQLITE_TEST
  முழு bLegacyPrepare;        /* True to use sqlite3_prepare() */
#endif
};

கட்டமைப்பு IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  unsigned முழு isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *pNext;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
நிலையான முழு strlen30(மாறிலி எழுத்து *z){
  மாறிலி எழுத்து *z2 = z;
  போது( *z2 ){ z2++; }
  திரும்பு 0x3fffffff & (முழு)(z2 - z);
}


#ifndef SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
நிலையான வெறுமை closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *pNext;

  க்கு(p=pDb->pIncrblob; p; p=pNext){
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
நிலையான முழு SQLITE_TCLAPI incrblobClose2(
  ClientData instanceData,
  Tcl_Interp *interp,
  முழு flags
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  முழு  rc;
  sqlite3 *db = p->pDb->db;

  எனில்( flags ){
    p->isClosed |= flags;
    திரும்பு TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  எனில்( p->pNext ){
    p->pNext->pPrev = p->pPrev;
  }
  எனில்( p->pPrev ){
    p->pPrev->pNext = p->pNext;
  }
  எனில்( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->pNext;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((எழுத்து *)p);

  எனில்( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (எழுத்து *)sqlite3_errmsg(db), TCL_VOLATILE);
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}
நிலையான முழு SQLITE_TCLAPI incrblobClose(
  ClientData instanceData,
  Tcl_Interp *interp
){
  திரும்பு incrblobClose2(instanceData, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
நிலையான முழு SQLITE_TCLAPI incrblob_உள்ளீடு(
  ClientData instanceData,
  எழுத்து *buf,
  முழு bufSize,
  முழு *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nRead = bufSize;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  முழு rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  எனில்( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  எனில்( nRead<=0 ){
    திரும்பு 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (வெறுமை *)buf, (முழு)nRead, (முழு)p->iSeek);
  எனில்( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    திரும்பு -1;
  }

  p->iSeek += nRead;
  திரும்பு nRead;
}

/*
** Write data to an incremental blob channel.
*/
நிலையான முழு SQLITE_TCLAPI incrblobOutput(
  ClientData instanceData,
  மாறிலி எழுத்து *buf,
  முழு toWrite,
  முழு *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  முழு rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  எனில்( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    திரும்பு -1;
  }
  எனில்( nWrite<=0 ){
    திரும்பு 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (வெறுமை*)buf,(முழு)nWrite, (முழு)p->iSeek);
  எனில்( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    திரும்பு -1;
  }

  p->iSeek += nWrite;
  திரும்பு nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#எனில் TCL_MAJOR_VERSION==9
# define WideSeekProcType long long
#இல்லையெனில்
# define WideSeekProcType Tcl_WideInt
#endif

/*
** Seek an incremental blob channel.
*/
நிலையான WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  ClientData instanceData,
  WideSeekProcType offset,
  முழு seekMode,
  முழு *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instanceData;

  தேர்வு( seekMode ){
    நிலை SEEK_SET:
      p->iSeek = offset;
      நிறுத்து;
    நிலை SEEK_CUR:
      p->iSeek += offset;
      நிறுத்து;
    நிலை SEEK_END:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      நிறுத்து;

    இயல்பு: ப்ரதிஜ்ஞா(!"Bad seekMode");
  }

  திரும்பு p->iSeek;
}
நிலையான முழு SQLITE_TCLAPI incrblobSeek(
  ClientData instanceData,
  long offset,
  முழு seekMode,
  முழு *errorCodePtr
){
  திரும்பு incrblobWideSeek(instanceData,offset,seekMode,errorCodePtr);
}


நிலையான வெறுமை SQLITE_TCLAPI incrblobWatch(
  ClientData instanceData,
  முழு mode
){
  /* NO-OP */
}
நிலையான முழு SQLITE_TCLAPI incrblobHandle(
  ClientData instanceData,
  முழு dir,
  ClientData *hPtr
){
  திரும்பு TCL_ERROR;
}

நிலையான Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_VERSION_5,             /* version                              */
  incrblobClose,                     /* closeProc                            */
  incrblob_உள்ளீடு,                     /* inputProc                            */
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
நிலையான முழு createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  மாறிலி எழுத்து *zDb,
  மாறிலி எழுத்து *zTable,
  மாறிலி எழுத்து *zColumn,
  sqlite_int64 iRow,
  முழு isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  முழு rc;
  முழு flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  நிலையான முழு மொத்தம் = 0;
  எழுத்து zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  எனில்( rc!=SQLITE_OK ){
    Tcl_SetResult(interp, (எழுத்து *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    திரும்பு TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(அளவு(IncrblobChannel));
  memset(p, 0, அளவு(*p));
  p->pBlob = pBlob;
  எனில்( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(அளவு(zChannel), zChannel, "incrblob_%d", ++மொத்தம்);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->pNext = pDb->pIncrblob;
  p->pPrev = 0;
  எனில்( p->pNext ){
    p->pNext->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_SetResult(interp, (எழுத்து *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  திரும்பு TCL_OK;
}
#இல்லையெனில்  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
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
நிலையான முழு safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  மாறிலி எழுத்து *z;
  Tcl_Size n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  போது( n-- > 0 ){
    முழு c = *(z++);
    எனில்( c=='$' || c=='[' || c==';' ) திரும்பு 0;
  }
  திரும்பு 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
நிலையான SqlFunc *findSqlFunc(SqliteDb *pDb, மாறிலி எழுத்து *zName){
  SqlFunc *p, *pNew;
  முழு nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( அளவு(*pNew) + nName + 1 );
  pNew->zName = (எழுத்து*)&pNew[1];
  memcpy(pNew->zName, zName, nName+1);
  க்கு(p=pDb->pFunc; p; p=p->pNext){
    எனில்( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((எழுத்து*)pNew);
      திரும்பு p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->pNext = pDb->pFunc;
  pDb->pFunc = pNew;
  திரும்பு pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
நிலையான வெறுமை dbFreeStmt(SqlPreparedStmt *pStmt){
#ifdef SQLITE_TEST
  எனில்( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((எழுத்து *)pStmt->zSql);
  }
#endif
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((எழுத்து *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
நிலையான வெறுமை flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *pNext;

  க்கு(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pNext){
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
நிலையான வெறுமை addDatabaseRef(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
நிலையான வெறுமை delDatabaseRef(SqliteDb *pDb){
  ப்ரதிஜ்ஞா( pDb->nRef>0 );
  pDb->nRef--;
  எனில்( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    போது( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->pNext;
      ப்ரதிஜ்ஞா( pFunc->pDb==pDb );
      Tcl_Decr_Ref_மொத்தம்(pFunc->pScript);
      Tcl_Free((எழுத்து*)pFunc);
    }
    போது( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->pNext;
      Tcl_Free((எழுத்து*)pCollate);
    }
    எனில்( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    எனில்( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    எனில்( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    எனில்( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    எனில்( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    எனில்( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    எனில்( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    எனில்( pDb->pUpdateHook ){
      Tcl_Decr_Ref_மொத்தம்(pDb->pUpdateHook);
    }
    எனில்( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_மொத்தம்(pDb->pPreUpdateHook);
    }
    எனில்( pDb->pRollbackHook ){
      Tcl_Decr_Ref_மொத்தம்(pDb->pRollbackHook);
    }
    எனில்( pDb->pWalHook ){
      Tcl_Decr_Ref_மொத்தம்(pDb->pWalHook);
    }
    எனில்( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_மொத்தம்(pDb->pCollateNeeded);
    }
    Tcl_Free((எழுத்து*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
நிலையான வெறுமை SQLITE_TCLAPI DbDeleteCmd(வெறுமை *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
நிலையான முழு DbBusyHandler(வெறுமை *cd, முழு nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  முழு rc;
  எழுத்து zVal[30];

  sqlite3_snprintf(அளவு(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (எழுத்து*)0);
  எனில்( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    திரும்பு 0;
  }
  திரும்பு 1;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
நிலையான முழு DbProgressHandler(வெறுமை *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  முழு rc;

  ப்ரதிஜ்ஞா( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  எனில்( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    திரும்பு 1;
  }
  திரும்பு 0;
}
#endif

#எனில் !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
நிலையான வெறுமை DbTraceHandler(வெறுமை *cd, மாறிலி எழுத்து *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_மதிப்பு(&str));
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
நிலையான முழு DbTraceV2Handler(
  unsigned type, /* One of the SQLITE_TRACE_* event types. */
  வெறுமை *cd,      /* The original context data pointer. */
  வெறுமை *pd,      /* Primary event data, depends on event type. */
  வெறுமை *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  தேர்வு( type ){
    நிலை SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      எழுத்து *zSql = (எழுத்து *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_மொத்தம்(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_மொத்தம்(pCmd);
      Tcl_ResetResult(pDb->interp);
      நிறுத்து;
    }
    நிலை SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_மொத்தம்(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_மொத்தம்(pCmd);
      Tcl_ResetResult(pDb->interp);
      நிறுத்து;
    }
    நிலை SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_மொத்தம்(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_மொத்தம்(pCmd);
      Tcl_ResetResult(pDb->interp);
      நிறுத்து;
    }
    நிலை SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_மொத்தம்(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_மொத்தம்(pCmd);
      Tcl_ResetResult(pDb->interp);
      நிறுத்து;
    }
  }
  திரும்பு SQLITE_OK;
}
#endif

#எனில் !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
நிலையான வெறுமை DbProfileHandler(வெறுமை *cd, மாறிலி எழுத்து *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  எழுத்து zTm[100];

  sqlite3_snprintf(அளவு(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_மதிப்பு(&str));
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
நிலையான முழு DbCommitHandler(வெறுமை *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  முழு rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  எனில்( rc!=TCL_OK || atoi(Tcl_GetStringResult(pDb->interp)) ){
    திரும்பு 1;
  }
  திரும்பு 0;
}

நிலையான வெறுமை DbRollbackHandler(வெறுமை *clientData){
  SqliteDb *pDb = (SqliteDb*)clientData;
  ப்ரதிஜ்ஞா(pDb->pRollbackHook);
  எனில்( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
நிலையான முழு DbWalHandler(
  வெறுமை *clientData,
  sqlite3 *db,
  மாறிலி எழுத்து *zDb,
  முழு nEntry
){
  முழு ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)clientData;
  Tcl_Interp *interp = pDb->interp;
  ப்ரதிஜ்ஞா(pDb->pWalHook);

  ப்ரதிஜ்ஞா( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_மொத்தம்(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  எனில்( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_மொத்தம்(p);

  திரும்பு ret;
}

#எனில் defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
நிலையான வெறுமை setTestUnlockNotifyVars(Tcl_Interp *interp, முழு iArg, முழு nArg){
  எழுத்து zBuf[64];
  sqlite3_snprintf(அளவு(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(அளவு(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#இல்லையெனில்
# define setTestUnlockNotifyVars(x,y,z)
#endif

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
நிலையான வெறுமை DbUnlockNotify(வெறுமை **apArg, முழு nArg){
  முழு i;
  க்கு(i=0; i<nArg; i++){
    மாறிலி முழு flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    setTestUnlockNotifyVars(pDb->interp, i, nArg);
    ப்ரதிஜ்ஞா( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_மொத்தம்(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
நிலையான வெறுமை DbPreUpdateHandler(
  வெறுமை *p,
  sqlite3 *db,
  முழு op,
  மாறிலி எழுத்து *zDb,
  மாறிலி எழுத்து *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  நிலையான மாறிலி எழுத்து *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ப்ரதிஜ்ஞா( (SQLITE_DELETE-1)/9 == 0 );
  ப்ரதிஜ்ஞா( (SQLITE_செருகு-1)/9 == 1 );
  ப்ரதிஜ்ஞா( (SQLITE_UPDATE-1)/9 == 2 );
  ப்ரதிஜ்ஞா( pDb->pPreUpdateHook );
  ப்ரதிஜ்ஞா( db==pDb->db );
  ப்ரதிஜ்ஞா( op==SQLITE_செருகு || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_மொத்தம்(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_மொத்தம்(pCmd);
}
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */

நிலையான வெறுமை DbUpdateHandler(
  வெறுமை *p,
  முழு op,
  மாறிலி எழுத்து *zDb,
  மாறிலி எழுத்து *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  நிலையான மாறிலி எழுத்து *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ப்ரதிஜ்ஞா( (SQLITE_DELETE-1)/9 == 0 );
  ப்ரதிஜ்ஞா( (SQLITE_செருகு-1)/9 == 1 );
  ப்ரதிஜ்ஞா( (SQLITE_UPDATE-1)/9 == 2 );

  ப்ரதிஜ்ஞா( pDb->pUpdateHook );
  ப்ரதிஜ்ஞா( op==SQLITE_செருகு || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_மொத்தம்(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_மொத்தம்(pCmd);
}

நிலையான வெறுமை tclCollateNeeded(
  வெறுமை *pCtx,
  sqlite3 *db,
  முழு enc,
  மாறிலி எழுத்து *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_மொத்தம்(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_மொத்தம்(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
நிலையான முழு tclSqlCollate(
  வெறுமை *pCtx,
  முழு nA,
  மாறிலி வெறுமை *zA,
  முழு nB,
  மாறிலி வெறுமை *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_மொத்தம்(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_மொத்தம்(pCmd);
  திரும்பு (atoi(Tcl_GetStringResult(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
நிலையான வெறுமை tclSqlFunc(sqlite3_context *context, முழு argc, sqlite3_மதிப்பு**argv){
  SqlFunc *p = sqlite3_user_data(context);
  Tcl_Obj *pCmd;
  முழு i;
  முழு rc;

  எனில்( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_மொத்தம்(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_மொத்தம்(pCmd);
  }இல்லையெனில்{
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
    எனில்( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
      திரும்பு;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_மொத்தம்(pCmd);
    க்கு(i=0; i<argc; i++){
      sqlite3_மதிப்பு *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      தேர்வு( sqlite3_மதிப்பு_type(pIn) ){
        நிலை SQLITE_BLOB: {
          முழு bytes = sqlite3_மதிப்பு_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_மதிப்பு_blob(pIn), bytes);
          நிறுத்து;
        }
        நிலை SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_மதிப்பு_int64(pIn);
          எனில்( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((முழு)v);
          }இல்லையெனில்{
            pVal = Tcl_NewWideIntObj(v);
          }
          நிறுத்து;
        }
        நிலை SQLITE_FLOAT: {
          இரட்டை r = sqlite3_மதிப்பு_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          நிறுத்து;
        }
        நிலை SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          நிறுத்து;
        }
        இயல்பு: {
          முழு bytes = sqlite3_மதிப்பு_bytes(pIn);
          pVal = Tcl_NewStringObj((எழுத்து *)sqlite3_மதிப்பு_text(pIn), bytes);
          நிறுத்து;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      எனில்( rc ){
        Tcl_Decr_Ref_மொத்தம்(pCmd);
        sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
        திரும்பு;
      }
    }
    எனில்( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_Decr_Ref_மொத்தம்(pCmd);
  }

  எனில்( TCL_BREAK==rc ){
    sqlite3_result_null(context);
  }இல்லையெனில் எனில்( rc && rc!=TCL_RETURN ){
    sqlite3_result_error(context, Tcl_GetStringResult(p->interp), -1);
  }இல்லையெனில்{
    Tcl_Obj *pVar = Tcl_GetObjResult(p->interp);
    Tcl_Size n;
    u8 *data;
    மாறிலி எழுத்து *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    எழுத்து c = zType[0];
    முழு eType = p->eType;

    எனில்( eType==SQLITE_NULL ){
      எனில்( c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }இல்லையெனில் எனில்( (c=='b' && pVar->bytes==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }இல்லையெனில் எனில்( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }இல்லையெனில்{
        eType = SQLITE_TEXT;
      }
    }

    தேர்வு( eType ){
      நிலை SQLITE_BLOB: {
        data = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_result_blob64(context, data, n, SQLITE_TRANSIENT);
        நிறுத்து;
      }
      நிலை SQLITE_INTEGER: {
        Tcl_WideInt v;
        எனில்( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_result_int64(context, v);
          நிறுத்து;
        }
        /* fall-through */
      }
      நிலை SQLITE_FLOAT: {
        இரட்டை r;
        எனில்( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_result_double(context, r);
          நிறுத்து;
        }
        /* fall-through */
      }
      இயல்பு: {
        data = (unsigned எழுத்து *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_result_text64(context, (எழுத்து *)data, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        நிறுத்து;
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
நிலையான முழு auth_callback(
  வெறுமை *pArg,
  முழு code,
  மாறிலி எழுத்து *zArg1,
  மாறிலி எழுத்து *zArg2,
  மாறிலி எழுத்து *zArg3,
  மாறிலி எழுத்து *zArg4
){
  மாறிலி எழுத்து *zCode;
  Tcl_DString str;
  முழு rc;
  மாறிலி எழுத்து *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  எனில்( pDb->disableAuth ) திரும்பு SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  தேர்வு( code ){
    நிலை SQLITE_COPY              : zCode="SQLITE_COPY"; நிறுத்து;
    நிலை SQLITE_CREATE_INDEX      : zCode="SQLITE_CREATE_INDEX"; நிறுத்து;
    நிலை SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; நிறுத்து;
    நிலை SQLITE_CREATE_TEMP_INDEX : zCode="SQLITE_CREATE_TEMP_INDEX"; நிறுத்து;
    நிலை SQLITE_CREATE_TEMP_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; நிறுத்து;
    நிலை SQLITE_CREATE_TEMP_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; நிறுத்து;
    நிலை SQLITE_CREATE_TEMP_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; நிறுத்து;
    நிலை SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; நிறுத்து;
    நிலை SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; நிறுத்து;
    நிலை SQLITE_DELETE            : zCode="SQLITE_DELETE"; நிறுத்து;
    நிலை SQLITE_DROP_INDEX        : zCode="SQLITE_DROP_INDEX"; நிறுத்து;
    நிலை SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; நிறுத்து;
    நிலை SQLITE_DROP_TEMP_INDEX   : zCode="SQLITE_DROP_TEMP_INDEX"; நிறுத்து;
    நிலை SQLITE_DROP_TEMP_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; நிறுத்து;
    நிலை SQLITE_DROP_TEMP_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; நிறுத்து;
    நிலை SQLITE_DROP_TEMP_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; நிறுத்து;
    நிலை SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; நிறுத்து;
    நிலை SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; நிறுத்து;
    நிலை SQLITE_செருகு            : zCode="SQLITE_INSERT"; நிறுத்து;
    நிலை SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; நிறுத்து;
    நிலை SQLITE_READ              : zCode="SQLITE_READ"; நிறுத்து;
    நிலை SQLITE_SELECT            : zCode="SQLITE_SELECT"; நிறுத்து;
    நிலை SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; நிறுத்து;
    நிலை SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; நிறுத்து;
    நிலை SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; நிறுத்து;
    நிலை SQLITE_DETACH            : zCode="SQLITE_DETACH"; நிறுத்து;
    நிலை SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; நிறுத்து;
    நிலை SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; நிறுத்து;
    நிலை SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; நிறுத்து;
    நிலை SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; நிறுத்து;
    நிலை SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; நிறுத்து;
    நிலை SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; நிறுத்து;
    நிலை SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; நிறுத்து;
    நிலை SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; நிறுத்து;
    இயல்பு                       : zCode="????"; நிறுத்து;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, zCode);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_மதிப்பு(&str));
  Tcl_DStringFree(&str);
  zReply = rc==TCL_OK ? Tcl_GetStringResult(pDb->interp) : "SQLITE_DENY";
  எனில்( strcmp(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }இல்லையெனில் எனில்( strcmp(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }இல்லையெனில் எனில்( strcmp(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }இல்லையெனில்{
    rc = 999;
  }
  திரும்பு rc;
}
#endif /* SQLITE_OMIT_AUTHORIZATION */

#எனில் 0
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
நிலையான எழுத்து *local_getline(எழுத்து *zPrompt, FILE *in){
  எழுத்து *z_வரி;
  முழு n_வரி;
  முழு n;

  n_வரி = 100;
  z_வரி = malloc( n_வரி );
  எனில்( z_வரி==0 ) திரும்பு 0;
  n = 0;
  போது( 1 ){
    எனில்( n+100>n_வரி ){
      n_வரி = n_வரி*2 + 100;
      z_வரி = realloc(z_வரி, n_வரி);
      எனில்( z_வரி==0 ) திரும்பு 0;
    }
    எனில்( fgets(&z_வரி[n], n_வரி - n, in)==0 ){
      எனில்( n==0 ){
        விடுவி(z_வரி);
        திரும்பு 0;
      }
      z_வரி[n] = 0;
      நிறுத்து;
    }
    போது( z_வரி[n] ){ n++; }
    எனில்( n>0 && z_வரி[n-1]=='\n' ){
      n--;
      z_வரி[n] = 0;
      நிறுத்து;
    }
  }
  z_வரி = realloc( z_வரி, n+1 );
  திரும்பு z_வரி;
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
நிலையான முழு SQLITE_TCLAPI DbTransPostCmd(
  ClientData data[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  முழு result                           /* Result of evaluating SCRIPT */
){
  நிலையான மாறிலி எழுத்து *மாறிலி azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)data[0];
  முழு rc = result;
  மாறிலி எழுத்து *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  எனில்( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    எனில்( rc!=TCL_ERROR ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (எழுத்து*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  திரும்பு rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
நிலையான முழு dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  மாறிலி எழுத்து *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  மாறிலி எழுத்து **pzOut              /* OUT: Pointer to next SQL statement */
){
  unsigned முழு prepFlags = 0;
#ifdef SQLITE_TEST
  எனில்( pDb->bLegacyPrepare ){
    திரும்பு sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#endif
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  எனில்( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  திரும்பு sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
நிலையான முழு dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  எழுத்து மாறிலி *zIn,                /* SQL to compile */
  எழுத்து மாறிலி **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  மாறிலி எழுத்து *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  முழு nSql;                       /* Length of zSql in bytes */
  முழு nVar = 0;                   /* Number of variables in statement */
  முழு iParm = 0;                  /* Next free entry in apParm */
  எழுத்து c;
  முழு i;
  முழு needResultReset = 0;        /* Need to invoke Tcl_ResetResult() */
  முழு rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  போது( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  க்கு(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->pNext){
    முழு n = pPreStmt->nSql;
    எனில்( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      எனில்( pPreStmt->pPrev ){
        pPreStmt->pPrev->pNext = pPreStmt->pNext;
      }இல்லையெனில்{
        pDb->stmtList = pPreStmt->pNext;
      }
      எனில்( pPreStmt->pNext ){
        pPreStmt->pNext->pPrev = pPreStmt->pPrev;
      }இல்லையெனில்{
        pDb->stmtLast = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_மொத்தம்(pStmt);
      நிறுத்து;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  எனில்( pPreStmt==0 ){
    முழு nByte;

    எனில்( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      திரும்பு TCL_ERROR;
    }
    எனில்( pStmt==0 ){
      எனில்( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        திரும்பு TCL_ERROR;
      }இல்லையெனில்{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        திரும்பு TCL_OK;
      }
    }

    ப்ரதிஜ்ஞா( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_மொத்தம்(pStmt);
    nByte = அளவு(SqlPreparedStmt) + nVar*அளவு(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (முழு)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ifdef SQLITE_TEST
    எனில்( pPreStmt->zSql==0 ){
      எழுத்து *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      memcpy(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#endif
  }
  ப்ரதிஜ்ஞா( pPreStmt );
  ப்ரதிஜ்ஞா( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  ப்ரதிஜ்ஞா( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  க்கு(i=1; i<=nVar; i++){
    மாறிலி எழுத்து *zVar = sqlite3_bind_parameter_name(pStmt, i);
    எனில்( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      எனில்( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        முழு rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_மொத்தம்(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        எனில்( needResultReset ) Tcl_ResetResult(interp);
        needResultReset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_Decr_Ref_மொத்தம்(pCmd);
        எனில்( rx==TCL_OK ){
          pVar = Tcl_GetObjResult(interp);
        }இல்லையெனில் எனில்( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          நிறுத்து;
        }இல்லையெனில்{
          pVar = 0;
        }
      }
      எனில்( pVar ){
        Tcl_Size n;
        u8 *data;
        மாறிலி எழுத்து *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        எனில்( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          data = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, data, n, SQLITE_STATIC);
          Tcl_Incr_Ref_மொத்தம்(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }இல்லையெனில் எனில்( c=='b' && pVar->bytes==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          முழு nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }இல்லையெனில் எனில்( c=='d' && strcmp(zType,"double")==0 ){
          இரட்டை r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }இல்லையெனில் எனில்( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }இல்லையெனில்{
          data = (unsigned எழுத்து *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (எழுத்து *)data, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_மொத்தம்(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }இல்லையெனில்{
        sqlite3_bind_null(pStmt, i);
      }
      எனில்( needResultReset ) Tcl_ResetResult(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  எனில்( needResultReset && rc==TCL_OK ) Tcl_ResetResult(pDb->interp);

  திரும்பு rc;
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
நிலையான வெறுமை dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  முழு discard                     /* True to delete (not cache) the pPreStmt */
){
  முழு i;

  /* Free the bound string and blob parameters */
  க்கு(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_மொத்தம்(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  எனில்( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }இல்லையெனில்{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->pNext = pDb->stmtList;
    pPreStmt->pPrev = 0;
    எனில்( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    எனில்( pDb->stmtLast==0 ){
      ப்ரதிஜ்ஞா( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    போது( pDb->nStmt>pDb->maxStmt ){
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
வகைவரை கட்டமைப்பு DbEvalContext DbEvalContext;
கட்டமைப்பு DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  மாறிலி எழுத்து *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  முழு nCol;                       /* Number of columns returned by pStmt */
  முழு evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#define SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#define SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
நிலையான வெறுமை dbReleaseColumnNames(DbEvalContext *p){
  எனில்( p->apColName ){
    முழு i;
    க்கு(i=0; i<p->nCol; i++){
      Tcl_Decr_Ref_மொத்தம்(p->apColName[i]);
    }
    Tcl_Free((எழுத்து *)p->apColName);
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
நிலையான வெறுமை dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  முழு evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, அளவு(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_மொத்தம்(pSql);
  எனில்( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_மொத்தம்(pVarName);
  }
  p->evalFlags = evalFlags;
  addDatabaseRef(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
நிலையான வெறுமை dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  முழு *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  எனில்( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    முழு i;                        /* Iterator variable */
    முழு nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_மொத்தம்(pStmt);
    எனில்( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( அளவு(Tcl_Obj*)*nCol );
      க்கு(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_Incr_Ref_மொத்தம்(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    எனில்( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_மொத்தம்(pColList);
      Tcl_Incr_Ref_மொத்தம்(pStar);
      க்கு(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      எனில்( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }இல்லையெனில்{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, வெற்று, 0);
        எனில்( !pDict ){
          pDict = Tcl_NewDictObj();
        }இல்லையெனில் எனில்( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        எனில்( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, வெற்று, pDict, 0);
        }
        Tcl_Bounce_Ref_மொத்தம்(pDict);
      }
      Tcl_Decr_Ref_மொத்தம்(pStar);
      Tcl_Decr_Ref_மொத்தம்(pColList);
    }
  }

  எனில்( papColName ){
    *papColName = p->apColName;
  }
  எனில்( pnCol ){
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
நிலையான முழு dbEvalStep(DbEvalContext *p){
  மாறிலி எழுத்து *zPrevSql = 0;       /* Previous value of p->zSql */

  போது( p->zSql[0] || p->pPreStmt ){
    முழு rc;
    எனில்( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      எனில்( rc!=TCL_OK ) திரும்பு rc;
    }இல்லையெனில்{
      முழு rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      எனில்( rcs==SQLITE_ROW ){
        திரும்பு TCL_OK;
      }
      எனில்( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->nIndex = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      எனில்( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#எனில் SQLITE_TEST
        எனில்( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          தொடர்;
        }
#endif
        Tcl_SetObjResult(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        திரும்பு TCL_ERROR;
      }இல்லையெனில்{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  திரும்பு TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
நிலையான வெறுமை dbEvalFinalize(DbEvalContext *p){
  எனில்( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  எனில்( p->pVarName ){
    Tcl_Decr_Ref_மொத்தம்(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_மொத்தம்(p->pSql);
  dbReleaseColumnNames(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
நிலையான Tcl_Obj *db_Eval_Column_மதிப்பு(DbEvalContext *p, முழு iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  தேர்வு( sqlite3_column_type(pStmt, iCol) ){
    நிலை SQLITE_BLOB: {
      முழு bytes = sqlite3_column_bytes(pStmt, iCol);
      மாறிலி எழுத்து *zBlob = sqlite3_column_blob(pStmt, iCol);
      எனில்( !zBlob ) bytes = 0;
      திரும்பு Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    நிலை SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      எனில்( v>=-2147483647 && v<=2147483647 ){
        திரும்பு Tcl_NewIntObj((முழு)v);
      }இல்லையெனில்{
        திரும்பு Tcl_NewWideIntObj(v);
      }
    }
    நிலை SQLITE_FLOAT: {
      திரும்பு Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    நிலை SQLITE_NULL: {
      திரும்பு Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  திரும்பு Tcl_NewStringObj((எழுத்து*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#எனில் TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
நிலையான முழு DbUseNre(வெறுமை){
  முழு major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  திரும்பு( (major==8 && minor>=6) || major>8 );
}
#இல்லையெனில்
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (வெறுமை)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (வெறுமை)0
#endif

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
நிலையான முழு SQLITE_TCLAPI DbEvalNextCmd(
  ClientData data[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  முழு result                           /* Result so far */
){
  முழு rc = result;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)data[0];
  Tcl_Obj * மாறிலி pScript = (Tcl_Obj *)data[1];
  Tcl_Obj * மாறிலி pVarName = p->pVarName;

  போது( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    முழு i;
    முழு nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    க்கு(i=0; i<nCol; i++){
      எனில்( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, db_Eval_Column_மதிப்பு(p,i), 0);
      }இல்லையெனில் எனில்( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        எனில்( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }இல்லையெனில்{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, வெற்று, 0);
          எனில்( pDict ){
            எனில்( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            எனில்( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, வெற்று, pDict, 0);
            }
            Tcl_Bounce_Ref_மொத்தம்(pDict);
          }
        }
      }இல்லையெனில் எனில்( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       db_Eval_Column_மதிப்பு(p,i), 0);
      }இல்லையெனில்{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, வெற்று, 0);
        எனில்( !pDict ){
          pDict = Tcl_NewDictObj();
        }இல்லையெனில் எனில்( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        எனில்( Tcl_DictObjPut(interp, pDict, apColName[i],
                           db_Eval_Column_மதிப்பு(p,i))==TCL_OK ){
          Tcl_ObjSetVar2(interp, pVarName, வெற்று, pDict, 0);
        }
        Tcl_Bounce_Ref_மொத்தம்(pDict);
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
    எனில்( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbEvalNextCmd, (வெறுமை*)p, (வெறுமை*)pScript, 0, 0);
      திரும்பு Tcl_NREvalObj(interp, pScript, 0);
    }இல்லையெனில்{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_மொத்தம்(pScript);
  dbEvalFinalize(p);
  Tcl_Free((எழுத்து *)p);

  எனில்( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_ResetResult(interp);
    rc = TCL_OK;
  }
  திரும்பு rc;
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
நிலையான வெறுமை DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  எனில்( *ppHook ){
    Tcl_SetObjResult(interp, *ppHook);
    எனில்( pArg ){
      Tcl_Decr_Ref_மொத்தம்(*ppHook);
      *ppHook = 0;
    }
  }
  எனில்( pArg ){
    ப்ரதிஜ்ஞா( !(*ppHook) );
    எனில்( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_மொத்தம்(*ppHook);
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
நிலையான முழு dbQrf(SqliteDb *pDb, முழு objc, Tcl_Obj *மாறிலி*objv){
#ifndef SQLITE_QRF_H
  Tcl_SetResult(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  திரும்பு TCL_ERROR;
#இல்லையெனில்
  எழுத்து *zResult = 0;             /* Result to be returned */
  மாறிலி எழுத்து *zSql = 0;          /* SQL to run */
  முழு i;                         /* Loop counter */
  முழு rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  நிலையான மாறிலி எழுத்து *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  நிலையான மாறிலி unsigned எழுத்து aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_இடது,
    QRF_ALIGN_நடு, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_வலது,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_உச்சி,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, அளவு(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &zResult;
  க்கு(i=2; i<objc; i++){
    மாறிலி எழுத்து *zArg = Tcl_GetString(objv[i]);
    மாறிலி எழுத்து *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    மாறிலி unsigned எழுத்து aBoolMap[] = { 0, 2, 1, 2, 1 };
    எனில்( zArg[0]!='-' ){
      எனில்( zSql ){
        Tcl_AppendResult(pDb->interp, "unknown argument: ", zArg, (எழுத்து*)0);
        rc = TCL_ERROR;
        செல் format_failed;
      }
      zSql  = zArg;
    }இல்லையெனில் எனில்( i==objc-1 ){
      Tcl_AppendResult(pDb->interp, "option has no argument: ", zArg,(எழுத்து*)0);
      rc = TCL_ERROR;
      செல் format_failed;
    }இல்லையெனில் எனில்( strcmp(zArg,"-style")==0 ){
      நிலையான மாறிலி எழுத்து *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      நிலையான unsigned எழுத்து aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_மொத்தம்,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_செருகு,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_வரி,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      முழு style;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      எனில்( rc ) செல் format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-esc")==0 ){
      நிலையான மாறிலி எழுத்து *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      நிலையான unsigned எழுத்து aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_குறியீடு
      };
      முழு esc;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      எனில்( rc ) செல் format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      நிலையான மாறிலி எழுத்து *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      நிலையான unsigned எழுத்து aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      முழு txt;
      முழு k = zArg[2]=='e';
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      எனில்( rc ) செல் format_failed;
      எனில்( k ){
        qrf.eText = aTextMap[txt];
      }இல்லையெனில் எனில்( txt<=2 ){
        ப்ரதிஜ்ஞா( QRF_No==1 );
        ப்ரதிஜ்ஞா( QRF_Yes==2 );
        ப்ரதிஜ்ஞா( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }இல்லையெனில்{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-blob")==0 ){
      நிலையான மாறிலி எழுத்து *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      நிலையான unsigned எழுத்து aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_Size
      };
      முழு blob;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      எனில்( rc ) செல் format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-wordwrap")==0 ){
      முழு v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      எனில்( rc ) செல் format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      முழு v = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      எனில்( rc ) செல் format_failed;
      எனில்( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }இல்லையெனில் எனில்( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }இல்லையெனில் எனில்( zArg[1]=='r' ){
        qrf.b_Row_மொத்தம் = aBoolMap[v];
      }இல்லையெனில்{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      முழு ax = 0;
      rc = Tcl_GetIndexFromObj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      எனில்( rc ) செல் format_failed;
      எனில்( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }இல்லையெனில்{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      முழு v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      எனில்( rc ) செல் format_failed;
      எனில்( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }இல்லையெனில் எனில்( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      எனில்( zArg[1]=='w' ){
        qrf.nWrap = v;
      }இல்லையெனில் எனில்( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }இல்லையெனில் எனில்( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }இல்லையெனில்{
        qrf.n_வரி_Limit = v;
      }
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-charlimit")==0 ){
      முழு v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      எனில்( rc ) செல் format_failed;
      எனில்( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-multiinsert")==0 ){
      முழு v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      எனில்( rc ) செல் format_failed;
      எனில்( v<0 ) v = 0;
      qrf.n_Multi_செருகு = v;
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-align")==0 ){
      Tcl_Size n = 0;
      முழு jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      எனில்( rc ) செல் format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*அளவு(qrf.aAlign[0]) );
      எனில்( qrf.aAlign==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (எழுத்து*)0);
        rc = TCL_ERROR;
        செல் format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*அளவு(qrf.aAlign[0]));
      qrf.nAlign = n;
      க்கு(jj=0; jj<n; jj++){
        முழு x;
        Tcl_Obj *pTerm;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        எனில்( rc ) செல் format_failed;
        rc = Tcl_GetIndexFromObj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        எனில்( rc ) செல் format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-widths")==0 ){
      Tcl_Size n = 0;
      முழு jj;
      rc = Tcl_ListObjLength(pDb->interp, objv[i+1], &n);
      எனில்( rc ) செல் format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*அளவு(qrf.aWidth[0]) );
      எனில்( qrf.aWidth==0 ){
        Tcl_AppendResult(pDb->interp, "out of memory", (எழுத்து*)0);
        rc = TCL_ERROR;
        செல் format_failed;
      }
      memset(qrf.aWidth, 0, (n+1)*அளவு(qrf.aWidth[0]));
      qrf.nWidth = n;
      க்கு(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        முழு v;
        rc = Tcl_ListObjIndex(pDb->interp, objv[i+1], jj, &pTerm);
        எனில்( rc ) செல் format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        எனில்( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }இல்லையெனில் எனில்( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (short முழு)v;
      }
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-ifmt")==0 ){
      எழுத்து *zFmt = Tcl_GetString(objv[i+1]);
      எனில்( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }இல்லையெனில் எனில்( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_AppendResult(pDb->interp, "bad format for -ifmt", (எழுத்து*)0);
        rc = TCL_ERROR;
        செல் format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-fpfmt")==0 ){
      எழுத்து *zFmt = Tcl_GetString(objv[i+1]);
      எனில்( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }இல்லையெனில் எனில்( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_AppendResult(pDb->interp, "bad format for -fpfmt", (எழுத்து*)0);
        rc = TCL_ERROR;
        செல் format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }இல்லையெனில் எனில்( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }இல்லையெனில்{
      Tcl_AppendResult(pDb->interp, "unknown option: ", zArg, (எழுத்து*)0);
      rc = TCL_ERROR;
      செல் format_failed;
    }
  }
  போது( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    எழுத்து *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    எனில்( rc ) செல் format_failed;
    எனில்( pStmt==0 ) தொடர்;
    rc = sqlite3_format_query_result(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    எனில்( rc ){
      Tcl_SetResult(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      செல் format_failed;
    }
  }
  Tcl_SetResult(pDb->interp, zResult, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.aWidth);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(zResult);
  திரும்பு rc;

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
நிலையான முழு SQLITE_TCLAPI DbObjCmd(
  வெறுமை *cd,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *மாறிலி*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  முழு choice;
  முழு rc = TCL_OK;
  நிலையான மாறிலி எழுத்து *DB_strs[] = {
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
  எண்ணிக்கை DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_LOAD_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_INTERRUPT,            DB_LAST_செருகு_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_PROGRESS,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_STATUS,                DB_TIMEOUT,              DB_TOTAL_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_VERSION,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  எனில்( objc<2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SUBCOMMAND ...");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIndexFromObj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    திரும்பு TCL_ERROR;
  }

  தேர்வு( (எண்ணிக்கை DB_enum)choice ){

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
  நிலை DB_AUTHORIZER: {
#ifdef SQLITE_OMIT_AUTHORIZATION
    Tcl_AppendResult(interp, "authorization not available in this build",
                     (எழுத்து*)0);
    திரும்பு TCL_ERROR;
#இல்லையெனில்
    எனில்( objc>3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?CALLBACK?");
      திரும்பு TCL_ERROR;
    }இல்லையெனில் எனில்( objc==2 ){
      எனில்( pDb->zAuth ){
        Tcl_AppendResult(interp, pDb->zAuth, (எழுத்து*)0);
      }
    }இல்லையெனில்{
      எழுத்து *zAuth;
      Tcl_Size len;
      எனில்( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      எனில்( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        memcpy(pDb->zAuth, zAuth, len+1);
      }இல்லையெனில்{
        pDb->zAuth = 0;
      }
      எனில்( pDb->zAuth ){
        வகைவரை முழு (*sqlite3_auth_cb)(
           வெறுமை*,முழு,மாறிலி எழுத்து*,மாறிலி எழுத்து*,
           மாறிலி எழுத்து*,மாறிலி எழுத்து*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }இல்லையெனில்{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#endif
    நிறுத்து;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  நிலை DB_BACKUP: {
    மாறிலி எழுத்து *zDestFile;
    மாறிலி எழுத்து *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    எனில்( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }இல்லையெனில் எனில்( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }இல்லையெனில்{
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?DATABASE? FILENAME");
      திரும்பு TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    எனில்( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (எழுத்து*)0);
      sqlite3_close(pDest);
      திரும்பு TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    எனில்( pBackup==0 ){
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (எழுத்து*)0);
      sqlite3_close(pDest);
      திரும்பு TCL_ERROR;
    }
    போது(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    எனில்( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }இல்லையெனில்{
      Tcl_AppendResult(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (எழுத்து*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    நிறுத்து;
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
  நிலை DB_BIND_FALLBACK: {
    எனில்( objc>3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?CALLBACK?");
      திரும்பு TCL_ERROR;
    }இல்லையெனில் எனில்( objc==2 ){
      எனில்( pDb->zBindFallback ){
        Tcl_AppendResult(interp, pDb->zBindFallback, (எழுத்து*)0);
      }
    }இல்லையெனில்{
      எழுத்து *zCallback;
      Tcl_Size len;
      எனில்( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      எனில்( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBindFallback, zCallback, len+1);
      }இல்லையெனில்{
        pDb->zBindFallback = 0;
      }
    }
    நிறுத்து;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  நிலை DB_BUSY: {
    எனில்( objc>3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "CALLBACK");
      திரும்பு TCL_ERROR;
    }இல்லையெனில் எனில்( objc==2 ){
      எனில்( pDb->zBusy ){
        Tcl_AppendResult(interp, pDb->zBusy, (எழுத்து*)0);
      }
    }இல்லையெனில்{
      எழுத்து *zBusy;
      Tcl_Size len;
      எனில்( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      எனில்( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        memcpy(pDb->zBusy, zBusy, len+1);
      }இல்லையெனில்{
        pDb->zBusy = 0;
      }
      எனில்( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }இல்லையெனில்{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    நிறுத்து;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  நிலை DB_CACHE: {
    எழுத்து *subCmd;
    முழு n;

    எனில்( objc<=2 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "cache option ?arg?");
      திரும்பு TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    எனில்( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      எனில்( objc!=3 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "flush");
        திரும்பு TCL_ERROR;
      }இல்லையெனில்{
        flushStmtCache( pDb );
      }
    }இல்லையெனில் எனில்( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      எனில்( objc!=4 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "size n");
        திரும்பு TCL_ERROR;
      }இல்லையெனில்{
        எனில்( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_AppendResult( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (எழுத்து*)0);
          திரும்பு TCL_ERROR;
        }இல்லையெனில்{
          எனில்( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }இல்லையெனில் எனில்( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }இல்லையெனில்{
      Tcl_AppendResult( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
    நிறுத்து;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  நிலை DB_CHANGES: {
    Tcl_Obj *pResult;
    எனில்( objc!=2 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
      திரும்பு TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_changes64(pDb->db));
    நிறுத்து;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  நிலை DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    நிறுத்து;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  நிலை DB_COLLATE: {
    SqlCollate *pCollate;
    எழுத்து *zName;
    எழுத்து *zScript;
    Tcl_Size nScript;
    எனில்( objc!=4 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "NAME SCRIPT");
      திரும்பு TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( அளவு(*pCollate) + nScript + 1 );
    எனில்( pCollate==0 ) திரும்பு TCL_ERROR;
    pCollate->interp = interp;
    pCollate->pNext = pDb->pCollate;
    pCollate->zScript = (எழுத்து*)&pCollate[1];
    pDb->pCollate = pCollate;
    memcpy(pCollate->zScript, zScript, nScript+1);
    எனில்( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_SetResult(interp, (எழுத்து *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      திரும்பு TCL_ERROR;
    }
    நிறுத்து;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  நிலை DB_COLLATION_NEEDED: {
    எனில்( objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "SCRIPT");
      திரும்பு TCL_ERROR;
    }
    எனில்( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_மொத்தம்(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_மொத்தம்(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    நிறுத்து;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  நிலை DB_COMMIT_HOOK: {
    எனில்( objc>3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?CALLBACK?");
      திரும்பு TCL_ERROR;
    }இல்லையெனில் எனில்( objc==2 ){
      எனில்( pDb->zCommit ){
        Tcl_AppendResult(interp, pDb->zCommit, (எழுத்து*)0);
      }
    }இல்லையெனில்{
      மாறிலி எழுத்து *zCommit;
      Tcl_Size len;
      எனில்( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      எனில்( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        memcpy(pDb->zCommit, zCommit, len+1);
      }இல்லையெனில்{
        pDb->zCommit = 0;
      }
      எனில்( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }இல்லையெனில்{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    நிறுத்து;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  நிலை DB_COMPLETE: {
#ifndef SQLITE_OMIT_COMPLETE
    Tcl_Obj *pResult;
    முழு isComplete;
    எனில்( objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "SQL");
      திரும்பு TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetBooleanObj(pResult, isComplete);
#endif
    நிறுத்து;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  நிலை DB_CONFIG: {
    நிலையான மாறிலி கட்டமைப்பு DbConfigChoices {
      மாறிலி எழுத்து *zName;
      முழு op;
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
    முழு ii;
    எனில்( objc>4 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      திரும்பு TCL_ERROR;
    }
    எனில்( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      pResult = Tcl_NewListObj(0,0);
      க்கு(ii=0; ii<அளவு(aDbConfig)/அளவு(aDbConfig[0]); ii++){
        முழு v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, pResult,
           Tcl_NewIntObj(v));
      }
    }இல்லையெனில்{
      மாறிலி எழுத்து *zOpt = Tcl_GetString(objv[2]);
      முழு onoff = -1;
      முழு v = 0;
      எனில்( zOpt[0]=='-' ) zOpt++;
      க்கு(ii=0; ii<அளவு(aDbConfig)/அளவு(aDbConfig[0]); ii++){
        எனில்( strcmp(aDbConfig[ii].zName, zOpt)==0 ) நிறுத்து;
      }
      எனில்( ii>=அளவு(aDbConfig)/அளவு(aDbConfig[0]) ){
        Tcl_AppendResult(interp, "unknown config option: \"", zOpt,
                                "\"", (வெறுமை*)0);
        திரும்பு TCL_ERROR;
      }
      எனில்( objc==4 ){
        எனில்( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          திரும்பு TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      pResult = Tcl_NewIntObj(v);
    }
    Tcl_SetObjResult(interp, pResult);
    நிறுத்து;
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
  நிலை DB_COPY: {
    எழுத்து *zTable;               /* Insert data into this table */
    எழுத்து *zFile;                /* The file from which to extract data */
    எழுத்து *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    முழு nCol;                   /* Number of columns in the table */
    முழு nByte;                  /* Number of bytes in an SQL string */
    முழு i, j;                   /* Loop counters */
    முழு nSep;                   /* Number of bytes in zSep[] */
    முழு nNull;                  /* Number of bytes in zNull[] */
    எழுத்து *zSql;                 /* An SQL statement */
    எழுத்து *z_வரி;                /* A single line of input from the file */
    எழுத்து **azCol;               /* zLine[] broken up into columns */
    மாறிலி எழுத்து *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    முழு lineno = 0;             /* Line number of input file */
    எழுத்து z_வரி_இலக்கம்[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *pResult;           /* interp result */

    மாறிலி எழுத்து *zSep;
    மாறிலி எழுத்து *zNull;
    எனில்( objc<5 || objc>7 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      திரும்பு TCL_ERROR;
    }
    எனில்( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }இல்லையெனில்{
      zSep = "\t";
    }
    எனில்( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }இல்லையெனில்{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    எனில்( nSep==0 ){
      Tcl_AppendResult(interp,"Error: non-null separator required for copy",
                       (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
    எனில்(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_AppendResult(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    எனில்( zSql==0 ){
      Tcl_AppendResult(interp, "Error: no such table: ", zTable, (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    எனில்( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (எழுத்து*)0);
      nCol = 0;
    }இல்லையெனில்{
      nCol = sqlite3_column_மொத்தம்(pStmt);
    }
    sqlite3_finalize(pStmt);
    எனில்( nCol==0 ) {
      திரும்பு TCL_ERROR;
    }
    zSql = malloc( nByte + 50 + nCol*2 );
    எனில்( zSql==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    க்கு(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    விடுவி(zSql);
    எனில்( rc ){
      Tcl_AppendResult(interp, "Error: ", sqlite3_errmsg(pDb->db), (எழுத்து*)0);
      sqlite3_finalize(pStmt);
      திரும்பு TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    எனில்( in==0 ){
      sqlite3_finalize(pStmt);
      திரும்பு TCL_ERROR;
    }
    Tcl_SetChannelOption(வெற்று, in, "-translation", "auto");
    azCol = malloc( அளவு(azCol[0])*(nCol+1) );
    எனில்( azCol==0 ) {
      Tcl_AppendResult(interp, "Error: can't malloc()", (எழுத்து*)0);
      Tcl_Close(interp, in);
      திரும்பு TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_மொத்தம்(str);
    (வெறுமை)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    போது( Tcl_GetsObj(in, str)>=0 ) {
      எழுத்து *z;
      Tcl_Size byteLen;
      lineno++;
      z_வரி = (எழுத்து *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = z_வரி;
      க்கு(i=0, z=z_வரி; *z; z++){
        எனில்( *z==zSep[0] && strncmp(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          எனில்( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      எனில்( i+1!=nCol ){
        எழுத்து *zErr;
        முழு nErr = strlen30(zFile) + 200;
        zErr = malloc(nErr);
        எனில்( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_AppendResult(interp, zErr, (எழுத்து*)0);
          விடுவி(zErr);
        }
        zCommit = "ROLLBACK";
        நிறுத்து;
      }
      க்கு(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        எனில்( (nNull>0 && strcmp(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }இல்லையெனில்{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_SetObjLength(str, 0);
      எனில்( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp,"Error: ", sqlite3_errmsg(pDb->db), (எழுத்து*)0);
        zCommit = "ROLLBACK";
        நிறுத்து;
      }
    }
    Tcl_Decr_Ref_மொத்தம்(str);
    விடுவி(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (வெறுமை)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    எனில்( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      pResult = Tcl_GetObjResult(interp);
      எனில்( Tcl_IsShared(pResult) ){
        pResult = Tcl_DuplicateObj(pResult);
      }
      Tcl_SetIntObj(pResult, lineno);
      rc = TCL_OK;
    }இல்லையெனில்{
      /* failure, append lineno where failed */
      sqlite3_snprintf(அளவு(z_வரி_இலக்கம்), z_வரி_இலக்கம்,"%d",lineno);
      Tcl_AppendResult(interp,", failed while processing line: ",z_வரி_இலக்கம்,
                       (எழுத்து*)0);
      rc = TCL_ERROR;
    }
    நிறுத்து;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  நிலை DB_DESERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (எழுத்து*)0);
    rc = TCL_ERROR;
#இல்லையெனில்
    மாறிலி எழுத்து *zSchema = 0;
    Tcl_Obj *p_மதிப்பு = 0;
    unsigned எழுத்து *pBA;
    unsigned எழுத்து *pData;
    Tcl_Size len;
    முழு xrc;
    sqlite3_int64 mxSize = 0;
    முழு i;
    முழு isReadonly = 0;


    எனில்( objc<3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      நிறுத்து;
    }
    க்கு(i=2; i<objc-1; i++){
      மாறிலி எழுத்து *z = Tcl_GetString(objv[i]);
      எனில்( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        எனில்( rc ) செல் deserialize_error;
        mxSize = x;
        தொடர்;
      }
      எனில்( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        எனில்( rc ) செல் deserialize_error;
        தொடர்;
      }
      எனில்( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        தொடர்;
      }
      Tcl_AppendResult(interp, "unknown option: ", z, (எழுத்து*)0);
      rc = TCL_ERROR;
      செல் deserialize_error;
    }
    p_மதிப்பு = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_மதிப்பு, &len);
    pData = sqlite3_malloc64( len );
    எனில்( pData==0 && len>0 ){
      Tcl_AppendResult(interp, "out of memory", (எழுத்து*)0);
      rc = TCL_ERROR;
    }இல்லையெனில்{
      முழு flags;
      எனில்( len>0 ) memcpy(pData, pBA, len);
      எனில்( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }இல்லையெனில்{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, pData, len, len, flags);
      எனில்( xrc ){
        Tcl_AppendResult(interp, "unable to set MEMDB content", (எழுத்து*)0);
        rc = TCL_ERROR;
      }
      எனில்( mxSize>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_SIZE_LIMIT,&mxSize);
      }
    }
deserialize_error:
#endif
    நிறுத்து; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  நிலை DB_ENABLE_LOAD_EXTENSION: {
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    முழு onoff;
    எனில்( objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "BOOLEAN");
      திரும்பு TCL_ERROR;
    }
    எனில்( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      திரும்பு TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    நிறுத்து;
#இல்லையெனில்
    Tcl_AppendResult(interp, "extension loading is turned off at compile-time",
                     (எழுத்து*)0);
    திரும்பு TCL_ERROR;
#endif
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  நிலை DB_ERRORCODE: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    நிறுத்து;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  நிலை DB_ERROROFFSET: {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    நிறுத்து;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  நிலை DB_EXISTS:
  நிலை DB_ONECOLUMN: {
    Tcl_Obj *pResult = 0;
    DbEvalContext sEval;
    எனில்( objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "SQL");
      திரும்பு TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    எனில்( choice==DB_ONECOLUMN ){
      எனில்( rc==TCL_OK ){
        pResult = db_Eval_Column_மதிப்பு(&sEval, 0);
      }இல்லையெனில் எனில்( rc==TCL_BREAK ){
        Tcl_ResetResult(interp);
      }
    }இல்லையெனில் எனில்( rc==TCL_BREAK || rc==TCL_OK ){
      pResult = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    எனில்( pResult ) Tcl_SetObjResult(interp, pResult);

    எனில்( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    நிறுத்து;
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
  நிலை DB_EVAL: {
    முழு evalFlags = 0;
    மாறிலி எழுத்து *zOpt;
    போது( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      எனில்( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }இல்லையெனில் எனில்( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }இல்லையெனில்{
        Tcl_AppendResult(interp, "unknown option: \"", zOpt, "\"", (வெறுமை*)0);
        திரும்பு TCL_ERROR;
      }
      objc--;
      objv++;
    }
    எனில்( objc<3 || objc>5 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      திரும்பு TCL_ERROR;
    }

    எனில்( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_மொத்தம்(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      போது( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        முழு i;
        முழு nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        க்கு(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_Column_மதிப்பு(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      எனில்( rc==TCL_BREAK ){
        Tcl_SetObjResult(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_மொத்தம்(pRet);
    }இல்லையெனில்{
      ClientData cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      எனில்( objc>=5 && *(எழுத்து *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_மொத்தம்(pScript);

      p = (DbEvalContext *)Tcl_Alloc(அளவு(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (வெறுமை *)p;
      cd2[1] = (வெறுமை *)pScript;
      rc = DbEvalNextCmd(cd2, interp, TCL_OK);
    }
    நிறுத்து;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  நிலை DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    நிறுத்து;
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
  நிலை DB_FUNCTION: {
    முழு flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    எழுத்து *zName;
    முழு nArg = -1;
    முழு i;
    முழு eType = SQLITE_NULL;
    எனில்( objc<4 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      திரும்பு TCL_ERROR;
    }
    க்கு(i=3; i<(objc-1); i++){
      மாறிலி எழுத்து *z = Tcl_GetString(objv[i]);
      முழு n = strlen30(z);
      எனில்( n>1 && strncmp(z, "-argcount",n)==0 ){
        எனில்( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(எழுத்து*)0);
          திரும்பு TCL_ERROR;
        }
        எனில்( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) திரும்பு TCL_ERROR;
        எனில்( nArg<0 ){
          Tcl_AppendResult(interp, "number of arguments must be non-negative",
                           (எழுத்து*)0);
          திரும்பு TCL_ERROR;
        }
        i++;
      }இல்லையெனில்
      எனில்( n>1 && strncmp(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }இல்லையெனில்
      எனில்( n>1 && strncmp(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }இல்லையெனில்
      எனில்( n>1 && strncmp(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }இல்லையெனில்
      எனில்( n>1 && strncmp(z, "-returntype", n)==0 ){
        மாறிலி எழுத்து *azType[] = {"integer", "real", "text", "blob", "any", 0};
        ப்ரதிஜ்ஞா( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        ப்ரதிஜ்ஞா( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        எனில்( i==(objc-2) ){
          Tcl_AppendResult(interp, "option requires an argument: ", z,(எழுத்து*)0);
          திரும்பு TCL_ERROR;
        }
        i++;
        எனில்( Tcl_GetIndexFromObj(interp, objv[i], azType, "type", 0, &eType) ){
          திரும்பு TCL_ERROR;
        }
        eType++;
      }இல்லையெனில்{
        Tcl_AppendResult(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (எழுத்து*)0
        );
        திரும்பு TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    எனில்( pFunc==0 ) திரும்பு TCL_ERROR;
    எனில்( pFunc->pScript ){
      Tcl_Decr_Ref_மொத்தம்(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_மொத்தம்(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    எனில்( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_SetResult(interp, (எழுத்து *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    நிறுத்து;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  நிலை DB_INCRBLOB: {
#ifdef SQLITE_OMIT_INCRBLOB
    Tcl_AppendResult(interp, "incrblob not available in this build", (எழுத்து*)0);
    திரும்பு TCL_ERROR;
#இல்லையெனில்
    முழு isReadonly = 0;
    மாறிலி எழுத்து *zDb = "main";
    மாறிலி எழுத்து *zTable;
    மாறிலி எழுத்து *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    எனில்( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    எனில்( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      திரும்பு TCL_ERROR;
    }

    எனில்( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    எனில்( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#endif
    நிறுத்து;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  நிலை DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    நிறுத்து;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  நிலை DB_NULLVALUE: {
    எனில்( objc!=2 && objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "NULLVALUE");
      திரும்பு TCL_ERROR;
    }
    எனில்( objc==3 ){
      Tcl_Size len;
      எழுத்து *zNull = Tcl_GetStringFromObj(objv[2], &len);
      எனில்( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      எனில்( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        memcpy(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }இல்லையெனில்{
        pDb->zNull = 0;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewStringObj(pDb->zNull, -1));
    நிறுத்து;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  நிலை DB_LAST_செருகு_ROWID: {
    Tcl_Obj *pResult;
    Tcl_WideInt rowid;
    எனில்( objc!=2 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
      திரும்பு TCL_ERROR;
    }
    rowid = sqlite3_last_செருகு_rowid(pDb->db);
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, rowid);
    நிறுத்து;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  நிலை DB_PROGRESS: {
    எனில்( objc==2 ){
      எனில்( pDb->zProgress ){
        Tcl_AppendResult(interp, pDb->zProgress, (எழுத்து*)0);
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#endif
    }இல்லையெனில் எனில்( objc==4 ){
      எழுத்து *zProgress;
      Tcl_Size len;
      முழு N;
      எனில்( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        திரும்பு TCL_ERROR;
      };
      எனில்( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      எனில்( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProgress, zProgress, len+1);
      }இல்லையெனில்{
        pDb->zProgress = 0;
      }
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
      எனில்( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }இல்லையெனில்{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#endif
    }இல்லையெனில்{
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "N CALLBACK");
      திரும்பு TCL_ERROR;
    }
    நிறுத்து;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  நிலை DB_PROFILE: {
    எனில்( objc>3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?CALLBACK?");
      திரும்பு TCL_ERROR;
    }இல்லையெனில் எனில்( objc==2 ){
      எனில்( pDb->zProfile ){
        Tcl_AppendResult(interp, pDb->zProfile, (எழுத்து*)0);
      }
    }இல்லையெனில்{
      எழுத்து *zProfile;
      Tcl_Size len;
      எனில்( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      எனில்( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        memcpy(pDb->zProfile, zProfile, len+1);
      }இல்லையெனில்{
        pDb->zProfile = 0;
      }
#எனில் !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      எனில்( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }இல்லையெனில்{
        sqlite3_profile(pDb->db, 0, 0);
      }
#endif
    }
    நிறுத்து;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  நிலை DB_REKEY: {
    எனில்( objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "KEY");
      திரும்பு TCL_ERROR;
    }
    நிறுத்து;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  நிலை DB_RESTORE: {
    மாறிலி எழுத்து *zSrcFile;
    மாறிலி எழுத்து *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    முழு nTimeout = 0;

    எனில்( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }இல்லையெனில் எனில்( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }இல்லையெனில்{
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?DATABASE? FILENAME");
      திரும்பு TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    எனில்( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (எழுத்து*)0);
      sqlite3_close(pSrc);
      திரும்பு TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    எனில்( pBackup==0 ){
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (எழுத்து*)0);
      sqlite3_close(pSrc);
      திரும்பு TCL_ERROR;
    }
    போது( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      எனில்( rc==SQLITE_BUSY ){
        எனில்( nTimeout++ >= 3 ) நிறுத்து;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    எனில்( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }இல்லையெனில் எனில்( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_AppendResult(interp, "restore failed: source database busy",
                       (எழுத்து*)0);
      rc = TCL_ERROR;
    }இல்லையெனில்{
      Tcl_AppendResult(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (எழுத்து*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    நிறுத்து;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  நிலை DB_SERIALIZE: {
#ifdef SQLITE_OMIT_DESERIALIZE
    Tcl_AppendResult(interp, "MEMDB not available in this build",
                     (எழுத்து*)0);
    rc = TCL_ERROR;
#இல்லையெனில்
    மாறிலி எழுத்து *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    unsigned எழுத்து *pData;
    எனில்( objc!=2 && objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }இல்லையெனில்{
      முழு needFree;
      pData = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      எனில்( pData ){
        needFree = 0;
      }இல்லையெனில்{
        pData = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_SetObjResult(interp, Tcl_NewByteArrayObj(pData,sz));
      எனில்( needFree ) sqlite3_free(pData);
    }
#endif
    நிறுத்து;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  நிலை DB_STATUS: {
    முழு v;
    மாறிலி எழுத்து *zOp;
    எனில்( objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "(step|sort|autoindex)");
      திரும்பு TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    எனில்( strcmp(zOp, "step")==0 ){
      v = pDb->nStep;
    }இல்லையெனில் எனில்( strcmp(zOp, "sort")==0 ){
      v = pDb->nSort;
    }இல்லையெனில் எனில்( strcmp(zOp, "autoindex")==0 ){
      v = pDb->nIndex;
    }இல்லையெனில் எனில்( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }இல்லையெனில்{
      Tcl_AppendResult(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(v));
    நிறுத்து;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  நிலை DB_TIMEOUT: {
    முழு ms;
    எனில்( objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "MILLISECONDS");
      திரும்பு TCL_ERROR;
    }
    எனில்( Tcl_GetIntFromObj(interp, objv[2], &ms) ) திரும்பு TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    நிறுத்து;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  நிலை DB_TOTAL_CHANGES: {
    Tcl_Obj *pResult;
    எனில்( objc!=2 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
      திரும்பு TCL_ERROR;
    }
    pResult = Tcl_GetObjResult(interp);
    Tcl_SetWideIntObj(pResult, sqlite3_total_changes64(pDb->db));
    நிறுத்து;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  நிலை DB_TRACE: {
    எனில்( objc>3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?CALLBACK?");
      திரும்பு TCL_ERROR;
    }இல்லையெனில் எனில்( objc==2 ){
      எனில்( pDb->zTrace ){
        Tcl_AppendResult(interp, pDb->zTrace, (எழுத்து*)0);
      }
    }இல்லையெனில்{
      எழுத்து *zTrace;
      Tcl_Size len;
      எனில்( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      எனில்( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTrace, zTrace, len+1);
      }இல்லையெனில்{
        pDb->zTrace = 0;
      }
#எனில் !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      எனில்( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }இல்லையெனில்{
        sqlite3_trace(pDb->db, 0, 0);
      }
#endif
    }
    நிறுத்து;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  நிலை DB_TRACE_V2: {
    எனில்( objc>4 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?CALLBACK? ?MASK?");
      திரும்பு TCL_ERROR;
    }இல்லையெனில் எனில்( objc==2 ){
      எனில்( pDb->zTraceV2 ){
        Tcl_AppendResult(interp, pDb->zTraceV2, (எழுத்து*)0);
      }
    }இல்லையெனில்{
      எழுத்து *zTraceV2;
      Tcl_Size len;
      Tcl_WideInt wMask = 0;
      எனில்( objc==4 ){
        நிலையான மாறிலி எழுத்து *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        எண்ணிக்கை TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_Size i;
        எனில்( TCL_OK!=Tcl_ListObjLength(interp, objv[3], &len) ){
          திரும்பு TCL_ERROR;
        }
        க்கு(i=0; i<len; i++){
          Tcl_Obj *pObj;
          முழு ttype;
          எனில்( TCL_OK!=Tcl_ListObjIndex(interp, objv[3], i, &pObj) ){
            திரும்பு TCL_ERROR;
          }
          எனில்( Tcl_GetIndexFromObj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
            Tcl_Incr_Ref_மொத்தம்(pError);
            எனில்( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_மொத்தம்(pError);
              wMask |= wType;
            }இல்லையெனில்{
              Tcl_SetObjResult(interp, pError);
              Tcl_Decr_Ref_மொத்தம்(pError);
              திரும்பு TCL_ERROR;
            }
          }இல்லையெனில்{
            தேர்வு( (எண்ணிக்கை TTYPE_enum)ttype ){
              நிலை TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    நிறுத்து;
              நிலை TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; நிறுத்து;
              நிலை TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     நிறுத்து;
              நிலை TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   நிறுத்து;
            }
          }
        }
      }இல்லையெனில்{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      எனில்( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      எனில்( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        memcpy(pDb->zTraceV2, zTraceV2, len+1);
      }இல்லையெனில்{
        pDb->zTraceV2 = 0;
      }
#எனில் !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      எனில்( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (unsigned)wMask, DbTraceV2Handler, pDb);
      }இல்லையெனில்{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#endif
    }
    நிறுத்து;
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
  நிலை DB_TRANSACTION: {
    Tcl_Obj *pScript;
    மாறிலி எழுத்து *zBegin = "SAVEPOINT _tcl_transaction";
    எனில்( objc!=3 && objc!=4 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "[TYPE] SCRIPT");
      திரும்பு TCL_ERROR;
    }

    எனில்( pDb->nTransaction==0 && objc==4 ){
      நிலையான மாறிலி எழுத்து *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      எண்ணிக்கை TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      முழு ttype;
      எனில்( Tcl_GetIndexFromObj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        திரும்பு TCL_ERROR;
      }
      தேர்வு( (எண்ணிக்கை TTYPE_enum)ttype ){
        நிலை TTYPE_DEFERRED:    /* no-op */;                 நிறுத்து;
        நிலை TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  நிறுத்து;
        நிலை TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  நிறுத்து;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    எனில்( rc!=SQLITE_OK ){
      Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    எனில்( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (வெறுமை)Tcl_NREvalObj(interp, pScript, 0);
    }இல்லையெனில்{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    நிறுத்து;
  }

  /*
  **    $db unlock_notify ?script?
  */
  நிலை DB_UNLOCK_NOTIFY: {
#ifndef SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_AppendResult(interp, "unlock_notify not available in this build",
                     (எழுத்து*)0);
    rc = TCL_ERROR;
#இல்லையெனில்
    எனில்( objc!=2 && objc!=3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }இல்லையெனில்{
      வெறுமை (*xNotify)(வெறுமை **, முழு) = 0;
      வெறுமை *pNotifyArg = 0;

      எனில்( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_மொத்தம்(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      எனில்( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (வெறுமை *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_மொத்தம்(pDb->pUnlockNotify);
      }

      எனில்( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (எழுத்து*)0);
        rc = TCL_ERROR;
      }
    }
#endif
    நிறுத்து;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  நிலை DB_PREUPDATE: {
#ifndef SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_AppendResult(interp, "preupdate_hook was omitted at compile-time", 
                     (எழுத்து*)0);
    rc = TCL_ERROR;
#இல்லையெனில்
    நிலையான மாறிலி எழுத்து *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    எண்ணிக்கை DbPreupdateSubCmd {
      PRE_மொத்தம், PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    முழு iSub;

    எனில்( objc<3 ){
      Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    எனில்( Tcl_GetIndexFromObj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      திரும்பு TCL_ERROR;
    }

    தேர்வு( (எண்ணிக்கை DbPreupdateSubCmd)iSub ){
      நிலை PRE_மொத்தம்: {
        முழு nCol = sqlite3_preupdate_மொத்தம்(pDb->db);
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nCol));
        நிறுத்து;
      }

      நிலை PRE_HOOK: {
        எனில்( objc>4 ){
          Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "hook ?SCRIPT?");
          திரும்பு TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        நிறுத்து;
      }

      நிலை PRE_DEPTH: {
        Tcl_Obj *pRet;
        எனில்( objc!=3 ){
          Tcl_Wrong_இலக்கம்_Args(interp, 3, objv, "");
          திரும்பு TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_SetObjResult(interp, pRet);
        நிறுத்து;
      }

      நிலை PRE_NEW:
      நிலை PRE_OLD: {
        முழு iIdx;
        sqlite3_மதிப்பு *p_மதிப்பு;
        எனில்( objc!=4 ){
          Tcl_Wrong_இலக்கம்_Args(interp, 3, objv, "INDEX");
          திரும்பு TCL_ERROR;
        }
        எனில்( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          திரும்பு TCL_ERROR;
        }

        எனில்( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_மதிப்பு);
        }இல்லையெனில்{
          ப்ரதிஜ்ஞா( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_மதிப்பு);
        }

        எனில்( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((எழுத்து*)sqlite3_மதிப்பு_text(p_மதிப்பு), -1);
          Tcl_SetObjResult(interp, pObj);
        }இல்லையெனில்{
          Tcl_AppendResult(interp, sqlite3_errmsg(pDb->db), (எழுத்து*)0);
          திரும்பு TCL_ERROR;
        }
      }
    }
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
    நிறுத்து;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  நிலை DB_WAL_HOOK:
  நிலை DB_UPDATE_HOOK:
  நிலை DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    எனில்( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    எனில்( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    எனில்( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    எனில்( objc>3 ){
       Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?SCRIPT?");
       திரும்பு TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    நிறுத்து;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  நிலை DB_VERSION: {
    முழு i;
    க்கு(i=2; i<objc; i++){
      மாறிலி எழுத்து *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ifdef SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      எனில்( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        எனில்( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          திரும்பு TCL_ERROR;
        }
      }இல்லையெனில்

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      எனில்( strcmp(zArg, "-last-stmt-ptr")==0 ){
        வெளி மாறிலி எழுத்து *sqlite3TestPtrToText(வெறுமை*);
        வெறுமை *ptr = (வெறுமை*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_SetResult(interp, (எழுத்து*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }இல்லையெனில்
#endif /* SQLITE_TEST */
      {
        Tcl_AppendResult(interp, "unknown argument: ", zArg, (எழுத்து*)0);
        திரும்பு TCL_ERROR;
      }
    }
    எனில்( i==2 ){   
      Tcl_SetResult(interp, (எழுத்து *)sqlite3_libversion(), TCL_STATIC);
    }
    நிறுத்து;
  }


  } /* End of the SWITCH statement */
  திரும்பு rc;
}

#எனில் SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
நிலையான முழு SQLITE_TCLAPI DbObjCmdAdaptor(
  வெறுமை *cd,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *மாறிலி*objv
){
  திரும்பு Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#endif /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
நிலையான முழு sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *மாறிலி*objv
){
  Tcl_Wrong_இலக்கம்_Args(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  திரும்பு TCL_ERROR;
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
நிலையான முழு SQLITE_TCLAPI DbMain(
  வெறுமை *cd,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *மாறிலி*objv
){
  SqliteDb *p;
  மாறிலி எழுத்து *zArg;
  எழுத்து *z_Err_செய்தி;
  முழு i;
  மாறிலி எழுத்து *zFile = 0;
  மாறிலி எழுத்து *zVfs = 0;
  முழு flags;
  முழு bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  முழு rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ifdef SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#இல்லையெனில்
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#endif

  எனில்( objc==1 ) திரும்பு sqliteCmdUsage(interp, objv);
  எனில்( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    எனில்( strcmp(zArg,"-version")==0 ){
      Tcl_AppendResult(interp,sqlite3_libversion(), (எழுத்து*)0);
      திரும்பு TCL_OK;
    }
    எனில்( strcmp(zArg,"-sourceid")==0 ){
      Tcl_AppendResult(interp,sqlite3_sourceid(), (எழுத்து*)0);
      திரும்பு TCL_OK;
    }
    எனில்( strcmp(zArg,"-has-codec")==0 ){
      Tcl_AppendResult(interp,"0",(எழுத்து*)0);
      திரும்பு TCL_OK;
    }
    எனில்( zArg[0]=='-' ) திரும்பு sqliteCmdUsage(interp, objv);
  }
  க்கு(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    எனில்( zArg[0]!='-' ){
      எனில்( zFile!=0 ) திரும்பு sqliteCmdUsage(interp, objv);
      zFile = zArg;
      தொடர்;
    }
    எனில்( i==objc-1 ) திரும்பு sqliteCmdUsage(interp, objv);
    i++;
    எனில்( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }இல்லையெனில் எனில்( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }இல்லையெனில் எனில்( strcmp(zArg, "-readonly")==0 ){
      முழு b;
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) திரும்பு TCL_ERROR;
      எனில்( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }இல்லையெனில்{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }இல்லையெனில் எனில்( strcmp(zArg, "-create")==0 ){
      முழு b;
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) திரும்பு TCL_ERROR;
      எனில்( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }இல்லையெனில்{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }இல்லையெனில் எனில்( strcmp(zArg, "-nofollow")==0 ){
      முழு b;
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) திரும்பு TCL_ERROR;
      எனில்( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }இல்லையெனில்{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }இல்லையெனில் எனில்( strcmp(zArg, "-nomutex")==0 ){
      முழு b;
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) திரும்பு TCL_ERROR;
      எனில்( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }இல்லையெனில்{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }இல்லையெனில் எனில்( strcmp(zArg, "-fullmutex")==0 ){
      முழு b;
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) திரும்பு TCL_ERROR;
      எனில்( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }இல்லையெனில்{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }இல்லையெனில் எனில்( strcmp(zArg, "-uri")==0 ){
      முழு b;
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) திரும்பு TCL_ERROR;
      எனில்( b ){
        flags |= SQLITE_OPEN_URI;
      }இல்லையெனில்{
        flags &= ~SQLITE_OPEN_URI;
      }
    }இல்லையெனில் எனில்( strcmp(zArg, "-translatefilename")==0 ){
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        திரும்பு TCL_ERROR;
      }
    }இல்லையெனில்{
      Tcl_AppendResult(interp, "unknown option: ", zArg, (எழுத்து*)0);
      திரும்பு TCL_ERROR;
    }
  }
  z_Err_செய்தி = 0;
  p = (SqliteDb*)Tcl_Alloc( அளவு(*p) );
  memset(p, 0, அளவு(*p));
  எனில்( zFile==0 ) zFile = "";
  எனில்( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  எனில்( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  எனில்( p->db ){
    எனில்( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_செய்தி = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }இல்லையெனில்{
    z_Err_செய்தி = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  எனில்( p->db==0 ){
    Tcl_SetResult(interp, z_Err_செய்தி, TCL_VOLATILE);
    Tcl_Free((எழுத்து*)p);
    sqlite3_free(z_Err_செய்தி);
    திரும்பு TCL_ERROR;
  }
  p->maxStmt = இலக்கம்_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  எனில்( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (எழுத்து*)p, DbDeleteCmd);
  }இல்லையெனில்{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (எழுத்து*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  திரும்பு TCL_OK;
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
EXTERN முழு Sqlite3_Init(Tcl_Interp *interp){
  முழு rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  எனில்( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ifndef SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#endif
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  திரும்பு rc;
}
EXTERN முழு Tclsqlite3_Init(Tcl_Interp *interp){ திரும்பு Sqlite3_Init(interp); }
EXTERN முழு Sqlite3_Unload(Tcl_Interp *interp, முழு flags){ திரும்பு TCL_OK; }
EXTERN முழு Tclsqlite3_Unload(Tcl_Interp *interp, முழு flags){ திரும்பு TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN முழு Sqlite3_SafeInit(Tcl_Interp *interp){ திரும்பு TCL_ERROR; }
EXTERN முழு Sqlite3_SafeUnload(Tcl_Interp *interp, முழு flags){திரும்பு TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN முழு Sqlite_Init(Tcl_Interp *interp){ திரும்பு Sqlite3_Init(interp);}
EXTERN முழு Tclsqlite_Init(Tcl_Interp *interp){ திரும்பு Sqlite3_Init(interp); }
EXTERN முழு Sqlite_Unload(Tcl_Interp *interp, முழு flags){ திரும்பு TCL_OK; }
EXTERN முழு Tclsqlite_Unload(Tcl_Interp *interp, முழு flags){ திரும்பு TCL_OK; }
EXTERN முழு Sqlite_SafeInit(Tcl_Interp *interp){ திரும்பு TCL_ERROR; }
EXTERN முழு Sqlite_SafeUnload(Tcl_Interp *interp, முழு flags){திரும்பு TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN முழு sqlite3_Init(Tcl_Interp *interp){ திரும்பு Sqlite3_Init(interp);}
EXTERN முழு sqlite_Init(Tcl_Interp *interp){ திரும்பு Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#எனில் defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
நிலையான மாறிலி எழுத்து *tclsh_main_loop(வெறுமை){
  நிலையான மாறிலி எழுத்து zMainloop[] =
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
  திரும்பு zMainloop;
}

#ifndef TCLSH_MAIN
# define TCLSH_MAIN முதன்மை
#endif
முழு SQLITE_CDECL TCLSH_MAIN(முழு argc, எழுத்து **argv){
  Tcl_Interp *interp;
  முழு i;
  மாறிலி எழுத்து *zScript = 0;
  எழுத்து zArgc[32];
#எனில் defined(TCLSH_INIT_PROC)
  வெளி மாறிலி எழுத்து *TCLSH_INIT_PROC(Tcl_Interp*);
#endif

#எனில் !defined(_WIN32_WCE)
  எனில்( getenv("SQLITE_DEBUG_BREAK") ){
    எனில்( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }இல்லையெனில்{
#எனில் defined(_WIN32) || defined(WIN32)
      DebugBreak();
#அல்லதெனில் defined(SIGTRAP)
      எழுப்பு(SIGTRAP);
#endif
    }
  }
#endif

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_SetSystemEncoding(வெற்று, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(அளவு(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  க்கு(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_மதிப்பு);
  }
#எனில் defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#endif
  எனில்( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  எனில்( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    மாறிலி எழுத்து *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    எனில்( zInfo==0 ) zInfo = Tcl_GetStringResult(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    திரும்பு 1;
  }
  திரும்பு 0;
}
#endif /* TCLSH */
