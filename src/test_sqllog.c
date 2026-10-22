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
#অন্তর্ভাব "sqlite3.h"
#endif
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>

#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <unistd.h>
স্থির পূর্ণসংখ্যা getProcessId(শূন্য){
#ifdef _WIN32
  ফেরত (পূর্ণসংখ্যা)_getpid();
#অন্যথা
  ফেরত (পূর্ণসংখ্যা)getpid();
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
কাঠামো SLConn {
  পূর্ণসংখ্যা isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  পূর্ণসংখ্যা iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
স্থির কাঠামো SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  পূর্ণসংখ্যা nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  পূর্ণসংখ্যা bConditional;               /* Only trace if *-sqllog file is present */
  পূর্ণসংখ্যা bReuse;                     /* True to avoid extra copies of db files */
  অক্ষর zPrefix[SQLLOG_NAMESZ];    /* Prefix for all created files */
  অক্ষর zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  পূর্ণসংখ্যা i_পরবর্তী_Log;                   /* Used to allocate file names */
  পূর্ণসংখ্যা i_পরবর্তী_Db;                    /* Used to allocate database file names */
  পূর্ণসংখ্যা bRec;                       /* True if testSqllog() is called rec. */
  পূর্ণসংখ্যা iClock;                     /* Clock value */
  কাঠামো SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
স্থির পূর্ণসংখ্যা sqllog_isspace(অক্ষর c){
  ফেরত (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
স্থির শূন্য sqllogTokenize(ধ্রুবক অক্ষর *z, ধ্রুবক অক্ষর **pz, পূর্ণসংখ্যা *pn){
  ধ্রুবক অক্ষর *p = z;
  পূর্ণসংখ্যা n;

  /* Skip past any whitespace */
  যখন( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  যখন( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
স্থির অক্ষর *sqllogFindFile(ধ্রুবক অক্ষর *zFile){
  অক্ষর *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  যদি( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    ফেরত 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  যখন( feof(fd)==0 ){
    অক্ষর z_সারি[SQLLOG_NAMESZ*2+5];
    যদি( fgets(z_সারি, আকার(z_সারি), fd) ){
      পূর্ণসংখ্যা n;
      অক্ষর *z;

      z_সারি[আকার(z_সারি)-1] = '\0';
      z = z_সারি;
      যখন( *z>='0' && *z<='9' ) z++;
      যখন( *z==' ' ) z++;

      n = strlen(z);
      যখন( n>0 && sqllog_isspace(z[n-1]) ) n--;

      যদি( n==strlen(zFile) && 0==memcmp(zFile, z, n) ){
        অক্ষর zBuf[16];
        memset(zBuf, 0, আকার(zBuf));
        z = z_সারি;
        যখন( *z>='0' && *z<='9' ){
          zBuf[z-z_সারি] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.zPrefix, zBuf);
        বিরতি;
      }
    }
  }

  যদি( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  ফেরত zRet;
}

স্থির পূর্ণসংখ্যা sqllogFindAttached(
  sqlite3 *db,                    /* Database connection */
  ধ্রুবক অক্ষর *z_অনুসন্ধান,            /* Name to search for (or NULL) */
  অক্ষর *zName,                    /* OUT: Name of attached database */
  অক্ষর *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  পূর্ণসংখ্যা rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  প্রতিজ্ঞা( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  যদি( rc==SQLITE_OK ){
    যখন( SQLITE_ROW==sqlite3_step(pStmt) ){
      ধ্রুবক অক্ষর *zVal1; পূর্ণসংখ্যা nVal1;
      ধ্রুবক অক্ষর *zVal2; পূর্ণসংখ্যা nVal2;

      zVal1 = (ধ্রুবক অক্ষর*)sqlite3_column_text(pStmt, 1);
      nVal1 = sqlite3_column_bytes(pStmt, 1);
      যদি( zName ){
        memcpy(zName, zVal1, nVal1+1);
      }

      zVal2 = (ধ্রুবক অক্ষর*)sqlite3_column_text(pStmt, 2);
      nVal2 = sqlite3_column_bytes(pStmt, 2);
      memcpy(zFile, zVal2, nVal2+1);

      যদি( z_অনুসন্ধান && strlen(z_অনুসন্ধান)==nVal1 
       && 0==sqlite3_strnicmp(z_অনুসন্ধান, zVal1, nVal1)
      ){
        বিরতি;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  যদি( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  ফেরত rc;
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
স্থির শূন্য sqllogCopydb(কাঠামো SLConn *p, ধ্রুবক অক্ষর *z_অনুসন্ধান, পূর্ণসংখ্যা bLog){
  অক্ষর zName[SQLLOG_NAMESZ];      /* Attached database name */
  অক্ষর zFile[SQLLOG_NAMESZ];      /* Database file name */
  অক্ষর *zFree;
  অক্ষর *zInit = 0;
  পূর্ণসংখ্যা rc;

  rc = sqllogFindAttached(p->db, z_অনুসন্ধান, zName, zFile);
  যদি( rc!=SQLITE_OK ) ফেরত;

  যদি( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }অন্যথা{
    যদি( sqllogglobal.bReuse ){
      zInit = sqllogFindFile(zFile);
    }অন্যথা{
      zInit = 0;
    }
    যদি( zInit==0 ){
      পূর্ণসংখ্যা rc;
      sqlite3 *copy = 0;
      পূর্ণসংখ্যা iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.i_পরবর্তী_Db++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.zPrefix, iDb);

      /* Create the backup */
      প্রতিজ্ঞা( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      যদি( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        যদি( pBak ){
          sqlite3_backup_step(pBak, -1);
          rc = sqlite3_backup_finish(pBak);
        }অন্যথা{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      যদি( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        যদি( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }অন্যথা{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  যদি( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }অন্যথা{
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
স্থির শূন্য sqllogOpenlog(কাঠামো SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  যদি( p->fd==0 ){
    অক্ষর *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    যদি( sqllogglobal.zPrefix[0]==0 ){
      FILE *fd;
      অক্ষর *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      যদি( zVar==0 || strlen(zVar)+10>=(আকার(sqllogglobal.zPrefix)) ) ফেরত;
      sqlite3_snprintf(আকার(sqllogglobal.zPrefix), sqllogglobal.zPrefix,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(আকার(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.zPrefix);
      যদি( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      যদি( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.zPrefix, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    যদি( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
স্থির শূন্য testSqllogStmt(কাঠামো SLConn *p, ধ্রুবক অক্ষর *zSql){
  ধ্রুবক অক্ষর *zFirst;             /* Pointer to first token in zSql */
  পূর্ণসংখ্যা nFirst;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &zFirst, &nFirst);
  যদি( nFirst!=6 || 0!=sqlite3_strnicmp("ATTACH", zFirst, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }অন্যথা{
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
স্থির পূর্ণসংখ্যা sqllogTraceDb(sqlite3 *db){
  পূর্ণসংখ্যা bRet = 1;
  যদি( sqllogglobal.bConditional ){
    অক্ষর zFile[SQLLOG_NAMESZ];      /* Attached database name */
    পূর্ণসংখ্যা rc = sqllogFindAttached(db, "main", 0, zFile);
    যদি( rc==SQLITE_OK ){
      পূর্ণসংখ্যা nFile = strlen(zFile);
      যদি( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }অন্যথা{
        memcpy(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  ফেরত bRet;
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
স্থির শূন্য testSqllog(শূন্য *pCtx, sqlite3 *db, ধ্রুবক অক্ষর *zSql, পূর্ণসংখ্যা eType){
  কাঠামো SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  প্রতিজ্ঞা( eType==0 || eType==1 || eType==2 );
  প্রতিজ্ঞা( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  যদি( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    যদি( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    যদি( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.i_পরবর্তী_Log++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      যদি( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  অন্যথা{

    পূর্ণসংখ্যা i;
    জন্য(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      যদি( p->db==db ) বিরতি;
    }

    /* A database handle close command */
    যদি( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      যদি( i<sqllogglobal.nConn ){
        যদি( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      যদি( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }অন্যথা যদি( i<sqllogglobal.nConn ){
        পূর্ণসংখ্যা nShift = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        যদি( nShift>0 ){
          memmove(p, &p[1], nShift*আকার(কাঠামো SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }অন্যথা যদি( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      যদি( sqllogglobal.bRec==0 ){
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
শূন্য sqlite3_init_sqllog(শূন্য){
  যদি( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    যদি( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      memset(&sqllogglobal, 0, আকার(sqllogglobal));
      sqllogglobal.bReuse = 1;
      যদি( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
