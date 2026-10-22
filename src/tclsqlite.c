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
#यदि_परिभाषितम् TCLSH_INIT_PROC
# define TCLSH 1
#यदि_अन्तः

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#यदि defined(INCLUDE_MSVC_H)
# include "msvc.h"
#यदि_अन्तः

/****** Copy of tclsqlite.h ******/
#यदि defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#अन्यथा
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#यदि_अन्तः
/* Compatability between Tcl8.6 and Tcl9.0 */
#यदि TCL_MAJOR_संस्करणम्==9
# define CONST ध्रुव
#नोचेद्यदि !defined(Tcl_आकार)
  प्रकारनिर्देश पूर्णाङ्क Tcl_आकार;
# ifndef Tcl_Bounce_Ref_गणनम्
#  define Tcl_Bounce_Ref_गणनम्(X) Tcl_Incr_Ref_गणनम्(X); Tcl_Decr_Ref_गणनम्(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#यदि_अन्तः
/**** End copy of tclsqlite.h ****/

#अन्तर्भाव <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#यदि_अपरिभाषितम् SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  प्रकारनिर्देश अचिह्नित वर्ण u8;
# ifndef SQLITE_PTRSIZE
#   यदि defined(__SIZEOF_POINTER__)
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
# यदि defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    प्रकारनिर्देश uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    प्रकारनिर्देश अचिह्नित पूर्णाङ्क uptr;
# अन्यथा
    प्रकारनिर्देश sqlite3_uint64 uptr;
# endif
#यदि_अन्तः
#अन्तर्भाव <ctype.h>

/* Used to get the current process ID */
#यदि !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#नोचेद्यदि !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_माध्यम्
#   define WIN32_LEAN_AND_माध्यम्
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (पूर्णाङ्क)GetCurrentProcessId
#यदि_अन्तः

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#यदि_परिभाषितम् BUILD_sqlite
#अपरिभाषय TCL_STORAGE_CLASS
#परिभाषय TCL_STORAGE_CLASS DLLEXPORT
#यदि_अन्तः /* BUILD_sqlite */

#परिभाषय अङ्कः_PREPARED_STMTS 10
#परिभाषय MAX_PREPARED_STMTS 100

/* Forward declaration */
प्रकारनिर्देश संरचना SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#यदि defined(SQLITE_ENABLE_QRF_अन्तर्_TCL)
#अन्तर्भाव "qrf.h"
#यदि_अन्तः

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
प्रकारनिर्देश संरचना SqlFunc SqlFunc;
संरचना SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  पूर्णाङ्क useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  पूर्णाङ्क eType;            /* Type of value to return */
  वर्ण *zName;          /* Name of this function */
  SqlFunc *p_अग्र;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
प्रकारनिर्देश संरचना SqlCollate SqlCollate;
संरचना SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  वर्ण *zScript;        /* The script to be run */
  SqlCollate *p_अग्र;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
प्रकारनिर्देश संरचना SqlPreparedStmt SqlPreparedStmt;
संरचना SqlPreparedStmt {
  SqlPreparedStmt *p_अग्र;  /* Next in linked list */
  SqlPreparedStmt *p_पूर्व;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  पूर्णाङ्क nSql;                /* chars in zSql[] */
  ध्रुव वर्ण *zSql;        /* Text of the SQL statement */
  पूर्णाङ्क nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

प्रकारनिर्देश संरचना IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
संरचना SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  वर्ण *zBusy;               /* The busy callback routine */
  वर्ण *zCommit;             /* The commit hook callback routine */
  वर्ण *zTrace;              /* The trace callback routine */
  वर्ण *zTraceV2;            /* The trace_v2 callback routine */
  वर्ण *zProfile;            /* The profile callback routine */
  वर्ण *z_प्रगतिः;           /* The progress callback routine */
  वर्ण *zBindFallback;       /* Callback to invoke on a binding miss */
  वर्ण *zAuth;               /* The authorization callback routine */
  पूर्णाङ्क disableAuth;           /* Disable the authorizer if it exists */
  वर्ण *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  पूर्णाङ्क rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmt_चरमः; /* Last statement in the list */
  पूर्णाङ्क maxStmt;               /* The next maximum number of stmtList */
  पूर्णाङ्क nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  पूर्णाङ्क n_चरणः, n_क्रमय, n_सूचकांक;  /* Statistics for most recent operation */
  पूर्णाङ्क nVMStep;               /* Another statistic for most recent operation */
  पूर्णाङ्क nTransaction;          /* Number of nested [transaction] methods */
  पूर्णाङ्क openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  पूर्णाङ्क nRef;                  /* Delete object when this reaches 0 */
#यदि_परिभाषितम् SQLITE_TEST
  पूर्णाङ्क bLegacyPrepare;        /* True to use sqlite3_prepare() */
#यदि_अन्तः
};

संरचना IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  अचिह्नित पूर्णाङ्क isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *p_अग्र;   /* Linked list of all open incrblob channels */
  IncrblobChannel *p_पूर्व;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
स्थिर पूर्णाङ्क strlen30(ध्रुव वर्ण *z){
  ध्रुव वर्ण *z2 = z;
  यावत्( *z2 ){ z2++; }
  प्रत्यागम 0x3fffffff & (पूर्णाङ्क)(z2 - z);
}


#यदि_अपरिभाषितम् SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
स्थिर शून्य closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *p_अग्र;

  प्रति(p=pDb->pIncrblob; p; p=p_अग्र){
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI incrblobClose2(
  Client_दत्त instance_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क flags
){
  IncrblobChannel *p = (IncrblobChannel *)instance_दत्त;
  पूर्णाङ्क  rc;
  sqlite3 *db = p->pDb->db;

  यदि( flags ){
    p->isClosed |= flags;
    प्रत्यागम TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  यदि( p->p_अग्र ){
    p->p_अग्र->p_पूर्व = p->p_पूर्व;
  }
  यदि( p->p_पूर्व ){
    p->p_पूर्व->p_अग्र = p->p_अग्र;
  }
  यदि( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->p_अग्र;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((वर्ण *)p);

  यदि( rc!=SQLITE_OK ){
    Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3_errmsg(db), TCL_VOLATILE);
    प्रत्यागम TCL_ERROR;
  }
  प्रत्यागम TCL_OK;
}
स्थिर पूर्णाङ्क SQLITE_TCLAPI incrblobClose(
  Client_दत्त instance_दत्त,
  Tcl_Interp *interp
){
  प्रत्यागम incrblobClose2(instance_दत्त, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI incrblob_निवेशः(
  Client_दत्त instance_दत्त,
  वर्ण *buf,
  पूर्णाङ्क buf_आकार,
  पूर्णाङ्क *error_संहिता_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_दत्त;
  sqlite3_int64 nRead = buf_आकार;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  पूर्णाङ्क rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_अष्टकानि(p->pBlob);
  यदि( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  यदि( nRead<=0 ){
    प्रत्यागम 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (शून्य *)buf, (पूर्णाङ्क)nRead, (पूर्णाङ्क)p->iSeek);
  यदि( rc!=SQLITE_OK ){
    *error_संहिता_Ptr = rc;
    प्रत्यागम -1;
  }

  p->iSeek += nRead;
  प्रत्यागम nRead;
}

/*
** Write data to an incremental blob channel.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI incrblobOutput(
  Client_दत्त instance_दत्त,
  ध्रुव वर्ण *buf,
  पूर्णाङ्क गन्तव्यम्_Write,
  पूर्णाङ्क *error_संहिता_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_दत्त;
  sqlite3_int64 nWrite = गन्तव्यम्_Write;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  पूर्णाङ्क rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_अष्टकानि(p->pBlob);
  यदि( (p->iSeek+nWrite)>nBlob ){
    *error_संहिता_Ptr = EINVAL;
    प्रत्यागम -1;
  }
  यदि( nWrite<=0 ){
    प्रत्यागम 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (शून्य*)buf,(पूर्णाङ्क)nWrite, (पूर्णाङ्क)p->iSeek);
  यदि( rc!=SQLITE_OK ){
    *error_संहिता_Ptr = EIO;
    प्रत्यागम -1;
  }

  p->iSeek += nWrite;
  प्रत्यागम nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#यदि TCL_MAJOR_संस्करणम्==9
# define WideSeekProcType दीर्घ दीर्घ
#अन्यथा
# define WideSeekProcType Tcl_WideInt
#यदि_अन्तः

/*
** Seek an incremental blob channel.
*/
स्थिर WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  Client_दत्त instance_दत्त,
  WideSeekProcType विस्थापनम्,
  पूर्णाङ्क seek_बहुलकम्,
  पूर्णाङ्क *error_संहिता_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_दत्त;

  विकल्प( seek_बहुलकम् ){
    पक्ष SEEK_SET:
      p->iSeek = विस्थापनम्;
      भङ्ग;
    पक्ष SEEK_CUR:
      p->iSeek += विस्थापनम्;
      भङ्ग;
    पक्ष SEEK_END:
      p->iSeek = sqlite3_blob_अष्टकानि(p->pBlob) + विस्थापनम्;
      भङ्ग;

    स्वभाव: प्रतिज्ञा(!"Bad seekMode");
  }

  प्रत्यागम p->iSeek;
}
स्थिर पूर्णाङ्क SQLITE_TCLAPI incrblobSeek(
  Client_दत्त instance_दत्त,
  दीर्घ विस्थापनम्,
  पूर्णाङ्क seek_बहुलकम्,
  पूर्णाङ्क *error_संहिता_Ptr
){
  प्रत्यागम incrblobWideSeek(instance_दत्त,विस्थापनम्,seek_बहुलकम्,error_संहिता_Ptr);
}


स्थिर शून्य SQLITE_TCLAPI incrblobWatch(
  Client_दत्त instance_दत्त,
  पूर्णाङ्क बहुलकम्
){
  /* NO-OP */
}
स्थिर पूर्णाङ्क SQLITE_TCLAPI incrblobHandle(
  Client_दत्त instance_दत्त,
  पूर्णाङ्क dir,
  Client_दत्त *hPtr
){
  प्रत्यागम TCL_ERROR;
}

स्थिर Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_संस्करणम्_5,             /*संस्करणम्*/
  incrblobClose,                     /* closeProc                            */
  incrblob_निवेशः,                     /* inputProc                            */
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
स्थिर पूर्णाङ्क createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  ध्रुव वर्ण *zDb,
  ध्रुव वर्ण *z_सारणी,
  ध्रुव वर्ण *zColumn,
  sqlite_int64 iRow,
  पूर्णाङ्क isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  पूर्णाङ्क rc;
  पूर्णाङ्क flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  स्थिर पूर्णाङ्क गणनम् = 0;
  वर्ण zChannel[64];

  rc = sqlite3_blob_open(db, zDb, z_सारणी, zColumn, iRow, !isReadonly, &pBlob);
  यदि( rc!=SQLITE_OK ){
    Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    प्रत्यागम TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(परिमाण(IncrblobChannel));
  स्मृतिं_पूरय(p, 0, परिमाण(*p));
  p->pBlob = pBlob;
  यदि( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(परिमाण(zChannel), zChannel, "incrblob_%d", ++गणनम्);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_पञ्जिकाकोशः_Channel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->p_अग्र = pDb->pIncrblob;
  p->p_पूर्व = 0;
  यदि( p->p_अग्र ){
    p->p_अग्र->p_पूर्व = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  प्रत्यागम TCL_OK;
}
#अन्यथा  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
  #परिभाषय closeIncrblobChannels(pDb)
#यदि_अन्तः

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
स्थिर पूर्णाङ्क safe_गन्तव्यम्_Use_Eval_Objv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  ध्रुव वर्ण *z;
  Tcl_आकार n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  यावत्( n-- > 0 ){
    पूर्णाङ्क c = *(z++);
    यदि( c=='$' || c=='[' || c==';' ) प्रत्यागम 0;
  }
  प्रत्यागम 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
स्थिर SqlFunc *findSqlFunc(SqliteDb *pDb, ध्रुव वर्ण *zName){
  SqlFunc *p, *pNew;
  पूर्णाङ्क nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( परिमाण(*pNew) + nName + 1 );
  pNew->zName = (वर्ण*)&pNew[1];
  स्मृतिं_प्रतिलिख(pNew->zName, zName, nName+1);
  प्रति(p=pDb->pFunc; p; p=p->p_अग्र){
    यदि( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((वर्ण*)pNew);
      प्रत्यागम p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->p_अग्र = pDb->pFunc;
  pDb->pFunc = pNew;
  प्रत्यागम pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
स्थिर शून्य dbFreeStmt(SqlPreparedStmt *pStmt){
#यदि_परिभाषितम् SQLITE_TEST
  यदि( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((वर्ण *)pStmt->zSql);
  }
#यदि_अन्तः
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((वर्ण *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
स्थिर शून्य flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *p_अग्र;

  प्रति(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=p_अग्र){
    p_अग्र = pPreStmt->p_अग्र;
    dbFreeStmt(pPreStmt);
  }
  pDb->nStmt = 0;
  pDb->stmt_चरमः = 0;
  pDb->stmtList = 0;
}

/*
** Increment the reference counter on the SqliteDb object. The reference
** should be released by calling delDatabaseRef().
*/
स्थिर शून्य सङ्कलय_Database_Ref(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
स्थिर शून्य delDatabaseRef(SqliteDb *pDb){
  प्रतिज्ञा( pDb->nRef>0 );
  pDb->nRef--;
  यदि( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    यावत्( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->p_अग्र;
      प्रतिज्ञा( pFunc->pDb==pDb );
      Tcl_Decr_Ref_गणनम्(pFunc->pScript);
      Tcl_Free((वर्ण*)pFunc);
    }
    यावत्( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->p_अग्र;
      Tcl_Free((वर्ण*)pCollate);
    }
    यदि( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    यदि( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    यदि( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    यदि( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    यदि( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    यदि( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    यदि( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    यदि( pDb->pUpdateHook ){
      Tcl_Decr_Ref_गणनम्(pDb->pUpdateHook);
    }
    यदि( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_गणनम्(pDb->pPreUpdateHook);
    }
    यदि( pDb->pRollbackHook ){
      Tcl_Decr_Ref_गणनम्(pDb->pRollbackHook);
    }
    यदि( pDb->pWalHook ){
      Tcl_Decr_Ref_गणनम्(pDb->pWalHook);
    }
    यदि( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_गणनम्(pDb->pCollateNeeded);
    }
    Tcl_Free((वर्ण*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
स्थिर शून्य SQLITE_TCLAPI DbDeleteCmd(शून्य *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
स्थिर पूर्णाङ्क DbBusyHandler(शून्य *cd, पूर्णाङ्क nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  पूर्णाङ्क rc;
  वर्ण zVal[30];

  sqlite3_snprintf(परिमाण(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (वर्ण*)0);
  यदि( rc!=TCL_OK || atoi(Tcl_Get_String_परिणाम(pDb->interp)) ){
    प्रत्यागम 0;
  }
  प्रत्यागम 1;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_प्रगतिः_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
स्थिर पूर्णाङ्क Db_प्रगतिः_Handler(शून्य *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  पूर्णाङ्क rc;

  प्रतिज्ञा( pDb->z_प्रगतिः );
  rc = Tcl_Eval(pDb->interp, pDb->z_प्रगतिः);
  यदि( rc!=TCL_OK || atoi(Tcl_Get_String_परिणाम(pDb->interp)) ){
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}
#यदि_अन्तः

#यदि !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
स्थिर शून्य DbTraceHandler(शून्य *cd, ध्रुव वर्ण *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_मानम्(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_परिणाम(pDb->interp);
}
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
स्थिर पूर्णाङ्क DbTraceV2Handler(
  अचिह्नित type, /* One of the SQLITE_TRACE_* event types. */
  शून्य *cd,      /* The original context data pointer. */
  शून्य *pd,      /* Primary event data, depends on event type. */
  शून्य *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  विकल्प( type ){
    पक्ष SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      वर्ण *zSql = (वर्ण *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_गणनम्(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
      Tcl_Decr_Ref_गणनम्(pCmd);
      Tcl_Reset_परिणाम(pDb->interp);
      भङ्ग;
    }
    पक्ष SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_गणनम्(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
      Tcl_Decr_Ref_गणनम्(pCmd);
      Tcl_Reset_परिणाम(pDb->interp);
      भङ्ग;
    }
    पक्ष SQLITE_TRACE_ROW: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_गणनम्(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
      Tcl_Decr_Ref_गणनम्(pCmd);
      Tcl_Reset_परिणाम(pDb->interp);
      भङ्ग;
    }
    पक्ष SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_गणनम्(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
      Tcl_Decr_Ref_गणनम्(pCmd);
      Tcl_Reset_परिणाम(pDb->interp);
      भङ्ग;
    }
  }
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः

#यदि !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
स्थिर शून्य DbProfileHandler(शून्य *cd, ध्रुव वर्ण *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  वर्ण zTm[100];

  sqlite3_snprintf(परिमाण(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_मानम्(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_परिणाम(pDb->interp);
}
#यदि_अन्तः

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
स्थिर पूर्णाङ्क DbCommitHandler(शून्य *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  पूर्णाङ्क rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  यदि( rc!=TCL_OK || atoi(Tcl_Get_String_परिणाम(pDb->interp)) ){
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

स्थिर शून्य DbRollbackHandler(शून्य *client_दत्त){
  SqliteDb *pDb = (SqliteDb*)client_दत्त;
  प्रतिज्ञा(pDb->pRollbackHook);
  यदि( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
स्थिर पूर्णाङ्क DbWalHandler(
  शून्य *client_दत्त,
  sqlite3 *db,
  ध्रुव वर्ण *zDb,
  पूर्णाङ्क n_प्रविष्टिः
){
  पूर्णाङ्क ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)client_दत्त;
  Tcl_Interp *interp = pDb->interp;
  प्रतिज्ञा(pDb->pWalHook);

  प्रतिज्ञा( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_गणनम्(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(n_प्रविष्टिः));
  यदि( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_Get_Obj_परिणाम(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_गणनम्(p);

  प्रत्यागम ret;
}

#यदि defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
स्थिर शून्य समुच्चयः_Test_Unlock_Notify_Vars(Tcl_Interp *interp, पूर्णाङ्क iArg, पूर्णाङ्क nArg){
  वर्ण zBuf[64];
  sqlite3_snprintf(परिमाण(zBuf), zBuf, "%d", iArg);
  Tcl_समुच्चयः_Var(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(परिमाण(zBuf), zBuf, "%d", nArg);
  Tcl_समुच्चयः_Var(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#अन्यथा
# define समुच्चयः_Test_Unlock_Notify_Vars(x,y,z)
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_UNLOCK_NOTIFY
स्थिर शून्य DbUnlockNotify(शून्य **apArg, पूर्णाङ्क nArg){
  पूर्णाङ्क i;
  प्रति(i=0; i<nArg; i++){
    ध्रुव पूर्णाङ्क flags = (TCL_EVAL_GLOBAL|TCL_EVAL_प्रत्यक्षम्);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    समुच्चयः_Test_Unlock_Notify_Vars(pDb->interp, i, nArg);
    प्रतिज्ञा( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_गणनम्(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
स्थिर शून्य DbPreUpdateHandler(
  शून्य *p,
  sqlite3 *db,
  पूर्णाङ्क op,
  ध्रुव वर्ण *zDb,
  ध्रुव वर्ण *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  स्थिर ध्रुव वर्ण *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  प्रतिज्ञा( (SQLITE_DELETE-1)/9 == 0 );
  प्रतिज्ञा( (SQLITE_निवेशय-1)/9 == 1 );
  प्रतिज्ञा( (SQLITE_UPDATE-1)/9 == 2 );
  प्रतिज्ञा( pDb->pPreUpdateHook );
  प्रतिज्ञा( db==pDb->db );
  प्रतिज्ञा( op==SQLITE_निवेशय || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_गणनम्(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
  Tcl_Decr_Ref_गणनम्(pCmd);
}
#यदि_अन्तः /* SQLITE_ENABLE_PREUPDATE_HOOK */

स्थिर शून्य DbUpdateHandler(
  शून्य *p,
  पूर्णाङ्क op,
  ध्रुव वर्ण *zDb,
  ध्रुव वर्ण *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  स्थिर ध्रुव वर्ण *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  प्रतिज्ञा( (SQLITE_DELETE-1)/9 == 0 );
  प्रतिज्ञा( (SQLITE_निवेशय-1)/9 == 1 );
  प्रतिज्ञा( (SQLITE_UPDATE-1)/9 == 2 );

  प्रतिज्ञा( pDb->pUpdateHook );
  प्रतिज्ञा( op==SQLITE_निवेशय || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_गणनम्(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
  Tcl_Decr_Ref_गणनम्(pCmd);
}

स्थिर शून्य tclCollateNeeded(
  शून्य *pCtx,
  sqlite3 *db,
  पूर्णाङ्क enc,
  ध्रुव वर्ण *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_गणनम्(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_गणनम्(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
स्थिर पूर्णाङ्क tclSqlCollate(
  शून्य *pCtx,
  पूर्णाङ्क nA,
  ध्रुव शून्य *zA,
  पूर्णाङ्क nB,
  ध्रुव शून्य *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_गणनम्(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
  Tcl_Decr_Ref_गणनम्(pCmd);
  प्रत्यागम (atoi(Tcl_Get_String_परिणाम(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
स्थिर शून्य tclSqlFunc(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम्**argv){
  SqlFunc *p = sqlite3_user_दत्त(context);
  Tcl_Obj *pCmd;
  पूर्णाङ्क i;
  पूर्णाङ्क rc;

  यदि( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_गणनम्(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_गणनम्(pCmd);
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
    यदि( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_परिणाम_error(context, Tcl_Get_String_परिणाम(p->interp), -1);
      प्रत्यागम;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_गणनम्(pCmd);
    प्रति(i=0; i<argc; i++){
      sqlite3_मानम् *p_अन्तर् = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      विकल्प( sqlite3_मानम्_type(p_अन्तर्) ){
        पक्ष SQLITE_BLOB: {
          पूर्णाङ्क अष्टकानि = sqlite3_मानम्_अष्टकानि(p_अन्तर्);
          pVal = Tcl_NewByteArrayObj(sqlite3_मानम्_blob(p_अन्तर्), अष्टकानि);
          भङ्ग;
        }
        पक्ष SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_मानम्_int64(p_अन्तर्);
          यदि( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((पूर्णाङ्क)v);
          }अन्यथा{
            pVal = Tcl_NewWideIntObj(v);
          }
          भङ्ग;
        }
        पक्ष SQLITE_FLOAT: {
          द्विगुण r = sqlite3_मानम्_double(p_अन्तर्);
          pVal = Tcl_NewDoubleObj(r);
          भङ्ग;
        }
        पक्ष SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          भङ्ग;
        }
        स्वभाव: {
          पूर्णाङ्क अष्टकानि = sqlite3_मानम्_अष्टकानि(p_अन्तर्);
          pVal = Tcl_NewStringObj((वर्ण *)sqlite3_मानम्_text(p_अन्तर्), अष्टकानि);
          भङ्ग;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      यदि( rc ){
        Tcl_Decr_Ref_गणनम्(pCmd);
        sqlite3_परिणाम_error(context, Tcl_Get_String_परिणाम(p->interp), -1);
        प्रत्यागम;
      }
    }
    यदि( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
    Tcl_Decr_Ref_गणनम्(pCmd);
  }

  यदि( TCL_BREAK==rc ){
    sqlite3_परिणाम_null(context);
  }अन्यथा यदि( rc && rc!=TCL_RETURN ){
    sqlite3_परिणाम_error(context, Tcl_Get_String_परिणाम(p->interp), -1);
  }अन्यथा{
    Tcl_Obj *pVar = Tcl_Get_Obj_परिणाम(p->interp);
    Tcl_आकार n;
    u8 *दत्त;
    ध्रुव वर्ण *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    वर्ण c = zType[0];
    पूर्णाङ्क eType = p->eType;

    यदि( eType==SQLITE_NULL ){
      यदि( c=='b' && वाक्यं_तुलय(zType,"bytearray")==0 && pVar->अष्टकानि==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }अन्यथा यदि( (c=='b' && pVar->अष्टकानि==0 && वाक्यं_तुलय(zType,"boolean")==0 )
             || (c=='b' && pVar->अष्टकानि==0 && वाक्यं_तुलय(zType,"booleanString")==0 )
             || (c=='w' && वाक्यं_तुलय(zType,"wideInt")==0)
             || (c=='i' && वाक्यं_तुलय(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }अन्यथा यदि( c=='d' && वाक्यं_तुलय(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }अन्यथा{
        eType = SQLITE_TEXT;
      }
    }

    विकल्प( eType ){
      पक्ष SQLITE_BLOB: {
        दत्त = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_परिणाम_blob64(context, दत्त, n, SQLITE_TRANSIENT);
        भङ्ग;
      }
      पक्ष SQLITE_INTEGER: {
        Tcl_WideInt v;
        यदि( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_परिणाम_int64(context, v);
          भङ्ग;
        }
        /* fall-through */
      }
      पक्ष SQLITE_FLOAT: {
        द्विगुण r;
        यदि( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_परिणाम_double(context, r);
          भङ्ग;
        }
        /* fall-through */
      }
      स्वभाव: {
        दत्त = (अचिह्नित वर्ण *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_परिणाम_text64(context, (वर्ण *)दत्त, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        भङ्ग;
      }
    }

  }
}

#यदि_अपरिभाषितम् SQLITE_OMIT_AUTHORIZATION
/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
स्थिर पूर्णाङ्क auth_callback(
  शून्य *pArg,
  पूर्णाङ्क संहिता,
  ध्रुव वर्ण *zArg1,
  ध्रुव वर्ण *zArg2,
  ध्रुव वर्ण *zArg3,
  ध्रुव वर्ण *zArg4
){
  ध्रुव वर्ण *z_संहिता;
  Tcl_DString str;
  पूर्णाङ्क rc;
  ध्रुव वर्ण *z_प्रत्युत्तरम्;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  यदि( pDb->disableAuth ) प्रत्यागम SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  विकल्प( संहिता ){
    पक्ष SQLITE_COPY              : z_संहिता="SQLITE_COPY"; भङ्ग;
    पक्ष SQLITE_CREATE_सूचकांक      : z_संहिता="SQLITE_CREATE_INDEX"; भङ्ग;
    पक्ष SQLITE_CREATE_सारणी      : z_संहिता="SQLITE_CREATE_TABLE"; भङ्ग;
    पक्ष SQLITE_CREATE_अस्थायी_सूचकांक : z_संहिता="SQLITE_CREATE_TEMP_INDEX"; भङ्ग;
    पक्ष SQLITE_CREATE_अस्थायी_सारणी : z_संहिता="SQLITE_CREATE_TEMP_TABLE"; भङ्ग;
    पक्ष SQLITE_CREATE_अस्थायी_TRIGGER: z_संहिता="SQLITE_CREATE_TEMP_TRIGGER"; भङ्ग;
    पक्ष SQLITE_CREATE_अस्थायी_VIEW  : z_संहिता="SQLITE_CREATE_TEMP_VIEW"; भङ्ग;
    पक्ष SQLITE_CREATE_TRIGGER    : z_संहिता="SQLITE_CREATE_TRIGGER"; भङ्ग;
    पक्ष SQLITE_CREATE_VIEW       : z_संहिता="SQLITE_CREATE_VIEW"; भङ्ग;
    पक्ष SQLITE_DELETE            : z_संहिता="SQLITE_DELETE"; भङ्ग;
    पक्ष SQLITE_DROP_सूचकांक        : z_संहिता="SQLITE_DROP_INDEX"; भङ्ग;
    पक्ष SQLITE_DROP_सारणी        : z_संहिता="SQLITE_DROP_TABLE"; भङ्ग;
    पक्ष SQLITE_DROP_अस्थायी_सूचकांक   : z_संहिता="SQLITE_DROP_TEMP_INDEX"; भङ्ग;
    पक्ष SQLITE_DROP_अस्थायी_सारणी   : z_संहिता="SQLITE_DROP_TEMP_TABLE"; भङ्ग;
    पक्ष SQLITE_DROP_अस्थायी_TRIGGER : z_संहिता="SQLITE_DROP_TEMP_TRIGGER"; भङ्ग;
    पक्ष SQLITE_DROP_अस्थायी_VIEW    : z_संहिता="SQLITE_DROP_TEMP_VIEW"; भङ्ग;
    पक्ष SQLITE_DROP_TRIGGER      : z_संहिता="SQLITE_DROP_TRIGGER"; भङ्ग;
    पक्ष SQLITE_DROP_VIEW         : z_संहिता="SQLITE_DROP_VIEW"; भङ्ग;
    पक्ष SQLITE_निवेशय            : z_संहिता="SQLITE_INSERT"; भङ्ग;
    पक्ष SQLITE_PRAGMA            : z_संहिता="SQLITE_PRAGMA"; भङ्ग;
    पक्ष SQLITE_READ              : z_संहिता="SQLITE_READ"; भङ्ग;
    पक्ष SQLITE_SELECT            : z_संहिता="SQLITE_SELECT"; भङ्ग;
    पक्ष SQLITE_TRANSACTION       : z_संहिता="SQLITE_TRANSACTION"; भङ्ग;
    पक्ष SQLITE_UPDATE            : z_संहिता="SQLITE_UPDATE"; भङ्ग;
    पक्ष SQLITE_ATTACH            : z_संहिता="SQLITE_ATTACH"; भङ्ग;
    पक्ष SQLITE_DETACH            : z_संहिता="SQLITE_DETACH"; भङ्ग;
    पक्ष SQLITE_ALTER_सारणी       : z_संहिता="SQLITE_ALTER_TABLE"; भङ्ग;
    पक्ष SQLITE_REINDEX           : z_संहिता="SQLITE_REINDEX"; भङ्ग;
    पक्ष SQLITE_ANALYZE           : z_संहिता="SQLITE_ANALYZE"; भङ्ग;
    पक्ष SQLITE_CREATE_VTABLE     : z_संहिता="SQLITE_CREATE_VTABLE"; भङ्ग;
    पक्ष SQLITE_DROP_VTABLE       : z_संहिता="SQLITE_DROP_VTABLE"; भङ्ग;
    पक्ष SQLITE_FUNCTION          : z_संहिता="SQLITE_FUNCTION"; भङ्ग;
    पक्ष SQLITE_SAVEPOINT         : z_संहिता="SQLITE_SAVEPOINT"; भङ्ग;
    पक्ष SQLITE_RECURSIVE         : z_संहिता="SQLITE_RECURSIVE"; भङ्ग;
    स्वभाव                       : z_संहिता="????"; भङ्ग;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, z_संहिता);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_मानम्(&str));
  Tcl_DStringFree(&str);
  z_प्रत्युत्तरम् = rc==TCL_OK ? Tcl_Get_String_परिणाम(pDb->interp) : "SQLITE_DENY";
  यदि( वाक्यं_तुलय(z_प्रत्युत्तरम्,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }अन्यथा यदि( वाक्यं_तुलय(z_प्रत्युत्तरम्,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }अन्यथा यदि( वाक्यं_तुलय(z_प्रत्युत्तरम्,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }अन्यथा{
    rc = 999;
  }
  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_OMIT_AUTHORIZATION */

#यदि 0
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
स्थिर वर्ण *local_getline(वर्ण *zPrompt, FILE *अन्तर्){
  वर्ण *z_पङ्क्तिः;
  पूर्णाङ्क n_पङ्क्तिः;
  पूर्णाङ्क n;

  n_पङ्क्तिः = 100;
  z_पङ्क्तिः = स्मृतिं_देहि( n_पङ्क्तिः );
  यदि( z_पङ्क्तिः==0 ) प्रत्यागम 0;
  n = 0;
  यावत्( 1 ){
    यदि( n+100>n_पङ्क्तिः ){
      n_पङ्क्तिः = n_पङ्क्तिः*2 + 100;
      z_पङ्क्तिः = realloc(z_पङ्क्तिः, n_पङ्क्तिः);
      यदि( z_पङ्क्तिः==0 ) प्रत्यागम 0;
    }
    यदि( fgets(&z_पङ्क्तिः[n], n_पङ्क्तिः - n, अन्तर्)==0 ){
      यदि( n==0 ){
        मुक्त(z_पङ्क्तिः);
        प्रत्यागम 0;
      }
      z_पङ्क्तिः[n] = 0;
      भङ्ग;
    }
    यावत्( z_पङ्क्तिः[n] ){ n++; }
    यदि( n>0 && z_पङ्क्तिः[n-1]=='\n' ){
      n--;
      z_पङ्क्तिः[n] = 0;
      भङ्ग;
    }
  }
  z_पङ्क्तिः = realloc( z_पङ्क्तिः, n+1 );
  प्रत्यागम z_पङ्क्तिः;
}
#यदि_अन्तः


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI DbTransPostCmd(
  Client_दत्त दत्त[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  पूर्णाङ्क परिणाम                           /* Result of evaluating SCRIPT */
){
  स्थिर ध्रुव वर्ण *ध्रुव az_अन्तः[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)दत्त[0];
  पूर्णाङ्क rc = परिणाम;
  ध्रुव वर्ण *z_अन्तः;

  pDb->nTransaction--;
  z_अन्तः = az_अन्तः[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  यदि( sqlite3_exec(pDb->db, z_अन्तः, 0, 0, 0) ){
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
    यदि( rc!=TCL_ERROR ){
      Tcl_Append_परिणाम(interp, sqlite3_errmsg(pDb->db), (वर्ण*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  प्रत्यागम rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
स्थिर पूर्णाङ्क dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  ध्रुव वर्ण *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  ध्रुव वर्ण **pz_बहिर्गमनम्              /* OUT: Pointer to next SQL statement */
){
  अचिह्नित पूर्णाङ्क prepFlags = 0;
#यदि_परिभाषितम् SQLITE_TEST
  यदि( pDb->bLegacyPrepare ){
    प्रत्यागम sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pz_बहिर्गमनम्);
  }
#यदि_अन्तः
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  यदि( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  प्रत्यागम sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pz_बहिर्गमनम्);
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
स्थिर पूर्णाङ्क dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  वर्ण ध्रुव *z_अन्तर्,                /* SQL to compile */
  वर्ण ध्रुव **pz_बहिर्गमनम्,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  ध्रुव वर्ण *zSql = z_अन्तर्;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  पूर्णाङ्क nSql;                       /* Length of zSql in bytes */
  पूर्णाङ्क nVar = 0;                   /* Number of variables in statement */
  पूर्णाङ्क iParm = 0;                  /* Next free entry in apParm */
  वर्ण c;
  पूर्णाङ्क i;
  पूर्णाङ्क need_परिणाम_Reset = 0;        /* Need to invoke Tcl_ResetResult() */
  पूर्णाङ्क rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  यावत्( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  प्रति(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->p_अग्र){
    पूर्णाङ्क n = pPreStmt->nSql;
    यदि( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pz_बहिर्गमनम् = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      यदि( pPreStmt->p_पूर्व ){
        pPreStmt->p_पूर्व->p_अग्र = pPreStmt->p_अग्र;
      }अन्यथा{
        pDb->stmtList = pPreStmt->p_अग्र;
      }
      यदि( pPreStmt->p_अग्र ){
        pPreStmt->p_अग्र->p_पूर्व = pPreStmt->p_पूर्व;
      }अन्यथा{
        pDb->stmt_चरमः = pPreStmt->p_पूर्व;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_गणनम्(pStmt);
      भङ्ग;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  यदि( pPreStmt==0 ){
    पूर्णाङ्क nByte;

    यदि( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pz_बहिर्गमनम्) ){
      Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      प्रत्यागम TCL_ERROR;
    }
    यदि( pStmt==0 ){
      यदि( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        प्रत्यागम TCL_ERROR;
      }अन्यथा{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        प्रत्यागम TCL_OK;
      }
    }

    प्रतिज्ञा( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_गणनम्(pStmt);
    nByte = परिमाण(SqlPreparedStmt) + nVar*परिमाण(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    स्मृतिं_पूरय(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (पूर्णाङ्क)(*pz_बहिर्गमनम् - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#यदि_परिभाषितम् SQLITE_TEST
    यदि( pPreStmt->zSql==0 ){
      वर्ण *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      स्मृतिं_प्रतिलिख(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#यदि_अन्तः
  }
  प्रतिज्ञा( pPreStmt );
  प्रतिज्ञा( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  प्रतिज्ञा( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  प्रति(i=1; i<=nVar; i++){
    ध्रुव वर्ण *zVar = sqlite3_bind_parameter_name(pStmt, i);
    यदि( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      यदि( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        पूर्णाङ्क rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_गणनम्(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        यदि( need_परिणाम_Reset ) Tcl_Reset_परिणाम(interp);
        need_परिणाम_Reset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_प्रत्यक्षम्);
        Tcl_Decr_Ref_गणनम्(pCmd);
        यदि( rx==TCL_OK ){
          pVar = Tcl_Get_Obj_परिणाम(interp);
        }अन्यथा यदि( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          भङ्ग;
        }अन्यथा{
          pVar = 0;
        }
      }
      यदि( pVar ){
        Tcl_आकार n;
        u8 *दत्त;
        ध्रुव वर्ण *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        यदि( zVar[0]=='@' ||
           (c=='b' && वाक्यं_तुलय(zType,"bytearray")==0 && pVar->अष्टकानि==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          दत्त = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, दत्त, n, SQLITE_STATIC);
          Tcl_Incr_Ref_गणनम्(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }अन्यथा यदि( c=='b' && pVar->अष्टकानि==0 
               && (वाक्यं_तुलय(zType,"booleanString")==0
                   || वाक्यं_तुलय(zType,"boolean")==0)
        ){
          पूर्णाङ्क nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }अन्यथा यदि( c=='d' && वाक्यं_तुलय(zType,"double")==0 ){
          द्विगुण r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }अन्यथा यदि( (c=='w' && वाक्यं_तुलय(zType,"wideInt")==0) ||
              (c=='i' && वाक्यं_तुलय(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }अन्यथा{
          दत्त = (अचिह्नित वर्ण *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (वर्ण *)दत्त, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_गणनम्(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }अन्यथा{
        sqlite3_bind_null(pStmt, i);
      }
      यदि( need_परिणाम_Reset ) Tcl_Reset_परिणाम(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  यदि( need_परिणाम_Reset && rc==TCL_OK ) Tcl_Reset_परिणाम(pDb->interp);

  प्रत्यागम rc;
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
स्थिर शून्य dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  पूर्णाङ्क discard                     /* True to delete (not cache) the pPreStmt */
){
  पूर्णाङ्क i;

  /* Free the bound string and blob parameters */
  प्रति(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_गणनम्(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  यदि( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }अन्यथा{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->p_अग्र = pDb->stmtList;
    pPreStmt->p_पूर्व = 0;
    यदि( pDb->stmtList ){
     pDb->stmtList->p_पूर्व = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    यदि( pDb->stmt_चरमः==0 ){
      प्रतिज्ञा( pDb->nStmt==0 );
      pDb->stmt_चरमः = pPreStmt;
    }अन्यथा{
      प्रतिज्ञा( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    यावत्( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *p_चरमः = pDb->stmt_चरमः;
      pDb->stmt_चरमः = p_चरमः->p_पूर्व;
      pDb->stmt_चरमः->p_अग्र = 0;
      pDb->nStmt--;
      dbFreeStmt(p_चरमः);
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
प्रकारनिर्देश संरचना DbEvalContext DbEvalContext;
संरचना DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  ध्रुव वर्ण *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  पूर्णाङ्क nCol;                       /* Number of columns returned by pStmt */
  पूर्णाङ्क evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **apColName;            /* Array of column names */
};

#परिभाषय SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#परिभाषय SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
स्थिर शून्य db_Release_Column_नामानि(DbEvalContext *p){
  यदि( p->apColName ){
    पूर्णाङ्क i;
    प्रति(i=0; i<p->nCol; i++){
      Tcl_Decr_Ref_गणनम्(p->apColName[i]);
    }
    Tcl_Free((वर्ण *)p->apColName);
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
स्थिर शून्य dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  पूर्णाङ्क evalFlags                   /* Flags controlling evaluation */
){
  स्मृतिं_पूरय(p, 0, परिमाण(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_गणनम्(pSql);
  यदि( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_गणनम्(pVarName);
  }
  p->evalFlags = evalFlags;
  सङ्कलय_Database_Ref(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
स्थिर शून्य dbEvalRowInfo(
  DbEvalContext *p,               /* Evaluation context */
  पूर्णाङ्क *pnCol,                     /* OUT: Number of column names */
  Tcl_Obj ***papColName           /* OUT: Array of column names */
){
  /* Compute column names */
  यदि( 0==p->apColName ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    पूर्णाङ्क i;                        /* Iterator variable */
    पूर्णाङ्क nCol;                     /* Number of columns returned by pStmt */
    Tcl_Obj **apColName = 0;      /* Array of column names */

    p->nCol = nCol = sqlite3_column_गणनम्(pStmt);
    यदि( nCol>0 && (papColName || p->pVarName) ){
      apColName = (Tcl_Obj**)Tcl_Alloc( परिमाण(Tcl_Obj*)*nCol );
      प्रति(i=0; i<nCol; i++){
        apColName[i] = Tcl_NewStringObj(sqlite3_column_name(pStmt,i), -1);
        Tcl_Incr_Ref_गणनम्(apColName[i]);
      }
      p->apColName = apColName;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    यदि( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *pColList = Tcl_NewObj();
      Tcl_Obj *p_तारकम् = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_गणनम्(pColList);
      Tcl_Incr_Ref_गणनम्(p_तारकम्);
      प्रति(i=0; i<nCol; i++){
        Tcl_ListObjAppendElement(interp, pColList, apColName[i]);
      }
      यदि( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_Obj_समुच्चयः_Var2(interp, p->pVarName, p_तारकम्, pColList, 0);
      }अन्यथा{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, रिक्त, 0);
        यदि( !pDict ){
          pDict = Tcl_NewDictObj();
        }अन्यथा यदि( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        यदि( Tcl_DictObjPut(interp, pDict, p_तारकम्, pColList)==TCL_OK ){
          Tcl_Obj_समुच्चयः_Var2(interp, p->pVarName, रिक्त, pDict, 0);
        }
        Tcl_Bounce_Ref_गणनम्(pDict);
      }
      Tcl_Decr_Ref_गणनम्(p_तारकम्);
      Tcl_Decr_Ref_गणनम्(pColList);
    }
  }

  यदि( papColName ){
    *papColName = p->apColName;
  }
  यदि( pnCol ){
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
स्थिर पूर्णाङ्क db_Eval_चरणः(DbEvalContext *p){
  ध्रुव वर्ण *z_पूर्व_Sql = 0;       /* Previous value of p->zSql */

  यावत्( p->zSql[0] || p->pPreStmt ){
    पूर्णाङ्क rc;
    यदि( p->pPreStmt==0 ){
      z_पूर्व_Sql = (p->zSql==z_पूर्व_Sql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      यदि( rc!=TCL_OK ) प्रत्यागम rc;
    }अन्यथा{
      पूर्णाङ्क rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_चरणः(pStmt);
      यदि( rcs==SQLITE_ROW ){
        प्रत्यागम TCL_OK;
      }
      यदि( p->pVarName ){
        dbEvalRowInfo(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->n_चरणः = sqlite3_stmt_स्थितिः(pStmt,SQLITE_STMTSTATUS_FULLSCAN_चरणः,1);
      pDb->n_क्रमय = sqlite3_stmt_स्थितिः(pStmt,SQLITE_STMTSTATUS_क्रमय,1);
      pDb->n_सूचकांक = sqlite3_stmt_स्थितिः(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_स्थितिः(pStmt,SQLITE_STMTSTATUS_VM_चरणः,1);
      db_Release_Column_नामानि(p);
      p->pPreStmt = 0;

      यदि( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#यदि SQLITE_TEST
        यदि( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && z_पूर्व_Sql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = z_पूर्व_Sql;
          अनुवर्त;
        }
#यदि_अन्तः
        Tcl_समुच्चयः_Obj_परिणाम(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        प्रत्यागम TCL_ERROR;
      }अन्यथा{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /*परिसमाप्तम्*/
  प्रत्यागम TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
स्थिर शून्य dbEvalFinalize(DbEvalContext *p){
  यदि( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  यदि( p->pVarName ){
    Tcl_Decr_Ref_गणनम्(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_गणनम्(p->pSql);
  db_Release_Column_नामानि(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
स्थिर Tcl_Obj *db_Eval_Column_मानम्(DbEvalContext *p, पूर्णाङ्क iCol){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  विकल्प( sqlite3_column_type(pStmt, iCol) ){
    पक्ष SQLITE_BLOB: {
      पूर्णाङ्क अष्टकानि = sqlite3_column_अष्टकानि(pStmt, iCol);
      ध्रुव वर्ण *zBlob = sqlite3_column_blob(pStmt, iCol);
      यदि( !zBlob ) अष्टकानि = 0;
      प्रत्यागम Tcl_NewByteArrayObj((u8*)zBlob, अष्टकानि);
    }
    पक्ष SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
      यदि( v>=-2147483647 && v<=2147483647 ){
        प्रत्यागम Tcl_NewIntObj((पूर्णाङ्क)v);
      }अन्यथा{
        प्रत्यागम Tcl_NewWideIntObj(v);
      }
    }
    पक्ष SQLITE_FLOAT: {
      प्रत्यागम Tcl_NewDoubleObj(sqlite3_column_double(pStmt, iCol));
    }
    पक्ष SQLITE_NULL: {
      प्रत्यागम Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  प्रत्यागम Tcl_NewStringObj((वर्ण*)sqlite3_column_text(pStmt, iCol), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#यदि TCL_MAJOR_संस्करणम्>8 || !defined(TCL_MINOR_संस्करणम्) \
                        || TCL_MINOR_संस्करणम्>=6
# define SQLITE_TCL_NRE 1
स्थिर पूर्णाङ्क DbUseNre(शून्य){
  पूर्णाङ्क major, minor;
  Tcl_Get_संस्करणम्(&major, &minor, 0, 0);
  प्रत्यागम( (major==8 && minor>=6) || major>8 );
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (शून्य)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (शून्य)0
#यदि_अन्तः

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI Db_Eval_अग्र_Cmd(
  Client_दत्त दत्त[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  पूर्णाङ्क परिणाम                           /* Result so far */
){
  पूर्णाङ्क rc = परिणाम;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)दत्त[0];
  Tcl_Obj * ध्रुव pScript = (Tcl_Obj *)दत्त[1];
  Tcl_Obj * ध्रुव pVarName = p->pVarName;

  यावत्( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = db_Eval_चरणः(p)) ){
    पूर्णाङ्क i;
    पूर्णाङ्क nCol;
    Tcl_Obj **apColName;
    dbEvalRowInfo(p, &nCol, &apColName);
    प्रति(i=0; i<nCol; i++){
      यदि( pVarName==0 ){
        Tcl_Obj_समुच्चयः_Var2(interp, apColName[i], 0, db_Eval_Column_मानम्(p,i), 0);
      }अन्यथा यदि( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_column_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        यदि( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(apColName[i]), 0);
        }अन्यथा{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, रिक्त, 0);
          यदि( pDict ){
            यदि( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            यदि( Tcl_DictObjRemove(interp, pDict, apColName[i])==TCL_OK ){
              Tcl_Obj_समुच्चयः_Var2(interp, pVarName, रिक्त, pDict, 0);
            }
            Tcl_Bounce_Ref_गणनम्(pDict);
          }
        }
      }अन्यथा यदि( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_Obj_समुच्चयः_Var2(interp, pVarName, apColName[i],
                       db_Eval_Column_मानम्(p,i), 0);
      }अन्यथा{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, रिक्त, 0);
        यदि( !pDict ){
          pDict = Tcl_NewDictObj();
        }अन्यथा यदि( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        यदि( Tcl_DictObjPut(interp, pDict, apColName[i],
                           db_Eval_Column_मानम्(p,i))==TCL_OK ){
          Tcl_Obj_समुच्चयः_Var2(interp, pVarName, रिक्त, pDict, 0);
        }
        Tcl_Bounce_Ref_गणनम्(pDict);
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
    यदि( DbUseNre() ){
      Tcl_NRAddCallback(interp, Db_Eval_अग्र_Cmd, (शून्य*)p, (शून्य*)pScript, 0, 0);
      प्रत्यागम Tcl_NREvalObj(interp, pScript, 0);
    }अन्यथा{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_गणनम्(pScript);
  dbEvalFinalize(p);
  Tcl_Free((वर्ण *)p);

  यदि( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_Reset_परिणाम(interp);
    rc = TCL_OK;
  }
  प्रत्यागम rc;
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
स्थिर शून्य DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  यदि( *ppHook ){
    Tcl_समुच्चयः_Obj_परिणाम(interp, *ppHook);
    यदि( pArg ){
      Tcl_Decr_Ref_गणनम्(*ppHook);
      *ppHook = 0;
    }
  }
  यदि( pArg ){
    प्रतिज्ञा( !(*ppHook) );
    यदि( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_गणनम्(*ppHook);
    }
  }

#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
  sqlite3_preupdate_hook(db, (pDb->pPreUpdateHook?DbPreUpdateHandler:0), pDb);
#यदि_अन्तः
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
स्थिर पूर्णाङ्क dbQrf(SqliteDb *pDb, पूर्णाङ्क objc, Tcl_Obj *ध्रुव*objv){
#यदि_अपरिभाषितम् SQLITE_QRF_H
  Tcl_समुच्चयः_परिणाम(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  प्रत्यागम TCL_ERROR;
#अन्यथा
  वर्ण *z_परिणाम = 0;             /* Result to be returned */
  ध्रुव वर्ण *zSql = 0;          /* SQL to run */
  पूर्णाङ्क i;                         /* Loop counter */
  पूर्णाङ्क rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  स्थिर ध्रुव वर्ण *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  स्थिर ध्रुव अचिह्नित वर्ण aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_वामः,
    QRF_ALIGN_मध्यः, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_दक्षिणः,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_शिखरम्,
    QRF_ALIGN_W
  };

  स्मृतिं_पूरय(&qrf, 0, परिमाण(qrf));
  qrf.i_संस्करणम् = 2;
  qrf.pzOutput = &z_परिणाम;
  प्रति(i=2; i<objc; i++){
    ध्रुव वर्ण *zArg = Tcl_GetString(objv[i]);
    ध्रुव वर्ण *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    ध्रुव अचिह्नित वर्ण aBoolMap[] = { 0, 2, 1, 2, 1 };
    यदि( zArg[0]!='-' ){
      यदि( zSql ){
        Tcl_Append_परिणाम(pDb->interp, "unknown argument: ", zArg, (वर्ण*)0);
        rc = TCL_ERROR;
        गच्छ format_failed;
      }
      zSql  = zArg;
    }अन्यथा यदि( i==objc-1 ){
      Tcl_Append_परिणाम(pDb->interp, "option has no argument: ", zArg,(वर्ण*)0);
      rc = TCL_ERROR;
      गच्छ format_failed;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-style")==0 ){
      स्थिर ध्रुव वर्ण *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      स्थिर अचिह्नित वर्ण aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_Column,
        QRF_STYLE_गणनम्,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_निवेशय,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_पङ्क्तिः,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_सारणी
      };
      पूर्णाङ्क style;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      यदि( rc ) गच्छ format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-esc")==0 ){
      स्थिर ध्रुव वर्ण *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      स्थिर अचिह्नित वर्ण aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_चिह्नम्
      };
      पूर्णाङ्क esc;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      यदि( rc ) गच्छ format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-text")==0 || वाक्यं_तुलय(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      स्थिर ध्रुव वर्ण *azText[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      स्थिर अचिह्नित वर्ण aTextMap[] = {
        QRF_TEXT_Auto,      QRF_TEXT_Csv,       QRF_TEXT_Html,
        QRF_TEXT_Json,      QRF_TEXT_Plain,     QRF_TEXT_Relaxed,
        QRF_TEXT_Sql,       QRF_TEXT_Tcl
      };
      पूर्णाङ्क txt;
      पूर्णाङ्क k = zArg[2]=='e';
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], &azText[k*3], zArg,
                               0, &txt);
      यदि( rc ) गच्छ format_failed;
      यदि( k ){
        qrf.eText = aTextMap[txt];
      }अन्यथा यदि( txt<=2 ){
        प्रतिज्ञा( QRF_No==1 );
        प्रतिज्ञा( QRF_Yes==2 );
        प्रतिज्ञा( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_TEXT_Auto;
      }अन्यथा{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = aTextMap[txt-3];
      }
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-blob")==0 ){
      स्थिर ध्रुव वर्ण *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      स्थिर अचिह्नित वर्ण aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_Text,      QRF_BLOB_Sql,
        QRF_BLOB_आकार
      };
      पूर्णाङ्क blob;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      यदि( rc ) गच्छ format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-wordwrap")==0 ){
      पूर्णाङ्क v = 0;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      यदि( rc ) गच्छ format_failed;
      qrf.bWordWrap = aBoolMap[v];
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-textjsonb")==0
           || वाक्यं_तुलय(zArg,"-splitcolumn")==0
           || वाक्यं_तुलय(zArg,"-border")==0
           || वाक्यं_तुलय(zArg,"-rowcount")==0
    ){
      पूर्णाङ्क v = 0;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      यदि( rc ) गच्छ format_failed;
      यदि( zArg[1]=='t' ){
        qrf.bTextJsonb = aBoolMap[v];
      }अन्यथा यदि( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }अन्यथा यदि( zArg[1]=='r' ){
        qrf.b_Row_गणनम् = aBoolMap[v];
      }अन्यथा{
        qrf.b_विभज_Column = aBoolMap[v];
      }
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-defaultalign")==0
           || वाक्यं_तुलय(zArg,"-titlealign")==0){
      पूर्णाङ्क ax = 0;
      rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      यदि( rc ) गच्छ format_failed;
      यदि( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }अन्यथा{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-wrap")==0
           || वाक्यं_तुलय(zArg,"-screenwidth")==0 
           || वाक्यं_तुलय(zArg,"-linelimit")==0 
           || वाक्यं_तुलय(zArg,"-titlelimit")==0 
    ){
      पूर्णाङ्क v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      यदि( rc ) गच्छ format_failed;
      यदि( v<QRF_MIN_विस्तारः ){
        v = QRF_MIN_विस्तारः;
      }अन्यथा यदि( v>QRF_MAX_विस्तारः ){
        v = QRF_MAX_विस्तारः;
      }
      यदि( zArg[1]=='w' ){
        qrf.nWrap = v;
      }अन्यथा यदि( zArg[1]=='s' ){
        qrf.n_Screen_विस्तारः = v;
      }अन्यथा यदि( zArg[1]=='t' ){
        qrf.n_Title_सीमा = v;
      }अन्यथा{
        qrf.n_पङ्क्तिः_सीमा = v;
      }
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-charlimit")==0 ){
      पूर्णाङ्क v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      यदि( rc ) गच्छ format_failed;
      यदि( v<0 ) v = 0;
      qrf.n_Char_सीमा = v;
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-multiinsert")==0 ){
      पूर्णाङ्क v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      यदि( rc ) गच्छ format_failed;
      यदि( v<0 ) v = 0;
      qrf.n_Multi_निवेशय = v;
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-align")==0 ){
      Tcl_आकार n = 0;
      पूर्णाङ्क jj;
      rc = Tcl_List_Obj_दीर्घता(pDb->interp, objv[i+1], &n);
      यदि( rc ) गच्छ format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*परिमाण(qrf.aAlign[0]) );
      यदि( qrf.aAlign==0 ){
        Tcl_Append_परिणाम(pDb->interp, "out of memory", (वर्ण*)0);
        rc = TCL_ERROR;
        गच्छ format_failed;
      }
      स्मृतिं_पूरय(qrf.aAlign, 0, (n+1)*परिमाण(qrf.aAlign[0]));
      qrf.nAlign = n;
      प्रति(jj=0; jj<n; jj++){
        पूर्णाङ्क x;
        Tcl_Obj *p_पदांशः;
        rc = Tcl_List_Obj_सूचकांक(pDb->interp, objv[i+1], jj, &p_पदांशः);
        यदि( rc ) गच्छ format_failed;
        rc = Tcl_Get_सूचकांक_From_Obj(pDb->interp, p_पदांशः, azAlign,
                          "column alignment (-align)", 0, &x);
        यदि( rc ) गच्छ format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-widths")==0 ){
      Tcl_आकार n = 0;
      पूर्णाङ्क jj;
      rc = Tcl_List_Obj_दीर्घता(pDb->interp, objv[i+1], &n);
      यदि( rc ) गच्छ format_failed;
      sqlite3_free(qrf.a_विस्तारः);
      qrf.a_विस्तारः = sqlite3_malloc64( (n+1)*परिमाण(qrf.a_विस्तारः[0]) );
      यदि( qrf.a_विस्तारः==0 ){
        Tcl_Append_परिणाम(pDb->interp, "out of memory", (वर्ण*)0);
        rc = TCL_ERROR;
        गच्छ format_failed;
      }
      स्मृतिं_पूरय(qrf.a_विस्तारः, 0, (n+1)*परिमाण(qrf.a_विस्तारः[0]));
      qrf.n_विस्तारः = n;
      प्रति(jj=0; jj<n; jj++){
        Tcl_Obj *p_पदांशः;
        पूर्णाङ्क v;
        rc = Tcl_List_Obj_सूचकांक(pDb->interp, objv[i+1], jj, &p_पदांशः);
        यदि( rc ) गच्छ format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, p_पदांशः, &v);
        यदि( v<(-QRF_MAX_विस्तारः) ){
          v = -QRF_MAX_विस्तारः;
        }अन्यथा यदि( v>QRF_MAX_विस्तारः ){
          v = QRF_MAX_विस्तारः;
        }
        qrf.a_विस्तारः[jj] = (ह्रस्व पूर्णाङ्क)v;
      }
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-columnsep")==0 ){
      qrf.zColumnSep = Tcl_GetString(objv[i+1]);
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-rowsep")==0 ){
      qrf.zRowSep = Tcl_GetString(objv[i+1]);
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-tablename")==0 ){
      qrf.z_सारणी_Name = Tcl_GetString(objv[i+1]);
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-ifmt")==0 ){
      वर्ण *zFmt = Tcl_GetString(objv[i+1]);
      यदि( zFmt[0]==0 || वाक्यं_तुलय(zFmt,"auto")==0 ){
        zFmt = 0;
      }अन्यथा यदि( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_Append_परिणाम(pDb->interp, "bad format for -ifmt", (वर्ण*)0);
        rc = TCL_ERROR;
        गच्छ format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-fpfmt")==0 ){
      वर्ण *zFmt = Tcl_GetString(objv[i+1]);
      यदि( zFmt[0]==0 || वाक्यं_तुलय(zFmt,"auto")==0 ){
        zFmt = 0;
      }अन्यथा यदि( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_Append_परिणाम(pDb->interp, "bad format for -fpfmt", (वर्ण*)0);
        rc = TCL_ERROR;
        गच्छ format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }अन्यथा यदि( वाक्यं_तुलय(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.i_संस्करणम् = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }अन्यथा{
      Tcl_Append_परिणाम(pDb->interp, "unknown option: ", zArg, (वर्ण*)0);
      rc = TCL_ERROR;
      गच्छ format_failed;
    }
  }
  यावत्( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    वर्ण *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    यदि( rc ) गच्छ format_failed;
    यदि( pStmt==0 ) अनुवर्त;
    rc = sqlite3_format_query_परिणाम(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    यदि( rc ){
      Tcl_समुच्चयः_परिणाम(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      गच्छ format_failed;
    }
  }
  Tcl_समुच्चयः_परिणाम(pDb->interp, z_परिणाम, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.a_विस्तारः);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(z_परिणाम);
  प्रत्यागम rc;

#यदि_अन्तः
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI DbObjCmd(
  शून्य *cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *ध्रुव*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  पूर्णाङ्क choice;
  पूर्णाङ्क rc = TCL_OK;
  स्थिर ध्रुव वर्ण *DB_strs[] = {
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
  गणना DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_आरोपय_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_व्यवधानम्,            DB_चरमः_निवेशय_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_प्रगतिः,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_स्थितिः,                DB_TIMEOUT,              DB_योग_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_संस्करणम्,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  यदि( objc<2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "SUBCOMMAND ...");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_Get_सूचकांक_From_Obj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    प्रत्यागम TCL_ERROR;
  }

  विकल्प( (गणना DB_enum)choice ){

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
  पक्ष DB_AUTHORIZER: {
#यदि_परिभाषितम् SQLITE_OMIT_AUTHORIZATION
    Tcl_Append_परिणाम(interp, "authorization not available in this build",
                     (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
#अन्यथा
    यदि( objc>3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?CALLBACK?");
      प्रत्यागम TCL_ERROR;
    }अन्यथा यदि( objc==2 ){
      यदि( pDb->zAuth ){
        Tcl_Append_परिणाम(interp, pDb->zAuth, (वर्ण*)0);
      }
    }अन्यथा{
      वर्ण *zAuth;
      Tcl_आकार len;
      यदि( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      यदि( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->zAuth, zAuth, len+1);
      }अन्यथा{
        pDb->zAuth = 0;
      }
      यदि( pDb->zAuth ){
        प्रकारनिर्देश पूर्णाङ्क (*sqlite3_auth_cb)(
           शून्य*,पूर्णाङ्क,ध्रुव वर्ण*,ध्रुव वर्ण*,
           ध्रुव वर्ण*,ध्रुव वर्ण*);
        pDb->interp = interp;
        sqlite3_समुच्चयः_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }अन्यथा{
        sqlite3_समुच्चयः_authorizer(pDb->db, 0, 0);
      }
    }
#यदि_अन्तः
    भङ्ग;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  पक्ष DB_BACKUP: {
    ध्रुव वर्ण *zDestFile;
    ध्रुव वर्ण *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    यदि( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }अन्यथा यदि( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }अन्यथा{
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?DATABASE? FILENAME");
      प्रत्यागम TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    यदि( rc!=SQLITE_OK ){
      Tcl_Append_परिणाम(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (वर्ण*)0);
      sqlite3_close(pDest);
      प्रत्यागम TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    यदि( pBackup==0 ){
      Tcl_Append_परिणाम(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (वर्ण*)0);
      sqlite3_close(pDest);
      प्रत्यागम TCL_ERROR;
    }
    यावत्(  (rc = sqlite3_backup_चरणः(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    यदि( rc==SQLITE_समाप्तम् ){
      rc = TCL_OK;
    }अन्यथा{
      Tcl_Append_परिणाम(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (वर्ण*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    भङ्ग;
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
  पक्ष DB_BIND_FALLBACK: {
    यदि( objc>3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?CALLBACK?");
      प्रत्यागम TCL_ERROR;
    }अन्यथा यदि( objc==2 ){
      यदि( pDb->zBindFallback ){
        Tcl_Append_परिणाम(interp, pDb->zBindFallback, (वर्ण*)0);
      }
    }अन्यथा{
      वर्ण *zCallback;
      Tcl_आकार len;
      यदि( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      यदि( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->zBindFallback, zCallback, len+1);
      }अन्यथा{
        pDb->zBindFallback = 0;
      }
    }
    भङ्ग;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  पक्ष DB_BUSY: {
    यदि( objc>3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "CALLBACK");
      प्रत्यागम TCL_ERROR;
    }अन्यथा यदि( objc==2 ){
      यदि( pDb->zBusy ){
        Tcl_Append_परिणाम(interp, pDb->zBusy, (वर्ण*)0);
      }
    }अन्यथा{
      वर्ण *zBusy;
      Tcl_आकार len;
      यदि( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      यदि( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->zBusy, zBusy, len+1);
      }अन्यथा{
        pDb->zBusy = 0;
      }
      यदि( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }अन्यथा{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    भङ्ग;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  पक्ष DB_CACHE: {
    वर्ण *subCmd;
    पूर्णाङ्क n;

    यदि( objc<=2 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "cache option ?arg?");
      प्रत्यागम TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    यदि( *subCmd=='f' && वाक्यं_तुलय(subCmd,"flush")==0 ){
      यदि( objc!=3 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "flush");
        प्रत्यागम TCL_ERROR;
      }अन्यथा{
        flushStmtCache( pDb );
      }
    }अन्यथा यदि( *subCmd=='s' && वाक्यं_तुलय(subCmd,"size")==0 ){
      यदि( objc!=4 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "size n");
        प्रत्यागम TCL_ERROR;
      }अन्यथा{
        यदि( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_Append_परिणाम( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (वर्ण*)0);
          प्रत्यागम TCL_ERROR;
        }अन्यथा{
          यदि( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }अन्यथा यदि( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }अन्यथा{
      Tcl_Append_परिणाम( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
    भङ्ग;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  पक्ष DB_CHANGES: {
    Tcl_Obj *p_परिणाम;
    यदि( objc!=2 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "");
      प्रत्यागम TCL_ERROR;
    }
    p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
    Tcl_समुच्चयः_Wide_Int_Obj(p_परिणाम, sqlite3_changes64(pDb->db));
    भङ्ग;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  पक्ष DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    भङ्ग;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  पक्ष DB_COLLATE: {
    SqlCollate *pCollate;
    वर्ण *zName;
    वर्ण *zScript;
    Tcl_आकार nScript;
    यदि( objc!=4 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "NAME SCRIPT");
      प्रत्यागम TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( परिमाण(*pCollate) + nScript + 1 );
    यदि( pCollate==0 ) प्रत्यागम TCL_ERROR;
    pCollate->interp = interp;
    pCollate->p_अग्र = pDb->pCollate;
    pCollate->zScript = (वर्ण*)&pCollate[1];
    pDb->pCollate = pCollate;
    स्मृतिं_प्रतिलिख(pCollate->zScript, zScript, nScript+1);
    यदि( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      प्रत्यागम TCL_ERROR;
    }
    भङ्ग;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  पक्ष DB_COLLATION_NEEDED: {
    यदि( objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "SCRIPT");
      प्रत्यागम TCL_ERROR;
    }
    यदि( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_गणनम्(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_गणनम्(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    भङ्ग;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  पक्ष DB_COMMIT_HOOK: {
    यदि( objc>3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?CALLBACK?");
      प्रत्यागम TCL_ERROR;
    }अन्यथा यदि( objc==2 ){
      यदि( pDb->zCommit ){
        Tcl_Append_परिणाम(interp, pDb->zCommit, (वर्ण*)0);
      }
    }अन्यथा{
      ध्रुव वर्ण *zCommit;
      Tcl_आकार len;
      यदि( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      यदि( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->zCommit, zCommit, len+1);
      }अन्यथा{
        pDb->zCommit = 0;
      }
      यदि( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }अन्यथा{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    भङ्ग;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  पक्ष DB_COMPLETE: {
#यदि_अपरिभाषितम् SQLITE_OMIT_COMPLETE
    Tcl_Obj *p_परिणाम;
    पूर्णाङ्क isComplete;
    यदि( objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "SQL");
      प्रत्यागम TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
    Tcl_समुच्चयः_Boolean_Obj(p_परिणाम, isComplete);
#यदि_अन्तः
    भङ्ग;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  पक्ष DB_CONFIG: {
    स्थिर ध्रुव संरचना DbConfigChoices {
      ध्रुव वर्ण *zName;
      पूर्णाङ्क op;
    } aDbConfig[] = {
        { "defensive",          SQLITE_DBCONFIG_DEFENSIVE             },
        { "dqs_ddl",            SQLITE_DBCONFIG_DQS_DDL               },
        { "dqs_dml",            SQLITE_DBCONFIG_DQS_DML               },
        { "enable_fkey",        SQLITE_DBCONFIG_ENABLE_FKEY           },
        { "enable_qpsg",        SQLITE_DBCONFIG_ENABLE_QPSG           },
        { "enable_trigger",     SQLITE_DBCONFIG_ENABLE_TRIGGER        },
        { "enable_view",        SQLITE_DBCONFIG_ENABLE_VIEW           },
        { "fts3_tokenizer",     SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER },
        { "legacy_alter_table", SQLITE_DBCONFIG_LEGACY_ALTER_सारणी    },
        { "legacy_file_format", SQLITE_DBCONFIG_LEGACY_FILE_FORMAT    },
        { "load_extension",     SQLITE_DBCONFIG_ENABLE_आरोपय_EXTENSION },
        { "no_ckpt_on_close",   SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE      },
        { "reset_database",     SQLITE_DBCONFIG_RESET_DATABASE        },
        { "trigger_eqp",        SQLITE_DBCONFIG_TRIGGER_EQP           },
        { "trusted_schema",     SQLITE_DBCONFIG_TRUSTED_SCHEMA        },
        { "writable_schema",    SQLITE_DBCONFIG_WRITABLE_SCHEMA       },
    };
    Tcl_Obj *p_परिणाम;
    पूर्णाङ्क ii;
    यदि( objc>4 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      प्रत्यागम TCL_ERROR;
    }
    यदि( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      p_परिणाम = Tcl_NewListObj(0,0);
      प्रति(ii=0; ii<परिमाण(aDbConfig)/परिमाण(aDbConfig[0]); ii++){
        पूर्णाङ्क v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, p_परिणाम,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, p_परिणाम,
           Tcl_NewIntObj(v));
      }
    }अन्यथा{
      ध्रुव वर्ण *zOpt = Tcl_GetString(objv[2]);
      पूर्णाङ्क onoff = -1;
      पूर्णाङ्क v = 0;
      यदि( zOpt[0]=='-' ) zOpt++;
      प्रति(ii=0; ii<परिमाण(aDbConfig)/परिमाण(aDbConfig[0]); ii++){
        यदि( वाक्यं_तुलय(aDbConfig[ii].zName, zOpt)==0 ) भङ्ग;
      }
      यदि( ii>=परिमाण(aDbConfig)/परिमाण(aDbConfig[0]) ){
        Tcl_Append_परिणाम(interp, "unknown config option: \"", zOpt,
                                "\"", (शून्य*)0);
        प्रत्यागम TCL_ERROR;
      }
      यदि( objc==4 ){
        यदि( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          प्रत्यागम TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      p_परिणाम = Tcl_NewIntObj(v);
    }
    Tcl_समुच्चयः_Obj_परिणाम(interp, p_परिणाम);
    भङ्ग;
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
  पक्ष DB_COPY: {
    वर्ण *z_सारणी;               /* Insert data into this table */
    वर्ण *zFile;                /* The file from which to extract data */
    वर्ण *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    पूर्णाङ्क nCol;                   /* Number of columns in the table */
    पूर्णाङ्क nByte;                  /* Number of bytes in an SQL string */
    पूर्णाङ्क i, j;                   /* Loop counters */
    पूर्णाङ्क nSep;                   /* Number of bytes in zSep[] */
    पूर्णाङ्क nNull;                  /* Number of bytes in zNull[] */
    वर्ण *zSql;                 /* An SQL statement */
    वर्ण *z_पङ्क्तिः;                /* A single line of input from the file */
    वर्ण **azCol;               /* zLine[] broken up into columns */
    ध्रुव वर्ण *zCommit;        /* How to commit changes */
    Tcl_Channel अन्तर्;             /* The input file */
    पूर्णाङ्क lineno = 0;             /* Line number of input file */
    वर्ण z_पङ्क्तिः_अङ्कः[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *p_परिणाम;           /* interp result */

    ध्रुव वर्ण *zSep;
    ध्रुव वर्ण *zNull;
    यदि( objc<5 || objc>7 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      प्रत्यागम TCL_ERROR;
    }
    यदि( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }अन्यथा{
      zSep = "\t";
    }
    यदि( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }अन्यथा{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    z_सारणी = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    यदि( nSep==0 ){
      Tcl_Append_परिणाम(interp,"Error: non-null separator required for copy",
                       (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
    यदि(वाक्यं_तुलय(zConflict, "rollback") != 0 &&
       वाक्यं_तुलय(zConflict, "abort"   ) != 0 &&
       वाक्यं_तुलय(zConflict, "fail"    ) != 0 &&
       वाक्यं_तुलय(zConflict, "ignore"  ) != 0 &&
       वाक्यं_तुलय(zConflict, "replace" ) != 0 ) {
      Tcl_Append_परिणाम(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", z_सारणी);
    यदि( zSql==0 ){
      Tcl_Append_परिणाम(interp, "Error: no such table: ", z_सारणी, (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    यदि( rc ){
      Tcl_Append_परिणाम(interp, "Error: ", sqlite3_errmsg(pDb->db), (वर्ण*)0);
      nCol = 0;
    }अन्यथा{
      nCol = sqlite3_column_गणनम्(pStmt);
    }
    sqlite3_finalize(pStmt);
    यदि( nCol==0 ) {
      प्रत्यागम TCL_ERROR;
    }
    zSql = स्मृतिं_देहि( nByte + 50 + nCol*2 );
    यदि( zSql==0 ) {
      Tcl_Append_परिणाम(interp, "Error: can't malloc()", (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, z_सारणी);
    j = strlen30(zSql);
    प्रति(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    मुक्त(zSql);
    यदि( rc ){
      Tcl_Append_परिणाम(interp, "Error: ", sqlite3_errmsg(pDb->db), (वर्ण*)0);
      sqlite3_finalize(pStmt);
      प्रत्यागम TCL_ERROR;
    }
    अन्तर् = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    यदि( अन्तर्==0 ){
      sqlite3_finalize(pStmt);
      प्रत्यागम TCL_ERROR;
    }
    Tcl_समुच्चयः_Channel_Option(रिक्त, अन्तर्, "-translation", "auto");
    azCol = स्मृतिं_देहि( परिमाण(azCol[0])*(nCol+1) );
    यदि( azCol==0 ) {
      Tcl_Append_परिणाम(interp, "Error: can't malloc()", (वर्ण*)0);
      Tcl_Close(interp, अन्तर्);
      प्रत्यागम TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_गणनम्(str);
    (शून्य)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    यावत्( Tcl_GetsObj(अन्तर्, str)>=0 ) {
      वर्ण *z;
      Tcl_आकार byteLen;
      lineno++;
      z_पङ्क्तिः = (वर्ण *)Tcl_GetByteArrayFromObj(str, &byteLen);
      azCol[0] = z_पङ्क्तिः;
      प्रति(i=0, z=z_पङ्क्तिः; *z; z++){
        यदि( *z==zSep[0] && वाक्ये_तुलय(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          यदि( i<nCol ){
            azCol[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      यदि( i+1!=nCol ){
        वर्ण *zErr;
        पूर्णाङ्क nErr = strlen30(zFile) + 200;
        zErr = स्मृतिं_देहि(nErr);
        यदि( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, nCol, i+1);
          Tcl_Append_परिणाम(interp, zErr, (वर्ण*)0);
          मुक्त(zErr);
        }
        zCommit = "ROLLBACK";
        भङ्ग;
      }
      प्रति(i=0; i<nCol; i++){
        /* check for null data, if so, bind as null */
        यदि( (nNull>0 && वाक्यं_तुलय(azCol[i], zNull)==0)
          || strlen30(azCol[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }अन्यथा{
          sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_चरणः(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_समुच्चयः_Obj_दीर्घता(str, 0);
      यदि( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp,"Error: ", sqlite3_errmsg(pDb->db), (वर्ण*)0);
        zCommit = "ROLLBACK";
        भङ्ग;
      }
    }
    Tcl_Decr_Ref_गणनम्(str);
    मुक्त(azCol);
    Tcl_Close(interp, अन्तर्);
    sqlite3_finalize(pStmt);
    (शून्य)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    यदि( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
      यदि( Tcl_IsShared(p_परिणाम) ){
        p_परिणाम = Tcl_DuplicateObj(p_परिणाम);
      }
      Tcl_समुच्चयः_Int_Obj(p_परिणाम, lineno);
      rc = TCL_OK;
    }अन्यथा{
      /* failure, append lineno where failed */
      sqlite3_snprintf(परिमाण(z_पङ्क्तिः_अङ्कः), z_पङ्क्तिः_अङ्कः,"%d",lineno);
      Tcl_Append_परिणाम(interp,", failed while processing line: ",z_पङ्क्तिः_अङ्कः,
                       (वर्ण*)0);
      rc = TCL_ERROR;
    }
    भङ्ग;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  पक्ष DB_DESERIALIZE: {
#यदि_परिभाषितम् SQLITE_OMIT_DESERIALIZE
    Tcl_Append_परिणाम(interp, "MEMDB not available in this build",
                     (वर्ण*)0);
    rc = TCL_ERROR;
#अन्यथा
    ध्रुव वर्ण *zSchema = 0;
    Tcl_Obj *p_मानम् = 0;
    अचिह्नित वर्ण *pBA;
    अचिह्नित वर्ण *p_दत्त;
    Tcl_आकार len;
    पूर्णाङ्क xrc;
    sqlite3_int64 mx_आकार = 0;
    पूर्णाङ्क i;
    पूर्णाङ्क isReadonly = 0;


    यदि( objc<3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      भङ्ग;
    }
    प्रति(i=2; i<objc-1; i++){
      ध्रुव वर्ण *z = Tcl_GetString(objv[i]);
      यदि( वाक्यं_तुलय(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        यदि( rc ) गच्छ deserialize_error;
        mx_आकार = x;
        अनुवर्त;
      }
      यदि( वाक्यं_तुलय(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        यदि( rc ) गच्छ deserialize_error;
        अनुवर्त;
      }
      यदि( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        अनुवर्त;
      }
      Tcl_Append_परिणाम(interp, "unknown option: ", z, (वर्ण*)0);
      rc = TCL_ERROR;
      गच्छ deserialize_error;
    }
    p_मानम् = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_मानम्, &len);
    p_दत्त = sqlite3_malloc64( len );
    यदि( p_दत्त==0 && len>0 ){
      Tcl_Append_परिणाम(interp, "out of memory", (वर्ण*)0);
      rc = TCL_ERROR;
    }अन्यथा{
      पूर्णाङ्क flags;
      यदि( len>0 ) स्मृतिं_प्रतिलिख(p_दत्त, pBA, len);
      यदि( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }अन्यथा{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, p_दत्त, len, len, flags);
      यदि( xrc ){
        Tcl_Append_परिणाम(interp, "unable to set MEMDB content", (वर्ण*)0);
        rc = TCL_ERROR;
      }
      यदि( mx_आकार>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_आकार_सीमा,&mx_आकार);
      }
    }
deserialize_error:
#यदि_अन्तः
    भङ्ग; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  पक्ष DB_ENABLE_आरोपय_EXTENSION: {
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
    पूर्णाङ्क onoff;
    यदि( objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "BOOLEAN");
      प्रत्यागम TCL_ERROR;
    }
    यदि( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      प्रत्यागम TCL_ERROR;
    }
    sqlite3_enable_आरोपय_extension(pDb->db, onoff);
    भङ्ग;
#अन्यथा
    Tcl_Append_परिणाम(interp, "extension loading is turned off at compile-time",
                     (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
#यदि_अन्तः
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  पक्ष DB_ERRORCODE: {
    Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    भङ्ग;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  पक्ष DB_ERROROFFSET: {
    Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(sqlite3_error_विस्थापनम्(pDb->db)));
    भङ्ग;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  पक्ष DB_EXISTS:
  पक्ष DB_ONECOLUMN: {
    Tcl_Obj *p_परिणाम = 0;
    DbEvalContext sEval;
    यदि( objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "SQL");
      प्रत्यागम TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = db_Eval_चरणः(&sEval);
    यदि( choice==DB_ONECOLUMN ){
      यदि( rc==TCL_OK ){
        p_परिणाम = db_Eval_Column_मानम्(&sEval, 0);
      }अन्यथा यदि( rc==TCL_BREAK ){
        Tcl_Reset_परिणाम(interp);
      }
    }अन्यथा यदि( rc==TCL_BREAK || rc==TCL_OK ){
      p_परिणाम = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    यदि( p_परिणाम ) Tcl_समुच्चयः_Obj_परिणाम(interp, p_परिणाम);

    यदि( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    भङ्ग;
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
  पक्ष DB_EVAL: {
    पूर्णाङ्क evalFlags = 0;
    ध्रुव वर्ण *zOpt;
    यावत्( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      यदि( वाक्यं_तुलय(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }अन्यथा यदि( वाक्यं_तुलय(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }अन्यथा{
        Tcl_Append_परिणाम(interp, "unknown option: \"", zOpt, "\"", (शून्य*)0);
        प्रत्यागम TCL_ERROR;
      }
      objc--;
      objv++;
    }
    यदि( objc<3 || objc>5 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      प्रत्यागम TCL_ERROR;
    }

    यदि( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_गणनम्(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      यावत्( TCL_OK==(rc = db_Eval_चरणः(&sEval)) ){
        पूर्णाङ्क i;
        पूर्णाङ्क nCol;
        dbEvalRowInfo(&sEval, &nCol, 0);
        प्रति(i=0; i<nCol; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_Column_मानम्(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      यदि( rc==TCL_BREAK ){
        Tcl_समुच्चयः_Obj_परिणाम(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_गणनम्(pRet);
    }अन्यथा{
      Client_दत्त cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      यदि( objc>=5 && *(वर्ण *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_गणनम्(pScript);

      p = (DbEvalContext *)Tcl_Alloc(परिमाण(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (शून्य *)p;
      cd2[1] = (शून्य *)pScript;
      rc = Db_Eval_अग्र_Cmd(cd2, interp, TCL_OK);
    }
    भङ्ग;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  पक्ष DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    भङ्ग;
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
  पक्ष DB_FUNCTION: {
    पूर्णाङ्क flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    वर्ण *zName;
    पूर्णाङ्क nArg = -1;
    पूर्णाङ्क i;
    पूर्णाङ्क eType = SQLITE_NULL;
    यदि( objc<4 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      प्रत्यागम TCL_ERROR;
    }
    प्रति(i=3; i<(objc-1); i++){
      ध्रुव वर्ण *z = Tcl_GetString(objv[i]);
      पूर्णाङ्क n = strlen30(z);
      यदि( n>1 && वाक्ये_तुलय(z, "-argcount",n)==0 ){
        यदि( i==(objc-2) ){
          Tcl_Append_परिणाम(interp, "option requires an argument: ", z,(वर्ण*)0);
          प्रत्यागम TCL_ERROR;
        }
        यदि( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) प्रत्यागम TCL_ERROR;
        यदि( nArg<0 ){
          Tcl_Append_परिणाम(interp, "number of arguments must be non-negative",
                           (वर्ण*)0);
          प्रत्यागम TCL_ERROR;
        }
        i++;
      }अन्यथा
      यदि( n>1 && वाक्ये_तुलय(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }अन्यथा
      यदि( n>1 && वाक्ये_तुलय(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }अन्यथा
      यदि( n>1 && वाक्ये_तुलय(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }अन्यथा
      यदि( n>1 && वाक्ये_तुलय(z, "-returntype", n)==0 ){
        ध्रुव वर्ण *azType[] = {"integer", "real", "text", "blob", "any", 0};
        प्रतिज्ञा( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_TEXT==3 );
        प्रतिज्ञा( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        यदि( i==(objc-2) ){
          Tcl_Append_परिणाम(interp, "option requires an argument: ", z,(वर्ण*)0);
          प्रत्यागम TCL_ERROR;
        }
        i++;
        यदि( Tcl_Get_सूचकांक_From_Obj(interp, objv[i], azType, "type", 0, &eType) ){
          प्रत्यागम TCL_ERROR;
        }
        eType++;
      }अन्यथा{
        Tcl_Append_परिणाम(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (वर्ण*)0
        );
        प्रत्यागम TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = findSqlFunc(pDb, zName);
    यदि( pFunc==0 ) प्रत्यागम TCL_ERROR;
    यदि( pFunc->pScript ){
      Tcl_Decr_Ref_गणनम्(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_गणनम्(pScript);
    pFunc->useEvalObjv = safe_गन्तव्यम्_Use_Eval_Objv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    यदि( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    भङ्ग;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  पक्ष DB_INCRBLOB: {
#यदि_परिभाषितम् SQLITE_OMIT_INCRBLOB
    Tcl_Append_परिणाम(interp, "incrblob not available in this build", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
#अन्यथा
    पूर्णाङ्क isReadonly = 0;
    ध्रुव वर्ण *zDb = "main";
    ध्रुव वर्ण *z_सारणी;
    ध्रुव वर्ण *zColumn;
    Tcl_WideInt iRow;

    /* Check for the -readonly option */
    यदि( objc>3 && वाक्यं_तुलय(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    यदि( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      प्रत्यागम TCL_ERROR;
    }

    यदि( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    z_सारणी = Tcl_GetString(objv[objc-3]);
    zColumn = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &iRow);

    यदि( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, z_सारणी, zColumn, (sqlite3_int64)iRow, isReadonly
      );
    }
#यदि_अन्तः
    भङ्ग;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  पक्ष DB_व्यवधानम्: {
    sqlite3_व्यवधानम्(pDb->db);
    भङ्ग;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  पक्ष DB_NULLVALUE: {
    यदि( objc!=2 && objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "NULLVALUE");
      प्रत्यागम TCL_ERROR;
    }
    यदि( objc==3 ){
      Tcl_आकार len;
      वर्ण *zNull = Tcl_GetStringFromObj(objv[2], &len);
      यदि( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      यदि( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }अन्यथा{
        pDb->zNull = 0;
      }
    }
    Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj(pDb->zNull, -1));
    भङ्ग;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  पक्ष DB_चरमः_निवेशय_ROWID: {
    Tcl_Obj *p_परिणाम;
    Tcl_WideInt rowid;
    यदि( objc!=2 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "");
      प्रत्यागम TCL_ERROR;
    }
    rowid = sqlite3_चरमः_निवेशय_rowid(pDb->db);
    p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
    Tcl_समुच्चयः_Wide_Int_Obj(p_परिणाम, rowid);
    भङ्ग;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  पक्ष DB_प्रगतिः: {
    यदि( objc==2 ){
      यदि( pDb->z_प्रगतिः ){
        Tcl_Append_परिणाम(interp, pDb->z_प्रगतिः, (वर्ण*)0);
      }
#यदि_अपरिभाषितम् SQLITE_OMIT_प्रगतिः_CALLBACK
      sqlite3_प्रगतिः_handler(pDb->db, 0, 0, 0);
#यदि_अन्तः
    }अन्यथा यदि( objc==4 ){
      वर्ण *z_प्रगतिः;
      Tcl_आकार len;
      पूर्णाङ्क N;
      यदि( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        प्रत्यागम TCL_ERROR;
      };
      यदि( pDb->z_प्रगतिः ){
        Tcl_Free(pDb->z_प्रगतिः);
      }
      z_प्रगतिः = Tcl_GetStringFromObj(objv[3], &len);
      यदि( z_प्रगतिः && len>0 ){
        pDb->z_प्रगतिः = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->z_प्रगतिः, z_प्रगतिः, len+1);
      }अन्यथा{
        pDb->z_प्रगतिः = 0;
      }
#यदि_अपरिभाषितम् SQLITE_OMIT_प्रगतिः_CALLBACK
      यदि( pDb->z_प्रगतिः ){
        pDb->interp = interp;
        sqlite3_प्रगतिः_handler(pDb->db, N, Db_प्रगतिः_Handler, pDb);
      }अन्यथा{
        sqlite3_प्रगतिः_handler(pDb->db, 0, 0, 0);
      }
#यदि_अन्तः
    }अन्यथा{
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "N CALLBACK");
      प्रत्यागम TCL_ERROR;
    }
    भङ्ग;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  पक्ष DB_PROFILE: {
    यदि( objc>3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?CALLBACK?");
      प्रत्यागम TCL_ERROR;
    }अन्यथा यदि( objc==2 ){
      यदि( pDb->zProfile ){
        Tcl_Append_परिणाम(interp, pDb->zProfile, (वर्ण*)0);
      }
    }अन्यथा{
      वर्ण *zProfile;
      Tcl_आकार len;
      यदि( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      यदि( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->zProfile, zProfile, len+1);
      }अन्यथा{
        pDb->zProfile = 0;
      }
#यदि !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      यदि( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }अन्यथा{
        sqlite3_profile(pDb->db, 0, 0);
      }
#यदि_अन्तः
    }
    भङ्ग;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  पक्ष DB_REKEY: {
    यदि( objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "KEY");
      प्रत्यागम TCL_ERROR;
    }
    भङ्ग;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  पक्ष DB_RESTORE: {
    ध्रुव वर्ण *zSrcFile;
    ध्रुव वर्ण *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    पूर्णाङ्क nTimeout = 0;

    यदि( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }अन्यथा यदि( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }अन्यथा{
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?DATABASE? FILENAME");
      प्रत्यागम TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    यदि( rc!=SQLITE_OK ){
      Tcl_Append_परिणाम(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (वर्ण*)0);
      sqlite3_close(pSrc);
      प्रत्यागम TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    यदि( pBackup==0 ){
      Tcl_Append_परिणाम(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (वर्ण*)0);
      sqlite3_close(pSrc);
      प्रत्यागम TCL_ERROR;
    }
    यावत्( (rc = sqlite3_backup_चरणः(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      यदि( rc==SQLITE_BUSY ){
        यदि( nTimeout++ >= 3 ) भङ्ग;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    यदि( rc==SQLITE_समाप्तम् ){
      rc = TCL_OK;
    }अन्यथा यदि( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_Append_परिणाम(interp, "restore failed: source database busy",
                       (वर्ण*)0);
      rc = TCL_ERROR;
    }अन्यथा{
      Tcl_Append_परिणाम(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (वर्ण*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    भङ्ग;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  पक्ष DB_SERIALIZE: {
#यदि_परिभाषितम् SQLITE_OMIT_DESERIALIZE
    Tcl_Append_परिणाम(interp, "MEMDB not available in this build",
                     (वर्ण*)0);
    rc = TCL_ERROR;
#अन्यथा
    ध्रुव वर्ण *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    अचिह्नित वर्ण *p_दत्त;
    यदि( objc!=2 && objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }अन्यथा{
      पूर्णाङ्क needFree;
      p_दत्त = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      यदि( p_दत्त ){
        needFree = 0;
      }अन्यथा{
        p_दत्त = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewByteArrayObj(p_दत्त,sz));
      यदि( needFree ) sqlite3_free(p_दत्त);
    }
#यदि_अन्तः
    भङ्ग;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  पक्ष DB_स्थितिः: {
    पूर्णाङ्क v;
    ध्रुव वर्ण *zOp;
    यदि( objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "(step|sort|autoindex)");
      प्रत्यागम TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    यदि( वाक्यं_तुलय(zOp, "step")==0 ){
      v = pDb->n_चरणः;
    }अन्यथा यदि( वाक्यं_तुलय(zOp, "sort")==0 ){
      v = pDb->n_क्रमय;
    }अन्यथा यदि( वाक्यं_तुलय(zOp, "autoindex")==0 ){
      v = pDb->n_सूचकांक;
    }अन्यथा यदि( वाक्यं_तुलय(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }अन्यथा{
      Tcl_Append_परिणाम(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
    Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(v));
    भङ्ग;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  पक्ष DB_TIMEOUT: {
    पूर्णाङ्क ms;
    यदि( objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "MILLISECONDS");
      प्रत्यागम TCL_ERROR;
    }
    यदि( Tcl_GetIntFromObj(interp, objv[2], &ms) ) प्रत्यागम TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    भङ्ग;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  पक्ष DB_योग_CHANGES: {
    Tcl_Obj *p_परिणाम;
    यदि( objc!=2 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "");
      प्रत्यागम TCL_ERROR;
    }
    p_परिणाम = Tcl_Get_Obj_परिणाम(interp);
    Tcl_समुच्चयः_Wide_Int_Obj(p_परिणाम, sqlite3_योग_changes64(pDb->db));
    भङ्ग;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  पक्ष DB_TRACE: {
    यदि( objc>3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?CALLBACK?");
      प्रत्यागम TCL_ERROR;
    }अन्यथा यदि( objc==2 ){
      यदि( pDb->zTrace ){
        Tcl_Append_परिणाम(interp, pDb->zTrace, (वर्ण*)0);
      }
    }अन्यथा{
      वर्ण *zTrace;
      Tcl_आकार len;
      यदि( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      यदि( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->zTrace, zTrace, len+1);
      }अन्यथा{
        pDb->zTrace = 0;
      }
#यदि !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      यदि( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }अन्यथा{
        sqlite3_trace(pDb->db, 0, 0);
      }
#यदि_अन्तः
    }
    भङ्ग;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  पक्ष DB_TRACE_V2: {
    यदि( objc>4 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?CALLBACK? ?MASK?");
      प्रत्यागम TCL_ERROR;
    }अन्यथा यदि( objc==2 ){
      यदि( pDb->zTraceV2 ){
        Tcl_Append_परिणाम(interp, pDb->zTraceV2, (वर्ण*)0);
      }
    }अन्यथा{
      वर्ण *zTraceV2;
      Tcl_आकार len;
      Tcl_WideInt w_आवरणम् = 0;
      यदि( objc==4 ){
        स्थिर ध्रुव वर्ण *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        गणना TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ROW, TTYPE_CLOSE
        };
        Tcl_आकार i;
        यदि( TCL_OK!=Tcl_List_Obj_दीर्घता(interp, objv[3], &len) ){
          प्रत्यागम TCL_ERROR;
        }
        प्रति(i=0; i<len; i++){
          Tcl_Obj *pObj;
          पूर्णाङ्क ttype;
          यदि( TCL_OK!=Tcl_List_Obj_सूचकांक(interp, objv[3], i, &pObj) ){
            प्रत्यागम TCL_ERROR;
          }
          यदि( Tcl_Get_सूचकांक_From_Obj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_Get_Obj_परिणाम(interp));
            Tcl_Incr_Ref_गणनम्(pError);
            यदि( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_गणनम्(pError);
              w_आवरणम् |= wType;
            }अन्यथा{
              Tcl_समुच्चयः_Obj_परिणाम(interp, pError);
              Tcl_Decr_Ref_गणनम्(pError);
              प्रत्यागम TCL_ERROR;
            }
          }अन्यथा{
            विकल्प( (गणना TTYPE_enum)ttype ){
              पक्ष TTYPE_STMT:    w_आवरणम् |= SQLITE_TRACE_STMT;    भङ्ग;
              पक्ष TTYPE_PROFILE: w_आवरणम् |= SQLITE_TRACE_PROFILE; भङ्ग;
              पक्ष TTYPE_ROW:     w_आवरणम् |= SQLITE_TRACE_ROW;     भङ्ग;
              पक्ष TTYPE_CLOSE:   w_आवरणम् |= SQLITE_TRACE_CLOSE;   भङ्ग;
            }
          }
        }
      }अन्यथा{
        w_आवरणम् = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      यदि( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      यदि( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        स्मृतिं_प्रतिलिख(pDb->zTraceV2, zTraceV2, len+1);
      }अन्यथा{
        pDb->zTraceV2 = 0;
      }
#यदि !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_POINT)
      यदि( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (अचिह्नित)w_आवरणम्, DbTraceV2Handler, pDb);
      }अन्यथा{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#यदि_अन्तः
    }
    भङ्ग;
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
  पक्ष DB_TRANSACTION: {
    Tcl_Obj *pScript;
    ध्रुव वर्ण *zBegin = "SAVEPOINT _tcl_transaction";
    यदि( objc!=3 && objc!=4 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "[TYPE] SCRIPT");
      प्रत्यागम TCL_ERROR;
    }

    यदि( pDb->nTransaction==0 && objc==4 ){
      स्थिर ध्रुव वर्ण *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      गणना TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_तात्कालिकम्
      };
      पूर्णाङ्क ttype;
      यदि( Tcl_Get_सूचकांक_From_Obj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        प्रत्यागम TCL_ERROR;
      }
      विकल्प( (गणना TTYPE_enum)ttype ){
        पक्ष TTYPE_DEFERRED:    /* no-op */;                 भङ्ग;
        पक्ष TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  भङ्ग;
        पक्ष TTYPE_तात्कालिकम्:   zBegin = "BEGIN IMMEDIATE";  भङ्ग;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    यदि( rc!=SQLITE_OK ){
      Tcl_Append_परिणाम(interp, sqlite3_errmsg(pDb->db), (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    सङ्कलय_Database_Ref(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    यदि( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (शून्य)Tcl_NREvalObj(interp, pScript, 0);
    }अन्यथा{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    भङ्ग;
  }

  /*
  **    $db unlock_notify ?script?
  */
  पक्ष DB_UNLOCK_NOTIFY: {
#यदि_अपरिभाषितम् SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_Append_परिणाम(interp, "unlock_notify not available in this build",
                     (वर्ण*)0);
    rc = TCL_ERROR;
#अन्यथा
    यदि( objc!=2 && objc!=3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }अन्यथा{
      शून्य (*xNotify)(शून्य **, पूर्णाङ्क) = 0;
      शून्य *pNotifyArg = 0;

      यदि( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_गणनम्(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      यदि( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (शून्य *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_गणनम्(pDb->pUnlockNotify);
      }

      यदि( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_Append_परिणाम(interp, sqlite3_errmsg(pDb->db), (वर्ण*)0);
        rc = TCL_ERROR;
      }
    }
#यदि_अन्तः
    भङ्ग;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  पक्ष DB_PREUPDATE: {
#यदि_अपरिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_Append_परिणाम(interp, "preupdate_hook was omitted at compile-time", 
                     (वर्ण*)0);
    rc = TCL_ERROR;
#अन्यथा
    स्थिर ध्रुव वर्ण *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    गणना DbPreupdateSubCmd {
      PRE_गणनम्, PRE_गभीरता, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    पूर्णाङ्क iSub;

    यदि( objc<3 ){
      Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    यदि( Tcl_Get_सूचकांक_From_Obj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      प्रत्यागम TCL_ERROR;
    }

    विकल्प( (गणना DbPreupdateSubCmd)iSub ){
      पक्ष PRE_गणनम्: {
        पूर्णाङ्क nCol = sqlite3_preupdate_गणनम्(pDb->db);
        Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(nCol));
        भङ्ग;
      }

      पक्ष PRE_HOOK: {
        यदि( objc>4 ){
          Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "hook ?SCRIPT?");
          प्रत्यागम TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        भङ्ग;
      }

      पक्ष PRE_गभीरता: {
        Tcl_Obj *pRet;
        यदि( objc!=3 ){
          Tcl_Wrong_अङ्कः_तर्काः(interp, 3, objv, "");
          प्रत्यागम TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_गभीरता(pDb->db));
        Tcl_समुच्चयः_Obj_परिणाम(interp, pRet);
        भङ्ग;
      }

      पक्ष PRE_NEW:
      पक्ष PRE_OLD: {
        पूर्णाङ्क iIdx;
        sqlite3_मानम् *p_मानम्;
        यदि( objc!=4 ){
          Tcl_Wrong_अङ्कः_तर्काः(interp, 3, objv, "INDEX");
          प्रत्यागम TCL_ERROR;
        }
        यदि( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          प्रत्यागम TCL_ERROR;
        }

        यदि( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_मानम्);
        }अन्यथा{
          प्रतिज्ञा( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_मानम्);
        }

        यदि( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((वर्ण*)sqlite3_मानम्_text(p_मानम्), -1);
          Tcl_समुच्चयः_Obj_परिणाम(interp, pObj);
        }अन्यथा{
          Tcl_Append_परिणाम(interp, sqlite3_errmsg(pDb->db), (वर्ण*)0);
          प्रत्यागम TCL_ERROR;
        }
      }
    }
#यदि_अन्तः /* SQLITE_ENABLE_PREUPDATE_HOOK */
    भङ्ग;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  पक्ष DB_WAL_HOOK:
  पक्ष DB_UPDATE_HOOK:
  पक्ष DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    यदि( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    यदि( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    यदि( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    यदि( objc>3 ){
       Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?SCRIPT?");
       प्रत्यागम TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    भङ्ग;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  पक्ष DB_संस्करणम्: {
    पूर्णाङ्क i;
    प्रति(i=2; i<objc; i++){
      ध्रुव वर्ण *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#यदि_परिभाषितम् SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      यदि( वाक्यं_तुलय(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        यदि( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          प्रत्यागम TCL_ERROR;
        }
      }अन्यथा

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      यदि( वाक्यं_तुलय(zArg, "-last-stmt-ptr")==0 ){
        बाह्य ध्रुव वर्ण *sqlite3_Test_Ptr_गन्तव्यम्_Text(शून्य*);
        शून्य *ptr = (शून्य*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_समुच्चयः_परिणाम(interp, (वर्ण*)sqlite3_Test_Ptr_गन्तव्यम्_Text(ptr), TCL_VOLATILE);
      }अन्यथा
#यदि_अन्तः /* SQLITE_TEST */
      {
        Tcl_Append_परिणाम(interp, "unknown argument: ", zArg, (वर्ण*)0);
        प्रत्यागम TCL_ERROR;
      }
    }
    यदि( i==2 ){   
      Tcl_समुच्चयः_परिणाम(interp, (वर्ण *)sqlite3_libversion(), TCL_STATIC);
    }
    भङ्ग;
  }


  } /* End of the SWITCH statement */
  प्रत्यागम rc;
}

#यदि SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI DbObjCmdAdaptor(
  शून्य *cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *ध्रुव*objv
){
  प्रत्यागम Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#यदि_अन्तः /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
स्थिर पूर्णाङ्क sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *ध्रुव*objv
){
  Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  प्रत्यागम TCL_ERROR;
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI DbMain(
  शून्य *cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *ध्रुव*objv
){
  SqliteDb *p;
  ध्रुव वर्ण *zArg;
  वर्ण *z_Err_सन्देशः;
  पूर्णाङ्क i;
  ध्रुव वर्ण *zFile = 0;
  ध्रुव वर्ण *zVfs = 0;
  पूर्णाङ्क flags;
  पूर्णाङ्क bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  पूर्णाङ्क rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#यदि_परिभाषितम् SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#अन्यथा
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#यदि_अन्तः

  यदि( objc==1 ) प्रत्यागम sqliteCmdUsage(interp, objv);
  यदि( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    यदि( वाक्यं_तुलय(zArg,"-version")==0 ){
      Tcl_Append_परिणाम(interp,sqlite3_libversion(), (वर्ण*)0);
      प्रत्यागम TCL_OK;
    }
    यदि( वाक्यं_तुलय(zArg,"-sourceid")==0 ){
      Tcl_Append_परिणाम(interp,sqlite3_sourceid(), (वर्ण*)0);
      प्रत्यागम TCL_OK;
    }
    यदि( वाक्यं_तुलय(zArg,"-has-codec")==0 ){
      Tcl_Append_परिणाम(interp,"0",(वर्ण*)0);
      प्रत्यागम TCL_OK;
    }
    यदि( zArg[0]=='-' ) प्रत्यागम sqliteCmdUsage(interp, objv);
  }
  प्रति(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    यदि( zArg[0]!='-' ){
      यदि( zFile!=0 ) प्रत्यागम sqliteCmdUsage(interp, objv);
      zFile = zArg;
      अनुवर्त;
    }
    यदि( i==objc-1 ) प्रत्यागम sqliteCmdUsage(interp, objv);
    i++;
    यदि( वाक्यं_तुलय(zArg,"-key")==0 ){
      /* no-op */
    }अन्यथा यदि( वाक्यं_तुलय(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }अन्यथा यदि( वाक्यं_तुलय(zArg, "-readonly")==0 ){
      पूर्णाङ्क b;
      यदि( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) प्रत्यागम TCL_ERROR;
      यदि( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }अन्यथा यदि( वाक्यं_तुलय(zArg, "-create")==0 ){
      पूर्णाङ्क b;
      यदि( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) प्रत्यागम TCL_ERROR;
      यदि( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }अन्यथा यदि( वाक्यं_तुलय(zArg, "-nofollow")==0 ){
      पूर्णाङ्क b;
      यदि( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) प्रत्यागम TCL_ERROR;
      यदि( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }अन्यथा यदि( वाक्यं_तुलय(zArg, "-nomutex")==0 ){
      पूर्णाङ्क b;
      यदि( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) प्रत्यागम TCL_ERROR;
      यदि( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }अन्यथा यदि( वाक्यं_तुलय(zArg, "-fullmutex")==0 ){
      पूर्णाङ्क b;
      यदि( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) प्रत्यागम TCL_ERROR;
      यदि( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }अन्यथा यदि( वाक्यं_तुलय(zArg, "-uri")==0 ){
      पूर्णाङ्क b;
      यदि( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) प्रत्यागम TCL_ERROR;
      यदि( b ){
        flags |= SQLITE_OPEN_URI;
      }अन्यथा{
        flags &= ~SQLITE_OPEN_URI;
      }
    }अन्यथा यदि( वाक्यं_तुलय(zArg, "-translatefilename")==0 ){
      यदि( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        प्रत्यागम TCL_ERROR;
      }
    }अन्यथा{
      Tcl_Append_परिणाम(interp, "unknown option: ", zArg, (वर्ण*)0);
      प्रत्यागम TCL_ERROR;
    }
  }
  z_Err_सन्देशः = 0;
  p = (SqliteDb*)Tcl_Alloc( परिमाण(*p) );
  स्मृतिं_पूरय(p, 0, परिमाण(*p));
  यदि( zFile==0 ) zFile = "";
  यदि( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  यदि( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  यदि( p->db ){
    यदि( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_सन्देशः = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }अन्यथा{
    z_Err_सन्देशः = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  यदि( p->db==0 ){
    Tcl_समुच्चयः_परिणाम(interp, z_Err_सन्देशः, TCL_VOLATILE);
    Tcl_Free((वर्ण*)p);
    sqlite3_free(z_Err_सन्देशः);
    प्रत्यागम TCL_ERROR;
  }
  p->maxStmt = अङ्कः_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  यदि( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (वर्ण*)p, DbDeleteCmd);
  }अन्यथा{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (वर्ण*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  प्रत्यागम TCL_OK;
}

/*
** Provide a dummy Tcl_InitStubs if we are using this as a static
** library.
*/
#यदि_अपरिभाषितम् USE_TCL_STUBS
# undef  Tcl_InitStubs
# define Tcl_InitStubs(a,b,c) TCL_संस्करणम्
#यदि_अन्तः

/*
** Make sure we have a PACKAGE_VERSION macro defined.  This will be
** defined automatically by the TEA makefile.  But other makefiles
** do not define it.
*/
#यदि_अपरिभाषितम् PACKAGE_संस्करणम्
# define PACKAGE_संस्करणम् SQLITE_संस्करणम्
#यदि_अन्तः

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
EXTERN पूर्णाङ्क Sqlite3_Init(Tcl_Interp *interp){
  पूर्णाङ्क rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  यदि( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#यदि_अपरिभाषितम् SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#यदि_अन्तः
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_संस्करणम्);
  }
  प्रत्यागम rc;
}
EXTERN पूर्णाङ्क Tclsqlite3_Init(Tcl_Interp *interp){ प्रत्यागम Sqlite3_Init(interp); }
EXTERN पूर्णाङ्क Sqlite3_Unload(Tcl_Interp *interp, पूर्णाङ्क flags){ प्रत्यागम TCL_OK; }
EXTERN पूर्णाङ्क Tclsqlite3_Unload(Tcl_Interp *interp, पूर्णाङ्क flags){ प्रत्यागम TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN पूर्णाङ्क Sqlite3_SafeInit(Tcl_Interp *interp){ प्रत्यागम TCL_ERROR; }
EXTERN पूर्णाङ्क Sqlite3_SafeUnload(Tcl_Interp *interp, पूर्णाङ्क flags){प्रत्यागम TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN पूर्णाङ्क Sqlite_Init(Tcl_Interp *interp){ प्रत्यागम Sqlite3_Init(interp);}
EXTERN पूर्णाङ्क Tclsqlite_Init(Tcl_Interp *interp){ प्रत्यागम Sqlite3_Init(interp); }
EXTERN पूर्णाङ्क Sqlite_Unload(Tcl_Interp *interp, पूर्णाङ्क flags){ प्रत्यागम TCL_OK; }
EXTERN पूर्णाङ्क Tclsqlite_Unload(Tcl_Interp *interp, पूर्णाङ्क flags){ प्रत्यागम TCL_OK; }
EXTERN पूर्णाङ्क Sqlite_SafeInit(Tcl_Interp *interp){ प्रत्यागम TCL_ERROR; }
EXTERN पूर्णाङ्क Sqlite_SafeUnload(Tcl_Interp *interp, पूर्णाङ्क flags){प्रत्यागम TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN पूर्णाङ्क sqlite3_Init(Tcl_Interp *interp){ प्रत्यागम Sqlite3_Init(interp);}
EXTERN पूर्णाङ्क sqlite_Init(Tcl_Interp *interp){ प्रत्यागम Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#यदि defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
स्थिर ध्रुव वर्ण *tclsh_main_loop(शून्य){
  स्थिर ध्रुव वर्ण zMainloop[] =
    "if {[llength $argv]>=1} {\n"
#यदि_परिभाषितम् WIN32
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
#यदि_अन्तः
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
  प्रत्यागम zMainloop;
}

#यदि_अपरिभाषितम् TCLSH_MAIN
# define TCLSH_MAIN मुख्यम्
#यदि_अन्तः
पूर्णाङ्क SQLITE_CDECL TCLSH_MAIN(पूर्णाङ्क argc, वर्ण **argv){
  Tcl_Interp *interp;
  पूर्णाङ्क i;
  ध्रुव वर्ण *zScript = 0;
  वर्ण zArgc[32];
#यदि defined(TCLSH_INIT_PROC)
  बाह्य ध्रुव वर्ण *TCLSH_INIT_PROC(Tcl_Interp*);
#यदि_अन्तः

#यदि !defined(_WIN32_WCE)
  यदि( getenv("SQLITE_DEBUG_BREAK") ){
    यदि( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }अन्यथा{
#यदि defined(_WIN32) || defined(WIN32)
      DebugBreak();
#नोचेद्यदि defined(SIGTRAP)
      उत्थापय(SIGTRAP);
#यदि_अन्तः
    }
  }
#यदि_अन्तः

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_FindExecutable(argv[0]);
  Tcl_समुच्चयः_System_Encoding(रिक्त, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(परिमाण(zArgc), zArgc, "%d", argc-1);
  Tcl_समुच्चयः_Var(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_समुच्चयः_Var(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_समुच्चयः_Var(interp,"argv", "", TCL_GLOBAL_ONLY);
  प्रति(i=1; i<argc; i++){
    Tcl_समुच्चयः_Var(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_मानम्);
  }
#यदि defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#यदि_अन्तः
  यदि( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  यदि( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    ध्रुव वर्ण *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    यदि( zInfo==0 ) zInfo = Tcl_Get_String_परिणाम(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}
#यदि_अन्तः /* TCLSH */
