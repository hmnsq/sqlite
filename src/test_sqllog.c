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
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#endif
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getProcessId(ଶୂନ୍ୟ){
#ifdef _WIN32
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)_getpid();
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)getpid();
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
ସଂରଚନା SLConn {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
ସ୍ଥିର ସଂରଚନା SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bConditional;               /* Only trace if *-sqllog file is present */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bReuse;                     /* True to avoid extra copies of db files */
  ଅକ୍ଷର zPrefix[SQLLOG_NAMESZ];    /* Prefix for all created files */
  ଅକ୍ଷର zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNextLog;                   /* Used to allocate file names */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNextDb;                    /* Used to allocate database file names */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bRec;                       /* True if testSqllog() is called rec. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iClock;                     /* Clock value */
  ସଂରଚନା SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqllog_isspace(ଅକ୍ଷର c){
  ଫେରନ୍ତୁ (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
ସ୍ଥିର ଶୂନ୍ୟ sqllogTokenize(ଅଚଳ ଅକ୍ଷର *z, ଅଚଳ ଅକ୍ଷର **pz, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pn){
  ଅଚଳ ଅକ୍ଷର *p = z;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

  /* Skip past any whitespace */
  ଯେତେବେଳେ( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  ଯେତେବେଳେ( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
ସ୍ଥିର ଅକ୍ଷର *sqllogFindFile(ଅଚଳ ଅକ୍ଷର *zFile){
  ଅକ୍ଷର *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  ଯଦି( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    ଫେରନ୍ତୁ 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  ଯେତେବେଳେ( feof(fd)==0 ){
    ଅକ୍ଷର z_ଧାଡ଼ି[SQLLOG_NAMESZ*2+5];
    ଯଦି( fgets(z_ଧାଡ଼ି, ଆକାର(z_ଧାଡ଼ି), fd) ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
      ଅକ୍ଷର *z;

      z_ଧାଡ଼ି[ଆକାର(z_ଧାଡ଼ି)-1] = '\0';
      z = z_ଧାଡ଼ି;
      ଯେତେବେଳେ( *z>='0' && *z<='9' ) z++;
      ଯେତେବେଳେ( *z==' ' ) z++;

      n = strlen(z);
      ଯେତେବେଳେ( n>0 && sqllog_isspace(z[n-1]) ) n--;

      ଯଦି( n==strlen(zFile) && 0==memcmp(zFile, z, n) ){
        ଅକ୍ଷର zBuf[16];
        memset(zBuf, 0, ଆକାର(zBuf));
        z = z_ଧାଡ଼ି;
        ଯେତେବେଳେ( *z>='0' && *z<='9' ){
          zBuf[z-z_ଧାଡ଼ି] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.zPrefix, zBuf);
        ଭାଙ୍ଗ;
      }
    }
  }

  ଯଦି( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  ଫେରନ୍ତୁ zRet;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqllogFindAttached(
  sqlite3 *db,                    /* Database connection */
  ଅଚଳ ଅକ୍ଷର *z_ସନ୍ଧାନ,            /* Name to search for (or NULL) */
  ଅକ୍ଷର *zName,                    /* OUT: Name of attached database */
  ଅକ୍ଷର *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  ପ୍ରତିଜ୍ଞା( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  ଯଦି( rc==SQLITE_OK ){
    ଯେତେବେଳେ( SQLITE_ROW==sqlite3_step(pStmt) ){
      ଅଚଳ ଅକ୍ଷର *zVal1; ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVal1;
      ଅଚଳ ଅକ୍ଷର *zVal2; ପୂର୍ଣ୍ଣସଂଖ୍ୟା nVal2;

      zVal1 = (ଅଚଳ ଅକ୍ଷର*)sqlite3_column_text(pStmt, 1);
      nVal1 = sqlite3_column_bytes(pStmt, 1);
      ଯଦି( zName ){
        memcpy(zName, zVal1, nVal1+1);
      }

      zVal2 = (ଅଚଳ ଅକ୍ଷର*)sqlite3_column_text(pStmt, 2);
      nVal2 = sqlite3_column_bytes(pStmt, 2);
      memcpy(zFile, zVal2, nVal2+1);

      ଯଦି( z_ସନ୍ଧାନ && strlen(z_ସନ୍ଧାନ)==nVal1 
       && 0==sqlite3_strnicmp(z_ସନ୍ଧାନ, zVal1, nVal1)
      ){
        ଭାଙ୍ଗ;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  ଯଦି( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ଶୂନ୍ୟ sqllogCopydb(ସଂରଚନା SLConn *p, ଅଚଳ ଅକ୍ଷର *z_ସନ୍ଧାନ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bLog){
  ଅକ୍ଷର zName[SQLLOG_NAMESZ];      /* Attached database name */
  ଅକ୍ଷର zFile[SQLLOG_NAMESZ];      /* Database file name */
  ଅକ୍ଷର *zFree;
  ଅକ୍ଷର *zInit = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  rc = sqllogFindAttached(p->db, z_ସନ୍ଧାନ, zName, zFile);
  ଯଦି( rc!=SQLITE_OK ) ଫେରନ୍ତୁ;

  ଯଦି( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }ଅନ୍ୟଥା{
    ଯଦି( sqllogglobal.bReuse ){
      zInit = sqllogFindFile(zFile);
    }ଅନ୍ୟଥା{
      zInit = 0;
    }
    ଯଦି( zInit==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
      sqlite3 *copy = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.iNextDb++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.zPrefix, iDb);

      /* Create the backup */
      ପ୍ରତିଜ୍ଞା( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      ଯଦି( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        ଯଦି( pBak ){
          sqlite3_backup_step(pBak, -1);
          rc = sqlite3_backup_finish(pBak);
        }ଅନ୍ୟଥା{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      ଯଦି( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        ଯଦି( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }ଅନ୍ୟଥା{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  ଯଦି( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ଶୂନ୍ୟ sqllogOpenlog(ସଂରଚନା SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  ଯଦି( p->fd==0 ){
    ଅକ୍ଷର *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    ଯଦି( sqllogglobal.zPrefix[0]==0 ){
      FILE *fd;
      ଅକ୍ଷର *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      ଯଦି( zVar==0 || strlen(zVar)+10>=(ଆକାର(sqllogglobal.zPrefix)) ) ଫେରନ୍ତୁ;
      sqlite3_snprintf(ଆକାର(sqllogglobal.zPrefix), sqllogglobal.zPrefix,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(ଆକାର(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.zPrefix);
      ଯଦି( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      ଯଦି( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.zPrefix, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    ଯଦି( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
ସ୍ଥିର ଶୂନ୍ୟ testSqllogStmt(ସଂରଚନା SLConn *p, ଅଚଳ ଅକ୍ଷର *zSql){
  ଅଚଳ ଅକ୍ଷର *zFirst;             /* Pointer to first token in zSql */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFirst;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &zFirst, &nFirst);
  ଯଦି( nFirst!=6 || 0!=sqlite3_strnicmp("ATTACH", zFirst, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }ଅନ୍ୟଥା{
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqllogTraceDb(sqlite3 *db){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bRet = 1;
  ଯଦି( sqllogglobal.bConditional ){
    ଅକ୍ଷର zFile[SQLLOG_NAMESZ];      /* Attached database name */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqllogFindAttached(db, "main", 0, zFile);
    ଯଦି( rc==SQLITE_OK ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFile = strlen(zFile);
      ଯଦି( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }ଅନ୍ୟଥା{
        memcpy(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  ଫେରନ୍ତୁ bRet;
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
ସ୍ଥିର ଶୂନ୍ୟ testSqllog(ଶୂନ୍ୟ *pCtx, sqlite3 *db, ଅଚଳ ଅକ୍ଷର *zSql, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType){
  ସଂରଚନା SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  ପ୍ରତିଜ୍ଞା( eType==0 || eType==1 || eType==2 );
  ପ୍ରତିଜ୍ଞା( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  ଯଦି( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    ଯଦି( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    ଯଦି( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.iNextLog++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      ଯଦି( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  ଅନ୍ୟଥା{

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      ଯଦି( p->db==db ) ଭାଙ୍ଗ;
    }

    /* A database handle close command */
    ଯଦି( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      ଯଦି( i<sqllogglobal.nConn ){
        ଯଦି( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      ଯଦି( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }ଅନ୍ୟଥା ଯଦି( i<sqllogglobal.nConn ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nShift = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        ଯଦି( nShift>0 ){
          memmove(p, &p[1], nShift*ଆକାର(ସଂରଚନା SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }ଅନ୍ୟଥା ଯଦି( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      ଯଦି( sqllogglobal.bRec==0 ){
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
ଶୂନ୍ୟ sqlite3_init_sqllog(ଶୂନ୍ୟ){
  ଯଦି( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    ଯଦି( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      memset(&sqllogglobal, 0, ଆକାର(sqllogglobal));
      sqllogglobal.bReuse = 1;
      ଯଦି( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
