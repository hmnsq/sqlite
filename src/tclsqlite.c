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
#اگر_تعریف_شدہ TCLSH_INIT_PROC
# define TCLSH 1
#اگر_ختم

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#اگر defined(INCLUDE_MSVC_H)
# include "msvc.h"
#اگر_ختم

/****** Copy of tclsqlite.h ******/
#اگر defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#ورنہ
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#اگر_ختم
/* Compatability between Tcl8.6 and Tcl9.0 */
#اگر TCL_MAJOR_نسخہ==9
# define CONST ثابت
#وگرنہ_اگر !defined(Tcl_سائز)
  قسمتعریف صحیح Tcl_سائز;
# ifndef Tcl_Bounce_Ref_گنتی
#  define Tcl_Bounce_Ref_گنتی(X) Tcl_Incr_Ref_گنتی(X); Tcl_Decr_Ref_گنتی(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#اگر_ختم
/**** End copy of tclsqlite.h ****/

#شامل <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#اگر_غیر_تعریف_شدہ SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  قسمتعریف غیرعلامتی حرف u8;
# ifndef SQLITE_PTRSIZE
#   اگر defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   ورنہ
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# اگر defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    قسمتعریف uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    قسمتعریف غیرعلامتی صحیح uptr;
# ورنہ
    قسمتعریف sqlite3_uint64 uptr;
# endif
#اگر_ختم
#شامل <ctype.h>

/* Used to get the current process ID */
#اگر !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#وگرنہ_اگر !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_اوسط
#   define WIN32_LEAN_AND_اوسط
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (صحیح)GetCurrentProcessId
#اگر_ختم

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#اگر_تعریف_شدہ BUILD_sqlite
#تعریف_منسوخ TCL_STORAGE_CLASS
#تعریف_کرو TCL_STORAGE_CLASS DLLEXPORT
#اگر_ختم /* BUILD_sqlite */

#تعریف_کرو عدد_PREPARED_STMTS 10
#تعریف_کرو MAX_PREPARED_STMTS 100

/* Forward declaration */
قسمتعریف ساخت SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#اگر defined(SQLITE_ENABLE_QRF_اندر_TCL)
#شامل "qrf.h"
#اگر_ختم

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
قسمتعریف ساخت SqlFunc SqlFunc;
ساخت SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  صحیح useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  صحیح eType;            /* Type of value to return */
  حرف *zName;          /* Name of this function */
  SqlFunc *p_اگلا;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
قسمتعریف ساخت SqlCollate SqlCollate;
ساخت SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  حرف *zScript;        /* The script to be run */
  SqlCollate *p_اگلا;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
قسمتعریف ساخت SqlPreparedStmt SqlPreparedStmt;
ساخت SqlPreparedStmt {
  SqlPreparedStmt *p_اگلا;  /* Next in linked list */
  SqlPreparedStmt *p_پچھلا;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  صحیح nSql;                /* chars in zSql[] */
  ثابت حرف *zSql;        /* Text of the SQL statement */
  صحیح nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

قسمتعریف ساخت IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
ساخت SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  حرف *zBusy;               /* The busy callback routine */
  حرف *zCommit;             /* The commit hook callback routine */
  حرف *zTrace;              /* The trace callback routine */
  حرف *zTraceV2;            /* The trace_v2 callback routine */
  حرف *zProfile;            /* The profile callback routine */
  حرف *z_پیش_رفت;           /* The progress callback routine */
  حرف *zBindFallback;       /* Callback to invoke on a binding miss */
  حرف *zAuth;               /* The authorization callback routine */
  صحیح disableAuth;           /* Disable the authorizer if it exists */
  حرف *zNull;               /* Text to substitute for an SQL NULL value */
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
  SqlPreparedStmt *stmt_آخری; /* Last statement in the list */
  صحیح maxStmt;               /* The next maximum number of stmtList */
  صحیح nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  صحیح n_قدم, n_ترتیب_دو, n_اشاریہ;  /* Statistics for most recent operation */
  صحیح nVMStep;               /* Another statistic for most recent operation */
  صحیح nTransaction;          /* Number of nested [transaction] methods */
  صحیح openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  صحیح nRef;                  /* Delete object when this reaches 0 */
#اگر_تعریف_شدہ SQLITE_TEST
  صحیح bLegacyPrepare;        /* True to use sqlite3_prepare() */
#اگر_ختم
};

ساخت IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  غیرعلامتی صحیح isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *p_اگلا;   /* Linked list of all open incrblob channels */
  IncrblobChannel *p_پچھلا;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
ساکن صحیح strlen30(ثابت حرف *z){
  ثابت حرف *z2 = z;
  جب( *z2 ){ z2++; }
  واپس 0x3fffffff & (صحیح)(z2 - z);
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
ساکن خالی closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *p_اگلا;

  برائے(p=pDb->pIncrblob; p; p=p_اگلا){
    p_اگلا = p->p_اگلا;

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
ساکن صحیح SQLITE_TCLAPI incrblobClose2(
  Client_ڈیٹا instance_ڈیٹا,
  Tcl_Interp *interp,
  صحیح flags
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ڈیٹا;
  صحیح  rc;
  sqlite3 *db = p->pDb->db;

  اگر( flags ){
    p->isClosed |= flags;
    واپس TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  اگر( p->p_اگلا ){
    p->p_اگلا->p_پچھلا = p->p_پچھلا;
  }
  اگر( p->p_پچھلا ){
    p->p_پچھلا->p_اگلا = p->p_اگلا;
  }
  اگر( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->p_اگلا;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((حرف *)p);

  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3_errmsg(db), TCL_VOLATILE);
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}
ساکن صحیح SQLITE_TCLAPI incrblobClose(
  Client_ڈیٹا instance_ڈیٹا,
  Tcl_Interp *interp
){
  واپس incrblobClose2(instance_ڈیٹا, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
ساکن صحیح SQLITE_TCLAPI incrblob_داخلہ(
  Client_ڈیٹا instance_ڈیٹا,
  حرف *buf,
  صحیح buf_سائز,
  صحیح *error_رمز_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ڈیٹا;
  sqlite3_int64 nRead = buf_سائز;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  صحیح rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_بائٹیں(p->pBlob);
  اگر( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  اگر( nRead<=0 ){
    واپس 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (خالی *)buf, (صحیح)nRead, (صحیح)p->iSeek);
  اگر( rc!=SQLITE_OK ){
    *error_رمز_Ptr = rc;
    واپس -1;
  }

  p->iSeek += nRead;
  واپس nRead;
}

/*
** Write data to an incremental blob channel.
*/
ساکن صحیح SQLITE_TCLAPI incrblobOutput(
  Client_ڈیٹا instance_ڈیٹا,
  ثابت حرف *buf,
  صحیح مقصد_Write,
  صحیح *error_رمز_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ڈیٹا;
  sqlite3_int64 nWrite = مقصد_Write;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  صحیح rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_بائٹیں(p->pBlob);
  اگر( (p->iSeek+nWrite)>nBlob ){
    *error_رمز_Ptr = EINVAL;
    واپس -1;
  }
  اگر( nWrite<=0 ){
    واپس 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (خالی*)buf,(صحیح)nWrite, (صحیح)p->iSeek);
  اگر( rc!=SQLITE_OK ){
    *error_رمز_Ptr = EIO;
    واپس -1;
  }

  p->iSeek += nWrite;
  واپس nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#اگر TCL_MAJOR_نسخہ==9
# define WideSeekProcType طویل طویل
#ورنہ
# define WideSeekProcType Tcl_WideInt
#اگر_ختم

/*
** Seek an incremental blob channel.
*/
ساکن WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  Client_ڈیٹا instance_ڈیٹا,
  WideSeekProcType انحراف,
  صحیح seek_منوال,
  صحیح *error_رمز_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ڈیٹا;

  انتخاب( seek_منوال ){
    حالت SEEK_SET:
      p->iSeek = انحراف;
      توڑو;
    حالت SEEK_CUR:
      p->iSeek += انحراف;
      توڑو;
    حالت SEEK_END:
      p->iSeek = sqlite3_blob_بائٹیں(p->pBlob) + انحراف;
      توڑو;

    طےشدہ: أكد(!"Bad seekMode");
  }

  واپس p->iSeek;
}
ساکن صحیح SQLITE_TCLAPI incrblobSeek(
  Client_ڈیٹا instance_ڈیٹا,
  طویل انحراف,
  صحیح seek_منوال,
  صحیح *error_رمز_Ptr
){
  واپس incrblobWideSeek(instance_ڈیٹا,انحراف,seek_منوال,error_رمز_Ptr);
}


ساکن خالی SQLITE_TCLAPI incrblobWatch(
  Client_ڈیٹا instance_ڈیٹا,
  صحیح منوال
){
  /* NO-OP */
}
ساکن صحیح SQLITE_TCLAPI incrblobHandle(
  Client_ڈیٹا instance_ڈیٹا,
  صحیح dir,
  Client_ڈیٹا *hPtr
){
  واپس TCL_ERROR;
}

ساکن Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_نسخہ_5,             /*نسخہ*/
  incrblobClose,                     /* closeProc                            */
  incrblob_داخلہ,                     /* inputProc                            */
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
ساکن صحیح createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  ثابت حرف *zDb,
  ثابت حرف *z_جدول,
  ثابت حرف *z_عمود,
  sqlite_int64 i_صف,
  صحیح isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  صحیح rc;
  صحیح flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  ساکن صحیح گنتی = 0;
  حرف zChannel[64];

  rc = sqlite3_blob_open(db, zDb, z_جدول, z_عمود, i_صف, !isReadonly, &pBlob);
  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    واپس TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(حجم(IncrblobChannel));
  memset(p, 0, حجم(*p));
  p->pBlob = pBlob;
  اگر( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(حجم(zChannel), zChannel, "incrblob_%d", ++گنتی);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_رجسٹر_Channel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->p_اگلا = pDb->pIncrblob;
  p->p_پچھلا = 0;
  اگر( p->p_اگلا ){
    p->p_اگلا->p_پچھلا = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_سیٹ_نتیجہ(interp, (حرف *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  واپس TCL_OK;
}
#ورنہ  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
  #تعریف_کرو closeIncrblobChannels(pDb)
#اگر_ختم

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
ساکن صحیح محفوظ_مقصد_Use_Eval_Objv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  ثابت حرف *z;
  Tcl_سائز n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  جب( n-- > 0 ){
    صحیح c = *(z++);
    اگر( c=='$' || c=='[' || c==';' ) واپس 0;
  }
  واپس 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
ساکن SqlFunc *ڈھونڈو_Sql_Func(SqliteDb *pDb, ثابت حرف *zName){
  SqlFunc *p, *pNew;
  صحیح nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( حجم(*pNew) + nName + 1 );
  pNew->zName = (حرف*)&pNew[1];
  حافظہ_نقل(pNew->zName, zName, nName+1);
  برائے(p=pDb->pFunc; p; p=p->p_اگلا){
    اگر( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((حرف*)pNew);
      واپس p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->p_اگلا = pDb->pFunc;
  pDb->pFunc = pNew;
  واپس pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
ساکن خالی dbFreeStmt(SqlPreparedStmt *pStmt){
#اگر_تعریف_شدہ SQLITE_TEST
  اگر( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((حرف *)pStmt->zSql);
  }
#اگر_ختم
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((حرف *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
ساکن خالی flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *p_اگلا;

  برائے(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=p_اگلا){
    p_اگلا = pPreStmt->p_اگلا;
    dbFreeStmt(pPreStmt);
  }
  pDb->nStmt = 0;
  pDb->stmt_آخری = 0;
  pDb->stmtList = 0;
}

/*
** Increment the reference counter on the SqliteDb object. The reference
** should be released by calling delDatabaseRef().
*/
ساکن خالی جوڑو_Database_Ref(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
ساکن خالی delDatabaseRef(SqliteDb *pDb){
  أكد( pDb->nRef>0 );
  pDb->nRef--;
  اگر( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    جب( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->p_اگلا;
      أكد( pFunc->pDb==pDb );
      Tcl_Decr_Ref_گنتی(pFunc->pScript);
      Tcl_Free((حرف*)pFunc);
    }
    جب( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->p_اگلا;
      Tcl_Free((حرف*)pCollate);
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
      Tcl_Decr_Ref_گنتی(pDb->pUpdateHook);
    }
    اگر( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_گنتی(pDb->pPreUpdateHook);
    }
    اگر( pDb->pRollbackHook ){
      Tcl_Decr_Ref_گنتی(pDb->pRollbackHook);
    }
    اگر( pDb->pWalHook ){
      Tcl_Decr_Ref_گنتی(pDb->pWalHook);
    }
    اگر( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_گنتی(pDb->pCollateNeeded);
    }
    Tcl_Free((حرف*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
ساکن خالی SQLITE_TCLAPI DbDeleteCmd(خالی *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
ساکن صحیح DbBusyHandler(خالی *cd, صحیح nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحیح rc;
  حرف zVal[30];

  sqlite3_snprintf(حجم(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (حرف*)0);
  اگر( rc!=TCL_OK || atoi(Tcl_Get_String_نتیجہ(pDb->interp)) ){
    واپس 0;
  }
  واپس 1;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_پیش_رفت_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
ساکن صحیح Db_پیش_رفت_Handler(خالی *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحیح rc;

  أكد( pDb->z_پیش_رفت );
  rc = Tcl_Eval(pDb->interp, pDb->z_پیش_رفت);
  اگر( rc!=TCL_OK || atoi(Tcl_Get_String_نتیجہ(pDb->interp)) ){
    واپس 1;
  }
  واپس 0;
}
#اگر_ختم

#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_نقطہ) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
ساکن خالی DbTraceHandler(خالی *cd, ثابت حرف *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_قدر(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_نتیجہ(pDb->interp);
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
ساکن صحیح DbTraceV2Handler(
  غیرعلامتی type, /* One of the SQLITE_TRACE_* event types. */
  خالی *cd,      /* The original context data pointer. */
  خالی *pd,      /* Primary event data, depends on event type. */
  خالی *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  انتخاب( type ){
    حالت SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      حرف *zSql = (حرف *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_گنتی(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_براہ_راست);
      Tcl_Decr_Ref_گنتی(pCmd);
      Tcl_Reset_نتیجہ(pDb->interp);
      توڑو;
    }
    حالت SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_گنتی(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_براہ_راست);
      Tcl_Decr_Ref_گنتی(pCmd);
      Tcl_Reset_نتیجہ(pDb->interp);
      توڑو;
    }
    حالت SQLITE_TRACE_صف: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_گنتی(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_براہ_راست);
      Tcl_Decr_Ref_گنتی(pCmd);
      Tcl_Reset_نتیجہ(pDb->interp);
      توڑو;
    }
    حالت SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_گنتی(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_براہ_راست);
      Tcl_Decr_Ref_گنتی(pCmd);
      Tcl_Reset_نتیجہ(pDb->interp);
      توڑو;
    }
  }
  واپس SQLITE_OK;
}
#اگر_ختم

#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_نقطہ) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
ساکن خالی DbProfileHandler(خالی *cd, ثابت حرف *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  حرف zTm[100];

  sqlite3_snprintf(حجم(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_قدر(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_نتیجہ(pDb->interp);
}
#اگر_ختم

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
ساکن صحیح DbCommitHandler(خالی *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحیح rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  اگر( rc!=TCL_OK || atoi(Tcl_Get_String_نتیجہ(pDb->interp)) ){
    واپس 1;
  }
  واپس 0;
}

ساکن خالی DbRollbackHandler(خالی *client_ڈیٹا){
  SqliteDb *pDb = (SqliteDb*)client_ڈیٹا;
  أكد(pDb->pRollbackHook);
  اگر( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
ساکن صحیح DbWalHandler(
  خالی *client_ڈیٹا,
  sqlite3 *db,
  ثابت حرف *zDb,
  صحیح n_اندراج
){
  صحیح ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)client_ڈیٹا;
  Tcl_Interp *interp = pDb->interp;
  أكد(pDb->pWalHook);

  أكد( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_گنتی(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(n_اندراج));
  اگر( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_Get_Obj_نتیجہ(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_گنتی(p);

  واپس ret;
}

#اگر defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ساکن خالی سیٹ_Test_Unlock_Notify_Vars(Tcl_Interp *interp, صحیح iArg, صحیح nArg){
  حرف zBuf[64];
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", iArg);
  Tcl_سیٹ_Var(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(حجم(zBuf), zBuf, "%d", nArg);
  Tcl_سیٹ_Var(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#ورنہ
# define سیٹ_Test_Unlock_Notify_Vars(x,y,z)
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_UNLOCK_NOTIFY
ساکن خالی DbUnlockNotify(خالی **apArg, صحیح nArg){
  صحیح i;
  برائے(i=0; i<nArg; i++){
    ثابت صحیح flags = (TCL_EVAL_GLOBAL|TCL_EVAL_براہ_راست);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    سیٹ_Test_Unlock_Notify_Vars(pDb->interp, i, nArg);
    أكد( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_گنتی(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
ساکن خالی DbPreUpdateHandler(
  خالی *p,
  sqlite3 *db,
  صحیح op,
  ثابت حرف *zDb,
  ثابت حرف *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ساکن ثابت حرف *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  أكد( (SQLITE_DELETE-1)/9 == 0 );
  أكد( (SQLITE_داخل_کرو-1)/9 == 1 );
  أكد( (SQLITE_UPDATE-1)/9 == 2 );
  أكد( pDb->pPreUpdateHook );
  أكد( db==pDb->db );
  أكد( op==SQLITE_داخل_کرو || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_گنتی(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_براہ_راست);
  Tcl_Decr_Ref_گنتی(pCmd);
}
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */

ساکن خالی DbUpdateHandler(
  خالی *p,
  صحیح op,
  ثابت حرف *zDb,
  ثابت حرف *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ساکن ثابت حرف *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  أكد( (SQLITE_DELETE-1)/9 == 0 );
  أكد( (SQLITE_داخل_کرو-1)/9 == 1 );
  أكد( (SQLITE_UPDATE-1)/9 == 2 );

  أكد( pDb->pUpdateHook );
  أكد( op==SQLITE_داخل_کرو || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_گنتی(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_براہ_راست);
  Tcl_Decr_Ref_گنتی(pCmd);
}

ساکن خالی tclCollateNeeded(
  خالی *pCtx,
  sqlite3 *db,
  صحیح enc,
  ثابت حرف *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_گنتی(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_گنتی(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
ساکن صحیح tclSqlCollate(
  خالی *pCtx,
  صحیح nA,
  ثابت خالی *zA,
  صحیح nB,
  ثابت خالی *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_گنتی(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_براہ_راست);
  Tcl_Decr_Ref_گنتی(pCmd);
  واپس (atoi(Tcl_Get_String_نتیجہ(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
ساکن خالی tclSqlFunc(sqlite3_context *context, صحیح argc, sqlite3_قدر**argv){
  SqlFunc *p = sqlite3_user_ڈیٹا(context);
  Tcl_Obj *pCmd;
  صحیح i;
  صحیح rc;

  اگر( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_گنتی(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_گنتی(pCmd);
  }ورنہ{
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
    Tcl_سائز nArg;
    اگر( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_نتیجہ_error(context, Tcl_Get_String_نتیجہ(p->interp), -1);
      واپس;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_گنتی(pCmd);
    برائے(i=0; i<argc; i++){
      sqlite3_قدر *p_اندر = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      انتخاب( sqlite3_قدر_type(p_اندر) ){
        حالت SQLITE_BLOB: {
          صحیح بائٹیں = sqlite3_قدر_بائٹیں(p_اندر);
          pVal = Tcl_NewByteArrayObj(sqlite3_قدر_blob(p_اندر), بائٹیں);
          توڑو;
        }
        حالت SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_قدر_int64(p_اندر);
          اگر( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((صحیح)v);
          }ورنہ{
            pVal = Tcl_NewWideIntObj(v);
          }
          توڑو;
        }
        حالت SQLITE_FLOAT: {
          دوہرا r = sqlite3_قدر_double(p_اندر);
          pVal = Tcl_NewDoubleObj(r);
          توڑو;
        }
        حالت SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          توڑو;
        }
        طےشدہ: {
          صحیح بائٹیں = sqlite3_قدر_بائٹیں(p_اندر);
          pVal = Tcl_NewStringObj((حرف *)sqlite3_قدر_متن(p_اندر), بائٹیں);
          توڑو;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      اگر( rc ){
        Tcl_Decr_Ref_گنتی(pCmd);
        sqlite3_نتیجہ_error(context, Tcl_Get_String_نتیجہ(p->interp), -1);
        واپس;
      }
    }
    اگر( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_براہ_راست);
    Tcl_Decr_Ref_گنتی(pCmd);
  }

  اگر( TCL_BREAK==rc ){
    sqlite3_نتیجہ_null(context);
  }ورنہ اگر( rc && rc!=TCL_RETURN ){
    sqlite3_نتیجہ_error(context, Tcl_Get_String_نتیجہ(p->interp), -1);
  }ورنہ{
    Tcl_Obj *pVar = Tcl_Get_Obj_نتیجہ(p->interp);
    Tcl_سائز n;
    u8 *ڈیٹا;
    ثابت حرف *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    حرف c = zType[0];
    صحیح eType = p->eType;

    اگر( eType==SQLITE_NULL ){
      اگر( c=='b' && strcmp(zType,"bytearray")==0 && pVar->بائٹیں==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }ورنہ اگر( (c=='b' && pVar->بائٹیں==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->بائٹیں==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }ورنہ اگر( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }ورنہ{
        eType = SQLITE_متن;
      }
    }

    انتخاب( eType ){
      حالت SQLITE_BLOB: {
        ڈیٹا = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_نتیجہ_blob64(context, ڈیٹا, n, SQLITE_TRANSIENT);
        توڑو;
      }
      حالت SQLITE_INTEGER: {
        Tcl_WideInt v;
        اگر( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_نتیجہ_int64(context, v);
          توڑو;
        }
        /* fall-through */
      }
      حالت SQLITE_FLOAT: {
        دوہرا r;
        اگر( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_نتیجہ_double(context, r);
          توڑو;
        }
        /* fall-through */
      }
      طےشدہ: {
        ڈیٹا = (غیرعلامتی حرف *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_نتیجہ_text64(context, (حرف *)ڈیٹا, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        توڑو;
      }
    }

  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
ساکن صحیح auth_callback(
  خالی *pArg,
  صحیح رمز,
  ثابت حرف *zArg1,
  ثابت حرف *zArg2,
  ثابت حرف *zArg3,
  ثابت حرف *zArg4
){
  ثابت حرف *z_رمز;
  Tcl_DString str;
  صحیح rc;
  ثابت حرف *z_جواب;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  اگر( pDb->disableAuth ) واپس SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  انتخاب( رمز ){
    حالت SQLITE_COPY              : z_رمز="SQLITE_COPY"; توڑو;
    حالت SQLITE_CREATE_اشاریہ      : z_رمز="SQLITE_CREATE_INDEX"; توڑو;
    حالت SQLITE_CREATE_جدول      : z_رمز="SQLITE_CREATE_TABLE"; توڑو;
    حالت SQLITE_CREATE_عارضی_اشاریہ : z_رمز="SQLITE_CREATE_TEMP_INDEX"; توڑو;
    حالت SQLITE_CREATE_عارضی_جدول : z_رمز="SQLITE_CREATE_TEMP_TABLE"; توڑو;
    حالت SQLITE_CREATE_عارضی_TRIGGER: z_رمز="SQLITE_CREATE_TEMP_TRIGGER"; توڑو;
    حالت SQLITE_CREATE_عارضی_VIEW  : z_رمز="SQLITE_CREATE_TEMP_VIEW"; توڑو;
    حالت SQLITE_CREATE_TRIGGER    : z_رمز="SQLITE_CREATE_TRIGGER"; توڑو;
    حالت SQLITE_CREATE_VIEW       : z_رمز="SQLITE_CREATE_VIEW"; توڑو;
    حالت SQLITE_DELETE            : z_رمز="SQLITE_DELETE"; توڑو;
    حالت SQLITE_DROP_اشاریہ        : z_رمز="SQLITE_DROP_INDEX"; توڑو;
    حالت SQLITE_DROP_جدول        : z_رمز="SQLITE_DROP_TABLE"; توڑو;
    حالت SQLITE_DROP_عارضی_اشاریہ   : z_رمز="SQLITE_DROP_TEMP_INDEX"; توڑو;
    حالت SQLITE_DROP_عارضی_جدول   : z_رمز="SQLITE_DROP_TEMP_TABLE"; توڑو;
    حالت SQLITE_DROP_عارضی_TRIGGER : z_رمز="SQLITE_DROP_TEMP_TRIGGER"; توڑو;
    حالت SQLITE_DROP_عارضی_VIEW    : z_رمز="SQLITE_DROP_TEMP_VIEW"; توڑو;
    حالت SQLITE_DROP_TRIGGER      : z_رمز="SQLITE_DROP_TRIGGER"; توڑو;
    حالت SQLITE_DROP_VIEW         : z_رمز="SQLITE_DROP_VIEW"; توڑو;
    حالت SQLITE_داخل_کرو            : z_رمز="SQLITE_INSERT"; توڑو;
    حالت SQLITE_PRAGMA            : z_رمز="SQLITE_PRAGMA"; توڑو;
    حالت SQLITE_READ              : z_رمز="SQLITE_READ"; توڑو;
    حالت SQLITE_SELECT            : z_رمز="SQLITE_SELECT"; توڑو;
    حالت SQLITE_TRANSACTION       : z_رمز="SQLITE_TRANSACTION"; توڑو;
    حالت SQLITE_UPDATE            : z_رمز="SQLITE_UPDATE"; توڑو;
    حالت SQLITE_ATTACH            : z_رمز="SQLITE_ATTACH"; توڑو;
    حالت SQLITE_DETACH            : z_رمز="SQLITE_DETACH"; توڑو;
    حالت SQLITE_ALTER_جدول       : z_رمز="SQLITE_ALTER_TABLE"; توڑو;
    حالت SQLITE_REINDEX           : z_رمز="SQLITE_REINDEX"; توڑو;
    حالت SQLITE_ANALYZE           : z_رمز="SQLITE_ANALYZE"; توڑو;
    حالت SQLITE_CREATE_VTABLE     : z_رمز="SQLITE_CREATE_VTABLE"; توڑو;
    حالت SQLITE_DROP_VTABLE       : z_رمز="SQLITE_DROP_VTABLE"; توڑو;
    حالت SQLITE_FUNCTION          : z_رمز="SQLITE_FUNCTION"; توڑو;
    حالت SQLITE_SAVEPOINT         : z_رمز="SQLITE_SAVEPOINT"; توڑو;
    حالت SQLITE_RECURSIVE         : z_رمز="SQLITE_RECURSIVE"; توڑو;
    طےشدہ                       : z_رمز="????"; توڑو;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, z_رمز);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_قدر(&str));
  Tcl_DStringFree(&str);
  z_جواب = rc==TCL_OK ? Tcl_Get_String_نتیجہ(pDb->interp) : "SQLITE_DENY";
  اگر( strcmp(z_جواب,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }ورنہ اگر( strcmp(z_جواب,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }ورنہ اگر( strcmp(z_جواب,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }ورنہ{
    rc = 999;
  }
  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_AUTHORIZATION */

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
ساکن حرف *local_getline(حرف *zPrompt, FILE *اندر){
  حرف *z_سطر;
  صحیح n_سطر;
  صحیح n;

  n_سطر = 100;
  z_سطر = حافظہ_دو( n_سطر );
  اگر( z_سطر==0 ) واپس 0;
  n = 0;
  جب( 1 ){
    اگر( n+100>n_سطر ){
      n_سطر = n_سطر*2 + 100;
      z_سطر = realloc(z_سطر, n_سطر);
      اگر( z_سطر==0 ) واپس 0;
    }
    اگر( fgets(&z_سطر[n], n_سطر - n, اندر)==0 ){
      اگر( n==0 ){
        آزاد(z_سطر);
        واپس 0;
      }
      z_سطر[n] = 0;
      توڑو;
    }
    جب( z_سطر[n] ){ n++; }
    اگر( n>0 && z_سطر[n-1]=='\n' ){
      n--;
      z_سطر[n] = 0;
      توڑو;
    }
  }
  z_سطر = realloc( z_سطر, n+1 );
  واپس z_سطر;
}
#اگر_ختم


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
ساکن صحیح SQLITE_TCLAPI DbTransPostCmd(
  Client_ڈیٹا ڈیٹا[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  صحیح نتیجہ                           /* Result of evaluating SCRIPT */
){
  ساکن ثابت حرف *ثابت az_انتہا[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)ڈیٹا[0];
  صحیح rc = نتیجہ;
  ثابت حرف *z_انتہا;

  pDb->nTransaction--;
  z_انتہا = az_انتہا[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  اگر( sqlite3_exec(pDb->db, z_انتہا, 0, 0, 0) ){
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
      Tcl_Append_نتیجہ(interp, sqlite3_errmsg(pDb->db), (حرف*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  واپس rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
ساکن صحیح dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  ثابت حرف *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  ثابت حرف **pz_برآمد              /* OUT: Pointer to next SQL statement */
){
  غیرعلامتی صحیح prepFlags = 0;
#اگر_تعریف_شدہ SQLITE_TEST
  اگر( pDb->bLegacyPrepare ){
    واپس sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pz_برآمد);
  }
#اگر_ختم
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  اگر( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  واپس sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pz_برآمد);
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
ساکن صحیح dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  حرف ثابت *z_اندر,                /* SQL to compile */
  حرف ثابت **pz_برآمد,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  ثابت حرف *zSql = z_اندر;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  صحیح nSql;                       /* Length of zSql in bytes */
  صحیح nVar = 0;                   /* Number of variables in statement */
  صحیح iParm = 0;                  /* Next free entry in apParm */
  حرف c;
  صحیح i;
  صحیح need_نتیجہ_Reset = 0;        /* Need to invoke Tcl_ResetResult() */
  صحیح rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  جب( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  برائے(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->p_اگلا){
    صحیح n = pPreStmt->nSql;
    اگر( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pz_برآمد = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      اگر( pPreStmt->p_پچھلا ){
        pPreStmt->p_پچھلا->p_اگلا = pPreStmt->p_اگلا;
      }ورنہ{
        pDb->stmtList = pPreStmt->p_اگلا;
      }
      اگر( pPreStmt->p_اگلا ){
        pPreStmt->p_اگلا->p_پچھلا = pPreStmt->p_پچھلا;
      }ورنہ{
        pDb->stmt_آخری = pPreStmt->p_پچھلا;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_گنتی(pStmt);
      توڑو;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  اگر( pPreStmt==0 ){
    صحیح nByte;

    اگر( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pz_برآمد) ){
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      واپس TCL_ERROR;
    }
    اگر( pStmt==0 ){
      اگر( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        واپس TCL_ERROR;
      }ورنہ{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        واپس TCL_OK;
      }
    }

    أكد( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_گنتی(pStmt);
    nByte = حجم(SqlPreparedStmt) + nVar*حجم(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (صحیح)(*pz_برآمد - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#اگر_تعریف_شدہ SQLITE_TEST
    اگر( pPreStmt->zSql==0 ){
      حرف *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      حافظہ_نقل(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#اگر_ختم
  }
  أكد( pPreStmt );
  أكد( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  أكد( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  برائے(i=1; i<=nVar; i++){
    ثابت حرف *zVar = sqlite3_bind_parameter_name(pStmt, i);
    اگر( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      اگر( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        صحیح rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_گنتی(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        اگر( need_نتیجہ_Reset ) Tcl_Reset_نتیجہ(interp);
        need_نتیجہ_Reset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_براہ_راست);
        Tcl_Decr_Ref_گنتی(pCmd);
        اگر( rx==TCL_OK ){
          pVar = Tcl_Get_Obj_نتیجہ(interp);
        }ورنہ اگر( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          توڑو;
        }ورنہ{
          pVar = 0;
        }
      }
      اگر( pVar ){
        Tcl_سائز n;
        u8 *ڈیٹا;
        ثابت حرف *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        اگر( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->بائٹیں==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          ڈیٹا = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, ڈیٹا, n, SQLITE_STATIC);
          Tcl_Incr_Ref_گنتی(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }ورنہ اگر( c=='b' && pVar->بائٹیں==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          صحیح nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }ورنہ اگر( c=='d' && strcmp(zType,"double")==0 ){
          دوہرا r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }ورنہ اگر( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }ورنہ{
          ڈیٹا = (غیرعلامتی حرف *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (حرف *)ڈیٹا, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_گنتی(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }ورنہ{
        sqlite3_bind_null(pStmt, i);
      }
      اگر( need_نتیجہ_Reset ) Tcl_Reset_نتیجہ(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  اگر( need_نتیجہ_Reset && rc==TCL_OK ) Tcl_Reset_نتیجہ(pDb->interp);

  واپس rc;
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
ساکن خالی dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  صحیح discard                     /* True to delete (not cache) the pPreStmt */
){
  صحیح i;

  /* Free the bound string and blob parameters */
  برائے(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_گنتی(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  اگر( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }ورنہ{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->p_اگلا = pDb->stmtList;
    pPreStmt->p_پچھلا = 0;
    اگر( pDb->stmtList ){
     pDb->stmtList->p_پچھلا = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    اگر( pDb->stmt_آخری==0 ){
      أكد( pDb->nStmt==0 );
      pDb->stmt_آخری = pPreStmt;
    }ورنہ{
      أكد( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    جب( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *p_آخری = pDb->stmt_آخری;
      pDb->stmt_آخری = p_آخری->p_پچھلا;
      pDb->stmt_آخری->p_اگلا = 0;
      pDb->nStmt--;
      dbFreeStmt(p_آخری);
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
قسمتعریف ساخت DbEvalContext DbEvalContext;
ساخت DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  ثابت حرف *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  صحیح n_ستون;                       /* Number of columns returned by pStmt */
  صحیح evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **ap_ستون_Name;            /* Array of column names */
};

#تعریف_کرو SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#تعریف_کرو SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
ساکن خالی db_Release_عمود_نام(DbEvalContext *p){
  اگر( p->ap_ستون_Name ){
    صحیح i;
    برائے(i=0; i<p->n_ستون; i++){
      Tcl_Decr_Ref_گنتی(p->ap_ستون_Name[i]);
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
ساکن خالی dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  صحیح evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, حجم(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_گنتی(pSql);
  اگر( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_گنتی(pVarName);
  }
  p->evalFlags = evalFlags;
  جوڑو_Database_Ref(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
ساکن خالی db_Eval_صف_Info(
  DbEvalContext *p,               /* Evaluation context */
  صحیح *pn_ستون,                     /* OUT: Number of column names */
  Tcl_Obj ***pap_ستون_Name           /* OUT: Array of column names */
){
  /* Compute column names */
  اگر( 0==p->ap_ستون_Name ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    صحیح i;                        /* Iterator variable */
    صحیح n_ستون;                     /* Number of columns returned by pStmt */
    Tcl_Obj **ap_ستون_Name = 0;      /* Array of column names */

    p->n_ستون = n_ستون = sqlite3_عمود_گنتی(pStmt);
    اگر( n_ستون>0 && (pap_ستون_Name || p->pVarName) ){
      ap_ستون_Name = (Tcl_Obj**)Tcl_Alloc( حجم(Tcl_Obj*)*n_ستون );
      برائے(i=0; i<n_ستون; i++){
        ap_ستون_Name[i] = Tcl_NewStringObj(sqlite3_عمود_name(pStmt,i), -1);
        Tcl_Incr_Ref_گنتی(ap_ستون_Name[i]);
      }
      p->ap_ستون_Name = ap_ستون_Name;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    اگر( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *p_ستون_List = Tcl_NewObj();
      Tcl_Obj *p_ستارہ = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_گنتی(p_ستون_List);
      Tcl_Incr_Ref_گنتی(p_ستارہ);
      برائے(i=0; i<n_ستون; i++){
        Tcl_ListObjAppendElement(interp, p_ستون_List, ap_ستون_Name[i]);
      }
      اگر( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_Obj_سیٹ_Var2(interp, p->pVarName, p_ستارہ, p_ستون_List, 0);
      }ورنہ{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, تہی, 0);
        اگر( !pDict ){
          pDict = Tcl_NewDictObj();
        }ورنہ اگر( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        اگر( Tcl_DictObjPut(interp, pDict, p_ستارہ, p_ستون_List)==TCL_OK ){
          Tcl_Obj_سیٹ_Var2(interp, p->pVarName, تہی, pDict, 0);
        }
        Tcl_Bounce_Ref_گنتی(pDict);
      }
      Tcl_Decr_Ref_گنتی(p_ستارہ);
      Tcl_Decr_Ref_گنتی(p_ستون_List);
    }
  }

  اگر( pap_ستون_Name ){
    *pap_ستون_Name = p->ap_ستون_Name;
  }
  اگر( pn_ستون ){
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
ساکن صحیح db_Eval_قدم(DbEvalContext *p){
  ثابت حرف *z_پچھلا_Sql = 0;       /* Previous value of p->zSql */

  جب( p->zSql[0] || p->pPreStmt ){
    صحیح rc;
    اگر( p->pPreStmt==0 ){
      z_پچھلا_Sql = (p->zSql==z_پچھلا_Sql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      اگر( rc!=TCL_OK ) واپس rc;
    }ورنہ{
      صحیح rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_قدم(pStmt);
      اگر( rcs==SQLITE_صف ){
        واپس TCL_OK;
      }
      اگر( p->pVarName ){
        db_Eval_صف_Info(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->n_قدم = sqlite3_stmt_حالت(pStmt,SQLITE_STMTSTATUS_FULLSCAN_قدم,1);
      pDb->n_ترتیب_دو = sqlite3_stmt_حالت(pStmt,SQLITE_STMTSTATUS_ترتیب_دو,1);
      pDb->n_اشاریہ = sqlite3_stmt_حالت(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_حالت(pStmt,SQLITE_STMTSTATUS_VM_قدم,1);
      db_Release_عمود_نام(p);
      p->pPreStmt = 0;

      اگر( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#اگر SQLITE_TEST
        اگر( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && z_پچھلا_Sql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = z_پچھلا_Sql;
          جاری;
        }
#اگر_ختم
        Tcl_سیٹ_Obj_نتیجہ(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        واپس TCL_ERROR;
      }ورنہ{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /*ختم_شدہ*/
  واپس TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
ساکن خالی dbEvalFinalize(DbEvalContext *p){
  اگر( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  اگر( p->pVarName ){
    Tcl_Decr_Ref_گنتی(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_گنتی(p->pSql);
  db_Release_عمود_نام(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
ساکن Tcl_Obj *db_Eval_عمود_قدر(DbEvalContext *p, صحیح i_ستون){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  انتخاب( sqlite3_عمود_type(pStmt, i_ستون) ){
    حالت SQLITE_BLOB: {
      صحیح بائٹیں = sqlite3_عمود_بائٹیں(pStmt, i_ستون);
      ثابت حرف *zBlob = sqlite3_عمود_blob(pStmt, i_ستون);
      اگر( !zBlob ) بائٹیں = 0;
      واپس Tcl_NewByteArrayObj((u8*)zBlob, بائٹیں);
    }
    حالت SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_عمود_int64(pStmt, i_ستون);
      اگر( v>=-2147483647 && v<=2147483647 ){
        واپس Tcl_NewIntObj((صحیح)v);
      }ورنہ{
        واپس Tcl_NewWideIntObj(v);
      }
    }
    حالت SQLITE_FLOAT: {
      واپس Tcl_NewDoubleObj(sqlite3_عمود_double(pStmt, i_ستون));
    }
    حالت SQLITE_NULL: {
      واپس Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  واپس Tcl_NewStringObj((حرف*)sqlite3_عمود_متن(pStmt, i_ستون), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#اگر TCL_MAJOR_نسخہ>8 || !defined(TCL_MINOR_نسخہ) \
                        || TCL_MINOR_نسخہ>=6
# define SQLITE_TCL_NRE 1
ساکن صحیح DbUseNre(خالی){
  صحیح major, minor;
  Tcl_Get_نسخہ(&major, &minor, 0, 0);
  واپس( (major==8 && minor>=6) || major>8 );
}
#ورنہ
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (خالی)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (خالی)0
#اگر_ختم

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
ساکن صحیح SQLITE_TCLAPI Db_Eval_اگلا_Cmd(
  Client_ڈیٹا ڈیٹا[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  صحیح نتیجہ                           /* Result so far */
){
  صحیح rc = نتیجہ;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)ڈیٹا[0];
  Tcl_Obj * ثابت pScript = (Tcl_Obj *)ڈیٹا[1];
  Tcl_Obj * ثابت pVarName = p->pVarName;

  جب( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = db_Eval_قدم(p)) ){
    صحیح i;
    صحیح n_ستون;
    Tcl_Obj **ap_ستون_Name;
    db_Eval_صف_Info(p, &n_ستون, &ap_ستون_Name);
    برائے(i=0; i<n_ستون; i++){
      اگر( pVarName==0 ){
        Tcl_Obj_سیٹ_Var2(interp, ap_ستون_Name[i], 0, db_Eval_عمود_قدر(p,i), 0);
      }ورنہ اگر( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_عمود_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        اگر( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(ap_ستون_Name[i]), 0);
        }ورنہ{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, تہی, 0);
          اگر( pDict ){
            اگر( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            اگر( Tcl_DictObjRemove(interp, pDict, ap_ستون_Name[i])==TCL_OK ){
              Tcl_Obj_سیٹ_Var2(interp, pVarName, تہی, pDict, 0);
            }
            Tcl_Bounce_Ref_گنتی(pDict);
          }
        }
      }ورنہ اگر( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_Obj_سیٹ_Var2(interp, pVarName, ap_ستون_Name[i],
                       db_Eval_عمود_قدر(p,i), 0);
      }ورنہ{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, تہی, 0);
        اگر( !pDict ){
          pDict = Tcl_NewDictObj();
        }ورنہ اگر( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        اگر( Tcl_DictObjPut(interp, pDict, ap_ستون_Name[i],
                           db_Eval_عمود_قدر(p,i))==TCL_OK ){
          Tcl_Obj_سیٹ_Var2(interp, pVarName, تہی, pDict, 0);
        }
        Tcl_Bounce_Ref_گنتی(pDict);
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
      Tcl_NRAddCallback(interp, Db_Eval_اگلا_Cmd, (خالی*)p, (خالی*)pScript, 0, 0);
      واپس Tcl_NREvalObj(interp, pScript, 0);
    }ورنہ{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_گنتی(pScript);
  dbEvalFinalize(p);
  Tcl_Free((حرف *)p);

  اگر( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_Reset_نتیجہ(interp);
    rc = TCL_OK;
  }
  واپس rc;
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
ساکن خالی DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  اگر( *ppHook ){
    Tcl_سیٹ_Obj_نتیجہ(interp, *ppHook);
    اگر( pArg ){
      Tcl_Decr_Ref_گنتی(*ppHook);
      *ppHook = 0;
    }
  }
  اگر( pArg ){
    أكد( !(*ppHook) );
    اگر( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_گنتی(*ppHook);
    }
  }

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
  sqlite3_preupdate_hook(db, (pDb->pPreUpdateHook?DbPreUpdateHandler:0), pDb);
#اگر_ختم
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
ساکن صحیح dbQrf(SqliteDb *pDb, صحیح objc, Tcl_Obj *ثابت*objv){
#اگر_غیر_تعریف_شدہ SQLITE_QRF_H
  Tcl_سیٹ_نتیجہ(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  واپس TCL_ERROR;
#ورنہ
  حرف *z_نتیجہ = 0;             /* Result to be returned */
  ثابت حرف *zSql = 0;          /* SQL to run */
  صحیح i;                         /* Loop counter */
  صحیح rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  ساکن ثابت حرف *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  ساکن ثابت غیرعلامتی حرف aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_بایاں,
    QRF_ALIGN_درمیان, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_دایاں,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_چوٹی,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, حجم(qrf));
  qrf.i_نسخہ = 2;
  qrf.pzOutput = &z_نتیجہ;
  برائے(i=2; i<objc; i++){
    ثابت حرف *zArg = Tcl_GetString(objv[i]);
    ثابت حرف *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    ثابت غیرعلامتی حرف aBoolMap[] = { 0, 2, 1, 2, 1 };
    اگر( zArg[0]!='-' ){
      اگر( zSql ){
        Tcl_Append_نتیجہ(pDb->interp, "unknown argument: ", zArg, (حرف*)0);
        rc = TCL_ERROR;
        جاؤ format_failed;
      }
      zSql  = zArg;
    }ورنہ اگر( i==objc-1 ){
      Tcl_Append_نتیجہ(pDb->interp, "option has no argument: ", zArg,(حرف*)0);
      rc = TCL_ERROR;
      جاؤ format_failed;
    }ورنہ اگر( strcmp(zArg,"-style")==0 ){
      ساکن ثابت حرف *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      ساکن غیرعلامتی حرف aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_عمود,
        QRF_STYLE_گنتی,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_داخل_کرو,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_سطر,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_جدول
      };
      صحیح style;
      rc = Tcl_Get_اشاریہ_From_Obj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      اگر( rc ) جاؤ format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }ورنہ اگر( strcmp(zArg,"-esc")==0 ){
      ساکن ثابت حرف *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      ساکن غیرعلامتی حرف aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_علامت
      };
      صحیح esc;
      rc = Tcl_Get_اشاریہ_From_Obj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      اگر( rc ) جاؤ format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }ورنہ اگر( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      ساکن ثابت حرف *az_متن[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      ساکن غیرعلامتی حرف a_متن_Map[] = {
        QRF_متن_Auto,      QRF_متن_Csv,       QRF_متن_Html,
        QRF_متن_Json,      QRF_متن_Plain,     QRF_متن_Relaxed,
        QRF_متن_Sql,       QRF_متن_Tcl
      };
      صحیح txt;
      صحیح k = zArg[2]=='e';
      rc = Tcl_Get_اشاریہ_From_Obj(pDb->interp, objv[i+1], &az_متن[k*3], zArg,
                               0, &txt);
      اگر( rc ) جاؤ format_failed;
      اگر( k ){
        qrf.e_متن = a_متن_Map[txt];
      }ورنہ اگر( txt<=2 ){
        أكد( QRF_No==1 );
        أكد( QRF_Yes==2 );
        أكد( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_متن_Auto;
      }ورنہ{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = a_متن_Map[txt-3];
      }
      i++;
    }ورنہ اگر( strcmp(zArg,"-blob")==0 ){
      ساکن ثابت حرف *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      ساکن غیرعلامتی حرف aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_متن,      QRF_BLOB_Sql,
        QRF_BLOB_سائز
      };
      صحیح blob;
      rc = Tcl_Get_اشاریہ_From_Obj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      اگر( rc ) جاؤ format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }ورنہ اگر( strcmp(zArg,"-wordwrap")==0 ){
      صحیح v = 0;
      rc = Tcl_Get_اشاریہ_From_Obj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      اگر( rc ) جاؤ format_failed;
      qrf.b_لفظ_Wrap = aBoolMap[v];
      i++;
    }ورنہ اگر( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      صحیح v = 0;
      rc = Tcl_Get_اشاریہ_From_Obj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      اگر( rc ) جاؤ format_failed;
      اگر( zArg[1]=='t' ){
        qrf.b_متن_Jsonb = aBoolMap[v];
      }ورنہ اگر( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }ورنہ اگر( zArg[1]=='r' ){
        qrf.b_صف_گنتی = aBoolMap[v];
      }ورنہ{
        qrf.b_بانٹو_عمود = aBoolMap[v];
      }
      i++;
    }ورنہ اگر( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      صحیح ax = 0;
      rc = Tcl_Get_اشاریہ_From_Obj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      اگر( rc ) جاؤ format_failed;
      اگر( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }ورنہ{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }ورنہ اگر( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      صحیح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      اگر( rc ) جاؤ format_failed;
      اگر( v<QRF_MIN_چوڑائی ){
        v = QRF_MIN_چوڑائی;
      }ورنہ اگر( v>QRF_MAX_چوڑائی ){
        v = QRF_MAX_چوڑائی;
      }
      اگر( zArg[1]=='w' ){
        qrf.nWrap = v;
      }ورنہ اگر( zArg[1]=='s' ){
        qrf.n_Screen_چوڑائی = v;
      }ورنہ اگر( zArg[1]=='t' ){
        qrf.n_Title_حد_بندی = v;
      }ورنہ{
        qrf.n_سطر_حد_بندی = v;
      }
      i++;
    }ورنہ اگر( strcmp(zArg,"-charlimit")==0 ){
      صحیح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      اگر( rc ) جاؤ format_failed;
      اگر( v<0 ) v = 0;
      qrf.n_Char_حد_بندی = v;
      i++;
    }ورنہ اگر( strcmp(zArg,"-multiinsert")==0 ){
      صحیح v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      اگر( rc ) جاؤ format_failed;
      اگر( v<0 ) v = 0;
      qrf.n_Multi_داخل_کرو = v;
      i++;
    }ورنہ اگر( strcmp(zArg,"-align")==0 ){
      Tcl_سائز n = 0;
      صحیح jj;
      rc = Tcl_List_Obj_لمبائی(pDb->interp, objv[i+1], &n);
      اگر( rc ) جاؤ format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*حجم(qrf.aAlign[0]) );
      اگر( qrf.aAlign==0 ){
        Tcl_Append_نتیجہ(pDb->interp, "out of memory", (حرف*)0);
        rc = TCL_ERROR;
        جاؤ format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*حجم(qrf.aAlign[0]));
      qrf.nAlign = n;
      برائے(jj=0; jj<n; jj++){
        صحیح x;
        Tcl_Obj *p_رکن;
        rc = Tcl_List_Obj_اشاریہ(pDb->interp, objv[i+1], jj, &p_رکن);
        اگر( rc ) جاؤ format_failed;
        rc = Tcl_Get_اشاریہ_From_Obj(pDb->interp, p_رکن, azAlign,
                          "column alignment (-align)", 0, &x);
        اگر( rc ) جاؤ format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }ورنہ اگر( strcmp(zArg,"-widths")==0 ){
      Tcl_سائز n = 0;
      صحیح jj;
      rc = Tcl_List_Obj_لمبائی(pDb->interp, objv[i+1], &n);
      اگر( rc ) جاؤ format_failed;
      sqlite3_free(qrf.a_چوڑائی);
      qrf.a_چوڑائی = sqlite3_malloc64( (n+1)*حجم(qrf.a_چوڑائی[0]) );
      اگر( qrf.a_چوڑائی==0 ){
        Tcl_Append_نتیجہ(pDb->interp, "out of memory", (حرف*)0);
        rc = TCL_ERROR;
        جاؤ format_failed;
      }
      memset(qrf.a_چوڑائی, 0, (n+1)*حجم(qrf.a_چوڑائی[0]));
      qrf.n_چوڑائی = n;
      برائے(jj=0; jj<n; jj++){
        Tcl_Obj *p_رکن;
        صحیح v;
        rc = Tcl_List_Obj_اشاریہ(pDb->interp, objv[i+1], jj, &p_رکن);
        اگر( rc ) جاؤ format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, p_رکن, &v);
        اگر( v<(-QRF_MAX_چوڑائی) ){
          v = -QRF_MAX_چوڑائی;
        }ورنہ اگر( v>QRF_MAX_چوڑائی ){
          v = QRF_MAX_چوڑائی;
        }
        qrf.a_چوڑائی[jj] = (مختصر صحیح)v;
      }
      i++;
    }ورنہ اگر( strcmp(zArg,"-columnsep")==0 ){
      qrf.z_عمود_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }ورنہ اگر( strcmp(zArg,"-rowsep")==0 ){
      qrf.z_صف_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }ورنہ اگر( strcmp(zArg,"-tablename")==0 ){
      qrf.z_جدول_Name = Tcl_GetString(objv[i+1]);
      i++;
    }ورنہ اگر( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }ورنہ اگر( strcmp(zArg,"-ifmt")==0 ){
      حرف *zFmt = Tcl_GetString(objv[i+1]);
      اگر( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }ورنہ اگر( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_Append_نتیجہ(pDb->interp, "bad format for -ifmt", (حرف*)0);
        rc = TCL_ERROR;
        جاؤ format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }ورنہ اگر( strcmp(zArg,"-fpfmt")==0 ){
      حرف *zFmt = Tcl_GetString(objv[i+1]);
      اگر( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }ورنہ اگر( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_Append_نتیجہ(pDb->interp, "bad format for -fpfmt", (حرف*)0);
        rc = TCL_ERROR;
        جاؤ format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }ورنہ اگر( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.i_نسخہ = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }ورنہ{
      Tcl_Append_نتیجہ(pDb->interp, "unknown option: ", zArg, (حرف*)0);
      rc = TCL_ERROR;
      جاؤ format_failed;
    }
  }
  جب( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    حرف *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    اگر( rc ) جاؤ format_failed;
    اگر( pStmt==0 ) جاری;
    rc = sqlite3_format_query_نتیجہ(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    اگر( rc ){
      Tcl_سیٹ_نتیجہ(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      جاؤ format_failed;
    }
  }
  Tcl_سیٹ_نتیجہ(pDb->interp, z_نتیجہ, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.a_چوڑائی);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(z_نتیجہ);
  واپس rc;

#اگر_ختم
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
ساکن صحیح SQLITE_TCLAPI DbObjCmd(
  خالی *cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  صحیح choice;
  صحیح rc = TCL_OK;
  ساکن ثابت حرف *DB_strs[] = {
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
  شمار DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_بھرو_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_مداخلت,            DB_آخری_داخل_کرو_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_پیش_رفت,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_حالت,                DB_TIMEOUT,              DB_کل_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_نسخہ,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SUBCOMMAND ...");
    واپس TCL_ERROR;
  }
  اگر( Tcl_Get_اشاریہ_From_Obj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    واپس TCL_ERROR;
  }

  انتخاب( (شمار DB_enum)choice ){

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
#اگر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
    Tcl_Append_نتیجہ(interp, "authorization not available in this build",
                     (حرف*)0);
    واپس TCL_ERROR;
#ورنہ
    اگر( objc>3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?CALLBACK?");
      واپس TCL_ERROR;
    }ورنہ اگر( objc==2 ){
      اگر( pDb->zAuth ){
        Tcl_Append_نتیجہ(interp, pDb->zAuth, (حرف*)0);
      }
    }ورنہ{
      حرف *zAuth;
      Tcl_سائز len;
      اگر( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->zAuth, zAuth, len+1);
      }ورنہ{
        pDb->zAuth = 0;
      }
      اگر( pDb->zAuth ){
        قسمتعریف صحیح (*sqlite3_auth_cb)(
           خالی*,صحیح,ثابت حرف*,ثابت حرف*,
           ثابت حرف*,ثابت حرف*);
        pDb->interp = interp;
        sqlite3_سیٹ_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }ورنہ{
        sqlite3_سیٹ_authorizer(pDb->db, 0, 0);
      }
    }
#اگر_ختم
    توڑو;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  حالت DB_BACKUP: {
    ثابت حرف *zDestFile;
    ثابت حرف *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    اگر( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }ورنہ اگر( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }ورنہ{
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?DATABASE? FILENAME");
      واپس TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    اگر( rc!=SQLITE_OK ){
      Tcl_Append_نتیجہ(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (حرف*)0);
      sqlite3_close(pDest);
      واپس TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    اگر( pBackup==0 ){
      Tcl_Append_نتیجہ(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (حرف*)0);
      sqlite3_close(pDest);
      واپس TCL_ERROR;
    }
    جب(  (rc = sqlite3_backup_قدم(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_اختتام(pBackup);
    اگر( rc==SQLITE_مکمل ){
      rc = TCL_OK;
    }ورنہ{
      Tcl_Append_نتیجہ(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (حرف*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    توڑو;
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
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?CALLBACK?");
      واپس TCL_ERROR;
    }ورنہ اگر( objc==2 ){
      اگر( pDb->zBindFallback ){
        Tcl_Append_نتیجہ(interp, pDb->zBindFallback, (حرف*)0);
      }
    }ورنہ{
      حرف *zCallback;
      Tcl_سائز len;
      اگر( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->zBindFallback, zCallback, len+1);
      }ورنہ{
        pDb->zBindFallback = 0;
      }
    }
    توڑو;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  حالت DB_BUSY: {
    اگر( objc>3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "CALLBACK");
      واپس TCL_ERROR;
    }ورنہ اگر( objc==2 ){
      اگر( pDb->zBusy ){
        Tcl_Append_نتیجہ(interp, pDb->zBusy, (حرف*)0);
      }
    }ورنہ{
      حرف *zBusy;
      Tcl_سائز len;
      اگر( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->zBusy, zBusy, len+1);
      }ورنہ{
        pDb->zBusy = 0;
      }
      اگر( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }ورنہ{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    توڑو;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  حالت DB_CACHE: {
    حرف *subCmd;
    صحیح n;

    اگر( objc<=2 ){
      Tcl_Wrong_عدد_دلائل(interp, 1, objv, "cache option ?arg?");
      واپس TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    اگر( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      اگر( objc!=3 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "flush");
        واپس TCL_ERROR;
      }ورنہ{
        flushStmtCache( pDb );
      }
    }ورنہ اگر( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      اگر( objc!=4 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "size n");
        واپس TCL_ERROR;
      }ورنہ{
        اگر( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_Append_نتیجہ( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (حرف*)0);
          واپس TCL_ERROR;
        }ورنہ{
          اگر( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }ورنہ اگر( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }ورنہ{
      Tcl_Append_نتیجہ( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (حرف*)0);
      واپس TCL_ERROR;
    }
    توڑو;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  حالت DB_CHANGES: {
    Tcl_Obj *p_نتیجہ;
    اگر( objc!=2 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
      واپس TCL_ERROR;
    }
    p_نتیجہ = Tcl_Get_Obj_نتیجہ(interp);
    Tcl_سیٹ_Wide_Int_Obj(p_نتیجہ, sqlite3_changes64(pDb->db));
    توڑو;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  حالت DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    توڑو;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  حالت DB_COLLATE: {
    SqlCollate *pCollate;
    حرف *zName;
    حرف *zScript;
    Tcl_سائز nScript;
    اگر( objc!=4 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "NAME SCRIPT");
      واپس TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( حجم(*pCollate) + nScript + 1 );
    اگر( pCollate==0 ) واپس TCL_ERROR;
    pCollate->interp = interp;
    pCollate->p_اگلا = pDb->pCollate;
    pCollate->zScript = (حرف*)&pCollate[1];
    pDb->pCollate = pCollate;
    حافظہ_نقل(pCollate->zScript, zScript, nScript+1);
    اگر( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      واپس TCL_ERROR;
    }
    توڑو;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  حالت DB_COLLATION_NEEDED: {
    اگر( objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "SCRIPT");
      واپس TCL_ERROR;
    }
    اگر( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_گنتی(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_گنتی(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    توڑو;
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
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?CALLBACK?");
      واپس TCL_ERROR;
    }ورنہ اگر( objc==2 ){
      اگر( pDb->zCommit ){
        Tcl_Append_نتیجہ(interp, pDb->zCommit, (حرف*)0);
      }
    }ورنہ{
      ثابت حرف *zCommit;
      Tcl_سائز len;
      اگر( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->zCommit, zCommit, len+1);
      }ورنہ{
        pDb->zCommit = 0;
      }
      اگر( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }ورنہ{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    توڑو;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  حالت DB_COMPLETE: {
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPLETE
    Tcl_Obj *p_نتیجہ;
    صحیح isComplete;
    اگر( objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "SQL");
      واپس TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    p_نتیجہ = Tcl_Get_Obj_نتیجہ(interp);
    Tcl_سیٹ_Boolean_Obj(p_نتیجہ, isComplete);
#اگر_ختم
    توڑو;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  حالت DB_CONFIG: {
    ساکن ثابت ساخت DbConfigChoices {
      ثابت حرف *zName;
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
        { "legacy_alter_table", SQLITE_DBCONFIG_LEGACY_ALTER_جدول    },
        { "legacy_file_format", SQLITE_DBCONFIG_LEGACY_FILE_FORMAT    },
        { "load_extension",     SQLITE_DBCONFIG_ENABLE_بھرو_EXTENSION },
        { "no_ckpt_on_close",   SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE      },
        { "reset_database",     SQLITE_DBCONFIG_RESET_DATABASE        },
        { "trigger_eqp",        SQLITE_DBCONFIG_TRIGGER_EQP           },
        { "trusted_schema",     SQLITE_DBCONFIG_TRUSTED_SCHEMA        },
        { "writable_schema",    SQLITE_DBCONFIG_WRITABLE_SCHEMA       },
    };
    Tcl_Obj *p_نتیجہ;
    صحیح ii;
    اگر( objc>4 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      واپس TCL_ERROR;
    }
    اگر( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      p_نتیجہ = Tcl_NewListObj(0,0);
      برائے(ii=0; ii<حجم(aDbConfig)/حجم(aDbConfig[0]); ii++){
        صحیح v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, p_نتیجہ,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, p_نتیجہ,
           Tcl_NewIntObj(v));
      }
    }ورنہ{
      ثابت حرف *zOpt = Tcl_GetString(objv[2]);
      صحیح onoff = -1;
      صحیح v = 0;
      اگر( zOpt[0]=='-' ) zOpt++;
      برائے(ii=0; ii<حجم(aDbConfig)/حجم(aDbConfig[0]); ii++){
        اگر( strcmp(aDbConfig[ii].zName, zOpt)==0 ) توڑو;
      }
      اگر( ii>=حجم(aDbConfig)/حجم(aDbConfig[0]) ){
        Tcl_Append_نتیجہ(interp, "unknown config option: \"", zOpt,
                                "\"", (خالی*)0);
        واپس TCL_ERROR;
      }
      اگر( objc==4 ){
        اگر( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          واپس TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      p_نتیجہ = Tcl_NewIntObj(v);
    }
    Tcl_سیٹ_Obj_نتیجہ(interp, p_نتیجہ);
    توڑو;
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
    حرف *z_جدول;               /* Insert data into this table */
    حرف *zFile;                /* The file from which to extract data */
    حرف *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    صحیح n_ستون;                   /* Number of columns in the table */
    صحیح nByte;                  /* Number of bytes in an SQL string */
    صحیح i, j;                   /* Loop counters */
    صحیح nSep;                   /* Number of bytes in zSep[] */
    صحیح nNull;                  /* Number of bytes in zNull[] */
    حرف *zSql;                 /* An SQL statement */
    حرف *z_سطر;                /* A single line of input from the file */
    حرف **az_ستون;               /* zLine[] broken up into columns */
    ثابت حرف *zCommit;        /* How to commit changes */
    Tcl_Channel اندر;             /* The input file */
    صحیح lineno = 0;             /* Line number of input file */
    حرف z_سطر_عدد[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *p_نتیجہ;           /* interp result */

    ثابت حرف *zSep;
    ثابت حرف *zNull;
    اگر( objc<5 || objc>7 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      واپس TCL_ERROR;
    }
    اگر( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }ورنہ{
      zSep = "\t";
    }
    اگر( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }ورنہ{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    z_جدول = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    اگر( nSep==0 ){
      Tcl_Append_نتیجہ(interp,"Error: non-null separator required for copy",
                       (حرف*)0);
      واپس TCL_ERROR;
    }
    اگر(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_Append_نتیجہ(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (حرف*)0);
      واپس TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", z_جدول);
    اگر( zSql==0 ){
      Tcl_Append_نتیجہ(interp, "Error: no such table: ", z_جدول, (حرف*)0);
      واپس TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    اگر( rc ){
      Tcl_Append_نتیجہ(interp, "Error: ", sqlite3_errmsg(pDb->db), (حرف*)0);
      n_ستون = 0;
    }ورنہ{
      n_ستون = sqlite3_عمود_گنتی(pStmt);
    }
    sqlite3_finalize(pStmt);
    اگر( n_ستون==0 ) {
      واپس TCL_ERROR;
    }
    zSql = حافظہ_دو( nByte + 50 + n_ستون*2 );
    اگر( zSql==0 ) {
      Tcl_Append_نتیجہ(interp, "Error: can't malloc()", (حرف*)0);
      واپس TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, z_جدول);
    j = strlen30(zSql);
    برائے(i=1; i<n_ستون; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    آزاد(zSql);
    اگر( rc ){
      Tcl_Append_نتیجہ(interp, "Error: ", sqlite3_errmsg(pDb->db), (حرف*)0);
      sqlite3_finalize(pStmt);
      واپس TCL_ERROR;
    }
    اندر = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    اگر( اندر==0 ){
      sqlite3_finalize(pStmt);
      واپس TCL_ERROR;
    }
    Tcl_سیٹ_Channel_Option(تہی, اندر, "-translation", "auto");
    az_ستون = حافظہ_دو( حجم(az_ستون[0])*(n_ستون+1) );
    اگر( az_ستون==0 ) {
      Tcl_Append_نتیجہ(interp, "Error: can't malloc()", (حرف*)0);
      Tcl_Close(interp, اندر);
      واپس TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_گنتی(str);
    (خالی)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    جب( Tcl_GetsObj(اندر, str)>=0 ) {
      حرف *z;
      Tcl_سائز byteLen;
      lineno++;
      z_سطر = (حرف *)Tcl_GetByteArrayFromObj(str, &byteLen);
      az_ستون[0] = z_سطر;
      برائے(i=0, z=z_سطر; *z; z++){
        اگر( *z==zSep[0] && لڑی_موازنہ(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          اگر( i<n_ستون ){
            az_ستون[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      اگر( i+1!=n_ستون ){
        حرف *zErr;
        صحیح nErr = strlen30(zFile) + 200;
        zErr = حافظہ_دو(nErr);
        اگر( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, n_ستون, i+1);
          Tcl_Append_نتیجہ(interp, zErr, (حرف*)0);
          آزاد(zErr);
        }
        zCommit = "ROLLBACK";
        توڑو;
      }
      برائے(i=0; i<n_ستون; i++){
        /* check for null data, if so, bind as null */
        اگر( (nNull>0 && strcmp(az_ستون[i], zNull)==0)
          || strlen30(az_ستون[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }ورنہ{
          sqlite3_bind_متن(pStmt, i+1, az_ستون[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_قدم(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_سیٹ_Obj_لمبائی(str, 0);
      اگر( rc!=SQLITE_OK ){
        Tcl_Append_نتیجہ(interp,"Error: ", sqlite3_errmsg(pDb->db), (حرف*)0);
        zCommit = "ROLLBACK";
        توڑو;
      }
    }
    Tcl_Decr_Ref_گنتی(str);
    آزاد(az_ستون);
    Tcl_Close(interp, اندر);
    sqlite3_finalize(pStmt);
    (خالی)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    اگر( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      p_نتیجہ = Tcl_Get_Obj_نتیجہ(interp);
      اگر( Tcl_IsShared(p_نتیجہ) ){
        p_نتیجہ = Tcl_DuplicateObj(p_نتیجہ);
      }
      Tcl_سیٹ_Int_Obj(p_نتیجہ, lineno);
      rc = TCL_OK;
    }ورنہ{
      /* failure, append lineno where failed */
      sqlite3_snprintf(حجم(z_سطر_عدد), z_سطر_عدد,"%d",lineno);
      Tcl_Append_نتیجہ(interp,", failed while processing line: ",z_سطر_عدد,
                       (حرف*)0);
      rc = TCL_ERROR;
    }
    توڑو;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  حالت DB_DESERIALIZE: {
#اگر_تعریف_شدہ SQLITE_OMIT_DESERIALIZE
    Tcl_Append_نتیجہ(interp, "MEMDB not available in this build",
                     (حرف*)0);
    rc = TCL_ERROR;
#ورنہ
    ثابت حرف *zSchema = 0;
    Tcl_Obj *p_قدر = 0;
    غیرعلامتی حرف *pBA;
    غیرعلامتی حرف *p_ڈیٹا;
    Tcl_سائز len;
    صحیح xrc;
    sqlite3_int64 mx_سائز = 0;
    صحیح i;
    صحیح isReadonly = 0;


    اگر( objc<3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      توڑو;
    }
    برائے(i=2; i<objc-1; i++){
      ثابت حرف *z = Tcl_GetString(objv[i]);
      اگر( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        اگر( rc ) جاؤ deserialize_error;
        mx_سائز = x;
        جاری;
      }
      اگر( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        اگر( rc ) جاؤ deserialize_error;
        جاری;
      }
      اگر( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        جاری;
      }
      Tcl_Append_نتیجہ(interp, "unknown option: ", z, (حرف*)0);
      rc = TCL_ERROR;
      جاؤ deserialize_error;
    }
    p_قدر = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_قدر, &len);
    p_ڈیٹا = sqlite3_malloc64( len );
    اگر( p_ڈیٹا==0 && len>0 ){
      Tcl_Append_نتیجہ(interp, "out of memory", (حرف*)0);
      rc = TCL_ERROR;
    }ورنہ{
      صحیح flags;
      اگر( len>0 ) حافظہ_نقل(p_ڈیٹا, pBA, len);
      اگر( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }ورنہ{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, p_ڈیٹا, len, len, flags);
      اگر( xrc ){
        Tcl_Append_نتیجہ(interp, "unable to set MEMDB content", (حرف*)0);
        rc = TCL_ERROR;
      }
      اگر( mx_سائز>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_سائز_حد_بندی,&mx_سائز);
      }
    }
deserialize_error:
#اگر_ختم
    توڑو; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  حالت DB_ENABLE_بھرو_EXTENSION: {
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
    صحیح onoff;
    اگر( objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "BOOLEAN");
      واپس TCL_ERROR;
    }
    اگر( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      واپس TCL_ERROR;
    }
    sqlite3_enable_بھرو_extension(pDb->db, onoff);
    توڑو;
#ورنہ
    Tcl_Append_نتیجہ(interp, "extension loading is turned off at compile-time",
                     (حرف*)0);
    واپس TCL_ERROR;
#اگر_ختم
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  حالت DB_ERRORCODE: {
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    توڑو;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  حالت DB_ERROROFFSET: {
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(sqlite3_error_انحراف(pDb->db)));
    توڑو;
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
    Tcl_Obj *p_نتیجہ = 0;
    DbEvalContext sEval;
    اگر( objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "SQL");
      واپس TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = db_Eval_قدم(&sEval);
    اگر( choice==DB_ONECOLUMN ){
      اگر( rc==TCL_OK ){
        p_نتیجہ = db_Eval_عمود_قدر(&sEval, 0);
      }ورنہ اگر( rc==TCL_BREAK ){
        Tcl_Reset_نتیجہ(interp);
      }
    }ورنہ اگر( rc==TCL_BREAK || rc==TCL_OK ){
      p_نتیجہ = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    اگر( p_نتیجہ ) Tcl_سیٹ_Obj_نتیجہ(interp, p_نتیجہ);

    اگر( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    توڑو;
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
    ثابت حرف *zOpt;
    جب( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      اگر( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }ورنہ اگر( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }ورنہ{
        Tcl_Append_نتیجہ(interp, "unknown option: \"", zOpt, "\"", (خالی*)0);
        واپس TCL_ERROR;
      }
      objc--;
      objv++;
    }
    اگر( objc<3 || objc>5 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      واپس TCL_ERROR;
    }

    اگر( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_گنتی(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      جب( TCL_OK==(rc = db_Eval_قدم(&sEval)) ){
        صحیح i;
        صحیح n_ستون;
        db_Eval_صف_Info(&sEval, &n_ستون, 0);
        برائے(i=0; i<n_ستون; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_عمود_قدر(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      اگر( rc==TCL_BREAK ){
        Tcl_سیٹ_Obj_نتیجہ(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_گنتی(pRet);
    }ورنہ{
      Client_ڈیٹا cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      اگر( objc>=5 && *(حرف *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_گنتی(pScript);

      p = (DbEvalContext *)Tcl_Alloc(حجم(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (خالی *)p;
      cd2[1] = (خالی *)pScript;
      rc = Db_Eval_اگلا_Cmd(cd2, interp, TCL_OK);
    }
    توڑو;
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
    توڑو;
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
    حرف *zName;
    صحیح nArg = -1;
    صحیح i;
    صحیح eType = SQLITE_NULL;
    اگر( objc<4 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      واپس TCL_ERROR;
    }
    برائے(i=3; i<(objc-1); i++){
      ثابت حرف *z = Tcl_GetString(objv[i]);
      صحیح n = strlen30(z);
      اگر( n>1 && لڑی_موازنہ(z, "-argcount",n)==0 ){
        اگر( i==(objc-2) ){
          Tcl_Append_نتیجہ(interp, "option requires an argument: ", z,(حرف*)0);
          واپس TCL_ERROR;
        }
        اگر( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) واپس TCL_ERROR;
        اگر( nArg<0 ){
          Tcl_Append_نتیجہ(interp, "number of arguments must be non-negative",
                           (حرف*)0);
          واپس TCL_ERROR;
        }
        i++;
      }ورنہ
      اگر( n>1 && لڑی_موازنہ(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }ورنہ
      اگر( n>1 && لڑی_موازنہ(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }ورنہ
      اگر( n>1 && لڑی_موازنہ(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }ورنہ
      اگر( n>1 && لڑی_موازنہ(z, "-returntype", n)==0 ){
        ثابت حرف *azType[] = {"integer", "real", "text", "blob", "any", 0};
        أكد( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_متن==3 );
        أكد( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        اگر( i==(objc-2) ){
          Tcl_Append_نتیجہ(interp, "option requires an argument: ", z,(حرف*)0);
          واپس TCL_ERROR;
        }
        i++;
        اگر( Tcl_Get_اشاریہ_From_Obj(interp, objv[i], azType, "type", 0, &eType) ){
          واپس TCL_ERROR;
        }
        eType++;
      }ورنہ{
        Tcl_Append_نتیجہ(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (حرف*)0
        );
        واپس TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = ڈھونڈو_Sql_Func(pDb, zName);
    اگر( pFunc==0 ) واپس TCL_ERROR;
    اگر( pFunc->pScript ){
      Tcl_Decr_Ref_گنتی(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_گنتی(pScript);
    pFunc->useEvalObjv = محفوظ_مقصد_Use_Eval_Objv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    اگر( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    توڑو;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  حالت DB_INCRBLOB: {
#اگر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
    Tcl_Append_نتیجہ(interp, "incrblob not available in this build", (حرف*)0);
    واپس TCL_ERROR;
#ورنہ
    صحیح isReadonly = 0;
    ثابت حرف *zDb = "main";
    ثابت حرف *z_جدول;
    ثابت حرف *z_عمود;
    Tcl_WideInt i_صف;

    /* Check for the -readonly option */
    اگر( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    اگر( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      واپس TCL_ERROR;
    }

    اگر( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    z_جدول = Tcl_GetString(objv[objc-3]);
    z_عمود = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &i_صف);

    اگر( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, z_جدول, z_عمود, (sqlite3_int64)i_صف, isReadonly
      );
    }
#اگر_ختم
    توڑو;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  حالت DB_مداخلت: {
    sqlite3_مداخلت(pDb->db);
    توڑو;
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
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "NULLVALUE");
      واپس TCL_ERROR;
    }
    اگر( objc==3 ){
      Tcl_سائز len;
      حرف *zNull = Tcl_GetStringFromObj(objv[2], &len);
      اگر( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      اگر( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }ورنہ{
        pDb->zNull = 0;
      }
    }
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(pDb->zNull, -1));
    توڑو;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  حالت DB_آخری_داخل_کرو_ROWID: {
    Tcl_Obj *p_نتیجہ;
    Tcl_WideInt rowid;
    اگر( objc!=2 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
      واپس TCL_ERROR;
    }
    rowid = sqlite3_آخری_داخل_کرو_rowid(pDb->db);
    p_نتیجہ = Tcl_Get_Obj_نتیجہ(interp);
    Tcl_سیٹ_Wide_Int_Obj(p_نتیجہ, rowid);
    توڑو;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  حالت DB_پیش_رفت: {
    اگر( objc==2 ){
      اگر( pDb->z_پیش_رفت ){
        Tcl_Append_نتیجہ(interp, pDb->z_پیش_رفت, (حرف*)0);
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_پیش_رفت_CALLBACK
      sqlite3_پیش_رفت_handler(pDb->db, 0, 0, 0);
#اگر_ختم
    }ورنہ اگر( objc==4 ){
      حرف *z_پیش_رفت;
      Tcl_سائز len;
      صحیح N;
      اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        واپس TCL_ERROR;
      };
      اگر( pDb->z_پیش_رفت ){
        Tcl_Free(pDb->z_پیش_رفت);
      }
      z_پیش_رفت = Tcl_GetStringFromObj(objv[3], &len);
      اگر( z_پیش_رفت && len>0 ){
        pDb->z_پیش_رفت = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->z_پیش_رفت, z_پیش_رفت, len+1);
      }ورنہ{
        pDb->z_پیش_رفت = 0;
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_پیش_رفت_CALLBACK
      اگر( pDb->z_پیش_رفت ){
        pDb->interp = interp;
        sqlite3_پیش_رفت_handler(pDb->db, N, Db_پیش_رفت_Handler, pDb);
      }ورنہ{
        sqlite3_پیش_رفت_handler(pDb->db, 0, 0, 0);
      }
#اگر_ختم
    }ورنہ{
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "N CALLBACK");
      واپس TCL_ERROR;
    }
    توڑو;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  حالت DB_PROFILE: {
    اگر( objc>3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?CALLBACK?");
      واپس TCL_ERROR;
    }ورنہ اگر( objc==2 ){
      اگر( pDb->zProfile ){
        Tcl_Append_نتیجہ(interp, pDb->zProfile, (حرف*)0);
      }
    }ورنہ{
      حرف *zProfile;
      Tcl_سائز len;
      اگر( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->zProfile, zProfile, len+1);
      }ورنہ{
        pDb->zProfile = 0;
      }
#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_نقطہ) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      اگر( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }ورنہ{
        sqlite3_profile(pDb->db, 0, 0);
      }
#اگر_ختم
    }
    توڑو;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  حالت DB_REKEY: {
    اگر( objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "KEY");
      واپس TCL_ERROR;
    }
    توڑو;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  حالت DB_RESTORE: {
    ثابت حرف *zSrcFile;
    ثابت حرف *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    صحیح nTimeout = 0;

    اگر( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }ورنہ اگر( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }ورنہ{
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?DATABASE? FILENAME");
      واپس TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    اگر( rc!=SQLITE_OK ){
      Tcl_Append_نتیجہ(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (حرف*)0);
      sqlite3_close(pSrc);
      واپس TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    اگر( pBackup==0 ){
      Tcl_Append_نتیجہ(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (حرف*)0);
      sqlite3_close(pSrc);
      واپس TCL_ERROR;
    }
    جب( (rc = sqlite3_backup_قدم(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      اگر( rc==SQLITE_BUSY ){
        اگر( nTimeout++ >= 3 ) توڑو;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_اختتام(pBackup);
    اگر( rc==SQLITE_مکمل ){
      rc = TCL_OK;
    }ورنہ اگر( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_Append_نتیجہ(interp, "restore failed: source database busy",
                       (حرف*)0);
      rc = TCL_ERROR;
    }ورنہ{
      Tcl_Append_نتیجہ(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (حرف*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    توڑو;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  حالت DB_SERIALIZE: {
#اگر_تعریف_شدہ SQLITE_OMIT_DESERIALIZE
    Tcl_Append_نتیجہ(interp, "MEMDB not available in this build",
                     (حرف*)0);
    rc = TCL_ERROR;
#ورنہ
    ثابت حرف *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    غیرعلامتی حرف *p_ڈیٹا;
    اگر( objc!=2 && objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }ورنہ{
      صحیح needFree;
      p_ڈیٹا = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      اگر( p_ڈیٹا ){
        needFree = 0;
      }ورنہ{
        p_ڈیٹا = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewByteArrayObj(p_ڈیٹا,sz));
      اگر( needFree ) sqlite3_free(p_ڈیٹا);
    }
#اگر_ختم
    توڑو;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  حالت DB_حالت: {
    صحیح v;
    ثابت حرف *zOp;
    اگر( objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "(step|sort|autoindex)");
      واپس TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    اگر( strcmp(zOp, "step")==0 ){
      v = pDb->n_قدم;
    }ورنہ اگر( strcmp(zOp, "sort")==0 ){
      v = pDb->n_ترتیب_دو;
    }ورنہ اگر( strcmp(zOp, "autoindex")==0 ){
      v = pDb->n_اشاریہ;
    }ورنہ اگر( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }ورنہ{
      Tcl_Append_نتیجہ(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (حرف*)0);
      واپس TCL_ERROR;
    }
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(v));
    توڑو;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  حالت DB_TIMEOUT: {
    صحیح ms;
    اگر( objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "MILLISECONDS");
      واپس TCL_ERROR;
    }
    اگر( Tcl_GetIntFromObj(interp, objv[2], &ms) ) واپس TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    توڑو;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  حالت DB_کل_CHANGES: {
    Tcl_Obj *p_نتیجہ;
    اگر( objc!=2 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
      واپس TCL_ERROR;
    }
    p_نتیجہ = Tcl_Get_Obj_نتیجہ(interp);
    Tcl_سیٹ_Wide_Int_Obj(p_نتیجہ, sqlite3_کل_changes64(pDb->db));
    توڑو;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  حالت DB_TRACE: {
    اگر( objc>3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?CALLBACK?");
      واپس TCL_ERROR;
    }ورنہ اگر( objc==2 ){
      اگر( pDb->zTrace ){
        Tcl_Append_نتیجہ(interp, pDb->zTrace, (حرف*)0);
      }
    }ورنہ{
      حرف *zTrace;
      Tcl_سائز len;
      اگر( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->zTrace, zTrace, len+1);
      }ورنہ{
        pDb->zTrace = 0;
      }
#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_نقطہ) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      اگر( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }ورنہ{
        sqlite3_trace(pDb->db, 0, 0);
      }
#اگر_ختم
    }
    توڑو;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  حالت DB_TRACE_V2: {
    اگر( objc>4 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?CALLBACK? ?MASK?");
      واپس TCL_ERROR;
    }ورنہ اگر( objc==2 ){
      اگر( pDb->zTraceV2 ){
        Tcl_Append_نتیجہ(interp, pDb->zTraceV2, (حرف*)0);
      }
    }ورنہ{
      حرف *zTraceV2;
      Tcl_سائز len;
      Tcl_WideInt w_نقاب = 0;
      اگر( objc==4 ){
        ساکن ثابت حرف *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        شمار TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_صف, TTYPE_CLOSE
        };
        Tcl_سائز i;
        اگر( TCL_OK!=Tcl_List_Obj_لمبائی(interp, objv[3], &len) ){
          واپس TCL_ERROR;
        }
        برائے(i=0; i<len; i++){
          Tcl_Obj *pObj;
          صحیح ttype;
          اگر( TCL_OK!=Tcl_List_Obj_اشاریہ(interp, objv[3], i, &pObj) ){
            واپس TCL_ERROR;
          }
          اگر( Tcl_Get_اشاریہ_From_Obj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_Get_Obj_نتیجہ(interp));
            Tcl_Incr_Ref_گنتی(pError);
            اگر( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_گنتی(pError);
              w_نقاب |= wType;
            }ورنہ{
              Tcl_سیٹ_Obj_نتیجہ(interp, pError);
              Tcl_Decr_Ref_گنتی(pError);
              واپس TCL_ERROR;
            }
          }ورنہ{
            انتخاب( (شمار TTYPE_enum)ttype ){
              حالت TTYPE_STMT:    w_نقاب |= SQLITE_TRACE_STMT;    توڑو;
              حالت TTYPE_PROFILE: w_نقاب |= SQLITE_TRACE_PROFILE; توڑو;
              حالت TTYPE_صف:     w_نقاب |= SQLITE_TRACE_صف;     توڑو;
              حالت TTYPE_CLOSE:   w_نقاب |= SQLITE_TRACE_CLOSE;   توڑو;
            }
          }
        }
      }ورنہ{
        w_نقاب = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      اگر( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      اگر( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        حافظہ_نقل(pDb->zTraceV2, zTraceV2, len+1);
      }ورنہ{
        pDb->zTraceV2 = 0;
      }
#اگر !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_نقطہ)
      اگر( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (غیرعلامتی)w_نقاب, DbTraceV2Handler, pDb);
      }ورنہ{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#اگر_ختم
    }
    توڑو;
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
    ثابت حرف *zBegin = "SAVEPOINT _tcl_transaction";
    اگر( objc!=3 && objc!=4 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "[TYPE] SCRIPT");
      واپس TCL_ERROR;
    }

    اگر( pDb->nTransaction==0 && objc==4 ){
      ساکن ثابت حرف *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      شمار TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_فوری
      };
      صحیح ttype;
      اگر( Tcl_Get_اشاریہ_From_Obj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        واپس TCL_ERROR;
      }
      انتخاب( (شمار TTYPE_enum)ttype ){
        حالت TTYPE_DEFERRED:    /* no-op */;                 توڑو;
        حالت TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  توڑو;
        حالت TTYPE_فوری:   zBegin = "BEGIN IMMEDIATE";  توڑو;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    اگر( rc!=SQLITE_OK ){
      Tcl_Append_نتیجہ(interp, sqlite3_errmsg(pDb->db), (حرف*)0);
      واپس TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    جوڑو_Database_Ref(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    اگر( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (خالی)Tcl_NREvalObj(interp, pScript, 0);
    }ورنہ{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    توڑو;
  }

  /*
  **    $db unlock_notify ?script?
  */
  حالت DB_UNLOCK_NOTIFY: {
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_Append_نتیجہ(interp, "unlock_notify not available in this build",
                     (حرف*)0);
    rc = TCL_ERROR;
#ورنہ
    اگر( objc!=2 && objc!=3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }ورنہ{
      خالی (*xNotify)(خالی **, صحیح) = 0;
      خالی *pNotifyArg = 0;

      اگر( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_گنتی(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      اگر( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (خالی *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_گنتی(pDb->pUnlockNotify);
      }

      اگر( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_Append_نتیجہ(interp, sqlite3_errmsg(pDb->db), (حرف*)0);
        rc = TCL_ERROR;
      }
    }
#اگر_ختم
    توڑو;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  حالت DB_PREUPDATE: {
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_Append_نتیجہ(interp, "preupdate_hook was omitted at compile-time", 
                     (حرف*)0);
    rc = TCL_ERROR;
#ورنہ
    ساکن ثابت حرف *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    شمار DbPreupdateSubCmd {
      PRE_گنتی, PRE_گہرائی, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    صحیح iSub;

    اگر( objc<3 ){
      Tcl_Wrong_عدد_دلائل(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    اگر( Tcl_Get_اشاریہ_From_Obj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      واپس TCL_ERROR;
    }

    انتخاب( (شمار DbPreupdateSubCmd)iSub ){
      حالت PRE_گنتی: {
        صحیح n_ستون = sqlite3_preupdate_گنتی(pDb->db);
        Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(n_ستون));
        توڑو;
      }

      حالت PRE_HOOK: {
        اگر( objc>4 ){
          Tcl_Wrong_عدد_دلائل(interp, 2, objv, "hook ?SCRIPT?");
          واپس TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        توڑو;
      }

      حالت PRE_گہرائی: {
        Tcl_Obj *pRet;
        اگر( objc!=3 ){
          Tcl_Wrong_عدد_دلائل(interp, 3, objv, "");
          واپس TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_گہرائی(pDb->db));
        Tcl_سیٹ_Obj_نتیجہ(interp, pRet);
        توڑو;
      }

      حالت PRE_NEW:
      حالت PRE_OLD: {
        صحیح iIdx;
        sqlite3_قدر *p_قدر;
        اگر( objc!=4 ){
          Tcl_Wrong_عدد_دلائل(interp, 3, objv, "INDEX");
          واپس TCL_ERROR;
        }
        اگر( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          واپس TCL_ERROR;
        }

        اگر( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_قدر);
        }ورنہ{
          أكد( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_قدر);
        }

        اگر( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((حرف*)sqlite3_قدر_متن(p_قدر), -1);
          Tcl_سیٹ_Obj_نتیجہ(interp, pObj);
        }ورنہ{
          Tcl_Append_نتیجہ(interp, sqlite3_errmsg(pDb->db), (حرف*)0);
          واپس TCL_ERROR;
        }
      }
    }
#اگر_ختم /* SQLITE_ENABLE_PREUPDATE_HOOK */
    توڑو;
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
       Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?SCRIPT?");
       واپس TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    توڑو;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  حالت DB_نسخہ: {
    صحیح i;
    برائے(i=2; i<objc; i++){
      ثابت حرف *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#اگر_تعریف_شدہ SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      اگر( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        اگر( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          واپس TCL_ERROR;
        }
      }ورنہ

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      اگر( strcmp(zArg, "-last-stmt-ptr")==0 ){
        خارجی ثابت حرف *sqlite3_Test_Ptr_مقصد_متن(خالی*);
        خالی *ptr = (خالی*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_سیٹ_نتیجہ(interp, (حرف*)sqlite3_Test_Ptr_مقصد_متن(ptr), TCL_VOLATILE);
      }ورنہ
#اگر_ختم /* SQLITE_TEST */
      {
        Tcl_Append_نتیجہ(interp, "unknown argument: ", zArg, (حرف*)0);
        واپس TCL_ERROR;
      }
    }
    اگر( i==2 ){   
      Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3_libversion(), TCL_STATIC);
    }
    توڑو;
  }


  } /* End of the SWITCH statement */
  واپس rc;
}

#اگر SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
ساکن صحیح SQLITE_TCLAPI DbObjCmdAdaptor(
  خالی *cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  واپس Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#اگر_ختم /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
ساکن صحیح sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *ثابت*objv
){
  Tcl_Wrong_عدد_دلائل(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  واپس TCL_ERROR;
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
ساکن صحیح SQLITE_TCLAPI DbMain(
  خالی *cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  SqliteDb *p;
  ثابت حرف *zArg;
  حرف *z_Err_پیغام;
  صحیح i;
  ثابت حرف *zFile = 0;
  ثابت حرف *zVfs = 0;
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
#اگر_تعریف_شدہ SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#ورنہ
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#اگر_ختم

  اگر( objc==1 ) واپس sqliteCmdUsage(interp, objv);
  اگر( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    اگر( strcmp(zArg,"-version")==0 ){
      Tcl_Append_نتیجہ(interp,sqlite3_libversion(), (حرف*)0);
      واپس TCL_OK;
    }
    اگر( strcmp(zArg,"-sourceid")==0 ){
      Tcl_Append_نتیجہ(interp,sqlite3_sourceid(), (حرف*)0);
      واپس TCL_OK;
    }
    اگر( strcmp(zArg,"-has-codec")==0 ){
      Tcl_Append_نتیجہ(interp,"0",(حرف*)0);
      واپس TCL_OK;
    }
    اگر( zArg[0]=='-' ) واپس sqliteCmdUsage(interp, objv);
  }
  برائے(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    اگر( zArg[0]!='-' ){
      اگر( zFile!=0 ) واپس sqliteCmdUsage(interp, objv);
      zFile = zArg;
      جاری;
    }
    اگر( i==objc-1 ) واپس sqliteCmdUsage(interp, objv);
    i++;
    اگر( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }ورنہ اگر( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }ورنہ اگر( strcmp(zArg, "-readonly")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) واپس TCL_ERROR;
      اگر( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }ورنہ{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }ورنہ اگر( strcmp(zArg, "-create")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) واپس TCL_ERROR;
      اگر( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }ورنہ{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }ورنہ اگر( strcmp(zArg, "-nofollow")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) واپس TCL_ERROR;
      اگر( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }ورنہ{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }ورنہ اگر( strcmp(zArg, "-nomutex")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) واپس TCL_ERROR;
      اگر( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }ورنہ{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }ورنہ اگر( strcmp(zArg, "-fullmutex")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) واپس TCL_ERROR;
      اگر( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }ورنہ{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }ورنہ اگر( strcmp(zArg, "-uri")==0 ){
      صحیح b;
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) واپس TCL_ERROR;
      اگر( b ){
        flags |= SQLITE_OPEN_URI;
      }ورنہ{
        flags &= ~SQLITE_OPEN_URI;
      }
    }ورنہ اگر( strcmp(zArg, "-translatefilename")==0 ){
      اگر( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        واپس TCL_ERROR;
      }
    }ورنہ{
      Tcl_Append_نتیجہ(interp, "unknown option: ", zArg, (حرف*)0);
      واپس TCL_ERROR;
    }
  }
  z_Err_پیغام = 0;
  p = (SqliteDb*)Tcl_Alloc( حجم(*p) );
  memset(p, 0, حجم(*p));
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
      z_Err_پیغام = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }ورنہ{
    z_Err_پیغام = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  اگر( p->db==0 ){
    Tcl_سیٹ_نتیجہ(interp, z_Err_پیغام, TCL_VOLATILE);
    Tcl_Free((حرف*)p);
    sqlite3_free(z_Err_پیغام);
    واپس TCL_ERROR;
  }
  p->maxStmt = عدد_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  اگر( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (حرف*)p, DbDeleteCmd);
  }ورنہ{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (حرف*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  واپس TCL_OK;
}

/*
** Provide a dummy Tcl_InitStubs if we are using this as a static
** library.
*/
#اگر_غیر_تعریف_شدہ USE_TCL_STUBS
# undef  Tcl_InitStubs
# define Tcl_InitStubs(a,b,c) TCL_نسخہ
#اگر_ختم

/*
** Make sure we have a PACKAGE_VERSION macro defined.  This will be
** defined automatically by the TEA makefile.  But other makefiles
** do not define it.
*/
#اگر_غیر_تعریف_شدہ PACKAGE_نسخہ
# define PACKAGE_نسخہ SQLITE_نسخہ
#اگر_ختم

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
#اگر_غیر_تعریف_شدہ SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#اگر_ختم
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_نسخہ);
  }
  واپس rc;
}
EXTERN صحیح Tclsqlite3_Init(Tcl_Interp *interp){ واپس Sqlite3_Init(interp); }
EXTERN صحیح Sqlite3_Unload(Tcl_Interp *interp, صحیح flags){ واپس TCL_OK; }
EXTERN صحیح Tclsqlite3_Unload(Tcl_Interp *interp, صحیح flags){ واپس TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN صحیح Sqlite3_محفوظ_Init(Tcl_Interp *interp){ واپس TCL_ERROR; }
EXTERN صحیح Sqlite3_محفوظ_Unload(Tcl_Interp *interp, صحیح flags){واپس TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN صحیح Sqlite_Init(Tcl_Interp *interp){ واپس Sqlite3_Init(interp);}
EXTERN صحیح Tclsqlite_Init(Tcl_Interp *interp){ واپس Sqlite3_Init(interp); }
EXTERN صحیح Sqlite_Unload(Tcl_Interp *interp, صحیح flags){ واپس TCL_OK; }
EXTERN صحیح Tclsqlite_Unload(Tcl_Interp *interp, صحیح flags){ واپس TCL_OK; }
EXTERN صحیح Sqlite_محفوظ_Init(Tcl_Interp *interp){ واپس TCL_ERROR; }
EXTERN صحیح Sqlite_محفوظ_Unload(Tcl_Interp *interp, صحیح flags){واپس TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN صحیح sqlite3_Init(Tcl_Interp *interp){ واپس Sqlite3_Init(interp);}
EXTERN صحیح sqlite_Init(Tcl_Interp *interp){ واپس Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#اگر defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
ساکن ثابت حرف *tclsh_main_loop(خالی){
  ساکن ثابت حرف zMainloop[] =
    "if {[llength $argv]>=1} {\n"
#اگر_تعریف_شدہ WIN32
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
#اگر_ختم
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
  واپس zMainloop;
}

#اگر_غیر_تعریف_شدہ TCLSH_MAIN
# define TCLSH_MAIN مین
#اگر_ختم
صحیح SQLITE_CDECL TCLSH_MAIN(صحیح argc, حرف **argv){
  Tcl_Interp *interp;
  صحیح i;
  ثابت حرف *zScript = 0;
  حرف zArgc[32];
#اگر defined(TCLSH_INIT_PROC)
  خارجی ثابت حرف *TCLSH_INIT_PROC(Tcl_Interp*);
#اگر_ختم

#اگر !defined(_WIN32_WCE)
  اگر( getenv("SQLITE_DEBUG_BREAK") ){
    اگر( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }ورنہ{
#اگر defined(_WIN32) || defined(WIN32)
      DebugBreak();
#وگرنہ_اگر defined(SIGTRAP)
      اٹھاؤ(SIGTRAP);
#اگر_ختم
    }
  }
#اگر_ختم

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_ڈھونڈو_Executable(argv[0]);
  Tcl_سیٹ_System_Encoding(تہی, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(حجم(zArgc), zArgc, "%d", argc-1);
  Tcl_سیٹ_Var(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_سیٹ_Var(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_سیٹ_Var(interp,"argv", "", TCL_GLOBAL_ONLY);
  برائے(i=1; i<argc; i++){
    Tcl_سیٹ_Var(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_قدر);
  }
#اگر defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#اگر_ختم
  اگر( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  اگر( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    ثابت حرف *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    اگر( zInfo==0 ) zInfo = Tcl_Get_String_نتیجہ(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    واپس 1;
  }
  واپس 0;
}
#اگر_ختم /* TCLSH */
