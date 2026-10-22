/*
** 2012 November 26
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** OVERVIEW
**
**   This file contains experimental code used to record data from live
**   SQLite applications that may be useful for offline analysis. 
**   Specifically, this module can be used to capture the following
**   information:
**
**     1) The initial contents of all database files opened by the 
**        application, and
**
**     2) All SQL statements executed by the application.
**
**   The captured information can then be used to run (for example)
**   performance analysis looking for slow queries or to look for
**   optimization opportunities in either the application or in SQLite
**   itself.
**
** USAGE
**
**   To use this module, SQLite must be compiled with the SQLITE_ENABLE_SQLLOG
**   pre-processor symbol defined and this file linked into the application.
**   One way to link this file into the application is to append the content
**   of this file onto the end of the "sqlite3.c" amalgamation and then 
**   recompile the application as normal except with the addition  of the
**   -DSQLITE_ENABLE_SQLLOG option.
**
**   At runtime, logging is enabled by setting environment variable
**   SQLITE_SQLLOG_DIR to the name of a directory in which to store logged 
**   data. The logging directory must already exist.
**
**   Usually, if the application opens the same database file more than once
**   (either by attaching it or by using more than one database handle), only
**   a single copy is made. This behavior may be overridden (so that a 
**   separate copy is taken each time the database file is opened or attached)
**   by setting the environment variable SQLITE_SQLLOG_REUSE_FILES to 0.
**
**   If the environment variable SQLITE_SQLLOG_CONDITIONAL is defined, then
**   logging is only done for database connections if a file named
**   "<database>-sqllog" exists in the same directly as the main database
**   file when it is first opened ("<database>" is replaced by the actual 
**   name of the main database file).
**
** OUTPUT:
**
**   The SQLITE_SQLLOG_DIR is populated with three types of files:
**
**      sqllog_N.db   - Copies of database files. N may be any integer.
**
**      sqllog_N.sql  - A list of SQL statements executed by a single
**                      connection. N may be any integer.
**
**      sqllog.idx    - An index mapping from integer N to a database
**                      file name - indicating the full path of the
**                      database from which sqllog_N.db was copied.
**
** ERROR HANDLING:
**
**   This module attempts to make a best effort to continue logging if an
**   IO or other error is encountered. For example, if a log file cannot 
**   be opened logs are not collected for that connection, but other
**   logging proceeds as expected. Errors are logged by calling sqlite3_log().
*/

#ifndef _SQLITE3_H_
#ضمن "sqlite3.h"
#endif
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>

#ضمن <sys/types.h>
#ضمن <unistd.h>
ساكن صحيح getProcessId(فراغ){
#ifdef _WIN32
  أرجع (صحيح)_getpid();
#وإلا
  أرجع (صحيح)getpid();
#endif
}

/* Names of environment variables to be used */
#define ENVIRONMENT_VARIABLE1_NAME "SQLITE_SQLLOG_DIR"
#define ENVIRONMENT_VARIABLE2_NAME "SQLITE_SQLLOG_REUSE_FILES"
#define ENVIRONMENT_VARIABLE3_NAME "SQLITE_SQLLOG_CONDITIONAL"

/* Assume that all database and database file names are shorted than this. */
#define SQLLOG_NAMESZ 512

/* Maximum number of simultaneous database connections the process may
** open (if any more are opened an error is logged using sqlite3_log()
** and processing is halted).
*/
#define MAX_CONNECTIONS 256

/* There is one instance of this object for each SQLite database connection
** that is being logged.
*/
بنية SLConn {
  صحيح isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  صحيح iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
ساكن بنية SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  صحيح nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  صحيح bConditional;               /* Only trace if *-sqllog file is present */
  صحيح bReuse;                     /* True to avoid extra copies of db files */
  حرف zPrefix[SQLLOG_NAMESZ];    /* Prefix for all created files */
  حرف zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  صحيح iNextLog;                   /* Used to allocate file names */
  صحيح iNextDb;                    /* Used to allocate database file names */
  صحيح bRec;                       /* True if testSqllog() is called rec. */
  صحيح iClock;                     /* Clock value */
  بنية SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
ساكن صحيح sqllog_isspace(حرف c){
  أرجع (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
ساكن فراغ sqllogTokenize(ثابت حرف *z, ثابت حرف **pz, صحيح *pn){
  ثابت حرف *p = z;
  صحيح n;

  /* Skip past any whitespace */
  طالما( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  طالما( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
  *pn = n;
}

/*
** Check if the logs directory already contains a copy of database file 
** zFile. If so, return a pointer to the full path of the copy. Otherwise,
** return NULL.
**
** If a non-NULL value is returned, then the caller must arrange to 
** eventually free it using sqlite3_free().
*/
ساكن حرف *sqllogFindFile(ثابت حرف *zFile){
  حرف *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  إذا( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    أرجع 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  طالما( feof(fd)==0 ){
    حرف z_سطر[SQLLOG_NAMESZ*2+5];
    إذا( fgets(z_سطر, حجم(z_سطر), fd) ){
      صحيح n;
      حرف *z;

      z_سطر[حجم(z_سطر)-1] = '\0';
      z = z_سطر;
      طالما( *z>='0' && *z<='9' ) z++;
      طالما( *z==' ' ) z++;

      n = strlen(z);
      طالما( n>0 && sqllog_isspace(z[n-1]) ) n--;

      إذا( n==strlen(zFile) && 0==memcmp(zFile, z, n) ){
        حرف zBuf[16];
        memset(zBuf, 0, حجم(zBuf));
        z = z_سطر;
        طالما( *z>='0' && *z<='9' ){
          zBuf[z-z_سطر] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.zPrefix, zBuf);
        اكسر;
      }
    }
  }

  إذا( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  أرجع zRet;
}

ساكن صحيح sqllogFindAttached(
  sqlite3 *db,                    /* Database connection */
  ثابت حرف *z_بحث,            /* Name to search for (or NULL) */
  حرف *zName,                    /* OUT: Name of attached database */
  حرف *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  صحيح rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  أكد( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  إذا( rc==SQLITE_OK ){
    طالما( SQLITE_ROW==sqlite3_step(pStmt) ){
      ثابت حرف *zVal1; صحيح nVal1;
      ثابت حرف *zVal2; صحيح nVal2;

      zVal1 = (ثابت حرف*)sqlite3_column_text(pStmt, 1);
      nVal1 = sqlite3_column_bytes(pStmt, 1);
      إذا( zName ){
        memcpy(zName, zVal1, nVal1+1);
      }

      zVal2 = (ثابت حرف*)sqlite3_column_text(pStmt, 2);
      nVal2 = sqlite3_column_bytes(pStmt, 2);
      memcpy(zFile, zVal2, nVal2+1);

      إذا( z_بحث && strlen(z_بحث)==nVal1 
       && 0==sqlite3_strnicmp(z_بحث, zVal1, nVal1)
      ){
        اكسر;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  إذا( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  أرجع rc;
}


/*
** Parameter zSearch is the name of a database attached to the database 
** connection associated with the first argument. This function creates
** a backup of this database in the logs directory.
**
** The name used for the backup file is automatically generated. Call
** it zFile.
**
** If the bLog parameter is true, then a statement of the following form
** is written to the log file associated with *p:
**
**    ATTACH 'zFile' AS 'zName';
**
** Otherwise, if bLog is false, a comment is added to the log file:
**
**    -- Main database file is 'zFile'
**
** The SLGlobal.mutex mutex is always held when this function is called.
*/
ساكن فراغ sqllogCopydb(بنية SLConn *p, ثابت حرف *z_بحث, صحيح bLog){
  حرف zName[SQLLOG_NAMESZ];      /* Attached database name */
  حرف zFile[SQLLOG_NAMESZ];      /* Database file name */
  حرف *zFree;
  حرف *zInit = 0;
  صحيح rc;

  rc = sqllogFindAttached(p->db, z_بحث, zName, zFile);
  إذا( rc!=SQLITE_OK ) أرجع;

  إذا( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }وإلا{
    إذا( sqllogglobal.bReuse ){
      zInit = sqllogFindFile(zFile);
    }وإلا{
      zInit = 0;
    }
    إذا( zInit==0 ){
      صحيح rc;
      sqlite3 *copy = 0;
      صحيح iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.iNextDb++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.zPrefix, iDb);

      /* Create the backup */
      أكد( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      إذا( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        إذا( pBak ){
          sqlite3_backup_step(pBak, -1);
          rc = sqlite3_backup_finish(pBak);
        }وإلا{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      إذا( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        إذا( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }وإلا{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  إذا( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }وإلا{
    zFree = sqlite3_mprintf("-- Main database is '%q'\n", zInit);
  }
  fprintf(p->fd, "%s", zFree);
  sqlite3_free(zFree);

  sqlite3_free(zInit);
}

/*
** If it is not already open, open the log file for connection *p. 
**
** The SLGlobal.mutex mutex is always held when this function is called.
*/
ساكن فراغ sqllogOpenlog(بنية SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  إذا( p->fd==0 ){
    حرف *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    إذا( sqllogglobal.zPrefix[0]==0 ){
      FILE *fd;
      حرف *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      إذا( zVar==0 || strlen(zVar)+10>=(حجم(sqllogglobal.zPrefix)) ) أرجع;
      sqlite3_snprintf(حجم(sqllogglobal.zPrefix), sqllogglobal.zPrefix,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(حجم(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.zPrefix);
      إذا( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      إذا( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.zPrefix, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    إذا( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
ساكن فراغ testSqllogStmt(بنية SLConn *p, ثابت حرف *zSql){
  ثابت حرف *zFirst;             /* Pointer to first token in zSql */
  صحيح nFirst;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &zFirst, &nFirst);
  إذا( nFirst!=6 || 0!=sqlite3_strnicmp("ATTACH", zFirst, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }وإلا{
    /* This is an ATTACH statement. Copy the database. */
    sqllogCopydb(p, 0, 1);
  }
}

/*
** The database handle passed as the only argument has just been opened.
** Return true if this module should log initial databases and SQL 
** statements for this connection, or false otherwise.
**
** If an error occurs, sqlite3_log() is invoked to report it to the user
** and zero returned.
*/
ساكن صحيح sqllogTraceDb(sqlite3 *db){
  صحيح bRet = 1;
  إذا( sqllogglobal.bConditional ){
    حرف zFile[SQLLOG_NAMESZ];      /* Attached database name */
    صحيح rc = sqllogFindAttached(db, "main", 0, zFile);
    إذا( rc==SQLITE_OK ){
      صحيح nFile = strlen(zFile);
      إذا( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }وإلا{
        memcpy(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  أرجع bRet;
}

/*
** The SQLITE_CONFIG_SQLLOG callback registered by sqlite3_init_sqllog().
**
** The eType parameter has the following values:
**
**    0:  Opening a new database connection.  zSql is the name of the
**        file being opened.  db is a pointer to the newly created database
**        connection.
**
**    1:  An SQL statement has run to completion.  zSql is the text of the
**        SQL statement with all parameters expanded to their actual values.
**
**    2:  Closing a database connection.  zSql is NULL.  The db pointer to
**        the database connection being closed has already been shut down
**        and cannot be used for any further SQL.
**
** The pCtx parameter is a copy of the pointer that was originally passed
** into the sqlite3_config(SQLITE_CONFIG_SQLLOG) statement.  In this
** particular implementation, pCtx is always a pointer to the 
** sqllogglobal global variable define above.
*/
ساكن فراغ testSqllog(فراغ *pCtx, sqlite3 *db, ثابت حرف *zSql, صحيح eType){
  بنية SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  أكد( eType==0 || eType==1 || eType==2 );
  أكد( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  إذا( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    إذا( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    إذا( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.iNextLog++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      إذا( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  وإلا{

    صحيح i;
    لكل(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      إذا( p->db==db ) اكسر;
    }

    /* A database handle close command */
    إذا( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      إذا( i<sqllogglobal.nConn ){
        إذا( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      إذا( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }وإلا إذا( i<sqllogglobal.nConn ){
        صحيح nShift = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        إذا( nShift>0 ){
          memmove(p, &p[1], nShift*حجم(بنية SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }وإلا إذا( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      إذا( sqllogglobal.bRec==0 ){
        testSqllogStmt(p, zSql);
      }
      sqlite3_mutex_leave(sqllogglobal.mutex);
    }
  }
}

/*
** This function is called either before sqlite3_initialized() or by it.
** It checks if the SQLITE_SQLLOG_DIR variable is defined, and if so 
** registers an SQLITE_CONFIG_SQLLOG callback to record the applications
** database activity.
*/
فراغ sqlite3_init_sqllog(فراغ){
  إذا( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    إذا( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      memset(&sqllogglobal, 0, حجم(sqllogglobal));
      sqllogglobal.bReuse = 1;
      إذا( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
