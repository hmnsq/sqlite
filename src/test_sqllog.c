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

#यदि_अपरिभाषितम् _SQLITE3_H_
#अन्तर्भाव "sqlite3.h"
#यदि_अन्तः
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <unistd.h>
स्थिर पूर्णाङ्क getProcessId(शून्य){
#यदि_परिभाषितम् _WIN32
  प्रत्यागम (पूर्णाङ्क)_getpid();
#अन्यथा
  प्रत्यागम (पूर्णाङ्क)getpid();
#यदि_अन्तः
}

/* Names of environment variables to be used */
#परिभाषय ENVIRONMENT_VARIABLE1_NAME "SQLITE_SQLLOG_DIR"
#परिभाषय ENVIRONMENT_VARIABLE2_NAME "SQLITE_SQLLOG_REUSE_FILES"
#परिभाषय ENVIRONMENT_VARIABLE3_NAME "SQLITE_SQLLOG_CONDITIONAL"

/* Assume that all database and database file names are shorted than this. */
#परिभाषय SQLLOG_NAMESZ 512

/* Maximum number of simultaneous database connections the process may
** open (if any more are opened an error is logged using sqlite3_log()
** and processing is halted).
*/
#परिभाषय MAX_CONNECTIONS 256

/* There is one instance of this object for each SQLite database connection
** that is being logged.
*/
संरचना SLConn {
  पूर्णाङ्क isErr;                      /* True if an error has occurred */
  sqlite3 *db;                    /* Connection handle */
  पूर्णाङ्क iLog;                       /* First integer value used in file names */
  FILE *fd;                       /* File descriptor for log file */
};

/* This object is a singleton that keeps track of all data loggers.
*/
स्थिर संरचना SLGlobal {
  /* Protected by MUTEX_STATIC_MAIN */
  sqlite3_mutex *mutex;           /* Recursive mutex */
  पूर्णाङ्क nConn;                      /* Size of aConn[] array */

  /* Protected by SLGlobal.mutex */
  पूर्णाङ्क bConditional;               /* Only trace if *-sqllog file is present */
  पूर्णाङ्क bReuse;                     /* True to avoid extra copies of db files */
  वर्ण z_उपसर्गः[SQLLOG_NAMESZ];    /* Prefix for all created files */
  वर्ण zIdx[SQLLOG_NAMESZ];       /* Full path to *.idx file */
  पूर्णाङ्क i_अग्र_Log;                   /* Used to allocate file names */
  पूर्णाङ्क i_अग्र_Db;                    /* Used to allocate database file names */
  पूर्णाङ्क bRec;                       /* True if testSqllog() is called rec. */
  पूर्णाङ्क iClock;                     /* Clock value */
  संरचना SLConn aConn[MAX_CONNECTIONS];
} sqllogglobal;

/*
** Return true if c is an ASCII whitespace character.
*/
स्थिर पूर्णाङ्क sqllog_isspace(वर्ण c){
  प्रत्यागम (c==' ' || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r');
}

/*
** The first argument points to a nul-terminated string containing an SQL
** command. Before returning, this function sets *pz to point to the start
** of the first token in this command, and *pn to the number of bytes in 
** the token. This is used to check if the SQL command is an "ATTACH" or 
** not.
*/
स्थिर शून्य sqllogTokenize(ध्रुव वर्ण *z, ध्रुव वर्ण **pz, पूर्णाङ्क *pn){
  ध्रुव वर्ण *p = z;
  पूर्णाङ्क n;

  /* Skip past any whitespace */
  यावत्( sqllog_isspace(*p) ){
    p++;
  }

  /* Figure out how long the first token is */
  *pz = p;
  n = 0;
  यावत्( (p[n]>='a' && p[n]<='z') || (p[n]>='A' && p[n]<='Z') ) n++;
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
स्थिर वर्ण *sqllogFindFile(ध्रुव वर्ण *zFile){
  वर्ण *zRet = 0;
  FILE *fd = 0;

  /* Open the index file for reading */
  fd = fopen(sqllogglobal.zIdx, "r");
  यदि( fd==0 ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error in fopen()");
    प्रत्यागम 0;
  }

  /* Loop through each entry in the index file. If zFile is not NULL and the
  ** entry is a match, then set zRet to point to the filename of the existing
  ** copy and break out of the loop.  */
  यावत्( feof(fd)==0 ){
    वर्ण z_पङ्क्तिः[SQLLOG_NAMESZ*2+5];
    यदि( fgets(z_पङ्क्तिः, परिमाण(z_पङ्क्तिः), fd) ){
      पूर्णाङ्क n;
      वर्ण *z;

      z_पङ्क्तिः[परिमाण(z_पङ्क्तिः)-1] = '\0';
      z = z_पङ्क्तिः;
      यावत्( *z>='0' && *z<='9' ) z++;
      यावत्( *z==' ' ) z++;

      n = वाक्यदैर्घ्यम्(z);
      यावत्( n>0 && sqllog_isspace(z[n-1]) ) n--;

      यदि( n==वाक्यदैर्घ्यम्(zFile) && 0==memcmp(zFile, z, n) ){
        वर्ण zBuf[16];
        स्मृतिं_पूरय(zBuf, 0, परिमाण(zBuf));
        z = z_पङ्क्तिः;
        यावत्( *z>='0' && *z<='9' ){
          zBuf[z-z_पङ्क्तिः] = *z;
          z++;
        }
        zRet = sqlite3_mprintf("%s_%s.db", sqllogglobal.z_उपसर्गः, zBuf);
        भङ्ग;
      }
    }
  }

  यदि( ferror(fd) ){
    sqlite3_log(SQLITE_IOERR, "sqllogFindFile(): error reading index file");
  }

  fclose(fd);
  प्रत्यागम zRet;
}

स्थिर पूर्णाङ्क sqllogFindAttached(
  sqlite3 *db,                    /* Database connection */
  ध्रुव वर्ण *z_अन्वेषणम्,            /* Name to search for (or NULL) */
  वर्ण *zName,                    /* OUT: Name of attached database */
  वर्ण *zFile                     /* OUT: Name of attached file */
){
  sqlite3_stmt *pStmt;
  पूर्णाङ्क rc;

  /* The "PRAGMA database_list" command returns a list of databases in the
  ** order that they were attached. So a newly attached database is 
  ** described by the last row returned.  */
  प्रतिज्ञा( sqllogglobal.bRec==0 );
  sqllogglobal.bRec = 1;
  rc = sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &pStmt, 0);
  यदि( rc==SQLITE_OK ){
    यावत्( SQLITE_ROW==sqlite3_चरणः(pStmt) ){
      ध्रुव वर्ण *zVal1; पूर्णाङ्क nVal1;
      ध्रुव वर्ण *zVal2; पूर्णाङ्क nVal2;

      zVal1 = (ध्रुव वर्ण*)sqlite3_column_text(pStmt, 1);
      nVal1 = sqlite3_column_अष्टकानि(pStmt, 1);
      यदि( zName ){
        स्मृतिं_प्रतिलिख(zName, zVal1, nVal1+1);
      }

      zVal2 = (ध्रुव वर्ण*)sqlite3_column_text(pStmt, 2);
      nVal2 = sqlite3_column_अष्टकानि(pStmt, 2);
      स्मृतिं_प्रतिलिख(zFile, zVal2, nVal2+1);

      यदि( z_अन्वेषणम् && वाक्यदैर्घ्यम्(z_अन्वेषणम्)==nVal1 
       && 0==sqlite3_strnicmp(z_अन्वेषणम्, zVal1, nVal1)
      ){
        भङ्ग;
      }
    }
    rc = sqlite3_finalize(pStmt);
  }
  sqllogglobal.bRec = 0;

  यदि( rc!=SQLITE_OK ){
    sqlite3_log(rc, "sqllogFindAttached(): error in \"PRAGMA database_list\"");
  }
  प्रत्यागम rc;
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
स्थिर शून्य sqllogCopydb(संरचना SLConn *p, ध्रुव वर्ण *z_अन्वेषणम्, पूर्णाङ्क bLog){
  वर्ण zName[SQLLOG_NAMESZ];      /* Attached database name */
  वर्ण zFile[SQLLOG_NAMESZ];      /* Database file name */
  वर्ण *zFree;
  वर्ण *zInit = 0;
  पूर्णाङ्क rc;

  rc = sqllogFindAttached(p->db, z_अन्वेषणम्, zName, zFile);
  यदि( rc!=SQLITE_OK ) प्रत्यागम;

  यदि( zFile[0]=='\0' ){
    zInit = sqlite3_mprintf("");
  }अन्यथा{
    यदि( sqllogglobal.bReuse ){
      zInit = sqllogFindFile(zFile);
    }अन्यथा{
      zInit = 0;
    }
    यदि( zInit==0 ){
      पूर्णाङ्क rc;
      sqlite3 *copy = 0;
      पूर्णाङ्क iDb;

      /* Generate a file-name to use for the copy of this database */
      iDb = sqllogglobal.i_अग्र_Db++;
      zInit = sqlite3_mprintf("%s_%02d.db", sqllogglobal.z_उपसर्गः, iDb);

      /* Create the backup */
      प्रतिज्ञा( sqllogglobal.bRec==0 );
      sqllogglobal.bRec = 1;
      rc = sqlite3_open(zInit, &copy);
      यदि( rc==SQLITE_OK ){
        sqlite3_backup *pBak;
        sqlite3_exec(copy, "PRAGMA synchronous = 0", 0, 0, 0);
        pBak = sqlite3_backup_init(copy, "main", p->db, zName);
        यदि( pBak ){
          sqlite3_backup_चरणः(pBak, -1);
          rc = sqlite3_backup_finish(pBak);
        }अन्यथा{
          rc = sqlite3_errcode(copy);
        }
        sqlite3_close(copy);
      }
      sqllogglobal.bRec = 0;

      यदि( rc==SQLITE_OK ){
        /* Write an entry into the database index file */
        FILE *fd = fopen(sqllogglobal.zIdx, "a");
        यदि( fd ){
          fprintf(fd, "%d %s\n", iDb, zFile);
          fclose(fd);
        }
      }अन्यथा{
        sqlite3_log(rc, "sqllogCopydb(): error backing up database");
      }
    }
  }

  यदि( bLog ){
    zFree = sqlite3_mprintf("ATTACH '%q' AS '%q'; -- clock=%d\n", 
        zInit, zName, sqllogglobal.iClock++
    );
  }अन्यथा{
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
स्थिर शून्य sqllogOpenlog(संरचना SLConn *p){
  /* If the log file has not yet been opened, open it now. */
  यदि( p->fd==0 ){
    वर्ण *zLog;

    /* If it is still NULL, have global.zPrefix point to a copy of 
    ** environment variable $ENVIRONMENT_VARIABLE1_NAME.  */
    यदि( sqllogglobal.z_उपसर्गः[0]==0 ){
      FILE *fd;
      वर्ण *zVar = getenv(ENVIRONMENT_VARIABLE1_NAME);
      यदि( zVar==0 || वाक्यदैर्घ्यम्(zVar)+10>=(परिमाण(sqllogglobal.z_उपसर्गः)) ) प्रत्यागम;
      sqlite3_snprintf(परिमाण(sqllogglobal.z_उपसर्गः), sqllogglobal.z_उपसर्गः,
                        "%s/sqllog_%05d", zVar, getProcessId());
      sqlite3_snprintf(परिमाण(sqllogglobal.zIdx), sqllogglobal.zIdx,
                        "%s.idx", sqllogglobal.z_उपसर्गः);
      यदि( getenv(ENVIRONMENT_VARIABLE2_NAME) ){
        sqllogglobal.bReuse = atoi(getenv(ENVIRONMENT_VARIABLE2_NAME));
      }
      fd = fopen(sqllogglobal.zIdx, "w");
      यदि( fd ) fclose(fd);
    }

    /* Open the log file */
    zLog = sqlite3_mprintf("%s_%05d.sql", sqllogglobal.z_उपसर्गः, p->iLog);
    p->fd = fopen(zLog, "w");
    sqlite3_free(zLog);
    यदि( p->fd==0 ){
      sqlite3_log(SQLITE_IOERR, "sqllogOpenlog(): Failed to open log file");
    }
  }
}

/*
** This function is called if the SQLLOG callback is invoked to report
** execution of an SQL statement. Parameter p is the connection the statement
** was executed by and parameter zSql is the text of the statement itself.
*/
स्थिर शून्य testSqllogStmt(संरचना SLConn *p, ध्रुव वर्ण *zSql){
  ध्रुव वर्ण *zFirst;             /* Pointer to first token in zSql */
  पूर्णाङ्क nFirst;                     /* Size of token zFirst in bytes */

  sqllogTokenize(zSql, &zFirst, &nFirst);
  यदि( nFirst!=6 || 0!=sqlite3_strnicmp("ATTACH", zFirst, 6) ){
    /* Not an ATTACH statement. Write this directly to the log. */
    fprintf(p->fd, "%s; -- clock=%d\n", zSql, sqllogglobal.iClock++);
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
स्थिर पूर्णाङ्क sqllogTraceDb(sqlite3 *db){
  पूर्णाङ्क bRet = 1;
  यदि( sqllogglobal.bConditional ){
    वर्ण zFile[SQLLOG_NAMESZ];      /* Attached database name */
    पूर्णाङ्क rc = sqllogFindAttached(db, "main", 0, zFile);
    यदि( rc==SQLITE_OK ){
      पूर्णाङ्क nFile = वाक्यदैर्घ्यम्(zFile);
      यदि( (SQLLOG_NAMESZ-nFile)<8 ){
        sqlite3_log(SQLITE_IOERR, 
            "sqllogTraceDb(): database name too long (%d bytes)", nFile
        );
        bRet = 0;
      }अन्यथा{
        स्मृतिं_प्रतिलिख(&zFile[nFile], "-sqllog", 8);
        bRet = !access(zFile, F_OK);
      }
    }
  }
  प्रत्यागम bRet;
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
स्थिर शून्य testSqllog(शून्य *pCtx, sqlite3 *db, ध्रुव वर्ण *zSql, पूर्णाङ्क eType){
  संरचना SLConn *p = 0;
  sqlite3_mutex *mainmtx = sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_MAIN);

  प्रतिज्ञा( eType==0 || eType==1 || eType==2 );
  प्रतिज्ञा( (eType==2)==(zSql==0) );

  /* This is a database open command. */
  यदि( eType==0 ){
    sqlite3_mutex_enter(mainmtx);
    यदि( sqllogglobal.mutex==0 ){
      sqllogglobal.mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_RECURSIVE);
    }
    sqlite3_mutex_leave(mainmtx);

    sqlite3_mutex_enter(sqllogglobal.mutex);
    यदि( sqllogglobal.bRec==0 && sqllogTraceDb(db) ){

      sqlite3_mutex_enter(mainmtx);
      p = &sqllogglobal.aConn[sqllogglobal.nConn++];
      p->fd = 0;
      p->db = db;
      p->iLog = sqllogglobal.i_अग्र_Log++;
      sqlite3_mutex_leave(mainmtx);

      /* Open the log and take a copy of the main database file */
      sqllogOpenlog(p);
      यदि( p->fd ) sqllogCopydb(p, "main", 0);
    }
    sqlite3_mutex_leave(sqllogglobal.mutex);
  }

  अन्यथा{

    पूर्णाङ्क i;
    प्रति(i=0; i<sqllogglobal.nConn; i++){
      p = &sqllogglobal.aConn[i];
      यदि( p->db==db ) भङ्ग;
    }

    /* A database handle close command */
    यदि( eType==2 ){
      sqlite3_mutex_enter(mainmtx);
      यदि( i<sqllogglobal.nConn ){
        यदि( p->fd ) fclose(p->fd);
        p->db = 0;
        p->fd = 0;
        sqllogglobal.nConn--;
      }

      यदि( sqllogglobal.nConn==0 ){
        sqlite3_mutex_free(sqllogglobal.mutex);
        sqllogglobal.mutex = 0;
      }अन्यथा यदि( i<sqllogglobal.nConn ){
        पूर्णाङ्क n_सरणम् = &sqllogglobal.aConn[sqllogglobal.nConn] - p;
        यदि( n_सरणम्>0 ){
          memmove(p, &p[1], n_सरणम्*परिमाण(संरचना SLConn));
        }
      }
      sqlite3_mutex_leave(mainmtx);

    /* An ordinary SQL command. */
    }अन्यथा यदि( i<sqllogglobal.nConn && p->fd ){
      sqlite3_mutex_enter(sqllogglobal.mutex);
      यदि( sqllogglobal.bRec==0 ){
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
  यदि( getenv(ENVIRONMENT_VARIABLE1_NAME) ){
    यदि( SQLITE_OK==sqlite3_config(SQLITE_CONFIG_SQLLOG, testSqllog, 0) ){
      स्मृतिं_पूरय(&sqllogglobal, 0, परिमाण(sqllogglobal));
      sqllogglobal.bReuse = 1;
      यदि( getenv(ENVIRONMENT_VARIABLE3_NAME) ){
        sqllogglobal.bConditional = 1;
      }
    }
  }
}
