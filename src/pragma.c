/*
** 2003 April 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used to implement the PRAGMA command.
*/
#അന്തര്ഭാവ "sqliteInt.h"

#എങ്കിൽ !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  എങ്കിൽ defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  അല്ലെങ്കിൽ
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#endif

/***************************************************************************
** The "pragma.h" include file is an automatically generated file that
** that includes the PragType_XXXX macro definitions and the aPragmaName[]
** object.  This ensures that the aPragmaName[] table is arranged in
** lexicographical order to facility a binary search of the pragma name.
** Do not edit pragma.h directly.  Edit and rerun the script in at
** ../tool/mkpragmatab.tcl. */
#അന്തര്ഭാവ "pragma.h"

/*
** When the 0x10 bit of PRAGMA optimize is set, any ANALYZE commands
** will be run with an analysis_limit set to the lessor of the value of
** the following macro or to the actual analysis_limit if it is non-zero,
** in order to prevent PRAGMA optimize from running for too long.
**
** The value of 2000 is chosen empirically so that the worst-case run-time
** for PRAGMA optimize does not exceed 100 milliseconds against a variety
** of test databases on a RaspberryPI-4 compiled using -Os and without
** -DSQLITE_DEBUG.  Of course, your mileage may vary.  For the purpose of
** this paragraph, "worst-case" means that ANALYZE ends up being
** run on every table in the database.  The worst case typically only
** happens if PRAGMA optimize is run on a database file for which ANALYZE
** has not been previously run and the 0x10000 flag is included so that
** all tables are analyzed.  The usual case for PRAGMA optimize is that
** no ANALYZE commands will be run at all, or if any ANALYZE happens it
** will be against a single table, so that expected timing for PRAGMA
** optimize on a PI-4 is more like 1 millisecond or less with the 0x10000
** flag or less than 100 microseconds without the 0x10000 flag.
**
** An analysis limit of 2000 is almost always sufficient for the query
** planner to fully characterize an index.  The additional accuracy from
** a larger analysis is not usually helpful.
*/
#ifndef SQLITE_DEFAULT_OPTIMIZE_LIMIT
# define SQLITE_DEFAULT_OPTIMIZE_LIMIT 2000
#endif

/*
** Interpret the given string as a safety level.  Return 0 for OFF,
** 1 for ON or NORMAL, 2 for FULL, and 3 for EXTRA.  Return 1 for an empty or
** unrecognized string argument.  The FULL and EXTRA option is disallowed
** if the omitFull parameter it 1.
**
** Note that the values returned are one less that the values that
** should be passed into sqlite3BtreeSetSafetyLevel().  The is done
** to support legacy SQL code.  The safety level used to be boolean
** and older scripts may have used numbers 0 for OFF and 1 for ON.
*/
സ്ഥിതം u8 getSafetyLevel(സ്ഥിരം അക്ഷരം *z, പൂർണ്ണസംഖ്യ omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  സ്ഥിതം സ്ഥിരം അക്ഷരം zText[] = "onoffalseyestruextrafull";
  സ്ഥിതം സ്ഥിരം u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  സ്ഥിതം സ്ഥിരം u8 iLength[] = {2, 2, 3,  5,    3,   4,   5,    4};
  സ്ഥിതം സ്ഥിരം u8 i_മൂല്യം[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  പൂർണ്ണസംഖ്യ i, n;
  എങ്കിൽ( sqlite3Isdigit(*z) ){
    തിരികെ sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  വേണ്ടി(i=0; i<ArraySize(iLength); i++){
    എങ്കിൽ( iLength[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || i_മൂല്യം[i]<=1)
    ){
      തിരികെ i_മൂല്യം[i];
    }
  }
  തിരികെ dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(സ്ഥിരം അക്ഷരം *z, u8 dflt){
  തിരികെ getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#എങ്കിൽ !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ getLockingMode(സ്ഥിരം അക്ഷരം *z){
  എങ്കിൽ( z ){
    എങ്കിൽ( 0==sqlite3StrICmp(z, "exclusive") ) തിരികെ PAGER_LOCKINGMODE_EXCLUSIVE;
    എങ്കിൽ( 0==sqlite3StrICmp(z, "normal") ) തിരികെ PAGER_LOCKINGMODE_NORMAL;
  }
  തിരികെ PAGER_LOCKINGMODE_QUERY;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ getAutoVacuum(സ്ഥിരം അക്ഷരം *z){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( 0==sqlite3StrICmp(z, "none") ) തിരികെ BTREE_AUTOVACUUM_NONE;
  എങ്കിൽ( 0==sqlite3StrICmp(z, "full") ) തിരികെ BTREE_AUTOVACUUM_FULL;
  എങ്കിൽ( 0==sqlite3StrICmp(z, "incremental") ) തിരികെ BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  തിരികെ (u8)((i>=0&&i<=2)?i:0);
}
#endif /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ getTempStore(സ്ഥിരം അക്ഷരം *z){
  എങ്കിൽ( z[0]>='0' && z[0]<='2' ){
    തിരികെ z[0] - '0';
  }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3StrICmp(z, "file")==0 ){
    തിരികെ 1;
  }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3StrICmp(z, "memory")==0 ){
    തിരികെ 2;
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
  }
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ invalidateTempStorage(Parse *pParse){
  sqlite3 *db = pParse->db;
  എങ്കിൽ( db->aDb[1].pBt!=0 ){
    എങ്കിൽ( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_സന്ദേശം(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      തിരികെ SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  തിരികെ SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ changeTempStorage(Parse *pParse, സ്ഥിരം അക്ഷരം *zStorageType){
  പൂർണ്ണസംഖ്യ ts = getTempStore(zStorageType);
  sqlite3 *db = pParse->db;
  എങ്കിൽ( db->temp_store==ts ) തിരികെ SQLITE_OK;
  എങ്കിൽ( invalidateTempStorage( pParse ) != SQLITE_OK ){
    തിരികെ SQLITE_ERROR;
  }
  db->temp_store = (u8)ts;
  തിരികെ SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
സ്ഥിതം ശൂന്യം setPragmaResultColumnNames(
  Vdbe *v,                     /* The query under construction */
  സ്ഥിരം PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_Set_അക്കം_Cols(v, n==0 ? 1 : n);
  എങ്കിൽ( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ i, j;
    വേണ്ടി(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
സ്ഥിതം ശൂന്യം returnSingleInt(Vdbe *v, i64 മൂല്യം){
  sqlite3VdbeAddInt64(v, 1, മൂല്യം);
  sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
}

/*
** Generate code to return a single text value.
*/
സ്ഥിതം ശൂന്യം returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  സ്ഥിരം അക്ഷരം *z_മൂല്യം      /* Value to be returned */
){
  എങ്കിൽ( z_മൂല്യം ){
    sqlite3VdbeLoadString(v, 1, (സ്ഥിരം അക്ഷരം*)z_മൂല്യം);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
സ്ഥിതം ശൂന്യം setAllPagerFlags(sqlite3 *db){
  എങ്കിൽ( db->autoCommit ){
    Db *pDb = db->aDb;
    പൂർണ്ണസംഖ്യ n = db->nDb;
    പ്രതിജ്ഞാ( SQLITE_FullFSync==PAGER_FULLFSYNC );
    പ്രതിജ്ഞാ( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    പ്രതിജ്ഞാ( SQLITE_CacheSpill==PAGER_CACHESPILL );
    പ്രതിജ്ഞാ( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    പ്രതിജ്ഞാ( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    ആകുമ്പോൾ( (n--) > 0 ){
      എങ്കിൽ( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#അല്ലെങ്കിൽ
# define setAllPagerFlags(X)  /* no-op */
#endif


/*
** Return a human-readable name for a constraint resolution action.
*/
#ifndef SQLITE_OMIT_FOREIGN_താക്കോൽ
സ്ഥിതം സ്ഥിരം അക്ഷരം *actionName(u8 action){
  സ്ഥിരം അക്ഷരം *zName;
  തിരഞ്ഞെടുപ്പ്( action ){
    അവസ്ഥ OE_SetNull:  zName = "SET NULL";        നിർത്തുക;
    അവസ്ഥ OE_SetDflt:  zName = "SET DEFAULT";     നിർത്തുക;
    അവസ്ഥ OE_Cascade:  zName = "CASCADE";         നിർത്തുക;
    അവസ്ഥ OE_Restrict: zName = "RESTRICT";        നിർത്തുക;
    സ്ഥിരസ്ഥിതി:          zName = "NO ACTION"; 
                      പ്രതിജ്ഞാ( action==OE_None ); നിർത്തുക;
  }
  തിരികെ zName;
}
#endif


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
സ്ഥിരം അക്ഷരം *sqlite3JournalModename(പൂർണ്ണസംഖ്യ eMode){
  സ്ഥിതം അക്ഷരം * സ്ഥിരം azModeName[] = {
    "delete", "persist", "off", "truncate", "memory"
#ifndef SQLITE_OMIT_WAL
     , "wal"
#endif
  };
  പ്രതിജ്ഞാ( PAGER_JOURNALMODE_DELETE==0 );
  പ്രതിജ്ഞാ( PAGER_JOURNALMODE_PERSIST==1 );
  പ്രതിജ്ഞാ( PAGER_JOURNALMODE_OFF==2 );
  പ്രതിജ്ഞാ( PAGER_JOURNALMODE_TRUNCATE==3 );
  പ്രതിജ്ഞാ( PAGER_JOURNALMODE_MEMORY==4 );
  പ്രതിജ്ഞാ( PAGER_JOURNALMODE_WAL==5 );
  പ്രതിജ്ഞാ( eMode>=0 && eMode<=ArraySize(azModeName) );

  എങ്കിൽ( eMode==ArraySize(azModeName) ) തിരികെ 0;
  തിരികെ azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
സ്ഥിതം സ്ഥിരം PragmaName *pragmaLocate(സ്ഥിരം അക്ഷരം *zName){
  പൂർണ്ണസംഖ്യ upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = ArraySize(aPragmaName)-1;
  ആകുമ്പോൾ( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    എങ്കിൽ( rc==0 ) നിർത്തുക;
    എങ്കിൽ( rc<0 ){
      upr = mid - 1;
    }അല്ലെങ്കിൽ{
      lwr = mid + 1;
    }
  }
  തിരികെ lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
സ്ഥിതം ശൂന്യം pragma_Funclist_വരി(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  പൂർണ്ണസംഖ്യ isBuiltin,         /* True if this is a built-in function */
  പൂർണ്ണസംഖ്യ showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  എങ്കിൽ( showInternFuncs ) mask = 0xffffffff;
  വേണ്ടി(; p; p=p->pNext){
    സ്ഥിരം അക്ഷരം *zType;
    സ്ഥിതം സ്ഥിരം അക്ഷരം *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    പ്രതിജ്ഞാ( SQLITE_FUNC_ENCMASK==0x3 );
    പ്രതിജ്ഞാ( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    പ്രതിജ്ഞാ( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    പ്രതിജ്ഞാ( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    എങ്കിൽ( p->xSFunc==0 ) തുടരുക;
    എങ്കിൽ( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      തുടരുക;
    }   
    എങ്കിൽ( p->x_മൂല്യം!=0 ){
      zType = "w";
    }അല്ലെങ്കിൽ എങ്കിൽ( p->xFinalize!=0 ){
      zType = "a";
    }അല്ലെങ്കിൽ{
      zType = "s";
    }
    sqlite3VdbeMultiLoad(v, 1, "sissii",
       p->zName, isBuiltin,
       zType, azEnc[p->funcFlags&SQLITE_FUNC_ENCMASK],
       p->nArg,
       (p->funcFlags & mask) ^ SQLITE_INNOCUOUS
    );
  }
}


/*
** Helper subroutine for PRAGMA integrity_check:
**
** Generate code to output a single-column result row with a value of the
** string held in register 3.  Decrement the result count in register 1
** and halt if the maximum number of result rows have been issued.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ integrityCheckResultRow(Vdbe *v){
  പൂർണ്ണസംഖ്യ addr;
  sqlite3VdbeAddOp2(v, OP_ResultRow, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  തിരികെ addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tableSkipIntegrityCheck(സ്ഥിരം Table *pTab, സ്ഥിരം Table *pObjTab){
  എങ്കിൽ( pObjTab ){
    തിരികെ pTab!=pObjTab;
  }അല്ലെങ്കിൽ{
    തിരികെ (pTab->tabFlags & TF_Imposter)!=0;
  }
}

/*
** Process a pragma statement. 
**
** Pragmas are of this form:
**
**      PRAGMA [schema.]id [= value]
**
** The identifier might also be a string.  The value is a string, and
** identifier, or a number.  If minusFlag is true, then the value is
** a number that was preceded by a minus sign.
**
** If the left side is "database.id" then pId1 is the database name
** and pId2 is the id.  If the left side is just "id" then pId1 is the
** id and pId2 is any empty string.
*/
ശൂന്യം sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_മൂല്യം,      /* Token for <value>, or NULL */
  പൂർണ്ണസംഖ്യ minusFlag       /* True if a '-' sign preceded <value> */
){
  അക്ഷരം *z_ഇടത് = 0;       /* Nul-terminated UTF-8 string <id> */
  അക്ഷരം *z_വലത് = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  സ്ഥിരം അക്ഷരം *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  അക്ഷരം *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  പൂർണ്ണസംഖ്യ iDb;               /* Database index for <database> */
  പൂർണ്ണസംഖ്യ rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  സ്ഥിരം PragmaName *pPragma;   /* The pragma */

  എങ്കിൽ( v==0 ) തിരികെ;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  എങ്കിൽ( iDb<0 ) തിരികെ;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  എങ്കിൽ( iDb==1 && sqlite3OpenTempDatabase(pParse) ){
    തിരികെ;
  }

  z_ഇടത് = sqlite3NameFromToken(db, pId);
  എങ്കിൽ( !z_ഇടത് ) തിരികെ;
  എങ്കിൽ( minusFlag ){
    z_വലത് = sqlite3MPrintf(db, "-%T", p_മൂല്യം);
  }അല്ലെങ്കിൽ{
    z_വലത് = sqlite3NameFromToken(db, p_മൂല്യം);
  }

  പ്രതിജ്ഞാ( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  എങ്കിൽ( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_ഇടത്, z_വലത്, zDb) ){
    പോകുക pragma_out;
  }

  /* Send an SQLITE_FCNTL_PRAGMA file-control to the underlying VFS
  ** connection.  If it returns SQLITE_OK, then assume that the VFS
  ** handled the pragma and generate a no-op prepared statement.
  **
  ** IMPLEMENTATION-OF: R-12238-55120 Whenever a PRAGMA statement is parsed,
  ** an SQLITE_FCNTL_PRAGMA file control is sent to the open sqlite3_file
  ** object corresponding to the database file to which the pragma
  ** statement refers.
  **
  ** IMPLEMENTATION-OF: R-29875-31678 The argument to the SQLITE_FCNTL_PRAGMA
  ** file control is an array of pointers to strings (char**) in which the
  ** second element of the array is the name of the pragma and the third
  ** element is the argument to the pragma or NULL if the pragma has no
  ** argument.
  */
  aFcntl[0] = 0;
  aFcntl[1] = z_ഇടത്;
  aFcntl[2] = z_വലത്;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (ശൂന്യം*)aFcntl);
  എങ്കിൽ( rc==SQLITE_OK ){
    sqlite3_Vdbe_Set_അക്കം_Cols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    പോകുക pragma_out;
  }
  എങ്കിൽ( rc!=SQLITE_NOTFOUND ){
    എങ്കിൽ( aFcntl[0] ){
      sqlite3_Error_സന്ദേശം(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    പോകുക pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_ഇടത്);
  എങ്കിൽ( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    പോകുക pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  എങ്കിൽ( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    എങ്കിൽ( sqlite3ReadSchema(pParse) ) പോകുക pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  എങ്കിൽ( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_വലത്==0)
  ){
    setPragmaResultColumnNames(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  തിരഞ്ഞെടുപ്പ്( pPragma->ePragTyp ){
 
#എങ്കിൽ !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
  /*
  **  PRAGMA [schema.]default_cache_size
  **  PRAGMA [schema.]default_cache_size=N
  **
  ** The first form reports the current persistent setting for the
  ** page cache size.  The value returned is the maximum number of
  ** pages in the page cache.  The second form sets both the current
  ** page cache size value and the persistent page cache size value
  ** stored in the database file.
  **
  ** Older versions of SQLite would set the default cache size to a
  ** negative number to indicate synchronous=OFF.  These days, synchronous
  ** is always on by default regardless of the sign of the default cache
  ** size.  But continue to take the absolute value of the default cache
  ** size of historical compatibility.
  */
  അവസ്ഥ PragTyp_DEFAULT_CACHE_SIZE: {
    സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ iLn = VDBE_OFFSET_LINENO(2);
    സ്ഥിതം സ്ഥിരം VdbeOpList getCacheSize[] = {
      { OP_Transaction, 0, 0,        0},                         /* 0 */
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_SIZE},  /* 1 */
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_Subtract,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                         /* 6 */
      { OP_Noop,        0, 0,        0},
      { OP_ResultRow,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3VdbeUsesBtree(v, iDb);
    എങ്കിൽ( !z_വലത് ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(getCacheSize));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(getCacheSize), getCacheSize, iLn);
      എങ്കിൽ( ONLY_IF_REALLOC_STRESS(aOp==0) ) നിർത്തുക;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_SIZE;
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ size = sqlite3AbsInt32(sqlite3Atoi(z_വലത്));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_SIZE, size);
      പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    നിർത്തുക;
  }
#endif /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#എങ്കിൽ !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  അവസ്ഥ PragTyp_PAGE_SIZE: {
    Btree *pBt = pDb->pBt;
    പ്രതിജ്ഞാ( pBt!=0 );
    എങ്കിൽ( !z_വലത് ){
      പൂർണ്ണസംഖ്യ size = ALWAYS(pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
      returnSingleInt(v, size);
    }അല്ലെങ്കിൽ{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->nextPagesize = sqlite3Atoi(z_വലത്);
      എങ്കിൽ( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    നിർത്തുക;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  അവസ്ഥ PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    പൂർണ്ണസംഖ്യ b = -1;
    പ്രതിജ്ഞാ( pBt!=0 );
    എങ്കിൽ( z_വലത് ){
      എങ്കിൽ( sqlite3_stricmp(z_വലത്, "fast")==0 ){
        b = 2;
      }അല്ലെങ്കിൽ{
        b = sqlite3GetBoolean(z_വലത്, 0);
      }
    }
    എങ്കിൽ( pId2->n==0 && b>=0 ){
      പൂർണ്ണസംഖ്യ ii;
      വേണ്ടി(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    നിർത്തുക;
  }

  /*
  **  PRAGMA [schema.]max_page_count
  **  PRAGMA [schema.]max_page_count=N
  **
  ** The first form reports the current setting for the
  ** maximum number of pages in the database file.  The
  ** second form attempts to change this setting.  Both
  ** forms return the current setting.
  **
  ** The absolute value of N is used.  This is undocumented and might
  ** change.  The only purpose is to provide an easy way to test
  ** the sqlite3AbsInt32() function.
  **
  **  PRAGMA [schema.]page_count
  **
  ** Return the number of pages in the specified database.
  */
  അവസ്ഥ Prag_Typ_PAGE_എണ്ണം: {
    പൂർണ്ണസംഖ്യ iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    എങ്കിൽ( sqlite3Tolower(z_ഇടത്[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( z_വലത് && sqlite3DecOrHexToI64(z_വലത്,&x)==0 ){
        എങ്കിൽ( x<0 ) x = 0;
        അല്ലെങ്കിൽ എങ്കിൽ( x>0xfffffffe ) x = 0xfffffffe;
      }അല്ലെങ്കിൽ{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (പൂർണ്ണസംഖ്യ)x);
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, iReg, 1);
    നിർത്തുക;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  അവസ്ഥ PragTyp_LOCKING_MODE: {
    സ്ഥിരം അക്ഷരം *zRet = "normal";
    പൂർണ്ണസംഖ്യ eMode = getLockingMode(z_വലത്);

    എങ്കിൽ( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }അല്ലെങ്കിൽ{
      Pager *pPager;
      എങ്കിൽ( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        പൂർണ്ണസംഖ്യ ii;
        പ്രതിജ്ഞാ(pDb==&db->aDb[0]);
        വേണ്ടി(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }

    പ്രതിജ്ഞാ( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    എങ്കിൽ( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    നിർത്തുക;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  അവസ്ഥ PragTyp_JOURNAL_MODE: {
    പൂർണ്ണസംഖ്യ eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    പൂർണ്ണസംഖ്യ ii;           /* Loop counter */

    എങ്കിൽ( z_വലത്==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }അല്ലെങ്കിൽ{
      സ്ഥിരം അക്ഷരം *zMode;
      പൂർണ്ണസംഖ്യ n = sqlite3Strlen30(z_വലത്);
      വേണ്ടി(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        എങ്കിൽ( sqlite3StrNICmp(z_വലത്, zMode, n)==0 ) നിർത്തുക;
      }
      എങ്കിൽ( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      എങ്കിൽ( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    എങ്കിൽ( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    വേണ്ടി(ii=db->nDb-1; ii>=0; ii--){
      എങ്കിൽ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    നിർത്തുക;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  അവസ്ഥ PragTyp_JOURNAL_SIZE_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    എങ്കിൽ( z_വലത് ){
      sqlite3DecOrHexToI64(z_വലത്, &iLimit);
      എങ്കിൽ( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    നിർത്തുക;
  }

#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

  /*
  **  PRAGMA [schema.]auto_vacuum
  **  PRAGMA [schema.]auto_vacuum=N
  **
  ** Get or set the value of the database 'auto-vacuum' parameter.
  ** The value is one of:  0 NONE 1 FULL 2 INCREMENTAL
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  അവസ്ഥ PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    പ്രതിജ്ഞാ( pBt!=0 );
    എങ്കിൽ( !z_വലത് ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ eAuto = getAutoVacuum(z_വലത്);
      പ്രതിജ്ഞാ( eAuto>=0 && eAuto<=2 );
      db->nextAutovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      എങ്കിൽ( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ iLn = VDBE_OFFSET_LINENO(2);
        സ്ഥിതം സ്ഥിരം VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_വേര്_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        പൂർണ്ണസംഖ്യ iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, ArraySize(setMeta6), setMeta6, iLn);
        എങ്കിൽ( ONLY_IF_REALLOC_STRESS(aOp==0) ) നിർത്തുക;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    നിർത്തുക;
  }
#endif

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  അവസ്ഥ PragTyp_INCREMENTAL_VACUUM: {
    പൂർണ്ണസംഖ്യ iLimit = 0, addr;
    എങ്കിൽ( z_വലത്==0 || !sqlite3GetInt32(z_വലത്, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ResultRow, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    നിർത്തുക;
  }
#endif

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
  /*
  **  PRAGMA [schema.]cache_size
  **  PRAGMA [schema.]cache_size=N
  **
  ** The first form reports the current local setting for the
  ** page cache size. The second form sets the local
  ** page cache size value.  If N is positive then that is the
  ** number of pages in the cache.  If N is negative, then the
  ** number of pages is adjusted so that the cache uses -N kibibytes
  ** of memory.
  */
  അവസ്ഥ PragTyp_CACHE_SIZE: {
    പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    എങ്കിൽ( !z_വലത് ){
      returnSingleInt(v, pDb->pSchema->cache_size);
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ size = sqlite3Atoi(z_വലത്);
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    നിർത്തുക;
  }

  /*
  **  PRAGMA [schema.]cache_spill
  **  PRAGMA cache_spill=BOOLEAN
  **  PRAGMA [schema.]cache_spill=N
  **
  ** The first form reports the current local setting for the
  ** page cache spill size. The second form turns cache spill on
  ** or off.  When turning cache spill on, the size is set to the
  ** current cache_size.  The third form sets a spill size that
  ** may be different form the cache size.
  ** If N is positive then that is the
  ** number of pages in the cache.  If N is negative, then the
  ** number of pages is adjusted so that the cache uses -N kibibytes
  ** of memory.
  **
  ** If the number of cache_spill pages is less then the number of
  ** cache_size pages, no spilling occurs until the page count exceeds
  ** the number of cache_size pages.
  **
  ** The cache_spill=BOOLEAN setting applies to all attached schemas,
  ** not just the schema specified.
  */
  അവസ്ഥ PragTyp_CACHE_SPILL: {
    പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    എങ്കിൽ( !z_വലത് ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3BtreeSetSpillSize(pDb->pBt,0));
    }അല്ലെങ്കിൽ{
      പൂർണ്ണസംഖ്യ size = 1;
      എങ്കിൽ( sqlite3GetInt32(z_വലത്, &size) ){
        sqlite3BtreeSetSpillSize(pDb->pBt, size);
      }
      എങ്കിൽ( sqlite3GetBoolean(z_വലത്, size!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }അല്ലെങ്കിൽ{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    നിർത്തുക;
  }

  /*
  **  PRAGMA [schema.]mmap_size(N)
  **
  ** Used to set mapping size limit. The mapping size limit is
  ** used to limit the aggregate size of all memory mapped regions of the
  ** database file. If this parameter is set to zero, then memory mapping
  ** is not used at all.  If N is negative, then the default memory map
  ** limit determined by sqlite3_config(SQLITE_CONFIG_MMAP_SIZE) is set.
  ** The parameter N is measured in bytes.
  **
  ** This value is advisory.  The underlying VFS is free to memory map
  ** as little or as much as it wants.  Except, if N is set to 0 then the
  ** upper layers will never invoke the xFetch interfaces to the VFS.
  */
  അവസ്ഥ PragTyp_MMAP_SIZE: {
    sqlite3_int64 sz;
#എങ്കിൽ SQLITE_MAX_MMAP_SIZE>0
    പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    എങ്കിൽ( z_വലത് ){
      പൂർണ്ണസംഖ്യ ii;
      sqlite3DecOrHexToI64(z_വലത്, &sz);
      എങ്കിൽ( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      എങ്കിൽ( pId2->n==0 ) db->szMmap = sz;
      വേണ്ടി(ii=db->nDb-1; ii>=0; ii--){
        എങ്കിൽ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);
#അല്ലെങ്കിൽ
    sz = 0;
    rc = SQLITE_OK;
#endif
    എങ്കിൽ( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }അല്ലെങ്കിൽ എങ്കിൽ( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    നിർത്തുക;
  }

  /*
  **   PRAGMA temp_store
  **   PRAGMA temp_store = "default"|"memory"|"file"
  **
  ** Return or set the local value of the temp_store flag.  Changing
  ** the local value does not make changes to the disk file and the default
  ** value will be restored the next time the database is opened.
  **
  ** Note that it is possible for the library compile-time options to
  ** override this setting
  */
  അവസ്ഥ PragTyp_TEMP_STORE: {
    എങ്കിൽ( !z_വലത് ){
      returnSingleInt(v, db->temp_store);
    }അല്ലെങ്കിൽ{
      changeTempStorage(pParse, z_വലത്);
    }
    നിർത്തുക;
  }

  /*
  **   PRAGMA temp_store_directory
  **   PRAGMA temp_store_directory = ""|"directory_name"
  **
  ** Return or set the local value of the temp_store_directory flag.  Changing
  ** the value sets a specific directory to be used for temporary files.
  ** Setting to a null string reverts to the default temporary directory search.
  ** If temporary directory is changed, then invalidateTempStorage.
  **
  */
  അവസ്ഥ PragTyp_TEMP_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    എങ്കിൽ( !z_വലത് ){
      returnSingleText(v, sqlite3_temp_directory);
    }അല്ലെങ്കിൽ{
#ifndef SQLITE_OMIT_WSD
      എങ്കിൽ( z_വലത്[0] ){
        പൂർണ്ണസംഖ്യ res;
        rc = sqlite3OsAccess(db->pVfs, z_വലത്, SQLITE_ACCESS_READWRITE, &res);
        എങ്കിൽ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_സന്ദേശം(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          പോകുക pragma_out;
        }
      }
      എങ്കിൽ( SQLITE_TEMP_STORE==0
       || (SQLITE_TEMP_STORE==1 && db->temp_store<=1)
       || (SQLITE_TEMP_STORE==2 && db->temp_store==1)
      ){
        invalidateTempStorage(pParse);
      }
      sqlite3_free(sqlite3_temp_directory);
      എങ്കിൽ( z_വലത്[0] ){
        sqlite3_temp_directory = sqlite3_mprintf("%s", z_വലത്);
      }അല്ലെങ്കിൽ{
        sqlite3_temp_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    നിർത്തുക;
  }

#എങ്കിൽ SQLITE_OS_WIN
  /*
  **   PRAGMA data_store_directory
  **   PRAGMA data_store_directory = ""|"directory_name"
  **
  ** Return or set the local value of the data_store_directory flag.  Changing
  ** the value sets a specific directory to be used for database files that
  ** were specified with a relative pathname.  Setting to a null string reverts
  ** to the default database directory, which for database files specified with
  ** a relative path will probably be based on the current directory for the
  ** process.  Database file specified with an absolute path are not impacted
  ** by this setting, regardless of its value.
  **
  */
  അവസ്ഥ PragTyp_DATA_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    എങ്കിൽ( !z_വലത് ){
      returnSingleText(v, sqlite3_data_directory);
    }അല്ലെങ്കിൽ{
#ifndef SQLITE_OMIT_WSD
      എങ്കിൽ( z_വലത്[0] ){
        പൂർണ്ണസംഖ്യ res;
        rc = sqlite3OsAccess(db->pVfs, z_വലത്, SQLITE_ACCESS_READWRITE, &res);
        എങ്കിൽ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_സന്ദേശം(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          പോകുക pragma_out;
        }
      }
      sqlite3_free(sqlite3_data_directory);
      എങ്കിൽ( z_വലത്[0] ){
        sqlite3_data_directory = sqlite3_mprintf("%s", z_വലത്);
      }അല്ലെങ്കിൽ{
        sqlite3_data_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    നിർത്തുക;
  }
#endif

#എങ്കിൽ SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  അവസ്ഥ PragTyp_LOCK_PROXY_FILE: {
    എങ്കിൽ( !z_വലത് ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      അക്ഷരം *proxy_file_path = ശൂന്യ;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }അല്ലെങ്കിൽ{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      പൂർണ്ണസംഖ്യ res;
      എങ്കിൽ( z_വലത്[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     z_വലത്);
      } അല്ലെങ്കിൽ {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     ശൂന്യ);
      }
      എങ്കിൽ( res!=SQLITE_OK ){
        sqlite3_Error_സന്ദേശം(pParse, "failed to set lock proxy file");
        പോകുക pragma_out;
      }
    }
    നിർത്തുക;
  }
#endif /* SQLITE_ENABLE_LOCKING_STYLE */     
   
  /*
  **   PRAGMA [schema.]synchronous
  **   PRAGMA [schema.]synchronous=OFF|ON|NORMAL|FULL|EXTRA
  **
  ** Return or set the local value of the synchronous flag.  Changing
  ** the local value does not make changes to the disk file and the
  ** default value will be restored the next time the database is
  ** opened.
  */
  അവസ്ഥ PragTyp_SYNCHRONOUS: {
    എങ്കിൽ( !z_വലത് ){
      returnSingleInt(v, pDb->safety_level-1);
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( !db->autoCommit ){
        sqlite3_Error_സന്ദേശം(pParse,
            "Safety level may not be changed inside a transaction");
      }അല്ലെങ്കിൽ എങ്കിൽ( iDb!=1 ){
        പൂർണ്ണസംഖ്യ iLevel;
        എങ്കിൽ( sqlite3Isdigit(z_വലത്[0]) ){
          iLevel = sqlite3Atoi(z_വലത്);
        }അല്ലെങ്കിൽ{
          iLevel = getSafetyLevel(z_വലത്,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        എങ്കിൽ( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    നിർത്തുക;
  }
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  അവസ്ഥ PragTyp_FLAG: {
    എങ്കിൽ( z_വലത്==0 ){
      setPragmaResultColumnNames(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }അല്ലെങ്കിൽ{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      എങ്കിൽ( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_Foreign_താക്കോലുകൾ);
      }

      എങ്കിൽ( sqlite3GetBoolean(z_വലത്, 0) ){
        എങ്കിൽ( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }അല്ലെങ്കിൽ{
        db->flags &= ~mask;
        എങ്കിൽ( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        എങ്കിൽ( (mask & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_വലത്, "reset")==0
        ){
          /* IMP: R-60817-01178 If the argument is "RESET" then schema
          ** writing is disabled (as with "PRAGMA writable_schema=OFF") and,
          ** in addition, the schema is reloaded. */
          sqlite3ResetAllSchemasOfConnection(db);
        }
      }

      /* Many of the flag-pragmas modify the code generated by the SQL
      ** compiler (eg. count_changes). So add an opcode to expire all
      ** compiled SQL statements after modifying a pragma value.
      */
      sqlite3VdbeAddOp0(v, OP_Expire);
      setAllPagerFlags(db);
    }
    നിർത്തുക;
  }
#endif /* SQLITE_OMIT_FLAG_PRAGMAS */

#ifndef SQLITE_OMIT_SCHEMA_PRAGMAS
  /*
  **   PRAGMA table_info(<table>)
  **
  ** Return a single row for each column of the named table. The columns of
  ** the returned data set are:
  **
  ** cid:        Column id (numbered from left to right, starting at 0)
  ** name:       Column name
  ** type:       Column declaration type.
  ** notnull:    True if 'NOT NULL' is part of column declaration
  ** dflt_value: The default value for the column, if any.
  ** pk:         Non-zero for PK fields.
  */
  അവസ്ഥ PragTyp_TABLE_INFO: എങ്കിൽ( z_വലത് ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_വലത്, zDb);
    എങ്കിൽ( pTab ){
      പൂർണ്ണസംഖ്യ i, k;
      പൂർണ്ണസംഖ്യ nHidden = 0;
      Column *pCol;
      Index *pPk = sqlite3_Primary_താക്കോൽ_Index(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      വേണ്ടി(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        പൂർണ്ണസംഖ്യ isHidden = 0;
        സ്ഥിരം വ്യഞ്ജകം *p_Col_വ്യഞ്ജകം;
        എങ്കിൽ( pCol->colFlags & COLFLAG_NOINSERT ){
          എങ്കിൽ( pPragma->iArg==0 ){
            nHidden++;
            തുടരുക;
          }
          എങ്കിൽ( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }അല്ലെങ്കിൽ എങ്കിൽ( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }അല്ലെങ്കിൽ{ പ്രതിജ്ഞാ( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        എങ്കിൽ( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }അല്ലെങ്കിൽ എങ്കിൽ( pPk==0 ){
          k = 1;
        }അല്ലെങ്കിൽ{
          വേണ്ടി(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        p_Col_വ്യഞ്ജകം = sqlite3_Column_വ്യഞ്ജകം(pTab,pCol);
        പ്രതിജ്ഞാ( p_Col_വ്യഞ്ജകം==0 || p_Col_വ്യഞ്ജകം->op==TK_SPAN || isHidden>=2 );
        പ്രതിജ്ഞാ( p_Col_വ്യഞ്ജകം==0 || !വ്യഞ്ജകം_Has_Property(p_Col_വ്യഞ്ജകം, EP_Int_മൂല്യം)
                  || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || p_Col_വ്യഞ്ജകം==0) ? 0 : p_Col_വ്യഞ്ജകം->u.zToken,
               k,
               isHidden);
      }
    }
  }
  നിർത്തുക;

  /*
  **   PRAGMA table_list
  **
  ** Return a single row for each table, virtual table, or view in the
  ** entire schema.
  **
  ** schema:     Name of attached database hold this table
  ** name:       Name of the table itself
  ** type:       "table", "view", "virtual", "shadow"
  ** ncol:       Number of columns
  ** wr:         True for a WITHOUT ROWID table
  ** strict:     True for a STRICT table
  */
  അവസ്ഥ PragTyp_TABLE_LIST: {
    പൂർണ്ണസംഖ്യ ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    വേണ്ടി(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      പൂർണ്ണസംഖ്യ initNCol;
      എങ്കിൽ( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) തുടരുക;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_എണ്ണം(pHash);
      ആകുമ്പോൾ( initNCol-- ){
        വേണ്ടി(k=sqliteHashFirst(pHash); 1; k=sqliteHashNext(k) ){
          Table *pTab;
          എങ്കിൽ( k==0 ){ initNCol = 0; നിർത്തുക; }
          pTab = sqliteHashData(k);
          എങ്കിൽ( pTab->nCol==0 ){
            അക്ഷരം *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            എങ്കിൽ( zSql ){
              sqlite3_stmt *pDummy = 0;
              (ശൂന്യം)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (ശൂന്യം)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            എങ്കിൽ( db->mallocFailed ){
              sqlite3_Error_സന്ദേശം(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            നിർത്തുക;
          }
        }
      }

      വേണ്ടി(k=sqliteHashFirst(pHash); k; k=sqliteHashNext(k) ){
        Table *pTab = sqliteHashData(k);
        സ്ഥിരം അക്ഷരം *zType;
        എങ്കിൽ( z_വലത് && sqlite3_stricmp(z_വലത്, pTab->zName)!=0 ) തുടരുക;
        എങ്കിൽ( IsView(pTab) ){
          zType = "view";
        }അല്ലെങ്കിൽ എങ്കിൽ( IsVirtual(pTab) ){
          zType = "virtual";
        }അല്ലെങ്കിൽ എങ്കിൽ( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }അല്ലെങ്കിൽ{
          zType = "table";
        }
        sqlite3VdbeMultiLoad(v, 1, "sssiii",
           db->aDb[ii].zDbSName,
           sqlite3PreferredTableName(pTab->zName),
           zType,
           pTab->nCol,
           (pTab->tabFlags & TF_WithoutRowid)!=0,
           (pTab->tabFlags & TF_Strict)!=0
        );
      }
    }
  }
  നിർത്തുക;

#ifdef SQLITE_DEBUG
  അവസ്ഥ PragTyp_STATS: {
    Index *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    വേണ്ടി(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqliteHashNext(i)){
      Table *pTab = sqliteHashData(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      വേണ്ടി(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 5);
      }
    }
  }
  നിർത്തുക;
#endif

  അവസ്ഥ PragTyp_INDEX_INFO: എങ്കിൽ( z_വലത് ){
    Index *pIdx;
    Table *pTab;
    pIdx = sqlite3FindIndex(db, z_വലത്, zDb);
    എങ്കിൽ( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_വലത്, zDb);
      എങ്കിൽ( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_താക്കോൽ_Index(pTab);
      }
    }
    എങ്കിൽ( pIdx ){
      പൂർണ്ണസംഖ്യ iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
      പൂർണ്ണസംഖ്യ i;
      പൂർണ്ണസംഖ്യ mx;
      എങ്കിൽ( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }അല്ലെങ്കിൽ{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_താക്കോൽ_Col;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      പ്രതിജ്ഞാ( pParse->nMem<=pPragma->nPragCName );
      വേണ്ടി(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        എങ്കിൽ( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->n_താക്കോൽ_Col);
        }
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, pParse->nMem);
      }
    }
  }
  നിർത്തുക;

  അവസ്ഥ PragTyp_INDEX_LIST: എങ്കിൽ( z_വലത് ){
    Index *pIdx;
    Table *pTab;
    പൂർണ്ണസംഖ്യ i;
    pTab = sqlite3FindTable(db, z_വലത്, zDb);
    എങ്കിൽ( pTab ){
      പൂർണ്ണസംഖ്യ iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      വേണ്ടി(pIdx=pTab->pIndex, i=0; pIdx; pIdx=pIdx->pNext, i++){
        സ്ഥിരം അക്ഷരം *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           IsUniqueIndex(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  നിർത്തുക;

  അവസ്ഥ PragTyp_DATABASE_LIST: {
    പൂർണ്ണസംഖ്യ i;
    pParse->nMem = 3;
    വേണ്ടി(i=0; i<db->nDb; i++){
      എങ്കിൽ( db->aDb[i].pBt==0 ) തുടരുക;
      പ്രതിജ്ഞാ( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  നിർത്തുക;

  അവസ്ഥ PragTyp_COLLATION_LIST: {
    പൂർണ്ണസംഖ്യ i = 0;
    HashElem *p;
    pParse->nMem = 2;
    വേണ്ടി(p=sqliteHashFirst(&db->aCollSeq); p; p=sqliteHashNext(p)){
      CollSeq *pColl = (CollSeq *)sqliteHashData(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  നിർത്തുക;

#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  അവസ്ഥ PragTyp_FUNCTION_LIST: {
    പൂർണ്ണസംഖ്യ i;
    HashElem *j;
    FuncDef *p;
    പൂർണ്ണസംഖ്യ showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    വേണ്ടി(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      വേണ്ടി(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        പ്രതിജ്ഞാ( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_വരി(v, p, 1, showInternFunc);
      }
    }
    വേണ്ടി(j=sqliteHashFirst(&db->aFunc); j; j=sqliteHashNext(j)){
      p = (FuncDef*)sqliteHashData(j);
      പ്രതിജ്ഞാ( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_വരി(v, p, 0, showInternFunc);
    }
  }
  നിർത്തുക;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  അവസ്ഥ PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    വേണ്ടി(j=sqliteHashFirst(&db->aModule); j; j=sqliteHashNext(j)){
      Module *pMod = (Module*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  നിർത്തുക;
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  അവസ്ഥ PragTyp_PRAGMA_LIST: {
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<ArraySize(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  നിർത്തുക;
#endif /* SQLITE_INTROSPECTION_PRAGMAS */

#endif /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ifndef SQLITE_OMIT_FOREIGN_താക്കോൽ
  അവസ്ഥ Prag_Typ_FOREIGN_താക്കോൽ_LIST: എങ്കിൽ( z_വലത് ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, z_വലത്, zDb);
    എങ്കിൽ( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      എങ്കിൽ( pFK ){
        പൂർണ്ണസംഖ്യ iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        പൂർണ്ണസംഖ്യ i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        ആകുമ്പോൾ(pFK){
          പൂർണ്ണസംഖ്യ j;
          വേണ്ടി(j=0; j<pFK->nCol; j++){
            sqlite3VdbeMultiLoad(v, 1, "iissssss",
                   i,
                   j,
                   pFK->zTo,
                   pTab->aCol[pFK->aCol[j].iFrom].zCnName,
                   pFK->aCol[j].zCol,
                   actionName(pFK->aAction[1]),  /* ON UPDATE */
                   actionName(pFK->aAction[0]),  /* ON DELETE */
                   "NONE");
          }
          ++i;
          pFK = pFK->pNextFrom;
        }
      }
    }
  }
  നിർത്തുക;
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_FOREIGN_താക്കോൽ
#ifndef SQLITE_OMIT_TRIGGER
  അവസ്ഥ Prag_Typ_FOREIGN_താക്കോൽ_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    Index *pIdx;           /* Index in the parent table */
    പൂർണ്ണസംഖ്യ i;                 /* Loop counter:  Foreign key number for pTab */
    പൂർണ്ണസംഖ്യ j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    പൂർണ്ണസംഖ്യ x;                 /* result variable */
    പൂർണ്ണസംഖ്യ regResult;         /* 3 registers to hold a result row */
    പൂർണ്ണസംഖ്യ regRow;            /* Registers to hold a row from pTab */
    പൂർണ്ണസംഖ്യ addr_മുകൾ;           /* Top of a loop checking foreign keys */
    പൂർണ്ണസംഖ്യ addrOk;            /* Jump here if the key is OK */
    പൂർണ്ണസംഖ്യ *aiCols;           /* child to parent column mapping */

    regResult = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    ആകുമ്പോൾ( k ){
      എങ്കിൽ( z_വലത് ){
        pTab = sqlite3LocateTable(pParse, 0, z_വലത്, zDb);
        k = 0;
      }അല്ലെങ്കിൽ{
        pTab = (Table*)sqliteHashData(k);
        k = sqliteHashNext(k);
      }
      എങ്കിൽ( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) തുടരുക;
      iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, regResult, pTab->zName);
      പ്രതിജ്ഞാ( IsOrdinaryTable(pTab) );
      വേണ്ടി(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        എങ്കിൽ( pParent==0 ) തുടരുക;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
        എങ്കിൽ( x==0 ){
          എങ്കിൽ( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }അല്ലെങ്കിൽ{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_താക്കോൽ_Info(pParse, pIdx);
          }
        }അല്ലെങ്കിൽ{
          k = 0;
          നിർത്തുക;
        }
      }
      പ്രതിജ്ഞാ( pParse->nErr>0 || pFK==0 );
      എങ്കിൽ( pFK ) നിർത്തുക;
      എങ്കിൽ( pParse->nTab<i ) pParse->nTab = i;
      addr_മുകൾ = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      പ്രതിജ്ഞാ( IsOrdinaryTable(pTab) );
      വേണ്ടി(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        എങ്കിൽ( pParent ){
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          പ്രതിജ്ഞാ( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        വേണ്ടി(j=0; j<pFK->nCol; j++){
          പൂർണ്ണസംഖ്യ iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3_വ്യഞ്ജകം_Code_Get_Column_Of_Table(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        എങ്കിൽ( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3IndexAffinityStr(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }അല്ലെങ്കിൽ എങ്കിൽ( pParent ){
          പൂർണ്ണസംഖ്യ jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          പ്രതിജ്ഞാ( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        എങ്കിൽ( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, regResult+1);
        }അല്ലെങ്കിൽ{
          sqlite3VdbeAddOp2(v, OP_Null, 0, regResult+1);
        }
        sqlite3VdbeMultiLoad(v, regResult+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_Next, 0, addr_മുകൾ+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addr_മുകൾ);
    }
  }
  നിർത്തുക;
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  അവസ്ഥ PragTyp_CASE_SENSITIVE_LIKE: {
    എങ്കിൽ( z_വലത് ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(z_വലത്, 0));
    }
  }
  നിർത്തുക;
#endif /* SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA */

#ifndef SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#endif

#ifndef SQLITE_OMIT_INTEGRITY_CHECK
  /*    PRAGMA integrity_check
  **    PRAGMA integrity_check(N)
  **    PRAGMA quick_check
  **    PRAGMA quick_check(N)
  **
  ** Verify the integrity of the database.
  **
  ** The "quick_check" is reduced version of
  ** integrity_check designed to detect most database corruption
  ** without the overhead of cross-checking indexes.  Quick_check
  ** is linear time whereas integrity_check is O(NlogN).
  **
  ** The maximum number of errors is 100 by default.  A different default
  ** can be specified using a numeric parameter N.
  **
  ** Or, the parameter N can be the name of a table.  In that case, only
  ** the one table named is verified.  The freelist is only verified if
  ** the named table is "sqlite_schema" (or one of its aliases).
  **
  ** All schemas are checked by default.  To check just a single
  ** schema, use the form:
  **
  **      PRAGMA schema.integrity_check;
  */
  അവസ്ഥ PragTyp_INTEGRITY_CHECK: {
    പൂർണ്ണസംഖ്യ i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    പൂർണ്ണസംഖ്യ isQuick = (sqlite3Tolower(z_ഇടത്[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    പ്രതിജ്ഞാ( iDb>=0 );
    പ്രതിജ്ഞാ( iDb==0 || pId2->z );
    എങ്കിൽ( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    എങ്കിൽ( z_വലത് ){
      എങ്കിൽ( sqlite3GetInt32(p_മൂല്യം->z, &mxErr) ){
        എങ്കിൽ( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }അല്ലെങ്കിൽ{
        pObjTab = sqlite3LocateTable(pParse, 0, z_വലത്,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    വേണ്ടി(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      പൂർണ്ണസംഖ്യ *a_വേര്;      /* Array of root page numbers of all btrees */
      പൂർണ്ണസംഖ്യ cnt = 0;     /* Number of entries in aRoot[] */

      എങ്കിൽ( OMIT_TEMPDB && i==1 ) തുടരുക;
      എങ്കിൽ( iDb>=0 && i!=iDb ) തുടരുക;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      പ്രതിജ്ഞാ( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      വേണ്ടി(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);  /* Current table */
        Index *pIdx;                      /* An index on pTab */
        എങ്കിൽ( tableSkipIntegrityCheck(pTab,pObjTab) ) തുടരുക;
        എങ്കിൽ( HasRowid(pTab) ) cnt++;
        വേണ്ടി(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){ cnt++; }
      }
      എങ്കിൽ( cnt==0 ) തുടരുക;
      എങ്കിൽ( pObjTab ) cnt++;
      a_വേര് = sqlite3DbMallocRawNN(db, വലുപ്പം(പൂർണ്ണസംഖ്യ)*(cnt+1));
      എങ്കിൽ( a_വേര്==0 ) നിർത്തുക;
      cnt = 0;
      എങ്കിൽ( pObjTab ) a_വേര്[++cnt] = 0;
      വേണ്ടി(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        എങ്കിൽ( tableSkipIntegrityCheck(pTab,pObjTab) ) തുടരുക;
        എങ്കിൽ( HasRowid(pTab) ) a_വേര്[++cnt] = pTab->tnum;
        വേണ്ടി(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          a_വേര്[++cnt] = pIdx->tnum;
        }
      }
      a_വേര്[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3ClearTempRegCache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (അക്ഷരം*)a_വേര്,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u16)i);
      addr = sqlite3VdbeAddOp1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3VdbeAddOp3(v, OP_Concat, 2, 3, 3);
      integrityCheckResultRow(v);
      sqlite3VdbeJumpHere(v, addr);

      /* Check that the indexes all have the right number of rows */
      cnt = pObjTab ? 1 : 0;
      sqlite3VdbeLoadString(v, 2, "wrong # of entries in index ");
      വേണ്ടി(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        പൂർണ്ണസംഖ്യ iTab = 0;
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        എങ്കിൽ( tableSkipIntegrityCheck(pTab,pObjTab) ) തുടരുക;
        എങ്കിൽ( HasRowid(pTab) ){
          iTab = cnt++;
        }അല്ലെങ്കിൽ{
          iTab = cnt;
          വേണ്ടി(pIdx=pTab->pIndex; ALWAYS(pIdx); pIdx=pIdx->pNext){
            എങ്കിൽ( Is_Primary_താക്കോൽ_Index(pIdx) ) നിർത്തുക;
            iTab++;
          }
        }
        വേണ്ടി(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          എങ്കിൽ( pIdx->pPartIdxWhere==0 ){
            addr = sqlite3VdbeAddOp3(v, OP_Eq, 8+cnt, 0, 8+iTab);
            VdbeCoverageNeverNull(v);
            sqlite3VdbeLoadString(v, 4, pIdx->zName);
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 2, 3);
            integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, addr);
          }
          cnt++;
        }
      }

      /* Make sure all the indices are constructed correctly.
      */
      വേണ്ടി(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx, *pPk;
        Index *pPrior = 0;      /* Previous index */
        പൂർണ്ണസംഖ്യ loop_മുകൾ;
        പൂർണ്ണസംഖ്യ iDataCur, iIdxCur;
        പൂർണ്ണസംഖ്യ r1 = -1;
        പൂർണ്ണസംഖ്യ bStrict;            /* True for a STRICT table */
        പൂർണ്ണസംഖ്യ r2;                 /* Previous key for WITHOUT ROWID tables */
        പൂർണ്ണസംഖ്യ mxCol;              /* Maximum non-virtual column number */

        എങ്കിൽ( tableSkipIntegrityCheck(pTab,pObjTab) ) തുടരുക;
        എങ്കിൽ( !IsOrdinaryTable(pTab) ) തുടരുക;
        എങ്കിൽ( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }അല്ലെങ്കിൽ{
          pPk = sqlite3_Primary_താക്കോൽ_Index(pTab);
          r2 = sqlite3GetTempRange(pParse, pPk->n_താക്കോൽ_Col);
          sqlite3VdbeAddOp3(v, OP_Null, 1, r2, r2+pPk->n_താക്കോൽ_Col-1);
        }
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &iDataCur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        വേണ്ടി(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        പ്രതിജ്ഞാ( pParse->nMem>=8+j );
        പ്രതിജ്ഞാ( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, iDataCur, 0); VdbeCoverage(v);
        loop_മുകൾ = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        പ്രതിജ്ഞാ( !IsVirtual(pTab) );
        എങ്കിൽ( HasRowid(pTab) ){
          mxCol = -1;
          വേണ്ടി(j=0; j<pTab->nCol; j++){
            എങ്കിൽ( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          എങ്കിൽ( mxCol==pTab->iPKey ) mxCol--;
        }അല്ലെങ്കിൽ{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3_Primary_താക്കോൽ_Index(pTab)->nColumn-1;
        }
        എങ്കിൽ( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, iDataCur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        എങ്കിൽ( !isQuick ){
          എങ്കിൽ( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            പൂർണ്ണസംഖ്യ a1;
            അക്ഷരം *zErr;
            a1 = sqlite3VdbeAddOp4Int(v, OP_IdxGT, iDataCur, 0,r2,pPk->n_താക്കോൽ_Col);
            VdbeCoverage(v);
            sqlite3VdbeAddOp1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, a1);
            sqlite3VdbeJumpHere(v, a1+1);
            വേണ്ടി(j=0; j<pPk->n_താക്കോൽ_Col; j++){
              sqlite3_വ്യഞ്ജകം_Code_Load_Index_Column(pParse, pPk, iDataCur, j, r2+j);
            }
          }
        }
        /* Verify datatypes for all columns:
        **
        **   (1) NOT NULL columns may not contain a NULL
        **   (2) Datatype must be exact for non-ANY columns in STRICT tables
        **   (3) Datatype for TEXT columns in non-STRICT tables must be
        **       NULL, TEXT, or BLOB.
        **   (4) Datatype for numeric columns in non-STRICT tables must not
        **       be a TEXT value that can be losslessly converted to numeric.
        */
        bStrict = (pTab->tabFlags & TF_Strict)!=0;
        വേണ്ടി(j=0; j<pTab->nCol; j++){
          അക്ഷരം *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          പൂർണ്ണസംഖ്യ labelError;               /* Jump here to report an error */
          പൂർണ്ണസംഖ്യ labelOk;                  /* Jump here if all looks ok */
          പൂർണ്ണസംഖ്യ p1, p3, p4;               /* Operands to the OP_IsType opcode */
          പൂർണ്ണസംഖ്യ doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          എങ്കിൽ( j==pTab->iPKey ) തുടരുക;
          എങ്കിൽ( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }അല്ലെങ്കിൽ{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          എങ്കിൽ( pCol->notNull==0 && !doTypeCheck ) തുടരുക;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          എങ്കിൽ( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3_വ്യഞ്ജകം_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
            p1 = -1;
            p3 = 3;
          }അല്ലെങ്കിൽ{
            എങ്കിൽ( pCol->iDflt ){
              sqlite3_മൂല്യം *p_Dflt_മൂല്യം = 0;
              sqlite3_മൂല്യം_From_വ്യഞ്ജകം(db, sqlite3_Column_വ്യഞ്ജകം_Auth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &p_Dflt_മൂല്യം);
              എങ്കിൽ( p_Dflt_മൂല്യം ){
                p4 = sqlite3_മൂല്യം_type(p_Dflt_മൂല്യം);
                sqlite3_മൂല്യം_Free(p_Dflt_മൂല്യം);
              }
            }
            p1 = iDataCur;
            എങ്കിൽ( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3TableColumnToIndex(sqlite3_Primary_താക്കോൽ_Index(pTab), j);
            }അല്ലെങ്കിൽ{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          എങ്കിൽ( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            പൂർണ്ണസംഖ്യ jmp3;
            പൂർണ്ണസംഖ്യ jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            എങ്കിൽ( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }അല്ലെങ്കിൽ{
              sqlite3VdbeChangeP5(v, 0x0d); /* INT, TEXT, or BLOB */
              /* OP_IsType does not detect NaN values in the database file
              ** which should be treated as a NULL.  So if the header type
              ** is REAL, we have to load the actual data using OP_Column
              ** to reliably determine if the value is a NULL. */
              sqlite3VdbeAddOp3(v, OP_Column, p1, p3, 3);
              sqlite3ColumnDefault(v, pTab, j, 3);
              jmp3 = sqlite3VdbeAddOp2(v, OP_NotNull, 3, labelOk);
              VdbeCoverage(v);
            }           
            zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName,
                                pCol->zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            എങ്കിൽ( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }അല്ലെങ്കിൽ{
              /* VDBE byte code will fall thru */
            }
          }
          എങ്കിൽ( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            സ്ഥിതം unsigned അക്ഷരം aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            പ്രതിജ്ഞാ( pCol->eCType>=1 && pCol->eCType<=വലുപ്പം(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }അല്ലെങ്കിൽ എങ്കിൽ( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }അല്ലെങ്കിൽ എങ്കിൽ( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            എങ്കിൽ( p1>=0 ){
              sqlite3_വ്യഞ്ജകം_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
            }
            sqlite3VdbeAddOp4(v, OP_Affinity, 3, 1, 0, "C", P4_STATIC);
            sqlite3VdbeAddOp4Int(v, OP_IsType, -1, labelOk, 3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "TEXT value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }
          sqlite3VdbeResolveLabel(v, labelError);
          integrityCheckResultRow(v);
          sqlite3VdbeResolveLabel(v, labelOk);
        }
        /* Verify CHECK constraints */
        എങ്കിൽ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          വ്യഞ്ജകം_List *pCheck = sqlite3_വ്യഞ്ജകം_List_Dup(db, pTab->pCheck, 0);
          എങ്കിൽ( db->mallocFailed==0 ){
            പൂർണ്ണസംഖ്യ addrCkFault = sqlite3VdbeMakeLabel(pParse);
            പൂർണ്ണസംഖ്യ addrCkOk = sqlite3VdbeMakeLabel(pParse);
            അക്ഷരം *zErr;
            പൂർണ്ണസംഖ്യ k;
            pParse->iSelfTab = iDataCur + 1;
            വേണ്ടി(k=pCheck->n_വ്യഞ്ജകം-1; k>0; k--){
              sqlite3_വ്യഞ്ജകം_If_False(pParse, pCheck->a[k].p_വ്യഞ്ജകം, addrCkFault, 0);
            }
            sqlite3_വ്യഞ്ജകം_If_True(pParse, pCheck->a[0].p_വ്യഞ്ജകം, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_വ്യഞ്ജകം_List_Delete(db, pCheck);
        }
        എങ്കിൽ( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          വേണ്ടി(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            പൂർണ്ണസംഖ്യ jmp2, jmp3, jmp4, jmp5, label6;
            പൂർണ്ണസംഖ്യ kk;
            പൂർണ്ണസംഖ്യ ckUniq = sqlite3VdbeMakeLabel(pParse);
            എങ്കിൽ( pPk==pIdx ) തുടരുക;
            r1 = sqlite3_Generate_Index_താക്കോൽ(pParse, pIdx, iDataCur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFind_താക്കോൽ, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (സ്ഥിരം അക്ഷരം*)pIdx, P4_INDEX);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0,
              sqlite3MPrintf(db, "index %s stores an imprecise floating-point "
                                 "value for row ", pIdx->zName),
              P4_DYNAMIC);
            sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
            integrityCheckResultRow(v);
            sqlite3VdbeAddOp2(v, OP_Goto, 0, ckUniq);

            sqlite3VdbeJumpHere(v, jmp2);
            sqlite3VdbeLoadString(v, 3, "row ");
            sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
            sqlite3VdbeLoadString(v, 4, " missing from index ");
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3VdbeLoadString(v, 4, pIdx->zName);
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, ckUniq);

            /* The OP_IdxRowid opcode is an optimized version of OP_Column
            ** that extracts the rowid off the end of the index record.
            ** But it only works correctly if index record does not have
            ** any extra bytes at the end.  Verify that this is the case. */
            എങ്കിൽ( HasRowid(pTab) ){
              പൂർണ്ണസംഖ്യ jmp7;
              sqlite3VdbeAddOp2(v, OP_IdxRowid, iIdxCur+j, 3);
              jmp7 = sqlite3VdbeAddOp3(v, OP_Eq, 3, 0, r1+pIdx->nColumn-1);
              VdbeCoverageNeverNull(v);
              sqlite3VdbeLoadString(v, 3,
                 "rowid not at end-of-record for row ");
              sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
              sqlite3VdbeLoadString(v, 4, " of index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3VdbeJumpHere(v, jmp7);
            }

            /* Any indexed columns with non-BINARY collations must still hold
            ** the exact same text value as the table. */
            label6 = 0;
            വേണ്ടി(kk=0; kk<pIdx->n_താക്കോൽ_Col; kk++){
              എങ്കിൽ( pIdx->azColl[kk]==sqlite3_Str_ദ്വിമാന ) തുടരുക;
              എങ്കിൽ( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            എങ്കിൽ( label6 ){
              പൂർണ്ണസംഖ്യ jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
              sqlite3VdbeResolveLabel(v, label6);
              sqlite3VdbeLoadString(v, 3, "row ");
              sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
              sqlite3VdbeLoadString(v, 4, " values differ from index ");
              sqlite3VdbeGoto(v, jmp5-1);
              sqlite3VdbeJumpHere(v, jmp6);
            }
             
            /* For UNIQUE indexes, verify that only one entry exists with the
            ** current key.  The entry is unique if (1) any column is NULL
            ** or (2) the next entry has a different key */
            എങ്കിൽ( IsUniqueIndex(pIdx) ){
              പൂർണ്ണസംഖ്യ uniqOk = sqlite3VdbeMakeLabel(pParse);
              പൂർണ്ണസംഖ്യ jmp6;
              വേണ്ടി(kk=0; kk<pIdx->n_താക്കോൽ_Col; kk++){
                പൂർണ്ണസംഖ്യ iCol = pIdx->aiColumn[kk];
                പ്രതിജ്ഞാ( iCol!=XN_ROWID && iCol<pTab->nCol );
                എങ്കിൽ( iCol>=0 && pTab->aCol[iCol].notNull ) തുടരുക;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_Next, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_താക്കോൽ_Col); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_Next, iDataCur, loop_മുകൾ); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loop_മുകൾ-1);
        എങ്കിൽ( pPk ){
          പ്രതിജ്ഞാ( !isQuick );
          sqlite3ReleaseTempRange(pParse, r2, pPk->n_താക്കോൽ_Col);
        }
      }

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      വേണ്ടി(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        sqlite3_vtab *pVTab;
        പൂർണ്ണസംഖ്യ a1;
        എങ്കിൽ( tableSkipIntegrityCheck(pTab,pObjTab) ) തുടരുക;
        എങ്കിൽ( IsOrdinaryTable(pTab) ) തുടരുക;
        എങ്കിൽ( !IsVirtual(pTab) ) തുടരുക;
        എങ്കിൽ( pTab->nCol<=0 ){
          സ്ഥിരം അക്ഷരം *zMod = pTab->u.vtab.azArg[0];
          എങ്കിൽ( sqlite3HashFind(&db->aModule, zMod)==0 ) തുടരുക;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        എങ്കിൽ( pTab->u.vtab.p==0 ) തുടരുക;
        pVTab = pTab->u.vtab.p->pVtab;
        എങ്കിൽ( NEVER(pVTab==0) ) തുടരുക;
        എങ്കിൽ( NEVER(pVTab->pModule==0) ) തുടരുക;
        എങ്കിൽ( pVTab->pModule->iVersion<4 ) തുടരുക;
        എങ്കിൽ( pVTab->pModule->xIntegrity==0 ) തുടരുക;
        sqlite3VdbeAddOp3(v, OP_VCheck, i, 3, isQuick);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3VdbeAddOp1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrityCheckResultRow(v);
        sqlite3VdbeJumpHere(v, a1);
        തുടരുക;
      }
#endif
    }
    {
      സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ iLn = VDBE_OFFSET_LINENO(2);
      സ്ഥിതം സ്ഥിരം VdbeOpList endCode[] = {
        { OP_AddImm,      1, 0,        0},    /* 0 */
        { OP_IfNotZero,   1, 4,        0},    /* 1 */
        { OP_String8,     0, 3,        0},    /* 2 */
        { OP_ResultRow,   3, 1,        0},    /* 3 */
        { OP_Halt,        0, 0,        0},    /* 4 */
        { OP_String8,     0, 3,        0},    /* 5 */
        { OP_Goto,        0, 3,        0},    /* 6 */
      };
      VdbeOp *aOp;

      aOp = sqlite3VdbeAddOpList(v, ArraySize(endCode), endCode, iLn);
      എങ്കിൽ( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (അക്ഷരം*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  നിർത്തുക;
#endif /* SQLITE_OMIT_INTEGRITY_CHECK */

#ifndef SQLITE_OMIT_UTF16
  /*
  **   PRAGMA encoding
  **   PRAGMA encoding = "utf-8"|"utf-16"|"utf-16le"|"utf-16be"
  **
  ** In its first form, this pragma returns the encoding of the main
  ** database. If the database is not initialized, it is initialized now.
  **
  ** The second form of this pragma is a no-op if the main database file
  ** has not already been initialized. In this case it sets the default
  ** encoding that will be used for the main database file if a new file
  ** is created. If an existing main database file is opened, then the
  ** default text encoding for the existing database is used.
  **
  ** In all cases new databases created using the ATTACH command are
  ** created to use the same default text encoding as the main database. If
  ** the main database has not been initialized and/or created when ATTACH
  ** is executed, this is done before the ATTACH operation.
  **
  ** In the second form this pragma sets the text encoding to be used in
  ** new database files created using this database handle. It is only
  ** useful if invoked immediately after the main database i
  */
  അവസ്ഥ PragTyp_ENCODING: {
    സ്ഥിതം സ്ഥിരം ഘടന EncName {
      അക്ഷരം *zName;
      u8 enc;
    } encnames[] = {
      { "UTF8",     SQLITE_UTF8        },
      { "UTF-8",    SQLITE_UTF8        },  /* Must be element [1] */
      { "UTF-16le", SQLITE_UTF16LE     },  /* Must be element [2] */
      { "UTF-16be", SQLITE_UTF16BE     },  /* Must be element [3] */
      { "UTF16le",  SQLITE_UTF16LE     },
      { "UTF16be",  SQLITE_UTF16BE     },
      { "UTF-16",   0                  }, /* SQLITE_UTF16NATIVE */
      { "UTF16",    0                  }, /* SQLITE_UTF16NATIVE */
      { 0, 0 }
    };
    സ്ഥിരം ഘടന EncName *pEnc;
    എങ്കിൽ( !z_വലത് ){    /* "PRAGMA encoding" */
      എങ്കിൽ( sqlite3ReadSchema(pParse) ) പോകുക pragma_out;
      പ്രതിജ്ഞാ( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      പ്രതിജ്ഞാ( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      പ്രതിജ്ഞാ( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }അല്ലെങ്കിൽ{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      എങ്കിൽ( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        വേണ്ടി(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          എങ്കിൽ( 0==sqlite3StrICmp(z_വലത്, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            നിർത്തുക;
          }
        }
        എങ്കിൽ( !pEnc->zName ){
          sqlite3_Error_സന്ദേശം(pParse, "unsupported encoding: %s", z_വലത്);
        }
      }
    }
  }
  നിർത്തുക;
#endif /* SQLITE_OMIT_UTF16 */

#ifndef SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS
  /*
  **   PRAGMA [schema.]schema_version
  **   PRAGMA [schema.]schema_version = <integer>
  **
  **   PRAGMA [schema.]user_version
  **   PRAGMA [schema.]user_version = <integer>
  **
  **   PRAGMA [schema.]freelist_count
  **
  **   PRAGMA [schema.]data_version
  **
  **   PRAGMA [schema.]application_id
  **   PRAGMA [schema.]application_id = <integer>
  **
  ** The pragma's schema_version and user_version are used to set or get
  ** the value of the schema-version and user-version, respectively. Both
  ** the schema-version and the user-version are 32-bit signed integers
  ** stored in the database header.
  **
  ** The schema-cookie is usually only manipulated internally by SQLite. It
  ** is incremented by SQLite whenever the database schema is modified (by
  ** creating or dropping a table or index). The schema version is used by
  ** SQLite each time a query is executed to ensure that the internal cache
  ** of the schema used when compiling the SQL query matches the schema of
  ** the database against which the compiled query is actually executed.
  ** Subverting this mechanism by using "PRAGMA schema_version" to modify
  ** the schema-version is potentially dangerous and may lead to program
  ** crashes or database corruption. Use with caution!
  **
  ** The user-version is not used internally by SQLite. It may be used by
  ** applications for any purpose.
  */
  അവസ്ഥ Prag_Typ_HEADER_മൂല്യം: {
    പൂർണ്ണസംഖ്യ iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    എങ്കിൽ( z_വലത് && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      സ്ഥിതം സ്ഥിരം VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(setCookie), setCookie, 0);
      എങ്കിൽ( ONLY_IF_REALLOC_STRESS(aOp==0) ) നിർത്തുക;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_വലത്);
      aOp[1].p5 = 1;
      എങ്കിൽ( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }അല്ലെങ്കിൽ{
      /* Read the specified cookie value */
      സ്ഥിതം സ്ഥിരം VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ResultRow,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(readCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(readCookie),readCookie,0);
      എങ്കിൽ( ONLY_IF_REALLOC_STRESS(aOp==0) ) നിർത്തുക;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  നിർത്തുക;
#endif /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  അവസ്ഥ PragTyp_COMPILE_OPTIONS: {
    പൂർണ്ണസംഖ്യ i = 0;
    സ്ഥിരം അക്ഷരം *zOpt;
    pParse->nMem = 1;
    ആകുമ്പോൾ( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  നിർത്തുക;
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ifndef SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  അവസ്ഥ PragTyp_WAL_CHECKPOINT: {
    പൂർണ്ണസംഖ്യ iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    പൂർണ്ണസംഖ്യ eMode = SQLITE_CHECKPOINT_PASSIVE;
    എങ്കിൽ( z_വലത് ){
      എങ്കിൽ( sqlite3StrICmp(z_വലത്, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3StrICmp(z_വലത്, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3StrICmp(z_വലത്, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3StrICmp(z_വലത്, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 3);
  }
  നിർത്തുക;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  അവസ്ഥ PragTyp_WAL_AUTOCHECKPOINT: {
    എങ്കിൽ( z_വലത് ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_വലത്));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  നിർത്തുക;
#endif

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  അവസ്ഥ PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    നിർത്തുക;
  }

  /*
  **  PRAGMA optimize
  **  PRAGMA optimize(MASK)
  **  PRAGMA schema.optimize
  **  PRAGMA schema.optimize(MASK)
  **
  ** Attempt to optimize the database.  All schemas are optimized in the first
  ** two forms, and only the specified schema is optimized in the latter two.
  **
  ** The details of optimizations performed by this pragma are expected
  ** to change and improve over time.  Applications should anticipate that
  ** this pragma will perform new optimizations in future releases.
  **
  ** The optional argument is a bitmask of optimizations to perform:
  **
  **    0x00001    Debugging mode.  Do not actually perform any optimizations
  **               but instead return one line of text for each optimization
  **               that would have been done.  Off by default.
  **
  **    0x00002    Run ANALYZE on tables that might benefit.  On by default.
  **               See below for additional information.
  **
  **    0x00010    Run all ANALYZE operations using an analysis_limit that
  **               is the lessor of the current analysis_limit and the
  **               SQLITE_DEFAULT_OPTIMIZE_LIMIT compile-time option.
  **               The default value of SQLITE_DEFAULT_OPTIMIZE_LIMIT is
  **               currently (2024-02-19) set to 2000, which is such that
  **               the worst case run-time for PRAGMA optimize on a 100MB
  **               database will usually be less than 100 milliseconds on
  **               a RaspberryPI-4 class machine.  On by default.
  **
  **    0x10000    Look at tables to see if they need to be reanalyzed
  **               due to growth or shrinkage even if they have not been
  **               queried during the current connection.  Off by default.
  **
  ** The default MASK is and always shall be 0x0fffe.  In the current
  ** implementation, the default mask only covers the 0x00002 optimization,
  ** though additional optimizations that are covered by 0x0fffe might be
  ** added in the future.  Optimizations that are off by default and must
  ** be explicitly requested have masks of 0x10000 or greater.
  **
  ** DETERMINATION OF WHEN TO RUN ANALYZE
  **
  ** In the current implementation, a table is analyzed if only if all of
  ** the following are true:
  **
  ** (1) MASK bit 0x00002 is set.
  **
  ** (2) The table is an ordinary table, not a virtual table or view.
  **
  ** (3) The table name does not begin with "sqlite_".
  **
  ** (4) One or more of the following is true:
  **      (4a) The 0x10000 MASK bit is set.
  **      (4b) One or more indexes on the table lacks an entry
  **           in the sqlite_stat1 table.
  **      (4c) The query planner used sqlite_stat1-style statistics for one
  **           or more indexes of the table at some point during the lifetime
  **           of the current connection.
  **
  ** (5) One or more of the following is true:
  **      (5a) One or more indexes on the table lacks an entry
  **           in the sqlite_stat1 table.  (Same as 4a)
  **      (5b) The number of rows in the table has increased or decreased by
  **           10-fold.  In other words, the current size of the table is
  **           10 times larger than the size in sqlite_stat1 or else the
  **           current size is less than 1/10th the size in sqlite_stat1.
  **
  ** The rules for when tables are analyzed are likely to change in
  ** future releases.  Future versions of SQLite might accept a string
  ** literal argument to this pragma that contains a mnemonic description
  ** of the options rather than a bitmap.
  */
  അവസ്ഥ PragTyp_OPTIMIZE: {
    പൂർണ്ണസംഖ്യ iDbLast;           /* Loop termination point for the schema loop */
    പൂർണ്ണസംഖ്യ iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    Index *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    അക്ഷരം *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    പൂർണ്ണസംഖ്യ nLimit;            /* Analysis limit to use */
    പൂർണ്ണസംഖ്യ nCheck = 0;        /* Number of tables to be optimized */
    പൂർണ്ണസംഖ്യ nBtree = 0;        /* Number of btrees to scan */
    പൂർണ്ണസംഖ്യ nIndex;            /* Number of indexes on the current table */

    എങ്കിൽ( z_വലത് ){
      opMask = (u32)sqlite3Atoi(z_വലത്);
      എങ്കിൽ( (opMask & 0x02)==0 ) നിർത്തുക;
    }അല്ലെങ്കിൽ{
      opMask = 0xfffe;
    }
    എങ്കിൽ( (opMask & 0x10)==0 ){
      nLimit = 0;
    }അല്ലെങ്കിൽ എങ്കിൽ( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }അല്ലെങ്കിൽ{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    വേണ്ടി(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      എങ്കിൽ( iDb==1 ) തുടരുക;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      വേണ്ടി(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
        pTab = (Table*)sqliteHashData(k);

        /* This only works for ordinary tables */
        എങ്കിൽ( !IsOrdinaryTable(pTab) ) തുടരുക;

        /* Do not scan system tables */
        എങ്കിൽ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) തുടരുക;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        nIndex = 0;
        വേണ്ടി(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          nIndex++;
          എങ്കിൽ( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        എങ്കിൽ( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }അല്ലെങ്കിൽ എങ്കിൽ( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }അല്ലെങ്കിൽ എങ്കിൽ( pTab->pIndex!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }അല്ലെങ്കിൽ{
          /* Otherwise, we can skip this table */
          തുടരുക;
        }

        nCheck++;
        എങ്കിൽ( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += nIndex+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        എങ്കിൽ( szThreshold>=0 ){
          സ്ഥിരം LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_IfSizeBetween, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1),
                         szThreshold>=iRange ? szThreshold-iRange : -1,
                         szThreshold+iRange);
          VdbeCoverage(v);
        }അല്ലെങ്കിൽ{
          sqlite3VdbeAddOp2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        എങ്കിൽ( opMask & 0x01 ){
          പൂർണ്ണസംഖ്യ r1 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ResultRow, r1, 1);
        }അല്ലെങ്കിൽ{
          sqlite3VdbeAddOp4(v, OP_SqlExec, nLimit ? 0x02 : 00, nLimit, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    എങ്കിൽ( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      പൂർണ്ണസംഖ്യ iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      എങ്കിൽ( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      വേണ്ടി(iAddr=0; iAddr<iEnd; iAddr++){
        എങ്കിൽ( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    നിർത്തുക;
  }

  /*
  **   PRAGMA busy_timeout
  **   PRAGMA busy_timeout = N
  **
  ** Call sqlite3_busy_timeout(db, N).  Return the current timeout value
  ** if one is set.  If no busy handler or a different busy handler is set
  ** then 0 is returned.  Setting the busy_timeout to 0 or negative
  ** disables the timeout.
  */
  /*case PragTyp_BUSY_TIMEOUT*/ സ്ഥിരസ്ഥിതി: {
    പൂർണ്ണസംഖ്യ n = 0;
    പ്രതിജ്ഞാ( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    എങ്കിൽ( z_വലത് && sqlite3GetInt32(z_വലത്,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    നിർത്തുക;
  }

  /*
  **   PRAGMA soft_heap_limit
  **   PRAGMA soft_heap_limit = N
  **
  ** IMPLEMENTATION-OF: R-26343-45930 This pragma invokes the
  ** sqlite3_soft_heap_limit64() interface with the argument N, if N is
  ** specified and is a non-negative integer.
  ** IMPLEMENTATION-OF: R-64451-07163 The soft_heap_limit pragma always
  ** returns the same integer that would be returned by the
  ** sqlite3_soft_heap_limit64(-1) C-language function.
  */
  അവസ്ഥ PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    എങ്കിൽ( z_വലത് && sqlite3DecOrHexToI64(z_വലത്, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    നിർത്തുക;
  }

  /*
  **   PRAGMA hard_heap_limit
  **   PRAGMA hard_heap_limit = N
  **
  ** Invoke sqlite3_hard_heap_limit64() to query or set the hard heap
  ** limit.  The hard heap limit can be activated or lowered by this
  ** pragma, but not raised or deactivated.  Only the
  ** sqlite3_hard_heap_limit64() C-language API can raise or deactivate
  ** the hard heap limit.  This allows an application to set a heap limit
  ** constraint that cannot be relaxed by an untrusted SQL script.
  */
  അവസ്ഥ PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    എങ്കിൽ( z_വലത് && sqlite3DecOrHexToI64(z_വലത്, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      എങ്കിൽ( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    നിർത്തുക;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  അവസ്ഥ PragTyp_THREADS: {
    sqlite3_int64 N;
    എങ്കിൽ( z_വലത്
     && sqlite3DecOrHexToI64(z_വലത്, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (പൂർണ്ണസംഖ്യ)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    നിർത്തുക;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  അവസ്ഥ PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    എങ്കിൽ( z_വലത്
     && sqlite3DecOrHexToI64(z_വലത്, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (പൂർണ്ണസംഖ്യ)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    നിർത്തുക;
  }

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  അവസ്ഥ PragTyp_LOCK_STATUS: {
    സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    പൂർണ്ണസംഖ്യ i;
    pParse->nMem = 2;
    വേണ്ടി(i=0; i<db->nDb; i++){
      Btree *pBt;
      സ്ഥിരം അക്ഷരം *zState = "unknown";
      പൂർണ്ണസംഖ്യ j;
      എങ്കിൽ( db->aDb[i].zDbSName==0 ) തുടരുക;
      pBt = db->aDb[i].pBt;
      എങ്കിൽ( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    നിർത്തുക;
  }
#endif

#എങ്കിൽ defined(SQLITE_ENABLE_CEROD)
  അവസ്ഥ PragTyp_ACTIVATE_EXTENSIONS: എങ്കിൽ( z_വലത് ){
    എങ്കിൽ( sqlite3StrNICmp(z_വലത്, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_വലത്[6]);
    }
  }
  നിർത്തുക;
#endif

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  എങ്കിൽ( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_വലത് ){
    sqlite3VdbeVerifyNoResultRow(v);
  }

pragma_out:
  sqlite3DbFree(db, z_ഇടത്);
  sqlite3DbFree(db, z_വലത്);
}
#ifndef SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
തരനിർവചനം ഘടന PragmaVtab PragmaVtab;
തരനിർവചനം ഘടന PragmaVtabCursor PragmaVtabCursor;
ഘടന PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  സ്ഥിരം PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
ഘടന PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  അക്ഷരം *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  സ്ഥിരം PragmaName *pPragma = (സ്ഥിരം PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ i, j;
  അക്ഷരം cSep = '(';
  StrAccum acc;
  അക്ഷരം zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, വലുപ്പം(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  വേണ്ടി(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  എങ്കിൽ( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  എങ്കിൽ( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  എങ്കിൽ( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  പ്രതിജ്ഞാ( strlen(zBuf) < വലുപ്പം(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  എങ്കിൽ( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(വലുപ്പം(PragmaVtab));
    എങ്കിൽ( pTab==0 ){
      rc = SQLITE_NOMEM;
    }അല്ലെങ്കിൽ{
      memset(pTab, 0, വലുപ്പം(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }അല്ലെങ്കിൽ{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  തിരികെ rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  തിരികെ SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  സ്ഥിരം ഘടന sqlite3_index_constraint *pConstraint;
  പൂർണ്ണസംഖ്യ i, j;
  പൂർണ്ണസംഖ്യ seen[2];

  pIdxInfo->estimatedCost = (ഇരട്ട)1;
  എങ്കിൽ( pTab->nHidden==0 ){ തിരികെ SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  വേണ്ടി(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    എങ്കിൽ( pConstraint->iColumn < pTab->iHidden ) തുടരുക;
    എങ്കിൽ( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) തുടരുക;
    എങ്കിൽ( pConstraint->usable==0 ) തിരികെ SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    പ്രതിജ്ഞാ( j < 2 );
    seen[j] = i+1;
  }
  എങ്കിൽ( seen[0]==0 ){
    pIdxInfo->estimatedCost = (ഇരട്ട)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    തിരികെ SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argvIndex = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (ഇരട്ട)20;
  pIdxInfo->estimatedRows = 20;
  എങ്കിൽ( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argvIndex = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  തിരികെ SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(വലുപ്പം(*pCsr));
  എങ്കിൽ( pCsr==0 ) തിരികെ SQLITE_NOMEM;
  memset(pCsr, 0, വലുപ്പം(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  തിരികെ SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
സ്ഥിതം ശൂന്യം pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  പൂർണ്ണസംഖ്യ i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  വേണ്ടി(i=0; i<ArraySize(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  തിരികെ SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabNext(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  പ്രതിജ്ഞാ( pCsr->pPragma );
  എങ്കിൽ( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  തിരികെ rc;
}

/*
** Pragma virtual table module xFilter method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ i, j;
  StrAccum acc;
  അക്ഷരം *zSql;

  UNUSED_PARAMETER(idx_അക്കം);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  വേണ്ടി(i=0; i<argc; i++, j++){
    സ്ഥിരം അക്ഷരം *zText = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
    പ്രതിജ്ഞാ( j<ArraySize(pCsr->azArg) );
    പ്രതിജ്ഞാ( pCsr->azArg[j]==0 );
    എങ്കിൽ( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      എങ്കിൽ( pCsr->azArg[j]==0 ){
        തിരികെ SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_LENGTH]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  എങ്കിൽ( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  എങ്കിൽ( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  എങ്കിൽ( zSql==0 ) തിരികെ SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  എങ്കിൽ( rc!=SQLITE_OK ){
    pTab->base.z_Err_സന്ദേശം = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    തിരികെ rc;
  }
  തിരികെ pragmaVtabNext(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  തിരികെ (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  പൂർണ്ണസംഖ്യ i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  എങ്കിൽ( i<pTab->iHidden ){
    sqlite3_result_മൂല്യം(ctx, sqlite3_column_മൂല്യം(pCsr->pPragma, i));
  }അല്ലെങ്കിൽ{
    sqlite3_result_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  തിരികെ SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  തിരികെ SQLITE_OK;
}

/* The pragma virtual table object */
സ്ഥിതം സ്ഥിരം sqlite3_module pragmaVtabModule = {
  0,                           /* iVersion */
  0,                           /* xCreate - create a table */
  pragmaVtabConnect,           /* xConnect - connect to an existing table */
  pragmaVtabBestIndex,         /* xBestIndex - Determine search strategy */
  pragmaVtabDisconnect,        /* xDisconnect - Disconnect from a table */
  0,                           /* xDestroy - Drop a table */
  pragmaVtabOpen,              /* xOpen - open a cursor */
  pragmaVtabClose,             /* xClose - close a cursor */
  pragmaVtabFilter,            /* xFilter - configure scan constraints */
  pragmaVtabNext,              /* xNext - advance a cursor */
  pragmaVtabEof,               /* xEof */
  pragmaVtabColumn,            /* xColumn - read data */
  pragmaVtabRowid,             /* xRowid - read data */
  0,                           /* xUpdate - write data */
  0,                           /* xBegin - begin transaction */
  0,                           /* xSync - sync transaction */
  0,                           /* xCommit - commit transaction */
  0,                           /* xRollback - rollback transaction */
  0,                           /* xFindFunction - function overloading */
  0,                           /* xRename - rename the table */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};

/*
** Check to see if zTabName is really the name of a pragma.  If it is,
** then register an eponymous virtual table for that pragma and return
** a pointer to the Module object for the new virtual table.
*/
Module *sqlite3PragmaVtabRegister(sqlite3 *db, സ്ഥിരം അക്ഷരം *zName){
  സ്ഥിരം PragmaName *pName;
  പ്രതിജ്ഞാ( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  എങ്കിൽ( pName==0 ) തിരികെ 0;
  എങ്കിൽ( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) തിരികെ 0;
  പ്രതിജ്ഞാ( sqlite3HashFind(&db->aModule, zName)==0 );
  തിരികെ sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (ശൂന്യം*)pName, 0);
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */

#endif /* SQLITE_OMIT_PRAGMA */
