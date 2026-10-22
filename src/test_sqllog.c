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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _SQLITE3_H_
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
ਸਥਿਰ ਪੂਰਨ getProcessId(ਖਾਲੀ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਵਾਪਸ (ਪੂਰਨ)_getpid();
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ (ਪੂਰਨ)getpid();
#ਜੇ_ਅੰਤ
}

/* Names of environment variables to be used */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENVIRONMENT_VARIABLE1_NAME "SQLITE_SQLLOG_DIR"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENVIRONMENT_VARIABLE2_NAME "SQLITE_SQLLOG_REUSE_FILES"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENVIRONMENT_VARIABLE3_NAME "SQLITE_SQLLOG_CONDITIONAL"

/* Assume that all database and database file names are shorted than this. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLLOG_NAMESZ 512

/* Maximum number of simultaneous database connections the process may
** open (if any more are opened an error is logged using sqlite3_log()
** and processing is halted).
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_CONNECTIONS 256

/* There is one instance of this object for each SQLite database connection
** that is being logged.
*/
ਬਣਤਰ SLConn {
  ਪੂਰਨ isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  ਪੂਰਨ iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
ਸਥਿਰ ਬਣਤਰ SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  ਪੂਰਨ nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  ਪੂਰਨ bConditional;               /* Only trace if *-sqllog file is present */
  ਪੂਰਨ bReuse;                     /* True to avoid extra copies of db files */
  ਅੱਖਰ z_ਅਗੇਤਰ[SQLLOG_NAMESZ];    /* Prefix for all created files */
  ਅੱਖਰ zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  ਪੂਰਨ i_ਅਗਲਾ_Log;                   /* Used to allocate file names */
  ਪੂਰਨ i_ਅਗਲਾ_Db;                    /* Used to allocate database file names */
  ਪੂਰਨ bRec;                       /* True if testSqllog() is called rec. */
  ਪੂਰਨ iClock;                     /* Clock value */
  ਬਣਤਰ SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
ਸਥਿਰ ਪੂਰਨ sqllog_isspace(ਅੱਖਰ c){
  ਵਾਪਸ (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
ਸਥਿਰ ਖਾਲੀ sqllogTokenize(ਅਚਲ ਅੱਖਰ *z, ਅਚਲ ਅੱਖਰ **pz, ਪੂਰਨ *pn){
  ਅਚਲ ਅੱਖਰ *p = z;
  ਪੂਰਨ n;

  /* Skip past any whitespace */
  ਜਦੋਂ( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  ਜਦੋਂ( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
ਸਥਿਰ ਅੱਖਰ *sqllog_ਲੱਭੋ_File(ਅਚਲ ਅੱਖਰ *zFile){
  ਅੱਖਰ *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  ਜੇ( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    ਵਾਪਸ 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  ਜਦੋਂ( feof(fd)==0 ){
    ਅੱਖਰ z_ਸਤਰ[SQLLOG_NAMESZ*2+5];
    ਜੇ( fgets(z_ਸਤਰ, ਆਕਾਰ(z_ਸਤਰ), fd) ){
      ਪੂਰਨ n;
      ਅੱਖਰ *z;

      z_ਸਤਰ[ਆਕਾਰ(z_ਸਤਰ)-1] = '\0';
      z = z_ਸਤਰ;
      ਜਦੋਂ( *z>='0' && *z<='9' ) z++;
      ਜਦੋਂ( *z==' ' ) z++;

      n = ਲੜੀ_ਲੰਬਾਈ(z);
      ਜਦੋਂ( n>0 && sqllog_isspace(z[n-1]) ) n--;

      ਜੇ( n==ਲੜੀ_ਲੰਬਾਈ(zFile) && 0==memcmp(zFile, z, n) ){
        ਅੱਖਰ zBuf[16];
        memset(zBuf, 0, ਆਕਾਰ(zBuf));
        z = z_ਸਤਰ;
        ਜਦੋਂ( *z>='0' && *z<='9' ){
          zBuf[z-z_ਸਤਰ] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.z_ਅਗੇਤਰ, zBuf);
        ਰੋਕੋ;
      }
    }
  }

  ਜੇ( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  ਵਾਪਸ zRet;
}

ਸਥਿਰ ਪੂਰਨ sqllog_ਲੱਭੋ_Attached(
  sqlite3 *db,                    /* Database connection */
  ਅਚਲ ਅੱਖਰ *z_ਖੋਜ,            /* Name to search for (or NULL) */
  ਅੱਖਰ *zName,                    /* OUT: Name of attached database */
  ਅੱਖਰ *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  ਪੂਰਨ rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  ਜੇ( rc==SQLITE_OK ){
    ਜਦੋਂ( SQLITE_ਪੰਕਤੀ==sqlite3_ਕਦਮ(pStmt) ){
      ਅਚਲ ਅੱਖਰ *zVal1; ਪੂਰਨ nVal1;
      ਅਚਲ ਅੱਖਰ *zVal2; ਪੂਰਨ nVal2;

      zVal1 = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, 1);
      nVal1 = sqlite3_ਕਾਲਮ_ਬਾਈਟਾਂ(pStmt, 1);
      ਜੇ( zName ){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(zName, zVal1, nVal1+1);
      }

      zVal2 = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, 2);
      nVal2 = sqlite3_ਕਾਲਮ_ਬਾਈਟਾਂ(pStmt, 2);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zFile, zVal2, nVal2+1);

      ਜੇ( z_ਖੋਜ && ਲੜੀ_ਲੰਬਾਈ(z_ਖੋਜ)==nVal1 
       && 0==sqlite3_strnicmp(z_ਖੋਜ, zVal1, nVal1)
      ){
        ਰੋਕੋ;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  ਜੇ( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਖਾਲੀ sqllogCopydb(ਬਣਤਰ SLConn *p, ਅਚਲ ਅੱਖਰ *z_ਖੋਜ, ਪੂਰਨ bLog){
  ਅੱਖਰ zName[SQLLOG_NAMESZ];      /* Attached database name */
  ਅੱਖਰ zFile[SQLLOG_NAMESZ];      /* Database file name */
  ਅੱਖਰ *zFree;
  ਅੱਖਰ *zInit = 0;
  ਪੂਰਨ rc;

  rc = sqllog_ਲੱਭੋ_Attached(p->db, z_ਖੋਜ, zName, zFile);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ;

  ਜੇ( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }ਨਹੀਂਤਾਂ{
    ਜੇ( sqllogglobal.bReuse ){
      zInit = sqllog_ਲੱਭੋ_File(zFile);
    }ਨਹੀਂਤਾਂ{
      zInit = 0;
    }
    ਜੇ( zInit==0 ){
      ਪੂਰਨ rc;
      sqlite3 *copy = 0;
      ਪੂਰਨ iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.i_ਅਗਲਾ_Db++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.z_ਅਗੇਤਰ, iDb);

      /* Create the backup */
      ਪ੍ਰਤਿਜ੍ਞਾ( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      ਜੇ( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        ਜੇ( pBak ){
          sqlite3_backup_ਕਦਮ(pBak, -1);
          rc = sqlite3_backup_ਅੰਤ(pBak);
        }ਨਹੀਂਤਾਂ{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      ਜੇ( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        ਜੇ( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }ਨਹੀਂਤਾਂ{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  ਜੇ( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }ਨਹੀਂਤਾਂ{
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
ਸਥਿਰ ਖਾਲੀ sqllogOpenlog(ਬਣਤਰ SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  ਜੇ( p->fd==0 ){
    ਅੱਖਰ *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    ਜੇ( sqllogglobal.z_ਅਗੇਤਰ[0]==0 ){
      FILE *fd;
      ਅੱਖਰ *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      ਜੇ( zVar==0 || ਲੜੀ_ਲੰਬਾਈ(zVar)+10>=(ਆਕਾਰ(sqllogglobal.z_ਅਗੇਤਰ)) ) ਵਾਪਸ;
      sqlite3_snprintf(ਆਕਾਰ(sqllogglobal.z_ਅਗੇਤਰ), sqllogglobal.z_ਅਗੇਤਰ,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(ਆਕਾਰ(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.z_ਅਗੇਤਰ);
      ਜੇ( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      ਜੇ( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.z_ਅਗੇਤਰ, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    ਜੇ( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
ਸਥਿਰ ਖਾਲੀ testSqllogStmt(ਬਣਤਰ SLConn *p, ਅਚਲ ਅੱਖਰ *zSql){
  ਅਚਲ ਅੱਖਰ *z_ਪਹਿਲਾ;             /* Pointer to first token in zSql */
  ਪੂਰਨ n_ਪਹਿਲਾ;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &z_ਪਹਿਲਾ, &n_ਪਹਿਲਾ);
  ਜੇ( n_ਪਹਿਲਾ!=6 || 0!=sqlite3_strnicmp("ATTACH", z_ਪਹਿਲਾ, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }ਨਹੀਂਤਾਂ{
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
ਸਥਿਰ ਪੂਰਨ sqllogTraceDb(sqlite3 *db){
  ਪੂਰਨ bRet = 1;
  ਜੇ( sqllogglobal.bConditional ){
    ਅੱਖਰ zFile[SQLLOG_NAMESZ];      /* Attached database name */
    ਪੂਰਨ rc = sqllog_ਲੱਭੋ_Attached(db, "main", 0, zFile);
    ਜੇ( rc==SQLITE_OK ){
      ਪੂਰਨ nFile = ਲੜੀ_ਲੰਬਾਈ(zFile);
      ਜੇ( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }ਨਹੀਂਤਾਂ{
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  ਵਾਪਸ bRet;
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
ਸਥਿਰ ਖਾਲੀ testSqllog(ਖਾਲੀ *pCtx, sqlite3 *db, ਅਚਲ ਅੱਖਰ *zSql, ਪੂਰਨ eType){
  ਬਣਤਰ SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  ਪ੍ਰਤਿਜ੍ਞਾ( eType==0 || eType==1 || eType==2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  ਜੇ( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    ਜੇ( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    ਜੇ( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.i_ਅਗਲਾ_Log++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      ਜੇ( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  ਨਹੀਂਤਾਂ{

    ਪੂਰਨ i;
    ਲਈ(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      ਜੇ( p->db==db ) ਰੋਕੋ;
    }

    /* A database handle close command */
    ਜੇ( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      ਜੇ( i<sqllogglobal.nConn ){
        ਜੇ( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      ਜੇ( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }ਨਹੀਂਤਾਂ ਜੇ( i<sqllogglobal.nConn ){
        ਪੂਰਨ n_ਖਿਸਕਾਓ = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        ਜੇ( n_ਖਿਸਕਾਓ>0 ){
          memmove(p, &p[1], n_ਖਿਸਕਾਓ*ਆਕਾਰ(ਬਣਤਰ SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }ਨਹੀਂਤਾਂ ਜੇ( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      ਜੇ( sqllogglobal.bRec==0 ){
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
ਖਾਲੀ sqlite3_init_sqllog(ਖਾਲੀ){
  ਜੇ( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    ਜੇ( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      memset(&sqllogglobal, 0, ਆਕਾਰ(sqllogglobal));
      sqllogglobal.bReuse = 1;
      ਜੇ( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
