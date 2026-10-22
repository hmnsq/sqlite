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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TCLSH_INIT_PROC
# define TCLSH 1
#ಆದರೆ_ಅಂತ್ಯ

/*
** If requested, include the SQLite compiler options file for MSVC.
*/
#ಆದರೆ defined(INCLUDE_MSVC_H)
# include "msvc.h"
#ಆದರೆ_ಅಂತ್ಯ

/****** Copy of tclsqlite.h ******/
#ಆದರೆ defined(INCLUDE_SQLITE_TCL_H)
# include "sqlite_tcl.h"   /* Special case for Windows using STDCALL */
#ಇಲ್ಲದಿದ್ದರೆ
# include <tcl.h>          /* All normal cases */
# ifndef SQLITE_TCLAPI
#   define SQLITE_TCLAPI
# endif
#ಆದರೆ_ಅಂತ್ಯ
/* Compatability between Tcl8.6 and Tcl9.0 */
#ಆದರೆ TCL_MAJOR_ಆವೃತ್ತಿ==9
# define CONST ಸ್ಥಿರಾಂಕ
#ಅಥವಾಆದರೆ !defined(Tcl_ಗಾತ್ರ)
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ Tcl_ಗಾತ್ರ;
# ifndef Tcl_Bounce_Ref_ಎಣಿಕೆ
#  define Tcl_Bounce_Ref_ಎಣಿಕೆ(X) Tcl_Incr_Ref_ಎಣಿಕೆ(X); Tcl_Decr_Ref_ಎಣಿಕೆ(X)
   /* https://www.tcl-lang.org/man/tcl9.0/TclLib/Object.html */
# endif
#ಆದರೆ_ಅಂತ್ಯ
/**** End copy of tclsqlite.h ****/

#ಅನ್ತರ್ಭಾವ <errno.h>

/*
** Some additional include files are needed if this file is not
** appended to the amalgamation.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_AMALGAMATION
# include "sqlite3.h"
# include <stdlib.h>
# include <string.h>
# include <assert.h>
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ u8;
# ifndef SQLITE_PTRSIZE
#   ಆದರೆ defined(__SIZEOF_POINTER__)
#     define SQLITE_PTRSIZE __SIZEOF_POINTER__
#   elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
         defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
        (defined(__APPLE__) && defined(__POWERPC__)) ||                     \
        (defined(__TOS_AIX__) && !defined(__64BIT__))
#     define SQLITE_PTRSIZE 4
#   ಇಲ್ಲದಿದ್ದರೆ
#     define SQLITE_PTRSIZE 8
#   endif
# endif /* SQLITE_PTRSIZE */
# ಆದರೆ defined(HAVE_STDINT_H) || (defined(__STDC_VERSION__) &&  \
                                (__STDC_VERSION__ >= 199901L))
#   include <stdint.h>
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uintptr_t uptr;
# elif SQLITE_PTRSIZE==4
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ uptr;
# ಇಲ್ಲದಿದ್ದರೆ
    ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ sqlite3_uint64 uptr;
# endif
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <ctype.h>

/* Used to get the current process ID */
#ಆದರೆ !defined(_WIN32)
# include <signal.h>
# include <unistd.h>
# define GETPID getpid
#ಅಥವಾಆದರೆ !defined(_WIN32_WCE)
# ifndef SQLITE_AMALGAMATION
#  ifndef WIN32_LEAN_AND_ಸರಾಸರಿ
#   define WIN32_LEAN_AND_ಸರಾಸರಿ
#  endif
#  include <windows.h>
# endif
# include <io.h>
# define isatty(h) _isatty(h)
# define GETPID (ಪೂರ್ಣಾಂಕ)GetCurrentProcessId
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Windows needs to know which symbols to export.  Unix does not.
 * BUILD_sqlite should be undefined for Unix.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BUILD_sqlite
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು TCL_STORAGE_CLASS
#ವ್ಯಾಖ್ಯಾನಿಸು TCL_STORAGE_CLASS DLLEXPORT
#ಆದರೆ_ಅಂತ್ಯ /* BUILD_sqlite */

#ವ್ಯಾಖ್ಯಾನಿಸು ಅಂಕೆ_PREPARED_STMTS 10
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_PREPARED_STMTS 100

/* Forward declaration */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SqliteDb SqliteDb;

/* Add -DSQLITE_ENABLE_QRF_IN_TCL to add the Query Result Formatter (QRF)
** into the build of the TCL extension, when building using separate
** source files.  The QRF is included automatically when building from
** the tclsqlite3.c amalgamation.
*/
#ಆದರೆ defined(SQLITE_ENABLE_QRF_ಒಳ_TCL)
#ಅನ್ತರ್ಭಾವ "qrf.h"
#ಆದರೆ_ಅಂತ್ಯ

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SqlFunc SqlFunc;
ರಚನೆ SqlFunc {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  Tcl_Obj *pScript;     /* The Tcl_Obj representation of the script */
  SqliteDb *pDb;        /* Database connection that owns this function */
  ಪೂರ್ಣಾಂಕ useEvalObjv;      /* True if it is safe to use Tcl_EvalObjv */
  ಪೂರ್ಣಾಂಕ eType;            /* Type of value to return */
  ಅಕ್ಷರ *zName;          /* Name of this function */
  SqlFunc *p_ಮುಂದಿನ;       /* Next function on the list of them all */
};

/*
** New collation sequences function can be created as TCL scripts.  Each such
** function is described by an instance of the following structure.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SqlCollate SqlCollate;
ರಚನೆ SqlCollate {
  Tcl_Interp *interp;   /* The TCL interpret to execute the function */
  ಅಕ್ಷರ *zScript;        /* The script to be run */
  SqlCollate *p_ಮುಂದಿನ;    /* Next function on the list of them all */
};

/*
** Prepared statements are cached for faster execution.  Each prepared
** statement is described by an instance of the following structure.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SqlPreparedStmt SqlPreparedStmt;
ರಚನೆ SqlPreparedStmt {
  SqlPreparedStmt *p_ಮುಂದಿನ;  /* Next in linked list */
  SqlPreparedStmt *pPrev;  /* Previous on the list */
  sqlite3_stmt *pStmt;     /* The prepared statement */
  ಪೂರ್ಣಾಂಕ nSql;                /* chars in zSql[] */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql;        /* Text of the SQL statement */
  ಪೂರ್ಣಾಂಕ nParm;               /* Size of apParm array */
  Tcl_Obj **apParm;        /* Array of referenced object pointers */
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ IncrblobChannel IncrblobChannel;

/*
** There is one instance of this structure for each SQLite database
** that has been opened by the SQLite TCL interface.
**
** If this module is built with SQLITE_TEST defined (to create the SQLite
** testfixture executable), then it may be configured to use either
** sqlite3_prepare_v2() or sqlite3_prepare() to prepare SQL statements.
** If SqliteDb.bLegacyPrepare is true, sqlite3_prepare() is used.
*/
ರಚನೆ SqliteDb {
  sqlite3 *db;               /* The "real" database structure. MUST BE FIRST */
  Tcl_Interp *interp;        /* The interpreter used for this database */
  ಅಕ್ಷರ *zBusy;               /* The busy callback routine */
  ಅಕ್ಷರ *zCommit;             /* The commit hook callback routine */
  ಅಕ್ಷರ *zTrace;              /* The trace callback routine */
  ಅಕ್ಷರ *zTraceV2;            /* The trace_v2 callback routine */
  ಅಕ್ಷರ *zProfile;            /* The profile callback routine */
  ಅಕ್ಷರ *z_ಪ್ರಗತಿ;           /* The progress callback routine */
  ಅಕ್ಷರ *zBindFallback;       /* Callback to invoke on a binding miss */
  ಅಕ್ಷರ *zAuth;               /* The authorization callback routine */
  ಪೂರ್ಣಾಂಕ disableAuth;           /* Disable the authorizer if it exists */
  ಅಕ್ಷರ *zNull;               /* Text to substitute for an SQL NULL value */
  SqlFunc *pFunc;            /* List of SQL functions */
  Tcl_Obj *pUpdateHook;      /* Update hook script (if any) */
  Tcl_Obj *pPreUpdateHook;   /* Pre-update hook script (if any) */
  Tcl_Obj *pRollbackHook;    /* Rollback hook script (if any) */
  Tcl_Obj *pWalHook;         /* WAL hook script (if any) */
  Tcl_Obj *pUnlockNotify;    /* Unlock notify script (if any) */
  SqlCollate *pCollate;      /* List of SQL collation functions */
  ಪೂರ್ಣಾಂಕ rc;                    /* Return code of most recent sqlite3_exec() */
  Tcl_Obj *pCollateNeeded;   /* Collation needed script */
  SqlPreparedStmt *stmtList; /* List of prepared statements*/
  SqlPreparedStmt *stmt_ಕೊನೆಯ; /* Last statement in the list */
  ಪೂರ್ಣಾಂಕ maxStmt;               /* The next maximum number of stmtList */
  ಪೂರ್ಣಾಂಕ nStmt;                 /* Number of statements in stmtList */
  IncrblobChannel *pIncrblob;/* Linked list of open incrblob channels */
  ಪೂರ್ಣಾಂಕ n_ಹೆಜ್ಜೆ, n_ವಿಂಗಡಿಸು, n_ಸೂಚ್ಯಂಕ;  /* Statistics for most recent operation */
  ಪೂರ್ಣಾಂಕ nVMStep;               /* Another statistic for most recent operation */
  ಪೂರ್ಣಾಂಕ nTransaction;          /* Number of nested [transaction] methods */
  ಪೂರ್ಣಾಂಕ openFlags;             /* Flags used to open.  (SQLITE_OPEN_URI) */
  ಪೂರ್ಣಾಂಕ nRef;                  /* Delete object when this reaches 0 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಪೂರ್ಣಾಂಕ bLegacyPrepare;        /* True to use sqlite3_prepare() */
#ಆದರೆ_ಅಂತ್ಯ
};

ರಚನೆ IncrblobChannel {
  sqlite3_blob *pBlob;      /* sqlite3 blob handle */
  SqliteDb *pDb;            /* Associated database connection */
  sqlite3_int64 iSeek;      /* Current seek offset */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ isClosed;    /* TCL_CLOSE_READ or TCL_CLOSE_WRITE */
  Tcl_Channel channel;      /* Channel identifier */
  IncrblobChannel *p_ಮುಂದಿನ;   /* Linked list of all open incrblob channels */
  IncrblobChannel *pPrev;   /* Linked list of all open incrblob channels */
};

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ strlen30(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z2 = z;
  ಆಗ( *z2 ){ z2++; }
  ಹಿಂತಿರುಗಿ 0x3fffffff & (ಪೂರ್ಣಾಂಕ)(z2 - z);
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
/*
** Close all incrblob channels opened using database connection pDb.
** This is called when shutting down the database connection.
*/
ಸ್ಥಿರ ಶೂನ್ಯ closeIncrblobChannels(SqliteDb *pDb){
  IncrblobChannel *p;
  IncrblobChannel *p_ಮುಂದಿನ;

  ಗಾಗಿ(p=pDb->pIncrblob; p; p=p_ಮುಂದಿನ){
    p_ಮುಂದಿನ = p->p_ಮುಂದಿನ;

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI incrblobClose2(
  Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ flags
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ದತ್ತಾಂಶ;
  ಪೂರ್ಣಾಂಕ  rc;
  sqlite3 *db = p->pDb->db;

  ಆದರೆ( flags ){
    p->isClosed |= flags;
    ಹಿಂತಿರುಗಿ TCL_OK;
  }

  /* If we reach this point, then we really do need to close the channel */
  rc = sqlite3_blob_close(p->pBlob);

  /* Remove the channel from the SqliteDb.pIncrblob list. */
  ಆದರೆ( p->p_ಮುಂದಿನ ){
    p->p_ಮುಂದಿನ->pPrev = p->pPrev;
  }
  ಆದರೆ( p->pPrev ){
    p->pPrev->p_ಮುಂದಿನ = p->p_ಮುಂದಿನ;
  }
  ಆದರೆ( p->pDb->pIncrblob==p ){
    p->pDb->pIncrblob = p->p_ಮುಂದಿನ;
  }

  /* Free the IncrblobChannel structure */
  Tcl_Free((ಅಕ್ಷರ *)p);

  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3_errmsg(db), TCL_VOLATILE);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI incrblobClose(
  Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ,
  Tcl_Interp *interp
){
  ಹಿಂತಿರುಗಿ incrblobClose2(instance_ದತ್ತಾಂಶ, interp, 0);
}


/*
** Read data from an incremental blob channel.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI incrblob_ಒಳಹರಿವು(
  Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ,
  ಅಕ್ಷರ *buf,
  ಪೂರ್ಣಾಂಕ buf_ಗಾತ್ರ,
  ಪೂರ್ಣಾಂಕ *error_ಸಂಕೇತ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ದತ್ತಾಂಶ;
  sqlite3_int64 nRead = buf_ಗಾತ್ರ;   /* Number of bytes to read */
  sqlite3_int64 nBlob;             /* Total size of the blob */
  ಪೂರ್ಣಾಂಕ rc;                          /* sqlite error code */

  nBlob = sqlite3_blob_ಬೈಟುಗಳು(p->pBlob);
  ಆದರೆ( (p->iSeek+nRead)>nBlob ){
    nRead = nBlob-p->iSeek;
  }
  ಆದರೆ( nRead<=0 ){
    ಹಿಂತಿರುಗಿ 0;
  }

  rc = sqlite3_blob_read(p->pBlob, (ಶೂನ್ಯ *)buf, (ಪೂರ್ಣಾಂಕ)nRead, (ಪೂರ್ಣಾಂಕ)p->iSeek);
  ಆದರೆ( rc!=SQLITE_OK ){
    *error_ಸಂಕೇತ_Ptr = rc;
    ಹಿಂತಿರುಗಿ -1;
  }

  p->iSeek += nRead;
  ಹಿಂತಿರುಗಿ nRead;
}

/*
** Write data to an incremental blob channel.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI incrblobOutput(
  Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buf,
  ಪೂರ್ಣಾಂಕ ಗಮ್ಯ_Write,
  ಪೂರ್ಣಾಂಕ *error_ಸಂಕೇತ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ದತ್ತಾಂಶ;
  sqlite3_int64 nWrite = ಗಮ್ಯ_Write;   /* Number of bytes to write */
  sqlite3_int64 nBlob;              /* Total size of the blob */
  ಪೂರ್ಣಾಂಕ rc;                           /* sqlite error code */

  nBlob = sqlite3_blob_ಬೈಟುಗಳು(p->pBlob);
  ಆದರೆ( (p->iSeek+nWrite)>nBlob ){
    *error_ಸಂಕೇತ_Ptr = EINVAL;
    ಹಿಂತಿರುಗಿ -1;
  }
  ಆದರೆ( nWrite<=0 ){
    ಹಿಂತಿರುಗಿ 0;
  }

  rc = sqlite3_blob_write(p->pBlob, (ಶೂನ್ಯ*)buf,(ಪೂರ್ಣಾಂಕ)nWrite, (ಪೂರ್ಣಾಂಕ)p->iSeek);
  ಆದರೆ( rc!=SQLITE_OK ){
    *error_ಸಂಕೇತ_Ptr = EIO;
    ಹಿಂತಿರುಗಿ -1;
  }

  p->iSeek += nWrite;
  ಹಿಂತಿರುಗಿ nWrite;
}

/* The datatype of Tcl_DriverWideSeekProc changes between tcl8.6 and tcl9.0 */
#ಆದರೆ TCL_MAJOR_ಆವೃತ್ತಿ==9
# define WideSeekProcType ದೀರ್ಘ ದೀರ್ಘ
#ಇಲ್ಲದಿದ್ದರೆ
# define WideSeekProcType Tcl_WideInt
#ಆದರೆ_ಅಂತ್ಯ

/*
** Seek an incremental blob channel.
*/
ಸ್ಥಿರ WideSeekProcType SQLITE_TCLAPI incrblobWideSeek(
  Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ,
  WideSeekProcType ಸ್ಥಾನಾಂತರ,
  ಪೂರ್ಣಾಂಕ seek_ಬಹುಲಕ,
  ಪೂರ್ಣಾಂಕ *error_ಸಂಕೇತ_Ptr
){
  IncrblobChannel *p = (IncrblobChannel *)instance_ದತ್ತಾಂಶ;

  ಆಯ್ಕೆ( seek_ಬಹುಲಕ ){
    ಸ್ಥಿತಿ SEEK_SET:
      p->iSeek = ಸ್ಥಾನಾಂತರ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SEEK_CUR:
      p->iSeek += ಸ್ಥಾನಾಂತರ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SEEK_END:
      p->iSeek = sqlite3_blob_ಬೈಟುಗಳು(p->pBlob) + ಸ್ಥಾನಾಂತರ;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ: ಪ್ರತಿಜ್ಞಾ(!"Bad seekMode");
  }

  ಹಿಂತಿರುಗಿ p->iSeek;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI incrblobSeek(
  Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ,
  ದೀರ್ಘ ಸ್ಥಾನಾಂತರ,
  ಪೂರ್ಣಾಂಕ seek_ಬಹುಲಕ,
  ಪೂರ್ಣಾಂಕ *error_ಸಂಕೇತ_Ptr
){
  ಹಿಂತಿರುಗಿ incrblobWideSeek(instance_ದತ್ತಾಂಶ,ಸ್ಥಾನಾಂತರ,seek_ಬಹುಲಕ,error_ಸಂಕೇತ_Ptr);
}


ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_TCLAPI incrblobWatch(
  Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ ಬಹುಲಕ
){
  /* NO-OP */
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI incrblobHandle(
  Client_ದತ್ತಾಂಶ instance_ದತ್ತಾಂಶ,
  ಪೂರ್ಣಾಂಕ dir,
  Client_ದತ್ತಾಂಶ *hPtr
){
  ಹಿಂತಿರುಗಿ TCL_ERROR;
}

ಸ್ಥಿರ Tcl_ChannelType IncrblobChannelType = {
  "incrblob",                        /* typeName                             */
  TCL_CHANNEL_ಆವೃತ್ತಿ_5,             /*ಆವೃತ್ತಿ*/
  incrblobClose,                     /* closeProc                            */
  incrblob_ಒಳಹರಿವು,                     /* inputProc                            */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ createIncrblobChannel(
  Tcl_Interp *interp,
  SqliteDb *pDb,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸ್ತಂಭ,
  sqlite_int64 i_ಅಡ್ಡಸಾಲು,
  ಪೂರ್ಣಾಂಕ isReadonly
){
  IncrblobChannel *p;
  sqlite3 *db = pDb->db;
  sqlite3_blob *pBlob;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ flags = TCL_READABLE|(isReadonly ? 0 : TCL_WRITABLE);

  /* This variable is used to name the channels: "incrblob_[incr count]" */
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
  ಅಕ್ಷರ zChannel[64];

  rc = sqlite3_blob_open(db, zDb, z_ಕೋಷ್ಟಕ, z_ಸ್ತಂಭ, i_ಅಡ್ಡಸಾಲು, !isReadonly, &pBlob);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  p = (IncrblobChannel *)Tcl_Alloc(ಗಾತ್ರ(IncrblobChannel));
  memset(p, 0, ಗಾತ್ರ(*p));
  p->pBlob = pBlob;
  ಆದರೆ( (flags & TCL_WRITABLE)==0 ) p->isClosed |= TCL_CLOSE_WRITE;

  sqlite3_snprintf(ಗಾತ್ರ(zChannel), zChannel, "incrblob_%d", ++ಎಣಿಕೆ_2);
  p->channel = Tcl_CreateChannel(&IncrblobChannelType, zChannel, p, flags);
  Tcl_ರಿಜಿಸ್ಟರ್_Channel(interp, p->channel);

  /* Link the new channel into the SqliteDb.pIncrblob list. */
  p->p_ಮುಂದಿನ = pDb->pIncrblob;
  p->pPrev = 0;
  ಆದರೆ( p->p_ಮುಂದಿನ ){
    p->p_ಮುಂದಿನ->pPrev = p;
  }
  pDb->pIncrblob = p;
  p->pDb = pDb;

  Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)Tcl_GetChannelName(p->channel), TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಇಲ್ಲದಿದ್ದರೆ  /* else clause for "#ifndef SQLITE_OMIT_INCRBLOB" */
  #ವ್ಯಾಖ್ಯಾನಿಸು closeIncrblobChannels(pDb)
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸುರಕ್ಷಿತ_ಗಮ್ಯ_Use_Eval_Objv(Tcl_Obj *pCmd){
  /* We could try to do something with Tcl_Parse().  But we will instead
  ** just do a search for forbidden characters.  If any of the forbidden
  ** characters appear in pCmd, we will report the string as unsafe.
  */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
  Tcl_ಗಾತ್ರ n;
  z = Tcl_GetStringFromObj(pCmd, &n);
  ಆಗ( n-- > 0 ){
    ಪೂರ್ಣಾಂಕ c = *(z++);
    ಆದರೆ( c=='$' || c=='[' || c==';' ) ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Find an SqlFunc structure with the given name.  Or create a new
** one if an existing one cannot be found.  Return a pointer to the
** structure.
*/
ಸ್ಥಿರ SqlFunc *ಕಂಡುಹಿಡಿ_Sql_Func(SqliteDb *pDb, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  SqlFunc *p, *pNew;
  ಪೂರ್ಣಾಂಕ nName = strlen30(zName);
  pNew = (SqlFunc*)Tcl_Alloc( ಗಾತ್ರ(*pNew) + nName + 1 );
  pNew->zName = (ಅಕ್ಷರ*)&pNew[1];
  ಸ್ಮರಣೆ_ನಕಲು(pNew->zName, zName, nName+1);
  ಗಾಗಿ(p=pDb->pFunc; p; p=p->p_ಮುಂದಿನ){
    ಆದರೆ( sqlite3_stricmp(p->zName, pNew->zName)==0 ){
      Tcl_Free((ಅಕ್ಷರ*)pNew);
      ಹಿಂತಿರುಗಿ p;
    }
  }
  pNew->interp = pDb->interp;
  pNew->pDb = pDb;
  pNew->pScript = 0;
  pNew->p_ಮುಂದಿನ = pDb->pFunc;
  pDb->pFunc = pNew;
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Free a single SqlPreparedStmt object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ dbFreeStmt(SqlPreparedStmt *pStmt){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಆದರೆ( sqlite3_sql(pStmt->pStmt)==0 ){
    Tcl_Free((ಅಕ್ಷರ *)pStmt->zSql);
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_finalize(pStmt->pStmt);
  Tcl_Free((ಅಕ್ಷರ *)pStmt);
}

/*
** Finalize and free a list of prepared statements
*/
ಸ್ಥಿರ ಶೂನ್ಯ flushStmtCache(SqliteDb *pDb){
  SqlPreparedStmt *pPreStmt;
  SqlPreparedStmt *p_ಮುಂದಿನ;

  ಗಾಗಿ(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=p_ಮುಂದಿನ){
    p_ಮುಂದಿನ = pPreStmt->p_ಮುಂದಿನ;
    dbFreeStmt(pPreStmt);
  }
  pDb->nStmt = 0;
  pDb->stmt_ಕೊನೆಯ = 0;
  pDb->stmtList = 0;
}

/*
** Increment the reference counter on the SqliteDb object. The reference
** should be released by calling delDatabaseRef().
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_Database_Ref(SqliteDb *pDb){
  pDb->nRef++;
}

/*
** Decrement the reference counter associated with the SqliteDb object.
** If it reaches zero, delete the object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ delDatabaseRef(SqliteDb *pDb){
  ಪ್ರತಿಜ್ಞಾ( pDb->nRef>0 );
  pDb->nRef--;
  ಆದರೆ( pDb->nRef==0 ){
    flushStmtCache(pDb);
    closeIncrblobChannels(pDb);
    sqlite3_close(pDb->db);
    ಆಗ( pDb->pFunc ){
      SqlFunc *pFunc = pDb->pFunc;
      pDb->pFunc = pFunc->p_ಮುಂದಿನ;
      ಪ್ರತಿಜ್ಞಾ( pFunc->pDb==pDb );
      Tcl_Decr_Ref_ಎಣಿಕೆ(pFunc->pScript);
      Tcl_Free((ಅಕ್ಷರ*)pFunc);
    }
    ಆಗ( pDb->pCollate ){
      SqlCollate *pCollate = pDb->pCollate;
      pDb->pCollate = pCollate->p_ಮುಂದಿನ;
      Tcl_Free((ಅಕ್ಷರ*)pCollate);
    }
    ಆದರೆ( pDb->zBusy ){
      Tcl_Free(pDb->zBusy);
    }
    ಆದರೆ( pDb->zTrace ){
      Tcl_Free(pDb->zTrace);
    }
    ಆದರೆ( pDb->zTraceV2 ){
      Tcl_Free(pDb->zTraceV2);
    }
    ಆದರೆ( pDb->zProfile ){
      Tcl_Free(pDb->zProfile);
    }
    ಆದರೆ( pDb->zBindFallback ){
      Tcl_Free(pDb->zBindFallback);
    }
    ಆದರೆ( pDb->zAuth ){
      Tcl_Free(pDb->zAuth);
    }
    ಆದರೆ( pDb->zNull ){
      Tcl_Free(pDb->zNull);
    }
    ಆದರೆ( pDb->pUpdateHook ){
      Tcl_Decr_Ref_ಎಣಿಕೆ(pDb->pUpdateHook);
    }
    ಆದರೆ( pDb->pPreUpdateHook ){
      Tcl_Decr_Ref_ಎಣಿಕೆ(pDb->pPreUpdateHook);
    }
    ಆದರೆ( pDb->pRollbackHook ){
      Tcl_Decr_Ref_ಎಣಿಕೆ(pDb->pRollbackHook);
    }
    ಆದರೆ( pDb->pWalHook ){
      Tcl_Decr_Ref_ಎಣಿಕೆ(pDb->pWalHook);
    }
    ಆದರೆ( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_ಎಣಿಕೆ(pDb->pCollateNeeded);
    }
    Tcl_Free((ಅಕ್ಷರ*)pDb);
  }
}

/*
** TCL calls this procedure when an sqlite3 database command is
** deleted.
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_TCLAPI DbDeleteCmd(ಶೂನ್ಯ *db){
  SqliteDb *pDb = (SqliteDb*)db;
  delDatabaseRef(pDb);
}

/*
** This routine is called when a database file is locked while trying
** to execute SQL.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DbBusyHandler(ಶೂನ್ಯ *cd, ಪೂರ್ಣಾಂಕ nTries){
  SqliteDb *pDb = (SqliteDb*)cd;
  ಪೂರ್ಣಾಂಕ rc;
  ಅಕ್ಷರ zVal[30];

  sqlite3_snprintf(ಗಾತ್ರ(zVal), zVal, "%d", nTries);
  rc = Tcl_VarEval(pDb->interp, pDb->zBusy, " ", zVal, (ಅಕ್ಷರ*)0);
  ಆದರೆ( rc!=TCL_OK || atoi(Tcl_Get_String_ಫಲಿತಾಂಶ(pDb->interp)) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ 1;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
/*
** This routine is invoked as the 'progress callback' for the database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ Db_ಪ್ರಗತಿ_Handler(ಶೂನ್ಯ *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( pDb->z_ಪ್ರಗತಿ );
  rc = Tcl_Eval(pDb->interp, pDb->z_ಪ್ರಗತಿ);
  ಆದರೆ( rc!=TCL_OK || atoi(Tcl_Get_String_ಫಲಿತಾಂಶ(pDb->interp)) ){
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ಬಿಂದು) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite trace handler whenever a new
** block of SQL is executed.  The TCL script in pDb->zTrace is executed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ DbTraceHandler(ಶೂನ್ಯ *cd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;

  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zTrace, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_Eval(pDb->interp, Tcl_DString_ಮೌಲ್ಯ(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_ಫಲಿತಾಂಶ(pDb->interp);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE
/*
** This routine is called by the SQLite trace_v2 handler whenever a new
** supported event is generated.  Unsupported event types are ignored.
** The TCL script in pDb->zTraceV2 is executed, with the arguments for
** the event appended to it (as list elements).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DbTraceV2Handler(
  ಚಿಹ್ನೆರಹಿತ type, /* One of the SQLITE_TRACE_* event types. */
  ಶೂನ್ಯ *cd,      /* The original context data pointer. */
  ಶೂನ್ಯ *pd,      /* Primary event data, depends on event type. */
  ಶೂನ್ಯ *xd       /* Extra event data, depends on event type. */
){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_Obj *pCmd;

  ಆಯ್ಕೆ( type ){
    ಸ್ಥಿತಿ SQLITE_TRACE_STMT: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      ಅಕ್ಷರ *zSql = (ಅಕ್ಷರ *)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewStringObj(zSql, -1));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ನೇರ);
      Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
      Tcl_Reset_ಫಲಿತಾಂಶ(pDb->interp);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_TRACE_PROFILE: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;
      sqlite3_int64 ns = *(sqlite3_int64*)xd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)ns));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ನೇರ);
      Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
      Tcl_Reset_ಫಲಿತಾಂಶ(pDb->interp);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_TRACE_ಅಡ್ಡಸಾಲು: {
      sqlite3_stmt *pStmt = (sqlite3_stmt *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)pStmt));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ನೇರ);
      Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
      Tcl_Reset_ಫಲಿತಾಂಶ(pDb->interp);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SQLITE_TRACE_CLOSE: {
      sqlite3 *db = (sqlite3 *)pd;

      pCmd = Tcl_NewStringObj(pDb->zTraceV2, -1);
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
      Tcl_ListObjAppendElement(pDb->interp, pCmd,
                               Tcl_NewWideIntObj((Tcl_WideInt)(uptr)db));
      Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ನೇರ);
      Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
      Tcl_Reset_ಫಲಿತಾಂಶ(pDb->interp);
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ಬಿಂದು) && \
    !defined(SQLITE_OMIT_DEPRECATED)
/*
** This routine is called by the SQLite profile handler after a statement
** SQL has executed.  The TCL script in pDb->zProfile is evaluated.
*/
ಸ್ಥಿರ ಶೂನ್ಯ DbProfileHandler(ಶೂನ್ಯ *cd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql, sqlite_uint64 tm){
  SqliteDb *pDb = (SqliteDb*)cd;
  Tcl_DString str;
  ಅಕ್ಷರ zTm[100];

  sqlite3_snprintf(ಗಾತ್ರ(zTm)-1, zTm, "%lld", tm);
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zProfile, -1);
  Tcl_DStringAppendElement(&str, zSql);
  Tcl_DStringAppendElement(&str, zTm);
  Tcl_Eval(pDb->interp, Tcl_DString_ಮೌಲ್ಯ(&str));
  Tcl_DStringFree(&str);
  Tcl_Reset_ಫಲಿತಾಂಶ(pDb->interp);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** This routine is called when a transaction is committed.  The
** TCL script in pDb->zCommit is executed.  If it returns non-zero or
** if it throws an exception, the transaction is rolled back instead
** of being committed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DbCommitHandler(ಶೂನ್ಯ *cd){
  SqliteDb *pDb = (SqliteDb*)cd;
  ಪೂರ್ಣಾಂಕ rc;

  rc = Tcl_Eval(pDb->interp, pDb->zCommit);
  ಆದರೆ( rc!=TCL_OK || atoi(Tcl_Get_String_ಫಲಿತಾಂಶ(pDb->interp)) ){
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ DbRollbackHandler(ಶೂನ್ಯ *client_ದತ್ತಾಂಶ){
  SqliteDb *pDb = (SqliteDb*)client_ದತ್ತಾಂಶ;
  ಪ್ರತಿಜ್ಞಾ(pDb->pRollbackHook);
  ಆದರೆ( TCL_OK!=Tcl_EvalObjEx(pDb->interp, pDb->pRollbackHook, 0) ){
    Tcl_BackgroundError(pDb->interp);
  }
}

/*
** This procedure handles wal_hook callbacks.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DbWalHandler(
  ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,
  ಪೂರ್ಣಾಂಕ n_ನಮೂದು
){
  ಪೂರ್ಣಾಂಕ ret = SQLITE_OK;
  Tcl_Obj *p;
  SqliteDb *pDb = (SqliteDb*)client_ದತ್ತಾಂಶ;
  Tcl_Interp *interp = pDb->interp;
  ಪ್ರತಿಜ್ಞಾ(pDb->pWalHook);

  ಪ್ರತಿಜ್ಞಾ( db==pDb->db );
  p = Tcl_DuplicateObj(pDb->pWalHook);
  Tcl_Incr_Ref_ಎಣಿಕೆ(p);
  Tcl_ListObjAppendElement(interp, p, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(interp, p, Tcl_NewIntObj(n_ನಮೂದು));
  ಆದರೆ( TCL_OK!=Tcl_EvalObjEx(interp, p, 0)
   || TCL_OK!=Tcl_GetIntFromObj(interp, Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp), &ret)
  ){
    Tcl_BackgroundError(interp);
  }
  Tcl_Decr_Ref_ಎಣಿಕೆ(p);

  ಹಿಂತಿರುಗಿ ret;
}

#ಆದರೆ defined(SQLITE_TEST) && defined(SQLITE_ENABLE_UNLOCK_NOTIFY)
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_Test_Unlock_Notify_Vars(Tcl_Interp *interp, ಪೂರ್ಣಾಂಕ iArg, ಪೂರ್ಣಾಂಕ nArg){
  ಅಕ್ಷರ zBuf[64];
  sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "%d", iArg);
  Tcl_ಗಣ_Var(interp, "sqlite_unlock_notify_arg", zBuf, TCL_GLOBAL_ONLY);
  sqlite3_snprintf(ಗಾತ್ರ(zBuf), zBuf, "%d", nArg);
  Tcl_ಗಣ_Var(interp, "sqlite_unlock_notify_argcount", zBuf, TCL_GLOBAL_ONLY);
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಗಣ_Test_Unlock_Notify_Vars(x,y,z)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNLOCK_NOTIFY
ಸ್ಥಿರ ಶೂನ್ಯ DbUnlockNotify(ಶೂನ್ಯ **apArg, ಪೂರ್ಣಾಂಕ nArg){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<nArg; i++){
    ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ flags = (TCL_EVAL_GLOBAL|TCL_EVAL_ನೇರ);
    SqliteDb *pDb = (SqliteDb *)apArg[i];
    ಗಣ_Test_Unlock_Notify_Vars(pDb->interp, i, nArg);
    ಪ್ರತಿಜ್ಞಾ( pDb->pUnlockNotify);
    Tcl_EvalObjEx(pDb->interp, pDb->pUnlockNotify, flags);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pDb->pUnlockNotify);
    pDb->pUnlockNotify = 0;
  }
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
/*
** Pre-update hook callback.
*/
ಸ್ಥಿರ ಶೂನ್ಯ DbPreUpdateHandler(
  ಶೂನ್ಯ *p,
  sqlite3 *db,
  ಪೂರ್ಣಾಂಕ op,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTbl,
  sqlite_int64 iKey1,
  sqlite_int64 iKey2
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ಪ್ರತಿಜ್ಞಾ( (SQLITE_DELETE-1)/9 == 0 );
  ಪ್ರತಿಜ್ಞಾ( (SQLITE_ಸೇರಿಸು-1)/9 == 1 );
  ಪ್ರತಿಜ್ಞಾ( (SQLITE_UPDATE-1)/9 == 2 );
  ಪ್ರತಿಜ್ಞಾ( pDb->pPreUpdateHook );
  ಪ್ರತಿಜ್ಞಾ( db==pDb->db );
  ಪ್ರತಿಜ್ಞಾ( op==SQLITE_ಸೇರಿಸು || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pPreUpdateHook);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(iKey2));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ನೇರ);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */

ಸ್ಥಿರ ಶೂನ್ಯ DbUpdateHandler(
  ಶೂನ್ಯ *p,
  ಪೂರ್ಣಾಂಕ op,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTbl,
  sqlite_int64 rowid
){
  SqliteDb *pDb = (SqliteDb *)p;
  Tcl_Obj *pCmd;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azStr[] = {"DELETE", "INSERT", "UPDATE"};

  ಪ್ರತಿಜ್ಞಾ( (SQLITE_DELETE-1)/9 == 0 );
  ಪ್ರತಿಜ್ಞಾ( (SQLITE_ಸೇರಿಸು-1)/9 == 1 );
  ಪ್ರತಿಜ್ಞಾ( (SQLITE_UPDATE-1)/9 == 2 );

  ಪ್ರತಿಜ್ಞಾ( pDb->pUpdateHook );
  ಪ್ರತಿಜ್ಞಾ( op==SQLITE_ಸೇರಿಸು || op==SQLITE_UPDATE || op==SQLITE_DELETE );

  pCmd = Tcl_DuplicateObj(pDb->pUpdateHook);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(azStr[(op-1)/9], -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zDb, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewStringObj(zTbl, -1));
  Tcl_ListObjAppendElement(0, pCmd, Tcl_NewWideIntObj(rowid));
  Tcl_EvalObjEx(pDb->interp, pCmd, TCL_EVAL_ನೇರ);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
}

ಸ್ಥಿರ ಶೂನ್ಯ tclCollateNeeded(
  ಶೂನ್ಯ *pCtx,
  sqlite3 *db,
  ಪೂರ್ಣಾಂಕ enc,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName
){
  SqliteDb *pDb = (SqliteDb *)pCtx;
  Tcl_Obj *pScript = Tcl_DuplicateObj(pDb->pCollateNeeded);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
  Tcl_ListObjAppendElement(0, pScript, Tcl_NewStringObj(zName, -1));
  Tcl_EvalObjEx(pDb->interp, pScript, 0);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pScript);
}

/*
** This routine is called to evaluate an SQL collation function implemented
** using TCL script.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclSqlCollate(
  ಶೂನ್ಯ *pCtx,
  ಪೂರ್ಣಾಂಕ nA,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zA,
  ಪೂರ್ಣಾಂಕ nB,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zB
){
  SqlCollate *p = (SqlCollate *)pCtx;
  Tcl_Obj *pCmd;

  pCmd = Tcl_NewStringObj(p->zScript, -1);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zA, nA));
  Tcl_ListObjAppendElement(p->interp, pCmd, Tcl_NewStringObj(zB, nB));
  Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_ನೇರ);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
  ಹಿಂತಿರುಗಿ (atoi(Tcl_Get_String_ಫಲಿತಾಂಶ(p->interp)));
}

/*
** This routine is called to evaluate an SQL function implemented
** using TCL script.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tclSqlFunc(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ**argv){
  SqlFunc *p = sqlite3_user_ದತ್ತಾಂಶ(context);
  Tcl_Obj *pCmd;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( argc==0 ){
    /* If there are no arguments to the function, call Tcl_EvalObjEx on the
    ** script object directly.  This allows the TCL compiler to generate
    ** bytecode for the command on the first invocation and thus make
    ** subsequent invocations much faster. */
    pCmd = p->pScript;
    Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
    rc = Tcl_EvalObjEx(p->interp, pCmd, 0);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
  }ಇಲ್ಲದಿದ್ದರೆ{
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
    Tcl_ಗಾತ್ರ nArg;
    ಆದರೆ( Tcl_ListObjGetElements(p->interp, p->pScript, &nArg, &aArg) ){
      sqlite3_ಫಲಿತಾಂಶ_error(context, Tcl_Get_String_ಫಲಿತಾಂಶ(p->interp), -1);
      ಹಿಂತಿರುಗಿ;
    }
    pCmd = Tcl_NewListObj(nArg, aArg);
    Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
    ಗಾಗಿ(i=0; i<argc; i++){
      sqlite3_ಮೌಲ್ಯ *p_ಒಳ = argv[i];
      Tcl_Obj *pVal;

      /* Set pVal to contain the i'th column of this row. */
      ಆಯ್ಕೆ( sqlite3_ಮೌಲ್ಯ_type(p_ಒಳ) ){
        ಸ್ಥಿತಿ SQLITE_BLOB: {
          ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(p_ಒಳ);
          pVal = Tcl_NewByteArrayObj(sqlite3_ಮೌಲ್ಯ_blob(p_ಒಳ), ಬೈಟುಗಳು);
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ SQLITE_INTEGER: {
          sqlite_int64 v = sqlite3_ಮೌಲ್ಯ_int64(p_ಒಳ);
          ಆದರೆ( v>=-2147483647 && v<=2147483647 ){
            pVal = Tcl_NewIntObj((ಪೂರ್ಣಾಂಕ)v);
          }ಇಲ್ಲದಿದ್ದರೆ{
            pVal = Tcl_NewWideIntObj(v);
          }
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ SQLITE_FLOAT: {
          ದ್ವಿಗುಣ r = sqlite3_ಮೌಲ್ಯ_double(p_ಒಳ);
          pVal = Tcl_NewDoubleObj(r);
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ SQLITE_NULL: {
          pVal = Tcl_NewStringObj(p->pDb->zNull, -1);
          ನಿಲ್ಲಿಸು;
        }
        ಪೂರ್ವನಿಯೋಜಿತ: {
          ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(p_ಒಳ);
          pVal = Tcl_NewStringObj((ಅಕ್ಷರ *)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p_ಒಳ), ಬೈಟುಗಳು);
          ನಿಲ್ಲಿಸು;
        }
      }
      rc = Tcl_ListObjAppendElement(p->interp, pCmd, pVal);
      ಆದರೆ( rc ){
        Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
        sqlite3_ಫಲಿತಾಂಶ_error(context, Tcl_Get_String_ಫಲಿತಾಂಶ(p->interp), -1);
        ಹಿಂತಿರುಗಿ;
      }
    }
    ಆದರೆ( !p->useEvalObjv ){
      /* Tcl_EvalObjEx() will automatically call Tcl_EvalObjv() if pCmd
      ** is a list without a string representation.  To prevent this from
      ** happening, make sure pCmd has a valid string representation */
      Tcl_GetString(pCmd);
    }
    rc = Tcl_EvalObjEx(p->interp, pCmd, TCL_EVAL_ನೇರ);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
  }

  ಆದರೆ( TCL_BREAK==rc ){
    sqlite3_ಫಲಿತಾಂಶ_null(context);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc && rc!=TCL_RETURN ){
    sqlite3_ಫಲಿತಾಂಶ_error(context, Tcl_Get_String_ಫಲಿತಾಂಶ(p->interp), -1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    Tcl_Obj *pVar = Tcl_Get_Obj_ಫಲಿತಾಂಶ(p->interp);
    Tcl_ಗಾತ್ರ n;
    u8 *ದತ್ತಾಂಶ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType = (pVar->typePtr ? pVar->typePtr->name : "");
    ಅಕ್ಷರ c = zType[0];
    ಪೂರ್ಣಾಂಕ eType = p->eType;

    ಆದರೆ( eType==SQLITE_NULL ){
      ಆದರೆ( c=='b' && strcmp(zType,"bytearray")==0 && pVar->ಬೈಟುಗಳು==0 ){
        /* Only return a BLOB type if the Tcl variable is a bytearray and
        ** has no string representation. */
        eType = SQLITE_BLOB;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (c=='b' && pVar->ಬೈಟುಗಳು==0 && strcmp(zType,"boolean")==0 )
             || (c=='b' && pVar->ಬೈಟುಗಳು==0 && strcmp(zType,"booleanString")==0 )
             || (c=='w' && strcmp(zType,"wideInt")==0)
             || (c=='i' && strcmp(zType,"int")==0)
      ){
        eType = SQLITE_INTEGER;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='d' && strcmp(zType,"double")==0 ){
        eType = SQLITE_FLOAT;
      }ಇಲ್ಲದಿದ್ದರೆ{
        eType = SQLITE_ಪಠ್ಯ;
      }
    }

    ಆಯ್ಕೆ( eType ){
      ಸ್ಥಿತಿ SQLITE_BLOB: {
        ದತ್ತಾಂಶ = Tcl_GetByteArrayFromObj(pVar, &n);
        sqlite3_ಫಲಿತಾಂಶ_blob64(context, ದತ್ತಾಂಶ, n, SQLITE_TRANSIENT);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ SQLITE_INTEGER: {
        Tcl_WideInt v;
        ಆದರೆ( TCL_OK==Tcl_GetWideIntFromObj(0, pVar, &v) ){
          sqlite3_ಫಲಿತಾಂಶ_int64(context, v);
          ನಿಲ್ಲಿಸು;
        }
        /* fall-through */
      }
      ಸ್ಥಿತಿ SQLITE_FLOAT: {
        ದ್ವಿಗುಣ r;
        ಆದರೆ( TCL_OK==Tcl_GetDoubleFromObj(0, pVar, &r) ){
          sqlite3_ಫಲಿತಾಂಶ_double(context, r);
          ನಿಲ್ಲಿಸು;
        }
        /* fall-through */
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ದತ್ತಾಂಶ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)Tcl_GetStringFromObj(pVar, &n);
        sqlite3_ಫಲಿತಾಂಶ_text64(context, (ಅಕ್ಷರ *)ದತ್ತಾಂಶ, n, SQLITE_TRANSIENT,
                              SQLITE_UTF8);
        ನಿಲ್ಲಿಸು;
      }
    }

  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
/*
** This is the authentication function.  It appends the authentication
** type code and the two arguments to zCmd[] then invokes the result
** on the interpreter.  The reply is examined to determine if the
** authentication fails or succeeds.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ auth_callback(
  ಶೂನ್ಯ *pArg,
  ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg1,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg2,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg3,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg4
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸಂಕೇತ;
  Tcl_DString str;
  ಪೂರ್ಣಾಂಕ rc;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಉತ್ತರ;
  /* EVIDENCE-OF: R-38590-62769 The first parameter to the authorizer
  ** callback is a copy of the third parameter to the
  ** sqlite3_set_authorizer() interface.
  */
  SqliteDb *pDb = (SqliteDb*)pArg;
  ಆದರೆ( pDb->disableAuth ) ಹಿಂತಿರುಗಿ SQLITE_OK;

  /* EVIDENCE-OF: R-56518-44310 The second parameter to the callback is an
  ** integer action code that specifies the particular action to be
  ** authorized. */
  ಆಯ್ಕೆ( ಸಂಕೇತ_2 ){
    ಸ್ಥಿತಿ SQLITE_COPY              : z_ಸಂಕೇತ="SQLITE_COPY"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_ಸೂಚ್ಯಂಕ      : z_ಸಂಕೇತ="SQLITE_CREATE_INDEX"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_ಕೋಷ್ಟಕ      : z_ಸಂಕೇತ="SQLITE_CREATE_TABLE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_ತಾತ್ಕಾಲಿಕ_ಸೂಚ್ಯಂಕ : z_ಸಂಕೇತ="SQLITE_CREATE_TEMP_INDEX"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ : z_ಸಂಕೇತ="SQLITE_CREATE_TEMP_TABLE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_ತಾತ್ಕಾಲಿಕ_TRIGGER: z_ಸಂಕೇತ="SQLITE_CREATE_TEMP_TRIGGER"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_ತಾತ್ಕಾಲಿಕ_VIEW  : z_ಸಂಕೇತ="SQLITE_CREATE_TEMP_VIEW"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_TRIGGER    : z_ಸಂಕೇತ="SQLITE_CREATE_TRIGGER"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_VIEW       : z_ಸಂಕೇತ="SQLITE_CREATE_VIEW"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DELETE            : z_ಸಂಕೇತ="SQLITE_DELETE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_ಸೂಚ್ಯಂಕ        : z_ಸಂಕೇತ="SQLITE_DROP_INDEX"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_ಕೋಷ್ಟಕ        : z_ಸಂಕೇತ="SQLITE_DROP_TABLE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_ತಾತ್ಕಾಲಿಕ_ಸೂಚ್ಯಂಕ   : z_ಸಂಕೇತ="SQLITE_DROP_TEMP_INDEX"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ   : z_ಸಂಕೇತ="SQLITE_DROP_TEMP_TABLE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_ತಾತ್ಕಾಲಿಕ_TRIGGER : z_ಸಂಕೇತ="SQLITE_DROP_TEMP_TRIGGER"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_ತಾತ್ಕಾಲಿಕ_VIEW    : z_ಸಂಕೇತ="SQLITE_DROP_TEMP_VIEW"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_TRIGGER      : z_ಸಂಕೇತ="SQLITE_DROP_TRIGGER"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_VIEW         : z_ಸಂಕೇತ="SQLITE_DROP_VIEW"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_ಸೇರಿಸು            : z_ಸಂಕೇತ="SQLITE_INSERT"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_PRAGMA            : z_ಸಂಕೇತ="SQLITE_PRAGMA"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_READ              : z_ಸಂಕೇತ="SQLITE_READ"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_SELECT            : z_ಸಂಕೇತ="SQLITE_SELECT"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_TRANSACTION       : z_ಸಂಕೇತ="SQLITE_TRANSACTION"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_UPDATE            : z_ಸಂಕೇತ="SQLITE_UPDATE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_ATTACH            : z_ಸಂಕೇತ="SQLITE_ATTACH"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DETACH            : z_ಸಂಕೇತ="SQLITE_DETACH"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_ALTER_ಕೋಷ್ಟಕ       : z_ಸಂಕೇತ="SQLITE_ALTER_TABLE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_REINDEX           : z_ಸಂಕೇತ="SQLITE_REINDEX"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_ANALYZE           : z_ಸಂಕೇತ="SQLITE_ANALYZE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_CREATE_VTABLE     : z_ಸಂಕೇತ="SQLITE_CREATE_VTABLE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_DROP_VTABLE       : z_ಸಂಕೇತ="SQLITE_DROP_VTABLE"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_FUNCTION          : z_ಸಂಕೇತ="SQLITE_FUNCTION"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_SAVEPOINT         : z_ಸಂಕೇತ="SQLITE_SAVEPOINT"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_RECURSIVE         : z_ಸಂಕೇತ="SQLITE_RECURSIVE"; ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ                       : z_ಸಂಕೇತ="????"; ನಿಲ್ಲಿಸು;
  }
  Tcl_DStringInit(&str);
  Tcl_DStringAppend(&str, pDb->zAuth, -1);
  Tcl_DStringAppendElement(&str, z_ಸಂಕೇತ);
  Tcl_DStringAppendElement(&str, zArg1 ? zArg1 : "");
  Tcl_DStringAppendElement(&str, zArg2 ? zArg2 : "");
  Tcl_DStringAppendElement(&str, zArg3 ? zArg3 : "");
  Tcl_DStringAppendElement(&str, zArg4 ? zArg4 : "");
  rc = Tcl_GlobalEval(pDb->interp, Tcl_DString_ಮೌಲ್ಯ(&str));
  Tcl_DStringFree(&str);
  z_ಉತ್ತರ = rc==TCL_OK ? Tcl_Get_String_ಫಲಿತಾಂಶ(pDb->interp) : "SQLITE_DENY";
  ಆದರೆ( strcmp(z_ಉತ್ತರ,"SQLITE_OK")==0 ){
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(z_ಉತ್ತರ,"SQLITE_DENY")==0 ){
    rc = SQLITE_DENY;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(z_ಉತ್ತರ,"SQLITE_IGNORE")==0 ){
    rc = SQLITE_IGNORE;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = 999;
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_AUTHORIZATION */

#ಆದರೆ 0
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
ಸ್ಥಿರ ಅಕ್ಷರ *local_getline(ಅಕ್ಷರ *zPrompt, FILE *ಒಳ){
  ಅಕ್ಷರ *z_ಸಾಲು;
  ಪೂರ್ಣಾಂಕ n_ಸಾಲು;
  ಪೂರ್ಣಾಂಕ n;

  n_ಸಾಲು = 100;
  z_ಸಾಲು = ಸ್ಮರಣೆ_ಕೊಡು( n_ಸಾಲು );
  ಆದರೆ( z_ಸಾಲು==0 ) ಹಿಂತಿರುಗಿ 0;
  n = 0;
  ಆಗ( 1 ){
    ಆದರೆ( n+100>n_ಸಾಲು ){
      n_ಸಾಲು = n_ಸಾಲು*2 + 100;
      z_ಸಾಲು = realloc(z_ಸಾಲು, n_ಸಾಲು);
      ಆದರೆ( z_ಸಾಲು==0 ) ಹಿಂತಿರುಗಿ 0;
    }
    ಆದರೆ( fgets(&z_ಸಾಲು[n], n_ಸಾಲು - n, ಒಳ)==0 ){
      ಆದರೆ( n==0 ){
        ಬಿಡುಗಡೆ(z_ಸಾಲು);
        ಹಿಂತಿರುಗಿ 0;
      }
      z_ಸಾಲು[n] = 0;
      ನಿಲ್ಲಿಸು;
    }
    ಆಗ( z_ಸಾಲು[n] ){ n++; }
    ಆದರೆ( n>0 && z_ಸಾಲು[n-1]=='\n' ){
      n--;
      z_ಸಾಲು[n] = 0;
      ನಿಲ್ಲಿಸು;
    }
  }
  z_ಸಾಲು = realloc( z_ಸಾಲು, n+1 );
  ಹಿಂತಿರುಗಿ z_ಸಾಲು;
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** This function is part of the implementation of the command:
**
**   $db transaction [-deferred|-immediate|-exclusive] SCRIPT
**
** It is invoked after evaluating the script SCRIPT to commit or rollback
** the transaction or savepoint opened by the [transaction] command.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI DbTransPostCmd(
  Client_ದತ್ತಾಂಶ ದತ್ತಾಂಶ[],                   /* data[0] is the Sqlite3Db* for $db */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ                           /* Result of evaluating SCRIPT */
){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ az_ಕೊನೆ[] = {
    "RELEASE _tcl_transaction",        /* rc==TCL_ERROR, nTransaction!=0 */
    "COMMIT",                          /* rc!=TCL_ERROR, nTransaction==0 */
    "ROLLBACK TO _tcl_transaction ; RELEASE _tcl_transaction",
    "ROLLBACK"                         /* rc==TCL_ERROR, nTransaction==0 */
  };
  SqliteDb *pDb = (SqliteDb*)ದತ್ತಾಂಶ[0];
  ಪೂರ್ಣಾಂಕ rc = ಫಲಿತಾಂಶ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೊನೆ;

  pDb->nTransaction--;
  z_ಕೊನೆ = az_ಕೊನೆ[(rc==TCL_ERROR)*2 + (pDb->nTransaction==0)];

  pDb->disableAuth++;
  ಆದರೆ( sqlite3_exec(pDb->db, z_ಕೊನೆ, 0, 0, 0) ){
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
    ಆದರೆ( rc!=TCL_ERROR ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
    }
    sqlite3_exec(pDb->db, "ROLLBACK", 0, 0, 0);
  }
  pDb->disableAuth--;

  delDatabaseRef(pDb);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Unless SQLITE_TEST is defined, this function is a simple wrapper around
** sqlite3_prepare_v2(). If SQLITE_TEST is defined, then it uses either
** sqlite3_prepare_v2() or legacy interface sqlite3_prepare(), depending
** on whether or not the [db_use_legacy_prepare] command has been used to
** configure the connection.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbPrepare(
  SqliteDb *pDb,                  /* Database object */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql,               /* SQL to compile */
  sqlite3_stmt **ppStmt,          /* OUT: Prepared statement */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz_ಹೊರಹೋಗು              /* OUT: Pointer to next SQL statement */
){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ prepFlags = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಆದರೆ( pDb->bLegacyPrepare ){
    ಹಿಂತಿರುಗಿ sqlite3_prepare(pDb->db, zSql, -1, ppStmt, pz_ಹೊರಹೋಗು);
  }
#ಆದರೆ_ಅಂತ್ಯ
  /* If the statement cache is large, use the SQLITE_PREPARE_PERSISTENT
  ** flags, which uses less lookaside memory.  But if the cache is small,
  ** omit that flag to make full use of lookaside */
  ಆದರೆ( pDb->maxStmt>5 ) prepFlags = SQLITE_PREPARE_PERSISTENT;

  ಹಿಂತಿರುಗಿ sqlite3_prepare_v3(pDb->db, zSql, -1, prepFlags, ppStmt, pz_ಹೊರಹೋಗು);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbPrepareAndBind(
  SqliteDb *pDb,                  /* Database object */
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *z_ಒಳ,                /* SQL to compile */
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ **pz_ಹೊರಹೋಗು,             /* OUT: Pointer to next SQL statement */
  SqlPreparedStmt **ppPreStmt     /* OUT: Object used to cache statement */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = z_ಒಳ;         /* Pointer to first SQL statement in zIn */
  sqlite3_stmt *pStmt = 0;        /* Prepared statement object */
  SqlPreparedStmt *pPreStmt;      /* Pointer to cached statement */
  ಪೂರ್ಣಾಂಕ nSql;                       /* Length of zSql in bytes */
  ಪೂರ್ಣಾಂಕ nVar = 0;                   /* Number of variables in statement */
  ಪೂರ್ಣಾಂಕ iParm = 0;                  /* Next free entry in apParm */
  ಅಕ್ಷರ c;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ need_ಫಲಿತಾಂಶ_Reset = 0;        /* Need to invoke Tcl_ResetResult() */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Value to return */
  Tcl_Interp *interp = pDb->interp;

  *ppPreStmt = 0;

  /* Trim spaces from the start of zSql and calculate the remaining length. */
  ಆಗ( (c = zSql[0])==' ' || c=='\t' || c=='\r' || c=='\n' ){ zSql++; }
  nSql = strlen30(zSql);

  ಗಾಗಿ(pPreStmt = pDb->stmtList; pPreStmt; pPreStmt=pPreStmt->p_ಮುಂದಿನ){
    ಪೂರ್ಣಾಂಕ n = pPreStmt->nSql;
    ಆದರೆ( nSql>=n
        && memcmp(pPreStmt->zSql, zSql, n)==0
        && (zSql[n]==0 || zSql[n-1]==';')
    ){
      pStmt = pPreStmt->pStmt;
      *pz_ಹೊರಹೋಗು = &zSql[pPreStmt->nSql];

      /* When a prepared statement is found, unlink it from the
      ** cache list.  It will later be added back to the beginning
      ** of the cache list in order to implement LRU replacement.
      */
      ಆದರೆ( pPreStmt->pPrev ){
        pPreStmt->pPrev->p_ಮುಂದಿನ = pPreStmt->p_ಮುಂದಿನ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->stmtList = pPreStmt->p_ಮುಂದಿನ;
      }
      ಆದರೆ( pPreStmt->p_ಮುಂದಿನ ){
        pPreStmt->p_ಮುಂದಿನ->pPrev = pPreStmt->pPrev;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->stmt_ಕೊನೆಯ = pPreStmt->pPrev;
      }
      pDb->nStmt--;
      nVar = sqlite3_bind_parameter_ಎಣಿಕೆ(pStmt);
      ನಿಲ್ಲಿಸು;
    }
  }

  /* If no prepared statement was found. Compile the SQL text. Also allocate
  ** a new SqlPreparedStmt structure.  */
  ಆದರೆ( pPreStmt==0 ){
    ಪೂರ್ಣಾಂಕ nByte;

    ಆದರೆ( SQLITE_OK!=dbPrepare(pDb, zSql, &pStmt, pz_ಹೊರಹೋಗು) ){
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( pStmt==0 ){
      ಆದರೆ( SQLITE_OK!=sqlite3_errcode(pDb->db) ){
        /* A compile-time error in the statement. */
        Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* The statement was a no-op.  Continue to the next statement
        ** in the SQL string.
        */
        ಹಿಂತಿರುಗಿ TCL_OK;
      }
    }

    ಪ್ರತಿಜ್ಞಾ( pPreStmt==0 );
    nVar = sqlite3_bind_parameter_ಎಣಿಕೆ(pStmt);
    nByte = ಗಾತ್ರ(SqlPreparedStmt) + nVar*ಗಾತ್ರ(Tcl_Obj *);
    pPreStmt = (SqlPreparedStmt*)Tcl_Alloc(nByte);
    memset(pPreStmt, 0, nByte);

    pPreStmt->pStmt = pStmt;
    pPreStmt->nSql = (ಪೂರ್ಣಾಂಕ)(*pz_ಹೊರಹೋಗು - zSql);
    pPreStmt->zSql = sqlite3_sql(pStmt);
    pPreStmt->apParm = (Tcl_Obj **)&pPreStmt[1];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
    ಆದರೆ( pPreStmt->zSql==0 ){
      ಅಕ್ಷರ *zCopy = Tcl_Alloc(pPreStmt->nSql + 1);
      ಸ್ಮರಣೆ_ನಕಲು(zCopy, zSql, pPreStmt->nSql);
      zCopy[pPreStmt->nSql] = '\0';
      pPreStmt->zSql = zCopy;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಪ್ರತಿಜ್ಞಾ( pPreStmt );
  ಪ್ರತಿಜ್ಞಾ( strlen30(pPreStmt->zSql)==pPreStmt->nSql );
  ಪ್ರತಿಜ್ಞಾ( 0==memcmp(pPreStmt->zSql, zSql, pPreStmt->nSql) );

  /* Bind values to parameters that begin with $ or : */
  ಗಾಗಿ(i=1; i<=nVar; i++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVar = sqlite3_bind_parameter_name(pStmt, i);
    ಆದರೆ( zVar!=0 && (zVar[0]=='$' || zVar[0]==':' || zVar[0]=='@') ){
      Tcl_Obj *pVar = Tcl_GetVar2Ex(interp, &zVar[1], 0, 0);
      ಆದರೆ( pVar==0 && pDb->zBindFallback!=0 ){
        Tcl_Obj *pCmd;
        ಪೂರ್ಣಾಂಕ rx;
        pCmd = Tcl_NewStringObj(pDb->zBindFallback, -1);
        Tcl_Incr_Ref_ಎಣಿಕೆ(pCmd);
        Tcl_ListObjAppendElement(interp, pCmd, Tcl_NewStringObj(zVar,-1));
        ಆದರೆ( need_ಫಲಿತಾಂಶ_Reset ) Tcl_Reset_ಫಲಿತಾಂಶ(interp);
        need_ಫಲಿತಾಂಶ_Reset = 1;
        rx = Tcl_EvalObjEx(interp, pCmd, TCL_EVAL_ನೇರ);
        Tcl_Decr_Ref_ಎಣಿಕೆ(pCmd);
        ಆದರೆ( rx==TCL_OK ){
          pVar = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rx==TCL_ERROR ){
          rc = TCL_ERROR;
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pVar = 0;
        }
      }
      ಆದರೆ( pVar ){
        Tcl_ಗಾತ್ರ n;
        u8 *ದತ್ತಾಂಶ;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType = (pVar->typePtr ? pVar->typePtr->name : "");
        c = zType[0];
        ಆದರೆ( zVar[0]=='@' ||
           (c=='b' && strcmp(zType,"bytearray")==0 && pVar->ಬೈಟುಗಳು==0) ){
          /* Load a BLOB type if the Tcl variable is a bytearray and
          ** it has no string representation or the host
          ** parameter name begins with "@". */
          ದತ್ತಾಂಶ = Tcl_GetByteArrayFromObj(pVar, &n);
          sqlite3_bind_blob64(pStmt, i, ದತ್ತಾಂಶ, n, SQLITE_STATIC);
          Tcl_Incr_Ref_ಎಣಿಕೆ(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='b' && pVar->ಬೈಟುಗಳು==0 
               && (strcmp(zType,"booleanString")==0
                   || strcmp(zType,"boolean")==0)
        ){
          ಪೂರ್ಣಾಂಕ nn;
          Tcl_GetBooleanFromObj(interp, pVar, &nn);
          sqlite3_bind_int(pStmt, i, nn);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='d' && strcmp(zType,"double")==0 ){
          ದ್ವಿಗುಣ r;
          Tcl_GetDoubleFromObj(interp, pVar, &r);
          sqlite3_bind_double(pStmt, i, r);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (c=='w' && strcmp(zType,"wideInt")==0) ||
              (c=='i' && strcmp(zType,"int")==0) ){
          Tcl_WideInt v;
          Tcl_GetWideIntFromObj(interp, pVar, &v);
          sqlite3_bind_int64(pStmt, i, v);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ದತ್ತಾಂಶ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)Tcl_GetStringFromObj(pVar, &n);
          sqlite3_bind_text64(pStmt, i, (ಅಕ್ಷರ *)ದತ್ತಾಂಶ, n, SQLITE_STATIC,
                              SQLITE_UTF8);
          Tcl_Incr_Ref_ಎಣಿಕೆ(pVar);
          pPreStmt->apParm[iParm++] = pVar;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_bind_null(pStmt, i);
      }
      ಆದರೆ( need_ಫಲಿತಾಂಶ_Reset ) Tcl_Reset_ಫಲಿತಾಂಶ(pDb->interp);
    }
  }
  pPreStmt->nParm = iParm;
  *ppPreStmt = pPreStmt;
  ಆದರೆ( need_ಫಲಿತಾಂಶ_Reset && rc==TCL_OK ) Tcl_Reset_ಫಲಿತಾಂಶ(pDb->interp);

  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಶೂನ್ಯ dbReleaseStmt(
  SqliteDb *pDb,                  /* Database handle */
  SqlPreparedStmt *pPreStmt,      /* Prepared statement handle to release */
  ಪೂರ್ಣಾಂಕ discard                     /* True to delete (not cache) the pPreStmt */
){
  ಪೂರ್ಣಾಂಕ i;

  /* Free the bound string and blob parameters */
  ಗಾಗಿ(i=0; i<pPreStmt->nParm; i++){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pPreStmt->apParm[i]);
  }
  pPreStmt->nParm = 0;

  ಆದರೆ( pDb->maxStmt<=0 || discard ){
    /* If the cache is turned off, deallocated the statement */
    dbFreeStmt(pPreStmt);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Add the prepared statement to the beginning of the cache list. */
    pPreStmt->p_ಮುಂದಿನ = pDb->stmtList;
    pPreStmt->pPrev = 0;
    ಆದರೆ( pDb->stmtList ){
     pDb->stmtList->pPrev = pPreStmt;
    }
    pDb->stmtList = pPreStmt;
    ಆದರೆ( pDb->stmt_ಕೊನೆಯ==0 ){
      ಪ್ರತಿಜ್ಞಾ( pDb->nStmt==0 );
      pDb->stmt_ಕೊನೆಯ = pPreStmt;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( pDb->nStmt>0 );
    }
    pDb->nStmt++;

    /* If we have too many statement in cache, remove the surplus from
    ** the end of the cache list.  */
    ಆಗ( pDb->nStmt>pDb->maxStmt ){
      SqlPreparedStmt *p_ಕೊನೆಯ = pDb->stmt_ಕೊನೆಯ;
      pDb->stmt_ಕೊನೆಯ = p_ಕೊನೆಯ->pPrev;
      pDb->stmt_ಕೊನೆಯ->p_ಮುಂದಿನ = 0;
      pDb->nStmt--;
      dbFreeStmt(p_ಕೊನೆಯ);
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ DbEvalContext DbEvalContext;
ರಚನೆ DbEvalContext {
  SqliteDb *pDb;                  /* Database handle */
  Tcl_Obj *pSql;                  /* Object holding string zSql */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql;               /* Remaining SQL to execute */
  SqlPreparedStmt *pPreStmt;      /* Current statement */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;                       /* Number of columns returned by pStmt */
  ಪೂರ್ಣಾಂಕ evalFlags;                  /* Flags used */
  Tcl_Obj *pVarName;              /* Name of target array/dict variable */
  Tcl_Obj **ap_ಲಂಬಸಾಲು_Name;            /* Array of column names */
};

#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_EVAL_WITHOUTNULLS  0x00001  /* Unset array(*) for NULL */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_EVAL_ASDICT        0x00002  /* Use dict instead of array */

/*
** Release any cache of column names currently held as part of
** the DbEvalContext structure passed as the first argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ db_Release_ಸ್ತಂಭ_ಹೆಸರುಗಳು(DbEvalContext *p){
  ಆದರೆ( p->ap_ಲಂಬಸಾಲು_Name ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<p->n_ಲಂಬಸಾಲು; i++){
      Tcl_Decr_Ref_ಎಣಿಕೆ(p->ap_ಲಂಬಸಾಲು_Name[i]);
    }
    Tcl_Free((ಅಕ್ಷರ *)p->ap_ಲಂಬಸಾಲು_Name);
    p->ap_ಲಂಬಸಾಲು_Name = 0;
  }
  p->n_ಲಂಬಸಾಲು = 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ dbEvalInit(
  DbEvalContext *p,               /* Pointer to structure to initialize */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pSql,                  /* Object containing SQL script */
  Tcl_Obj *pVarName,              /* Name of Tcl array to set (*) element of */
  ಪೂರ್ಣಾಂಕ evalFlags                   /* Flags controlling evaluation */
){
  memset(p, 0, ಗಾತ್ರ(DbEvalContext));
  p->pDb = pDb;
  p->zSql = Tcl_GetString(pSql);
  p->pSql = pSql;
  Tcl_Incr_Ref_ಎಣಿಕೆ(pSql);
  ಆದರೆ( pVarName ){
    p->pVarName = pVarName;
    Tcl_Incr_Ref_ಎಣಿಕೆ(pVarName);
  }
  p->evalFlags = evalFlags;
  ಕೂಡಿಸು_Database_Ref(p->pDb);
}

/*
** Obtain information about the row that the DbEvalContext passed as the
** first argument currently points to.
*/
ಸ್ಥಿರ ಶೂನ್ಯ db_Eval_ಅಡ್ಡಸಾಲು_Info(
  DbEvalContext *p,               /* Evaluation context */
  ಪೂರ್ಣಾಂಕ *pn_ಲಂಬಸಾಲು,                     /* OUT: Number of column names */
  Tcl_Obj ***pap_ಲಂಬಸಾಲು_Name           /* OUT: Array of column names */
){
  /* Compute column names */
  ಆದರೆ( 0==p->ap_ಲಂಬಸಾಲು_Name ){
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    ಪೂರ್ಣಾಂಕ i;                        /* Iterator variable */
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;                     /* Number of columns returned by pStmt */
    Tcl_Obj **ap_ಲಂಬಸಾಲು_Name = 0;      /* Array of column names */

    p->n_ಲಂಬಸಾಲು = n_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಎಣಿಕೆ(pStmt);
    ಆದರೆ( n_ಲಂಬಸಾಲು>0 && (pap_ಲಂಬಸಾಲು_Name || p->pVarName) ){
      ap_ಲಂಬಸಾಲು_Name = (Tcl_Obj**)Tcl_Alloc( ಗಾತ್ರ(Tcl_Obj*)*n_ಲಂಬಸಾಲು );
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
        ap_ಲಂಬಸಾಲು_Name[i] = Tcl_NewStringObj(sqlite3_ಸ್ತಂಭ_name(pStmt,i), -1);
        Tcl_Incr_Ref_ಎಣಿಕೆ(ap_ಲಂಬಸಾಲು_Name[i]);
      }
      p->ap_ಲಂಬಸಾಲು_Name = ap_ಲಂಬಸಾಲು_Name;
    }

    /* If results are being stored in a variable then create the
    ** array(*) or dict(*) entry for that variable.
    */
    ಆದರೆ( p->pVarName ){
      Tcl_Interp *interp = p->pDb->interp;
      Tcl_Obj *p_ಲಂಬಸಾಲು_List = Tcl_NewObj();
      Tcl_Obj *p_ನಕ್ಷತ್ರ = Tcl_NewStringObj("*", -1);

      Tcl_Incr_Ref_ಎಣಿಕೆ(p_ಲಂಬಸಾಲು_List);
      Tcl_Incr_Ref_ಎಣಿಕೆ(p_ನಕ್ಷತ್ರ);
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
        Tcl_ListObjAppendElement(interp, p_ಲಂಬಸಾಲು_List, ap_ಲಂಬಸಾಲು_Name[i]);
      }
      ಆದರೆ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        Tcl_Obj_ಗಣ_Var2(interp, p->pVarName, p_ನಕ್ಷತ್ರ, p_ಲಂಬಸಾಲು_List, 0);
      }ಇಲ್ಲದಿದ್ದರೆ{
        Tcl_Obj * pDict = Tcl_ObjGetVar2(interp, p->pVarName, ಖಾಲಿ, 0);
        ಆದರೆ( !pDict ){
          pDict = Tcl_NewDictObj();
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ಆದರೆ( Tcl_DictObjPut(interp, pDict, p_ನಕ್ಷತ್ರ, p_ಲಂಬಸಾಲು_List)==TCL_OK ){
          Tcl_Obj_ಗಣ_Var2(interp, p->pVarName, ಖಾಲಿ, pDict, 0);
        }
        Tcl_Bounce_Ref_ಎಣಿಕೆ(pDict);
      }
      Tcl_Decr_Ref_ಎಣಿಕೆ(p_ನಕ್ಷತ್ರ);
      Tcl_Decr_Ref_ಎಣಿಕೆ(p_ಲಂಬಸಾಲು_List);
    }
  }

  ಆದರೆ( pap_ಲಂಬಸಾಲು_Name ){
    *pap_ಲಂಬಸಾಲು_Name = p->ap_ಲಂಬಸಾಲು_Name;
  }
  ಆದರೆ( pn_ಲಂಬಸಾಲು ){
    *pn_ಲಂಬಸಾಲು = p->n_ಲಂಬಸಾಲು;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_Eval_ಹೆಜ್ಜೆ(DbEvalContext *p){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPrevSql = 0;       /* Previous value of p->zSql */

  ಆಗ( p->zSql[0] || p->pPreStmt ){
    ಪೂರ್ಣಾಂಕ rc;
    ಆದರೆ( p->pPreStmt==0 ){
      zPrevSql = (p->zSql==zPrevSql ? 0 : p->zSql);
      rc = dbPrepareAndBind(p->pDb, p->zSql, &p->zSql, &p->pPreStmt);
      ಆದರೆ( rc!=TCL_OK ) ಹಿಂತಿರುಗಿ rc;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ rcs;
      SqliteDb *pDb = p->pDb;
      SqlPreparedStmt *pPreStmt = p->pPreStmt;
      sqlite3_stmt *pStmt = pPreStmt->pStmt;

      rcs = sqlite3_ಹೆಜ್ಜೆ(pStmt);
      ಆದರೆ( rcs==SQLITE_ಅಡ್ಡಸಾಲು ){
        ಹಿಂತಿರುಗಿ TCL_OK;
      }
      ಆದರೆ( p->pVarName ){
        db_Eval_ಅಡ್ಡಸಾಲು_Info(p, 0, 0);
      }
      rcs = sqlite3_reset(pStmt);

      pDb->n_ಹೆಜ್ಜೆ = sqlite3_stmt_ಸ್ಥಿತಿ(pStmt,SQLITE_STMTSTATUS_FULLSCAN_ಹೆಜ್ಜೆ,1);
      pDb->n_ವಿಂಗಡಿಸು = sqlite3_stmt_ಸ್ಥಿತಿ(pStmt,SQLITE_STMTSTATUS_ವಿಂಗಡಿಸು,1);
      pDb->n_ಸೂಚ್ಯಂಕ = sqlite3_stmt_ಸ್ಥಿತಿ(pStmt,SQLITE_STMTSTATUS_AUTOINDEX,1);
      pDb->nVMStep = sqlite3_stmt_ಸ್ಥಿತಿ(pStmt,SQLITE_STMTSTATUS_VM_ಹೆಜ್ಜೆ,1);
      db_Release_ಸ್ತಂಭ_ಹೆಸರುಗಳು(p);
      p->pPreStmt = 0;

      ಆದರೆ( rcs!=SQLITE_OK ){
        /* If a run-time error occurs, report the error and stop reading
        ** the SQL.  */
        dbReleaseStmt(pDb, pPreStmt, 1);
#ಆದರೆ SQLITE_TEST
        ಆದರೆ( p->pDb->bLegacyPrepare && rcs==SQLITE_SCHEMA && zPrevSql ){
          /* If the runtime error was an SQLITE_SCHEMA, and the database
          ** handle is configured to use the legacy sqlite3_prepare()
          ** interface, retry prepare()/step() on the same SQL statement.
          ** This only happens once. If there is a second SQLITE_SCHEMA
          ** error, the error will be returned to the caller. */
          p->zSql = zPrevSql;
          ಮುಂದುವರಿ;
        }
#ಆದರೆ_ಅಂತ್ಯ
        Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(pDb->interp,
                         Tcl_NewStringObj(sqlite3_errmsg(pDb->db), -1));
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }ಇಲ್ಲದಿದ್ದರೆ{
        dbReleaseStmt(pDb, pPreStmt, 0);
      }
    }
  }

  /*ಪೂರ್ಣಗೊಂಡ*/
  ಹಿಂತಿರುಗಿ TCL_BREAK;
}

/*
** Free all resources currently held by the DbEvalContext structure passed
** as the first argument. There should be exactly one call to this function
** for each call to dbEvalInit().
*/
ಸ್ಥಿರ ಶೂನ್ಯ dbEvalFinalize(DbEvalContext *p){
  ಆದರೆ( p->pPreStmt ){
    sqlite3_reset(p->pPreStmt->pStmt);
    dbReleaseStmt(p->pDb, p->pPreStmt, 0);
    p->pPreStmt = 0;
  }
  ಆದರೆ( p->pVarName ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(p->pVarName);
    p->pVarName = 0;
  }
  Tcl_Decr_Ref_ಎಣಿಕೆ(p->pSql);
  db_Release_ಸ್ತಂಭ_ಹೆಸರುಗಳು(p);
  delDatabaseRef(p->pDb);
}

/*
** Return a pointer to a Tcl_Obj structure with ref-count 0 that contains
** the value for the iCol'th column of the row currently pointed to by
** the DbEvalContext structure passed as the first argument.
*/
ಸ್ಥಿರ Tcl_Obj *db_Eval_ಸ್ತಂಭ_ಮೌಲ್ಯ(DbEvalContext *p, ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು){
  sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
  ಆಯ್ಕೆ( sqlite3_ಸ್ತಂಭ_type(pStmt, i_ಲಂಬಸಾಲು) ){
    ಸ್ಥಿತಿ SQLITE_BLOB: {
      ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು = sqlite3_ಸ್ತಂಭ_ಬೈಟುಗಳು(pStmt, i_ಲಂಬಸಾಲು);
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zBlob = sqlite3_ಸ್ತಂಭ_blob(pStmt, i_ಲಂಬಸಾಲು);
      ಆದರೆ( !zBlob ) ಬೈಟುಗಳು = 0;
      ಹಿಂತಿರುಗಿ Tcl_NewByteArrayObj((u8*)zBlob, ಬೈಟುಗಳು);
    }
    ಸ್ಥಿತಿ SQLITE_INTEGER: {
      sqlite_int64 v = sqlite3_ಸ್ತಂಭ_int64(pStmt, i_ಲಂಬಸಾಲು);
      ಆದರೆ( v>=-2147483647 && v<=2147483647 ){
        ಹಿಂತಿರುಗಿ Tcl_NewIntObj((ಪೂರ್ಣಾಂಕ)v);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ Tcl_NewWideIntObj(v);
      }
    }
    ಸ್ಥಿತಿ SQLITE_FLOAT: {
      ಹಿಂತಿರುಗಿ Tcl_NewDoubleObj(sqlite3_ಸ್ತಂಭ_double(pStmt, i_ಲಂಬಸಾಲು));
    }
    ಸ್ಥಿತಿ SQLITE_NULL: {
      ಹಿಂತಿರುಗಿ Tcl_NewStringObj(p->pDb->zNull, -1);
    }
  }

  ಹಿಂತಿರುಗಿ Tcl_NewStringObj((ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, i_ಲಂಬಸಾಲು), -1);
}

/*
** If using Tcl version 8.6 or greater, use the NR functions to avoid
** recursive evaluation of scripts by the [db eval] and [db trans]
** commands. Even if the headers used while compiling the extension
** are 8.6 or newer, the code still tests the Tcl version at runtime.
** This allows stubs-enabled builds to be used with older Tcl libraries.
*/
#ಆದರೆ TCL_MAJOR_ಆವೃತ್ತಿ>8 || !defined(TCL_MINOR_ಆವೃತ್ತಿ) \
                        || TCL_MINOR_ಆವೃತ್ತಿ>=6
# define SQLITE_TCL_NRE 1
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ DbUseNre(ಶೂನ್ಯ){
  ಪೂರ್ಣಾಂಕ major, minor;
  Tcl_Get_ಆವೃತ್ತಿ(&major, &minor, 0, 0);
  ಹಿಂತಿರುಗಿ( (major==8 && minor>=6) || major>8 );
}
#ಇಲ್ಲದಿದ್ದರೆ
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
# define Tcl_NRAddCallback(a,b,c,d,e,f) (ಶೂನ್ಯ)0
# define Tcl_NREvalObj(a,b,c) 0
# define Tcl_NRCreateCommand(a,b,c,d,e,f) (ಶೂನ್ಯ)0
#ಆದರೆ_ಅಂತ್ಯ

/*
** This function is part of the implementation of the command:
**
**   $db eval SQL ?TGT-NAME? SCRIPT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI Db_Eval_ಮುಂದಿನ_Cmd(
  Client_ದತ್ತಾಂಶ ದತ್ತಾಂಶ[],                   /* data[0] is the (DbEvalContext*) */
  Tcl_Interp *interp,                  /* Tcl interpreter */
  ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ                           /* Result so far */
){
  ಪೂರ್ಣಾಂಕ rc = ಫಲಿತಾಂಶ;                     /* Return code */

  /* The first element of the data[] array is a pointer to a DbEvalContext
  ** structure allocated using Tcl_Alloc(). The second element of data[]
  ** is a pointer to a Tcl_Obj containing the script to run for each row
  ** returned by the queries encapsulated in data[0]. */
  DbEvalContext *p = (DbEvalContext *)ದತ್ತಾಂಶ[0];
  Tcl_Obj * ಸ್ಥಿರಾಂಕ pScript = (Tcl_Obj *)ದತ್ತಾಂಶ[1];
  Tcl_Obj * ಸ್ಥಿರಾಂಕ pVarName = p->pVarName;

  ಆಗ( (rc==TCL_OK || rc==TCL_CONTINUE) && TCL_OK==(rc = db_Eval_ಹೆಜ್ಜೆ(p)) ){
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;
    Tcl_Obj **ap_ಲಂಬಸಾಲು_Name;
    db_Eval_ಅಡ್ಡಸಾಲು_Info(p, &n_ಲಂಬಸಾಲು, &ap_ಲಂಬಸಾಲು_Name);
    ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
      ಆದರೆ( pVarName==0 ){
        Tcl_Obj_ಗಣ_Var2(interp, ap_ಲಂಬಸಾಲು_Name[i], 0, db_Eval_ಸ್ತಂಭ_ಮೌಲ್ಯ(p,i), 0);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (p->evalFlags & SQLITE_EVAL_WITHOUTNULLS)!=0
             && sqlite3_ಸ್ತಂಭ_type(p->pPreStmt->pStmt, i)==SQLITE_NULL
      ){
        /* Remove NULL-containing column from the target container... */
        ಆದರೆ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
          /* Target is an array */
          Tcl_UnsetVar2(interp, Tcl_GetString(pVarName),
                        Tcl_GetString(ap_ಲಂಬಸಾಲು_Name[i]), 0);
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Target is a dict */
          Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ಖಾಲಿ, 0);
          ಆದರೆ( pDict ){
            ಆದರೆ( Tcl_IsShared(pDict) ){
              pDict = Tcl_DuplicateObj(pDict);
            }
            ಆದರೆ( Tcl_DictObjRemove(interp, pDict, ap_ಲಂಬಸಾಲು_Name[i])==TCL_OK ){
              Tcl_Obj_ಗಣ_Var2(interp, pVarName, ಖಾಲಿ, pDict, 0);
            }
            Tcl_Bounce_Ref_ಎಣಿಕೆ(pDict);
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( 0==(SQLITE_EVAL_ASDICT & p->evalFlags) ){
        /* Target is an array: set target(colName) = colValue */
        Tcl_Obj_ಗಣ_Var2(interp, pVarName, ap_ಲಂಬಸಾಲು_Name[i],
                       db_Eval_ಸ್ತಂಭ_ಮೌಲ್ಯ(p,i), 0);
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Target is a dict: set target(colName) = colValue */
        Tcl_Obj *pDict = Tcl_ObjGetVar2(interp, pVarName, ಖಾಲಿ, 0);
        ಆದರೆ( !pDict ){
          pDict = Tcl_NewDictObj();
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( Tcl_IsShared(pDict) ){
          pDict = Tcl_DuplicateObj(pDict);
        }
        ಆದರೆ( Tcl_DictObjPut(interp, pDict, ap_ಲಂಬಸಾಲು_Name[i],
                           db_Eval_ಸ್ತಂಭ_ಮೌಲ್ಯ(p,i))==TCL_OK ){
          Tcl_Obj_ಗಣ_Var2(interp, pVarName, ಖಾಲಿ, pDict, 0);
        }
        Tcl_Bounce_Ref_ಎಣಿಕೆ(pDict);
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
    ಆದರೆ( DbUseNre() ){
      Tcl_NRAddCallback(interp, Db_Eval_ಮುಂದಿನ_Cmd, (ಶೂನ್ಯ*)p, (ಶೂನ್ಯ*)pScript, 0, 0);
      ಹಿಂತಿರುಗಿ Tcl_NREvalObj(interp, pScript, 0);
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = Tcl_EvalObjEx(interp, pScript, 0);
    }
  }

  Tcl_Decr_Ref_ಎಣಿಕೆ(pScript);
  dbEvalFinalize(p);
  Tcl_Free((ಅಕ್ಷರ *)p);

  ಆದರೆ( rc==TCL_OK || rc==TCL_BREAK ){
    Tcl_Reset_ಫಲಿತಾಂಶ(interp);
    rc = TCL_OK;
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಶೂನ್ಯ DbHookCmd(
  Tcl_Interp *interp,             /* Tcl interpreter */
  SqliteDb *pDb,                  /* Database handle */
  Tcl_Obj *pArg,                  /* SCRIPT argument (or NULL) */
  Tcl_Obj **ppHook                /* Pointer to member of SqliteDb */
){
  sqlite3 *db = pDb->db;

  ಆದರೆ( *ppHook ){
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, *ppHook);
    ಆದರೆ( pArg ){
      Tcl_Decr_Ref_ಎಣಿಕೆ(*ppHook);
      *ppHook = 0;
    }
  }
  ಆದರೆ( pArg ){
    ಪ್ರತಿಜ್ಞಾ( !(*ppHook) );
    ಆದರೆ( Tcl_GetString(pArg)[0] ){
      *ppHook = pArg;
      Tcl_Incr_Ref_ಎಣಿಕೆ(*ppHook);
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
  sqlite3_preupdate_hook(db, (pDb->pPreUpdateHook?DbPreUpdateHandler:0), pDb);
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dbQrf(SqliteDb *pDb, ಪೂರ್ಣಾಂಕ objc, Tcl_Obj *ಸ್ಥಿರಾಂಕ*objv){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_QRF_H
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(pDb->interp, "QRF not available in this build", TCL_VOLATILE);
  ಹಿಂತಿರುಗಿ TCL_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
  ಅಕ್ಷರ *z_ಫಲಿತಾಂಶ = 0;             /* Result to be returned */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = 0;          /* SQL to run */
  ಪೂರ್ಣಾಂಕ i;                         /* Loop counter */
  ಪೂರ್ಣಾಂಕ rc;                        /* Result code */
  sqlite3_qrf_spec qrf;          /* Formatting spec */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azAlign[] = {
    "auto",           "bottom",          "c",
    "center",         "e",               "left",
    "middle",         "n",               "ne",
    "nw",             "right",           "s",
    "se",             "sw",              "top",
    "w",              0
  };
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aAlignMap[] = {
    QRF_ALIGN_Auto,   QRF_ALIGN_Bottom,  QRF_ALIGN_C,
    QRF_ALIGN_Center, QRF_ALIGN_E,       QRF_ALIGN_ಎಡ,
    QRF_ALIGN_ಮಧ್ಯ, QRF_ALIGN_N,       QRF_ALIGN_NE,
    QRF_ALIGN_NW,     QRF_ALIGN_ಬಲ,   QRF_ALIGN_S,
    QRF_ALIGN_SE,     QRF_ALIGN_SW,      QRF_ALIGN_ತುದಿ,
    QRF_ALIGN_W
  };

  memset(&qrf, 0, ಗಾತ್ರ(qrf));
  qrf.i_ಆವೃತ್ತಿ = 2;
  qrf.pzOutput = &z_ಫಲಿತಾಂಶ;
  ಗಾಗಿ(i=2; i<objc; i++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg = Tcl_GetString(objv[i]);
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azBool[] = { "auto", "yes", "no", "on", "off", 0 };
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aBoolMap[] = { 0, 2, 1, 2, 1 };
    ಆದರೆ( zArg[0]!='-' ){
      ಆದರೆ( zSql ){
        Tcl_Append_ಫಲಿತಾಂಶ(pDb->interp, "unknown argument: ", zArg, (ಅಕ್ಷರ*)0);
        rc = TCL_ERROR;
        ಹೋಗು format_failed;
      }
      zSql  = zArg;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( i==objc-1 ){
      Tcl_Append_ಫಲಿತಾಂಶ(pDb->interp, "option has no argument: ", zArg,(ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
      ಹೋಗು format_failed;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-style")==0 ){
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azStyles[] = {
        "auto",             "box",              "column",
        "count",            "csv",              "eqp",
        "explain",          "html",             "insert",
        "jobject",          "json",             "line",
        "list",             "markdown",         "quote",
        "stats",            "stats-est",        "stats-vm",
        "table",            0
      };
      ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aStyleMap[] = {
        QRF_STYLE_Auto,     QRF_STYLE_Box,      QRF_STYLE_ಸ್ತಂಭ,
        QRF_STYLE_ಎಣಿಕೆ,    QRF_STYLE_Csv,      QRF_STYLE_Eqp,
        QRF_STYLE_Explain,  QRF_STYLE_Html,     QRF_STYLE_ಸೇರಿಸು,
        QRF_STYLE_JObject,  QRF_STYLE_Json,     QRF_STYLE_ಸಾಲು,
        QRF_STYLE_List,     QRF_STYLE_Markdown, QRF_STYLE_Quote,
        QRF_STYLE_Stats,    QRF_STYLE_StatsEst, QRF_STYLE_StatsVm,
        QRF_STYLE_ಕೋಷ್ಟಕ
      };
      ಪೂರ್ಣಾಂಕ style;
      rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pDb->interp, objv[i+1], azStyles,
                              "format style (-style)", 0, &style);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      qrf.eStyle = aStyleMap[style];
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-esc")==0 ){
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azEsc[] = {
        "ascii",        "auto",         "off",      "symbol",   0
      };
      ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aEscMap[] = {
        QRF_ESC_Ascii,  QRF_ESC_Auto,   QRF_ESC_Off, QRF_ESC_ಚಿಹ್ನೆ
      };
      ಪೂರ್ಣಾಂಕ esc;
      rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pDb->interp, objv[i+1], azEsc,
                              "control character escape (-esc)", 0, &esc);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      qrf.eEsc = aEscMap[esc];
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-text")==0 || strcmp(zArg, "-title")==0 ){
      /* NB: --title can be "off" or "on but --text may not be.  Thus we put
      ** the "off" and "on" choices first and start the search on the
      ** thrid element of the array when processing --text */
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *az_ಪಠ್ಯ[] = {           "off",   "on",  "always",
        "auto",             "csv",              "html",
        "json",             "plain",            "relaxed",
        "sql",              "tcl",              0
      };
      ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ a_ಪಠ್ಯ_Map[] = {
        QRF_ಪಠ್ಯ_Auto,      QRF_ಪಠ್ಯ_Csv,       QRF_ಪಠ್ಯ_Html,
        QRF_ಪಠ್ಯ_Json,      QRF_ಪಠ್ಯ_Plain,     QRF_ಪಠ್ಯ_Relaxed,
        QRF_ಪಠ್ಯ_Sql,       QRF_ಪಠ್ಯ_Tcl
      };
      ಪೂರ್ಣಾಂಕ txt;
      ಪೂರ್ಣಾಂಕ k = zArg[2]=='e';
      rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pDb->interp, objv[i+1], &az_ಪಠ್ಯ[k*3], zArg,
                               0, &txt);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      ಆದರೆ( k ){
        qrf.e_ಪಠ್ಯ = a_ಪಠ್ಯ_Map[txt];
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( txt<=2 ){
        ಪ್ರತಿಜ್ಞಾ( QRF_No==1 );
        ಪ್ರತಿಜ್ಞಾ( QRF_Yes==2 );
        ಪ್ರತಿಜ್ಞಾ( QRF_Always==3 );
        qrf.bTitles = txt+QRF_No;
        qrf.eTitle = QRF_ಪಠ್ಯ_Auto;
      }ಇಲ್ಲದಿದ್ದರೆ{
        qrf.bTitles = QRF_Yes;
        qrf.eTitle = a_ಪಠ್ಯ_Map[txt-3];
      }
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-blob")==0 ){
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azBlob[] = {
        "auto",             "hex",              "json",
        "tcl",              "text",             "sql",
        "size",             0
      };
      ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aBlobMap[] = {
        QRF_BLOB_Auto,      QRF_BLOB_Hex,       QRF_BLOB_Json,
        QRF_BLOB_Tcl,       QRF_BLOB_ಪಠ್ಯ,      QRF_BLOB_Sql,
        QRF_BLOB_ಗಾತ್ರ
      };
      ಪೂರ್ಣಾಂಕ blob;
      rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pDb->interp, objv[i+1], azBlob,
                              "BLOB encoding (-blob)", 0, &blob);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      qrf.eBlob = aBlobMap[blob];
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-wordwrap")==0 ){
      ಪೂರ್ಣಾಂಕ v = 0;
      rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pDb->interp, objv[i+1], azBool,
                              "-wordwrap", 0, &v);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      qrf.b_ಶಬ್ದ_Wrap = aBoolMap[v];
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-textjsonb")==0
           || strcmp(zArg,"-splitcolumn")==0
           || strcmp(zArg,"-border")==0
           || strcmp(zArg,"-rowcount")==0
    ){
      ಪೂರ್ಣಾಂಕ v = 0;
      rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pDb->interp, objv[i+1], azBool,
                              zArg, 0, &v);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      ಆದರೆ( zArg[1]=='t' ){
        qrf.b_ಪಠ್ಯ_Jsonb = aBoolMap[v];
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zArg[1]=='b' ){
        qrf.bBorder = aBoolMap[v];
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zArg[1]=='r' ){
        qrf.b_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ = aBoolMap[v];
      }ಇಲ್ಲದಿದ್ದರೆ{
        qrf.b_ವಿಭಜಿಸು_ಸ್ತಂಭ = aBoolMap[v];
      }
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-defaultalign")==0
           || strcmp(zArg,"-titlealign")==0){
      ಪೂರ್ಣಾಂಕ ax = 0;
      rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pDb->interp, objv[i+1], azAlign,
                    zArg[1]=='d' ?  "default alignment (-defaultalign)" :
                                    "title alignment (-titlealign)",
                    0, &ax);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      ಆದರೆ( zArg[1]=='d' ){
        qrf.eDfltAlign = aAlignMap[ax];
      }ಇಲ್ಲದಿದ್ದರೆ{
        qrf.eTitleAlign = aAlignMap[ax];
      }
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-wrap")==0
           || strcmp(zArg,"-screenwidth")==0 
           || strcmp(zArg,"-linelimit")==0 
           || strcmp(zArg,"-titlelimit")==0 
    ){
      ಪೂರ್ಣಾಂಕ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      ಆದರೆ( v<QRF_MIN_ಅಗಲ ){
        v = QRF_MIN_ಅಗಲ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v>QRF_MAX_ಅಗಲ ){
        v = QRF_MAX_ಅಗಲ;
      }
      ಆದರೆ( zArg[1]=='w' ){
        qrf.nWrap = v;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zArg[1]=='s' ){
        qrf.n_Screen_ಅಗಲ = v;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zArg[1]=='t' ){
        qrf.n_Title_ಎಲ್ಲೆ = v;
      }ಇಲ್ಲದಿದ್ದರೆ{
        qrf.n_ಸಾಲು_ಎಲ್ಲೆ = v;
      }
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-charlimit")==0 ){
      ಪೂರ್ಣಾಂಕ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      ಆದರೆ( v<0 ) v = 0;
      qrf.n_Char_ಎಲ್ಲೆ = v;
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-multiinsert")==0 ){
      ಪೂರ್ಣಾಂಕ v = 0;
      rc = Tcl_GetIntFromObj(pDb->interp, objv[i+1], &v);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      ಆದರೆ( v<0 ) v = 0;
      qrf.n_Multi_ಸೇರಿಸು = v;
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-align")==0 ){
      Tcl_ಗಾತ್ರ n = 0;
      ಪೂರ್ಣಾಂಕ jj;
      rc = Tcl_List_Obj_ಉದ್ದಳತೆ(pDb->interp, objv[i+1], &n);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      sqlite3_free(qrf.aAlign);
      qrf.aAlign = sqlite3_malloc64( (n+1)*ಗಾತ್ರ(qrf.aAlign[0]) );
      ಆದರೆ( qrf.aAlign==0 ){
        Tcl_Append_ಫಲಿತಾಂಶ(pDb->interp, "out of memory", (ಅಕ್ಷರ*)0);
        rc = TCL_ERROR;
        ಹೋಗು format_failed;
      }
      memset(qrf.aAlign, 0, (n+1)*ಗಾತ್ರ(qrf.aAlign[0]));
      qrf.nAlign = n;
      ಗಾಗಿ(jj=0; jj<n; jj++){
        ಪೂರ್ಣಾಂಕ x;
        Tcl_Obj *p_ಪದ;
        rc = Tcl_List_Obj_ಸೂಚ್ಯಂಕ(pDb->interp, objv[i+1], jj, &p_ಪದ);
        ಆದರೆ( rc ) ಹೋಗು format_failed;
        rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(pDb->interp, p_ಪದ, azAlign,
                          "column alignment (-align)", 0, &x);
        ಆದರೆ( rc ) ಹೋಗು format_failed;
        qrf.aAlign[jj] = aAlignMap[x];
      }
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-widths")==0 ){
      Tcl_ಗಾತ್ರ n = 0;
      ಪೂರ್ಣಾಂಕ jj;
      rc = Tcl_List_Obj_ಉದ್ದಳತೆ(pDb->interp, objv[i+1], &n);
      ಆದರೆ( rc ) ಹೋಗು format_failed;
      sqlite3_free(qrf.a_ಅಗಲ);
      qrf.a_ಅಗಲ = sqlite3_malloc64( (n+1)*ಗಾತ್ರ(qrf.a_ಅಗಲ[0]) );
      ಆದರೆ( qrf.a_ಅಗಲ==0 ){
        Tcl_Append_ಫಲಿತಾಂಶ(pDb->interp, "out of memory", (ಅಕ್ಷರ*)0);
        rc = TCL_ERROR;
        ಹೋಗು format_failed;
      }
      memset(qrf.a_ಅಗಲ, 0, (n+1)*ಗಾತ್ರ(qrf.a_ಅಗಲ[0]));
      qrf.n_ಅಗಲ = n;
      ಗಾಗಿ(jj=0; jj<n; jj++){
        Tcl_Obj *p_ಪದ;
        ಪೂರ್ಣಾಂಕ v;
        rc = Tcl_List_Obj_ಸೂಚ್ಯಂಕ(pDb->interp, objv[i+1], jj, &p_ಪದ);
        ಆದರೆ( rc ) ಹೋಗು format_failed;
        rc = Tcl_GetIntFromObj(pDb->interp, p_ಪದ, &v);
        ಆದರೆ( v<(-QRF_MAX_ಅಗಲ) ){
          v = -QRF_MAX_ಅಗಲ;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v>QRF_MAX_ಅಗಲ ){
          v = QRF_MAX_ಅಗಲ;
        }
        qrf.a_ಅಗಲ[jj] = (ಗಿಡ್ಡ ಪೂರ್ಣಾಂಕ)v;
      }
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-columnsep")==0 ){
      qrf.z_ಸ್ತಂಭ_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-rowsep")==0 ){
      qrf.z_ಅಡ್ಡಸಾಲು_Sep = Tcl_GetString(objv[i+1]);
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-tablename")==0 ){
      qrf.z_ಕೋಷ್ಟಕ_Name = Tcl_GetString(objv[i+1]);
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-null")==0 ){
      qrf.zNull = Tcl_GetString(objv[i+1]);
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-ifmt")==0 ){
      ಅಕ್ಷರ *zFmt = Tcl_GetString(objv[i+1]);
      ಆದರೆ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_qrf_ckformat(zFmt)!=1 ){
        Tcl_Append_ಫಲಿತಾಂಶ(pDb->interp, "bad format for -ifmt", (ಅಕ್ಷರ*)0);
        rc = TCL_ERROR;
        ಹೋಗು format_failed;
      }
      qrf.zIFmt = zFmt;
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-fpfmt")==0 ){
      ಅಕ್ಷರ *zFmt = Tcl_GetString(objv[i+1]);
      ಆದರೆ( zFmt[0]==0 || strcmp(zFmt,"auto")==0 ){
        zFmt = 0;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_qrf_ckformat(zFmt)!=2 ){
        Tcl_Append_ಫಲಿತಾಂಶ(pDb->interp, "bad format for -fpfmt", (ಅಕ್ಷರ*)0);
        rc = TCL_ERROR;
        ಹೋಗು format_failed;
      }
      qrf.zFpFmt = zFmt;
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg,"-version")==0 ){
      /* Undocumented. Testing use only */
      qrf.i_ಆವೃತ್ತಿ = atoi(Tcl_GetString(objv[i+1]));
      i++;
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Append_ಫಲಿತಾಂಶ(pDb->interp, "unknown option: ", zArg, (ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
      ಹೋಗು format_failed;
    }
  }
  ಆಗ( zSql && zSql[0] ){
    SqlPreparedStmt *pStmt = 0;        /* Next statement to run */
    ಅಕ್ಷರ *zErr = 0;                    /* Error message from QRF */

    rc = dbPrepareAndBind(pDb, zSql, &zSql, &pStmt);
    ಆದರೆ( rc ) ಹೋಗು format_failed;
    ಆದರೆ( pStmt==0 ) ಮುಂದುವರಿ;
    rc = sqlite3_format_query_ಫಲಿತಾಂಶ(pStmt->pStmt, &qrf, &zErr);
    dbReleaseStmt(pDb, pStmt, 0);
    ಆದರೆ( rc ){
      Tcl_ಗಣ_ಫಲಿತಾಂಶ(pDb->interp, zErr, TCL_VOLATILE);
      sqlite3_free(zErr);
      rc = TCL_ERROR;
      ಹೋಗು format_failed;
    }
  }
  Tcl_ಗಣ_ಫಲಿತಾಂಶ(pDb->interp, z_ಫಲಿತಾಂಶ, TCL_VOLATILE);
  rc = TCL_OK;
  /* Fall through...*/
  
format_failed:
  sqlite3_free(qrf.a_ಅಗಲ);
  sqlite3_free(qrf.aAlign);
  sqlite3_free(z_ಫಲಿತಾಂಶ);
  ಹಿಂತಿರುಗಿ rc;

#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI DbObjCmd(
  ಶೂನ್ಯ *cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *ಸ್ಥಿರಾಂಕ*objv
){
  SqliteDb *pDb = (SqliteDb*)cd;
  ಪೂರ್ಣಾಂಕ choice;
  ಪೂರ್ಣಾಂಕ rc = TCL_OK;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *DB_strs[] = {
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
  ಎಣಿಕೆ DB_enum {
    DB_AUTHORIZER,            DB_BACKUP,               DB_BIND_FALLBACK,
    DB_BUSY,                  DB_CACHE,                DB_CHANGES,
    DB_CLOSE,                 DB_COLLATE,              DB_COLLATION_NEEDED,
    DB_COMMIT_HOOK,           DB_COMPLETE,             DB_CONFIG,
    DB_COPY,                  DB_DESERIALIZE,          DB_ENABLE_ತುಂಬು_EXTENSION,
    DB_ERRORCODE,             DB_ERROROFFSET,          DB_EVAL,
    DB_EXISTS,                DB_FORMAT,               DB_FUNCTION,
    DB_INCRBLOB,              DB_ಅಡಚಣೆ,            DB_ಕೊನೆಯ_ಸೇರಿಸು_ROWID,
    DB_NULLVALUE,             DB_ONECOLUMN,            DB_PREUPDATE,
    DB_PROFILE,               DB_ಪ್ರಗತಿ,             DB_REKEY,
    DB_RESTORE,               DB_ROLLBACK_HOOK,        DB_SERIALIZE,
    DB_ಸ್ಥಿತಿ,                DB_TIMEOUT,              DB_ಒಟ್ಟು_CHANGES,
    DB_TRACE,                 DB_TRACE_V2,             DB_TRANSACTION,
    DB_UNLOCK_NOTIFY,         DB_UPDATE_HOOK,          DB_ಆವೃತ್ತಿ,
    DB_WAL_HOOK
  };
  /* don't leave trailing commas on DB_enum, it confuses the AIX xlc compiler */

  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SUBCOMMAND ...");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[1], DB_strs, "option", 0, &choice) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆಯ್ಕೆ( (ಎಣಿಕೆ DB_enum)choice ){

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
  ಸ್ಥಿತಿ DB_AUTHORIZER: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTHORIZATION
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "authorization not available in this build",
                     (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( objc>3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?CALLBACK?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==2 ){
      ಆದರೆ( pDb->zAuth ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, pDb->zAuth, (ಅಕ್ಷರ*)0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ *zAuth;
      Tcl_ಗಾತ್ರ len;
      ಆದರೆ( pDb->zAuth ){
        Tcl_Free(pDb->zAuth);
      }
      zAuth = Tcl_GetStringFromObj(objv[2], &len);
      ಆದರೆ( zAuth && len>0 ){
        pDb->zAuth = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->zAuth, zAuth, len+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->zAuth = 0;
      }
      ಆದರೆ( pDb->zAuth ){
        ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*sqlite3_auth_cb)(
           ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
        pDb->interp = interp;
        sqlite3_ಗಣ_authorizer(pDb->db,(sqlite3_auth_cb)auth_callback,pDb);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಗಣ_authorizer(pDb->db, 0, 0);
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
    ನಿಲ್ಲಿಸು;
  }

  /*    $db backup ?DATABASE? FILENAME
  **
  ** Open or create a database file named FILENAME.  Transfer the
  ** content of local database DATABASE (default: "main") into the
  ** FILENAME database.
  */
  ಸ್ಥಿತಿ DB_BACKUP: {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDestFile;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSrcDb;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;

    ಆದರೆ( objc==3 ){
      zSrcDb = "main";
      zDestFile = Tcl_GetString(objv[2]);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==4 ){
      zSrcDb = Tcl_GetString(objv[2]);
      zDestFile = Tcl_GetString(objv[3]);
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?DATABASE? FILENAME");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zDestFile, &pDest,
               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE| pDb->openFlags, 0);
    ಆದರೆ( rc!=SQLITE_OK ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "cannot open target database: ",
           sqlite3_errmsg(pDest), (ಅಕ್ಷರ*)0);
      sqlite3_close(pDest);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDest, "main", pDb->db, zSrcDb);
    ಆದರೆ( pBackup==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (ಅಕ್ಷರ*)0);
      sqlite3_close(pDest);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆಗ(  (rc = sqlite3_backup_ಹೆಜ್ಜೆ(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_ಮುಕ್ತಾಯ(pBackup);
    ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
      rc = TCL_OK;
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "backup failed: ",
           sqlite3_errmsg(pDest), (ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pDest);
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ DB_BIND_FALLBACK: {
    ಆದರೆ( objc>3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?CALLBACK?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==2 ){
      ಆದರೆ( pDb->zBindFallback ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, pDb->zBindFallback, (ಅಕ್ಷರ*)0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ *zCallback;
      Tcl_ಗಾತ್ರ len;
      ಆದರೆ( pDb->zBindFallback ){
        Tcl_Free(pDb->zBindFallback);
      }
      zCallback = Tcl_GetStringFromObj(objv[2], &len);
      ಆದರೆ( zCallback && len>0 ){
        pDb->zBindFallback = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->zBindFallback, zCallback, len+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->zBindFallback = 0;
      }
    }
    ನಿಲ್ಲಿಸು;
  }

  /*    $db busy ?CALLBACK?
  **
  ** Invoke the given callback if an SQL statement attempts to open
  ** a locked database file.
  */
  ಸ್ಥಿತಿ DB_BUSY: {
    ಆದರೆ( objc>3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "CALLBACK");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==2 ){
      ಆದರೆ( pDb->zBusy ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, pDb->zBusy, (ಅಕ್ಷರ*)0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ *zBusy;
      Tcl_ಗಾತ್ರ len;
      ಆದರೆ( pDb->zBusy ){
        Tcl_Free(pDb->zBusy);
      }
      zBusy = Tcl_GetStringFromObj(objv[2], &len);
      ಆದರೆ( zBusy && len>0 ){
        pDb->zBusy = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->zBusy, zBusy, len+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->zBusy = 0;
      }
      ಆದರೆ( pDb->zBusy ){
        pDb->interp = interp;
        sqlite3_busy_handler(pDb->db, DbBusyHandler, pDb);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_busy_handler(pDb->db, 0, 0);
      }
    }
    ನಿಲ್ಲಿಸು;
  }

  /*     $db cache flush
  **     $db cache size n
  **
  ** Flush the prepared statement cache, or set the maximum number of
  ** cached statements.
  */
  ಸ್ಥಿತಿ DB_CACHE: {
    ಅಕ್ಷರ *subCmd;
    ಪೂರ್ಣಾಂಕ n;

    ಆದರೆ( objc<=2 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "cache option ?arg?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    subCmd = Tcl_GetStringFromObj( objv[2], 0 );
    ಆದರೆ( *subCmd=='f' && strcmp(subCmd,"flush")==0 ){
      ಆದರೆ( objc!=3 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "flush");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }ಇಲ್ಲದಿದ್ದರೆ{
        flushStmtCache( pDb );
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( *subCmd=='s' && strcmp(subCmd,"size")==0 ){
      ಆದರೆ( objc!=4 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "size n");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( TCL_ERROR==Tcl_GetIntFromObj(interp, objv[3], &n) ){
          Tcl_Append_ಫಲಿತಾಂಶ( interp, "cannot convert \"",
               Tcl_GetStringFromObj(objv[3],0), "\" to integer", (ಅಕ್ಷರ*)0);
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಆದರೆ( n<0 ){
            flushStmtCache( pDb );
            n = 0;
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n>MAX_PREPARED_STMTS ){
            n = MAX_PREPARED_STMTS;
          }
          pDb->maxStmt = n;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Append_ಫಲಿತಾಂಶ( interp, "bad option \"",
          Tcl_GetStringFromObj(objv[2],0), "\": must be flush or size",
          (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ನಿಲ್ಲಿಸು;
  }

  /*     $db changes
  **
  ** Return the number of rows that were modified, inserted, or deleted by
  ** the most recent INSERT, UPDATE or DELETE statement, not including
  ** any changes made by trigger programs.
  */
  ಸ್ಥಿತಿ DB_CHANGES: {
    Tcl_Obj *p_ಫಲಿತಾಂಶ;
    ಆದರೆ( objc!=2 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    p_ಫಲಿತಾಂಶ = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
    Tcl_ಗಣ_Wide_Int_Obj(p_ಫಲಿತಾಂಶ, sqlite3_changes64(pDb->db));
    ನಿಲ್ಲಿಸು;
  }

  /*    $db close
  **
  ** Shutdown the database
  */
  ಸ್ಥಿತಿ DB_CLOSE: {
    Tcl_DeleteCommand(interp, Tcl_GetStringFromObj(objv[0], 0));
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db collate NAME SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  ಸ್ಥಿತಿ DB_COLLATE: {
    SqlCollate *pCollate;
    ಅಕ್ಷರ *zName;
    ಅಕ್ಷರ *zScript;
    Tcl_ಗಾತ್ರ nScript;
    ಆದರೆ( objc!=4 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "NAME SCRIPT");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    zName = Tcl_GetStringFromObj(objv[2], 0);
    zScript = Tcl_GetStringFromObj(objv[3], &nScript);
    pCollate = (SqlCollate*)Tcl_Alloc( ಗಾತ್ರ(*pCollate) + nScript + 1 );
    ಆದರೆ( pCollate==0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
    pCollate->interp = interp;
    pCollate->p_ಮುಂದಿನ = pDb->pCollate;
    pCollate->zScript = (ಅಕ್ಷರ*)&pCollate[1];
    pDb->pCollate = pCollate;
    ಸ್ಮರಣೆ_ನಕಲು(pCollate->zScript, zScript, nScript+1);
    ಆದರೆ( sqlite3_create_collation(pDb->db, zName, SQLITE_UTF8,
        pCollate, tclSqlCollate) ){
      Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db collation_needed SCRIPT
  **
  ** Create a new SQL collation function called NAME.  Whenever
  ** that function is called, invoke SCRIPT to evaluate the function.
  */
  ಸ್ಥಿತಿ DB_COLLATION_NEEDED: {
    ಆದರೆ( objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "SCRIPT");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( pDb->pCollateNeeded ){
      Tcl_Decr_Ref_ಎಣಿಕೆ(pDb->pCollateNeeded);
    }
    pDb->pCollateNeeded = Tcl_DuplicateObj(objv[2]);
    Tcl_Incr_Ref_ಎಣಿಕೆ(pDb->pCollateNeeded);
    sqlite3_collation_needed(pDb->db, pDb, tclCollateNeeded);
    ನಿಲ್ಲಿಸು;
  }

  /*    $db commit_hook ?CALLBACK?
  **
  ** Invoke the given callback just before committing every SQL transaction.
  ** If the callback throws an exception or returns non-zero, then the
  ** transaction is aborted.  If CALLBACK is an empty string, the callback
  ** is disabled.
  */
  ಸ್ಥಿತಿ DB_COMMIT_HOOK: {
    ಆದರೆ( objc>3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?CALLBACK?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==2 ){
      ಆದರೆ( pDb->zCommit ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, pDb->zCommit, (ಅಕ್ಷರ*)0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCommit;
      Tcl_ಗಾತ್ರ len;
      ಆದರೆ( pDb->zCommit ){
        Tcl_Free(pDb->zCommit);
      }
      zCommit = Tcl_GetStringFromObj(objv[2], &len);
      ಆದರೆ( zCommit && len>0 ){
        pDb->zCommit = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->zCommit, zCommit, len+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->zCommit = 0;
      }
      ಆದರೆ( pDb->zCommit ){
        pDb->interp = interp;
        sqlite3_commit_hook(pDb->db, DbCommitHandler, pDb);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_commit_hook(pDb->db, 0, 0);
      }
    }
    ನಿಲ್ಲಿಸು;
  }

  /*    $db complete SQL
  **
  ** Return TRUE if SQL is a complete SQL statement.  Return FALSE if
  ** additional lines of input are needed.  This is similar to the
  ** built-in "info complete" command of Tcl.
  */
  ಸ್ಥಿತಿ DB_COMPLETE: {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPLETE
    Tcl_Obj *p_ಫಲಿತಾಂಶ;
    ಪೂರ್ಣಾಂಕ isComplete;
    ಆದರೆ( objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "SQL");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    isComplete = sqlite3_complete( Tcl_GetStringFromObj(objv[2], 0) );
    p_ಫಲಿತಾಂಶ = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
    Tcl_ಗಣ_Boolean_Obj(p_ಫಲಿತಾಂಶ, isComplete);
#ಆದರೆ_ಅಂತ್ಯ
    ನಿಲ್ಲಿಸು;
  }

  /*    $db config ?OPTION? ?BOOLEAN?
  **
  ** Configure the database connection using the sqlite3_db_config()
  ** interface.
  */
  ಸ್ಥಿತಿ DB_CONFIG: {
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ DbConfigChoices {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
      ಪೂರ್ಣಾಂಕ op;
    } aDbConfig[] = {
        { "defensive",          SQLITE_DBCONFIG_DEFENSIVE             },
        { "dqs_ddl",            SQLITE_DBCONFIG_DQS_DDL               },
        { "dqs_dml",            SQLITE_DBCONFIG_DQS_DML               },
        { "enable_fkey",        SQLITE_DBCONFIG_ENABLE_FKEY           },
        { "enable_qpsg",        SQLITE_DBCONFIG_ENABLE_QPSG           },
        { "enable_trigger",     SQLITE_DBCONFIG_ENABLE_TRIGGER        },
        { "enable_view",        SQLITE_DBCONFIG_ENABLE_VIEW           },
        { "fts3_tokenizer",     SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER },
        { "legacy_alter_table", SQLITE_DBCONFIG_LEGACY_ALTER_ಕೋಷ್ಟಕ    },
        { "legacy_file_format", SQLITE_DBCONFIG_LEGACY_FILE_FORMAT    },
        { "load_extension",     SQLITE_DBCONFIG_ENABLE_ತುಂಬು_EXTENSION },
        { "no_ckpt_on_close",   SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE      },
        { "reset_database",     SQLITE_DBCONFIG_RESET_DATABASE        },
        { "trigger_eqp",        SQLITE_DBCONFIG_TRIGGER_EQP           },
        { "trusted_schema",     SQLITE_DBCONFIG_TRUSTED_SCHEMA        },
        { "writable_schema",    SQLITE_DBCONFIG_WRITABLE_SCHEMA       },
    };
    Tcl_Obj *p_ಫಲಿತಾಂಶ;
    ಪೂರ್ಣಾಂಕ ii;
    ಆದರೆ( objc>4 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?OPTION? ?BOOLEAN?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( objc==2 ){
      /* With no arguments, list all configuration options and with the
      ** current value */
      p_ಫಲಿತಾಂಶ = Tcl_NewListObj(0,0);
      ಗಾಗಿ(ii=0; ii<ಗಾತ್ರ(aDbConfig)/ಗಾತ್ರ(aDbConfig[0]); ii++){
        ಪೂರ್ಣಾಂಕ v = 0;
        sqlite3_db_config(pDb->db, aDbConfig[ii].op, -1, &v);
        Tcl_ListObjAppendElement(interp, p_ಫಲಿತಾಂಶ,
           Tcl_NewStringObj(aDbConfig[ii].zName,-1));
        Tcl_ListObjAppendElement(interp, p_ಫಲಿತಾಂಶ,
           Tcl_NewIntObj(v));
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOpt = Tcl_GetString(objv[2]);
      ಪೂರ್ಣಾಂಕ onoff = -1;
      ಪೂರ್ಣಾಂಕ v = 0;
      ಆದರೆ( zOpt[0]=='-' ) zOpt++;
      ಗಾಗಿ(ii=0; ii<ಗಾತ್ರ(aDbConfig)/ಗಾತ್ರ(aDbConfig[0]); ii++){
        ಆದರೆ( strcmp(aDbConfig[ii].zName, zOpt)==0 ) ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( ii>=ಗಾತ್ರ(aDbConfig)/ಗಾತ್ರ(aDbConfig[0]) ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "unknown config option: \"", zOpt,
                                "\"", (ಶೂನ್ಯ*)0);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( objc==4 ){
        ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[3], &onoff) ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
      }
      sqlite3_db_config(pDb->db, aDbConfig[ii].op, onoff, &v);
      p_ಫಲಿತಾಂಶ = Tcl_NewIntObj(v);
    }
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, p_ಫಲಿತಾಂಶ);
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ DB_COPY: {
    ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ;               /* Insert data into this table */
    ಅಕ್ಷರ *zFile;                /* The file from which to extract data */
    ಅಕ್ಷರ *zConflict;            /* The conflict algorithm to use */
    sqlite3_stmt *pStmt;        /* A statement */
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;                   /* Number of columns in the table */
    ಪೂರ್ಣಾಂಕ nByte;                  /* Number of bytes in an SQL string */
    ಪೂರ್ಣಾಂಕ i, j;                   /* Loop counters */
    ಪೂರ್ಣಾಂಕ nSep;                   /* Number of bytes in zSep[] */
    ಪೂರ್ಣಾಂಕ nNull;                  /* Number of bytes in zNull[] */
    ಅಕ್ಷರ *zSql;                 /* An SQL statement */
    ಅಕ್ಷರ *z_ಸಾಲು;                /* A single line of input from the file */
    ಅಕ್ಷರ **az_ಲಂಬಸಾಲು;               /* zLine[] broken up into columns */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCommit;        /* How to commit changes */
    Tcl_Channel ಒಳ;             /* The input file */
    ಪೂರ್ಣಾಂಕ lineno = 0;             /* Line number of input file */
    ಅಕ್ಷರ z_ಸಾಲು_ಅಂಕೆ[80];          /* Line number print buffer */
    Tcl_Obj *str;
    Tcl_Obj *p_ಫಲಿತಾಂಶ;           /* interp result */

    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSep;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNull;
    ಆದರೆ( objc<5 || objc>7 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv,
         "CONFLICT-ALGORITHM TABLE FILENAME ?SEPARATOR? ?NULLINDICATOR?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( objc>=6 ){
      zSep = Tcl_GetStringFromObj(objv[5], 0);
    }ಇಲ್ಲದಿದ್ದರೆ{
      zSep = "\t";
    }
    ಆದರೆ( objc>=7 ){
      zNull = Tcl_GetStringFromObj(objv[6], 0);
    }ಇಲ್ಲದಿದ್ದರೆ{
      zNull = "";
    }
    zConflict = Tcl_GetStringFromObj(objv[2], 0);
    z_ಕೋಷ್ಟಕ = Tcl_GetStringFromObj(objv[3], 0);
    zFile = Tcl_GetStringFromObj(objv[4], 0);
    nSep = strlen30(zSep);
    nNull = strlen30(zNull);
    ಆದರೆ( nSep==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp,"Error: non-null separator required for copy",
                       (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ(strcmp(zConflict, "rollback") != 0 &&
       strcmp(zConflict, "abort"   ) != 0 &&
       strcmp(zConflict, "fail"    ) != 0 &&
       strcmp(zConflict, "ignore"  ) != 0 &&
       strcmp(zConflict, "replace" ) != 0 ) {
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error: \"", zConflict,
            "\", conflict-algorithm must be one of: rollback, "
            "abort, fail, ignore, or replace", (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    zSql = sqlite3_mprintf("SELECT * FROM '%q'", z_ಕೋಷ್ಟಕ);
    ಆದರೆ( zSql==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error: no such table: ", z_ಕೋಷ್ಟಕ, (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    nByte = strlen30(zSql);
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    ಆದರೆ( rc ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error: ", sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
      n_ಲಂಬಸಾಲು = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಎಣಿಕೆ(pStmt);
    }
    sqlite3_finalize(pStmt);
    ಆದರೆ( n_ಲಂಬಸಾಲು==0 ) {
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    zSql = ಸ್ಮರಣೆ_ಕೊಡು( nByte + 50 + n_ಲಂಬಸಾಲು*2 );
    ಆದರೆ( zSql==0 ) {
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error: can't malloc()", (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    sqlite3_snprintf(nByte+50, zSql, "INSERT OR %q INTO '%q' VALUES(?",
         zConflict, z_ಕೋಷ್ಟಕ);
    j = strlen30(zSql);
    ಗಾಗಿ(i=1; i<n_ಲಂಬಸಾಲು; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(pDb->db, zSql, -1, &pStmt, 0);
    ಬಿಡುಗಡೆ(zSql);
    ಆದರೆ( rc ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error: ", sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
      sqlite3_finalize(pStmt);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಒಳ = Tcl_OpenFileChannel(interp, zFile, "rb", 0666);
    ಆದರೆ( ಒಳ==0 ){
      sqlite3_finalize(pStmt);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    Tcl_ಗಣ_Channel_Option(ಖಾಲಿ, ಒಳ, "-translation", "auto");
    az_ಲಂಬಸಾಲು = ಸ್ಮರಣೆ_ಕೊಡು( ಗಾತ್ರ(az_ಲಂಬಸಾಲು[0])*(n_ಲಂಬಸಾಲು+1) );
    ಆದರೆ( az_ಲಂಬಸಾಲು==0 ) {
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "Error: can't malloc()", (ಅಕ್ಷರ*)0);
      Tcl_Close(interp, ಒಳ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    str = Tcl_NewObj();
    Tcl_Incr_Ref_ಎಣಿಕೆ(str);
    (ಶೂನ್ಯ)sqlite3_exec(pDb->db, "BEGIN", 0, 0, 0);
    zCommit = "COMMIT";
    ಆಗ( Tcl_GetsObj(ಒಳ, str)>=0 ) {
      ಅಕ್ಷರ *z;
      Tcl_ಗಾತ್ರ byteLen;
      lineno++;
      z_ಸಾಲು = (ಅಕ್ಷರ *)Tcl_GetByteArrayFromObj(str, &byteLen);
      az_ಲಂಬಸಾಲು[0] = z_ಸಾಲು;
      ಗಾಗಿ(i=0, z=z_ಸಾಲು; *z; z++){
        ಆದರೆ( *z==zSep[0] && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z, zSep, nSep)==0 ){
          *z = 0;
          i++;
          ಆದರೆ( i<n_ಲಂಬಸಾಲು ){
            az_ಲಂಬಸಾಲು[i] = &z[nSep];
            z += nSep-1;
          }
        }
      }
      ಆದರೆ( i+1!=n_ಲಂಬಸಾಲು ){
        ಅಕ್ಷರ *zErr;
        ಪೂರ್ಣಾಂಕ nErr = strlen30(zFile) + 200;
        zErr = ಸ್ಮರಣೆ_ಕೊಡು(nErr);
        ಆದರೆ( zErr ){
          sqlite3_snprintf(nErr, zErr,
             "Error: %s line %d: expected %d columns of data but found %d",
             zFile, lineno, n_ಲಂಬಸಾಲು, i+1);
          Tcl_Append_ಫಲಿತಾಂಶ(interp, zErr, (ಅಕ್ಷರ*)0);
          ಬಿಡುಗಡೆ(zErr);
        }
        zCommit = "ROLLBACK";
        ನಿಲ್ಲಿಸು;
      }
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
        /* check for null data, if so, bind as null */
        ಆದರೆ( (nNull>0 && strcmp(az_ಲಂಬಸಾಲು[i], zNull)==0)
          || strlen30(az_ಲಂಬಸಾಲು[i])==0
        ){
          sqlite3_bind_null(pStmt, i+1);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_bind_ಪಠ್ಯ(pStmt, i+1, az_ಲಂಬಸಾಲು[i], -1, SQLITE_STATIC);
        }
      }
      sqlite3_ಹೆಜ್ಜೆ(pStmt);
      rc = sqlite3_reset(pStmt);
      Tcl_ಗಣ_Obj_ಉದ್ದಳತೆ(str, 0);
      ಆದರೆ( rc!=SQLITE_OK ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp,"Error: ", sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
        zCommit = "ROLLBACK";
        ನಿಲ್ಲಿಸು;
      }
    }
    Tcl_Decr_Ref_ಎಣಿಕೆ(str);
    ಬಿಡುಗಡೆ(az_ಲಂಬಸಾಲು);
    Tcl_Close(interp, ಒಳ);
    sqlite3_finalize(pStmt);
    (ಶೂನ್ಯ)sqlite3_exec(pDb->db, zCommit, 0, 0, 0);

    ಆದರೆ( zCommit[0] == 'C' ){
      /* success, set result as number of lines processed */
      p_ಫಲಿತಾಂಶ = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
      ಆದರೆ( Tcl_IsShared(p_ಫಲಿತಾಂಶ) ){
        p_ಫಲಿತಾಂಶ = Tcl_DuplicateObj(p_ಫಲಿತಾಂಶ);
      }
      Tcl_ಗಣ_Int_Obj(p_ಫಲಿತಾಂಶ, lineno);
      rc = TCL_OK;
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* failure, append lineno where failed */
      sqlite3_snprintf(ಗಾತ್ರ(z_ಸಾಲು_ಅಂಕೆ), z_ಸಾಲು_ಅಂಕೆ,"%d",lineno);
      Tcl_Append_ಫಲಿತಾಂಶ(interp,", failed while processing line: ",z_ಸಾಲು_ಅಂಕೆ,
                       (ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
    }
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db deserialize ?-maxsize N? ?-readonly BOOL? ?DATABASE? VALUE
  **
  ** Reopen DATABASE (default "main") using the content in $VALUE
  */
  ಸ್ಥಿತಿ DB_DESERIALIZE: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_DESERIALIZE
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "MEMDB not available in this build",
                     (ಅಕ್ಷರ*)0);
    rc = TCL_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema = 0;
    Tcl_Obj *p_ಮೌಲ್ಯ = 0;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pBA;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p_ದತ್ತಾಂಶ;
    Tcl_ಗಾತ್ರ len;
    ಪೂರ್ಣಾಂಕ xrc;
    sqlite3_int64 mx_ಗಾತ್ರ = 0;
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ isReadonly = 0;


    ಆದರೆ( objc<3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?DATABASE? VALUE");
      rc = TCL_ERROR;
      ನಿಲ್ಲಿಸು;
    }
    ಗಾಗಿ(i=2; i<objc-1; i++){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = Tcl_GetString(objv[i]);
      ಆದರೆ( strcmp(z,"-maxsize")==0 && i<objc-2 ){
        Tcl_WideInt x;
        rc = Tcl_GetWideIntFromObj(interp, objv[++i], &x);
        ಆದರೆ( rc ) ಹೋಗು deserialize_error;
        mx_ಗಾತ್ರ = x;
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( strcmp(z,"-readonly")==0 && i<objc-2 ){
        rc = Tcl_GetBooleanFromObj(interp, objv[++i], &isReadonly);
        ಆದರೆ( rc ) ಹೋಗು deserialize_error;
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( zSchema==0 && i==objc-2 && z[0]!='-' ){
        zSchema = z;
        ಮುಂದುವರಿ;
      }
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "unknown option: ", z, (ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
      ಹೋಗು deserialize_error;
    }
    p_ಮೌಲ್ಯ = objv[objc-1];
    pBA = Tcl_GetByteArrayFromObj(p_ಮೌಲ್ಯ, &len);
    p_ದತ್ತಾಂಶ = sqlite3_malloc64( len );
    ಆದರೆ( p_ದತ್ತಾಂಶ==0 && len>0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "out of memory", (ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ flags;
      ಆದರೆ( len>0 ) ಸ್ಮರಣೆ_ನಕಲು(p_ದತ್ತಾಂಶ, pBA, len);
      ಆದರೆ( isReadonly ){
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_READONLY;
      }ಇಲ್ಲದಿದ್ದರೆ{
        flags = SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE;
      }
      xrc = sqlite3_deserialize(pDb->db, zSchema, p_ದತ್ತಾಂಶ, len, len, flags);
      ಆದರೆ( xrc ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "unable to set MEMDB content", (ಅಕ್ಷರ*)0);
        rc = TCL_ERROR;
      }
      ಆದರೆ( mx_ಗಾತ್ರ>0 ){
        sqlite3_file_control(pDb->db, zSchema,SQLITE_FCNTL_ಗಾತ್ರ_ಎಲ್ಲೆ,&mx_ಗಾತ್ರ);
      }
    }
deserialize_error:
#ಆದರೆ_ಅಂತ್ಯ
    ನಿಲ್ಲಿಸು; 
  }

  /*
  **    $db enable_load_extension BOOLEAN
  **
  ** Turn the extension loading feature on or off.  It if off by
  ** default.
  */
  ಸ್ಥಿತಿ DB_ENABLE_ತುಂಬು_EXTENSION: {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
    ಪೂರ್ಣಾಂಕ onoff;
    ಆದರೆ( objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "BOOLEAN");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[2], &onoff) ){
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    sqlite3_enable_ತುಂಬು_extension(pDb->db, onoff);
    ನಿಲ್ಲಿಸು;
#ಇಲ್ಲದಿದ್ದರೆ
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "extension loading is turned off at compile-time",
                     (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
#ಆದರೆ_ಅಂತ್ಯ
  }

  /*
  **    $db errorcode
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  ಸ್ಥಿತಿ DB_ERRORCODE: {
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(sqlite3_errcode(pDb->db)));
    ನಿಲ್ಲಿಸು;
  }

  /*
  **    $db erroroffset
  **
  ** Return the numeric error code that was returned by the most recent
  ** call to sqlite3_exec().
  */
  ಸ್ಥಿತಿ DB_ERROROFFSET: {
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(sqlite3_error_ಸ್ಥಾನಾಂತರ(pDb->db)));
    ನಿಲ್ಲಿಸು;
  }

  /*
  **    $db exists $sql
  **    $db onecolumn $sql
  **
  ** The onecolumn method is the equivalent of:
  **     lindex [$db eval $sql] 0
  */
  ಸ್ಥಿತಿ DB_EXISTS:
  ಸ್ಥಿತಿ DB_ONECOLUMN: {
    Tcl_Obj *p_ಫಲಿತಾಂಶ = 0;
    DbEvalContext sEval;
    ಆದರೆ( objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "SQL");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }

    dbEvalInit(&sEval, pDb, objv[2], 0, 0);
    rc = db_Eval_ಹೆಜ್ಜೆ(&sEval);
    ಆದರೆ( choice==DB_ONECOLUMN ){
      ಆದರೆ( rc==TCL_OK ){
        p_ಫಲಿತಾಂಶ = db_Eval_ಸ್ತಂಭ_ಮೌಲ್ಯ(&sEval, 0);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==TCL_BREAK ){
        Tcl_Reset_ಫಲಿತಾಂಶ(interp);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==TCL_BREAK || rc==TCL_OK ){
      p_ಫಲಿತಾಂಶ = Tcl_NewBooleanObj(rc==TCL_OK);
    }
    dbEvalFinalize(&sEval);
    ಆದರೆ( p_ಫಲಿತಾಂಶ ) Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, p_ಫಲಿತಾಂಶ);

    ಆದರೆ( rc==TCL_BREAK ){
      rc = TCL_OK;
    }
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ DB_EVAL: {
    ಪೂರ್ಣಾಂಕ evalFlags = 0;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOpt;
    ಆಗ( objc>3 && (zOpt = Tcl_GetString(objv[2]))!=0 && zOpt[0]=='-' ){
      ಆದರೆ( strcmp(zOpt, "-withoutnulls")==0 ){
        evalFlags |= SQLITE_EVAL_WITHOUTNULLS;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zOpt, "-asdict")==0 ){
        evalFlags |= SQLITE_EVAL_ASDICT;
      }ಇಲ್ಲದಿದ್ದರೆ{
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "unknown option: \"", zOpt, "\"", (ಶೂನ್ಯ*)0);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      objc--;
      objv++;
    }
    ಆದರೆ( objc<3 || objc>5 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv,
          "?OPTIONS? SQL ?VAR-NAME? ?SCRIPT?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }

    ಆದರೆ( objc==3 ){
      DbEvalContext sEval;
      Tcl_Obj *pRet = Tcl_NewObj();
      Tcl_Incr_Ref_ಎಣಿಕೆ(pRet);
      dbEvalInit(&sEval, pDb, objv[2], 0, 0);
      ಆಗ( TCL_OK==(rc = db_Eval_ಹೆಜ್ಜೆ(&sEval)) ){
        ಪೂರ್ಣಾಂಕ i;
        ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;
        db_Eval_ಅಡ್ಡಸಾಲು_Info(&sEval, &n_ಲಂಬಸಾಲು, 0);
        ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
          Tcl_ListObjAppendElement(interp, pRet, db_Eval_ಸ್ತಂಭ_ಮೌಲ್ಯ(&sEval, i));
        }
      }
      dbEvalFinalize(&sEval);
      ಆದರೆ( rc==TCL_BREAK ){
        Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);
        rc = TCL_OK;
      }
      Tcl_Decr_Ref_ಎಣಿಕೆ(pRet);
    }ಇಲ್ಲದಿದ್ದರೆ{
      Client_ದತ್ತಾಂಶ cd2[2];
      DbEvalContext *p;
      Tcl_Obj *pVarName = 0;
      Tcl_Obj *pScript;

      ಆದರೆ( objc>=5 && *(ಅಕ್ಷರ *)Tcl_GetString(objv[3]) ){
        pVarName = objv[3];
      }
      pScript = objv[objc-1];
      Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);

      p = (DbEvalContext *)Tcl_Alloc(ಗಾತ್ರ(DbEvalContext));
      dbEvalInit(p, pDb, objv[2], pVarName, evalFlags);

      cd2[0] = (ಶೂನ್ಯ *)p;
      cd2[1] = (ಶೂನ್ಯ *)pScript;
      rc = Db_Eval_ಮುಂದಿನ_Cmd(cd2, interp, TCL_OK);
    }
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db format [OPTIONS] SQL
  **
  ** Run the SQL statement(s) given as the final argument.  Use the
  ** Query Result Formatter extension of SQLite to format the output as
  ** text and return that text.
  */
  ಸ್ಥಿತಿ DB_FORMAT: {
    rc = dbQrf(pDb, objc, objv);
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ DB_FUNCTION: {
    ಪೂರ್ಣಾಂಕ flags = SQLITE_UTF8;
    SqlFunc *pFunc;
    Tcl_Obj *pScript;
    ಅಕ್ಷರ *zName;
    ಪೂರ್ಣಾಂಕ nArg = -1;
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ eType = SQLITE_NULL;
    ಆದರೆ( objc<4 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "NAME ?SWITCHES? SCRIPT");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಗಾಗಿ(i=3; i<(objc-1); i++){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = Tcl_GetString(objv[i]);
      ಪೂರ್ಣಾಂಕ n = strlen30(z);
      ಆದರೆ( n>1 && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z, "-argcount",n)==0 ){
        ಆದರೆ( i==(objc-2) ){
          Tcl_Append_ಫಲಿತಾಂಶ(interp, "option requires an argument: ", z,(ಅಕ್ಷರ*)0);
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        ಆದರೆ( Tcl_GetIntFromObj(interp, objv[i+1], &nArg) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
        ಆದರೆ( nArg<0 ){
          Tcl_Append_ಫಲಿತಾಂಶ(interp, "number of arguments must be non-negative",
                           (ಅಕ್ಷರ*)0);
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        i++;
      }ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ( n>1 && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z, "-deterministic",n)==0 ){
        flags |= SQLITE_DETERMINISTIC;
      }ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ( n>1 && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z, "-directonly",n)==0 ){
        flags |= SQLITE_DIRECTONLY;
      }ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ( n>1 && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z, "-innocuous",n)==0 ){
        flags |= SQLITE_INNOCUOUS;
      }ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ( n>1 && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(z, "-returntype", n)==0 ){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azType[] = {"integer", "real", "text", "blob", "any", 0};
        ಪ್ರತಿಜ್ಞಾ( SQLITE_INTEGER==1 && SQLITE_FLOAT==2 && SQLITE_ಪಠ್ಯ==3 );
        ಪ್ರತಿಜ್ಞಾ( SQLITE_BLOB==4 && SQLITE_NULL==5 );
        ಆದರೆ( i==(objc-2) ){
          Tcl_Append_ಫಲಿತಾಂಶ(interp, "option requires an argument: ", z,(ಅಕ್ಷರ*)0);
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        i++;
        ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[i], azType, "type", 0, &eType) ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        eType++;
      }ಇಲ್ಲದಿದ್ದರೆ{
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "bad option \"", z,
            "\": must be -argcount, -deterministic, -directonly,"
            " -innocuous, or -returntype", (ಅಕ್ಷರ*)0
        );
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
    }

    pScript = objv[objc-1];
    zName = Tcl_GetStringFromObj(objv[2], 0);
    pFunc = ಕಂಡುಹಿಡಿ_Sql_Func(pDb, zName);
    ಆದರೆ( pFunc==0 ) ಹಿಂತಿರುಗಿ TCL_ERROR;
    ಆದರೆ( pFunc->pScript ){
      Tcl_Decr_Ref_ಎಣಿಕೆ(pFunc->pScript);
    }
    pFunc->pScript = pScript;
    Tcl_Incr_Ref_ಎಣಿಕೆ(pScript);
    pFunc->useEvalObjv = ಸುರಕ್ಷಿತ_ಗಮ್ಯ_Use_Eval_Objv(pScript);
    pFunc->eType = eType;
    rc = sqlite3_create_function(pDb->db, zName, nArg, flags,
        pFunc, tclSqlFunc, 0, 0);
    ಆದರೆ( rc!=SQLITE_OK ){
      rc = TCL_ERROR;
      Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3_errmsg(pDb->db), TCL_VOLATILE);
    }
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db incrblob ?-readonly? ?DB? TABLE COLUMN ROWID
  */
  ಸ್ಥಿತಿ DB_INCRBLOB: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_INCRBLOB
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "incrblob not available in this build", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಪೂರ್ಣಾಂಕ isReadonly = 0;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = "main";
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸ್ತಂಭ;
    Tcl_WideInt i_ಅಡ್ಡಸಾಲು;

    /* Check for the -readonly option */
    ಆದರೆ( objc>3 && strcmp(Tcl_GetString(objv[2]), "-readonly")==0 ){
      isReadonly = 1;
    }

    ಆದರೆ( objc!=(5+isReadonly) && objc!=(6+isReadonly) ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?-readonly? ?DB? TABLE COLUMN ROWID");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }

    ಆದರೆ( objc==(6+isReadonly) ){
      zDb = Tcl_GetString(objv[2+isReadonly]);
    }
    z_ಕೋಷ್ಟಕ = Tcl_GetString(objv[objc-3]);
    z_ಸ್ತಂಭ = Tcl_GetString(objv[objc-2]);
    rc = Tcl_GetWideIntFromObj(interp, objv[objc-1], &i_ಅಡ್ಡಸಾಲು);

    ಆದರೆ( rc==TCL_OK ){
      rc = createIncrblobChannel(
          interp, pDb, zDb, z_ಕೋಷ್ಟಕ, z_ಸ್ತಂಭ, (sqlite3_int64)i_ಅಡ್ಡಸಾಲು, isReadonly
      );
    }
#ಆದರೆ_ಅಂತ್ಯ
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db interrupt
  **
  ** Interrupt the execution of the inner-most SQL interpreter.  This
  ** causes the SQL statement to return an error of SQLITE_INTERRUPT.
  */
  ಸ್ಥಿತಿ DB_ಅಡಚಣೆ: {
    sqlite3_ಅಡಚಣೆ(pDb->db);
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db nullvalue ?STRING?
  **
  ** Change text used when a NULL comes back from the database. If ?STRING?
  ** is not present, then the current string used for NULL is returned.
  ** If STRING is present, then STRING is returned.
  **
  */
  ಸ್ಥಿತಿ DB_NULLVALUE: {
    ಆದರೆ( objc!=2 && objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "NULLVALUE");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( objc==3 ){
      Tcl_ಗಾತ್ರ len;
      ಅಕ್ಷರ *zNull = Tcl_GetStringFromObj(objv[2], &len);
      ಆದರೆ( pDb->zNull ){
        Tcl_Free(pDb->zNull);
      }
      ಆದರೆ( zNull && len>0 ){
        pDb->zNull = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->zNull, zNull, len);
        pDb->zNull[len] = '\0';
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->zNull = 0;
      }
    }
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(pDb->zNull, -1));
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db last_insert_rowid
  **
  ** Return an integer which is the ROWID for the most recent insert.
  */
  ಸ್ಥಿತಿ DB_ಕೊನೆಯ_ಸೇರಿಸು_ROWID: {
    Tcl_Obj *p_ಫಲಿತಾಂಶ;
    Tcl_WideInt rowid;
    ಆದರೆ( objc!=2 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    rowid = sqlite3_ಕೊನೆಯ_ಸೇರಿಸು_rowid(pDb->db);
    p_ಫಲಿತಾಂಶ = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
    Tcl_ಗಣ_Wide_Int_Obj(p_ಫಲಿತಾಂಶ, rowid);
    ನಿಲ್ಲಿಸು;
  }

  /*
  ** The DB_ONECOLUMN method is implemented together with DB_EXISTS.
  */

  /*    $db progress ?N CALLBACK?
  **
  ** Invoke the given callback every N virtual machine opcodes while executing
  ** queries.
  */
  ಸ್ಥಿತಿ DB_ಪ್ರಗತಿ: {
    ಆದರೆ( objc==2 ){
      ಆದರೆ( pDb->z_ಪ್ರಗತಿ ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, pDb->z_ಪ್ರಗತಿ, (ಅಕ್ಷರ*)0);
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
      sqlite3_ಪ್ರಗತಿ_handler(pDb->db, 0, 0, 0);
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==4 ){
      ಅಕ್ಷರ *z_ಪ್ರಗತಿ;
      Tcl_ಗಾತ್ರ len;
      ಪೂರ್ಣಾಂಕ N;
      ಆದರೆ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &N) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      };
      ಆದರೆ( pDb->z_ಪ್ರಗತಿ ){
        Tcl_Free(pDb->z_ಪ್ರಗತಿ);
      }
      z_ಪ್ರಗತಿ = Tcl_GetStringFromObj(objv[3], &len);
      ಆದರೆ( z_ಪ್ರಗತಿ && len>0 ){
        pDb->z_ಪ್ರಗತಿ = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->z_ಪ್ರಗತಿ, z_ಪ್ರಗತಿ, len+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->z_ಪ್ರಗತಿ = 0;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
      ಆದರೆ( pDb->z_ಪ್ರಗತಿ ){
        pDb->interp = interp;
        sqlite3_ಪ್ರಗತಿ_handler(pDb->db, N, Db_ಪ್ರಗತಿ_Handler, pDb);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಪ್ರಗತಿ_handler(pDb->db, 0, 0, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "N CALLBACK");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ನಿಲ್ಲಿಸು;
  }

  /*    $db profile ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine after each SQL statement
  ** that has run.  The text of the SQL and the amount of elapse time are
  ** appended to CALLBACK before the script is run.
  */
  ಸ್ಥಿತಿ DB_PROFILE: {
    ಆದರೆ( objc>3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?CALLBACK?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==2 ){
      ಆದರೆ( pDb->zProfile ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, pDb->zProfile, (ಅಕ್ಷರ*)0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ *zProfile;
      Tcl_ಗಾತ್ರ len;
      ಆದರೆ( pDb->zProfile ){
        Tcl_Free(pDb->zProfile);
      }
      zProfile = Tcl_GetStringFromObj(objv[2], &len);
      ಆದರೆ( zProfile && len>0 ){
        pDb->zProfile = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->zProfile, zProfile, len+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->zProfile = 0;
      }
#ಆದರೆ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ಬಿಂದು) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ಆದರೆ( pDb->zProfile ){
        pDb->interp = interp;
        sqlite3_profile(pDb->db, DbProfileHandler, pDb);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_profile(pDb->db, 0, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db rekey KEY
  **
  ** Change the encryption key on the currently open database.
  */
  ಸ್ಥಿತಿ DB_REKEY: {
    ಆದರೆ( objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "KEY");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ನಿಲ್ಲಿಸು;
  }

  /*    $db restore ?DATABASE? FILENAME
  **
  ** Open a database file named FILENAME.  Transfer the content
  ** of FILENAME into the local database DATABASE (default: "main").
  */
  ಸ್ಥಿತಿ DB_RESTORE: {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSrcFile;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDestDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    ಪೂರ್ಣಾಂಕ nTimeout = 0;

    ಆದರೆ( objc==3 ){
      zDestDb = "main";
      zSrcFile = Tcl_GetString(objv[2]);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==4 ){
      zDestDb = Tcl_GetString(objv[2]);
      zSrcFile = Tcl_GetString(objv[3]);
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?DATABASE? FILENAME");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    rc = sqlite3_open_v2(zSrcFile, &pSrc,
                         SQLITE_OPEN_READONLY | pDb->openFlags, 0);
    ಆದರೆ( rc!=SQLITE_OK ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "cannot open source database: ",
           sqlite3_errmsg(pSrc), (ಅಕ್ಷರ*)0);
      sqlite3_close(pSrc);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    pBackup = sqlite3_backup_init(pDb->db, zDestDb, pSrc, "main");
    ಆದರೆ( pBackup==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
      sqlite3_close(pSrc);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆಗ( (rc = sqlite3_backup_ಹೆಜ್ಜೆ(pBackup,100))==SQLITE_OK
              || rc==SQLITE_BUSY ){
      ಆದರೆ( rc==SQLITE_BUSY ){
        ಆದರೆ( nTimeout++ >= 3 ) ನಿಲ್ಲಿಸು;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_ಮುಕ್ತಾಯ(pBackup);
    ಆದರೆ( rc==SQLITE_ಮುಗಿದ ){
      rc = TCL_OK;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "restore failed: source database busy",
                       (ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "restore failed: ",
           sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
      rc = TCL_ERROR;
    }
    sqlite3_close(pSrc);
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db serialize ?DATABASE?
  **
  ** Return a serialization of a database.  
  */
  ಸ್ಥಿತಿ DB_SERIALIZE: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_DESERIALIZE
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "MEMDB not available in this build",
                     (ಅಕ್ಷರ*)0);
    rc = TCL_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema = objc>=3 ? Tcl_GetString(objv[2]) : "main";
    sqlite3_int64 sz = 0;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p_ದತ್ತಾಂಶ;
    ಆದರೆ( objc!=2 && objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?DATABASE?");
      rc = TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ needFree;
      p_ದತ್ತಾಂಶ = sqlite3_serialize(pDb->db, zSchema, &sz, SQLITE_SERIALIZE_NOCOPY);
      ಆದರೆ( p_ದತ್ತಾಂಶ ){
        needFree = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        p_ದತ್ತಾಂಶ = sqlite3_serialize(pDb->db, zSchema, &sz, 0);
        needFree = 1;
      }
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewByteArrayObj(p_ದತ್ತಾಂಶ,sz));
      ಆದರೆ( needFree ) sqlite3_free(p_ದತ್ತಾಂಶ);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db status (step|sort|autoindex|vmstep)
  **
  ** Display SQLITE_STMTSTATUS_FULLSCAN_STEP or
  ** SQLITE_STMTSTATUS_SORT for the most recent eval.
  */
  ಸ್ಥಿತಿ DB_ಸ್ಥಿತಿ: {
    ಪೂರ್ಣಾಂಕ v;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOp;
    ಆದರೆ( objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "(step|sort|autoindex)");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    zOp = Tcl_GetString(objv[2]);
    ಆದರೆ( strcmp(zOp, "step")==0 ){
      v = pDb->n_ಹೆಜ್ಜೆ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zOp, "sort")==0 ){
      v = pDb->n_ವಿಂಗಡಿಸು;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zOp, "autoindex")==0 ){
      v = pDb->n_ಸೂಚ್ಯಂಕ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zOp, "vmstep")==0 ){
      v = pDb->nVMStep;
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Append_ಫಲಿತಾಂಶ(interp,
            "bad argument: should be autoindex, step, sort or vmstep",
            (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(v));
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db timeout MILLESECONDS
  **
  ** Delay for the number of milliseconds specified when a file is locked.
  */
  ಸ್ಥಿತಿ DB_TIMEOUT: {
    ಪೂರ್ಣಾಂಕ ms;
    ಆದರೆ( objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "MILLISECONDS");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &ms) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
    sqlite3_busy_timeout(pDb->db, ms);
    ನಿಲ್ಲಿಸು;
  }

  /*
  **     $db total_changes
  **
  ** Return the number of rows that were modified, inserted, or deleted
  ** since the database handle was created.
  */
  ಸ್ಥಿತಿ DB_ಒಟ್ಟು_CHANGES: {
    Tcl_Obj *p_ಫಲಿತಾಂಶ;
    ಆದರೆ( objc!=2 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    p_ಫಲಿತಾಂಶ = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
    Tcl_ಗಣ_Wide_Int_Obj(p_ಫಲಿತಾಂಶ, sqlite3_ಒಟ್ಟು_changes64(pDb->db));
    ನಿಲ್ಲಿಸು;
  }

  /*    $db trace ?CALLBACK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each SQL statement
  ** that is executed.  The text of the SQL is appended to CALLBACK before
  ** it is executed.
  */
  ಸ್ಥಿತಿ DB_TRACE: {
    ಆದರೆ( objc>3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?CALLBACK?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==2 ){
      ಆದರೆ( pDb->zTrace ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, pDb->zTrace, (ಅಕ್ಷರ*)0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ *zTrace;
      Tcl_ಗಾತ್ರ len;
      ಆದರೆ( pDb->zTrace ){
        Tcl_Free(pDb->zTrace);
      }
      zTrace = Tcl_GetStringFromObj(objv[2], &len);
      ಆದರೆ( zTrace && len>0 ){
        pDb->zTrace = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->zTrace, zTrace, len+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->zTrace = 0;
      }
#ಆದರೆ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ಬಿಂದು) && \
    !defined(SQLITE_OMIT_DEPRECATED)
      ಆದರೆ( pDb->zTrace ){
        pDb->interp = interp;
        sqlite3_trace(pDb->db, DbTraceHandler, pDb);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_trace(pDb->db, 0, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
    ನಿಲ್ಲಿಸು;
  }

  /*    $db trace_v2 ?CALLBACK? ?MASK?
  **
  ** Make arrangements to invoke the CALLBACK routine for each trace event
  ** matching the mask that is generated.  The parameters are appended to
  ** CALLBACK before it is executed.
  */
  ಸ್ಥಿತಿ DB_TRACE_V2: {
    ಆದರೆ( objc>4 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?CALLBACK? ?MASK?");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc==2 ){
      ಆದರೆ( pDb->zTraceV2 ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, pDb->zTraceV2, (ಅಕ್ಷರ*)0);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ *zTraceV2;
      Tcl_ಗಾತ್ರ len;
      Tcl_WideInt w_ಮುಖವಾಡ = 0;
      ಆದರೆ( objc==4 ){
        ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *TTYPE_strs[] = {
          "statement", "profile", "row", "close", 0
        };
        ಎಣಿಕೆ TTYPE_enum {
          TTYPE_STMT, TTYPE_PROFILE, TTYPE_ಅಡ್ಡಸಾಲು, TTYPE_CLOSE
        };
        Tcl_ಗಾತ್ರ i;
        ಆದರೆ( TCL_OK!=Tcl_List_Obj_ಉದ್ದಳತೆ(interp, objv[3], &len) ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        ಗಾಗಿ(i=0; i<len; i++){
          Tcl_Obj *pObj;
          ಪೂರ್ಣಾಂಕ ttype;
          ಆದರೆ( TCL_OK!=Tcl_List_Obj_ಸೂಚ್ಯಂಕ(interp, objv[3], i, &pObj) ){
            ಹಿಂತಿರುಗಿ TCL_ERROR;
          }
          ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, pObj, TTYPE_strs, "trace type",
                                  0, &ttype)!=TCL_OK ){
            Tcl_WideInt wType;
            Tcl_Obj *pError = Tcl_DuplicateObj(Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp));
            Tcl_Incr_Ref_ಎಣಿಕೆ(pError);
            ಆದರೆ( TCL_OK==Tcl_GetWideIntFromObj(interp, pObj, &wType) ){
              Tcl_Decr_Ref_ಎಣಿಕೆ(pError);
              w_ಮುಖವಾಡ |= wType;
            }ಇಲ್ಲದಿದ್ದರೆ{
              Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pError);
              Tcl_Decr_Ref_ಎಣಿಕೆ(pError);
              ಹಿಂತಿರುಗಿ TCL_ERROR;
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಆಯ್ಕೆ( (ಎಣಿಕೆ TTYPE_enum)ttype ){
              ಸ್ಥಿತಿ TTYPE_STMT:    w_ಮುಖವಾಡ |= SQLITE_TRACE_STMT;    ನಿಲ್ಲಿಸು;
              ಸ್ಥಿತಿ TTYPE_PROFILE: w_ಮುಖವಾಡ |= SQLITE_TRACE_PROFILE; ನಿಲ್ಲಿಸು;
              ಸ್ಥಿತಿ TTYPE_ಅಡ್ಡಸಾಲು:     w_ಮುಖವಾಡ |= SQLITE_TRACE_ಅಡ್ಡಸಾಲು;     ನಿಲ್ಲಿಸು;
              ಸ್ಥಿತಿ TTYPE_CLOSE:   w_ಮುಖವಾಡ |= SQLITE_TRACE_CLOSE;   ನಿಲ್ಲಿಸು;
            }
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        w_ಮುಖವಾಡ = SQLITE_TRACE_STMT; /* use the "legacy" default */
      }
      ಆದರೆ( pDb->zTraceV2 ){
        Tcl_Free(pDb->zTraceV2);
      }
      zTraceV2 = Tcl_GetStringFromObj(objv[2], &len);
      ಆದರೆ( zTraceV2 && len>0 ){
        pDb->zTraceV2 = Tcl_Alloc( len + 1 );
        ಸ್ಮರಣೆ_ನಕಲು(pDb->zTraceV2, zTraceV2, len+1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pDb->zTraceV2 = 0;
      }
#ಆದರೆ !defined(SQLITE_OMIT_TRACE) && !defined(SQLITE_OMIT_FLOATING_ಬಿಂದು)
      ಆದರೆ( pDb->zTraceV2 ){
        pDb->interp = interp;
        sqlite3_trace_v2(pDb->db, (ಚಿಹ್ನೆರಹಿತ)w_ಮುಖವಾಡ, DbTraceV2Handler, pDb);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_trace_v2(pDb->db, 0, 0, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
    ನಿಲ್ಲಿಸು;
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
  ಸ್ಥಿತಿ DB_TRANSACTION: {
    Tcl_Obj *pScript;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zBegin = "SAVEPOINT _tcl_transaction";
    ಆದರೆ( objc!=3 && objc!=4 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "[TYPE] SCRIPT");
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }

    ಆದರೆ( pDb->nTransaction==0 && objc==4 ){
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *TTYPE_strs[] = {
        "deferred",   "exclusive",  "immediate", 0
      };
      ಎಣಿಕೆ TTYPE_enum {
        TTYPE_DEFERRED, TTYPE_EXCLUSIVE, TTYPE_ತಕ್ಷಣ
      };
      ಪೂರ್ಣಾಂಕ ttype;
      ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[2], TTYPE_strs, "transaction type",
                              0, &ttype) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆಯ್ಕೆ( (ಎಣಿಕೆ TTYPE_enum)ttype ){
        ಸ್ಥಿತಿ TTYPE_DEFERRED:    /* no-op */;                 ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ TTYPE_EXCLUSIVE:   zBegin = "BEGIN EXCLUSIVE";  ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ TTYPE_ತಕ್ಷಣ:   zBegin = "BEGIN IMMEDIATE";  ನಿಲ್ಲಿಸು;
      }
    }
    pScript = objv[objc-1];

    /* Run the SQLite BEGIN command to open a transaction or savepoint. */
    pDb->disableAuth++;
    rc = sqlite3_exec(pDb->db, zBegin, 0, 0, 0);
    pDb->disableAuth--;
    ಆದರೆ( rc!=SQLITE_OK ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    pDb->nTransaction++;

    /* If using NRE, schedule a callback to invoke the script pScript, then
    ** a second callback to commit (or rollback) the transaction or savepoint
    ** opened above. If not using NRE, evaluate the script directly, then
    ** call function DbTransPostCmd() to commit (or rollback) the transaction
    ** or savepoint.  */
    ಕೂಡಿಸು_Database_Ref(pDb);          /* DbTransPostCmd() calls delDatabaseRef() */
    ಆದರೆ( DbUseNre() ){
      Tcl_NRAddCallback(interp, DbTransPostCmd, cd, 0, 0, 0);
      (ಶೂನ್ಯ)Tcl_NREvalObj(interp, pScript, 0);
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = DbTransPostCmd(&cd, interp, Tcl_EvalObjEx(interp, pScript, 0));
    }
    ನಿಲ್ಲಿಸು;
  }

  /*
  **    $db unlock_notify ?script?
  */
  ಸ್ಥಿತಿ DB_UNLOCK_NOTIFY: {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_UNLOCK_NOTIFY
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "unlock_notify not available in this build",
                     (ಅಕ್ಷರ*)0);
    rc = TCL_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( objc!=2 && objc!=3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?SCRIPT?");
      rc = TCL_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಶೂನ್ಯ (*xNotify)(ಶೂನ್ಯ **, ಪೂರ್ಣಾಂಕ) = 0;
      ಶೂನ್ಯ *pNotifyArg = 0;

      ಆದರೆ( pDb->pUnlockNotify ){
        Tcl_Decr_Ref_ಎಣಿಕೆ(pDb->pUnlockNotify);
        pDb->pUnlockNotify = 0;
      }

      ಆದರೆ( objc==3 ){
        xNotify = DbUnlockNotify;
        pNotifyArg = (ಶೂನ್ಯ *)pDb;
        pDb->pUnlockNotify = objv[2];
        Tcl_Incr_Ref_ಎಣಿಕೆ(pDb->pUnlockNotify);
      }

      ಆದರೆ( sqlite3_unlock_notify(pDb->db, xNotify, pNotifyArg) ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
        rc = TCL_ERROR;
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
    ನಿಲ್ಲಿಸು;
  }

  /*
  **    $db preupdate_hook count
  **    $db preupdate_hook hook ?SCRIPT?
  **    $db preupdate_hook new INDEX
  **    $db preupdate_hook old INDEX
  */
  ಸ್ಥಿತಿ DB_PREUPDATE: {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "preupdate_hook was omitted at compile-time", 
                     (ಅಕ್ಷರ*)0);
    rc = TCL_ERROR;
#ಇಲ್ಲದಿದ್ದರೆ
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azSub[] = {"count", "depth", "hook", "new", "old", 0};
    ಎಣಿಕೆ DbPreupdateSubCmd {
      PRE_ಎಣಿಕೆ, PRE_ಆಳ, PRE_HOOK, PRE_NEW, PRE_OLD
    };
    ಪೂರ್ಣಾಂಕ iSub;

    ಆದರೆ( objc<3 ){
      Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "SUB-COMMAND ?ARGS?");
    }
    ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[2], azSub, "sub-command", 0, &iSub) ){
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }

    ಆಯ್ಕೆ( (ಎಣಿಕೆ DbPreupdateSubCmd)iSub ){
      ಸ್ಥಿತಿ PRE_ಎಣಿಕೆ: {
        ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = sqlite3_preupdate_ಎಣಿಕೆ(pDb->db);
        Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(n_ಲಂಬಸಾಲು));
        ನಿಲ್ಲಿಸು;
      }

      ಸ್ಥಿತಿ PRE_HOOK: {
        ಆದರೆ( objc>4 ){
          Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "hook ?SCRIPT?");
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        DbHookCmd(interp, pDb, (objc==4 ? objv[3] : 0), &pDb->pPreUpdateHook);
        ನಿಲ್ಲಿಸು;
      }

      ಸ್ಥಿತಿ PRE_ಆಳ: {
        Tcl_Obj *pRet;
        ಆದರೆ( objc!=3 ){
          Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 3, objv, "");
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        pRet = Tcl_NewIntObj(sqlite3_preupdate_ಆಳ(pDb->db));
        Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);
        ನಿಲ್ಲಿಸು;
      }

      ಸ್ಥಿತಿ PRE_NEW:
      ಸ್ಥಿತಿ PRE_OLD: {
        ಪೂರ್ಣಾಂಕ iIdx;
        sqlite3_ಮೌಲ್ಯ *p_ಮೌಲ್ಯ;
        ಆದರೆ( objc!=4 ){
          Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 3, objv, "INDEX");
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        ಆದರೆ( Tcl_GetIntFromObj(interp, objv[3], &iIdx) ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }

        ಆದರೆ( iSub==PRE_OLD ){
          rc = sqlite3_preupdate_old(pDb->db, iIdx, &p_ಮೌಲ್ಯ);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪ್ರತಿಜ್ಞಾ( iSub==PRE_NEW );
          rc = sqlite3_preupdate_new(pDb->db, iIdx, &p_ಮೌಲ್ಯ);
        }

        ಆದರೆ( rc==SQLITE_OK ){
          Tcl_Obj *pObj;
          pObj = Tcl_NewStringObj((ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(p_ಮೌಲ್ಯ), -1);
          Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pObj);
        }ಇಲ್ಲದಿದ್ದರೆ{
          Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3_errmsg(pDb->db), (ಅಕ್ಷರ*)0);
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
      }
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */
    ನಿಲ್ಲಿಸು;
  }

  /*
  **    $db wal_hook ?script?
  **    $db update_hook ?script?
  **    $db rollback_hook ?script?
  */
  ಸ್ಥಿತಿ DB_WAL_HOOK:
  ಸ್ಥಿತಿ DB_UPDATE_HOOK:
  ಸ್ಥಿತಿ DB_ROLLBACK_HOOK: {
    /* set ppHook to point at pUpdateHook or pRollbackHook, depending on
    ** whether [$db update_hook] or [$db rollback_hook] was invoked.
    */
    Tcl_Obj **ppHook = 0;
    ಆದರೆ( choice==DB_WAL_HOOK ) ppHook = &pDb->pWalHook;
    ಆದರೆ( choice==DB_UPDATE_HOOK ) ppHook = &pDb->pUpdateHook;
    ಆದರೆ( choice==DB_ROLLBACK_HOOK ) ppHook = &pDb->pRollbackHook;
    ಆದರೆ( objc>3 ){
       Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?SCRIPT?");
       ಹಿಂತಿರುಗಿ TCL_ERROR;
    }

    DbHookCmd(interp, pDb, (objc==3 ? objv[2] : 0), ppHook);
    ನಿಲ್ಲಿಸು;
  }

  /*    $db version
  **
  ** Return the version string for this database.
  */
  ಸ್ಥಿತಿ DB_ಆವೃತ್ತಿ: {
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=2; i<objc; i++){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg = Tcl_GetString(objv[i]);
      /* Optional arguments to $db version are used for testing purpose */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
      /* $db version -use-legacy-prepare BOOLEAN
      **
      ** Turn the use of legacy sqlite3_prepare() on or off.
      */
      ಆದರೆ( strcmp(zArg, "-use-legacy-prepare")==0 && i+1<objc ){
        i++;
        ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i], &pDb->bLegacyPrepare) ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
      }ಇಲ್ಲದಿದ್ದರೆ

      /* $db version -last-stmt-ptr
      **
      ** Return a string which is a hex encoding of the pointer to the
      ** most recent sqlite3_stmt in the statement cache.
      */
      ಆದರೆ( strcmp(zArg, "-last-stmt-ptr")==0 ){
        ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(ಶೂನ್ಯ*);
        ಶೂನ್ಯ *ptr = (ಶೂನ್ಯ*)(pDb->stmtList ? pDb->stmtList->pStmt: 0);
        Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ*)sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(ptr), TCL_VOLATILE);
      }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_TEST */
      {
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "unknown argument: ", zArg, (ಅಕ್ಷರ*)0);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
    }
    ಆದರೆ( i==2 ){   
      Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3_libversion(), TCL_STATIC);
    }
    ನಿಲ್ಲಿಸು;
  }


  } /* End of the SWITCH statement */
  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ SQLITE_TCL_NRE
/*
** Adaptor that provides an objCmd interface to the NRE-enabled
** interface implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI DbObjCmdAdaptor(
  ಶೂನ್ಯ *cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *ಸ್ಥಿರಾಂಕ*objv
){
  ಹಿಂತಿರುಗಿ Tcl_NRCallObjProc(interp, DbObjCmd, cd, objc, objv);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_TCL_NRE */

/*
** Issue the usage message when the "sqlite3" command arguments are
** incorrect.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqliteCmdUsage(
  Tcl_Interp *interp,
  Tcl_Obj *ಸ್ಥಿರಾಂಕ*objv
){
  Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv,
    "HANDLE ?FILENAME? ?-vfs VFSNAME? ?-readonly BOOLEAN? ?-create BOOLEAN?"
    " ?-nofollow BOOLEAN?"
    " ?-nomutex BOOLEAN? ?-fullmutex BOOLEAN? ?-uri BOOLEAN?"
  );
  ಹಿಂತಿರುಗಿ TCL_ERROR;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI DbMain(
  ಶೂನ್ಯ *cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *ಸ್ಥಿರಾಂಕ*objv
){
  SqliteDb *p;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg;
  ಅಕ್ಷರ *z_Err_ಸಂದೇಶ;
  ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs = 0;
  ಪೂರ್ಣಾಂಕ flags;
  ಪೂರ್ಣಾಂಕ bTranslateFileName = 1;
  Tcl_DString translatedFilename;
  ಪೂರ್ಣಾಂಕ rc;

  /* In normal use, each TCL interpreter runs in a single thread.  So
  ** by default, we can turn off mutexing on SQLite database connections.
  ** However, for testing purposes it is useful to have mutexes turned
  ** on.  So, by default, mutexes default off.  But if compiled with
  ** SQLITE_TCL_DEFAULT_FULLMUTEX then mutexes default on.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TCL_DEFAULT_FULLMUTEX
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
#ಇಲ್ಲದಿದ್ದರೆ
  flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( objc==1 ) ಹಿಂತಿರುಗಿ sqliteCmdUsage(interp, objv);
  ಆದರೆ( objc==2 ){
    zArg = Tcl_GetStringFromObj(objv[1], 0);
    ಆದರೆ( strcmp(zArg,"-version")==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp,sqlite3_libversion(), (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_OK;
    }
    ಆದರೆ( strcmp(zArg,"-sourceid")==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp,sqlite3_sourceid(), (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_OK;
    }
    ಆದರೆ( strcmp(zArg,"-has-codec")==0 ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp,"0",(ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_OK;
    }
    ಆದರೆ( zArg[0]=='-' ) ಹಿಂತಿರುಗಿ sqliteCmdUsage(interp, objv);
  }
  ಗಾಗಿ(i=2; i<objc; i++){
    zArg = Tcl_GetString(objv[i]);
    ಆದರೆ( zArg[0]!='-' ){
      ಆದರೆ( zFile!=0 ) ಹಿಂತಿರುಗಿ sqliteCmdUsage(interp, objv);
      zFile = zArg;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( i==objc-1 ) ಹಿಂತಿರುಗಿ sqliteCmdUsage(interp, objv);
    i++;
    ಆದರೆ( strcmp(zArg,"-key")==0 ){
      /* no-op */
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg, "-vfs")==0 ){
      zVfs = Tcl_GetString(objv[i]);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg, "-readonly")==0 ){
      ಪೂರ್ಣಾಂಕ b;
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
      ಆದರೆ( b ){
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        flags |= SQLITE_OPEN_READONLY;
      }ಇಲ್ಲದಿದ್ದರೆ{
        flags &= ~SQLITE_OPEN_READONLY;
        flags |= SQLITE_OPEN_READWRITE;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg, "-create")==0 ){
      ಪೂರ್ಣಾಂಕ b;
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
      ಆದರೆ( b && (flags & SQLITE_OPEN_READONLY)==0 ){
        flags |= SQLITE_OPEN_CREATE;
      }ಇಲ್ಲದಿದ್ದರೆ{
        flags &= ~SQLITE_OPEN_CREATE;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg, "-nofollow")==0 ){
      ಪೂರ್ಣಾಂಕ b;
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
      ಆದರೆ( b ){
        flags |= SQLITE_OPEN_NOFOLLOW;
      }ಇಲ್ಲದಿದ್ದರೆ{
        flags &= ~SQLITE_OPEN_NOFOLLOW;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg, "-nomutex")==0 ){
      ಪೂರ್ಣಾಂಕ b;
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
      ಆದರೆ( b ){
        flags |= SQLITE_OPEN_NOMUTEX;
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }ಇಲ್ಲದಿದ್ದರೆ{
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg, "-fullmutex")==0 ){
      ಪೂರ್ಣಾಂಕ b;
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
      ಆದರೆ( b ){
        flags |= SQLITE_OPEN_FULLMUTEX;
        flags &= ~SQLITE_OPEN_NOMUTEX;
      }ಇಲ್ಲದಿದ್ದರೆ{
        flags &= ~SQLITE_OPEN_FULLMUTEX;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg, "-uri")==0 ){
      ಪೂರ್ಣಾಂಕ b;
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i], &b) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
      ಆದರೆ( b ){
        flags |= SQLITE_OPEN_URI;
      }ಇಲ್ಲದಿದ್ದರೆ{
        flags &= ~SQLITE_OPEN_URI;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( strcmp(zArg, "-translatefilename")==0 ){
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i], &bTranslateFileName) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "unknown option: ", zArg, (ಅಕ್ಷರ*)0);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
  }
  z_Err_ಸಂದೇಶ = 0;
  p = (SqliteDb*)Tcl_Alloc( ಗಾತ್ರ(*p) );
  memset(p, 0, ಗಾತ್ರ(*p));
  ಆದರೆ( zFile==0 ) zFile = "";
  ಆದರೆ( bTranslateFileName ){
    zFile = Tcl_TranslateFileName(interp, zFile, &translatedFilename);
  }
  rc = sqlite3_open_v2(zFile, &p->db, flags, zVfs);
  ಆದರೆ( bTranslateFileName ){
    Tcl_DStringFree(&translatedFilename);
  }
  ಆದರೆ( p->db ){
    ಆದರೆ( SQLITE_OK!=sqlite3_errcode(p->db) ){
      z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", sqlite3_errmsg(p->db));
      sqlite3_close(p->db);
      p->db = 0;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", sqlite3_errstr(rc));
  }
  ಆದರೆ( p->db==0 ){
    Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, z_Err_ಸಂದೇಶ, TCL_VOLATILE);
    Tcl_Free((ಅಕ್ಷರ*)p);
    sqlite3_free(z_Err_ಸಂದೇಶ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  p->maxStmt = ಅಂಕೆ_PREPARED_STMTS;
  p->openFlags = flags & SQLITE_OPEN_URI;
  p->interp = interp;
  zArg = Tcl_GetStringFromObj(objv[1], 0);
  ಆದರೆ( DbUseNre() ){
    Tcl_NRCreateCommand(interp, zArg, DbObjCmdAdaptor, DbObjCmd,
                        (ಅಕ್ಷರ*)p, DbDeleteCmd);
  }ಇಲ್ಲದಿದ್ದರೆ{
    Tcl_CreateObjCommand(interp, zArg, DbObjCmd, (ಅಕ್ಷರ*)p, DbDeleteCmd);
  }
  p->nRef = 1;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Provide a dummy Tcl_InitStubs if we are using this as a static
** library.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ USE_TCL_STUBS
# undef  Tcl_InitStubs
# define Tcl_InitStubs(a,b,c) TCL_ಆವೃತ್ತಿ
#ಆದರೆ_ಅಂತ್ಯ

/*
** Make sure we have a PACKAGE_VERSION macro defined.  This will be
** defined automatically by the TEA makefile.  But other makefiles
** do not define it.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ PACKAGE_ಆವೃತ್ತಿ
# define PACKAGE_ಆವೃತ್ತಿ SQLITE_ಆವೃತ್ತಿ
#ಆದರೆ_ಅಂತ್ಯ

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
EXTERN ಪೂರ್ಣಾಂಕ Sqlite3_Init(Tcl_Interp *interp){
  ಪೂರ್ಣಾಂಕ rc = Tcl_InitStubs(interp, "8.5-", 0) ? TCL_OK : TCL_ERROR;
  ಆದರೆ( rc==TCL_OK ){
    Tcl_CreateObjCommand(interp, "sqlite3", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_3_SUFFIX_ONLY
    /* The "sqlite" alias is undocumented.  It is here only to support
    ** legacy scripts.  All new scripts should use only the "sqlite3"
    ** command. */
    Tcl_CreateObjCommand(interp, "sqlite", (Tcl_ObjCmdProc*)DbMain, 0, 0);
#ಆದರೆ_ಅಂತ್ಯ
    rc = Tcl_PkgProvide(interp, "sqlite3", PACKAGE_ಆವೃತ್ತಿ);
  }
  ಹಿಂತಿರುಗಿ rc;
}
EXTERN ಪೂರ್ಣಾಂಕ Tclsqlite3_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ Sqlite3_Init(interp); }
EXTERN ಪೂರ್ಣಾಂಕ Sqlite3_Unload(Tcl_Interp *interp, ಪೂರ್ಣಾಂಕ flags){ ಹಿಂತಿರುಗಿ TCL_OK; }
EXTERN ಪೂರ್ಣಾಂಕ Tclsqlite3_Unload(Tcl_Interp *interp, ಪೂರ್ಣಾಂಕ flags){ ಹಿಂತಿರುಗಿ TCL_OK; }

/* Because it accesses the file-system and uses persistent state, SQLite
** is not considered appropriate for safe interpreters.  Hence, we cause
** the _SafeInit() interfaces return TCL_ERROR.
*/
EXTERN ಪೂರ್ಣಾಂಕ Sqlite3_ಸುರಕ್ಷಿತ_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ TCL_ERROR; }
EXTERN ಪೂರ್ಣಾಂಕ Sqlite3_ಸುರಕ್ಷಿತ_Unload(Tcl_Interp *interp, ಪೂರ್ಣಾಂಕ flags){ಹಿಂತಿರುಗಿ TCL_ERROR;}

/*
** Versions of all of the above entry points that omit the "3" at the end
** of the name.  Years ago (circa 2004) the "3" was necessary to distinguish
** SQLite version 3 from Sqlite version 2.  But two decades have elapsed.
** SQLite2 is not longer a conflict.  So it is ok to omit the "3".
**
** Omitting the "3" helps TCL find the entry point.
*/
EXTERN ಪೂರ್ಣಾಂಕ Sqlite_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ Sqlite3_Init(interp);}
EXTERN ಪೂರ್ಣಾಂಕ Tclsqlite_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ Sqlite3_Init(interp); }
EXTERN ಪೂರ್ಣಾಂಕ Sqlite_Unload(Tcl_Interp *interp, ಪೂರ್ಣಾಂಕ flags){ ಹಿಂತಿರುಗಿ TCL_OK; }
EXTERN ಪೂರ್ಣಾಂಕ Tclsqlite_Unload(Tcl_Interp *interp, ಪೂರ್ಣಾಂಕ flags){ ಹಿಂತಿರುಗಿ TCL_OK; }
EXTERN ಪೂರ್ಣಾಂಕ Sqlite_ಸುರಕ್ಷಿತ_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ TCL_ERROR; }
EXTERN ಪೂರ್ಣಾಂಕ Sqlite_ಸುರಕ್ಷಿತ_Unload(Tcl_Interp *interp, ಪೂರ್ಣಾಂಕ flags){ಹಿಂತಿರುಗಿ TCL_ERROR;}

/* Also variants with a lowercase "s".  I'm told that these are
** deprecated in Tcl9, but they continue to be included for backwards
** compatibility. */
EXTERN ಪೂರ್ಣಾಂಕ sqlite3_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ Sqlite3_Init(interp);}
EXTERN ಪೂರ್ಣಾಂಕ sqlite_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ Sqlite3_Init(interp);}


/*
** If the TCLSH macro is defined, add code to make a stand-alone program.
*/
#ಆದರೆ defined(TCLSH)

/* This is the main routine for an ordinary TCL shell.  If there are
** arguments, run the first argument as a script.  Otherwise, read TCL
** commands from standard input
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tclsh_main_loop(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ zMainloop[] =
    "if {[llength $argv]>=1} {\n"
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WIN32
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
#ಆದರೆ_ಅಂತ್ಯ
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
  ಹಿಂತಿರುಗಿ zMainloop;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ TCLSH_MAIN
# define TCLSH_MAIN ಮುಖ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ SQLITE_CDECL TCLSH_MAIN(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv){
  Tcl_Interp *interp;
  ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zScript = 0;
  ಅಕ್ಷರ zArgc[32];
#ಆದರೆ defined(TCLSH_INIT_PROC)
  ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *TCLSH_INIT_PROC(Tcl_Interp*);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(_WIN32_WCE)
  ಆದರೆ( getenv("SQLITE_DEBUG_BREAK") ){
    ಆದರೆ( isatty(0) && isatty(2) ){
      fprintf(stderr,
          "attach debugger to process %d and press any key to continue.\n",
          GETPID());
      fgetc(stdin);
    }ಇಲ್ಲದಿದ್ದರೆ{
#ಆದರೆ defined(_WIN32) || defined(WIN32)
      DebugBreak();
#ಅಥವಾಆದರೆ defined(SIGTRAP)
      ಎತ್ತು(SIGTRAP);
#ಆದರೆ_ಅಂತ್ಯ
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Call sqlite3_shutdown() once before doing anything else. This is to
  ** test that sqlite3_shutdown() can be safely called by a process before
  ** sqlite3_initialize() is. */
  sqlite3_shutdown();

  Tcl_ಕಂಡುಹಿಡಿ_Executable(argv[0]);
  Tcl_ಗಣ_System_Encoding(ಖಾಲಿ, "utf-8");
  interp = Tcl_CreateInterp();
  Sqlite3_Init(interp);

  sqlite3_snprintf(ಗಾತ್ರ(zArgc), zArgc, "%d", argc-1);
  Tcl_ಗಣ_Var(interp,"argc", zArgc, TCL_GLOBAL_ONLY);
  Tcl_ಗಣ_Var(interp,"argv0",argv[0],TCL_GLOBAL_ONLY);
  Tcl_ಗಣ_Var(interp,"argv", "", TCL_GLOBAL_ONLY);
  ಗಾಗಿ(i=1; i<argc; i++){
    Tcl_ಗಣ_Var(interp, "argv", argv[i],
        TCL_GLOBAL_ONLY | TCL_LIST_ELEMENT | TCL_APPEND_ಮೌಲ್ಯ);
  }
#ಆದರೆ defined(TCLSH_INIT_PROC)
  zScript = TCLSH_INIT_PROC(interp);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( zScript==0 ){
    zScript = tclsh_main_loop();
  }
  ಆದರೆ( Tcl_GlobalEval(interp, zScript)!=TCL_OK ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zInfo = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    ಆದರೆ( zInfo==0 ) zInfo = Tcl_Get_String_ಫಲಿತಾಂಶ(interp);
    fprintf(stderr,"%s: %s\n", *argv, zInfo);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* TCLSH */
