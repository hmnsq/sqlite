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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _SQLITE3_H_
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getProcessId(ಶೂನ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)_getpid();
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)getpid();
#ಆದರೆ_ಅಂತ್ಯ
}

/* Names of environment variables to be used */
#ವ್ಯಾಖ್ಯಾನಿಸು ENVIRONMENT_VARIABLE1_NAME "SQLITE_SQLLOG_DIR"
#ವ್ಯಾಖ್ಯಾನಿಸು ENVIRONMENT_VARIABLE2_NAME "SQLITE_SQLLOG_REUSE_FILES"
#ವ್ಯಾಖ್ಯಾನಿಸು ENVIRONMENT_VARIABLE3_NAME "SQLITE_SQLLOG_CONDITIONAL"

/* Assume that all database and database file names are shorted than this. */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLLOG_NAMESZ 512

/* Maximum number of simultaneous database connections the process may
** open (if any more are opened an error is logged using sqlite3_log()
** and processing is halted).
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_CONNECTIONS 256

/* There is one instance of this object for each SQLite database connection
** that is being logged.
*/
ರಚನೆ SLConn {
  ಪೂರ್ಣಾಂಕ isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  ಪೂರ್ಣಾಂಕ iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
ಸ್ಥಿರ ರಚನೆ SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  ಪೂರ್ಣಾಂಕ nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  ಪೂರ್ಣಾಂಕ bConditional;               /* Only trace if *-sqllog file is present */
  ಪೂರ್ಣಾಂಕ bReuse;                     /* True to avoid extra copies of db files */
  ಅಕ್ಷರ z_ಪೂರ್ವಪ್ರತ್ಯಯ[SQLLOG_NAMESZ];    /* Prefix for all created files */
  ಅಕ್ಷರ zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  ಪೂರ್ಣಾಂಕ i_ಮುಂದಿನ_Log;                   /* Used to allocate file names */
  ಪೂರ್ಣಾಂಕ i_ಮುಂದಿನ_Db;                    /* Used to allocate database file names */
  ಪೂರ್ಣಾಂಕ bRec;                       /* True if testSqllog() is called rec. */
  ಪೂರ್ಣಾಂಕ iClock;                     /* Clock value */
  ರಚನೆ SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqllog_isspace(ಅಕ್ಷರ c){
  ಹಿಂತಿರುಗಿ (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqllogTokenize(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz, ಪೂರ್ಣಾಂಕ *pn){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = z;
  ಪೂರ್ಣಾಂಕ n;

  /* Skip past any whitespace */
  ಆಗ( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  ಆಗ( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
ಸ್ಥಿರ ಅಕ್ಷರ *sqllog_ಕಂಡುಹಿಡಿ_File(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile){
  ಅಕ್ಷರ *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  ಆದರೆ( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    ಹಿಂತಿರುಗಿ 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  ಆಗ( feof(fd)==0 ){
    ಅಕ್ಷರ z_ಸಾಲು[SQLLOG_NAMESZ*2+5];
    ಆದರೆ( fgets(z_ಸಾಲು, ಗಾತ್ರ(z_ಸಾಲು), fd) ){
      ಪೂರ್ಣಾಂಕ n;
      ಅಕ್ಷರ *z;

      z_ಸಾಲು[ಗಾತ್ರ(z_ಸಾಲು)-1] = '\0';
      z = z_ಸಾಲು;
      ಆಗ( *z>='0' && *z<='9' ) z++;
      ಆಗ( *z==' ' ) z++;

      n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z);
      ಆಗ( n>0 && sqllog_isspace(z[n-1]) ) n--;

      ಆದರೆ( n==ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zFile) && 0==memcmp(zFile, z, n) ){
        ಅಕ್ಷರ zBuf[16];
        memset(zBuf, 0, ಗಾತ್ರ(zBuf));
        z = z_ಸಾಲು;
        ಆಗ( *z>='0' && *z<='9' ){
          zBuf[z-z_ಸಾಲು] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.z_ಪೂರ್ವಪ್ರತ್ಯಯ, zBuf);
        ನಿಲ್ಲಿಸು;
      }
    }
  }

  ಆದರೆ( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  ಹಿಂತಿರುಗಿ zRet;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqllog_ಕಂಡುಹಿಡಿ_Attached(
  sqlite3 *db,                    /* Database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಹುಡುಕು,            /* Name to search for (or NULL) */
  ಅಕ್ಷರ *zName,                    /* OUT: Name of attached database */
  ಅಕ್ಷರ *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  ಪೂರ್ಣಾಂಕ rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  ಪ್ರತಿಜ್ಞಾ( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    ಆಗ( SQLITE_ಅಡ್ಡಸಾಲು==sqlite3_ಹೆಜ್ಜೆ(pStmt) ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVal1; ಪೂರ್ಣಾಂಕ nVal1;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVal2; ಪೂರ್ಣಾಂಕ nVal2;

      zVal1 = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, 1);
      nVal1 = sqlite3_ಸ್ತಂಭ_ಬೈಟುಗಳು(pStmt, 1);
      ಆದರೆ( zName ){
        ಸ್ಮರಣೆ_ನಕಲು(zName, zVal1, nVal1+1);
      }

      zVal2 = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, 2);
      nVal2 = sqlite3_ಸ್ತಂಭ_ಬೈಟುಗಳು(pStmt, 2);
      ಸ್ಮರಣೆ_ನಕಲು(zFile, zVal2, nVal2+1);

      ಆದರೆ( z_ಹುಡುಕು && ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಹುಡುಕು)==nVal1 
       && 0==sqlite3_strnicmp(z_ಹುಡುಕು, zVal1, nVal1)
      ){
        ನಿಲ್ಲಿಸು;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  ಆದರೆ( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಶೂನ್ಯ sqllogCopydb(ರಚನೆ SLConn *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಹುಡುಕು, ಪೂರ್ಣಾಂಕ bLog){
  ಅಕ್ಷರ zName[SQLLOG_NAMESZ];      /* Attached database name */
  ಅಕ್ಷರ zFile[SQLLOG_NAMESZ];      /* Database file name */
  ಅಕ್ಷರ *zFree;
  ಅಕ್ಷರ *zInit = 0;
  ಪೂರ್ಣಾಂಕ rc;

  rc = sqllog_ಕಂಡುಹಿಡಿ_Attached(p->db, z_ಹುಡುಕು, zName, zFile);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ;

  ಆದರೆ( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( sqllogglobal.bReuse ){
      zInit = sqllog_ಕಂಡುಹಿಡಿ_File(zFile);
    }ಇಲ್ಲದಿದ್ದರೆ{
      zInit = 0;
    }
    ಆದರೆ( zInit==0 ){
      ಪೂರ್ಣಾಂಕ rc;
      sqlite3 *copy = 0;
      ಪೂರ್ಣಾಂಕ iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.i_ಮುಂದಿನ_Db++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.z_ಪೂರ್ವಪ್ರತ್ಯಯ, iDb);

      /* Create the backup */
      ಪ್ರತಿಜ್ಞಾ( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      ಆದರೆ( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        ಆದರೆ( pBak ){
          sqlite3_backup_ಹೆಜ್ಜೆ(pBak, -1);
          rc = sqlite3_backup_ಮುಕ್ತಾಯ(pBak);
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      ಆದರೆ( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        ಆದರೆ( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  ಆದರೆ( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }ಇಲ್ಲದಿದ್ದರೆ{
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
ಸ್ಥಿರ ಶೂನ್ಯ sqllogOpenlog(ರಚನೆ SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  ಆದರೆ( p->fd==0 ){
    ಅಕ್ಷರ *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    ಆದರೆ( sqllogglobal.z_ಪೂರ್ವಪ್ರತ್ಯಯ[0]==0 ){
      FILE *fd;
      ಅಕ್ಷರ *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      ಆದರೆ( zVar==0 || ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zVar)+10>=(ಗಾತ್ರ(sqllogglobal.z_ಪೂರ್ವಪ್ರತ್ಯಯ)) ) ಹಿಂತಿರುಗಿ;
      sqlite3_snprintf(ಗಾತ್ರ(sqllogglobal.z_ಪೂರ್ವಪ್ರತ್ಯಯ), sqllogglobal.z_ಪೂರ್ವಪ್ರತ್ಯಯ,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(ಗಾತ್ರ(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.z_ಪೂರ್ವಪ್ರತ್ಯಯ);
      ಆದರೆ( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      ಆದರೆ( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.z_ಪೂರ್ವಪ್ರತ್ಯಯ, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    ಆದರೆ( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
ಸ್ಥಿರ ಶೂನ್ಯ testSqllogStmt(ರಚನೆ SLConn *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮೊದಲ;             /* Pointer to first token in zSql */
  ಪೂರ್ಣಾಂಕ n_ಮೊದಲ;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &z_ಮೊದಲ, &n_ಮೊದಲ);
  ಆದರೆ( n_ಮೊದಲ!=6 || 0!=sqlite3_strnicmp("ATTACH", z_ಮೊದಲ, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }ಇಲ್ಲದಿದ್ದರೆ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqllogTraceDb(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ bRet = 1;
  ಆದರೆ( sqllogglobal.bConditional ){
    ಅಕ್ಷರ zFile[SQLLOG_NAMESZ];      /* Attached database name */
    ಪೂರ್ಣಾಂಕ rc = sqllog_ಕಂಡುಹಿಡಿ_Attached(db, "main", 0, zFile);
    ಆದರೆ( rc==SQLITE_OK ){
      ಪೂರ್ಣಾಂಕ nFile = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zFile);
      ಆದರೆ( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಸ್ಮರಣೆ_ನಕಲು(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  ಹಿಂತಿರುಗಿ bRet;
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
ಸ್ಥಿರ ಶೂನ್ಯ testSqllog(ಶೂನ್ಯ *pCtx, sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql, ಪೂರ್ಣಾಂಕ eType){
  ರಚನೆ SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  ಪ್ರತಿಜ್ಞಾ( eType==0 || eType==1 || eType==2 );
  ಪ್ರತಿಜ್ಞಾ( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  ಆದರೆ( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    ಆದರೆ( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    ಆದರೆ( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.i_ಮುಂದಿನ_Log++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      ಆದರೆ( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  ಇಲ್ಲದಿದ್ದರೆ{

    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      ಆದರೆ( p->db==db ) ನಿಲ್ಲಿಸು;
    }

    /* A database handle close command */
    ಆದರೆ( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      ಆದರೆ( i<sqllogglobal.nConn ){
        ಆದರೆ( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      ಆದರೆ( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( i<sqllogglobal.nConn ){
        ಪೂರ್ಣಾಂಕ n_ಸರಿಸು = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        ಆದರೆ( n_ಸರಿಸು>0 ){
          memmove(p, &p[1], n_ಸರಿಸು*ಗಾತ್ರ(ರಚನೆ SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      ಆದರೆ( sqllogglobal.bRec==0 ){
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
  ಆದರೆ( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    ಆದರೆ( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      memset(&sqllogglobal, 0, ಗಾತ್ರ(sqllogglobal));
      sqllogglobal.bReuse = 1;
      ಆದರೆ( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
