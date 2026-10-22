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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ಯದಿ !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  ಯದಿ defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  ಅನ್ಯಥಾ
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
#ಅನ್ತರ್ಭಾವ "pragma.h"

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
ಸ್ಥಿರ u8 getSafetyLevel(ಧ್ರುವ ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ omitFull, u8 dflt){
                             /* 123456789 123456789 123 */
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ zText[] = "onoffalseyestruextrafull";
  ಸ್ಥಿರ ಧ್ರುವ u8 iOffset[] = {0, 1, 2,  4,    9,  12,  15,   20};
  ಸ್ಥಿರ ಧ್ರುವ u8 iLength[] = {2, 2, 3,  5,    3,   4,   5,    4};
  ಸ್ಥಿರ ಧ್ರುವ u8 i_ಮೌಲ್ಯ[] =  {1, 0, 0,  0,    1,   1,   3,    2};
                            /* on no off false yes true extra full */
  ಪೂರ್ಣಾಙ್ಕ i, n;
  ಯದಿ( sqlite3Isdigit(*z) ){
    ಪ್ರತ್ಯಾಗಮ sqlite3Atoi(z)!=0;
  }
  n = sqlite3Strlen30(z);
  ಪ್ರತಿ(i=0; i<ArraySize(iLength); i++){
    ಯದಿ( iLength[i]==n && sqlite3StrNICmp(&zText[iOffset[i]],z,n)==0
     && (!omitFull || i_ಮೌಲ್ಯ[i]<=1)
    ){
      ಪ್ರತ್ಯಾಗಮ i_ಮೌಲ್ಯ[i];
    }
  }
  ಪ್ರತ್ಯಾಗಮ dflt;
}

/*
** Interpret the given string as a boolean value.
*/
u8 sqlite3GetBoolean(ಧ್ರುವ ವರ್ಣ *z, u8 dflt){
  ಪ್ರತ್ಯಾಗಮ getSafetyLevel(z,1,dflt)!=0;
}

/* The sqlite3GetBoolean() function is used by other modules but the
** remainder of this file is specific to PRAGMA processing.  So omit
** the rest of the file if PRAGMAs are omitted from the build.
*/
#ಯದಿ !defined(SQLITE_OMIT_PRAGMA)

/*
** Interpret the given string as a locking mode value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getLockingMode(ಧ್ರುವ ವರ್ಣ *z){
  ಯದಿ( z ){
    ಯದಿ( 0==sqlite3StrICmp(z, "exclusive") ) ಪ್ರತ್ಯಾಗಮ PAGER_LOCKINGMODE_EXCLUSIVE;
    ಯದಿ( 0==sqlite3StrICmp(z, "normal") ) ಪ್ರತ್ಯಾಗಮ PAGER_LOCKINGMODE_NORMAL;
  }
  ಪ್ರತ್ಯಾಗಮ PAGER_LOCKINGMODE_QUERY;
}

#ifndef SQLITE_OMIT_AUTOVACUUM
/*
** Interpret the given string as an auto-vacuum mode value.
**
** The following strings, "none", "full" and "incremental" are
** acceptable, as are their numeric equivalents: 0, 1 and 2 respectively.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getAutoVacuum(ಧ್ರುವ ವರ್ಣ *z){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( 0==sqlite3StrICmp(z, "none") ) ಪ್ರತ್ಯಾಗಮ BTREE_AUTOVACUUM_NONE;
  ಯದಿ( 0==sqlite3StrICmp(z, "full") ) ಪ್ರತ್ಯಾಗಮ BTREE_AUTOVACUUM_FULL;
  ಯದಿ( 0==sqlite3StrICmp(z, "incremental") ) ಪ್ರತ್ಯಾಗಮ BTREE_AUTOVACUUM_INCR;
  i = sqlite3Atoi(z);
  ಪ್ರತ್ಯಾಗಮ (u8)((i>=0&&i<=2)?i:0);
}
#endif /* ifndef SQLITE_OMIT_AUTOVACUUM */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Interpret the given string as a temp db location. Return 1 for file
** backed temporary databases, 2 for the Red-Black tree in memory database
** and 0 to use the compile-time default.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getTempStore(ಧ್ರುವ ವರ್ಣ *z){
  ಯದಿ( z[0]>='0' && z[0]<='2' ){
    ಪ್ರತ್ಯಾಗಮ z[0] - '0';
  }ಅನ್ಯಥಾ ಯದಿ( sqlite3StrICmp(z, "file")==0 ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ ಯದಿ( sqlite3StrICmp(z, "memory")==0 ){
    ಪ್ರತ್ಯಾಗಮ 2;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** Invalidate temp storage, either when the temp storage is changed
** from default, or when 'file' and the temp_store_directory has changed
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ invalidateTempStorage(Parse *pParse){
  sqlite3 *db = pParse->db;
  ಯದಿ( db->aDb[1].pBt!=0 ){
    ಯದಿ( !db->autoCommit
     || sqlite3BtreeTxnState(db->aDb[1].pBt)!=SQLITE_TXN_NONE
    ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "temporary storage cannot be changed "
        "from within a transaction");
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_PAGER_PRAGMAS
/*
** If the TEMP database is open, close it and mark the database schema
** as needing reloading.  This must be done when using the SQLITE_TEMP_STORE
** or DEFAULT_TEMP_STORE pragmas.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ changeTempStorage(Parse *pParse, ಧ್ರುವ ವರ್ಣ *zStorageType){
  ಪೂರ್ಣಾಙ್ಕ ts = getTempStore(zStorageType);
  sqlite3 *db = pParse->db;
  ಯದಿ( db->temp_store==ts ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  ಯದಿ( invalidateTempStorage( pParse ) != SQLITE_OK ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
  db->temp_store = (u8)ts;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
#endif /* SQLITE_PAGER_PRAGMAS */

/*
** Set result column names for a pragma.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setPragmaResultColumnNames(
  Vdbe *v,                     /* The query under construction */
  ಧ್ರುವ PragmaName *pPragma    /* The pragma */
){
  u8 n = pPragma->nPragCName;
  sqlite3_Vdbe_Set_ಅಂಕೆ_Cols(v, n==0 ? 1 : n);
  ಯದಿ( n==0 ){
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, pPragma->zName, SQLITE_STATIC);
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ i, j;
    ಪ್ರತಿ(i=0, j=pPragma->iPragCName; i<n; i++, j++){
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, pragCName[j], SQLITE_STATIC);
    }
  }
}

/*
** Generate code to return a single integer value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ returnSingleInt(Vdbe *v, i64 ಮೌಲ್ಯ){
  sqlite3VdbeAddInt64(v, 1, ಮೌಲ್ಯ);
  sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
}

/*
** Generate code to return a single text value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ returnSingleText(
  Vdbe *v,                /* Prepared statement under construction */
  ಧ್ರುವ ವರ್ಣ *z_ಮೌಲ್ಯ      /* Value to be returned */
){
  ಯದಿ( z_ಮೌಲ್ಯ ){
    sqlite3VdbeLoadString(v, 1, (ಧ್ರುವ ವರ್ಣ*)z_ಮೌಲ್ಯ);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
  }
}


/*
** Set the safety_level and pager flags for pager iDb.  Or if iDb<0
** set these values for all pagers.
*/
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
ಸ್ಥಿರ ಶೂನ್ಯ setAllPagerFlags(sqlite3 *db){
  ಯದಿ( db->autoCommit ){
    Db *pDb = db->aDb;
    ಪೂರ್ಣಾಙ್ಕ n = db->nDb;
    ಪ್ರತಿಜ್ಞಾ( SQLITE_FullFSync==PAGER_FULLFSYNC );
    ಪ್ರತಿಜ್ಞಾ( SQLITE_CkptFullFSync==PAGER_CKPT_FULLFSYNC );
    ಪ್ರತಿಜ್ಞಾ( SQLITE_CacheSpill==PAGER_CACHESPILL );
    ಪ್ರತಿಜ್ಞಾ( (PAGER_FULLFSYNC | PAGER_CKPT_FULLFSYNC | PAGER_CACHESPILL)
             ==  PAGER_FLAGS_MASK );
    ಪ್ರತಿಜ್ಞಾ( (pDb->safety_level & PAGER_SYNCHRONOUS_MASK)==pDb->safety_level );
    ಯಾವತ್( (n--) > 0 ){
      ಯದಿ( pDb->pBt ){
        sqlite3BtreeSetPagerFlags(pDb->pBt,
                 pDb->safety_level | (db->flags & PAGER_FLAGS_MASK) );
      }
      pDb++;
    }
  }
}
#ಅನ್ಯಥಾ
# define setAllPagerFlags(X)  /* no-op */
#endif


/*
** Return a human-readable name for a constraint resolution action.
*/
#ifndef SQLITE_OMIT_FOREIGN_ಕೀಲಿ
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *actionName(u8 action){
  ಧ್ರುವ ವರ್ಣ *zName;
  ವಿಕಲ್ಪ( action ){
    ಪಕ್ಷ OE_SetNull:  zName = "SET NULL";        ಭಙ್ಗ;
    ಪಕ್ಷ OE_SetDflt:  zName = "SET DEFAULT";     ಭಙ್ಗ;
    ಪಕ್ಷ OE_Cascade:  zName = "CASCADE";         ಭಙ್ಗ;
    ಪಕ್ಷ OE_Restrict: zName = "RESTRICT";        ಭಙ್ಗ;
    ಸ್ವಭಾವ:          zName = "NO ACTION"; 
                      ಪ್ರತಿಜ್ಞಾ( action==OE_None ); ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ zName;
}
#endif


/*
** Parameter eMode must be one of the PAGER_JOURNALMODE_XXX constants
** defined in pager.h. This function returns the associated lowercase
** journal-mode name.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3JournalModename(ಪೂರ್ಣಾಙ್ಕ eMode){
  ಸ್ಥಿರ ವರ್ಣ * ಧ್ರುವ azModeName[] = {
    "delete", "persist", "off", "truncate", "memory"
#ifndef SQLITE_OMIT_WAL
     , "wal"
#endif
  };
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_DELETE==0 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_PERSIST==1 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_OFF==2 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_TRUNCATE==3 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_MEMORY==4 );
  ಪ್ರತಿಜ್ಞಾ( PAGER_JOURNALMODE_WAL==5 );
  ಪ್ರತಿಜ್ಞಾ( eMode>=0 && eMode<=ArraySize(azModeName) );

  ಯದಿ( eMode==ArraySize(azModeName) ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ azModeName[eMode];
}

/*
** Locate a pragma in the aPragmaName[] array.
*/
ಸ್ಥಿರ ಧ್ರುವ PragmaName *pragmaLocate(ಧ್ರುವ ವರ್ಣ *zName){
  ಪೂರ್ಣಾಙ್ಕ upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = ArraySize(aPragmaName)-1;
  ಯಾವತ್( lwr<=upr ){
    mid = (lwr+upr)/2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    ಯದಿ( rc==0 ) ಭಙ್ಗ;
    ಯದಿ( rc<0 ){
      upr = mid - 1;
    }ಅನ್ಯಥಾ{
      lwr = mid + 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ lwr>upr ? 0 : &aPragmaName[mid];
}

/*
** Create zero or more entries in the output for the SQL functions
** defined by FuncDef p.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pragma_Funclist_ಸಾಲು(
  Vdbe *v,               /* The prepared statement being created */
  FuncDef *p,            /* A particular function definition */
  ಪೂರ್ಣಾಙ್ಕ isBuiltin,         /* True if this is a built-in function */
  ಪೂರ್ಣಾಙ್ಕ showInternFuncs    /* True if showing internal functions */
){
  u32 mask =
      SQLITE_DETERMINISTIC |
      SQLITE_DIRECTONLY |
      SQLITE_SUBTYPE |
      SQLITE_INNOCUOUS |
      SQLITE_FUNC_INTERNAL
  ;
  ಯದಿ( showInternFuncs ) mask = 0xffffffff;
  ಪ್ರತಿ(; p; p=p->pNext){
    ಧ್ರುವ ವರ್ಣ *zType;
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *azEnc[] = { 0, "utf8", "utf16le", "utf16be" };

    ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_ENCMASK==0x3 );
    ಪ್ರತಿಜ್ಞಾ( strcmp(azEnc[SQLITE_UTF8],"utf8")==0 );
    ಪ್ರತಿಜ್ಞಾ( strcmp(azEnc[SQLITE_UTF16LE],"utf16le")==0 );
    ಪ್ರತಿಜ್ಞಾ( strcmp(azEnc[SQLITE_UTF16BE],"utf16be")==0 );

    ಯದಿ( p->xSFunc==0 ) ಅನುವರ್ತ;
    ಯದಿ( (p->funcFlags & SQLITE_FUNC_INTERNAL)!=0
     && showInternFuncs==0
    ){
      ಅನುವರ್ತ;
    }   
    ಯದಿ( p->x_ಮೌಲ್ಯ!=0 ){
      zType = "w";
    }ಅನ್ಯಥಾ ಯದಿ( p->xFinalize!=0 ){
      zType = "a";
    }ಅನ್ಯಥಾ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ integrityCheckResultRow(Vdbe *v){
  ಪೂರ್ಣಾಙ್ಕ addr;
  sqlite3VdbeAddOp2(v, OP_ResultRow, 3, 1);
  addr = sqlite3VdbeAddOp3(v, OP_IfPos, 1, sqlite3VdbeCurrentAddr(v)+2, 1);
  VdbeCoverage(v);
  sqlite3VdbeAddOp0(v, OP_Halt);
  ಪ್ರತ್ಯಾಗಮ addr;
}

/*
** Should table pTab be skipped when doing an integrity_check?
** Return true or false.
**
** If pObjTab is not null, the return true if pTab matches pObjTab.
**
** If pObjTab is null, then return true only if pTab is an imposter table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tableSkipIntegrityCheck(ಧ್ರುವ Table *pTab, ಧ್ರುವ Table *pObjTab){
  ಯದಿ( pObjTab ){
    ಪ್ರತ್ಯಾಗಮ pTab!=pObjTab;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ (pTab->tabFlags & TF_Imposter)!=0;
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
ಶೂನ್ಯ sqlite3Pragma(
  Parse *pParse,
  Token *pId1,        /* First part of [schema.]id field */
  Token *pId2,        /* Second part of [schema.]id field, or NULL */
  Token *p_ಮೌಲ್ಯ,      /* Token for <value>, or NULL */
  ಪೂರ್ಣಾಙ್ಕ minusFlag       /* True if a '-' sign preceded <value> */
){
  ವರ್ಣ *z_ಎಡ = 0;       /* Nul-terminated UTF-8 string <id> */
  ವರ್ಣ *z_ಬಲ = 0;      /* Nul-terminated UTF-8 string <value>, or NULL */
  ಧ್ರುವ ವರ್ಣ *zDb = 0;   /* The database name */
  Token *pId;            /* Pointer to <id> token */
  ವರ್ಣ *aFcntl[4];       /* Argument to SQLITE_FCNTL_PRAGMA */
  ಪೂರ್ಣಾಙ್ಕ iDb;               /* Database index for <database> */
  ಪೂರ್ಣಾಙ್ಕ rc;                      /* return value form SQLITE_FCNTL_PRAGMA */
  sqlite3 *db = pParse->db;    /* The database connection */
  Db *pDb;                     /* The specific database being pragmaed */
  Vdbe *v = sqlite3GetVdbe(pParse);  /* Prepared statement */
  ಧ್ರುವ PragmaName *pPragma;   /* The pragma */

  ಯದಿ( v==0 ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  /* Interpret the [schema.] part of the pragma statement. iDb is the
  ** index of the database this pragma is being applied to in db.aDb[]. */
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  ಯದಿ( iDb<0 ) ಪ್ರತ್ಯಾಗಮ;
  pDb = &db->aDb[iDb];

  /* If the temp database has been explicitly named as part of the
  ** pragma, make sure it is open.
  */
  ಯದಿ( iDb==1 && sqlite3OpenTempDatabase(pParse) ){
    ಪ್ರತ್ಯಾಗಮ;
  }

  z_ಎಡ = sqlite3NameFromToken(db, pId);
  ಯದಿ( !z_ಎಡ ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( minusFlag ){
    z_ಬಲ = sqlite3MPrintf(db, "-%T", p_ಮೌಲ್ಯ);
  }ಅನ್ಯಥಾ{
    z_ಬಲ = sqlite3NameFromToken(db, p_ಮೌಲ್ಯ);
  }

  ಪ್ರತಿಜ್ಞಾ( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, z_ಎಡ, z_ಬಲ, zDb) ){
    ಗಚ್ಛ pragma_out;
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
  aFcntl[1] = z_ಎಡ;
  aFcntl[2] = z_ಬಲ;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (ಶೂನ್ಯ*)aFcntl);
  ಯದಿ( rc==SQLITE_OK ){
    sqlite3_Vdbe_Set_ಅಂಕೆ_Cols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    ಗಚ್ಛ pragma_out;
  }
  ಯದಿ( rc!=SQLITE_NOTFOUND ){
    ಯದಿ( aFcntl[0] ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    ಗಚ್ಛ pragma_out;
  }

  /* Locate the pragma in the lookup table */
  pPragma = pragmaLocate(z_ಎಡ);
  ಯದಿ( pPragma==0 ){
    /* IMP: R-43042-22504 No error messages are generated if an
    ** unknown pragma is issued. */
    ಗಚ್ಛ pragma_out;
  }

  /* Make sure the database schema is loaded if the pragma requires that */
  ಯದಿ( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    ಯದಿ( sqlite3ReadSchema(pParse) ) ಗಚ್ಛ pragma_out;
  }

  /* Register the result column names for pragmas that return results */
  ಯದಿ( (pPragma->mPragFlg & PragFlg_NoColumns)==0
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || z_ಬಲ==0)
  ){
    setPragmaResultColumnNames(v, pPragma);
  }

  /* Jump to the appropriate pragma handler */
  ವಿಕಲ್ಪ( pPragma->ePragTyp ){
 
#ಯದಿ !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
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
  ಪಕ್ಷ PragTyp_DEFAULT_CACHE_SIZE: {
    ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ iLn = VDBE_OFFSET_LINENO(2);
    ಸ್ಥಿರ ಧ್ರುವ VdbeOpList getCacheSize[] = {
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
    ಯದಿ( !z_ಬಲ ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(getCacheSize));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(getCacheSize), getCacheSize, iLn);
      ಯದಿ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ಭಙ್ಗ;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_SIZE;
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ size = sqlite3AbsInt32(sqlite3Atoi(z_ಬಲ));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_SIZE, size);
      ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    ಭಙ್ಗ;
  }
#endif /* !SQLITE_OMIT_PAGER_PRAGMAS && !SQLITE_OMIT_DEPRECATED */

#ಯದಿ !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  /*
  **  PRAGMA [schema.]page_size
  **  PRAGMA [schema.]page_size=N
  **
  ** The first form reports the current setting for the
  ** database page size in bytes.  The second form sets the
  ** database page size value.  The value can only be set if
  ** the database has not yet been created.
  */
  ಪಕ್ಷ PragTyp_PAGE_SIZE: {
    Btree *pBt = pDb->pBt;
    ಪ್ರತಿಜ್ಞಾ( pBt!=0 );
    ಯದಿ( !z_ಬಲ ){
      ಪೂರ್ಣಾಙ್ಕ size = ALWAYS(pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
      returnSingleInt(v, size);
    }ಅನ್ಯಥಾ{
      /* Malloc may fail when setting the page-size, as there is an internal
      ** buffer that the pager module resizes using sqlite3_realloc().
      */
      db->nextPagesize = sqlite3Atoi(z_ಬಲ);
      ಯದಿ( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize,0,0) ){
        sqlite3OomFault(db);
      }
    }
    ಭಙ್ಗ;
  }

  /*
  **  PRAGMA [schema.]secure_delete
  **  PRAGMA [schema.]secure_delete=ON/OFF/FAST
  **
  ** The first form reports the current setting for the
  ** secure_delete flag.  The second form changes the secure_delete
  ** flag setting and reports the new value.
  */
  ಪಕ್ಷ PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    ಪೂರ್ಣಾಙ್ಕ b = -1;
    ಪ್ರತಿಜ್ಞಾ( pBt!=0 );
    ಯದಿ( z_ಬಲ ){
      ಯದಿ( sqlite3_stricmp(z_ಬಲ, "fast")==0 ){
        b = 2;
      }ಅನ್ಯಥಾ{
        b = sqlite3GetBoolean(z_ಬಲ, 0);
      }
    }
    ಯದಿ( pId2->n==0 && b>=0 ){
      ಪೂರ್ಣಾಙ್ಕ ii;
      ಪ್ರತಿ(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    ಭಙ್ಗ;
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
  ಪಕ್ಷ Prag_Typ_PAGE_ಎಣಿಕೆ: {
    ಪೂರ್ಣಾಙ್ಕ iReg;
    i64 x = 0;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    ಯದಿ( sqlite3Tolower(z_ಎಡ[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }ಅನ್ಯಥಾ{
      ಯದಿ( z_ಬಲ && sqlite3DecOrHexToI64(z_ಬಲ,&x)==0 ){
        ಯದಿ( x<0 ) x = 0;
        ಅನ್ಯಥಾ ಯದಿ( x>0xfffffffe ) x = 0xfffffffe;
      }ಅನ್ಯಥಾ{
        x = 0;
      }
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, (ಪೂರ್ಣಾಙ್ಕ)x);
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, iReg, 1);
    ಭಙ್ಗ;
  }

  /*
  **  PRAGMA [schema.]locking_mode
  **  PRAGMA [schema.]locking_mode = (normal|exclusive)
  */
  ಪಕ್ಷ PragTyp_LOCKING_MODE: {
    ಧ್ರುವ ವರ್ಣ *zRet = "normal";
    ಪೂರ್ಣಾಙ್ಕ eMode = getLockingMode(z_ಬಲ);

    ಯದಿ( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      /* Simple "PRAGMA locking_mode;" statement. This is a query for
      ** the current default locking mode (which may be different to
      ** the locking-mode of the main database).
      */
      eMode = db->dfltLockMode;
    }ಅನ್ಯಥಾ{
      Pager *pPager;
      ಯದಿ( pId2->n==0 ){
        /* This indicates that no database name was specified as part
        ** of the PRAGMA command. In this case the locking-mode must be
        ** set on all attached databases, as well as the main db file.
        **
        ** Also, the sqlite3.dfltLockMode variable is set so that
        ** any subsequently attached databases also use the specified
        ** locking mode.
        */
        ಪೂರ್ಣಾಙ್ಕ ii;
        ಪ್ರತಿಜ್ಞಾ(pDb==&db->aDb[0]);
        ಪ್ರತಿ(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }

    ಪ್ರತಿಜ್ಞಾ( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    ಯದಿ( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    ಭಙ್ಗ;
  }

  /*
  **  PRAGMA [schema.]journal_mode
  **  PRAGMA [schema.]journal_mode =
  **                      (delete|persist|off|truncate|memory|wal|off)
  */
  ಪಕ್ಷ PragTyp_JOURNAL_MODE: {
    ಪೂರ್ಣಾಙ್ಕ eMode;        /* One of the PAGER_JOURNALMODE_XXX symbols */
    ಪೂರ್ಣಾಙ್ಕ ii;           /* Loop counter */

    ಯದಿ( z_ಬಲ==0 ){
      /* If there is no "=MODE" part of the pragma, do a query for the
      ** current mode */
      eMode = PAGER_JOURNALMODE_QUERY;
    }ಅನ್ಯಥಾ{
      ಧ್ರುವ ವರ್ಣ *zMode;
      ಪೂರ್ಣಾಙ್ಕ n = sqlite3Strlen30(z_ಬಲ);
      ಪ್ರತಿ(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        ಯದಿ( sqlite3StrNICmp(z_ಬಲ, zMode, n)==0 ) ಭಙ್ಗ;
      }
      ಯದಿ( !zMode ){
        /* If the "=MODE" part does not match any known journal mode,
        ** then do a query */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      ಯದಿ( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow journal-mode "OFF" in defensive since the database
        ** can become corrupted using ordinary SQL when the journal is off */
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    ಯದಿ( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      /* Convert "PRAGMA journal_mode" into "PRAGMA main.journal_mode" */
      iDb = 0;
      pId2->n = 1;
    }
    ಪ್ರತಿ(ii=db->nDb-1; ii>=0; ii--){
      ಯದಿ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    ಭಙ್ಗ;
  }

  /*
  **  PRAGMA [schema.]journal_size_limit
  **  PRAGMA [schema.]journal_size_limit=N
  **
  ** Get or set the size limit on rollback journal files.
  */
  ಪಕ್ಷ PragTyp_JOURNAL_SIZE_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    ಯದಿ( z_ಬಲ ){
      sqlite3DecOrHexToI64(z_ಬಲ, &iLimit);
      ಯದಿ( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    ಪ್ರತಿಜ್ಞಾ( pBt!=0 );
    ಯದಿ( !z_ಬಲ ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ eAuto = getAutoVacuum(z_ಬಲ);
      ಪ್ರತಿಜ್ಞಾ( eAuto>=0 && eAuto<=2 );
      db->nextAutovac = (u8)eAuto;
      /* Call SetAutoVacuum() to set initialize the internal auto and
      ** incr-vacuum flags. This is required in case this connection
      ** creates the database file. It is important that it is created
      ** as an auto-vacuum capable db.
      */
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      ಯದಿ( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        /* When setting the auto_vacuum mode to either "full" or
        ** "incremental", write the value of meta[6] in the database
        ** file. Before writing to meta[6], check that meta[3] indicates
        ** that this really is an auto-vacuum capable database.
        */
        ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ iLn = VDBE_OFFSET_LINENO(2);
        ಸ್ಥಿರ ಧ್ರುವ VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},    /* 0 */
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_ಬೇರು_PAGE},
          { OP_If,             1,         0,                 0},    /* 2 */
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},    /* 3 */
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},    /* 4 */
        };
        VdbeOp *aOp;
        ಪೂರ್ಣಾಙ್ಕ iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, ArraySize(setMeta6), setMeta6, iLn);
        ಯದಿ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ಭಙ್ಗ;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    ಭಙ್ಗ;
  }
#endif

  /*
  **  PRAGMA [schema.]incremental_vacuum(N)
  **
  ** Do N steps of incremental vacuuming on a database.
  */
#ifndef SQLITE_OMIT_AUTOVACUUM
  ಪಕ್ಷ PragTyp_INCREMENTAL_VACUUM: {
    ಪೂರ್ಣಾಙ್ಕ iLimit = 0, addr;
    ಯದಿ( z_ಬಲ==0 || !sqlite3GetInt32(z_ಬಲ, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ResultRow, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_CACHE_SIZE: {
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಯದಿ( !z_ಬಲ ){
      returnSingleInt(v, pDb->pSchema->cache_size);
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ size = sqlite3Atoi(z_ಬಲ);
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_CACHE_SPILL: {
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಯದಿ( !z_ಬಲ ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 :
            sqlite3BtreeSetSpillSize(pDb->pBt,0));
    }ಅನ್ಯಥಾ{
      ಪೂರ್ಣಾಙ್ಕ size = 1;
      ಯದಿ( sqlite3GetInt32(z_ಬಲ, &size) ){
        sqlite3BtreeSetSpillSize(pDb->pBt, size);
      }
      ಯದಿ( sqlite3GetBoolean(z_ಬಲ, size!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }ಅನ್ಯಥಾ{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_MMAP_SIZE: {
    sqlite3_int64 sz;
#ಯದಿ SQLITE_MAX_MMAP_SIZE>0
    ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
    ಯದಿ( z_ಬಲ ){
      ಪೂರ್ಣಾಙ್ಕ ii;
      sqlite3DecOrHexToI64(z_ಬಲ, &sz);
      ಯದಿ( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      ಯದಿ( pId2->n==0 ) db->szMmap = sz;
      ಪ್ರತಿ(ii=db->nDb-1; ii>=0; ii--){
        ಯದಿ( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);
#ಅನ್ಯಥಾ
    sz = 0;
    rc = SQLITE_OK;
#endif
    ಯದಿ( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }ಅನ್ಯಥಾ ಯದಿ( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_TEMP_STORE: {
    ಯದಿ( !z_ಬಲ ){
      returnSingleInt(v, db->temp_store);
    }ಅನ್ಯಥಾ{
      changeTempStorage(pParse, z_ಬಲ);
    }
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_TEMP_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ಯದಿ( !z_ಬಲ ){
      returnSingleText(v, sqlite3_temp_directory);
    }ಅನ್ಯಥಾ{
#ifndef SQLITE_OMIT_WSD
      ಯದಿ( z_ಬಲ[0] ){
        ಪೂರ್ಣಾಙ್ಕ res;
        rc = sqlite3OsAccess(db->pVfs, z_ಬಲ, SQLITE_ACCESS_READWRITE, &res);
        ಯದಿ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          ಗಚ್ಛ pragma_out;
        }
      }
      ಯದಿ( SQLITE_TEMP_STORE==0
       || (SQLITE_TEMP_STORE==1 && db->temp_store<=1)
       || (SQLITE_TEMP_STORE==2 && db->temp_store==1)
      ){
        invalidateTempStorage(pParse);
      }
      sqlite3_free(sqlite3_temp_directory);
      ಯದಿ( z_ಬಲ[0] ){
        sqlite3_temp_directory = sqlite3_mprintf("%s", z_ಬಲ);
      }ಅನ್ಯಥಾ{
        sqlite3_temp_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ಭಙ್ಗ;
  }

#ಯದಿ SQLITE_OS_WIN
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
  ಪಕ್ಷ PragTyp_DATA_STORE_DIRECTORY: {
    sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ಯದಿ( !z_ಬಲ ){
      returnSingleText(v, sqlite3_data_directory);
    }ಅನ್ಯಥಾ{
#ifndef SQLITE_OMIT_WSD
      ಯದಿ( z_ಬಲ[0] ){
        ಪೂರ್ಣಾಙ್ಕ res;
        rc = sqlite3OsAccess(db->pVfs, z_ಬಲ, SQLITE_ACCESS_READWRITE, &res);
        ಯದಿ( rc!=SQLITE_OK || res==0 ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "not a writable directory");
          sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
          ಗಚ್ಛ pragma_out;
        }
      }
      sqlite3_free(sqlite3_data_directory);
      ಯದಿ( z_ಬಲ[0] ){
        sqlite3_data_directory = sqlite3_mprintf("%s", z_ಬಲ);
      }ಅನ್ಯಥಾ{
        sqlite3_data_directory = 0;
      }
#endif /* SQLITE_OMIT_WSD */
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
    ಭಙ್ಗ;
  }
#endif

#ಯದಿ SQLITE_ENABLE_LOCKING_STYLE
  /*
  **   PRAGMA [schema.]lock_proxy_file
  **   PRAGMA [schema.]lock_proxy_file = ":auto:"|"lock_file_path"
  **
  ** Return or set the value of the lock_proxy_file flag.  Changing
  ** the value sets a specific file to be used for database access locks.
  **
  */
  ಪಕ್ಷ PragTyp_LOCK_PROXY_FILE: {
    ಯದಿ( !z_ಬಲ ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      ವರ್ಣ *proxy_file_path = ರಿಕ್ತ;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE,
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }ಅನ್ಯಥಾ{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      ಪೂರ್ಣಾಙ್ಕ res;
      ಯದಿ( z_ಬಲ[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     z_ಬಲ);
      } ಅನ್ಯಥಾ {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE,
                                     ರಿಕ್ತ);
      }
      ಯದಿ( res!=SQLITE_OK ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "failed to set lock proxy file");
        ಗಚ್ಛ pragma_out;
      }
    }
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_SYNCHRONOUS: {
    ಯದಿ( !z_ಬಲ ){
      returnSingleInt(v, pDb->safety_level-1);
    }ಅನ್ಯಥಾ{
      ಯದಿ( !db->autoCommit ){
        sqlite3_Error_ಸಂದೇಶ(pParse,
            "Safety level may not be changed inside a transaction");
      }ಅನ್ಯಥಾ ಯದಿ( iDb!=1 ){
        ಪೂರ್ಣಾಙ್ಕ iLevel;
        ಯದಿ( sqlite3Isdigit(z_ಬಲ[0]) ){
          iLevel = sqlite3Atoi(z_ಬಲ);
        }ಅನ್ಯಥಾ{
          iLevel = getSafetyLevel(z_ಬಲ,0,1);
        }
        iLevel = (iLevel+1) & PAGER_SYNCHRONOUS_MASK;
        ಯದಿ( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    ಭಙ್ಗ;
  }
#endif /* SQLITE_OMIT_PAGER_PRAGMAS */

#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  ಪಕ್ಷ PragTyp_FLAG: {
    ಯದಿ( z_ಬಲ==0 ){
      setPragmaResultColumnNames(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }ಅನ್ಯಥಾ{
      u64 mask = pPragma->iArg;    /* Mask of bits to set or clear. */
      ಯದಿ( db->autoCommit==0 ){
        /* Foreign key support may not be enabled or disabled while not
        ** in auto-commit mode.  */
        mask &= ~(SQLITE_Foreign_ಕೀಲಿಗಳು);
      }

      ಯದಿ( sqlite3GetBoolean(z_ಬಲ, 0) ){
        ಯದಿ( (mask & SQLITE_WriteSchema)==0
         || (db->flags & SQLITE_Defensive)==0
        ){
          db->flags |= mask;
        }
      }ಅನ್ಯಥಾ{
        db->flags &= ~mask;
        ಯದಿ( mask==SQLITE_DeferFKs ){
          db->nDeferredImmCons = 0;
          db->nDeferredCons = 0;
        }
        ಯದಿ( (mask & SQLITE_WriteSchema)!=0
         && sqlite3_stricmp(z_ಬಲ, "reset")==0
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
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_TABLE_INFO: ಯದಿ( z_ಬಲ ){
    Table *pTab;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_ಬಲ, zDb);
    ಯದಿ( pTab ){
      ಪೂರ್ಣಾಙ್ಕ i, k;
      ಪೂರ್ಣಾಙ್ಕ nHidden = 0;
      Column *pCol;
      Index *pPk = sqlite3_Primary_ಕೀಲಿ_Index(pTab);
      pParse->nMem = 7;
      sqlite3ViewGetColumnNames(pParse, pTab);
      ಪ್ರತಿ(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        ಪೂರ್ಣಾಙ್ಕ isHidden = 0;
        ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_Col_ಅಭಿವ್ಯಕ್ತಿ;
        ಯದಿ( pCol->colFlags & COLFLAG_NOINSERT ){
          ಯದಿ( pPragma->iArg==0 ){
            nHidden++;
            ಅನುವರ್ತ;
          }
          ಯದಿ( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;  /* GENERATED ALWAYS AS ... VIRTUAL */
          }ಅನ್ಯಥಾ ಯದಿ( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;  /* GENERATED ALWAYS AS ... STORED */
          }ಅನ್ಯಥಾ{ ಪ್ರತಿಜ್ಞಾ( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;  /* HIDDEN */
          }
        }
        ಯದಿ( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }ಅನ್ಯಥಾ ಯದಿ( pPk==0 ){
          k = 1;
        }ಅನ್ಯಥಾ{
          ಪ್ರತಿ(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        p_Col_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_Column_ಅಭಿವ್ಯಕ್ತಿ(pTab,pCol);
        ಪ್ರತಿಜ್ಞಾ( p_Col_ಅಭಿವ್ಯಕ್ತಿ==0 || p_Col_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SPAN || isHidden>=2 );
        ಪ್ರತಿಜ್ಞಾ( p_Col_ಅಭಿವ್ಯಕ್ತಿ==0 || !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_Col_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ)
                  || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zCnName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               (isHidden>=2 || p_Col_ಅಭಿವ್ಯಕ್ತಿ==0) ? 0 : p_Col_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,
               k,
               isHidden);
      }
    }
  }
  ಭಙ್ಗ;

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
  ಪಕ್ಷ PragTyp_TABLE_LIST: {
    ಪೂರ್ಣಾಙ್ಕ ii;
    pParse->nMem = 6;
    sqlite3CodeVerifyNamedSchema(pParse, zDb);
    ಪ್ರತಿ(ii=0; ii<db->nDb; ii++){
      HashElem *k;
      Hash *pHash;
      ಪೂರ್ಣಾಙ್ಕ initNCol;
      ಯದಿ( zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName)!=0 ) ಅನುವರ್ತ;

      /* Ensure that the Table.nCol field is initialized for all views
      ** and virtual tables.  Each time we initialize a Table.nCol value
      ** for a table, that can potentially disrupt the hash table, so restart
      ** the initialization scan.
      */
      pHash = &db->aDb[ii].pSchema->tblHash;
      initNCol = sqlite_Hash_ಎಣಿಕೆ(pHash);
      ಯಾವತ್( initNCol-- ){
        ಪ್ರತಿ(k=sqliteHashFirst(pHash); 1; k=sqliteHashNext(k) ){
          Table *pTab;
          ಯದಿ( k==0 ){ initNCol = 0; ಭಙ್ಗ; }
          pTab = sqliteHashData(k);
          ಯದಿ( pTab->nCol==0 ){
            ವರ್ಣ *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
            ಯದಿ( zSql ){
              sqlite3_stmt *pDummy = 0;
              (ಶೂನ್ಯ)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG,
                                       &pDummy, 0);
              (ಶೂನ್ಯ)sqlite3_finalize(pDummy);
              sqlite3DbFree(db, zSql);
            }
            ಯದಿ( db->mallocFailed ){
              sqlite3_Error_ಸಂದೇಶ(db->pParse, "out of memory");
              db->pParse->rc = SQLITE_NOMEM_BKPT;
            }
            pHash = &db->aDb[ii].pSchema->tblHash;
            ಭಙ್ಗ;
          }
        }
      }

      ಪ್ರತಿ(k=sqliteHashFirst(pHash); k; k=sqliteHashNext(k) ){
        Table *pTab = sqliteHashData(k);
        ಧ್ರುವ ವರ್ಣ *zType;
        ಯದಿ( z_ಬಲ && sqlite3_stricmp(z_ಬಲ, pTab->zName)!=0 ) ಅನುವರ್ತ;
        ಯದಿ( IsView(pTab) ){
          zType = "view";
        }ಅನ್ಯಥಾ ಯದಿ( IsVirtual(pTab) ){
          zType = "virtual";
        }ಅನ್ಯಥಾ ಯದಿ( pTab->tabFlags & TF_Shadow ){
          zType = "shadow";
        }ಅನ್ಯಥಾ{
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
  ಭಙ್ಗ;

#ifdef SQLITE_DEBUG
  ಪಕ್ಷ PragTyp_STATS: {
    Index *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    ಪ್ರತಿ(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqliteHashNext(i)){
      Table *pTab = sqliteHashData(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           sqlite3PreferredTableName(pTab->zName),
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 5);
      }
    }
  }
  ಭಙ್ಗ;
#endif

  ಪಕ್ಷ PragTyp_INDEX_INFO: ಯದಿ( z_ಬಲ ){
    Index *pIdx;
    Table *pTab;
    pIdx = sqlite3FindIndex(db, z_ಬಲ, zDb);
    ಯದಿ( pIdx==0 ){
      /* If there is no index named zRight, check to see if there is a
      ** WITHOUT ROWID table named zRight, and if there is, show the
      ** structure of the PRIMARY KEY index for that table. */
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, z_ಬಲ, zDb);
      ಯದಿ( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3_Primary_ಕೀಲಿ_Index(pTab);
      }
    }
    ಯದಿ( pIdx ){
      ಪೂರ್ಣಾಙ್ಕ iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪೂರ್ಣಾಙ್ಕ mx;
      ಯದಿ( pPragma->iArg ){
        /* PRAGMA index_xinfo (newer version with more rows and columns) */
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }ಅನ್ಯಥಾ{
        /* PRAGMA index_info (legacy version) */
        mx = pIdx->n_ಕೀಲಿ_Col;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      ಪ್ರತಿಜ್ಞಾ( pParse->nMem<=pPragma->nPragCName );
      ಪ್ರತಿ(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zCnName);
        ಯದಿ( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->n_ಕೀಲಿ_Col);
        }
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, pParse->nMem);
      }
    }
  }
  ಭಙ್ಗ;

  ಪಕ್ಷ PragTyp_INDEX_LIST: ಯದಿ( z_ಬಲ ){
    Index *pIdx;
    Table *pTab;
    ಪೂರ್ಣಾಙ್ಕ i;
    pTab = sqlite3FindTable(db, z_ಬಲ, zDb);
    ಯದಿ( pTab ){
      ಪೂರ್ಣಾಙ್ಕ iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      ಪ್ರತಿ(pIdx=pTab->pIndex, i=0; pIdx; pIdx=pIdx->pNext, i++){
        ಧ್ರುವ ವರ್ಣ *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           IsUniqueIndex(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  ಭಙ್ಗ;

  ಪಕ್ಷ PragTyp_DATABASE_LIST: {
    ಪೂರ್ಣಾಙ್ಕ i;
    pParse->nMem = 3;
    ಪ್ರತಿ(i=0; i<db->nDb; i++){
      ಯದಿ( db->aDb[i].pBt==0 ) ಅನುವರ್ತ;
      ಪ್ರತಿಜ್ಞಾ( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  ಭಙ್ಗ;

  ಪಕ್ಷ PragTyp_COLLATION_LIST: {
    ಪೂರ್ಣಾಙ್ಕ i = 0;
    HashElem *p;
    pParse->nMem = 2;
    ಪ್ರತಿ(p=sqliteHashFirst(&db->aCollSeq); p; p=sqliteHashNext(p)){
      CollSeq *pColl = (CollSeq *)sqliteHashData(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  ಭಙ್ಗ;

#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  ಪಕ್ಷ PragTyp_FUNCTION_LIST: {
    ಪೂರ್ಣಾಙ್ಕ i;
    HashElem *j;
    FuncDef *p;
    ಪೂರ್ಣಾಙ್ಕ showInternFunc = (db->mDbFlags & DBFLAG_InternalFunc)!=0;
    pParse->nMem = 6;
    ಪ್ರತಿ(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      ಪ್ರತಿ(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        ಪ್ರತಿಜ್ಞಾ( p->funcFlags & SQLITE_FUNC_BUILTIN );
        pragma_Funclist_ಸಾಲು(v, p, 1, showInternFunc);
      }
    }
    ಪ್ರತಿ(j=sqliteHashFirst(&db->aFunc); j; j=sqliteHashNext(j)){
      p = (FuncDef*)sqliteHashData(j);
      ಪ್ರತಿಜ್ಞಾ( (p->funcFlags & SQLITE_FUNC_BUILTIN)==0 );
      pragma_Funclist_ಸಾಲು(v, p, 0, showInternFunc);
    }
  }
  ಭಙ್ಗ;

#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಪಕ್ಷ PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    ಪ್ರತಿ(j=sqliteHashFirst(&db->aModule); j; j=sqliteHashNext(j)){
      Module *pMod = (Module*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  ಭಙ್ಗ;
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  ಪಕ್ಷ PragTyp_PRAGMA_LIST: {
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<ArraySize(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  ಭಙ್ಗ;
#endif /* SQLITE_INTROSPECTION_PRAGMAS */

#endif /* SQLITE_OMIT_SCHEMA_PRAGMAS */

#ifndef SQLITE_OMIT_FOREIGN_ಕೀಲಿ
  ಪಕ್ಷ Prag_Typ_FOREIGN_ಕೀಲಿ_LIST: ಯದಿ( z_ಬಲ ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, z_ಬಲ, zDb);
    ಯದಿ( pTab && IsOrdinaryTable(pTab) ){
      pFK = pTab->u.tab.pFKey;
      ಯದಿ( pFK ){
        ಪೂರ್ಣಾಙ್ಕ iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        ಪೂರ್ಣಾಙ್ಕ i = 0;
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        ಯಾವತ್(pFK){
          ಪೂರ್ಣಾಙ್ಕ j;
          ಪ್ರತಿ(j=0; j<pFK->nCol; j++){
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
  ಭಙ್ಗ;
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_FOREIGN_ಕೀಲಿ
#ifndef SQLITE_OMIT_TRIGGER
  ಪಕ್ಷ Prag_Typ_FOREIGN_ಕೀಲಿ_CHECK: {
    FKey *pFK;             /* A foreign key constraint */
    Table *pTab;           /* Child table contain "REFERENCES" keyword */
    Table *pParent;        /* Parent table that child points to */
    Index *pIdx;           /* Index in the parent table */
    ಪೂರ್ಣಾಙ್ಕ i;                 /* Loop counter:  Foreign key number for pTab */
    ಪೂರ್ಣಾಙ್ಕ j;                 /* Loop counter:  Field of the foreign key */
    HashElem *k;           /* Loop counter:  Next table in schema */
    ಪೂರ್ಣಾಙ್ಕ x;                 /* result variable */
    ಪೂರ್ಣಾಙ್ಕ regResult;         /* 3 registers to hold a result row */
    ಪೂರ್ಣಾಙ್ಕ regRow;            /* Registers to hold a row from pTab */
    ಪೂರ್ಣಾಙ್ಕ addr_ತುದಿ;           /* Top of a loop checking foreign keys */
    ಪೂರ್ಣಾಙ್ಕ addrOk;            /* Jump here if the key is OK */
    ಪೂರ್ಣಾಙ್ಕ *aiCols;           /* child to parent column mapping */

    regResult = pParse->nMem+1;
    pParse->nMem += 4;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    ಯಾವತ್( k ){
      ಯದಿ( z_ಬಲ ){
        pTab = sqlite3LocateTable(pParse, 0, z_ಬಲ, zDb);
        k = 0;
      }ಅನ್ಯಥಾ{
        pTab = (Table*)sqliteHashData(k);
        k = sqliteHashNext(k);
      }
      ಯದಿ( pTab==0 || !IsOrdinaryTable(pTab) || pTab->u.tab.pFKey==0 ) ಅನುವರ್ತ;
      iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      zDb = db->aDb[iDb].zDbSName;
      sqlite3CodeVerifySchema(pParse, iDb);
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
      sqlite3TouchRegister(pParse, pTab->nCol+regRow);
      sqlite3OpenTable(pParse, 0, iDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, regResult, pTab->zName);
      ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pTab) );
      ಪ್ರತಿ(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        ಯದಿ( pParent==0 ) ಅನುವರ್ತ;
        pIdx = 0;
        sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
        ಯದಿ( x==0 ){
          ಯದಿ( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iDb, pParent, OP_OpenRead);
          }ಅನ್ಯಥಾ{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iDb);
            sqlite3_Vdbe_Set_P4_ಕೀಲಿ_Info(pParse, pIdx);
          }
        }ಅನ್ಯಥಾ{
          k = 0;
          ಭಙ್ಗ;
        }
      }
      ಪ್ರತಿಜ್ಞಾ( pParse->nErr>0 || pFK==0 );
      ಯದಿ( pFK ) ಭಙ್ಗ;
      ಯದಿ( pParse->nTab<i ) pParse->nTab = i;
      addr_ತುದಿ = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      ಪ್ರತಿಜ್ಞಾ( IsOrdinaryTable(pTab) );
      ಪ್ರತಿ(i=1, pFK=pTab->u.tab.pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        ಯದಿ( pParent ){
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          ಪ್ರತಿಜ್ಞಾ( x==0 || db->mallocFailed );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);

        /* Generate code to read the child key values into registers
        ** regRow..regRow+n. If any of the child key values are NULL, this
        ** row cannot cause an FK violation. Jump directly to addrOk in
        ** this case. */
        sqlite3TouchRegister(pParse, regRow + pFK->nCol);
        ಪ್ರತಿ(j=0; j<pFK->nCol; j++){
          ಪೂರ್ಣಾಙ್ಕ iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column_Of_Table(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }

        /* Generate code to query the parent index for a matching parent
        ** key. If a match is found, jump to addrOk. */
        ಯದಿ( pIdx ){
          sqlite3VdbeAddOp4(v, OP_Affinity, regRow, pFK->nCol, 0,
              sqlite3IndexAffinityStr(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regRow, pFK->nCol);
          VdbeCoverage(v);
        }ಅನ್ಯಥಾ ಯದಿ( pParent ){
          ಪೂರ್ಣಾಙ್ಕ jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          ಪ್ರತಿಜ್ಞಾ( pFK->nCol==1 || db->mallocFailed );
        }

        /* Generate code to report an FK violation to the caller. */
        ಯದಿ( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, regResult+1);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeAddOp2(v, OP_Null, 0, regResult+1);
        }
        sqlite3VdbeMultiLoad(v, regResult+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_Next, 0, addr_ತುದಿ+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addr_ತುದಿ);
    }
  }
  ಭಙ್ಗ;
#endif /* !defined(SQLITE_OMIT_TRIGGER) */
#endif /* !defined(SQLITE_OMIT_FOREIGN_KEY) */

#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  /* Reinstall the LIKE and GLOB functions.  The variant of LIKE
  ** used will be case sensitive or not depending on the RHS.
  */
  ಪಕ್ಷ PragTyp_CASE_SENSITIVE_LIKE: {
    ಯದಿ( z_ಬಲ ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(z_ಬಲ, 0));
    }
  }
  ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_INTEGRITY_CHECK: {
    ಪೂರ್ಣಾಙ್ಕ i, j, addr, mxErr;
    Table *pObjTab = 0;     /* Check only this one table, if not NULL */

    ಪೂರ್ಣಾಙ್ಕ isQuick = (sqlite3Tolower(z_ಎಡ[0])=='q');

    /* If the PRAGMA command was of the form "PRAGMA <db>.integrity_check",
    ** then iDb is set to the index of the database identified by <db>.
    ** In this case, the integrity of database iDb only is verified by
    ** the VDBE created below.
    **
    ** Otherwise, if the command was simply "PRAGMA integrity_check" (or
    ** "PRAGMA quick_check"), then iDb is set to 0. In this case, set iDb
    ** to -1 here, to indicate that the VDBE should verify the integrity
    ** of all attached databases.  */
    ಪ್ರತಿಜ್ಞಾ( iDb>=0 );
    ಪ್ರತಿಜ್ಞಾ( iDb==0 || pId2->z );
    ಯದಿ( pId2->z==0 ) iDb = -1;

    /* Initialize the VDBE program */
    pParse->nMem = 6;

    /* Set the maximum error count */
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    ಯದಿ( z_ಬಲ ){
      ಯದಿ( sqlite3GetInt32(p_ಮೌಲ್ಯ->z, &mxErr) ){
        ಯದಿ( mxErr<=0 ){
          mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
        }
      }ಅನ್ಯಥಾ{
        pObjTab = sqlite3LocateTable(pParse, 0, z_ಬಲ,
                      iDb>=0 ? db->aDb[iDb].zDbSName : 0);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1); /* reg[1] holds errors left */

    /* Do an integrity check on each database file */
    ಪ್ರತಿ(i=0; i<db->nDb; i++){
      HashElem *x;     /* For looping over tables in the schema */
      Hash *pTbls;     /* Set of all tables in the schema */
      ಪೂರ್ಣಾಙ್ಕ *a_ಬೇರು;      /* Array of root page numbers of all btrees */
      ಪೂರ್ಣಾಙ್ಕ cnt = 0;     /* Number of entries in aRoot[] */

      ಯದಿ( OMIT_TEMPDB && i==1 ) ಅನುವರ್ತ;
      ಯದಿ( iDb>=0 && i!=iDb ) ಅನುವರ್ತ;

      sqlite3CodeVerifySchema(pParse, i);
      pParse->okConstFactor = 0;  /* tag-20230327-1 */

      /* Do an integrity check of the B-Tree
      **
      ** Begin by finding the root pages numbers
      ** for all tables and indices in the database.
      */
      ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      ಪ್ರತಿ(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);  /* Current table */
        Index *pIdx;                      /* An index on pTab */
        ಯದಿ( tableSkipIntegrityCheck(pTab,pObjTab) ) ಅನುವರ್ತ;
        ಯದಿ( HasRowid(pTab) ) cnt++;
        ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){ cnt++; }
      }
      ಯದಿ( cnt==0 ) ಅನುವರ್ತ;
      ಯದಿ( pObjTab ) cnt++;
      a_ಬೇರು = sqlite3DbMallocRawNN(db, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)*(cnt+1));
      ಯದಿ( a_ಬೇರು==0 ) ಭಙ್ಗ;
      cnt = 0;
      ಯದಿ( pObjTab ) a_ಬೇರು[++cnt] = 0;
      ಪ್ರತಿ(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        ಯದಿ( tableSkipIntegrityCheck(pTab,pObjTab) ) ಅನುವರ್ತ;
        ಯದಿ( HasRowid(pTab) ) a_ಬೇರು[++cnt] = pTab->tnum;
        ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          a_ಬೇರು[++cnt] = pIdx->tnum;
        }
      }
      a_ಬೇರು[0] = cnt;

      /* Make sure sufficient number of registers have been allocated */
      sqlite3TouchRegister(pParse, 8+cnt);
      sqlite3VdbeAddOp3(v, OP_Null, 0, 8, 8+cnt);
      sqlite3ClearTempRegCache(pParse);

      /* Do the b-tree integrity checks */
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 1, cnt, 8, (ವರ್ಣ*)a_ಬೇರು,P4_INTARRAY);
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
      ಪ್ರತಿ(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        ಪೂರ್ಣಾಙ್ಕ iTab = 0;
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        ಯದಿ( tableSkipIntegrityCheck(pTab,pObjTab) ) ಅನುವರ್ತ;
        ಯದಿ( HasRowid(pTab) ){
          iTab = cnt++;
        }ಅನ್ಯಥಾ{
          iTab = cnt;
          ಪ್ರತಿ(pIdx=pTab->pIndex; ALWAYS(pIdx); pIdx=pIdx->pNext){
            ಯದಿ( Is_Primary_ಕೀಲಿ_Index(pIdx) ) ಭಙ್ಗ;
            iTab++;
          }
        }
        ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          ಯದಿ( pIdx->pPartIdxWhere==0 ){
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
      ಪ್ರತಿ(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx, *pPk;
        Index *pPrior = 0;      /* Previous index */
        ಪೂರ್ಣಾಙ್ಕ loop_ತುದಿ;
        ಪೂರ್ಣಾಙ್ಕ iDataCur, iIdxCur;
        ಪೂರ್ಣಾಙ್ಕ r1 = -1;
        ಪೂರ್ಣಾಙ್ಕ bStrict;            /* True for a STRICT table */
        ಪೂರ್ಣಾಙ್ಕ r2;                 /* Previous key for WITHOUT ROWID tables */
        ಪೂರ್ಣಾಙ್ಕ mxCol;              /* Maximum non-virtual column number */

        ಯದಿ( tableSkipIntegrityCheck(pTab,pObjTab) ) ಅನುವರ್ತ;
        ಯದಿ( !IsOrdinaryTable(pTab) ) ಅನುವರ್ತ;
        ಯದಿ( isQuick || HasRowid(pTab) ){
          pPk = 0;
          r2 = 0;
        }ಅನ್ಯಥಾ{
          pPk = sqlite3_Primary_ಕೀಲಿ_Index(pTab);
          r2 = sqlite3GetTempRange(pParse, pPk->n_ಕೀಲಿ_Col);
          sqlite3VdbeAddOp3(v, OP_Null, 1, r2, r2+pPk->n_ಕೀಲಿ_Col-1);
        }
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &iDataCur, &iIdxCur);
        /* reg[7] counts the number of entries in the table.
        ** reg[8+i] counts the number of entries in the i-th index
        */
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        ಪ್ರತಿ(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j); /* index entries counter */
        }
        ಪ್ರತಿಜ್ಞಾ( pParse->nMem>=8+j );
        ಪ್ರತಿಜ್ಞಾ( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, iDataCur, 0); VdbeCoverage(v);
        loop_ತುದಿ = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);

        /* Fetch the right-most column from the table.  This will cause
        ** the entire record header to be parsed and sanity checked.  It
        ** will also prepopulate the cursor column cache that is used
        ** by the OP_IsType code, so it is a required step.
        */
        ಪ್ರತಿಜ್ಞಾ( !IsVirtual(pTab) );
        ಯದಿ( HasRowid(pTab) ){
          mxCol = -1;
          ಪ್ರತಿ(j=0; j<pTab->nCol; j++){
            ಯದಿ( (pTab->aCol[j].colFlags & COLFLAG_VIRTUAL)==0 ) mxCol++;
          }
          ಯದಿ( mxCol==pTab->iPKey ) mxCol--;
        }ಅನ್ಯಥಾ{
          /* COLFLAG_VIRTUAL columns are not included in the WITHOUT ROWID
          ** PK index column-count, so there is no need to account for them
          ** in this case. */
          mxCol = sqlite3_Primary_ಕೀಲಿ_Index(pTab)->nColumn-1;
        }
        ಯದಿ( mxCol>=0 ){
          sqlite3VdbeAddOp3(v, OP_Column, iDataCur, mxCol, 3);
          sqlite3VdbeTypeofColumn(v, 3);
        }

        ಯದಿ( !isQuick ){
          ಯದಿ( pPk ){
            /* Verify WITHOUT ROWID keys are in ascending order */
            ಪೂರ್ಣಾಙ್ಕ a1;
            ವರ್ಣ *zErr;
            a1 = sqlite3VdbeAddOp4Int(v, OP_IdxGT, iDataCur, 0,r2,pPk->n_ಕೀಲಿ_Col);
            VdbeCoverage(v);
            sqlite3VdbeAddOp1(v, OP_IsNull, r2); VdbeCoverage(v);
            zErr = sqlite3MPrintf(db,
                   "row not in PRIMARY KEY order for %s",
                    pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, a1);
            sqlite3VdbeJumpHere(v, a1+1);
            ಪ್ರತಿ(j=0; j<pPk->n_ಕೀಲಿ_Col; j++){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Load_Index_Column(pParse, pPk, iDataCur, j, r2+j);
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
        ಪ್ರತಿ(j=0; j<pTab->nCol; j++){
          ವರ್ಣ *zErr;
          Column *pCol = pTab->aCol + j;  /* The column to be checked */
          ಪೂರ್ಣಾಙ್ಕ labelError;               /* Jump here to report an error */
          ಪೂರ್ಣಾಙ್ಕ labelOk;                  /* Jump here if all looks ok */
          ಪೂರ್ಣಾಙ್ಕ p1, p3, p4;               /* Operands to the OP_IsType opcode */
          ಪೂರ್ಣಾಙ್ಕ doTypeCheck;              /* Check datatypes (besides NOT NULL) */

          ಯದಿ( j==pTab->iPKey ) ಅನುವರ್ತ;
          ಯದಿ( bStrict ){
            doTypeCheck = pCol->eCType>COLTYPE_ANY;
          }ಅನ್ಯಥಾ{
            doTypeCheck = pCol->affinity>SQLITE_AFF_BLOB;
          }
          ಯದಿ( pCol->notNull==0 && !doTypeCheck ) ಅನುವರ್ತ;

          /* Compute the operands that will be needed for OP_IsType */
          p4 = SQLITE_NULL;
          ಯದಿ( pCol->colFlags & COLFLAG_VIRTUAL ){
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
            p1 = -1;
            p3 = 3;
          }ಅನ್ಯಥಾ{
            ಯದಿ( pCol->iDflt ){
              sqlite3_ಮೌಲ್ಯ *p_Dflt_ಮೌಲ್ಯ = 0;
              sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(db, sqlite3_Column_ಅಭಿವ್ಯಕ್ತಿ_Auth(pTab,pCol,pParse),
                                   ENC(db), pCol->affinity, &p_Dflt_ಮೌಲ್ಯ);
              ಯದಿ( p_Dflt_ಮೌಲ್ಯ ){
                p4 = sqlite3_ಮೌಲ್ಯ_type(p_Dflt_ಮೌಲ್ಯ);
                sqlite3_ಮೌಲ್ಯ_Free(p_Dflt_ಮೌಲ್ಯ);
              }
            }
            p1 = iDataCur;
            ಯದಿ( !HasRowid(pTab) ){
              testcase( j!=sqlite3TableColumnToStorage(pTab, j) );
              p3 = sqlite3TableColumnToIndex(sqlite3_Primary_ಕೀಲಿ_Index(pTab), j);
            }ಅನ್ಯಥಾ{
              p3 = sqlite3TableColumnToStorage(pTab,j);
              testcase( p3!=j);
            }
          }

          labelError = sqlite3VdbeMakeLabel(pParse);
          labelOk = sqlite3VdbeMakeLabel(pParse);
          ಯದಿ( pCol->notNull ){
            /* (1) NOT NULL columns may not contain a NULL */
            ಪೂರ್ಣಾಙ್ಕ jmp3;
            ಪೂರ್ಣಾಙ್ಕ jmp2 = sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            VdbeCoverage(v);
            ಯದಿ( p1<0 ){
              sqlite3VdbeChangeP5(v, 0x0f); /* INT, REAL, TEXT, or BLOB */
              jmp3 = jmp2;
            }ಅನ್ಯಥಾ{
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
            ಯದಿ( doTypeCheck ){
              sqlite3VdbeGoto(v, labelError);
              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeJumpHere(v, jmp3);
            }ಅನ್ಯಥಾ{
              /* VDBE byte code will fall thru */
            }
          }
          ಯದಿ( bStrict && doTypeCheck ){
            /* (2) Datatype must be exact for non-ANY columns in STRICT tables*/
            ಸ್ಥಿರ unsigned ವರ್ಣ aStdTypeMask[] = {
               0x1f,    /* ANY */
               0x18,    /* BLOB */
               0x11,    /* INT */
               0x11,    /* INTEGER */
               0x13,    /* REAL */
               0x14     /* TEXT */
            };
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            ಪ್ರತಿಜ್ಞಾ( pCol->eCType>=1 && pCol->eCType<=ಪರಿಮಾಣ(aStdTypeMask) );
            sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType-1]);
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "non-%s value in %s.%s",
                                  sqlite3StdType[pCol->eCType-1],
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }ಅನ್ಯಥಾ ಯದಿ( !bStrict && pCol->affinity==SQLITE_AFF_TEXT ){
            /* (3) Datatype for TEXT columns in non-STRICT tables must be
            **     NULL, TEXT, or BLOB. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1c); /* NULL, TEXT, or BLOB */
            VdbeCoverage(v);
            zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s",
                                  pTab->zName, pTab->aCol[j].zCnName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          }ಅನ್ಯಥಾ ಯದಿ( !bStrict && pCol->affinity>=SQLITE_AFF_NUMERIC ){
            /* (4) Datatype for numeric columns in non-STRICT tables must not
            **     be a TEXT value that can be converted to numeric. */
            sqlite3VdbeAddOp4Int(v, OP_IsType, p1, labelOk, p3, p4);
            sqlite3VdbeChangeP5(v, 0x1b); /* NULL, INT, FLOAT, or BLOB */
            VdbeCoverage(v);
            ಯದಿ( p1>=0 ){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Get_Column_Of_Table(v, pTab, iDataCur, j, 3);
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
        ಯದಿ( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          ಅಭಿವ್ಯಕ್ತಿ_List *pCheck = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pTab->pCheck, 0);
          ಯದಿ( db->mallocFailed==0 ){
            ಪೂರ್ಣಾಙ್ಕ addrCkFault = sqlite3VdbeMakeLabel(pParse);
            ಪೂರ್ಣಾಙ್ಕ addrCkOk = sqlite3VdbeMakeLabel(pParse);
            ವರ್ಣ *zErr;
            ಪೂರ್ಣಾಙ್ಕ k;
            pParse->iSelfTab = iDataCur + 1;
            ಪ್ರತಿ(k=pCheck->n_ಅಭಿವ್ಯಕ್ತಿ-1; k>0; k--){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pCheck->a[k].p_ಅಭಿವ್ಯಕ್ತಿ, addrCkFault, 0);
            }
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(pParse, pCheck->a[0].p_ಅಭಿವ್ಯಕ್ತಿ, addrCkOk,
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pCheck);
        }
        ಯದಿ( !isQuick ){ /* Omit the remaining tests for quick_check */
          /* Validate index entries for the current row */
          ಪ್ರತಿ(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            ಪೂರ್ಣಾಙ್ಕ jmp2, jmp3, jmp4, jmp5, label6;
            ಪೂರ್ಣಾಙ್ಕ kk;
            ಪೂರ್ಣಾಙ್ಕ ckUniq = sqlite3VdbeMakeLabel(pParse);
            ಯದಿ( pPk==pIdx ) ಅನುವರ್ತ;
            r1 = sqlite3_Generate_Index_ಕೀಲಿ(pParse, pIdx, iDataCur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1);/* increment entry count */
            /* Verify that an index entry exists for the current table row */
            sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            jmp2 = sqlite3VdbeAddOp3(v, OP_IFind_ಕೀಲಿ, iIdxCur+j, ckUniq, r1); 
            VdbeCoverage(v);
            sqlite3VdbeChangeP4(v, -1, (ಧ್ರುವ ವರ್ಣ*)pIdx, P4_INDEX);
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
            ಯದಿ( HasRowid(pTab) ){
              ಪೂರ್ಣಾಙ್ಕ jmp7;
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
            ಪ್ರತಿ(kk=0; kk<pIdx->n_ಕೀಲಿ_Col; kk++){
              ಯದಿ( pIdx->azColl[kk]==sqlite3_Str_ದ್ವಿಮಾನ ) ಅನುವರ್ತ;
              ಯದಿ( label6==0 ) label6 = sqlite3VdbeMakeLabel(pParse);
              sqlite3VdbeAddOp3(v, OP_Column, iIdxCur+j, kk, 3);
              sqlite3VdbeAddOp3(v, OP_Ne, 3, label6, r1+kk); VdbeCoverage(v);
            }
            ಯದಿ( label6 ){
              ಪೂರ್ಣಾಙ್ಕ jmp6 = sqlite3VdbeAddOp0(v, OP_Goto);
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
            ಯದಿ( IsUniqueIndex(pIdx) ){
              ಪೂರ್ಣಾಙ್ಕ uniqOk = sqlite3VdbeMakeLabel(pParse);
              ಪೂರ್ಣಾಙ್ಕ jmp6;
              ಪ್ರತಿ(kk=0; kk<pIdx->n_ಕೀಲಿ_Col; kk++){
                ಪೂರ್ಣಾಙ್ಕ iCol = pIdx->aiColumn[kk];
                ಪ್ರತಿಜ್ಞಾ( iCol!=XN_ROWID && iCol<pTab->nCol );
                ಯದಿ( iCol>=0 && pTab->aCol[iCol].notNull ) ಅನುವರ್ತ;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_Next, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->n_ಕೀಲಿ_Col); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_Next, iDataCur, loop_ತುದಿ); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loop_ತುದಿ-1);
        ಯದಿ( pPk ){
          ಪ್ರತಿಜ್ಞಾ( !isQuick );
          sqlite3ReleaseTempRange(pParse, r2, pPk->n_ಕೀಲಿ_Col);
        }
      }

#ifndef SQLITE_OMIT_VIRTUALTABLE
      /* Second pass to invoke the xIntegrity method on all virtual
      ** tables.
      */
      ಪ್ರತಿ(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        sqlite3_vtab *pVTab;
        ಪೂರ್ಣಾಙ್ಕ a1;
        ಯದಿ( tableSkipIntegrityCheck(pTab,pObjTab) ) ಅನುವರ್ತ;
        ಯದಿ( IsOrdinaryTable(pTab) ) ಅನುವರ್ತ;
        ಯದಿ( !IsVirtual(pTab) ) ಅನುವರ್ತ;
        ಯದಿ( pTab->nCol<=0 ){
          ಧ್ರುವ ವರ್ಣ *zMod = pTab->u.vtab.azArg[0];
          ಯದಿ( sqlite3HashFind(&db->aModule, zMod)==0 ) ಅನುವರ್ತ;
        }
        sqlite3ViewGetColumnNames(pParse, pTab);
        ಯದಿ( pTab->u.vtab.p==0 ) ಅನುವರ್ತ;
        pVTab = pTab->u.vtab.p->pVtab;
        ಯದಿ( NEVER(pVTab==0) ) ಅನುವರ್ತ;
        ಯದಿ( NEVER(pVTab->pModule==0) ) ಅನುವರ್ತ;
        ಯದಿ( pVTab->pModule->iVersion<4 ) ಅನುವರ್ತ;
        ಯದಿ( pVTab->pModule->xIntegrity==0 ) ಅನುವರ್ತ;
        sqlite3VdbeAddOp3(v, OP_VCheck, i, 3, isQuick);
        pTab->nTabRef++;
        sqlite3VdbeAppendP4(v, pTab, P4_TABLEREF);
        a1 = sqlite3VdbeAddOp1(v, OP_IsNull, 3); VdbeCoverage(v);
        integrityCheckResultRow(v);
        sqlite3VdbeJumpHere(v, a1);
        ಅನುವರ್ತ;
      }
#endif
    }
    {
      ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ iLn = VDBE_OFFSET_LINENO(2);
      ಸ್ಥಿರ ಧ್ರುವ VdbeOpList endCode[] = {
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
      ಯದಿ( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (ವರ್ಣ*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_ENCODING: {
    ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ EncName {
      ವರ್ಣ *zName;
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
    ಧ್ರುವ ಸಂರಚನಾ EncName *pEnc;
    ಯದಿ( !z_ಬಲ ){    /* "PRAGMA encoding" */
      ಯದಿ( sqlite3ReadSchema(pParse) ) ಗಚ್ಛ pragma_out;
      ಪ್ರತಿಜ್ಞಾ( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      ಪ್ರತಿಜ್ಞಾ( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      ಪ್ರತಿಜ್ಞಾ( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }ಅನ್ಯಥಾ{                        /* "PRAGMA encoding = XXX" */
      /* Only change the value of sqlite.enc if the database handle is not
      ** initialized. If the main database exists, the new sqlite.enc value
      ** will be overwritten when the schema is next loaded. If it does not
      ** already exists, it will be created to use the new encoding value.
      */
      ಯದಿ( (db->mDbFlags & DBFLAG_EncodingFixed)==0 ){
        ಪ್ರತಿ(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          ಯದಿ( 0==sqlite3StrICmp(z_ಬಲ, pEnc->zName) ){
            u8 enc = pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            SCHEMA_ENC(db) = enc;
            sqlite3SetTextEncoding(db, enc);
            ಭಙ್ಗ;
          }
        }
        ಯದಿ( !pEnc->zName ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "unsupported encoding: %s", z_ಬಲ);
        }
      }
    }
  }
  ಭಙ್ಗ;
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
  ಪಕ್ಷ Prag_Typ_HEADER_ಮೌಲ್ಯ: {
    ಪೂರ್ಣಾಙ್ಕ iCookie = pPragma->iArg;  /* Which cookie to read or write */
    sqlite3VdbeUsesBtree(v, iDb);
    ಯದಿ( z_ಬಲ && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      /* Write the specified cookie value */
      ಸ್ಥಿರ ಧ್ರುವ VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},    /* 0 */
        { OP_SetCookie,      0,  0,  0},    /* 1 */
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(setCookie), setCookie, 0);
      ಯದಿ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ಭಙ್ಗ;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(z_ಬಲ);
      aOp[1].p5 = 1;
      ಯದಿ( iCookie==BTREE_SCHEMA_VERSION && (db->flags & SQLITE_Defensive)!=0 ){
        /* Do not allow the use of PRAGMA schema_version=VALUE in defensive
        ** mode.  Change the OP_SetCookie opcode into a no-op.  */
        aOp[1].opcode = OP_Noop;
      }
    }ಅನ್ಯಥಾ{
      /* Read the specified cookie value */
      ಸ್ಥಿರ ಧ್ರುವ VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},    /* 0 */
        { OP_ReadCookie,      0,  1,  0},    /* 1 */
        { OP_ResultRow,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(readCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(readCookie),readCookie,0);
      ಯದಿ( ONLY_IF_REALLOC_STRESS(aOp==0) ) ಭಙ್ಗ;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  ಭಙ್ಗ;
#endif /* SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS */

#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  /*
  **   PRAGMA compile_options
  **
  ** Return the names of all compile-time options used in this build,
  ** one option per row.
  */
  ಪಕ್ಷ PragTyp_COMPILE_OPTIONS: {
    ಪೂರ್ಣಾಙ್ಕ i = 0;
    ಧ್ರುವ ವರ್ಣ *zOpt;
    pParse->nMem = 1;
    ಯಾವತ್( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  ಭಙ್ಗ;
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

#ifndef SQLITE_OMIT_WAL
  /*
  **   PRAGMA [schema.]wal_checkpoint = passive|full|restart|truncate
  **
  ** Checkpoint the database.
  */
  ಪಕ್ಷ PragTyp_WAL_CHECKPOINT: {
    ಪೂರ್ಣಾಙ್ಕ iBt = (pId2->z?iDb:SQLITE_MAX_DB);
    ಪೂರ್ಣಾಙ್ಕ eMode = SQLITE_CHECKPOINT_PASSIVE;
    ಯದಿ( z_ಬಲ ){
      ಯದಿ( sqlite3StrICmp(z_ಬಲ, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }ಅನ್ಯಥಾ ಯದಿ( sqlite3StrICmp(z_ಬಲ, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }ಅನ್ಯಥಾ ಯದಿ( sqlite3StrICmp(z_ಬಲ, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }ಅನ್ಯಥಾ ಯದಿ( sqlite3StrICmp(z_ಬಲ, "noop")==0 ){
        eMode = SQLITE_CHECKPOINT_NOOP;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 3);
  }
  ಭಙ್ಗ;

  /*
  **   PRAGMA wal_autocheckpoint
  **   PRAGMA wal_autocheckpoint = N
  **
  ** Configure a database connection to automatically checkpoint a database
  ** after accumulating N frames in the log. Or query for the current value
  ** of N.
  */
  ಪಕ್ಷ PragTyp_WAL_AUTOCHECKPOINT: {
    ಯದಿ( z_ಬಲ ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(z_ಬಲ));
    }
    returnSingleInt(v,
       db->xWalCallback==sqlite3WalDefaultHook ?
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  ಭಙ್ಗ;
#endif

  /*
  **  PRAGMA shrink_memory
  **
  ** IMPLEMENTATION-OF: R-23445-46109 This pragma causes the database
  ** connection on which it is invoked to free up as much memory as it
  ** can, by calling sqlite3_db_release_memory().
  */
  ಪಕ್ಷ PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_OPTIMIZE: {
    ಪೂರ್ಣಾಙ್ಕ iDbLast;           /* Loop termination point for the schema loop */
    ಪೂರ್ಣಾಙ್ಕ iTabCur;           /* Cursor for a table whose size needs checking */
    HashElem *k;           /* Loop over tables of a schema */
    Schema *pSchema;       /* The current schema */
    Table *pTab;           /* A table in the schema */
    Index *pIdx;           /* An index of the table */
    LogEst szThreshold;    /* Size threshold above which reanalysis needed */
    ವರ್ಣ *zSubSql;         /* SQL statement for the OP_SqlExec opcode */
    u32 opMask;            /* Mask of operations to perform */
    ಪೂರ್ಣಾಙ್ಕ nLimit;            /* Analysis limit to use */
    ಪೂರ್ಣಾಙ್ಕ nCheck = 0;        /* Number of tables to be optimized */
    ಪೂರ್ಣಾಙ್ಕ nBtree = 0;        /* Number of btrees to scan */
    ಪೂರ್ಣಾಙ್ಕ nIndex;            /* Number of indexes on the current table */

    ಯದಿ( z_ಬಲ ){
      opMask = (u32)sqlite3Atoi(z_ಬಲ);
      ಯದಿ( (opMask & 0x02)==0 ) ಭಙ್ಗ;
    }ಅನ್ಯಥಾ{
      opMask = 0xfffe;
    }
    ಯದಿ( (opMask & 0x10)==0 ){
      nLimit = 0;
    }ಅನ್ಯಥಾ ಯದಿ( db->nAnalysisLimit>0
           && db->nAnalysisLimit<SQLITE_DEFAULT_OPTIMIZE_LIMIT ){
      nLimit = 0;
    }ಅನ್ಯಥಾ{
      nLimit = SQLITE_DEFAULT_OPTIMIZE_LIMIT;
    }
    iTabCur = pParse->nTab++;
    ಪ್ರತಿ(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      ಯದಿ( iDb==1 ) ಅನುವರ್ತ;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      ಪ್ರತಿ(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
        pTab = (Table*)sqliteHashData(k);

        /* This only works for ordinary tables */
        ಯದಿ( !IsOrdinaryTable(pTab) ) ಅನುವರ್ತ;

        /* Do not scan system tables */
        ಯದಿ( 0==sqlite3StrNICmp(pTab->zName, "sqlite_", 7) ) ಅನುವರ್ತ;

        /* Find the size of the table as last recorded in sqlite_stat1.
        ** If any index is unanalyzed, then the threshold is -1 to
        ** indicate a new, unanalyzed index
        */
        szThreshold = pTab->nRowLogEst;
        nIndex = 0;
        ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          nIndex++;
          ಯದಿ( !pIdx->hasStat1 ){
            szThreshold = -1; /* Always analyze if any index lacks statistics */
          }
        }

        /* If table pTab has not been used in a way that would benefit from
        ** having analysis statistics during the current session, then skip it,
        ** unless the 0x10000 MASK bit is set. */
        ಯದಿ( (pTab->tabFlags & TF_MaybeReanalyze)!=0 ){
          /* Check for size change if stat1 has been used for a query */
        }ಅನ್ಯಥಾ ಯದಿ( opMask & 0x10000 ){
          /* Check for size change if 0x10000 is set */
        }ಅನ್ಯಥಾ ಯದಿ( pTab->pIndex!=0 && szThreshold<0 ){
          /* Do analysis if unanalyzed indexes exists */
        }ಅನ್ಯಥಾ{
          /* Otherwise, we can skip this table */
          ಅನುವರ್ತ;
        }

        nCheck++;
        ಯದಿ( nCheck==2 ){
          /* If ANALYZE might be invoked two or more times, hold a write
          ** transaction for efficiency */
          sqlite3BeginWriteOperation(pParse, 0, iDb);
        }
        nBtree += nIndex+1;

        /* Reanalyze if the table is 10 times larger or smaller than
        ** the last analysis.  Unconditional reanalysis if there are
        ** unanalyzed indexes. */
        sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
        ಯದಿ( szThreshold>=0 ){
          ಧ್ರುವ LogEst iRange = 33;   /* 10x size change */
          sqlite3VdbeAddOp4Int(v, OP_IfSizeBetween, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1),
                         szThreshold>=iRange ? szThreshold-iRange : -1,
                         szThreshold+iRange);
          VdbeCoverage(v);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeAddOp2(v, OP_Rewind, iTabCur,
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1));
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        ಯದಿ( opMask & 0x01 ){
          ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ResultRow, r1, 1);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeAddOp4(v, OP_SqlExec, nLimit ? 0x02 : 00, nLimit, 0,
                            zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);

    /* In a schema with a large number of tables and indexes, scale back
    ** the analysis_limit to avoid excess run-time in the worst case.
    */
    ಯದಿ( !db->mallocFailed && nLimit>0 && nBtree>100 ){
      ಪೂರ್ಣಾಙ್ಕ iAddr, iEnd;
      VdbeOp *aOp;
      nLimit = 100*nLimit/nBtree;
      ಯದಿ( nLimit<100 ) nLimit = 100;
      aOp = sqlite3VdbeGetOp(v, 0);
      iEnd = sqlite3VdbeCurrentAddr(v);
      ಪ್ರತಿ(iAddr=0; iAddr<iEnd; iAddr++){
        ಯದಿ( aOp[iAddr].opcode==OP_SqlExec ) aOp[iAddr].p2 = nLimit;
      }
    }
    ಭಙ್ಗ;
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
  /*case PragTyp_BUSY_TIMEOUT*/ ಸ್ವಭಾವ: {
    ಪೂರ್ಣಾಙ್ಕ n = 0;
    ಪ್ರತಿಜ್ಞಾ( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    ಯದಿ( z_ಬಲ && sqlite3GetInt32(z_ಬಲ,&n) ){
      sqlite3_busy_timeout(db, n);
    }
    returnSingleInt(v, db->busyTimeout);
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    ಯದಿ( z_ಬಲ && sqlite3DecOrHexToI64(z_ಬಲ, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    ಭಙ್ಗ;
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
  ಪಕ್ಷ PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    ಯದಿ( z_ಬಲ && sqlite3DecOrHexToI64(z_ಬಲ, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      ಯದಿ( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    ಭಙ್ಗ;
  }

  /*
  **   PRAGMA threads
  **   PRAGMA threads = N
  **
  ** Configure the maximum number of worker threads.  Return the new
  ** maximum, which might be less than requested.
  */
  ಪಕ್ಷ PragTyp_THREADS: {
    sqlite3_int64 N;
    ಯದಿ( z_ಬಲ
     && sqlite3DecOrHexToI64(z_ಬಲ, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (ಪೂರ್ಣಾಙ್ಕ)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    ಭಙ್ಗ;
  }

  /*
  **   PRAGMA analysis_limit
  **   PRAGMA analysis_limit = N
  **
  ** Configure the maximum number of rows that ANALYZE will examine
  ** in each index that it looks at.  Return the new limit.
  */
  ಪಕ್ಷ PragTyp_ANALYSIS_LIMIT: {
    sqlite3_int64 N;
    ಯದಿ( z_ಬಲ
     && sqlite3DecOrHexToI64(z_ಬಲ, &N)==SQLITE_OK /* IMP: R-40975-20399 */
     && N>=0
    ){
      db->nAnalysisLimit = (ಪೂರ್ಣಾಙ್ಕ)(N&0x7fffffff);
    }
    returnSingleInt(v, db->nAnalysisLimit); /* IMP: R-57594-65522 */
    ಭಙ್ಗ;
  }

#ಯದಿ defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  /*
  ** Report the current state of file logs for all databases
  */
  ಪಕ್ಷ PragTyp_LOCK_STATUS: {
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    ಪೂರ್ಣಾಙ್ಕ i;
    pParse->nMem = 2;
    ಪ್ರತಿ(i=0; i<db->nDb; i++){
      Btree *pBt;
      ಧ್ರುವ ವರ್ಣ *zState = "unknown";
      ಪೂರ್ಣಾಙ್ಕ j;
      ಯದಿ( db->aDb[i].zDbSName==0 ) ಅನುವರ್ತ;
      pBt = db->aDb[i].pBt;
      ಯದಿ( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }ಅನ್ಯಥಾ ಯದಿ( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0,
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    ಭಙ್ಗ;
  }
#endif

#ಯದಿ defined(SQLITE_ENABLE_CEROD)
  ಪಕ್ಷ PragTyp_ACTIVATE_EXTENSIONS: ಯದಿ( z_ಬಲ ){
    ಯದಿ( sqlite3StrNICmp(z_ಬಲ, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&z_ಬಲ[6]);
    }
  }
  ಭಙ್ಗ;
#endif

  } /* End of the PRAGMA switch */

  /* The following block is a no-op unless SQLITE_DEBUG is defined. Its only
  ** purpose is to execute assert() statements to verify that if the
  ** PragFlg_NoColumns1 flag is set and the caller specified an argument
  ** to the PRAGMA, the implementation has not added any OP_ResultRow
  ** instructions to the VM.  */
  ಯದಿ( (pPragma->mPragFlg & PragFlg_NoColumns1) && z_ಬಲ ){
    sqlite3VdbeVerifyNoResultRow(v);
  }

pragma_out:
  sqlite3DbFree(db, z_ಎಡ);
  sqlite3DbFree(db, z_ಬಲ);
}
#ifndef SQLITE_OMIT_VIRTUALTABLE
/*****************************************************************************
** Implementation of an eponymous virtual table that runs a pragma.
**
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ PragmaVtab PragmaVtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ PragmaVtabCursor PragmaVtabCursor;
ಸಂರಚನಾ PragmaVtab {
  sqlite3_vtab base;        /* Base class.  Must be first */
  sqlite3 *db;              /* The database connection to which it belongs */
  ಧ್ರುವ PragmaName *pName;  /* Name of the pragma */
  u8 nHidden;               /* Number of hidden columns */
  u8 iHidden;               /* Index of the first hidden column */
};
ಸಂರಚನಾ PragmaVtabCursor {
  sqlite3_vtab_cursor base; /* Base class.  Must be first */
  sqlite3_stmt *pPragma;    /* The pragma statement to run */
  sqlite_int64 iRowid;      /* Current rowid */
  ವರ್ಣ *azArg[2];           /* Value of the argument and schema */
};

/*
** Pragma virtual table module xConnect method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  ಧ್ರುವ PragmaName *pPragma = (ಧ್ರುವ PragmaName*)pAux;
  PragmaVtab *pTab = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ವರ್ಣ cSep = '(';
  StrAccum acc;
  ವರ್ಣ zBuf[200];

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);
  sqlite3StrAccumInit(&acc, 0, zBuf, ಪರಿಮಾಣ(zBuf), 0);
  sqlite3_str_appendall(&acc, "CREATE TABLE x");
  ಪ್ರತಿ(i=0, j=pPragma->iPragCName; i<pPragma->nPragCName; i++, j++){
    sqlite3_str_appendf(&acc, "%c\"%s\"", cSep, pragCName[j]);
    cSep = ',';
  }
  ಯದಿ( i==0 ){
    sqlite3_str_appendf(&acc, "(\"%s\"", pPragma->zName);
    i++;
  }
  j = 0;
  ಯದಿ( pPragma->mPragFlg & PragFlg_Result1 ){
    sqlite3_str_appendall(&acc, ",arg HIDDEN");
    j++;
  }
  ಯದಿ( pPragma->mPragFlg & (PragFlg_SchemaOpt|PragFlg_SchemaReq) ){
    sqlite3_str_appendall(&acc, ",schema HIDDEN");
    j++;
  }
  sqlite3_str_append(&acc, ")", 1);
  sqlite3StrAccumFinish(&acc);
  ಪ್ರತಿಜ್ಞಾ( strlen(zBuf) < ಪರಿಮಾಣ(zBuf)-1 );
  rc = sqlite3_declare_vtab(db, zBuf);
  ಯದಿ( rc==SQLITE_OK ){
    pTab = (PragmaVtab*)sqlite3_malloc(ಪರಿಮಾಣ(PragmaVtab));
    ಯದಿ( pTab==0 ){
      rc = SQLITE_NOMEM;
    }ಅನ್ಯಥಾ{
      memset(pTab, 0, ಪರಿಮಾಣ(PragmaVtab));
      pTab->pName = pPragma;
      pTab->db = db;
      pTab->iHidden = i;
      pTab->nHidden = j;
    }
  }ಅನ್ಯಥಾ{
    *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Pragma virtual table module xDisconnect method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabDisconnect(sqlite3_vtab *pVtab){
  PragmaVtab *pTab = (PragmaVtab*)pVtab;
  sqlite3_free(pTab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Figure out the best index to use to search a pragma virtual table.
**
** There are not really any index choices.  But we want to encourage the
** query planner to give == constraints on as many hidden parameters as
** possible, and especially on the first hidden parameter.  So return a
** high cost if hidden parameters are unconstrained.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  PragmaVtab *pTab = (PragmaVtab*)tab;
  ಧ್ರುವ ಸಂರಚನಾ sqlite3_index_constraint *pConstraint;
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಪೂರ್ಣಾಙ್ಕ seen[2];

  pIdxInfo->estimatedCost = (ದ್ವಿಗುಣ)1;
  ಯದಿ( pTab->nHidden==0 ){ ಪ್ರತ್ಯಾಗಮ SQLITE_OK; }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  ಪ್ರತಿ(i=0; i<pIdxInfo->nConstraint; i++, pConstraint++){
    ಯದಿ( pConstraint->iColumn < pTab->iHidden ) ಅನುವರ್ತ;
    ಯದಿ( pConstraint->op!=SQLITE_INDEX_CONSTRAINT_EQ ) ಅನುವರ್ತ;
    ಯದಿ( pConstraint->usable==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_CONSTRAINT;
    j = pConstraint->iColumn - pTab->iHidden;
    ಪ್ರತಿಜ್ಞಾ( j < 2 );
    seen[j] = i+1;
  }
  ಯದಿ( seen[0]==0 ){
    pIdxInfo->estimatedCost = (ದ್ವಿಗುಣ)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }
  j = seen[0]-1;
  pIdxInfo->aConstraintUsage[j].argvIndex = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (ದ್ವಿಗುಣ)20;
  pIdxInfo->estimatedRows = 20;
  ಯದಿ( seen[1] ){
    j = seen[1]-1;
    pIdxInfo->aConstraintUsage[j].argvIndex = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Create a new cursor for the pragma virtual table */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor*)sqlite3_malloc(ಪರಿಮಾಣ(*pCsr));
  ಯದಿ( pCsr==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pCsr, 0, ಪರಿಮಾಣ(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Clear all content from pragma virtual table cursor. */
ಸ್ಥಿರ ಶೂನ್ಯ pragmaVtabCursorClear(PragmaVtabCursor *pCsr){
  ಪೂರ್ಣಾಙ್ಕ i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  ಪ್ರತಿ(i=0; i<ArraySize(pCsr->azArg); i++){
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}

/* Close a pragma virtual table cursor */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabClose(sqlite3_vtab_cursor *cur){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Advance the pragma virtual table cursor to the next row */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabNext(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;

  /* Increment the xRowid value */
  pCsr->iRowid++;
  ಪ್ರತಿಜ್ಞಾ( pCsr->pPragma );
  ಯದಿ( SQLITE_ROW!=sqlite3_step(pCsr->pPragma) ){
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Pragma virtual table module xFilter method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabFilter(
  sqlite3_vtab_cursor *pVtabCursor,
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ i, j;
  StrAccum acc;
  ವರ್ಣ *zSql;

  UNUSED_PARAMETER(idx_ಅಂಕೆ);
  UNUSED_PARAMETER(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & PragFlg_Result1)!=0 ? 0 : 1;
  ಪ್ರತಿ(i=0; i<argc; i++, j++){
    ಧ್ರುವ ವರ್ಣ *zText = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
    ಪ್ರತಿಜ್ಞಾ( j<ArraySize(pCsr->azArg) );
    ಪ್ರತಿಜ್ಞಾ( pCsr->azArg[j]==0 );
    ಯದಿ( zText ){
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      ಯದಿ( pCsr->azArg[j]==0 ){
        ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_LENGTH]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  ಯದಿ( pCsr->azArg[1] ){
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  ಯದಿ( pCsr->azArg[0] ){
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  ಯದಿ( zSql==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  ಯದಿ( rc!=SQLITE_OK ){
    pTab->base.z_Err_ಸಂದೇಶ = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    ಪ್ರತ್ಯಾಗಮ rc;
  }
  ಪ್ರತ್ಯಾಗಮ pragmaVtabNext(pVtabCursor);
}

/*
** Pragma virtual table module xEof method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  ಪ್ರತ್ಯಾಗಮ (pCsr->pPragma==0);
}

/* The xColumn method simply returns the corresponding column from
** the PRAGMA. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabColumn(
  sqlite3_vtab_cursor *pVtabCursor,
  sqlite3_context *ctx,
  ಪೂರ್ಣಾಙ್ಕ i
){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab*)(pVtabCursor->pVtab);
  ಯದಿ( i<pTab->iHidden ){
    sqlite3_result_ಮೌಲ್ಯ(ctx, sqlite3_column_ಮೌಲ್ಯ(pCsr->pPragma, i));
  }ಅನ್ಯಥಾ{
    sqlite3_result_text(ctx, pCsr->azArg[i-pTab->iHidden],-1,SQLITE_TRANSIENT);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Pragma virtual table module xRowid method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p){
  PragmaVtabCursor *pCsr = (PragmaVtabCursor*)pVtabCursor;
  *p = pCsr->iRowid;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* The pragma virtual table object */
ಸ್ಥಿರ ಧ್ರುವ sqlite3_module pragmaVtabModule = {
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
Module *sqlite3PragmaVtabRegister(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zName){
  ಧ್ರುವ PragmaName *pName;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_strnicmp(zName, "pragma_", 7)==0 );
  pName = pragmaLocate(zName+7);
  ಯದಿ( pName==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( (pName->mPragFlg & (PragFlg_Result0|PragFlg_Result1))==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತಿಜ್ಞಾ( sqlite3HashFind(&db->aModule, zName)==0 );
  ಪ್ರತ್ಯಾಗಮ sqlite3VtabCreateModule(db, zName, &pragmaVtabModule, (ಶೂನ್ಯ*)pName, 0);
}

#endif /* SQLITE_OMIT_VIRTUALTABLE */

#endif /* SQLITE_OMIT_PRAGMA */
