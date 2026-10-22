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

#నిర్వచితం_కాకపోతే _SQLITE3_H_
#అన్తర్భావ "sqlite3.h"
#అయితే_ముగింపు
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>

#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <unistd.h>
స్థిర పూర్ణాంకం getProcessId(శూన్య){
#నిర్వచితమైతే _WIN32
  తిరిగి (పూర్ణాంకం)_getpid();
#లేకపోతే
  తిరిగి (పూర్ణాంకం)getpid();
#అయితే_ముగింపు
}

/* Names of environment variables to be used */
#నిర్వచించు ENVIRONMENT_VARIABLE1_NAME "SQLITE_SQLLOG_DIR"
#నిర్వచించు ENVIRONMENT_VARIABLE2_NAME "SQLITE_SQLLOG_REUSE_FILES"
#నిర్వచించు ENVIRONMENT_VARIABLE3_NAME "SQLITE_SQLLOG_CONDITIONAL"

/* Assume that all database and database file names are shorted than this. */
#నిర్వచించు SQLLOG_NAMESZ 512

/* Maximum number of simultaneous database connections the process may
** open (if any more are opened an error is logged using sqlite3_log()
** and processing is halted).
*/
#నిర్వచించు MAX_CONNECTIONS 256

/* There is one instance of this object for each SQLite database connection
** that is being logged.
*/
నిర్మాణం SLConn {
  పూర్ణాంకం isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  పూర్ణాంకం iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
స్థిర నిర్మాణం SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  పూర్ణాంకం nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  పూర్ణాంకం bConditional;               /* Only trace if *-sqllog file is present */
  పూర్ణాంకం bReuse;                     /* True to avoid extra copies of db files */
  అక్షరం z_ఉపసర్గ[SQLLOG_NAMESZ];    /* Prefix for all created files */
  అక్షరం zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  పూర్ణాంకం i_తదుపరి_Log;                   /* Used to allocate file names */
  పూర్ణాంకం i_తదుపరి_Db;                    /* Used to allocate database file names */
  పూర్ణాంకం bRec;                       /* True if testSqllog() is called rec. */
  పూర్ణాంకం iClock;                     /* Clock value */
  నిర్మాణం SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
స్థిర పూర్ణాంకం sqllog_isspace(అక్షరం c){
  తిరిగి (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
స్థిర శూన్య sqllogTokenize(స్థిరరాశి అక్షరం *z, స్థిరరాశి అక్షరం **pz, పూర్ణాంకం *pn){
  స్థిరరాశి అక్షరం *p = z;
  పూర్ణాంకం n;

  /* Skip past any whitespace */
  అయితే( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  అయితే( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
స్థిర అక్షరం *sqllog_కనుగొను_File(స్థిరరాశి అక్షరం *zFile){
  అక్షరం *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  ఒకవేళ( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    తిరిగి 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  అయితే( feof(fd)==0 ){
    అక్షరం z_పంక్తి[SQLLOG_NAMESZ*2+5];
    ఒకవేళ( fgets(z_పంక్తి, పరిమాణం(z_పంక్తి), fd) ){
      పూర్ణాంకం n;
      అక్షరం *z;

      z_పంక్తి[పరిమాణం(z_పంక్తి)-1] = '\0';
      z = z_పంక్తి;
      అయితే( *z>='0' && *z<='9' ) z++;
      అయితే( *z==' ' ) z++;

      n = అక్షరశ్రేణి_పొడవు(z);
      అయితే( n>0 && sqllog_isspace(z[n-1]) ) n--;

      ఒకవేళ( n==అక్షరశ్రేణి_పొడవు(zFile) && 0==memcmp(zFile, z, n) ){
        అక్షరం zBuf[16];
        memset(zBuf, 0, పరిమాణం(zBuf));
        z = z_పంక్తి;
        అయితే( *z>='0' && *z<='9' ){
          zBuf[z-z_పంక్తి] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.z_ఉపసర్గ, zBuf);
        ఆపు;
      }
    }
  }

  ఒకవేళ( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  తిరిగి zRet;
}

స్థిర పూర్ణాంకం sqllog_కనుగొను_Attached(
  sqlite3 *db,                    /* Database connection */
  స్థిరరాశి అక్షరం *z_శోధన,            /* Name to search for (or NULL) */
  అక్షరం *zName,                    /* OUT: Name of attached database */
  అక్షరం *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  పూర్ణాంకం rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  ప్రతిజ్ఞా( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    అయితే( SQLITE_అడ్డువరుస==sqlite3_అడుగు(pStmt) ){
      స్థిరరాశి అక్షరం *zVal1; పూర్ణాంకం nVal1;
      స్థిరరాశి అక్షరం *zVal2; పూర్ణాంకం nVal2;

      zVal1 = (స్థిరరాశి అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(pStmt, 1);
      nVal1 = sqlite3_స్తంభం_బైట్లు(pStmt, 1);
      ఒకవేళ( zName ){
        జ్ఞాపకం_నకలు(zName, zVal1, nVal1+1);
      }

      zVal2 = (స్థిరరాశి అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(pStmt, 2);
      nVal2 = sqlite3_స్తంభం_బైట్లు(pStmt, 2);
      జ్ఞాపకం_నకలు(zFile, zVal2, nVal2+1);

      ఒకవేళ( z_శోధన && అక్షరశ్రేణి_పొడవు(z_శోధన)==nVal1 
       && 0==sqlite3_strnicmp(z_శోధన, zVal1, nVal1)
      ){
        ఆపు;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  ఒకవేళ( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  తిరిగి rc;
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
స్థిర శూన్య sqllogCopydb(నిర్మాణం SLConn *p, స్థిరరాశి అక్షరం *z_శోధన, పూర్ణాంకం bLog){
  అక్షరం zName[SQLLOG_NAMESZ];      /* Attached database name */
  అక్షరం zFile[SQLLOG_NAMESZ];      /* Database file name */
  అక్షరం *zFree;
  అక్షరం *zInit = 0;
  పూర్ణాంకం rc;

  rc = sqllog_కనుగొను_Attached(p->db, z_శోధన, zName, zFile);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి;

  ఒకవేళ( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }లేకపోతే{
    ఒకవేళ( sqllogglobal.bReuse ){
      zInit = sqllog_కనుగొను_File(zFile);
    }లేకపోతే{
      zInit = 0;
    }
    ఒకవేళ( zInit==0 ){
      పూర్ణాంకం rc;
      sqlite3 *copy = 0;
      పూర్ణాంకం iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.i_తదుపరి_Db++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.z_ఉపసర్గ, iDb);

      /* Create the backup */
      ప్రతిజ్ఞా( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      ఒకవేళ( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        ఒకవేళ( pBak ){
          sqlite3_backup_అడుగు(pBak, -1);
          rc = sqlite3_backup_ముగింపు(pBak);
        }లేకపోతే{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      ఒకవేళ( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        ఒకవేళ( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }లేకపోతే{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  ఒకవేళ( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }లేకపోతే{
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
స్థిర శూన్య sqllogOpenlog(నిర్మాణం SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  ఒకవేళ( p->fd==0 ){
    అక్షరం *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    ఒకవేళ( sqllogglobal.z_ఉపసర్గ[0]==0 ){
      FILE *fd;
      అక్షరం *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      ఒకవేళ( zVar==0 || అక్షరశ్రేణి_పొడవు(zVar)+10>=(పరిమాణం(sqllogglobal.z_ఉపసర్గ)) ) తిరిగి;
      sqlite3_snprintf(పరిమాణం(sqllogglobal.z_ఉపసర్గ), sqllogglobal.z_ఉపసర్గ,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(పరిమాణం(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.z_ఉపసర్గ);
      ఒకవేళ( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      ఒకవేళ( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.z_ఉపసర్గ, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    ఒకవేళ( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
స్థిర శూన్య testSqllogStmt(నిర్మాణం SLConn *p, స్థిరరాశి అక్షరం *zSql){
  స్థిరరాశి అక్షరం *z_మొదటి;             /* Pointer to first token in zSql */
  పూర్ణాంకం n_మొదటి;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &z_మొదటి, &n_మొదటి);
  ఒకవేళ( n_మొదటి!=6 || 0!=sqlite3_strnicmp("ATTACH", z_మొదటి, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }లేకపోతే{
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
స్థిర పూర్ణాంకం sqllogTraceDb(sqlite3 *db){
  పూర్ణాంకం bRet = 1;
  ఒకవేళ( sqllogglobal.bConditional ){
    అక్షరం zFile[SQLLOG_NAMESZ];      /* Attached database name */
    పూర్ణాంకం rc = sqllog_కనుగొను_Attached(db, "main", 0, zFile);
    ఒకవేళ( rc==SQLITE_OK ){
      పూర్ణాంకం nFile = అక్షరశ్రేణి_పొడవు(zFile);
      ఒకవేళ( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }లేకపోతే{
        జ్ఞాపకం_నకలు(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  తిరిగి bRet;
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
స్థిర శూన్య testSqllog(శూన్య *pCtx, sqlite3 *db, స్థిరరాశి అక్షరం *zSql, పూర్ణాంకం eType){
  నిర్మాణం SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  ప్రతిజ్ఞా( eType==0 || eType==1 || eType==2 );
  ప్రతిజ్ఞా( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  ఒకవేళ( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    ఒకవేళ( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    ఒకవేళ( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.i_తదుపరి_Log++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      ఒకవేళ( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  లేకపోతే{

    పూర్ణాంకం i;
    కొరకు(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      ఒకవేళ( p->db==db ) ఆపు;
    }

    /* A database handle close command */
    ఒకవేళ( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      ఒకవేళ( i<sqllogglobal.nConn ){
        ఒకవేళ( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      ఒకవేళ( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }లేకపోతే ఒకవేళ( i<sqllogglobal.nConn ){
        పూర్ణాంకం n_జరుపు = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        ఒకవేళ( n_జరుపు>0 ){
          memmove(p, &p[1], n_జరుపు*పరిమాణం(నిర్మాణం SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }లేకపోతే ఒకవేళ( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      ఒకవేళ( sqllogglobal.bRec==0 ){
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
శూన్య sqlite3_init_sqllog(శూన్య){
  ఒకవేళ( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    ఒకవేళ( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      memset(&sqllogglobal, 0, పరిమాణం(sqllogglobal));
      sqllogglobal.bReuse = 1;
      ఒకవేళ( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
