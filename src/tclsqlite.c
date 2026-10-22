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
#अगर_घोषित TCLSH_INIT_PROC
# define TCLSH 1
#पूर्ण_अगर

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#अगर defined(INCLUDE_MSVC_H)
# include "msvc.h"
#पूर्ण_अगर

/****** Copy of tclsqlite.h ******/
#अगर defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#अन्यथा
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#पूर्ण_अगर
/* Compatability between Tcl8.6 and Tcl9.0 */
#अगर TCL_MAJOR_VERSION==9
# define CONST स्थिर
#या_अगर !defined(Tcl_आकार)
  प्रकार पूर्णांक Tcl_आकार;
# ifndef Tcl_Bounce_Ref_गिनती
#  define Tcl_Bounce_Ref_गिनती(X) Tcl_Incr_Ref_गिनती(X); Tcl_Decr_Ref_गिनती(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#पूर्ण_अगर
/**** End copy of tclsqlite.h ****/

#समावेश <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#अगर_अघोषित SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  प्रकार अचिन्हित अक्षर u8;
# ifndef SQLITE_PTRSIZE
#   अगर defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   अन्यथा
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# अगर defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    प्रकार uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    प्रकार अचिन्हित पूर्णांक uptr;
# अन्यथा
    प्रकार sqlite3_uint64 uptr;
# endif
#पूर्ण_अगर
#समावेश <ctype.h>

/* Used to get the current process ID */
#अगर !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#या_अगर !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (पूर्णांक)GetCurrentProcessId
#पूर्ण_अगर

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#अगर_घोषित BUILD_sqlite
#अघोषित TCL_STORAGE_CLASS
#घोषणा TCL_STORAGE_CLASS DLLEXPORT
#पूर्ण_अगर /* BUILD_sqlite */

#घोषणा अंक_PREPARED_STMTS 10
#घोषणा MAX_PREPARED_STMTS 100

/* Forward declaration */
प्रकार काष्ठा SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#अगर defined(SQLITE_ENABLE_QRF_IN_TCL)
#समावेश "qrf.h"
#पूर्ण_अगर

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
प्रकार काष्ठा SqlFunc SqlFunc;
काष्ठा SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  पूर्णांक useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  पूर्णांक eType;            /* Type of value to return */
  अक्षर *zName;          /* Name of this function */
  SqlFunc *p_अग्र;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
प्रकार काष्ठा SqlCollate SqlCollate;
काष्ठा SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  अक्षर *zScript;        /* The script to be run */
  SqlCollate *p_अग्र;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
प्रकार काष्ठा SqlPreparedStmt SqlPreparedStmt;
काष्ठा SqlPreparedStmt {
  SqlPreparedStmt *p_अग्र;  /* Next in linked list */
  SqlPreparedStmt *p_पूर्व;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  पूर्णांक nSql;                /* chars in zSql[] */
  स्थिर अक्षर *zSql;        /* Text of the SQL statement */
  पूर्णांक nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

प्रकार काष्ठा IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
काष्ठा SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  अक्षर *zBusy;               /* The busy callback routine */
  अक्षर *zCommit;             /* The commit hook callback routine */
  अक्षर *zTrace;              /* The trace callback routine */
  अक्षर *zTraceV2;            /* The trace_v2 callback routine */
  अक्षर *zProfile;            /* The profile callback routine */
  अक्षर *zProgress;           /* The progress callback routine */
  अक्षर *zBindFallback;       /* Callback to invoke on a binding miss */
  अक्षर *zAuth;               /* The authorization callback routine */
  पूर्णांक disableAuth;           /* Disable the authorizer if it exists */
  अक्षर *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  पूर्णांक rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmtLast; /* Last statement in the list */
  पूर्णांक maxStmt;               /* The next maximum number of stmtList */
  पूर्णांक nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  पूर्णांक nStep, nSort, n_सूचकांक;  /* Statistics for most recent operation */
  पूर्णांक nVMStep;               /* Another statistic for most recent operation */
  पूर्णांक nTransaction;          /* Number of nested [transaction] methods */
  पूर्णांक openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  पूर्णांक nRef;                  /* Delete object when this reaches 0 */
#अगर_घोषित SQLITE_TEST
  पूर्णांक bLegacyPrepare;        /* True to use sqlite3_prepare() */
#पूर्ण_अगर
};

काष्ठा IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  अचिन्हित पूर्णांक isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *p_अग्र;   /* Linked list of all open incrblob channels */
  IncrblobChannel *p_पूर्व;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
अटल पूर्णांक strlen30(स्थिर अक्षर *z){
  स्थिर अक्षर *z2 = z;
  जबतक( *z2 ){ z2++; }
  वापस 0x3fffffff & (पूर्णांक)(z2 - z);
}


#अगर_अघोषित SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
अटल व्योम closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *p_अग्र;

  क्रम(p=pDb->pIncrblob; p; p=p_अग्र){
    p_अग्र = p->p_अग्र;

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
अटल पूर्णांक SQLITE_TCLAPI incrblobClose2(
  Client_तथ्य instance_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक flags
){
  IncrblobChannel *p = (IncrblobChannel *)instance_तथ्य;
  पूर्णांक  rc;
  sqlite3 *db = p->pDb->db;

  अगर( flags ){
    p->isClosed |= flags;
    वापस TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  अगर( p->p_अग्र ){
    p->p_अग्र->p_पूर्व = p->p_पूर्व;
  }
  अगर( p->p_पूर्व ){
    p->p_पूर्व->p_अग्र = p->p_अग्र;
  }
  अगर( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->p_अग्र;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((अक्षर *)p);

  अगर( rc!=SQLITE_OK ){
    Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3_errmsg(db), TCL_VOLATILE);
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}
अटल पूर्णांक SQLITE_TCLAPI incrblobClose(
  Client_तथ्य instance_तथ्य,
  Tcl_Interp *interp
){
  वापस incrblobClose2(instance_तथ्य, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
अटल पूर्णांक SQLITE_TCLAPI incrblob_निवेश(
  Client_तथ्य instance_तथ्य,
  अक्षर *buf,
  पूर्णांक buf_आकार,
  पूर्णांक *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_तथ्य;
  sqlite3_int64 nRead = buf_आकार;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  पूर्णांक rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  अगर( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  अगर( nRead<=0 ){
    वापस 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (व्योम *)buf, (पूर्णांक)nRead, (पूर्णांक)p->iSeek);
  अगर( rc!=SQLITE_OK ){
    *errorCodePtr = rc;
    वापस -1;
  }

  p->iSeek += nRead;
  वापस nRead;
}

/*
** Write data to an incremental blob channel.
*/
अटल पूर्णांक SQLITE_TCLAPI incrblobOutput(
  Client_तथ्य instance_तथ्य,
  स्थिर अक्षर *buf,
  पूर्णांक toWrite,
  पूर्णांक *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_तथ्य;
  sqlite3_int64 nWrite = toWrite;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  पूर्णांक rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_bytes(p->pBlob);
  अगर( (p->iSeek+nWrite)>nBlob ){
    *errorCodePtr = EINVAL;
    वापस -1;
  }
  अगर( nWrite<=0 ){
    वापस 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (व्योम*)buf,(पूर्णांक)nWrite, (पूर्णांक)p->iSeek);
  अगर( rc!=SQLITE_OK ){
    *errorCodePtr = EIO;
    वापस -1;
  }

  p->iSeek += nWrite;
  वापस nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#अगर TCL_MAJOR_VERSION==9
# define WideSeekProcType दीर्घ दीर्घ
#अन्यथा
# define WideSeekProcType Tcl_WideInt
#पूर्ण_अगर

/*
** Seek an incremental blob channel.
*/
अटल WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  Client_तथ्य instance_तथ्य,
  WideSeekProcType offset,
  पूर्णांक seekMode,
  पूर्णांक *errorCodePtr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_तथ्य;

  चयन( seekMode ){
    हाल शुरू_से:
      p->iSeek = offset;
      अवरोध;
    हाल प्रस्तुत_से:
      p->iSeek += offset;
      अवरोध;
    हाल अंत_से:
      p->iSeek = sqlite3_blob_bytes(p->pBlob) + offset;
      अवरोध;

    शेष: निश्चित(!"Bad seekMode");
  }

  वापस p->iSeek;
}
अटल पूर्णांक SQLITE_TCLAPI incrblobSeek(
  Client_तथ्य instance_तथ्य,
  दीर्घ offset,
  पूर्णांक seekMode,
  पूर्णांक *errorCodePtr
){
  वापस incrblobWideSeek(instance_तथ्य,offset,seekMode,errorCodePtr);
}


अटल व्योम SQLITE_TCLAPI incrblobWatch(
  Client_तथ्य instance_तथ्य,
  पूर्णांक mode
){
  /* NO-OP */
}
अटल पूर्णांक SQLITE_TCLAPI incrblobHandle(
  Client_तथ्य instance_तथ्य,
  पूर्णांक dir,
  Client_तथ्य *hPtr
){
  वापस TCL_ERROR;
}

अटल Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_VERSION_5,             /* version                              */
  incrblobClose,                     /* closeProc                            */
  incrblob_निवेश,                     /* inputProc                            */
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
अटल पूर्णांक createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  स्थिर अक्षर *zDb,
  स्थिर अक्षर *zTable,
  स्थिर अक्षर *zColumn,
  sqlite_int64 iRow,
  पूर्णांक isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  पूर्णांक rc;
  पूर्णांक flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  अटल पूर्णांक गिनती = 0;
  अक्षर zChannel[64];

  rc = sqlite3_blob_open(db, zDb, zTable, zColumn, iRow, !isReadonly, &pBlob);
  अगर( rc!=SQLITE_OK ){
    Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    वापस TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(माप(IncrblobChannel));
  स_रखो(p, 0, माप(*p));
  p->pBlob = pBlob;
  अगर( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(माप(zChannel), zChannel, "incrblob_%d", ++गिनती);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_RegisterChannel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->p_अग्र = pDb->pIncrblob;
  p->p_पूर्व = 0;
  अगर( p->p_अग्र ){
    p->p_अग्र->p_पूर्व = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_Set_परिणाम(interp, (अक्षर *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  वापस TCL_OK;
}
#अन्यथा  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
  #घोषणा closeIncrblobChannels(pDb)
#पूर्ण_अगर

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
अटल पूर्णांक safeToUseEvalObjv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  स्थिर अक्षर *z;
  Tcl_आकार n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  जबतक( n-- > 0 ){
    पूर्णांक c = *(z++);
    अगर( c=='$' || c=='[' || c==';' ) वापस 0;
  }
  वापस 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
अटल SqlFunc *findSqlFunc(SqliteDb *pDb, स्थिर अक्षर *zName){
  SqlFunc *p, *pNew;
  पूर्णांक nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( माप(*pNew) + nName + 1 );
  pNew->zName = (अक्षर*)&pNew[1];
  स_नकल(pNew->zName, zName, nName+1);
  क्रम(p=pDb->pFunc; p; p=p->p_अग्र){
    अगर( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((अक्षर*)pNew);
      वापस p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->p_अग्र = pDb->pFunc;
  pDb->pFunc = pNew;
  वापस pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
अटल व्योम dbFreeStmt(SqlPreparedStmt *pStmt){
#अगर_घोषित SQLITE_TEST
  अगर( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((अक्षर *)pStmt->zSql);
  }
#पूर्ण_अगर
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((अक्षर *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
अटल व्योम flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *p_अग्र;

  क्रम(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=p_अग्र){
    p_अग्र = pPreStmt->p_अग्र;
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
अटल व्योम addDatabaseRef(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
अटल व्योम delDatabaseRef(SqliteDb *pDb){
  निश्चित( pDb->nRef>0 );
  pDb->nRef--;
  अगर( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    जबतक( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->p_अग्र;
      निश्चित( pFunc->pDb==pDb );
      Tcl_Decr_Ref_गिनती(pFunc->pScript);
      Tcl_Free((अक्षर*)pFunc);
    }
    जबतक( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->p_अग्र;
      Tcl_Free((अक्षर*)pCollate);
    }
    अगर( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    अगर( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    अगर( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    अगर( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    अगर( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    अगर( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    अगर( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    अगर( pDb->pUpdateHook ){
      Tcl_Decr_Ref_गिनती(pDb->pUpdateHook);
    }
    अगर( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_गिनती(pDb->pPreUpdateHook);
    }
    अगर( pDb->pRollbackHook ){
      Tcl_Decr_Ref_गिनती(pDb->pRollbackHook);
    }
    अगर( pDb->pWalHook ){
      Tcl_Decr_Ref_गिनती(pDb->pWalHook);
    }
    अगर( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_गिनती(pDb->pCollateNeeded);
    }
    Tcl_Free((अक्षर*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
अटल व्योम SQLITE_TCLAPI DbDeleteCmd(व्योम *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
अटल पूर्णांक DbBusyHandler(व्योम *cd, पूर्णांक nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  पूर्णांक rc;
  अक्षर zVal[30];

  sqlite3_snprintf(माप(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (अक्षर*)0);
  अगर( rc!=TCL_OK || म_से_प(Tcl_Get_String_परिणाम(pDb->interp)) ){
    वापस 0;
  }
  वापस 1;
}

#अगर_अघोषित SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
अटल पूर्णांक DbProgressHandler(व्योम *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  पूर्णांक rc;

  निश्चित( pDb->zProgress );
  rc = Tcl_Eval(pDb->interp, pDb->zProgress);
  अगर( rc!=TCL_OK || म_से_प(Tcl_Get_String_परिणाम(pDb->interp)) ){
    वापस 1;
  }
  वापस 0;
}
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
अटल व्योम DbTraceHandler(व्योम *cd, स्थिर अक्षर *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_मूल्य(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_परिणाम(pDb->interp);
}
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
अटल पूर्णांक DbTraceV2Handler(
  अचिन्हित type, /* One of the SQLITE_TRACE_* event types. */
  व्योम *cd,      /* The original context data pointer. */
  व्योम *pd,      /* Primary event data, depends on event type. */
  व्योम *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  चयन( type ){
    हाल SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      अक्षर *zSql = (अक्षर *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_गिनती(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_गिनती(pCmd);
      Tcl_Reset_परिणाम(pDb->interp);
      अवरोध;
    }
    हाल SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_गिनती(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_गिनती(pCmd);
      Tcl_Reset_परिणाम(pDb->interp);
      अवरोध;
    }
    हाल SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_गिनती(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_गिनती(pCmd);
      Tcl_Reset_परिणाम(pDb->interp);
      अवरोध;
    }
    हाल SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_गिनती(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
      Tcl_Decr_Ref_गिनती(pCmd);
      Tcl_Reset_परिणाम(pDb->interp);
      अवरोध;
    }
  }
  वापस SQLITE_OK;
}
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
अटल व्योम DbProfileHandler(व्योम *cd, स्थिर अक्षर *zSql, sqlite_uint64 पंचांग){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  अक्षर zTm[100];

  sqlite3_snprintf(माप(zTm)-1, zTm, "%lld", पंचांग);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_मूल्य(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_परिणाम(pDb->interp);
}
#पूर्ण_अगर

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
अटल पूर्णांक DbCommitHandler(व्योम *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  पूर्णांक rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  अगर( rc!=TCL_OK || म_से_प(Tcl_Get_String_परिणाम(pDb->interp)) ){
    वापस 1;
  }
  वापस 0;
}

अटल व्योम DbRollbackHandler(व्योम *client_तथ्य){
  SqliteDb *pDb = (SqliteDb*)client_तथ्य;
  निश्चित(pDb->pRollbackHook);
  अगर( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
अटल पूर्णांक DbWalHandler(
  व्योम *client_तथ्य,
  sqlite3 *db,
  स्थिर अक्षर *zDb,
  पूर्णांक nEntry
){
  पूर्णांक ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)client_तथ्य;
  Tcl_Interp *interp = pDb->interp;
  निश्चित(pDb->pWalHook);

  निश्चित( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_गिनती(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(nEntry));
  अगर( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_Get_Obj_परिणाम(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_गिनती(p);

  वापस ret;
}

#अगर defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
अटल व्योम setTestUnlockNotifyVars(Tcl_Interp *interp, पूर्णांक iArg, पूर्णांक nArg){
  अक्षर zBuf[64];
  sqlite3_snprintf(माप(zBuf), zBuf, "%d", iArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(माप(zBuf), zBuf, "%d", nArg);
  Tcl_SetVar(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#अन्यथा
# define setTestUnlockNotifyVars(x,y,z)
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_UNLOCK_NOTIFY
अटल व्योम DbUnlockNotify(व्योम **apArg, पूर्णांक nArg){
  पूर्णांक i;
  क्रम(i=0; i<nArg; i++){
    स्थिर पूर्णांक flags = (TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    setTestUnlockNotifyVars(pDb->interp, i, nArg);
    निश्चित( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_गिनती(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
अटल व्योम DbPreUpdateHandler(
  व्योम *p,
  sqlite3 *db,
  पूर्णांक op,
  स्थिर अक्षर *zDb,
  स्थिर अक्षर *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  अटल स्थिर अक्षर *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  निश्चित( (SQLITE_DELETE-1)/9 == 0 );
  निश्चित( (SQLITE_जोड़ो-1)/9 == 1 );
  निश्चित( (SQLITE_UPDATE-1)/9 == 2 );
  निश्चित( pDb->pPreUpdateHook );
  निश्चित( db==pDb->db );
  निश्चित( op==SQLITE_जोड़ो || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_गिनती(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_गिनती(pCmd);
}
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */

अटल व्योम DbUpdateHandler(
  व्योम *p,
  पूर्णांक op,
  स्थिर अक्षर *zDb,
  स्थिर अक्षर *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  अटल स्थिर अक्षर *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  निश्चित( (SQLITE_DELETE-1)/9 == 0 );
  निश्चित( (SQLITE_जोड़ो-1)/9 == 1 );
  निश्चित( (SQLITE_UPDATE-1)/9 == 2 );

  निश्चित( pDb->pUpdateHook );
  निश्चित( op==SQLITE_जोड़ो || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_गिनती(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_गिनती(pCmd);
}

अटल व्योम tclCollateNeeded(
  व्योम *pCtx,
  sqlite3 *db,
  पूर्णांक enc,
  स्थिर अक्षर *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_गिनती(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_गिनती(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
अटल पूर्णांक tclSqlCollate(
  व्योम *pCtx,
  पूर्णांक nA,
  स्थिर व्योम *zA,
  पूर्णांक nB,
  स्थिर व्योम *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_गिनती(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
  Tcl_Decr_Ref_गिनती(pCmd);
  वापस (म_से_प(Tcl_Get_String_परिणाम(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
अटल व्योम tclSqlFunc(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य**argv){
  SqlFunc *p = sqlite3_user_तथ्य(context);
  Tcl_Obj *pCmd;
  पूर्णांक i;
  पूर्णांक rc;

  अगर( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_गिनती(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_गिनती(pCmd);
  }अन्यथा{
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
    Tcl_आकार nArg;
    अगर( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_परिणाम_error(context, Tcl_Get_String_परिणाम(p->interp), -1);
      वापस;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_गिनती(pCmd);
    क्रम(i=0; i<argc; i++){
      sqlite3_मूल्य *pIn = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      चयन( sqlite3_मूल्य_type(pIn) ){
        हाल SQLITE_BLOB: {
          पूर्णांक bytes = sqlite3_मूल्य_bytes(pIn);
          pVal = Tcl_NewByteArrayObj(sqlite3_मूल्य_blob(pIn), bytes);
          अवरोध;
        }
        हाल SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_मूल्य_int64(pIn);
          अगर( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((पूर्णांक)v);
          }अन्यथा{
            pVal = Tcl_NewWideIntObj(v);
          }
          अवरोध;
        }
        हाल SQLITE_FLOAT: {
          द्विगुन r = sqlite3_मूल्य_double(pIn);
          pVal = Tcl_NewDoubleObj(r);
          अवरोध;
        }
        हाल SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          अवरोध;
        }
        शेष: {
          पूर्णांक bytes = sqlite3_मूल्य_bytes(pIn);
          pVal = Tcl_NewStringObj((अक्षर *)sqlite3_मूल्य_text(pIn), bytes);
          अवरोध;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      अगर( rc ){
        Tcl_Decr_Ref_गिनती(pCmd);
        sqlite3_परिणाम_error(context, Tcl_Get_String_परिणाम(p->interp), -1);
        वापस;
      }
    }
    अगर( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_DIRECT);
    Tcl_Decr_Ref_गिनती(pCmd);
  }

  अगर( TCL_BREAK==rc ){
    sqlite3_परिणाम_null(context);
  }अन्यथा अगर( rc && rc!=TCL_RETURN ){
    sqlite3_परिणाम_error(context, Tcl_Get_String_परिणाम(p->interp), -1);
  }अन्यथा{
    Tcl_Obj *pVar = Tcl_Get_Obj_परिणाम(p->interp);
    Tcl_आकार n;
    u8 *तथ्य;
    स्थिर अक्षर *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    अक्षर c = zType[0];
    पूर्णांक eType = p->eType;

    अगर( eType==SQLITE_NULL ){
      अगर( c=='b' && म_भेद(zType,"bytearray")==0 && pVar->bytes==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }अन्यथा अगर( (c=='b' && pVar->bytes==0 && म_भेद(zType,"boolean")==0 )
             || (c=='b' && pVar->bytes==0 && म_भेद(zType,"booleanString")==0 )
             || (c=='w' && म_भेद(zType,"wideInt")==0)
             || (c=='i' && म_भेद(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }अन्यथा अगर( c=='d' && म_भेद(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }अन्यथा{
        eType = SQLITE_TEXT;
      }
    }

    चयन( eType ){
      हाल SQLITE_BLOB: {
        तथ्य = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_परिणाम_blob64(context, तथ्य, n, SQLITE_TRANSIENT);
        अवरोध;
      }
      हाल SQLITE_INTEGER: {
        Tcl_WideInt v;
        अगर( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_परिणाम_int64(context, v);
          अवरोध;
        }
        /* fall-through */
      }
      हाल SQLITE_FLOAT: {
        द्विगुन r;
        अगर( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_परिणाम_double(context, r);
          अवरोध;
        }
        /* fall-through */
      }
      शेष: {
        तथ्य = (अचिन्हित अक्षर *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_परिणाम_text64(context, (अक्षर *)तथ्य, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        अवरोध;
      }
    }

  }
}

#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
अटल पूर्णांक auth_callback(
  व्योम *pArg,
  पूर्णांक code,
  स्थिर अक्षर *zArg1,
  स्थिर अक्षर *zArg2,
  स्थिर अक्षर *zArg3,
  स्थिर अक्षर *zArg4
){
  स्थिर अक्षर *zCode;
  Tcl_DString str;
  पूर्णांक rc;
  स्थिर अक्षर *zReply;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  अगर( pDb->disableAuth ) वापस SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  चयन( code ){
    हाल SQLITE_COPY              : zCode="SQLITE_COPY"; अवरोध;
    हाल SQLITE_CREATE_सूचकांक      : zCode="SQLITE_CREATE_INDEX"; अवरोध;
    हाल SQLITE_CREATE_TABLE      : zCode="SQLITE_CREATE_TABLE"; अवरोध;
    हाल SQLITE_CREATE_अस्थायी_सूचकांक : zCode="SQLITE_CREATE_TEMP_INDEX"; अवरोध;
    हाल SQLITE_CREATE_अस्थायी_TABLE : zCode="SQLITE_CREATE_TEMP_TABLE"; अवरोध;
    हाल SQLITE_CREATE_अस्थायी_TRIGGER: zCode="SQLITE_CREATE_TEMP_TRIGGER"; अवरोध;
    हाल SQLITE_CREATE_अस्थायी_VIEW  : zCode="SQLITE_CREATE_TEMP_VIEW"; अवरोध;
    हाल SQLITE_CREATE_TRIGGER    : zCode="SQLITE_CREATE_TRIGGER"; अवरोध;
    हाल SQLITE_CREATE_VIEW       : zCode="SQLITE_CREATE_VIEW"; अवरोध;
    हाल SQLITE_DELETE            : zCode="SQLITE_DELETE"; अवरोध;
    हाल SQLITE_DROP_सूचकांक        : zCode="SQLITE_DROP_INDEX"; अवरोध;
    हाल SQLITE_DROP_TABLE        : zCode="SQLITE_DROP_TABLE"; अवरोध;
    हाल SQLITE_DROP_अस्थायी_सूचकांक   : zCode="SQLITE_DROP_TEMP_INDEX"; अवरोध;
    हाल SQLITE_DROP_अस्थायी_TABLE   : zCode="SQLITE_DROP_TEMP_TABLE"; अवरोध;
    हाल SQLITE_DROP_अस्थायी_TRIGGER : zCode="SQLITE_DROP_TEMP_TRIGGER"; अवरोध;
    हाल SQLITE_DROP_अस्थायी_VIEW    : zCode="SQLITE_DROP_TEMP_VIEW"; अवरोध;
    हाल SQLITE_DROP_TRIGGER      : zCode="SQLITE_DROP_TRIGGER"; अवरोध;
    हाल SQLITE_DROP_VIEW         : zCode="SQLITE_DROP_VIEW"; अवरोध;
    हाल SQLITE_जोड़ो            : zCode="SQLITE_INSERT"; अवरोध;
    हाल SQLITE_PRAGMA            : zCode="SQLITE_PRAGMA"; अवरोध;
    हाल SQLITE_READ              : zCode="SQLITE_READ"; अवरोध;
    हाल SQLITE_SELECT            : zCode="SQLITE_SELECT"; अवरोध;
    हाल SQLITE_TRANSACTION       : zCode="SQLITE_TRANSACTION"; अवरोध;
    हाल SQLITE_UPDATE            : zCode="SQLITE_UPDATE"; अवरोध;
    हाल SQLITE_ATTACH            : zCode="SQLITE_ATTACH"; अवरोध;
    हाल SQLITE_DETACH            : zCode="SQLITE_DETACH"; अवरोध;
    हाल SQLITE_ALTER_TABLE       : zCode="SQLITE_ALTER_TABLE"; अवरोध;
    हाल SQLITE_REINDEX           : zCode="SQLITE_REINDEX"; अवरोध;
    हाल SQLITE_ANALYZE           : zCode="SQLITE_ANALYZE"; अवरोध;
    हाल SQLITE_CREATE_VTABLE     : zCode="SQLITE_CREATE_VTABLE"; अवरोध;
    हाल SQLITE_DROP_VTABLE       : zCode="SQLITE_DROP_VTABLE"; अवरोध;
    हाल SQLITE_FUNCTION          : zCode="SQLITE_FUNCTION"; अवरोध;
    हाल SQLITE_SAVEPOINT         : zCode="SQLITE_SAVEPOINT"; अवरोध;
    हाल SQLITE_RECURSIVE         : zCode="SQLITE_RECURSIVE"; अवरोध;
    शेष                       : zCode="????"; अवरोध;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, zCode);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_मूल्य(&str));
  Tcl_DStringFree(&str);
  zReply = rc==TCL_OK ? Tcl_Get_String_परिणाम(pDb->interp) : "SQLITE_DENY";
  अगर( म_भेद(zReply,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }अन्यथा अगर( म_भेद(zReply,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }अन्यथा अगर( म_भेद(zReply,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }अन्यथा{
    rc = 999;
  }
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_OMIT_AUTHORIZATION */

#अगर 0
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
अटल अक्षर *local_getline(अक्षर *zPrompt, खाता *in){
  अक्षर *z_पंक्ति;
  पूर्णांक n_पंक्ति;
  पूर्णांक n;

  n_पंक्ति = 100;
  z_पंक्ति = दो_स्मृति( n_पंक्ति );
  अगर( z_पंक्ति==0 ) वापस 0;
  n = 0;
  जबतक( 1 ){
    अगर( n+100>n_पंक्ति ){
      n_पंक्ति = n_पंक्ति*2 + 100;
      z_पंक्ति = पुनः_स्मृति(z_पंक्ति, n_पंक्ति);
      अगर( z_पंक्ति==0 ) वापस 0;
    }
    अगर( ख_माला_लो(&z_पंक्ति[n], n_पंक्ति - n, in)==0 ){
      अगर( n==0 ){
        मुक्त(z_पंक्ति);
        वापस 0;
      }
      z_पंक्ति[n] = 0;
      अवरोध;
    }
    जबतक( z_पंक्ति[n] ){ n++; }
    अगर( n>0 && z_पंक्ति[n-1]=='\n' ){
      n--;
      z_पंक्ति[n] = 0;
      अवरोध;
    }
  }
  z_पंक्ति = पुनः_स्मृति( z_पंक्ति, n+1 );
  वापस z_पंक्ति;
}
#पूर्ण_अगर


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
अटल पूर्णांक SQLITE_TCLAPI DbTransPostCmd(
  Client_तथ्य तथ्य[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  पूर्णांक परिणाम                           /* Result of evaluating SCRIPT */
){
  अटल स्थिर अक्षर *स्थिर azEnd[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)तथ्य[0];
  पूर्णांक rc = परिणाम;
  स्थिर अक्षर *zEnd;

  pDb->nTransaction--;
  zEnd = azEnd[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  अगर( sqlite3_exec(pDb->db, zEnd, 0, 0, 0) ){
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
    अगर( rc!=TCL_ERROR ){
      Tcl_Append_परिणाम(interp, sqlite3_errmsg(pDb->db), (अक्षर*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  वापस rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
अटल पूर्णांक dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  स्थिर अक्षर *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  स्थिर अक्षर **pzOut              /* OUT: Pointer to next SQL statement */
){
  अचिन्हित पूर्णांक prepFlags = 0;
#अगर_घोषित SQLITE_TEST
  अगर( pDb->bLegacyPrepare ){
    वापस sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pzOut);
  }
#पूर्ण_अगर
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  अगर( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  वापस sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pzOut);
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
अटल पूर्णांक dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  अक्षर स्थिर *zIn,                /* SQL to compile */
  अक्षर स्थिर **pzOut,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  स्थिर अक्षर *zSql = zIn;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  पूर्णांक nSql;                       /* Length of zSql in bytes */
  पूर्णांक nVar = 0;                   /* Number of variables in statement */
  पूर्णांक iParm = 0;                  /* Next free entry in apParm */
  अक्षर c;
  पूर्णांक i;
  पूर्णांक need_परिणाम_Reset = 0;        /* Need to invoke Tcl_ResetResult() */
  पूर्णांक rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  जबतक( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  क्रम(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->p_अग्र){
    पूर्णांक n = pPreStmt->nSql;
    अगर( nSql>=n
        && स_भेद(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pzOut = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      अगर( pPreStmt->p_पूर्व ){
        pPreStmt->p_पूर्व->p_अग्र = pPreStmt->p_अग्र;
      }अन्यथा{
        pDb->stmtList = pPreStmt->p_अग्र;
      }
      अगर( pPreStmt->p_अग्र ){
        pPreStmt->p_अग्र->p_पूर्व = pPreStmt->p_पूर्व;
      }अन्यथा{
        pDb->stmtLast = pPreStmt->p_पूर्व;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_गिनती(pStmt);
      अवरोध;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  अगर( pPreStmt==0 ){
    पूर्णांक nByte;

    अगर( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pzOut) ){
      Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      वापस TCL_ERROR;
    }
    अगर( pStmt==0 ){
      अगर( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        वापस TCL_ERROR;
      }अन्यथा{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        वापस TCL_OK;
      }
    }

    निश्चित( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_गिनती(pStmt);
    nByte = माप(SqlPreparedStmt) + nVar*माप(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    स_रखो(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (पूर्णांक)(*pzOut - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#अगर_घोषित SQLITE_TEST
    अगर( pPreStmt->zSql==0 ){
      अक्षर *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      स_नकल(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#पूर्ण_अगर
  }
  निश्चित( pPreStmt );
  निश्चित( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  निश्चित( 0==स_भेद(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  क्रम(i=1; i<=nVar; i++){
    स्थिर अक्षर *zVar = sqlite3_bind_parameter_name(pStmt, i);
    अगर( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      अगर( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        पूर्णांक rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_गिनती(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        अगर( need_परिणाम_Reset ) Tcl_Reset_परिणाम(interp);
        need_परिणाम_Reset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_DIRECT);
        Tcl_Decr_Ref_गिनती(pCmd);
        अगर( rx==TCL_OK ){
          pVar = Tcl_Get_Obj_परिणाम(interp);
        }अन्यथा अगर( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          अवरोध;
        }अन्यथा{
          pVar = 0;
        }
      }
      अगर( pVar ){
        Tcl_आकार n;
        u8 *तथ्य;
        स्थिर अक्षर *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        अगर( zVar[0]=='@' ||
           (c=='b' && म_भेद(zType,"bytearray")==0 && pVar->bytes==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          तथ्य = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, तथ्य, n, SQLITE_STATIC);
          Tcl_Incr_Ref_गिनती(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }अन्यथा अगर( c=='b' && pVar->bytes==0 
               && (म_भेद(zType,"booleanString")==0
                   || म_भेद(zType,"boolean")==0)
        ){
          पूर्णांक nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }अन्यथा अगर( c=='d' && म_भेद(zType,"double")==0 ){
          द्विगुन r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }अन्यथा अगर( (c=='w' && म_भेद(zType,"wideInt")==0) ||
              (c=='i' && म_भेद(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }अन्यथा{
          तथ्य = (अचिन्हित अक्षर *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (अक्षर *)तथ्य, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_गिनती(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }अन्यथा{
        sqlite3_bind_null(pStmt, i);
      }
      अगर( need_परिणाम_Reset ) Tcl_Reset_परिणाम(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  अगर( need_परिणाम_Reset && rc==TCL_OK ) Tcl_Reset_परिणाम(pDb->interp);

  वापस rc;
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
अटल व्योम dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  पूर्णांक discard                     /* True to delete (not cache) the pPreStmt */
){
  पूर्णांक i;

  /* Free the bound string and blob parameters */
  क्रम(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_गिनती(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  अगर( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }अन्यथा{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->p_अग्र = pDb->stmtList;
    pPreStmt->p_पूर्व = 0;
    अगर( pDb->stmtList ){
     pDb->stmtList->p_पूर्व = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    अगर( pDb->stmtLast==0 ){
      निश्चित( pDb->nStmt==0 );
      pDb->stmtLast = pPreStmt;
    }अन्यथा{
      निश्चित( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    जबतक( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *pLast = pDb->stmtLast;
      pDb->stmtLast = pLast->p_पूर्व;
      pDb->stmtLast->p_अग्र = 0;
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
प्रकार काष्ठा DbEvalContext DbEvalContext;
काष्ठा DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  स्थिर अक्षर *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  पूर्णांक nCol;                       /* Number of columns returned by pStmt */
  पूर्णांक evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#घोषणा SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#घोषणा SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
अटल व्योम dbReleaseColumnNames(DbEvalContext *p){
  अगर( p->apColName ){
    पूर्णांक i;
    क्रम(i=0; i<p->nCol; i++){
      Tcl_Decr_Ref_गिनती(p->apColName[i]);
    }
    Tcl_Free((अक्षर *)p->apColName);
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
अटल व्योम dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  पूर्णांक evalFlags                   /* Flags controlling evaluation */
){
  स_रखो(p, 0, माप(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_गिनती(pSql);
  अगर( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_गिनती(pVarName);
  }
  p->evalFlags = evalFlags;
  addDatabaseRef(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
अटल व्योम dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  पूर्णांक *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  अगर( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    पूर्णांक i;                        /* Iterator variable */
    पूर्णांक nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_गिनती(pStmt);
    अगर( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( माप(Tcl_Obj*)*nCol );
      क्रम(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_Incr_Ref_गिनती(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    अगर( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *pStar = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_गिनती(pColList);
      Tcl_Incr_Ref_गिनती(pStar);
      क्रम(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      अगर( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_ObjSetVar2(interp, p->pVarName, pStar, pColList, 0);
      }अन्यथा{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, शून्य, 0);
        अगर( !pDict ){
          pDict = Tcl_NewDictObj();
        }अन्यथा अगर( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        अगर( Tcl_DictObjPut(interp, pDict, pStar, pColList)==TCL_OK ){
          Tcl_ObjSetVar2(interp, p->pVarName, शून्य, pDict, 0);
        }
        Tcl_Bounce_Ref_गिनती(pDict);
      }
      Tcl_Decr_Ref_गिनती(pStar);
      Tcl_Decr_Ref_गिनती(pColList);
    }
  }

  अगर( papColName ){
    *papColName = p->apColName;
  }
  अगर( pnCol ){
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
अटल पूर्णांक dbEvalStep(DbEvalContext *p){
  स्थिर अक्षर *z_पूर्व_Sql = 0;       /* Previous value of p->zSql */

  जबतक( p->zSql[0] || p->pPreStmt ){
    पूर्णांक rc;
    अगर( p->pPreStmt==0 ){
      z_पूर्व_Sql = (p->zSql==z_पूर्व_Sql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      अगर( rc!=TCL_OK ) वापस rc;
    }अन्यथा{
      पूर्णांक rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_step(pStmt);
      अगर( rcs==SQLITE_ROW ){
        वापस TCL_OK;
      }
      अगर( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->nStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_FULLSCAN_STEP,1);
      pDb->nSort = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_SORT,1);
      pDb->n_सूचकांक = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_status(pStmt,SQLITE_STMTSTATUS_VM_STEP,1);
      dbReleaseColumnNames(p);
      p->pPreStmt = 0;

      अगर( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#अगर SQLITE_TEST
        अगर( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && z_पूर्व_Sql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = z_पूर्व_Sql;
          जारी;
        }
#पूर्ण_अगर
        Tcl_Set_Obj_परिणाम(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        वापस TCL_ERROR;
      }अन्यथा{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /* Finished */
  वापस TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
अटल व्योम dbEvalFinalize(DbEvalContext *p){
  अगर( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  अगर( p->pVarName ){
    Tcl_Decr_Ref_गिनती(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_गिनती(p->pSql);
  dbReleaseColumnNames(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
अटल Tcl_Obj *db_Eval_Column_मूल्य(DbEvalContext *p, पूर्णांक iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  चयन( sqlite3_column_type(pStmt, iCol) ){
    हाल SQLITE_BLOB: {
      पूर्णांक bytes = sqlite3_column_bytes(pStmt, iCol);
      स्थिर अक्षर *zBlob = sqlite3_column_blob(pStmt, iCol);
      अगर( !zBlob ) bytes = 0;
      वापस Tcl_NewByteArrayObj((u8*)zBlob, bytes);
    }
    हाल SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      अगर( v>=-2147483647 && v<=2147483647 ){
        वापस Tcl_NewIntObj((पूर्णांक)v);
      }अन्यथा{
        वापस Tcl_NewWideIntObj(v);
      }
    }
    हाल SQLITE_FLOAT: {
      वापस Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    हाल SQLITE_NULL: {
      वापस Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  वापस Tcl_NewStringObj((अक्षर*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#अगर TCL_MAJOR_VERSION>8 || !defined(TCL_MINOR_VERSION) \
                        || TCL_MINOR_VERSION>=6
# define SQLITE_TCL_NRE 1
अटल पूर्णांक DbUseNre(व्योम){
  पूर्णांक major, minor;
  Tcl_GetVersion(&major, &minor, 0, 0);
  वापस( (major==8 && minor>=6) || major>8 );
}
#अन्यथा
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (व्योम)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (व्योम)0
#पूर्ण_अगर

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
अटल पूर्णांक SQLITE_TCLAPI Db_Eval_अग्र_Cmd(
  Client_तथ्य तथ्य[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  पूर्णांक परिणाम                           /* Result so far */
){
  पूर्णांक rc = परिणाम;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)तथ्य[0];
  Tcl_Obj * स्थिर pScript = (Tcl_Obj *)तथ्य[1];
  Tcl_Obj * स्थिर pVarName = p->pVarName;

  जबतक( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = dbEvalStep(p)) ){
    पूर्णांक i;
    पूर्णांक nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    क्रम(i=0; i<nCol; i++){
      अगर( pVarName==0 ){
        Tcl_ObjSetVar2(interp, apColName[i], 0, db_Eval_Column_मूल्य(p,i), 0);
      }अन्यथा अगर( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        अगर( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }अन्यथा{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, शून्य, 0);
          अगर( pDict ){
            अगर( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            अगर( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_ObjSetVar2(interp, pVarName, शून्य, pDict, 0);
            }
            Tcl_Bounce_Ref_गिनती(pDict);
          }
        }
      }अन्यथा अगर( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_ObjSetVar2(interp, pVarName, apColName[i],
                       db_Eval_Column_मूल्य(p,i), 0);
      }अन्यथा{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, शून्य, 0);
        अगर( !pDict ){
          pDict = Tcl_NewDictObj();
        }अन्यथा अगर( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        अगर( Tcl_DictObjPut(interp, pDict, apColName[i],
                           db_Eval_Column_मूल्य(p,i))==TCL_OK ){
          Tcl_ObjSetVar2(interp, pVarName, शून्य, pDict, 0);
        }
        Tcl_Bounce_Ref_गिनती(pDict);
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
    अगर( DbUseNre() ){
      Tcl_NRAddCallback(interp, Db_Eval_अग्र_Cmd, (व्योम*)p, (व्योम*)pScript, 0, 0);
      वापस Tcl_NREvalObj(interp, pScript, 0);
    }अन्यथा{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_गिनती(pScript);
  dbEvalFinalize(p);
  Tcl_Free((अक्षर *)p);

  अगर( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_Reset_परिणाम(interp);
    rc = TCL_OK;
  }
  वापस rc;
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
अटल व्योम DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  अगर( *ppHook ){
    Tcl_Set_Obj_परिणाम(interp, *ppHook);
    अगर( pArg ){
      Tcl_Decr_Ref_गिनती(*ppHook);
      *ppHook = 0;
    }
  }
  अगर( pArg ){
    निश्चित( !(*ppHook) );
    अगर( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_गिनती(*ppHook);
    }
  }

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
  sqlite3_preupdate_hook(db, (pDb->pPreUpdateHook?DbPreUpdateHandler:0), pDb);
#पूर्ण_अगर
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
अटल पूर्णांक dbQrf(SqliteDb *pDb, पूर्णांक objc, Tcl_Obj *स्थिर*objv){
#अगर_अघोषित SQLITE_QRF_H
  Tcl_Set_परिणाम(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  वापस TCL_ERROR;
#अन्यथा
  अक्षर *z_परिणाम = 0;             /* Result to be returned */
  स्थिर अक्षर *zSql = 0;          /* SQL to run */
  पूर्णांक i;                         /* Loop counter */
  पूर्णांक rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  अटल स्थिर अक्षर *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  अटल स्थिर अचिन्हित अक्षर aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_बायाँ,
    QRF_ALIGN_मध्य, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_दायाँ,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_शीर्ष,
    QRF_ALIGN_W
  };

  स_रखो(&qrf, 0, माप(qrf));
  qrf.iVersion = 2;
  qrf.pzOutput = &z_परिणाम;
  क्रम(i=2; i<objc; i++){
    स्थिर अक्षर *zArg = Tcl_GetString(objv[i]);
    स्थिर अक्षर *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    स्थिर अचिन्हित अक्षर aBoolMap[] = { 0, 2, 1, 2, 1 };
    अगर( zArg[0]!='-' ){
      अगर( zSql ){
        Tcl_Append_परिणाम(pDb->interp, "unknown argument: ", zArg, (अक्षर*)0);
        rc = TCL_ERROR;
        जाओ format_failed;
      }
      zSql  = zArg;
    }अन्यथा अगर( i==objc-1 ){
      Tcl_Append_परिणाम(pDb->interp, "option has no argument: ", zArg,(अक्षर*)0);
      rc = TCL_ERROR;
      जाओ format_failed;
    }अन्यथा अगर( म_भेद(zArg,"-style")==0 ){
      अटल स्थिर अक्षर *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      अटल अचिन्हित अक्षर aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_गिनती,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_जोड़ो,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_पंक्ति,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_Table
      };
      पूर्णांक style;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      अगर( rc ) जाओ format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-esc")==0 ){
      अटल स्थिर अक्षर *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      अटल अचिन्हित अक्षर aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_प्रतीक
      };
      पूर्णांक esc;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      अगर( rc ) जाओ format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-text")==0 || म_भेद(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      अटल स्थिर अक्षर *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      अटल अचिन्हित अक्षर aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      पूर्णांक txt;
      पूर्णांक k = zArg[2]=='e';
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      अगर( rc ) जाओ format_failed;
      अगर( k ){
        qrf.eText = aTextMap[txt];
      }अन्यथा अगर( txt<=2 ){
        निश्चित( QRF_No==1 );
        निश्चित( QRF_Yes==2 );
        निश्चित( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }अन्यथा{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-blob")==0 ){
      अटल स्थिर अक्षर *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      अटल अचिन्हित अक्षर aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_आकार
      };
      पूर्णांक blob;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      अगर( rc ) जाओ format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-wordwrap")==0 ){
      पूर्णांक v = 0;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      अगर( rc ) जाओ format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-textjsonb")==0
           || म_भेद(zArg,"-splitcolumn")==0
           || म_भेद(zArg,"-border")==0
           || म_भेद(zArg,"-rowcount")==0
    ){
      पूर्णांक v = 0;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      अगर( rc ) जाओ format_failed;
      अगर( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }अन्यथा अगर( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }अन्यथा अगर( zArg[1]=='r' ){
        qrf.b_Row_गिनती = aBoolMap[v];
      }अन्यथा{
        qrf.bSplitColumn = aBoolMap[v];
      }
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-defaultalign")==0
           || म_भेद(zArg,"-titlealign")==0){
      पूर्णांक ax = 0;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      अगर( rc ) जाओ format_failed;
      अगर( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }अन्यथा{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-wrap")==0
           || म_भेद(zArg,"-screenwidth")==0 
           || म_भेद(zArg,"-linelimit")==0 
           || म_भेद(zArg,"-titlelimit")==0 
    ){
      पूर्णांक v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      अगर( rc ) जाओ format_failed;
      अगर( v<QRF_MIN_WIDTH ){
        v = QRF_MIN_WIDTH;
      }अन्यथा अगर( v>QRF_MAX_WIDTH ){
        v = QRF_MAX_WIDTH;
      }
      अगर( zArg[1]=='w' ){
        qrf.nWrap = v;
      }अन्यथा अगर( zArg[1]=='s' ){
        qrf.nScreenWidth = v;
      }अन्यथा अगर( zArg[1]=='t' ){
        qrf.nTitleLimit = v;
      }अन्यथा{
        qrf.n_पंक्ति_Limit = v;
      }
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-charlimit")==0 ){
      पूर्णांक v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      अगर( rc ) जाओ format_failed;
      अगर( v<0 ) v = 0;
      qrf.nCharLimit = v;
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-multiinsert")==0 ){
      पूर्णांक v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      अगर( rc ) जाओ format_failed;
      अगर( v<0 ) v = 0;
      qrf.n_Multi_जोड़ो = v;
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-align")==0 ){
      Tcl_आकार n = 0;
      पूर्णांक jj;
      rc = Tcl_List_Obj_लंबाई(pDb->interp, objv[i+1], &n);
      अगर( rc ) जाओ format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*माप(qrf.aAlign[0]) );
      अगर( qrf.aAlign==0 ){
        Tcl_Append_परिणाम(pDb->interp, "out of memory", (अक्षर*)0);
        rc = TCL_ERROR;
        जाओ format_failed;
      }
      स_रखो(qrf.aAlign, 0, (n+1)*माप(qrf.aAlign[0]));
      qrf.nAlign = n;
      क्रम(jj=0; jj<n; jj++){
        पूर्णांक x;
        Tcl_Obj *pTerm;
        rc = Tcl_List_Obj_सूचकांक(pDb->interp, objv[i+1], jj, &pTerm);
        अगर( rc ) जाओ format_failed;
        rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, pTerm, azAlign,
                          "column alignment (-align)", 0, &x);
        अगर( rc ) जाओ format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-widths")==0 ){
      Tcl_आकार n = 0;
      पूर्णांक jj;
      rc = Tcl_List_Obj_लंबाई(pDb->interp, objv[i+1], &n);
      अगर( rc ) जाओ format_failed;
      sqlite3_free(qrf.aWidth);
      qrf.aWidth = sqlite3_malloc64( (n+1)*माप(qrf.aWidth[0]) );
      अगर( qrf.aWidth==0 ){
        Tcl_Append_परिणाम(pDb->interp, "out of memory", (अक्षर*)0);
        rc = TCL_ERROR;
        जाओ format_failed;
      }
      स_रखो(qrf.aWidth, 0, (n+1)*माप(qrf.aWidth[0]));
      qrf.nWidth = n;
      क्रम(jj=0; jj<n; jj++){
        Tcl_Obj *pTerm;
        पूर्णांक v;
        rc = Tcl_List_Obj_सूचकांक(pDb->interp, objv[i+1], jj, &pTerm);
        अगर( rc ) जाओ format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, pTerm, &v);
        अगर( v<(-QRF_MAX_WIDTH) ){
          v = -QRF_MAX_WIDTH;
        }अन्यथा अगर( v>QRF_MAX_WIDTH ){
          v = QRF_MAX_WIDTH;
        }
        qrf.aWidth[jj] = (लघु पूर्णांक)v;
      }
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-tablename")==0 ){
      qrf.zTableName = Tcl_GetString(objv[i+1]);
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-ifmt")==0 ){
      अक्षर *zFmt = Tcl_GetString(objv[i+1]);
      अगर( zFmt[0]==0 || म_भेद(zFmt,"auto")==0 ){
        zFmt = 0;
      }अन्यथा अगर( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_Append_परिणाम(pDb->interp, "bad format for -ifmt", (अक्षर*)0);
        rc = TCL_ERROR;
        जाओ format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-fpfmt")==0 ){
      अक्षर *zFmt = Tcl_GetString(objv[i+1]);
      अगर( zFmt[0]==0 || म_भेद(zFmt,"auto")==0 ){
        zFmt = 0;
      }अन्यथा अगर( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_Append_परिणाम(pDb->interp, "bad format for -fpfmt", (अक्षर*)0);
        rc = TCL_ERROR;
        जाओ format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }अन्यथा अगर( म_भेद(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.iVersion = म_से_प(Tcl_GetString(objv[i+1]));
      i++;
    }अन्यथा{
      Tcl_Append_परिणाम(pDb->interp, "unknown option: ", zArg, (अक्षर*)0);
      rc = TCL_ERROR;
      जाओ format_failed;
    }
  }
  जबतक( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    अक्षर *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    अगर( rc ) जाओ format_failed;
    अगर( pStmt==0 ) जारी;
    rc = sqlite3_format_query_परिणाम(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    अगर( rc ){
      Tcl_Set_परिणाम(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      जाओ format_failed;
    }
  }
  Tcl_Set_परिणाम(pDb->interp, z_परिणाम, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.aWidth);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(z_परिणाम);
  वापस rc;

#पूर्ण_अगर
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
अटल पूर्णांक SQLITE_TCLAPI DbObjCmd(
  व्योम *cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *स्थिर*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  पूर्णांक choice;
  पूर्णांक rc = TCL_OK;
  अटल स्थिर अक्षर *DB_strs[] = {
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
  क्रमागत DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_LOAD_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_INTERRUPT,            DB_LAST_जोड़ो_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_PROGRESS,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_STATUS,                DB_TIMEOUT,              DB_योग_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_VERSION,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  अगर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUBCOMMAND ...");
    वापस TCL_ERROR;
  }
  अगर( Tcl_Get_सूचकांक_From_Obj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    वापस TCL_ERROR;
  }

  चयन( (क्रमागत DB_enum)choice ){

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
  हाल DB_AUTHORIZER: {
#अगर_घोषित SQLITE_OMIT_AUTHORIZATION
    Tcl_Append_परिणाम(interp, "authorization not available in this build",
                     (अक्षर*)0);
    वापस TCL_ERROR;
#अन्यथा
    अगर( objc>3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?CALLBACK?");
      वापस TCL_ERROR;
    }अन्यथा अगर( objc==2 ){
      अगर( pDb->zAuth ){
        Tcl_Append_परिणाम(interp, pDb->zAuth, (अक्षर*)0);
      }
    }अन्यथा{
      अक्षर *zAuth;
      Tcl_आकार len;
      अगर( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      अगर( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zAuth, zAuth, len+1);
      }अन्यथा{
        pDb->zAuth = 0;
      }
      अगर( pDb->zAuth ){
        प्रकार पूर्णांक (*sqlite3_auth_cb)(
           व्योम*,पूर्णांक,स्थिर अक्षर*,स्थिर अक्षर*,
           स्थिर अक्षर*,स्थिर अक्षर*);
        pDb->interp = interp;
        sqlite3_set_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }अन्यथा{
        sqlite3_set_authorizer(pDb->db, 0, 0);
      }
    }
#पूर्ण_अगर
    अवरोध;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  हाल DB_BACKUP: {
    स्थिर अक्षर *zDestFile;
    स्थिर अक्षर *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    अगर( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }अन्यथा अगर( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }अन्यथा{
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?DATABASE? FILENAME");
      वापस TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    अगर( rc!=SQLITE_OK ){
      Tcl_Append_परिणाम(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (अक्षर*)0);
      sqlite3_close(pDest);
      वापस TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    अगर( pBackup==0 ){
      Tcl_Append_परिणाम(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (अक्षर*)0);
      sqlite3_close(pDest);
      वापस TCL_ERROR;
    }
    जबतक(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    अगर( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }अन्यथा{
      Tcl_Append_परिणाम(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (अक्षर*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    अवरोध;
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
  हाल DB_BIND_FALLBACK: {
    अगर( objc>3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?CALLBACK?");
      वापस TCL_ERROR;
    }अन्यथा अगर( objc==2 ){
      अगर( pDb->zBindFallback ){
        Tcl_Append_परिणाम(interp, pDb->zBindFallback, (अक्षर*)0);
      }
    }अन्यथा{
      अक्षर *zCallback;
      Tcl_आकार len;
      अगर( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      अगर( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zBindFallback, zCallback, len+1);
      }अन्यथा{
        pDb->zBindFallback = 0;
      }
    }
    अवरोध;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  हाल DB_BUSY: {
    अगर( objc>3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "CALLBACK");
      वापस TCL_ERROR;
    }अन्यथा अगर( objc==2 ){
      अगर( pDb->zBusy ){
        Tcl_Append_परिणाम(interp, pDb->zBusy, (अक्षर*)0);
      }
    }अन्यथा{
      अक्षर *zBusy;
      Tcl_आकार len;
      अगर( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      अगर( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zBusy, zBusy, len+1);
      }अन्यथा{
        pDb->zBusy = 0;
      }
      अगर( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }अन्यथा{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    अवरोध;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  हाल DB_CACHE: {
    अक्षर *subCmd;
    पूर्णांक n;

    अगर( objc<=2 ){
      Tcl_Wrong_अंक_Args(interp, 1, objv, "cache option ?arg?");
      वापस TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    अगर( *subCmd=='f' && म_भेद(subCmd,"flush")==0 ){
      अगर( objc!=3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "flush");
        वापस TCL_ERROR;
      }अन्यथा{
        flushStmtCache( pDb );
      }
    }अन्यथा अगर( *subCmd=='s' && म_भेद(subCmd,"size")==0 ){
      अगर( objc!=4 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "size n");
        वापस TCL_ERROR;
      }अन्यथा{
        अगर( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_Append_परिणाम( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (अक्षर*)0);
          वापस TCL_ERROR;
        }अन्यथा{
          अगर( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }अन्यथा अगर( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }अन्यथा{
      Tcl_Append_परिणाम( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (अक्षर*)0);
      वापस TCL_ERROR;
    }
    अवरोध;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  हाल DB_CHANGES: {
    Tcl_Obj *p_परिणाम;
    अगर( objc!=2 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "");
      वापस TCL_ERROR;
    }
    p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
    Tcl_SetWideIntObj(p_परिणाम, sqlite3_changes64(pDb->db));
    अवरोध;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  हाल DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    अवरोध;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  हाल DB_COLLATE: {
    SqlCollate *pCollate;
    अक्षर *zName;
    अक्षर *zScript;
    Tcl_आकार nScript;
    अगर( objc!=4 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "NAME SCRIPT");
      वापस TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( माप(*pCollate) + nScript + 1 );
    अगर( pCollate==0 ) वापस TCL_ERROR;
    pCollate->interp = interp;
    pCollate->p_अग्र = pDb->pCollate;
    pCollate->zScript = (अक्षर*)&pCollate[1];
    pDb->pCollate = pCollate;
    स_नकल(pCollate->zScript, zScript, nScript+1);
    अगर( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      वापस TCL_ERROR;
    }
    अवरोध;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  हाल DB_COLLATION_NEEDED: {
    अगर( objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "SCRIPT");
      वापस TCL_ERROR;
    }
    अगर( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_गिनती(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_गिनती(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    अवरोध;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  हाल DB_COMMIT_HOOK: {
    अगर( objc>3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?CALLBACK?");
      वापस TCL_ERROR;
    }अन्यथा अगर( objc==2 ){
      अगर( pDb->zCommit ){
        Tcl_Append_परिणाम(interp, pDb->zCommit, (अक्षर*)0);
      }
    }अन्यथा{
      स्थिर अक्षर *zCommit;
      Tcl_आकार len;
      अगर( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      अगर( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zCommit, zCommit, len+1);
      }अन्यथा{
        pDb->zCommit = 0;
      }
      अगर( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }अन्यथा{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    अवरोध;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  हाल DB_COMPLETE: {
#अगर_अघोषित SQLITE_OMIT_COMPLETE
    Tcl_Obj *p_परिणाम;
    पूर्णांक isComplete;
    अगर( objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "SQL");
      वापस TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
    Tcl_SetBooleanObj(p_परिणाम, isComplete);
#पूर्ण_अगर
    अवरोध;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  हाल DB_CONFIG: {
    अटल स्थिर काष्ठा DbConfigChoices {
      स्थिर अक्षर *zName;
      पूर्णांक op;
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
    Tcl_Obj *p_परिणाम;
    पूर्णांक ii;
    अगर( objc>4 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      वापस TCL_ERROR;
    }
    अगर( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      p_परिणाम = Tcl_NewListObj(0,0);
      क्रम(ii=0; ii<माप(aDbConfig)/माप(aDbConfig[0]); ii++){
        पूर्णांक v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, p_परिणाम,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, p_परिणाम,
           Tcl_NewIntObj(v));
      }
    }अन्यथा{
      स्थिर अक्षर *zOpt = Tcl_GetString(objv[2]);
      पूर्णांक onoff = -1;
      पूर्णांक v = 0;
      अगर( zOpt[0]=='-' ) zOpt++;
      क्रम(ii=0; ii<माप(aDbConfig)/माप(aDbConfig[0]); ii++){
        अगर( म_भेद(aDbConfig[ii].zName, zOpt)==0 ) अवरोध;
      }
      अगर( ii>=माप(aDbConfig)/माप(aDbConfig[0]) ){
        Tcl_Append_परिणाम(interp, "unknown config option: \"", zOpt,
                                "\"", (व्योम*)0);
        वापस TCL_ERROR;
      }
      अगर( objc==4 ){
        अगर( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          वापस TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      p_परिणाम = Tcl_NewIntObj(v);
    }
    Tcl_Set_Obj_परिणाम(interp, p_परिणाम);
    अवरोध;
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
  हाल DB_COPY: {
    अक्षर *zTable;               /* Insert data into this table */
    अक्षर *zFile;                /* The file from which to extract data */
    अक्षर *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    पूर्णांक nCol;                   /* Number of columns in the table */
    पूर्णांक nByte;                  /* Number of bytes in an SQL string */
    पूर्णांक i, j;                   /* Loop counters */
    पूर्णांक nSep;                   /* Number of bytes in zSep[] */
    पूर्णांक nNull;                  /* Number of bytes in zNull[] */
    अक्षर *zSql;                 /* An SQL statement */
    अक्षर *z_पंक्ति;                /* A single line of input from the file */
    अक्षर **azCol;               /* zLine[] broken up into columns */
    स्थिर अक्षर *zCommit;        /* How to commit changes */
    Tcl_Channel in;             /* The input file */
    पूर्णांक lineno = 0;             /* Line number of input file */
    अक्षर z_पंक्ति_अंक[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *p_परिणाम;           /* interp result */

    स्थिर अक्षर *zSep;
    स्थिर अक्षर *zNull;
    अगर( objc<5 || objc>7 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      वापस TCL_ERROR;
    }
    अगर( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }अन्यथा{
      zSep = "\t";
    }
    अगर( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }अन्यथा{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    zTable = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    अगर( nSep==0 ){
      Tcl_Append_परिणाम(interp,"Error: non-null separator required for copy",
                       (अक्षर*)0);
      वापस TCL_ERROR;
    }
    अगर(म_भेद(zConflict, "rollback") != 0 &&
       म_भेद(zConflict, "abort"   ) != 0 &&
       म_भेद(zConflict, "fail"    ) != 0 &&
       म_भेद(zConflict, "ignore"  ) != 0 &&
       म_भेद(zConflict, "replace" ) != 0 ) {
      Tcl_Append_परिणाम(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (अक्षर*)0);
      वापस TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", zTable);
    अगर( zSql==0 ){
      Tcl_Append_परिणाम(interp, "Error: no such table: ", zTable, (अक्षर*)0);
      वापस TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    अगर( rc ){
      Tcl_Append_परिणाम(interp, "Error: ", sqlite3_errmsg(pDb->db), (अक्षर*)0);
      nCol = 0;
    }अन्यथा{
      nCol = sqlite3_column_गिनती(pStmt);
    }
    sqlite3_finalize(pStmt);
    अगर( nCol==0 ) {
      वापस TCL_ERROR;
    }
    zSql = दो_स्मृति( nByte + 50 + nCol*2 );
    अगर( zSql==0 ) {
      Tcl_Append_परिणाम(interp, "Error: can't malloc()", (अक्षर*)0);
      वापस TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, zTable);
    j = strlen30(zSql);
    क्रम(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    मुक्त(zSql);
    अगर( rc ){
      Tcl_Append_परिणाम(interp, "Error: ", sqlite3_errmsg(pDb->db), (अक्षर*)0);
      sqlite3_finalize(pStmt);
      वापस TCL_ERROR;
    }
    in = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    अगर( in==0 ){
      sqlite3_finalize(pStmt);
      वापस TCL_ERROR;
    }
    Tcl_SetChannelOption(शून्य, in, "-translation", "auto");
    azCol = दो_स्मृति( माप(azCol[0])*(nCol+1) );
    अगर( azCol==0 ) {
      Tcl_Append_परिणाम(interp, "Error: can't malloc()", (अक्षर*)0);
      Tcl_Close(interp, in);
      वापस TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_गिनती(str);
    (व्योम)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    जबतक( Tcl_GetsObj(in, str)>=0 ) {
      अक्षर *z;
      Tcl_आकार byteLen;
      lineno++;
      z_पंक्ति = (अक्षर *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = z_पंक्ति;
      क्रम(i=0, z=z_पंक्ति; *z; z++){
        अगर( *z==zSep[0] && म_भेदन(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          अगर( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      अगर( i+1!=nCol ){
        अक्षर *zErr;
        पूर्णांक nErr = strlen30(zFile) + 200;
        zErr = दो_स्मृति(nErr);
        अगर( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_Append_परिणाम(interp, zErr, (अक्षर*)0);
          मुक्त(zErr);
        }
        zCommit = "ROLLBACK";
        अवरोध;
      }
      क्रम(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        अगर( (nNull>0 && म_भेद(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }अन्यथा{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_step(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_Set_Obj_लंबाई(str, 0);
      अगर( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp,"Error: ", sqlite3_errmsg(pDb->db), (अक्षर*)0);
        zCommit = "ROLLBACK";
        अवरोध;
      }
    }
    Tcl_Decr_Ref_गिनती(str);
    मुक्त(azCol);
    Tcl_Close(interp, in);
    sqlite3_finalize(pStmt);
    (व्योम)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    अगर( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
      अगर( Tcl_IsShared(p_परिणाम) ){
        p_परिणाम = Tcl_DuplicateObj(p_परिणाम);
      }
      Tcl_SetIntObj(p_परिणाम, lineno);
      rc = TCL_OK;
    }अन्यथा{
      /* failure, append lineno where failed */
      sqlite3_snprintf(माप(z_पंक्ति_अंक), z_पंक्ति_अंक,"%d",lineno);
      Tcl_Append_परिणाम(interp,", failed while processing line: ",z_पंक्ति_अंक,
                       (अक्षर*)0);
      rc = TCL_ERROR;
    }
    अवरोध;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  हाल DB_DESERIALIZE: {
#अगर_घोषित SQLITE_OMIT_DESERIALIZE
    Tcl_Append_परिणाम(interp, "MEMDB not available in this build",
                     (अक्षर*)0);
    rc = TCL_ERROR;
#अन्यथा
    स्थिर अक्षर *zSchema = 0;
    Tcl_Obj *p_मूल्य = 0;
    अचिन्हित अक्षर *pBA;
    अचिन्हित अक्षर *p_तथ्य;
    Tcl_आकार len;
    पूर्णांक xrc;
    sqlite3_int64 mx_आकार = 0;
    पूर्णांक i;
    पूर्णांक isReadonly = 0;


    अगर( objc<3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      अवरोध;
    }
    क्रम(i=2; i<objc-1; i++){
      स्थिर अक्षर *z = Tcl_GetString(objv[i]);
      अगर( म_भेद(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        अगर( rc ) जाओ deserialize_error;
        mx_आकार = x;
        जारी;
      }
      अगर( म_भेद(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        अगर( rc ) जाओ deserialize_error;
        जारी;
      }
      अगर( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        जारी;
      }
      Tcl_Append_परिणाम(interp, "unknown option: ", z, (अक्षर*)0);
      rc = TCL_ERROR;
      जाओ deserialize_error;
    }
    p_मूल्य = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_मूल्य, &len);
    p_तथ्य = sqlite3_malloc64( len );
    अगर( p_तथ्य==0 && len>0 ){
      Tcl_Append_परिणाम(interp, "out of memory", (अक्षर*)0);
      rc = TCL_ERROR;
    }अन्यथा{
      पूर्णांक flags;
      अगर( len>0 ) स_नकल(p_तथ्य, pBA, len);
      अगर( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }अन्यथा{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, p_तथ्य, len, len, flags);
      अगर( xrc ){
        Tcl_Append_परिणाम(interp, "unable to set MEMDB content", (अक्षर*)0);
        rc = TCL_ERROR;
      }
      अगर( mx_आकार>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_आकार_LIMIT,&mx_आकार);
      }
    }
deserialize_error:
#पूर्ण_अगर
    अवरोध; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  हाल DB_ENABLE_LOAD_EXTENSION: {
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
    पूर्णांक onoff;
    अगर( objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "BOOLEAN");
      वापस TCL_ERROR;
    }
    अगर( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      वापस TCL_ERROR;
    }
    sqlite3_enable_load_extension(pDb->db, onoff);
    अवरोध;
#अन्यथा
    Tcl_Append_परिणाम(interp, "extension loading is turned off at compile-time",
                     (अक्षर*)0);
    वापस TCL_ERROR;
#पूर्ण_अगर
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  हाल DB_ERRORCODE: {
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    अवरोध;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  हाल DB_ERROROFFSET: {
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(sqlite3_error_offset(pDb->db)));
    अवरोध;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  हाल DB_EXISTS:
  हाल DB_ONECOLUMN: {
    Tcl_Obj *p_परिणाम = 0;
    DbEvalContext sEval;
    अगर( objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "SQL");
      वापस TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = dbEvalStep(&sEval);
    अगर( choice==DB_ONECOLUMN ){
      अगर( rc==TCL_OK ){
        p_परिणाम = db_Eval_Column_मूल्य(&sEval, 0);
      }अन्यथा अगर( rc==TCL_BREAK ){
        Tcl_Reset_परिणाम(interp);
      }
    }अन्यथा अगर( rc==TCL_BREAK || rc==TCL_OK ){
      p_परिणाम = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    अगर( p_परिणाम ) Tcl_Set_Obj_परिणाम(interp, p_परिणाम);

    अगर( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    अवरोध;
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
  हाल DB_EVAL: {
    पूर्णांक evalFlags = 0;
    स्थिर अक्षर *zOpt;
    जबतक( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      अगर( म_भेद(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }अन्यथा अगर( म_भेद(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }अन्यथा{
        Tcl_Append_परिणाम(interp, "unknown option: \"", zOpt, "\"", (व्योम*)0);
        वापस TCL_ERROR;
      }
      objc--;
      objv++;
    }
    अगर( objc<3 || objc>5 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      वापस TCL_ERROR;
    }

    अगर( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_गिनती(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      जबतक( TCL_OK==(rc = dbEvalStep(&sEval)) ){
        पूर्णांक i;
        पूर्णांक nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        क्रम(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_Column_मूल्य(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      अगर( rc==TCL_BREAK ){
        Tcl_Set_Obj_परिणाम(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_गिनती(pRet);
    }अन्यथा{
      Client_तथ्य cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      अगर( objc>=5 && *(अक्षर *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_गिनती(pScript);

      p = (DbEvalContext *)Tcl_Alloc(माप(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (व्योम *)p;
      cd2[1] = (व्योम *)pScript;
      rc = Db_Eval_अग्र_Cmd(cd2, interp, TCL_OK);
    }
    अवरोध;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  हाल DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    अवरोध;
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
  हाल DB_FUNCTION: {
    पूर्णांक flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    अक्षर *zName;
    पूर्णांक nArg = -1;
    पूर्णांक i;
    पूर्णांक eType = SQLITE_NULL;
    अगर( objc<4 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      वापस TCL_ERROR;
    }
    क्रम(i=3; i<(objc-1); i++){
      स्थिर अक्षर *z = Tcl_GetString(objv[i]);
      पूर्णांक n = strlen30(z);
      अगर( n>1 && म_भेदन(z, "-argcount",n)==0 ){
        अगर( i==(objc-2) ){
          Tcl_Append_परिणाम(interp, "option requires an argument: ", z,(अक्षर*)0);
          वापस TCL_ERROR;
        }
        अगर( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) वापस TCL_ERROR;
        अगर( nArg<0 ){
          Tcl_Append_परिणाम(interp, "number of arguments must be non-negative",
                           (अक्षर*)0);
          वापस TCL_ERROR;
        }
        i++;
      }अन्यथा
      अगर( n>1 && म_भेदन(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }अन्यथा
      अगर( n>1 && म_भेदन(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }अन्यथा
      अगर( n>1 && म_भेदन(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }अन्यथा
      अगर( n>1 && म_भेदन(z, "-returntype", n)==0 ){
        स्थिर अक्षर *azType[] = {"integer", "real", "text", "blob", "any", 0};
        निश्चित( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        निश्चित( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        अगर( i==(objc-2) ){
          Tcl_Append_परिणाम(interp, "option requires an argument: ", z,(अक्षर*)0);
          वापस TCL_ERROR;
        }
        i++;
        अगर( Tcl_Get_सूचकांक_From_Obj(interp, objv[i], azType, "type", 0, &eType) ){
          वापस TCL_ERROR;
        }
        eType++;
      }अन्यथा{
        Tcl_Append_परिणाम(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (अक्षर*)0
        );
        वापस TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    अगर( pFunc==0 ) वापस TCL_ERROR;
    अगर( pFunc->pScript ){
      Tcl_Decr_Ref_गिनती(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_गिनती(pScript);
    pFunc->useEvalObjv = safeToUseEvalObjv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    अगर( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    अवरोध;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  हाल DB_INCRBLOB: {
#अगर_घोषित SQLITE_OMIT_INCRBLOB
    Tcl_Append_परिणाम(interp, "incrblob not available in this build", (अक्षर*)0);
    वापस TCL_ERROR;
#अन्यथा
    पूर्णांक isReadonly = 0;
    स्थिर अक्षर *zDb = "main";
    स्थिर अक्षर *zTable;
    स्थिर अक्षर *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    अगर( objc>3 && म_भेद(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    अगर( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      वापस TCL_ERROR;
    }

    अगर( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    zTable = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    अगर( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, zTable, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#पूर्ण_अगर
    अवरोध;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  हाल DB_INTERRUPT: {
    sqlite3_interrupt(pDb->db);
    अवरोध;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  हाल DB_NULLVALUE: {
    अगर( objc!=2 && objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "NULLVALUE");
      वापस TCL_ERROR;
    }
    अगर( objc==3 ){
      Tcl_आकार len;
      अक्षर *zNull = Tcl_GetStringFromObj(objv[2], &len);
      अगर( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      अगर( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }अन्यथा{
        pDb->zNull = 0;
      }
    }
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(pDb->zNull, -1));
    अवरोध;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  हाल DB_LAST_जोड़ो_ROWID: {
    Tcl_Obj *p_परिणाम;
    Tcl_WideInt rowid;
    अगर( objc!=2 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "");
      वापस TCL_ERROR;
    }
    rowid = sqlite3_last_जोड़ो_rowid(pDb->db);
    p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
    Tcl_SetWideIntObj(p_परिणाम, rowid);
    अवरोध;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  हाल DB_PROGRESS: {
    अगर( objc==2 ){
      अगर( pDb->zProgress ){
        Tcl_Append_परिणाम(interp, pDb->zProgress, (अक्षर*)0);
      }
#अगर_अघोषित SQLITE_OMIT_PROGRESS_CALLBACK
      sqlite3_progress_handler(pDb->db, 0, 0, 0);
#पूर्ण_अगर
    }अन्यथा अगर( objc==4 ){
      अक्षर *zProgress;
      Tcl_आकार len;
      पूर्णांक N;
      अगर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        वापस TCL_ERROR;
      };
      अगर( pDb->zProgress ){
        Tcl_Free(pDb->zProgress);
      }
      zProgress = Tcl_GetStringFromObj(objv[3], &len);
      अगर( zProgress && len>0 ){
        pDb->zProgress = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zProgress, zProgress, len+1);
      }अन्यथा{
        pDb->zProgress = 0;
      }
#अगर_अघोषित SQLITE_OMIT_PROGRESS_CALLBACK
      अगर( pDb->zProgress ){
        pDb->interp = interp;
        sqlite3_progress_handler(pDb->db, N, DbProgressHandler, pDb);
      }अन्यथा{
        sqlite3_progress_handler(pDb->db, 0, 0, 0);
      }
#पूर्ण_अगर
    }अन्यथा{
      Tcl_Wrong_अंक_Args(interp, 2, objv, "N CALLBACK");
      वापस TCL_ERROR;
    }
    अवरोध;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  हाल DB_PROFILE: {
    अगर( objc>3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?CALLBACK?");
      वापस TCL_ERROR;
    }अन्यथा अगर( objc==2 ){
      अगर( pDb->zProfile ){
        Tcl_Append_परिणाम(interp, pDb->zProfile, (अक्षर*)0);
      }
    }अन्यथा{
      अक्षर *zProfile;
      Tcl_आकार len;
      अगर( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      अगर( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zProfile, zProfile, len+1);
      }अन्यथा{
        pDb->zProfile = 0;
      }
#अगर !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      अगर( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }अन्यथा{
        sqlite3_profile(pDb->db, 0, 0);
      }
#पूर्ण_अगर
    }
    अवरोध;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  हाल DB_REKEY: {
    अगर( objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "KEY");
      वापस TCL_ERROR;
    }
    अवरोध;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  हाल DB_RESTORE: {
    स्थिर अक्षर *zSrcFile;
    स्थिर अक्षर *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    पूर्णांक nTimeout = 0;

    अगर( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }अन्यथा अगर( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }अन्यथा{
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?DATABASE? FILENAME");
      वापस TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    अगर( rc!=SQLITE_OK ){
      Tcl_Append_परिणाम(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (अक्षर*)0);
      sqlite3_close(pSrc);
      वापस TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    अगर( pBackup==0 ){
      Tcl_Append_परिणाम(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (अक्षर*)0);
      sqlite3_close(pSrc);
      वापस TCL_ERROR;
    }
    जबतक( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      अगर( rc==SQLITE_BUSY ){
        अगर( nTimeout++ >= 3 ) अवरोध;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    अगर( rc==SQLITE_DONE ){
      rc = TCL_OK;
    }अन्यथा अगर( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_Append_परिणाम(interp, "restore failed: source database busy",
                       (अक्षर*)0);
      rc = TCL_ERROR;
    }अन्यथा{
      Tcl_Append_परिणाम(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (अक्षर*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    अवरोध;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  हाल DB_SERIALIZE: {
#अगर_घोषित SQLITE_OMIT_DESERIALIZE
    Tcl_Append_परिणाम(interp, "MEMDB not available in this build",
                     (अक्षर*)0);
    rc = TCL_ERROR;
#अन्यथा
    स्थिर अक्षर *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    अचिन्हित अक्षर *p_तथ्य;
    अगर( objc!=2 && objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }अन्यथा{
      पूर्णांक needFree;
      p_तथ्य = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      अगर( p_तथ्य ){
        needFree = 0;
      }अन्यथा{
        p_तथ्य = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_Set_Obj_परिणाम(interp, Tcl_NewByteArrayObj(p_तथ्य,sz));
      अगर( needFree ) sqlite3_free(p_तथ्य);
    }
#पूर्ण_अगर
    अवरोध;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  हाल DB_STATUS: {
    पूर्णांक v;
    स्थिर अक्षर *zOp;
    अगर( objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "(step|sort|autoindex)");
      वापस TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    अगर( म_भेद(zOp, "step")==0 ){
      v = pDb->nStep;
    }अन्यथा अगर( म_भेद(zOp, "sort")==0 ){
      v = pDb->nSort;
    }अन्यथा अगर( म_भेद(zOp, "autoindex")==0 ){
      v = pDb->n_सूचकांक;
    }अन्यथा अगर( म_भेद(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }अन्यथा{
      Tcl_Append_परिणाम(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (अक्षर*)0);
      वापस TCL_ERROR;
    }
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(v));
    अवरोध;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  हाल DB_TIMEOUT: {
    पूर्णांक ms;
    अगर( objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "MILLISECONDS");
      वापस TCL_ERROR;
    }
    अगर( Tcl_GetIntFromObj(interp, objv[2], &ms) ) वापस TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    अवरोध;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  हाल DB_योग_CHANGES: {
    Tcl_Obj *p_परिणाम;
    अगर( objc!=2 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "");
      वापस TCL_ERROR;
    }
    p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
    Tcl_SetWideIntObj(p_परिणाम, sqlite3_योग_changes64(pDb->db));
    अवरोध;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  हाल DB_TRACE: {
    अगर( objc>3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?CALLBACK?");
      वापस TCL_ERROR;
    }अन्यथा अगर( objc==2 ){
      अगर( pDb->zTrace ){
        Tcl_Append_परिणाम(interp, pDb->zTrace, (अक्षर*)0);
      }
    }अन्यथा{
      अक्षर *zTrace;
      Tcl_आकार len;
      अगर( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      अगर( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zTrace, zTrace, len+1);
      }अन्यथा{
        pDb->zTrace = 0;
      }
#अगर !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      अगर( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }अन्यथा{
        sqlite3_trace(pDb->db, 0, 0);
      }
#पूर्ण_अगर
    }
    अवरोध;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  हाल DB_TRACE_V2: {
    अगर( objc>4 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?CALLBACK? ?MASK?");
      वापस TCL_ERROR;
    }अन्यथा अगर( objc==2 ){
      अगर( pDb->zTraceV2 ){
        Tcl_Append_परिणाम(interp, pDb->zTraceV2, (अक्षर*)0);
      }
    }अन्यथा{
      अक्षर *zTraceV2;
      Tcl_आकार len;
      Tcl_WideInt wMask = 0;
      अगर( objc==4 ){
        अटल स्थिर अक्षर *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        क्रमागत TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_आकार i;
        अगर( TCL_OK!=Tcl_List_Obj_लंबाई(interp, objv[3], &len) ){
          वापस TCL_ERROR;
        }
        क्रम(i=0; i<len; i++){
          Tcl_Obj *pObj;
          पूर्णांक ttype;
          अगर( TCL_OK!=Tcl_List_Obj_सूचकांक(interp, objv[3], i, &pObj) ){
            वापस TCL_ERROR;
          }
          अगर( Tcl_Get_सूचकांक_From_Obj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_Get_Obj_परिणाम(interp));
            Tcl_Incr_Ref_गिनती(pError);
            अगर( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_गिनती(pError);
              wMask |= wType;
            }अन्यथा{
              Tcl_Set_Obj_परिणाम(interp, pError);
              Tcl_Decr_Ref_गिनती(pError);
              वापस TCL_ERROR;
            }
          }अन्यथा{
            चयन( (क्रमागत TTYPE_enum)ttype ){
              हाल TTYPE_STMT:    wMask |= SQLITE_TRACE_STMT;    अवरोध;
              हाल TTYPE_PROFILE: wMask |= SQLITE_TRACE_PROFILE; अवरोध;
              हाल TTYPE_ROW:     wMask |= SQLITE_TRACE_ROW;     अवरोध;
              हाल TTYPE_CLOSE:   wMask |= SQLITE_TRACE_CLOSE;   अवरोध;
            }
          }
        }
      }अन्यथा{
        wMask = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      अगर( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      अगर( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        स_नकल(pDb->zTraceV2, zTraceV2, len+1);
      }अन्यथा{
        pDb->zTraceV2 = 0;
      }
#अगर !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      अगर( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (अचिन्हित)wMask, DbTraceV2Handler, pDb);
      }अन्यथा{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#पूर्ण_अगर
    }
    अवरोध;
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
  हाल DB_TRANSACTION: {
    Tcl_Obj *pScript;
    स्थिर अक्षर *zBegin = "SAVEPOINT _tcl_transaction";
    अगर( objc!=3 && objc!=4 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "[TYPE] SCRIPT");
      वापस TCL_ERROR;
    }

    अगर( pDb->nTransaction==0 && objc==4 ){
      अटल स्थिर अक्षर *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      क्रमागत TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_IMMEDIATE
      };
      पूर्णांक ttype;
      अगर( Tcl_Get_सूचकांक_From_Obj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        वापस TCL_ERROR;
      }
      चयन( (क्रमागत TTYPE_enum)ttype ){
        हाल TTYPE_DEFERRED:    /* no-op */;                 अवरोध;
        हाल TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  अवरोध;
        हाल TTYPE_IMMEDIATE:   zBegin = "BEGIN IMMEDIATE";  अवरोध;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    अगर( rc!=SQLITE_OK ){
      Tcl_Append_परिणाम(interp, sqlite3_errmsg(pDb->db), (अक्षर*)0);
      वापस TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    addDatabaseRef(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    अगर( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (व्योम)Tcl_NREvalObj(interp, pScript, 0);
    }अन्यथा{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    अवरोध;
  }

  /*
  **    $db unlock_notify ?script?
  */
  हाल DB_UNLOCK_NOTIFY: {
#अगर_अघोषित SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_Append_परिणाम(interp, "unlock_notify not available in this build",
                     (अक्षर*)0);
    rc = TCL_ERROR;
#अन्यथा
    अगर( objc!=2 && objc!=3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }अन्यथा{
      व्योम (*xNotify)(व्योम **, पूर्णांक) = 0;
      व्योम *pNotifyArg = 0;

      अगर( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_गिनती(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      अगर( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (व्योम *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_गिनती(pDb->pUnlockNotify);
      }

      अगर( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_Append_परिणाम(interp, sqlite3_errmsg(pDb->db), (अक्षर*)0);
        rc = TCL_ERROR;
      }
    }
#पूर्ण_अगर
    अवरोध;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  हाल DB_PREUPDATE: {
#अगर_अघोषित SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_Append_परिणाम(interp, "preupdate_hook was omitted at compile-time", 
                     (अक्षर*)0);
    rc = TCL_ERROR;
#अन्यथा
    अटल स्थिर अक्षर *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    क्रमागत DbPreupdateSubCmd {
      PRE_गिनती, PRE_DEPTH, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    पूर्णांक iSub;

    अगर( objc<3 ){
      Tcl_Wrong_अंक_Args(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    अगर( Tcl_Get_सूचकांक_From_Obj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      वापस TCL_ERROR;
    }

    चयन( (क्रमागत DbPreupdateSubCmd)iSub ){
      हाल PRE_गिनती: {
        पूर्णांक nCol = sqlite3_preupdate_गिनती(pDb->db);
        Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(nCol));
        अवरोध;
      }

      हाल PRE_HOOK: {
        अगर( objc>4 ){
          Tcl_Wrong_अंक_Args(interp, 2, objv, "hook ?SCRIPT?");
          वापस TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        अवरोध;
      }

      हाल PRE_DEPTH: {
        Tcl_Obj *pRet;
        अगर( objc!=3 ){
          Tcl_Wrong_अंक_Args(interp, 3, objv, "");
          वापस TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_depth(pDb->db));
        Tcl_Set_Obj_परिणाम(interp, pRet);
        अवरोध;
      }

      हाल PRE_NEW:
      हाल PRE_OLD: {
        पूर्णांक iIdx;
        sqlite3_मूल्य *p_मूल्य;
        अगर( objc!=4 ){
          Tcl_Wrong_अंक_Args(interp, 3, objv, "INDEX");
          वापस TCL_ERROR;
        }
        अगर( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          वापस TCL_ERROR;
        }

        अगर( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_मूल्य);
        }अन्यथा{
          निश्चित( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_मूल्य);
        }

        अगर( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((अक्षर*)sqlite3_मूल्य_text(p_मूल्य), -1);
          Tcl_Set_Obj_परिणाम(interp, pObj);
        }अन्यथा{
          Tcl_Append_परिणाम(interp, sqlite3_errmsg(pDb->db), (अक्षर*)0);
          वापस TCL_ERROR;
        }
      }
    }
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */
    अवरोध;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  हाल DB_WAL_HOOK:
  हाल DB_UPDATE_HOOK:
  हाल DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    अगर( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    अगर( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    अगर( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    अगर( objc>3 ){
       Tcl_Wrong_अंक_Args(interp, 2, objv, "?SCRIPT?");
       वापस TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    अवरोध;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  हाल DB_VERSION: {
    पूर्णांक i;
    क्रम(i=2; i<objc; i++){
      स्थिर अक्षर *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#अगर_घोषित SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      अगर( म_भेद(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        अगर( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          वापस TCL_ERROR;
        }
      }अन्यथा

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      अगर( म_भेद(zArg, "-last-stmt-ptr")==0 ){
        बाह्य स्थिर अक्षर *sqlite3TestPtrToText(व्योम*);
        व्योम *ptr = (व्योम*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_Set_परिणाम(interp, (अक्षर*)sqlite3TestPtrToText(ptr), TCL_VOLATILE);
      }अन्यथा
#पूर्ण_अगर /* SQLITE_TEST */
      {
        Tcl_Append_परिणाम(interp, "unknown argument: ", zArg, (अक्षर*)0);
        वापस TCL_ERROR;
      }
    }
    अगर( i==2 ){   
      Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3_libversion(), TCL_STATIC);
    }
    अवरोध;
  }


  } /* End of the SWITCH statement */
  वापस rc;
}

#अगर SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
अटल पूर्णांक SQLITE_TCLAPI DbObjCmdAdaptor(
  व्योम *cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *स्थिर*objv
){
  वापस Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#पूर्ण_अगर /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
अटल पूर्णांक sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *स्थिर*objv
){
  Tcl_Wrong_अंक_Args(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  वापस TCL_ERROR;
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
अटल पूर्णांक SQLITE_TCLAPI DbMain(
  व्योम *cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *स्थिर*objv
){
  SqliteDb *p;
  स्थिर अक्षर *zArg;
  अक्षर *z_Err_संदेश;
  पूर्णांक i;
  स्थिर अक्षर *zFile = 0;
  स्थिर अक्षर *zVfs = 0;
  पूर्णांक flags;
  पूर्णांक bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  पूर्णांक rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#अगर_घोषित SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#अन्यथा
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#पूर्ण_अगर

  अगर( objc==1 ) वापस sqliteCmdUsage(interp, objv);
  अगर( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    अगर( म_भेद(zArg,"-version")==0 ){
      Tcl_Append_परिणाम(interp,sqlite3_libversion(), (अक्षर*)0);
      वापस TCL_OK;
    }
    अगर( म_भेद(zArg,"-sourceid")==0 ){
      Tcl_Append_परिणाम(interp,sqlite3_sourceid(), (अक्षर*)0);
      वापस TCL_OK;
    }
    अगर( म_भेद(zArg,"-has-codec")==0 ){
      Tcl_Append_परिणाम(interp,"0",(अक्षर*)0);
      वापस TCL_OK;
    }
    अगर( zArg[0]=='-' ) वापस sqliteCmdUsage(interp, objv);
  }
  क्रम(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    अगर( zArg[0]!='-' ){
      अगर( zFile!=0 ) वापस sqliteCmdUsage(interp, objv);
      zFile = zArg;
      जारी;
    }
    अगर( i==objc-1 ) वापस sqliteCmdUsage(interp, objv);
    i++;
    अगर( म_भेद(zArg,"-key")==0 ){
      /* no-op */
    }अन्यथा अगर( म_भेद(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }अन्यथा अगर( म_भेद(zArg, "-readonly")==0 ){
      पूर्णांक b;
      अगर( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) वापस TCL_ERROR;
      अगर( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }अन्यथा अगर( म_भेद(zArg, "-create")==0 ){
      पूर्णांक b;
      अगर( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) वापस TCL_ERROR;
      अगर( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }अन्यथा अगर( म_भेद(zArg, "-nofollow")==0 ){
      पूर्णांक b;
      अगर( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) वापस TCL_ERROR;
      अगर( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }अन्यथा अगर( म_भेद(zArg, "-nomutex")==0 ){
      पूर्णांक b;
      अगर( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) वापस TCL_ERROR;
      अगर( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }अन्यथा अगर( म_भेद(zArg, "-fullmutex")==0 ){
      पूर्णांक b;
      अगर( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) वापस TCL_ERROR;
      अगर( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }अन्यथा अगर( म_भेद(zArg, "-uri")==0 ){
      पूर्णांक b;
      अगर( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) वापस TCL_ERROR;
      अगर( b ){
        flags |= SQLITE_OPEN_URI;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_URI;
      }
    }अन्यथा अगर( म_भेद(zArg, "-translatefilename")==0 ){
      अगर( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        वापस TCL_ERROR;
      }
    }अन्यथा{
      Tcl_Append_परिणाम(interp, "unknown option: ", zArg, (अक्षर*)0);
      वापस TCL_ERROR;
    }
  }
  z_Err_संदेश = 0;
  p = (SqliteDb*)Tcl_Alloc( माप(*p) );
  स_रखो(p, 0, माप(*p));
  अगर( zFile==0 ) zFile = "";
  अगर( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  अगर( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  अगर( p->db ){
    अगर( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_संदेश = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }अन्यथा{
    z_Err_संदेश = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  अगर( p->db==0 ){
    Tcl_Set_परिणाम(interp, z_Err_संदेश, TCL_VOLATILE);
    Tcl_Free((अक्षर*)p);
    sqlite3_free(z_Err_संदेश);
    वापस TCL_ERROR;
  }
  p->maxStmt = अंक_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  अगर( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (अक्षर*)p, DbDeleteCmd);
  }अन्यथा{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (अक्षर*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  वापस TCL_OK;
}

/*
** Provide a dummy Tcl_InitStubs if we are using this as a static
** library.
*/
#अगर_अघोषित USE_TCL_STUBS
# undef  Tcl_InitStubs
# define Tcl_InitStubs(a,b,c) TCL_VERSION
#पूर्ण_अगर

/*
** Make sure we have a PACKAGE_VERSION macro defined.  This will be
** defined automatically by the TEA makefile.  But other makefiles
** do not define it.
*/
#अगर_अघोषित PACKAGE_VERSION
# define PACKAGE_VERSION SQLITE_VERSION
#पूर्ण_अगर

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
EXTERN पूर्णांक Sqlite3_Init(Tcl_Interp *interp){
  पूर्णांक rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  अगर( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#अगर_अघोषित SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#पूर्ण_अगर
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_VERSION);
  }
  वापस rc;
}
EXTERN पूर्णांक Tclsqlite3_Init(Tcl_Interp *interp){ वापस Sqlite3_Init(interp); }
EXTERN पूर्णांक Sqlite3_Unload(Tcl_Interp *interp, पूर्णांक flags){ वापस TCL_OK; }
EXTERN पूर्णांक Tclsqlite3_Unload(Tcl_Interp *interp, पूर्णांक flags){ वापस TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN पूर्णांक Sqlite3_SafeInit(Tcl_Interp *interp){ वापस TCL_ERROR; }
EXTERN पूर्णांक Sqlite3_SafeUnload(Tcl_Interp *interp, पूर्णांक flags){वापस TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN पूर्णांक Sqlite_Init(Tcl_Interp *interp){ वापस Sqlite3_Init(interp);}
EXTERN पूर्णांक Tclsqlite_Init(Tcl_Interp *interp){ वापस Sqlite3_Init(interp); }
EXTERN पूर्णांक Sqlite_Unload(Tcl_Interp *interp, पूर्णांक flags){ वापस TCL_OK; }
EXTERN पूर्णांक Tclsqlite_Unload(Tcl_Interp *interp, पूर्णांक flags){ वापस TCL_OK; }
EXTERN पूर्णांक Sqlite_SafeInit(Tcl_Interp *interp){ वापस TCL_ERROR; }
EXTERN पूर्णांक Sqlite_SafeUnload(Tcl_Interp *interp, पूर्णांक flags){वापस TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN पूर्णांक sqlite3_Init(Tcl_Interp *interp){ वापस Sqlite3_Init(interp);}
EXTERN पूर्णांक sqlite_Init(Tcl_Interp *interp){ वापस Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#अगर defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
अटल स्थिर अक्षर *tclsh_main_loop(व्योम){
  अटल स्थिर अक्षर zMainloop[] =
    "if {[llength $argv]>=1} {\n"
#अगर_घोषित WIN32
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
#पूर्ण_अगर
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
  वापस zMainloop;
}

#अगर_अघोषित TCLSH_MAIN
# define TCLSH_MAIN मुख्य
#पूर्ण_अगर
पूर्णांक SQLITE_CDECL TCLSH_MAIN(पूर्णांक argc, अक्षर **argv){
  Tcl_Interp *interp;
  पूर्णांक i;
  स्थिर अक्षर *zScript = 0;
  अक्षर zArgc[32];
#अगर defined(TCLSH_INIT_PROC)
  बाह्य स्थिर अक्षर *TCLSH_INIT_PROC(Tcl_Interp*);
#पूर्ण_अगर

#अगर !defined(_WIN32_WCE)
  अगर( दो_पर्या("SQLITE_DEBUG_BREAK") ){
    अगर( isatty(0) && isatty(2) ){
      ख_लिखो(मानक_त्रुटि,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      ख_अक्षर_लो(मानक_निवेश);
    }अन्यथा{
#अगर defined(_WIN32) || defined(WIN32)
      DebugBreak();
#या_अगर defined(SIGTRAP)
      उठाओ(SIGTRAP);
#पूर्ण_अगर
    }
  }
#पूर्ण_अगर

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_SetSystemEncoding(शून्य, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(माप(zArgc), zArgc, "%d", argc-1);
  Tcl_SetVar(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp,"argv", "", TCL_GLOBAL_ONLY);
  क्रम(i=1; i<argc; i++){
    Tcl_SetVar(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_मूल्य);
  }
#अगर defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#पूर्ण_अगर
  अगर( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  अगर( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    स्थिर अक्षर *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    अगर( zInfo==0 ) zInfo = Tcl_Get_String_परिणाम(interp);
    ख_लिखो(मानक_त्रुटि,"%s: %s\n", *argv, zInfo);
    वापस 1;
  }
  वापस 0;
}
#पूर्ण_अगर /* TCLSH */
