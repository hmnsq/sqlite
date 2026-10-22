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
#அந்தர்பாவ "sqlite3.h"
#endif
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>

#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <unistd.h>
நிலையான முழு getProcessId(வெறுமை){
#ifdef _WIN32
  திரும்பு (முழு)_getpid();
#இல்லையெனில்
  திரும்பு (முழு)getpid();
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
கட்டமைப்பு SLConn {
  முழு isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  முழு iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
நிலையான கட்டமைப்பு SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  முழு nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  முழு bConditional;               /* Only trace if *-sqllog file is present */
  முழு bReuse;                     /* True to avoid extra copies of db files */
  எழுத்து zPrefix[SQLLOG_NAMESZ];    /* Prefix for all created files */
  எழுத்து zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  முழு iNextLog;                   /* Used to allocate file names */
  முழு iNextDb;                    /* Used to allocate database file names */
  முழு bRec;                       /* True if testSqllog() is called rec. */
  முழு iClock;                     /* Clock value */
  கட்டமைப்பு SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
நிலையான முழு sqllog_isspace(எழுத்து c){
  திரும்பு (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
நிலையான வெறுமை sqllogTokenize(மாறிலி எழுத்து *z, மாறிலி எழுத்து **pz, முழு *pn){
  மாறிலி எழுத்து *p = z;
  முழு n;

  /* Skip past any whitespace */
  போது( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  போது( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
நிலையான எழுத்து *sqllogFindFile(மாறிலி எழுத்து *zFile){
  எழுத்து *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  எனில்( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    திரும்பு 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  போது( feof(fd)==0 ){
    எழுத்து z_வரி[SQLLOG_NAMESZ*2+5];
    எனில்( fgets(z_வரி, அளவு(z_வரி), fd) ){
      முழு n;
      எழுத்து *z;

      z_வரி[அளவு(z_வரி)-1] = '\0';
      z = z_வரி;
      போது( *z>='0' && *z<='9' ) z++;
      போது( *z==' ' ) z++;

      n = strlen(z);
      போது( n>0 && sqllog_isspace(z[n-1]) ) n--;

      எனில்( n==strlen(zFile) && 0==memcmp(zFile, z, n) ){
        எழுத்து zBuf[16];
        memset(zBuf, 0, அளவு(zBuf));
        z = z_வரி;
        போது( *z>='0' && *z<='9' ){
          zBuf[z-z_வரி] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.zPrefix, zBuf);
        நிறுத்து;
      }
    }
  }

  எனில்( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  திரும்பு zRet;
}

நிலையான முழு sqllogFindAttached(
  sqlite3 *db,                    /* Database connection */
  மாறிலி எழுத்து *z_தேடல்,            /* Name to search for (or NULL) */
  எழுத்து *zName,                    /* OUT: Name of attached database */
  எழுத்து *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  முழு rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  ப்ரதிஜ்ஞா( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  எனில்( rc==SQLITE_OK ){
    போது( SQLITE_ROW==sqlite3_step(pStmt) ){
      மாறிலி எழுத்து *zVal1; முழு nVal1;
      மாறிலி எழுத்து *zVal2; முழு nVal2;

      zVal1 = (மாறிலி எழுத்து*)sqlite3_column_text(pStmt, 1);
      nVal1 = sqlite3_column_bytes(pStmt, 1);
      எனில்( zName ){
        memcpy(zName, zVal1, nVal1+1);
      }

      zVal2 = (மாறிலி எழுத்து*)sqlite3_column_text(pStmt, 2);
      nVal2 = sqlite3_column_bytes(pStmt, 2);
      memcpy(zFile, zVal2, nVal2+1);

      எனில்( z_தேடல் && strlen(z_தேடல்)==nVal1 
       && 0==sqlite3_strnicmp(z_தேடல், zVal1, nVal1)
      ){
        நிறுத்து;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  எனில்( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  திரும்பு rc;
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
நிலையான வெறுமை sqllogCopydb(கட்டமைப்பு SLConn *p, மாறிலி எழுத்து *z_தேடல், முழு bLog){
  எழுத்து zName[SQLLOG_NAMESZ];      /* Attached database name */
  எழுத்து zFile[SQLLOG_NAMESZ];      /* Database file name */
  எழுத்து *zFree;
  எழுத்து *zInit = 0;
  முழு rc;

  rc = sqllogFindAttached(p->db, z_தேடல், zName, zFile);
  எனில்( rc!=SQLITE_OK ) திரும்பு;

  எனில்( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }இல்லையெனில்{
    எனில்( sqllogglobal.bReuse ){
      zInit = sqllogFindFile(zFile);
    }இல்லையெனில்{
      zInit = 0;
    }
    எனில்( zInit==0 ){
      முழு rc;
      sqlite3 *copy = 0;
      முழு iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.iNextDb++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.zPrefix, iDb);

      /* Create the backup */
      ப்ரதிஜ்ஞா( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      எனில்( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        எனில்( pBak ){
          sqlite3_backup_step(pBak, -1);
          rc = sqlite3_backup_finish(pBak);
        }இல்லையெனில்{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      எனில்( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        எனில்( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }இல்லையெனில்{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  எனில்( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }இல்லையெனில்{
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
நிலையான வெறுமை sqllogOpenlog(கட்டமைப்பு SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  எனில்( p->fd==0 ){
    எழுத்து *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    எனில்( sqllogglobal.zPrefix[0]==0 ){
      FILE *fd;
      எழுத்து *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      எனில்( zVar==0 || strlen(zVar)+10>=(அளவு(sqllogglobal.zPrefix)) ) திரும்பு;
      sqlite3_snprintf(அளவு(sqllogglobal.zPrefix), sqllogglobal.zPrefix,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(அளவு(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.zPrefix);
      எனில்( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      எனில்( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.zPrefix, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    எனில்( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
நிலையான வெறுமை testSqllogStmt(கட்டமைப்பு SLConn *p, மாறிலி எழுத்து *zSql){
  மாறிலி எழுத்து *zFirst;             /* Pointer to first token in zSql */
  முழு nFirst;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &zFirst, &nFirst);
  எனில்( nFirst!=6 || 0!=sqlite3_strnicmp("ATTACH", zFirst, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }இல்லையெனில்{
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
நிலையான முழு sqllogTraceDb(sqlite3 *db){
  முழு bRet = 1;
  எனில்( sqllogglobal.bConditional ){
    எழுத்து zFile[SQLLOG_NAMESZ];      /* Attached database name */
    முழு rc = sqllogFindAttached(db, "main", 0, zFile);
    எனில்( rc==SQLITE_OK ){
      முழு nFile = strlen(zFile);
      எனில்( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }இல்லையெனில்{
        memcpy(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  திரும்பு bRet;
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
நிலையான வெறுமை testSqllog(வெறுமை *pCtx, sqlite3 *db, மாறிலி எழுத்து *zSql, முழு eType){
  கட்டமைப்பு SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  ப்ரதிஜ்ஞா( eType==0 || eType==1 || eType==2 );
  ப்ரதிஜ்ஞா( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  எனில்( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    எனில்( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    எனில்( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.iNextLog++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      எனில்( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  இல்லையெனில்{

    முழு i;
    க்கு(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      எனில்( p->db==db ) நிறுத்து;
    }

    /* A database handle close command */
    எனில்( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      எனில்( i<sqllogglobal.nConn ){
        எனில்( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      எனில்( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }இல்லையெனில் எனில்( i<sqllogglobal.nConn ){
        முழு nShift = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        எனில்( nShift>0 ){
          memmove(p, &p[1], nShift*அளவு(கட்டமைப்பு SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }இல்லையெனில் எனில்( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      எனில்( sqllogglobal.bRec==0 ){
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
வெறுமை sqlite3_init_sqllog(வெறுமை){
  எனில்( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    எனில்( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      memset(&sqllogglobal, 0, அளவு(sqllogglobal));
      sqllogglobal.bReuse = 1;
      எனில்( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
