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
#அந்தர்பாவ "sqliteInt.h"

#எனில் !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  எனில் defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  இல்லையெனில்
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
#அந்தர்பாவ "pragma.h"

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
நிலையான u8 getSafetyLevel(மாறிலி எழுத்து *z, முழு omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  நிலையான மாறிலி எழுத்து zText[] = "onoffalseyestruextrafull";
  நிலையான மாறிலி u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  நிலையான மாறிலி u8 iLength[] = {2, 2, 3,  5,    3,   4,   5,    4};
  நிலையான மாறிலி u8 i_மதிப்பு[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  முழு i, n;
  எனில்( sqlite3Isdigit(*z) ){
    திரும்பு sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  க்கு(i=0; i<ArraySize(iLength); i++){
    எனில்( iLength[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || i_மதிப்பு[i]<=1)
    ){
      திரும்பு i_மதிப்பு[i];
    }
  }
  திரும்பு dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(மாறிலி எழுத்து *z, u8 dflt){
  திரும்பு getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#எனில் !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
நிலையான முழு getLockingMode(மாறிலி எழுத்து *z){
  எனில்( z ){
    எனில்( 0==sqlite3StrICmp(z, "exclusive") ) திரும்பு PAGER_LOCKINGMODE_EXCLUSIVE;
    எனில்( 0==sqlite3StrICmp(z, "normal") ) திரும்பு PAGER_LOCKINGMODE_NORMAL;
  }
  திரும்பு PAGER_LOCKINGMODE_QUERY;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
நிலையான முழு getAutoVacuum(மாறிலி எழுத்து *z){
  முழு i;
  எனில்( 0==sqlite3StrICmp(z, "none") ) திரும்பு BTREE_AUTOVACUUM_NONE;
  எனில்( 0==sqlite3StrICmp(z, "full") ) திரும்பு BTREE_AUTOVACUUM_FULL;
  எனில்( 0==sqlite3StrICmp(z, "incremental") ) திரும்பு BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  திரும்பு (u8)((i>=0&&i<=2)?i:0);
}
#endif /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
நிலையான முழு getTempStore(மாறிலி எழுத்து *z){
  எனில்( z[0]>='0' && z[0]<='2' ){
    திரும்பு z[0] - '0';
  }இல்லையெனில் எனில்( sqlite3StrICmp(z, "file")==0 ){
    திரும்பு 1;
  }இல்லையெனில் எனில்( sqlite3StrICmp(z, "memory")==0 ){
    திரும்பு 2;
  }இல்லையெனில்{
    திரும்பு 0;
  }
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
நிலையான முழு invalidateTempStorage(Parse *pParse){
  sqlite3 *db = pParse->db;
  எனில்( db->aDb[1].pBt!=0 ){
    எனில்( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_செய்தி(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      திரும்பு SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  திரும்பு SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
நிலையான முழு changeTempStorage(Parse *pParse, மாறிலி எழுத்து *zStorageType){
  முழு ts = getTempStore(zStorageType);
  sqlite3 *db = pParse->db;
  எனில்( db->temp_store==ts ) திரும்பு SQLITE_OK;
  எனில்( invalidateTempStorage( pParse ) != SQLITE_OK ){
    திரும்பு SQLITE_ERROR;
  }
  db->temp_store = (u8)ts;
  திரும்பு SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
நிலையான வெறுமை setPragmaResultColumnNames(
  Vdbe *v,                     /* The query under construction */
  மாறிலி PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_Set_இலக்கம்_Cols(v, n==0 ? 1 : n);
  எனில்( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }இல்லையெனில்{
    முழு i, j;
    க்கு(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
நிலையான வெறுமை returnSingleInt(Vdbe *v, i64 மதிப்பு){
  sqlite3VdbeAddInt64(v, 1, மதிப்பு);
  sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
}

/*
** Generate code to return a single text value.
*/
நிலையான வெறுமை returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  மாறிலி எழுத்து *z_மதிப்பு      /* Value to be returned */
){
  எனில்( z_மதிப்பு ){
    sqlite3VdbeLoadString(v, 1, (மாறிலி எழுத்து*)z_மதிப்பு);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
நிலையான வெறுமை setAllPagerFlags(sqlite3 *db){
  எனில்( db->autoCommit ){
    Db *pDb = db->aDb;
    முழு n = db->nDb;
    ப்ரதிஜ்ஞா( SQLITE_FullFSync==PAGER_FULLFSYNC );
    ப்ரதிஜ்ஞா( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    ப்ரதிஜ்ஞா( SQLITE_CacheSpill==PAGER_CACHESPILL );
    ப்ரதிஜ்ஞா( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    ப்ரதிஜ்ஞா( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    போது( (n--) > 0 ){
      எனில்( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#இல்லையெனில்
# define setAllPagerFlags(X)  /* no-op */
#endif


/*
** Return a human-readable name for a constraint resolution action.
*/
#ifndef SQLITE_OMIT_FOREIGN_சாவி
நிலையான மாறிலி எழுத்து *actionName(u8 action){
  மாறிலி எழுத்து *zName;
  தேர்வு( action ){
    நிலை OE_SetNull:  zName = "SET NULL";        நிறுத்து;
    நிலை OE_SetDflt:  zName = "SET DEFAULT";     நிறுத்து;
    நிலை OE_Cascade:  zName = "CASCADE";         நிறுத்து;
    நிலை OE_Restrict: zName = "RESTRICT";        நிறுத்து;
    இயல்பு:          zName = "NO ACTION"; 
                      ப்ரதிஜ்ஞா( action==OE_None ); நிறுத்து;
  }
  திரும்பு zName;
}
#endif


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
மாறிலி எழுத்து *sqlite3JournalModename(முழு eMode){
  நிலையான எழுத்து * மாறிலி azModeName[] = {
    "delete", "persist", "off", "truncate", "memory"
#ifndef SQLITE_OMIT_WAL
     , "wal"
#endif
  };
  ப்ரதிஜ்ஞா( PAGER_JOURNALMODE_DELETE==0 );
  ப்ரதிஜ்ஞா( PAGER_JOURNALMODE_PERSIST==1 );
  ப்ரதிஜ்ஞா( PAGER_JOURNALMODE_OFF==2 );
  ப்ரதிஜ்ஞா( PAGER_JOURNALMODE_TRUNCATE==3 );
  ப்ரதிஜ்ஞா( PAGER_JOURNALMODE_MEMORY==4 );
  ப்ரதிஜ்ஞா( PAGER_JOURNALMODE_WAL==5 );
  ப்ரதிஜ்ஞா( eMode>=0 && eMode<=ArraySize(azModeName) );

  எனில்( eMode==ArraySize(azModeName) ) திரும்பு 0;
  திரும்பு azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
நிலையான மாறிலி PragmaName *pragmaLocate(மாறிலி எழுத்து *zName){
  முழு upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = ArraySize(aPragmaName)-1;
  போது( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    எனில்( rc==0 ) நிறுத்து;
    எனில்( rc<0 ){
      upr = mid - 1;
    }இல்லையெனில்{
      lwr = mid + 1;
    }
  }
  திரும்பு lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
நிலையான வெறுமை pragma_Funclist_வரி(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  முழு isBuiltin,         /* True if this is a built-in function */
  முழு showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  எனில்( showInternFuncs ) mask = 0xffffffff;
  க்கு(; p; p=p->pNext){
    மாறிலி எழுத்து *zType;
    நிலையான மாறிலி எழுத்து *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    ப்ரதிஜ்ஞா( SQLITE_FUNC_ENCMASK==0x3 );
    ப்ரதிஜ்ஞா( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    ப்ரதிஜ்ஞா( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    ப்ரதிஜ்ஞா( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    எனில்( p->xSFunc==0 ) தொடர்;
    எனில்( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      தொடர்;
    }   
    எனில்( p->x_மதிப்பு!=0 ){
      zType = "w";
    }இல்லையெனில் எனில்( p->xFinalize!=0 ){
      zType = "a";
    }இல்லையெனில்{
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
நிலையான முழு integrityCheckResultRow(Vdbe *v){
  முழு addr;
  sqlite3VdbeAddOp2(v, OP_ResultRow, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  திரும்பு addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
நிலையான முழு tableSkipIntegrityCheck(மாறிலி Table *pTab, மாறிலி Table *pObjTab){
  எனில்( pObjTab ){
    திரும்பு pTab!=pObjTab;
  }இல்லையெனில்{
    திரும்பு (pTab->tabFlags & TF_Imposter)!=0;
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
வெறுமை sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_மதிப்பு,      /* Token for <value>, or NULL */
  முழு minusFlag       /* True if a '-' sign preceded <value> */
){
  எழுத்து *z_இடது = 0;       /* Nul-terminated UTF-8 string <id> */
  எழுத்து *z_வலது = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  மாறிலி எழுத்து *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  எழுத்து *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  முழு iDb;               /* Database index for <database> */
  முழு rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  மாறிலி PragmaName *pPragma;   /* The pragma */

  எனில்( v==0 ) திரும்பு;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  எனில்( iDb<0 ) திரும்பு;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  எனில்( iDb==1 && sqlite3OpenTempDatabase(pParse) ){
    திரும்பு;
  }

  z_இடது = sqlite3NameFromToken(db, pId);
  எனில்( !z_இடது ) திரும்பு;
  எனில்( minusFlag ){
    z_வலது = sqlite3MPrintf(db, "-%T", p_மதிப்பு);
  }இல்லையெனில்{
    z_வலது = sqlite3NameFromToken(db, p_மதிப்பு);
  }

  ப்ரதிஜ்ஞா( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  எனில்( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_இடது, z_வலது, zDb) ){
    செல் pragma_out;
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
  aFcntl[1] = z_இடது;
  aFcntl[2] = z_வலது;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (வெறுமை*)aFcntl);
  எனில்( rc==SQLITE_OK ){
    sqlite3_Vdbe_Set_இலக்கம்_Cols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    செல் pragma_out;
  }
  எனில்( rc!=SQLITE_NOTFOUND ){
    எனில்( aFcntl[0] ){
      sqlite3_Error_செய்தி(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    செல் pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_இடது);
  எனில்( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    செல் pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  எனில்( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    எனில்( sqlite3ReadSchema(pParse) ) செல் pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  எனில்( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_வலது==0)
  ){
    setPragmaResultColumnNames(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  தேர்வு( pPragma->ePragTyp ){
 
#எனில் !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  நிலை PragTyp_DEFAULT_CACHE_SIZE: {
    நிலையான மாறிலி முழு iLn = VDBE_OFFSET_LINENO(2);
    நிலையான மாறிலி VdbeOpList getCacheSize[] = {
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
    எனில்( !z_வலது ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(getCacheSize));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(getCacheSize), getCacheSize, iLn);
      எனில்( ONLY_IF_REALLOC_STRESS(aOp==0) ) நிறுத்து;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_SIZE;
    }இல்லையெனில்{
      முழு size = sqlite3AbsInt32(sqlite3Atoi(z_வலது));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_SIZE, size);
      ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    நிறுத்து;
  }
#endif /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#எனில் !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  நிலை PragTyp_PAGE_SIZE: {
    Btree *pBt = pDb->pBt;
    ப்ரதிஜ்ஞா( pBt!=0 );
    எனில்( !z_வலது ){
      முழு size = ALWAYS(pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
      returnSingleInt(v, size);
    }இல்லையெனில்{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->nextPagesize = sqlite3Atoi(z_வலது);
      எனில்( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    நிறுத்து;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  நிலை PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    முழு b = -1;
    ப்ரதிஜ்ஞா( pBt!=0 );
    எனில்( z_வலது ){
      எனில்( sqlite3_stricmp(z_வலது, "fast")==0 ){
        b = 2;
      }இல்லையெனில்{
        b = sqlite3GetBoolean(z_வலது, 0);
      }
    }
    எனில்( pId2->n==0 && b>=0 ){
      முழு ii;
      க்கு(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    நிறுத்து;
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
  நிலை Prag_Typ_PAGE_மொத்தம்: {
    முழு iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    எனில்( sqlite3Tolower(z_இடது[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }இல்லையெனில்{
      எனில்( z_வலது && sqlite3DecOrHexToI64(z_வலது,&x)==0 ){
        எனில்( x<0 ) x = 0;
        இல்லையெனில் எனில்( x>0xfffffffe ) x = 0xfffffffe;
      }இல்லையெனில்{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (முழு)x);
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, iReg, 1);
    நிறுத்து;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  நிலை PragTyp_LOCKING_MODE: {
    மாறிலி எழுத்து *zRet = "normal";
    முழு eMode = getLockingMode(z_வலது);

    எனில்( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }இல்லையெனில்{
      Pager *pPager;
      எனில்( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        முழு ii;
        ப்ரதிஜ்ஞா(pDb==&db->aDb[0]);
        க்கு(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }

    ப்ரதிஜ்ஞா( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    எனில்( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    நிறுத்து;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  நிலை PragTyp_JOURNAL_MODE: {
    முழு eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    முழு ii;           /* Loop counter */

    எனில்( z_வலது==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }இல்லையெனில்{
      மாறிலி எழுத்து *zMode;
      முழு n = sqlite3Strlen30(z_வலது);
      க்கு(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        எனில்( sqlite3StrNICmp(z_வலது, zMode, n)==0 ) நிறுத்து;
      }
      எனில்( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      எனில்( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    எனில்( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    க்கு(ii=db->nDb-1; ii>=0; ii--){
      எனில்( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    நிறுத்து;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  நிலை PragTyp_JOURNAL_SIZE_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    எனில்( z_வலது ){
      sqlite3DecOrHexToI64(z_வலது, &iLimit);
      எனில்( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    நிறுத்து;
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
  நிலை PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    ப்ரதிஜ்ஞா( pBt!=0 );
    எனில்( !z_வலது ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }இல்லையெனில்{
      முழு eAuto = getAutoVacuum(z_வலது);
      ப்ரதிஜ்ஞா( eAuto>=0 && eAuto<=2 );
      db->nextAutovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      எனில்( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        நிலையான மாறிலி முழு iLn = VDBE_OFFSET_LINENO(2);
        நிலையான மாறிலி VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_வேர்_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        முழு iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, ArraySize(setMeta6), setMeta6, iLn);
        எனில்( ONLY_IF_REALLOC_STRESS(aOp==0) ) நிறுத்து;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    நிறுத்து;
  }
#endif

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  நிலை PragTyp_INCREMENTAL_VACUUM: {
    முழு iLimit = 0, addr;
    எனில்( z_வலது==0 || !sqlite3GetInt32(z_வலது, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ResultRow, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    நிறுத்து;
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
  நிலை PragTyp_CACHE_SIZE: {
    ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, iDb, 0) );
    எனில்( !z_வலது ){
      returnSingleInt(v, pDb->pSchema->cache_size);
    }இல்லையெனில்{
      முழு size = sqlite3Atoi(z_வலது);
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    நிறுத்து;
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
  நிலை PragTyp_CACHE_SPILL: {
    ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, iDb, 0) );
    எனில்( !z_வலது ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3BtreeSetSpillSize(pDb->pBt,0));
    }இல்லையெனில்{
      முழு size = 1;
      எனில்( sqlite3GetInt32(z_வலது, &size) ){
        sqlite3BtreeSetSpillSize(pDb->pBt, size);
      }
      எனில்( sqlite3GetBoolean(z_வலது, size!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }இல்லையெனில்{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    நிறுத்து;
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
  நிலை PragTyp_MMAP_SIZE: {
    sqlite3_int64 sz;
#எனில் SQLITE_MAX_MMAP_SIZE>0
    ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, iDb, 0) );
    எனில்( z_வலது ){
      முழு ii;
      sqlite3DecOrHexToI64(z_வலது, &sz);
      எனில்( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      எனில்( pId2->n==0 ) db->szMmap = sz;
      க்கு(ii=db->nDb-1; ii>=0; ii--){
        எனில்( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);
#இல்லையெனில்
    sz = 0;
    rc = SQLITE_OK;
#endif
    எனில்( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }இல்லையெனில் எனில்( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    நிறுத்து;
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
  நிலை PragTyp_TEMP_STORE: {
    எனில்( !z_வலது ){
      returnSingleInt(v, db->temp_store);
    }இல்லையெனில்{
      changeTempStorage(pParse, z_வலது);
    }
    நிறுத்து;
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
  நிலை PragTyp_TEMP_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    எனில்( !z_வலது ){
      returnSingleText(v, sqlite3_temp_directory);
    }இல்லையெனில்{
#ifndef SQLITE_OMIT_WSD
      எனில்( z_வலது[0] ){
        முழு res;
        rc = sqlite3OsAccess(db->pVfs, z_வலது, SQLITE_ACCESS_READWRITE, &res);
        எனில்( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_செய்தி(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          செல் pragma_out;
        }
      }
      எனில்( SQLITE_TEMP_STORE==0
       || (SQLITE_TEMP_STORE==1 && db->temp_store<=1)
       || (SQLITE_TEMP_STORE==2 && db->temp_store==1)
      ){
        invalidateTempStorage(pParse);
      }
      sqlite3_free(sqlite3_temp_directory);
      எனில்( z_வலது[0] ){
        sqlite3_temp_directory = sqlite3_mprintf("%s", z_வலது);
      }இல்லையெனில்{
        sqlite3_temp_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    நிறுத்து;
  }

#எனில் SQLITE_OS_WIN
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
  நிலை PragTyp_DATA_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    எனில்( !z_வலது ){
      returnSingleText(v, sqlite3_data_directory);
    }இல்லையெனில்{
#ifndef SQLITE_OMIT_WSD
      எனில்( z_வலது[0] ){
        முழு res;
        rc = sqlite3OsAccess(db->pVfs, z_வலது, SQLITE_ACCESS_READWRITE, &res);
        எனில்( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_செய்தி(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          செல் pragma_out;
        }
      }
      sqlite3_free(sqlite3_data_directory);
      எனில்( z_வலது[0] ){
        sqlite3_data_directory = sqlite3_mprintf("%s", z_வலது);
      }இல்லையெனில்{
        sqlite3_data_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    நிறுத்து;
  }
#endif

#எனில் SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  நிலை PragTyp_LOCK_PROXY_FILE: {
    எனில்( !z_வலது ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      எழுத்து *proxy_file_path = வெற்று;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }இல்லையெனில்{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      முழு res;
      எனில்( z_வலது[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     z_வலது);
      } இல்லையெனில் {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     வெற்று);
      }
      எனில்( res!=SQLITE_OK ){
        sqlite3_Error_செய்தி(pParse, "failed to set lock proxy file");
        செல் pragma_out;
      }
    }
    நிறுத்து;
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
  நிலை PragTyp_SYNCHRONOUS: {
    எனில்( !z_வலது ){
      returnSingleInt(v, pDb->safety_level-1);
    }இல்லையெனில்{
      எனில்( !db->autoCommit ){
        sqlite3_Error_செய்தி(pParse,
            "Safety level may not be changed inside a transaction");
      }இல்லையெனில் எனில்( iDb!=1 ){
        முழு iLevel;
        எனில்( sqlite3Isdigit(z_வலது[0]) ){
          iLevel = sqlite3Atoi(z_வலது);
        }இல்லையெனில்{
          iLevel = getSafetyLevel(z_வலது,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        எனில்( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    நிறுத்து;
  }
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  நிலை PragTyp_FLAG: {
    எனில்( z_வலது==0 ){
      setPragmaResultColumnNames(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }இல்லையெனில்{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      எனில்( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_Foreign_சாவிகள்);
      }

      எனில்( sqlite3GetBoolean(z_வலது, 0) ){
        எனில்( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }இல்லையெனில்{
        db->flags &= ~mask;
        எனில்( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        எனில்( (mask & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_வலது, "reset")==0
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
    நிறுத்து;
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
  நிலை PragTyp_TABLE_INFO: எனில்( z_வலது ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_வலது, zDb);
    எனில்( pTab ){
      முழு i, k;
      முழு nHidden = 0;
      Column *pCol;
      Index *pPk = sqlite3_Primary_சாவி_Index(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      க்கு(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        முழு isHidden = 0;
        மாறிலி கோவை *p_Col_கோவை;
        எனில்( pCol->colFlags & COLFLAG_NOINSERT ){
          எனில்( pPragma->iArg==0 ){
            nHidden++;
            தொடர்;
          }
          எனில்( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }இல்லையெனில் எனில்( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }இல்லையெனில்{ ப்ரதிஜ்ஞா( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        எனில்( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }இல்லையெனில் எனில்( pPk==0 ){
          k = 1;
        }இல்லையெனில்{
          க்கு(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        p_Col_கோவை = sqlite3_Column_கோவை(pTab,pCol);
        ப்ரதிஜ்ஞா( p_Col_கோவை==0 || p_Col_கோவை->op==TK_SPAN || isHidden>=2 );
        ப்ரதிஜ்ஞா( p_Col_கோவை==0 || !கோவை_Has_Property(p_Col_கோவை, EP_Int_மதிப்பு)
                  || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || p_Col_கோவை==0) ? 0 : p_Col_கோவை->u.zToken,
               k,
               isHidden);
      }
    }
  }
  நிறுத்து;

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
  நிலை PragTyp_TABLE_LIST: {
    முழு ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    க்கு(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      முழு initNCol;
      எனில்( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) தொடர்;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_மொத்தம்(pHash);
      போது( initNCol-- ){
        க்கு(k=sqliteHashFirst(pHash); 1; k=sqliteHashNext(k) ){
          Table *pTab;
          எனில்( k==0 ){ initNCol = 0; நிறுத்து; }
          pTab = sqliteHashData(k);
          எனில்( pTab->nCol==0 ){
            எழுத்து *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            எனில்( zSql ){
              sqlite3_stmt *pDummy = 0;
              (வெறுமை)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (வெறுமை)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            எனில்( db->mallocFailed ){
              sqlite3_Error_செய்தி(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            நிறுத்து;
          }
        }
      }

      க்கு(k=sqliteHashFirst(pHash); k; k=sqliteHashNext(k) ){
        Table *pTab = sqliteHashData(k);
        மாறிலி எழுத்து *zType;
        எனில்( z_வலது && sqlite3_stricmp(z_வலது, pTab->zName)!=0 ) தொடர்;
        எனில்( IsView(pTab) ){
          zType = "view";
        }இல்லையெனில் எனில்( IsVirtual(pTab) ){
          zType = "virtual";
        }இல்லையெனில் எனில்( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }இல்லையெனில்{
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
  நிறுத்து;

#ifdef SQLITE_DEBUG
  நிலை PragTyp_STATS: {
    Index *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    க்கு(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqliteHashNext(i)){
      Table *pTab = sqliteHashData(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      க்கு(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 5);
      }
    }
  }
  நிறுத்து;
#endif

  நிலை PragTyp_INDEX_INFO: எனில்( z_வலது ){
    Index *pIdx;
    Table *pTab;
    pIdx = sqlite3FindIndex(db, z_வலது, zDb);
    எனில்( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_வலது, zDb);
      எனில்( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_சாவி_Index(pTab);
      }
    }
    எனில்( pIdx ){
      முழு iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
      முழு i;
      முழு mx;
      எனில்( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }இல்லையெனில்{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_சாவி_Col;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      ப்ரதிஜ்ஞா( pParse->nMem<=pPragma->nPragCName );
      க்கு(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        எனில்( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->n_சாவி_Col);
        }
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, pParse->nMem);
      }
    }
  }
  நிறுத்து;

  நிலை PragTyp_INDEX_LIST: எனில்( z_வலது ){
    Index *pIdx;
    Table *pTab;
    முழு i;
    pTab = sqlite3FindTable(db, z_வலது, zDb);
    எனில்( pTab ){
      முழு iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      க்கு(pIdx=pTab->pIndex, i=0; pIdx; pIdx=pIdx->pNext, i++){
        மாறிலி எழுத்து *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           IsUniqueIndex(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  நிறுத்து;

  நிலை PragTyp_DATABASE_LIST: {
    முழு i;
    pParse->nMem = 3;
    க்கு(i=0; i<db->nDb; i++){
      எனில்( db->aDb[i].pBt==0 ) தொடர்;
      ப்ரதிஜ்ஞா( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  நிறுத்து;

  நிலை PragTyp_COLLATION_LIST: {
    முழு i = 0;
    HashElem *p;
    pParse->nMem = 2;
    க்கு(p=sqliteHashFirst(&db->aCollSeq); p; p=sqliteHashNext(p)){
      CollSeq *pColl = (CollSeq *)sqliteHashData(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  நிறுத்து;

#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  நிலை PragTyp_FUNCTION_LIST: {
    முழு i;
    HashElem *j;
    FuncDef *p;
    முழு showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    க்கு(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      க்கு(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        ப்ரதிஜ்ஞா( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_வரி(v, p, 1, showInternFunc);
      }
    }
    க்கு(j=sqliteHashFirst(&db->aFunc); j; j=sqliteHashNext(j)){
      p = (FuncDef*)sqliteHashData(j);
      ப்ரதிஜ்ஞா( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_வரி(v, p, 0, showInternFunc);
    }
  }
  நிறுத்து;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  நிலை PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    க்கு(j=sqliteHashFirst(&db->aModule); j; j=sqliteHashNext(j)){
      Module *pMod = (Module*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  நிறுத்து;
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  நிலை PragTyp_PRAGMA_LIST: {
    முழு i;
    க்கு(i=0; i<ArraySize(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  நிறுத்து;
#endif /* SQLITE_INTROSPECTION_PRAGMAS */

#endif /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ifndef SQLITE_OMIT_FOREIGN_சாவி
  நிலை Prag_Typ_FOREIGN_சாவி_LIST: எனில்( z_வலது ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, z_வலது, zDb);
    எனில்( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      எனில்( pFK ){
        முழு iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        முழு i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        போது(pFK){
          முழு j;
          க்கு(j=0; j<pFK->nCol; j++){
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
  நிறுத்து;
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_FOREIGN_சாவி
#ifndef SQLITE_OMIT_TRIGGER
  நிலை Prag_Typ_FOREIGN_சாவி_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    Index *pIdx;           /* Index in the parent table */
    முழு i;                 /* Loop counter:  Foreign key number for pTab */
    முழு j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    முழு x;                 /* result variable */
    முழு regResult;         /* 3 registers to hold a result row */
    முழு regRow;            /* Registers to hold a row from pTab */
    முழு addr_உச்சி;           /* Top of a loop checking foreign keys */
    முழு addrOk;            /* Jump here if the key is OK */
    முழு *aiCols;           /* child to parent column mapping */

    regResult = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    போது( k ){
      எனில்( z_வலது ){
        pTab = sqlite3LocateTable(pParse, 0, z_வலது, zDb);
        k = 0;
      }இல்லையெனில்{
        pTab = (Table*)sqliteHashData(k);
        k = sqliteHashNext(k);
      }
      எனில்( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) தொடர்;
      iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, regResult, pTab->zName);
      ப்ரதிஜ்ஞா( IsOrdinaryTable(pTab) );
      க்கு(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        எனில்( pParent==0 ) தொடர்;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
        எனில்( x==0 ){
          எனில்( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }இல்லையெனில்{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_சாவி_Info(pParse, pIdx);
          }
        }இல்லையெனில்{
          k = 0;
          நிறுத்து;
        }
      }
      ப்ரதிஜ்ஞா( pParse->nErr>0 || pFK==0 );
      எனில்( pFK ) நிறுத்து;
      எனில்( pParse->nTab<i ) pParse->nTab = i;
      addr_உச்சி = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      ப்ரதிஜ்ஞா( IsOrdinaryTable(pTab) );
      க்கு(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        எனில்( pParent ){
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          ப்ரதிஜ்ஞா( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        க்கு(j=0; j<pFK->nCol; j++){
          முழு iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3_கோவை_Code_Get_Column_Of_Table(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        எனில்( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3IndexAffinityStr(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }இல்லையெனில் எனில்( pParent ){
          முழு jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          ப்ரதிஜ்ஞா( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        எனில்( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, regResult+1);
        }இல்லையெனில்{
          sqlite3VdbeAddOp2(v, OP_Null, 0, regResult+1);
        }
        sqlite3VdbeMultiLoad(v, regResult+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_Next, 0, addr_உச்சி+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addr_உச்சி);
    }
  }
  நிறுத்து;
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  நிலை PragTyp_CASE_SENSITIVE_LIKE: {
    எனில்( z_வலது ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(z_வலது, 0));
    }
  }
  நிறுத்து;
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
  நிலை PragTyp_INTEGRITY_CHECK: {
    முழு i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    முழு isQuick = (sqlite3Tolower(z_இடது[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    ப்ரதிஜ்ஞா( iDb>=0 );
    ப்ரதிஜ்ஞா( iDb==0 || pId2->z );
    எனில்( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    எனில்( z_வலது ){
      எனில்( sqlite3GetInt32(p_மதிப்பு->z, &mxErr) ){
        எனில்( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }இல்லையெனில்{
        pObjTab = sqlite3LocateTable(pParse, 0, z_வலது,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    க்கு(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      முழு *a_வேர்;      /* Array of root page numbers of all btrees */
      முழு cnt = 0;     /* Number of entries in aRoot[] */

      எனில்( OMIT_TEMPDB && i==1 ) தொடர்;
      எனில்( iDb>=0 && i!=iDb ) தொடர்;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      ப்ரதிஜ்ஞா( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      க்கு(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);  /* Current table */
        Index *pIdx;                      /* An index on pTab */
        எனில்( tableSkipIntegrityCheck(pTab,pObjTab) ) தொடர்;
        எனில்( HasRowid(pTab) ) cnt++;
        க்கு(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){ cnt++; }
      }
      எனில்( cnt==0 ) தொடர்;
      எனில்( pObjTab ) cnt++;
      a_வேர் = sqlite3DbMallocRawNN(db, அளவு(முழு)*(cnt+1));
      எனில்( a_வேர்==0 ) நிறுத்து;
      cnt = 0;
      எனில்( pObjTab ) a_வேர்[++cnt] = 0;
      க்கு(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        எனில்( tableSkipIntegrityCheck(pTab,pObjTab) ) தொடர்;
        எனில்( HasRowid(pTab) ) a_வேர்[++cnt] = pTab->tnum;
        க்கு(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          a_வேர்[++cnt] = pIdx->tnum;
        }
      }
      a_வேர்[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3ClearTempRegCache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (எழுத்து*)a_வேர்,P4_INTARRAY);
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
      க்கு(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        முழு iTab = 0;
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        எனில்( tableSkipIntegrityCheck(pTab,pObjTab) ) தொடர்;
        எனில்( HasRowid(pTab) ){
          iTab = cnt++;
        }இல்லையெனில்{
          iTab = cnt;
          க்கு(pIdx=pTab->pIndex; ALWAYS(pIdx); pIdx=pIdx->pNext){
            எனில்( Is_Primary_சாவி_Index(pIdx) ) நிறுத்து;
            iTab++;
          }
        }
        க்கு(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          எனில்( pIdx->pPartIdxWhere==0 ){
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
      க்கு(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx, *pPk;
        Index *pPrior = 0;      /* Previous index */
        முழு loop_உச்சி;
        முழு iDataCur, iIdxCur;
        முழு r1 = -1;
        முழு bStrict;            /* True for a STRICT table */
        முழு r2;                 /* Previous key for WITHOUT ROWID tables */
        முழு mxCol;              /* Maximum non-virtual column number */

        எனில்( tableSkipIntegrityCheck(pTab,pObjTab) ) தொடர்;
        எனில்( !IsOrdinaryTable(pTab) ) தொடர்;
        எனில்( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }இல்லையெனில்{
          pPk = sqlite3_Primary_சாவி_Index(pTab);
          r2 = sqlite3GetTempRange(pParse, pPk->n_சாவி_Col);
          sqlite3VdbeAddOp3(v, OP_Null, 1, r2, r2+pPk->n_சாவி_Col-1);
        }
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &iDataCur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        க்கு(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        ப்ரதிஜ்ஞா( pParse->nMem>=8+j );
        ப்ரதிஜ்ஞா( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, iDataCur, 0); VdbeCoverage(v);
        loop_உச்சி = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        ப்ரதிஜ்ஞா( !IsVirtual(pTab) );
        எனில்( HasRowid(pTab) ){
          mxCol = -1;
          க்கு(j=0; j<pTab->nCol; j++){
            எனில்( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          எனில்( mxCol==pTab->iPKey ) mxCol--;
        }இல்லையெனில்{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3_Primary_சாவி_Index(pTab)->nColumn-1;
        }
        எனில்( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, iDataCur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        எனில்( !isQuick ){
          எனில்( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            முழு a1;
            எழுத்து *zErr;
            a1 = sqlite3VdbeAddOp4Int(v, OP_IdxGT, iDataCur, 0,r2,pPk->n_சாவி_Col);
            VdbeCoverage(v);
            sqlite3VdbeAddOp1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, a1);
            sqlite3VdbeJumpHere(v, a1+1);
            க்கு(j=0; j<pPk->n_சாவி_Col; j++){
              sqlite3_கோவை_Code_Load_Index_Column(pParse, pPk, iDataCur, j, r2+j);
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
        க்கு(j=0; j<pTab->nCol; j++){
          எழுத்து *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          முழு labelError;               /* Jump here to report an error */
          முழு labelOk;                  /* Jump here if all looks ok */
          முழு p1, p3, p4;               /* Operands to the OP_IsType opcode */
          முழு doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          எனில்( j==pTab->iPKey ) தொடர்;
          எனில்( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }இல்லையெனில்{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          எனில்( pCol->notNull==0 && !doTypeCheck ) தொடர்;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          எனில்( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3_கோவை_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
            p1 = -1;
            p3 = 3;
          }இல்லையெனில்{
            எனில்( pCol->iDflt ){
              sqlite3_மதிப்பு *p_Dflt_மதிப்பு = 0;
              sqlite3_மதிப்பு_From_கோவை(db, sqlite3_Column_கோவை_Auth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &p_Dflt_மதிப்பு);
              எனில்( p_Dflt_மதிப்பு ){
                p4 = sqlite3_மதிப்பு_type(p_Dflt_மதிப்பு);
                sqlite3_மதிப்பு_Free(p_Dflt_மதிப்பு);
              }
            }
            p1 = iDataCur;
            எனில்( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3TableColumnToIndex(sqlite3_Primary_சாவி_Index(pTab), j);
            }இல்லையெனில்{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          எனில்( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            முழு jmp3;
            முழு jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            எனில்( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }இல்லையெனில்{
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
            எனில்( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }இல்லையெனில்{
              /* VDBE byte code will fall thru */
            }
          }
          எனில்( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            நிலையான unsigned எழுத்து aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            ப்ரதிஜ்ஞா( pCol->eCType>=1 && pCol->eCType<=அளவு(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }இல்லையெனில் எனில்( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }இல்லையெனில் எனில்( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            எனில்( p1>=0 ){
              sqlite3_கோவை_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
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
        எனில்( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          கோவை_List *pCheck = sqlite3_கோவை_List_Dup(db, pTab->pCheck, 0);
          எனில்( db->mallocFailed==0 ){
            முழு addrCkFault = sqlite3VdbeMakeLabel(pParse);
            முழு addrCkOk = sqlite3VdbeMakeLabel(pParse);
            எழுத்து *zErr;
            முழு k;
            pParse->iSelfTab = iDataCur + 1;
            க்கு(k=pCheck->n_கோவை-1; k>0; k--){
              sqlite3_கோவை_If_False(pParse, pCheck->a[k].p_கோவை, addrCkFault, 0);
            }
            sqlite3_கோவை_If_True(pParse, pCheck->a[0].p_கோவை, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_கோவை_List_Delete(db, pCheck);
        }
        எனில்( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          க்கு(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            முழு jmp2, jmp3, jmp4, jmp5, label6;
            முழு kk;
            முழு ckUniq = sqlite3VdbeMakeLabel(pParse);
            எனில்( pPk==pIdx ) தொடர்;
            r1 = sqlite3_Generate_Index_சாவி(pParse, pIdx, iDataCur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFind_சாவி, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (மாறிலி எழுத்து*)pIdx, P4_INDEX);
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
            எனில்( HasRowid(pTab) ){
              முழு jmp7;
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
            க்கு(kk=0; kk<pIdx->n_சாவி_Col; kk++){
              எனில்( pIdx->azColl[kk]==sqlite3_Str_இருமம் ) தொடர்;
              எனில்( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            எனில்( label6 ){
              முழு jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
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
            எனில்( IsUniqueIndex(pIdx) ){
              முழு uniqOk = sqlite3VdbeMakeLabel(pParse);
              முழு jmp6;
              க்கு(kk=0; kk<pIdx->n_சாவி_Col; kk++){
                முழு iCol = pIdx->aiColumn[kk];
                ப்ரதிஜ்ஞா( iCol!=XN_ROWID && iCol<pTab->nCol );
                எனில்( iCol>=0 && pTab->aCol[iCol].notNull ) தொடர்;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_Next, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_சாவி_Col); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_Next, iDataCur, loop_உச்சி); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loop_உச்சி-1);
        எனில்( pPk ){
          ப்ரதிஜ்ஞா( !isQuick );
          sqlite3ReleaseTempRange(pParse, r2, pPk->n_சாவி_Col);
        }
      }

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      க்கு(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        sqlite3_vtab *pVTab;
        முழு a1;
        எனில்( tableSkipIntegrityCheck(pTab,pObjTab) ) தொடர்;
        எனில்( IsOrdinaryTable(pTab) ) தொடர்;
        எனில்( !IsVirtual(pTab) ) தொடர்;
        எனில்( pTab->nCol<=0 ){
          மாறிலி எழுத்து *zMod = pTab->u.vtab.azArg[0];
          எனில்( sqlite3HashFind(&db->aModule, zMod)==0 ) தொடர்;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        எனில்( pTab->u.vtab.p==0 ) தொடர்;
        pVTab = pTab->u.vtab.p->pVtab;
        எனில்( NEVER(pVTab==0) ) தொடர்;
        எனில்( NEVER(pVTab->pModule==0) ) தொடர்;
        எனில்( pVTab->pModule->iVersion<4 ) தொடர்;
        எனில்( pVTab->pModule->xIntegrity==0 ) தொடர்;
        sqlite3VdbeAddOp3(v, OP_VCheck, i, 3, isQuick);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3VdbeAddOp1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrityCheckResultRow(v);
        sqlite3VdbeJumpHere(v, a1);
        தொடர்;
      }
#endif
    }
    {
      நிலையான மாறிலி முழு iLn = VDBE_OFFSET_LINENO(2);
      நிலையான மாறிலி VdbeOpList endCode[] = {
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
      எனில்( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (எழுத்து*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  நிறுத்து;
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
  நிலை PragTyp_ENCODING: {
    நிலையான மாறிலி கட்டமைப்பு EncName {
      எழுத்து *zName;
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
    மாறிலி கட்டமைப்பு EncName *pEnc;
    எனில்( !z_வலது ){    /* "PRAGMA encoding" */
      எனில்( sqlite3ReadSchema(pParse) ) செல் pragma_out;
      ப்ரதிஜ்ஞா( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      ப்ரதிஜ்ஞா( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      ப்ரதிஜ்ஞா( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }இல்லையெனில்{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      எனில்( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        க்கு(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          எனில்( 0==sqlite3StrICmp(z_வலது, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            நிறுத்து;
          }
        }
        எனில்( !pEnc->zName ){
          sqlite3_Error_செய்தி(pParse, "unsupported encoding: %s", z_வலது);
        }
      }
    }
  }
  நிறுத்து;
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
  நிலை Prag_Typ_HEADER_மதிப்பு: {
    முழு iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    எனில்( z_வலது && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      நிலையான மாறிலி VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(setCookie), setCookie, 0);
      எனில்( ONLY_IF_REALLOC_STRESS(aOp==0) ) நிறுத்து;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_வலது);
      aOp[1].p5 = 1;
      எனில்( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }இல்லையெனில்{
      /* Read the specified cookie value */
      நிலையான மாறிலி VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ResultRow,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(readCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(readCookie),readCookie,0);
      எனில்( ONLY_IF_REALLOC_STRESS(aOp==0) ) நிறுத்து;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  நிறுத்து;
#endif /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  நிலை PragTyp_COMPILE_OPTIONS: {
    முழு i = 0;
    மாறிலி எழுத்து *zOpt;
    pParse->nMem = 1;
    போது( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  நிறுத்து;
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ifndef SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  நிலை PragTyp_WAL_CHECKPOINT: {
    முழு iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    முழு eMode = SQLITE_CHECKPOINT_PASSIVE;
    எனில்( z_வலது ){
      எனில்( sqlite3StrICmp(z_வலது, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }இல்லையெனில் எனில்( sqlite3StrICmp(z_வலது, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }இல்லையெனில் எனில்( sqlite3StrICmp(z_வலது, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }இல்லையெனில் எனில்( sqlite3StrICmp(z_வலது, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 3);
  }
  நிறுத்து;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  நிலை PragTyp_WAL_AUTOCHECKPOINT: {
    எனில்( z_வலது ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_வலது));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  நிறுத்து;
#endif

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  நிலை PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    நிறுத்து;
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
  நிலை PragTyp_OPTIMIZE: {
    முழு iDbLast;           /* Loop termination point for the schema loop */
    முழு iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    Index *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    எழுத்து *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    முழு nLimit;            /* Analysis limit to use */
    முழு nCheck = 0;        /* Number of tables to be optimized */
    முழு nBtree = 0;        /* Number of btrees to scan */
    முழு nIndex;            /* Number of indexes on the current table */

    எனில்( z_வலது ){
      opMask = (u32)sqlite3Atoi(z_வலது);
      எனில்( (opMask & 0x02)==0 ) நிறுத்து;
    }இல்லையெனில்{
      opMask = 0xfffe;
    }
    எனில்( (opMask & 0x10)==0 ){
      nLimit = 0;
    }இல்லையெனில் எனில்( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }இல்லையெனில்{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    க்கு(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      எனில்( iDb==1 ) தொடர்;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      க்கு(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
        pTab = (Table*)sqliteHashData(k);

        /* This only works for ordinary tables */
        எனில்( !IsOrdinaryTable(pTab) ) தொடர்;

        /* Do not scan system tables */
        எனில்( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) தொடர்;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        nIndex = 0;
        க்கு(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          nIndex++;
          எனில்( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        எனில்( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }இல்லையெனில் எனில்( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }இல்லையெனில் எனில்( pTab->pIndex!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }இல்லையெனில்{
          /* Otherwise, we can skip this table */
          தொடர்;
        }

        nCheck++;
        எனில்( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += nIndex+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        எனில்( szThreshold>=0 ){
          மாறிலி LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_IfSizeBetween, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1),
                         szThreshold>=iRange ? szThreshold-iRange : -1,
                         szThreshold+iRange);
          VdbeCoverage(v);
        }இல்லையெனில்{
          sqlite3VdbeAddOp2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        எனில்( opMask & 0x01 ){
          முழு r1 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ResultRow, r1, 1);
        }இல்லையெனில்{
          sqlite3VdbeAddOp4(v, OP_SqlExec, nLimit ? 0x02 : 00, nLimit, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    எனில்( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      முழு iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      எனில்( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      க்கு(iAddr=0; iAddr<iEnd; iAddr++){
        எனில்( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    நிறுத்து;
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
  /*case PragTyp_BUSY_TIMEOUT*/ இயல்பு: {
    முழு n = 0;
    ப்ரதிஜ்ஞா( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    எனில்( z_வலது && sqlite3GetInt32(z_வலது,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    நிறுத்து;
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
  நிலை PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    எனில்( z_வலது && sqlite3DecOrHexToI64(z_வலது, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    நிறுத்து;
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
  நிலை PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    எனில்( z_வலது && sqlite3DecOrHexToI64(z_வலது, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      எனில்( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    நிறுத்து;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  நிலை PragTyp_THREADS: {
    sqlite3_int64 N;
    எனில்( z_வலது
     && sqlite3DecOrHexToI64(z_வலது, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (முழு)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    நிறுத்து;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  நிலை PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    எனில்( z_வலது
     && sqlite3DecOrHexToI64(z_வலது, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (முழு)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    நிறுத்து;
  }

#எனில் defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  நிலை PragTyp_LOCK_STATUS: {
    நிலையான மாறிலி எழுத்து *மாறிலி azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    முழு i;
    pParse->nMem = 2;
    க்கு(i=0; i<db->nDb; i++){
      Btree *pBt;
      மாறிலி எழுத்து *zState = "unknown";
      முழு j;
      எனில்( db->aDb[i].zDbSName==0 ) தொடர்;
      pBt = db->aDb[i].pBt;
      எனில்( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }இல்லையெனில் எனில்( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    நிறுத்து;
  }
#endif

#எனில் defined(SQLITE_ENABLE_CEROD)
  நிலை PragTyp_ACTIVATE_EXTENSIONS: எனில்( z_வலது ){
    எனில்( sqlite3StrNICmp(z_வலது, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_வலது[6]);
    }
  }
  நிறுத்து;
#endif

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  எனில்( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_வலது ){
    sqlite3VdbeVerifyNoResultRow(v);
  }

pragma_out:
  sqlite3DbFree(db, z_இடது);
  sqlite3DbFree(db, z_வலது);
}
#ifndef SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
வகைவரை கட்டமைப்பு PragmaVtab PragmaVtab;
வகைவரை கட்டமைப்பு PragmaVtabCursor PragmaVtabCursor;
கட்டமைப்பு PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  மாறிலி PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
கட்டமைப்பு PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  எழுத்து *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
நிலையான முழு pragmaVtabConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  மாறிலி PragmaName *pPragma = (மாறிலி PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  முழு rc;
  முழு i, j;
  எழுத்து cSep = '(';
  StrAccum acc;
  எழுத்து zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, அளவு(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  க்கு(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  எனில்( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  எனில்( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  எனில்( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  ப்ரதிஜ்ஞா( strlen(zBuf) < அளவு(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  எனில்( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(அளவு(PragmaVtab));
    எனில்( pTab==0 ){
      rc = SQLITE_NOMEM;
    }இல்லையெனில்{
      memset(pTab, 0, அளவு(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }இல்லையெனில்{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  திரும்பு rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
நிலையான முழு pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  திரும்பு SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
நிலையான முழு pragmaVtabBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  மாறிலி கட்டமைப்பு sqlite3_index_constraint *pConstraint;
  முழு i, j;
  முழு seen[2];

  pIdxInfo->estimatedCost = (இரட்டை)1;
  எனில்( pTab->nHidden==0 ){ திரும்பு SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  க்கு(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    எனில்( pConstraint->iColumn < pTab->iHidden ) தொடர்;
    எனில்( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) தொடர்;
    எனில்( pConstraint->usable==0 ) திரும்பு SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    ப்ரதிஜ்ஞா( j < 2 );
    seen[j] = i+1;
  }
  எனில்( seen[0]==0 ){
    pIdxInfo->estimatedCost = (இரட்டை)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    திரும்பு SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argvIndex = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (இரட்டை)20;
  pIdxInfo->estimatedRows = 20;
  எனில்( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argvIndex = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  திரும்பு SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
நிலையான முழு pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(அளவு(*pCsr));
  எனில்( pCsr==0 ) திரும்பு SQLITE_NOMEM;
  memset(pCsr, 0, அளவு(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  திரும்பு SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
நிலையான வெறுமை pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  முழு i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  க்கு(i=0; i<ArraySize(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
நிலையான முழு pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  திரும்பு SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
நிலையான முழு pragmaVtabNext(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  முழு rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  ப்ரதிஜ்ஞா( pCsr->pPragma );
  எனில்( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  திரும்பு rc;
}

/*
** Pragma virtual table module xFilter method.
*/
நிலையான முழு pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  முழு rc;
  முழு i, j;
  StrAccum acc;
  எழுத்து *zSql;

  UNUSED_PARAMETER(idx_இலக்கம்);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  க்கு(i=0; i<argc; i++, j++){
    மாறிலி எழுத்து *zText = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]);
    ப்ரதிஜ்ஞா( j<ArraySize(pCsr->azArg) );
    ப்ரதிஜ்ஞா( pCsr->azArg[j]==0 );
    எனில்( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      எனில்( pCsr->azArg[j]==0 ){
        திரும்பு SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_LENGTH]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  எனில்( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  எனில்( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  எனில்( zSql==0 ) திரும்பு SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  எனில்( rc!=SQLITE_OK ){
    pTab->base.z_Err_செய்தி = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    திரும்பு rc;
  }
  திரும்பு pragmaVtabNext(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
நிலையான முழு pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  திரும்பு (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
நிலையான முழு pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  முழு i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  எனில்( i<pTab->iHidden ){
    sqlite3_result_மதிப்பு(ctx, sqlite3_column_மதிப்பு(pCsr->pPragma, i));
  }இல்லையெனில்{
    sqlite3_result_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  திரும்பு SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
நிலையான முழு pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  திரும்பு SQLITE_OK;
}

/* The pragma virtual table object */
நிலையான மாறிலி sqlite3_module pragmaVtabModule = {
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
Module *sqlite3PragmaVtabRegister(sqlite3 *db, மாறிலி எழுத்து *zName){
  மாறிலி PragmaName *pName;
  ப்ரதிஜ்ஞா( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  எனில்( pName==0 ) திரும்பு 0;
  எனில்( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) திரும்பு 0;
  ப்ரதிஜ்ஞா( sqlite3HashFind(&db->aModule, zName)==0 );
  திரும்பு sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (வெறுமை*)pName, 0);
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */

#endif /* SQLITE_OMIT_PRAGMA */
