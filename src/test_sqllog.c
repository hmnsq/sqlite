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
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#endif
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getProcessId(ಶೂನ್ಯ){
#ifdef _WIN32
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)_getpid();
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)getpid();
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
ಸಂರಚನಾ SLConn {
  ಪೂರ್ಣಾಙ್ಕ isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  ಪೂರ್ಣಾಙ್ಕ iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
ಸ್ಥಿರ ಸಂರಚನಾ SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  ಪೂರ್ಣಾಙ್ಕ nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  ಪೂರ್ಣಾಙ್ಕ bConditional;               /* Only trace if *-sqllog file is present */
  ಪೂರ್ಣಾಙ್ಕ bReuse;                     /* True to avoid extra copies of db files */
  ವರ್ಣ zPrefix[SQLLOG_NAMESZ];    /* Prefix for all created files */
  ವರ್ಣ zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  ಪೂರ್ಣಾಙ್ಕ iNextLog;                   /* Used to allocate file names */
  ಪೂರ್ಣಾಙ್ಕ iNextDb;                    /* Used to allocate database file names */
  ಪೂರ್ಣಾಙ್ಕ bRec;                       /* True if testSqllog() is called rec. */
  ಪೂರ್ಣಾಙ್ಕ iClock;                     /* Clock value */
  ಸಂರಚನಾ SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqllog_isspace(ವರ್ಣ c){
  ಪ್ರತ್ಯಾಗಮ (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqllogTokenize(ಧ್ರುವ ವರ್ಣ *z, ಧ್ರುವ ವರ್ಣ **pz, ಪೂರ್ಣಾಙ್ಕ *pn){
  ಧ್ರುವ ವರ್ಣ *p = z;
  ಪೂರ್ಣಾಙ್ಕ n;

  /* Skip past any whitespace */
  ಯಾವತ್( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  ಯಾವತ್( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
ಸ್ಥಿರ ವರ್ಣ *sqllogFindFile(ಧ್ರುವ ವರ್ಣ *zFile){
  ವರ್ಣ *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  ಯದಿ( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    ಪ್ರತ್ಯಾಗಮ 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  ಯಾವತ್( feof(fd)==0 ){
    ವರ್ಣ z_ಸಾಲು[SQLLOG_NAMESZ*2+5];
    ಯದಿ( fgets(z_ಸಾಲು, ಪರಿಮಾಣ(z_ಸಾಲು), fd) ){
      ಪೂರ್ಣಾಙ್ಕ n;
      ವರ್ಣ *z;

      z_ಸಾಲು[ಪರಿಮಾಣ(z_ಸಾಲು)-1] = '\0';
      z = z_ಸಾಲು;
      ಯಾವತ್( *z>='0' && *z<='9' ) z++;
      ಯಾವತ್( *z==' ' ) z++;

      n = strlen(z);
      ಯಾವತ್( n>0 && sqllog_isspace(z[n-1]) ) n--;

      ಯದಿ( n==strlen(zFile) && 0==memcmp(zFile, z, n) ){
        ವರ್ಣ zBuf[16];
        memset(zBuf, 0, ಪರಿಮಾಣ(zBuf));
        z = z_ಸಾಲು;
        ಯಾವತ್( *z>='0' && *z<='9' ){
          zBuf[z-z_ಸಾಲು] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.zPrefix, zBuf);
        ಭಙ್ಗ;
      }
    }
  }

  ಯದಿ( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  ಪ್ರತ್ಯಾಗಮ zRet;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqllogFindAttached(
  sqlite3 *db,                    /* Database connection */
  ಧ್ರುವ ವರ್ಣ *z_ಹುಡುಕು,            /* Name to search for (or NULL) */
  ವರ್ಣ *zName,                    /* OUT: Name of attached database */
  ವರ್ಣ *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಙ್ಕ rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  ಪ್ರತಿಜ್ಞಾ( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  ಯದಿ( rc==SQLITE_OK ){
    ಯಾವತ್( SQLITE_ROW==sqlite3_step(pStmt) ){
      ಧ್ರುವ ವರ್ಣ *zVal1; ಪೂರ್ಣಾಙ್ಕ nVal1;
      ಧ್ರುವ ವರ್ಣ *zVal2; ಪೂರ್ಣಾಙ್ಕ nVal2;

      zVal1 = (ಧ್ರುವ ವರ್ಣ*)sqlite3_column_text(pStmt, 1);
      nVal1 = sqlite3_column_bytes(pStmt, 1);
      ಯದಿ( zName ){
        memcpy(zName, zVal1, nVal1+1);
      }

      zVal2 = (ಧ್ರುವ ವರ್ಣ*)sqlite3_column_text(pStmt, 2);
      nVal2 = sqlite3_column_bytes(pStmt, 2);
      memcpy(zFile, zVal2, nVal2+1);

      ಯದಿ( z_ಹುಡುಕು && strlen(z_ಹುಡುಕು)==nVal1 
       && 0==sqlite3_strnicmp(z_ಹುಡುಕು, zVal1, nVal1)
      ){
        ಭಙ್ಗ;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  ಯದಿ( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  ಪ್ರತ್ಯಾಗಮ rc;
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
ಸ್ಥಿರ ಶೂನ್ಯ sqllogCopydb(ಸಂರಚನಾ SLConn *p, ಧ್ರುವ ವರ್ಣ *z_ಹುಡುಕು, ಪೂರ್ಣಾಙ್ಕ bLog){
  ವರ್ಣ zName[SQLLOG_NAMESZ];      /* Attached database name */
  ವರ್ಣ zFile[SQLLOG_NAMESZ];      /* Database file name */
  ವರ್ಣ *zFree;
  ವರ್ಣ *zInit = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;

  rc = sqllogFindAttached(p->db, z_ಹುಡುಕು, zName, zFile);
  ಯದಿ( rc!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }ಅನ್ಯಥಾ{
    ಯದಿ( sqllogglobal.bReuse ){
      zInit = sqllogFindFile(zFile);
    }ಅನ್ಯಥಾ{
      zInit = 0;
    }
    ಯದಿ( zInit==0 ){
      ಪೂರ್ಣಾಙ್ಕ rc;
      sqlite3 *copy = 0;
      ಪೂರ್ಣಾಙ್ಕ iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.iNextDb++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.zPrefix, iDb);

      /* Create the backup */
      ಪ್ರತಿಜ್ಞಾ( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      ಯದಿ( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        ಯದಿ( pBak ){
          sqlite3_backup_step(pBak, -1);
          rc = sqlite3_backup_finish(pBak);
        }ಅನ್ಯಥಾ{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      ಯದಿ( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        ಯದಿ( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }ಅನ್ಯಥಾ{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  ಯದಿ( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಶೂನ್ಯ sqllogOpenlog(ಸಂರಚನಾ SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  ಯದಿ( p->fd==0 ){
    ವರ್ಣ *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    ಯದಿ( sqllogglobal.zPrefix[0]==0 ){
      FILE *fd;
      ವರ್ಣ *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      ಯದಿ( zVar==0 || strlen(zVar)+10>=(ಪರಿಮಾಣ(sqllogglobal.zPrefix)) ) ಪ್ರತ್ಯಾಗಮ;
      sqlite3_snprintf(ಪರಿಮಾಣ(sqllogglobal.zPrefix), sqllogglobal.zPrefix,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(ಪರಿಮಾಣ(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.zPrefix);
      ಯದಿ( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      ಯದಿ( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.zPrefix, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    ಯದಿ( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testSqllogStmt(ಸಂರಚನಾ SLConn *p, ಧ್ರುವ ವರ್ಣ *zSql){
  ಧ್ರುವ ವರ್ಣ *zFirst;             /* Pointer to first token in zSql */
  ಪೂರ್ಣಾಙ್ಕ nFirst;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &zFirst, &nFirst);
  ಯದಿ( nFirst!=6 || 0!=sqlite3_strnicmp("ATTACH", zFirst, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqllogTraceDb(sqlite3 *db){
  ಪೂರ್ಣಾಙ್ಕ bRet = 1;
  ಯದಿ( sqllogglobal.bConditional ){
    ವರ್ಣ zFile[SQLLOG_NAMESZ];      /* Attached database name */
    ಪೂರ್ಣಾಙ್ಕ rc = sqllogFindAttached(db, "main", 0, zFile);
    ಯದಿ( rc==SQLITE_OK ){
      ಪೂರ್ಣಾಙ್ಕ nFile = strlen(zFile);
      ಯದಿ( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }ಅನ್ಯಥಾ{
        memcpy(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ bRet;
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
ಸ್ಥಿರ ಶೂನ್ಯ testSqllog(ಶೂನ್ಯ *pCtx, sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zSql, ಪೂರ್ಣಾಙ್ಕ eType){
  ಸಂರಚನಾ SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  ಪ್ರತಿಜ್ಞಾ( eType==0 || eType==1 || eType==2 );
  ಪ್ರತಿಜ್ಞಾ( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  ಯದಿ( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    ಯದಿ( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    ಯದಿ( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.iNextLog++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      ಯದಿ( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  ಅನ್ಯಥಾ{

    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      ಯದಿ( p->db==db ) ಭಙ್ಗ;
    }

    /* A database handle close command */
    ಯದಿ( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      ಯದಿ( i<sqllogglobal.nConn ){
        ಯದಿ( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      ಯದಿ( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }ಅನ್ಯಥಾ ಯದಿ( i<sqllogglobal.nConn ){
        ಪೂರ್ಣಾಙ್ಕ nShift = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        ಯದಿ( nShift>0 ){
          memmove(p, &p[1], nShift*ಪರಿಮಾಣ(ಸಂರಚನಾ SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }ಅನ್ಯಥಾ ಯದಿ( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      ಯದಿ( sqllogglobal.bRec==0 ){
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
ಶೂನ್ಯ sqlite3_init_sqllog(ಶೂನ್ಯ){
  ಯದಿ( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    ಯದಿ( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      memset(&sqllogglobal, 0, ಪರಿಮಾಣ(sqllogglobal));
      sqllogglobal.bReuse = 1;
      ಯದಿ( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
