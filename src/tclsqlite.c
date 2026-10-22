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
#که_تعریف_شوی TCLSH_INIT_PROC
# define TCLSH 1
#د_که_پای

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#إذا defined(INCLUDE_MSVC_H)
# include "msvc.h"
#د_که_پای

/****** Copy of tclsqlite.h ******/
#إذا defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#وإلا
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#د_که_پای
/* Compatability between Tcl8.6 and Tcl9.0 */
#إذا TCL_MAJOR_نسخه==9
# define CONST ثابت
#وإلاإذا !defined(Tcl_اندازه)
  تعريفنوع صحيح Tcl_اندازه;
# ifndef Tcl_Bounce_Ref_شمېر
#  define Tcl_Bounce_Ref_شمېر(X) Tcl_Incr_Ref_شمېر(X); Tcl_Decr_Ref_شمېر(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#د_که_پای
/**** End copy of tclsqlite.h ****/

#ضمن <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#که_نه_تعریف_شوی SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  تعريفنوع بینښه حرف u8;
# ifndef SQLITE_PTRSIZE
#   إذا defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   وإلا
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# إذا defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    تعريفنوع uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    تعريفنوع بینښه صحيح uptr;
# وإلا
    تعريفنوع sqlite3_uint64 uptr;
# endif
#د_که_پای
#ضمن <ctype.h>

/* Used to get the current process ID */
#إذا !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#وإلاإذا !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_منځنۍ
#   define WIN32_LEAN_AND_منځنۍ
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (صحيح)GetCurrentProcessId
#د_که_پای

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#که_تعریف_شوی BUILD_sqlite
#تعریف_لغوه TCL_STORAGE_CLASS
#تعریف_کړه TCL_STORAGE_CLASS DLLEXPORT
#د_که_پای /* BUILD_sqlite */

#تعریف_کړه عدد_PREPARED_STMTS 10
#تعریف_کړه MAX_PREPARED_STMTS 100

/* Forward declaration */
تعريفنوع بنية SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#إذا defined(SQLITE_ENABLE_QRF_دننه_TCL)
#ضمن "qrf.h"
#د_که_پای

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
تعريفنوع بنية SqlFunc SqlFunc;
بنية SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  صحيح useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  صحيح eType;            /* Type of value to return */
  حرف *zName;          /* Name of this function */
  SqlFunc *p_بل;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
تعريفنوع بنية SqlCollate SqlCollate;
بنية SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  حرف *zScript;        /* The script to be run */
  SqlCollate *p_بل;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
تعريفنوع بنية SqlPreparedStmt SqlPreparedStmt;
بنية SqlPreparedStmt {
  SqlPreparedStmt *p_بل;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  صحيح nSql;                /* chars in zSql[] */
  ثابت حرف *zSql;        /* Text of the SQL statement */
  صحيح nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

تعريفنوع بنية IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
بنية SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  حرف *zBusy;               /* The busy callback routine */
  حرف *zCommit;             /* The commit hook callback routine */
  حرف *zTrace;              /* The trace callback routine */
  حرف *zTraceV2;            /* The trace_v2 callback routine */
  حرف *zProfile;            /* The profile callback routine */
  حرف *z_پرمختګ;           /* The progress callback routine */
  حرف *zBindFallback;       /* Callback to invoke on a binding miss */
  حرف *zAuth;               /* The authorization callback routine */
  صحيح disableAuth;           /* Disable the authorizer if it exists */
  حرف *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  صحيح rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmt_وروستی; /* Last statement in the list */
  صحيح maxStmt;               /* The next maximum number of stmtList */
  صحيح nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  صحيح n_ګام, n_ترتیبول, n_شاخص;  /* Statistics for most recent operation */
  صحيح nVMStep;               /* Another statistic for most recent operation */
  صحيح nTransaction;          /* Number of nested [transaction] methods */
  صحيح openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  صحيح nRef;                  /* Delete object when this reaches 0 */
#که_تعریف_شوی SQLITE_TEST
  صحيح bLegacyPrepare;        /* True to use sqlite3_prepare() */
#د_که_پای
};

بنية IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  بینښه صحيح isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *p_بل;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
ساكن صحيح strlen30(ثابت حرف *z){
  ثابت حرف *z2 = z;
  طالما( *z2 ){ z2++; }
  أرجع 0x3fffffff & (صحيح)(z2 - z);
}


#که_نه_تعریف_شوی SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
ساكن فراغ closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *p_بل;

  لكل(p=pDb->pIncrblob; p; p=p_بل){
    p_بل = p->p_بل;

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
ساكن صحيح SQLITE_TCLAPI incrblobClose2(
  Client_معلومات instance_معلومات,
  Tcl_Interp *interp,
  صحيح flags
){
  IncrblobChannel *p = (IncrblobChannel *)instance_معلومات;
  صحيح  rc;
  sqlite3 *db = p->pDb->db;

  إذا( flags ){
    p->isClosed |= flags;
    أرجع TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  إذا( p->p_بل ){
    p->p_بل->pPrev = p->pPrev;
  }
  إذا( p->pPrev ){
    p->pPrev->p_بل = p->p_بل;
  }
  إذا( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->p_بل;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((حرف *)p);

  إذا( rc!=SQLITE_OK ){
    Tcl_سېټ_پایله(interp, (حرف *)sqlite3_errmsg(db), TCL_VOLATILE);
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}
ساكن صحيح SQLITE_TCLAPI incrblobClose(
  Client_معلومات instance_معلومات,
  Tcl_Interp *interp
){
  أرجع incrblobClose2(instance_معلومات, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
ساكن صحيح SQLITE_TCLAPI incrblob_ننوت(
  Client_معلومات instance_معلومات,
  حرف *buf,
  صحيح buf_اندازه,
  صحيح *error_کوډ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_معلومات;
  sqlite3_int64 nRead = buf_اندازه;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  صحيح rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_بایټونه(p->pBlob);
  إذا( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  إذا( nRead<=0 ){
    أرجع 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (فراغ *)buf, (صحيح)nRead, (صحيح)p->iSeek);
  إذا( rc!=SQLITE_OK ){
    *error_کوډ_Ptr = rc;
    أرجع -1;
  }

  p->iSeek += nRead;
  أرجع nRead;
}

/*
** Write data to an incremental blob channel.
*/
ساكن صحيح SQLITE_TCLAPI incrblobOutput(
  Client_معلومات instance_معلومات,
  ثابت حرف *buf,
  صحيح منزل_Write,
  صحيح *error_کوډ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_معلومات;
  sqlite3_int64 nWrite = منزل_Write;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  صحيح rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_بایټونه(p->pBlob);
  إذا( (p->iSeek+nWrite)>nBlob ){
    *error_کوډ_Ptr = EINVAL;
    أرجع -1;
  }
  إذا( nWrite<=0 ){
    أرجع 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (فراغ*)buf,(صحيح)nWrite, (صحيح)p->iSeek);
  إذا( rc!=SQLITE_OK ){
    *error_کوډ_Ptr = EIO;
    أرجع -1;
  }

  p->iSeek += nWrite;
  أرجع nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#إذا TCL_MAJOR_نسخه==9
# define WideSeekProcType اوږد اوږد
#وإلا
# define WideSeekProcType Tcl_WideInt
#د_که_پای

/*
** Seek an incremental blob channel.
*/
ساكن WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  Client_معلومات instance_معلومات,
  WideSeekProcType دننه_واټن,
  صحيح seek_ډېرپېښ,
  صحيح *error_کوډ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_معلومات;

  بدل( seek_ډېرپېښ ){
    حالة SEEK_SET:
      p->iSeek = دننه_واټن;
      اكسر;
    حالة SEEK_CUR:
      p->iSeek += دننه_واټن;
      اكسر;
    حالة SEEK_END:
      p->iSeek = sqlite3_blob_بایټونه(p->pBlob) + دننه_واټن;
      اكسر;

    افتراضي: أكد(!"Bad seekMode");
  }

  أرجع p->iSeek;
}
ساكن صحيح SQLITE_TCLAPI incrblobSeek(
  Client_معلومات instance_معلومات,
  اوږد دننه_واټن,
  صحيح seek_ډېرپېښ,
  صحيح *error_کوډ_Ptr
){
  أرجع incrblobWideSeek(instance_معلومات,دننه_واټن,seek_ډېرپېښ,error_کوډ_Ptr);
}


ساكن فراغ SQLITE_TCLAPI incrblobWatch(
  Client_معلومات instance_معلومات,
  صحيح ډېرپېښ
){
  /* NO-OP */
}
ساكن صحيح SQLITE_TCLAPI incrblobHandle(
  Client_معلومات instance_معلومات,
  صحيح dir,
  Client_معلومات *hPtr
){
  أرجع TCL_ERROR;
}

ساكن Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_نسخه_5,             /*نسخه*/
  incrblobClose,                     /* closeProc                            */
  incrblob_ننوت,                     /* inputProc                            */
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
ساكن صحيح createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  ثابت حرف *zDb,
  ثابت حرف *z_جدول,
  ثابت حرف *z_ستنه,
  sqlite_int64 i_لیکه,
  صحيح isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  صحيح rc;
  صحيح flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  ساكن صحيح شمېر = 0;
  حرف zChannel[64];

  rc = sqlite3_blob_open(db, zDb, z_جدول, z_ستنه, i_لیکه, !isReadonly, &pBlob);
  إذا( rc!=SQLITE_OK ){
    Tcl_سېټ_پایله(interp, (حرف *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    أرجع TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(حجم(IncrblobChannel));
  memset(p, 0, حجم(*p));
  p->pBlob = pBlob;
  إذا( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(حجم(zChannel), zChannel, "incrblob_%d", ++شمېر);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_ثبتوونکی_Channel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->p_بل = pDb->pIncrblob;
  p->pPrev = 0;
  إذا( p->p_بل ){
    p->p_بل->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_سېټ_پایله(interp, (حرف *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  أرجع TCL_OK;
}
#وإلا  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
  #تعریف_کړه closeIncrblobChannels(pDb)
#د_که_پای

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
ساكن صحيح خوندي_منزل_Use_Eval_Objv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  ثابت حرف *z;
  Tcl_اندازه n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  طالما( n-- > 0 ){
    صحيح c = *(z++);
    إذا( c=='$' || c=='[' || c==';' ) أرجع 0;
  }
  أرجع 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
ساكن SqlFunc *پیدا_کړه_Sql_Func(SqliteDb *pDb, ثابت حرف *zName){
  SqlFunc *p, *pNew;
  صحيح nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( حجم(*pNew) + nName + 1 );
  pNew->zName = (حرف*)&pNew[1];
  حافظه_نقل(pNew->zName, zName, nName+1);
  لكل(p=pDb->pFunc; p; p=p->p_بل){
    إذا( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((حرف*)pNew);
      أرجع p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->p_بل = pDb->pFunc;
  pDb->pFunc = pNew;
  أرجع pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
ساكن فراغ dbFreeStmt(SqlPreparedStmt *pStmt){
#که_تعریف_شوی SQLITE_TEST
  إذا( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((حرف *)pStmt->zSql);
  }
#د_که_پای
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((حرف *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
ساكن فراغ flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *p_بل;

  لكل(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=p_بل){
    p_بل = pPreStmt->p_بل;
    dbFreeStmt(pPreStmt);
  }
  pDb->nStmt = 0;
  pDb->stmt_وروستی = 0;
  pDb->stmtList = 0;
}

/*
** Increment the reference counter on the SqliteDb object. The reference
** should be released by calling delDatabaseRef().
*/
ساكن فراغ جمع_کړه_Database_Ref(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
ساكن فراغ delDatabaseRef(SqliteDb *pDb){
  أكد( pDb->nRef>0 );
  pDb->nRef--;
  إذا( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    طالما( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->p_بل;
      أكد( pFunc->pDb==pDb );
      Tcl_Decr_Ref_شمېر(pFunc->pScript);
      Tcl_Free((حرف*)pFunc);
    }
    طالما( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->p_بل;
      Tcl_Free((حرف*)pCollate);
    }
    إذا( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    إذا( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    إذا( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    إذا( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    إذا( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    إذا( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    إذا( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    إذا( pDb->pUpdateHook ){
      Tcl_Decr_Ref_شمېر(pDb->pUpdateHook);
    }
    إذا( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_شمېر(pDb->pPreUpdateHook);
    }
    إذا( pDb->pRollbackHook ){
      Tcl_Decr_Ref_شمېر(pDb->pRollbackHook);
    }
    إذا( pDb->pWalHook ){
      Tcl_Decr_Ref_شمېر(pDb->pWalHook);
    }
    إذا( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_شمېر(pDb->pCollateNeeded);
    }
    Tcl_Free((حرف*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
ساكن فراغ SQLITE_TCLAPI DbDeleteCmd(فراغ *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
ساكن صحيح DbBusyHandler(فراغ *cd, صحيح nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحيح rc;
  حرف zVal[30];

  sqlite3_snprintf(حجم(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (حرف*)0);
  إذا( rc!=TCL_OK || atoi(Tcl_Get_String_پایله(pDb->interp)) ){
    أرجع 0;
  }
  أرجع 1;
}

#که_نه_تعریف_شوی SQLITE_OMIT_پرمختګ_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
ساكن صحيح Db_پرمختګ_Handler(فراغ *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحيح rc;

  أكد( pDb->z_پرمختګ );
  rc = Tcl_Eval(pDb->interp, pDb->z_پرمختګ);
  إذا( rc!=TCL_OK || atoi(Tcl_Get_String_پایله(pDb->interp)) ){
    أرجع 1;
  }
  أرجع 0;
}
#د_که_پای

#إذا !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ټکی) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
ساكن فراغ DbTraceHandler(فراغ *cd, ثابت حرف *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_ارزښت(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_پایله(pDb->interp);
}
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
ساكن صحيح DbTraceV2Handler(
  بینښه type, /* One of the SQLITE_TRACE_* event types. */
  فراغ *cd,      /* The original context data pointer. */
  فراغ *pd,      /* Primary event data, depends on event type. */
  فراغ *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  بدل( type ){
    حالة SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      حرف *zSql = (حرف *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_شمېر(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_مستقیم);
      Tcl_Decr_Ref_شمېر(pCmd);
      Tcl_Reset_پایله(pDb->interp);
      اكسر;
    }
    حالة SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_شمېر(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_مستقیم);
      Tcl_Decr_Ref_شمېر(pCmd);
      Tcl_Reset_پایله(pDb->interp);
      اكسر;
    }
    حالة SQLITE_TRACE_لیکه: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_شمېر(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_مستقیم);
      Tcl_Decr_Ref_شمېر(pCmd);
      Tcl_Reset_پایله(pDb->interp);
      اكسر;
    }
    حالة SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_شمېر(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_مستقیم);
      Tcl_Decr_Ref_شمېر(pCmd);
      Tcl_Reset_پایله(pDb->interp);
      اكسر;
    }
  }
  أرجع SQLITE_OK;
}
#د_که_پای

#إذا !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ټکی) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
ساكن فراغ DbProfileHandler(فراغ *cd, ثابت حرف *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  حرف zTm[100];

  sqlite3_snprintf(حجم(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_ارزښت(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_پایله(pDb->interp);
}
#د_که_پای

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
ساكن صحيح DbCommitHandler(فراغ *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحيح rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  إذا( rc!=TCL_OK || atoi(Tcl_Get_String_پایله(pDb->interp)) ){
    أرجع 1;
  }
  أرجع 0;
}

ساكن فراغ DbRollbackHandler(فراغ *client_معلومات){
  SqliteDb *pDb = (SqliteDb*)client_معلومات;
  أكد(pDb->pRollbackHook);
  إذا( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
ساكن صحيح DbWalHandler(
  فراغ *client_معلومات,
  sqlite3 *db,
  ثابت حرف *zDb,
  صحيح n_ثبت
){
  صحيح ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)client_معلومات;
  Tcl_Interp *interp = pDb->interp;
  أكد(pDb->pWalHook);

  أكد( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_شمېر(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(n_ثبت));
  إذا( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_Get_Obj_پایله(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_شمېر(p);

  أرجع ret;
}

#إذا defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ساكن فراغ سېټ_Test_Unlock_Notify_Vars(Tcl_Interp *interp, صحيح iArg, صحيح nArg){
  حرف zBuf[64];
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", iArg);
  Tcl_سېټ_Var(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", nArg);
  Tcl_سېټ_Var(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#وإلا
# define سېټ_Test_Unlock_Notify_Vars(x,y,z)
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_UNLOCK_NOTIFY
ساكن فراغ DbUnlockNotify(فراغ **apArg, صحيح nArg){
  صحيح i;
  لكل(i=0; i<nArg; i++){
    ثابت صحيح flags = (TCL_EVAL_GLOBAL|TCL_EVAL_مستقیم);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    سېټ_Test_Unlock_Notify_Vars(pDb->interp, i, nArg);
    أكد( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_شمېر(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
ساكن فراغ DbPreUpdateHandler(
  فراغ *p,
  sqlite3 *db,
  صحيح op,
  ثابت حرف *zDb,
  ثابت حرف *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ساكن ثابت حرف *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  أكد( (SQLITE_DELETE-1)/9 == 0 );
  أكد( (SQLITE_ورننباسه-1)/9 == 1 );
  أكد( (SQLITE_UPDATE-1)/9 == 2 );
  أكد( pDb->pPreUpdateHook );
  أكد( db==pDb->db );
  أكد( op==SQLITE_ورننباسه || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_شمېر(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_مستقیم);
  Tcl_Decr_Ref_شمېر(pCmd);
}
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */

ساكن فراغ DbUpdateHandler(
  فراغ *p,
  صحيح op,
  ثابت حرف *zDb,
  ثابت حرف *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ساكن ثابت حرف *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  أكد( (SQLITE_DELETE-1)/9 == 0 );
  أكد( (SQLITE_ورننباسه-1)/9 == 1 );
  أكد( (SQLITE_UPDATE-1)/9 == 2 );

  أكد( pDb->pUpdateHook );
  أكد( op==SQLITE_ورننباسه || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_شمېر(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_مستقیم);
  Tcl_Decr_Ref_شمېر(pCmd);
}

ساكن فراغ tclCollateNeeded(
  فراغ *pCtx,
  sqlite3 *db,
  صحيح enc,
  ثابت حرف *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_شمېر(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_شمېر(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
ساكن صحيح tclSqlCollate(
  فراغ *pCtx,
  صحيح nA,
  ثابت فراغ *zA,
  صحيح nB,
  ثابت فراغ *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_شمېر(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_مستقیم);
  Tcl_Decr_Ref_شمېر(pCmd);
  أرجع (atoi(Tcl_Get_String_پایله(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
ساكن فراغ tclSqlFunc(sqlite3_context *context, صحيح argc, sqlite3_ارزښت**argv){
  SqlFunc *p = sqlite3_user_معلومات(context);
  Tcl_Obj *pCmd;
  صحيح i;
  صحيح rc;

  إذا( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_شمېر(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_شمېر(pCmd);
  }وإلا{
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
    Tcl_اندازه nArg;
    إذا( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_پایله_error(context, Tcl_Get_String_پایله(p->interp), -1);
      أرجع;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_شمېر(pCmd);
    لكل(i=0; i<argc; i++){
      sqlite3_ارزښت *p_دننه = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      بدل( sqlite3_ارزښت_type(p_دننه) ){
        حالة SQLITE_BLOB: {
          صحيح بایټونه = sqlite3_ارزښت_بایټونه(p_دننه);
          pVal = Tcl_NewByteArrayObj(sqlite3_ارزښت_blob(p_دننه), بایټونه);
          اكسر;
        }
        حالة SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_ارزښت_int64(p_دننه);
          إذا( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((صحيح)v);
          }وإلا{
            pVal = Tcl_NewWideIntObj(v);
          }
          اكسر;
        }
        حالة SQLITE_FLOAT: {
          مزدوج r = sqlite3_ارزښت_double(p_دننه);
          pVal = Tcl_NewDoubleObj(r);
          اكسر;
        }
        حالة SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          اكسر;
        }
        افتراضي: {
          صحيح بایټونه = sqlite3_ارزښت_بایټونه(p_دننه);
          pVal = Tcl_NewStringObj((حرف *)sqlite3_ارزښت_متن(p_دننه), بایټونه);
          اكسر;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      إذا( rc ){
        Tcl_Decr_Ref_شمېر(pCmd);
        sqlite3_پایله_error(context, Tcl_Get_String_پایله(p->interp), -1);
        أرجع;
      }
    }
    إذا( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_مستقیم);
    Tcl_Decr_Ref_شمېر(pCmd);
  }

  إذا( TCL_BREAK==rc ){
    sqlite3_پایله_null(context);
  }وإلا إذا( rc && rc!=TCL_RETURN ){
    sqlite3_پایله_error(context, Tcl_Get_String_پایله(p->interp), -1);
  }وإلا{
    Tcl_Obj *pVar = Tcl_Get_Obj_پایله(p->interp);
    Tcl_اندازه n;
    u8 *معلومات;
    ثابت حرف *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    حرف c = zType[0];
    صحيح eType = p->eType;

    إذا( eType==SQLITE_NULL ){
      إذا( c=='b' && strcmp(zType,"bytearray")==0 && pVar->بایټونه==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }وإلا إذا( (c=='b' && pVar->بایټونه==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->بایټونه==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }وإلا إذا( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }وإلا{
        eType = SQLITE_متن;
      }
    }

    بدل( eType ){
      حالة SQLITE_BLOB: {
        معلومات = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_پایله_blob64(context, معلومات, n, SQLITE_TRANSIENT);
        اكسر;
      }
      حالة SQLITE_INTEGER: {
        Tcl_WideInt v;
        إذا( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_پایله_int64(context, v);
          اكسر;
        }
        /* fall-through */
      }
      حالة SQLITE_FLOAT: {
        مزدوج r;
        إذا( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_پایله_double(context, r);
          اكسر;
        }
        /* fall-through */
      }
      افتراضي: {
        معلومات = (بینښه حرف *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_پایله_text64(context, (حرف *)معلومات, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        اكسر;
      }
    }

  }
}

#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
ساكن صحيح auth_callback(
  فراغ *pArg,
  صحيح کوډ,
  ثابت حرف *zArg1,
  ثابت حرف *zArg2,
  ثابت حرف *zArg3,
  ثابت حرف *zArg4
){
  ثابت حرف *z_کوډ;
  Tcl_DString str;
  صحيح rc;
  ثابت حرف *z_ځواب;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  إذا( pDb->disableAuth ) أرجع SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  بدل( کوډ ){
    حالة SQLITE_COPY              : z_کوډ="SQLITE_COPY"; اكسر;
    حالة SQLITE_CREATE_شاخص      : z_کوډ="SQLITE_CREATE_INDEX"; اكسر;
    حالة SQLITE_CREATE_جدول      : z_کوډ="SQLITE_CREATE_TABLE"; اكسر;
    حالة SQLITE_CREATE_لنډمهاله_شاخص : z_کوډ="SQLITE_CREATE_TEMP_INDEX"; اكسر;
    حالة SQLITE_CREATE_لنډمهاله_جدول : z_کوډ="SQLITE_CREATE_TEMP_TABLE"; اكسر;
    حالة SQLITE_CREATE_لنډمهاله_TRIGGER: z_کوډ="SQLITE_CREATE_TEMP_TRIGGER"; اكسر;
    حالة SQLITE_CREATE_لنډمهاله_VIEW  : z_کوډ="SQLITE_CREATE_TEMP_VIEW"; اكسر;
    حالة SQLITE_CREATE_TRIGGER    : z_کوډ="SQLITE_CREATE_TRIGGER"; اكسر;
    حالة SQLITE_CREATE_VIEW       : z_کوډ="SQLITE_CREATE_VIEW"; اكسر;
    حالة SQLITE_DELETE            : z_کوډ="SQLITE_DELETE"; اكسر;
    حالة SQLITE_DROP_شاخص        : z_کوډ="SQLITE_DROP_INDEX"; اكسر;
    حالة SQLITE_DROP_جدول        : z_کوډ="SQLITE_DROP_TABLE"; اكسر;
    حالة SQLITE_DROP_لنډمهاله_شاخص   : z_کوډ="SQLITE_DROP_TEMP_INDEX"; اكسر;
    حالة SQLITE_DROP_لنډمهاله_جدول   : z_کوډ="SQLITE_DROP_TEMP_TABLE"; اكسر;
    حالة SQLITE_DROP_لنډمهاله_TRIGGER : z_کوډ="SQLITE_DROP_TEMP_TRIGGER"; اكسر;
    حالة SQLITE_DROP_لنډمهاله_VIEW    : z_کوډ="SQLITE_DROP_TEMP_VIEW"; اكسر;
    حالة SQLITE_DROP_TRIGGER      : z_کوډ="SQLITE_DROP_TRIGGER"; اكسر;
    حالة SQLITE_DROP_VIEW         : z_کوډ="SQLITE_DROP_VIEW"; اكسر;
    حالة SQLITE_ورننباسه            : z_کوډ="SQLITE_INSERT"; اكسر;
    حالة SQLITE_PRAGMA            : z_کوډ="SQLITE_PRAGMA"; اكسر;
    حالة SQLITE_READ              : z_کوډ="SQLITE_READ"; اكسر;
    حالة SQLITE_SELECT            : z_کوډ="SQLITE_SELECT"; اكسر;
    حالة SQLITE_TRANSACTION       : z_کوډ="SQLITE_TRANSACTION"; اكسر;
    حالة SQLITE_UPDATE            : z_کوډ="SQLITE_UPDATE"; اكسر;
    حالة SQLITE_ATTACH            : z_کوډ="SQLITE_ATTACH"; اكسر;
    حالة SQLITE_DETACH            : z_کوډ="SQLITE_DETACH"; اكسر;
    حالة SQLITE_ALTER_جدول       : z_کوډ="SQLITE_ALTER_TABLE"; اكسر;
    حالة SQLITE_REINDEX           : z_کوډ="SQLITE_REINDEX"; اكسر;
    حالة SQLITE_ANALYZE           : z_کوډ="SQLITE_ANALYZE"; اكسر;
    حالة SQLITE_CREATE_VTABLE     : z_کوډ="SQLITE_CREATE_VTABLE"; اكسر;
    حالة SQLITE_DROP_VTABLE       : z_کوډ="SQLITE_DROP_VTABLE"; اكسر;
    حالة SQLITE_FUNCTION          : z_کوډ="SQLITE_FUNCTION"; اكسر;
    حالة SQLITE_SAVEPOINT         : z_کوډ="SQLITE_SAVEPOINT"; اكسر;
    حالة SQLITE_RECURSIVE         : z_کوډ="SQLITE_RECURSIVE"; اكسر;
    افتراضي                       : z_کوډ="????"; اكسر;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, z_کوډ);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_ارزښت(&str));
  Tcl_DStringFree(&str);
  z_ځواب = rc==TCL_OK ? Tcl_Get_String_پایله(pDb->interp) : "SQLITE_DENY";
  إذا( strcmp(z_ځواب,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }وإلا إذا( strcmp(z_ځواب,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }وإلا إذا( strcmp(z_ځواب,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }وإلا{
    rc = 999;
  }
  أرجع rc;
}
#د_که_پای /* SQLITE_OMIT_AUTHORIZATION */

#إذا 0
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
ساكن حرف *local_getline(حرف *zPrompt, FILE *دننه){
  حرف *z_کرښه;
  صحيح n_کرښه;
  صحيح n;

  n_کرښه = 100;
  z_کرښه = حافظه_ورکړه( n_کرښه );
  إذا( z_کرښه==0 ) أرجع 0;
  n = 0;
  طالما( 1 ){
    إذا( n+100>n_کرښه ){
      n_کرښه = n_کرښه*2 + 100;
      z_کرښه = realloc(z_کرښه, n_کرښه);
      إذا( z_کرښه==0 ) أرجع 0;
    }
    إذا( fgets(&z_کرښه[n], n_کرښه - n, دننه)==0 ){
      إذا( n==0 ){
        حرر(z_کرښه);
        أرجع 0;
      }
      z_کرښه[n] = 0;
      اكسر;
    }
    طالما( z_کرښه[n] ){ n++; }
    إذا( n>0 && z_کرښه[n-1]=='\n' ){
      n--;
      z_کرښه[n] = 0;
      اكسر;
    }
  }
  z_کرښه = realloc( z_کرښه, n+1 );
  أرجع z_کرښه;
}
#د_که_پای


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
ساكن صحيح SQLITE_TCLAPI DbTransPostCmd(
  Client_معلومات معلومات[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  صحيح پایله                           /* Result of evaluating SCRIPT */
){
  ساكن ثابت حرف *ثابت az_پای_ټکی[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)معلومات[0];
  صحيح rc = پایله;
  ثابت حرف *z_پای_ټکی;

  pDb->nTransaction--;
  z_پای_ټکی = az_پای_ټکی[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  إذا( sqlite3_exec(pDb->db, z_پای_ټکی, 0, 0, 0) ){
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
    إذا( rc!=TCL_ERROR ){
      Tcl_Append_پایله(interp, sqlite3_errmsg(pDb->db), (حرف*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  أرجع rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
ساكن صحيح dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  ثابت حرف *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  ثابت حرف **pz_بهر              /* OUT: Pointer to next SQL statement */
){
  بینښه صحيح prepFlags = 0;
#که_تعریف_شوی SQLITE_TEST
  إذا( pDb->bLegacyPrepare ){
    أرجع sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pz_بهر);
  }
#د_که_پای
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  إذا( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  أرجع sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pz_بهر);
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
ساكن صحيح dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  حرف ثابت *z_دننه,                /* SQL to compile */
  حرف ثابت **pz_بهر,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  ثابت حرف *zSql = z_دننه;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  صحيح nSql;                       /* Length of zSql in bytes */
  صحيح nVar = 0;                   /* Number of variables in statement */
  صحيح iParm = 0;                  /* Next free entry in apParm */
  حرف c;
  صحيح i;
  صحيح need_پایله_Reset = 0;        /* Need to invoke Tcl_ResetResult() */
  صحيح rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  طالما( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  لكل(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->p_بل){
    صحيح n = pPreStmt->nSql;
    إذا( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pz_بهر = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      إذا( pPreStmt->pPrev ){
        pPreStmt->pPrev->p_بل = pPreStmt->p_بل;
      }وإلا{
        pDb->stmtList = pPreStmt->p_بل;
      }
      إذا( pPreStmt->p_بل ){
        pPreStmt->p_بل->pPrev = pPreStmt->pPrev;
      }وإلا{
        pDb->stmt_وروستی = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_شمېر(pStmt);
      اكسر;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  إذا( pPreStmt==0 ){
    صحيح nByte;

    إذا( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pz_بهر) ){
      Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      أرجع TCL_ERROR;
    }
    إذا( pStmt==0 ){
      إذا( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        أرجع TCL_ERROR;
      }وإلا{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        أرجع TCL_OK;
      }
    }

    أكد( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_شمېر(pStmt);
    nByte = حجم(SqlPreparedStmt) + nVar*حجم(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (صحيح)(*pz_بهر - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#که_تعریف_شوی SQLITE_TEST
    إذا( pPreStmt->zSql==0 ){
      حرف *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      حافظه_نقل(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#د_که_پای
  }
  أكد( pPreStmt );
  أكد( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  أكد( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  لكل(i=1; i<=nVar; i++){
    ثابت حرف *zVar = sqlite3_bind_parameter_name(pStmt, i);
    إذا( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      إذا( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        صحيح rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_شمېر(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        إذا( need_پایله_Reset ) Tcl_Reset_پایله(interp);
        need_پایله_Reset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_مستقیم);
        Tcl_Decr_Ref_شمېر(pCmd);
        إذا( rx==TCL_OK ){
          pVar = Tcl_Get_Obj_پایله(interp);
        }وإلا إذا( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          اكسر;
        }وإلا{
          pVar = 0;
        }
      }
      إذا( pVar ){
        Tcl_اندازه n;
        u8 *معلومات;
        ثابت حرف *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        إذا( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->بایټونه==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          معلومات = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, معلومات, n, SQLITE_STATIC);
          Tcl_Incr_Ref_شمېر(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }وإلا إذا( c=='b' && pVar->بایټونه==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          صحيح nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }وإلا إذا( c=='d' && strcmp(zType,"double")==0 ){
          مزدوج r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }وإلا إذا( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }وإلا{
          معلومات = (بینښه حرف *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (حرف *)معلومات, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_شمېر(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }وإلا{
        sqlite3_bind_null(pStmt, i);
      }
      إذا( need_پایله_Reset ) Tcl_Reset_پایله(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  إذا( need_پایله_Reset && rc==TCL_OK ) Tcl_Reset_پایله(pDb->interp);

  أرجع rc;
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
ساكن فراغ dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  صحيح discard                     /* True to delete (not cache) the pPreStmt */
){
  صحيح i;

  /* Free the bound string and blob parameters */
  لكل(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_شمېر(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  إذا( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }وإلا{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->p_بل = pDb->stmtList;
    pPreStmt->pPrev = 0;
    إذا( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    إذا( pDb->stmt_وروستی==0 ){
      أكد( pDb->nStmt==0 );
      pDb->stmt_وروستی = pPreStmt;
    }وإلا{
      أكد( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    طالما( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *p_وروستی = pDb->stmt_وروستی;
      pDb->stmt_وروستی = p_وروستی->pPrev;
      pDb->stmt_وروستی->p_بل = 0;
      pDb->nStmt--;
      dbFreeStmt(p_وروستی);
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
تعريفنوع بنية DbEvalContext DbEvalContext;
بنية DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  ثابت حرف *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  صحيح n_ستون;                       /* Number of columns returned by pStmt */
  صحيح evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **ap_ستون_Name;            /* Array of column names */
};

#تعریف_کړه SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#تعریف_کړه SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
ساكن فراغ db_Release_ستنه_نومونه(DbEvalContext *p){
  إذا( p->ap_ستون_Name ){
    صحيح i;
    لكل(i=0; i<p->n_ستون; i++){
      Tcl_Decr_Ref_شمېر(p->ap_ستون_Name[i]);
    }
    Tcl_Free((حرف *)p->ap_ستون_Name);
    p->ap_ستون_Name = 0;
  }
  p->n_ستون = 0;
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
ساكن فراغ dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  صحيح evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, حجم(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_شمېر(pSql);
  إذا( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_شمېر(pVarName);
  }
  p->evalFlags = evalFlags;
  جمع_کړه_Database_Ref(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
ساكن فراغ db_Eval_لیکه_Info(
  DbEvalContext *p,               /* Evaluation context */
  صحيح *pn_ستون,                     /* OUT: Number of column names */
  Tcl_Obj ***pap_ستون_Name           /* OUT: Array of column names */
){
  /* Compute column names */
  إذا( 0==p->ap_ستون_Name ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    صحيح i;                        /* Iterator variable */
    صحيح n_ستون;                     /* Number of columns returned by pStmt */
    Tcl_Obj **ap_ستون_Name = 0;      /* Array of column names */

    p->n_ستون = n_ستون = sqlite3_ستنه_شمېر(pStmt);
    إذا( n_ستون>0 && (pap_ستون_Name || p->pVarName) ){
      ap_ستون_Name = (Tcl_Obj**)Tcl_Alloc( حجم(Tcl_Obj*)*n_ستون );
      لكل(i=0; i<n_ستون; i++){
        ap_ستون_Name[i] = Tcl_NewStringObj(sqlite3_ستنه_name(pStmt,i), -1);
        Tcl_Incr_Ref_شمېر(ap_ستون_Name[i]);
      }
      p->ap_ستون_Name = ap_ستون_Name;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    إذا( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *p_ستون_List = Tcl_NewObj();
      Tcl_Obj *p_ستوری = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_شمېر(p_ستون_List);
      Tcl_Incr_Ref_شمېر(p_ستوری);
      لكل(i=0; i<n_ستون; i++){
        Tcl_ListObjAppendElement(interp, p_ستون_List, ap_ستون_Name[i]);
      }
      إذا( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_Obj_سېټ_Var2(interp, p->pVarName, p_ستوری, p_ستون_List, 0);
      }وإلا{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, عدم, 0);
        إذا( !pDict ){
          pDict = Tcl_NewDictObj();
        }وإلا إذا( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        إذا( Tcl_DictObjPut(interp, pDict, p_ستوری, p_ستون_List)==TCL_OK ){
          Tcl_Obj_سېټ_Var2(interp, p->pVarName, عدم, pDict, 0);
        }
        Tcl_Bounce_Ref_شمېر(pDict);
      }
      Tcl_Decr_Ref_شمېر(p_ستوری);
      Tcl_Decr_Ref_شمېر(p_ستون_List);
    }
  }

  إذا( pap_ستون_Name ){
    *pap_ستون_Name = p->ap_ستون_Name;
  }
  إذا( pn_ستون ){
    *pn_ستون = p->n_ستون;
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
ساكن صحيح db_Eval_ګام(DbEvalContext *p){
  ثابت حرف *zPrevSql = 0;       /* Previous value of p->zSql */

  طالما( p->zSql[0] || p->pPreStmt ){
    صحيح rc;
    إذا( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      إذا( rc!=TCL_OK ) أرجع rc;
    }وإلا{
      صحيح rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_ګام(pStmt);
      إذا( rcs==SQLITE_لیکه ){
        أرجع TCL_OK;
      }
      إذا( p->pVarName ){
        db_Eval_لیکه_Info(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->n_ګام = sqlite3_stmt_حالت(pStmt,SQLITE_STMTSTATUS_FULLSCAN_ګام,1);
      pDb->n_ترتیبول = sqlite3_stmt_حالت(pStmt,SQLITE_STMTSTATUS_ترتیبول,1);
      pDb->n_شاخص = sqlite3_stmt_حالت(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_حالت(pStmt,SQLITE_STMTSTATUS_VM_ګام,1);
      db_Release_ستنه_نومونه(p);
      p->pPreStmt = 0;

      إذا( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#إذا SQLITE_TEST
        إذا( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          استمر;
        }
#د_که_پای
        Tcl_سېټ_Obj_پایله(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        أرجع TCL_ERROR;
      }وإلا{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /*پای_ته_رسېدلی*/
  أرجع TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
ساكن فراغ dbEvalFinalize(DbEvalContext *p){
  إذا( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  إذا( p->pVarName ){
    Tcl_Decr_Ref_شمېر(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_شمېر(p->pSql);
  db_Release_ستنه_نومونه(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
ساكن Tcl_Obj *db_Eval_ستنه_ارزښت(DbEvalContext *p, صحيح i_ستون){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  بدل( sqlite3_ستنه_type(pStmt, i_ستون) ){
    حالة SQLITE_BLOB: {
      صحيح بایټونه = sqlite3_ستنه_بایټونه(pStmt, i_ستون);
      ثابت حرف *zBlob = sqlite3_ستنه_blob(pStmt, i_ستون);
      إذا( !zBlob ) بایټونه = 0;
      أرجع Tcl_NewByteArrayObj((u8*)zBlob, بایټونه);
    }
    حالة SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_ستنه_int64(pStmt, i_ستون);
      إذا( v>=-2147483647 && v<=2147483647 ){
        أرجع Tcl_NewIntObj((صحيح)v);
      }وإلا{
        أرجع Tcl_NewWideIntObj(v);
      }
    }
    حالة SQLITE_FLOAT: {
      أرجع Tcl_NewDoubleObj(sqlite3_ستنه_double(pStmt, i_ستون));
    }
    حالة SQLITE_NULL: {
      أرجع Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  أرجع Tcl_NewStringObj((حرف*)sqlite3_ستنه_متن(pStmt, i_ستون), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#إذا TCL_MAJOR_نسخه>8 || !defined(TCL_MINOR_نسخه) \
                        || TCL_MINOR_نسخه>=6
# define SQLITE_TCL_NRE 1
ساكن صحيح DbUseNre(فراغ){
  صحيح major, minor;
  Tcl_Get_نسخه(&major, &minor, 0, 0);
  أرجع( (major==8 && minor>=6) || major>8 );
}
#وإلا
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (فراغ)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (فراغ)0
#د_که_پای

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
ساكن صحيح SQLITE_TCLAPI Db_Eval_بل_Cmd(
  Client_معلومات معلومات[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  صحيح پایله                           /* Result so far */
){
  صحيح rc = پایله;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)معلومات[0];
  Tcl_Obj * ثابت pScript = (Tcl_Obj *)معلومات[1];
  Tcl_Obj * ثابت pVarName = p->pVarName;

  طالما( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = db_Eval_ګام(p)) ){
    صحيح i;
    صحيح n_ستون;
    Tcl_Obj **ap_ستون_Name;
    db_Eval_لیکه_Info(p, &n_ستون, &ap_ستون_Name);
    لكل(i=0; i<n_ستون; i++){
      إذا( pVarName==0 ){
        Tcl_Obj_سېټ_Var2(interp, ap_ستون_Name[i], 0, db_Eval_ستنه_ارزښت(p,i), 0);
      }وإلا إذا( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_ستنه_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        إذا( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(ap_ستون_Name[i]), 0);
        }وإلا{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, عدم, 0);
          إذا( pDict ){
            إذا( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            إذا( Tcl_DictObjRemove(interp, pDict, ap_ستون_Name[i])==TCL_OK ){
              Tcl_Obj_سېټ_Var2(interp, pVarName, عدم, pDict, 0);
            }
            Tcl_Bounce_Ref_شمېر(pDict);
          }
        }
      }وإلا إذا( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_Obj_سېټ_Var2(interp, pVarName, ap_ستون_Name[i],
                       db_Eval_ستنه_ارزښت(p,i), 0);
      }وإلا{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, عدم, 0);
        إذا( !pDict ){
          pDict = Tcl_NewDictObj();
        }وإلا إذا( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        إذا( Tcl_DictObjPut(interp, pDict, ap_ستون_Name[i],
                           db_Eval_ستنه_ارزښت(p,i))==TCL_OK ){
          Tcl_Obj_سېټ_Var2(interp, pVarName, عدم, pDict, 0);
        }
        Tcl_Bounce_Ref_شمېر(pDict);
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
    إذا( DbUseNre() ){
      Tcl_NRAddCallback(interp, Db_Eval_بل_Cmd, (فراغ*)p, (فراغ*)pScript, 0, 0);
      أرجع Tcl_NREvalObj(interp, pScript, 0);
    }وإلا{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_شمېر(pScript);
  dbEvalFinalize(p);
  Tcl_Free((حرف *)p);

  إذا( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_Reset_پایله(interp);
    rc = TCL_OK;
  }
  أرجع rc;
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
ساكن فراغ DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  إذا( *ppHook ){
    Tcl_سېټ_Obj_پایله(interp, *ppHook);
    إذا( pArg ){
      Tcl_Decr_Ref_شمېر(*ppHook);
      *ppHook = 0;
    }
  }
  إذا( pArg ){
    أكد( !(*ppHook) );
    إذا( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_شمېر(*ppHook);
    }
  }

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
  sqlite3_preupdate_hook(db, (pDb->pPreUpdateHook?DbPreUpdateHandler:0), pDb);
#د_که_پای
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
ساكن صحيح dbQrf(SqliteDb *pDb, صحيح objc, Tcl_Obj *ثابت*objv){
#که_نه_تعریف_شوی SQLITE_QRF_H
  Tcl_سېټ_پایله(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  أرجع TCL_ERROR;
#وإلا
  حرف *z_پایله = 0;             /* Result to be returned */
  ثابت حرف *zSql = 0;          /* SQL to run */
  صحيح i;                         /* Loop counter */
  صحيح rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  ساكن ثابت حرف *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  ساكن ثابت بینښه حرف aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_کیڼ,
    QRF_ALIGN_منځ, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_ښی,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_سر,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, حجم(qrf));
  qrf.i_نسخه = 2;
  qrf.pzOutput = &z_پایله;
  لكل(i=2; i<objc; i++){
    ثابت حرف *zArg = Tcl_GetString(objv[i]);
    ثابت حرف *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    ثابت بینښه حرف aBoolMap[] = { 0, 2, 1, 2, 1 };
    إذا( zArg[0]!='-' ){
      إذا( zSql ){
        Tcl_Append_پایله(pDb->interp, "unknown argument: ", zArg, (حرف*)0);
        rc = TCL_ERROR;
        اذهب format_failed;
      }
      zSql  = zArg;
    }وإلا إذا( i==objc-1 ){
      Tcl_Append_پایله(pDb->interp, "option has no argument: ", zArg,(حرف*)0);
      rc = TCL_ERROR;
      اذهب format_failed;
    }وإلا إذا( strcmp(zArg,"-style")==0 ){
      ساكن ثابت حرف *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      ساكن بینښه حرف aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_ستنه,
        QRF_STYLE_شمېر,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_ورننباسه,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_کرښه,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_جدول
      };
      صحيح style;
      rc = Tcl_Get_شاخص_From_Obj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      إذا( rc ) اذهب format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }وإلا إذا( strcmp(zArg,"-esc")==0 ){
      ساكن ثابت حرف *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      ساكن بینښه حرف aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_نښه
      };
      صحيح esc;
      rc = Tcl_Get_شاخص_From_Obj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      إذا( rc ) اذهب format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }وإلا إذا( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      ساكن ثابت حرف *az_متن[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      ساكن بینښه حرف a_متن_Map[] = {
        QRF_متن_Auto,      QRF_متن_Csv,       QRF_متن_Html,
        QRF_متن_Json,      QRF_متن_Plain,     QRF_متن_Relaxed,
        QRF_متن_Sql,       QRF_متن_Tcl
      };
      صحيح txt;
      صحيح k = zArg[2]=='e';
      rc = Tcl_Get_شاخص_From_Obj(pDb->interp, objv[i+1], &az_متن[k*3], zArg,
                               0, &txt);
      إذا( rc ) اذهب format_failed;
      إذا( k ){
        qrf.e_متن = a_متن_Map[txt];
      }وإلا إذا( txt<=2 ){
        أكد( QRF_No==1 );
        أكد( QRF_Yes==2 );
        أكد( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_متن_Auto;
      }وإلا{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = a_متن_Map[txt-3];
      }
      i++;
    }وإلا إذا( strcmp(zArg,"-blob")==0 ){
      ساكن ثابت حرف *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      ساكن بینښه حرف aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_متن,      QRF_BLOB_Sql,
        QRF_BLOB_اندازه
      };
      صحيح blob;
      rc = Tcl_Get_شاخص_From_Obj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      إذا( rc ) اذهب format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }وإلا إذا( strcmp(zArg,"-wordwrap")==0 ){
      صحيح v = 0;
      rc = Tcl_Get_شاخص_From_Obj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      إذا( rc ) اذهب format_failed;
      qrf.b_کلمه_Wrap = aBoolMap[v];
      i++;
    }وإلا إذا( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      صحيح v = 0;
      rc = Tcl_Get_شاخص_From_Obj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      إذا( rc ) اذهب format_failed;
      إذا( zArg[1]=='t' ){
        qrf.b_متن_Jsonb = aBoolMap[v];
      }وإلا إذا( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }وإلا إذا( zArg[1]=='r' ){
        qrf.b_لیکه_شمېر = aBoolMap[v];
      }وإلا{
        qrf.b_وېشل_ستنه = aBoolMap[v];
      }
      i++;
    }وإلا إذا( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      صحيح ax = 0;
      rc = Tcl_Get_شاخص_From_Obj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      إذا( rc ) اذهب format_failed;
      إذا( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }وإلا{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }وإلا إذا( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      صحيح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      إذا( rc ) اذهب format_failed;
      إذا( v<QRF_MIN_پلنوالی ){
        v = QRF_MIN_پلنوالی;
      }وإلا إذا( v>QRF_MAX_پلنوالی ){
        v = QRF_MAX_پلنوالی;
      }
      إذا( zArg[1]=='w' ){
        qrf.nWrap = v;
      }وإلا إذا( zArg[1]=='s' ){
        qrf.n_Screen_پلنوالی = v;
      }وإلا إذا( zArg[1]=='t' ){
        qrf.n_Title_حد = v;
      }وإلا{
        qrf.n_کرښه_حد = v;
      }
      i++;
    }وإلا إذا( strcmp(zArg,"-charlimit")==0 ){
      صحيح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      إذا( rc ) اذهب format_failed;
      إذا( v<0 ) v = 0;
      qrf.n_Char_حد = v;
      i++;
    }وإلا إذا( strcmp(zArg,"-multiinsert")==0 ){
      صحيح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      إذا( rc ) اذهب format_failed;
      إذا( v<0 ) v = 0;
      qrf.n_Multi_ورننباسه = v;
      i++;
    }وإلا إذا( strcmp(zArg,"-align")==0 ){
      Tcl_اندازه n = 0;
      صحيح jj;
      rc = Tcl_List_Obj_اوږدتیا(pDb->interp, objv[i+1], &n);
      إذا( rc ) اذهب format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*حجم(qrf.aAlign[0]) );
      إذا( qrf.aAlign==0 ){
        Tcl_Append_پایله(pDb->interp, "out of memory", (حرف*)0);
        rc = TCL_ERROR;
        اذهب format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*حجم(qrf.aAlign[0]));
      qrf.nAlign = n;
      لكل(jj=0; jj<n; jj++){
        صحيح x;
        Tcl_Obj *p_غړی;
        rc = Tcl_List_Obj_شاخص(pDb->interp, objv[i+1], jj, &p_غړی);
        إذا( rc ) اذهب format_failed;
        rc = Tcl_Get_شاخص_From_Obj(pDb->interp, p_غړی, azAlign,
                          "column alignment (-align)", 0, &x);
        إذا( rc ) اذهب format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }وإلا إذا( strcmp(zArg,"-widths")==0 ){
      Tcl_اندازه n = 0;
      صحيح jj;
      rc = Tcl_List_Obj_اوږدتیا(pDb->interp, objv[i+1], &n);
      إذا( rc ) اذهب format_failed;
      sqlite3_free(qrf.a_پلنوالی);
      qrf.a_پلنوالی = sqlite3_malloc64( (n+1)*حجم(qrf.a_پلنوالی[0]) );
      إذا( qrf.a_پلنوالی==0 ){
        Tcl_Append_پایله(pDb->interp, "out of memory", (حرف*)0);
        rc = TCL_ERROR;
        اذهب format_failed;
      }
      memset(qrf.a_پلنوالی, 0, (n+1)*حجم(qrf.a_پلنوالی[0]));
      qrf.n_پلنوالی = n;
      لكل(jj=0; jj<n; jj++){
        Tcl_Obj *p_غړی;
        صحيح v;
        rc = Tcl_List_Obj_شاخص(pDb->interp, objv[i+1], jj, &p_غړی);
        إذا( rc ) اذهب format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, p_غړی, &v);
        إذا( v<(-QRF_MAX_پلنوالی) ){
          v = -QRF_MAX_پلنوالی;
        }وإلا إذا( v>QRF_MAX_پلنوالی ){
          v = QRF_MAX_پلنوالی;
        }
        qrf.a_پلنوالی[jj] = (لنډ صحيح)v;
      }
      i++;
    }وإلا إذا( strcmp(zArg,"-columnsep")==0 ){
      qrf.z_ستنه_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }وإلا إذا( strcmp(zArg,"-rowsep")==0 ){
      qrf.z_لیکه_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }وإلا إذا( strcmp(zArg,"-tablename")==0 ){
      qrf.z_جدول_Name = Tcl_GetString(objv[i+1]);
      i++;
    }وإلا إذا( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }وإلا إذا( strcmp(zArg,"-ifmt")==0 ){
      حرف *zFmt = Tcl_GetString(objv[i+1]);
      إذا( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }وإلا إذا( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_Append_پایله(pDb->interp, "bad format for -ifmt", (حرف*)0);
        rc = TCL_ERROR;
        اذهب format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }وإلا إذا( strcmp(zArg,"-fpfmt")==0 ){
      حرف *zFmt = Tcl_GetString(objv[i+1]);
      إذا( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }وإلا إذا( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_Append_پایله(pDb->interp, "bad format for -fpfmt", (حرف*)0);
        rc = TCL_ERROR;
        اذهب format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }وإلا إذا( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.i_نسخه = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }وإلا{
      Tcl_Append_پایله(pDb->interp, "unknown option: ", zArg, (حرف*)0);
      rc = TCL_ERROR;
      اذهب format_failed;
    }
  }
  طالما( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    حرف *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    إذا( rc ) اذهب format_failed;
    إذا( pStmt==0 ) استمر;
    rc = sqlite3_format_query_پایله(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    إذا( rc ){
      Tcl_سېټ_پایله(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      اذهب format_failed;
    }
  }
  Tcl_سېټ_پایله(pDb->interp, z_پایله, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.a_پلنوالی);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(z_پایله);
  أرجع rc;

#د_که_پای
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
ساكن صحيح SQLITE_TCLAPI DbObjCmd(
  فراغ *cd,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *ثابت*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحيح choice;
  صحيح rc = TCL_OK;
  ساكن ثابت حرف *DB_strs[] = {
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
  تعداد DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_بارول_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_مداخله,            DB_وروستی_ورننباسه_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_پرمختګ,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_حالت,                DB_TIMEOUT,              DB_ټول_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_نسخه,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  إذا( objc<2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "SUBCOMMAND ...");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_Get_شاخص_From_Obj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    أرجع TCL_ERROR;
  }

  بدل( (تعداد DB_enum)choice ){

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
  حالة DB_AUTHORIZER: {
#که_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
    Tcl_Append_پایله(interp, "authorization not available in this build",
                     (حرف*)0);
    أرجع TCL_ERROR;
#وإلا
    إذا( objc>3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?CALLBACK?");
      أرجع TCL_ERROR;
    }وإلا إذا( objc==2 ){
      إذا( pDb->zAuth ){
        Tcl_Append_پایله(interp, pDb->zAuth, (حرف*)0);
      }
    }وإلا{
      حرف *zAuth;
      Tcl_اندازه len;
      إذا( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      إذا( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->zAuth, zAuth, len+1);
      }وإلا{
        pDb->zAuth = 0;
      }
      إذا( pDb->zAuth ){
        تعريفنوع صحيح (*sqlite3_auth_cb)(
           فراغ*,صحيح,ثابت حرف*,ثابت حرف*,
           ثابت حرف*,ثابت حرف*);
        pDb->interp = interp;
        sqlite3_سېټ_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }وإلا{
        sqlite3_سېټ_authorizer(pDb->db, 0, 0);
      }
    }
#د_که_پای
    اكسر;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  حالة DB_BACKUP: {
    ثابت حرف *zDestFile;
    ثابت حرف *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    إذا( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }وإلا إذا( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }وإلا{
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?DATABASE? FILENAME");
      أرجع TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    إذا( rc!=SQLITE_OK ){
      Tcl_Append_پایله(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (حرف*)0);
      sqlite3_close(pDest);
      أرجع TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    إذا( pBackup==0 ){
      Tcl_Append_پایله(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (حرف*)0);
      sqlite3_close(pDest);
      أرجع TCL_ERROR;
    }
    طالما(  (rc = sqlite3_backup_ګام(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_پای(pBackup);
    إذا( rc==SQLITE_بشپړ ){
      rc = TCL_OK;
    }وإلا{
      Tcl_Append_پایله(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (حرف*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    اكسر;
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
  حالة DB_BIND_FALLBACK: {
    إذا( objc>3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?CALLBACK?");
      أرجع TCL_ERROR;
    }وإلا إذا( objc==2 ){
      إذا( pDb->zBindFallback ){
        Tcl_Append_پایله(interp, pDb->zBindFallback, (حرف*)0);
      }
    }وإلا{
      حرف *zCallback;
      Tcl_اندازه len;
      إذا( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      إذا( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->zBindFallback, zCallback, len+1);
      }وإلا{
        pDb->zBindFallback = 0;
      }
    }
    اكسر;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  حالة DB_BUSY: {
    إذا( objc>3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "CALLBACK");
      أرجع TCL_ERROR;
    }وإلا إذا( objc==2 ){
      إذا( pDb->zBusy ){
        Tcl_Append_پایله(interp, pDb->zBusy, (حرف*)0);
      }
    }وإلا{
      حرف *zBusy;
      Tcl_اندازه len;
      إذا( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      إذا( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->zBusy, zBusy, len+1);
      }وإلا{
        pDb->zBusy = 0;
      }
      إذا( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }وإلا{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    اكسر;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  حالة DB_CACHE: {
    حرف *subCmd;
    صحيح n;

    إذا( objc<=2 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "cache option ?arg?");
      أرجع TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    إذا( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      إذا( objc!=3 ){
        Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "flush");
        أرجع TCL_ERROR;
      }وإلا{
        flushStmtCache( pDb );
      }
    }وإلا إذا( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      إذا( objc!=4 ){
        Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "size n");
        أرجع TCL_ERROR;
      }وإلا{
        إذا( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_Append_پایله( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (حرف*)0);
          أرجع TCL_ERROR;
        }وإلا{
          إذا( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }وإلا إذا( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }وإلا{
      Tcl_Append_پایله( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (حرف*)0);
      أرجع TCL_ERROR;
    }
    اكسر;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  حالة DB_CHANGES: {
    Tcl_Obj *p_پایله;
    إذا( objc!=2 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "");
      أرجع TCL_ERROR;
    }
    p_پایله = Tcl_Get_Obj_پایله(interp);
    Tcl_سېټ_Wide_Int_Obj(p_پایله, sqlite3_changes64(pDb->db));
    اكسر;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  حالة DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    اكسر;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  حالة DB_COLLATE: {
    SqlCollate *pCollate;
    حرف *zName;
    حرف *zScript;
    Tcl_اندازه nScript;
    إذا( objc!=4 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "NAME SCRIPT");
      أرجع TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( حجم(*pCollate) + nScript + 1 );
    إذا( pCollate==0 ) أرجع TCL_ERROR;
    pCollate->interp = interp;
    pCollate->p_بل = pDb->pCollate;
    pCollate->zScript = (حرف*)&pCollate[1];
    pDb->pCollate = pCollate;
    حافظه_نقل(pCollate->zScript, zScript, nScript+1);
    إذا( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_سېټ_پایله(interp, (حرف *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      أرجع TCL_ERROR;
    }
    اكسر;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  حالة DB_COLLATION_NEEDED: {
    إذا( objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "SCRIPT");
      أرجع TCL_ERROR;
    }
    إذا( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_شمېر(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_شمېر(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    اكسر;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  حالة DB_COMMIT_HOOK: {
    إذا( objc>3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?CALLBACK?");
      أرجع TCL_ERROR;
    }وإلا إذا( objc==2 ){
      إذا( pDb->zCommit ){
        Tcl_Append_پایله(interp, pDb->zCommit, (حرف*)0);
      }
    }وإلا{
      ثابت حرف *zCommit;
      Tcl_اندازه len;
      إذا( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      إذا( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->zCommit, zCommit, len+1);
      }وإلا{
        pDb->zCommit = 0;
      }
      إذا( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }وإلا{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    اكسر;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  حالة DB_COMPLETE: {
#که_نه_تعریف_شوی SQLITE_OMIT_COMPLETE
    Tcl_Obj *p_پایله;
    صحيح isComplete;
    إذا( objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "SQL");
      أرجع TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    p_پایله = Tcl_Get_Obj_پایله(interp);
    Tcl_سېټ_Boolean_Obj(p_پایله, isComplete);
#د_که_پای
    اكسر;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  حالة DB_CONFIG: {
    ساكن ثابت بنية DbConfigChoices {
      ثابت حرف *zName;
      صحيح op;
    } aDbConfig[] = {
        { "defensive",          SQLITE_DBCONFIG_DEFENSIVE             },
        { "dqs_ddl",            SQLITE_DBCONFIG_DQS_DDL               },
        { "dqs_dml",            SQLITE_DBCONFIG_DQS_DML               },
        { "enable_fkey",        SQLITE_DBCONFIG_ENABLE_FKEY           },
        { "enable_qpsg",        SQLITE_DBCONFIG_ENABLE_QPSG           },
        { "enable_trigger",     SQLITE_DBCONFIG_ENABLE_TRIGGER        },
        { "enable_view",        SQLITE_DBCONFIG_ENABLE_VIEW           },
        { "fts3_tokenizer",     SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER },
        { "legacy_alter_table", SQLITE_DBCONFIG_LEGACY_ALTER_جدول    },
        { "legacy_file_format", SQLITE_DBCONFIG_LEGACY_FILE_FORMAT    },
        { "load_extension",     SQLITE_DBCONFIG_ENABLE_بارول_EXTENSION },
        { "no_ckpt_on_close",   SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE      },
        { "reset_database",     SQLITE_DBCONFIG_RESET_DATABASE        },
        { "trigger_eqp",        SQLITE_DBCONFIG_TRIGGER_EQP           },
        { "trusted_schema",     SQLITE_DBCONFIG_TRUSTED_SCHEMA        },
        { "writable_schema",    SQLITE_DBCONFIG_WRITABLE_SCHEMA       },
    };
    Tcl_Obj *p_پایله;
    صحيح ii;
    إذا( objc>4 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      أرجع TCL_ERROR;
    }
    إذا( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      p_پایله = Tcl_NewListObj(0,0);
      لكل(ii=0; ii<حجم(aDbConfig)/حجم(aDbConfig[0]); ii++){
        صحيح v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, p_پایله,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, p_پایله,
           Tcl_NewIntObj(v));
      }
    }وإلا{
      ثابت حرف *zOpt = Tcl_GetString(objv[2]);
      صحيح onoff = -1;
      صحيح v = 0;
      إذا( zOpt[0]=='-' ) zOpt++;
      لكل(ii=0; ii<حجم(aDbConfig)/حجم(aDbConfig[0]); ii++){
        إذا( strcmp(aDbConfig[ii].zName, zOpt)==0 ) اكسر;
      }
      إذا( ii>=حجم(aDbConfig)/حجم(aDbConfig[0]) ){
        Tcl_Append_پایله(interp, "unknown config option: \"", zOpt,
                                "\"", (فراغ*)0);
        أرجع TCL_ERROR;
      }
      إذا( objc==4 ){
        إذا( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          أرجع TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      p_پایله = Tcl_NewIntObj(v);
    }
    Tcl_سېټ_Obj_پایله(interp, p_پایله);
    اكسر;
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
  حالة DB_COPY: {
    حرف *z_جدول;               /* Insert data into this table */
    حرف *zFile;                /* The file from which to extract data */
    حرف *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    صحيح n_ستون;                   /* Number of columns in the table */
    صحيح nByte;                  /* Number of bytes in an SQL string */
    صحيح i, j;                   /* Loop counters */
    صحيح nSep;                   /* Number of bytes in zSep[] */
    صحيح nNull;                  /* Number of bytes in zNull[] */
    حرف *zSql;                 /* An SQL statement */
    حرف *z_کرښه;                /* A single line of input from the file */
    حرف **az_ستون;               /* zLine[] broken up into columns */
    ثابت حرف *zCommit;        /* How to commit changes */
    Tcl_Channel دننه;             /* The input file */
    صحيح lineno = 0;             /* Line number of input file */
    حرف z_کرښه_عدد[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *p_پایله;           /* interp result */

    ثابت حرف *zSep;
    ثابت حرف *zNull;
    إذا( objc<5 || objc>7 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      أرجع TCL_ERROR;
    }
    إذا( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }وإلا{
      zSep = "\t";
    }
    إذا( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }وإلا{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    z_جدول = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    إذا( nSep==0 ){
      Tcl_Append_پایله(interp,"Error: non-null separator required for copy",
                       (حرف*)0);
      أرجع TCL_ERROR;
    }
    إذا(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_Append_پایله(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (حرف*)0);
      أرجع TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", z_جدول);
    إذا( zSql==0 ){
      Tcl_Append_پایله(interp, "Error: no such table: ", z_جدول, (حرف*)0);
      أرجع TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    إذا( rc ){
      Tcl_Append_پایله(interp, "Error: ", sqlite3_errmsg(pDb->db), (حرف*)0);
      n_ستون = 0;
    }وإلا{
      n_ستون = sqlite3_ستنه_شمېر(pStmt);
    }
    sqlite3_finalize(pStmt);
    إذا( n_ستون==0 ) {
      أرجع TCL_ERROR;
    }
    zSql = حافظه_ورکړه( nByte + 50 + n_ستون*2 );
    إذا( zSql==0 ) {
      Tcl_Append_پایله(interp, "Error: can't malloc()", (حرف*)0);
      أرجع TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, z_جدول);
    j = strlen30(zSql);
    لكل(i=1; i<n_ستون; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    حرر(zSql);
    إذا( rc ){
      Tcl_Append_پایله(interp, "Error: ", sqlite3_errmsg(pDb->db), (حرف*)0);
      sqlite3_finalize(pStmt);
      أرجع TCL_ERROR;
    }
    دننه = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    إذا( دننه==0 ){
      sqlite3_finalize(pStmt);
      أرجع TCL_ERROR;
    }
    Tcl_سېټ_Channel_Option(عدم, دننه, "-translation", "auto");
    az_ستون = حافظه_ورکړه( حجم(az_ستون[0])*(n_ستون+1) );
    إذا( az_ستون==0 ) {
      Tcl_Append_پایله(interp, "Error: can't malloc()", (حرف*)0);
      Tcl_Close(interp, دننه);
      أرجع TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_شمېر(str);
    (فراغ)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    طالما( Tcl_GetsObj(دننه, str)>=0 ) {
      حرف *z;
      Tcl_اندازه byteLen;
      lineno++;
      z_کرښه = (حرف *)Tcl_GetByteArrayFromObj(str, &byteLen);
      az_ستون[0] = z_کرښه;
      لكل(i=0, z=z_کرښه; *z; z++){
        إذا( *z==zSep[0] && مزي_پرتله(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          إذا( i<n_ستون ){
            az_ستون[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      إذا( i+1!=n_ستون ){
        حرف *zErr;
        صحيح nErr = strlen30(zFile) + 200;
        zErr = حافظه_ورکړه(nErr);
        إذا( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, n_ستون, i+1);
          Tcl_Append_پایله(interp, zErr, (حرف*)0);
          حرر(zErr);
        }
        zCommit = "ROLLBACK";
        اكسر;
      }
      لكل(i=0; i<n_ستون; i++){
        /* check for null data, if so, bind as null */
        إذا( (nNull>0 && strcmp(az_ستون[i], zNull)==0)
          || strlen30(az_ستون[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }وإلا{
          sqlite3_bind_متن(pStmt, i+1, az_ستون[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_ګام(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_سېټ_Obj_اوږدتیا(str, 0);
      إذا( rc!=SQLITE_OK ){
        Tcl_Append_پایله(interp,"Error: ", sqlite3_errmsg(pDb->db), (حرف*)0);
        zCommit = "ROLLBACK";
        اكسر;
      }
    }
    Tcl_Decr_Ref_شمېر(str);
    حرر(az_ستون);
    Tcl_Close(interp, دننه);
    sqlite3_finalize(pStmt);
    (فراغ)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    إذا( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      p_پایله = Tcl_Get_Obj_پایله(interp);
      إذا( Tcl_IsShared(p_پایله) ){
        p_پایله = Tcl_DuplicateObj(p_پایله);
      }
      Tcl_سېټ_Int_Obj(p_پایله, lineno);
      rc = TCL_OK;
    }وإلا{
      /* failure, append lineno where failed */
      sqlite3_snprintf(حجم(z_کرښه_عدد), z_کرښه_عدد,"%d",lineno);
      Tcl_Append_پایله(interp,", failed while processing line: ",z_کرښه_عدد,
                       (حرف*)0);
      rc = TCL_ERROR;
    }
    اكسر;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  حالة DB_DESERIALIZE: {
#که_تعریف_شوی SQLITE_OMIT_DESERIALIZE
    Tcl_Append_پایله(interp, "MEMDB not available in this build",
                     (حرف*)0);
    rc = TCL_ERROR;
#وإلا
    ثابت حرف *zSchema = 0;
    Tcl_Obj *p_ارزښت = 0;
    بینښه حرف *pBA;
    بینښه حرف *p_معلومات;
    Tcl_اندازه len;
    صحيح xrc;
    sqlite3_int64 mx_اندازه = 0;
    صحيح i;
    صحيح isReadonly = 0;


    إذا( objc<3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      اكسر;
    }
    لكل(i=2; i<objc-1; i++){
      ثابت حرف *z = Tcl_GetString(objv[i]);
      إذا( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        إذا( rc ) اذهب deserialize_error;
        mx_اندازه = x;
        استمر;
      }
      إذا( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        إذا( rc ) اذهب deserialize_error;
        استمر;
      }
      إذا( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        استمر;
      }
      Tcl_Append_پایله(interp, "unknown option: ", z, (حرف*)0);
      rc = TCL_ERROR;
      اذهب deserialize_error;
    }
    p_ارزښت = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_ارزښت, &len);
    p_معلومات = sqlite3_malloc64( len );
    إذا( p_معلومات==0 && len>0 ){
      Tcl_Append_پایله(interp, "out of memory", (حرف*)0);
      rc = TCL_ERROR;
    }وإلا{
      صحيح flags;
      إذا( len>0 ) حافظه_نقل(p_معلومات, pBA, len);
      إذا( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }وإلا{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, p_معلومات, len, len, flags);
      إذا( xrc ){
        Tcl_Append_پایله(interp, "unable to set MEMDB content", (حرف*)0);
        rc = TCL_ERROR;
      }
      إذا( mx_اندازه>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_اندازه_حد,&mx_اندازه);
      }
    }
deserialize_error:
#د_که_پای
    اكسر; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  حالة DB_ENABLE_بارول_EXTENSION: {
#که_نه_تعریف_شوی SQLITE_OMIT_بارول_EXTENSION
    صحيح onoff;
    إذا( objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "BOOLEAN");
      أرجع TCL_ERROR;
    }
    إذا( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      أرجع TCL_ERROR;
    }
    sqlite3_enable_بارول_extension(pDb->db, onoff);
    اكسر;
#وإلا
    Tcl_Append_پایله(interp, "extension loading is turned off at compile-time",
                     (حرف*)0);
    أرجع TCL_ERROR;
#د_که_پای
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  حالة DB_ERRORCODE: {
    Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    اكسر;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  حالة DB_ERROROFFSET: {
    Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(sqlite3_error_دننه_واټن(pDb->db)));
    اكسر;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  حالة DB_EXISTS:
  حالة DB_ONECOLUMN: {
    Tcl_Obj *p_پایله = 0;
    DbEvalContext sEval;
    إذا( objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "SQL");
      أرجع TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = db_Eval_ګام(&sEval);
    إذا( choice==DB_ONECOLUMN ){
      إذا( rc==TCL_OK ){
        p_پایله = db_Eval_ستنه_ارزښت(&sEval, 0);
      }وإلا إذا( rc==TCL_BREAK ){
        Tcl_Reset_پایله(interp);
      }
    }وإلا إذا( rc==TCL_BREAK || rc==TCL_OK ){
      p_پایله = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    إذا( p_پایله ) Tcl_سېټ_Obj_پایله(interp, p_پایله);

    إذا( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    اكسر;
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
  حالة DB_EVAL: {
    صحيح evalFlags = 0;
    ثابت حرف *zOpt;
    طالما( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      إذا( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }وإلا إذا( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }وإلا{
        Tcl_Append_پایله(interp, "unknown option: \"", zOpt, "\"", (فراغ*)0);
        أرجع TCL_ERROR;
      }
      objc--;
      objv++;
    }
    إذا( objc<3 || objc>5 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      أرجع TCL_ERROR;
    }

    إذا( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_شمېر(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      طالما( TCL_OK==(rc = db_Eval_ګام(&sEval)) ){
        صحيح i;
        صحيح n_ستون;
        db_Eval_لیکه_Info(&sEval, &n_ستون, 0);
        لكل(i=0; i<n_ستون; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_ستنه_ارزښت(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      إذا( rc==TCL_BREAK ){
        Tcl_سېټ_Obj_پایله(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_شمېر(pRet);
    }وإلا{
      Client_معلومات cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      إذا( objc>=5 && *(حرف *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_شمېر(pScript);

      p = (DbEvalContext *)Tcl_Alloc(حجم(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (فراغ *)p;
      cd2[1] = (فراغ *)pScript;
      rc = Db_Eval_بل_Cmd(cd2, interp, TCL_OK);
    }
    اكسر;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  حالة DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    اكسر;
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
  حالة DB_FUNCTION: {
    صحيح flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    حرف *zName;
    صحيح nArg = -1;
    صحيح i;
    صحيح eType = SQLITE_NULL;
    إذا( objc<4 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      أرجع TCL_ERROR;
    }
    لكل(i=3; i<(objc-1); i++){
      ثابت حرف *z = Tcl_GetString(objv[i]);
      صحيح n = strlen30(z);
      إذا( n>1 && مزي_پرتله(z, "-argcount",n)==0 ){
        إذا( i==(objc-2) ){
          Tcl_Append_پایله(interp, "option requires an argument: ", z,(حرف*)0);
          أرجع TCL_ERROR;
        }
        إذا( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) أرجع TCL_ERROR;
        إذا( nArg<0 ){
          Tcl_Append_پایله(interp, "number of arguments must be non-negative",
                           (حرف*)0);
          أرجع TCL_ERROR;
        }
        i++;
      }وإلا
      إذا( n>1 && مزي_پرتله(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }وإلا
      إذا( n>1 && مزي_پرتله(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }وإلا
      إذا( n>1 && مزي_پرتله(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }وإلا
      إذا( n>1 && مزي_پرتله(z, "-returntype", n)==0 ){
        ثابت حرف *azType[] = {"integer", "real", "text", "blob", "any", 0};
        أكد( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_متن==3 );
        أكد( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        إذا( i==(objc-2) ){
          Tcl_Append_پایله(interp, "option requires an argument: ", z,(حرف*)0);
          أرجع TCL_ERROR;
        }
        i++;
        إذا( Tcl_Get_شاخص_From_Obj(interp, objv[i], azType, "type", 0, &eType) ){
          أرجع TCL_ERROR;
        }
        eType++;
      }وإلا{
        Tcl_Append_پایله(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (حرف*)0
        );
        أرجع TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = پیدا_کړه_Sql_Func(pDb, zName);
    إذا( pFunc==0 ) أرجع TCL_ERROR;
    إذا( pFunc->pScript ){
      Tcl_Decr_Ref_شمېر(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_شمېر(pScript);
    pFunc->useEvalObjv = خوندي_منزل_Use_Eval_Objv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    إذا( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_سېټ_پایله(interp, (حرف *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    اكسر;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  حالة DB_INCRBLOB: {
#که_تعریف_شوی SQLITE_OMIT_INCRBLOB
    Tcl_Append_پایله(interp, "incrblob not available in this build", (حرف*)0);
    أرجع TCL_ERROR;
#وإلا
    صحيح isReadonly = 0;
    ثابت حرف *zDb = "main";
    ثابت حرف *z_جدول;
    ثابت حرف *z_ستنه;
    Tcl_WideInt i_لیکه;

    /* Check for the -readonly option */
    إذا( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    إذا( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      أرجع TCL_ERROR;
    }

    إذا( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    z_جدول = Tcl_GetString(objv[objc-3]);
    z_ستنه = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &i_لیکه);

    إذا( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, z_جدول, z_ستنه, (sqlite3_int64)i_لیکه, isReadonly
      );
    }
#د_که_پای
    اكسر;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  حالة DB_مداخله: {
    sqlite3_مداخله(pDb->db);
    اكسر;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  حالة DB_NULLVALUE: {
    إذا( objc!=2 && objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "NULLVALUE");
      أرجع TCL_ERROR;
    }
    إذا( objc==3 ){
      Tcl_اندازه len;
      حرف *zNull = Tcl_GetStringFromObj(objv[2], &len);
      إذا( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      إذا( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }وإلا{
        pDb->zNull = 0;
      }
    }
    Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj(pDb->zNull, -1));
    اكسر;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  حالة DB_وروستی_ورننباسه_ROWID: {
    Tcl_Obj *p_پایله;
    Tcl_WideInt rowid;
    إذا( objc!=2 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "");
      أرجع TCL_ERROR;
    }
    rowid = sqlite3_وروستی_ورننباسه_rowid(pDb->db);
    p_پایله = Tcl_Get_Obj_پایله(interp);
    Tcl_سېټ_Wide_Int_Obj(p_پایله, rowid);
    اكسر;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  حالة DB_پرمختګ: {
    إذا( objc==2 ){
      إذا( pDb->z_پرمختګ ){
        Tcl_Append_پایله(interp, pDb->z_پرمختګ, (حرف*)0);
      }
#که_نه_تعریف_شوی SQLITE_OMIT_پرمختګ_CALLBACK
      sqlite3_پرمختګ_handler(pDb->db, 0, 0, 0);
#د_که_پای
    }وإلا إذا( objc==4 ){
      حرف *z_پرمختګ;
      Tcl_اندازه len;
      صحيح N;
      إذا( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        أرجع TCL_ERROR;
      };
      إذا( pDb->z_پرمختګ ){
        Tcl_Free(pDb->z_پرمختګ);
      }
      z_پرمختګ = Tcl_GetStringFromObj(objv[3], &len);
      إذا( z_پرمختګ && len>0 ){
        pDb->z_پرمختګ = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->z_پرمختګ, z_پرمختګ, len+1);
      }وإلا{
        pDb->z_پرمختګ = 0;
      }
#که_نه_تعریف_شوی SQLITE_OMIT_پرمختګ_CALLBACK
      إذا( pDb->z_پرمختګ ){
        pDb->interp = interp;
        sqlite3_پرمختګ_handler(pDb->db, N, Db_پرمختګ_Handler, pDb);
      }وإلا{
        sqlite3_پرمختګ_handler(pDb->db, 0, 0, 0);
      }
#د_که_پای
    }وإلا{
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "N CALLBACK");
      أرجع TCL_ERROR;
    }
    اكسر;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  حالة DB_PROFILE: {
    إذا( objc>3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?CALLBACK?");
      أرجع TCL_ERROR;
    }وإلا إذا( objc==2 ){
      إذا( pDb->zProfile ){
        Tcl_Append_پایله(interp, pDb->zProfile, (حرف*)0);
      }
    }وإلا{
      حرف *zProfile;
      Tcl_اندازه len;
      إذا( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      إذا( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->zProfile, zProfile, len+1);
      }وإلا{
        pDb->zProfile = 0;
      }
#إذا !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ټکی) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      إذا( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }وإلا{
        sqlite3_profile(pDb->db, 0, 0);
      }
#د_که_پای
    }
    اكسر;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  حالة DB_REKEY: {
    إذا( objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "KEY");
      أرجع TCL_ERROR;
    }
    اكسر;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  حالة DB_RESTORE: {
    ثابت حرف *zSrcFile;
    ثابت حرف *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    صحيح nTimeout = 0;

    إذا( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }وإلا إذا( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }وإلا{
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?DATABASE? FILENAME");
      أرجع TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    إذا( rc!=SQLITE_OK ){
      Tcl_Append_پایله(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (حرف*)0);
      sqlite3_close(pSrc);
      أرجع TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    إذا( pBackup==0 ){
      Tcl_Append_پایله(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (حرف*)0);
      sqlite3_close(pSrc);
      أرجع TCL_ERROR;
    }
    طالما( (rc = sqlite3_backup_ګام(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      إذا( rc==SQLITE_BUSY ){
        إذا( nTimeout++ >= 3 ) اكسر;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_پای(pBackup);
    إذا( rc==SQLITE_بشپړ ){
      rc = TCL_OK;
    }وإلا إذا( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_Append_پایله(interp, "restore failed: source database busy",
                       (حرف*)0);
      rc = TCL_ERROR;
    }وإلا{
      Tcl_Append_پایله(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (حرف*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    اكسر;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  حالة DB_SERIALIZE: {
#که_تعریف_شوی SQLITE_OMIT_DESERIALIZE
    Tcl_Append_پایله(interp, "MEMDB not available in this build",
                     (حرف*)0);
    rc = TCL_ERROR;
#وإلا
    ثابت حرف *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    بینښه حرف *p_معلومات;
    إذا( objc!=2 && objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }وإلا{
      صحيح needFree;
      p_معلومات = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      إذا( p_معلومات ){
        needFree = 0;
      }وإلا{
        p_معلومات = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_سېټ_Obj_پایله(interp, Tcl_NewByteArrayObj(p_معلومات,sz));
      إذا( needFree ) sqlite3_free(p_معلومات);
    }
#د_که_پای
    اكسر;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  حالة DB_حالت: {
    صحيح v;
    ثابت حرف *zOp;
    إذا( objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "(step|sort|autoindex)");
      أرجع TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    إذا( strcmp(zOp, "step")==0 ){
      v = pDb->n_ګام;
    }وإلا إذا( strcmp(zOp, "sort")==0 ){
      v = pDb->n_ترتیبول;
    }وإلا إذا( strcmp(zOp, "autoindex")==0 ){
      v = pDb->n_شاخص;
    }وإلا إذا( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }وإلا{
      Tcl_Append_پایله(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (حرف*)0);
      أرجع TCL_ERROR;
    }
    Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(v));
    اكسر;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  حالة DB_TIMEOUT: {
    صحيح ms;
    إذا( objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "MILLISECONDS");
      أرجع TCL_ERROR;
    }
    إذا( Tcl_GetIntFromObj(interp, objv[2], &ms) ) أرجع TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    اكسر;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  حالة DB_ټول_CHANGES: {
    Tcl_Obj *p_پایله;
    إذا( objc!=2 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "");
      أرجع TCL_ERROR;
    }
    p_پایله = Tcl_Get_Obj_پایله(interp);
    Tcl_سېټ_Wide_Int_Obj(p_پایله, sqlite3_ټول_changes64(pDb->db));
    اكسر;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  حالة DB_TRACE: {
    إذا( objc>3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?CALLBACK?");
      أرجع TCL_ERROR;
    }وإلا إذا( objc==2 ){
      إذا( pDb->zTrace ){
        Tcl_Append_پایله(interp, pDb->zTrace, (حرف*)0);
      }
    }وإلا{
      حرف *zTrace;
      Tcl_اندازه len;
      إذا( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      إذا( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->zTrace, zTrace, len+1);
      }وإلا{
        pDb->zTrace = 0;
      }
#إذا !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ټکی) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      إذا( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }وإلا{
        sqlite3_trace(pDb->db, 0, 0);
      }
#د_که_پای
    }
    اكسر;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  حالة DB_TRACE_V2: {
    إذا( objc>4 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?CALLBACK? ?MASK?");
      أرجع TCL_ERROR;
    }وإلا إذا( objc==2 ){
      إذا( pDb->zTraceV2 ){
        Tcl_Append_پایله(interp, pDb->zTraceV2, (حرف*)0);
      }
    }وإلا{
      حرف *zTraceV2;
      Tcl_اندازه len;
      Tcl_WideInt w_ماسک = 0;
      إذا( objc==4 ){
        ساكن ثابت حرف *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        تعداد TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_لیکه, TTYPE_CLOSE
        };
        Tcl_اندازه i;
        إذا( TCL_OK!=Tcl_List_Obj_اوږدتیا(interp, objv[3], &len) ){
          أرجع TCL_ERROR;
        }
        لكل(i=0; i<len; i++){
          Tcl_Obj *pObj;
          صحيح ttype;
          إذا( TCL_OK!=Tcl_List_Obj_شاخص(interp, objv[3], i, &pObj) ){
            أرجع TCL_ERROR;
          }
          إذا( Tcl_Get_شاخص_From_Obj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_Get_Obj_پایله(interp));
            Tcl_Incr_Ref_شمېر(pError);
            إذا( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_شمېر(pError);
              w_ماسک |= wType;
            }وإلا{
              Tcl_سېټ_Obj_پایله(interp, pError);
              Tcl_Decr_Ref_شمېر(pError);
              أرجع TCL_ERROR;
            }
          }وإلا{
            بدل( (تعداد TTYPE_enum)ttype ){
              حالة TTYPE_STMT:    w_ماسک |= SQLITE_TRACE_STMT;    اكسر;
              حالة TTYPE_PROFILE: w_ماسک |= SQLITE_TRACE_PROFILE; اكسر;
              حالة TTYPE_لیکه:     w_ماسک |= SQLITE_TRACE_لیکه;     اكسر;
              حالة TTYPE_CLOSE:   w_ماسک |= SQLITE_TRACE_CLOSE;   اكسر;
            }
          }
        }
      }وإلا{
        w_ماسک = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      إذا( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      إذا( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        حافظه_نقل(pDb->zTraceV2, zTraceV2, len+1);
      }وإلا{
        pDb->zTraceV2 = 0;
      }
#إذا !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ټکی)
      إذا( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (بینښه)w_ماسک, DbTraceV2Handler, pDb);
      }وإلا{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#د_که_پای
    }
    اكسر;
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
  حالة DB_TRANSACTION: {
    Tcl_Obj *pScript;
    ثابت حرف *zBegin = "SAVEPOINT _tcl_transaction";
    إذا( objc!=3 && objc!=4 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "[TYPE] SCRIPT");
      أرجع TCL_ERROR;
    }

    إذا( pDb->nTransaction==0 && objc==4 ){
      ساكن ثابت حرف *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      تعداد TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_سمدستي
      };
      صحيح ttype;
      إذا( Tcl_Get_شاخص_From_Obj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        أرجع TCL_ERROR;
      }
      بدل( (تعداد TTYPE_enum)ttype ){
        حالة TTYPE_DEFERRED:    /* no-op */;                 اكسر;
        حالة TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  اكسر;
        حالة TTYPE_سمدستي:   zBegin = "BEGIN IMMEDIATE";  اكسر;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    إذا( rc!=SQLITE_OK ){
      Tcl_Append_پایله(interp, sqlite3_errmsg(pDb->db), (حرف*)0);
      أرجع TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    جمع_کړه_Database_Ref(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    إذا( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (فراغ)Tcl_NREvalObj(interp, pScript, 0);
    }وإلا{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    اكسر;
  }

  /*
  **    $db unlock_notify ?script?
  */
  حالة DB_UNLOCK_NOTIFY: {
#که_نه_تعریف_شوی SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_Append_پایله(interp, "unlock_notify not available in this build",
                     (حرف*)0);
    rc = TCL_ERROR;
#وإلا
    إذا( objc!=2 && objc!=3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }وإلا{
      فراغ (*xNotify)(فراغ **, صحيح) = 0;
      فراغ *pNotifyArg = 0;

      إذا( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_شمېر(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      إذا( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (فراغ *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_شمېر(pDb->pUnlockNotify);
      }

      إذا( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_Append_پایله(interp, sqlite3_errmsg(pDb->db), (حرف*)0);
        rc = TCL_ERROR;
      }
    }
#د_که_پای
    اكسر;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  حالة DB_PREUPDATE: {
#که_نه_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_Append_پایله(interp, "preupdate_hook was omitted at compile-time", 
                     (حرف*)0);
    rc = TCL_ERROR;
#وإلا
    ساكن ثابت حرف *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    تعداد DbPreupdateSubCmd {
      PRE_شمېر, PRE_ژوروالی, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    صحيح iSub;

    إذا( objc<3 ){
      Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    إذا( Tcl_Get_شاخص_From_Obj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      أرجع TCL_ERROR;
    }

    بدل( (تعداد DbPreupdateSubCmd)iSub ){
      حالة PRE_شمېر: {
        صحيح n_ستون = sqlite3_preupdate_شمېر(pDb->db);
        Tcl_سېټ_Obj_پایله(interp, Tcl_NewIntObj(n_ستون));
        اكسر;
      }

      حالة PRE_HOOK: {
        إذا( objc>4 ){
          Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "hook ?SCRIPT?");
          أرجع TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        اكسر;
      }

      حالة PRE_ژوروالی: {
        Tcl_Obj *pRet;
        إذا( objc!=3 ){
          Tcl_Wrong_عدد_دلیلونه(interp, 3, objv, "");
          أرجع TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_ژوروالی(pDb->db));
        Tcl_سېټ_Obj_پایله(interp, pRet);
        اكسر;
      }

      حالة PRE_NEW:
      حالة PRE_OLD: {
        صحيح iIdx;
        sqlite3_ارزښت *p_ارزښت;
        إذا( objc!=4 ){
          Tcl_Wrong_عدد_دلیلونه(interp, 3, objv, "INDEX");
          أرجع TCL_ERROR;
        }
        إذا( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          أرجع TCL_ERROR;
        }

        إذا( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_ارزښت);
        }وإلا{
          أكد( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_ارزښت);
        }

        إذا( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((حرف*)sqlite3_ارزښت_متن(p_ارزښت), -1);
          Tcl_سېټ_Obj_پایله(interp, pObj);
        }وإلا{
          Tcl_Append_پایله(interp, sqlite3_errmsg(pDb->db), (حرف*)0);
          أرجع TCL_ERROR;
        }
      }
    }
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */
    اكسر;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  حالة DB_WAL_HOOK:
  حالة DB_UPDATE_HOOK:
  حالة DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    إذا( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    إذا( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    إذا( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    إذا( objc>3 ){
       Tcl_Wrong_عدد_دلیلونه(interp, 2, objv, "?SCRIPT?");
       أرجع TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    اكسر;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  حالة DB_نسخه: {
    صحيح i;
    لكل(i=2; i<objc; i++){
      ثابت حرف *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#که_تعریف_شوی SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      إذا( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        إذا( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          أرجع TCL_ERROR;
        }
      }وإلا

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      إذا( strcmp(zArg, "-last-stmt-ptr")==0 ){
        خارجي ثابت حرف *sqlite3_Test_Ptr_منزل_متن(فراغ*);
        فراغ *ptr = (فراغ*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_سېټ_پایله(interp, (حرف*)sqlite3_Test_Ptr_منزل_متن(ptr), TCL_VOLATILE);
      }وإلا
#د_که_پای /* SQLITE_TEST */
      {
        Tcl_Append_پایله(interp, "unknown argument: ", zArg, (حرف*)0);
        أرجع TCL_ERROR;
      }
    }
    إذا( i==2 ){   
      Tcl_سېټ_پایله(interp, (حرف *)sqlite3_libversion(), TCL_STATIC);
    }
    اكسر;
  }


  } /* End of the SWITCH statement */
  أرجع rc;
}

#إذا SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
ساكن صحيح SQLITE_TCLAPI DbObjCmdAdaptor(
  فراغ *cd,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *ثابت*objv
){
  أرجع Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#د_که_پای /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
ساكن صحيح sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *ثابت*objv
){
  Tcl_Wrong_عدد_دلیلونه(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  أرجع TCL_ERROR;
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
ساكن صحيح SQLITE_TCLAPI DbMain(
  فراغ *cd,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *ثابت*objv
){
  SqliteDb *p;
  ثابت حرف *zArg;
  حرف *z_Err_پیغام;
  صحيح i;
  ثابت حرف *zFile = 0;
  ثابت حرف *zVfs = 0;
  صحيح flags;
  صحيح bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  صحيح rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#که_تعریف_شوی SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#وإلا
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#د_که_پای

  إذا( objc==1 ) أرجع sqliteCmdUsage(interp, objv);
  إذا( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    إذا( strcmp(zArg,"-version")==0 ){
      Tcl_Append_پایله(interp,sqlite3_libversion(), (حرف*)0);
      أرجع TCL_OK;
    }
    إذا( strcmp(zArg,"-sourceid")==0 ){
      Tcl_Append_پایله(interp,sqlite3_sourceid(), (حرف*)0);
      أرجع TCL_OK;
    }
    إذا( strcmp(zArg,"-has-codec")==0 ){
      Tcl_Append_پایله(interp,"0",(حرف*)0);
      أرجع TCL_OK;
    }
    إذا( zArg[0]=='-' ) أرجع sqliteCmdUsage(interp, objv);
  }
  لكل(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    إذا( zArg[0]!='-' ){
      إذا( zFile!=0 ) أرجع sqliteCmdUsage(interp, objv);
      zFile = zArg;
      استمر;
    }
    إذا( i==objc-1 ) أرجع sqliteCmdUsage(interp, objv);
    i++;
    إذا( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }وإلا إذا( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }وإلا إذا( strcmp(zArg, "-readonly")==0 ){
      صحيح b;
      إذا( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) أرجع TCL_ERROR;
      إذا( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }وإلا{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }وإلا إذا( strcmp(zArg, "-create")==0 ){
      صحيح b;
      إذا( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) أرجع TCL_ERROR;
      إذا( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }وإلا{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }وإلا إذا( strcmp(zArg, "-nofollow")==0 ){
      صحيح b;
      إذا( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) أرجع TCL_ERROR;
      إذا( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }وإلا{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }وإلا إذا( strcmp(zArg, "-nomutex")==0 ){
      صحيح b;
      إذا( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) أرجع TCL_ERROR;
      إذا( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }وإلا{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }وإلا إذا( strcmp(zArg, "-fullmutex")==0 ){
      صحيح b;
      إذا( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) أرجع TCL_ERROR;
      إذا( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }وإلا{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }وإلا إذا( strcmp(zArg, "-uri")==0 ){
      صحيح b;
      إذا( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) أرجع TCL_ERROR;
      إذا( b ){
        flags |= SQLITE_OPEN_URI;
      }وإلا{
        flags &= ~SQLITE_OPEN_URI;
      }
    }وإلا إذا( strcmp(zArg, "-translatefilename")==0 ){
      إذا( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        أرجع TCL_ERROR;
      }
    }وإلا{
      Tcl_Append_پایله(interp, "unknown option: ", zArg, (حرف*)0);
      أرجع TCL_ERROR;
    }
  }
  z_Err_پیغام = 0;
  p = (SqliteDb*)Tcl_Alloc( حجم(*p) );
  memset(p, 0, حجم(*p));
  إذا( zFile==0 ) zFile = "";
  إذا( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  إذا( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  إذا( p->db ){
    إذا( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_پیغام = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }وإلا{
    z_Err_پیغام = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  إذا( p->db==0 ){
    Tcl_سېټ_پایله(interp, z_Err_پیغام, TCL_VOLATILE);
    Tcl_Free((حرف*)p);
    sqlite3_free(z_Err_پیغام);
    أرجع TCL_ERROR;
  }
  p->maxStmt = عدد_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  إذا( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (حرف*)p, DbDeleteCmd);
  }وإلا{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (حرف*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  أرجع TCL_OK;
}

/*
** Provide a dummy Tcl_InitStubs if we are using this as a static
** library.
*/
#که_نه_تعریف_شوی USE_TCL_STUBS
# undef  Tcl_InitStubs
# define Tcl_InitStubs(a,b,c) TCL_نسخه
#د_که_پای

/*
** Make sure we have a PACKAGE_VERSION macro defined.  This will be
** defined automatically by the TEA makefile.  But other makefiles
** do not define it.
*/
#که_نه_تعریف_شوی PACKAGE_نسخه
# define PACKAGE_نسخه SQLITE_نسخه
#د_که_پای

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
EXTERN صحيح Sqlite3_Init(Tcl_Interp *interp){
  صحيح rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  إذا( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#که_نه_تعریف_شوی SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#د_که_پای
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_نسخه);
  }
  أرجع rc;
}
EXTERN صحيح Tclsqlite3_Init(Tcl_Interp *interp){ أرجع Sqlite3_Init(interp); }
EXTERN صحيح Sqlite3_Unload(Tcl_Interp *interp, صحيح flags){ أرجع TCL_OK; }
EXTERN صحيح Tclsqlite3_Unload(Tcl_Interp *interp, صحيح flags){ أرجع TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN صحيح Sqlite3_خوندي_Init(Tcl_Interp *interp){ أرجع TCL_ERROR; }
EXTERN صحيح Sqlite3_خوندي_Unload(Tcl_Interp *interp, صحيح flags){أرجع TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN صحيح Sqlite_Init(Tcl_Interp *interp){ أرجع Sqlite3_Init(interp);}
EXTERN صحيح Tclsqlite_Init(Tcl_Interp *interp){ أرجع Sqlite3_Init(interp); }
EXTERN صحيح Sqlite_Unload(Tcl_Interp *interp, صحيح flags){ أرجع TCL_OK; }
EXTERN صحيح Tclsqlite_Unload(Tcl_Interp *interp, صحيح flags){ أرجع TCL_OK; }
EXTERN صحيح Sqlite_خوندي_Init(Tcl_Interp *interp){ أرجع TCL_ERROR; }
EXTERN صحيح Sqlite_خوندي_Unload(Tcl_Interp *interp, صحيح flags){أرجع TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN صحيح sqlite3_Init(Tcl_Interp *interp){ أرجع Sqlite3_Init(interp);}
EXTERN صحيح sqlite_Init(Tcl_Interp *interp){ أرجع Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#إذا defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
ساكن ثابت حرف *tclsh_main_loop(فراغ){
  ساكن ثابت حرف zMainloop[] =
    "if {[llength $argv]>=1} {\n"
#که_تعریف_شوی WIN32
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
#د_که_پای
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
  أرجع zMainloop;
}

#که_نه_تعریف_شوی TCLSH_MAIN
# define TCLSH_MAIN اصلي
#د_که_پای
صحيح SQLITE_CDECL TCLSH_MAIN(صحيح argc, حرف **argv){
  Tcl_Interp *interp;
  صحيح i;
  ثابت حرف *zScript = 0;
  حرف zArgc[32];
#إذا defined(TCLSH_INIT_PROC)
  خارجي ثابت حرف *TCLSH_INIT_PROC(Tcl_Interp*);
#د_که_پای

#إذا !defined(_WIN32_WCE)
  إذا( getenv("SQLITE_DEBUG_BREAK") ){
    إذا( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }وإلا{
#إذا defined(_WIN32) || defined(WIN32)
      DebugBreak();
#وإلاإذا defined(SIGTRAP)
      ارفع(SIGTRAP);
#د_که_پای
    }
  }
#د_که_پای

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_پیدا_کړه_Executable(argv[0]);
  Tcl_سېټ_System_Encoding(عدم, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(حجم(zArgc), zArgc, "%d", argc-1);
  Tcl_سېټ_Var(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_سېټ_Var(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_سېټ_Var(interp,"argv", "", TCL_GLOBAL_ONLY);
  لكل(i=1; i<argc; i++){
    Tcl_سېټ_Var(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_ارزښت);
  }
#إذا defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#د_که_پای
  إذا( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  إذا( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    ثابت حرف *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    إذا( zInfo==0 ) zInfo = Tcl_Get_String_پایله(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    أرجع 1;
  }
  أرجع 0;
}
#د_که_پای /* TCLSH */
