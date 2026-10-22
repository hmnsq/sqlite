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
#अन्तर्भाव "sqlite3.h"
#endif
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <unistd.h>
स्थिर पूर्णांक getProcessId(शून्य){
#ifdef _WIN32
  परत (पूर्णांक)_getpid();
#अन्यथा
  परत (पूर्णांक)getpid();
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
संरचना SLConn {
  पूर्णांक isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  पूर्णांक iLog;                       /* First integer value used in file names */
  खाता *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
स्थिर संरचना SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  पूर्णांक nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  पूर्णांक bConditional;               /* Only trace if *-sqllog file is present */
  पूर्णांक bReuse;                     /* True to avoid extra copies of db files */
  अक्षर zPrefix[SQLLOG_NAMESZ];    /* Prefix for all created files */
  अक्षर zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  पूर्णांक iNextLog;                   /* Used to allocate file names */
  पूर्णांक iNextDb;                    /* Used to allocate database file names */
  पूर्णांक bRec;                       /* True if testSqllog() is called rec. */
  पूर्णांक iClock;                     /* Clock value */
  संरचना SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
स्थिर पूर्णांक sqllog_isspace(अक्षर c){
  परत (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
स्थिर शून्य sqllogTokenize(स्थिरांक अक्षर *z, स्थिरांक अक्षर **pz, पूर्णांक *pn){
  स्थिरांक अक्षर *p = z;
  पूर्णांक n;

  /* Skip past any whitespace */
  पर्यंत( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  पर्यंत( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
स्थिर अक्षर *sqllogFindFile(स्थिरांक अक्षर *zFile){
  अक्षर *zRet = 0;
  खाता *fd = 0;

  /* Open the index file for reading */
  fd = ख_खोलो(sqllogglobal.zIdx, "r");
  जर( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    परत 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  पर्यंत( ख_पूर्ण(fd)==0 ){
    अक्षर z_ओळ[SQLLOG_NAMESZ*2+5];
    जर( ख_माला_लो(z_ओळ, आकार(z_ओळ), fd) ){
      पूर्णांक n;
      अक्षर *z;

      z_ओळ[आकार(z_ओळ)-1] = '\0';
      z = z_ओळ;
      पर्यंत( *z>='0' && *z<='9' ) z++;
      पर्यंत( *z==' ' ) z++;

      n = म_माप(z);
      पर्यंत( n>0 && sqllog_isspace(z[n-1]) ) n--;

      जर( n==म_माप(zFile) && 0==स_भेद(zFile, z, n) ){
        अक्षर zBuf[16];
        स_रखो(zBuf, 0, आकार(zBuf));
        z = z_ओळ;
        पर्यंत( *z>='0' && *z<='9' ){
          zBuf[z-z_ओळ] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.zPrefix, zBuf);
        थांबा;
      }
    }
  }

  जर( ख_त्रुटि(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  ख_बंद(fd);
  परत zRet;
}

स्थिर पूर्णांक sqllogFindAttached(
  sqlite3 *db,                    /* Database connection */
  स्थिरांक अक्षर *z_शोध,            /* Name to search for (or NULL) */
  अक्षर *zName,                    /* OUT: Name of attached database */
  अक्षर *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  पूर्णांक rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  प्रतिज्ञा( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  जर( rc==SQLITE_OK ){
    पर्यंत( SQLITE_ROW==sqlite3_step(pStmt) ){
      स्थिरांक अक्षर *zVal1; पूर्णांक nVal1;
      स्थिरांक अक्षर *zVal2; पूर्णांक nVal2;

      zVal1 = (स्थिरांक अक्षर*)sqlite3_column_text(pStmt, 1);
      nVal1 = sqlite3_column_bytes(pStmt, 1);
      जर( zName ){
        स_नकल(zName, zVal1, nVal1+1);
      }

      zVal2 = (स्थिरांक अक्षर*)sqlite3_column_text(pStmt, 2);
      nVal2 = sqlite3_column_bytes(pStmt, 2);
      स_नकल(zFile, zVal2, nVal2+1);

      जर( z_शोध && म_माप(z_शोध)==nVal1 
       && 0==sqlite3_strnicmp(z_शोध, zVal1, nVal1)
      ){
        थांबा;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  जर( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  परत rc;
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
स्थिर शून्य sqllogCopydb(संरचना SLConn *p, स्थिरांक अक्षर *z_शोध, पूर्णांक bLog){
  अक्षर zName[SQLLOG_NAMESZ];      /* Attached database name */
  अक्षर zFile[SQLLOG_NAMESZ];      /* Database file name */
  अक्षर *zFree;
  अक्षर *zInit = 0;
  पूर्णांक rc;

  rc = sqllogFindAttached(p->db, z_शोध, zName, zFile);
  जर( rc!=SQLITE_OK ) परत;

  जर( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }अन्यथा{
    जर( sqllogglobal.bReuse ){
      zInit = sqllogFindFile(zFile);
    }अन्यथा{
      zInit = 0;
    }
    जर( zInit==0 ){
      पूर्णांक rc;
      sqlite3 *copy = 0;
      पूर्णांक iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.iNextDb++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.zPrefix, iDb);

      /* Create the backup */
      प्रतिज्ञा( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      जर( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        जर( pBak ){
          sqlite3_backup_step(pBak, -1);
          rc = sqlite3_backup_finish(pBak);
        }अन्यथा{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      जर( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        खाता *fd = ख_खोलो(sqllogglobal.zIdx, "a");
        जर( fd ){
          ख_लिखो(fd, "%d %s\n", iDb, zFile);
          ख_बंद(fd);
        }
      }अन्यथा{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  जर( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }अन्यथा{
    zFree = sqlite3_mprintf("-- Main database is '%q'\n", zInit);
  }
  ख_लिखो(p->fd, "%s", zFree);
  sqlite3_free(zFree);

  sqlite3_free(zInit);
}

/*
** If it is not already open, open the log file for connection *p. 
**
** The SLGlobal.mutex mutex is always held when this function is called.
*/
स्थिर शून्य sqllogOpenlog(संरचना SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  जर( p->fd==0 ){
    अक्षर *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    जर( sqllogglobal.zPrefix[0]==0 ){
      खाता *fd;
      अक्षर *zVar = दो_पर्या(ENVIRONMENT_VARIABLE1_NAME);
      जर( zVar==0 || म_माप(zVar)+10>=(आकार(sqllogglobal.zPrefix)) ) परत;
      sqlite3_snprintf(आकार(sqllogglobal.zPrefix), sqllogglobal.zPrefix,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(आकार(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.zPrefix);
      जर( दो_पर्या(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = म_से_प(दो_पर्या(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = ख_खोलो(sqllogglobal.zIdx, "w");
      जर( fd ) ख_बंद(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.zPrefix, p->iLog);
    p->fd = ख_खोलो(zLog, "w");
    sqlite3_free(zLog);
    जर( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
स्थिर शून्य testSqllogStmt(संरचना SLConn *p, स्थिरांक अक्षर *zSql){
  स्थिरांक अक्षर *zFirst;             /* Pointer to first token in zSql */
  पूर्णांक nFirst;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &zFirst, &nFirst);
  जर( nFirst!=6 || 0!=sqlite3_strnicmp("ATTACH", zFirst, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    ख_लिखो(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
  }अन्यथा{
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
स्थिर पूर्णांक sqllogTraceDb(sqlite3 *db){
  पूर्णांक bRet = 1;
  जर( sqllogglobal.bConditional ){
    अक्षर zFile[SQLLOG_NAMESZ];      /* Attached database name */
    पूर्णांक rc = sqllogFindAttached(db, "main", 0, zFile);
    जर( rc==SQLITE_OK ){
      पूर्णांक nFile = म_माप(zFile);
      जर( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }अन्यथा{
        स_नकल(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  परत bRet;
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
स्थिर शून्य testSqllog(शून्य *pCtx, sqlite3 *db, स्थिरांक अक्षर *zSql, पूर्णांक eType){
  संरचना SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  प्रतिज्ञा( eType==0 || eType==1 || eType==2 );
  प्रतिज्ञा( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  जर( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    जर( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    जर( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.iNextLog++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      जर( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  अन्यथा{

    पूर्णांक i;
    साठी(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      जर( p->db==db ) थांबा;
    }

    /* A database handle close command */
    जर( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      जर( i<sqllogglobal.nConn ){
        जर( p->fd ) ख_बंद(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      जर( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }अन्यथा जर( i<sqllogglobal.nConn ){
        पूर्णांक nShift = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        जर( nShift>0 ){
          स_हटाओ(p, &p[1], nShift*आकार(संरचना SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }अन्यथा जर( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      जर( sqllogglobal.bRec==0 ){
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
शून्य sqlite3_init_sqllog(शून्य){
  जर( दो_पर्या(ENVIRONMENT_VARIABLE1_NAME) ){
    जर( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      स_रखो(&sqllogglobal, 0, आकार(sqllogglobal));
      sqllogglobal.bReuse = 1;
      जर( दो_पर्या(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
